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

#include "bstd.h"                /* standard types */
#include "brdc_private.h"
#include "bchp_fmisc.h"
#include "bkni.h"
#include "bchp_sun_gisb_arb.h"

BDBG_MODULE(BRDC);

/* Internal constant */
#define BRDC_P_SEMAPHORE_ACQUIRE_DELAY           (1000)

/***************************************************************************
Summary:
	Reset RDC

Description:

Input:
	hRdc - The RDC handle.

Output:

Returns:

****************************************************************************/
BERR_Code BRDC_P_SoftReset
(
	BRDC_Handle   hRdc
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t  ulReg;
	int       i;
	uint32_t  ulRegAddr;
	uint32_t  ulRegConfig;
	uint32_t ulTrigSelect;

	BDBG_ENTER(BRDC_P_SoftReset);

#ifdef BCHP_FMISC_SW_INIT
	/* Write a 1 to the reset bit.*/
	ulReg  = BRDC_P_Read32(hRdc, BCHP_FMISC_SW_INIT);
	ulReg |= BCHP_FIELD_DATA(FMISC_SW_INIT, RDC, 1);
	BRDC_P_Write32(hRdc, BCHP_FMISC_SW_INIT, ulReg);

	/* Write a 0 to reset. */
	ulReg &= ~BCHP_FIELD_DATA(FMISC_SW_INIT, RDC, 1);
	BRDC_P_Write32(hRdc, BCHP_FMISC_SW_INIT, ulReg);
#else
	/* Write a 1 to the reset bit.*/
	ulReg  = BRDC_P_Read32(hRdc, BCHP_FMISC_SOFT_RESET);
	ulReg |= BCHP_FIELD_DATA(FMISC_SOFT_RESET, RDC, 1);
	BRDC_P_Write32(hRdc, BCHP_FMISC_SOFT_RESET, ulReg);

	/* Write a 0 to reset. */
	ulReg &= ~BCHP_FIELD_DATA(FMISC_SOFT_RESET, RDC, 1);
	BRDC_P_Write32(hRdc, BCHP_FMISC_SOFT_RESET, ulReg);
#endif

	/******************
	 * Set known good values for all registers (read/modify/write to keep good default reset values)
	 */
	ulReg = BRDC_P_Read32(hRdc, BCHP_RDC_config);
#if (!BRDC_P_SUPPORT_SEGMENTED_RUL)
	ulReg &= ~BCHP_MASK(RDC_config, same_trigger);
	ulReg |= BCHP_FIELD_DATA(RDC_config, same_trigger, 0);
#else
	ulReg &= ~(
		BCHP_MASK(RDC_config, scb_buffer_enable) |
		BCHP_MASK(RDC_config, trig_arbitration_mode));
	ulReg |=
		BCHP_FIELD_ENUM(RDC_config, scb_buffer_enable, BUF_01_EN) | /* two contexts enabled */
		BCHP_FIELD_ENUM(RDC_config, trig_arbitration_mode, SEGMENTED); /* 0 - convention ; 1 - segmented mode */
#endif
	BRDC_P_Write32(hRdc, BCHP_RDC_config, ulReg);

	/* Get trigger select value. */
	ulTrigSelect = hRdc->aTrigInfo[BRDC_Trigger_UNKNOWN].ulTrigVal;

	/* setup known values for descriptors */
	ulRegAddr = BCHP_FIELD_DATA(RDC_desc_0_addr, addr, 0x0);
	ulRegConfig =
		BCHP_FIELD_DATA(RDC_desc_0_config, count,           0x0          ) |
		BCHP_FIELD_DATA(RDC_desc_0_config, trigger_select,  ulTrigSelect ) |
		BCHP_FIELD_DATA(RDC_desc_0_config, repeat,          0            ) |
		BCHP_FIELD_DATA(RDC_desc_0_config, enable,          0            ) |
#if (BRDC_P_SUPPORT_HIGH_PRIORITY_SLOT)
		BCHP_FIELD_DATA(RDC_desc_0_config, high_priority,   0            ) |
#endif
#if (BRDC_P_SUPPORT_SEGMENTED_RUL) /* no side effect if RUL is traditional; */
		BCHP_FIELD_ENUM(RDC_desc_0_config, segmented,       SEGMENTED    ) |
#endif
		BCHP_FIELD_DATA(RDC_desc_0_config, done,            1            ) |
		BCHP_FIELD_DATA(RDC_desc_0_config, error,           1            ) |
		BCHP_FIELD_DATA(RDC_desc_0_config, dropped_trigger, 1            );

	/* set all descriptors */
	for (i=0; i<32; ++i)
	{
		BKNI_EnterCriticalSection();

		/* write address and config */
		BRDC_P_Write32(hRdc, BCHP_RDC_desc_0_addr + 16 * i,   ulRegAddr);
		BRDC_P_Write32(hRdc, BCHP_RDC_desc_0_config + 16 * i, ulRegConfig);

		BKNI_LeaveCriticalSection();
	}

	/* PR 10095:
	 * RDC can only be used to program BVN registers by default (set at bootup).
	 * Need to enable it for B0 if need to access IFD registers. */
	/* Unmask RDC so it can access registers outside BVN, such as IFD regs. */
	BKNI_EnterCriticalSection();
	ulReg  = BREG_Read32_isr(hRdc->hReg, BCHP_SUN_GISB_ARB_REQ_MASK);
#ifdef BCHP_SUN_GISB_ARB_REQ_MASK_rdc_MASK
	ulReg &= ~BCHP_SUN_GISB_ARB_REQ_MASK_rdc_MASK;
#endif
#ifdef BCHP_SUN_GISB_ARB_REQ_MASK_req_mask_5_MASK
	ulReg &= ~BCHP_SUN_GISB_ARB_REQ_MASK_req_mask_5_MASK;
#endif
	BREG_Write32_isr(hRdc->hReg, BCHP_SUN_GISB_ARB_REQ_MASK, ulReg);
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BRDC_P_SoftReset);
	return err;
}


/***************************************************************************
Summary:
	Private function to get next available slot

Description:

Input:
	hRdc - The RDC handle.

Output:
	pSlotID - The returned slot ID.

Returns:

****************************************************************************/
BERR_Code BRDC_Slot_P_GetNextSlot
(
	BRDC_Handle   hRdc,
	BRDC_SlotId  *pSlotId
)
{
	BERR_Code    err = BERR_SUCCESS;
	BRDC_SlotId  eSlotId;

	BDBG_ENTER(BRDC_Slot_P_GetNextSlot);

	for( eSlotId = BRDC_SlotId_eSlot0; eSlotId < hRdc->ulMaxAvailableSlot; eSlotId++ )
	{
		if( !hRdc->bSlotUsed[eSlotId] )
		{
			*pSlotId = (BRDC_SlotId) eSlotId;
			goto done;
		}
	}

	/* Can't find any slot available */
	err = BERR_TRACE(BRDC_SLOT_ERR_ALL_USED);

done:
	BDBG_LEAVE(BRDC_Slot_P_GetNextSlot);
	return err;
}

/***************************************************************************
Summary:
	Private function to fill in hardware registers for DMA

	This function assumes DMA is already locked if necessary.

Description:

Input:
	hSlot - The slot to activate.
	ui32_trigger - The trigger used to fire the slot.
	bRecurring - Whether to allow multiple firings of the trigger to execute
	             the slot repeatedly.

Output:
	pSlotID - The returned slot ID.

Returns:

****************************************************************************/
BERR_Code BRDC_Slot_P_Write_Registers_isr
(
	BRDC_Slot_Handle hSlot,
	BRDC_Trigger     eRDCTrigger,
	bool             bRecurring,
	bool             ExecuteOnTrigger
)
{
	uint32_t ulRegVal, ulTrigSelect;

	BDBG_ENTER(BRDC_Slot_P_Write_Registers_isr);

	/* Set RDC_desc_x_addr */
	BRDC_Slot_P_Write32(hSlot, BCHP_RDC_desc_0_addr + hSlot->ulRegOffset,
		hSlot->hList->ulAddrOffset);

	/* Set RDC_desc_x_config */
	ulRegVal = BRDC_Slot_P_Read32(hSlot, BCHP_RDC_desc_0_config + hSlot->ulRegOffset);

	/* Get trigger select value. */
	ulTrigSelect = hSlot->hRdc->aTrigInfo[eRDCTrigger].ulTrigVal;

	if( ExecuteOnTrigger )
	{
		ulRegVal &= ~(
			BCHP_MASK(RDC_desc_0_config, enable         ) |
			BCHP_MASK(RDC_desc_0_config, repeat         ) |
			BCHP_MASK(RDC_desc_0_config, trigger_select ) |
#if (BRDC_P_SUPPORT_HIGH_PRIORITY_SLOT)
			BCHP_MASK(RDC_desc_0_config, high_priority  ) |
#endif
			BCHP_MASK(RDC_desc_0_config, count          ));

		ulRegVal |= (
			BCHP_FIELD_DATA(RDC_desc_0_config, enable,         1                          ) |
			BCHP_FIELD_DATA(RDC_desc_0_config, repeat,         bRecurring                 ) |
			BCHP_FIELD_DATA(RDC_desc_0_config, trigger_select, ulTrigSelect               ) |
#if (BRDC_P_SUPPORT_HIGH_PRIORITY_SLOT)
			BCHP_FIELD_DATA(RDC_desc_0_config, high_priority,  hSlot->stSlotSetting.bHighPriority) |
#endif
			BCHP_FIELD_DATA(RDC_desc_0_config, count,          hSlot->hList->ulEntries -1));

		BRDC_Slot_P_Write32(hSlot, BCHP_RDC_desc_0_config + hSlot->ulRegOffset, ulRegVal);
	}
	else
	{
		/* previously not enabled? */
		if (!BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, enable))
		{
			/* we are forcing a descriptor that doesn't have a trigger
			   so we should set the trigger to an undefined value (so
			   we can later turn on the enable) and turn off the repeat so this
			   slot is executed only once */
			/* Get trigger select value. */
			ulTrigSelect = hSlot->hRdc->aTrigInfo[BRDC_Trigger_UNKNOWN].ulTrigVal;

			ulRegVal &= ~(
				BCHP_MASK(RDC_desc_0_config, trigger_select ) |
				BCHP_MASK(RDC_desc_0_config, repeat         ));
			ulRegVal |= (
				BCHP_FIELD_DATA(RDC_desc_0_config, trigger_select, ulTrigSelect) |
				BCHP_FIELD_DATA(RDC_desc_0_config, repeat,         0));
		}

		/* enable descriptor and update count */
		ulRegVal &= ~(
			BCHP_MASK(RDC_desc_0_config, enable ) |
#if (BRDC_P_SUPPORT_HIGH_PRIORITY_SLOT)
			BCHP_MASK(RDC_desc_0_config, high_priority  ) |
#endif
			BCHP_MASK(RDC_desc_0_config, count  ));

		ulRegVal |= (
			BCHP_FIELD_DATA(RDC_desc_0_config, enable, 1                          ) |
#if (BRDC_P_SUPPORT_HIGH_PRIORITY_SLOT)
			BCHP_FIELD_DATA(RDC_desc_0_config, high_priority,  hSlot->stSlotSetting.bHighPriority) |
#endif
			BCHP_FIELD_DATA(RDC_desc_0_config, count,  hSlot->hList->ulEntries - 1));

		BRDC_Slot_P_Write32(hSlot, BCHP_RDC_desc_0_config + hSlot->ulRegOffset, ulRegVal);

		/* Set RDC_desc_x_immediate */
		ulRegVal = BCHP_FIELD_DATA(RDC_desc_0_immediate, trigger, 1 );
		BRDC_Slot_P_Write32(hSlot, BCHP_RDC_desc_0_immediate + hSlot->ulRegOffset, ulRegVal);
	}

	BDBG_LEAVE(BRDC_Slot_P_Write_Registers_isr);
	return BERR_SUCCESS;

}

/***************************************************************************
Summary:
	Isr function to acquire semaphore from slot.

Description:

Input:
	hSlot - The slot to acquire semaphore from.
	ulRegOffset - Offset to the slot's registers.

Output:

Returns:

****************************************************************************/
BERR_Code BRDC_P_AcquireSemaphore_isr
(
	BRDC_Handle      hRdc,
	BRDC_List_Handle hList,
	BRDC_SlotId      eSlotId
)
{
	int      iDMABusy = 0;
	bool     bDMABusy;
	uint32_t ulRegVal, ulRegOffset;

	/* calculate offset for this slot */
	ulRegOffset = 16 * (eSlotId - BRDC_SlotId_eSlot0);

	/* If DMA is not busy, this read will acquire the semaphore */
	ulRegVal = BRDC_P_Read32(hRdc, BCHP_RDC_desc_0_lock + ulRegOffset);

	/* All RDC_desc_x_lock bit definitions are same */
	bDMABusy = (BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_lock, semaphore) != 0);

	/* Wait to get semaphore to lock DMA */
	while( bDMABusy )
	{
		/* PR13108: This is the very rare case that we can't acquire
		 * semaphore for the slot.
		 * The common belief is that a RUL execution time should be maxed by
		 * 1/2000th of a second. Therefore the max delay caused by a loss of
		 * semaphore should be the same plus some delta just in case.
		 * In this case, choose total dealy = 1/2000 sec + 100 us (delta) =
		 * 600 us, for BRDC_P_SEMAPHORE_ACQUIRE_DELAY tries. */
		BKNI_Delay(1);

		if (BRDC_P_SEMAPHORE_ACQUIRE_DELAY == ++iDMABusy)
		{
			/* could not acquire semaphore within a reasonable amount of time */
			BDBG_ERR(( "Cannot acquire semaphore" ));
			BRDC_P_DumpSlot_isr(hRdc, hList, eSlotId);
			return BERR_TRACE(BERR_TIMEOUT);
		}

		/* If DMA is not busy, this read will acquire the semaphore */
		ulRegVal = BRDC_P_Read32(hRdc, BCHP_RDC_desc_0_lock + ulRegOffset);

		/* All RDC_desc_x_lock bit definitions are same */
		bDMABusy = (BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_lock, semaphore) != 0);

	}

	/* semaphore acquired */
	return BERR_SUCCESS;
}

/***************************************************************************
Summary:
	Release semaphore to hardware

Description:

Input:
	hSlot - The slot to release semaphore.
	ulRegOffset - Offset to the slot's registers.

Output:

Returns:

****************************************************************************/
void BRDC_P_ReleaseSemaphore_isr
(
	BRDC_Handle      hRdc,
	BRDC_List_Handle hList,
	BRDC_SlotId      eSlotId
)
{
	uint32_t ulRegVal;
	uint32_t ulRegOffset;

	BSTD_UNUSED(hList);

	/* calculate offset for this slot */
	ulRegOffset = 16 * (eSlotId - BRDC_SlotId_eSlot0);

	/* Release semaphore. Write 1 to clear. */
	ulRegVal = BCHP_MASK(RDC_desc_0_lock, semaphore);
	BRDC_P_Write32(hRdc, BCHP_RDC_desc_0_lock + ulRegOffset, ulRegVal);
}

void BRDC_P_DumpSlot_isr
(
	BRDC_Handle      hRdc,
	BRDC_List_Handle hList,
	BRDC_SlotId      eSlotId
)
{
	uint32_t  ulRegOffset;
	uint32_t  ulRegVal;

	BSTD_UNUSED(hList);

	/* determine offset of registers for this slot */
	ulRegOffset = 16 * (eSlotId - BRDC_SlotId_eSlot0);

	/* header */
	BDBG_MSG(("-------------------------------\n"));

	/* read and display address register */
	ulRegVal = BRDC_P_Read32(hRdc, BCHP_RDC_desc_0_addr + ulRegOffset);
	BDBG_MSG(("RDC_desc_%d_addr\n"
		"\taddr: 0x%08x\n",
		eSlotId,
		BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_addr, addr)));

	/* read and display config register */
	ulRegVal = BRDC_P_Read32(hRdc, BCHP_RDC_desc_0_config + ulRegOffset);
	BDBG_MSG(("RDC_desc_%d_config\n"
		"\tcount:           %d\n"
		"\ttrigger_select:  %d\n"
		"\trepeat:          %d\n"
		"\tenable:          %d\n"
		"\tdone:            %d\n"
		"\tbusy:            %d\n"
		"\terror:           %d\n"
		"\tdropped_trigger: %d\n"
		"\tlock_rd:         %d\n",
		eSlotId,
		BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, count),
		BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, trigger_select),
		BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, repeat),
		BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, enable),
		BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, done),
		BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, busy),
		BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, error),
		BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, dropped_trigger),
		BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, lock_rd)));

	/* contents of RUL */
	BDBG_MSG(("See RUL dump for RUL contents. \n"));
	BSTD_UNUSED(ulRegVal);
}

/* end of file */
