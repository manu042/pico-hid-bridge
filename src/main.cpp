#include <cstdio>

#include "hardware/clocks.h"
#include "pico/stdlib.h"

#include "router/router.h"
#include "usb_device/hid_device.h"
#include "usb_host/kbd_host.h"

int main() {
    // PIO-USB bit-bangs full speed USB (12 Mbit/s) and needs a sys clock that
    // is a multiple of 12 MHz (the default 125 MHz is not).
    // 120 MHz is what the Pico-PIO-USB examples use.
    set_sys_clock_khz(120000, true);

    // Debug output goes to UART0 (GPIO 0/1)
    stdio_init_all();
    std::printf("pico-hid-bridge starting\n");

    // The three parts of the bridge:
    //   host   - reads the keyboard through PIO-USB (GPIO 27/28)
    //   router - passes reports from the keyboard to the PC and the LED state
    //            (Caps Lock etc.) from the PC back to the keyboard
    //   device - the HID keyboard the PC sees on the micro USB port
    auto& device = HidDevice::instance();
    auto& host = KbdHost::instance();
    auto& router = Router::instance();

    // Both USB stacks must be initialised once before the loop starts.
    device.init();
    host.init();

    // Everything is polled from this single loop; there are no blocking calls,
    // so each task() has to return quickly or USB timing will suffer.
    while (true) {
        device.task();  // TinyUSB device stack: talks to the PC
        host.task();    // TinyUSB host stack: talks to the keyboard
        router.task();  // forwards pending reports and LED changes
    }
}
