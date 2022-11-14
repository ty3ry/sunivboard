ALSA_TEST_VERSION = 1.0
ALSA_TEST_SITE = $(BR2_EXTERNAL_SUNIVBOARD_PATH)/package/alsa_test/src
ALSA_TEST_SITE_METHOD = local

ALSA_TEST_SECTION = audio
ALSA_TEST_DESCRIPTION = Advanced Linux Sound Architecture plugins
ALSA_TEST_OPKG_DEPENDENCIES = alsa-lib,libasound
ALSA_TEST_DEPENDENCIES = alsa-lib

define ALSA_TEST_BUILD_CMDS
	$(TARGET_CC) -o $(@D)/alsa_test $(@D)/show_type_format.c -lasound
	$(TARGET_CC) -o $(@D)/play_random $(@D)/play_random.c -lasound
	$(TARGET_CC) -o $(@D)/play_raw $(@D)/play_raw.c -lasound
	# $(TARGET_CC) -o $(@D)/simple_mp3_player $(@D)/simple_mp3_player.c -lasound -lmad
	$(TARGET_CC) -o $(@D)/mad_alsa $(@D)/mad_alsa.c -lasound -lmad -lpthread -lid3tag
endef

define ALSA_TEST_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/alsa_test $(TARGET_DIR)/usr/bin/alsa_test
	$(INSTALL) -D -m 755 $(@D)/play_random $(TARGET_DIR)/usr/bin/play_random
	$(INSTALL) -D -m 755 $(@D)/play_raw $(TARGET_DIR)/usr/bin/play_raw
	# $(INSTALL) -D -m 755 $(@D)/simple_mp3_player $(TARGET_DIR)/usr/bin/simple_mp3_player
	$(INSTALL) -D -m 755 $(@D)/mad_alsa $(TARGET_DIR)/usr/bin/mad_alsa
endef

$(eval $(generic-package))