#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#ifndef __PACKET_H
#define __PACKET_H

#include <utils/Log.h>

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "MODEM_CTRL"
#endif

#ifdef MODEM_DEBUG
#define MODEM_LOGIF ALOGD
#else
#define MODEM_LOGIF(x...)
#endif

#define MODEM_LOGE ALOGE
#define MODEM_LOGD ALOGD


#define cpu2be16(wValue) (((wValue & 0xFF) << 8) | ((wValue >> 8) & 0xFF))
#define cpu2be32(dwValue)                                       \
  (((dwValue & 0xFF) << 24) | (((dwValue >> 8) & 0xFF) << 16) | \
   (((dwValue >> 16) & 0xFF) << 8) | (dwValue >> 24))

extern int send_connect_message(int fd, int flag);
extern int send_start_message(int fd, int size, unsigned long addr, int flag);
extern int send_end_message(int fd, int flag);
extern int send_data_message(int fd, char *buffer, int data_size, int flag,
                             int image_size, int image_fd);
extern int send_exec_message(int fd, unsigned long addr, int flag);
extern int uart_send_change_spi_mode_message(int fd, int max_transfer_size);
extern unsigned short frm_chk(const unsigned short *src, int len);
extern unsigned short boot_checksum(const unsigned char *src, int len);
unsigned short calculate_crc(unsigned short crc, char const *buffer, int len);
#endif  //__PACKET_H
