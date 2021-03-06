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

#define LOG_TAG "isp_u_binning4awb"

#include "isp_drv.h"

cmr_s32 isp_u_binning4awb_block(isp_handle handle, void *block_info)
{
	cmr_s32 ret = 0;
	struct isp_file *file = NULL;
	struct isp_io_param param;

	if (!handle || !block_info) {
		ISP_LOGE("handle is null error: 0x%lx 0x%lx", (cmr_uint) handle, (cmr_uint) block_info);
		return -1;
	}

	file = (struct isp_file *)(handle);
	param.isp_id = file->isp_id;
	param.sub_block = ISP_BLOCK_BINNING;
	param.property = ISP_PRO_BINNING_BLOCK;
	param.property_param = block_info;

	ret = ioctl(file->fd, SPRD_ISP_IO_CFG_PARAM, &param);

	return ret;
}

cmr_s32 isp_u_binning4awb_bypass(isp_handle handle, cmr_u32 bypass)
{
	cmr_s32 ret = 0;
	struct isp_file *file = NULL;
	struct isp_io_param param;

	if (!handle) {
		ISP_LOGE("handle is null error.");
		return -1;
	}

	file = (struct isp_file *)(handle);
	param.isp_id = file->isp_id;
	param.sub_block = ISP_BLOCK_BINNING;
	param.property = ISP_PRO_BINNING_BYPASS;
	param.property_param = &bypass;

	ret = ioctl(file->fd, SPRD_ISP_IO_CFG_PARAM, &param);

	return ret;
}

cmr_s32 isp_u_binning4awb_endian(isp_handle handle, cmr_u32 endian)
{
	cmr_s32 ret = 0;
	struct isp_file *file = NULL;
	struct isp_io_param param;

	if (!handle) {
		ISP_LOGE("handle is null error.");
		return -1;
	}

	file = (struct isp_file *)(handle);
	param.isp_id = file->isp_id;
	param.sub_block = ISP_BLOCK_BINNING;
	param.property = ISP_PRO_BINNING_ENDIAN;
	param.property_param = &endian;
	ISP_LOGE("endian %d", endian);
	ret = ioctl(file->fd, SPRD_ISP_IO_CFG_PARAM, &param);

	return ret;
}

cmr_s32 isp_u_binning4awb_scaling_ratio(isp_handle handle, cmr_u32 vertical, cmr_u32 horizontal)
{
	cmr_s32 ret = 0;
	struct isp_file *file = NULL;
	struct isp_io_param param;
	struct isp_scaling_ratio scaling_ratio;

	if (!handle) {
		ISP_LOGE("handle is null error.");
		return -1;
	}

	file = (struct isp_file *)(handle);
	param.isp_id = file->isp_id;
	param.sub_block = ISP_BLOCK_BINNING;
	param.property = ISP_PRO_BINNING_SCALING_RATIO;
	scaling_ratio.vertical = vertical;
	scaling_ratio.horizontal = horizontal;
	param.property_param = &scaling_ratio;

	ret = ioctl(file->fd, SPRD_ISP_IO_CFG_PARAM, &param);

	return ret;
}

cmr_s32 isp_u_binning4awb_get_scaling_ratio(isp_handle handle, cmr_u32 * vertical, cmr_u32 * horizontal)
{
	cmr_s32 ret = 0;
	struct isp_file *file = NULL;
	struct isp_io_param param;
	struct isp_scaling_ratio scaling_ratio;

	if (!handle) {
		ISP_LOGE("handle is null error.");
		return -1;
	}

	file = (struct isp_file *)(handle);
	param.isp_id = file->isp_id;
	param.sub_block = ISP_BLOCK_BINNING;
	param.property = ISP_PRO_BINNING_GET_SCALING_RATIO;
	param.property_param = &scaling_ratio;

	ret = ioctl(file->fd, SPRD_ISP_IO_CFG_PARAM, &param);

	*vertical = scaling_ratio.vertical;
	*horizontal = scaling_ratio.horizontal;

	return ret;
}

cmr_s32 isp_u_binning4awb_mem_addr(isp_handle handle, cmr_u32 phy_addr)
{
	cmr_s32 ret = 0;
	struct isp_file *file = NULL;
	struct isp_io_param param;

	if (!handle) {
		ISP_LOGE("handle is null error.");
		return -1;
	}

	file = (struct isp_file *)(handle);
	param.isp_id = file->isp_id;
	param.sub_block = ISP_BLOCK_BINNING;
	param.property = ISP_PRO_BINNING_MEM_ADDR;
	param.property_param = &phy_addr;

	ret = ioctl(file->fd, SPRD_ISP_IO_CFG_PARAM, &param);

	return ret;
}

cmr_s32 isp_u_binning4awb_statistics_buf(isp_handle handle, cmr_u32 * buf_id)
{
	cmr_s32 ret = 0;
	struct isp_file *file = NULL;
	struct isp_io_param param;
	cmr_u32 id;

	if (!handle) {
		ISP_LOGE("handle is null error.");
		return -1;
	}

	file = (struct isp_file *)(handle);
	param.isp_id = file->isp_id;
	param.sub_block = ISP_BLOCK_BINNING;
	param.property = ISP_PRO_BINNING_STATISTICS_BUF;
	param.property_param = &id;

	ret = ioctl(file->fd, SPRD_ISP_IO_CFG_PARAM, &param);
	*buf_id = id;

	return ret;
}

cmr_s32 isp_u_binning4awb_transaddr(isp_handle handle, cmr_u32 phys0, cmr_u32 phys1)
{
	cmr_s32 ret = 0;
	struct isp_file *file = NULL;
	struct isp_io_param param;
	struct isp_b4awb_phys phys_addr;

	if (!handle) {
		ISP_LOGE("handle is null error: %lx", (cmr_uint) handle);
		return -1;
	}

	file = (struct isp_file *)(handle);
	param.isp_id = file->isp_id;
	param.sub_block = ISP_BLOCK_BINNING;
	param.property = ISP_PRO_BINNING_TRANSADDR;
	phys_addr.phys0 = phys0;
	phys_addr.phys1 = phys1;
	param.property_param = &phys_addr;

	ret = ioctl(file->fd, SPRD_ISP_IO_CFG_PARAM, &param);

	return ret;
}

cmr_s32 isp_u_binning4awb_initbuf(isp_handle handle)
{
	cmr_s32 ret = 0;
	struct isp_file *file = NULL;
	struct isp_io_param param;

	if (!handle) {
		ISP_LOGE("handle is null error: %lx", (cmr_uint) handle);
		return -1;
	}

	file = (struct isp_file *)(handle);
	param.isp_id = file->isp_id;
	param.sub_block = ISP_BLOCK_BINNING;
	param.property = ISP_PRO_BINNING_INITBUF;
	param.property_param = &ret;

	ret = ioctl(file->fd, SPRD_ISP_IO_CFG_PARAM, &param);

	return ret;
}
