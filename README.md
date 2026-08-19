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
| Game Mode | **Fn + G** — только игровые клавиши, остальное гасится; импульс ~150 мс |
| Сутки | **Fn + `** цикл день → сумерки → ночь (пишется в EEPROM). Авто по сессии: 0–4 ч день, 4–6 ч сумерки, 6+ ч ночь. 45 мин без ввода → снова день |
| RGB | **Fn+, / .** (или ISO **<>**) цвет. **Fn+↑/↓** яркость. **Fn+←/→** скорость. **Fn+\\** эффект. Подсветка Fn — легенда клавиш, не «правая зона» |
| Bootloader | Options: трижды **Q** |
| Сброс EEPROM | **Fn**, три раза **Space** (или Options: трижды **Z**) |

**Сутки:** часов нет — только аптайм сессии после питания или выхода ПК из сна. Смена режима (авто или **Fn+`**) — плавный переход ~60 с. Простой гасит подсветку: 10 мин днём, 5 мин в сумерках, 2 мин ночью. ПК выключен/сон USB — красный ночник ~20%, питание LED не режется.

**Game Mode:** перекрывает сутки, таймеры сессии заморожены. Win/Super заблокирован, SOCD last-win на WASD, NKRO. Неиспользуемые клавиши чёрные; WASD/стрелки, 1–5 и моды подсвечены; нажатие — короткий белый импульс. Повтор **Fn+G** возвращает суточный профиль. SOCD в Valorant считается Snap Tap — бан.

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
