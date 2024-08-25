#pragma once

#include <stdint.h>

#define KEY_BIT_START	0x80
#define KEY_BIT_SEL		0x40
#define KEY_BIT_B		0x20
#define KEY_BIT_A		0x10
#define KEY_BIT_DOWN	0x08
#define KEY_BIT_UP		0x04
#define KEY_BIT_LEFT	0x02
#define KEY_BIT_RIGHT	0x01

uint_fast8_t prvKeysMap(uint32_t sta);