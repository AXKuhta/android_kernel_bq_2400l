USE_VOLD_EX := true
PRODUCT_PROPERTY_OVERRIDES += \
	ro.storage.flash_type=2

ifndef STORAGE_PRIMARY
  STORAGE_PRIMARY := internal
endif

STORAGE_ORIGINAL := false
PRODUCT_REVISION += storage_trunk
include $(APPLY_PRODUCT_REVISION)

-include $(PLATCOMM)/storage_device.mk
