#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// Native USB (micro USB -> PC) is the device on roothub port 0,
// PIO-USB (GP27/GP28 -> keyboard) is the host on roothub port 1.
#define BOARD_TUD_RHPORT 0
#define BOARD_TUH_RHPORT 1

#define CFG_TUD_ENABLED 1
#define CFG_TUH_ENABLED 1
#define CFG_TUH_RPI_PIO_USB 1

#define CFG_TUSB_DEBUG 0

#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))

// --- Device (Pico -> PC): one HID keyboard -----------------------------------
#define CFG_TUD_ENDPOINT0_SIZE 64
#define CFG_TUD_HID 1
#define CFG_TUD_HID_EP_BUFSIZE 16
#define CFG_TUD_CDC 0
#define CFG_TUD_MSC 0
#define CFG_TUD_MIDI 0
#define CFG_TUD_VENDOR 0

// --- Host (keyboard -> Pico) ----------------------------------------------------
#define CFG_TUH_ENUMERATION_BUFSIZE 256
#define CFG_TUH_HUB 1
#define CFG_TUH_DEVICE_MAX (CFG_TUH_HUB ? 4 : 1)
#define CFG_TUH_HID 4
#define CFG_TUH_HID_EPIN_BUFSIZE 64
#define CFG_TUH_HID_EPOUT_BUFSIZE 64
#define CFG_TUH_CDC 0
#define CFG_TUH_MSC 0
#define CFG_TUH_VENDOR 0

#ifdef __cplusplus
}
#endif

#endif
