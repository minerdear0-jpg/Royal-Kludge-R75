#include "lighting_profile.h"
#include "game_mode.h"
#include "rgb_matrix.h"
#include "eeconfig.h"
#include "timer.h"
#include <lib/lib8tion/lib8tion.h>

enum { LP_DAY = 0, LP_TWILIGHT, LP_NIGHT };

#define SESSION_TWILIGHT_AT (4UL * 60UL * 60UL * 1000UL)
#define SESSION_NIGHT_AT    (6UL * 60UL * 60UL * 1000UL)
#define IDLE_RESET_MS       (45UL * 60UL * 1000UL)
#define SLEEP_DAY_MS        (10UL * 60UL * 1000UL)
#define SLEEP_TWILIGHT_MS   (5UL * 60UL * 1000UL)
#define SLEEP_NIGHT_MS      (2UL * 60UL * 1000UL)
#define FADE_MS             (60UL * 1000UL)
#define NIGHTLIGHT_V        51

static const hsv_t k_hsv[3] = {
    {0, 0, 255},
    {21, 255, 148},
    {0, 255, NIGHTLIGHT_V},
};

static uint8_t  profile            = LP_DAY;
static uint32_t session_start      = 0;
static uint32_t pause_accum        = 0;
static uint32_t game_mark          = 0;
static uint32_t last_input         = 0;
static bool     frozen             = false;
static bool     rgb_asleep         = false;
static bool     nightlight         = false;
static bool     fading             = false;
static uint32_t fade_start         = 0;
static hsv_t    fade_from          = {0, 0, 255};
static hsv_t    fade_to            = {0, 0, 255};

static uint8_t lerp8(uint8_t a, uint8_t b, uint8_t t) {
    return (uint8_t)(((uint16_t)a * (255 - t) + (uint16_t)b * t) / 255);
}

static uint8_t lerp_hue(uint8_t a, uint8_t b, uint8_t t) {
    int16_t dh = (int16_t)b - (int16_t)a;
    if (dh > 127) {
        dh -= 256;
    } else if (dh < -128) {
        dh += 256;
    }
    return (uint8_t)(a + (dh * (int16_t)t) / 255);
}

static void write_eeprom(void) {
    eeconfig_update_user((uint32_t)profile);
}

static void apply_hsv(hsv_t hsv) {
    rgb_matrix_enable_noeeprom();
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
    rgb_matrix_sethsv_noeeprom(hsv.h, hsv.s, hsv.v);
}

static void start_fade(uint8_t next) {
    profile    = next;
    fade_from  = rgb_matrix_get_hsv();
    fade_to    = k_hsv[next];
    fade_start = timer_read32();
    fading     = true;
    rgb_asleep = false;
    apply_hsv(fade_from);
}

static uint32_t session_ms(void) {
    const uint32_t elapsed = timer_elapsed32(session_start);
    if (elapsed < pause_accum) {
        return 0;
    }
    return elapsed - pause_accum;
}

static uint32_t sleep_timeout_ms(void) {
    switch (profile) {
        case LP_TWILIGHT:
            return SLEEP_TWILIGHT_MS;
        case LP_NIGHT:
            return SLEEP_NIGHT_MS;
        default:
            return SLEEP_DAY_MS;
    }
}

void eeconfig_init_user(void) {
    eeconfig_update_user(LP_DAY);
}

void lighting_profile_init(void) {
    const uint32_t raw = eeconfig_read_user();
    profile            = (raw <= LP_NIGHT) ? (uint8_t)raw : LP_DAY;
    session_start      = timer_read32();
    pause_accum        = 0;
    last_input         = timer_read32();
    frozen             = false;
    rgb_asleep         = false;
    nightlight         = false;
    fading             = false;
    apply_hsv(k_hsv[profile]);
}

void lighting_profile_note_activity(void) {
    last_input = timer_read32();
    if (rgb_asleep && !game_mode_is_active()) {
        rgb_asleep = false;
        apply_hsv(k_hsv[profile]);
    }
}

void lighting_profile_cycle(void) {
    start_fade((uint8_t)((profile + 1) % 3));
    write_eeprom();
    lighting_profile_note_activity();
}

void lighting_profile_game_enter(void) {
    frozen      = true;
    game_mark   = timer_read32();
    fading      = false;
    rgb_asleep  = false;
    nightlight  = false;
    rgb_matrix_enable_noeeprom();
}

void lighting_profile_game_exit(void) {
    if (frozen) {
        pause_accum += timer_elapsed32(game_mark);
        frozen = false;
    }
    last_input = timer_read32();
    start_fade(profile);
}

void lighting_profile_on_game_exit(void) {
    lighting_profile_game_exit();
}

bool lighting_profile_user_selected(void) {
    return true;
}

void lighting_profile_host_off(void) {
    if (game_mode_is_active()) {
        return;
    }
    /* Linux USB autosuspend hits after ~2s idle. Only nightlight after the
     * same idle window as RGB sleep, so a pause while typing is not "PC off". */
    if (timer_elapsed32(last_input) < sleep_timeout_ms()) {
        return;
    }
    nightlight = true;
    fading     = false;
    rgb_asleep = false;
    apply_hsv(k_hsv[LP_NIGHT]);
    rgb_matrix_set_color_all(NIGHTLIGHT_V, 0, 0);
    rgb_matrix_update_pwm_buffers();
}

void lighting_profile_host_on(void) {
    last_input = timer_read32();
    if (!nightlight) {
        return;
    }
    nightlight    = false;
    session_start = timer_read32();
    pause_accum   = 0;
    frozen        = false;
    start_fade(profile);
}

void lighting_profile_task(void) {
    if (nightlight || game_mode_is_active()) {
        return;
    }

    if (timer_elapsed32(last_input) >= IDLE_RESET_MS) {
        session_start = timer_read32();
        pause_accum   = 0;
        last_input    = timer_read32();
        if (profile != LP_DAY) {
            profile = LP_DAY;
            write_eeprom();
        }
        fading = false;
        if (!rgb_asleep) {
            start_fade(LP_DAY);
        }
        return;
    }

    if (!frozen && !fading && !rgb_asleep) {
        const uint32_t up = session_ms();
        if (up >= SESSION_NIGHT_AT && profile != LP_NIGHT) {
            start_fade(LP_NIGHT);
        } else if (up >= SESSION_TWILIGHT_AT && profile == LP_DAY) {
            start_fade(LP_TWILIGHT);
        }
    }

    if (fading) {
        uint32_t elapsed = timer_elapsed32(fade_start);
        if (elapsed >= FADE_MS) {
            fading = false;
            apply_hsv(fade_to);
        } else {
            const uint8_t t = (uint8_t)((elapsed * 255UL) / FADE_MS);
            hsv_t         hsv;
            hsv.h = lerp_hue(fade_from.h, fade_to.h, t);
            hsv.s = lerp8(fade_from.s, fade_to.s, t);
            hsv.v = lerp8(fade_from.v, fade_to.v, t);
            apply_hsv(hsv);
        }
        return;
    }

    if (!rgb_asleep && timer_elapsed32(last_input) >= sleep_timeout_ms()) {
        rgb_asleep = true;
        rgb_matrix_disable_noeeprom();
        return;
    }

    if (!rgb_asleep && profile == LP_TWILIGHT) {
        const uint8_t wave = sin8((uint8_t)(timer_read() / 24));
        const uint8_t val  = (uint8_t)(140 + ((int8_t)(wave - 128) / 12));
        rgb_matrix_sethsv_noeeprom(21, 255, val);
    }
}
