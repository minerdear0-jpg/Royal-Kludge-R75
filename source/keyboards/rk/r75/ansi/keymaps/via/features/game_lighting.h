#pragma once

#include "defines.h"
#include "game_mode.h"

/* Edit this file to retune Game lighting without touching fade/EEPROM logic.
 * Values are source RGB and brightness percent from the C-layer spec. */

#define GM_SCALE(c, pct) ((uint8_t)(((uint16_t)(c) * (pct)) / 100))

#define GM_MOVE_R GM_SCALE(215, 40)
#define GM_MOVE_G GM_SCALE(255, 40)
#define GM_MOVE_B GM_SCALE(0, 40)

#define GM_NUM_R GM_SCALE(170, 40)
#define GM_NUM_G GM_SCALE(0, 40)
#define GM_NUM_B GM_SCALE(255, 40)

#define GM_MOD_R GM_SCALE(0, 30)
#define GM_MOD_G GM_SCALE(200, 30)
#define GM_MOD_B GM_SCALE(200, 30)

#define GM_TAC_R GM_SCALE(0, 30)
#define GM_TAC_G GM_SCALE(180, 30)
#define GM_TAC_B GM_SCALE(255, 30)

#define GM_REACTIVE_MS 150

static const uint8_t gm_move[] = {
    GAME_MODE_LED_W, GAME_MODE_LED_A, GAME_MODE_LED_S, GAME_MODE_LED_D,
    LED_Q, LED_E, LED_F, LED_G,
    LED_LEFT, LED_DOWN, LED_UP, LED_RIGHT,
};

static const uint8_t gm_nums[] = {23, 24, 25, 26, 27, 28};

static const uint8_t gm_mods[] = {LED_LSFT, LED_RSFT, LED_LCTL, LED_SPACE};

static const uint8_t gm_tactic[] = {LED_Z, LED_X, LED_C, LED_V};
