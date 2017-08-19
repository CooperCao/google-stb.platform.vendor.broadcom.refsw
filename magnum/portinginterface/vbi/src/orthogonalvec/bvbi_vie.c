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

#include "bstd.h"                       /* standard types */
#include "bdbg.h"                       /* Dbglib */
#include "bvbi.h"                       /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"          /* VBI internal data structures */
#include "bchp_vec_cfg.h"

BDBG_MODULE(BVBI);

/* This is better than having many ifdefs scattered throughout the file */
#if (BVBI_NUM_VEC > 0)
#define BCHP_VEC_CFG_SW_RESET_CCE_0        BCHP_VEC_CFG_SW_INIT_CCE_0
#define BCHP_VEC_CFG_SW_RESET_WSE_0        BCHP_VEC_CFG_SW_INIT_WSE_0
#define BCHP_VEC_CFG_SW_RESET_TTE_0        BCHP_VEC_CFG_SW_INIT_TTE_0
#define BCHP_VEC_CFG_SW_RESET_GSE_0        BCHP_VEC_CFG_SW_INIT_GSE_0
#define BCHP_VEC_CFG_SW_RESET_AMOLE_0      BCHP_VEC_CFG_SW_INIT_AMOLE_0
#define BCHP_VEC_CFG_SW_RESET_CGMSAE_0     BCHP_VEC_CFG_SW_INIT_CGMSAE_0
#endif
#if (BVBI_NUM_PTVEC > 0)
#define BCHP_VEC_CFG_SW_RESET_CCE_ANCIL_0   BCHP_VEC_CFG_SW_INIT_CCE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_WSE_ANCIL_0   BCHP_VEC_CFG_SW_INIT_WSE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_TTE_ANCIL_0   BCHP_VEC_CFG_SW_INIT_TTE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_GSE_ANCIL_0   BCHP_VEC_CFG_SW_INIT_GSE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_AMOLE_ANCIL_0                             \
    BCHP_VEC_CFG_SW_INIT_AMOLE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_ANCI656_ANCIL_0                           \
    BCHP_VEC_CFG_SW_INIT_ANCI656_ANCIL_0
#endif

/* This will make code more legible, in special cases. Like, chipsets that do
 * not support 656 output.
 */
#if (BVBI_NUM_CCE_656 == 0)
#undef BCHP_VEC_CFG_SW_RESET_CCE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_CCE_ANCIL_0 0xFFFFFFFF
#endif
#if (BVBI_NUM_WSE_656 == 0)
#undef BCHP_VEC_CFG_SW_RESET_WSE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_WSE_ANCIL_0 0xFFFFFFFF
#endif
#if (BVBI_NUM_TTE_656 == 0)
#undef BCHP_VEC_CFG_SW_RESET_TTE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_TTE_ANCIL_0 0xFFFFFFFF
#endif
#if (BVBI_NUM_GSE_656 == 0)
#undef BCHP_VEC_CFG_SW_RESET_GSE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_GSE_ANCIL_0 0xFFFFFFFF
#endif
#if (BVBI_NUM_AMOLE_656 == 0)
#undef BCHP_VEC_CFG_SW_RESET_AMOLE_ANCIL_0
#define BCHP_VEC_CFG_SW_RESET_AMOLE_ANCIL_0 0xFFFFFFFF
#endif

/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/


/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/


/***************************************************************************
* Implementation of supporting VBI_ENC functions that are not in API
***************************************************************************/

BERR_Code BVBI_P_VIE_SoftReset_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    uint32_t whichStandard)
{
    uint32_t ulRegAddr;
    uint32_t ulRegBase = 0xFFFFFFFF;

    BDBG_ENTER(BVBI_P_VIE_SoftReset_isr);

    switch (whichStandard)
    {
    case BVBI_P_SELECT_CC:
        if (is656)
        {
            #if (BVBI_NUM_PTVEC >= 1)
            ulRegBase = BCHP_VEC_CFG_SW_RESET_CCE_ANCIL_0;
            #endif
        }
        else
        {
            #if (BVBI_NUM_VEC >= 1)
            ulRegBase = BCHP_VEC_CFG_SW_RESET_CCE_0;
            #endif
        }
        break;
#if (BVBI_NUM_TTE > 0) || (BVBI_NUM_TTE_656 > 0)
    case BVBI_P_SELECT_TT:
        if (is656)
        {
            #if (BVBI_NUM_PTVEC >= 1)
            ulRegBase = BCHP_VEC_CFG_SW_RESET_TTE_ANCIL_0;
            #endif
        }
        else
        {
            #if (BVBI_NUM_VEC >= 1)
            ulRegBase = BCHP_VEC_CFG_SW_RESET_TTE_0;
            #endif
        }
        break;
#endif
    case BVBI_P_SELECT_WSS:
        if (is656)
        {
            #if (BVBI_NUM_PTVEC >= 1)
            ulRegBase = BCHP_VEC_CFG_SW_RESET_WSE_ANCIL_0;
            #endif
        }
        else
        {
            #if (BVBI_NUM_VEC >= 1)
            ulRegBase = BCHP_VEC_CFG_SW_RESET_WSE_0;
            #endif
        }
            break;
#if (BVBI_NUM_GSE > 0)
    case BVBI_P_SELECT_GS:
        if (is656)
        {
            #if (BVBI_NUM_PTVEC >= 1)
            ulRegBase = BCHP_VEC_CFG_SW_RESET_GSE_ANCIL_0;
            #endif
        }
        else
        {
            #if (BVBI_NUM_VEC >= 1)
            ulRegBase = BCHP_VEC_CFG_SW_RESET_GSE_0;
            #endif
        }
            break;
#endif
#if (BVBI_NUM_AMOLE > 0)
    case BVBI_P_SELECT_AMOL:
        if (is656)
        {
            #if (BVBI_NUM_PTVEC >= 1)
            ulRegBase = BCHP_VEC_CFG_SW_RESET_AMOLE_ANCIL_0;
            #endif
        }
        else
        {
            #if (BVBI_NUM_VEC >= 1)
            ulRegBase = BCHP_VEC_CFG_SW_RESET_AMOLE_0;
            #endif
        }
            break;
#endif
    #if (BVBI_NUM_VEC >= 1)
    case BVBI_P_SELECT_CGMSA:
    case BVBI_P_SELECT_CGMSB:
        ulRegBase = BCHP_VEC_CFG_SW_RESET_CGMSAE_0;
        break;
    #endif
#if (BVBI_NUM_SCTEE > 0)
    case BVBI_P_SELECT_SCTE:
        ulRegBase = BCHP_VEC_CFG_SW_RESET_SCTE_0;
        break;
#endif
    }

    /* Take care of errors above */
    if (ulRegBase == 0xFFFFFFFF)
    {
        BDBG_LEAVE(BVBI_P_VIE_SoftReset_isr);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Finally, program the soft reset register */
    ulRegAddr = ulRegBase + 4 * hwCoreIndex;
    BREG_Write32 (hReg, ulRegAddr, 0x1);
    BREG_Write32 (hReg, ulRegAddr, 0x0);

    BDBG_LEAVE(BVBI_P_VIE_SoftReset_isr);
    return BERR_SUCCESS;
}

#if (BVBI_NUM_ANCI656_656 > 0)
BERR_Code BVBI_P_VIE_AncilSoftReset (
        BREG_Handle hReg,
        uint8_t hwCoreIndex)
{
        uint32_t ulRegBase;
        uint32_t ulRegAddr;

        BDBG_ENTER(BVBI_P_VIE_AncilSoftReset);

        /* Figure out which encoder core to use */
        ulRegBase = BCHP_VEC_CFG_SW_RESET_ANCI656_ANCIL_0;
        ulRegAddr = ulRegBase + 4 * hwCoreIndex;

        /* Program the soft reset register */
        BREG_Write32 (hReg, ulRegAddr, 0x1);
        BREG_Write32 (hReg, ulRegAddr, 0x0);

        BDBG_LEAVE(BVBI_P_AncilSoftReset);
        return BERR_SUCCESS;
}
#endif

/***************************************************************************
* Static (private) functions
***************************************************************************/

/* End of file */
