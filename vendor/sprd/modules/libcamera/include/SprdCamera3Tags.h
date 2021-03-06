#ifndef ANDROiD_SERVERS_CAMERA_CAMERA2TAG_H
#define ANDROiD_SERVERS_CAMERA_CAMERA2TAG_H
namespace android {

typedef enum sprd_camera_ext_tags {
    /*sprd add flag start*/
    ANDROID_SPRD_BRIGHTNESS = VENDOR_SECTION_START,
    ANDROID_SPRD_CONTRAST,
    ANDROID_SPRD_SATURATION,
    ANDROID_SPRD_CAPTURE_MODE,
    ANDROID_SPRD_SENSOR_ORIENTATION,
    ANDROID_SPRD_SENSOR_ROTATION,
    ANDROID_SPRD_UCAM_SKIN_LEVEL,
    ANDROID_SPRD_BURST_CAP_CNT,
    ANDROID_SPRD_REC_BUF_INFO,
    ANDROID_SPRD_ISO,
    ANDROID_SPRD_SLOW_MOTION,
    ANDROID_SPRD_METERING_MODE,
    ANDROID_SPRD_METERING_AREA,
    ANDROID_SPRD_VIDEO_SNAPSHOT_SUPPORT,
    ANDROID_SPRD_AVAILABLE_BRIGHTNESS,
    ANDROID_SPRD_AVAILABLE_CONTRAST,
    ANDROID_SPRD_AVAILABLE_SATURATION,
    ANDROID_SPRD_AVAILABLE_CAPTURE_MODE,
    ANDROID_SPRD_AVAILABLE_ISO,
    ANDROID_SPRD_AVAILABLE_SLOW_MOTION,
    ANDROID_SPRD_AVAILABLE_METERING_MODE,
    ANDROID_SPRD_FLASH_MODE_SUPPORT,
    ANDROID_SPRD_PRV_REC_DIFFERENT_SIZE_SUPPORT,
    ANDROID_SPRD_NOTIFY_FLAG_REC_SYNC,
    ANDROID_SPRD_ZSL_ENABLED,
    ANDROID_SPRD_CONTROL_FRONT_CAMERA_MIRROR,
    ANDROID_SPRD_EIS_ENABLED,
    ANDROID_SPRD_EIS_CROP,
    ANDROID_SPRD_PIPVIV_ENABLED,
    ANDROID_SPRD_HIGHISO_ENABLED,
    ANDROID_SPRD_AVAILABLE_SMILEENABLE,
    ANDROID_SPRD_AVAILABLE_ANTIBAND_AUTOSUPPORTED,
    ANDROID_SPRD_CONTROL_REFOCUS_ENABLE,
    ANDROID_SPRD_SET_TOUCH_INFO,
    ANDROID_SPRD_IS_SUPPORT_REFOCUS,
    ANDROID_SPRD_AF_MODE_MACRO_FIXED,
    ANDROID_SPRD_VCM_STEP,
    ANDROID_SPRD_OTP_DATA,
    ANDROID_SPRD_DUAL_OTP_FLAG,
    ANDROID_SPRD_3DCALIBRATION_ENABLED,
    ANDROID_SPRD_3DCALIBRATION_CAPTURE_SIZE,
    ANDROID_SPRD_BURSTMODE_ENABLED,
    ANDROID_SPRD_3D_RANGEFINDER_DISTANCE,
    ANDROID_SPRD_3D_RANGEFINDER_POINTS,
    ANDROID_SPRD_MULTI_CAM3_PREVIEW_ID,
    ANDROID_SPRD_BLUR_F_NUMBER,
    ANDROID_SPRD_BLUR_COVERED,
    ANDROID_SPRD_AVAILABLE_SENSOR_SELF_SHOT,
    ANDROID_SPRD_BLUR_CIRCLE_SIZE,
    ANDROID_SPRD_MAX_PREVIEW_SIZE,
    ANDROID_SPRD_SENSOR_ROTATION_FOR_FRONT_BLUR,
    ANDROID_SPRD_HDR_PLUS_ENABLED,
    ANDROID_SPRD_FIXED_FPS_ENABLED,
    ANDROID_SPRD_3DNR_ENABLED,
    ANDROID_SPRD_FILTER_TYPE,
    ANDROID_SPRD_IS_TAKEPICTURE_WITH_FLASH,
    ANDROID_SPRD_FAST_THUMB,
    ANDROID_SPRD_AVAILABLE_FLASH_LEVEL,
    ANDROID_SPRD_ADJUST_FLASH_LEVEL,
    VENDOR_SECTION_END,
    /*not parameter but only flag between framework and hal*/
    /*sprd add flag end*/
} sprd_camera_metadata_tag_t;
}

// ANDROID_SPRD_VIDEO_SNAPSHOT_SUPPORT
typedef enum camera_metadata_enum_android_sprd_video_snapshot_support {
    ANDROID_SPRD_VIDEO_SNAPSHOT_SUPPORT_OFF,
    ANDROID_SPRD_VIDEO_SNAPSHOT_SUPPORT_ON,
} camera_metadata_enum_android_sprd_video_snapshot_support_t;
#endif
