/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BDMA_PRIV_H__
#define BDMA_PRIV_H__

#include "bdma.h"
#include "bdma_errors.h"
#include "bkni_multi.h"

#ifdef __cplusplus
extern "C" {
#endif


#if (BCHP_CHIP == 7400)
#if (BCHP_VER>=BCHP_VER_B0)
#define BDMA_P_MEM_2_ADDR_BASE                (0x70000000)
#define BDMA_P_MEM_2_ADDR_SIZE                (64*1024*1024)
#endif
#define BDMA_P_MEM_1_ADDR_BASE                (0x60000000)
#define BDMA_P_MEM_1_ADDR_SIZE                (64*1024*1024)
#elif (BCHP_CHIP==7405) || (BCHP_CHIP == 7325)
#define BDMA_P_MEM_1_ADDR_BASE                (0x60000000)
#define BDMA_P_MEM_1_ADDR_SIZE                (64*1024*1024)
#elif (BCHP_CHIP==3563)
#define BDMA_P_MEM_1_ADDR_BASE                (0x60000000)
#define BDMA_P_MEM_1_ADDR_SIZE                (32*1024*1024)
#endif

#if (BCHP_CHIP==7400) || (BCHP_CHIP==7401)
#define BDMA_P_MEM_ADDR_SIZE                  (7<<28)
#else
#define BDMA_P_MEM_ADDR_SIZE                  (1<<28)
#endif

/* number of mem ctrl */
#if (BCHP_CHIP==7400) && (BCHP_VER>=BCHP_VER_B0)
#define BDMA_P_SUPPORT_MEM_CTRL                3
#elif (BCHP_CHIP==7405) || (BCHP_CHIP == 7325) || (BCHP_CHIP == 7335)
#define BDMA_P_SUPPORT_MEM_CTRL                2
#elif (BCHP_CHIP==3563)
#define BDMA_P_SUPPORT_MEM_CTRL                2
#else
#define BDMA_P_SUPPORT_MEM_CTRL                1
#endif

#if ((BCHP_CHIP==7440) && (BCHP_VER>=BCHP_VER_B0)) || \
    (BCHP_CHIP == 7601) || (BCHP_CHIP == 7635) || (BCHP_CHIP == 7630) || (BCHP_CHIP == 7640) \
    || ((BCHP_CHIP == 35230) && (BCHP_VER==BCHP_VER_C0)) \
    || ((BCHP_CHIP ==  7550) && (BCHP_VER>=BCHP_VER_B0)) \
    || (BCHP_CHIP ==  35233) 
#define BDMA_P_SUPPORT_SHARF_DMA_ENGINE        2
#endif

/* number of mem dma HW engines */
#if (BCHP_CHIP==7400) && (BCHP_VER>=BCHP_VER_B0)
#define BDMA_P_SUPPORT_MEM_DMA_ENGINE          2
#elif(BCHP_CHIP==7550) 
#define BDMA_P_SUPPORT_MEM_DMA_ENGINE          0
#else
#define BDMA_P_SUPPORT_MEM_DMA_ENGINE          1
#endif

/* number of pci dma HW engines */
#if (BCHP_CHIP == 3548) || (BCHP_CHIP == 3556)
#define BDMA_P_SUPPORT_PCI_DMA_ENGINE          0
#else
#define BDMA_P_SUPPORT_PCI_DMA_ENGINE          1
#endif

/* register name support mutiple engine or not */
#if ((BCHP_CHIP==3548) || (BCHP_CHIP==3556) || \
     (BCHP_CHIP==3563) || (BCHP_CHIP==7118) || (BCHP_CHIP==7401) || (BCHP_CHIP==7403) || \
     (BCHP_CHIP==7440) || ((BCHP_CHIP==7400) && (BCHP_VER<BCHP_VER_B0)) || \
     (BCHP_CHIP==7601) || (BCHP_CHIP==7635)) || (BCHP_CHIP==7630)
#define BDMA_P_SUPPORT_MEM_DMA_REG_NAME_i      0
#else
#define BDMA_P_SUPPORT_MEM_DMA_REG_NAME_i      1
#endif

/* Scatter-Gather */
#if ((BCHP_CHIP==3563) || (BCHP_CHIP==7118) || (BCHP_CHIP==7401) || (BCHP_CHIP==7403) || \
     ((BCHP_CHIP==7400) && (BCHP_VER<BCHP_VER_B0)) || \
     ((BCHP_CHIP==7440) && (BCHP_VER<BCHP_VER_B0)))
#define BDMA_P_SUPPORT_MEM_DMA_SCATTER_GATHER  0
#else
#define BDMA_P_SUPPORT_MEM_DMA_SCATTER_GATHER  1
#endif

/* auto-pend so that scatter-gather link list can be appended with non-scatter-gather linked list */
#if ((BCHP_CHIP==3563) || (BCHP_CHIP==7118) || (BCHP_CHIP==7401) || (BCHP_CHIP==7403) || \
     ((BCHP_CHIP==7400) && (BCHP_VER<BCHP_VER_D0)) || (BCHP_CHIP==7440) || \
     ((BCHP_CHIP==7405) && (BCHP_VER<BCHP_VER_B0)))
#define BDMA_P_SUPPORT_MEM_DMA_AUTO_APPEND     0
#else
/* XXX ??? AUTO_APPEND currently cause bad data with dyn link */
#define BDMA_P_SUPPORT_MEM_DMA_AUTO_APPEND     0
#endif

/* 41 key slots */
#if ((BCHP_CHIP==3548) || (BCHP_CHIP==3556) || (BCHP_CHIP==3560) || \
	 (BCHP_CHIP==3563) || (BCHP_CHIP==7038) || \
	 (BCHP_CHIP==7118) || (BCHP_CHIP==7325) || (BCHP_CHIP==7335) || \
	 (BCHP_CHIP==7400) || (BCHP_CHIP==7401) || (BCHP_CHIP==7403) || \
	 (BCHP_CHIP==7438) || (BCHP_CHIP==7440) || \
	 ((BCHP_CHIP==7405) && (BCHP_VER<BCHP_VER_B0)))
#define BDMA_P_SUPPORT_MEM_DMA_41_KEY_SLOTS    0
#else
#define BDMA_P_SUPPORT_MEM_DMA_41_KEY_SLOTS    1
#endif

#define BDMA_P_WATER_MARK_SG_OPEN              (0x2A)
#define BDMA_P_WATER_MARK_SG_CLOSE             (0x33)
#define BDMA_P_WATER_MARK_MASK                 (0x3F)


/* code configure */
#define BDMA_P_USE_CURR_DESC_ADDR	           1

#define BDMA_P_CHECK_BASIC                     0
#define BDMA_P_CHECK_NEXT_DESC                 0
#define BDMA_P_CHECK_SCATTER_GATHER            0
#define BDMA_P_CHECK_CORRUPT                   0

#define BDMA_P_SHOW_DESC                       0
#define BDMA_P_SHOW_SG_OPEN                    0
#define BDMA_P_SHOW_DYN_LINK_FAIL              0


/***************************************************************************
 *
 *  Generic utility macro
 *
 ****************************************************************************/

/* This macro take the check for a validity of a handle, and
 * cast to context pointer.
 */
#define BDMA_P_GENERIC_GET_CONTEXT(handle, context, structname) \
{ \
	if(!(handle) || \
	   (((structname*)(handle))->ulBlackMagic != \
	    (sizeof(structname) | 0xbac98700))) \
	{ \
		BDBG_ERR(("NULL context handle\n")); \
		(context) = NULL; \
	} \
	else \
	{ \
		(context) = (handle); \
	} \
}

/* This macro set the black magic for later handle validation
 */
#define BDMA_P_GENERIC_SET_BLACK_MAGIC(handle, structname) \
{ \
	((structname*)(handle))->ulBlackMagic = sizeof(structname) | 0xbac98700; \
}

/* This macro ends the function if error "result" is seen
 */
#define BDMA_P_END_IF_ERR(result, done_lable) \
	if ( BERR_SUCCESS != (result)) \
	{\
		goto done_lable;  \
	}

/* This macro shred the garbage
 */
#if BDBG_DEBUG_BUILD
#define BDMA_P_GENERIC_DESTROY_CONTEXT(struct_ptr, structname) \
{ \
	BKNI_Memset((void*)struct_ptr, 0xA3, sizeof(structname)); \
	BKNI_Free((void*)struct_ptr); \
	struct_ptr = NULL; \
}
#else
#define BDMA_P_GENERIC_DESTROY_CONTEXT(struct_ptr, structname) \
{ \
	BKNI_Free((void*)struct_ptr); \
}
#endif

/***************************************************************************
 *
 *  DMA main context utility macro
 *
 ****************************************************************************/

#define BDMA_P_MAIN_GET_CONTEXT(handle, context) \
	BDMA_P_GENERIC_GET_CONTEXT(handle, context, BDMA_P_Context)

#define BDMA_P_MAIN_SET_BLACK_MAGIC(handle) \
	BDMA_P_GENERIC_SET_BLACK_MAGIC(handle, BDMA_P_Context)

#define BDMA_P_MAIN_DESTROY_CONTEXT(struct_ptr) \
	BDMA_P_GENERIC_DESTROY_CONTEXT(struct_ptr, BDMA_P_Context)


/***************************************************************************
 *
 *  Main context struct and utility functions
 *
 ****************************************************************************/

/***************************************************************************
 * BDMA module Context
 */
typedef struct BDMA_P_Context
{
	uint32_t   ulBlackMagic;   /* Black magic for handle validation */

	/* handed down from up level sw */
	BCHP_Handle    hChip;
	BREG_Handle    hRegister;
	BMEM_Handle    hMemory;
	BINT_Handle    hInterrupt;

	/* protect dma API calls from diff threads */
	BKNI_MutexHandle hMutex;

#if BDMA_P_SUPPORT_MEM_DMA_ENGINE
	BDMA_Mem_Handle hMemDma[BDMA_P_SUPPORT_MEM_DMA_ENGINE];
#endif

#if BDMA_P_SUPPORT_PCI_DMA_ENGINE
	BDMA_Pci_Handle hPciDma[BDMA_P_SUPPORT_PCI_DMA_ENGINE];
#endif

#ifdef BDMA_P_SUPPORT_SHARF_DMA_ENGINE
	BDMA_Mem_Handle hSharfDma[BDMA_P_SUPPORT_SHARF_DMA_ENGINE];
#endif

} BDMA_P_Context;

/***************************************************************************
 * BDMA private function to get diff handles passed in as module open
 * no paramter validation
 */
BCHP_Handle BDMA_P_GetChipHandle( BDMA_Handle  hDma );
BREG_Handle BDMA_P_GetRegisterHandle( BDMA_Handle  hDma );
BMEM_Handle BDMA_P_GetMemoryHandle( BDMA_Handle  hDma );
BINT_Handle BDMA_P_GetInterruptHandle( BDMA_Handle  hDma );

/***************************************************************************
 * BDMA private function to set sub-module handle
 * no paramter validation
 * Note that BDMA_P_GetMemDmaHandle and BDMA_P_SetMemDmaHandle are for
 * backward compatible.  For 7400 B0 chip, these 2 functions will set or
 * return the handle of the last MEM engine created.
 */
#if BDMA_P_SUPPORT_MEM_DMA_ENGINE
BDMA_Mem_Handle BDMA_P_GetMemDmaHandle(	BDMA_Handle hDma, BDMA_MemDmaEngine eEngine );
void BDMA_P_SetMemDmaHandle( BDMA_Handle hDma, BDMA_MemDmaEngine eEngine, BDMA_Mem_Handle hMemDma );
#endif

#if BDMA_P_SUPPORT_PCI_DMA_ENGINE
BDMA_Pci_Handle BDMA_P_GetPciDmaHandle(	BDMA_Handle hDma, BDMA_PciDmaEngine eEngine );
void BDMA_P_SetPciDmaHandle( BDMA_Handle hDma, BDMA_PciDmaEngine eEngine, BDMA_Pci_Handle hPciDma );
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BDMA_PRIV_H__ */

/* end of file */
