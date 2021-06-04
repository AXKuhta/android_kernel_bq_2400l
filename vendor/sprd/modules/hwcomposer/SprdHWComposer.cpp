/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/******************************************************************************
 **                   Edit    History                                         *
 **---------------------------------------------------------------------------*
 ** DATE          Module              DESCRIPTION                             *
 ** 22/09/2013    Hardware Composer   Responsible for processing some         *
 **                                   Hardware layers. These layers comply    *
 **                                   with display controller specification,  *
 **                                   can be displayed directly, bypass       *
 **                                   SurfaceFligner composition. It will     *
 **                                   improve system performance.             *
 ******************************************************************************
 ** File: SprdHWComposer.cpp          DESCRIPTION                             *
 **                                   comunicate with SurfaceFlinger and      *
 **                                   other class objects of HWComposer       *
 ******************************************************************************
 ******************************************************************************
 ** Author:         zhongjun.chen@spreadtrum.com                              *
 *****************************************************************************/

#include "SprdHWComposer.h"
#include "AndroidFence.h"

using namespace android;

void SprdHWComposer::resetDisplayAttributes() {
  for (int i = 0; i < MAX_DISPLAYS; i++) {
    DisplayAttributes *dpyAttr = &(mDisplayAttributes[i]);
    dpyAttr->configsIndex = 0;
    dpyAttr->connected = false;
    dpyAttr->AcceleratorMode = ACCELERATOR_NON;
  }
}

bool SprdHWComposer::Init() {
  resetDisplayAttributes();

#ifdef HWC_SUPPORT_FBD_DISPLAY
  mDisplayCore = new SprdFrameBufferDevice();
#else
  mDisplayCore = new SprdADFWrapper();
#endif
  if (mDisplayCore == NULL) {
    ALOGE("new SprdDisplayCore failed");
    return false;
  }

  if (!(mDisplayCore->Init())) {
    ALOGE("SprdDisplayCore Init failed");
    return false;
  }

  /*
   *  SprdPrimaryDisplayDevice information
   * */
  mPrimaryDisplay = new SprdPrimaryDisplayDevice();
  if (mPrimaryDisplay == NULL) {
    ALOGE("new SprdPrimaryDisplayDevice failed");
    return false;
  }

  if (!(mPrimaryDisplay->Init(mDisplayCore))) {
    ALOGE("mPrimaryDisplayDevice init failed");
    return false;
  }

  /*
   *  SprdExternalDisplayDevice information
   * */
  mExternalDisplay = new SprdExternalDisplayDevice();
  if (mExternalDisplay == NULL) {
    ALOGE("new mExternalDisplayDevice failed");
    return false;
  }

  if (!(mExternalDisplay->Init(mDisplayCore))) {
    ALOGE("mExternalDisplay Init failed");
    return false;
  }

  /*
   *  SprdVirtualDisplayDevice information
   * */
  mVirtualDisplay = new SprdVirtualDisplayDevice();
  if (mVirtualDisplay == NULL) {
    ALOGE("new mVirtualDisplayDevice failed");
    return false;
  }

  if ((mVirtualDisplay->Init() != 0)) {
    ALOGE("VirtualDisplay Init failed");
    return false;
  }

  openSprdFence();

  mInitFlag = 1;

  return true;
}

SprdHWComposer::~SprdHWComposer() {
  closeSprdFence();

  if (mPrimaryDisplay) {
    delete mPrimaryDisplay;
    mPrimaryDisplay = NULL;
  }

  if (mExternalDisplay) {
    delete mExternalDisplay;
    mExternalDisplay = NULL;
  }

  if (mVirtualDisplay) {
    delete mVirtualDisplay;
    mVirtualDisplay = NULL;
  }
  mInitFlag = 0;
}

int SprdHWComposer::DevicePropertyProbe(size_t numDisplays,
                                        hwc_display_contents_1_t **displays) {
  int ret = -1;
  for (unsigned int i = 0; i < numDisplays; i++) {
    hwc_display_contents_1_t *display = displays[i];

    switch (i) {
      case DISPLAY_PRIMARY:
        mDisplayAttributes[DISPLAY_PRIMARY].AcceleratorMode |=
            ACCELERATOR_DISPC;
        mDisplayAttributes[DISPLAY_PRIMARY].AcceleratorMode |= ACCELERATOR_GSP;
        mDisplayAttributes[DISPLAY_PRIMARY].AcceleratorMode |=
            ACCELERATOR_OVERLAYCOMPOSER;
        ret = 0;
        break;
      case DISPLAY_EXTERNAL:
        ret = 0;
        break;
      case DISPLAY_VIRTUAL:
        if (display != NULL) {
          mDisplayAttributes[DISPLAY_VIRTUAL].AcceleratorMode |=
              ACCELERATOR_GSP;
          // mDisplayAttributes[DISPLAY_PRIMARY].AcceleratorMode &=
          // ~ACCELERATOR_GSP;
        }
        ret = 0;
        break;
      default:
        ret = -EINVAL;
    }
  }

  return ret;
}

int SprdHWComposer::prepareDisplays(size_t numDisplays,
                                    hwc_display_contents_1_t **displays) {
  int ret = 0;
  queryDebugFlag(&mDebugFlag);

#ifdef FORCE_ADJUST_ACCELERATOR
  DevicePropertyProbe(numDisplays, displays);
#endif

  for (unsigned int i = 0; i < numDisplays; i++) {
    hwc_display_contents_1_t *display = displays[i];

    switch (i) {
      case DISPLAY_PRIMARY:
        mPrimaryDisplay->prepare(
            display, mDisplayAttributes[DISPLAY_PRIMARY].AcceleratorMode);
        break;
      case DISPLAY_EXTERNAL:
        mExternalDisplay->prepare(
            display, mDisplayAttributes[DISPLAY_EXTERNAL].AcceleratorMode);
        break;
      case DISPLAY_VIRTUAL:
        mVirtualDisplay->prepare(
            display, mDisplayAttributes[DISPLAY_VIRTUAL].AcceleratorMode);
        break;
      default:
        ret = -EINVAL;
    }
  }

  return ret;
}

int SprdHWComposer::commitDisplays(size_t numDisplays,
                                   hwc_display_contents_1_t **displays) {
  int ret = 0;

  hwc_display_contents_1_t *VDList = NULL;

  for (int i = numDisplays - 1; i >= 0; i--) {
    hwc_display_contents_1_t *display = displays[i];

    switch (i) {
      case DISPLAY_PRIMARY:
        mPrimaryDisplay->commit(display);
        break;
      case DISPLAY_EXTERNAL:
        mExternalDisplay->commit(display);
        break;
      case DISPLAY_VIRTUAL:
        mVirtualDisplay->commit(display);
        VDList = display;
        break;
      default:
        ret = -EINVAL;
    }
  }

  if (VDList) {
    mVirtualDisplay->WaitForDisplay();
  }
  DisplayTrack tracker;
  tracker.releaseFenceFd = -1;
  tracker.retiredFenceFd = -1;
  ret = mDisplayCore->PostDisplay(&tracker);

  /*
   *  Build Sync data for each display device
   * */
  for (unsigned int i = 0; i < numDisplays; i++) {
    hwc_display_contents_1_t *display = displays[i];

    switch (i) {
      case DISPLAY_PRIMARY:
	mPrimaryDisplay->buildSyncData(display, &tracker);
        break;
      case DISPLAY_EXTERNAL:
        mExternalDisplay->buildSyncData(display, &tracker);
        break;
      case DISPLAY_VIRTUAL:
        mVirtualDisplay->buildSyncData(display, &tracker);
        break;
      default:
        ret = -EINVAL;
    }
  }

  /*
   *  Recycle file descriptor
   * */
  ALOGI_IF(mDebugFlag, "<11> close DisplayCore output rel_fd:%d,retire_fd:%d.",
           tracker.releaseFenceFd, tracker.retiredFenceFd);
  closeFence(&tracker.releaseFenceFd);
  closeFence(&tracker.retiredFenceFd);

  return ret;
}

int SprdHWComposer::blank(int disp, int blank) {
  int ret = 0;
  // queryDebugFlag(&mDebugFlag);
  ALOGI_IF(mDebugFlag, "%s : %s display:%d", __func__,
           (blank == 1) ? "Blanking" : "UnBlanking", disp);

  ret = mDisplayCore->Blank(disp, blank);
  if (blank) {
    /*
     *  Here, we need free up all the Overlay Plane and
     *  Primary plane.
     * */
  }

  return 0;
}

int SprdHWComposer::query(int what, int *value) {
  HWC_IGNORE(what);
  HWC_IGNORE(value);
  return 0;
}

void SprdHWComposer::dump(char *buff, int buff_len) {
  HWC_IGNORE(buff);
  HWC_IGNORE(buff_len);
}

int SprdHWComposer::getDisplayConfigs(int disp, uint32_t *configs,
                                      size_t *numConfigs) {
  int ret = -1;
  int size = 0;
  uint32_t *p = NULL;

  if (mDisplayCore->GetConfigs(disp, configs, numConfigs)) {
    ALOGE("SprdHWComposer:: getDisplayConfigs disp: %d failed", disp);
    return -1;
  }

  switch (disp) {
    case DISPLAY_PRIMARY:
      p = mDisplayAttributes[DISPLAY_PRIMARY].configIndexSets;
      size = (*numConfigs > MAX_NUM_CONFIGS) ? MAX_NUM_CONFIGS : *numConfigs;
      memcpy(p, configs, size);
      mDisplayAttributes[DISPLAY_PRIMARY].numConfigs = size;
      ret = 0;
      break;
    case DISPLAY_EXTERNAL:
      ret = 0;
      break;
    default:
      return ret;
  }

  return ret;
}

int SprdHWComposer::getDisplayAttributes(int disp, uint32_t config,
                                         const uint32_t *attributes,
                                         int32_t *value) {
  if (DISPLAY_EXTERNAL == disp && !(mDisplayAttributes[disp].connected)) {
    // ALOGD("External Display Device is not connected");
    return -1;
  }

  if (DISPLAY_VIRTUAL == disp && !(mDisplayAttributes[disp].connected)) {
    // ALOGD("VIRTUAL Display Device is not connected");
    return -1;
  }

  if (mDisplayCore->GetConfigAttributes(disp, config, attributes, value) < 0) {
    ALOGE("SprdHWComposer:: getDisplayAttributes get Config attributes error");
    return -1;
  }

  AttributesSet *dpyAttr = &(mDisplayAttributes[disp].sets[config]);
  bool is_end = false;

  for (unsigned int i = 0; i < NUM_DISPLAY_ATTRIBUTES - 1; i++) {
    switch (attributes[i]) {
      case HWC_DISPLAY_VSYNC_PERIOD:
        value[i] = value[i] * 1000;
        dpyAttr->vsync_period = value[i];
        ALOGI("getDisplayAttributes: disp:%d vsync_period:%d", disp,
              dpyAttr->vsync_period);
        if (dpyAttr->vsync_period == 0) {
          ALOGI(
              "getDisplayAttributes: disp:%d vsync_period:0,set to default "
              "60fps.",
              disp);
          dpyAttr->vsync_period = (1e9 / 60);
        }
        break;
      case HWC_DISPLAY_WIDTH:
        dpyAttr->xres = value[i];
        ALOGI("getDisplayAttributes: disp:%d width:%d", disp, dpyAttr->xres);
        break;
      case HWC_DISPLAY_HEIGHT:
        dpyAttr->yres = value[i];
        ALOGI("getDisplayAttributes: disp:%d height:%d", disp, dpyAttr->yres);
        break;
      case HWC_DISPLAY_DPI_X:
        dpyAttr->xdpi = (float)(value[i]);
        if (dpyAttr->xdpi == 0) {
          // the driver doesn't return that information
          // default to 160 dpi
          dpyAttr->xdpi = 160;
        }
        ALOGI("getDisplayAttributes: disp:%d xdpi:%f", disp, dpyAttr->xdpi);
        break;
      case HWC_DISPLAY_DPI_Y:
        dpyAttr->ydpi = (float)(value[i]);
        if (dpyAttr->ydpi == 0) {
          // the driver doesn't return that information
          // default to 160 dpi
          dpyAttr->ydpi = 160;
        }
        ALOGI("getDisplayAttributes: disp:%d ydpi:%f", disp, dpyAttr->ydpi);
        break;
#ifdef GET_FRAMEBUFFER_FORMAT_FROM_HWC
      case HWC_DISPLAY_FBFORMAT:
#ifdef EN_RGB_565
        value[i] = HAL_PIXEL_FORMAT_RGB_565;
#else
        value[i] = HAL_PIXEL_FORMAT_RGBA_8888;
#endif
        ALOGI("getDisplayAttributes HWC_DISPLAY_FBFORMAT: FORMAT:%d", value[i]);
        break;
#endif
      case HWC_DISPLAY_NO_ATTRIBUTE:
        is_end = true;
        ALOGI("getDisplayAttributes: HWC_DISPLAY_NO_ATTRIBUTE");
        break;
      default:
        ALOGE("Unknown Display Attributes:%d", attributes[i]);
        return -EINVAL;
    }

    if (is_end)
    {
      ALOGI("getDisplayAttributes end, i=%d", i);
      break;
    }
  }

  switch (disp) {
    case DISPLAY_PRIMARY:
      mPrimaryDisplay->syncAttributes(dpyAttr);
      mDisplayAttributes[disp].connected = true;
      break;
    case DISPLAY_EXTERNAL:
      mExternalDisplay->syncAttributes(dpyAttr);
      // mDisplayAttributes[disp].connected = true;
      break;
    case DISPLAY_VIRTUAL:
      mVirtualDisplay->syncAttributes(dpyAttr);
      // mDisplayAttributes[disp].connected = true;
      break;
    default:
      ALOGE(
          "SprdHWComposer:: getDisplayAttributes do not support display type "
          "%d",
          disp);
      break;
  }

  return 0;
}

int SprdHWComposer::getActiveConfig(int disp) {
  int ret = -1;

  switch (disp) {
    case DISPLAY_PRIMARY:
      ret = mDisplayAttributes[DISPLAY_PRIMARY].configsIndex;
      break;
    case DISPLAY_EXTERNAL:
      if (mDisplayAttributes[DISPLAY_EXTERNAL].connected) {
        ret = mDisplayAttributes[DISPLAY_EXTERNAL].configsIndex;
      }
      break;
    default:
      return ret;
  }

  return ret;
}

int SprdHWComposer::setActiveConfig(int disp, int index) {
  int ret = -1;

  if (index < 0) {
    ALOGE("setActiveConfig input index is invalid");
    return -1;
  }

  switch (disp) {
    case DISPLAY_PRIMARY:
      mDisplayAttributes[DISPLAY_PRIMARY].configsIndex = index;
      ret =
          mPrimaryDisplay->ActiveConfig(&(mDisplayAttributes[DISPLAY_PRIMARY]));
      break;
    case DISPLAY_EXTERNAL:
      if (mDisplayAttributes[DISPLAY_EXTERNAL].connected) {
        mDisplayAttributes[DISPLAY_EXTERNAL].configsIndex = index;
        ret = mExternalDisplay->ActiveConfig(
            &(mDisplayAttributes[DISPLAY_EXTERNAL]));
      }
      break;
    default:
      return ret;
  }

  if (ret < 0) {
    ALOGE("setActiveConfig display: %d ActiveConfig failed", disp);
  }

  return ret;
}

int SprdHWComposer::setPowerMode(int disp, int mode) {
  int ret = -1;

  switch (disp) {
    case DISPLAY_PRIMARY:
      ret = mPrimaryDisplay->setPowerMode(mode);
      break;
    case DISPLAY_EXTERNAL:
      ret = mExternalDisplay->setPowerMode(mode);
      break;
    default:
      return ret;
  }

  if (ret < 0) {
    ALOGE("SprdHWComposer:: setPowerMode display: %d, failed", disp);
  }

  return ret;
}

int SprdHWComposer::setCursorPositionAsync(int disp, int x_pos, int y_pos) {
  int ret = -1;

  switch (disp) {
    case DISPLAY_PRIMARY:
      ret = mPrimaryDisplay->setCursorPositionAsync(x_pos, y_pos);
      break;
    case DISPLAY_VIRTUAL:
      ret = mVirtualDisplay->setCursorPositionAsync(x_pos, y_pos);
    case DISPLAY_EXTERNAL:
      if (mDisplayAttributes[DISPLAY_EXTERNAL].connected) {
        ret = mExternalDisplay->setCursorPositionAsync(x_pos, y_pos);
      }
      break;
    default:
      return ret;
  }

  return ret;
}

int SprdHWComposer::getBuiltInDisplayNum(uint32_t *number) {
  int ret = -1;

  ret = mPrimaryDisplay->getBuiltInDisplayNum(number);

  return ret;
}

void SprdHWComposer::registerProcs(hwc_procs_t const *procs) {
  /*
   *  At present, the Android callback procs is just
   *  used by Primary Display Device.
   *  Other Display Device do not generate vsync event.
   * */
  mDisplayCore->SetEventProcs(procs);
}

bool SprdHWComposer::eventControl(int disp, int event, int enabled) {
  /*
   *  At present, only Primary Display Device support
   *  Hardware vsync event.
   * */
  mDisplayCore->EventControl(disp, event, enabled);

  return true;
}

/*
 *  HWC module info
 * */

static int hwc_eventControl(hwc_composer_device_1 *dev, int disp, int event,
                            int enabled) {
  int status = -EINVAL;
  bool ret = false;

  SprdHWComposer *HWC = static_cast<SprdHWComposer *>(dev);
  if (HWC == NULL) {
    ALOGE("Can NOT get SprdHWComposer reference");
    return status;
  }

  ret = HWC->eventControl(disp, event, enabled);
  if (!ret) {
    ALOGE("Vsync event control failed");
    status = -EPERM;
    return status;
  }

  return 0;
}

static int hwc_blank(hwc_composer_device_1 *dev, int disp, int blank) {
  int status = -EINVAL;

  SprdHWComposer *HWC = static_cast<SprdHWComposer *>(dev);
  if (HWC == NULL) {
    ALOGE("Can NOT get SprdHWComposer reference");
    return status;
  }

  status = HWC->blank(disp, blank);

  return status;
}

static int hwc_query(hwc_composer_device_1 *dev, int what, int *value) {
  int status = -EINVAL;

  SprdHWComposer *HWC = static_cast<SprdHWComposer *>(dev);
  if (HWC == NULL) {
    ALOGE("Can NOT get SprdHWComposer reference");
    return status;
  }

  status = HWC->query(what, value);

  return status;
}

static void hwc_dump(hwc_composer_device_1 *dev, char *buff, int buff_len) {
  SprdHWComposer *HWC = static_cast<SprdHWComposer *>(dev);
  if (HWC == NULL) {
    ALOGE("Can NOT get SprdHWComposer reference");
    return;
  }

  HWC->dump(buff, buff_len);
}

static int hwc_getDisplayConfigs(hwc_composer_device_1 *dev, int disp,
                                 uint32_t *configs, size_t *numConfigs) {
  int status = -EINVAL;

  SprdHWComposer *HWC = static_cast<SprdHWComposer *>(dev);
  if (HWC == NULL) {
    ALOGE("Can NOT get SprdHWComposer reference");
    return status;
  }

  status = HWC->getDisplayConfigs(disp, configs, numConfigs);

  return status;
}

static int hwc_getDisplayAttributes(hwc_composer_device_1 *dev, int disp,
                                    uint32_t config, const uint32_t *attributes,
                                    int32_t *value) {
  int status = -EINVAL;

  SprdHWComposer *HWC = static_cast<SprdHWComposer *>(dev);
  if (HWC == NULL) {
    ALOGE("Can NOT get SprdHWComposer reference");
    return status;
  }

  status = HWC->getDisplayAttributes(disp, config, attributes, value);

  return status;
}

static int hwc_getActiveConfig(hwc_composer_device_1 *dev, int disp) {
  int status = -EINVAL;

  SprdHWComposer *HWC = static_cast<SprdHWComposer *>(dev);
  if (HWC == NULL) {
    ALOGE("Can NOT get SprdHWComposer reference");
    return status;
  }

  status = HWC->getActiveConfig(disp);

  return status;
}

static int hwc_setActiveConfig(hwc_composer_device_1 *dev, int disp,
                               int index) {
  int status = -EINVAL;

  SprdHWComposer *HWC = static_cast<SprdHWComposer *>(dev);
  if (HWC == NULL) {
    ALOGE("Can NOT get SprdHWComposer reference");
    return status;
  }

  status = HWC->setActiveConfig(disp, index);

  return status;
}

static int hwc_setPowerMode(hwc_composer_device_1 *dev, int disp, int mode) {
  int status = -EINVAL;

  SprdHWComposer *HWC = static_cast<SprdHWComposer *>(dev);
  if (HWC == NULL) {
    ALOGE("Can NOT get SprdHWComposer reference");
    return status;
  }

  status = HWC->setPowerMode(disp, mode);

  return status;
}

static int hwc_setCursorPositionAsync(hwc_composer_device_1 *dev, int disp,
                                      int x_pos, int y_pos) {
  int status = -EINVAL;

  SprdHWComposer *HWC = static_cast<SprdHWComposer *>(dev);
  if (HWC == NULL) {
    ALOGE("Can NOT get SprdHWComposer reference");
    return status;
  }

  status = HWC->setCursorPositionAsync(disp, x_pos, y_pos);

  return status;
}

static int hwc_getBuiltInDisplayNum(hwc_composer_device_1 *dev,
                                    uint32_t *number) {
  int status = -EINVAL;

  SprdHWComposer *HWC = static_cast<SprdHWComposer *>(dev);
  if (HWC == NULL) {
    ALOGE("Can NOT get SprdHWComposer reference");
    return status;
  }

  status = HWC->getBuiltInDisplayNum(number);

  return status;
}

static int hwc_prepareDisplays(hwc_composer_device_1 *dev, size_t numDisplays,
                               hwc_display_contents_1_t **displays) {
  int status = -EINVAL;

  SprdHWComposer *HWC = static_cast<SprdHWComposer *>(dev);
  if (HWC == NULL) {
    ALOGE("Can NOT get SprdHWComposer reference");
    return status;
  }

  status = HWC->prepareDisplays(numDisplays, displays);

  return status;
}

static int hwc_setDisplays(hwc_composer_device_1 *dev, size_t numDisplays,
                           hwc_display_contents_1_t **displays) {
  int status = -EINVAL;

  SprdHWComposer *HWC = static_cast<SprdHWComposer *>(dev);
  if (HWC == NULL) {
    ALOGE("Can NOT get SprdHWComposer reference");
    return status;
  }

  status = HWC->commitDisplays(numDisplays, displays);

  return status;
}

static int hwc_device_close(struct hw_device_t *dev) {
  SprdHWComposer *HWC = (SprdHWComposer *)(dev);
  if (HWC != NULL) {
    delete HWC;
    HWC = NULL;
  }

  return 0;
}

static void hwc_registerProcs(hwc_composer_device_1 *dev,
                              hwc_procs_t const *procs) {
  SprdHWComposer *HWC = static_cast<SprdHWComposer *>(dev);
  if (HWC == NULL) {
    ALOGE("Can NOT get SprdHWComposer reference");
    return;
  }

  HWC->registerProcs(procs);
}

static int hwc_device_open(const struct hw_module_t *module, const char *name,
                           struct hw_device_t **device) {
  int status = -EINVAL;

  if (strcmp(name, HWC_HARDWARE_COMPOSER)) {
    ALOGE("The module name is not HWC_HARDWARE_COMPOSER");
    return status;
  }

  SprdHWComposer *HWC = new SprdHWComposer();
  if (HWC == NULL) {
    ALOGE("Can NOT create SprdHWComposer object");
    status = -ENOMEM;
    return status;
  }

  bool ret = HWC->Init();
  if (!ret) {
    ALOGE("Init HWComposer failed");
    delete HWC;
    HWC = NULL;
    status = -ENOMEM;
    return status;
  }

  HWC->hwc_composer_device_1_t::common.tag = HARDWARE_DEVICE_TAG;
  HWC->hwc_composer_device_1_t::common.version = HWC_DEVICE_API_VERSION_1_4;
  HWC->hwc_composer_device_1_t::common.module =
      const_cast<hw_module_t *>(module);
  HWC->hwc_composer_device_1_t::common.close = hwc_device_close;

  HWC->hwc_composer_device_1_t::prepare = hwc_prepareDisplays;
  HWC->hwc_composer_device_1_t::set = hwc_setDisplays;
  HWC->hwc_composer_device_1_t::eventControl = hwc_eventControl;
  HWC->hwc_composer_device_1_t::blank = hwc_blank;
  HWC->hwc_composer_device_1_t::query = hwc_query;
  HWC->hwc_composer_device_1_t::registerProcs = hwc_registerProcs;
  HWC->hwc_composer_device_1_t::dump = hwc_dump;
  HWC->hwc_composer_device_1_t::getDisplayConfigs = hwc_getDisplayConfigs;
  HWC->hwc_composer_device_1_t::getDisplayAttributes = hwc_getDisplayAttributes;
  HWC->hwc_composer_device_1_t::getActiveConfig = hwc_getActiveConfig;
  HWC->hwc_composer_device_1_t::setActiveConfig = hwc_setActiveConfig;
  HWC->hwc_composer_device_1_t::setPowerMode = hwc_setPowerMode;
#ifdef __LP64__
  HWC->hwc_composer_device_1_t::setCursorPositionAsync =
      hwc_setCursorPositionAsync;
#endif
  /*  HWC->hwc_composer_device_1_t::getBuiltInDisplayNum =
    hwc_getBuiltInDisplayNum;*/

  *device = &HWC->hwc_composer_device_1_t::common;

  status = 0;

  return status;
}

static struct hw_module_methods_t hwc_module_methods = {open : hwc_device_open};

hwc_module_t HAL_MODULE_INFO_SYM = {
  common : {
    tag : HARDWARE_MODULE_TAG,
    version_major : 3,
    version_minor : 0,
    id : HWC_HARDWARE_MODULE_ID,
    name : "SPRD HWComposer Module",
    author : "The AndroidL Open Source Project",
    methods : &hwc_module_methods,
    dso : 0,
    reserved : {0},
  }
};
