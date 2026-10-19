#include "usb_device/hid_device.h"

#include <array>
#include <cstring>

#include "pico/unique_id.h"
#include "tusb.h"

namespace {

// --- Descriptors -----------------------------------------------------------------

constexpr tusb_desc_device_t kDeviceDescriptor = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x1209,  // pid.codes open-source VID
    .idProduct = 0x0001,  // pid.codes shared test PID; request an own PID at pid.codes
    .bcdDevice = 0x0100,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

constexpr uint8_t kHidReportDescriptor[] = {TUD_HID_REPORT_DESC_KEYBOARD()};

enum { ITF_NUM_HID, ITF_NUM_TOTAL };
constexpr uint8_t kEpHid = 0x81;
constexpr uint16_t kConfigTotalLen = TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN;

constexpr uint8_t kConfigDescriptor[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, kConfigTotalLen, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_KEYBOARD, sizeof(kHidReportDescriptor), kEpHid,
                       CFG_TUD_HID_EP_BUFSIZE, 1),
};

char serial[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];

// Index 0 is the language ID and handled separately.
const char* stringFor(uint8_t index) {
    switch (index) {
        case 1: return "RPI";
        case 2: return "Keyboard";
        case 3: return serial;
        default: return nullptr;
    }
}

}  // namespace

extern "C" {

const uint8_t* tud_descriptor_device_cb(void) { return reinterpret_cast<const uint8_t*>(&kDeviceDescriptor); }

const uint8_t* tud_descriptor_configuration_cb(uint8_t) { return kConfigDescriptor; }

const uint8_t* tud_hid_descriptor_report_cb(uint8_t) { return kHidReportDescriptor; }

const uint16_t* tud_descriptor_string_cb(uint8_t index, uint16_t) {
    static std::array<uint16_t, 32> buf;
    size_t count;

    if (index == 0) {
        buf[1] = 0x0409;  // English
        count = 1;
    } else {
        const char* str = stringFor(index);
        if (!str) return nullptr;
        count = std::min<size_t>(std::strlen(str), buf.size() - 1);
        for (size_t i = 0; i < count; i++) buf[1 + i] = static_cast<uint8_t>(str[i]);
    }

    buf[0] = static_cast<uint16_t>((TUSB_DESC_STRING << 8) | (2 * count + 2));
    return buf.data();
}

uint16_t tud_hid_get_report_cb(uint8_t, uint8_t, hid_report_type_t, uint8_t*, uint16_t) { return 0; }

// The PC sets the keyboard LEDs (Caps Lock etc.) with an output report.
void tud_hid_set_report_cb(uint8_t, uint8_t report_id, hid_report_type_t report_type, const uint8_t* buffer,
                           uint16_t bufsize) {
    if (report_type != HID_REPORT_TYPE_OUTPUT || bufsize == 0) return;
    // Single report without ID: the LED byte is buffer[0]. Some stacks
    // prepend the ID, so take the last byte in that case.
    Router::instance().setLeds(report_id ? buffer[bufsize - 1] : buffer[0]);
}

}  // extern "C"

// --- HidDevice -----------------------------------------------------------------------

HidDevice& HidDevice::instance() {
    static HidDevice device;
    return device;
}

void HidDevice::init() {
    pico_get_unique_board_id_string(serial, sizeof serial);
    tud_init(BOARD_TUD_RHPORT);
}

void HidDevice::task() { tud_task(); }

bool HidDevice::ready() const { return tud_hid_ready(); }
bool HidDevice::suspended() const { return tud_suspended(); }
void HidDevice::wakeup() { tud_remote_wakeup(); }

void HidDevice::sendKeyboard(const KeyboardReport& report) {
    tud_hid_keyboard_report(0, report[0], &report[2]);
}
