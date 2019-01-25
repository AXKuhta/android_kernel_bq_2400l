#ifndef __SPI_ESE_H
#define __SPI_ESE_H

#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/ioctl.h>
#include <linux/spi/spi.h>

#define SPI_DRV_NAME	"spi_ese"


#define IOCTL_MAGIC_NO			0xBB
#define IOCTL_SPI_ESE_ENABLE_SIMVDD2	                   _IO(IOCTL_MAGIC_NO, 1)
#define IOCTL_SPI_ESE_DISABLE_SIMVDD2	                   _IO(IOCTL_MAGIC_NO, 2)
#define IOCTL_SPI_ESE_SET_CSPIN_OUTPUT_HIGH	                   _IO(IOCTL_MAGIC_NO, 3)
#define IOCTL_SPI_ESE_SET_CSPIN_OUTPUT_LOW	                   _IO(IOCTL_MAGIC_NO, 4)
#define IOCTL_SPI_ESE_SET_CSPIN_INPUT	                   _IO(IOCTL_MAGIC_NO, 5)

struct spi_ese_device {
    unsigned int cs_pin;
    struct spi_device *spi;
};

#endif

