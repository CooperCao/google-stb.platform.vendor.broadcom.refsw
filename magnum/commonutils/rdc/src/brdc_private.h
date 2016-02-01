/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
#ifndef BRDC__PRIVATE_H__
#define BRDC__PRIVATE_H__

#include "blst_list.h"           /* Link list support */
#include "brdc.h"
#include "brdc_dbg.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
 * Defines
 ***************************************************************************/

#define BRDC_P_Read32(hRdc, reg)                BREG_Read32(hRdc->hReg, reg)
#define BRDC_P_Write32(hRdc, reg, data)         BREG_Write32(hRdc->hReg, reg, data)

#define BRDC_Slot_P_Read32(hSlot, reg)          BRDC_P_Read32(hSlot->hRdc, reg)
#define BRDC_Slot_P_Write32(hSlot, reg, data)   BRDC_P_Write32(hSlot->hRdc, reg, data)

/* Number of rul to capture. */
#define BRDC_P_MAX_COUNT                        (1024*1024)

/* Null int id */
#define BRDC_P_NULL_BINTID                      ((BINT_Id)(-1))

/* Macro to make a RDC trigger.*/
#define BRDC_P_TRIGGER(trigger)                  BCHP_RDC_desc_0_config_trigger_select_##trigger

/* Unkown trigger value. */
#define BRDC_P_TRIGGER_UNKNOWN_VAL              (0x7f)

/* RDC general purpose registers address*/
#define BRDC_P_NO_TRACKING_ADDR                  UINT32_C(-1)

/***************************************************************************
Summary:
	Scratch Registers wrapper

Description:
	The following moduals have reserved scratch registers:
		RDC: the first 27 or 32 scratch registers are reserved internally for
		     slot execution tracking;
		VDC: the last 3 scratch registers are reserved for VDC format switch,
			 which are BRDC_SCRATCH_REG_END ~ (BRDC_SCRATCH_REG_END-2);
****************************************************************************/
#define BRDC_P_NUM_OF_SLOTS            (32)
#define BRDC_P_MAX_NUM_OF_DISPLAYS     (3)

#if BCHP_RDC_scratch_i_ARRAY_BASE /* 7403 and beyond */
	/* VBI: the last 4 scratch reigsters are reserved to coordinate programming
	 * of VBI encoder control registers between BVBI and BVDC. (see bavc.h
	 * for details) */
	#define BRDC_P_NUM_OF_SCRATCH_FOR_VBI  (4)

	/* the first 32 scratch registers are reserved for RDC slot execution tracking */
	#define BRDC_P_SCRATCH_REG_START   (0)
	#define BRDC_P_SCRATCH_REG_END     (BCHP_RDC_scratch_i_ARRAY_END - BRDC_P_NUM_OF_SCRATCH_FOR_VBI)

	#define BRDC_P_SCRATCH_REG_ADDR(varx) \
		(BCHP_RDC_scratch_i_ARRAY_BASE + ((varx) * sizeof(uint32_t)))

	/* track all slots */
	#define BRDC_P_TRACK_REG_ADDR(x) BRDC_P_SCRATCH_REG_ADDR(x)

	/* BRDC_P_SCRATCH_FIRST_AVAILABLE needs to exclude RDC internally reserved
	   slot-tracking registers;
	   we also need to reserve at least 3 scratch registers for VDC display format
	   switch usage; */
	#define BRDC_P_SCRATCH_FIRST_AVAILABLE  (BRDC_P_NUM_OF_SLOTS)
#else /* 3563 and previous chipsets */
	/* the first 27 or 32 scratch registers are reserved for RDC slot execution
	 * tracking */
	#define BRDC_P_SCRATCH_REG_START        (BRDC_Variable_Max)
	#define BRDC_P_SCRATCH_REG_END          (BCHP_RDC_data_i_ARRAY_END)

	#define BRDC_P_SCRATCH_REG_ADDR(varx) \
		(BCHP_RDC_data_i_ARRAY_BASE + ((varx) * sizeof(uint32_t)))

	/* BRDC_P_SCRATCH_FIRST_AVAILABLE needs to exclude RDC internally reserved
	   slot-tracking registers;
	   we also need to reserve at least 3 scratch registers for VDC display format
	   switch usage; */
	#if ((BRDC_P_SCRATCH_REG_END) >= (BRDC_P_SCRATCH_REG_START+BRDC_P_NUM_OF_SLOTS+BRDC_P_MAX_NUM_OF_DISPLAYS)) /* 3563 */
	#define BRDC_P_TRACK_REG_ADDR(x) BRDC_P_SCRATCH_REG_ADDR(x)
	#define BRDC_P_SCRATCH_FIRST_AVAILABLE  (BRDC_P_SCRATCH_REG_START+BRDC_P_NUM_OF_SLOTS)
	#else /* previous chipsets: 7038/7438/7118/7401/7400A0/3560 */
	#define BRDC_P_TRACK_REG_ADDR(x) \
		(((x) > BRDC_P_SCRATCH_REG_END-BRDC_P_MAX_NUM_OF_DISPLAYS)? \
		BRDC_P_NO_TRACKING_ADDR : BRDC_P_SCRATCH_REG_ADDR(x))
	#define BRDC_P_SCRATCH_FIRST_AVAILABLE  (BRDC_P_SCRATCH_REG_END-BRDC_P_MAX_NUM_OF_DISPLAYS+1)
	#endif
#endif

#if (BRDC_P_SCRATCH_FIRST_AVAILABLE+BRDC_P_MAX_NUM_OF_DISPLAYS-1)>BRDC_P_SCRATCH_REG_END
#error "RDC doesn't have enough scratch registers! Please adjust MACRO above!"
#endif

#ifdef BCHP_RDC_desc_0_config_segmented_MASK
#define BRDC_P_SUPPORT_SEGMENTED_RUL       (1)
#else
#define BRDC_P_SUPPORT_SEGMENTED_RUL       (0)
#endif

#ifdef BCHP_RDC_desc_0_config_high_priority_MASK
#define BRDC_P_SUPPORT_HIGH_PRIORITY_SLOT  (1)
#else
#define BRDC_P_SUPPORT_HIGH_PRIORITY_SLOT  (0)
#endif


/***************************************************************************
 * Data Structure
 ***************************************************************************/

/***************************************************************************
 * BRDC_P_Slot_Head
 *      Head of the double Link List for slot
 ***************************************************************************/
typedef struct BRDC_P_Slot_Head BRDC_P_Slot_Head;
BLST_D_HEAD(BRDC_P_Slot_Head, BRDC_P_Slot_Handle);

/***************************************************************************
 * BRDC_P_Handle
 ***************************************************************************/
typedef struct BRDC_P_Handle
{
	BDBG_OBJECT(BRDC_RDC)

	BREG_Handle                        hReg;             /* Register module handle */
	BCHP_Handle                        hChp;             /* Chip module handle */
	BMMA_Heap_Handle                   hMem;             /* Memory module handle */
	BRDC_Settings                      stRdcSettings;    /* Global RDC Settings */
	const BRDC_TrigInfo               *aTrigInfo;        /* Contain this chip trigger information. */

	/* Scratch registers use flags */
	bool                               abScratchRegUsed[BRDC_P_SCRATCH_REG_END - BRDC_P_SCRATCH_FIRST_AVAILABLE + 1];
	bool                               bSlotUsed[BRDC_SlotId_eSlotMAX];
	BRDC_Slot_Handle                   apSlot[BRDC_SlotId_eSlotMAX];
	uint32_t                           ulMaxAvailableSlot;
#ifdef BRDC_USE_CAPTURE_BUFFER
	BRDC_DBG_CaptureBuffer             captureBuffer;
#endif

	/* RDC blockout */
	BRDC_List_Handle                   hRdcBlockOutList; /* prealloced RUL list for RDC block out */
	BRDC_BlockOut                      astBlockOut[BRDC_MAX_RDC_BLOCKOUT_LIST_COUNT];
	bool                               bRdcBlockOutEnabled;
} BRDC_P_Handle;


/***************************************************************************
 * BRDC_P_Slot_Handle
 ***************************************************************************/
typedef struct BRDC_P_Slot_Handle
{
	BDBG_OBJECT(BRDC_SLT)

	BRDC_Handle        hRdc;         /* Parent handle */
	BRDC_List_Handle   hList;        /* RUL list handle */

	bool               bRecurring;   /* Is it recurring? */
	BRDC_SlotId        eSlotId;      /* RDMA descriptor Id */
	BRDC_Trigger       eRDCTrigger;  /* Trigger */
	BINT_Id            SlotIntId;    /* L2 interrupt id of this slot. */
	uint32_t           ulRegOffset;  /* Byte offset from slot 0 */

	/* Keep track if a list has been executed when assigned to a slot. */
	bool               bTrackExecution;   /* Enable keep track of execution. */
	uint32_t           ulTrackCount;      /* SW updates this. */
	uint32_t           ulTrackRegAddr;    /* HW updates this. */

	/* store the desc_config setting */
	uint32_t          *pulRulConfigPrevVal; /* point to the previous RUL's config value */
	uint32_t          *pulRulConfigVal; /* point to the current RUL's config value */

	BRDC_Slot_Settings  stSlotSetting;

	BLST_D_ENTRY(BRDC_P_Slot_Handle)  link;         /* doubly-linked list support */

} BRDC_P_Slot_Handle;


/***************************************************************************
 * BRDC_P_List_Handle
 ***************************************************************************/
typedef struct BRDC_P_List_Handle
{
	BDBG_OBJECT(BRDC_LST)

	BRDC_Handle         hRdc;              /* Parent handle */

	BMMA_Block_Handle   hRULBlock;	       /* Managed memory block */
	uint32_t           *pulRULAddr;        /* RUL address */
	uint32_t            ulAddrOffset;      /* Device offset address */
	uint32_t            ulEntries;         /* Number of entries in slot */
	uint32_t            ulMaxEntries;      /* Max number of entries */
	bool                bLastExecuted;     /* Check if last list assignment executed. */

	uint32_t            ulNumSlotAssigned; /* Number of slots the list assigned to */
	BRDC_P_Slot_Head   *pSlotAssigned;     /* Double link list to keep track of which slots the list assigned to */

	/* for RDC list debugging */
	BRDC_DBG_ListEntry  eNextEntry;
	uint32_t           *pulCurListAddr;
	uint32_t            ulNumEntries;
	uint32_t            ulCurrCommand;
	int                 iCommandIndex;
	int                 iDataCount;

} BRDC_P_List_Handle;


/***************************************************************************
 * Functions
 ***************************************************************************/
BERR_Code BRDC_P_SoftReset
	( BRDC_Handle                      hRdc );

BERR_Code BRDC_Slot_P_GetNextSlot
	( BRDC_Handle                      hRdc,
	  BRDC_SlotId                     *pSlotId );

BERR_Code BRDC_Slot_P_Write_Registers_isr
	( BRDC_Slot_Handle                 hSlot,
	  BRDC_Trigger                     eRDCTrigger,
	  bool                             bRecurring,
	  bool                             bExecuteOnTrigger );

BERR_Code BRDC_P_AcquireSemaphore_isr
	( BRDC_Handle                      hRdc,
	  BRDC_List_Handle                 hList,
	  BRDC_SlotId                      eSlotId );

void BRDC_P_ReleaseSemaphore_isr
	( BRDC_Handle                      hRdc,
	  BRDC_List_Handle                 hList,
	  BRDC_SlotId                      eSlotId );

void BRDC_P_DumpSlot_isr
	( BRDC_Handle                      hRdc,
	  BRDC_List_Handle                 hList,
	  BRDC_SlotId                      eSlotId );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BRDC__PRIVATE_H__ */

/* end of file */
