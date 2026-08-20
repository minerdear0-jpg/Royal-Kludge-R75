#include "tap_hold.h"
#include "quantum.h"
#include "lighting_profile.h"

void safe_reset(tap_dance_state_t *state, void *user_data) {
    if (state->count >= 3) {
        lighting_profile_enter_bootloader();
        reset_tap_dance(state);
    }
}

void safe_clear(tap_dance_state_t *state, void *user_data) {
    if (state->count >= 3) {
        lighting_profile_wipe_then_reset();
        reset_tap_dance(state);
    }
}
