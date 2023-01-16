HIFIBERRY_DAC_VERSION = 1.0
HIFIBERRY_DAC_SITE = $(BR2_EXTERNAL_SBCBOARD_PATH)/package/hifiberry_dac/src
HIFIBERRY_DAC_SITE_METHOD = local

$(eval $(kernel-module))
$(eval $(generic-package))
