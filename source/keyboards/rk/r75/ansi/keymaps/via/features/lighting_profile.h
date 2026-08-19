#pragma once

#include <stdbool.h>

void lighting_profile_init(void);
void lighting_profile_cycle(void);
void lighting_profile_task(void);
void lighting_profile_note_activity(void);
void lighting_profile_game_enter(void);
void lighting_profile_game_exit(void);
void lighting_profile_on_game_exit(void);
void lighting_profile_host_off(void);
void lighting_profile_host_on(void);
bool lighting_profile_user_selected(void);
