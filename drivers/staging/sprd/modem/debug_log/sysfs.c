/*
 * Copyright (C) 2014 Spreadtrum Communications Inc.
 *
 * Author: Haibing.Yang <haibing.yang@spreadtrum.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/sysfs.h>

#include "core.h"

static ssize_t freq_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct dbg_log_device *dbg = dev_get_drvdata(dev);
	struct phy_ctx *phy = dbg->phy;
	int ret = 0;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", phy->freq);
	return ret;
}

static ssize_t freq_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf,
				size_t count)
{
	struct dbg_log_device *dbg = dev_get_drvdata(dev);
	struct phy_ctx *phy = dbg->phy;
	u32 channel;
	int ret = 0;

	ret = kstrtouint(buf, 10, &phy->freq);
	if (ret) {
		pr_err("Invalid input for phy freq\n");
		return -EINVAL;
	}

	pr_info("input phy freq is %d\n", phy->freq);

	channel = dbg->channel;
	if (channel) {
		dbg->channel = 0;
		dbg_log_channel_sel(dbg);

		dbg->channel = channel;
		dbg_log_channel_sel(dbg);
	} else
		pr_info("serdes channel was not open\n");

	return count;
}
static DEVICE_ATTR_RW(freq);

static ssize_t channel_show(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)
{
	struct dbg_log_device *dbg = dev_get_drvdata(dev);
	int ret;

	switch (dbg->channel) {
	case CH_DISABLE:
		ret = snprintf(buf, PAGE_SIZE, "DISABLE\n");
		break;
	case CH_EN_TRAINING:
		ret = snprintf(buf, PAGE_SIZE, "TRAINING\n");
		break;
	case CH_EN_WTL:
		ret = snprintf(buf, PAGE_SIZE, "WTL\n");
		break;
	case CH_EN_DBG_SYS:
		ret = snprintf(buf, PAGE_SIZE, "DBG_SYS\n");
		break;
	case CH_EN_DBG_BUS:
		ret = snprintf(buf, PAGE_SIZE, "DBG_BUS\n");
		break;
	case CH_EN_MDAR:
		ret = snprintf(buf, PAGE_SIZE, "MDAR\n");
		break;
	default:
		ret = snprintf(buf, PAGE_SIZE,
			"unknown channel: %u\n", dbg->channel);
		break;
	}

	return ret;
}

static ssize_t channel_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct dbg_log_device *dbg = dev_get_drvdata(dev);
	int ret;
	u32 channel;

	ret = kstrtouint(buf, 10, &channel);
	if (ret) {
		pr_err("error: Invalid input!\n");
		return -EINVAL;
	}

	if (channel >= CH_EN_MAX) {
		pr_err("error: channel not support!\n");
		return -EINVAL;
	}

	pr_info("input serdes channel is %u\n", channel);

	dbg->channel = channel;
	dbg_log_channel_sel(dbg);

	return count;
}
static DEVICE_ATTR_RW(channel);

static struct attribute *dbg_log_attrs[] = {
	&dev_attr_channel.attr,
	&dev_attr_freq.attr,
	NULL,
};
ATTRIBUTE_GROUPS(dbg_log);

int dbg_log_sysfs_init(struct device *dev)
{
	int rc;

	rc = sysfs_create_groups(&dev->kobj, dbg_log_groups);
	if (rc)
		pr_err("create dbg log attr node failed, rc=%d\n", rc);

	return rc;
}
EXPORT_SYMBOL(dbg_log_sysfs_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("leon.he@spreadtrum.com");
MODULE_DESCRIPTION("Add modem dbg log attribute nodes for userspace");
