################################################################################
#
# lvgl custom package
#
################################################################################

LVGL_TEST_VERSION = 1.0
LVGL_TEST_SITE = $(BR2_EXTERNAL_SBCBOARD_PATH)/package/lvgl_test/src
LVGL_TEST_SITE_METHOD = local

LVGL_TEST_DESCRIPTION = LVGL test implementation

define LVGL_TEST_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) all
endef

define LVGL_TEST_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/lvgl_test $(TARGET_DIR)/usr/bin/lvgl_test
endef

$(eval $(generic-package))
