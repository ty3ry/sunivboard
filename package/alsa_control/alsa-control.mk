ALSA_CONTROL_VERSION = 1.0
ALSA_CONTROL_SITE = $(BR2_EXTERNAL_SBCBOARD_PATH)/package/alsa_control/src
ALSA_CONTROL_SITE_METHOD = local

ALSA_CONTROL_SECTION = audio
ALSA_CONTROL_DESCRIPTION = Advanced Linux Sound Architecture plugins
ALSA_CONTROL_OPKG_DEPENDENCIES = alsa-lib,libasound
ALSA_CONTROL_DEPENDENCIES = alsa-lib

define ALSA_CONTROL_BUILD_CMDS
	$(TARGET_CC) -o $(@D)/alsa_control $(@D)/alsa_control.c -lasound
endef

define ALSA_CONTROL_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/alsa_control $(TARGET_DIR)/usr/bin/alsa_control
endef

$(eval $(generic-package))