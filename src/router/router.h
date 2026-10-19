#pragma once
#include <array>
#include <cstdint>

// Boot protocol keyboard report: [0] modifiers, [1] reserved, [2..7] keycodes.
using KeyboardReport = std::array<uint8_t, 8>;

// Routes keyboard reports to the PC and LED state back to the keyboard.
// Reports are full-state snapshots, so only the latest one is kept. A slow PC
// can therefore never make us lose a key release.
class Router {
public:
    static Router& instance();

    void task();

    // From the host side (keyboard -> Pico).
    void keyboardReport(const KeyboardReport& report);
    void keyboardReleased();

    // From the device side (PC -> Pico): HID LED bitmask (Num/Caps/Scroll...).
    void setLeds(uint8_t leds);

private:
    Router() = default;

    KeyboardReport pending_{};
    bool hasPending_ = false;
    uint8_t leds_ = 0;
    bool ledsDirty_ = false;
};
