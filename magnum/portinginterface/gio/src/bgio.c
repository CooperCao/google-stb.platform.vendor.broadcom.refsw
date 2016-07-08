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
 *
 ***************************************************************************/

#include "bgio.h"             /*  */
#include "bgio_priv.h"        /*  */
#include "bkni.h"

BDBG_MODULE(BGIO);
BDBG_OBJECT_ID(BGIO);

/***************************************************************************
 *
 */
BERR_Code BGIO_Open(
    BGIO_Handle *         phGpio,
    BCHP_Handle           hChip,
    BREG_Handle           hRegister )
{
    BERR_Code  eResult = BERR_SUCCESS;
    BGIO_P_Context *  pGpio = NULL;

    if ( (NULL == phGpio) ||
         (NULL == hChip) ||
         (NULL == hRegister) )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    pGpio = (BGIO_P_Context *)BKNI_Malloc( sizeof(BGIO_P_Context) );
    if ( NULL == pGpio )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset((void*)pGpio, 0x0, sizeof(BGIO_P_Context));
    BDBG_OBJECT_SET(pGpio, BGIO);

    /* init pin list */
    BLST_D_INIT(&pGpio->PinHead);

    pGpio->hChip = hChip;
    pGpio->hRegister = hRegister;
    BGIO_P_MAIN_SET_BLACK_MAGIC(pGpio);

    *phGpio = pGpio;
    return eResult;
}

/***************************************************************************
 *
 */
BERR_Code BGIO_Close(
    BGIO_Handle           hGpio )
{
    BDBG_OBJECT_ASSERT(hGpio, BGIO);

    /* sub-modules have to be explicitly destroied first */
    if ( true != BLST_D_EMPTY(&hGpio->PinHead) )
    {
        return BERR_TRACE(BERR_LEAKED_RESOURCE);
    }

    BDBG_OBJECT_DESTROY(hGpio, BGIO);
    BKNI_Free((void*)hGpio);
    return BERR_SUCCESS;
}

/* End of File */
