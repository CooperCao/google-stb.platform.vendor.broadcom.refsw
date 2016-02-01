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

#include "nexus_wmdrmpd_module.h"

/* Currently, all DRM routines pass through to the core module. */

void NEXUS_WmDrmPd_GetDefaultSettings(
    NEXUS_WmDrmPdSettings *pSettings    /* [out] default settings */
    )
{
    NEXUS_WmDrmPdCoreSettings coreSettings;

    BDBG_ASSERT(NULL != pSettings);

    BKNI_Memset(pSettings, 0, sizeof(NEXUS_WmDrmPdSettings));

    NEXUS_WmDrmPdCore_GetDefaultSettings(&coreSettings);
    pSettings->heap = coreSettings.heap;
    pSettings->playpump = coreSettings.playpump;
    pSettings->transportType = coreSettings.transportType;
    pSettings->policyCallback = coreSettings.policyCallback;
}

NEXUS_WmDrmPdHandle NEXUS_WmDrmPd_Create(
    const NEXUS_WmDrmPdSettings *pSettings  
    )
{
    NEXUS_WmDrmPdCoreSettings coreSettings;
    NEXUS_WmDrmPdSettings defaults;
    NEXUS_WmDrmPdCoreHandle hCore;

    if ( NULL == pSettings )
    {
        NEXUS_WmDrmPd_GetDefaultSettings(&defaults);
        pSettings = &defaults;
    }

    NEXUS_WmDrmPdCore_GetDefaultSettings(&coreSettings);
    coreSettings.heap = pSettings->heap;
    coreSettings.playpump = pSettings->playpump;
    coreSettings.transportType = pSettings->transportType;
    coreSettings.policyCallback = pSettings->policyCallback;
    coreSettings.ioHandle = nexus_wmdrmpd_p_create_io();
    if(NULL == coreSettings.ioHandle)
    {
        (void)BERR_TRACE(NEXUS_UNKNOWN);
        return NULL;
    }

    hCore = NEXUS_WmDrmPdCore_Create(&coreSettings);
    if ( NULL == hCore )
    {
        nexus_wmdrmpd_p_destroy_io();
        (void)BERR_TRACE(NEXUS_UNKNOWN);
    }
    return hCore;
}

void NEXUS_WmDrmPd_Destroy(
    NEXUS_WmDrmPdHandle handle
    )
{
    NEXUS_WmDrmPdCore_Destroy(handle);
    nexus_wmdrmpd_p_destroy_io();
}

NEXUS_Error NEXUS_WmDrmPd_ConfigureKeySlot(
    NEXUS_WmDrmPdHandle handle,
    NEXUS_KeySlotHandle keySlot
    )
{
    return NEXUS_WmDrmPdCore_ConfigureKeySlot(handle, keySlot);
}

void NEXUS_WmDrmPd_GetDefaultPsiObjectInfo(
    NEXUS_WmDrmPdPsiObjectInfo *pObject     /* [out] */
    )
{
    NEXUS_WmDrmPdCore_GetDefaultPsiObjectInfo(pObject);
}

NEXUS_Error NEXUS_WmDrmPd_SetPsiObject(
    NEXUS_WmDrmPdHandle handle,
    const NEXUS_WmDrmPdPsiObjectInfo *pInfo,
    const void *pData,
    size_t dataLength
    )
{
    return NEXUS_WmDrmPdCore_SetPsiObject(handle, 
                                          pInfo, 
                                          pData, 
                                          dataLength
                                          );
}

void NEXUS_WmDrmPd_GetDefaultPsshBoxInfo(
    NEXUS_WmDrmPdMp4PsshBoxInfo *pObject     /* [out] */
    )
{
    NEXUS_WmDrmPdCore_GetDefaultPsshBoxInfo(pObject);
}

NEXUS_Error NEXUS_WmDrmPd_SetPsshBox(
    NEXUS_WmDrmPdHandle handle,
    const NEXUS_WmDrmPdMp4PsshBoxInfo *pInfo,
    const void *pData,
    size_t dataLength
    )
{
    return NEXUS_WmDrmPdCore_SetPsshBox(handle,
                                        pInfo,
                                        pData,
                                        dataLength);
}


void NEXUS_WmDrmPd_GetDefaultProtectionSchemeInfo(
    NEXUS_WmDrmPdMp4ProtectionSchemeInfo *pObject     /* [out] */
    )
{
    NEXUS_WmDrmPdCore_GetDefaultProtectionSchemeInfo(pObject);
}

NEXUS_Error NEXUS_WmDrmPd_SetProtectionSchemeBox(
    NEXUS_WmDrmPdHandle handle,
    const NEXUS_WmDrmPdMp4ProtectionSchemeInfo *pInfo
    )
{
    return NEXUS_WmDrmPdCore_SetProtectionSchemeBox(handle,
                                                    pInfo);
}

NEXUS_Error NEXUS_WmDrmPd_SetCencrObject(
    NEXUS_WmDrmPdHandle handle,
    const void *pSecurityData,  
    size_t securityLength,      /* security data length in bytes */
    const void *pProtocolData,  
    size_t protocolLength,      /* protocol length in bytes */
    const void *pKeyIdData,     
    size_t keyIdLength,         /* key id length in bytes */
    const void *pLicenseUrlData,
    size_t licenseUrlLength     /* license url length in bytes */
    )
{
    return NEXUS_WmDrmPdCore_SetCencrObject(handle, 
                                            pSecurityData,
                                            securityLength,
                                            pProtocolData,
                                            protocolLength,
                                            pKeyIdData,
                                            keyIdLength,
                                            pLicenseUrlData,
                                            licenseUrlLength
                                            );
}

NEXUS_Error NEXUS_WmDrmPd_SetXcencrObject(
    NEXUS_WmDrmPdHandle handle,
    const void *pData,  
    size_t dataLength   /* extended encr size in bytes */
    )
{
    return NEXUS_WmDrmPdCore_SetXcencrObject(handle, 
                                             pData,
                                             dataLength
                                             );
}

void NEXUS_WmDrmPd_GetDefaultDigsignObjectInfo(
    NEXUS_WmDrmPdDigsignObjectInfo *pObject     /* [out] */
    )
{
    NEXUS_WmDrmPdCore_GetDefaultDigsignObjectInfo(pObject);
}

NEXUS_Error NEXUS_WmDrmPd_SetDigsignObject(
    NEXUS_WmDrmPdHandle handle,
    const NEXUS_WmDrmPdDigsignObjectInfo *pInfo,
    const void *pData, 
    size_t dataLength   /* extended encr size in bytes */
    )
{
    return NEXUS_WmDrmPdCore_SetDigsignObject(handle, 
                                              pInfo,
                                              pData,
                                              dataLength
                                              );
}

NEXUS_Error NEXUS_WmDrmPd_LoadLicense(
    NEXUS_WmDrmPdHandle handle
    )
{
    return NEXUS_WmDrmPdCore_LoadLicense(handle);
}

NEXUS_Error NEXUS_WmDrmPd_GetLicenseChallenge(
    NEXUS_WmDrmPdHandle handle,
    NEXUS_WmDrmPdLicenseChallenge *pChallenge   /* [out] */
    )
{
    return NEXUS_WmDrmPdCore_GetLicenseChallenge(handle, pChallenge);
}

NEXUS_Error NEXUS_WmDrmPd_GetCustomLicenseChallenge(
    NEXUS_WmDrmPdHandle handle,
    const void *pCustomData, /* attr{nelem=customDataLength;reserved=128} */
    size_t customDataLength,
    NEXUS_WmDrmPdLicenseChallenge *pChallenge   /* [out] */
    )
{
    return NEXUS_WmDrmPdCore_GetCustomLicenseChallenge(handle, pCustomData, customDataLength, pChallenge);
}

NEXUS_Error NEXUS_WmDrmPd_LicenseChallengeComplete(
    NEXUS_WmDrmPdHandle handle,
    unsigned responseLength,        /* Response length in bytes */
    unsigned responseOffset         /* Offset to the start of the response within the data buffer */
    )
{
    return NEXUS_WmDrmPdCore_LicenseChallengeComplete(handle, responseLength, responseOffset);
}

NEXUS_Error NEXUS_WmDrmPd_ProcessLicenseAcquisitionResponse(
    NEXUS_WmDrmPdHandle handle,
    const void *pResponse, /* attr{nelem=responseLength} */
    size_t responseLength,  
    unsigned responseOffset
    )
{
    return NEXUS_WmDrmPdCore_ProcessLicenseAcquisitionResponse(handle, pResponse, responseLength, responseOffset);
}

NEXUS_Error NEXUS_WmDrmPd_GetPolicyStatus(
    NEXUS_WmDrmPdHandle handle,
    NEXUS_WmDrmPdPolicyStatus *pStatus  /* [out] */
    )
{
    return NEXUS_WmDrmPdCore_GetPolicyStatus(handle, pStatus);
}


void NEXUS_WmDrmPd_GetDefaultAesCounterInfo(
    NEXUS_WmdrmPdAesCounterInfo *pInfo
    )
{
    NEXUS_WmDrmPdCore_GetDefaultAesCounterInfo(pInfo);
}


NEXUS_Error NEXUS_WmDrmPd_ProcessBlocksAesCounter(
    NEXUS_WmDrmPdHandle handle,
    const NEXUS_WmdrmPdAesCounterInfo *pInfo,
    const NEXUS_DmaJobBlockSettings *pDmaBlocks,
    unsigned nBlocks
    )
{
    return NEXUS_WmDrmPdCore_ProcessBlocksAesCounter(handle, pInfo, pDmaBlocks, nBlocks);
}


void NEXUS_WmDrmPd_CleanupLicenseStore(
    NEXUS_WmDrmPdHandle handle
    )
{
    NEXUS_WmDrmPdCore_CleanupLicenseStore(handle);
    return;
}

NEXUS_Error NEXUS_WmDrmPd_StoreRevocationPackage(
    NEXUS_WmDrmPdHandle handle,
    const void *pPackage,  /* attr{nelem=dataLength} */
    size_t dataLength
    )
{
    return NEXUS_WmDrmPdCore_StoreRevocationPackage(handle, pPackage, dataLength);
}


NEXUS_Error NEXUS_WmDrmPd_GetRequiredBufferSize(
    NEXUS_WmDrmPdHandle handle,
    NEXUS_WmDrmPdBufferId id, 
    size_t *dataLength   /* [out] */
    )
{
    return NEXUS_WmDrmPdCore_GetRequiredBufferSize(handle, id, dataLength);
}


NEXUS_Error NEXUS_WmDrmPd_ContentGetProperty(
    NEXUS_WmDrmPdHandle handle,
    NEXUS_WmDrmPdBufferId id,
    void *pData, /* [out] attr{nelem=dataLength;nelem_out=pDataLengthOut} */
    size_t dataLength,
    size_t *pDataLengthOut /* [out] */
    )
{
    return NEXUS_WmDrmPdCore_ContentGetProperty(handle, id, pData, dataLength, pDataLengthOut);
}


NEXUS_Error NEXUS_WmDrmPd_ContentSetProperty(
    NEXUS_WmDrmPdHandle handle,
    NEXUS_WmDrmPdContentSetProperty id,
    const void *pData,  /* attr{nelem=dataLength} */
    size_t dataLength
    )
{
    return NEXUS_WmDrmPdCore_ContentSetProperty(handle, id, pData, dataLength);
}

NEXUS_Error NEXUS_WmDrmPd_Bind(
    NEXUS_WmDrmPdHandle handle,
    NEXUS_WmDrmPdDecryptHandle decryptHandle /* attr{null_allowed=y} may be NULL for default settings */
    )
{
    return NEXUS_WmDrmPdCore_Bind(handle, decryptHandle);
}


NEXUS_Error NEXUS_WmDrmPd_Commit(
    NEXUS_WmDrmPdHandle handle
    )
{
    return NEXUS_WmDrmPdCore_Commit(handle);
}

void NEXUS_WmDrmPd_GetDefaultDecryptSettings(
    NEXUS_WmDrmPdDecryptSettings *pSettings
    )
{
    NEXUS_WmDrmPdCore_GetDefaultDecryptSettings(pSettings);
    return;
}

NEXUS_WmDrmPdDecryptHandle NEXUS_WmDrmPd_AllocateDecryptContext(
    NEXUS_WmDrmPdHandle handle,
    const NEXUS_WmDrmPdDecryptSettings *pSettings
    )
{
    return NEXUS_WmDrmPdCore_AllocateDecryptContext(handle, pSettings);
}


void NEXUS_WmDrmPd_FreeDecryptContext(
    NEXUS_WmDrmPdDecryptHandle decryptHandle
    )
{
    NEXUS_WmDrmPdCore_FreeDecryptContext(decryptHandle);
    return;
}

NEXUS_Error NEXUS_WmDrmPd_ProcessLicenseResponse(
    NEXUS_WmDrmPdHandle handle,
    const void *pData,   /* attr{nelem=dataLength} */
    unsigned dataLength,
    NEXUS_WmDrmPdLicenseResponse *pLicenseResponse,   /* [out] */
    NEXUS_WmDrmPdLicenseAck *pAcks,   /* [out] attr{nelem=maxAcksSupported;nelem_out=pMaxAcksCount} */
    unsigned maxAcksSupported,
    unsigned *pMaxAcksCount   /* [out] */
    )
{
    return NEXUS_WmDrmPdCore_ProcessLicenseResponse(handle, pData, dataLength, pLicenseResponse, pAcks, maxAcksSupported, pMaxAcksCount);
}

NEXUS_Error NEXUS_WmDrmPd_ProcessLicenseAckResponse(
    NEXUS_WmDrmPdHandle handle,
    const void *pResponse,      /* attr{nelem=responseLength} */
    size_t responseLength
    )
{
    return NEXUS_WmDrmPdCore_ProcessLicenseAckResponse(handle, pResponse, responseLength);
}

NEXUS_Error NEXUS_WmDrmPd_GetRequiredBufferSizeForLicenseChallengeAck(
    NEXUS_WmDrmPdHandle handle,
    const NEXUS_WmDrmPdLicenseResponse *pLicenseResponse,
    const NEXUS_WmDrmPdLicenseAck *pAcks,  /* attr{nelem=maxAcksCount} */
    unsigned maxAcksCount,
    unsigned *pChallengeLength   /* [out] */
    )
{
    return NEXUS_WmDrmPdCore_GetRequiredBufferSizeForLicenseChallengeAck(handle, pLicenseResponse, pAcks, maxAcksCount, pChallengeLength);
}

NEXUS_Error NEXUS_WmDrmPd_LicenseAcquisitionGenerateChallengeAck(
    NEXUS_WmDrmPdHandle handle,
    const NEXUS_WmDrmPdLicenseResponse *pLicenseResponse,
    const NEXUS_WmDrmPdLicenseAck *pAcks,  /* attr{nelem=maxAcksCount} */
    unsigned maxAcksCount,
    void* pChallenge,   /* [out] attr{nelem=challengeLength;nelem_out=pChallengeLengthOut} */
    unsigned challengeLength,
    unsigned *pChallengeLengthOut /* [out] */
    )
{
    return NEXUS_WmDrmPdCore_LicenseAcquisitionGenerateChallengeAck(handle, pLicenseResponse, pAcks, maxAcksCount, pChallenge, challengeLength, pChallengeLengthOut);
}

