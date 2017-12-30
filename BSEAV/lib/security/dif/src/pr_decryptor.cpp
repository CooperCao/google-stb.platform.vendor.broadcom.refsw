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

#include "pr_decryptor.h"

#include "drm_prdy_http.h"

BDBG_MODULE(pr_decryptor);
#include "dump_hex.h"

using namespace dif_streamer;

DRM_Prdy_Handle_t PlayreadyDecryptor::s_drmHandle = NULL;
uint32_t PlayreadyDecryptor::s_nextSessionId = 1;
uint8_t PlayreadyDecryptor::s_sessionNum = 0;

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
        LOGE(("%s: pssh length doesn't match: psshLen=%lu pssh.size=%u", BSTD_FUNCTION, (long unsigned)*ptr32, (uint32_t)pssh->size()));
        return;
    }

    uint16_t* ptr16 = (uint16_t*)++ptr32;
    uint16_t recordCount = *ptr16++;
    LOGD(("%s: recordCount=%u", BSTD_FUNCTION, recordCount));

    for (; recordCount > 0; recordCount--) {
        uint16_t dataType = *ptr16++;
        uint16_t dataLen = *ptr16++;
        LOGD(("%s: dataType=%u", BSTD_FUNCTION, dataType));
        LOGD(("%s: dataLen=%u", BSTD_FUNCTION, dataLen));

        if (dataType == kWRMHEADERRecord) {
            wrmheader->assign((const char*)ptr16, dataLen);
            return;
        }

        uint8_t* ptr = (uint8_t*)ptr16;
        ptr16 = (uint16_t*)(ptr + dataLen);
    }
}

PlayreadyDecryptor::PlayreadyDecryptor()
    : BaseDecryptor()
{
    LOGD(("%s: enter", BSTD_FUNCTION));
    s_sessionNum++;
    m_drmDecryptContext = NULL;
    m_valid = false;
}

PlayreadyDecryptor::~PlayreadyDecryptor()
{
    LOGD(("%s: enter", BSTD_FUNCTION));
    DRM_Prdy_Error_e rc = DRM_Prdy_ok;

    if (m_drmDecryptContext != NULL) {
        rc = DRM_Prdy_Reader_Close(m_drmDecryptContext);
        LOGW(("DRM_Prdy_Reader_Close done"));

        if (rc != DRM_Prdy_ok) {
            LOGE(("DRM_Prdy_Reader_Close rc %x", rc));
        }

        BKNI_Free(m_drmDecryptContext);
        m_drmDecryptContext = NULL;
    }

    s_sessionNum--;

    if (s_sessionNum == 0) {
        if (s_drmHandle != NULL) {
            LOGD(("DRM_Prdy_Uninitialize s_drmHandle:%p\n", (void*)s_drmHandle));
            rc = DRM_Prdy_Uninitialize(s_drmHandle);
            if (rc != DRM_Prdy_ok) {
                LOGE(("DRM_Prdy_Uninitialize failed rc %x", rc));
            }

            s_drmHandle = NULL;
        }
    }

    LOGD(("%s: leaving", BSTD_FUNCTION));
}

bool PlayreadyDecryptor::Initialize(std::string& pssh)
{
    dump_hex("Initialize: pssh", pssh.data(), pssh.size());
    DRM_Prdy_Error_e rc = DRM_Prdy_ok;

    m_pssh.assign(pssh);
    m_keyId.assign(pssh.substr(4));
    dump_hex("Initialize: keyId", m_keyId.data(), m_keyId.size());

    if (s_drmHandle == NULL) {
        DRM_Prdy_Init_t initSettings;
        DRM_Prdy_GetDefaultParamSettings(&initSettings);
        s_drmHandle = DRM_Prdy_Initialize(&initSettings);
        LOGD(("s_drmHandle:%p\n", (void*)s_drmHandle));

        if (!s_drmHandle) {
            LOGE(("\n\n Leave ^^^^^^^^^^^^^^^^ %s::%d Failed to create DRM Handle ^^^^^^^^^^^^^^^^ \n\n", BSTD_FUNCTION, __LINE__));
            return false;
        }
    }

#ifdef SECURE_CLOCK_FEATURE //disable secure clock until stable
    /* Getting the current state of the secure clock*/
    uint32_t secClkStatus; /* secure clock status */

    rc = DRM_Prdy_SecureClock_GetStatus(s_drmHandle, &secClkStatus);
    if (rc !=  DRM_Prdy_ok) {
        LOGE(("%s: DRM_Prdy_SecureClock_GetStatus: 0x%x", BSTD_FUNCTION, rc));
        return false;
    }

    if ( secClkStatus != DRM_PRDY_CLK_SET) {
        /* setup the Playready secure clock */
        if (initSecureClock(s_drmHandle) != 0) {
            LOGE(("%d Failed to initiize Secure Clock, quitting....\n", __LINE__));
            return false;
        }
    }

    rc = DRM_Prdy_TurnSecureStop(m_DrmHandle, 1);
    if (rc !=  DRM_Prdy_ok) {
        LOGE(("%s: DRM_Prdy_TurnSecureStop: 0x%x", BSTD_FUNCTION, rc));
        return false;
    } else {
        m_IsSecureStopEnabled = true;
    }
#endif // SECURE_CLOCK_FEATURE

    m_drmDecryptContext = reinterpret_cast<DRM_Prdy_DecryptContext_t*>(BKNI_Malloc(sizeof(DRM_Prdy_DecryptContext_t)));

    if (m_drmDecryptContext == NULL) {
        LOGE(("failed to allocate m_DrmDecryptContext"));
        return false;
    }

    BKNI_Memset(m_drmDecryptContext, 0, sizeof(DRM_Prdy_DecryptContext_t));

    ParsePssh(&m_pssh, &m_wrmheader);
    if (m_wrmheader.empty()) {
        dump_hex("Initialize->SetProperty", m_pssh.data(), m_pssh.size());
        LOGD(("Initialize->SetProperty with pssh"));
        rc = DRM_Prdy_Content_SetProperty(s_drmHandle,
            DRM_Prdy_contentSetProperty_eAutoDetectHeader,
            (const uint8_t*)m_pssh.data(), m_pssh.size());
    } else {
        dump_hex("Initialize->SetProperty", m_wrmheader.data(), m_wrmheader.size());
        LOGD(("Initialize->SetProperty with wrmheader"));
        rc = DRM_Prdy_Content_SetProperty(s_drmHandle,
            DRM_Prdy_contentSetProperty_eAutoDetectHeader,
            (const uint8_t*)m_wrmheader.data(), m_wrmheader.size());
    }
    if (rc != DRM_Prdy_ok) {
        LOGE(("%s: DRM_Prdy_Content_SetProperty: 0x%x", BSTD_FUNCTION, rc));
        return false;
    }

    return true;
}

bool PlayreadyDecryptor::GenerateKeyRequest(std::string initData, dif_streamer::SessionType type)
{
    BSTD_UNUSED(initData);
    BSTD_UNUSED(type);
    uint32_t challengeSize;
    char     *pCh_url = NULL;
    uint32_t pUrl_len;
    // Figure out the buffer allocation sizes for the challenge.
    DRM_Prdy_Error_e rc = DRM_Prdy_ok;
    std::vector<unsigned char> challenge;
    rc =  DRM_Prdy_Get_Buffer_Size(
              s_drmHandle,
              DRM_Prdy_getBuffer_licenseAcq_challenge,
              NULL,
              0,
              (uint32_t *) &pUrl_len,/*&cchURL,*/
              (uint32_t *)&challengeSize);

    if ( rc != DRM_Prdy_ok ) {
        LOGE(("%s: DRM_Prdy_Get_Buffer_Size: 0x%x", BSTD_FUNCTION, rc));
        return false;
    }

    LOGD(("challengeSize:%d, challenge.size:%d\n", challengeSize, (uint32_t)challenge.size()));
    // Now get the challenge.
    challenge.resize(challengeSize);
    LOGD(("challengeSize:%d, challenge.resize:%d, pUrl_len:%d\n", challengeSize, (uint32_t)challenge.size(), pUrl_len));

    if (pUrl_len != 0)
        pCh_url = (char*)(BKNI_Malloc(pUrl_len));

    LOGD(("pCh_url:%p\n", pCh_url));
    rc  = DRM_Prdy_LicenseAcq_GenerateChallenge(
              s_drmHandle,
              NULL, 0,
              pCh_url, &pUrl_len,
              (char *)&challenge[0],
              &challengeSize);

    if (rc != DRM_Prdy_ok) {
        LOGE(("%s: DRM_Prdy_LicenseAcq_GenerateChallenge: 0x%x", BSTD_FUNCTION, rc));
        if (pCh_url)
            BKNI_Free(pCh_url);
        return false;
    }
    LOGD(("challengeSize:%d\n", challengeSize));

    // All done.
    if (rc == NEXUS_SUCCESS) {
        m_sessionId.assign(int_to_string(s_nextSessionId++));

        if (challenge[0]) {
            m_defaultUrl.assign(pCh_url, pUrl_len);
            LOGD(("default url(%d): %s\n", (uint32_t)m_defaultUrl.size(), m_defaultUrl.c_str()));
        }

        if ((challenge[0]) && (challengeSize)) {
            m_keyMessage.assign((const char*)&challenge[0], challengeSize);
        }

        LOGD(("GenerateKeyRequest: keyMessage(%d): %s", (uint32_t)m_keyMessage.size(), m_keyMessage.c_str()));

        uint32_t numDeleted;
        DRM_Prdy_Error_e rc;
        rc =  DRM_Prdy_StoreMgmt_DeleteLicenses(s_drmHandle, NULL, 0, &numDeleted);

        if (rc != DRM_Prdy_ok) {
            LOGE(("%s: DRM_Prdy_StoreMgmt_DeleteLicenses: 0x%x", BSTD_FUNCTION, rc));
            if (pCh_url)
                BKNI_Free(pCh_url);
            return false;
        }

        LOGD(("DRM_Prdy_StoreMgmt_DeleteLicenses: numDeleted=%u", numDeleted));
    }

    if (pCh_url)
        BKNI_Free(pCh_url);

    return true;
}

#ifdef USE_CURL
#include <curl/curl.h>
std::string s_prBuffer;

static uint32_t curl_writeback( void *ptr, uint32_t size, uint32_t nmemb, void *stream)
{
    BSTD_UNUSED(stream);
    s_prBuffer.append((char*)ptr, size * nmemb);
    return size * nmemb;
}

std::string PlayreadyDecryptor::GetKeyRequestResponse(std::string url)
{
    std::string drm_msg;
    std::string message = "";
    s_prBuffer.assign("");

    if (!url.empty()) {
        m_defaultUrl.assign(url);
    }

    LOGW(("%s: server_url: %s", BSTD_FUNCTION, m_defaultUrl.c_str()));
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (curl == NULL) {
        LOGE(("%s: curl_easy_init returned NULL", BSTD_FUNCTION));
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
    LOGW(("%s: s_prBuffer(%d): %s, res: %d", BSTD_FUNCTION, (uint32_t)s_prBuffer.size(), s_prBuffer.c_str(), res));

    if (res != 0) {
        LOGE(("%s: curl error %d", BSTD_FUNCTION, res));
        curl_easy_cleanup(curl);
        return drm_msg;
    }
    size_t body_head = s_prBuffer.find("<soap:Envelope");
    if (body_head == std::string::npos) {
        LOGE(("%s: no body found in response", BSTD_FUNCTION));
        curl_easy_cleanup(curl);
        return drm_msg;
    }
    drm_msg.clear();
    size_t drm_head = s_prBuffer.find("\r\n\r\n", body_head);
    if (drm_head != std::string::npos) {
        LOGD(("%s: DRM message found", BSTD_FUNCTION));
        drm_head += 4;
        drm_msg = s_prBuffer.substr(drm_head);
    } else {
        LOGW(("%s: return body anyway", BSTD_FUNCTION));
        drm_msg = s_prBuffer.substr(body_head);
    }

    LOGD(("HTTP response body: (%u bytes): %s", (uint32_t)drm_msg.size(), drm_msg.c_str()));
    curl_easy_cleanup(curl);
    return drm_msg;
}
#else // USE_CURL
std::string PlayreadyDecryptor::GetKeyRequestResponse(std::string url)
{
    LOGD(("%s: url(%d): %s", BSTD_FUNCTION, url.size(), url.c_str()));

    uint8_t resp_buffer[64*1024];
    uint8_t *pResponse = resp_buffer;
    uint32_t respLen;
    uint32_t respOffset;
    uint32_t rc;

    std::string drm_msg;

    if (!url.empty()) {
        m_defaultUrl.assign(url);
    }

    // Just use the URL from DRM_Prdy_LicenseAcq_GenerateChallenge
    rc = DRM_Prdy_http_client_license_post_soap((char*)m_defaultUrl.c_str(),
        (char*)m_keyMessage.data(), 1, 150, (unsigned char **)&pResponse,
        &respOffset, &respLen);
    if (rc != 0) {
        LOGE(("%s: DRM_Prdy_http_client_license_post_soap(): 0x%x", BSTD_FUNCTION, rc));
        return drm_msg;
    }

    drm_msg.assign((const char*)&pResponse[respOffset], respLen);

    LOGD(("GetKeyRequestResponse: response body(%d): %s", drm_msg.size(), drm_msg.c_str()));

    return drm_msg;
}
#endif // USE_CURL

bool PlayreadyDecryptor::AddKey(std::string key)
{
    LOGD(("%s enter", BSTD_FUNCTION));
    DRM_Prdy_Error_e rc;

    if (!m_wrmheader.empty()) {
        LOGD(("AddKey->SetProperty with wrmheader"));
        rc = DRM_Prdy_Content_SetProperty(s_drmHandle,
            DRM_Prdy_contentSetProperty_eAutoDetectHeader,
            (const uint8_t*)m_wrmheader.data(), m_wrmheader.size());
        if (rc != DRM_Prdy_ok) {
            LOGE(("%s: DRM_Prdy_Content_SetProperty: 0x%x", BSTD_FUNCTION, rc));
            return false;
        }
    }

#ifdef SECURE_CLOCK_FEATURE //disable secure clock for stability issue
    if (m_IsSecureStopEnabled) {
        uint8_t tmpSessionIdBuf[16];

        rc = DRM_Prdy_LicenseAcq_ProcessResponseEx(s_drmHandle,
            key.c_str(), key.size(), tmpSessionIdBuf, NULL);
        if (rc != DRM_Prdy_ok) {
            LOGE(("%s: DRM_Prdy_LicenseAcq_ProcessResponseEx: 0x%x", BSTD_FUNCTION, rc));
            return false;
        }
    } else {
        rc = DRM_Prdy_LicenseAcq_ProcessResponseEx(s_drmHandle,
            key.c_str(), key.size(), NULL, NULL);
        if (rc != DRM_Prdy_ok) {
            LOGE(("%s: DRM_Prdy_LicenseAcq_ProcessResponseEx: 0x%x", BSTD_FUNCTION, rc));
            return false;
        }
    }
#else // SECURE_CLOCK_FEATURE
    rc = DRM_Prdy_LicenseAcq_ProcessResponse(s_drmHandle,
        key.c_str(), key.size(), NULL);

    if (rc != DRM_Prdy_ok) {
        LOGE(("%s: DRM_Prdy_LicenseAcq_ProcessResponse: 0x%x", BSTD_FUNCTION, rc));
        return false;
    }
#endif // SECURE_CLOCK_FEATURE

    LOGD(("%s: calling DRM_Prdy_Reader_Bind %p\n", BSTD_FUNCTION, (void*)m_drmDecryptContext));
    rc = DRM_Prdy_Reader_Bind(s_drmHandle, m_drmDecryptContext);

    if (rc != DRM_Prdy_ok) {
        LOGE(("%s: DRM_Prdy_Reader_Bind: 0x%x\n", BSTD_FUNCTION, rc));
        return false;
    }

    LOGD(("%s: calling DRM_Prdy_Reader_Commit", BSTD_FUNCTION));
    rc = DRM_Prdy_Reader_Commit(s_drmHandle);

    if (rc != DRM_Prdy_ok) {
        LOGE(("%s: DRM_Prdy_Reader_Commit: 0x%x\n", BSTD_FUNCTION, rc));
        return false;
    }

    m_valid = true;

    LOGD(("%s leaving", BSTD_FUNCTION));
    return true;
}

bool PlayreadyDecryptor::GetProtectionPolicy(DRM_Prdy_policy_t *policy)
{
    DRM_Prdy_Error_e rc;
    LOGD(("%s enter", BSTD_FUNCTION));
    do {
        rc = DRM_Prdy_Get_Protection_Policy(s_drmHandle, policy);

        if ((rc != DRM_Prdy_ok) && (rc != DRM_Prdy_no_policy)) {
            LOGE(("Leave %s Error DRM_Prdy_Get_Protection_Policy 0x%08lx",
                              BSTD_FUNCTION, static_cast<unsigned long>(rc)));
            return false;
        }
    } while (rc != DRM_Prdy_no_policy);

    LOGD(("%s leaving", BSTD_FUNCTION));
    return true;
}

bool PlayreadyDecryptor::CancelKeyRequest()
{
    // TODO
    return true;
}

uint32_t PlayreadyDecryptor::DecryptSample(
    SampleInfo *pSample,
    IBuffer *input,
    IBuffer *output,
    uint32_t sampleSize)
{
    DRM_Prdy_Error_e rc = DRM_Prdy_ok;
    DRM_Prdy_AES_CTR_Info_t ctrContext;

    uint8_t i = 0;
    uint32_t bytes_processed = 0;

    uint32_t clearSize = 0;
    uint32_t encSize = 0;
    for (i = 0; i < pSample->nbOfEntries; i++) {
#ifdef USE_PR_DECRYPT_OPAQUE
        // Transfer clear data
        // assuming clear data comes first in subsamples
        if (pSample->entries[i].bytesOfClearData > 0)
            output->Copy(clearSize + encSize,
                input->GetPtr() + clearSize + encSize,
                pSample->entries[i].bytesOfClearData);
#endif

        clearSize += pSample->entries[i].bytesOfClearData;
        encSize += pSample->entries[i].bytesOfEncData;
    }
    LOGD(("%s: sampleSize=%u clearSize=%u encSize=%u", BSTD_FUNCTION, sampleSize, clearSize, encSize));

    uint8_t *encrypted_buffer = NULL;

#ifdef USE_PR_DECRYPT_OPAQUE
    uint8_t *decrypted_buffer = NULL;
    encrypted_buffer = (uint8_t*)input->GetPtr();
    decrypted_buffer = (uint8_t*)output->GetPtr();
#else
    output->Copy(0, input->GetPtr(), sampleSize);
    encrypted_buffer = (uint8_t*)output->GetPtr();
#endif

    // IV
    uint64_t playready_iv = 0LL;
    if (BKNI_Memcmp(&pSample->iv[0], &playready_iv, sizeof(playready_iv)) != 0) {
        BKNI_Memcpy(&playready_iv, &pSample->iv[0], sizeof(playready_iv));
        playready_iv = bswap_64(playready_iv);
        BKNI_Memcpy(&ctrContext.qwInitializationVector, &playready_iv, sizeof(playready_iv));
        dump_hex("iv", (const char*)&ctrContext.qwInitializationVector, 16);
    }

    ctrContext.qwBlockOffset = 0;
    ctrContext.bByteOffset = 0;

    if (encSize == 0) {
        if (playready_iv != 0LL) {
            rc = DRM_Prdy_Reader_Decrypt(m_drmDecryptContext, &ctrContext,
                encrypted_buffer, sampleSize);
            if (rc != DRM_Prdy_ok) {
                LOGE(("%s: %d Reader_Decrypt failed: 0x%x", BSTD_FUNCTION, __LINE__, rc));
                return bytes_processed;
            }
        }
        bytes_processed += sampleSize;
        LOGD(("%s: bytes_processed=%u", BSTD_FUNCTION, bytes_processed));
        return bytes_processed;
    }

    uint64_t     qwOffset = 0;

    for (i = 0; i <  pSample->nbOfEntries; i++) {
        uint32_t num_clear = pSample->entries[i].bytesOfClearData;
        uint32_t num_enc = pSample->entries[i].bytesOfEncData;

        encrypted_buffer += num_clear;
#ifdef USE_PR_DECRYPT_OPAQUE
        decrypted_buffer += num_clear;
#endif

        if (num_enc > 0) {
            ctrContext.qwBlockOffset = qwOffset / 16;
            ctrContext.bByteOffset = qwOffset % 16;

#ifdef USE_PR_DECRYPT_OPAQUE
            rc = DRM_Prdy_Reader_DecryptOpaque(m_drmDecryptContext, &ctrContext,
                encrypted_buffer, decrypted_buffer, num_enc);
#else
            rc = DRM_Prdy_Reader_Decrypt(m_drmDecryptContext, &ctrContext,
                encrypted_buffer, num_enc);
#endif
            if (rc != DRM_Prdy_ok) {
                LOGE(("%s: %d Reader_Decrypt failed: 0x%x", BSTD_FUNCTION, __LINE__, rc));
                return bytes_processed;
            }
        }

        encrypted_buffer += num_enc;
        qwOffset += num_enc;
        bytes_processed += num_clear + num_enc;
    }

    LOGD(("%s: bytes_processed=%u", BSTD_FUNCTION, bytes_processed));
    return bytes_processed;
}
