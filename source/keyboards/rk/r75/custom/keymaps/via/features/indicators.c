#include "color.h"
#include "quantum.h"
#include "rgb_matrix.h"
#include "action_layer.h"

#include "indicators.h"
#include "defines.h"
#include "indicator_queue.h"
#include "game_mode.h"

// clang-format off
/*  LED Matrix ANSI
    ESC  F1   F2   F3   F4   F5   F6   F7   F8   F9   F10  F11  F12  DEL   ENC
    21   20   19   18   17   16   15   14   13   12   11   10   9    8

    `    1    2    3    4    5    6    7    8    9    0    -    =    BKSP Home
    22   23   24   25   26   27   28   29   30   31   32   33   34   35   7

    Tab  Q    W    E    R    T    Y    U    I    O    P    [    ]    \    PGUP
    49   48   47   46   45   44   43   42   41   40   39   38   37   36   6

    CAPS A    S    D    F    G    H    J    K    L    ;    '         ENT  PGDN
    50   51   52   53   54   55   56   57   58   59   60   61        62   5

    SHIFT Z   X    C    V    B    N    M    ,    .    /    RSHIFT    UP
    75    74  73   72   71   70   69   68   67   66   65   64        63

    LCTL WIN  ALT           SPACE              RALT FN  LEFT DOWN RIGHT
    76   77   78            79                 0    1   2    3    4
*/
// clang-format on

void blink_numbers(bool isEnabling) {
    for (uint8_t i = 23; i <= 34; i++) {
        if (isEnabling) {
            indicator_enqueue(i, 200, 3, RGB_WHITE);
        } else {
            indicator_enqueue(i, 150, 4, RGB_RED);
        }
    }
}

void blink_arrows(void) {
    indicator_enqueue(LED_LEFT, 200, 3, RGB_WHITE);
    indicator_enqueue(LED_DOWN, 200, 3, RGB_WHITE);
    indicator_enqueue(63, 200, 3, RGB_WHITE);
    indicator_enqueue(LED_RIGHT, 200, 3, RGB_WHITE);
}

void blink_space(bool extended) {
    indicator_enqueue(LED_SPACE, 200, 3, RGB_WHITE);
    if (extended) {
        indicator_enqueue(LED_LALT, 200, 3, RGB_BLACK);
        indicator_enqueue(LED_RALT, 200, 3, RGB_BLACK);
    }
}

void blink_NKRO(bool isEnabling) {
    if (isEnabling) {
        const uint8_t led_indexes[12] = {71, 70, 69, 68, 67, 55, 56, 57, 58, 43, 42, 41};
        for (uint8_t i = 0; i < 12; i++) {
            indicator_enqueue(led_indexes[i], 200, 3, RGB_WHITE);
        }
    } else {
        const uint8_t led_indexes[4] = {70, 68, 56, 57};
        for (uint8_t i = 0; i < 4; i++) {
            indicator_enqueue(led_indexes[i], 150, 3, RGB_RED);
        }
    }
}

void highlight_fn_keys(uint8_t led_min, uint8_t led_max) {
    HSV current_hsv = rgb_matrix_get_hsv();
    current_hsv.v   = 255;

    rgb_led_t rgb     = hsv_to_rgb(current_hsv);
    rgb_led_t new_rgb = get_complementary_color(rgb, false);
    for (uint8_t i = 23; i <= 34; i++) {
        RGB_MATRIX_INDICATOR_SET_COLOR(i, new_rgb.r, new_rgb.g, new_rgb.b);
    }
}

static void fn_hint(uint8_t led_min, uint8_t led_max, const uint8_t *ids, uint8_t n, uint8_t r, uint8_t g, uint8_t b) {
    for (uint8_t i = 0; i < n; i++) {
        RGB_MATRIX_INDICATOR_SET_COLOR(ids[i], r, g, b);
    }
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    const uint8_t current_layer = get_highest_layer(layer_state);

    if (game_mode_is_active() && current_layer == _WIN_LYR) {
        process_indicator_queue(led_min, led_max);
        game_mode_apply_lighting(led_min, led_max);
        return true;
    }

    if (current_layer == _WIN_LYR) {
        process_indicator_queue(led_min, led_max);
        return true;
    }

    if (IS_LAYER_ON(_CTL_LYR) || IS_LAYER_ON(_NUM_LYR)) {
        for (uint8_t i = led_min; i <= led_max; i++) {
            RGB_MATRIX_INDICATOR_SET_COLOR(i, 0, 0, 0);
        }
    }

    if (IS_LAYER_ON(_WIN_FN_LYR)) {
        const uint8_t media[] = {20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8};
        fn_hint(led_min, led_max, media, 13, 0x00, 0x80, 0x80);

        const uint8_t hue[] = {67, 66}; /* , . */
        fn_hint(led_min, led_max, hue, 2, 0xFF, 0x40, 0xFF);

        const uint8_t bright[] = {63, 3};
        fn_hint(led_min, led_max, bright, 2, 0xFF, 0xFF, 0xFF);

        const uint8_t speed[] = {2, 4};
        fn_hint(led_min, led_max, speed, 2, 0x00, 0x80, 0xFF);

        const uint8_t mode[] = {38, 37, 36}; /* [ ] \ */
        fn_hint(led_min, led_max, mode, 3, 0x00, 0xFF, 0x80);

        RGB_MATRIX_INDICATOR_SET_COLOR(39, 0xFF, 0xA5, 0x00);
        RGB_MATRIX_INDICATOR_SET_COLOR(LED_G, 0xFF, 0x00, 0x00);
        RGB_MATRIX_INDICATOR_SET_COLOR(64, 0xFF, 0x00, 0x00);
        RGB_MATRIX_INDICATOR_SET_COLOR(LED_SPACE, 0x7A, 0x00, 0xFF);
        RGB_MATRIX_INDICATOR_SET_COLOR(22, 0xFF, 0xE0, 0x80);
    }

    if (IS_LAYER_ON(_CTL_LYR)) {
        RGB_MATRIX_INDICATOR_SET_COLOR(69, 0xFF, 0x00, 0x00);
        RGB_MATRIX_INDICATOR_SET_COLOR(44, 0xFF, 0xA5, 0x00);
        RGB_MATRIX_INDICATOR_SET_COLOR(48, 0xFF, 0xFF, 0x00);
        RGB_MATRIX_INDICATOR_SET_COLOR(74, 0x7A, 0x00, 0xFF);
        RGB_MATRIX_INDICATOR_SET_COLOR(65, 0xFF, 0x00, 0x00);
    }

    if (IS_LAYER_ON(_NUM_LYR)) {
        RGB_MATRIX_INDICATOR_SET_COLOR(28, 0xFF, 0x00, 0x00);
        const uint8_t numpad[16] = {29, 30, 31, 32, 42, 41, 40, 39, 57, 58, 59, 60, 68, 67, 66, 65};
        for (uint8_t i = 0; i < 16; i++) {
            RGB_MATRIX_INDICATOR_SET_COLOR(numpad[i], 0x00, 0xFF, 0x00);
        }
        RGB_MATRIX_INDICATOR_SET_COLOR(20, 0x7A, 0x00, 0xFF);
    }

    process_indicator_queue(led_min, led_max);
    game_mode_apply_lighting(led_min, led_max);
    return true;
}
