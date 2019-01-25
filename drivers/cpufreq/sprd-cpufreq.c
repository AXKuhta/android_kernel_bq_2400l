/*
 * Copyright (C) 2015 Spreadtrum Communications Inc.
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

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_opp.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/sprd_otp.h>
#include <linux/of_platform.h>
#include <linux/sprd-cpufreq.h>

#define PR_DEBUG(fmt, ...) pr_info("%s: " fmt, __func__, ##__VA_ARGS__)
#define PR_ERROR(fmt, ...) pr_err("error: " fmt, ##__VA_ARGS__)

/* Default voltage_tolerance */
#define DEF_VOLT_TOL		0
/* Regulator Supply */
#define CORE_SUPPLY		"cpu"
/* Core Clocks */
#define CORE_CLK	"core_clk"
#define LOW_FREQ_CLK_PARENT	"low_freq_clk_parent"
#define HIGH_FREQ_CLK_PARENT	"high_freq_clk_parent"
/*0-cluster0; 1-custer1;  2/3-cci or fcm*/
#define SPRD_CPUFREQ_MAX_MODULE  4
/*0-cluster0; 1-custer1*/
#define SPRD_CPUFREQ_MAX_CLUSTER  2
#define SPRD_CPUFREQ_MAX_FREQ_VOLT 10
#define SPRD_CPUFREQ_MAX_TEMP  4
#define SPRD_CPUFREQ_TEMP_FALL_HZ  (2*HZ)
#define SPRD_CPUFREQ_TEMP_UPDATE_HZ  (HZ/2)
#define SPRD_CPUFREQ_TEMP_MAX  200
#define SPRD_CPUFREQ_TEMP_MIN  (-200)
#define SPRD_CPUFREQ_DRV_BOOST_DURATOIN	(60ul*HZ)

#define sprd_cpufreq_data(cpu) \
	cpufreq_datas[topology_physical_package_id(cpu)]
#define is_big_cluster(cpu) (topology_physical_package_id(cpu) != 0)

struct sprd_cpufreq_group {
	unsigned long freq;/*HZ*/
	unsigned long volt;/*uV*/
};
struct sprd_cpufreq_driver_data {
	unsigned int cluster;
	unsigned int online;
	struct mutex *volt_lock;
	struct device *cpu_dev;
	struct regulator *reg;
	/*const means clk_high is constant clk source*/
	unsigned int clk_high_freq_const;
	unsigned int clk_en;
	struct clk *clk;
	struct clk *clk_low_freq_p;
	struct clk *clk_high_freq_p;
	unsigned long clk_low_freq_p_max;
	unsigned long clk_high_freq_p_max;
	/* voltage tolerance in percentage */
	unsigned int volt_tol;
	unsigned int volt_tol_var;
	/*volt requested by this cluster*/
	unsigned long volt_req;
	unsigned int volt_share_hosts_bits;
	unsigned int volt_share_masters_bits;
	unsigned int volt_share_slaves_bits;
	unsigned long freq_req;
	unsigned int freq_sync_hosts_bits;
	unsigned int freq_sync_slaves_bits;
	unsigned int freqvolts;
	struct sprd_cpufreq_group
		freqvolt[SPRD_CPUFREQ_MAX_FREQ_VOLT];
	unsigned int temp_max_freq;
	int temp_list[SPRD_CPUFREQ_MAX_TEMP];
	int temp_max;
	int temp_top;
	int temp_now;
	int temp_bottom;
	unsigned long temp_fall_time;
};

static struct sprd_cpufreq_driver_data
	*cpufreq_datas[SPRD_CPUFREQ_MAX_MODULE];
static unsigned long boot_done_timestamp;
static int boost_mode_flag = 1;
static struct cpufreq_driver sprd_cpufreq_driver;
static int sprd_cpufreq_set_boost(int state);
static int sprd_cpufreq_set_target(
	struct sprd_cpufreq_driver_data *cpufreq_data,
	unsigned int idx, bool force);

static int efuse_uid_waferid_get(struct device *dev,
	struct device_node *np_cpufreq_data, u32 *p_binning_data)
{
	char uid[50];
	const struct property *prop = NULL, *prop1 = NULL;
	struct device_node *dn_cpufreq_data;
	char *p1, *p2;

	if ((dev == NULL && np_cpufreq_data == NULL) ||
	p_binning_data == NULL) {
		pr_info("%s: inputs are NULL\n", __func__);
		return -ENOENT;
	}

	if (dev && !of_node_get(dev->of_node)) {
		dev_info(dev, "sprd_cpufreq: %s not found cpu node\n",
			__func__);
		return -ENOENT;
	}

	if (dev)
		prop = of_find_property(dev->of_node,
			"sprd,ss-waferid-names", NULL);
	prop1 = of_find_property(np_cpufreq_data,
		"sprd,ss-waferid-names", NULL);

	if (!prop && !prop1)
		return -ENODEV;
	if (prop && !prop->value)
		return -ENODATA;
	if (prop1 && !prop1->value)
		return -ENODATA;
	if (prop)
		dn_cpufreq_data = dev->of_node;
	if (prop1)
		dn_cpufreq_data = np_cpufreq_data;

	memset(uid, 0, sizeof(uid));
	sprd_get_chip_uid(uid);
	/*find second '_', and set 0, strcpy(uid, "T8F465_9_12_28");*/
	p1 = strchr(uid, '_');
	if (!p1)
		return -ENODATA;
	p1++;
	p2 = strchr(p1, '_');
	if (!p2)
		return -ENODATA;
	*p2 = 0;
	if (of_property_match_string(dn_cpufreq_data,
				"sprd,ss-waferid-names", uid) >= 0) {
		*p_binning_data = 3;
	} else {
		*p_binning_data = 4;
	}

	pr_info("waferid BIN[%u] uid[%s]\n", *p_binning_data, uid);
	return 0;
}

#if defined(CONFIG_OTP_SPRD_AP_PUBLIC_EFUSE)
static int efuse_binning_low_volt_get(struct device *dev,
	struct device_node *np_cpufreq_data, u32 *p_binning_data)
{
	const struct property *prop, *prop1;
	int nLSB;
	u32 efuse_blk_binning;
	u32 efuse_blk_binning_bits;
	u32 efuse_blk_binning_data;
	struct device_node *cpu_np;
	const __be32 *val;

	if (!dev || !np_cpufreq_data || !p_binning_data)
		return -ENOENT;

	cpu_np = of_node_get(dev->of_node);
	if (!cpu_np) {
		dev_info(dev, "sprd_cpufreq: efuse_binning_get failed to find cpu node\n");
		return -ENOENT;
	}

	prop = of_find_property(dev->of_node,
					"sprd,efuse-blk-binning-low-volt", NULL);
	prop1 = of_find_property(np_cpufreq_data,
					"sprd,efuse-blk-binning-low-volt", NULL);
	if (!prop && !prop1)
		return -ENODEV;
	if (prop && !prop->value)
		return -ENODATA;
	if (prop1 && !prop1->value)
		return -ENODATA;
	if (prop1)
		prop = prop1;

	if (prop->length  != (2*sizeof(u32))) {
		dev_err(dev, "sprd_cpufreq: %s: Invalid efuse_blk_binning(prop->length %d)\n",
				__func__, prop->length);
		return -EINVAL;
	}
	val = prop->value;
	efuse_blk_binning = be32_to_cpup(val++);
	efuse_blk_binning_bits = be32_to_cpup(val);
	nLSB = __ffs(efuse_blk_binning_bits);

	efuse_blk_binning_data = sprd_efuse_double_read(efuse_blk_binning, 1);
	efuse_blk_binning_data = efuse_blk_binning_data & efuse_blk_binning_bits;
	if (nLSB != 0) {
		efuse_blk_binning_data = efuse_blk_binning_data>>nLSB;
	}
	dev_info(dev, "sprd_cpufreq: blk 0x%x, bits 0x%x, nLSB %d, BIN %u\n",
			efuse_blk_binning, efuse_blk_binning_bits,
			nLSB, efuse_blk_binning_data);
	if (efuse_blk_binning_data == 0) {
		*p_binning_data = 0;
		return -1;
	}
	*p_binning_data = efuse_blk_binning_data;

	return 0;
}
#endif

static int efuse_binning_get(struct device *dev,
struct device_node *np_cpufreq_data, u32 *p_binning_data)
{
	const struct property *prop = NULL, *prop1 = NULL;
	int nLSB;
	u32 efuse_blk_binning;
	u32 efuse_blk_binning_bits;
	u32 efuse_blk_binning_data;
	const __be32 *val;

	if ((dev == NULL && np_cpufreq_data == NULL) ||
	p_binning_data == NULL) {
		pr_info("%s: inputs are NULL\n", __func__);
		return -ENOENT;
	}

	if (dev && !of_node_get(dev->of_node)) {
		dev_info(dev, "sprd_cpufreq: %s not found cpu node\n",
			__func__);
		return -ENOENT;
	}

	if (dev)
		prop = of_find_property(dev->of_node,
			"sprd,efuse-blk-binning", NULL);
	prop1 = of_find_property(np_cpufreq_data,
					"sprd,efuse-blk-binning", NULL);
	if (!prop && !prop1)
		return -ENODEV;
	if (prop && !prop->value)
		return -ENODATA;
	if (prop1 && !prop1->value)
		return -ENODATA;
	if (prop1)
		prop = prop1;

	if (prop->length  != (2*sizeof(u32))) {
		pr_err("%s: Invalid efuse_blk_binning(prop->length %d)\n",
				__func__, prop->length);
		return -EINVAL;
	}
	val = prop->value;
	efuse_blk_binning = be32_to_cpup(val++);
	efuse_blk_binning_bits = be32_to_cpup(val);
	nLSB = __ffs(efuse_blk_binning_bits);

#if defined(CONFIG_OTP_SPRD_AP_EFUSE) || defined(CONFIG_OTP_SPRD_AP_IEFUSE)
	efuse_blk_binning_data = sprd_ap_efuse_read(efuse_blk_binning);
#elif defined(CONFIG_OTP_SPRD_AP_PUBLIC_EFUSE)
	efuse_blk_binning_data = sprd_efuse_double_read(efuse_blk_binning, 1);
#else
	efuse_blk_binning_data = 0;
#endif

	efuse_blk_binning_data = efuse_blk_binning_data & efuse_blk_binning_bits;
	if (nLSB != 0)
		efuse_blk_binning_data = efuse_blk_binning_data>>nLSB;

	pr_info("blk 0x%x, bits 0x%x, nLSB %d, BIN %u\n",
			efuse_blk_binning, efuse_blk_binning_bits,
			nLSB, efuse_blk_binning_data);

	if (efuse_blk_binning_data == 0 || efuse_blk_binning_data > 4) {
		*p_binning_data = 0;
		return -1;
	}
	*p_binning_data = efuse_blk_binning_data;

	return 0;
}

static int temp_binning_get(struct device *dev,
	struct device_node *np_cpufreq_data,
	struct sprd_cpufreq_driver_data *cpufreq_data,
	char *opp_temp)
{
	const struct property *prop = NULL, *prop1 = NULL;
	int i = 0;
	u32 temp_threshold;
	const __be32 *val;
	int temp_index;

	if ((dev == NULL && np_cpufreq_data == NULL) ||
	cpufreq_data == NULL ||
	opp_temp == NULL) {
		pr_info("%s: inputs are NULL\n", __func__);
		return -ENOENT;
	}

	if (dev && !of_node_get(dev->of_node)) {
		dev_info(dev, "sprd_cpufreq: %s not found cpu node\n",
			__func__);
		return -ENOENT;
	}

	if (dev)
		prop = of_find_property(dev->of_node,
			"sprd,cpufreq-temp-threshold", NULL);
	prop1 = of_find_property(np_cpufreq_data,
		"sprd,cpufreq-temp-threshold", NULL);
	if (!prop && !prop1)
		return -ENODEV;
	if (prop && !prop->value)
		return -ENODATA;
	if (prop1 && !prop1->value)
		return -ENODATA;
	if (prop1)
		prop = prop1;

	if (prop->length < sizeof(u32)) {
		pr_err("sprd_cpufreq: %s: Invalid temp_binning_get(prop->length %d)\n",
				__func__, prop->length);
		return -EINVAL;
	}

	cpufreq_data->temp_max_freq = 0;
	cpufreq_data->temp_max = 0;
	temp_index = -1;
	val = prop->value;
	for (i = 0;
	i < (prop->length/sizeof(u32)) && i < SPRD_CPUFREQ_MAX_TEMP;
	i++) {
		/*TODO: need to compatible with negative degree celsius*/
		temp_threshold = be32_to_cpup(val++);
		if (cpufreq_data->temp_now >= temp_threshold) {
			temp_index = i;
			sprintf(opp_temp, "-%d", temp_threshold);
		}
		cpufreq_data->temp_list[i] = temp_threshold;
		cpufreq_data->temp_max++;
		pr_debug("found temp %u\n", temp_threshold);
	}

	cpufreq_data->temp_bottom = temp_index < 0 ?
				SPRD_CPUFREQ_TEMP_MIN :
				cpufreq_data->temp_list[temp_index];
	cpufreq_data->temp_top = (temp_index + 1) >= cpufreq_data->temp_max ?
				SPRD_CPUFREQ_TEMP_MAX :
				cpufreq_data->temp_list[temp_index + 1];

	pr_debug("temp_binning_get max num=%d bottom=%d top=%d\n",
		cpufreq_data->temp_max,
		cpufreq_data->temp_bottom,
		cpufreq_data->temp_top);

	pr_info("temp_binning_get[%s] by temp %d\n",
		opp_temp, cpufreq_data->temp_now);
	return 0;
}

/* Initializes OPP tables based on old-deprecated bindings */
static int dev_pm_opp_of_add_table_binning(int cluster,
	struct device *dev,
	struct device_node *np_cpufreq_data,
	struct sprd_cpufreq_driver_data *cpufreq_data)
{
	struct device_node *cpu_np = NULL;
	struct device_node *np = NULL, *np1 = NULL;
	const struct property *prop = NULL, *prop1 = NULL;
	const __be32 *val;
	int nr;
	u32 p_binning_data = 0;
	int index = 0, count = 0;
	char opp_string[30] = "operating-points";
	char temp_string[20] = "";
#if defined(CONFIG_OTP_SPRD_AP_PUBLIC_EFUSE)
	u32 p_binning_low_volt = 0;
#endif

	if ((dev == NULL && np_cpufreq_data == NULL) ||
	cpufreq_data == NULL) {
		pr_info("%s: inputs are NULL\n", __func__);
		return -ENOENT;
	}

	if (dev && np_cpufreq_data == NULL) {
		cpu_np = of_node_get(dev->of_node);
		if (!cpu_np) {
			dev_err(dev, "sprd_cpufreq: failed to find cpu node\n");
			return -ENOENT;
		}

		np = of_parse_phandle(cpu_np, "cpufreq-data", 0);
		np1 = of_parse_phandle(cpu_np, "cpufreq-data-v1", 0);
		if (!np && !np1) {
			dev_err(dev, "sprd_cpufreq: failed to find cpufreq-data\n");
			of_node_put(cpu_np);
			return -ENOENT;
		}
		if (np1)
			np = np1;
		np_cpufreq_data = np;
	}

#if defined(CONFIG_OTP_SPRD_AP_EFUSE) || defined(CONFIG_OTP_SPRD_AP_IEFUSE)
	if (!efuse_binning_get(dev, np_cpufreq_data, &p_binning_data))
#elif defined(CONFIG_OTP_SPRD_AP_PUBLIC_EFUSE)
	if (efuse_binning_get(dev, np_cpufreq_data, &p_binning_data) >= 0)
#endif
	{
		pr_info("p_binning_data=0x%x by BIN\n",
				p_binning_data);

		index = strlen(opp_string);
		opp_string[index++] = '-';
		opp_string[index++] = '0' + p_binning_data;
		opp_string[index] = '\0';

	#if defined(CONFIG_OTP_SPRD_AP_PUBLIC_EFUSE)
		efuse_binning_low_volt_get(dev, np_cpufreq_data, &p_binning_low_volt);

		opp_string[index++] = '-';
		opp_string[index++] = '0' + p_binning_low_volt;
		opp_string[index] = '\0';
	#endif

		/*select dvfs table by temp only if bin is not zero*/
		if (!temp_binning_get(dev,
		np_cpufreq_data, cpufreq_data, temp_string))
		strcat(opp_string, temp_string);

	} else {
		efuse_uid_waferid_get(dev, np_cpufreq_data, &p_binning_data);
		pr_info("p_binning_data=0x%x by UID\n",
				p_binning_data);

		if (p_binning_data > 0) {
			index = strlen(opp_string);
			opp_string[index++] = '-';
			opp_string[index++] = '0' + p_binning_data;
			opp_string[index] = '\0';
			/*select dvfs table by temp only if bin is not zero*/
			if (!temp_binning_get(dev,
			np_cpufreq_data, cpufreq_data, temp_string))
				strcat(opp_string, temp_string);
		}
	}

	pr_info("opp_string[%s]\n", opp_string);

	if (dev)
		prop = of_find_property(dev->of_node, opp_string, NULL);
	prop1 = of_find_property(np_cpufreq_data, opp_string, NULL);
	if (!prop && !prop1)
		return -ENODEV;
	if (prop && !prop->value)
		return -ENODATA;
	if (prop1 && !prop1->value)
		return -ENODATA;
	if (prop1)
		prop = prop1;
	/*
	 * Each OPP is a set of tuples consisting of frequency and
	 * voltage like <freq-kHz vol-uV>.
	 */
	nr = prop->length / sizeof(u32);
	if (nr % 2) {
		pr_err("%s: Invalid OPP list\n", __func__);
		return -EINVAL;
	}

	cpufreq_data->freqvolts = 0;
	val = prop->value;
	while (nr) {
		unsigned long freq = be32_to_cpup(val++) * 1000;
		unsigned long volt = be32_to_cpup(val++);

		if (dev)
			dev_pm_opp_remove(dev, freq);
		if (dev && dev_pm_opp_add(dev, freq, volt))
			dev_warn(dev, "sprd_cpufreq:dev_pm Failed to add OPP %ld\n",
				freq);
		else {
			if (freq/1000 > cpufreq_data->temp_max_freq)
				cpufreq_data->temp_max_freq = freq/1000;
			pr_info("cluster%d opp_add [freq-%lu & volt-%lu]\n",
					cluster,
					freq, volt);
		}
		if (count < SPRD_CPUFREQ_MAX_FREQ_VOLT) {
			cpufreq_data->freqvolt[count].freq = freq;
			cpufreq_data->freqvolt[count].volt = volt;
			cpufreq_data->freqvolts++;
		}

		count++;
		nr -= 2;
	}

	return 0;
}

static int sprd_verify_opp_with_regulator(struct device *cpu_dev,
		struct regulator *cpu_reg, unsigned int volt_tol)
{
	unsigned long opp_freq = 0;
	unsigned long min_uV = ~0, max_uV = 0;

	while (1) {
		struct dev_pm_opp *opp;
		unsigned long opp_uV, tol_uV;

		rcu_read_lock();
		opp = dev_pm_opp_find_freq_ceil(cpu_dev, &opp_freq);
		if (IS_ERR(opp)) {
			/* We dont have more freq in opp table,
			 * break the loop
			*/
			rcu_read_unlock();
			pr_debug("invalid opp freq %lu\n", opp_freq);
			break;
		}
		opp_uV = dev_pm_opp_get_voltage(opp);
		rcu_read_unlock();
		tol_uV = opp_uV * volt_tol / 100;
		if (regulator_is_supported_voltage(cpu_reg, opp_uV,
							   opp_uV + tol_uV)) {
			if (opp_uV < min_uV)
				min_uV = opp_uV;
			if (opp_uV > max_uV)
				max_uV = opp_uV;
		} else {
			pr_warn("disabling unsupported opp_freq %lu\n",
					opp_freq);
			dev_pm_opp_disable(cpu_dev, opp_freq);
		}
	opp_freq++;
	}
	pr_debug("regulator min volt %lu, max volt %lu\n", min_uV, max_uV);

	return regulator_set_voltage_time(cpu_reg, min_uV, max_uV);
}

/**
 * sprd_cpufreq_update_opp() - returns the max freq of a cpu
 * and update dvfs table by temp_now
 * @cpu: which cpu you want to update dvfs table
 * @temp_now: current temperature on this cpu, mini-degree.
 *
 * Return:
 * 1.cluster is not working, then return 0
 * 2.succeed to update dvfs table
 * then return max freq(KHZ) of this cluster
 */
unsigned int sprd_cpufreq_update_opp(int cpu, int temp_now)
{
	unsigned int max_freq = 0;
	struct sprd_cpufreq_driver_data *c;
	int cluster;

	temp_now = temp_now/1000;
	if (temp_now <= SPRD_CPUFREQ_TEMP_MIN ||
	temp_now >= SPRD_CPUFREQ_TEMP_MAX)
		return 0;

	cluster = topology_physical_package_id(cpu);
	if (cluster > SPRD_CPUFREQ_MAX_CLUSTER) {
		pr_err("cpu%d is overflowd %d\n", cpu,
			SPRD_CPUFREQ_MAX_CLUSTER);
		return 0;
	}

	c = cpufreq_datas[cluster];

	if (c != NULL &&
	c->online &&
	c->temp_max > 0) {
		/*Never block IPA thread*/
		if (mutex_trylock(c->volt_lock) != 1)
			return 0;
		c->temp_now = temp_now;
		if (temp_now < c->temp_bottom && !c->temp_fall_time)
			c->temp_fall_time = jiffies + SPRD_CPUFREQ_TEMP_FALL_HZ;
		if (temp_now >= c->temp_bottom)
			c->temp_fall_time = 0;
		if (temp_now >= c->temp_top ||
		(c->temp_fall_time &&
		time_after(jiffies, c->temp_fall_time))) {
			c->temp_fall_time = 0;
			if (!dev_pm_opp_of_add_table_binning(
				c->cluster, c->cpu_dev,
				NULL, c))
				max_freq = c->temp_max_freq;
			dev_info(c->cpu_dev,
					"update temp_max_freq %u\n", max_freq);
		}
		mutex_unlock(c->volt_lock);
	}

	return max_freq;
}
EXPORT_SYMBOL_GPL(sprd_cpufreq_update_opp);

static int sprd_cpufreq_set_clock(struct sprd_cpufreq_driver_data *c,
						unsigned long freq)
{
	struct clk *clk_low_freq_p;
	struct clk *clk_high_freq_p;
	unsigned long clk_low_freq_p_max = 0;
	int ret = 0;

	if (c == NULL || freq == 0)
		return -ENODEV;

	clk_low_freq_p = c->clk_low_freq_p;
	clk_high_freq_p = c->clk_high_freq_p;
	clk_low_freq_p_max = c->clk_low_freq_p_max;

	ret = clk_set_parent(c->clk, clk_low_freq_p);
	if (ret) {
		PR_ERROR("error in setting clk_low_freq_p as parent\n");
		return ret;
	}
	if (freq != clk_low_freq_p_max) {
		if (clk_set_rate(clk_high_freq_p, freq))
			PR_ERROR("in setting clk_high_freq_p freq %luKHz\n",
				freq / 1000);
		ret = clk_set_parent(c->clk, clk_high_freq_p);
		if (ret) {
			PR_ERROR("in setting clk_high_freq_p as parent\n");
			return ret;
		}
	}
	return 0;
}

static int sprd_cpufreq_set_clock_v1(struct sprd_cpufreq_driver_data *c,
						unsigned long freq)
{
	int ret = 0;

	if (c == NULL || freq == 0)
		return -ENODEV;

	if (c->freq_req == freq)
		return 0;

	pr_debug("%s freq=%lu\n", __func__, freq);

	if (freq > c->clk_low_freq_p_max) {
		if (c->clk_high_freq_const == 0)
			ret = clk_set_parent(c->clk, c->clk_low_freq_p);
		if (ret) {
			pr_err("error in pre-setting clk_low_freq_p as parent\n");
			goto exit_err;
		}

		ret = clk_set_rate(c->clk_high_freq_p, freq);
		if (ret) {
			pr_err("in setting clk_high_freq_p freq %luKHz\n",
				freq / 1000);
			goto exit_err;
		}
		ret = clk_set_parent(c->clk, c->clk_high_freq_p);
		if (ret) {
			pr_err("in setting clk_high_freq_p as parent\n");
			goto exit_err;
		}
		PR_DEBUG("set cluster%u setting clk_high as parent\n",
			c->cluster);
	} else if (freq == c->clk_low_freq_p_max) {
		ret = clk_set_parent(c->clk, c->clk_low_freq_p);
		if (ret) {
			pr_err("error in setting clk_low_freq_p as parent\n");
			goto exit_err;
		}
		PR_DEBUG("set cluster%u setting clk_low as parent\n",
			c->cluster);
	} else {
		pr_err("error in setting clk_low_freq_p as parent\n");
		goto exit_err;
	}

	PR_DEBUG("set cluster%u freq %luHZ\n", c->cluster, freq);
	c->freq_req = freq;
	return 0;
exit_err:
	return ret;
}

static struct regulator *sprd_volt_share_reg(struct sprd_cpufreq_driver_data *c)
{
	int cluster;
	struct regulator *reg = NULL;

	if (c == NULL)
		return NULL;

	for (cluster = 0; cluster < SPRD_CPUFREQ_MAX_MODULE; cluster++)
		if ((c->volt_share_masters_bits >> cluster) & 0x1) {
			pr_debug("check master cluster%u\n", cluster);
			if (cpufreq_datas[cluster] != NULL &&
			cpufreq_datas[cluster]->reg) {
				reg = cpufreq_datas[cluster]->reg;
				break;
			}
		}

	if (reg == NULL)
		for (cluster = 0; cluster < SPRD_CPUFREQ_MAX_MODULE; cluster++)
			if ((c->volt_share_hosts_bits >> cluster) & 0x1) {
				pr_debug("check host cluster%u\n", cluster);
				if (cpufreq_datas[cluster] != NULL &&
				cpufreq_datas[cluster]->reg) {
					reg = cpufreq_datas[cluster]->reg;
					break;
				}
			}

	PR_DEBUG("cluster %u masters 0x%x hosts 0x%x reg %p\n",
		c->cluster,
		c->volt_share_masters_bits,
		c->volt_share_hosts_bits,
		reg);

	return reg;
}
static struct mutex *sprd_volt_share_lock(struct sprd_cpufreq_driver_data *c)
{
	int cluster;
	struct mutex *volt_lock = NULL;

	if (c == NULL)
		return NULL;

	for (cluster = 0; cluster < SPRD_CPUFREQ_MAX_MODULE; cluster++)
		if ((c->volt_share_masters_bits >> cluster) & 0x1) {
			pr_debug("check master cluster%u\n", cluster);
			if (cpufreq_datas[cluster] != NULL &&
			cpufreq_datas[cluster]->volt_lock) {
				volt_lock = cpufreq_datas[cluster]->volt_lock;
				break;
			}
		}

	if (volt_lock == NULL)
		for (cluster = 0; cluster < SPRD_CPUFREQ_MAX_MODULE; cluster++)
			if ((c->volt_share_hosts_bits >> cluster) & 0x1) {
				pr_debug("check host cluster%u\n", cluster);
				if (cpufreq_datas[cluster] != NULL &&
				cpufreq_datas[cluster]->volt_lock) {
					volt_lock =
					cpufreq_datas[cluster]->volt_lock;
					break;
				}
			}

	PR_DEBUG("cluster %u masters 0x%x hosts 0x%x volt_lock %p\n",
		c->cluster,
		c->volt_share_masters_bits,
		c->volt_share_hosts_bits,
		volt_lock);

	return volt_lock;
}

static unsigned int sprd_volt_tol_min(struct sprd_cpufreq_driver_data *c,
	bool online)
{
	unsigned int cluster, volt_tol_min = 0;

	if (c == NULL)
		return 0;

	volt_tol_min = c->volt_tol_var;
	for (cluster = 0; cluster < SPRD_CPUFREQ_MAX_MODULE; cluster++)
		if ((c->volt_share_masters_bits >> cluster) & 0x1) {
			pr_debug("check master cluster%u\n", cluster);
			if (cpufreq_datas[cluster] != NULL &&
			(online ? cpufreq_datas[cluster]->online : 1) &&
			cpufreq_datas[cluster]->volt_tol_var < volt_tol_min) {
				volt_tol_min =
					cpufreq_datas[cluster]->volt_tol_var;
		}
	}

	pr_debug("cluster %u volt_share_masters_bits %u volt_tol_min %u\n",
		c->cluster,
		c->volt_share_masters_bits,
		volt_tol_min);

	return volt_tol_min;
}
/**
 * sprd_volt_req_max()  - get volt_req_max
 * @c:        cluster
 * @volt_max_aim: aimed max between *volt_max_p
 *	and max volt of online or all clusters.
 * @online: 0-just search all clusters; 1-just search online clusters.
 * @except_self: 0-just search all clusters; 1-just search online clusters.
 * @return: current max volt of online/all clusters.
 */
static unsigned long sprd_volt_req_max(struct sprd_cpufreq_driver_data *c,
	unsigned long *volt_max_aim, bool online, bool except_self)
{
	int cluster;
	unsigned long volt_max_req = 0, ret = 0;

	if (c == NULL)
		return 0;

	for (cluster = 0; cluster < SPRD_CPUFREQ_MAX_MODULE; cluster++)
		if ((c->volt_share_masters_bits >> cluster) & 0x1) {
			pr_debug("check master cluster%u\n", cluster);
			if ((except_self ? (cluster != c->cluster) : 1) &&
			(cpufreq_datas[cluster] != NULL) &&
			(online ? cpufreq_datas[cluster]->online : 1) &&
			(cpufreq_datas[cluster]->volt_req > volt_max_req)) {
				volt_max_req = cpufreq_datas[cluster]->volt_req;
			}
		}

	if (volt_max_req > *volt_max_aim)
		*volt_max_aim = volt_max_req;
	else
		ret = volt_max_req;

	pr_debug("cluster %u volt_share_masters_bits %u volt_max %lu\n",
		c->cluster,
		c->volt_share_masters_bits,
		*volt_max_aim);

	return ret;
}

static int sprd_freq_sync_by_volt(
	struct sprd_cpufreq_driver_data *c,
	unsigned long volt)
{
	int i = 0, ret = -ENODEV;

	while (i < c->freqvolts) {
		if (c->freqvolt[i].volt > 0 &&
		volt >= c->freqvolt[i].volt)
			break;
		i++;
	}
	if (i < c->freqvolts) {
		ret = sprd_cpufreq_set_clock_v1(c, c->freqvolt[i].freq);
		if (!ret)
			c->volt_req = c->freqvolt[i].volt;
	} else
		pr_info("%s not found more than volt%lu\n", __func__, volt);

	return ret;
}

static int sprd_volt_share_slaves_notify(
	struct sprd_cpufreq_driver_data *host,
	unsigned long volt)
{
	unsigned int cluster;
	int ret;

	pr_debug("%s volt_share_slaves_bits%u, volt %lu\n",
		__func__, host->volt_share_slaves_bits, volt);

	for (cluster = 0; cluster < SPRD_CPUFREQ_MAX_MODULE; cluster++)
		if ((host->volt_share_slaves_bits >> cluster) & 0x1) {
			if (cpufreq_datas[cluster] != NULL &&
			cpufreq_datas[cluster]->online &&
			cpufreq_datas[cluster]->volt_share_hosts_bits) {
				ret = sprd_freq_sync_by_volt(
					cpufreq_datas[cluster], volt);
				if (ret)
					goto EXIT_ERR;
			}
		}

	return 0;
EXIT_ERR:
	pr_info("%s cluster%u,EXIT_ERR!\n", __func__, cluster);
	return ret;
}

static int sprd_freq_sync_slaves_notify(
	struct sprd_cpufreq_driver_data *host,
	const unsigned int idx, bool force)
{
	unsigned int cluster;
	int ret;

	if (!host) {
		PR_DEBUG("invalid host cpufreq_data\n");
		return -ENODEV;
	}

	for (cluster = 0; cluster < SPRD_CPUFREQ_MAX_MODULE; cluster++)
		if ((host->freq_sync_slaves_bits >> cluster) & 0x1) {
			if (cpufreq_datas[cluster] != NULL &&
			cpufreq_datas[cluster]->online &&
			cpufreq_datas[cluster]->freq_sync_hosts_bits) {
				ret = sprd_cpufreq_set_target(
					cpufreq_datas[cluster],
					idx, force);
				if (ret)
					goto EXIT_ERR;
			}
		}

	return 0;
EXIT_ERR:
	pr_info("%s sub-cluster%u EXIT_ERR!\n", __func__, cluster);
	return ret;
}

static int sprd_cpufreq_set_target(
	struct sprd_cpufreq_driver_data *cpufreq_data,
	unsigned int idx, bool force)
{
	struct dev_pm_opp *opp;
	unsigned long volt_new = 0, volt_old = 0;
	unsigned long old_freq_hz, new_freq_hz, freq_Hz, opp_freq_hz;
	unsigned int volt_tol = 0;
	struct regulator *cpu_reg;
	struct clk *cpu_clk;
	struct device *cpu_dev;
	int cluster;
	int ret = 0;

	if (!cpufreq_data ||
	(cpufreq_data && idx >= cpufreq_data->freqvolts)) {
		PR_ERROR("invalid cpufreq_data/idx%u, returning\n", idx);
		return -ENODEV;
	}

	pr_debug("setting target for cluster %d, freq idx %u freqvolts %u\n",
		cpufreq_data->cluster,
		idx,
		cpufreq_data->freqvolts);

	if (IS_ERR(cpufreq_data->clk) ||
	cpufreq_data->clk == NULL ||
	IS_ERR(cpufreq_data->reg) ||
	cpufreq_data->reg == NULL) {
		PR_ERROR("no regulator/clk for cluster %d\n",
			cpufreq_data->cluster);
		return -ENODEV;
	}

	if (idx > (cpufreq_data->freqvolts - 1))
		idx = 0;
	else
		idx = (cpufreq_data->freqvolts - 1 - idx);

	cpu_dev = cpufreq_data->cpu_dev;
	cpu_clk = cpufreq_data->clk;
	cpu_reg = cpufreq_data->reg;
	cluster = cpufreq_data->cluster;
	freq_Hz = clk_round_rate(cpu_clk, cpufreq_data->freqvolt[idx].freq);
	if (freq_Hz <= 0)
		freq_Hz = cpufreq_data->freqvolt[idx].freq;
	new_freq_hz = freq_Hz;
	old_freq_hz = clk_get_rate(cpu_clk);

	pr_debug("freq idx %u, freq %lu in freqvolt\n",
		idx, cpufreq_data->freqvolt[idx].freq);
	if (cpu_dev) {
		/*for cpu device, get freq&volt from opp*/
		rcu_read_lock();
		opp = dev_pm_opp_find_freq_ceil(cpu_dev, &freq_Hz);
		if (IS_ERR(opp)) {
			rcu_read_unlock();
			PR_ERROR("failed to find OPP for %luKhz\n",
					freq_Hz / 1000);
			return PTR_ERR(opp);
		}
		volt_new = dev_pm_opp_get_voltage(opp);
		opp_freq_hz = dev_pm_opp_get_freq(opp);
		rcu_read_unlock();
	} else {
		/*for sub device, get freq&volt from table*/
		volt_new = cpufreq_data->freqvolt[idx].volt;
		opp_freq_hz = cpufreq_data->freqvolt[idx].freq;
	}
	volt_tol = volt_new * cpufreq_data->volt_tol / 100;

	if (force)
		mutex_lock(cpufreq_data->volt_lock);
	else {
		if (mutex_trylock(cpufreq_data->volt_lock) != 1) {
			pr_info("cannot acquire lock for cluster %d\n",
				cpufreq_data->cluster);
			return -EBUSY;
		}
	}

	/*must get real volt_old in mutex_lock domain*/
	volt_old = regulator_get_voltage(cpu_reg);
	pr_debug("Found OPP: %ld kHz, %ld uV, tolerance: %u\n",
		opp_freq_hz / 1000, volt_new, volt_tol);

	sprd_volt_req_max(cpufreq_data, &volt_new, true, true);
	if (!volt_new) {
		goto EXIT_ERR;
		PR_DEBUG("fail to get volt_new=0\n");
	}

	pr_debug("cluster%u scaling from %lu MHz, %ld mV, --> %lu MHz, %ld mV\n",
		cpufreq_data->cluster,
		old_freq_hz / 1000000,
		(volt_old > 0) ? volt_old / 1000 : -1,
		new_freq_hz / 1000000,
		volt_new ? volt_new / 1000 : -1);

	if (volt_new < volt_old) {
		ret  = sprd_volt_share_slaves_notify(
			cpufreq_data,
			volt_new);
		if (ret)
			goto EXIT_ERR;
	}

	/* scaling up?  scale voltage before frequency */
	if (new_freq_hz > old_freq_hz) {
		pr_debug("scaling voltage to %lu\n", volt_new);
		ret = regulator_set_voltage_tol(cpu_reg, volt_new, volt_tol);
		if (ret) {
			PR_ERROR("failed to scale voltage %lu %u up: %d\n",
				volt_new, volt_tol, ret);
			goto EXIT_ERR;
		}
	}

	ret = sprd_cpufreq_set_clock(cpufreq_data, new_freq_hz);
	if (ret) {
		PR_ERROR("failed to set clock %luMhz rate: %d\n",
					new_freq_hz / 1000000, ret);
		if ((volt_old > 0) && (new_freq_hz > old_freq_hz)) {
			pr_info("scaling to old voltage %lu\n", volt_old);
			regulator_set_voltage_tol(cpu_reg, volt_old, volt_tol);
		}
		goto EXIT_ERR;
	}

	/* scaling down?  scale voltage after frequency */
	if (new_freq_hz < old_freq_hz) {
		ret = regulator_set_voltage_tol(cpu_reg, volt_new, volt_tol);
		if (ret) {
			pr_warn("failed to scale volt %lu %u down: %d\n",
				volt_new, volt_tol, ret);
			ret = sprd_cpufreq_set_clock(cpufreq_data, old_freq_hz);
			goto EXIT_ERR;
		}
	}

	if (volt_new >= volt_old) {
		ret  = sprd_volt_share_slaves_notify(
			cpufreq_data,
			volt_new);
	}
	cpufreq_data->volt_tol_var = volt_tol;
	cpufreq_data->volt_req = volt_new;
	cpufreq_data->freq_req = new_freq_hz;

	pr_debug("cluster%u After transition, new clk rate %luMhz, volt %dmV\n",
		cpufreq_data->cluster,
		clk_get_rate(cpufreq_data->clk) / 1000000,
		regulator_get_voltage(cpu_reg) / 1000);

	mutex_unlock(cpufreq_data->volt_lock);
	return ret;
EXIT_ERR:
	sprd_volt_share_slaves_notify(
		cpufreq_data,
		volt_old);

	mutex_unlock(cpufreq_data->volt_lock);
	return ret;
}
static int sprd_cpufreq_set_target_index(struct cpufreq_policy *policy,
				unsigned int idx)
{
	int ret;

	/*never dvfs until boot_done_timestamp*/
	if (unlikely(boot_done_timestamp &&
		time_after(jiffies, boot_done_timestamp))) {
		sprd_cpufreq_set_boost(0);
		sprd_cpufreq_driver.boost_enabled = false;
		pr_info("sprd_cpufreq_cpufreq: Disables boost because it is time %lu seconds after boot up\n",
			SPRD_CPUFREQ_DRV_BOOST_DURATOIN/HZ);
	}

	/*
	  *boost_mode_flag is true and cpu is on max freq
	  *so return 0 here, reject changing freq
	*/
	if (unlikely(boost_mode_flag)) {
		if (policy->max >= policy->cpuinfo.max_freq) {
			ret = 0;
			goto EXIT;
		} else {
			sprd_cpufreq_set_boost(0);
			sprd_cpufreq_driver.boost_enabled = false;
			pr_info("sprd_cpufreq_cpufreq: Disables boost because IPA maybe decrease policy->max(%d<%d)\n",
				policy->max, policy->cpuinfo.max_freq);
		}
	}

	ret = sprd_cpufreq_set_target(policy->driver_data,
		idx,
		false);
	if (!ret)
		ret = sprd_freq_sync_slaves_notify(policy->driver_data,
			idx,
			false);
EXIT:
	return ret;
}

static int sprd_cpufreq_init_slaves(struct device_node *np_host)
{
	int ret = 0, i;
	struct device_node *np;
	struct sprd_cpufreq_driver_data *c;
	unsigned int cluster;

	if (np_host == NULL)
		return -ENODEV;

	for (i = 0; i < SPRD_CPUFREQ_MAX_MODULE; i++) {
		np = of_parse_phandle(np_host, "cpufreq-sub-clusters", i);
		if (!np) {
			pr_debug("index %d not found sub-clusters\n", i);
			return 0;
		}

		pr_info("slave index %d name is found %s\n",
			i, np->full_name);

		if (of_property_read_u32(np, "cpufreq-cluster-id", &cluster)) {
			pr_err("index %d not found cpufreq_custer_id\n", i);
			ret = -ENODEV;
			goto free_np;
		}

		if (cluster < SPRD_CPUFREQ_MAX_CLUSTER ||
		cluster >= SPRD_CPUFREQ_MAX_MODULE) {
			pr_err("slave index %d custer %u is overflowed\n",
				i, cluster);
			ret = -ENODEV;
			goto free_np;
		}

		if (cpufreq_datas[cluster] == NULL) {
			c = kzalloc(sizeof(*c), GFP_KERNEL);
			if (!c) {
				ret = -ENOMEM;
				goto free_np;
			}
		} else
			c = cpufreq_datas[cluster];

		c->cluster = cluster;
		c->clk = of_clk_get_by_name(np, "clk");
		if (IS_ERR(c->clk)) {
			PR_ERROR("slave index %d failed to get clk, %ld\n", i,
					PTR_ERR(c->clk));
			ret = PTR_ERR(c->clk);
			goto free_mem;
		}
		c->clk_low_freq_p =
			of_clk_get_by_name(np, "clk_low");
		if (IS_ERR(c->clk_low_freq_p)) {
			pr_info("slave index %d clk_low is not defined\n", i);
			ret = PTR_ERR(c->clk_low_freq_p);
			goto free_clk;
		} else {
			c->clk_low_freq_p_max =
				clk_get_rate(c->clk_low_freq_p);
			pr_debug("index %d clk_low_freq_p_max[%lu]Khz\n", i,
				c->clk_low_freq_p_max / 1000);
		}

		of_property_read_u32(np, "clk-high-freq-const",
					&c->clk_high_freq_const);
		c->clk_high_freq_p =
			of_clk_get_by_name(np, "clk_high");
		if (IS_ERR(c->clk_high_freq_p)) {
			pr_info("slave index %d failed in getting clk_high%ld\n",
				i, PTR_ERR(c->clk_high_freq_p));
			ret = PTR_ERR(c->clk_high_freq_p);
			goto free_clk;
		} else {
			if (c->clk_high_freq_const)
				c->clk_high_freq_p_max =
					clk_get_rate(c->clk_high_freq_p);
			pr_debug("index %d clk_high_freq_p_max[%lu]Khz\n", i,
				c->clk_high_freq_p_max / 1000);
		}

		if (!c->clk_en) {
			ret = clk_prepare_enable(c->clk);
			if (ret) {
				pr_info("slave index %d cluster%d clk_en failed\n",
						i, cluster);
					goto free_clk;
			}
			c->clk_en = 1;
		}

		if (of_property_read_u32(np, "voltage-tolerance",
			&c->volt_tol))
			c->volt_tol = DEF_VOLT_TOL;
		of_property_read_u32(np, "volt-share-hosts-bits",
			&c->volt_share_hosts_bits);
		of_property_read_u32(np, "volt-share-masters-bits",
			&c->volt_share_masters_bits);
		of_property_read_u32(np, "volt-share-slaves-bits",
			&c->volt_share_slaves_bits);
		of_property_read_u32(np, "freq-sync-hosts-bits",
			&c->freq_sync_hosts_bits);
		of_property_read_u32(np, "freq-sync-slaves-bits",
			&c->freq_sync_slaves_bits);

		c->volt_lock = sprd_volt_share_lock(c);
		if (c->volt_lock == NULL) {
			pr_info("slave index %d can find host volt_lock!\n", i);
			ret = -ENOMEM;
			goto free_clk;
		}

		c->reg = sprd_volt_share_reg(c);
		if (c->reg == NULL ||
		IS_ERR(c->reg)) {
			PR_ERROR("failed to get regulator, %ld\n",
				PTR_ERR(c->reg));
			ret = -ENODEV;
			goto free_clk;
		}

		ret = dev_pm_opp_of_add_table_binning(cluster,
			NULL,
			np,
			c);
		if (ret)
			goto free_clk;

		of_node_put(np);

		c->online = 1;
		cpufreq_datas[cluster] = c;


		pr_info("slave index %d cpu_dev=%p\n",
			i, c->cpu_dev);
		pr_info("slave index %d cpufreq_custer_id=%u\n",
			i, c->cluster);
		pr_info("slave index %d online=%u\n",
			i, c->online);
		pr_info("slave index %d volt_lock=%p\n",
			i, c->volt_lock);
		pr_info("slave index %d reg=%p\n",
			i, c->reg);
		pr_info("slave index %d clk_high_freq_const=%u\n",
			i, c->clk_high_freq_const);
		pr_info("slave index %d clk=%p\n",
			i, c->clk);
		pr_info("slave index %d clk_low_freq_p=%p\n",
			i, c->clk_low_freq_p);
		pr_info("slave index %d clk_high_freq_p=%p\n",
			i, c->clk_high_freq_p);
		pr_info("slave index %d clk_low_freq_p_max=%lu\n",
			i, c->clk_low_freq_p_max);
		pr_info("slave index %d clk_high_freq_p_max=%lu\n",
			i, c->clk_high_freq_p_max);
		pr_info("slave index %d volt_tol=%u\n",
			i, c->volt_tol);
		pr_info("slave index %d volt_req=%lu\n",
			i, c->volt_req);
		pr_info("slave index %d volt_share_hosts_bits=%u\n",
			i, c->volt_share_hosts_bits);
		pr_info("slave index %d volt_share_masters_bits=%u\n",
			i, c->volt_share_masters_bits);
		pr_info("slave index %d volt_share_slaves_bits=%u\n",
			i, c->volt_share_slaves_bits);
		pr_info("slave index %d freq_sync_hosts_bits=%u\n",
			i, c->freq_sync_hosts_bits);
		pr_info("slave index %d freq_sync_slaves_bits=%u\n",
			i, c->freq_sync_slaves_bits);
	}

	return ret;
free_clk:
	if (!IS_ERR(c->clk)) {
		clk_put(c->clk);
		c->clk = ERR_PTR(-ENOENT);
	}
	if (!IS_ERR(c->clk_low_freq_p))
		clk_put(c->clk_low_freq_p);
	if (!IS_ERR(c->clk_high_freq_p))

		clk_put(c->clk_high_freq_p);
free_mem:
	kfree(c);
	cpufreq_datas[cluster] = NULL;
free_np:
	if (np)
		of_node_put(np);
	return ret;
}

static int sprd_cpufreq_init(struct cpufreq_policy *policy)
{
	struct dev_pm_opp *opp;
	unsigned long freq_Hz = 0;
	int ret = 0;
	struct device *cpu_dev;
	struct regulator *cpu_reg = NULL;
	struct device_node *cpu_np;
	struct device_node *np, *np1;
	struct clk *cpu_clk;
	struct sprd_cpufreq_driver_data *cpufreq_data;
	unsigned int volt_tol = 0;
	unsigned int transition_latency = CPUFREQ_ETERNAL;
	unsigned long clk_low_freq_p_max = 0;
	struct cpufreq_frequency_table *freq_table = NULL;
	int cpu = 0;

	if (!policy) {
		PR_ERROR("invalid cpufreq_policy\n");
		return -ENODEV;
	}

	if (topology_physical_package_id(policy->cpu)
	>= SPRD_CPUFREQ_MAX_CLUSTER) {
		PR_ERROR("cpu%d in invalid cluster %d\n", policy->cpu,
			topology_physical_package_id(policy->cpu));
		return -ENODEV;
	}

	cpu = policy->cpu;
	PR_DEBUG("going to get cpu%d device\n", cpu);
	cpu_dev = get_cpu_device(cpu);
	if (IS_ERR(cpu_dev)) {
		PR_ERROR("failed to get cpu%d device\n", cpu);
		return -ENODEV;
	}

	cpu_np = of_node_get(cpu_dev->of_node);
	if (!cpu_np) {
		PR_ERROR("failed to find cpu%d node\n", cpu);
		return -ENOENT;
	}

	np = of_parse_phandle(cpu_np, "cpufreq-data", 0);
	np1 = of_parse_phandle(cpu_np, "cpufreq-data-v1", 0);
	if (!np && !np1) {
		PR_ERROR("failed to find cpufreq-data for cpu%d\n", cpu);
		of_node_put(cpu_np);
		return -ENOENT;
	}
	if (np1)
		np = np1;

	if (sprd_cpufreq_data(cpu))
		cpufreq_data = sprd_cpufreq_data(cpu);
	else {
		cpufreq_data = kzalloc(sizeof(*cpufreq_data), GFP_KERNEL);
		if (!cpufreq_data) {
			ret = -ENOMEM;
			goto free_np;
		}
	}

	of_property_read_u32(np, "volt-share-masters-bits",
		&cpufreq_data->volt_share_masters_bits);
	of_property_read_u32(np, "volt-share-slaves-bits",
		&cpufreq_data->volt_share_slaves_bits);
	of_property_read_u32(np, "freq-sync-hosts-bits",
		&cpufreq_data->freq_sync_hosts_bits);
	of_property_read_u32(np, "freq-sync-slaves-bits",
		&cpufreq_data->freq_sync_slaves_bits);
	pr_debug("read dts, masters_bits %u, slaves_bits %u\n",
		cpufreq_data->volt_share_masters_bits,
		cpufreq_data->volt_share_slaves_bits);

	if (!cpufreq_data->volt_lock) {
		cpufreq_data->volt_lock = sprd_volt_share_lock(cpufreq_data);
		if (!cpufreq_data->volt_lock) {
			cpufreq_data->volt_lock =
				kzalloc(sizeof(struct mutex), GFP_KERNEL);
			if (!cpufreq_data->volt_lock) {
				ret = -ENOMEM;
				goto free_mem;
			}
			mutex_init(cpufreq_data->volt_lock);
		}
	}

	mutex_lock(cpufreq_data->volt_lock);

	cpufreq_data->cluster = topology_physical_package_id(cpu);

	cpu_clk = of_clk_get_by_name(np, CORE_CLK);
	if (IS_ERR(cpu_clk)) {
		PR_ERROR("failed to get cpu clock, %ld\n",
				PTR_ERR(cpu_clk));
		ret = PTR_ERR(cpu_clk);
		goto free_mem;
	}

	cpufreq_data->clk_low_freq_p =
		of_clk_get_by_name(np, LOW_FREQ_CLK_PARENT);
	if (IS_ERR(cpufreq_data->clk_low_freq_p)) {
		pr_info("clk_low_freq_p is not defined\n");
		clk_low_freq_p_max = 0;
	} else {
		clk_low_freq_p_max =
			clk_get_rate(cpufreq_data->clk_low_freq_p);
		pr_debug("clk_low_freq_p_max[%lu]Khz\n",
			clk_low_freq_p_max / 1000);
	}

	cpufreq_data->clk_high_freq_p =
		of_clk_get_by_name(np, HIGH_FREQ_CLK_PARENT);
	if (IS_ERR(cpufreq_data->clk_high_freq_p)) {
		PR_ERROR("failed in getting clk_high_freq_p %ld\n",
			PTR_ERR(cpufreq_data->clk_high_freq_p));
		ret = PTR_ERR(cpufreq_data->clk_high_freq_p);
		goto free_clk;
	}

	if (of_property_read_u32(np, "clock-latency", &transition_latency))
		transition_latency = CPUFREQ_ETERNAL;
	if (of_property_read_u32(np, "voltage-tolerance", &volt_tol))
		volt_tol = DEF_VOLT_TOL;
	pr_debug("value of transition_latency %u, voltage_tolerance %u\n",
			transition_latency, volt_tol);

	cpu_reg = sprd_volt_share_reg(cpufreq_data);
	if (cpu_reg == NULL)
		cpu_reg = devm_regulator_get(cpu_dev, CORE_SUPPLY);
	if (cpu_reg == NULL || IS_ERR(cpu_reg)) {
		PR_ERROR("failed to get regulator, %ld\n", PTR_ERR(cpu_reg));
		ret = PTR_ERR(cpu_reg);
		goto free_clk;
	}

	/*TODO: need to get new temperature from thermal zone after hotplug*/
	if (cpufreq_datas[0] != NULL)
		cpufreq_data->temp_now = cpufreq_datas[0]->temp_now;
	ret = dev_pm_opp_of_add_table_binning(is_big_cluster(cpu),
		cpu_dev, np, cpufreq_data);

	if (ret < 0) {
		PR_ERROR("failed to init opp table, %d\n", ret);
		goto free_reg;
	}

	if (!IS_ERR(cpu_reg)) {
		pr_debug("going to verify opp with regulator\n");
		ret = sprd_verify_opp_with_regulator(cpu_dev,
				cpu_reg, volt_tol);
		if (ret > 0)
			transition_latency += ret * 1000;
	}

	pr_debug("going to initialize freq_table\n");
	ret = dev_pm_opp_init_cpufreq_table(cpu_dev, &freq_table);
	if (ret) {
		PR_ERROR("%d in initializing freq_table\n", ret);
		goto free_opp;
	}

	ret = cpufreq_table_validate_and_show(policy, freq_table);
	if (ret) {
		PR_ERROR("invalid frequency table: %d\n", ret);
		goto free_table;
	}
	pr_debug("going to prepare clock\n");

	if (!cpufreq_data->clk_en) {
		if (clk_prepare_enable(cpu_clk)) {
			PR_ERROR("CPU%d clk_prepare_enable failed\n",
				policy->cpu);
				goto free_table;
		}
		cpufreq_data->clk_en = 1;
	}
	#ifdef CONFIG_SMP
	/* CPUs in the same cluster share a clock and power domain. */
	cpumask_or(policy->cpus, policy->cpus, cpu_coregroup_mask(policy->cpu));
	#endif

	cpufreq_data->online = 1;
	cpufreq_data->cpu_dev = cpu_dev;
	cpufreq_data->reg = cpu_reg;
	cpufreq_data->clk = cpu_clk;
	cpufreq_data->volt_tol = volt_tol;
	cpufreq_data->clk_low_freq_p_max = clk_low_freq_p_max;

	if (cpufreq_data->cluster < SPRD_CPUFREQ_MAX_CLUSTER &&
	cpufreq_datas[cpufreq_data->cluster] == NULL) {
		cpufreq_datas[cpufreq_data->cluster] = cpufreq_data;
		PR_DEBUG("cpu%d got new cpufreq_data\n", cpu);
	}

	if (sprd_cpufreq_init_slaves(np))
		goto free_table;

	mutex_unlock(cpufreq_data->volt_lock);

	policy->driver_data = cpufreq_data;
	policy->clk = cpu_clk;
	policy->suspend_freq = freq_table[0].frequency;
	policy->cur = clk_get_rate(cpu_clk) / 1000;
	policy->cpuinfo.transition_latency = transition_latency;
	freq_Hz = policy->cur*1000;
	rcu_read_lock();
	opp = dev_pm_opp_find_freq_ceil(cpu_dev, &freq_Hz);
	cpufreq_data->volt_req = dev_pm_opp_get_voltage(opp);
	cpufreq_data->freq_req = dev_pm_opp_get_freq(opp);
	rcu_read_unlock();

	if (freq_Hz != policy->cur*1000) {
		sprd_cpufreq_set_target(cpufreq_data, 0, true);
	}

	pr_info("init cpu%d is ok, freq=%ld, freq_req=%ld, volt_req=%ld\n",
		cpu, freq_Hz, cpufreq_data->freq_req, cpufreq_data->volt_req);

	goto free_np;

free_table:
	if (policy->freq_table != NULL)
		dev_pm_opp_free_cpufreq_table(cpu_dev,
			&policy->freq_table);
free_opp:
	dev_pm_opp_of_remove_table(cpu_dev);
free_reg:
	if (!IS_ERR(cpu_reg))
		devm_regulator_put(cpu_reg);
free_clk:
	if (!IS_ERR(cpu_clk)) {
		clk_put(cpu_clk);
		policy->clk = ERR_PTR(-ENOENT);
	}
	if (!IS_ERR(cpufreq_data->clk_low_freq_p))
		clk_put(cpufreq_data->clk_low_freq_p);
	if (!IS_ERR(cpufreq_data->clk_high_freq_p))
		clk_put(cpufreq_data->clk_high_freq_p);
free_mem:
	if (cpufreq_data->volt_lock) {
		mutex_unlock(cpufreq_data->volt_lock);
		mutex_destroy(cpufreq_data->volt_lock);
		kfree(cpufreq_data->volt_lock);
	}
	kfree(cpufreq_data);
	sprd_cpufreq_data(cpu) = NULL;
free_np:
	if (np)
		of_node_put(np);
	if (np1)
		of_node_put(np1);
	if (cpu_np)
		of_node_put(cpu_np);
	return ret;
}

static int sprd_cpufreq_exit(struct cpufreq_policy *policy)
{
	struct sprd_cpufreq_driver_data *cpufreq_data;

	if (!policy)
		return -ENODEV;

	pr_info("releasing resources for cpu %d\n", policy->cpu);
	cpufreq_data = policy->driver_data;
	if (cpufreq_data == NULL)
		return 0;

	mutex_lock(cpufreq_data->volt_lock);

	if (policy->freq_table != NULL)
		dev_pm_opp_free_cpufreq_table(cpufreq_data->cpu_dev,
			&policy->freq_table);

	if (!IS_ERR(policy->clk)) {
		clk_put(policy->clk);
		policy->clk = ERR_PTR(-ENOENT);
		cpufreq_data->clk = NULL;
	}
	if (!IS_ERR(cpufreq_data->clk_low_freq_p)) {
		clk_put(cpufreq_data->clk_low_freq_p);
		cpufreq_data->clk_low_freq_p = NULL;
	}
	if (!IS_ERR(cpufreq_data->clk_high_freq_p)) {
		clk_put(cpufreq_data->clk_high_freq_p);
		cpufreq_data->clk_high_freq_p = NULL;
	}

	policy->driver_data = NULL;

	mutex_unlock(cpufreq_data->volt_lock);
	return 0;
}

static int sprd_cpufreq_table_verify(struct cpufreq_policy *policy)
{
	return cpufreq_generic_frequency_table_verify(policy);
}


static unsigned int sprd_cpufreq_get(unsigned int cpu)
{
	return cpufreq_generic_get(cpu);
}

static int sprd_cpufreq_suspend(struct cpufreq_policy *policy)
{
	if (policy && strcmp(policy->governor->name, "interactive")) {
		pr_debug("%s: do nothing for governor->name %s\n",
			__func__, policy->governor->name);
		return 0;
	}

	return cpufreq_generic_suspend(policy);
}

static int sprd_cpufreq_resume(struct cpufreq_policy *policy)
{
	if (policy && strcmp(policy->governor->name, "interactive")) {
		pr_debug("%s: do nothing for governor->name %s\n",
			__func__, policy->governor->name);
		return 0;
	}

	return cpufreq_generic_suspend(policy);
}

/**
 * sprd_cpufreq_set_boost: set cpufreq driver to be boost mode.
 *
 * @state: 0->disable boost mode;1->enable boost mode;
 *
 * Return: zero on success, otherwise non-zero on failure.
 */
static int sprd_cpufreq_set_boost(int state)
{
	/*
	  *if (boost_mode_flag != state && state) {
	  *TODO: if cpufreq.c set boost mode,
	  *dvfs driver should set max freq on all CPUs here
	  *}
	  */

	/*
	  *cpufreq.c caller takes control of cpufreq driver boost
	  *disalbe boot_done_timestamp function at once
	  */
	boot_done_timestamp = 0;
	boost_mode_flag = state;
	pr_info("sprd_cpufreq_set_boost=%d\n", boost_mode_flag);

	return 0;
}

static struct cpufreq_driver sprd_cpufreq_driver = {
	.name = "sprd-cpufreq",
	.flags = CPUFREQ_STICKY
			| CPUFREQ_NEED_INITIAL_FREQ_CHECK
			| CPUFREQ_HAVE_GOVERNOR_PER_POLICY,
	.init = sprd_cpufreq_init,
	.exit = sprd_cpufreq_exit,
	.verify = sprd_cpufreq_table_verify,
	.target_index = sprd_cpufreq_set_target_index,
	.get = sprd_cpufreq_get,
	.suspend = sprd_cpufreq_suspend,
	.resume = sprd_cpufreq_resume,
	.attr = cpufreq_generic_attr,
	/* platform specific boost support code */
	.boost_supported = true,
	.boost_enabled = true,
	.set_boost = sprd_cpufreq_set_boost,
};
static int sprd_cpufreq_cpu_callback(struct notifier_block *nfb,
	unsigned long action, void *hcpu)
{
	unsigned int cpu = (unsigned long)hcpu;
	unsigned int olcpu = 0;
	unsigned int volt_tol_min = 0, index = 0;
	unsigned long volt_max_online = 0;
	struct sprd_cpufreq_driver_data *c;

	c = sprd_cpufreq_data(cpu);

	if (c == NULL || (c && c->volt_share_masters_bits == 0))
		return NOTIFY_OK;
	if (!is_big_cluster(cpu))
		return NOTIFY_OK;

	/* Big is online then dont proceed */
	for_each_online_cpu(olcpu)
		if (is_big_cluster(olcpu))
			return NOTIFY_OK;

	switch (action & ~CPU_TASKS_FROZEN) {
	case CPU_UP_PREPARE:
		pr_info("BIG CLUSTER ON: vol to old_vol of cpu %u\n", cpu);

		mutex_lock(c->volt_lock);
		volt_tol_min = sprd_volt_tol_min(c, true);
		sprd_volt_req_max(c, &volt_max_online, true, true);

		if (c->volt_req > volt_max_online) {
			if (regulator_set_voltage_tol(c->reg,
				c->volt_req,
				volt_tol_min))
				PR_ERROR("fail to set voltage %lu\n",
					c->volt_req);
			else
				sprd_volt_share_slaves_notify(
					sprd_cpufreq_data(cpu),
					c->volt_req);
		}
		c->online = 1;
		mutex_unlock(c->volt_lock);

		pr_debug("volt_req %lu, volt_max_online %lu, new volt=%lu\n",
			c->volt_req, volt_max_online,
			c->volt_req > volt_max_online ?
			c->volt_req : volt_max_online);
		/*notify slaves recover freq*/
		for (index = 0; index < SPRD_CPUFREQ_MAX_FREQ_VOLT; index++)
			if (c->volt_req >= c->freqvolt[index].volt)
				break;
		sprd_freq_sync_slaves_notify(c, index, true);

		break;
	case CPU_UP_CANCELED:
	case CPU_POST_DEAD:
		pr_info("BIG CLUSTER OFF: vol to 0 of cpu %u\n", cpu);
		mutex_lock(c->volt_lock);
		c->online = 0;
		mutex_unlock(c->volt_lock);
		/*notify slaves set min freq*/
		sprd_freq_sync_slaves_notify(c, 0, true);
		break;
	}
	return NOTIFY_OK;
}

static struct notifier_block __refdata sprd_cpufreq_cpu_notifier = {
	.notifier_call = sprd_cpufreq_cpu_callback,
};

static int  sprd_cpufreq_probe(struct platform_device *pdev)
{
	struct device *cpu_dev = NULL;
	struct clk *cpu_clk = NULL;
	struct regulator *cpu_reg = NULL;
	struct device_node *cpu_np;
	struct device_node *np = NULL, *np1 = NULL;
	int ret = 0, i;
	/* doing probe only for cpu 0 */
	int cpu = 0;

	boot_done_timestamp = jiffies + SPRD_CPUFREQ_DRV_BOOST_DURATOIN;

	PR_DEBUG("cpufreq probe begins\n");
	cpu_dev = get_cpu_device(cpu);
	if (!cpu_dev) {
		PR_ERROR("failed to get cpu%d device\n", cpu);
		return -ENODEV;
	}

	cpu_np = of_node_get(cpu_dev->of_node);
	if (!cpu_np) {
		PR_ERROR("failed to find cpu node\n");
		return -ENODEV;
	}

	np = of_parse_phandle(cpu_np, "cpufreq-data", 0);
	np1 = of_parse_phandle(cpu_np, "cpufreq-data-v1", 0);
	if (!np && !np1) {
		PR_ERROR("failed to find cpufreq-data for cpu%d\n", cpu);
		of_node_put(cpu_np);
		return -ENOENT;
	}
	if (np1)
		np = np1;

	cpu_clk = of_clk_get_by_name(np, CORE_CLK);
	if (IS_ERR(cpu_clk)) {
		PR_ERROR("failed in clock getting, %ld\n",
				PTR_ERR(cpu_clk));
		ret = PTR_ERR(cpu_clk);
		goto put_np;
	}

	cpu_reg = devm_regulator_get(cpu_dev, CORE_SUPPLY);
	if (IS_ERR(cpu_reg)) {
		PR_ERROR("failed  in regulator getting, %ld\n",
			PTR_ERR(cpu_reg));
		ret = PTR_ERR(cpu_reg);
		goto put_clk;
	}

/* Put regulator and clock here, before registering the driver
 * we will get them again while per cpu initialization in cpufreq_init
*/
	if (!IS_ERR(cpu_reg)) {
		PR_DEBUG("putting regulator\n");
		devm_regulator_put(cpu_reg);
	}

	for (i = 0; i < SPRD_CPUFREQ_MAX_MODULE; i++)
		cpufreq_datas[i] = NULL;

put_clk:
	if (!IS_ERR(cpu_clk)) {
		PR_DEBUG("putting clk\n");
		clk_put(cpu_clk);
	}

put_np:
	if (np)
		of_node_put(np);
	if (np1)
		of_node_put(np1);
	if (cpu_np)
		of_node_put(cpu_np);

	/* ret is not zero? we encountered an error.
	 * return failure/probe deferred
	*/
	if (ret)
		return ret;

	pr_info("going to register cpufreq driver\n");
	ret = cpufreq_register_driver(&sprd_cpufreq_driver);
	if (ret)
		pr_info("cpufreq driver register failed %d\n", ret);
	else
		pr_info("cpufreq driver register succcess\n");
	register_hotcpu_notifier(&sprd_cpufreq_cpu_notifier);

	return ret;
}

static int sprd_cpufreq_remove(struct platform_device *pdev)
{
	return cpufreq_unregister_driver(&sprd_cpufreq_driver);
}

static struct platform_driver sprd_cpufreq_platdrv = {
	.driver = {
		.name	= "sprd-cpufreq",
		.owner	= THIS_MODULE,
	},
	.probe		= sprd_cpufreq_probe,
	.remove		= sprd_cpufreq_remove,
};

module_platform_driver(sprd_cpufreq_platdrv);

/* If required, we can move device registration code in other file */
static struct platform_device sprd_cpufreq_pdev = {
	.name = "sprd-cpufreq",
};

static int  __init sprd_cpufreq_init_pdev(void)
{
	return platform_device_register(&sprd_cpufreq_pdev);
}

device_initcall(sprd_cpufreq_init_pdev);

MODULE_DESCRIPTION("sprd cpufreq driver");
MODULE_LICENSE("GPL");
