#
# Copyright 2015 The Android Open-Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

include device/sprd/sharkle/common/BoardCommon.mk

#chipram tool for arm64
TOOLCHAIN_64 := true

TARGET_CPU_SMP := true
ARCH_ARM_HAVE_TLS_REGISTER := true

sprdiskexist := $(shell if [ -f $(TOPDIR)sprdisk/Makefile -a "$(TARGET_BUILD_VARIANT)" = "userdebug" ]; then echo "exist"; else echo "notexist"; fi;)
ifneq ($(sprdiskexist), exist)
TARGET_NO_SPRDISK := true
else
TARGET_NO_SPRDISK := false
endif
SPRDISK_BUILD_PATH := sprdisk/


# ext4 partition layout
#BOARD_VENDORIMAGE_PARTITION_SIZE := 300000000
#BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE := ext4
TARGET_USERIMAGES_USE_EXT4 := true
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 1073741824
BOARD_CACHEIMAGE_PARTITION_SIZE := 150000000
BOARD_PRODNVIMAGE_PARTITION_SIZE := 5242880
BOARD_USBMSCIMAGE_PARTITION_SIZE := 31457280 #30M
BOARD_USERDATAIMAGE_PARTITION_SIZE := 536870912
BOARD_FLASH_BLOCK_SIZE := 4096
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_PRODNVIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_USBMSCIMAGE_FILE_SYSTEM_TYPE := ext4

TARGET_SYSTEMIMAGES_SPARSE_EXT_DISABLED := true
TARGET_USERIMAGES_SPARSE_EXT_DISABLED := false
BOARD_PERSISTIMAGE_PARTITION_SIZE := 2097152
TARGET_PRODNVIMAGES_SPARSE_EXT_DISABLED := true
TARGET_USBMSCIMAGES_SPARSE_EXT_DISABLED := true
TARGET_CACHEIMAGES_SPARSE_EXT_DISABLED := false
USE_SPRD_SENSOR_HUB := false

#===============start camera configuration===============
#------section 1: software structure------

TARGET_BOARD_SPRD_EXFRAMEWORKS_SUPPORT := true

#start camera configuration
#HAL1.0  HAL2.0  HAL3.0
TARGET_BOARD_CAMERA_HAL_VERSION := HAL3.0

#sc8830g isp ver 0;sc9630 isp ver 1;tshark2 isp version 2
TARGET_BOARD_CAMERA_ISP_SOFTWARE_VERSION := 2

#support 64bit isp
TARGET_BOARD_CAMERA_ISP_64BIT := true

# temp for isp3.0
TARGET_BOARD_CAMERA_ISP_DIR := 2.3

#camera offline structure
TARGET_BOARD_CAMERA_OFFLINE := true
#end camera configuration
TARGET_BOARD_IS_SC_FPGA := false

#------section 2: sensor & flash config------
#camera auto detect sensor
TARGET_BOARD_CAMERA_AUTO_DETECT_SENSOR := false

#select camera 2M,3M,5M,8M,13M,16M,21M
CAMERA_SUPPORT_SIZE := 2M
FRONT_CAMERA_SUPPORT_SIZE := 0M3
TARGET_BOARD_NO_FRONT_SENSOR := false
TARGET_BOARD_SENSOR2_SUPPORT := false
TARGET_BOARD_SENSOR3_SUPPORT := false
TARGET_BOARD_FRONT_CAMERA_ROTATION := false
TARGET_BOARD_CAMERA_FLASH_CTRL := false
# camera sensor type
CAMERA_SENSOR_TYPE_BACK := "gc2385,sp250a"
CAMERA_SENSOR_TYPE_FRONT := "gc030a_f,sp0a09_f"

#select ccir pclk src(source0, source1)
TARGET_BOARD_FRONT_CAMERA_CCIR_PCLK := source0
TARGET_BOARD_BACK_CAMERA_CCIR_PCLK := source0

#PDAF feature
TARGET_BOARD_CAMERA_PDAF := false

TARGET_BOARD_CAMERA_FLASH_CTRL := false

#flash led  feature
TARGET_BOARD_CAMERA_FLASH_LED_0 := false # flash led0 config
TARGET_BOARD_CAMERA_FLASH_LED_1 := false # flash led1 config

#------section 3: feature config------
#select camera zsl cap mode
TARGET_BOARD_CAMERA_CAPTURE_MODE := false
TARGET_BOARD_CAMERA_CAPTURE_NOZSL := true

#select camera not support autofocus
TARGET_BOARD_CAMERA_NO_AUTOFOCUS_DEV := false

#select camera support autofocus
TARGET_BOARD_CAMERA_AUTOFOCUS := false

#select continuous auto focus
TARGET_BOARD_CAMERA_CAF := false

#VCM DRIVER
TARGET_BOARD_AF_VCM_DW9800W := false

TARGET_BOARD_FRONT_CAMERA_ROTATION := false

#power optimization
TARGET_BOARD_CAMERA_POWER_OPTIMIZATION := false

ifneq ($(strip $(CMCC_PROJECT)),true)
#face detect
TARGET_BOARD_CAMERA_FACE_DETECT := false
TARGET_BOARD_CAMERA_FD_LIB := omron

#hdr capture
TARGET_BOARD_CAMERA_HDR_CAPTURE := false
TARGET_BOARD_CAMERA_HDR_SPRD_LIB := false

#UCAM feature
TARGET_BOARD_CAMERA_FACE_BEAUTY := false

#covered camera enble
TARGET_BOARD_COVERED_SENSOR_SUPPORT := false

#blur mode enble
TARGET_BOARD_BLUR_MODE_SUPPORT := false
endif

#GYRO
TARGET_BOARD_CAMERA_GYRO := false

#support auto anti-flicker
TARGET_BOARD_CAMERA_ANTI_FLICKER := true

#uv denoise
TARGET_BOARD_CAMERA_UV_DENOISE := false

#------section 4: optimize config------
#capture mem
TARGET_BOARD_LOW_CAPTURE_MEM := true

#sensor interface
TARGET_BOARD_BACK_CAMERA_INTERFACE := mipi
TARGET_BOARD_FRONT_CAMERA_INTERFACE := mipi

#rotation capture
TARGET_BOARD_CAMERA_ROTATION_CAPTURE := false

#image angle in different project
TARGET_BOARD_CAMERA_ADAPTER_IMAGE := 180

#pre_allocate capture memory
TARGET_BOARD_CAMERA_PRE_ALLOC_CAPTURE_MEM := false


#low capture memory
TARGET_BOARD_LOW_CAPTURE_MEM := true

#use arcsoft filter lib
TARGET_BOARD_ARCSOFT_FILTER := false

#use power hint
TARGET_BOARD_POWER_HINT_USED := false

#support 4k record
TARGET_BOARD_CAMERA_SUPPORT_4K_RECORD := false

#Slowmotion optimize
TARGET_BOARD_SPRD_SLOWMOTION_OPTIMIZE := false

#------section 5: other misc config------
#sensor multi-instance
TARGET_BOARD_CAMERA_SENSOR_MULTI_INSTANCE_SUPPORT := false


# ===============end of camera configuration ===============

#GNSS GPS
BOARD_USE_SPRD_GNSS := ge2

#SPRD: SUPPORT EXTERNAL WCN
SPRD_CP_LOG_WCN := MARLIN2

# select FMRadio
BOARD_USE_SPRD_FMAPP := false
BOARD_HAVE_FM_BCM := false
BOARD_HAVE_BLUETOOTH := true

# select sdcard
TARGET_USE_SDCARDFS := false
USE_VENDOR_LIB := true

#Audio NR enable
AUDIO_RECORD_NR := true
# NXP smart pa
AUDIO_SMART_PA_TYPE := NXP

# WIFI configs
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
WPA_SUPPLICANT_VERSION      := VER_2_1_DEVEL
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_sprdwl
BOARD_HOSTAPD_DRIVER        := NL80211
BOARD_HOSTAPD_PRIVATE_LIB   := lib_driver_cmd_sprdwl
BOARD_WLAN_DEVICE           := sp9832e
WIFI_DRIVER_FW_PATH_PARAM   := "/data/misc/wifi/fwpath"
WIFI_DRIVER_FW_PATH_STA     := "sta_mode"
WIFI_DRIVER_FW_PATH_P2P     := "p2p_mode"
WIFI_DRIVER_FW_PATH_AP      := "ap_mode"
WIFI_DRIVER_MODULE_PATH     := "/lib/modules/sprdwl_ng.ko"
WIFI_DRIVER_MODULE_NAME     := "sprdwl_ng"
BOARD_SEPOLICY_DIRS += device/sprd/sharkle/common/sepolicy \
    build/target/board/generic/sepolicy

#SPRD: acquire powerhint during playing video
POWER_HINT_VIDEO_CONTROL_CORE := true

# select sensor
USE_SPRD_SENSOR_LIB := NULL
BOARD_HAVE_ACC := NULL
BOARD_ACC_INSTALL := NULL
BOARD_HAVE_ORI := NULL
BOARD_ORI_INSTALL := NULL
BOARD_HAVE_PLS := NULL
BOARD_PLS_COMPATIBLE := NULL
CP_LOG_DIR_IN_AP := ylog

# WFD max support 720P
TARGET_BOARD_WFD_MAX_SUPPORT_720P := true

# WFD support 30fps
TARGET_BOARD_WFD_SUPPOERT_30FPS :=true

#SUPPORT LOWPOWER WITH LCD 30 FPS
LOWPOWER_DISPLAY_30FPS := false


# Graphics
FPHONE_ENFORCE_RGB_565_FLAG := true

TARGET_PROVIDES_B2G_INIT_RC := true
BOARD_SECURE_BOOT_ENABLE := false
