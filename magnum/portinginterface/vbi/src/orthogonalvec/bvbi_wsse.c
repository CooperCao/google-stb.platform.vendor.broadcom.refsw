/***************************************************************************
 * Copyright (C) 2016-2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/

#include "bstd.h"           /* standard types */
#include "bkni.h"           /* For critical sections */
#include "bvbi.h"           /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"      /* VBI internal data structures */
#if (BVBI_NUM_WSE >= 1)
#include "bchp_wse_0.h"  /* RDB info for primary WSSE core */
#endif
#if (BVBI_NUM_WSE_656 >= 1)
#include "bchp_wse_ancil_0.h"   /* RDB info for ITU-R 656 "bypass" WSSE core */
#endif

BDBG_MODULE(BVBI);


/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/
#if (BVBI_NUM_WSE >= 1)
static uint32_t P_GetCoreOffset_isrsafe (uint8_t hwCoreIndex);
#endif
#if (BVBI_NUM_WSE_656 >= 1)
static uint32_t P_GetCoreOffset_656_isrsafe (uint8_t hwCoreIndex);
#endif


/***************************************************************************
* Implementation of supporting WSS functions that are not in API
***************************************************************************/

#if (BVBI_NUM_WSE >= 1)
/***************************************************************************
 *
 */
void BVBI_P_WSS_Enc_Init (BREG_Handle hReg, uint8_t hwCoreIndex)
{
    uint32_t ulCoreOffset;
    uint32_t ulDataReg;
    uint32_t ulControlReg;

    BDBG_ENTER(BVBI_P_WSS_Enc_Init);

    BVBI_P_VIE_SoftReset_isrsafe (hReg, false, hwCoreIndex, BVBI_P_SELECT_WSS);

    /* Determine which core to access */
    ulCoreOffset = P_GetCoreOffset_isrsafe (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_ASSERT (0);
    }

    BKNI_EnterCriticalSection();

    /* Program the control register */
    ulControlReg = BREG_Read32 (hReg, BCHP_WSE_0_control + ulCoreOffset);
    ulControlReg &= ~(
        BCHP_MASK       (WSE_0_control, output_attenuation             ) |
        BCHP_MASK       (WSE_0_control, OUTPUT_ORDER                   ) |
        BCHP_MASK       (WSE_0_control, anci656_enable                 ) |
        BCHP_MASK       (WSE_0_control, invert_data                    ) |
        BCHP_MASK       (WSE_0_control, active_line                    ) |
        BCHP_MASK       (WSE_0_control, enable                         ) );
    ulControlReg |= (
        BCHP_FIELD_DATA (WSE_0_control, output_attenuation,        0x64) |
        BCHP_FIELD_ENUM (WSE_0_control, OUTPUT_ORDER,       Low_Pad_2nd) |
        BCHP_FIELD_DATA (WSE_0_control, anci656_enable,               1) |
        BCHP_FIELD_ENUM (WSE_0_control, invert_data,                Off) |
        BCHP_FIELD_DATA (WSE_0_control, active_line,                 22) |
        BCHP_FIELD_DATA (WSE_0_control, enable,                       0) );
    BREG_Write32 (hReg, BCHP_WSE_0_control + ulCoreOffset, ulControlReg);

    /* Clear burst lock status */
    ulDataReg = BREG_Read32 (hReg, BCHP_WSE_0_wss_data + ulCoreOffset);
    ulDataReg &= ~BCHP_MASK       (WSE_0_wss_data, wss_data     );
    ulDataReg |=  BCHP_FIELD_DATA (WSE_0_wss_data, wss_data, 0x0);
    BREG_Write32 (hReg, BCHP_WSE_0_wss_data + ulCoreOffset, ulDataReg);

    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BVBI_P_WSS_Enc_Init);
}
#endif

#if (BVBI_NUM_WSE_656 >= 1)
/***************************************************************************
 *
 */
void BVBI_P_WSS_Enc_656_Init (BREG_Handle hReg, uint8_t hwCoreIndex)
{
    uint32_t ulCoreOffset;
    uint32_t ulDataReg;
    uint32_t ulControlReg;

    BDBG_ENTER(BVBI_P_WSS_Enc_656_Init);

    /* Determine which core to access */
    ulCoreOffset = P_GetCoreOffset_656_isrsafe (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_ASSERT (0);
    }

    BKNI_EnterCriticalSection();

    /* Start by doing a reset */

    BVBI_P_VIE_SoftReset_isrsafe (hReg, true, hwCoreIndex, BVBI_P_SELECT_WSS);

    /* Program the control register */
    ulControlReg = BREG_Read32 (hReg, BCHP_WSE_ANCIL_0_control);
    ulControlReg &= ~(
        BCHP_MASK       (WSE_ANCIL_0_control, OUTPUT_ORDER                   ) |
        BCHP_MASK       (WSE_ANCIL_0_control, anci656_enable                 ) |
        BCHP_MASK       (WSE_ANCIL_0_control, active_line                    ) );
    ulControlReg |= (
        BCHP_FIELD_ENUM (WSE_ANCIL_0_control, OUTPUT_ORDER,       Low_Pad_2nd) |
        BCHP_FIELD_DATA (WSE_ANCIL_0_control, anci656_enable,               1) |
        BCHP_FIELD_DATA (WSE_ANCIL_0_control, active_line,                 22) );
    BREG_Write32 (hReg, BCHP_WSE_ANCIL_0_control, ulControlReg);

    /* Clear burst lock status */
    ulDataReg = BREG_Read32 (hReg, BCHP_WSE_ANCIL_0_wss_data);
    ulDataReg &= ~BCHP_MASK       (WSE_ANCIL_0_wss_data, wss_data     );
    ulDataReg |=  BCHP_FIELD_DATA (WSE_ANCIL_0_wss_data, wss_data, 0x0);
    BREG_Write32 (hReg, BCHP_WSE_ANCIL_0_wss_data, ulDataReg);

    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BVBI_P_WSS_Enc_656_Init);
}
#endif

#if (BVBI_NUM_WSE >= 1)
BERR_Code BVBI_P_WSS_Enc_Program (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    bool bActive,
    BFMT_VideoFmt eVideoFormat)
{
/*
    Programming note: the implementation here assumes that the bitfield layout
    within registers is the same for all WSS encoder cores in the chip.

    If a chip is built that has multiple WSS encoder cores that are not
    identical, then this routine will have to be redesigned.
*/
    BERR_Code eErr;
    uint32_t ulCoreOffset;
    uint32_t ulControlReg;
    uint32_t start_delay;
    uint32_t ulActiveLine;

    BDBG_ENTER(BVBI_P_WSS_Enc_Program);

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_isrsafe (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_WSE_Enc_Program);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Complain if video format is not supported */
    switch (eVideoFormat)
    {
    /* NTSC case is just for testing */
    case BFMT_VideoFmt_eNTSC:
    case BFMT_VideoFmt_eNTSC_J:
    case BFMT_VideoFmt_e720x482_NTSC:
    case BFMT_VideoFmt_e720x482_NTSC_J:
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
        start_delay = 0xE;
        ulActiveLine = 22;
        break;
    case BFMT_VideoFmt_e576p_50Hz:
        /* Assumes use of special 54 MHz VEC IT microcode */
        start_delay = 0x7;
        ulActiveLine = 42;
        break;
    case BFMT_VideoFmt_eSECAM_L:
    case BFMT_VideoFmt_eSECAM_B:
    case BFMT_VideoFmt_eSECAM_G:
    case BFMT_VideoFmt_eSECAM_D:
    case BFMT_VideoFmt_eSECAM_K:
    case BFMT_VideoFmt_eSECAM_H:
        start_delay = 0x18;
        ulActiveLine = 22;
        break;

    default:
        start_delay = 0x0;
        ulActiveLine = 0;
        if (bActive)
        {
            BDBG_ERR(("BVBI_WSSE: video format %d not supported",
                eVideoFormat));
            return BERR_TRACE (BVBI_ERR_VFMT_CONFLICT);
        }
    }
    BSTD_UNUSED(start_delay);

    /* Program the active line in the control register */
    ulControlReg = BREG_Read32 (hReg, BCHP_WSE_0_control + ulCoreOffset);
    ulControlReg &=
        ~BCHP_MASK       (WSE_0_control, active_line              );
    ulControlReg |=
         BCHP_FIELD_DATA (WSE_0_control, active_line, ulActiveLine);
    ulControlReg &=
        ~BCHP_MASK       (WSE_0_control, start_delay              );
    ulControlReg |=
         BCHP_FIELD_DATA (WSE_0_control, start_delay,  start_delay);
    ulControlReg &=
        ~BCHP_MASK       (WSE_0_control, AUTO_PARITY_TYP          );
    ulControlReg |=
         BCHP_FIELD_ENUM (WSE_0_control, AUTO_PARITY_TYP,      ODD);
    ulControlReg &=
        ~BCHP_MASK       (WSE_0_control, AUTO_PARITY_EN           );
    ulControlReg |=
         BCHP_FIELD_DATA (WSE_0_control, AUTO_PARITY_EN ,        1);
    ulControlReg &=
        ~BCHP_MASK       (WSE_0_control, AUTO_PARITY_EN_656       );
    ulControlReg |=
         BCHP_FIELD_DATA (WSE_0_control, AUTO_PARITY_EN_656,     1);
    ulControlReg &=
        ~BCHP_MASK       (WSE_0_control, DECIMATOR_EN             );
    ulControlReg |=
         BCHP_FIELD_ENUM (WSE_0_control, DECIMATOR_EN,     DISABLE);
    BREG_Write32 (hReg, BCHP_WSE_0_control + ulCoreOffset, ulControlReg);

    eErr = BERR_TRACE (BVBI_P_WSS_Encode_Enable_isrsafe (
        hReg, hwCoreIndex, BFMT_VideoFmt_ePAL_G, bActive));
    if (eErr != BERR_SUCCESS)
        goto done;

done:
    BDBG_LEAVE(BVBI_P_WSS_Enc_Program);
    return eErr;
}
#endif

#if (BVBI_NUM_WSE_656 >= 1)
BERR_Code BVBI_P_WSS_Enc_656_Program (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    bool bActive,
    bool bPR18010_bad_line_number,
    BFMT_VideoFmt eVideoFormat)
{
/*
    Programming note: the implementation here assumes that the bitfield layout
    within registers is the same for all WSS encoder cores in the chip.

    If a chip is built that has multiple WSS encoder cores that are not
    identical, then this routine will have to be redesigned.
*/
    BERR_Code eErr;
    uint32_t ulCoreOffset;
    uint32_t ulControlReg;
    uint32_t start_delay;
    uint32_t ulActiveLine;

    BDBG_ENTER(BVBI_P_WSS_Enc_656_Program);

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_656_isrsafe (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_WSE_Enc_656_Program);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Complain if video format is not supported */
    switch (eVideoFormat)
    {
    /* NTSC case is just for testing */
    case BFMT_VideoFmt_eNTSC:
    case BFMT_VideoFmt_eNTSC_J:
    case BFMT_VideoFmt_e720x482_NTSC:
    case BFMT_VideoFmt_e720x482_NTSC_J:
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
        start_delay = 0xE;
        ulActiveLine = 22;
        break;
    case BFMT_VideoFmt_e576p_50Hz:
        /* Assumes use of special 54 MHz VEC IT microcode */
        start_delay = 0x7;
        ulActiveLine = 42;
        break;
    case BFMT_VideoFmt_eSECAM_L:
    case BFMT_VideoFmt_eSECAM_B:
    case BFMT_VideoFmt_eSECAM_G:
    case BFMT_VideoFmt_eSECAM_D:
    case BFMT_VideoFmt_eSECAM_K:
    case BFMT_VideoFmt_eSECAM_H:
        start_delay = 0x18;
        ulActiveLine = 22;
        break;

    default:
        start_delay = 0x0;
        ulActiveLine = 0;
        if (bActive)
        {
            BDBG_ERR(("BVBI_WSSE: video format %d not supported",
                eVideoFormat));
            return BERR_TRACE (BVBI_ERR_VFMT_CONFLICT);
        }
    }
    BSTD_UNUSED(start_delay);

    /* PR18010 workaround, with exception. */
    if (!bPR18010_bad_line_number)
    {
        ulActiveLine -= 1;
    }

    /* Program the active line in the control register */
    ulControlReg = BREG_Read32 (hReg, BCHP_WSE_ANCIL_0_control + ulCoreOffset);
    {
        ulControlReg &=
            ~BCHP_MASK       (WSE_ANCIL_0_control, active_line              );
        ulControlReg |=
             BCHP_FIELD_DATA (WSE_ANCIL_0_control, active_line, ulActiveLine);
        ulControlReg &=
            ~BCHP_MASK       (WSE_ANCIL_0_control, AUTO_PARITY_EN_656       );
        ulControlReg |=
             BCHP_FIELD_DATA (WSE_ANCIL_0_control, AUTO_PARITY_EN_656,     1);
    }

    BREG_Write32 (hReg, BCHP_WSE_ANCIL_0_control + ulCoreOffset, ulControlReg);

    eErr = BERR_TRACE (BVBI_P_WSS_Encode_656_Enable_isrsafe (
        hReg, hwCoreIndex, BFMT_VideoFmt_ePAL_G, bActive));
    if (eErr != BERR_SUCCESS)
        goto done;

done:
    BDBG_LEAVE(BVBI_P_WSS_Enc_656_Program);
    return eErr;
}
#endif

#if (BVBI_NUM_WSE >= 1)
uint32_t BVBI_P_WSS_Encode_Data_isr (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    BAVC_Polarity polarity,
    uint16_t usData)
{
    uint32_t ulCoreOffset;
    uint32_t ulDataReg;
    uint32_t ulErrInfo = 0;

    BDBG_ENTER(BVBI_P_WSS_Encode_Data_isr);

    /* WSS is for top field only */
    if ((polarity != BAVC_Polarity_eTopField) &&
        (polarity != BAVC_Polarity_eFrame))
    {
        return BVBI_LINE_ERROR_PARITY_CONFLICT;
    }

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_isrsafe (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_WSS_Encode_Data_isr);
        return (uint32_t)(-1);
    }

    /* Write new register value */
    ulDataReg = BREG_Read32 (hReg, BCHP_WSE_0_wss_data + ulCoreOffset);
    ulDataReg &=
        ~BCHP_MASK       (WSE_0_wss_data, wss_data                  );
    ulDataReg |=
         BCHP_FIELD_DATA (WSE_0_wss_data, wss_data, (uint32_t)usData);
    BREG_Write32 (hReg, BCHP_WSE_0_wss_data + ulCoreOffset, ulDataReg);

    BDBG_LEAVE(BVBI_P_WSS_Encode_Data_isr);
    return ulErrInfo;
}
#endif

#if (BVBI_NUM_WSE_656 >= 1)
uint32_t BVBI_P_WSS_Encode_656_Data_isr (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    BAVC_Polarity polarity,
    uint16_t usData)
{
    uint32_t ulCoreOffset;
    uint32_t ulDataReg;
    uint32_t ulErrInfo = 0;

    BDBG_ENTER(BVBI_P_WSS_Encode_656_Data_isr);

    /* WSS is for top field only */
    if ((polarity != BAVC_Polarity_eTopField) &&
        (polarity != BAVC_Polarity_eFrame))
    {
        return BVBI_LINE_ERROR_PARITY_CONFLICT;
    }

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_656_isrsafe (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_WSS_Encode_656_Data_isr);
        return (uint32_t)(-1);
    }

    /* Write new register value */
    ulDataReg = BREG_Read32 (hReg, BCHP_WSE_ANCIL_0_wss_data + ulCoreOffset);
    ulDataReg &=
        ~BCHP_MASK       (WSE_ANCIL_0_wss_data, wss_data                  );
    ulDataReg |=
         BCHP_FIELD_DATA (WSE_ANCIL_0_wss_data, wss_data, (uint32_t)usData);
    BREG_Write32 (hReg, BCHP_WSE_ANCIL_0_wss_data + ulCoreOffset, ulDataReg);

    BDBG_LEAVE(BVBI_P_WSS_Encode_656_Data_isr);
    return ulErrInfo;
}
#endif

#if (BVBI_NUM_WSE >= 1)
BERR_Code BVBI_P_WSS_Encode_Enable_isrsafe (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    bool bActive)
{
/*
    Programming note: the implementation here assumes that the bitfield layout
    within registers is the same for all WSS encoder cores in the chip.

    If a chip is built that has multiple WSS encoder cores that are not
    identical, then this routine will have to be redesigned.
*/
    uint32_t ulCoreOffset;
    uint32_t ulControlReg;

    /* TODO: handle progressive video */
    BSTD_UNUSED (eVideoFormat);

    BDBG_ENTER(BVBI_P_WSS_Encode_Enable_isrsafe);

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_isrsafe (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_WSS_Encode_Enable_isrsafe);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    ulControlReg = BREG_Read32 (hReg, BCHP_WSE_0_control + ulCoreOffset);
    ulControlReg &= ~BCHP_MASK       (WSE_0_control, enable   );
    if (bActive)
    {
        ulControlReg |=  BCHP_FIELD_DATA (WSE_0_control, enable, 1);
    }
    else
    {
        ulControlReg |=  BCHP_FIELD_DATA (WSE_0_control, enable, 0);
    }
    BREG_Write32 (hReg, BCHP_WSE_0_control + ulCoreOffset, ulControlReg);

    BDBG_LEAVE(BVBI_P_WSS_Encode_Enable_isrsafe);
    return BERR_SUCCESS;
}
#endif

#if (BVBI_NUM_WSE_656 >= 1)
BERR_Code BVBI_P_WSS_Encode_656_Enable_isrsafe (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    bool bActive)
{
/*
    Programming note: the implementation here assumes that the bitfield layout
    within registers is the same for all WSS encoder cores in the chip.

    If a chip is built that has multiple WSS encoder cores that are not
    identical, then this routine will have to be redesigned.
*/
    uint32_t ulCoreOffset;
    uint32_t ulControlReg;

    /* TODO: handle progressive video */
    BSTD_UNUSED (eVideoFormat);

    BDBG_ENTER(BVBI_P_WSS_Encode_656_Enable_isrsafe);

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_656_isrsafe (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_WSS_Encode_656_Enable_isrsafe);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    ulControlReg = BREG_Read32 (hReg, BCHP_WSE_ANCIL_0_control + ulCoreOffset);
    ulControlReg &= ~BCHP_MASK       (WSE_ANCIL_0_control, anci656_enable   );
    if (bActive)
    {
        ulControlReg |=  BCHP_FIELD_DATA (WSE_ANCIL_0_control, anci656_enable, 1);
    }
    else
    {
        ulControlReg |=  BCHP_FIELD_DATA (WSE_ANCIL_0_control, anci656_enable, 0);
    }
    BREG_Write32 (hReg, BCHP_WSE_ANCIL_0_control + ulCoreOffset, ulControlReg);

    BDBG_LEAVE(BVBI_P_WSS_Encode_656_Enable_isrsafe);
    return BERR_SUCCESS;
}
#endif


/***************************************************************************
* Static (private) functions
***************************************************************************/

#if (BVBI_NUM_WSE >= 1)
/***************************************************************************
 *
 */
static uint32_t P_GetCoreOffset_isrsafe (uint8_t hwCoreIndex)
{
    uint32_t ulCoreOffset = 0xFFFFFFFF;

    switch (hwCoreIndex)
    {
#if (BVBI_NUM_WSE >= 1)
    case 0:
        ulCoreOffset = 0;
        break;
#endif
#if (BVBI_NUM_WSE >= 2)
    case 1:
        ulCoreOffset = (BCHP_WSE_1_REG_START - BCHP_WSE_0_REG_START);
        break;
#endif
#if (BVBI_NUM_WSE >= 3)
    case 2:
        ulCoreOffset = (BCHP_WSE_2_REG_START - BCHP_WSE_0_REG_START);
        break;
#endif
    default:
        break;
    }

    return ulCoreOffset;
}
#endif

#if (BVBI_NUM_WSE_656 >= 1)
/***************************************************************************
 *
 */
static uint32_t P_GetCoreOffset_656_isrsafe (uint8_t hwCoreIndex)
{
    uint32_t ulCoreOffset = 0xFFFFFFFF;

    switch (hwCoreIndex)
    {
#if (BVBI_NUM_WSE_656 >= 1)
    case 0:
        ulCoreOffset = 0;
        break;
#endif
#if (BVBI_NUM_WSE_656 >= 2)
    case 1:
        ulCoreOffset =
            (BCHP_WSE_ANCIL_1_REG_START - BCHP_WSE_ANCIL_0_REG_START);
        break;
#endif
#if (BVBI_NUM_WSE_656 >= 3)
    case 2:
        ulCoreOffset =
            (BCHP_WSE_ANCIL_2_REG_START - BCHP_WSE_ANCIL_0_REG_START);
        break;
#endif
    default:
        break;
    }

    return ulCoreOffset;
}
#endif
