DUMMY_CODEC_VERSION = 1.0
DUMMY_CODEC_SITE = $(BR2_EXTERNAL_SBCBOARD_PATH)/package/dummy_codec/src
DUMMY_CODEC_SITE_METHOD = local

$(eval $(kernel-module))
$(eval $(generic-package))
