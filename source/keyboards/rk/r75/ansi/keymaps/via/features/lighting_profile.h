#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "quantum.h"

void lighting_profile_init(void);
void lighting_profile_cycle(void);
void lighting_profile_toggle_gaming(void);
void lighting_profile_task(void);
void lighting_profile_paint(uint8_t led_min, uint8_t led_max);
void lighting_profile_note_activity(void);
void lighting_profile_host_off(void);
void lighting_profile_host_on(void);
bool lighting_profile_is_gaming(void);
bool lighting_profile_process(uint16_t keycode, keyrecord_t *record);
