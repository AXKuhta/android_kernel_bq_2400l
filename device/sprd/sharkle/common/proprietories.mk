PROPRIETARY_BOARD := sharkle

#ifneq ($(shell ls -d vendor/sprd/proprietories-source 2>/dev/null),)
# for spreadtrum internal proprietories modules: only support package module names

#not for haps
ifeq ($(BUILD_SPRD_CUSTOM_ANDROID),)

#FIXME: C99[-Werror,-Wimplicit-function-declaration]
PRODUCT_PACKAGES :=              \
    libomx_m4vh263dec_sw_sprd    \
    libomx_avcdec_sw_sprd        \
    libomx_avcdec_hw_sprd        \
    libomx_avcenc_hw_sprd        \
    libomx_mp3dec_sprd           \
    libomx_mp3enc_sprd           \
    thermald

# add for volte
ifeq ($(strip $(VOLTE_SERVICE_ENABLE)), true)
# video call engine modules
PRODUCT_PACKAGES += \
        libVideoCallEngineService \
        libVideoCallEngineClient \
        libVideoCallEngine \
        libvideo_call_engine_jni \
        modemDriver_vpad_main
endif

ifeq ($(strip $(VOWIFI_SERVICE_ENABLE)), true)
# Add for vowifi
# add for vowifi source code module
PRODUCT_PACKAGES += \
    SprdVoWifiService \
    ImsCM \
    operator_info.xml \
    operator_config.xml

#ims bridge daemon
PRODUCT_PACKAGES += \
    ims_bridged \
    libimsbrd

#ip monitor
PRODUCT_PACKAGES += ip_monitor.sh

# add for vowifi bin module
PRODUCT_PACKAGES += vowifi_sdk \
    libavatar \
    libzmf \
    libCamdrv24 \
    liblemon \
    libmme_jrtc \
    ju_ipsec_server
endif

# for spreadtrum customer proprietories modules: only support direct copy
#FIXME: C99[-Werror,-Wimplicit-function-declaration]
PROPMODS :=                                     \
    system/lib/libomx_m4vh263dec_sw_sprd.so     \
    system/lib/libomx_avcdec_sw_sprd.so         \
    system/lib/libomx_avcdec_hw_sprd.so         \
    system/lib/libomx_avcenc_hw_sprd.so         \
    system/lib/libomx_mp3dec_sprd.so            \
    system/lib/libomx_mp3enc_sprd.so            \
    system/vendor/bin/thermald                  \
    system/bin/ylog                             \
    system/bin/ylog_cli                         \
    system/bin/ylog_benchmark                   \
    system/bin/ylog_benchmark_socket_server     \
    system/bin/ylogd                            \
    system/bin/exfatfsck                        \
    system/bin/mkfsexfat

#config video engine for volte video call
ifeq ($(strip $(VOLTE_SERVICE_ENABLE)), true)
# video call engine modules
PROPMODS += \
        system/lib/libvideo_2_0.so \
        system/bin/modemDriver_vpad_main

# For idh script to copy libvideo_2_0_intermediates folder to IDH package
BOARD_LIBVIDEO_2_0_SHARED_LIBRARY := true
endif

endif #ifeq ($(BUILD_SPRD_CUSTOM_ANDROID),)
