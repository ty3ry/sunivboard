/***
 * reserved
*/
#ifndef USB_HID_KBD_H
#define USB_HID_KBD_H

#define DEVICE_NAME "/dev/hidg0"

int usbhid_open();
void set_keycode(int fd, uint8_t modifier_key_ctrl, uint8_t key_ctrl, uint8_t auto_release);

#endif /* USB_HID_KBD_H*/