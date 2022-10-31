!/bin/sh
#grep -q "GADGET_SERIAL" "${TARGET_DIR}/etc/inittab" \
#	|| echo '/dev/ttyGS0::respawn:/sbin/getty -L  /dev/ttyGS0 0 vt100 # GADGET_SERIAL' >> "${TARGET_DIR}/etc/inittab"
# grep -q "ubi0:persist" "${TARGET_DIR}/etc/fstab" \
# 	|| echo 'ubi0:persist	/root		ubifs	defaults	0	0' >> "${TARGET_DIR}/etc/fstab"

# remove all content in target firmware/brcm
# and copy specific file in overlay dir
rm -rf "${TARGET_DIR}"/lib/firmware/brcm/*
cp "${BR2_EXTERNAL_SUNIVBOARD_PATH}"/board/sunivboard_v2/rootfs_overlay/lib/firmware/brcm/* "${TARGET_DIR}"/lib/firmware/brcm

# remove cypress firmware directory
# this firmware always selected if config BR2_PACKAGE_LINUX_FIRMWARE_BRCM_BCM43XX enabled
# remove to save space in limited spi flash
rm -rf "${TARGET_DIR}"/lib/firmware/cypress/*