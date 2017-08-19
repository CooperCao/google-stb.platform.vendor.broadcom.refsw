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
#include "bkni.h"                       /* For critical sections */
#include "bvbi.h"           /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"      /* VBI internal data structures */
#if (BVBI_NUM_GSE >= 1)
#include "bchp_gse_0.h"  /* RDB info for primary Gemstar encoder core */
#endif
#if (BVBI_NUM_GSE_656 >= 1)
#include "bchp_gse_ancil_0.h"   /* RDB info for bypass Gemstar encoder core */
#endif


BDBG_MODULE(BVBI);

/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/

#if (BVBI_NUM_GSE >= 1)
static void BVBI_P_ProgramNull_isr (
        BREG_Handle hReg, uint32_t coreOffset,
        uint32_t ulWritePointer, uint32_t value);
static uint32_t P_GetCoreOffset_isr (uint8_t hwCoreIndex);
#endif

#if (BVBI_NUM_GSE_656 >= 1)
static void BVBI_P_ProgramNull_656_isr (
        BREG_Handle hReg, uint32_t coreOffset,
        uint32_t ulWritePointer, uint32_t value);
static uint32_t P_GetCoreOffset_656_isr (uint8_t hwCoreIndex);
#endif


/***************************************************************************
* Implementation of supporting GS functions that are not in API
***************************************************************************/

#if (BVBI_NUM_GSE >= 1)  /** { **/
/***************************************************************************
 *
 */
void BVBI_P_GS_Enc_Init (BREG_Handle hReg, uint8_t hwCoreIndex)
{
        BDBG_ENTER(BVBI_P_GS_Enc_Init);

        BVBI_P_VIE_SoftReset_isr (hReg, false, hwCoreIndex, BVBI_P_SELECT_GS);

        BDBG_LEAVE(BVBI_P_GS_Enc_Init);
}
#endif /** }  BVBI_NUM_GSE **/

#if (BVBI_NUM_GSE_656 >= 1)  /** { **/
/***************************************************************************
 *
 */
void BVBI_P_GS_Enc_656_Init (BREG_Handle hReg, uint8_t hwCoreIndex)
{
        BDBG_ENTER(BVBI_P_GS_Enc_656_Init);

        BVBI_P_VIE_SoftReset_isr (hReg, true, hwCoreIndex, BVBI_P_SELECT_GS);

        BDBG_LEAVE(BVBI_P_GS_Enc_656_Init);
}
#endif /** }  BVBI_NUM_GSE_656 **/

#if (BVBI_NUM_GSE >= 1)  /** { **/
BERR_Code BVBI_P_GS_Enc_Program (
        BREG_Handle hReg,
        uint8_t hwCoreIndex,
        bool bActive,
        BFMT_VideoFmt eVideoFormat,
        bool bArib480p,
        BVBI_GSOptions* gsOptions)
{
/*
        Programming note: the implementation here assumes that the bitfield layout
        within registers is the same for all GS encoder cores in the chip.

        If a chip is built that has multiple GS encoder cores that are not
        identical, then this routine will have to be redesigned.
*/

        uint32_t ulCoreOffset;
        uint32_t ulGse_controlReg;
        uint32_t baseline;
        uint32_t linemask;
        uint32_t waveform = (
                gsOptions->bTvg2x ?
                        BCHP_GSE_0_CONTROL_WAVE_MODE_TVG2X_CEA2020 :
                        BCHP_GSE_0_CONTROL_WAVE_MODE_GEMSTAR);

        BDBG_ENTER(BVBI_P_GS_Enc_Program);

        /* Figure out which encoder core to use */
        ulCoreOffset = P_GetCoreOffset_isr (hwCoreIndex);
        if (ulCoreOffset == 0xFFFFFFFF)
        {
                /* This should never happen!  This parameter was checked by
                   BVBI_Encode_Create() */
                BDBG_LEAVE(BVBI_P_GS_Enc_Program);
                return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        /* Complain if video format is not supported */
        switch (eVideoFormat)
        {
        case BFMT_VideoFmt_eNTSC:
        case BFMT_VideoFmt_eNTSC_J:
        case BFMT_VideoFmt_e720x482_NTSC:
        case BFMT_VideoFmt_e720x482_NTSC_J:
                break;

        default:
                if (bActive)
                {
                        BDBG_ERR(("BVBI_GSE: video format %d not supported", eVideoFormat));
                        return BERR_TRACE (BVBI_ERR_VFMT_CONFLICT);
                }
        }

        baseline = gsOptions->baseline_top;
        linemask = gsOptions->linemask_top;
        if (bArib480p)
        {
                if (baseline == 0)
                {
                        linemask >>= 1;
                }
                else
                {
                        --baseline;
                }
        }
        ulGse_controlReg = 0;
        ulGse_controlReg |= (
                BCHP_FIELD_DATA (GSE_0_ACTIVE_LINE_TOP, ACTIVE_LINE,     linemask) |
                BCHP_FIELD_ENUM (GSE_0_ACTIVE_LINE_TOP, PED_LINE5,        DISABLE) |
                BCHP_FIELD_ENUM (GSE_0_ACTIVE_LINE_TOP, PED_LINE4,        DISABLE) |
                BCHP_FIELD_ENUM (GSE_0_ACTIVE_LINE_TOP, PED_LINE3,        DISABLE) |
                BCHP_FIELD_ENUM (GSE_0_ACTIVE_LINE_TOP, PED_LINE2,        DISABLE) |
                BCHP_FIELD_ENUM (GSE_0_ACTIVE_LINE_TOP, PED_LINE1,        DISABLE) |
                BCHP_FIELD_DATA (GSE_0_ACTIVE_LINE_TOP, BASE,            baseline)
        );
        BREG_Write32 (
                hReg, BCHP_GSE_0_ACTIVE_LINE_TOP + ulCoreOffset, ulGse_controlReg);

        baseline = gsOptions->baseline_bot - 256;
        linemask = gsOptions->linemask_bot;
        if (bArib480p)
        {
                if (baseline == 0)
                {
                        linemask >>= 1;
                }
                else
                {
                        --baseline;
                }
        }
        ulGse_controlReg = 0;
        ulGse_controlReg |= (
                BCHP_FIELD_DATA (GSE_0_ACTIVE_LINE_BOT, ACTIVE_LINE,     linemask) |
                BCHP_FIELD_ENUM (GSE_0_ACTIVE_LINE_BOT, PED_LINE5,        DISABLE) |
                BCHP_FIELD_ENUM (GSE_0_ACTIVE_LINE_BOT, PED_LINE4,        DISABLE) |
                BCHP_FIELD_ENUM (GSE_0_ACTIVE_LINE_BOT, PED_LINE3,        DISABLE) |
                BCHP_FIELD_ENUM (GSE_0_ACTIVE_LINE_BOT, PED_LINE2,        DISABLE) |
                BCHP_FIELD_ENUM (GSE_0_ACTIVE_LINE_BOT, PED_LINE1,        DISABLE) |
                BCHP_FIELD_DATA (GSE_0_ACTIVE_LINE_BOT, BASE,            baseline)
        );
        BREG_Write32 (
                hReg, BCHP_GSE_0_ACTIVE_LINE_BOT + ulCoreOffset, ulGse_controlReg);

        ulGse_controlReg = 0;
        ulGse_controlReg |= (
                BCHP_FIELD_DATA (GSE_0_GAIN_TOP, LINE4, 0x48) |
                BCHP_FIELD_DATA (GSE_0_GAIN_TOP, LINE3, 0x48) |
                BCHP_FIELD_DATA (GSE_0_GAIN_TOP, LINE2, 0x48) |
                BCHP_FIELD_DATA (GSE_0_GAIN_TOP, LINE1, 0x48)
        );
        BREG_Write32 (
                hReg, BCHP_GSE_0_GAIN_TOP + ulCoreOffset, ulGse_controlReg);

        ulGse_controlReg = 0;
        ulGse_controlReg |=
                BCHP_FIELD_DATA (GSE_0_GAIN_EXT_TOP, LINE5, 70);
        BREG_Write32 (
                hReg, BCHP_GSE_0_GAIN_EXT_TOP + ulCoreOffset, ulGse_controlReg);

        ulGse_controlReg = 0;
        ulGse_controlReg |= (
                BCHP_FIELD_DATA (GSE_0_GAIN_BOT, LINE4, 70) |
                BCHP_FIELD_DATA (GSE_0_GAIN_BOT, LINE3, 70) |
                BCHP_FIELD_DATA (GSE_0_GAIN_BOT, LINE2, 70) |
                BCHP_FIELD_DATA (GSE_0_GAIN_BOT, LINE1, 70)
        );
        BREG_Write32 (
                hReg, BCHP_GSE_0_GAIN_BOT + ulCoreOffset, ulGse_controlReg);

        ulGse_controlReg = 0;
        ulGse_controlReg |=
                BCHP_FIELD_DATA (GSE_0_GAIN_EXT_BOT, LINE5, 70);
        BREG_Write32 (
                hReg, BCHP_GSE_0_GAIN_EXT_BOT + ulCoreOffset, ulGse_controlReg);

        ulGse_controlReg = 0;
        ulGse_controlReg |= (
                BCHP_FIELD_DATA (GSE_0_NULL, NULL_ENABLE_BANK3,    0) |
                BCHP_FIELD_DATA (GSE_0_NULL, NULL_ENABLE_BANK2,    0) |
                BCHP_FIELD_DATA (GSE_0_NULL, NULL_ENABLE_BANK1,    0) |
                BCHP_FIELD_DATA (GSE_0_NULL, NULL_ENABLE_BANK0,    0) |
                BCHP_FIELD_DATA (GSE_0_NULL, CHARACTER,         0x20)
        );
        BREG_Write32 (hReg, BCHP_GSE_0_NULL + ulCoreOffset, ulGse_controlReg);

        ulGse_controlReg = 0;
        ulGse_controlReg |= (
                BCHP_FIELD_DATA (GSE_0_CONTROL, FIFO_FREEZE,                    0) |
                BCHP_FIELD_DATA (GSE_0_CONTROL, NULL_MODE,                      1) |
                BCHP_FIELD_ENUM (GSE_0_CONTROL, BIT_WIDTH,                   NTSC) |
                BCHP_FIELD_DATA (GSE_0_CONTROL, DELAY_COUNT,                 0x42) |
                BCHP_FIELD_ENUM (GSE_0_CONTROL, PARITY_TYPE,                 EVEN) |
                BCHP_FIELD_ENUM (GSE_0_CONTROL, TOP_FLD_PARITY,         AUTOMATIC) |
                BCHP_FIELD_ENUM (GSE_0_CONTROL, BOT_FLD_PARITY,         AUTOMATIC) |
                BCHP_FIELD_ENUM (GSE_0_CONTROL, BYTE_SWAP_656_ANCIL,
                                                                           LITTLE_ENDIAN) |
                BCHP_FIELD_ENUM (GSE_0_CONTROL, BYTE_SWAP_VIDEO_SAMPLE,
                                                                           LITTLE_ENDIAN) |
                BCHP_FIELD_ENUM (GSE_0_CONTROL, SHIFT_DIRECTION,          LSB2MSB)
        );
        if (bActive)
        {
                ulGse_controlReg |= (
                        BCHP_FIELD_DATA (GSE_0_CONTROL, WAVE_MODE, waveform) |
                        BCHP_FIELD_ENUM (GSE_0_CONTROL,    ENABLE,  ENABLED) );
        }
        else
        {
                ulGse_controlReg |=
                        BCHP_FIELD_ENUM (GSE_0_CONTROL, ENABLE, DISABLED);
        }
        BREG_Write32 (hReg, BCHP_GSE_0_CONTROL + ulCoreOffset, ulGse_controlReg);

        BDBG_LEAVE(BVBI_P_GS_Enc_Program);
        return BERR_SUCCESS;
}
#endif /** }  BVBI_NUM_GSE **/

#if (BVBI_NUM_GSE_656 >= 1)  /** { **/
BERR_Code BVBI_P_GS_Enc_656_Program (
        BREG_Handle hReg,
        uint8_t hwCoreIndex,
        bool bActive,
        BFMT_VideoFmt eVideoFormat,
        bool bArib480p,
        BVBI_GSOptions* gsOptions)
{
/*
        Programming note: the implementation here assumes that the bitfield layout
        within registers is the same for all GS encoder cores in the chip.

        If a chip is built that has multiple GS encoder cores that are not
        identical, then this routine will have to be redesigned.
*/

        uint32_t ulCoreOffset;
        uint32_t ulGse_controlReg;
        uint32_t baseline;
        uint32_t linemask;

        BDBG_ENTER(BVBI_P_GS_Enc_656_Program);

        /* Figure out which encoder core to use */
        ulCoreOffset = P_GetCoreOffset_656_isr (hwCoreIndex);
        if (ulCoreOffset == 0xFFFFFFFF)
        {
                /* This should never happen!  This parameter was checked by
                   BVBI_Encode_Create() */
                BDBG_LEAVE(BVBI_P_GS_Enc_656_Program);
                return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        /* Complain if video format is not supported */
        switch (eVideoFormat)
        {
        case BFMT_VideoFmt_eNTSC:
        case BFMT_VideoFmt_eNTSC_J:
        case BFMT_VideoFmt_e720x482_NTSC:
        case BFMT_VideoFmt_e720x482_NTSC_J:
                break;

        default:
                if (bActive)
                {
                        BDBG_ERR(("BVBI_GSE: video format %d not supported", eVideoFormat));
                        return BERR_TRACE (BVBI_ERR_VFMT_CONFLICT);
                }
        }

        baseline = gsOptions->baseline_top;
        linemask = gsOptions->linemask_top;
        if (bArib480p)
        {
                if (baseline == 0)
                {
                        linemask >>= 1;
                }
                else
                {
                        --baseline;
                }
        }
        ulGse_controlReg = 0;
        ulGse_controlReg |= (
                BCHP_FIELD_DATA (GSE_ANCIL_0_ACTIVE_LINE_TOP, ACTIVE_LINE,     linemask) |
                BCHP_FIELD_ENUM (GSE_ANCIL_0_ACTIVE_LINE_TOP, PED_LINE5,        DISABLE) |
                BCHP_FIELD_ENUM (GSE_ANCIL_0_ACTIVE_LINE_TOP, PED_LINE4,        DISABLE) |
                BCHP_FIELD_ENUM (GSE_ANCIL_0_ACTIVE_LINE_TOP, PED_LINE3,        DISABLE) |
                BCHP_FIELD_ENUM (GSE_ANCIL_0_ACTIVE_LINE_TOP, PED_LINE2,        DISABLE) |
                BCHP_FIELD_ENUM (GSE_ANCIL_0_ACTIVE_LINE_TOP, PED_LINE1,        DISABLE) |
                BCHP_FIELD_DATA (GSE_ANCIL_0_ACTIVE_LINE_TOP, BASE,            baseline)
        );
        BREG_Write32 (
                hReg, BCHP_GSE_ANCIL_0_ACTIVE_LINE_TOP + ulCoreOffset, ulGse_controlReg);

        baseline = gsOptions->baseline_bot - 256;
        linemask = gsOptions->linemask_bot;
        if (bArib480p)
        {
                if (baseline == 0)
                {
                        linemask >>= 1;
                }
                else
                {
                        --baseline;
                }
        }
        ulGse_controlReg = 0;
        ulGse_controlReg |= (
                BCHP_FIELD_DATA (GSE_ANCIL_0_ACTIVE_LINE_BOT, ACTIVE_LINE,     linemask) |
                BCHP_FIELD_ENUM (GSE_ANCIL_0_ACTIVE_LINE_BOT, PED_LINE5,        DISABLE) |
                BCHP_FIELD_ENUM (GSE_ANCIL_0_ACTIVE_LINE_BOT, PED_LINE4,        DISABLE) |
                BCHP_FIELD_ENUM (GSE_ANCIL_0_ACTIVE_LINE_BOT, PED_LINE3,        DISABLE) |
                BCHP_FIELD_ENUM (GSE_ANCIL_0_ACTIVE_LINE_BOT, PED_LINE2,        DISABLE) |
                BCHP_FIELD_ENUM (GSE_ANCIL_0_ACTIVE_LINE_BOT, PED_LINE1,        DISABLE) |
                BCHP_FIELD_DATA (GSE_ANCIL_0_ACTIVE_LINE_BOT, BASE,            baseline)
        );
        BREG_Write32 (
                hReg, BCHP_GSE_ANCIL_0_ACTIVE_LINE_BOT + ulCoreOffset, ulGse_controlReg);

        ulGse_controlReg = 0;
        ulGse_controlReg |= (
                BCHP_FIELD_DATA (GSE_ANCIL_0_NULL, NULL_ENABLE_BANK3,    0) |
                BCHP_FIELD_DATA (GSE_ANCIL_0_NULL, NULL_ENABLE_BANK2,    0) |
                BCHP_FIELD_DATA (GSE_ANCIL_0_NULL, NULL_ENABLE_BANK1,    0) |
                BCHP_FIELD_DATA (GSE_ANCIL_0_NULL, NULL_ENABLE_BANK0,    0) |
                BCHP_FIELD_DATA (GSE_ANCIL_0_NULL, CHARACTER,         0x20)
        );
        BREG_Write32 (hReg, BCHP_GSE_ANCIL_0_NULL + ulCoreOffset, ulGse_controlReg);

        ulGse_controlReg = 0;
        ulGse_controlReg |= (
                BCHP_FIELD_DATA (GSE_ANCIL_0_CONTROL, FIFO_FREEZE,                    0) |
                BCHP_FIELD_DATA (GSE_ANCIL_0_CONTROL, NULL_MODE,                      1) |
                BCHP_FIELD_ENUM (GSE_ANCIL_0_CONTROL, BIT_WIDTH,                   NTSC) |
                BCHP_FIELD_ENUM (GSE_ANCIL_0_CONTROL, PARITY_TYPE,                 EVEN) |
                BCHP_FIELD_ENUM (GSE_ANCIL_0_CONTROL, TOP_FLD_PARITY,         AUTOMATIC) |
                BCHP_FIELD_ENUM (GSE_ANCIL_0_CONTROL, BOT_FLD_PARITY,         AUTOMATIC) |
                BCHP_FIELD_ENUM (GSE_ANCIL_0_CONTROL, BYTE_SWAP_656_ANCIL,
                                                                           LITTLE_ENDIAN) |
                BCHP_FIELD_ENUM (GSE_ANCIL_0_CONTROL, SHIFT_DIRECTION,          LSB2MSB)
        );
        if (bActive)
        {
                ulGse_controlReg |= (
                        BCHP_FIELD_ENUM (GSE_ANCIL_0_CONTROL,    ENABLE,  ENABLED) );
        }
        else
        {
                ulGse_controlReg |=
                        BCHP_FIELD_ENUM (GSE_ANCIL_0_CONTROL, ENABLE, DISABLED);
        }
        BREG_Write32 (hReg, BCHP_GSE_ANCIL_0_CONTROL + ulCoreOffset, ulGse_controlReg);

        BDBG_LEAVE(BVBI_P_GS_Enc_656_Program);
        return BERR_SUCCESS;
}
#endif /** }  BVBI_NUM_GSE_656 **/

#if (BVBI_NUM_GSE >= 1) /** { **/
uint32_t BVBI_P_GS_Encode_Data_isr (
        BREG_Handle hReg,
        uint8_t hwCoreIndex,
        BFMT_VideoFmt eVideoFormat,
        BAVC_Polarity polarity,
        BVBI_GSData* pGSData)
{
        uint32_t ulCoreOffset;
        uint32_t ulRegVal;
        uint32_t ulRegAddr;
        uint32_t ulReadPointer;
        uint32_t ulWritePointer;
        unsigned int iiLine;
        unsigned int iLine;
        unsigned int jLine;
        unsigned int jjLine;
        uint32_t lineIndex;
        uint32_t bankIndex;
        uint32_t hwActiveLine;
        uint32_t hwBase;
        uint32_t baseAdj;

        /* Debug code
        uint32_t dread_pointer[2];
        uint32_t dwrite_pointer[2];
        uint32_t status;
        */

        BDBG_ENTER(BVBI_P_GS_Encode_Data_isr);

        /* Size check for field data */
        if (!pGSData)
        {
                return (BVBI_LINE_ERROR_FLDH_CONFLICT);
        }

        /* Refuse service for data with known errors */
        if (pGSData->ulErrorLines != 0x0)
        {
                return (BVBI_LINE_ERROR_FLDH_CONFLICT);
        }

        /* Figure out which encoder core to use */
        ulCoreOffset = P_GetCoreOffset_isr (hwCoreIndex);
        if (ulCoreOffset == 0xFFFFFFFF)
        {
                /* This should never happen!  This parameter was checked by
                   BVBI_Encode_Create() */
                BDBG_LEAVE(BVBI_P_GS_Encode_Data_isr);
                return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        /* Complain if video format is not supported */
        switch (eVideoFormat)
        {
        case BFMT_VideoFmt_eNTSC:
        case BFMT_VideoFmt_eNTSC_J:
        case BFMT_VideoFmt_e720x482_NTSC:
        case BFMT_VideoFmt_e720x482_NTSC_J:
                break;

        default:
                /* Should not happen */
                BDBG_ERR(("BVBI_GSE: video format %d not supported", eVideoFormat));
                return (-1);
        }

        /* Retrieve GSE configuration */
        if (polarity == BAVC_Polarity_eTopField)
        {
                ulRegAddr = BCHP_GSE_0_ACTIVE_LINE_TOP;
                baseAdj = 0;
        }
        else
        {
                ulRegAddr = BCHP_GSE_0_ACTIVE_LINE_BOT;
                baseAdj = 7;
        }
        ulRegAddr += ulCoreOffset;
        ulRegVal = BREG_Read32 (hReg, ulRegAddr);
        hwActiveLine =
                BCHP_GET_FIELD_DATA (ulRegVal, GSE_0_ACTIVE_LINE_TOP, ACTIVE_LINE);
        hwBase       =
                BCHP_GET_FIELD_DATA (ulRegVal, GSE_0_ACTIVE_LINE_TOP,        BASE);
        hwBase -= baseAdj;

        /* Refuse service if any of the user's data is inconsistent with current
         * settings */
        for (iLine = 0 ; iLine < 32 ; ++iLine)
        {
                if (pGSData->ulDataLines & (1 << iLine))
                {
                        /* Equivalent hardware line */
                        if (
                                (iLine < hwBase) ||
                                !((hwActiveLine & (1 << (iLine - hwBase))))
                        )
                        {
                                return (BVBI_LINE_ERROR_FLDH_CONFLICT);
                        }
                }
        }

        /* Clear status bits */
        ulRegVal = BREG_Read32 (hReg, BCHP_GSE_0_STATUS + ulCoreOffset);
        /* Debug code
        status = ulRegVal;
        */
        ulRegVal &= 0x000003FF;
        BREG_Write32 (hReg, BCHP_GSE_0_STATUS + ulCoreOffset, ulRegVal);
        /* TODO: Check GSE_0_STATUS register? */

        /* Get FIFO pointers */
        ulRegVal = BREG_Read32 (hReg, BCHP_GSE_0_WRPTR + ulCoreOffset);
        ulWritePointer = BCHP_GET_FIELD_DATA (ulRegVal, GSE_0_WRPTR, VALUE);
        bankIndex = ulWritePointer & 0x00000003;
        ulRegVal = BREG_Read32 (hReg, BCHP_GSE_0_RDPTR + ulCoreOffset);
        ulReadPointer = BCHP_GET_FIELD_DATA (ulRegVal, GSE_0_RDPTR, VALUE);

        /* Debug code
        dread_pointer[0]  = ulReadPointer;
        dwrite_pointer[0] = ulWritePointer;
        */

        /* Check for FIFO full */
        if (((ulReadPointer & 0x3) == bankIndex     ) &&
            (ulReadPointer         != ulWritePointer)    )
        {
                /* Debug code
                printf ("\n   ***   Gemstar FIFO full!!!   ***\n\n");
                */
                return BVBI_LINE_ERROR_GEMSTAR_OVERRUN;
        }

        /* Handle field misalignment */
        if (
                ((bankIndex == 0) || (bankIndex == 2)) &&
                (polarity != BAVC_Polarity_eTopField)
        )
        {
                BVBI_P_ProgramNull_isr (hReg, ulCoreOffset, bankIndex, 1);
                ++ulWritePointer;
        }
        else if (
                ((bankIndex == 1) || (bankIndex == 3)) &&
                (polarity != BAVC_Polarity_eBotField)
        )
        {
                BVBI_P_ProgramNull_isr (hReg, ulCoreOffset, bankIndex, 1);
                ++ulWritePointer;
        }
        else
        {
                BVBI_P_ProgramNull_isr (hReg, ulCoreOffset, bankIndex, 0);
        }

        /* Now write the user's data */
        /* Programming note:
         * iLine and jLine apply to user's data.
         * iiLine and jjLine apply to hardware registers
         */
        jjLine = 0;
        jLine = 0;
        iiLine = 0;
        iLine = 0;
        /* Loop over user data */
        for ( ; iLine < 32 ; ++iLine)
        {
                if (pGSData->ulDataLines & (1 << iLine))
                {
                        /* Loop over hardware configuration bits */
                        for ( ; iiLine < iLine ; ++iiLine)
                        {
                                if ((hwActiveLine & (1 << (iiLine - hwBase))))
                                        ++jjLine;
                        }
                        lineIndex = jjLine;
                        ulRegAddr =
                                BCHP_GSE_0_DATA_LINE1_BANK0 + ulCoreOffset +
                                (20 * bankIndex)                          +
                                ( 4 * lineIndex);
                        BREG_Write32 (hReg, ulRegAddr, pGSData->ulData[jLine]);
                        ++jLine;
                }
        }

        /* Program the write pointer into hardware */
        ++ulWritePointer;
        ulRegVal = BCHP_FIELD_DATA (GSE_0_WRPTR, VALUE, ulWritePointer);
        BREG_Write32 (hReg, BCHP_GSE_0_WRPTR + ulCoreOffset, ulRegVal);

        /* Debug code
        ulRegVal = BREG_Read32 (hReg, BCHP_GSE_0_WRPTR + ulCoreOffset);
        ulWritePointer = BCHP_GET_FIELD_DATA (ulRegVal, GSE_0_WRPTR, VALUE);
        dwrite_pointer[1] = ulWritePointer;
        ulRegVal = BREG_Read32 (hReg, BCHP_GSE_0_RDPTR + ulCoreOffset);
        dread_pointer[1] = BCHP_GET_FIELD_DATA (ulRegVal, GSE_0_RDPTR, VALUE);
        printf (
                "Field %c:  status: %03x  R/W (%d/%d) --> (%d/%d)  data (%08x %08x %08x)\n",
                ((polarity == BAVC_Polarity_eTopField) ? 'T' : 'B'),
                status,
                dread_pointer[0], dwrite_pointer[0],
                dread_pointer[1], dwrite_pointer[1],
                pGSData->ulData[0],  pGSData->ulData[1],  pGSData->ulData[2]);
        */

        BDBG_LEAVE(BVBI_P_GS_Encode_Data_isr);
        return 0x0;
}
#endif /** } BVBI_NUM_GSE **/

#if (BVBI_NUM_GSE_656 >= 1) /** { **/
uint32_t BVBI_P_GS_Encode_656_Data_isr (
        BREG_Handle hReg,
        uint8_t hwCoreIndex,
        BFMT_VideoFmt eVideoFormat,
        BAVC_Polarity polarity,
        BVBI_GSData* pGSData)
{
        uint32_t ulCoreOffset;
        uint32_t ulRegVal;
        uint32_t ulRegAddr;
        uint32_t ulReadPointer;
        uint32_t ulWritePointer;
        unsigned int iiLine;
        unsigned int iLine;
        unsigned int jLine;
        unsigned int jjLine;
        uint32_t lineIndex;
        uint32_t bankIndex;
        uint32_t hwActiveLine;
        uint32_t hwBase;
        uint32_t baseAdj;

        /* Debug code
        uint32_t dread_pointer[2];
        uint32_t dwrite_pointer[2];
        uint32_t status;
        */

        BDBG_ENTER(BVBI_P_GS_Encode_656_Data_isr);

        /* Size check for field data */
        if (!pGSData)
        {
                return (BVBI_LINE_ERROR_FLDH_CONFLICT);
        }

        /* Refuse service for data with known errors */
        if (pGSData->ulErrorLines != 0x0)
        {
                return (BVBI_LINE_ERROR_FLDH_CONFLICT);
        }

        /* Figure out which encoder core to use */
        ulCoreOffset = P_GetCoreOffset_656_isr (hwCoreIndex);
        if (ulCoreOffset == 0xFFFFFFFF)
        {
                /* This should never happen!  This parameter was checked by
                   BVBI_Encode_Create() */
                BDBG_LEAVE(BVBI_P_GS_Encode_656_Data_isr);
                return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        /* Complain if video format is not supported */
        switch (eVideoFormat)
        {
        case BFMT_VideoFmt_eNTSC:
        case BFMT_VideoFmt_eNTSC_J:
        case BFMT_VideoFmt_e720x482_NTSC:
        case BFMT_VideoFmt_e720x482_NTSC_J:
                break;

        default:
                /* Should not happen */
                BDBG_ERR(("BVBI_GSE: video format %d not supported", eVideoFormat));
                return (-1);
        }

        /* Retrieve GSE configuration */
        if (polarity == BAVC_Polarity_eTopField)
        {
                ulRegAddr = BCHP_GSE_ANCIL_0_ACTIVE_LINE_TOP;
                baseAdj = 0;
        }
        else
        {
                ulRegAddr = BCHP_GSE_ANCIL_0_ACTIVE_LINE_BOT;
                baseAdj = 7;
        }
        ulRegAddr += ulCoreOffset;
        ulRegVal = BREG_Read32 (hReg, ulRegAddr);
        hwActiveLine =
                BCHP_GET_FIELD_DATA (ulRegVal, GSE_ANCIL_0_ACTIVE_LINE_TOP, ACTIVE_LINE);
        hwBase       =
                BCHP_GET_FIELD_DATA (ulRegVal, GSE_ANCIL_0_ACTIVE_LINE_TOP,        BASE);
        hwBase -= baseAdj;

        /* Refuse service if any of the user's data is inconsistent with current
         * settings */
        for (iLine = 0 ; iLine < 32 ; ++iLine)
        {
                if (pGSData->ulDataLines & (1 << iLine))
                {
                        /* Equivalent hardware line */
                        if (
                                (iLine < hwBase) ||
                                !((hwActiveLine & (1 << (iLine - hwBase))))
                        )
                        {
                                return (BVBI_LINE_ERROR_FLDH_CONFLICT);
                        }
                }
        }

        /* Clear status bits */
        ulRegVal = BREG_Read32 (hReg, BCHP_GSE_ANCIL_0_STATUS + ulCoreOffset);
        /* Debug code
        status = ulRegVal;
        */
        ulRegVal &= 0x000003FF;
        BREG_Write32 (hReg, BCHP_GSE_ANCIL_0_STATUS + ulCoreOffset, ulRegVal);
        /* TODO: Check GSE_ANCIL_0_STATUS register? */

        /* Get FIFO pointers */
        ulRegVal = BREG_Read32 (hReg, BCHP_GSE_ANCIL_0_WRPTR + ulCoreOffset);
        ulWritePointer = BCHP_GET_FIELD_DATA (ulRegVal, GSE_ANCIL_0_WRPTR, VALUE);
        bankIndex = ulWritePointer & 0x00000003;
        ulRegVal = BREG_Read32 (hReg, BCHP_GSE_ANCIL_0_RDPTR + ulCoreOffset);
        ulReadPointer = BCHP_GET_FIELD_DATA (ulRegVal, GSE_ANCIL_0_RDPTR, VALUE);

        /* Debug code
        dread_pointer[0]  = ulReadPointer;
        dwrite_pointer[0] = ulWritePointer;
        */

        /* Check for FIFO full */
        if (((ulReadPointer & 0x3) == bankIndex     ) &&
            (ulReadPointer         != ulWritePointer)    )
        {
                /* Debug code
                printf ("\n   ***   Gemstar FIFO full!!!   ***\n\n");
                */
                return BVBI_LINE_ERROR_GEMSTAR_OVERRUN;
        }

        /* Handle field misalignment */
        if (
                ((bankIndex == 0) || (bankIndex == 2)) &&
                (polarity != BAVC_Polarity_eTopField)
        )
        {
                BVBI_P_ProgramNull_656_isr (hReg, ulCoreOffset, bankIndex, 1);
                ++ulWritePointer;
        }
        else if (
                ((bankIndex == 1) || (bankIndex == 3)) &&
                (polarity != BAVC_Polarity_eBotField)
        )
        {
                BVBI_P_ProgramNull_656_isr (hReg, ulCoreOffset, bankIndex, 1);
                ++ulWritePointer;
        }
        else
        {
                BVBI_P_ProgramNull_656_isr (hReg, ulCoreOffset, bankIndex, 0);
        }

        /* Now write the user's data */
        /* Programming note:
         * iLine and jLine apply to user's data.
         * iiLine and jjLine apply to hardware registers
         */
        jjLine = 0;
        jLine = 0;
        iiLine = 0;
        iLine = 0;
        /* Loop over user data */
        for ( ; iLine < 32 ; ++iLine)
        {
                if (pGSData->ulDataLines & (1 << iLine))
                {
                        /* Loop over hardware configuration bits */
                        for ( ; iiLine < iLine ; ++iiLine)
                        {
                                if ((hwActiveLine & (1 << (iiLine - hwBase))))
                                        ++jjLine;
                        }
                        lineIndex = jjLine;
                        ulRegAddr =
                                BCHP_GSE_ANCIL_0_DATA_LINE1_BANK0 + ulCoreOffset +
                                (20 * bankIndex)                          +
                                ( 4 * lineIndex);
                        BREG_Write32 (hReg, ulRegAddr, pGSData->ulData[jLine]);
                        ++jLine;
                }
        }

        /* Program the write pointer into hardware */
        ++ulWritePointer;
        ulRegVal = BCHP_FIELD_DATA (GSE_ANCIL_0_WRPTR, VALUE, ulWritePointer);
        BREG_Write32 (hReg, BCHP_GSE_ANCIL_0_WRPTR + ulCoreOffset, ulRegVal);

        /* Debug code
        ulRegVal = BREG_Read32 (hReg, BCHP_GSE_ANCIL_0_WRPTR + ulCoreOffset);
        ulWritePointer = BCHP_GET_FIELD_DATA (ulRegVal, GSE_ANCIL_0_WRPTR, VALUE);
        dwrite_pointer[1] = ulWritePointer;
        ulRegVal = BREG_Read32 (hReg, BCHP_GSE_ANCIL_0_RDPTR + ulCoreOffset);
        dread_pointer[1] = BCHP_GET_FIELD_DATA (ulRegVal, GSE_ANCIL_0_RDPTR, VALUE);
        printf (
                "Field %c:  status: %03x  R/W (%d/%d) --> (%d/%d)  data (%08x %08x %08x)\n",
                ((polarity == BAVC_Polarity_eTopField) ? 'T' : 'B'),
                status,
                dread_pointer[0], dwrite_pointer[0],
                dread_pointer[1], dwrite_pointer[1],
                pGSData->ulData[0],  pGSData->ulData[1],  pGSData->ulData[2]);
        */

        BDBG_LEAVE(BVBI_P_GS_Encode_656_Data_isr);
        return 0x0;
}
#endif /** } BVBI_NUM_GSE **/

#if !B_REFSW_MINIMAL
#if (BVBI_NUM_GSE >= 1) /** { **/
/***************************************************************************
 *
 */
BERR_Code BVBI_P_GS_Encode_Enable_isr (
        BREG_Handle hReg,
        uint8_t hwCoreIndex,
        BFMT_VideoFmt eVideoFormat,
        bool bEnable)
{

        uint32_t ulCoreOffset;
        uint32_t ulGse_controlReg;

        BSTD_UNUSED (eVideoFormat);

        BDBG_ENTER(BVBI_P_GS_Encode_Enable_isr);

        /* Figure out which encoder core to use */
        ulCoreOffset = P_GetCoreOffset_isr (hwCoreIndex);
        if (ulCoreOffset == 0xFFFFFFFF)
        {
                /* This should never happen!  This parameter was checked by
                   BVBI_Encode_Create() */
                BDBG_LEAVE(BVBI_P_GS_Encode_Enable_isr);
                return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        ulGse_controlReg = BREG_Read32 (hReg, BCHP_GSE_0_CONTROL + ulCoreOffset);
        ulGse_controlReg &= ~(
                BCHP_MASK       (GSE_0_CONTROL, ENABLE                   ) );
        if (bEnable)
        {
                ulGse_controlReg |= (
                        BCHP_FIELD_DATA (GSE_0_CONTROL, ENABLE,                 1) );
        }
        else
        {
                ulGse_controlReg |= (
                        BCHP_FIELD_DATA (GSE_0_CONTROL, ENABLE,                 0) );
        }
        BREG_Write32 (hReg, BCHP_GSE_0_CONTROL + ulCoreOffset, ulGse_controlReg);

        BDBG_LEAVE(BVBI_P_GS_Encode_Enable_isr);
        return BERR_SUCCESS;
}
#endif /** } BVBI_NUM_GSE **/
#endif

#if !B_REFSW_MINIMAL
#if (BVBI_NUM_GSE_656 >= 1) /** { **/
/***************************************************************************
 *
 */
BERR_Code BVBI_P_GS_Encode_656_Enable_isr (
        BREG_Handle hReg,
        uint8_t hwCoreIndex,
        BFMT_VideoFmt eVideoFormat,
        bool bEnable)
{

        uint32_t ulCoreOffset;
        uint32_t ulGse_controlReg;

        BSTD_UNUSED (eVideoFormat);

        BDBG_ENTER(BVBI_P_GS_Encode_656_Enable_isr);

        /* Figure out which encoder core to use */
        ulCoreOffset = P_GetCoreOffset_656_isr (hwCoreIndex);
        if (ulCoreOffset == 0xFFFFFFFF)
        {
                /* This should never happen!  This parameter was checked by
                   BVBI_Encode_Create() */
                BDBG_LEAVE(BVBI_P_GS_Encode_656_Enable_isr);
                return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        ulGse_controlReg = BREG_Read32 (hReg, BCHP_GSE_ANCIL_0_CONTROL + ulCoreOffset);
        ulGse_controlReg &= ~(
                BCHP_MASK       (GSE_ANCIL_0_CONTROL, ENABLE                   ) );
        if (bEnable)
        {
                ulGse_controlReg |= (
                        BCHP_FIELD_DATA (GSE_ANCIL_0_CONTROL, ENABLE,                 1) );
        }
        else
        {
                ulGse_controlReg |= (
                        BCHP_FIELD_DATA (GSE_ANCIL_0_CONTROL, ENABLE,                 0) );
        }
        BREG_Write32 (hReg, BCHP_GSE_ANCIL_0_CONTROL + ulCoreOffset, ulGse_controlReg);

        BDBG_LEAVE(BVBI_P_GS_Encode_656_Enable_isr);
        return BERR_SUCCESS;
}
#endif /** } BVBI_NUM_GSE_656 **/
#endif


/***************************************************************************
* Static (private) functions
***************************************************************************/

#if (BVBI_NUM_GSE >= 1) /** { **/

static void BVBI_P_ProgramNull_isr (
        BREG_Handle hReg, uint32_t coreOffset,
        uint32_t ulWritePointer, uint32_t value)
{
        uint32_t ulRegAddr = BCHP_GSE_0_NULL + coreOffset;
        uint32_t ulRegVal  = BREG_Read32 (hReg, ulRegAddr);

        switch (ulWritePointer & 0x3)
        {
        case 0:
                ulRegVal &= ~BCHP_MASK       (GSE_0_NULL, NULL_ENABLE_BANK0       );
                ulRegVal |=  BCHP_FIELD_DATA (GSE_0_NULL, NULL_ENABLE_BANK0, value);
                break;
        case 1:
                ulRegVal &= ~BCHP_MASK       (GSE_0_NULL, NULL_ENABLE_BANK1       );
                ulRegVal |=  BCHP_FIELD_DATA (GSE_0_NULL, NULL_ENABLE_BANK1, value);
                break;
        case 2:
                ulRegVal &= ~BCHP_MASK       (GSE_0_NULL, NULL_ENABLE_BANK2       );
                ulRegVal |=  BCHP_FIELD_DATA (GSE_0_NULL, NULL_ENABLE_BANK2, value);
                break;
        case 3:
                ulRegVal &= ~BCHP_MASK       (GSE_0_NULL, NULL_ENABLE_BANK3       );
                ulRegVal |=  BCHP_FIELD_DATA (GSE_0_NULL, NULL_ENABLE_BANK3, value);
                break;
        /* coverity[dead_error_begin: FALSE] */
        default:
                /* Should never happen! Programming error! */
                BDBG_ASSERT (false);
                break;
        }

        BREG_Write32 (hReg, ulRegAddr, ulRegVal);
}

/***************************************************************************
 *
 */
static uint32_t P_GetCoreOffset_isr (uint8_t hwCoreIndex)
{
        uint32_t ulCoreOffset = 0xFFFFFFFF;

        switch (hwCoreIndex)
        {
#if (BVBI_NUM_GSE >= 1)
        case 0:
                ulCoreOffset = 0;
                break;
#endif
#if (BVBI_NUM_GSE >= 2)
        case 1:
                ulCoreOffset = (BCHP_GSE_1_REG_START - BCHP_GSE_0_REG_START);
                break;
#endif
#if (BVBI_NUM_GSE >= 3)
        case 2:
                ulCoreOffset = (BCHP_GSE_2_REG_START - BCHP_GSE_0_REG_START);
                break;
#endif
        default:
                break;
        }

        return ulCoreOffset;
}

#endif /** } BVBI_NUM_GSE **/

#if (BVBI_NUM_GSE_656 >= 1) /** { **/

static void BVBI_P_ProgramNull_656_isr (
        BREG_Handle hReg, uint32_t coreOffset,
        uint32_t ulWritePointer, uint32_t value)
{
        uint32_t ulRegAddr = BCHP_GSE_ANCIL_0_NULL + coreOffset;
        uint32_t ulRegVal  = BREG_Read32 (hReg, ulRegAddr);

        switch (ulWritePointer & 0x3)
        {
        case 0:
                ulRegVal &= ~BCHP_MASK       (GSE_ANCIL_0_NULL, NULL_ENABLE_BANK0       );
                ulRegVal |=  BCHP_FIELD_DATA (GSE_ANCIL_0_NULL, NULL_ENABLE_BANK0, value);
                break;
        case 1:
                ulRegVal &= ~BCHP_MASK       (GSE_ANCIL_0_NULL, NULL_ENABLE_BANK1       );
                ulRegVal |=  BCHP_FIELD_DATA (GSE_ANCIL_0_NULL, NULL_ENABLE_BANK1, value);
                break;
        case 2:
                ulRegVal &= ~BCHP_MASK       (GSE_ANCIL_0_NULL, NULL_ENABLE_BANK2       );
                ulRegVal |=  BCHP_FIELD_DATA (GSE_ANCIL_0_NULL, NULL_ENABLE_BANK2, value);
                break;
        case 3:
                ulRegVal &= ~BCHP_MASK       (GSE_ANCIL_0_NULL, NULL_ENABLE_BANK3       );
                ulRegVal |=  BCHP_FIELD_DATA (GSE_ANCIL_0_NULL, NULL_ENABLE_BANK3, value);
                break;
        /* coverity[dead_error_begin: FALSE] */
        default:
                /* Should never happen! Programming error! */
                BDBG_ASSERT (false);
                break;
        }

        BREG_Write32 (hReg, ulRegAddr, ulRegVal);
}

/***************************************************************************
 *
 */
static uint32_t P_GetCoreOffset_656_isr (uint8_t hwCoreIndex)
{
        uint32_t ulCoreOffset = 0xFFFFFFFF;

        switch (hwCoreIndex)
        {
#if (BVBI_NUM_GSE_656 >= 1)
        case 0:
                ulCoreOffset = 0;
                break;
#endif
#if (BVBI_NUM_GSE_656 >= 2)
        case 1:
                ulCoreOffset =
                    (BCHP_GSE_ANCIL_1_REG_START - BCHP_GSE_ANCIL_0_REG_START);
                break;
#endif
#if (BVBI_NUM_GSE_656 >= 3)
        case 2:
                ulCoreOffset =
                    (BCHP_GSE_ANCIL_2_REG_START - BCHP_GSE_ANCIL_0_REG_START);
                break;
#endif
        default:
                break;
        }

        return ulCoreOffset;
}

#endif /** } BVBI_NUM_GSE_656 **/
