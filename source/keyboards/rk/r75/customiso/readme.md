# RK R75 Custom ISO

* Keyboard Maintainer: [sdk66](https://github.com/sdk66)
* Hardware Supported: RK R75 Custom ISO (WB32FQ95)
* Hardware Availability: [rk](http://www.rkgaming.com)

Make example for this keyboard (after setting up your build environment):

    make rk/r75/customiso:via
        
Flashing example for this keyboard:

    make rk/r75/customiso:via:flash

To reset the board into bootloader mode, do one of the following:

* Hold the Reset switch mounted on the bottom side of the PCB while connecting the USB cable
* Hold the Escape key while connecting the USB cable (also erases persistent settings)
* Fn+R_Shift+Esc will reset the board to bootloader mode if you have flashed the default QMK keymap

## Features

### RGB Lighting
* 80 addressable RGB LEDs
* Integration with VIA, OpenRGB, and SignalRGB
* Multiple lighting effects and customization options

### Layers
* **Layer 0**: Default Windows layout
* **Layer 1**: Function layer (Fn keys, media controls, RGB control)
* **Layer 2**: Options layer (SOCD, NKRO, reset, EEPROM clear, RGB mode switching)
* **Layer 3**: Mac layout
* **Layer 4**: Numpad layer

### Advanced Features
* **SOCD Cleaner**: Simultaneous Opposite Cardinal Direction cleaning for competitive gaming
* **NKRO Toggle**: Full N-Key Rollover support
* **Encoder Support**: Volume and media control
* **Safe Reset/EEPROM Clear**: Triple-tap protection via Tap Dance
* **VIA Support**: Real-time key remapping and macro programming

### Game Mode
Activate Game Mode by pressing **Fn + Right Shift + G** (configure in VIA).

When enabled:
* WASD keys are highlighted in red for easy identification during gaming
* Optimized LED handling for minimal performance impact
* Can be toggled on/off without leaving your game

To configure the Game Mode activation key:
1. Open VIA
2. Go to the "Special" tab
3. Assign a custom keycode to activate Game Mode toggle

## Performance Optimizations

This firmware includes several optimizations for better performance:

* **Efficient LED Updates**: Batched LED updates reduce processing overhead
* **Named Constants**: All LED indices use named constants for maintainability
* **Modular Architecture**: Features are separated into independent modules
* **Queue-based Indicators**: Non-blocking indicator animations
* **Optimized Layer Detection**: Cached layer state checks

## Build Options

Enable/disable features in `rules.mk`:
* `OPENRGB_ENABLE` - OpenRGB integration
* `SIGNALRGB_SUPPORT_ENABLE` - SignalRGB integration
* `ENCODER_MAP_ENABLE` - Encoder support

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

