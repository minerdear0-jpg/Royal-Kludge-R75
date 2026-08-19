ENCODER_MAP_ENABLE = yes
VIA_ENABLE = yes
TAP_DANCE_ENABLE = yes
LTO_ENABLE = yes
DEBOUNCE_TYPE = asym_eager_defer_pk
OPT_LEVEL = 3

# Оптимизация производительности для игр
CONSOLE_ENABLE = no      # Отключаем debug вывод (экономит CPU и место)
COMMAND_ENABLE = no      # Отключаем USB команды
NKRO_ENABLE = no         # Обычный N-Key Rollover быстрее чем full NKRO
AUDIO_ENABLE = no        # Отключаем аудио
MUSIC_ENABLE = no        # Отключаем музыку
OPENRGB_ENABLE = yes     # Оставляем для совместимости
RAW_ENABLE = yes         # Нужно для VIA
RGB_MATRIX_KEYREACTIVE_ENABLED = yes
SIGNALRGB_SUPPORT_ENABLE = yes

# Исходные файлы модулей
SRC += features/indicator_queue.c
SRC += features/tap_hold.c
SRC += features/indicators.c
SRC += features/rgb_keys.c
SRC += features/socd_cleaner.c
SRC += features/game_mode.c