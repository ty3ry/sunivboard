PRESENTER_VERSION = 1.0
PRESENTER_SITE = $(BR2_EXTERNAL_SUNIVBOARD_PATH)/package/presenter/src
PRESENTER_SITE_METHOD = local

PRESENTER_SECTION = ir to usb hid
PRESENTER_DESCRIPTION = presenter using ir 

define PRESENTER_BUILD_CMDS
	$(TARGET_CC) -o $(@D)/presenter $(@D)/presenter.c $(@D)/ir_recv.c $(@D)/usb_hid_kbd.c
endef

define PRESENTER_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/presenter $(TARGET_DIR)/usr/bin/presenter
endef

$(eval $(generic-package))