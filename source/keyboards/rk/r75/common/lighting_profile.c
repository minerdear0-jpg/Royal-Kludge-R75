#include "lighting_profile.h"
#include "custom_keycodes.h"
#include "features/defines.h"
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
#define EEPROM_DEFER_MS     2500
#define USB_SUSPEND_RGB_MS  10000

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
static bool     eeprom_dirty;
static uint32_t eeprom_at;
static bool     usb_down;
static uint32_t usb_down_at;
static bool     host_rgb_off;

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

static void schedule_eeprom(void) {
    eeprom_dirty = true;
    eeprom_at    = timer_read32();
}

static void flush_eeprom(void) {
    if (!eeprom_dirty) {
        return;
    }
    eeconfig_update_user((uint32_t)profile);
    eeprom_dirty = false;
}

static void rgb_hold_black(void) {
    if (host_rgb_off) {
        return;
    }
    host_rgb_off = true;
    flush_eeprom();
    rgb_matrix_set_color_all(0, 0, 0);
    rgb_matrix_update_pwm_buffers();
    rgb_matrix_disable_noeeprom();
}

static void apply_base(rgb_t rgb) {
    shown = rgb;
    rgb_matrix_enable_noeeprom();
    if (rgb_matrix_get_mode() != RGB_MATRIX_SOLID_COLOR) {
        rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
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
    schedule_eeprom();
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
    if (host_rgb_off) {
        host_rgb_off = false;
        rgb_matrix_enable_noeeprom();
        apply_base(shown);
        sync_game_features();
    }
}

bool lighting_profile_is_gaming(void) {
    return profile == LP_GAMING && !fading && !blinking;
}

void lighting_profile_host_off(void) {
    /* Linux HID autosuspend fires after ~2s idle. Do not black the strip
     * immediately — wait USB_SUSPEND_RGB_MS of continuous suspend (lid close /
     * host sleep). A5 / LED_ENABLE_PIN stays high; cutting it glitches RGB. */
    if (!usb_down) {
        usb_down    = true;
        usb_down_at = timer_read32();
    }
}

void lighting_profile_host_on(void) {
    last_input = timer_read32();
    usb_down   = false;
    if (host_rgb_off) {
        host_rgb_off = false;
        if (!rgb_asleep) {
            rgb_matrix_enable_noeeprom();
            apply_base(shown);
        }
    }
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
    if (eeprom_dirty && timer_elapsed32(eeprom_at) >= EEPROM_DEFER_MS) {
        flush_eeprom();
    }

    if (usb_down && !rgb_asleep && !host_rgb_off && timer_elapsed32(usb_down_at) >= USB_SUSPEND_RGB_MS) {
        rgb_hold_black();
    }

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

    if (!rgb_asleep && !host_rgb_off && timer_elapsed32(last_input) >= sleep_ms()) {
        rgb_asleep = true;
        game_mode_set_active(false);
        flush_eeprom();
        rgb_matrix_disable_noeeprom();
    }
}

void lighting_profile_paint(uint8_t led_min, uint8_t led_max) {
    if (rgb_asleep || host_rgb_off) {
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
