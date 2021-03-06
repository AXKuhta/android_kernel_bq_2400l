/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _ISP_PARAM_TUNE_COM_H_
#define _ISP_PARAM_TUNE_COM_H_
#include <sys/types.h>
#include "sensor_drv_u.h"

#ifdef   __cplusplus
extern "C" {
#endif

typedef cmr_s32(*isp_fun) (cmr_handle isp_handler, void *param_ptr);

#define ISP_AE_TAB_NUM 0x04
#define ISP_CMC_NUM 0x09
#define ISP_AWB_NUM 0x09
#define ISP_MAP_NUM 0x08

#define ISP_END_ID 0xffff

#define ISP_PACKET_VERIFY_ID 0x71717567
#define ISP_PACKET_END_ID 0x69656e64

#define ISP_TOOL_CMD_ID 0x80000000

#define ISP_PARASER_VERSION_INFO_SIZE sizeof(struct sensor_version_info)

#define ISP_PARSER_DOWN 0x0000
#define ISP_PARSER_UP_PARAM   0x0001
#define ISP_PARSER_UP_PRV_DATA 0x0003
#define ISP_PARSER_UP_CAP_DATA 0x0004
#define ISP_PARSER_UP_CAP_SIZE 0x0005
#define ISP_PARSER_UP_MAIN_INFO 0x0006
#define ISP_PARSER_UP_SENSOR_REG 0x0007
#define ISP_PARSER_UP_INFO 0x0008

#define ISP_TYPE_CMD   0x0000
#define ISP_TYPE_PARAM 0x0001
#define ISP_TYPE_LEVEL 0x0002
#define ISP_TYPE_PRV_DATA  0x0003
#define ISP_TYPE_CAP_DATA  0x0004
#define ISP_TYPE_MAIN_INFO 0x0005
#define ISP_TYPE_SENSOR_REG 0x0006
#define ISP_TYPE_INFO 0x0007

#define ISP_PACKET_ALL 0x0000
#define ISP_PACKET_BLC 0x0001
#define ISP_PACKET_NLC 0x0002
#define ISP_PACKET_LNC 0x0003
#define ISP_PACKET_AE 0x0004
#define ISP_PACKET_AWB 0x0005
#define ISP_PACKET_BPC 0x0006
#define ISP_PACKET_DENOISE 0x0007
#define ISP_PACKET_GRGB 0x0008
#define ISP_PACKET_CFA 0x0009
#define ISP_PACKET_CMC 0x000A
#define ISP_PACKET_GAMMA 0x000B
#define ISP_PACKET_UV_DIV 0x000C
#define ISP_PACKET_PREF 0x000D
#define ISP_PACKET_BRIGHT 0x000E
#define ISP_PACKET_CONTRAST 0x000F
#define ISP_PACKET_HIST 0x0010
#define ISP_PACKET_AUTO_CONTRAST 0x0011
#define ISP_PACKET_SATURATION 0x0012
#define ISP_PACKET_CSS 0x0013
#define ISP_PACKET_AF 0x0014
#define ISP_PACKET_EDGE 0x0015
#define ISP_PACKET_SPECIAL_EFFECT 0x0016
#define ISP_PACKET_HDR 0x0017
#define ISP_PACKET_GLOBAL 0x0018
#define ISP_PACKET_CHN 0x0019
#define ISP_PACKET_LNC_PARAM 0x001a
#define ISP_PACKET_AWB_MAP 0x001b
#define ISP_PACKET_MAX 0xFFFF

#define ISP_VIDEO_YUV422_2FRAME (1<<0)
#define ISP_VIDEO_YUV420_2FRAME (1<<1)
#define ISP_VIDEO_NORMAL_RAW10 (1<<2)
#define ISP_VIDEO_MIPI_RAW10 (1<<3)
#define ISP_VIDEO_JPG (1<<4)
#define ISP_VIDEO_YVU420_2FRAME (1<<6)

#define ISP_UINT8 0x01
#define ISP_UINT16 0x02
#define ISP_UINT32 0x04
#define ISP_INT8 0x01
#define ISP_INT16 0x02
#define ISP_INT32 0x04

enum isp_parser_cmd {
	ISP_PREVIEW = 0x00,
	ISP_STOP_PREVIEW,
	ISP_CAPTURE,
	ISP_UP_PREVIEW_DATA,
	ISP_UP_PARAM,
	ISP_TAKE_PICTURE_SIZE,
	ISP_MAIN_INFO,
	ISP_READ_SENSOR_REG,
	ISP_WRITE_SENSOR_REG,
	ISP_BIN_TO_PACKET,
	ISP_PACKET_TO_BIN,
	ISP_INFO,
	ISP_PARSER_CMD_MAX
};

struct isp_main_info {
	char sensor_id[32];
	cmr_u32 version_id;
	cmr_u32 preview_format;
	cmr_u32 preview_size;
	cmr_u32 capture_format;
	cmr_u32 capture_num;
	cmr_u32 capture_size[8];
	char version_info[2][ISP_PARASER_VERSION_INFO_SIZE];
};

struct isp_version_info {
	cmr_u32 version_id;
	cmr_u32 srtuct_size;
	cmr_u32 reserve;
};

struct isp_param_info {
	char main_type[32];
	char sub_type[32];
	char third_type[32];
	cmr_u32 data_type;
	cmr_u32 data_num;
	cmr_u32 addr_offset;
};

struct isp_parser_up_data {
	cmr_u32 format;
	cmr_u32 pattern;
	cmr_u32 width;
	cmr_u32 height;
	cmr_u32 *buf_addr;
	cmr_u32 buf_len;
};

struct isp_parser_buf_in {
	cmr_uint buf_addr;
	cmr_u32 buf_len;
};

struct isp_parser_buf_rtn {
	cmr_uint buf_addr;
	cmr_u32 buf_len;
};

struct isp_parser_cmd_param {
	enum isp_parser_cmd cmd;
	cmr_u32 param[48];	// capture param format/width/height
};

struct isp_param_fun {
	cmr_u32 cmd;
	 cmr_s32(*param_fun) (cmr_handle isp_handler, void *param_ptr);
};

cmr_u32 ispParserGetSizeID(cmr_u32 width, cmr_u32 height);
cmr_s32 ispParser(cmr_handle isp_handler, cmr_u32 cmd, void *in_param_ptr, void *rtn_param_ptr);
cmr_u32 *ispParserAlloc(cmr_u32 size);
cmr_s32 ispParserFree(void *addr);

#ifdef   __cplusplus
}
#endif
#endif
