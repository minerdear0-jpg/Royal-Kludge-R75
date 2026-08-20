ENCODER_MAP_ENABLE = yes
VIA_ENABLE = yes
TAP_DANCE_ENABLE = yes
LTO_ENABLE = yes
DEBOUNCE_TYPE = asym_eager_defer_pk
MOUSEKEY_ENABLE = no
CONSOLE_ENABLE = no
COMMAND_ENABLE = no

include $(abspath $(dir $(lastword $(MAKEFILE_LIST)))../../../common/rules.mk)
SRC += features/indicators.c
