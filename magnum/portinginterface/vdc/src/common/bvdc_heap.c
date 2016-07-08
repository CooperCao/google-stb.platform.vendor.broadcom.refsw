/******************************************************************************
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
 ******************************************************************************/
#include "bstd.h"                /* standard types */
#include "bdbg.h"                /* Dbglib */
#include "bkni.h"                /* malloc */

#include "bmem.h"
#include "bvdc.h"                /* Video display */
#include "bvdc_bufferheap_priv.h"
#include "bvdc_priv.h"

BDBG_MODULE(BVDC_HEAP);
BDBG_FILE_MODULE(BVDC_WIN_BUF);

/***************************************************************************
 *
 */
BERR_Code BVDC_Heap_Create
    ( BVDC_Handle                       hVdc,
      BVDC_Heap_Handle                 *phHeap,
      BMMA_Heap_Handle                  hMem,
      const BVDC_Heap_Settings         *pSettings )
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Heap_Create);
    BDBG_ASSERT(hVdc);
    BDBG_ASSERT(hMem);
    BDBG_ASSERT(pSettings);

    err = BVDC_P_CheckHeapSettings(pSettings);
    if( err != BERR_SUCCESS )
    {
        return BERR_TRACE(err);
    }

    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("--------User heap settings--------"));
    BVDC_P_PrintHeapInfo(pSettings);
    err = BVDC_P_BufferHeap_Create(hVdc, phHeap, hMem, pSettings);
    if (err != BERR_SUCCESS)
        return BERR_TRACE(err);

    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("BVDC_Heap_Create: heap 0x%lx created",
        (unsigned long)(*phHeap)));
    BDBG_LEAVE(BVDC_Heap_Create);
    return BERR_SUCCESS;

}

/***************************************************************************
 *
 */
BERR_Code BVDC_Heap_Destroy
    ( BVDC_Heap_Handle                 hHeap)
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Heap_Destroy);
    BDBG_ASSERT(hHeap);

    err = BVDC_P_BufferHeap_Destroy(hHeap);
    if (err != BERR_SUCCESS)
        return err;

    BDBG_LEAVE(BVDC_Heap_Destroy);
    return BERR_SUCCESS;
}
