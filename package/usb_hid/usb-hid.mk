USB_HID_VERSION = 1.0
USB_HID_SITE = $(BR2_EXTERNAL_SUNIVBOARD_PATH)/package/usb_hid/src
USB_HID_SITE_METHOD = local

USB_HID_SECTION = usb
USB_HID_DESCRIPTION = USB HID test implementation
#USB_HID_OPKG_DEPENDENCIES = alsa-lib,libasound

define USB_HID_BUILD_CMDS
	$(TARGET_CC) -o $(@D)/usb_hid $(@D)/usb_hid.c
endef

define USB_HID_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/usb_hid $(TARGET_DIR)/usr/bin/usb_hid
endef

$(eval $(generic-package))