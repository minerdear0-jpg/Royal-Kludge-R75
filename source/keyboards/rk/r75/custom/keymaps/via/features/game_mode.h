#pragma once

#include QMK_KEYBOARD_H

#define GAME_MODE_LED_W 47
#define GAME_MODE_LED_S 52
#define GAME_MODE_LED_A 51
#define GAME_MODE_LED_D 53
#define GAME_MODE_RED 0xFF, 0x00, 0x00

void game_mode_init(void);
bool game_mode_toggle(void);
bool game_mode_is_active(void);
void game_mode_apply_lighting(uint8_t led_min, uint8_t led_max);
