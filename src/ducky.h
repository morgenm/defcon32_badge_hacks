#pragma once

#define DELAY_CODE 255
#define MAX_SCRIPT_KEYCODES 2048

extern char example_script[];

int scriptToKeyPresses(char *script, uint8_t **keycodes, uint8_t **mods);