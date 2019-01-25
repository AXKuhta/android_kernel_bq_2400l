/*
 * Spreadtrum OSC support
 *
 * Copyright (C) 2018 spreadtrum, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/vmalloc.h>
#include <linux/sprd_otp.h>
#include <linux/mfd/sprd/pmic_glb_reg.h>
#include <linux/slab.h>

#include "leds_sprd_light.h"

struct flash_driver_data {
	struct regmap *reg_map;
	spinlock_t slock;
	unsigned int val;
	void *priv;
};

static struct class *light_class;
static struct device *light_dev;
struct flash_driver_data *drv_data;

static void sc2721s_init(struct flash_driver_data *drv_data)
{
	/* flash ctrl */
	regmap_update_bits(drv_data->reg_map,
			   FLASH_CTRL_REG, 0, 0);
}

static int sprd_flash_sc2721s_leds_open_torch(void *drvd, uint8_t idx)
{
	struct flash_driver_data *drv_data = (struct flash_driver_data *)drvd;

	if (!drv_data)
		return -EFAULT;

	if (SPRD_FLASH_LED0 & idx) {
		regmap_update_bits(drv_data->reg_map,
			FLASH_CTRL_REG, FLASH_PON|FLASH_V_SW_SETP2, FLASH_PON|FLASH_V_SW_SETP2);
	}

	if (SPRD_FLASH_LED1 & idx) {
		regmap_update_bits(drv_data->reg_map,
			FLASH_CTRL_REG, FLASH_PON|FLASH_V_SW_SETP2, FLASH_PON|FLASH_V_SW_SETP2);
	}

	return 0;
}

static int sprd_flash_sc2721s_leds_close_torch(void *drvd, uint8_t idx)
{
	struct flash_driver_data *drv_data = (struct flash_driver_data *)drvd;

	if (!drv_data)
		return -EFAULT;

	if (SPRD_FLASH_LED0 & idx) {
		regmap_update_bits(drv_data->reg_map,
					FLASH_CTRL_REG,
					FLASH_PON,
					~(unsigned int)FLASH_PON);
		regmap_update_bits(drv_data->reg_map,
			FLASH_CTRL_REG, FLASH_V_HW_EN,
			~(unsigned int)FLASH_V_HW_EN);
	}

	if (SPRD_FLASH_LED1 & idx) {
		regmap_update_bits(drv_data->reg_map,
					FLASH_CTRL_REG,
					FLASH_PON,
					~(unsigned int)FLASH_PON);
		regmap_update_bits(drv_data->reg_map,
			FLASH_CTRL_REG, FLASH_V_HW_EN,
			~(unsigned int)FLASH_V_HW_EN);
	}

	return 0;
}

static ssize_t leds_show(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%u\n", drv_data->val);
}

static ssize_t leds_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{

	int ret = -EPERM;
	unsigned int val;

	if (!dev)
		goto exit;

	ret = kstrtouint(buf, 10, &val);
	if (ret)
		goto exit;

	switch (val) {
	case 0:
		sprd_flash_sc2721s_leds_close_torch(drv_data,
			SPRD_FLASH_LED0);
		break;
	case 1:
		sprd_flash_sc2721s_leds_open_torch(drv_data,
			SPRD_FLASH_LED0);
		break;
	default:
		break;
	}

	drv_data->val = val;
	return 10;
exit:
	return ret;
}

static struct device_attribute dev_attr_enable = {
	.attr = {
		.name = "enable",
		.mode = 0666,
	},
	.show = leds_show,
	.store = leds_store,
};

static int leds_probe(struct platform_device *pdev)
{
	int err = 0;

	if (pdev == NULL)
		return -EINVAL;

	light_class = class_create(THIS_MODULE,
			"flashlight");
	if (light_class == NULL)
		goto out1;

	light_dev = device_create(light_class, NULL, 0, NULL, "torch");
	if (light_dev == NULL)
		goto out2;

	err = device_create_file(light_dev, &dev_attr_enable);
	if (err < 0)
		goto out3;

	drv_data = devm_kzalloc(&pdev->dev,
			sizeof(struct flash_driver_data),
			GFP_KERNEL);
	if (!drv_data)
		goto out3;

	spin_lock_init(&drv_data->slock);

	spin_lock(&drv_data->slock);
	pdev->dev.platform_data = (void *)drv_data;
	spin_unlock(&drv_data->slock);

	drv_data->reg_map = dev_get_regmap(pdev->dev.parent, NULL);
	if (drv_data->reg_map == NULL)
		goto out4;

	sc2721s_init(drv_data);

	return 0;

out4:
	kfree(drv_data);
out3:
	device_remove_file(light_dev,
		&dev_attr_enable);
out2:
	class_destroy(light_class);
out1:
	return err;
}

static int leds_suspend(struct platform_device *dev, pm_message_t state)
{
	pr_info("entry leds_suspend\n");
	return 0;
}

static int leds_resume(struct platform_device *dev)
{
	pr_info("entry leds_resume\n");
	return 0;
}


static int leds_remove(struct platform_device *pdev)
{
	kfree(drv_data);
	device_remove_file(light_dev,
		&dev_attr_enable);
	class_destroy(light_class);
	return 0;
}

static const struct of_device_id leds_flash_of_match[] = {
	{ .compatible = "sprd,sc2721-flash", },
	{},
};

static struct platform_driver led_light_drv = {
	.probe = leds_probe,
	.remove = leds_remove,
	.suspend = leds_suspend,
	.resume  = leds_resume,
	.driver = {
		.name = "sc2721-flash",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(leds_flash_of_match),
	},
};

module_platform_driver(led_light_drv);

MODULE_AUTHOR("spreadtrum.com");
MODULE_DESCRIPTION("SC272S LED driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:leds-torch");

