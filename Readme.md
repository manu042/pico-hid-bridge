# pico-hid-bridge
Turns a Raspberry Pi Pico 2 W into a pass-through USB keyboard. Plug a keyboard into the Pico, plug the Pico into your PC, and every keystroke is forwarded. The router in between is the place to add remapping, filtering or logging later.

```
USB keyboard ──(GPIO 27/28, PIO-USB host)──> Pico 2 W ──(micro USB, HID device)──> PC
```

## Wiring
The keyboard needs a USB-A socket on the Pico side. Either use a USB-A breakout board or cut the keyboard cable and solder it on directly.

| Signal | Usual wire color | Pico          |
|--------|------------------|---------------|
| VBUS   | red              | VBUS (pin 40) |
| D+     | green            | GPIO 27       |
| D-     | white            | GPIO 28       |
| GND    | black            | GND (pin 38)  |

The Pico's own micro USB port goes to the PC and powers everything, keyboard included. Debug output goes to UART0 (GPIO 0 TX, GPIO 1 RX, 115200 baud), since the USB port is taken.


## Build
You need CMake 3.20 or newer, Git and the Arm GNU toolchain. The Pico SDK and Pico-PIO-USB are downloaded during configure.

```sh
cmake -B build
cmake --build build
```

To flash, hold BOOTSEL while plugging in the Pico and copy `build/kb_bridge.uf2` onto the drive that shows up.


## Limitations
- Low and full speed devices only. That covers basically every keyboard, but not high speed devices.
- No remapping or filtering yet. Reports are passed through unchanged.


## Code layout
- `src/usb_host/`: reads the keyboard via PIO-USB
- `src/usb_device/`: the HID keyboard the PC sees
- `src/router/`: passes reports from host to device
