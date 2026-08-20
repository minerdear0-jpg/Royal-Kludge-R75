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
#define FADE_SLEEP_MS 2000
#define BLINK_MS      120
#define SLEEP_DAY_MS        (10UL * 60UL * 1000UL)
#define SLEEP_TWILIGHT_MS   (5UL * 60UL * 1000UL)
#define SLEEP_NIGHT_MS      (10UL * 60UL * 1000UL)
#define SLEEP_GAMING_MS     (10UL * 60UL * 1000UL)
#define EEPROM_DEFER_MS     2500
#define USB_SUSPEND_RGB_MS  10000
#define DEFAULT_VAL         128

#define SCALE(c, pct) ((uint8_t)(((uint16_t)(c) * (pct)) / 100))

static const rgb_t k_rgb[LP_COUNT] = {
    {255, 255, 255},
    {SCALE(255, 50), SCALE(140, 50), SCALE(0, 50)},
    {SCALE(0, 15), SCALE(95, 15), SCALE(10, 15)},
    {SCALE(215, 40), SCALE(255, 40), SCALE(0, 40)},
};

static uint8_t  profile     = LP_DAY;
static uint8_t  last_circ   = LP_DAY;
static uint8_t  vals[LP_COUNT];
static bool     nl_armed;
static uint8_t  nl_level;
static bool     nl_run;
static bool     nl_usb;
static uint8_t  nl_from;
static uint8_t  nl_to;
static uint32_t nl_at;
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
static uint8_t  courtesy      = 255;
static bool     courtesy_run;
static bool     courtesy_host;
static bool     courtesy_lamp;
static uint8_t  courtesy_from = 255;
static uint8_t  courtesy_to   = 255;
static uint32_t courtesy_at;

static uint8_t pack_val(uint8_t v) {
    return (uint8_t)(v >> 1);
}

static uint8_t unpack_val(uint8_t p) {
    return (uint8_t)(p << 1);
}

static uint32_t pack_store(void) {
    uint32_t raw = (uint32_t)(profile & 3);
    if (nl_armed) {
        raw |= (1UL << 2);
    }
    raw |= ((uint32_t)pack_val(vals[0]) << 3);
    raw |= ((uint32_t)pack_val(vals[1]) << 10);
    raw |= ((uint32_t)pack_val(vals[2]) << 17);
    raw |= ((uint32_t)pack_val(vals[3]) << 24);
    return raw;
}

static void unpack_store(uint32_t raw) {
    uint8_t i;
    if (raw < LP_COUNT) {
        profile  = (uint8_t)raw;
        nl_armed = false;
        for (i = 0; i < LP_COUNT; i++) {
            vals[i] = DEFAULT_VAL;
        }
        return;
    }
    profile  = (uint8_t)(raw & 3);
    nl_armed = (raw & (1UL << 2)) != 0;
    vals[0]  = unpack_val((uint8_t)((raw >> 3) & 0x7F));
    vals[1]  = unpack_val((uint8_t)((raw >> 10) & 0x7F));
    vals[2]  = unpack_val((uint8_t)((raw >> 17) & 0x7F));
    vals[3]  = unpack_val((uint8_t)((raw >> 24) & 0x7F));
}

static void apply_val(uint8_t v) {
    rgb_matrix_sethsv_noeeprom(rgb_matrix_get_hue(), rgb_matrix_get_sat(), v);
}

static uint8_t val_slot(void) {
    return (nl_armed || nl_usb || nl_level == 255) ? LP_NIGHT : profile;
}

static uint8_t scale_chan(uint8_t c) {
    return (uint8_t)((uint32_t)c * rgb_matrix_get_val() * courtesy / (255UL * 255UL));
}

static rgb_t with_val(rgb_t c) {
    rgb_t o;
    o.r = scale_chan(c.r);
    o.g = scale_chan(c.g);
    o.b = scale_chan(c.b);
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
    eeconfig_update_user(pack_store());
    eeprom_dirty = false;
}

static void rgb_park(void) {
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

static bool in_nightlight(void) {
    return nl_armed || nl_usb || nl_level != 0 || nl_run || courtesy_lamp;
}

static void begin_courtesy(uint8_t to, bool lamp) {
    if (courtesy == to && !courtesy_run) {
        return;
    }
    if (courtesy_run && courtesy_to == to) {
        return;
    }
    courtesy_from = courtesy;
    courtesy_to   = to;
    courtesy_at   = timer_read32();
    courtesy_run  = true;
    courtesy_host = (to == 0) && usb_down && !lamp;
    courtesy_lamp = lamp;
    rgb_asleep    = false;
    if (to != 0) {
        rgb_matrix_enable_noeeprom();
        apply_base(shown);
        sync_game_features();
    }
}

static void snap_courtesy_on(void) {
    courtesy      = 255;
    courtesy_run  = false;
    courtesy_host = false;
    courtesy_lamp = false;
    rgb_asleep    = false;
}

static void lamp_click(void) {
    last_input = timer_read32();
    if (courtesy_run) {
        begin_courtesy(courtesy_to == 0 ? 255 : 0, true);
        return;
    }
    begin_courtesy(courtesy == 0 ? 255 : 0, true);
}

static void begin_nl(uint8_t to, bool usb) {
    if (nl_level == to && !nl_run) {
        if (usb) {
            nl_usb = true;
        }
        return;
    }
    if (nl_run && nl_to == to) {
        if (usb) {
            nl_usb = true;
        }
        return;
    }
    nl_from = nl_level;
    nl_to   = to;
    nl_at   = timer_read32();
    nl_run  = true;
    nl_usb  = usb || (to == 255 && usb_down);
    snap_courtesy_on();
    rgb_matrix_enable_noeeprom();
    if (to == 255) {
        apply_val(vals[LP_NIGHT]);
    }
    apply_base(shown);
}

static void lighting_profile_toggle_nightlight(void) {
    nl_armed = !nl_armed;
    schedule_eeprom();
    last_input = timer_read32();
    if (nl_armed) {
        begin_nl(255, false);
    } else if (!usb_down && !nl_usb) {
        begin_nl(0, false);
    }
}

static void begin_switch(uint8_t next) {
    const uint8_t prev = profile;
    vals[val_slot()]   = rgb_matrix_get_val();
    snap_courtesy_on();
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
    if (!nl_armed && nl_level == 0 && !nl_run) {
        apply_val(vals[next]);
    }
    apply_base(fade_from);
    sync_game_features();
}

void eeconfig_init_user(void) {
    uint8_t i;
    profile  = LP_DAY;
    nl_armed = false;
    for (i = 0; i < LP_COUNT; i++) {
        vals[i] = DEFAULT_VAL;
    }
    eeconfig_update_user(pack_store());
}

void lighting_profile_init(void) {
    unpack_store(eeconfig_read_user());
    last_circ  = (profile == LP_GAMING) ? LP_DAY : profile;
    last_input = timer_read32();
    snap_courtesy_on();
    blinking   = false;
    fading     = false;
    nl_run     = false;
    nl_usb     = false;
    nl_level   = nl_armed ? 255 : 0;
    shown      = profile_paint_rgb(profile);
    apply_val(nl_armed ? vals[LP_NIGHT] : vals[profile]);
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
    if (host_rgb_off) {
        host_rgb_off = false;
    }
    if (rgb_asleep || (!courtesy_lamp && (courtesy < 255 || (courtesy_run && courtesy_to == 0)))) {
        begin_courtesy(255, false);
    }
}

bool lighting_profile_is_gaming(void) {
    return profile == LP_GAMING && !fading && !blinking;
}

void lighting_profile_host_off(void) {
    /* Linux HID autosuspend fires after ~2s idle. Wait USB_SUSPEND_RGB_MS
     * then fade into nightlight. A5 / LED_ENABLE_PIN stays high. */
    if (!usb_down) {
        usb_down    = true;
        usb_down_at = timer_read32();
    }
}

void lighting_profile_host_on(void) {
    last_input = timer_read32();
    usb_down   = false;
    if (rgb_asleep) {
        if (host_rgb_off) {
            host_rgb_off = false;
        }
        return;
    }
    if (host_rgb_off) {
        host_rgb_off = false;
        begin_courtesy(255, false);
    }
    if (nl_usb || (nl_run && nl_to == 255 && !nl_armed)) {
        nl_usb = false;
        if (!nl_armed) {
            begin_nl(0, false);
        }
    }
}

bool lighting_profile_is_nightlight(void) {
    return in_nightlight();
}

bool lighting_profile_encoder(bool clockwise) {
    if (!in_nightlight()) {
        return true;
    }
    uint8_t v = vals[LP_NIGHT];
    if (clockwise) {
        v = (v > (uint8_t)(255 - RGB_MATRIX_VAL_STEP)) ? 255 : (uint8_t)(v + RGB_MATRIX_VAL_STEP);
    } else {
        v = (v < RGB_MATRIX_VAL_STEP) ? 0 : (uint8_t)(v - RGB_MATRIX_VAL_STEP);
    }
    vals[LP_NIGHT] = v;
    apply_val(v);
    schedule_eeprom();
    last_input = timer_read32();
    return false;
}

bool lighting_profile_process(uint16_t keycode, keyrecord_t *record) {
    if (keycode == KC_MUTE && in_nightlight()) {
        if (record->event.pressed) {
            lamp_click();
        }
        return false;
    }
    const bool nl = (keycode == LIGHT_NIGHTLIGHT) ||
                    (IS_LAYER_ON(_WIN_FN_LYR) && record->event.key.row == NIGHTLIGHT_ROW &&
                     record->event.key.col == NIGHTLIGHT_COL);
    if (nl) {
        if (record->event.pressed) {
            lighting_profile_toggle_nightlight();
        }
        return false;
    }
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

    if (!nl_run && !courtesy_run && !fading && !blinking && !rgb_asleep) {
        const uint8_t slot = val_slot();
        const uint8_t v    = rgb_matrix_get_val();
        if (vals[slot] != v) {
            vals[slot] = v;
            schedule_eeprom();
        }
    }

    if (usb_down && !rgb_asleep && timer_elapsed32(usb_down_at) >= USB_SUSPEND_RGB_MS) {
        if (courtesy_run && courtesy_to == 0) {
            courtesy_run = false;
            courtesy     = 255;
        }
        begin_nl(255, true);
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

    if (nl_run) {
        const uint32_t elapsed = timer_elapsed32(nl_at);
        if (elapsed >= FADE_SLEEP_MS) {
            nl_level = nl_to;
            nl_run   = false;
            if (nl_level == 0) {
                nl_usb = false;
                apply_val(vals[profile]);
            }
        } else {
            nl_level = lerp8(nl_from, nl_to, (uint16_t)elapsed, FADE_SLEEP_MS);
        }
        return;
    }

    if (courtesy_run) {
        const uint32_t elapsed = timer_elapsed32(courtesy_at);
        if (elapsed >= FADE_SLEEP_MS) {
            courtesy     = courtesy_to;
            courtesy_run = false;
            if (courtesy == 0) {
                if (courtesy_lamp) {
                    courtesy_host = false;
                } else if (usb_down || courtesy_host) {
                    host_rgb_off  = true;
                    courtesy_host = false;
                    rgb_park();
                } else {
                    rgb_asleep = true;
                    game_mode_set_active(false);
                    rgb_park();
                }
            } else {
                courtesy_host = false;
                if (!courtesy_lamp) {
                    courtesy_lamp = false;
                }
                sync_game_features();
            }
        } else {
            courtesy = lerp8(courtesy_from, courtesy_to, (uint16_t)elapsed, FADE_SLEEP_MS);
        }
        return;
    }

    if (!nl_armed && nl_level == 0 && !courtesy_lamp && !rgb_asleep && !host_rgb_off &&
        timer_elapsed32(last_input) >= sleep_ms()) {
        begin_courtesy(0, false);
    }
}

void lighting_profile_paint(uint8_t led_min, uint8_t led_max) {
    if (rgb_asleep || host_rgb_off) {
        return;
    }
    if (nl_level) {
        const rgb_t base = (blinking || fading) ? shown : profile_paint_rgb(profile);
        fill_range(led_min, led_max, lerp_rgb(base, k_rgb[LP_NIGHT], nl_level, 255));
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
    return scale_chan(c);
}
