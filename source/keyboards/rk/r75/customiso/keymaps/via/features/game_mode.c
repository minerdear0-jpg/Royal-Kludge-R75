#include "game_mode.h"
#include "rgb_matrix.h"

// Массив LED индексов для WSAD
const uint8_t game_mode_leds[] = {
    GAME_MODE_LED_W,  // W
    GAME_MODE_LED_S,  // S
    GAME_MODE_LED_A,  // A
    GAME_MODE_LED_D   // D
};

const uint8_t game_mode_led_count = sizeof(game_mode_leds) / sizeof(game_mode_leds[0]);

// Состояние Game Mode
bool game_mode_active = false;

void game_mode_enable(void) {
    game_mode_active = true;
}

void game_mode_disable(void) {
    game_mode_active = false;
    // Очистить подсветку WSAD при отключении
    for (uint8_t i = 0; i < game_mode_led_count; i++) {
        RGB_MATRIX_INDICATOR_SET_COLOR(game_mode_leds[i], 0, 0, 0);
    }
}

void game_mode_toggle(void) {
    if (game_mode_active) {
        game_mode_disable();
    } else {
        game_mode_enable();
    }
}

void game_mode_update_leds(uint8_t led_min, uint8_t led_max) {
    if (!game_mode_active) {
        return;
    }

    // Подсветка WSAD красным цветом
    for (uint8_t i = 0; i < game_mode_led_count; i++) {
        uint8_t led_index = game_mode_leds[i];
        
        // Проверка диапазона для оптимизации
        if (led_index >= led_min && led_index < led_max) {
            rgb_matrix_set_color(led_index, 255, 0, 0);  // Красный цвет
        }
    }
}

bool process_game_mode_keycode(uint16_t keycode, keyrecord_t *record) {
    // Обработка активации Game Mode через сочетание клавиш
    // Fn + R_Shift + G будет обрабатываться в keymap.c через слой
    return true;
}
