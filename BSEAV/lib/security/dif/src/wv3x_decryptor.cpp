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
#include <stdlib.h>
#include <errno.h>
#if defined(__linux__)
#include <sys/utsname.h>
#endif
#ifdef WV_CDM_V32
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
    LOGW(("%s: Cert req keyMessage(%d): %s", __FUNCTION__, (uint32_t)keyMessage.size(), keyMessage.c_str()));
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
    LOGD(("%s: enter", __FUNCTION__));

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
    LOGD(("%s: enter", __FUNCTION__));
#if 0
    if (!m_sessionId.empty()) {
        LOGD(("%s: calling CancelKeyRequest sessionId=%s",
            __FUNCTION__, m_sessionId.c_str()));
        CancelKeyRequest();
    }
#endif
    if (m_cdm) {
        LOGD(("%s: closing session_id:%s", __FUNCTION__, m_sessionId.c_str()));
        m_cdm->close(m_sessionId);
        delete m_cdm;
        m_cdm = NULL;
    }

    if (m_cdmHost) {
        delete m_cdmHost;
        m_cdmHost = NULL;
    }
    LOGD(("%s: leave", __FUNCTION__));
}

bool Widevine3xDecryptor::Initialize(std::string& pssh)
{
    Cdm::Status status;
    char* logLevelStr;
    int logLevel = 0;
    LOGD(("%s: pssh(%d): %s", __FUNCTION__, (uint32_t)pssh.size(), b2a_hex(pssh).c_str()));
    m_pssh.assign(pssh);
    m_keyId.assign(pssh.substr(4));
    LOGD(("%s: default keyId(%d): %s", __FUNCTION__, (uint32_t)m_keyId.size(), b2a_hex(m_keyId).c_str()));

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
    // TODO: privacy mode disabled for now
    bool privacyMode = false;
    if (m_cdm == NULL)
        m_cdm = reinterpret_cast<widevine::Cdm*>(Cdm::create(this, m_cdmHost, privacyMode));

    if (m_cdm == NULL) {
        LOGE(("Failed to create CDM"));
        return false;
    }

#if defined( WV_CDM_V32 )
    if (!privacyMode) {
        const std::string kDefaultCertificate = wvcdm::a2bs_hex(
        "0ABF020803121028703454C008F63618ADE7443DB6C4C8188BE7F99005228E023082010A02"
        "82010100B52112B8D05D023FCC5D95E2C251C1C649B4177CD8D2BEEF355BB06743DE661E3D"
        "2ABC3182B79946D55FDC08DFE95407815E9A6274B322A2C7F5E067BB5F0AC07A89D45AEA94"
        "B2516F075B66EF811D0D26E1B9A6B894F2B9857962AA171C4F66630D3E4C602718897F5E1E"
        "F9B6AAF5AD4DBA2A7E14176DF134A1D3185B5A218AC05A4C41F081EFFF80A3A040C50B09BB"
        "C740EEDCD8F14D675A91980F92CA7DDC646A06ADAD5101F74A0E498CC01F00532BAC217850"
        "BD905E90923656B7DFEFEF42486767F33EF6283D4F4254AB72589390BEE55808F1D668080D"
        "45D893C2BCA2F74D60A0C0D0A0993CEF01604703334C3638139486BC9DAF24FD67A07F9AD9"
        "4302030100013A1273746167696E672E676F6F676C652E636F6D128003983E30352675F40B"
        "A715FC249BDAE5D4AC7249A2666521E43655739529721FF880E0AAEFC5E27BC980DAEADABF"
        "3FC386D084A02C82537848CC753FF497B011A7DA97788A00E2AA6B84CD7D71C07A48EBF616"
        "02CCA5A3F32030A7295C30DA915B91DC18B9BC9593B8DE8BB50F0DEDC12938B8E9E039CDDE"
        "18FA82E81BB032630FE955D85A566CE154300BF6D4C1BD126966356B287D657B18CE63D0EF"
        "D45FC5269E97EAB11CB563E55643B26FF49F109C2101AFCAF35B832F288F0D9D45960E259E"
        "85FB5D24DBD2CF82764C5DD9BF727EFBE9C861F869321F6ADE18905F4D92F9A6DA6536DB84"
        "75871D168E870BB2303CF70C6E9784C93D2DE845AD8262BE7E0D4E2E4A0759CEF82D109D25"
        "92C72429F8C01742BAE2B3DECADBC33C3E5F4BAF5E16ECB74EADBAFCB7C6705F7A9E3B6F39"
        "40383F9C5116D202A20C9229EE969C2519718303B50D0130C3352E06B014D838540F8A0C22"
        "7C0011E0F5B38E4E298ED2CB301EB4564965F55C5D79757A250A4EB9C84AB3E6539F6B6FDF"
        "56899EA29914");

        status = m_cdm->setServiceCertificate(kDefaultCertificate);
        if (status != Cdm::kSuccess) {
            LOGE(("setServiceCertificate failed: %d", (int)status));
            return false;
        }
    }
#endif
    return true;
}

bool Widevine3xDecryptor::Provision()
{
    Cdm::Status status;
    LOGD(("%s: enter", __FUNCTION__));

    status = m_cdm->createSession(m_cdmSessionType, &m_sessionId);
    if (status != Cdm::kSuccess) {
        return false;
    }
    LOGD(("%s: created session_id:%s", __FUNCTION__, m_sessionId.c_str()));

    status = m_cdm->generateRequest(m_sessionId, Cdm::kCenc, m_initData);
    if (status != Cdm::kSuccess && status != Cdm::kDeferred) {
        LOGE(("GenerateKeyRequest failed."));
        m_cdm->close(m_sessionId);
        return false;
    }

    // Wait for the callback
    while (!m_gotCertReq){
        LOGW(("%s: waiting for cert request from CDM", __FUNCTION__));
        usleep(100000);
    }

    if (!m_certReq.empty()) {
        m_certificate = GetCertRequestResponse(m_certReq);

        LOGW(("%s: m_certificate=%s", __FUNCTION__, m_certificate.c_str()));

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
    LOGW(("%s: %d initData(%d): %s", __FUNCTION__, __LINE__, (uint32_t)initData.size(), b2a_hex(initData).c_str()));
    LOGW(("%s: type:%d", __FUNCTION__, type));

    m_hasKeyMessage = false;

    // Convert session type - actual values are same
    m_cdmSessionType = (Cdm::SessionType)type;

    m_initData.assign(initData);

    if (!Provision()) {
        LOGW(("%s: First provisioning failed", __FUNCTION__));
        // Try again just in case
        if (!Provision()) {
            LOGE(("%s: Provisioning failed", __FUNCTION__));
            return false;
        }
    }

    while (!HasNewKeyMessage()) {
        LOGW(("%s: waiting for new key", __FUNCTION__));
        usleep(100000);
    }

    return true;
}

std::string Widevine3xDecryptor::GetKeyRequestResponse(std::string url)
{
    m_url = url;
    std::string drm_msg;

        LOGW(("%s: keyMessage(%d): %s", __FUNCTION__, (uint32_t)m_keyMessage.size(), b2a_hex(m_keyMessage).c_str()));
        std::string message = "";
        s_wvBuffer.assign("");
#ifdef USE_CURL
       // url+= "SecurityLevel=L3";
        LOGW(("%s: server_url: %s", __FUNCTION__, url.c_str()));
        CURL *curl;
        CURLcode res;
        curl = curl_easy_init();
        if (curl == NULL) {
            LOGE(("%s: curl_easy_init returned NULL", __FUNCTION__));
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

bool Widevine3xDecryptor::AddKey(std::string key)
{
    std::string response;
    Cdm::Status status;
    Cdm::KeyStatusMap key_statuses;

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
    uint32_t sampleSize)
{
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
    LOGD(("%s: sampleSize=%u clearSize=%u encSize=%u", __FUNCTION__, sampleSize, clearSize, encSize));

    // DMA Transfer to the destination buffer
    if (output->IsSecure()) {
        output->Copy(0, input->GetPtr(), sampleSize);
    }
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

        LOGD(("%s: encrypted %u @ %u blkOffset=%u", __FUNCTION__,
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
            LOGE(("%s: Decrypt returned NoKey\n", __FUNCTION__));
            goto ErrorExit;
        } else if (result != Cdm::kSuccess) {
            LOGE(("%s: Decrypt returned error %d\n", __FUNCTION__, (int)result));
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
    LOGW(("%s: sessionId=%s", __FUNCTION__, m_sessionId.c_str()));
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
    LOGW(("%s: session_id=%s", __FUNCTION__, session_id.c_str()));
    status = m_cdm->getKeyStatuses(session_id, &key_statuses);
    if (status != Cdm::kSuccess) {
        LOGE(("%s: getKeyStatuses Error: %d", __FUNCTION__, status));
        return;
    }

    LOGW(("%s: keyStatus=%d", __FUNCTION__, key_statuses[session_id]));
    if (key_statuses[session_id] == Cdm::kUsable) {
        m_valid = true;
        return;
    }
    else if (key_statuses[session_id] == Cdm::kExpired) {
        LOGW(("%s: session %s expired", __FUNCTION__, session_id.c_str()));
    }
    m_valid = false;
}

void Widevine3xDecryptor::onRemoveComplete(
    const std::string& session_id)
{
    LOGW(("%s: session_id=%s", __FUNCTION__, session_id.c_str()));
}

void Widevine3xDecryptor::onDeferredComplete(
    const std::string& session_id, Cdm::Status result)
{
    LOGW(("%s: session_id=%s result=%d", __FUNCTION__, session_id.c_str(), result));
    m_gotCertReq = true;
}

void Widevine3xDecryptor::onDirectIndividualizationRequest(
    const std::string& session_id, const std::string& request)
{
    LOGW(("%s: session_id=%s request=%s", __FUNCTION__, session_id.c_str(), request.c_str()));
    m_certReq = request;
    m_gotCertReq = true;
}

void Widevine3xDecryptor::onMessage(
    const std::string& session_id,
    Cdm::MessageType message_type,
    const std::string& message)
{
    LOGW(("%s: session_id=%s type=%d message(%d)=%s", __FUNCTION__,
        session_id.c_str(), message_type, (uint32_t)message.size(), message.c_str()));
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
    LOGD(("%s: basePath=%s", __FUNCTION__, m_basePath.c_str()));
}

CdmHost::~CdmHost()
{
    LOGD(("%s: CdmHost %p destroyed", __FUNCTION__, (void*)this));
}

bool CdmHost::read(const std::string& name,
    std::string* data)
{
    LOGD(("CdmHost::%s: name=%s", __FUNCTION__, name.c_str()));

    if (data == NULL) {
        LOGD(("CdmHost::%s: data is NULL", __FUNCTION__));
        return false;
    }

    struct stat buf;
    std::string filePath = m_basePath;
    filePath.append(name);
    if (stat(filePath.c_str(), &buf) != 0) {
        LOGW(("%s: stat failed(%d): %s", __FUNCTION__, errno, filePath.c_str()));
        return false;
    }

    char* buffer = (char*)malloc(buf.st_size);
    FILE* fd = fopen(filePath.c_str(), "r+b");
    uint32_t len = fread(buffer, sizeof(char), buf.st_size, fd);
    if (len == 0) {
        LOGW(("%s: fread failed(%d): %s", __FUNCTION__, errno, filePath.c_str()));
    } else if ((int32_t)len != buf.st_size) {
        LOGW(("%s: fread incomplete (%d/%d): %s", __FUNCTION__, (int32_t)len, (int32_t)buf.st_size, filePath.c_str()));
    } else {
        data->assign(buffer, len);
    }

    return true;
}

static bool createDirectory(std::string path)
{
    LOGD(("%s: path: %s", __FUNCTION__, path.c_str()));
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
            LOGW(("%s: can't create: %s", __FUNCTION__, filePath.c_str()));
            return NULL;
        }
        fclose(fd);
        fd = NULL;
    }

    fd = fopen(filePath.c_str(), "r+b");
    if (fd == NULL) {
        LOGW(("%s: can't open: %s", __FUNCTION__, filePath.c_str()));
        return NULL;
    }

    return fd;
}

bool CdmHost::write(const std::string& name,
    const std::string& data)
{
    LOGW(("CdmHost::%s: name=%s", __FUNCTION__, name.c_str()));
    dump_hex("data", data.data(), data.size());

    FILE* fd = openFile(m_basePath, name);
    uint32_t len = fwrite(data.data(), sizeof(char), data.size(), fd);
    if (len != data.size()) {
        LOGW(("%s: fwrite failed(%d): len=%d %s", __FUNCTION__, errno, (int32_t)len, name.c_str()));
        return false;
    }
    fflush(fd);
    return true;
}

bool CdmHost::exists(const std::string& name)
{
    LOGD(("CdmHost::%s: name=%s", __FUNCTION__, name.c_str()));

    std::string filePath = m_basePath;
    filePath.append(name);
    FILE* fd = fopen(filePath.c_str(), "r+b");
    if (fd == NULL) {
        LOGD(("%s: can't open: %s", __FUNCTION__, filePath.c_str()));
        return false;
    }

    LOGD(("CdmHost::%s: return true", __FUNCTION__));
    return true;
}

bool CdmHost::remove(const std::string& name)
{
    bool rc = false;
    LOGD(("CdmHost::%s: name=%s", __FUNCTION__, name.c_str()));

    std::string filePath = m_basePath;
    filePath.append(name);
    rc = (unlink(filePath.c_str()) == 0);
    return rc;
}

int32_t CdmHost::size(const std::string& name)
{
    LOGD(("CdmHost::%s: name=%s", __FUNCTION__, name.c_str()));

    struct stat buf;
    std::string filePath = m_basePath;
    filePath.append(name);
    if (stat(filePath.c_str(), &buf) != 0) {
        LOGW(("%s: stat failed(%d): %s", __FUNCTION__, errno, filePath.c_str()));
        return -1;
    }
    LOGD(("CdmHost::%s: size=%d", __FUNCTION__, (int32_t)buf.st_size));
    return (int32_t)buf.st_size;
}

#ifdef WV_CDM_V32
bool CdmHost::list(std::vector<std::string>* names)
{
    LOGD(("CdmHost::%s", __FUNCTION__));
    DIR *dir;
    struct dirent *dirEnt;

    if (names == NULL) {
        LOGD(("CdmHost::%s names is NULL", __FUNCTION__));
    }

    names->clear();
    dir = opendir(m_basePath.c_str());
    if (dir)
    {
        while ((dirEnt = readdir(dir)) != NULL)
        {
            LOGD(("CdmHost::%s d_name=%s", __FUNCTION__, dirEnt->d_name));
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
