ALSA_TEST_VERSION = 1.0
ALSA_TEST_SITE = $(BR2_EXTERNAL_SUNIVBOARD_PATH)/package/alsa_test/src
ALSA_TEST_SITE_METHOD = local

ALSA_TEST_SECTION = audio
ALSA_TEST_DESCRIPTION = Advanced Linux Sound Architecture plugins
ALSA_TEST_OPKG_DEPENDENCIES = alsa-lib,libasound
ALSA_TEST_DEPENDENCIES = alsa-lib

define ALSA_TEST_BUILD_CMDS
	$(TARGET_CC) -o $(@D)/alsa_test $(@D)/show_type_format.c -lasound
endef

define ALSA_TEST_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/alsa_test $(TARGET_DIR)/usr/bin/alsa_test
endef

$(eval $(generic-package))