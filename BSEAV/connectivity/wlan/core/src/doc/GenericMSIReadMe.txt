************************************************************************************************************
* Copyright 2000-2011, Broadcom Corporation                                                                            *
*                                                                                                                                     *
* Please read this document if you are using either Generic Asynchronous MSI or                 *
* Generic Synchronous MSI bus interfaces on BCM4210 BCM4211 or BCM4413 chips.	        *
* Please email hnsupport@broadcom.com if you have any questions                                    *
************************************************************************************************************

Accessing Slow Registers in  MSI Mode
=============================

BCM4413, BCM4211 and BCM4210 provide two choices for host interfaces (i) PCI and  
(ii) Microprocessor Slave Interface (MSI). MSI mode supports 16-bit read/write accesses to 
the chip registers. One of the MSI mode interfaces is called Generic MSI. Generic MSI mode 
can be either synchronous or asynchronous depending on the style of bus transactions.
(Please see the data sheets for timing diagrams for MSI Generic Mode as well as the 
section Slow Access Registers).

BCM4413, BCM4211 and BCM4210 have both fast and slow access registers. Consecutive 
read/write accesses to slow registers results in non-deterministic results. To prevent these 
hazards a delay equal to certain multiple of the master clock (tcyc) is needed between 
consecutive accesses to slow access registers registers. PCI and all MSI modes besides 
Generic MSI, provide hardware support to take care of the above hazards associated with 
accessing Slow Access Registers. In MSI Generic Mode (Asynchronous or Synchronous) 
without hardware support to avoid the above delay hazard, appropriate delays need to be
provided in software. This document contains sample code to solve the above problem.

In order to make this delay independant of the master clock speed, it is expressed in terms 
of the number of reads to fast registers. In the sample code the device status register is read 
to provide the delay, since reading it does not result in any side effects.

=====================
	ILINE
=====================

The slow registers are in the range 0x40 thru 0x7F and 0x94.

44XX  - In MSI generic (synchronous as well as asynchronous) mode, a slow register read is 
           completed when the RegisterReadReady bit in Device Status Register is set.
         - In MSI generic asynchronous mode  slow register writes take 14 tcyc and 
           fast register reads take 3 tcyc. Hence we need atleast 5 fast register reads between 
           consecutive slow register writes.
         - In MSI generic synchronous mode  slow register writes take 13 tcyc and fast register 
           reads take 2 tcyc. Hence we need atleast 7 fast register reads between consecutive 
           slow register writes.

42XX  - Here the software has no way of knowing whether it is in generic 
           synchronous/asynchronous mode. Hence we use the following:

           let    Tsminrd = minimum no. of cycles for slow register reads.
	    Tsminwr = minimum no. of cycles for slow register writes.
                  Tfminrd = minimum no. of cycles for fast register reads.

           no. of cycles for slow register reads = maximum of 
           (Tsminrd in generic synchronous MSI , Tsminrd in generic asynchronous MSI)

           no. of cycles for slow register writes = maximum of 
           (Tsminwr in generic synchronous MSI , Tsminwr in generic asynchronous MSI)

           no. of cycles for fast register reads = minimum of 
           (Tfminrd in generic synchronous MSI , Tfminrd in generic asynchronous MSI)

         - Generic MSI slow register reads take 9 tcyc, and fast register reads take 3 tcyc, 
           hence we need atleast 3 fast register reads between consecutive slow register reads.
         - Generic MSI slow register writes take 8 tcyc, and fast register reads take 3 tcyc, 
           hence we need atleast 3 fast register reads between consecutive slow reg writes.
	
Implementation:
===========

additions in src/bcm42xx/sys/il_(os specific).h
==================================

#ifdef IL_MSIGENERIC

#define IL_SLOW_ACCESS_REG(offset) \
	(((offset == 0x94)||(offset == 0x96)||((offset >= 0x40)&&(offset <= 0x7F))) ? 1: 0)	

/* minimum no. of fast register reads between consecutive accesses to slow registers */
#define IL_44XX_GENASYNCMSI_WRDELAY	5 /* for generic async msi  slow reg writes */ 					      	
#define IL_44XX_GENSYNCMSI_WRDELAY	7 /* for generic sync msi slow reg writes */

#define IL_42XX_GENMSI_RDDELAY		3 /* for generic sync/async msi slow reg reads */
#define IL_42XX_GENMSI_WRDELAY		3 /* for generic sync/async msi slow reg writes */

uint16	il_read_reg(void *il, volatile uint16 *reg);
void	il_write_reg(void *il, volatile uint16 *reg, uint16 value);

#define MSIGEN_R_REG(r)		os_specific_function_for_16bit_register_read((uint16 *)r)
#define MSIGEN_W_REG(r, v)	os_specific_function_for_16bit_register_write((uint16 *)r, (uint16)v )

#define MSI_R_REG(r)	il_read_reg((void *)ilc, (volatile uint16 *)r)
#define MSI_W_REG(r, v)	il_write_reg((void *)ilc, (volatile uint16 *)r, v)

#ifdef IL_BIGENDIAN
#define	R_REG(r)	((sizeof *(r) == sizeof (uint32))? \
	((uint32)( MSI_R_REG(&((uint16*)(r))[0]) | (MSI_R_REG(&((uint16*)(r))[1]) << 16))) \
	: MSI_R_REG(r))
#define	W_REG(r, v)	((sizeof *(r) == sizeof (uint32))? \
	(MSI_W_REG((&((uint16*)r)[0]), ((v) & 0xffff)), MSI_W_REG((&((uint16*)r)[1]), (((v) >> 16) & 0xffff))) \
	: (MSI_W_REG(r , v)))
#else	/* LITTLE_ENDIAN */
#define	R_REG(r)	((sizeof *(r) == sizeof (uint32))? \
	((uint32)((MSI_R_REG(&((uint16*)(r))[0]) << 16) | MSI_R_REG(&((uint16*)(r))[1]))) \
	: MSI_R_REG(r))
#define	W_REG(r, v)	((sizeof *(r) == sizeof (uint32))? \
	(MSI_W_REG((&((uint16*)r)[1]), ((v) & 0xffff)), MSI_W_REG((&((uint16*)r)[0]), (((v) >> 16) & 0xffff))) \
	: (MSI_W_REG(r , v)))
#endif	/* BIG_ENDIAN */


#else	/* PCI */

#define	R_REG(r)	((sizeof *(r) == sizeof (uint32))? \
			os_specific_function_for_32bit_register_read((uint32 *)r) : \
			os_specific_function_for_16bit_register_read((uint16 *)r))

#define	W_REG(r, v)	((sizeof *(r) == sizeof (uint32))? \
			os_specific_function_for_32bit_register_write((uint32 *)r, (uint32)v) : \
			os_specific_function_for_16bit_register_wriite((uint16 *)r, (uint16)v))

#endif	/* MSI */

additions in src/bcm42xx/sys/il_(os specific).c
==================================
typedef struct il_info {
		:
		:
		:
/* Add the following */
#ifdef IL_MSIGENERIC			
	uint	msigenrddelay;		/* delay for consecutive slow reg reads */
	uint	msigenwrdelay;		/* delay for consecutive slow reg writes */
#endif
} il_info_t;


#ifdef IL_MSIGENERIC
uint16 
il_read_reg(void *il, volatile uint16 *reg)
{
	uint		tmp, delay;
	ilc_info_t	*ilc = &((il_info_t *)il)->ilc;

	tmp = (uint16 *)ilc->regs - reg ;
	if (IL_SLOW_ACCESS_REG(tmp)){
		/* we need to access a slow register */
		tmp = (uint)MSIGEN_R_REG(reg);
		
		if (IS4410(ilc->deviceid)){
			delay = 100000;/* so that we don't spin here for ever! */
			while ((!(MSIGEN_R_REG(&((uint16 *)(ilc->regs->devstatus))[0]) & DS_RR))&&(delay))
				delay--;
		}else{
			delay = ((il_info_t *)il)->msigenrddelay;
			while (delay--)
			{
				tmp = MSIGEN_R_REG(&((uint16 *)(ilc->regs->devstatus))[0]);
			}
		}
	}
	return (MSIGEN_R_REG(reg));
}

void
il_write_reg(void *il, volatile uint16 *reg, uint16 value)
{
	uint		tmp;
	ilc_info_t	*ilc = &((il_info_t *)il)->ilc;

	MSIGEN_W_REG(reg, value);

	tmp = (uint16 *)ilc->regs - reg ;

	if (IL_SLOW_ACCESS_REG(tmp)) {
		/* we need to access a slow register */
		tmp = ((il_info_t *)il)->msigenwrdelay;
		while (tmp--) {
			value = MSIGEN_R_REG(&((uint16 *)(ilc->regs->devstatus))[0]);
		}
	}
}

#endif /* IL_MSIGENERIC */


=====================
	Ethernet
=====================

The slow registers on the Ethernet interface are in the range 0x400 thru 0x5FF and 0x94. Also 

44XX  - In MSI generic (synchronous as well as asynchronous) mode, a slow register read is 
           completed when the RegisterReadReady bit in Device Status Register is set.
         - In MSI generic asynchronous mode  slow register writes take 14 tcyc and 
           fast register reads take 3 tcyc. Hence we need atleast 5 fast register reads between 
           consecutive slow register writes.
         - In MSI generic synchronous mode  slow register writes take 13 tcyc and fast register 
           reads take 2 tcyc. Hence we need atleast 7 fast register reads between consecutive 
           slow register writes.

Implementation:
===========

additions in components/et/sys/et_(os specific).h
==================================

#ifdef ET_MSIGENERIC
#define ET_SLOW_ACCESS_REG(offset) \
	(((offset == 0x94)||(offset == 0x96)||((offset >= 0x400)&&(offset <= 0x5FF))) ? 1: 0)
	
/* minimum no. of fast register reads between consecutive accesses to slow registers */
#define ET_44XX_GENASYNCMSI_WRDELAY	5 /* for generic async msi  slow reg writes */
#define ET_44XX_GENSYNCMSI_WRDELAY	7 /* for generic sync msi slow reg writes */

uint16	et_read_reg(void *et, volatile uint16 *reg);
void	et_write_reg(void *et, volatile uint16 *reg, uint16 value);

#define MSIGEN_R_REG(r)		os_specific_function_for_16bit_register_read((uint16 *)r)
#define MSIGEN_W_REG(r, v)	os_specific_function_for_16bit_register_write((uint16 *)r, (uint16)v )

#define MSI_R_REG(r)	et_read_reg((void *)etc, (volatile uint16 *)r)
#define MSI_W_REG(r, v)	et_write_reg((void *)etc, (volatile uint16 *)r, (uint16)v)

#ifdef ET_BIGENDIAN

#define	R_REG(r)	((sizeof *(r) == sizeof (uint32))? \
	((uint32)( MSI_R_REG(&((uint16*)(r))[0]) | (MSI_R_REG(&((uint16*)(r))[1]) << 16))) \
	: MSI_R_REG(r))
#define	W_REG(r, v)	((sizeof *(r) == sizeof (uint32))? \
	(MSI_W_REG((&((uint16*)r)[0]), ((v) & 0xffff)), MSI_W_REG((&((uint16*)r)[1]), (((v) >> 16) & 0xffff))) \
	: (MSI_W_REG(r , v)))

#else	/* LITTLE_ENDIAN */

#define	R_REG(r)	((sizeof *(r) == sizeof (uint32))? \
	((uint32)((MSI_R_REG(&((uint16*)(r))[0]) << 16) | MSI_R_REG(&((uint16*)(r))[1]))) \
	: MSI_R_REG(r))
#define	W_REG(r, v)	((sizeof *(r) == sizeof (uint32))? \
	(MSI_W_REG((&((uint16*)r)[1]), ((v) & 0xffff)), MSI_W_REG((&((uint16*)r)[0]), (((v) >> 16) & 0xffff))) \
	: (MSI_W_REG(r , v)))

#endif	/* BIG_ENDIAN */

#else	/* ET_MSIGENERIC */

#define	R_REG(r)	((sizeof *(r) == sizeof (uint32))? \
			os_specific_function_for_32bit_register_read((uint32 *)r) : \
			os_specific_function_for_16bit_register_read((uint16 *)r))

#define	W_REG(r, v)	((sizeof *(r) == sizeof (uint32))? \
			os_specific_function_for_32bit_register_write((uint32 *)r, (uint32)v) : \
			os_specific_function_for_16bit_register_wriite((uint16 *)r, (uint16)v))

#endif /* ET_MSIGENERIC */


additions in components/et/sys/et_(os specific).c
==================================


typedef struct et_info {
		:
		:
		:
/* Add the following */
#ifdef ET_MSIGENERIC
	uint	msigenrddelay;		/* delay for consecutive slow reg reads */
	uint	msigenwrdelay;		/* delay for consecutive slow reg writes */
#endif
} et_info_t;

#ifdef ET_MSIGENERIC

uint16 
et_read_reg(void *et, volatile uint16 *reg)
{
	uint		tmp, delay;
	etc_info_t	*etc = &((et_info_t *)et)->etc;
	
	tmp = (uint16 *)etc->regs - reg ;
	if (ET_SLOW_ACCESS_REG(tmp)){
		/* we need to access a slow register */
		tmp = (uint)MSIGEN_R_REG(reg);
		
		if (IS4412(etc->deviceid)){
			delay = 100000;/* so that we don't spin here for ever! */
			while ((!(MSIGEN_R_REG(&((uint16 *)(etc->regs->devstatus))[0]) & DS_RR))&&(delay))
				delay--;
		}
	}
	return (MSIGEN_R_REG(reg));
}

void
et_write_reg(void *et, volatile uint16 *reg, uint16 value)
{
	uint		tmp;
	etc_info_t	*etc = &((et_info_t *)et)->etc;

	MSIGEN_W_REG(reg, value);
	tmp = (uint16 *)etc->regs - reg ;

	if (ET_SLOW_ACCESS_REG(tmp)) {
		/* we need to access a slow register */
		tmp = ((et_info_t *)et)->msigenwrdelay;
		while (tmp--)
		{
			value = MSIGEN_R_REG(&((uint16 *)(etc->regs->devstatus))[0]);
		}
	}
}

#endif /* ET_MSIGENERIC */
