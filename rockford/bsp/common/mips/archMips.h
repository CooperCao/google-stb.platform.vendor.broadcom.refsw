/* archMips.h - MIPS architecture specific header */

/* Copyright 1984-2001 Wind River Systems, Inc. */

/*
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01t,16jul01,ros  add CofE comment
01s,07jun01,mem  Re-introduce optimized kernel.
01r,13feb01,pes  Add support for RM7000 extended interrupts
01q,05jan01,mem  Remove decl of _wdbDbgCtx routines.
01p,04jan01,agf  Adding TLB_ENTRIES define
01o,03jan01,pes  Remove definition of IS_KSEGM. This is not supported in
                 Tornado 2.
01n,20dec00,pes  Merge some definitions from target/h/arch/r4000.h.
01m,12oct99,yvp  Merge from .../tor2_0_0.coretools-baseline branch to make
                 Tor2 code work with T3 main/LATEST compiler
01l,07sep99,myz  added CW4000_16 compiler switch
01k,22jan97,alp  added  CW4000 and CW4010 support.
01l,11nov99,dra  Updated for T3 toolchain.
01k,18oct99,dra  added CW4000, CW4011, VR4100, VR5000 and VR5400 support.
		 added test for both MIPSEB and MIPSEL defined.
01k,00oct00,sn   removed varargs/typedefs for GNUC
01j,14oct96,kkk  added R4650 support.
01i,05aug96,kkk  Changed _ALLOC_ALIGN_SIZE & _STACK_ALIGN_SIZE to 16.
01i,27jun96,kkk  undo 01h.
01h,30apr96,mem  fixed varargs support for R4000, use 64 bit stack slots.
01g,27may95,cd	 switched from MIPSE[BL] to _BYTE_ORDER for ansi compliance
01f,15oct93,cd   added support for R4000, SDE compiler and 64 bit kernel.
01e,22sep92,rrr  added support for c++
01d,07jun92,ajm  added _ARCH_MULTIPLE_CACHES for cache init
01c,03jun92,ajm  updated file name referenced to match real name
01b,26may92,rrr  the tree shuffle
01a,22apr92,yao  written.
*/

#ifndef __INCarchMipsh
#define __INCarchMipsh

#ifdef __cplusplus
extern "C" {
#endif

#define	_ARCH_MULTIPLE_CACHELIB		TRUE
#define _DYNAMIC_BUS_SIZING		FALSE	/* no dynamic bus sizing */
#define	_ALLOC_ALIGN_SIZE		16
#define _STACK_ALIGN_SIZE		16
#define _CACHE_ALIGN_SIZE		32 	/* max cache line size */

#if defined(MIPSEB) || defined(__MIPSEB__)
#undef _BYTE_ORDER
#define	_BYTE_ORDER             _BIG_ENDIAN
#elif defined(MIPSEL) || defined(__MIPSEL__)
#undef _BYTE_ORDER
#define	_BYTE_ORDER		_LITTLE_ENDIAN
#else
#warning "One of MIPSEL or MIPSEB must be defined"
#endif

#ifdef __GNUC__
#define _ARCH_LONG_MIN			(-LONG_MAX-1)
#define _ARCH_INT_MIN			(-INT_MAX-1)
#endif

#if	(CPU == MIPS32)

#define _WRS_INT_REGISTER_SIZE		4
#define _WRS_FP_REGISTER_SIZE		4

#ifndef _ASMLANGUAGE
typedef unsigned long _RType;			/* registers are 32 bits */
#endif

#elif	(CPU == MIPS64)

#define _WRS_INT_REGISTER_SIZE		8
#define _WRS_FP_REGISTER_SIZE		8

#ifndef _ASMLANGUAGE
typedef unsigned long long _RType;		/* registers are 64 bits */
#endif

#else	/* CPU */
#error "Invalid CPU value"
#endif	/* CPU */

#define _RTypeSize			_WRS_INT_REGISTER_SIZE

#define SR_INT_ENABLE			SR_IE	/* interrupt enable bit */

#ifdef _WRS_R3K_EXC_SUPPORT

#define POP_SR(p) (((p) & ~(SR_KUMSK)) | (((p) & SR_KUMSK) >> 2))

#else	/* _WRS_R3K_EXC_SUPPORT */

#define POP_SR(p) ((p) & ~SR_EXL)

#endif	/* _WRS_R3K_EXC_SUPPORT */

/*
 * Segment base addresses and sizes
 */

#define	K0BASE		0x80000000
#define	K0SIZE		0x20000000
#define	K1BASE		0xA0000000
#define	K1SIZE		0x20000000

#define KMBASE		0xC0000000		/* mapped kernel space */
#define	KMSIZE		0x40000000

#define	KSBASE		0xC0000000
#define	KSSIZE		0x20000000
#define	K2BASE		0xC0000000
#define	K2SIZE		0x40000000
#define	K3BASE		0xE0000000
#define	K3SIZE		0x20000000

/*
 * Address conversion macros
 */

#define KSEG2_TO_KSEG0_MASK	(~(1<<30))

#define	K0_TO_K1(x)	((unsigned)(x)|0xA0000000)	/* kseg0 to kseg1 */
#define	K1_TO_K0(x)	((unsigned)(x)&0x9FFFFFFF)	/* kseg1 to kseg0 */

#define	K0_TO_PHYS(x)	((unsigned)(x)&0x1FFFFFFF)	/* kseg0 to physical */
#define	K1_TO_PHYS(x)	((unsigned)(x)&0x1FFFFFFF)	/* kseg1 to physical */

#define	PHYS_TO_K0(x)	((unsigned)(x)|0x80000000)	/* physical to kseg0 */
#define	PHYS_TO_K1(x)	((unsigned)(x)|0xA0000000)	/* physical to kseg1 */

/*
 * The "KM" conversion macros only work for limited sections of the
 * mapped kernel space.
 */

#define	KM_TO_K0(x)	((unsigned)(x)&~0x40000000)
#define	KM_TO_K1(x)	(((unsigned)(x)&~0x40000000)|0x20000000)
#define	KM_TO_PHYS(x)	((unsigned)(x)&0x1FFFFFFF)
#define	PHYS_TO_KM(x)	((unsigned)(x)|0xC0000000)

/*
 * Address predicates
 */

#define	IS_KSEG0(x)	((unsigned)(x) >= K0BASE && (unsigned)(x) < K1BASE)
#define	IS_KSEG1(x)	((unsigned)(x) >= K1BASE && (unsigned)(x) < KMBASE)
#define	IS_KUSEG(x)	((unsigned)(x) < K0BASE)

/* PAL additions */
#define FLOAT_NORM			/* xdr */
#define ffsLib_PORTABLE			/* ffsLib.c */

#define STK_PAGE_GUARD_ATTR		PAGE_MGR_OPTS_SUP_DATA_RO
#define _WRS_ARCH_BLIB
#define	LOGICAL_TO_VIRT(X)		PHYS_TO_KM(K0_TO_PHYS(X))
#define _WRS_DUAL_MAPPED_SYS_PAGE
#define _WRS_VIRTUALLY_INDEXED_CACHES
#define _WRS_CACHE_PAL			/* cache library has been PAL-ified */

/* dbgTaskLib.c, wdbBpLib.c 
 *
 * On MIPS CPUs, when a breakpoint exception occurs in a branch delay slot,
 * the PC has been changed in the breakpoint handler to match with the
 * breakpoint address.
 * Once the matching has been made, the PC is modified to have its normal
 * value (the preceding jump instruction).
 */
#ifndef _WRS_ADJUST_PC_FOR_BRANCH_DELAY
#define _WRS_ADJUST_PC_FOR_BRANCH_DELAY(pReg) \
    if (pReg->cause & CAUSE_BD)  \
        pReg->reg_pc--;
#endif /* _WRS_ADJUST_PC_FOR_BRANCH_DELAY */

/* vxwTaskLib.h */
#define _WRS_NEED_SRSET			/* target/h/vxwTaskLib.h */
#ifndef _WRS_SRSET_ARG_TYPE
#define _WRS_SRSET_ARG_TYPE	UINT32		/* argument to SRSet */
#endif /* _WRS_SRSET_ARG_TYPE */

/* wdbDbgLib.h */
#define WDB_CTX_LOAD(pRegs) _wdbDbgCtxLoad(pRegs)
#define WDB_CTX_SAVE(pRegs) _wdbDbgCtxSave(pRegs)

#ifndef _WRS_SPY_TASK_SIZE
#define _WRS_SPY_TASK_SIZE	10000			/* spyLib.c */
#endif /* _WRS_SPY_TASK_SIZE */

#define _WRS_ARCH_LOADELF	LOADELF_MIPS_MODEL	/* loadElf.c */

#define _WRS_CHECK_REGISTER_WIDTH	/* for register display */

/* Target shell support */
#define _WRS_SHELL_DATA_TYPES		SHELL_ALL_DATA_SUPPORT

/*
 * Exception vectors
 */
#ifdef _WRS_R3K_EXC_SUPPORT

#define	UT_VEC		K0BASE			/* utlbmiss vector */
#define	E_VEC		(K0BASE+0x80)		/* exception vector */

#else	/* _WRS_R3K_EXC_SUPPORT */

#define	T_VEC		K0BASE			/* tlbmiss vector */
#define	X_VEC		(K0BASE+0x80)		/* xtlbmiss vector */
#define	C_VEC		(K1BASE+0x100)		/* cache exception vector */
#define	E_VEC		(K0BASE+0x180)		/* exception vector */

#endif	/* _WRS_R3K_EXC_SUPPORT */

#define	R_VEC		(K1BASE+0x1fc00000)	/* reset vector */

/*
 * Cache alignment macros
 *
 * NOTE: These definitions may migrate to vxWorks.h in a future release.
 */
#define	CACHE_ROUND_UP(x)	ROUND_UP(x, _CACHE_ALIGN_SIZE)
#define	CACHE_ROUND_DOWN(x)	ROUND_DOWN(x, _CACHE_ALIGN_SIZE)

/*
* Cache size constants
*/

#define	R3KMINCACHE	+(1*512)	/* leading plus for mas's benefit */
#define	R3KMAXCACHE	+(256*1024)	/* leading plus for mas's benefit */
#define	R4KMINCACHE	+(1*1024)	/* leading plus for mas's benefit */
#define	R4KMAXCACHE	+(256*1024)	/* leading plus for mas's benefit */

/*
 * Cause bit definitions
 */
#define	CAUSE_BD	0x80000000	/* Branch delay slot */
#define	CAUSE_CEMASK	0x30000000	/* coprocessor error */
#define	CAUSE_CESHIFT	28

#define	CAUSE_IP8	0x00008000	/* External level 8 pending */
#define	CAUSE_IP7	0x00004000	/* External level 7 pending */
#define	CAUSE_IP6	0x00002000	/* External level 6 pending */
#define	CAUSE_IP5	0x00001000	/* External level 5 pending */
#define	CAUSE_IP4	0x00000800	/* External level 4 pending */
#define	CAUSE_IP3	0x00000400	/* External level 3 pending */
#define	CAUSE_SW2	0x00000200	/* Software level 2 pending */
#define	CAUSE_SW1	0x00000100	/* Software level 1 pending */

/* extended interrupt cause bits (e.g., RM7000) */
#define CAUSE_IP16	0x00800000	/* RESERVED */
#define CAUSE_IP15	0x00400000	/* RESERVED */
#define CAUSE_IP14	0x00200000	/* Perf counter */
#define CAUSE_IP13	0x00100000	/* Alt Timer */
#define CAUSE_IP12	0x00080000	/* External level 12 pending */
#define CAUSE_IP11	0x00040000	/* External level 11 pending */
#define CAUSE_IP10	0x00020000	/* External level 10 pending */
#define CAUSE_IP9	0x00010000	/* External level 9 pending */

/* Note: mask includes extended interrupt bit positions */
#define	CAUSE_IPMASK	0x00FFFF00	/* Pending interrupt mask */
#define	CAUSE_IPSHIFT	8

#ifdef _WRS_R3K_EXC_SUPPORT

#define	CAUSE_EXCMASK	0x0000003C	/* Cause code bits */
#define	CAUSE_EXCSHIFT	2

#else	/* _WRS_R3K_EXC_SUPPORT */

#define	CAUSE_EXCMASK	0x0000007C	/* Cause code bits */
#define	CAUSE_EXCSHIFT	2

#endif	/* _WRS_R3K_EXC_SUPPORT */

/*
 * Status definition bits
 */
#define	SR_CUMASK	0xf0000000	/* coproc usable bits */
#define	SR_CU3		0x80000000	/* Coprocessor 3 usable */
#define	SR_CU2		0x40000000	/* Coprocessor 2 usable */
#define	SR_CU1		0x20000000	/* Coprocessor 1 usable */
#define	SR_CU0		0x10000000	/* Coprocessor 0 usable */
#define SR_RP		0x08000000      /* Use reduced power mode */
#define SR_FR		0x04000000      /* Enable extra floating point regs */
#define SR_RE		0x02000000      /* Reverse endian in user mode */
#define	SR_BEV		0x00400000	/* use boot exception vectors */
#define	SR_TS		0x00200000	/* TLB shutdown */
#define SR_SR		0x00100000	/* soft reset occurred */
#define	SR_CH		0x00040000	/* cache hit */
#define	SR_CE		0x00020000	/* use ECC reg */
#define	SR_DE		0x00010000	/* disable cache errors */
#define	SR_IMASK	0x0000ff00	/* Interrupt mask */
#define	SR_IMASK8	0x00000000	/* mask level 8 */
#define	SR_IMASK7	0x00008000	/* mask level 7 */
#define	SR_IMASK6	0x0000c000	/* mask level 6 */
#define	SR_IMASK5	0x0000e000	/* mask level 5 */
#define	SR_IMASK4	0x0000f000	/* mask level 4 */
#define	SR_IMASK3	0x0000f800	/* mask level 3 */
#define	SR_IMASK2	0x0000fc00	/* mask level 2 */
#define	SR_IMASK1	0x0000fe00	/* mask level 1 */
#define	SR_IMASK0	0x0000ff00	/* mask level 0 */
#define	SR_IBIT8	0x00008000	/* bit level 8 */
#define	SR_IBIT7	0x00004000	/* bit level 7 */
#define	SR_IBIT6	0x00002000	/* bit level 6 */
#define	SR_IBIT5	0x00001000	/* bit level 5 */
#define	SR_IBIT4	0x00000800	/* bit level 4 */
#define	SR_IBIT3	0x00000400	/* bit level 3 */
#define	SR_IBIT2	0x00000200	/* bit level 2 */
#define	SR_IBIT1	0x00000100	/* bit level 1 */

#define	SR_IMASKSHIFT	8

/* R4K specific */
#define	SR_KX		0x00000080	/* enable kernel 64bit addresses */
#define	SR_SX		0x00000040	/* enable supervisor 64bit addresses */
#define	SR_UX		0x00000020	/* enable user 64bit addresses */
#define	SR_KSUMASK	0x00000018	/* mode mask */
#define SR_KSU_K	0x00000000	/* kernel mode */
#define SR_KSU_S	0x00000008	/* supervisor mode */
#define SR_KSU_U	0x00000010	/* user mode */
#define	SR_ERL		0x00000004	/* Error Level */
#define	SR_EXL		0x00000002	/* Exception Level */
#define	SR_IE		0x00000001	/* interrupt enable, 1 => enable */

/* R3K specific */
#define	SR_PE		0x00100000	/* R3K: cache parity error */
#define	SR_CM		0x00080000	/* R3K: cache miss */
#define	SR_PZ		0x00040000	/* R3K: cache parity zero */
#define	SR_SWC		0x00020000	/* R3K: swap cache */
#define	SR_ISC		0x00010000	/* R3K: Isolate data cache */

#define	SR_KUO		0x00000020	/* old kernel/user, 0 => k, 1 => u */
#define	SR_IEO		0x00000010	/* old interrupt enable, 1 => enable */
#define	SR_KUP		0x00000008	/* prev kernel/user, 0 => k, 1 => u */
#define	SR_IEP		0x00000004	/* prev interrupt enable, 1 => enable */
#define	SR_KUC		0x00000002	/* cur kernel/user, 0 => k, 1 => u */
#define	SR_IEC		0x00000001	/* cur interrupt enable, 1 => enable */
#define SR_KUMSK	(SR_KUO|SR_IEO|SR_KUP|SR_IEP|SR_KUC|SR_IEC)

/*
 * fpa definitions
 */

#define FP_ROUND	0x3		/* r4010 round mode mask */
#define FP_STICKY 	0x7c		/* r4010 sticky bits mask */
#define FP_ENABLE 	0xf80		/* r4010 enable mode mask */
#define FP_EXC		0x3f000		/* r4010 exception mask */

#define FP_ROUND_N	0x0		/* round to nearest */
#define FP_ROUND_Z 	0x1		/* round to zero */
#define FP_ROUND_P 	0x2		/* round to + infinity */
#define FP_ROUND_M 	0x3		/* round to - infinity */

#define FP_STICKY_I	0x4		/* sticky inexact operation */
#define FP_STICKY_U	0x8		/* sticky underflow */
#define FP_STICKY_O	0x10		/* sticky overflow */
#define FP_STICKY_Z	0x20		/* sticky divide by zero */
#define FP_STICKY_V	0x40		/* sticky invalid operation */

#define FP_ENABLE_I	0x80		/* enable inexact operation */
#define FP_ENABLE_U	0x100		/* enable underflow exc  */
#define FP_ENABLE_O	0x200		/* enable overflow exc  */
#define FP_ENABLE_Z	0x400		/* enable divide by zero exc  */
#define FP_ENABLE_V	0x800		/* enable invalid operation exc  */

#define FP_EXC_I	0x1000		/* inexact operation */
#define FP_EXC_U	0x2000		/* underflow */
#define FP_EXC_O	0x4000		/* overflow */
#define FP_EXC_Z	0x8000		/* divide by zero */
#define FP_EXC_V	0x10000		/* invalid operation */
#define FP_EXC_E	0x20000		/* unimplemented operation */

#define FP_COND		0x800000	/* condition bit */
#define FP_FS		0x1000000	/* flush denormalised results to zero */

#define FP_EXC_SHIFT	12
#define FP_ENABLE_SHIFT	7
#define FP_EXC_MASK	(FP_EXC_I|FP_EXC_U|FP_EXC_O|FP_EXC_Z|FP_EXC_V|FP_EXC_E)
#define FP_ENABLE_MASK	(FP_ENABLE_I|FP_ENABLE_U|FP_ENABLE_O|FP_ENABLE_Z| \
			 FP_ENABLE_V)
#define FP_TASK_STATUS	(FP_FS) 	/* all FP exceptions are disabled
					   we only attempt to hide denormalised
					   results for signals (see fppALib.s
					   and spr #7665) */

/*
 * R4000 Config Register
 */
#define CFG_CM		0x80000000	/* Master-Checker mode */
#define CFG_ECMASK	0x70000000	/* System Clock Ratio */
#define CFG_ECBY2	0x00000000 	/* divide by 2 */
#define CFG_ECBY3	0x00000000 	/* divide by 3 */
#define CFG_ECBY4	0x00000000 	/* divide by 4 */
#define CFG_EPMASK	0x0f000000	/* Transmit data pattern */
#define CFG_EPD		0x00000000	/* D */
#define CFG_EPDDX	0x01000000	/* DDX */
#define CFG_EPDDXX	0x02000000	/* DDXX */
#define CFG_EPDXDX	0x03000000	/* DXDX */
#define CFG_EPDDXXX	0x04000000	/* DDXXX */
#define CFG_EPDDXXXX	0x05000000	/* DDXXXX */
#define CFG_EPDXXDXX	0x06000000	/* DXXDXX */
#define CFG_EPDDXXXXX	0x07000000	/* DDXXXXX */
#define CFG_EPDXXXDXXX	0x08000000	/* DXXXDXXX */
#define CFG_SBMASK	0x00c00000	/* Secondary cache block size */
#define CFG_SBSHIFT	22
#define CFG_SB4		0x00000000	/* 4 words */
#define CFG_SB8		0x00400000	/* 8 words */
#define CFG_SB16	0x00800000	/* 16 words */
#define CFG_SB32	0x00c00000	/* 32 words */
#define CFG_SS		0x00200000	/* Split secondary cache */
#define CFG_SW		0x00100000	/* Secondary cache port width */
#define CFG_EWMASK	0x000c0000	/* System port width */
#define CFG_EWSHIFT	18
#define CFG_EW64	0x00000000	/* 64 bit */
#define CFG_EW32	0x00010000	/* 32 bit */
#define CFG_SC		0x00020000	/* Secondary cache absent */
#define CFG_SM		0x00010000	/* Dirty Shared mode disabled */
#define CFG_BE		0x00008000	/* Big Endian */
#define CFG_EM		0x00004000	/* ECC mode enable */
#define CFG_EB		0x00002000	/* Block ordering */
#define CFG_ICMASK	0x00000e00	/* Instruction cache size */
#define CFG_ICSHIFT	9
#define CFG_DCMASK	0x000001c0	/* Data cache size */
#define CFG_DCSHIFT	6
#define CFG_IB		0x00000020	/* Instruction cache block size */
#define CFG_DB		0x00000010	/* Data cache block size */
#define CFG_CU		0x00000008	/* Update on Store Conditional */
#define CFG_K0MASK	0x00000007	/* KSEG0 coherency algorithm */

/* 
 * R3000 Config Register
 */
#define M_CFG4000       0xbfff0000      /* BBCC Cache Config Reg */

#ifdef _ASMLANGUAGE
#define CFG4000_REG     M_CFG4000
#else   /* _ASMLANGUAGE */
#define CFG4000_REG     ((volatile ULONG *)M_CFG4000)   /* config register */
#endif  /* _ASMLANGUAGE */

#define CFG_CMODESHFT   8
#define CFG_CMODEMASK   (3<<CFG_CMODESHFT)      /* cache mode */
#define CFG_CMODE_IDATA (1<<CFG_CMODESHFT)
#define CFG_CMODE_ITEST (2<<CFG_CMODESHFT)
#define CFG_CMODE_DTEST (3<<CFG_CMODESHFT)
#define CFG_DCEN        (1<<4)  /* D-cache enable */
#define CFG_IS1EN       (1<<1)  /* I-cache set 1 enable */
#define CFG_ICEN        (1<<0)  /* I-cache enable */

#define CFG_DSIZESHFT   5
#define CFG_DSIZEMASK   (3<<CFG_DSIZESHFT)      /* D-cache refill size */
#define CFG_ISIZESHFT   2
#define CFG_ISIZEMASK   (3<<CFG_ISIZESHFT)      /* I-cache refill size */

/*
 * Primary cache mode
 */
#define CFG_C_UNCACHED		2
#define CFG_C_NONCOHERENT	3
#define CFG_C_COHERENTXCL	4
#define CFG_C_COHERENTXCLW	5
#define CFG_C_COHERENTUPD	6

/* 
 * Primary Cache TagLo 
 */
#define TAG_PTAG_MASK           0xffffff00      /* Primary Tag */
#define TAG_PTAG_SHIFT          0x00000008
#define TAG_PSTATE_MASK         0x000000c0      /* Primary Cache State */
#define TAG_PSTATE_SHIFT        0x00000006
#define TAG_PARITY_MASK         0x0000000f      /* Primary Tag Parity */
#define TAG_PARITY_SHIFT        0x00000000


/* 
 * Secondary Cache TagLo 
 */
#define TAG_STAG_MASK           0xffffe000      /* Secondary Tag */
#define TAG_STAG_SHIFT          0x0000000d
#define TAG_SSTATE_MASK         0x00001c00      /* Secondary Cache State */
#define TAG_SSTATE_SHIFT        0x0000000a
#define TAG_VINDEX_MASK         0x00000380      /* Secondary Virtual Index */
#define TAG_VINDEX_SHIFT        0x00000007
#define TAG_ECC_MASK            0x0000007f      /* Secondary Tag ECC */
#define TAG_ECC_SHIFT           0x00000000
#define TAG_STAG_SIZE			19	/* Secondary Tag Width */

/*
 * CacheErr register
 */
#define CACHEERR_TYPE		0x80000000	/* reference type: 
						   0=Instr, 1=Data */
#define CACHEERR_LEVEL		0x40000000	/* cache level:
						   0=Primary, 1=Secondary */
#define CACHEERR_DATA		0x20000000	/* data field:
						   0=No error, 1=Error */
#define CACHEERR_TAG		0x10000000	/* tag field:
						   0=No error, 1=Error */
#define CACHEERR_REQ		0x08000000	/* request type:
						   0=Internal, 1=External */
#define CACHEERR_BUS		0x04000000	/* error on bus:
						   0=No, 1=Yes */
#define CACHEERR_BOTH		0x02000000	/* Data & Instruction error:
						   0=No, 1=Yes */
#define CACHEERR_ECC		0x01000000	/* ECC error :
						   0=No, 1=Yes */
#define CACHEERR_SIDX_MASK	0x003ffff8	/* PADDR(21..3) */
#define CACHEERR_SIDX_SHIFT		 3
#define CACHEERR_PIDX_MASK	0x00000007	/* VADDR(14..12) */
#define CACHEERR_PIDX_SHIFT		12


/*
 * Cache operations
 */
#define Index_Invalidate_I               0x0         /* 0       0 */
#define Index_Writeback_Inv_D            0x1         /* 0       1 */
#define Index_Invalidate_SI              0x2         /* 0       2 */
#define Index_Writeback_Inv_SD           0x3         /* 0       3 */
#define Index_Load_Tag_I                 0x4         /* 1       0 */
#define Index_Load_Tag_D                 0x5         /* 1       1 */
#define Index_Load_Tag_SI                0x6         /* 1       2 */
#define Index_Load_Tag_SD                0x7         /* 1       3 */
#define Index_Store_Tag_I                0x8         /* 2       0 */
#define Index_Store_Tag_D                0x9         /* 2       1 */
#define Index_Store_Tag_SI               0xA         /* 2       2 */
#define Index_Store_Tag_SD               0xB         /* 2       3 */
#define Create_Dirty_Exc_D               0xD         /* 3       1 */
#define Create_Dirty_Exc_SD              0xF         /* 3       3 */
#define Hit_Invalidate_I                 0x10        /* 4       0 */
#define Hit_Invalidate_D                 0x11        /* 4       1 */
#define Hit_Invalidate_SI                0x12        /* 4       2 */
#define Hit_Invalidate_SD                0x13        /* 4       3 */
#define Hit_Writeback_Inv_D              0x15        /* 5       1 */
#define Hit_Writeback_Inv_SD             0x17        /* 5       3 */
#define Fill_I                           0x14        /* 5       0 */
#define Hit_Writeback_D                  0x19        /* 6       1 */
#define Hit_Writeback_SD                 0x1B        /* 6       3 */
#define Hit_Writeback_I                  0x18        /* 6       0 */
#define Lock_I				 0x1C	     /* 7	0 */
#define Lock_D				 0x1D	     /* 7	1 */
#define Hit_Set_Virtual_SI               0x1E        /* 7       2 */
#define Hit_Set_Virtual_SD               0x1F        /* 7       3 */

/*
 * Coprocessor 0 operations
 */
#define	C0_READI  0x1		/* read ITLB entry addressed by C0_INDEX */
#define	C0_WRITEI 0x2		/* write ITLB entry addressed by C0_INDEX */
#define	C0_WRITER 0x6		/* write ITLB entry addressed by C0_RAND */
#define	C0_PROBE  0x8		/* probe for ITLB entry addressed by TLBHI */
#define	C0_ERET	  0x18		/* restore for exception */

/*
 * TLB definitions
 */
#define TLB_ENTRIES   64        /* set to size of largest supported core */

/* END PAL */

#ifdef __cplusplus
}
#endif

#endif /* __INCarchMipsh */
