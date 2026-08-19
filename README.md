# Royal Kludge R75 — QMK

Прошивка для проводной RK R75 (WB32FQ95). RGB и слои живут на клавиатуре; VIA — только раскладка. OpenRGB и SignalRGB намеренно не используются.

Linux на слое 0 — это нормальная «Windows» раскладка: Super стоит там же, где Win. Слой Mac нужен только macOS.

USB опрос 1 мс (1000 Гц). Debounce 5 мс, тип `asym_eager_defer_pk` (нажатие сразу, отпускание с антидребезгом). Энкодер: `TAP_CODE_DELAY` / `ENCODER_MAP_KEY_DELAY` 10 мс вместо дефолтных 100.

Готовые `.hex` / `.bin` — в папке [`firmware/`](firmware/) этого репозитория и в [GitHub Releases](https://github.com/minerdear0-jpg/Royal-Kludge-R75/releases) (когда релиз опубликован).

## Какой файл качать

| Файл | Клавиатура | USB VID:PID |
| :--- | :--- | :--- |
| `royal_kludge_r75_customiso_via.hex` | **ISO** (Enter высокой формы, `<>` слева от Z) | `342d:e483` |
| `royal_kludge_r75_custom_via.hex` | **ANSI** (рекомендуется) | `342d:e484` |
| `royal_kludge_r75_ansi_via.hex` | ANSI, тот же PID — запасной билд | `342d:e484` |

Не прошивай ISO-файл на ANSI и наоборот: разъедет RGB и Enter.

## Как войти в bootloader (DFU)

Клавиатура должна мигать / пропасть как обычный HID и появиться как WB32 DFU.

1. Выдерни USB.
2. Зажми **Escape** (сотрёт EEPROM) **или** кнопку Reset на нижней стороне PCB (настройки сохранятся).
3. Воткни USB, не отпуская клавишу/кнопку, держи ещё секунду.

Если текущая прошивка уже QMK: **Fn + Right Shift**, трижды **Q** (tap dance reset).

## Прошивка

Нужен [wb32-dfu-updater](https://github.com/WestberryTech/wb32-dfu-updater/releases) (это не STM32 DFU). QMK Toolbox умеет WB32, если в нём есть этот бэкенд.

Подставь путь к скачанному `.hex`. Адрес вспышки: `0x08000000`.

### Linux

```shell
# Arch / CachyOS: пакет или бинарник с GitHub
# udev, чтобы не нужен был root (после — переподключи клавиатуру):
# SUBSYSTEM=="usb", ATTRS{idVendor}=="342d", MODE="0666"

wb32-dfu-updater_cli -t -s 0x08000000 -D royal_kludge_r75_customiso_via.hex
```

Если команда не видит устройство — ты не в DFU (повтори шаг с Escape).

### Windows

1. Скачай `wb32-dfu-updater` для Windows или [QMK Toolbox](https://github.com/qmk/qmk_toolbox/releases).
2. Войди в DFU.
3. Toolbox: Open `.hex` → Flash.  
   CLI: `wb32-dfu-updater_cli.exe -t -s 0x08000000 -D royal_kludge_r75_custom_via.hex`
4. Если Windows ставит «неизвестное устройство» — Zadig, WinUSB на WB32 DFU.

### macOS

1. QMK Toolbox **или** `wb32-dfu-updater` (бинарник с GitHub).
2. Войди в DFU, разреши доступ к USB, если спросит.
3. Toolbox: Open → Flash.  
   CLI: `wb32-dfu-updater_cli -t -s 0x08000000 -D royal_kludge_r75_custom_via.hex`

После прошивки выдерни USB и воткни снова.

## VIA

1. [usevia.app](https://usevia.app/) → Settings → включи **Design**.
2. Загрузи JSON из `layouts/`: `VIA Layout ISO.json` для ISO, `VIA Layout.json` для ANSI.

## Что умеет MCU в офисе и в IDE

Клавиатура не видит монитор по DDC и не управляет окнами IDE сама. Она шлёт HID. Имеет смысл то, что ОС принимает без демона на ПК.

| На клавиатуре | Зачем |
| :--- | :--- |
| **Fn + энкодер** | Яркость экрана (`KC_BRID` / `KC_BRIU` → `XF86MonBrightness*`) |
| Энкодер без Fn | Громкость |
| Fn + Right Shift + энкодер | Перемотка медиа |
| Fn+G | Game Mode (Win/Super lock, SOCD, NKRO, WASD) |
| VIA | Макросы IDE, F13–F24, слой под i3/Sway/Hyprland |

На ноутбуке и на многих DE Linux яркость с энкодера работает сразу. На **внешнем** мониторе HID яркость часто ничего не делает: там нужен `ddcutil` (или бинд WM), это уже хост, не прошивка.

Не копируется из [lazercore](https://github.com/pk-vishnu/lazercore): Type Alchemy (Unicode на хосте) и аудиовизуализатор (Python + Raw HID).

## Светодиоды Mac / Win-lock

На плате два отдельных LED, оба **active-low** (горит = пин в `0`).

| LED | Смысл |
| :--- | :--- |
| **Mac** | Только слой 3 (модификаторы macOS) |
| **Win-lock** | Заблокирован GUI/Super (`keymap_config.no_gui`, обычно Game Mode) |

Linux: слой 0, слой Mac не включать.

## Слои

| Слой | Роль |
| :--- | :--- |
| 0 | День: Linux / Windows |
| 1 | Fn: медиа, RGB, яркость на энкодере, Fn+G |
| 2 | Options: bootloader, EEPROM, NKRO, SOCD, Mac/numpad |
| 3 | macOS (Alt/GUI swap) |
| 4 | Numpad на альфа-клавишах |

## Сборка из исходников

Официальный QMK, fork с OpenRGB не нужен. Клавиатуры лежат в `source/keyboards/rk/r75/` — скопируй `customiso`, `custom`, `ansi` в `qmk_firmware/keyboards/royal_kludge/r75/`.

```shell
qmk compile -kb royal_kludge/r75/customiso -km via
qmk compile -kb royal_kludge/r75/custom -km via
qmk compile -kb royal_kludge/r75/ansi -km via
```

## Credits

[@irfanjmdn](https://github.com/irfanjmdn/), [@sdk66](https://github.com/sdk66/), [@iamdanielv](https://github.com/iamdanielv). Идея Fn+энкодер = яркость — [lazercore](https://github.com/pk-vishnu/lazercore).
