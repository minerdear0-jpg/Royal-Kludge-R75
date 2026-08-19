#pragma once

enum layer_names {
    _WIN_LYR,    // 0
    _WIN_FN_LYR, // 1
    _CTL_LYR,    // 2
    _MAC_LYR,    // 3
    _NUM_LYR,    // 4
};

#define TD_KB_RST TD(TD_RESET)
#define TD_KB_CLR TD(TD_CLEAR)

/* ISO LED indices */
#define LED_RALT 0
#define LED_FN 1
#define LED_LEFT 2
#define LED_DOWN 3
#define LED_RIGHT 4
#define LED_SPACE 80
#define LED_LALT 79
#define LED_QUOTE 61
#define LED_SCLN 60
#define LED_O 40
#define LED_I 41
#define LED_L 59
#define LED_K 58
#define LED_DOT 66
#define LED_COMM 67
#define LED_G 55
#define LED_LSFT 76
#define LED_LCTL 77
#define LED_LGUI 78
