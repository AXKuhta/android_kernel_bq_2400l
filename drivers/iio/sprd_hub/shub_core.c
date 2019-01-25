/*
 * File:shub_core.c
 * Author:bao.yue@spreadtrum.com
 * Created:2015-10-21
 * Description:Spreadtrum Sensor Hub Driver
 *
 * Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 */

 #include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/fcntl.h>
#include <linux/firmware.h>
#include <linux/iio/buffer.h>
#include <linux/iio/iio.h>
#include <linux/iio/kfifo_buf.h>
#include <linux/iio/trigger.h>
#include <linux/iio/trigger_consumer.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/sched.h>
#include <linux/sipc.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>
#include <linux/suspend.h>

#include "shub_common.h"
#include "shub_core.h"
#include "shub_protocol.h"
#include "shub_opcode.h"
#include <linux/wakelock.h>

static struct task_struct *thread;
static struct task_struct *thread_nwu;
static DECLARE_WAIT_QUEUE_HEAD(waiter);
static int reader_flag, probe_retry_count;
struct shub_data *g_sensor;
static struct wake_lock sensorhub_wake_lock;

#if SHUB_DATA_DUMP
#define MAX_RX_LEN 102400
static int total_read_byte_cnt;
u8 sipc_rx_data[MAX_RX_LEN];
#endif
/* for debug flush event*/
static int flush_setcnt;
static int flush_getcnt;
#define NS         (int64_t)1000000ll
#define US         1000ll
/*sensor id*/
static struct hw_sensor_id_tag hw_sensor_id[_HW_SENSOR_TOTAL] = {
	{0, 0xFF, 0xFF, ""},
	{0, 0xFF, 0xFF, ""},
	{0, 0xFF, 0xFF, ""},
	{0, 0xFF, 0xFF, ""},
	{0, 0xFF, 0xFF, ""},
	{0, 0xFF, 0xFF, ""},
};
static int get_sensor_id(struct shub_data *sensor);
const char calibration_filename[ANDROID_SENSORS_ID_END][CALIB_PATH_MAX_LENG] = {
	"none",
	"acc",
	"mag",
	"orientation",
	"gyro",
	"light",
	"pressure",
	"tempreature",
	"proximity"
};
static int debug_flag;
static int shub_send_event_to_iio(struct shub_data *sensor, u8 *data, u16 len);
static int shub_download_calibration_data(struct shub_data *sensor);
static void shub_save_calibration_data(struct work_struct *work);
static void shub_synctimestamp(struct shub_data *sensor);

/* send data
  * handler time must less than 5s
  */
static int shub_send_data(struct shub_data *sensor, u8 *buf, u32 len)
{
	int nwrite = 0;
	int sent_len = 0;
	int timeout = 100;
	int retry = 0;

	do {
		nwrite =
			sbuf_write(SIPC_ID_PM_SYS, SMSG_CH_PIPE, SIPC_PM_BUFID0,
			(void *)(buf + sent_len), len - sent_len,
			msecs_to_jiffies(timeout));
		if (nwrite > 0)
			sent_len += nwrite;
		if (nwrite < len || nwrite < 0)
			SH_ERR("nwrite=%d,len=%d,sent_len=%d,timeout=%dms\n",
					nwrite, len, sent_len, timeout);
		/* only handle boot exception */
		if (nwrite < 0) {
			if (nwrite == -ENODEV) {
				msleep(100);
				retry++;
				if (retry > 10)
					break;
			} else {
				SH_ERR("nwrite=%d\n", nwrite);
				SH_ERR("task #: %s, pid = %d, tgid = %d\n",
				current->comm, current->pid, current->tgid);
				WARN_ONCE(1, "%s timeout: %dms\n",
							__func__, timeout);
				/* BUG(); */
				break;
			}
		}
	} while (nwrite < 0 || sent_len  < len);
	return nwrite;
}

static int shub_send_command(struct shub_data *sensor, int sensor_ID,
	enum shub_subtype_id opcode, const char *data, int len)
{
	struct cmd_data cmddata;
	int encode_len;
	int nwrite = 0;
	int ret = 0;

	mutex_lock(&sensor->send_command_mutex);
#if SHUB_DEBUG
	sensor->sent_cmddata.type = sensor_ID;
	sensor->sent_cmddata.sub_type = opcode;
	sensor->sent_cmddata.status = RESPONSE_FAIL;
	sensor->sent_cmddata.condition = false;
#endif
	cmddata.type = sensor_ID;
	cmddata.subtype = opcode;
	cmddata.length = len;
	/* no data command  set default data 0xFF */
	if (len == 0) {
		cmddata.buff[0] = 0xFF;
		cmddata.length = len = 1;
	} else {
		memcpy(cmddata.buff, data, len);
	}
	encode_len =
	    shub_encode_one_packet(&cmddata, sensor->writebuff,
				   SERIAL_WRITE_BUFFER_MAX);
	/* SH_LOG("encode_len= %d\n",encode_len); */

	if (encode_len > SHUB_MAX_HEAD_LEN + SHUB_MAX_DATA_CRC) {
		nwrite =
		    shub_send_data(sensor, sensor->writebuff, encode_len);
	}
       /* SH_LOG("nwrite= %d\n",nwrite); */
	/* command timeout test*/
#if SHUB_DEBUG
	SH_LOG("send_wait begin timetout:msecs_to_jiffies(%d)=%d\n",
			RESPONSE_WAIT_TIMEOUT_MS,
			msecs_to_jiffies(RESPONSE_WAIT_TIMEOUT_MS));
	ret = wait_event_timeout(sensor->rw_wait_queue,
			(true == sensor->sent_cmddata.condition),
			msecs_to_jiffies(RESPONSE_WAIT_TIMEOUT_MS));
	SH_LOG("send_wait end ret=%d\n", ret);
	/* ret= 0 :timeout */
	if (!ret) {
		sensor->sent_cmddata.status = RESPONSE_TIMEOUT;
		ret = RESPONSE_TIMEOUT;
	} else
		ret = sensor->sent_cmddata.status;	/* get from callback */
#endif
      /* SH_LOG("send_wait exit ret=%d\n", ret); */
	ret = nwrite;
	mutex_unlock(&sensor->send_command_mutex);

	return ret;
}

static int shub_sipc_channel_read(struct shub_data *sensor)
{
	int nread = 0;
	int ret = 0;
	int retry = 0;

	do {
		nread =
			sbuf_read(SIPC_ID_PM_SYS, SMSG_CH_PIPE, SIPC_PM_BUFID0,
			(void *)sensor->readbuff,
			SERIAL_READ_BUFFER_MAX - 1, -1);
		if (nread < 0) {
			retry++;
			msleep(500);
			SH_ERR("nread=%d,retry=%d\n", nread, retry);
			if (retry > 20)
				break;
		}
	} while (nread < 0);

	if (nread > 0) {
		/* for debug */
		#if SHUB_DATA_DUMP
		memcpy(sipc_rx_data+total_read_byte_cnt,
				sensor->readbuff, nread);
		total_read_byte_cnt += nread;
		if (total_read_byte_cnt >= MAX_RX_LEN) {
			total_read_byte_cnt = 0;
			memset(sipc_rx_data, 0, sizeof(sipc_rx_data));
		}
		#endif
		ret =
		    shub_parse_one_packet(&shub_stream_processor,
					  sensor->readbuff, nread);
		/* SH_LOG("ret=%d nread=%d\n",ret, nread); */
		memset(sensor->readbuff, 0, sizeof(sensor->readbuff));
	} else
		SH_LOG("can not get data\n");

	return nread;
}

static int shub_sipc_channel_rdnwu(struct shub_data *sensor)
{
	int nread = 0;
	int ret = 0;
	int retry = 0;

	do {
		nread =
			sbuf_read(SIPC_ID_PM_SYS, SMSG_CH_PIPE, SIPC_PM_BUFID1,
			(void *)sensor->readbuff_nwu,
			SERIAL_READ_BUFFER_MAX - 1, -1);
		if (nread < 0) {
			retry++;
			msleep(500);
			SH_ERR("nread=%d,retry=%d\n", nread, retry);
			if (retry > 20)
				break;
		}
	} while (nread < 0);

	if (nread > 0) {
		ret =
		    shub_parse_one_packet(&shub_stream_processor_nwu,
					  sensor->readbuff_nwu, nread);
		/* SH_LOG("ret=%d nread=%d\n",ret, nread); */
		memset(sensor->readbuff_nwu, 0, sizeof(sensor->readbuff));
	} else
		SH_LOG("can not get data\n");

	return nread;
}

static int shub_read_thread(void *data)
{
	struct sched_param param = {.sched_priority = 5 };
	struct shub_data *sensor = data;

	sched_setscheduler(current, SCHED_RR, &param);

	shub_init_parse_packet(&shub_stream_processor);
	wait_event_interruptible(waiter, (reader_flag != 0));
	set_current_state(TASK_RUNNING);
	do {
		shub_sipc_channel_read(sensor);
	} while (!kthread_should_stop());

	return 0;
}

static int shub_read_thread_nwu(void *data)
{
	struct sched_param param = {.sched_priority = 5 };
	struct shub_data *sensor = data;

	sched_setscheduler(current, SCHED_RR, &param);

	shub_init_parse_packet(&shub_stream_processor_nwu);
	wait_event_interruptible(waiter, (reader_flag != 0));
	set_current_state(TASK_RUNNING);
	do {
		shub_sipc_channel_rdnwu(sensor);
	} while (!kthread_should_stop());

	return 0;
}

static void shub_data_callback(struct shub_data *sensor, u8 *data, u32 len)
{
	switch (data[0]) {
	case HAL_FLUSH:
#if SHUB_DATA_DUMP
		flush_getcnt++;
#endif
	case HAL_SEN_DATA:
/*		for(i = 0;i < len;i++)
 *			SH_LOG("len=%d,data[%d]=%d\n",len,i,data[i]);
 */
		shub_send_event_to_iio(sensor, data, len);
		break;

	default:
		break;
	}
}

static void shub_readcmd_callback(struct shub_data *sensor, u8 *data, u32 len)
{
	switch (data[0]) {
	case KNL_CMD:
		if (sensor->rx_buf != NULL && sensor->rx_len ==  (len - 1)) {
			memcpy(sensor->rx_buf, (data + 1), sensor->rx_len);
			sensor->rx_status = true;
			wake_up_interruptible(&sensor->rxwq);
		}
		break;
	default:
		break;
	}

}

static void shub_cm4_read_callback(struct shub_data *sensor,
		enum shub_subtype_id subtype, u8 *data, u32 len)
{
	SH_LOG("subtype=%d\n", subtype);
	switch (subtype) {
	case SHUB_SET_TIMESYNC_SUBTYPE:
		shub_synctimestamp(sensor);
		break;
	default:
		break;
	}

}

static void parse_cmd_response_callback(
	struct shub_data *sensor, u8 *data, u32 len)
{
	struct shub_data *mcu = sensor;

	SH_LOG
	    ("mcu->sent_cmddata.condition=%d data[0]=%d  data[1]=%d\n",
	     mcu->sent_cmddata.condition, data[0], data[1]);
	/* already timeout ,do nothing */
	if (mcu->sent_cmddata.status == RESPONSE_TIMEOUT)
		return;

	/*   the response is  real sent command */
	if (mcu->sent_cmddata.sub_type == data[0]) {
		mcu->sent_cmddata.status = data[1];	/* cmd_resp_status_t */
		/* maybe the write thread is not hold on,
		 * the routine have done
		 */
		mcu->sent_cmddata.condition = true;
		wake_up(&mcu->rw_wait_queue);
		SH_LOG("send_wait wake up rw_wait_queue\n");
	}
}

static int shub_download_opcode(struct shub_data *sensor)
{
	const struct firmware *fw;
	const u8 *fw_data = NULL;
	struct fwshub_head *fw_head = NULL;
	struct iic_unit *fw_body = NULL;
	int ret = 0;
	int j = 0;
	int sensor_type;

	SH_LOG("loading opcode\n");
	for (j = 0; j < SHUB_SENSOR_NUM; j++) {
		if (j == 0) {
			ret =
			    request_firmware(&fw, "shub_fw_accelerometer",
					     &(sensor->sensor_pdev->dev));
			sensor_type = ANDROID_ACCELEROMETER;
		} else if (j == 1) {
			ret =
			    request_firmware(&fw, "shub_fw_gyroscope",
					     &(sensor->sensor_pdev->dev));
			sensor_type = ANDROID_GYROSCOPE;
		} else if (j == 2) {
			ret =
			    request_firmware(&fw, "shub_fw_magnetic",
					     &(sensor->sensor_pdev->dev));
			sensor_type = ANDROID_GEOMAGNETIC_FIELD;
		} else if (j == 3) {
			ret =
			    request_firmware(&fw, "shub_fw_light",
					     &(sensor->sensor_pdev->dev));
			sensor_type = ANDROID_LIGHT;
		} else if (j == 4) {
			ret =
			    request_firmware(&fw, "shub_fw_proximity",
					     &(sensor->sensor_pdev->dev));
			sensor_type = ANDROID_PROXIMITY;
		} else if (j == 5) {
			ret =
			    request_firmware(&fw, "shub_fw_pressure",
					     &(sensor->sensor_pdev->dev));
			sensor_type = ANDROID_PRESSURE;
		} else if (j == 6) {
			ret =
			    request_firmware(&fw, "shub_fw_calibration",
					     &(sensor->sensor_pdev->dev));
			sensor_type = SENSOR_TYPE_CALIBRATION_CFG;
		}
		if (ret)
			SH_ERR("Failed to load firmware: %d\n", ret);
		else {
			SH_LOG("loading opcode success, size is %d\n",
			       (int)fw->size);
			fw_data = fw->data;
			fw_head = (struct fwshub_head *) fw_data;
			fw_body = (struct iic_unit *)
			    (fw_data + sizeof(struct fwshub_head));
			SH_LOG("j= %d:sensor Name = %s, type = %d\n", j,
			       fw_head->name, fw_head->type);
			if (j == 6) {
				ret =
				    shub_send_command(sensor, sensor_type,
					SHUB_DOWNLOAD_CALIBRATION_SUBTYPE,
					fw->data, fw->size);
			} else {
				ret =
				    shub_send_command(sensor, sensor_type,
					SHUB_DOWNLOAD_OPCODE_SUBTYPE, fw->data,
						      fw->size);
			}
			if (j < SHUB_SENSOR_NUM - 1)
				msleep(200);
			release_firmware(fw);
		}

	}

	return ret;
}

static int shub_sipc_read(
	struct shub_data *sensor, u8 reg_addr, u8 *data, u8 len)
{
	int err = 0;
	int wait_ret;

	if (reader_flag == 0) {
		SH_LOG("run kthread\n");
		reader_flag = 1;
		wake_up_interruptible(&waiter);
	}

	mutex_lock(&sensor->mutex_read);
	sensor->rx_buf = data;
	sensor->rx_len = len;
	sensor->rx_status = false;

	shub_send_command(sensor, HANDLE_MAX,
		(enum shub_subtype_id)reg_addr,
		NULL, 0);
	wait_ret =
	    wait_event_interruptible_timeout(sensor->rxwq,
					     (true == sensor->rx_status),
					     msecs_to_jiffies
					     (RECEIVE_TIMEOUT_MS));
	/* SH_LOG("wait_ret=%d,sensor->rx_status=%d,
	  * msecs_to_jiffies(%d)=%d\n", wait_ret,
	  * sensor->rx_status,RECEIVE_TIMEOUT_MS,
	  * msecs_to_jiffies(RECEIVE_TIMEOUT_MS));
	 */
	sensor->rx_buf = NULL;
	sensor->rx_len = 0;
	if (false == sensor->rx_status)
		err = -1;
	mutex_unlock(&sensor->mutex_read);

	return err;
}

static int shub_send_event_to_iio(struct shub_data *sensor, u8 *data, u16 len)
{
	u8 event[MAX_CM4_MSG_SIZE];
	u8 i = 0;

	mutex_lock(&sensor->mutex_send);
	memset(event, 0x00, MAX_CM4_MSG_SIZE);
	memcpy(event, data, len);

	if (sensor->log_control.udata[5] == 1) {
		for (i = 0; i < len; i++)
			SH_LOG("data[%d]=%d  ", i, data[i]);
			SH_LOG("\n");
	}
	/* SH_ERR("%d 0x%08lx\n",len,*(sensor->indio_dev->active_scan_mask)); */
	if (sensor->indio_dev->active_scan_mask &&
			(!bitmap_empty(sensor->indio_dev->active_scan_mask,
			sensor->indio_dev->masklength))) {
		iio_push_to_buffers(sensor->indio_dev, event);
	} else if (sensor->indio_dev->active_scan_mask == NULL) {
		SH_ERR("active_scan_mask = NULL, event might be missing\n");
	}

	mutex_unlock(&sensor->mutex_send);

	return 0;
}

static int shub_download_opcodefile(struct shub_data *sensor)
{
	int ret = 0;


	ret = shub_download_opcode(sensor);

	SH_LOG("ret=%d\n", ret);

	return ret;
}

static void shub_send_ap_status(struct shub_data *sensor, uint8_t status)
{
	int ret = 0;

	SH_LOG("status=%d\n", status);
	if (sensor->mcu_mode <= SHUB_OPDOWNLOAD) {
		SH_LOG("mcu_mode == CW_BOOT!\n");
		return;
	}
	sensor->mcu_mode = status;
	ret = shub_send_command(sensor, HANDLE_MAX,
				      SHUB_SET_HOST_STATUS_SUBTYPE, &status, 1);
	SH_LOG("ret = %d\n", ret);
}

static void shub_synctimestamp(struct shub_data *sensor)
{
	struct timespec kt;
	s64 k_timestamp;

	if (sensor->mcu_mode != SHUB_NORMAL)
		return;
	get_monotonic_boottime(&kt);
	k_timestamp = timespec_to_ns(&kt);
	k_timestamp /= 1000000;
	shub_send_command(sensor,
		HANDLE_MAX,
		SHUB_SET_TIMESYNC_SUBTYPE,
		(char *)&k_timestamp,
		sizeof(k_timestamp));
}

static void shub_synctime_work(struct work_struct *work)
{
	struct shub_data *sensor = container_of((struct delayed_work *)work,
		struct shub_data, delay_work);

	shub_synctimestamp(sensor);
	atomic_set(&sensor->delay, SYNC_TIME_DELAY_MS);
	queue_delayed_work(sensor->driver_wq,
		&sensor->delay_work,
		msecs_to_jiffies(atomic_read(&sensor->delay)));
}

static void shub_download_work(struct work_struct *work)
{
	struct shub_data *sensor = container_of(work,
		struct shub_data, download_work);
	int i;
	int ret;

	if (sensor->mcu_mode == SHUB_NORMAL) {
		shub_download_calibration_data(sensor);
		/*try to read id*/
		for (i = 0; i < 5; i++) {
			ret = get_sensor_id(sensor);
			if (ret >= 0)
				break;
			msleep(200);
		}
		if (i >= 5) {/*fail*/
			for (i = 0; i < _HW_SENSOR_TOTAL; i++)
				hw_sensor_id[i].id_status = _IDSTA_FAIL;
		}
	}
}

static ssize_t reader_show(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	SH_LOG("debug_flag=%d\n", debug_flag);
	return sprintf(buf, "debug_flag=%d\n", debug_flag);
}

static ssize_t reader_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct shub_data *sensor = dev_get_drvdata(dev);
	int len;

	SH_LOG("user echo is:%s  count=%d sensor=%p\n", buf, (int)count,
	       sensor);
	len = sscanf(buf, "%d\n", &debug_flag);
	return count;
}

static ssize_t sipc_data_show(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	#if SHUB_DATA_DUMP
	/* save rx raw data file
	  * productinfo/sensor_calibration_data/none
	  * for debug
	  */
	SH_LOG("total_read_byte_cnt =%d\n", total_read_byte_cnt);
	total_read_byte_cnt = 0;
	memset(sipc_rx_data, 0, sizeof(sipc_rx_data));
	return sprintf(buf, "total_read_byte_cnt=%d\n", total_read_byte_cnt);
	#else
	char *prompt = "not dump data\n";

	return sprintf(buf, "%s", prompt);
	#endif
}

static ssize_t sipc_data_store(struct device *dev,
			       struct device_attribute *attr, const char *buf,
			       size_t count)
{
	struct shub_data *sensor = dev_get_drvdata(dev);

	shub_send_ap_status(sensor, SHUB_NORMAL);
	return count;
}

static ssize_t opcode_download(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct shub_data *sensor = dev_get_drvdata(dev);
	uint8_t data[4], i;
	int16_t version = -1;

	for (i = 0; i < 5; i++) {
		if (sensor->mcu_mode == SHUB_BOOT) {
			if (shub_sipc_read(sensor,
				SHUB_GET_FWVERSION_SUBTYPE,
				data, 4) >= 0) {
				sensor->mcu_mode = SHUB_OPDOWNLOAD;
				version = ((u16)data[1]) << 8 | (u16)data[0];
				SH_LOG("CM4 Version:%d(M:%u,D:%u,V:%u,SV:%u)\n",
					data[3], data[2], data[1],
					data[0], version);
			} else {
				SH_LOG("Get Version Fail retry %d times\n",
						i+1);
				msleep(500);
				continue;
			}
		}
	}

	if (sensor->mcu_mode == SHUB_OPDOWNLOAD) {
		shub_download_opcodefile(sensor);
		sensor->mcu_mode = SHUB_NORMAL;
		schedule_work(&sensor->download_work);
		cancel_delayed_work_sync(&sensor->delay_work);
		queue_delayed_work(sensor->driver_wq, &sensor->delay_work, 0);
	}

	return sprintf(buf, "%d %u\n", version, data[0]);
}

static ssize_t logctl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	char *a = "Description:\n";
	char *b = "\techo \"bit val \" > logctl\n\n";
	char *c = "Detail:\n";
	char *d = "\t bit: [0 ~ 7]\n";
	char *d1 = "\t\tbit0: debug message\n";
	char *d2 = "\t\tbit1: sysfs operate message\n";
	char *d3 = "\t\tbit2: function entry message\n";
	char *d4 = "\t\tbit3: data path message\n";
	char *d5 = "\t\tbit4: dump sensor data\n";
	char *e = "\t val: [0 ~ 1], 0 means close, 1 means open\n\n";
	char *f = "\t bit [8] control all flag, all open or close\n";

	return sprintf(buf, "%s%s%s%s%s%s%s%s%s%s%s",
		a, b, c, d, d1, d2, d3, d4, d5, e, f);
}

static ssize_t logctl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	/*
	* buf:  bit-flag, bit-val
	*         bit-flag, bit-val
	*/
	int idx = 0;
	int i = 0;
	int total = count / 4;
	int len;
	uint32_t bit_flag = 0;
	uint32_t bit_val = 0;

	struct shub_data *sensor = dev_get_drvdata(dev);

	sensor->log_control.cmd = HAL_LOG_CTL;
	sensor->log_control.length = total;

	SH_LOG("logctl_store total[%d], count[%d]\n", total, (int)count);
	SH_LOG("char %s\n", buf);

	for (idx = 0; idx < total; idx++)	{
		len = sscanf(buf + 4 * idx, "%u %u\n", &bit_flag, &bit_val);
		SH_LOG("logctl_store bit_flag[%d], bit_val[%d]\n",
				bit_flag, bit_val);

		if (bit_flag < MAX_SENSOR_LOG_CTL_FLAG_LEN)
			sensor->log_control.udata[bit_flag] = bit_val;

		if (bit_flag == MAX_SENSOR_LOG_CTL_FLAG_LEN) {
			for (i = 0; i < MAX_SENSOR_LOG_CTL_FLAG_LEN; i++)
				sensor->log_control.udata[i] = bit_val;
		}
	}

	shub_send_event_to_iio(sensor, (u8 *)&(sensor->log_control),
	sizeof(sensor->log_control));

	return count;
}
 /*
  * return 0:success
  * return < 0:fail
  */
static void shub_save_calibration_data(struct work_struct *work)
{
	int err = 0, nwrite = 0;
	struct file *pfile = NULL;
	mm_segment_t old_fs;
	char file_path[CALIB_PATH_MAX_LENG];
	struct shub_data *sensor = container_of(work,
		struct shub_data, savecalifile_work);

	/* debuginfor(sensor->calibrated_data,CALIBRATION_DATA_LENGTH); */
	/* change to KERNEL_DS address limit */
	old_fs = get_fs();
	set_fs(KERNEL_DS);

	sprintf(file_path, CALIBRATION_NODE"%s",
		calibration_filename[sensor->cal_id]);
	SH_LOG("sensor_id=%d,file_path=%s\n", sensor->cal_id, file_path);
	pfile = filp_open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (IS_ERR(pfile)) {
		err = PTR_ERR(pfile);
		SH_ERR("open file %s error=%d\n", file_path, err);
		sensor->cal_savests = err;
		set_fs(old_fs);
		return;
	}
	nwrite =
	    vfs_write(pfile, sensor->calibrated_data,
			    CALIBRATION_DATA_LENGTH,
				&pfile->f_pos);
	/* SH_LOG("nwrite=%d\n", nwrite); */
	if (nwrite < 0) {
		SH_ERR("nwrite=%d\n", nwrite);
		err = nwrite;
		sensor->cal_savests = err;
	} else
		sensor->cal_savests = 0;
	if (pfile)
		filp_close(pfile, NULL);

	/* restore previous address limit */
	set_fs(old_fs);
	SH_LOG("end\n");
}

static void shub_save_mag_offset(struct shub_data *sensor,
	u8 *data, u32 len)
{
	int err = 0, nwrite = 0;
	struct file *pfile = NULL;
	mm_segment_t old_fs;
	char file_path[CALIB_PATH_MAX_LENG];

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	sprintf(file_path, CALIBRATION_NODE"%s",
		calibration_filename[2]);
	SH_LOG("file_path=%s\n", file_path);
	pfile = filp_open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (IS_ERR(pfile)) {
		err = PTR_ERR(pfile);
		SH_ERR("open file %s error=%d\n", file_path, err);
		set_fs(old_fs);
		return;
	}
	nwrite =
		vfs_write(pfile, (const char __user *)data,
				CALIBRATION_DATA_LENGTH,
				&pfile->f_pos);

	if (nwrite < 0) {
		SH_ERR("nwrite=%d\n", nwrite);
		err = nwrite;
	}
	if (pfile)
		filp_close(pfile, NULL);

	set_fs(old_fs);
	SH_LOG("end\n");
}

static int shub_download_calibration_data(struct shub_data *sensor)
{
	int err = 0;
	struct file *pfile = NULL;
	mm_segment_t old_fs;
	char file_path[CALIB_PATH_MAX_LENG];
	char *raw_cali_data =  NULL;
	int cal_file_size = 0;
	struct kstat stat;
	int sensor_type = 0, j = 0;

	/* change to KERNEL_DS address limit */
	old_fs = get_fs();
	set_fs(KERNEL_DS);

	for (sensor_type = 0; sensor_type < 9; sensor_type++) {
		SH_LOG("sensor_type=%d,filename=%s\n", sensor_type,
			calibration_filename[sensor_type]);
		sprintf(file_path, CALIBRATION_NODE"%s",
			calibration_filename[sensor_type]);
		SH_LOG("sensor_id=%d file_path=%s\n", sensor_type, file_path);

		pfile = filp_open(file_path, O_RDONLY, 0);
		if (IS_ERR(pfile)) {
			err = (int)PTR_ERR(pfile);
			SH_LOG("open file %s ret=%d\n", file_path, err);
			continue;
		}

		err = vfs_stat(file_path, &stat);
		if (err) {
			SH_ERR("Failed to find file stat\n");
			continue;
		}
		cal_file_size = (int) stat.size;
		SH_LOG("cal_file_size=%d\n", cal_file_size);
		/* check data size */
		if (cal_file_size != CALIBRATION_DATA_LENGTH)
			continue;
		raw_cali_data = kmalloc(cal_file_size, GFP_KERNEL);
		if (raw_cali_data == NULL) {
			SH_ERR("Failed to allocate raw_cali_data memory\n");
			continue;
		}
		if (vfs_read(pfile, raw_cali_data, cal_file_size, &pfile->f_pos)
			!= cal_file_size) {
			SH_ERR("Error: file read failed\n");
			kfree(raw_cali_data);
			raw_cali_data = NULL;
			if (pfile)
				filp_close(pfile, NULL);
			continue;
		}
		/* for debug */
		for (j = 0; j < cal_file_size; j++)
			SH_LOG("raw_cali_data[%d]=%d\n", j, raw_cali_data[j]);

		err = shub_send_command(sensor, sensor_type,
				      SHUB_SET_CALIBRATION_DATA_SUBTYPE,
				      raw_cali_data, cal_file_size);
		kfree(raw_cali_data);
		raw_cali_data = NULL;
		if (pfile)
			filp_close(pfile, NULL);

		SH_LOG("%s calibration success\n",
			calibration_filename[sensor_type]);
	}
	/* restore previous address limit */
	set_fs(old_fs);
	return err;
}

static ssize_t active_set(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct shub_data *sensor = dev_get_drvdata(dev);
	int handle, enabled, len;
	enum shub_subtype_id subtype;

	SH_LOG("buf=%s\n", buf);
	if (sensor->mcu_mode <= SHUB_OPDOWNLOAD) {
		SH_LOG("[%s]mcu_mode == %d!\n", __func__, sensor->mcu_mode);
		return count;
	}

	len = sscanf(buf, "%d %d\n", &handle, &enabled);
	SH_LOG("handle = %d, enabled = %d\n", handle, enabled);
	subtype = (enabled == 0) ? SHUB_SET_DISABLE_SUBTYPE :
		SHUB_SET_ENABLE_SUBTYPE;
	if (shub_send_command(sensor, handle, subtype, NULL, 0) < 0)
		SH_ERR("Write SetEn/Disable fail\n");

	return count;
}

static ssize_t batch_set(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	struct shub_data *sensor = dev_get_drvdata(dev);
	int flag = 0;
	struct sensor_batch_cmd batch_cmd;

	SH_LOG("buf=%s\n", buf);
	if (sensor->mcu_mode <= SHUB_OPDOWNLOAD) {
		SH_LOG("mcu_mode == %d!\n",  sensor->mcu_mode);
		return count;
	}

	sscanf(buf, "%d %d %d %lld\n",
		&batch_cmd.handle, &flag,
		&batch_cmd.report_rate,
		&batch_cmd.batch_timeout);
	SH_LOG("handle = %hu, rate = %d, enabled = %d\n",
		batch_cmd.handle,
		batch_cmd.report_rate, flag);

	if (shub_send_command(sensor, batch_cmd.handle,
			SHUB_SET_BATCH_SUBTYPE,
			(char *)&batch_cmd.report_rate,
			12) < 0)
		SH_ERR("Write batch_set Fail\n");

	return count;
}

static ssize_t flush_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	SH_FUN();
	return sprintf(buf, "flush_setcnt=%d,flush_getcnt=%d\n",
		flush_setcnt, flush_getcnt);
}

static ssize_t flush_set(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	struct shub_data *sensor = dev_get_drvdata(dev);
	int handle, len;

	SH_LOG("buf=%s\n", buf);
	if (sensor->mcu_mode <= SHUB_OPDOWNLOAD) {
		SH_ERR("mcu_mode == SHUB_BOOT\n");
		return count;
	}
	len = sscanf(buf, "%d\n", &handle);
#if SHUB_DATA_DUMP
	flush_setcnt++;
#endif
	if (shub_send_command(sensor, handle,
		SHUB_SET_FLUSH_SUBTYPE,
		NULL, 0x00) < 0)
		SH_LOG("Write flush_set Fail\n");

	wake_lock_timeout(&(sensorhub_wake_lock), 200);

	return count;
}

static int set_calib_cmd(struct shub_data *sensor, uint8_t cmd, uint8_t id,
			 uint8_t type, int golden_value)
{
	char data[6];
	int err;

	if (sensor->mcu_mode <= SHUB_OPDOWNLOAD) {
		SH_ERR("mcu_mode == SHUB_BOOT\n");
		return -1;
	}

	data[0] = cmd;
	data[1] = type;
	memcpy(&data[2], &golden_value, sizeof(golden_value));
	err = shub_send_command(sensor, id,
		SHUB_SET_CALIBRATION_CMD_SUBTYPE,
		data, sizeof(data));
	if (!err)
		SH_LOG("Write CalibratorCmd Fail\n");

	return err;
}

static ssize_t mcu_mode_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	struct shub_data *sensor = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", sensor->mcu_mode);
}

static ssize_t mcu_model_set(struct device *dev, struct device_attribute *attr,
			     const char *buf, size_t count)
{
	struct shub_data *sensor = dev_get_drvdata(dev);
	int mode = 0;
	int len;

	len = sscanf(buf, "%d\n", &mode);
	sensor->mcu_mode = mode;
	return count;
}

static ssize_t set_calibrator_cmd(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct shub_data *sensor = dev_get_drvdata(dev);
	int err, len;

	if (sensor->mcu_mode <= SHUB_OPDOWNLOAD) {
		SH_ERR("mcu_mode == SHUB_BOOT!\n");
		return count;
	}

	SH_LOG("buf=%s\n", buf);
	len = sscanf(buf, "%d %d %d %d\n", &sensor->cal_cmd, &sensor->cal_id,
	       &sensor->cal_type, &sensor->golden_sample);
	err = set_calib_cmd(sensor, sensor->cal_cmd, sensor->cal_id,
			  sensor->cal_type, sensor->golden_sample);
	SH_LOG("cmd:%d,id:%d,type:%d,golden:%d\n", sensor->cal_cmd,
		       sensor->cal_id, sensor->cal_type, sensor->golden_sample);
	if (err < 0)
		SH_ERR(" Write Fail!\n");

	return count;
}

static ssize_t get_calibrator_cmd(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct shub_data *sensor = dev_get_drvdata(dev);

	return sprintf(buf, "Cmd:%d,Id:%d,Type:%d\n", sensor->cal_cmd,
		       sensor->cal_id, sensor->cal_type);
}

static ssize_t get_calibrator_data(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct shub_data *sensor = dev_get_drvdata(dev);
	uint8_t cal_data[CALIBRATION_DATA_LENGTH] = { 0 };
	int err, i, n = 0;

	if (sensor->mcu_mode <= SHUB_OPDOWNLOAD) {
		SH_ERR("mcu_mode == SHUB_BOOT!\n");
		return 0;
	}

	err = shub_sipc_read(sensor,
		SHUB_GET_CALIBRATION_DATA_SUBTYPE,
		cal_data, CALIBRATION_DATA_LENGTH);
	if (err < 0)
		SH_LOG(" Read CalibratorData Fail\n");

	SH_LOG("cmd:%d,id:%d,type:%d\n", sensor->cal_cmd,
		       sensor->cal_id, sensor->cal_type);
	if (sensor->cal_cmd == CALIB_DATA_READ) {
		/* debuginfor(cal_data,CALIBRATION_DATA_LENGTH); */
		sensor->calibrated_data = cal_data;
		sensor->cal_savests = 1;
		schedule_work(&sensor->savecalifile_work);
		while (sensor->cal_savests == 1) {
			msleep(100);
			SH_LOG("saving cali data...\n");
		}
		/* 0 success ,else error */
		err = sensor->cal_savests;
		SH_LOG("done err=%d\n", err);
		return sprintf(buf, "%d\n", err);
	}
	if (sensor->cal_cmd == CALIB_CHECK_STATUS)
		return sprintf(buf, "%d\n", cal_data[0]);

	for (i = 0; i < CALIBRATION_DATA_LENGTH; i++)
		n += sprintf(buf + n, "%d ", cal_data[i]);
	return n;
}

static ssize_t set_calibrator_data(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct shub_data *sensor = dev_get_drvdata(dev);
	char data[30];
	int temp[CALIBRATION_DATA_LENGTH] = { 0 };
	int i, err;

	if (sensor->mcu_mode <= SHUB_OPDOWNLOAD) {
		SH_ERR("mcu_mode == SHUB_BOOT!\n");
		return count;
	}

	sscanf(buf,
	       "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
	       &temp[0], &temp[1], &temp[2], &temp[3], &temp[4], &temp[5],
	       &temp[6], &temp[7], &temp[8], &temp[9], &temp[10], &temp[11],
	       &temp[12], &temp[13], &temp[14], &temp[15], &temp[16], &temp[17],
	       &temp[18], &temp[19], &temp[20], &temp[21], &temp[22], &temp[23],
	       &temp[24], &temp[25], &temp[26], &temp[27], &temp[28],
	       &temp[29]);

	for (i = 0; i < 30; i++)
		data[i] = (uint8_t) temp[i];
	err = shub_send_command(sensor, HANDLE_MAX,
		SHUB_SET_CALIBRATION_DATA_SUBTYPE,
		data, sizeof(data));
	if (err < 0)
		SH_ERR("Write CalibratorData Fail\n");

	return count;
}

static ssize_t version_show(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	struct shub_data *sensor = dev_get_drvdata(dev);
	uint8_t data[4];
	int16_t version = -1;

	SH_FUN();
	if (sensor == NULL) {
		SH_ERR("sensor data fail\r\n");
		return -1;
	}

	if (sensor->mcu_mode <= SHUB_OPDOWNLOAD) {
		SH_LOG("mcu_mode == SHUB_BOOT!\n");
		return -1;
	}

	if (shub_sipc_read(sensor, SHUB_GET_FWVERSION_SUBTYPE, data, 4) >= 0) {
		version =
		    (int16_t) (((uint16_t) data[1]) << 8 | (uint16_t) data[0]);
		SH_LOG(" Check FW Version[3-0]: (M:%u,D:%u,V:%u,SV:%u)\n",
		       data[3], data[2], data[1], data[0]);
	} else {
		SH_LOG("Read  FW Version Fail\n");
	}

	return sprintf(buf, "%d\n", version);
}

static ssize_t get_raw_data0(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	struct shub_data *sensor = dev_get_drvdata(dev);
	uint8_t data[6];
	uint16_t *ptr;
	int err;

	ptr = (uint16_t *) data;
	if (sensor->mcu_mode <= SHUB_OPDOWNLOAD) {
		SH_LOG("mcu_mode == SHUB_BOOT!\n");
		return -1;
	}

	err = shub_sipc_read(sensor,
		SHUB_GET_ACCELERATION_RAWDATA_SUBTYPE, data, 6);
	if (err < 0) {
		SH_ERR("read RegMapR_GetAccelerationRawData failed!\n");
		return -1;
	}
	SH_LOG("RawData0:%u,%u,%u)\n", ptr[0], ptr[1], ptr[2]);
	return sprintf(buf, "%d %u %u %u\n", err, ptr[0], ptr[1], ptr[2]);
}

static ssize_t get_raw_data1(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	struct shub_data *sensor = dev_get_drvdata(dev);
	uint8_t data[6];
	uint16_t *ptr;
	int err;

	ptr = (uint16_t *) data;
	if (sensor->mcu_mode <= SHUB_OPDOWNLOAD) {
		SH_LOG("mcu_mode == SHUB_BOOT!\n");
		return -1;
	}
	err = shub_sipc_read(sensor,
		SHUB_GET_MAGNETIC_RAWDATA_SUBTYPE, data, 6);
	if (err < 0) {
		SH_ERR("read RegMapR_GetMagneticRawData failed!\n");
		return err;
	}
	SH_LOG("RawData1:%u,%u,%u)\n", ptr[0], ptr[1], ptr[2]);
	return sprintf(buf, "%d %u %u %u\n", err, ptr[0], ptr[1], ptr[2]);
}

static ssize_t get_raw_data2(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	struct shub_data *sensor = dev_get_drvdata(dev);
	uint8_t data[6];
	uint16_t *ptr;
	int err;

	ptr = (uint16_t *) data;
	if (sensor->mcu_mode <= SHUB_OPDOWNLOAD) {
		SH_LOG("mcu_mode == SHUB_BOOT!\n");
		return -1;
	}
	err = shub_sipc_read(sensor,
		SHUB_GET_GYROSCOPE_RAWDATA_SUBTYPE, data, 6);
	if (err < 0) {
		SH_ERR("read RegMapR_GetGyroRawData failed!\n");
		return err;
	}
	SH_LOG("RawData2:%u,%u,%u)\n", ptr[0], ptr[1], ptr[2]);
	return sprintf(buf, "%d %u %u %u\n", err, ptr[0], ptr[1], ptr[2]);
}

static ssize_t get_raw_data3(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	struct shub_data *sensor = dev_get_drvdata(dev);
	uint8_t data[6];
	uint16_t *ptr;
	int err;

	ptr = (uint16_t *) data;
	if (sensor->mcu_mode <= SHUB_OPDOWNLOAD) {
		SH_LOG("mcu_mode == SHUB_BOOT!\n");
		return -1;
	}

	err = shub_sipc_read(sensor, SHUB_GET_LIGHT_RAWDATA_SUBTYPE, data, 6);
	if (err < 0) {
		SH_ERR("read RegMapR_GetLightRawData failed!\n");
		return err;
	}
	SH_LOG("RawData3:%u,%u,%u)\n", ptr[0], ptr[1], ptr[2]);
	return sprintf(buf, "%d %u %u %u\n", err, ptr[0], ptr[1], ptr[2]);
}

static ssize_t get_raw_data4(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	struct shub_data *sensor = dev_get_drvdata(dev);
	uint8_t data[6];
	uint16_t *ptr;
	int err;

	SH_FUN();
	ptr = (uint16_t *) data;
	if (sensor->mcu_mode <= SHUB_OPDOWNLOAD) {
		SH_LOG("mcu_mode == SHUB_BOOT!\n");
		return -1;
	}
	err = shub_sipc_read(sensor,
		SHUB_GET_PROXIMITY_RAWDATA_SUBTYPE, data, 6);
	if (err < 0) {
		SH_ERR("read RegMapR_GetProximityRawData failed!\n");
		return err;
	}
	SH_LOG("RawData4:%u,%u,%u)\n", ptr[0], ptr[1], ptr[2]);
	return sprintf(buf, "%d %u %u %u\n", err, ptr[0], ptr[1], ptr[2]);
}

static ssize_t sensorhub_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct shub_data *sensor = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", sensor->is_sensorhub);
}

static ssize_t sensorhub_store(struct device *dev,
			struct device_attribute *attr,
			 const char *buf, size_t count)
{
	int len;
	struct shub_data *sensor = dev_get_drvdata(dev);

	SH_LOG("buf=%s\n", buf);
	len = sscanf(buf, "%d\n",
		&sensor->is_sensorhub);
	return count;
}
/*sensor id*/
static char *id_toname(u32 v_id)
{/*id = 256 * vendorid + chip id*/
	static const struct id_to_name_tag name_list[] = {
		{0x0000, "don't know"},
		{0x0011, "icm20600,Acc and Gyro DEVICE"},
		{0x003d, "st,lis3mdl"},
		{0x0069, "st,lsm6ds3"},
		{0x006A, "st,lsm6dsl"},
		{0x0033, "st,lis3dh"},
		{0x00BD, "st,lps25h"},
		{0x00E0, "rpr0521,ALSPRX DEVICE"},
		{0x00F8, "BOSCH, bma223"},
		{0x0006, "DYNA, ap3426"},
		{0x0092, "ltr553"},
		{0x0080, "ltr558"},
		{0x0068, "epl2182"},
		{0x0088, "epl2590"},
		{0x0048, "akm09916,Magnetometer"},
	};
	int i;

	for (i = 0; i < sizeof(name_list) / sizeof(struct
		id_to_name_tag); i++) {

		if (v_id == name_list[i].id)
			return name_list[i].pname;
	}
	return name_list[0].pname;
}
static int get_sensor_id(struct shub_data *sensor)
{
	u8 buf[2 * _HW_SENSOR_TOTAL];/* 2bytes per sensor*/
	int err;
	int i;

	err = shub_sipc_read(sensor,
		SHUB_GET_SENSORINFO_SUBTYPE,
		buf, sizeof(buf));
	if (err < 0) {
		SH_ERR("spic read fail, err %d\n", err);
		return err;
	}

	for (i = 0; i < _HW_SENSOR_TOTAL; i++) {
		if (sensor->log_control.udata[5] == 1)
			SH_LOG("[%d]0x%02x 0x%02x\n", i,
				buf[i * 2], buf[i * 2 + 1]);

		hw_sensor_id[i].id_status = buf[i * 2];
		hw_sensor_id[i].vendor_id = 0x00;
		hw_sensor_id[i].chip_id = buf[i * 2 + 1];
		hw_sensor_id[i].pname = id_toname(hw_sensor_id[i].chip_id);
		if (hw_sensor_id[i].id_status == _IDSTA_NOT)
			hw_sensor_id[i].id_status = _IDSTA_FAIL;
	}
	return 0;
}
static ssize_t hwsensor_id_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	int i;
	int len = 0;
	u32 temp_id;
	const char * const TB_TYPENAME[] = {
		"acc", "mag", "gyr",
		"prox", "light", "prs"};

	SH_FUN();
	/*get_sensor_id(g_sensor);*/

	for (i = 0; i < _HW_SENSOR_TOTAL; i++) {
		if (i > 0)
			len += sprintf(buf + len, "\n");

		len += snprintf(buf + len, 10, "%s:%d",
			TB_TYPENAME[i],
			hw_sensor_id[i].id_status);

		if (hw_sensor_id[i].id_status == _IDSTA_OK &&
			hw_sensor_id[i].pname != NULL) {

			temp_id = hw_sensor_id[i].vendor_id * 256 +
					hw_sensor_id[i].chip_id;
			len += snprintf(buf + len, 32, "(0x%04x, %s)",
				temp_id, hw_sensor_id[i].pname);
		}
	}
	len += snprintf(buf + len, 50,
		"\n#read id status:0 not ready, 1 OK, 2 fail.\n");

	return len;
}

static ssize_t sensor_info_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	int i;
	int len = 0;
	const char * const SENSOR_TYPENAME[] = {
		"acc", "mag", "gyro",
		"prox", "light", "prs"};

	for (i = 0; i < _HW_SENSOR_TOTAL;) {

		len += snprintf(buf + len, 16, "%d %s name:",
			(hw_sensor_id[i].id_status == _IDSTA_OK),
			SENSOR_TYPENAME[i]);

		if (hw_sensor_id[i].id_status == _IDSTA_OK &&
			hw_sensor_id[i].pname != NULL) {

			len += snprintf(buf + len, 32, "%s",
				hw_sensor_id[i].pname);
		}

		len += sprintf(buf + len, "\n");
		i += 1;
	}

	return len;
}

static ssize_t mag_cali_flag_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	char data_buf[16];
	unsigned short mag_cali_check;
	int len;
	int flag = 0;
	long mag_library_size = 0;
	struct shub_data *sensor = dev_get_drvdata(dev);

	len = sscanf(buf, "%d %ld %hu", &flag, &mag_library_size,
			&mag_cali_check);
	SH_LOG("the flag = %d, the mag size is %ld,	crc_check = %d\r\n",
			flag, mag_library_size, mag_cali_check);

	memcpy(data_buf, &flag, sizeof(flag));
	memcpy(data_buf + sizeof(flag), &mag_library_size,
			sizeof(mag_library_size));
	memcpy(data_buf + sizeof(flag) + sizeof(mag_library_size),
			&mag_cali_check, sizeof(mag_cali_check));

	shub_send_command(sensor, SHUB_NODATA,
		SHUB_SEND_MAG_CALIBRATION_FLAG,
		data_buf, 16);

	return 0;
}

static ssize_t shub_debug_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct shub_data *sensor = dev_get_drvdata(dev);
	char *a = "Description:\n";
	char *b = "\techo \"op sid reg val mask\" > shub_debug\n";
	char *c = "\tcat shub_debug\n\n";
	char *d = "Detail:\n";
	char *e = "\t op(operator): 0:open 1:close 2:read 3:write\n";
	char *f = "\t sid(sensorid): 0:acc 1:mag 2:gyro ";
	char *g = "3:proximity 4:light 5:pressure 6:new_dev\n";
	char *h = "\t reg: the operate register that you want\n";
	char *i = "\t value: The value to be written or show read value\n";
	char *j = "\t mask: The mask of the operation\n\n";
	char *k = "\t status: the state of execution. 1:success 0:fail\n\n";
	char l[50], m[50];

	snprintf(l, sizeof(l), "[shub_debug]operate:0x%x sensor_id:0x%x",
		sensor->log_control.debug_data[0],
		sensor->log_control.debug_data[1]);
	snprintf(m, sizeof(m), " reg:0x%x value:0x%x status:0x%x\n\n",
		sensor->log_control.debug_data[2],
		sensor->log_control.debug_data[3],
		sensor->log_control.debug_data[4]);

	return sprintf(buf, "\n%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		a, b, c, d, e, f, g, h, i, j, k, l, m);

}

static ssize_t shub_debug_store(struct device *dev,
			struct device_attribute *attr,
			 const char *buf, size_t count)
{
	int len;
	u8 data_buf[5] = {0};
	struct shub_data *sensor = dev_get_drvdata(dev);

	len = sscanf(buf, "%4hhx %4hhx %4hhx %4hhx %4hhx\n",
			&data_buf[0], &data_buf[1], &data_buf[2],
			&data_buf[3], &data_buf[4]);

	shub_send_command(sensor, SHUB_NODATA,
		SHUB_SEND_DEBUG_DATA,
		data_buf, sizeof(data_buf));

	return count;
}

static struct device_attribute attributes[] = {
	__ATTR(debug_data, 0644, sipc_data_show, sipc_data_store),
	__ATTR(reader_enable, 0644, reader_show, reader_store),
	__ATTR(op_download, 0444, opcode_download, NULL),
	__ATTR(logctl, 0644, logctl_show, logctl_store),
	__ATTR(enable, 0200,  NULL, active_set),
	__ATTR(batch, 0200, NULL, batch_set),
	__ATTR(flush, 0644, flush_show, flush_set),
	__ATTR(mcu_mode, 0644, mcu_mode_show, mcu_model_set),
	__ATTR(calibrator_cmd, 0644, get_calibrator_cmd, set_calibrator_cmd),
	__ATTR(calibrator_data, 0644, get_calibrator_data, set_calibrator_data),
	__ATTR(version, 0444, version_show, NULL),
	__ATTR(raw_data0, 0444, get_raw_data0, NULL),
	__ATTR(raw_data1, 0444, get_raw_data1, NULL),
	__ATTR(raw_data2, 0444, get_raw_data2, NULL),
	__ATTR(raw_data3, 0444, get_raw_data3, NULL),
	__ATTR(raw_data4, 0444, get_raw_data4, NULL),
	__ATTR(sensorhub, 0644, sensorhub_show, sensorhub_store),
	__ATTR(hwsensor_id, 0444, hwsensor_id_show, NULL),
	__ATTR(sensor_info, 0444, sensor_info_show, NULL),
	__ATTR(mag_cali_flag, 0200, NULL, mag_cali_flag_store),
	__ATTR(shub_debug, 0644, shub_debug_show, shub_debug_store),
};

static int shub_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int shub_resume(struct platform_device *pdev)
{
	return 0;
}

static int shub_notifier_fn(struct notifier_block *nb,
			unsigned long action, void *data)
{
	struct shub_data *sensor = container_of(nb, struct shub_data,
			early_suspend);

	switch (action) {
	case PM_SUSPEND_PREPARE:
		SH_LOG("shub_suspend.\n");
		cancel_delayed_work_sync(&sensor->delay_work);
		shub_synctimestamp(sensor);
		shub_send_ap_status(sensor, SHUB_SLEEP);
		break;
	case PM_POST_SUSPEND:
		SH_LOG("shub_resume.\n");
		shub_send_ap_status(sensor, SHUB_NORMAL);
		queue_delayed_work(sensor->driver_wq, &sensor->delay_work, 0);
		break;
	default:
		break;
	}
	return 0;
}

/*=======iio device reg=========*/
static void iio_trigger_work(struct irq_work *work)
{
	struct shub_data *mcu_data =
	    container_of((struct irq_work *)work,
					struct shub_data, iio_irq_work);
	iio_trigger_poll(mcu_data->trig);
}

static irqreturn_t shub_trigger_handler(int irq, void *p)
{
	struct iio_poll_func *pf = p;
	struct iio_dev *indio_dev = pf->indio_dev;
	struct shub_data *mcu_data = iio_priv(indio_dev);

	SH_LOG("%s", __func__);
	mutex_lock(&mcu_data->mutex_lock);
	iio_trigger_notify_done(mcu_data->indio_dev->trig);
	mutex_unlock(&mcu_data->mutex_lock);
	return IRQ_HANDLED;
}

static const struct iio_buffer_setup_ops shub_iio_buffer_setup_ops = {
	.postenable = iio_triggered_buffer_postenable,
	.predisable = iio_triggered_buffer_predisable,
};

static int shub_pseudo_irq_enable(struct iio_dev *indio_dev)
{
	struct shub_data *mcu_data = iio_priv(indio_dev);

	if (!atomic_cmpxchg(&mcu_data->pseudo_irq_enable, 0, 1))
		SH_FUN();
	return 0;
}

static int shub_pseudo_irq_disable(struct iio_dev *indio_dev)
{
	struct shub_data *mcu_data = iio_priv(indio_dev);

	if (atomic_cmpxchg(&mcu_data->pseudo_irq_enable, 1, 0))
		SH_FUN();
	return 0;
}

static int shub_set_pseudo_irq(struct iio_dev *indio_dev, int enable)
{
	SH_LOG("enable=%d\n", enable);

	if (enable)
		shub_pseudo_irq_enable(indio_dev);
	else
		shub_pseudo_irq_disable(indio_dev);
	return 0;
}

static int shub_data_rdy_trigger_set_state(struct iio_trigger *trig, bool state)
{
	struct iio_dev *indio_dev =
	    (struct iio_dev *)iio_trigger_get_drvdata(trig);
	struct shub_data *mcu_data = iio_priv(indio_dev);

	mutex_lock(&mcu_data->mutex_lock);
	SH_LOG("state=%d\n", state);
	shub_set_pseudo_irq(indio_dev, state);
	mutex_unlock(&mcu_data->mutex_lock);
	return 0;
}

static const struct iio_trigger_ops shub_iio_trigger_ops = {
	.owner = THIS_MODULE,
	.set_trigger_state = &shub_data_rdy_trigger_set_state,
};

static int shub_probe_trigger(struct iio_dev *iio_dev)
{
	struct shub_data *mcu_data = iio_priv(iio_dev);
	int ret;

	iio_dev->pollfunc = iio_alloc_pollfunc(&iio_pollfunc_store_time,
					       &shub_trigger_handler,
					       IRQF_ONESHOT,
					       iio_dev,
					       "%s_consumer%d",
					       iio_dev->name, iio_dev->id);

	if (iio_dev->pollfunc == NULL) {
		ret = -ENOMEM;
		goto error_ret;
	}
	SH_LOG("iio pollfunc success\r\n");
	mcu_data->trig =
	    iio_trigger_alloc("%s-dev%d", iio_dev->name, iio_dev->id);
	if (!mcu_data->trig) {
		ret = -ENOMEM;

		goto error_ret;
	}

	mcu_data->trig->dev.parent = &mcu_data->sensor_pdev->dev;
	mcu_data->trig->ops = &shub_iio_trigger_ops;
	iio_trigger_set_drvdata(mcu_data->trig, iio_dev);

	ret = iio_trigger_register(mcu_data->trig);
	if (ret)
		goto error_free_trig;

	return 0;

error_free_trig:
	iio_trigger_free(mcu_data->trig);
	iio_dealloc_pollfunc(iio_dev->pollfunc);
error_ret:
	return ret;
}

static int shub_read_raw(struct iio_dev *indio_dev,
			 struct iio_chan_spec const *chan, int *val, int *val2,
			 long mask)
{
	struct shub_data *mcu_data = iio_priv(indio_dev);
	int ret = -EINVAL;

	if (chan->type != IIO_ACCEL)
		return ret;

	mutex_lock(&mcu_data->mutex_lock);
	switch (mask) {
	case 0:
		*val = mcu_data->iio_data[chan->channel2 - IIO_MOD_X];
		ret = IIO_VAL_INT;
		break;

	case IIO_CHAN_INFO_SCALE:
		/* Gain : counts / uT = 1000 [nT] */
		/* Scaling factor : 1000000 / Gain = 1000 */
		*val = 0;
		*val2 = 1000;
		ret = IIO_VAL_INT_PLUS_MICRO;
		break;

	default:
		break;
	}
	mutex_unlock(&mcu_data->mutex_lock);
	return ret;
}

#define IIO_ST(si, rb, sb, sh)						\
	{ .sign = si, .realbits = rb, .storagebits = sb, .shift = sh }

#define SHUB_CHANNEL(axis, bits) {                  \
	.type = IIO_ACCEL,	                    \
	.modified = 1,                          \
	.channel2 = axis + 1,                     \
	.scan_index = axis,	                    \
	.scan_type = IIO_ST('u', bits, bits, 0)	    \
}

static const struct iio_chan_spec shub_channels[] = {
	SHUB_CHANNEL(SHUB_SCAN_ID, 64),
	SHUB_CHANNEL(SHUB_SCAN_RAW_0, 64),
	SHUB_CHANNEL(SHUB_SCAN_RAW_1, 64),
	SHUB_CHANNEL(SHUB_SCAN_RAW_2, 64),
	IIO_CHAN_SOFT_TIMESTAMP(SHUB_SCAN_TIMESTAMP)
};

static unsigned long shub_allchannel_scan_masks[] = {
	/* timestamp channel is not managed by scan mask */
	BIT(SHUB_SCAN_ID) | BIT(SHUB_SCAN_RAW_0) |
	BIT(SHUB_SCAN_RAW_1) | BIT(SHUB_SCAN_RAW_2),
	0x00000000
};

static const struct iio_info shub_iio_info = {
	.read_raw = &shub_read_raw,
	.driver_module = THIS_MODULE,
};

static int shub_config_kfifo(struct iio_dev *iio_dev)
{
	int ret;
	struct iio_buffer *buffer;

	buffer = devm_iio_kfifo_allocate(&iio_dev->dev);
	if (!buffer) {
		ret = -ENOMEM;
		goto error_ret;
	}

	iio_device_attach_buffer(iio_dev, buffer);
	buffer->scan_timestamp = true;
	/* set scan mask OR set iio:device1/scan_elements/XXX_en */
	buffer->scan_mask = shub_allchannel_scan_masks;
	iio_dev->buffer = buffer;
	iio_dev->setup_ops = &shub_iio_buffer_setup_ops;
	iio_dev->modes |= INDIO_BUFFER_TRIGGERED;
	SH_LOG("iio buff register success\n");

	return 0;

error_ret:
	return ret;
}

static int create_sysfs_interfaces(struct shub_data *mcu_data)
{
	int i;
	int res = 0;

	mcu_data->sensor_class = class_create(THIS_MODULE, "sprd_sensorhub");
	if (IS_ERR(mcu_data->sensor_class))
		return PTR_ERR(mcu_data->sensor_class);

	mcu_data->sensor_dev =
	    device_create(mcu_data->sensor_class, NULL, 0, "%s", "sensor_hub");
	if (IS_ERR(mcu_data->sensor_dev)) {
		res = PTR_ERR(mcu_data->sensor_dev);
		goto err_device_create;
	}
	debugfs_create_symlink("sensor", NULL,
		"/sys/class/sprd_sensorhub/sensor_hub");

	dev_set_drvdata(mcu_data->sensor_dev, mcu_data);
	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		if (device_create_file(mcu_data->sensor_dev, attributes + i))
			goto error;

	res =
	    sysfs_create_link(&mcu_data->sensor_dev->kobj,
			      &mcu_data->indio_dev->dev.kobj, "iio");
	if (res < 0)
		goto error;

	return 0;

error:
	while (--i >= 0)
		device_remove_file(mcu_data->sensor_dev, attributes + i);
	put_device(mcu_data->sensor_dev);
	device_unregister(mcu_data->sensor_dev);
err_device_create:
	class_destroy(mcu_data->sensor_class);
	return res;
}


static void shub_remove_trigger(struct iio_dev *indio_dev)
{
	struct shub_data *mcu_data = iio_priv(indio_dev);

	iio_trigger_unregister(mcu_data->trig);
	iio_trigger_free(mcu_data->trig);
	iio_dealloc_pollfunc(indio_dev->pollfunc);
}

static void shub_remove_buffer(struct iio_dev *indio_dev)
{
	iio_kfifo_free(indio_dev->buffer);
}

static void shub_config_init(struct shub_data *sensor)
{
	int i = 0;

	for (i = 0; i < HANDLE_ID_END; i++)
		sensor->enabled_list[i] = 0;

	sensor->interrupt_status = 0;
	sensor->cal_cmd = 0;
	sensor->cal_type = 0;
	sensor->cal_id = 0;
	sensor->is_sensorhub = 1;
}

static int shub_probe(struct platform_device *pdev)
{
	struct shub_data *mcu;
	struct iio_dev *indio_dev;
	int error = 0;

	if (probe_retry_count < 1) {
		probe_retry_count++;
		SH_LOG("probe_retry_count = %d\n", probe_retry_count);
		return -EPROBE_DEFER;
	}

	SH_LOG("MAIN_VERSION : %s\n", MAIN_VERSION);
	if (!pdev) {
		SH_ERR("-- error : pdev is NULL\n");
		return -EINVAL;
	}

	indio_dev = iio_device_alloc(sizeof(*mcu));
	if (!indio_dev) {
		SH_ERR(" %s: iio_device_alloc failed\n", __func__);
		return -ENOMEM;
	}

	indio_dev->name = SHUB_NAME;
	indio_dev->dev.parent = &pdev->dev;
	indio_dev->info = &shub_iio_info;
	indio_dev->channels = shub_channels;
	indio_dev->num_channels = ARRAY_SIZE(shub_channels);

	mcu = iio_priv(indio_dev);
	mcu->sensor_pdev = pdev;
	mcu->indio_dev = indio_dev;
	g_sensor = mcu;

	mcu->mcu_mode = SHUB_BOOT;
	dev_set_drvdata(&pdev->dev, mcu);

	mcu->data_callback = shub_data_callback;
	mcu->save_mag_offset = shub_save_mag_offset;
	mcu->readcmd_callback = shub_readcmd_callback;
	mcu->cm4_read_callback = shub_cm4_read_callback;
	init_waitqueue_head(&mcu->rxwq);

	mcu->resp_cmdstatus_callback = parse_cmd_response_callback;
	init_waitqueue_head(&mcu->rw_wait_queue);

	mutex_init(&mcu->mutex_lock);
	mutex_init(&mcu->mutex_read);
	mutex_init(&mcu->mutex_send);
	mutex_init(&mcu->send_command_mutex);

	error = shub_config_kfifo(indio_dev);
	if (error != 0) {
		SH_ERR("iio yas_probe_buffer failed\n");
		goto error_free_dev;
	}

	error = shub_probe_trigger(indio_dev);
	if (error) {
		SH_ERR("%s: iio yas_probe_trigger failed\n", __func__);
		goto error_remove_buffer;
	}
	error = devm_iio_device_register(&pdev->dev, indio_dev);
	if (error) {
		SH_ERR("iio_device_register failed\n");
		goto error_remove_trigger;
	}

	error = create_sysfs_interfaces(mcu);
	if (error != 0)
		goto err_free_mem;

	INIT_WORK(&mcu->download_work, shub_download_work);
	INIT_WORK(&mcu->savecalifile_work, shub_save_calibration_data);
	wake_lock_init(&sensorhub_wake_lock, WAKE_LOCK_SUSPEND,
			"sensorhub_wake_lock");
	/* send system_time */
	INIT_DELAYED_WORK(&mcu->delay_work, shub_synctime_work);
	mcu->driver_wq = create_singlethread_workqueue("shub_synctime");

	init_irq_work(&mcu->iio_irq_work, iio_trigger_work);

	shub_config_init(mcu);
	pm_runtime_enable(&pdev->dev);

	thread = kthread_run(shub_read_thread, mcu, SHUB_RD);
	if (IS_ERR(thread)) {
		error = PTR_ERR(thread);
		SH_ERR("failed to create kernel thread: %d\n", error);
	}

	thread_nwu = kthread_run(shub_read_thread_nwu, mcu, SHUB_RD_NWU);
	if (IS_ERR(thread_nwu)) {
		error = PTR_ERR(thread_nwu);
		SH_ERR("failed to create kernel thread_nwu: %d\n", error);
	}
	sbuf_set_no_need_wake_lock(SIPC_ID_PM_SYS,
		SMSG_CH_PIPE, SIPC_PM_BUFID1);
	mcu->early_suspend.notifier_call = shub_notifier_fn;
	register_pm_notifier(&mcu->early_suspend);

	SH_LOG("success.\n");
	return 0;

err_free_mem:
	iio_device_unregister(indio_dev);
error_remove_trigger:
	shub_remove_trigger(indio_dev);
error_remove_buffer:
	shub_remove_buffer(indio_dev);
error_free_dev:
	iio_device_free(indio_dev);

	SH_LOG(" sensorhub probe fail.\n");
	return error;
}

static int shub_remove(struct platform_device *pdev)
{
	struct shub_data *mcu = platform_get_drvdata(pdev);

	if (!IS_ERR_OR_NULL(thread))
		kthread_stop(thread);
	if (!IS_ERR_OR_NULL(thread_nwu))
		kthread_stop(thread_nwu);
	kfree(mcu);
	return 0;
}

static const struct of_device_id shub_match_table[] = {
	{.compatible = "sprd,sensorhub",},
	{},
};

static struct platform_driver shub_driver = {
	.probe = shub_probe,
	.remove = shub_remove,
	.driver = {
		   .name = SHUB_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = shub_match_table,
	   },
	.suspend = shub_suspend,
	.resume = shub_resume,
};

module_platform_driver(shub_driver);

MODULE_DESCRIPTION("Spreadtrum Sensor Hub");
MODULE_AUTHOR("Bao Yue <bao.yue@spreadtrum.com>");
MODULE_LICENSE("GPL");
