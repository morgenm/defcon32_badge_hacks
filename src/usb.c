#include "usb.h"
#include "tusb.h"
#include "bsp/board_api.h"
#include "usb_descriptors.h"
//#include "pico/mutex.h"

//bool usbMounted = false;
//mutex_t usbMutex;

// Invoked when device is mounted
void tud_mount_cb(void) {
    //mutex_enter_blocking(&usbMutex);
    //usbMounted = true;
    //mutex_exit(&usbMutex);
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
    //mutex_enter_blocking(&usbMutex); 
    //usbMounted = false;
    //mutex_exit(&usbMutex); 
}

bool isUsbMounted() {
    //bool mounted;
    //mutex_enter_blocking(&usbMutex);  // Lock the mutex
    //mounted = usbMounted;
    //mutex_exit(&usbMutex);  // Unlock the mutex
    //return mounted;
    return tud_ready();
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
  
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
    //mutex_enter_blocking(&usbMutex); 
    //usbMounted = tud_mounted();
    //mutex_exit(&usbMutex); 
}

void usbHIDInit() {
    board_init();

    //mutex_init(&usbMutex);

    // init device stack on configured roothub port
    tud_init(BOARD_TUD_RHPORT);

    if (board_init_after_tusb) {
        board_init_after_tusb();
    }
}

void usbTask() {
    tud_task();
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;

  //uint8_t next_report_id = report[0] + 1u;

  /*if (next_report_id < REPORT_ID_COUNT)
  {
    send_hid_report(next_report_id, );
  }*/
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
}

// Function to convert ASCII to TinyUSB HID keycode
static void asciiToKeyCode(char c, uint8_t *keycode, uint8_t *mod) {
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

static void send_hid_report(uint8_t report_id, uint32_t mod, uint32_t key)
{
    if (!tud_hid_ready()) return;

    uint8_t keycode[6] = {0};
    keycode[0] = key;

    // Send key press
    tud_hid_keyboard_report(report_id, mod, keycode);

    // Add a small delay to simulate keypress duration
    board_delay(10); // 10ms delay (adjust as needed)

    // Send key release
    keycode[0] = 0;
    tud_hid_keyboard_report(report_id, mod, keycode);
}

void usbSendKey(uint8_t keycode, uint8_t modifier) {
    send_hid_report(REPORT_ID_KEYBOARD, modifier, keycode);
}

void usbSendString(const char *str) {
    while (*str) {
        uint8_t keycode;
        uint8_t modifier;

        // Convert ASCII to HID keycode and modifier
        asciiToKeyCode(*str, &keycode, &modifier);

        // Send the keystroke
        usbSendKey(keycode, modifier);

        // Move to the next character
        str++;

        // Add a small delay between keystrokes
        board_delay(5); // 5ms delay (adjust as needed)
    }
}


void usbDuckyTest() {
    const char *s = "Hello world!!!!!";
    usbSendString(s);
}