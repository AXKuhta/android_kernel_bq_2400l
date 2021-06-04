/*
 * Copyright (C) 2014 Spreadtrum Communications Inc.
 *
 * Authors:<jinglong.chen@spreadtrum.com>
 * Owner:
 *      jinglong.chen
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
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <net/genetlink.h>
#include <linux/types.h>
#include "wlan_common.h"
#define WLAN_NL_GENERAL_SOCK_ID 1022
enum wlan_nl_commands
{
	WLAN_NL_CMD_UNSPEC,
	WLAN_NL_CMD_NPI,
	WLAN_NL_CMD_GET_INFO,
	WLAN_NL_CMD_MAX,
};

enum wlan_nl_attrs
{
	WLAN_NL_ATTR_UNSPEC,
	WLAN_NL_ATTR_COMMON_USR_TO_DRV,
	WLAN_NL_ATTR_COMMON_DRV_TO_USR,
	WLAN_NL_ATTR_MAX,
};
extern unsigned int g_dbg;
extern wlan_info_t g_wlan;
static int wlan_nl_send_generic(struct genl_info *info, u8 attr, u8 cmd, u32 len, u8 *data);
extern int wlan_cmd_npi_send_recv(unsigned char *s_buf,unsigned short s_len, unsigned char *r_buf, unsigned short *r_len );
#ifdef CONFIG_MACH_SAMSUNG
static bool is_npi_start_cmd(void *buf)
{
#define WLNPI_CMD_START (0)
       struct npi_cmd_hdr *msg;

       msg = (struct npi_cmd_hdr *)buf;
       if ((msg->type == HOST_TO_MARLIN_CMD) &&
              (msg->subtype == WLNPI_CMD_START))
                  return true;
        else
                  return false;
}
static bool is_sta_or_p2p_open(void)
{
        if (g_wlan.netif[0].mode == ITM_NONE_MODE ||
              g_wlan.netif[1].mode == ITM_NONE_MODE)
                  return false;
        else
                  return true;
}
#endif

static int wlan_nl_npi_handler(struct sk_buff *skb_2,	 struct genl_info *info)
{
	int ret = -EINVAL;
	unsigned short s_len = 0;
	unsigned char *s_buf = NULL;
	unsigned char  r_buf[1024] = {0};
	unsigned short r_len = 0;
	unsigned char  dbgStr[64] = {0};
#ifdef CONFIG_MACH_SAMSUNG
        bool flag = true;
          /*npi start response head*/
        struct npi_cmd_hdr hdr = {
                  .type = 0x2,
                  .subtype = 0x0,
        };
        int err = -100;
#endif
	printkd("[%s][enter]\n", __func__);
	if (info == NULL)
	{
		printkd("[%s][%d][ERROR]\n", __func__, __LINE__);
#ifdef CONFIG_MACH_SAMSUNG
                flag = false;
#endif
		goto out;
	}
	if (info->attrs[WLAN_NL_ATTR_COMMON_USR_TO_DRV])
	{
		s_buf  = nla_data(info->attrs[WLAN_NL_ATTR_COMMON_USR_TO_DRV]);
		if(s_buf == NULL){
			printkd("[%s][ERROR]s_buf is NULL\n", __func__);
			goto out;
		}
		s_len  = nla_len(info->attrs[WLAN_NL_ATTR_COMMON_USR_TO_DRV]);
#ifdef CONFIG_MACH_SAMSUNG
                if (is_npi_start_cmd(s_buf)) {
                     if (is_sta_or_p2p_open()) {
                               flag = false;
                               hdr.len = sizeof(err);
                               r_len = sizeof(hdr) + hdr.len;
                               memcpy(r_buf, &hdr, sizeof(hdr));
                               memcpy(r_buf + sizeof(hdr), &err, hdr.len);
                               printke("wifi is already open, please close!!!\n");
                               goto out;
                      }
                }
#endif
		sprintf(dbgStr, "[iwnpi][SEND][%d]:", s_len );
		hex_dump(dbgStr, strlen(dbgStr), s_buf, s_len);
		wlan_cmd_npi_send_recv(s_buf, s_len, r_buf, &r_len);
		sprintf(dbgStr, "[iwnpi][RECV][%d]:", r_len );
		hex_dump(dbgStr, strlen(dbgStr), r_buf, r_len);
	}
	ret = wlan_nl_send_generic(info, WLAN_NL_ATTR_COMMON_DRV_TO_USR, WLAN_NL_CMD_NPI, r_len, r_buf );
#ifdef CONFIG_MACH_SAMSUNG
        if (s_buf && is_npi_start_cmd(s_buf) && flag == true) {
                  msleep(100);
                  wlan_cmd_set_psm_cap();
        }
#endif

out:
	return ret;
}
static int wlan_nl_get_info_handler(struct sk_buff *skb_2,	 struct genl_info *info)
{
	int ret;
	unsigned char  r_buf[64] = {0};
	unsigned short r_len = 0;
	memcpy(r_buf, &( g_wlan.netif[0].ndev->dev_addr[0] ), 6 );
	r_len = 6;
	printkd("[%s][enter]\n", __func__ );
	ret = wlan_nl_send_generic(info, WLAN_NL_ATTR_COMMON_DRV_TO_USR, WLAN_NL_CMD_GET_INFO, r_len, r_buf );
	return ret;
}

static struct nla_policy wlan_genl_policy[WLAN_NL_ATTR_MAX + 1] =
{
	[WLAN_NL_ATTR_COMMON_USR_TO_DRV] = {.type = NLA_BINARY, .len = 1024},
	[WLAN_NL_ATTR_COMMON_DRV_TO_USR] = {.type = NLA_BINARY, .len = 1024},
};

static struct genl_ops wlan_nl_ops[] =
{
	{
		.cmd = WLAN_NL_CMD_NPI,
		.policy = wlan_genl_policy,
		.doit = wlan_nl_npi_handler,
	},
	{
		.cmd = WLAN_NL_CMD_GET_INFO,
		.policy = wlan_genl_policy,
		.doit = wlan_nl_get_info_handler,
	},	
};

static struct genl_family wlan_nl_genl_family =
{
	.id = WLAN_NL_GENERAL_SOCK_ID,
	.hdrsize = 0,
	.name = "WLAN_NL",
	.version = 1,
	.maxattr = WLAN_NL_ATTR_MAX,
};

static int wlan_nl_send_generic(struct genl_info *info, u8 attr, u8 cmd, u32 len, u8 *data)
{
	struct sk_buff *skb;
	void *hdr;
	int ret;
	skb = nlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;
	hdr = genlmsg_put(skb, info->snd_portid, info->snd_seq, &wlan_nl_genl_family, 0, cmd);
	if (IS_ERR(hdr))
	{
		ret = PTR_ERR(hdr);
		goto err_put;
	}
	if (nla_put(skb, attr, len, data))
	{
		ret = -1;
		goto err_put;
	}

	genlmsg_end(skb, hdr);
	return genlmsg_reply(skb, info);

err_put:
	nlmsg_free(skb);
	return ret;
}

void wlan_nl_init(void )
{
	int ret;
	ret = genl_register_family_with_ops(&wlan_nl_genl_family,  wlan_nl_ops, ARRAY_SIZE(wlan_nl_ops));
	if (ret)
	{
		printkd("genl_register_family_with_ops error ret:%d\n", ret);
		return;
	}
	printkd("%s\n", __func__);
	return;
}

void wlan_nl_deinit(void )
{
	int ret;
	ret = genl_unregister_family(&wlan_nl_genl_family);
	if (ret)
	{
		printkd("genl_unregister_family error ret:%d\n", ret);
		return;
	}
	printkd("%s\n", __func__);
	return;
}


