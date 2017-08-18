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

#include "bstd.h"           /* standard types */
#include "bdbg.h"           /* Dbglib */
#include "bkni.h"           /* For critical sections */
#include "bvbi.h"           /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"      /* VBI internal data structures */
#if (BVBI_NUM_CCE >= 1)
#include "bchp_cce_0.h"  /* RDB info for primary CCE core */
#endif
#if (BVBI_NUM_CCE_656 >= 1)
#include "bchp_cce_ancil_0.h" /* RDB info for ITU-R 656 passthrough CCE core */
#endif

BDBG_MODULE(BVBI);


/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/
#if BVBI_NUM_CCE >= 1
static uint32_t P_GetCoreOffset_isr (uint8_t hwCoreIndex);
#endif
#if BVBI_NUM_CCE_656 >= 1
static uint32_t P_GetCoreOffset_656_isr (uint8_t hwCoreIndex);
#endif


/***************************************************************************
* Implementation supporting closed caption functions that are not in API
***************************************************************************/

#if BVBI_NUM_CCE >= 1
BERR_Code BVBI_P_MCC_Enc_Program (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    bool bActive,
    BFMT_VideoFmt eVideoFormat,
    bool bArib480p)
{
/*
    Programming note: the implementation here assumes that the bitfield layout
    within registers is the same for all CC encoder cores in the chip.

    If a chip is built that has multiple CC encoder cores that are not
    identical, then this routine will have to be redesigned.
*/
    uint32_t ulCoreOffset;
    uint32_t ulControlReg;
    uint32_t ulGainDelayReg = 0;
    uint32_t ulGain = 0;
    uint32_t ulDelayCount = 0;
    uint32_t ulScteBaseReg = 0;
    uint32_t ulLineTop = 0;
    uint32_t ulLineBot = 0;

    BDBG_ENTER(BVBI_P_MCC_Enc_Program);

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_isr (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_MCC_Enc_Program);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* If user wants to turn off closed caption processing, just use the
       enable bit. */
    if (!bActive)
    {
        ulControlReg =
            BREG_Read32 ( hReg, BCHP_CCE_0_Control + ulCoreOffset );
        ulControlReg &=
            ~BCHP_MASK      (CCE_0_Control, ENABLE_CLOSED_CAPTION          );
        ulControlReg |=
            BCHP_FIELD_ENUM (CCE_0_Control, ENABLE_CLOSED_CAPTION, DISABLED);
        BREG_Write32 (
            hReg, BCHP_CCE_0_Control + ulCoreOffset, ulControlReg );
        BDBG_LEAVE(BVBI_P_MCC_Enc_Program);
        return BERR_SUCCESS;
    }

    /* Select video format */
    switch (eVideoFormat)
    {
    case BFMT_VideoFmt_eNTSC:
    case BFMT_VideoFmt_eNTSC_J:
    case BFMT_VideoFmt_e720x482_NTSC:
    case BFMT_VideoFmt_e720x482_NTSC_J:
    case BFMT_VideoFmt_ePAL_M:
        ulGain = 0x47;
        ulDelayCount = 0x3A;

        /* PAL_M always seems to have an issue */
        if (eVideoFormat == BFMT_VideoFmt_ePAL_M)
        {
            ulGain = 0x4A;
            ulDelayCount = 0xD;
        }
        else
        {
            ulLineTop = 8;
            ulLineBot = 271;
            if (bArib480p)
            {
                --ulLineTop;
                --ulLineBot;
            }
        }
        break;

    case BFMT_VideoFmt_ePAL_B:
    case BFMT_VideoFmt_ePAL_B1:
    case BFMT_VideoFmt_ePAL_D:
    case BFMT_VideoFmt_ePAL_D1:
    case BFMT_VideoFmt_ePAL_G:
    case BFMT_VideoFmt_ePAL_H:
    case BFMT_VideoFmt_ePAL_K:
    case BFMT_VideoFmt_ePAL_I:
    case BFMT_VideoFmt_ePAL_N:
    case BFMT_VideoFmt_ePAL_NC:
    case BFMT_VideoFmt_eSECAM_L:
    case BFMT_VideoFmt_eSECAM_B:
    case BFMT_VideoFmt_eSECAM_G:
    case BFMT_VideoFmt_eSECAM_D:
    case BFMT_VideoFmt_eSECAM_K:
    case BFMT_VideoFmt_eSECAM_H:
        /* set the format specific gain delay values, if any. */
        ulGain = 0x46;
        ulDelayCount = 0x01;

        /* SCTE line origin */
        ulLineTop = 5;
        ulLineBot = 318;
        break;

    default:
        BKNI_LeaveCriticalSection();
        BDBG_LEAVE(BVBI_P_MCC_Enc_Program);
        BDBG_ERR(("BVBI_MCCE: video format %d not supported", eVideoFormat));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
        break;
    }

    BKNI_EnterCriticalSection();

    /* write the gain delay register */
    ulGainDelayReg = BREG_Read32 (
        hReg, BCHP_CCE_0_Gain_Delay + ulCoreOffset );
    ulGainDelayReg &= ~(
        BCHP_MASK       ( CCE_0_Gain_Delay, GAIN                     ) |
        BCHP_MASK       ( CCE_0_Gain_Delay, DELAY_COUNT              )   );
    ulGainDelayReg |= (
        BCHP_FIELD_DATA ( CCE_0_Gain_Delay, GAIN,              ulGain) |
        BCHP_FIELD_DATA ( CCE_0_Gain_Delay, DELAY_COUNT, ulDelayCount)   );
    BREG_Write32 (
        hReg, BCHP_CCE_0_Gain_Delay + ulCoreOffset, ulGainDelayReg );

    /* Write the SCTE base lines register */
    ulScteBaseReg = BREG_Read32 (
        hReg, BCHP_CCE_0_SCTE_Base_Lines + ulCoreOffset );
    ulScteBaseReg &= ~(
        BCHP_MASK       ( CCE_0_SCTE_Base_Lines, LINE_TOP           ) |
        BCHP_MASK       ( CCE_0_SCTE_Base_Lines, LINE_BOT           )   );
    ulScteBaseReg |= (
        BCHP_FIELD_DATA ( CCE_0_SCTE_Base_Lines, LINE_TOP, ulLineTop) |
        BCHP_FIELD_DATA ( CCE_0_SCTE_Base_Lines, LINE_BOT, ulLineBot)   );
    BREG_Write32 (
        hReg, BCHP_CCE_0_SCTE_Base_Lines +ulCoreOffset, ulScteBaseReg );

    /* program the control register with non-format specific values */
    ulControlReg = BREG_Read32 ( hReg, BCHP_CCE_0_Control + ulCoreOffset );
    ulControlReg &= ~(
        BCHP_MASK      (CCE_0_Control, reserved0                           )|
        BCHP_MASK      (CCE_0_Control, reserved_for_eco1                   )|
        BCHP_MASK      (CCE_0_Control, SCTE_MODE                           )|
        BCHP_MASK      (CCE_0_Control, TOP_FLD_PARITY                      )|
        BCHP_MASK      (CCE_0_Control, BOT_FLD_PARITY                      )|
        BCHP_MASK      (CCE_0_Control, TOP_FLD_STAT                        )|
        BCHP_MASK      (CCE_0_Control, BOT_FLD_STAT                        )|
        BCHP_MASK      (CCE_0_Control, NULL_CHARACTER                      )|
        BCHP_MASK      (CCE_0_Control, BYTEIF_ENDIAN_ORDER                 )|
        BCHP_MASK      (CCE_0_Control, BYTE_SWAP                           )|
        BCHP_MASK      (CCE_0_Control, SHIFT_DIRECTION                     )|
        BCHP_MASK      (CCE_0_Control, reserved2                           )|
        BCHP_MASK      (CCE_0_Control, REGISTER_USE_MODE                   )|
        BCHP_MASK      (CCE_0_Control, ENABLE_CLOSED_CAPTION               )
    );
    ulControlReg |= (
        BCHP_FIELD_DATA(CCE_0_Control, reserved0,             0            )|
        BCHP_FIELD_DATA(CCE_0_Control, reserved_for_eco1,     0            )|
        BCHP_FIELD_ENUM( CCE_0_Control, SCTE_MODE,            SCTE_ON      )|
        BCHP_FIELD_ENUM(CCE_0_Control, TOP_FLD_PARITY,        AUTOMATIC    )|
        BCHP_FIELD_ENUM(CCE_0_Control, BOT_FLD_PARITY,        AUTOMATIC    )|
        BCHP_FIELD_DATA(CCE_0_Control, TOP_FLD_STAT,          0            )|
        BCHP_FIELD_DATA(CCE_0_Control, BOT_FLD_STAT,          0            )|
        BCHP_FIELD_DATA(CCE_0_Control, NULL_CHARACTER,        0x80         )|
        BCHP_FIELD_ENUM(CCE_0_Control, BYTEIF_ENDIAN_ORDER,   MAINTAIN     )|
        BCHP_FIELD_ENUM(CCE_0_Control, BYTE_SWAP,             LITTLE_ENDIAN)|
        BCHP_FIELD_ENUM(CCE_0_Control, SHIFT_DIRECTION,       LSB2MSB      )|
        BCHP_FIELD_DATA(CCE_0_Control, reserved2,             0            )|
        BCHP_FIELD_ENUM(CCE_0_Control, REGISTER_USE_MODE,     SPLIT        )|
        BCHP_FIELD_ENUM(CCE_0_Control, ENABLE_CLOSED_CAPTION, ENABLED      )
    );

    /* Format register for PAL vs NTSC */
    ulControlReg &= ~BCHP_MASK ( CCE_0_Control, VIDEO_FORMAT      );
    if ((eVideoFormat == BFMT_VideoFmt_eNTSC  ) ||
        (eVideoFormat == BFMT_VideoFmt_eNTSC_J) ||
        (eVideoFormat == BFMT_VideoFmt_e720x482_NTSC) ||
        (eVideoFormat == BFMT_VideoFmt_e720x482_NTSC_J) ||
        (eVideoFormat == BFMT_VideoFmt_ePAL_M )  )
    {
        ulControlReg |= BCHP_FIELD_ENUM(CCE_0_Control, VIDEO_FORMAT, NTSC);
    }
    else /* eVideoFormat == BFMT_VideoFmt_ePAL_whatever */
    {
        ulControlReg |= BCHP_FIELD_ENUM(CCE_0_Control, VIDEO_FORMAT, PAL );
    }

    /* Write the finished control register value, finally. */
    BREG_Write32 ( hReg, BCHP_CCE_0_Control + ulCoreOffset, ulControlReg );

    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BVBI_P_MCC_Enc_Program);
    return BERR_SUCCESS;
}
#endif

#if BVBI_NUM_CCE_656 >= 1
BERR_Code BVBI_P_MCC_Enc_656_Program (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    bool bActive,
    BFMT_VideoFmt eVideoFormat,
    bool bArib480p)
{
/*
    Programming note: the implementation here assumes that the bitfield layout
    within registers is the same for all CC encoder cores in the chip.

    If a chip is built that has multiple CC encoder cores that are not
    identical, then this routine will have to be redesigned.
*/
    uint32_t ulCoreOffset;
    uint32_t ulControlReg;
    uint32_t ulScteBaseReg = 0;
    uint32_t ulLineTop = 0;
    uint32_t ulLineBot = 0;

    BDBG_ENTER(BVBI_P_MCC_Enc_656_Program);

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_656_isr (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_MCC_Enc_656_Program);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* If user wants to turn off closed caption processing, just use the
       enable bit. */
    if (!bActive)
    {
        ulControlReg =
            BREG_Read32 ( hReg, BCHP_CCE_ANCIL_0_Ancil_Control + ulCoreOffset );
        ulControlReg &=
            ~BCHP_MASK      (CCE_ANCIL_0_Ancil_Control, ENABLE_CLOSED_CAPTION          );
        ulControlReg |=
            BCHP_FIELD_ENUM (CCE_ANCIL_0_Ancil_Control, ENABLE_CLOSED_CAPTION, DISABLED);
        BREG_Write32 (
            hReg, BCHP_CCE_ANCIL_0_Ancil_Control + ulCoreOffset, ulControlReg );
        BDBG_LEAVE(BVBI_P_MCC_Enc_656_Program);
        return BERR_SUCCESS;
    }

    /* Select video format */
    switch (eVideoFormat)
    {
    case BFMT_VideoFmt_eNTSC:
    case BFMT_VideoFmt_eNTSC_J:
    case BFMT_VideoFmt_e720x482_NTSC:
    case BFMT_VideoFmt_e720x482_NTSC_J:
    case BFMT_VideoFmt_ePAL_M:

        /* PAL_M always seems to have an issue */
        if (eVideoFormat != BFMT_VideoFmt_ePAL_M)
        {
            ulLineTop = 8;
            ulLineBot = 271;
            if (bArib480p)
            {
                --ulLineTop;
                --ulLineBot;
            }
        }
        break;

    case BFMT_VideoFmt_ePAL_B:
    case BFMT_VideoFmt_ePAL_B1:
    case BFMT_VideoFmt_ePAL_D:
    case BFMT_VideoFmt_ePAL_D1:
    case BFMT_VideoFmt_ePAL_G:
    case BFMT_VideoFmt_ePAL_H:
    case BFMT_VideoFmt_ePAL_K:
    case BFMT_VideoFmt_ePAL_I:
    case BFMT_VideoFmt_ePAL_N:
    case BFMT_VideoFmt_ePAL_NC:
    case BFMT_VideoFmt_eSECAM_L:
    case BFMT_VideoFmt_eSECAM_B:
    case BFMT_VideoFmt_eSECAM_G:
    case BFMT_VideoFmt_eSECAM_D:
    case BFMT_VideoFmt_eSECAM_K:
    case BFMT_VideoFmt_eSECAM_H:
        /* SCTE line origin */
        ulLineTop = 5;
        ulLineBot = 318;
        break;

    default:
        BKNI_LeaveCriticalSection();
        BDBG_LEAVE(BVBI_P_MCC_Enc_656_Program);
        BDBG_ERR(("BVBI_MCCE: video format %d not supported", eVideoFormat));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
        break;
    }

    BKNI_EnterCriticalSection();

    /* Write the SCTE base lines register */
    ulScteBaseReg = BREG_Read32 (
        hReg, BCHP_CCE_ANCIL_0_Ancil_SCTE_Base_Lines + ulCoreOffset );
    ulScteBaseReg &= ~(
        BCHP_MASK       ( CCE_ANCIL_0_Ancil_SCTE_Base_Lines, LINE_TOP           ) |
        BCHP_MASK       ( CCE_ANCIL_0_Ancil_SCTE_Base_Lines, LINE_BOT           )   );
    ulScteBaseReg |= (
        BCHP_FIELD_DATA ( CCE_ANCIL_0_Ancil_SCTE_Base_Lines, LINE_TOP, ulLineTop) |
        BCHP_FIELD_DATA ( CCE_ANCIL_0_Ancil_SCTE_Base_Lines, LINE_BOT, ulLineBot)   );
    BREG_Write32 (
        hReg, BCHP_CCE_ANCIL_0_Ancil_SCTE_Base_Lines +ulCoreOffset, ulScteBaseReg );

    /* program the control register with non-format specific values */
    ulControlReg = BREG_Read32 ( hReg, BCHP_CCE_ANCIL_0_Ancil_Control + ulCoreOffset );
    ulControlReg &= ~(
        BCHP_MASK      (CCE_ANCIL_0_Ancil_Control, reserved0                           )|
        BCHP_MASK      (CCE_ANCIL_0_Ancil_Control, reserved_for_eco1                   )|
        BCHP_MASK      (CCE_ANCIL_0_Ancil_Control, SCTE_MODE                           )|
        BCHP_MASK      (CCE_ANCIL_0_Ancil_Control, TOP_FLD_PARITY                      )|
        BCHP_MASK      (CCE_ANCIL_0_Ancil_Control, BOT_FLD_PARITY                      )|
        BCHP_MASK      (CCE_ANCIL_0_Ancil_Control, TOP_FLD_STAT                        )|
        BCHP_MASK      (CCE_ANCIL_0_Ancil_Control, BOT_FLD_STAT                        )|
        BCHP_MASK      (CCE_ANCIL_0_Ancil_Control, NULL_CHARACTER                      )|
        BCHP_MASK      (CCE_ANCIL_0_Ancil_Control, BYTEIF_ENDIAN_ORDER                 )|
        BCHP_MASK      (CCE_ANCIL_0_Ancil_Control, BYTE_SWAP                           )|
        BCHP_MASK      (CCE_ANCIL_0_Ancil_Control, SHIFT_DIRECTION                     )|
        BCHP_MASK      (CCE_ANCIL_0_Ancil_Control, reserved2                           )|
        BCHP_MASK      (CCE_ANCIL_0_Ancil_Control, REGISTER_USE_MODE                   )|
        BCHP_MASK      (CCE_ANCIL_0_Ancil_Control, ENABLE_CLOSED_CAPTION               )
    );
    ulControlReg |= (
        BCHP_FIELD_DATA(CCE_ANCIL_0_Ancil_Control, reserved0,             0            )|
        BCHP_FIELD_DATA(CCE_ANCIL_0_Ancil_Control, reserved_for_eco1,     0            )|
        BCHP_FIELD_ENUM( CCE_ANCIL_0_Ancil_Control, SCTE_MODE,            SCTE_ON      )|
        BCHP_FIELD_ENUM(CCE_ANCIL_0_Ancil_Control, TOP_FLD_PARITY,        AUTOMATIC    )|
        BCHP_FIELD_ENUM(CCE_ANCIL_0_Ancil_Control, BOT_FLD_PARITY,        AUTOMATIC    )|
        BCHP_FIELD_DATA(CCE_ANCIL_0_Ancil_Control, TOP_FLD_STAT,          0            )|
        BCHP_FIELD_DATA(CCE_ANCIL_0_Ancil_Control, BOT_FLD_STAT,          0            )|
        BCHP_FIELD_DATA(CCE_ANCIL_0_Ancil_Control, NULL_CHARACTER,        0x80         )|
        BCHP_FIELD_ENUM(CCE_ANCIL_0_Ancil_Control, BYTEIF_ENDIAN_ORDER,   MAINTAIN     )|
        BCHP_FIELD_ENUM(CCE_ANCIL_0_Ancil_Control, BYTE_SWAP,             LITTLE_ENDIAN)|
        BCHP_FIELD_ENUM(CCE_ANCIL_0_Ancil_Control, SHIFT_DIRECTION,       LSB2MSB      )|
        BCHP_FIELD_DATA(CCE_ANCIL_0_Ancil_Control, reserved2,             0            )|
        BCHP_FIELD_ENUM(CCE_ANCIL_0_Ancil_Control, REGISTER_USE_MODE,     SPLIT        )|
        BCHP_FIELD_ENUM(CCE_ANCIL_0_Ancil_Control, ENABLE_CLOSED_CAPTION, ENABLED      )
    );

    /* Format register for PAL vs NTSC */
    ulControlReg &= ~BCHP_MASK ( CCE_ANCIL_0_Ancil_Control, VIDEO_FORMAT      );
    if ((eVideoFormat == BFMT_VideoFmt_eNTSC  ) ||
        (eVideoFormat == BFMT_VideoFmt_eNTSC_J) ||
        (eVideoFormat == BFMT_VideoFmt_e720x482_NTSC) ||
        (eVideoFormat == BFMT_VideoFmt_e720x482_NTSC_J) ||
        (eVideoFormat == BFMT_VideoFmt_ePAL_M )  )
    {
        ulControlReg |= BCHP_FIELD_ENUM(CCE_ANCIL_0_Ancil_Control, VIDEO_FORMAT, NTSC);
    }
    else /* eVideoFormat == BFMT_VideoFmt_ePAL_whatever */
    {
        ulControlReg |= BCHP_FIELD_ENUM(CCE_ANCIL_0_Ancil_Control, VIDEO_FORMAT, PAL );
    }

    /* Write the finished control register value, finally. */
    BREG_Write32 ( hReg, BCHP_CCE_ANCIL_0_Ancil_Control + ulCoreOffset, ulControlReg );

    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BVBI_P_MCC_Enc_656_Program);
    return BERR_SUCCESS;
}
#endif

#if BVBI_NUM_CCE >= 1
uint32_t BVBI_P_MCC_Encode_Data_isr (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    BAVC_Polarity polarity,
    BVBI_MCCData* pMCCData)
{
    int iLine;
    unsigned int numSet;
    uint8_t *datum;
    uint8_t  ucRegNum;
    uint16_t usWord;
    uint32_t ulCoreOffset;
    uint32_t ulDataReg;
    uint32_t ulDataRegAddr;
    uint32_t ulLinesRegAddr;
    uint32_t ulLinesReg;
    uint32_t lineMask;
    uint32_t ulErrInfo = 0;

    BDBG_ENTER(BVBI_P_MCC_Encode_Data_isr);

    /* Get register offset */
    ulCoreOffset = P_GetCoreOffset_isr (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_MCC_Encode_Data_isr);
        return (uint32_t)(-1);
    }

    /* Choose first data register */
    ucRegNum =  (uint8_t)(polarity == BAVC_Polarity_eTopField) ? 0 : 3;
    ulDataRegAddr =
        BCHP_CCE_0_Data0 + ulCoreOffset + (sizeof(uint32_t) * ucRegNum);

    /* Prepare to index into user data */
    datum = pMCCData->ucData;
    numSet = 0;
    lineMask = pMCCData->uhLineMask;
    /* PAL-M always seems to cause problems */
    if (eVideoFormat == BFMT_VideoFmt_ePAL_M)
        lineMask >>= 3;

    /* Loop over possible video lines */
    for (iLine = 0 ; iLine < 16 ; ++iLine)
    {
        /* If current video line is selected */
        if (lineMask & 0x1)
        {
            /* Get existing data register value */
            ulDataReg = BREG_Read32 (hReg, ulDataRegAddr);

            /* Encode VBI data back into register value */
            usWord  = *datum++;
            usWord |= ((uint16_t)(*datum++) << 8);
            /* usWord = BVBI_P_SetCCParityBits_isr (usWord); */
            if (numSet & 0x1)
            {
                ulDataReg &= ~BCHP_MASK       (CCE_0_Data0, WORD1        );
                ulDataReg |=  BCHP_FIELD_DATA (CCE_0_Data0, WORD1, usWord);
            }
            else
            {
                ulDataReg &= ~BCHP_MASK       (CCE_0_Data0, WORD0        );
                ulDataReg |=  BCHP_FIELD_DATA (CCE_0_Data0, WORD0, usWord);
            }

            /* Write the data register back */
            BREG_Write32 (hReg, ulDataRegAddr, ulDataReg);

            /* Advance counters */
            if (numSet & 0x1)
            {
                ++ucRegNum;
                ulDataRegAddr =
                    BCHP_CCE_0_Data0 + ulCoreOffset +
                        (sizeof(uint32_t) * ucRegNum);
            }
            ++numSet;
        }

        /* Advance */
        lineMask >>= 1;
    }

    /* Adjust line mask for hardware oddities */
    lineMask = pMCCData->uhLineMask;
    /* PAL-M always seems to cause problems */
    if (eVideoFormat == BFMT_VideoFmt_ePAL_M)
        lineMask >>= 3;

    /* Finally, set active lines register. */
    switch (polarity)
    {
    case BAVC_Polarity_eTopField:
        ulLinesRegAddr = BCHP_CCE_0_Active_Lines + ulCoreOffset;
        ulLinesReg =
            BREG_Read32 ( hReg,  ulLinesRegAddr );
        ulLinesReg &= ~BCHP_MASK       (
            CCE_0_Active_Lines, SCTE_ON_SCTE_TOP_ACTIVE);
        ulLinesReg |=  BCHP_FIELD_DATA (
            CCE_0_Active_Lines, SCTE_ON_SCTE_TOP_ACTIVE, lineMask );
            break;
    case BAVC_Polarity_eBotField:
        ulLinesRegAddr = BCHP_CCE_0_Active_Lines_1 + ulCoreOffset;
        ulLinesReg =
            BREG_Read32 ( hReg,  ulLinesRegAddr );
        ulLinesReg &= ~BCHP_MASK       (
            CCE_0_Active_Lines_1, SCTE_BOT_ACTIVE);
        ulLinesReg |=  BCHP_FIELD_DATA (
            CCE_0_Active_Lines_1, SCTE_BOT_ACTIVE, lineMask );
            break;
        default:
            BDBG_LEAVE(BVBI_P_MCC_Encode_Data_isr);
            return (uint32_t)(-1);
            break;
    }
    BREG_Write32 ( hReg, ulLinesRegAddr, ulLinesReg );

    BDBG_LEAVE(BVBI_P_MCC_Encode_Data_isr);
    return ulErrInfo;
}
#endif

#if BVBI_NUM_CCE_656 >= 1
uint32_t BVBI_P_MCC_Encode_656_Data_isr (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    BAVC_Polarity polarity,
    BVBI_MCCData* pMCCData)
{
    int iLine;
    unsigned int numSet;
    uint8_t *datum;
    uint8_t  ucRegNum;
    uint16_t usWord;
    uint32_t ulCoreOffset;
    uint32_t ulDataReg;
    uint32_t ulDataRegAddr;
    uint32_t ulLinesRegAddr;
    uint32_t ulLinesReg;
    uint32_t lineMask;
    uint32_t ulErrInfo = 0;

    BDBG_ENTER(BVBI_P_MCC_Encode_656_Data_isr);

    /* Get register offset */
    ulCoreOffset = P_GetCoreOffset_656_isr (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_MCC_Encode_656_Data_isr);
        return (uint32_t)(-1);
    }

    /* Choose first data register */
    ucRegNum =  (uint8_t)(polarity == BAVC_Polarity_eTopField) ? 0 : 3;
    ulDataRegAddr =
        BCHP_CCE_ANCIL_0_Ancil_Data0 + ulCoreOffset + (sizeof(uint32_t) * ucRegNum);

    /* Prepare to index into user data */
    datum = pMCCData->ucData;
    numSet = 0;
    lineMask = pMCCData->uhLineMask;
    /* PAL-M always seems to cause problems */
    if (eVideoFormat == BFMT_VideoFmt_ePAL_M)
        lineMask >>= 3;

    /* Loop over possible video lines */
    for (iLine = 0 ; iLine < 16 ; ++iLine)
    {
        /* If current video line is selected */
        if (lineMask & 0x1)
        {
            /* Get existing data register value */
            ulDataReg = BREG_Read32 (hReg, ulDataRegAddr);

            /* Encode VBI data back into register value */
            usWord  = *datum++;
            usWord |= ((uint16_t)(*datum++) << 8);
            /* usWord = BVBI_P_SetCCParityBits_isr (usWord); */
            if (numSet & 0x1)
            {
                ulDataReg &= ~BCHP_MASK       (CCE_ANCIL_0_Ancil_Data0, WORD1        );
                ulDataReg |=  BCHP_FIELD_DATA (CCE_ANCIL_0_Ancil_Data0, WORD1, usWord);
            }
            else
            {
                ulDataReg &= ~BCHP_MASK       (CCE_ANCIL_0_Ancil_Data0, WORD0        );
                ulDataReg |=  BCHP_FIELD_DATA (CCE_ANCIL_0_Ancil_Data0, WORD0, usWord);
            }

            /* Write the data register back */
            BREG_Write32 (hReg, ulDataRegAddr, ulDataReg);

            /* Advance counters */
            if (numSet & 0x1)
            {
                ++ucRegNum;
                ulDataRegAddr =
                    BCHP_CCE_ANCIL_0_Ancil_Data0 + ulCoreOffset +
                        (sizeof(uint32_t) * ucRegNum);
            }
            ++numSet;
        }

        /* Advance */
        lineMask >>= 1;
    }

    /* Adjust line mask for hardware oddities */
    lineMask = pMCCData->uhLineMask;
    /* PAL-M always seems to cause problems */
    if (eVideoFormat == BFMT_VideoFmt_ePAL_M)
        lineMask >>= 3;

    /* Finally, set active lines register. */
    switch (polarity)
    {
    case BAVC_Polarity_eTopField:
        ulLinesRegAddr = BCHP_CCE_ANCIL_0_Ancil_Active_Lines + ulCoreOffset;
        ulLinesReg =
            BREG_Read32 ( hReg,  ulLinesRegAddr );
        ulLinesReg &= ~BCHP_MASK       (
            CCE_ANCIL_0_Ancil_Active_Lines, SCTE_ON_SCTE_TOP_ACTIVE);
        ulLinesReg |=  BCHP_FIELD_DATA (
            CCE_ANCIL_0_Ancil_Active_Lines, SCTE_ON_SCTE_TOP_ACTIVE, lineMask );
            break;
    case BAVC_Polarity_eBotField:
        ulLinesRegAddr = BCHP_CCE_ANCIL_0_Ancil_Active_Lines_1 + ulCoreOffset;
        ulLinesReg =
            BREG_Read32 ( hReg,  ulLinesRegAddr );
        ulLinesReg &= ~BCHP_MASK       (
            CCE_ANCIL_0_Ancil_Active_Lines_1, SCTE_BOT_ACTIVE);
        ulLinesReg |=  BCHP_FIELD_DATA (
            CCE_ANCIL_0_Ancil_Active_Lines_1, SCTE_BOT_ACTIVE, lineMask );
            break;
        default:
            BDBG_LEAVE(BVBI_P_MCC_Encode_656_Data_isr);
            return (uint32_t)(-1);
            break;
    }
    BREG_Write32 ( hReg, ulLinesRegAddr, ulLinesReg );

    BDBG_LEAVE(BVBI_P_MCC_Encode_656_Data_isr);
    return ulErrInfo;
}
#endif

#if !B_REFSW_MINIMAL
#if BVBI_NUM_CCE >= 1
/***************************************************************************
 *
 */
BERR_Code BVBI_P_MCC_Encode_Enable_isr (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    bool bEnable)
{
    uint32_t ulCoreOffset;
    uint32_t ulControlReg;

    /* TODO: handle progressive video */
    BSTD_UNUSED (eVideoFormat);

    BDBG_ENTER(BVBI_P_MCC_Encode_Enable_isr);

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_isr (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_MCC_Encode_Enable_isr);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    ulControlReg = BREG_Read32 ( hReg, BCHP_CCE_0_Control + ulCoreOffset );
    ulControlReg &=
        ~BCHP_MASK (CCE_0_Control, ENABLE_CLOSED_CAPTION);
    if (bEnable)
    {
        ulControlReg |=
            BCHP_FIELD_ENUM (CCE_0_Control, ENABLE_CLOSED_CAPTION, ENABLED);
    }
    else
    {
        ulControlReg |=
            BCHP_FIELD_ENUM (CCE_0_Control, ENABLE_CLOSED_CAPTION, DISABLED);
    }
    BREG_Write32 ( hReg, BCHP_CCE_0_Control + ulCoreOffset, ulControlReg );

    BDBG_LEAVE(BVBI_P_MCC_Encode_Enable_isr);
    return BERR_SUCCESS;
}
#endif
#endif

#if !B_REFSW_MINIMAL
#if BVBI_NUM_CCE_656 >= 1
/***************************************************************************
 *
 */
BERR_Code BVBI_P_MCC_Encode_656_Enable_isr (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    bool bEnable)
{
    uint32_t ulCoreOffset;
    uint32_t ulControlReg;

    /* TODO: handle progressive video */
    BSTD_UNUSED (eVideoFormat);

    BDBG_ENTER(BVBI_P_MCC_Encode_656_Enable_isr);

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_656_isr (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_MCC_Encode_656_Enable_isr);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    ulControlReg = BREG_Read32 ( hReg, BCHP_CCE_ANCIL_0_Ancil_Control + ulCoreOffset );
    ulControlReg &=
        ~BCHP_MASK (CCE_ANCIL_0_Ancil_Control, ENABLE_CLOSED_CAPTION);
    if (bEnable)
    {
        ulControlReg |=
            BCHP_FIELD_ENUM (CCE_ANCIL_0_Ancil_Control, ENABLE_CLOSED_CAPTION, ENABLED);
    }
    else
    {
        ulControlReg |=
            BCHP_FIELD_ENUM (CCE_ANCIL_0_Ancil_Control, ENABLE_CLOSED_CAPTION, DISABLED);
    }
    BREG_Write32 ( hReg, BCHP_CCE_ANCIL_0_Ancil_Control + ulCoreOffset, ulControlReg );

    BDBG_LEAVE(BVBI_P_MCC_Encode_656_Enable_isr);
    return BERR_SUCCESS;
}
#endif
#endif


/***************************************************************************
* Static (private) functions
***************************************************************************/

#if BVBI_NUM_CCE >= 1
/***************************************************************************
 *
 */
static uint32_t P_GetCoreOffset_isr (uint8_t hwCoreIndex)
{
    uint32_t ulCoreOffset = 0xFFFFFFFF;

    switch (hwCoreIndex)
    {
#if (BVBI_NUM_CCE >= 1)
    case 0:
        ulCoreOffset = 0;
        break;
#endif
#if (BVBI_NUM_CCE >= 2)
    case 1:
        ulCoreOffset = (BCHP_CCE_1_REG_START - BCHP_CCE_0_REG_START);
        break;
#endif
#if (BVBI_NUM_CCE >= 3)
    case 2:
        ulCoreOffset = (BCHP_CCE_2_REG_START - BCHP_CCE_0_REG_START);
        break;
#endif
    default:
        break;
    }

    return ulCoreOffset;
}
#endif

#if BVBI_NUM_CCE_656 >= 1
/***************************************************************************
 *
 */
static uint32_t P_GetCoreOffset_656_isr (uint8_t hwCoreIndex)
{
    uint32_t ulCoreOffset = 0xFFFFFFFF;

    switch (hwCoreIndex)
    {
#if (BVBI_NUM_CCE_656 >= 1)
    case 0:
        ulCoreOffset = 0;
        break;
#endif
#if (BVBI_NUM_CCE_656 >= 2)
    case 1:
        ulCoreOffset =
            (BCHP_CCE_ANCIL_1_REG_START - BCHP_CCE_ANCIL_0_REG_START);
        break;
#endif
#if (BVBI_NUM_CCE_656 >= 3)
    case 2:
        ulCoreOffset =
            (BCHP_CCE_ANCIL_2_REG_START - BCHP_CCE_ANCIL_0_REG_START);
        break;
#endif
    default:
        break;
    }

    return ulCoreOffset;
}
#endif
