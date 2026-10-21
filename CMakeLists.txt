cmake_minimum_required(VERSION 3.20)

set(PICO_BOARD pico2_w CACHE STRING "Pico board")

# --- Pico SDK ------------------------------------------------------------------
# Fetched before project() so the SDK can pick the Arm cross-toolchain.
include(FetchContent)
FetchContent_Declare(
    pico_sdk
    GIT_REPOSITORY https://github.com/raspberrypi/pico-sdk.git
    GIT_TAG 2.3.1
    GIT_SHALLOW TRUE
    # Only the submodules we need: TinyUSB and the CYW43 (Wi-Fi chip) driver
    GIT_SUBMODULES lib/tinyusb lib/cyw43-driver
    GIT_SUBMODULES_RECURSE TRUE
    # Download only; the SDK is imported manually below
    SOURCE_SUBDIR _fetch_only
)
FetchContent_MakeAvailable(pico_sdk)
set(PICO_SDK_PATH "${pico_sdk_SOURCE_DIR}")
set(PICO_TINYUSB_PATH "${PICO_SDK_PATH}/lib/tinyusb")
include("${PICO_SDK_PATH}/external/pico_sdk_import.cmake")

project(pico_hid_bridge C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# A system-wide picotool with a different version makes the SDK abort.
# Build the version matching the SDK instead; pass -DPICOTOOL_FORCE_FETCH_FROM_GIT=OFF
# to use an installed picotool.
set(PICOTOOL_FORCE_FETCH_FROM_GIT ON CACHE BOOL "Always build the picotool matching the SDK")

pico_sdk_init()

# --- Pico-PIO-USB --------------------------------------------------------------
# Software USB host on two GPIOs (PIO), used for the keyboard.
FetchContent_Declare(
    pico_pio_usb
    GIT_REPOSITORY https://github.com/sekigon-gonnoc/Pico-PIO-USB.git
    GIT_TAG 0.7.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(pico_pio_usb)

# --- Firmware ------------------------------------------------------------------
add_executable(kb_bridge
    src/main.cpp
    src/usb_device/hid_device.cpp
    src/usb_host/kbd_host.cpp
    src/router/router.cpp
)

target_include_directories(kb_bridge PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# TinyUSB's host driver that glues the host stack to Pico-PIO-USB.
set(TUSB_PIO_USB_DIR "${PICO_TINYUSB_PATH}/src/portable/raspberrypi/pio_usb")
if (NOT EXISTS "${TUSB_PIO_USB_DIR}/hcd_pio_usb.c")
    message(FATAL_ERROR "TinyUSB has no PIO-USB host driver at ${TUSB_PIO_USB_DIR}")
endif()
target_sources(kb_bridge PRIVATE "${TUSB_PIO_USB_DIR}/hcd_pio_usb.c")

target_link_libraries(kb_bridge PRIVATE
    pico_stdlib
    pico_multicore
    pico_unique_id
    tinyusb_device      # native USB port: Pico acts as a device towards the PC
    tinyusb_host        # host stack: used with PIO-USB for the keyboard
    tinyusb_board
    pico_pio_usb
    pico_cyw43_arch_none  # onboard LED on W boards
)

# The native USB port is used by the HID device, so debug output goes over UART.
pico_enable_stdio_usb(kb_bridge 0)
pico_enable_stdio_uart(kb_bridge 1)

# Generate .uf2 (for drag-and-drop flashing)
pico_add_extra_outputs(kb_bridge)
