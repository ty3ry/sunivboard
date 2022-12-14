################################################################################
#
# oled player
# c_e - 20221214
#
################################################################################

OLED_PLAYER_VERSION = v0.1
OLED_PLAYER_SITE = $(BR2_EXTERNAL_SBCBOARD_PATH)/package/oled_player/src
OLED_PLAYER_SITE_METHOD = local
OLED_PLAYER_DEPENDENCIES = linux-headers
OLED_PLAYER_LICENSE = GPL-3.0+
OLED_PLAYER_LICENSE_FILES = LICENSE
OLED_PLAYER_SUPPORTS_IN_SOURCE_BUILD = NO
OLED_PLAYER_SUBDIR = .

define OLED_PLAYER_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/buildroot-build/oled_player $(TARGET_DIR)/usr/bin/oled_player
endef

OLED_PLAYER_POST_INSTALL_TARGET_HOOKS += OLED_INSTALL_CONFIG

$(eval $(cmake-package))
