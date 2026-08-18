#pragma once

#include QMK_KEYBOARD_H

// LED индексы для WSAD (оптимизировано для прямого доступа)
#define GAME_MODE_LED_W 47
#define GAME_MODE_LED_S 52
#define GAME_MODE_LED_A 51
#define GAME_MODE_LED_D 53

// Статический массив LED для Game Mode - без накладных расходов на extern
static const uint8_t game_mode_leds[] = {GAME_MODE_LED_W, GAME_MODE_LED_S, GAME_MODE_LED_A, GAME_MODE_LED_D};
#define GAME_MODE_LED_COUNT 4

// Состояние Game Mode - только внутреннее использование
bool game_mode_is_active(void);

// Функции управления Game Mode (событийная модель - только по изменению состояния)
void game_mode_enable(void);      // Включить режим
void game_mode_disable(void);     // Выключить режим
void game_mode_toggle(void);      // Переключить режим
void game_mode_apply(void);       // Применить подсветку (вызывать только при изменении состояния)
bool process_game_mode_keycode(uint16_t keycode, keyrecord_t *record);  // Обработка нажатий
