# include by nand BoardConfig

# nand fstab
FSTAB_SUFFIX :=
ifeq ($(BOARD_SECURE_BOOT_ENABLE), true)
  FSTAB_SUFFIX := .secure_boot
endif

TARGET_RECOVERY_FSTAB := $(PLATCOMM)/nand/recovery$(FSTAB_SUFFIX).fstab

# not have selinux now
NOT_HAVE_SELINUX := true
