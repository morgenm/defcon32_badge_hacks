/*
	(c) 2021 Dmitry Grinberg   https://dmitry.gr
	Non-commercial use only OR licensing@dmitry.gr
*/

#ifndef _TIMEBASE_H_
#define _TIMEBASE_H_

#include <stdint.h>


void timebaseInit(void);
uint64_t getTime(void);

void delayMsec(uint32_t msec);
void delayUsec(uint32_t usec);

#define TICKS_PER_SECOND 125000000U

#endif