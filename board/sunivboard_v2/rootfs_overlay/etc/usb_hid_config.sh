#!/bin/sh

modprobe configfs
modprobe libcomposite
mount -t configfs none /sys/kernel/config
cd /sys/kernel/config/usb_gadget
mkdir g1
cd g1
echo "64" > bMaxPacketSize0
echo "0x200" > bcdUSB
echo "0x100" > bcdDevice
echo "0x1234" > idVendor
echo "0x5678" > idProduct
mkdir strings/0x409
mkdir configs/c1.1
echo "Demo USB gadget device" > strings/0x409/manufacturer
echo "Product" > strings/0x409/product
echo 0 > strings/0x409/serialnumber
mkdir configs/c1.1/strings/0x409/ -p
echo "Product Configuration " > configs/c1.1/strings/0x409/configuration
echo 120 > configs/c1.1/MaxPower

# emulate hid keyboard
mkdir functions/hid.0
echo 1 > functions/hid.0/protocol                      #  set the HID protocol
echo 1 > functions/hid.0/subclass                      #  set the device subclass
echo 8 > functions/hid.0/report_length                 #  set the byte length of HID reports
#cat "/path/to/stdard/hid/keyabort/report/descripor.bin" > functions/hid.0/report_desc        
ln -s functions/hid.0 configs/c1.1 

# enable the USB device contoller
echo "musb-hdrc.1.auto.usb-otg" > UDC