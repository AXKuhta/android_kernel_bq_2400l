LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= cmd_services.c

LOCAL_SHARED_LIBRARIES := libcutils libc liblog

LOCAL_INIT_RC := cmd_services.rc

LOCAL_MODULE := cmd_services

LOCAL_MODULE_TAGS := optional

LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)

CUSTOM_MODULES += cmd_services
