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

include device/sprd/sharkle/sp9820e_2h10/sp9820e_2h10_base.mk

TARGET_BOOTLOADER_BOARD_NAME := sp9820e_2h10
CHIPRAM_DEFCONFIG := sp9820e_2h10
UBOOT_DEFCONFIG := sp9820e_2h10
UBOOT_TARGET_DTB := sp9820e_2h10_kaios

PRODUCT_NAME := sp9820e_2h10_k_native
PRODUCT_DEVICE := sp9820e_2h10
PRODUCT_BRAND := SPRD
PRODUCT_MODEL := sp9820e_2h10_native
PRODUCT_WIFI_DEVICE := sprd
PRODUCT_MANUFACTURER := SPRD

#DEVICE_PACKAGE_OVERLAYS := $(BOARDDIR)/overlay $(PLATDIR)/overlay $(PLATCOMM)/overlay

#DTS
TARGET_DTB := sp9820e-2h10-kaios-native

BOARD_FEATUREPHONE_CONFIG := true
BOARD_KAIOS_CONFIG := true
BOARD_TEE_CONFIG := trusty
include $(PLATCOMM)/security_feature.mk

PRODUCT_COPY_FILES += \
    $(BOARDDIR)/sp9820e_2h10_k.xml:$(PRODUCT_OUT)/sp9820e_2h10.xml

PRODUCT_AAPT_CONFIG := normal large xlarge mdpi 420dpi xxhdpi
PRODUCT_AAPT_PREF_CONFIG := ldpi
PRODUCT_AAPT_PREBUILT_DPI := 320hdpi xhdpi

#  add librecovery.so control
ENABLE_LIBRECOVERY := true

# add libnfccplc for NXP nfc factory test
PRODUCT_PACKAGES += \
  libnfccplc \
  saveNfcCplc

# Enable NFC in gecko
PRODUCT_PROPERTY_OVERRIDES += \
       ro.moz.nfc.enabled=true

#add PN80T(NFC)
NFC_VENDOR := nxp
PRODUCT_PACKAGES += \
        nfc_nci.pn54x.default \
        libnfc-brcm.conf \
        libnfc-nxp.conf \
        libnfc-nxp_RF.conf\
        libpn553_fw.so

# Enable nfcd daemon and nci/hal libraries for nxp.
PRODUCT_PACKAGES += \
  libnfc-nci \
  libnfc-mifare \
  libese-jcop-kit \
  nfcd

# Copy NFC configuration files.
PRODUCT_COPY_FILES += \
    external/libnfc-nci/halimpl/pn54x/libnfc-nxp-PN80S_sd_example.conf:system/etc/libnfc-nxp.conf \
    external/libnfc-nci/halimpl/pn54x/libnfc-nxp_RF-PN80S_example.conf:system/etc/libnfc-nxp_RF.conf \
    external/libnfc-nci/halimpl/pn54x/libnfc-nxp_RF-PN80S_example.conf:system/vendor/libnfc-nxp_RF.conf \
    external/libnfc-nci/halimpl/pn54x/libnfc-brcm.conf:system/etc/libnfc-brcm.conf \
    external/libnfc-nci/halimpl/pn54x/firmware/libpn553_fw.so:system/vendor/firmware/libpn553_fw.so
