#include "router/router.h"

#include "usb_device/hid_device.h"
#include "usb_host/kbd_host.h"

Router& Router::instance() {
    static Router router;
    return router;
}

void Router::keyboardReport(const KeyboardReport& report) {
    pending_ = report;
    hasPending_ = true;
}

void Router::keyboardReleased() {
    pending_.fill(0);
    hasPending_ = true;
}

void Router::setLeds(uint8_t leds) {
    leds_ = leds;
    ledsDirty_ = true;
}

void Router::task() {
    auto& device = HidDevice::instance();

    if (hasPending_) {
        if (device.suspended()) {
            device.wakeup();  // report stays pending until the bus resumes
        } else if (device.ready()) {
            device.sendKeyboard(pending_);
            hasPending_ = false;
        }
    }

    if (ledsDirty_) {
        ledsDirty_ = false;
        KbdHost::instance().setLeds(leds_);
    }
}
