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

#include "usb_hid.h"

#define DEVICE_NAME "/dev/hidg0"

static char keyb_report_desc[] = {

    0x05, 0x01, // Usage Page (Generic Desktop Ctrls)
    0x09, 0x06, // Usage (Keyboard)
    0xA1, 0x01, // Collection (Application)
    0x05, 0x07, //   Usage Page (Kbrd/Keypad)
    0x19, 0xE0, //   Usage Minimum (0xE0)
    0x29, 0xE7, //   Usage Maximum (0xE7)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x01, //   Logical Maximum (1)
    0x75, 0x01, //   Report Size (1)
    0x95, 0x08, //   Report Count (8)
    0x81, 0x02, //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01, //   Report Count (1)
    0x75, 0x08, //   Report Size (8)
    0x81, 0x03, //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x05, //   Report Count (5)
    0x75, 0x01, //   Report Size (1)
    0x05, 0x08, //   Usage Page (LEDs)
    0x19, 0x01, //   Usage Minimum (Num Lock)
    0x29, 0x05, //   Usage Maximum (Kana)
    0x91, 0x02, //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x95, 0x01, //   Report Count (1)
    0x75, 0x03, //   Report Size (3)
    0x91, 0x03, //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x95, 0x06, //   Report Count (6)
    0x75, 0x08, //   Report Size (8)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x65, //   Logical Maximum (101)
    0x05, 0x07, //   Usage Page (Kbrd/Keypad)
    0x19, 0x00, //   Usage Minimum (0x00)
    0x29, 0x65, //   Usage Maximum (0x65)
    0x81, 0x00, //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,       // End Collection

    // 63 bytes
};

static int mkpath(char *p)
{
    char *path = strdup(p);
    char *save_path = path;
    char *sep1;
    char *sep2 = 0;
    do
    {
        int idx = (sep2 - path) < 0 ? 0 : sep2 - path;
        sep1 = strchr(path + idx, '/');
        sep2 = strchr(sep1 + 1, '/');
        if (sep2)
        {
            path[sep2 - path] = 0;
        }
        if (mkdir(path, 0777) && errno != EEXIST)
            return -1;
        if (sep2)
        {
            path[sep2 - path] = '/';
        }
    } while (sep2);

    free(save_path);
    return 0;
}
static int mkfile(char *filename, char *value)
{
    int fd = open(filename, O_RDWR);
    if (fd < 0)
        return -1;
    int n = write(fd, value, strlen(value));
    close(fd);
    return n;
}
static int mkfile1(char *filename, char *value, int len)
{
    int fd = open(filename, O_RDWR);
    if (fd < 0)
        return -1;
    int n = write(fd, value, len);
    close(fd);
    return n;
}
static int usbhid_setup()
{
    FILE *pipe;
    char line[128];
    int linenr;

    pipe = popen("modprobe configfs;modprobe libcomposite;", "r");
    if (pipe == NULL)
    {  /* check for errors */
        perror("popen");   /* report error message */
        return -1;          /* return with exit code indicating error */
    }

    /* Read script output from the pipe line by line */
    linenr = 1;
    while (fgets(line, 128, pipe) != NULL)
    {
        printf("Script output line %d: %s", linenr, line);
        +linenr;
    }

    pipe = popen("mount -t configfs none /sys/kernel/config", "r");
    if (pipe == NULL)
    {  /* check for errors */
        perror("popen");   /* report error message */
        return -1;          /* return with exit code indicating error */
    }

    pclose(pipe);

    printf("start config \n");
    mkpath("/sys/kernel/config/usb_gadget/g1");
    mkfile("/sys/kernel/config/usb_gadget/g1/bMaxPacketSize0", "64");
    mkfile("/sys/kernel/config/usb_gadget/g1/bcdUSB", "0x200");
    mkfile("/sys/kernel/config/usb_gadget/g1/idVendor", "0x13bb");
    mkfile("/sys/kernel/config/usb_gadget/g1/idProduct", "0x7801");

    mkpath("/sys/kernel/config/usb_gadget/g1/configs/c1.1/strings/0x409");
    mkfile("/sys/kernel/config/usb_gadget/g1/strings/c1.1/MaxPower", "120");
    mkfile("/sys/kernel/config/usb_gadget/g1/configs/c1.1/strings/0x409/configuration", "Test Device");

    mkpath("/sys/kernel/config/usb_gadget/g1/strings/0x409");
    mkfile("/sys/kernel/config/usb_gadget/g1/strings/0x409/manufacturer", "Keyboard Testing INC");
    mkfile("/sys/kernel/config/usb_gadget/g1/strings/0x409/product", "Polytron");
    mkfile("/sys/kernel/config/usb_gadget/g1/strings/0x409/serialnumber", "0");

#if 0
	// mass storage
	mkpath ("/sys/kernel/config/usb_gadget/g1/functions/mass_storage.ms0/lun.0");
	mkfile ("/sys/kernel/config/usb_gadget/g1/functions/mass_storage.ms0/lun.0/file","/home/root/fat.fs");
	mkfile ("/sys/kernel/config/usb_gadget/g1/functions/mass_storage.ms0/lun.0/removable","1");
	symlink("/sys/kernel/config/usb_gadget/g1/functions/mass_storage.ms0", "/sys/kernel/config/usb_gadget/g1/configs/c1.1/mass_storage.ms0");// functions/mass_storage.ms0->configs/c1.1
#endif

    // hid
    mkpath("/sys/kernel/config/usb_gadget/g1/functions/hid.0");
    mkfile("/sys/kernel/config/usb_gadget/g1/functions/hid.0/protocol", "1");
    mkfile("/sys/kernel/config/usb_gadget/g1/functions/hid.0/subclass", "1");
    mkfile("/sys/kernel/config/usb_gadget/g1/functions/hid.0/report_length", "8");
    mkfile1("/sys/kernel/config/usb_gadget/g1/functions/hid.0/report_desc", keyb_report_desc, sizeof(keyb_report_desc));

    symlink("/sys/kernel/config/usb_gadget/g1/functions/hid.0", "/sys/kernel/config/usb_gadget/g1/configs/c1.1/hid.0"); // functions/hid.0->configs/c1.1

    mkfile("/sys/kernel/config/usb_gadget/g1/UDC", "musb-hdrc.1.auto");
}

int usbhid_diable()
{
    mkfile("/sys/kernel/config/usb_gadget/g1/UDC", "");
}

int usbhid_enable()
{
    mkfile("/sys/kernel/config/usb_gadget/g1/UDC", "musb-hdrc.1.auto");
}

int usbhid_open()
{
    int fd;
    usbhid_setup();
    usleep(500000);

    if ((fd = open(DEVICE_NAME, O_RDWR, 0666)) == -1)
    {
        perror(DEVICE_NAME);
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
char report[8];
#define RIGHT_META  (1<<7) 
#define RIGHT_ALT   (1<<6)
#define RIGHT_SHIFT (1<<5) 
#define RIGHT_CTRL  (1<<4)
#define LEFT_META   (1<<3) 
#define LEFT_ALT    (1<<2) 
#define LEFT_SHIFT  (1<<1)
#define LEFT_CTRL   (1<<0)

void set_keycode(int fd, uint8_t modifier_key_ctrl, uint16_t key_ctrl)
{
    report[0] = modifier_key_ctrl;
    report[2] = key_ctrl;
    /* press key event */
    usbhid_write(fd, report, 8);
    /* reset report */
    memset(report, 0x0, sizeof(report));
    usbhid_write(fd, report, 8); /* release key */
}

#define CONFIG_SETUP_USB_DRIVER
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
