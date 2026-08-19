#pragma once

#include QMK_KEYBOARD_H
#include "action.h"

#if defined(QMK_KEYCODES_VERSION_BCD) && (QMK_KEYCODES_VERSION_BCD >= 0x00000008)
#    define RGB_TOG RM_TOGG
#    define RGB_MOD RM_NEXT
#    define RGB_RMOD RM_PREV
#    define RGB_HUI RM_HUEU
#    define RGB_HUD RM_HUED
#    define RGB_SAI RM_SATU
#    define RGB_SAD RM_SATD
#    define RGB_VAI RM_VALU
#    define RGB_VAD RM_VALD
#    define RGB_SPI RM_SPDU
#    define RGB_SPD RM_SPDD
#endif

bool process_rgb_keys(uint16_t keycode, keyrecord_t *record);
