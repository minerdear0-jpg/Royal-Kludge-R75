# Royal Kludge R75

Кастомный QMK для **проводной** RK R75 (MCU WB32FQ95). Подсветка и слои на клавиатуре. [VIA](https://usevia.app/) — только раскладка.

**Нет OpenRGB и SignalRGB.** Архив старых `.hex` и плагинов — [`firmware/archive/`](firmware/archive/). Не прошивать.

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

Уже на этом QMK: **Fn+Esc** или Options **три Q**. Esc и Caps ~2.5 с мигают короткими вспышками как TX, затем HID пропадает — это DFU. Любая другая клавиша или энкодер в эти 2.5 с **отменяет** вход, прошивка остаётся. Кадр WS2812 в DFU не держится: ROM-загрузчик сбрасывает GPIO, ленту во время прошивки мигать нельзя.

**Выйти из DFU хоткеем нельзя** — ROM не читает матрицу. Выход: выдернуть USB (питание сбросится, старт обычной прошивки) или прошить файл. Reset на днище снова кидает в DFU, не наружу.

**Esc при втыкании кабеля** и Reset на днище — сразу DFU, мигания нет.

Клавиатура пропадает как обычный HID и видна как WB32 DFU. Если прошивальщик «не находит устройство» — ты не в DFU.

Адрес flash: `0x08000000`. Ниже подставь свой `.hex`.

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
| Game Mode | **Fn+G** или Options **/**. Пока зажат Fn, **G** контрастный к текущему цвету (не красный на красном). Зоны в `features/game_lighting.h`. Win-lock и NKRO, **не** SOCD |
| Пресеты света | ISO **Fn+<>** / ANSI **Fn+\\**: белый → янтарь → зелёный → красный → гейминг. Яркость **каждого** (**Fn+↑/↓**) — EEPROM ~2.5 с |
| Ночник | Лампа: ПК выключен, USB с питанием. Через 10 с всегда **красный** (не режим цикла). Энкодер: яркость лампы (EEPROM) и вежливый клик. Не путать с красным пресетом |
| RGB | **Fn+↑/↓** яркость текущего пресета. В ночнике яркость лампы крутит энкодер |
| SOCD last-win | **Fn+Right Shift, T**. Пока включён — тлеет **Home**. Valorant: Snap Tap, бан. С Game Mode сам не включается |
| Bootloader | **Fn+Esc** / Options три **Q**: TX-мигание ~2.5 с, любая клавиша — отмена. Уже в DFU — только выдернуть USB. Reset на днище снова в DFU |
| Сброс EEPROM | **Fn**, три раза **Space** (или Options: трижды **Z**). Перед сбросом — ледяная полоса слева направо |

**Пресеты (C-слой):** белый / янтарь / зелёный / красный / гейминг. У каждого своя last-used яркость **Fn+↑/↓** в EEPROM. Смена — вспышка, fade 3 с (гейминг 0.5 с). Sleep: 10 / 5 / 10 / 10 / 10 мин.

USB suspend (ПК спит / выключен, VBUS есть): через **10 с** — красная лампа-ночник (своя яркость в EEPROM, не яркость красного пресета). Энкодер крутит лампу, клик — плавное выкл/вкл. Пробуждение хоста возвращает текущий пресет. Короткий Linux autosuspend (~2 с) не трогает ленту. Пин A5 не опускаем. `RGB_DISABLE_WHEN_USB_SUSPENDED` специально не включён.

**Гейминг:** WASD+QEFG+стрелки лимон 40%, 1–6 фиолетовый 40%, Ctrl/Shift/Space бирюза 30%, ZXCV лёд 30%, B и правый блок выключены. Вспышка 150 мс. Win-lock и NKRO включаются с этим пресетом. SOCD — отдельно, Options **T**.

**Options** (удерживай **Fn + Right Shift**): F1 = Linux/Win слой, F2 = Mac, F3 = numpad, **T** = SOCD, **N** = NKRO, **/** = Game Mode.

Светодиод **Mac** — только слой Mac. Светодиод **Win-lock** — только когда заблокирован Super (обычно Game Mode).

USB 1000 Гц (`polling_interval` 1 мс), debounce 5 мс.

**Вход в DFU не завязан на VIA:** Bootmagic — матрица Esc при подключении кабеля. **Fn+Esc** — `QK_BOOT` из прошивки. Reset на днище — железный загрузчик.

## Сборка

Исходники платы: `source/keyboards/rk/r75/{common,customiso,custom,ansi}`. Общая логика (профили, Game Mode, SOCD) — в `common/`, раскладки не копируют эти `.c`. Полного QMK в репозитории нет. `iso/` нет: это был шаблон RK851.

В **своём** дереве [qmk_firmware](https://docs.qmk.fm/) положи их так, чтобы цель компиляции совпала с папкой:

```shell
rsync -a source/keyboards/rk/r75/common/     ~/qmk_firmware/keyboards/royal_kludge/r75/common/
rsync -a source/keyboards/rk/r75/customiso/ ~/qmk_firmware/keyboards/royal_kludge/r75/customiso/
rsync -a source/keyboards/rk/r75/custom/    ~/qmk_firmware/keyboards/royal_kludge/r75/custom/
rsync -a source/keyboards/rk/r75/ansi/      ~/qmk_firmware/keyboards/royal_kludge/r75/ansi/

qmk compile -kb royal_kludge/r75/customiso -km via
qmk compile -kb royal_kludge/r75/custom -km via
qmk compile -kb royal_kludge/r75/ansi -km via
```

Путь `royal_kludge/r75` — это имя каталога **после копирования**, не поле `"manufacturer": "RK"` в `keyboard.json`. Собирать как `rk/r75/customiso` можно, только если скопировал в `keyboards/rk/r75/`. Fork QMK с OpenRGB не нужен.

Компилятор — дефолт QMK (`-Os` + LTO). `OPT_LEVEL=3` не ставим: для этой платы важнее размер, чем мифические 15% с `-O3`. NKRO включён (`force_nkro` + Game Mode), это не «выключен ради скорости».

## Релиз и ревью

Готовые `.hex`/`.bin` — в [`firmware/`](firmware/). Архив OpenRGB/SignalRGB — [`firmware/archive/`](firmware/archive/).

**Аудитор и чужие PR.** Можно предлагать патчи. Зелёный CI (`.github/workflows/firmware.yml`) не равен мержу. Владелец смотрит дифф (HID, RGB, SOCD, EEPROM) и говорит, что входит в релиз. Автомерж и «починить заодно» без согласования не используются. `CODEOWNERS` — `@minerdear0-jpg`.

## Credits

[@irfanjmdn](https://github.com/irfanjmdn/), [@sdk66](https://github.com/sdk66/), [@iamdanielv](https://github.com/iamdanielv). Яркость на Fn+энкодере — идея [lazercore](https://github.com/pk-vishnu/lazercore).
