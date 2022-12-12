################################################################################
#
# oled custom package
# c_e - 20221212
#
################################################################################

OLED_VERSION = v0.20.0
OLED_SITE = $(BR2_EXTERNAL_SBCBOARD_PATH)/package/oled/src
OLED_SITE_METHOD = local
OLED_DEPENDENCIES = linux-headers
OLED_LICENSE = GPL-3.0+
OLED_LICENSE_FILES = LICENSE
OLED_SUPPORTS_IN_SOURCE_BUILD = NO
OLED_SUBDIR = .

# ifeq ($(BR2_PACKAGE_CSPOT_ALSA),y)
# CSPOT_CONF_OPTS = -DUSE_ALSA=ON
# endif

define OLED_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/buildroot-build/oled_test $(TARGET_DIR)/usr/bin/oled_test
endef

OLED_POST_INSTALL_TARGET_HOOKS += OLED_INSTALL_CONFIG

$(eval $(cmake-package))
