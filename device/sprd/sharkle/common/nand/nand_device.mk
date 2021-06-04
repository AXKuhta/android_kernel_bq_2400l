
FSTAB_SUFFIX :=
PRODUCT_COPY_FILES += \
  $(PLATCOMM)/nand/fstab$(FSTAB_SUFFIX).sc8830:root/fstab.$(TARGET_BOARD)

-include $(PLATCOMM)/storage_device.mk
