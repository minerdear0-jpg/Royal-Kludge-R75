R75_COMMON := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
VPATH += $(R75_COMMON)
EXTRACFLAGS += -I$(R75_COMMON)
CFLAGS += -I$(R75_COMMON)
# sym_defer_pk: wait DEBOUNCE ms of stable state before press and release.
# Fixes double/triple chars from press bounce; asym_eager/sym_eager report too early.
DEBOUNCE_TYPE = sym_defer_pk
SRC += lighting_profile.c
SRC += game_mode.c
SRC += socd_cleaner.c
SRC += indicator_queue.c
SRC += tap_hold.c
