#include "pio/pinoutRp2350defcon.h"
#include "timebase.h"
#include "util/printf.h"
#include "sleep.h"
#include "2350.h"
#include "ui.h"

void doSleep(void)
{
	sio_hw->gpio_clr = 1 << PIN_SELF_PWR;
}

