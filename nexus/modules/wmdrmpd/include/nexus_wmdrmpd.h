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

#include "nexus_wmdrmpd_types.h"
#include "nexus_wmdrmpd_core.h"
#include "nexus_playpump.h"
#include "nexus_dma.h"
#include "nexus_security.h"

#ifndef NEXUS_WMDRMPD_H_
#define NEXUS_WMDRMPD_H_

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
Summary: 
WMDRMPD Handle
***************************************************************************/
typedef struct NEXUS_WmDrmPdCore *NEXUS_WmDrmPdHandle;

/***************************************************************************
Summary:
WMDRMPD settings.

Description:
Settings provided to create a WMDRMPD instance
***************************************************************************/
typedef struct NEXUS_WmDrmPdSettings
{
    NEXUS_HeapHandle heap;                  /* Optional heap to allocate internal data structures.  Must be CPU accessible. */
    NEXUS_PlaypumpHandle playpump;          /* Playpump handle required for ASF */
    NEXUS_DmaHandle dma;                    /* This parameter is unused and deprecated. */
    NEXUS_TransportType transportType;      /* Transport container used.  ASF and MP4 are supported. */
    NEXUS_CallbackDesc policyCallback;      /* Policy update callback */
} NEXUS_WmDrmPdSettings;

/***************************************************************************
Summary: 
Get WMDRMPD default settings. 
***************************************************************************/
void NEXUS_WmDrmPd_GetDefaultSettings(
    NEXUS_WmDrmPdSettings *pSettings    /* [out] default settings */
    );

/***************************************************************************
Summary: 
Create a WMDRMPD handle. 
***************************************************************************/
NEXUS_WmDrmPdHandle NEXUS_WmDrmPd_Create(
    const NEXUS_WmDrmPdSettings *pSettings  
    );

/***************************************************************************
Summary: 
Destroy a WMDRMPD handle. 
***************************************************************************/
void NEXUS_WmDrmPd_Destroy(
    NEXUS_WmDrmPdHandle handle
    );

/***************************************************************************
Summary: 
Setup a generic keyslot for DRM decryption. 
 
Description: 
This Keyslot should be provided to NEXUS_PlaypumpSettings.securityContext 
to link this DRM context with NEXUS_Playpump. 
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_ConfigureKeySlot(
    NEXUS_WmDrmPdHandle handle,
    NEXUS_KeySlotHandle keySlot
    );

/***************************************************************************
Summary: 
Get Default ASF PSI Object Info
***************************************************************************/
void NEXUS_WmDrmPd_GetDefaultPsiObjectInfo(
    NEXUS_WmDrmPdPsiObjectInfo *pObject     /* [out] */
    );

/***************************************************************************
Summary: 
Set ASF PSI Object
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_SetPsiObject(
    NEXUS_WmDrmPdHandle handle,
    const NEXUS_WmDrmPdPsiObjectInfo *pObjectInfo,
    const void *pData,  /* attr{nelem=dataLength;reserved=128} */
    size_t dataLength
    );

/***************************************************************************
Summary: 
Set ASF Cencr Object
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_SetCencrObject(
    NEXUS_WmDrmPdHandle handle,
    const void *pSecurityData,  /* attr{nelem=securityLength;reserved=128} */
    size_t securityLength,      /* security data length in bytes */
    const void *pProtocolData,  /* attr{nelem=protocolLength;reserved=128} */
    size_t protocolLength,      /* protocol length in bytes */
    const void *pKeyIdData,     /* attr{nelem=keyIdLength;reserved=128} */
    size_t keyIdLength,         /* key id length in bytes */
    const void *pLicenseUrlData,/* attr{nelem=licenseUrlLength;reserved=128} */
    size_t licenseUrlLength     /* license url length in bytes */
    );

/***************************************************************************
Summary: 
Set ASF Xcencr Object
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_SetXcencrObject(
    NEXUS_WmDrmPdHandle handle,
    const void *pData,  /* attr{nelem=dataLength;reserved=128} */
    size_t dataLength   /* extended encr size in bytes */
    );

/***************************************************************************
Summary: 
Get Default ASF Digsign Object Info
***************************************************************************/
void NEXUS_WmDrmPd_GetDefaultDigsignObjectInfo(
    NEXUS_WmDrmPdDigsignObjectInfo *pObject     /* [out] */
    );

/***************************************************************************
Summary: 
Set ASF Digsign Object
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_SetDigsignObject(
    NEXUS_WmDrmPdHandle handle,
    const NEXUS_WmDrmPdDigsignObjectInfo *pInfo,
    const void *pData,  /* attr{nelem=dataLength;reserved=128} */
    size_t dataLength   /* extended encr size in bytes */
    );

/***************************************************************************
Summary: 
Get Default PSSH Box
***************************************************************************/
void NEXUS_WmDrmPd_GetDefaultPsshBoxInfo(
    NEXUS_WmDrmPdMp4PsshBoxInfo *pObject     /* [out] */
    );

/***************************************************************************
Summary: 
Set MP4 PSSH Box
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_SetPsshBox(
    NEXUS_WmDrmPdHandle handle,
    const NEXUS_WmDrmPdMp4PsshBoxInfo *pInfo,
    const void *pData,   /* attr{nelem=dataLength;reserved=128} */
    size_t dataLength
    );

/***************************************************************************
Summary: 
Get Default Protection Scheme Information Box
***************************************************************************/
void NEXUS_WmDrmPd_GetDefaultProtectionSchemeInfo(
    NEXUS_WmDrmPdMp4ProtectionSchemeInfo *pObject     /* [out] */
    );

/***************************************************************************
Summary: 
Set Protection Scheme Information Box for a given track.
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_SetProtectionSchemeBox(
    NEXUS_WmDrmPdHandle handle,
    const NEXUS_WmDrmPdMp4ProtectionSchemeInfo *pInfo
    );

/***************************************************************************
Summary: 
Attempt to Load a DRM License 
 
Description: 
After the file encryption objects have been set with one or more of the 
NEXUS_WmdrmPd_SetXxxxObject() routines, this function will check if a 
license is available in the license store for the content.  If a license 
is available, the routine will return NEXUS_SUCCESS.  If not, the 
routine will return NEXUS_NOT_AVAILABLE and the application will be 
responsible for calling NEXUS_WmDrmPd_GetLicenseChallenge and 
NEXUS_WmDrmPd_LicenseChallengeComplete(). 
 
See Also: 
NEXUS_WmDrmPd_SetPsiObject 
NEXUS_WmDrmPd_SetCencrObject 
NEXUS_WmDrmPd_SetXcencrObject 
NEXUS_WmDrmPd_SetDigsignObject 
NEXUS_WmDrmPd_GetLicenseChallenge 
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_LoadLicense(
    NEXUS_WmDrmPdHandle handle
    );

/***************************************************************************
Summary: 
Request License Challenge Information 
 
Description: 
If an error is returned from NEXUS_WmDrmPd_LoadLicense, this routine should 
be called to get the license challenge information required to acquire 
a new license.  When the server responds with the challenge data, the 
application should call NEXUS_WmDrmPd_LicenseChallengeComplete().  If 
an error occurs (e.g. timeout), the application should call 
NEXUS_WmDrmPd_LicenseChallengeComplete(handle, 0, 0) to report the 
failure. 
 
See Also: 
NEXUS_WmDrmPd_LoadLicense 
NEXUS_WmDrmPd_LicenseChallengeComplete 
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_GetLicenseChallenge(
    NEXUS_WmDrmPdHandle handle,
    NEXUS_WmDrmPdLicenseChallenge *pChallenge   /* [out] */
    );

/***************************************************************************
Summary: 
Request License Challenge Information 
 
Description: 
If an error is returned from NEXUS_WmDrmPd_LoadLicense, this routine should 
be called to get the license challenge information required to acquire 
a new license.

The application can specify custom data to be added to the challenge. If no
custom data is necessary, use NEXUS_WmDrmPd_GetLicenseChallenge() instead.

When the server responds with the challenge data, the 
application should call NEXUS_WmDrmPd_LicenseChallengeComplete().  If 
an error occurs (e.g. timeout), the application should call 
NEXUS_WmDrmPd_LicenseChallengeComplete(handle, 0, 0) to report the 
failure. 
 
See Also: 
NEXUS_WmDrmPd_LoadLicense 
NEXUS_WmDrmPd_LicenseChallengeComplete 
NEXUS_WmDrmPd_GetLicenseChallenge
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_GetCustomLicenseChallenge(
    NEXUS_WmDrmPdHandle handle,
    const void *pCustomData, /* attr{nelem=customDataLength;reserved=128} */
    size_t customDataLength,
    NEXUS_WmDrmPdLicenseChallenge *pChallenge   /* [out] */
    );

/***************************************************************************
Summary: 
Send License Challenge Response. Then, attempts to bind the internal decryption
context to the newly acquired license.
 
Description: 
This call should indicate the response data length and offset to the 
response payload within the NEXUS_WmDrmPdLicenseChallenge.pResponseBuffer 
buffer.  If an error occurred (e.g. timeout), pass an offset and length 
of 0 into this routine. 
 
See Also: 
NEXUS_WmDrmPd_GetLicenseChallenge .
NEXUS_WmDrmPd_ProcessLicenseAcquisitionResponse
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_LicenseChallengeComplete(
    NEXUS_WmDrmPdHandle handle,
    unsigned responseLength,        /* Response length in bytes */
    unsigned responseOffset         /* Offset to the start of the response within the data buffer */
    );


/***************************************************************************
Summary: 
Process a license acquisition response from the server.
The application still needs to bind the license to a decrypt context by
calling NEXUS_WmDrmPd_Bind().
 
See Also: 
NEXUS_WmDrmPd_Bind 
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_ProcessLicenseAcquisitionResponse(
    NEXUS_WmDrmPdHandle handle,
    const void *pResponse, /* attr{nelem=responseLength} */
    size_t responseLength,
    unsigned responseOffset
    );
/***************************************************************************
Summary: 
Get License Policy Status
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_GetPolicyStatus(
    NEXUS_WmDrmPdHandle handle,
    NEXUS_WmDrmPdPolicyStatus *pStatus  /* [out] */
    );

/***************************************************************************
Summary: 
Deletes all expired licenses from the license store and performs maintenance
on the data store file.
***************************************************************************/
void NEXUS_WmDrmPd_CleanupLicenseStore(
    NEXUS_WmDrmPdHandle handle
    );

/***************************************************************************
Summary: 
Get Default ASE counter mode Information.
***************************************************************************/
void NEXUS_WmDrmPd_GetDefaultAesCounterInfo(
    NEXUS_WmdrmPdAesCounterInfo *pInfo
    );

/***************************************************************************
Summary: 
Does an AES-CTR operation on the DMA blocks.
 
Description: 
This function does an AES CTR operation using the Playready content key. 
This function is useful for application models where the data must be decrypted
before is feed to the play pump. Like in the case of Smooth Streaming for example
when the SDK feeds decrypt ES packets to the play pump.

***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_ProcessBlocksAesCounter(
    NEXUS_WmDrmPdHandle handle,
    const NEXUS_WmdrmPdAesCounterInfo *pInfo,
    const NEXUS_DmaJobBlockSettings *pDmaBlocks,  /* attr{nelem=nBlocks;reserved=4} pointer to array of DMA blocks */
    unsigned nBlocks                              /* Must be < NEXUS_DmaJobSettings.numBlocks */
    );

/***************************************************************************
Summary: 
Store revocation package.
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_StoreRevocationPackage(
    NEXUS_WmDrmPdHandle handle,
    const void *pPackage,  /* attr{nelem=dataLength} */
    size_t dataLength      /* Size of the revocation package in bytes. */
    );
/***************************************************************************
Summary: 
Get required size of the buffer that the application needs to allocate on a nexus heap to store
the data.

Description: 
Through out it's Playready SDK, Microsoft  uses a model where 'get' APIs are used for 2 things. 
1) recover some data stored in the SDK. For example a content property.
2) Figure out the size of the buffer the application needs to allocate to store the data as follow.

 uint8_t *pBuf;
 uint32_t dataSize = 0;

 ...
 if(Drm_Content_GetProperty(f_poAppContext, DRM_CGP_PLAYREADY_OBJ, NULL, &dataSize) == DRM_E_BUFFERTOOSMALL)
 {
    NEXUS_Memory_Allocate(dataSize, allocSettings, &pBuf);

    Drm_Content_GetProperty(f_poAppContext, DRM_CGP_PLAYREADY_OBJ, pBuf, &dataSize);
    ...
 }

This works well in a model where a parameter can be used for input and output. This isn't
allowed in nexus. 

As a result this function is provided to handle the size recovery operation.
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_GetRequiredBufferSize(
    NEXUS_WmDrmPdHandle handle,
    NEXUS_WmDrmPdBufferId id, 
    size_t *dataLength   /* [out] */
    );

/***************************************************************************
Summary: 
Get Content Property from the DRM header

See Also:
NEXUS_WmDrmPd_ContentSetProperty
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_ContentGetProperty(
    NEXUS_WmDrmPdHandle handle,
    NEXUS_WmDrmPdBufferId id,
    void *pData, /* [out] attr{nelem=dataLength;nelem_out=pDataLengthOut} */
    size_t dataLength,
    size_t *pDataLengthOut /* [out] */
    );

/***************************************************************************
Summary: 
Sets Content Property used for DRM tasks.

See Also:
NEXUS_WmDrmPd_ContentGetProperty,
NEXUS_WmDrmPd_Bind,
NEXUS_WmDrmPd_Commit
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_ContentSetProperty(
    NEXUS_WmDrmPdHandle handle,
    NEXUS_WmDrmPdContentSetProperty id,
    const void *pData,  /* attr{nelem=dataLength} */
    size_t dataLength
    );

/***************************************************************************
Summary: 
Binds a decrypt handle to valid license. Note that a valid license is license
which match the header set during the call to NEXUS_WmDrmPd_ContentSetProperty().

Note that when decryptHandle is set to NULL. The internal playready decryption 
handle will be used to bind to the license. And no key rotation will be supported 
for that content.

See Also:
NEXUS_WmDrmPd_ContentSetProperty,
NEXUS_WmDrmPd_Bind,
NEXUS_WmDrmPd_Commit
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_Bind(
    NEXUS_WmDrmPdHandle handle,
    NEXUS_WmDrmPdDecryptHandle decryptHandle /* attr{null_allowed=y} may be NULL for default settings */
    );

/***************************************************************************
Summary: 
Commits all the state data to the datastore.

See Also:
NEXUS_WmDrmPd_ContentSetProperty,
NEXUS_WmDrmPd_Bind,
NEXUS_WmDrmPd_Commit
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_Commit(
    NEXUS_WmDrmPdHandle handle
    );


/***************************************************************************
Summary: 
Get default decrypt settings. 
***************************************************************************/
void NEXUS_WmDrmPd_GetDefaultDecryptSettings(
    NEXUS_WmDrmPdDecryptSettings *pSettings
    );



/***************************************************************************
Summary: 
Allocates a decryption context. This context can be used by the application
to support scenarios requiering key rotation. 

See Also:
NEXUS_WmDrmPd_FreeDecryptContext
***************************************************************************/
NEXUS_WmDrmPdDecryptHandle NEXUS_WmDrmPd_AllocateDecryptContext(
    NEXUS_WmDrmPdHandle handle,
    const NEXUS_WmDrmPdDecryptSettings *pSettings
    );

/***************************************************************************
Summary: 
Frees up a decryption context. 

See Also:
NEXUS_WmDrmPd_FreeDecryptContext
***************************************************************************/
void NEXUS_WmDrmPd_FreeDecryptContext(
    NEXUS_WmDrmPdDecryptHandle decryptHandle
    );

/***************************************************************************
Summary:
Processes response from license server, usually containing a series
of licenses.
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_ProcessLicenseResponse(
    NEXUS_WmDrmPdHandle handle,
    const void *pData,   /* attr{nelem=dataLength} */
    unsigned dataLength,
    NEXUS_WmDrmPdLicenseResponse *pLicenseResponse,   /* [out] */
    NEXUS_WmDrmPdLicenseAck *pAcks,   /* [out] attr{nelem=maxAcksSupported;nelem_out=pMaxAcksCount} */
    unsigned maxAcksSupported,
    unsigned *pMaxAcksCount   /* [out] */
    );

/***************************************************************************
Summary:
Processes a license acknowledgement response from the license server.
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_ProcessLicenseAckResponse(
    NEXUS_WmDrmPdHandle handle,
    const void *pResponse,      /* attr{nelem=responseLength} */
    size_t responseLength
    );

/***************************************************************************
Summary:
Get required buffer size for NEXUS_WmDrmPd_LicenseAcquisitionGenerateChallengeAck
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_GetRequiredBufferSizeForLicenseChallengeAck(
    NEXUS_WmDrmPdHandle handle,
    const NEXUS_WmDrmPdLicenseResponse *pLicenseResponse,
    const NEXUS_WmDrmPdLicenseAck *pAcks,  /* attr{nelem=maxAcksCount} */
    unsigned maxAcksCount,
    unsigned *pChallengeLength   /* [out] */
    );

/***************************************************************************
Summary:
Generates a license acquisition acknowledgement XML to be sent to the
to the license server
***************************************************************************/
NEXUS_Error NEXUS_WmDrmPd_LicenseAcquisitionGenerateChallengeAck(
    NEXUS_WmDrmPdHandle handle,
    const NEXUS_WmDrmPdLicenseResponse *pLicenseResponse,
    const NEXUS_WmDrmPdLicenseAck *pAcks,  /* attr{nelem=maxAcksCount} */
    unsigned maxAcksCount,
    void* pChallenge,   /* [out] attr{nelem=challengeLength;nelem_out=pChallengeLengthOut} */
    unsigned challengeLength,
    unsigned *pChallengeLengthOut /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_WMDRMPD_H_ */
