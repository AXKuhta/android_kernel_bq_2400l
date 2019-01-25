obj-$(CONFIG_SPRD_CAMERA_DCAM_IF_R4P0) += dcam_if_r4p0/

ccflags-y += -I$(srctree)/drivers/misc/sprd_camera/common

ifeq ($(CONFIG_SPRD_CAM_PW_DOMAIN_R5P0), y)
ccflags-y += -I$(srctree)/drivers/misc/sprd_camera/common/r5p0 \
	     	-I$(srctree)/drivers/misc/sprd_camera/sensor \
		-I$(srctree)/drivers/misc/sprd_camera/flash
endif
