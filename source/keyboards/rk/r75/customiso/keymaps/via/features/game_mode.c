#include "game_mode.h"
#include "socd_cleaner.h"
#include "lighting_profile.h"
#include "rgb_matrix.h"
#include "defines.h"
#include "custom_keycodes.h"
#include "game_lighting.h"

static bool game_mode_enabled = false;
static bool saved_no_gui      = false;

static void paint(uint8_t led_min, uint8_t led_max, uint8_t idx, uint8_t r, uint8_t g, uint8_t b) {
    if (idx >= led_min && idx < led_max) {
        rgb_matrix_set_color(idx, r, g, b);
    }
}

static void paint_list(uint8_t led_min, uint8_t led_max, const uint8_t *ids, uint8_t n, uint8_t r, uint8_t g, uint8_t b) {
    for (uint8_t i = 0; i < n; i++) {
        paint(led_min, led_max, ids[i], r, g, b);
    }
}

static bool in_list(uint8_t idx, const uint8_t *ids, uint8_t n) {
    for (uint8_t i = 0; i < n; i++) {
        if (ids[i] == idx) {
            return true;
        }
    }
    return false;
}

static bool is_game_led(uint8_t idx) {
    return in_list(idx, gm_move, (uint8_t)(sizeof(gm_move) / sizeof(gm_move[0]))) ||
           in_list(idx, gm_nums, (uint8_t)(sizeof(gm_nums) / sizeof(gm_nums[0]))) ||
           in_list(idx, gm_mods, (uint8_t)(sizeof(gm_mods) / sizeof(gm_mods[0]))) ||
           in_list(idx, gm_tactic, (uint8_t)(sizeof(gm_tactic) / sizeof(gm_tactic[0])));
}

void game_mode_init(void) {
    game_mode_enabled    = false;
    socd_cleaner_enabled = false;
}

void game_mode_set_active(bool on) {
    if (on == game_mode_enabled) {
        return;
    }
    game_mode_enabled = on;
    if (on) {
        socd_cleaner_enabled = true;
        saved_no_gui         = keymap_config.no_gui;
        keymap_config.no_gui = true;
        if (!keymap_config.nkro) {
            clear_keyboard();
            keymap_config.nkro = true;
        }
    } else {
        socd_cleaner_enabled = false;
        keymap_config.no_gui = saved_no_gui;
    }
}

bool game_mode_toggle(void) {
    lighting_profile_toggle_gaming();
    return game_mode_is_active();
}

bool game_mode_is_active(void) {
    return game_mode_enabled;
}

bool game_mode_process(uint16_t keycode, keyrecord_t *record) {
    const bool hit = (keycode == GAME_MODE_TOG) ||
                     (IS_LAYER_ON(_WIN_FN_LYR) && record->event.key.row == GAME_MODE_ROW &&
                      record->event.key.col == GAME_MODE_COL);
    if (!hit) {
        return true;
    }
    if (record->event.pressed) {
        lighting_profile_toggle_gaming();
    }
    return false;
}

void game_mode_apply_lighting(uint8_t led_min, uint8_t led_max) {
    for (uint8_t i = led_min; i < led_max; i++) {
        rgb_matrix_set_color(i, 0, 0, 0);
    }

    paint_list(led_min, led_max, gm_move, (uint8_t)(sizeof(gm_move) / sizeof(gm_move[0])), lighting_profile_scale_u8(GM_MOVE_R), lighting_profile_scale_u8(GM_MOVE_G), lighting_profile_scale_u8(GM_MOVE_B));
    paint_list(led_min, led_max, gm_nums, (uint8_t)(sizeof(gm_nums) / sizeof(gm_nums[0])), lighting_profile_scale_u8(GM_NUM_R), lighting_profile_scale_u8(GM_NUM_G), lighting_profile_scale_u8(GM_NUM_B));
    paint_list(led_min, led_max, gm_mods, (uint8_t)(sizeof(gm_mods) / sizeof(gm_mods[0])), lighting_profile_scale_u8(GM_MOD_R), lighting_profile_scale_u8(GM_MOD_G), lighting_profile_scale_u8(GM_MOD_B));
    paint_list(led_min, led_max, gm_tactic, (uint8_t)(sizeof(gm_tactic) / sizeof(gm_tactic[0])), lighting_profile_scale_u8(GM_TAC_R), lighting_profile_scale_u8(GM_TAC_G), lighting_profile_scale_u8(GM_TAC_B));

#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
    for (uint8_t j = 0; j < g_last_hit_tracker.count; j++) {
        const uint16_t tick = g_last_hit_tracker.tick[j];
        const uint8_t  idx  = g_last_hit_tracker.index[j];
        if (tick >= GM_REACTIVE_MS || !is_game_led(idx)) {
            continue;
        }
        const uint8_t v = lighting_profile_scale_u8((uint8_t)(255 - (tick * 255 / GM_REACTIVE_MS)));
        paint(led_min, led_max, idx, v, v, v);
    }
#endif
}
