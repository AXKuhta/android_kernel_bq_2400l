
#ifndef FMHALSOURCE_H
#define FMHALSOURCE_H


#include <stdint.h>
#include <sys/types.h>
#include <limits.h>

#include <utils/Atomic.h>
#include <utils/Errors.h>
#include <utils/threads.h>

#include <utils/SortedVector.h>
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>

#include <tinyalsa/asoundlib.h>


#include "RingBuffer.h"
#include "FMHalSource.h"

namespace android {

class FMTrack: public RefBase
{
public:

    enum{
        IDLE,
        ACTIVE,
        TERMINATED,
    };

    FMTrack(uint32_t bufferCount,uint32_t BufferUnitSize);
    ~FMTrack();
    uint8_t *GetBuf(uint32_t  wait);
    void PutData(uint32_t wake);
    uint8_t *GetData(uint32_t wait);
    uint32_t GetDataCount();
    void PutBuf(uint32_t wake);
    uint32_t BufCount();
    uint32_t BufUnitSize();
    int	    SetState(int state);
    int     GetState();

protected:
    friend class  FMHalThread;
    class RingBuffer *ring;
    int                 mState;
    int	    overflow;
};


class FMHalThread : public Thread
{
public:

    FMHalThread(uint32_t  readUnitSize,int card, int device,struct pcm_config * cfg);
    ~FMHalThread();
    int32_t    start(sp<FMTrack> &track);
    int32_t     stop(sp<FMTrack> &track);
    void    pause(bool paused);
    bool    is_activited(void);
    virtual status_t    readyToRun() { return NO_ERROR; }
    virtual void        onFirstRef();
    virtual bool        threadLoop();
    void      exit();

private:

    struct pcm * pcmHandle;
    Vector<sp<FMTrack>  >  mTrack;
    mutable     Mutex                               mLock;
    mutable     Mutex                               mpcmLock;
    uint8_t  * readBuf;
    Condition               mWaitCond;
    uint32_t readSize;
    uint8_t exit_var;
    struct pcm_config pcm_cfg;
    int mCard;
    int mDevice;
    bool recordbypass;
    bool isactivited;
};


class FMHalSource:public RefBase
{

public:
    FMHalSource(int samplerate,int channel);
    ~FMHalSource();

    enum{
        IDLE,
        START,
        STOP,
    };
    int32_t start(void);
    int32_t stop(void);
    void    pause(bool paused);
    bool    is_activited(void);
    int32_t read(void *  buf, uint32_t  bytes, bool flag = false);

private:
    sp<FMTrack>    mTrack;
    uint8_t  *ply_buf;
    uint32_t ply_len;
    int state;
    int is_ready;
    Mutex            mLock_l;

    static  Mutex            mLock;
    static struct pcm *pcmHandle;
    static uint32_t  openCount;
    static sp<FMHalThread>  FMThread;

    static int mChannel;
    static int mSamplerate;
    static int mPeriodSize;
    static int mPeriodCount;
    static int mUnitBytes;

};


extern "C"{
    typedef void *  FMPcmHandler;
    FMPcmHandler fm_pcm_open(int samplerate,int channels,int bytes,int required_channel);
    int fm_pcm_read(FMPcmHandler pcm,void* buf,int bytes,int waitflag,int channel);
    void fm_pcm_close(FMPcmHandler pcm);
    void fm_pcm_pause(FMPcmHandler pcm,bool paused);
    bool fm_pcm_isactivited(FMPcmHandler pcm);
}

}
#endif
