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

include device/sprd/sharkle/sp9820e_2c10/sp9820e_2c10_base.mk

TARGET_BOOTLOADER_BOARD_NAME := sp9820e_2c10
CHIPRAM_DEFCONFIG := sp9820e_2c10
UBOOT_DEFCONFIG := sp9820e_2c10
UBOOT_TARGET_DTB := sp9820e_2c10_kaios

PRODUCT_NAME := sp9820e_2c10_k_native
PRODUCT_DEVICE := sp9820e_2c10
PRODUCT_BRAND := SPRD
PRODUCT_MODEL := sp9820e_2c10_native
PRODUCT_WIFI_DEVICE := sprd
PRODUCT_MANUFACTURER := SPRD

#DEVICE_PACKAGE_OVERLAYS := $(BOARDDIR)/overlay $(PLATDIR)/overlay $(PLATCOMM)/overlay

#DTS
TARGET_DTB := sp9820e-2c10-kaios-native

BOARD_SECBOOT_CONFIG := true
BOARD_FEATUREPHONE_CONFIG := true
BOARD_KAIOS_CONFIG := true
BOARD_TEE_CONFIG := trusty
include $(PLATCOMM)/security_feature.mk

PRODUCT_COPY_FILES += \
    $(BOARDDIR)/sp9820e_2c10_k.xml:$(PRODUCT_OUT)/sp9820e_2c10.xml

PRODUCT_AAPT_CONFIG := normal large xlarge mdpi 420dpi xxhdpi
PRODUCT_AAPT_PREF_CONFIG := ldpi
PRODUCT_AAPT_PREBUILT_DPI := 320hdpi xhdpi

#  add librecovery.so control
ENABLE_LIBRECOVERY := true

#  add idemia se
BOARD_ESE_VENDOR := idemia


BOARD_ENABLE_OMADM_REDBEND := true


# add libnfccplc for samsung nfc factory test
PRODUCT_PACKAGES += \
  libnfccplc \
  saveNfcCplc


# Enable NFC in gecko
PRODUCT_PROPERTY_OVERRIDES += \
       ro.moz.nfc.enabled=true

PRODUCT_PROPERTY_OVERRIDES += \
       ro.embms.enable=1


# Enable nfcd daemon and nci/hal libraries for samsung.
# add sec nfc

NFC_VENDOR := samsung
PRODUCT_PACKAGES += \
  libnfc-nci \
  nfc_nci.sec \
  nfcd    \
  climax

# Copy NFC configuration files.
PRODUCT_COPY_FILES += \
  vendor/sprd/partner/samsung/KaiosNfc/device/samsung/nfc/libnfc-sec.conf:system/etc/libnfc-sec.conf \
  vendor/sprd/partner/samsung/KaiosNfc/device/samsung/nfc/libnfc-sec-hal.conf:system/etc/libnfc-sec-hal.conf \
  vendor/sprd/partner/samsung/KaiosNfc/device/samsung/nfc/sec_s3nrn82_firmware.bin:system/etc/sec_s3nrn82_firmware.bin \
  vendor/sprd/partner/samsung/KaiosNfc/device/samsung/nfc/sec_s3nrn82_rfreg.bin:system/etc/sec_s3nrn82_rfreg.bin
PRODUCT_COPY_FILES += \
  sps.image/sp9820e_2c10-modem-sign/ltemodem-sign.bin:$(TARGET_OUT)/ltemodem.bin \
  sps.image/sp9820e_2c10-modem-sign/ltedsp-sign.bin:$(TARGET_OUT)/ltedsp.bin \
  sps.image/sp9820e_2c10-modem-sign/ltegdsp-sign.bin:$(TARGET_OUT)/ltegdsp.bin \
  sps.image/sp9820e_2c10-modem-sign/pmsys-sign.bin:$(TARGET_OUT)/pmsys.bin \
  device/sprd/sharkle/sp9820e_2c10/modem_bins/ltenvitem.bin:$(TARGET_OUT)/ltenvitem.bin \
  device/sprd/sharkle/sp9820e_2c10/modem_bins/wcnmodem.bin:$(TARGET_OUT)/wcnmodem.bin \
  device/sprd/sharkle/sp9820e_2c10/modem_bins/gnssmodem.bin:$(TARGET_OUT)/gnssmodem.bin \
  device/sprd/sharkle/sp9820e_2c10/modem_bins/gnssbdmodem.bin:$(TARGET_OUT)/gnssbdmodem.bin \
  device/sprd/sharkle/sp9820e_2c10/modem_bins/kai_240320.bmp:$(TARGET_OUT)/kai_240320.bmp

