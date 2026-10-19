#pragma once
#include "router/router.h"

// USB device towards the PC (native USB / micro USB): a single HID keyboard.
class HidDevice {
public:
    static HidDevice& instance();

    void init();
    void task();

    bool ready() const;
    bool suspended() const;
    void wakeup();
    void sendKeyboard(const KeyboardReport& report);

private:
    HidDevice() = default;
};
