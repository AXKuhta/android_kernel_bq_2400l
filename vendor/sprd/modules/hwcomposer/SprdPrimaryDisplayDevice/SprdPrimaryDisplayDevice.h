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
 ** File: SprdPrimaryDisplayDevice.h  DESCRIPTION                             *
 **                                   Manage the PrimaryDisplayDevice         *
 **                                   including prepare and commit            *
 ******************************************************************************
 ******************************************************************************
 ** Author:         zhongjun.chen@spreadtrum.com                              *
 *****************************************************************************/

#ifndef _SPRD_PRIMARY_DISPLAY_DEVICE_H_
#define _SPRD_PRIMARY_DISPLAY_DEVICE_H_

#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>
#include <fcntl.h>
#include <errno.h>

#include <EGL/egl.h>

#include <utils/RefBase.h>
#include <cutils/properties.h>
#include <cutils/atomic.h>
#include <cutils/log.h>

#include "SprdHWLayerList.h"
#include "SprdOverlayPlane.h"
#include "SprdPrimaryPlane.h"
#include "SprdFrameBufferHAL.h"
#include "../SprdDisplayDevice.h"
#include "../AndroidFence.h"
#include "../SprdDisplayCore.h"
#ifdef UPDATE_SYSTEM_FPS_FOR_POWER_SAVE
#include "../FileOp.h"
#endif

#ifdef OVERLAY_COMPOSER_GPU
#include "../OverlayComposer/OverlayComposer.h"
#endif

#include "../SprdUtil.h"
#include "../dump.h"

using namespace android;

class SprdHWLayerList;
class SprdUtil;

class SprdPrimaryDisplayDevice {
 public:
  SprdPrimaryDisplayDevice();

  ~SprdPrimaryDisplayDevice();

  /*
   *  Initialize the SprdPrimaryDisplayDevice member
   * */
  bool Init(SprdDisplayCore *core);

  /*
   *  Traversal layer list, and find layers which comply with SprdDisplayPlane
   *  and mark them as HWC_OVERLAY.
   * */
  int prepare(hwc_display_contents_1_t *list, unsigned int accelerator);

  /*
   *  Post layers to SprdDisplayPlane.
   * */
  int commit(hwc_display_contents_1_t *list);

  /*
   *  Build Sync data for SurfaceFligner
   * */
  int buildSyncData(hwc_display_contents_1_t *list, DisplayTrack *tracker);

  /*
   *  Primary Display Device event control interface.
   *  Make vsync event enable or disable.
   * */
  void eventControl(int enabled);

  /*
   *  Display configure attribution.
   * */
  int syncAttributes(AttributesSet *dpyAttributes);

  int ActiveConfig(DisplayAttributes *dpyAttributes);

  int setPowerMode(int mode);

  /*
   *  Asynchronously update the location of the cursor layer.
   * */
  int setCursorPositionAsync(int x_pos, int y_pos);

  /*
   *  Recycle DispalyPlane buffer for saving memory.
   * */
  int reclaimPlaneBuffer(bool condition);

  /*
   *  pass the number of builtin display vendor support to SurfaceFlinger.
   * */
  int getBuiltInDisplayNum(uint32_t *number);

 private:
  FrameBufferInfo *mFBInfo;
  SprdDisplayCore *mDispCore;
  void *mPrimaryDisplayContext;
  SprdHWLayerList *mLayerList;
  SprdOverlayPlane *mOverlayPlane;
  SprdPrimaryPlane *mPrimaryPlane;
#ifdef OVERLAY_COMPOSER_GPU
  sp<OverlayNativeWindow> mWindow;
  sp<OverlayComposer> mOverlayComposer;
#endif
  SprdHWLayer *mComposedLayer;
  SprdUtil *mUtil;
  SprdUtilSource *mUtilSource;
  SprdUtilTarget *mUtilTarget;
  bool mDisplayFBTarget;
  bool mDisplayPrimaryPlane;
  bool mDisplayOverlayPlane;
  bool mDisplayOVC;
  #ifdef ACC_BY_DISPC
  bool mDisplayDispC;
  #endif
  bool mSchedualUtil;
  bool mFirstFrameFlag;
#ifdef UPDATE_SYSTEM_FPS_FOR_POWER_SAVE
  bool mIsPrimary60Fps;
#endif
  int mHWCDisplayFlag;
  unsigned int mAcceleratorMode;

  bool mBlank;
  Mutex mLock;

  int mDebugFlag;
  int mDumpFlag;

  typedef struct sprdRect DisplayRect;

  inline SprdHWLayerList *getHWLayerList() { return mLayerList; }

  /*
   *  And then attach these HWC_OVERLAY layers to SprdDisplayPlane.
   * */
  int attachToDisplayPlane(int DisplayFlag);

  int WrapFBTargetLayer(hwc_display_contents_1_t *list);

  int WrapOverlayLayer(native_handle_t* buf, int format, int fenceFd,hwc_display_contents_1_t *list);

  int WrapOverlayLayer(native_handle_t* buf, int format, int fenceFd);
  /*
   *  Compute the DirtyRegion for PrimaryDisplay.
   *  DirtyRegion: the region will be read by
   *  Display Controller.
   *  the region will be smaller than Screen size,
   *  it is usefull for Partial update.
   * */
  int computeDisplayDirtyRegion(DisplayRect &DirtyRegion);

  int AcceleratorProbe();

  int AcceleratorAdapt(int DisplayDeviceAccelerator);

#ifdef HWC_DUMP_CAMERA_SHAKE_TEST
  void dumpCameraShakeTest(hwc_display_contents_1_t *list);
#endif

  int SprdUtilScheldule(SprdUtilSource *Source, SprdUtilTarget *Target);

#ifdef OVERLAY_COMPOSER_GPU
  int OverlayComposerScheldule(hwc_display_contents_1_t *list,
                               SprdDisplayPlane *DisplayPlane);
#endif

#ifdef UPDATE_SYSTEM_FPS_FOR_POWER_SAVE
void UpdateSystemFps(hwc_display_contents_1_t *list);
#endif
};

#endif  // #ifndef _SPRD_PRIMARY_DISPLAY_DEVICE_H_
