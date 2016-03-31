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
 *
 * Module Description:
 *
 * DRM Integration Framework
 *
 *****************************************************************************/
#define LOG_NDEBUG 0
#define LOG_TAG "wv_decryptor"
#include <utils/Log.h>
#define LOGE(x) ALOGE x
#define LOGW(x) ALOGW x
#define LOGD(x) ALOGD x
#define LOGV(x) ALOGV x

#include "string_conversions.h"
#include <arpa/inet.h>

#include "android_wv_decryptor.h"

using namespace wvcdm;
using namespace dif_streamer;

std::string s_wvBuffer;

#ifdef USE_CURL
#include <curl/curl.h>

static size_t curl_writeback( void *ptr, size_t size, size_t nmemb, void *stream)
{
    BSTD_UNUSED(stream);
    s_wvBuffer.append((char*)ptr, size * nmemb);
    return size * nmemb;
}
#else
#include "url_request.h"
#endif // USE_CURL

static std::string GetCertRequestResponse(
    std::string& keyMessage, std::string& server_url)
{
    std::string message = "";
    s_wvBuffer.assign("");
#ifdef USE_CURL
    server_url += "&signedRequest=";
    server_url += keyMessage;
    LOGD(("%s: server_url: %s", __FUNCTION__, server_url.c_str()));
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, server_url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, 0);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writeback);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        res = curl_easy_perform(curl);
        LOGD(("s_wvBuffer(%d): %s, res: %d", s_wvBuffer.size(), s_wvBuffer.c_str(), res));
        message = s_wvBuffer;

        curl_easy_cleanup(curl);
    }
#else // USE_CURL
    // Use secure connection and chunk transfer coding.
    UrlRequest url_request(server_url);
    if (!url_request.is_connected()) {
      return "";
    }

    url_request.PostCertRequestInQueryString(keyMessage);
    int resp_bytes = url_request.GetResponse(&message);
    LOGW(("end %d bytes response dump", resp_bytes));

    int status_code = url_request.GetStatusCode(message);
    if (status_code != 200) {
        LOGE(("GetStatusCode error: %d", status_code));
    }
#endif // USE_CURL
    return message;
}

WidevineDecryptor::WidevineDecryptor()
    : BaseDecryptor()
{
    LOGD(("%s: enter", __FUNCTION__));
    m_cdm = NULL;
    m_valid = false;
}

WidevineDecryptor::~WidevineDecryptor()
{
    LOGD(("%s: enter", __FUNCTION__));
    if (m_cdm == NULL)
        return;

    CancelKeyRequest();
    m_cdm->CloseSession(m_sessionId);
    delete m_cdm;
    m_cdm = NULL;

    LOGD(("%s: leave", __FUNCTION__));
}

bool WidevineDecryptor::Initialize(std::string& pssh)
{
    LOGD(("%s: pssh(%d): %s", __FUNCTION__, pssh.size(), b2a_hex(pssh).c_str()));
    m_pssh.assign(pssh);
    m_keyId.assign(pssh.substr(4));
    LOGD(("%s: default keyId(%d): %s", __FUNCTION__, m_keyId.size(), b2a_hex(m_keyId).c_str()));

    if (m_cdm == NULL)
        m_cdm = new WvContentDecryptionModule();

    CdmResponseType status = m_cdm->OpenSession(
        kWidevineKeySystem, NULL, &m_sessionId);
    LOGD(("OpenSession returned: %d", status));
    if (status == NO_ERROR)
        return true;
    else if (status != NEED_PROVISIONING)
        return false;

    LOGD(("calling GetProvisioningRequest"));
    std::string provisioning_server_url;
    std::string cert_authority;

    status = m_cdm->GetProvisioningRequest(
        kCertificateWidevine, cert_authority, &m_keyMessage,
        &provisioning_server_url);
    if (NO_ERROR != status) {
        LOGE(("GetProvisioningRequest Error: %d", status));
        return false;
    }
    LOGD(("GetProvisioningRequest: url=%s", provisioning_server_url.c_str()));

    std::string response = GetCertRequestResponse(m_keyMessage, provisioning_server_url);

    LOGD(("calling HandleProvisioningResponse"));
    std::string cert, wrapped_key;
    status = m_cdm->HandleProvisioningResponse(
        response, &cert, &wrapped_key);
    if (NO_ERROR != status) {
        LOGE(("HandleProvisioningResponse Error: %d", status));
        return false;
    }
    LOGD(("HandleProvisioningResponse returned status=%d", status));

    LOGD(("calling CloseSession session_id: %s", m_sessionId.c_str()));
    status = m_cdm->CloseSession(m_sessionId);
    LOGD(("CloseSession returned status=%d", status));
    status = m_cdm->OpenSession(
        kWidevineKeySystem, NULL, &m_sessionId);
    if (NO_ERROR != status) {
        LOGE(("OpenSession Error: %d", status));
        return false;
    }

    // TODO: Register for events?
/*
    WVEventListener eventListener;
    bool listenerAttached =
        m_cdm->AttachEventListener(m_sessionId, &eventListener);
*/

    return true;
}

bool WidevineDecryptor::GenerateKeyRequest(std::string initData)
{
LOGD(("%s: %d initData= %s", __FUNCTION__, __LINE__, b2a_hex(initData).c_str()));
    CdmAppParameterMap app_parameters;
    std::string server_url;

    m_cdm->GenerateKeyRequest(
        m_sessionId, m_keySetId, "video/mp4",
        initData, kLicenseTypeStreaming, app_parameters, NULL,
        &m_keyMessage, &server_url);
    LOGD(("GenerateKeyRequest returned session_id: %s", m_sessionId.c_str()));
    m_initData.assign(initData);

    return true;
}

std::string WidevineDecryptor::GetKeyRequestResponse(std::string url)
{
    LOGD(("%s: keyMessage(%d): %s", __FUNCTION__, m_keyMessage.size(), b2a_hex(m_keyMessage).c_str()));
    std::string drm_msg;
    std::string message = "";
    s_wvBuffer.assign("");
#ifdef USE_CURL
    LOGW(("%s: server_url: %s", __FUNCTION__, url.c_str()));
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (curl == NULL) {
        LOGE(("%s: curl_easy_init returned NULL", __FUNCTION__));
        return drm_msg;
    }
//    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, m_keyMessage.data());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, m_keyMessage.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writeback);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Widevine CDM v1.0");
    res = curl_easy_perform(curl);
    LOGD(("s_wvBuffer: %s, res: %d", s_wvBuffer.c_str(), res));

    if (res != 0) {
        LOGE(("%s: curl error %d", __FUNCTION__, res));
        curl_easy_cleanup(curl);
        return drm_msg;
    }
#else // USE_CURL
    UrlRequest url_request(url);
    if (!url_request.is_connected()) {
        return "";
    }
    url_request.PostRequest(m_keyMessage);
    int resp_bytes = url_request.GetResponse(&message);

    int status_code = url_request.GetStatusCode(message);

    LOGD(("GetStatusCode returned %d", status_code));
    if (status_code != 200) {
        LOGE(("%s: error status_code=%d", __FUNCTION__, status_code));
        return drm_msg;
    }
    s_wvBuffer = message;
#endif // USE_CURL
    size_t body_head = s_wvBuffer.find("\r\n\r\n");
    if (body_head == std::string::npos) {
        LOGE(("%s: no body found in response", __FUNCTION__));
#ifdef USE_CURL
        curl_easy_cleanup(curl);
#endif
        return drm_msg;
    }
    drm_msg.clear();
    body_head += 4;
    size_t drm_head = s_wvBuffer.find("\r\n\r\n", body_head);
    if (drm_head != std::string::npos) {
        LOGD(("%s: DRM message found", __FUNCTION__));
        drm_head += 4;
        drm_msg = s_wvBuffer.substr(drm_head);
    } else {
        drm_head = s_wvBuffer.find("\r\n", body_head);
        if (drm_head != std::string::npos) {
            LOGD(("%s: old style DRM message found", __FUNCTION__));
            drm_head += 2;
            drm_msg = s_wvBuffer.substr(drm_head);
        } else {
            LOGD(("%s: return body anyway", __FUNCTION__));
            drm_msg = s_wvBuffer.substr(body_head);
        }
    }

    LOGD(("HTTP response body: (%u bytes): %s", drm_msg.size(), b2a_hex(drm_msg).c_str()));
#ifdef USE_CURL
    curl_easy_cleanup(curl);
#endif
    return drm_msg;

}

bool WidevineDecryptor::AddKey(std::string key)
{
    LOGD(("calling AddKey session_id=%s", m_sessionId.c_str()));
    m_cdm->AddKey(m_sessionId, key, &m_keySetId);
    m_valid = true;
    return true;
}

bool WidevineDecryptor::CancelKeyRequest()
{
    LOGD(("%s: enter", __FUNCTION__));
    if (m_sessionId.empty())
        return true;

    LOGD(("%s: calling CancelKeyRequest sessionId=%s",
        __FUNCTION__, m_sessionId.c_str()));
    CdmResponseType status = m_cdm->CancelKeyRequest(m_sessionId);
    m_valid = false;
    if (status != NO_ERROR) {
        LOGE(("CloseSession Error: %d", status));
        return false;
    }
    LOGD(("%s: leave", __FUNCTION__));
    return true;
}

void incrementIV(uint64_t increaseBy, std::vector<uint8_t>* ivPtr)
{
  std::vector<uint8_t>& iv = *ivPtr;
  uint64_t* counterBuffer = reinterpret_cast<uint64_t*>(&iv[8]);
  (*counterBuffer) = htonq(ntohq(*counterBuffer) + increaseBy);
}

uint32_t WidevineDecryptor::DecryptSample(
    SampleInfo *pSample,
    IBuffer *input,
    IBuffer *output,
    uint32_t sampleSize)
{
    uint32_t rc = 0;
    uint8_t i = 0;
    uint32_t bytes_processed = 0;

    size_t clearSize = 0;
    size_t encSize = 0;
    for (i = 0; i < pSample->nbOfEntries; i++) {
        clearSize += pSample->entries[i].bytesOfClearData;
        encSize += pSample->entries[i].bytesOfEncData;
    }
    LOGD(("%s: sampleSize=%u clearSize=%u encSize=%u", __FUNCTION__, sampleSize, clearSize, encSize));

    uint8_t *encrypted_buffer = NULL;

    // DMA Transfer to the destination buffer
    if (output->IsSecure()) {
        output->Copy(0, input->GetPtr(), sampleSize);
        encrypted_buffer = (uint8_t*)output->GetPtr();
    } else {
        encrypted_buffer = input->GetPtr();
    }

    if (encSize == 0) {
        // No decrypt needed - just return
        bytes_processed += sampleSize;
        return bytes_processed;
    }

    CdmDecryptionParameters decryption_parameters;
    decryption_parameters.is_secure = output->IsSecure();
    decryption_parameters.key_id = &m_keyId;
    std::vector<uint8_t> ivVector(pSample->iv, pSample->iv + KEY_IV_SIZE);
    decryption_parameters.iv = &ivVector;
    decryption_parameters.decrypt_buffer = output->GetPtr();
    decryption_parameters.decrypt_buffer_length = sampleSize;

    size_t offset = 0;
    size_t blockOffset = 0;

    for (i = 0; i < pSample->nbOfEntries; i++) {
        uint32_t num_clear = pSample->entries[i].bytesOfClearData;
        uint32_t num_enc = pSample->entries[i].bytesOfEncData;
        uint8_t encryptedFlags = 0;

        if (i == 0) {
            encryptedFlags |= OEMCrypto_FirstSubsample;
        }
        if (i == pSample->nbOfEntries - 1) {
            encryptedFlags |= OEMCrypto_LastSubsample;
        }

        /* Skip over remaining clear units */
        offset += num_clear;

        LOGV(("%s: encrypted %u @ %u blkOffset=%u", __FUNCTION__,
            pSample->entries[i].bytesOfEncData, offset, blockOffset));

        decryption_parameters.is_encrypted = true;
        decryption_parameters.encrypt_buffer = (uint8_t *)encrypted_buffer + offset;
        decryption_parameters.encrypt_length = num_enc;
        decryption_parameters.block_offset = blockOffset;
        decryption_parameters.decrypt_buffer_offset = offset;
        decryption_parameters.subsample_flags = encryptedFlags;

        CdmResponseType res = m_cdm->Decrypt(
            m_sessionId, true, decryption_parameters);
        if (res != NO_ERROR && res != KEY_ADDED && res != KEY_MESSAGE &&
            res != KEY_CANCELED) {
            LOGE(("%s: Decrypt failed - %d", __FUNCTION__, res));
            goto ErrorExit;
        }

        offset += num_enc;
        blockOffset += num_enc;
        incrementIV(blockOffset / 16, &ivVector);
        blockOffset %= 16;
    }

    bytes_processed += sampleSize;

ErrorExit:
    return bytes_processed;
}
