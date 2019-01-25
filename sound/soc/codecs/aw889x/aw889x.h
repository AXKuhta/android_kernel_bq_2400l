#ifndef _AW889X_H_
#define _AW889X_H_


/* 
 * i2c transaction on Linux limited to 64k
 * (See Linux kernel documentation: Documentation/i2c/writing-clients)
*/
#define MAX_I2C_BUFFER_SIZE 65536

#define AW889X_FLAG_DSP_START_ON_MUTE   (1 << 0)
#define AW889X_FLAG_SKIP_INTERRUPTS     (1 << 1)
#define AW889X_FLAG_SAAM_AVAILABLE      (1 << 2)
#define AW889X_FLAG_STEREO_DEVICE       (1 << 3)
#define AW889X_FLAG_MULTI_MIC_INPUTS    (1 << 4)
#define AW889X_FLAG_DSP_START_ON        (1 << 5)

#define AW889X_NUM_RATES                9

#define AW889X_MAX_REGISTER             0xff

#define AW889X_DSP_FW_BASE              0x8c00
#define AW889X_DSP_CFG_BASE             0x8380
#define AW889X_DSP_FW_VER_BASE          0x0f80

//#define BST_PASS_THROUGH                /* dsp boost pass through: vbat>4.5V */

enum aw889x_chipid{
    AW8990_ID,
};

enum aw889x_mode_spk_rcv{
    AW889X_SPEAKER_MODE = 0,
    AW889X_RECEIVER_MODE = 1,
};

enum aw889x_dsp_fw_version{
    AW889X_DSP_FW_VER_NONE = 0,
    AW889X_DSP_FW_VER_D = 1,
    AW889X_DSP_FW_VER_E = 2,
};

enum aw889x_dsp_init_state {
    AW889X_DSP_INIT_STOPPED,        /* DSP not running */
    AW889X_DSP_INIT_RECOVER,        /* DSP error detected at runtime */
    AW889X_DSP_INIT_FAIL,           /* DSP init failed */
    AW889X_DSP_INIT_PENDING,        /* DSP start requested */
    AW889X_DSP_INIT_DONE,           /* DSP running */
    AW889X_DSP_INIT_INVALIDATED,    /* DSP was running, requires re-init */
};

enum aw889x_dsp_fw_state {
    AW889X_DSP_FW_NONE = 0,
    AW889X_DSP_FW_PENDING,
    AW889X_DSP_FW_FAIL,
    AW889X_DSP_FW_FAIL_COUNT,
    AW889X_DSP_FW_FAIL_REG_DSP,
    AW889X_DSP_FW_FAIL_PROBE,
    AW889X_DSP_FW_OK,
};

enum aw889x_dsp_cfg_state {
    AW889X_DSP_CFG_NONE = 0,
    AW889X_DSP_CFG_PENDING,
    AW889X_DSP_CFG_FAIL,
    AW889X_DSP_CFG_FAIL_COUNT,
    AW889X_DSP_CFG_FAIL_REG_DSP,
    AW889X_DSP_CFG_FAIL_PROBE,
    AW889X_DSP_CFG_OK,
};


struct aw889x {
    struct regmap *regmap;
    struct i2c_client *i2c;
    struct snd_soc_codec *codec;
    struct mutex dsp_lock;
    int dsp_init;
    int dsp_fw_state;
    int dsp_cfg_state;
    int dsp_fw_len;
    int dsp_cfg_len;
    int dsp_fw_ver;
    int sysclk;
    u16 rev;
    int rate;
    struct device *dev;
    int pstream;
    int cstream;
    struct input_dev *input;

    int rst_gpio;
    int reset_gpio;
    int power_gpio;
    int irq_gpio;

#ifdef CONFIG_DEBUG_FS
    struct dentry *dbg_dir;
#endif
    u8 reg;

    unsigned int flags;
    unsigned int chipid;
    unsigned int init;
    unsigned int spk_rcv_mode;
};

struct aw889x_container{
    int len;
    unsigned char data[];
};

static int aw889x_i2c_write(struct aw889x *aw889x, 
        unsigned char reg_addr, unsigned int reg_data);
static int aw889x_i2c_read(struct aw889x *aw889x,
        unsigned char reg_addr, unsigned int *reg_data);
  
static void aw889x_interrupt_setup(struct aw889x *aw889x);
static void aw889x_interrupt_clear(struct aw889x *aw889x);
#endif
