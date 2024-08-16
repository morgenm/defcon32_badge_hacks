#ifndef TUSB_CONFIG_H_
#define TUSB_CONFIG_H_

#define CFG_TUSB_MCU          OPT_MCU_RP2040

// Enable the USB Device stack
#define CFG_TUSB_DEVICE_ENABLED   1
#define CFG_TUD_ENABLED 1

// Enable HID class driver
#define CFG_TUSB_DEVICE_HID       1

// Enable CDC class driver
#define CFG_TUSB_DEVICE_CDC       1

// Enable MSC class driver (if needed)
#define CFG_TUSB_DEVICE_MSC       0

// Enable MTP class driver (if needed)
#define CFG_TUSB_DEVICE_MTP       0

// Enable other class drivers as needed
#define CFG_TUSB_DEVICE_RNDIS     0
#define CFG_TUSB_DEVICE_VENDOR    0

// USB string descriptors
#define CFG_TUSB_DESCRIPTOR_STRINGS 1

// Enable USB Serial (CDC) support
#define CFG_TUSB_DEVICE_CDC_EP_BUFSIZE 64

// Enable USB HID (Human Interface Device) support
#define CFG_TUSB_DEVICE_HID_EP_BUFSIZE 64

// Enable other necessary features here
// ...

#endif /* TUSB_CONFIG_H_ */
