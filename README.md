# Royal Kludge R75

Кастомный QMK для **проводной** RK R75 (MCU WB32FQ95). Подсветка и слои на клавиатуре. [VIA](https://usevia.app/) — только раскладка.

**Нет OpenRGB и SignalRGB.** Старые `.hex` с ними в `firmware/` — архив, их не ставить.

Беспроводная R75 этим репозиторием не поддерживается.

## Что качать

Готовые файлы: папка [`firmware/`](firmware/).

| Файл | Клавиатура | VID:PID |
| :--- | :--- | :--- |
| [`royal_kludge_r75_customiso_via.hex`](firmware/royal_kludge_r75_customiso_via.hex) | **ISO** — высокий Enter, клавиша `<>` слева от Z | `342d:e483` |
| [`royal_kludge_r75_custom_via.hex`](firmware/royal_kludge_r75_custom_via.hex) | **ANSI** | `342d:e484` |

`royal_kludge_r75_ansi_via.hex` — тот же ANSI, что `custom`. ISO и ANSI не путать: разъедутся Enter и RGB.

Рядом лежат `.bin` — для `wb32-dfu-updater`, если удобнее не hex.

## Прошивка (Windows, macOS, Linux)

Загрузчик **WB32 DFU**, не STM32. Нужны [wb32-dfu-updater](https://github.com/WestberryTech/wb32-dfu-updater/releases) и/или [QMK Toolbox](https://github.com/qmk/qmk_toolbox/releases).

### Bootloader

1. Выдерни USB.
2. Зажми **Esc** (сотрёт EEPROM) **или** Reset на днище платы (настройки останутся).
3. Воткни кабель, не отпуская, ещё ~1 с.

Уже на этом QMK: **Fn + Right Shift**, три раза **Q**.

Клавиатура пропадает как обычный HID и видна как WB32 DFU. Если прошивальщик «не находит устройство» — ты не в DFU.

Адрес вспышки: `0x08000000`. Ниже подставь свой `.hex`.

### Linux

```shell
# udev (один раз), чтобы не нужен был root. Потом переподключи USB:
# SUBSYSTEM=="usb", ATTRS{idVendor}=="342d", MODE="0666"

wb32-dfu-updater_cli -t -s 0x08000000 -D royal_kludge_r75_customiso_via.hex
```

### Windows

QMK Toolbox: Open `.hex` → Flash.  
Или `wb32-dfu-updater_cli.exe -t -s 0x08000000 -D royal_kludge_r75_custom_via.hex`  
Если устройство «неизвестное» — Zadig, WinUSB на WB32 DFU.

### macOS

QMK Toolbox или `wb32-dfu-updater_cli` с GitHub. Разреши доступ к USB, если спросит. Та же команда, что на Linux.

После прошивки выдерни USB и воткни снова.

## VIA

[usevia.app](https://usevia.app/) → Settings → **Show Design tab** → загрузи JSON из [`layouts/`](layouts/):

- ISO → `VIA Layout ISO.json`
- ANSI → `VIA Layout.json`

## Как пользоваться

**Linux и Windows** — слой 0. Клавиша Win/Super на месте. Слой Mac не нужен (он меняет Alt и GUI под Apple).

| Действие | Как |
| :--- | :--- |
| Громкость | Энкодер |
| Яркость экрана | **Fn + энкодер** (`XF86MonBrightness*` — ноутбук/встроенный экран; внешний монитор часто нет, там `ddcutil`) |
| Перемотка | **Fn + Right Shift + энкодер** |
| Game Mode | Четвёртый пресет **Fn+<>** (ISO) / **Fn+\\** (ANSI), либо **Fn+G**. Карта зон в `features/game_lighting.h` |
| Пресеты света | **Fn+<>** (ISO, солнышко) цикл: день → сумерки → ночь → гейминг. ANSI: **Fn+\\**. Пишется в EEPROM |
| RGB | **Fn+, / .** цвет. **Fn+↑/↓** яркость. **Fn+←/→** скорость. **Fn+[ ]** эффект |
| Bootloader | **Fn + Esc**. Или USB выдернуть, держать **Esc**, воткнуть. Или Reset на днище. Options: трижды **Q** |
| Сброс EEPROM | **Fn**, три раза **Space** (или Options: трижды **Z**) |

**Пресеты (C-слой):** день белый 100% / сумерки янтарь 50% / ночь зелёный 15% / гейминг (blackout + зоны). Смена — вспышка целевым цветом, затем fade 3 с (в гейминг 0.5 с). Sleep: 10 / 5 / 2 / 10 мин. Профиль в EEPROM.

**Гейминг:** WASD+QEFG+стрелки лимон 40%, 1–6 фиолетовый 40%, Ctrl/Shift/Space бирюза 30%, ZXCV лёд 30%, B и правый блок выключены. Вспышка 150 мс. Win-lock и SOCD включаются вместе с этим пресетом. SOCD в Valorant — Snap Tap, бан.

**Вход в DFU не завязан на VIA:** Bootmagic — матрица Esc при подключении кабеля. **Fn+Esc** — `QK_BOOT` из прошивки. Reset на днище — железный загрузчик.

**Options** (удерживай **Fn + Right Shift**): F1 = Linux/Win слой, F2 = Mac, F3 = numpad, **T** = SOCD, **N** = NKRO, **/** = Game Mode.

Светодиод **Mac** — только слой Mac. Светодиод **Win-lock** — только когда заблокирован Super (обычно Game Mode).

USB 1000 Гц (`polling_interval` 1 мс), debounce 5 мс.

## Сборка

Официальный [QMK](https://docs.qmk.fm/), fork с OpenRGB не нужен. Скопируй `source/keyboards/rk/r75/{customiso,custom,ansi}` → `qmk_firmware/keyboards/royal_kludge/r75/`.

```shell
qmk compile -kb royal_kludge/r75/customiso -km via
qmk compile -kb royal_kludge/r75/custom -km via
```

## Credits

[@irfanjmdn](https://github.com/irfanjmdn/), [@sdk66](https://github.com/sdk66/), [@iamdanielv](https://github.com/iamdanielv). Яркость на Fn+энкодере — идея [lazercore](https://github.com/pk-vishnu/lazercore).
