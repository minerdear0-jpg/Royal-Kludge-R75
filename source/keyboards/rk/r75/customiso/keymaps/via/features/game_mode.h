#pragma once

#include QMK_KEYBOARD_H

// LED индексы для WSAD
#define GAME_MODE_LED_W 47
#define GAME_MODE_LED_S 52
#define GAME_MODE_LED_A 51
#define GAME_MODE_LED_D 53

// Массив LED для Game Mode
extern const uint8_t game_mode_leds[];
extern const uint8_t game_mode_led_count;

// Состояние Game Mode
extern bool game_mode_active;

// Функции управления Game Mode
void game_mode_enable(void);
void game_mode_disable(void);
void game_mode_toggle(void);
void game_mode_update_leds(uint8_t led_min, uint8_t led_max);
bool process_game_mode_keycode(uint16_t keycode, keyrecord_t *record);
