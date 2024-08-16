# Defcon32 Badge Hacks
Some custom firmware modifications I made to the Defcon 32 Badge. Adds some hacking features to make the badge into a portable hacking tool.

## Planned Features
- Basic Enumeration via USB: Enumerate some basic device info via a USB connection.
- USB Rubber Ducky Script Support: Turn your badge into a USB Rubber Ducky and run Ducky scripts via USB.
- USB-C Ethernet Hacking?

## About Firmware
The firmware was originally developed by dmitrygr for Defcon 32. This is a fork of the 1.5.0 firmware from [dmitrygr's site](http://dmitry.gr/).

## Compiling
Run the following to compile the firmware:

```bash
make uGB.bin
```

To flash the badge, you can do the following:
1. Copy `uGB.bin` to the device's SD card.
2. Rename the firmware file to `FIRMWARE.BIN` in the root directory of the SD card.
3. Boot the device and select `Firmware Update` in the device menu.
4. Once the status bar appears to be almost full, wait a few seconds, then reboot the device.

## License 
License for non-commerical use from the original developer:
![License](license.png)