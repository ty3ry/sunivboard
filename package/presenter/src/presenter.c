#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <sys/select.h>
#include <poll.h>

#include "key_map.h"
#include "ir_recv.h"
#include "usb_hid_kbd.h"

unsigned char *device;
static uint8_t key_modifier = 0;

int main(int argc, char **argv)
{
    int fd, fd_hid;
    struct irc code;

    fd = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
    /* Let's open our input device */
    if (fd < 0)
    {
        fprintf(stderr, "Error opening %s for reading", device);
        exit(EXIT_FAILURE);
    }

    fd_hid = open(DEVICE_NAME, O_RDWR, 0666);//usbhid_open();
    /** open fd hid keyboard */
    if ( fd_hid < 0 ) {
        fprintf(stderr, "Error open hid device \n", DEVICE_NAME);
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        code = scan_ir_code(fd);

        switch (code.event) {
        case IR_EV_IDLE:
        break;

        case IR_EV_SHORTPRESS:
            printf("[Short Press] Value: %d \n", code.value);
            switch(code.value){
            /** ESC */
            case KEY_POWER: set_keycode(fd_hid, key_modifier, HID_KEY_ESC, 1); break;
            case KEY_NUMERIC_0: set_keycode(fd_hid, key_modifier, HID_KEY_A, 1); break;
            case KEY_NUMERIC_1: set_keycode(fd_hid, key_modifier, HID_KEY_B, 1); break;
            case KEY_NUMERIC_2: set_keycode(fd_hid, key_modifier, HID_KEY_C, 1); break;
            case KEY_NUMERIC_3: set_keycode(fd_hid, key_modifier, HID_KEY_D, 1); break;
            case KEY_NUMERIC_4: set_keycode(fd_hid, key_modifier, HID_KEY_E, 1); break;
            case KEY_NUMERIC_5: set_keycode(fd_hid, key_modifier, HID_KEY_F, 1); break;
            case KEY_NUMERIC_6: set_keycode(fd_hid, key_modifier, HID_KEY_G, 1); break;
            case KEY_NUMERIC_7: set_keycode(fd_hid, key_modifier, HID_KEY_H, 1); break;
            case KEY_NUMERIC_8: set_keycode(fd_hid, key_modifier, HID_KEY_I, 1); break;
            case KEY_NUMERIC_9: set_keycode(fd_hid, key_modifier, HID_KEY_J, 1); break;

            case KEY_PLAYPAUSE: set_keycode(fd_hid, key_modifier, HID_KEY_ENTER, 1); break;
            case KEY_BLUETOOTH: set_keycode(fd_hid, key_modifier, HID_KEY_F11, 1); break;
            case KEY_NEXT:      set_keycode(fd_hid, key_modifier, HID_KEY_RIGHT, 1); break;
            case KEY_PREVIOUS:  set_keycode(fd_hid, key_modifier, HID_KEY_LEFT, 1); break;
            case KEY_VOLUMEUP:  set_keycode(fd_hid, key_modifier, HID_KEY_UP, 1); break;
            case KEY_VOLUMEDOWN:    set_keycode(fd_hid, key_modifier, HID_KEY_DOWN, 1); break;
            case KEY_MEMO:          
                set_keycode(fd_hid, key_modifier, HID_KEY_TAB, 1);
                break;

            /* present current slide */
            case KEY_MODE: set_keycode(fd_hid, RIGHT_SHIFT, HID_KEY_F5, 1); break;
            /* light as Alt (right) */
            case KEY_LIGHTS_TOGGLE:  
                key_modifier |= (RIGHT_SHIFT);
                break;
            case KEY_MUTE:  
                key_modifier |= (RIGHT_ALT);
                set_keycode(fd_hid, 0, HID_KEY_RIGHTALT, 0);
                break;
            default: break;
            }
        break;

        case IR_EV_PRESSHOLD:
            printf("[Long  Press] Value: %d \n", code.value);

            switch(code.value){
            /* light as Alt (right) */
            case KEY_LIGHTS_TOGGLE:  
                key_modifier &= ~(RIGHT_SHIFT);
                break;
            case KEY_MUTE:  
                key_modifier &= ~(RIGHT_ALT);
                break;
            
            case KEY_NEXT:      set_keycode(fd_hid, key_modifier, HID_KEY_RIGHT, 1); break;
            case KEY_PREVIOUS:  set_keycode(fd_hid, key_modifier, HID_KEY_LEFT, 1); break;
            case KEY_VOLUMEUP:  set_keycode(fd_hid, key_modifier, HID_KEY_UP, 1); break;
            case KEY_VOLUMEDOWN:    set_keycode(fd_hid, key_modifier, HID_KEY_DOWN, 1); break;

            default : break;
            }

        break;

        
        }

    }
    
    close(fd);
    close(fd_hid);

    return EXIT_SUCCESS;
}