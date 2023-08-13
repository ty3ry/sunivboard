FB_TEST_VERSION = 1.0
FB_TEST_SITE = $(BR2_EXTERNAL_SBCBOARD_PATH)/package/fb_test/src
FB_TEST_SITE_METHOD = local

FB_TEST_SECTION = usb
FB_TEST_DESCRIPTION = Framebuffer test implementation

define FB_TEST_BUILD_CMDS
	$(TARGET_CC) -o $(@D)/fb_test $(@D)/fb_test.c
endef

define FB_TEST_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/fb_test $(TARGET_DIR)/usr/bin/fb_test
endef

$(eval $(generic-package))