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

 ******************************************************************************/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include "log.h" // to call wvcdm InitLogging
#undef LOGE
#undef LOGW
#undef LOGD
#undef LOGV
#define LOGE BDBG_ERR
#define LOGW BDBG_WRN
#define LOGD BDBG_MSG
#define LOGV BDBG_MSG

#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>

#include "string_conversions.h"

#include "wv_decryptor.h"
#pragma GCC diagnostic pop

BDBG_MODULE(wv_decryptor);
#include "dump_hex.h"

using namespace wvcdm;
using namespace dif_streamer;

const char kBasePathPrefix[] = "./widevine/";
const char kL1[] = "L1";
const char kL2[] = "L2";
const char kL3[] = "L3";
const char kCertificate[] = "cert.bin";
const char kDirectoryDelimiter = '/';

static bool s_wvcdmLogInit = false;

std::string s_wvBuffer;

#ifdef USE_CURL
#include <curl/curl.h>

static uint32_t curl_writeback( void *ptr, uint32_t size, uint32_t nmemb, void *stream)
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
        LOGW(("%s: s_wvBuffer(%d): %s, res: %d", __FUNCTION__, (uint32_t)s_wvBuffer.size(), s_wvBuffer.c_str(), res));
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

void* GetCdmHost(int host_interface_version, void* user_data)
{
    if (host_interface_version != cdm::kHostInterfaceVersion)
        return NULL;

    return user_data;
}

WidevineDecryptor::WidevineDecryptor()
    : cdm::Host(), BaseDecryptor()
{
    LOGD(("%s: enter", __FUNCTION__));

    if (!s_wvcdmLogInit) {
        InitLogging(0, NULL);
        s_wvcdmLogInit = true;
    }

    m_cdm = NULL;
    m_valid = false;

    m_file = NULL;
    m_certificate.clear();
    if (NULL != getenv("WIDEVINE_BASE_PATH")) {
        m_basePath = getenv("WIDEVINE_BASE_PATH");
        if (m_basePath.at(m_basePath.size() - 1) != kDirectoryDelimiter) {
            m_basePath += kDirectoryDelimiter;
        }
    } else {
        m_basePath = kBasePathPrefix;
    }
#ifdef USE_SECURE_PLAYBACK
    m_basePath += kL1;
#else
    m_basePath += kL3;
#endif
    m_basePath += kDirectoryDelimiter;

    m_certFilePath = m_basePath + kCertificate;

    LOGD(("%s: basePath=%s", __FUNCTION__, m_basePath.c_str()));
}

WidevineDecryptor::~WidevineDecryptor()
{
    LOGD(("%s: enter", __FUNCTION__));
    if (!m_sessionId.empty()) {
        LOGD(("%s: calling CancelKeyRequest sessionId=%s",
            __FUNCTION__, m_sessionId.c_str()));
        CancelKeyRequest();
    }
    if (m_cdm) {
        m_cdm->Destroy();
        m_cdm = NULL;
    }
    if (m_file) {
        fflush(m_file);
        fsync(fileno(m_file));
        fclose(m_file);
        m_file = NULL;
    }
    LOGD(("%s: leave", __FUNCTION__));
}

bool WidevineDecryptor::Provision()
{
    std::string provisioning_server_url;

    LOGD(("calling GetProvisioningRequest"));
    cdm::Status status = m_cdm->GetProvisioningRequest(
        &m_keyMessage, &provisioning_server_url);
    if (cdm::kSuccess != status) {
        LOGE(("GetProvisioningRequest Error: %d", status));
        return false;
    }
    LOGD(("GetProvisioningRequest: url=%s", provisioning_server_url.c_str()));

    dump_hex("ProvisioningReq", m_keyMessage.data(), m_keyMessage.size(), true);
    std::string response = GetCertRequestResponse(m_keyMessage, provisioning_server_url);
    dump_hex("Cert response", response.data(), response.size(), true);

    LOGD(("calling HandleProvisioningResponse"));
    status = m_cdm->HandleProvisioningResponse(response);
    if (cdm::kSuccess != status) {
        LOGE(("HandleProvisioningResponse Error: %d", status));
        return false;
    }
    LOGD(("HandleProvisioningResponse returned status=%d", status));
    return true;
}

bool WidevineDecryptor::Initialize(std::string& pssh)
{
    LOGW(("%s: pssh(%d): %s", __FUNCTION__, (uint32_t)pssh.size(), b2a_hex(pssh).c_str()));
    m_pssh.assign(pssh);
    m_keyId.assign(pssh.substr(4));
    LOGD(("%s: default keyId(%d): %s", __FUNCTION__, (uint32_t)m_keyId.size(), b2a_hex(m_keyId).c_str()));

    // Initialize the CDM module before creating a CDM instance.
    INITIALIZE_CDM_MODULE();

    // Create the CDM.
    if (m_cdm == NULL)
        m_cdm = reinterpret_cast<cdm::ContentDecryptionModule*>(
            ::CreateCdmInstance(
            cdm::kCdmInterfaceVersion, NULL, 0, GetCdmHost, this));

    if (m_cdm == NULL) {
        LOGE(("Failed to create CDM"));
        return false;
    }

    // Check the certificate file
    if (ReadDeviceCertificate()) {
        LOGW(("Device is provisioned!!"));
        return true;
    }

    if (!CreateDeviceCertificate()) {
        LOGE(("Failed to create certificate file"));
        return false;
    }

    if (!Provision()) {
        LOGW(("%s: First provisioning failed", __FUNCTION__));
        // Retry once again
        if (!Provision()) {
            LOGE(("%s: Provisioning failed", __FUNCTION__));
            return false;
        }
    }

    // TODO: Register for events?
/*
    WVEventListener eventListener;
    bool listenerAttached =
        m_cdm->AttachEventListener(m_sessionId, &eventListener);
*/

    return true;
}

bool WidevineDecryptor::GenerateKeyRequest(std::string initData, dif_streamer::SessionType type)
{
    BSTD_UNUSED(type);
    LOGW(("%s: %d initData(%d): %s", __FUNCTION__, __LINE__, (uint32_t)initData.size(), b2a_hex(initData).c_str()));
    m_hasKeyMessage = false;
    cdm::Status status = m_cdm->GenerateKeyRequest(
        NULL, 0, (const uint8_t*)initData.data(), initData.length());
    if (status == cdm::kNeedsDeviceCertificate) {
        // Just in case
        if (Initialize(m_pssh)) {
            status = m_cdm->GenerateKeyRequest(
                NULL, 0, (const uint8_t*)initData.data(), initData.length());
            if (status != cdm::kSuccess) {
                LOGE(("Second GenerateKeyRequest failed: %d", status));
                return false;
            }
        } else {
            LOGE(("GenerateKeyRequest-Initialize failed."));
            return false;
        }
    } else if (status != cdm::kSuccess) {
        LOGE(("First GenerateKeyRequest failed: %d", status));
        return false;
    }
    LOGD(("GenerateKeyRequest returned session_id: %s", m_sessionId.c_str()));
    m_initData.assign(initData);

    while (!HasNewKeyMessage()) {
        LOGW(("%s: waiting for new key", __FUNCTION__));
        usleep(100000);
    }

    return true;
}

std::string WidevineDecryptor::GetKeyRequestResponse(std::string url)
{
    LOGD(("%s: keyMessage(%d): %s", __FUNCTION__, (uint32_t)m_keyMessage.size(), b2a_hex(m_keyMessage).c_str()));
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
    LOGW(("%s: s_wvBuffer(%d): %s, res: %d", __FUNCTION__, (uint32_t)s_wvBuffer.size(), s_wvBuffer.c_str(), res));

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
        LOGW(("%s: return body anyway", __FUNCTION__));
        drm_msg = s_wvBuffer.substr(body_head);
    }

    LOGD(("HTTP response body: (%u bytes): %s", (uint32_t)drm_msg.size(), b2a_hex(drm_msg).c_str()));
#ifdef USE_CURL
    curl_easy_cleanup(curl);
#endif
    return drm_msg;

}

bool WidevineDecryptor::AddKey(std::string key)
{
    cdm::Status status = m_cdm->AddKey(m_sessionId.data(), m_sessionId.length(),
        (const uint8_t*)key.data(), key.length(),
        (const uint8_t*)m_initData.data(), m_initData.length());
    if (cdm::kSuccess != status) {
        LOGE(("AddKey Error: %d", status));
        return false;
    }
    m_valid = true;
    return true;
}

bool WidevineDecryptor::CancelKeyRequest()
{
    cdm::Status status = m_cdm->CloseSession(m_sessionId.data(), m_sessionId.length());
    m_sessionId.clear();
    m_valid = false;
    if (cdm::kSuccess != status) {
        LOGE(("CloseSession Error: %d", status));
        return false;
    }
    return true;
}

uint32_t WidevineDecryptor::DecryptSample(
    SampleInfo *pSample,
    IBuffer *input,
    IBuffer *output,
    uint32_t sampleSize)
{
    uint8_t i = 0;
    uint32_t bytes_processed = 0;

    cdm::InputBuffer inputBuffer;

    // set up subsamples
    std::vector<cdm::SubsampleEntry> subsamples;
    subsamples.reserve(pSample->nbOfEntries);

    uint32_t clearSize = 0;
    uint32_t encSize = 0;
    for (i = 0; i < pSample->nbOfEntries; i++) {
        // Transfer clear data: (SWSECDRM-1256)
        // assuming clear data comes first in subsamples
        if (pSample->entries[i].bytesOfClearData > 0)
            output->Copy(clearSize + encSize,
                input->GetPtr() + clearSize + encSize,
                pSample->entries[i].bytesOfClearData);

        subsamples.push_back(cdm::SubsampleEntry(pSample->entries[i].bytesOfClearData, pSample->entries[i].bytesOfEncData));
        clearSize += pSample->entries[i].bytesOfClearData;
        encSize += pSample->entries[i].bytesOfEncData;
    }
    LOGD(("%s: sampleSize=%u clearSize=%u encSize=%u", __FUNCTION__, sampleSize, clearSize, encSize));

    if (encSize == 0) {
        // No decrypt needed - just return
        bytes_processed += sampleSize;
        return bytes_processed;
    }

    inputBuffer.subsamples = subsamples.data();
    inputBuffer.num_subsamples = subsamples.size();

    // IV
    inputBuffer.iv = pSample->iv;
    inputBuffer.iv_size = BMP4_MAX_IV_ENTRIES;

    // Key Id
    inputBuffer.key_id = (const uint8_t*)m_keyId.data();
    inputBuffer.key_id_size = m_keyId.size();
LOGD(("keyId: %x %x %x %x %x %x %x %x", inputBuffer.key_id[0], inputBuffer.key_id[1],inputBuffer.key_id[2],inputBuffer.key_id[3],inputBuffer.key_id[4],inputBuffer.key_id[5],inputBuffer.key_id[6],inputBuffer.key_id[7]));

    // Data
    inputBuffer.data_size = sampleSize;
    inputBuffer.data_offset = 0;
    inputBuffer.data = input->GetPtr();

    CdmOutputBuffer outputBuffer((uint8_t*)output->GetPtr(), sampleSize);
    ProxyDecryptedBlock outputBlock;
    outputBlock.SetDecryptedBuffer(&outputBuffer);

    cdm::Status result = m_cdm->Decrypt(inputBuffer, &outputBlock);

    if (result == cdm::kNoKey) {
        LOGE(("%s: Decrypt returned NoKey\n", __FUNCTION__));
        return bytes_processed;
    } else if (result != cdm::kSuccess) {
        LOGE(("%s: Decrypt returned error %d\n", __FUNCTION__, (int)result));
        return bytes_processed;
    }

    bytes_processed += sampleSize;

    return bytes_processed;
}

bool WidevineDecryptor::IsKeyValid(const std::string& key_id)
{
    bool ret = m_cdm->IsKeyValid((const uint8_t*)key_id.data(), key_id.size());
    return ret;
}

cdm::Buffer* WidevineDecryptor::Allocate(int32_t capacity)
{
	BSTD_UNUSED(capacity);
	return NULL;
}

void WidevineDecryptor::SetTimer(int64_t delayMs, void* context)
{
    // TODO
    double expiryTime = m_currentTime + (delayMs / 1000.0);
    m_timers.push(Timer(expiryTime, context));
}

double WidevineDecryptor::GetCurrentWallTimeInSeconds()
{
    return m_currentTime;
}

void WidevineDecryptor::SendKeyMessage(const char* sessionId,
    int32_t sessionIdLength, const char* message, int32_t messageLength,
    const char* defaultUrl, int32_t defaultUrlLength)
{
    LOGD(("%s: enter", __FUNCTION__));
    m_sessionId.assign(sessionId, sessionIdLength);
    m_keyMessage.assign(message, messageLength);
    m_defaultUrl.assign(defaultUrl, defaultUrlLength);
    m_hasKeyMessage = true;
    LOGD(("%s: keyMessage(%d): %s", __FUNCTION__, (uint32_t)m_keyMessage.size(), b2a_hex(m_keyMessage).c_str()));
}

void WidevineDecryptor::SendKeyError(const char* sessionId,
    int32_t sessionIdLength, cdm::MediaKeyError errorCode, uint32_t systemCode)
{
    m_sessionId.assign(sessionId, sessionIdLength);
    m_errorCode = errorCode;
    m_systemCode = systemCode;
    m_hasKeyError = true;
}

void WidevineDecryptor::GetPlatformString(const std::string& name,
    std::string* value)
{
    LOGD(("%s: name=%s", __FUNCTION__, name.c_str()));
    if (name.compare("DeviceCertificate") == 0) {
        if (!m_certificate.empty()) {
            *value = m_certificate;
            return;
        }

        if (ReadDeviceCertificate()) {
            m_platformStrings[name] = m_certificate;
        }
    } else if (name.compare("SecurityLevel") == 0) {
#ifdef USE_SECURE_PLAYBACK
        m_platformStrings[name] = kL1;
#else
        m_platformStrings[name] = kL3;
#endif
    }

    *value = m_platformStrings[name];
}

void WidevineDecryptor::SetPlatformString(const std::string& name,
                                 const std::string& value)
{
    LOGD(("%s: name=%s", __FUNCTION__, name.c_str()));
    m_platformStrings[name] = value;

    if (name.compare("DeviceCertificate") == 0) {
        if (m_file == NULL) {
            return;
        }
        m_certificate = value;
        WriteDeviceCertificate(value.data(), value.size());
    }
}

static bool CreateDirectory(std::string path)
{
    LOGD(("%s: path: %s", __FUNCTION__, path.c_str()));
    uint32_t size = path.size();
    if ((size == 1) && (path[0] == kDirectoryDelimiter))
        return true;

    if (size <= 1)
        return false;

    size_t pos = path.find(kDirectoryDelimiter, 1);
    while (pos < size) {
        path[pos] = '\0';
        if (mkdir(path.c_str(), 0775) != 0) {
            if (errno != EEXIST) {
                LOGW(("%s: mkdir failed(%d): %s", __FUNCTION__, errno, path.c_str()));
                return false;
            }
        }
        path[pos] = kDirectoryDelimiter;
        pos = path.find(kDirectoryDelimiter, pos + 1);
    }

    if (path[size - 1] != kDirectoryDelimiter) {
        if (mkdir(path.c_str(), 0775) != 0) {
            if (errno != EEXIST) {
                LOGW(("%s: mkdir failed(%d): %s", __FUNCTION__, errno, path.c_str()));
                return false;
            }
        }
    }
    return true;
}

bool WidevineDecryptor::CreateDeviceCertificate()
{
    if (CreateDirectory(m_basePath) == false) {
        return false;
    }

    struct stat buf;
    if (stat(m_certFilePath.c_str(), &buf) != 0) {
        m_file = fopen(m_certFilePath.c_str(), "w+");
        if (m_file == NULL) {
            LOGW(("%s: can't create: %s", __FUNCTION__, m_certFilePath.c_str()));
            return false;
        }
        fclose(m_file);
        m_file = NULL;
    }

    m_file = fopen(m_certFilePath.c_str(), "r+b");
    if (m_file == NULL) {
        LOGW(("%s: can't open: %s", __FUNCTION__, m_certFilePath.c_str()));
        return false;
    }

    return true;
}

bool WidevineDecryptor::ReadDeviceCertificate()
{
    LOGD(("%s: certFilePath=%s", __FUNCTION__, m_certFilePath.c_str()));
    struct stat buf;
    if (stat(m_certFilePath.c_str(), &buf) != 0) {
        LOGW(("%s: stat failed(%d): %s", __FUNCTION__, errno, m_certFilePath.c_str()));
        return false;
    }

    if (m_file == NULL) {
        m_file = fopen(m_certFilePath.c_str(), "rb");
        if (m_file == NULL) {
            LOGW(("%s: fopen failed(%d): %s", __FUNCTION__, errno, m_certFilePath.c_str()));
            return false;
        }
    }

    m_certificate.clear();
    char* buffer = (char*)malloc(buf.st_size);
    uint32_t len = fread(buffer, sizeof(char), buf.st_size, m_file);
    if (len == 0) {
        LOGW(("%s: fread failed(%d): %s", __FUNCTION__, errno, m_certFilePath.c_str()));
    } else if (len != buf.st_size) {
        LOGW(("%s: fread incomplete (%d/%d): %s", __FUNCTION__, (int32_t)len, (int32_t)buf.st_size, m_certFilePath.c_str()));
    } else {
        m_certificate.assign(buffer, len);
    }
    free(buffer);

    return !m_certificate.empty();
}

bool WidevineDecryptor::WriteDeviceCertificate(const char* buffer, uint32_t bytes)
{
    LOGD(("%s: size=%u", __FUNCTION__, bytes));
    uint32_t len = fwrite(buffer, sizeof(char), bytes, m_file);
    if (len == 0) {
        LOGW(("%s: fwrite failed(%d): %s", __FUNCTION__, errno, m_certFilePath.c_str()));
        return false;
    }
    return true;
}
