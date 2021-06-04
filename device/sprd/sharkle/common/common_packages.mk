# The default product packages these packages will set to trunk/prime products without feature configs
PRODUCT_PACKAGES += \
    FMPlayer \
    SprdRamOptimizer \
    Flashlight \
    FileExplorer \
    NoteBook \
    EngineerMode \
    EngineerInternal \
    ValidationTools \
    DrmProvider \
    CellBroadcastReceiver \
    SprdQuickSearchBox \
    Carddav-Sync \
    SystemUpdate \
    Caldav-Sync.apk

ifeq ($(BUILD_SPRD_CUSTOM_ANDROID),)
PRODUCT_PACKAGES += \
    libsprd_agps_agent\
    FMRadio \
    CarrierConfig \
    UASetting \
    DreamFMRadio \
    DreamSoundRecorder \
    SprdMediaProvider \
    DreamCamera2 \
    QuickCamera \
    Omacp \
    NewGallery2 \
    NewMusic

# TODO Add for not block ptr1
#ifeq ($(BUILD_SPRD_CUSTOM_ANDROID),)
PRODUCT_PACKAGES += \
    SimLockSupport
#endif

# add for secnfc
PRODUCT_PACKAGES += \
    com.android.secnfc

AUTOTEST_PACKAGES += \
    SprdSlt

PRODUCT_PACKAGES_ENG += $(AUTOTEST_PACKAGES)
PRODUCT_PACKAGES_DEBUG += $(AUTOTEST_PACKAGES)

endif #ifeq ($(BUILD_SPRD_CUSTOM_ANDROID),)

ifneq ($(strip $(PDK_FUSION_PLATFORM_ZIP)),)
PRODUCT_PACKAGES += \
	PrebuildCalculator \
	PrebuildCalendar \
	PrebuildCalendarProvider \
	PrebuildContacts \
	PrebuildDeskClock \
	PrebuildDialer \
	PrebuildEmail \
	PrebuildExchange2 \
	PrebuildGallery2 \
	PrebuildMms \
	PrebuildLauncher3 \
	PrebuildMusic \
	PrebuildMusicFX \
	PrebuildTeleService \
	PrebuildChrome \
	PrebuildSoundRecorder \
	PrebuildFileExplorer \
	PrebuildEngineerMode \
	PrebuildTelecom \
	PrebuildGmail \
	PrebuildKeyChain \
	PrebuildMmsService \
	LegacyCamera.apk \
	CustomLocale.apk \
	Development.apk \
	DevelopmentSettings.apk \
        libjni_legacymosaic \
	Prebuildbootanimation \
	PrebuildDocumentsUI
endif

ifeq ($(BUILD_SPRD_CUSTOM_ANDROID),)
#add for thermalmanager
PRODUCT_PACKAGES += sprdthermal
#endif

#add for F2FS
ifeq (f2fs,$(strip $(BOARD_USERDATAIMAGE_FILE_SYSTEM_TYPE)))
TARGET_USERIMAGES_USE_F2FS := true
PRODUCT_PACKAGES += libf2fs libf2fs_static libf2fs_host \
                    mkfs.f2fs libf2fs_mkfs_static mkfs.f2fs_recovery \
                    fsck.f2fs libf2fs_fsck_static \
                    libf2fs_fmt-host libf2fs_fmt_host_dyn
PRODUCT_PACKAGES += libf2fs_ioutils_host \
                    libf2fs_dlutils_host libf2fs_dlutils_static libf2fs_dlutils \
                    libf2fs_sparseblock f2fs_sparseblock \
                    libf2fs_utils_host libf2fs_utils_static \
                    make_f2fs mkf2fsuserimg.sh
PRODUCT_PACKAGES += simg2img_host img2simg_host
endif

endif #ifeq ($(BUILD_SPRD_CUSTOM_ANDROID),)

