/************************************************
 * NAME	    : nand_reg.h
 * Version  : 2019.01.28
 *
 * common stuff for SAMSUNG s5pc110 SoC
 ************************************************/

#ifndef __ARCH_ARM_ASM_NAND_REG_H__
#define __ARCH_ARM_ASM_NAND_REG_H__

#ifndef __ASSEMBLY__
/* NAND FLASH (see S5PV210 manual section 05) */
struct s5pc110_nand {
    unsigned int    nfconf;
    unsigned int    nfcont;
    unsigned int    nfcmmd;
    unsigned int    nfaddr;
    unsigned int    nfdata;
    unsigned int    nfmeccd0;
    unsigned int    nfmeccd1;
    unsigned int    nfseccd;
    unsigned int    nfsblk;
    unsigned int    nfeblk;
    unsigned int    nfstat;
    unsigned int    nfeccerr0;
    unsigned int    nfeccerr1;
    unsigned int    nfmecc0;
    unsigned int    nfmecc1;
    unsigned int    nfsecc;
    unsigned int    nfmlcbitpt;
};
#endif

#endif  /* __ARCH_ARM_ASM_NAND_REG_H__ */
