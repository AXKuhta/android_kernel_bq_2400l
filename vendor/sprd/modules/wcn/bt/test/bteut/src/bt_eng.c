/******************************************************************************
 *
 *  Copyright (C) 2018 Spreadtrum Corporation
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

#undef LOG_TAG
#define LOG_TAG "bt_npi"


#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/prctl.h>
#include <sys/capability.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <private/android_filesystem_config.h>
#include <dlfcn.h>
#include "bt_types.h"
#include "hcidefs.h"
#include "bt_eng.h"
#include "bt_hal.h"


#define BTD(param, ...) ALOGD("ENG %s "param, __FUNCTION__, ## __VA_ARGS__)
#define BTE(param, ...) ALOGE("ENG %s "param, __FUNCTION__, ## __VA_ARGS__)

#define UNUSED(x) (void)(x)
#define CMD_RESULT_BUFFER_LEN (128)

#define RES_OK 1
#define RES_FAIL 0

static int socket_cb = -1;

static const bt_test_kit_t *bt_test_kit = NULL;

static int dut_state = 0;

static int bt_str2bd(const char *str, bt_bdaddr_t *addr) {
    unsigned char i = 0;

    for (i = 0; i < 6; i++) {
        addr->address[i] = (unsigned char)strtoul(str, (char **)&str, 16);
        str++;
    }

    return 0;
}

static int action_result_transmit(int fd, char *src, uint8_t ok) {
    int ret;
    char buf[255] = {0};

    if (fd < 0) {
        BTE("write %s to invalid fd \n", src);
        return -1;
    }

    if (!src) {
       snprintf(buf, sizeof(buf), "%s", (ok ? "OK" : "FAIL"));
    } else {
       snprintf(buf, sizeof(buf), "%s %s", (ok ? "OK" : "FAIL"), src);
    }

    ret = write(fd, buf, strlen(buf) + 1);

    if (ret != (int)(strlen(buf) + 1)) {
        BTE("fd: %d, %s, count: %d", fd, buf, ret);
        return -1;
    }

    BTD("fd: %d, %s, count: %d", fd, buf, ret);
    return 0;
}


static void eng_nonsig_test_rx_recv(int status, uint8_t rssi, uint32_t pkt_cnt,
                                      uint32_t pkt_err_cnt, uint32_t bit_cnt,
                                      uint32_t bit_err_cnt) {
    char buf[255] = { 0 };

    snprintf(buf, sizeof(buf), "rssi:%d, pkt_cnt:%d, pkt_err_cnt:%d, bit_cnt:%d, bit_err_cnt:%d",
             rssi, pkt_cnt, pkt_err_cnt, bit_cnt, bit_err_cnt);

    if (status == 0) {
        BTD("%s SUCCESS", buf);
        action_result_transmit(socket_cb, buf, RES_OK);
    } else {
        BTE("%s FAILED", buf);
        action_result_transmit(socket_cb,
            "eng_nonsig_test_rx_recv response from controller is invalid",
            RES_FAIL);
    }
}

static void eng_dut_mode_recv(uint16_t opcode, uint8_t *buf, uint8_t len) {
    char result_buf[255] = { 0 };

    BTD("opcode = %d, len = %d", opcode, len);

    if (HCI_DUT_GET_RXDATA == opcode && 2 == len) {
        uint8_t status = *buf;
        uint8_t rssi = *(buf + 1);
        BTD("status = %d, RSSI = %d", status, rssi);
        if (status == 0) {
            snprintf(result_buf, sizeof(result_buf), "HCI_DUT_GET_RXDATA status:%d, rssi:%d", rssi,
                     status);
            action_result_transmit(socket_cb, result_buf, RES_OK);
        } else {
            snprintf(result_buf, sizeof(result_buf), "HCI_DUT_GET_RXDATA status:%d, rssi:%d", rssi,
                     status);
            action_result_transmit(socket_cb, result_buf, RES_FAIL);
        }
    } else {
        BTE("leaving IN Parameters is ERROR, return.");
    }

}


static bthal_callbacks_t callbacks = {
    .size = sizeof(bthal_callbacks_t),
    .nonsig_test_rx_recv_cb = eng_nonsig_test_rx_recv,
    .dut_mode_recv_cb = eng_dut_mode_recv
};


static int action_bt_on(int argc, char **argv) {
    int ret;
    BTD();
    UNUSED(argc);
    UNUSED(argv);
    ret = bt_test_kit->enable(&callbacks);
    if (ret) {
        BTE("failed: %d", ret);
        action_result_transmit(socket_cb, "bt_status=0", RES_FAIL);
        return -1;
    }
    action_result_transmit(socket_cb, "bt_status=1", RES_OK);
    return ret;
}

static int action_bt_off(int argc, char **argv) {
    int ret;
    BTD();
    UNUSED(argc);
    UNUSED(argv);
    ret = bt_test_kit->disable();
    if (ret) {
        BTE("failed: %d", ret);
        action_result_transmit(socket_cb, "bt_status=0", RES_FAIL);
        return -1;
    }
    action_result_transmit(socket_cb, "bt_status=0", RES_OK);
    return ret;
}


static int action_dut_mode_configure(int argc, char **argv) {
    int ret = -1, mode = -1;

    BTD();
    UNUSED(argc);

    argv++;
    argc--;


    if ((*argv)[0] == '0')
        mode = 0;
    else if ((*argv)[0] == '1')
        mode = 1;

    if (mode == dut_state) {
        action_result_transmit(socket_cb,
            mode ? "dut already enable" : "dut already disable",
            RES_OK);
        return 0;
    }


    if (mode) {
        if (!bt_test_kit->is_enable())
            ret = bt_test_kit->enable(&callbacks);
        if (ret) {
            action_result_transmit(socket_cb, "enable bt failed", RES_FAIL);
            return -1;
        }
        ret = bt_test_kit->dut_mode_configure(1);
        if (ret) {
            action_result_transmit(socket_cb, "enter dut failed", RES_FAIL);
            return -1;
        }
    } else {
        ret = bt_test_kit->dut_mode_configure(0);
        if (ret) {
            action_result_transmit(socket_cb, "exit dut failed", RES_FAIL);
            return -1;
        }
        ret = bt_test_kit->disable();
        if (ret) {
            action_result_transmit(socket_cb, "disable bt failed", RES_FAIL);
            return -1;
        }
    }

    action_result_transmit(socket_cb,
        mode ? "enter dut ok" : "exit dut ok",
        RES_OK);
    dut_state = mode;
    return 0;
}

static int action_get_dut_state(int argc, char **argv) {

    char buf[255] = { 0 };

    BTD();
    UNUSED(argc);
    UNUSED(argv);

    if (dut_state)
            snprintf(buf, sizeof(buf), "return value: %d", bt_test_kit->is_enable());
    else
            snprintf(buf, sizeof(buf), "return value: %d", 0);

    action_result_transmit(socket_cb, buf, RES_OK);
    return 0;
}

static int action_set_nonsig_tx_testmode(int argc, char **argv)
{
    int ret;
    uint32_t data[10] = { 0 };
    uint32_t *p = data;
    int m, i;
    int power_gain_base = 0xce00;
    int power_level_base[2] = { 0x1012, 0x0000 };

    UNUSED(argc);
    UNUSED(argv);

    if (argc != 10) {
        BTE("parameter invalid.");
       return -1;
    } else {
        for (i = 0; i < argc - 1; i++)
            data[i] = atoi(argv[i + 1]);
    }

    BTD();

    uint32_t enable = *p++;
    uint32_t is_le = *p++;
    uint32_t pattern = *p++;
    uint32_t channel = *p++;
    uint32_t pac_type = *p++;
    uint32_t pac_len = *p++;
    uint32_t power_type = *p++;
    uint32_t power_value = *p++;
    uint32_t pac_cnt = *p;

    BTD("enable     = %d", enable);
    BTD("is_le      = %d", is_le);
    BTD("pattern    = %d", pattern);
    BTD("channel    = %d", channel);
    BTD("pac_type   = 0x%x", pac_type);
    BTD("pac_len    = %d", pac_len);
    BTD("power_type = %d", power_type);
    BTD("power_value= 0x%x", power_value);
    BTD("pac_cnt    = %d", pac_cnt);

    ret = bt_test_kit->set_nonsig_tx_testmode(enable, is_le, pattern, channel, pac_type, pac_len, power_type,
                                 power_value, pac_cnt);

    if (ret == 0) {
        BTD("SUCCESS");
        action_result_transmit(socket_cb, "set_nosig_tx_testmode_ok ok", RES_OK);
    } else {
        BTE("FAILED");
        action_result_transmit(socket_cb, "set_nosig_tx_testmode_ok fail", RES_FAIL);
        ret = -1;
    }

    return ret;
}

static int  action_set_nonsig_rx_testmode(int argc, char **argv) {
    int ret;
    uint32_t data[7] = { 0 };
    uint32_t *p = data;
    bt_bdaddr_t addr;
    char *paddr;
    int m, i;


    if (argc != 8) {
            BTE("parameter invalid");
            return -1;
        } else {
            for (i = 0; i < argc - 1; i++)
                data[i] = atoi(argv[i + 1]);
        }

    paddr = argv[argc - 1];
    memset(&addr, 0, sizeof(bt_bdaddr_t));

    uint32_t enable = *p++;
    uint32_t is_le = *p++;
    uint32_t pattern = *p++;
    uint32_t channel = *p++;
    uint32_t pac_type = *p++;
    uint32_t rx_gain = *p;

    BTD();
    bt_str2bd(paddr, &addr);

    BTD("enable     = %d", enable);
    BTD("is_le      = %d", is_le);
    BTD("pattern    = %d", pattern);
    BTD("channel    = %d", channel);
    BTD("pac_type   = 0x%x", pac_type);
    BTD("rx_gain    = %d", rx_gain);
    BTD("addr : %02x:%02x:%02x:%02x:%02x:%02x", addr.address[0], addr.address[1],
               addr.address[2], addr.address[3], addr.address[4], addr.address[5]);

    /* enable: 0 NONSIG_RX_DISABLE      1 NONSIG_RX_ENABLE*/
    ret = bt_test_kit->set_nonsig_rx_testmode(enable, is_le, pattern, channel, pac_type, rx_gain, &addr);


    if (ret == 0) {
        BTD("SUCCESS");
        action_result_transmit(socket_cb, "set_nosig_rx_testmode ok", RES_OK);
    } else {
        BTE("FAILED");
        action_result_transmit(socket_cb, "set_nosig_rx_testmode fail", RES_FAIL);
        ret = -1;
    }

    return ret;
}



static int action_le_set_nonsig_recv_data(int argc, char **argv) {
    int ret;

    UNUSED(argc);
    UNUSED(argv);

    ret = bt_test_kit->get_nonsig_rx_data(1);
    if (ret == 0) {
        BTD("SUCCESS");
    } else {
        BTE("FAILED");
        ret = -1;
    }

    return ret;
}


static int action_classic_set_nonsig_recv_data(int argc, char **argv) {
    int ret;

    UNUSED(argc);
    UNUSED(argv);

    ret = bt_test_kit->get_nonsig_rx_data(0);
    if (ret == 0) {
        BTD("SUCCESS");
    } else {
        BTE("FAILED");
        ret = -1;
    }

    return ret;
}


static int action_dut_mode_send(int argc, char **argv) {
    int ret;

    UNUSED(argc);
    UNUSED(argv);

    ret = bt_test_kit->dut_mode_send(HCI_DUT_GET_RXDATA, NULL, 0);
    if (ret == 0) {
        BTD("SUCCESS");
    } else {
        BTE("FAILED");
        ret = -1;
    }

    return ret;
}

/*
* cmd: le_enhanced_transmitter_test
* Parameter 1: TX_Channel, Size: 1 byte, Range: 0x00 - 0x27, Display: 2402 MHz to 2480 MHz
* Parameter 2: Length_Of_Test_Data, Size: 1byte, Range: 0x00 - 0xFF, Display: XX length
* Parameter 3: Packet_Payload, Size: 1 byte, Range: 0x00 - 0x07, Display: {11111111100000111101, 11110000,
*     10101010, PRBS15, 11111111, 00000000, 00001111, 01010101}
* Parameter 4: PHY, Size 1byte, Range: 0x01 - 0x04, Display: {LE 1M, LE 2M, LE Coded S8, LE Coded S2}
*/
static int action_le_enhanced_transmitter_test(int argc, char **argv)
{
    int ret = 0;
    uint32_t data[10] = { 0 };
    uint32_t *p = data;
    int i;


    if (argc != 5) {
        BTE("parameter invalid.");
       return -1;
    } else {
        for (i = 0; i < argc - 1; i++)
            data[i] = atoi(argv[i + 1]);
    }

    BTD();

    uint32_t channel = *p++;
    uint32_t length = *p++;
    uint32_t payload = *p++;
    uint32_t phy = *p++;

    BTD("channel     = %d", channel);
    BTD("is_le      = %d", length);
    BTD("payload    = %d", payload);
    BTD("phy    = %d", phy);


    ret = engpc_bt_le_enhanced_transmitter(channel & 0xFF, length & 0xFF, payload & 0xFF, phy & 0xFF);


    if (ret == 0) {
        BTD("SUCCESS");
        action_result_transmit(socket_cb, "le_enhanced_transmitter_test ok", RES_OK);
    } else {
        BTE("FAILED");
        action_result_transmit(socket_cb, "le_enhanced_transmitter_test fail", RES_FAIL);
        ret = -1;
    }

    return ret;
}


/*
* cmd: le_enhanced_receiver_test
* Parameter 1: RX_Channel, Size: 1 byte, Range: 0x00 - 0x27, Display: 2402 MHz to 2480 MHz
* Parameter 2: PHY, Size: 1byte, Range: 0x01 - 0x03, Display: {LE 1M, LE 2M, LE Coded}
* Parameter 3: Modulation_Index, Size: 1byte, Range: 0x00 - 0x01, Display: {standard, stable}
*/
static int action_le_enhanced_receiver_test(int argc, char **argv)
{
    int ret = 0;
    uint32_t data[10] = { 0 };
    uint32_t *p = data;
    int i;

    if (argc != 4) {
        BTE("parameter invalid.");
       return -1;
    } else {
        for (i = 0; i < argc - 1; i++)
            data[i] = atoi(argv[i + 1]);
    }

    BTD();

    uint32_t channel = *p++;
    uint32_t phy = *p++;
    uint32_t index = *p++;

    BTD("channel     = %d", channel);
    BTD("phy    = %d", phy);
    BTD("index    = %d", index);

    ret = engpc_bt_le_enhanced_receiver(channel & 0xFF, phy & 0xFF, index & 0xFF);

    if (ret == 0) {
        BTD("SUCCESS");
        action_result_transmit(socket_cb, "le_enhanced_receiver_test ok", RES_OK);
    } else {
        BTE("FAILED");
        action_result_transmit(socket_cb, "le_enhanced_receiver_test fail", RES_FAIL);
        ret = -1;
    }

    return ret;
}

/*
* cmd: le_test_end
*/
static int action_le_test_end(int argc, char **argv)
{
    int ret = 0;
    uint32_t data[10] = { 0 };
    uint32_t *p = data;
    int i;

    UNUSED(argc);
    UNUSED(argv);


    BTD();

    ret = engpc_bt_le_test_end();

    if (ret == 0) {
        BTD("SUCCESS");
        action_result_transmit(socket_cb, "le_test_end ok", RES_OK);
    } else {
        BTE("FAILED");
        action_result_transmit(socket_cb, "le_test_end fail", RES_FAIL);
        ret = -1;
    }

    return ret;
}


/*
* cmd: set_rf_path
* Parameter 1: Path, Size: 1 byte, Range: 0x01 - 0x02, Display: {1, 2}
*/
static int action_set_rf_path(int argc, char **argv)
{
    int ret = 0;
    uint32_t data[10] = { 0 };
    uint32_t *p = data;
    int i;

    argv++;
    argc--;

    if (argc != 1) {
        BTE("parameter invalid.");
       return -1;
    } else {
        for (i = 0; i < argc - 1; i++)
            data[i] = atoi(argv[i + 1]);
    }

    BTD();

    uint32_t path = *p++;

    BTD("path     = %d", path);

    ret = engpc_bt_set_rf_path(path & 0xFF);

    if (ret == 0) {
        BTD("SUCCESS");
        action_result_transmit(socket_cb, "set_rf_path ok", RES_OK);
    } else {
        BTE("FAILED");
        action_result_transmit(socket_cb, "set_rf_path fail", RES_FAIL);
        ret = -1;
    }

    return ret;
}


btif_eng_t btif_eng[] = {
    {CMD_BT_ON_STR, action_bt_on},
    {CMD_BT_OFF_STR, action_bt_off},
    {CMD_DUT_MODE_STR, action_dut_mode_configure},
    {CMD_DUT_STATUS_STR, action_get_dut_state},
    {CMD_DUT_RECV_DATA, action_dut_mode_send},
    {CMD_NONSIG_TX_MODE_STR, action_set_nonsig_tx_testmode},
    {CMD_NONSIG_RX_MODE_STR, action_set_nonsig_rx_testmode},
    {CMD_NONSIG_RX_RECV_DATA_STR, action_classic_set_nonsig_recv_data},
    {CMD_NONSIG_RX_RECV_DATA_LE_STR, action_le_set_nonsig_recv_data},
    {CMD_LE_ENHANCED_TRANSMITTER_TEST_STR, action_le_enhanced_transmitter_test},
    {CMD_LE_ENHANCED_REVEIVER_TEST_STR, action_le_enhanced_receiver_test},
    {CMD_LE_TEST_END_STR, action_le_test_end},
    {CMD_SET_RF_PATH_STR, action_set_rf_path},
    {NULL, NULL}
};



int bt_runcommand(int client_fd, int argc, char **argv) {
    char result_buf[CMD_RESULT_BUFFER_LEN];
    int ret = -1;
    int i = 0;
    int m = 0;
    uint32_t data[10] = { 0 };
    btif_eng_t *p = &btif_eng[0];

    memset(result_buf, 0, sizeof(result_buf));
    socket_cb = client_fd;

    for (i = 0; i < argc; i++) {
        BTD("arg[%d]: %s", i, *(argv + i));
    }

    if (!bt_test_kit) {
        bt_test_kit = bt_test_kit_get_interface();
        if (!bt_test_kit) {
            BTD("get bt_test_kit failed");
            return -1;
        }
    }

    while(p->cmd) {
        if (!strncmp(*argv, p->cmd, strlen(*argv))) {
            break;
        }
        p++;
    }

    if (!p->cmd) {
        BTE("rcv cmd is invalid.");
        fprintf(stderr, "rcv cmd is invalid.\n");
        action_result_transmit(client_fd, result_buf, 0);
    }

    BTD("Rcv cmd: %s", p->cmd);

    if (p->func)
        ret = p->func(argc, argv);
    else
        BTE("%s func not found", p->cmd);

    return ret;
}


int engpc_bt_on() {
    if (!bt_test_kit) {
        bt_test_kit = bt_test_kit_get_interface();
        if (!bt_test_kit) {
            BTD("get bt_test_kit failed");
            return -1;
        }
    }
    return bt_test_kit->enable(&callbacks);
}

int engpc_bt_off() {
    if (!bt_test_kit) {
        bt_test_kit = bt_test_kit_get_interface();
        if (!bt_test_kit) {
            BTD("get bt_test_kit failed");
            return -1;
        }
    }
    return bt_test_kit->disable();
}

int engpc_bt_dut_mode_configure(uint8_t enable) {
    return bt_test_kit->dut_mode_configure(enable);
}

int engpc_bt_set_nonsig_tx_testmode(uint16_t enable, uint16_t is_le, uint16_t pattern,
                                    uint16_t channel, uint16_t pac_type, uint16_t pac_len,
                                    uint16_t power_type, uint16_t power_value, uint16_t pac_cnt) {
    int ret;
    int power_gain_base = 0xce00;
    int power_level_base[2] = { 0x1012, 0x0000 };

    BTD();

    BTD("enable     = %d", enable);
    BTD("is_le      = %d", is_le);
    BTD("pattern    = %d", pattern);
    BTD("channel    = %d", channel);
    BTD("pac_type   = 0x%x", pac_type);
    BTD("pac_len    = %d", pac_len);
    BTD("power_type = %d", power_type);
    BTD("power_value= 0x%x", power_value);
    BTD("pac_cnt    = %d", pac_cnt);

    ret = bt_test_kit->set_nonsig_tx_testmode(enable, is_le, pattern, channel, pac_type, pac_len, power_type,
                                 power_value, pac_cnt);
    if (ret != 0) {
        BTE("ret: %d", ret);
        return -1;
    }

    return 0;
}


int engpc_bt_set_nonsig_rx_testmode(uint16_t enable, uint16_t is_le, uint16_t pattern,
                                    uint16_t channel, uint16_t pac_type, uint16_t rx_gain,
                                    bt_bdaddr_t *addr) {
    int ret;

    BTD();

    BTD("enable     = %d", enable);
    BTD("is_le      = %d", is_le);
    BTD("pattern    = %d", pattern);
    BTD("channel    = %d", channel);
    BTD("pac_type   = 0x%x", pac_type);
    BTD("rx_gain    = %d", rx_gain);
    BTD("addr : %02x:%02x:%02x:%02x:%02x:%02x", addr->address[0], addr->address[1],
               addr->address[2], addr->address[3], addr->address[4], addr->address[5]);

    /* enable: 0 NONSIG_RX_DISABLE      1 NONSIG_RX_ENABLE*/
    ret = bt_test_kit->set_nonsig_rx_testmode(enable, is_le, pattern, channel, pac_type, rx_gain, addr);

    if (ret != 0) {
        BTE("ret: %d", ret);
        return -1;
    }

    return 0;
}

int engpc_bt_dut_mode_send(uint16_t opcode, uint8_t *buf, uint8_t len) {
    if (opcode == HCI_DUT_SET_TXPWR || opcode == HCI_DUT_SET_RXGIAN) {
        return bt_test_kit->dut_mode_send(opcode, buf, len);
    } else {
        return -1;
    }
}


int engpc_bt_get_nonsig_rx_data(uint16_t le, char *buf, uint16_t buf_len, uint16_t *read_len) {
    int ret = -1;
    int sock_fd[2] = { 0 };
    char *argv[1] = { 0 };
    int argc = 1;
    int n = 0;
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sock_fd) >= 0) {
        if (!le) {
            argv[0] = strdup(CMD_NONSIG_RX_RECV_DATA_STR);
        } else {
            argv[0] = strdup(CMD_NONSIG_RX_RECV_DATA_LE_STR);
        }
        ret = bt_runcommand(sock_fd[0], argc, (char **)argv);
        free(argv[0]);
        for (;;) {
            memset(buf, 0, buf_len);
            BTD("waiting for rx_data");
            n = read(sock_fd[1], buf, buf_len);
            BTD("get %d bytes %s\n", n, buf);
            if (n > 0) {
                *read_len = n;
                break;
            }
        }
        close(sock_fd[0]);
        close(sock_fd[1]);
    } else {
        BTE("ERR:create socket pair failed: %d %s\n", errno, strerror(errno));
    }
    return ret;
}

int engpc_bt_le_enhanced_receiver(uint8_t channel, uint8_t phy, uint8_t modulation_index)
{
    int ret;

    BTD();

    BTD("channel     = %u", channel);
    BTD("phy      = %u", phy);
    BTD("modulation_index    = %u", modulation_index);

    ret = bt_test_kit->le_enhanced_receiver(channel, phy, modulation_index);

    if (ret != 0) {
        BTE("ret: %d", ret);
        return -1;
    }
    return 0;
}

int engpc_bt_le_enhanced_transmitter(uint8_t channel, uint8_t length, uint8_t payload, uint8_t phy)
{
    int ret;

    BTD();

    BTD("channel     = %u", channel);
    BTD("length      = %u", length);
    BTD("payload    = %u", payload);
    BTD("phy    = %u", phy);

    ret = bt_test_kit->le_enhanced_transmitter(channel, length, payload, phy);

    if (ret != 0) {
        BTE("ret: %d", ret);
        return -1;
    }
    return 0;
}

int engpc_bt_le_test_end(void)
{
    int ret;

    BTD();

    ret = bt_test_kit->le_test_end();

    if (ret != 0) {
        BTE("ret: %d", ret);
        return -1;
    }
    return 0;
}

int engpc_bt_set_rf_path(uint8_t path)
{
    int ret;

    BTD("path: %u", path);

    ret = bt_test_kit->set_rf_path(path);

    if (ret != 0) {
        BTE("ret: %d", ret);
        return -1;
    }
    return 0;

}

int engpc_bt_get_rf_path(void)
{
    BTD();

    return bt_test_kit->get_rf_path();
}


int engpc_bt_dut_mode_get_rx_data(char *buf, uint16_t buf_len, uint16_t *read_len) {
    int ret = -1;

    int sock_fd[2] = { 0 };
    char *argv[1] = { 0 };
    int argc = 1;
    int n = 0;
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sock_fd) >= 0) {
        argv[0] = strdup(CMD_DUT_RECV_DATA);
        ret = bt_runcommand(sock_fd[0], argc, (char **)argv);
        free(argv[0]);
        for (;;) {
            memset(buf, 0, buf_len);
            BTD("waiting for rx_data");
            n = read(sock_fd[1], buf, buf_len);
            BTD("get %d bytes %s\n", n, buf);
            if (n > 0) {
                *read_len = n;
                break;
            }
        }
        close(sock_fd[0]);
        close(sock_fd[1]);
    } else {
        BTE("ERR:create socket pair failed: %d %s\n", errno, strerror(errno));
    }
    return ret;
}
