#include <sys/select.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#define LOG_TAG "ext_contrl"
#include "Tone_generate.h"
#include "HW_ul_Test.h"

static int out_devices_test_config(struct audiotest_config *config,struct str_parms *parms);

#ifdef AUDIO_TEST_INTERFACE_V2
#include "endpoint_test.h"
#endif

#ifdef SUPPORT_OLD_AUDIO_TEST_INTERFACE
extern int audPlayerOpen( int outdev, int sampleRate, int stereo, int sync );
extern int audPlayerPlay( const unsigned char * data, int size );
extern int audPlayerStop( void );
extern int audPlayerClose( void );

extern int audRcrderOpen( int indev, int sampleRate );
extern int audRcrderRecord( unsigned char * data, int size );
extern int audRcrderStop( void );
extern int audRcrderClose( void );
extern int sprd_audiorecord_test_start(int devices);
extern int sprd_audiorecord_test_read(int size);
extern int sprd_audiorecord_test_stop(void);
extern int sprd_audiotrack_test_start(int channel,int devices, char *buf, int size);
extern int sprd_audiotrack_test_stop(void);
extern int sprd_fm_test_open(void);
extern int sprd_fm_test_play(void);
extern int sprd_fm_test_stop(void);
#endif
extern int audio_skd_test(void * dev,struct str_parms *parms,int type);
extern void set_dump_switch(int value);

#define DefaultBufferLength 30000*100
#define FILE_PATH_MUSIC  "/data/local/media/audio_dumpmusic.pcm"
#define FILE_PATH_SCO  "/data/local/media/audio_dumpsco.pcm"
#define FILE_PATH_BTSCO  "/data/local/media/audio_dumpbtsco.pcm"
#define FILE_PATH_VAUDIO  "/data/local/media/audio_dumpvaudio.pcm"
#define FILE_PATH_INREAD  "/data/local/media/audio_dumpinread.pcm"
#define FILE_PATH_INREAD_AFTER_PROCESS  "/data/local/media/audio_dumpinread_noprocess.pcm"
#define FILE_PATH_INREAD_AFTER_RESAMPLER  "/data/local/media/audio_dumpinread_aftersrc.pcm"
#define FILE_PATH_INREAD_AFTER_NR  "/data/local/media/audio_dumpinread_afternr.pcm"

#define FILE_PATH_INREAD_VBC  "/data/local/media/audio_dumpinread_vbc.pcm"
#define FILE_PATH_MUSIC_WAV  "/data/local/media/audio_dumpmusic.wav"
#define FILE_PATH_SCO_WAV  "/data/local/media/audio_dumpsco.wav"
#define FILE_PATH_BTSCO_WAV  "/data/local/media/audio_dumpbtsco.wav"
#define FILE_PATH_VAUDIO_WAV  "/data/local/media/audio_dumpvaudio.wav"
#define FILE_PATH_INREAD_WAV  "/data/local/media/audio_dumpinread.wav"
#define FILE_PATH_INREAD_AFTER_PROCESS_WAV  "/data/local/media/audio_dumpinread_noprocess.wav"
#define FILE_PATH_INREAD_AFTER_SRC_WAV  "/data/local/media/audio_dumpinread_aftersrc.wav"
#define FILE_PATH_INREAD_AFTER_NR_WAV  "/data/local/media/audio_dumpinread_afternr.wav"
#define FILE_PATH_INREAD_VBC_WAV  "/data/local/media/audio_dumpinread_vbc.wav"

#define FILE_PATH_HAL_INFO  "data/local/media/audio_hw_info.txt"
#define FILE_PATH_HELP  "data/local/media/help.txt"
#define FILE_PATH_CP_RAM_PRE  "data/local/media/voicepoint"
#define MAX_FILE_LENGTH 255
#define FILE_PATH_CP_ENABLE_POINT_INFO  "data/local/media/cppoint.txt"

#define CP_DUMP_DATA "dumpvoice"
#define CP_DUMP_POINTSIZE "getfilledsize"
#define CP_DUMP_DURATION "setduration"
#define CP_DUMP_POINT_GETDUARTION "getduration"//"whichpoint"
#define CP_DUMP_POINT_ENABLE "setpointon"
#define CP_DUMP_POINT_DISABLE "setpointoff"
#define CP_DUMP_POINT_DISPLAY "getpoint"//"whichpoint"
#define CP_MAX_POINT 30
#define CP_MAX_DURATION 60
#define NB_SAMPLE_RATE 8000
#define VOICE_FRAME_SIZE 2

pthread_t control_audio_loop;

#define AUTOTEST_DEVICES_OUT_TEST   "autotest_audiotracktest"
#define AUTOTEST_DEVICES_IN_TEST    "autotest_audiorecordtest"
#define AUTOTEST_FM_TEST    "autotest_fmtest"

#define TESE_AUDIO_IN_DEVICES_TEST  "input_devices_test"
#define TESE_AUDIO_OUT_DEVICES_TEST  "out_devices_test"
#define TESE_AUDIO_OUT_DEVICES_LOOP_TEST  "audio_loop_test"
#define TEST_HUAWEILAOHUA_AUDIO_OUT_DEVICES_LOOP_TEST  "huaweilaohua_audio_loop_test"
#define TESE_AUDIO_DEVICES_STOP_CMD  "testAudioDevicesStop"
#define TEST_AUDIO_SAMPLE_RATE "sample_rate:"
#define TEST_AUDIO_CHANNEL "channel:"
#define TEST_AUDIO_FILE "file:"
#define TEST_AUDIO_INPUT_DEVICES "input_devices:"
#define TEST_AUDIO_OUTPUT_DEVICES "output_devices:"
#define TESE_AUDIO_CP_LOOP_TEST "audio_cp_loop_test"
#define SPLIT ","
#define GET_DUMP_DATA_SWITCH "get_dump_data_switch"
#define SET_DUMP_DATA_SWITCH "set_dump_data_switch"
#define SET_DUMP_DATA_PATH "set_dump_data_path"
#define SET_AUDIO_DUMP "set_media.audio.dump"

#define TEST_AUDIO_INVALID_DATA_MS 500
#define TEST_AUDIO_PLAYTHREAD_WAKEUP_DATA_COUNT (TEST_AUDIO_INVALID_DATA_COUNT+1)
#define TEST_LOOPBACK_BUFFER_DATA_MS  400
#define MMI_DEFAULT_PCM_FILE  "/data/local/media/aploopback.pcm"
#define MMI_LOOP_PCM_FILE  "/data/local/media/loop.pcm"
#ifdef FLAG_VENDOR_ETC
#define AUDIO_TEST_CP_LOOP_PLAY_PCM_FILE_ROOT "vendor/etc/rx_data.pcm"
#else
#define AUDIO_TEST_CP_LOOP_PLAY_PCM_FILE_ROOT "/etc/rx_data.pcm"
#endif
#define AUDIO_TEST_CP_LOOP_PLAY_PCM_FILE "/data/local/media/rx_data.pcm"
#define AUDIO_TEST_CP_LOOP_RECD_PCM_FILE "/data/local/media/tx_data.pcm"

#define AUDIO_LOOP_TEST_RESULT_PROPERITY  "audioloop.test.result"
#define TONE_VOLUME 50000
#define TONE_LEN    16000

#define AUDIO_EXT_CONTROL_PIPE "/dev/pipe/mmi.audio.ctrl"
#define AUDIO_EXT_DATA_CONTROL_PIPE "/data/local/media/mmi.audio.ctrl"

static const struct pcm_config default_config = {
    .channels = 2,
    .rate = DEFAULT_OUT_SAMPLING_RATE,
    .period_size = SHORT_PERIOD_SIZE,
    .period_count = CAPTURE_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

static const struct pcm_config default_record_config = {
    .channels = 1,
    .rate = 16000,
    .period_size = 640,
    .period_count = 8,
    .format = PCM_FORMAT_S16_LE,
};

static const struct pcm_config cp_loop_record_config = {
    .channels = 1,
    .rate = 16000,
    .period_size = 640,
    .period_count = 8,
    .format = PCM_FORMAT_S16_LE,
};

static const struct pcm_config cp_loop_playback_config = {
    .channels = 1,
    .rate = 16000,
    .period_size = 320,
    .period_count = 4,
    .format = PCM_FORMAT_S16_LE,
};

struct pcm_config playback_config = {
    .channels = 2,
    .rate = DEFAULT_OUT_SAMPLING_RATE,
    .period_size = SHORT_PERIOD_SIZE,
    .period_count = PLAYBACK_SHORT_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

struct pcm_config record_config = {
    .channels = 1,
    .rate = 16000,
    .period_size = 320,
    .period_count = 8,
    .format = PCM_FORMAT_S16_LE,
};

static const unsigned char s_pcmdata_mono[] = {
    0x92,0x02,0xcb,0x0b,0xd0,0x14,0x1d,0x1d,0xfc,0x24,0x17,0x2c,0x4a,0x32,0x69,0x37,
    0x92,0x3b,0x4e,0x3e,0x22,0x40,0x56,0x40,0x92,0x3f,0x12,0x3d,0x88,0x39,0x10,0x35,
    0xf0,0x2e,0x51,0x28,0xce,0x20,0x7f,0x18,0xd5,0x0f,0xda,0x06,0xdf,0xfd,0xa4,0xf4,
    0xa2,0xeb,0x39,0xe3,0x57,0xdb,0x3d,0xd4,0x1f,0xce,0xe2,0xc8,0xb1,0xc4,0xc0,0xc1,
    0xec,0xbf,0xc1,0xbf,0xa4,0xc0,0xf2,0xc2,0x18,0xc6,0xc2,0xca,0xc8,0xd0,0x36,0xd7,
    0xbb,0xde,0xe6,0xe6,0xa5,0xef,0xa6,0xf8,
};
#ifdef SUPPORT_OLD_AUDIO_TEST_INTERFACE
int sprd_audiorecord_test(struct str_parms *parms,int opt){
    int channel=0;
    int devices=0;
    char value[50];
    int ret = -1;
    int val_int = 0;
    int data_size=0;
    ALOGD("sprd_audiorecord_test opt:%d",opt);

    switch(opt){
        case 1:{
            ret = str_parms_get_str(parms,"autotest_indevices", value, sizeof(value));
            if(ret >= 0){
                ALOGD("%s %d",__func__,__LINE__);
                devices = strtoul(value,NULL,0);
                ALOGD("%s %d %d",__func__,__LINE__,devices);
            }
            ret=sprd_audiorecord_test_start(devices);
        }
            break;
        case 2:
            ret = str_parms_get_str(parms,"autotest_datasize", value, sizeof(value));
            if(ret >= 0){
                ALOGD("%s %d",__func__,__LINE__);
                data_size = strtoul(value,NULL,0);
                ALOGD("%s %d %d",__func__,__LINE__,devices);
            }
            ret=sprd_audiorecord_test_read(data_size);
            break;
        default:
            ret=sprd_audiorecord_test_stop();
            break;
    }
    return ret;
}
#endif
int string_to_hex(unsigned char * dst,const unsigned char *str, int max_size){
    int size=0;
    unsigned char *tmp=str;
    unsigned char data=0;
    unsigned char char_tmp=0;
    while(1){
        char_tmp=(unsigned char)*tmp;
        if((char_tmp==NULL) || (char_tmp=='\0')){
            break;
        }
        if((char_tmp>='0') && (char_tmp<='9')){
            data= (char_tmp-'0')<<4;
        }else if((char_tmp>='a') && (char_tmp<='f')){
            data= ((char_tmp-'a')+10)<<4;
        }else if((char_tmp>='A') && (char_tmp<='F')){
            data= ((char_tmp-'A')+10)<<4;
        }else{
            break;
        }

        char_tmp=(unsigned char)*(tmp+1);
        if((char_tmp==NULL) || (char_tmp=='\0')){
            break;
        }

        if((char_tmp>='0') && (char_tmp<='9')){
            data |= (char_tmp-'0');
        }else if((char_tmp>='a') && (char_tmp<='f')){
            data |= ((char_tmp-'a')+10);
        }else if((char_tmp>='A') && (char_tmp<='F')){
            data |= (char_tmp-'A')+10;
        }else{
            break;
        }

        tmp+=2;
        dst[size++]=data;

        if(size>=max_size){
            break;
        }

        data=0;
    }
    return size;
}
#ifdef SUPPORT_OLD_AUDIO_TEST_INTERFACE
int sprd_audiotrack_test(struct str_parms *parms,bool is_start){
    int channel=0;
    int devices=0;
    char value[1024];
    int ret = -1;
    int val_int = 0;
    int data_size=0;
    char *data=NULL;
    ALOGD("%s %d",__func__,__LINE__);
    if(true==is_start){
        ret = str_parms_get_str(parms,"autotest_outdevices", value, sizeof(value));
        if(ret >= 0){
            ALOGD("%s %d",__func__,__LINE__);
            devices = strtoul(value,NULL,0);
            ALOGD("%s %d %d",__func__,__LINE__,devices);
        }

        ret = str_parms_get_str(parms,"autotest_channels", value, sizeof(value));
        if(ret >= 0){
            channel = strtoul(value,NULL,0);
            ALOGD("%s %d %d",__func__,__LINE__,channel);
        }

        ret = str_parms_get_str(parms,"autotest_datasize", value, sizeof(value));
        if(ret >= 0){
            data_size = strtoul(value,NULL,0);
            ALOGD("%s %d %d",__func__,__LINE__,data_size);

            if(data_size>0){
                data=(char *)malloc(data_size);
                if(NULL == data){
                    ALOGE("sprd_audiotrack_test malloc failed");
                    data_size=0;
                }
            }
        }

        ret = str_parms_get_str(parms,"autotest_data", value, sizeof(value));
        if(ret >= 0){
            if(NULL==data){
                ALOGE("autotest_data NULL ERR");
            }

            int size =string_to_hex(data,value,data_size);
            if(data_size!=size){
                ALOGE("autotest_data ERR:%x %x",size,data_size);
            }
        }

        ret=sprd_audiotrack_test_start(channel,devices,data,data_size);
        if(NULL!=data){
            free(data);
        }
    }else{
        ret=sprd_audiotrack_test_stop();
    }
    return ret;
}
#endif
void *out_test_thread(void *args);

void *in_test_thread(void *args){
    struct in_test_t *in_test=(struct in_test_t *)args;
    struct dev_test_t *dev_test = in_test->dev_test;
    struct pcm *pcm;
    char *buffer,*proc_buf;
    unsigned int size,proc_buf_size;
    unsigned int bytes_read = 0;
    int card = -1;
    int num_read = 0;
    int read_count = 0;
    int process_init = 0;
    int is_notify_to_play = 0;
    int invalid_data_size=0;
    ALOGD("%s %d",__func__,__LINE__);

    if(in_test->isloop == true){
        in_test->config.fd=open(MMI_LOOP_PCM_FILE, O_WRONLY|O_CREAT,0660);
        if(in_test->config.fd<0){
            ALOGE("in_test_thread create:%s failed",MMI_LOOP_PCM_FILE);
            return NULL;
         }
    }

    card=get_snd_card_number(CARD_SPRDPHONE);
    if(card < 0) {
    ALOGD("%s %d",__func__,__LINE__);

        return NULL;
    }

    pcm=NULL;
    buffer=NULL;
    proc_buf=NULL;
    proc_buf_size=0;
    bytes_read=0;

    ALOGD("capture_thread wait");
    sem_wait(&in_test->sem);
    ALOGD("capture_thread running");

    pcm = pcm_open(card, 0, PCM_IN, &in_test->config.config);
    if (!pcm || !pcm_is_ready(pcm)) {
        ALOGW("Unable to open PCM device (%s)\n",pcm_get_error(pcm));
        goto ERR;
    }

    size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));
    buffer = malloc(size);
    if (!buffer) {
        ALOGW("Unable to allocate %d bytes\n", size);
        goto ERR;
    }

    invalid_data_size=(TEST_AUDIO_INVALID_DATA_MS *in_test->config.config.rate
                *in_test->config.config.channels*2/1000);

    process_init = init_rec_process(GetAudio_InMode_number_from_device(in_test->dev_test->adev->in_devices),
        in_test->config.config.rate);
    proc_buf_size = size;
    proc_buf = malloc(proc_buf_size);
    if (!proc_buf) {
        ALOGW("Unable to allocate %d bytes\n", size);
        goto ERR;
    }

    memset(buffer,0,size);
    memset(proc_buf,0,proc_buf_size);
    ALOGD("begin capture!!!!!!!!!!!!!!!!");

    do{
        num_read=pcm_read(pcm, buffer, size);
        ALOGD("capture read:0x%x req:0x%x",num_read,size);
        read_count++;

        if (!num_read){
            bytes_read += size;
            aud_rec_do_process(buffer, size,in_test->config.config.channels,proc_buf,proc_buf_size);

            if(bytes_read >invalid_data_size){

                if(in_test->config.fd > 0){
                    if (write(in_test->config.fd,buffer,size) != size){
                        ALOGW("audiotest_capture_thread write err\n");
                    }
                }
            }

            if((in_test->isloop == true) && (is_notify_to_play == 0)
                && (bytes_read > ((TEST_LOOPBACK_BUFFER_DATA_MS *in_test->config.config.rate
                *in_test->config.config.channels*2/1000)+invalid_data_size))) {
                is_notify_to_play = 1;
                sem_post(&dev_test->out_test.sem);
            }
            ALOGD("capture %d bytes bytes_read:0x%x",size,bytes_read);
        }else{
            ALOGW("aploop_capture_thread: no data read num_read:%d",num_read);
            usleep(20000);
        }
    }while(in_test->state);

ERR:

    if(buffer){
        free(buffer);
        buffer=NULL;
    }

    if(proc_buf){
        free(proc_buf);
        proc_buf=NULL;
    }

    if(pcm){
        pcm_close(pcm);
        pcm=NULL;
    }
    if(in_test->config.fd >0){
        close(in_test->config.fd);
        in_test->config.fd=-1;
    }
    if(process_init)
        AUDPROC_DeInitDp();

    ALOGE("audiotest_capture_thread exit");
    return NULL;
}

static int in_devices_test_config(struct audiotest_config *config,struct str_parms *parms)
{
    char value[50];
    int ret = -1;
    int val_int = 0;
    int fd = -1;

    ALOGD("%s %d",__func__,__LINE__);
    memcpy(&config->config, &default_record_config, sizeof(struct pcm_config));
    ret = str_parms_get_str(parms,"samplerate", value, sizeof(value));
    if(ret >= 0){
        ALOGD("%s %d",__func__,__LINE__);
        val_int = atoi(value);
    }
    config->config.rate = val_int;

    ret = str_parms_get_str(parms,"channels", value, sizeof(value));
    if(ret >= 0){
        val_int = atoi(value);
        ALOGD("%s %d %d",__func__,__LINE__,val_int);
    }
    if((val_int !=1) && (val_int != 2)) {
        ALOGD("%s %d %d",__func__,__LINE__,val_int);
        goto error;
    }
    config->config.channels = val_int;
    ret = str_parms_get_str(parms,"filepath", value, sizeof(value));
    if(ret >= 0){
        ALOGD("%s %d:%s",__func__,__LINE__,value);
        fd = open(value, O_WRONLY);
        if(fd >= 0) {
            config->fd = fd;
        }
        else
            config->fd = -1;
    }
    else {
        ALOGD("%s %d",__func__,__LINE__);
        fd = open(MMI_DEFAULT_PCM_FILE, O_WRONLY);
        if(fd >= 0) {
            config->fd = fd;
        }
        else{
            config->fd = -1;
        }
    }
    ALOGD("%s %d",__func__,__LINE__);
    return config->fd;
error:
    return -1;
}


static int loop_devices_test_config(struct audiotest_config *config,struct str_parms *parms)
{
    char value[50];
    int ret = -1;
    int val_int = 0;
    int fd = -1;

    ALOGD("%s %d",__func__,__LINE__);
    memcpy(&config->config, &default_record_config, sizeof(struct pcm_config));
    ret = str_parms_get_str(parms,"samplerate", value, sizeof(value));
    if(ret >= 0){
        ALOGD("%s %d",__func__,__LINE__);
        val_int = atoi(value);
    }
    config->config.rate = val_int;
    ret = str_parms_get_str(parms,"channels", value, sizeof(value));
    if(ret >= 0){
        val_int = atoi(value);
        ALOGD("%s %d %d",__func__,__LINE__,val_int);
    }
    if((val_int !=1) && (val_int != 2)) {
        ALOGD("%s %d %d",__func__,__LINE__,val_int);
        goto error;
    }
    config->config.channels = val_int;
    config->fd = -1;
    ALOGD("%s %d",__func__,__LINE__);
    return 0;
error:
    return -1;
}

void *huaweilaohua_in_test_thread(void *args){
    struct in_test_t *in_test=(struct in_test_t *)args;
    struct dev_test_t *dev_test = in_test->dev_test;
    struct pcm *pcm;
    char *buffer,*proc_buf;
    unsigned int size,proc_buf_size;
    unsigned int bytes_read = 0;
    int card = -1;
    int num_read = 0;
    int i = 0 ;
    int read_count = 0;
    int process_init = 0;
    int invalid_data_size=0;
    int16 pass_ratio_thd = 29490;
    int16 Energy_ratio_thd = 2;
    int is_notify_to_playback_exit = 0;
    int count = 0;
    pcm = NULL ;
    buffer=NULL;
    proc_buf=NULL;
    proc_buf_size=0;
    bytes_read=0;

    card=get_snd_card_number(CARD_SPRDPHONE);
    if(card < 0) {
        return NULL;
    }

    AUDIO_LOOP_TEST_INFO_STRUCT_T *audio_test_verify_info;
    audio_test_verify_info = malloc(sizeof(AUDIO_LOOP_TEST_INFO_STRUCT_T));
    if(!audio_test_verify_info) {
       ALOGE("audio_test_verify_info creating failed !!!!");
       goto ERR;
    }
    memset(audio_test_verify_info , 0 , sizeof(AUDIO_LOOP_TEST_INFO_STRUCT_T));


    ALOGD("capture_thread wait");
    sem_wait(&in_test->sem);
    ALOGD("capture_thread running");

    pcm = pcm_open(card, 0, PCM_IN, &in_test->config.config);
    if (!pcm || !pcm_is_ready(pcm)) {
        ALOGW("Unable to open PCM device (%s)\n",pcm_get_error(pcm));
        goto ERR;
    }

    size = sizeof(audio_test_verify_info->input);
    buffer = malloc(size);
    if (!buffer) {
        ALOGW("Unable to allocate %d bytes\n", size);
        goto ERR;
    }

    invalid_data_size=(TEST_AUDIO_INVALID_DATA_MS *in_test->config.config.rate
                *in_test->config.config.channels*2/1000);

    process_init = init_rec_process(GetAudio_InMode_number_from_device(in_test->dev_test->adev->in_devices),
        in_test->config.config.rate);
    proc_buf_size = size;
    proc_buf = malloc(proc_buf_size);
    if (!proc_buf) {
        ALOGW("Unable to allocate %d bytes\n", size);
        goto ERR;
    }

    audio_test_verify_info->frequency = dev_test->out_test.frequency ;
    ALOGD("neo: frequency %d" ,audio_test_verify_info->frequency );

    memset(buffer,0,size);
    memset(proc_buf,0,proc_buf_size);
    ALOGD("begin capture!!!!!!!!!!!!!!!!");

    // 1. init
    audio_hw_test_handle  audio_test_verify_handle;
    audio_test_verify_handle = audio_HW_ul_Test_init(Energy_ratio_thd ,pass_ratio_thd );

   // 2. read and process
    do{
        num_read=pcm_read(pcm, buffer, size);
        ALOGD("capture read:0x%x req:0x%x",num_read,size);
        read_count++;

        if (!num_read){
            bytes_read += size;
            aud_rec_do_process(buffer, size,in_test->config.config.channels,proc_buf,proc_buf_size);

            if(bytes_read >(invalid_data_size + 10240) ){
                     memcpy(audio_test_verify_info->input, buffer , sizeof(audio_test_verify_info->input));
                     audio_HW_ul_Test(audio_test_verify_info, audio_test_verify_handle);
                     ALOGE("neo***:audio_loop_devices_test %d %d  count = %d", audio_test_verify_info->hwStestResult,audio_test_verify_info->isReady , count++);
            }
            ALOGD("capture %d bytes bytes_read:0x%x",size,bytes_read);
        }else{
            ALOGW("aploop_capture_thread: no data read num_read:%d",num_read);
            usleep(20000);
        }
    }while( !audio_test_verify_info->isReady  &&  in_test->state);

    // 3. deinit
    audio_HW_ul_Test_deinit(audio_test_verify_handle);

    // 4. set test result
    if(audio_test_verify_info->hwStestResult)
        property_set(AUDIO_LOOP_TEST_RESULT_PROPERITY,"1");
    else
        property_set(AUDIO_LOOP_TEST_RESULT_PROPERITY,"0");

ERR:

    // 5. let the playback thread to exit
    if(is_notify_to_playback_exit == 0) {
        is_notify_to_playback_exit = 1 ;
        dev_test->out_test.state = 0;
    }

    if(audio_test_verify_info){
        free(audio_test_verify_info);
        audio_test_verify_info = NULL;
    }
    if(buffer){
        free(buffer);
        buffer=NULL;
    }

    if(proc_buf){
        free(proc_buf);
        proc_buf=NULL;
    }

    if(pcm){
        pcm_close(pcm);
        pcm=NULL;
    }
    if(process_init)
        AUDPROC_DeInitDp();

    in_test->state=0;
    sem_destroy(&in_test->sem);
    ALOGE("audiotest_capture_thread exit");
    return NULL;
}

void *huaweilaohua_out_test_thread(void *args)
{
    struct out_test_t *out_test=(struct out_test_t *)args;
    struct dev_test_t *dev_test = out_test->dev_test;

    struct pcm *pcm = NULL;
    unsigned int size;
    int playback_fd;
    int num_write;
    int ret = 0;
    int i = 3 ;
    int is_notify_to_capture = 0;
    short len ;
    int card=get_snd_card_number(CARD_SPRDPHONE);
    if(card < 0) {
        return NULL;
    }

    AUDIO_TONE_GENERATE_STRUCT_T   *tone_test_struct;
    int16 *sout ;
    int16 test_frequency;
    const uint16 Tone_frequency_table[10] = {875,1000,1125,1250,1375,1500,1625,1750,1875,2000};


    // 1. produce different frequency tone
    sout    =  (int16 *)malloc(TONE_LEN*sizeof(int16));
    if(sout == NULL){
        ALOGE("neo: sout malloc error " );
        return NULL;
     }

    tone_test_struct = (AUDIO_TONE_GENERATE_STRUCT_T*)malloc(sizeof(AUDIO_TONE_GENERATE_STRUCT_T));
    if(tone_test_struct == NULL){
        ALOGE("neo: tone_test_struct malloc error " );
        goto ERR;
    }
    srand(time(0));   //  produce random seed

    tone_test_struct->freq       = Tone_frequency_table[rand()%10]; // frequency
    tone_test_struct->volume     = TONE_VOLUME;                     // volume
    tone_test_struct->len        = TONE_LEN;                        // len
    tone_test_struct->output_ptr = sout;
    out_test->frequency          = tone_test_struct->freq;
    ALOGE("neo: test_frequency %d" ,  tone_test_struct->freq);
    audio_tone_generator(tone_test_struct);



    pcm = pcm_open(card, 0, PCM_OUT, &out_test->config.config);
    if (!pcm || !pcm_is_ready(pcm)) {
        ALOGW("Unable to open PCM device %u (%s)\n",
              0, pcm_get_error(pcm));
        goto ERR;
    }

    ALOGD("playback_thread wait");
    sem_wait(&out_test->sem);
    ALOGD("playback_thread running");

    // 2. playback tone
    do {
            num_write = TONE_LEN;
            ALOGD("neo: num_write %d",num_write );

            ret = pcm_write(pcm, sout,num_write);
            if((out_test->isloop == true) && (is_notify_to_capture == 0) && !(i--)) {
                is_notify_to_capture= 1;
                sem_post(&out_test->dev_test->in_test.sem);
            }
       if (ret) {
            int ret = 0;
            ALOGW("Error playing 1 sample:%s\n",pcm_get_error(pcm));
            ret = pcm_close(pcm);
            pcm = pcm_open(card, 0, PCM_OUT, &out_test->config.config);
            if (!pcm || !pcm_is_ready(pcm)) {
                ALOGW("Unable to open PCM device %u (%s)\n",
                      0, pcm_get_error(pcm));
                goto ERR;
            }
        }
        ALOGE("peter: write ok");
    }while(out_test->state);


ERR:


    if(pcm) {
        pcm_close(pcm);
        pcm = NULL;
    }

    if(sout){
        free(sout);
    }
    if(tone_test_struct){
        free(tone_test_struct);
    }
    sem_destroy(&out_test->sem);
    dev_test->test_mode=TEST_AUDIO_IDLE; // exit test

    ALOGE("audiotest_playback_thread exit");
    return NULL;
}
int prepare_capture_thread(struct in_test_t * in_test , struct str_parms *parms) {
    int ret=0;
    pthread_attr_t capture_attr;

    in_test->isloop=true;
    ret = loop_devices_test_config((struct audiotest_config *)&(in_test->config),parms);
    if(ret < 0 ) {
        ALOGD("%s %d",__func__,__LINE__);
    }

    in_test->state=true;
    sem_init(&in_test->sem, 0, 0);

    // set thread attr DETACHED to auto free thread resource
    pthread_attr_init(&capture_attr);
    pthread_attr_setdetachstate(&capture_attr, PTHREAD_CREATE_DETACHED);
    if(pthread_create(&in_test->thread, &capture_attr, huaweilaohua_in_test_thread, (void *)in_test)) {
        ALOGE("audiotest_capture_thread creating failed !!!!");
        ret = -1;
    }
    return ret;
}

int prepare_playback_thread(struct out_test_t * out_test ,struct str_parms *parms ) {
    int ret;
    pthread_attr_t playback_attr;

    out_test->isloop=true;
    ret = out_devices_test_config((struct audiotest_config *)&(out_test->config),parms);
    if(ret < 0 ) {
        ALOGE("out_devices_test_config  failed !!!!");;
    }
    out_test->state=true;
    sem_init(&out_test->sem, 0, 1);

    // set thread attr DETACHED to auto free thread resource
    pthread_attr_init(&playback_attr);
    pthread_attr_setdetachstate(&playback_attr, PTHREAD_CREATE_DETACHED);
    if(pthread_create(&out_test->thread, &playback_attr, huaweilaohua_out_test_thread, (void *)out_test)) {
        ALOGE("audio_out_devices_test creating failed !!!!");
        ret = -1;
    }
    return ret ;
}
static int huaweilaohua_audio_loop_devices_test(struct tiny_audio_device *adev,struct str_parms *parms){
    int ret=0;

    struct dev_test_t *dev_test = (struct dev_test_t *) &adev->ext_contrl->dev_test;
    struct in_test_t *in_test = (struct in_test_t *) &dev_test->in_test;
    struct out_test_t *out_test = (struct out_test_t *) &dev_test->out_test;
    in_test->dev_test=dev_test;
    out_test->dev_test=dev_test;
    dev_test->adev=adev;

    if(TEST_AUDIO_IDLE != dev_test->test_mode){
        ALOGE("audio_loop_devices_test mode:%d",dev_test->test_mode);
        return -1;
    }else{
        dev_test->test_mode=TEST_AUDIO_OUT_IN_LOOP;
    }


    // 1. create playback thread
    ret = prepare_playback_thread(out_test,parms);
    if(ret < 0 ) {
        goto error;
    }

    // 2. create capture thread  and verify the result
    ret = prepare_capture_thread(in_test,parms);
    if(ret < 0 ) {
        goto error;
    }


    return 0;
error:
    return -1;
}

static int audio_loop_devices_test(struct tiny_audio_device *adev,struct str_parms *parms){
    int ret=0;

    struct dev_test_t *dev_test = (struct dev_test_t *) &adev->ext_contrl->dev_test;
    struct in_test_t *in_test = (struct in_test_t *) &dev_test->in_test;
    struct out_test_t *out_test = (struct out_test_t *) &dev_test->out_test;
    in_test->dev_test=dev_test;
    dev_test->adev=adev;

    pthread_mutex_lock(&adev->lock);
    if(TEST_AUDIO_IDLE != dev_test->test_mode){
        ALOGE("audio_loop_devices_test mode:%d",dev_test->test_mode);
        goto error;
    }else{
        dev_test->test_mode=TEST_AUDIO_IN_OUT_LOOP;
    }

    force_all_standby(adev);

    in_test->isloop=true;
    ret = loop_devices_test_config((struct audiotest_config *)&(in_test->config),parms);
    if(ret < 0 ) {
        ALOGD("%s %d",__func__,__LINE__);
        goto error;
    }

    in_test->state=true;
    sem_init(&in_test->sem, 0, 1);
    if(pthread_create(&in_test->thread, NULL, in_test_thread, (void *)in_test)) {
        ALOGE("audiotest_capture_thread creating failed !!!!");
        goto error;
    }

    out_test->isloop=true;
    ret = loop_devices_test_config((struct audiotest_config *)&(out_test->config),parms);
    if(ret < 0 ) {
        goto error;
    }
    out_test->state=true;
    sem_init(&out_test->sem, 0, 0);
    if(pthread_create(&out_test->thread, NULL, out_test_thread, (void *)out_test)) {
        ALOGE("audio_out_devices_test creating failed !!!!");
        goto error;
    }

    pthread_mutex_unlock(&adev->lock);
    return 0;
error:
    dev_test->test_mode=TEST_AUDIO_IDLE;
    pthread_mutex_unlock(&adev->lock);
    return -1;
}

static int audio_loop_devices_test_stop(struct tiny_audio_device *adev,struct str_parms *parms){
    struct dev_test_t *dev_test = (struct dev_test_t *) &adev->ext_contrl->dev_test;
    struct in_test_t *in_test = (struct in_test_t *) &dev_test->in_test;
    struct out_test_t *out_test = (struct out_test_t *) &dev_test->out_test;
    int ret = -1;
    if(TEST_AUDIO_IN_OUT_LOOP != dev_test->test_mode){
        return -1;
    }
    in_test->state=0;
    out_test->state=0;
    ret = pthread_join(in_test->thread, NULL);
    ret = pthread_join(out_test->thread, NULL);
    ALOGE("audio_loop_devices_test_stop thread destory ret is %d", ret);
    sem_destroy(&in_test->sem);
    sem_destroy(&out_test->sem);
    pthread_mutex_lock(&adev->lock);
    dev_test->test_mode=TEST_AUDIO_IDLE;
    pthread_mutex_unlock(&adev->lock);
    return 0;
}

static int audio_in_devices_test(struct tiny_audio_device *adev,struct str_parms *parms){
    struct dev_test_t *dev_test = (struct dev_test_t *) &adev->ext_contrl->dev_test;
    struct in_test_t *in_test = (struct in_test_t *) &dev_test->in_test;
    int ret = -1;

    if(TEST_AUDIO_IDLE != dev_test->test_mode){
        ALOGE("audio_in_devices_test mode:%d",dev_test->test_mode);
        return -1;
    }else{
        dev_test->test_mode=TEST_AUDIO_IN_DEVICES;
    }

    in_test->dev_test=dev_test;
    dev_test->adev=adev;
    in_test->isloop=false;
    ret = in_devices_test_config((struct audiotest_config *)&(in_test->config),parms);
    if(ret < 0 ) {
        ALOGD("%s %d",__func__,__LINE__);
        goto error;
    }
    in_test->state=true;
    sem_init(&in_test->sem, 0, 1);
    if(pthread_create(&in_test->thread, NULL, in_test_thread, (void *)in_test)) {
        ALOGE("audiotest_capture_thread creating failed !!!!");
        goto error;
    }
    return 0;
error:
    dev_test->test_mode=TEST_AUDIO_IDLE;
    return -1;
}

static int audio_in_devices_test_stop(struct tiny_audio_device *adev,struct str_parms *parms){
    struct dev_test_t *dev_test = (struct dev_test_t *) &adev->ext_contrl->dev_test;
    struct in_test_t *in_test = (struct in_test_t *) &dev_test->in_test;
    int ret = -1;
    if(TEST_AUDIO_IN_DEVICES != dev_test->test_mode){
        return -1;
    }
    in_test->state=0;
    ret = pthread_join(in_test->thread, NULL);
    ALOGE("audio_in_devices_test_stop thread destory ret is %d", ret);
    sem_destroy(&in_test->sem);

    dev_test->test_mode=TEST_AUDIO_IDLE;
    return 0;
}


static int out_devices_test_config(struct audiotest_config *config,struct str_parms *parms)
{
    char value[50];
    int ret = -1;
    int val_int = 0;
    int fd = -1;

    ALOGD("%s %d",__func__,__LINE__);
    memcpy(&config->config, &playback_config, sizeof(struct pcm_config));
    ret = str_parms_get_str(parms,"samplerate", value, sizeof(value));
    if(ret >= 0){
        ALOGD("%s %d",__func__,__LINE__);
        val_int = atoi(value);
    }
    config->config.rate = val_int;

    ret = str_parms_get_str(parms,"channels", value, sizeof(value));
    if(ret >= 0){
        val_int = atoi(value);
        ALOGD("%s %d %d",__func__,__LINE__,val_int);
    }
    if((val_int !=1) && (val_int != 2)) {
        ALOGD("%s %d %d",__func__,__LINE__,val_int);
        goto error;
    }
    config->config.channels = val_int;
    ret = str_parms_get_str(parms,"filepath", value, sizeof(value));
    if(ret >= 0){
        ALOGD("%s %d:%s",__func__,__LINE__,value);
        fd = open(value, O_RDONLY);
        if(fd >= 0) {
            config->fd = fd;
        }
        else{
            config->fd = -1;
            ALOGE("open:%s failed",value);
        }
    }else{
        config->fd = -1;
    }
    return 0;
error:
    return -1;
}

void *out_test_thread(void *args)
{
    struct out_test_t *out_test=(struct in_test_t *)args;
    struct pcm *pcm;
    char *buffer;
    unsigned int size;
    int playback_fd;
    int num_read;
    int ret = 0;
    int try=5;
    struct dev_test_t *dev_test=(struct dev_test_t *)out_test->dev_test;
    struct tiny_audio_device *adev=dev_test->adev;
    int interrupt_playback_support=adev->cp->interrupt_playback_support;

    int card=get_snd_card_number(CARD_SPRDPHONE);
    if(card < 0) {
        return NULL;
    }

    pcm = NULL;
    buffer = NULL;
    num_read = 0;
    size = 0;
    if(out_test->isloop == true){
        while(try--){
            out_test->config.fd=open(MMI_LOOP_PCM_FILE, O_RDONLY);
            if(out_test->config.fd>0)
                break;
            usleep(200);
        }

        if(out_test->config.fd<0){
            ALOGE("out_test_thread open:%s failed",MMI_LOOP_PCM_FILE);
            goto ERR;
         }
    }

    ALOGD("playback_thread wait");
    sem_wait(&out_test->sem);
    ALOGD("playback_thread running interrupt_playback_support:%d",interrupt_playback_support);

   if(interrupt_playback_support){
        pcm = pcm_open(card, 0, PCM_OUT, &out_test->config.config);
   }else{
        pcm = pcm_open(card, 0, PCM_OUT | PCM_MMAP | PCM_NOIRQ | PCM_MONOTONIC, &out_test->config.config);
   }
    if (!pcm || !pcm_is_ready(pcm)) {
        ALOGW("Unable to open PCM device %u (%s)\n",
              0, pcm_get_error(pcm));
        goto ERR;
    }
    size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));
    buffer = malloc(size);
    if (!buffer) {
        ALOGW("Unable to allocate %d bytes\n", size);
        goto ERR;
    }

    /*add this for bug645006 -there is no sound in mmi test*/
    vb_effect_sync_devices(adev->out_devices, adev->in_devices);
    int mode = GetAudio_mode_number_from_device(adev, adev->out_devices, adev->in_devices);
    vb_effect_profile_apply(mode);

    do {
        if(out_test->config.fd > 0){
            num_read = read(out_test->config.fd, buffer, size);
            if(num_read<=0){
                lseek(out_test->config.fd,0,SEEK_SET);
                num_read = read(out_test->config.fd, buffer, size);
            }

            if(interrupt_playback_support){
                 ret = pcm_write(pcm, buffer,num_read);
            }else{
                 ret = pcm_mmap_write(pcm, buffer,num_read);
            }
        }
        else {
            num_read = sizeof(s_pcmdata_mono);
           if(interrupt_playback_support){
                ret = pcm_write(pcm, s_pcmdata_mono,num_read);
           }else{
                ret = pcm_mmap_write(pcm, s_pcmdata_mono,num_read);
           }
        }
        if (ret) {
            int ret = 0;
            ALOGW("Error playing 1 sample:%s\n",pcm_get_error(pcm));
            ret = pcm_close(pcm);

            if(interrupt_playback_support){
                 pcm = pcm_open(card, 0, PCM_OUT, &out_test->config.config);
            }else{
                 pcm = pcm_open(card, 0, PCM_OUT | PCM_MMAP | PCM_NOIRQ | PCM_MONOTONIC, &out_test->config.config);
            }
            if (!pcm || !pcm_is_ready(pcm)) {
                ALOGW("Unable to open PCM device %u (%s)\n",
                      0, pcm_get_error(pcm));
                goto ERR;
            }
        }
        ALOGE("peter: write ok");
    }while(out_test->state);


ERR:

    if(buffer) {
        free(buffer);
        buffer = NULL;
    }

    if(pcm) {
        pcm_close(pcm);
        pcm = NULL;
    }

    if(out_test->config.fd > 0) {
        close(out_test->config.fd);
        out_test->config.fd = -1;
    }
    usleep(150*1000);
    ALOGE("audiotest_playback_thread exit");
    return NULL;
}

static int audio_out_devices_test(struct tiny_audio_device *adev,struct str_parms *parms){
    struct dev_test_t *dev_test = (struct dev_test_t *) &adev->ext_contrl->dev_test;
    struct out_test_t *out_test = (struct out_test_t *) &dev_test->out_test;
    int ret = -1;

    if(TEST_AUDIO_IDLE != dev_test->test_mode){
        ALOGE("audio_out_devices_test mode:%d",dev_test->test_mode);
        return -1;
    }else{
        dev_test->test_mode=TEST_AUDIO_OUT_DEVICES;
    }

    dev_test->adev=adev;
    out_test->isloop=false;
    out_test->dev_test=dev_test;
    ret = out_devices_test_config((struct audiotest_config *)&(out_test->config),parms);
    if(ret < 0 ) {
        goto error;
    }
    out_test->state=true;
    sem_init(&out_test->sem, 0, 1);
    if(pthread_create(&out_test->thread, NULL, out_test_thread, (void *)out_test)) {
        ALOGE("audio_out_devices_test creating failed !!!!");
        goto error;
    }
    return 0;
error:
    dev_test->test_mode=TEST_AUDIO_IDLE;
    return -1;
}

static int audio_out_devices_test_stop(struct tiny_audio_device *adev,struct str_parms *parms){
    struct dev_test_t *dev_test = (struct dev_test_t *) &adev->ext_contrl->dev_test;
    struct out_test_t *out_test = (struct out_test_t *) &dev_test->out_test;
    int ret = -1;
    if(TEST_AUDIO_OUT_DEVICES != dev_test->test_mode){
        return -1;
    }

    out_test->state=0;
    ret = pthread_join(out_test->thread, NULL);
    ALOGE("audio_out_devices_test_stop thread destory ret is %d", ret);
    sem_destroy(&out_test->sem);

    dev_test->test_mode=TEST_AUDIO_IDLE;
    return 0;
}

int ext_control_close (struct ext_control_mgr *pEntryMgr)
{
    char str[] = "entry_exit";
    int w_size = 0;

    if (NULL == pEntryMgr) {
        ALOGE("%s argin is NULL!", __FUNCTION__);
        return -1;
    }

    if(access(AUDIO_EXT_CONTROL_PIPE, R_OK) ==0){
        pEntryMgr->fd = open(AUDIO_EXT_CONTROL_PIPE, O_RDWR);
    }else{
        pEntryMgr->fd = open(AUDIO_EXT_DATA_CONTROL_PIPE, O_RDWR);
    }

    if (pEntryMgr->fd < 0) {
        ALOGE("%s fd is invalid!", __FUNCTION__);
        return -2;
    }

    pEntryMgr->isExit = true;
    w_size = write(pEntryMgr->fd, str, sizeof(str));
    if (w_size > 0) {
        pthread_join(pEntryMgr->thread_id, (void **) NULL);
    }
    close(pEntryMgr->fd);
    pEntryMgr->fd = -1;
    ALOGI("ext_control_close exit w_size = %d", w_size);
    return 0;
}

static void *control_audio_loop_process(void *arg);
int ext_control_open(struct tiny_audio_device *adev){
    ALOGI("%s---",__func__);

    if (mkfifo(MMI_DEFAULT_PCM_FILE,S_IFIFO|0666) <0) {
        if (errno != EEXIST) {
            ALOGE("%s create audio fifo error %s\n",__FUNCTION__,strerror(errno));
            return -1;
        }
    }
    if(chmod(MMI_DEFAULT_PCM_FILE, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH) != 0) {
        ALOGE("%s Cannot set RW to \"%s\": %s", __FUNCTION__,AUDFIFO, strerror(errno));
    }

    /*Create mmi.audio.ctrl*/
    if(access(AUDIO_EXT_CONTROL_PIPE, R_OK) != 0){
        if(access(AUDIO_EXT_DATA_CONTROL_PIPE, R_OK) != 0){
            if (mkfifo(AUDIO_EXT_DATA_CONTROL_PIPE,S_IFIFO|0666) <0) {
                if (errno != EEXIST) {
                    LOG_E("%s create audio fifo error %s\n",__FUNCTION__,strerror(errno));
                    return -1;
                }
            }

            if(chmod(AUDIO_EXT_DATA_CONTROL_PIPE, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH) != 0) {
                LOG_E("%s Cannot set RW to \"%s\": %s", __FUNCTION__,AUDIO_EXT_DATA_CONTROL_PIPE, strerror(errno));
            }
        }
    }

    adev->ext_contrl->entryMgr.fd = -1;
    adev->ext_contrl->entryMgr.isExit = false;
    if(pthread_create(&adev->ext_contrl->entryMgr.thread_id, NULL, control_audio_loop_process, (void *)adev)) {
        ALOGE("control_audio_loop thread creating failed !!!!");
        return -1;
    }
    adev->ext_contrl->dev_test.test_mode=TEST_AUDIO_IDLE;
    return 0;
}

static int read_noblock_l(int fd,int8_t *buf,int bytes){
    int ret = 0;
    ret = read(fd,buf,bytes);
    return ret;
}

static void empty_command_pipe(int fd){
    char buff[16];
    int ret;
    do {
        ret = read(fd, &buff, sizeof(buff));
    } while (ret > 0 || (ret < 0 && errno == EINTR));
}

/***********************************************************
 *function: init dump buffer info;
 *
 * *********************************************************/
int init_dump_info(out_dump_t* out_dump,const char* filepath,size_t buffer_length,bool need_cache,bool save_as_wav){
    ALOGE("%s ",__func__);
    if(out_dump == NULL){
        ALOGE("%s, can not init dump info ,out_dump is null",__func__);
        return -1;
    }
    //1,create dump fife
    out_dump->dump_fd = fopen(filepath,"wb");
    if(out_dump->dump_fd == NULL){
        ALOGE("%s, creat dump file err",__func__);
    }
    if(save_as_wav){
        out_dump->wav_fd = open(filepath,O_RDWR);
        if(out_dump->wav_fd <= 0){
           LOG_E("%s creat wav file error",__func__);
        }
    }else{
        out_dump->wav_fd = 0;
    }

    //2,malloc cache buffer
    if(need_cache){
        out_dump->cache_buffer = malloc(buffer_length);
        if(out_dump->cache_buffer == NULL){
            ALOGE("malloc cache buffer err!");
            if(out_dump->dump_fd > 0){
                fclose(out_dump->dump_fd);
                out_dump->dump_fd = NULL;
            }
            if(out_dump->wav_fd > 0){
                close(out_dump->wav_fd);
                out_dump->wav_fd = 0;
            }
            return -1;
        }
        memset(out_dump->cache_buffer,0,buffer_length);
    }else{
        out_dump->cache_buffer = NULL;
    }
    out_dump->buffer_length = buffer_length;
    out_dump->write_flag = 0;
    out_dump->more_one = false;
    out_dump->total_length = 0;
    if(0 == strcmp(filepath,FILE_PATH_INREAD_WAV) || 0 == strcmp(filepath,FILE_PATH_INREAD) ||
            0 == strcmp(filepath,FILE_PATH_INREAD_AFTER_PROCESS_WAV) || 0 == strcmp(filepath,FILE_PATH_INREAD_AFTER_PROCESS)){
        out_dump->sampleRate = 8000;
        out_dump->channels = 1;
    }else if(0 == strcmp(filepath,FILE_PATH_INREAD_AFTER_SRC_WAV) || 0 == strcmp(filepath,FILE_PATH_INREAD_AFTER_RESAMPLER)){
        out_dump->sampleRate = 16000;
        out_dump->channels = 1;
    }else{
        out_dump->sampleRate = 44100;
        out_dump->channels = 2;
    }
    return 0;
}

/***********************************************************
 *function: release dump buffer info;
 *
 * *********************************************************/
int release_dump_info(out_dump_t* out_dump){
    LOG_I("%s ",__func__);
    if(out_dump == NULL){
        ALOGE("out_dump is null");
        return -1;
    }
    //1 relese buffer
    LOG_I("release buffer");
    if(out_dump->cache_buffer){
        free(out_dump->cache_buffer);
        out_dump->cache_buffer = NULL;
    }

    //2 close file fd
    LOG_I("release fd");
    if(out_dump->dump_fd){
        fclose(out_dump->dump_fd);
        out_dump->dump_fd = NULL;
    }
    if(out_dump->wav_fd){
        close(out_dump->wav_fd);
        out_dump->wav_fd = 0;
    }

    out_dump->write_flag = 0;
    out_dump->more_one = false;
    out_dump->total_length = 0;
    return 0;
}

/********************************************
 *function: save cache buffer to file
 *
 * ******************************************/
int save_cache_buffer(out_dump_t* out_dump)
{
    LOG_I("%s ",__func__);
    if (out_dump == NULL || out_dump->cache_buffer == NULL) {
        LOG_E("adev or DumpBuffer is NULL");
        return -1;
    }
    size_t written = 0;
    if(out_dump->dump_fd == NULL){
        LOG_E("dump fd is null ");
        return -1;
    }

   if (out_dump->more_one) {
        size_t size1 = out_dump->buffer_length - out_dump->write_flag;
        size_t size2 = out_dump->write_flag;
        LOG_I("size1:%d,size2:%d,buffer_length:%d",size1,size2,out_dump->buffer_length);
        written = fwrite(((uint8_t *)out_dump->cache_buffer + out_dump->write_flag), size1, 1, out_dump->dump_fd);
        written += fwrite((uint8_t *)out_dump->cache_buffer, size2, 1, out_dump->dump_fd);
        out_dump->total_length = out_dump->buffer_length;
    } else {
        written += fwrite((uint8_t *)out_dump->cache_buffer, out_dump->write_flag, 1, out_dump->dump_fd);
        out_dump->total_length = out_dump->write_flag;
    }
    LOG_E("writen:%ld",out_dump->total_length);
    return written;
}

/******************************************
 *function: write dump data to cache buffer
 *
 ******************************************/
size_t dump_to_buffer(out_dump_t *out_dump, void* buf, size_t size)
{

    LOG_I("%s  ",__func__);
    if (out_dump == NULL || out_dump->cache_buffer == NULL || buf == NULL ) {
         LOG_E("adev or DumpBuffer is NULL or buf is NULL");
        return -1;
    }
    size_t copy = 0;
    size_t bytes = size;
    uint8_t *src = (uint8_t *)buf;
    //size>BufferLength,  size larger then the left space,size smaller then the left space
    if (size > out_dump->buffer_length) {
        int Multi = size/out_dump->buffer_length;
        src= buf + (size - (Multi-1) * out_dump->buffer_length);
        bytes = out_dump->buffer_length;
        out_dump->write_flag = 0;
    }
    if (bytes > (out_dump->buffer_length - out_dump->write_flag)) {
        out_dump ->more_one = true;
        size_t size1 = out_dump->buffer_length - out_dump->write_flag;
        size_t size2 = bytes - size1;
        memcpy(out_dump->cache_buffer + out_dump->write_flag,src,size1);
        memcpy(out_dump->cache_buffer,src+size1,size2);
        out_dump->write_flag = size2;
    } else {
        memcpy(out_dump->cache_buffer + out_dump->write_flag,src,bytes);
        out_dump->write_flag += bytes;
        if (out_dump->write_flag >= out_dump->buffer_length) {
            out_dump->write_flag -= out_dump->buffer_length;
        }
    }
    copy = bytes;
    return copy;
}

/***********************************************
 *function: write dump to file directly
 *
 ***********************************************/
int dump_to_file(FILE *out_fd ,void* buffer, size_t size)
{
    LOG_D("%s ,%p,%p,%d",__func__,out_fd,buffer,size);
    int ret = 0;
    if(out_fd){
        ret = fwrite((uint8_t *)buffer,size, 1, out_fd);
        if(ret < 0){
            LOG_W("%s fwrite filed:%d",__func__,size);
        }
    }else{
        LOG_E("out_fd is NULL, can not write");
    }
    return ret;
}

/********************************************
 * function:add wav header
 *
 * *****************************************/
int add_wav_header(out_dump_t* out_dump){
    char header[44];
    long totalAudioLen = out_dump->total_length;
    long totalDataLen = totalAudioLen + 36;
    long longSampleRate = out_dump->sampleRate;
    int channels = out_dump->channels;
    long byteRate = out_dump->sampleRate * out_dump->channels * 2;
    LOG_E("%s ",__func__);
    header[0] = 'R'; // RIFF/WAVE header
    header[1] = 'I';
    header[2] = 'F';
    header[3] = 'F';
    header[4] = (char) (totalDataLen & 0xff);
    header[5] = (char) ((totalDataLen >> 8) & 0xff);
    header[6] = (char) ((totalDataLen >> 16) & 0xff);
    header[7] = (char) ((totalDataLen >> 24) & 0xff);
    header[8] = 'W';
    header[9] = 'A';
    header[10] = 'V';
    header[11] = 'E';
    header[12] = 'f'; // 'fmt ' chunk
    header[13] = 'm';
    header[14] = 't';
    header[15] = ' ';
    header[16] = 16; // 4 bytes: size of 'fmt ' chunk
    header[17] = 0;
    header[18] = 0;
    header[19] = 0;
    header[20] = 1; // format = 1
    header[21] = 0;
    header[22] = (char) channels;
    header[23] = 0;
    header[24] = (char) (longSampleRate & 0xff);
    header[25] = (char) ((longSampleRate >> 8) & 0xff);
    header[26] = (char) ((longSampleRate >> 16) & 0xff);
    header[27] = (char) ((longSampleRate >> 24) & 0xff);
    header[28] = (char) (byteRate & 0xff);
    header[29] = (char) ((byteRate >> 8) & 0xff);
    header[30] = (char) ((byteRate >> 16) & 0xff);
    header[31] = (char) ((byteRate >> 24) & 0xff);
    header[32] = (char) (channels * 16 / 8); // block align
    header[33] = 0;
    header[34] = 16; // bits per sample
    header[35] = 0;
    header[36] = 'd';
    header[37] = 'a';
    header[38] = 't';
    header[39] = 'a';
    header[40] = (char) (totalAudioLen & 0xff);
    header[41] = (char) ((totalAudioLen >> 8) & 0xff);
    header[42] = (char) ((totalAudioLen >> 16) & 0xff);
    header[43] = (char) ((totalAudioLen >> 24) & 0xff);
    if(out_dump == NULL){
        LOG_E("%s,err:out_dump is null",__func__);
        return -1;
    }
    lseek(out_dump->wav_fd,0,SEEK_SET);
    write(out_dump->wav_fd,header,sizeof(header));

    return 0;
}


static void dump_process(out_dump_t *out_dump, bool use_cache,void* buf, size_t size)
{
    if((NULL!=out_dump)&&(NULL!=buf)&&(size>0)){
        if(use_cache){
            dump_to_buffer(out_dump,buf,size);
        }else{
            dump_to_file(out_dump->dump_fd,buf,size);
        }
    }else{
        ALOGW("dump_process null point:%p %p 0x%x",out_dump,buf,size);
    }
}

/*************************************************
 *function:the interface of dump
 *
 * ***********************************************/
void do_dump(dump_info_t* dump_info, dump_switch_e opt,void* buffer, size_t size){
    out_dump_t *out_dump=NULL;

    if(dump_info == NULL){
        LOG_E("err:out_dump or ext_contrl is null");
        return;
    }
    switch(opt){
        case DUMP_MINIPRIMARY:
            out_dump=dump_info->out_music;
            break;

        case DUMP_MINIVOIP:
            out_dump=dump_info->out_voip;
            break;

        case DUMP_MINIVAUDIO:
            out_dump=dump_info->out_vaudio;
            break;

        case DUMP_MINI_BT_SCO:
            out_dump=dump_info->out_bt_sco;
            break;

        case DUMP_MINI_RECORD_HAL:
            out_dump=dump_info->in_read;
            break;

        case DUMP_MINI_RECORD_AFTER_PROCESS:
            out_dump=dump_info->in_read_afterprocess;
            break;

        case DUMP_MINI_RECORD_VBC:
            out_dump=dump_info->in_read_vbc;
            break;

        case DUMP_MINI_RECORD_AFTER_SRC:
            out_dump=dump_info->in_read_aftersrc;
            break;

        case DUMP_MINI_RECORD_AFTER_NR:
            out_dump=dump_info->in_read_afternr;
            break;

        default:
            break;
    }
    dump_process(out_dump,dump_info->dump_to_cache,buffer,size);
    return;
}

void set_dump_status(void * dev,int switch_val){
    struct str_parms *parms;
    int ret=-1;
    char value[32];
    int val_int=0;
    struct tiny_audio_device *adev = (struct tiny_audio_device *)dev;

    if(switch_val&(1<<DUMP_MINIPRIMARY)){
            if(false==adev->ext_contrl->dump_info->dump_music){
                if(adev->ext_contrl->dump_info->dump_as_wav){
                     init_dump_info(adev->ext_contrl->dump_info->out_music,FILE_PATH_MUSIC_WAV,
                                adev->ext_contrl->dump_info->out_music->buffer_length,
                                adev->ext_contrl->dump_info->dump_to_cache,
                                adev->ext_contrl->dump_info->dump_as_wav);
                }else{
                    init_dump_info(adev->ext_contrl->dump_info->out_music,FILE_PATH_MUSIC,
                                adev->ext_contrl->dump_info->out_music->buffer_length,
                                adev->ext_contrl->dump_info->dump_to_cache,
                                adev->ext_contrl->dump_info->dump_as_wav);
                }
                adev->ext_contrl->dump_info->dump_music = true;
            }
        }else{
            if(true==adev->ext_contrl->dump_info->dump_music){
                adev->ext_contrl->dump_info->dump_music = false;
                if(adev->ext_contrl->dump_info->dump_to_cache){
                    save_cache_buffer(adev->ext_contrl->dump_info->out_music);
                }
                if(adev->ext_contrl->dump_info->dump_as_wav){
                    add_wav_header(adev->ext_contrl->dump_info->out_music);
                }
                release_dump_info(adev->ext_contrl->dump_info->out_music);
            }
        }

    if(switch_val&(1<<DUMP_MINIVOIP)){
            if(false==adev->ext_contrl->dump_info->dump_voip){
                if(adev->ext_contrl->dump_info->dump_as_wav){
                     init_dump_info(adev->ext_contrl->dump_info->out_voip,FILE_PATH_SCO_WAV,
                                adev->ext_contrl->dump_info->out_voip->buffer_length,
                                adev->ext_contrl->dump_info->dump_to_cache,
                                adev->ext_contrl->dump_info->dump_as_wav);
                }else{
                    init_dump_info(adev->ext_contrl->dump_info->out_voip,FILE_PATH_SCO,
                                adev->ext_contrl->dump_info->out_voip->buffer_length,
                                adev->ext_contrl->dump_info->dump_to_cache,
                                adev->ext_contrl->dump_info->dump_as_wav);
                }
                adev->ext_contrl->dump_info->dump_voip = true;
            }
        }else{
            if(true==adev->ext_contrl->dump_info->dump_voip){
                adev->ext_contrl->dump_info->dump_voip = false;
                if(adev->ext_contrl->dump_info->dump_to_cache){
                    save_cache_buffer(adev->ext_contrl->dump_info->out_voip);
                }
                if(adev->ext_contrl->dump_info->dump_as_wav){
                    add_wav_header(adev->ext_contrl->dump_info->out_voip);
                }
                release_dump_info(adev->ext_contrl->dump_info->out_voip);
            }
        }

    if(switch_val&(1<<DUMP_MINI_BT_SCO)){
            if(false==adev->ext_contrl->dump_info->dump_bt_sco){
                if(adev->ext_contrl->dump_info->dump_as_wav){
                     init_dump_info(adev->ext_contrl->dump_info->out_bt_sco,FILE_PATH_BTSCO_WAV,
                                adev->ext_contrl->dump_info->out_bt_sco->buffer_length,
                                adev->ext_contrl->dump_info->dump_to_cache,
                                adev->ext_contrl->dump_info->dump_as_wav);
                }else{
                    init_dump_info(adev->ext_contrl->dump_info->out_bt_sco,FILE_PATH_BTSCO,
                                adev->ext_contrl->dump_info->out_bt_sco->buffer_length,
                                adev->ext_contrl->dump_info->dump_to_cache,
                                adev->ext_contrl->dump_info->dump_as_wav);
                }
                adev->ext_contrl->dump_info->dump_bt_sco = true;
            }
        }else{
            if(true==adev->ext_contrl->dump_info->dump_bt_sco){
                adev->ext_contrl->dump_info->dump_bt_sco = false;
                if(adev->ext_contrl->dump_info->dump_to_cache){
                    save_cache_buffer(adev->ext_contrl->dump_info->out_bt_sco);
                }
                if(adev->ext_contrl->dump_info->dump_as_wav){
                    add_wav_header(adev->ext_contrl->dump_info->out_bt_sco);
                }
                release_dump_info(adev->ext_contrl->dump_info->out_bt_sco);
            }
        }

    if(switch_val&(1<<DUMP_MINIVAUDIO)){
            if(false==adev->ext_contrl->dump_info->dump_vaudio){
                if(adev->ext_contrl->dump_info->dump_as_wav){
                     init_dump_info(adev->ext_contrl->dump_info->out_vaudio,FILE_PATH_VAUDIO_WAV,
                                adev->ext_contrl->dump_info->out_vaudio->buffer_length,
                                adev->ext_contrl->dump_info->dump_to_cache,
                                adev->ext_contrl->dump_info->dump_as_wav);
                }else{
                    init_dump_info(adev->ext_contrl->dump_info->out_vaudio,FILE_PATH_VAUDIO,
                                adev->ext_contrl->dump_info->out_vaudio->buffer_length,
                                adev->ext_contrl->dump_info->dump_to_cache,
                                adev->ext_contrl->dump_info->dump_as_wav);
                }
                adev->ext_contrl->dump_info->dump_vaudio = true;
            }
        }else{
            if(true==adev->ext_contrl->dump_info->dump_vaudio){
                adev->ext_contrl->dump_info->dump_vaudio = false;
                if(adev->ext_contrl->dump_info->dump_to_cache){
                    save_cache_buffer(adev->ext_contrl->dump_info->out_vaudio);
                }
                if(adev->ext_contrl->dump_info->dump_as_wav){
                    add_wav_header(adev->ext_contrl->dump_info->out_vaudio);
                }
                release_dump_info(adev->ext_contrl->dump_info->out_vaudio);
            }
    }

    if(switch_val&(1<<DUMP_MINI_RECORD_HAL)){
            if(false==adev->ext_contrl->dump_info->dump_in_read){
                if(adev->ext_contrl->dump_info->dump_as_wav){
                    init_dump_info(adev->ext_contrl->dump_info->in_read,FILE_PATH_INREAD_WAV,
                               adev->ext_contrl->dump_info->in_read->buffer_length,
                               adev->ext_contrl->dump_info->dump_to_cache,
                               adev->ext_contrl->dump_info->dump_as_wav);
                }else{
                    init_dump_info(adev->ext_contrl->dump_info->in_read,FILE_PATH_INREAD,
                               adev->ext_contrl->dump_info->in_read->buffer_length,
                               adev->ext_contrl->dump_info->dump_to_cache,
                               adev->ext_contrl->dump_info->dump_as_wav);
                }
                adev->ext_contrl->dump_info->dump_in_read = true;
            }
        }else{
            if(true==adev->ext_contrl->dump_info->dump_in_read){
                adev->ext_contrl->dump_info->dump_in_read = false;
                if(adev->ext_contrl->dump_info->dump_to_cache){
                    save_cache_buffer(adev->ext_contrl->dump_info->in_read);
                }
                if(adev->ext_contrl->dump_info->dump_as_wav){
                    add_wav_header(adev->ext_contrl->dump_info->in_read);
                }
                release_dump_info(adev->ext_contrl->dump_info->in_read);
            }
        }

    if(switch_val&(1<<DUMP_MINI_RECORD_AFTER_PROCESS)){
            if(false==adev->ext_contrl->dump_info->dump_in_read_afterprocess){
                if(adev->ext_contrl->dump_info->dump_as_wav){
                    init_dump_info(adev->ext_contrl->dump_info->in_read_afterprocess,FILE_PATH_INREAD_AFTER_PROCESS_WAV,
                               adev->ext_contrl->dump_info->in_read_afterprocess->buffer_length,
                               adev->ext_contrl->dump_info->dump_to_cache,
                               adev->ext_contrl->dump_info->dump_as_wav);
                }else{
                     init_dump_info(adev->ext_contrl->dump_info->in_read_afterprocess,FILE_PATH_INREAD_AFTER_PROCESS,
                               adev->ext_contrl->dump_info->in_read_afterprocess->buffer_length,
                               adev->ext_contrl->dump_info->dump_to_cache,
                               adev->ext_contrl->dump_info->dump_as_wav);
                }
                adev->ext_contrl->dump_info->dump_in_read_afterprocess = true;
            }
        }else{
            if(true==adev->ext_contrl->dump_info->dump_in_read_afterprocess){
                adev->ext_contrl->dump_info->dump_in_read_afterprocess = false;
                if(adev->ext_contrl->dump_info->dump_to_cache){
                    save_cache_buffer(adev->ext_contrl->dump_info->in_read_afterprocess);
                }
                if(adev->ext_contrl->dump_info->dump_as_wav){
                    add_wav_header(adev->ext_contrl->dump_info->in_read_afterprocess);
                }
                release_dump_info(adev->ext_contrl->dump_info->in_read_afterprocess);
            }
        }

    if(switch_val&(1<<DUMP_MINI_RECORD_AFTER_NR)){
            if(false==adev->ext_contrl->dump_info->dump_in_read_afternr){
                if(adev->ext_contrl->dump_info->dump_as_wav){
                    init_dump_info(adev->ext_contrl->dump_info->in_read_afternr,FILE_PATH_INREAD_AFTER_NR_WAV,
                               adev->ext_contrl->dump_info->in_read_afternr->buffer_length,
                               adev->ext_contrl->dump_info->dump_to_cache,
                               adev->ext_contrl->dump_info->dump_as_wav);
                }else{
                     init_dump_info(adev->ext_contrl->dump_info->in_read_afternr,FILE_PATH_INREAD_AFTER_NR,
                               adev->ext_contrl->dump_info->in_read_afternr->buffer_length,
                               adev->ext_contrl->dump_info->dump_to_cache,
                               adev->ext_contrl->dump_info->dump_as_wav);
                }
                adev->ext_contrl->dump_info->dump_in_read_afternr = true;
            }
        }else{
            if(true==adev->ext_contrl->dump_info->dump_in_read_afternr){
                adev->ext_contrl->dump_info->dump_in_read_afternr = false;
                if(adev->ext_contrl->dump_info->dump_to_cache){
                    save_cache_buffer(adev->ext_contrl->dump_info->in_read_afternr);
                }
                if(adev->ext_contrl->dump_info->dump_as_wav){
                    add_wav_header(adev->ext_contrl->dump_info->in_read_afternr);
                }
                release_dump_info(adev->ext_contrl->dump_info->in_read_afternr);
            }
        }

    if(switch_val&(1<<DUMP_MINI_RECORD_VBC)){
            if(false==adev->ext_contrl->dump_info->dump_in_read_vbc){
                if(adev->ext_contrl->dump_info->dump_as_wav){
                    init_dump_info(adev->ext_contrl->dump_info->in_read_vbc,FILE_PATH_INREAD_VBC_WAV,
                               adev->ext_contrl->dump_info->in_read_vbc->buffer_length,
                               adev->ext_contrl->dump_info->dump_to_cache,
                               adev->ext_contrl->dump_info->dump_as_wav);
                }else{
                     init_dump_info(adev->ext_contrl->dump_info->in_read_vbc,FILE_PATH_INREAD_VBC,
                               adev->ext_contrl->dump_info->in_read_vbc->buffer_length,
                               adev->ext_contrl->dump_info->dump_to_cache,
                               adev->ext_contrl->dump_info->dump_as_wav);
                }
                adev->ext_contrl->dump_info->dump_in_read_vbc = true;
            }
        }else{
            if(true==adev->ext_contrl->dump_info->dump_in_read_vbc){
                adev->ext_contrl->dump_info->dump_in_read_vbc = false;
                if(adev->ext_contrl->dump_info->dump_to_cache){
                    save_cache_buffer(adev->ext_contrl->dump_info->in_read_vbc);
                }
                if(adev->ext_contrl->dump_info->dump_as_wav){
                    add_wav_header(adev->ext_contrl->dump_info->in_read_vbc);
                }
                release_dump_info(adev->ext_contrl->dump_info->in_read_vbc);
            }
        }

    if(switch_val&(1<<DUMP_MINI_RECORD_AFTER_SRC)){
            if(false==adev->ext_contrl->dump_info->dump_in_read_aftersrc){
                if(adev->ext_contrl->dump_info->dump_as_wav){
                    init_dump_info(adev->ext_contrl->dump_info->in_read_aftersrc,FILE_PATH_INREAD_AFTER_SRC_WAV,
                               adev->ext_contrl->dump_info->in_read_aftersrc->buffer_length,
                               adev->ext_contrl->dump_info->dump_to_cache,
                               adev->ext_contrl->dump_info->dump_as_wav);
                }else{
                     init_dump_info(adev->ext_contrl->dump_info->in_read_aftersrc,FILE_PATH_INREAD_AFTER_RESAMPLER,
                               adev->ext_contrl->dump_info->in_read_aftersrc->buffer_length,
                               adev->ext_contrl->dump_info->dump_to_cache,
                               adev->ext_contrl->dump_info->dump_as_wav);
                }
                adev->ext_contrl->dump_info->dump_in_read_aftersrc = true;
            }
        }else{
            if(true==adev->ext_contrl->dump_info->dump_in_read_aftersrc){
                adev->ext_contrl->dump_info->dump_in_read_aftersrc = false;
                if(adev->ext_contrl->dump_info->dump_to_cache){
                    save_cache_buffer(adev->ext_contrl->dump_info->in_read_aftersrc);
                }
                if(adev->ext_contrl->dump_info->dump_as_wav){
                    add_wav_header(adev->ext_contrl->dump_info->in_read_aftersrc);
                }
                release_dump_info(adev->ext_contrl->dump_info->in_read_aftersrc);
            }
    }
}

/*************************************************
 *function:dump hal info to file
 *
 * ***********************************************/
void dump_hal_info(struct tiny_audio_device * adev){
    FILE* fd = fopen(FILE_PATH_HAL_INFO,"w+");
    if(fd == NULL){
        LOG_E("%s, open file err",__func__);
        return;
    }
    LOG_D("%s",__func__);
    fprintf(fd,"audio_mode_t:%d \n",adev->mode);
    fprintf(fd,"out_devices:%d \n",adev->out_devices);
    fprintf(fd,"in_devices:%d \n",adev->in_devices);
    fprintf(fd,"prev_out_devices:%d \n",adev->prev_out_devices);
    fprintf(fd,"prev_in_devices:%d \n",adev->prev_in_devices);
    fprintf(fd,"routeDev:%d \n",adev->routeDev);
    fprintf(fd,"cur_vbpipe_fd:%d \n",adev->cur_vbpipe_fd);
    fprintf(fd,"cp_type:%d \n",adev->cp_type);
    fprintf(fd,"call_start:%d \n",adev->call_start);
    fprintf(fd,"call_connected:%d \n",adev->call_connected);
    fprintf(fd,"call_prestop:%d \n",adev->call_prestop);
    fprintf(fd,"vbc_2arm:%d \n",adev->vbc_2arm);
    fprintf(fd,"voice_volume:%f \n",adev->voice_volume);
    fprintf(fd,"mic_mute:%d \n",adev->mic_mute);
    fprintf(fd,"bluetooth_nrec:%d \n",adev->bluetooth_nrec);
    fprintf(fd,"bluetooth_type:%d \n",adev->bluetooth_type);
    fprintf(fd,"low_power:%d \n",adev->low_power);
    fprintf(fd,"realCall:%d \n",adev->realCall);
    fprintf(fd,"num_dev_cfgs:%d \n",adev->num_dev_cfgs);
    fprintf(fd,"num_dev_linein_cfgs:%d \n",adev->num_dev_linein_cfgs);
    fprintf(fd,"eq_available:%d \n",adev->eq_available);
    fprintf(fd,"bt_sco_state:%d \n",adev->bt_sco_state);
    fprintf(fd,"voip_state:%d \n",adev->voip_state);
    fprintf(fd,"voip_start:%d \n",adev->voip_start);
    fprintf(fd,"master_mute:%d \n",adev->master_mute);
    fprintf(fd,"cache_mute:%d \n",adev->cache_mute);
    fprintf(fd,"fm_volume:%d \n",adev->fm_volume);
    fprintf(fd,"fm_open:%d \n",adev->fm_open);
    fprintf(fd,"requested_channel_cnt:%d \n",adev->requested_channel_cnt);
    fprintf(fd,"input_source:%d \n",adev->input_source);

    fprintf(fd,"adev dump info: \n");
    fprintf(fd,"loglevel:%d \n",log_level);
    fprintf(fd,"dump_to_cache:%d \n",adev->ext_contrl->dump_info->dump_to_cache);
    fprintf(fd,"dump_as_wav:%d \n",adev->ext_contrl->dump_info->dump_as_wav);
    fprintf(fd,"dump_music:%d \n",adev->ext_contrl->dump_info->dump_music);
    fprintf(fd,"dump_vaudio:%d \n",adev->ext_contrl->dump_info->dump_vaudio);
    fprintf(fd,"dump_voip:%d \n",adev->ext_contrl->dump_info->dump_voip);
    fprintf(fd,"dump_bt_sco:%d \n",adev->ext_contrl->dump_info->dump_bt_sco);

    fclose(fd);
    return;
}

static int sendandrecv(char* pipe,char*cmdstring,struct timeval* timeout,void* buffer,int size){
    int ret = 0;
    int max_fd_dump;
    int pipe_dump;
    int left = size;
    LOG_D("%s : %s",__func__,cmdstring);
    pipe_dump = open(pipe, O_RDWR);
    if(pipe_dump < 0){
        LOG_E("%s, open %s fail %d",__func__,pipe,errno);
        return -1;
    } else {
        if((fcntl(pipe_dump,F_SETFL,O_NONBLOCK))<0)
        {
            ALOGD("cat set pipe_dump nonblock error!");
        }
        ret = write_nonblock(pipe_dump,cmdstring,strlen(cmdstring));
        if(ret < 0){
            LOG_E("wrrite noblock error ");
            goto exit;
        }
        fd_set fds_read_dump;
        max_fd_dump = pipe_dump + 1;
        while(left > 0) {
            FD_ZERO(&fds_read_dump);
            FD_SET(pipe_dump,&fds_read_dump);
            ret = select(max_fd_dump,&fds_read_dump,NULL,NULL,timeout);
            if(ret < 0){
                LOG_E("cat select error ");
                goto exit;
            }
            if(FD_ISSET(pipe_dump,&fds_read_dump) <= 0 ){
                ret = -1;
                LOG_E("cat SELECT OK BUT NO fd is set");
                goto exit;
            }
            ret = read_noblock_l(pipe_dump,buffer+(size-left),left);
            if(ret < 0){
                LOG_E("cat read data err");
                goto exit;
            }
            left -= ret;
        }
    }
exit:
    close(pipe_dump);
    return ret;
}


static int set_duration(char* pipe,char* value){
    char *curindx = NULL;
    char *preindx = NULL;
    char data[32]= {0};
    int duration = 0;//s
    int rsp = 0;
    int ret = 0,result;
    struct timeval timeout = {5,0};
    if(strlen(value) != 0){
        duration = atoi(value);
        if( 0 < duration && duration <= CP_MAX_DURATION){
            sprintf(data,"%s=%d",CP_DUMP_DURATION,duration);
            ret = sendandrecv(pipe,data,&timeout,&rsp,sizeof(int));
            LOG_D("set duration %d sucess. ",rsp);
        } else {
            LOG_D("Because of ARM precious resource.the required duration %d bigger than the MAX duration %d must be fail.",duration,CP_MAX_DURATION);
        }
    }
    return ret;
}
static int string2intarray(char* string,int* pointarray,int arraysize){
    char *firstnum= NULL;
    char *preindx = NULL;
    char *curindx = NULL;
    bool isnum = 0;
    char substr[32] = {0};
    int ret = 0;
    int point = 0;
    int pointnum = 0;
    if(string == NULL || pointarray == NULL){
        LOG_D(" NULL pointer.",__func__);
        return -1;
    }
    LOG_D("%s command:%s.",__func__,string);
    if(strlen(string) != 0){
        firstnum = string;
        while((firstnum - string) < sizeof(string)){
            char c = *firstnum;
            if(c <='9'&& c >= '0'){
                isnum = 1;
                break;
            }
            firstnum++;
        }
        preindx = firstnum;
        LOG_V("parse from:%d character",(firstnum-string)+1);

        while((preindx != NULL) && isnum) {
            bool illegal_zero = 0;
            curindx = strstr(preindx,",");
            if(curindx == NULL){
                memcpy(substr,preindx,strlen(preindx));
            } else {
                memcpy(substr,preindx,curindx-preindx);
            }
            point = atoi(substr);
            if( (point==0) && (*substr !='0')){
                illegal_zero = 1 ;// atio("abcd") return 0
            }

            if(point >=0 && point < arraysize && !illegal_zero){
                LOG_D("set point:%s,int:%d",substr,point);
                pointarray[pointnum] = point;
                pointnum++;
            } else {
                if(illegal_zero){
                    LOG_D("fail: illegal string %s ",substr);
                } else {
                    LOG_D("fail: the point %s: reach the MAX NUM supported in cp",substr);
                }
            }
            preindx = (curindx == NULL) ? NULL:(curindx+1);
            memset(substr,0x00,sizeof(substr));
        }
    }
    return pointnum;
}

static int set_point(char *pipe,char* value,int enable)
{
    char *curindx = NULL;
    char *preindx = NULL;
    char *firstnum= NULL;
    bool isnum = 0;
    char cmd[32] = {0};
    int ret = 0;
    int point = 0;
    int pointarray[32] = {0};
    int pointnum = 0;
    struct timeval timeout = {5,0};
    int cur = 0;
    int rsp = 0;
    pointnum  = string2intarray(value,pointarray,sizeof(pointarray));
    if(pointnum <= 0){
        return -1;
    }
    while(cur < pointnum){
        sprintf(cmd,"%s=%d",(enable ? CP_DUMP_POINT_ENABLE : CP_DUMP_POINT_DISABLE),pointarray[cur]);
        ret = sendandrecv(pipe,cmd,&timeout,&rsp,sizeof(int));
        if(ret < 0){
            LOG_E("read data err");
            return -1;
        }
        LOG_D("%s point:%d suceess.",(enable ? "enable":"disable"),rsp);
        cur++;
    }
    return ret;
}
static int get_duration(char *pipe){
    int ret = 0;
    struct timeval timeout = {5,0};
    int duration = 0;
    ret = sendandrecv(pipe,CP_DUMP_POINT_GETDUARTION,&timeout,&duration,sizeof(int));
    if(ret  < -1){
        return -1;
    }
    LOG_D("%s,ret:%d,duration:0x%x",__func__,ret,duration);
    return duration;
}

static int get_enablepoint(char *pipe){
    int ret = 0;
    struct timeval timeout = {5,0};
    int enablebits = 0;
    ret = sendandrecv(pipe,CP_DUMP_POINT_DISPLAY,&timeout,&enablebits,sizeof(int));
    if(ret  < -1){
        return -1;
    }
    LOG_D("%s,ret:%d,enablebits:0x%x",__func__,ret,enablebits);
    return enablebits;
}

static  int get_pointinfo(char *pipe,bool savefile) {
    LOG_V("cat cppoint start f");
    int ret = -1,result;
    struct timeval timeout = {5,0};
    char buffer[32] = {0};
    FILE* dump_fd = NULL;
    int enablebits = 0;
    int temp = 0;
    int point[CP_MAX_POINT]= {0};
    int tzero = 0;
    int duration = 0;
    int indx = 0;
    enablebits = get_enablepoint(pipe);
    if(enablebits < -1){
        return -1;
    }

    LOG_D("cat enablebits,enablebits:0x%x,enablenumber:%d",enablebits,__builtin_popcount(enablebits));
    if(__builtin_popcount(enablebits) == 0){
        LOG_E(" No enable point");
    }
    //parse the enable point from int
    temp = enablebits;
    while(tzero < CP_MAX_POINT && 0 != temp){
        tzero = __builtin_ctz(temp);
        if(tzero < CP_MAX_POINT){
            point[tzero] = 1;
            LOG_E("The enable point %d .",tzero);
        }
        temp &= ~(1 << tzero);
    }

    duration = get_duration(pipe);
    if(duration < -1){
        goto exit;
    }
    LOG_D("cat duration,ret:%d,duration:0x%x",ret,duration);

    if(savefile){
        dump_fd = fopen(FILE_PATH_CP_ENABLE_POINT_INFO,"wb");
        if(dump_fd == NULL){
            LOG_E("cat fopen %s err",FILE_PATH_CP_ENABLE_POINT_INFO);
            goto exit;
        }
        ret = dump_to_file(dump_fd,"enable point: ",strlen("enable point:"));
        for(indx = 0;indx < CP_MAX_POINT;indx++){
            if(enablebits & (1<<indx)){
                memset(buffer,0x00,sizeof(buffer));
                sprintf(buffer,"%d  ",indx);
                ret = dump_to_file(dump_fd,buffer,strlen(buffer));
                if(ret < 0){
                    fclose(dump_fd);
                    LOG_E("cat dump read data err");
                    goto exit;
                }
            }
        }
        ret = dump_to_file(dump_fd,"\n",strlen("\n"));

        memset(buffer,0x00,sizeof(buffer));
        sprintf(buffer,"duration: %ds",duration);
        ret = dump_to_file(dump_fd,buffer,strlen(buffer));
        if(ret < 0){
            LOG_E("cat dump read data err");
            fclose(dump_fd);
            goto exit;
        }
        ret = dump_to_file(dump_fd,"\n",strlen("\n"));
        fclose(dump_fd);
    }
exit:
    return ret;
}

char* get_pointinfo_hal(char *pipe) {
    LOG_V("cat cppoint start f");
    int ret = -1,result;
    struct timeval timeout = {5,0};
    char buffer[32] = {0};
    char buffer_all[1024] = {0};
    int enablebits = 0;
    int temp = 0;
    int point[CP_MAX_POINT]= {0};
    int tzero = 0;
    int duration = 0;
    int indx = 0;
    enablebits = get_enablepoint(pipe);
    if(enablebits < -1){
        return NULL;
    }

    LOG_D("cat enablebits,enablebits:0x%x,enablenumber:%d",enablebits,__builtin_popcount(enablebits));
    if(__builtin_popcount(enablebits) == 0){
        LOG_E(" No enable point");
    }
    //parse the enable point from int
    temp = enablebits;
    while(tzero < CP_MAX_POINT && 0 != temp){
        tzero = __builtin_ctz(temp);
        if(tzero < CP_MAX_POINT){
            point[tzero] = 1;
            LOG_E("The enable point %d .",tzero);
        }
        temp &= ~(1 << tzero);
    }

    duration = get_duration(pipe);
    if(duration < -1){
        goto exit;
    }
    LOG_D("cat duration,ret:%d,duration:0x%x",ret,duration);
    memset(buffer,0x00,sizeof(buffer));
    sprintf(buffer,"enable point:  ");
    strcat(buffer_all,buffer);
    for(indx = 0;indx < CP_MAX_POINT;indx++){
        if(enablebits & (1<<indx)){
            memset(buffer,0x00,sizeof(buffer));
            sprintf(buffer,"%d  ",indx);
            strcat(buffer_all,buffer);
        }
    }
    memset(buffer,0x00,sizeof(buffer));
    sprintf(buffer,"\n\nduration: %ds\n",duration);
    strcat(buffer_all,buffer);
    return buffer_all;
exit:
    return NULL;
}


static int ext_property_get(const char* key, char *get_vaule, const char *default_value) {
    int rc = 0;
    int length = 0;
    if ((NULL == key) || (NULL == get_vaule)) {
        LOG_E("%s error! argin is NULL %p %p", __FUNCTION__, key, get_vaule);
        rc = -1;
        goto error;
    }
    length = property_get(key , get_vaule, default_value);
    LOG_I("%s call property_get ! key:%s get_value:%s length = %d",
        __FUNCTION__, key, get_vaule, length);
    return rc;
error:
    LOG_E("%s is fail! rc = %d", __FUNCTION__, rc);
    return rc;
}

static int ext_property_set(const char* key, const char* value) {
    int rc = 0;
    if (NULL == key || NULL == value) {
        LOG_E("%s error: argin is NULL! %p %p", __FUNCTION__, key, value);
        rc = -1;
        goto error;
    }
    rc = property_set(key, value);
    if (0 != rc) {
        LOG_E("%s error: call property_set is fail!", __FUNCTION__);
        goto error;
    }
    LOG_I("key = %s value = %s", key, value);
    return rc;

error:
    LOG_E("%s is error! rc = %d", __FUNCTION__, rc);
    return rc;
}

static int ext_get_dump_data_switch(void) {
    int rc = 0;
    char value[92]={0};
    int dump_switch = 0;
    rc = ext_property_get("media.dump.switch", value, NULL);
    if (0 != rc) {
        LOG_E("%s call ext_propery_get is fail! rc = %d", __FUNCTION__, rc);
        rc = -1;
    }

    rc = sscanf(value , "%x",&(dump_switch));
    if (rc <0 ) {
        ALOGE(" dump switch is error!");
        rc = -2;
    }
    set_dump_switch(dump_switch);
    return rc;
}


void  savememory2file(void*buffer,int size,int point){
    char filepath[MAX_FILE_LENGTH] = {0};
    FILE* dump_fd = NULL;
    int ret = 0;
    sprintf(filepath,"%s%d%s",FILE_PATH_CP_RAM_PRE,point,".pcm");
    LOG_E("%s %s",__func__,filepath);
    dump_fd = fopen(filepath,"wb");
    if(dump_fd == NULL){
        LOG_E("cat fopen data err:%s",filepath);
        return;
    }
    ret = dump_to_file(dump_fd,buffer,size);
    if(ret < 0){
        LOG_E("cat dump read data err");
        fclose(dump_fd);
        return;
    }
    fclose(dump_fd);
    return;
}

static int dump_voice(char* value,char* pipe)
{
    LOG_V("ramdump start.");
    int ret;
    struct timeval timeout = {5,0};
    void *buffer = NULL;
    int length = 0;
    int enablebits = 0;
    int cur = 0;
    int duration = 0;
    char cmd[32] = {0};
    int pointarray[32] = {0};
    int pointnum = 0;
    enablebits = get_enablepoint(pipe);
    if(enablebits <= 0){
        return -1;
    }
    pointnum  = string2intarray(value,pointarray,sizeof(pointarray));
    while(cur < pointnum){
        LOG_D("%s %d poind%d,%d",__func__,pointnum, cur,pointarray[cur]);
        cur++;
    }

    if(0 == strncmp(value,"all",strlen("all"))){
        pointnum = CP_MAX_POINT; //dump all point
        for(cur=0;cur < sizeof(pointarray)/sizeof(pointarray[0]);cur++){
            pointarray[cur] = cur;
        }
    }

    if(pointnum > 0){
        for(cur=0;cur<pointnum;cur++){
            if(enablebits & (1 << pointarray[cur])){
                sprintf(cmd,"%s=%d",CP_DUMP_POINTSIZE,pointarray[cur]);
                ret = sendandrecv(pipe,cmd,&timeout,&length,sizeof(int));
                LOG_D("%s,point:%d,length:%d",__func__,pointarray[cur],length);
                if(ret < 0 || (length == 0)){
                    LOG_D("%s,ret:%d,length:%d",__func__,ret,length);
                    continue;
                }
                buffer = malloc(length);
                if(buffer == NULL){
                    return -1;
                }
                sprintf(cmd,"%s=%d",CP_DUMP_DATA,pointarray[cur]);
                ret = sendandrecv(pipe,cmd,&timeout,buffer,length);
                if(ret < 0){
                    break;
                }
                savememory2file(buffer,length,pointarray[cur]);
                free(buffer);
                buffer = NULL;
            }
        }
    }
    if(buffer != NULL){
        free(buffer);
        buffer = NULL;
    }
    return ret;
}

void *audio_devices_cp_loop_in_test_thread(void *args){
    struct in_test_t *in_test=(struct in_test_t *)args;
    struct dev_test_t *dev_test = in_test->dev_test;
    struct pcm *pcm;
    char *buffer,*proc_buf;
    unsigned int size,proc_buf_size;
    unsigned int bytes_read = 0;
    int card = -1;
    int num_read = 0;
    int read_count = 0;
    int process_init = 0;
    int is_notify_to_play = 0;
    int invalid_data_size=0;

    if(dev_test)
//        card=get_snd_card_number(audio_devices_cp_loop_get_card_name(dev_test->adev));
        card=get_snd_card_number(adev_get_cp_card_name(dev_test->adev));
    if(card < 0) {
        return NULL;
    }
    pcm=NULL;
    buffer=NULL;
    proc_buf=NULL;
    proc_buf_size=0;
    bytes_read=0;

    LOG_E("yaye audio_devices_cp_loop_in_test_thread wait");
    sem_wait(&in_test->sem);
    LOG_E("yaye audio_devices_cp_loop_in_test_thread running");

    pcm = pcm_open(card, PORT_MM, PCM_IN, &in_test->config.config);
    if (!pcm || !pcm_is_ready(pcm)) {
        LOG_E("yaye audio_devices_cp_loop_in_test_thread  Unable to open PCM device (%s)\n",pcm_get_error(pcm));
        goto ERR;
    }
    LOG_E("YAYE audio_devices_cp_loop_in_test_thread pcm_open success!!!");

    size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));
    buffer = malloc(size);
    if (!buffer) {
        LOG_E("yaye audio_devices_cp_loop_in_test_thread  Unable to allocate %d bytes\n", size);
        goto ERR;
    }

    LOG_E("YAYE audio_devices_cp_loop_in_test_thread malloc_buffer success,size:%d,isloop:%d!!!", size,in_test->isloop);
    invalid_data_size=(TEST_AUDIO_INVALID_DATA_MS *in_test->config.config.rate
                *in_test->config.config.channels*2/1000);

    proc_buf_size = size;
    proc_buf = malloc(proc_buf_size);
    if (!proc_buf) {
        LOG_E("yaye audio_devices_cp_loop_in_test_thread Unable to allocate %d bytes\n", size);
        goto ERR;
    }
    LOG_E("YAYE audio_devices_cp_loop_in_test_thread malloc_buffer success,invalid_data_size:%d!!!", invalid_data_size);

    if(in_test->isloop == true){
        in_test->config.fd=open(AUDIO_TEST_CP_LOOP_RECD_PCM_FILE, O_WRONLY|O_CREAT,0660);
        if(in_test->config.fd<0){
            LOG_E("yaye audio_devices_cp_loop_in_test_thread Unable to open file%s", AUDIO_TEST_CP_LOOP_RECD_PCM_FILE);
            goto ERR;
        }
        if(chmod(AUDIO_TEST_CP_LOOP_RECD_PCM_FILE, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) != 0){
            LOG_E("%s Cannot set RW to \"%s\": %s", __FUNCTION__, AUDIO_TEST_CP_LOOP_RECD_PCM_FILE, strerror(errno));
        }
    }
    memset(buffer,0,size);
    memset(proc_buf,0,proc_buf_size);
    LOG_E("yaye audio_devices_cp_loop_in_test_thread begin capture!!!!!!!!!!!!!!!!");

    do{
        num_read=pcm_read(pcm, buffer, size);
        LOG_D("yaye audio_devices_cp_loop_in_test_thread  capture read:%d req:0x%x",num_read,size);
        read_count++;

        if (!num_read){
            bytes_read += size;

            if(bytes_read >invalid_data_size){

                if(in_test->config.fd > 0){
                    if (write(in_test->config.fd,buffer,size) != size){
                        LOG_E("yaye audio_devices_cp_loop_in_test_thread write err\n");
                    }
                }
            }

            if((in_test->isloop == true) && (is_notify_to_play == 0)
                && (bytes_read > ((TEST_LOOPBACK_BUFFER_DATA_MS *in_test->config.config.rate
                *in_test->config.config.channels*2/1000)+invalid_data_size))) {
                is_notify_to_play = 1;
                sem_post(&dev_test->out_test.sem);
            }
            LOG_D("yaye audio_devices_cp_loop_in_test_thread %d bytes bytes_read:0x%x",size,bytes_read);
        }else{
            LOG_E("yaye audio_devices_cp_loop_in_test_thread: no data read num_read:%d",num_read);
            usleep(20000);
        }
    }while(in_test->state);

ERR:

    if(buffer){
        free(buffer);
        buffer=NULL;
    }

    if(proc_buf){
        free(proc_buf);
        proc_buf=NULL;
    }

    if(pcm){
        pcm_close(pcm);
        pcm=NULL;
    }
    if(in_test->config.fd >0){
        close(in_test->config.fd);
        in_test->config.fd=-1;
    }

    LOG_E("yaye audio_devices_cp_loop_in_test_thread exit");
    return NULL;
}

void *audio_devices_cp_loop_out_test_thread(void *args){
    struct in_test_t *out_test=(struct in_test_t *)args;
    struct dev_test_t *dev_test = out_test->dev_test;
    struct pcm *pcm;
    char *buffer;
    unsigned int size;
    int playback_fd;
    int num_read;
    int ret = 0;
    int card=-1;

    if(dev_test)
//        card=get_snd_card_number(audio_devices_cp_loop_get_card_name(dev_test->adev));
        card=get_snd_card_number(adev_get_cp_card_name(dev_test->adev));
    if(card < 0) {
        return NULL;
    }

    pcm = NULL;
    buffer = NULL;
    num_read = 0;
    size = 0;

    LOG_E("YAYE audio_devices_cp_loop_out_test_thread wait");
    sem_wait(&out_test->sem);
    LOG_E("YAYE audio_devices_cp_loop_out_test_thread running");

    pcm = pcm_open(card, PORT_MM, PCM_OUT, &out_test->config.config);
    if (!pcm || !pcm_is_ready(pcm)) {
        LOG_E("YAYE audio_devices_cp_loop_out_test_thread  Unable to open PCM device %u (%s)\n",
              0, pcm_get_error(pcm));
        goto ERR;
    }
    LOG_E("YAYE audio_devices_cp_loop_out_test_thread pcm_open success!!!");

    size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));
    buffer = malloc(size);
    if (!buffer) {
        LOG_E("YAYE audio_devices_cp_loop_out_test_thread  Unable to allocate %d bytes\n", size);
        goto ERR;
    }

    LOG_E("YAYE audio_devices_cp_loop_out_test_thread malloc_buffer success,size:%d,isloop:%d!!!", size,out_test->isloop);
    if(out_test->isloop == true){
        out_test->config.fd=open(AUDIO_TEST_CP_LOOP_PLAY_PCM_FILE, O_RDONLY,0660);
        if(out_test->config.fd<0)
            out_test->config.fd=open(AUDIO_TEST_CP_LOOP_PLAY_PCM_FILE_ROOT, O_RDONLY,0660);
        if(out_test->config.fd<0){
            LOG_E("YAYE audio_devices_cp_loop_out_test_thread unable to open file %s!!!", AUDIO_TEST_CP_LOOP_PLAY_PCM_FILE_ROOT);
            goto ERR;
        }
    }

    LOG_E("YAYE audio_devices_cp_loop_out_test_thread state %d!!!", out_test->state);
    do {
        if(out_test->config.fd > 0){
            num_read = read(out_test->config.fd, buffer, size);
            LOG_D("YAYE audio_devices_cp_loop_out_test_thread read_data num:%d", num_read);
            if(num_read<=0){
                lseek(out_test->config.fd,0,SEEK_SET);
                num_read = read(out_test->config.fd, buffer, size);
                LOG_D("YAYE audio_devices_cp_loop_out_test_thread 1 read_data1 num:%d", num_read);
            }
            ret = pcm_write(pcm, buffer,num_read);
        }
        else {
            num_read = sizeof(s_pcmdata_mono);
            ret = pcm_write(pcm, s_pcmdata_mono,num_read);
            LOG_E("YAYE audio_devices_cp_loop_out_test_thread Error write fixed data ret%d", ret);
        }
        if (ret) {
            int ret = 0;
            LOG_E("YAYE audio_devices_cp_loop_out_test_thread Error playing 1 sample:%s\n",pcm_get_error(pcm));
            ret = pcm_close(pcm);
            pcm = pcm_open(card, PORT_MM, PCM_OUT, &out_test->config.config);
            if (!pcm || !pcm_is_ready(pcm)) {
                LOG_E("YAYE audio_devices_cp_loop_out_test_thread Error Unable to open PCM device %u (%s)\n",
                      0, pcm_get_error(pcm));
                goto ERR;
            }
        }
        LOG_D("YAYE audio_devices_cp_loop_out_test_thread write ok");
    }while(out_test->state);

ERR:

    if(buffer) {
        free(buffer);
        buffer = NULL;
    }

    if(pcm) {
        pcm_close(pcm);
        pcm = NULL;
    }

    if(out_test->config.fd > 0) {
        close(out_test->config.fd);
        out_test->config.fd = -1;
    }
    LOG_E("YAYE audio_devices_cp_loop_out_test_thread exit");
    return NULL;
}

static int audio_devices_cp_loop_in_test(struct tiny_audio_device *adev,int us_loop_route){
    int ret=0;
    struct dev_test_t *dev_test = (struct dev_test_t *) &adev->ext_contrl->dev_test;
    struct in_test_t *in_test = (struct in_test_t *) &dev_test->in_test;
    struct mixer_ctl *ctl1;
    struct mixer_ctl *ctl2;
    struct mixer_ctl *ctl3;
    struct mixer *in_mixer= NULL;

    LOG_E("YAYE audio_devices_cp_loop_in_test entry, us_loop_route:%d!!!",us_loop_route);
    in_test->dev_test=dev_test;
    dev_test->adev=adev;

    {
        LOG_E("yaye audio_devices_cp_loop_in_test entry!!!");
        in_mixer = adev->cp_mixer[adev->cp_type];
        ctl2 = mixer_get_ctl_by_name(in_mixer,"PCM LOOP Type");
        mixer_ctl_set_value(ctl2, 0, us_loop_route);
        ctl1 = mixer_get_ctl_by_name(in_mixer,"PCM LOOP Enable");
        mixer_ctl_set_value(ctl1, 0, 1);
        LOG_E("yaye audio_devices_cp_loop_in_test set_mixer ctl1:%x value:%d ctl2:%x value:%",
            ctl1, 1, ctl2, us_loop_route);

        ctl3 = mixer_get_ctl_by_name(in_mixer,"PCM CAPTURE Route");
        mixer_ctl_set_value(ctl3, 0, 2);
        LOG_E("yaye audio_devices_cp_loop_in_test set_mixer ctl:%x value:%d",
            ctl3, 2);
    }

    in_test->isloop=true;
    in_test->config.config = cp_loop_record_config;
    in_test->state=true;

    sem_init(&in_test->sem, 0, 1);
    if(pthread_create(&in_test->thread, NULL, audio_devices_cp_loop_in_test_thread, (void *)in_test)){
        LOG_E("audiotest_capture_thread creating failed !!!!");
        goto error;
    }

    return 0;
error:
    return -1;
}

static int audio_devices_cp_loop_out_test(struct tiny_audio_device *adev,int us_loop_route){
    int ret=0;
    struct dev_test_t *dev_test = (struct dev_test_t *) &adev->ext_contrl->dev_test;
    struct out_test_t *out_test = (struct out_test_t *) &dev_test->out_test;
    struct mixer_ctl *ctl1;
    struct mixer_ctl *ctl2;
    struct mixer_ctl *ctl3;
    struct mixer *out_mixer= NULL;

    LOG_E("YAYE audio_devices_cp_loop_out_test entry, us_loop_route:%d!!!",us_loop_route);
    out_test->dev_test=dev_test;
    dev_test->adev=adev;

    {
        LOG_E("yaye audio_devices_cp_loop_out_test entry!!!");
        out_mixer = adev->cp_mixer[adev->cp_type];
        ctl2 = mixer_get_ctl_by_name(out_mixer,"PCM LOOP Type");
        mixer_ctl_set_value(ctl2, 0, us_loop_route);
        ctl1 = mixer_get_ctl_by_name(out_mixer,"PCM LOOP Enable");
        mixer_ctl_set_value(ctl1, 0, 1);
        LOG_E("yaye audio_devices_cp_loop_out_test ctl1:%x value:%d ctl2:%x value:%d", ctl1, 1, ctl2, us_loop_route);

        ctl3 = mixer_get_ctl_by_name(out_mixer,"PCM PLAYBACK Route");
        mixer_ctl_set_value(ctl3, 0, 1);
        LOG_E("yaye audio_devices_cp_loop_out_test ctl:%x value:%d", ctl3, 1);
    }

    out_test->isloop=true;
    out_test->config.config = cp_loop_playback_config;
    out_test->state=true;

    sem_init(&out_test->sem, 0, 1);
    if(pthread_create(&out_test->thread, NULL, audio_devices_cp_loop_out_test_thread, (void *)out_test)){
        LOG_E("audio_out_devices_test creating failed !!!!");
        goto error;
    }

    return 0;
error:
    return -1;
}

static int audio_devices_cp_loop_loop_test(struct tiny_audio_device *adev,int us_loop_route){
    int ret=0;

    struct dev_test_t *dev_test = (struct dev_test_t *) &adev->ext_contrl->dev_test;
    struct mixer_ctl *ctl1;
    struct mixer_ctl *ctl2;
    struct mixer *loop_mixer= NULL;

    loop_mixer = adev->cp_mixer[adev->cp_type];
    ctl2 = mixer_get_ctl_by_name(loop_mixer,"PCM LOOP Type");
    mixer_ctl_set_value(ctl2, 0, us_loop_route);
    ctl1 = mixer_get_ctl_by_name(loop_mixer,"PCM LOOP Enable");
    mixer_ctl_set_value(ctl1, 0, 1);
    LOG_E("YAYE mixer_ctl_set_value card_num:%d ctl1:%x value:%d ctl2:%x value:%d", ctl1, 1, ctl2, us_loop_route);

    return 0;
}

static int audio_devices_cp_loop_test(struct tiny_audio_device *adev,int us_loop_route){
    int ret=0;

    struct dev_test_t *dev_test = (struct dev_test_t *) &adev->ext_contrl->dev_test;
    struct in_test_t *in_test = (struct in_test_t *) &dev_test->in_test;
    struct out_test_t *out_test = (struct out_test_t *) &dev_test->out_test;
    in_test->dev_test=dev_test;
    dev_test->adev=adev;

    LOG_E("YAYE audio_devices_cp_loop_test entry test_mode:%d, adev:%x, dev_test:%x, in_test%x, out_test:%x",
        dev_test->test_mode, adev, dev_test, in_test, out_test);
    if(TEST_AUDIO_IDLE != dev_test->test_mode){
        LOG_E("audio_loop_devices_test mode:%d",dev_test->test_mode);
        return -1;
    }else{
        dev_test->test_mode=TEST_AUDIO_CP_LOOP;
    }

    switch(us_loop_route)
    {
        case 0:
            ret = audio_devices_cp_loop_out_test(adev, us_loop_route);
            break;
        case 1:
            ret = audio_devices_cp_loop_in_test(adev, us_loop_route);
            break;
        case 2:
            ret = audio_devices_cp_loop_loop_test(adev, us_loop_route);
            break;
        default:
            LOG_E("yaye audio_devices_cp_loop_test invalid loop_route:%d !!!!", us_loop_route);
            break;
    }

    LOG_E("YAYE audio_devices_cp_loop_test start result:%d!!! test_mode:%d",ret,dev_test->test_mode);
    if(0 != ret){
        dev_test->test_mode = TEST_AUDIO_IDLE;
        LOG_E("YAYE audio_devices_cp_loop_test start return err!!! test_mode:%d",dev_test->test_mode);
        return -1;
    }
    return 0;
}

static int audio_devices_cp_loop_test_stop(struct tiny_audio_device *adev,int us_loop_route){
    int ret = -1;
    struct dev_test_t *dev_test = (struct dev_test_t *) &adev->ext_contrl->dev_test;
    struct in_test_t *in_test = (struct in_test_t *) &dev_test->in_test;
    struct out_test_t *out_test = (struct out_test_t *) &dev_test->out_test;
    struct mixer_ctl *ctl1;
    struct mixer_ctl *ctl2;
    struct mixer_ctl *ctl3;
    struct mixer_ctl *ctl4;
    struct mixer *s_mixer= NULL;

    LOG_E("YAYE audio_devices_cp_loop_test_stop entry test_mode:%d, in_test->state:%d, in_test->thread:%x, out_test->state:%d, out_test->thread:%x",
        dev_test->test_mode, in_test->state, in_test->thread, out_test->state, out_test->thread);
    if(TEST_AUDIO_CP_LOOP != dev_test->test_mode){
        LOG_E("YAYE audio_devices_cp_loop_test_stop exit! not been start, no need to stop!");
        return -1;
    }

    in_test->state=0;
    out_test->state=0;
    if(in_test->thread){
        ret = pthread_join(in_test->thread, NULL);
        sem_destroy(&in_test->sem);
        in_test->thread = 0;
    }

    if(out_test->thread){
        ret = pthread_join(out_test->thread, NULL);
        sem_destroy(&out_test->sem);
        out_test->thread = 0;
    }

    ALOGE("audio_loop_devices_test_stop thread destory ret is %d", ret);

    {
        s_mixer = adev->cp_mixer[adev->cp_type];
        ctl2 = mixer_get_ctl_by_name(s_mixer,"PCM LOOP Type");
        mixer_ctl_set_value(ctl2, 0, us_loop_route);
        ctl1 = mixer_get_ctl_by_name(s_mixer,"PCM LOOP Enable");
        mixer_ctl_set_value(ctl1, 0, 0);
        LOG_E("YAYE audio_devices_cp_loop_test_stop ctl1:%x value:%d ctl2:%x value:%d", ctl1, 0, ctl2, us_loop_route);

        ctl3 = mixer_get_ctl_by_name(s_mixer,"PCM PLAYBACK Route");
        mixer_ctl_set_value(ctl3, 0, 0);
        ctl4 = mixer_get_ctl_by_name(s_mixer,"PCM CAPTURE Route");
        mixer_ctl_set_value(ctl4, 0, 0);
        LOG_E("YAYE audio_devices_cp_loop_test_stop ctl3:%x value:%d ctl4:%x value:%d", ctl3, 0, ctl4, 0);
    }
    dev_test->test_mode=TEST_AUDIO_IDLE;

    return 0;
}

static void *control_audio_loop_process(void *arg){
    int pipe_fd,max_fd;
    fd_set fds_read;
    int result;
    int count;
    void* data;
    int val_int;
    struct str_parms *parms;
    char value[30];
    int ret = 0;
    int retdump;
    struct tiny_audio_device *adev = (struct tiny_audio_device *)arg;
    FILE* help_fd = NULL;
    void* help_buffer = NULL;
    FILE* pipe_d = NULL;

    if(access(AUDIO_EXT_CONTROL_PIPE, R_OK) ==0){
        pipe_fd = open(AUDIO_EXT_CONTROL_PIPE, O_RDWR);
    }else{
        pipe_fd = open(AUDIO_EXT_DATA_CONTROL_PIPE, O_RDWR);
    }

    if(pipe_fd < 0){
        LOG_E("%s, open pipe error!! ",__func__);
        return NULL;
    }

    max_fd = pipe_fd + 1;
    if((fcntl(pipe_fd,F_SETFL,O_NONBLOCK)) <0){
        LOG_E("set flag RROR --------");
    }
    data = (char*)malloc(1024);
    if(data == NULL){
        LOG_E("malloc data err");
        return NULL;
    }
    LOG_I("begin to receive audio control message");
    while(!adev->ext_contrl->entryMgr.isExit && adev != NULL){
        FD_ZERO(&fds_read);
        FD_SET(pipe_fd,&fds_read);
        result = select(max_fd,&fds_read,NULL,NULL,NULL);
        if(result < 0){
            LOG_E("select error ");
            continue;
        }
        if(FD_ISSET(pipe_fd,&fds_read) <= 0 ){
            LOG_E("SELECT OK BUT NO fd is set");
            continue;
        }
        memset(data,0,1024);
        count = read_noblock_l(pipe_fd,data,1024);
        if(count < 0){
            LOG_E("read data err");
            empty_command_pipe(pipe_fd);
            continue;
        }
        LOG_E("data:%s ",data);
        if (0 == strncmp(data, "entry_exit", strlen("entry_exit"))) {
            ALOGI("control_audio_loop_process is exit!");
            continue;
        }
        adev_set_parameters(adev,data);
        parms = str_parms_create_str(data);

        ret = str_parms_get_str(parms,"loglevel", value, sizeof(value));
        if(ret >= 0){
            val_int = atoi(value);
            LOG_D("log is :%d",val_int);
            if(val_int >= 0){
                log_level = val_int;
            }
        }

        ret = str_parms_get_str(parms,"help",value,sizeof(value));
        if(ret >= 0){
            help_fd = fopen(FILE_PATH_HELP,"rb");
            pipe_d = fopen("/dev/pipe/mmi.audio.ctrl","wb");
            help_buffer = (void*)malloc(1024);
            if(help_fd == NULL || pipe_d == NULL){
                LOG_E("ERROR ------------");
            }else{
                while(fgets(help_buffer,1024,help_fd)){
                    fputs(help_buffer,pipe_d);
                }
            }
            fclose(pipe_d);
            fclose(help_fd);
            free(help_buffer);
            sleep(5);
        }

        ret = str_parms_get_str(parms,"dumphalinfo",value,sizeof(value));
        if(ret >= 0){
            LOG_D("dump audio hal info");
            dump_hal_info(adev);
        }

        //echo setpointon=17,5 > dev/pipe/mmi.audio.ctrl
        ret = str_parms_get_str(parms,CP_DUMP_POINT_ENABLE,value,sizeof(value));
        if(ret >= 0){
            LOG_V("enable dump point:%s,pipe:%s",value,adev->cp_nbio_pipe);
            ret = set_point(adev->cp_nbio_pipe,value,1);
        }

        //echo setpointoff=17 > dev/pipe/mmi.audio.ctrl
        ret = str_parms_get_str(parms,CP_DUMP_POINT_DISABLE,value,sizeof(value));
        if(ret >= 0){
            LOG_V("disable dump point:%s,pipe:%s",value,adev->cp_nbio_pipe);
            ret = set_point(adev->cp_nbio_pipe,value,0);
        }

        //echo getpointinfo=1 > dev/pipe/mmi.audio.ctrl
        ret = str_parms_get_str(parms,CP_DUMP_POINT_DISPLAY,value,sizeof(value));
        if(ret >= 0){
            LOG_V("get enabled point:%s.pipe:%s",value,adev->cp_nbio_pipe);
            ret = get_pointinfo(adev->cp_nbio_pipe,1);
        }

        //echo dumpvoice=1,2,17 > dev/pipe/mmi.audio.ctrl
        ret = str_parms_get_str(parms,CP_DUMP_DATA,value,sizeof(value));
        if(ret >= 0){
            ret = dump_voice(value,adev->cp_nbio_pipe);
        }

        //echo setduration=15> dev/pipe/mmi.audio.ctrl
        ret = str_parms_get_str(parms,CP_DUMP_DURATION,value,sizeof(value));
        if(ret >= 0){
            ret = set_duration(adev->cp_nbio_pipe,value);
        }

        //echo setduration=15> dev/pipe/mmi.audio.ctrl
        ret = str_parms_get_str(parms,TESE_AUDIO_IN_DEVICES_TEST,value,sizeof(value));
        ALOGE("peter: ret is %d", ret);
        if(ret >= 0){
            val_int = atoi(value);
            ALOGE("peter: val_int is %d", val_int);
            if(val_int == 1) {
                ret = audio_in_devices_test(adev,parms);
            }
            else if(val_int == 0) {
                ret = audio_in_devices_test_stop(adev,parms);
            }
        }

        ret = str_parms_get_str(parms,TESE_AUDIO_OUT_DEVICES_TEST,value,sizeof(value));
        ALOGE("peter: ret is %d", ret);
        if(ret >= 0){
            val_int = atoi(value);
            ALOGE("peter: val_int is %d", val_int);
            if(val_int == 1) {
                ret = audio_out_devices_test(adev,parms);
            }
            else if(val_int == 0) {
                ret = audio_out_devices_test_stop(adev,parms);
            }
        }
        ret = str_parms_get_str(parms,TEST_HUAWEILAOHUA_AUDIO_OUT_DEVICES_LOOP_TEST,value,sizeof(value));
        ALOGE("peter: ret is %d", ret);
        if(ret >= 0){
            val_int = atoi(value);
            ALOGE("peter: val_int is %d", val_int);
            if(val_int == 1) {
                ret = huaweilaohua_audio_loop_devices_test(adev,parms);
            }
        }



        ret = str_parms_get_str(parms,TESE_AUDIO_OUT_DEVICES_LOOP_TEST,value,sizeof(value));
        ALOGE("peter: ret is %d", ret);
        if(ret >= 0){
            val_int = atoi(value);
            ALOGE("peter: val_int is %d", val_int);
            if(val_int == 1) {
                ret = audio_loop_devices_test(adev,parms);
            }
            else if(val_int == 0) {
                ret = audio_loop_devices_test_stop(adev,parms);
            }
        }

        ret = str_parms_get_str(parms,AUTOTEST_DEVICES_OUT_TEST,value,sizeof(value));
        if(ret >= 0){
            val_int = atoi(value);
            ALOGE("%s: val_int is %d", AUTOTEST_DEVICES_OUT_TEST,val_int);


            #ifdef SUPPORT_OLD_AUDIO_TEST_INTERFACE
            ret = sprd_audiotrack_test(parms,val_int);
            #endif
        }

        ret = str_parms_get_str(parms,AUTOTEST_DEVICES_IN_TEST,value,sizeof(value));
        if(ret >= 0){
            val_int = atoi(value);
            ALOGE("%s: val_int is %d", AUTOTEST_DEVICES_IN_TEST,val_int);
            #ifdef SUPPORT_OLD_AUDIO_TEST_INTERFACE
            ret = sprd_audiorecord_test(parms,val_int);
            #endif
        }

        ret = str_parms_get_str(parms,"skd_test",value,sizeof(value));{
            if(ret >= 0){
                val_int = atoi(value);
                audio_skd_test(adev,parms,val_int);
            }
        }
        ret = str_parms_get_str(parms,AUTOTEST_FM_TEST,value,sizeof(value));
        if(ret >= 0){
            val_int = atoi(value);
            ALOGE("%s: val_int is %d", AUTOTEST_FM_TEST,val_int);
            switch(val_int){
                case 1:
#ifdef AUDIO_TEST_INTERFACE_V2
                    adev_set_parameters(adev,"FM_Volume=0");
#else
#ifdef SUPPORT_OLD_AUDIO_TEST_INTERFACE
                    sprd_fm_test_open();
#endif
#endif
                    break;
                case 2:
#ifdef AUDIO_TEST_INTERFACE_V2
                    adev_set_parameters(adev,"handleFm=1;FM_Volume=11;");
#else
#ifdef SUPPORT_OLD_AUDIO_TEST_INTERFACE
                    sprd_fm_test_play();
#endif
#endif
                    break;
                case 0:
#ifdef AUDIO_TEST_INTERFACE_V2
                    adev_set_parameters(adev,"handleFm=0;");
#else
#ifdef SUPPORT_OLD_AUDIO_TEST_INTERFACE
                    sprd_fm_test_stop();
#endif
#endif
                    break;
                default:
                    break;
            }
        }

        ret = str_parms_get_str(parms,TESE_AUDIO_CP_LOOP_TEST,value,sizeof(value));
        if(ret >= 0){
            int is_loop_enable = 0;
            int us_loop_route = 0;
            val_int = atoi(value);
            is_loop_enable = (val_int & 0xff00)>>8;
            us_loop_route = val_int & 0x00ff;
            LOG_E("%s: val_int is 0x%x, is_loop_enable:%d, us_loop_route is:%d",
                TESE_AUDIO_CP_LOOP_TEST,val_int,is_loop_enable, us_loop_route);
            if(is_loop_enable)
                audio_devices_cp_loop_test(adev,us_loop_route);
            else
                audio_devices_cp_loop_test_stop(adev,us_loop_route);
        }

        ret = str_parms_get_str(parms,GET_DUMP_DATA_SWITCH,value,sizeof(value));
        if(ret >= 0){
            LOG_I("%s: value: %s", GET_DUMP_DATA_SWITCH,value);
            ext_get_dump_data_switch();
        }
        int len = 0;
        ret = str_parms_get_str(parms,SET_DUMP_DATA_SWITCH,value,sizeof(value));
        if(ret >= 0){
            len = strlen(value);
            LOG_I("%s: value: %s len = %d", SET_DUMP_DATA_SWITCH, value, len);
            if (0xa == value[len - 1]) {
                value[len - 1] = '\0';
            }
            ext_property_set("media.dump.switch", value);
            ext_get_dump_data_switch();
        }

        ret = str_parms_get_str(parms,SET_DUMP_DATA_PATH,value,sizeof(value));
        if(ret >= 0){
            len = strlen(value);
            LOG_I("%s: value: %s len = %d", SET_DUMP_DATA_PATH, value, len);
            if (0xa == value[len - 1]) {
                value[len - 1] = '\0';
            }
            adev->dump_flag=strtoul(value,NULL,0);
            ALOGI("update dump_flag:0x%x",adev->dump_flag);
            ext_property_set("media.dump.path", value);
        }

        ret = str_parms_get_str(parms,SET_AUDIO_DUMP,value,sizeof(value));
        if(ret >= 0){
            len = strlen(value);
            if (0xa == value[len - 1]) {
                value[len - 1] = '\0';
            }
            ext_property_set("media.audio.dump", value);
            adev->dump_flag=strtoul(value,NULL,0);
            ALOGI("update dump_flag:0x%x",adev->dump_flag);
        }


#ifdef AUDIO_TEST_INTERFACE_V2
        ret = str_parms_get_str(parms,"endpoint_test", value, sizeof(value));
        ALOGE("peter:endpoint_test ret:%d",ret);
        if(ret >= 0){
            ALOGI("%s line:%d",__func__,__LINE__);
            audio_endpoint_test(adev,parms,value);
            ALOGI("%s line:%d",__func__,__LINE__);
        }
#endif
        ret = str_parms_get_str(parms,"dumpmusic",value,sizeof(value));
        if(ret >= 0){
            set_dump_status(adev,(1<<DUMP_MINIPRIMARY));
        }

        ret = str_parms_get_str(parms,"dumpsco",value,sizeof(value));
        if(ret >= 0){
            set_dump_status(adev,(1<<DUMP_MINIVOIP));
        }

        ret = str_parms_get_str(parms,"dumpbtsco",value,sizeof(value));
        if(ret >= 0){
            set_dump_status(adev,(1<<DUMP_MINI_BT_SCO));
        }

        ret = str_parms_get_str(parms,"dumpvaudio",value,sizeof(value));
        if(ret >= 0){
            set_dump_status(adev,(1<<DUMP_MINIVAUDIO));
        }

        ret = str_parms_get_str(parms,"dumpinread",value,sizeof(value));
        if(ret >= 0){
            set_dump_status(adev,(1<<DUMP_MINI_RECORD_VBC));
        }
        str_parms_destroy(parms);
        memset(value,0x00,sizeof(value));
    }
    free(data);
    close(pipe_fd);
    ALOGI("control_audio_loop_process is exit");
    return NULL;
}
