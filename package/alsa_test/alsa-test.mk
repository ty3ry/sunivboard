ALSA_TEST_VERSION = 1.0
ALSA_TEST_SITE = $(BR2_EXTERNAL_SUNIVBOARD_PATH)/package/alsa_test/src
ALSA_TEST_SITE_METHOD = local

ALSA_TEST_SECTION = audio
ALSA_TEST_DESCRIPTION = Advanced Linux Sound Architecture plugins
ALSA_TEST_OPKG_DEPENDENCIES = alsa-lib,libasound
ALSA_TEST_DEPENDENCIES = alsa-lib

define ALSA_TEST_BUILD_CMDS
	$(TARGET_CC) -o $(@D)/alsa_test $(@D)/show_type_format.c -lasound
	$(TARGET_CC) -o $(@D)/alsa_play_random $(@D)/alsa_play_random.c -lasound
	$(TARGET_CC) -o $(@D)/alsa_play_tone $(@D)/alsa_play_tone.c -lasound
	$(TARGET_CC) -o $(@D)/alsa_play_raw $(@D)/alsa_play_raw.c -lasound
	$(TARGET_CC) -o $(@D)/simple_mp3_player $(@D)/simple_mp3_player.c -lasound -lmad
	$(TARGET_CC) -o $(@D)/alsa_mad $(@D)/alsa_mad.c -lasound -lmad -lpthread -lid3tag
	$(TARGET_CC) -o $(@D)/minimad $(@D)/minimad.c -lasound -lmad
	$(TARGET_CC) -o $(@D)/alsa_loop $(@D)/alsa_loop.c -lasound
	$(TARGET_CC) -o $(@D)/alsa_capture $(@D)/alsa_capture.c -lasound
	$(TARGET_CC) -o $(@D)/alsa_device_info $(@D)/alsa_device_info.c -lasound
	$(TARGET_CC) -o $(@D)/playback_capture $(@D)/playback_capture.c -lasound
endef

define ALSA_TEST_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/alsa_test $(TARGET_DIR)/usr/bin/alsa_test
	$(INSTALL) -D -m 755 $(@D)/alsa_play_random $(TARGET_DIR)/usr/bin/alsa_play_random
	$(INSTALL) -D -m 755 $(@D)/alsa_play_tone $(TARGET_DIR)/usr/bin/alsa_play_tone
	$(INSTALL) -D -m 755 $(@D)/alsa_play_raw $(TARGET_DIR)/usr/bin/alsa_play_raw
	$(INSTALL) -D -m 755 $(@D)/simple_mp3_player $(TARGET_DIR)/usr/bin/simple_mp3_player
	$(INSTALL) -D -m 755 $(@D)/alsa_mad $(TARGET_DIR)/usr/bin/alsa_mad
	$(INSTALL) -D -m 755 $(@D)/minimad $(TARGET_DIR)/usr/bin/minimad
	$(INSTALL) -D -m 755 $(@D)/alsa_loop $(TARGET_DIR)/usr/bin/alsa_loop
	$(INSTALL) -D -m 755 $(@D)/alsa_capture $(TARGET_DIR)/usr/bin/alsa_capture
	$(INSTALL) -D -m 755 $(@D)/alsa_device_info $(TARGET_DIR)/usr/bin/alsa_device_info
	$(INSTALL) -D -m 755 $(@D)/playback_capture $(TARGET_DIR)/usr/bin/playback_capture
endef

$(eval $(generic-package))