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
#include "bvbi_priv.h"      /* VBI internal data structures */

#if (BVBI_P_HAS_XSER_TT != 0) /** { **/

#include "bchp_itu656_0.h"  /* RDB info for ITU656 registers */

BDBG_MODULE(BVBI);


/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/

/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/

/***************************************************************************
* Implementation of supporting VBI_ENC functions that are not in API
***************************************************************************/

BERR_Code BVBI_P_ITU656_Init(
    BREG_Handle hReg, const BVBI_XSER_Settings* pXSERdefaultSettings )
{
    uint32_t ulReg;
    uint32_t iMode;

    BDBG_ENTER(BVBI_P_ITU656_Init);

    switch (pXSERdefaultSettings->ttSerialDataSync)
    {
    case BVBI_TTserialDataSync_EAV:
        iMode = BCHP_ITU656_0_ITU656_TTE_CTRL_TTE_MODE_EAV;
        break;
    case BVBI_TTserialDataSync_SAV:
        iMode = BCHP_ITU656_0_ITU656_TTE_CTRL_TTE_MODE_SAV;
        break;
    case BVBI_TTserialDataSync_RQ:
        iMode = BCHP_ITU656_0_ITU656_TTE_CTRL_TTE_MODE_RQ;
        break;
    default:
        iMode = BCHP_ITU656_0_ITU656_TTE_CTRL_TTE_MODE_DISABLED;
        break;
    }

    ulReg = (
    BCHP_FIELD_DATA (ITU656_0_ITU656_TTE_CTRL, TTE_MODE,    iMode) |
    BCHP_FIELD_DATA (ITU656_0_ITU656_TTE_CTRL, DELAY_COUNT,
          pXSERdefaultSettings->iTTserialDataSyncDelay) );
    BREG_Write32 (hReg, BCHP_ITU656_0_ITU656_TTE_CTRL, ulReg);

    BDBG_LEAVE(BVBI_P_ITU656_Init);

    return BERR_SUCCESS;
}

BERR_Code BVBI_P_ITU656_Enc_Program (
    BREG_Handle hReg,
    BVBI_XSER_Settings* pSettings,
    uint32_t ulActive_XSER_Standards)
{
    uint32_t ulReg;
    uint32_t iMode;

    BDBG_ENTER(BVBI_P_ITU656_Enc_Program);

    switch (pSettings->ttSerialDataSync)
    {
    case BVBI_TTserialDataSync_EAV:
        iMode = BCHP_ITU656_0_ITU656_TTE_CTRL_TTE_MODE_EAV;
        break;
    case BVBI_TTserialDataSync_SAV:
        iMode = BCHP_ITU656_0_ITU656_TTE_CTRL_TTE_MODE_SAV;
        break;
    case BVBI_TTserialDataSync_RQ:
        iMode = BCHP_ITU656_0_ITU656_TTE_CTRL_TTE_MODE_RQ;
        break;
    default:
        iMode = BCHP_ITU656_0_ITU656_TTE_CTRL_TTE_MODE_DISABLED;
        break;
    }

    if ((ulActive_XSER_Standards & BVBI_P_SELECT_TT) == 0)
        iMode = BCHP_ITU656_0_ITU656_TTE_CTRL_TTE_MODE_DISABLED;

    ulReg = (
    BCHP_FIELD_DATA (ITU656_0_ITU656_TTE_CTRL, TTE_MODE,    iMode) |
    BCHP_FIELD_DATA (ITU656_0_ITU656_TTE_CTRL, DELAY_COUNT,
          pSettings->iTTserialDataSyncDelay) );
    BREG_Write32 (hReg, BCHP_ITU656_0_ITU656_TTE_CTRL, ulReg);

    BDBG_LEAVE(BVBI_P_ITU656_Enc_Program);


    return BERR_SUCCESS;
}

#endif /** }  BVBI_P_HAS_XSER_TT **/
