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

/******************
 * RGB Indicators *
 ******************/
void blink_numbers(bool isEnabling) {
    for (int i = 23; i >= 34; i--) // 1(55) to EQL(44)
    {
        if (isEnabling) {
            // enabling, flash white
            indicator_enqueue(i, 200, 3, RGB_WHITE);
        } else {
            // disabling, flash red
            indicator_enqueue(i, 150, 4, RGB_RED);
        }
    }
}

void blink_arrows(void) {
    indicator_enqueue(2, 200, 3, RGB_WHITE); // left
    indicator_enqueue(3, 200, 3, RGB_WHITE); // down
    indicator_enqueue(63, 200, 3, RGB_WHITE); // up
    indicator_enqueue(4, 200, 3, RGB_WHITE); // right
}

void blink_space(bool extended) {
    indicator_enqueue(79, 200, 3, RGB_WHITE); // blink space too
    if (extended) {
        indicator_enqueue(78, 200, 3, RGB_BLACK);  // blink left alt
        indicator_enqueue(0, 200, 3, RGB_BLACK); // blink right alt
    }
}

void blink_NKRO(bool isEnabling) {
    if (isEnabling) {
        const uint8_t led_indexes[12] = {
            71,70,69,68,67,  // V B N M ,
            55,56,57,58,     // K J H G
            43,42,41        // Y U I
        };

        for (int i = 0; i < 12; i++) {
            indicator_enqueue(led_indexes[i], 200, 3, RGB_WHITE);
        }
    } else {
        const uint8_t led_indexes[4] = {
            70, 68, // B M
            56, 57 // H J
        };

        for (int i = 0; i < 4; i++) {
            indicator_enqueue(led_indexes[i], 150, 3, RGB_RED);
        }
    }
}

void highlight_fn_keys(uint8_t led_min, uint8_t led_max) {
    // get the current hsv value
    HSV current_hsv = rgb_matrix_get_hsv();
    // maximize brightness
    current_hsv.v = 255;

    rgb_led_t rgb     = hsv_to_rgb(current_hsv);
    rgb_led_t new_rgb = get_complementary_color(rgb, false);
    for (int i = 23; i >= 34; i--) { // 55 - 44 are the number keys and - =
        RGB_MATRIX_INDICATOR_SET_COLOR(i, new_rgb.r, new_rgb.g, new_rgb.b);
    }
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    uint8_t current_layer = get_highest_layer(layer_state);
    
    // Apply Game Mode lighting first (highest priority when active)
    if (game_mode_is_active()) {
        game_mode_apply_lighting(led_min, led_max);
    }
    
    if (current_layer == 0) {
        if (rgb_matrix_get_flags() == LED_FLAG_INDICATOR) {
            for (int i = led_min; i < led_max; i++) {
                rgb_matrix_set_color(i, 0, 0, 0);
            }
        }
    }

    if (IS_LAYER_ON(1) ||
        IS_LAYER_ON(2) ||
        IS_LAYER_ON(4) ) {
        for (int i = led_min; i <= led_max; i++) {
            RGB_MATRIX_INDICATOR_SET_COLOR(i, 0, 0, 0);
        }
    }

    if (IS_LAYER_ON(1)) {
        const uint8_t fn_leds[13] = {20,19,18,17,16,15,14,13,12,11,10,9,8};
        for (uint8_t i = 0; i < 13; i++) {
            RGB_MATRIX_INDICATOR_SET_COLOR(fn_leds[i], 0x00, 0x80, 0x80);
        }        
        
        const uint8_t rgb_leds[15] = {41,40,39,38,37,36,58,59,60,67,66,2,3,4,63};
        for (uint8_t i = 0; i < 15; i++) {
            RGB_MATRIX_INDICATOR_SET_COLOR(rgb_leds[i], 0, 0, 255);
        }

        RGB_MATRIX_INDICATOR_SET_COLOR(64, 0xFF, 0x00, 0x00);
    }

    if (IS_LAYER_ON(2)) {
        const uint8_t nkro_led = 69;
        const uint8_t snaptap_led = 44;
        const uint8_t reset_led = 48;
        const uint8_t clear_led = 74;
        const uint8_t signal_led = 52;
        const uint8_t openrgb_led = 40;
        
        RGB_MATRIX_INDICATOR_SET_COLOR(nkro_led, 0xFF, 0x00, 0x00);
        RGB_MATRIX_INDICATOR_SET_COLOR(snaptap_led, 0xFF, 0xA5, 0x00);
        RGB_MATRIX_INDICATOR_SET_COLOR(reset_led, 0xFF, 0xFF, 0x00);
        RGB_MATRIX_INDICATOR_SET_COLOR(clear_led, 0x7A, 0x00, 0xFF);
        RGB_MATRIX_INDICATOR_SET_COLOR(signal_led, 0x00, 0xFF, 0x00);
        RGB_MATRIX_INDICATOR_SET_COLOR(openrgb_led, 0x00, 0xFF, 0xFF);
    }

    if (IS_LAYER_ON(4)) {
        const uint8_t nav_led = 28;
        const uint8_t numpad_leds[16] = {29,30,31,32,42,41,40,39,57,58,59,60,68,67,66,65};
        const uint8_t f1_led = 20;
        
        RGB_MATRIX_INDICATOR_SET_COLOR(nav_led, 0xFF, 0x00, 0x00);
        for (uint8_t i = 0; i < 16; i++) {
            RGB_MATRIX_INDICATOR_SET_COLOR(numpad_leds[i], 0x00, 0xFF, 0x00);
        }
        RGB_MATRIX_INDICATOR_SET_COLOR(f1_led, 0x7A, 0x00, 0xFF);
    }

    process_indicator_queue(led_min, led_max);

    return true;
}
