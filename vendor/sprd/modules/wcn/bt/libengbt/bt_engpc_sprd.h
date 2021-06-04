/******************************************************************************
 *
 *  Copyright (C) 2017 Spreadtrum Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#ifndef BT_LIBENGBT_BT_ENGPC_SPRD_H_
#define BT_LIBENGBT_BT_ENGPC_SPRD_H_

#include "stdio.h"
#include "stdlib.h"

#define EUT_BT_OK ("+SPBTTEST:OK")
#define EUT_BT_ERROR ("+SPBTTEST:ERR=")
#define EUT_BT_REQ ("+SPBTTEST:EUT=")

#define EUT_BLE_OK ("+SPBLETEST:OK")
#define EUT_BLE_ERROR ("+SPBLETEST:ERR=")
#define EUT_BLE_REQ ("+SPBLETEST:EUT=")

#define ENG_BT_TX_REQ ("TX?")
#define ENG_BT_TX ("TX")
#define ENG_BT_RX_REQ ("RX?")
#define ENG_BT_RX ("RX")

#define ENG_BT_TXCH_REQ ("TXCH?")
#define ENG_BT_TXCH ("TXCH")

#define ENG_BT_RXCH_REQ ("RXCH?")
#define ENG_BT_RXCH ("RXCH")

#define ENG_BT_TXPATTERN_REQ ("TXPATTERN?")
#define ENG_BT_TXPATTERN ("TXPATTERN")

#define ENG_BT_RXPATTERN_REQ ("RXPATTERN?")
#define ENG_BT_RXPATTERN ("RXPATTERN")

#define ENG_BT_TXPKTTYPE_REQ ("TXPKTTYPE?")
#define ENG_BT_TXPKTTYPE ("TXPKTTYPE")

#define ENG_BT_RXPKTTYPE_REQ ("RXPKTTYPE?")
#define ENG_BT_RXPKTTYPE ("RXPKTTYPE")

#define ENG_BT_TXPKTLEN_REQ ("TXPKTLEN?")
#define ENG_BT_TXPKTLEN ("TXPKTLEN")

#define ENG_BT_TXPWR_REQ ("TXPWR?")
#define ENG_BT_TXPWR ("TXPWR")

#define ENG_BT_RXGAIN_REQ ("RXGAIN?")
#define ENG_BT_RXGAIN ("RXGAIN")

#define ENG_BT_ADDRESS_REQ ("TESTADDRESS?")
#define ENG_BT_ADDRESS ("TESTADDRESS")

#define ENG_BT_RXDATA_REQ ("RXDATA?")

#define ENG_BT_TESTMODE_REQ ("TESTMODE?")
#define ENG_BT_TESTMODE ("TESTMODE")

#define ENG_BT_TXPHYTYPE_REQ ("TXPHYTYPE?")
#define ENG_BT_TXPHYTYPE ("TXPHYTYPE")

#define ENG_BT_RXPHYTYPE_REQ ("RXPHYTYPE?")
#define ENG_BT_RXPHYTYPE ("RXPHYTYPE")

#define ENG_BT_RXMODINDEX_REQ ("RXMODINDEX?")
#define ENG_BT_RXMODINDEX ("RXMODINDEX")

#define ENG_BT_RFPATH_REQ ("RFPATH?")
#define ENG_BT_RFPATH ("RFPATH")

typedef enum {
    BT_MODULE_INDEX = 0,
    WIFI_MODULE_INDEX,
    GPS_MODULE_INDEX,
    BLE_MODULE_INDEX
} eut_modules;

typedef enum {
    BT_TX_REQ_INDEX,//0
    BT_TX_INDEX,
    BT_RX_REQ_INDEX,
    BT_RX_INDEX,

    /* BT TX CHANNEL */
    BT_TXCH_REQ_INDEX,
    BT_TXCH_INDEX,

    /* BT RX CHANNEL */
    BT_RXCH_REQ_INDEX,
    BT_RXCH_INDEX,

    /* BT TX PATTERN */
    BT_TXPATTERN_REQ_INDEX,
    BT_TXPATTERN_INDEX,

    /* BT RX PATTERN */
    BT_RXPATTERN_REQ_INDEX,
    BT_RXPATTERN_INDEX,

    /* TXPKTTYPE */
    BT_TXPKTTYPE_REQ_INDEX,
    BT_TXPKTTYPE_INDEX,

    /* RXPKTTYPE */
    BT_RXPKTTYPE_REQ_INDEX,
    BT_RXPKTTYPE_INDEX,

    /* TXPKTLEN */
    BT_TXPKTLEN_REQ_INDEX,
    BT_TXPKTLEN_INDEX,

    /* TXPWR */
    BT_TXPWR_REQ_INDEX,
    BT_TXPWR_INDEX,

    /* RX Gain */
    BT_RXGAIN_REQ_INDEX,
    BT_RXGAIN_INDEX,

    /* ADDRESS */
    BT_ADDRESS_REQ_INDEX,
    BT_ADDRESS_INDEX,

    /* RXDATA */
    BT_RXDATA_REQ_INDEX,

    /* TESTMODE */
    BT_TESTMODE_REQ_INDEX,
    BT_TESTMODE_INDEX,

    /* TXPHYTYPE */
    BT_TXPHYTYPE_REQ_INDEX,
    BT_TXPHYTYPE_INDEX,

    /* RXPHYTYPE */
    BT_RXPHYTYPE_REQ_INDEX,
    BT_RXPHYTYPE_INDEX,

    /* RXMODINDEX */
    BT_RXMODINDEX_REQ_INDEX,
    BT_RXMODINDEXE_INDEX,

    /* RFPATH */
    BT_RFPATH_REQ_INDEX,
    BT_RFPATH_INDEX,

} eut_cmd_enum;

struct bt_eut_cmd {
    int index;
    char *name;
};

struct bt_eut_cmd bt_eut_cmds[] = {
    /* BT TXCH*/
    { BT_TXCH_REQ_INDEX, ENG_BT_TXCH_REQ },
    { BT_TXCH_INDEX, ENG_BT_TXCH },

    /* BT RXCH*/
    { BT_RXCH_REQ_INDEX, ENG_BT_RXCH_REQ },
    { BT_RXCH_INDEX, ENG_BT_RXCH },

    /* BT TX PATTERN */
    { BT_TXPATTERN_REQ_INDEX, ENG_BT_TXPATTERN_REQ },
    { BT_TXPATTERN_INDEX, ENG_BT_TXPATTERN },

    /* BT RX PATTERN */
    { BT_RXPATTERN_REQ_INDEX, ENG_BT_RXPATTERN_REQ },
    { BT_RXPATTERN_INDEX, ENG_BT_RXPATTERN },

    /* TXPKTTYPE */
    { BT_TXPKTTYPE_REQ_INDEX, ENG_BT_TXPKTTYPE_REQ },
    { BT_TXPKTTYPE_INDEX, ENG_BT_TXPKTTYPE },

    /* RXPKTTYPE */
    { BT_RXPKTTYPE_REQ_INDEX, ENG_BT_RXPKTTYPE_REQ },
    { BT_RXPKTTYPE_INDEX, ENG_BT_RXPKTTYPE },

    /* TXPKTLEN */
    { BT_TXPKTLEN_REQ_INDEX, ENG_BT_TXPKTLEN_REQ },
    { BT_TXPKTLEN_INDEX, ENG_BT_TXPKTLEN },

    /* TXPWR */
    { BT_TXPWR_REQ_INDEX, ENG_BT_TXPWR_REQ },
    { BT_TXPWR_INDEX, ENG_BT_TXPWR },

    /* RX Gain */
    { BT_RXGAIN_REQ_INDEX, ENG_BT_RXGAIN_REQ },
    { BT_RXGAIN_INDEX, ENG_BT_RXGAIN },

    /* ADDRESS */
    { BT_ADDRESS_REQ_INDEX, ENG_BT_ADDRESS_REQ },
    { BT_ADDRESS_INDEX, ENG_BT_ADDRESS },

    /* RXDATA */
    { BT_RXDATA_REQ_INDEX, ENG_BT_RXDATA_REQ },

    /* TESTMODE */
    { BT_TESTMODE_REQ_INDEX, ENG_BT_TESTMODE_REQ },
    { BT_TESTMODE_INDEX, ENG_BT_TESTMODE },

    /* TX */
    {BT_TX_REQ_INDEX, ENG_BT_TX_REQ},
    {BT_TX_INDEX, ENG_BT_TX},

    /* RX */
    {BT_RX_REQ_INDEX, ENG_BT_RX_REQ},
    {BT_RX_INDEX, ENG_BT_RX},

    /* TXPHYTYPE */
    { BT_TXPHYTYPE_REQ_INDEX, ENG_BT_TXPHYTYPE_REQ },
    { BT_TXPHYTYPE_INDEX, ENG_BT_TXPHYTYPE },

    /* RXPHYTYPE */
    { BT_RXPHYTYPE_REQ_INDEX, ENG_BT_RXPHYTYPE_REQ },
    { BT_RXPHYTYPE_INDEX, ENG_BT_RXPHYTYPE },

    /* RXMODINDEX */
    { BT_RXMODINDEX_REQ_INDEX, ENG_BT_RXMODINDEX_REQ },
    { BT_RXMODINDEXE_INDEX, ENG_BT_RXMODINDEX },

    /* RFPATH */
    { BT_RFPATH_REQ_INDEX, ENG_BT_RFPATH_REQ },
    { BT_RFPATH_INDEX, ENG_BT_RFPATH },

};

// BT
#define BT_TESTMODE_REQ_RET ("+SPBTTEST:TESTMODE=")

#define BT_ADDRESS_REQ_RET ("+SPBTTEST:TESTADDRESS=")

#define BT_TX_CHANNEL_REQ_RET ("+SPBTTEST:TXCH=")
#define BT_RX_CHANNEL_REQ_RET ("+SPBTTEST:RXCH=")

#define BT_TX_PATTERN_REQ_RET ("+SPBTTEST:TXPATTERN=")
#define BT_RX_PATTERN_REQ_RET ("+SPBTTEST:RXPATTERN=")

#define BT_TX_PKTTYPE_REQ_RET ("+SPBTTEST:TXPKTTYPE=")
#define BT_RX_PKTTYPE_REQ_RET ("+SPBTTEST:RXPKTTYPE=")

#define BT_TXPKTLEN_REQ_RET ("+SPBTTEST:TXPKTLEN=")

#define BT_TXPWR_REQ_RET ("+SPBTTEST:TXPWR=")

#define BT_RXGAIN_REQ_RET ("+SPBTTEST:RXGAIN=")

#define BT_TX_REQ_RET ("+SPBTTEST:TX=")

#define BT_RX_REQ_RET ("+SPBTTEST:RX=")

#define BT_RXDATA_REQ_RET ("+SPBTTEST:RXDATA=")

// BLE
#define BLE_TESTMODE_REQ_RET ("+SPBLETEST:TESTMODE=")

#define BLE_ADDRESS_REQ_RET ("+SPBLETEST:TESTADDRESS=")

#define BLE_TX_CHANNEL_REQ_RET ("+SPBLETEST:TXCH=")
#define BLE_RX_CHANNEL_REQ_RET ("+SPBLETEST:RXCH=")

#define BLE_TX_PATTERN_REQ_RET ("+SPBLETEST:TXPATTERN=")
#define BLE_RX_PATTERN_REQ_RET ("+SPBLETEST:RXPATTERN=")

#define BLE_TX_PKTTYPE_REQ_RET ("+SPBLETEST:TXPKTTYPE=")
#define BLE_RX_PKTTYPE_REQ_RET ("+SPBLETEST:RXPKTTYPE=")

#define BLE_TXPKTLEN_REQ_RET ("+SPBLETEST:TXPKTLEN=")

#define BLE_TXPWR_REQ_RET ("+SPBLETEST:TXPWR=")

#define BLE_RXGAIN_REQ_RET ("+SPBLETEST:RXGAIN=")

#define BLE_TX_REQ_RET ("+SPBLETEST:TX=")

#define BLE_RX_REQ_RET ("+SPBLETEST:RX=")

#define BLE_RXDATA_REQ_RET ("+SPBLETEST:RXDATA=")

#define BLE_TX_PHY_REQ_RET ("+SPBLETEST:TXPHYTYPE=")
#define BLE_RX_PHY_REQ_RET ("+SPBLETEST:RXPHYTYPE=")

#define BLE_RXMODINDEX_REQ_RET ("+SPBLETEST:RXMODINDEX=")

#define BLE_RFPATH_REQ_RET ("+SPBLETEST:RFPATH=")

#define BT_EUT_COMMAND_MAX_LEN (128)
#define BT_EUT_COMMAND_RSP_MAX_LEN (128)

#define BT_MAC_STR_MAX_LEN (12 + 5)

/* Default Value TX */
#define BT_EUT_TX_PATTERN_DEAFULT_VALUE (4) /*PRBS9*/
#define BT_EUT_TX_CHANNEL_DEAFULT_VALUE (5)
#define BT_EUT_TX_PKTTYPE_DEAFULT_VALUE (4) /*DH1*/
#define BT_EUT_PKTLEN_DEAFULT_VALUE (27)
#define BT_EUT_POWER_TYPE_DEAFULT_VALUE (0)
#define BT_EUT_POWER_VALUE_DEAFULT_VALUE (0)
#define BT_EUT_PAC_CNT_DEAFULT_VALUE (0)

/* Default Value RX */
#define BT_EUT_RX_PATTERN_DEAFULT_VALUE (7) /* RX Pattern is only 7 */
#define BT_EUT_RX_CHANNEL_DEAFULT_VALUE (5)
#define BT_EUT_RX_PKTTYPE_DEAFULT_VALUE (4) /*DH1*/
#define BT_EUT_RX_RXGAIN_DEAFULT_VALUE (0)  /* Auto */
#define BT_EUT_RX_ADDR_DEAFULT_VALUE ("00:00:88:C0:FF:EE")

// --------------------Struct and Enum-------------------
/* Command Type */
typedef enum {
    BTEUT_CMD_TYPE_TX = 0,
    BTEUT_CMD_TYPE_RX,
    BTEUT_CMD_TYPE_INVALID = 0xff
} bteut_cmd_type;

/* TXPWR Type */
typedef enum {
    BTEUT_TXPWR_TYPE_POWER_LEVEL = 0,
    BTEUT_TXPWR_TYPE_GAIN_VALUE,
    BTEUT_TXPWR_TYPE_INVALID = 0xff
} bteut_txpwr_type;

/* Gain */
typedef enum {
    BTEUT_GAIN_MODE_AUTO = 0,
    BTEUT_GAIN_MODE_FIX,
    BTEUT_GAIN_MODE_INVALID = 0xff
} bteut_gain_mode;

/* TX mode */
typedef enum {
    BTEUT_TX_MODE_CONTINUES = 0,
    BTEUT_TX_MODE_SINGLE,
    BTEUT_TX_MODE_INVALID = 0xff
} bteut_tx_mode;

/* BT's TXRX status */
typedef enum {
    BTEUT_TXRX_STATUS_OFF = 0,
    BTEUT_TXRX_STATUS_TXING,
    BTEUT_TXRX_STATUS_RXING,
    BTEUT_TXRX_STATUS_INVALID = 0xff
} bteut_txrx_status;

/* Test Mode */
typedef enum {
    BTEUT_TESTMODE_LEAVE = 0,
    BTEUT_TESTMODE_ENTER_EUT,
    BTEUT_TESTMODE_ENTER_NONSIG,
    BTEUT_TESTMODE_INVALID = 0xff
} bteut_testmode;

typedef enum {
    BTEUT_BT_MODE_OFF = 0,
    BTEUT_BT_MODE_CLASSIC,
    BTEUT_BT_MODE_BLE,
    BTEUT_BT_MODE_INVALID = 0xff
} bteut_bt_mode;

typedef enum {
    BTEUT_EUT_RUNNING_OFF = 0,
    BTEUT_EUT_RUNNING_ON,
    BTEUT_EUT_RUNNING_INVALID = 0xff
} bteut_eut_running;

/* power */
typedef struct {
    bteut_txpwr_type power_type;
    unsigned int power_value;
} BT_EUT_POWER;

/* TX pamater */
typedef struct {
    int pattern;
    int channel;
    int pkttype;
    int pktlen;
    uint8_t phy;
    BT_EUT_POWER txpwr;
} BTEUT_TX_ELEMENT;

/* Gain */
typedef struct {
    bteut_gain_mode mode;
    unsigned int value;
} BT_EUT_GAIN;

/* RX pamater */
typedef struct {
    int pattern;
    int channel;
    int pkttype;
    uint8_t phy;
    uint8_t modulation_index;
    BT_EUT_GAIN rxgain;
    char addr[BT_MAC_STR_MAX_LEN + 1];
} BTEUT_RX_ELEMENT;

/* RX Data */
typedef struct {
    unsigned char rssi;
    unsigned int error_bits;
    unsigned int total_bits;
    unsigned int error_packets;
    unsigned int total_packets;
} BTEUT_RX_DATA;

typedef enum {
    BTEUT_LE_LEGACY_PHY,
    BTEUT_LE_1M_PHY,
    BTEUT_LE_2M_PHY,
    BTEUT_LE_Coded_Both_PHY,
    BTEUT_LE_Coded_S8_PHY,
    BTEUT_LE_Coded_S2_PHY,
} bteut_phy_type;

typedef enum {
    BTEUT_STANDARD_INDEX = 0,
    BTEUT_STABLE_INDEX,
} bteut_modulation_index;

typedef enum {
    BTEUT_RFPATH_FIRSTLY,
    BTEUT_RFPATH_SECONDARY,
    BTEUT_RFPATH_UNKNOWN = 0xFF,
} bteut_rfpath_index;

/*******************************Function Declaration*******************************/
extern int bt_testmode_set(bteut_bt_mode bt_mode, bteut_testmode testmode, char *rsp);
extern int bt_testmode_get(bteut_bt_mode bt_mode, char *rsp);

extern int bt_address_set(const char *addr, char *rsp);
extern int bt_address_get(char *rsp);

extern int bt_channel_set(bteut_cmd_type cmd_type, int ch, char *rsp);
extern int bt_channel_get(bteut_cmd_type cmd_type, char *rsp);

extern int bt_pattern_set(bteut_cmd_type cmd_type, int pattern, char *rsp);
extern int bt_pattern_get(bteut_cmd_type cmd_type, char *rsp);

extern int bt_pkttype_set(bteut_cmd_type cmd_type, int pkttype, char *rsp);
extern int bt_pkttype_get(bteut_cmd_type cmd_type, char *rsp);

extern int bt_txpktlen_set(unsigned int pktlen, char *rsp);
extern int bt_txpktlen_get(char *rsp);

extern int bt_txpwr_set(bteut_txpwr_type txpwr_type, unsigned int value, char *rsp);
extern int bt_txpwr_get(char *rsp);

extern int bt_rxgain_set(bteut_gain_mode mode, unsigned int value, char *rsp);
extern int bt_rxgain_get(char *rsp);

extern int bt_tx_set(int on_off, int tx_mode, unsigned int pktcnt, char *rsp);
extern int bt_tx_get(char *rsp);

extern int bt_rx_set(int on_off, char *rsp);
extern int bt_rx_get(char *rsp);

extern int bt_rxdata_get(char *rsp);

extern int engpc_bt_dut_mode_configure(uint8_t enable);
extern int engpc_bt_off();
extern int engpc_bt_on();
extern int engpc_bt_dut_mode_send(uint16_t opcode, uint8_t *buf, uint8_t len);
extern int engpc_bt_set_nonsig_tx_testmode(uint16_t enable, uint16_t is_le, uint16_t pattern,
                                    uint16_t channel, uint16_t pac_type, uint16_t pac_len,
                                    uint16_t power_type, uint16_t power_value, uint16_t pac_cnt);

extern int engpc_bt_set_nonsig_rx_testmode(uint16_t enable, uint16_t is_le, uint16_t pattern,
                                    uint16_t channel, uint16_t pac_type, uint16_t rx_gain,
                                    bt_bdaddr_t addr);
extern int engpc_bt_get_nonsig_rx_data(uint16_t le, char *buf, uint16_t buf_len, uint16_t *read_len);
extern int engpc_bt_dut_mode_get_rx_data(char *buf, uint16_t buf_len, uint16_t *read_len);
extern int engpc_bt_le_test_mode(uint16_t opcode, uint8_t *buf, uint8_t len);

int bt_phy_set(bteut_cmd_type cmd_type, int phy, char *rsp);
int bt_phy_get(bteut_cmd_type cmd_type, char *rsp);

int bt_rx_mod_index_set(int index, char *rsp);
int bt_rx_mod_index_get(char *rsp);

int bt_prf_path_set(int index, char *rsp);
int bt_prf_path_get(char *rsp);

#endif  // BT_LIBENGBT_BT_ENGPC_SPRD_H_
