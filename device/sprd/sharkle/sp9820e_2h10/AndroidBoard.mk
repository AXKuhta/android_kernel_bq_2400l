LOCAL_PATH := $(call my-dir)

$(call add-radio-file,modem_bins/ltemodem.bin)
$(call add-radio-file,modem_bins/ltegdsp.bin)
$(call add-radio-file,modem_bins/ltedsp.bin)
$(call add-radio-file,modem_bins/pmsys.bin)
$(call add-radio-file,modem_bins/ltenvitem.bin)
$(call add-radio-file,modem_bins/wcnmodem.bin)
$(call add-radio-file,modem_bins/gnssmodem.bin)
$(call add-radio-file,modem_bins/gnssbdmodem.bin)
#$(call add-radio-file,modem_bins/ltedeltanv.bin)

# Compile U-Boot
ifneq ($(strip $(TARGET_NO_BOOTLOADER)),true)
INSTALLED_UBOOT_TARGET := $(PRODUCT_OUT)/u-boot.bin
INSTALLED_CHIPRAM_TARGET := $(PRODUCT_OUT)/u-boot-spl-16k.bin
SPRD_MCP_XLS := vendor/sprd/tools/mcp_gen/mcp.xls
UBOOT_SPRD_NAND_PARAM_H := u-boot15/drivers/mtd/nand_sprd/sprd_nand_param.h
-include u-boot15/AndroidUBoot.mk
-include chipram/AndroidChipram.mk

INSTALLED_RADIOIMAGE_TARGET += $(PRODUCT_OUT)/u-boot.bin
INSTALLED_RADIOIMAGE_TARGET += $(PRODUCT_OUT)/u-boot-spl-16k.bin

$(UBOOT_SPRD_NAND_PARAM_H):$(SPRD_MCP_XLS)
	@echo "Start to autogenerate u-boot sprd_nand_param.h"
	@perl vendor/sprd/tools/mcp_gen/nandgen.pl -h $(UBOOT_SPRD_NAND_PARAM_H)

$(INSTALLED_UBOOT_TARGET):$(UBOOT_SPRD_NAND_PARAM_H)

.PHONY: bootloader
bootloader: $(INSTALLED_UBOOT_TARGET)

ALL_DEFAULT_INSTALLED_MODULES += $(INSTALLED_UBOOT_TARGET)
ALL_MODULES.$(LOCAL_MODULE).INSTALLED += $(INSTALLED_UBOOT_TARGET)

CHIPRAM_SPRD_NAND_PARAM_H := chipram/include/sprd_nand_param.h
$(CHIPRAM_SPRD_NAND_PARAM_H):$(SPRD_MCP_XLS)
	@echo "Start to autogenerate chipram sprd_nand_param.h"
	@perl vendor/sprd/tools/mcp_gen/nandgen.pl -h $(CHIPRAM_SPRD_NAND_PARAM_H)

$(INSTALLED_CHIPRAM_TARGET):$(CHIPRAM_SPRD_NAND_PARAM_H)

.PHONY: chipram
chipram: $(INSTALLED_CHIPRAM_TARGET)

ALL_DEFAULT_INSTALLED_MODULES += $(INSTALLED_CHIPRAM_TARGET)
ALL_MODULES.$(LOCAL_MODULE).INSTALLED += $(INSTALLED_CHIPRAM_TARGET)
endif # End of U-Boot

# Compile Linux Kernel
ifneq ($(strip $(TARGET_NO_KERNEL)),true)
-include device/sprd/sharkle/common/AndroidKernel.mk
file := $(PRODUCT_OUT)/kernel
#ALL_PREBUILT += $(file)
$(file) : $(TARGET_PREBUILT_KERNEL) | $(ACP)
	$(transform-prebuilt-to-target)
endif # End of Kernel

# Compile dtb
ifeq ($(strip $(BOARD_KERNEL_SEPARATED_DT)),true)
include device/sprd/sharkle/common/generate_dt_image.mk
endif # End of dtb

# Compile sprdisk
ifneq ($(strip $(TARGET_NO_SPRDISK)),true)
INSTALLED_SPRDISK_TARGET := $(PRODUCT_OUT)/sprdiskboot.img
-include sprdisk/Makefile

INSTALLED_RADIOIMAGE_TARGET += $(PRODUCT_OUT)/sprdiskboot.img

.PHONY: sprdisk
sprdisk: $(INSTALLED_SPRDISK_TARGET)

ALL_DEFAULT_INSTALLED_MODULES += $(INSTALLED_SPRDISK_TARGET)
ALL_MODULES.$(LOCAL_MODULE).INSTALLED += $(INSTALLED_SPRDISK_TARGET)
endif # End of sprdisk
