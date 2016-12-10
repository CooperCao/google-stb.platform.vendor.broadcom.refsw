/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/


#include "bstd.h"
#include "bkni.h"

#include "bsagelib.h"
#include "bsagelib_boot.h"
#include "bsagelib_priv.h"
#include "priv/bsagelib_shared_types.h"
#include "bsage.h"

#include "bhsm.h"
#include "bhsm_keyladder.h"
#include "bhsm_keyladder_enc.h"
#include "bhsm_bseck.h"
#include "bhsm_verify_reg.h"
#include "bhsm_otpmsp.h"
#include "bhsm_misc.h"
#include "bsp_s_commands.h"
#include "bsp_s_misc.h"
#include "bsp_s_hw.h"
#include "bsp_s_download.h"
#include "bsp_s_otp_common.h"
#include "bsp_s_otp.h"
#include "bsp_s_mem_auth.h"

#include "bchp_common.h"


BDBG_MODULE(BSAGElib);



/****************************************
 * Public API
 ***************************************/

void
BSAGElib_Boot_GetDefaultSettings(
    BSAGElib_BootSettings *pBootSettings /* [in/out] */)
{
    BDBG_ENTER(BSAGElib_Boot_GetDefaultSettings);

    BDBG_ASSERT(pBootSettings);
    BKNI_Memset(pBootSettings, 0, sizeof(*pBootSettings));

    BDBG_LEAVE(BSAGElib_Boot_GetDefaultSettings);
}

BERR_Code
BSAGElib_Boot_Launch(
    BSAGElib_Handle hSAGElib,
    BSAGElib_BootSettings *pBootSettings)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGE_BootSettings sageBootSettings;
    uint8_t *hsi_buffers;
    uint32_t HsiOffset;

    BDBG_ENTER(BSAGElib_Boot_Launch);

    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);
    BDBG_ASSERT(pBootSettings);

    sageBootSettings.vklBoot = BCMD_VKL_eMax;

    /* Validate boot settings */
    if ((pBootSettings->pBootloader == NULL) ||
        (pBootSettings->bootloaderSize == 0) ||
        (pBootSettings->pFramework == NULL)      ||
        (pBootSettings->frameworkSize == 0)) {
        BDBG_ERR(("%s - Invalid SAGE image buffer.", __FUNCTION__));
        goto end;
    }

    if (hSAGElib->core_handles.hHsm == NULL) {
        BDBG_ERR(("%s - Invalid HSM handle.", __FUNCTION__));
        goto end;
    }

    sageBootSettings.bootloaderOffset = (uint32_t)BSAGElib_iAddrToOffset((const void *)pBootSettings->pBootloader);
    sageBootSettings.bootloaderSize   = pBootSettings->bootloaderSize;

    sageBootSettings.frameworkOffset = (uint32_t)BSAGElib_iAddrToOffset((const void *)pBootSettings->pFramework);
    sageBootSettings.frameworkSize = pBootSettings->frameworkSize;

    sageBootSettings.bootloaderDevOffset = (uint32_t)BSAGElib_iAddrToOffset((const void *)pBootSettings->pBootloaderDev);
    sageBootSettings.bootloaderDevSize   = pBootSettings->bootloaderDevSize;

    sageBootSettings.frameworkDevOffset = (uint32_t)BSAGElib_iAddrToOffset((const void *)pBootSettings->pFrameworkDev);
    sageBootSettings.frameworkDevSize = pBootSettings->frameworkDevSize;

    /* Buffer holding the parameters of SAGE log buffer*/
    sageBootSettings.logBufferOffset = pBootSettings->logBufferOffset;
    sageBootSettings.logBufferSize = pBootSettings->logBufferSize;

    /* Regions map; memory block that mus be accessible by SAGE-side
       (see bsagelib_shared_globalsram.h for more details) */
    sageBootSettings.regionMapOffset = (uint32_t)BSAGElib_iAddrToOffset((const void *)pBootSettings->pRegionMap);
    sageBootSettings.regionMapNum = pBootSettings->regionMapNum;
    BSAGElib_iFlush_isrsafe(pBootSettings->pRegionMap, pBootSettings->regionMapNum * sizeof(BSAGElib_RegionInfo));

    if(pBootSettings->HSIBufferOffset == 0)
    {
        hsi_buffers = BSAGElib_iMalloc(SAGE_HOST_BUF_SIZE*4);
        if (hsi_buffers == NULL) {
            rc = BERR_OUT_OF_DEVICE_MEMORY;
            BDBG_ERR(("%s: Cannot allocating hsi buffers", __FUNCTION__));
            goto end;
        }
        HsiOffset = BSAGElib_iAddrToOffset(hsi_buffers);
    }else{
        HsiOffset = pBootSettings->HSIBufferOffset;
    }

    sageBootSettings.HSIBufferOffset = HsiOffset;

    rc = hSAGElib->bsage.Boot_Launch(hSAGElib->core_handles.hHsm,&sageBootSettings);
/*    rc = BSAGE_Boot_Launch(hSAGElib->core_handles.hHsm,pBootSettings);*/
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BSAGE_Boot_Launch() fails %d", __FUNCTION__, rc));
        goto end;
    }

    BSAGElib_iLockHsm();
    hSAGElib->bsage.Boot_GetInfo(&hSAGElib->chipInfo,&hSAGElib->bootloaderInfo,&hSAGElib->frameworkInfo);
/*    BSAGE_Boot_GetInfo(&hSAGElib->chipInfo,&hSAGElib->bootloaderInfo,&hSAGElib->frameworkInfo);*/
    BSAGElib_iUnlockHsm();

end:

    BDBG_LEAVE(BSAGElib_Boot_Launch);
    return rc;
}

void
BSAGElib_Boot_GetBinariesInfo(
    BSAGElib_Handle hSAGElib,
    BSAGElib_ImageInfo *pBootloaderInfo,
    BSAGElib_ImageInfo *pFrameworkInfo)
{
    BDBG_ENTER(BSAGElib_Boot_GetBinariesInfo);
    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);

    if (pBootloaderInfo != NULL) {
        *pBootloaderInfo = hSAGElib->bootloaderInfo;
    }

    if (pFrameworkInfo != NULL) {
        *pFrameworkInfo = hSAGElib->frameworkInfo;
    }

    BDBG_LEAVE(BSAGElib_Boot_GetBinariesInfo);
}
