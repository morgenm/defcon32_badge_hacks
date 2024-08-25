#pragma once

#include <stdbool.h>

void usbHIDInit();
void usbTask();
bool isUsbMounted();

bool usbDuckyTest();