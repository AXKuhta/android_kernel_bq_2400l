
#ifndef __SPRD_NAND_PARAM_H__
#define __SPRD_NAND_PARAM_H__

#define NAND_MAX_ID_LEN 5

#define BW_08 0x00
#define BW_16 0x01

#define CYCLES_3 0x03
#define CYCLES_4 0x04
#define CYCLES_5 0x05

#define NUM_BLOCK(n) (n)

#define SZ_K_BLOCK(n) (n * 1024)
#define SZ_K_PAGE(n) (n * 1024)

#define SZ_B_BLOCK(n) (n)
#define SZ_B_PAGE(n) (n)

#define SZ_B_SPARE(n) (n)
#define SZ_B_OOB(n) (n)
#define SZ_B_SECTOR(n) (n)

#define ECC_BITS(n) (n)

#define SZ_ECC(n) (n)
#define POS_ECC(n) (n)
#define SZ_INFO(n) (n)
#define POS_INFO(n) (n)

#define CALC_ECC_SIZE(n) ((14 * (n) + 7) / 8)


struct sprd_nand_device {
	uint8_t id_device;
	char* p_name;
	char* p_type;
};

struct sprd_nand_maker {
	uint8_t idmaker;
	char* p_name;
	struct sprd_nand_device* p_devtab;
};

struct sprd_nand_oob {
	uint16_t oob_size; //16
	uint8_t ecc_bits;
	uint16_t ecc_pos;
	uint16_t ecc_size;  //4
	uint16_t info_pos;
	uint16_t info_size;
};

struct sprd_nand_timing {
	uint8_t	ace_ns;	/* ALE, CLE end of delay timing, unit: ns */
	uint8_t	rwl_ns;	/* WE, RE, IO, pulse  timing, unit: ns */
	uint8_t	rwh_ns;	/* WE, RE, IO, high hold  timing, unit: ns */
};

struct sprd_nand_vendor_param {
	uint8_t id[NAND_MAX_ID_LEN];
	uint8_t idmaker;
	uint8_t id_device;
	uint32_t blk_size; //256K
	uint32_t blk_num; //
	uint32_t page_size; //4K
	uint16_t nsect_size; //512
	uint16_t nspare_size;
	uint8_t nbus_width;
	uint8_t ncycles;
	struct sprd_nand_timing s_timing;
	struct sprd_nand_oob s_oob;
};

struct sprd_nand_device adata_device_table[] = {
	{0xBC,"APZZLA4GAZ-N22MAB","2Gb,x08,1.8V"},
	{0x00,NULL,NULL}
};

struct sprd_nand_device fidelix_device_table[] = {
	{0xAC,"FMN4ET2TCM","4Gb,x08,1.8V"},
	{0xBC,"NCSPN4N2A","2Gb,x08,1.8V"},
	{0x00,NULL,NULL}
};

struct sprd_nand_device kington_device_table[] = {
	{0xBC,"KSLCCBL2GA2H2A","2Gb,x08,1.8V"},
	{0xAC,"NM1482KSLAXCL","4Gb,x08,1.8V"},
	{0xBA,"Kingtonxxxxx","1Gb,x08,1.8V"},
	{0x00,NULL,NULL}
};

struct sprd_nand_device micron_device_table[] = {
	{0xAC,"MT29RZ4B2DZZHGSK","4Gb,x08,1.8V"},
	{0xAC,"MT29RZ4B2DZZHHWD","4Gb,x08,1.8V"},
	{0xBC,"MT29F4G16ABBEA","2Gb,x08,1.8V"},
	{0x00,NULL,NULL}
};

struct sprd_nand_device jsc_device_table[] = {
	{0xBC,"JS27AP4G15SD ","2Gb,x08,1.8V"},
	{0x00,NULL,NULL}
};

struct sprd_nand_device hynix_device_table[] = {
	{0xAC,"JSFCBF3YHHBBL","4Gb,x08,1.8V"},
	{0xBC,"JSFCBF3YHHBBL","2Gb,x08,1.8V"},
	{0x00,NULL,NULL}
};

struct sprd_nand_maker maker_table[] = {
	{0x01,"Fidelix",fidelix_device_table},
	{0x2C,"Micron",micron_device_table},
	{0xC8,"Adata",adata_device_table},
	{0xEC,"Jsc",jsc_device_table},
	{0x98,"Kington",kington_device_table},
	{0xAD,"Hynix",hynix_device_table},
	{0x00,NULL,NULL}
};

struct sprd_nand_vendor_param sprd_nand_vendor_param_table[] = {
	{//Micron
		{0x2C,0xBC,0x90,0x66,0x54},0x2C,0xBC, 
		SZ_K_BLOCK(128),NUM_BLOCK(2048), SZ_K_PAGE(2),SZ_B_SECTOR(512),SZ_B_SPARE(112),BW_08,CYCLES_5,
		{10,15,10},
		{SZ_B_OOB(28),ECC_BITS(8),POS_ECC(14),SZ_ECC(14),POS_INFO(14),SZ_INFO(0)}
	},
	{//Kington
		{0x98,0xBC,0x90,0x66,0x76},0x98,0xBC, 
		SZ_K_BLOCK(128),NUM_BLOCK(2048), SZ_K_PAGE(2),SZ_B_SECTOR(512),SZ_B_SPARE(112),BW_08,CYCLES_5,
		{10,20,20},
		{SZ_B_OOB(28),ECC_BITS(8),POS_ECC(14),SZ_ECC(14),POS_INFO(14),SZ_INFO(0)}
	},
	{//Hynix
		{0xAD,0xBC,0x90,0x55,0x56},0xAD,0xBC, 
		SZ_K_BLOCK(64),NUM_BLOCK(4096), SZ_K_PAGE(1),SZ_B_SECTOR(512),SZ_B_SPARE(32),BW_08,CYCLES_5,
		{10,20,20},
		{SZ_B_OOB(16),ECC_BITS(4),POS_ECC(9),SZ_ECC(7),POS_INFO(9),SZ_INFO(0)}
	},
	{//Adata
		{0xC8,0xBC,0x90,0x55,0x54},0xC8,0xBC, 
		SZ_K_BLOCK(64),NUM_BLOCK(4096), SZ_K_PAGE(1),SZ_B_SECTOR(512),SZ_B_SPARE(32),BW_08,CYCLES_5,
		{10,20,20},
		{SZ_B_OOB(16),ECC_BITS(4),POS_ECC(9),SZ_ECC(7),POS_INFO(9),SZ_INFO(0)}
	},
	{//Kington
		{0x98,0xBA,0x90,0x55,0x76},0x98,0xBA, 
		SZ_K_BLOCK(64),NUM_BLOCK(2048), SZ_K_PAGE(1),SZ_B_SECTOR(512),SZ_B_SPARE(64),BW_08,CYCLES_5,
		{10,20,20},
		{SZ_B_OOB(32),ECC_BITS(8),POS_ECC(18),SZ_ECC(14),POS_INFO(18),SZ_INFO(0)}
	},
	{//Jsc
		{0xEC,0xBC,0x00,0x6A,0x56},0xEC,0xBC, 
		SZ_K_BLOCK(128),NUM_BLOCK(2048), SZ_K_PAGE(2),SZ_B_SECTOR(512),SZ_B_SPARE(128),BW_08,CYCLES_5,
		{10,20,20},
		{SZ_B_OOB(32),ECC_BITS(8),POS_ECC(18),SZ_ECC(14),POS_INFO(18),SZ_INFO(0)}
	},
	{//Fidelix
		{0x01,0xBC,0x90,0x55,0x56},0x01,0xBC, 
		SZ_K_BLOCK(64),NUM_BLOCK(4096), SZ_K_PAGE(1),SZ_B_SECTOR(512),SZ_B_SPARE(32),BW_08,CYCLES_5,
		{10,20,20},
		{SZ_B_OOB(16),ECC_BITS(4),POS_ECC(9),SZ_ECC(7),POS_INFO(9),SZ_INFO(0)}
	},
	{//Kington
		{0x98,0xAC,0x90,0x26,0x76},0x98,0xAC, 
		SZ_K_BLOCK(256),NUM_BLOCK(2048), SZ_K_PAGE(4),SZ_B_SECTOR(512),SZ_B_SPARE(256),BW_08,CYCLES_5,
		{10,20,20},
		{SZ_B_OOB(32),ECC_BITS(8),POS_ECC(18),SZ_ECC(14),POS_INFO(18),SZ_INFO(0)}
	},
	{//Fidelix
		{0x01,0xAC,0x90,0x15,0x56},0x01,0xAC, 
		SZ_K_BLOCK(128),NUM_BLOCK(4096), SZ_K_PAGE(2),SZ_B_SECTOR(512),SZ_B_SPARE(64),BW_08,CYCLES_5,
		{10,20,20},
		{SZ_B_OOB(16),ECC_BITS(4),POS_ECC(9),SZ_ECC(7),POS_INFO(9),SZ_INFO(0)}
	},
	{//Hynix
		{0xAD,0xAC,0x90,0x15,0x56},0xAD,0xAC, 
		SZ_K_BLOCK(128),NUM_BLOCK(4096), SZ_K_PAGE(2),SZ_B_SECTOR(512),SZ_B_SPARE(64),BW_08,CYCLES_5,
		{10,20,20},
		{SZ_B_OOB(16),ECC_BITS(4),POS_ECC(9),SZ_ECC(7),POS_INFO(9),SZ_INFO(0)}
	},
	{//Micron
		{0x2C,0xAC,0x90,0x15,0x56},0x2C,0xAC, 
		SZ_K_BLOCK(128),NUM_BLOCK(4096), SZ_K_PAGE(2),SZ_B_SECTOR(512),SZ_B_SPARE(64),BW_08,CYCLES_5,
		{10,20,20},
		{SZ_B_OOB(16),ECC_BITS(4),POS_ECC(9),SZ_ECC(7),POS_INFO(9),SZ_INFO(0)}
	},
	{//Micron
		{0x2C,0xAC,0x90,0x26,0x54},0x2C,0xAC, 
		SZ_K_BLOCK(256),NUM_BLOCK(2048), SZ_K_PAGE(4),SZ_B_SECTOR(512),SZ_B_SPARE(224),BW_08,CYCLES_5,
		{10,15,10},
		{SZ_B_OOB(28),ECC_BITS(8),POS_ECC(14),SZ_ECC(14),POS_INFO(14),SZ_INFO(0)}
	},
	{
		{0x00, 0x00, 0x00, 0x00, 0x00},0x00,0x00, 
		SZ_K_BLOCK(256),NUM_BLOCK(2048), SZ_K_PAGE(4),SZ_B_SECTOR(512),SZ_B_SPARE(128),0x01,0x05,
		{10,21,15},
		{SZ_B_OOB(16),ECC_BITS(8),POS_ECC(2),SZ_ECC(14),POS_INFO(0),SZ_INFO(0)}
	}
};


#endif
