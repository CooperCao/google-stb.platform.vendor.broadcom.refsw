/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "nexus_platform.h"
#include "nexus_memory.h"
#include "nexus_random_number.h"
#include "drm_prdy.h"
#include "drm_metadata.h"
#include "drm_common.h"

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"

#define DUMP_DATA_HEX(string,data,size) {        \
   char tmp[512]= "\0";                          \
   uint32_t i=0, l=strlen(string);               \
   sprintf(tmp,"%s",string);                     \
   while( i<size && l < 512) {                   \
    sprintf(tmp+l," %02x", data[i]); ++i; l+=3;} \
   printf(tmp); printf("\n");                    \
}

static
int gen_random_num( uint32_t numberOfBytes, uint8_t *pRandomBytes)
{
    int rc = 0;
    NEXUS_RandomNumberGenerateSettings settings;
    NEXUS_RandomNumberOutput rngOutput;
    NEXUS_Error nxs_rc = NEXUS_SUCCESS;

    NEXUS_RandomNumber_GetDefaultGenerateSettings(&settings);
    settings.randomNumberSize = numberOfBytes;

    nxs_rc = NEXUS_RandomNumber_Generate(&settings, &rngOutput);
    if( (nxs_rc != NEXUS_SUCCESS) || (rngOutput.size != numberOfBytes) )
    {
        printf("%s - Error generating '%u' random bytes (only '%u' bytes returned) ", BSTD_FUNCTION, numberOfBytes, rngOutput.size);
        rc = -1;
        goto ErrorExit;
    }

    BKNI_Memcpy(pRandomBytes, rngOutput.buffer, numberOfBytes);

ErrorExit:
    return rc;
}

static
DRM_Prdy_Error_e  _testCertificateCallback(
        void                            *pvDataCallbackContext,
        DRM_Prdy_ND_Certificate_Data_t  *pCertificateData,
        const DRM_Prdy_ID_t             *pCustomDataTypeID,
        const uint8_t                   *pbCustomData,
        uint16_t                         cbCustomData )
{
    printf("\t CertificateCallback...\n");
    BSTD_UNUSED(pvDataCallbackContext);
    BSTD_UNUSED(pCertificateData);
    BSTD_UNUSED(pCustomDataTypeID);
    BSTD_UNUSED(pbCustomData);
    BSTD_UNUSED(cbCustomData);
#if 0
    DRM_RESULT           dr     = DRM_SUCCESS;
    DRM_KEYFILE_CONTEXT *pkfCtx = (DRM_KEYFILE_CONTEXT*)f_pvCertificateCallbackContext;
    DRM_BYTE            *pbCert = NULL;
    DRM_DWORD            cbCert = 0;

    /*
    ** This callback is where we could look at custom data to do extra cert / custom data validation.
    ** For test purposes, just make sure the callback receives what it should have received.
    */
    if( pkfCtx != NULL )
    {
        ChkDR( DRM_KF_GetCertificate(
            pkfCtx,
            eKF_CERT_TYPE_PLAYREADY,
            &pbCert,
            &cbCert ) );

        ChkBOOL( f_pCertificateData != NULL, DRM_E_TEST_UNEXPECTED_OUTPUT );

        /* Verify dwType - all scenarios using this test should have a device certificate. */
        ChkBOOL( f_pCertificateData->dwType == DRM_BCERT_CERTTYPE_DEVICE, DRM_E_TEST_UNEXPECTED_OUTPUT );

        /* Verify dwSecurityLevel. */
        ChkBOOL( f_pCertificateData->dwSecurityLevel == 150, DRM_E_TEST_UNEXPECTED_OUTPUT );

        /* Don't verify anything else. We don't know what values they will take. */
    }
    else
    {
        ChkBOOL( f_pCertificateData == NULL, DRM_E_TEST_UNEXPECTED_OUTPUT );
    }

    ChkBOOL( f_pCustomDataTypeID != NULL,                                                                             DRM_E_TEST_UNEXPECTED_OUTPUT );
    ChkBOOL( 0                   == DRMCRT_memcmp( f_pCustomDataTypeID, &g_customDataTypeID, SIZEOF(DRM_ID)),         DRM_E_TEST_UNEXPECTED_OUTPUT );
    ChkBOOL( f_cbCustomData      == g_cbCustomData,                                                                   DRM_E_TEST_UNEXPECTED_OUTPUT );
    ChkBOOL( 0                   == DRMCRT_memcmp( f_pbCustomData, g_rgbCustomData, f_cbCustomData ),                 DRM_E_TEST_UNEXPECTED_OUTPUT );

    if ( g_fFailCertCallback )
    {
        ChkDR( DRM_E_BCERT_VERIFICATION_ERRORS );
    }

ErrorExit:
    return dr;
#endif
    return DRM_Prdy_ok;
}

static
DRM_Prdy_Error_e _contentIdentifier_CallBack(
        void                                    *pvContentIdentifierCallbackContext,
        DRM_Prdy_ND_Content_Identifier_Type_e    eContentIdentifierType,
        const uint8_t                           *pbContentIdentifier,
        uint16_t                                 cbContentIdentifier,
        Drm_Prdy_KID_t                          *pKidContent )
{
    Drm_Prdy_KID_t      *reqKID = NULL;

    BSTD_UNUSED(pbContentIdentifier);
    BSTD_UNUSED(cbContentIdentifier);

    if(pvContentIdentifierCallbackContext == NULL) {
        printf("[FAILED] %s %d - failed to Store license.", BSTD_FUNCTION,__LINE__);
        return DRM_Prdy_fail;
    }

    if(eContentIdentifierType == eDRM_PRDY_ND_CONTENT_IDENTIFIER_TYPE_KID)
    {
        if(pKidContent == NULL) {
            printf("[FAILED] %s %d - failed to get the Key ID from the callback.", BSTD_FUNCTION,__LINE__);
            return DRM_Prdy_fail;
        }
        reqKID = pvContentIdentifierCallbackContext;
        BKNI_Memcpy(reqKID->data, pKidContent->data, 16);
        DUMP_DATA_HEX("\t Content CallBack - Requesting Key ID: ",reqKID->data,16);
    }

    return DRM_Prdy_ok;
}

/****************************************************************************
 * Purpose: To test the registration messages
 *  1. Rceiver    : DRM_Prdy_ND_Receiver_RegistrationRequest_Generate
 *
 *  2. Transmitter: DRM_Prdy_ND_Transmitter_RegistrationRequest_Process
 *                  DRM_Prdy_ND_Transmitter_RegistrationResponse_Generate
 *
 *  3. Receiver   : DRM_Prdy_ND_Receiver_RegistrationResponse_Process
 *
 ****************************************************************************/
bool test_registration(DRM_Prdy_Handle_t                  drm,
                       DRM_Prdy_ND_Receiver_Context_t     pRxCtx,
                       DRM_Prdy_ND_Transmitter_Context_t  pTxCtx)
{
    /* Prdy ND specific */
    DRM_Prdy_ID_t                             sessionIDTx;
    DRM_Prdy_ID_t                             sessionIDRx;
    DRM_Prdy_ND_Proximity_Detection_Type_e    eProxiDetectType;
    uint8_t                                  *pbTxProxiDetectChl;
    uint16_t                                  cbTxProxiDetectChl;
    uint16_t                                  dwFlags;
    uint32_t                                  pPrdyRC;
    DRM_Prdy_ND_Message_Type_e                eMsgType = eDRM_PRDY_ND_MESSAGE_TYPE_INVALID;

    uint8_t  *pbMsgRxToTx=NULL;
    uint16_t  cbMsgRxToTx=0;
    uint8_t  *pbMsgTxToRx= NULL;
    uint16_t  cbMsgTxToRx= 0;

    if( DRM_Prdy_ND_Receiver_RegistrationRequest_Generate(
                drm,
                NULL,
                NULL,
                NULL,
                0,
                DRM_PRDY_ND_NO_FLAGS,
                &pbMsgRxToTx,
                &cbMsgRxToTx ) != DRM_Prdy_ok )
    {
       printf("[FAILED] %d  Receiver - Failed to Generate RegistrationRequest, quitting....\n",__LINE__);
       goto exit ;
    }

    if(DRM_Prdy_ND_GetMessageType(pbMsgRxToTx,cbMsgRxToTx,&eMsgType) == DRM_Prdy_ok ) {
        if( eMsgType != eDRM_PRDY_ND_MESSAGE_TYPE_REGISTRATION_REQUEST) {
            printf("[FAILED] %d  Receiver - Failed, invalid message was generated. quitting....\n",__LINE__);
            goto exit ;
        }
    } else {
        printf("[FAILED] %d  Receiver - GetMessageType failed, quitting....\n",__LINE__);
        goto exit ;
    }
    printf("[PASSED] Receiver - Successfully generated RegistrationRequest, size %d.\n",cbMsgRxToTx);

    if( DRM_Prdy_ND_Transmitter_RegistrationRequest_Process(
                drm,
                pbMsgRxToTx,
                cbMsgRxToTx,
                _testCertificateCallback,
                pTxCtx,
                &sessionIDTx,
                &eProxiDetectType,
                &pbTxProxiDetectChl,
                &cbTxProxiDetectChl,
                &dwFlags,
                &pPrdyRC) != DRM_Prdy_ok )
    {
       printf("[FAILED] %d  Transmitter - Failed to process RegistrationRequest, quitting....\n",__LINE__);
       goto exit ;
    }
    printf("[PASSED] Transmitter - Successfully processed RegistrationRequest.\n");
    DUMP_DATA_HEX("\t Transmitter - Session ID: ",sessionIDTx.data,16);
    printf("\t Transmitter - Proximity Type %d\n",eProxiDetectType);
    DUMP_DATA_HEX("\t Transmitter - Proximity Channel:",pbTxProxiDetectChl,cbTxProxiDetectChl);
    printf("\t Transmitter - Proximity Channel: %s\n",pbTxProxiDetectChl);
    printf("\t Transmitter - dwFlag %d RC %d\n",dwFlags,pPrdyRC);

    /* free the message */
    DRM_Prdy_ND_MemFree(pbMsgRxToTx);
    pbMsgRxToTx = NULL;
    /* free the channel message */
    DRM_Prdy_ND_MemFree(pbTxProxiDetectChl);
    pbTxProxiDetectChl = NULL;

    if( DRM_Prdy_ND_Transmitter_RegistrationResponse_Generate(
                drm,
                NULL,
                NULL,
                0,
                DRM_PRDY_ND_NO_FLAGS,
                &pbMsgTxToRx,
                &cbMsgTxToRx) != DRM_Prdy_ok )
    {
       printf("[FAILED] %d  Transmitter - Failed to Generate RegistrationResponse, quitting....\n",__LINE__);
       goto exit ;
    }

    eMsgType = eDRM_PRDY_ND_MESSAGE_TYPE_INVALID;
    if(DRM_Prdy_ND_GetMessageType(pbMsgTxToRx,cbMsgTxToRx,&eMsgType) == DRM_Prdy_ok ) {
        if( eMsgType != eDRM_PRDY_ND_MESSAGE_TYPE_REGISTRATION_RESPONSE) {
            printf("[FAILED] %d  Transmitter - Failed, invalid message was generated. quitting....\n",__LINE__);
            goto exit ;
        }
    } else {
        printf("[FAILED] %d  Transmitter - GetMessageType failed, quitting....\n",__LINE__);
        goto exit ;
    }

    printf("[PASSED] Transmitter - successfully generated RegistrationResponse, size %d.\n",cbMsgTxToRx);

    if( DRM_Prdy_ND_Receiver_RegistrationResponse_Process(
                drm,
                pbMsgTxToRx,
                cbMsgTxToRx,
                _testCertificateCallback,
                pRxCtx,
                &sessionIDRx,
                &eProxiDetectType,
                &pbTxProxiDetectChl,
                &cbTxProxiDetectChl,
                &dwFlags,
                &pPrdyRC) != DRM_Prdy_ok )
    {
       printf("[FAILED] %d  Transmitter - Failed to process RegistrationRequest, quitting....\n",__LINE__);
       goto exit ;
    }

    if( BKNI_Memcmp( sessionIDTx.data,sessionIDRx.data,16) == 0) {
        printf("[PASSED] Receiver - Successfully processed RegistrationResponse.\n");
        DUMP_DATA_HEX("\t Receiver - Session ID: ",sessionIDTx.data,16);
        printf("\t Receiver - Proximity Type %d\n",eProxiDetectType);
        DUMP_DATA_HEX("\t Receiver - Proximity Channel:",pbTxProxiDetectChl,cbTxProxiDetectChl);
        printf("\t Receiver - Proximity Channel: %s\n",pbTxProxiDetectChl);
        printf("\t Receiver - dwFlag %d RC %d\n",dwFlags,pPrdyRC);
    }
    else {
        printf("[FAILED] %d  Receiver - WARNING: Session ID did not match the one generated from Transmitter \n",__LINE__);
        DUMP_DATA_HEX("\t Transmitter - Session ID: ",sessionIDTx.data,16);
        DUMP_DATA_HEX("\t Receiver    - Session ID: ",sessionIDRx.data,16);
    }

    /* free the message */
    DRM_Prdy_ND_MemFree(pbMsgTxToRx);
    /* free the channel message */
    DRM_Prdy_ND_MemFree(pbTxProxiDetectChl);

    printf("%s: END.\n",BSTD_FUNCTION);
    return true;

exit:
    /* free the message */
    if(pbMsgRxToTx != NULL) DRM_Prdy_ND_MemFree(pbMsgRxToTx);
    if(pbMsgTxToRx != NULL) DRM_Prdy_ND_MemFree(pbMsgTxToRx);
    /* free the channel message */
    if(pbTxProxiDetectChl != NULL) DRM_Prdy_ND_MemFree(pbTxProxiDetectChl);
    return false;
}

/****************************************************************************
 *  1. Rceiver    : DRM_Prdy_ND_Receiver_ProximityDetectionStart_Generate
 *
 *  2. Transmitter: DRM_Prdy_ND_Transmitter_ProximityDetectionStart_Process
 *
 *  3. Receiver   : DRM_Prdy_ND_Receiver_ProximityDetectionChallenge_Process
 *
 *  4. Transmitter: DRM_Prdy_ND_Transmitter_ProximityDetectionResponse_Process
 *
 *  5. Receiver   : DRM_Prdy_ND_Receiver_ProximityDetectionResult_Process
 *
 ****************************************************************************/
bool test_proximityDectection(DRM_Prdy_Handle_t   drm)
{
    DRM_Prdy_ND_Message_Type_e   eMsgType = eDRM_PRDY_ND_MESSAGE_TYPE_INVALID;
    uint8_t  *pbMsgRxToTx=NULL;
    uint16_t  cbMsgRxToTx=0;
    uint8_t  *pbMsgTxToRx= NULL;
    uint16_t  cbMsgTxToRx= 0;
    uint16_t  dwFlags;

    if( DRM_Prdy_ND_Receiver_ProximityDetectionStart_Generate(
                drm,
                DRM_PRDY_ND_NO_FLAGS,
                &pbMsgRxToTx,
                &cbMsgRxToTx) != DRM_Prdy_ok )
    {
       printf("[FAILED] %d  Receiver - Failed to Generate ProximityDetectionStart, quitting....\n",__LINE__);
       goto exit ;
    }

    if(DRM_Prdy_ND_GetMessageType(pbMsgRxToTx,cbMsgRxToTx,&eMsgType) == DRM_Prdy_ok ) {
        if( eMsgType != eDRM_PRDY_ND_MESSAGE_TYPE_PROXIMITY_DETECTION_START) {
            printf("[FAILED] %d  Receiver - Failed, invalid message was generated. quitting....\n",__LINE__);
            goto exit ;
        }
    } else {
        printf("[FAILED] %d  Receiver - GetMessageType Failed, quitting....\n",__LINE__);
        goto exit ;
    }
    /* comment out printfs to avoid timeout of the proximity challenge (only 7ms to complete)*/
    printf("[PASSED] Receiver - successfully generated ProximityDetectionStart, size %d.\n",cbMsgRxToTx);
    if( DRM_Prdy_ND_Transmitter_ProximityDetectionStart_Process(
                drm,
                pbMsgRxToTx,
                cbMsgRxToTx,
                DRM_PRDY_ND_NO_FLAGS,
                &pbMsgTxToRx,
                &cbMsgTxToRx,
                &dwFlags ) != DRM_Prdy_ok )
    {
       printf("[FAILED] %d  Transmitter - Failed to process ProximityDetectionStart, quitting....\n",__LINE__);
       goto exit ;
    }

    DRM_Prdy_ND_MemFree(pbMsgRxToTx);
    pbMsgRxToTx = NULL;
    eMsgType = eDRM_PRDY_ND_MESSAGE_TYPE_INVALID;
    if(DRM_Prdy_ND_GetMessageType(pbMsgTxToRx,cbMsgTxToRx,&eMsgType) == DRM_Prdy_ok ) {
        if( eMsgType != eDRM_PRDY_ND_MESSAGE_TYPE_PROXIMITY_DETECTION_CHALLENGE) {
            printf("[FAILED] %d  Transmitter - Failed, invalid message was generated. quitting....\n",__LINE__);
            goto exit ;
        }
    } else {
        printf("[FAILED] %d  Transmitter - GetMessageType failed, quitting....\n",__LINE__);
        goto exit ;
    }
    printf("[PASSED] Transmitter - successfully processed and generated ProximityDetection Challenge, size %d.\n",cbMsgTxToRx);

    /* Receiver: Process challenge */

    if( DRM_Prdy_ND_Receiver_ProximityDetectionChallenge_Process(
                drm,
                pbMsgTxToRx,
                cbMsgTxToRx,
                DRM_PRDY_ND_NO_FLAGS,
                &pbMsgRxToTx,
                &cbMsgRxToTx,
                &dwFlags ) != DRM_Prdy_ok )
    {
       printf("[FAILED] %d  Receiver - Failed to process ProximityDetectionChallenge, quitting....\n",__LINE__);
       goto exit ;
    }

    DRM_Prdy_ND_MemFree(pbMsgTxToRx);
    pbMsgTxToRx = NULL;
    eMsgType = eDRM_PRDY_ND_MESSAGE_TYPE_INVALID;
    if(DRM_Prdy_ND_GetMessageType(pbMsgRxToTx,cbMsgRxToTx,&eMsgType) == DRM_Prdy_ok ) {
        if( eMsgType != eDRM_PRDY_ND_MESSAGE_TYPE_PROXIMITY_DETECTION_RESPONSE) {
            printf("[FAILED] %d  Receiver - Failed, invalid message was generated. quitting....\n",__LINE__);
            goto exit ;
        }
    } else {
        printf("[FAILED] %d  Receiver - GetMessageType failed, quitting....\n",__LINE__);
        goto exit ;
    }

    printf("[PASSED] Receiver - successfully processed and generated ProximityDetectionResponse, size %d.\n",cbMsgRxToTx);

    if( DRM_Prdy_ND_Transmitter_ProximityDetectionResponse_Process(
                drm,
                pbMsgRxToTx,
                cbMsgRxToTx,
                DRM_PRDY_ND_NO_FLAGS,
                &pbMsgTxToRx,
                &cbMsgTxToRx,
                &dwFlags ) != DRM_Prdy_ok )
    {
       printf("[FAILED] %d  Transmitter - Failed to process ProximityDetectionStart, quitting....\n",__LINE__);
       goto exit ;
    }

    DRM_Prdy_ND_MemFree(pbMsgRxToTx);
    pbMsgRxToTx = NULL;
    eMsgType = eDRM_PRDY_ND_MESSAGE_TYPE_INVALID;
    if(DRM_Prdy_ND_GetMessageType(pbMsgTxToRx,cbMsgTxToRx,&eMsgType) == DRM_Prdy_ok ) {
        if( eMsgType != eDRM_PRDY_ND_MESSAGE_TYPE_PROXIMITY_DETECTION_RESULT) {
            printf("[FAILED] %d  Transmitter - Failed, invalid message was generated. quitting....\n",__LINE__);
            goto exit ;
        }
    } else {
        printf("[FAILED] %d  Transmitter - GetMessageType failed, quitting....\n",__LINE__);
        goto exit ;
    }

    printf("[PASSED] Transmitter - successfully processed and generated ProximityDetection Result, size %d.\n",cbMsgTxToRx);


    /* Receiver: Process result */
    if( DRM_Prdy_ND_Receiver_ProximityDetectionResult_Process(
                drm,
                pbMsgTxToRx,
                cbMsgTxToRx,
                &dwFlags ) != DRM_Prdy_ok )
    {
       printf("[FAILED] %d  Receiver - Failed to process ProximityDetectionResult, quitting....\n",__LINE__);
       goto exit ;
    }

    printf("[PASSED] Receiver - successfully processed ProximityDetectionResponse, size %d.\n",cbMsgRxToTx);

    if(pbMsgRxToTx != NULL) DRM_Prdy_ND_MemFree(pbMsgRxToTx);
    if(pbMsgTxToRx != NULL) DRM_Prdy_ND_MemFree(pbMsgTxToRx);

    printf("%s: END.\n",BSTD_FUNCTION);
    return true;

exit:
    /* free the message */
    if(pbMsgRxToTx != NULL) DRM_Prdy_ND_MemFree(pbMsgRxToTx);
    if(pbMsgTxToRx != NULL) DRM_Prdy_ND_MemFree(pbMsgTxToRx);
    return false;

}

/****************************************************************************
 *  1. Transmitter: DRM_Prdy_LocalLicense_CreateLicense
 *                  DRM_Prdy_LocalLicense_GetKID
 *                  DRM_Prdy_LocalLicense_StoreLicense
 *                  DRM_Prdy_LocalLicense_EncryptSample
 *                  DRM_Prdy_LocalLicense_GetKID_base64W
 *                  DRM_Prdy_Content_SetProperty
 *                  DRM_Prdy_Reader_Bind
 *                  DRM_Prdy_ND_Transmitter_PrepareLicensesForTransmit
 *                  DRM_Prdy_Reader_Commit
 *                  DRM_Prdy_Reader_Close
 *
 *  2. Rceiver    : DRM_Prdy_ND_Receiver_LicenseRequest_Generate
 *
 *  3. Transmitter: DRM_Prdy_ND_Transmitter_LicenseRequest_Process
 *                  DRM_Prdy_ND_Transmitter_LicenseTransmit_Generate
 *
 *  4. DRM_Prdy_StoreMgmt_DeleteLicenses
 *
 *  5. Receiver   : DRM_Prdy_ND_Receiver_LicenseTransmit_Process
 *                  DRM_Prdy_Content_SetProperty
 *                  DRM_Prdy_Reader_Bind
 *                  DRM_Prdy_Reader_Commit
 *                  DRM_Prdy_Reader_Decrypt
 *                  DRM_Prdy_Reader_Close
 *
 *  6. DRM_Prdy_LocalLicense_Release
 *
 ****************************************************************************/
bool test_license(DRM_Prdy_Handle_t   drm)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    DRM_Prdy_ND_Message_Type_e   eMsgType = eDRM_PRDY_ND_MESSAGE_TYPE_INVALID;
    DRM_Prdy_local_license_policy_descriptor_t licPoDes;
    DRM_Prdy_license_handle  phLicense=NULL;
    Drm_Prdy_KID_t KIDCheck;
    Drm_Prdy_KID_t kid;
    Drm_Prdy_KID_t requestingkid;
    DRM_Prdy_DecryptSettings_t pDecryptSettings;
    DRM_Prdy_DecryptContext_t pDecryptCtx;
    DRM_Prdy_AES_CTR_Info_t  aesCtrInfo;
    uint32_t  numOfLicDeleted = 0;
    uint16_t  kidBase64W[25] = {0};
    uint32_t  kidBase64Size=0;
    uint8_t  *pbMsgRxToTx=NULL;
    uint16_t  cbMsgRxToTx=0;
    uint8_t  *pbMsgTxToRx= NULL;
    uint16_t  cbMsgTxToRx= 0;
    uint16_t  dwFlags;
    uint32_t  pPrdyRC;
    char      sampleText[] = {"This is a clear text"};
    uint8_t  *clearText = NULL;
    uint8_t  *cipherText = NULL;
    uint8_t   iv[8] = {0};

    /* printf("%s: START...\n",BSTD_FUNCTION);*/

    /* First cleanup any expired license */
    if( DRM_Prdy_Cleanup_LicenseStores( drm) != DRM_Prdy_ok)
    {
       printf("[WARNING] %d Transmitter - Failed to cleanup the license store.\n",__LINE__);
    }

    /* create a local simple license */
    if( DRM_Prdy_LocalLicense_InitializePolicyDescriptor( &licPoDes) != DRM_Prdy_ok)
    {
        printf("[FAILED] %d  Failed to initialize License Policy Descriptor.\n",__LINE__);
        goto exit;
    }

    printf("[PASSED] Transmitter - initialize policy descryptor\n");
    licPoDes.wSecurityLevel = 150;
    licPoDes.fCannotPersist = 0;
    licPoDes.cPlayEnablers = 2;
    BKNI_Memcpy(&licPoDes.rgoPlayEnablers[0],
                &DRM_PRDY_ND_PLAYENABLER_UNKNOWN_OUTPUT , sizeof(DRM_Prdy_guid_t));
    BKNI_Memcpy(&licPoDes.rgoPlayEnablers[1],
                &DRM_PRDY_ND_PLAYENABLER_CONSTRAINED_RESOLUTION_UNKNOWN_OUTPUT , sizeof(DRM_Prdy_guid_t));

    /* randomly generate a Key ID for the new license */
    if( gen_random_num(16, kid.data) != 0) {
        printf("[FAILED] %d  Failed to generate key ID.\n",__LINE__);
        goto exit;
    }

    if( DRM_Prdy_LocalLicense_CreateLicense(
                drm,
                &licPoDes,
                eDRM_Prdy_eLocal_license_bound_simple,/* set the license type to local_license_bound_simple */
                &kid,
                0,
                NULL,
                NULL,
                &phLicense) != DRM_Prdy_ok)
    {
        printf("[FAILED] %d  Transmitter - Failed to create license.\n",__LINE__);
        goto exit;
    }
    printf("[PASSED] Transmitter - Successfully created local license.\n");

    /* sanity check */
    if( DRM_Prdy_LocalLicense_GetKID( phLicense,&KIDCheck) !=  DRM_Prdy_ok)
    {
        printf("[FAILED] %d  Transmitter - Sanity check failed, can't get the KID from the created license.\n",__LINE__);
        goto exit;
    }

    if( memcmp( &kid.data[0],&KIDCheck.data[0],16) != 0)
    {
        printf("[FAILED] %d  Transmitter - Sanity check failed, the KIDs did not match.\n",__LINE__);
        goto exit;
    }

    /* store the license in XMR*/
    if( DRM_Prdy_LocalLicense_StoreLicense( phLicense,
                                            eDRM_Prdy_elocal_license_xmr_store) != DRM_Prdy_ok)
    {
        printf("[FAILED] %d  Transmitter - Failed to Store license.\n",__LINE__);
        goto exit;
    }

    printf("[PASSED] Transmitter - StoreLicense successfully.\n");

    /* encryption test */
    rc = NEXUS_Memory_Allocate(sizeof(sampleText), NULL, (void *)&clearText);
    if(rc != NEXUS_SUCCESS)
    {
        printf("[FAILED] %d  Transmitter - Failed to allocate memory for the clear text for encryption test.\n",__LINE__);
        goto exit;
    }
    BKNI_Memcpy( clearText,sampleText,sizeof(sampleText) );

    rc = NEXUS_Memory_Allocate(sizeof(sampleText), NULL, (void *)&cipherText);
    if(rc != NEXUS_SUCCESS)
    {
        printf("[FAILED] %d  Transmitter - Failed to allocate memory for the cipher text for encryption test.\n",__LINE__);
        goto exit;
    }
    BKNI_Memset(cipherText, 0, sizeof(sampleText));

    if(DRM_Prdy_LocalLicense_EncryptSample( phLicense,
                                            clearText,
                                            cipherText,
                                            sizeof(sampleText),
                                            iv) != DRM_Prdy_ok)
    {
        printf("[FAILED] %d  Transmitter - DRM_Prdy_LocalLicense_EncryptSample failed. Encryption test failed.\n",__LINE__);
        goto exit;
    };

    printf("[PASSED] Transmitter - LocalLicense_EncryptSample.\n");
    DUMP_DATA_HEX("\t Transmitter - clearText  : ",clearText,sizeof(sampleText));
    DUMP_DATA_HEX("\t Transmitter - cipherText : ",cipherText,sizeof(sampleText));

    /* now bind the license to app context */
    DRM_Prdy_LocalLicense_GetKID_base64W(phLicense,
                                         kidBase64W,
                                         &kidBase64Size);
    /* Content_SetProperty() */
    if(  DRM_Prdy_Content_SetProperty( drm,
                                       DRM_Prdy_contentSetProperty_eKID,
                                       (uint8_t *) kidBase64W,
                                       kidBase64Size*sizeof(uint16_t)) != DRM_Prdy_ok )
    {
        printf("[FAILED] %d  Transmitter - Failed to SetProperty for the KID, exiting...\n",__LINE__);
        goto exit;
    }
    printf("[PASSED] Transmitter - Content_SetProperty.\n");

    /* initialize the DecryptContext */
    DRM_Prdy_GetDefaultDecryptSettings( &pDecryptSettings);
    if( DRM_Prdy_SetDecryptContext ( &pDecryptSettings,
                                     &pDecryptCtx ) != DRM_Prdy_ok )
    {
        printf("[FAILED] %d  Transmitter - Set Decrypt Context, exiting...\n",__LINE__);
        goto exit;
    }
    printf("[PASSED] Transmitter - SetDecryptContext.\n");

    if( DRM_Prdy_Reader_Bind( drm, &pDecryptCtx)!= DRM_Prdy_ok )
    {
        printf("[FAILED] %d  Transmitter - Failed to Bind the license.\n",__LINE__);
        goto exit;
    }
    printf("[PASSED] Transmitter - Reader_Bind.\n");

    if( DRM_Prdy_ND_Transmitter_PrepareLicensesForTransmit(drm) != DRM_Prdy_ok )
    {
        printf("[FAILED] %d  Transmitter - Failed to PrepareLicensesForTransmit. Exiting...\n",__LINE__);
        goto exit;
    }
    printf("[PASSED] Transmitter - PrepareLicensesForTransmit.\n");

    if( DRM_Prdy_Reader_Commit( drm) != DRM_Prdy_ok )
    {
        printf("[FAILED] %d  Transmitter - Failed to commit the license after bind. Exiting...\n",__LINE__);
        goto exit;
    }
    printf("[PASSED] Transmitter - Reader commit...\n");

    if( DRM_Prdy_Reader_Close( &pDecryptCtx) != DRM_Prdy_ok )
    {
        printf("[FAILED] %d  Transmitter -  DRM_Prdy_Reader_Close failed. Exiting...\n",__LINE__);
        goto exit;
    }
    printf("[PASSED] Transmitter - Reader Close...\n");


    pDecryptCtx.pKeyContext = NULL;
    pDecryptCtx.pDecrypt = NULL;

    if( DRM_Prdy_ND_Receiver_LicenseRequest_Generate(
                drm,
                &DRM_PRDY_ND_ACTION_PLAY,
                NULL,
                eDRM_PRDY_ND_CONTENT_IDENTIFIER_TYPE_KID,
                (const uint8_t *) &kid,
                sizeof(DRM_Prdy_ID_t),
                NULL,
                NULL,
                0,
                DRM_PRDY_ND_NO_FLAGS,
                &pbMsgRxToTx,
                &cbMsgRxToTx) != DRM_Prdy_ok )
    {
       printf("[FAILED] %d  Receiver - Failed to Generate ProximityDetectionStart, quitting....\n",__LINE__);
       goto exit ;
    }

    if(DRM_Prdy_ND_GetMessageType(pbMsgRxToTx,cbMsgRxToTx,&eMsgType) == DRM_Prdy_ok ) {
        if( eMsgType != eDRM_PRDY_ND_MESSAGE_TYPE_LICENSE_REQUEST ) {
            printf("[FAILED] %d  Receiver - Failed, invalid message was generated. quitting....\n",__LINE__);
            goto exit ;
        }
    } else {
        printf("[FAILED] %d  Receiver - GetMessageType Failed, quitting....\n",__LINE__);
        goto exit ;
    }
    printf("[PASSED] Receiver - successfully generated a LicenseRequest, size %d.\n",cbMsgRxToTx);

    if( DRM_Prdy_ND_Transmitter_LicenseRequest_Process(
                drm,
                pbMsgRxToTx,
                cbMsgRxToTx,
                NULL,
                NULL,
                _contentIdentifier_CallBack,
                (void *)&requestingkid,
                &dwFlags,
                &pPrdyRC ) != DRM_Prdy_ok )
    {
       printf("[FAILED] %d  Transmitter - Failed to process LicenseRequest, quitting....\n",__LINE__);
       goto exit ;
    }

    printf("[PASSED] Transmitter - Processed LicenseRequest successfully.\n");

    if( DRM_Prdy_ND_Transmitter_LicenseTransmit_Generate(
                drm,
                NULL,
                NULL,
                0,
                DRM_PRDY_ND_NO_FLAGS,
                &pbMsgTxToRx,
                &cbMsgTxToRx) != DRM_Prdy_ok )
    {
       printf("[FAILED] %d  Transmitter - Failed to process ProximityDetectionStart, quitting....\n",__LINE__);
       goto exit ;
    }

    DRM_Prdy_ND_MemFree(pbMsgRxToTx);
    pbMsgRxToTx = NULL;
    eMsgType = eDRM_PRDY_ND_MESSAGE_TYPE_INVALID;
    if(DRM_Prdy_ND_GetMessageType(pbMsgTxToRx,cbMsgTxToRx,&eMsgType) == DRM_Prdy_ok ) {
        if( eMsgType != eDRM_PRDY_ND_MESSAGE_TYPE_LICENSE_TRANSMIT ) {
            printf("[FAILED] %d  Transmitter - Failed, invalid message was generated. quitting....\n",__LINE__);
            goto exit ;
        }
    } else {
        printf("[FAILED] %d  Transmitter - GetMessageType failed, quitting....\n",__LINE__);
        goto exit ;
    }
    printf("[PASSED] Transmitter - successfully generated LicenseTransmit, size %d.\n",cbMsgTxToRx);

    /* Now we delete the license to make sure that receiver uses the transmitted license
     * and confirm if the transmitted license can be processed */
    /*printf("\t Transmitter - Delete the License. \n"); */
    if( DRM_Prdy_StoreMgmt_DeleteLicenses(
                drm,
                kidBase64W,
                kidBase64Size,
                &numOfLicDeleted ) != DRM_Prdy_ok )
    {
       printf("[FAILED] %d  Transmitter - WARNING Failed to delete the license.\n",__LINE__);
    }
    else
    {
       printf("[PASSED] Transmitter - Successfully deleted the license, number of license deleted: %d.\n",numOfLicDeleted );
       /* Check if the Reader_Bind should fail */
       if(  DRM_Prdy_Content_SetProperty(
                   drm,
                   DRM_Prdy_contentSetProperty_eKID,
                   (uint8_t *) kidBase64W,
                   kidBase64Size*sizeof(uint16_t)) != DRM_Prdy_ok )
       {
           printf("[FAILED] %d  Transmitter - Failed to SetProperty for the KID, exiting...\n",__LINE__);
       }
       else if ( DRM_Prdy_Reader_Bind( drm, &pDecryptCtx) != DRM_Prdy_ok )
       {
           printf("[PASSED] Transmitter - Expected failure to Bind the deleted license, continue...\n");
       }
       else
       {
           printf("[FAILED] %d  Transmitter - WARNING still be able to bind the deleted license, continue...\n",__LINE__);
       }
    }

    /* Receiver: Process LicenseTransmit */
    if( DRM_Prdy_ND_Receiver_LicenseTransmit_Process(
                drm,
                pbMsgTxToRx,
                cbMsgTxToRx,
                _testCertificateCallback,
                NULL,
                &dwFlags ) != DRM_Prdy_ok )
    {
       printf("[FAILED] %d  Receiver - Failed to process LicenseTransmit, quitting....\n",__LINE__);
       goto exit ;
    }
    printf("[PASSED] Receiver - successfully processed LicenseTransmit, size %d.\n",cbMsgRxToTx);

    DRM_Prdy_ND_MemFree(pbMsgTxToRx);
    pbMsgTxToRx = NULL;

    /* Now check if we can bind the transmitted license again */
    if(  DRM_Prdy_Content_SetProperty( drm,
                                       DRM_Prdy_contentSetProperty_eKID,
                                       (uint8_t *) kidBase64W,
                                       kidBase64Size*sizeof(uint16_t)) != DRM_Prdy_ok )
    {
        printf("[FAILED] %d  Receiver - Failed to SetProperty for the KID, exiting...\n",__LINE__);
        goto exit;
    }

    if( DRM_Prdy_Reader_Bind( drm, &pDecryptCtx) != DRM_Prdy_ok )
    {
        printf("[FAILED] %d  Receiver - Failed to Bind the license.\n",__LINE__);
        goto exit;
    }

    printf("[PASSED] Receiver - Reader Bind succeeded.\n");

    if( DRM_Prdy_Reader_Commit( drm) != DRM_Prdy_ok )
    {
        printf("[FAILED] %d  Receiver- Failed to commit the license after bind. Exiting...\n",__LINE__);
        goto exit;
    }
    printf("[PASSED] Receiver - Reader commit.\n");

    /* Decryption test */
    BKNI_Memcpy( &aesCtrInfo.qwInitializationVector,iv,8 );
    aesCtrInfo.qwBlockOffset = 0;
    aesCtrInfo.bByteOffset = 0;
    DUMP_DATA_HEX("\t Receiver - cipherText : ",cipherText,sizeof(sampleText));
    fflush(stdout);
    if(DRM_Prdy_Reader_Decrypt( &pDecryptCtx,
                                &aesCtrInfo,
                                (uint8_t *) cipherText,
                                sizeof(sampleText) ) != DRM_Prdy_ok)
    {
        printf("[FAILED] %d  Receiver - Reader_Decrypt.\n",__LINE__);
    }
    else
    {
        printf("[PASSED] Receiver - Reader_Decrypt.\n");
        if( memcmp( clearText,cipherText,sizeof(sampleText)) != 0)
        {
            printf("[FAILED] %d  Receiver - failed to decrypt the cipher text properly.\n",__LINE__);
        }
        else
        {
            printf("[PASSED] Receiver - Decryption succeeded.\n");
        }
        DUMP_DATA_HEX("\t Receiver - clearText  : ",clearText,sizeof(sampleText));
        fflush(stdout);
        DUMP_DATA_HEX("\t Receiver - decrypted  : ",cipherText,sizeof(sampleText));
        fflush(stdout);
    }

    if( DRM_Prdy_Reader_Close( &pDecryptCtx) != DRM_Prdy_ok )
    {
        printf("[FAILED] %d  Receiver-  DRM_Prdy_Reader_Close failed.\n",__LINE__);
        goto exit;
    }

    if(  phLicense != NULL) {
        DRM_Prdy_LocalLicense_Release(&phLicense);
    }

    printf("%s: END...\n",BSTD_FUNCTION);
    fflush(stdout);
    return true;

exit:
    /* free the message */
    if(pbMsgRxToTx != NULL) DRM_Prdy_ND_MemFree(pbMsgRxToTx);
    if(pbMsgTxToRx != NULL) DRM_Prdy_ND_MemFree(pbMsgTxToRx);
    if(clearText != NULL) NEXUS_Memory_Free(clearText);
    if(cipherText != NULL) NEXUS_Memory_Free(cipherText);

    if(  phLicense != NULL) {
        DRM_Prdy_LocalLicense_Release(&phLicense);
    }
    return false;
}

int main(int argc, char* argv[])
{
    int testResult = -1;

    NEXUS_PlatformSettings platformSettings;

    /* DRM_Prdy specific */
    DRM_Prdy_Init_t     prdyParamSettings;
    DRM_Prdy_Handle_t   drm=NULL;

    /* Prdy ND specific */
    DRM_Prdy_ND_Receiver_Context_t     pRxCtx=NULL;
    DRM_Prdy_ND_Transmitter_Context_t  pTxCtx=NULL;

    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    printf("\n\n");

    /* init Nexus */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    DRM_Prdy_GetDefaultParamSettings(&prdyParamSettings);
    drm =  DRM_Prdy_Initialize( &prdyParamSettings);
    if( drm == NULL)
    {
       printf("[FAILED] %d  Failed to create drm, quitting....",__LINE__);
       goto clean_exit ;
    }
    printf("[PASSED] %s: Created a DRM_Prdy_context.\n",BSTD_FUNCTION);

    /* initialize the Receiver context */
    pRxCtx = DRM_Prdy_ND_Receiver_Initialize();
    if( pRxCtx == NULL)
    {
       printf("[FAILED] %d  Failed to initialize Receiver context, quitting....\n",__LINE__);
       goto clean_exit ;
    }
    printf("[PASSED] %s: Initialized Receiver context.\n",BSTD_FUNCTION);

    /****************** BEGIN SESSIONS ****************************/
    if( DRM_Prdy_ND_Receiver_BeginSession(drm,pRxCtx) != DRM_Prdy_ok )
    {
       printf("[FAILED] %d  Receiver - Failed to Begin Session, quitting....\n",__LINE__);
       goto clean_exit ;
    }
    printf("[PASSED] Receiver - Begin Session.\n");

    /* initialize the Transmitter context */
    pTxCtx = DRM_Prdy_ND_Transmitter_Initialize();
    if( pTxCtx == NULL)
    {
       printf("[FAILED] %d  Failed to initialize Transmitter context, quitting....\n",__LINE__);
       goto clean_exit ;
    }
    printf("[PASSED] %s: Initialized Transmitter context.\n",BSTD_FUNCTION);

    if( DRM_Prdy_ND_Transmitter_BeginSession(drm,pTxCtx) != DRM_Prdy_ok )
    {
       printf("[FAILED] %d  Transmitter - Failed to Begin Session, quitting....\n",__LINE__);
       goto clean_exit ;
    }
    printf("[PASSED] Transmitter - Begin Session.\n");

    printf("\n");
    if( !test_registration(drm,pRxCtx,pTxCtx)) {
       printf("[FAILED] %d - Registration test\n",__LINE__);
       goto clean_exit ;
    }
    printf("[PASSED]  - test_registration\n\n");

    if( !test_proximityDectection(drm)) {
       printf("[FAILED] %d - ProximityDectection test\n",__LINE__);
       /*goto clean_exit ;
        */
    }
    else {
        printf("[PASSED]  - test_proximityDectection\n\n");
    }

    if( !test_license(drm)) {
       printf("[FAILED] %d - License test\n",__LINE__);
       goto clean_exit ;
    }
    printf("[PASSED]  - test_license\n\n");

    /****************** ENDING SESSIONS ****************************/
    if( DRM_Prdy_ND_Transmitter_EndSession(drm) != DRM_Prdy_ok )
    {
       printf("[FAILED] %d  Transmitter - Failed to End Session, quitting....\n",__LINE__);
       goto clean_exit ;
    }
    printf("[PASSED] Transmitter - End Session.\n");

    if( DRM_Prdy_ND_Receiver_EndSession(drm) != DRM_Prdy_ok )
    {
       printf("[FAILED] %d  Receiver - Failed to End Session, quitting....\n",__LINE__);
       goto clean_exit ;
    }
    printf("[PASSED] Receiver - End Session.\n");

    printf("\n[PASSED]  - All tests complete.\n\n");

    testResult = 0;

clean_exit:

    if( drm != NULL) DRM_Prdy_Uninitialize(drm);
    if( pRxCtx != NULL) DRM_Prdy_ND_Receiver_Uninitialize(pRxCtx );
    if( pTxCtx != NULL) DRM_Prdy_ND_Transmitter_Uninitialize(pTxCtx );
    NEXUS_Platform_Uninit();

    return testResult;
}
