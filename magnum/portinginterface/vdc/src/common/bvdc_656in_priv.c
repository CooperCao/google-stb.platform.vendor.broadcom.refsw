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
 *
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"
#include "bvdc_priv.h"
#include "bvdc_656in_priv.h"
#include "bvdc_window_priv.h"

/***************************************************************************/
/* Has some support for at least one VDEC? */
#if (BVDC_P_SUPPORT_NEW_656_IN_VER)

#include "bvdc_displayfmt_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_656in_priv.h"

#include "bchp_ext_656_top_0.h"
#if (BVDC_P_NUM_656IN_SUPPORT > 1)
#include "bchp_ext_656_top_1.h"
#endif

BDBG_MODULE(BVDC_656);
BDBG_OBJECT_ID(BVDC_656);

/***************************************************************************
 *
 */
static void BVDC_P_656In_BuildVsyncRul_isr
    ( BVDC_P_656In_Handle              h656In,
      BVDC_P_ListInfo                 *pList )
{
    if(h656In->bVideoDetected)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ =
            BRDC_REGISTER(BCHP_EXT_656_TOP_0_ext_656_host_enable + h656In->ulOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_host_enable, HOST_ENABLE, 0x1 );
    }

    return;
}

/***************************************************************************
 *
 */
static void BVDC_P_656In_BuildResetRul_isr
    ( const BVDC_P_656In_Handle        h656In,
      BVDC_P_ListInfo                 *pList)
{

#if (BVDC_P_SUPPORT_NEW_656_IN_VER != BVDC_P_656IN_NEW_VER_2)
    /* EXT_656_TOP_0_ext_656_reset (W) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ =
        BRDC_REGISTER(BCHP_EXT_656_TOP_0_ext_656_reset + h656In->ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_reset, reset, 1);

    /* wait for 3 clocks */
    *pList->pulCurrent++ = BRDC_OP_NOP();
    *pList->pulCurrent++ = BRDC_OP_NOP();
    *pList->pulCurrent++ = BRDC_OP_NOP();

    /* EXT_656_TOP_0_ext_656_reset (W) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ =
        BRDC_REGISTER(BCHP_EXT_656_TOP_0_ext_656_reset + h656In->ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_reset, reset, 0);
#endif

    /* EXT_656_TOP_0_ext_656_control (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ =
        BRDC_REGISTER(BCHP_EXT_656_TOP_0_ext_656_control + h656In->ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_control, reserved0,               0       ) |
#if (BVDC_P_SUPPORT_NEW_656_IN_VER == BVDC_P_656IN_NEW_VER_1)
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_control, reserved_for_eco1,       0       ) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_control, reserved_for_eco2,       0x7     ) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_control, reserved_for_eco3,       0       ) |
        BCHP_FIELD_ENUM(EXT_656_TOP_0_ext_656_control, bvb_replace_ff,          DISABLED) |

#elif (BVDC_P_SUPPORT_NEW_656_IN_VER == BVDC_P_656IN_NEW_VER_2)
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_control, reserved_for_eco1,       0       ) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_control, reserved_for_eco2,       0x7     ) |

#elif (BVDC_P_SUPPORT_NEW_656_IN_VER == BVDC_P_656IN_NEW_VER_3)
        BCHP_FIELD_ENUM(EXT_656_TOP_0_ext_656_control, field_1_count_enable,    ENABLED ) |
        BCHP_FIELD_ENUM(EXT_656_TOP_0_ext_656_control, field_0_count_enable,    ENABLED ) |
        BCHP_FIELD_ENUM(EXT_656_TOP_0_ext_656_control, itu656_sav_msb_polarity, ONE     ) |
        BCHP_FIELD_ENUM(EXT_656_TOP_0_ext_656_control, hibernation_enabled,     DISABLED) |
        BCHP_FIELD_ENUM(EXT_656_TOP_0_ext_656_control, bvb_replace_ff,          DISABLED) |

#endif
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_control, data_width,              0       ) |
        BCHP_FIELD_ENUM(EXT_656_TOP_0_ext_656_control, bvb_output_enabled,      ENABLED );

    /* EXT_656_TOP_0_ext_656_host_legacy (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ =
        BRDC_REGISTER(BCHP_EXT_656_TOP_0_ext_656_host_legacy + h656In->ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_host_legacy , reserved0,           0) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_host_legacy , reserved1,           0) |
#if (BVDC_P_SUPPORT_NEW_656_IN_VER == BVDC_P_656IN_NEW_VER_3)
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_host_legacy , USE_HOST_EN_DIS,     0) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_host_legacy , USE_DISCARD_PICTURE, 0) |
#endif
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_host_legacy , USE_LEGACY_TRIGGER,  0);


    /* EXT_656_TOP_0_ext_656_interrupt (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ =
        BRDC_REGISTER(BCHP_EXT_656_TOP_0_ext_656_interrupt + h656In->ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_interrupt, reserved0, 0   ) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_interrupt, field_1,   0x16) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_interrupt, reserved1, 0   ) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_interrupt, field_0,   0x16);

    /* EXT_656_TOP_0_ext_656_trigger (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ =
        BRDC_REGISTER(BCHP_EXT_656_TOP_0_ext_656_trigger + h656In->ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_trigger , reserved0,      0      ) |
        BCHP_FIELD_ENUM(EXT_656_TOP_0_ext_656_trigger , field_1_enable, ENABLED) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_trigger , field_1,        0x6    ) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_trigger , reserved1,      0      ) |
        BCHP_FIELD_ENUM(EXT_656_TOP_0_ext_656_trigger , field_0_enable, ENABLED) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_trigger , field_0,        0x6);

    return;
}

/***************************************************************************
 *
 */
static void BVDC_P_656In_BuildFormatRul_isr
    ( const BVDC_P_656In_Handle        h656In,
      BVDC_P_ListInfo                 *pList)
{
    uint32_t                     ulVSize, ulHSize;
    uint32_t                     ulAutoRepeat, ulPicDelay;
    const BFMT_VideoInfo        *pFmtInfo;

    BDBG_OBJECT_ASSERT(h656In->hSource, BVDC_SRC);

    /* Current settings */
    pFmtInfo = h656In->hSource->stCurInfo.pFmtInfo;

    BDBG_ASSERT(
        BFMT_IS_PAL(pFmtInfo->eVideoFmt) ||
        BFMT_IS_NTSC(pFmtInfo->eVideoFmt));

    ulHSize = pFmtInfo->ulDigitalWidth / 2;
    ulVSize = (pFmtInfo->bInterlaced)
        ? (pFmtInfo->ulDigitalHeight / BVDC_P_FIELD_PER_FRAME)
        : (pFmtInfo->ulDigitalHeight);

    /* New trigger mode.  How often to repeat the trigger in sysclk cycle unit */
    ulAutoRepeat = (BVDC_P_BVB_BUS_CLOCK / pFmtInfo->ulVertFreq) * BFMT_FREQ_FACTOR;
    ulPicDelay   = ((ulAutoRepeat << pFmtInfo->bInterlaced) / pFmtInfo->ulScanHeight) * BVDC_P_656IN_TRIGGER_OFFSET;

#if (BVDC_P_SUPPORT_NEW_656_IN_VER <= BVDC_P_656IN_NEW_VER_2)
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ =
        BRDC_REGISTER(BCHP_EXT_656_TOP_0_ext_656_count + h656In->ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_count , reserved0, 0    ) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_count , vsize,     ulVSize) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_count , reserved1, 0    ) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_count , hsize,     ulHSize);
#else
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ =
        BRDC_REGISTER(BCHP_EXT_656_TOP_0_ext_656_short_count + h656In->ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_short_count , reserved0           , 0        ) |
        BCHP_FIELD_ENUM(EXT_656_TOP_0_ext_656_short_count , vsize_compare_enable, ENABLED  ) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_short_count , vsize               , ulVSize-1) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_short_count , reserved1           , 0        ) |
        BCHP_FIELD_ENUM(EXT_656_TOP_0_ext_656_short_count , hsize_compare_enable, ENABLED  ) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_short_count , hsize               , ulHSize-1);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ =
        BRDC_REGISTER(BCHP_EXT_656_TOP_0_ext_656_long_count + h656In->ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_long_count , reserved0           , 0        ) |
        BCHP_FIELD_ENUM(EXT_656_TOP_0_ext_656_long_count , vsize_compare_enable, ENABLED  ) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_long_count , vsize               , ulVSize  ) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_long_count , reserved1           , 0        ) |
        BCHP_FIELD_ENUM(EXT_656_TOP_0_ext_656_long_count , hsize_compare_enable, ENABLED  ) |
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_long_count , hsize               , ulHSize  );
#endif

    /* EXT_656_TOP_0_ext_656_auto_repeat (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ =
        BRDC_REGISTER(BCHP_EXT_656_TOP_0_ext_656_auto_repeat + h656In->ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_auto_repeat , AUTO_REPEAT, ulAutoRepeat);

    /* EXT_656_TOP_0_ext_656_picture_delay (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ =
        BRDC_REGISTER(BCHP_EXT_656_TOP_0_ext_656_picture_delay + h656In->ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(EXT_656_TOP_0_ext_656_picture_delay, PICTURE_DELAY, ulPicDelay );

    BDBG_MSG(("New entries added by 656 Build Format RUL"));
    return;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_P_656In_Create
    ( BVDC_P_656In_Handle             *ph656In,
      BVDC_P_656Id                     e656Id,
      BVDC_Source_Handle               hSource )
{
    BVDC_P_656InContext   *p656In;

    BDBG_ENTER(BVDC_P_656In_Create);
    BDBG_ASSERT(ph656In);

    /* BDBG_SetModuleLevel("BVDC_656", BDBG_eMsg);  */

    /* Get Source context */
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    /* The handle will be NULL if create fails. */
    *ph656In = NULL;

    /* (1) Alloc the context. */
    p656In = (BVDC_P_656InContext*)
        (BKNI_Malloc(sizeof(BVDC_P_656InContext)));
    if(!p656In)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)p656In, 0x0, sizeof(BVDC_P_656InContext));
    BDBG_OBJECT_SET(p656In, BVDC_656);

    /* Store the id & hRegister for activating the triggers. */
    p656In->eId                   = e656Id;
    p656In->hSource               = hSource;

#if (BVDC_P_NUM_656IN_SUPPORT > 1)
    /* Getting the offset of various 656 sub-modules.  The offset is
     * from the 656_0.*/
    if(p656In->eId ==  BVDC_P_656Id_e656In1)
    {
        p656In->ulOffset = BCHP_EXT_656_TOP_1_ext_656_rev_id
            - BCHP_EXT_656_TOP_0_ext_656_rev_id;
    }
#endif

    /* (2) All done. now return the new fresh context to user. */
    *ph656In = (BVDC_P_656In_Handle)p656In;
    BVDC_P_656In_Init(*ph656In);

    BDBG_LEAVE(BVDC_P_656In_Create);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_656In_Destroy
    ( BVDC_P_656In_Handle              h656In )
{
    BDBG_ENTER(BVDC_P_656In_Destroy);
    BDBG_OBJECT_ASSERT(h656In, BVDC_656);

    BDBG_OBJECT_DESTROY(h656In, BVDC_656);
    /* [1] Release context in system memory */
    BKNI_Free((void*)h656In);

    BDBG_LEAVE(BVDC_P_656In_Destroy);
    return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_656In_Init
    ( BVDC_P_656In_Handle              h656In )
{
    BDBG_ENTER(BVDC_P_656In_Init);
    BDBG_OBJECT_ASSERT(h656In, BVDC_656);

    /* 656In states */
    h656In->bVideoDetected    = false;
    h656In->ulDelayStart      = BVDEC_P_BVB_OUT_SLOW_START_COUNT;
    h656In->eFrameRateCode    = BAVC_FrameRateCode_e29_97;

    BDBG_LEAVE(BVDC_P_656In_Init);
    return;
}


/***************************************************************************
 * {private}
 *
 * Update Vdec status.
 */
void BVDC_P_656In_UpdateStatus_isr
    ( BVDC_P_656In_Handle              h656In )
{
    bool                      bVideoDetected;
    BVDC_P_Source_Info       *pCurInfo;
    BVDC_P_Source_DirtyBits  *pCurDirty;

    BDBG_OBJECT_ASSERT(h656In, BVDC_656);
    BDBG_OBJECT_ASSERT(h656In->hSource, BVDC_SRC);
    pCurInfo  = &h656In->hSource->stCurInfo;
    pCurDirty = &pCurInfo->stDirty;

    /* Video is probably garbage when sourc is unplugpged, and need vec
     * to drive triggers. */
    bVideoDetected =
        (BVDC_P_TriggerCtrl_eSource == h656In->hSource->eTrigCtrl) ? true : false;
    if(h656In->bVideoDetected != bVideoDetected)
    {
        if(bVideoDetected)
        {
            BDBG_MSG(("656in detects input"));
            h656In->ulDelayStart = BVDEC_P_BVB_OUT_SLOW_START_COUNT;
            h656In->eFrameRateCode = BFMT_IS_525_LINES(pCurInfo->pFmtInfo->eVideoFmt)?
                BAVC_FrameRateCode_e29_97 : BAVC_FrameRateCode_e25;
        }
        h656In->bVideoDetected = bVideoDetected;
        pCurDirty->stBits.bVideoDetected = BVDC_P_DIRTY;
        h656In->hSource->bStartFeed = bVideoDetected;
    }

    return;
}


/***************************************************************************
 * {private}
 *
 * Get Vdec status.
 */
void BVDC_P_656In_GetStatus_isr
    ( const BVDC_P_656In_Handle        h656In,
      bool                            *pbVideoDetected )
{
    BDBG_OBJECT_ASSERT(h656In, BVDC_656);

    /* (2) VDEC's input video detected? */
    if(pbVideoDetected)
    {
        *pbVideoDetected = h656In->bVideoDetected && (!h656In->ulDelayStart);
    }

    return;
}


/***************************************************************************
 *
 */
void BVDC_P_656In_Bringup_isr
    ( const BVDC_P_656In_Handle        h656In )
{
    uint32_t               i;
    BRDC_List_Handle       hList;
    BRDC_Slot_Handle       hSlot;
    BVDC_P_ListInfo        stList, *pList;

    BDBG_OBJECT_ASSERT(h656In, BVDC_656);
    BDBG_OBJECT_ASSERT(h656In->hSource, BVDC_SRC);

    /* Just build on the top field only!  This is a force execution. */
    BVDC_P_SRC_NEXT_RUL(h656In->hSource, BAVC_Polarity_eTopField);
    hSlot = BVDC_P_SRC_GET_SLOT(h656In->hSource, BAVC_Polarity_eTopField);
    hList = BVDC_P_SRC_GET_LIST(h656In->hSource, BAVC_Polarity_eTopField);

    /* This will arm all the slot with trigger executions. */
    for(i = 0; i < h656In->hSource->ulSlotUsed; i++)
    {
        BRDC_Slot_ExecuteOnTrigger_isr(h656In->hSource->ahSlot[i],
            h656In->hSource->aeTrigger[i], true);
    }

    /* Get the list infos */
    pList = &stList;
    BVDC_P_ReadListInfo_isr(pList, hList);

    /* Build drian RUL .  Only need to build it once. */
    BVDC_P_Drain_BuildRul_isr(&h656In->hSource->stDrain, pList);

    /* Build reset RUL */
    BVDC_P_656In_BuildResetRul_isr(h656In, pList);

    /* Build format RUL */
    BVDC_P_656In_BuildFormatRul_isr(h656In, pList);

    /* Build vsync RUL */
    BVDC_P_656In_BuildVsyncRul_isr(h656In, pList);

    /* Update the list */
    BVDC_P_WriteListInfo_isr(pList, hList);

    /* Initial bringup list */
    BRDC_Slot_SetList_isr(hSlot, hList);
    BRDC_Slot_Execute_isr(hSlot);

    BDBG_MSG(("Bringup source[%d].", h656In->hSource->eId));
    return;

}

/***************************************************************************
 *
 */
void BVDC_P_656In_BuildRul_isr
    ( const BVDC_P_656In_Handle        h656In,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldId )
{
    BVDC_P_Source_Info       *pCurInfo;
    BVDC_P_Source_DirtyBits  *pCurDirty, *pOldDirty;

    BDBG_OBJECT_ASSERT(h656In, BVDC_656);
    BDBG_OBJECT_ASSERT(h656In->hSource, BVDC_SRC);

    pOldDirty = &h656In->hSource->astOldDirty[eFieldId];
    pCurInfo  = &h656In->hSource->stCurInfo;
    pCurDirty = &pCurInfo->stDirty;

    /* Clear old dirty bits. */
    if(BVDC_P_IS_DIRTY(pOldDirty))
    {
        if(!pList->bLastExecuted)
        {
            BVDC_P_OR_ALL_DIRTY(pCurDirty, pOldDirty);
        }
        else
        {
            BVDC_P_CLEAN_ALL_DIRTY(pOldDirty);
        }
    }

    /* Let's see what we need to build. */
    if(BVDC_P_IS_DIRTY(pCurDirty) || h656In->hSource->bDeferSrcPendingCb)
    {
        BDBG_MSG(("eVideoFmt      = %s, dirty = %d",
            pCurInfo->pFmtInfo->pchFormatStr, pCurDirty->stBits.bInputFormat));
        BDBG_MSG(("pfGenCallback  = %d, dirty = %d",
            pCurInfo->pfGenericCallback, pCurDirty->stBits.bGenCallback));
        BDBG_MSG(("bVideoDetected = %d, dirty = %d",
            h656In->bVideoDetected, pCurDirty->stBits.bVideoDetected));
        BDBG_MSG(("eFrameRateCode = %d, dirty = %d,",
            h656In->eFrameRateCode, pCurDirty->stBits.bFrameRateCode));
        BDBG_MSG(("------------------------------intP%d", eFieldId));

        /* Note: The order is important because build NTSC/PAL contains a
         * reset, so it must precede other build ruls. */
        if(pCurDirty->stBits.bInputFormat)
        {
            h656In->hSource->ulVertFreq = pCurInfo->pFmtInfo->ulVertFreq;
            BVDC_P_656In_BuildResetRul_isr(h656In, pList);
            BVDC_P_656In_BuildFormatRul_isr(h656In, pList);
            BVDC_P_Drain_BuildFormatRul_isr(&h656In->hSource->stDrain,
                &h656In->hSource->stScanOut, pCurInfo->pFmtInfo, pList);
        }

        /* Callback when following things changes:
         * 1) Macrovision detection
         * 2) New format detected
         * 3) Video status */
        if((pCurInfo->pfGenericCallback) &&
           ((pCurDirty->stBits.bGenCallback)   ||
            (pCurDirty->stBits.bInputFormat)   ||
            (pCurDirty->stBits.bVideoDetected) ||
            (pCurDirty->stBits.bFrameRateCode) ||
            (h656In->hSource->bDeferSrcPendingCb) ||
            (pCurDirty->stBits.bAddWin && pCurInfo->eResumeMode)))
        {
            BVDC_Source_CallbackData *pCbData = &h656In->hSource->stSourceCbData;
            BVDC_Source_CallbackMask *pCbMask = &pCbData->stMask;

            /* Clear dirty bits */
            BVDC_P_CB_CLEAN_ALL_DIRTY(pCbMask);

            /* Issue src pending call back when shutdown BVN completed. */
            if(h656In->hSource->bDeferSrcPendingCb)
            {
                BVDC_P_Source_CheckAndIssueCallback_isr(h656In->hSource, pCbMask);
            }

            /* Make sure the callback happen at least once with status info,
             * NOT the source pending on first
             * installation of callback to report the current status. */
            if(pCurDirty->stBits.bGenCallback)
            {
                /* Which one triggers callback? */
                pCbMask->bActive     = BVDC_P_DIRTY;
                pCbMask->bFmtInfo    = BVDC_P_DIRTY;
                pCbMask->bFrameRate  = BVDC_P_DIRTY;
                h656In->hSource->bDeferSrcPendingCb = true;
            }
            else
            {
                /* Which one triggers callback? */
                pCbMask->bActive     = pCurDirty->stBits.bVideoDetected;
                pCbMask->bFmtInfo    = pCurDirty->stBits.bInputFormat;
                pCbMask->bFrameRate  = pCurDirty->stBits.bFrameRateCode;

                /* defer source pending callback until all its windows are shutdown! */
                if((pCurDirty->stBits.bInputFormat || pCurDirty->stBits.bAddWin) &&
                   (pCurInfo->eResumeMode))
                {
                    h656In->hSource->bDeferSrcPendingCb = true;
                }
            }

            /* callback only if something changed */
            if(BVDC_P_CB_IS_DIRTY_isr(pCbMask))
            {
                /* Update Callback data */
                pCbData->bActive        = h656In->bVideoDetected;
                pCbData->pFmtInfo       = pCurInfo->pFmtInfo;
                pCbData->eFrameRateCode = h656In->eFrameRateCode;
                if (pCbMask->bFrameRate)
                {
                    pCbData->ulVertRefreshRate =
                        BVDC_P_Source_RefreshRate_FromFrameRateCode_isrsafe(pCbData->eFrameRateCode);
                }

                pCurInfo->pfGenericCallback(pCurInfo->pvGenericParm1,
                    pCurInfo->iGenericParm2, (void*)pCbData);
            }
        }

        /* Clear dirty bits. */
        *pOldDirty = *pCurDirty;
        BVDC_P_CLEAN_ALL_DIRTY(pCurDirty);
    }

    /* This get build every vsync. */
    BVDC_P_656In_BuildVsyncRul_isr(h656In, pList);

    /* Countdown before start feeding out */
    if(h656In->ulDelayStart)
    {
        h656In->ulDelayStart--;
    }

    return ;
}
#endif

/* End of File */
