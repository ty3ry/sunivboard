################################################################################
#
# cspot custom package
# c_e - 20221018
#
################################################################################

CSPOT_VERSION = v0.20.0
CSPOT_SITE = $(BR2_EXTERNAL_SUNIVBOARD_PATH)/package/cspot/src
CSPOT_SITE_METHOD = local
CSPOT_DEPENDENCIES = alsa-lib avahi
CSPOT_LICENSE = GPL-3.0+
CSPOT_LICENSE_FILES = LICENSE
CSPOT_SUPPORTS_IN_SOURCE_BUILD = NO
CSPOT_SUBDIR = targets
#CSPOT_INSTALL_STAGING = YES

ifeq ($(BR2_PACKAGE_CSPOT_ALSA),y)
CSPOT_CONF_OPTS = -DUSE_ALSA=ON
endif

#define CSPOT_INSTALL_CONFIG
define CSPOT_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/targets/buildroot-build/cspotcli $(TARGET_DIR)/usr/bin/cspotcli
endef

CSPOT_POST_INSTALL_TARGET_HOOKS += CSPOT_INSTALL_CONFIG

$(eval $(cmake-package))
