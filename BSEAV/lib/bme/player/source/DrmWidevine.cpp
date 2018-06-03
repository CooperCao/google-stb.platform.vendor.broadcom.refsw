/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include <thread>
#include <algorithm>
#include <sys/time.h>
// for printing int64_t
#include <inttypes.h>
// for memcpy, memcmp
#include <cstring>
// has to use curl for posting provision request
#include <curl/curl.h>

#include "Drm.h"
#include "DrmWidevine.h"
#include "MediaDrmAdaptor.h"

static const uint8_t kWidevineSystemId[] = {
    0xED, 0xEF, 0x8B, 0xA9, 0x79, 0xD6, 0x4A, 0xCE,
    0xA3, 0xC8, 0x27, 0xDC, 0xD5, 0x1D, 0x21, 0xED};

using namespace widevine;

namespace Broadcom {
namespace Media {

TRLS_DBG_MODULE(DrmWidevine);

// url is from ce_cdm-3.1/core/src/certificate_provisioning.cpp
static const std::string kProvisioningServerUrl =
    "https://www.googleapis.com/"
    "certificateprovisioning/v1/devicecertificates/create"
    "?key=AIzaSyB-5OLKTx2iU5mko18DfdwK5611JIjbUhE";

static std::string msgBuffer;
static size_t curl_writeback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    msgBuffer.append((char*)ptr, size * nmemb);
    return size * nmemb;
}

static void sendPostRequest(std::string& request,
                            std::string& response)
{
    std::string server_url = kProvisioningServerUrl;

    response.clear();

    msgBuffer.clear();
    server_url += "&signedRequest=";
    server_url += request;
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (curl) {
        BME_DEBUG_TRACE(("[CURL]server_url: %s", server_url.c_str()));
        curl_easy_setopt(curl, CURLOPT_URL, server_url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, 0);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writeback);
#ifdef SKIP_PEER_VERIFICATION
        /*
         * If you want to connect to a site who isn't using a certificate that is
         * signed by one of the certs in the CA bundle you have, you can skip the
         * verification of the server's certificate. This makes the connection
         * A LOT LESS SECURE.
         *
         * If you have a CA cert for the server stored someplace else than in the
         * default bundle, then the CURLOPT_CAPATH option might come handy for
         * you.
         */
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
#ifdef SKIP_HOSTNAME_VERFICATION
        /*
         * If the site you're connecting to uses a different host name that what
         * they have mentioned in their server certificate's commonName (or
         * subjectAltName) fields, libcurl will refuse to connect. You can skip
         * this check, but this will make the connection less secure.
         */
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
        res = curl_easy_perform(curl);
        response = msgBuffer;
        BME_DEBUG_TRACE(("response: %s, res: %d", response.c_str(), res));
        curl_easy_cleanup(curl);
    }

    BME_DEBUG_EXIT();
}

static void swapBytes(void *data, uint8_t size)
{
    uint8_t *bytes = static_cast<uint8_t *>(data);

    uint8_t tmp;
    for (uint8_t i = 0, j = size-1; i < j; ++i, --j) {
        tmp = bytes[i];
        bytes[i] = bytes[j];
        bytes[j] = tmp;
    }
}

// string help functions copied from ce_cdm-3.2/core/src/string_conversions.cpp
static bool CharToDigit(char ch, unsigned char* digit) {
  if (ch >= '0' && ch <= '9') {
    *digit = ch - '0';
  } else {
    ch = tolower(ch);
    if ((ch >= 'a') && (ch <= 'f')) {
      *digit = ch - 'a' + 10;
    } else {
      return false;
    }
  }
  return true;
}

static std::vector<uint8_t> a2b_hex(const std::string& byte) {
  std::vector<uint8_t> array;
  unsigned int count = byte.size();
  if (count == 0 || (count % 2) != 0) {
    BME_DEBUG_ERROR(("Invalid input size %u for string %s", count, byte.c_str()));
    return array;
  }

  for (unsigned int i = 0; i < count / 2; ++i) {
    unsigned char msb = 0;  // most significant 4 bits
    unsigned char lsb = 0;  // least significant 4 bits
    if (!CharToDigit(byte[i * 2], &msb) ||
        !CharToDigit(byte[i * 2 + 1], &lsb)) {
      BME_DEBUG_ERROR(("Invalid hex value %c%c at index %d",
           byte[i * 2], byte[i * 2 + 1], i));
      return array;
    }
    array.push_back((msb << 4) | lsb);
  }
  return array;
}

static std::string a2bs_hex(const std::string& byte) {
  std::vector<uint8_t> array = a2b_hex(byte);
  return std::string(array.begin(), array.end());
}

// default certificate from BSEAV/lib/security/dif/src/wv3x_decryptor.cpp
static const std::string kDefaultCertificate = a2bs_hex(
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

static int64_t now_ms()
{
    struct timeval tv;
    tv.tv_sec = tv.tv_usec = 0;
    gettimeofday(&tv, NULL);
    int64_t ms = (tv.tv_sec * 1000LL) + (tv.tv_usec / 1000LL);
    return ms;
}

class WidevineCdmClockImpl : public Cdm::IClock
{
  public:
    static WidevineCdmClockImpl* getInstance() {
        if (!instance) {
            instance = new WidevineCdmClockImpl();
        }

        return instance;
    };
    virtual int64_t now() {
        return now_ms();
    };

  private:
    static WidevineCdmClockImpl *instance;

    WidevineCdmClockImpl() {}
};
WidevineCdmClockImpl* WidevineCdmClockImpl::instance = NULL;

typedef std::map<std::string, std::string> StorageMap;
class WidevineCdmStorageImpl : public Cdm::IStorage
{
  public:
    static WidevineCdmStorageImpl* getInstance() {
        if (!instance) {
            instance = new WidevineCdmStorageImpl();
        }

        return instance;
    };

    virtual bool read(const std::string& name, std::string* data);
    virtual bool write(const std::string& name, const std::string& data);
    virtual bool exists(const std::string& name);
    virtual bool remove(const std::string& name);
    virtual int32_t size(const std::string& name);
    virtual bool list(std::vector<std::string>* file_names);

  private:
    static WidevineCdmStorageImpl *instance;
    StorageMap _files;

    WidevineCdmStorageImpl() {}
};

WidevineCdmStorageImpl* WidevineCdmStorageImpl::instance = NULL;

bool WidevineCdmStorageImpl::read(const std::string& name, std::string* data)
{
    StorageMap::iterator it = _files.find(name);
    bool ok = it != _files.end();
    BME_DEBUG_TRACE(("cdm read: %s: %s. total record: %d",
                name.c_str(), ok ? "ok" : "fail",
                _files.size()));
    if (!ok) return false;
    *data = it->second;
    return true;
}

bool WidevineCdmStorageImpl::write(const std::string& name, const std::string& data)
{
    BME_DEBUG_TRACE(("cdm write: %s", name.c_str()));
    _files[name] = data;
    return true;
}

bool WidevineCdmStorageImpl::exists(const std::string& name)
{
    StorageMap::iterator it = _files.find(name);
    bool ok = it != _files.end();
    BME_DEBUG_TRACE(("cdm exists? %s: %s", name.c_str(), ok ? "yes" : "no"));
    return ok;
}

bool WidevineCdmStorageImpl::remove(const std::string& name)
{
    BME_DEBUG_TRACE(("cdm remove: %s", name.c_str()));
    _files.erase(name);
    return true;
}

int32_t WidevineCdmStorageImpl::size(const std::string& name)
{
    StorageMap::iterator it = _files.find(name);
    if (it == _files.end()) return -1;
    return it->second.size();
}

bool WidevineCdmStorageImpl::list(std::vector<std::string>* file_names)
{
      file_names->clear();
      for (StorageMap::iterator it = _files.begin(); it != _files.end(); it++) {
          file_names->push_back(it->first);
      }
      return true;
}

class WidevineTimer
{
 public:
    int64_t expiry_time;
    ITimerClient *client;
    void* context;

    WidevineTimer() : expiry_time(0), client(NULL), context(NULL) {}

    WidevineTimer(int64_t expiry_time, ITimerClient *client, void* context)
          : expiry_time(expiry_time), client(client), context(context) {}

    WidevineTimer(const WidevineTimer& t)
          : expiry_time(t.expiry_time), client(t.client), context(t.context) {}

    bool operator>(const WidevineTimer &rhs) const {
        return expiry_time > rhs.expiry_time;
    }
};

class WidevineCdmTimerImpl : public Cdm::ITimer
{
  public:
    static WidevineCdmTimerImpl* getInstance() {
        if (!instance) {
            instance = new WidevineCdmTimerImpl();
        }

        return instance;
    };

    virtual void setTimeout(int64_t delay_ms,
                            ITimerClient *client,
                            void* context);
    virtual void cancel(ITimerClient *client);

    void launchCheckerThread();
    void stopCheckerThread();

  private:
    static WidevineCdmTimerImpl *instance;

    std::mutex checker_mutex;
    std::thread checker_thread;
    std::vector<WidevineTimer> timers;
    unsigned int ref_count = 0;

    WidevineCdmTimerImpl() {}
    unsigned int getRefCount();
    void timerChecker();
    void clearAllTimers();
};

WidevineCdmTimerImpl* WidevineCdmTimerImpl::instance = NULL;

void WidevineCdmTimerImpl::setTimeout(
        int64_t delay_ms,
        ITimerClient *client,
        void* context)
{
    BME_DEBUG_TRACE(("cdm timer timeout after %" PRIu64 " ms", delay_ms));

    std::lock_guard<std::mutex> lock(checker_mutex);

    timers.emplace_back(now_ms() + delay_ms, client, context);
    std::sort(timers.begin(), timers.end(), std::greater<WidevineTimer>());
}

void WidevineCdmTimerImpl::cancel(ITimerClient *client)
{
    BME_DEBUG_TRACE(("cdm asks to cancel a timer"));

    std::lock_guard<std::mutex> lock(checker_mutex);

    // sequential search
    for (auto it = timers.begin(); it != timers.end(); ++it) {
        if (it->client == client) {
            timers.erase(it);
            BME_DEBUG_TRACE(("timer found and cancelled"));
            return;
        }
    }

    BME_DEBUG_TRACE(("cdm cancels a timer that cannot be found"));
}

unsigned int WidevineCdmTimerImpl::getRefCount()
{
    // return ref_count after other functions have done updating the value
    std::lock_guard<std::mutex> lock(checker_mutex);

    return ref_count;
}

// this function should run in another thread and check timers periodically
void WidevineCdmTimerImpl::timerChecker()
{
    BME_DEBUG_TRACE(("cdm timer checker thread launched"));

    std::vector<WidevineTimer> timeoutTimers;
    WidevineTimer timer;

    do {
        {
            std::lock_guard<std::mutex> lock(checker_mutex);
            // check timers
            while (!timers.empty()) {
                timer = timers.back();
                if (now_ms() < timer.expiry_time) {
                    break;
                }
                // handle timeout timers later because we hold the lock here
                timeoutTimers.push_back(timer);
                timers.pop_back();
            }
        }

        while (!timeoutTimers.empty()) {
            timer = timeoutTimers.back();
            timeoutTimers.pop_back();
            // onTimerExpired() may add timer again. cannot hold lock here.
            timer.client->onTimerExpired(timer.context);
        }

        // check timer every 200 milliseconds
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    } while (getRefCount());

    BME_DEBUG_TRACE(("cdm timer checker thread exit"));
}

void WidevineCdmTimerImpl::launchCheckerThread()
{
    // each WidevineCdmProxy instance should only call this function once and
    // it should be paired with stopCheckerThread() when deleting the instance.
    std::lock_guard<std::mutex> lock(checker_mutex);
    if (0 == ref_count) {
        // need to hold lock here otherwise checker thread will exit
        checker_thread = std::thread(
                std::bind(&WidevineCdmTimerImpl::timerChecker, instance));
    }
    ++ref_count;
}

void WidevineCdmTimerImpl::stopCheckerThread()
{
    if (0 == getRefCount()) {
        clearAllTimers();
        return;
    } else {
        std::lock_guard<std::mutex> lock(checker_mutex);
        --ref_count;
    }

    if (0 == getRefCount()) {
        // do not hold lock here. checker thread needs the lock to exit
        checker_thread.join();
        clearAllTimers();
    }
}

void WidevineCdmTimerImpl::clearAllTimers()
{
    std::lock_guard<std::mutex> lock(checker_mutex);

    timers.clear();
}

// ========== DrmWidevineCdm ==========
DrmWidevineCdm::DrmWidevineCdm()
    : pCdm(nullptr)
{
}

DrmWidevineCdm::~DrmWidevineCdm()
{
    std::lock_guard<std::mutex> lock(_cdm_lock);

    release();
}

void DrmWidevineCdm::release()
{
    // no lock here since functions that call release()
    // should have already set the lock
    if (!_sessionMap.empty()) {
        BME_DEBUG_ERROR(("releasing cdm while some sessions still exist"));
    }

    if (pCdm == nullptr) {
        return;
    }

    _sessionMap.clear();
    delete pCdm;
    pCdm = nullptr;
    WidevineCdmTimerImpl::getInstance()->stopCheckerThread();
}

bool DrmWidevineCdm::createSession(
    DrmWidevine* session,
    std::string& session_id)
{
    std::lock_guard<std::mutex> lock(_cdm_lock);

    Cdm::Status result;
    session_id.clear();
    if (pCdm == nullptr) {
        // TODO: use some variables to init client info
        Cdm::ClientInfo client_info;
        client_info.product_name = "BRCM STB";  // required
        client_info.company_name = "Broadcom";  // required
        client_info.device_name = "";
        client_info.model_name = "STB";         // required
        client_info.arch_name = "";
        client_info.build_info = "";

        bool bCdmPrivacyMode = true;

        WidevineCdmTimerImpl *cdm_timer_impl = WidevineCdmTimerImpl::getInstance();
        cdm_timer_impl->launchCheckerThread();
        // log level: kSilent,kErrors,kWarnings,kInfo,kDebug,kVerbose
        result = Cdm::initialize(Cdm::kOpaqueHandle,
                                 client_info,
                                 WidevineCdmStorageImpl::getInstance(),
                                 WidevineCdmClockImpl::getInstance(),
                                 cdm_timer_impl,
                                 Cdm::kWarnings);
        if (result != Cdm::kSuccess) {
            BME_DEBUG_ERROR(("CDM initialization error. status=0x%x", result));
            cdm_timer_impl->stopCheckerThread();
            BME_DEBUG_EXIT();
            return false;
        }
        BME_DEBUG_PRINT(("CDM library version: %s", Cdm::version()));
        // If privacy_mode is true, the CDM will use a server certificate to encrypt
        // the client identification in the license request. When privacy mode
        // is enabled and setServerCertificate() is not called, a server certificate
        // will be provisioned as part of the license request flow.
        BME_DEBUG_PRINT(("CDM privacy mode: %s", bCdmPrivacyMode ? "on" : "off"));
        pCdm = Cdm::create(this, NULL, bCdmPrivacyMode);

        if (!pCdm) {
            BME_DEBUG_ERROR(("fail to create CDM instance"));
            cdm_timer_impl->stopCheckerThread();
            BME_DEBUG_EXIT();
            return false;
        }

        BME_DEBUG_TRACE(("cdm instance 0x%x", pCdm));
    }

    result = pCdm->setServiceCertificate(kDefaultCertificate);
    if (result != Cdm::kSuccess) {
        BME_DEBUG_ERROR(("CDM setServiceCertificate() error. result=0x%x", result));
        BME_DEBUG_EXIT();
        return false;
    }
    BME_DEBUG_TRACE(("cdm service certificate set successfully"));

    result = pCdm->createSession(Cdm::kTemporary, &session_id);
    if (result != Cdm::kSuccess) {
        BME_DEBUG_ERROR(("CDM createSession error. result=0x%x", result));
        BME_DEBUG_EXIT();
        return false;
    }

    auto it = _sessionMap.find(session_id);
    if (it != _sessionMap.end()) {
        BME_DEBUG_ERROR(("cdm session exist: ", session_id.c_str()));
        BME_DEBUG_EXIT();
        return false;
    }
    BME_DEBUG_TRACE(("cdm session created. id=%s", session_id.c_str()));
    _sessionMap[session_id] = session;

    BME_DEBUG_EXIT();
    return true;
}

bool DrmWidevineCdm::closeSession(const std::string& session_id)
{
    std::lock_guard<std::mutex> lock(_cdm_lock);

    auto it = _sessionMap.find(session_id);
    if (it == _sessionMap.end()) {
        BME_DEBUG_ERROR(("cdm session does not exist: ", session_id.c_str()));
        BME_DEBUG_EXIT();
        return false;
    }
    _sessionMap.erase(session_id);

    pCdm->close(session_id);

    if (_sessionMap.empty()) {
        release();
    }
}

bool DrmWidevineCdm::generateKeyRequest(
        const std::string& session_id,
        bool bWebM,
        const std::string& initData)
{
    std::lock_guard<std::mutex> lock(_cdm_lock);
    BME_DEBUG_ENTER();

    Cdm::Status result;

    BME_DEBUG_TRACE(("session_id: %s", session_id.c_str()));
    BME_DEBUG_TRACE(("Cdm::InitDataType is %s", (bWebM) ? "kWebM" : "kCenc"));
    result = pCdm->generateRequest(
            session_id,
            (bWebM) ? Cdm::kWebM : Cdm::kCenc,
            initData);

    if (Cdm::kSuccess == result) {
        BME_DEBUG_TRACE(("Cdm::generateRequest success"));
    } else if (Cdm::kInvalidState == result) {
        BME_DEBUG_TRACE(("might be multiple GenerateKeyRequest call. ignore."));
        BME_DEBUG_EXIT();
        return true;
    } else {
        BME_DEBUG_ERROR(("Cdm::generateRequest error. result=0x%x", result));
        BME_DEBUG_EXIT();
        return false;
    }

    // if cdm needs to do device provision, pCdm->generateRequest() will call
    // onDirectIndividualizationRequest() synchronously. Only after the call,
    // cdm will update some of its variables for later update() callback.
    // Because our sending request and getting response are also synchronous,
    // we cannot call update() in onDirectIndividualizationRequest().
    // So we save the request in onDirectIndividualizationRequest() and handle
    // it after pCdm->generateRequest().
    while (!_cdm_request.empty()) {
        std::string request = _cdm_request;
        std::string response;

        _cdm_request.clear();
        sendPostRequest(request, response);
        pCdm->update(session_id, response);
    }

    BME_DEBUG_EXIT();
    return true;
}

bool DrmWidevineCdm::addKey(
        const std::string& session_id,
        const std::string& key)
{
    std::lock_guard<std::mutex> lock(_cdm_lock);

    BME_DEBUG_ENTER();
    BME_DEBUG_TRACE(("adding key for session: %s", session_id.c_str()));

    Cdm::Status result;
    result = pCdm->update(session_id, key);
    if (Cdm::kSuccess != result) {
        BME_DEBUG_ERROR(("Cdm::update (add key) error. result=0x%x", result));
        BME_DEBUG_EXIT();
        return false;
    }

    BME_DEBUG_EXIT();
    return true;
}

void DrmWidevineCdm::getAllKeyIds(
        const std::string& session_id,
        std::vector<std::string>& keyIdArray)
{
    Cdm::Status result;
    Cdm::KeyStatusMap keyStatuses;

    result = pCdm->getKeyStatuses(session_id, &keyStatuses);
    if (Cdm::kSuccess != result) {
        BME_DEBUG_ERROR(("error in getting key statuses. result=0x%x", result));
        return;
    }

    for (auto it = keyStatuses.begin(); it != keyStatuses.end(); ++it) {
        if (it->second == Cdm::kUsable) {
            keyIdArray.push_back(it->first);
        }
    }
}

void DrmWidevineCdm::onMessage(
        const std::string& session_id,
        Cdm::MessageType message_type,
        const std::string& message)
{
    // no lock here since this function is called synchronously
    // and it should be already locked.
    BME_DEBUG_ENTER();

    auto it = _sessionMap.find(session_id);
    if (it == _sessionMap.end()) {
        BME_DEBUG_ERROR(("%s() cannot find session: %s",
                    __FUNCTION__, session_id.c_str()));
        BME_DEBUG_EXIT();
        return;
    }
    BME_DEBUG_TRACE(("%s() for session: %s", __FUNCTION__, session_id.c_str()));

    it->second->onMessage(message);
    BME_DEBUG_EXIT();
}

void DrmWidevineCdm::onKeyStatusesChange(const std::string& session_id)
{
    // no lock here since this function is called synchronously
    // and it should be already locked.
    BME_DEBUG_ENTER();

    auto it = _sessionMap.find(session_id);
    if (it == _sessionMap.end()) {
        BME_DEBUG_ERROR(("%s() cannot find session: %s",
                    __FUNCTION__, session_id.c_str()));
        BME_DEBUG_EXIT();
        return;
    }

    BME_DEBUG_TRACE(("%s() for session: %s", __FUNCTION__, session_id.c_str()));

    Cdm::Status result;
    Cdm::KeyStatusMap keyStatuses;
    result = pCdm->getKeyStatuses(session_id, &keyStatuses);
    if (Cdm::kSuccess != result) {
        BME_DEBUG_ERROR(("error in getting key statuses. result=0x%x", result));
        return;
    }

    // check if there is any usable key
    bool bNoUsableKey = true;
    for (auto it = keyStatuses.begin(); it != keyStatuses.end(); ++it) {
        if (it->second == Cdm::kUsable) {
            bNoUsableKey = false;
            break;
        }
    }
    if (bNoUsableKey) {
        BME_DEBUG_TRACE(("no usable key. (it's a server certificate?)"));
        // if widevine privacy mode is enabled, this key is the server
        // certificate. so do not send KeyError here.
        return;
    }

    BME_DEBUG_TRACE(("session contains vaild key(s): %s", session_id.c_str()));

    it->second->onKeyAdded();
    BME_DEBUG_EXIT();
}

void DrmWidevineCdm::onRemoveComplete(const std::string& session_id)
{
    BME_DEBUG_TRACE(("%s() on seesion=%s", __FUNCTION__, session_id.c_str()));
}

void DrmWidevineCdm::onDeferredComplete(
        const std::string& session_id,
        Cdm::Status result)
{
    BME_DEBUG_TRACE(("%s() on seesion=%s", __FUNCTION__, session_id.c_str()));
}

void DrmWidevineCdm::onDirectIndividualizationRequest(
        const std::string& session_id,
        const std::string& request)
{
    // this is called synchronously by pCdm->generateRequest().
    // there is no lock here since it is already locked when
    // generateKeyRequest() is called.
    BME_DEBUG_TRACE(("device provision request. seesion=%s", session_id.c_str()));

    if (_cdm_request.empty()) {
        _cdm_request = request;
    } else {
        BME_DEBUG_ERROR(("previous request exists"));
    }
}

// ========== DrmWidevine ==========
const std::string DrmWidevine::s_key_system = "com.widevine.alpha";
DrmType DrmWidevine::s_drm_type = DrmType::Widevine;
DrmWidevineCdm DrmWidevine::s_cdm;

DrmType DrmWidevine::getDrmType(const std::string& keySystem)
{
    if (keySystem == s_key_system) {
        return s_drm_type;
    }

    return DrmType::NotSupported;
}

DrmWidevine::DrmWidevine(MediaDrmAdaptor *mediaDrmAdaptor)
    : Drm(mediaDrmAdaptor),
      _key_added(false)
{
}

DrmWidevine::~DrmWidevine()
{
    if (_session_id.empty()) {
        return;
    }

    BME_DEBUG_TRACE(("close session: %s", _session_id.c_str()));
    s_cdm.closeSession(_session_id);
}

bool DrmWidevine::generateKeyRequest(const std::string& initData)
{
    BME_DEBUG_ENTER();

    if (_key_added) {
        BME_DEBUG_ERROR(("failed to generate key request because key exists"));
        BME_DEBUG_EXIT();
        return false;
    }

    if (!s_cdm.createSession(this, _session_id)) {
        BME_DEBUG_ERROR(("failed to create session()"));
        BME_DEBUG_EXIT();
        return false;
    }

    if (!s_cdm.generateKeyRequest(_session_id, isWebMFormat(), initData)) {
        BME_DEBUG_ERROR(("failed to generate key request"));
        BME_DEBUG_EXIT();
        return false;
    }

    BME_DEBUG_TRACE(("session created: %s", _session_id.c_str()));
    BME_DEBUG_EXIT();
    return true;
}

bool DrmWidevine::addKey(const std::string& key)
{
    return s_cdm.addKey(_session_id, key);
}

void DrmWidevine::getAllKeyIds(std::vector<std::string>& keyIdArray)
{
    s_cdm.getAllKeyIds(_session_id, keyIdArray);
}

void* DrmWidevine::getNativeHandle(const std::string keyId)
{
    if (!_key_added) {
        return nullptr;
    }

    std::shared_ptr<WidevineKeyContext> widevineKeyContext(
            new WidevineKeyContext(s_cdm.getWidevineCdm(), _session_id, keyId));

    // keep this context valid until this instance got destroyed
    _widevine_key_context.push_back(widevineKeyContext);
    return widevineKeyContext.get();
}

void DrmWidevine::onMessage(const std::string& message)
{
    // it seems cobalt does not care about message url
    std::string emptyUrl;
    m_mediaDrmAdaptor->keyMessage(_session_id, message, emptyUrl);
}

void DrmWidevine::onKeyAdded()
{
    _key_added = true;
    m_mediaDrmAdaptor->keyAdded(_session_id);
}

}  // namespace Media
}  // namespace Broadcom
