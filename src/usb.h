<<<<<<< HEAD
#pragma once

#include <stdbool.h>

void usbHIDInit();
void usbTask();
bool isUsbMounted();

bool usbDuckyTest();
=======
/*#include "tusb.h"

#define USB_VID           0xCafe  // Replace with your Vendor ID
#define USB_PID           0x4002  // Replace with your Product ID
#define USB_BCD           0x0200  // USB 2.0

tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = USB_BCD,
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100, // Device version 1.00

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *) &desc_device;
}
*/
>>>>>>> master
