/*
 *    Copyright (C) 2013 SAMSUNG S.LSI
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at:
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 *   Author: Woonki Lee <woonki84.lee@samsung.com>
 *
 */
#ifndef __NFC_SEC_HAL__
#define __NFC_SEC_HAL__

#include <hardware/nfc.h>
#include <string.h>
#include <stdlib.h>

#include "osi.h"
#include "hal_msg.h"
#include "product.h"

#ifndef __bool_true_false_are_defined
#define __bool_true_false_are_defined
typedef enum {false, true} bool;
#endif

/***************************************
* DEVICE
***************************************/
typedef enum {
    NFC_DEV_MODE_OFF = 0,
    NFC_DEV_MODE_ON,
    NFC_DEV_MODE_BOOTLOADER,
} eNFC_DEV_MODE;

/***************************************
* States
***************************************/
typedef enum {
    HAL_STATE_INIT,
    HAL_STATE_DEINIT,
    HAL_STATE_OPEN,
    HAL_STATE_FW,
    HAL_STATE_VS,
    HAL_STATE_POSTINIT,
    HAL_STATE_SERVICE,
    HAL_STATE_GRANTED,
} eHAL_STATE;

/* FW sub-state */
#define FW_BASE_ADDRESS_N3  (0x8000)    // secure F/W download (N3)
#define FW_BASE_ADDRESS_N5  (0x5000)    // secure F/W download (N5)
#define FW_DATA_PAYLOAD_MAX (256)

typedef enum {
    FW_ENTER_UPDATE_MODE = 1,
    FW_W4_FW_RSP = 0x10,
    FW_W4_ENTER_UPDATEMODE_RSP,
    FW_W4_HASH_DATA_RSP,
    FW_W4_SIGN_DATA_RSP,
    FW_W4_UPDATE_SECT_RSP,
    FW_W4_FW_DATA_RSP,
    FW_W4_COMPLETE_RSP,
    FW_W4_NCI_RSP = 0x20,
    FW_W4_NCI_PROP_FW_CFG,
    FW_W4_NCI_RESET_RSP,
    FW_W4_NCI_INIT_RSP,
} eNFC_HAL_FW_STATE;

/* VS sub-state */
typedef enum {
    VS_INIT,
    VS_W4_GET_RFREG_VER_RSP,
    VS_W4_SET_RFREG_VER_RSP,
    VS_W4_START_RFREG_RSP,
    VS_W4_STOP_RFREG_RSP,
    VS_W4_FILE_RFREG_RSP,
    VS_W4_CFG_RFREG_RSP,
    VS_W4_PROP_RSP,
    VS_W4_COMPLETE,
} eNFC_HAL_VS_STATE;

/***************************************
* Structures
***************************************/
/* FW related */
typedef struct {
    uint8_t     major;
    uint8_t     build1;
    uint8_t     build2;
    uint8_t     target;
} tNFC_HAL_FW_VER_INFO;

typedef struct {
    unsigned char       *signature;
    unsigned long int   signatureLen;
    unsigned char       *hashCode;
    unsigned long int   hashCodeLen;
    unsigned char       *rawData;
    unsigned long int   rawDataLen;
    unsigned int        cur;
} tNFC_HAL_FW_IMAGE_INFO;

typedef struct {
    eNFC_SNFC_PRODUCTS  product;
    uint8_t             version[4];
#define NFC_HAL_BL_VER_KEYINFO  2
    uint16_t            sector_size;
    uint16_t            page_size;
    uint16_t            frame_max_size;
    uint16_t            hw_buffer_size;
    uint16_t            base_address;
} tNFC_HAL_FW_BL_INFO;

typedef struct {
    tNFC_HAL_FW_VER_INFO    image_ver;
    tNFC_HAL_FW_VER_INFO    fw_ver;
    tNFC_HAL_FW_IMAGE_INFO  image_info;
    tNFC_HAL_FW_BL_INFO     bl_info;
    eNFC_HAL_FW_STATE       state;
    int                     target_sector;  // Target sector during FW updating
    uint8_t                 seq_no;         // sequence number
#ifdef FWU_APK
    bool                    efs_fw_signcheck;
#endif
} tNFC_HAL_FW_INFO;

/* VS related */
#define RFREG_VERSION_SIZE  8
typedef struct {
    eNFC_HAL_VS_STATE   state;
    int                 rfregid;
    int                 rfregcnt;
    int                 propid;
    int                 propcnt;
    uint8_t             lastoid;
    char                rfreg_file[256];
    uint8_t             rfreg_version[RFREG_VERSION_SIZE];
    uint8_t             rfreg_img_version[RFREG_VERSION_SIZE];
    uint8_t             rfreg_number_version[4];
    uint8_t             rfreg_img_number_version[4];
    uint32_t            check_sum;
#ifdef FWU_APK
    bool                efs_rf_signcheck;
#endif
} tNFC_HAL_VS_INFO;

enum {
    CFG_SLEEP_TIMEOUT = 0,
    CFG_WAKEUP_DELAY,
    CFG_RF_REG,
    CFG_RFREG_FILE,
    CFG_NCI_PROP,
    CFG_FW_FILE,
    CFG_POWER_DRIVER,
    CFG_TRANS_DRIVER,
    CFG_TRACE_LEVEL,
    CFG_DATA_TRACE,
    CFG_UPDATE_MODE,
    CFG_FW_CLK_TYPE,
    CFG_FW_CLK_SPEED,
    CFG_FW_CLK_REQ,
    CFG_FW_BASE_ADDRESS
};

static char *cfg_name_table[] = {
    "SLEEP_TIMEOUT",
    "WAKEUP_DELAY",
    "RF_REG",
    "RFREG_FILE",
    "NCI_PROP",
    "FW_IMAGE",
    "POWER_DRIVER",
    "TRANS_DRIVER",
    "TRACE_LEVEL",
    "DATA_TRACE",
    "FW_UPDATE_MODE",
    "FW_CFG_CLK_TYPE",
    "FW_CFG_CLK_SPEED",
    "FW_CFG_CLK_REQ",
    "FW_BASE_ADDRESS",
    };

typedef struct {
    uint32_t    sleep_timeout;
    char        cfg_file[64];
#ifdef FWU_APK
    char		*fw_file;
    char		system_fw_file[64];
    char		user_fw_file[64]; /* Urgent FW update */
    char        system_rf_file[64];
    char        user_rf_file[64]; /* Urgent RF update */
#else
    char        fw_file[64];
#endif
} tNFC_HAL_CONFIG;

/* Granted related */
#define HAL_GRANT_SEND_NEXT     0x00
#define HAL_GRANT_WAIT_READ     0x01
#define HAL_GRANT_FINISH        0x02
typedef uint8_t (tNFC_HAL_GRANT_CALLBACK) (tNFC_NCI_PKT *pkt);

/* FLAGS */
#define HAL_FLAG_MASK_USING_TIMER  0x000F
#define HAL_FLAG_W4_CORE_RESET_RSP 0x0001
#define HAL_FLAG_W4_CORE_INIT_RSP  0x0002

#define HAL_FLAG_PROP_RESET        0x0010
#define HAL_FLAG_CLK_SET           0x0020

#define HAL_FLAG_MASK_ALREADY      0x0F00
#define HAL_FLAG_ALREADY_RESET     0x0100
#define HAL_FLAG_ALREADY_INIT      0x0200

#define HAL_FLAG_MASK_SYSTEM       0xF000
#define HAL_FLAG_FORCE_FW_UPDATE   0x1000
#define HAL_FLAG_NTF_TRNS_ERROR    0x2000
#define HAL_FLAG_RETRY_TRNS        0x4000

/* FW UPDATE */
#define FW_UPDATE_STATUS_NONE       0
#define FW_UPDATE_STATUS_START      1
#define FW_UPDATE_STATUS_COMPLETED  2
#define FW_UPDATE_STATUS_ERROR      3

#define FW_UPDATE_BY_DIFFER_VER     0x00
#define FW_UPDATE_BY_UPPER_VER      0x01
#define FW_UPDATE_BY_FORCE_VER      0x02

/***************************************
* Main information(context block)
***************************************/
typedef struct {
    tNFC_HAL_CONFIG             cfg;
    eHAL_STATE                  state;              /* HAL state */
    tNFC_HAL_FW_INFO            fw_info;
    tNFC_HAL_VS_INFO            vs_info;

    tOSI_TASK_HANDLER           msg_task;           /* HAL main task */
    tOSI_QUEUE_HANDLER          msg_q;
    nfc_stack_callback_t        *stack_cback;       /* Callback for HAL event */
    nfc_stack_data_callback_t   *data_cback;        /* Callback for data event */
    tNFC_NCI_PKT                *nci_last_pkt;      /* last sent package */
    tNFC_NCI_PKT                *nci_fragment_pkt;  /* Control msg flagmentation */
    tOSI_TIMER_HANDLER          nci_timer;          /* Timer for NCI message */
    tOSI_TIMER_HANDLER          sleep_timer;          /* Timer for NCI message */
    int                         trans_dev;          /* transport device */
    int                         power_dev;          /* power device */
    tOSI_QUEUE_HANDLER          nci_q;
    tNFC_HAL_GRANT_CALLBACK     *grant_cback;
    unsigned int                flag;
} tNFC_HAL_CB;

/*************************************
 * Global
 *************************************/
tNFC_HAL_CB   nfc_hal_info;

/*************************************
 * NFC HAL API prototype
 *************************************/
int nfc_hal_init(void);
void nfc_hal_deinit(void);
int nfc_hal_open(const struct nfc_nci_device *p_dev,
    nfc_stack_callback_t *p_cback, nfc_stack_data_callback_t *p_data_cback);
int nfc_hal_write(const struct nfc_nci_device *p_dev,
    uint16_t data_len, const uint8_t *p_data);
int nfc_hal_core_initialized(const struct nfc_nci_device *p_dev,
    uint8_t* p_core_init_rsp_params);
int nfc_hal_pre_discover(const struct nfc_nci_device *p_dev);
int nfc_hal_close(const struct nfc_nci_device *p_dev);
int nfc_hal_control_granted(const struct nfc_nci_device *p_dev);
int nfc_hal_power_cycle(const struct nfc_nci_device *p_dev);

/*************************************
 * NFC HAL functions.
 *************************************/
bool nfc_stack_cback(nfc_event_t event, nfc_status_t event_status);
bool nfc_data_callback(tNFC_NCI_PKT *pkt);

void nfc_hal_task(void);

/* Internal */
#define FROM_LITTLE_ARRY(data, arry, nn)    do { int n = nn; data = 0;\
        while (n--) data += ((*((arry)+n)) << (n * 8)); } while(0)
#define TO_LITTLE_ARRY(arry, data, nn)      do { int n = nn; \
        while (n--) *((arry)+n) = (uint8_t)(((data) >> (n * 8))) & 0xFF; } while(0)

void nfc_hal_grant_sm(tNFC_HAL_MSG *msg);
void nfc_hal_service_sm(tNFC_HAL_MSG *msg);
void nfc_hal_vs_sm(tNFC_HAL_MSG *msg);
void nfc_hal_fw_sm(tNFC_HAL_MSG *msg);
void nfc_hal_postinit_sm(tNFC_HAL_MSG *msg);
void nfc_hal_open_sm(tNFC_HAL_MSG *msg);

char *event_to_string(uint8_t event);
char *state_to_string(eHAL_STATE state);

// SM
void hal_update_sleep_timer(void);
int __send_to_device(uint8_t* data, size_t len);

// FW
void fw_force_update(void *param);
bool hal_fw_get_image_path(void);
int nfc_fw_cal_next_frame_length(tNFC_HAL_FW_INFO *fw, int max);
size_t nfc_fw_send_cmd(eNFC_FW_BLCMD cmd, uint8_t *param, size_t paramLen);
int nfc_fw_send_data(uint8_t *data, int len);
bool nfc_fw_getver_from_image(tNFC_HAL_FW_INFO *fw, char *file_name);
bool nfc_fw_getsecinfo_from_image(tNFC_HAL_FW_INFO *fw, char *file_name, uint8_t keyType);
bool nfc_fw_force_update_check(tNFC_HAL_FW_INFO *fw);
int fw_read_payload(tNFC_HAL_MSG *msg);

// NCI
int hal_nci_send(tNFC_NCI_PKT *pkt);
void hal_nci_send_reset(void);
void hal_nci_send_init(void);
void hal_nci_send_prop_fw_cfg(uint8_t product);
void hal_nci_send_prop_get_rfreg_ver(void);
void hal_nci_send_clearLmrt(void);
void nci_init_timeout(void *param);
bool nfc_hal_prehandler(tNFC_NCI_PKT *pkt);
int nci_read_payload(tNFC_HAL_MSG *msg);

// VS
int hal_vs_send(uint8_t *data, size_t size);
int hal_vs_nci_send(uint8_t oid, uint8_t *data, size_t size);
int hal_vs_get_prop(int n, tNFC_NCI_PKT *pkt);
size_t nfc_hal_vs_get_rfreg(int id, tNFC_NCI_PKT *pkt);
bool hal_vs_is_rfreg_file(tNFC_HAL_VS_INFO *vs);
bool hal_vs_check_rfreg_update(tNFC_HAL_VS_INFO *vs, tNFC_NCI_PKT *pkt);
bool hal_vs_rfreg_update(tNFC_HAL_VS_INFO *vs);
bool nfc_rf_getver_from_image(uint8_t *rfreg_time, char *file_name);

// TRACE
void sec_nci_analyzer(tNFC_NCI_PKT *pkt);

// M/W Patch Management
#define sec_nfc_checkVersion_moreThen(version) \
    (((fw->image_ver.major * 0x10000) + \
      (fw->image_ver.build1 * 0x100) + \
      (fw->image_ver.build2)) >= version)

#define sec_nfc_checkVersion_lessThen(version) \
    (((fw->image_ver.major * 0x10000) + \
      (fw->image_ver.build1 * 0x100) + \
      (fw->image_ver.build2)) <= version)

#define sec_nfc_checkVersion_equal(version) \
    (((fw->image_ver.major * 0x10000) + \
      (fw->image_ver.build1 * 0x100) + \
      (fw->image_ver.build2)) == version)

#define sec_nfc_checkVersion_range(version1,version2) \
    (sec_nfc_checkVersion_moreThen(version1) && \\
     sec_nfc_checkVersion_lessThen(version2))

#endif  // __NFC_SEC_HAL__
