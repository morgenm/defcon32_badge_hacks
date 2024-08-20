#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "timebase.h"

#define SYSTICK_BITS 24

static uint32_t mTicks = 0;

bool timer_callback(repeating_timer_t *rt) {
    mTicks++;
    return true; // Return true to keep repeating
}

void timebaseInit(void) {
    static repeating_timer_t timer;
    add_repeating_timer_ms(1, timer_callback, NULL, &timer);
}

uint64_t getTime(void) {
    uint32_t hi, lo;

    do {
        hi = mTicks;
        asm volatile("":::"memory");
        lo = time_us_32();
        asm volatile("":::"memory");
    } while (hi != mTicks);

    return (((uint64_t)hi) << SYSTICK_BITS) + (((1 << SYSTICK_BITS) - 1) - lo);
}

void delayMsec(uint32_t msec) {
    uint64_t till = getTime() + (uint64_t)msec * (TICKS_PER_SECOND / 1000);
    while (getTime() < till);
}

void delayUsec(uint32_t usec) {
    uint64_t till = getTime() + (uint64_t)usec * (TICKS_PER_SECOND / 1000000);
    while (getTime() < till);
}