//
// Spreadtrum Auto Tester
//
// anli   2012-11-09
//
#include <stdlib.h>


//------------------------------------------------------------------------------
// enable or disable local debug
#define DBG_ENABLE_DBGMSG
#define DBG_ENABLE_WRNMSG
#define DBG_ENABLE_ERRMSG
#define DBG_ENABLE_INFMSG
#define DBG_ENABLE_FUNINF
#include "debug.h"
//------------------------------------------------------------------------------
#include "type.h"
#include "audio.h"
#include "battery.h"
#include "bt.h"
#include "camera.h"
#include "cmmb.h"
#include "diag.h"
#include "driver.h"
#include "fm.h"
#include "gps.h"
#include "tp.h"
#include "input.h"
#include "lcd.h"
#include "light.h"
#include "sensor.h"
#include "sim.h"
#include "tcard.h"
#include "tester.h"
#include "util.h"
#include "ver.h"
#include "vibrator.h"
#include "wifi.h"
#include "key_common.h"
#include "ui.h"
#include "otg.h"
#include "sprd_atci.h"
#include "autotest_modules.h"

#include <signal.h>
#include <cutils/properties.h>
#include <hardware_legacy/power.h>
#include <system/audio.h>
#include <unistd.h>

#include <dlfcn.h>
#include<stdlib.h>
#include <signal.h>
#include <stdio.h>
#include "fcntl.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "stdint.h"
#include <fcntl.h>
//------------------------------------------------------------------------------
//add the audio new interface to adapt to the new kernel4.4---------------------
#define AUDIO_EXT_CONTROL_PIPE "/dev/pipe/mmi.audio.ctrl"
#define AUDIO_EXT_DATA_CONTROL_PIPE "/data/local/media/mmi.audio.ctrl"

//the old the adapt to the kernel3.18
#define AUDIO_TEST_FILE "/data/local/media/aploopback.pcm"
#define AUDCTL "/dev/pipe/mmi.audio.ctrl"
//------------------------------------------------------------------------------
// enable or disable local debug
#define DBG_ENABLE_DBGMSG
#define DBG_ENABLE_WRNMSG
#define DBG_ENABLE_ERRMSG
#define DBG_ENABLE_INFMSG
#define DBG_ENABLE_FUNINF
#include "debug.h"
#include "nfc.h"
//------------------------------------------------------------------------------

#include <cutils/properties.h>
#define RTC_PATH "/proc/driver/rtc"

/** Base path of the hal modules */

#if defined(__LP64__)
 #define HAL_LIBRARY_PATH1 "/system/lib64/hw"
 #define HAL_LIBRARY_PATH2 "/vendor/lib64/hw"
 #define LIB_FINGERSOR_PATH "/system/lib64/libfactorylib.so"
#else
 #define HAL_LIBRARY_PATH1 "/system/lib/hw"
 #define HAL_LIBRARY_PATH2 "/vendor/lib/hw"
 #define LIB_FINGERSOR_PATH "/system/lib/libfactorylib.so"
#endif

extern "C"
{
extern void test_result_init(void);
extern void test_lcd_start(void);
extern void test_lcd_splitdisp(int data);
extern void skd_test_lcd_start(void);
extern void test_bl_lcd_start(void);
extern void test_lcd_uinit(void);
}
extern int test_gsensor_start(void);
extern int test_lsensor_start(void);
extern int test_asensor_start(void);
extern int test_msensor_start(void);
extern int test_psensor_start(void);

enum  READ_ITEM_RESULT{
    READ_ITEM_NONE = 0,
    READ_ITEM_ASENSOR = 0x1c,
    READ_ITEM_LSENSOR = 0x20,
    READ_ITEM_GSENSOR = 0x21,
    READ_ITEM_MSENSOR = 0x22,
    READ_ITEM_PSENSOR = 0x23,
    READ_ITEM_PFSENSOR = 0x24,
};

struct list_head test_head;

volatile unsigned char skd_tp_r = RESULT_FAIL;//0: pass 1: fail
volatile unsigned char skd_lcd = 0x02;//0: pass 1: fail
volatile unsigned char skd_backlight = 0x02;//0: pass 1: fail
volatile unsigned char skd_fcamare = 0x02;//0: pass 1: fail
volatile unsigned char skd_bcamare = 0x02;//0: pass 1: fail
volatile unsigned char skd_flash_r = 0x02;//0: pass 1: fail
volatile unsigned char skd_retvalue = 0x02;//0: pass 1: fail

volatile unsigned char skd_vibrator = 0x02;//0: pass 1: fail
volatile unsigned char skd_keybacklight = 0x02;//0: pass 1: fail
unsigned char skd_fm_status_r = 0x02;//0: pass 1: fail
unsigned int skd_fm_freq_r = 0;
int skd_fm_rssi_r = 0;
int bbat_fm_rssi_r = 0;

volatile unsigned char skd_gsensor_result = RESULT_FAIL;
volatile unsigned char skd_lsensor_result = RESULT_FAIL;
volatile unsigned char skd_asensor_result = RESULT_FAIL;
volatile unsigned char skd_msensor_result = RESULT_FAIL;
volatile unsigned char skd_psensor_result = RESULT_FAIL;
volatile unsigned char skd_pfsensor_result = RESULT_FAIL;

int testAudioLoop(int mic, int spk);
int testAudioLoopStop(void);

static const char *variant_keys[] = {
    "ro.hardware",  /* This goes first so that it can pick up a different file on the emulator. */
    "ro.product.board",
    "ro.board.platform",
    "ro.arch"
};

static const int HAL_VARIANT_KEYS_COUNT =
    (sizeof(variant_keys)/sizeof(variant_keys[0]));


//#include<../../device/sprd/common/libs/libcamera/sc8830/inc/SprdCameraHardware_autest_Interface.h>

//for camera auto-test by wangtao 2014-02-14  start
#define MIPI_YUV_YUV 0x0A
#define MIPI_YUV_JPG 0x09
#define MIPI_RAW_RAW 0x0C
#define MIPI_RAW_JPG 0x0D

#define FM_START_FREQ 875

enum auto_test_calibration_cmd_id {
	AUTO_TEST_CALIBRATION_AWB= 0,
	AUTO_TEST_CALIBRATION_LSC,
	AUTO_TEST_CALIBRATION_FLASHLIGHT,
	AUTO_TEST_CALIBRATION_CAP_JPG,
	AUTO_TEST_CALIBRATION_CAP_YUV,
	AUTO_TEST_CALIBRATION_MAX
};
#define	MAX_LEN 200

uint32_t x_value;
uint32_t  y_value;
uint32_t  z_value;
int symbol_x;
int symbol_y;
int symbol_z;
int thread_flag;
int LPsensor;
int m_level;

char path[MAX_LEN]=" ";//"/system/lib/hw/camera.scx15.so";

char func_set_testmode[MAX_LEN]="autotest_set_testmode";
char func_cam_from_buf[MAX_LEN]="autotest_cam_from_buf";
char func_close_testmode[MAX_LEN]="autotest_close_testmode";

FILE *fp = NULL;
int filesize ;



char prefix[MAX_LEN]="/data/image_save.%s";
char test_name[MAX_LEN]="/data/image_save.%s";


int8_t choice;

void *camera_handle;
int ret;

int camera_interface=MIPI_YUV_JPG;
int maincmd=0;
int image_width=640 ;
int image_height=480;

typedef int32_t (*at_set_testmode)(int camerinterface,int maincmd ,int subcmd,int cameraid,int width,int height);
typedef int (*at_cam_from_buf)(void**pp_image_addr,int size,int *out_size);
typedef int (*at_close_testmode)(void);
typedef int (*pf_eng_tst_camera_init)(int camera_id, minui_backend* backend, GRSurface* draw);
typedef int (*pf_eng_tst_camera_deinit)(void);

at_set_testmode atcamera_set_testmode=NULL;
at_cam_from_buf atcamera_get_image_from_buf=NULL;
at_close_testmode atcamera_close_testmode=NULL;
pf_eng_tst_camera_init eng_tst_camera_init=NULL;
pf_eng_tst_camera_deinit eng_tst_camera_deinit=NULL;


int find_cam_lib_path(char*path,int len)
{
	char prop[PATH_MAX];
	char name[PATH_MAX]="camera";
	int i = 0;

	if(path==NULL)
		return 1;

	/* Loop through the configuration variants looking for a module */
	for (i=0 ; i<HAL_VARIANT_KEYS_COUNT+1 ; i++) {
		if (i < HAL_VARIANT_KEYS_COUNT) {
			if (property_get(variant_keys[i], prop, NULL) == 0) {
				continue;
			}
			snprintf(path, len, "%s/%s.%s.so",
					 HAL_LIBRARY_PATH2, name, prop);
			if (access(path, R_OK) == 0) break;

			snprintf(path, len, "%s/%s.%s.so",
					 HAL_LIBRARY_PATH1, name, prop);
			if (access(path, R_OK) == 0) break;
		} else {
			snprintf(path, len, "%s/%s.default.so",
					 HAL_LIBRARY_PATH2, name);
			if (access(path, R_OK) == 0) break;

			snprintf(path, len, "%s/%s.default.so",
					 HAL_LIBRARY_PATH1, name);
			if (access(path, R_OK) == 0) break;
		}
	}

return 0;

}

int loadlibrary_camera_so()
{
find_cam_lib_path(path,sizeof(path));
DBGMSG("path =%s\n",path);

camera_handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
if (!camera_handle){
	char const *err_str = dlerror(); /*err_str is never used*/
	DBGMSG("load:module=%s \n %s",path,err_str?err_str:"unknow");
	return -1;
}

// open auto test mode
/*
atcamera_set_testmode = (at_set_testmode)dlsym(camera_handle, func_set_testmode);
if (atcamera_set_testmode==NULL) {
	DBGMSG("atcamera_set_testmode is null\n");
dlclose(camera_handle);
return -1;
}
*/
atcamera_get_image_from_buf = (at_cam_from_buf)dlsym(camera_handle, "autotest_read_cam_buf");
if(atcamera_get_image_from_buf==NULL)
{
	DBGMSG("atcamera_get_image_from_buf is null\n");
	return -1;
}

// close auto test mode
/*
atcamera_close_testmode = (at_close_testmode)dlsym(camera_handle, func_close_testmode);
if(atcamera_close_testmode==NULL)
{
	DBGMSG("atcamera_close_testmode is null\n");
	return -1;
}
*/
eng_tst_camera_init = (pf_eng_tst_camera_init)dlsym(camera_handle,"autotest_camera_init" );
if(eng_tst_camera_init==NULL)
{
	DBGMSG("eng_tst_camera_init is null\n");
     return -1;
}
else
{
	INFMSG("eng_tst_camera_init.");
}

//deinit camera
eng_tst_camera_deinit = (pf_eng_tst_camera_deinit)dlsym(camera_handle,"autotest_camera_deinit" );
if(eng_tst_camera_deinit==NULL)
{
	DBGMSG("eng_tst_camera_init is null\n");
	return -1;
}else{
	INFMSG("eng_tst_camera_deinit.");
}

return 0;
}

//for camera auto-test by wangtao 2014-02-14 end

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//--namespace sci_tester {
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//------------------------------------------------------------------------------
// enable or disable local debug
#define DBG_ENABLE_DBGMSG
#define DBG_ENABLE_WRNMSG
#define DBG_ENABLE_ERRMSG
#define DBG_ENABLE_INFMSG
#define DBG_ENABLE_FUNINF

//------------------------------------------------------------------------------

#define AT_WAKE_LOCK_NAME  "wl_autotest"

//------------------------------------------------------------------------------
struct diag_header_t {
	uint   sn;
	ushort len;
	uchar  cmd;
	uchar  sub_cmd;
};

#define DIAG_MAX_DATA_SIZE  400 // bytes

//------------------------------------------------------------------------------
/*
using namespace sci_aud;
using namespace sci_bat;
using namespace sci_bt;
using namespace sci_cam;
using namespace sci_diag;
using namespace sci_drv;
using namespace sci_fm;
using namespace sci_input;
using namespace sci_lcd;
using namespace sci_light;
using namespace sci_sim;
using namespace sci_tcard;
using namespace sci_vib;
using namespace sci_wifi;
*/
//------------------------------------------------------------------------------
typedef int  (*PFUN_TEST)(const uchar * data, int data_len, uchar *rsp, int rsp_size);

static int    testRegisterFun( uchar cmd, PFUN_TEST fun );

static int    testReserved(const uchar * data, int data_len, uchar *rsp, int rsp_size);
static int    testKPD(const uchar * data, int data_len, uchar *rsp, int rsp_size);
static int    testLCD(const uchar * data, int data_len, uchar *rsp, int rsp_size);
static int    testCamParal(const uchar * data, int data_len, uchar *rsp, int rsp_size);
static int    testGpio(const uchar * data, int data_len, uchar *rsp, int rsp_size);
static int    testTCard(const uchar * data, int data_len, uchar *rsp, int rsp_size);
static int    testSIM(const uchar * data, int data_len, uchar *rsp, int rsp_size);
static int    testAudioIN(const uchar * data, int data_len, uchar *rsp, int rsp_size);
static int    testAudioOUT(const uchar * data, int data_len, uchar *rsp, int rsp_size);
static int    testLKBV(const uchar * data, int data_len, uchar *rsp, int rsp_size);
static int    testFM(const uchar * data, int data_len, uchar *rsp, int rsp_size);
static int    testBT(const uchar * data, int data_len, uchar *rsp, int rsp_size);
static int    testWIFI(const uchar * data, int data_len, uchar *rsp, int rsp_size);
static int    testIIC(const uchar * data, int data_len, uchar *rsp, int rsp_size);
static int    testCharger(const uchar * data, int data_len, uchar *rsp, int rsp_size);
static int    testSensor(const uchar * data, int data_len, uchar *rsp, int rsp_size);
static int    testGps(const uchar * data, int data_len, uchar *rsp, int rsp_size);
static int    testmipiCamParal(const uchar * data, int data_len, uchar * rsp, int rsp_size);
static void   cam_sd_tcard_addvoltage(void);

static void lcd_test_open(void);
static void skdLcdTest(volatile unsigned char* ret_val);
static void skdBacklightTest(volatile unsigned char* ret_val);
static int    skd_testCamParal(const uchar * data, int data_len, uchar *rsp, int rsp_size);
static int    skd_test_result(const uchar * data, int data_len, uchar *rsp, int rsp_size);
static void   skdCameraTest(int sensor_id,volatile unsigned char* ret_val);
static void   skdCameraFlashTest(volatile unsigned char* ret_val);
static int    testRTC(const uchar * data, int data_len, uchar *rsp, int rsp_size);
//>> modify the headset_state patch as the new plan for k318kernel changed
//#define HEADSET_STATE "/sys/class/switch/h2w/state"----old patch
#define HEADSET_STATE "/sys/kernel/headset/state"

unsigned char headsetPlugState( void )
{
    FUN_ENTER;
    char buf[64] = {0};
    //buf[0] = 0;
    int fd = open(HEADSET_STATE, O_RDONLY);

    if( fd < 0 ) {
        ERRMSG("open '%s' error: %s\n", HEADSET_STATE, strerror(errno));
        return fd;
    }

    read(fd, buf, sizeof buf);
    DBGMSG("headset state = %s\n", buf);
    unsigned char plugState = (unsigned char)atoi(buf);
    close(fd);

    FUN_EXIT;
    return plugState;
}

int SendAudioTestCmd(const uchar * cmd,int bytes)
{
    int fd = -1;
    int ret = -1;
    int bytes_to_read = bytes;
    const uchar *write_ptr=NULL;
	DBGMSG("SendAudioTestCmd cmd = %s\n", cmd);
    if (cmd == NULL) {
	return -1;
    }

    if (fd < 0) {
	fd = open(AUDIO_EXT_CONTROL_PIPE, O_WRONLY | O_NONBLOCK);
	if (fd < 0) {
	    fd = open(AUDIO_EXT_DATA_CONTROL_PIPE, O_WRONLY | O_NONBLOCK);
	}
    }

    if (fd < 0) {
	return -1;
    } else {
	write_ptr=cmd;
	do {
	    ret = write(fd, write_ptr, bytes);
		DBGMSG("SendAudioTestCmd cmd = %s\n", cmd);
	    if (ret > 0) {
		if (ret <= bytes) {
		    bytes -= ret;
		    write_ptr+=ret;
		}
	    } else if ((!((errno == EAGAIN) || (errno == EINTR))) || (0 == ret)) {
		ALOGE("pipe write error %d, bytes read is %d", errno, bytes_to_read - bytes);
		break;
	    } else {
		ALOGD("pipe_write_warning: %d, ret is %d", errno, ret);
	    }
	} while (bytes);
    }

    if (fd > 0) {
	close(fd);
    }

    if (bytes == bytes_to_read)
	return ret;
    else
	return (bytes_to_read - bytes);
}
#if 0
int SendAudioTestCmd(const uchar * cmd,int bytes){
    int fd = -1;
    int ret=-1;
    int bytes_to_read = bytes;

    if(cmd==NULL){
        return -1;
    }

    if(fd < 0) {
        fd = open(AUDCTL, O_WRONLY | O_NONBLOCK);
    }

    if(fd < 0) {
        return -1;
    }else{
        do {
            ret = write(fd, cmd, bytes);
            if( ret > 0) {
                if(ret <= bytes) {
                    bytes -= ret;
                }
            }
            else if((!((errno == EAGAIN) || (errno == EINTR))) || (0 == ret)) {
                ALOGE("pipe write error %d,bytes read is %d",errno,bytes_to_read - bytes);
                break;
            }
            else {
                ALOGW("pipe_write_warning: %d,ret is %d",errno,ret);
            }
        }while(bytes);
    }

    if(fd > 0) {
        close(fd);
    }

    if(bytes == bytes_to_read)
        return ret ;
    else
        return (bytes_to_read - bytes);
}
#endif
//------------------------------------------------------------------------------
static PFUN_TEST       sTestFuns[DIAG_CMD_MAX];
static int sensor_id = 0;//0:back ,1:front

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void signalProcess( int signo )
{
	WRNMSG("On signal: %d\n", signo);

	test_Deinit();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int test_Init( void )
{
	acquire_wake_lock(PARTIAL_WAKE_LOCK, AT_WAKE_LOCK_NAME);

    FUN_ENTER;

	char ver[256];
	snprintf(ver, 256, "Autotest Version: %s %s", verGetDescripter(), verGetVer());
	DBGMSG("%s\n", ver);

	// disable java framework
	utilEnableService("media");
	utilDisableService("bootanim");
	utilDisableService("netd");
	utilDisableService("surfaceflinger");
	usleep(100 * 1000);
	if( utilDisableService("zygote") >= 0 ) {
		int sys_svc_pid = utilGetPidByName("system_server");
		DBGMSG("system_service pid = %d\n", sys_svc_pid);
		if( sys_svc_pid > 0 ) {
			kill(sys_svc_pid, SIGTERM);
		}
	}
	usleep(200 * 1000);
	utilDisableService("netd");
	utilDisableService("surfaceflinger");
	utilDisableService("zygote");
	usleep(200 * 1000);

	// for debug
	//usleep(1000 * 1000);
	//system("ps > /data/atps.log");
	//--------------------------------------------------------------------------
	//
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_flags   = SA_RESETHAND | SA_NODEFER;
	sa.sa_handler = signalProcess;
	sigaction(SIGHUP,  &sa, NULL);
	sigaction(SIGABRT, &sa, NULL);

	//--------------------------------------------------------------------------
	//
    memset(sTestFuns, 0, sizeof(sTestFuns));

    testRegisterFun(DIAG_CMD_RESERVED,  testReserved);
    testRegisterFun(DIAG_CMD_KPD,       testKPD);
    testRegisterFun(DIAG_CMD_LCD,       testLCD);
    testRegisterFun(DIAG_CMD_CAM_PARAL, testmipiCamParal);//testCamParal
    testRegisterFun(DIAG_CMD_CAM_MIPI, testmipiCamParal);
    testRegisterFun(DIAG_CMD_GPIO,      testGpio);
    testRegisterFun(DIAG_CMD_TCARD,     testTCard);
    testRegisterFun(DIAG_CMD_SIM,       testSIM);
    testRegisterFun(DIAG_CMD_AUDIO_IN,  testAudioIN);
    testRegisterFun(DIAG_CMD_AUDIO_OUT, testAudioOUT);
    testRegisterFun(DIAG_CMD_LKBV,      testLKBV);
    testRegisterFun(DIAG_CMD_FM,        testFM);
    testRegisterFun(DIAG_CMD_BT,        testBT);
    testRegisterFun(DIAG_CMD_WIFI,      testWIFI);
    testRegisterFun(DIAG_CMD_IIC,       testIIC);
    testRegisterFun(DIAG_CMD_CHARGE,    testCharger);
    testRegisterFun(DIAG_CMD_SENSOR,    testSensor);
    testRegisterFun(DIAG_CMD_GPS,       testGps);
    testRegisterFun(DIAG_CMD_LCD_SPI, skd_test_result);
	testRegisterFun(DIAG_CMD_RTC, testRTC);
    drvOpen();

    key_init();

    autotest_modules_load(&test_head);

    FUN_EXIT;
    return 0;
}

//------------------------------------------------------------------------------

int test_DoTest( const uchar * req, int req_len, uchar * rsp, int rsp_size )
{
    FUN_ENTER;
    struct diag_header_t * dh = (struct diag_header_t *)req;
	DBGMSG("test_DoTest dh->len=%d,req_len=%d\n",dh->len,req_len);
	AT_ASSERT( dh->len <= req_len );
	AT_ASSERT( 0x38 == dh->cmd );

	if( dh->sub_cmd >= DIAG_CMD_MAX ) {
		ERRMSG("invalid sub cmd: 0x%x\n", dh->sub_cmd);
		return -1;
	}
	DBGMSG("test_DoTest cmd is %d,su_cmd=%d",dh->cmd,dh->sub_cmd);
	int ret = -1;
	if( sTestFuns[dh->sub_cmd] != NULL ) {
		int dpos = sizeof(struct diag_header_t);
		const uchar * data = (const uchar *)(req + dpos);
		ret = sTestFuns[dh->sub_cmd](data, req_len - dpos, rsp, rsp_size);
	}

    FUN_EXIT;
    return ret;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

int testRegisterFun( uchar cmd, PFUN_TEST fun )
{
    AT_ASSERT( cmd < DIAG_CMD_MAX );

    if( cmd >= DIAG_CMD_MAX ) {
        ERRMSG("Invalid: cmd = %d, max = %d\n", cmd, DIAG_CMD_MAX);
        return -1;
    }

    if( sTestFuns[cmd] != NULL ) {
        WRNMSG("cmd(%d) already register!\n", cmd);
    }

    sTestFuns[cmd] = fun;

    return 0;
}

int testReserved(const uchar * data, int data_len, uchar * rsp, int rsp_size)
{
	FUN_ENTER;
    INFMSG("data[0] = %d\n", *data);

	int ret = 0;
    switch( *data ) {
    case 0: // version
        snprintf((char *)rsp, rsp_size, "Autotest Version: %s %s", verGetDescripter(), verGetVer());
		sensorOpen();
		ret = strlen((char *)rsp);
        break;
    case 1: //
		dbgMsg2FileEnable(1);
        break;
    case 2: //
		dbgMsg2FileEnable(0);
        break;
    default:
        break;
    }

    FUN_EXIT;
    return ret;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int testKPD(const uchar * data, int data_len, uchar * rsp, int rsp_size)
{
    FUN_ENTER;
    INFMSG("data[0] = %d\n", *data);

	int ret = 0;

    switch( *data ) {
    case 1: // init
        if( inputOpen() < 0 ) {
            ret  = -1;
        }
        break;
    case 2: // get pressed key
    {
		struct kpd_info_t kpdinfo;

		ret = 6;
        if( inputKPDGetKeyInfo(&kpdinfo) < 0 ) {
            rsp[0]  = 0xFF; // col
			rsp[1]  = 0xFF; // row
			rsp[2]  = 0x00; // key high byte
			rsp[3]  = 0x00; // key low byte
			rsp[4]  = 0x00; // gpio high byte
			rsp[5]  = 0x00; // gpio low byte
        } else {
            rsp[0] = ((uchar)(kpdinfo.col)) & 0xFF;
			rsp[1] = ((uchar)(kpdinfo.row)) & 0xFF;
			rsp[2] = ((uchar)(kpdinfo.key >> 8));   // key high byte
			rsp[3] = ((uchar)(kpdinfo.key & 0xFF)); // key low byte
			rsp[4] = ((uchar)(kpdinfo.gio >> 8));   // gpio high byte
			rsp[5] = ((uchar)(kpdinfo.gio & 0xFF)); // gpio low byte
        }
    }
        break;
    case 3: // end test
        inputClose();
        break;
    default:
        break;
    }

    FUN_EXIT;
    return ret;
}
static int    testRTC(const uchar * data, int data_len, uchar *rsp, int rsp_size)
{
	FUN_ENTER;
	uint8_t buf[100];
	int bytes,i,ret,command;
	int min_time=0;
	int sec_time=0;
	int hour_flag,min_flag;
	int new_min_time=0;
	int new_sec_time=0;
	hour_flag=0;
	min_flag=0;
	command=data[0];
	LOGD("data[0] = %d\n", *data);
	int fd = open(RTC_PATH, O_RDONLY);
	memset(buf,0,sizeof(buf));
	switch(command)
	{
		case 1:
			lseek(fd, 0, SEEK_SET);
			bytes=read(fd,buf,sizeof(buf));
			LOGD("time:%s\r\n",buf);
			if(bytes<0)
			{
				LOGD("read failed\r\n");
				ret=-1;
				break;
			}
			else 
				ret=0;
			break;
		case 3:
			ret=1;
			bytes=read(fd,buf,sizeof(buf));
			LOGD("time:%s\r\n",buf);
			if(bytes<0)
				break;
			else
			{
				for(i=0;i<bytes;i++)
				{
					 if((buf[i]==':')&&(buf[i+1]!=' ')&&(min_flag==0))
					{
						min_time=(buf[i+1]-48)*10+(buf[i+2]-48);
						min_flag=1;
						LOGD("min_time:%d\n",min_time);
					}
					else if((buf[i]==':')&&(min_flag==1))
					{
						sec_time=(buf[i+1]-48)*10+(buf[i+2]-48);
						min_flag=0;
						LOGD("sec_time:%d\n",sec_time);
						break;
					}
				}
			}
			memset(buf,0,sizeof(buf));
			sleep(3);
			lseek(fd, 0, SEEK_SET);
			bytes=read(fd,buf,sizeof(buf));
			LOGD("time:%s\r\n",buf);
			if(bytes<0)
			{
				LOGD("read failed\r\n");
				ret=-1;
				break;
			}
			else
			{
				for(i=0;i<bytes;i++)
				{
					 if((buf[i]==':')&&(buf[i+1]!=' ')&&(min_flag==0))
					{
						new_min_time=(buf[i+1]-48)*10+(buf[i+2]-48);
						min_flag=1;
						LOGD("new_min_time:%d\n",new_min_time);
					}
					else if((buf[i]==':')&&(min_flag==1))
					{
						new_sec_time=(buf[i+1]-48)*10+(buf[i+2]-48);
						LOGD("new_sec_time:%d\n",new_sec_time);
						min_flag=0;
						break;
					}
				}
			}
			if(new_sec_time==(sec_time+3))
				{
				rsp[0]=0;
				break;
				}
			else if(sec_time>=57)
				{
				if(new_sec_time==((sec_time+3)%60))
				rsp[0]=0;
				break;
				}
			else
				rsp[0]=1;
			LOGD("rsp[0]:%d\n",rsp[0]);
		break;
		case 4:
			lseek(fd, 0, SEEK_SET);
			bytes=read(fd,buf,sizeof(buf));
			LOGD("time:%s\r\n",buf);
			if(bytes<0)
				ret=-1;
			else 
				ret=0;
			break;
		default:
			break;	

	}
	close(fd);
	FUN_EXIT;

	return ret;

}


int skd_test_result(const uchar * data, int data_len, uchar * rsp, int rsp_size)
{
	FUN_ENTER;
	INFMSG("data[0] = 0x%x\n", *data);
	int ret = 1;

	switch(data[0])
	{
	case 7:
		INFMSG("case 7 rsp no stick to 0x%0x !\n", skd_fcamare);
		rsp[0]= skd_fcamare;
		break;
	case 8:
		INFMSG("case 8 rsp no stick to 0x%0x !\n", skd_bcamare);
		rsp[0]= skd_bcamare;
		break;
	case 9:
		INFMSG("case 9 rsp no stick to 0x%0x !\n", skd_flash_r);
		rsp[0]= skd_flash_r;
		break;

	case READ_ITEM_ASENSOR:
		rsp[0]= skd_asensor_result;
		break;

	case READ_ITEM_LSENSOR:
		rsp[0]= skd_lsensor_result;
		break;
	case READ_ITEM_GSENSOR:
		rsp[0]= skd_gsensor_result;
		break;
	case READ_ITEM_MSENSOR:
		rsp[0]= skd_msensor_result;
		break;
	case READ_ITEM_PSENSOR:
		rsp[0]= skd_psensor_result;
		break;
	case READ_ITEM_PFSENSOR:
		rsp[0]= skd_pfsensor_result;
		break;

	case 5:
		rsp[0]= skd_lcd;
		break;
	case 6:
		rsp[0]= skd_backlight;
		break;
	case 0x0b: //get tp test result
		rsp[0]= skd_tp_r;
		break;
	case 0x15:
	{
		if (skd_fm_status_r == 0) {
			ret = 7;
			rsp[0]= skd_fm_status_r;
			rsp[1]= skd_fm_freq_r>>8;
			rsp[2]= skd_fm_freq_r>>0;
			rsp[3]= skd_fm_rssi_r>>24;
			rsp[4]= skd_fm_rssi_r>>16;
			rsp[5]= skd_fm_rssi_r>>8;
			rsp[6]= skd_fm_rssi_r>>0;
		} else {
			ret = 1;
			rsp[0]= skd_fm_status_r;
		}
	}
		break;
	default:
		INFMSG("data[0] = 0x%x no stick to!\n", *data);
		break;
	}

	return ret;

}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int testLCD(const uchar * data, int data_len, uchar * rsp, int rsp_size)
{
    FUN_ENTER;
    INFMSG("data[0] = %d,\n", *data);

	int ret = 0;

	if( 0x11 == *data ) {
	    lcd_test_open();
	} else if( 0x12 == *data ) {
	    test_lcd_uinit();
	} else if( 0x20 == *data ) {
	    skdLcdTest(&skd_lcd);
	}else if( 0x21 == *data ) {
	    INFMSG("data[1] = %d", data[1]);
	    test_result_init();
	    test_lcd_splitdisp(data[1]);
	} else {
	    ret = -1;
	}

    FUN_EXIT;
       return ret;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int testCamParal(const uchar * data, int data_len, uchar * rsp, int rsp_size)
{
    int ret = 0;

    #define CAM_IDX       1
    #define CAM_W         240 //(320 * 2)
    #define CAM_H         320 //(240 * 2)
    #define CAM_PIX_SIZE  2
    #define CAM_DAT_SIZE  (240 * CAM_PIX_SIZE)

    FUN_ENTER;
    INFMSG("data[0] = %d\n", *data);

    switch( data[0] ) {
    case 1:
        if( camOpen(CAM_IDX, CAM_W, CAM_H) < 0 ) {
            ret = -1;
        }
        break;
    case 2:
        ret = CAM_DAT_SIZE;
        if( camStart() < 0 || camGetData((uchar *)rsp, ret) < 0 ) {
            ret = -1;
        }
        break;
    case 3:
        camStop();
		camClose();
        break;
    default:
        break;
    }

    FUN_EXIT;
    return ret;
}

int skd_testCamParal(const uchar * data, int data_len, uchar * rsp, int rsp_size)
{
	int ret = 1;
	FUN_ENTER;
	INFMSG("testCamParal  data[0] = %d, sensor_id=%d", *data, rsp_size, sensor_id);

	loadlibrary_camera_so();

	switch( data[0] ) {
		case 0:
		INFMSG("testCamParal test camera 0");
		skdCameraTest(0, &skd_bcamare);
		break;
	case 1:
		INFMSG("testCamParal test camera 1");
		skdCameraTest(1 ,&skd_fcamare);
		break;
	case 2:
		break;
	case 3:
		eng_tst_camera_deinit();//deinit back camera and start preview
		INFMSG("testCamParal close.........");
		inputClose();
		break;
	default:
		break;
	}

	FUN_EXIT;
	return ret;
}

void skdCameraTest(int sensor_id,volatile unsigned char* ret_val){

    uchar buttonPressed=0x00;// 0: init state   1: power   2: vol down   3: vol up
    uchar btn_save=0x00;
    bool inputRunning = true;
    if(eng_tst_camera_init(sensor_id,gr_backend_get(),gr_draw_get()))   //init  camera and start preview
    {
	INFMSG("testCamParal %s fail to call eng_test_camera_init ", __FUNCTION__);
    }
	INFMSG("testCamParal open.........");

    if(inputOpen()<0)
    {
        INFMSG("skd_testCamParal, input fail !!\n");
    }
	struct kpd_info_t kpdinfo;
	while(inputRunning){
		if( inputKPDWaitKeyPress(&kpdinfo, 6) < 0) {
			INFMSG("testCamParal kpdinfo.key read fail");
			*ret_val = 0x03;
			inputRunning = false;

		}else{
			btn_save = ((uchar)(kpdinfo.key & 0xFF));
			if( btn_save==0x72 ){
				INFMSG("testCamParal pressed vol down pass");
				*ret_val = 0x00;
				inputRunning = false;
			}else if( btn_save==0x74 ){
				INFMSG("testCamParal pressed power  fail");
				*ret_val = 0x01;
				inputRunning = false;
			}else if( btn_save==0x73 ){
			    INFMSG("testCamParal pressed vol up test again");
			    *ret_val = 0x02;
			    inputRunning = false;
			}

		}
	}

	eng_tst_camera_deinit();//deinit camera and start preview
	INFMSG("testCamParal close.........");
	inputClose();
}

void skdCameraFlashTest(volatile unsigned char* ret_val){

    uchar btn_save=0x00;
    bool inputRunning = true;
    if(inputOpen()<0)
    {
        INFMSG("skd Flash, input fail !!\n");
    }
	struct kpd_info_t kpdinfo;
	flashlightSetValue(48); //open flash
	while(inputRunning){
		if( inputKPDWaitKeyPress(&kpdinfo, 5*1000) < 0) {
			INFMSG("skd Flash kpdinfo.key read fail");
			continue;
		}else{
			btn_save = ((uchar)(kpdinfo.key & 0xFF));
			if( btn_save==0x72 ){
				INFMSG("skd Flash pressed vol down pass");
				*ret_val = 0x00;
				inputRunning = false;
			}else if( btn_save==0x74 ){
				INFMSG("skd Flash pressed power  fail");
				*ret_val = 0x01;
				inputRunning = false;
			}else if( btn_save==0x73 ){
			    INFMSG("skd Flash pressed vol up test again");
			    *ret_val = 0x02;
			    inputRunning = false;
			}

		}
	}
	flashlightSetValue(49);
	inputClose();
}


int testmipiCamParal(const uchar * data, int data_len, uchar * rsp, int rsp_size)
{
   int ret = 0;
    int rec_image_size=0;
    #define CAM_IDX       1
    #define CAM_W_VGA         640
    #define CAM_H_VGA          480
    #define CAM_PIX_SIZE  2
    #define CAM_DAT_SIZE_VGA  (CAM_W_VGA * CAM_H_VGA * CAM_PIX_SIZE)
    #define SBUFFER_SIZE  (600*1024)
    uchar * sBuffer = NULL;
    sBuffer = new uchar[600*1024];
    static int sensor_id = 0;//0:back  ,1:front

    FUN_ENTER;
    INFMSG("testmipiCamParal  data[0] = %d, data[1] = %d,  rsp_size =%d, sensor_id=%d \n", *data, data[1], rsp_size,sensor_id);

    switch( data[0] ) {
    case 1:
	  loadlibrary_camera_so();
	  if( eng_tst_camera_init(sensor_id,NULL,NULL) < 0 ) {
	  ret = -1;
        }
        break;
    case 2:
        if(atcamera_get_image_from_buf((void**)&sBuffer, SBUFFER_SIZE, &rec_image_size)< 0 ) {
            ret = -1;
        }else{
	     if(rec_image_size > rsp_size -1){
		 	memcpy(rsp, sBuffer, 768/*(rsp_size - 1)*/);
		       ret = 768;//rsp_size-1;
	     	}else{
		 	memcpy(rsp, sBuffer, 768/*rec_image_size*/);
		 	ret =  768;//rec_image_size;
	     	}
	 }
	  INFMSG("testmipiCamParal  rec_image_size = %d\n", rec_image_size);
        break;
    case 3:
      if(eng_tst_camera_deinit() < 0 ) {
            ret = -1;
        }
	    sensor_id = 0;
        break;
    case 4:
        sensor_id = data[1];
        break;
	case 0x20:   //for skd camera test
		data = data +1;
		ret = skd_testCamParal(data, data_len,rsp, rsp_size);
		break;
    default:
        break;
    }
	delete [] sBuffer;

    FUN_EXIT;
    return ret;
}

//----------------------------------------------------------------------------
void   cam_sd_tcard_addvoltage(void)
{
    system("echo 1 > /sys/kernel/debug/sprd-regulator/vddcamio/enable");
    usleep(100*1000);
    system("echo 1 > /sys/kernel/debug/sprd-regulator/vddsdcore/enable");
    usleep(100*1000);
    system("echo 1 > /sys/kernel/debug/sprd-regulator/vddsdio/enable");
    usleep(100*1000);
    system("echo 1 > /sys/kernel/debug/sprd-regulator/vddsd/enable");
    usleep(100*1000);
    system("echo 1 > /sys/kernel/debug/sprd-regulator/vddsim0/enable");
    usleep(100*1000);
    system("echo 1 > /sys/kernel/debug/sprd-regulator/vddsim1/enable");
    usleep(100*1000);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int testGpio(const uchar * data, int data_len, uchar * rsp, int rsp_size)
{
    int ret = 0;

    FUN_ENTER;
    INFMSG("data[0] = %d\n", *data);
    //as not the gpio drv
   autotest_modules *modules_list = NULL;
    struct list_head *list_find;

    list_for_each(list_find,&test_head){

        modules_list = list_entry(list_find, autotest_modules, node);
        INFMSG("engpc_callback cmd %x\n",modules_list->callback.diag_ap_cmd);
        if(modules_list->callback.diag_ap_cmd == DIAG_CMD_GPIO){
            ret = modules_list->callback.autotest_func(data,data_len,rsp,rsp_size);
        }

    }
    FUN_EXIT;
    return ret;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int testTCard(const uchar * data, int data_len, uchar * rsp, int rsp_size)
{
    int ret = -1;
    char mount_path[256]= {0};
    FUN_ENTER;
    INFMSG("data[0] = %d\n", *data);
    if(tcardIsMount(mount_path)){
	ret = 0;
    }
#if 0
    if( tcardOpen() >= 0 && tcardIsPresent() >= 0 ) {
	ret = 0;
    }
    tcardClose();
#endif
    FUN_EXIT;
    return ret;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int testSIM(const uchar * data, int data_len, uchar * rsp, int rsp_size)
{
    int ret = -1;
	int respLen = 0;
	char cmd[128];
	char resp[128];
    FUN_ENTER;
    INFMSG("data[0] = %d\n", *data);
	memset(resp, 0, sizeof(resp));
	snprintf(cmd, sizeof(cmd),"AT+EUICC?");
	ret = sendATCmd(*data, cmd, resp, sizeof(resp));
	if(ret==-1)
	{
	INFMSG("send the command fail\n");
	}
	else if (ret==0)
	{ 
		RLOGD("resp = %s", resp);
		char * pval = strstr(resp, "EUICC");
		DBGMSG("read OK: %s\n", resp);
		if( NULL != pval ) {
		DBGMSG("pval=%s\n", pval);
		pval += 5;
                while( *pval && (*pval < '0' || *pval > '9') ) {
                    pval++;
                }
		DBGMSG("%s\n", pval);
		ret = ('0' == *pval) ? 0 : -1;
		}
		else{
		ret = -1;
		}
	}

	DBGMSG("simCheck: ret = %d\n", ret);
    FUN_EXIT;
    return ret;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int testAudioIN(const uchar *data, int data_len, uchar *rsp, int rsp_size)
{
    static int data_fd = -1;
    static int test_audioin_result=2;//need re-test
    int   ret = 0;
    uchar mic = data[0];
    uchar act = data[1];
    char write_buf[256] = {0};
    int devices = 0;
    FUN_ENTER;
    ALOGE("wangzuo:data[0] = %d %d  data_len:%d:\n", mic, act,data_len);

#define RECORD_SAMPLE_RATE  16000
#define RECORD_MAX_SIZE    1600
    static uchar rcrdBuf[RECORD_MAX_SIZE];

    if(data_len==4){
        uchar loop_act = data[0];
        uchar loop_mic = data[1];
        uchar loop_out = data[2];
        int row = 3;
        int key = -1;
        ALOGE("testAudioIN:len:%d loop_act: %02x %02x %02x \n",data_len, loop_act,data[1],data[2]);
        if(loop_act==0x20){
            int key = -1;
            int input_devices=0;
            int output_devices=0;
            switch(loop_mic){
                case AUD_INDEV_BUILTIN_MIC:
                    input_devices=AUDIO_DEVICE_IN_BUILTIN_MIC;
                    break;
                case AUD_INDEV_HEADSET_MIC:
                    input_devices=AUDIO_DEVICE_IN_WIRED_HEADSET;
                    break;
                case AUD_INDEV_BACK_MIC:
                    input_devices=AUDIO_DEVICE_IN_BACK_MIC;
                    break;
                default:
                    break;
            }

            switch(loop_out){
                case AUD_OUTDEV_SPEAKER:
                    output_devices=AUDIO_DEVICE_OUT_SPEAKER;
                    break;
                case AUD_OUTDEV_EARPIECE:
                    output_devices=AUDIO_DEVICE_OUT_EARPIECE;
                    break;
                case AUD_OUTDEV_HEADSET:
                    output_devices=AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
                    break;
                default:
                    break;
            }

            snprintf(write_buf,
                sizeof(write_buf) - 1,"audio_loop_test=1;test_in_stream_route=0x%x;test_stream_route=%d;samplerate=%d;channels=%d",
                input_devices,output_devices,48000,2);

            ALOGD("write:%s", write_buf);
            SendAudioTestCmd((const uchar *)write_buf,strlen(write_buf));
            usleep(400*1000);

            ui_clear_key_queue();
            key = ui_wait_key_simp();
            INFMSG("testAudioIN test, key = %d\n", key);

            if(key == 114)
            {
                test_audioin_result=0;//pass
            } else if (key == 116)
            {
                test_audioin_result=1;//failed
            } else if (key == 115) {
                test_audioin_result=2;//re-test
            }
            rsp[0]=test_audioin_result;
            ret = 1;
            snprintf(write_buf, sizeof(write_buf) - 1, "audio_loop_test=0");
            ALOGD("write:%s", write_buf);
            SendAudioTestCmd((const uchar *)write_buf,strlen(write_buf));
            return 0;
        }else if(loop_act==0x10){
            testAudioLoop(data[1],data[2]);
            return 0;
        }
        }else if(data_len==2){
            uchar loop_act = data[0];
            if(loop_act=0x11){
                testAudioLoopStop();
                return 0;
        }
    }

    switch( act ){
    case 0x01:
        snprintf(write_buf,sizeof(write_buf) -1,"autotest_audiorecordtest=1;autotest_indevices=%d",mic);
        ALOGD("write:%s", write_buf);
        SendAudioTestCmd((const uchar *)write_buf,strlen(write_buf));
        break;
    case 0x02:
        ret = 0;
        if(data_fd < 0) {
            ALOGD("testAudioIN start open");
            //data_fd = open(AUDIO_EXT_DATA_CONTROL_PIPE, O_RDONLY|O_NONBLOCK);
		data_fd = open(AUDIO_TEST_FILE, O_RDONLY|O_NONBLOCK);

        }
        if( data_fd < 0 ) {
            ret = -1;
            ALOGD("testAudioIN open failed");
            break;
        }
        break;
    case 0x03: {
        uchar idx = data[2];
        ALOGE("index = %d\n", idx);
        if(data_fd < 0) {
            ALOGD("testAudioIN begin open");
            //data_fd = open(AUDIO_EXT_DATA_CONTROL_PIPE, O_RDONLY|O_NONBLOCK);
		data_fd = open(AUDIO_TEST_FILE, O_RDONLY|O_NONBLOCK);

            ALOGD("testAudioIN begin end");
        }
        if( data_fd < 0 ) {
            ALOGD("testAudioIN open failed");
            ret = -1;
            break;
        }
        if( 0 == idx ) {
            int to_read = RECORD_MAX_SIZE;
            uchar *buf = (uchar *)rcrdBuf;
            int num_read = 0;
            snprintf(write_buf,sizeof(write_buf)-1,"autotest_audiorecordtest=2;autotest_datasize=%d",to_read);
            ALOGD("write:%s", write_buf);
            SendAudioTestCmd((const uchar *)write_buf,strlen(write_buf));
            usleep(10000);
            if(data_fd < 0) {
                ALOGD("testAudioIN start open");
                //data_fd = open(AUDIO_EXT_DATA_CONTROL_PIPE, O_RDONLY|O_NONBLOCK);
		data_fd = open(AUDIO_TEST_FILE, O_RDONLY|O_NONBLOCK);
                ALOGD("testAudioIN end open");
            }
            if( data_fd < 0 ) {
                //ALOGE("testAudioIN open:%s failed",AUDIO_EXT_DATA_CONTROL_PIPE);
		ALOGE("testAudioIN open:%s failed",AUDIO_TEST_FILE);
                ret = -1;
                break;
            }
            while(to_read) {
                num_read = read(data_fd, (uchar *)buf, to_read);
                if(num_read <= 0) {
                    usleep(10000);
                    continue;
                }
                if(num_read < to_read) {
                    usleep(10000);
                }
                to_read -= num_read;
                buf += num_read;
                ALOGD("testAudioIN 3 num_read:0x%x err:%s", num_read,strerror(errno));
            }
        }
        ret = RECORD_MAX_SIZE - (idx * DIAG_MAX_DATA_SIZE);
        ALOGE("ret = %d\n", ret);

        if( ret > DIAG_MAX_DATA_SIZE ) {
            ret = DIAG_MAX_DATA_SIZE;
        } else if( ret <= 0 ) {
            ret = 0;
            break;
        }
        memcpy(rsp, rcrdBuf + idx * DIAG_MAX_DATA_SIZE, ret);
    }
    break;
    case 0x04:
        snprintf(write_buf, sizeof(write_buf) - 1, "autotest_audiorecordtest=0");
        ALOGD("write:%s", write_buf);
        SendAudioTestCmd((const uchar *)write_buf,strlen(write_buf));
        close(data_fd);
        data_fd = -1;
        break;
    case 0x05:
    // for test
    /*    {
                #define RCRD_SIZE (160 * 1024)
                FILE  * fp;
                uchar * pcm = (uchar *)malloc(RCRD_SIZE);
                int size = RCRD_SIZE;

                if( audRcrderRecord(pcm, size) < 0 ) {
                    ret = -1;
                }

                fp = fopen("/mnt/sdcard/rcrd.pcm", "w");
                if( fp ) {
                    size = fwrite(pcm, 1, size, fp);
                    fclose(fp);
                }
                free(pcm);
        }
    */
    // no break
    default:
        ret = -1;
        break;
    }

    FUN_EXIT;
    return ret;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

static const uchar s_pcmdata_mono[] = {
    0x92,0x02,0xcb,0x0b,0xd0,0x14,0x1d,0x1d,0xfc,0x24,0x17,0x2c,0x4a,0x32,0x69,0x37,
    0x92,0x3b,0x4e,0x3e,0x22,0x40,0x56,0x40,0x92,0x3f,0x12,0x3d,0x88,0x39,0x10,0x35,
    0xf0,0x2e,0x51,0x28,0xce,0x20,0x7f,0x18,0xd5,0x0f,0xda,0x06,0xdf,0xfd,0xa4,0xf4,
    0xa2,0xeb,0x39,0xe3,0x57,0xdb,0x3d,0xd4,0x1f,0xce,0xe2,0xc8,0xb1,0xc4,0xc0,0xc1,
    0xec,0xbf,0xc1,0xbf,0xa4,0xc0,0xf2,0xc2,0x18,0xc6,0xc2,0xca,0xc8,0xd0,0x36,0xd7,
    0xbb,0xde,0xe6,0xe6,0xa5,0xef,0xa6,0xf8,
};

static const uchar s_pcmdata_left[] = {
	0x92,0x02,0x00,0x00,0xCB,0x0B,0x00,0x00,0xD0,0x14,0x00,0x00,0x1D,0x1D,0x00,0x00,
	0xFC,0x24,0x00,0x00,0x17,0x2C,0x00,0x00,0x4A,0x32,0x00,0x00,0x69,0x37,0x00,0x00,
	0x92,0x3B,0x00,0x00,0x4E,0x3E,0x00,0x00,0x22,0x40,0x00,0x00,0x56,0x40,0x00,0x00,
	0x92,0x3F,0x00,0x00,0x12,0x3D,0x00,0x00,0x88,0x39,0x00,0x00,0x10,0x35,0x00,0x00,
	0xF0,0x2E,0x00,0x00,0x51,0x28,0x00,0x00,0xCE,0x20,0x00,0x00,0x7F,0x18,0x00,0x00,
	0xD5,0x0F,0x00,0x00,0xDA,0x06,0x00,0x00,0xDF,0xFD,0x00,0x00,0xA4,0xF4,0x00,0x00,
	0xA2,0xEB,0x00,0x00,0x39,0xE3,0x00,0x00,0x57,0xDB,0x00,0x00,0x3D,0xD4,0x00,0x00,
	0x1F,0xCE,0x00,0x00,0xE2,0xC8,0x00,0x00,0xB1,0xC4,0x00,0x00,0xC0,0xC1,0x00,0x00,
	0xEC,0xBF,0x00,0x00,0xC1,0xBF,0x00,0x00,0xA4,0xC0,0x00,0x00,0xF2,0xC2,0x00,0x00,
	0x18,0xC6,0x00,0x00,0xC2,0xCA,0x00,0x00,0xC8,0xD0,0x00,0x00,0x36,0xD7,0x00,0x00,
	0xBB,0xDE,0x00,0x00,0xE6,0xE6,0x00,0x00,0xA5,0xEF,0x00,0x00,0xA6,0xF8,0x00,0x00,
};

static const uchar s_pcmdata_right[] = {
	0x00,0x00,0x92,0x02,0x00,0x00,0xCB,0x0B,0x00,0x00,0xD0,0x14,0x00,0x00,0x1D,0x1D,
	0x00,0x00,0xFC,0x24,0x00,0x00,0x17,0x2C,0x00,0x00,0x4A,0x32,0x00,0x00,0x69,0x37,
	0x00,0x00,0x92,0x3B,0x00,0x00,0x4E,0x3E,0x00,0x00,0x22,0x40,0x00,0x00,0x56,0x40,
	0x00,0x00,0x92,0x3F,0x00,0x00,0x12,0x3D,0x00,0x00,0x88,0x39,0x00,0x00,0x10,0x35,
	0x00,0x00,0xF0,0x2E,0x00,0x00,0x51,0x28,0x00,0x00,0xCE,0x20,0x00,0x00,0x7F,0x18,
	0x00,0x00,0xD5,0x0F,0x00,0x00,0xDA,0x06,0x00,0x00,0xDF,0xFD,0x00,0x00,0xA4,0xF4,
	0x00,0x00,0xA2,0xEB,0x00,0x00,0x39,0xE3,0x00,0x00,0x57,0xDB,0x00,0x00,0x3D,0xD4,
	0x00,0x00,0x1F,0xCE,0x00,0x00,0xE2,0xC8,0x00,0x00,0xB1,0xC4,0x00,0x00,0xC0,0xC1,
	0x00,0x00,0xEC,0xBF,0x00,0x00,0xC1,0xBF,0x00,0x00,0xA4,0xC0,0x00,0x00,0xF2,0xC2,
	0x00,0x00,0x18,0xC6,0x00,0x00,0xC2,0xCA,0x00,0x00,0xC8,0xD0,0x00,0x00,0x36,0xD7,
	0x00,0x00,0xBB,0xDE,0x00,0x00,0xE6,0xE6,0x00,0x00,0xA5,0xEF,0x00,0x00,0xA6,0xF8,
};

int test_GetMonoPcm( const uchar ** pcm_data, int * pcm_bytes )
{
	*pcm_data  = s_pcmdata_mono;
	*pcm_bytes = sizeof s_pcmdata_mono;
	return 0;
}

int test_GetLeftPcm( const uchar ** pcm_data, int * pcm_bytes )
{
	*pcm_data  = s_pcmdata_left;
	*pcm_bytes = sizeof s_pcmdata_left;
	return 0;
}

int test_GetRightPcm( const uchar ** pcm_data, int * pcm_bytes )
{
	*pcm_data  = s_pcmdata_right;
	*pcm_bytes = sizeof s_pcmdata_right;
	return 0;
}

static unsigned char* hex_to_string(uchar *data, int size){
     int  i=0;
    unsigned char ch;
    unsigned char *out_hex_ptr=NULL;
    unsigned char *hex_ptr=(unsigned char*)malloc(size*2+1);
    if(NULL==hex_ptr){
        return NULL;
    }
    out_hex_ptr=hex_ptr;
    memset(hex_ptr,0,(size*2+1));

    for(i=0;i<size;i++){
        ch=(char)((data[i]&0xf0)>>4);

        if(ch<=9){
            *hex_ptr=(char)(ch+'0');
        }else{
            *hex_ptr=(char)(ch+'a'-10);
        }

        hex_ptr++;

        ch=(char)(data[i]&0x0f);


        if(ch<=9){
            *hex_ptr=(char)(ch+'0');
        }else{
            *hex_ptr=(char)(ch+'a'-10);

        }
        hex_ptr++;
    }
    return out_hex_ptr;   

}

int testAudioLoop(int mic, int spk){
    int ret=0;

    int output_devices=0;
    int input_devices=0;

    char write_buf[1024] = {0};
    ALOGI("testAudioLoop mic:0x%02x spk:0x%02x",mic,spk);

    switch(spk){
        case AUD_OUTDEV_SPEAKER:
            output_devices=AUDIO_DEVICE_OUT_SPEAKER;
            break;
        case AUD_OUTDEV_EARPIECE:
            output_devices=AUDIO_DEVICE_OUT_EARPIECE;
            break;
        case AUD_OUTDEV_HEADSET:
            output_devices=AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
            break;
        default:
            break;
        }

    switch(mic){
        case AUD_INDEV_BUILTIN_MIC:
            input_devices=AUDIO_DEVICE_IN_BUILTIN_MIC;
            break;
        case AUD_INDEV_HEADSET_MIC:
            input_devices=AUDIO_DEVICE_IN_WIRED_HEADSET;
            break;
        case AUD_INDEV_BACK_MIC:
            input_devices=AUDIO_DEVICE_IN_BACK_MIC;
            break;
        default:
            break;
    }

    snprintf(write_buf,sizeof(write_buf) -1,"test_stream_route=%d;test_in_stream_route=%d;skd_test=3",output_devices,input_devices);

    ALOGD("write:%s", write_buf);
    SendAudioTestCmd((const uchar *)write_buf,strlen(write_buf));

    return ret;
}

int testAudioLoopStop(void){
    char write_buf[1024] = {0};

    snprintf(write_buf,sizeof(write_buf) -1,"skd_test=0");

    ALOGD("write:%s", write_buf);
    SendAudioTestCmd((const uchar *)write_buf,strlen(write_buf));

    return 0;
}

int testAudioOUT(const uchar *data, int data_len, uchar *rsp, int rsp_size)
{
    int ret = 0;
    uchar out = data[0];
    uchar act = data[1];
    uchar chl = data[2];
    static int fd = -1;
    char write_buf[1024] = {0};
    int devices = 0;
    FUN_ENTER;

    if(out==0x20){
        out = data[1];
        int key = -1;
	ret = 1;
        int output_devices=0;
        switch(out){
            case AUD_OUTDEV_SPEAKER:
                output_devices=AUDIO_DEVICE_OUT_SPEAKER;
                break;
            case AUD_OUTDEV_EARPIECE:
                output_devices=AUDIO_DEVICE_OUT_EARPIECE;
                break;
            case AUD_OUTDEV_HEADSET:
                output_devices=AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
                break;
            default:
                break;
        }

        snprintf(write_buf,
            sizeof(write_buf) -
            1,"out_devices_test=1;test_stream_route=%d;samplerate=%d;channels=%d",
            output_devices,44100,2);

        ALOGD("write:%s", write_buf);
        SendAudioTestCmd((const uchar *)write_buf,strlen(write_buf));
        usleep(400*1000);

        ui_clear_key_queue();
        key = ui_wait_key_simp();
        INFMSG("testAudioOUT test, key = %d\n", key);

        if(key == 114)
        {
	    rsp[0] = 0;/*pass*/
        } else if (key == 116)
        {
	    rsp[0] = 1;/*fail*/
        } else if (key == 115) {
            rsp[0] = 2;/*test again*/
        }
        snprintf(write_buf, sizeof(write_buf) - 1, "out_devices_test=0");
        ALOGD("write:%s", write_buf);
        SendAudioTestCmd((const uchar *)write_buf,strlen(write_buf));
        return ret;

    }

    switch( act ) {
    case 0x01:{
        snprintf(write_buf,
                    sizeof(write_buf) - 1,"autotest_audiotracktest=1;autotest_outdevices=%d;autotest_channels=%d",
                    out,chl);
        ALOGD("write:%s", write_buf);
        SendAudioTestCmd((const uchar *)write_buf,strlen(write_buf));
        break;
    }
    case 0x02: {
        int data_size=0;
        unsigned char* str=NULL;
        data_size=data_len-4;
        str=hex_to_string((uchar *)(data+3),data_size);
        if(NULL==str){
            ret=-1;
        }else{
            snprintf(write_buf,
                        sizeof(write_buf)-
                        1,"autotest_audiotracktest=1;autotest_outdevices=%d;autotest_channels=%d;autotest_datasize=%d;autotest_data=%s",
                        out,chl,data_size,str);
            ALOGD("write:%s", write_buf);
            SendAudioTestCmd((const uchar *)write_buf,strlen(write_buf));
            free(str);
        }
        break;
    }
    case 0x03:
        snprintf(write_buf,sizeof(write_buf) - 1,"autotest_audiotracktest=0");
        ALOGD("write:%s", write_buf);
        SendAudioTestCmd((const uchar *)write_buf,strlen(write_buf));
        break;
    default:
        ret = -1;
        break;
    }

    FUN_EXIT;
    return ret;
}


/*
*skd test keybaklight
*/
int testKeyBL(void)
{
	int timeout = 2;// second.
	int key = -1;
	int ret = 0;
	int row = 3;

	test_result_init();

	ui_fill_locked();
	ui_show_title(MENU_TEST_KEYBL);
	ui_set_color(CL_WHITE);
	row = ui_show_text(row, 0, TEXT_KEYBL_ILLUSTRATE);
	gr_flip();

	lightOpen();
	lightSetKeypad(255);
	usleep(200*1000);
	lightSetKeypad(0);
	usleep(200*1000);
	lightSetKeypad(255);
	usleep(200*1000);
	lightSetKeypad(0);

	ui_set_color(CL_WHITE);
	row = ui_show_text(row, 0, TEXT_KEYBL_OVER);
	gr_flip();
	ui_clear_key_queue();
	key = ui_wait_key_simp();
	INFMSG("keybacklight test, key = %d\n", key);

	if(key == 114) {
		ui_set_color(CL_GREEN);
		row = ui_show_text(row, 0, TEXT_PASS);
		gr_flip();
		ret = RL_PASS;
	} else if (key == 116) {
		ui_set_color(CL_RED);
		row = ui_show_text(row, 0, TEXT_FAIL);
		gr_flip();
		ret = RL_FAIL;
	} else {
		ui_set_color(CL_BLUE);
		row = ui_show_text(row, 0, TEXT_AGAIN);
		gr_flip();
		ret = RL_NA;
	}
	return ret;
}

/*
*skd test rgb
*/
int testRGB(void)
{
        int timeout = 2;// second.
        int key = -1;
        int ret = 0;
        int row = 3;

        test_result_init();

        ui_fill_locked();
        ui_show_title(MENU_TEST_RGB);
        ui_set_color(CL_WHITE);
        row = ui_show_text(row, 0, TEXT_RGB_ILLUSTRATE);
        gr_flip();

        lightOpen();
        lightSetRgb(0x00,0x01);
        usleep(1000*1000);
	lightSetRgb(0x00,0x00);
        lightSetRgb(0x01,0x01);
        usleep(1000*1000);
	lightSetRgb(0x01,0x00);
        lightSetRgb(0x02,0x01);
        usleep(1000*1000);
        lightSetRgb(0x02,0x00);

        ui_set_color(CL_WHITE);
        row = ui_show_text(row, 0, TEXT_RGB_OVER);
        gr_flip();
        ui_clear_key_queue();
        key = ui_wait_key_simp();
        INFMSG("RGB test, key = %d\n", key);

        if(key == 114) {
                ui_set_color(CL_GREEN);
                row = ui_show_text(row, 0, TEXT_PASS);
                gr_flip();
                ret = RL_PASS;
        } else if (key == 116) {
                ui_set_color(CL_RED);
                row = ui_show_text(row, 0, TEXT_FAIL);
                gr_flip();
                ret = RL_FAIL;
        } else {
                ui_set_color(CL_BLUE);
                row = ui_show_text(row, 0, TEXT_AGAIN);
                gr_flip();
                ret = RL_NA;
        }
        return ret;
}


/*
*skd test vibrator
*/
int testVIB(void)
{
	int timeout = 2;// second.
	int key = -1;
	int ret = 0;
	int row = 3;
	int ms = 100;

	test_result_init();

	ui_fill_locked();
	ui_show_title(MENU_TEST_VIBRATOR);
	ui_set_color(CL_WHITE);
	row = ui_show_text(row, 0, TEXT_VIB_START);
	gr_flip();

	vibOpen();
	vibTurnOn(timeout);
	usleep(ms*1000);
	vibTurnOff();
	usleep(ms*1000);
	vibTurnOn(timeout);
	usleep(ms*1000);
	vibTurnOff();
	usleep(ms*1000);
	vibTurnOn(timeout);
	usleep(ms*1000);
	vibTurnOff();
	vibClose();

	ui_set_color(CL_GREEN);
	row = ui_show_text(row, 0, TEXT_VIB_FINISH);
	row = ui_show_text(row, 0, TEXT_FINISH);
	gr_flip();

	ui_clear_key_queue();
	key = ui_wait_key_simp();
	INFMSG("vibrator test, key = %d\n", key);

	if(key == 114) {
		ui_set_color(CL_GREEN);
		row = ui_show_text(row, 0, TEXT_PASS);
		gr_flip();
		ret = RL_PASS;
	} else if (key == 116) {
		ui_set_color(CL_RED);
		row = ui_show_text(row, 0, TEXT_FAIL);
		gr_flip();
		ret = RL_FAIL;
	} else {
		ui_set_color(CL_BLUE);
		row = ui_show_text(row, 0, TEXT_AGAIN);
		gr_flip();
		ret = RL_NA;
	}
	return ret;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// lcd backlight, keypad backlight, vibrator, flashlight
int testLKBV(const uchar * data, int data_len, uchar * rsp, int rsp_size)
{
    int ret = 0;
    int ret_var = -1;

    FUN_ENTER;

    INFMSG("data[0] = %d,data[1] = %d\n,data[2] = %d\n", *data,data[1],data[2]);

    switch( *data ) {
    case 0x00: //LCDBACKLIGHT
    {
        int value = data[1];
		lightOpen();
        if( value > 0 ) {
            lightSetLCD(value);
        } else {
            lightSetLCD(0);
            lightClose();
        }
    }
        break;
    case 0x02: // KPDBACKLIGHT
	{
        int value = data[1];
	lightOpen();
        if( value > 0 ) {
	    lightSetKeypad(value);
        } else {
	    lightSetKeypad(0);
	    lightClose();
        }
    }
        break;
    case 0x03: // vibrator
    {
        int timeout = data[1];
        if( timeout > 0 ) {
	    ret = vibOpen();
	    if(-1 == ret){
		LOGD("the vib open fail !!!\n");
	    }else{
		vibTurnOn(timeout);
	    }
        } else {
            vibTurnOff();
	    vibClose();
        }
    }
        break;
    case 0x04: // flash light
    {
		int cmd_val = 0;
		//0x00:off,!0x00 open
		if(data[1] != 0x00) {
			cmd_val |= 0x00; //open flashlight
			cmd_val |= (data[1] & 0x0f) << 4; //flash index
		} else if(data[1] == 0x00) {
			cmd_val |= 0x31; //close flashlight
		}
		if( flashlightSetValue(cmd_val) < 0 ) {
			ret = -1;
		}
    }
    break;
    case 0x05: //otg
    {
        switch(data[1]) {
            case 0x01:
                {
                    if(otgDisable() < 0) {
                        ret = -1;
                    }
                }
                break;
            case 0x02:
                {
                    if(otgIdStatus() == 0) {
                        rsp[0] = 0;
                        ret = 1;
                    }
                    else if(otgIdStatus() == 1) {
                        rsp[0] = 1;
                        ret = 1;
                    }
                    else {
                        ret = -1;
                    }
                }
                break;
            case 0x03:
                {
                    if(data[2] == 1) {
                        if(otgVbusOpen() < 0) {
                            ret = -1;
                        }
                    }
                    else {
                        if(otgVbusClose() < 0) {
                            ret = -1;
                        }
                    }
                }
                break;
            default:
                {
                }
                break;
        }
    }
        break;
	case 0x06: // mic
	{
		uchar state = headsetPlugState();
		/*state is unsigned, always equal-to or greater-than 0*/
		/*if (state >= 0)*/{
			rsp[0] = state;
			ret = 1;
		}
	}
		break;
	case 0x07: //tp
        {
	switch (data[1])
	{
	case 1:
		test_result_init();
		if (TouchPanel_Id() < 0)
			rsp[0] = 1;
		else
			rsp[0] = 0;
		ret = 1;
	break;
	case 2:
		test_result_init();
		ret = TouchPanel_Point(rsp);
		if(1 == ret) {
			LOGD("rsp[0]=0x%02X", rsp[0]);
		} else if (8 == ret) {//two point
			LOGD("X1=0x%02X%02X, Y1=0x%02X%02X", rsp[0],
				rsp[1], rsp[2], rsp[3]);
			LOGD("X2=0x%02X%02X, Y2=0x%02X%02X", rsp[4],
				rsp[5], rsp[6], rsp[7]);
		} else {
			LOGD("X1=0x%02X%02X, Y1=0x%02X%02X", rsp[0],
				rsp[1], rsp[2], rsp[3]);
		}

	break;
	default:
		;
	}
        }
        break;
	case 0x09://RGB
	{
		int value1 = data[1];//0x00:RED , 0x01:GREEN , 0x02:BLUE
		int value2 = data[2];//0x01:on ,0x00:off

		if(value1 >= 0x00)
			lightSetRgb(value1 , value2);
		else
			ret = -1;
	}
		break;
		
 	 case 0x0a: //battery tempture
	{

	    	char bat_temp[5]={0};
	    	int rettemp = -2;
	     	int i=0;
		int a=0;
		int b=0;
		int c=0;
		memset(bat_temp, 0, 5);
		INFMSG(" testLKBV i=%d  bat_temp=%s  \n", sizeof(bat_temp),bat_temp);
	       for(i=0;i<4;i++){
                  rsp[i]=0x00;
	   	  INFMSG(" testLKBV i=%d  rsp[i]=%x\n", i, rsp[i]);
	  	}
	      // INFMSG(" testLKBV  bat_temp=%s\n", bat_temp);
              rettemp = batGetTempureValue(bat_temp);
	  	 if (rettemp != 0)
	           	 ret = -1;
	  	 else if  (rettemp == 0)
		{
		   	 ret = 4;
	          	 INFMSG(" testLKBV rettemp = %d bat_temp=%s sizeof(bat_temp)=%d  strlen(bat_temp)=%d\n", rettemp, bat_temp, sizeof(bat_temp),strlen(bat_temp));
	           	 //INFMSG(" testLKBV 0=%c 1 = %c 2=%c 3=%c \n", bat_temp[0], bat_temp[1], bat_temp[2], bat_temp[3]);
	      		a = atoi(bat_temp);
	     		INFMSG(" testLKBV a=%d \n", a);
	   		a= a+250;
	   		INFMSG(" testLKBV a=%d  \n", a);
         		 b= a/255;
	  		 c= a%255;
	  		INFMSG(" testLKBV b=%d  c=%d\n", b, c);

        	 	for(i=0;i<b;i++){
        	  	rsp[i]=0xff;
	   	  	INFMSG(" testLKBV i=%d  rsp[i]=%x\n", i, rsp[i]);
	  		}

	  		 rsp[i]=(uchar)c;
	  
         		 INFMSG(" testLKBV rsp[i]=%x i=%d \n", rsp[i],i);
	  		 INFMSG(" testLKBV rsp=%s ret=%d \n", rsp, ret);
		}
	 }
		break;
	case 0x0b:{
		int cmd_val = 0;
		if(data[1] != 0x00) {
			cmd_val |= 0x06; //open torch
			cmd_val |= 1<<4; //flash index
			cmd_val |= ((data[1] & 0xff) << 8) & 0xff;
			flashlightSetValue(0x0010); //open.
		} else if(data[1] == 0x00) {
			cmd_val |= 0x01; //close torch
			cmd_val |= 1<<4; //flash index
			cmd_val |= ((data[1] & 0xff) << 8) & 0xff;
		}
		if( flashlightSetValue(cmd_val) < 0 ) {
			ret = -1;
		}
	}
	break;
	case 0x20:
	{
 		switch(data[1]){
			case 0x00:
			{
				skdBacklightTest(&skd_backlight);
			}
				break;
			case 0x03: //skd vibrator
			{
				ret = 1;
				ret_var = testVIB();
				if(2 == ret_var){
				    rsp[0] = 0;
				} else if(1 == ret_var) {
				    rsp[0] = 1;
				} else if (0 == ret_var){
				    rsp[0] = 2;//skd vibrator test again
				}
			}
			break;

			case 0x01: //skd keybacklight
			{
				ret = 1;
				ret_var = testKeyBL();
				if(2 == ret_var){
				    rsp[0] = 0x00;
				} else if(1 == ret_var) {
				    rsp[0] = 0x01;
				} else if(0 == ret_var) {
				    rsp[0] = 0x02;//skd keybacklight test again
				}

			}
			break;
			
			case 0x09: //skd rgb
                        {
                                ret = 1;
                                ret_var =testRGB();
                                if(2 == ret_var){
                                    rsp[0] = 0x00;
                                } else if(1 == ret_var) {
                                    rsp[0] = 0x01;
                                } else if(0 == ret_var) {
                                    rsp[0] = 0x02;//skd rgb test again
                                }
                        }
                        break;

			case 0x04:
			{
			    skdCameraFlashTest(&skd_flash_r);
			}
			break;
			case 0x07: //skd tp test
				test_result_init();
				ret = testTpStart();
				switch (ret)
				{
					case 0x72:/*v-,pass*/
					skd_tp_r = RESULT_PASS;
					break;
					case 0x73:/*v+,again*/
					skd_tp_r = RESULT_AGAIN;
					break;
					case 0x74:/*power,fail*/
					skd_tp_r = RESULT_FAIL;
					break;
				}
				rsp[0] = skd_tp_r;
				INFMSG("TP test rsp[0]=%d\n", rsp[0]);
				ret = 1;
				break;
			default:
				break;
		}
	}
		break;
    default:
        break;
    }

    FUN_EXIT;
    return ret;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int testFM(const uchar * data, int data_len, uchar * rsp, int rsp_size)
{
    int ret = 0;

    FUN_ENTER;
    INFMSG("data[0] = %d\n", *data);

    switch( *data ) {
	case 1:
	    {
		uint freq = ((data[1] << 16) | (data[2] << 8) | (data[3] << 0)); //899

		//for android 6 and upon platform Using google original apk  or marlin
#if ((defined( GOOGLE_FM_INCLUDED) || defined (SPRD_WCNBT_MARLIN) )&&(!defined(BOARD_HAVE_FM_BCM)))

		if( Radio_Open(freq) < 0 || Radio_Play(freq) < 0 ){
		    ret = -1;
		    bbat_fm_rssi_r = 0;
		    INFMSG("Radio_Open or Radio_Play the fm open failll!!!\n");
		}else{
		    ret =  Radio_GetRssi(&bbat_fm_rssi_r);
		    if(0!=ret){
			ret = -1;
			INFMSG("Radio_Open or Radio_Play the fm open failll!!!\n");
		    }else{
			ret = 4;
			rsp[0]  = bbat_fm_rssi_r >> 24;
			rsp[1]  = (bbat_fm_rssi_r >> 16)&0xFF;
			rsp[2]  = (bbat_fm_rssi_r >> 8)&0xFF;
			rsp[3]  = (bbat_fm_rssi_r >> 0)&0xFF;
			INFMSG("-----------rsp[0] = %d rsp[1] = %d rsp[2]= %d rsp[3] = %d  \n", rsp[0], rsp[1], rsp[2], rsp[3]);
		}

		}
#else
#ifdef SPRD_WCNBT_SR2351 //2351
		if( fmOpen() < 0 || fmPlay(freq) < 0 ){ ret = -1;}
#else
#ifdef BOARD_HAVE_FM_BCM //BCM

		if( Bcm_fmOpen() < 0 || Bcm_fmPlay(freq,&bbat_fm_rssi_r) < 0 ){
		    //ret = 5;
		    ret = -1;
		INFMSG("the fm open failll!!!\n");
		}else{
		    ret = 4;
		    //(unsigned char &)bbat_fm_rssi_r;
		    rsp[0]  = bbat_fm_rssi_r >> 24;
		    rsp[1]  = (bbat_fm_rssi_r >> 16)&0xFF;
		    rsp[2]  = (bbat_fm_rssi_r >> 8)&0xFF;
		    rsp[3]  = (bbat_fm_rssi_r >> 0)&0xFF;

		    INFMSG("-----------rsp[0] = %d rsp[1] = %d rsp[2]= %d rsp[3] = %d  \n", rsp[0], rsp[1], rsp[2], rsp[3]);
		}


#endif
#endif
#endif
	    }
	    break;
	case 2:
	    //for android 6 and upon platform Using google original apk marlin
#if ((defined( GOOGLE_FM_INCLUDED) || defined (SPRD_WCNBT_MARLIN))&&(!defined(BOARD_HAVE_FM_BCM)))
	    if( Radio_Close() < 0 )
		ret = -1;
#endif

#ifdef SPRD_WCNBT_SR2351 //2351
	    fmStop();
	    fmClose();
#endif

#ifdef BOARD_HAVE_FM_BCM //bcm
	    Bcm_fmStop();
	    Bcm_fmClose();
#endif
	    break;
	case 0x20:
	    {
		skd_fm_freq_r = FM_START_FREQ;
		ret = Radio_SKD_Test(&skd_fm_status_r, &skd_fm_freq_r, &skd_fm_rssi_r);
		INFMSG("status = %d, freq = %d, rssi = %d\n",
			skd_fm_status_r, skd_fm_freq_r, skd_fm_rssi_r);
	    }
	    break;
	default:
	    break;
    }

    FUN_EXIT;
    return ret;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int testBT(const uchar * data, int data_len, uchar * rsp, int rsp_size)
{
    int ret = -1;
    int num = 0;
    struct bdremote_t bdrmt[MAX_SUPPORT_RMTDEV_NUM];

    FUN_ENTER;
    INFMSG("BT Test CMD:%d\n", *data);

    switch( *data ) {
    case 1: // open bt
        if( !btIsOpened() ) {
            ret = btOpen();
        }
        break;
    case 2: // inquire
        num = btGetInquireResult(bdrmt, MAX_SUPPORT_RMTDEV_NUM);
        ret = -1;
        if(BT_STATUS_INQUIRE_UNK== btGetInquireStatus()){
            DBGMSG("BT IRQ:Begin to Search!");
            btAsyncInquire();
        }else if(BT_STATUS_INQUIRE_END== btGetInquireStatus() || BT_STATUS_INQUIRING== btGetInquireStatus()){
            if(btGetInquireResult(bdrmt, MAX_SUPPORT_RMTDEV_NUM) > 0){
                ret = 0;
            } else if(BT_STATUS_INQUIRE_END== btGetInquireStatus()){
                DBGMSG("BT_STATUS_INQUIRE_END with no device found!");
                btSetInquireStatus(BT_STATUS_INQUIRE_UNK);
            } else
                DBGMSG("BT_STATUS_INQUIRING with no device found now!");
        }
        break;
    case 3: // get inquire
    {
        num = btGetInquireResult(bdrmt, 1);
        if( num > 0 ) {
            int i;
            uchar * pb = (uchar *)rsp;
            ret = num * 6;

            DBGMSG("BT get inquire:num = %d, pb = %p\n", num, pb);
            for( i = 0; i < num; ++i ) {
                //memcpy(pb, bdrmt[i].baddr, 6);
                pb[0] = bdrmt[i].addr_u8[5];  pb[1] = bdrmt[i].addr_u8[4];
                pb[2] = bdrmt[i].addr_u8[3];  pb[3] = bdrmt[i].addr_u8[2];
                pb[4] = bdrmt[i].addr_u8[1];  pb[5] = bdrmt[i].addr_u8[0];
                pb += 6;
            }
        } else {
            ret = -1;
        }
    }
        break;
    case 4:
        ret = btClose();
        break;
    case 5:  //get RSSI
        num = btGetInquireResult(bdrmt, 1);
        if(num > 0) {
            int i;
            uchar * pb = (uchar *)rsp;
            ret = num * (6 + 4);

            DBGMSG("BT get inquire:num = %d, pb = %p\n", num, pb);
            for( i = 0; i < num; ++i ) {
                int sig = bdrmt[i].rssi_val;
                pb[0] = bdrmt[i].addr_u8[5];  pb[1] = bdrmt[i].addr_u8[4];
                pb[2] = bdrmt[i].addr_u8[3];  pb[3] = bdrmt[i].addr_u8[2];
                pb[4] = bdrmt[i].addr_u8[1];  pb[5] = bdrmt[i].addr_u8[0];
                pb += 6;
                *pb++ = (sig >> 0)  & 0xFF;
                *pb++ = (sig >> 8)  & 0xFF;
                *pb++ = (sig >> 16) & 0xFF;
                *pb++ = (sig >> 24) & 0xFF;

            }
        } else {
            ret = -1;
        }
	    break;
    default:
        break;
    }

    DBGMSG("Return Value:%d",ret);
    FUN_EXIT;
    return ret;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int testWIFI(const uchar * data, int data_len, uchar * rsp, int rsp_size)
{
    int ret = 0;
    int wifiStatus = wifiGetStatus();

    #define MAX_SUPPORT_AP_NUM 10

    FUN_ENTER;
    INFMSG("testWIFI cmd = %d\n", *data);

    switch( *data ) {
    case 1: // open wifi
        if( !(WIFI_STATUS_OPENED & wifiStatus) ) {
            if( wifiOpen() < 0 ) {
                ret = -1;
            }
        }
        break;
    case 2: // inquire
        if( !(WIFI_STATUS_SCAN_END & wifiStatus) ) {
            if( !(WIFI_STATUS_SCANNING & wifiStatus) ) {
                ret = wifiAsyncScanAP();
            }
        }

	/*if wifi don't successfully get the AP , it should be return fail*/
	if(  !(WIFI_STATUS_SCAN_END & wifiStatus) )
		ret = -1;
        break;
    case 3: // get inquired
    {
        struct wifi_ap_t aps[MAX_SUPPORT_AP_NUM];
        int num = wifiGetAPs(aps, MAX_SUPPORT_AP_NUM);
        if( num > 0 ) {
            int i;
            uchar * pb = (uchar *)rsp;
            ret = num * 6;

            //DBGMSG("num = %d, pb = %p\n", num, pb);
            for( i = 0; i < num; ++i ) {
                //memcpy(pb, aps[i].bmac, 6);
                pb[0] = aps[i].bmac[5];  pb[1] = aps[i].bmac[4];
                pb[2] = aps[i].bmac[3];  pb[3] = aps[i].bmac[2];
                pb[4] = aps[i].bmac[1];  pb[5] = aps[i].bmac[0];
                pb += 6;
            }
        } else {
            ret = -1;
        }
    }
        break;
    case 4:
        wifiClose();
        break;
    case 5: // get inquired ex
    {
        struct wifi_ap_t aps[MAX_SUPPORT_AP_NUM];
        int num = wifiGetAPs(aps, MAX_SUPPORT_AP_NUM);
        if( num > 0 ) {
            int i;
            uchar * pb = (uchar *)rsp;
            ret = num * (6 + 4);

            //DBGMSG("num = %d, pb = %p\n", num, pb);
            for( i = 0; i < num; ++i ) {
                //memcpy(pb, aps[i].bmac, 6);
                pb[0] = aps[i].bmac[5];  pb[1] = aps[i].bmac[4];
                pb[2] = aps[i].bmac[3];  pb[3] = aps[i].bmac[2];
                pb[4] = aps[i].bmac[1];  pb[5] = aps[i].bmac[0];
                pb += 6;
                //memcpy(pb, &(aps[i].signal), 4);
                int sig = aps[i].sig_level;
                pb[0] = (sig >> 0)  & 0xFF;
                pb[1] = (sig >> 8)  & 0xFF;
                pb[2] = (sig >> 16) & 0xFF;
                pb[3] = (sig >> 24) & 0xFF;
                pb += 4;
            }
        } else {
            ret = -1;
        }
    }
        break;

    default:
        break;
    }

    FUN_EXIT;
    return ret;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int testIIC(const uchar * data, int data_len, uchar * rsp, int rsp_size)
{
    int ret = 0;
    uchar val;

    FUN_ENTER;
    INFMSG("addr = %02X, reg = %02X, bus = %02X, ack = %d\n", data[0], data[1], data[2], data[3]);

    if( drvOpen() >= 0 && drvI2CRead(data[2], data[0], data[1], &val) >= 0 ) {
        ret = 2;
        rsp[0] = 0x00;
        rsp[1] = val;
    } else {
        ret = -1;
    }

	drvClose();

    FUN_EXIT;
    return ret;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int testCharger(const uchar * data, int data_len, uchar * rsp, int rsp_size)
{
    int ret = 0;

    FUN_ENTER;
    INFMSG("data[0] = %d\n", *data);

    switch(*data)
    {
    case 1:
        //batOpen();
        if( batEnableCharger(1) < 0 ) {
            ret = -1;
        }
        break;
    case 2:
        if( batStatus() < 0 ) {
            ret = -1;
        }
        break;
    case 3:
        if( batEnableCharger(0) < 0 ) {
            ret = -1;
        }
        //batClose();
        break;
    default:
        break;
    }

    FUN_EXIT;
    return ret;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int testSensor(const uchar * data, int data_len, uchar *rsp, int rsp_size)
{
    int ret = -1;
    unsigned int item_result=RESULT_FAIL;
    static unsigned int   isEntry=0;
    void *sprd_handle_fingersor_dl;
    typedef int (*spi_test_dl)();
	FUN_ENTER;
	INFMSG("data[0] = %d\n", *data);
//*********************************
   INFMSG("start new plan\n");
   autotest_modules *modules_list = NULL;
    struct list_head *list_find;
    list_for_each(list_find,&test_head){

        modules_list = list_entry(list_find, autotest_modules, node);
        INFMSG("engpc_callback cmd %x\n",modules_list->callback.diag_ap_cmd);
        if(modules_list->callback.diag_ap_cmd == DIAG_CMD_SENSOR){
            ret = modules_list->callback.autotest_func(data,data_len,rsp,rsp_size);
        }
    }
	INFMSG("end new plan\n");
	if ((ret > 0 )|| (ret == 0) )
	return ret ;
//**************************
    INFMSG(" deta[0] = 0x%02x\n", *data);
    switch (data[0]) {
	//BBAT MODE
	case 0x01:  //A sensor
	case 0x02:  //M sensor
	case 0x04:  //G sensor
	case 0x05:  //ALS sensor
	case 0x06:  //presure sensor
	case 0x08:  //PS sensor
	    if( sensorOpen() >= 0 && sensorActivate(data[0]) >= 0 ) {
		rsp[0] = 0x00;  //PASS
	    } else {
		rsp[0] = 0x01;
	    }
	    sensorClose();
		ret = 1;
	    break;
	    //>>add the whale 2 virtual sensor no support
	case 0x03:  //O sensor
	case 0x07:  //Tempture sensor
	case 0x09:
	case 0x0a:
	case 0x0b:
	    rsp[0] = 0x01;
		ret = 1;
	    break;

	case 0x0D:
		ret=0;
		LOGD("test NFC\n");
		switch (data[1]) {
			case 0x01://open nfc
				ret=nfc_open();
			break;
			case 0x02://check ID
				ret=1;
				rsp[0]=nfc_checkID();
					ALOGD(" rsp[0] = %d\n", rsp[0]);
			break;
			case 0x03://close nfc
				ret=nfc_close();
			break;
		}
		break ;

	case 0x20:
	    LOGD("test sensor \n"); //CANNOT ENTRY AUTO TESTMODE("autotest path check fail") if uese LOGI
	    switch (data[1]) {
		LOGD("yuebao ==%s= data[1]=0x%02x\n",__FUNCTION__,data[1]);
		case ITEM_ASENSOR:
			if(data[2]==1)
			{
			INFMSG("entry Asenor test....\n");	
			item_result = skd_asensor_result =test_asensor_start();
			if (item_result == 0)
			{
				ret=12;								
				rsp[0]=symbol_x; //+ or -
				rsp[1]=(uint8_t)(x_value>>16);
				rsp[2]=(uint8_t)(x_value>>8);
				rsp[3]=(uint8_t)(x_value);
				rsp[4]=symbol_y; //+ or -
				rsp[5]=(uint8_t)(y_value>>16);
				rsp[6]=(uint8_t)(y_value>>8);
				rsp[7]=(uint8_t)(y_value);
				rsp[8]=symbol_z; //+ or -
				rsp[9]=(uint8_t)(z_value>>16);
				rsp[10]=(uint8_t)(z_value>>8);
				rsp[11]=(uint8_t)(z_value);		
				for (int ii=0;ii<12;ii++)
				LOGD("rsp[%d]=%d\r\n",ii,rsp[ii]);	
			}else{
		      	INFMSG("%s the   not support \n",__FUNCTION__);
		  	 	ret =-1;
	        }
		  }
			break;
		case ITEM_LSENSOR:
		if(data[2]==1)	{
			LPsensor=1;
			item_result = skd_lsensor_result =test_lsensor_start();
			if (item_result == 0)
				{
				ret=2;
				rsp[0]=(uint8_t)(x_value>>8);
				rsp[1]=(uint8_t)(x_value);
				LOGD("rsp[0]:%d,rsp[1]:%d\r\n",rsp[0],rsp[1]);
				}else{
		      	INFMSG("%s the   not support \n",__FUNCTION__);
		  	 	ret =-1;
				}	
		}else if(data[2]==2){
			LPsensor=2;
			item_result = skd_lsensor_result =test_lsensor_start();	
			if (item_result == 0){
				ret=1;			
				rsp[0]=(uint8_t)(y_value);
				LOGD("rsp[0]:%d\r\n",rsp[0]);	
			}else{
			INFMSG("%s   not support \n",__FUNCTION__);
			ret=-1;
	    	}
		}
		break;
		case ITEM_GSENSOR:
		INFMSG("entry Gsenor test....\n");
		item_result = skd_gsensor_result = test_gsensor_start();
		break;

		case ITEM_MSENSOR:
			if(data[2]==1)
				{
				item_result = skd_msensor_result=test_msensor_start();	
				if (item_result == 0){
					ret=10;
					INFMSG("%s   test  SUCCESS\n",__FUNCTION__);	
					rsp[0]=m_level; //precision level
					rsp[1]=symbol_x; //+ or -
					rsp[2]=(uint8_t)(x_value>>8);
					rsp[3]=(uint8_t)(x_value);
					
					rsp[4]=symbol_y; //+ or -				
					rsp[5]=(uint8_t)(y_value>>8);
					rsp[6]=(uint8_t)(y_value);
					
					rsp[7]=symbol_z; //+ or -				
					rsp[8]=(uint8_t)(z_value>>8);
					rsp[9]=(uint8_t)(z_value);
					for (int ii=0;ii<10;ii++)
					LOGD("rsp[%d]=%d\r\n",ii,rsp[ii]);
				}else{
				INFMSG("%s   not support \n",__FUNCTION__);
				ret =-1;
				}
			}
			break;
		case ITEM_PSENSOR:
			if(data[2]==1)
			{
				item_result = skd_psensor_result=test_psensor_start();
				if (item_result == 0){			
				ret=3;
				rsp[0]=(uint8_t)(x_value>>16); 
				rsp[1]=(uint8_t)(x_value>>8); 
				rsp[2]=(uint8_t)(x_value);	
				}else{
				INFMSG("%s   not support\n",__FUNCTION__);
				ret =-1;
			 	}
			}	
			break;

		default:
		INFMSG("%s   NOT   SUPPORT THE COMMAND \n",__FUNCTION__);
		ret =-1;
		break;
      }
	}
    DBGMSG("Return Value:%d",ret);
    FUN_EXIT;
    return ret;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int testGps(const uchar * data, int data_len, uchar *rsp, int rsp_size)
{
    int ret = -1;

    FUN_ENTER;
    INFMSG("data[0] = %d\n", *data);
    INFMSG("the gpsOpen interface func crash int err = sGpsIntrfc->init(&sGpsCallbacks);\n ");
    switch(*data)
    {
	case 0: // init, not used
	    break;
	case 1:
	    if( gpsOpen() >= 0 ) {
		ret = 0;
	    }
	    break;
	case 2:
	    if( gpsStart() >= 0 ) {
		ret = 0;
	    }
	    break;
	case 3:{
		   int svn = gpsGetSVNum();
		   rsp[0] = (uchar)svn;
		   ret    = 1;
	       }
	       break;
	case 4:
	       gpsStop();
	       gpsClose();
	       ret = 0;
	       break;
	case 5:
	       {
		   int i = 0;
		   int num_gps = 0;
		   int snr[64] = {0};
		   gpsGet_PRN_SN_Num(&num_gps, snr);
		   INFMSG("testGps: num_gps = %d\n", num_gps);
		   rsp[0] = (uchar)num_gps;
		   for(i=0;i<num_gps;i++) {
		       rsp[i+1] = (uchar)snr[i];
		       INFMSG("testGps: i=%d, snr[i]=%d rsp[i+1]=%x\n", i, snr[i], rsp[i+1]);
		   }
		   ret = num_gps + 1;
	       }
	       break;
	default:
	       break;
    }

    FUN_EXIT;
    return ret;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int test_Deinit( void )
{
	FUN_ENTER;

	drvClose();
	release_wake_lock(AT_WAKE_LOCK_NAME);

	FUN_EXIT;
	return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//--} // namespace
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void lcd_test_open(void)
{
	test_result_init();
	test_lcd_start();
}

void skdLcdTest(volatile unsigned char* ret_val)
{

	uchar btn_save=0x00;
	bool inputRunning = true;

	test_result_init();
	skd_test_lcd_start();

	if(inputOpen()<0)
	{
		INFMSG("skdLcdTest, input fail !!\n");
	}
	struct kpd_info_t kpdinfo;
	while(inputRunning){
		if( inputKPDWaitKeyPress(&kpdinfo, 5*1000) < 0) {
			INFMSG("testLcd kpdinfo.key read fail");
			continue;
		}else{
			btn_save = ((uchar)(kpdinfo.key & 0xFF));
			if( btn_save==0x72 ){
				INFMSG("testLcd pressed vol down pass");
				*ret_val = 0x00;
				inputRunning = false;
			}else if( btn_save==0x74 ){
				INFMSG("testLcd pressed power fail");
				*ret_val = 0x01;
				inputRunning = false;
			}else if( btn_save==0x73 ){
			    INFMSG("testLcd pressed vol up test again");
			    *ret_val = 0x02;
			    inputRunning = false;
			}

		}
	}

	test_lcd_uinit();
	INFMSG("testLcd close.........");
	inputClose();
}
void skdBacklightTest(volatile unsigned char* ret_val)
{
	int max_brightness = 255;
	uchar btn_save=0x00;
	bool inputRunning = true;
	test_result_init();
	test_bl_lcd_start();
	lightOpen();
	for(int i = 0; i < 5; i++) {
		lightSetLCD(max_brightness>>i);
		usleep(500*1000);
	}
	lightSetLCD(max_brightness);

	if(inputOpen()<0)
	{
		INFMSG("skdBacklightTest, input fail !!\n");
	}
	struct kpd_info_t kpdinfo;
	while(inputRunning){
		if( inputKPDWaitKeyPress(&kpdinfo, 5*1000) < 0) {
			INFMSG("testBacklight kpdinfo.key read fail");
			continue;
		}else{
			btn_save = ((uchar)(kpdinfo.key & 0xFF));
			if( btn_save==0x72 ){
				INFMSG("testBacklight pressed vol down pass");
				*ret_val = 0x00;
				inputRunning = false;
			}else if( btn_save==0x74 ){
				INFMSG("testBacklight pressed power fail");
				*ret_val = 0x01;
				inputRunning = false;
			}else if( btn_save==0x73 ){
			    INFMSG("testBacklight pressed vol up test again");
			    *ret_val = 0x02;
			    inputRunning = false;
			}

		}
	}

	lightSetLCD(127);//default backlight brightness
	lightClose();
	INFMSG("testBacklight close.........");
	inputClose();
}
