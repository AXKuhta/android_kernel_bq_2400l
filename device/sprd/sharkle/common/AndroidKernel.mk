KERNEL_OUT := $(TARGET_OUT_INTERMEDIATES)/KERNEL
KERNEL_CONFIG := $(KERNEL_OUT)/.config
KERNEL_MODULES_OUT := $(TARGET_ROOT_OUT)/lib/modules
KERNEL_USR_HEADERS := $(KERNEL_OUT)/usr
KERNEL_DIFF_CONFIG_ARCH := kernel/sprd-diffconfig/sharkle/$(TARGET_KERNEL_ARCH)
KERNEL_DIFF_CONFIG_COMMON := kernel/sprd-diffconfig/sharkle/common

JOBS := $(shell if [ $(cat /proc/cpuinfo | grep processor | wc -l) -gt 8 ]; then echo 8; else echo 4; fi)
TARGET_PREBUILT_KERNEL := $(KERNEL_OUT)/arch/$(TARGET_KERNEL_ARCH)/boot/Image

$(KERNEL_OUT):
	@echo "==== Start Kernel Compiling ... ===="

$(KERNEL_CONFIG): kernel/arch/$(TARGET_KERNEL_ARCH)/configs/$(KERNEL_DEFCONFIG)
	echo "KERNEL_OUT = $(KERNEL_OUT),  KERNEL_DEFCONFIG = $(KERNEL_DEFCONFIG)"
	mkdir -p $(KERNEL_OUT)
	$(MAKE) ARCH=$(TARGET_KERNEL_ARCH) -C kernel O=../$(KERNEL_OUT) $(KERNEL_DEFCONFIG)

define  sprd_create_user_config
	$(shell ./kernel/scripts/sprd_create_user_config.sh $(1) $(2))
endef

ifeq ($(strip $(BOARD_TEE_CONFIG)),trusty)
ifeq ($(strip $(BOARD_FEATUREPHONE_CONFIG)),true)
TARGET_DEVICE_TRUSTY_CONFIG := $(KERNEL_DIFF_CONFIG_ARCH)/trusty_aarch32_diff_config
else
TARGET_DEVICE_TRUSTY_CONFIG := $(KERNEL_DIFF_CONFIG_ARCH)/trusty_aarch64_diff_config
endif
endif

ifeq ($(strip $(BOARD_FINGERPRINT_CONFIG)),sunwave)
TARGET_DEVICE_SUNWAVE_CONFIG := $(KERNEL_DIFF_CONFIG_COMMON)/sunwave_diff_config
endif

SHARKLE_KAI_SPEC_CONFIG := $(KERNEL_DIFF_CONFIG_ARCH)/kaios_diff_config
TARGET_BOARD_SPEC_CONFIG := $(KERNEL_DIFF_CONFIG_ARCH)/$(TARGET_BOARD)_kaios_diff_config
ifeq ($(TARGET_BUILD_VARIANT),user)
DEBUGMODE := BUILD=no
USER_CONFIG := $(TARGET_OUT)/dummy
ifeq ($(TARGET_KERNEL_ARCH), arm)
	TARGET_DEVICE_USER_CONFIG := $(KERNEL_DIFF_CONFIG_ARCH)/aarch32_user_diff_config
endif
ifeq ($(TARGET_KERNEL_ARCH), arm64)
	TARGET_DEVICE_USER_CONFIG := $(KERNEL_DIFF_CONFIG_ARCH)/aarch64_user_diff_config
endif
$(USER_CONFIG) : $(KERNEL_CONFIG)
	$(call sprd_create_user_config, $(KERNEL_CONFIG), $(SHARKLE_KAI_SPEC_CONFIG))
	$(call sprd_create_user_config, $(KERNEL_CONFIG), $(TARGET_DEVICE_USER_CONFIG))
	$(call sprd_create_user_config, $(KERNEL_CONFIG), $(TARGET_BOARD_SPEC_CONFIG))
ifeq ($(strip $(BOARD_TEE_CONFIG)),trusty)
	$(call sprd_create_user_config, $(KERNEL_CONFIG), $(TARGET_DEVICE_TRUSTY_CONFIG))
endif
ifeq ($(strip $(BOARD_FINGERPRINT_CONFIG)),sunwave)
	$(call sprd_create_user_config, $(KERNEL_CONFIG), $(TARGET_DEVICE_SUNWAVE_CONFIG))
endif
else
DEBUGMODE := $(DEBUGMODE)
USER_CONFIG  := $(TARGET_OUT)/dummy
$(USER_CONFIG) : $(KERNEL_CONFIG)
	$(call sprd_create_user_config, $(KERNEL_CONFIG), $(SHARKLE_KAI_SPEC_CONFIG))
	$(call sprd_create_user_config, $(KERNEL_CONFIG), $(TARGET_BOARD_SPEC_CONFIG))
ifeq ($(strip $(BOARD_TEE_CONFIG)),trusty)
	$(call sprd_create_user_config, $(KERNEL_CONFIG), $(TARGET_DEVICE_TRUSTY_CONFIG))
endif
ifeq ($(strip $(BOARD_FINGERPRINT_CONFIG)),sunwave)
	$(call sprd_create_user_config, $(KERNEL_CONFIG), $(TARGET_DEVICE_SUNWAVE_CONFIG))
endif
endif

$(TARGET_PREBUILT_KERNEL) : $(KERNEL_OUT) $(USER_CONFIG)
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=$(TARGET_KERNEL_ARCH) CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) headers_install
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=$(TARGET_KERNEL_ARCH) CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) -j${JOBS}
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=$(TARGET_KERNEL_ARCH) CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) modules
	@-mkdir -p $(KERNEL_MODULES_OUT)
	@-find $(TARGET_OUT_INTERMEDIATES) -name *.ko | xargs -I{} cp {} $(KERNEL_MODULES_OUT)

$(KERNEL_USR_HEADERS) : $(KERNEL_OUT)
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=$(TARGET_KERNEL_ARCH) CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) headers_install

kernelheader:
	mkdir -p $(KERNEL_OUT)
	$(MAKE) ARCH=$(TARGET_KERNEL_ARCH) -C kernel O=../$(KERNEL_OUT) headers_install
