#ifndef __SPRD_DFS_DRV_H__
#define __SPRD_DFS_DRV_H__

extern int dfs_inquire(unsigned int *data, unsigned int index, unsigned int event);
extern int dfs_set_para(unsigned int para, unsigned int index, unsigned int event);
extern int dfs_disable(unsigned int cmd);
extern int dfs_enable(unsigned int cmd);
extern int inq_axi_busmonitor(unsigned int channel);
extern int set_axi_busmonitor(unsigned int channel, unsigned int value);
extern int scene_dfs_request(char *scenario);
extern void scene_exit(char *scenario);
extern void dfs_scene_request(bool active, int id, unsigned int freq);
extern struct list_head scene_dfs_list;
extern int dfs_scene_count;

enum dfs_event {
	INQ_DDR_FREQ		= 0x00,
	INQ_CP_FREQ		= 0x01,
	INQ_STATUS		= 0x02,
	INQ_AUTO_STATUS		= 0x03,
	INQ_DDR_TABLE		= 0x04,
	INQ_OVERFLOW		= 0x05,
	INQ_UNDERFLOW		= 0x06,
	INQ_TIMER		= 0x07,
	INQ_COUNT		= 0x08,
	INQ_AXI_STATUS          = 0x09,
	INQ_AXI_WLTC            = 0x0a,
	INQ_AXI_RLTC            = 0x0b,
	SET_FREQ		= 0x11,
	SET_OVERFLOW		= 0x12,
	SET_UNDERFLOW		= 0x13,
	SET_TIMER		= 0x14,
	SET_ENABLE		= 0x15,
	SET_DISABLE		= 0x16,
	SET_AUTO_ENABLE		= 0x17,
	SET_AUTO_DISABLE	= 0x18,
	SET_AXI_ENABLE          = 0x19,
	SET_AXI_DISABLE         = 0x1a,
	SET_AXI_WLTC            = 0x1b,
	SET_AXI_RLTC            = 0x1c,
	SET_DEBUG		= 0x1f
};

struct dfs_scene {
	int scene_id;
	unsigned int scene_freq;
	bool active;
	int active_count;
	struct list_head scene_list;
};

#endif
