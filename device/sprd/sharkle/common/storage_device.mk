
# storage init files
ifeq ($(STORAGE_ORIGINAL), true)
    PRODUCT_COPY_FILES += \
        $(ROOTCOMM)/root/init.storage_original.rc:root/init.storage.rc
    PRODUCT_PROPERTY_OVERRIDES += \
        persist.storage.type=2 \
        sys.internal.emulated=1
else
PRODUCT_COPY_FILES += \
    $(ROOTCOMM)/root/init.storage_sprd.rc:root/init.storage.rc
  PRODUCT_PROPERTY_OVERRIDES += \
    sys.internal.emulated=1
  ifeq ($(STORAGE_PRIMARY), internal)
    PRODUCT_PROPERTY_OVERRIDES += \
        persist.storage.type=2
  endif
  ifeq ($(STORAGE_PRIMARY), external)
    PRODUCT_PROPERTY_OVERRIDES += \
        ro.vold.primary_physical=1 \
        persist.storage.type=1
  endif
endif

ifndef INSTALL_INTERNAL
  INSTALL_INTERNAL := false
endif

ifeq ($(strip $(INSTALL_INTERNAL)),true)
PRODUCT_PROPERTY_OVERRIDES += \
    ro.storage.install2internal=1
else
PRODUCT_PROPERTY_OVERRIDES += \
    ro.storage.install2internal=0
endif

