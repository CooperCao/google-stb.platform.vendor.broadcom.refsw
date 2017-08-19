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

#include "bstd.h"             /* standard types */
#include "bdbg.h"             /* Dbglib */
#include "bkni.h"             /* For critical sections */
#include "bvbi.h"             /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"        /* VBI internal data structures */
#include "bavc_hdmi.h"

#if BVBI_NUM_CGMSAE > 0 /** { **/

#if (BVBI_NUM_CGMSAE >= 1)
#include "bchp_cgmsae_0.h" /* RDB info for primary CGMSE core */
#endif

BDBG_MODULE(BVBI);

/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/

static uint32_t P_GetCoreOffset_isr (uint8_t hwCoreIndex);


/***************************************************************************
* Implementation of supporting CGMS functions that are not in API
***************************************************************************/

void BVBI_P_CGMS_Enc_Init (BREG_Handle hReg, uint8_t hwCoreIndex)
{
    BDBG_ENTER(BVBI_P_CGMS_Enc_Init);

    BVBI_P_VIE_SoftReset_isr (hReg, false, hwCoreIndex, BVBI_P_SELECT_CGMSA);

    BDBG_LEAVE(BVBI_P_CGMS_Enc_Init);
}

BERR_Code BVBI_P_CGMSA_Enc_Program (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    bool bActive,
    BFMT_VideoFmt eVideoFormat,
    bool bArib480p)
{
/*
    Programming note: the implementation here assumes that the bitfield layout
    within registers is the same for all CGMS encoder cores in the chip.

    If a chip is built that has multiple CGMS encoder cores that are not
    identical, then this routine will have to be redesigned.
*/
    uint32_t ulCoreOffset;
    uint32_t ulTop_FormatReg;
    uint32_t ulBot_FormatReg;
    uint32_t ulTop_ControlReg;
    uint32_t ulBot_ControlReg;

    uint32_t rise_time;
    uint32_t gain;
    uint32_t pulse_width;
    uint32_t init_delay;
    uint32_t top_line;
    uint32_t bot_line;
    uint32_t line_start;
    uint32_t enable_top;
    uint32_t enable_bot;

    BERR_Code eErr = BERR_SUCCESS;

    BDBG_ENTER(BVBI_P_CGMSA_Enc_Program);

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_isr (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_CGMSA_Enc_Program);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Determine some tuning parameters according to video standard */
    switch (eVideoFormat)
    {
    case BFMT_VideoFmt_eNTSC:
    case BFMT_VideoFmt_eNTSC_J:
    case BFMT_VideoFmt_e720x482_NTSC:
    case BFMT_VideoFmt_e720x482_NTSC_J:
        rise_time   = BCHP_CGMSAE_0_Top_Control_RISE_TIME_MED;
        gain        = 0x62;
        pulse_width = 0x1E3;
        init_delay  = 53;
        top_line    = 20;
        bot_line    = 283 - 256;
        if (bArib480p)
        {
            --top_line;
            --bot_line;
        }
        line_start  = BCHP_CGMSAE_0_Bot_Control_VBI_START_LINE256;
        enable_top = BCHP_CGMSAE_0_Bot_Control_ENABLE_CGMSA;
        enable_bot = BCHP_CGMSAE_0_Bot_Control_ENABLE_CGMSA;
        break;

    case BFMT_VideoFmt_ePAL_M:
        rise_time   = BCHP_CGMSAE_0_Top_Control_RISE_TIME_MED;
        gain        = 0x62;
        pulse_width = 0x1E3;
        init_delay  = 37;
        top_line    = 17;
        bot_line    = 280 - 256;
        line_start  = BCHP_CGMSAE_0_Bot_Control_VBI_START_LINE256;
        enable_top = BCHP_CGMSAE_0_Bot_Control_ENABLE_CGMSA;
        enable_bot = BCHP_CGMSAE_0_Bot_Control_ENABLE_CGMSA;
        break;

    case BFMT_VideoFmt_e1080i:
    case BFMT_VideoFmt_e1080i_50Hz:
        rise_time   = BCHP_CGMSAE_0_Top_Control_RISE_TIME_FAST;
        gain        = 0x67;
        pulse_width = 0x268;
        init_delay  = 0xAF;
        top_line    = 19;
        bot_line    = 582 - 544;
        line_start  = BCHP_CGMSAE_0_Bot_Control_VBI_START_LINE544;
        enable_top = BCHP_CGMSAE_0_Bot_Control_ENABLE_CGMSA;
        enable_bot = BCHP_CGMSAE_0_Bot_Control_ENABLE_CGMSA;
        break;

    case BFMT_VideoFmt_e720p:
    case BFMT_VideoFmt_e720p_24Hz:
    case BFMT_VideoFmt_e720p_25Hz:
    case BFMT_VideoFmt_e720p_30Hz:
    case BFMT_VideoFmt_e720p_50Hz:
        rise_time   = BCHP_CGMSAE_0_Top_Control_RISE_TIME_FAST;
        gain        = 0x67;
        pulse_width = 0x1D0;
        init_delay  = 0xB;
        top_line    = 24;
        bot_line    = 0;
        line_start  = 0;
        enable_top  = BCHP_CGMSAE_0_Bot_Control_ENABLE_CGMSA;
        enable_bot  = BCHP_CGMSAE_0_Bot_Control_ENABLE_DISABLED;
        break;

    case BFMT_VideoFmt_e576p_50Hz:
        /* TODO: tune these settings */
        rise_time   = 1;
        gain        = 0x62;
        pulse_width = 0x86;
        init_delay  = 0x26;
        top_line    = 43;
        bot_line    = 0;
        line_start  = 0;
        enable_top  = BCHP_CGMSAE_0_Bot_Control_ENABLE_RAW_DATA;
        enable_bot  = BCHP_CGMSAE_0_Bot_Control_ENABLE_DISABLED;
        break;

    case BFMT_VideoFmt_e480p:
    case BFMT_VideoFmt_e720x483p:
        /* VEC is operating at double rate (54 MHz sampling) */
        rise_time   = BCHP_CGMSAE_0_Top_Control_RISE_TIME_MED;
        gain        = 0x67;
        pulse_width = 0x1A0;
        init_delay  = 0x3E;
        top_line    = 41;
        bot_line    = 0;
        if (bArib480p)
        {
            --top_line;
        }
        line_start  = 0;
        enable_top  = BCHP_CGMSAE_0_Bot_Control_ENABLE_CGMSA;
        enable_bot  = BCHP_CGMSAE_0_Bot_Control_ENABLE_DISABLED;
        break;

    default:
        /* Shut up with the compiler warnings */
        rise_time   = 0;
        gain        = 0x0;
        pulse_width = 0;
        init_delay  = 0;
        top_line    = 0;
        bot_line    = 0;
        line_start  = 0;
        enable_top  = BCHP_CGMSAE_0_Bot_Control_ENABLE_DISABLED;
        enable_bot  = BCHP_CGMSAE_0_Bot_Control_ENABLE_DISABLED;
        if (bActive)
        {
            BDBG_ERR(("BVBI_CGMSE: video format %d not supported",
                eVideoFormat));
            return BERR_TRACE (BVBI_ERR_VFMT_CONFLICT);
        }
        break;
    }

    BKNI_EnterCriticalSection();

    /* Read control registers */
    ulTop_ControlReg =
        BREG_Read32 ( hReg, BCHP_CGMSAE_0_Top_Control + ulCoreOffset );
    ulBot_ControlReg =
        BREG_Read32 ( hReg, BCHP_CGMSAE_0_Bot_Control + ulCoreOffset );

    /* If enabling encoding */
    if (bActive)
    {
        /* Fill in the control registers */
        ulTop_ControlReg &= ~(
            BCHP_MASK       ( CGMSAE_0_Top_Control, RISE_TIME            ) |
            BCHP_MASK       ( CGMSAE_0_Top_Control, GAIN                 ) |
            BCHP_MASK       ( CGMSAE_0_Top_Control, RAW_COUNT            ) |
            BCHP_MASK       ( CGMSAE_0_Top_Control, VBI_LINE             ) |
            BCHP_MASK       ( CGMSAE_0_Top_Control, BIT_ORDER            ) |
            BCHP_MASK       ( CGMSAE_0_Top_Control, ENABLE               ) );
        ulTop_ControlReg |= (
            BCHP_FIELD_DATA ( CGMSAE_0_Top_Control, RISE_TIME, rise_time ) |
            BCHP_FIELD_DATA ( CGMSAE_0_Top_Control, GAIN,           gain ) |
            BCHP_FIELD_DATA ( CGMSAE_0_Top_Control, RAW_COUNT,        21 ) |
            BCHP_FIELD_DATA ( CGMSAE_0_Top_Control, VBI_LINE,   top_line ) |
            BCHP_FIELD_ENUM ( CGMSAE_0_Top_Control, BIT_ORDER, LSB_FIRST ) |
            BCHP_FIELD_DATA ( CGMSAE_0_Top_Control, ENABLE,   enable_top ) );
        ulBot_ControlReg &= ~(
            BCHP_MASK       ( CGMSAE_0_Bot_Control, GAIN                 ) |
            BCHP_MASK       ( CGMSAE_0_Bot_Control, RAW_COUNT            ) |
            BCHP_MASK       ( CGMSAE_0_Bot_Control, VBI_LINE             ) |
            BCHP_MASK       ( CGMSAE_0_Bot_Control, VBI_START            ) |
            BCHP_MASK       ( CGMSAE_0_Bot_Control, BIT_ORDER            ) |
            BCHP_MASK       ( CGMSAE_0_Bot_Control, ENABLE               ) );
        ulBot_ControlReg |= (
            BCHP_FIELD_DATA ( CGMSAE_0_Bot_Control, GAIN,           gain ) |
            BCHP_FIELD_DATA ( CGMSAE_0_Bot_Control, RAW_COUNT,        21 ) |
            BCHP_FIELD_DATA ( CGMSAE_0_Bot_Control, VBI_LINE,   bot_line ) |
            BCHP_FIELD_DATA ( CGMSAE_0_Bot_Control, VBI_START,line_start ) |
            BCHP_FIELD_ENUM ( CGMSAE_0_Bot_Control, BIT_ORDER, LSB_FIRST ) |
            BCHP_FIELD_DATA ( CGMSAE_0_Bot_Control, ENABLE,   enable_bot ) );

        /* Program the format registers */
        ulTop_FormatReg =
            BREG_Read32 ( hReg, BCHP_CGMSAE_0_Top_Format + ulCoreOffset );
        ulTop_FormatReg &= ~(
            BCHP_MASK       ( CGMSAE_0_Top_Format, PULSE_WIDTH       ) |
            BCHP_MASK       ( CGMSAE_0_Top_Format, INIT_DELAY        ) );
        ulTop_FormatReg |= (
            BCHP_FIELD_DATA ( CGMSAE_0_Top_Format, PULSE_WIDTH,
                                                            pulse_width ) |
            BCHP_FIELD_DATA ( CGMSAE_0_Top_Format, INIT_DELAY,
                                                             init_delay ) );
        BREG_Write32 (
            hReg, BCHP_CGMSAE_0_Top_Format + ulCoreOffset, ulTop_FormatReg );
        ulBot_FormatReg =
            BREG_Read32 ( hReg, BCHP_CGMSAE_0_Bot_Format + ulCoreOffset );
        ulBot_FormatReg &= ~(
            BCHP_MASK       ( CGMSAE_0_Bot_Format, PULSE_WIDTH       ) |
            BCHP_MASK       ( CGMSAE_0_Bot_Format, INIT_DELAY        ) );
        ulBot_FormatReg |= (
            BCHP_FIELD_DATA ( CGMSAE_0_Bot_Format, PULSE_WIDTH,
                                                            pulse_width ) |
            BCHP_FIELD_DATA ( CGMSAE_0_Bot_Format, INIT_DELAY,
                                                             init_delay ) );
        BREG_Write32 (
            hReg, BCHP_CGMSAE_0_Bot_Format + ulCoreOffset, ulBot_FormatReg );
    }
    else /* Disable encoding */
    {
        ulTop_ControlReg &=
            ~BCHP_MASK       ( CGMSAE_0_Top_Control, ENABLE           );
        ulTop_ControlReg |=
             BCHP_FIELD_ENUM ( CGMSAE_0_Top_Control, ENABLE, DISABLED );
        ulBot_ControlReg &=
            ~BCHP_MASK       ( CGMSAE_0_Bot_Control, ENABLE           );
        ulBot_ControlReg |=
             BCHP_FIELD_ENUM ( CGMSAE_0_Bot_Control, ENABLE, DISABLED );
    }

    /* Write the finished control register values */
    BREG_Write32 (
        hReg, BCHP_CGMSAE_0_Top_Control + ulCoreOffset, ulTop_ControlReg );
    BREG_Write32 (
        hReg, BCHP_CGMSAE_0_Bot_Control + ulCoreOffset, ulBot_ControlReg );

    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BVBI_P_CGMSA_Enc_Program);
    return eErr;
}

uint32_t BVBI_P_CGMSA_Encode_Data_isr (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    BAVC_Polarity polarity,
    uint32_t ulData)
{
    uint32_t ulCoreOffset;
    uint32_t ulErrInfo = 0;

    /* Debug code
    static uint32_t debugcounter = 0;
    ++debugcounter;
    */

    BDBG_ENTER(BVBI_P_CGMSA_Encode_Data_isr);

    /* Get register offset */
    ulCoreOffset = P_GetCoreOffset_isr (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* Should never happen */
        BDBG_LEAVE(BVBI_P_CGMSA_Encode_Data_isr);
        return 0xFFFFFFFF;
    }

    /* If top field */
    if ((polarity == BAVC_Polarity_eTopField) ||
        (polarity == BAVC_Polarity_eFrame   )   )
    {
        /* Write new register value */
        BREG_Write32 (
            hReg, BCHP_CGMSAE_0_Top_Data + ulCoreOffset, ulData );

        /* Debug code
        if ((debugcounter > 200) && (debugcounter <= 220))
            printf ("Wrote %08x to top field\n", ulData);
        */
    }
    else /* polarity == BAVC_Polarity_eBotField */
    {
        /* Write new register value */
        BREG_Write32 (
            hReg, BCHP_CGMSAE_0_Bot_Data + ulCoreOffset, ulData );

        /* Debug code
        if ((debugcounter > 200) && (debugcounter <= 220))
            printf ("Wrote %08x to bottom field\n", ulData);
        */
    }

    BDBG_LEAVE(BVBI_P_CGMSA_Encode_Data_isr);
    return ulErrInfo;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVBI_P_CGMSA_Encode_Enable_isr (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    bool bEnable)
{
    uint32_t ulCoreOffset;
    uint32_t ulControlReg;
    uint32_t enable_top;
    uint32_t enable_bot;

    BDBG_ENTER(BVBI_P_CGMSA_Encode_Enable_isr);

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_isr (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_CGMSA_Encode_Enable_isr);
        BDBG_ERR(("Invalid parameter"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Determine a tuning parameter according to video standard */
    switch (eVideoFormat)
    {
    case BFMT_VideoFmt_eNTSC:
    case BFMT_VideoFmt_eNTSC_J:
    case BFMT_VideoFmt_e720x482_NTSC:
    case BFMT_VideoFmt_e720x482_NTSC_J:
    case BFMT_VideoFmt_ePAL_M:
        enable_top = BCHP_CGMSAE_0_Bot_Control_ENABLE_CGMSA;
        enable_bot = BCHP_CGMSAE_0_Bot_Control_ENABLE_CGMSA;
        break;

    case BFMT_VideoFmt_e1080i:
        enable_top = BCHP_CGMSAE_0_Bot_Control_ENABLE_CGMSA;
        enable_bot = BCHP_CGMSAE_0_Bot_Control_ENABLE_CGMSA;
        break;

    case BFMT_VideoFmt_e1080i_50Hz:
        enable_top = BCHP_CGMSAE_0_Bot_Control_ENABLE_CGMSA;
        enable_bot = BCHP_CGMSAE_0_Bot_Control_ENABLE_CGMSA;
        break;

    case BFMT_VideoFmt_e720p:
        enable_top  = BCHP_CGMSAE_0_Bot_Control_ENABLE_CGMSA;
        enable_bot  = BCHP_CGMSAE_0_Bot_Control_ENABLE_DISABLED;
        break;

    case BFMT_VideoFmt_e720p_50Hz:
        enable_top  = BCHP_CGMSAE_0_Bot_Control_ENABLE_CGMSA;
        enable_bot  = BCHP_CGMSAE_0_Bot_Control_ENABLE_DISABLED;
        break;

    case BFMT_VideoFmt_e480p:
    case BFMT_VideoFmt_e720x483p:
        enable_top  = BCHP_CGMSAE_0_Bot_Control_ENABLE_CGMSA;
        enable_bot  = BCHP_CGMSAE_0_Bot_Control_ENABLE_DISABLED;
        break;

    case BFMT_VideoFmt_e576p_50Hz:
        enable_top  = BCHP_CGMSAE_0_Bot_Control_ENABLE_RAW_DATA;
        enable_bot  = BCHP_CGMSAE_0_Bot_Control_ENABLE_DISABLED;
        break;

    default:
        /* Shut up with the compiler warning */
        enable_top  = BCHP_CGMSAE_0_Bot_Control_ENABLE_DISABLED;
        enable_bot  = BCHP_CGMSAE_0_Bot_Control_ENABLE_DISABLED;
        if (bEnable)
        {
            BDBG_ERR(("BVBI_CGMSE: video format %d not supported",
                eVideoFormat));
            return BERR_TRACE (BVBI_ERR_VFMT_CONFLICT);
        }
        break;
    }

    ulControlReg =
        BREG_Read32 (hReg, BCHP_CGMSAE_0_Top_Control + ulCoreOffset);
    ulControlReg &=
        ~BCHP_MASK (CGMSAE_0_Top_Control, ENABLE);
    if (bEnable)
    {
        ulControlReg |=
            BCHP_FIELD_DATA (CGMSAE_0_Top_Control, ENABLE, enable_top);
    }
    else
    {
        ulControlReg |=
            BCHP_FIELD_ENUM (CGMSAE_0_Top_Control, ENABLE, DISABLED);
    }
    BREG_Write32 (hReg, BCHP_CGMSAE_0_Top_Control + ulCoreOffset, ulControlReg);

    ulControlReg =
        BREG_Read32 (hReg, BCHP_CGMSAE_0_Bot_Control + ulCoreOffset);
    ulControlReg &=
        ~BCHP_MASK (CGMSAE_0_Bot_Control, ENABLE);
    if (bEnable)
    {
        ulControlReg |=
            BCHP_FIELD_DATA (CGMSAE_0_Bot_Control, ENABLE, enable_bot);
    }
    else
    {
        ulControlReg |=
            BCHP_FIELD_ENUM (CGMSAE_0_Bot_Control, ENABLE, DISABLED);
    }
    BREG_Write32 (
        hReg, BCHP_CGMSAE_0_Bot_Control + ulCoreOffset, ulControlReg);

    BDBG_LEAVE(BVBI_P_CGMSA_Encode_Enable_isr);
    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 *
 */
uint32_t BVBI_P_CGMSB_Encode_Data_isr (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    BAVC_Polarity polarity,
    BVBI_CGMSB_Datum cgmsbDatum )
{
    uint32_t ulCoreOffset;
    uint32_t ulUpdate;
    uint32_t ulErrInfo = 0;

    BDBG_ENTER(BVBI_P_CGMSB_Encode_Data_isr);

    /* Get register offset */
    ulCoreOffset = P_GetCoreOffset_isr (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* Should never happen */
        BDBG_LEAVE(BVBI_P_CGMSB_Encode_Data_isr);
        return 0xFFFFFFFF;
    }

    /* If top field */
    if ((polarity == BAVC_Polarity_eTopField) ||
        (polarity == BAVC_Polarity_eFrame   )   )
    {
        /* Write new register values */
        BREG_Write32 (
            hReg, BCHP_CGMSAE_0_Top_Data_B0 + ulCoreOffset, cgmsbDatum[0] );
        BREG_Write32 (
            hReg, BCHP_CGMSAE_0_Top_Data_B1 + ulCoreOffset, cgmsbDatum[1] );
        BREG_Write32 (
            hReg, BCHP_CGMSAE_0_Top_Data_B2 + ulCoreOffset, cgmsbDatum[2] );
        BREG_Write32 (
            hReg, BCHP_CGMSAE_0_Top_Data_B3 + ulCoreOffset, cgmsbDatum[3] );
        BREG_Write32 (
            hReg, BCHP_CGMSAE_0_Top_Data_B4 + ulCoreOffset, cgmsbDatum[4] );
        ulUpdate  = BCHP_FIELD_DATA (CGMSAE_0_Reg_updt, TOP, 1);
    }
    else /* polarity == BAVC_Polarity_eBotField */
    {
        /* Write new register values */
        BREG_Write32 (
            hReg, BCHP_CGMSAE_0_Bot_Data_B0 + ulCoreOffset, cgmsbDatum[0] );
        BREG_Write32 (
            hReg, BCHP_CGMSAE_0_Bot_Data_B1 + ulCoreOffset, cgmsbDatum[1] );
        BREG_Write32 (
            hReg, BCHP_CGMSAE_0_Bot_Data_B2 + ulCoreOffset, cgmsbDatum[2] );
        BREG_Write32 (
            hReg, BCHP_CGMSAE_0_Bot_Data_B3 + ulCoreOffset, cgmsbDatum[3] );
        BREG_Write32 (
            hReg, BCHP_CGMSAE_0_Bot_Data_B4 + ulCoreOffset, cgmsbDatum[4] );
        ulUpdate  = BCHP_FIELD_DATA (CGMSAE_0_Reg_updt, BOT, 1);
    }

    /* Tell hardware we are done */
    BREG_Write32 (hReg, BCHP_CGMSAE_0_Reg_updt + ulCoreOffset, ulUpdate);

    BDBG_LEAVE(BVBI_P_CGMSB_Encode_Data_isr);
    return ulErrInfo;
}


/***************************************************************************
 *
 */
BERR_Code BVBI_P_CGMSB_Enc_Program (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    bool bActive,
    BFMT_VideoFmt eVideoFormat,
    bool bArib480p,
    bool bCea805dStyle)
{
/*
    Programming note: the implementation here assumes that the bitfield layout
    within registers is the same for all CGMS encoder cores in the chip.

    If a chip is built that has multiple CGMS encoder cores that are not
    identical, then this routine will have to be redesigned.
*/
    uint32_t ulCoreOffset;
    uint32_t ulTop_FormatReg;
    uint32_t ulBot_FormatReg;
    uint32_t ulTop_ControlReg;
    uint32_t ulBot_ControlReg;

    uint32_t rise_time;
    uint32_t gain;
    uint32_t pulse_width;
    uint32_t init_delay;
    uint32_t top_line;
    uint32_t bot_line;
    uint32_t line_start;
    uint32_t enable_top;
    uint32_t enable_bot;
    uint32_t bit_order;
    uint32_t crc_meth;

    BERR_Code eErr = BERR_SUCCESS;

    BDBG_ENTER(BVBI_P_CGMSB_Enc_Program);

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_isr (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_CGMSB_Enc_Program);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Determine some tuning parameters according to video standard */
    switch (eVideoFormat)
    {
    case BFMT_VideoFmt_eNTSC:
    case BFMT_VideoFmt_eNTSC_J:
    case BFMT_VideoFmt_e720x482_NTSC:
    case BFMT_VideoFmt_e720x482_NTSC_J:
    case BFMT_VideoFmt_ePAL_M:
        rise_time   = BCHP_CGMSAE_0_Top_Control_B_RISE_TIME_MED;
        gain        = 0x62;
        pulse_width = 0x1E3;
        init_delay  = 0x55;
        top_line    = 20;
        bot_line    = 283 - 256;
        if (bArib480p)
        {
            --top_line;
            --bot_line;
        }
        line_start  = BCHP_CGMSAE_0_Bot_Control_B_VBI_START_LINE256;
        enable_top = BCHP_CGMSAE_0_Top_Control_B_ENABLE_CGMSA;
        enable_bot = BCHP_CGMSAE_0_Bot_Control_B_ENABLE_CGMSA;
        break;

    case BFMT_VideoFmt_e1080i:
    case BFMT_VideoFmt_e1080i_50Hz:
        rise_time   = 2;
        gain        = 0x5F;
        pulse_width = 0x50;
        init_delay  = 0xB5;
        top_line    = 18;
        bot_line    = 581 - 544;
        line_start  = BCHP_CGMSAE_0_Bot_Control_B_VBI_START_LINE544;
        enable_top = BCHP_CGMSAE_0_Top_Control_B_ENABLE_CGMSA;
        enable_bot = BCHP_CGMSAE_0_Bot_Control_B_ENABLE_CGMSA;
        break;

    case BFMT_VideoFmt_e720p:
    case BFMT_VideoFmt_e720p_50Hz:
        rise_time   = BCHP_CGMSAE_0_Top_Control_B_RISE_TIME_FAST_37;
        gain        = 0x68;
        pulse_width = 0x40;
        init_delay  = 0x10;
        top_line    = 23;
        bot_line    = 0;
        line_start  = 0;
        enable_top  = BCHP_CGMSAE_0_Top_Control_B_ENABLE_CGMSA;
        enable_bot  = BCHP_CGMSAE_0_Bot_Control_B_ENABLE_DISABLED;
        break;

    case BFMT_VideoFmt_e576p_50Hz:
        /* TODO: tune these settings */
        rise_time   = 1;
        gain        = 0x62;
        pulse_width = 0x86;
        init_delay  = 0x26;
        top_line    = 43;
        bot_line    = 0;
        line_start  = 0;
        enable_top  = BCHP_CGMSAE_0_Top_Control_B_ENABLE_RAW_DATA;
        enable_bot  = BCHP_CGMSAE_0_Bot_Control_B_ENABLE_DISABLED;
        break;

    case BFMT_VideoFmt_e480p:
    case BFMT_VideoFmt_e720x483p:
        /* VEC is operating at double rate (54 MHz) */
        rise_time   = BCHP_CGMSAE_0_Top_Control_B_RISE_TIME_MED_37;
        gain        = 0x62;
        pulse_width = 0x41;
        init_delay  = 0x49;
        top_line    = 40;
        bot_line    = 0;
        if (bArib480p)
        {
            --top_line;
        }
        line_start  = 0;
        enable_top  = BCHP_CGMSAE_0_Top_Control_B_ENABLE_CGMSA;
        enable_bot  = BCHP_CGMSAE_0_Bot_Control_B_ENABLE_DISABLED;
        break;

    default:
        /* Shut up with the compiler warnings */
        rise_time   = 0;
        gain        = 0x0;
        pulse_width = 0;
        init_delay  = 0;
        top_line    = 0;
        bot_line    = 0;
        line_start  = 0;
        enable_top  = BCHP_CGMSAE_0_Top_Control_B_ENABLE_DISABLED;
        enable_bot  = BCHP_CGMSAE_0_Bot_Control_B_ENABLE_DISABLED;
        if (bActive)
        {
            BDBG_ERR(("BVBI_CGMSE: video format %d not supported",
                eVideoFormat));
            return BERR_TRACE (BVBI_ERR_VFMT_CONFLICT);
        }
        break;
    }

    /* Make the choice defined in CEA-805-D */
/* Being careful */
#if BCHP_CGMSAE_0_Top_Control_BIT_ORDER_MSB_FIRST != \
    BCHP_CGMSAE_0_Bot_Control_BIT_ORDER_MSB_FIRST
    #error Programming error
#endif
#if BCHP_CGMSAE_0_Top_Control_BIT_ORDER_LSB_FIRST != \
    BCHP_CGMSAE_0_Bot_Control_BIT_ORDER_LSB_FIRST
    #error Programming error
#endif
#if BCHP_CGMSAE_0_Top_Control_B_CRC_METHOD_METHOD1 != \
    BCHP_CGMSAE_0_Bot_Control_B_CRC_METHOD_METHOD1
    #error Programming error
#endif
#if BCHP_CGMSAE_0_Top_Control_B_CRC_METHOD_METHOD2 != \
    BCHP_CGMSAE_0_Bot_Control_B_CRC_METHOD_METHOD2
    #error Programming error
#endif
    if (bCea805dStyle)
    {
        crc_meth = BCHP_CGMSAE_0_Top_Control_B_CRC_METHOD_METHOD2;
    }
    else
    {
        crc_meth = BCHP_CGMSAE_0_Top_Control_B_CRC_METHOD_METHOD1;
    }
        bit_order = BCHP_CGMSAE_0_Top_Control_BIT_ORDER_MSB_FIRST;

    BKNI_EnterCriticalSection();

    /* Read control registers */
    ulTop_ControlReg =
        BREG_Read32 ( hReg, BCHP_CGMSAE_0_Top_Control_B + ulCoreOffset );
    ulBot_ControlReg =
        BREG_Read32 ( hReg, BCHP_CGMSAE_0_Bot_Control_B + ulCoreOffset );

    /* If enabling encoding */
    if (bActive)
    {
        /* Fill in the control registers */
        ulTop_ControlReg &= ~(
            BCHP_MASK       (CGMSAE_0_Top_Control_B, RISE_TIME           ) |
            BCHP_MASK       (CGMSAE_0_Top_Control_B, GAIN                ) |
            BCHP_MASK       (CGMSAE_0_Top_Control_B, RAW_COUNT           ) |
            BCHP_MASK       (CGMSAE_0_Top_Control_B, VBI_LINE            ) |
            BCHP_MASK       (CGMSAE_0_Top_Control_B, BIT_ORDER           ) |
            BCHP_MASK       (CGMSAE_0_Top_Control_B, CRC_METHOD          ) |
            BCHP_MASK       (CGMSAE_0_Top_Control_B, ENABLE              ) );
        ulTop_ControlReg |= (
            BCHP_FIELD_DATA (CGMSAE_0_Top_Control_B, RISE_TIME, rise_time) |
            BCHP_FIELD_DATA (CGMSAE_0_Top_Control_B, GAIN,           gain) |
            BCHP_FIELD_DATA (CGMSAE_0_Top_Control_B, RAW_COUNT,        21) |
            BCHP_FIELD_DATA (CGMSAE_0_Top_Control_B, VBI_LINE,   top_line) |
            BCHP_FIELD_DATA (CGMSAE_0_Top_Control_B, BIT_ORDER, bit_order) |
            BCHP_FIELD_DATA (CGMSAE_0_Top_Control_B, CRC_METHOD, crc_meth) |
            BCHP_FIELD_DATA (CGMSAE_0_Top_Control_B, ENABLE,   enable_top) );
        ulBot_ControlReg &= ~(
            BCHP_MASK       (CGMSAE_0_Bot_Control_B, GAIN                ) |
            BCHP_MASK       (CGMSAE_0_Bot_Control_B, RAW_COUNT           ) |
            BCHP_MASK       (CGMSAE_0_Bot_Control_B, VBI_LINE            ) |
            BCHP_MASK       (CGMSAE_0_Bot_Control_B, VBI_START           ) |
            BCHP_MASK       (CGMSAE_0_Bot_Control_B, BIT_ORDER           ) |
            BCHP_MASK       (CGMSAE_0_Bot_Control_B, CRC_METHOD          ) |
            BCHP_MASK       (CGMSAE_0_Bot_Control_B, ENABLE              ) );
        ulBot_ControlReg |= (
            BCHP_FIELD_DATA (CGMSAE_0_Bot_Control_B, GAIN,           gain) |
            BCHP_FIELD_DATA (CGMSAE_0_Bot_Control_B, RAW_COUNT,        21) |
            BCHP_FIELD_DATA (CGMSAE_0_Bot_Control_B, VBI_LINE,   bot_line) |
            BCHP_FIELD_DATA (CGMSAE_0_Bot_Control_B, VBI_START,line_start) |
            BCHP_FIELD_DATA (CGMSAE_0_Bot_Control_B, BIT_ORDER, bit_order) |
            BCHP_FIELD_DATA (CGMSAE_0_Bot_Control_B, CRC_METHOD, crc_meth) |
            BCHP_FIELD_DATA (CGMSAE_0_Bot_Control_B, ENABLE,   enable_bot) );

        /* Program the format registers */
        ulTop_FormatReg =
            BREG_Read32 ( hReg, BCHP_CGMSAE_0_Top_Format_B + ulCoreOffset);
        ulTop_FormatReg &= ~(
            BCHP_MASK       (CGMSAE_0_Top_Format_B,      HEADER      ) |
            BCHP_MASK       (CGMSAE_0_Top_Format_B, PULSE_WIDTH      ) |
            BCHP_MASK       (CGMSAE_0_Top_Format_B,  INIT_DELAY      ) );
        ulTop_FormatReg |= (
            BCHP_FIELD_DATA (CGMSAE_0_Top_Format_B,      HEADER, 0x32) |
            BCHP_FIELD_DATA (CGMSAE_0_Top_Format_B, PULSE_WIDTH,
                                                            pulse_width ) |
            BCHP_FIELD_DATA (CGMSAE_0_Top_Format_B, INIT_DELAY,
                                                             init_delay ) );
        BREG_Write32 (
            hReg, BCHP_CGMSAE_0_Top_Format_B + ulCoreOffset,
            ulTop_FormatReg);
        ulBot_FormatReg =
            BREG_Read32 (hReg, BCHP_CGMSAE_0_Bot_Format_B + ulCoreOffset);
        ulBot_FormatReg &= ~(
            BCHP_MASK       (CGMSAE_0_Bot_Format_B,      HEADER     ) |
            BCHP_MASK       (CGMSAE_0_Bot_Format_B, PULSE_WIDTH     ) |
            BCHP_MASK       (CGMSAE_0_Bot_Format_B, INIT_DELAY      ) );
        ulBot_FormatReg |= (
            BCHP_FIELD_DATA (CGMSAE_0_Bot_Format_B,      HEADER, 0x32) |
            BCHP_FIELD_DATA (CGMSAE_0_Bot_Format_B, PULSE_WIDTH,
                                                            pulse_width) |
            BCHP_FIELD_DATA (CGMSAE_0_Bot_Format_B, INIT_DELAY,
                                                             init_delay) );
        BREG_Write32 (
            hReg, BCHP_CGMSAE_0_Bot_Format_B + ulCoreOffset,
            ulBot_FormatReg );
    }
    else /* Disable encoding */
    {
        ulTop_ControlReg &=
            ~BCHP_MASK       (CGMSAE_0_Top_Control_B, ENABLE          );
        ulTop_ControlReg |=
             BCHP_FIELD_ENUM (CGMSAE_0_Top_Control_B, ENABLE, DISABLED);
        ulBot_ControlReg &=
            ~BCHP_MASK       (CGMSAE_0_Bot_Control_B, ENABLE          );
        ulBot_ControlReg |=
             BCHP_FIELD_ENUM (CGMSAE_0_Bot_Control_B, ENABLE, DISABLED);
    }

    /* Write the finished control register values */
    BREG_Write32 (
        hReg, BCHP_CGMSAE_0_Top_Control_B + ulCoreOffset, ulTop_ControlReg);
    BREG_Write32 (
        hReg, BCHP_CGMSAE_0_Bot_Control_B + ulCoreOffset, ulBot_ControlReg);

    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BVBI_P_CGMSB_Enc_Program);
    return eErr;
}

/***************************************************************************
* Static (private) functions
***************************************************************************/

/***************************************************************************
 *
 */
static uint32_t P_GetCoreOffset_isr (uint8_t hwCoreIndex)
{
    uint32_t ulCoreOffset = 0xFFFFFFFF;

    switch (hwCoreIndex)
    {
#if (BVBI_NUM_CGMSAE >= 1)
    case 0:
        ulCoreOffset = 0;
        break;
#endif
#if (BVBI_NUM_CGMSAE >= 2)
    case 1:
        ulCoreOffset = (BCHP_CGMSAE_1_REG_START - BCHP_CGMSAE_0_REG_START);
        break;
#endif
#if (BVBI_NUM_CGMSAE >= 3)
    case 2:
        ulCoreOffset = (BCHP_CGMSAE_2_REG_START - BCHP_CGMSAE_0_REG_START);
        break;
#endif
    default:
        break;
    }

    return ulCoreOffset;
}

#endif /** } BVBI_NUM_CGMSAE > 0 **/
