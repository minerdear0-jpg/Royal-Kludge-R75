// Copyright 2024 R75 Custom Firmware
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include QMK_KEYBOARD_H

// LED Indices for WSAD keys
#define LED_W_INDEX 47
#define LED_S_INDEX 52
#define LED_A_INDEX 51
#define LED_D_INDEX 53

// Game Mode WSAD color (Red)
#define GAME_MODE_RED 0xFF, 0x00, 0x00

/**
 * @brief Initialize Game Mode state
 */
void game_mode_init(void);

/**
 * @brief Toggle Game Mode on/off
 * @return true if Game Mode is now enabled, false otherwise
 */
bool game_mode_toggle(void);

/**
 * @brief Check if Game Mode is currently active
 * @return true if Game Mode is enabled
 */
bool game_mode_is_active(void);

/**
 * @brief Apply Game Mode lighting to WSAD keys
 * @param led_min Minimum LED index to process
 * @param led_max Maximum LED index to process
 */
void game_mode_apply_lighting(uint8_t led_min, uint8_t led_max);

/**
 * @brief Clear Game Mode lighting from WSAD keys
 * @param led_min Minimum LED index to process
 * @param led_max Maximum LED index to process
 */
void game_mode_clear_lighting(uint8_t led_min, uint8_t led_max);
