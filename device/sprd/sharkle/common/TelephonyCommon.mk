# Copyright (C) 2015 The Android Open Source Project
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

PRODUCT_PROPERTY_OVERRIDES += \
    ro.radio.modemtype=l\
    ro.telephony.default_network = 11\
    ro.modem.l.count=1 \
    persist.msms.phone_count=1 \
    persist.radio.multisim.config=ssss \
    keyguard.no_require_sim=true \
    ro.com.android.dataroaming=false \
    ro.simlock.unlock.autoshow=1 \
    ro.simlock.unlock.bynv=0 \
    ro.simlock.onekey.lock=0

$(call inherit-product, build/target/product/telephony.mk)
# telephony modules
PRODUCT_PACKAGES += \
    MmsFolderView \
    messaging \
    SprdStk  \
    SprdDialer \
    CallFireWall \
    UplmnSettings \
    sprdrild \
    librilsprd \
    libsprd-ril \
    librilutils \
    modemd \
    libatci \
    libDivRIL

ifeq ($(BUILD_SPRD_CUSTOM_ANDROID),)

# SPRD: FEATURE_VOLTE_CALLFORWARD_OPTIONS
PRODUCT_PACKAGES += \
    CallSettings \
    CallFireWall

# add sprd Contacts
PRODUCT_PACKAGES += \
    SprdContacts \
    SprdContactsProvider \
    ContactsBlackListAddon \
    ContactsDefaultContactAddon \
    ContactsEFDisplayAddon \
    EFDisplaySupportAddon \
    FastScrollBarSupportAddon

# add mobile tracker
PRODUCT_PACKAGES += \
    SprdMobileTracker

endif #ifeq ($(BUILD_SPRD_CUSTOM_ANDROID),)

# telephony resource
include $(wildcard vendor/sprd/telephony-res/apply_telephony_res.mk)

# sprd cbcustomsetting
#$(call inherit-product, vendor/sprd/platform/packages/apps/CbCustomSetting/etc/device-sprd-cb.mk)
