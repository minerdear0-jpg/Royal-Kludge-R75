#include "game_mode.h"
#include "socd_cleaner.h"
#include "rgb_matrix.h"

static bool    game_mode_enabled = false;
static uint8_t saved_rgb_mode    = RGB_MATRIX_SOLID_COLOR;
static bool    saved_no_gui      = false;

static const uint8_t wsad_leds[] = {GAME_MODE_LED_W, GAME_MODE_LED_S, GAME_MODE_LED_A, GAME_MODE_LED_D};

void game_mode_init(void) {
    game_mode_enabled    = false;
    socd_cleaner_enabled = false;
}

static void game_mode_enable(void) {
    game_mode_enabled    = true;
    socd_cleaner_enabled = true;
    saved_rgb_mode       = rgb_matrix_get_mode();
    saved_no_gui         = keymap_config.no_gui;
    keymap_config.no_gui = true;
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);

    if (!keymap_config.nkro) {
        clear_keyboard();
        keymap_config.nkro = true;
    }
}

static void game_mode_disable(void) {
    game_mode_enabled    = false;
    socd_cleaner_enabled = false;
    keymap_config.no_gui = saved_no_gui;
    rgb_matrix_mode_noeeprom(saved_rgb_mode);
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

    for (uint8_t i = 0; i < 4; i++) {
        const uint8_t led_index = wsad_leds[i];
        if (led_index >= led_min && led_index < led_max) {
            rgb_matrix_set_color(led_index, GAME_MODE_RED);
        }
    }
}
