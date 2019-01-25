obj-y := cpp_lite_r3p0.o

cpp_lite_r3p0-objs := cpp_core.o scale_drv.o rot_drv.o dma_drv.o

ccflags-y += -I$(srctree)/drivers/misc/sprd_camera/common
ifeq ($(CONFIG_SPRD_CAM_PW_DOMAIN_R5P0), y)
ccflags-y += -I$(srctree)/drivers/misc/sprd_camera/common/r5p0
endif
