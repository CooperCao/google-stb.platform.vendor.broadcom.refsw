/***************************************************************************
 *     (c)2007-2011 Broadcom Corporation
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
 * Module Description:
 * DRM Service shall encapsulate a key slot and operations associated 
   with a key slot. 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#ifndef _B_DVR_DRMSERVICE_H
#define _B_DVR_DRMSERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "b_dvr_const.h"
#include "b_dvr_datatypes.h"
BDBG_OBJECT_ID_DECLARE(B_DVR_DRMService);
#define DRM_DEBUG

/*************************************************************************************************
Summary: 
B_DVR_DRMService_KeyLoaderCallback shall be application provided callback for keyloading.
**************************************************************************************************/
typedef void (*B_DVR_DRMService_KeyLoaderCallback)(NEXUS_KeySlotHandle keySlotHandle);

/****************************************************************************************************
Summary:
B_DVR_DRMServiceRequest shall be an input parameter for B_DVR_DRMService_Open
*****************************************************************************************************/
typedef struct B_DVR_DRMServiceRequest
{
    B_DVR_DRMServiceType drmServiceType;
    B_DVR_DRMServiceKeyType keyType;
    B_DVR_Service service;
    NEXUS_SecurityEngine securityEngine;
    NEXUS_KeySlotHandle keySlot;
    NEXUS_DmaHandle dma;
    B_DVR_DRMService_KeyLoaderCallback controlWordKeyLoader;
    B_DVR_DRMService_KeyLoaderCallback sessionKeyLoader;
    B_DVR_DRMService_KeyLoaderCallback ivKeyLoader; 
}B_DVR_DRMServiceRequest;

/*************************************************************************************************
Summary:
B_DVR_DRMServiceSettings shall have settings for a DRM service instance
**************************************************************************************************/

typedef struct B_DVR_DRMServiceSettings
{
    unsigned char *keys;
    unsigned char *ivKeys;
    unsigned char *sessionKeys;
    unsigned keyLength;
    #if DRM_SUPPORT
    NEXUS_SecurityAlgorithm drmAlgoType;
    NEXUS_SecurityAlgorithmConfigDestination drmDest;
    NEXUS_SecurityAlgorithmScPolarity scPolarity;
    NEXUS_SecurityOperation operation;
    #endif
}B_DVR_DRMServiceSettings;

/*************************************************************************************************
Summary:
B_DVR_DRMService shall all information required for a DRM Service instance
**************************************************************************************************/
struct B_DVR_DRMService{
  BDBG_OBJECT(B_DVR_DRMService)
  unsigned index;
  NEXUS_KeySlotHandle keySlot;
  NEXUS_DmaHandle dma;
  NEXUS_DmaJobHandle dmaJob;
  B_DVR_DRMServiceRequest drmServiceRequest;
  B_DVR_DRMServiceSettings drmServiceSettings;
  B_DVR_DRMService_KeyLoaderCallback vendorSpecificKeyloader;
  B_MutexHandle drmMutex;
  bool internalAssignedKeyslot; 
  BKNI_EventHandle dmaEvent;
 };

/*************************************************************************************************
Summary:
B_DVR_DRMService_BufferInfo  shall stream bufferInfo and m2m complete event required for home networking enc/dec 
**************************************************************************************************/
typedef struct B_DVR_DRMServiceStreamBufferInfo
{
    unsigned char  *streamBuf;  
    unsigned int  streamBufLen;
}B_DVR_DRMServiceStreamBufferInfo;

/*****************************************************************************
Summary:
B_DVR_DRMService_Open shall  open an instance of a DRM service.
Param[in]
drmServiceRequest - drmService request parameters.
return value
B_DVR_DRMServiceHandle - Handle for DRM service instance.
*****************************************************************************/
B_DVR_DRMServiceHandle B_DVR_DRMService_Open(
    B_DVR_DRMServiceRequest *drmServiceRequest);

/******************************************************************************** 
Summary:
B_DVR_DRMService_Close shall de-allocate all the key slots allocated per acquiring 
service and close an instance of a DRM service
Param[in]
drmService - Handle for a DRM service instance.
return value
B_DVR_ERROR - error code returned.
*********************************************************************************/
B_DVR_ERROR B_DVR_DRMService_Close(
    B_DVR_DRMServiceHandle drmService);

/*******************************************************************************
Summary:
B_DVR_DRMService_ConfigureKeySlot shall configure a DRM service instance.
Param[in] 
drmService - Handle for a DRM service instance.
Param[in]
drmServiceSettings - settings
return value
B_DVR_ERROR - Error code returned
*******************************************************************************/
B_DVR_ERROR B_DVR_DRMService_Configure(
    B_DVR_DRMServiceHandle drmService,
    B_DVR_DRMServiceSettings   *drmServiceSettings);


/******************************************************************************
Summary:
B_DVR_DRMService_AddPidChannel shall associate a DRM Service to a nexus 
provide pid Channel for a PID.
Param[in] 
drmService - Handle for a DRM service instance.
Param[in]
pid - PID Channel Handle.
return value
B_DVR_ERROR - Error code returned
*******************************************************************************/

B_DVR_ERROR B_DVR_DRMService_AddPidChannel(
    B_DVR_DRMServiceHandle drmService,
    NEXUS_PidChannelHandle pid);

/******************************************************************************
Summary:
B_DVR_DRMService_RemovePidChannel shall disossociate a DRM Service from a nexus 
provided pid Channel for a PID.
Param[in] 
drmService - Handle for a DRM service instance.
Param[in]
pid - PID Channel Handle.
return value
B_DVR_ERROR - Error code returned
*******************************************************************************/

B_DVR_ERROR B_DVR_DRMService_RemovePidChannel(
    B_DVR_DRMServiceHandle drmService,
    NEXUS_PidChannelHandle pid);

/******************************************************************************
Summary:
B_DVR_DRMService_InstallKeyloaderCallback shall install for vendor specific key loader 
Param[in] 
drmService - Handle for a DRM service instance.
Param[in] 
vendorSprcificKeyloaderCallback - Application provided callback.
Param[in] 
keySlotHandle - Keyslot handle.
return value
B_DVR_ERROR - Error code returned
*******************************************************************************/

B_DVR_ERROR B_DVR_DRMService_InstallKeyloaderCallback(
    B_DVR_DRMServiceHandle drmService,
    B_DVR_DRMService_KeyLoaderCallback vendorSprcificKeyloaderCallback,
    NEXUS_KeySlotHandle keySlotHandle);


/******************************************************************************
Summary:
B_DVR_DRMService_RemoveKeyloaderCallback shall remove for vendor specific key loader 
Param[in] 
drmService - Handle for a DRM service instance.
return value
B_DVR_ERROR - Error code returned
*******************************************************************************/

B_DVR_ERROR B_DVR_DRMService_RemoveKeyloaderCallback(
    B_DVR_DRMServiceHandle drmService);

/******************************************************************************
Summary:
B_DVR_DRMService_EncryptData shall encrypt the stream referred to bufferInfo for home network
Param[in] 
drmService - Handle for a DRM service instance.
bufferInfo - stream pointer to be encrypted and event handle for complete callback
return value
B_DVR_ERROR - Error code returned
*******************************************************************************/

B_DVR_ERROR B_DVR_DRMService_EncryptData(
    B_DVR_DRMServiceHandle drmService,
    B_DVR_DRMServiceStreamBufferInfo *pBufferInfo);

/******************************************************************************
Summary:
B_DVR_DRMService_DecryptData shall decrypt the stream referred to bufferInfo for home network
Param[in] 
drmService - Handle for a DRM service instance.
bufferInfo - stream pointer to be decrypted and event handle for complete callback
return value
B_DVR_ERROR - Error code returned
*******************************************************************************/

B_DVR_ERROR B_DVR_DRMService_DecryptData(
    B_DVR_DRMServiceHandle drmService,
    B_DVR_DRMServiceStreamBufferInfo *pBufferInfo);


#ifdef __cplusplus
}
#endif

#endif /*_B_DVR_DRMSERVICE_H */

