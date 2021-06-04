# The Spreadtrum Communication Inc. 2016

ifdef VENDOR_PLATFORM

localDebug :=

LOCAL_VENDOR_PATH := $(wildcard $(VENDOR_PLATFORM)/$(LOCAL_PATH))
# Don't afriad of the Android.mk is under $(VENDOR_PLATFORM)

ifdef LOCAL_VENDOR_PATH

# Prepare vendor hooks, and define the LOCAL_VENDOR_RELATIVE_PATH hook the additional.mk
include $(wildcard $(SPRD_BUILD_SYSTEM)/prepare_hook.mk)

# ---------------- raw hook of package.mk ----------------------


# 1. Hook additional src files

include $(wildcard $(SPRD_BUILD_SYSTEM)/subcmds/gather_c_files.mk)

# 2. Hook the proguard.flags

include $(wildcard $(SPRD_BUILD_SYSTEM)/subcmds/gather_c_includes.mk)


endif # If define LOCAL_VENDOR_PATH

include $(wildcard $(SPRD_BUILD_SYSTEM)/prepare_hook_done.mk)

endif # If define VENDOR_PLATFORM

