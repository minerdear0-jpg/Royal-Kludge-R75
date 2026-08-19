#include "color.h"
#include "quantum.h"
#include "rgb_matrix.h"
#include "action_layer.h"

#include "indicators.h"
#include "defines.h"
#include "indicator_queue.h"
#include "game_mode.h"

// clang-format off

/*  LED Matrix
    ESC  F1    F2    F3    F4    F5    F6    F7    F8    F9    F10    F11    F12    DEL     ENCODER
    21   20    19    18    17    16    15    14    13    12    11     10     9      8
    
    `     1    2     3     4     5     6     7     8     9     0      -      =      BKSP    Home
    22    23   24    25    26    27    28    29    30    31    32     33     34     35      7
    
    Tab   Q    W     E     R     T     Y     U     I     O     P      [       ]      \      PGUP
    49    48   47    46    45    44    43    42    41    40    39     38      37     36     6
    
    CAPS  A    S     D     F     G     H     J     K     L     ;        '           ENTER   PGDN      
    50    51   52    53    54    55    56    57    58    59    60       61          62      5      
    
    SHIFT Z    X     C     V     B     N     M     ,     .     /        RSHIFT      UP         
    75    74   73    72    71    70    69    68    67    66    65       64          63    
    
    LCTRL    WIN    ALT            SPACE                 ALT   FN       LEFT        DOWN    RIGHT        
    76       77     78               79                  0     1        2           3       4
*/
// clang-format on


/*  LED Matrix ISO
    ESC  F1    F2    F3    F4    F5    F6    F7    F8    F9    F10    F11    F12    DEL     ENCODER
    21   20    19    18    17    16    15    14    13    12    11     10     9      8
    
    `     1    2     3     4     5     6     7     8     9     0      -      =      BKSP    Home
    22    23   24    25    26    27    28    29    30    31    32     33     34     35      7
    
    Tab   Q    W     E     R     T     Y     U     I     O     P      [       ]     ENTER   PGUP
    49    48   47    46    45    44    43    42    41    40    39     38      37     36     6
    
    CAPS  A    S     D     F     G     H     J     K     L     ;        '           \      PGDN      
    50    51   52    53    54    55    56    57    58    59    60       61          62      5      
    
    SHIFT  <>      Z    X     C     V    B    N     M     ,     .     /   RSHIFT   UP         
    76     75    74   73    72    71    70    69    68    67    66    65     64      63    
    
    LCTRL    WIN    ALT            SPACE                 ALT   FN       LEFT        DOWN    RIGHT        
    77       78     79               80                  0     1        2           3       4
*/
// clang-format on

// Именованные константы для LED (оптимизация: избегаем магических чисел)
static const uint8_t LED_F_KEYS[] = {20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8};
static const uint8_t LED_FN_ROW[] = {41, 40, 39, 38, 37, 36, 58, 59, 60, 67, 66, 2, 3, 4, 63};
static const uint8_t LED_NUMPAD[] = {29, 30, 31, 32, 42, 41, 40, 39, 57, 58, 59, 60, 68, 67, 66, 65};
static const uint8_t LED_NKRO_ON[] = {71, 70, 69, 68, 67, 55, 56, 57, 58, 43, 42, 41};
static const uint8_t LED_NKRO_OFF[] = {70, 68, 56, 57};

// Оптимизированная функция очистки диапазона LED
static inline void clear_led_range(uint8_t led_min, uint8_t led_max) {
    for (uint8_t i = led_min; i <= led_max; i++) {
        rgb_matrix_set_color(i, 0, 0, 0);
    }
}

// Оптимизированная функция установки цвета для массива LED
static inline void set_led_array(const uint8_t* leds, uint8_t count, uint8_t r, uint8_t g, uint8_t b) {
    for (uint8_t i = 0; i < count; i++) {
        rgb_matrix_set_color(leds[i], r, g, b);
    }
}

/******************
 * RGB Indicators *
 ******************/
void blink_numbers(bool isEnabling) {
    if (isEnabling) {
        for (uint8_t i = 23; i <= 34; i++) {
            indicator_enqueue(i, 200, 3, RGB_WHITE);
        }
    } else {
        for (uint8_t i = 23; i <= 34; i++) {
            indicator_enqueue(i, 150, 4, RGB_RED);
        }
    }
}

void blink_arrows(void) {
    indicator_enqueue(2, 200, 3, RGB_WHITE);
    indicator_enqueue(3, 200, 3, RGB_WHITE);
    indicator_enqueue(63, 200, 3, RGB_WHITE);
    indicator_enqueue(4, 200, 3, RGB_WHITE);
}

void blink_space(bool extended) {
    indicator_enqueue(79, 200, 3, RGB_WHITE);
    if (extended) {
        indicator_enqueue(78, 200, 3, RGB_BLACK);
        indicator_enqueue(0, 200, 3, RGB_BLACK);
    }
}

void blink_NKRO(bool isEnabling) {
    if (isEnabling) {
        for (uint8_t i = 0; i < 12; i++) {
            indicator_enqueue(LED_NKRO_ON[i], 200, 3, RGB_WHITE);
        }
    } else {
        for (uint8_t i = 0; i < 4; i++) {
            indicator_enqueue(LED_NKRO_OFF[i], 150, 3, RGB_RED);
        }
    }
}

void highlight_fn_keys(uint8_t led_min, uint8_t led_max) {
    HSV current_hsv = rgb_matrix_get_hsv();
    current_hsv.v = 255;

    rgb_led_t rgb = hsv_to_rgb(current_hsv);
    rgb_led_t new_rgb = get_complementary_color(rgb, false);
    
    for (uint8_t i = led_min; i < led_max; i++) {
        rgb_matrix_set_color(i, new_rgb.r, new_rgb.g, new_rgb.b);
    }
}

// Оптимизированная функция индикаторов RGB
// Вызывается на каждом цикле сканирования матрицы - критично для производительности
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    // Быстрое получение текущего слоя
    const uint8_t current_layer = get_highest_layer(layer_state);
    
    // Слой 0 (Base): только обработка индикаторов
    if (current_layer == 0) {
        if (rgb_matrix_get_flags() == LED_FLAG_INDICATOR) {
            clear_led_range(led_min, led_max);
        }
        // Game Mode: применяем подсветку WSAD ТОЛЬКО если активен
        // Это событийная модель - game_mode_apply вызывается только при изменении состояния
        if (game_mode_is_active()) {
            game_mode_apply();
        }
        process_indicator_queue(led_min, led_max);
        return true;
    }
    
    // Слои 1, 2, 4: очистка фона и установка индикаторов
    if (IS_LAYER_ON(1) || IS_LAYER_ON(2) || IS_LAYER_ON(4)) {
        clear_led_range(led_min, led_max);
    }
    
    // Слой 1 (Fn Layer)
    if (IS_LAYER_ON(1)) {
        set_led_array(LED_F_KEYS, 13, 0x00, 0x80, 0x80);
        set_led_array(LED_FN_ROW, 15, 0, 0, 255);
        rgb_matrix_set_color(64, 0xFF, 0x00, 0x00);
    }
    
    // Слой 2 (Options Layer)
    if (IS_LAYER_ON(2)) {
        rgb_matrix_set_color(69, 0xFF, 0x00, 0x00);  // NKRO
        rgb_matrix_set_color(44, 0xFF, 0xA5, 0x00); // SnapTap
        rgb_matrix_set_color(48, 0xFF, 0xFF, 0x00); // Reset
        rgb_matrix_set_color(74, 0x7A, 0x00, 0xFF); // Clear EEPROM
        rgb_matrix_set_color(52, 0x00, 0xFF, 0x00); // SignalRGB
        rgb_matrix_set_color(40, 0x00, 0xFF, 0xFF); // OpenRGB
    }
    
    // Слой 4 (Numpad Layer)
    if (IS_LAYER_ON(4)) {
        rgb_matrix_set_color(28, 0xFF, 0x00, 0x00);
        set_led_array(LED_NUMPAD, 16, 0x00, 0xFF, 0x00);
        rgb_matrix_set_color(20, 0x7A, 0x00, 0xFF);
    }
    
    // Обработка очереди индикаторов
    process_indicator_queue(led_min, led_max);
    
    return true;
}
