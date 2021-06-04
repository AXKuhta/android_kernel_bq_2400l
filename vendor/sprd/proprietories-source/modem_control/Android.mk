LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(BOARD_SECURE_BOOT_ENABLE), true)
    LOCAL_C_INCLUDES := \
        vendor/sprd/proprietories-source/sprd_verify
endif

LOCAL_SRC_FILES:= \
    main.c

ifeq ($(BOARD_EXTERNEL_MODEM), true)
    LOCAL_SRC_FILES += modem_boot.c packet.c crc16.c
else
    LOCAL_SRC_FILES += modem_control.c nv_read.c
endif

ifeq ($(BOARD_SECURE_BOOT_ENABLE), true)
    LOCAL_SRC_FILES += modem_verify.c
    LOCAL_STATIC_LIBRARIES += libsprd_verify
endif

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libhardware_legacy

ifeq ($(strip $(TARGET_USERIMAGES_USE_EXT4)),true)
LOCAL_CFLAGS := -DCONFIG_EMMC
endif

ifeq ($(BOARD_SECURE_BOOT_ENABLE), true)
  LOCAL_CFLAGS += -DSECURE_BOOT_ENABLE
endif

#Temporary remove for secureboot ptr0
ifeq ($(strip $(PRODUCT_SECURE_BOOT)),SPRD)
    ifeq ($(strip $(BOARD_TEE_CONFIG)),trusty)
        LOCAL_C_INCLUDES += vendor/sprd/proprietories-source/sprdtrusty/vendor/sprd/modules/kernelbootcp_ca
        LOCAL_CFLAGS += -DCONFIG_SPRD_SECBOOT
        LOCAL_SHARED_LIBRARIES += libkernelbootcp.trusty
    endif
endif

LOCAL_MODULE := modem_control

LOCAL_INIT_RC := modem_control.rc

LOCAL_MODULE_TAGS := optional

#LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)
