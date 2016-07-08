/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

/* base modules */
#include "bstd.h"           /* standard types */
#include "bdbg.h"           /* debug interface */
#include "berr.h"           /* error code */
#include "bkni.h"           /* kernel interface */

#include "bdsp_auth.h"
#include "bdsp_raaga.h"
#include "bdsp_raaga_img.h"
#include "bdsp_raaga_fwdownload.h"

BDBG_MODULE(bdsp_auth);

BERR_Code
BDSP_DumpImage(
   unsigned uiFirmwareId,
   void *pBuffer,
   unsigned uiBufferSize,
   void **pvCodeStart,
   unsigned *puiCodeSize
)
{
    BERR_Code rc = BERR_SUCCESS;
    BDSP_RaagaImgCacheEntry ImgCache[BDSP_IMG_ID_MAX];
    const BDSP_RaagaUsageOptions Usage;
    bool bUseBDSPMacro = true;
    unsigned i;
    unsigned uiFwBinSize, uiFwBinSizeWithGuardBand;

    BSTD_UNUSED( uiFirmwareId );

    *pvCodeStart = NULL;
    *puiCodeSize = 0;

    /* Find all the sizes for supported binaries */

    BKNI_Memset( ImgCache, 0, (sizeof(BDSP_RaagaImgCacheEntry)*BDSP_IMG_ID_MAX));
    uiFwBinSize = BDSP_Raaga_P_AssignAlgoSizes(
                &BDSP_IMG_Interface,
                (void **)BDSP_IMG_Context,
                ImgCache,
                &Usage,
                bUseBDSPMacro);

    for(i=0; i < BDSP_IMG_ID_MAX ; i ++ )
        BDBG_MSG((" %d : size = %d", i ,ImgCache[i].size));

    /* Guard band required for Raaga Code access */
    uiFwBinSizeWithGuardBand = uiFwBinSize + BDSP_CODE_DWNLD_GUARD_BAND_SIZE;

    if( uiFwBinSizeWithGuardBand > uiBufferSize )
    {
        BDBG_ERR((" Allocated memory for binary download less than the required memory. "));
        BDBG_ERR((" Please increase the define value at bdsp_auth.h. "));
        BDBG_ERR((" Allocated Size = %d firmware size = %d Diff(required - allocated ) = %d", uiBufferSize, uiFwBinSizeWithGuardBand, uiFwBinSizeWithGuardBand- uiBufferSize  ));
        rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }

    BKNI_Memset( pBuffer, 0, uiFwBinSizeWithGuardBand );
    rc = BDSP_Raaga_P_PreLoadFwImages(
                &BDSP_IMG_Interface,
                (void **)BDSP_IMG_Context,
                ImgCache,
                pBuffer,
                uiFwBinSize,
                NULL);
    if(rc != BERR_SUCCESS)
        goto error;

    *pvCodeStart = pBuffer;
    *puiCodeSize = uiFwBinSizeWithGuardBand;

error:
   return rc;
}
