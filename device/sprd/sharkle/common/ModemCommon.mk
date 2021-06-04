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
# Modem Functions

PRODUCT_PACKAGES += \
    calibration_init \
    engpc \
    modem_control \
    refnotify \
    slogmodem \
    iqfeed \
    cplogctl \
	autotest \
    cp_diskserver

# Modem Prop
ifeq ($(TARGET_BUILD_VARIANT),user)
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.engpc.disable=1 \
    persist.sys.sprd.modemreset=1
else
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.engpc.disable=1 \
    persist.sys.sprd.modemreset=0
endif # TARGET_BUILD_VARIANT == user


PRODUCT_PROPERTY_OVERRIDES += \
    ro.product.partitionpath=/dev/block/platform/soc/soc:ap-ahb/20600000.sdio/by-name/ \
    ro.modem.l.dev=/proc/cptl/ \
    ro.modem.l.tty=/dev/stty_lte \
    ro.modem.l.eth=seth_lte \
    ro.modem.l.snd=1 \
    ro.modem.l.diag=/dev/sdiag_lte \
    ro.modem.l.log=/dev/slog_lte \
    ro.modem.l.loop=/dev/spipe_lte0 \
    ro.modem.l.nv=/dev/spipe_lte1 \
    ro.modem.l.assert=/dev/spipe_lte2 \
    ro.modem.l.vbc=/dev/spipe_lte6 \
    ro.modem.l.id=0 \
    ro.modem.l.fixnv_size=0xc8000 \
    ro.modem.l.runnv_size=0xe8000 \
    persist.modem.l.nvp=l_ \
    persist.modem.l.enable=1

