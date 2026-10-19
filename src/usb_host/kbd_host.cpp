#include "usb_host/kbd_host.h"

#include <algorithm>
#include <cstdio>

extern "C" {
#include "pio_usb.h"
}
#include "tusb.h"

#include "router/router.h"

namespace {
// D+ = GP27, D- = GP28. PIO-USB requires D- to be D+ + 1.
constexpr uint8_t kPioUsbDpPin = 27;
}  // namespace

KbdHost& KbdHost::instance() {
    static KbdHost host;
    return host;
}

void KbdHost::init() {
    pio_usb_configuration_t cfg = PIO_USB_DEFAULT_CONFIG;
    cfg.pin_dp = kPioUsbDpPin;
    tuh_configure(BOARD_TUH_RHPORT, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &cfg);
    tuh_init(BOARD_TUH_RHPORT);
}

void KbdHost::task() { tuh_task(); }

void KbdHost::setLeds(uint8_t leds) {
    if (!mounted_) return;
    ledReport_ = leds;
    tuh_hid_set_report(addr_, instance_, 0, HID_REPORT_TYPE_OUTPUT, &ledReport_, sizeof ledReport_);
}

bool KbdHost::isKeyboard(uint8_t devAddr, uint8_t instance) const {
    return mounted_ && devAddr == addr_ && instance == instance_;
}

void KbdHost::onMount(uint8_t devAddr, uint8_t instance) {
    // Only the boot keyboard interface is bridged. Switching it to the boot
    // protocol guarantees the fixed 8-byte report format we forward 1:1.
    if (tuh_hid_interface_protocol(devAddr, instance) == HID_ITF_PROTOCOL_KEYBOARD) {
        addr_ = devAddr;
        instance_ = instance;
        mounted_ = true;
        tuh_hid_set_protocol(devAddr, instance, HID_PROTOCOL_BOOT);
        std::printf("keyboard mounted: addr=%u instance=%u\n", devAddr, instance);
    }
    tuh_hid_receive_report(devAddr, instance);
}

void KbdHost::onUnmount(uint8_t devAddr, uint8_t instance) {
    if (!isKeyboard(devAddr, instance)) return;
    mounted_ = false;
    Router::instance().keyboardReleased();  // avoid stuck keys on the PC
    std::printf("keyboard removed\n");
}

void KbdHost::onReport(uint8_t devAddr, uint8_t instance, const uint8_t* report, uint16_t len) {
    if (isKeyboard(devAddr, instance) && len >= sizeof(KeyboardReport)) {
        KeyboardReport r;
        std::copy_n(report, r.size(), r.begin());
        Router::instance().keyboardReport(r);
    }
    tuh_hid_receive_report(devAddr, instance);
}

extern "C" {

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, const uint8_t*, uint16_t) {
    KbdHost::instance().onMount(dev_addr, instance);
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
    KbdHost::instance().onUnmount(dev_addr, instance);
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, const uint8_t* report, uint16_t len) {
    KbdHost::instance().onReport(dev_addr, instance, report, len);
}

}  // extern "C"
