/*
 * Copyright (C) 2017 Spreadtrum Communications Inc.
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

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/mfd/syscon.h>
#include <linux/mfd/syscon/sprd/sharkl3/glb.h>
#include <linux/regmap.h>

#include "sprd_dispc.h"

static struct dispc_clk_context {
	struct clk *clk_src_128m;
	struct clk *clk_src_153m6;
	struct clk *clk_src_384m;
	struct clk *clk_dpu_core;
	struct clk *clk_dpu_dpi;
} dispc_clk_ctx;

static struct dispc_glb_context {
	struct clk *clk_aon_apb_disp_eb;
	struct clk *clk_aon_apb_disp_emc_eb;
	struct regmap *aon_apb;
} dispc_glb_ctx;

static int dispc_clk_parse_dt(struct dispc_context *ctx,
				struct device_node *np)
{
	int ret;
	struct dispc_clk_context *clk_ctx = &dispc_clk_ctx;

	clk_ctx->clk_src_128m =
		of_clk_get_by_name(np, "clk_src_128m");
	clk_ctx->clk_src_153m6 =
		of_clk_get_by_name(np, "clk_src_153m6");
	clk_ctx->clk_src_384m =
		of_clk_get_by_name(np, "clk_src_384m");
	clk_ctx->clk_dpu_core =
		of_clk_get_by_name(np, "clk_dpu_core");
	clk_ctx->clk_dpu_dpi =
		of_clk_get_by_name(np, "clk_dpu_dpi");

	if (IS_ERR(clk_ctx->clk_src_128m)) {
		pr_warn("read clk_src_128m failed\n");
		clk_ctx->clk_src_128m = NULL;
	}

	if (IS_ERR(clk_ctx->clk_src_153m6)) {
		pr_warn("read clk_src_153m6 failed\n");
		clk_ctx->clk_src_153m6 = NULL;
	}

	if (IS_ERR(clk_ctx->clk_src_384m)) {
		pr_warn("read clk_src_384m failed\n");
		clk_ctx->clk_src_384m = NULL;
	}

	if (IS_ERR(clk_ctx->clk_dpu_core)) {
		pr_warn("read clk_dpu_core failed\n");
		clk_ctx->clk_dpu_core = NULL;
	}

	if (IS_ERR(clk_ctx->clk_dpu_dpi)) {
		pr_warn("read clk_dpu_dpi failed\n");
		clk_ctx->clk_dpu_dpi = NULL;
	}

	ret = of_property_read_u32(np, "sprd,dpi-clk-src", &ctx->dpi_clk_src);
	if (ret) {
		pr_warn("read sprd,dpi-clk-src failed");
		ctx->dpi_clk_src = 128000000;
	}
	pr_info("the dpi clock source from dts is %d\n", ctx->dpi_clk_src);

	return 0;
}

static int dispc_clk_init(struct dispc_context *ctx)
{
	int ret;
	struct dispc_clk_context *clk_ctx = &dispc_clk_ctx;

	ret = clk_set_parent(clk_ctx->clk_dpu_core, clk_ctx->clk_src_384m);
	if (ret)
		pr_warn("set dpu core clk source failed\n");

	ret = clk_set_parent(clk_ctx->clk_dpu_dpi, clk_ctx->clk_src_153m6);
	if (ret)
		pr_warn("set dpi clk source failed\n");

	return ret;
}

static int dispc_clk_enable(struct dispc_context *ctx)
{
	int ret;
	struct dispc_clk_context *clk_ctx = &dispc_clk_ctx;

	ret = clk_prepare_enable(clk_ctx->clk_dpu_core);
	if (ret) {
		pr_err("enable clk_dpu_core error\n");
		return ret;
	}

	ret = clk_prepare_enable(clk_ctx->clk_dpu_dpi);
	if (ret) {
		pr_err("enable clk_dpu_dpi error\n");
		clk_disable_unprepare(clk_ctx->clk_dpu_core);
		return ret;
	}

	return 0;
}

static int dispc_clk_disable(struct dispc_context *ctx)
{
	struct dispc_clk_context *clk_ctx = &dispc_clk_ctx;

	clk_disable_unprepare(clk_ctx->clk_dpu_dpi);
	clk_disable_unprepare(clk_ctx->clk_dpu_core);

	clk_set_parent(clk_ctx->clk_dpu_dpi, clk_ctx->clk_src_128m);
	clk_set_parent(clk_ctx->clk_dpu_core, clk_ctx->clk_src_153m6);

	return 0;
}

static int dispc_clk_update(struct dispc_context *ctx, int clk_id, int val)
{
	int ret;
	struct dispc_clk_context *clk_ctx = &dispc_clk_ctx;

	switch (clk_id) {
	case DISPC_CLK_ID_CORE:
		pr_err("dispc core clk doesn't support update\n");
		break;

	case DISPC_CLK_ID_DPI:
		pr_info("dpi_clk = %d\n", val);
		ret = clk_set_rate(clk_ctx->clk_dpu_dpi, val);
		if (ret)
			pr_err("dispc update dbi clk rate fail\n");
		break;

	default:
		pr_err("clk id %d doesn't support\n", clk_id);
		break;
	}

	return 0;
}

static int dispc_glb_parse_dt(struct dispc_context *ctx,
				struct device_node *np)
{
	struct dispc_glb_context *glb_ctx = &dispc_glb_ctx;

	glb_ctx->aon_apb = syscon_regmap_lookup_by_phandle(np,
					    "sprd,syscon-aon-apb");
	if (IS_ERR(glb_ctx->aon_apb))
		pr_warn("parse syscon-aon-apb failed\n");

	glb_ctx->clk_aon_apb_disp_eb =
		of_clk_get_by_name(np, "clk_aon_apb_disp_eb");
	if (IS_ERR(glb_ctx->clk_aon_apb_disp_eb)) {
		pr_warn("read clk_aon_apb_disp_eb failed\n");
		glb_ctx->clk_aon_apb_disp_eb = NULL;
	}

	glb_ctx->clk_aon_apb_disp_emc_eb =
		of_clk_get_by_name(np, "clk_aon_apb_disp_emc_eb");
	if (IS_ERR(glb_ctx->clk_aon_apb_disp_emc_eb)) {
		pr_warn("read clk_aon_apb_disp_emc_eb failed\n");
		glb_ctx->clk_aon_apb_disp_emc_eb = NULL;
	}

	return 0;
}

static void dispc_glb_enable(struct dispc_context *ctx)
{
	int ret;
	struct dispc_glb_context *glb_ctx = &dispc_glb_ctx;

	ret = clk_prepare_enable(glb_ctx->clk_aon_apb_disp_eb);
	if (ret) {
		pr_err("enable clk_aon_apb_disp_eb failed!\n");
		return;
	}

	ret = clk_prepare_enable(glb_ctx->clk_aon_apb_disp_emc_eb);
	if (ret) {
		pr_err("enable clk_aon_apb_disp_emc_eb failed!\n");
		clk_disable_unprepare(glb_ctx->clk_aon_apb_disp_eb);
	}
}

static void dispc_glb_disable(struct dispc_context *ctx)
{
	struct dispc_glb_context *glb_ctx = &dispc_glb_ctx;

	clk_disable_unprepare(glb_ctx->clk_aon_apb_disp_emc_eb);
	clk_disable_unprepare(glb_ctx->clk_aon_apb_disp_eb);
}

static void dispc_reset(struct dispc_context *ctx)
{
	struct dispc_glb_context *glb_ctx = &dispc_glb_ctx;

	regmap_update_bits(glb_ctx->aon_apb,
		    REG_AON_APB_APB_RST1, BIT_AON_APB_DISP_SOFT_RST,
		    BIT_AON_APB_DISP_SOFT_RST);
	udelay(10);
	regmap_update_bits(glb_ctx->aon_apb,
		    REG_AON_APB_APB_RST1, BIT_AON_APB_DISP_SOFT_RST,
		    (unsigned int)(~BIT_AON_APB_DISP_SOFT_RST));
}

static void dispc_power_domain(struct dispc_context *ctx, int enable)
{
	/* The dispc power domain code is in video/disp_pw_domain. */
}

static struct dispc_clk_ops dispc_clk_ops = {
	.parse_dt = dispc_clk_parse_dt,
	.init = dispc_clk_init,
	.enable = dispc_clk_enable,
	.disable = dispc_clk_disable,
	.update = dispc_clk_update,
};

static struct dispc_glb_ops dispc_glb_ops = {
	.parse_dt = dispc_glb_parse_dt,
	.reset = dispc_reset,
	.enable = dispc_glb_enable,
	.disable = dispc_glb_disable,
	.power = dispc_power_domain,
};

static struct ops_entry clk_entry = {
	.ver = "sharkl3",
	.ops = &dispc_clk_ops,
};

static struct ops_entry glb_entry = {
	.ver = "sharkl3",
	.ops = &dispc_glb_ops,
};

static int __init dispc_glb_register(void)
{
	dispc_clk_ops_register(&clk_entry);
	dispc_glb_ops_register(&glb_entry);
	return 0;
}

subsys_initcall(dispc_glb_register);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("leon.he@spreadtrum.com");
MODULE_DESCRIPTION("sprd sharkl3 dispc global and clk regs config");
