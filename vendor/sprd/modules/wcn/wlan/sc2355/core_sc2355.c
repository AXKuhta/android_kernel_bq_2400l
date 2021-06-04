/*
 * Copyright (C) 2016 Spreadtrum Communications Inc.
 *
 * Authors	:
 * star.liu <star.liu@spreadtrum.com>
 * yifei.li <yifei.li@spreadtrum.com>
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

#include <linux/platform_device.h>
#include <linux/utsname.h>
#include <linux/debugfs.h>
#include <linux/marlin_platform.h>
#include "sprdwl.h"
#include "if_sc2355.h"
#include "core_sc2355.h"
#include "tx_msg_sc2355.h"
#include "rx_msg_sc2355.h"
#include "msg.h"
#include "txrx.h"

int sprdwl_debug_level = L_INFO;
struct device *sprdwl_dev;
void adjust_debug_level(char *buf)
{
	int level = buf[12] - '0';

	wl_err("input debug level: %d!\n", level);
	switch (level) {
	case L_ERR:
		sprdwl_debug_level = L_ERR;
		break;
	case L_INFO:
		sprdwl_debug_level = L_INFO;
		break;
	case L_DBG:
		sprdwl_debug_level = L_DBG;
		break;
	default:
		sprdwl_debug_level = L_ERR;
		wl_err("input wrong debug level\n");
	}

	wl_err("set sprdwl_debug_level: %d\n", sprdwl_debug_level);
}

struct debuginfo_s {
	void (*func)(char *);
	char str[20];
} debuginfo[] = {
	{adjust_debug_level, "debug_level="}
};

/* TODO: Could we use netdev_alloc_frag instead of kmalloc?
 *       So we did not need to distinguish buffer type
 *       Maybe it could speed up alloc process, too
 */
void sprdwl_free_data(void *data, int buffer_type)
{
	if (buffer_type) { /* Fragment page buffer */
		put_page(virt_to_head_page(data));
	} else { /* Normal buffer */
		kfree(data);
	}
}

void sprdwl_event_tdls_flow_count(struct sprdwl_vif *vif, u8 *data, u16 len)
{
	struct sprdwl_intf *intf = vif->priv->hw_priv;
	u8 i;
	u8 found = 0;
	struct tdls_update_peer_infor *peer_info =
		(struct tdls_update_peer_infor *)data;
	ktime_t kt;

	if (len < sizeof(struct tdls_update_peer_infor)) {
		wl_err("%s, event data len not in range\n", __func__);
			return;
	}
	for (i = 0; i < MAX_TDLS_PEER; i++) {
		if (ether_addr_equal(intf->tdls_flow_count[i].da,
				     peer_info->da)) {
			found = 1;
			break;
		}
	}
	/*0 to delete entry*/
	if (peer_info->valid == 0) {
		if (found == 0) {
			wl_err("%s, invalid da, fail to del\n", __func__);
			return;
		}
		memset(&intf->tdls_flow_count[i],
		       0,
		       sizeof(struct tdls_flow_count_para));

		for (i = 0; i < MAX_TDLS_PEER; i++) {
			if (intf->tdls_flow_count[i].valid == 1)
				found++;
		}
		if (found == 1)
			intf->tdls_flow_count_enable = 0;
	} else if (peer_info->valid == 1) {
		if (found == 0) {
			for (i = 0; i < MAX_TDLS_PEER; i++) {
				if (intf->tdls_flow_count[i].valid == 0) {
					found = 1;
					break;
				}
			}
		}
		if (found == 0) {
			wl_err("%s, no free TDLS entry\n", __func__);
			i = 0;
		}

		intf->tdls_flow_count_enable = 1;
		intf->tdls_flow_count[i].valid = 1;
		ether_addr_copy(intf->tdls_flow_count[i].da, peer_info->da);
		intf->tdls_flow_count[i].threshold = peer_info->txrx_len;
		intf->tdls_flow_count[i].data_len_counted = 0;

		wl_info("%s,%d, tdls_id=%d,threshold=%d, timer=%d, da=(%pM)\n",
			__func__, __LINE__, i,
			intf->tdls_flow_count[i].threshold,
			peer_info->timer, peer_info->da);

		kt = ktime_get();
		intf->tdls_flow_count[i].start_mstime =
			(u32)(div_u64(kt.tv64, NSEC_PER_MSEC));
		intf->tdls_flow_count[i].timer =
			peer_info->timer;
		wl_info("%s,%d, tdls_id=%d,start_time:%u\n",
			__func__, __LINE__, i,
			intf->tdls_flow_count[i].start_mstime);
	}
}

void count_tdls_flow(struct sprdwl_vif *vif, u8 *data, u16 len)
{
	u8 i, found = 0;
	u32 msec;
	u8 elapsed_time;
	u8 unit_time;
	ktime_t kt;
	struct sprdwl_intf *intf = (struct sprdwl_intf *)vif->priv->hw_priv;
	int ret = 0;

	for (i = 0; i < MAX_TDLS_PEER; i++) {
		if ((intf->tdls_flow_count[i].valid == 1) &&
		    (ether_addr_equal(data, intf->tdls_flow_count[i].da)))
			goto count_it;
	}
	return;

count_it:
	kt = ktime_get();
	msec = (u32)(div_u64(kt.tv64, NSEC_PER_MSEC));
	elapsed_time =
		(msec - intf->tdls_flow_count[i].start_mstime) / MSEC_PER_SEC;
	unit_time = elapsed_time / intf->tdls_flow_count[i].timer;
	wl_info("%s,%d, tdls_id=%d, len_counted=%d, len=%d, threshold=%d\n",
		__func__, __LINE__, i,
		intf->tdls_flow_count[i].data_len_counted, len,
		intf->tdls_flow_count[i].threshold);
	wl_info("currenttime=%u, elapsetime=%d, how_many_time=%d\n",
		msec, elapsed_time, unit_time);

	if ((intf->tdls_flow_count[i].data_len_counted == 0 &&
	     len > intf->tdls_flow_count[i].threshold) ||
	    (intf->tdls_flow_count[i].data_len_counted > 0 &&
	    ((intf->tdls_flow_count[i].data_len_counted + len) >
	     intf->tdls_flow_count[i].threshold *
	     ((unit_time == 0) ? 1 : unit_time)))) {
		ret = sprdwl_send_tdls_cmd(vif, vif->ctx_id,
					   (u8 *)intf->tdls_flow_count[i].da,
					   SPRDWL_TDLS_CMD_CONNECT, 0);
		if (ret != -1)
			memset(&intf->tdls_flow_count[i],
			       0,
			       sizeof(struct tdls_flow_count_para));
	} else {
		if (intf->tdls_flow_count[i].data_len_counted == 0) {
			intf->tdls_flow_count[i].start_mstime = msec;
			intf->tdls_flow_count[i].data_len_counted += len;
		}
		if ((intf->tdls_flow_count[i].data_len_counted > 0) &&
		    unit_time > 1) {
			intf->tdls_flow_count[i].start_mstime = msec;
			intf->tdls_flow_count[i].data_len_counted = len;
		}
		if ((intf->tdls_flow_count[i].data_len_counted > 0) &&
		    unit_time <= 1) {
			intf->tdls_flow_count[i].data_len_counted += len;
		}
	}
	for (i = 0; i < MAX_TDLS_PEER; i++) {
		if (intf->tdls_flow_count[i].valid == 1)
			found++;
	}
	if (found == 0)
		intf->tdls_flow_count_enable = 0;
}

#define SPRDWL_SDIO_DEBUG_BUFLEN 128
static ssize_t sprdwl_intf_read_info(struct file *file,
				     char __user *user_buf,
				     size_t count, loff_t *ppos)
{
	size_t ret = 0;
	unsigned int buflen, len;
	unsigned char *buf;
	struct sprdwl_intf *sdev;
	struct sprdwl_tx_msg *tx_msg;

	sdev = (struct sprdwl_intf *)file->private_data;
	tx_msg = (struct sprdwl_tx_msg *)sdev->sprdwl_tx;
	buflen = SPRDWL_SDIO_DEBUG_BUFLEN;
	buf = kzalloc(buflen, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	len = 0;
	len += scnprintf(buf, buflen,
			 "net: stop %lu, start %lu\n drop cnt:\n"
			 "cmd %lu, sta %lu, p2p %lu,\n"
			 "ring_ap:%lu ring_cp:%lu red_flow:%u,\n"
			 "green_flow:%u blue_flow:%u white_flow:%u\n",
			 tx_msg->net_stop_cnt, tx_msg->net_start_cnt,
			 tx_msg->drop_cmd_cnt, tx_msg->drop_data1_cnt,
			 tx_msg->drop_data2_cnt,
			 tx_msg->ring_ap, tx_msg->ring_cp,
			 atomic_read(&tx_msg->flow_ctrl[0].flow),
			 atomic_read(&tx_msg->flow_ctrl[1].flow),
			 atomic_read(&tx_msg->flow_ctrl[2].flow),
			 atomic_read(&tx_msg->flow_ctrl[3].flow));
	if (len > buflen)
		len = buflen;

	ret = simple_read_from_buffer(user_buf, count, ppos, buf, len);
	kfree(buf);

	return ret;
}

static ssize_t sprdwl_intf_write(struct file *file,
				 const char __user *__user_buf,
				 size_t count, loff_t *ppos)
{
	char buf[20];
	struct sprdwl_intf *sdev;
	int type = 0;
	int debug_size = sizeof(debuginfo)/sizeof(struct debuginfo_s);

	sdev = (struct sprdwl_intf *)file->private_data;

	if (!count || count >= sizeof(buf)) {
		wl_err("write len too long:%zu >= %zu\n", count, sizeof(buf));
		return -EINVAL;
	}
	if (copy_from_user(buf, __user_buf, count))
		return -EFAULT;
	buf[count] = '\0';
	wl_err("write info:%s\n", buf);
	for (type = 0; type < debug_size; type++)
		if (!strncmp(debuginfo->str, buf, strlen(debuginfo->str))) {
			debuginfo->func(buf);
			break;
		}

	return count;
}

static const struct file_operations sprdwl_intf_debug_fops = {
	.read = sprdwl_intf_read_info,
	.write = sprdwl_intf_write,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek
};

void sprdwl_debugfs(void *spdev, struct dentry *dir)
{
	struct sprdwl_intf *intf;

	intf = (struct sprdwl_intf *)spdev;
	debugfs_create_file("sprdwlinfo", S_IRUSR,
			    dir, intf, &sprdwl_intf_debug_fops);
}

static struct dentry *sprdwl_debug_root;

void sprdwl_debugfs_init(void)
{
	/* create debugfs */
	sprdwl_debug_root = debugfs_create_dir("sprdwl_debug", NULL);
	debugfs_create_file("log_level", 0444,
		sprdwl_debug_root, NULL, &sprdwl_intf_debug_fops);
}

static int sprdwl_ini_download_status(void)
{
	/*disable download ini function, just return 1*/
	/*	return 1; */
	/*fw is ready for receive ini file*/
	return !is_first_power_on(MARLIN_WIFI);
}

static void sprdwl_force_exit(void *spdev)
{
	struct sprdwl_intf *intf;
	struct sprdwl_tx_msg *tx_msg;

	intf = (struct sprdwl_intf *)spdev;
	tx_msg = (struct sprdwl_tx_msg *)intf->sprdwl_tx;
	intf->exit = 1;
	wake_up_all(&tx_msg->waitq);
}

static int sprdwl_is_exit(void *spdev)
{
	struct sprdwl_intf *intf;

	intf = (struct sprdwl_intf *)spdev;
	return intf->exit;
}

static void sprdwl_tcp_drop_msg(void *spdev, struct sprdwl_msg_buf *msgbuf)
{
	enum sprdwl_mode mode;
	struct sprdwl_msg_list *list;
	struct sprdwl_intf *intf = (struct sprdwl_intf *)spdev;

	if (msgbuf->skb)
		dev_kfree_skb(msgbuf->skb);
	mode = msgbuf->mode;
	list = msgbuf->msglist;
	sprdwl_free_msg_buf(msgbuf, list);
	sprdwl_wake_net_ifneed(intf, list, mode);
}

static struct sprdwl_if_ops sprdwl_core_ops = {
	.get_msg_buf = sprdwl_get_msg_buf,
	.free_msg_buf = sprdwl_tx_free_msg_buf,
#ifdef SPRDWL_TX_SELF
	.tx = sprdwl_tx_self_msg,
#else
	.tx = sprdwl_tx_msg_func,
#endif
	.force_exit = sprdwl_force_exit,
	.is_exit = sprdwl_is_exit,
	.debugfs = sprdwl_debugfs,
	.tcp_drop_msg = sprdwl_tcp_drop_msg,
	.ini_download_status = sprdwl_ini_download_status
};

enum sprdwl_hw_type sprd_core_get_hwintf_mode(void)
{
	return SPRDWL_HW_SC2355_SDIO;/*TBD*/
}

static int sprdwl_probe(struct platform_device *pdev)
{
	struct sprdwl_intf *intf;
	struct sprdwl_priv *priv;
	int ret;
	u8 i;

	if (start_marlin(MARLIN_WIFI)) {
		wl_err("%s power on chipset failed\n", __func__);
		return -ENODEV;
	}

	intf = kzalloc(sizeof(*intf), GFP_ATOMIC);
	if (!intf) {
		ret = -ENOMEM;
		wl_err("%s alloc intf fail: %d\n", __func__, ret);
		goto err;
	}

	platform_set_drvdata(pdev, intf);
	intf->pdev = pdev;
	sprdwl_dev = &pdev->dev;

	for (i = 0; i < MAX_LUT_NUM; i++)
		intf->peer_entry[i].ctx_id = 0xff;

	priv = sprdwl_core_create(sprd_core_get_hwintf_mode(),
				  &sprdwl_core_ops);
	if (!priv) {
		wl_err("%s core create fail\n", __func__);
		ret = -ENXIO;
		goto err_core_create;
	}
	memcpy(priv->wl_ver.kernel_ver, utsname()->release,
			strlen(utsname()->release));
	memcpy(priv->wl_ver.drv_ver, SPRDWL_DRIVER_VERSION,
			strlen(SPRDWL_DRIVER_VERSION));
	memcpy(priv->wl_ver.update, SPRDWL_UPDATE, strlen(SPRDWL_UPDATE));
	memcpy(priv->wl_ver.reserve, SPRDWL_RESERVE, strlen(SPRDWL_RESERVE));
	wl_info("Spreadtrum WLAN Version:");
	wl_info("Kernel:%s,Driver:%s,update:%s,reserved:%s\n",
			 utsname()->release, SPRDWL_DRIVER_VERSION,
			 SPRDWL_UPDATE, SPRDWL_RESERVE);

	if (priv->hw_type == SPRDWL_HW_SC2355_SDIO) {
		intf->hif_offset = sizeof(struct sdiohal_puh);
		intf->rx_cmd_port = SDIO_RX_CMD_PORT;
		intf->rx_data_port = SDIO_RX_DATA_PORT;
		intf->tx_cmd_port = SDIO_TX_CMD_PORT;
		intf->tx_data_port = SDIO_TX_DATA_PORT;
	} else {
		intf->rx_cmd_port = PCIE_RX_CMD_PORT;
		intf->rx_data_port = PCIE_RX_DATA_PORT;
		intf->tx_cmd_port = PCIE_TX_CMD_PORT;
		intf->tx_data_port = PCIE_TX_DATA_PORT;
	}

	ret = sprdwl_intf_init(priv, intf);
	if (ret) {
		wl_err("%s intf init failed: %d\n", __func__, ret);
		goto err_if_init;
	}

	ret = sprdwl_rx_init(intf);
	if (ret) {
		wl_err("%s rx init failed: %d\n", __func__, ret);
		goto err_rx_init;
	}

	ret = sprdwl_tx_init(intf);
	if (ret) {
		wl_err("%s tx_list init failed\n", __func__);
		goto err_tx_init;
	}

	ret = sprdwl_core_init(&pdev->dev, priv);
	if (ret)
		goto err_core_init;

#if defined FPGA_LOOPBACK_TEST
	intf->loopback_n = 0;
	sprdwl_intf_tx_data_fpga_test(intf, NULL, 0);
#endif

	sprdwl_debugfs_init();

	return ret;

err_core_init:
	sprdwl_tx_deinit(intf);
err_tx_init:
	sprdwl_rx_deinit(intf);
err_rx_init:
	sprdwl_intf_deinit(intf);
err_if_init:
	sprdwl_core_free((struct sprdwl_priv *)intf->priv);
err_core_create:
	kfree(intf);
err:
	return ret;
}

static int sprdwl_remove(struct platform_device *pdev)
{
	struct sprdwl_intf *intf = platform_get_drvdata(pdev);
	struct sprdwl_priv *priv = intf->priv;

	sprdwl_core_deinit(priv);
	sprdwl_tx_deinit(intf);
	sprdwl_rx_deinit(intf);
	sprdwl_intf_deinit(intf);
	sprdwl_core_free(priv);
	kfree(intf);
	stop_marlin(MARLIN_WIFI);
	wl_info("%s\n", __func__);

	return 0;
}

static const struct of_device_id sprdwl_of_match[] = {
	{.compatible = "sprd,sc2355-wifi",},
	{}
};
MODULE_DEVICE_TABLE(of, sprdwl_of_match);

static struct platform_driver sprdwl_driver = {
	.probe = sprdwl_probe,
	.remove = sprdwl_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "sc2355",
		.of_match_table = sprdwl_of_match,
	}
};

module_platform_driver(sprdwl_driver);

MODULE_DESCRIPTION("Spreadtrum Wireless LAN Driver");
MODULE_AUTHOR("Spreadtrum WCN Division");
MODULE_LICENSE("GPL");
