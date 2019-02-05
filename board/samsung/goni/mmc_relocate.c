typedef unsigned int (*copy_sd_mmc_to_mem) (unsigned int channel, unsigned int start_block, unsigned char block_size, unsigned int *trg, unsigned int init);

void copy_code_to_dram(void)
{
	unsigned long ch;
	unsigned long dest = 0x34800000;	// dest addr in DDR
	unsigned int  sec_no = 49;			// sector number in SD card

	unsigned int  ret;

	ch = *(volatile unsigned int *)(0xD0037488);

	copy_sd_mmc_to_mem copy_bl2 = (copy_sd_mmc_to_mem) (*(unsigned int *) (0xD0037F98));

	// channel 0
	if(ch == 0xEB000000)
	{
		// 0: channel 0
		// 49: secter number, 1 sector = 512 bytes
		ret = copy_bl2(0, sec_no,       128, (unsigned int *)dest, 0);					// 64K
		ret = copy_bl2(0, sec_no + 128, 128, (unsigned int *)(dest + 0x10000), 0);		// 128K
		ret = copy_bl2(0, sec_no + 256, 128, (unsigned int *)(dest + 0x20000), 0);		// 192K
		ret = copy_bl2(0, sec_no + 384, 128, (unsigned int *)(dest + 0x30000), 0);		// 256K
	}
}
