#include "lighting_profile.h"
#include "custom_keycodes.h"
#include "defines.h"
#include "game_mode.h"
#include "rgb_matrix.h"
#include "eeconfig.h"
#include "timer.h"
#include "color.h"

enum {
    LP_DAY = 0,
    LP_TWILIGHT,
    LP_NIGHT,
    LP_GAMING,
    LP_COUNT
};

#define FADE_CIRC_MS  3000
#define FADE_GAME_MS  500
#define BLINK_MS      120
#define SLEEP_DAY_MS        (10UL * 60UL * 1000UL)
#define SLEEP_TWILIGHT_MS   (5UL * 60UL * 1000UL)
#define SLEEP_NIGHT_MS      (2UL * 60UL * 1000UL)
#define SLEEP_GAMING_MS     (10UL * 60UL * 1000UL)

#define SCALE(c, pct) ((uint8_t)(((uint16_t)(c) * (pct)) / 100))

static const rgb_t k_rgb[LP_COUNT] = {
    {255, 255, 255},
    {SCALE(255, 50), SCALE(140, 50), SCALE(0, 50)},
    {SCALE(0, 15), SCALE(95, 15), SCALE(10, 15)},
    {SCALE(215, 40), SCALE(255, 40), SCALE(0, 40)},
};

static uint8_t  profile     = LP_DAY;
static uint8_t  last_circ   = LP_DAY;
static uint32_t last_input  = 0;
static bool     rgb_asleep  = false;
static bool     blinking    = false;
static bool     fading      = false;
static uint32_t blink_at    = 0;
static uint32_t fade_at     = 0;
static uint16_t fade_ms     = FADE_CIRC_MS;
static rgb_t    fade_from   = {255, 255, 255};
static rgb_t    fade_to     = {255, 255, 255};
static rgb_t    shown       = {255, 255, 255};

static rgb_t with_val(rgb_t c) {
    const uint8_t v = rgb_matrix_get_val();
    rgb_t         o;
    o.r = (uint8_t)((uint16_t)c.r * v / 255);
    o.g = (uint8_t)((uint16_t)c.g * v / 255);
    o.b = (uint8_t)((uint16_t)c.b * v / 255);
    return o;
}

static void fill_range(uint8_t led_min, uint8_t led_max, rgb_t c) {
    const rgb_t s = with_val(c);
    for (uint8_t i = led_min; i < led_max; i++) {
        rgb_matrix_set_color(i, s.r, s.g, s.b);
    }
}

static uint8_t lerp8(uint8_t a, uint8_t b, uint16_t t, uint16_t max) {
    if (max == 0) {
        return b;
    }
    return (uint8_t)(a + (((int16_t)b - (int16_t)a) * (int16_t)t) / (int16_t)max);
}

static rgb_t lerp_rgb(rgb_t a, rgb_t b, uint16_t t, uint16_t max) {
    rgb_t o;
    o.r = lerp8(a.r, b.r, t, max);
    o.g = lerp8(a.g, b.g, t, max);
    o.b = lerp8(a.b, b.b, t, max);
    return o;
}

static rgb_t profile_paint_rgb(uint8_t id) {
    if (id == LP_GAMING) {
        return (rgb_t){0, 0, 0};
    }
    return k_rgb[id];
}

static uint32_t sleep_ms(void) {
    switch (profile) {
        case LP_TWILIGHT:
            return SLEEP_TWILIGHT_MS;
        case LP_NIGHT:
            return SLEEP_NIGHT_MS;
        case LP_GAMING:
            return SLEEP_GAMING_MS;
        default:
            return SLEEP_DAY_MS;
    }
}

static void write_eeprom(void) {
    eeconfig_update_user((uint32_t)profile);
}

static void apply_base(rgb_t rgb) {
    shown = rgb;
    rgb_matrix_enable_noeeprom();
    if (rgb_matrix_get_mode() != RGB_MATRIX_NONE) {
        rgb_matrix_mode_noeeprom(RGB_MATRIX_NONE);
    }
}

static void sync_game_features(void) {
    game_mode_set_active(profile == LP_GAMING && !fading && !blinking);
}

static void begin_switch(uint8_t next) {
    const uint8_t prev = profile;
    fade_from          = shown;
    profile            = next;
    if (next != LP_GAMING) {
        last_circ = next;
    }
    write_eeprom();
    fade_to  = profile_paint_rgb(next);
    fade_ms  = (prev == LP_GAMING || next == LP_GAMING) ? FADE_GAME_MS : FADE_CIRC_MS;
    blinking = true;
    fading   = false;
    rgb_asleep = false;
    blink_at = timer_read32();
    apply_base(fade_from);
    sync_game_features();
}

void eeconfig_init_user(void) {
    eeconfig_update_user(LP_DAY);
}

void lighting_profile_init(void) {
    const uint32_t raw = eeconfig_read_user();
    profile            = (raw < LP_COUNT) ? (uint8_t)raw : LP_DAY;
    last_circ          = (profile == LP_GAMING) ? LP_DAY : profile;
    last_input         = timer_read32();
    rgb_asleep         = false;
    blinking           = false;
    fading             = false;
    shown              = profile_paint_rgb(profile);
    apply_base(shown);
    sync_game_features();
}

void lighting_profile_cycle(void) {
    begin_switch((uint8_t)((profile + 1) % LP_COUNT));
    last_input = timer_read32();
}

void lighting_profile_toggle_gaming(void) {
    if (profile == LP_GAMING) {
        begin_switch(last_circ);
    } else {
        begin_switch(LP_GAMING);
    }
    last_input = timer_read32();
}

void lighting_profile_note_activity(void) {
    last_input = timer_read32();
    if (rgb_asleep) {
        rgb_asleep = false;
        rgb_matrix_enable_noeeprom();
        shown = profile_paint_rgb(profile);
        apply_base(shown);
        sync_game_features();
    }
}

bool lighting_profile_is_gaming(void) {
    return profile == LP_GAMING && !fading && !blinking;
}

void lighting_profile_host_off(void) {}

void lighting_profile_host_on(void) {
    last_input = timer_read32();
}

bool lighting_profile_process(uint16_t keycode, keyrecord_t *record) {
    const bool hit = (keycode == LIGHT_PROFILE_CYC) ||
                     (IS_LAYER_ON(_WIN_FN_LYR) && record->event.key.row == LIGHT_CYCLE_ROW &&
                      record->event.key.col == LIGHT_CYCLE_COL);
    if (!hit) {
        return true;
    }
    if (record->event.pressed) {
        lighting_profile_cycle();
    }
    return false;
}

void lighting_profile_task(void) {
    if (blinking && timer_elapsed32(blink_at) >= BLINK_MS) {
        blinking = false;
        fading   = true;
        fade_at  = timer_read32();
        apply_base(fade_from);
        sync_game_features();
    }

    if (fading) {
        const uint32_t elapsed = timer_elapsed32(fade_at);
        if (elapsed >= fade_ms) {
            fading = false;
            shown  = fade_to;
            apply_base(shown);
            sync_game_features();
        } else {
            shown = lerp_rgb(fade_from, fade_to, (uint16_t)elapsed, fade_ms);
        }
        return;
    }

    if (!rgb_asleep && timer_elapsed32(last_input) >= sleep_ms()) {
        rgb_asleep = true;
        game_mode_set_active(false);
        rgb_matrix_disable_noeeprom();
    }
}

void lighting_profile_paint(uint8_t led_min, uint8_t led_max) {
    if (rgb_asleep) {
        return;
    }
    if (blinking) {
        fill_range(led_min, led_max, k_rgb[profile]);
        return;
    }
    if (fading) {
        fill_range(led_min, led_max, shown);
        return;
    }
    if (profile == LP_GAMING) {
        game_mode_apply_lighting(led_min, led_max);
        return;
    }
    fill_range(led_min, led_max, k_rgb[profile]);
}

uint8_t lighting_profile_scale_u8(uint8_t c) {
    return (uint8_t)((uint16_t)c * rgb_matrix_get_val() / 255);
}
