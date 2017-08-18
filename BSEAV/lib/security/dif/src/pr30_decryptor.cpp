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
#undef LOGE
#undef LOGW
#undef LOGD
#undef LOGV
#define LOGE BDBG_ERR
#define LOGW BDBG_WRN
#define LOGD BDBG_MSG
#define LOGV BDBG_MSG

#include <string.h>
#include <sstream>
#include <byteswap.h>
#include <time.h>
#include <sys/time.h>
#include "prdy_http.h"
#include "pr30_decryptor.h"

#include "drmsecuretimeconstants.h"
BDBG_MODULE(pr30_decryptor);
#include "dump_hex.h"

using namespace dif_streamer;

DRM_APP_CONTEXT* Playready30Decryptor::s_pDrmAppCtx = NULL;
DRM_VOID* Playready30Decryptor::s_pOEMContext = NULL;
DRM_BYTE* Playready30Decryptor::s_pbOpaqueBuffer = NULL;
DRM_DWORD Playready30Decryptor::s_cbOpaqueBuffer = 0;
DRM_BYTE* Playready30Decryptor::s_pbRevocationBuffer = NULL;
uint32_t Playready30Decryptor::s_nextSessionId = 1;
uint8_t Playready30Decryptor::s_sessionNum = 0;

#define MAX_TIME_CHALLENGE_RESPONSE_LENGTH (1024*64)
#define MAX_URL_LENGTH (512)

static int initSecureClock( DRM_APP_CONTEXT *pDrmAppCtx)
{
    int                   rc = 0;
    DRM_DWORD             cbChallenge     = 0;
    DRM_BYTE             *pbChallenge     = NULL;
    DRM_BYTE             *pbResponse      = NULL;
    char                 *pTimeChallengeURL = NULL;
    char                  secureTimeUrlStr[MAX_URL_LENGTH];
    bool                  redirect = true;
    int32_t               petRC=0;
    uint32_t              petRespCode = 0;
    uint32_t              startOffset;
    uint32_t              length;
    uint32_t              post_ret;
    NEXUS_MemoryAllocationSettings allocSettings;
    DRM_RESULT            drResponse = DRM_SUCCESS;
    DRM_RESULT            dr = DRM_SUCCESS;

    dr = Drm_SecureTime_GenerateChallenge( pDrmAppCtx,
                                           &cbChallenge,
                                           &pbChallenge );
    ChkDR(dr);

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    rc = NEXUS_Memory_Allocate(MAX_URL_LENGTH, &allocSettings, (void **)(&pTimeChallengeURL ));
    if(rc != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%s - %d NEXUS_Memory_Allocate failed for time challenge response buffer, rc = %d\n",__FUNCTION__, __LINE__, rc));
        goto ErrorExit;
    }

    /* send the petition request to Microsoft with HTTP GET */
    petRC = PRDY_HTTP_Client_GetForwardLinkUrl((char *)g_dstrHttpSecureTimeServerUrl.pszString,
                                               &petRespCode,
                                               (char**)&pTimeChallengeURL);

    if( petRC != 0)
    {
       BDBG_ERR(("%d Secure Time forward link petition request failed, rc = %d\n",__LINE__, petRC));
       rc = petRC;
       goto ErrorExit;
    }

    do
    {
        redirect = false;

        /* we need to check if the Pettion responded with redirection */
        if( petRespCode == 200)
        {
            redirect = false;
        }
        else if( petRespCode == 302 || petRespCode == 301)
        {
            redirect = true;
            memset(secureTimeUrlStr, 0, MAX_URL_LENGTH);
            strcpy(secureTimeUrlStr, pTimeChallengeURL);
            memset(pTimeChallengeURL, 0, MAX_URL_LENGTH);

            petRC = PRDY_HTTP_Client_GetSecureTimeUrl(secureTimeUrlStr,
                                                      &petRespCode,
                                                      (char**)&pTimeChallengeURL);

            if( petRC != 0)
            {
               BDBG_ERR(("%d Secure Time URL petition request failed, rc = %d\n",__LINE__, petRC));
               rc = petRC;
               goto ErrorExit;
            }
        }
        else
        {
           BDBG_ERR(("%d Secure Clock Petition responded with unsupported result, rc = %d, can't get the time challenge URL\n",__LINE__, petRespCode));
           rc = -1;
           goto ErrorExit;
        }
    } while (redirect);

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    rc = NEXUS_Memory_Allocate(MAX_TIME_CHALLENGE_RESPONSE_LENGTH, &allocSettings, (void **)(&pbResponse ));
    if(rc != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%d NEXUS_Memory_Allocate failed for time challenge response buffer, rc = %d\n",__LINE__, rc));
        goto ErrorExit;
    }

    BKNI_Memset(pbResponse, 0, MAX_TIME_CHALLENGE_RESPONSE_LENGTH);
    post_ret = PRDY_HTTP_Client_SecureTimeChallengePost(pTimeChallengeURL,
                                                 (char *)pbChallenge,
                                                 1,
                                                 150,
                                                 (unsigned char**)&(pbResponse),
                                                 &startOffset,
                                                 &length);
    if( post_ret != 0)
    {
        BDBG_ERR(("%d Secure Time Challenge request failed, rc = %d\n",__LINE__, post_ret));
        rc = post_ret;
        goto ErrorExit;
    }

    drResponse = Drm_SecureTime_ProcessResponse(
                                    pDrmAppCtx,
                                    length,
                                    (uint8_t *) pbResponse);
    if ( drResponse != DRM_SUCCESS )
    {
       BDBG_ERR(("%s - %d Drm_SecureTime_ProcessResponse failed, drResponse = %x\n",__FUNCTION__, __LINE__, (unsigned)drResponse));
       dr = drResponse;
       ChkDR( drResponse);

    }
    BDBG_LOG(("%d Initialized Playready Secure Clock success.",__LINE__));

    /* NOW testing the system time */

ErrorExit:

    ChkVOID( SAFE_OEM_FREE( pbChallenge ) );

    if( pTimeChallengeURL    != NULL)
        NEXUS_Memory_Free(pTimeChallengeURL  );

    if( pbResponse != NULL )
        NEXUS_Memory_Free(pbResponse);

    return rc;
}

DRM_API DRM_RESULT DRM_CALL DRMTOOLS_PrintOPLOutputIDs( __in const DRM_OPL_OUTPUT_IDS *f_pOPLs )
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_DWORD i;
    DRM_WCHAR rgwszGUID[DRM_GUID_STRING_LEN+1] = {0};
    DRM_CHAR  rgszGUID[DRM_NO_OF(rgwszGUID)] = {0};

    printf("    (%d GUIDs)\r\n", f_pOPLs->cIds );
    for( i = 0; i < f_pOPLs->cIds; i++ )
    {
        dr = DRM_UTL_GuidToString(&f_pOPLs->rgIds[i], rgwszGUID);
        /* Safe to use, input parameter is in ASCII */
        DRM_UTL_DemoteUNICODEtoASCII(rgwszGUID, rgszGUID, DRM_NO_OF(rgwszGUID) - 1);

        printf("    GUID = %s\r\n", rgszGUID);
    }
    printf("\r\n");
    return dr;
}

DRM_API DRM_RESULT DRM_CALL DRMTOOLS_PrintVideoOutputProtectionIDs( __in const DRM_VIDEO_OUTPUT_PROTECTION_IDS_EX *f_pOPLs )
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_DWORD i;
    DRM_WCHAR rgwszGUID[DRM_GUID_STRING_LEN+1] = {0};
    DRM_CHAR  rgszGUID[DRM_NO_OF(rgwszGUID)] = {0};

    printf("    (%d entries)\r\n", f_pOPLs->cEntries );
    for( i = 0; i < f_pOPLs->cEntries; i++ )
    {
        ChkDR( DRM_UTL_GuidToString( &f_pOPLs->rgVop[i].guidId,
                            rgwszGUID ) );
        /* Safe to use, input parameter is in ASCII */
        DRM_UTL_DemoteUNICODEtoASCII( rgwszGUID, rgszGUID, DRM_NO_OF(rgwszGUID)-1 );

        printf("    GUID = %s\r\n", rgszGUID);
    }
    printf("\r\n");
ErrorExit:
    return dr;
}

DRM_RESULT policy_callback(
    const DRM_VOID *f_pvPolicyCallbackData,
    DRM_POLICY_CALLBACK_TYPE f_dwCallbackType,
    const DRM_VOID *f_pv)
{
    DRM_RESULT dr = DRM_SUCCESS;
    const DRM_PLAY_OPL_EX *oplPlay = NULL;

    BSTD_UNUSED(f_pv);

    switch( f_dwCallbackType )
    {
        case DRM_PLAY_OPL_CALLBACK:
            printf("  Got DRM_PLAY_OPL_CALLBACK from Bind:\r\n");
            ChkArg( f_pvPolicyCallbackData != NULL );
            oplPlay = (const DRM_PLAY_OPL_EX*)f_pvPolicyCallbackData;

            printf("    minOPL:\r\n");
            printf("    wCompressedDigitalVideo   = %d\r\n", oplPlay->minOPL.wCompressedDigitalVideo);
            printf("    wUncompressedDigitalVideo = %d\r\n", oplPlay->minOPL.wUncompressedDigitalVideo);
            printf("    wAnalogVideo              = %d\r\n", oplPlay->minOPL.wAnalogVideo);
            printf("    wCompressedDigitalAudio   = %d\r\n", oplPlay->minOPL.wCompressedDigitalAudio);
            printf("    wUncompressedDigitalAudio = %d\r\n", oplPlay->minOPL.wUncompressedDigitalAudio);
            printf("\r\n");

            printf("    oplIdReserved:\r\n");
            ChkDR( DRMTOOLS_PrintOPLOutputIDs( &oplPlay->oplIdReserved ) );

            printf("    vopi:\r\n");
            ChkDR( DRMTOOLS_PrintVideoOutputProtectionIDs( &oplPlay->vopi ) );

            break;

        case DRM_EXTENDED_RESTRICTION_QUERY_CALLBACK:
            {
                const DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT *pExtCallback = (const DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT*)f_pvPolicyCallbackData;
                DRM_DWORD i = 0;
                printf("  Got DRM_EXTENDED_RESTRICTION_QUERY_CALLBACK from Bind:\r\n");

                printf("    wRightID = %d\r\n", pExtCallback->wRightID);
                printf("    wType    = %d\r\n", pExtCallback->pRestriction->wType);
                printf("    wFlags   = 0x%x\r\n", pExtCallback->pRestriction->wFlags);

                printf("    Data     = ");

                for( i = pExtCallback->pRestriction->ibData; (i - pExtCallback->pRestriction->ibData) < pExtCallback->pRestriction->cbData; i++ )
                {
                    printf("0x%.2X ", pExtCallback->pRestriction->pbBuffer[ i ] );
                }
                printf("\r\n\r\n");

                /* Report that restriction was not understood */
                dr = DRM_E_EXTENDED_RESTRICTION_NOT_UNDERSTOOD;
            }
            break;
        case DRM_EXTENDED_RESTRICTION_CONDITION_CALLBACK:
            {
                const DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT *pExtCallback = (const DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT*)f_pvPolicyCallbackData;
                DRM_DWORD i = 0;

                printf("  Got DRM_EXTENDED_RESTRICTION_CONDITION_CALLBACK from Bind:\r\n");

                printf("    wRightID = %d\r\n", pExtCallback->wRightID);
                printf("    wType    = %d\r\n", pExtCallback->pRestriction->wType);
                printf("    wFlags   = 0x%x\r\n", pExtCallback->pRestriction->wFlags);

                printf("    Data     = ");
                for( i = pExtCallback->pRestriction->ibData; (i - pExtCallback->pRestriction->ibData) < pExtCallback->pRestriction->cbData; i++ )
                {
                    printf("0x%.2X ", pExtCallback->pRestriction->pbBuffer[ i ] );
                }
                printf("\r\n\r\n");
            }
            break;
        case DRM_EXTENDED_RESTRICTION_ACTION_CALLBACK:
            {
                const DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT *pExtCallback = (const DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT*)f_pvPolicyCallbackData;
                DRM_DWORD i = 0;

                printf("  Got DRM_EXTENDED_RESTRICTION_ACTION_CALLBACK from Bind:\r\n");

                printf("    wRightID = %d\r\n", pExtCallback->wRightID);
                printf("    wType    = %d\r\n", pExtCallback->pRestriction->wType);
                printf("    wFlags   = 0x%x\r\n", pExtCallback->pRestriction->wFlags);

                printf("    Data     = ");
                for( i = pExtCallback->pRestriction->ibData; (i - pExtCallback->pRestriction->ibData) < pExtCallback->pRestriction->cbData; i++ )
                {
                    printf("0x%.2X ", pExtCallback->pRestriction->pbBuffer[ i ] );
                }
                printf("\r\n\r\n");
            }
            break;
    default:
        printf("  Callback from Bind with unknown callback type of %d.\r\n", f_dwCallbackType);

        /* Report that this callback type is not implemented */
        ChkDR( DRM_E_NOTIMPL );
    }

ErrorExit:
    return dr;
}

static std::string int_to_string(uint32_t i)
{
    std::ostringstream stream;
    stream << i;
    return stream.str();
}

static const uint16_t kWRMHEADERRecord = 0x1;

static void ParsePssh(std::string* pssh, std::string* wrmheader)
{
    wrmheader->clear();
    // PlayReady PSSH consists of:
    // 4 byte: size of pssh
    // 2 byte: PlayReady record count, followed by a sequence of records:
    //   2 byte: type of data (1: WRMHEADER, 3: embedded license store),
    //   2 byte: data length, exclusive
    //   finally, the blob of data
    uint32_t* ptr32 = (uint32_t*)pssh->data();
    if (*ptr32 != pssh->size()) {
        LOGE(("%s: pssh length doesn't match: psshLen=%u pssh.size=%u", __FUNCTION__, *ptr32, (uint32_t)pssh->size()));
        return;
    }

    uint16_t* ptr16 = (uint16_t*)++ptr32;
    uint16_t recordCount = *ptr16++;
    LOGD(("%s: recordCount=%u", __FUNCTION__, recordCount));

    for (; recordCount > 0; recordCount--) {
        uint16_t dataType = *ptr16++;
        uint16_t dataLen = *ptr16++;
        LOGD(("%s: dataType=%u", __FUNCTION__, dataType));
        LOGD(("%s: dataLen=%u", __FUNCTION__, dataLen));

        if (dataType == kWRMHEADERRecord) {
            wrmheader->assign((const char*)ptr16, dataLen);
            return;
        }

        uint8_t* ptr = (uint8_t*)ptr16;
        ptr16 = (uint16_t*)(ptr + dataLen);
    }
}

Playready30Decryptor::Playready30Decryptor()
    : BaseDecryptor()
{
    LOGD(("%s: enter", __FUNCTION__));
    s_sessionNum++;
    m_drmDecryptContext = NULL;
    m_valid = false;
}

Playready30Decryptor::~Playready30Decryptor()
{
    LOGD(("%s: enter", __FUNCTION__));

    if (m_drmDecryptContext != NULL) {
        LOGD(("%s: Drm_Reader_Close", __FUNCTION__));
        Drm_Reader_Close(m_drmDecryptContext);

        BKNI_Free(m_drmDecryptContext);
        m_drmDecryptContext = NULL;
    }

    s_sessionNum--;

    if (s_sessionNum == 0) {
        if (s_pDrmAppCtx != NULL) {
            LOGD(("%s: Drm_Cleanup_LicenseStore", __FUNCTION__));
            Drm_StoreMgmt_CleanupStore(s_pDrmAppCtx,
                DRM_STORE_CLEANUP_ALL,
                NULL,
                5,
                NULL);

            LOGD(("%s: Drm_Uninitialize AppCtx=%p", __FUNCTION__, (void*)s_pDrmAppCtx));
            Drm_Uninitialize(s_pDrmAppCtx);
            Oem_MemFree(s_pDrmAppCtx);
            s_pDrmAppCtx = NULL;
        }

        if (s_pbOpaqueBuffer != NULL) {
            Oem_MemFree(s_pbOpaqueBuffer);
            s_pbOpaqueBuffer = NULL;
        }

        if (s_pbRevocationBuffer != NULL) {
            Oem_MemFree(s_pbRevocationBuffer);
            s_pbRevocationBuffer = NULL;
        }

        if (s_pOEMContext != NULL) {
            LOGD(("%s: Drm_Platform_Uninitialize", __FUNCTION__));
            Drm_Platform_Uninitialize(s_pOEMContext);
            s_pOEMContext = NULL;
        }
    }

    LOGD(("%s: leaving", __FUNCTION__));
}

bool Playready30Decryptor::Initialize(std::string& pssh)
{
    /* DRM_Prdy specific */
    DRM_RESULT dr = DRM_SUCCESS;
    OEM_Settings oemSettings;

    DRM_WCHAR *hdsDir = bdrm_get_hds_dir();
    DRM_WCHAR *hdsFname = bdrm_get_pr3x_hds_fname();

    DRM_CONST_STRING sDstrHDSPath = DRM_EMPTY_DRM_STRING;
    DRM_WCHAR sRgwchHDSPath[DRM_MAX_PATH];

    DRM_DWORD dwEncryptionMode  = OEM_TEE_DECRYPTION_MODE_NOT_SECURE;

    dump_hex("Initialize: pssh", pssh.data(), pssh.size());

    m_pssh.assign(pssh);
    m_keyId.assign(pssh.substr(4));
    dump_hex("Initialize: keyId", m_keyId.data(), m_keyId.size());

    if (s_pDrmAppCtx == NULL) {
        BKNI_Memset(&oemSettings, 0, sizeof(oemSettings));
        oemSettings.binFileName = NULL;
        oemSettings.keyHistoryFileName = NULL;

        s_pDrmAppCtx = (DRM_APP_CONTEXT*)Oem_MemAlloc(sizeof(DRM_APP_CONTEXT));
        if (!s_pDrmAppCtx) {
            LOGE(("%s - failed to allocate pDrmAppCtx\n", __FUNCTION__));
            return false;
        }
        BKNI_Memset((uint8_t*)s_pDrmAppCtx, 0, sizeof(DRM_APP_CONTEXT));

        dr = Drm_Platform_Initialize((void *)&oemSettings);
        if (dr != DRM_SUCCESS) {
            LOGE(("%s: Drm_Platform_Initialize: 0x%x", __FUNCTION__, (unsigned)dr));
            return false;
        }

        s_pOEMContext = oemSettings.f_pOEMContext;
        if (!s_pOEMContext) {
            LOGE(("%s - invalid OEMContext\n", __FUNCTION__));
            return false;
        }
        LOGD(("%s - s_pOEMContext %p\n", __FUNCTION__, (void*)s_pOEMContext));

        /* Initialize OpaqueBuffer and RevocationBuffer */
        s_cbOpaqueBuffer = MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE;
        s_pbOpaqueBuffer = (uint8_t*)Oem_MemAlloc(MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE);
        if (s_pbOpaqueBuffer == NULL) {
            LOGE(("%s: failed to allocate s_pbOpaqueBuffer", __FUNCTION__));
            return false;
        }
        s_pbRevocationBuffer = (uint8_t*)Oem_MemAlloc(REVOCATION_BUFFER_SIZE);
        if (s_pbRevocationBuffer == NULL) {
            LOGE(("%s: failed to allocate s_pbRevocationBuffer", __FUNCTION__));
            return false;
        }

        /* Drm_Initialize */
        sDstrHDSPath.pwszString = sRgwchHDSPath;
        sDstrHDSPath.cchString = DRM_MAX_PATH;

        /* Convert the HDS path to DRM_STRING. */
        if (bdrm_get_hds_dir_lgth() > 0) {
            BKNI_Memcpy((DRM_WCHAR*)sDstrHDSPath.pwszString, hdsDir, bdrm_get_hds_dir_lgth() * sizeof(DRM_WCHAR));
        }
        BKNI_Memcpy((DRM_WCHAR*)sDstrHDSPath.pwszString + bdrm_get_hds_dir_lgth(),
            hdsFname, (bdrm_get_pr3x_hds_fname_lgth() + 1) * sizeof(DRM_WCHAR));


        if (hdsFname != NULL && bdrm_get_pr3x_hds_fname_lgth() > 0) {
            if (bdrm_get_hds_dir_lgth() > 0)
            {
                BKNI_Memcpy((DRM_WCHAR*)sDstrHDSPath.pwszString, hdsDir, bdrm_get_hds_dir_lgth() * sizeof(DRM_WCHAR));
                BKNI_Memcpy((DRM_WCHAR*)sDstrHDSPath.pwszString + bdrm_get_hds_dir_lgth(),
                        hdsFname, (bdrm_get_pr3x_hds_fname_lgth() + 1) * sizeof(DRM_WCHAR));
            }
        }

        dr = Drm_Initialize(s_pDrmAppCtx, s_pOEMContext,
            s_pbOpaqueBuffer, s_cbOpaqueBuffer, &sDstrHDSPath);

        if (dr != DRM_SUCCESS) {
            LOGE(("\n\n Leave ^^^^^^^^^^^^^^^^ %s::%d Failed to initialize PR30  ^^^^^^^^^^^^^^^^ \n\n", __FUNCTION__, __LINE__));
            return false;
        }
    }

// TODO: leve it for now
#ifdef SECURE_CLOCK_FEATURE //disable secure clock until stable
    /* Getting the current state of the secure clock*/
    uint32_t secClkStatus; /* secure clock status */

    dr = DRM_Prdy_SecureClock_GetStatus(s_drmHandle, &secClkStatus);
    if (dr !=  DRM_SUCCESS) {
        LOGE(("%s: DRM_Prdy_SecureClock_GetStatus: 0x%x", __FUNCTION__, (unsigned)dr));
        return false;
    }

    if ( secClkStatus != DRM_PRDY_CLK_SET) {
        /* setup the Playready secure clock */
        if (initSecureClock(s_drmHandle) != 0) {
            LOGE(("%d Failed to initiize Secure Clock, quitting....\n", __LINE__));
            return false;
        }
    }

    dr = DRM_Prdy_TurnSecureStop(m_DrmHandle, 1);
    if (dr !=  DRM_SUCCESS) {
        LOGE(("%s: DRM_Prdy_TurnSecureStop: 0x%x", __FUNCTION__, (unsigned)dr));
        return false;
    } else {
        m_IsSecureStopEnabled = true;
    }
#endif // SECURE_CLOCK_FEATURE

    dr = Drm_Revocation_SetBuffer(s_pDrmAppCtx,
        s_pbRevocationBuffer, REVOCATION_BUFFER_SIZE);
    if (dr != DRM_SUCCESS) {
        LOGE(("%s: Drm_Revocation_SetBuffer: 0x%x", __FUNCTION__, (unsigned)dr));
        return false;
    }

    m_drmDecryptContext = reinterpret_cast<DRM_DECRYPT_CONTEXT*>(Oem_MemAlloc(sizeof(DRM_DECRYPT_CONTEXT)));

    if (m_drmDecryptContext == NULL) {
        LOGE(("%s: failed to allocate m_DrmDecryptContext", __FUNCTION__));
        return false;
    }

    BKNI_Memset(m_drmDecryptContext, 0, sizeof(DRM_DECRYPT_CONTEXT));

    ParsePssh(&m_pssh, &m_wrmheader);
    if (m_wrmheader.empty()) {
        dump_hex("Initialize->SetProperty", m_pssh.data(), m_pssh.size());
        LOGD(("Initialize->SetProperty with pssh"));
        dr = Drm_Content_SetProperty(s_pDrmAppCtx,
            DRM_CSP_AUTODETECT_HEADER,
            (const uint8_t*)m_pssh.data(), m_pssh.size());
    } else {
        dump_hex("Initialize->SetProperty", m_wrmheader.data(), m_wrmheader.size());
        LOGD(("Initialize->SetProperty with wrmheader"));
        dr = Drm_Content_SetProperty(s_pDrmAppCtx,
            DRM_CSP_AUTODETECT_HEADER,
            (const uint8_t*)m_wrmheader.data(), m_wrmheader.size());
    }
    if (dr != DRM_SUCCESS) {
        LOGE(("%s: Drm_Content_SetProperty: 0x%x", __FUNCTION__, (unsigned)dr));
        return false;
    }

    /* set encryption/decryption mode */
    dwEncryptionMode = OEM_TEE_DECRYPTION_MODE_HANDLE;
    dr = Drm_Content_SetProperty(
        s_pDrmAppCtx, DRM_CSP_DECRYPTION_OUTPUT_MODE,
        (const DRM_BYTE*)&dwEncryptionMode, sizeof(DRM_DWORD)) ;
    if (dr != DRM_SUCCESS) {
        LOGE(("%s: Drm_Content_SetProperty: 0x%x", __FUNCTION__, (unsigned)dr));
        return false;
    }

    return true;
}

bool Playready30Decryptor::GenerateKeyRequest(std::string initData, dif_streamer::SessionType type)
{
    BSTD_UNUSED(initData);
    BSTD_UNUSED(type);
    DRM_RESULT dr = DRM_SUCCESS;
    char *pCh_url = NULL;
    uint8_t *pCh_data = NULL;
    DRM_DWORD urlLen;
    DRM_DWORD chLen;
    std::vector<unsigned char> challenge;
    DRM_CHAR *pszCustomDataUsed = NULL;
    DRM_DWORD cchCustomDataUsed = 0;
    const DRM_CONST_STRING *rgstrRights[ 1 ] = { &g_dstrWMDRM_RIGHT_PLAYBACK };
    DRMFILETIME               ftSystemTime; /* Initialized by Drm_SecureTime_GetValue */
    DRM_SECURETIME_CLOCK_TYPE eClockType;   /* Initialized by Drm_SecureTime_GetValue */


    dr = Drm_SecureTime_GetValue( s_pDrmAppCtx, &ftSystemTime, &eClockType  );
    if( (dr == DRM_E_SECURETIME_CLOCK_NOT_SET) || (dr == DRM_E_TEE_PROVISIONING_REQUIRED) )
    {
       /* setup the Playready secure clock */
       if(initSecureClock(s_pDrmAppCtx) != 0)
       {
           BDBG_ERR(("%d Failed to initiize Secure Clock, quitting....\n",__LINE__));
           return false;
       }
    }
    else if (dr == DRM_E_CLK_NOT_SUPPORTED)  /* Secure Clock not supported, try the Anti-Rollback Clock */
    {
        DRMSYSTEMTIME   systemTime;
       struct timeval  tv;
       struct tm      *tm;

       BDBG_LOG(("%d Secure Clock not supported, trying the Anti-Rollback Clock...\n",__LINE__));

       gettimeofday(&tv, NULL);
       tm = gmtime(&tv.tv_sec);

       systemTime.wYear         = tm->tm_year+1900;
       systemTime.wMonth        = tm->tm_mon+1;
       systemTime.wDayOfWeek    = tm->tm_wday;
       systemTime.wDay          = tm->tm_mday;
       systemTime.wHour         = tm->tm_hour;
       systemTime.wMinute       = tm->tm_min;
       systemTime.wSecond       = tm->tm_sec;
       systemTime.wMilliseconds = tv.tv_usec/1000;

       if(Drm_AntiRollBackClock_Init(s_pDrmAppCtx, &systemTime) != 0)
       {
           printf(" Failed to initiize Anti-Rollback Clock, quitting....\n");
           return false;
       }
    }
    else
    {
        BDBG_ERR(("%d Expect platform to support Secure Clock or Anti-Rollback Clock.  Possible certificate error.\n",__LINE__));
        return false;
    }

    dr = Drm_LicenseAcq_GenerateChallenge(
        s_pDrmAppCtx,
        rgstrRights,
        sizeof(rgstrRights)/sizeof(DRM_CONST_STRING*), /*1,*/
        NULL,
        pszCustomDataUsed,
        cchCustomDataUsed,
        NULL,
        &urlLen,
        NULL,
        NULL,
        NULL,
        &chLen,
        NULL);

    if (dr != DRM_E_BUFFERTOOSMALL) {
        LOGE(("%s: Drm_LicenseAcq_GenerateChallenge(): 0x%x", __FUNCTION__, (unsigned)dr));
        return false;
    }

    if (urlLen != 0)
        pCh_url = (char*)(BKNI_Malloc(urlLen));
    if (pCh_url == NULL) {
        LOGE(("%s: failed to allocate pCh_url", __FUNCTION__));
        return false;
    }

    if (chLen != 0)
        pCh_data = (uint8_t*)(BKNI_Malloc(chLen));
    if (pCh_data == NULL) {
        LOGE(("%s: failed to allocate pCh_data", __FUNCTION__));
        return false;
    }

    // Now get the challenge.
    dr = Drm_LicenseAcq_GenerateChallenge(
        s_pDrmAppCtx,
        rgstrRights,
        sizeof(rgstrRights)/sizeof(DRM_CONST_STRING*), /*1,*/
        NULL,
        pszCustomDataUsed,
        cchCustomDataUsed,
        pCh_url,
        &urlLen, /*(pUrl_len>0)?&cchURL:NULL, */
        NULL,
        NULL,
        pCh_data,
        &chLen,
        NULL);

    pCh_data[chLen] = 0;

    if (dr != DRM_SUCCESS) {
        LOGE(("%s: Drm_LicenseAcq_GenerateChallenge: 0x%x", __FUNCTION__, (unsigned)dr));
        if (pCh_url)
            BKNI_Free(pCh_url);
        if (pCh_data)
            BKNI_Free(pCh_data);
        return false;
    }

    // All done.
    m_sessionId.assign(int_to_string(s_nextSessionId++));

    if (pCh_url[0]) {
        m_defaultUrl.assign(pCh_url, urlLen - 1);
        LOGD(("default url(%d): \"%s\"", (uint32_t)m_defaultUrl.size(), m_defaultUrl.c_str()));
    }

    if ((pCh_data[0]) && chLen != 0) {
        m_keyMessage.assign((const char*)&pCh_data[0], chLen);
    }

    LOGD(("GenerateKeyRequest: keyMessage(%d): %s", (uint32_t)m_keyMessage.size(), m_keyMessage.c_str()));

    uint32_t numDeleted;
    dr = Drm_StoreMgmt_DeleteLicenses(s_pDrmAppCtx, NULL, NULL, (DRM_DWORD*)&numDeleted);

    if (dr != DRM_SUCCESS) {
        LOGE(("%s: Drm_StoreMgmt_DeleteLicenses: 0x%x", __FUNCTION__, (unsigned)dr));
        if (pCh_url)
            BKNI_Free(pCh_url);
        if (pCh_data)
            BKNI_Free(pCh_data);
        return false;
    }

    LOGD(("Drm_StoreMgmt_DeleteLicenses: numDeleted=%u", numDeleted));

    if (pCh_url)
        BKNI_Free(pCh_url);
    if (pCh_data)
        BKNI_Free(pCh_data);

    return true;
}

#ifdef USE_CURL
#include <curl/curl.h>
std::string s_pr30Buffer;

static uint32_t curl_writeback( void *ptr, uint32_t size, uint32_t nmemb, void *stream)
{
    BSTD_UNUSED(stream);
    s_pr30Buffer.append((char*)ptr, size * nmemb);
    return size * nmemb;
}

std::string Playready30Decryptor::GetKeyRequestResponse(std::string url)
{
    std::string drm_msg;
    std::string message = "";
    s_pr30Buffer.assign("");

    if (!url.empty()) {
        LOGD(("given url(%d): \"%s\"", (uint32_t)url.size(), url.c_str()));
        m_defaultUrl.assign(url);
    }

    size_t param_seek = m_defaultUrl.find("?");
    if (param_seek == std::string::npos) {
        m_defaultUrl.append("?PlayRight=1&SecurityLevel=3000");
    } else {
        param_seek = m_defaultUrl.find("SecurityLevel");
        if (param_seek == std::string::npos) {
            m_defaultUrl.append("&PlayRight=1&SecurityLevel=3000");
        }
    }

    LOGW(("%s: server_url(%d): %s", __FUNCTION__, (uint32_t)m_defaultUrl.size(), m_defaultUrl.c_str()));
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (curl == NULL) {
        LOGE(("%s: curl_easy_init returned NULL", __FUNCTION__));
        return drm_msg;
    }
    struct curl_slist *slist = NULL;
    slist = curl_slist_append(slist, "Accept: */*");
    slist = curl_slist_append(slist, "Content-Type: text/xml; charset=utf-8");
    slist = curl_slist_append(slist, "SOAPAction: \"http://schemas.microsoft.com/DRM/2007/03/protocols/AcquireLicense\"");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

//    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, m_defaultUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, m_keyMessage.data());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, m_keyMessage.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writeback);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)");
    res = curl_easy_perform(curl);
    LOGW(("%s: s_pr30Buffer(%d): %s, res: %d", __FUNCTION__, (uint32_t)s_pr30Buffer.size(), s_pr30Buffer.c_str(), res));

    if (res != 0) {
        LOGE(("%s: curl error %d", __FUNCTION__, res));
        curl_easy_cleanup(curl);
        return drm_msg;
    }
    size_t body_head = s_pr30Buffer.find("<soap:Envelope");
    if (body_head == std::string::npos) {
        LOGE(("%s: no body found in response", __FUNCTION__));
        curl_easy_cleanup(curl);
        return drm_msg;
    }
    drm_msg.clear();
    size_t drm_head = s_pr30Buffer.find("\r\n\r\n", body_head);
    if (drm_head != std::string::npos) {
        LOGD(("%s: DRM message found", __FUNCTION__));
        drm_head += 4;
        drm_msg = s_pr30Buffer.substr(drm_head);
    } else {
        LOGW(("%s: return body anyway", __FUNCTION__));
        drm_msg = s_pr30Buffer.substr(body_head);
    }

    LOGD(("HTTP response body: (%u bytes): %s", (uint32_t)drm_msg.size(), drm_msg.c_str()));
    curl_easy_cleanup(curl);
    return drm_msg;
}
#else // USE_CURL
#endif // USE_CURL

bool Playready30Decryptor::AddKey(std::string key)
{
    LOGD(("%s enter", __FUNCTION__));
    DRM_RESULT dr = DRM_SUCCESS;
    const DRM_CONST_STRING *rgstrRights[ 1 ] = { &g_dstrWMDRM_RIGHT_PLAYBACK };
    uint8_t *pbNewOpaqueBuffer = NULL;
    uint32_t cbNewOpaqueBuffer = s_cbOpaqueBuffer * 2;
    DRM_LICENSE_RESPONSE oResponse;

#if 1
    if (!m_wrmheader.empty()) {
        LOGD(("AddKey->SetProperty with wrmheader"));
        dr = Drm_Content_SetProperty(s_pDrmAppCtx,
            DRM_CSP_AUTODETECT_HEADER,
            (const uint8_t*)m_wrmheader.data(), m_wrmheader.size());
        if (dr != DRM_SUCCESS) {
            LOGE(("%s: Drm_Content_SetProperty: 0x%x", __FUNCTION__, (unsigned)dr));
            return false;
        }
    }
#endif

#ifdef SECURE_CLOCK_FEATURE //disable secure clock for stability issue
    if (m_IsSecureStopEnabled) {
        uint8_t tmpSessionIdBuf[16];

        rc = DRM_Prdy_LicenseAcq_ProcessResponseEx(s_drmHandle,
            key.c_str(), key.size(), tmpSessionIdBuf, NULL);
        if (rc != DRM_Prdy_ok) {
            LOGE(("%s: DRM_Prdy_LicenseAcq_ProcessResponseEx: 0x%x", __FUNCTION__, (unsigned)rc));
            return false;
        }
    } else {
        rc = DRM_Prdy_LicenseAcq_ProcessResponseEx(s_drmHandle,
            key.c_str(), key.size(), NULL, NULL);
        if (rc != DRM_Prdy_ok) {
            LOGE(("%s: DRM_Prdy_LicenseAcq_ProcessResponseEx: 0x%x", __FUNCTION__, (unsigned)rc));
            return false;
        }
    }
#else // SECURE_CLOCK_FEATURE

    BKNI_Memset(&oResponse, 0, sizeof(DRM_LICENSE_RESPONSE));

    dr = Drm_LicenseAcq_ProcessResponse(
        s_pDrmAppCtx,
        DRM_PROCESS_LIC_RESPONSE_NO_FLAGS,
        (const uint8_t *)key.c_str(),
        key.size(),
        &oResponse );

    if (dr != DRM_SUCCESS) {
        LOGE(("%s: Drm_LicenseAcq_ProcessResponse: 0x%x", __FUNCTION__, (unsigned)dr));
        return false;
    }
#endif // SECURE_CLOCK_FEATURE

    LOGD(("%s: calling Drm_Reader_Bind %p\n", __FUNCTION__, (void*)m_drmDecryptContext));

    while((dr = Drm_Reader_Bind(
       s_pDrmAppCtx,
       rgstrRights,
       1,
       (DRMPFNPOLICYCALLBACK)policy_callback,
       (void*)this,
       m_drmDecryptContext)) == DRM_E_BUFFERTOOSMALL)
    {
        BDBG_ASSERT(cbNewOpaqueBuffer > s_cbOpaqueBuffer); /* overflow check */

        if (cbNewOpaqueBuffer > DRM_MAXIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE)
        {
            LOGE(("%s: Drm_Reader_Bind: cbNewOpaqueBuffer too larg too large", __FUNCTION__));
            return false;
        }

        pbNewOpaqueBuffer = (uint8_t*)Oem_MemAlloc(cbNewOpaqueBuffer);
        if (pbNewOpaqueBuffer == NULL) {
            LOGE(("%s: failed to allocate pbNewOpaqueBuffer\n", __FUNCTION__));
            return false;
        }

        dr = Drm_ResizeOpaqueBuffer(
            s_pDrmAppCtx,
            pbNewOpaqueBuffer,
            cbNewOpaqueBuffer);

        /*
         Free the old buffer and then transfer the new buffer ownership
         Free must happen after Drm_ResizeOpaqueBuffer because that
         function assumes the existing buffer is still valid
        */
        SAFE_OEM_FREE(s_pbOpaqueBuffer);
        s_cbOpaqueBuffer = cbNewOpaqueBuffer;
        s_pbOpaqueBuffer = pbNewOpaqueBuffer;
        pbNewOpaqueBuffer = NULL;
    }

    if (DRM_FAILED(dr)) {
        if (dr == DRM_E_LICENSE_NOT_FOUND) {
            /* could not find a license for the KID */
            LOGE(("%s: no licenses found in the license store. Please request one from the license server.\n", __FUNCTION__));
        }
        else if(dr == DRM_E_LICENSE_EXPIRED) {
            /* License is expired */
            LOGE(("%s: License expired. Please request one from the license server.\n", __FUNCTION__));
        }
        else if(dr == DRM_E_RIV_TOO_SMALL ||
            dr == DRM_E_LICEVAL_REQUIRED_REVOCATION_LIST_NOT_AVAILABLE)
        {
            /* Revocation Package must be update */
            LOGE(("%s: Revocation Package must be update. 0x%x\n", __FUNCTION__,(unsigned)dr));
        }
        else {
            LOGE(("%s: unexpected failure during bind. 0x%x\n", __FUNCTION__,(unsigned)dr));
        }
    }

    LOGD(("%s: calling Drm_Reader_Commit dr 0x%x\n", __FUNCTION__, (unsigned)dr));
    dr = Drm_Reader_Commit(s_pDrmAppCtx, NULL, NULL);

    if (dr != DRM_SUCCESS) {
        LOGE(("%s: Drm_Reader_Commit: 0x%x\n", __FUNCTION__, (unsigned)dr));
        return false;
    }

    m_valid = true;

    LOGD(("%s leaving", __FUNCTION__));
    return true;
}

#if 0 // FIXME
bool Playready30Decryptor::GetProtectionPolicy(DRM_Prdy_policy_t *policy)
{
    DRM_Prdy_Error_e rc;
    LOGD(("%s enter", __FUNCTION__));
    do {
        rc = DRM_Prdy_Get_Protection_Policy(s_drmHandle, policy);

        if ((rc != DRM_Prdy_ok) && (rc != DRM_Prdy_no_policy)) {
            LOGE(("Leave %s Error DRM_Prdy_Get_Protection_Policy 0x%08lx",
                              __FUNCTION__, static_cast<unsigned long>(rc)));
            return false;
        }
    } while (rc != DRM_Prdy_no_policy);

    LOGD(("%s leaving", __FUNCTION__));
    return true;
}
#endif

bool Playready30Decryptor::CancelKeyRequest()
{
    // TODO
    return true;
}

uint32_t Playready30Decryptor::DecryptSample(
    SampleInfo *pSample,
    IBuffer *input,
    IBuffer *output,
    uint32_t sampleSize)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_AES_COUNTER_MODE_CONTEXT aesCtrInfo;

    uint8_t i = 0;
    uint32_t bytes_processed = 0;

    uint32_t clearSize = 0;
    uint32_t encSize = 0;
    for (i = 0; i < pSample->nbOfEntries; i++) {
        clearSize += pSample->entries[i].bytesOfClearData;
        encSize += pSample->entries[i].bytesOfEncData;
    }
    LOGD(("%s: sampleSize=%u clearSize=%u encSize=%u", __FUNCTION__, sampleSize, clearSize, encSize));

    uint8_t *encrypted_buffer = NULL;

    output->Copy(0, input->GetPtr(), sampleSize);
    encrypted_buffer = (uint8_t*)output->GetPtr();

    // IV
    uint64_t playready_iv = 0LL;
    if (BKNI_Memcmp(&pSample->iv[0], &playready_iv, sizeof(playready_iv)) != 0) {
        BKNI_Memcpy(&playready_iv, &pSample->iv[0], sizeof(playready_iv));
        playready_iv = bswap_64(playready_iv);
        BKNI_Memcpy(&aesCtrInfo.qwInitializationVector, &playready_iv, sizeof(playready_iv));
        dump_hex("iv", (const char*)&aesCtrInfo.qwInitializationVector, 16);
    }

    aesCtrInfo.qwBlockOffset = 0;
    aesCtrInfo.bByteOffset = 0;

    if (encSize == 0) {
        if (playready_iv != 0LL) {
            uint32_t encryptedRegionMappings[2];
            encryptedRegionMappings[0] = 0; /* 0 bytes of clear */
            encryptedRegionMappings[1] = sampleSize; /* 0 bytes of clear */

            dr = Drm_Reader_DecryptOpaque(
                m_drmDecryptContext,
                2,
                encryptedRegionMappings,
                aesCtrInfo.qwInitializationVector,
                sampleSize,
                encrypted_buffer,
                &sampleSize,
                &encrypted_buffer);

            if (dr != DRM_SUCCESS) {
                LOGE(("%s: %d Reader_Decrypt failed: 0x%x", __FUNCTION__, __LINE__, (unsigned)dr));
                return bytes_processed;
            }
        }
        bytes_processed += sampleSize;
        LOGD(("%s: bytes_processed=%u", __FUNCTION__, bytes_processed));
        return bytes_processed;
    }

    uint32_t *pEncryptedRegionMappings = (uint32_t*)BKNI_Malloc(sizeof(uint32_t) * pSample->nbOfEntries * 2);
    uint32_t entryNb = 0;

    for (i = 0; i <  pSample->nbOfEntries; i++) {
        uint32_t num_clear = pSample->entries[i].bytesOfClearData;
        uint32_t num_enc = pSample->entries[i].bytesOfEncData;

        pEncryptedRegionMappings[entryNb++] = num_clear;
        pEncryptedRegionMappings[entryNb++] = num_enc;
    }

    dr = Drm_Reader_DecryptOpaque(
        m_drmDecryptContext,
        pSample->nbOfEntries * 2,
        pEncryptedRegionMappings,
        aesCtrInfo.qwInitializationVector,
        sampleSize,
        encrypted_buffer,
        &sampleSize,
        &encrypted_buffer);

    BKNI_Free(pEncryptedRegionMappings);

    if (dr != DRM_SUCCESS) {
        LOGE(("%s: %d Reader_Decrypt failed: 0x%x", __FUNCTION__, __LINE__, (unsigned)dr));
        return bytes_processed;
    }

    bytes_processed += sampleSize;
    LOGD(("%s: bytes_processed=%u", __FUNCTION__, bytes_processed));
    return bytes_processed;
}
