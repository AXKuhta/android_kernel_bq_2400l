/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * V1.0
 */
 /*History
 *Date                  Modification                                 Reason
 *
 */

#ifndef _SENSOR_GC033A_MIPI_RAW_H_
#define _SENSOR_GC033A_MIPI_RAW_H_


#include <utils/Log.h>
#include "sensor.h"
#include "jpeg_exif_header.h"
#include "sensor_drv_u.h"
#include "sensor_raw.h"

#include "parameters/sensor_gc033a_raw_param_main.c"

//#define FEATURE_OTP

#define VENDOR_NUM 			1
#define SENSOR_NAME			"gc033a_mipi_raw"
#define I2C_SLAVE_ADDR 			0x42 		/* 8bit slave address*/

#define GC033A_PID_ADDR			0xf0
#define GC033A_PID_VALUE		0x03
#define GC033A_VER_ADDR			0xf1
#define GC033A_VER_VALUE		0x3a

/* sensor parameters begin */

/* effective sensor output image size */
#define PREVIEW_WIDTH			640
#define PREVIEW_HEIGHT			480
#define SNAPSHOT_WIDTH			640 
#define SNAPSHOT_HEIGHT			480

/*Raw Trim parameters*/
#define PREVIEW_TRIM_X			0
#define PREVIEW_TRIM_Y			0
#define PREVIEW_TRIM_W			640
#define PREVIEW_TRIM_H			480
#define SNAPSHOT_TRIM_X			0
#define SNAPSHOT_TRIM_Y			0
#define SNAPSHOT_TRIM_W			640
#define SNAPSHOT_TRIM_H			480

/*Mipi output*/
#define LANE_NUM			1
#define RAW_BITS			10

#define PREVIEW_MIPI_PER_LANE_BPS	  144  /* 2*Mipi clk */
#define SNAPSHOT_MIPI_PER_LANE_BPS	  144  /* 2*Mipi clk */

/*line time unit: 1ns*/
#define PREVIEW_LINE_TIME		  65800
#define SNAPSHOT_LINE_TIME		  65800

/* frame length*/
#define PREVIEW_FRAME_LENGTH		506
#define SNAPSHOT_FRAME_LENGTH		506

/* please ref your spec */
#define FRAME_OFFSET			0
#define SENSOR_MAX_GAIN 0xabcd
#define SENSOR_BASE_GAIN		0x40
#define SENSOR_MIN_SHUTTER		6

/* please ref your spec
 * 1 : average binning
 * 2 : sum-average binning
 * 4 : sum binning
 */
#define BINNING_FACTOR			1

/* please ref spec
 * 1: sensor auto caculate
 * 0: driver caculate
 */
/* sensor parameters end */

/* isp parameters, please don't change it*/
#define ISP_BASE_GAIN			0x80

/* please don't change it */
#define EX_MCLK				24

#define IMAGE_NORMAL_MIRROR 
//#define IMAGE_H_MIRROR 
//#define IMAGE_V_MIRROR 
//#define IMAGE_HV_MIRROR 

#ifdef IMAGE_NORMAL_MIRROR
#define MIRROR 0x54
#define STARTY 0x03
#define STARTX 0x01
#endif

#ifdef IMAGE_H_MIRROR
#define MIRROR 0x55
#define STARTY 0x03
#define STARTX 0x02
#endif

#ifdef IMAGE_V_MIRROR
#define MIRROR 0x56
#define STARTY 0x02
#define STARTX 0x01
#endif

#ifdef IMAGE_HV_MIRROR
#define MIRROR 0x57
#define STARTY 0x02
#define STARTX 0x02
#endif


/*==============================================================================
 * Description:
 * register setting
 *============================================================================*/

static const SENSOR_REG_T gc033a_init_setting[] = {
	/*SYS*/
	{0xfe,0x80},
	{0xfe,0x80},
	{0xfe,0x80},
	{0xf7,0x01},
	{0xf8,0x05},
	{0xf9,0x0f},
	{0xfa,0x00},
	{0xfc,0x0f},
	{0xfe,0x00},
	
	/*ANALOG & CISCTL*/
	{0xfe,0x00},
	{0x03,0x01},
	{0x04,0xc5},
	{0x05,0x03},
	{0x06,0x7b},
	{0x07,0x00},
	{0x08,0x06},
	{0x0a,0x00},
	{0x0c,0x08},
	{0x0d,0x01},
	{0x0e,0xe8},
	{0x0f,0x02},
	{0x10,0x88},
	{0x17,MIRROR},//Don't Change Here!!!
	{0x18,0x12},
	{0x19,0x07},
	{0x1a,0x1b},
	{0x1d,0x48},//40 travis20160318
	{0x1e,0x50},
	{0x1f,0x80},
	{0x23,0x01},
	{0x24,0xc8},
	{0x27,0xaf},
	{0x28,0x24},
	{0x29,0x1a},
	{0x2f,0x14},
	{0x30,0x00},
	{0x31,0x04},	
	{0x32,0x08},
	{0x33,0x0c},
	{0x34,0x0d},
	{0x35,0x0e},
	{0x36,0x0f},
	{0x72,0x98},
	{0x73,0x1a},
	{0x74,0x47},
	{0x76,0x82},
	{0x7a,0xcb},
	{0xc2,0x0c},
	{0xce,0x03},
	{0xcf,0x48},
	{0xeb,0x78},
	{0xd0,0x10},
	{0xdc,0x75},
  
	/*ISP*/
	{0xfe,0x00},
	{0x89,0x03},
	{0x8d,0x02},
	{0x90,0x01},
	{0x92,STARTY},//Don't Change Here!!!
	{0x94,STARTX},//Don't Change Here!!!
	{0x95,0x01},
	{0x96,0xe0},
	{0x97,0x02},
	{0x98,0x80},
	
	/*Gain*/
	{0xfe,0x00}, 
	{0xb0,0x50},
	{0xb1,0x01},
	{0xb2,0x00},	
	{0xb3,0x40},
	{0xb4,0x40},
	{0xb5,0x40},
	{0xb6,0x00},
	
	/*BLK*/
	{0x40,0x26},
	{0x4f,0x3c},
	{0x4e,0x00},
	
	/*Dark Sun*/
	{0xfe,0x00},
	{0xe0,0x9f},
	{0xe1,0x90},
	{0xe4,0x0f},
	{0xe5,0xff},
	{0xfe,0x00}, 
	
	/*MIPI*/
	{0xfe,0x03},
	{0x10,0x00},	
	{0x01,0x03},
	{0x02,0x22},
	{0x03,0x96},
	{0x04,0x01},
	{0x05,0x00},
	{0x06,0x80},
	{0x11,0x2b},
	{0x12,0x20},
	{0x13,0x03},
	{0x15,0x00},//02->00
	{0x21,0x10},
	{0x22,0x00},
	{0x23,0x10},
	{0x24,0x10},
	{0x25,0x10},
	{0x26,0x03},
	{0x29,0x01},
	{0x2a,0x0a},
	{0x2b,0x03},
	{0xfe,0x00},
	{0xf9,0x0e},
	{0xfc,0x0e},	
	{0xfe,0x00},
	{0x25,0xa2},
	{0x3f,0x1a},
};

static const SENSOR_REG_T gc033a_preview_setting[] = {
};

static const SENSOR_REG_T gc033a_snapshot_setting[] = {
};

static struct sensor_res_tab_info s_gc033a_resolution_tab_raw[VENDOR_NUM] = {
	{
      .module_id = MODULE_SUNNY,
      .reg_tab = {
        {ADDR_AND_LEN_OF_ARRAY(gc033a_init_setting), PNULL, 0,
        .width = 0, .height = 0,
        .xclk_to_sensor = EX_MCLK, .image_format = SENSOR_IMAGE_FORMAT_RAW},
		
       /* {ADDR_AND_LEN_OF_ARRAY(gc033a_preview_setting), PNULL, 0,
        .width = PREVIEW_WIDTH, .height = PREVIEW_HEIGHT,
        .xclk_to_sensor = EX_MCLK, .image_format = SENSOR_IMAGE_FORMAT_RAW},*/

        {ADDR_AND_LEN_OF_ARRAY(gc033a_snapshot_setting), PNULL, 0,
        .width = SNAPSHOT_WIDTH, .height = SNAPSHOT_HEIGHT,
        .xclk_to_sensor = EX_MCLK, .image_format = SENSOR_IMAGE_FORMAT_RAW}
		}
	}

/*If there are multiple modules,please add here*/
};

static SENSOR_TRIM_T s_gc033a_resolution_trim_tab[VENDOR_NUM] = {
{
     .module_id = MODULE_SUNNY,
     .trim_info = {
       {0, 0, 0, 0, 0, 0, 0, {0, 0, 0, 0}},
	   
	   /*{.trim_start_x = PREVIEW_TRIM_X, .trim_start_y = PREVIEW_TRIM_Y,
        .trim_width = PREVIEW_TRIM_W,   .trim_height = PREVIEW_TRIM_H,
        .line_time = PREVIEW_LINE_TIME, .bps_per_lane = PREVIEW_MIPI_PER_LANE_BPS,
        .frame_line = PREVIEW_FRAME_LENGTH,
        .scaler_trim = {.x = PREVIEW_TRIM_X, .y = PREVIEW_TRIM_Y, .w = PREVIEW_TRIM_W, .h = PREVIEW_TRIM_H}},*/
       
	   {
        .trim_start_x = SNAPSHOT_TRIM_X, .trim_start_y = SNAPSHOT_TRIM_Y,
        .trim_width = SNAPSHOT_TRIM_W,   .trim_height = SNAPSHOT_TRIM_H,
        .line_time = SNAPSHOT_LINE_TIME, .bps_per_lane = SNAPSHOT_MIPI_PER_LANE_BPS,
        .frame_line = SNAPSHOT_FRAME_LENGTH,
        .scaler_trim = {.x = SNAPSHOT_TRIM_X, .y = SNAPSHOT_TRIM_Y, .w = SNAPSHOT_TRIM_W, .h = SNAPSHOT_TRIM_H}},
		}
	}

    /*If there are multiple modules,please add here*/

};

static SENSOR_REG_T gc033a_shutter_reg[] = {
    {0xfe, 0x00}, 
    {0x03, 0x01}, 
    {0x04, 0xc5}, 
};

static struct sensor_i2c_reg_tab gc033a_shutter_tab = {
    .settings = gc033a_shutter_reg, 
	.size = ARRAY_SIZE(gc033a_shutter_reg),
};

static SENSOR_REG_T gc033a_again_reg[] = {
	{0xfe, 0x00}, 
	{0xb6, 0x00}, 
	{0xb1, 0x01}, 
	{0xb2, 0x00}, 
};

static struct sensor_i2c_reg_tab gc033a_again_tab = {
    .settings = gc033a_again_reg, 
	.size = ARRAY_SIZE(gc033a_again_reg),
};

static SENSOR_REG_T gc033a_dgain_reg[] = {
   
};

static struct sensor_i2c_reg_tab gc033a_dgain_tab = {
    .settings = gc033a_dgain_reg, 
	.size = ARRAY_SIZE(gc033a_dgain_reg),
};

static SENSOR_REG_T gc033a_frame_length_reg[] = {

};

static struct sensor_i2c_reg_tab gc033a_frame_length_tab = {
    .settings = gc033a_frame_length_reg,
    .size = ARRAY_SIZE(gc033a_frame_length_reg),
};

static struct sensor_aec_i2c_tag gc033a_aec_info = {
    .slave_addr = (I2C_SLAVE_ADDR >> 1),
    .addr_bits_type = SENSOR_I2C_REG_8BIT,
    .data_bits_type = SENSOR_I2C_VAL_8BIT,
    .shutter = &gc033a_shutter_tab,
    .again = &gc033a_again_tab,
    .dgain = &gc033a_dgain_tab,
    .frame_length = &gc033a_frame_length_tab,
};


static SENSOR_STATIC_INFO_T s_gc033a_static_info[VENDOR_NUM] = {
    {.module_id = MODULE_SUNNY,
     .static_info = {
        .f_num = 280,
        .focal_length = 354,
        .max_fps = 30,
        .max_adgain = 4,
        .ois_supported = 0,
        .pdaf_supported = 0,
        .exp_valid_frame_num = 1,
        .clamp_level = 0,
        .adgain_valid_frame_num = 1,
        .fov_info = {{4.614f, 3.444f}, 4.222f}}
    }
    /*If there are multiple modules,please add here*/
};

static SENSOR_MODE_FPS_INFO_T s_gc033a_mode_fps_info[VENDOR_NUM] = {
    {.module_id = MODULE_SUNNY,
       {.is_init = 0,
         {{SENSOR_MODE_COMMON_INIT, 0, 1, 0, 0},
         {SENSOR_MODE_PREVIEW_ONE, 0, 1, 0, 0},
         {SENSOR_MODE_SNAPSHOT_ONE_FIRST, 0, 1, 0, 0},
         {SENSOR_MODE_SNAPSHOT_ONE_SECOND, 0, 1, 0, 0},
         {SENSOR_MODE_SNAPSHOT_ONE_THIRD, 0, 1, 0, 0},
         {SENSOR_MODE_PREVIEW_TWO, 0, 1, 0, 0},
         {SENSOR_MODE_SNAPSHOT_TWO_FIRST, 0, 1, 0, 0},
         {SENSOR_MODE_SNAPSHOT_TWO_SECOND, 0, 1, 0, 0},
         {SENSOR_MODE_SNAPSHOT_TWO_THIRD, 0, 1, 0, 0}}}
    }
    /*If there are multiple modules,please add here*/
};


static struct sensor_module_info s_gc033a_module_info_tab[VENDOR_NUM] = {
    {.module_id = MODULE_SUNNY,
     .module_info = {
         .major_i2c_addr = I2C_SLAVE_ADDR >> 1,
         .minor_i2c_addr = I2C_SLAVE_ADDR >> 1,

         .reg_addr_value_bits = SENSOR_I2C_REG_8BIT | SENSOR_I2C_VAL_8BIT |
                                SENSOR_I2C_FREQ_400,

         .avdd_val = SENSOR_AVDD_2800MV,
         .iovdd_val = SENSOR_AVDD_1800MV,
         .dvdd_val = SENSOR_AVDD_1800MV,

         .image_pattern = SENSOR_IMAGE_PATTERN_RAWRGB_B,

         .preview_skip_num = 1,
         .capture_skip_num = 1,
         .flash_capture_skip_num = 3,
         .mipi_cap_skip_num = 0,
         .preview_deci_num = 0,
         .video_preview_deci_num = 0,

         .threshold_eb = 0,
         .threshold_mode = 0,
         .threshold_start = 0,
         .threshold_end = 0,

         .sensor_interface = {
              .type = SENSOR_INTERFACE_TYPE_CSI2,
              .bus_width = LANE_NUM,
              .pixel_width = RAW_BITS,
              .is_loose = 0,
          },
         .change_setting_skip_num = 1,
         .horizontal_view_angle = 65,
         .vertical_view_angle = 60
      }
    }

/*If there are multiple modules,please add here*/
};

static struct sensor_ic_ops s_gc033a_ops_tab;
struct sensor_raw_info *s_gc033a_mipi_raw_info_ptr = &s_gc033a_mipi_raw_info;


/*==============================================================================
 * Description:
 * sensor all info
 * please modify this variable acording your spec
 *============================================================================*/
SENSOR_INFO_T g_gc033a_mipi_raw_info = {
    .hw_signal_polarity = SENSOR_HW_SIGNAL_PCLK_P | SENSOR_HW_SIGNAL_VSYNC_P |
                          SENSOR_HW_SIGNAL_HSYNC_P,
    .environment_mode = SENSOR_ENVIROMENT_NORMAL | SENSOR_ENVIROMENT_NIGHT,
    .image_effect = SENSOR_IMAGE_EFFECT_NORMAL |
                    SENSOR_IMAGE_EFFECT_BLACKWHITE | SENSOR_IMAGE_EFFECT_RED |
                    SENSOR_IMAGE_EFFECT_GREEN | SENSOR_IMAGE_EFFECT_BLUE |
                    SENSOR_IMAGE_EFFECT_YELLOW | SENSOR_IMAGE_EFFECT_NEGATIVE |
                    SENSOR_IMAGE_EFFECT_CANVAS,

    .wb_mode = 0,
    .step_count = 7,
    .reset_pulse_level = SENSOR_LOW_PULSE_RESET,
    .reset_pulse_width = 50,
    .power_down_level = SENSOR_HIGH_LEVEL_PWDN,
    .identify_count = 1,
    .identify_code =
        {{ .reg_addr = GC033A_PID_ADDR, .reg_value = GC033A_PID_VALUE},
         { .reg_addr = GC033A_VER_ADDR, .reg_value = GC033A_VER_VALUE}},

    .source_width_max = SNAPSHOT_WIDTH,
    .source_height_max = SNAPSHOT_HEIGHT,
    .name = (cmr_s8 *)SENSOR_NAME,
    .image_format = SENSOR_IMAGE_FORMAT_RAW,

    .module_info_tab = s_gc033a_module_info_tab,
    .module_info_tab_size = ARRAY_SIZE(s_gc033a_module_info_tab),

    .resolution_tab_info_ptr = s_gc033a_resolution_tab_raw,
    .sns_ops = &s_gc033a_ops_tab,
    .raw_info_ptr = &s_gc033a_mipi_raw_info_ptr,

    .video_tab_info_ptr = NULL,
    .sensor_version_info = (cmr_s8 *)"gc033a_v1",
};

#endif
