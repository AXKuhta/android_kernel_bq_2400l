/*
 *Copyright (C) 2012 Spreadtrum Communications Inc.
 *
 *This software is licensed under the terms of the GNU General Public
 *License version 2, as published by the Free Software Foundation, and
 *may be copied, distributed, and modified under those terms.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 */

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <linux/io.h>
#include <linux/device.h>

#ifdef pr_fmt
#undef pr_fmt
#define pr_fmt(fmt) "[modem-dbg-log][%20s] "fmt,  __func__
#endif

enum channel_en {
	CH_DISABLE = 0,
	CH_EN_TRAINING,
	CH_EN_WTL,
	CH_EN_MDAR,
	CH_EN_DBG_SYS,
	CH_EN_DBG_BUS,
	CH_EN_MAX
};

struct dbg_log_device;

struct dbg_log_ops {
	void (*init)(struct dbg_log_device *dbg);
	void (*exit)(struct dbg_log_device *dbg);
	void (*select)(struct dbg_log_device *dbg);
};

struct phy_ctx {
	unsigned long base;
	unsigned int freq;
	unsigned int lanes;
};

struct dbg_log_device {
	struct device dev;
	struct phy_ctx *phy;
	struct dbg_log_ops *ops;
	u32 channel;
	bool is_inited;
};

static inline void reg_write(unsigned long reg, u32 val)
{
	writel(val, (void __iomem *)reg);
}

static inline u32 reg_read(unsigned long reg)
{
	return readl((void __iomem *)reg);
}

static inline void reg_bits_set(unsigned long reg, u32 bits)
{
	reg_write(reg, reg_read(reg) | bits);
}

static inline void reg_bits_clr(unsigned long reg, u32 bits)
{
	reg_write(reg, reg_read(reg) & ~bits);
}

void dbg_log_channel_sel(struct dbg_log_device *dbg);
struct dbg_log_device *dbg_log_device_register(
			struct device *parent,
			struct dbg_log_ops *ops,
			struct phy_ctx *phy);
int dbg_log_sysfs_init(struct device *dev);

#endif
