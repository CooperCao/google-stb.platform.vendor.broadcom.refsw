/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
* API Description:
*   API name: wmdrmpd
*    Specific APIs related to Microsoft Windows Media DRM PD
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_wmdrmpd_core_module.h"
#include "nexus_memory.h"
#include "priv/nexus_core.h"
#include "priv/nexus_playpump_priv.h"
#include "priv/nexus_core_security.h"
#include "priv/nexus_wmdrmpd_core_priv.h"
#include "nexus_security_types.h"
#include "prdy_core.h"
#include "prdy_mp4.h"
#include "prdy_play.h"
#include "prdy_priv.h"
#include "prdy_decryptor.h"
#include "prdy_response.h"
#include "prdy_challenge.h"

BDBG_MODULE(nexus_wmdrmpd_core);

#define MAX_LICENCE_RESPONSE_LENGTH (1024*64)
#define DEFAULT_LICENSE_CHALLENGE_LENGTH (1024*64)
#define DEFAULT_LICENSE_URL_LENGTH (1024*1)
#define POLICY_POOL_SIZE (2)

typedef struct NEXUS_WmDrmPdCore
{
    NEXUS_OBJECT(NEXUS_WmDrmPdCore);
    NEXUS_WmDrmPdCoreSettings settings;
    bdrm_t bdrm;
    NEXUS_TaskCallbackHandle policyCallback;
    bdrm_decryptor_t bdrmDecryptor;
    NEXUS_KeySlotHandle keySlot;
    NEXUS_WmDrmPdEncryptionType encrType;
    NEXUS_WmDrmPdLicenseChallenge licenseData;
    bdrm_policy_t policy[POLICY_POOL_SIZE];  /* Some license have more than 1 policy to enforce before playing content. This array is
                                                used to store them up when the bdrm policy callback fires. */
#ifdef PRDY_ROBUSTNESS
    CommonCryptoHandle cryptoHandle;
#endif
    uint32_t nbOfPolicyStacked;
} NEXUS_WmDrmPdCore;

static void NEXUS_WmDrmPdCore_P_Finalizer(
    NEXUS_WmDrmPdCoreHandle hCore
    )
{
    NEXUS_WMDRMPD_SET_ACTIVE_IO(hCore->settings.ioHandle);
    
    if(hCore->licenseData.pResponseBuffer != NULL) NEXUS_Memory_Free((void*)(hCore->licenseData.pResponseBuffer));
    if(hCore->licenseData.pData != NULL)  NEXUS_Memory_Free((void*)(hCore->licenseData.pData));
    if(hCore->licenseData.pUrl != NULL) NEXUS_Memory_Free((void*)(hCore->licenseData.pUrl));
#if B_HAS_ASF
    if(hCore->settings.transportType == NEXUS_TransportType_eAsf){
        if (hCore->encrType == NEXUS_WmDrmPdEncryptionType_eAesCtr) 
        {
            if(hCore->bdrm->headers.asf.psi.data != NULL) NEXUS_Memory_Free((void*)(hCore->bdrm->headers.asf.psi.data));
        }
        if (hCore->encrType == NEXUS_WmDrmPdEncryptionType_eWmdrm)
        {
            if(hCore->bdrm->headers.asf.encr.sec_data != NULL)  NEXUS_Memory_Free((void *)(hCore->bdrm->headers.asf.encr.sec_data));
            if(hCore->bdrm->headers.asf.encr.prot_type != NULL) NEXUS_Memory_Free((void *)(hCore->bdrm->headers.asf.encr.prot_type));
            if(hCore->bdrm->headers.asf.encr.keyid != NULL) NEXUS_Memory_Free((void *)(hCore->bdrm->headers.asf.encr.keyid));
            if(hCore->bdrm->headers.asf.encr.licurl != NULL) NEXUS_Memory_Free((void *)(hCore->bdrm->headers.asf.encr.licurl));
            if(hCore->bdrm->headers.asf.xencr.data != NULL)NEXUS_Memory_Free((void *)(hCore->bdrm->headers.asf.xencr.data));
        }
    }
#endif

#ifdef PRDY_ROBUSTNESS
    CommonCrypto_Close(hCore->cryptoHandle);
#endif
    bdrm_close(hCore->bdrm);
    NEXUS_TaskCallback_Destroy(hCore->policyCallback);
    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    NEXUS_OBJECT_ASSERT(NEXUS_WmDrmPdCore, hCore);
    NEXUS_OBJECT_DESTROY(NEXUS_WmDrmPdCore, hCore);
    BKNI_Free(hCore);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_WmDrmPdCore, NEXUS_WmDrmPdCore_Destroy);


static bdrm_err NEXUS_WmDrmPdCore_P_PolicyCallback(
    bdrm_policy_t *policy,
    const void* ctx
    )
{
    NEXUS_WmDrmPdCoreHandle handle = (NEXUS_WmDrmPdCoreHandle)ctx;

    if(handle->nbOfPolicyStacked < POLICY_POOL_SIZE){ 
        BKNI_Memcpy(&handle->policy[handle->nbOfPolicyStacked], policy, sizeof(bdrm_policy_t));
        handle->nbOfPolicyStacked++;
    }
    else {
        BDBG_ERR(("Policy lost. The application needs to call NEXUS_WmDrmPdCore_GetPolicyStatus()"));
    }

    NEXUS_TaskCallback_Fire(handle->policyCallback);

    return bdrm_err_ok;
}

void NEXUS_WmDrmPdCore_GetDefaultSettings(
    NEXUS_WmDrmPdCoreSettings *pSettings    /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);

    BKNI_Memset(pSettings, 0, sizeof(NEXUS_WmDrmPdCoreSettings));

    pSettings->heap = NULL;
    pSettings->playpump = NULL;
    pSettings->transportType = NEXUS_TransportType_eUnknown;
}

NEXUS_WmDrmPdCoreHandle NEXUS_WmDrmPdCore_Create( /* attr{destructor=NEXUS_WmDrmPdCore_Destroy} */
    const NEXUS_WmDrmPdCoreSettings *pSettings  
    )
{
    NEXUS_WmDrmPdCoreSettings defaults;
    NEXUS_WmDrmPdCoreHandle hCore;
    NEXUS_HeapHandle hHeap;
    NEXUS_MemoryAllocationSettings allocSettings;
    NEXUS_Error errCode;
    bdrm_cfg cfg;
#ifdef PRDY_ROBUSTNESS
    CommonCryptoSettings cryptoSettings;
#endif

    if ( NULL == pSettings )
    {
        NEXUS_WmDrmPdCore_GetDefaultSettings(&defaults);
        pSettings = &defaults;
    }

    hCore = BKNI_Malloc(sizeof(NEXUS_WmDrmPdCore));
    if ( NULL == hCore )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }
    BKNI_Memset(hCore, 0, sizeof(NEXUS_WmDrmPdCore));

    hHeap = NEXUS_P_DefaultHeap(pSettings->heap, NEXUS_DefaultHeapType_eFull);
    if ( NULL == hHeap )
    {
        hHeap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
    }

    if (!NEXUS_P_CpuAccessibleHeap(hHeap)) 
    {
        BDBG_ERR(("Core heap is not CPU accessible.  Please provide a CPU-accessible heap in NEXUS_WmDrmPdCoreSettings."));
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto err_heap;
    }

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    allocSettings.heap = hHeap;
    errCode = NEXUS_Memory_Allocate(MAX_LICENCE_RESPONSE_LENGTH, &allocSettings, (void *)(&hCore->licenseData.pResponseBuffer));
    if ( errCode )
    {
       (void)BERR_TRACE(errCode);
        goto err_alloc_response;
    }

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    allocSettings.heap = hHeap;
    errCode = NEXUS_Memory_Allocate(DEFAULT_LICENSE_CHALLENGE_LENGTH, &allocSettings, (void *)(&hCore->licenseData.pData));
    if ( errCode )
    {
       (void)BERR_TRACE(errCode);
        goto err_alloc_data;
    }

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    allocSettings.heap = hHeap;
    errCode = NEXUS_Memory_Allocate(DEFAULT_LICENSE_URL_LENGTH, &allocSettings, (void *)(&hCore->licenseData.pUrl));
    if ( errCode )
    {
       (void)BERR_TRACE(errCode);
        goto err_alloc_url;
    }

    hCore->settings = *pSettings;
    hCore->settings.heap = hHeap;
    NEXUS_OBJECT_SET(NEXUS_WmDrmPdCore, hCore);

    NEXUS_WMDRMPD_SET_ACTIVE_IO(hCore->settings.ioHandle);

    BKNI_Memset(&cfg, 0, sizeof(bdrm_cfg));
    cfg.ext_asf_scanning = true;

    switch(pSettings->transportType){
#if B_HAS_ASF
    case NEXUS_TransportType_eAsf:
        cfg.cnt_type = bdrm_cnt_asf;
        /* Get asf handle */
        if(pSettings->playpump)
        {
             if(cfg.cnt_type == bdrm_cnt_asf)
             {
                 cfg.asf = NEXUS_Playpump_GetAsfHandle_priv(pSettings->playpump);
                 if(  cfg.asf == NULL )
                 {
                     BDBG_WRN(("drm asf is NULL"));
                 }
             }
             else 
             {
                 cfg.asf = NULL;
             }
        }
        else
        {
            BDBG_WRN(("Playpump handle not set"));
        }
        break;
#endif        
    case NEXUS_TransportType_eMp4Fragment:
        cfg.cnt_type = bdrm_cnt_mp4;
        break;
    default:
        errCode = NEXUS_INVALID_PARAMETER;
        (void)BERR_TRACE(errCode);
        goto err_type;
        break;
    }

    cfg.hHeap = hHeap;

    if(pSettings->policyCallback.callback){
        hCore->policyCallback = NEXUS_TaskCallback_Create(hCore, NULL);
        if(hCore->policyCallback == NULL){
            BDBG_ERR(("No policy callback provided in the configuration"));
            errCode = NEXUS_INVALID_PARAMETER;
            (void)BERR_TRACE(errCode);
            goto err_type;
        }
        
        NEXUS_TaskCallback_Set(hCore->policyCallback, &pSettings->policyCallback);

        cfg.opl_cback = NEXUS_WmDrmPdCore_P_PolicyCallback;
        cfg.opl_ctx = hCore;
    }
    else {
        BDBG_ERR(("No policy callback provided in the configuration"));
        errCode = NEXUS_INVALID_PARAMETER;
        (void)BERR_TRACE(errCode);
        goto err_type;    
    }

    if((hCore->bdrm = bdrm_open("", &cfg)) == NULL)
    {
        BDBG_ERR(("DRM Open failed"));
        goto err_drm_open;
    }
    else 
    {
        if( bdrm_init_handlers( hCore->bdrm ) == bdrm_err_ok )
        {
            if( bdrm_dcrypt_instance( hCore->bdrm, &hCore->bdrmDecryptor ) == bdrm_err_ok )
            {
                BDBG_MSG(("bdrm_dcrypt_instance success"));
            }
            else
            {
                BDBG_WRN(("bdrm_dcrypt_instance failed"));
            }
        }
        else
        {
            BDBG_WRN(("bdrm_init_handlers failed"));
        }
    }

#ifdef PRDY_ROBUSTNESS
    CommonCrypto_GetDefaultSettings(&cryptoSettings);
    hCore->cryptoHandle = CommonCrypto_Open(&cryptoSettings);
    if(hCore->cryptoHandle == NULL) goto err_drm_open;
#endif

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return hCore;

err_drm_open:
    NEXUS_TaskCallback_Destroy(hCore->policyCallback);
err_type:
    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    NEXUS_Memory_Free((void *)hCore->licenseData.pUrl);
err_alloc_url:
    NEXUS_Memory_Free((void *)hCore->licenseData.pData);
err_alloc_data:
    NEXUS_Memory_Free((void *)hCore->licenseData.pResponseBuffer);
err_alloc_response:
err_heap:
    BKNI_Free(hCore);
err_malloc:
    return NULL;
}

NEXUS_Error NEXUS_WmDrmPdCore_ConfigureKeySlot(
    NEXUS_WmDrmPdCoreHandle handle,
    NEXUS_KeySlotHandle keySlot
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_WmDrmPdCore, handle);
    NEXUS_OBJECT_ASSERT(NEXUS_KeySlot, keySlot);
    if ( keySlot->cryptoEngine != NEXUS_SecurityEngine_eGeneric )
    {
        BDBG_ERR(("A generic keyslot is required for DRM decryption"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if(handle->settings.transportType == NEXUS_TransportType_eAsf){
        NEXUS_KeySlot_SetTag(keySlot, handle->bdrmDecryptor);
    }else {
        NEXUS_KeySlot_SetTag(keySlot, handle->bdrm);
    }
    return BERR_SUCCESS;
}

void NEXUS_WmDrmPdCore_GetDefaultPsiObjectInfo(
    NEXUS_WmDrmPdPsiObjectInfo *pObject     /* [out] */
    )
{   
    BKNI_Memset(pObject, 0, sizeof(*pObject));
}

NEXUS_Error NEXUS_WmDrmPdCore_SetPsiObject(
    NEXUS_WmDrmPdCoreHandle handle,
    const NEXUS_WmDrmPdPsiObjectInfo *pInfo,
    const void *pData,
    size_t dataLength
    )
{
#if B_HAS_ASF
    NEXUS_Error errCode = NEXUS_SUCCESS;
    NEXUS_MemoryAllocationSettings allocSettings;

    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle); 
    NEXUS_OBJECT_ASSERT(NEXUS_WmDrmPdCore, handle);

    handle->encrType = NEXUS_WmDrmPdEncryptionType_eMax;

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    allocSettings.heap = handle->settings.heap;
    errCode = NEXUS_Memory_Allocate(dataLength, &allocSettings, (void *)(&handle->bdrm->headers.asf.psi.data));
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
        return BERR_OUT_OF_SYSTEM_MEMORY;
    }

    BDBG_MSG(("SET PSI HEADER"));
    BKNI_Memcpy(handle->bdrm->headers.asf.psi.data, pData, dataLength);
    handle->bdrm->headers.asf.psi.size = dataLength;
    handle->bdrm->headers.asf.psi.sysversion = pInfo->sysVersion;
    BKNI_Memcpy(&(handle->bdrm->headers.asf.psi.systemId), &(pInfo->systemId), sizeof(NEXUS_WmDrmPdGuid));
    handle->bdrm->headers.asf.psi.state = bdrm_hdr_read_ok;
    handle->encrType = NEXUS_WmDrmPdEncryptionType_eAesCtr;

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return BERR_SUCCESS;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pInfo);
    BSTD_UNUSED(pData);
    BSTD_UNUSED(dataLength);
    BDBG_ERR(("Please set MEDIA_ASF_SUPPORT=y"));
    return NEXUS_NOT_SUPPORTED;
#endif
}

NEXUS_Error NEXUS_WmDrmPdCore_SetCencrObject(
    NEXUS_WmDrmPdCoreHandle handle,
    const void *pSecurityData,  
    size_t securityLength,      
    const void *pProtocolData,  
    size_t protocolLength,      
    const void *pKeyIdData,     
    size_t keyIdLength,         
    const void *pLicenseUrlData,
    size_t licenseUrlLength     
    )
{
#if B_HAS_ASF
    NEXUS_Error errCode = NEXUS_SUCCESS;
    NEXUS_MemoryAllocationSettings allocSettings;

    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle); 
    NEXUS_OBJECT_ASSERT(NEXUS_WmDrmPdCore, handle);

    handle->encrType = NEXUS_WmDrmPdEncryptionType_eMax;

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    allocSettings.heap = handle->settings.heap;
    errCode = NEXUS_Memory_Allocate(securityLength, &allocSettings, (void *)(&handle->bdrm->headers.asf.encr.sec_data));
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
        return BERR_OUT_OF_SYSTEM_MEMORY;
    }
    errCode = NEXUS_Memory_Allocate(protocolLength, &allocSettings, (void *)(&handle->bdrm->headers.asf.encr.prot_type));
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
        return BERR_OUT_OF_SYSTEM_MEMORY;
    }
    errCode = NEXUS_Memory_Allocate(keyIdLength, &allocSettings, (void *)(&handle->bdrm->headers.asf.encr.keyid));
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
        return BERR_OUT_OF_SYSTEM_MEMORY;
    }
    errCode = NEXUS_Memory_Allocate(licenseUrlLength, &allocSettings, (void *)(&handle->bdrm->headers.asf.encr.licurl));
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
        return BERR_OUT_OF_SYSTEM_MEMORY;
    }

    BDBG_MSG(("SET CENCR HEADER"));
    BKNI_Memcpy(handle->bdrm->headers.asf.encr.sec_data, pSecurityData, securityLength);
    BKNI_Memcpy(handle->bdrm->headers.asf.encr.prot_type, pProtocolData, protocolLength);
    BKNI_Memcpy(handle->bdrm->headers.asf.encr.keyid, pKeyIdData, keyIdLength);
    BKNI_Memcpy(handle->bdrm->headers.asf.encr.licurl, pLicenseUrlData, licenseUrlLength);
    handle->bdrm->headers.asf.encr.sec_dlen = securityLength;
    handle->bdrm->headers.asf.encr.prot_tlen = protocolLength;
    handle->bdrm->headers.asf.encr.keyid_len = keyIdLength;
    handle->bdrm->headers.asf.encr.licurl_len = licenseUrlLength;
    handle->bdrm->headers.asf.encr.state = bdrm_hdr_read_ok;
    handle->encrType = NEXUS_WmDrmPdEncryptionType_eWmdrm;

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return BERR_SUCCESS;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSecurityData);  
    BSTD_UNUSED(securityLength);      
    BSTD_UNUSED(pProtocolData);  
    BSTD_UNUSED(protocolLength);      
    BSTD_UNUSED(pKeyIdData);     
    BSTD_UNUSED(keyIdLength);         
    BSTD_UNUSED(pLicenseUrlData);
    BSTD_UNUSED(licenseUrlLength);     
    BDBG_ERR(("Please set MEDIA_ASF_SUPPORT=y"));
    return NEXUS_NOT_SUPPORTED;
#endif
}

NEXUS_Error NEXUS_WmDrmPdCore_SetXcencrObject(
    NEXUS_WmDrmPdCoreHandle handle,
    const void *pData,  
    size_t dataLength  
    )
{
#if B_HAS_ASF
    NEXUS_Error errCode = NEXUS_SUCCESS;
    NEXUS_MemoryAllocationSettings allocSettings;

    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle); 
    NEXUS_OBJECT_ASSERT(NEXUS_WmDrmPdCore, handle);

    handle->encrType = NEXUS_WmDrmPdEncryptionType_eMax;

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    allocSettings.heap = handle->settings.heap;
    errCode = NEXUS_Memory_Allocate(dataLength, &allocSettings, (void *)(&handle->bdrm->headers.asf.xencr.data));
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
        return BERR_OUT_OF_SYSTEM_MEMORY;
    }

    BKNI_Memcpy(handle->bdrm->headers.asf.xencr.data, pData, dataLength);
    handle->bdrm->headers.asf.xencr.size = dataLength;
    handle->bdrm->headers.asf.xencr.state = bdrm_hdr_read_ok;
    handle->encrType = NEXUS_WmDrmPdEncryptionType_eWmdrm;

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return BERR_SUCCESS;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pData);
    BSTD_UNUSED(dataLength);
    BDBG_ERR(("Please set MEDIA_ASF_SUPPORT=y"));
    return NEXUS_NOT_SUPPORTED;
#endif
}

void NEXUS_WmDrmPdCore_GetDefaultDigsignObjectInfo(
    NEXUS_WmDrmPdDigsignObjectInfo *pObject     /* [out] */
    )
{   
    BKNI_Memset(pObject, 0, sizeof(*pObject));
}

NEXUS_Error NEXUS_WmDrmPdCore_SetDigsignObject(
    NEXUS_WmDrmPdCoreHandle handle,
    const NEXUS_WmDrmPdDigsignObjectInfo *pInfo,
    const void *pData,  
    size_t dataLength   
    )
{
#if B_HAS_ASF
    NEXUS_Error errCode = NEXUS_SUCCESS;
    NEXUS_MemoryAllocationSettings allocSettings;

    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle); 
    NEXUS_OBJECT_ASSERT(NEXUS_WmDrmPdCore, handle);

    handle->encrType = NEXUS_WmDrmPdEncryptionType_eMax;

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    allocSettings.heap = handle->settings.heap;
    errCode = NEXUS_Memory_Allocate(dataLength, &allocSettings, (void *)(&handle->bdrm->headers.asf.sign.data));
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
        return BERR_OUT_OF_SYSTEM_MEMORY;
    }

    BDBG_MSG(("SET DIGISIGN HEADER"));
    BKNI_Memcpy(handle->bdrm->headers.asf.sign.data, pData, dataLength);
    handle->bdrm->headers.asf.sign.len = dataLength;
    handle->bdrm->headers.asf.sign.type = pInfo->type;
    handle->bdrm->headers.asf.sign.state = bdrm_hdr_read_ok;
    handle->encrType = NEXUS_WmDrmPdEncryptionType_eWmdrm;

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return BERR_SUCCESS;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pInfo);
    BSTD_UNUSED(pData);
    BSTD_UNUSED(dataLength);
    BDBG_ERR(("Please set MEDIA_ASF_SUPPORT=y"));
    return NEXUS_NOT_SUPPORTED;
#endif
}

void NEXUS_WmDrmPdCore_GetDefaultPsshBoxInfo(
    NEXUS_WmDrmPdMp4PsshBoxInfo *pObject     /* [out] */
    )
{   
    BKNI_Memset(pObject, 0, sizeof(*pObject));
}

NEXUS_Error NEXUS_WmDrmPdCore_SetPsshBox(
    NEXUS_WmDrmPdCoreHandle handle,
    const NEXUS_WmDrmPdMp4PsshBoxInfo *pInfo,
    const void *pData,
    size_t dataLength
    )
{
    NEXUS_Error errCode = NEXUS_SUCCESS;

    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle); 
    NEXUS_OBJECT_ASSERT(NEXUS_WmDrmPdCore, handle);

    if(bdrm_mp4_set_pssh(handle->bdrm, pData, dataLength, pInfo->systemId.data, 16) != bdrm_err_ok)
    {
        (void)BERR_TRACE(NEXUS_NOT_AVAILABLE);
        errCode = NEXUS_NOT_AVAILABLE;
    }

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return errCode;
}

void NEXUS_WmDrmPdCore_GetDefaultProtectionSchemeInfo(
    NEXUS_WmDrmPdMp4ProtectionSchemeInfo *pObject     /* [out] */
    )
{   
    BKNI_Memset(pObject, 0, sizeof(*pObject));
}

NEXUS_Error NEXUS_WmDrmPdCore_SetProtectionSchemeBox(
    NEXUS_WmDrmPdCoreHandle handle,
    const NEXUS_WmDrmPdMp4ProtectionSchemeInfo *pInfo
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    bdrm_mp4_protect_scheme scheme;
    bdrm_encr_state encrState;

    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle); 
    NEXUS_OBJECT_ASSERT(NEXUS_WmDrmPdCore, handle);

    scheme.frma.codingname = pInfo->originalFormat.codingName;
    
    scheme.schm.fullbox.version = pInfo->schemeType.version;
    scheme.schm.fullbox.flags = pInfo->schemeType.flags;
    scheme.schm.scheme_type = pInfo->schemeType.schemeType;
    scheme.schm.scheme_version = pInfo->schemeType.schemeVersion;

    scheme.te.fullbox.version = pInfo->trackEncryption.version;
    scheme.te.fullbox.flags =  pInfo->trackEncryption.flags;
    scheme.te.info.alg_id  = pInfo->trackEncryption.info.algorithm;
    scheme.te.info.iv_size = pInfo->trackEncryption.info.ivSize;
    BKNI_Memcpy(&scheme.te.info.kid, pInfo->trackEncryption.info.keyId, NEXUS_WMDRMPD_KEY_ID_LENGTH);

    if(bdrm_mp4_setProtectionScheme(handle->bdrm, (const bdrm_mp4_protect_scheme *)&scheme, pInfo->trackId) != bdrm_err_ok)
    {
        (void)BERR_TRACE(NEXUS_NOT_AVAILABLE);
        rc =  NEXUS_NOT_AVAILABLE;
    }
    else {
        encrState = bdrm_is_drmencrypted(handle->bdrm);
        if(encrState == bdrm_encr_aes_ctr){
            handle->encrType = NEXUS_WmDrmPdEncryptionType_eAesCtr;
        } else if(encrState == bdrm_encr_aes_cbc){
            handle->encrType = NEXUS_WmDrmPdEncryptionType_eAesCbc;
        }
        else {
            handle->encrType = NEXUS_WmDrmPdEncryptionType_eNone;
        }
    }

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return rc;
}

NEXUS_Error NEXUS_WmDrmPdCore_LoadLicense(
    NEXUS_WmDrmPdCoreHandle handle
    )
{
    bdrm_license_t license = NULL;
    NEXUS_Error rc = NEXUS_SUCCESS; 
    bdrm_err err = bdrm_err_ok;

    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle); 

    err = bdrm_load_license(handle->bdrm, &license);
    if(err != bdrm_err_ok){
        if(err == bdrm_err_license_expired)
    {
            rc = NEXUS_WMDRMPD_LICENSE_EXPIRED;
        }
        else if( err == bdrm_err_revocation_package_expired)
        {
            rc = NEXUS_WMDRMPD_REVOCATION_PACKAGE;
        }
        else if( err == bdrm_err_domain_join)
    {
            rc = NEXUS_NOT_SUPPORTED;
        }
        else {
            rc = NEXUS_UNKNOWN;
        }
    }
    else
    {
        bdrm_decryptor_assign(handle->bdrm->dcrypt, handle->bdrm);
        bdrm_decryptor_set_encr_state(handle->bdrm->dcrypt, (bdrm_encr_state)handle->encrType);
    }

    if(license) Oem_MemFree(license);
    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return rc;
}

NEXUS_Error NEXUS_WmDrmPdCore_ContentSetProperty(
    NEXUS_WmDrmPdCoreHandle handle,
    NEXUS_WmDrmPdContentSetProperty id,
    const void *pData,  /* attr{nelem=dataLength} */
    size_t dataLength
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS; 
    bdrm_err err = bdrm_err_ok;
    bdrm_ContentSetProperty idLocal = bdrm_ContentSetProperty_eHeaderNotSet;

    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle); 

    /* Map content set properties from NEXUS_WmDrmPdContentSetProperty to bdrm_ContentSetProperty */
    switch (id) {
        case NEXUS_WmDrmPdContentSetProperty_eHeaderNotSet:
            idLocal = bdrm_ContentSetProperty_eHeaderNotSet;
            break;
        case NEXUS_WmDrmPdContentSetProperty_eV1Header:
            idLocal = bdrm_ContentSetProperty_eV1Header;
            break;
        case NEXUS_WmDrmPdContentSetProperty_eV2Header:
            idLocal = bdrm_ContentSetProperty_eV2Header;
            break;
        case NEXUS_WmDrmPdContentSetProperty_eKID:
            idLocal = bdrm_ContentSetProperty_eKID;
            break;
        case NEXUS_WmDrmPdContentSetProperty_eV2_4Header:
            idLocal = bdrm_ContentSetProperty_eV2_4Header;
            break;
        case NEXUS_WmDrmPdContentSetProperty_eV4Header:
            idLocal = bdrm_ContentSetProperty_eV4Header;
            break;
        case NEXUS_WmDrmPdContentSetProperty_eAutoDetectHeader:
            idLocal = bdrm_ContentSetProperty_eAutoDetectHeader;
            break;
        case NEXUS_WmDrmPdContentSetProperty_ePlayreadyObject:
            idLocal = bdrm_ContentSetProperty_ePlayreadyObj;
            break;
        case NEXUS_WmDrmPdContentSetProperty_eV4_1Header:
            idLocal = bdrm_ContentSetProperty_eV4_1Header;
            break;
        case NEXUS_WmDrmPdContentSetProperty_ePlayreadyObjectWithKID:
            idLocal = bdrm_ContentSetProperty_ePlayreadyObjWithKID;
            break;
        case NEXUS_WmDrmPdContentSetProperty_eHeaderComponents:
            idLocal = bdrm_ContentSetProperty_eHeaderComponents;
            break;
        default:
            idLocal = bdrm_ContentSetProperty_eHeaderNotSet;
            BDBG_ERR(("%s: Could not parse content set property=%x.  Defaulting to bdrm_ContentSetProperty_eHeaderNotSet\n", __FUNCTION__, (uint8_t)id));
            break;
    }

    /* Call Playready API */
    err = bdrm_content_set_property(handle->bdrm, idLocal, pData, dataLength);

    /* Process error code for return */
    if(err != bdrm_err_ok){
        rc = NEXUS_NOT_SUPPORTED;
        BDBG_ERR(("%s: Call failed with return code err=%x. Exiting with error.", __FUNCTION__, err));
    }
    else {
        BDBG_MSG(("%s: Success. Exiting with rc=%x", __FUNCTION__, rc));
    }
    
    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();

    return rc;
}

NEXUS_Error NEXUS_WmDrmPdCore_P_GetCustomLicenseChallenge(
    NEXUS_WmDrmPdCoreHandle handle,
    const void *pCustomData, /* attr{nelem=customDataLength;reserved=128} */
    size_t customDataLength,
    NEXUS_WmDrmPdLicenseChallenge *pChallenge   /* [out] */
    )
{
    bdrm_license_challenge challenge;
    bdrm_err err = bdrm_err_ok;
    
    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle);

    if((err = bdrm_gen_challenge(handle->bdrm, pCustomData, customDataLength, &challenge, NULL)) == bdrm_err_ok)
    {
        BKNI_Memset((void *)(handle->licenseData.pData), 0, DEFAULT_LICENSE_CHALLENGE_LENGTH);
        BKNI_Memcpy((void *)(handle->licenseData.pData), challenge.data, challenge.len);
        handle->licenseData.dataLength = challenge.len;

        if (challenge.url) 
        {
            BKNI_Memset((void *)(handle->licenseData.pUrl), 0, DEFAULT_LICENSE_URL_LENGTH);
            BKNI_Memcpy((void *)(handle->licenseData.pUrl), challenge.url, b_strlen(challenge.url));
        }
        else 
        {
            handle->licenseData.pUrl = NULL;
        }
        
        BKNI_Memcpy((void *)pChallenge, &(handle->licenseData), sizeof(NEXUS_WmDrmPdLicenseChallenge));
        
        /*
        pChallenge = &(handle->licenseData);
        */
        handle->licenseData.pResponseBuffer = pChallenge->pResponseBuffer;

        bdrm_clean_challenge(&challenge);
    }
    else
    {
        BDBG_ERR(("bdrm_gen_challenge failed\n"));
    }

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_WmDrmPdCore_GetLicenseChallenge(
    NEXUS_WmDrmPdCoreHandle handle,
    NEXUS_WmDrmPdLicenseChallenge *pChallenge   /* [out] */
    )
{
    return NEXUS_WmDrmPdCore_P_GetCustomLicenseChallenge(handle, NULL, 0, pChallenge);
}

NEXUS_Error NEXUS_WmDrmPdCore_GetCustomLicenseChallenge(
    NEXUS_WmDrmPdCoreHandle handle,
    const void *pCustomData, /* attr{nelem=customDataLength;reserved=128} */
    size_t customDataLength,
    NEXUS_WmDrmPdLicenseChallenge *pChallenge   /* [out] */
    )
{
    return NEXUS_WmDrmPdCore_P_GetCustomLicenseChallenge(handle, pCustomData, customDataLength, pChallenge);
}

void NEXUS_WmDrmPdCore_GetDefaultAesCounterInfo(
    NEXUS_WmdrmPdAesCounterInfo *pInfo
    )
{
    BKNI_Memset(pInfo, 0, sizeof(NEXUS_WmdrmPdAesCounterInfo));
}


NEXUS_Error NEXUS_WmDrmPdCore_ProcessBlocksAesCounter(
    NEXUS_WmDrmPdCoreHandle handle,
    const NEXUS_WmdrmPdAesCounterInfo *pInfo,
    const NEXUS_DmaJobBlockSettings *pDmaBlocks,
    unsigned nBlocks
    )
{
    NEXUS_Error rc = NEXUS_INVALID_PARAMETER;
    NEXUS_KeySlotHandle key = 0;
    void *pDecryptCtx = NULL;
    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle); 

    if (!NEXUS_P_CpuAccessibleAddress((void *)pDmaBlocks->pSrcAddr))
    {
        BDBG_ERR(("Source buffer address is not CPU accessible."));
        (void)BERR_TRACE(rc);
        return rc;
    }

    if(pDmaBlocks->pSrcAddr != pDmaBlocks->pDestAddr) {
        if (!NEXUS_P_CpuAccessibleAddress(pDmaBlocks->pDestAddr))
        {
            BDBG_ERR(("Destination buffer address is not CPU accessible."));
            (void)BERR_TRACE(rc);
            return rc;
        }
    }

    if(pInfo->decrypt != NULL){
        key = pInfo->decrypt->key;
        pDecryptCtx = pInfo->decrypt->pDecrypt;
    }

    if(bdrm_decrypt_aes_ctr_samples(handle->bdrm, pInfo->nonce, pInfo->blockCounter, pInfo->byteOffset, pDmaBlocks, nBlocks, key, pDecryptCtx) != bdrm_err_ok){
        rc = NEXUS_WMDRMPD_CRYPTOGRAPHIC_ERROR;
    }
    else {
        rc = NEXUS_SUCCESS;
    }

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return rc;
}


NEXUS_Error NEXUS_WmDrmPdCore_P_ProcessLicenseAcquisitionResponse(
    NEXUS_WmDrmPdCoreHandle handle,
    char *pResponse, /* attr{nelem=responseLength;reserved=128} */
    size_t responseLength,  
    uint32_t responseOffset,
    bdrm_license_t license
    )
{
    bdrm_license_response response;
    bdrm_err err = bdrm_err_ok;
    NEXUS_Error rc = NEXUS_SUCCESS;

    if(responseLength)
    {
        response.data = (unsigned char*)(pResponse);
        response.resp_start = (unsigned char*)(pResponse + responseOffset);
        response.len = responseLength;

        if((err = bdrm_feed_license(handle->bdrm, &response, license)) != bdrm_err_ok)
        {
            BDBG_ERR(("bdrm_feed_license failed=0x%x", err));
            rc = NEXUS_WMDRMPD_INVALID_RESPONSE; 
        }
    }

    return rc;

}

NEXUS_Error NEXUS_WmDrmPdCore_LicenseChallengeComplete(
    NEXUS_WmDrmPdCoreHandle handle,
    unsigned responseLength,        /* Response length in bytes */
    unsigned responseOffset         /* Offset to the start of the response within the data buffer */
    )
{
    bdrm_license_t license = NULL;
    bdrm_err err = bdrm_err_ok;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle); 

    rc = NEXUS_WmDrmPdCore_P_ProcessLicenseAcquisitionResponse(handle, handle->licenseData.pResponseBuffer,
        responseLength, responseOffset, license);
    if(rc == NEXUS_SUCCESS)
    {
        if(( err = bdrm_load_license(handle->bdrm, &license)) != bdrm_err_ok)
        {
            if(err == bdrm_err_license_expired)
            {
                BDBG_ERR(("bdrm_load_license failed"));
                rc = NEXUS_WMDRMPD_LICENSE_EXPIRED;
            }
            else if( err == bdrm_err_revocation_package_expired)
            {
                BDBG_ERR(("revocation package expired"));
                rc = NEXUS_WMDRMPD_REVOCATION_PACKAGE;
            }
            else if( err == bdrm_err_domain_join)
            {
                BDBG_ERR(("Domain Join required. This feature is not supported"));
                rc = NEXUS_NOT_SUPPORTED;
            }
        }
        else {
            bdrm_decryptor_assign(handle->bdrm->dcrypt, handle->bdrm);
            bdrm_decryptor_set_encr_state(handle->bdrm->dcrypt, (bdrm_encr_state)handle->encrType);
        }
    }

    if(license) Oem_MemFree(license);
    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return rc;
}

/* Feed license response to sdk. Does not attempt to load license afterward. */
NEXUS_Error NEXUS_WmDrmPdCore_ProcessLicenseAcquisitionResponse(
    NEXUS_WmDrmPdCoreHandle handle,
    const void *pResponse,
    size_t responseLength,
    unsigned responseOffset
    )
{
    bdrm_license_t license = NULL;
    NEXUS_Error rc;
    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle);

    if (!NEXUS_P_CpuAccessibleAddress(pResponse)) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    rc = NEXUS_WmDrmPdCore_P_ProcessLicenseAcquisitionResponse(handle, (char *)pResponse,
        responseLength, responseOffset, license);

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return rc;
}

static NEXUS_Error NEXUS_WmDrmPdCore_P_GetPlayPolicy(
    NEXUS_WmDrmPdPlayPolicy *pDest,
    bdrm_opl_play_t *pSrc
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if(pSrc->i_vop_cEntries > NEXUS_WMDRMPD_MAX_VIDEO_OUTPUT_PROTECTION){
        BDBG_ERR(("Not enough space available in the videoOutputEntries array to copy all the entries. Please increase NEXUS_DRM_MAX_VIDEO_OUTPUT_PROTECTION. The number of entries needed is %d", pSrc->i_vop_cEntries));
        rc = NEXUS_INVALID_PARAMETER;
        return rc;
    }

    if(pSrc->i_aop_cEntries > NEXUS_WMDRMPD_MAX_AUDIO_OUTPUT_PROTECTION){
        BDBG_ERR(("Not enough space available in the audioOutputEntries array to copy all the entries. Please increase NEXUS_DRM_MAX_AUDIO_OUTPUT_PROTECTION. The number of entries needed is %d", pSrc->i_aop_cEntries));
        rc = NEXUS_INVALID_PARAMETER;
        return rc;
    }

    pDest->compressedDigitalVideo = pSrc->i_compressedDigitalVideo;
    pDest->uncompressedDigitalVideo = pSrc->i_uncompressedDigitalVideo;
    pDest->analogVideo = pSrc->i_analogVideo;
    pDest->compressedDigitalAudio = pSrc->i_compressedDigitalAudio;
    pDest->uncompressedDigitalAudio = pSrc->i_uncompressedDigitalAudio;

    pDest->numVideoOutputEntries = (unsigned)pSrc->i_vop_cEntries;
    BKNI_Memcpy(pDest->videoOutputEntries, pSrc->i_vop,  sizeof(NEXUS_WmDrmPdOutProtection) * pDest->numVideoOutputEntries);

    pDest->numAudioOutputEntries = (unsigned)pSrc->i_aop_cEntries;
    BKNI_Memcpy(pDest->audioOutputEntries, pSrc->i_aop,  sizeof(NEXUS_WmDrmPdOutProtection) * pDest->numAudioOutputEntries);

    return rc;
}


static NEXUS_Error NEXUS_WmDrmPdCore_P_GetCopyPolicy(
    NEXUS_WmDrmPdCopyPolicy *pDest,
    bdrm_opl_copy_t *pSrc
    )
{

    if(pSrc->i_includes_cIds > NEXUS_WMDRMPD_MAX_INCLUSION_GUIDS){
        BDBG_ERR(("Not enough space available in the inclusionIds array to copy all the entries. Please increase NEXUS_MAX_INCLUSION_GUIDS. The number of entries needed is %d", pSrc->i_includes_cIds));
        return NEXUS_INVALID_PARAMETER;
    }

    if(pSrc->i_excludes_cIds > NEXUS_WMDRMPD_MAX_EXCLUSION_GUIDS){
        BDBG_ERR(("Not enough space available in the exclusionIds array to copy all the entries. Please increase NEXUS_MAX_EXCLUSION_GUIDS. The number of entries needed is %d", pSrc->i_excludes_cIds));
        return NEXUS_INVALID_PARAMETER;
    }

    pDest->minimumCopyLevel = pSrc->i_minimumCopyLevel;

    pDest->numInclusionIds = pSrc->i_includes_cIds;
    BKNI_Memcpy(pDest->inclusionIds, pSrc->i_includes_rgIds,  sizeof(NEXUS_WmDrmPdGuid) * pDest->numInclusionIds);

    pDest->numExclusionIds = pSrc->i_excludes_cIds;
    BKNI_Memcpy(pDest->exclusionIds, pSrc->i_excludes_rgIds,  sizeof(NEXUS_WmDrmPdGuid) * pDest->numExclusionIds);

    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_WmDrmPdCore_P_GetInclusionListPolicy(
    NEXUS_WmDrmPdInclusionList *pDest,
    bdrm_inclusion_list_t *pSrc
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if(BDRM_MAX_INCLUSION_GUIDS > NEXUS_WMDRMPD_MAX_INCLUSION_GUIDS){
        BDBG_ERR(("Not enough space available in inclusionList array to copy all the entries. Please increase NEXUS_MAX_INCLUSION_GUIDS. The number of entries is %d", BDRM_MAX_INCLUSION_GUIDS));
        rc =  NEXUS_INVALID_PARAMETER;
    }
    else {
        pDest->chainDepth = pSrc->dwChainDepth;

        BKNI_Memcpy(pDest->inclusionList, pSrc->rgInclusionList, sizeof(pSrc->rgInclusionList));
        BKNI_Memcpy(pDest->inclusionListValid, pSrc->rgfInclusionListValid, sizeof(pSrc->rgfInclusionListValid));
    }

    return rc;
}

NEXUS_Error NEXUS_WmDrmPdCore_GetPolicyStatus(
    NEXUS_WmDrmPdCoreHandle handle,
    NEXUS_WmDrmPdPolicyStatus *pStatus  /* [out] */
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle); 

    if(handle->nbOfPolicyStacked > 0){
        handle->nbOfPolicyStacked--;

        pStatus->policyType = handle->policy[handle->nbOfPolicyStacked].type;
        switch(pStatus->policyType){
            case PLAY_OPL:
                BDBG_MSG(("PLAY_OPL policy detected\n"));
                rc = NEXUS_WmDrmPdCore_P_GetPlayPolicy(&pStatus->policy.play, &handle->policy[handle->nbOfPolicyStacked].t.play);
                break;
            case COPY_OPL:
                BDBG_MSG(("COPY_OPL policy detected\n"));
                rc = NEXUS_WmDrmPdCore_P_GetCopyPolicy(&pStatus->policy.copy, &handle->policy[handle->nbOfPolicyStacked].t.copy);
                break;
            case INCLUSION_LIST:
                BDBG_MSG(("INCLUSION_LIST policy detected\n"));
                rc = NEXUS_WmDrmPdCore_P_GetInclusionListPolicy(&pStatus->policy.inclusionList, &handle->policy[handle->nbOfPolicyStacked].t.inc_list);
                break;
            case EXTENDED_RESTRICTION_CONDITION:
            case EXTENDED_RESTRICTION_ACTION:
            case EXTENDED_RESTRICTION_QUERY:
            case SECURE_STATE_TOKEN_RESOLVE:
            default:
                BDBG_ERR(("%s - policy_type %d NOT IMPLEMENTED\n", __FUNCTION__, pStatus->policyType));
                rc = NEXUS_NOT_SUPPORTED;
                break;
        }
    }

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return rc;
}

void NEXUS_WmDrmPdCore_CleanupLicenseStore(
    NEXUS_WmDrmPdCoreHandle handle
    )
{
    bdrm_err err;
    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle); 

    if((err = bdrm_cleanup_license_store(handle->bdrm)) != bdrm_err_ok)
    {
        BDBG_ERR(("bdrm_cleanup_hds failed=0x%x", err));
    }

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return;
}

NEXUS_Error NEXUS_WmDrmPdCore_StoreRevocationPackage(
    NEXUS_WmDrmPdCoreHandle handle,
    const void *pPackage,   /* attr{nelem=dataLength} */
    size_t dataLength
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle); 

    if(bdrm_store_revocation_package(handle->bdrm, pPackage, dataLength) != bdrm_err_ok){
        
        rc = NEXUS_INVALID_PARAMETER;
    }

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return rc;
}

NEXUS_Error NEXUS_WmDrmPdCore_GetRequiredBufferSize(
    NEXUS_WmDrmPdCoreHandle handle,
    NEXUS_WmDrmPdBufferId id, 
    size_t *dataLength   /* [out] */
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle); 

    switch(id)
    {
        case NEXUS_WmDrmPdBufferId_eContentPropertyHeaderKid:
            if(bdrm_get_contentProperty(handle->bdrm, bdrm_eContentPropertyHeaderKid, NULL, dataLength) != bdrm_err_ok)
                rc = NEXUS_INVALID_PARAMETER;
            break;
        case NEXUS_WmDrmPdBufferId_eContentPropertyHeaderType:
            if(bdrm_get_contentProperty(handle->bdrm, bdrm_eContentPropertyHeaderType, NULL, dataLength) != bdrm_err_ok)
                rc = NEXUS_INVALID_PARAMETER;
            break;
        case NEXUS_WmDrmPdBufferId_eContentPropertyHeader:
            if(bdrm_get_contentProperty(handle->bdrm, bdrm_eContentPropertyHeader, NULL, dataLength) != bdrm_err_ok)
                rc = NEXUS_INVALID_PARAMETER;
            break;
        case NEXUS_WmDrmPdBufferId_eContentPropertyPlayreadyObject:
            if(bdrm_get_contentProperty(handle->bdrm, bdrm_eContentPropertyPlayreadyObject, NULL, dataLength) != bdrm_err_ok)
                rc = NEXUS_INVALID_PARAMETER;
            break;
        case NEXUS_WmDrmPdBufferId_eContentPropertyCipherType:
            if(bdrm_get_contentProperty(handle->bdrm, bdrm_eContentPropertyCipherType, NULL, dataLength) != bdrm_err_ok)
                rc = NEXUS_INVALID_PARAMETER;
            break;
        case NEXUS_WmDrmPdBufferId_eContentPropertyDecryptorSetup:
            if(bdrm_get_contentProperty(handle->bdrm, bdrm_eContentPropertyDecryptorSetup, NULL, dataLength) != bdrm_err_ok)
                rc = NEXUS_INVALID_PARAMETER;
            break;
        case NEXUS_WmDrmPdBufferId_eDevicePropertyDeviceCertificateMd:
        case NEXUS_WmDrmPdBufferId_eDevicePropertyDeviceCertificatPd:
        case NEXUS_WmDrmPdBufferId_eDevicePropertyClientInformation:
        case NEXUS_WmDrmPdBufferId_eDevicePropertyPlayreadyVersion:
        case NEXUS_WmDrmPdBufferId_eDevicePropertySecurityVersion:
        case NEXUS_WmDrmPdBufferId_eDevicePropertyWmdrmPdVersion:
        default:
            rc = NEXUS_NOT_SUPPORTED;
            break;
    }

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return rc;
}

NEXUS_Error NEXUS_WmDrmPdCore_ContentGetProperty(
    NEXUS_WmDrmPdCoreHandle handle,
    NEXUS_WmDrmPdBufferId id,
    void *pData, /* [out] attr{nelem=dataLength;nelem_out=pDataLengthOut} */
    size_t dataLength,
    size_t *pDataLengthOut /* [out] */
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    bdrm_err err;
    bdrm_content_property_id drm_id = 0xFFFFFFFF;

    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle); 

    switch(id){
        case NEXUS_WmDrmPdBufferId_eContentPropertyHeaderKid:
            drm_id = bdrm_eContentPropertyHeaderKid;
            break;
        case NEXUS_WmDrmPdBufferId_eContentPropertyHeaderType:
            drm_id = bdrm_eContentPropertyHeaderType;
            break;
        case NEXUS_WmDrmPdBufferId_eContentPropertyHeader:
            drm_id = bdrm_eContentPropertyHeader;
            break;
        case NEXUS_WmDrmPdBufferId_eContentPropertyPlayreadyObject:
            drm_id = bdrm_eContentPropertyPlayreadyObject;
            break;
        case NEXUS_WmDrmPdBufferId_eContentPropertyCipherType:
            drm_id = bdrm_eContentPropertyCipherType;
            break;
        case NEXUS_WmDrmPdBufferId_eContentPropertyDecryptorSetup:
            drm_id = bdrm_eContentPropertyDecryptorSetup;
            break;
        default:
            rc = NEXUS_INVALID_PARAMETER;
            break;
    }

    if(rc){
        /* Call Playready API */
        err = bdrm_get_contentProperty(handle->bdrm, drm_id, pData, &dataLength);

        /* Prepare to pass up updated dataLength from Playready API */ 
        *pDataLengthOut = dataLength;
        
        /* Process error code for return */
        if(err != bdrm_err_ok){
            if(err == bdrm_err_xml_not_found)
                rc = NEXUS_WMDRMPD_XML_NOT_FOUND;
            else if(err == bdrm_err_invalid_header)
                rc = NEXUS_WMDRMPD_INVALID_HEADER;
            else 
                rc = NEXUS_INVALID_PARAMETER;
        }
    }

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return rc;
}


NEXUS_Error NEXUS_WmDrmPdCore_Bind(
    NEXUS_WmDrmPdCoreHandle handle,
    NEXUS_WmDrmPdDecryptHandle decryptHandle      /* attr{null_allowed=y} may be NULL for default settings */
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    bdrm_err err;
    NEXUS_KeySlotHandle key = 0;
    void *pDecryptCtx = NULL;

    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle); 

    if(decryptHandle != NULL){
        key = decryptHandle->key;
        pDecryptCtx = decryptHandle->pDecrypt;
    }

    /* Call Playready API */
    err = bdrm_reader_bind(handle->bdrm, key, pDecryptCtx);

    /* Process error code for return */
    if(err != bdrm_err_ok){
        switch(err){
        case bdrm_err_license_expired:
            rc = NEXUS_WMDRMPD_LICENSE_EXPIRED;
            break;
        case bdrm_err_header_not_set:
            rc = NEXUS_WMDRMPD_CONTENT_HEADER_NOT_SET; 
            break;
        case bdrm_err_revocation_package_expired:
            rc = NEXUS_WMDRMPD_REVOCATION_PACKAGE;
            break;
        case bdrm_err_domain_join:
        default:
            rc = NEXUS_NOT_SUPPORTED;
            break;
        }
        BDBG_ERR(("%s: Call failed with return code err=%x. Exiting with error.", __FUNCTION__, err));
    }
    else{
        BDBG_MSG(("%s: Success. Exiting with rc=%x", __FUNCTION__, rc));
    }

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return rc;
}


NEXUS_Error NEXUS_WmDrmPdCore_Commit(
    NEXUS_WmDrmPdCoreHandle handle
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    bdrm_err err, err1, err2;

    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle); 

    /* Call Playready API */
    err = bdrm_reader_commit(handle->bdrm);

    /* Process error code for return */
    if(err != bdrm_err_ok){
        rc = NEXUS_INVALID_PARAMETER;
        BDBG_ERR(("%s: Call failed with return code err=%x. Exiting with error.", __FUNCTION__, err));
    }
    else{
        /* Call Playready API to assign decryptor */
        err1 = bdrm_decryptor_assign(handle->bdrm->dcrypt, handle->bdrm);
        err2 = bdrm_decryptor_set_encr_state(handle->bdrm->dcrypt, (bdrm_encr_state)handle->encrType);
        if( (err1 != bdrm_err_ok) || (err2 != bdrm_err_ok) ){
            rc = NEXUS_INVALID_PARAMETER;
            BDBG_ERR(("%s: Call failed with return codes err1=%x err2=%x. Exiting with error.", __FUNCTION__, err1, err2));
        }
        else{
            BDBG_MSG(("%s: Success. Exiting with rc=%x", __FUNCTION__, rc));
        }
    }

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();

    return rc;
}

void NEXUS_WmDrmPdCore_GetDefaultDecryptSettings(
    NEXUS_WmDrmPdDecryptSettings *pSettings
    )
{
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_WmDrmPdDecryptSettings));
    pSettings->opType = NEXUS_SecurityOperation_eDecrypt;
    pSettings->algType = NEXUS_SecurityAlgorithm_eAes128;
    pSettings->algVariant = NEXUS_SecurityAlgorithmVariant_eCounter;
    pSettings->termMode = NEXUS_SecurityTerminationMode_eClear;
    pSettings->enableExtKey = true;
    pSettings->enableExtIv = true;
    pSettings->aesCounterSize = NEXUS_SecurityAesCounterSize_e64Bits;
}

#ifdef PRDY_ROBUSTNESS
NEXUS_WmDrmPdDecryptHandle NEXUS_WmDrmPdCore_AllocateDecryptContext(
    NEXUS_WmDrmPdCoreHandle handle,
    const NEXUS_WmDrmPdDecryptSettings *pSettings
    )
{
    NEXUS_WmDrmPdDecryptContext *pContext;
    NEXUS_Error rc = NEXUS_SUCCESS;
    CommonCryptoKeyConfigSettings  algSettings;
    NEXUS_SecurityKeySlotSettings  keySlotSettings;
    NEXUS_MemoryAllocationSettings allocSettings;
    uint32_t size;

    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle); 
    
    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    allocSettings.heap = handle->settings.heap;
    rc = NEXUS_Memory_Allocate(sizeof(NEXUS_WmDrmPdDecryptContext), &allocSettings, (void *)(&pContext));
    if (rc)
    {
        (void)BERR_TRACE(rc);
        goto ErrorExit;
    }

    /* Allocate key slot */
    NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    pContext->key = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
    if(pContext == NULL) 
    {
        (void)BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto ErrorExit;
    }

    CommonCrypto_GetDefaultKeyConfigSettings(&algSettings);
    algSettings.keySlot = pContext->key;
    algSettings.settings.opType = pSettings->opType;
    algSettings.settings.algType = pSettings->algType;
    algSettings.settings.algVariant = pSettings->algVariant;
    algSettings.settings.termMode = pSettings->termMode;
    algSettings.settings.enableExtKey = pSettings->enableExtKey;
    algSettings.settings.enableExtIv = pSettings->enableExtIv;
    algSettings.settings.aesCounterSize = pSettings->aesCounterSize;
    algSettings.settings.aesCounterMode = NEXUS_SecurityCounterMode_ePartialBlockInNextPacket;

    /* Configure key slot */
    if(CommonCrypto_LoadKeyConfig(handle->cryptoHandle, &algSettings) != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%s - CommonCrypto_ConfigAlg failed aes ctr\n", __FUNCTION__));
        goto ErrorExit;
    }

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    allocSettings.heap = handle->settings.heap;
    size = bdrm_get_decrypt_context_size();

    rc = NEXUS_Memory_Allocate(size, &allocSettings, (void *)(&pContext->pDecrypt));
    if (rc)
    {
        (void)BERR_TRACE(rc);
        goto ErrorExit;
    }
    BKNI_Memset(pContext->pDecrypt, 0, size);

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return pContext;

ErrorExit:
    if(pContext->key) NEXUS_Security_FreeKeySlot(pContext->key);
    if(pContext) Oem_MemFree(pContext);
    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();
    return NULL;
}

void NEXUS_WmDrmPdCore_FreeDecryptContext(
    NEXUS_WmDrmPdDecryptHandle decryptHandle
    )
{
    if(decryptHandle->pDecrypt)NEXUS_Memory_Free(decryptHandle->pDecrypt);
    if(decryptHandle->key) NEXUS_Security_FreeKeySlot(decryptHandle->key);
    if(decryptHandle)NEXUS_Memory_Free(decryptHandle);
    return;
}
#else
NEXUS_WmDrmPdDecryptHandle NEXUS_WmDrmPdCore_AllocateDecryptContext(
    NEXUS_WmDrmPdCoreHandle handle,
    const NEXUS_WmDrmPdDecryptSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return NULL;
}

void NEXUS_WmDrmPdCore_FreeDecryptContext(
    NEXUS_WmDrmPdDecryptHandle decryptHandle
    )
{
    BSTD_UNUSED(decryptHandle);
    return;
}
#endif

#if PRDY_SDK_2_0
/***************************************************************************
Summary:
Processes response from license server, usually containing a series
of licenses.
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPdCore_ProcessLicenseResponse(
    NEXUS_WmDrmPdCoreHandle handle,
    const void *pData,   /* attr{nelem=dataLength} */
    unsigned dataLength,
    NEXUS_WmDrmPdLicenseResponse *pLicenseResponse,   /* [out] */
    NEXUS_WmDrmPdLicenseAck *pAcks,   /* [out] attr{nelem=maxAcksSupported;nelem_out=pMaxAcksCount} */
    unsigned maxAcksSupported,
    unsigned *pMaxAcksCount   /* [out] */
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    bdrm_err err = bdrm_err_ok;
    int i, j;
    bdrm_processed_license_response *pLicenseResponseLocal;

    /* Limit support for number of NEXUS_WmDrmPdLicenseAcks to stay under parameter size limit of 3KB */
    if (maxAcksSupported > NEXUS_WMDRM_MAX_EXT_LICENSE_ACK) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle);

    /* Prepare local version of output struct based on bdrm_processed_license_response */
    pLicenseResponseLocal = BKNI_Malloc(sizeof(bdrm_processed_license_response));
    BKNI_Memset(pLicenseResponseLocal, 0, sizeof(bdrm_processed_license_response));
    pLicenseResponseLocal->protocolType = unknownProtocol;
    pLicenseResponseLocal->pAcks = (bdrm_license_ack*)pAcks;
    pLicenseResponseLocal->cMaxAcks = *pMaxAcksCount;

    /* Call Playready API */
    err = bdrm_process_response(handle->bdrm, pData, dataLength, pLicenseResponseLocal);

    /* Convert output structs from bdrm_processed_license_response */
    switch(pLicenseResponseLocal->protocolType) {
        case unknownProtocol:
            pLicenseResponse->protocolType = NEXUS_WmDrmPdLicenseProtocolVersion_eUnknown;
            break;
        case v2Protocol:
            pLicenseResponse->protocolType = NEXUS_WmDrmPdLicenseProtocolVersion_eV2;
            break;
        case v3Protocol:
            pLicenseResponse->protocolType = NEXUS_WmDrmPdLicenseProtocolVersion_eV3;
            break;
        default:
            pLicenseResponse->protocolType = NEXUS_WmDrmPdLicenseProtocolVersion_eUnknown;
            BDBG_ERR(("%s: Could not parse license protocol=%x.  Defaulting to NEXUS_WmDrmPdLicenseProtocolVersion_eUnknown\n", __FUNCTION__, (uint8_t)pLicenseResponseLocal->protocolType));
            break;
    }

    for (i = 0; i<NEXUS_WMDRM_MAX_TRANSACTION_ID; i++) {
        pLicenseResponse->transactionID[i] = pLicenseResponseLocal->rgbTransactionID[i];
    }

    pLicenseResponse->transactionIDLength = pLicenseResponseLocal->cbTransactionID;

    for (i = 0; i<NEXUS_WMDRM_MAX_LICENSE_ACK; i++) {
        for (j = 0; j<NEXUS_WMDRM_ID_SIZE; j++) {
            pLicenseResponse->acks[i].kid.id[j] = pLicenseResponseLocal->rgoAcks[i].KID.rgb[j];
            pLicenseResponse->acks[i].lid.id[j] = pLicenseResponseLocal->rgoAcks[i].LID.rgb[j];
        }
        pLicenseResponse->acks[i].result = pLicenseResponseLocal->rgoAcks[i].dwResult;
        pLicenseResponse->acks[i].flags = pLicenseResponseLocal->rgoAcks[i].dwFlags;
    }

    pAcks = (NEXUS_WmDrmPdLicenseAck*)pLicenseResponseLocal->pAcks;
    *pMaxAcksCount = pLicenseResponseLocal->cMaxAcks;
    pLicenseResponse->acksCount = pLicenseResponseLocal->cAcks;
    pLicenseResponse->result = pLicenseResponseLocal->dwResult;

    /* Process error code for return */
    if(err != bdrm_err_ok){
        rc = NEXUS_NOT_SUPPORTED;
        BDBG_ERR(("%s: Call failed with return code err=%x. Exiting with error.", __FUNCTION__, err));
    }
    else {
        BDBG_MSG(("%s: Success. Exiting with rc=%x", __FUNCTION__, rc));
    }

    BKNI_Free(pLicenseResponseLocal);

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();

    return rc;
}

NEXUS_Error NEXUS_WmDrmPdCore_ProcessLicenseAckResponse(
    NEXUS_WmDrmPdCoreHandle handle,
    const void *pResponse,      /* attr{nelem=responseLength} */
    size_t responseLength
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    bdrm_err err = bdrm_err_ok;

    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle);

    /* Call Playready API */
    err = bdrm_process_license_ack_response(handle->bdrm, pResponse, responseLength);

    /* Process error code for return */
    if(err != bdrm_err_ok){
        rc = NEXUS_NOT_SUPPORTED;
        BDBG_ERR(("%s: Call failed with return code err=%x. Exiting with error.", __FUNCTION__, err));
    }
    else {
        BDBG_MSG(("%s: Success. Exiting with rc=%x", __FUNCTION__, rc));
    }

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();

    return rc;
}

NEXUS_Error NEXUS_WmDrmPdCore_GetRequiredBufferSizeForLicenseChallengeAck(
    NEXUS_WmDrmPdCoreHandle handle,
    const NEXUS_WmDrmPdLicenseResponse *pLicenseResponse,
    const NEXUS_WmDrmPdLicenseAck *pAcks,  /* attr{nelem=maxAcksCount} */
    unsigned maxAcksCount,
    unsigned *pChallengeLength   /* [out] */
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    bdrm_err err = bdrm_err_ok;
    int i, j;
    bdrm_processed_license_response *pLicenseResponseLocal;

    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle);

    /* Limit support for number of NEXUS_WmDrmPdLicenseAcks to stay under parameter size limit of 3KB */
    if ( maxAcksCount > NEXUS_WMDRM_MAX_EXT_LICENSE_ACK )
    {
        BDBG_ERR(("%s: maxAcksCount=%x indicates too many NEXUS_WmDrmPdLicenseAcks.  Exiting with error.\n", __FUNCTION__, (uint8_t)maxAcksCount));
        rc = NEXUS_INVALID_PARAMETER;
        return rc;
    }

    /* Map license response from NEXUS_WmDrmPdLicenseResponse to bdrm_processed_license_response */
    pLicenseResponseLocal = BKNI_Malloc(sizeof(bdrm_processed_license_response));
    BKNI_Memset(pLicenseResponseLocal, 0, sizeof(bdrm_processed_license_response));

    switch(pLicenseResponse->protocolType) {
        case NEXUS_WmDrmPdLicenseProtocolVersion_eUnknown:
            pLicenseResponseLocal->protocolType = unknownProtocol;
            break;
        case NEXUS_WmDrmPdLicenseProtocolVersion_eV2:
            pLicenseResponseLocal->protocolType = v2Protocol;
            break;
        case NEXUS_WmDrmPdLicenseProtocolVersion_eV3:
            pLicenseResponseLocal->protocolType = v3Protocol;
            break;
        default:
            pLicenseResponseLocal->protocolType = eUnknownProtocol;
            BDBG_ERR(("%s: Could not parse license protocol=%x.  Defaulting to NEXUS_WmDrmPdLicenseProtocolVersion_eUnknown\n", __FUNCTION__, (uint8_t)pLicenseResponse->protocolType));
            break;
    }

    for (i = 0; i<NEXUS_WMDRM_MAX_TRANSACTION_ID; i++) {
        pLicenseResponseLocal->rgbTransactionID[i] = pLicenseResponse->transactionID[i];
    }

    pLicenseResponseLocal->cbTransactionID = pLicenseResponse->transactionIDLength;

    for (i = 0; i<NEXUS_WMDRM_MAX_LICENSE_ACK; i++) {
        for (j = 0; j<NEXUS_WMDRM_ID_SIZE; j++) {
            pLicenseResponseLocal->rgoAcks[i].KID.rgb[j] = pLicenseResponse->acks[i].kid.id[j];
            pLicenseResponseLocal->rgoAcks[i].LID.rgb[j] = pLicenseResponse->acks[i].lid.id[j];
        }
        pLicenseResponseLocal->rgoAcks[i].dwResult = pLicenseResponse->acks[i].result;
        pLicenseResponseLocal->rgoAcks[i].dwFlags = pLicenseResponse->acks[i].flags;
    }

    pLicenseResponseLocal->pAcks = (bdrm_license_ack*)pAcks;
    pLicenseResponseLocal->cMaxAcks = maxAcksCount;
    pLicenseResponseLocal->cAcks = pLicenseResponse->acksCount;
    pLicenseResponseLocal->dwResult = pLicenseResponse->result;

    /* Call Playready API */
    err = bdrm_lic_acquire_generate_ack(handle->bdrm, pLicenseResponseLocal, (int8_t*)NULL, pChallengeLength);

    /* Process error code for return */
    if(err != bdrm_err_buffer_size){
        rc = NEXUS_UNKNOWN;
        BDBG_ERR(("%s: Call failed with return code err=%x. Exiting with error.", __FUNCTION__, err));
    }
    else {
        BDBG_MSG(("%s: Success. Exiting with rc=%x", __FUNCTION__, rc));
    }

    BKNI_Free(pLicenseResponseLocal);

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();

    return rc;
}

NEXUS_Error NEXUS_WmDrmPdCore_LicenseAcquisitionGenerateChallengeAck(
    NEXUS_WmDrmPdCoreHandle handle,
    const NEXUS_WmDrmPdLicenseResponse *pLicenseResponse,
    const NEXUS_WmDrmPdLicenseAck *pAcks,  /* attr{nelem=maxAcksCount} */
    unsigned maxAcksCount,
    void* pChallenge,   /* [out] attr{nelem=challengeLength;nelem_out=pChallengeLengthOut} */
    unsigned challengeLength,
    unsigned *pChallengeLengthOut /* [out] */
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    bdrm_err err = bdrm_err_ok;
    int i, j;
    bdrm_processed_license_response *pLicenseResponseLocal;

    NEXUS_WMDRMPD_SET_ACTIVE_IO(handle->settings.ioHandle);

    /* Limit support for number of NEXUS_WmDrmPdLicenseAcks to stay under parameter size limit of 3KB */
    if ( maxAcksCount > NEXUS_WMDRM_MAX_EXT_LICENSE_ACK )
    {
        BDBG_ERR(("%s: maxAcksCount=%x indicates too many NEXUS_WmDrmPdLicenseAcks.  Exiting with error.\n", __FUNCTION__, (uint8_t)maxAcksCount));
        rc = NEXUS_INVALID_PARAMETER;
        return rc;
    }

    /* Map license response from NEXUS_WmDrmPdLicenseResponse to bdrm_processed_license_response */
    pLicenseResponseLocal = BKNI_Malloc(sizeof(bdrm_processed_license_response));
    BKNI_Memset(pLicenseResponseLocal, 0, sizeof(bdrm_processed_license_response));

    switch(pLicenseResponse->protocolType) {
        case NEXUS_WmDrmPdLicenseProtocolVersion_eUnknown:
            pLicenseResponseLocal->protocolType = unknownProtocol;
            break;
        case NEXUS_WmDrmPdLicenseProtocolVersion_eV2:
            pLicenseResponseLocal->protocolType = v2Protocol;
            break;
        case NEXUS_WmDrmPdLicenseProtocolVersion_eV3:
            pLicenseResponseLocal->protocolType = v3Protocol;
            break;
        default:
            pLicenseResponseLocal->protocolType = eUnknownProtocol;
            BDBG_ERR(("%s: Could not parse license protocol=%x.  Defaulting to NEXUS_WmDrmPdLicenseProtocolVersion_eUnknown\n", __FUNCTION__, (uint8_t)pLicenseResponse->protocolType));
            break;
    }

    for (i = 0; i<NEXUS_WMDRM_MAX_TRANSACTION_ID; i++) {
        pLicenseResponseLocal->rgbTransactionID[i] = pLicenseResponse->transactionID[i];
    }

    pLicenseResponseLocal->cbTransactionID = pLicenseResponse->transactionIDLength;

    for (i = 0; i<NEXUS_WMDRM_MAX_LICENSE_ACK; i++) {
        for (j = 0; j<NEXUS_WMDRM_ID_SIZE; j++) {
            pLicenseResponseLocal->rgoAcks[i].KID.rgb[j] = pLicenseResponse->acks[i].kid.id[j];
            pLicenseResponseLocal->rgoAcks[i].LID.rgb[j] = pLicenseResponse->acks[i].lid.id[j];
        }
        pLicenseResponseLocal->rgoAcks[i].dwResult = pLicenseResponse->acks[i].result;
        pLicenseResponseLocal->rgoAcks[i].dwFlags = pLicenseResponse->acks[i].flags;
    }

    pLicenseResponseLocal->pAcks = (bdrm_license_ack*)pAcks;
    pLicenseResponseLocal->cMaxAcks = maxAcksCount;
    pLicenseResponseLocal->cAcks = pLicenseResponse->acksCount;
    pLicenseResponseLocal->dwResult = pLicenseResponse->result;

    /* Call Playready API */
    err = bdrm_lic_acquire_generate_ack(handle->bdrm, pLicenseResponseLocal, pChallenge, &challengeLength);

    /* Prepare to pass up updated challengeLength from Playready API */
    *pChallengeLengthOut = challengeLength;

    /* Process error code for return */
    if(err != bdrm_err_ok){
        rc = NEXUS_UNKNOWN;
        BDBG_ERR(("%s: Call failed with return code err=%x. Exiting with error.", __FUNCTION__, err));
    }
    else {
        BDBG_MSG(("%s: Success. Exiting with rc=%x", __FUNCTION__, rc));
    }

    BKNI_Free(pLicenseResponseLocal);

    NEXUS_WMDRMPD_CLEAR_ACTIVE_IO();

    return rc;
}

#else
NEXUS_Error NEXUS_WmDrmPdCore_ProcessLicenseResponse(
    NEXUS_WmDrmPdCoreHandle handle,
    const void *pData,   /* attr{nelem=dataLength} */
    unsigned dataLength,
    NEXUS_WmDrmPdLicenseResponse *pLicenseResponse,   /* [out] */
    NEXUS_WmDrmPdLicenseAck *pAcks,   /* [out] attr{nelem=maxAcksSupported;nelem_out=pMaxAcksCount} */
    unsigned maxAcksSupported,
    unsigned *pMaxAcksCount   /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pData);
    BSTD_UNUSED(dataLength);
    BSTD_UNUSED(pLicenseResponse);
    BSTD_UNUSED(pAcks);
    BSTD_UNUSED(maxAcksSupported);
    BSTD_UNUSED(pMaxAcksCount);
    return NEXUS_NOT_SUPPORTED;
}

NEXUS_Error NEXUS_WmDrmPdCore_ProcessLicenseAckResponse(
    NEXUS_WmDrmPdCoreHandle handle,
    const void *pResponse,      /* attr{nelem=responseLength} */
    size_t responseLength
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pResponse);
    BSTD_UNUSED(responseLength);
    return NEXUS_NOT_SUPPORTED;
}

NEXUS_Error NEXUS_WmDrmPdCore_GetRequiredBufferSizeForLicenseChallengeAck(
    NEXUS_WmDrmPdCoreHandle handle,
    const NEXUS_WmDrmPdLicenseResponse *pLicenseResponse,
    const NEXUS_WmDrmPdLicenseAck *pAcks,  /* attr{nelem=maxAcksCount} */
    unsigned maxAcksCount,
    unsigned *pChallengeLength   /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pLicenseResponse);
    BSTD_UNUSED(pAcks);
    BSTD_UNUSED(maxAcksCount);
    BSTD_UNUSED(pChallengeLength);
    return NEXUS_NOT_SUPPORTED;
}

NEXUS_Error NEXUS_WmDrmPdCore_LicenseAcquisitionGenerateChallengeAck(
    NEXUS_WmDrmPdCoreHandle handle,
    const NEXUS_WmDrmPdLicenseResponse *pLicenseResponse,
    const NEXUS_WmDrmPdLicenseAck *pAcks,  /* attr{nelem=maxAcksCount} */
    unsigned maxAcksCount,
    void* pChallenge,   /* [out] attr{nelem=challengeLength;nelem_out=pChallengeLengthOut} */
    unsigned challengeLength,
    unsigned *pChallengeLengthOut /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pLicenseResponse);
    BSTD_UNUSED(pAcks);
    BSTD_UNUSED(maxAcksCount);
    BSTD_UNUSED(pChallenge);
    BSTD_UNUSED(challengeLength);
    BSTD_UNUSED(pChallengeLengthOut);
    return NEXUS_NOT_SUPPORTED;
}
#endif
