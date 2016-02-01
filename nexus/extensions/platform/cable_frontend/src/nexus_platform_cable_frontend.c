/***************************************************************************
 *     (c)2003-2012 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "nexus_platform_module.h"

BDBG_MODULE(nexus_platform_cable_frontend);

#if NEXUS_HAS_FRONTEND
/* This API is a tunnel from platform code to application code.
We can make use of the NEXUS_FrontendUserParameters, which serves the same purpose.
And piggy back on NEXUS_FrontendUserParameters.id for more storage here.
*/
static struct {
    unsigned mtsifNum;
    bool mtsifEnabled;
} g_data[NEXUS_MAX_FRONTENDS]; /* index is NEXUS_FrontendUserParameters.id */

NEXUS_Error NEXUS_Frontend_GetLTSID( NEXUS_FrontendHandle handle, NEXUS_FrontendLTSIDParameters *pParams )
{
    NEXUS_FrontendUserParameters userParams;
    
    BKNI_Memset(pParams, 0, sizeof(*pParams));
    
    NEXUS_Frontend_GetUserParameters(handle, &userParams);
    pParams->ltsidNum = userParams.param1;
    pParams->chipId = userParams.chipId;
    pParams->chipRevision = userParams.chipRevision;
    
    if (userParams.id < NEXUS_MAX_FRONTENDS) {
        pParams->mtsifNum = g_data[userParams.id].mtsifNum;
        pParams->mtsifEnabled = g_data[userParams.id].mtsifEnabled;
    }
    else {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    
    return 0;
}

NEXUS_Error NEXUS_Frontend_SetLTSID( NEXUS_FrontendHandle handle, const NEXUS_FrontendLTSIDParameters *pParams )
{
    NEXUS_FrontendUserParameters userParams;
    NEXUS_Error rc;
    
    NEXUS_Frontend_GetUserParameters(handle, &userParams);
    userParams.param1 = pParams->ltsidNum;
    userParams.chipId = pParams->chipId;
    userParams.chipRevision = pParams->chipRevision;
    rc = NEXUS_Frontend_SetUserParameters(handle, &userParams);
    if (rc) return BERR_TRACE(rc);
    
    if (userParams.id < NEXUS_MAX_FRONTENDS) {
        g_data[userParams.id].mtsifNum = pParams->mtsifNum;
        g_data[userParams.id].mtsifEnabled = pParams->mtsifEnabled;
    }
    else {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    
    return 0;
}
#else
NEXUS_Error NEXUS_Frontend_GetLTSID( NEXUS_FrontendHandle handle, NEXUS_FrontendLTSIDParameters *pParams )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pParams);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}
NEXUS_Error NEXUS_Frontend_SetLTSID( NEXUS_FrontendHandle handle, const NEXUS_FrontendLTSIDParameters *pParams )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pParams);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}
#endif
