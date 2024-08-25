#include "input.h"

#include "pio/pinoutRp2350defcon.h"

uint_fast8_t prvKeysMap(uint32_t sta)
{
	uint_fast8_t ret = 0;
	
	if (!(sta & (1 << PIN_BTN_U)))		ret |= KEY_BIT_UP;
	if (!(sta & (1 << PIN_BTN_D)))		ret |= KEY_BIT_DOWN;
	if (!(sta & (1 << PIN_BTN_L)))		ret |= KEY_BIT_LEFT;
	if (!(sta & (1 << PIN_BTN_R)))		ret |= KEY_BIT_RIGHT;
	
	if (!(sta & (1 << PIN_BTN_A)))		ret |= KEY_BIT_A;
	if (!(sta & (1 << PIN_BTN_B)))		ret |= KEY_BIT_B;
	if (!(sta & (1 << PIN_BTN_START)))	ret |= KEY_BIT_START;
	if (!(sta & (1 << PIN_BTN_SEL)))	ret |= KEY_BIT_SEL;
		
	return ret;
}