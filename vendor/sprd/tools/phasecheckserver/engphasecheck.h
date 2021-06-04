#ifndef _ENG_PHASECHECK_H
#define _ENG_PHASECHECK_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define ENG_LOG(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

#define MAX_SN_LEN 24
#define SP09_MAX_SN_LEN                    MAX_SN_LEN
#define SP09_MAX_STATION_NUM               (15)
#define SP09_MAX_STATION_NAME_LEN          (10)
#define SP09_SPPH_MAGIC_NUMBER             (0X53503039)    // "SP09"
#define SP05_SPPH_MAGIC_NUMBER             (0X53503035)    // "SP05"
#define SP09_MAX_LAST_DESCRIPTION_LEN      (32)
#define SIGN_TYPE 0
#define ITEM_TYPE 1

#define TYPE_GET_SN1 0
#define TYPE_GET_SN2 1
#define TYPE_WRITE_TESTED 2
#define TYPE_WRITE_PASS 3
#define TYPE_WRITE_FAIL 4
#define TYPE_GET_STATION 5
#define TYPE_CHARGE_SWITCH 6
#define TYPE_RED_LED_NODE 7
#define TYPE_BLUE_LED_NODE 8
#define TYPE_GREEN_LED_NODE 9
#define TYPE_PHASECHECK 10


typedef struct _tagSP09_PHASE_CHECK
{
    unsigned int Magic;           // "SP09"   (\C0\CF\u0153ӿ\DAΪSP05)
    char    SN1[SP09_MAX_SN_LEN]; // SN , SN_LEN=24
    char    SN2[SP09_MAX_SN_LEN]; // add for Mobile
    unsigned int StationNum;      // the test station number of the testing
    char    StationName[SP09_MAX_STATION_NUM][SP09_MAX_STATION_NAME_LEN];
    unsigned char Reserved[13];
    unsigned char SignFlag;
    char    szLastFailDescription[SP09_MAX_LAST_DESCRIPTION_LEN];
    unsigned short  iTestSign;    // Bit0~Bit14 ---> station0~station 14
    //if tested. 0: tested, 1: not tested
    unsigned short  iItem;        // part1: Bit0~ Bit_14 indicate test Station,1\B1\ED\CA\u0178Pass,

}SP09_PHASE_CHECK_T, *LPSP09_PHASE_CHECK_T;

/*add the struct add define to support the sp15*/
#define SP15_MAX_SN_LEN                 (64)
#define SP15_MAX_STATION_NUM            (20)
#define SP15_MAX_STATION_NAME_LEN       (15)
#define SP15_SPPH_MAGIC_NUMBER          (0X53503135)    // "SP15"
#define SP15_MAX_LAST_DESCRIPTION_LEN   (32)

typedef struct _tagSP15_PHASE_CHECK {
    unsigned int Magic;         // "SP15"
    char SN1[SP15_MAX_SN_LEN];  // SN , SN_LEN=64
    char SN2[SP15_MAX_SN_LEN];  // add for Mobile
    int StationNum;             // the test station number of the testing
    char StationName[SP15_MAX_STATION_NUM][SP15_MAX_STATION_NAME_LEN];
    unsigned char Reserved[13]; //
    unsigned char SignFlag;
    char szLastFailDescription[SP15_MAX_LAST_DESCRIPTION_LEN];
    unsigned int iTestSign;     // Bit0~Bit14 ---> station0~station 14
    //if tested. 0: tested, 1: not tested
    unsigned int iItem;         // part1: Bit0~ Bit_14 indicate test Station, 0: Pass, 1: fail
} SP15_PHASE_CHECK_T, *LPSP15_PHASE_CHECK_T;

static const int SP09_MAX_PHASE_BUFF_SIZE = sizeof(SP09_PHASE_CHECK_T);

int eng_getphasecheck(SP09_PHASE_CHECK_T* phase_check);

#ifdef __cplusplus
}
#endif

#endif
