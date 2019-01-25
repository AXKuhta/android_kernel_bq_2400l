
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/spi/spidev.h>
#include <linux/miscdevice.h>
#include <linux/regulator/consumer.h>

#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_platform.h>
#endif
#include <linux/of_gpio.h>

#include "spi_ese.h"

static struct spi_ese_device *g_spi_ese_dev = NULL;
static struct regulator *vddsim2_supply;

/*----------------------------------------------------------------------------*/
static int spi_ese_enable_simvdd2 (void)
{
    int ret = 0;
    pr_err("%s: enter\n", __func__);
    if (IS_ERR(vddsim2_supply)) {
        ret = PTR_ERR(vddsim2_supply);
        pr_err("failed to get regulator vddsim2\n");
    } else {
        ret = regulator_enable(vddsim2_supply);
        if (ret < 0) {
            pr_err("failed to enable vddsim2 regulator\n");
        } else {
            ret = regulator_set_voltage(vddsim2_supply, 1800000, 1800000);
            if (ret < 0)
                pr_err("failed to set voltage vddsim2\n");
        }
    }
    return ret;
}

static int spi_ese_disable_simvdd2 (void)
{
    int ret = 0;
    pr_err("%s: enter\n", __func__);
    if (IS_ERR(vddsim2_supply)) {
        ret = PTR_ERR(vddsim2_supply);
        pr_err("failed to get regulator vddsim2\n");
    } else {
        ret = regulator_disable(vddsim2_supply);
    }

    return ret;
}

static int spi_ese_cs_set_output_high(struct spi_ese_device *spi_ese_dev)
{
    int error = 0;

    error = gpio_direction_output(spi_ese_dev->cs_pin, 1);
    if (error < 0)
        pr_err("%s: gpio direction failed, error: %d\n",
            __func__, error);
    else
        gpio_set_value(spi_ese_dev->cs_pin, 1);

    return error;
}

static int spi_ese_cs_set_output_low(struct spi_ese_device *spi_ese_dev)
{
    int error = 0;

    error = gpio_direction_output(spi_ese_dev->cs_pin, 0);
    if (error < 0)
        pr_err("%s: gpio direction failed, error: %d\n",
            __func__, error);
    else
        gpio_set_value(spi_ese_dev->cs_pin, 0);

    return error;
}

static int spi_ese_cs_set_input(struct spi_ese_device *spi_ese_dev)
{
    int error = 0;

    error = gpio_direction_input(spi_ese_dev->cs_pin);
    if (error < 0)
        pr_err("%s: gpio direction failed, error: %d\n",
            __func__, error);
    return error;
}

static int spi_ese_open (struct inode *inode, struct file *filp)
{
    struct spi_ese_device *spi_ese_dev = g_spi_ese_dev;

    filp->private_data = spi_ese_dev;

    return 0;
}

/* -------------------------------------------------------------------- */
/* file operation function                                                                                */
/* -------------------------------------------------------------------- */
static long spi_ese_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct spi_ese_device *spi_ese_dev = filp->private_data;
    int error = 0;

    switch (cmd) {
    case IOCTL_SPI_ESE_ENABLE_SIMVDD2:
        error = spi_ese_enable_simvdd2();
        break;

    case IOCTL_SPI_ESE_DISABLE_SIMVDD2:
        error = spi_ese_disable_simvdd2();
        break;

    case IOCTL_SPI_ESE_SET_CSPIN_OUTPUT_HIGH:
        error = spi_ese_cs_set_output_high(spi_ese_dev);
        break;

    case IOCTL_SPI_ESE_SET_CSPIN_OUTPUT_LOW:
        error = spi_ese_cs_set_output_low(spi_ese_dev);
        break;

    case IOCTL_SPI_ESE_SET_CSPIN_INPUT:
        error = spi_ese_cs_set_input(spi_ese_dev);
        break;

    default:
        error = -ENOTTY;
        break;
    }

    return error;
}

/*----------------------------------------------------------------------------*/
static const struct file_operations spi_ese_fops = {
    .owner = THIS_MODULE,
    .open  = spi_ese_open,
    .unlocked_ioctl = spi_ese_ioctl,
};

/*----------------------------------------------------------------------------*/
static struct miscdevice spi_ese_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = SPI_DRV_NAME,
    .fops = &spi_ese_fops,
};

/*----------------------------------------------------------------------------*/
static int  spi_ese_probe (struct spi_device *spi)
{
    struct spi_ese_device *spi_ese_dev = NULL;
    int error;
    pr_err("spi_ese_probe enter\n");

    spi_ese_dev = devm_kzalloc(&spi->dev, sizeof (struct spi_ese_device), GFP_KERNEL);
    if (!spi_ese_dev)
        return -ENOMEM;
    g_spi_ese_dev = spi_ese_dev;

    spi_ese_dev->cs_pin = of_get_named_gpio(spi->dev.of_node, "spi-ese,csn-gpio", 0);
    pr_info("%s: csn : %d\n",__func__, spi_ese_dev->cs_pin);

    /* configure the gpio pins */
    error = gpio_request_one(spi_ese_dev->cs_pin, GPIOF_IN, "ese_csn");
    if (error < 0) {
        pr_err("Unable to request gpio ese_csn\n");
    }

    misc_register (&spi_ese_misc_device);

    vddsim2_supply = devm_regulator_get_optional(&spi->dev, "vddsim2");
    pr_err("spi_ese_probe ok\n");
    return 0;
}

/*----------------------------------------------------------------------------*/
#ifdef CONFIG_OF
static struct of_device_id spi_ese_of_table[] = {
    {.compatible = "sprd,spi-ese", },
    {},
};
MODULE_DEVICE_TABLE(of, spi_ese_of_table);
#endif

static struct spi_driver spi_ese_driver = {
    .driver = {
        .name	= SPI_DRV_NAME,
        .bus	= &spi_bus_type,
        .owner	= THIS_MODULE,
#ifdef CONFIG_OF
        .of_match_table = spi_ese_of_table,
#endif
    },
    .probe	= spi_ese_probe,
};

module_spi_driver(spi_ese_driver);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("sprd");
MODULE_DESCRIPTION ("sprd spi ese.");

