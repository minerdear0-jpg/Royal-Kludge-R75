# MCU name
MCU = WB32FQ95

# Processor frequency
F_CPU = 72000000

# Bootloader selection
BOOTLOADER = wb32-dfu

# Build Options
#   change to "no" to disable the options, or define them in the Makefile
#
BOOTMAGIC_ENABLE = yes       # Enable Bootmagic Lite
EXTRAKEY_ENABLE = yes        # Audio control and System control
CONSOLE_ENABLE = no          # Console for debug
COMMAND_ENABLE = no          # Commands for debug and configuration
NKRO_ENABLE = yes            # USB N-Key Rollover
MOUSEKEY_ENABLE = yes        # Mouse keys
RGB_MATRIX_ENABLE = yes      # Enable RGB matrix effects
RGB_MATRIX_DRIVER = ws2812   # WS2812 driver

# Encoder support
ENCODER_ENABLE = yes

# VIA support
VIA_ENABLE = yes

# EEPROM wear leveling
EEPROM_DRIVER = wear_leveling
WEAR_LEVELING_DRIVER = spi_flash

# Link Time Optimization for better performance
OPTIMIZE = yes
LTO_ENABLE = yes

# Custom source files
SRC += game_mode.c socd_cleaner.c

# Include common rules
include ../../../common_rules.mk
