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
 ** 16/08/2013    Hardware Composer   Add a new feature to Harware composer,  *
 **                                   verlayComposer use GPU to do the        *
 **                                   Hardware layer blending on Overlay      *
 **                                   buffer, and then post the OVerlay       *
 **                                   buffer to Display                       *
 ******************************************************************************
 ** Author:         fushou.yang@spreadtrum.com                                *
 **                 zhongjun.chen@spreadtrum.com                              *
 *****************************************************************************/



#include <hardware/hwcomposer.h>
#include <hardware/hardware.h>
#include <sys/ioctl.h>
//#include "sprd_fb.h"
#include "SyncThread.h"

#include "OverlayNativeWindow.h"
#include "dump.h"
#include "../AndroidFence.h"


namespace android {

static int mDebugFlag = 0;

OverlayNativeWindow::OverlayNativeWindow(SprdDisplayPlane *displayPlane)
    : mDisplayPlane(displayPlane),
      mWidth(1), mHeight(1), mFormat(-1),
      mWindowUsage(-1),
      mNumBuffers(displayPlane->getPlaneCount()),
      mNumFreeBuffers(displayPlane->getPlaneCount()), mBufferHead(0),
      mInvalidateCount(displayPlane->getPlaneCount()),
      mCurrentBufferIndex(0),
      mUpdateOnDemand(false),
      mDirtyTargetFlag(false)
{

}

bool OverlayNativeWindow::Init()
{
    mDisplayPlane->getPlaneGeometry(&mWidth, &mHeight, &mFormat);
    mWindowUsage = mDisplayPlane->getPlaneUsage();

    ANativeWindow::setSwapInterval = setSwapInterval;
    ANativeWindow::cancelBuffer    = cancelBuffer;
    ANativeWindow::dequeueBuffer   = dequeueBuffer;
    ANativeWindow::queueBuffer     = queueBuffer;
    ANativeWindow::query           = query;
    ANativeWindow::perform         = perform;

    const_cast<int&>(ANativeWindow::minSwapInterval) = 0;
    const_cast<int&>(ANativeWindow::maxSwapInterval) = 1;

    return true;
}

OverlayNativeWindow::~OverlayNativeWindow()
{
}

int OverlayNativeWindow:: releaseNativeBuffer()
{
    for (int i = 0; i < mNumBuffers; i++)
    {
        if (buffers[i] != NULL)
        {
            buffers[i] = 0;
        }
    }

    return 0;
}

void OverlayNativeWindow:: notifyDirtyTarget(bool flag)
{
    HWC_IGNORE(flag);
    OverlayNativeWindow* self = getSelf(this);
    if (!(self->mDirtyTargetFlag))
    {
        Mutex::Autolock _l(self->mutex);
        self->mDirtyTargetFlag = true;
        self->mInvalidateCount = mNumBuffers - 1;
    }
}

sp<NativeBuffer> OverlayNativeWindow::CreateGraphicBuffer(native_handle_t* buffer)
{
    sp<NativeBuffer> nativeBuffer = NULL;

    nativeBuffer = new NativeBuffer(ADP_WIDTH(buffer),
                                    ADP_HEIGHT(buffer),
                                    ADP_FORMAT(buffer),
                                    GRALLOC_USAGE_HW_FB);
    nativeBuffer->handle = (native_handle_t*)buffer;
    nativeBuffer->stride = ADP_WIDTH(buffer);

    return nativeBuffer;
}


int OverlayNativeWindow::dequeueBuffer(ANativeWindow* window,
        ANativeWindowBuffer** buffer, int* fenceFd)
{
    OverlayNativeWindow* self = getSelf(window);
    Mutex::Autolock _l(self->mutex);
    int index = -1;
    int fd = -1;

    // wait for a free buffer
    while (!self->mNumFreeBuffers) {
        self->mCondition.wait(self->mutex);
    }

    native_handle_t* Buffer = self->mDisplayPlane->dequeueBuffer(&fd);
    if (buffer == NULL)
    {
        ALOGE("Failed to get the Display plane buffer");
        return -1;
    }

    index = self->mDisplayPlane->getPlaneBufferIndex();
    if (index < 0 || index >2)
    {
        ALOGE("OverlayNativeWindow get invalid buffer index");
        return -1;
    }
    // get this buffer
    self->mNumFreeBuffers--;
    self->mCurrentBufferIndex = index;

    if (self->buffers[index] == NULL)
    {
        self->buffers[index] = CreateGraphicBuffer(Buffer);
        if (self->buffers[index] == NULL)
        {
            ALOGE("Failed to CreateGraphicBuffer");
            return -1;
        }
    }
    *buffer = self->buffers[index].get();
#ifdef INVALIDATE_WINDOW_TARGET
    if (self->mDirtyTargetFlag)
    {
        ADP_BUFINDEX(Buffer) = 0x100;
    }
#endif

    if(fd>-1)
    {
        *fenceFd = dup(fd);
        ALOGI_IF(mDebugFlag,"<01-2> OverlayBuffer GPU dequeue, get rel_fd:%d = dup(%d)",*fenceFd,fd);
    }
    else
    {
        *fenceFd = -1;
    }

    queryDebugFlag(&mDebugFlag);
    return 0;
}


int OverlayNativeWindow::queueBuffer(ANativeWindow* window,
        ANativeWindowBuffer* buffer, int fenceFd)
{
    native_handle_t *hnd = (native_handle_t *)buffer->handle;
    OverlayNativeWindow* self = getSelf(window);
    Mutex::Autolock _l(self->mutex);
    ALOGI_IF(mDebugFlag,"<02-2> OverlayComposerScheldule() return, dst acqFd:%d",fenceFd);
    self->mDisplayPlane->queueBuffer(fenceFd);

    postSem();

    const int index = self->mCurrentBufferIndex;
    self->front = static_cast<NativeBuffer*>(buffer);
    self->mNumFreeBuffers++;

    if ((self->mInvalidateCount)-- <= 0)
    {
        self->mDirtyTargetFlag = false;
    }
#ifdef INVALIDATE_WINDOW_TARGET
    ADP_BUFINDEX(hnd) = 0;
#endif
    self->mCondition.broadcast();

    queryDebugFlag(&mDebugFlag);

    return 0;
}

#if 0
int OverlayNativeWindow::lockBuffer(ANativeWindow* window,
        ANativeWindowBuffer* buffer)
{
    OverlayNativeWindow* self = getSelf(window);
    Mutex::Autolock _l(self->mutex);

    const int index = self->mCurrentBufferIndex;

    // wait that the buffer we're locking is not front anymore
    while (self->front == buffer) {
        self->mCondition.wait(self->mutex);
    }

    return 0;
}
#endif

int OverlayNativeWindow::cancelBuffer(ANativeWindow* window, ANativeWindowBuffer* buffer, int fenceFd)
{
    const OverlayNativeWindow* self = getSelf(window);
    Mutex::Autolock _l(self->mutex);

    if (fenceFd >= 0)
    {
        sp<Fence> fence(new Fence(fenceFd));
        int waitResult = fence->wait(Fence::TIMEOUT_NEVER);
        if (waitResult != OK)
        {
            ALOGE("OverlayNativeWindow::cancelBuffer Fence::wait returned an error: %d,buffer:%p", waitResult,buffer);
        }

        close(fenceFd);
        fenceFd = -1;
    }

    self->mDisplayPlane->resetPlaneBufferState(self->mCurrentBufferIndex);

    return 0;
}

int OverlayNativeWindow::query(const ANativeWindow* window,
        int what, int* value)
{
    const OverlayNativeWindow* self = getSelf(window);
    Mutex::Autolock _l(self->mutex);


    //ALOGI("%s %d",__func__,__LINE__);

    switch (what) {
        case NATIVE_WINDOW_FORMAT:
            *value = self->mFormat;
            return NO_ERROR;
        case NATIVE_WINDOW_QUEUES_TO_WINDOW_COMPOSER:
            *value = 0;
            return NO_ERROR;
        case NATIVE_WINDOW_CONCRETE_TYPE:
            *value = NATIVE_WINDOW_FRAMEBUFFER;
            return NO_ERROR;
        case NATIVE_WINDOW_DEFAULT_WIDTH:
            *value = self->mWidth;
            return NO_ERROR;
        case NATIVE_WINDOW_DEFAULT_HEIGHT:
            *value = self->mHeight;
            return NO_ERROR;
        case NATIVE_WINDOW_TRANSFORM_HINT:
            *value = 0;
            return NO_ERROR;
        case NATIVE_WINDOW_CONSUMER_RUNNING_BEHIND:
            *value = 0;
            return NO_ERROR;
        case NATIVE_WINDOW_CONSUMER_USAGE_BITS:
            *value = GRALLOC_USAGE_HW_FB | GRALLOC_USAGE_HW_RENDER |
                     GRALLOC_USAGE_HW_COMPOSER | self->mWindowUsage;
            return NO_ERROR;
        case NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS:
            *value = 1;
            return NO_ERROR;
        case NATIVE_WINDOW_DEFAULT_DATASPACE:
             *value = 0;
            return NO_ERROR;
	case NATIVE_WINDOW_BUFFER_AGE:
	    *value = 0;
	    return NO_ERROR;

    }
    return BAD_VALUE;

}

int OverlayNativeWindow::perform(ANativeWindow* window,
        int operation, ...)
{
    HWC_IGNORE(window);
    switch (operation) {
        case NATIVE_WINDOW_CONNECT:
        case NATIVE_WINDOW_DISCONNECT:
        case NATIVE_WINDOW_SET_USAGE:
        case NATIVE_WINDOW_SET_BUFFERS_GEOMETRY:
        case NATIVE_WINDOW_SET_BUFFERS_DIMENSIONS:
        case NATIVE_WINDOW_SET_BUFFERS_FORMAT:
        case NATIVE_WINDOW_SET_BUFFERS_TRANSFORM:
        case NATIVE_WINDOW_API_CONNECT:
        case NATIVE_WINDOW_API_DISCONNECT:
        case NATIVE_WINDOW_SET_BUFFERS_DATASPACE:
            // TODO: we should implement these
            return NO_ERROR;

        case NATIVE_WINDOW_LOCK:
        case NATIVE_WINDOW_UNLOCK_AND_POST:
        case NATIVE_WINDOW_SET_CROP:
        case NATIVE_WINDOW_SET_BUFFER_COUNT:
        case NATIVE_WINDOW_SET_BUFFERS_TIMESTAMP:
        case NATIVE_WINDOW_SET_SCALING_MODE:
            return INVALID_OPERATION;
    }
    return NAME_NOT_FOUND;
}


int OverlayNativeWindow::setSwapInterval(
        ANativeWindow* window, int interval)
{
    HWC_IGNORE(window);
    HWC_IGNORE(interval);

    //hwc_composer_device_t* fb = getSelf(window)->hwc_dev;
  //  return fb->setSwapInterval(fb, interval);
    return 0;
}

};
