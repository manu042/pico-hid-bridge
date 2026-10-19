#pragma once
#include <cstdint>

// USB host towards the keyboard (PIO-USB on GPIOs).
class KbdHost {
public:
    static KbdHost& instance();

    void init();
    void task();
    void setLeds(uint8_t leds);

    // Called from the TinyUSB host callbacks.
    void onMount(uint8_t devAddr, uint8_t instance);
    void onUnmount(uint8_t devAddr, uint8_t instance);
    void onReport(uint8_t devAddr, uint8_t instance, const uint8_t* report, uint16_t len);

private:
    KbdHost() = default;
    bool isKeyboard(uint8_t devAddr, uint8_t instance) const;

    uint8_t addr_ = 0;
    uint8_t instance_ = 0;
    bool mounted_ = false;
    uint8_t ledReport_ = 0;  // must outlive the async transfer
};
