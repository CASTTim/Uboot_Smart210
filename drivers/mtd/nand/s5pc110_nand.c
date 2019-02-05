/*
 * (C) Copyright 2006 OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

#include <nand.h>
#include <asm/arch/cpu.h>
#include <asm/arch/nand_reg.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>


#define S5PC110_NFCONF_MSG_LEN_512  	(0<<15)
#define S5PC110_NFCONF_ECCTYPE_1BIT 	(0<<23)
#define S5PC110_NFCONF_TACLS(x)     	((x)<<12)
#define S5PC110_NFCONF_TWRPH0(x)    	((x)<<8)
#define S5PC110_NFCONF_TWRPH1(x)    	((x)<<4)
#define S5PC110_NFCONF_SLC_FLASH 		(0<<3)
#define S5PC110_NFCONF_PAGE_SIZE_2K		(0<<2)
#define S5PC110_NFCONF_ADDR_CYCLE_5		(1<<1)

#define S5PC110_NFCONT_nCE3				(1<<23)
#define S5PC110_NFCONT_nCE2				(1<<22)
#define S5PC110_NFCONT_MLCEccDirection	(1<<18)
#define S5PC110_NFCONT_LockTight		(1<<17)
#define S5PC110_NFCONT_LOCK				(1<<16)
#define S5PC110_NFCONT_EnbMLCEncInt		(1<<13)
#define S5PC110_NFCONT_EnbMLCDecInt		(1<<12)
#define S5PC110_NFCONT_EnbIllegalAccINT	(1<<10)
#define S5PC110_NFCONT_EnbRnBINT		(1<<9)
#define S5PC110_NFCONT_RnB_TransMode	(1<<8)
#define S5PC110_NFCONT_MECCLock			(1<<7)
#define S5PC110_NFCONT_SECCLock			(1<<6)
#define S5PC110_NFCONT_InitMECC			(1<<5)
#define S5PC110_NFCONT_InitSECC			(1<<4)
#define S5PC110_NFCONT_HW_nCE			(1<<3)
#define S5PC110_NFCONT_nCE1				(1<<2)
#define S5PC110_NFCONT_nCE0				(1<<1)
#define S5PC110_NFCONT_MODE				(1<<0)

#define S5PC110_ADDR_NALE 0xC
#define S5PC110_ADDR_NCLE 0x8

#ifdef CONFIG_NAND_SPL

/* in the early stage of NAND flash booting, printf() is not available */
#define printf(fmt, args...)

static void nand_read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	int i;
	struct nand_chip *this = mtd->priv;

	for (i = 0; i < len; i++)
		buf[i] = readb(this->IO_ADDR_R);
}
#endif

static struct s5pc110_gpio *s5pc110_gpio;

static void s5pc110_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip *chip = mtd->priv;
	struct s5pc110_nand *nand = (struct s5pc110_nand *)samsung_get_base_nand();

	debug("hwcontrol(): 0x%02x 0x%02x\n", cmd, ctrl);

	ulong IO_ADDR_W = (ulong)nand;

	if (ctrl & NAND_CTRL_CHANGE) {

		if (ctrl & NAND_CLE)
			IO_ADDR_W |= S5PC110_ADDR_NCLE;		/* Command Register  */
		else if (ctrl & NAND_ALE)
			IO_ADDR_W |= S5PC110_ADDR_NALE;		/* Address Register */

		chip->IO_ADDR_W = (void *)IO_ADDR_W;

		if (ctrl & NAND_NCE)	/* select */
			writel(readl(&nand->nfcont) & ~S5PC110_NFCONT_nCE0,
			       &nand->nfcont);
		else					/* deselect */
			writel(readl(&nand->nfcont) | S5PC110_NFCONT_nCE0,
			       &nand->nfcont);
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, chip->IO_ADDR_W);
	else
		chip->IO_ADDR_W = &nand->nfdata;
}

static int s5pc110_dev_ready(struct mtd_info *mtd)
{
	struct s5pc110_nand *nand = (struct s5pc110_nand *)samsung_get_base_nand();
	debug("dev_ready\n");
	return readl(&nand->nfstat) & 0x01;
}

#ifdef CONFIG_S5PC110_NAND_HWECC
void s5pc110_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{
	struct s5pc110_nand *nand = (struct s5pc110_nand *)samsung_get_base_nand();
	debug("s5pc110_nand_enable_hwecc(%p, %d)\n", mtd, mode);
	writel(readl(&nand->nfconf) | S5PC110_NFCONF_INITECC, &nand->nfconf);
}

static int s5pc110_nand_calculate_ecc(struct mtd_info *mtd, const u_char *dat,
				      u_char *ecc_code)
{
	struct s5pc110_nand *nand = (struct s5pc110_nand *)samsung_get_base_nand();
	ecc_code[0] = readb(&nand->nfecc);
	ecc_code[1] = readb(&nand->nfecc + 1);
	ecc_code[2] = readb(&nand->nfecc + 2);
	debug("s5pc110_nand_calculate_hwecc(%p,): 0x%02x 0x%02x 0x%02x\n",
	       mtd , ecc_code[0], ecc_code[1], ecc_code[2]);

	return 0;
}

static int s5pc110_nand_correct_data(struct mtd_info *mtd, u_char *dat,
				     u_char *read_ecc, u_char *calc_ecc)
{
	if (read_ecc[0] == calc_ecc[0] &&
	    read_ecc[1] == calc_ecc[1] &&
	    read_ecc[2] == calc_ecc[2])
		return 0;

	printf("s5pc110_nand_correct_data: not implemented\n");
	return -1;
}
#endif

/**
 * s5pc110_nand_select_chip - [DEFAULT] control CE line
 * @mtd:	MTD device structure
 * @chipnr:	chipnumber to select, -1 for deselect, 0 for select
 *
 * Default select function for 1 chip devices.
 */
static void s5pc110_nand_select_chip(struct mtd_info *mtd, int chipnr)
{
	struct nand_chip *chip = mtd->priv;

	switch (chipnr) {
	case -1: /* deselect chip */
		chip->cmd_ctrl(mtd, NAND_CMD_NONE, 0 | NAND_CTRL_CHANGE);
		break;
	case 0: /* select chip */
		chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
		break;

	default:
		BUG();
	}
}

int board_nand_init(struct nand_chip *nand)
{
	u_int32_t cfg;
	u_int8_t tacls, twrph0, twrph1;
	struct s5pc110_nand *nand_reg = (struct s5pc110_nand *)samsung_get_base_nand();
	int i;

	debug("board_nand_init()\n");

	/* initialize hardware */
#if defined(CONFIG_S5PC110_CUSTOM_NAND_TIMING)
	tacls  = CONFIG_S5PC110_TACLS;
	twrph0 = CONFIG_S5PC110_TWRPH0;
	twrph1 = CONFIG_S5PC110_TWRPH1;
#else
	tacls  = 3; 
	twrph0 = 3;
	twrph1 = 2;
#endif

	cfg = S5PC110_NFCONF_MSG_LEN_512;
	cfg |= S5PC110_NFCONF_ECCTYPE_1BIT; 
	cfg |= S5PC110_NFCONF_TACLS(tacls - 1);
	cfg |= S5PC110_NFCONF_TWRPH0(twrph0 - 1);
	cfg |= S5PC110_NFCONF_TWRPH1(twrph1 - 1);
	cfg |= S5PC110_NFCONF_SLC_FLASH;	
	cfg |= S5PC110_NFCONF_PAGE_SIZE_2K;	
	cfg |= S5PC110_NFCONF_ADDR_CYCLE_5;	
	writel(cfg, &nand_reg->nfconf);

	cfg = S5PC110_NFCONT_MODE;
	cfg |= S5PC110_NFCONT_nCE0;
	writel(cfg, &nand_reg->nfcont);

	/*
 	 * port map
 	 * CE1-> Xm0CSn2 -> MP01_2
 	 * CLE-> Xm0FCLE -> MP03_0
 	 * ALE-> Xm0FALE -> MP03_1
 	 * WE -> Xm0FWEn -> MP03_2
 	 * RE -> Xm0FREn -> MP03_3
 	 * R/B1->Xm0FRnB0-> MP03_4
 	 * IO[7:0]->Xm0DATA[7:0]->MP0_6[7:0]
 	 */
	s5pc110_gpio = (struct s5pc110_gpio *)S5PC110_GPIO_BASE;
	s5p_gpio_cfg_pin(&s5pc110_gpio->mp0_1, 2, 0x3);
	for (i = 0; i < 5; i++)
	{
		s5p_gpio_cfg_pin(&s5pc110_gpio->mp0_3, i, 0x2);
	}
	for (i = 0; i < 8; i++)
	{
		s5p_gpio_cfg_pin(&s5pc110_gpio->mp0_6, i, 0x2);
	}
	/* initialize nand_chip data structure */
	nand->IO_ADDR_R = (void *)&nand_reg->nfdata;
	nand->IO_ADDR_W = (void *)&nand_reg->nfdata;

	nand->select_chip = s5pc110_nand_select_chip;

	/* read_buf and write_buf are default */
	/* read_byte and write_byte are default */
#ifdef CONFIG_NAND_SPL
	nand->read_buf = nand_read_buf;
#endif

	/* hwcontrol always must be implemented */
	nand->cmd_ctrl = s5pc110_hwcontrol;

	nand->dev_ready = s5pc110_dev_ready;

#ifdef CONFIG_S5PC110_NAND_HWECC
	nand->ecc.hwctl = s5pc110_nand_enable_hwecc;
	nand->ecc.calculate = s5pc110_nand_calculate_ecc;
	nand->ecc.correct = s5pc110_nand_correct_data;
	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.size = CONFIG_SYS_NAND_ECCSIZE;
	nand->ecc.bytes = CONFIG_SYS_NAND_ECCBYTES;
#else
	nand->ecc.mode = NAND_ECC_SOFT;
#endif

#ifdef CONFIG_S5PC110_NAND_BBT
	nand->options = NAND_USE_FLASH_BBT;
#else
	nand->options = 0;
#endif

	debug("end of nand_init\n");

	return 0;
}
