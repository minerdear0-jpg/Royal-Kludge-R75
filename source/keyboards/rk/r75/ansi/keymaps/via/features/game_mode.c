#include "game_mode.h"
#include "socd_cleaner.h"
#include "lighting_profile.h"
#include "rgb_matrix.h"
#include "defines.h"

#define LED_UP 63

static bool    game_mode_enabled = false;
static uint8_t saved_rgb_mode    = RGB_MATRIX_SOLID_COLOR;
static hsv_t saved_hsv = {0, 0, 128};
static bool    saved_no_gui      = false;

static void paint(uint8_t led_min, uint8_t led_max, uint8_t idx, uint8_t r, uint8_t g, uint8_t b) {
    if (idx >= led_min && idx < led_max) {
        rgb_matrix_set_color(idx, r, g, b);
    }
}

static void paint_list(uint8_t led_min, uint8_t led_max, const uint8_t *ids, uint8_t n, uint8_t r, uint8_t g, uint8_t b) {
    for (uint8_t i = 0; i < n; i++) {
        paint(led_min, led_max, ids[i], r, g, b);
    }
}

void game_mode_init(void) {
    game_mode_enabled    = false;
    socd_cleaner_enabled = false;
}

static void game_mode_enable(void) {
    game_mode_enabled    = true;
    socd_cleaner_enabled = true;
    saved_rgb_mode       = rgb_matrix_get_mode();
    saved_hsv            = rgb_matrix_get_hsv();
    saved_no_gui         = keymap_config.no_gui;
    keymap_config.no_gui = true;
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);

    if (!keymap_config.nkro) {
        clear_keyboard();
        keymap_config.nkro = true;
    }
    lighting_profile_game_enter();
}

static void game_mode_disable(void) {
    game_mode_enabled    = false;
    socd_cleaner_enabled = false;
    keymap_config.no_gui = saved_no_gui;
    if (lighting_profile_user_selected()) {
        lighting_profile_on_game_exit();
    } else {
        rgb_matrix_mode_noeeprom(saved_rgb_mode);
        rgb_matrix_sethsv_noeeprom(saved_hsv.h, saved_hsv.s, saved_hsv.v);
    }
}

bool game_mode_toggle(void) {
    if (game_mode_enabled) {
        game_mode_disable();
    } else {
        game_mode_enable();
    }
    return game_mode_enabled;
}

bool game_mode_is_active(void) {
    return game_mode_enabled;
}

void game_mode_apply_lighting(uint8_t led_min, uint8_t led_max) {
    if (!game_mode_enabled) {
        return;
    }

    for (uint8_t i = led_min; i < led_max; i++) {
        rgb_matrix_set_color(i, 0, 0, 0);
    }

    const uint8_t wasd[] = {GAME_MODE_LED_W, GAME_MODE_LED_S, GAME_MODE_LED_A, GAME_MODE_LED_D};
    paint_list(led_min, led_max, wasd, 4, 0xC8, 0xFF, 0x00);

    const uint8_t arrows[] = {LED_LEFT, LED_DOWN, LED_UP, LED_RIGHT};
    paint_list(led_min, led_max, arrows, 4, 0xC8, 0xFF, 0x00);

    const uint8_t slots[] = {23, 24, 25, 26, 27};
    paint_list(led_min, led_max, slots, 5, 0xA0, 0x00, 0xFF);

    paint(led_min, led_max, 28, 0x00, 0xC8, 0xC8); /* 6 */

    const uint8_t mods[] = {LED_LSFT, LED_LCTL, LED_LGUI, LED_LALT, LED_SPACE, 64};
    paint_list(led_min, led_max, mods, 6, 0x00, 0xC8, 0xC8);

#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
    for (uint8_t j = 0; j < g_last_hit_tracker.count; j++) {
        const uint16_t tick = g_last_hit_tracker.tick[j];
        if (tick >= 400) {
            continue;
        }
        const uint8_t v = (uint8_t)(255 - (tick * 255 / 400));
        paint(led_min, led_max, g_last_hit_tracker.index[j], v, v, v);
    }
#endif
}
