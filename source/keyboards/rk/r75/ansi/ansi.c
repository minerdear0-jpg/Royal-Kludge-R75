// Copyright 2024 SDK (@sdk66)
// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H

#define LED_ENABLE_PIN A5

void keyboard_pre_init_kb(void) {
    gpio_set_pin_output(LED_ENABLE_PIN);
    gpio_write_pin_high(LED_ENABLE_PIN);

    gpio_set_pin_output(LED_WIN_LOCK_PIN);
    gpio_write_pin_high(LED_WIN_LOCK_PIN);

    gpio_set_pin_output(LED_MAC_PIN);
    gpio_write_pin_high(LED_MAC_PIN);

    keyboard_pre_init_user();
}

void suspend_power_down_kb(void) {
    /* Keep the LED driver powered. Pulling A5 low on USB suspend glitches
     * WS2812 and was mistaken for "host off". RGB blackout is in lighting_profile. */
    gpio_write_pin_high(LED_ENABLE_PIN);
    suspend_power_down_user();
}

void suspend_wakeup_init_kb(void) {
    gpio_write_pin_high(LED_ENABLE_PIN);
    suspend_wakeup_init_user();
}
