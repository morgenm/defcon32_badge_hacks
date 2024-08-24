/*
hid_keycodes.h
Defines function for converting ascii chars to tinyusb keycodes.
*/

#pragma once

#include <stdint.h>

void asciiToKeyCode(char c, uint8_t *keycode, uint8_t *mod);