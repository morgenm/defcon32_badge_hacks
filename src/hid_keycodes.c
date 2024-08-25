//#include "tusb.h"
#include "hid_keycodes.h"
#include "tusb.h"

// Function to convert ASCII to TinyUSB HID keycode
void asciiToKeyCode(char c, uint8_t *keycode, uint8_t *mod) {
    const uint8_t ascii_to_keycode[128][2] = {
        // 0-31: Control characters (not used)
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
        // 32-47: Space and punctuation
        {HID_KEY_SPACE, 0}, {HID_KEY_1, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_APOSTROPHE, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_3, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_4, KEYBOARD_MODIFIER_LEFTSHIFT}, 
        {HID_KEY_5, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_7, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_APOSTROPHE, 0}, {HID_KEY_9, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_0, KEYBOARD_MODIFIER_LEFTSHIFT},
        {HID_KEY_8, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_EQUAL, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_COMMA, 0}, {HID_KEY_MINUS, 0}, {HID_KEY_PERIOD, 0}, {HID_KEY_SLASH, 0},
        // 48-57: Numbers 0-9
        {HID_KEY_0, 0}, {HID_KEY_1, 0}, {HID_KEY_2, 0}, {HID_KEY_3, 0}, {HID_KEY_4, 0}, 
        {HID_KEY_5, 0}, {HID_KEY_6, 0}, {HID_KEY_7, 0}, {HID_KEY_8, 0}, {HID_KEY_9, 0},
        // 58-64: Punctuation
        {HID_KEY_SEMICOLON, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_SEMICOLON, 0}, {HID_KEY_COMMA, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_EQUAL, 0}, {HID_KEY_PERIOD, KEYBOARD_MODIFIER_LEFTSHIFT}, 
        {HID_KEY_SLASH, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_2, KEYBOARD_MODIFIER_LEFTSHIFT},
        // 65-90: Uppercase letters A-Z
        {HID_KEY_A, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_B, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_C, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_D, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_E, KEYBOARD_MODIFIER_LEFTSHIFT},
        {HID_KEY_F, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_G, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_H, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_I, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_J, KEYBOARD_MODIFIER_LEFTSHIFT}, 
        {HID_KEY_K, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_L, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_M, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_N, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_O, KEYBOARD_MODIFIER_LEFTSHIFT}, 
        {HID_KEY_P, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_Q, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_R, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_S, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_T, KEYBOARD_MODIFIER_LEFTSHIFT},
        {HID_KEY_U, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_V, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_W, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_X, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_Y, KEYBOARD_MODIFIER_LEFTSHIFT}, 
        {HID_KEY_Z, KEYBOARD_MODIFIER_LEFTSHIFT},
        // 91-96: Punctuation
        {HID_KEY_BRACKET_LEFT, 0}, {HID_KEY_BACKSLASH, 0}, {HID_KEY_BRACKET_RIGHT, 0}, {HID_KEY_6, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_MINUS, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_GRAVE, KEYBOARD_MODIFIER_LEFTSHIFT},
        // 97-122: Lowercase letters a-z
        {HID_KEY_A, 0}, {HID_KEY_B, 0}, {HID_KEY_C, 0}, {HID_KEY_D, 0}, {HID_KEY_E, 0}, 
        {HID_KEY_F, 0}, {HID_KEY_G, 0}, {HID_KEY_H, 0}, {HID_KEY_I, 0}, {HID_KEY_J, 0}, 
        {HID_KEY_K, 0}, {HID_KEY_L, 0}, {HID_KEY_M, 0}, {HID_KEY_N, 0}, {HID_KEY_O, 0}, 
        {HID_KEY_P, 0}, {HID_KEY_Q, 0}, {HID_KEY_R, 0}, {HID_KEY_S, 0}, {HID_KEY_T, 0}, 
        {HID_KEY_U, 0}, {HID_KEY_V, 0}, {HID_KEY_W, 0}, {HID_KEY_X, 0}, {HID_KEY_Y, 0}, 
        {HID_KEY_Z, 0},
        // 123-127: Punctuation
        {HID_KEY_BRACKET_LEFT, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_BACKSLASH, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_BRACKET_RIGHT, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_GRAVE, KEYBOARD_MODIFIER_LEFTSHIFT}, {HID_KEY_DELETE, 0}
    };

    *keycode = ascii_to_keycode[(uint8_t)c][0];
    *mod = ascii_to_keycode[(uint8_t)c][1];
}