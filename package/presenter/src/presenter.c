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
            case KEY_POWER:
                printf("KEY_POWER\n");
                set_keycode(fd_hid, 0, HID_KEY_POWER);
                break;
            case KEY_NUMERIC_0: set_keycode(fd_hid, 0, HID_KEY_A); break;
            case KEY_NUMERIC_1: set_keycode(fd_hid, 0, HID_KEY_B); break;
            case KEY_NUMERIC_2: set_keycode(fd_hid, 0, HID_KEY_C); break;
            case KEY_NUMERIC_3: set_keycode(fd_hid, 0, HID_KEY_D); break;
            case KEY_NUMERIC_4: set_keycode(fd_hid, 0, HID_KEY_E); break;
            case KEY_NUMERIC_5: set_keycode(fd_hid, 0, HID_KEY_F); break;
            case KEY_NUMERIC_6: set_keycode(fd_hid, 0, HID_KEY_G); break;
            case KEY_NUMERIC_7: set_keycode(fd_hid, 0, HID_KEY_H); break;
            case KEY_NUMERIC_8: set_keycode(fd_hid, 0, HID_KEY_I); break;
            case KEY_NUMERIC_9: set_keycode(fd_hid, 0, HID_KEY_J); break;
            default: break;
            }
        break;

        case IR_EV_PRESSHOLD:
        printf("[Long  Press] Value: %d \n", code.value);
        break;
        }

    }
    
    close(fd);
    close(fd_hid);

    return EXIT_SUCCESS;
}