#include "tap_hold.h"
#include "quantum.h"

void safe_reset(tap_dance_state_t *state, void *user_data) {
    if (state->count >= 3) {
        reset_keyboard();
        reset_tap_dance(state);
    }
}

void safe_clear(tap_dance_state_t *state, void *user_data) {
    if (state->count >= 3) {
        eeconfig_init();
        soft_reset_keyboard();
        reset_tap_dance(state);
    }
}
