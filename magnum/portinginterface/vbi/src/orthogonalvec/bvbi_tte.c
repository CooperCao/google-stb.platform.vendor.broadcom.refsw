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
#include "bvbi.h"           /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bkni.h"           /* For critical sections */
#include "bvbi_priv.h"      /* VBI internal data structures */
#if (BVBI_NUM_TTE >= 1)
#include "bchp_tte_0.h"  /* RDB info for primary TTE core */
#endif
#if (BVBI_NUM_TTE >= 2)
#include "bchp_tte_1.h"   /* RDB info for secondary TTE core */
#endif
#if (BVBI_NUM_TTE >= 3)
#include "bchp_tte_2.h"   /* RDB info for tertiary TTE core */
#endif
#if (BVBI_NUM_TTE_656 >= 1)
#include "bchp_tte_ancil_0.h"   /* RDB info for ITU-R 656 "bypass" TTE core */
#endif

BDBG_MODULE(BVBI);

/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/
#if (BVBI_NUM_TTE >= 1) || (BVBI_NUM_TTE_656 >= 1)
static uint32_t P_GetCoreOffset_isr (bool is656, uint8_t hwCoreIndex);
#endif


/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/


/***************************************************************************
* Implementation of supporting teletext functions that are not in API
***************************************************************************/

#if (BVBI_NUM_TTE >= 1) || (BVBI_NUM_TTE_656 >= 1)
static BERR_Code BVBI_P_TT_Enc_Program_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    bool bActive,
    bool bXserActive,
    BFMT_VideoFmt eVideoFormat,
    bool tteShiftDirMsb2Lsb,
    BVBI_XSER_Settings* xserSettings,
    BVBI_P_TTData* topData,
    BVBI_P_TTData* botData
);
#endif

static void BVBI_P_TT_Enc_Init_isr (
    BREG_Handle hReg, bool is656, uint8_t hwCoreIndex);

/***************************************************************************
 *
 */
void BVBI_P_TT_Enc_Init (BREG_Handle hReg, bool is656, uint8_t hwCoreIndex)
{
    BKNI_EnterCriticalSection();
    BVBI_P_TT_Enc_Init_isr (hReg, is656, hwCoreIndex);
    BKNI_LeaveCriticalSection();
}

/***************************************************************************
 *
 */
void BVBI_P_TT_Enc_Init_isr (BREG_Handle hReg, bool is656, uint8_t hwCoreIndex)
{
    BDBG_ENTER(BVBI_P_TT_Enc_Init_isr);

    BVBI_P_VIE_SoftReset_isr (hReg, is656, hwCoreIndex, BVBI_P_SELECT_TT);

    BDBG_LEAVE(BVBI_P_TT_Enc_Init_isr);
}

#if (BVBI_NUM_TTE >= 1) || (BVBI_NUM_TTE_656 >= 1) /** { **/

/***************************************************************************
 *
 */
BERR_Code BVBI_P_TT_Enc_Program (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    bool bActive,
    bool bXserActive,
    BFMT_VideoFmt eVideoFormat,
    bool tteShiftDirMsb2Lsb,
    BVBI_XSER_Settings* xserSettings,
    BVBI_P_TTData* topData,
    BVBI_P_TTData* botData
)
{
    BERR_Code retval;
    BDBG_ENTER(BVBI_P_TT_Enc_Program);
    BKNI_EnterCriticalSection();
    retval = BVBI_P_TT_Enc_Program_isr (
        hReg, is656, hwCoreIndex, bActive, bXserActive,
        eVideoFormat, tteShiftDirMsb2Lsb, xserSettings, topData, botData);
    BKNI_LeaveCriticalSection();
    BDBG_LEAVE(BVBI_P_TT_Enc_Program);
    return retval;
}

/***************************************************************************
 *
 */
static BERR_Code BVBI_P_TT_Enc_Program_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    bool bActive,
    bool bXserActive,
    BFMT_VideoFmt eVideoFormat,
    bool tteShiftDirMsb2Lsb,
    BVBI_XSER_Settings* xserSettings,
    BVBI_P_TTData* topData,
    BVBI_P_TTData* botData
)
{
    uint32_t ulControlReg;
    uint32_t ulFormatReg;
#if (BVBI_P_HAS_XSER_TT >= 1)
    uint32_t iSerialPortMode;
    uint32_t iSerialPort;
#endif

    uint8_t  ucNumLinesTF;
    uint8_t  ucNumLinesBF;
    uint8_t  ucBytesPerLine;

    BMMA_DeviceOffset offset;
    uint64_t ullAddressReg;
    uint32_t ulCoreOffset;
    uint32_t ulShiftDir;
    BERR_Code eErr;

#if (BVBI_P_HAS_XSER_TT >= 1)
#else
    BSTD_UNUSED (bXserActive);
    BSTD_UNUSED (xserSettings);
#endif
    BSTD_UNUSED (eErr);

    BDBG_ENTER(BVBI_P_TT_Enc_Program_isr);

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_isr (is656, hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_TT_Enc_Program_isr);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* TODO: Verify little endian */

    /* If user wants to turn off teletext processing, just reset the
       entire core. */
    if (!bActive)
    {
        BVBI_P_TT_Enc_Init_isr (hReg, is656, hwCoreIndex);
        BDBG_LEAVE(BVBI_P_TT_Enc_Program_isr);
        return BERR_SUCCESS;
    }

#if (BVBI_P_HAS_XSER_TT >= 1)
    iSerialPort = (bXserActive ?
        BCHP_TTE_0_control_serial_port_ENABLE :
        BCHP_TTE_0_control_serial_port_DISABLE);
    switch (xserSettings->xsSerialDataContent)
    {
    case BVBI_TTserialDataContent_None:
        iSerialPortMode = BCHP_TTE_0_control_serial_port_mode_DATA_ONLY;
        iSerialPort = BCHP_TTE_0_control_serial_port_DISABLE;
        break;
    case BVBI_TTserialDataContent_DataOnly:
        iSerialPortMode = BCHP_TTE_0_control_serial_port_mode_DATA_ONLY;
        break;
    case BVBI_TTserialDataContent_DataMag:
        iSerialPortMode = BCHP_TTE_0_control_serial_port_mode_MAGAZINE_DATA;
        break;
    case BVBI_TTserialDataContent_DataMagFrm:
        iSerialPortMode =
            BCHP_TTE_0_control_serial_port_mode_FRM_MAGAZINE_DATA;
        break;
    case BVBI_TTserialDataContent_DataMagFrmRun:
        iSerialPortMode =
            BCHP_TTE_0_control_serial_port_mode_RUNIN_FRM_MAGAZINE_DATA;
        break;
    default:
        iSerialPortMode = BCHP_TTE_0_control_serial_port_mode_DATA_ONLY;
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_TT_Enc_Program_isr);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
        break;
    }
#endif

    if (tteShiftDirMsb2Lsb)
        ulShiftDir = BCHP_TTE_0_control_shift_direction_MSBToLSB;
    else
        ulShiftDir = BCHP_TTE_0_control_shift_direction_LSBToMSB;

    /* Start programming the TTE control register */
    ulControlReg = BREG_Read32 (hReg, BCHP_TTE_0_control + ulCoreOffset);
    ulControlReg &= ~(
#if (BVBI_P_HAS_XSER_TT >= 1)
        BCHP_MASK       (TTE_0_control, serial_port_mode         ) |
        BCHP_MASK       (TTE_0_control, serial_port              ) |
#endif
        BCHP_MASK       (TTE_0_control, constant_phase           ) |
        BCHP_MASK       (TTE_0_control, anci656_enable           ) |
        BCHP_MASK       (TTE_0_control, anci656_output_fc        ) |
        BCHP_MASK       (TTE_0_control, shift_direction          ) |
        BCHP_MASK       (TTE_0_control, enable_tf                ) |
        BCHP_MASK       (TTE_0_control, enable_bf                ) );
    ulControlReg |= (
#if (BVBI_P_HAS_XSER_TT >= 1)
         BCHP_FIELD_DATA (TTE_0_control, serial_port_mode,
                                                     iSerialPortMode) |
         BCHP_FIELD_DATA (TTE_0_control, serial_port, iSerialPort) |
#endif
        BCHP_FIELD_DATA (TTE_0_control, constant_phase,         0) |
        BCHP_FIELD_DATA (TTE_0_control, anci656_enable,         1) |
        BCHP_FIELD_DATA (TTE_0_control, anci656_output_fc,      1) |
        BCHP_FIELD_DATA (TTE_0_control, shift_direction, ulShiftDir) |
        BCHP_FIELD_DATA (TTE_0_control, enable_tf,              1) |
        BCHP_FIELD_DATA (TTE_0_control, enable_bf,              1) );

    /* Program the TTE top mask register */
    BREG_Write32 (hReg, BCHP_TTE_0_top_mask + ulCoreOffset, 0x0);

    /* Program the TTE bottom mask register */
    BREG_Write32 (hReg, BCHP_TTE_0_bottom_mask + ulCoreOffset, 0x0);

    /* Start programming the output format register */
    ulFormatReg =
        BREG_Read32 (hReg, BCHP_TTE_0_output_format + ulCoreOffset);

    /* Select video format */
    switch (eVideoFormat)
    {
    case BFMT_VideoFmt_eNTSC:
    case BFMT_VideoFmt_eNTSC_J:
    case BFMT_VideoFmt_e720x482_NTSC:
    case BFMT_VideoFmt_e720x482_NTSC_J:
    case BFMT_VideoFmt_ePAL_M:
        /* NTSC specific settings */

        ucNumLinesTF   =  11;
        ucNumLinesBF   =  11;
        ucBytesPerLine =  34;

        /* Continue programming the control register */
        ulControlReg &= ~(
            BCHP_MASK       (TTE_0_control, start_delay         ) |
            BCHP_MASK       (TTE_0_control, teletext_mode       ) );
        ulControlReg |= (
            BCHP_FIELD_DATA (TTE_0_control, start_delay,    0x1F) |
            BCHP_FIELD_ENUM (TTE_0_control, teletext_mode, NABTS) );

        /* Continue programming the output_format register */
        ulFormatReg &=
            ~BCHP_MASK       (TTE_0_output_format, output_attenuation     );
        ulFormatReg |=
             BCHP_FIELD_DATA (TTE_0_output_format, output_attenuation,0x63);

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
        /* 576I settings */

        ucNumLinesTF   = 17;
        ucNumLinesBF   = 18;
        ucBytesPerLine = 43;

        /* Continue programming the control register */
        ulControlReg &= ~(
            BCHP_MASK       (TTE_0_control, start_delay               ) |
            BCHP_MASK       (TTE_0_control, teletext_mode             ) );
        ulControlReg |= (
            BCHP_FIELD_DATA (TTE_0_control, start_delay,          0x00) |
            BCHP_FIELD_ENUM (TTE_0_control, teletext_mode, ETSTeletext) );

        /* Continue programming the output_format register */
        ulFormatReg &=
            ~BCHP_MASK       (TTE_0_output_format, output_attenuation      );
        ulFormatReg |=
             BCHP_FIELD_DATA (TTE_0_output_format, output_attenuation, 0x5a);

        break;

    default:
        BDBG_ERR(("BVBI_TTE: video format %d not supported", eVideoFormat));
        BDBG_LEAVE(BVBI_P_TT_Enc_Program_isr);
        return BERR_TRACE (BERR_INVALID_PARAMETER);
        break;
    }

    /* Prepare to send data in the encode handle */
    offset = topData->mmaData.pHwAccess;
    ullAddressReg =
        BCHP_FIELD_DATA (TTE_0_read_address_top, SlabAddress, offset);
    BREG_WriteAddr (hReg,
        BCHP_TTE_0_read_address_top    + ulCoreOffset, ullAddressReg);
    offset = botData->mmaData.pHwAccess;
    ullAddressReg =
        BCHP_FIELD_DATA (TTE_0_read_address_bottom, SlabAddress, offset);
    BREG_WriteAddr (hReg,
        BCHP_TTE_0_read_address_bottom + ulCoreOffset, ullAddressReg);

    /* Update the field handles that send the data */
    topData->ucLines    = ucNumLinesTF;
    topData->ucLineSize = ucBytesPerLine;
    botData->ucLines    = ucNumLinesBF;
    botData->ucLineSize = ucBytesPerLine;

    /* write the three registers with updated values */
    BREG_Write32 (
        hReg, BCHP_TTE_0_control       + ulCoreOffset, ulControlReg);
    BREG_Write32 (
        hReg, BCHP_TTE_0_output_format + ulCoreOffset,  ulFormatReg);

    BDBG_LEAVE(BVBI_P_TT_Enc_Program_isr);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
uint32_t BVBI_P_TT_Encode_Data_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    BAVC_Polarity polarity,
    bool bPR18010_bad_line_number,
    BVBI_P_TTData* pTTDataNext )
{
/*
    Programming note: the implementation here assumes that the bitfield layout
    within registers is the same for all teletext encoder cores in the chip.

    If a chip is built that has multiple teletext encoder cores that are not
    identical, then this routine will have to be redesigned.
*/
    uint32_t ulCoreOffset;
    uint32_t H_ReAdTop;
    uint32_t H_ReAdBot;
    uint32_t H_MaskTop;
    uint32_t H_MaskBot;
    uint32_t H_Lines;
    uint32_t H_Status;
    uint32_t ulStatusReg;
    uint64_t ullAddressReg;
    uint16_t usStartLineTF;
    uint16_t usStartLineBF;
    uint8_t  ucMinLines;
    uint8_t  ucMinLineSize;
    uint32_t ulLinesReg;
    uint32_t lineMask;
    BVBI_P_MmaData* pMmaData;
    uint32_t ulErrInfo = 0;

    /* Debug code
    uint8_t* printme = 0;
    */

#if (BVBI_NUM_TTE_656 >= 1)
#else
    BSTD_UNUSED (bPR18010_bad_line_number);
#endif

    BDBG_ENTER(BVBI_P_TT_Encode_Data_isr);

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_isr (is656, hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_TT_Encode_Data_isr);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    H_ReAdTop = BCHP_TTE_0_read_address_top + ulCoreOffset;
    H_ReAdBot = BCHP_TTE_0_read_address_bottom + ulCoreOffset;
    H_MaskTop = BCHP_TTE_0_top_mask + ulCoreOffset;
    H_MaskBot = BCHP_TTE_0_bottom_mask + ulCoreOffset;
    H_Lines   = BCHP_TTE_0_lines_active + ulCoreOffset;
    H_Status  = BCHP_TTE_0_status + ulCoreOffset;

    /* Verify that field handle is big enough to hold the TT data */
    switch (eVideoFormat)
    {
    case BFMT_VideoFmt_eNTSC:
    case BFMT_VideoFmt_eNTSC_J:
    case BFMT_VideoFmt_e720x482_NTSC:
    case BFMT_VideoFmt_e720x482_NTSC_J:
    case BFMT_VideoFmt_ePAL_M:
        ucMinLines    = 13;
        ucMinLineSize = 34;
        usStartLineTF  =  10 - 1;
        usStartLineBF  = 273 - 263;
#if (BVBI_NUM_TTE_656 >= 1)
        if (is656 && !bPR18010_bad_line_number)
        {
            usStartLineTF -= 1;
            usStartLineBF -= 1;
        }
#endif
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
        ucMinLines    = 18;
        ucMinLineSize = 43;
        usStartLineTF  = 6 - 1;
        usStartLineBF  = 318 - 313;
#if (BVBI_NUM_TTE_656 >= 1)
        if (is656 && !bPR18010_bad_line_number)
        {
            usStartLineTF -= 1;
            usStartLineBF -= 1;
        }
#endif
        /* PR18343 */
        usStartLineTF -= 1;
        break;
    default:
        /* This should never happen! */
        ulErrInfo = (uint32_t)(-1);
        BDBG_ERR(("BVBI_TTE: video format %d not supported", eVideoFormat));
        BDBG_LEAVE(BVBI_P_TT_Encode_Data_isr);
        return ulErrInfo;
        break;
    }
    if ( (pTTDataNext->ucLines     >    ucMinLines) ||
         (pTTDataNext->ucLineSize != ucMinLineSize)    )
    {
        ulErrInfo |= BVBI_LINE_ERROR_FLDH_CONFLICT;
        BDBG_LEAVE(BVBI_P_TT_Encode_Data_isr);
        return ulErrInfo;
    }

    /* Convenience */
    pMmaData = &(pTTDataNext->mmaData);

    /* Read the status register */
    ulStatusReg = BREG_Read32 (hReg, H_Status);

    /* Start programming the lines_active register */
    ulLinesReg = BREG_Read32 (hReg, H_Lines);

    /* If top field */
    if (polarity == BAVC_Polarity_eTopField)
    {
        /* Check for hardware busy */
        if ((ulStatusReg & BCHP_MASK (TTE_0_status, data_sent_tf)) == 0)
        {
            ulErrInfo |= BVBI_LINE_ERROR_TELETEXT_OVERRUN;
            goto done;
        }

        /* Will clear hardware status */
        ulStatusReg = BCHP_MASK (TTE_0_status, data_sent_tf);

        /* Give hardware a new place to encode data from */
        ullAddressReg = BCHP_FIELD_DATA (
            TTE_0_read_address_top, SlabAddress, pMmaData->pHwAccess);
        BREG_WriteAddr (hReg, H_ReAdTop, ullAddressReg);

        /* Program the masking register */
#if (BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE)
        lineMask =  pTTDataNext->lineMask;
#else
        lineMask =  BVBI_P_LEBE_SWAP (pTTDataNext->lineMask);
#endif
        BREG_Write32 (hReg, H_MaskTop, 0xFFFFFFFF);
        *(uint32_t*)(pMmaData->pSwAccess) = lineMask;
        BVBI_P_MmaFlush_isrsafe (pMmaData, sizeof(uint32_t));

        /* Continue programming the lines_active register */
        ulLinesReg &= ~BCHP_MASK (TTE_0_lines_active, startline_tf);
        ulLinesReg |=  BCHP_FIELD_DATA (
            TTE_0_lines_active, startline_tf,
            pTTDataNext->firstLine + usStartLineTF);

        /* Debug code
        printme = pTTDataNext->pucData;
        */
    }
    else /* Bottom field */
    {
        /* Check for hardware busy */
        if ((ulStatusReg & BCHP_MASK (TTE_0_status, data_sent_bf)) == 0)
        {
            ulErrInfo |= BVBI_LINE_ERROR_TELETEXT_OVERRUN;
            goto done;
        }

        /* Will clear hardware status */
        ulStatusReg = BCHP_MASK (TTE_0_status, data_sent_bf);

        /* Give hardware a new place to encode data from */
        ullAddressReg = BCHP_FIELD_DATA (
            TTE_0_read_address_bottom, SlabAddress, pMmaData->pHwAccess);
        BREG_WriteAddr (hReg, H_ReAdBot, ullAddressReg);

        /* Program the masking register */
#if (BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE)
        lineMask =  pTTDataNext->lineMask;
#else
        lineMask =  BVBI_P_LEBE_SWAP (pTTDataNext->lineMask);
#endif
        BREG_Write32 (hReg, H_MaskBot, 0xFFFFFFFF);
        *(uint32_t*)(pMmaData->pSwAccess) = lineMask;
        BVBI_P_MmaFlush_isrsafe (pMmaData, sizeof(uint32_t));

        /* Continue programming the lines_active register */
        ulLinesReg &= ~BCHP_MASK (TTE_0_lines_active, startline_bf);
        ulLinesReg |=  BCHP_FIELD_DATA (
            TTE_0_lines_active, startline_bf,
            pTTDataNext->firstLine + usStartLineBF);
    }

    /* Finish programming the lines_active register */
    BREG_Write32 (hReg, H_Lines, ulLinesReg);

    /* Finish clearing status */
    BREG_Write32 (hReg, H_Status, ulStatusReg);

    /* Debug code */
    /*
    {
    static uint32_t dcounter = 0;
    ++dcounter;
    if ((dcounter > 80) && (dcounter < 150))
    {
        if (printme)
        {
            uint32_t mask = *(uint32_t*)printme;
            char* p1 = printme + 4;
            char* p2 = printme + (4 + 34);
            printf ("%d%c: At %08x: encoding M:%08x \"%s\" \"%s\"\n",
                dcounter,
                (polarity == BAVC_Polarity_eTopField) ? 'T' : 'B',
                offset,
                mask, p1, p2);
        }
        else
            printf ("%d%c: Did not encode anything\n",
                dcounter,
                (polarity == BAVC_Polarity_eTopField) ? 'T' : 'B');
        {
        }
    }
    }
    */

done:
    BDBG_LEAVE(BVBI_P_TT_Encode_Data_isr);
    return ulErrInfo;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVBI_P_TT_Encode_Enable_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    bool bEnable)
{
    uint32_t ulCoreOffset;
    uint32_t ulControlReg;

    /* TODO: handle progressive video */
    BSTD_UNUSED (eVideoFormat);

    BDBG_ENTER(BVBI_P_TT_Encode_Enable_isr);

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_isr (is656, hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_TT_Encode_Enable_isr);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    ulControlReg = BREG_Read32 ( hReg, BCHP_TTE_0_control + ulCoreOffset );
    ulControlReg &= ~(
        BCHP_MASK (TTE_0_control, enable_tf) |
        BCHP_MASK (TTE_0_control, enable_bf) );
    if (bEnable)
    {
        ulControlReg |= (
            BCHP_FIELD_DATA (TTE_0_control, enable_tf, 1) |
            BCHP_FIELD_DATA (TTE_0_control, enable_bf, 1) );
    }
    else
    {
        ulControlReg |= (
            BCHP_FIELD_DATA (TTE_0_control, enable_tf, 0) |
            BCHP_FIELD_DATA (TTE_0_control, enable_bf, 0) );
    }
    BREG_Write32 ( hReg, BCHP_TTE_0_control + ulCoreOffset, ulControlReg );

    BDBG_LEAVE(BVBI_P_TT_Encode_Enable_isr);
    return BERR_SUCCESS;
}
#endif

#endif /** } (BVBI_NUM_TTE >= 1) **/

/***************************************************************************
* Static (private) functions
***************************************************************************/

#if (BVBI_NUM_TTE >= 1) || (BVBI_NUM_TTE_656 >= 1)
/***************************************************************************
 *
 */
static uint32_t P_GetCoreOffset_isr (bool is656, uint8_t hwCoreIndex)
{
    uint32_t ulCoreOffset = 0xFFFFFFFF;

    if (is656)
    {
#if (BVBI_NUM_TTE_656 >= 1)
        ulCoreOffset = (BCHP_TTE_ANCIL_0_status - BCHP_TTE_0_status);
#endif
    }
    else
    {
        switch (hwCoreIndex)
        {
#if (BVBI_NUM_TTE >= 1)
        case 0:
            ulCoreOffset = 0;
            break;
#endif
#if (BVBI_NUM_TTE >= 2)
        case 1:
            ulCoreOffset = (BCHP_TTE_1_status - BCHP_TTE_0_status);
            break;
#endif
#if (BVBI_NUM_TTE >= 3)
        case 2:
            ulCoreOffset = (BCHP_TTE_1_status - BCHP_TTE_0_status);
            break;
#endif
        default:
            break;
        }
    }

    return ulCoreOffset;
}
#endif

/* End of file */
