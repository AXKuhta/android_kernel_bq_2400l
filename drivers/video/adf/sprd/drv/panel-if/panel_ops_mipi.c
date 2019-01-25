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

#include <linux/delay.h>
#include <linux/kernel.h>

#include "sprd_panel.h"
#include "../dsi/mipi_dsi_api.h"

static int mipi_dsi_send_cmds(struct sprd_dsi *dsi, struct panel_cmds *in_cmds)
{
	int i;

	if ((in_cmds == NULL) || (dsi == NULL))
		return -1;

	for (i = 0; i < in_cmds->cmd_total; i++) {
		mipi_dsi_gen_write(dsi, in_cmds->cmds[i].payload,
				      in_cmds->cmds[i].header.len);
		if (in_cmds->cmds[i].header.wait)
			msleep(in_cmds->cmds[i].header.wait);
	}
	return 0;
}


static int32_t mipi_panel_init(struct panel_device *pd)
{
	struct sprd_dsi *dsi = dev_get_drvdata(pd->intf);
	struct panel_info *panel = pd->panel;

	mipi_dsi_lp_cmd_enable(dsi, true);
	mipi_dsi_send_cmds(dsi, panel->cmd_codes[CMD_CODE_INIT]);
	mipi_dsi_set_work_mode(dsi, panel->work_mode);
	mipi_dsi_state_reset(dsi);
	return 0;
}

static int32_t mipi_panel_esd_check(struct panel_device *pd)
{
	struct sprd_dsi *dsi = dev_get_drvdata(pd->intf);
	struct panel_info *panel = pd->panel;
	uint8_t read_buf[4] = {0};
	uint8_t i;

	mipi_dsi_lp_cmd_enable(dsi, false);
	mipi_dsi_set_max_return_size(dsi, panel->esd_read_count);
	mipi_dsi_dcs_read(dsi, panel->esd_reg, read_buf,
				panel->esd_read_count);

	for (i = 0; i < panel->esd_read_count; i++) {
		if (read_buf[i] != panel->esd_return_code[i]) {
			pr_err("error code read_buf[%d] = <0x%x>\n",
					i, read_buf[i]);
			return -ENODATA;
		}
		pr_debug("read_buf[%d] = 0x%x\n", i, read_buf[i]);
	}

	return 0;
}

static uint32_t mipi_panel_read_id(struct panel_device *pd)
{
	struct sprd_dsi *dsi = dev_get_drvdata(pd->intf);
	struct panel_info *panel = pd->panel;
	uint8_t read_buf[4] = {0};
	int i;

	mipi_dsi_lp_cmd_enable(dsi, true);
	mipi_dsi_set_max_return_size(dsi, panel->id_size);
	mipi_dsi_dcs_read(dsi, panel->id_reg, read_buf, panel->id_size);

	for (i = 0; i < panel->id_size; i++) {
		if (read_buf[i] != panel->id_val[i]) {
			pr_err("read id failed!\n");
			return -ENODATA;
		}
		pr_info("read id[%d] = 0x%x\n", i, read_buf[i]);
	}

	return 0;
}

static int32_t mipi_panel_sleep_in(struct panel_device *pd)
{
	struct sprd_dsi *dsi = dev_get_drvdata(pd->intf);
	struct panel_info *panel = pd->panel;

	mipi_dsi_set_work_mode(dsi, SPRD_MIPI_MODE_CMD);
	mipi_dsi_lp_cmd_enable(dsi, true);
	mipi_dsi_send_cmds(dsi, panel->cmd_codes[CMD_CODE_SLEEP_IN]);

	return 0;
}

static int32_t mipi_panel_send_cmd(struct panel_device *pd, int cmd_id)
{
	struct sprd_dsi *dsi = dev_get_drvdata(pd->intf);
	struct panel_info *panel = pd->panel;

	if (cmd_id >= CMD_CODE_MAX) {
		pr_err("cmd_id invalid\n");
		return -1;
	}

	mipi_dsi_lp_cmd_enable(dsi, true);
	mipi_dsi_send_cmds(dsi, panel->cmd_codes[cmd_id]);

	return 0;
}

static int32_t mipi_panel_set_brightness(struct panel_device *pd,
					int level)
{
	struct sprd_dsi *dsi = dev_get_drvdata(pd->intf);
	struct panel_info *panel = pd->panel;
	uint8_t *payload;
	uint8_t len;

	payload = panel->cmd_codes[CMD_OLED_BRIGHTNESS]->cmds[level].payload;
	len = panel->cmd_codes[CMD_OLED_BRIGHTNESS]->cmds[level].header.len;

	mipi_dsi_lp_cmd_enable(dsi, false);
	mipi_dsi_send_cmds(dsi, panel->cmd_codes[CMD_OLED_REG_LOCK]);
	mipi_dsi_gen_write(dsi, payload, len);
	mipi_dsi_send_cmds(dsi, panel->cmd_codes[CMD_OLED_REG_UNLOCK]);

	return 0;
}

struct panel_ops panel_ops_mipi = {
	.init = mipi_panel_init,
	.sleep_in = mipi_panel_sleep_in,
	.read_id = mipi_panel_read_id,
	.esd_check = mipi_panel_esd_check,
	.send_cmd = mipi_panel_send_cmd,
	.set_brightness = mipi_panel_set_brightness,
};
