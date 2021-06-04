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

PLATDIR := device/sprd/sharkle
PLATCOMM := $(PLATDIR)/common
TARGET_BOARD := sp9820e_2c10
BOARDDIR := $(PLATDIR)/$(TARGET_BOARD)
ROOTDIR := $(BOARDDIR)/rootdir
TARGET_BOARD_PLATFORM := sp9820e

TARGET_GPU_PLATFORM := midgard
TARGET_NO_BOOTLOADER := false

USE_XML_AUDIO_POLICY_CONF := 0

VOLTE_SERVICE_ENABLE := true
#dm-verity: new Board should not config this
TARGET_DM_VERITY := true

#Config Android Render With CPU; Default Android render with GPU
#GRAPHIC_RENDER_TYPE    := CPU

# graphics
USE_SPRD_HWCOMPOSER  := true

# copy media_profiles.xml before calling device.mk,
# because we want to use our file, not the common one
PRODUCT_COPY_FILES += $(BOARDDIR)/media_profiles.xml:system/etc/media_profiles.xml

#$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)
$(call inherit-product, $(PLATCOMM)/DeviceCommon.mk)
$(call inherit-product, $(PLATCOMM)/proprietories.mk)
$(call inherit-product-if-exists, vendor/sprd/modules/libcamera/libcam_device.mk)

BOARD_HAVE_SPRD_WCN_COMBO := sharkle
$(call inherit-product-if-exists, vendor/sprd/modules/wcn/connconfig/device-sprd-wcn.mk)


#fstab
ifeq (f2fs,$(strip $(BOARD_USERDATAIMAGE_FILE_SYSTEM_TYPE)))
  NORMAL_FSTAB_SUFFIX1 := .f2fs
endif
ifeq (true,$(strip $(BOARD_SECURE_BOOT_ENABLE)))
  NORMAL_FSTAB_SUFFIX2 :=
endif
ifeq (true,$(strip $(TARGET_DM_VERITY)))
  NORMAL_FSTAB_SUFFIX3 := .verify
endif
NORMAL_FSTAB_SUFFIX := $(NORMAL_FSTAB_SUFFIX1)$(NORMAL_FSTAB_SUFFIX2)$(NORMAL_FSTAB_SUFFIX3)
# $(warning NORMAL_FSTAB=$(LOCAL_PATH)/rootdir/root/fstab$(NORMAL_FSTAB_SUFFIX).$(TARGET_BOARD))
PRODUCT_COPY_FILES += $(BOARDDIR)/rootdir/root/fstab.$(TARGET_BOARD)$(NORMAL_FSTAB_SUFFIX):root/fstab.$(TARGET_BOARD)


PRODUCT_COPY_FILES += \
    $(BOARDDIR)/rootdir/root/init.$(TARGET_BOARD).rc:root/init.$(TARGET_BOARD).rc \
    $(ROOTDIR)/prodnv/PCBA.conf:system/etc/PCBA.conf \
    $(ROOTDIR)/prodnv/BBAT.conf:system/etc/BBAT.conf \
    $(ROOTDIR)/system/etc/audio_params/tiny_hw.xml:system/etc/tiny_hw.xml \
    $(ROOTDIR)/system/etc/audio_params/codec_pga.xml:system/etc/codec_pga.xml \
    $(ROOTDIR)/system/etc/audio_params/audio_hw.xml:system/etc/audio_hw.xml \
    $(ROOTDIR)/system/etc/audio_params/audio_para:system/etc/audio_para \
    $(ROOTDIR)/system/etc/audio_params/smart_amp_init.bin:system/etc/smart_amp_init.bin \
    $(ROOTDIR)/system/etc/audio_params/tfa9897.cnt:system/vendor/firmware/tfa9897.cnt \
    $(ROOTDIR)/system/etc/audio_params/record_tone_1.pcm:system/etc/record_tone_1.pcm \
    $(ROOTDIR)/system/etc/audio_params/record_tone_2.pcm:system/etc/record_tone_2.pcm \
    $(ROOTDIR)/system/etc/audio_params/rx_data.pcm:system/etc/rx_data.pcm \
    $(PLATCOMM)/rootdir/root/ueventd.common.rc:root/ueventd.$(TARGET_BOARD).rc \
    $(PLATCOMM)/rootdir/root/init.common.usb.rc:root/init.$(TARGET_BOARD).usb.rc \
    $(BOARDDIR)/volume.cfg:system/etc/volume.cfg \
    frameworks/native/data/etc/android.software.midi.xml:system/etc/permissions/android.software.midi.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.opengles.aep.xml:system/etc/permissions/android.hardware.opengles.aep.xml \
    $(BOARDDIR)/log_conf/slog_modem_$(TARGET_BUILD_VARIANT).conf:system/etc/slog_modem.conf \
    $(BOARDDIR)/log_conf/test_mode_$(TARGET_BUILD_VARIANT).conf:system/etc/test_mode.conf



#WLAN config
PRODUCT_PROPERTY_OVERRIDES += \
    ro.hotspot.enabled = 1 \
    reset_default_http_response = true \
    ro.void_charge_tip = true \
    ro.softaplte.coexist = true \
    ro.vowifi.softap.ee_warning = false \
    persist.sys.wifi.pocketmode = true \
    ro.wcn = enabled

ifeq ($(strip $(BOARD_HAVE_SPRD_WCN_COMBO)), marlin)
PRODUCT_PROPERTY_OVERRIDES += \
    ro.softap.whitelist = true \
    ro.btwifisoftap.coexist = true\
    persist.wifi.func.hidessid = false \
    ro.wifi.softap.maxstanum = 8 \
    ro.wifi.signal.optimized = true \
    ro.support.auto.roam = enabled \
    ro.wifip2p.coexist = true

endif

ifeq ($(strip $(BOARD_HAVE_SPRD_WCN_COMBO)), marlin2)
PRODUCT_PROPERTY_OVERRIDES += \
    ro.softap.whitelist = true \
    ro.btwifisoftap.coexist = true\
    persist.wifi.func.hidessid = true\
    ro.wifi.softap.maxstanum = 10 \
    ro.wifi.signal.optimized = true \
    ro.support.auto.roam = disabled \
    ro.wifip2p.coexist = true

endif

ifeq ($(strip $(BOARD_HAVE_SPRD_WCN_COMBO)), sharkle)
PRODUCT_PROPERTY_OVERRIDES += \
    ro.softap.whitelist = true \
    ro.btwifisoftap.coexist = true\
    persist.wifi.func.hidessid = true\
    ro.wifi.softap.maxstanum = 10 \
    ro.wifi.signal.optimized = true \
    ro.support.auto.roam = disabled \
    ro.wifip2p.coexist = true

endif

ifeq ($(strip $(BOARD_HAVE_SPRD_WCN_COMBO)), rs2351)
PRODUCT_PROPERTY_OVERRIDES += \
    ro.softap.whitelist = false \
    ro.btwifisoftap.coexist = false \
    persist.wifi.func.hidessid = false \
    ro.wifi.softap.maxstanum = 8 \
    ro.wifi.signal.optimized = true \
    ro.support.auto.roam = enabled \
    ro.wifip2p.coexist = false

endif

#copy audio policy config
ifeq ($(USE_XML_AUDIO_POLICY_CONF), 1)
PRODUCT_COPY_FILES += \
    $(ROOTDIR)/system/etc/audio_policy_config/audio_policy_configuration.xml:system/etc/audio_policy_configuration.xml \
    $(ROOTDIR)/system/etc/audio_policy_config/audio_policy_configuration_stub.xml:system/etc/audio_policy_configuration_stub.xml \
    $(ROOTDIR)/system/etc/audio_policy_config/a2dp_audio_policy_configuration.xml:system/etc/a2dp_audio_policy_configuration.xml \
    $(ROOTDIR)/system/etc/audio_policy_config/usb_audio_policy_configuration.xml:system/etc/usb_audio_policy_configuration.xml \
    $(ROOTDIR)/system/etc/audio_policy_config/r_submix_audio_policy_configuration.xml:system/etc/r_submix_audio_policy_configuration.xml \
    $(ROOTDIR)/system/etc/audio_policy_config/audio_policy_volumes.xml:system/etc/audio_policy_volumes.xml \
    $(ROOTDIR)/system/etc/audio_policy_config/default_volume_tables.xml:system/etc/default_volume_tables.xml
else
PRODUCT_COPY_FILES += \
    $(ROOTDIR)/system/etc/audio_params/audio_policy.conf:system/etc/audio_policy.conf \
    $(ROOTDIR)/system/etc/audio_params/audio_policy_nxp_smart_pa.conf:system/etc/audio_policy_nxp_smart_pa.conf
endif



# config sepolicy
#duplicate definition
#BOARD_SEPOLICY_DIRS += device/sprd/sharkle/common/sepolicy \
#    build/target/board/generic/sepolicy

SPRD_GNSS_ARCH := arm64
SPRD_GNSS_SHARKLE_PIKL2 := true
$(call inherit-product, vendor/sprd/modules/gps/GreenEye2/device-sprd-gps.mk)

#VOLTE_SERVICE_ENABLE := true

#WCN config
PRODUCT_PROPERTY_OVERRIDES += \
    ro.modem.wcn.enable=1 \
    ro.modem.wcn.diag=/dev/slog_wcn \
    ro.modem.wcn.id=1 \
    ro.modem.wcn.count=1 \
    ro.modem.gnss.diag=/dev/slog_gnss \
    persist.sys.support.vt=true \
    persist.sys.csvt=false \
    persist.radio.modem.workmode=3,10 \
    persist.radio.modem.config=TL_LF_W_G,G \
    persist.radio.ssda.mode=csfb \
    persist.radio.ssda.testmode=3 \
    persist.radio.ssda.testmode1=10 \
    ro.wcn.gpschip=ge2

# Dual-sim config
PRODUCT_PACKAGES += \
        Stk1 \
        MsmsStk \
        engmoded

ifeq ($(strip $(VOLTE_SERVICE_ENABLE)), true)
    PRODUCT_PROPERTY_OVERRIDES += persist.sys.volte.enable=true
    PRODUCT_PROPERTY_OVERRIDES += persist.sys.volte.mode=Normal
endif

# sprd hw module
PRODUCT_PACKAGES += \
    lights.$(TARGET_BOARD_PLATFORM) \
    sensors.$(TARGET_BOARD_PLATFORM) \
    tinymix \
    audio.primary.$(TARGET_BOARD_PLATFORM) \
    audio_hardware_test \
    camera.$(TARGET_BOARD_PLATFORM) \
    power.$(TARGET_BOARD_PLATFORM) \
    gnss_download \
    libgetmemorysize \
    libreadcamera \
    libdpu

#SANSA|SPRD|NONE
PRODUCT_SECURE_BOOT := NONE
PRODUCT_PACKAGES += imgheaderinsert \
                    packimage.sh \
                    FMRadio \
                    ims \
                    csvt \
            audio_vbc_eq \
                    #FileExplorer \
                    libGLES_android \
                    gralloc.$(TARGET_BOARD_PLATFORM)

#WLAN stuff
PRODUCT_PACKAGES += wpa_supplicant \
            wpa_supplicant.conf \
            wpa_supplicant_overlay.conf \
            hostapd

#wifi UI configs
    PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml
#    frameworks/native/data/etc/android.hardware.wifi.aware.xml:system/etc/permissions/android.hardware.wifi.aware.xml

PRODUCT_PACKAGES += iwnpi \
            libiwnpi \
            sprdwl_ng.ko

# add configuration for vowifi
$(call inherit-product, vendor/sprd/sprd_vowifi/device-vowifi.mk)

#disable refocus
PRODUCT_PROPERTY_OVERRIDES += persist.sys.cam.refocus.enable=false

# config selinux policy
#BOARD_SEPOLICY_DIRS += $(BOARDDIR)/sepolicy

#defalut disable lock screen
PRODUCT_PROPERTY_OVERRIDES += \
    ro.lockscreen.disable.default=true

#Tele config
PRODUCT_PROPERTY_OVERRIDES += \
    ro.moz.ril.numclients=1 \
    ro.moz.ril.query_icc_count=true \
    ro.moz.ril.radio_off_wo_card=true \
    ro.moz.ril.data_reg_on_demand=true \
    ro.moz.ril.ipv6 = true \
    ro.moz.ril.0.network_types=lte \
    ril.ecclist=112,911,000,08,118,110,119,120,122,999 \
    ro.ril.ecclist=112,911,000,08,118,110,119,120,122,999 \
    persist.sys.volte.enable=true

# add KaiOS default property configeration
EXPORT_DEVICE_PREFS := $(BOARDDIR)/default-prefs

# for bootanimation
PRODUCT_COPY_FILES += \
        $(BOARDDIR)/res/bootanimation.zip:system/media/bootanimation.zip
