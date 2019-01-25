/*
 * Copyright (C) 2012 Spreadtrum Communications Inc.
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
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>

#include "core.h"

static int dbg_log_init(struct dbg_log_device *dbg)
{
	if (dbg->is_inited)
		return 0;

	dbg->ops->init(dbg);

	dbg->is_inited = true;

	return 0;
}

static int dbg_log_exit(struct dbg_log_device *dbg)
{
	if (!dbg->is_inited)
		return 0;

	dbg->ops->exit(dbg);

	dbg->is_inited = false;

	return 0;
}

void dbg_log_channel_sel(struct dbg_log_device *dbg)
{
	if (dbg->channel) {
		dbg_log_init(dbg);
		dbg->ops->select(dbg);
	} else {
		dbg->ops->select(dbg);
		dbg_log_exit(dbg);
	}
}

struct dbg_log_device *dbg_log_device_register(struct device *parent,
						struct dbg_log_ops *ops,
						struct phy_ctx *phy)
{
	struct dbg_log_device *dbg;
	struct class *dbg_class;

	dbg = kzalloc(sizeof(struct dbg_log_device), GFP_KERNEL);
	if (!dbg)
		return NULL;

	dbg->phy = phy;
	dbg->ops = ops;

	dbg_class = class_create(THIS_MODULE, "modem");
	if (IS_ERR(dbg_class))
		pr_err("Unable to create modem class\n");

	dbg->dev.class = dbg_class;
	dbg->dev.parent = parent;
	dbg->dev.of_node = parent->of_node;
	dev_set_name(&dbg->dev, "debug-log");
	dev_set_drvdata(&dbg->dev, dbg);

	if (device_register(&dbg->dev)) {
		pr_err("modem dbg log device register failed\n");
		goto err;
	}

	if (dbg_log_sysfs_init(&dbg->dev)) {
		pr_err("create sysfs node failed\n");
		goto err;
	}

	return dbg;

err:
	kfree(dbg);
	return NULL;
}

