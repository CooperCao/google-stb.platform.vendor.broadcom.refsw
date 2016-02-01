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
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_divxdrm_module.h"
#include "priv/nexus_core_security.h"
#include "bdrm_decrypt.h"

BDBG_MODULE(nexus_divxdrm);


typedef struct NEXUS_DivxDrm
{
    NEXUS_OBJECT(NEXUS_DivxDrm);
    NEXUS_DivxDrmCreateSettings createSettings;
    NEXUS_DivxDrmStartSettings startSettings;
    NEXUS_DivxDrmState drmState;
    bdrm_decrypt_t bdrmDecrypt;
    NEXUS_TaskCallbackHandle rentalMessageCallback;
    NEXUS_TaskCallbackHandle outputProtectionCallback;
    NEXUS_TaskCallbackHandle ictCallback;
    NEXUS_KeySlotHandle keySlot;   
    NEXUS_DivxDrmStatus status;    
} NEXUS_DivxDrm;


void NEXUS_DivxDrm_P_RentalCallback(void *cnxt, uint8_t useCount, uint8_t useLimit)
{
    NEXUS_DivxDrmHandle handle = (NEXUS_DivxDrmHandle)cnxt;
    
    NEXUS_OBJECT_ASSERT(NEXUS_DivxDrm, handle);

    handle->status.rental.useCount = (unsigned)useCount;
    handle->status.rental.useLimit = (unsigned)useLimit;
    handle->status.rental.valid = true;
}

void NEXUS_DivxDrm_P_OutputProtectionCallback(void *cnxt, uint8_t macBit, uint8_t cgmsaBit)
{
    NEXUS_DivxDrmHandle handle = (NEXUS_DivxDrmHandle)cnxt;
    
    NEXUS_OBJECT_ASSERT(NEXUS_DivxDrm, handle);

    handle->status.outputProtection.acptbSignal = (unsigned)macBit;
    handle->status.outputProtection.cgmsaSignal = (unsigned)cgmsaBit;
    handle->status.outputProtection.valid = true;
}

void NEXUS_DivxDrm_P_IctCallback(void *cnxt, uint8_t ictSignal)
{
    NEXUS_DivxDrmHandle handle = (NEXUS_DivxDrmHandle)cnxt;
    
    NEXUS_OBJECT_ASSERT(NEXUS_DivxDrm, handle);

    handle->status.ict.ictSignal = (unsigned)ictSignal;   
    handle->status.ict.valid = true;
}

void NEXUS_DivxDrm_GetDefaultCreateSettings(
    NEXUS_DivxDrmCreateSettings *pSettings    
    )
{        
    BDBG_ASSERT(NULL != pSettings);

    BKNI_Memset(pSettings, 0, sizeof(NEXUS_DivxDrmCreateSettings));           
}

NEXUS_DivxDrmHandle NEXUS_DivxDrm_Create(
    const NEXUS_DivxDrmCreateSettings *pSettings  
    )
{    
    NEXUS_DivxDrmCreateSettings defaults;
    NEXUS_DivxDrmHandle hDivxDrm;
    NEXUS_Error errCode;
    bdrm_decrypt_settings drmSettings;
    unsigned i;

    if ( NULL == pSettings )
    {
        NEXUS_DivxDrm_GetDefaultCreateSettings(&defaults);
        pSettings = &defaults;
    }

    hDivxDrm = BKNI_Malloc(sizeof(NEXUS_DivxDrm));
    if ( NULL == hDivxDrm )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }

    BKNI_Memset(hDivxDrm, 0, sizeof(NEXUS_DivxDrm));    
    
    hDivxDrm->createSettings = *pSettings;
    NEXUS_OBJECT_SET(NEXUS_DivxDrm, hDivxDrm);

    bdrm_get_default_setting(&drmSettings);    

    BKNI_Memcpy(drmSettings.hardware_secret, pSettings->drmHardwareSecret, NEXUS_DIVXDRM_HARDWARE_SECRET_LENGTH );
    
    drmSettings.rental_cb = NEXUS_DivxDrm_P_RentalCallback;
    drmSettings.outputProtection_cb = NEXUS_DivxDrm_P_OutputProtectionCallback;
    drmSettings.ict_cb = NEXUS_DivxDrm_P_IctCallback;
    drmSettings.cnxt = hDivxDrm;

    hDivxDrm->bdrmDecrypt = bdrm_create(&drmSettings);
    if(!hDivxDrm->bdrmDecrypt) {
	BDBG_ERR(("Failed to create Drm Lib"));
	errCode = NEXUS_INVALID_PARAMETER;
	(void)BERR_TRACE(errCode);
	goto err_drm;
    }

    for(i=0; i<NEXUS_DIVXDRM_MAX_TRACKS; i++) {
	hDivxDrm->startSettings.track[i].type = NEXUS_PidType_eUnknown;	
    } 
    
    return hDivxDrm;

err_drm:    
    BKNI_Free(hDivxDrm);
err_malloc:
    return NULL;
}

static void NEXUS_DivxDrm_P_Finalizer(
    NEXUS_DivxDrmHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_DivxDrm, handle);

    bdrm_destroy(handle->bdrmDecrypt);
    if(handle->ictCallback)
	NEXUS_TaskCallback_Destroy(handle->ictCallback);
    if(handle->outputProtectionCallback)
	NEXUS_TaskCallback_Destroy(handle->outputProtectionCallback);
    if(handle->rentalMessageCallback)
	NEXUS_TaskCallback_Destroy(handle->rentalMessageCallback);
    
    NEXUS_OBJECT_DESTROY(NEXUS_DivxDrm, handle);
    
    BKNI_Free(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_DivxDrm, NEXUS_DivxDrm_Destroy);

void NEXUS_DivxDrm_GetDrmState(
    NEXUS_DivxDrmHandle handle,
    NEXUS_DivxDrmState *pDrmState     
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_DivxDrm, handle);

    bdrm_get_drm_memory(handle->drmState.fragment1, handle->drmState.fragment2, handle->drmState.fragment3);

    *pDrmState = handle->drmState;
}

void NEXUS_DivxDrm_SetDrmState(
    NEXUS_DivxDrmHandle handle,
    const NEXUS_DivxDrmState *pDrmState    
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_DivxDrm, handle);    

    if(!pDrmState->fragment1Length || !pDrmState->fragment2Length || !pDrmState->fragment3Length) {
	bdrm_set_drm_memory(NULL, NULL, NULL);
    } else {
	bdrm_set_drm_memory((uint8_t *)pDrmState->fragment1, (uint8_t *)pDrmState->fragment2, (uint8_t *)pDrmState->fragment3);
    }

    handle->drmState = *pDrmState;
}

void NEXUS_DivxDrm_GetDefaultStartSettings(
    NEXUS_DivxDrmStartSettings *pSettings    
    )
{
    unsigned i;

    BDBG_ASSERT(NULL != pSettings);

    BKNI_Memset(pSettings, 0, sizeof(NEXUS_DivxDrmStartSettings));  
    
    for(i=0; i<NEXUS_DIVXDRM_MAX_TRACKS; i++) {
	pSettings->track[i].type = NEXUS_PidType_eUnknown;	
    } 
}


NEXUS_Error NEXUS_DivxDrm_Start(
    NEXUS_DivxDrmHandle handle, 
    const NEXUS_DivxDrmStartSettings *pSettings
    )
{
    NEXUS_Error errCode=BERR_SUCCESS;
    bdrm_decrypt_settings drmSettings;
    unsigned i;

    NEXUS_OBJECT_ASSERT(NEXUS_DivxDrm, handle);
    BDBG_ASSERT(pSettings);

    bdrm_get_settings(handle->bdrmDecrypt, &drmSettings);  

    if(pSettings->rentalMessageCallback.callback){
        handle->rentalMessageCallback = NEXUS_TaskCallback_Create(handle, NULL);
        if(handle->rentalMessageCallback == NULL){
            BDBG_ERR(("Failed to create Rental Message Callback"));
            errCode = NEXUS_INVALID_PARAMETER;
            (void)BERR_TRACE(errCode);
            goto err_rental_cb;
        }
        
        NEXUS_TaskCallback_Set(handle->rentalMessageCallback, &pSettings->rentalMessageCallback);        
    }

    if(pSettings->outputProtectionCallback.callback){
        handle->outputProtectionCallback = NEXUS_TaskCallback_Create(handle, NULL);
        if(handle->outputProtectionCallback == NULL){
            BDBG_ERR(("Failed to create Output Protection Callback"));
            errCode = NEXUS_INVALID_PARAMETER;
            (void)BERR_TRACE(errCode);
            goto err_output_cb;
        }
        
        NEXUS_TaskCallback_Set(handle->outputProtectionCallback, &pSettings->outputProtectionCallback);        
    }

    if(pSettings->ictCallback.callback){
        handle->ictCallback = NEXUS_TaskCallback_Create(handle, NULL);
        if(handle->ictCallback == NULL){
            BDBG_ERR(("Failed to create Ict Protection Callback"));
            errCode = NEXUS_INVALID_PARAMETER;
            (void)BERR_TRACE(errCode);
            goto err_ict_cb;
        }
        
        NEXUS_TaskCallback_Set(handle->ictCallback, &pSettings->ictCallback);        
    }

    drmSettings.drm_header_data  = (uint8_t*)pSettings->drmHeaderData;
    for(i=0; i<NEXUS_DIVXDRM_MAX_TRACKS; i++) {
	if(pSettings->track[i].type == NEXUS_PidType_eVideo) {
	    drmSettings.trk_type[i] = bdecrypt_track_type_video;
	} else if(pSettings->track[i].type == NEXUS_PidType_eAudio) {
	    drmSettings.trk_type[i] = bdecrypt_track_type_audio;
	} else {
	    drmSettings.trk_type[i] = bdecrypt_track_type_other;
	    BDBG_WRN(("Unknown Track type"));
	}
    }
    
    errCode = bdrm_set_settings(handle->bdrmDecrypt, &drmSettings);
    if(errCode) { 
	errCode = BERR_TRACE(errCode);
	goto err_settings;
    }
    
    handle->startSettings = *pSettings;
    
    return errCode;

err_settings:
    NEXUS_TaskCallback_Destroy(handle->ictCallback);
err_ict_cb:
    NEXUS_TaskCallback_Destroy(handle->outputProtectionCallback);
err_output_cb:
    NEXUS_TaskCallback_Destroy(handle->rentalMessageCallback);
err_rental_cb:
    
    return errCode;
}

void NEXUS_DivxDrm_Stop(
    NEXUS_DivxDrmHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_DivxDrm, handle);

    bdrm_stop(handle->bdrmDecrypt);
}

NEXUS_Error NEXUS_DivxDrm_EnableDecrypt(
    NEXUS_DivxDrmHandle handle,
    NEXUS_KeySlotHandle keySlot
    )
{
    NEXUS_Error rc;
    
    NEXUS_OBJECT_ASSERT(NEXUS_DivxDrm, handle);
    NEXUS_OBJECT_ASSERT(NEXUS_KeySlot, keySlot);

    if ( keySlot->cryptoEngine != NEXUS_SecurityEngine_eGeneric )
    {
        BDBG_ERR(("A generic keyslot is required for DRM decryption"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    NEXUS_KeySlot_SetTag(keySlot, handle->bdrmDecrypt);

    rc = bdrm_start(handle->bdrmDecrypt, NULL);
    if(rc) {
	bdrm_stop(handle->bdrmDecrypt);
	rc=BERR_TRACE(rc);
	return rc;
    }

    return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_DivxDrm_GetRegistrationCodeString(
    NEXUS_DivxDrmHandle handle,   
    NEXUS_DivxDrmCodeString *pRegistrationCode
    )
{
    uint8_t registrationCodeString[NEXUS_DIVXDRM_REGISTRATION_CODE_LENGTH] = {0};
    NEXUS_Error rc;

    NEXUS_OBJECT_ASSERT(NEXUS_DivxDrm, handle);
    
    BKNI_Memset(pRegistrationCode->codeString, 0, NEXUS_DIVXDRM_REGISTRATION_CODE_LENGTH);
    
    rc = bdrm_get_registration_code(handle->bdrmDecrypt, registrationCodeString);
    if(rc) {	
	rc=BERR_TRACE(rc);
	return rc;
    }
    
    BKNI_Memcpy(pRegistrationCode->codeString, registrationCodeString, NEXUS_DIVXDRM_REGISTRATION_CODE_LENGTH);
    
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_DivxDrm_Deactivate(
    NEXUS_DivxDrmHandle handle,   
    NEXUS_DivxDrmCodeString *pDeactivationCode
    )
{
    uint8_t deactivationCodeString[NEXUS_DIVXDRM_REGISTRATION_CODE_LENGTH] = {0};
    NEXUS_Error rc;

    NEXUS_OBJECT_ASSERT(NEXUS_DivxDrm, handle);
    
    BKNI_Memset(pDeactivationCode->codeString, 0, NEXUS_DIVXDRM_REGISTRATION_CODE_LENGTH);

    rc = bdrm_deactivate_device(handle->bdrmDecrypt, deactivationCodeString);
    if(rc) {	
	rc=BERR_TRACE(rc);
	return rc;
    }
    
    BKNI_Memcpy(pDeactivationCode->codeString, deactivationCodeString, NEXUS_DIVXDRM_REGISTRATION_CODE_LENGTH);
    
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_DivxDrm_Decrypt(
    NEXUS_DivxDrmHandle handle, 
    NEXUS_KeySlotHandle keyslot,
    const void *pData,                 
    size_t length,    
    NEXUS_DivxDrmInfo *pDrmInfo    
    )
    
{
    NEXUS_Error rc;    
    unsigned drmContext;

    BSTD_UNUSED(handle);
    NEXUS_KeySlot_GetTag(keySlot, &drmContext);
    rc = bdrm_decrypt_mkv_payload((bdrm_decrypt_t)drmContext, pData, length, pDrmInfo->ddChunk, pDrmInfo->trackNum);

    return rc;
}

NEXUS_Error NEXUS_DivxDrm_GetStatus(
    NEXUS_DivxDrmHandle handle,  
    NEXUS_DivxDrmStatus *pStatus    
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_DivxDrm, handle);

    *pStatus = handle->status;       
    
    return NEXUS_SUCCESS;
}
