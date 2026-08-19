#include "rgb_keys.h"
#include "rgb_matrix.h"

#include "defines.h"
#include "indicators.h"

bool process_rgb_keys(uint16_t keycode, keyrecord_t *record) {
    /* RM_* (hue/val/mode) go to QMK process_rgb_matrix. RGB_M_P is leftover
     * rgblight id — map it to matrix solid color. */
    if (keycode == RGB_M_P && record->event.pressed) {
        rgb_matrix_mode(RGB_MATRIX_SOLID_COLOR);
        blink_space(true);
        return false;
    }
    return true;
}
