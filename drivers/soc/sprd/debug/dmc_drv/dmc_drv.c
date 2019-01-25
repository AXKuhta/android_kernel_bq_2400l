/*copyright (C) 2017 Spreadtrum Communications Inc.
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
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/mfd/syscon/sprd-glb.h>
#include <linux/mfd/syscon.h>
#include <linux/of.h>
#include <linux/regmap.h>


static unsigned long long  dram_size;
static struct proc_dir_entry *sprd_dmc_proc_dir;

static int sprd_dmc_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%lld\n", (long long int)(dram_size/1024/1024));
	return 0;
}

static int sprd_dmc_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, sprd_dmc_proc_show, PDE_DATA(inode));
}

static const struct file_operations sprd_dmc_fops = {
	.open = sprd_dmc_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int dmc_proc_creat(void)
{
#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *tmp_proc;

	sprd_dmc_proc_dir = proc_mkdir("sprd_dmc", NULL);
	if (!sprd_dmc_proc_dir)
		return -ENOMEM;

	tmp_proc = proc_create_data("property", S_IRUGO,
			     sprd_dmc_proc_dir, &sprd_dmc_fops, NULL);
	if (!tmp_proc)
		return -ENOMEM;
#endif
	return 0;
}

static int __init sci_dmc_init(void)
{
	u32 size_l, size_h;
	u32 reg_off_sizel, reg_off_sizeh;
	struct device_node *np;
	struct regmap *sprd_syscon_dmc;

	dram_size = 0;

	np = of_find_compatible_node(NULL, NULL, "sprd,sys-dmc-phy");
	if  (!np) {
		pr_err("Error: device node sprd,sys-dmc-phy is not found.\n");
		return -ENODEV;
	}

	sprd_syscon_dmc = syscon_regmap_lookup_by_compatible(
						"sprd,sys-dmc-phy");
	if (IS_ERR(sprd_syscon_dmc)) {
		pr_err("Error: sprd,sys-dmc-phy remap failed.\n");
		return -ENODEV;
	}

	if (of_property_read_u32_index(np,
			"sprd,sizel_off", 0, &reg_off_sizel)) {
		pr_err("Error: can't read sizel reg offset!\n");
		return -EINVAL;
	}
	if (of_property_read_u32_index(np,
			"sprd,sizeh_off", 0, &reg_off_sizeh)) {
		pr_err("Error: can't read sizeh reg offset!\n");
		return -EINVAL;
	}

	regmap_read(sprd_syscon_dmc, reg_off_sizel, &size_l);
	regmap_read(sprd_syscon_dmc, reg_off_sizeh, &size_h);
	dram_size = (((unsigned long long)size_h) << 32) | size_l;

	pr_info("sci_dmc_init: get dram_size  %#llx bytes\n", dram_size);

	dmc_proc_creat();

	return 0;
}

static void __exit sci_dmc_exit(void)
{
}

module_init(sci_dmc_init);
module_exit(sci_dmc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("spreadtrum");
MODULE_DESCRIPTION("spreadtrum dmc drv");
