#include <stdlib.h>

#include "usb.h"
#include "tusb.h"
#include "bsp/board_api.h"
#include "usb_descriptors.h"
#include "hid_keycodes.h"
#include "ducky.h"
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
    board_delay(5); // 5 ms delay

    if(keycode == HID_KEY_SPACE) {
        // Add more board delay for space key.
        // Not sure why, but it breaks input without it.
        board_delay(20); 
    }

    // Send empty report to release all keys
    send_hid_report(REPORT_ID_KEYBOARD, 0, 0);
    board_delay(5); // 5 ms delay

    if(keycode == HID_KEY_SPACE) {
        // Add more board delay for space key.
        // Not sure why, but it breaks input without it.
        board_delay(20); 
    }
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


bool usbDuckyTest() {
    uint8_t *keycodes = malloc(MAX_SCRIPT_KEYCODES * sizeof(uint8_t));
    uint8_t *mods = malloc(MAX_SCRIPT_KEYCODES * sizeof(uint8_t));
    int numKeycodes = scriptToKeyPresses(example_script, &keycodes, &mods);

    if(numKeycodes == -1) {
        printf("Error parsing script\n");
        return false;
    }

    for (int i = 0; i < numKeycodes; i++) {
        if(keycodes[i] == DELAY_CODE) {
            //board_delay(keycodes[i+1]);
            continue;
        }
        usbSendKey(keycodes[i], mods[i]);
        //uint8_t keycode = 0, mod = 0;
        //asciiToKeyCode('a', &keycode, &mod);
        //usbSendKey(keycode, mod);
        
    }

    free(keycodes);
    free(mods);
    return true;
}