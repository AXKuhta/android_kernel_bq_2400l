ifeq ($(BUILD_SPRD_CUSTOM_ANDROID),)
FEATURES.PRODUCT_PACKAGES += \
    ShakePhoneToStartRecording \
    SpeakerToHeadset \
    FlipToMute \
    FadeDownRingtoneToVibrate \
    PickUpToAnswerIncomingCall \
    MaxRingingVolumeAndVibrate
endif
