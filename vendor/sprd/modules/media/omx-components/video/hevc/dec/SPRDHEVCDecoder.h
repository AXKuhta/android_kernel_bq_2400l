/*
 * Copyright (C) 2011 The Android Open Source Project
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

#ifndef SPRD_HEVC_DECODER_H_
#define SPRD_HEVC_DECODER_H_

#include "SprdSimpleOMXComponent.h"
#include <utils/KeyedVector.h>
#include "MemIon.h"
#include "hevc_dec_api.h"

#define SPRD_ION_DEV "/dev/ion"

#define H265_DECODER_INTERNAL_BUFFER_SIZE (0x100000)
#define H265_DECODER_STREAM_BUFFER_SIZE (1024*1024*3)

struct tagHEVCHandle;

namespace android {

struct SPRDHEVCDecoder : public SprdSimpleOMXComponent {
    SPRDHEVCDecoder(const char *name,
                    const OMX_CALLBACKTYPE *callbacks,
                    OMX_PTR appData,
                    OMX_COMPONENTTYPE **component);

    OMX_ERRORTYPE initCheck() const;

protected:
    virtual ~SPRDHEVCDecoder();

    virtual OMX_ERRORTYPE internalGetParameter(
        OMX_INDEXTYPE index, OMX_PTR params);

    virtual OMX_ERRORTYPE internalSetParameter(
        OMX_INDEXTYPE index, const OMX_PTR params);

    virtual OMX_ERRORTYPE internalUseBuffer(
        OMX_BUFFERHEADERTYPE **buffer,
        OMX_U32 portIndex,
        OMX_PTR appPrivate,
        OMX_U32 size,
        OMX_U8 *ptr,
        BufferPrivateStruct* bufferPrivate=NULL);

    virtual OMX_ERRORTYPE allocateBuffer(
        OMX_BUFFERHEADERTYPE **header,
        OMX_U32 portIndex,
        OMX_PTR appPrivate,
        OMX_U32 size);

    virtual OMX_ERRORTYPE freeBuffer(
        OMX_U32 portIndex,
        OMX_BUFFERHEADERTYPE *header);

    virtual OMX_ERRORTYPE getConfig(OMX_INDEXTYPE index, OMX_PTR params);
    virtual OMX_ERRORTYPE setConfig(
        OMX_INDEXTYPE index, const OMX_PTR params);

    virtual void onQueueFilled(OMX_U32 portIndex);
    virtual void onPortFlushCompleted(OMX_U32 portIndex);
    virtual void onPortEnableCompleted(OMX_U32 portIndex, bool enabled);
    virtual void onPortFlushPrepare(OMX_U32 portIndex);
    virtual OMX_ERRORTYPE getExtensionIndex(const char *name, OMX_INDEXTYPE *index);
    virtual void onReset();

private:
    enum {
        kNumInputBuffers  = 5,
        kNumOutputBuffers = 7,
    };

    enum EOSStatus {
        INPUT_DATA_AVAILABLE,
        INPUT_EOS_SEEN,
        OUTPUT_FRAMES_FLUSHED,
    };

    enum OutputPortSettingChange {
        NONE,
        AWAITING_DISABLED,
        AWAITING_ENABLED
    };

    tagHEVCHandle *mHandle;

    size_t mInputBufferCount;
    int32_t mPicId;
    int mSetFreqCount;
    uint32_t mMinCompressionRatio;

    uint32_t mFrameWidth, mFrameHeight;
    uint32_t mStride, mSliceHeight;
    uint32_t mPictureSize;
    uint32_t mCropWidth, mCropHeight;

    Mutex mLock;
    Condition mCondition;
    OMX_BOOL mGettingPortFormat;

    EOSStatus mEOSStatus;
    OutputPortSettingChange mOutputPortSettingsChange;

    bool mHeadersDecoded;
    bool mSignalledError;
    bool mDecoderSwFlag;
    bool mChangeToSwDec;
    bool mAllocateBuffers;
    bool mNeedIVOP;
    bool mIOMMUEnabled;
    int mIOMMUID;
    bool mStopDecode;
    OMX_BOOL mThumbnailMode;

    uint8_t *mCodecInterBuffer;
    uint8_t *mCodecExtraBuffer;

    sp<MemIon> mPmem_stream;
    uint8_t *mPbuf_stream_v;
    unsigned long mPbuf_stream_p;
    size_t mPbuf_stream_size;

    sp<MemIon> mPmem_extra;
    uint8_t*  mPbuf_extra_v;
    unsigned long  mPbuf_extra_p;
    size_t  mPbuf_extra_size;

    sp<MemIon> mPmem_mbinfo[17];
    uint8_t *mPbuf_mbinfo_v[17];
    unsigned long  mPbuf_mbinfo_p[17];
    size_t  mPbuf_mbinfo_size[17];
    int mPbuf_mbinfo_idx;

    void* mLibHandle;
    FT_H265DecInit mH265DecInit;
    FT_H265DecGetInfo mH265DecGetInfo;
    FT_H265DecDecode mH265DecDecode;
    FT_H265DecRelease mH265DecRelease;
    FT_H265Dec_SetCurRecPic  mH265Dec_SetCurRecPic;
    FT_H265Dec_GetLastDspFrm  mH265Dec_GetLastDspFrm;
    FT_H265Dec_ReleaseRefBuffers  mH265Dec_ReleaseRefBuffers;
    FT_H265DecMemInit mH265DecMemInit;
    FT_H265GetCodecCapability mH265GetCodecCapability;
    FT_H265DecGetNALType mH265DecGetNALType;
    FT_H265DecSetparam mH265DecSetparam;
    FT_H265Dec_get_iova mH265DecGetIOVA;
    FT_H265Dec_free_iova mH265DecFreeIOVA;
    FT_H265Dec_get_IOMMU_status mH265DecGetIOMMUStatus;

    OMX_BOOL iUseAndroidNativeBuffer[2];
    MMDecCapability mCapability;

    OMX_ERRORTYPE mInitCheck;

    void initPorts();
    status_t initDecoder();
    void releaseDecoder();
    void updatePortDefinitions(bool updateCrop = true, bool updateInputSize = false);
    bool drainAllOutputBuffers();
    void drainOneOutputBuffer(int32_t picId, void* pBufferHeader);
    bool handleCropRectEvent(const CropParams* crop);
    bool handlePortSettingChangeEvent(const H265SwDecInfo *info);

    static int32_t CtuInfoMemAllocWrapper(void* aUserData, unsigned int size_mbinfo, unsigned long *pPhyAddr);
    static int32_t ExtMemAllocWrapper(void* aUserData, unsigned int size_extra) ;
    static int32_t BindFrameWrapper(void *aUserData, void *pHeader, unsigned long *pPhyAddr);
    static int32_t UnbindFrameWrapper(void *aUserData, void *pHeader, unsigned long *pPhyAddr);

    int VSP_malloc_ctuinfo_cb(unsigned int size_mbinfo, unsigned long *pPhyAddr);
    int VSP_malloc_cb(unsigned int size_extra);
    int VSP_bind_cb(void *pHeader, unsigned long *pPhyAddr);
    int VSP_unbind_cb(void *pHeader, unsigned long *pPhyAddr);
    bool openDecoder(const char* libName);

    bool outputBuffersNotEnough(const H265SwDecInfo *, OMX_U32 , OMX_U32 , OMX_BOOL);
    OMX_ERRORTYPE allocateStreamBuffer(OMX_U32 bufferSize);
    void releaseStreamBuffer();

    DISALLOW_EVIL_CONSTRUCTORS(SPRDHEVCDecoder);
};

}  // namespace android

#endif  // SPRD_HEVC_DECODER_H_

