/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/

#include "bstd.h"                       /* standard types */
#include "bint.h"
#include "bdbg.h"                       /* Dbglib */
#include "bkni.h"                       /* For critical sections */
#include "bchp_int_id_video_enc_intr2.h"
#include "bvbi.h"                       /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"          /* VBI internal data structures */
#include "bchp_vbi_enc.h"       /* RDB info for VBI_ENC registers */

BDBG_MODULE(BVBI);

/* This will make code more legible, in special cases. Like, chipsets that do
 * not support 656 output.
 */
#if (BVBI_NUM_CCE_656 == 0)
#define BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_CCE_ANCIL_0 0xFFFFFFFF
#endif
#if (BVBI_NUM_WSE_656 == 0)
#define BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_WSE_ANCIL_0 0xFFFFFFFF
#endif
#if (BVBI_NUM_TTE_656 == 0)
#define BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_TTE_ANCIL_0 0xFFFFFFFF
#endif
#if (BVBI_NUM_GSE_656 == 0)
#define BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_GSE_ANCIL_0 0xFFFFFFFF
#endif
#if (BVBI_NUM_AMOLE_656 == 0)
#define BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_AMOLE_ANCIL_0 0xFFFFFFFF
#endif

#if (BVBI_NUM_CCE_656 < 2)
#define BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_CCE_ANCIL_1 0xFFFFFFFF
#endif
#if (BVBI_NUM_WSE_656 < 2)
#define BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_WSE_ANCIL_1 0xFFFFFFFF
#endif
#if (BVBI_NUM_TTE_656 < 2)
#define BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_TTE_ANCIL_1 0xFFFFFFFF
#endif
#if (BVBI_NUM_GSE_656 < 2)
#define BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_GSE_ANCIL_1 0xFFFFFFFF
#endif
#if (BVBI_NUM_AMOLE_656 < 2)
#define BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_AMOLE_ANCIL_1 0xFFFFFFFF
#endif

/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/
static void BVBI_P_VE_Enc_Init (BREG_Handle hReg, uint32_t ulRegAddr);


/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/

BERR_Code BVBI_Encode_GetInterruptName(
    BAVC_VbiPath  eVbiPath,
    BAVC_Polarity eFieldPolarity,
        BINT_Id*      pInterruptName
)
{
        int index;
        BERR_Code eErr = BERR_SUCCESS;
        BINT_Id aEncodeInterruptName[2] = {0, 0};

        BDBG_ENTER(BVBI_Encode_GetInterruptName);

        /* Check for some obvious errors */
        if(!pInterruptName)
        {
                BDBG_ERR(("Invalid parameter"));
                eErr = BERR_TRACE(BERR_INVALID_PARAMETER);
                goto BVBI_Encode_GetInterruptName_Done;
        }
        switch (eFieldPolarity)
        {
                case BAVC_Polarity_eFrame:
                case BAVC_Polarity_eTopField:
                case BAVC_Polarity_eBotField:
                        break;
                default:
                        BDBG_ERR(("Invalid parameter"));
                        eErr = BERR_TRACE(BERR_INVALID_PARAMETER);
                        goto BVBI_Encode_GetInterruptName_Done;
                        break;
        }
        /* Interrupts to use according to VEC path */
        switch (eVbiPath)
        {
#if (BVBI_NUM_VEC >= 1)
        case BAVC_VbiPath_eVec0:
                aEncodeInterruptName[0] = BCHP_INT_ID_VBI_0_0_INTR;
                aEncodeInterruptName[1] = BCHP_INT_ID_VBI_0_1_INTR;
                break;
#endif
#if (BVBI_NUM_VEC >= 2)
        case BAVC_VbiPath_eVec1:
                aEncodeInterruptName[0] = BCHP_INT_ID_VBI_1_0_INTR;
                aEncodeInterruptName[1] = BCHP_INT_ID_VBI_1_1_INTR;
                break;
#endif
#if (BVBI_NUM_VEC >= 3)
        case BAVC_VbiPath_eVec2:
                aEncodeInterruptName[0] = BCHP_INT_ID_VBI_2_0_INTR;
                aEncodeInterruptName[1] = BCHP_INT_ID_VBI_2_1_INTR;
                break;
#endif
#if (BVBI_NUM_PTVEC > 0) /** { **/
        case BAVC_VbiPath_eBypass0:
#if   (BCHP_CHIP == 7422) || (BCHP_CHIP == 7425) || (BCHP_CHIP == 7435)
                aEncodeInterruptName[0] = BCHP_INT_ID_ANCIL_VBI_0_0_INTR;
                aEncodeInterruptName[1] = BCHP_INT_ID_ANCIL_VBI_1_0_INTR;
#else
            /* Weird */
                aEncodeInterruptName[0] = BCHP_INT_ID_ANCIL_VBI_0_0_INTR;
                aEncodeInterruptName[1] = BCHP_INT_ID_ANCIL_VBI_0_1_INTR;
#endif
                break;
#endif /** } (BVBI_NUM_PTVEC > 0) **/
        default:
                aEncodeInterruptName[0] = 0;
                aEncodeInterruptName[1] = 0;
                eErr = BVBI_ERR_HW_UNSUPPORTED;
                goto BVBI_Encode_GetInterruptName_Done;
                break;
        }

        /* Finally, apply field polarity */
        index = (eFieldPolarity == BAVC_Polarity_eBotField ? 1 : 0);
        *pInterruptName = aEncodeInterruptName[index];

BVBI_Encode_GetInterruptName_Done:
        BDBG_LEAVE(BVBI_Encode_GetInterruptName);
        return eErr;
}


/***************************************************************************
* Implementation of supporting VBI_ENC functions that are not in API
***************************************************************************/


BERR_Code BVBI_P_VE_Init( BVBI_P_Handle *pVbi )
{
        int iCore;
        uint8_t hwCoreIndex[BVBI_P_EncCoreType_eLAST];

        BDBG_ENTER(BVBI_P_VE_Init);

        for (iCore = 0 ; iCore < BVBI_P_EncCoreType_eLAST ; ++iCore)
                hwCoreIndex[iCore] = 0xFF;

#if (BVBI_NUM_VEC >= 1)
        BVBI_P_VE_Enc_Init (pVbi->hReg, BCHP_VBI_ENC_VBI_0_INTR_CTRL);
        BVBI_P_VE_Crossbar_Program (pVbi->hReg, BAVC_VbiPath_eVec0, hwCoreIndex);
#endif
#if (BVBI_NUM_VEC >= 2)
        BVBI_P_VE_Enc_Init (pVbi->hReg, BCHP_VBI_ENC_VBI_1_INTR_CTRL);
        BVBI_P_VE_Crossbar_Program (pVbi->hReg, BAVC_VbiPath_eVec1, hwCoreIndex);
#endif
#if (BVBI_NUM_VEC >= 3)
        BVBI_P_VE_Enc_Init (pVbi->hReg, BCHP_VBI_ENC_VBI_2_INTR_CTRL);
        BVBI_P_VE_Crossbar_Program (pVbi->hReg, BAVC_VbiPath_eVec2, hwCoreIndex);
#endif
#if (BVBI_NUM_PTVEC >= 1)
        BVBI_P_VE_Enc_Init (pVbi->hReg, BCHP_VBI_ENC_VBI_ANCIL_0_INTR_CTRL);
        BVBI_P_VE_Crossbar_Program (pVbi->hReg, BAVC_VbiPath_eBypass0, hwCoreIndex);
#endif
#if (BVBI_NUM_PTVEC >= 2)
        BVBI_P_VE_Enc_Init (pVbi->hReg, BCHP_VBI_ENC_VBI_ANCIL_1_INTR_CTRL);
        BVBI_P_VE_Crossbar_Program (pVbi->hReg, BAVC_VbiPath_eBypass1, hwCoreIndex);
#endif

        BDBG_LEAVE(BVBI_P_VE_Init);
        return BERR_SUCCESS;
}


BERR_Code BVBI_P_VE_Enc_Program (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    uint32_t ulActive_Standards,
    uint32_t ulActive_656_Standards,
    BFMT_VideoFmt eVideoFormat)
{
    uint32_t topLine;
    uint32_t botLine;
    uint32_t ulReg;
    uint32_t ulRegAddr = 0xFFFFFFFF;

    BDBG_ENTER(BVBI_P_VE_Enc_Program);

    BSTD_UNUSED (ulActive_Standards);
    BSTD_UNUSED (ulActive_656_Standards);

    /* Figure out which core register to use */
    switch (hwCoreIndex)
    {
    case BAVC_VbiPath_eVec0:
        /* coverity[dead_error_condition: FALSE] */
        if (!is656)
        {
#if (BVBI_NUM_VEC >= 1)
            /* coverity[dead_error_line: FALSE] */
            ulRegAddr = BCHP_VBI_ENC_VBI_0_INTR_CTRL;
#endif
        }
        break;
    case BAVC_VbiPath_eVec1:
        /* coverity[dead_error_condition: FALSE] */
        if (!is656)
        {
#if (BVBI_NUM_VEC >= 2)
            /* coverity[dead_error_line: FALSE] */
            ulRegAddr = BCHP_VBI_ENC_VBI_1_INTR_CTRL;
#endif
        }
        break;
    case BAVC_VbiPath_eVec2:
        /* coverity[dead_error_condition: FALSE] */
        if (!is656)
        {
#if (BVBI_NUM_VEC >= 3)
            /* coverity[dead_error_line: FALSE] */
            ulRegAddr = BCHP_VBI_ENC_VBI_2_INTR_CTRL;
#endif
        }
        break;
    case BAVC_VbiPath_eBypass0:
        /* coverity[dead_error_condition: FALSE] */
        if (is656)
        {
#if (BVBI_NUM_PTVEC >= 1)
            /* coverity[dead_error_line: FALSE] */
            ulRegAddr = BCHP_VBI_ENC_VBI_ANCIL_0_INTR_CTRL;
#endif
        }
        break;
    case BAVC_VbiPath_eBypass1:
        /* coverity[dead_error_condition: FALSE] */
        if (is656)
        {
#if (BVBI_NUM_PTVEC >= 2)
            /* coverity[dead_error_line: FALSE] */
            ulRegAddr = BCHP_VBI_ENC_VBI_ANCIL_1_INTR_CTRL;
#endif
        }
        break;
    default:
        break;
    }
    if (ulRegAddr == 0xFFFFFFFF)
    {
        /* This should never happen!  This parameter was checked by
           BVBI_Encode_Create() */
        BDBG_LEAVE(BVBI_P_VE_Enc_Program);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Prepare to program the interrupt control register */
    ulReg = BREG_Read32 ( hReg,  ulRegAddr );

    /* Select video format */
    switch (eVideoFormat)
    {
    case BFMT_VideoFmt_eNTSC:
    case BFMT_VideoFmt_eNTSC_J:
    case BFMT_VideoFmt_e720x482_NTSC:
    case BFMT_VideoFmt_e720x482_NTSC_J:
    case BFMT_VideoFmt_ePAL_M:
        /* NTSC specific settings */
        topLine = 37;
        botLine = 300;
        break;

    case BFMT_VideoFmt_e1080i:
    case BFMT_VideoFmt_e1080i_50Hz:
        topLine = 21;
        botLine = 584;
        break;

    case BFMT_VideoFmt_e720p:
    case BFMT_VideoFmt_e720p_24Hz:
    case BFMT_VideoFmt_e720p_25Hz:
    case BFMT_VideoFmt_e720p_30Hz:
    case BFMT_VideoFmt_e720p_50Hz:
        topLine = 26;
        botLine = 0;
        break;

    case BFMT_VideoFmt_e480p:
    case BFMT_VideoFmt_e720x483p:
        topLine = 46;
        botLine = 0;
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
        topLine = 47;
        botLine = 360;
        break;

    case BFMT_VideoFmt_e576p_50Hz:
        topLine = 23;
        botLine = 0;
        break;

    default:
        BDBG_LEAVE(BVBI_P_VE_Enc_Program);
        BDBG_ERR(("BVBI_VE: video format %d not supported", eVideoFormat));
        return BERR_TRACE (BERR_NOT_SUPPORTED);
        break;
    }

    /* Finish programming the interrupt control register */
#if (BVBI_NUM_VEC >= 1)
    ulReg &= ~(
         BCHP_MASK      (VBI_ENC_VBI_0_INTR_CTRL, INTR1_LINE         ) |
         BCHP_MASK      (VBI_ENC_VBI_0_INTR_CTRL, INTR0_LINE         )
    );
    ulReg |= (
         BCHP_FIELD_DATA(VBI_ENC_VBI_0_INTR_CTRL, INTR1_LINE, botLine) |
         BCHP_FIELD_DATA(VBI_ENC_VBI_0_INTR_CTRL, INTR0_LINE, topLine)
    );
#else
    ulReg &= ~(
         BCHP_MASK      (VBI_ENC_VBI_ANCIL_0_INTR_CTRL, INTR1_LINE         ) |
         BCHP_MASK      (VBI_ENC_VBI_ANCIL_0_INTR_CTRL, INTR0_LINE         )
    );
    ulReg |= (
         BCHP_FIELD_DATA(VBI_ENC_VBI_ANCIL_0_INTR_CTRL, INTR1_LINE, botLine) |
         BCHP_FIELD_DATA(VBI_ENC_VBI_ANCIL_0_INTR_CTRL, INTR0_LINE, topLine)
    );
#endif
    BREG_Write32 (hReg, ulRegAddr, ulReg);

    BDBG_LEAVE(BVBI_P_VE_Enc_Program);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
void BVBI_P_VE_Crossbar_Program (
    BREG_Handle hReg,
    BAVC_VbiPath eDest,
    uint8_t hwCoreIndex[BVBI_P_EncCoreType_eLAST])
{
    uint32_t ulRegMax;
    uint32_t ulRegAddr;
    uint32_t iReg;
    uint32_t ulBit;
    int iCore;
    uint8_t coreIndex;
    bool is656 = false;
    uint32_t ulRegBase = 0xFFFFFFFF;

    BDBG_ENTER (BVBI_P_VE_Crossbar_Program);

    switch (eDest)
    {
    #if (BVBI_NUM_VEC >= 1)
    case BAVC_VbiPath_eVec0:
        ulRegBase = BCHP_VBI_ENC_VBI_0_CORE_0_SEL;
        is656 = false;
        break;
    #endif
    #if (BVBI_NUM_VEC >= 2)
    case BAVC_VbiPath_eVec1:
        ulRegBase = BCHP_VBI_ENC_VBI_1_CORE_0_SEL;
        is656 = false;
        break;
    #endif
    #if (BVBI_NUM_VEC >= 3)
    case BAVC_VbiPath_eVec2:
        ulRegBase = BCHP_VBI_ENC_VBI_2_CORE_0_SEL;
        is656 = false;
        break;
    #endif
    #if (BVBI_NUM_PTVEC >= 1)
    case BAVC_VbiPath_eBypass0:
        ulRegBase = BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL;
        is656 = true;
        break;
    #endif
    #if (BVBI_NUM_PTVEC >= 2)
    case BAVC_VbiPath_eBypass1:
        ulRegBase = BCHP_VBI_ENC_VBI_ANCIL_1_CORE_0_SEL;
        is656 = true;
        break;
    #endif
    default:
        break;
    }
    BDBG_ASSERT (ulRegBase != 0xFFFFFFFF);
    /* coverity[dead_error_condition: FALSE] */
    if (is656)
    {
        /* coverity[dead_error_line: FALSE] */
        ulRegMax = BVBI_P_ENC_NUM_CROSSBAR_REG_656;
    }
    else
    {
        /* coverity[dead_error_line: FALSE] */
        ulRegMax = BVBI_P_ENC_NUM_CROSSBAR_REG;
    }

    iReg = 0;
    for (iCore = 0 ; iCore < BVBI_P_EncCoreType_eLAST ; ++iCore)
    {
        coreIndex = hwCoreIndex[iCore];
        switch (iCore)
        {
        case BVBI_P_EncCoreType_eCCE:
            switch (coreIndex)
            {
            case 0:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_CCE_ANCIL_0;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_CCE_0;
                    #endif
                }
                break;
#if (BVBI_NUM_CCE >= 2)
            case 1:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_CCE_ANCIL_1;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_CCE_1;
                    #endif
                }
                break;
#endif
#if (BVBI_NUM_CCE >= 3)
            case 2:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit = 0xFFFFFFFF;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_CCE_2;
                    #endif
                }
                break;
#endif
            default:
                ulBit = 0xFFFFFFFF;
                break;
            }
            break;
        case BVBI_P_EncCoreType_eCGMSAE:
            switch (coreIndex)
            {
            case 0:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit = 0xFFFFFFFF;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_CGMSAE_0;
                    #endif
                }
                break;
#if (BVBI_NUM_CGMSAE >= 2)
            case 1:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit = 0xFFFFFFFF;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_CGMSAE_1;
                    #endif
                }
                break;
#endif
#if (BVBI_NUM_CGMSAE >= 3)
            case 2:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit = 0xFFFFFFFF;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_CGMSAE_2;
                    #endif
                }
                break;
#endif
            default:
                ulBit = 0xFFFFFFFF;
                break;
            }
            break;
        case BVBI_P_EncCoreType_eWSE:
            switch (coreIndex)
            {
            case 0:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_WSE_ANCIL_0;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_WSE_0;
                    #endif
                }
                break;
#if (BVBI_NUM_WSE >= 2)
            case 1:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit =
                        BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_WSE_ANCIL_1 ;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_WSE_1;
                    #endif
                }
                break;
#endif
#if (BVBI_NUM_WSE >= 3)
            case 2:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit = 0xFFFFFFFF;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_WSE_2;
                    #endif
                }
                break;
#endif
            default:
                ulBit = 0xFFFFFFFF;
                break;
            }
            break;
        case BVBI_P_EncCoreType_eTTE:
            switch (coreIndex)
            {
#if (BVBI_NUM_TTE >= 1)
            case 0:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_TTE_ANCIL_0;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_TTE_0;
                    #endif
                }
                break;
#endif
#if (BVBI_NUM_TTE >= 2)
            case 1:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_TTE_ANCIL_1;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_TTE_1;
                    #endif
                }
                break;
#endif
#if (BVBI_NUM_TTE >= 3)
            case 2:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit = 0xFFFFFFFF;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_TTE_2;
                    #endif
                }
                break;
#endif
            default:
                ulBit = 0xFFFFFFFF;
                break;
            }
            break;
        case BVBI_P_EncCoreType_eGSE:
            switch (coreIndex)
            {
#if (BVBI_NUM_GSE >= 1)
            case 0:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_GSE_ANCIL_0;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_GSE_0;
                    #endif
                }
                break;
#endif
#if (BVBI_NUM_GSE >= 2)
            case 1:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_GSE_ANCIL_1;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_GSE_1;
                    #endif
                }
                break;
#endif
#if (BVBI_NUM_GSE >= 3)
            case 2:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit = 0xFFFFFFFF;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_GSE_2;
                    #endif
                }
                break;
#endif
            default:
                ulBit = 0xFFFFFFFF;
                break;
            }
            break;
        case BVBI_P_EncCoreType_eAMOLE:
            switch (coreIndex)
            {
#if (BVBI_NUM_AMOLE >= 1)
            case 0:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit =
                        BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_AMOLE_ANCIL_0;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_AMOLE_0;
                    #endif
                }
                break;
#endif
#if (BVBI_NUM_AMOLE >= 2)
            case 1:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit =
                        BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_AMOLE_ANCIL_1;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_AMOLE_1;
                    #endif
                }
                break;
#endif
#if (BVBI_NUM_AMOLE >= 3)
            case 2:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit = 0xFFFFFFFF;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_AMOLE_2;
                    #endif
                }
                break;
#endif
            default:
                ulBit = 0xFFFFFFFF;
                break;
            }
            break;
        case BVBI_P_EncCoreType_eSCTE:
            switch (coreIndex)
            {
#if (BVBI_NUM_SCTEE >= 1)
            case 0:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit = 0xFFFFFFFF;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_SCTE_0;
                    #endif
                }
                break;
#endif
#if (BVBI_NUM_SCTEE >= 2)
            case 1:
                if (is656)
                {
                    #if (BVBI_NUM_PTVEC >= 1)
                    ulBit = 0xFFFFFFFF;
                    #endif
                }
                else
                {
                    #if (BVBI_NUM_VEC >= 1)
                    ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_SCTE_1;
                    #endif
                }
                break;
#endif
            default:
                ulBit = 0xFFFFFFFF;
                break;
            }
            break;
        default:
            ulBit = 0xFFFFFFFF;
            break;
        }
        if (iReg >= ulRegMax)
        {
            /* Note: this is a silent failure. TODO. */
            break;
        }
        if (ulBit != 0xFFFFFFFF)
        {
            ulRegAddr = ulRegBase + 4 * iReg;
            BREG_Write32 (hReg, ulRegAddr, ulBit);
            ++iReg;
        }
    }

    /* Zero out unused crossbar entries */
    if (is656)
    {
#if (BVBI_NUM_PTVEC >= 1)
        ulBit = BCHP_VBI_ENC_VBI_ANCIL_0_CORE_0_SEL_SEL_DISABLE;
#endif
    }
    else
    {
#if (BVBI_NUM_VEC >= 1)
        ulBit = BCHP_VBI_ENC_VBI_0_CORE_0_SEL_SEL_DISABLE;
#endif
    }

    while (iReg < ulRegMax)
    {
        ulRegAddr = ulRegBase + 4 * iReg;
        BREG_Write32 (hReg, ulRegAddr, ulBit);
        ++iReg;
    }

    BDBG_LEAVE (BVBI_P_VE_Crossbar_Program);
}

/***************************************************************************
* Static (private) functions
***************************************************************************/

/***************************************************************************
 *
 */
static void BVBI_P_VE_Enc_Init (BREG_Handle hReg, uint32_t ulRegAddr)
{
    uint32_t topLine;
    uint32_t botLine;
    uint32_t ulReg;

    BDBG_ENTER(BVBI_P_VE_Enc_Init);

    /* Cause interrupts to occur according to start of NTSC active video
    (default) */
    ulReg = BREG_Read32 ( hReg,  ulRegAddr );
    topLine = 23;
    botLine = 286;
#if (BVBI_NUM_VEC >= 1)
    ulReg &= ~(
         BCHP_MASK      (VBI_ENC_VBI_0_INTR_CTRL, INTR1_LINE         ) |
         BCHP_MASK      (VBI_ENC_VBI_0_INTR_CTRL, INTR0_LINE         )
    );
    ulReg |= (
         BCHP_FIELD_DATA(VBI_ENC_VBI_0_INTR_CTRL, INTR1_LINE, botLine) |
         BCHP_FIELD_DATA(VBI_ENC_VBI_0_INTR_CTRL, INTR0_LINE, topLine)
    );
#else
    ulReg &= ~(
         BCHP_MASK      (VBI_ENC_VBI_ANCIL_0_INTR_CTRL, INTR1_LINE         ) |
         BCHP_MASK      (VBI_ENC_VBI_ANCIL_0_INTR_CTRL, INTR0_LINE         )
    );
    ulReg |= (
         BCHP_FIELD_DATA(VBI_ENC_VBI_ANCIL_0_INTR_CTRL, INTR1_LINE, botLine) |
         BCHP_FIELD_DATA(VBI_ENC_VBI_ANCIL_0_INTR_CTRL, INTR0_LINE, topLine)
    );
#endif
BREG_Write32 (hReg, ulRegAddr, ulReg);

    BDBG_LEAVE(BVBI_P_VE_Enc_Init);
}

/* End of file */
