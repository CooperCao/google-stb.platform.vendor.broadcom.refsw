// Copyright 2015 Google Inc. All Rights Reserved.
#include "cdm.h"

#include <assert.h>
#include <limits.h>  // LLONG_MAX
#include <string.h>  // memcpy

#include <vector>

// core:
#include "cdm_client_property_set.h"
#include "cdm_engine.h"
#include "clock.h"
#include "crypto_session.h"
#include "device_files.h"
#include "file_store.h"
#include "license.h"
#include "log.h"
#include "properties.h"
#include "wv_cdm_constants.h"
#include "wv_cdm_event_listener.h"

// CE:
#include "cdm_version.h"
#include "override.h"
#include "properties_ce.h"

namespace widevine {

using namespace wvcdm;

namespace {

const int64_t kPolicyTimerDurationMilliseconds = 5000;
void* const kPolicyTimerContext = NULL;

struct HostType {
  Cdm::IStorage* storage;
  Cdm::IClock* clock;
  Cdm::ITimer* timer;
  CdmEngine* provisioning_engine;
  bool initialized;

  HostType()
      : storage(NULL),
        clock(NULL),
        timer(NULL),
        provisioning_engine(NULL),
        initialized(false) {}
} host;

class PropertySet : public CdmClientPropertySet {
 public:
  PropertySet()
      : use_privacy_mode_(false) {}

  virtual ~PropertySet() {}

  virtual const std::string& security_level() const OVERRIDE {
    // Unused on CE platforms.  Used by Android to switch to L3.
    return empty_string_;
  }

  void set_use_privacy_mode(bool use) {
    use_privacy_mode_ = use;
  }

  virtual bool use_privacy_mode() const OVERRIDE {
    return use_privacy_mode_;
  }

  virtual const std::string& service_certificate() const OVERRIDE {
    return service_certificate_;
  }

  virtual void set_service_certificate(const std::string& cert) OVERRIDE {
    service_certificate_ = cert;
  }

  virtual bool is_session_sharing_enabled() const OVERRIDE {
    // Unused on CE platforms.
    return true;
  }

  virtual uint32_t session_sharing_id() const OVERRIDE {
    // Unused on CE platforms.
    return 1;
  }

  virtual void set_session_sharing_id(uint32_t id) OVERRIDE {
    // Unused on CE platforms.
    return;
  }

  virtual const std::string& app_id() const OVERRIDE {
    // Unused on CE platforms.
    return empty_string_;
  }

 private:
  bool use_privacy_mode_;
  std::string service_certificate_;

  // This is empty, but g++ 4.8 will not allow app_id() to return a string
  // literal as a const reference to std::string.
  const std::string empty_string_;
};

class CdmImpl : public Cdm,
                public WvCdmEventListener {
 public:
  CdmImpl(IEventListener* listener,
          bool privacy_mode);

  virtual ~CdmImpl();

  // Cdm:
  virtual Status setServerCertificate(const std::string& certificate) OVERRIDE;

  virtual Status createSession(SessionType session_type,
                               std::string* session_id) OVERRIDE;

  virtual Status generateRequest(const std::string& session_id,
                                 InitDataType init_data_type,
                                 const std::string& init_data) OVERRIDE;

  virtual Status load(const std::string& session_id) OVERRIDE;

  virtual Status update(const std::string& session_id,
                        const std::string& response) OVERRIDE;

  virtual Status getExpiration(const std::string& session_id,
                               int64_t* expiration) OVERRIDE;

  virtual Status getKeyStatuses(const std::string& session_id,
                                KeyStatusMap* key_statuses) OVERRIDE;

  virtual Status setAppParameter(const std::string& key,
                                 const std::string& value) OVERRIDE;

  virtual Status getAppParameter(const std::string& key,
                                 std::string* result) OVERRIDE;

  virtual Status removeAppParameter(const std::string& key) OVERRIDE;

  virtual Status clearAppParameters() OVERRIDE;

  virtual Status close(const std::string& session_id) OVERRIDE;

  virtual Status remove(const std::string& session_id) OVERRIDE;

  virtual Status decrypt(const InputBuffer& input,
                         const OutputBuffer& output) OVERRIDE;

  // ITimerClient:
  virtual void onTimerExpired(void* context) OVERRIDE;

  // WvCdmEventListener:
  virtual void OnSessionRenewalNeeded(const CdmSessionId& session_id) OVERRIDE;

  virtual void OnSessionKeysChange(const CdmSessionId& session_id,
                                   const CdmKeyStatusMap& keys_status,
                                   bool has_new_usable_key) OVERRIDE;

  virtual void OnExpirationUpdate(const CdmSessionId& session_id,
                                  int64_t new_expiry_time_seconds) OVERRIDE;

 private:
  IEventListener* listener_;
  bool policy_timer_enabled_;

  CdmEngine cdm_engine_;
  PropertySet property_set_;
  CdmAppParameterMap app_parameters_;

  struct SessionMetadata {
    bool callable;  // EME terminology: request generated or session loaded
    SessionType type;
    int64_t expiration;
    KeyStatusMap key_statuses;

    SessionMetadata()
        : callable(false),
          type((SessionType)-1),
          expiration(0) {}
  };
  std::map<std::string, SessionMetadata> sessions_;
};

CdmImpl::CdmImpl(IEventListener* listener,
                 bool privacy_mode)
    : listener_(listener),
      policy_timer_enabled_(false) {
  property_set_.set_use_privacy_mode(privacy_mode);
}

CdmImpl::~CdmImpl() {
  host.timer->cancel(this);
}

Cdm::Status CdmImpl::setServerCertificate(const std::string& certificate) {
  if (!property_set_.use_privacy_mode()) {
    LOGE("Cannot set server certificate if privacy mode is disabled.");
    return kNotSupported;
  }

  if (certificate.empty()) {
    LOGE("An empty server certificate is invalid.");
    return kTypeError;
  }

  if (CdmLicense::VerifySignedServiceCertificate(certificate) != NO_ERROR) {
    LOGE("Invalid server certificate!");
    return kTypeError;
  }

  property_set_.set_service_certificate(certificate);
  return kSuccess;
}

Cdm::Status CdmImpl::createSession(SessionType session_type,
                                   std::string* session_id) {
  if (!session_id) {
    LOGE("Missing session ID pointer.");
    return kTypeError;
  }
  // Important! The caller may pass a pre-filled string, which must be cleared
  // before being given to CdmEngine.
  session_id->clear();

  switch (session_type) {
    case kTemporary:
    case kPersistentLicense:
    case kPersistentUsageRecord:
      break;
    default:
      LOGE("Unsupported session type: %d", session_type);
      return kNotSupported;
  }

  std::string empty_origin;
  CdmResponseType result = cdm_engine_.OpenSession(
      "com.widevine.alpha", &property_set_, empty_origin, this,
      NULL, session_id);
  switch (result) {
    case NO_ERROR:
      sessions_[*session_id].type = session_type;
      return kSuccess;
    case NEED_PROVISIONING:
      LOGE("A device certificate is needed.");
      return kNeedsDeviceCertificate;
    default:
      LOGE("Unexpected error %d", result);
      return kUnexpectedError;
  }
}

Cdm::Status CdmImpl::generateRequest(const std::string& session_id,
                                     InitDataType init_data_type,
                                     const std::string& init_data) {
  if (!cdm_engine_.IsOpenSession(session_id)) {
    LOGE("No such session: %s", session_id.c_str());
    return kSessionNotFound;
  }

  if (sessions_[session_id].callable) {
    LOGE("Request already generated: %s", session_id.c_str());
    return kInvalidState;
  }

  SessionType session_type = sessions_[session_id].type;
  CdmLicenseType license_type;
  switch (session_type) {
    case kTemporary:
      license_type = kLicenseTypeTemporary;
      break;
    case kPersistentLicense:
      license_type = kLicenseTypeOffline;
      break;
    case kPersistentUsageRecord:
      license_type = kLicenseTypeStreaming;
      break;
    default:
      LOGE("Unexpected session type: %d", session_type);
      return kUnexpectedError;
  }

  std::string init_data_type_name;
  switch (init_data_type) {
    case kCenc:
      init_data_type_name = CENC_INIT_DATA_FORMAT;
      break;
    case kKeyIds:
      LOGE("Key IDs init data type is not supported.");
      return kNotSupported;
    case kWebM:
      init_data_type_name = WEBM_INIT_DATA_FORMAT;
      break;
    default:
      LOGE("Invalid init data type: %d", init_data_type);
      return kTypeError;
  }

  if (init_data.empty()) {
    LOGE("Empty init data is not valid.");
    return kTypeError;
  }

  InitializationData init_data_obj(init_data_type_name, init_data);
  if (init_data_obj.IsEmpty()) {
    // Note that InitializationData's idea of "empty" includes "failed to find
    // and parse a Widevine PSSH".  This should not happen for WebM init data,
    // which requires no parsing.
    LOGE("Failed to parse init data, may not contain a Widevine PSSH.");
    return kNotSupported;
  }

  std::string key_request;
  CdmKeyRequestType key_request_type;
  std::string ignored_server_url;

  CdmResponseType result = cdm_engine_.GenerateKeyRequest(
      session_id, session_id, init_data_obj,
      license_type, app_parameters_, &key_request, &key_request_type,
      &ignored_server_url, NULL);

  if (result != KEY_MESSAGE) {
    LOGE("Unexpected error %d", result);
    return kUnexpectedError;
  }

  sessions_[session_id].callable = true;
  assert(key_request_type == kKeyRequestTypeInitial);
  if (property_set_.use_privacy_mode() &&
      property_set_.service_certificate().empty()) {
    // We can deduce that this is a server cert request, even though CdmEgine
    // cannot currently inform us of this.
    // Previously, we used message type kIndividiualizationRequest for this.
    // The EME editor has clarified that this was a misinterpretation of the
    // spec, and that this should also be kLicenseRequest.
    LOGI("A server certificate request has been generated.");
  } else {
    LOGI("A license request has been generated.");
  }
  listener_->onMessage(session_id, kLicenseRequest, key_request);
  return kSuccess;
}

Cdm::Status CdmImpl::load(const std::string& session_id) {
  if (session_id.empty()) {
    LOGE("Empty session ID.");
    return kTypeError;
  }

  if (cdm_engine_.IsOpenSession(session_id)) {
    LOGE("Session ID already loaded.");
    return kQuotaExceeded;
  }

  std::string empty_origin;
  std::string ignored_session_id_output;
  CdmResponseType result = cdm_engine_.OpenSession(
      "com.widevine.alpha", &property_set_, empty_origin, this,
      &session_id, &ignored_session_id_output);
  switch (result) {
    case NO_ERROR:
      break;
    case NEED_PROVISIONING:
      LOGE("A device certificate is needed.");
      return kNeedsDeviceCertificate;
    default:
      LOGE("Unexpected error %d", result);
      return kUnexpectedError;
  }

  DeviceFiles f;
  if (!f.Init(kSecurityLevelUnknown)) {
    LOGE("Unexpected error, failed to init DeviceFiles");
    return kUnexpectedError;
  }

  if (!f.LicenseExists(session_id)) {
    // This might be a usage record session which needs to be loaded.
    CdmKeyMessage release_message;
    result = cdm_engine_.LoadUsageSession(session_id, &release_message);
    if (result == LOAD_USAGE_INFO_MISSING) {
      LOGE("Unable to load license: %s", session_id.c_str());
      cdm_engine_.CloseSession(session_id);
      return kSessionNotFound;
    } else if (result != KEY_MESSAGE) {
      LOGE("Unexpected error %d", result);
      cdm_engine_.CloseSession(session_id);
      return kUnexpectedError;
    }

    LOGI("A usage record release has been generated.");
    MessageType message_type = kLicenseRelease;
    listener_->onMessage(session_id, message_type, release_message);

    sessions_[session_id].type = kPersistentUsageRecord;
    sessions_[session_id].callable = true;
    return kSuccess;
  }

  result = cdm_engine_.RestoreKey(session_id, session_id);
  if (result == GET_RELEASED_LICENSE_ERROR) {
    // This was partially removed already.
    // The EME spec states that we should send a release message right away.
    InitializationData empty_initialization_data;
    CdmKeyMessage key_request;
    std::string ignored_server_url;

    CdmResponseType result = cdm_engine_.GenerateKeyRequest(
        session_id, session_id, empty_initialization_data,
        kLicenseTypeRelease, app_parameters_, &key_request, NULL,
        &ignored_server_url, NULL);
    if (result != KEY_MESSAGE) {
      LOGE("Unexpected error %d", result);
      cdm_engine_.CloseSession(session_id);
      return kUnexpectedError;
    }

    LOGI("A license release has been generated.");
    MessageType message_type = kLicenseRelease;
    listener_->onMessage(session_id, message_type, key_request);
  } else if (result != KEY_ADDED) {
    LOGE("Unexpected error %d", result);
    return kUnexpectedError;
  }

  sessions_[session_id].type = kPersistentLicense;
  sessions_[session_id].callable = true;
  return kSuccess;
}

Cdm::Status CdmImpl::update(const std::string& session_id,
                            const std::string& response) {
  if (!cdm_engine_.IsOpenSession(session_id)) {
    LOGE("No such session: %s", session_id.c_str());
    return kSessionNotFound;
  }

  if (!sessions_[session_id].callable) {
    LOGE("Request not yet generated: %s", session_id.c_str());
    return kInvalidState;
  }

  if (response.empty()) {
    LOGE("Empty response.");
    return kTypeError;
  }

  // NOTE: If the CdmSession object recognizes that this is not the first
  // AddKey(), it will internally delegate to RenewKey().
  CdmKeySetId key_set_id = session_id;
  CdmResponseType result =
      cdm_engine_.AddKey(session_id, response, &key_set_id);

  if (result == NEED_KEY) {
    // We just provisioned a server certificate.
    assert(property_set_.use_privacy_mode());

    // The cert is now available to all sessions in this CDM instance.
    // This is consistent with the behavior of the Chrome CDM.
    assert(!property_set_.service_certificate().empty());

    // The underlying session in CdmEngine has stored a copy of the original
    // init data, so we can use an empty one this time.
    InitializationData empty_init_data;
    std::string key_request;
    CdmKeyRequestType key_request_type;
    std::string ignored_server_url;

    CdmResponseType result = cdm_engine_.GenerateKeyRequest(
        session_id, session_id, empty_init_data, kLicenseTypeDeferred,
        app_parameters_, &key_request, &key_request_type,
        &ignored_server_url, NULL);

    if (result != KEY_MESSAGE) {
      LOGE("Unexpected error %d", result);
      return kUnexpectedError;
    }

    LOGI("A deferred license request has been generated.");
    assert(key_request_type == kKeyRequestTypeInitial);
    MessageType message_type = kLicenseRequest;
    listener_->onMessage(session_id, message_type, key_request);
    return kSuccess;
  } else if (result == OFFLINE_LICENSE_PROHIBITED) {
    LOGE("A temporary session cannot be used for a persistent license.");
    return kRangeError;
  } else if (result == STORAGE_PROHIBITED) {
    LOGE("A temporary session cannot be used for a persistent usage records.");
    return kRangeError;
  } else if (result != KEY_ADDED) {
    LOGE("Unexpected error %d", result);
    return kUnexpectedError;
  }

  if (!policy_timer_enabled_) {
    policy_timer_enabled_ = true;
    host.timer->setTimeout(kPolicyTimerDurationMilliseconds,
                           this,
                           kPolicyTimerContext);
  }

  if (cdm_engine_.IsReleaseSession(session_id)) {
    sessions_.erase(session_id);
    cdm_engine_.CloseSession(session_id);
    listener_->onRemoveComplete(session_id);
  }
  return kSuccess;
}

Cdm::Status CdmImpl::getExpiration(const std::string& session_id,
                                   int64_t* expiration) {
  if (!cdm_engine_.IsOpenSession(session_id)) {
    LOGE("No such session: %s", session_id.c_str());
    return kSessionNotFound;
  }

  *expiration = sessions_[session_id].expiration;
  return kSuccess;
}

Cdm::Status CdmImpl::getKeyStatuses(const std::string& session_id,
                                    KeyStatusMap* key_statuses) {
  if (!cdm_engine_.IsOpenSession(session_id)) {
    LOGE("No such session: %s", session_id.c_str());
    return kSessionNotFound;
  }

  *key_statuses = sessions_[session_id].key_statuses;
  return kSuccess;
}

Cdm::Status CdmImpl::setAppParameter(const std::string& key,
                                     const std::string& value) {
  if (key.empty()) {
    return kTypeError;
  }
  app_parameters_[key] = value;
  return kSuccess;
}

Cdm::Status CdmImpl::getAppParameter(const std::string& key,
                                     std::string* result) {
  if (NULL == result || key.empty() ||
      app_parameters_.find(key) == app_parameters_.end()) {
    return kTypeError;
  }
  *result = app_parameters_[key];
  return kSuccess;
}

Cdm::Status CdmImpl::removeAppParameter(const std::string& key) {
  if (key.empty()) {
    return kTypeError;
  }
  CdmAppParameterMap::iterator it = app_parameters_.find(key);
  if (it == app_parameters_.end()) {
    return kTypeError;
  }
  app_parameters_.erase(it);
  return kSuccess;
}

Cdm::Status CdmImpl::clearAppParameters() {
  app_parameters_.clear();
  return kSuccess;
}

Cdm::Status CdmImpl::close(const std::string& session_id) {
  if (!cdm_engine_.IsOpenSession(session_id)) {
    LOGE("No such session: %s", session_id.c_str());
    return kSessionNotFound;
  }

  CdmResponseType result = cdm_engine_.CloseSession(session_id);
  if (result != NO_ERROR) {
    LOGE("Unexpected error %d", result);
    return kUnexpectedError;
  }
  sessions_.erase(session_id);
  return kSuccess;
}

Cdm::Status CdmImpl::remove(const std::string& session_id) {
  if (!cdm_engine_.IsOpenSession(session_id)) {
    LOGE("No such session: %s", session_id.c_str());
    return kSessionNotFound;
  }

  if (!sessions_[session_id].callable) {
    LOGE("Request not yet generated: %s", session_id.c_str());
    return kInvalidState;
  }

  if (sessions_[session_id].type == kTemporary) {
    LOGE("Not a persistent session: %s", session_id.c_str());
    return kRangeError;
  }

  InitializationData empty_initialization_data;
  CdmKeyMessage key_request;
  std::string ignored_server_url;

  // Mark all keys as released ahead of generating the release request.
  // When released, cdm_engine_ will mark all keys as expired, which we will
  // ignore in this interface.
  KeyStatusMap& map = sessions_[session_id].key_statuses;
  for (KeyStatusMap::iterator it = map.begin(); it != map.end(); ++it) {
    it->second = kReleased;
  }

  CdmResponseType result = cdm_engine_.GenerateKeyRequest(
      session_id, session_id, empty_initialization_data,
      kLicenseTypeRelease, app_parameters_, &key_request, NULL,
      &ignored_server_url, NULL);
  if (result != KEY_MESSAGE) {
    LOGE("Unexpected error %d", result);
    cdm_engine_.CloseSession(session_id);
    return kUnexpectedError;
  }

  LOGI("A license release has been generated.");
  MessageType message_type = kLicenseRelease;
  listener_->onMessage(session_id, message_type, key_request);
  return kSuccess;
}

Cdm::Status CdmImpl::decrypt(const InputBuffer& input,
                             const OutputBuffer& output) {
  if (input.is_encrypted && input.iv_length != 16) {
    LOGE("The IV must be 16 bytes long.");
    return kTypeError;
  }
  if (PropertiesCE::GetSecureOutputType() == kNoSecureOutput &&
      output.is_secure) {
    LOGE("The CDM is configured without secure output support.");
    return kNotSupported;
  }

  std::string key_id(reinterpret_cast<const char *>(input.key_id),
                     input.key_id_length);
  std::vector<uint8_t> iv(input.iv, input.iv + input.iv_length);

  CdmDecryptionParameters parameters;
  parameters.is_encrypted = input.is_encrypted;
  parameters.is_secure = output.is_secure;
  parameters.key_id = &key_id;
  parameters.encrypt_buffer = input.data;
  parameters.encrypt_length = input.data_length;
  parameters.iv = &iv;
  parameters.block_offset = input.block_offset;
  parameters.decrypt_buffer = output.data;
  parameters.decrypt_buffer_length = output.data_length;
  parameters.decrypt_buffer_offset = output.data_offset;
  parameters.subsample_flags =
      (input.first_subsample ? OEMCrypto_FirstSubsample : 0) |
      (input.last_subsample ? OEMCrypto_LastSubsample : 0);
  parameters.is_video = input.is_video;

  CdmSessionId empty_session_id;
  CdmResponseType result = cdm_engine_.Decrypt(empty_session_id, parameters);
  if (result == NEED_KEY || result == SESSION_NOT_FOUND_FOR_DECRYPT) {
    LOGE("Key not available.");
    return kNoKey;
  }
  if (result == NO_ERROR) {
    return kSuccess;
  }

  LOGE("Decrypt error: %d", result);
  return kDecryptError;
}

void CdmImpl::onTimerExpired(void* context) {
  if (context == kPolicyTimerContext) {
    if (policy_timer_enabled_) {
      cdm_engine_.OnTimerEvent();
      host.timer->setTimeout(kPolicyTimerDurationMilliseconds,
                             this,
                             kPolicyTimerContext);
    }
  }
}

void CdmImpl::OnSessionRenewalNeeded(const CdmSessionId& session_id) {
  CdmKeyMessage message;
  std::string server_url;
  CdmResponseType result =
      cdm_engine_.GenerateRenewalRequest(session_id, &message, &server_url);
  if (result != KEY_MESSAGE) {
    LOGE("Unexpected error %d", result);
    return;
  }

  LOGI("A license renewal has been generated.");
  MessageType message_type = kLicenseRenewal;

  // Post the server_url before providing the message.
  // For systems that still require the server URL,
  // the listener will add the URL to its renewal request.
  listener_->onMessageUrl(session_id, server_url);
  listener_->onMessage(session_id, message_type, message);
}

void CdmImpl::OnSessionKeysChange(const CdmSessionId& session_id,
                                  const CdmKeyStatusMap& keys_status,
                                  bool has_new_usable_key) {
  KeyStatusMap& map = sessions_[session_id].key_statuses;

  CdmKeyStatusMap::const_iterator it;
  for (it = keys_status.begin(); it != keys_status.end(); ++it) {
    switch (it->second) {
      case kKeyStatusUsable:
        map[it->first] = kUsable;
        break;
      case kKeyStatusExpired: {
        KeyStatusMap::const_iterator it_old = map.find(it->first);
        if (it_old != map.end() && it_old->second == kReleased) {
          // This key has already been marked as "released".
          // Ignore the internal "expired" status.
        } else {
          map[it->first] = kExpired;
        }
        break;
      }
      case kKeyStatusOutputNotAllowed:
        map[it->first] = kOutputRestricted;
        break;
      case kKeyStatusPending:
        map[it->first] = kStatusPending;
        break;
      case kKeyStatusInternalError:
        map[it->first] = kInternalError;
        break;
      default:
        LOGE("Unrecognized key status: %d", it->second);
        map[it->first] = kInternalError;
        break;
    }
  }

  listener_->onKeyStatusesChange(session_id);
}

void CdmImpl::OnExpirationUpdate(const CdmSessionId& session_id,
                                 int64_t new_expiry_time_seconds) {
  // "Never expires" in core is LLONG_MAX.  In the CDM API, it's -1.
  if (new_expiry_time_seconds == LLONG_MAX) {
    sessions_[session_id].expiration = -1;
  } else {
    sessions_[session_id].expiration = new_expiry_time_seconds * 1000;
  }
}


bool VerifyL1() {
  CryptoSession cs;
  return cs.GetSecurityLevel() == kSecurityLevelL1;
}

}  // namespace


// static
Cdm::Status Cdm::initialize(
    SecureOutputType secure_output_type,
    const ClientInfo& client_info,
    IStorage* storage,
    IClock* clock,
    ITimer* timer,
    DeviceCertificateRequest* device_certificate_request,
    LogLevel verbosity) {
  // If you want to direct-render on L3, CryptoSession will pass that request
  // along to OEMCrypto.  But if you want to use an opaque handle on L3,
  // CryptoSession will silently ignore you and tell OEMCrypto to treat the
  // address as a clear buffer.  :-(
  //
  // So this logic mirrors that in CryptoSession.  Effectively, we are
  // detecting at init time the conditions that would prevent CryptoSession (in
  // its current form) from passing the desired buffer type constant to
  // OEMCrypto.
  // TODO: Discuss changes to CryptoSession.
  switch (secure_output_type) {
    case kOpaqueHandle:
      // This output type requires an OEMCrypto that reports L1.
      // This requirement comes from CryptoSession::SetDestinationBufferType().
      if (!VerifyL1()) {
        LOGE("Not an L1 implementation, kOpaqueHandle cannot be used!");
        return kNotSupported;
      }
      break;
    case kDirectRender:
    case kNoSecureOutput:
      break;
    default:
      LOGE("Invalid output type!");
      return kTypeError;
  }

  if (client_info.product_name.empty() ||
      client_info.company_name.empty() ||
      client_info.model_name.empty()) {
    LOGE("Client info requires product_name, company_name, model_name!");
    return kTypeError;
  }

  if (!storage || !clock || !timer) {
    LOGE("All interfaces are required!");
    return kTypeError;
  }

  if (!device_certificate_request) {
    LOGE("Device certificate request pointer is required!");
    return kTypeError;
  }

  // Our enum values match those in core/include/log.h
  g_cutoff = static_cast<LogPriority>(verbosity);

  PropertiesCE::SetSecureOutputType(secure_output_type);
  PropertiesCE::SetClientInfo(client_info);
  Properties::Init();
  host.storage = storage;
  host.clock = clock;
  host.timer = timer;

  device_certificate_request->needed = false;

  if (!host.provisioning_engine) {
    host.provisioning_engine = new CdmEngine();
  }
  bool has_cert = host.provisioning_engine->IsProvisioned(
      kSecurityLevelL1, "" /* origin */);

  if (!has_cert) {
    device_certificate_request->needed = true;
    std::string empty_authority;
    std::string empty_origin;
    std::string base_url;
    std::string signed_request;
    CdmResponseType result = host.provisioning_engine->GetProvisioningRequest(
        kCertificateWidevine, empty_authority, empty_origin,
        &signed_request, &base_url);
    if (result != NO_ERROR) {
      LOGE("Unexpected error %d", result);
      return kUnexpectedError;
    }
    device_certificate_request->url = base_url;
    device_certificate_request->url.append("&signedRequest=");
    device_certificate_request->url.append(signed_request);
  }

  host.initialized = true;
  return kSuccess;
}

// static
const char* Cdm::version() {
  return CDM_VERSION;
}

// static
Cdm* Cdm::create(IEventListener* listener,
                 bool privacy_mode) {
  if (!host.initialized) {
    LOGE("Not initialized!");
    return NULL;
  }
  if (!listener) {
    LOGE("No listener!");
    return NULL;
  }
  return new CdmImpl(listener, privacy_mode);
}

Cdm::Status Cdm::DeviceCertificateRequest::acceptReply(
    const std::string& reply) {
  if (!host.provisioning_engine) {
    LOGE("Provisioning reply received while not in a provisioning state!");
    return kTypeError;
  }

  std::string empty_origin;
  std::string ignored_cert;
  std::string ignored_wrapped_key;

  CdmResponseType result =
      host.provisioning_engine->HandleProvisioningResponse(
          empty_origin, reply, &ignored_cert, &ignored_wrapped_key);
  if (result != NO_ERROR) {
    LOGE("Unexpected error %d", result);
    return kUnexpectedError;
  }
  return kSuccess;
}

}  // namespace widevine

// Missing symbols from core:
namespace wvcdm {

using namespace widevine;

int64_t Clock::GetCurrentTime() {
  return host.clock->now() / 1000;
}

class File::Impl {
 public:
  std::string name;
  bool read_only;
  bool truncate;
};

File::File() : impl_(NULL) {}

File::~File() {
  Close();
}

bool File::Open(const std::string& file_path, int flags) {
  if (!(flags & kCreate) && !host.storage->exists(file_path)) {
    return false;
  }

  impl_ = new Impl;
  impl_->name = file_path;
  impl_->read_only = (flags & kReadOnly);
  impl_->truncate = (flags & kTruncate);
  return true;
}

ssize_t File::Read(char* buffer, size_t bytes) {
  if (!impl_) {
    return -1;
  }
  std::string data;
  if (!host.storage->read(impl_->name, &data)) {
    return -1;
  }

  size_t to_copy = std::min(bytes, data.size());
  memcpy(buffer, data.data(), to_copy);
  return to_copy;
}

ssize_t File::Write(const char* buffer, size_t bytes) {
  if (!impl_) {
    return -1;
  }
  if (!impl_->truncate) {
    LOGE("Internal error: files cannot be appended to.");
    return -1;
  }
  std::string data(buffer, bytes);
  if (!host.storage->write(impl_->name, data)) {
    return -1;
  }
  return bytes;
}

void File::Close() {
  if (impl_) {
    delete impl_;
  }
  impl_ = NULL;
}

bool File::Exists(const std::string& file_path) {
  // An empty path is the "base directory" for CE CDM's file storage.
  // Therefore, it should always be seen as existing.
  // If it ever does not exist, CdmEngine detects this as a "factory reset"
  // and wipes out all usage table data.
  return file_path.empty() || host.storage->exists(file_path);
}

bool File::Remove(const std::string& file_path) {
  return host.storage->remove(file_path);
}

bool File::Copy(const std::string& old_path, const std::string& new_path) {
  std::string data;
  bool read_ok = host.storage->read(old_path, &data);
  if (!read_ok) return false;
  return host.storage->write(new_path, data);
}

bool File::List(const std::string& path, std::vector<std::string>* files) {
  return false;
}

bool File::CreateDirectory(const std::string dir_path) {
  return true;
}

bool File::IsDirectory(const std::string& dir_path) {
  return false;
}

bool File::IsRegularFile(const std::string& file_path) {
  return host.storage->exists(file_path);
}

ssize_t File::FileSize(const std::string& file_path) {
  return host.storage->size(file_path);
}

}  // namespace wvcdm
