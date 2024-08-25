#pragma once

#define DELAY_CODE 255
#define MAX_SCRIPT_KEYCODES 2048

extern char example_script[];

int scriptToKeyPresses(char *script, uint32_t **keycodes, uint32_t **mods);