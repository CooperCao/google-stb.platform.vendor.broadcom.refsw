/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#if (BVBI_NUM_AMOLE >= 1)
#include "bchp_amole_0.h"  /* RDB info for primary AMOL encoder core */
#endif
#if (BVBI_NUM_AMOLE_656 >= 1)
#include "bchp_amole_ancil_0.h"
#endif

BDBG_MODULE(BVBI);

/***************************************************************************
* Private data
***************************************************************************/

/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/

#if (BVBI_NUM_AMOLE >= 1)
static void BVBI_P_ProgramNull_isr (
    BREG_Handle hReg, uint32_t coreOffset,
    uint32_t ulWritePointer, uint32_t value);
static uint32_t P_GetCoreOffset_isrsafe (uint8_t hwCoreIndex);
#endif /** } BVBI_NUM_AMOLE **/
#if (BVBI_NUM_AMOLE_656 >= 1)
static void BVBI_P_ProgramNull_656_isr (
    BREG_Handle hReg, uint32_t coreOffset,
    uint32_t ulWritePointer, uint32_t value);
static uint32_t P_GetCoreOffset_656_isrsafe (uint8_t hwCoreIndex);
#endif /** } BVBI_NUM_AMOLE_656 **/


/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/


/***************************************************************************
* Implementation of supporting AMOL functions that are not in API
***************************************************************************/

#if (BVBI_NUM_AMOLE > 0) /** { **/
/***************************************************************************
 *
 */
void BVBI_P_AMOL_Enc_Init (BREG_Handle hReg, uint8_t hwCoreIndex)
{
    BDBG_ENTER(BVBI_P_AMOL_Enc_Init);

    BVBI_P_VIE_SoftReset_isrsafe (hReg, false, hwCoreIndex, BVBI_P_SELECT_AMOL);

    BDBG_LEAVE(BVBI_P_AMOL_Enc_Init);

}
#endif /** } BVBI_NUM_AMOLE **/

#if (BVBI_NUM_AMOLE_656 > 0) /** { **/
/***************************************************************************
 *
 */
void BVBI_P_AMOL_Enc_656_Init (BREG_Handle hReg, uint8_t hwCoreIndex)
{
    BDBG_ENTER(BVBI_P_AMOL_Enc_656_Init);

    BKNI_EnterCriticalSection();
    BVBI_P_VIE_SoftReset_isrsafe (hReg, true, hwCoreIndex, BVBI_P_SELECT_AMOL);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BVBI_P_AMOL_Enc_656_Init);
}
#endif /** } BVBI_NUM_AMOLE_656 **/

#if (BVBI_NUM_AMOLE > 0) /** { **/
/***************************************************************************
 *
 */
BERR_Code BVBI_P_AMOL_Enc_Program (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    bool bActive,
    BFMT_VideoFmt eVideoFormat,
    bool bArib480p,
    BVBI_AMOL_Type amolType)
{
/*
    Programming note: the implementation here assumes that the bitfield layout
    within registers is the same for all AMOL encoder cores in the chip.

    If a chip is built that has multiple AMOL encoder cores that are not
    identical, then this routine will have to be redesigned.
*/
    uint16_t lineMask;
    uint32_t ulCoreOffset;
    uint32_t ulAmole_controlReg;
    uint32_t ulLineAdj;
    uint32_t nbyte_symbol = (amolType == BVBI_AMOL_Type_II_Highrate) ? 1 : 0;

    BDBG_ENTER(BVBI_P_AMOL_Enc_Program);

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_isrsafe (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This is difficult to do right. Just say bye. */
        return BERR_SUCCESS;
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
            BDBG_ERR(("BVBI_AMOLE: video format %d not supported",
                eVideoFormat));
            return BERR_TRACE (BVBI_ERR_VFMT_CONFLICT);
        }
    }

    ulLineAdj = (bArib480p ? 1 : 0);

    /* All three AMOL types use lines 20-top and 22-bottom */
    ulAmole_controlReg = 0;
    lineMask = (unsigned int)1 << (20 - 15);
    ulAmole_controlReg |= (
        BCHP_FIELD_DATA (AMOLE_0_ACT_LINE_BASE_TOP,        BASE,
            15 - ulLineAdj) |
        BCHP_FIELD_DATA (AMOLE_0_ACT_LINE_BASE_TOP, ACTIVE_LINE, lineMask) );
    if (amolType != BVBI_AMOL_Type_I)
    {
    lineMask = (unsigned int)1 << (22 - 15);
    ulAmole_controlReg |= (
        BCHP_FIELD_DATA (AMOLE_0_ACT_LINE_BASE_TOP, ACTIVE_LINE, lineMask) );
    }
    BREG_Write32 (
        hReg, BCHP_AMOLE_0_ACT_LINE_BASE_TOP + ulCoreOffset,
        ulAmole_controlReg);

    ulAmole_controlReg = 0;
    lineMask = (unsigned int)1 << (22 - 15);
    ulAmole_controlReg |= (
        BCHP_FIELD_DATA (AMOLE_0_ACT_LINE_BASE_BOT,        BASE,
            22 - ulLineAdj) |
        BCHP_FIELD_DATA (AMOLE_0_ACT_LINE_BASE_BOT, ACTIVE_LINE, lineMask) );
    if (amolType != BVBI_AMOL_Type_I)
    {
    lineMask = (unsigned int)1 << (20 - 15);
    ulAmole_controlReg |= (
        BCHP_FIELD_DATA (AMOLE_0_ACT_LINE_BASE_BOT, ACTIVE_LINE, lineMask) );
    }
    BREG_Write32 (
        hReg, BCHP_AMOLE_0_ACT_LINE_BASE_BOT + ulCoreOffset,
        ulAmole_controlReg);

    ulAmole_controlReg = 0;
    ulAmole_controlReg |= (
        BCHP_FIELD_DATA (AMOLE_0_GAIN_PED_TOP, GAIN_LINE22,    0x4d) |
        BCHP_FIELD_DATA (AMOLE_0_GAIN_PED_TOP, GAIN_LINE20,    0x4d) |
        BCHP_FIELD_ENUM (AMOLE_0_GAIN_PED_TOP,  PED_LINE22, DISABLE) |
        BCHP_FIELD_ENUM (AMOLE_0_GAIN_PED_TOP,  PED_LINE20, DISABLE)
    );
    BREG_Write32 (
        hReg, BCHP_AMOLE_0_GAIN_PED_TOP + ulCoreOffset, ulAmole_controlReg);

    ulAmole_controlReg = 0;
    ulAmole_controlReg |= (
        BCHP_FIELD_DATA (AMOLE_0_GAIN_PED_BOT, GAIN_LINE22,    0x4d) |
        BCHP_FIELD_DATA (AMOLE_0_GAIN_PED_BOT, GAIN_LINE20,    0x4d) |
        BCHP_FIELD_ENUM (AMOLE_0_GAIN_PED_BOT,  PED_LINE22, DISABLE) |
        BCHP_FIELD_ENUM (AMOLE_0_GAIN_PED_BOT,  PED_LINE20, DISABLE)
    );
    BREG_Write32 (
        hReg, BCHP_AMOLE_0_GAIN_PED_BOT + ulCoreOffset, ulAmole_controlReg);

    ulAmole_controlReg = 0;
    ulAmole_controlReg |= (
        BCHP_FIELD_DATA (
            AMOLE_0_BYTES_PER_LINE, LINE22_BANK3, nbyte_symbol) |
        BCHP_FIELD_DATA (
            AMOLE_0_BYTES_PER_LINE, LINE20_BANK3, nbyte_symbol) |
        BCHP_FIELD_DATA (
            AMOLE_0_BYTES_PER_LINE, LINE22_BANK2, nbyte_symbol) |
        BCHP_FIELD_DATA (
            AMOLE_0_BYTES_PER_LINE, LINE20_BANK2, nbyte_symbol) |
        BCHP_FIELD_DATA (
            AMOLE_0_BYTES_PER_LINE, LINE22_BANK1, nbyte_symbol) |
        BCHP_FIELD_DATA (
            AMOLE_0_BYTES_PER_LINE, LINE20_BANK1, nbyte_symbol) |
        BCHP_FIELD_DATA (
            AMOLE_0_BYTES_PER_LINE, LINE22_BANK0, nbyte_symbol) |
        BCHP_FIELD_DATA (
            AMOLE_0_BYTES_PER_LINE, LINE20_BANK0, nbyte_symbol)
    );
    BREG_Write32 (
        hReg, BCHP_AMOLE_0_BYTES_PER_LINE + ulCoreOffset,
        ulAmole_controlReg);

    ulAmole_controlReg = 0;
    ulAmole_controlReg |= (
        BCHP_FIELD_DATA (AMOLE_0_NULL, NULL_ENABLE_BANK3,    0) |
        BCHP_FIELD_DATA (AMOLE_0_NULL, NULL_ENABLE_BANK2,    0) |
        BCHP_FIELD_DATA (AMOLE_0_NULL, NULL_ENABLE_BANK1,    0) |
        BCHP_FIELD_DATA (AMOLE_0_NULL, NULL_ENABLE_BANK0,    0) |
        BCHP_FIELD_DATA (AMOLE_0_NULL, CHARACTER,         0x20)
    );
    BREG_Write32 (
        hReg, BCHP_AMOLE_0_NULL + ulCoreOffset, ulAmole_controlReg);

    ulAmole_controlReg = 0;
    ulAmole_controlReg |= (
        BCHP_FIELD_DATA (AMOLE_0_CONTROL, DELAY_COUNT, 0x6B) |
        BCHP_FIELD_DATA (AMOLE_0_CONTROL, FIFO_FREEZE,    0) |
        BCHP_FIELD_DATA (AMOLE_0_CONTROL, NULL_MODE,      1) );
    if (bActive)
    {
        ulAmole_controlReg |=
            BCHP_FIELD_ENUM (AMOLE_0_CONTROL, ENABLE, ON);
    }
    else
    {
        ulAmole_controlReg |=
            BCHP_FIELD_ENUM (AMOLE_0_CONTROL, ENABLE, OFF);
    }
    BREG_Write32 (
        hReg, BCHP_AMOLE_0_CONTROL + ulCoreOffset, ulAmole_controlReg);

    BDBG_LEAVE(BVBI_P_AMOL_Enc_Program);
    return BERR_SUCCESS;
}
#endif /** } BVBI_NUM_AMOLE **/

#if (BVBI_NUM_AMOLE_656 > 0) /** { **/
/***************************************************************************
 *
 */
BERR_Code BVBI_P_AMOL_Enc_656_Program (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    bool bActive,
    BFMT_VideoFmt eVideoFormat,
    bool bArib480p,
    BVBI_AMOL_Type amolType)
{
/*
    Programming note: the implementation here assumes that the bitfield layout
    within registers is the same for all AMOL encoder cores in the chip.

    If a chip is built that has multiple AMOL encoder cores that are not
    identical, then this routine will have to be redesigned.
*/
    uint16_t lineMask;
    uint32_t ulCoreOffset;
    uint32_t ulAmole_controlReg;
    uint32_t ulLineAdj;
    uint32_t nbyte_symbol = (amolType == BVBI_AMOL_Type_II_Highrate) ? 1 : 0;

    BDBG_ENTER(BVBI_P_AMOL_Enc_656_Program);

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_656_isrsafe (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This is difficult to do right. Just say bye. */
        return BERR_SUCCESS;
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
            BDBG_ERR(("BVBI_AMOLE: video format %d not supported",
                eVideoFormat));
            return BERR_TRACE (BVBI_ERR_VFMT_CONFLICT);
        }
    }

    ulLineAdj = (bArib480p ? 1 : 0);

    /* All three AMOL types use lines 20-top and 22-bottom */
    ulAmole_controlReg = 0;
    lineMask = (unsigned int)1 << (20 - 15);
    ulAmole_controlReg |= (
        BCHP_FIELD_DATA (AMOLE_ANCIL_0_ACT_LINE_BASE_TOP,        BASE,
            15 - ulLineAdj) |
        BCHP_FIELD_DATA (AMOLE_ANCIL_0_ACT_LINE_BASE_TOP, ACTIVE_LINE, lineMask) );
    if (amolType != BVBI_AMOL_Type_I)
    {
    lineMask = (unsigned int)1 << (22 - 15);
    ulAmole_controlReg |= (
        BCHP_FIELD_DATA (AMOLE_ANCIL_0_ACT_LINE_BASE_TOP, ACTIVE_LINE, lineMask) );
    }
    BREG_Write32 (
        hReg, BCHP_AMOLE_ANCIL_0_ACT_LINE_BASE_TOP + ulCoreOffset,
        ulAmole_controlReg);

    ulAmole_controlReg = 0;
    lineMask = (unsigned int)1 << (22 - 15);
    ulAmole_controlReg |= (
        BCHP_FIELD_DATA (AMOLE_ANCIL_0_ACT_LINE_BASE_BOT,        BASE,
            22 - ulLineAdj) |
        BCHP_FIELD_DATA (AMOLE_ANCIL_0_ACT_LINE_BASE_BOT, ACTIVE_LINE, lineMask) );
    if (amolType != BVBI_AMOL_Type_I)
    {
    lineMask = (unsigned int)1 << (20 - 15);
    ulAmole_controlReg |= (
        BCHP_FIELD_DATA (AMOLE_ANCIL_0_ACT_LINE_BASE_BOT, ACTIVE_LINE, lineMask) );
    }
    BREG_Write32 (
        hReg, BCHP_AMOLE_ANCIL_0_ACT_LINE_BASE_BOT + ulCoreOffset,
        ulAmole_controlReg);

    ulAmole_controlReg = 0;
    ulAmole_controlReg |= (
        BCHP_FIELD_DATA (
            AMOLE_ANCIL_0_BYTES_PER_LINE, LINE22_BANK3, nbyte_symbol) |
        BCHP_FIELD_DATA (
            AMOLE_ANCIL_0_BYTES_PER_LINE, LINE20_BANK3, nbyte_symbol) |
        BCHP_FIELD_DATA (
            AMOLE_ANCIL_0_BYTES_PER_LINE, LINE22_BANK2, nbyte_symbol) |
        BCHP_FIELD_DATA (
            AMOLE_ANCIL_0_BYTES_PER_LINE, LINE20_BANK2, nbyte_symbol) |
        BCHP_FIELD_DATA (
            AMOLE_ANCIL_0_BYTES_PER_LINE, LINE22_BANK1, nbyte_symbol) |
        BCHP_FIELD_DATA (
            AMOLE_ANCIL_0_BYTES_PER_LINE, LINE20_BANK1, nbyte_symbol) |
        BCHP_FIELD_DATA (
            AMOLE_ANCIL_0_BYTES_PER_LINE, LINE22_BANK0, nbyte_symbol) |
        BCHP_FIELD_DATA (
            AMOLE_ANCIL_0_BYTES_PER_LINE, LINE20_BANK0, nbyte_symbol)
    );
    BREG_Write32 (
        hReg, BCHP_AMOLE_ANCIL_0_BYTES_PER_LINE + ulCoreOffset,
        ulAmole_controlReg);

    ulAmole_controlReg = 0;
    ulAmole_controlReg |= (
        BCHP_FIELD_DATA (AMOLE_ANCIL_0_NULL, NULL_ENABLE_BANK3,    0) |
        BCHP_FIELD_DATA (AMOLE_ANCIL_0_NULL, NULL_ENABLE_BANK2,    0) |
        BCHP_FIELD_DATA (AMOLE_ANCIL_0_NULL, NULL_ENABLE_BANK1,    0) |
        BCHP_FIELD_DATA (AMOLE_ANCIL_0_NULL, NULL_ENABLE_BANK0,    0) |
        BCHP_FIELD_DATA (AMOLE_ANCIL_0_NULL, CHARACTER,         0x20)
    );
    BREG_Write32 (
        hReg, BCHP_AMOLE_ANCIL_0_NULL + ulCoreOffset, ulAmole_controlReg);

    ulAmole_controlReg = 0;
    ulAmole_controlReg |= (
        BCHP_FIELD_DATA (AMOLE_ANCIL_0_CONTROL, FIFO_FREEZE,    0) |
        BCHP_FIELD_DATA (AMOLE_ANCIL_0_CONTROL, NULL_MODE,      1) );
    if (bActive)
    {
        ulAmole_controlReg |=
            BCHP_FIELD_ENUM (AMOLE_ANCIL_0_CONTROL, ENABLE, ON);
    }
    else
    {
        ulAmole_controlReg |=
            BCHP_FIELD_ENUM (AMOLE_ANCIL_0_CONTROL, ENABLE, OFF);
    }
    BREG_Write32 (
        hReg, BCHP_AMOLE_ANCIL_0_CONTROL + ulCoreOffset, ulAmole_controlReg);

    BDBG_LEAVE(BVBI_P_AMOL_Enc_656_Program);
    return BERR_SUCCESS;
}
#endif /** } BVBI_NUM_AMOLE_656 **/


#if (BVBI_NUM_AMOLE > 0) /** { **/
/***************************************************************************
 *
 */
uint32_t BVBI_P_AMOL_Encode_Data_isr (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    BAVC_Polarity polarity,
    BVBI_AMOL_Type amolType,
    uint8_t* pAMOLData )
{
    uint32_t ulCoreOffset;
    uint32_t ulRegVal;
    uint32_t ulRegAddr;
    uint32_t ulReadPointer;
    uint32_t ulWritePointer;
    uint32_t bankIndex;
    int      totalBytes;
    int      iReg;

    /* Debug code
    uint32_t dread_pointer[2];
    uint32_t dwrite_pointer[2];
    uint32_t status;
    */

    BDBG_ENTER(BVBI_P_AMOL_Encode_Data_isr);

    /* Size check for field data */
    if (!pAMOLData)
    {
        return (BVBI_LINE_ERROR_FLDH_CONFLICT);
    }

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_isrsafe (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        ulCoreOffset = 0;
        BDBG_ASSERT (ulCoreOffset);
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
        BDBG_ERR(("BVBI_AMOLE: video format %d not supported", eVideoFormat));
        BDBG_ASSERT (0);
        break;
    }

    /* Decide how many bytes to copy */
    switch (amolType)
    {
    case BVBI_AMOL_Type_I:
        totalBytes = 6;
        break;
    case BVBI_AMOL_Type_II_Lowrate:
        totalBytes = 12;
        break;
    case BVBI_AMOL_Type_II_Highrate:
        totalBytes = 24;
        break;
    default:
        totalBytes = 0;
        BDBG_ASSERT (totalBytes);
        break;
    }

    /* Clear status bits */
    ulRegVal = BREG_Read32 (hReg, BCHP_AMOLE_0_STATUS + ulCoreOffset);
    /* Debug code
    status = ulRegVal;
    */
    ulRegVal &= 0x000003FF;
    BREG_Write32 (hReg, BCHP_AMOLE_0_STATUS + ulCoreOffset, ulRegVal);
    /* TODO: Check AMOLE_0_STATUS register? */

    /* Get FIFO pointers */
    ulRegVal = BREG_Read32 (hReg, BCHP_AMOLE_0_WRPTR + ulCoreOffset);
    ulWritePointer = BCHP_GET_FIELD_DATA (ulRegVal, AMOLE_0_WRPTR, VALUE);
    bankIndex = ulWritePointer & 0x00000003;
    ulRegVal = BREG_Read32 (hReg, BCHP_AMOLE_0_RDPTR + ulCoreOffset);
    ulReadPointer = BCHP_GET_FIELD_DATA (ulRegVal, AMOLE_0_RDPTR, VALUE);

    /* Debug code
    dread_pointer[0]  = ulReadPointer;
    dwrite_pointer[0] = ulWritePointer;
    */

    /* Check for FIFO full */
    if (((ulReadPointer & 0x3) == bankIndex     ) &&
        (ulReadPointer         != ulWritePointer)    )
    {
        /* Debug code
        printf ("\n   ***   AMOL FIFO full!!!   ***\n\n");
        */
        return BVBI_LINE_ERROR_AMOL_OVERRUN;
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

    /* Decide where to put user's data */
    switch (bankIndex)
    {
    case 0:
        ulRegAddr = BCHP_AMOLE_0_DATA_WORD0_BANK0 + ulCoreOffset;
        break;
    case 1:
        ulRegAddr = BCHP_AMOLE_0_DATA_WORD0_BANK1 + ulCoreOffset;
        break;
    case 2:
        ulRegAddr = BCHP_AMOLE_0_DATA_WORD0_BANK2 + ulCoreOffset;
        break;
    case 3:
        ulRegAddr = BCHP_AMOLE_0_DATA_WORD0_BANK3 + ulCoreOffset;
        break;
    /* coverity[dead_error_begin: FALSE] */
    default:
        ulRegAddr = 0x0;
        BDBG_ASSERT (ulRegAddr);
    }

    /* Now write the user's data */
    /* Programming note: the bit ordering is somewhat complicated. See bvbi.h
     * ----------------  for details. */
    switch (totalBytes)
    {
    case 6:
        ulRegVal = 0;
        ulRegVal |= (uint32_t)(*pAMOLData++)      ;
        ulRegVal |= (uint32_t)(*pAMOLData++) <<  8;
        ulRegVal |= (uint32_t)(*pAMOLData++) << 16;
        ulRegVal |= (uint32_t)(*pAMOLData++) << 24;
        BREG_Write32 (hReg, ulRegAddr, ulRegVal);
        ulRegAddr += 4;
        ulRegVal = 0;
        ulRegVal |= (uint32_t)(*pAMOLData++)      ;
        ulRegVal |= (uint32_t)(*pAMOLData++) <<  8;
        BREG_Write32 (hReg, ulRegAddr, ulRegVal);
        break;
    case 12:
        for (iReg = 0 ; iReg < 3 ; ++iReg)
        {
            ulRegVal = 0;
            ulRegVal |= (uint32_t)(*pAMOLData++)      ;
            ulRegVal |= (uint32_t)(*pAMOLData++) <<  8;
            ulRegVal |= (uint32_t)(*pAMOLData++) << 16;
            ulRegVal |= (uint32_t)(*pAMOLData++) << 24;
            BREG_Write32 (hReg, ulRegAddr, ulRegVal);
            ulRegAddr += 4;
        }
        break;
    case 24:
        for (iReg = 0 ; iReg < 6 ; ++iReg)
        {
            ulRegVal = 0;
            ulRegVal |= (uint32_t)(*pAMOLData++)      ;
            ulRegVal |= (uint32_t)(*pAMOLData++) <<  8;
            ulRegVal |= (uint32_t)(*pAMOLData++) << 16;
            ulRegVal |= (uint32_t)(*pAMOLData++) << 24;
            BREG_Write32 (hReg, ulRegAddr, ulRegVal);
            ulRegAddr += 4;
        }
        break;
    default:
        /* Should not happen */
        totalBytes = 0;
        BDBG_ASSERT (totalBytes);
        break;
    }

    /* Program the write pointer into hardware */
    ++ulWritePointer;
    ulRegVal = BCHP_FIELD_DATA (AMOLE_0_WRPTR, VALUE, ulWritePointer);
    BREG_Write32 (hReg, BCHP_AMOLE_0_WRPTR + ulCoreOffset, ulRegVal);

    /* Debug code
    ulRegVal = BREG_Read32 (hReg, BCHP_AMOLE_0_WRPTR + ulCoreOffset);
    ulWritePointer = BCHP_GET_FIELD_DATA (ulRegVal, AMOLE_0_WRPTR, VALUE);
    dwrite_pointer[1] = ulWritePointer;
    ulRegVal = BREG_Read32 (hReg, BCHP_AMOLE_0_RDPTR + ulCoreOffset);
    dread_pointer[1] = BCHP_GET_FIELD_DATA (ulRegVal, AMOLE_0_RDPTR, VALUE);
    printf (
        "Field %c:  status: %03x  R/W (%d/%d) --> (%d/%d)  data (%08x %08x %08x)\n",
        ((polarity == BAVC_Polarity_eTopField) ? 'T' : 'B'),
        status,
        dread_pointer[0], dwrite_pointer[0],
        dread_pointer[1], dwrite_pointer[1],
        pAMOLata->ulData[0],  pAMOLata->ulData[1],  pAMOLata->ulData[2]);
    */

    BDBG_LEAVE(BVBI_P_AMOL_Encode_Data_isr);
    return 0x0;
}
#endif /** } BVBI_NUM_AMOLE **/


#if (BVBI_NUM_AMOLE_656 > 0) /** { **/
/***************************************************************************
 *
 */
uint32_t BVBI_P_AMOL_Encode_656_Data_isr (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    BAVC_Polarity polarity,
    BVBI_AMOL_Type amolType,
    uint8_t* pAMOLData )
{
    uint32_t ulCoreOffset;
    uint32_t ulRegVal;
    uint32_t ulRegAddr;
    uint32_t ulReadPointer;
    uint32_t ulWritePointer;
    uint32_t bankIndex;
    int      totalBytes;
    int      iReg;

    /* Debug code
    uint32_t dread_pointer[2];
    uint32_t dwrite_pointer[2];
    uint32_t status;
    */

    BDBG_ENTER(BVBI_P_AMOL_Encode_656_Data_isr);

    /* Size check for field data */
    if (!pAMOLData)
    {
        return (BVBI_LINE_ERROR_FLDH_CONFLICT);
    }

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_656_isrsafe (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        ulCoreOffset = 0;
        BDBG_ASSERT (ulCoreOffset);
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
        BDBG_ERR(("BVBI_AMOLE: video format %d not supported", eVideoFormat));
        BDBG_ASSERT (0);
        break;
    }

    /* Decide how many bytes to copy */
    switch (amolType)
    {
    case BVBI_AMOL_Type_I:
        totalBytes = 6;
        break;
    case BVBI_AMOL_Type_II_Lowrate:
        totalBytes = 12;
        break;
    case BVBI_AMOL_Type_II_Highrate:
        totalBytes = 24;
        break;
    default:
        totalBytes = 0;
        BDBG_ASSERT (totalBytes);
        break;
    }

    /* Clear status bits */
    ulRegVal = BREG_Read32 (hReg, BCHP_AMOLE_ANCIL_0_STATUS + ulCoreOffset);
    /* Debug code
    status = ulRegVal;
    */
    ulRegVal &= 0x000003FF;
    BREG_Write32 (hReg, BCHP_AMOLE_ANCIL_0_STATUS + ulCoreOffset, ulRegVal);
    /* TODO: Check AMOLE_ANCIL_0_STATUS register? */

    /* Get FIFO pointers */
    ulRegVal = BREG_Read32 (hReg, BCHP_AMOLE_ANCIL_0_WRPTR + ulCoreOffset);
    ulWritePointer = BCHP_GET_FIELD_DATA (ulRegVal, AMOLE_ANCIL_0_WRPTR, VALUE);
    bankIndex = ulWritePointer & 0x00000003;
    ulRegVal = BREG_Read32 (hReg, BCHP_AMOLE_ANCIL_0_RDPTR + ulCoreOffset);
    ulReadPointer = BCHP_GET_FIELD_DATA (ulRegVal, AMOLE_ANCIL_0_RDPTR, VALUE);

    /* Debug code
    dread_pointer[0]  = ulReadPointer;
    dwrite_pointer[0] = ulWritePointer;
    */

    /* Check for FIFO full */
    if (((ulReadPointer & 0x3) == bankIndex     ) &&
        (ulReadPointer         != ulWritePointer)    )
    {
        /* Debug code
        printf ("\n   ***   AMOL FIFO full!!!   ***\n\n");
        */
        return BVBI_LINE_ERROR_AMOL_OVERRUN;
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

    /* Decide where to put user's data */
    switch (bankIndex)
    {
    case 0:
        ulRegAddr = BCHP_AMOLE_ANCIL_0_DATA_WORD0_BANK0 + ulCoreOffset;
        break;
    case 1:
        ulRegAddr = BCHP_AMOLE_ANCIL_0_DATA_WORD0_BANK1 + ulCoreOffset;
        break;
    case 2:
        ulRegAddr = BCHP_AMOLE_ANCIL_0_DATA_WORD0_BANK2 + ulCoreOffset;
        break;
    case 3:
        ulRegAddr = BCHP_AMOLE_ANCIL_0_DATA_WORD0_BANK3 + ulCoreOffset;
        break;
    /* PROGRAMMING NOTE: Coverity is flagging this as an error, even with the
     * ----------------  following  conventional comment. I always like to
     *                   have a default: clause, but too bad.
     */
    /* coverity[dead_error_begin: FALSE] */
    /*
    default:
        ulRegAddr = 0x0;
        BDBG_ASSERT (ulRegAddr);
        break;
    */
    }

    /* Now write the user's data */
    /* Programming note: the bit ordering is somewhat complicated. See bvbi.h
     * ----------------  for details. */
    switch (totalBytes)
    {
    case 6:
        ulRegVal = 0;
        ulRegVal |= (uint32_t)(*pAMOLData++)      ;
        ulRegVal |= (uint32_t)(*pAMOLData++) <<  8;
        ulRegVal |= (uint32_t)(*pAMOLData++) << 16;
        ulRegVal |= (uint32_t)(*pAMOLData++) << 24;
        BREG_Write32 (hReg, ulRegAddr, ulRegVal);
        ulRegAddr += 4;
        ulRegVal = 0;
        ulRegVal |= (uint32_t)(*pAMOLData++)      ;
        ulRegVal |= (uint32_t)(*pAMOLData++) <<  8;
        BREG_Write32 (hReg, ulRegAddr, ulRegVal);
        break;
    case 12:
        for (iReg = 0 ; iReg < 3 ; ++iReg)
        {
            ulRegVal = 0;
            ulRegVal |= (uint32_t)(*pAMOLData++)      ;
            ulRegVal |= (uint32_t)(*pAMOLData++) <<  8;
            ulRegVal |= (uint32_t)(*pAMOLData++) << 16;
            ulRegVal |= (uint32_t)(*pAMOLData++) << 24;
            BREG_Write32 (hReg, ulRegAddr, ulRegVal);
            ulRegAddr += 4;
        }
        break;
    case 24:
        for (iReg = 0 ; iReg < 6 ; ++iReg)
        {
            ulRegVal = 0;
            ulRegVal |= (uint32_t)(*pAMOLData++)      ;
            ulRegVal |= (uint32_t)(*pAMOLData++) <<  8;
            ulRegVal |= (uint32_t)(*pAMOLData++) << 16;
            ulRegVal |= (uint32_t)(*pAMOLData++) << 24;
            BREG_Write32 (hReg, ulRegAddr, ulRegVal);
            ulRegAddr += 4;
        }
        break;
    default:
        /* Should not happen */
        totalBytes = 0;
        BDBG_ASSERT (totalBytes);
        break;
    }

    /* Program the write pointer into hardware */
    ++ulWritePointer;
    ulRegVal = BCHP_FIELD_DATA (AMOLE_ANCIL_0_WRPTR, VALUE, ulWritePointer);
    BREG_Write32 (hReg, BCHP_AMOLE_ANCIL_0_WRPTR + ulCoreOffset, ulRegVal);

    /* Debug code
    ulRegVal = BREG_Read32 (hReg, BCHP_AMOLE_ANCIL_0_WRPTR + ulCoreOffset);
    ulWritePointer = BCHP_GET_FIELD_DATA (ulRegVal, AMOLE_ANCIL_0_WRPTR, VALUE);
    dwrite_pointer[1] = ulWritePointer;
    ulRegVal = BREG_Read32 (hReg, BCHP_AMOLE_ANCIL_0_RDPTR + ulCoreOffset);
    dread_pointer[1] = BCHP_GET_FIELD_DATA (ulRegVal, AMOLE_ANCIL_0_RDPTR, VALUE);
    printf (
        "Field %c:  status: %03x  R/W (%d/%d) --> (%d/%d)  data (%08x %08x %08x)\n",
        ((polarity == BAVC_Polarity_eTopField) ? 'T' : 'B'),
        status,
        dread_pointer[0], dwrite_pointer[0],
        dread_pointer[1], dwrite_pointer[1],
        pAMOLata->ulData[0],  pAMOLata->ulData[1],  pAMOLata->ulData[2]);
    */

    BDBG_LEAVE(BVBI_P_AMOL_Encode_656_Data_isr);
    return 0x0;
}
#endif /** } BVBI_NUM_AMOLE_656 **/


#if !B_REFSW_MINIMAL
#if (BVBI_NUM_AMOLE > 0) /** { **/
/***************************************************************************
 *
 */
BERR_Code BVBI_P_AMOL_Encode_Enable_isr (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    bool bEnable)
{
    uint32_t ulCoreOffset;
    uint32_t ulAmole_controlReg;

    BSTD_UNUSED (eVideoFormat);

    BDBG_ENTER(BVBI_P_AMOL_Encode_Enable_isr);

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_isrsafe (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_AMOL_Encode_Enable_isr);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    ulAmole_controlReg =
        BREG_Read32 (hReg, BCHP_AMOLE_0_CONTROL + ulCoreOffset);
    ulAmole_controlReg &= ~(
        BCHP_MASK       (AMOLE_0_CONTROL, ENABLE                   ) );
    if (bEnable)
    {
        ulAmole_controlReg |= (
            BCHP_FIELD_DATA (AMOLE_0_CONTROL, ENABLE,                 1) );
    }
    else
    {
        ulAmole_controlReg |= (
            BCHP_FIELD_DATA (AMOLE_0_CONTROL, ENABLE,                 0) );
    }
    BREG_Write32 (
        hReg, BCHP_AMOLE_0_CONTROL + ulCoreOffset, ulAmole_controlReg);

    BDBG_LEAVE(BVBI_P_AMOL_Encode_Enable_isr);
    return BERR_SUCCESS;
}
#endif /** } BVBI_NUM_AMOLE **/
#endif

#if !B_REFSW_MINIMAL
#if (BVBI_NUM_AMOLE_656 > 0) /** { **/
/***************************************************************************
 *
 */
BERR_Code BVBI_P_AMOL_Encode_656_Enable_isr (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    bool bEnable)
{
    uint32_t ulCoreOffset;
    uint32_t ulAmole_controlReg;

    BSTD_UNUSED (eVideoFormat);

    BDBG_ENTER(BVBI_P_AMOL_Encode_656_Enable_isr);

    /* Figure out which encoder core to use */
    ulCoreOffset = P_GetCoreOffset_656_isrsafe (hwCoreIndex);
    if (ulCoreOffset == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_AMOL_Encode_Enable_isr);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    ulAmole_controlReg =
        BREG_Read32 (hReg, BCHP_AMOLE_ANCIL_0_CONTROL + ulCoreOffset);
    ulAmole_controlReg &= ~(
        BCHP_MASK       (AMOLE_ANCIL_0_CONTROL, ENABLE                   ) );
    if (bEnable)
    {
        ulAmole_controlReg |= (
            BCHP_FIELD_DATA (AMOLE_ANCIL_0_CONTROL, ENABLE,                 1) );
    }
    else
    {
        ulAmole_controlReg |= (
            BCHP_FIELD_DATA (AMOLE_ANCIL_0_CONTROL, ENABLE,                 0) );
    }
    BREG_Write32 (
        hReg, BCHP_AMOLE_ANCIL_0_CONTROL + ulCoreOffset, ulAmole_controlReg);

    BDBG_LEAVE(BVBI_P_AMOL_Encode_Enable_isr);
    return BERR_SUCCESS;
}
#endif /** } BVBI_NUM_AMOLE_656 **/
#endif


/***************************************************************************
* Static (private) functions
***************************************************************************/


#if (BVBI_NUM_AMOLE > 0) /** { **/

/***************************************************************************
 *
 */
static void BVBI_P_ProgramNull_isr (
    BREG_Handle hReg, uint32_t coreOffset,
    uint32_t ulWritePointer, uint32_t value)
{
    uint32_t ulRegAddr = BCHP_AMOLE_0_NULL + coreOffset;
    uint32_t ulRegVal  = BREG_Read32 (hReg, ulRegAddr);

    switch (ulWritePointer & 0x3)
    {
    case 0:
        ulRegVal &=
            ~BCHP_MASK       (AMOLE_0_NULL, NULL_ENABLE_BANK0       );
        ulRegVal |=
             BCHP_FIELD_DATA (AMOLE_0_NULL, NULL_ENABLE_BANK0, value);
        break;
    case 1:
        ulRegVal &=
            ~BCHP_MASK       (AMOLE_0_NULL, NULL_ENABLE_BANK1       );
        ulRegVal |=
             BCHP_FIELD_DATA (AMOLE_0_NULL, NULL_ENABLE_BANK1, value);
        break;
    case 2:
        ulRegVal &=
            ~BCHP_MASK       (AMOLE_0_NULL, NULL_ENABLE_BANK2       );
        ulRegVal |=
             BCHP_FIELD_DATA (AMOLE_0_NULL, NULL_ENABLE_BANK2, value);
        break;
    case 3:
        ulRegVal &=
            ~BCHP_MASK       (AMOLE_0_NULL, NULL_ENABLE_BANK3       );
        ulRegVal |=
             BCHP_FIELD_DATA (AMOLE_0_NULL, NULL_ENABLE_BANK3, value);
        break;
    /* coverity[dead_error_begin: FALSE] */
    default:
        /* Should never happen! Programming error! */
        BDBG_ASSERT (false);
        break;
    }

    BREG_Write32 (hReg, ulRegAddr, ulRegVal);
}

static uint32_t P_GetCoreOffset_isrsafe (uint8_t hwCoreIndex)
{
    uint32_t ulCoreOffset = 0xFFFFFFFF;

    switch (hwCoreIndex)
    {
#if (BVBI_NUM_AMOLE >= 1)
    case 0:
        ulCoreOffset = 0;
        break;
#endif
#if (BVBI_NUM_AMOLE >= 2)
    case 1:
        ulCoreOffset = (BCHP_AMOLE_1_REG_START - BCHP_AMOLE_0_REG_START);
        break;
#endif
#if (BVBI_NUM_AMOLE >= 3)
    case 2:
        ulCoreOffset = (BCHP_AMOLE_2_REG_START - BCHP_AMOLE_0_REG_START);
        break;
#endif
    default:
        break;
    }

    return ulCoreOffset;
}

#endif /** } BVBI_NUM_AMOLE **/

#if (BVBI_NUM_AMOLE_656 > 0) /** { **/

/***************************************************************************
 *
 */
static void BVBI_P_ProgramNull_656_isr (
    BREG_Handle hReg, uint32_t coreOffset,
    uint32_t ulWritePointer, uint32_t value)
{
    uint32_t ulRegAddr = BCHP_AMOLE_ANCIL_0_NULL + coreOffset;
    uint32_t ulRegVal  = BREG_Read32 (hReg, ulRegAddr);

    switch (ulWritePointer & 0x3)
    {
    case 0:
        ulRegVal &=
            ~BCHP_MASK       (AMOLE_ANCIL_0_NULL, NULL_ENABLE_BANK0       );
        ulRegVal |=
             BCHP_FIELD_DATA (AMOLE_ANCIL_0_NULL, NULL_ENABLE_BANK0, value);
        break;
    case 1:
        ulRegVal &=
            ~BCHP_MASK       (AMOLE_ANCIL_0_NULL, NULL_ENABLE_BANK1       );
        ulRegVal |=
             BCHP_FIELD_DATA (AMOLE_ANCIL_0_NULL, NULL_ENABLE_BANK1, value);
        break;
    case 2:
        ulRegVal &=
            ~BCHP_MASK       (AMOLE_ANCIL_0_NULL, NULL_ENABLE_BANK2       );
        ulRegVal |=
             BCHP_FIELD_DATA (AMOLE_ANCIL_0_NULL, NULL_ENABLE_BANK2, value);
        break;
    case 3:
        ulRegVal &=
            ~BCHP_MASK       (AMOLE_ANCIL_0_NULL, NULL_ENABLE_BANK3       );
        ulRegVal |=
             BCHP_FIELD_DATA (AMOLE_ANCIL_0_NULL, NULL_ENABLE_BANK3, value);
        break;
    /* PROGRAMMING NOTE: Coverity is flagging this as an error, even with the
     * ----------------  following  conventional comment. I always like to
     *                   have a default: clause, but too bad.
     */
    /* Should never happen! Programming error! */
    /* coverity[dead_error_begin: FALSE] */
    /*
    default:
        BDBG_ASSERT (false);
        break;
    */
    }

    BREG_Write32 (hReg, ulRegAddr, ulRegVal);
}

static uint32_t P_GetCoreOffset_656_isrsafe (uint8_t hwCoreIndex)
{
    uint32_t ulCoreOffset = 0xFFFFFFFF;

    switch (hwCoreIndex)
    {
#if (BVBI_NUM_AMOLE_656 >= 1)
    case 0:
        ulCoreOffset = 0;
        break;
#endif
#if (BVBI_NUM_AMOLE_656 >= 2)
    case 1:
        ulCoreOffset =
            (BCHP_AMOLE_ANCIL_1_REG_START - BCHP_AMOLE_ANCIL_0_REG_START);
        break;
#endif
#if (BVBI_NUM_AMOLE_656 >= 3)
    case 2:
        ulCoreOffset =
            (BCHP_AMOLE_ANCIL_2_REG_START - BCHP_AMOLE_ANCIL_0_REG_START);
        break;
#endif
    default:
        break;
    }

    return ulCoreOffset;
}

#endif /** } BVBI_NUM_AMOLE_656 **/
