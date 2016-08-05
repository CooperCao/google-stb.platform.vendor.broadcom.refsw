/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
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
    uint32_t  ulRegOffset;
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
    ulRegOffset = BCHP_RDC_desc_1_addr - BCHP_RDC_desc_0_addr;
    ulRegConfig =
#if BCHP_RDC_desc_0_config_count
        BCHP_FIELD_DATA(RDC_desc_0_config, count,           0            ) |
#endif
        BCHP_FIELD_DATA(RDC_desc_0_config, trigger_select,  ulTrigSelect ) |
        BCHP_FIELD_DATA(RDC_desc_0_config, repeat,          0            ) |
        BCHP_FIELD_DATA(RDC_desc_0_config, enable,          0            )
#if (BRDC_P_SUPPORT_HIGH_PRIORITY_SLOT)
      | BCHP_FIELD_DATA(RDC_desc_0_config, high_priority,   0            )
#endif
#if (BRDC_P_SUPPORT_SEGMENTED_RUL) /* no side effect if RUL is traditional; */
      | BCHP_FIELD_ENUM(RDC_desc_0_config, segmented,       SEGMENTED    )
#endif
#if BCHP_RDC_desc_0_config_done_MASK
      | BCHP_FIELD_DATA(RDC_desc_0_config, done,            1            ) |
        BCHP_FIELD_DATA(RDC_desc_0_config, error,           1            ) |
        BCHP_FIELD_DATA(RDC_desc_0_config, dropped_trigger, 1            )
#endif
        ;

    /* set all descriptors */
    for (i=0; i<32; ++i)
    {
        BKNI_EnterCriticalSection();

        /* write address and config */
        BRDC_P_Write32(hRdc, BCHP_RDC_desc_0_addr + ulRegOffset * i, 0);
        BRDC_P_Write32(hRdc, BCHP_RDC_desc_0_config + ulRegOffset * i, ulRegConfig);

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
            BCHP_MASK(RDC_desc_0_config, trigger_select )
#if (BRDC_P_SUPPORT_HIGH_PRIORITY_SLOT)
            | BCHP_MASK(RDC_desc_0_config, high_priority  )
#endif
#if BCHP_RDC_desc_0_config_count_MASK
            | BCHP_MASK(RDC_desc_0_config, count          )
#endif
            );

        ulRegVal |= (
            BCHP_FIELD_DATA(RDC_desc_0_config, enable,         1                          ) |
            BCHP_FIELD_DATA(RDC_desc_0_config, repeat,         bRecurring                 ) |
            BCHP_FIELD_DATA(RDC_desc_0_config, trigger_select, ulTrigSelect               )
#if (BRDC_P_SUPPORT_HIGH_PRIORITY_SLOT)
            | BCHP_FIELD_DATA(RDC_desc_0_config, high_priority, hSlot->stSlotSetting.bHighPriority)
#endif
#if BCHP_RDC_desc_0_config_count_MASK
            | BCHP_FIELD_DATA(RDC_desc_0_config, count,      hSlot->hList->ulEntries -1)
#endif
            );

        BRDC_Slot_P_Write32(hSlot, BCHP_RDC_desc_0_config + hSlot->ulRegOffset, ulRegVal);

#if BCHP_RDC_desc_0_count
        ulRegVal = BCHP_FIELD_DATA(RDC_desc_0_count, count,  hSlot->hList->ulEntries -1);
        BRDC_Slot_P_Write32(hSlot, BCHP_RDC_desc_0_count + hSlot->ulRegOffset, ulRegVal);
#endif
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
            BCHP_MASK(RDC_desc_0_config, enable )
#if (BRDC_P_SUPPORT_HIGH_PRIORITY_SLOT)
            | BCHP_MASK(RDC_desc_0_config, high_priority  )
#endif
#if BCHP_RDC_desc_0_config_count_MASK
            | BCHP_MASK(RDC_desc_0_config, count          )
#endif
            );

        ulRegVal |= (
            BCHP_FIELD_DATA(RDC_desc_0_config, enable, 1                          )
#if (BRDC_P_SUPPORT_HIGH_PRIORITY_SLOT)
            | BCHP_FIELD_DATA(RDC_desc_0_config, high_priority,  hSlot->stSlotSetting.bHighPriority)
#endif
#if BCHP_RDC_desc_0_config_count_MASK
            | BCHP_FIELD_DATA(RDC_desc_0_config, count,          hSlot->hList->ulEntries -1)
#endif
            );

        BRDC_Slot_P_Write32(hSlot, BCHP_RDC_desc_0_config + hSlot->ulRegOffset, ulRegVal);


#if BCHP_RDC_desc_0_count
        ulRegVal = BCHP_FIELD_DATA(RDC_desc_0_count, count,   hSlot->hList->ulEntries -1);
        BRDC_Slot_P_Write32(hSlot, BCHP_RDC_desc_0_count + hSlot->ulRegOffset, ulRegVal);
#endif

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
#if BCHP_RDC_desc_0_lock
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
#else /* TODO: add new RDC support */
    BSTD_UNUSED(hRdc);
    BSTD_UNUSED(hList);
    BSTD_UNUSED(eSlotId);
#endif

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
#if BCHP_RDC_desc_0_lock
    uint32_t ulRegVal;
    uint32_t ulRegOffset;

    BSTD_UNUSED(hList);

    /* calculate offset for this slot */
    ulRegOffset = 16 * (eSlotId - BRDC_SlotId_eSlot0);

    /* Release semaphore. Write 1 to clear. */
    ulRegVal = BCHP_MASK(RDC_desc_0_lock, semaphore);
    BRDC_P_Write32(hRdc, BCHP_RDC_desc_0_lock + ulRegOffset, ulRegVal);
#else /* TODO: add new RDC support */
    BSTD_UNUSED(hRdc);
    BSTD_UNUSED(hList);
    BSTD_UNUSED(eSlotId);
#endif
}

/***************************************************************************
Summary:
    Acquire a synchronizer from pool

Description:

Input:
    hRdc - The instance handle to acquire synchronizer from.

Output:
    pulId - The synchronizer ID. Note (uint32_t)(-1) will be returned if no free synchronizer is available.

Returns:

See Also:
    BRDC_P_ReleaseSync
    BRDC_P_ArmSync_isr
****************************************************************************/
BERR_Code BRDC_P_AcquireSync
    ( BRDC_Handle                      hRdc,
      uint32_t                        *pulId )
{
#ifdef BCHP_RDC_sync_0_arm
    uint32_t i;
    uint32_t ulReg;
    uint32_t ulAddr;
    BERR_Code rc = BERR_SUCCESS;
    BDBG_ENTER(BRDC_P_AcquireSync);

    BDBG_ASSERT(hRdc);
    BDBG_ASSERT(pulId);

    for(i = 0; i < BRDC_P_MAX_SYNC; i++)
    {
        if(!hRdc->abSyncUsed[i])
        {
            break;
        }
    }

    if(i < BRDC_P_MAX_SYNC) {
        ulAddr = BCHP_RDC_sync_0_arm + sizeof(uint32_t) * i;
        ulReg = BCHP_FIELD_ENUM(RDC_sync_0_arm, arm, DISARMED);
        BRDC_P_Write32(hRdc, ulAddr, ulReg);
        BDBG_MSG(("RDC_sync_%u is acquired", i));
        hRdc->abSyncUsed[i] = true;
        *pulId = i;
    } else {
        *pulId = (uint32_t)(-1);
        BDBG_ERR(("RDC used all %u out of %u synchronizers!", BRDC_P_MAX_SYNC, BRDC_P_MAX_SYNC));
        BDBG_ASSERT(0);/* what can we do? */
        rc = BERR_INVALID_PARAMETER;
    }

    BDBG_LEAVE(BRDC_P_AcquireSync);
    return rc;
#else
    BSTD_UNUSED(hRdc);
    BDBG_ASSERT(pulId);
    *pulId = 0;
    return BERR_SUCCESS;
#endif
}

/***************************************************************************
Summary:
    Release sync to pool

Description:

Input:
    hRdc - The instance pool to release sync to.
    ulId  - sync id.

Output:

Returns:

****************************************************************************/
BERR_Code BRDC_P_ReleaseSync
    ( BRDC_Handle                      hRdc,
      uint32_t                         ulId )
{
#ifdef BCHP_RDC_sync_0_arm
    BDBG_ENTER(BRDC_P_ReleaseSync);

    BDBG_ASSERT(hRdc);
    if(ulId >= BRDC_P_MAX_SYNC) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    /* disarm the sync */
    BRDC_P_ArmSync_isr(hRdc, ulId, false);
    hRdc->abSyncUsed[ulId] = false;
    BDBG_LEAVE(BRDC_P_ReleaseSync);
#else
    BSTD_UNUSED(hRdc);
    BSTD_UNUSED(ulId);
#endif
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Arm/disarm the specific synchronizer

Description:

Input:
    hRdc - The instance handle.
    ulSyncId - sync id returned by BRDC_AcquireSync_isr.
    bArm - To arm or disarm the synchronizer.

Output:

Returns:

See Also:
    BRDC_P_AcquireSync
****************************************************************************/
BERR_Code BRDC_P_ArmSync_isr
    ( BRDC_Handle                      hRdc,
      uint32_t                         ulSyncId,
      bool                             bArm )
{
#ifdef BCHP_RDC_sync_0_arm
    uint32_t ulReg;
    uint32_t ulAddr;
    BDBG_ENTER(BRDC_P_ArmSync_isr);

    BDBG_ASSERT(hRdc);
    if(ulSyncId >= BRDC_P_MAX_SYNC) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    ulAddr = BCHP_RDC_sync_0_arm + sizeof(uint32_t) * ulSyncId;
    ulReg = BCHP_FIELD_DATA(RDC_sync_0_arm, arm, bArm);
    BRDC_P_Write32(hRdc, ulAddr, ulReg);
    BDBG_MSG(("RDC_sync_%u is %s", ulSyncId, bArm? "armed" : "disarmed"));

    BDBG_LEAVE(BRDC_P_ArmSync_isr);
#else
    BSTD_UNUSED(hRdc);
    BSTD_UNUSED(ulSyncId);
    BSTD_UNUSED(bArm);
#endif
    return BERR_SUCCESS;
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
    ulRegOffset = (BCHP_RDC_desc_1_addr - BCHP_RDC_desc_0_addr) * (eSlotId - BRDC_SlotId_eSlot0);

    /* header */
    BDBG_MSG(("-------------------------------"));

    /* read and display address register */
    ulRegVal = BRDC_P_Read32(hRdc, BCHP_RDC_desc_0_addr + ulRegOffset);
    BDBG_MSG(("RDC_desc_%d_addr", eSlotId));
    BDBG_MSG(("    addr: 0x%08x", ulRegVal));

    /* read and display config register */
    ulRegVal = BRDC_P_Read32(hRdc, BCHP_RDC_desc_0_config + ulRegOffset);
    BDBG_MSG(("RDC_desc_%d_config", eSlotId));
    BDBG_MSG(("    trigger_select:  %d",
        BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, trigger_select)));
    BDBG_MSG(("    repeat:          %d",
        BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, repeat)));
    BDBG_MSG(("    enable:          %d",
        BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, enable)));
#if BCHP_RDC_desc_0_config_high_priority_MASK
    BDBG_MSG(("    hi_priority      %d",
        BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, high_priority)));
#endif
#if BCHP_RDC_desc_0_config_segmented_MASK
    BDBG_MSG(("    segemented:      %d",
        BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, segmented)));
#endif
#if BCHP_RDC_desc_0_config_done_MASK
    BDBG_MSG(("    count:           %d",
        BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, count)));
    BDBG_MSG(("    done:            %d",
        BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, done)));
    BDBG_MSG(("    busy:            %d",
        BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, busy)));
    BDBG_MSG(("    error:           %d",
        BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, error)));
    BDBG_MSG(("    dropped_trigger: %d",
        BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, dropped_trigger)));
    BDBG_MSG(("    lock_rd:         %d",
        BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, lock_rd)));
#else
    BDBG_MSG(("    sync_sel:         %d",
        BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_config, sync_sel)));
    BDBG_MSG(("    count:            %d", eSlotId,
        BRDC_P_Read32(hRdc, BCHP_RDC_desc_0_count + ulRegOffset)));
    BDBG_MSG(("    count_direct:     %d", eSlotId,
        BRDC_P_Read32(hRdc, BCHP_RDC_desc_0_count_direct + ulRegOffset)));

    ulRegVal = BRDC_P_Read32(hRdc, BCHP_RDC_desc_0_status + ulRegOffset);
    BDBG_MSG(("BRDC_desc_%d_status", eSlotId));
    BDBG_MSG(("    done:            %d",
        BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_status, done)));
    BDBG_MSG(("    busy:            %d",
        BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_status, busy)));
    BDBG_MSG(("    bank:            %d",
        BCHP_GET_FIELD_DATA(ulRegVal, RDC_desc_0_status, bank)));
#endif

    /* contents of RUL */
    BDBG_MSG(("See RUL dump for RUL contents."));
    BSTD_UNUSED(ulRegVal);
}

/* end of file */
