#pragma once
#include "process_tap_dance.h"
#include QMK_KEYBOARD_H

void safe_reset(tap_dance_state_t *state, void *user_data);
void safe_clear(tap_dance_state_t *state, void *user_data);
