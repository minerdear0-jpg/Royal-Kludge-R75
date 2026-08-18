// Copyright 2024 R75 Custom Firmware
// SPDX-License-Identifier: GPL-2.0-or-later

#include "game_mode.h"
#include "rgb_matrix.h"

// Game Mode state - static to avoid global namespace pollution
static bool game_mode_enabled = false;

// WSAD LED indices array for efficient iteration
static const uint8_t wsad_leds[4] = {LED_W_INDEX, LED_S_INDEX, LED_A_INDEX, LED_D_INDEX};

void game_mode_init(void) {
    game_mode_enabled = false;
}

bool game_mode_toggle(void) {
    game_mode_enabled = !game_mode_enabled;
    return game_mode_enabled;
}

bool game_mode_is_active(void) {
    return game_mode_enabled;
}

void game_mode_apply_lighting(uint8_t led_min, uint8_t led_max) {
    if (!game_mode_enabled) {
        return;
    }
    
    // Optimized: Only process WSAD LEDs within the given range
    for (uint8_t i = 0; i < 4; i++) {
        const uint8_t led_index = wsad_leds[i];
        
        // Range check before setting color (avoids unnecessary function calls)
        if (led_index >= led_min && led_index < led_max) {
            rgb_matrix_set_color(led_index, GAME_MODE_RED);
        }
    }
}

void game_mode_clear_lighting(uint8_t led_min, uint8_t led_max) {
    if (!game_mode_enabled) {
        return;
    }
    
    // Clear WSAD LEDs within the given range
    for (uint8_t i = 0; i < 4; i++) {
        const uint8_t led_index = wsad_leds[i];
        
        if (led_index >= led_min && led_index < led_max) {
            rgb_matrix_set_color(led_index, 0, 0, 0);
        }
    }
}
