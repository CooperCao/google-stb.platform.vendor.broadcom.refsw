/***************************************************************
**
** Broadcom Corp. Confidential
** Copyright 1998-2000 Broadcom Corp. All Rights Reserved.
**
** THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED
** SOFTWARE LICENSE AGREEMENT BETWEEN THE USER AND BROADCOM.
** YOU HAVE NO RIGHT TO USE OR EXPLOIT THIS MATERIAL EXCEPT
** SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
**
** File:		bcm_mips_defs.h
** Description:  MIPS definitions.
**
****************************************************************/
#ifndef __BCM_MIPS_DEFS__
#define __BCM_MIPS_DEFS__

#define BCM_PHYS_TO_KSEG1(x)	((unsigned int)(x) | 0xA0000000)
#define BCM_CPU_TO_PHYS(x)	((unsigned int)(x) & ~0xA0000000)
#define BCM_PHYS_TO_KSEG0(x)	((unsigned int)(x) | 0x80000000)
/***************************************************************************
Summary:
	Macro to write a cp0 register.
	
Description:
	asm macro to write a cp0 register given the register, select and value. (MIPS32)
		
See Also:
	bcm_read_cp0
***************************************************************************/
#define bcm_write_cp0(reg, sel, value)					\
{		__asm__ __volatile__(".set\tpush\n\t"			\
			".set\tmips32\n\t"							\
			"mtc0\t%z0, " #reg ", " #sel "\n\t"	\
			".set\tpop\n\t"							\
			: /* none */								\
			: "r" ((unsigned int)value));				\
}

/***************************************************************************
Summary:
	Macro to read a cp0 register.
	
Description:
	asm macro to read a cp0 register given the register and select. (MIPS32)
		
See Also:
	bcm_read_cp0
***************************************************************************/
#define bcm_read_cp0(reg, sel)							\
({ unsigned int bcm_read_cp0_res;						\
		__asm__ __volatile__(	".set\tpush\n\t"		\
			".set\tmips32\n\t"							\
			"mfc0\t%0, " #reg ", " #sel "\n\t"			\
			".set\tpop\n\t"							\
			: "=r" (bcm_read_cp0_res));					\
	bcm_read_cp0_res;									\
})


#endif /* __BCM_MIPS_DEFS__ */

