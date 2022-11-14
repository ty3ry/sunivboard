#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include "key_map.h"
#include "usb_hid_kbd.h"

int usbhid_open()
{
    int fd;
    usleep(500000);

    if ((fd = open(DEVICE_NAME, O_RDWR, 0666)) == -1)
    {
        perror("open");
        return -1;
    }
    return fd;
}

int usbhid_read(int fd, char *buffer, int length)
{
    int n = read(fd, buffer, length);
    return n;
}

int usbhid_write(int fd, char *buffer, int length)
{
    int n = write(fd, buffer, length);
    return n;
}

void set_keycode(int fd, uint8_t modifier_key_ctrl, uint8_t key_ctrl, uint8_t auto_release)
{
    /**
     * hid report format:
     * [0] = modifier key control
     *      [Right Meta | Right Alt | Right Shift | Right Control | Left Meta | Left Alt | Left Shift | Left Control]
     * [1] = reserved code for OEM
     * [2] = key code . see. https://www.usb.org/sites/default/files/hut1_3_0.pdf
     * [3] = key code
     * [4] = key code
     * [5] = key code
     * [6] = key code
     * [7] = key code 
    */
    char report[8] = {0};

    report[0] = modifier_key_ctrl;
    report[2] = key_ctrl;
    /* press key event */
    usbhid_write(fd, report, 8);

    if (auto_release)
    {
        /* reset report */
        memset(report, 0x0, sizeof(report)); 
        usleep(100000);
        usbhid_write(fd, report, 8); /* release key */
    }
}

//#define CONFIG_SETUP_USB_DRIVER
#ifdef CONFIG_SETUP_USB_DRIVER
int main(int argc, char *argv[])
{
    
    int fd = usbhid_open();
    if (fd < 0)
        exit(1);
    // send message from device (usually embbeded linux device) to host (it can be a PC for example)
    for (int i = 0; i < 80; i++)
    {
        set_keycode(fd, 0, KEY_H);
        sleep(1);
    }
}
#endif
