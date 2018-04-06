/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "brdc.h"
#include "brdc_private.h"

#include "brdc_dbg.h"

BDBG_MODULE(BRDC_DBG);

#if(!B_REFSW_MINIMAL)
/***************************************************************************
 *
 */
BERR_Code BRDC_DBG_SetList_isr(
    BRDC_List_Handle  hList
    )
{
    BERR_Code  eErr = BERR_SUCCESS;

    /* initialize list state */
    hList->eNextEntry = BRDC_DBG_ListEntry_eCommand;

    hList->pulCurListAddr = BRDC_List_GetStartAddress_isr(hList);

    /* get number of entries */
    eErr = BERR_TRACE(BRDC_List_GetNumEntries_isr(hList, &hList->ulNumEntries));

    /*printf("++++++++++++++++++++++++++++++++++++++++++++++ hList->ulNumEntries = %d\n", hList->ulNumEntries);*/

    return eErr;
}

/***************************************************************************
 *
 */
BERR_Code BRDC_DBG_GetListEntry_isr(
    BRDC_List_Handle     hList,
    BRDC_DBG_ListEntry  *peEntry,
    uint32_t             aulArgs[4]
    )
{
    BERR_Code  eErr = BERR_SUCCESS;
    uint32_t   ulEntry;

    /* clear out arguments */
    BKNI_Memset(aulArgs, 0x0, sizeof(uint32_t) * 4);

    /* no more entries in list? */
    if(hList->ulNumEntries == 0)
    {
        /* next entry better be a command or we stopped in mid command */
        if(hList->eNextEntry != BRDC_DBG_ListEntry_eCommand)
        {
            /* error */
            eErr = BERR_TRACE(BERR_UNKNOWN);
            goto done;
        }

        /* list terminated correctly */
        *peEntry = BRDC_DBG_ListEntry_eEnd;
        goto done;
    }

    /* get next entry in list */
    *peEntry = hList->eNextEntry;
    ulEntry = *(hList->pulCurListAddr++);
    --hList->ulNumEntries;

    /* current entry should be a command? */
    if(hList->eNextEntry == BRDC_DBG_ListEntry_eCommand)
    {
        /* which command? */
        aulArgs[0] = BRDC_GET_OPCODE(ulEntry);
        hList->ulCurrCommand = aulArgs[0];
        hList->iCommandIndex = 0;
        hList->iDataCount = 0;

        /* get other command entries */
        switch(hList->ulCurrCommand)
        {
        case BRDC_OP_REG_TO_VAR_OPCODE:
        case BRDC_OP_IMM_TO_VAR_OPCODE:
        #if BRDC_64BIT_SUPPORT
        case BRDC_OP_REG_TO_VAR_OPCODE64:
        case BRDC_OP_IMM_TO_VAR_OPCODE64:
        #endif
            /* variable stored in bits 5-0 */
            aulArgs[1] = ulEntry & UINT32_C(0x3F);
            break;

        case BRDC_OP_VAR_TO_REG_OPCODE:
        case BRDC_OP_COND_SKIP_OPCODE:
        #if BRDC_64BIT_SUPPORT
        case BRDC_OP_VAR_TO_REG_OPCODE64:
        case BRDC_OP_COND_SKIP_OPCODE64:
        #endif
            /* variable stored in bits 17-12 */
            aulArgs[1] = (ulEntry >> 12) & UINT32_C(0x3F);
            break;

        case BRDC_OP_IMMS_TO_REG_OPCODE:
        case BRDC_OP_IMMS_TO_REGS_OPCODE:
        #if BRDC_64BIT_SUPPORT
        case BRDC_OP_IMMS_TO_REG_OPCODE64:
        case BRDC_OP_IMMS_TO_REGS_OPCODE64:
        #endif
            /* count-1 stored in bits 11-0 (store data count) */
            aulArgs[1] = (ulEntry & UINT32_C(0xFFF)) + 1;
            hList->iDataCount = aulArgs[1];
            break;

        case BRDC_OP_REG_TO_REG_OPCODE:
        case BRDC_OP_REGS_TO_REGS_OPCODE:
        #if BRDC_64BIT_SUPPORT
        case BRDC_OP_REG_TO_REG_OPCODE64:
        case BRDC_OP_REGS_TO_REGS_OPCODE64:
        #endif
            /* count-1 stored in bits 11-0 */
            aulArgs[1] = (ulEntry & UINT32_C(0xFFF)) + 1;
            break;

        case BRDC_OP_VAR_AND_VAR_TO_VAR_OPCODE:
        case BRDC_OP_VAR_OR_VAR_TO_VAR_OPCODE:
        case BRDC_OP_VAR_XOR_VAR_TO_VAR_OPCODE:
        case BRDC_OP_VAR_SUM_VAR_TO_VAR_OPCODE:
        #if BRDC_64BIT_SUPPORT
        case BRDC_OP_VAR_AND_VAR_TO_VAR_OPCODE64:
        case BRDC_OP_VAR_OR_VAR_TO_VAR_OPCODE64:
        case BRDC_OP_VAR_XOR_VAR_TO_VAR_OPCODE64:
        case BRDC_OP_VAR_SUM_VAR_TO_VAR_OPCODE64:
        #endif
            /* bit 17-12 = src1; bit 11-6 = src2; bit 5-0 = dst */
            aulArgs[1] = (ulEntry >> 12) & UINT32_C(0x3F);
            aulArgs[2] = (ulEntry >>  6) & UINT32_C(0x3F);
            aulArgs[3] = (ulEntry      ) & UINT32_C(0x3F);
            break;

        case BRDC_OP_VAR_AND_IMM_TO_VAR_OPCODE:
        case BRDC_OP_VAR_OR_IMM_TO_VAR_OPCODE:
        case BRDC_OP_VAR_XOR_IMM_TO_VAR_OPCODE:
        case BRDC_OP_NOT_VAR_TO_VAR_OPCODE:
        case BRDC_OP_VAR_SUM_IMM_TO_VAR_OPCODE:
        #if BRDC_64BIT_SUPPORT
        case BRDC_OP_VAR_AND_IMM_TO_VAR_OPCODE64:
        case BRDC_OP_VAR_OR_IMM_TO_VAR_OPCODE64:
        case BRDC_OP_VAR_XOR_IMM_TO_VAR_OPCODE64:
        case BRDC_OP_NOT_VAR_TO_VAR_OPCODE64:
        case BRDC_OP_VAR_SUM_IMM_TO_VAR_OPCODE64:
        #endif
            /* bit 17-12 = src; bit 5-0 = dst */
            aulArgs[1] = (ulEntry >> 12) & UINT32_C(0x3F);
            aulArgs[2] = (ulEntry      ) & UINT32_C(0x3F);
            break;

        case BRDC_OP_VAR_ROR_TO_VAR_OPCODE:
        #if BRDC_64BIT_SUPPORT
        case BRDC_OP_VAR_ROR_TO_VAR_OPCODE64:
        #endif
            /* bit 22-18 = rotate; bit 17-12 = src; bit 5-0 = dst */
            aulArgs[1] = (ulEntry >> 18) & UINT32_C(0x1F);
            aulArgs[2] = (ulEntry >> 12) & UINT32_C(0x3F);
            aulArgs[3] = (ulEntry      ) & UINT32_C(0x3F);
            break;

        case BRDC_OP_NOP_OPCODE:
        case BRDC_OP_IMM_TO_REG_OPCODE:
        case BRDC_OP_SKIP_OPCODE:
        case BRDC_OP_EXIT_OPCODE:
        #if BRDC_64BIT_SUPPORT
        case BRDC_OP_IMM_TO_REG_OPCODE64:
        #endif
            /* no extra data in command word */
            break;

        default:
            /* unknown command */
            eErr = BERR_TRACE(BERR_UNKNOWN);
            goto done;
        }

    /* next entry is not a command */
    } else
    {
        /* register? */
        if(hList->eNextEntry == BRDC_DBG_ListEntry_eRegister)
        {
            /* convert RDC register to offset register type */
            aulArgs[0] = BRDC_REGISTER_TO_OFFSET(ulEntry);

        /* data */
        } else
        {
            /* unmodified data */
            aulArgs[0] = ulEntry;
        }
    }

    /* determine what the next value should be next time */
    switch(hList->ulCurrCommand)
    {
        case BRDC_OP_IMM_TO_REG_OPCODE:
            /* register then data */
            if(hList->iCommandIndex == 0)
            {
                /* register */
                hList->eNextEntry = BRDC_DBG_ListEntry_eRegister;
            } else if(hList->iCommandIndex == 1)
            {
                /* data */
                hList->eNextEntry = BRDC_DBG_ListEntry_eData;
            } else
            {
                /* next command */
                hList->eNextEntry = BRDC_DBG_ListEntry_eCommand;
            }
            break;

        #if BRDC_64BIT_SUPPORT
        case BRDC_OP_IMM_TO_REG_OPCODE64:
            /* register then data  then data */
            if(hList->iCommandIndex == 0)
            {
                /* register */
                hList->eNextEntry = BRDC_DBG_ListEntry_eRegister;
            } else if(hList->iCommandIndex <=  2)
            {
                /* data */
                hList->eNextEntry = BRDC_DBG_ListEntry_eData;
            } else
            {
                /* next command */
                hList->eNextEntry = BRDC_DBG_ListEntry_eCommand;
            }
            break;
        #endif

        case BRDC_OP_VAR_TO_REG_OPCODE:
        case BRDC_OP_REG_TO_VAR_OPCODE:
        case BRDC_OP_COND_SKIP_OPCODE:
        #if BRDC_64BIT_SUPPORT
        case BRDC_OP_REG_TO_VAR_OPCODE64:
        case BRDC_OP_VAR_TO_REG_OPCODE64:
        case BRDC_OP_COND_SKIP_OPCODE64:
        #endif
            /* register only */
            if(hList->iCommandIndex == 0)
            {
                /* register */
                hList->eNextEntry = BRDC_DBG_ListEntry_eRegister;
            } else
            {
                /* next command */
                hList->eNextEntry = BRDC_DBG_ListEntry_eCommand;
            }
            break;

        case BRDC_OP_IMM_TO_VAR_OPCODE:
        case BRDC_OP_VAR_AND_IMM_TO_VAR_OPCODE:
        case BRDC_OP_VAR_OR_IMM_TO_VAR_OPCODE:
        case BRDC_OP_VAR_XOR_IMM_TO_VAR_OPCODE:
        case BRDC_OP_VAR_SUM_IMM_TO_VAR_OPCODE:
            /* data only */
            if(hList->iCommandIndex == 0)
            {
                /* register */
                hList->eNextEntry = BRDC_DBG_ListEntry_eData;
            } else
            {
                /* next command */
                hList->eNextEntry = BRDC_DBG_ListEntry_eCommand;
            }
            break;

        #if BRDC_64BIT_SUPPORT
        case BRDC_OP_IMM_TO_VAR_OPCODE64:
        case BRDC_OP_VAR_AND_IMM_TO_VAR_OPCODE64:
        case BRDC_OP_VAR_OR_IMM_TO_VAR_OPCODE64:
        case BRDC_OP_VAR_XOR_IMM_TO_VAR_OPCODE64:
        case BRDC_OP_VAR_SUM_IMM_TO_VAR_OPCODE64:
            /* data only, twice */
            if(hList->iCommandIndex <= 1)
            {
                /* data */
                hList->eNextEntry = BRDC_DBG_ListEntry_eData;
            } else
            {
                /* next command */
                hList->eNextEntry = BRDC_DBG_ListEntry_eCommand;
            }
            break;
        #endif

        case BRDC_OP_VAR_AND_VAR_TO_VAR_OPCODE:
        case BRDC_OP_VAR_OR_VAR_TO_VAR_OPCODE:
        case BRDC_OP_VAR_XOR_VAR_TO_VAR_OPCODE:
        case BRDC_OP_VAR_SUM_VAR_TO_VAR_OPCODE:
        #if BRDC_64BIT_SUPPORT
        case BRDC_OP_VAR_AND_VAR_TO_VAR_OPCODE64:
        case BRDC_OP_VAR_OR_VAR_TO_VAR_OPCODE64:
        case BRDC_OP_VAR_XOR_VAR_TO_VAR_OPCODE64:
        case BRDC_OP_VAR_SUM_VAR_TO_VAR_OPCODE64:
        #endif
            /* no data follows command */
            hList->eNextEntry = BRDC_DBG_ListEntry_eCommand;
            break;

        case BRDC_OP_IMMS_TO_REG_OPCODE:
        case BRDC_OP_IMMS_TO_REGS_OPCODE:
            /* single register then N data */
            if(hList->iCommandIndex == 0)
            {
                /* register */
                hList->eNextEntry = BRDC_DBG_ListEntry_eRegister;
            } else if(hList->iCommandIndex <= hList->iDataCount)
            {
                /* data */
                hList->eNextEntry = BRDC_DBG_ListEntry_eData;
            } else
            {
                /* next command */
                hList->eNextEntry = BRDC_DBG_ListEntry_eCommand;
            }
            break;

        #if BRDC_64BIT_SUPPORT
        case BRDC_OP_IMMS_TO_REG_OPCODE64:
        case BRDC_OP_IMMS_TO_REGS_OPCODE64:
            /* single register then 2*N data */
            if(hList->iCommandIndex == 0)
            {
                /* register */
                hList->eNextEntry = BRDC_DBG_ListEntry_eRegister;
            } else if(hList->iCommandIndex <= 2*hList->iDataCount)
            {
                /* data */
                hList->eNextEntry = BRDC_DBG_ListEntry_eData;
            } else
            {
                /* next command */
                hList->eNextEntry = BRDC_DBG_ListEntry_eCommand;
            }
            break;
        #endif

        case BRDC_OP_REG_TO_REG_OPCODE:
        case BRDC_OP_REGS_TO_REGS_OPCODE:
        #if BRDC_64BIT_SUPPORT
        case BRDC_OP_REG_TO_REG_OPCODE64:
        case BRDC_OP_REGS_TO_REGS_OPCODE64:
        #endif
            /* source register followed by destination register */
            if(hList->iCommandIndex <= 1)
            {
                /* register */
                hList->eNextEntry = BRDC_DBG_ListEntry_eRegister;
            } else
            {
                /* next command */
                hList->eNextEntry = BRDC_DBG_ListEntry_eCommand;
            }
            break;

        case BRDC_OP_NOT_VAR_TO_VAR_OPCODE:
        case BRDC_OP_VAR_ROR_TO_VAR_OPCODE:
        case BRDC_OP_NOP_OPCODE:
        case BRDC_OP_SKIP_OPCODE:
        case BRDC_OP_EXIT_OPCODE:
        #if BRDC_64BIT_SUPPORT
        case BRDC_OP_NOT_VAR_TO_VAR_OPCODE64:
        case BRDC_OP_VAR_ROR_TO_VAR_OPCODE64:
        #endif
            /* no data follows command */
            hList->eNextEntry = BRDC_DBG_ListEntry_eCommand;
            break;

        default:
            /* unknown command */
            eErr = BERR_TRACE(BERR_UNKNOWN);
            goto done;
    }

    /* next command index */
    ++hList->iCommandIndex;

done:
    return eErr;
}

/***************************************************************************
 *
 */
BERR_Code BRDC_DBG_SetList(
    BRDC_List_Handle  hList
    )
{
    BERR_Code  eErr = BERR_SUCCESS;

    /* initialize list state */
    hList->eNextEntry = BRDC_DBG_ListEntry_eCommand;

    BKNI_EnterCriticalSection();
    hList->pulCurListAddr = BRDC_List_GetStartAddress_isr(hList);

    /* get number of entries */
    eErr = BERR_TRACE(BRDC_List_GetNumEntries_isr(hList, &hList->ulNumEntries));
    BKNI_LeaveCriticalSection();

    return eErr;
}

BERR_Code BRDC_DBG_GetListEntry(
    BRDC_List_Handle     hList,
    BRDC_DBG_ListEntry  *peEntry,
    uint32_t             aulArgs[4]
    )
{
    BERR_Code  eErr = BERR_SUCCESS;
    BKNI_EnterCriticalSection();
    eErr = BRDC_DBG_GetListEntry_isr(hList, peEntry, aulArgs);
    BKNI_LeaveCriticalSection();
    return eErr;
}
#endif

#ifdef BRDC_USE_CAPTURE_BUFFER
/**************************************************************
*
* CaptureBuffer
*
*/

BERR_Code BRDC_DBG_CreateCaptureBuffer(BRDC_DBG_CaptureBuffer *buffer, int size)
{
    BKNI_Memset(buffer, 0, sizeof(*buffer));
    buffer->size = size;
    buffer->mem = BKNI_Malloc(size);
    if (!buffer->mem)
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    buffer->enable = true;
    return 0;
}

void BRDC_DBG_DestroyCaptureBuffer(BRDC_DBG_CaptureBuffer *buffer)
{
    BKNI_Free(buffer->mem);
}

static void BRDC_DBG_Write_isr(BRDC_DBG_CaptureBuffer *buffer, const void *mem_, int size)
{
    const uint8_t *mem = (uint8_t*)mem_;
    int remainder = 0;
    if (size > buffer->size - buffer->writeptr) {
        int temp = buffer->size - buffer->writeptr;
        remainder = size - temp;
        size = temp;
    }

    BKNI_Memcpy(&buffer->mem[buffer->writeptr], mem, size);
    buffer->writeptr += size;
    if (buffer->writeptr == buffer->size)
        buffer->writeptr = 0;

    if (remainder)
        BRDC_DBG_Write_isr(buffer, &mem[size], remainder);
}

void BRDC_DBG_WriteCapture_isr(BRDC_DBG_CaptureBuffer *buffer, BRDC_Slot_Handle hSlot, BRDC_List_Handle hList)
{
    int n = hList->ulEntries * sizeof(uint32_t);
    uint32_t prefix = BRDC_DBG_RUL;

    /* assumed to be called in isr context, so no synchronization is needed */

    if (buffer->enable)
    {
        char timestamp[80];

        buffer->num_ruls++;
        buffer->total_bytes += n;

        BKNI_Snprintf(timestamp, 80, "Slot%d timestamp = %u (us)", hSlot->eSlotId, BRDC_Slot_GetTimerSnapshot_isr(hSlot));
        BRDC_DBG_LogErrorCode_isr(hSlot->hRdc, BRDC_DBG_RUL_TIMESTAMP, timestamp);

        BRDC_DBG_Write_isr(buffer, &prefix, sizeof(prefix));
        BRDC_DBG_Write_isr(buffer, &hList->ulEntries, sizeof(hList->ulEntries));
        BRDC_DBG_Write_isr(buffer, &hSlot->eSlotId, sizeof(hSlot->eSlotId));
        BMMA_FlushCache_isr(hList->hRULBlock, hList->pulRULAddr, n);
        BRDC_DBG_Write_isr(buffer, hList->pulRULAddr, n);
    }
}

void BRDC_DBG_WriteCaptures_isr(BRDC_DBG_CaptureBuffer *buffer, BRDC_Slot_Handle *phSlots,BRDC_List_Handle hList, uint32_t ulSlots)
{
    int n = hList->ulEntries * sizeof(uint32_t);
    uint32_t prefix = BRDC_DBG_RUL;

    /* assumed to be called in isr context, so no synchronization is needed */

    if (buffer->enable)
    {
        char timestamp[80];
        uint32_t i;
        BRDC_Slot_Handle hSlot;

        buffer->num_ruls++;
        buffer->total_bytes += n;
        hSlot = phSlots[0];

        for(i=0; i<ulSlots; i++)
        {
            BKNI_Snprintf(timestamp, 80, "Slot%d timestamp = %u (us)", phSlots[i]->eSlotId, BRDC_Slot_GetTimerSnapshot_isr(phSlots[i]));
            BRDC_DBG_LogErrorCode_isr(hSlot->hRdc, BRDC_DBG_RUL_TIMESTAMP, timestamp);
        }

        BRDC_DBG_Write_isr(buffer, &prefix, sizeof(prefix));
        BRDC_DBG_Write_isr(buffer, &hList->ulEntries, sizeof(hList->ulEntries));
        BRDC_DBG_Write_isr(buffer, &hSlot->eSlotId, sizeof(hSlot->eSlotId));
        BMMA_FlushCache_isr(hList->hRULBlock, hList->pulRULAddr, n);
        BRDC_DBG_Write_isr(buffer, hList->pulRULAddr, n);
    }
}

void BRDC_DBG_ReadCapture_isr(BRDC_Handle rdc, uint8_t *mem, int size, int *read)
{
    int max;
    BRDC_DBG_CaptureBuffer *buffer = &rdc->captureBuffer;

    if (buffer->readptr <= buffer->writeptr) {
        max = buffer->writeptr - buffer->readptr;
    }
    else {
        /* don't read around the wrap, let the app do that */
        max = buffer->size - buffer->readptr;
    }

    if (size > max)
        size = max;
    *read = size;

    if (size) {
        BKNI_Memcpy(mem, &buffer->mem[buffer->readptr], size);
        buffer->readptr += size;
        if (buffer->readptr == buffer->size)
            buffer->readptr = 0;
    }
}

static int BKNI_Strlen(const char *str)
{
    int total = 0;
    while (*str++) total++;
    return total;
}
#endif

void BRDC_DBG_LogErrorCode_isr(BRDC_Handle rdc, uint32_t errortype, const char *str)
{
#ifdef BRDC_USE_CAPTURE_BUFFER
    uint32_t val[2];
    val[0] = errortype;
    val[1] = BKNI_Strlen(str);
    BRDC_DBG_Write_isr(&rdc->captureBuffer, val, sizeof(val));
    BRDC_DBG_Write_isr(&rdc->captureBuffer, str, val[1]);
#else
    BSTD_UNUSED(rdc);
    BSTD_UNUSED(errortype);
    BSTD_UNUSED(str);
#endif
}


#ifdef BRDC_USE_CAPTURE_BUFFER
void BRDC_DBG_EnableCapture_isr(BRDC_Handle rdc, bool enable)
{
    BRDC_DBG_CaptureBuffer *buffer = &rdc->captureBuffer;
    buffer->enable = enable;
}
#endif

#ifdef BRDC_DEBUG
/***************************************************************************
 * This function is for debugging purpose ONLY, to test slot-linked
 * synchronizer
 *
 * It's exactly same as BRDC_Slots_SetList_isr except it calls
 * BRDC_P_Slots_SetList_NoArmSync_isr instead of BRDC_P_Slots_SetList_isr
 */
BERR_Code BRDC_Slot_SetList_NoArmSync_isr
    ( BRDC_Slot_Handle                 hSlot,
      BRDC_List_Handle                 hList )
{
    uint32_t *pulStart   = hList->pulRULAddr;
    uint32_t *pulCurrent = pulStart + hList->ulEntries;
    BERR_Code  err = BERR_SUCCESS;

    BDBG_ENTER(BRDC_Slot_SetList_NoArmSync_isr);

    /* Update the number of time this list, assigned to a slot. */
    if(hSlot->bTrackExecution)
    {
        *pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pulCurrent++ = BRDC_REGISTER(hSlot->ulTrackRegAddr);
        *pulCurrent++ = ++(hSlot->ulTrackCount);
        hList->ulEntries = (uint32_t)(pulCurrent - pulStart);
    }

    /* Flush the list before setting it to dual slots. */
    BMMA_FlushCache_isr(hList->hRULBlock, hList->pulRULAddr, hList->ulEntries * sizeof(uint32_t));

    err = BRDC_P_Slots_SetList_NoArmSync_isr(&hSlot, hList, 1);

    BDBG_LEAVE(BRDC_Slot_SetList_NoArmSync_isr);
    return err;
}
#endif

/* end of file */
