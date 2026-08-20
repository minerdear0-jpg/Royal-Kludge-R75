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
    LP_GREEN,
    LP_RED,
    LP_GAMING,
    LP_COUNT
};

#define FADE_CIRC_MS  3000
#define FADE_GAME_MS  500
#define FADE_SLEEP_MS 2000
#define BLINK_MS      120
#define SLEEP_DAY_MS        (10UL * 60UL * 1000UL)
#define SLEEP_TWILIGHT_MS   (5UL * 60UL * 1000UL)
#define SLEEP_GREEN_MS      (10UL * 60UL * 1000UL)
#define SLEEP_RED_MS        (10UL * 60UL * 1000UL)
#define SLEEP_GAMING_MS     (10UL * 60UL * 1000UL)
#define EEPROM_DEFER_MS     2500
#define USB_SUSPEND_RGB_MS  10000
#define WIPE_MS             650
#define WIPE_BAND           18
#ifndef CAPS_LOCK_LED_PIN
#    define CAPS_LOCK_LED_PIN C4
#endif
#define HB_LED              21
#define TX_ON_MS            45
#define TX_PERIOD_MS        180
#define HB_JUMP_MS          2500
#define DEFAULT_VAL         128
#define DEFAULT_NL_VAL      64
#define STORE_MARK          0x80000000UL
#define STORE_V2            0x40000000UL

#define SCALE(c, pct) ((uint8_t)(((uint16_t)(c) * (pct)) / 100))

static const rgb_t k_rgb[LP_COUNT] = {
    {255, 255, 255},
    {SCALE(255, 70), SCALE(120, 70), 0},
    {SCALE(0, 40), SCALE(220, 40), SCALE(40, 40)},
    {SCALE(255, 40), SCALE(28, 40), 0},
    {SCALE(215, 40), SCALE(255, 40), SCALE(0, 40)},
};

static const rgb_t k_nl = {SCALE(255, 22), SCALE(8, 22), 0};

static uint8_t  profile     = LP_DAY;
static uint8_t  last_circ   = LP_DAY;
static uint8_t  vals[LP_COUNT];
static uint8_t  nl_val      = DEFAULT_NL_VAL;
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
static bool     wipe;
static uint32_t wipe_at;
static bool     hb;
static uint32_t hb_at;

static void abort_bootloader_wait(void);

static uint8_t pack_val4(uint8_t v) {
    return (uint8_t)(v >> 4);
}

static uint8_t unpack_val4(uint8_t p) {
    return (uint8_t)(p << 4);
}

static uint32_t pack_store(void) {
    uint8_t  i;
    uint32_t raw = STORE_MARK | STORE_V2 | (uint32_t)(profile & 7);
    raw |= ((uint32_t)pack_val4(nl_val) << 3);
    for (i = 0; i < LP_COUNT; i++) {
        raw |= ((uint32_t)pack_val4(vals[i]) << (7 + (i * 4)));
    }
    return raw;
}

static void default_vals(void) {
    uint8_t i;
    for (i = 0; i < LP_COUNT; i++) {
        vals[i] = DEFAULT_VAL;
    }
    nl_val = DEFAULT_NL_VAL;
}

static void unpack_store(uint32_t raw) {
    uint8_t i;
    if ((raw & STORE_MARK) && (raw & STORE_V2)) {
        profile = (uint8_t)(raw & 7);
        if (profile >= LP_COUNT) {
            profile = LP_DAY;
        }
        nl_val = unpack_val4((uint8_t)((raw >> 3) & 0x0F));
        for (i = 0; i < LP_COUNT; i++) {
            vals[i] = unpack_val4((uint8_t)((raw >> (7 + (i * 4))) & 0x0F));
        }
        return;
    }
    default_vals();
    if (raw & STORE_MARK) {
        profile = (uint8_t)(raw & 7);
        if (profile >= LP_COUNT) {
            profile = LP_DAY;
        }
        for (i = 0; i < LP_COUNT; i++) {
            vals[i] = (uint8_t)(((raw >> (3 + (i * 5))) & 0x1F) << 3);
        }
        return;
    }
    if (raw < 4) {
        profile = (uint8_t)raw;
        return;
    }
    {
        const uint8_t mapped[4] = {LP_DAY, LP_TWILIGHT, LP_GREEN, LP_GAMING};
        profile = mapped[raw & 3];
        vals[0] = (uint8_t)(((raw >> 3) & 0x7F) << 1);
        vals[1] = (uint8_t)(((raw >> 10) & 0x7F) << 1);
        vals[2] = (uint8_t)(((raw >> 17) & 0x7F) << 1);
        vals[4] = (uint8_t)(((raw >> 24) & 0x7F) << 1);
        vals[3] = DEFAULT_VAL;
    }
}

static void apply_val(uint8_t v) {
    rgb_matrix_sethsv_noeeprom(rgb_matrix_get_hue(), rgb_matrix_get_sat(), v);
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
        case LP_GREEN:
            return SLEEP_GREEN_MS;
        case LP_RED:
            return SLEEP_RED_MS;
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
    return nl_usb || nl_level != 0 || nl_run || courtesy_lamp;
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
        apply_val(nl_val);
    }
    apply_base(shown);
}

static void begin_switch(uint8_t next) {
    const uint8_t prev = profile;
    if (!in_nightlight()) {
        vals[profile] = rgb_matrix_get_val();
    }
    snap_courtesy_on();
    fade_from          = shown;
    profile            = next;
    if (next != LP_GAMING) {
        last_circ = next;
    }
    schedule_eeprom();
    fade_to    = profile_paint_rgb(next);
    fade_ms    = (prev == LP_GAMING || next == LP_GAMING) ? FADE_GAME_MS : FADE_CIRC_MS;
    blinking   = true;
    fading     = false;
    rgb_asleep = false;
    blink_at   = timer_read32();
    apply_val(vals[next]);
    apply_base(fade_from);
    sync_game_features();
}

void eeconfig_init_user(void) {
    profile = LP_DAY;
    default_vals();
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
    nl_level   = 0;
    shown      = profile_paint_rgb(profile);
    apply_val(vals[profile]);
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
    /* Linux HID autosuspend ~2s. After USB_SUSPEND_RGB_MS of real host sleep
     * (VBUS still on) fade to green nightlight. Encoder stays live. */
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
    if (nl_usb || nl_level || nl_run) {
        nl_usb = false;
        begin_nl(0, false);
    }
}

bool lighting_profile_is_nightlight(void) {
    return in_nightlight();
}

bool lighting_profile_encoder(bool clockwise) {
    if (hb) {
        abort_bootloader_wait();
        return false;
    }
    if (!in_nightlight()) {
        return true;
    }
    uint8_t v = nl_val;
    if (clockwise) {
        v = (v > (uint8_t)(255 - RGB_MATRIX_VAL_STEP)) ? 255 : (uint8_t)(v + RGB_MATRIX_VAL_STEP);
    } else {
        v = (v < RGB_MATRIX_VAL_STEP) ? 0 : (uint8_t)(v - RGB_MATRIX_VAL_STEP);
    }
    nl_val = v;
    apply_val(v);
    schedule_eeprom();
    last_input = timer_read32();
    return false;
}

static bool tx_lit(uint32_t elapsed) {
    return (elapsed % TX_PERIOD_MS) < TX_ON_MS;
}

static void tx_caps(bool on) {
#ifdef CAPS_LOCK_LED_PIN
    gpio_write_pin(CAPS_LOCK_LED_PIN, on ? LED_PIN_ON_STATE : (LED_PIN_ON_STATE ? 0 : 1));
#endif
}

static void abort_bootloader_wait(void) {
    hb = false;
    tx_caps(false);
#ifdef CAPS_LOCK_LED_PIN
    led_update_ports(host_keyboard_led_state());
#endif
}

static void paint_esc_tx(uint8_t led_min, uint8_t led_max, bool on) {
    tx_caps(on);
    for (uint8_t i = led_min; i < led_max; i++) {
        if (on && i == HB_LED) {
            rgb_matrix_set_color(i, 0xFF, 0x90, 0x00);
        } else {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
    }
}

bool lighting_profile_process(uint16_t keycode, keyrecord_t *record) {
    if (hb) {
        if (record->event.pressed && keycode != QK_BOOTLOADER) {
            abort_bootloader_wait();
        }
        return false;
    }
    if (keycode == QK_BOOTLOADER) {
        if (record->event.pressed) {
            lighting_profile_enter_bootloader();
        }
        return false;
    }
    if (keycode == KC_MUTE && in_nightlight()) {
        if (record->event.pressed) {
            lamp_click();
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

void lighting_profile_wipe_then_reset(void) {
    if (hb) {
        return;
    }
    wipe    = true;
    wipe_at = timer_read32();
    rgb_matrix_enable_noeeprom();
}

void lighting_profile_enter_bootloader(void) {
    if (wipe) {
        return;
    }
    hb    = true;
    hb_at = timer_read32();
    rgb_matrix_enable_noeeprom();
}

bool lighting_profile_wipe_busy(void) {
    return wipe || hb;
}

void lighting_profile_contrast_rgb(uint8_t *r, uint8_t *g, uint8_t *b) {
    rgb_t c;
    if (nl_level) {
        c = k_nl;
    } else if (profile == LP_GAMING) {
        c = (rgb_t){0, 0, 0};
    } else {
        c = k_rgb[profile];
    }
    uint8_t        cr = (uint8_t)(255 - c.r);
    uint8_t        cg = (uint8_t)(255 - c.g);
    uint8_t        cb = (uint8_t)(255 - c.b);
    const uint16_t y  = (uint16_t)(c.r * 3u + c.g * 6u + c.b) / 10u;
    const uint16_t yc = (uint16_t)(cr * 3u + cg * 6u + cb) / 10u;
    int16_t        d  = (int16_t)y - (int16_t)yc;
    if (d < 0) {
        d = (int16_t)(-d);
    }
    if (d < 80) {
        if (y >= 128) {
            cr = 0;
            cg = 0;
            cb = 0;
        } else {
            cr = 255;
            cg = 255;
            cb = 255;
        }
    }
    *r = cr;
    *g = cg;
    *b = cb;
}

void lighting_profile_task(void) {
    if (hb) {
        if (timer_elapsed32(hb_at) >= HB_JUMP_MS) {
            reset_keyboard();
        }
        return;
    }
    if (wipe) {
        if (timer_elapsed32(wipe_at) >= (WIPE_MS + 80)) {
            eeconfig_init();
            soft_reset_keyboard();
        }
        return;
    }
    if (eeprom_dirty && timer_elapsed32(eeprom_at) >= EEPROM_DEFER_MS) {
        flush_eeprom();
    }

    if (!nl_run && !courtesy_run && !fading && !blinking && !rgb_asleep) {
        const uint8_t v = rgb_matrix_get_val();
        if (in_nightlight()) {
            if (nl_val != v) {
                nl_val = v;
                schedule_eeprom();
            }
        } else if (vals[profile] != v) {
            vals[profile] = v;
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
                sync_game_features();
            }
        } else {
            courtesy = lerp8(courtesy_from, courtesy_to, (uint16_t)elapsed, FADE_SLEEP_MS);
        }
        return;
    }

    if (!nl_usb && nl_level == 0 && !courtesy_lamp && !rgb_asleep && !host_rgb_off &&
        timer_elapsed32(last_input) >= sleep_ms()) {
        begin_courtesy(0, false);
    }
}

void lighting_profile_paint(uint8_t led_min, uint8_t led_max) {
    if (hb) {
        paint_esc_tx(led_min, led_max, tx_lit(timer_elapsed32(hb_at)));
        return;
    }
    if (wipe) {
        const uint32_t elapsed = timer_elapsed32(wipe_at);
        const uint32_t pos     = (elapsed >= WIPE_MS) ? 224u : (elapsed * 224u / WIPE_MS);
        for (uint8_t i = led_min; i < led_max; i++) {
            const uint16_t x = g_led_config.point[i].x;
            if ((uint32_t)x + WIPE_BAND > pos && (uint32_t)x < pos + WIPE_BAND) {
                rgb_matrix_set_color(i, 0xE8, 0xF4, 0xFF);
            } else {
                rgb_matrix_set_color(i, 0, 0, 0);
            }
        }
        return;
    }
    if (rgb_asleep || host_rgb_off) {
        return;
    }
    if (nl_level) {
        const rgb_t base = (blinking || fading) ? shown : profile_paint_rgb(profile);
        fill_range(led_min, led_max, lerp_rgb(base, k_nl, nl_level, 255));
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
