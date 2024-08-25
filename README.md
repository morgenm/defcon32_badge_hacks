# Defcon32 Badge Hacks
Some custom firmware modifications I made to the Defcon 32 Badge. Adds some hacking features to make the badge into a portable hacking tool.

## Features
- [x] USB Ducky Script Support
- [ ] Load Ducky Scripts from SD card
- [ ] Composite USB and HID for Ducky Scripts?
- [ ] I2C over SAO
- [ ] IR Hacking?

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

This will give you `DEFCON32.uf2`, which can be flashed to the badge.

To flash the badge, you can do the following:
1. With the screen facing upright, hold the top-left button on the back of the badge.
2. You should see the pico as USB storage. Mount it.
3. Drag the uf2 file to the badge. It will dismount itself when it's done being flashed.

## License 
License for non-commerical use from the original developer:
![License](license.png)