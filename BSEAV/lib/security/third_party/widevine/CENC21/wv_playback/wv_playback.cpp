/******************************************************************************
 *    (c)2008-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Example to playback Playready DRM encrypted content
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/
/* Nexus example app: Widevine decrypt, MP4 parser, and PES conversion decode */

#ifdef ANDROID
#define LOG_NDEBUG 0
#define LOG_TAG "wv_playback"
#include <utils/Log.h>
#define LOGE(x) ALOGE x
#define LOGW(x) ALOGW x
#define LOGD(x) ALOGD x
#define LOGV(x) ALOGV x
#else
#include "log.h"
#undef LOGE
#undef LOGW
#undef LOGD
#undef LOGV
#define LOGE BDBG_ERR
#define LOGW BDBG_WRN
#define LOGD BDBG_MSG
#define LOGV BDBG_MSG
#endif

#include "nexus_platform.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_video_adj.h"
#include "nexus_playback.h"
#include "nexus_core_utils.h"

#include "common_crypto.h"
#include "wv_content_decryption_module.h"
#ifndef ANDROID
#include "content_decryption_module.h"
#include "device_cert.h"
#endif
#include "wv_cdm_constants.h"
#ifndef USE_CURL
#include "url_request.h"
#endif
#include "string_conversions.h"
#include <arpa/inet.h>

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <queue>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bmp4_util.h"
#include "bbase64.h"
#include "bfile_stdio.h"
extern "C" {
#include "mp4_parser.h"
#include "bmp4.h"
}
#include "bmedia_types.h"
#include "bmedia_probe.h"
#include "bmedia_probe_es.h"
#include "bmp4_parser.h"
#include "bmp4_probe.h"
#include "bfile_cache.h"

#include "nxclient.h"
#include "nexus_surface_client.h"

#ifdef USE_CURL
#include <curl/curl.h>
#endif // USE_CURL

#define NEED_TO_BE_TRUSTED_APP
#ifdef NEED_TO_BE_TRUSTED_APP
//#include <cutils/properties.h>
#define NEXUS_TRUSTED_DATA_PATH "/data/misc/nexus"
#endif

#define USE_MOCK_OEMCRYPTO 0
#define USE_SECURE_PLAYBACK (USE_SECURE_VIDEO_PLAYBACK || USE_SECURE_AUDIO_PLAYBACK)
#define DEBUG_OUTPUT_CAPTURE 0

#define ZORDER_TOP 10

#if USE_SECURE_PLAYBACK
#include "sage_srai.h"
#endif

#define REPACK_VIDEO_PES_ID 0xE0
#define REPACK_AUDIO_PES_ID 0xC0

#define BOX_HEADER_SIZE (8)
#define BUF_SIZE (1024 * 1024 * 2) /* 2MB */

#define CALCULATE_PTS(t) (((uint64_t)(t) / 10000LL) * 45LL)

/*
 * Flags indicating data endpoints in OEMCrypto_DecryptCTR.
 */
#define OEMCrypto_FirstSubsample 1
#define OEMCrypto_LastSubsample 2

BDBG_MODULE(wv_playback);

using namespace wvcdm;

typedef struct bmp4_probe  *bmp4_probe_t;

typedef struct b_mp4_probe_handler {
        bmp4_parser_handler handler; /* must be first */
        bmp4_probe_t probe; /* pointer to probe */
} b_mp4_probe_handler;

struct bmp4_probe {
        BDBG_OBJECT(bmp4_probe_t)
        bmp4_parser_t parser;
        bmp4_probe_stream *stream;
        bmp4_probe_track *track;
        off_t next_seek;
        bool movieheader_valid;
        bool movie_valid;
        bool stream_error;
        bool trackheader_valid;
        bool sample_valid;
        bool sample_size_valid;
        bool mediaheader_valid;
        bool handler_valid;
    bool mdat_valid;
    bfile_segment samplesize;
    bfile_segment compactsamplesize;
    batom_factory_t factory;
    uint32_t handler_type;
        b_mp4_probe_handler filetype_handler;
        b_mp4_probe_handler movie_handler;
        b_mp4_probe_handler track_handler;
        b_mp4_probe_handler media_handler;
        b_mp4_probe_handler mediainfo_handler;
        b_mp4_probe_handler sampletable_handler;
};

struct bmedia_probe {
    BDBG_OBJECT(bmedia_probe_t)
    batom_factory_t factory;
    batom_pipe_t pipe;
    bmedia_probe_es_t es_probe;
    bmedia_probe_base_t probes[20];
};

typedef struct app_ctx {
    FILE *fp_mp4;
    uint32_t mp4_filesize;

    uint8_t *pPayload;
    uint8_t *pAudioOutBuf;
    uint8_t *pVideoOutBuf;
    uint8_t *pAudioHeaderBuf;
    uint8_t *pVideoHeaderBuf;

    size_t videoOutBufSize;
    size_t audioOutBufSize;

    uint64_t last_video_fragment_time;
    uint64_t last_audio_fragment_time;

    WvContentDecryptionModule *decryptor;
    CdmSessionId *session_id;
} app_ctx;

#ifdef ANDROID
typedef struct DRM_Context {
    WvContentDecryptionModule decryptor;
    CdmKeyMessage key_msg;
    CdmSessionId session_id;
    CdmKeySetId key_set_id;
} DRM_Context;
#else
class OutputBuffer : public cdm::Buffer {
 public:

  virtual void Destroy() OVERRIDE {}

  virtual int32_t Capacity() const OVERRIDE {return capacity_;}
  virtual uint8_t* Data() OVERRIDE {return buffer_;}
  virtual void SetSize(int32_t size) OVERRIDE {size_ = size;}
  virtual int32_t Size() const OVERRIDE {return size_;}

  // OutputBuffer can only be created by calling Create().
  OutputBuffer(uint8_t* buffer, uint32_t capacity)
      : buffer_(buffer),
        capacity_(capacity) {
  }
  // OutputBuffer can only be destroyed by calling Destroy().
  virtual ~OutputBuffer() {
  }
  uint8_t* buffer_;
  int32_t capacity_;
  int32_t size_;

  CORE_DISALLOW_COPY_AND_ASSIGN (OutputBuffer);
};

class ProxyDecryptedBlock : public cdm::DecryptedBlock {
 public:
  ProxyDecryptedBlock() : buffer_(NULL), timestamp_(0) {}
  virtual ~ProxyDecryptedBlock() {}

  virtual void SetDecryptedBuffer(cdm::Buffer* buffer) OVERRIDE  {
    buffer_ = buffer;
  }

  virtual cdm::Buffer* DecryptedBuffer() OVERRIDE { return buffer_; }

  virtual void SetTimestamp(int64_t timestamp) OVERRIDE {
    timestamp_ = timestamp;
  }
  virtual int64_t Timestamp() const OVERRIDE { return timestamp_; }

 private:
  cdm::Buffer* buffer_;
  int64_t timestamp_;

  CORE_DISALLOW_COPY_AND_ASSIGN(ProxyDecryptedBlock);
};

class TestHost : public cdm::Host
{
public:
    // These structs are used to store the KeyMessages and KeyErrors passed to
    // this class' objects.
    struct KeyMessage {
        std::string session_id;
        std::string message;
        std::string default_url;
    };

    struct KeyError {
        KeyError() : error_code(cdm::kUnknownError), system_code(0) {}
        std::string session_id;
        cdm::MediaKeyError error_code;
        uint32_t system_code;
    };

    TestHost();
    virtual ~TestHost();

    // cdm::Host implementation.
    virtual cdm::Buffer* Allocate(int32_t capacity) OVERRIDE;

    virtual void SetTimer(int64_t delay_ms, void* context) OVERRIDE;

    virtual double GetCurrentWallTimeInSeconds() OVERRIDE;

    virtual void SendKeyMessage(const char* session_id, int32_t session_id_length,
                                const char* message, int32_t message_length,
                                const char* default_url,
                                int32_t default_url_length) OVERRIDE;

    virtual void SendKeyError(const char* session_id, int32_t session_id_length,
                              cdm::MediaKeyError error_code,
                              uint32_t system_code) OVERRIDE;

    virtual void GetPlatformString(const std::string& name,
                                   std::string* value) OVERRIDE;

    virtual void SetPlatformString(const std::string& name,
                                   const std::string& value) OVERRIDE;

	void SetCdmPtr(cdm::ContentDecryptionModule* cdm);
	cdm::ContentDecryptionModule* GetCdmPtr();

    bool HasNewKeyMessage() {return has_new_key_message_;}
    KeyMessage keyMessage;
private:
    struct Timer {
        Timer(double expiry_time, void* context)
            : expiry_time(expiry_time), context(context) {}

        bool operator<(const Timer& other) const {
            // We want to reverse the order so that the smallest expiry times go to
            // the top of the priority queue.
            return expiry_time > other.expiry_time;
        }

        double expiry_time;
        void* context;
    };

    double current_time_;
    std::priority_queue<Timer> timers_;

    std::vector<KeyMessage> key_messages_;
    std::vector<KeyError> key_errors_;

    bool has_new_key_message_;
    bool has_new_key_error_;

    std::map<std::string, std::string> platform_strings_;

    cdm::ContentDecryptionModule* cdm_;

    CORE_DISALLOW_COPY_AND_ASSIGN(TestHost);
};

TestHost::TestHost() : cdm_(NULL)
{
}

TestHost::~TestHost()
{
    if (cdm_)
        cdm_->Destroy();
}

cdm::Buffer* TestHost::Allocate(int32_t capacity)
{
	BSTD_UNUSED(capacity);
	return NULL;
//    return TestBuffer::Create(capacity);
}

void TestHost::SetTimer(int64_t delay_ms, void* context)
{
    double expiry_time = current_time_ + (delay_ms / 1000.0);
    timers_.push(Timer(expiry_time, context));
}

double TestHost::GetCurrentWallTimeInSeconds()
{
    return current_time_;
}

void TestHost::SendKeyMessage(const char* session_id, int32_t session_id_length,
                              const char* message, int32_t message_length,
                              const char* default_url,
                              int32_t default_url_length)
{
//    KeyMessage key_message;
    keyMessage.session_id.assign(session_id, session_id_length);
    keyMessage.message.assign(message, message_length);
    keyMessage.default_url.assign(default_url, default_url_length);
    key_messages_.push_back(keyMessage);
    has_new_key_message_ = true;
    LOGD(("%s: key_msg(%d): %s", __FUNCTION__, keyMessage.message.size(), b2a_hex(keyMessage.message).c_str()));
}


void TestHost::SendKeyError(const char* session_id, int32_t session_id_length,
                            cdm::MediaKeyError error_code,
                            uint32_t system_code)
{
    KeyError key_error;
    key_error.session_id.assign(session_id, session_id_length);
    key_error.error_code = error_code;
    key_error.system_code = system_code;
    key_errors_.push_back(key_error);
    has_new_key_error_ = true;
}




void TestHost::GetPlatformString(const std::string& name,
                                 std::string* value)
{
    *value = platform_strings_[name];
}

void TestHost::SetPlatformString(const std::string& name,
                                 const std::string& value)
{
    platform_strings_[name] = value;
}

void TestHost::SetCdmPtr(cdm::ContentDecryptionModule* cdm)
{
    if (cdm_) {
        cdm_->Destroy();
    }

    cdm_ = cdm;
}

cdm::ContentDecryptionModule* TestHost::GetCdmPtr()
{

    return cdm_;
}
#endif // ANDROID

void dump100(uint8_t* ptr)
{
LOGD(("ptr:%p", ptr));
LOGD(("0-7: %x %x %x %x %x %x %x %x",
 ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7]));
LOGD(("8-15: %x %x %x %x %x %x %x %x",
 ptr[8], ptr[9], ptr[10], ptr[11], ptr[12], ptr[13], ptr[14], ptr[15]));
LOGD(("16-23: %x %x %x %x %x %x %x %x",
 ptr[16], ptr[17], ptr[18], ptr[19], ptr[20], ptr[21], ptr[22], ptr[23]));
LOGD(("24-31: %x %x %x %x %x %x %x %x",
 ptr[24], ptr[25], ptr[26], ptr[27], ptr[28], ptr[29], ptr[30], ptr[31]));
LOGD(("32-39: %x %x %x %x %x %x %x %x",
 ptr[32], ptr[33], ptr[34], ptr[35], ptr[36], ptr[37], ptr[38], ptr[39]));
LOGD(("40-47: %x %x %x %x %x %x %x %x",
 ptr[40], ptr[41], ptr[42], ptr[43], ptr[44], ptr[45], ptr[46], ptr[47]));
LOGD(("48-55: %x %x %x %x %x %x %x %x",
 ptr[48], ptr[49], ptr[50], ptr[51], ptr[52], ptr[53], ptr[54], ptr[55]));
LOGD(("56-63: %x %x %x %x %x %x %x %x",
 ptr[56], ptr[57], ptr[58], ptr[59], ptr[60], ptr[61], ptr[62], ptr[63]));
LOGD(("64-71: %x %x %x %x %x %x %x %x",
 ptr[64], ptr[65], ptr[66], ptr[67], ptr[68], ptr[69], ptr[70], ptr[71]));
LOGD(("72-79: %x %x %x %x %x %x %x %x",
 ptr[72], ptr[73], ptr[74], ptr[75], ptr[76], ptr[77], ptr[78], ptr[79]));
LOGD(("80-87: %x %x %x %x %x %x %x %x",
 ptr[80], ptr[81], ptr[82], ptr[83], ptr[84], ptr[85], ptr[86], ptr[87]));
LOGD(("88-95: %x %x %x %x %x %x %x %x",
 ptr[88], ptr[89], ptr[90], ptr[91], ptr[92], ptr[93], ptr[94], ptr[95]));
}

// WV Specific
const std::string kWidevineKeySystem = "com.widevine.alpha";
const std::string kWidevineSystemId = "edef8ba979d64acea3c827dcd51d21ed";
const std::string kWvKeyId =
    "70737368"  // "pssh"
    "00000000";  // flags

// Content Protection license server data
const std::string kCpLicenseServer =
    "http://widevine-proxy.appspot.com/proxy";
const std::string kCpClientAuth = "";

// Google Play license server data
const std::string kGpLicenseServer =
"http://dash-mse-test.appspot.com/api/drm/widevine";
//    "http://license.uat.widevine.com/getlicense/widevine_test";
//    "https://jmt17.google.com/video/license/GetCencLicense";

// Test client authorization string.
// NOTE: Append a userdata attribute to place a unique marker that the
// server team can use to track down specific requests during debugging
// e.g., "<existing-client-auth-string>&userdata=<your-ldap>.<your-tag>"
//       "<existing-client-auth-string>&userdata=jbmr2.dev"
const std::string kGpClientAuth =
"?drm_system=widevine&source=YOUTUBE&video_id=03681262dc412c06&ip=0.0.0.0&ipbits=0&expire=19000000000&sparams=ip,ipbits,expire,source,video_id,drm_system&signature=289105AFC9747471DB0D2A998544CC1DAF75B8F9.18DE89BB7C1CE9B68533315D0F84DF86387C6BB3&key=test_key1";
//    "?source=YOUTUBE&video_id=EGHC6OHNbOo&oauth=ya.gtsqawidevine";

const std::string kGpPssh = "08011210e02562e04cd55351b14b3d748d36ed8e";

const std::string kGpClientOfflineQueryParameters =
"";
//    "&offline=true";
const std::string kGpClientOfflineRenewalQueryParameters =
    "&offline=true&renewal=true";
const std::string kGpClientOfflineReleaseQueryParameters =
    "&offline=true&release=true";

// global var
wvcdm::KeyId g_key_id;
wvcdm::KeyId g_key;
std::string g_pssh;
std::string g_license_server;
std::string g_client_auth;

#ifdef ANDROID
DRM_Context *drm_context_ = NULL;
#else
TestHost *host_ = NULL;
#endif

std::string msgBuffer;
#ifdef USE_CURL
static size_t curl_writeback( void *ptr, size_t size, size_t nmemb, void *stream)
{
	BSTD_UNUSED(stream);
    msgBuffer.append((char*)ptr, size * nmemb);
    return size * nmemb;
}
#endif // USE_CURL

static std::string GetKeyRequestResponse(
    const std::string& server_url, const std::string& client_auth)
{
#ifdef ANDROID
    LOGD(("%s: key_msg(%d): %s", __FUNCTION__, drm_context_->key_msg.size(), b2a_hex(drm_context_->key_msg).c_str()));
#else
    LOGD(("%s: key_msg(%d): %s", __FUNCTION__, host_->keyMessage.message.size(), b2a_hex(host_->keyMessage.message).c_str()));
#endif
    std::string drm_msg;
    std::string message = "";
    msgBuffer.assign("");
#ifdef USE_CURL
    std::string dest_url = server_url + client_auth;
    LOGW(("%s: server_url: %s", __FUNCTION__, dest_url.c_str()));
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (curl == NULL) {
        LOGE(("%s: curl_easy_init returned NULL", __FUNCTION__));
        return drm_msg;
    }
//    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, dest_url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
#ifdef ANDROID
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, drm_context_->key_msg.data());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, drm_context_->key_msg.size());
#else
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, host_->keyMessage.message.data());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, host_->keyMessage.message.size());
#endif
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writeback);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Widevine CDM v1.0");
    res = curl_easy_perform(curl);
    LOGD(("msgBuffer: %s, res: %d", msgBuffer.c_str(), res));

    if (res != 0) {
        LOGE(("%s: curl error %d", __FUNCTION__, res));
        curl_easy_cleanup(curl);
        return drm_msg;
    }
#else // USE_CURL
    UrlRequest url_request(server_url + client_auth);
    if (!url_request.is_connected()) {
        return "";
    }
#ifdef ANDROID
    url_request.PostRequest(drm_context_->key_msg);
#else
    url_request.PostRequest(host_->keyMessage.message);
#endif
    int resp_bytes = url_request.GetResponse(&message);

    int status_code = url_request.GetStatusCode(message);

    LOGD(("GetStatusCode returned %d", status_code));
    if (status_code != 200) {
        LOGE(("%s: error status_code=%d", __FUNCTION__, status_code));
        return drm_msg;
    }
    msgBuffer = message;
#endif // USE_CURL
    size_t body_head = msgBuffer.find("\r\n\r\n");
    if (body_head == std::string::npos) {
        LOGE(("%s: no body found in response", __FUNCTION__));
#ifdef USE_CURL
        curl_easy_cleanup(curl);
#endif
        return drm_msg;
    }
    drm_msg.clear();
    body_head += 4;
    size_t drm_head = msgBuffer.find("\r\n\r\n", body_head);
    if (drm_head != std::string::npos) {
        LOGD(("%s: DRM message found", __FUNCTION__));
        drm_head += 4;
        drm_msg = msgBuffer.substr(drm_head);
    } else {
        drm_head = msgBuffer.find("\r\n", body_head);
        if (drm_head != std::string::npos) {
            LOGD(("%s: old style DRM message found", __FUNCTION__));
            drm_head += 2;
            drm_msg = msgBuffer.substr(drm_head);
        } else {
            LOGD(("%s: return body anyway", __FUNCTION__));
            drm_msg = msgBuffer.substr(body_head);
        }
    }

    LOGD(("HTTP response body: (%u bytes): %s", drm_msg.size(), b2a_hex(drm_msg).c_str()));
#ifdef USE_CURL
    curl_easy_cleanup(curl);
#endif
    return drm_msg;
}

static std::string GetCertRequestResponse(std::string& server_url)
{
    std::string message = "";
    msgBuffer.assign("");
#ifdef USE_CURL
    server_url += "&signedRequest=";
#ifdef ANDROID
    server_url += drm_context_->key_msg;
#else
    server_url += host_->keyMessage.message;
#endif
    LOGW(("%s: server_url: %s", __FUNCTION__, server_url.c_str()));
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
        LOGW(("msgBuffer: %s, res: %d", msgBuffer.c_str(), res));
        message = msgBuffer;

        curl_easy_cleanup(curl);
    }
#else // USE_CURL
    // Use secure connection and chunk transfer coding.
    UrlRequest url_request(server_url);
    if (!url_request.is_connected()) {
      return "";
    }

#ifdef ANDROID
    url_request.PostCertRequestInQueryString(drm_context_->key_msg);
#else
    url_request.PostCertRequestInQueryString(host_->keyMessage.message);
#endif
    int resp_bytes = url_request.GetResponse(&message);
    LOGW(("end %d bytes response dump", resp_bytes));

    int status_code = url_request.GetStatusCode(message);
    if (status_code != 200) {
        LOGE(("GetStatusCode error: %d", status_code));
    }
    msgBuffer = message;
#endif // USE_CURL
    return message;
}

#if DEBUG_OUTPUT_CAPTURE
/* Input file pointer */
FILE *fp_vid;
FILE *fp_aud;
#endif

/* stream type */
int vc1_stream = 0;
typedef app_ctx * app_ctx_t;
static int video_decode_hdr;

static int gui_init( NEXUS_SurfaceClientHandle surfaceClient );

static void complete(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static NEXUS_Error SecureCopy(void *pDest, const void *pSrc, size_t nSize)
{
    NEXUS_DmaHandle dma;
    NEXUS_DmaJobHandle job;
    NEXUS_DmaJobSettings jobSettings;
    BKNI_EventHandle event;
    NEXUS_Error rc;

    LOGV(("%s: dest:%p, src:%p, size:%d", __FUNCTION__, pDest, pSrc, nSize));
    BKNI_CreateEvent(&event);
    dma = NEXUS_Dma_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT(dma);

    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.completionCallback.callback = complete;
    jobSettings.completionCallback.context = event;
    jobSettings.bypassKeySlot = NEXUS_BypassKeySlot_eGR2R;
    job = NEXUS_DmaJob_Create(dma, &jobSettings);
    BDBG_ASSERT(job);

    NEXUS_DmaJobBlockSettings blockSettings;
    NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);
    blockSettings.pSrcAddr = pSrc;
    blockSettings.pDestAddr = pDest;
    blockSettings.blockSize = nSize;
    blockSettings.cached = false;
    rc = NEXUS_DmaJob_ProcessBlocks(job, &blockSettings, 1);
    if (rc == NEXUS_DMA_QUEUED) {
        NEXUS_DmaJobStatus status;
        rc = BKNI_WaitForEvent(event, BKNI_INFINITE);
        BDBG_ASSERT(!rc);
        rc = NEXUS_DmaJob_GetStatus(job, &status);
        BDBG_ASSERT(!rc && (status.currentState == NEXUS_DmaJobState_eComplete));
    }
    else {
        LOGE(("%s: error in dma transfer, err:%d", __FUNCTION__, rc));
        return rc;
    }
    NEXUS_DmaJob_Destroy(job);
    NEXUS_Dma_Close(dma);
    BKNI_DestroyEvent(event);
    return NEXUS_SUCCESS;
}

static int mp4_playback_dma_buffer(CommonCryptoHandle commonCryptoHandle, void *dst,
        void *src, size_t size, bool flush)
{
    NEXUS_DmaJobBlockSettings blkSettings;
    CommonCryptoJobSettings cryptoJobSettings;

    NEXUS_DmaJob_GetDefaultBlockSettings(&blkSettings);
    blkSettings.pSrcAddr = src;
    blkSettings.pDestAddr = dst;
    blkSettings.blockSize = size;
    blkSettings.resetCrypto = true;
    blkSettings.scatterGatherCryptoStart = true;
    blkSettings.scatterGatherCryptoEnd = true;

    if (flush) {
        /* Need to flush manually the source buffer (non secure heap). We need to flush manually as soon as we copy data into
           the secure heap. Setting blkSettings[ii].cached = true would also try to flush the destination address in the secure heap
           which is not accessible. This would cause the whole memory to be flushed at once. */
        NEXUS_FlushCache(blkSettings.pSrcAddr, blkSettings.blockSize);
        blkSettings.cached = false; /* Prevent the DMA from flushing the buffers later on */
    }

    CommonCrypto_GetDefaultJobSettings(&cryptoJobSettings);
    CommonCrypto_DmaXfer(commonCryptoHandle,  &cryptoJobSettings, &blkSettings, 1);

    if (flush) {
        /* Need to flush manually the source buffer (non secure heap). We need to flush manually as soon as we copy data into
           the secure heap. Setting blkSettings[ii].cached = true would also try to flush the destination address in the secure heap
           which is not accessible. This would cause the whole memory to be flushed at once. */
        NEXUS_FlushCache(blkSettings.pSrcAddr, blkSettings.blockSize);
    }

    return 0;
}

static int parse_esds_config(bmedia_adts_hdr *hdr, bmedia_info_aac *info_aac, size_t payload_size)
{
    bmedia_adts_header adts_header;

    bmedia_adts_header_init_aac(&adts_header, info_aac);
    adts_header.adts[2] = 0x50;
    bmedia_adts_hdr_init(hdr, &adts_header, payload_size);

    return 0;
}

static int parse_avcc_config(uint8_t *avcc_hdr, size_t *hdr_len, size_t *nalu_len,
        uint8_t *cfg_data, size_t cfg_data_size)
{
    bmedia_h264_meta meta;
    unsigned int i, sps_len, pps_len;
    uint8_t *data;
    uint8_t *dst;
//    size_t size;

    bmedia_read_h264_meta(&meta, cfg_data, cfg_data_size);

    *nalu_len = meta.nalu_len;

    data = (uint8_t *)meta.sps.data;
    dst = avcc_hdr;
    *hdr_len = 0;

    for(i=0; i < meta.sps.no; i++) {
        sps_len = (((uint16_t)data[0]) <<8) | data[1];
        data += 2;
        /* Add NAL */
        BKNI_Memcpy(dst, bmp4_nal, sizeof(bmp4_nal)); dst += sizeof(bmp4_nal);
        /* Add SPS */
        BKNI_Memcpy(dst, data, sps_len);
        dst += sps_len;
        data += sps_len;
        *hdr_len += (sizeof(bmp4_nal) + sps_len);
    }

    data = (uint8_t *)meta.pps.data;
    for(i=0; i < meta.pps.no; i++) {
        pps_len = (((uint16_t)data[0]) <<8) | data[1];
        data += 2;
        /* Add NAL */
        BKNI_Memcpy(dst, bmp4_nal, sizeof(bmp4_nal));
        dst += sizeof(bmp4_nal);
        /* Add PPS */
        BKNI_Memcpy(dst, data, pps_len);
        dst += pps_len;
        data += pps_len;
        *hdr_len += (sizeof(bmp4_nal) + pps_len);
    }
    return 0;
}

#ifdef ANDROID
void incrementIV(uint64_t increaseBy, std::vector<uint8_t>* ivPtr)
{
  std::vector<uint8_t>& iv = *ivPtr;
  uint64_t* counterBuffer = reinterpret_cast<uint64_t*>(&iv[8]);
  (*counterBuffer) = htonq(ntohq(*counterBuffer) + increaseBy);
}
#endif

static int decrypt_sample(CommonCryptoHandle commonCryptoHandle,
        uint32_t sampleSize, batom_cursor * cursor,
        bmp4_drm_mp4_box_se_sample *pSample, uint32_t *bytes_processed,
        app_ctx *app, uint8_t *out, size_t decrypt_offset)
{
    uint32_t rc = 0;
    uint8_t i = 0;

    *bytes_processed = 0;

#ifndef ANDROID
    cdm::InputBuffer inputBuffer;

    // set up subsamples
    std::vector<cdm::SubsampleEntry> subsamples;
    subsamples.reserve(pSample->nbOfEntries);
#endif

    size_t nalSize = sizeof(bmp4_nal);
    size_t clearSize = 0;
    size_t encSize = 0;
    for (i = 0; i <  pSample->nbOfEntries; i++) {
#ifndef ANDROID
        subsamples.push_back(cdm::SubsampleEntry(pSample->entries[i].bytesOfClearData, pSample->entries[i].bytesOfEncData));
#endif
        clearSize += pSample->entries[i].bytesOfClearData;
        encSize += pSample->entries[i].bytesOfEncData;
    }
LOGD(("%s: sampleSize=%u clearSize=%u encSize=%u", __FUNCTION__, sampleSize, clearSize, encSize));

    uint8_t *pBuf = NULL;
    uint8_t *encrypted_buffer = NULL;
    // Allocate a temporary buffer from Nexus heap
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_MemoryAllocationSettings memSettings;
    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
    memSettings.heap = clientConfig.heap[1]; /* heap 1 is the eFull heap for the nxclient. */

    NEXUS_Memory_Allocate(sampleSize, &memSettings, (void **)&pBuf);

    if (pBuf == NULL) {
        LOGE(("%s: Memory not allocated from Nexus heap[1]", __FUNCTION__));
        return -1;
    }
//    LOGV(("%s >> allocated %d bytes to %p", __FUNCTION__, sampleSize, pBuf));

    if (decrypt_offset != 0) { // video case
        size_t dst_offset = 0;
        uint32_t entryDataSize;
        do {
            BKNI_Memcpy(pBuf + dst_offset, bmp4_nal, nalSize);
            entryDataSize = batom_cursor_uint32_be(cursor);
LOGV(("%s >> entryDataSize=%u", __FUNCTION__, entryDataSize));
            dst_offset += sizeof(entryDataSize);
            BKNI_Memcpy(pBuf + dst_offset, cursor->cursor, entryDataSize);
            dst_offset += entryDataSize;
            batom_cursor_skip(cursor, entryDataSize);
        } while (dst_offset < clearSize + encSize);
    } else { // audio case
        BKNI_Memcpy(pBuf, cursor->cursor, sampleSize);
        batom_cursor_skip(cursor, sampleSize);
    }

#if USE_SECURE_PLAYBACK && !USE_MOCK_OEMCRYPTO
    // DMA Transfer to the destination buffer
    mp4_playback_dma_buffer(commonCryptoHandle, out, pBuf, sampleSize, true);
    encrypted_buffer = out;
#else
    BKNI_Memcpy(out, pBuf, sampleSize);
    encrypted_buffer = pBuf;
#endif

    if (encSize == 0) {
        // No decrypt needed - just return
        *bytes_processed += sampleSize;
        if (pBuf)
            NEXUS_Memory_Free(pBuf);
        return rc;
    }

    g_key = g_pssh.substr(4);
#ifdef ANDROID
    CdmDecryptionParameters decryption_parameters;
#if USE_SECURE_PLAYBACK
    decryption_parameters.is_secure = true; // SVP
#else
    decryption_parameters.is_secure = false; // non-SVP
#endif
    decryption_parameters.key_id = &g_key;
//LOGD(("%s: %d key_id= %s", __FUNCTION__, __LINE__, b2a_hex(*decryption_parameters.key_id).c_str()));
    std::vector<uint8_t> ivVector(pSample->iv, pSample->iv + KEY_IV_SIZE);
    decryption_parameters.iv = &ivVector;
    decryption_parameters.decrypt_buffer = out;
    decryption_parameters.decrypt_buffer_length = sampleSize;

    size_t offset = 0;
    size_t blockOffset = 0;

    for (i = 0; i <  pSample->nbOfEntries; i++) {
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

        CdmResponseType res = app->decryptor->Decrypt(
            *app->session_id, true, decryption_parameters);
        if (res != NO_ERROR && res != KEY_ADDED && res != KEY_MESSAGE &&
            res != KEY_CANCELED) {
            LOGE(("%s: Decrypt failed - %d", __FUNCTION__, res));
            rc = -1;
            goto ErrorExit;
        }

        offset += num_enc;
        blockOffset += num_enc;
        incrementIV(blockOffset / 16, &ivVector);
        blockOffset %= 16;
    }
#else
    inputBuffer.subsamples = subsamples.data();
    inputBuffer.num_subsamples = subsamples.size();

    for (int i = 0; i < inputBuffer.num_subsamples; ++i) {
        const cdm::SubsampleEntry& subsample = inputBuffer.subsamples[i];
LOGD(("subsample[%d]: clearSize=%u encSize=%u", i, subsample.clear_bytes, subsample.cipher_bytes));
    }

    // IV
    inputBuffer.iv = pSample->iv;
    inputBuffer.iv_size = BMP4_MAX_IV_ENTRIES;

    // Key Id
    inputBuffer.key_id = (const uint8_t*)g_key.data();
    inputBuffer.key_id_size = g_key.size();

    // Data
    inputBuffer.data_size = sampleSize;
#if USE_SECURE_PLAYBACK && !USE_MOCK_OEMCRYPTO
    inputBuffer.data_offset = decrypt_offset;
#else
    inputBuffer.data_offset = 0;
#endif
    inputBuffer.data = encrypted_buffer;

    OutputBuffer outputBuffer(out, sampleSize);
    ProxyDecryptedBlock outputBlock;
    outputBlock.SetDecryptedBuffer(&outputBuffer);

    cdm::Status result = app->decryptor->Decrypt(inputBuffer, &outputBlock);

    if (result == cdm::kNoKey) {
        LOGE(("%s: Decrypt returned NoKey\n", __FUNCTION__));
        return -1;
    } else if (result != cdm::kSuccess) {
        LOGE(("%s: Decrypt returned error %d\n", __FUNCTION__, (int)result));
        return -1;
    }
#endif

    *bytes_processed += sampleSize;

ErrorExit:
    if (pBuf)
        NEXUS_Memory_Free(pBuf);
    return rc;
}

static int process_fragment(CommonCryptoHandle commonCryptoHandle, app_ctx *app, mp4_parse_frag_info *frag_info,
        size_t payload_size, void *decoder_data, size_t decoder_len)
{
    int rc = 0;
    uint32_t i, bytes_processed;
    bmp4_drm_mp4_box_se_sample *pSample;
    uint8_t pes_header[BMEDIA_PES_HEADER_MAX_SIZE];
    size_t pes_header_len;
    bmedia_pes_info pes_info;
    uint64_t frag_duration;
    uint8_t *pAudioOutBuf = app->pAudioOutBuf;
    uint8_t *pVideoOutBuf = app->pVideoOutBuf;
    size_t decrypt_offset = 0;

    app->audioOutBufSize = 0;
    app->videoOutBufSize = 0;
    bytes_processed = 0;
    if (frag_info->samples_enc->sample_count != 0) {
LOGD(("%s: #samples=%d",__FUNCTION__, frag_info->samples_enc->sample_count));
        for (i = 0; i < frag_info->samples_enc->sample_count; i++) {
            size_t numOfByteDecrypted = 0;
            size_t sampleSize = 0;

            pSample = &frag_info->samples_enc->samples[i];
            sampleSize = frag_info->sample_info[i].size;

#if 1
            if (frag_info->trackType == BMP4_SAMPLE_ENCRYPTED_VIDEO ||
                frag_info->trackType == BMP4_SAMPLE_AVC) {
#else
            if (frag_info->trackType == BMP4_SAMPLE_ENCRYPTED_VIDEO) {
#endif
                /* H.264 Decoder configuration parsing */
                uint8_t avcc_hdr[BMP4_MAX_PPS_SPS];
                size_t avcc_hdr_size = 0;
                size_t nalu_len = 0;

                parse_avcc_config(avcc_hdr, &avcc_hdr_size, &nalu_len, (uint8_t*)decoder_data, decoder_len);

                bmedia_pes_info_init(&pes_info, REPACK_VIDEO_PES_ID);
                frag_duration = app->last_video_fragment_time +
                    (int32_t)frag_info->sample_info[i].composition_time_offset;
                app->last_video_fragment_time += frag_info->sample_info[i].duration;

                pes_info.pts_valid = true;
                pes_info.pts = (uint32_t)CALCULATE_PTS(frag_duration);
                if (video_decode_hdr == 0) {
                    pes_header_len = bmedia_pes_header_init(pes_header,
                        (sampleSize + avcc_hdr_size - nalu_len + sizeof(bmp4_nal)), &pes_info);
                } else {
                    pes_header_len = bmedia_pes_header_init(pes_header,
                        (sampleSize - nalu_len + sizeof(bmp4_nal)), &pes_info);
                }

                BKNI_Memcpy(app->pVideoHeaderBuf, &pes_header, pes_header_len);

                if (video_decode_hdr == 0) {
                    BKNI_Memcpy(app->pVideoHeaderBuf + pes_header_len, avcc_hdr, avcc_hdr_size);
                    video_decode_hdr = 1;
                } else {
                    avcc_hdr_size = 0;
                }

LOGD(("%s: sample %d nalu_len=%u pes_header_len=%u avcc_hdr_size=%u", __FUNCTION__, i, nalu_len, pes_header_len, avcc_hdr_size));

                if (pes_header_len + avcc_hdr_size > 0) {
#if USE_SECURE_VIDEO_PLAYBACK && !USE_MOCK_OEMCRYPTO
                    mp4_playback_dma_buffer(commonCryptoHandle, pVideoOutBuf, app->pVideoHeaderBuf, pes_header_len + avcc_hdr_size, true);
#else
                    BKNI_Memcpy(pVideoOutBuf, app->pVideoHeaderBuf, pes_header_len + avcc_hdr_size);
#endif
                    pVideoOutBuf += (pes_header_len + avcc_hdr_size);
                    app->videoOutBufSize += (pes_header_len + avcc_hdr_size);
                }

                decrypt_offset = nalu_len;

                if (decrypt_sample(commonCryptoHandle, sampleSize,
                    &frag_info->cursor, pSample, &numOfByteDecrypted,
                    app, pVideoOutBuf, decrypt_offset) !=0)
                {
                    LOGE(("%s Failed to decrypt sample, can't continue - %d", __FUNCTION__, __LINE__));
                    return -1;
                    break;
                }

                pVideoOutBuf += numOfByteDecrypted;
                app->videoOutBufSize += numOfByteDecrypted;
#if 1
            } else if (frag_info->trackType == BMP4_SAMPLE_ENCRYPTED_AUDIO ||
                frag_info->trackType == BMP4_SAMPLE_MP4A) {
#else
            } else if (frag_info->trackType == BMP4_SAMPLE_ENCRYPTED_AUDIO) {
#endif
                /* AAC information parsing */
                bmedia_adts_hdr hdr;
                bmedia_info_aac *info_aac = (bmedia_info_aac *)decoder_data;

                parse_esds_config(&hdr, info_aac, sampleSize);

                bmedia_pes_info_init(&pes_info, REPACK_AUDIO_PES_ID);
                frag_duration = app->last_audio_fragment_time +
                    (int32_t)frag_info->sample_info[i].composition_time_offset;
                app->last_audio_fragment_time += frag_info->sample_info[i].duration;

                pes_info.pts_valid = true;
                pes_info.pts = (uint32_t)CALCULATE_PTS(frag_duration);

                pes_header_len = bmedia_pes_header_init(pes_header,
                    (sampleSize + BMEDIA_ADTS_HEADER_SIZE), &pes_info);

                BKNI_Memcpy(app->pAudioHeaderBuf, &pes_header, pes_header_len);
                BKNI_Memcpy(app->pAudioHeaderBuf + pes_header_len, &hdr.adts, BMEDIA_ADTS_HEADER_SIZE);

#if USE_SECURE_AUDIO_PLAYBACK && !USE_MOCK_OEMCRYPTO
                mp4_playback_dma_buffer(commonCryptoHandle, pAudioOutBuf, app->pAudioHeaderBuf, pes_header_len + BMEDIA_ADTS_HEADER_SIZE, true);
#else
                BKNI_Memcpy(pAudioOutBuf, app->pAudioHeaderBuf, pes_header_len + BMEDIA_ADTS_HEADER_SIZE);
#endif

                pAudioOutBuf += pes_header_len + BMEDIA_ADTS_HEADER_SIZE;
                app->audioOutBufSize += pes_header_len + BMEDIA_ADTS_HEADER_SIZE;

                decrypt_offset = 0;

                if (decrypt_sample(commonCryptoHandle, sampleSize,
                    &frag_info->cursor, pSample, &numOfByteDecrypted,
                    app, pAudioOutBuf, decrypt_offset) !=0)
                {
                    LOGE(("%s Failed to decrypt sample, can't continue - %d", __FUNCTION__, __LINE__));
                    return -1;
                    break;
                }

                pAudioOutBuf += numOfByteDecrypted;
                app->audioOutBufSize += numOfByteDecrypted;
            } else {
                LOGW(("%s Unsupported track type %d detected", __FUNCTION__, frag_info->trackType));
                return -1;
            }

            bytes_processed += numOfByteDecrypted;
        }
    }

#if 0 // for now
    if( bytes_processed != payload_size) {
        LOGW(("%s the number of bytes %d decrypted doesn't match the actual size %d of the payload, return failure...%d",__FUNCTION__,bytes_processed,payload_size, __LINE__));
        rc = -1;
    }
#else
LOGD(("%s: dec bytes %d payload= %d",__FUNCTION__,bytes_processed,payload_size));
#endif

LOGD(("%s returning %d",__FUNCTION__,rc));
    return rc;
}

static int send_fragment_data(
        CommonCryptoHandle commonCryptoHandle,
        uint8_t *pData,
        uint32_t dataSize,
        NEXUS_PlaypumpHandle playpump,
        BKNI_EventHandle event)
{
//BSTD_UNUSED(commonCryptoHandle);
LOGD(("%s: ccHandle=%p dataSize=%u %p", __FUNCTION__, commonCryptoHandle, dataSize, pData));

    NEXUS_PlaypumpStatus playpumpStatus;
    size_t fragment_size = dataSize;
    void *playpumpBuffer;
    size_t bufferSize;
    NEXUS_Playpump_GetStatus(playpump, &playpumpStatus);
    fragment_size += 512;
    BDBG_ASSERT(fragment_size <= playpumpStatus.fifoSize);
    for(;;) {
        NEXUS_Playpump_GetBuffer(playpump, &playpumpBuffer, &bufferSize);
LOGD(("%s: bufferSize=%u", __FUNCTION__, bufferSize));
        if(bufferSize >= fragment_size) {
            break;
        }
        if(bufferSize==0) {
            BKNI_WaitForEvent(event, 100);
            continue;
        }
        if((uint8_t *)playpumpBuffer >= (uint8_t *)playpumpStatus.bufferBase +
                (playpumpStatus.fifoSize - fragment_size)) {
LOGW(("%s: skip buffer bufferSize=%u", __FUNCTION__, bufferSize));
            NEXUS_Playpump_WriteComplete(playpump, bufferSize, 0); /* skip buffer what wouldn't be big enough */
        }
    }

    NEXUS_Error rc = 0;
    if (commonCryptoHandle) {
        rc = SecureCopy(playpumpBuffer, pData, dataSize);
    } else {
        BKNI_Memcpy(playpumpBuffer, pData, dataSize);
    }

#if USE_MOCK_OEMCRYPTO || !USE_SECURE_PLAYBACK
dump100((uint8_t*)playpumpBuffer);
#endif

    rc = NEXUS_Playpump_WriteComplete(playpump, 0, dataSize);
LOGD(("%s: WriteComplete returned %d", __FUNCTION__, rc));

    return 0;
}

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void
wait_for_drain(NEXUS_PlaypumpHandle videoPlaypump, NEXUS_SimpleVideoDecoderHandle videoDecoder,
               NEXUS_PlaypumpHandle audioPlaypump, NEXUS_SimpleAudioDecoderHandle audioDecoder)
{
    NEXUS_Error rc;
    NEXUS_PlaypumpStatus playpumpStatus;

    for(;;) {
        rc=NEXUS_Playpump_GetStatus(videoPlaypump, &playpumpStatus);
        if(rc!=NEXUS_SUCCESS)
            break;

        if(playpumpStatus.fifoDepth==0) {
            break;
        }
        BKNI_Sleep(100);
    }
    for(;;) {
        rc=NEXUS_Playpump_GetStatus(audioPlaypump, &playpumpStatus);
        if(rc!=NEXUS_SUCCESS)
            break;

        if(playpumpStatus.fifoDepth==0)
            break;

        BKNI_Sleep(100);
    }

    if(videoDecoder) {
        for(;;) {
            NEXUS_VideoDecoderStatus decoderStatus;
            rc=NEXUS_SimpleVideoDecoder_GetStatus(videoDecoder, &decoderStatus);
            if(rc!=NEXUS_SUCCESS)
                break;

            if(decoderStatus.queueDepth==0)
                break;

            BKNI_Sleep(100);
        }
    }
    if(audioDecoder) {
        for(;;) {
            NEXUS_AudioDecoderStatus decoderStatus;
            rc=NEXUS_SimpleAudioDecoder_GetStatus(audioDecoder, &decoderStatus);
            if(rc!=NEXUS_SUCCESS)
                break;

            if(decoderStatus.queuedFrames < 4)
                break;

            BKNI_Sleep(100);
        }
    }
    return;
}

static int complete_play_fragments(
        NEXUS_SimpleAudioDecoderHandle audioDecoder,
        NEXUS_SimpleVideoDecoderHandle videoDecoder,
        NEXUS_PlaypumpHandle videoPlaypump,
        NEXUS_PlaypumpHandle audioPlaypump,
        NEXUS_DisplayHandle display,
        NEXUS_PidChannelHandle audioPidChannel,
        NEXUS_PidChannelHandle videoPidChannel,
        NEXUS_VideoWindowHandle window,
        BKNI_EventHandle event)
{
    BSTD_UNUSED(window);
    BSTD_UNUSED(display);
    BSTD_UNUSED(event);
    wait_for_drain(videoPlaypump, videoDecoder, audioPlaypump, audioDecoder);

    if (event != NULL) BKNI_DestroyEvent(event);

    if (videoDecoder) {
        NEXUS_SimpleVideoDecoder_Stop(videoDecoder);
        NEXUS_Playpump_ClosePidChannel(videoPlaypump, videoPidChannel);
        NEXUS_Playpump_Stop(videoPlaypump);
        NEXUS_StopCallbacks(videoPlaypump);
    }
    if (audioDecoder) {
        NEXUS_SimpleAudioDecoder_Stop(audioDecoder);
        NEXUS_Playpump_ClosePidChannel(audioPlaypump, audioPidChannel);
        NEXUS_Playpump_Stop(audioPlaypump);
        NEXUS_StopCallbacks(audioPlaypump);
    }

    NEXUS_Playpump_Close(videoPlaypump);
    NEXUS_Playpump_Close(audioPlaypump);

    return 0;
}

#ifndef ANDROID
void* GetCdmHost(int host_interface_version, void* user_data)
{
    if (host_interface_version != cdm::kHostInterfaceVersion)
        return NULL;

    return user_data;
}

static int cdm_init()
{
	cdm::ContentDecryptionModule* cdm_;
    // Create the Host.
    host_ = new TestHost();

    // Initialize the CDM module before creating a CDM instance.
    INITIALIZE_CDM_MODULE();

    // Create the CDM.
    cdm_ = reinterpret_cast<cdm::ContentDecryptionModule*>(
            ::CreateCdmInstance(
            cdm::kCdmInterfaceVersion, NULL, 0, GetCdmHost, host_));

    if (cdm_ == NULL) {
        LOGE(("Failed to create CDM"));
		if (host_)
			delete host_;
        return -1;
    }

    // Tell the Host about the CDM.
    host_->SetCdmPtr(cdm_);
	LOGD(("CDM initialization is successful\n"));
	return 0;
}
#endif

void playback_mp4(NEXUS_SimpleVideoDecoderHandle videoDecoder,
                   NEXUS_SimpleAudioDecoderHandle audioDecoder,
                   char *mp4_file)
{
#if USE_SECURE_VIDEO_PLAYBACK
    uint8_t *pSecureVideoHeapBuffer = NULL;
#endif
#if USE_SECURE_AUDIO_PLAYBACK
    uint8_t *pSecureAudioHeapBuffer = NULL;
#endif
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_MemoryAllocationSettings memSettings;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_PlaypumpOpenPidChannelSettings videoPidSettings;
    NEXUS_PlaypumpOpenPidChannelSettings audioPidSettings;
    NEXUS_PlaypumpOpenSettings videoplaypumpOpenSettings;
    NEXUS_PlaypumpOpenSettings audioplaypumpOpenSettings;
    NEXUS_SimpleStcChannelSettings stcSettings;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_Error rc;
    CommonCryptoHandle commonCryptoHandle = NULL;
    CommonCryptoSettings  cmnCryptoSettings;

    NEXUS_DisplayHandle display;
    NEXUS_PidChannelHandle videoPidChannel = NULL;
    BKNI_EventHandle event = NULL;
    NEXUS_PlaypumpHandle videoPlaypump = NULL;
    NEXUS_PlaypumpHandle audioPlaypump = NULL;

    NEXUS_PidChannelHandle audioPidChannel = NULL;

    app_ctx app;
    bool moovBoxParsed = false;

//    uint8_t resp_buffer[64*1024];
    char *pCh_url = NULL;
    char *pCh_data = NULL;
//    uint8_t *pResponse = resp_buffer;
//    size_t respLen;
//    size_t respOffset;
//    size_t urlLen;
//    size_t chLen;
    mp4_parser_handle_t mp4_handle;
    bfile_io_read_t fd;

//    bmedia_probe_t probe = NULL;
//    bmedia_probe_config probe_config;
//    const bmedia_probe_stream *stream = NULL;
//    const bmedia_probe_track *track = NULL;
//    char stream_info[512];

    bool widevine_detected = false;
    struct mp4_parser_context *mp4_context;
//    cdm::ContentDecryptionModule* cdm_;
    std::string system_id;
    uint32_t key_id_len_n;
    uint32_t pssh_len, pssh_len_n;
    uint8_t *pssh_data;
    std::string key_id_len_str;
    std::string pssh_len_str, pssh_data_str;
    std::string client_auth;

    CdmAppParameterMap app_parameters;
    std::string server_url;
    std::string response;
    std::string provisioning_server_url;
    std::string cert_authority, cert, wrapped_key;

    LOGD(("%s - %d\n", __FUNCTION__, __LINE__));
    if(mp4_file == NULL ) {
        goto clean_exit;
    }

    LOGD(("MP4 file: %s\n",mp4_file));
    fflush(stdout);

    BKNI_Memset(&app, 0, sizeof( app_ctx));
    app.last_video_fragment_time = 0;
    app.last_audio_fragment_time = 0;

    app.fp_mp4 = fopen(mp4_file, "rb");
    if(app.fp_mp4 == NULL){
        LOGE(("failed to open %s", mp4_file));
        goto clean_exit;
    }

//	app.decryptor = (WvContentDecryptionModule*) host_->GetCdmPtr();

    fd = bfile_stdio_read_attach(app.fp_mp4);

#if 0
LOGD(("calling bmedia_probe_create"));
    probe = bmedia_probe_create();

LOGD(("calling bmedia_probe_default_cfg"));
    bmedia_probe_default_cfg(&probe_config);
    probe_config.file_name = mp4_file;
    probe_config.type = bstream_mpeg_type_unknown;
LOGD(("calling bmedia_probe_parse"));
    stream = bmedia_probe_parse(probe, fd, &probe_config);

    if (stream == NULL) {
LOGD(("stream is NULL"));
    } else if (stream->type == bstream_mpeg_type_mp4) {
LOGD(("MP4 type"));
    } else if (stream->type == bstream_mpeg_type_mp4_fragment) {
LOGD(("Fragmented MP4 type"));
    } else {
LOGD(("other type %d", stream->type));
    }

LOGD(("probe_id: %d", stream->probe_id));
int i;
for(i=0;i<20;i++) {
   if(probe->probes[i]==NULL) { /* create probe on demand */
       continue;
   }
LOGD(("probe[%d]: %p", i, probe->probes[i]));
    bmp4_probe_t mp4_probe = (bmp4_probe_t)probe->probes[i];
LOGD(("sample_valid=%d sample_size_valid=%d trackheader_valid=%d", mp4_probe->sample_valid, mp4_probe->sample_size_valid,mp4_probe->trackheader_valid));
}

    bmedia_stream_to_string(stream, stream_info, sizeof(stream_info));
    LOGD(("-- Media Probe: %s\n", stream_info));
    LOGD(("Duration: %u max_bitrate: %u", stream->duration, stream->max_bitrate));
    LOGD(("nprograms: %u ntracks: %u", stream->nprograms, stream->ntracks));

    for (track = BLST_SQ_FIRST(&stream->tracks); track; track = BLST_SQ_NEXT(track, link)) {
        switch (track->type) {
            case bmedia_track_type_audio:
LOGD(("Audio track %u codec: 0x%x",track->number, track->info.audio.codec));
                break;

            case bmedia_track_type_video:
LOGD(("Video track %u codec: 0x%x",track->number, track->info.video.codec));
                break;
            default:
LOGD(("Other track"));
                break;
        }

        if (stream->type == bstream_mpeg_type_mp4) {
            bmp4_probe_track *bmp4_track = (bmp4_probe_track *)track;
LOGD(("sample_count=%u duration=%llu encrypted=%d",bmp4_track->sample_count, bmp4_track->duration, bmp4_track->encrypted));

/*
    batom_t             atom_stsd = NULL;
    bmp4_sample_info    sample_info;
    MpegDashSessionState  * mpegDashSession  = playback_ip->mpegDashSessionState;
        segmentBuffer = B_PlaybackIp_MpegDashSegmentBufferAlloc(playback_ip);


    atom_stsd = batom_from_range(mpegDashSession->factory, segmentBuffer->buffer + bmp4_track->sampledescription.offset, bmp4_track->sampledescription.size, NULL, NULL);

    if (!bmp4_parse_sample_info(atom_stsd, &sample_info, BMP4_HANDLER_VISUAL)) {
        LOGE(("%s: Can't parse Sample Description (stsd) box!", __FUNCTION__));
    }
*/
        }
    }
    if (probe) bmedia_probe_destroy(probe);
#endif

    mp4_handle = mp4_parser_create(fd);
    if (!mp4_handle) {
        LOGE(("Unable to create MP4 parser context"));
        goto clean_exit;
    }
    mp4_context = (struct mp4_parser_context *)mp4_handle;

    fseek(app.fp_mp4, 0, SEEK_END);
    app.mp4_filesize = ftell(app.fp_mp4);
    fseek(app.fp_mp4, 0, SEEK_SET);

#if USE_SECURE_PLAYBACK
    CommonCrypto_GetDefaultSettings(&cmnCryptoSettings);
    commonCryptoHandle = CommonCrypto_Open(&cmnCryptoSettings);
#endif // USE_SECURE_PLAYBACK

    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
    memSettings.heap = clientConfig.heap[1]; /* heap 1 is the eFull heap for the nxclient. */

    app.pPayload = NULL;
    if (NEXUS_Memory_Allocate(BUF_SIZE, &memSettings, (void **)&app.pPayload) !=  NEXUS_SUCCESS) {
        fprintf(stderr,"NEXUS_Memory_Allocate failed");
        goto clean_up;
    }

    app.pAudioOutBuf = NULL;
#if USE_SECURE_AUDIO_PLAYBACK
#if USE_MOCK_OEMCRYPTO
    app.pAudioOutBuf = SRAI_Memory_Allocate(BUF_SIZE, SRAI_MemoryType_Shared);
#else
    app.pAudioOutBuf = SRAI_Memory_Allocate(BUF_SIZE, SRAI_MemoryType_SagePrivate);
#endif
    if (app.pAudioOutBuf == NULL) {
        fprintf(stderr,"SRAI_Memory_Allocate failed");
        goto clean_up;
    }
#else
    if (NEXUS_Memory_Allocate(BUF_SIZE, &memSettings, (void **)&app.pAudioOutBuf) !=  NEXUS_SUCCESS) {
        fprintf(stderr,"NEXUS_Memory_Allocate failed");
        goto clean_up;
    }
#endif

    app.pVideoOutBuf = NULL;
#if USE_SECURE_VIDEO_PLAYBACK
#if USE_MOCK_OEMCRYPTO
    app.pVideoOutBuf = SRAI_Memory_Allocate(BUF_SIZE, SRAI_MemoryType_Shared);
#else
    app.pVideoOutBuf = SRAI_Memory_Allocate(BUF_SIZE, SRAI_MemoryType_SagePrivate);
#endif
    if (app.pVideoOutBuf == NULL) {
        fprintf(stderr,"SRAI_Memory_Allocate failed");
        goto clean_up;
    }
#else
    if (NEXUS_Memory_Allocate(BUF_SIZE, &memSettings, (void **)&app.pVideoOutBuf) !=  NEXUS_SUCCESS) {
        fprintf(stderr,"NEXUS_Memory_Allocate failed");
        goto clean_up;
    }
#endif

    app.pAudioHeaderBuf = NULL;
    if (NEXUS_Memory_Allocate(BMEDIA_PES_HEADER_MAX_SIZE + BMP4_MAX_PPS_SPS,
        &memSettings, (void **)&app.pAudioHeaderBuf) !=  NEXUS_SUCCESS) {
        fprintf(stderr,"NEXUS_Memory_Allocate failed");
        goto clean_up;
    }

    app.pVideoHeaderBuf = NULL;
    if (NEXUS_Memory_Allocate(BMEDIA_PES_HEADER_MAX_SIZE + BMP4_MAX_PPS_SPS,
        &memSettings, (void **)&app.pVideoHeaderBuf) !=  NEXUS_SUCCESS) {
        fprintf(stderr,"NEXUS_Memory_Allocate failed");
        goto clean_up;
    }

    /* Perform parsing of the movie information */
    moovBoxParsed = mp4_parser_scan_movie_info(mp4_handle);

    if(!moovBoxParsed) {
        LOGE(("Failed to parse moov box, can't continue..."));
        goto clean_up;
    }

    LOGD(("Successfully parsed the moov box, continue...\n\n"));

    /* EXTRACT AND PLAYBACK THE MDAT */
    NEXUS_Playpump_GetDefaultOpenSettings(&videoplaypumpOpenSettings);
    videoplaypumpOpenSettings.fifoSize *= 7;
    videoplaypumpOpenSettings.numDescriptors *= 7;
#if USE_SECURE_VIDEO_PLAYBACK
    videoplaypumpOpenSettings.dataNotCpuAccessible = true;
    pSecureVideoHeapBuffer = SRAI_Memory_Allocate(videoplaypumpOpenSettings.fifoSize,
            SRAI_MemoryType_SagePrivate);
    if ( pSecureVideoHeapBuffer == NULL ) {
        LOGE((" Failed to allocate from Secure Video heap"));
        BDBG_ASSERT( false );
    }
    videoplaypumpOpenSettings.memory = NEXUS_MemoryBlock_FromAddress(pSecureVideoHeapBuffer);
#else
    videoplaypumpOpenSettings.heap = clientConfig.heap[1];
    videoplaypumpOpenSettings.boundsHeap = clientConfig.heap[1];
#endif

    videoPlaypump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &videoplaypumpOpenSettings);
    if (!videoPlaypump) {
        LOGE(("@@@ Video Playpump Open FAILED----"));
        goto clean_up;
    }
    BDBG_ASSERT(videoPlaypump != NULL);

#if USE_SECURE_AUDIO_PLAYBACK
    NEXUS_Playpump_GetDefaultOpenSettings(&audioplaypumpOpenSettings);
    audioplaypumpOpenSettings.dataNotCpuAccessible = true;
    pSecureAudioHeapBuffer = SRAI_Memory_Allocate(audioplaypumpOpenSettings.fifoSize,
            SRAI_MemoryType_SagePrivate);
    if ( pSecureAudioHeapBuffer == NULL ) {
        LOGE((" Failed to allocate from Secure Audio heap"));
        goto clean_up;
    }
    BDBG_ASSERT( pSecureAudioHeapBuffer != NULL );
    audioplaypumpOpenSettings.memory = NEXUS_MemoryBlock_FromAddress(pSecureAudioHeapBuffer);
#else
    NEXUS_Playpump_GetDefaultOpenSettings(&audioplaypumpOpenSettings);
    audioplaypumpOpenSettings.heap = clientConfig.heap[1];
    audioplaypumpOpenSettings.boundsHeap = clientConfig.heap[1];
#endif
    audioPlaypump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &audioplaypumpOpenSettings);
    if (!audioPlaypump) {
        LOGE(("@@@ Video Playpump Open FAILED----"));
        goto clean_up;
    }
    BDBG_ASSERT(audioPlaypump != NULL);

    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    NEXUS_SimpleStcChannel_GetSettings(stcChannel, &stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    rc = NEXUS_SimpleStcChannel_SetSettings(stcChannel, &stcSettings);
    if (rc) {
       LOGW(("@@@ Stc Set FAILED ---------------"));
    }

    BKNI_CreateEvent(&event);

    NEXUS_Playpump_GetSettings(videoPlaypump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    playpumpSettings.transportType = NEXUS_TransportType_eMpeg2Pes;
    NEXUS_Playpump_SetSettings(videoPlaypump, &playpumpSettings);

    NEXUS_Playpump_GetSettings(audioPlaypump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    playpumpSettings.transportType = NEXUS_TransportType_eMpeg2Pes;
    NEXUS_Playpump_SetSettings(audioPlaypump, &playpumpSettings);

    /* already connected in main */
    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);

    NEXUS_Playpump_Start(videoPlaypump);
    NEXUS_Playpump_Start(audioPlaypump);

    NEXUS_PlaypumpOpenPidChannelSettings video_pid_settings;
    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&video_pid_settings);
    video_pid_settings.pidType = NEXUS_PidType_eVideo;

    videoPidChannel = NEXUS_Playpump_OpenPidChannel(videoPlaypump, REPACK_VIDEO_PES_ID, &video_pid_settings);
#if USE_SECURE_VIDEO_PLAYBACK
    NEXUS_SetPidChannelBypassKeyslot(videoPidChannel, NEXUS_BypassKeySlot_eGR2R);
#endif
    if ( !videoPidChannel )
      LOGW(("@@@ videoPidChannel NULL"));
    else
      LOGW(("@@@ videoPidChannel OK"));

    audioPidChannel = NEXUS_Playpump_OpenPidChannel(audioPlaypump, REPACK_AUDIO_PES_ID, NULL);
#if USE_SECURE_AUDIO_PLAYBACK
    NEXUS_SetPidChannelBypassKeyslot(audioPidChannel, NEXUS_BypassKeySlot_eGR2R);
#endif

    if ( !audioPidChannel )
      LOGW(("@@@ audioPidChannel NULL"));
    else
      LOGW(("@@@ audioPidChannel OK"));

    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);

    if ( vc1_stream ) {
       LOGW(("@@@ set video audio program for vc1"));
       videoProgram.settings.codec = NEXUS_VideoCodec_eVc1;
       audioProgram.primary.codec = NEXUS_AudioCodec_eWmaPro;
    } else {
       LOGW(("@@@ set video audio program for h264"));
       videoProgram.settings.codec = NEXUS_VideoCodec_eH264;
       audioProgram.primary.codec = NEXUS_AudioCodec_eAacAdts;
    }

    videoProgram.settings.pidChannel = videoPidChannel;
    NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);

    audioProgram.primary.pidChannel = audioPidChannel;
    NEXUS_SimpleAudioDecoder_Start(audioDecoder, &audioProgram);

    if (videoProgram.settings.pidChannel) {
        LOGW(("@@@ set stc channel video"));
        NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);
    }

    if (audioProgram.primary.pidChannel) {
        LOGW(("@@@ set stc channel audio"));
        NEXUS_SimpleAudioDecoder_SetStcChannel(audioDecoder, stcChannel);
    }

    pssh_data = mp4_parser_get_pssh(mp4_handle, &pssh_len);
    if (!pssh_data) {
        LOGW(("Failed to obtain pssh data - should be clear video"));
    } else {
        LOGD(("pssh_len=%d", pssh_len));

// this crashes on android
//        system_id.assign((char*)mp4_context->mp4_mp4.psshSystemId.systemId.data);
        system_id.resize(BMP4_UUID_LENGTH);
        memcpy((void*)system_id.data(), mp4_context->mp4_mp4.psshSystemId.systemId.data, BMP4_UUID_LENGTH);

        if (system_id.compare(a2bs_hex(kWidevineSystemId)) != 0) {
            LOGE(("The input file is not Widevine"));
            LOGE(("system id from file=%s", b2a_hex(system_id).c_str()));
            goto clean_up;
        }

        LOGD(("Widevine System Id was detected"));
        widevine_detected = true;

#ifdef ANDROID
        drm_context_ = new DRM_Context();
        app.decryptor = &drm_context_->decryptor;

        CdmResponseType status = drm_context_->decryptor.OpenSession(
            kWidevineKeySystem, NULL, &drm_context_->session_id);
        LOGD(("OpenSession returned: %d", status));
        if (status == NO_ERROR)
            goto license_acquisition;
        else if (status != NEED_PROVISIONING)
            goto clean_up;
#else
        if ( cdm_init() != 0 )
            goto clean_up;
        app.decryptor = (WvContentDecryptionModule*) host_->GetCdmPtr();
#endif

        LOGD(("calling GetProvisioningRequest"));

#ifdef ANDROID
        status = drm_context_->decryptor.GetProvisioningRequest(
            kCertificateWidevine, cert_authority, &drm_context_->key_msg,
            &provisioning_server_url);
#else
        cdm::Status status = host_->GetCdmPtr()->GetProvisioningRequest(
            &host_->keyMessage.message, &provisioning_server_url);
#endif
        if (NO_ERROR != status) {
            LOGE(("GetProvisioningRequest Error: %d", status));
            goto clean_up;
        }
        LOGD(("GetProvisioningRequest: url=%s", provisioning_server_url.c_str()));

        response = GetCertRequestResponse(provisioning_server_url);

        LOGD(("calling HandleProvisioningResponse"));
#ifdef ANDROID
        status = drm_context_->decryptor.HandleProvisioningResponse(
            response, &cert, &wrapped_key);
#else
        status = host_->GetCdmPtr()->HandleProvisioningResponse(response);
#endif
        LOGD(("HandleProvisioningResponse returned status=%d", status));

#ifdef ANDROID
        LOGD(("calling CloseSession session_id: %s", drm_context_->session_id.c_str()));
        status = drm_context_->decryptor.CloseSession(drm_context_->session_id);
        LOGD(("CloseSession returned status=%d", status));
        status  = drm_context_->decryptor.OpenSession(
            kWidevineKeySystem, NULL, &drm_context_->session_id);
#endif
        // TODO: Register for events?
/*
        WVEventListener eventListener;
        bool listenerAttached =
            drm_context_->decryptor->AttachEventListener(drm_context.session_id, &eventListener);
*/

license_acquisition:
#if 1
        // Use Google Play server
        g_license_server.assign(kGpLicenseServer);
        g_client_auth.assign(kGpClientAuth);
#else
        // Use Content Protection server
        g_license_server.assign(kCpLicenseServer);
        g_client_auth.assign(kCpClientAuth);
#endif

        // The first field (key id length)
        key_id_len_str = std::string(4, (char)0);
        key_id_len_n = htonl(pssh_len + 32);
        memcpy((void*)key_id_len_str.data(), &key_id_len_n, 4);
        g_key_id.assign(key_id_len_str);
        LOGD(("KeyId: %s", b2a_hex(g_key_id).c_str()));

        // Append "pssh" + 4 zero bytes + WV system ID
        g_key_id += a2bs_hex(kWvKeyId);
        g_key_id += system_id;
        LOGD(("KeyId: %s", b2a_hex(g_key_id).c_str()));

        // Append 4 bytes with pssh size
        pssh_len_str = std::string(4, (char)0);
        pssh_len_n = htonl(pssh_len);
        memcpy((void*)pssh_len_str.data(), &pssh_len_n, 4);
        g_key_id += pssh_len_str;

        // Append actual pssh
        pssh_data_str = std::string(pssh_len, (char)0);
        memcpy((void*)pssh_data_str.data(), pssh_data, pssh_len);
        LOGD(("pssh in the file: %s", b2a_hex(pssh_data_str).c_str()));
#if 0
        g_pssh = a2bs_hex(kGpPssh);
#else
        g_pssh = pssh_data_str;
#endif
        g_key_id += g_pssh;

        LOGD(("KeyId: %s", b2a_hex(g_key_id).c_str()));

        // License acquisition for WV
        // GenerateKeyRequest
        //  TODO: need another session for audio?
#ifdef ANDROID
        drm_context_->decryptor.GenerateKeyRequest(
            drm_context_->session_id, drm_context_->key_set_id, "video/mp4",
            g_key_id, kLicenseTypeStreaming, app_parameters, NULL,
            &drm_context_->key_msg, &server_url);
#else
        status = host_->GetCdmPtr()->GenerateKeyRequest(
            NULL, 0, (const uint8_t*) g_key_id.data(), g_key_id.length());
        if (status != cdm::kSuccess)
        {
            LOGE(("GenerateKeyRequest failed."));
            goto clean_up;
        }
#endif
//        LOGD(("GenerateKeyRequest returned session_id: %s", drm_context.session_id.c_str()));

#ifndef ANDROID
        while (!host_->HasNewKeyMessage()) {
            LOGW(("%s: waiting for new key", __FUNCTION__));
            usleep(100000);
        }
#endif

        client_auth = g_client_auth;
        client_auth.append(kGpClientOfflineQueryParameters);
        response = GetKeyRequestResponse(g_license_server, client_auth);

        LOGD(("response: %s", b2a_hex(response).c_str()));
#ifdef ANDROID
        LOGD(("calling AddKey session_id=%s", drm_context_->session_id.c_str()));
        drm_context_->decryptor.AddKey(drm_context_->session_id, response,
            &drm_context_->key_set_id);
        app.session_id = &drm_context_->session_id;
#else
        LOGD(("calling AddKey session_id=%s", host_->keyMessage.session_id.c_str()));
        host_->GetCdmPtr()->AddKey(host_->keyMessage.session_id.data(),
            host_->keyMessage.session_id.length(),
            (const uint8_t*)response.data(), response.length(),
            (const uint8_t*)g_key_id.data(), g_key_id.length());
        app.session_id = &host_->keyMessage.session_id;
#endif
        LOGD(("AddKey done response: %s", b2a_hex(response).c_str()));

    }

    /* now go back to the begining and get all the moof boxes */
    fseek(app.fp_mp4, 0, SEEK_END);
    app.mp4_filesize = ftell(app.fp_mp4);
    fseek(app.fp_mp4, 0, SEEK_SET);

    video_decode_hdr = 0;

    /* Start parsing the the file to look for MOOFs and MDATs */
    while(!feof(app.fp_mp4)) {
        mp4_parse_frag_info frag_info;
        void *decoder_data;
        size_t decoder_len;

        if (!mp4_parser_scan_movie_fragment(mp4_handle, &frag_info, app.pPayload, BUF_SIZE)) {
            if (feof(app.fp_mp4)) {
                LOGW(("Reached EOF"));
                break;
            } else {
                LOGE(("Unable to parse movie fragment"));
                goto clean_up;
            }
        }
        decoder_data = mp4_parser_get_dec_data(mp4_handle, &decoder_len, frag_info.trackId);

#if 1
        if (frag_info.trackType == BMP4_SAMPLE_ENCRYPTED_VIDEO ||
            frag_info.trackType == BMP4_SAMPLE_AVC) {
#else
        if (frag_info.trackType == BMP4_SAMPLE_ENCRYPTED_VIDEO) {
#endif
#if USE_SECURE_VIDEO_PLAYBACK
            if(process_fragment(commonCryptoHandle, &app, &frag_info, (frag_info.mdat_size - BOX_HEADER_SIZE - frag_info.aux_info_size),
                    decoder_data, decoder_len) == 0) {
                send_fragment_data(commonCryptoHandle, app.pVideoOutBuf, app.videoOutBufSize,
                        videoPlaypump, event);
            }
#else
            if(process_fragment(NULL, &app, &frag_info, (frag_info.mdat_size - BOX_HEADER_SIZE - frag_info.aux_info_size),
                    decoder_data, decoder_len) == 0) {
#if DEBUG_OUTPUT_CAPTURE
                fwrite(app.pVideoOutBuf, 1, app.videoOutBufSize, fp_vid);
#endif
                send_fragment_data(NULL, app.pVideoOutBuf, app.videoOutBufSize,
                        videoPlaypump, event);
            }
#endif
#if 1
        } else if (frag_info.trackType == BMP4_SAMPLE_ENCRYPTED_AUDIO ||
            frag_info.trackType == BMP4_SAMPLE_MP4A) {
#else
        } else if (frag_info.trackType == BMP4_SAMPLE_ENCRYPTED_AUDIO) {
#endif
#if USE_SECURE_AUDIO_PLAYBACK
            if(process_fragment(commonCryptoHandle, &app, &frag_info, (frag_info.mdat_size - BOX_HEADER_SIZE - frag_info.aux_info_size),
                        decoder_data, decoder_len) == 0) {
                send_fragment_data(commonCryptoHandle, app.pAudioOutBuf, app.audioOutBufSize,
                        audioPlaypump, event);
            }
#else
            if(process_fragment(NULL, &app, &frag_info, (frag_info.mdat_size - BOX_HEADER_SIZE - frag_info.aux_info_size),
                        decoder_data, decoder_len) == 0) {
#if DEBUG_OUTPUT_CAPTURE
                fwrite(app.pAudioOutBuf, 1, app.audioOutBufSize, fp_aud);
#endif
                send_fragment_data(NULL, app.pAudioOutBuf, app.audioOutBufSize,
                        audioPlaypump, event);
            }
#endif // USE_SECURE_AUDIO_PLAYBACK
        }
    }
    complete_play_fragments(audioDecoder, videoDecoder, videoPlaypump,
            audioPlaypump, display, audioPidChannel, videoPidChannel, NULL, event);

clean_up:
    if(commonCryptoHandle) CommonCrypto_Close(commonCryptoHandle);
#if USE_SECURE_VIDEO_PLAYBACK
    if(pSecureVideoHeapBuffer) SRAI_Memory_Free(pSecureVideoHeapBuffer);
#endif
#if USE_SECURE_AUDIO_PLAYBACK
    if(pSecureAudioHeapBuffer) SRAI_Memory_Free(pSecureAudioHeapBuffer);
#endif
// TODO: WV release key?
//    if(app.decryptor.pDecrypt != NULL) DRM_Prdy_Reader_Close( &app.decryptor);
    if(app.pPayload) NEXUS_Memory_Free(app.pPayload);
#if USE_SECURE_AUDIO_PLAYBACK
    if(app.pAudioOutBuf) SRAI_Memory_Free(app.pAudioOutBuf);
#else
    if(app.pAudioOutBuf) NEXUS_Memory_Free(app.pAudioOutBuf);
#endif
#if USE_SECURE_VIDEO_PLAYBACK
    if(app.pVideoOutBuf) SRAI_Memory_Free(app.pVideoOutBuf);
#else
    if(app.pVideoOutBuf) NEXUS_Memory_Free(app.pVideoOutBuf);
#endif

    if(mp4_handle != NULL) mp4_parser_destroy(mp4_handle);

    bfile_stdio_read_detach(fd);
    if(app.fp_mp4) fclose(app.fp_mp4);

    if(pCh_url != NULL) BKNI_Free(pCh_url);
    if(pCh_data != NULL) BKNI_Free(pCh_data);

clean_exit:
    if (widevine_detected) {
        /* WV CDM specific */
#ifdef ANDROID
        LOGD(("calling CloseSession"));
        drm_context_->decryptor.CloseSession(drm_context_->session_id);
#else
        LOGW(("Widevine cleanup"));
        // cdm_ is destroyed in TestHost dtor
        if (host_) delete host_;
        LOGW(("host Destroyed"));
#endif
    }
    return;
}

int main(int argc, char* argv[])
{
#ifndef ANDROID
    wvcdm::InitLogging(argc, argv);
#ifndef DEBUG
    g_cutoff = LOG_WARN;
#endif
#endif

    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NEXUS_Error rc;

    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    NEXUS_SurfaceComposition comp;
    NEXUS_SurfaceClientHandle surfaceClient = NULL;
    NEXUS_SurfaceClientHandle videoSurfaceClient = NULL;
//    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_SimpleVideoDecoderHandle videoDecoder = NULL;
    NEXUS_SimpleAudioDecoderHandle audioDecoder = NULL;
//    BKNI_EventHandle event;

    if (argc < 2) {
        LOGE(("Usage : %s <input_file>", argv[0]));
        return 0;
    }

/*
#ifdef NEED_TO_BE_TRUSTED_APP
	char nx_key[PROPERTY_VALUE_MAX];
	FILE *key = NULL;

        sprintf(nx_key, "%s/nx_key", NEXUS_TRUSTED_DATA_PATH);
        key = fopen(nx_key, "r");

        if (key == NULL) {
           fprintf(stderr, "%s: failed to open key file \'%s\', err=%d (%s)\n", __FUNCTION__, nx_key, errno, strerror(errno));
        } else {
           memset(nx_key, 0, sizeof(nx_key));
           fread(nx_key, PROPERTY_VALUE_MAX, 1, key);
           fclose(key);
        }
#endif
*/
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "wv_playback");
    joinSettings.ignoreStandbyRequest = true;

/*
#ifdef NEED_TO_BE_TRUSTED_APP
        joinSettings.mode = NEXUS_ClientMode_eUntrusted;
        if (strlen(nx_key)) {
           if (strstr(nx_key, "trusted:") == nx_key) {
              const char *password = &nx_key[8];
              joinSettings.mode = NEXUS_ClientMode_eProtected;
              joinSettings.certificate.length = strlen(password);
              memcpy(joinSettings.certificate.data, password, joinSettings.certificate.length);
           }
        }
#endif
*/
    rc = NxClient_Join(&joinSettings);
    if (rc)
        return -1;

    NxClient_GetDefaultAllocSettings(&allocSettings);
#if 1
    allocSettings.simpleVideoDecoder = NEXUS_ANY_ID;
    allocSettings.simpleAudioDecoder = NEXUS_ANY_ID;
    allocSettings.surfaceClient = NEXUS_ANY_ID;
#else
    allocSettings.simpleVideoDecoder = 1;
    allocSettings.simpleAudioDecoder = 1;
    allocSettings.surfaceClient = 1;
#endif
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc)
        return BERR_TRACE(rc);

#if DEBUG_OUTPUT_CAPTURE
#ifdef ANDROID
    fp_vid = fopen ("/data/mediadrm/video.out", "wb");
    fp_aud = fopen ("/data/mediadrm/audio.out", "wb");
#else
    fp_vid = fopen ("./videos/video.out", "wb");
    fp_aud = fopen ("./videos/audio.out", "wb");
#endif
#endif
    LOGD(("@@@ Check Point #01"));

    if (allocResults.simpleVideoDecoder[0].id) {
        LOGD(("@@@ to acquire video decoder %d", allocResults.simpleVideoDecoder[0].id));
        videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
    }
    BDBG_ASSERT(videoDecoder);
    if (allocResults.simpleAudioDecoder.id) {
        LOGD(("@@@ to acquire audio decoder %d", allocResults.simpleAudioDecoder.id));
        audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(allocResults.simpleAudioDecoder.id);
    }
    BDBG_ASSERT(audioDecoder);
    if (allocResults.surfaceClient[0].id) {
        LOGD(("@@@ to acquire surfaceclient"));
        /* surfaceClient is the top-level graphics window in which video will fit.
        videoSurfaceClient must be "acquired" to associate the video window with surface compositor.
        Graphics do not have to be submitted to surfaceClient for video to work, but at least an
        "alpha hole" surface must be submitted to punch video through other client's graphics.
        Also, the top-level surfaceClient ID must be submitted to NxClient_ConnectSettings below. */
        surfaceClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
        videoSurfaceClient = NEXUS_SurfaceClient_AcquireVideoWindow(surfaceClient, 0);

        NxClient_GetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
        comp.zorder = ZORDER_TOP;   /* try to stay on top most */
        NxClient_SetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
    }
    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].surfaceClientId = allocResults.surfaceClient[0].id;
    connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) return BERR_TRACE(rc);

#if 1
        NxClient_AudioSettings audioSettings;
        NxClient_GetAudioSettings(&audioSettings);

        audioSettings.muted = false;
        audioSettings.volumeType = NEXUS_AudioVolumeType_eLinear;
        audioSettings.rightVolume = audioSettings.leftVolume = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
        rc = NxClient_SetAudioSettings(&audioSettings);
#endif

    gui_init( surfaceClient );

	playback_mp4(videoDecoder,
                  audioDecoder,
                  argv[1]);

#if DEBUG_OUTPUT_CAPTURE
    fclose (fp_vid);
    fclose (fp_aud);
#endif

    if (videoDecoder != NULL) {
        NEXUS_SimpleVideoDecoder_Release( videoDecoder );
    }

    if ( audioDecoder != NULL) {
        NEXUS_SimpleAudioDecoder_Release( audioDecoder );
    }

    if ( surfaceClient != NULL ) {
        NEXUS_SurfaceClient_Release( surfaceClient );
    }

    NxClient_Disconnect(connectId);
    NxClient_Free(&allocResults);
    NxClient_Uninit();

clean_exit:
    return 0;
}

static int gui_init( NEXUS_SurfaceClientHandle surfaceClient )
{
    NEXUS_Graphics2DHandle gfx;
    NEXUS_SurfaceHandle surface;

    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    int rc;

    if (!surfaceClient) return -1;

    LOGD(("@@@ gui_init surfaceclient %d", (int)surfaceClient));
    gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    rc = NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 720;
    createSettings.height = 480;
    surface = NEXUS_Surface_Create(&createSettings);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = surface;
    fillSettings.color = 0;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL); /* require to execute queue */

    rc = NEXUS_SurfaceClient_SetSurface(surfaceClient, surface);
    BDBG_ASSERT(!rc);

    return 0;
}
