# This makefile is used to generate device tree image

#----------------------------------------------------------------------
# Generate device tree image (dt.img)
#----------------------------------------------------------------------
DTBTOOL := $(HOST_OUT_EXECUTABLES)/dtbTool$(HOST_EXECUTABLE_SUFFIX)

INSTALLED_DTIMAGE_TARGET := $(PRODUCT_OUT)/dt.img

ifeq ($(TARGET_KERNEL_ARCH), arm64)
DTB := $(KERNEL_OUT)/arch/$(TARGET_KERNEL_ARCH)/boot/dts/sprd/
endif

ifeq ($(TARGET_KERNEL_ARCH), arm)
DTB := $(KERNEL_OUT)/arch/$(TARGET_KERNEL_ARCH)/boot/dts/
endif

define build-dtimage-target
    $(call pretty,"Target dt image: $(INSTALLED_DTIMAGE_TARGET)")
    @-find $(DTB) -name *.dtb ! -name $(TARGET_DTB).dtb | xargs -I{} rm {}
    $(DTBTOOL) -o $@ -s $(BOARD_KERNEL_PAGESIZE) -p $(KERNEL_OUT)/scripts/dtc/ $(DTB)
    $(hide) chmod a+r $@
endef

.PHONY: $(INSTALLED_DTIMAGE_TARGET)
$(INSTALLED_DTIMAGE_TARGET): $(DTBTOOL) $(TARGET_PREBUILT_KERNEL)
	$(build-dtimage-target)

ALL_DEFAULT_INSTALLED_MODULES += $(INSTALLED_DTIMAGE_TARGET)
ALL_MODULES.$(LOCAL_MODULE).INSTALLED += $(INSTALLED_DTIMAGE_TARGET)

.PHONY: dtimage
dtimage: $(DTBTOOL) $(KERNEL_CONFIG) FORCE
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=$(TARGET_KERNEL_ARCH) CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) dtbs
	@-find $(DTB) -name *.dtb ! -name $(TARGET_DTB).dtb | xargs -I{} rm {}
	$(DTBTOOL) -o $(INSTALLED_DTIMAGE_TARGET) -s $(BOARD_KERNEL_PAGESIZE) -p $(KERNEL_OUT)/scripts/dtc/ $(DTB)
	$(hide) chmod a+r $(INSTALLED_DTIMAGE_TARGET)
