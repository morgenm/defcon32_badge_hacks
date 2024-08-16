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
mkdir build
cd build
cmake ..
make
```

This will give you `FIRMWARE.BIN`, which can be flashed to the badge.

To flash the badge, you can do the following:
1. Copy `FIRMWARE.BIN` to the badge's SD card. Make sure it's in the root directory of the card.
2. Boot the device and select `Firmware Update` in the device menu.
3. Once the status bar appears to be almost full, wait a few seconds, then reboot the device.

## License 
License for non-commerical use from the original developer:
![License](license.png)