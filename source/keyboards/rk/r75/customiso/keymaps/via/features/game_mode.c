#include "game_mode.h"
#include "rgb_matrix.h"

// Внутреннее состояние - инкапсулировано, нет глобального extern
static bool game_mode_active = false;

// Получение состояния (inline для производительности)
bool game_mode_is_active(void) {
    return game_mode_active;
}

// Прямая работа с буфером RGB для максимальной производительности
// Избегаем накладных расходов rgb_matrix_set_color() в цикле
static void game_mode_set_leds_raw(uint8_t r, uint8_t g, uint8_t b) {
    // Проверка на инициализированный RGB матрицы
    if (!rgb_matrix_is_enabled()) {
        return;
    }
    
    // Прямая запись в буфер - минимальные накладные расходы
    for (uint8_t i = 0; i < GAME_MODE_LED_COUNT; i++) {
        uint8_t led_index = game_mode_leds[i];
        
        // Проверка диапазона перед записью (защита от выхода за границы)
        if (led_index < RGB_MATRIX_LED_COUNT) {
            rgb_matrix_led_buffer[led_index] = (rgb_led_t){r, g, b};
        }
    }
    
    // Принудительное обновление только измененных LED
    rgb_matrix_set_color_all(r, g, b);
}

// Включение Game Mode - событийная модель
void game_mode_enable(void) {
    if (game_mode_active) {
        return;  // Уже включено, избегаем лишней работы
    }
    
    game_mode_active = true;
    game_mode_apply();  // Применить изменения один раз
}

// Выключение Game Mode - событийная модель
void game_mode_disable(void) {
    if (!game_mode_active) {
        return;  // Уже выключено, избегаем лишней работы
    }
    
    game_mode_active = false;
    game_mode_apply();  // Очистить изменения один раз
}

// Переключение режима с визуальной обратной связью
void game_mode_toggle(void) {
    if (game_mode_active) {
        game_mode_disable();
    } else {
        game_mode_enable();
    }
}

// Применение текущего состояния (вызывать ТОЛЬКО при изменении состояния)
// Эта функция не должна вызываться в matrix_scan_user или rgb_matrix_indicators_advanced_user
void game_mode_apply(void) {
    if (game_mode_active) {
        // Включить красный цвет для WSAD
        game_mode_set_leds_raw(255, 0, 0);
    } else {
        // Очистить подсветку WSAD (черный цвет)
        // Примечание: фактическая очистка происходит через систему индикаторов
        // Здесь мы просто сбрасываем наш флаг
        game_mode_set_leds_raw(0, 0, 0);
    }
}

// Обработка нажатий клавиш Game Mode
bool process_game_mode_keycode(uint16_t keycode, keyrecord_t *record) {
    // Обрабатываем только нажатие (не отпускание) для экономии CPU
    if (!record->event.pressed) {
        return true;
    }
    
    switch (keycode) {
        case GAME_MODE_TOG:
            game_mode_toggle();
            return false;  // Клавиша обработана, не передавать дальше
        
        default:
            return true;  // Передать обработку другим модулям
    }
}
