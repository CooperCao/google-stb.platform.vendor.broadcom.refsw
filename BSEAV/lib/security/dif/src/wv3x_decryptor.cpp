/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
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
#include <queue>
#include <arpa/inet.h>

#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#if defined(__linux__)
#include <sys/utsname.h>
#endif
#if defined( WV_CDM_V32 ) || defined( WV_CDM_V35 )
#include <dirent.h>
#endif
#include "string_conversions.h"
#include "wv3x_decryptor.h"

#pragma GCC diagnostic pop
BDBG_MODULE(wv3x_decryptor);
#include "dump_hex.h"

using namespace wvcdm;
using namespace widevine;
using namespace dif_streamer;

const char kBasePathPrefix[] = "./widevine/";
const char kL1[] = "L1";
const char kL2[] = "L2";
const char kL3[] = "L3";
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

static std::string GetCertRequestResponse(std::string& keyMessage)
{
    LOGW(("%s: Cert req keyMessage(%d): %s", BSTD_FUNCTION, (uint32_t)keyMessage.size(), keyMessage.c_str()));
    std::string server_url;
    std::string message = "";
    s_wvBuffer.assign("");
#ifdef USE_CURL
    server_url = "https://www.googleapis.com/"
    "certificateprovisioning/v1/devicecertificates/create"
    "?key=AIzaSyB-5OLKTx2iU5mko18DfdwK5611JIjbUhE";

    server_url += "&signedRequest=";
    server_url += keyMessage;
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
        LOGW(("\ns_wvBuffer(%d): %s, res: %d\n", (uint32_t)s_wvBuffer.size(), s_wvBuffer.c_str(), res));
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

Widevine3xDecryptor::Widevine3xDecryptor()
    : BaseDecryptor(), IEventListener()
{
    LOGD(("%s: enter", BSTD_FUNCTION));

    if (!s_wvcdmLogInit) {
        InitLogging();
        s_wvcdmLogInit = true;
    }
    m_cdm = NULL;
    m_valid = false;

    m_certReq.clear();
    m_gotCertReq = false;
    m_certificate.clear();
}

Widevine3xDecryptor::~Widevine3xDecryptor()
{
    LOGD(("%s: enter", BSTD_FUNCTION));
#if 0
    if (!m_sessionId.empty()) {
        LOGD(("%s: calling CancelKeyRequest sessionId=%s",
            BSTD_FUNCTION, m_sessionId.c_str()));
        CancelKeyRequest();
    }
#endif
    if (m_cdm) {
        LOGD(("%s: closing session_id:%s", BSTD_FUNCTION, m_sessionId.c_str()));
        m_cdm->close(m_sessionId);
        delete m_cdm;
        m_cdm = NULL;
    }

    if (m_cdmHost) {
        delete m_cdmHost;
        m_cdmHost = NULL;
    }
    LOGD(("%s: leave", BSTD_FUNCTION));
}

bool Widevine3xDecryptor::Initialize(std::string& pssh)
{
    Cdm::Status status;
    char* logLevelStr;
    int logLevel = 0;
    LOGD(("%s: pssh(%d): %s", BSTD_FUNCTION, (uint32_t)pssh.size(), b2a_hex(pssh).c_str()));
    m_pssh.assign(pssh);
    m_keyId.assign(pssh.substr(4));
    LOGD(("%s: default keyId(%d): %s", BSTD_FUNCTION, (uint32_t)m_keyId.size(), b2a_hex(m_keyId).c_str()));

    // Initialize the CDM module before creating a CDM instance.

    Cdm::ClientInfo client_info;
    // Set client info that denotes this as the test suite:
    client_info.product_name = "SETTOP";
    client_info.company_name = "BROADCOM";
    client_info.model_name = "7xxx";
    client_info.device_name = "7xxx";
    client_info.arch_name = "7xxx";
    client_info.build_info = "SETTOP URSR";

    m_cdmHost = new CdmHost();

    if (NULL != getenv("WIDEVINE_LOG_LEVEL")) {
        logLevelStr = getenv("WIDEVINE_LOG_LEVEL");
        logLevel = atoi(logLevelStr);
        LOGW(("logLevel: %d", logLevel));
    }
    if (logLevel < -1 || logLevel > 4) {
        LOGW(("Bad logLevel: defaulting to 0"));
        logLevel = 0;
    }

    status = Cdm::initialize(
        Cdm::kOpaqueHandle, client_info, m_cdmHost, m_cdmHost, m_cdmHost,
        static_cast<Cdm::LogLevel>(logLevel));

    if (status != Cdm::kSuccess) {
        LOGE(("Failed to initialize CDM"));
        return false;
    }

    LOGW(("Cdm version=%s", Cdm::version()));

    // Create the CDM.
    // Use privacy mode by default
    bool privacyMode = true;

    // Provide a way to go without privacy mode
    const char *privacyModeEnv = getenv("WIDEVINE_PRIVACY_MODE");
    if (privacyModeEnv != NULL && strcmp(privacyModeEnv, "n") == 0) {
        privacyMode = false;
    }
    LOGW(("Privacy mode is set to %s", privacyMode ? "true" : "false"));

    if (m_cdm == NULL)
        m_cdm = reinterpret_cast<widevine::Cdm*>(Cdm::create(this, m_cdmHost, privacyMode));

    if (m_cdm == NULL) {
        LOGE(("Failed to create CDM"));
        return false;
    }

    const std::string kBrcmCertificate = wvcdm::a2bs_hex(
        "0AC102080312101705B917CC1204868B06333A2F772A8C1882B4829205228E02308201"
        "0A028201010099ED5B3B327DAB5E24EFC3B62A95B598520AD5BCCB37503E0645B814D8"
        "76B8DF40510441AD8CE3ADB11BB88C4E725A5E4A9E0795291D58584023A7E1AF0E38A9"
        "1279393008610B6F158C878C7E21BFFBFEEA77E1019E1E5781E8A45F46263D14E60E80"
        "58A8607ADCE04FAC8457B137A8D67CCDEB33705D983A21FB4EECBD4A10CA47490CA47E"
        "AA5D438218DDBAF1CADE3392F13D6FFB6442FD31E1BF40B0C604D1C4BA4C9520A4BF97"
        "EEBD60929AFCEEF55BBAF564E2D0E76CD7C55C73A082B996120B8359EDCE2470708268"
        "0D6F67C6D82C4AC5F3134490A74EEC37AF4B2F010C59E82843E2582F0B6B9F5DB0FC5E"
        "6EDF64FBD308B4711BCF1250019C9F5A0902030100013A146C6963656E73652E776964"
        "6576696E652E636F6D128003AE347314B5A835297F271388FB7BB8CB5277D249823CDD"
        "D1DA30B93339511EB3CCBDEA04B944B927C121346EFDBDEAC9D413917E6EC176A10438"
        "460A503BC1952B9BA4E4CE0FC4BFC20A9808AAAF4BFCD19C1DCFCDF574CCAC28D1B410"
        "416CF9DE8804301CBDB334CAFCD0D40978423A642E54613DF0AFCF96CA4A9249D855E4"
        "2B3A703EF1767F6A9BD36D6BF82BE76BBF0CBA4FDE59D2ABCC76FEB64247B85C431FBC"
        "A52266B619FC36979543FCA9CBBDBBFAFA0E1A55E755A3C7BCE655F9646F582AB9CF70"
        "AA08B979F867F63A0B2B7FDB362C5BC4ECD555D85BCAA9C593C383C857D49DAAB77E40"
        "B7851DDFD24998808E35B258E75D78EAC0CA16F7047304C20D93EDE4E8FF1C6F17E624"
        "3E3F3DA8FC1709870EC45FBA823A263F0CEFA1F7093B1909928326333705043A29BDA6"
        "F9B4342CC8DF543CB1A1182F7C5FFF33F10490FACA5B25360B76015E9C5A06AB8EE02F"
        "00D2E8D5986104AACC4DD475FD96EE9CE4E326F21B83C7058577B38732CDDABC6A6BED"
        "13FB0D49D38A45EB87A5F4");

#if defined( WV_CDM_V31 )
    if (privacyMode)
        status = m_cdm->setServerCertificate(kBrcmCertificate);
#else
    status = m_cdm->setServiceCertificate(kBrcmCertificate);
#endif
    if (status != Cdm::kSuccess) {
        LOGE(("setServiceCertificate failed: %d", (int)status));
        return false;
    }
    return true;
}

bool Widevine3xDecryptor::Load(std::string license)
{
    Cdm::Status status;
    std::vector<std::string> set_key_ids;
    LOGD(("%s: License: %s", BSTD_FUNCTION, license.c_str()));

#if defined( WV_CDM_V32 ) || defined( WV_CDM_V35 )
    bool match = false;
    status = m_cdm->listStoredLicenses(&set_key_ids);
    for (std::vector<std::string>::iterator it = set_key_ids.begin() ; it != set_key_ids.end(); ++it) {
        LOGD(("Compare stored license: %s", it->c_str()));
        if (it->compare(license) == 0) {
            LOGW(("%s: license(%s) is in the stored license list", BSTD_FUNCTION, license.c_str()));
            match = true;
            m_valid = false;
            break;
        }
    }

    if (!match) {
        LOGW(("%s: License(%s) is not in the stored license list", BSTD_FUNCTION, license.c_str()));
        return false;
    }
#endif

    m_sessionId.assign(license);
    LOGW(("%s: Loading license: %s", BSTD_FUNCTION, m_sessionId.c_str()));
    status = m_cdm->load(m_sessionId);
    if (status != Cdm::kSuccess) {
        LOGE(("%s: License load failed %d", BSTD_FUNCTION, status));
        m_sessionId.clear();
        m_valid = false;
        return false;
    }

    m_cdmSessionType = Cdm::kPersistentLicense;
    m_valid = true;
    return true;
}

bool Widevine3xDecryptor::Provision()
{
    Cdm::Status status;
    LOGD(("%s: enter", BSTD_FUNCTION));

    status = m_cdm->createSession(m_cdmSessionType, &m_sessionId);
    if (status != Cdm::kSuccess) {
        return false;
    }
    LOGD(("%s: created session_id:%s", BSTD_FUNCTION, m_sessionId.c_str()));

    status = m_cdm->generateRequest(m_sessionId, Cdm::kCenc, m_initData);
    if (status != Cdm::kSuccess && status != Cdm::kDeferred) {
        LOGE(("GenerateKeyRequest failed."));
        m_cdm->close(m_sessionId);
        return false;
    }

    // Wait for the callback
    while (!m_gotCertReq){
        LOGW(("%s: waiting for cert request from CDM", BSTD_FUNCTION));
        usleep(100000);
    }

    if (!m_certReq.empty()) {
        m_certificate = GetCertRequestResponse(m_certReq);

        LOGW(("%s: m_certificate=%s", BSTD_FUNCTION, m_certificate.c_str()));

        status = m_cdm->update(m_sessionId, m_certificate);
        if (status != Cdm::kSuccess) {
            LOGE(("update failed: status=%d", status));
            m_certificate.clear();
            m_certReq.clear();
            m_gotCertReq = false;
            m_cdm->close(m_sessionId);
            return false;
        }
    }

    return true;
}

bool Widevine3xDecryptor::GenerateKeyRequest(std::string initData, dif_streamer::SessionType type)
{
    LOGW(("%s: %d initData(%d): %s", BSTD_FUNCTION, __LINE__, (uint32_t)initData.size(), b2a_hex(initData).c_str()));
    LOGW(("%s: type:%d", BSTD_FUNCTION, type));

    if (m_valid) {
        LOGW(("%s: License has already been loaded", BSTD_FUNCTION));
        return true;
    }

    m_hasKeyMessage = false;

    // Convert session type - actual values are same
    m_cdmSessionType = (Cdm::SessionType)type;

    m_initData.assign(initData);

    if (!Provision()) {
        LOGW(("%s: First provisioning failed", BSTD_FUNCTION));
        // Try again just in case
        if (!Provision()) {
            LOGE(("%s: Provisioning failed", BSTD_FUNCTION));
            return false;
        }
    }

    while (!HasNewKeyMessage()) {
        LOGW(("%s: waiting for new key", BSTD_FUNCTION));
        usleep(100000);
    }

    return true;
}

std::string Widevine3xDecryptor::GetKeyRequestResponse(std::string url)
{
    m_url = url;
    std::string drm_msg;

        LOGW(("%s: keyMessage(%d): %s", BSTD_FUNCTION, (uint32_t)m_keyMessage.size(), b2a_hex(m_keyMessage).c_str()));
        std::string message = "";
        s_wvBuffer.assign("");
#ifdef USE_CURL
       // url+= "SecurityLevel=L3";
        LOGW(("%s: server_url: %s", BSTD_FUNCTION, url.c_str()));
        CURL *curl;
        CURLcode res;
        curl = curl_easy_init();
        if (curl == NULL) {
            LOGE(("%s: curl_easy_init returned NULL", BSTD_FUNCTION));
            return drm_msg;
        }
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, m_keyMessage.data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, m_keyMessage.size());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writeback);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Widevine v3.x");
        res = curl_easy_perform(curl);
        LOGW(("%s: s_wvBuffer(%d): %s, res: %d", BSTD_FUNCTION, (uint32_t)s_wvBuffer.size(), s_wvBuffer.c_str(), res));

        if (res != 0) {
            LOGE(("%s: curl error %d", BSTD_FUNCTION, res));
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
            LOGE(("%s: error status_code=%d", BSTD_FUNCTION, status_code));
            return drm_msg;
        }
        s_wvBuffer = message;
#endif // USE_CURL
        size_t body_head = s_wvBuffer.find("\r\n\r\n");
        if (body_head == std::string::npos) {
            LOGW(("%s: no body found in response", BSTD_FUNCTION));
#ifdef USE_CURL
            curl_easy_cleanup(curl);
#endif
            LOGW(("%s: return entire response", BSTD_FUNCTION));
            drm_msg.assign(s_wvBuffer);
            return drm_msg;
        }
        drm_msg.clear();
        body_head += 4;
        size_t drm_head = s_wvBuffer.find("\r\n\r\n", body_head);
        if (drm_head != std::string::npos) {
            drm_head += 4;
            drm_msg = s_wvBuffer.substr(drm_head);
        } else {
            LOGW(("%s: return body anyway", BSTD_FUNCTION));
            drm_msg = s_wvBuffer.substr(body_head);
        }

        LOGD(("HTTP response body: (%u bytes): %s", (uint32_t)drm_msg.size(), b2a_hex(drm_msg).c_str()));
#ifdef USE_CURL
        curl_easy_cleanup(curl);
#endif
        return drm_msg;
}

bool Widevine3xDecryptor::AddKey(std::string key, bmp4_protection_info& protectionInfo)
{
    BSTD_UNUSED(protectionInfo);
    std::string response;
    Cdm::Status status;
    Cdm::KeyStatusMap key_statuses;

    if (m_valid) {
        LOGW(("%s: License(%s) has already been loaded", BSTD_FUNCTION, m_sessionId.c_str()));
        return true;
    }

    status = m_cdm->update(m_sessionId, key);
    if (status != Cdm::kSuccess) {
        LOGE(("Add Key Error: %d", status));
        return false;
    }

    status = m_cdm->getKeyStatuses(m_sessionId, &key_statuses);

    while (key_statuses[m_sessionId] != Cdm::kUsable) {
        LOGW(("key_status=%d key is not yet usable", key_statuses[m_sessionId]));
        usleep(100000);
    }

    m_valid = true;
    return true;
}

// The iso spec only uses the lower 8 bytes of the iv as
// the counter.
#define CENC_IV_SIZE 8

void Ctr128Add(size_t block_count, uint8_t* counter)
{
    uint8_t carry = 0;
    uint8_t n = BMP4_MAX_IV_ENTRIES - 1;
    while (n >= CENC_IV_SIZE) {
        uint32_t temp = block_count & 0xff;
        temp += counter[n];
        temp += carry;
        counter[n] = temp & 0xff;
        carry = (temp & 0x100) ? 1 : 0;
        block_count = block_count >> 8;
        n--;
        if (!block_count && !carry) {
            break;
        }
    }
}

uint32_t Widevine3xDecryptor::DecryptSample(
    SampleInfo *pSample,
    IBuffer *input,
    IBuffer *output,
    uint32_t sampleSize,
    bmp4_protectionSchemeInfo& trackProtectionInfo)
{
    BSTD_UNUSED(trackProtectionInfo);
    uint8_t i = 0;
    uint32_t bytes_processed = 0;
    uint32_t clearSize = 0;
    uint32_t encSize = 0;
    Cdm::InputBuffer inputBuffer;
    Cdm::OutputBuffer outputBuffer;
    uint8_t *encrypted_buffer = NULL;
    uint32_t offset = 0;
    uint32_t enc_offset = 0;
    uint32_t blockOffset = 0;
    uint32_t blkCtr = 0;
    uint32_t lastBlkCtr = 0;

    for (i = 0; i < pSample->nbOfEntries; i++) {
        clearSize += pSample->entries[i].bytesOfClearData;
        encSize += pSample->entries[i].bytesOfEncData;
    }
    LOGD(("%s: sampleSize=%u clearSize=%u encSize=%u", BSTD_FUNCTION, sampleSize, clearSize, encSize));

    encrypted_buffer = (uint8_t*)input->GetPtr();

    if (encSize == 0) {
        // No decrypt needed - just return
        bytes_processed += sampleSize;
        return bytes_processed;
    }

    // IV
    inputBuffer.iv = pSample->iv;
    inputBuffer.iv_length= BMP4_MAX_IV_ENTRIES;

    // Key Id
    inputBuffer.key_id = (const uint8_t*)m_keyId.data();
    inputBuffer.key_id_length = m_keyId.size();
    dump_hex("keyId", (const char*)inputBuffer.key_id, inputBuffer.key_id_length);

    // Data
    inputBuffer.is_video = true;
    outputBuffer.is_secure = output->IsSecure();
    outputBuffer.data = (uint8_t*)output->GetPtr();
    outputBuffer.data_length = sampleSize;

    for (i = 0; i < pSample->nbOfEntries; i++) {
        uint32_t num_clear = pSample->entries[i].bytesOfClearData;
        uint32_t num_enc = pSample->entries[i].bytesOfEncData;
        uint8_t encryptedFlags = 0;
        inputBuffer.first_subsample = 0;
        inputBuffer.last_subsample= 0;

        if (i == 0) {
            encryptedFlags |= OEMCrypto_FirstSubsample;
            inputBuffer.first_subsample = encryptedFlags;
        }
        if (i == pSample->nbOfEntries - 1) {
            encryptedFlags |= OEMCrypto_LastSubsample;
            inputBuffer.last_subsample= encryptedFlags;
        }

        /* Skip over remaining clear units */
        offset += num_clear;

        LOGD(("%s: encrypted %u @ %u blkOffset=%u", BSTD_FUNCTION,
            pSample->entries[i].bytesOfEncData, offset, blockOffset));

        Cdm::Pattern p(0,0);
        inputBuffer.pattern = p;

        inputBuffer.encryption_scheme = Cdm::kAesCtr;

        inputBuffer.data = (uint8_t *)encrypted_buffer + offset;
        inputBuffer.data_length = num_enc;
        outputBuffer.data_offset = offset;
        inputBuffer.block_offset = blockOffset;

        // Update IV for each subsample
        blkCtr = enc_offset / BMP4_MAX_IV_ENTRIES;
        Ctr128Add(blkCtr - lastBlkCtr, (uint8_t*)inputBuffer.iv);
        lastBlkCtr = blkCtr;
        dump_hex("IV:", (const char*)inputBuffer.iv, inputBuffer.iv_length);

        Cdm::Status result = m_cdm->decrypt(inputBuffer, outputBuffer);

        if (result == Cdm::kNoKey) {
            LOGE(("%s: Decrypt returned NoKey\n", BSTD_FUNCTION));
            goto ErrorExit;
        } else if (result != Cdm::kSuccess) {
            LOGE(("%s: Decrypt returned error %d\n", BSTD_FUNCTION, (int)result));
            goto ErrorExit;
        }

        offset += num_enc;
        enc_offset += num_enc;
        blockOffset += num_enc;
        blockOffset %= 16;
    }

    bytes_processed += sampleSize;

ErrorExit:
    return bytes_processed;
}

bool Widevine3xDecryptor::CancelKeyRequest()
{
    LOGW(("%s: sessionId=%s", BSTD_FUNCTION, m_sessionId.c_str()));
    Cdm::Status status = m_cdm->close(m_sessionId);
    m_sessionId.clear();
    m_valid = false;
    if (Cdm::kSuccess != status) {
        LOGE(("CloseSession Error: %d", status));
        return false;
    }
    return true;
}

void Widevine3xDecryptor::onKeyStatusesChange(
    const std::string& session_id)
{
    Cdm::Status status;
    Cdm::KeyStatusMap key_statuses;
    LOGW(("%s: session_id=%s", BSTD_FUNCTION, session_id.c_str()));
    status = m_cdm->getKeyStatuses(session_id, &key_statuses);
    if (status != Cdm::kSuccess) {
        LOGE(("%s: getKeyStatuses Error: %d", BSTD_FUNCTION, status));
        return;
    }

    LOGW(("%s: keyStatus=%d", BSTD_FUNCTION, key_statuses[session_id]));
    if (key_statuses[session_id] == Cdm::kUsable) {
        m_valid = true;
        return;
    }
    else if (key_statuses[session_id] == Cdm::kExpired) {
        LOGW(("%s: session %s expired", BSTD_FUNCTION, session_id.c_str()));
    }
    m_valid = false;
}

void Widevine3xDecryptor::onRemoveComplete(
    const std::string& session_id)
{
    LOGW(("%s: session_id=%s", BSTD_FUNCTION, session_id.c_str()));
}

void Widevine3xDecryptor::onDeferredComplete(
    const std::string& session_id, Cdm::Status result)
{
    LOGW(("%s: session_id=%s result=%d", BSTD_FUNCTION, session_id.c_str(), result));
    m_gotCertReq = true;
}

void Widevine3xDecryptor::onDirectIndividualizationRequest(
    const std::string& session_id, const std::string& request)
{
    LOGW(("%s: session_id=%s request=%s", BSTD_FUNCTION, session_id.c_str(), request.c_str()));
    m_certReq = request;
    m_gotCertReq = true;
}

void Widevine3xDecryptor::onMessage(
    const std::string& session_id,
    Cdm::MessageType message_type,
    const std::string& message)
{
    LOGW(("%s: session_id=%s type=%d message size=%d", BSTD_FUNCTION,
        session_id.c_str(), message_type, (uint32_t)message.size()));
    dump_hex("onMessage", message.data(), message.size());
    m_sessionId = session_id;
    m_messageType = message_type;
    m_keyMessage = message;
    m_hasKeyMessage = true;
    m_gotCertReq = true;
}

CdmHost::CdmHost() : widevine::Cdm::IStorage(),
    widevine::Cdm::IClock(),
    widevine::Cdm::ITimer()
{
    CdmHost::Reset();

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
    LOGD(("%s: basePath=%s", BSTD_FUNCTION, m_basePath.c_str()));
}

CdmHost::~CdmHost()
{
    LOGD(("%s: CdmHost %p destroyed", BSTD_FUNCTION, (void*)this));
}

bool CdmHost::read(const std::string& name,
    std::string* data)
{
    LOGD(("CdmHost::%s: name=%s", BSTD_FUNCTION, name.c_str()));

    if (data == NULL) {
        LOGD(("CdmHost::%s: data is NULL", BSTD_FUNCTION));
        return false;
    }

    struct stat buf;
    std::string filePath = m_basePath;
    filePath.append(name);
    if (stat(filePath.c_str(), &buf) != 0) {
        LOGW(("%s: stat failed(%d): %s", BSTD_FUNCTION, errno, filePath.c_str()));
        return false;
    }

    char* buffer = (char*)malloc(buf.st_size);
    FILE* fd = fopen(filePath.c_str(), "r+b");
    uint32_t len = fread(buffer, sizeof(char), buf.st_size, fd);
    if (len == 0) {
        LOGW(("%s: fread failed(%d): %s", BSTD_FUNCTION, errno, filePath.c_str()));
    } else if ((int32_t)len != buf.st_size) {
        LOGW(("%s: fread incomplete (%d/%d): %s", BSTD_FUNCTION, (int32_t)len, (int32_t)buf.st_size, filePath.c_str()));
    } else {
        data->assign(buffer, len);
    }

    return true;
}

static bool createDirectory(std::string path)
{
    LOGD(("%s: path: %s", BSTD_FUNCTION, path.c_str()));
    uint32_t size = path.size();
    if ((size == 1) && (path[0] == kDirectoryDelimiter))
        return true;

    if (size <= 1)
        return false;

    uint32_t pos = path.find(kDirectoryDelimiter, 1);
    while (pos < size) {
        path[pos] = '\0';
        if (mkdir(path.c_str(), 0775) != 0) {
            if (errno != EEXIST) {
                LOGW(("%s: mkdir failed(%d): %s", BSTD_FUNCTION, errno, path.c_str()));
                return false;
            }
        }
        path[pos] = kDirectoryDelimiter;
        pos = path.find(kDirectoryDelimiter, pos + 1);
    }

    if (path[size - 1] != kDirectoryDelimiter) {
        if (mkdir(path.c_str(), 0775) != 0) {
            if (errno != EEXIST) {
                LOGW(("%s: mkdir failed(%d): %s", BSTD_FUNCTION, errno, path.c_str()));
                return false;
            }
        }
    }
    return true;
}

static FILE* openFile(
    std::string& basePath, const std::string& name)
{
    if (createDirectory(basePath) == false) {
        return NULL;
    }

    struct stat buf;
    FILE* fd = NULL;
    std::string filePath = basePath;
    filePath.append(name);
    if (stat(filePath.c_str(), &buf) != 0) {
        fd = fopen(filePath.c_str(), "w+");
        if (fd == NULL) {
            LOGW(("%s: can't create: %s", BSTD_FUNCTION, filePath.c_str()));
            return NULL;
        }
        fclose(fd);
        fd = NULL;
    }

    fd = fopen(filePath.c_str(), "r+b");
    if (fd == NULL) {
        LOGW(("%s: can't open: %s", BSTD_FUNCTION, filePath.c_str()));
        return NULL;
    }

    return fd;
}

bool CdmHost::write(const std::string& name,
    const std::string& data)
{
    LOGW(("CdmHost::%s: name=%s", BSTD_FUNCTION, name.c_str()));
    dump_hex("data", data.data(), data.size());

    FILE* fd = openFile(m_basePath, name);
    uint32_t len = fwrite(data.data(), sizeof(char), data.size(), fd);
    if (len != data.size()) {
        LOGW(("%s: fwrite failed(%d): len=%d %s", BSTD_FUNCTION, errno, (int32_t)len, name.c_str()));
        return false;
    }
    fflush(fd);
    return true;
}

bool CdmHost::exists(const std::string& name)
{
    LOGD(("CdmHost::%s: name=%s", BSTD_FUNCTION, name.c_str()));

    std::string filePath = m_basePath;
    filePath.append(name);
    FILE* fd = fopen(filePath.c_str(), "r+b");
    if (fd == NULL) {
        LOGD(("%s: can't open: %s", BSTD_FUNCTION, filePath.c_str()));
        return false;
    }

    LOGD(("CdmHost::%s: return true", BSTD_FUNCTION));
    return true;
}

bool CdmHost::remove(const std::string& name)
{
    bool rc = false;
    LOGD(("CdmHost::%s: name=%s", BSTD_FUNCTION, name.c_str()));

    std::string filePath = m_basePath;
    filePath.append(name);
    rc = (unlink(filePath.c_str()) == 0);
    return rc;
}

int32_t CdmHost::size(const std::string& name)
{
    LOGD(("CdmHost::%s: name=%s", BSTD_FUNCTION, name.c_str()));

    struct stat buf;
    std::string filePath = m_basePath;
    filePath.append(name);
    if (stat(filePath.c_str(), &buf) != 0) {
        LOGW(("%s: stat failed(%d): %s", BSTD_FUNCTION, errno, filePath.c_str()));
        return -1;
    }
    LOGD(("CdmHost::%s: size=%d", BSTD_FUNCTION, (int32_t)buf.st_size));
    return (int32_t)buf.st_size;
}

#if defined( WV_CDM_V32 ) || defined( WV_CDM_V35 )
bool CdmHost::list(std::vector<std::string>* names)
{
    LOGD(("CdmHost::%s", BSTD_FUNCTION));
    DIR *dir;
    struct dirent *dirEnt;

    if (names == NULL) {
        LOGD(("CdmHost::%s names is NULL", BSTD_FUNCTION));
    }

    names->clear();
    dir = opendir(m_basePath.c_str());
    if (dir)
    {
        while ((dirEnt = readdir(dir)) != NULL)
        {
            LOGD(("CdmHost::%s d_name=%s", BSTD_FUNCTION, dirEnt->d_name));
            names->push_back(dirEnt->d_name);
        }

        closedir(dir);
    }
    return true;
}
#endif

int64_t CdmHost::now()
{
    return now_;
}

void CdmHost::Reset()
{
    struct timeval tv;
    tv.tv_sec = tv.tv_usec = 0;
    gettimeofday(&tv, NULL);
    now_ = (tv.tv_sec * 1000LL) + (tv.tv_usec / 1000LL);

    // Surprisingly, std::priority_queue has no clear().
    while (!timers_.empty()) {
        timers_.pop();
    }
}

void CdmHost::setTimeout(int64_t delay_ms,
    IClient* client,
    void* context)
{
    int64_t expiry_time = now_ + delay_ms;
    timers_.push(Timer(expiry_time, client, context));
}

void CdmHost::cancel(IClient* client)
{
    BSTD_UNUSED(client);
}
