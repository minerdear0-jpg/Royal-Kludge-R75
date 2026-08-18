# RK R75 ANSI - Custom QMK Firmware

* Keyboard Maintainer: [sdk66](https://github.com/sdk66)
* Hardware Supported: Royal Kludge R75 (ANSI, проводная версия)
* Hardware Availability: [rk](http://www.rkgaming.com)

## Особенности кастомной прошивки

Эта кастомная прошивка QMK предоставляет расширенные функции по сравнению с оригинальной прошивкой:

* **Поддержка VIA** - конфигурация в реальном времени через графический интерфейс
* **SOCD Cleaner** - очистка одновременных нажатий для WASD (улучшает отзывчивость в играх)
* **Game Mode** - подсветка WSAD красными светодиодами для игр
* **Расширенные RGB индикаторы** - визуальная индикация активных слоёв и функций
* **Несколько слоёв** - Windows, Fn, Options, Mac, Numpad
* **Интеграция с OpenRGB и SignalRGB**

## Сборка и прошивка

Make example for this keyboard (after setting up your build environment):

    make rk/r75/ansi:via
        
Flashing example for this keyboard:

    make rk/r75/ansi:via:flash

To reset the board into bootloader mode, do one of the following:

* Hold the Reset switch mounted on the bottom side of the PCB while connecting the USB cable
* Hold the Escape key while connecting the USB cable (also erases persistent settings)
* Fn+R_Shift+Esc will reset the board to bootloader mode if you have flashed the default QMK keymap

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Слои клавиатуры

### Слой 0 - Base (Windows)
Основной слой для повседневного использования.

### Слой 1 - Fn Layer
Активируется удержанием `MO(1)` (правый Alt). Содержит:
* Управление медиа (плей, пауза, следующий, предыдущий)
* Управление громкостью
* Управление RGB подсветкой
* Print Screen, Insert, End, Pause, Scroll Lock

### Слой 2 - Options Layer
Активируется через `TO(2)` или `TD_CTL_TG`. Содержит:
* Переключение между слоями (TO(0), TO(3), TO(4))
* **SOCD Toggle** - включение/выключение SOCD cleaner (на T)
* **NKRO Toggle** - включение/выключение N-Key Rollover (на N)
* **Game Mode Toggle** - включение/выключение Game Mode (на /)
* Tap Dance для сброса платы (Q)
* Tap Dance для очистки EEPROM (Z)

### Слой 3 - Mac Layer
Переключает модификаторы для совместимости с macOS (Cmd/Alt).

### Слой 4 - Num Layer
Цифровой блок для ввода чисел.

### Слой 5 - Fn Layer (Alternative)
Альтернативный Fn слой (через `LT(_FN_LYR, KC_RALT)`).

## Game Mode

Game Mode подсвечивает клавиши **W, S, A, D** красными светодиодами для улучшения видимости во время игр.

### Включение/Выключение Game Mode:
1. Перейдите в **Options Layer** (Слой 2)
2. Нажмите клавишу **/** (слэш) - это `GAME_MODE_TOG`
3. При включении WSAD мигнёт красным 3 раза
4. При выключении WSAD мигнёт синим 2 раза

### Альтернативная активация:
Можно настроить комбинацию клавиш через VIA или добавить свой макрос в keymap.c

## SOCD Cleaner

SOCD (Simultaneous Opposing Cardinal Directions) Cleaner предотвращает конфликтующие нажатия противоположных клавиш (W+S, A+D), что улучшает отзывчивость в играх.

* По умолчанию используется режим **SOCD_CLEANER_LAST** - последнее нажатие имеет приоритет
* Включается/выключается через `SOCDTOG` на слое Options
* Также можно управлять через `socd_cleaner_enabled` в коде

## RGB Индикаторы

Различные слои имеют уникальную подсветку для визуальной индикации:

* **Слой 1 (Fn)**: F-ряд - бирюзовый, навигационные клавиши - синие, Right Shift - красный
* **Слой 2 (Options)**: NKRO - красный, SnapTap - оранжевый, Reset - жёлтый, Clear EEPROM - фиолетовый, SignalRGB - зелёный, OpenRGB - голубой
* **Слой 4 (Num)**: Nav - красный, Numpad - зелёный, F1 - фиолетовый

## Технические детали

* **Процессор**: WB32FQ95
* **RGB светодиодов**: 80
* **Поддержка энкодера**: Да
* **VIA**: Да
* **LTO**: Включено для оптимизации размера прошивки

## Известные проблемы

* Оригинальная беспроводная версия R75 не поддерживается
* Для некоторых функций требуется пересборка прошивки

## Вклад в проект

Pull requests приветствуются! Особенно:
* Улучшения производительности
* Новые функции
* Исправления ошибок
* Документация

## Лицензия

GPL-2.0-or-later