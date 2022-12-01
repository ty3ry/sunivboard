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
# /home/softwareml/frame1bit/embedded-linux/sunxi/sunivboard/package/cspot/src/targets/cli/build
ifeq ($(BR2_PACKAGE_CSPOT_ALSA),y)
CSPOT_CONF_OPTS = -DUSE_ALSA=ON
endif


$(eval $(cmake-package))
