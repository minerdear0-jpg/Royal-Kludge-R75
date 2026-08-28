// Copyright 2024 DV (@iamdanielv)
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#define LED_WIN_LOCK_PIN B9
#define LED_MAC_PIN B8

#define RGB_MATRIX_LED_COUNT 81

/* SPI */
#define SPI_DRIVER SPIDQ
#define SPI_SCK_PIN B3
#define SPI_MOSI_PIN B5
#define SPI_MISO_PIN B4

/* Flash */
#define EXTERNAL_FLASH_SPI_SLAVE_SELECT_PIN C12
#define WEAR_LEVELING_LOGICAL_SIZE (WEAR_LEVELING_BACKING_SIZE / 2)

/* Scan / USB */
#define DEBOUNCE 8
#define TAP_CODE_DELAY 10
#define ENCODER_MAP_KEY_DELAY 10

/* C-layer paints from indicators. SOLID_COLOR keeps the RGB task running
 * (mode 0 / NONE never calls indicators). KEYPRESSES feeds Game Mode flashes. */
#define RGB_MATRIX_KEYPRESSES
#define RGB_TRIGGER_ON_KEYDOWN
#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_SOLID_COLOR
#define RGB_MATRIX_DEFAULT_HUE 166
#define RGB_MATRIX_DEFAULT_SAT 255
#define RGB_MATRIX_DEFAULT_VAL 128

#define ENABLE_RGB_MATRIX_SOLID_COLOR

/* WS2812 */
#define WS2812_SPI_DRIVER SPIDM2
#define WS2812_SPI_DIVISOR 32
