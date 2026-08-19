// Copyright 2024 SDK (@sdk66)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "quantum.h"

#include "features/defines.h"
#include "features/custom_keycodes.h"
#include "features/indicator_queue.h"
#include "features/tap_hold.h"
#include "features/indicators.h"
#include "features/rgb_keys.h"
#include "features/socd_cleaner.h"
#include "features/game_mode.h"
#include "features/lighting_profile.h"

void keyboard_post_init_user(void) {
    game_mode_init();
    lighting_profile_init();
}

void suspend_power_down_user(void) {
    lighting_profile_host_off();
}

void suspend_wakeup_init_user(void) {
    lighting_profile_host_on();
}

void housekeeping_task_user(void) {
    lighting_profile_task();
    /* Hardware LEDs are active-low. Mac = macOS modifier layer only.
     * Win-lock = GUI/Super blocked (Game Mode). Fn/numpad must not steal these. */
    const bool mac_led = IS_LAYER_ON(_MAC_LYR);
    const bool win_lock_led = keymap_config.no_gui;

    gpio_write_pin(LED_MAC_PIN, !mac_led);
    gpio_write_pin(LED_WIN_LOCK_PIN, !win_lock_led);
}

socd_cleaner_t socd_v = {{KC_W, KC_S}, SOCD_CLEANER_LAST};
socd_cleaner_t socd_h = {{KC_A, KC_D}, SOCD_CLEANER_LAST};

enum tap_dance_keys {
    TD_RESET,
    TD_CLEAR,
    TD_CTL_TG
};

// clang-format off
tap_dance_action_t tap_dance_actions[] = {
    [TD_RESET]  = ACTION_TAP_DANCE_FN(safe_reset),
    [TD_CLEAR]  = ACTION_TAP_DANCE_FN(safe_clear),
    [TD_CTL_TG] = ACTION_TAP_DANCE_LAYER_TOGGLE(KC_RCTL, _CTL_LYR)
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_WIN_LYR] = LAYOUT(
        KC_ESC,   KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   KC_DEL,   KC_MUTE,
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,  KC_HOME,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,  KC_PGUP,
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_ENT,   KC_PGDN,
        KC_LSFT,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_RSFT,            KC_UP,
        KC_LCTL,  KC_LCMD,  KC_LALT,                      KC_SPC,                                 KC_RALT,  MO(_WIN_FN_LYR),   KC_LEFT,  KC_DOWN,  KC_RGHT
        ),

    [_WIN_FN_LYR] = LAYOUT(
        _______,  KC_MYCM,  KC_WHOM,  KC_MAIL,  KC_CALC,  KC_MSEL,  KC_MSTP,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,   KC_SCRL,  KC_PAUSE,
        LIGHT_PROFILE_CYC, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,   _______,  _______,  _______,  _______,  KC_PSCR,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  RGB_M_P,  RM_PREV,  RM_NEXT,  RM_NEXT,  KC_INS,
        _______,  _______,  _______,  _______,  _______,  GAME_MODE_TOG,  _______,  _______,  _______,  _______,  _______,  _______,             _______,  KC_END,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  RM_HUED,  RM_HUEU,  _______,   MO(_CTL_LYR),              RM_VALU,
        _______,  _______,  _______,                      TD_KB_CLR,                              _______,  _______,              RM_SPDD, RM_VALD,  RM_SPDU
        ),

    [_CTL_LYR] = LAYOUT(
        _______,  TO(_WIN_LYR), TO(_MAC_LYR), TO(_NUM_LYR), _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______,  TD_KB_RST,  _______,  _______,  _______,  SOCDTOG,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,         _______,  _______,
        _______,  TD_KB_CLR,  _______,  _______,  _______,  _______,  NK_TOGG,  _______,  _______,  _______,  GAME_MODE_TOG,  _______,           _______,
        _______,  _______,  _______,                      _______,                                 _______,  _______,            _______,  _______,  _______
        ),

    [_MAC_LYR] = LAYOUT(
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,  _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,  KC_LALT,  KC_LGUI,                      _______,                                 KC_RGUI,  _______,            _______,  _______,  _______
        ),

    [_NUM_LYR] = LAYOUT(
        _______,  TO(_WIN_LYR), _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______,  _______,  _______,  _______,  _______,  _______,  KC_NUM,   KC_P7,  KC_P8,  KC_P9,  KC_PAST,  _______,  _______,  _______,  _______,
        _______,  _______,  _______,  _______,  _______,  _______,  XXXXXXX,  KC_P4,  KC_P5,  KC_P6,  KC_PMNS,  _______,  _______,  _______,  _______,
        _______,  _______,  _______,  _______,  _______,  _______,  XXXXXXX,  KC_P1,  KC_P2,  KC_P3,  KC_PPLS,  _______,            _______,  _______,
        _______,  _______,  _______,  _______,  _______,  _______,  XXXXXXX,  KC_P0,  KC_PDOT,  KC_PSLS,  KC_PENT,  _______,            _______,
        _______,  _______,  _______,                      _______,                                 _______,  _______,            _______,  _______,  _______
        ),
};

#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [_WIN_LYR]    = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [_WIN_FN_LYR] = {ENCODER_CCW_CW(KC_BRID, KC_BRIU)},
    [_CTL_LYR]    = {ENCODER_CCW_CW(KC_MRWD, KC_MFFD)},
    [_MAC_LYR]    = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [_NUM_LYR]    = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
};
#endif
// clang-format on

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        lighting_profile_note_activity();
    }
    if (!process_socd_cleaner(keycode, record, &socd_v)) {
        return false;
    }
    if (!process_socd_cleaner(keycode, record, &socd_h)) {
        return false;
    }
    if (!process_rgb_keys(keycode, record)) {
        return false;
    }

    switch (keycode) {
        case GAME_MODE_TOG:
            if (record->event.pressed) {
                const bool enabled = game_mode_toggle();
                if (enabled) {
                    indicator_enqueue(GAME_MODE_LED_W, 200, 3, RGB_RED);
                    indicator_enqueue(GAME_MODE_LED_S, 200, 3, RGB_RED);
                    indicator_enqueue(GAME_MODE_LED_A, 200, 3, RGB_RED);
                    indicator_enqueue(GAME_MODE_LED_D, 200, 3, RGB_RED);
                } else {
                    indicator_enqueue(GAME_MODE_LED_W, 150, 2, RGB_BLUE);
                    indicator_enqueue(GAME_MODE_LED_S, 150, 2, RGB_BLUE);
                    indicator_enqueue(GAME_MODE_LED_A, 150, 2, RGB_BLUE);
                    indicator_enqueue(GAME_MODE_LED_D, 150, 2, RGB_BLUE);
                }
            }
            return false;

        case LIGHT_PROFILE_CYC:
            if (record->event.pressed) {
                lighting_profile_cycle();
            }
            return false;

        case QK_MAGIC_TOGGLE_NKRO:
            if (record->event.pressed) {
                clear_keyboard();
                keymap_config.nkro = !keymap_config.nkro;
                blink_NKRO(keymap_config.nkro);
            }
            return false;

        case SOCDON:
            if (record->event.pressed) {
                socd_cleaner_enabled = true;
            }
            return false;

        case SOCDOFF:
            if (record->event.pressed) {
                socd_cleaner_enabled = false;
            }
            return false;

        case SOCDTOG:
            if (record->event.pressed) {
                socd_cleaner_enabled = !socd_cleaner_enabled;
            }
            return false;

        default:
            return true;
    }
}
