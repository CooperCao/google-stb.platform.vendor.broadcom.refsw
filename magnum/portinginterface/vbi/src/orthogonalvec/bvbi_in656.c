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
 * This module provides access to the IN656 core for the VBI porting
 * interface (BVBI).  This module is private to BVBI.
 *
 ***************************************************************************/

#include "bstd.h"           /* standard types */
#include "bdbg.h"           /* Dbglib */
#include "bvbi.h"           /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"      /* VBI internal data structures */

#if (BVBI_NUM_IN656 > 0) /** { **/

#include "bchp_in656_0.h"   /* RDB info for IN656 registers */
#if (BVBI_NUM_IN656 >= 2)
#include "bchp_in656_1.h"   /* RDB info for IN656 registers */
#endif

BDBG_MODULE(BVBI);

/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/
static void BVBI_P_IN656_Dec_Init (BREG_Handle hReg, uint32_t ulCoreOffset);


/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/


/***************************************************************************
* Implementation of supporting VBI_DEC functions that are not in API
***************************************************************************/


BERR_Code BVBI_P_IN656_Init( BVBI_P_Handle *pVbi )
{
    uint32_t ulCoreOffset;

    BDBG_ENTER(BVBI_P_IN656_Init);

    /* Initialize IN656 core */
    ulCoreOffset = 0x0;
    BVBI_P_IN656_Dec_Init (pVbi->hReg, ulCoreOffset);
#if (BVBI_NUM_IN656 >= 2)
    ulCoreOffset = BCHP_IN656_1_REV_ID - BCHP_IN656_0_REV_ID;
    BVBI_P_IN656_Dec_Init (pVbi->hReg, ulCoreOffset);
#endif

    BDBG_LEAVE(BVBI_P_IN656_Init);
    return BERR_SUCCESS;
}


BERR_Code BVBI_P_IN656_Dec_Program (
    BREG_Handle hReg,
    BMEM_Handle hMem,
    BAVC_SourceId eSource,
    bool bActive,
    BVBI_656Fmt anci656Fmt,
    BVBI_P_SMPTE291Moptions* pMoptions,
    BFMT_VideoFmt eVideoFormat,
    uint8_t* topData,
    uint8_t* botData)
{
/*
    Programming note: the implementation here assumes that the bitfield layout
    within registers is the same for all IN656 cores in the chip.

    If a chip is built that has multiple IN656 decoder cores that are not
    identical, then this routine will have to be redesigned.
*/
    uint32_t offset;
    uint32_t ulOffset;
    uint32_t ulReg;
    bool     isPal;
    BERR_Code eErr;

    BDBG_ENTER(BVBI_P_IN656_Dec_Program);

    /* Figure out which decoder core to use */
    switch (eSource)
    {
    case BAVC_SourceId_e656In0:
        ulOffset = 0;
        break;
#if (BVBI_NUM_IN656 >= 2)
    case BAVC_SourceId_e656In1:
        ulOffset = BCHP_IN656_1_REV_ID - BCHP_IN656_0_REV_ID;
        break;
#endif
    default:
        /* This should never happen!  This parameter was checked by
           BVBI_Decode_Create() */
        BDBG_LEAVE(BVBI_P_IN656_Dec_Program);
        return BERR_TRACE (BERR_INVALID_PARAMETER);
        break;
    }

    /* Determine whether PAL or NTSC */
    switch (eVideoFormat)
    {
    case BFMT_VideoFmt_eNTSC:
    case BFMT_VideoFmt_eNTSC_J:
    case BFMT_VideoFmt_e720x482_NTSC:
    case BFMT_VideoFmt_e720x482_NTSC_J:
    case BFMT_VideoFmt_e1080i:
    case BFMT_VideoFmt_e720p:
    case BFMT_VideoFmt_e480p:
    case BFMT_VideoFmt_e720x483p:
        isPal = false;
        break;

    case BFMT_VideoFmt_ePAL_B:
    case BFMT_VideoFmt_ePAL_B1:
    case BFMT_VideoFmt_ePAL_D:
    case BFMT_VideoFmt_ePAL_D1:
    case BFMT_VideoFmt_ePAL_G:
    case BFMT_VideoFmt_ePAL_H:
    case BFMT_VideoFmt_ePAL_K:
    case BFMT_VideoFmt_ePAL_I:
    case BFMT_VideoFmt_ePAL_M:
    case BFMT_VideoFmt_ePAL_N:
    case BFMT_VideoFmt_ePAL_NC:
    case BFMT_VideoFmt_eSECAM_L:
    case BFMT_VideoFmt_eSECAM_B:
    case BFMT_VideoFmt_eSECAM_G:
    case BFMT_VideoFmt_eSECAM_D:
    case BFMT_VideoFmt_eSECAM_K:
    case BFMT_VideoFmt_eSECAM_H:
    case BFMT_VideoFmt_e1080i_50Hz:
    case BFMT_VideoFmt_e720p_50Hz:
    case BFMT_VideoFmt_e576p_50Hz:
        isPal = true;
        break;

    default:
        BDBG_LEAVE(BVBI_P_IN656_Dec_Program);
        BDBG_ERR(("BVBI_IN656: video format %d not supported", eVideoFormat));
        return BERR_TRACE (BERR_INVALID_PARAMETER);
        break;
    }

    /* Program the encapsulation method */
    ulReg = BREG_Read32 (hReg,  BCHP_IN656_0_STRM_CNTRL + ulOffset);
    ulReg &= ~(
        BCHP_MASK (IN656_0_STRM_CNTRL, SMPTE_COUNT_TYPE) |
        BCHP_MASK (IN656_0_STRM_CNTRL,          TEST) |
        BCHP_MASK (IN656_0_STRM_CNTRL,       MODE_FP) |
        BCHP_MASK (IN656_0_STRM_CNTRL,     VCNT_LAST) |
        BCHP_MASK (IN656_0_STRM_CNTRL,        VBLANK) |
        BCHP_MASK (IN656_0_STRM_CNTRL,          TYPE) |
        BCHP_MASK (IN656_0_STRM_CNTRL, ANCILLARY_WIN) |
        BCHP_MASK (IN656_0_STRM_CNTRL, ANCILLARY_PKT) );
    switch (anci656Fmt)
    {
    case BVBI_656Fmt_SAA7113:
        ulReg |=  BCHP_FIELD_ENUM (IN656_0_STRM_CNTRL, TYPE, SAA7113);
        break;
    case BVBI_656Fmt_SAA7114A:
        ulReg |=  BCHP_FIELD_ENUM (IN656_0_STRM_CNTRL, TYPE, SAA7114A);
        break;
    case BVBI_656Fmt_SAA7114B:
        ulReg |=  BCHP_FIELD_ENUM (IN656_0_STRM_CNTRL, TYPE, SAA7114B);
        break;
    case BVBI_656Fmt_SAA7114C:
        ulReg |=  BCHP_FIELD_ENUM (IN656_0_STRM_CNTRL, TYPE, SAA7114C);
        break;
    case BVBI_656Fmt_SAA7115:
        ulReg |=  BCHP_FIELD_ENUM (IN656_0_STRM_CNTRL, TYPE, SAA7115);
        break;
    case BVBI_656Fmt_SMPTE291:
        ulReg |=  BCHP_FIELD_ENUM (IN656_0_STRM_CNTRL, TYPE, SMPTE291);
        if (pMoptions->bBrokenDataCount)
        {
            ulReg |=
                BCHP_FIELD_ENUM (IN656_0_STRM_CNTRL, SMPTE_COUNT_TYPE, BYTE);
        }
        else
        {
            ulReg |=
                BCHP_FIELD_ENUM (IN656_0_STRM_CNTRL, SMPTE_COUNT_TYPE, DWORD);
        }
        break;
    default:
        BDBG_LEAVE(BVBI_P_IN656_Dec_Program);
        return BERR_TRACE (BERR_INVALID_PARAMETER);
        break;
    }

    /* Put in a reasonable value for other attributes */
    /* TODO: use enums when RDB and headers are fixed. */
    ulReg |=  BCHP_FIELD_ENUM (IN656_0_STRM_CNTRL, TEST, BUS_0);
    ulReg |=  BCHP_FIELD_ENUM (IN656_0_STRM_CNTRL, MODE_FP, AUTO);
    ulReg |=  BCHP_FIELD_DATA (IN656_0_STRM_CNTRL, VCNT_LAST, 0x22);
    ulReg |=  BCHP_FIELD_DATA (IN656_0_STRM_CNTRL, VBLANK, 0);
    ulReg |=  BCHP_FIELD_ENUM (IN656_0_STRM_CNTRL, DATA_MSB_POLARITY, ONE);
    ulReg |=  BCHP_FIELD_ENUM (IN656_0_STRM_CNTRL, ANCILLARY_WIN, ENABLE);

    /* Turn on or off */
    /* TODO: use enums when RDB and headers are fixed. */
    ulReg &= ~BCHP_MASK (IN656_0_STRM_CNTRL, ANCILLARY_PKT);
    if (bActive)
        ulReg |=  BCHP_FIELD_DATA (IN656_0_STRM_CNTRL, ANCILLARY_PKT, 1);
    else
        ulReg |=  BCHP_FIELD_DATA (IN656_0_STRM_CNTRL, ANCILLARY_PKT, 0);

    /* Done with one register */
    BREG_Write32 (hReg, BCHP_IN656_0_STRM_CNTRL + ulOffset, ulReg);

    /* Program the window for ancillary packet acceptance */
    ulReg = BREG_Read32 (hReg, BCHP_IN656_0_STRM_WIN + ulOffset);
    ulReg &= ~(
        BCHP_MASK      (IN656_0_STRM_WIN, END   ) |
        BCHP_MASK      (IN656_0_STRM_WIN, START ) );
    ulReg |= BCHP_FIELD_DATA (IN656_0_STRM_WIN, START, 0x16A);
    ulReg |= BCHP_FIELD_DATA (IN656_0_STRM_WIN, END, 0x1AD);
    /*
    if (isPal)
    {
        ulReg |= BCHP_FIELD_DATA (IN656_0_STRM_WIN, END, 432);
    }
    */
    BREG_Write32 (hReg, BCHP_IN656_0_STRM_WIN + ulOffset, ulReg);

    /* Tell the hardware where to put ancillary data packets that it finds */
    eErr = BERR_TRACE (BMEM_ConvertAddressToOffset (hMem, topData, &offset));
    if (eErr != BERR_SUCCESS)
    {
        BDBG_LEAVE(BVBI_P_IN656_Dec_Program);
        return eErr;
    }
    BREG_Write32 (hReg, BCHP_IN656_0_FLD_0_PTR + ulOffset, offset);
    eErr = BERR_TRACE (BMEM_ConvertAddressToOffset (hMem, botData, &offset));
    if (eErr != BERR_SUCCESS)
    {
        BDBG_LEAVE(BVBI_P_IN656_Dec_Program);
        return eErr;
    }
    BREG_Write32 ( hReg, BCHP_IN656_0_FLD_1_PTR + ulOffset, offset);

    BDBG_LEAVE(BVBI_P_IN656_Dec_Program);
    return BERR_SUCCESS;
}

BERR_Code BVBI_P_IN656_Decode_Data_isr (
    BREG_Handle hReg,
    BAVC_SourceId eSource,
    BAVC_Polarity polarity,
    bool* bDataFound)
{
/*
    Programming note: the implementation here assumes that the bitfield layout
    within registers is the same for all IN656 cores in the chip.

    If a chip is built that has multiple IN656 decoder cores that are not
    identical, then this routine will have to be redesigned.
*/
    uint32_t ulOffset;
    uint32_t ulErrors;
    uint32_t ulWriteComplete;
    uint32_t ulWriteCompleteMask;
    uint32_t ulReg;
    BERR_Code eErr;

    BDBG_ENTER(BVBI_P_IN656_Decode_Data_isr);

    /* Figure out which decoder core to use */
    switch (eSource)
    {
    case BAVC_SourceId_e656In0:
        ulOffset = 0;
        break;
#if (BVBI_NUM_IN656 >= 2)
        ulOffset = BCHP_IN656_1_REV_ID - BCHP_IN656_0_REV_ID;
        break;
#endif
    default:
        /* This should never happen!  This parameter was checked by
           BVBI_Decode_Create() */
        *bDataFound = false;
        BDBG_LEAVE(BVBI_P_IN656_Decode_Data_isr);
        return BERR_TRACE (BERR_INVALID_PARAMETER);
        break;
    }

    /* Choose top vs bottom field */
    switch (polarity)
    {
    case BAVC_Polarity_eTopField:
        ulWriteCompleteMask = BCHP_MASK (IN656_0_WRITE_COMPLETE, F0_COMPLETE);
        break;
    case BAVC_Polarity_eBotField:
        ulWriteCompleteMask = BCHP_MASK (IN656_0_WRITE_COMPLETE, F1_COMPLETE);
        break;
    default:
        *bDataFound = false;
        BDBG_LEAVE(BVBI_P_IN656_Decode_Data_isr);
        return BERR_TRACE (BERR_INVALID_PARAMETER);
        break;
    }

    /* Check and clear error status */
    ulReg = BREG_Read32 (hReg, BCHP_IN656_0_ERROR_STATUS + ulOffset);
    ulErrors = ulReg & (
        BCHP_MASK (IN656_0_ERROR_STATUS, FIFO_FULL    ) |
        BCHP_MASK (IN656_0_ERROR_STATUS, BAD_AP_PREFIX) |
        BCHP_MASK (IN656_0_ERROR_STATUS, BAD_SAV      ) );
    BREG_Write32 (hReg, BCHP_IN656_0_ERROR_STATUS + ulOffset, ulErrors);

    /* Check and clear capture status */
    ulReg = BREG_Read32 (hReg, BCHP_IN656_0_WRITE_COMPLETE + ulOffset);
    ulWriteComplete = ulReg & ulWriteCompleteMask;
    BREG_Write32 (
        hReg, BCHP_IN656_0_WRITE_COMPLETE + ulOffset, ulWriteComplete);
    *bDataFound =  ((ulErrors == 0) && (ulWriteComplete != 0));

    BDBG_LEAVE(BVBI_P_IN656_Decode_Data_isr);
    eErr =  (ulErrors == 0) ? BERR_SUCCESS : BVBI_ERR_FIELD_BADDATA;

    return eErr;
}

/***************************************************************************
* Static (private) functions
***************************************************************************/

/***************************************************************************
 *
 */
static void BVBI_P_IN656_Dec_Init (BREG_Handle hReg, uint32_t ulCoreOffset)
{
    uint32_t ulReg;

    BDBG_ENTER(BVBI_P_IN656_Dec_Init);

    /* Reset the core */
    ulReg = 0;
    BREG_Write32 (hReg, BCHP_IN656_0_RESET + ulCoreOffset, ulReg);

    /* Disable the action of the core */
    /* TODO: use enums when RDB and headers are fixed. */
    ulReg = BREG_Read32 (hReg, BCHP_IN656_0_STRM_CNTRL + ulCoreOffset);
    ulReg &= ~BCHP_MASK       (IN656_0_STRM_CNTRL, ANCILLARY_PKT);
    ulReg |=  BCHP_FIELD_DATA (IN656_0_STRM_CNTRL, ANCILLARY_PKT, 0);
    BREG_Write32 (hReg, BCHP_IN656_0_STRM_CNTRL + ulCoreOffset, ulReg);

    BDBG_LEAVE(BVBI_P_IN656_Dec_Init);
}

#endif /** } (BVBI_NUM_IN656 > 0) **/

/* End of file */
