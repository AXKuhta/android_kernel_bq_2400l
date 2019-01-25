/*
 * aw889x.c   aw889x codec module
 *
 * Version: v1.0.1
 *
 * Copyright (c) 2017 AWINIC Technology CO., LTD
 *
 *  Author: Nick Li <liweilei@awinic.com.cn>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/i2c.h>
#include <linux/debugfs.h>
#include <linux/version.h>
#include <linux/input.h>
#include <sound/tlv.h>
#include "aw889x.h"
#include "aw889x_reg.h"
#include "aw889x_reg_table.h"

/******************************************************
 *
 * Marco
 *
 ******************************************************/
#define AW889X_I2C_NAME "aw889x_smartpa"

#define AW889X_VERSION "v1.0.1"

#define AW889X_RATES SNDRV_PCM_RATE_8000_48000
#define AW889X_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | \
                        SNDRV_PCM_FMTBIT_S24_LE | \
                        SNDRV_PCM_FMTBIT_S32_LE)

//#define AWINIC_I2C_REGMAP

#define AW_I2C_RETRIES 5
#define AW_I2C_RETRY_DELAY 5  // 5ms
#define AW_READ_CHIPID_RETRIES 5
#define AW_READ_CHIPID_RETRY_DELAY 5

#define AW889X_MAX_DSP_START_TRY_COUNT    10


static int aw889x_spk_control = 0;
static int aw889x_rcv_control = 0;
static bool no_chip;

#define AW889X_FW_NAME_MAX     64
#define AW889X_MAX_FIRMWARE_LOAD_CNT 20
static char aw889x_fw_name[][AW889X_FW_NAME_MAX] = {
    {"aw889x_fw.bin"},
    {"aw889x_fw_d.bin"},
    {"aw889x_fw_e.bin"},
};
static char *aw889x_cfg_name = "aw889x_cfg.bin";

 /******************************************************
 *
 * aw889x i2c write/read
 *
 ******************************************************/
#ifndef AWINIC_I2C_REGMAP
static int i2c_write(struct aw889x *aw889x,
        unsigned char addr, unsigned int reg_data)
{
    int ret;
    u8 wbuf[512] = {0};

    struct i2c_msg msgs[] = {
        {
            .addr    = aw889x->i2c->addr,
            .flags    = 0,
            .len    = 3,
            .buf    = wbuf,
        },
    };

    wbuf[0] = addr;
    wbuf[1] = (unsigned char)((reg_data & 0xff00)>>8);
    wbuf[2] = (unsigned char)(reg_data & 0x00ff);

    ret = i2c_transfer(aw889x->i2c->adapter, msgs, 1);
    if (ret < 0) {
        pr_err("%s: i2c write error: %d\n", __func__, ret);
    }

    return ret;
}

static int i2c_read(struct aw889x *aw889x,
        unsigned char addr, unsigned int *reg_data)
{
    int ret;
    unsigned char rbuf[512] = {0};
    unsigned int get_data;

    struct i2c_msg msgs[] = {
        {
            .addr    = aw889x->i2c->addr,
            .flags    = 0,
            .len    = 1,
            .buf    = &addr,
        },
        {
            .addr    = aw889x->i2c->addr,
            .flags    = I2C_M_RD,
            .len    = 2,
            .buf    = rbuf,
        },
    };

    ret = i2c_transfer(aw889x->i2c->adapter, msgs, 2);
    if (ret < 0) {
        pr_err("%s: i2c read error: %d\n", __func__, ret);
        return ret;
    }

    get_data = (unsigned int)(rbuf[0] & 0x00ff);
    get_data <<= 8;
    get_data |= (unsigned int)rbuf[1];

    *reg_data = get_data;

    return ret;
}
#endif

 static int aw889x_i2c_write(struct aw889x *aw889x, 
         unsigned char reg_addr, unsigned int reg_data)
{
    int ret = -1;
    unsigned char cnt = 0;

    while(cnt < AW_I2C_RETRIES) {
#ifdef AWINIC_I2C_REGMAP
        ret = regmap_write(aw889x->regmap, reg_addr, reg_data);
        if(ret < 0) {
            pr_err("%s: regmap_write cnt=%d error=%d\n", __func__, cnt, ret);
        } else {
            break;
        }
#else
        ret = i2c_write(aw889x, reg_addr, reg_data);
        if(ret < 0) {
            pr_err("%s: i2c_write cnt=%d error=%d\n", __func__, cnt, ret);
        } else {
            break;
        }
#endif
        cnt ++;
    }

    return ret;
}

 static int aw889x_i2c_read(struct aw889x *aw889x, 
        unsigned char reg_addr, unsigned int *reg_data)
{
    int ret = -1;
    unsigned char cnt = 0;

    while(cnt < AW_I2C_RETRIES) {
#ifdef AWINIC_I2C_REGMAP
        ret = regmap_read(aw889x->regmap, reg_addr, reg_data);
        if(ret < 0) {
            pr_err("%s: regmap_read cnt=%d error=%d\n", __func__, cnt, ret);
        } else {
            break;
        }
#else
        ret = i2c_read(aw889x, reg_addr, reg_data);
        if(ret < 0) {
            pr_err("%s: i2c_read cnt=%d error=%d\n", __func__, cnt, ret);
        } else {
            break;
        }
#endif
        cnt ++;
    }

    return ret;
}

/******************************************************
 *
 * aw889x control
 *
 ******************************************************/
static void aw889x_run_mute(struct aw889x *aw889x, bool mute)
{
    unsigned int reg_val = 0;

    pr_debug("%s enter\n", __func__);

    aw889x_i2c_read(aw889x, AW889X_REG_PWMCTRL, &reg_val);
    if(mute) {
        reg_val |= AW889X_BIT_PWMCTRL_HMUTE;
    } else {
        reg_val &= (~AW889X_BIT_PWMCTRL_HMUTE);
    }
    aw889x_i2c_write(aw889x, AW889X_REG_PWMCTRL, reg_val);
}

static void aw889x_run_pwd(struct aw889x *aw889x, bool pwd)
{
    unsigned int reg_val = 0;

    pr_debug("%s enter\n", __func__);

    aw889x_i2c_read(aw889x, AW889X_REG_SYSCTRL, &reg_val);
    if(pwd) {
        reg_val |= AW889X_BIT_SYSCTRL_PWDN;
    } else {
        reg_val &= (~AW889X_BIT_SYSCTRL_PWDN);
    }
    aw889x_i2c_write(aw889x, AW889X_REG_SYSCTRL, reg_val);
}

static void aw889x_dsp_enable(struct aw889x *aw889x, bool dsp)
{
    unsigned int reg_val = 0;

    pr_debug("%s enter\n", __func__);

    aw889x_i2c_read(aw889x, AW889X_REG_SYSCTRL, &reg_val);
    if(dsp) {
        reg_val &= (~AW889X_BIT_SYSCTRL_DSPBY);
    } else {
        reg_val |= AW889X_BIT_SYSCTRL_DSPBY;
    }
    aw889x_i2c_write(aw889x, AW889X_REG_SYSCTRL, reg_val);
}

static void aw889x_init(struct aw889x *aw889x)
{
    unsigned int reg_val = 0;

    pr_debug("%s enter\n", __func__);

    // I2S Setting
    aw889x_i2c_read(aw889x, AW889X_REG_I2SCTRL, &reg_val);
    reg_val &= (~(AW889X_BIT_I2SCTRL_CHS_MASK |
            AW889X_BIT_I2SCTRL_MD_MASK |
            AW889X_BIT_I2SCTRL_FMS_MASK |
            AW889X_BIT_I2SCTRL_BCK_MASK |
            AW889X_BIT_I2SCTRL_SR_MASK));
    reg_val |= AW889X_BIT_I2SCTRL_CHS_MONO |
            AW889X_BIT_I2SCTRL_MD_STD |
            AW889X_BIT_I2SCTRL_FMS_16BIT | //AW889X_BIT_I2SCTRL_FMS_20BIT | //yht changed this as FAE want to
            AW889X_BIT_I2SCTRL_BCK_32FS | //AW889X_BIT_I2SCTRL_BCK_64FS | //yht changed this as FAE want to
            AW889X_BIT_I2SCTRL_SR_48K;
    aw889x_i2c_write(aw889x, AW889X_REG_I2SCTRL, reg_val);

    // smartpa config
    aw889x_i2c_read(aw889x, AW889X_REG_DBGCTRL, &reg_val);
    reg_val &= (~AW889X_BIT_DBGCTRL_PDUVL);
    aw889x_i2c_write(aw889x, AW889X_REG_DBGCTRL, reg_val);

    aw889x_i2c_read(aw889x, AW889X_REG_GENCTRL, &reg_val);
    reg_val |= AW889X_BIT_GENCTRL_CLIP_BP;
    reg_val &= (~AW889X_BIT_GENCTRL_BSTILIMIT_MASK);
    reg_val |= AW889X_BIT_GENCTRL_BSTILIMIT_3A;
    reg_val &= (~AW889X_BIT_GENCTRL_BSTVOUT_MASK);
    reg_val |= AW889X_BIT_GENCTRL_BSTVOUT_5P25V;
    aw889x_i2c_write(aw889x, AW889X_REG_GENCTRL, reg_val);

    aw889x_i2c_read(aw889x, AW889X_REG_BSTCTRL, &reg_val);
    reg_val &= (~AW889X_BIT_BSTCTRL_VMAX_MASK);
    reg_val |= AW889X_BIT_BSTCTRL_VMAX_1P9V;
    reg_val &= (~AW889X_BIT_BSTCTRL_RTH_MASK);
    reg_val |= 0x00F0;
    reg_val &= (~AW889X_BIT_BSTCTRL_ATH_MASK);
    reg_val |= 0x000E;
#ifdef BST_PASS_THROUGH
    reg_val &= (~AW889X_BIT_BSTCTRL_MODE_MASK);
    reg_val |= AW889X_BIT_BSTCTRL_MODE_TRSP;
#endif
    aw889x_i2c_write(aw889x, AW889X_REG_BSTCTRL, reg_val);

    aw889x_i2c_read(aw889x, AW889X_REG_PLLCTRL1, &reg_val);
    reg_val &= (~AW889X_BIT_PLLCTRL1_PFD_DLY_MASK);
    reg_val |= AW889X_BIT_PLLCTRL1_PFD_DLY_40NS;
    aw889x_i2c_write(aw889x, AW889X_REG_PLLCTRL1, reg_val);

    aw889x_i2c_read(aw889x, AW889X_REG_AMPDBG1, &reg_val);
    reg_val |= (AW889X_BIT_AMPDBG1_VCOM);
    reg_val &= (~AW889X_BIT_AMPDBG1_IPWM_20UA);
    reg_val &= (~AW889X_BIT_AMPDBG1_RFB_MASK);
    reg_val |= 0x0002;
    aw889x_i2c_write(aw889x, AW889X_REG_AMPDBG1, reg_val);

    aw889x_i2c_read(aw889x, AW889X_REG_AMPDBG2, &reg_val);
    reg_val &= (~AW889X_BIT_AMPDBG2_AMP_OCPF);
    reg_val &= (~AW889X_BIT_AMPDBG2_SR_CTRL_MASK);
    reg_val |= AW889X_BIT_AMPDBG2_SR_CTRL_10NS;
    reg_val |= AW889X_BIT_AMPDBG2_OCSWD;
    reg_val &= (~AW889X_BIT_AMPDBG2_OCDT_MASK);
    reg_val |= AW889X_BIT_AMPDBG2_OCDT_50NS;
    reg_val &= (~AW889X_BIT_AMPDBG2_IOC_MASK);
    reg_val |= AW889X_BIT_AMPDBG2_IOC_2P3A;
    aw889x_i2c_write(aw889x, AW889X_REG_AMPDBG2, reg_val);

    // I2S Enable, PLL Working, DSP Bypass
    aw889x_i2c_read(aw889x, AW889X_REG_SYSCTRL, &reg_val);
    reg_val &= (~AW889X_BIT_SYSCTRL_PWDN);
    reg_val &= (~AW889X_BIT_SYSCTRL_BSTPD);
    reg_val |= AW889X_BIT_SYSCTRL_INT_OD |
            AW889X_BIT_SYSCTRL_I2SEN |
            AW889X_BIT_SYSCTRL_DSPBY;
    aw889x_i2c_write(aw889x, AW889X_REG_SYSCTRL, reg_val);
}
static void aw889x_spk_rcv_mode(struct aw889x *aw889x)
{
    unsigned int reg_val = 0;

    pr_debug("%s spk_rcv=%d\n", __func__, aw889x->spk_rcv_mode);

    if(aw889x->spk_rcv_mode == AW889X_SPEAKER_MODE) {
        // RFB IDAC = 6V
        aw889x_i2c_read(aw889x, AW889X_REG_AMPDBG1, &reg_val);
        reg_val |= AW889X_BIT_AMPDBG1_OPD;
        reg_val &= (~AW889X_BIT_AMPDBG1_IPWM_20UA);
        reg_val &= (~AW889X_BIT_AMPDBG1_RFB_MASK);
        reg_val |= 0x0002;
        aw889x_i2c_write(aw889x, AW889X_REG_AMPDBG1, reg_val);

        // Speaker Mode
        aw889x_i2c_read(aw889x, AW889X_REG_SYSCTRL, &reg_val);
        reg_val &= (~AW889X_BIT_SYSCTRL_RCV_MODE);
        aw889x_i2c_write(aw889x, AW889X_REG_SYSCTRL, reg_val);

    } else if(aw889x->spk_rcv_mode == AW889X_RECEIVER_MODE){
        // RFB IDAC = 4V
        aw889x_i2c_read(aw889x, AW889X_REG_AMPDBG1, &reg_val);
        reg_val |= AW889X_BIT_AMPDBG1_OPD;
        reg_val |= (AW889X_BIT_AMPDBG1_IPWM_20UA);
        reg_val &= (~AW889X_BIT_AMPDBG1_RFB_MASK);
        reg_val |= 0x000B;
        aw889x_i2c_write(aw889x, AW889X_REG_AMPDBG1, reg_val);

        // Receiver Mode
        aw889x_i2c_read(aw889x, AW889X_REG_SYSCTRL, &reg_val);
        reg_val |= AW889X_BIT_SYSCTRL_RCV_MODE;
        aw889x_i2c_write(aw889x, AW889X_REG_SYSCTRL, reg_val);
    } else {
    }
}

static void aw889x_tx_cfg(struct aw889x *aw889x)
{
    unsigned int reg_val = 0;

    aw889x_i2c_read(aw889x, AW889X_REG_I2STXCFG, &reg_val);
    reg_val |= AW889X_BIT_I2STXCFG_TXEN;
    aw889x_i2c_write(aw889x, AW889X_REG_I2STXCFG, reg_val);

    if(aw889x->dsp_fw_ver == AW889X_DSP_FW_VER_D) {
        aw889x_i2c_read(aw889x, AW889X_REG_DBGCTRL, &reg_val);
        reg_val |= AW889X_BIT_DBGCTRL_LPBK_NEARE;
        aw889x_i2c_write(aw889x, AW889X_REG_DBGCTRL, reg_val);
    }
}

static void aw889x_start(struct aw889x *aw889x)
{
    pr_debug("%s enter\n", __func__);

    aw889x_run_pwd(aw889x, false);
    aw889x_run_mute(aw889x, false);
}
static void aw889x_stop(struct aw889x *aw889x)
{
    pr_debug("%s enter\n", __func__);

    aw889x_run_mute(aw889x, true);
    aw889x_run_pwd(aw889x, true);
}

static void aw889x_container_update(struct aw889x *aw889x, 
        struct aw889x_container *aw889x_cont, int base)
{
    int i = 0;
    int tmp_val = 0;
    int addr = 0;

    pr_debug("%s enter\n", __func__);

    for(i=0; i<aw889x_cont->len; i+=2) {
        tmp_val = (aw889x_cont->data[i+1]<<8) + aw889x_cont->data[i+0];
        aw889x_i2c_write(aw889x, AW889X_REG_DSPMADD, base+addr);
        aw889x_i2c_write(aw889x, AW889X_REG_DSPMDAT, tmp_val);
        addr++;
    }

    pr_debug("%s exit\n", __func__);
}

static void aw889x_cfg_loaded(const struct firmware *cont, void *context)
{
    struct aw889x *aw889x = context;
    struct aw889x_container *aw889x_cfg;
//    int i;
    unsigned int reg_val;

    aw889x->dsp_cfg_state = AW889X_DSP_CFG_FAIL;

    if (!cont) {
        pr_err("%s: failed to read %s\n", __func__, aw889x_cfg_name);
        release_firmware(cont);
        return;
    }

    pr_info("%s: loaded %s - size: %zu\n", __func__, aw889x_cfg_name,
                    cont ? cont->size : 0);
/*
    for(i=0; i<cont->size; i++) {
        pr_info("%s: addr:0x%04x, data:0x%02x\n", __func__, i, *(cont->data+i));
    }
*/
    aw889x_cfg = kzalloc(cont->size+sizeof(int), GFP_KERNEL);
    if (!aw889x_cfg) {
        release_firmware(cont);
        pr_err("%s: error allocating memory\n", __func__);
        return;
    }
    aw889x_cfg->len = cont->size;
    memcpy(aw889x_cfg->data, cont->data, cont->size);
    release_firmware(cont);

    aw889x_container_update(aw889x, aw889x_cfg, AW889X_DSP_CFG_BASE);

    aw889x->dsp_cfg_len = aw889x_cfg->len;

    pr_info("%s: cfg update complete\n", __func__);

    aw889x_dsp_enable(aw889x, true);

    msleep(1);

    aw889x_i2c_read(aw889x, AW889X_REG_WDT, &reg_val);
    if(reg_val) {
        aw889x_tx_cfg(aw889x);
        aw889x_spk_rcv_mode(aw889x);
        aw889x_start(aw889x);
        if(!(aw889x->flags & AW889X_FLAG_SKIP_INTERRUPTS)) {
            aw889x_interrupt_clear(aw889x);
            aw889x_interrupt_setup(aw889x);
        }
        aw889x->init = 1;
        aw889x->dsp_cfg_state = AW889X_DSP_CFG_OK;
        aw889x->dsp_fw_state = AW889X_DSP_FW_OK;
        pr_info("%s: init ok \n", __func__);
    } else {
        aw889x_dsp_enable(aw889x, false);
        pr_info("%s: dsp working wdt, dsp fw&cfg update failed \n", __func__);
    }
}

static int aw889x_load_cfg(struct aw889x *aw889x)
{
    pr_info("%s enter\n", __func__);

    if(aw889x->dsp_cfg_state == AW889X_DSP_CFG_OK) {
        pr_debug("%s: dsp cfg ok\n", __func__);
        return 0;
    }

    aw889x->dsp_cfg_state = AW889X_DSP_CFG_PENDING;

    return request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG,
                    aw889x_cfg_name, aw889x->dev, GFP_KERNEL,
                    aw889x, aw889x_cfg_loaded);
}


static void aw889x_fw_loaded(const struct firmware *cont, void *context)
{
    struct aw889x *aw889x = context;
    struct aw889x_container *aw889x_fw;
//    int i;
    int ret;

    pr_debug("%s enter\n", __func__);

    aw889x->dsp_fw_state = AW889X_DSP_FW_FAIL;

    if (!cont) {
        pr_err("%s: failed to read %s\n", __func__, aw889x_fw_name[aw889x->dsp_fw_ver]);
        release_firmware(cont);
        return;
    }

    pr_info("%s: loaded %s - size: %zu\n", __func__, aw889x_fw_name[aw889x->dsp_fw_ver],
                    cont ? cont->size : 0);
/*
    for(i=0; i<cont->size; i++) {
        pr_info("%s: addr:0x%04x, data:0x%02x\n", __func__, i, *(cont->data+i));
    }
*/
    aw889x_fw = kzalloc(cont->size+sizeof(int), GFP_KERNEL);
    if (!aw889x_fw) {
        release_firmware(cont);
        pr_err("%s: Error allocating memory\n", __func__);
        return;
    }
    aw889x_fw->len = cont->size;
    memcpy(aw889x_fw->data, cont->data, cont->size);
    release_firmware(cont);

    aw889x_container_update(aw889x, aw889x_fw, AW889X_DSP_FW_BASE);

    aw889x->dsp_fw_len = aw889x_fw->len;

    pr_info("%s: fw update complete\n", __func__);

    ret = aw889x_load_cfg(aw889x);
    if(ret) {
        pr_err("%s: cfg loading requested failed: %d\n", __func__, ret);
    }
}

static int aw889x_load_fw(struct aw889x *aw889x)
{
    unsigned int reg_val;
    unsigned int tmp_val;
    pr_info("%s enter\n", __func__);

    if(aw889x->dsp_fw_state == AW889X_DSP_FW_OK) {
        pr_info("%s: dsp fw ok\n", __func__);
        return 0;
    }

    aw889x->dsp_fw_state = AW889X_DSP_FW_PENDING;

    aw889x_i2c_write(aw889x, AW889X_REG_DSPMADD, AW889X_DSP_FW_VER_BASE);
    aw889x_i2c_read(aw889x, AW889X_REG_DSPMDAT, &reg_val);
    tmp_val |= reg_val;
    aw889x_i2c_write(aw889x, AW889X_REG_DSPMADD, AW889X_DSP_FW_VER_BASE+1);
    aw889x_i2c_read(aw889x, AW889X_REG_DSPMDAT, &reg_val);
    tmp_val |= (reg_val<<16);
    if(tmp_val == 0xdeac97d6) {
        aw889x->dsp_fw_ver = AW889X_DSP_FW_VER_E;
        pr_info("%s: dsp fw read version: AW889X_DSP_VER_FW_E: 0x%x\n", __func__, tmp_val);
    } else if (tmp_val == 0){
        aw889x->dsp_fw_ver = AW889X_DSP_FW_VER_D;
        pr_info("%s: dsp fw read version: AW889X_DSP_FW_VER_D: 0x%x\n", __func__, tmp_val);
    } else {
        aw889x->dsp_fw_ver = AW889X_DSP_FW_VER_NONE;
        pr_err("%s: dsp fw read version err: 0x%x\n", __func__, tmp_val);
        return -1;
    }

    return request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG,
                                aw889x_fw_name[aw889x->dsp_fw_ver], aw889x->dev, GFP_KERNEL,
                                aw889x, aw889x_fw_loaded);
}

static void aw889x_cold_start(struct aw889x *aw889x)
{
    int ret = -1;
    unsigned int reg_val = 0;

    pr_info("%s enter\n", __func__);

    aw889x_init(aw889x);

    if(aw889x->flags & AW889X_FLAG_DSP_START_ON) {
        aw889x_dsp_enable(aw889x, false);

        aw889x_i2c_read(aw889x, AW889X_REG_SYSST, &reg_val);
        if(reg_val & AW889X_BIT_SYSST_PLLS) {
            ret = aw889x_load_fw(aw889x);
            if(ret) {
                pr_err("%s: fw loading requested failed: %d\n", __func__, ret);
            }
        } else {
            pr_err("%s: plls=%d, no iis singal\n", __func__, reg_val&AW889X_BIT_SYSST_PLLS);
        }
    } else {
        aw889x_dsp_enable(aw889x, false);
    }
}

static void aw889x_speaker_cfg(struct aw889x *aw889x, bool flag)
{
    pr_info("%s, flag = %d\n", __func__, flag);

    if(flag == true) {
        if(aw889x->init == 0) {
            pr_info("%s, init = %d\n", __func__, aw889x->init);
            aw889x_cold_start(aw889x);
        } else {
            aw889x_spk_rcv_mode(aw889x);
            aw889x_start(aw889x);
        }
    } else {
        aw889x_stop(aw889x);
    }
}
static void aw889x_receiver_cfg(struct aw889x *aw889x, bool flag)
{
    pr_info("%s, flag = %d\n", __func__, flag);

    if(flag == true) {
        if(aw889x->init == 0) {
            pr_info("%s, init = %d\n", __func__, aw889x->init);
            aw889x_cold_start(aw889x);
        } else {
            aw889x_spk_rcv_mode(aw889x);
            aw889x_start(aw889x);
        }
    } else {
        aw889x_stop(aw889x);
    }
}
/******************************************************
 *
 * kcontrol
 *
 ******************************************************/
static const char *const spk_function[] = { "Off", "On" };
static const char *const rcv_function[] = { "Off", "On" };
static const DECLARE_TLV_DB_SCALE(digital_gain,0,50,0);

struct soc_mixer_control aw889x_mixer ={
    .reg    = AW889X_REG_DSPCFG,
    .shift  = AW889X_VOL_REG_SHIFT,
    .max    = AW889X_VOLUME_MAX,
    .min    = AW889X_VOLUME_MIN,
};

static int aw889x_volume_info(struct snd_kcontrol *kcontrol,struct snd_ctl_elem_info *uinfo)
{
    struct soc_mixer_control *mc = (struct soc_mixer_control*) kcontrol->private_value;

    //set kcontrol info
    uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
    uinfo->count = 1;
    uinfo->value.integer.min = 0;
    uinfo->value.integer.max = mc->max - mc->min;
    return 0;
}

static int aw889x_volume_get(struct snd_kcontrol *kcontrol,struct snd_ctl_elem_value *ucontrol)
{
    struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
    struct aw889x *aw889x = snd_soc_codec_get_drvdata(codec);
    int value;
    struct soc_mixer_control *mc = (struct soc_mixer_control*) kcontrol->private_value;

    aw889x_i2c_read(aw889x,AW889X_REG_DSPCFG,&value);
    ucontrol->value.integer.value[0] = (value >> mc->shift)\
                                   &(~AW889X_BIT_DSPCFG_VOL_MASK);
    return 0;
}

static int aw889x_volume_put(struct snd_kcontrol *kcontrol,struct snd_ctl_elem_value *ucontrol)
{
    struct soc_mixer_control *mc = (struct soc_mixer_control*) kcontrol->private_value;
    struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
    struct aw889x *aw889x = snd_soc_codec_get_drvdata(codec);
    unsigned int value, reg_value;

    //value is right
    value = ucontrol->value.integer.value[0];
    if(value > (mc->max-mc->min)|| value <0){
     pr_err("%s:value over range \n",__func__);
     return -1;
    }

    //smartpa have clk
    aw889x_i2c_read(aw889x,AW889X_REG_SYSST,&reg_value);
    if(!(reg_value&AW889X_BIT_SYSST_PLLS)){
     pr_err("%s: NO I2S CLK ,cat not write reg \n",__func__);
     return 0;
    }
    //cal real value
    value = value << mc->shift&AW889X_BIT_DSPCFG_VOL_MASK;
    aw889x_i2c_read(aw889x,AW889X_REG_DSPCFG,&reg_value);
    value = value | (reg_value&0x00ff);

    //write value
    aw889x_i2c_read(aw889x,AW889X_REG_SYSCTRL,&reg_value);
    if(reg_value&AW889X_BIT_SYSCTRL_DSPBY){
      aw889x_i2c_write(aw889x,AW889X_REG_DSPCFG,value);
    }else{
      aw889x_dsp_enable(aw889x,false);
      aw889x_i2c_write(aw889x,AW889X_REG_DSPCFG,value);
      aw889x_dsp_enable(aw889x,true);
    }

    return 0;
}

 static struct snd_kcontrol_new aw889x_volume = {
    .iface = SNDRV_CTL_ELEM_IFACE_MIXER,
    .name  = "aw889x_rx_volume",
    .access= SNDRV_CTL_ELEM_ACCESS_TLV_READ|SNDRV_CTL_ELEM_ACCESS_READWRITE,
    .tlv.p  = (digital_gain),
    .info = aw889x_volume_info,
    .get =  aw889x_volume_get,
    .put =  aw889x_volume_put,
    .private_value = (unsigned long)&aw889x_mixer,
};

static int aw889x_spk_get(struct snd_kcontrol *kcontrol, 
                struct snd_ctl_elem_value *ucontrol)
{
    pr_debug("%s: aw889x_spk_control=%d\n", __func__, aw889x_spk_control);
    ucontrol->value.integer.value[0] = aw889x_spk_control;
    return 0;
}

static int aw889x_spk_set(struct snd_kcontrol *kcontrol, 
                struct snd_ctl_elem_value *ucontrol)
{
    struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
    struct aw889x *aw889x = snd_soc_codec_get_drvdata(codec);

    pr_debug("%s: ucontrol->value.integer.value[0]=%ld\n ",
            __func__, ucontrol->value.integer.value[0]);
    if(ucontrol->value.integer.value[0] == aw889x_spk_control)
        return 1;

    aw889x_spk_control = ucontrol->value.integer.value[0];

    aw889x->spk_rcv_mode = AW889X_SPEAKER_MODE;

    if(ucontrol->value.integer.value[0]) {
        aw889x_speaker_cfg(aw889x, true);
    } else {
        aw889x_speaker_cfg(aw889x, false);
    }

    return 0;
}

static int aw889x_rcv_get(struct snd_kcontrol *kcontrol, 
                struct snd_ctl_elem_value *ucontrol)
{
    pr_debug("%s: aw889x_rcv_control=%d\n", __func__, aw889x_rcv_control);
    ucontrol->value.integer.value[0] = aw889x_rcv_control;
    return 0;
}
static int aw889x_rcv_set(struct snd_kcontrol *kcontrol, 
                struct snd_ctl_elem_value *ucontrol)
{
    struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
    struct aw889x *aw889x = snd_soc_codec_get_drvdata(codec);
    pr_debug("%s: ucontrol->value.integer.value[0]=%ld\n ",
            __func__, ucontrol->value.integer.value[0]);
    if(ucontrol->value.integer.value[0] == aw889x_rcv_control)
        return 1;

    aw889x_rcv_control = ucontrol->value.integer.value[0];

    aw889x->spk_rcv_mode = AW889X_RECEIVER_MODE;

    if(ucontrol->value.integer.value[0]) {
        aw889x_receiver_cfg(aw889x, true);
    } else {
        aw889x_receiver_cfg(aw889x, false);
    }
    return 0;
}
static const struct soc_enum aw889x_snd_enum[] = {
    SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(spk_function), spk_function),
    SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(rcv_function), rcv_function),
};

static struct snd_kcontrol_new aw889x_controls[] = {
    SOC_ENUM_EXT("aw889x_speaker_switch", aw889x_snd_enum[0], 
            aw889x_spk_get, aw889x_spk_set),
    SOC_ENUM_EXT("aw889x_receiver_switch", aw889x_snd_enum[1], 
            aw889x_rcv_get, aw889x_rcv_set),
};

static void aw889x_add_codec_controls(struct aw889x *aw889x)
{
    pr_info("%s enter\n", __func__);

    snd_soc_add_codec_controls(aw889x->codec, aw889x_controls,
                ARRAY_SIZE(aw889x_controls));

     snd_soc_add_codec_controls(aw889x->codec, &aw889x_volume,1);
}

/******************************************************
 *
 * DAPM Widget & Route
 *
 ******************************************************/
static struct snd_soc_dapm_widget aw889x_dapm_widgets_common[] = {
    /* Stream widgets */
    SND_SOC_DAPM_AIF_IN("AIF_IN", "AW89xx_AIF_Playback", 0, SND_SOC_NOPM, 0, 0),
    SND_SOC_DAPM_AIF_OUT("AIF_OUT", "AW89xx_AIF_Capture", 0, SND_SOC_NOPM, 0, 0),

    SND_SOC_DAPM_OUTPUT("OUTL"),
    SND_SOC_DAPM_INPUT("AEC_Loopback"),
};

static const struct snd_soc_dapm_route aw889x_dapm_routes_common[] = {
    { "OUTL", NULL, "AIF_IN" },
    { "AIF_OUT", NULL, "AEC_Loopback" },
};

static void aw889x_add_widgets(struct aw889x *aw889x)
{
    //struct snd_soc_dapm_context *dapm = &aw889x->codec->dapm;
    struct snd_soc_dapm_context *dapm = &aw889x->codec->component.dapm;
    struct snd_soc_dapm_widget *widgets;

    pr_info("%s enter\n", __func__);
    widgets = devm_kzalloc(&aw889x->i2c->dev,
            sizeof(struct snd_soc_dapm_widget) *
                ARRAY_SIZE(aw889x_dapm_widgets_common),
            GFP_KERNEL);
    if (!widgets)
        return;

    memcpy(widgets, aw889x_dapm_widgets_common,
            sizeof(struct snd_soc_dapm_widget) *
                ARRAY_SIZE(aw889x_dapm_widgets_common));

    snd_soc_dapm_new_controls(dapm, widgets,
                  ARRAY_SIZE(aw889x_dapm_widgets_common));
    snd_soc_dapm_add_routes(dapm, aw889x_dapm_routes_common,
                ARRAY_SIZE(aw889x_dapm_routes_common));
}




/******************************************************
 *
 * Digital Audio Interface
 *
 ******************************************************/
 static int aw889x_startup(struct snd_pcm_substream *substream,
    struct snd_soc_dai *dai)
{
    struct snd_soc_codec *codec = dai->codec;
    struct aw889x *aw889x = snd_soc_codec_get_drvdata(codec);

    pr_info("%s: enter\n", __func__);
    aw889x_run_pwd(aw889x, false);

    return 0;
}

static int aw889x_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
    //struct aw889x *aw889x = snd_soc_codec_get_drvdata(dai->codec);
    struct snd_soc_codec *codec = dai->codec;

    pr_info("%s: fmt=0x%x\n", __func__, fmt);

    /* Supported mode: regular I2S, slave, or PDM */
    switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
    case SND_SOC_DAIFMT_I2S:
        if ((fmt & SND_SOC_DAIFMT_MASTER_MASK) != SND_SOC_DAIFMT_CBS_CFS) {
            dev_err(codec->dev, "%s: invalid codec master mode\n", __func__);
            return -EINVAL;
        }
        break;
    default:
        dev_err(codec->dev, "%s: unsupported DAI format %d\n", __func__, 
            fmt & SND_SOC_DAIFMT_FORMAT_MASK);
        return -EINVAL;
    }
    return 0;
}

static int aw889x_set_dai_sysclk(struct snd_soc_dai *codec_dai,
    int clk_id, unsigned int freq, int dir)
{
    struct aw889x *aw889x = snd_soc_codec_get_drvdata(codec_dai->codec);

    pr_info("%s: freq=%d\n", __func__, freq);

    aw889x->sysclk = freq;
    return 0;
}

static int aw889x_hw_params(struct snd_pcm_substream *substream,
    struct snd_pcm_hw_params *params,
    struct snd_soc_dai *dai)
{
    struct snd_soc_codec *codec = dai->codec;
    struct aw889x *aw889x = snd_soc_codec_get_drvdata(codec);
    unsigned int rate;
    int reg_value,tmp_value;
    int width;
    /* Supported */

    //get rate param
    aw889x->rate=rate = params_rate(params);
    pr_debug("%s: requested rate: %d, sample size: %d\n", __func__, rate,
                        snd_pcm_format_width(params_format(params)));
    //match rate
    switch(rate)
    {
      case 8000:
          reg_value = AW889X_BIT_I2SCTRL_SR_8K;
          break;
      case 16000:
          reg_value = AW889X_BIT_I2SCTRL_SR_16K;
          break;
      case 32000:
          reg_value = AW889X_BIT_I2SCTRL_SR_32K;
          break;
      case 44100:
          reg_value = AW889X_BIT_I2SCTRL_SR_44K;
          break;
      case 48000:
          reg_value = AW889X_BIT_I2SCTRL_SR_48K;
          break;
      default:
        reg_value = -1;
        pr_err("rate can not support\n");
    }
    //set chip rate
    if(-1 != reg_value){
       aw889x_i2c_read(aw889x,AW889X_REG_I2SCTRL,&tmp_value);
       reg_value = reg_value | (tmp_value& (~AW889X_BIT_I2SCTRL_SR_MASK));
       aw889x_i2c_write(aw889x,AW889X_REG_I2SCTRL,reg_value);
    }

    //get bit width
    width = params_width(params);
    pr_debug("%s: width = %d \n",__func__,width);
    switch(width)
    {
      case 16:
         reg_value = AW889X_BIT_I2SCTRL_FMS_16BIT;
         break;
      case 24:
         reg_value = AW889X_BIT_I2SCTRL_FMS_24BIT;
         break;
      default:
         reg_value = -1;
         pr_debug("width can not support\n");
    }
    //set width
    if(-1 != reg_value){
      aw889x_i2c_read(aw889x,AW889X_REG_I2SCTRL,&tmp_value);
      reg_value = reg_value | (tmp_value&(~AW889X_BIT_I2SCTRL_FMS_MASK));
      aw889x_i2c_write(aw889x,AW889X_REG_I2SCTRL,reg_value);
    }

    return 0;
}

static int aw889x_mute(struct snd_soc_dai *dai, int mute, int stream)
{
    struct snd_soc_codec *codec = dai->codec;
    struct aw889x *aw889x = snd_soc_codec_get_drvdata(codec);

    pr_info("%s: mute state=%d\n", __func__, mute);

    if (!(aw889x->flags & AW889X_FLAG_DSP_START_ON_MUTE))
        return 0;

    if (mute) {
        /* stop DSP only when both playback and capture streams
         * are deactivated
         */
        if (stream == SNDRV_PCM_STREAM_PLAYBACK)
            aw889x->pstream = 0;
        else
            aw889x->cstream = 0;
        if (aw889x->pstream != 0 || aw889x->cstream != 0)
            return 0;

        if (aw889x->dsp_fw_state != AW889X_DSP_FW_OK)
            return 0;
        mutex_lock(&aw889x->dsp_lock);
        aw889x_stop(aw889x);
        mutex_unlock(&aw889x->dsp_lock);
    } else {
        if (stream == SNDRV_PCM_STREAM_PLAYBACK)
            aw889x->pstream = 1;
        else
            aw889x->cstream = 1;

        /* Start DSP */
        mutex_lock(&aw889x->dsp_lock);
        aw889x_start(aw889x);
        mutex_unlock(&aw889x->dsp_lock);
    }

    return 0;
}

static void aw889x_shutdown(struct snd_pcm_substream *substream,
    struct snd_soc_dai *dai)
{
    struct snd_soc_codec *codec = dai->codec;
    struct aw889x *aw889x = snd_soc_codec_get_drvdata(codec);

    aw889x->rate = 0;
    aw889x_run_pwd(aw889x, true);
}

static const struct snd_soc_dai_ops aw889x_dai_ops = {
    .startup = aw889x_startup,
    .set_fmt = aw889x_set_fmt,
    .set_sysclk = aw889x_set_dai_sysclk,
    .hw_params = aw889x_hw_params,
    .mute_stream = aw889x_mute,
    .shutdown = aw889x_shutdown,
};

static struct snd_soc_dai_driver aw889x_dai[] = {
    {
        .name = "aw889x-aif",
        .id = 1,
        .playback = {
            .stream_name = "Speaker_Playback",
            .channels_min = 1,
            .channels_max = 2,
            .rates = AW889X_RATES,
            .formats = AW889X_FORMATS,
        },
        .capture = {
             .stream_name = "Speaker_Capture",
             .channels_min = 1,
             .channels_max = 2,
             .rates = AW889X_RATES,
             .formats = AW889X_FORMATS,
         },
        .ops = &aw889x_dai_ops,
        .symmetric_rates = 1,
        .symmetric_channels = 1,
        .symmetric_samplebits = 1,
    },
	/* 1 - virt dai for voice call */
	{
		.name = "aw889x-aif-Voice",
		.id = 1,
		.playback = {
			 .stream_name = "Speaker_Playback voice",
			 .channels_min = 1,
			 .channels_max = 2,
			 .rates = AW889X_RATES,
			 .formats = AW889X_FORMATS,
		},
		.capture = {
			  .stream_name = "Speaker_Capture voice",
			  .channels_min = 1,
			  .channels_max = 2,
			  .rates = AW889X_RATES,
			  .formats = AW889X_FORMATS,
		},
		.ops = &aw889x_dai_ops,
		.symmetric_rates = 1,
		.symmetric_channels = 1,
		.symmetric_samplebits = 1,
	},
};

/******************************************************
 *
 * Input Event
 *
 ******************************************************/
/*int aw889x_inputdev_register(struct aw889x *aw889x)
{
    int err;
    struct input_dev *input;

    pr_info("%s enter\n", __func__);

    input = input_allocate_device();

    if (!input) {
        dev_err(aw889x->codec->dev, "%s: unable to allocate input device\n", __func__);
        return -ENOMEM;
    }

    input->evbit[0] = BIT_MASK(EV_KEY);
    input->keybit[BIT_WORD(BTN_0)] |= BIT_MASK(BTN_0);
    input->keybit[BIT_WORD(BTN_1)] |= BIT_MASK(BTN_1);
    input->keybit[BIT_WORD(BTN_2)] |= BIT_MASK(BTN_2);
    input->keybit[BIT_WORD(BTN_3)] |= BIT_MASK(BTN_3);
    input->keybit[BIT_WORD(BTN_4)] |= BIT_MASK(BTN_4);
    input->keybit[BIT_WORD(BTN_5)] |= BIT_MASK(BTN_5);
    input->keybit[BIT_WORD(BTN_6)] |= BIT_MASK(BTN_6);
    input->keybit[BIT_WORD(BTN_7)] |= BIT_MASK(BTN_7);
    input->keybit[BIT_WORD(BTN_8)] |= BIT_MASK(BTN_8);
    input->keybit[BIT_WORD(BTN_9)] |= BIT_MASK(BTN_9);

//    input->open = aw889x_input_open;
//    input->close = aw889x_input_close;

    input->name = "aw889x-tapdetect";

    input->id.bustype = BUS_I2C;
    input_set_drvdata(input, aw889x);

    err = input_register_device(input);
    if (err) {
        dev_err(aw889x->codec->dev, "%s: unable to register input device\n",
            __func__);
        goto err_free_dev;
    }

    dev_info(aw889x->codec->dev, "%s: input device for tap-detection registered: %s\n",
        __func__, input->name);
    aw889x->input = input;
    return 0;

err_free_dev:
    input_free_device(input);
    return err;
}

void aw889x_inputdev_unregister(struct aw889x *aw889x)
{
    input_unregister_device(aw889x->input);
    aw889x->input = NULL;
}
*/

/*****************************************************
 *
 * codec driver
 *
 *****************************************************/
static int aw889x_probe(struct snd_soc_codec *codec)
{
    struct aw889x *aw889x = snd_soc_codec_get_drvdata(codec);
    int ret = 0;

    pr_info("%s enter\n", __func__);

	if (no_chip) {
		pr_info("%s no_chip error!\n", __func__);
		return 0;
	}

    aw889x->codec = codec;

    aw889x_add_widgets(aw889x);

    aw889x_add_codec_controls(aw889x);

    //aw889x_inputdev_register(aw889x);

    if (codec->dev->of_node)
        dev_set_name(codec->dev, "%s", "tfasmartpa");

    pr_info("%s exit\n", __func__);

    return ret;
}

static int aw889x_remove(struct snd_soc_codec *codec)
{
    //struct aw889x *aw889x = snd_soc_codec_get_drvdata(codec);
    pr_info("%s enter\n", __func__);

    //aw889x_inputdev_unregister(aw889x);

    return 0;
}

struct regmap *aw889x_get_regmap(struct device *dev)
{
    struct aw889x *aw889x = dev_get_drvdata(dev);

    return aw889x->regmap;
}

static unsigned int aw889x_codec_read(struct snd_soc_codec *codec,unsigned int reg)
{
  struct aw889x *aw889x=snd_soc_codec_get_drvdata(codec);
  unsigned int value =0;
  int ret;
  pr_debug("%s:enter \n",__func__);

  if(aw889x_reg_access[reg]&REG_RD_ACCESS){
    ret=aw889x_i2c_read(aw889x,reg,&value);
    if(ret<0){
      pr_debug("%s: read register failed \n",__func__);
      return ret;
    }
  }else{
    pr_debug("%s:Register 0x%x NO read access\n",__func__,reg);
    return -1;
  }
  return value;
}

static int aw889x_codec_write(struct snd_soc_codec *codec,unsigned int reg,unsigned int value)
{
    int ret ;
    struct aw889x *aw889x=snd_soc_codec_get_drvdata(codec);
    pr_debug("%s:enter ,reg is 0x%x value is 0x%x\n",__func__,reg,value);

    if(aw889x_reg_access[reg]&REG_WR_ACCESS){
      ret=aw889x_i2c_write(aw889x,reg,value);

      return ret;
    }else{
      pr_debug("%s: Register 0x%x NO write access \n",__func__,reg);
    }

    return -1;
}
/*
static int aw889x_codec_readable(struct snd_soc_codec *codec,unsigned int reg)
{
    return aw889x_reg_access[reg]&REG_RD_ACCESS;
}
*/
static struct snd_soc_codec_driver soc_codec_dev_aw889x = {
    .probe = aw889x_probe,
    .remove = aw889x_remove,
    //.get_regmap = aw889x_get_regmap,
    .read = aw889x_codec_read,
    .write= aw889x_codec_write,
    //.readable_register= aw889x_readable,
    .reg_cache_size= AW889X_REG_MAX,
    .reg_word_size=2,
};

/*****************************************************
 *
 * regmap
 *
 *****************************************************/
bool aw889x_writeable_register(struct device *dev, unsigned int reg)
{
    /* enable read access for all registers */
    return 1;
}

bool aw889x_readable_register(struct device *dev, unsigned int reg)
{
    /* enable read access for all registers */
    return 1;
}

bool aw889x_volatile_register(struct device *dev, unsigned int reg)
{
    /* enable read access for all registers */
    return 1;
}

static const struct regmap_config aw889x_regmap = {
    .reg_bits = 8,
    .val_bits = 16,

    .max_register = AW889X_MAX_REGISTER,
    .writeable_reg = aw889x_writeable_register,
    .readable_reg = aw889x_readable_register,
    .volatile_reg = aw889x_volatile_register,
    .cache_type = REGCACHE_RBTREE,
};

/******************************************************
 *
 * irq 
 *
 ******************************************************/
static void aw889x_interrupt_setup(struct aw889x *aw889x)
{
    unsigned int reg_val;

    pr_info("%s enter\n", __func__);

    aw889x_i2c_read(aw889x, AW889X_REG_SYSINTM, &reg_val);
    reg_val &= (~AW889X_BIT_SYSINTM_PLLM);
    reg_val &= (~AW889X_BIT_SYSINTM_OTHM);
    reg_val &= (~AW889X_BIT_SYSINTM_OCDM);
    reg_val = 0;
    aw889x_i2c_write(aw889x, AW889X_REG_SYSINTM, reg_val);
}

static void aw889x_interrupt_clear(struct aw889x *aw889x)
{
    unsigned int reg_val;

    pr_info("%s enter\n", __func__);

    aw889x_i2c_read(aw889x, AW889X_REG_SYSST, &reg_val);
    pr_info("%s: reg SYSST=0x%x\n", __func__, reg_val);

    aw889x_i2c_read(aw889x, AW889X_REG_SYSINT, &reg_val);
    pr_info("%s: reg SYSINT=0x%x\n", __func__, reg_val);

    aw889x_i2c_read(aw889x, AW889X_REG_SYSINTM, &reg_val);
    pr_info("%s: reg SYSINTM=0x%x\n", __func__, reg_val);
}

static irqreturn_t aw889x_irq(int irq, void *data)
{
    struct aw889x *aw889x = data;

    pr_info("%s enter\n", __func__);

    aw889x_interrupt_clear(aw889x);

    pr_info("%s exit\n", __func__);

    return IRQ_HANDLED;
}

/*****************************************************
 *
 * device tree
 *
 *****************************************************/
static int aw889x_parse_dt(struct device *dev, struct aw889x *aw889x,
        struct device_node *np) {
    aw889x->reset_gpio = of_get_named_gpio(np, "reset-gpio", 0);
    if (aw889x->reset_gpio < 0) {
        dev_err(dev, "%s: no reset gpio provided, will not HW reset device\n", __func__);
        return -1;
    } else {
        dev_info(dev, "%s: reset gpio provided ok\n", __func__);
    }
    aw889x->irq_gpio =  of_get_named_gpio(np, "irq-gpio", 0);
    if (aw889x->irq_gpio < 0) {
        dev_info(dev, "%s: no irq gpio provided.\n", __func__);
    } else {
        dev_info(dev, "%s: irq gpio provided ok.\n", __func__);
    }

    return 0;
}

int aw889x_hw_reset(struct aw889x *aw889x)
{
    pr_info("%s enter\n", __func__);

    if (aw889x && gpio_is_valid(aw889x->reset_gpio)) {
        gpio_set_value_cansleep(aw889x->reset_gpio, 1);
        msleep(1);
        gpio_set_value_cansleep(aw889x->reset_gpio, 0);
        msleep(1);
    } else {
        dev_err(aw889x->dev, "%s:  failed\n", __func__);
    }
    return 0;
}

/*****************************************************
 *
 * check chip id
 *
 *****************************************************/
int aw889x_read_chipid(struct aw889x *aw889x)
{
    int ret = -1;
    unsigned int cnt = 0;
    unsigned int reg = 0;

    while(cnt < AW_READ_CHIPID_RETRIES) {
        ret = aw889x_i2c_read(aw889x, AW889X_REG_ID, &reg);
        if (ret < 0) {
            dev_err(aw889x->dev, "%s: failed to read register AW889X_REG_ID: %d\n", __func__, ret);
            return -EIO;
        }
        switch (reg) {
        case 0x0310:
            pr_info("%s aw889x detected\n", __func__);
            aw889x->flags |= AW889X_FLAG_SKIP_INTERRUPTS;
            aw889x->flags |= AW889X_FLAG_DSP_START_ON_MUTE;
            aw889x->flags |= AW889X_FLAG_DSP_START_ON;
            aw889x->chipid = AW8990_ID;
            pr_info("%s aw889x->flags=0x%x\n", __func__, aw889x->flags);
            return 0;
        default:
            pr_info("%s unsupported device revision (0x%x)\n", __func__, reg );
            break;
        }
        cnt ++;

        msleep(AW_READ_CHIPID_RETRY_DELAY);
    }

    return -EINVAL;
}

/******************************************************
 *
 * sys bin attribute
 *
 *****************************************************/
static ssize_t aw889x_reg_write(struct file *filp, struct kobject *kobj,
                struct bin_attribute *bin_attr,
                char *buf, loff_t off, size_t count)
{
    struct device *dev = container_of(kobj, struct device, kobj);
    struct aw889x *aw889x = dev_get_drvdata(dev);

    if (count != 1) {
        pr_info("invalid register address");
        return -EINVAL;
    }

    aw889x->reg = buf[0];

    return 1;
}

static ssize_t aw889x_rw_write(struct file *filp, struct kobject *kobj,
                struct bin_attribute *bin_attr,
                char *buf, loff_t off, size_t count)
{
    struct device *dev = container_of(kobj, struct device, kobj);
    struct aw889x *aw889x = dev_get_drvdata(dev);
    u8 *data;
    int ret;
    int retries = AW_I2C_RETRIES;

    data = kmalloc(count+1, GFP_KERNEL);
    if (data == NULL) {
        pr_err("can not allocate memory\n");
        return  -ENOMEM;
    }

    data[0] = aw889x->reg;
    memcpy(&data[1], buf, count);

retry:
    ret = i2c_master_send(aw889x->i2c, data, count+1);
    if (ret < 0) {
        pr_warn("i2c error, retries left: %d\n", retries);
        if (retries) {
            retries--;
            msleep(AW_I2C_RETRY_DELAY);
            goto retry;
        }
    }

    kfree(data);
    return ret;
}

static ssize_t aw889x_rw_read(struct file *filp, struct kobject *kobj,
                struct bin_attribute *bin_attr,
                char *buf, loff_t off, size_t count)
{
    struct device *dev = container_of(kobj, struct device, kobj);
    struct aw889x *aw889x = dev_get_drvdata(dev);
    struct i2c_msg msgs[] = {
        {
            .addr = aw889x->i2c->addr,
            .flags = 0,
            .len = 1,
            .buf = &aw889x->reg,
        },
        {
            .addr = aw889x->i2c->addr,
            .flags = I2C_M_RD,
            .len = count,
            .buf = buf,
        },
    };
    int ret;
    int retries = AW_I2C_RETRIES;
retry:
    ret = i2c_transfer(aw889x->i2c->adapter, msgs, ARRAY_SIZE(msgs));
    if (ret < 0) {
        pr_warn("i2c error, retries left: %d\n", retries);
        if (retries) {
            retries--;
            msleep(AW_I2C_RETRY_DELAY);
            goto retry;
        }
        return ret;
    }
    /* ret contains the number of i2c messages send */
    return 1 + ((ret > 1) ? count : 0);
}

static struct bin_attribute dev_attr_rw = {
    .attr = {
        .name = "rw",
        .mode = S_IRUSR | S_IWUSR,
    },
    .size = 0,
    .read = aw889x_rw_read,
    .write = aw889x_rw_write,
};

static struct bin_attribute dev_attr_regaddr = {
    .attr = {
        .name = "regaddr",
        .mode = S_IWUSR,
    },
    .size = 0,
    .read = NULL,
    .write = aw889x_reg_write,
};

/******************************************************
 *
 * sys group attribute: reg
 *
 ******************************************************/
static ssize_t aw889x_reg_store(struct device *dev, struct device_attribute *attr,
                  const char *buf, size_t count)
{
    struct aw889x *aw889x = dev_get_drvdata(dev);

    unsigned int databuf[2] = {0};

    if(2 == sscanf(buf, "%x %x", &databuf[0], &databuf[1])) {
        aw889x_i2c_write(aw889x, databuf[0], databuf[1]);
    }

    return count;
}

static ssize_t aw889x_reg_show(struct device *dev, struct device_attribute *attr,
                char *buf)
{
    struct aw889x *aw889x = dev_get_drvdata(dev);
    ssize_t len = 0;
    unsigned char i = 0;
    unsigned int reg_val = 0;
    for(i = 0; i < AW889X_REG_MAX; i ++) {
        if(!(aw889x_reg_access[i]&REG_RD_ACCESS))
           continue;
        aw889x_i2c_read(aw889x, i, &reg_val);
        len += snprintf(buf+len, PAGE_SIZE-len, "reg:0x%02x=0x%04x \n", i, reg_val);
    }
    return len;
}

static ssize_t aw889x_dsp_store(struct device *dev, struct device_attribute *attr,
                  const char *buf, size_t count)
{
    struct aw889x *aw889x = dev_get_drvdata(dev);

    unsigned int databuf[16] = {0};
    unsigned int reg_val = 0;

    if(1 == sscanf(buf, "%d", &databuf[0])) {
        if(databuf[0] == 1) {
            aw889x_i2c_read(aw889x, AW889X_REG_SYSST, &reg_val);
            if(reg_val & AW889X_BIT_SYSST_PLLS) {
                aw889x->init = 0;
                aw889x->dsp_fw_state = AW889X_DSP_FW_FAIL;
                aw889x->dsp_cfg_state = AW889X_DSP_CFG_FAIL;
                aw889x_speaker_cfg(aw889x, false);
                aw889x_speaker_cfg(aw889x, true);
            } else {
                count += snprintf((char *)(buf+count), PAGE_SIZE-count, "aw889x plls=%d, no iis signal\n",
                        reg_val&AW889X_BIT_SYSST_PLLS);
            }
        } else {
            aw889x_dsp_enable(aw889x, false);
        }
    }

    return count;
}

static ssize_t aw889x_dsp_show(struct device *dev, struct device_attribute *attr,
                char *buf)
{
    struct aw889x *aw889x = dev_get_drvdata(dev);
    ssize_t len = 0;
    unsigned int i = 0;
    unsigned int addr = 0;
    unsigned int reg_val = 0;

    aw889x_i2c_read(aw889x, AW889X_REG_SYSCTRL, &reg_val);

    if(reg_val & AW889X_BIT_SYSCTRL_DSPBY) {
        len += snprintf(buf+len, PAGE_SIZE-len, "aw889x dsp bypass\n");
    } else {
        aw889x_i2c_read(aw889x, AW889X_REG_SYSST, &reg_val);
        if(reg_val & AW889X_BIT_SYSST_PLLS) {
            len += snprintf(buf+len, PAGE_SIZE-len, "aw889x dsp working\n");

            aw889x_dsp_enable(aw889x, false);

            len += snprintf(buf+len, PAGE_SIZE-len, "aw889x dsp firmware:\n");
            addr = 0;
            for(i=0; i<aw889x->dsp_fw_len; i+=2) {
                aw889x_i2c_write(aw889x, AW889X_REG_DSPMADD, AW889X_DSP_FW_BASE+addr);
                aw889x_i2c_read(aw889x, AW889X_REG_DSPMDAT, &reg_val);
                len += snprintf(buf+len, PAGE_SIZE-len, "0x%02x,0x%02x,", reg_val&0x00FF, (reg_val>>8));
                if((i/2+1)%4 == 0)
                    len += snprintf(buf+len, PAGE_SIZE-len, "\n");
                addr ++;
            }
            len += snprintf(buf+len, PAGE_SIZE-len, "\n");

            len += snprintf(buf+len, PAGE_SIZE-len, "aw889x dsp config:\n");
            addr = 0;
            for(i=0; i<aw889x->dsp_cfg_len; i+=2) {
                aw889x_i2c_write(aw889x, AW889X_REG_DSPMADD, AW889X_DSP_CFG_BASE+addr);
                aw889x_i2c_read(aw889x, AW889X_REG_DSPMDAT, &reg_val);
                len += snprintf(buf+len, PAGE_SIZE-len, "0x%02x,0x%02x,", reg_val&0x00FF, (reg_val>>8));
                if((i/2+1)%4 == 0)
                     len += snprintf(buf+len, PAGE_SIZE-len, "\n");
                addr ++;
            }
            len += snprintf(buf+len, PAGE_SIZE-len, "\n");

            aw889x_dsp_enable(aw889x, true);
        } else {
                len += snprintf((char *)(buf+len), PAGE_SIZE-len, "aw889x plls=%d, no iis signal\n",
                        reg_val&AW889X_BIT_SYSST_PLLS);
        }
    }

    return len;
}

static ssize_t aw889x_spk_rcv_store(struct device *dev, struct device_attribute *attr,
                  const char *buf, size_t count)
{
    struct aw889x *aw889x = dev_get_drvdata(dev);

    unsigned int databuf[2] = {0};

    if(1 == sscanf(buf, "%d", &databuf[0])) {
        aw889x->spk_rcv_mode = databuf[0];
    }

    return count;
}

static ssize_t aw889x_spk_rcv_show(struct device *dev, struct device_attribute *attr,
                char *buf)
{
    struct aw889x *aw889x = dev_get_drvdata(dev);
    ssize_t len = 0;
    if(aw889x->spk_rcv_mode == AW889X_SPEAKER_MODE) {
        len += snprintf(buf+len, PAGE_SIZE-len, "aw889x spk_rcv: %d, speaker mode\n", aw889x->spk_rcv_mode);
    } else if (aw889x->spk_rcv_mode == AW889X_RECEIVER_MODE) {
        len += snprintf(buf+len, PAGE_SIZE-len, "aw889x spk_rcv: %d, receiver mode\n", aw889x->spk_rcv_mode);
    } else {
        len += snprintf(buf+len, PAGE_SIZE-len, "aw889x spk_rcv: %d, unknown mode\n", aw889x->spk_rcv_mode);
    }

    return len;
}

static DEVICE_ATTR(reg, S_IWUSR | S_IRUGO, aw889x_reg_show, aw889x_reg_store);
static DEVICE_ATTR(dsp, S_IWUSR | S_IRUGO, aw889x_dsp_show, aw889x_dsp_store);
static DEVICE_ATTR(spk_rcv, S_IWUSR | S_IRUGO, aw889x_spk_rcv_show, aw889x_spk_rcv_store);

static struct attribute *aw889x_attributes[] = {
    &dev_attr_reg.attr,
    &dev_attr_dsp.attr,
    &dev_attr_spk_rcv.attr,
    NULL
};

static struct attribute_group aw889x_attribute_group = {
    .attrs = aw889x_attributes
};


/******************************************************
 *
 * i2c driver
 *
 ******************************************************/
static ssize_t aw889x_chip_exist_show(struct kobject *kobj,
					   struct kobj_attribute *attr, char *buff)
{
	return sprintf(buff, "%d\n", !no_chip);
}

static int aw889x_sysfs_create_chip_exist(void)
{
	int ret;
	static struct kobj_attribute chip_exit =
		__ATTR(ext_smartpa_exist, 0644,
		aw889x_chip_exist_show,
		NULL);

	ret = sysfs_create_file(kernel_kobj, &chip_exit.attr);
	if (ret) {
		pr_err("create sysfs '%s' failed. ret = %d\n",
			   chip_exit.attr.name, ret);
		return ret;
	}

	return ret;
}

static void aw889x_i2c_probe_fail(struct i2c_client *i2c)
{
    struct aw889x *aw889x = i2c_get_clientdata(i2c);

    pr_info("%s enter\n", __func__);
    if (gpio_is_valid(aw889x->irq_gpio))
        devm_gpio_free(&i2c->dev, aw889x->irq_gpio);
    if (gpio_is_valid(aw889x->reset_gpio))
        devm_gpio_free(&i2c->dev, aw889x->reset_gpio);
}

static int aw889x_i2c_probe(struct i2c_client *i2c, const struct i2c_device_id *id)
{
    struct snd_soc_dai_driver *dai;
    struct aw889x *aw889x;
    struct device_node *np = i2c->dev.of_node;
    int irq_flags;
    int ret;

    pr_info("%s enter\n", __func__);

    if (!i2c_check_functionality(i2c->adapter, I2C_FUNC_I2C)) {
        dev_err(&i2c->dev, "check_functionality failed\n");
        return -EIO;
    }

    aw889x = devm_kzalloc(&i2c->dev, sizeof(struct aw889x), GFP_KERNEL);
    if (aw889x == NULL)
        return -ENOMEM;

    aw889x->dev = &i2c->dev;
    aw889x->i2c = i2c;

    /* aw889x regmap */
    aw889x->regmap = devm_regmap_init_i2c(i2c, &aw889x_regmap);
    if (IS_ERR(aw889x->regmap)) {
        ret = PTR_ERR(aw889x->regmap);
        dev_err(&i2c->dev, "%s: failed to allocate register map: %d\n", __func__, ret);
        goto err;
    }

    i2c_set_clientdata(i2c, aw889x);
    mutex_init(&aw889x->dsp_lock);
    /* aw889x rst & int */
    if (np) {
        ret = aw889x_parse_dt(&i2c->dev, aw889x, np);
        if (ret) {
            dev_err(&i2c->dev, "%s: failed to parse device tree node\n", __func__);
            goto err;
        }
    } else {
        aw889x->reset_gpio = -1;
        aw889x->irq_gpio = -1;
    }

    if (gpio_is_valid(aw889x->reset_gpio)) {
        ret = devm_gpio_request_one(&i2c->dev, aw889x->reset_gpio,
            GPIOF_OUT_INIT_LOW, "aw889x_rst");
        if (ret){
            dev_err(&i2c->dev, "%s: rst request failed\n", __func__);
            goto err;
        }
    }

    if (gpio_is_valid(aw889x->irq_gpio)) {
        ret = devm_gpio_request_one(&i2c->dev, aw889x->irq_gpio,
            GPIOF_DIR_IN, "aw889x_int");
        if (ret){
            dev_err(&i2c->dev, "%s: int request failed\n", __func__);
            goto err;
        }
    }

    /* hardware reset */
    aw889x_hw_reset(aw889x);

    /* aw889x chip id */
    ret = aw889x_read_chipid(aw889x);
    if (ret < 0) {
        dev_err(&i2c->dev, "%s: aw889x_read_chipid failed ret=%d\n", __func__, ret);
		no_chip = true;
		aw889x_i2c_probe_fail(i2c);
        return -EIO;
    }

	aw889x_sysfs_create_chip_exist();

    /* aw889x device name */
    if (i2c->dev.of_node) {
       dev_set_name(&i2c->dev, "%s", "aw889x_smartpa");
    } else {
        dev_err(&i2c->dev, "%s failed to set device name: %d\n", __func__, ret);
    }

    /* register codec */
    dai = devm_kzalloc(&i2c->dev, sizeof(aw889x_dai), GFP_KERNEL);
    if (!dai)
        return -ENOMEM;
    memcpy(dai, aw889x_dai, sizeof(aw889x_dai));
    pr_info("%s dai->name(%s)\n", __func__, dai->name);

    ret = snd_soc_register_codec(&i2c->dev, &soc_codec_dev_aw889x,
                dai, ARRAY_SIZE(aw889x_dai));
    if (ret < 0) {
        dev_err(&i2c->dev, "%s failed to register aw889x: %d\n", __func__, ret);
        goto err_off;
    }

    /* aw889x irq */
    if (gpio_is_valid(aw889x->irq_gpio) &&
        !(aw889x->flags & AW889X_FLAG_SKIP_INTERRUPTS)) {
        /* register irq handler */
        irq_flags = IRQF_TRIGGER_FALLING | IRQF_ONESHOT;
        ret = devm_request_threaded_irq(&i2c->dev,
                    gpio_to_irq(aw889x->irq_gpio),
                    NULL, aw889x_irq, irq_flags,
                    "aw889x", aw889x);
        if (ret != 0) {
            dev_err(&i2c->dev, "failed to request IRQ %d: %d\n",
                    gpio_to_irq(aw889x->irq_gpio), ret);
            goto err_off;
        }
    } else {
        dev_info(&i2c->dev, "%s skipping IRQ registration\n", __func__);
        /* disable feature support if gpio was invalid */
        aw889x->flags |= AW889X_FLAG_SKIP_INTERRUPTS;
    }

    /* Register the sysfs files for climax backdoor access */
    ret = device_create_bin_file(&i2c->dev, &dev_attr_rw);
    if (ret)
        dev_info(&i2c->dev, "%s error creating sysfs files: rw\n", __func__);
    ret = device_create_bin_file(&i2c->dev, &dev_attr_regaddr);
    if (ret)
        dev_info(&i2c->dev, "%s error creating sysfs files: regaddr\n", __func__);

    dev_set_drvdata(&i2c->dev, aw889x);
    ret = sysfs_create_group(&i2c->dev.kobj, &aw889x_attribute_group);
    if (ret < 0) {
        dev_info(&i2c->dev, "%s error creating sysfs attr files\n", __func__);
    }

    pr_info("%s probe completed successfully!\n", __func__);

    return 0;

err_off:

err:
    return ret;
}

static int aw889x_i2c_remove(struct i2c_client *i2c)
{
    struct aw889x *aw889x = i2c_get_clientdata(i2c);

    pr_info("%s enter\n", __func__);

    device_remove_bin_file(&i2c->dev, &dev_attr_regaddr);
    device_remove_bin_file(&i2c->dev, &dev_attr_rw);

    snd_soc_unregister_codec(&i2c->dev);

    if (gpio_is_valid(aw889x->irq_gpio))
        devm_gpio_free(&i2c->dev, aw889x->irq_gpio);
    if (gpio_is_valid(aw889x->reset_gpio))
        devm_gpio_free(&i2c->dev, aw889x->reset_gpio);

    return 0;
}

static const struct i2c_device_id aw889x_i2c_id[] = {
    { "tfa98xx", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, aw889x_i2c_id);

static struct of_device_id aw889x_dt_match[] = {
    { .compatible = "nxp,tfa98xx" },
    { },
};

static struct i2c_driver aw889x_i2c_driver = {
    .driver = {
        .name = AW889X_I2C_NAME,
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(aw889x_dt_match),
    },
    .probe = aw889x_i2c_probe,
    .remove = aw889x_i2c_remove,
    .id_table = aw889x_i2c_id,
};


static int __init aw889x_i2c_init(void)
{
    int ret = 0;

    pr_info("aw889x driver version %s\n", AW889X_VERSION);

    ret = i2c_add_driver(&aw889x_i2c_driver);
    if(ret){
        pr_err("fail to add aw889x device into i2c\n");
        return ret;
    }

    return 0;
}
module_init(aw889x_i2c_init);


static void __exit aw889x_i2c_exit(void)
{
    i2c_del_driver(&aw889x_i2c_driver);
}
module_exit(aw889x_i2c_exit);


MODULE_DESCRIPTION("ASoC AW889X Smart PA Driver");
MODULE_LICENSE("GPL v2");
