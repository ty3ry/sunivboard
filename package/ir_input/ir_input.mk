IR_INPUT_VERSION = 1.0
IR_INPUT_SITE = $(BR2_EXTERNAL_SBCBOARD_PATH)/package/ir_input/src
IR_INPUT_SITE_METHOD = local

define IR_INPUT_BUILD_CMDS
	$(TARGET_CC) -o $(@D)/ir_input $(@D)/ir_input.c
endef

define IR_INPUT_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/ir_input $(TARGET_DIR)/usr/bin/ir_input
endef

$(eval $(generic-package))
