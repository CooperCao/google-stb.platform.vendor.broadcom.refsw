// Copyright 2013 Google Inc. All Rights Reserved.

#include "cdm_engine.h"

#include <assert.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>

#include "cdm_session.h"
#include "clock.h"
#include "device_files.h"
#include "file_store.h"
#include "license_protocol.pb.h"
#include "log.h"
#include "properties.h"
#include "scoped_ptr.h"
#include "string_conversions.h"
#include "wv_cdm_constants.h"
#include "wv_cdm_event_listener.h"

namespace {
const uint32_t kUpdateUsageInformationPeriod = 60;  // seconds
const size_t kUsageReportsPerRequest = 1;
}  // namespace

namespace wvcdm {

class UsagePropertySet : public CdmClientPropertySet {
 public:
  UsagePropertySet() {}
  virtual ~UsagePropertySet() {}
  void set_security_level(SecurityLevel security_level) {
    if (kLevel3 == security_level)
      security_level_ = QUERY_VALUE_SECURITY_LEVEL_L3;
    else
      security_level_.clear();
  }
  virtual const std::string& security_level() const { return security_level_; }
  virtual bool use_privacy_mode() const { return false; }
  virtual const std::string& service_certificate() const { return empty_; }
  virtual void set_service_certificate(const std::string&) {}
  virtual bool is_session_sharing_enabled() const { return false; }
  virtual uint32_t session_sharing_id() const { return 0; }
  virtual void set_session_sharing_id(uint32_t /* id */) {}
  virtual const std::string& app_id() const { return app_id_; }
  void set_app_id(const std::string& appId) { app_id_ = appId; }

 private:
  std::string app_id_;
  std::string security_level_;
  const std::string empty_;
};

bool CdmEngine::seeded_ = false;

CdmEngine::CdmEngine()
    : cert_provisioning_(NULL),
      cert_provisioning_requested_security_level_(kLevelDefault),
      usage_session_(NULL),
      last_usage_information_update_time_(0) {
  Properties::Init();
  if (!seeded_) {
    Clock clock;
    srand(clock.GetCurrentTime());
    seeded_ = true;
  }
}

CdmEngine::~CdmEngine() {
  AutoLock lock(session_list_lock_);
  CdmSessionMap::iterator i(sessions_.begin());
  for (; i != sessions_.end(); ++i) {
    delete i->second;
  }
  sessions_.clear();
}

CdmResponseType CdmEngine::OpenSession(const CdmKeySystem& key_system,
                                       CdmClientPropertySet* property_set,
                                       const std::string& origin,
                                       WvCdmEventListener* event_listener,
                                       const CdmSessionId* forced_session_id,
                                       CdmSessionId* session_id) {
  LOGI("CdmEngine::OpenSession");

  if (!ValidateKeySystem(key_system)) {
    LOGI("CdmEngine::OpenSession: invalid key_system = %s", key_system.c_str());
    return INVALID_KEY_SYSTEM;
  }

  if (!session_id) {
    LOGE("CdmEngine::OpenSession: no session ID destination provided");
    return INVALID_PARAMETERS_ENG_1;
  }

  if (forced_session_id) {
    if (sessions_.find(*forced_session_id) != sessions_.end()) {
      return DUPLICATE_SESSION_ID_SPECIFIED;
    }
  }

  scoped_ptr<CdmSession> new_session(
      new CdmSession(property_set, origin, event_listener, forced_session_id));
  if (new_session->session_id().empty()) {
    LOGE("CdmEngine::OpenSession: failure to generate session ID");
    return EMPTY_SESSION_ID;
  }

  CdmResponseType sts = new_session->Init();
  if (sts != NO_ERROR) {
    if (sts == NEED_PROVISIONING) {
      cert_provisioning_requested_security_level_ =
          new_session->GetRequestedSecurityLevel();
    } else {
      LOGE("CdmEngine::OpenSession: bad session init: %d", sts);
    }
    return sts;
  }
  *session_id = new_session->session_id();
  AutoLock lock(session_list_lock_);
  sessions_[*session_id] = new_session.release();
  return NO_ERROR;
}

CdmResponseType CdmEngine::OpenKeySetSession(
    const CdmKeySetId& key_set_id, CdmClientPropertySet* property_set,
    const std::string& origin, WvCdmEventListener* event_listener) {
  LOGI("CdmEngine::OpenKeySetSession");

  if (key_set_id.empty()) {
    LOGE("CdmEngine::OpenKeySetSession: invalid key set id");
    return EMPTY_KEYSET_ID_ENG_1;
  }

  CdmSessionId session_id;
  CdmResponseType sts =
      OpenSession(KEY_SYSTEM, property_set, origin, event_listener,
                  NULL /* forced_session_id */, &session_id);

  if (sts != NO_ERROR) return sts;

  release_key_sets_[key_set_id] = session_id;
  return NO_ERROR;
}

CdmResponseType CdmEngine::CloseSession(const CdmSessionId& session_id) {
  LOGI("CdmEngine::CloseSession");
  AutoLock lock(session_list_lock_);
  CdmSessionMap::iterator iter = sessions_.find(session_id);
  if (iter == sessions_.end()) {
    LOGE("CdmEngine::CloseSession: session not found = %s", session_id.c_str());
    return SESSION_NOT_FOUND_1;
  }
  CdmSession* session = iter->second;
  sessions_.erase(session_id);
  delete session;
  return NO_ERROR;
}

CdmResponseType CdmEngine::CloseKeySetSession(const CdmKeySetId& key_set_id) {
  LOGI("CdmEngine::CloseKeySetSession");

  CdmReleaseKeySetMap::iterator iter = release_key_sets_.find(key_set_id);
  if (iter == release_key_sets_.end()) {
    LOGE("CdmEngine::CloseKeySetSession: key set id not found = %s",
         key_set_id.c_str());
    return KEYSET_ID_NOT_FOUND_1;
  }

  CdmResponseType sts = CloseSession(iter->second);
  release_key_sets_.erase(iter);
  return sts;
}

bool CdmEngine::IsOpenSession(const CdmSessionId& session_id) {
  CdmSessionMap::iterator iter = sessions_.find(session_id);
  return iter != sessions_.end();
}

CdmResponseType CdmEngine::GenerateKeyRequest(
    const CdmSessionId& session_id, const CdmKeySetId& key_set_id,
    const InitializationData& init_data, const CdmLicenseType license_type,
    CdmAppParameterMap& app_parameters, CdmKeyMessage* key_request,
    CdmKeyRequestType* key_request_type, std::string* server_url,
    CdmKeySetId* key_set_id_out) {
  LOGI("CdmEngine::GenerateKeyRequest");

  CdmSessionId id = session_id;
  CdmResponseType sts;

  // NOTE: If AlwaysUseKeySetIds() is true, there is no need to consult the
  // release_key_sets_ map for release licenses.
  if (license_type == kLicenseTypeRelease &&
      !Properties::AlwaysUseKeySetIds()) {
    if (key_set_id.empty()) {
      LOGE("CdmEngine::GenerateKeyRequest: invalid key set ID");
      return EMPTY_KEYSET_ID_ENG_2;
    }

    if (!session_id.empty()) {
      LOGE("CdmEngine::GenerateKeyRequest: invalid session ID = %s",
           session_id.c_str());
      return INVALID_SESSION_ID;
    }

    CdmReleaseKeySetMap::iterator iter = release_key_sets_.find(key_set_id);
    if (iter == release_key_sets_.end()) {
      LOGE("CdmEngine::GenerateKeyRequest: key set ID not found = %s",
           key_set_id.c_str());
      return KEYSET_ID_NOT_FOUND_2;
    }

    id = iter->second;
  }

  CdmSessionMap::iterator iter = sessions_.find(id);
  if (iter == sessions_.end()) {
    LOGE("CdmEngine::GenerateKeyRequest: session_id not found = %s",
         id.c_str());
    return SESSION_NOT_FOUND_2;
  }

  if (!key_request) {
    LOGE("CdmEngine::GenerateKeyRequest: no key request destination provided");
    return INVALID_PARAMETERS_ENG_2;
  }

  key_request->clear();

  if (license_type == kLicenseTypeRelease &&
      !iter->second->license_received()) {
    sts = iter->second->RestoreOfflineSession(key_set_id, kLicenseTypeRelease);
    if (sts != KEY_ADDED) {
      LOGE("CdmEngine::GenerateKeyRequest: key release restoration failed,"
           "sts = %d", static_cast<int>(sts));
      return sts;
    }
  }

  sts = iter->second->GenerateKeyRequest(
      init_data, license_type, app_parameters, key_request, key_request_type,
      server_url, key_set_id_out);

  if (KEY_MESSAGE != sts) {
    if (sts == NEED_PROVISIONING) {
      cert_provisioning_requested_security_level_ =
          iter->second->GetRequestedSecurityLevel();
    }
    LOGE("CdmEngine::GenerateKeyRequest: key request generation failed, "
         "sts = %d", static_cast<int>(sts));
    return sts;
  }

  if (license_type == kLicenseTypeRelease) {
    OnKeyReleaseEvent(key_set_id);
  }

  return KEY_MESSAGE;
}

CdmResponseType CdmEngine::AddKey(const CdmSessionId& session_id,
                                  const CdmKeyResponse& key_data,
                                  CdmKeySetId* key_set_id) {
  LOGI("CdmEngine::AddKey");

  CdmSessionId id = session_id;
  bool license_type_release = session_id.empty();

  if (license_type_release) {
    if (!key_set_id) {
      LOGE("CdmEngine::AddKey: no key set id provided");
      return INVALID_PARAMETERS_ENG_3;
    }

    if (key_set_id->empty()) {
      LOGE("CdmEngine::AddKey: invalid key set id");
      return EMPTY_KEYSET_ID_ENG_3;
    }

    CdmReleaseKeySetMap::iterator iter = release_key_sets_.find(*key_set_id);
    if (iter == release_key_sets_.end()) {
      LOGE("CdmEngine::AddKey: key set id not found = %s", key_set_id->c_str());
      return KEYSET_ID_NOT_FOUND_3;
    }

    id = iter->second;
  }

  CdmSessionMap::iterator iter = sessions_.find(id);

  if (iter == sessions_.end()) {
    LOGE("CdmEngine::AddKey: session id not found = %s", id.c_str());
    return SESSION_NOT_FOUND_3;
  }

  if (key_data.empty()) {
    LOGE("CdmEngine::AddKey: no key_data");
    return EMPTY_KEY_DATA_1;
  }

  CdmResponseType sts = iter->second->AddKey(key_data, key_set_id);

  switch (sts) {
    case KEY_ADDED:
      break;
    case NEED_KEY:
      LOGI("CdmEngine::AddKey: service certificate loaded, no key added");
      break;
    default:
      LOGE("CdmEngine::AddKey: keys not added, result = %d", sts);
      break;
  }

  return sts;
}

CdmResponseType CdmEngine::RestoreKey(const CdmSessionId& session_id,
                                      const CdmKeySetId& key_set_id) {
  LOGI("CdmEngine::RestoreKey");

  if (key_set_id.empty()) {
    LOGI("CdmEngine::RestoreKey: invalid key set id");
    return EMPTY_KEYSET_ID_ENG_4;
  }

  CdmSessionMap::iterator iter = sessions_.find(session_id);
  if (iter == sessions_.end()) {
    LOGE("CdmEngine::RestoreKey: session_id not found = %s ",
         session_id.c_str());
    return SESSION_NOT_FOUND_4;
  }

  CdmResponseType sts =
      iter->second->RestoreOfflineSession(key_set_id, kLicenseTypeOffline);
  if (sts == NEED_PROVISIONING) {
    cert_provisioning_requested_security_level_ =
        iter->second->GetRequestedSecurityLevel();
  }
  if (sts != KEY_ADDED && sts != GET_RELEASED_LICENSE_ERROR) {
    LOGE("CdmEngine::RestoreKey: restore offline session failed = %d", sts);
  }
  return sts;
}

CdmResponseType CdmEngine::RemoveKeys(const CdmSessionId& session_id) {
  LOGI("CdmEngine::RemoveKeys");

  CdmSessionMap::iterator iter = sessions_.find(session_id);
  if (iter == sessions_.end()) {
    LOGE("CdmEngine::RemoveKeys: session_id not found = %s",
         session_id.c_str());
    return SESSION_NOT_FOUND_5;
  }

  iter->second->ReleaseCrypto();
  return NO_ERROR;
}

CdmResponseType CdmEngine::GenerateRenewalRequest(
    const CdmSessionId& session_id, CdmKeyMessage* key_request,
    std::string* server_url) {
  LOGI("CdmEngine::GenerateRenewalRequest");

  CdmSessionMap::iterator iter = sessions_.find(session_id);
  if (iter == sessions_.end()) {
    LOGE("CdmEngine::GenerateRenewalRequest: session_id not found = %s",
         session_id.c_str());
    return SESSION_NOT_FOUND_6;
  }

  if (!key_request) {
    LOGE("CdmEngine::GenerateRenewalRequest: no key request destination");
    return INVALID_PARAMETERS_ENG_4;
  }

  key_request->clear();

  CdmResponseType sts =
      iter->second->GenerateRenewalRequest(key_request, server_url);

  if (KEY_MESSAGE != sts) {
    LOGE("CdmEngine::GenerateRenewalRequest: key request gen. failed, sts=%d",
         sts);
    return sts;
  }

  return KEY_MESSAGE;
}

CdmResponseType CdmEngine::RenewKey(const CdmSessionId& session_id,
                                    const CdmKeyResponse& key_data) {
  LOGI("CdmEngine::RenewKey");

  CdmSessionMap::iterator iter = sessions_.find(session_id);
  if (iter == sessions_.end()) {
    LOGE("CdmEngine::RenewKey: session_id not found = %s", session_id.c_str());
    return SESSION_NOT_FOUND_7;
  }

  if (key_data.empty()) {
    LOGE("CdmEngine::RenewKey: no key_data");
    return EMPTY_KEY_DATA_2;
  }

  CdmResponseType sts = iter->second->RenewKey(key_data);
  if (KEY_ADDED != sts) {
    LOGE("CdmEngine::RenewKey: keys not added, sts=%d", static_cast<int>(sts));
    return sts;
  }

  return KEY_ADDED;
}

CdmResponseType CdmEngine::QueryStatus(SecurityLevel security_level,
                                       const std::string& key,
                                       std::string* value) {
  LOGI("CdmEngine::QueryStatus");
  CryptoSession crypto_session;
  if (security_level == kLevel3) {
    CdmResponseType status = crypto_session.Open(kLevel3);
    if (NO_ERROR != status) return INVALID_QUERY_STATUS;
  }

  if (key == QUERY_KEY_SECURITY_LEVEL) {
    CdmSecurityLevel security_level = crypto_session.GetSecurityLevel();
    switch (security_level) {
      case kSecurityLevelL1:
        *value = QUERY_VALUE_SECURITY_LEVEL_L1;
        break;
      case kSecurityLevelL2:
        *value = QUERY_VALUE_SECURITY_LEVEL_L2;
        break;
      case kSecurityLevelL3:
        *value = QUERY_VALUE_SECURITY_LEVEL_L3;
        break;
      case kSecurityLevelUninitialized:
      case kSecurityLevelUnknown:
        *value = QUERY_VALUE_SECURITY_LEVEL_UNKNOWN;
        break;
      default:
        LOGW("CdmEngine::QueryStatus: Unknown security level: %d",
             security_level);
        return UNKNOWN_ERROR;
    }
  } else if (key == QUERY_KEY_DEVICE_ID) {
    std::string deviceId;
    if (!crypto_session.GetDeviceUniqueId(&deviceId)) {
      LOGW("CdmEngine::QueryStatus: GetDeviceUniqueId failed");
      return UNKNOWN_ERROR;
    }

    *value = deviceId;
  } else if (key == QUERY_KEY_SYSTEM_ID) {
    uint32_t system_id;
    if (!crypto_session.GetSystemId(&system_id)) {
      LOGW("CdmEngine::QueryStatus: GetSystemId failed");
      return UNKNOWN_ERROR;
    }

    std::ostringstream system_id_stream;
    system_id_stream << system_id;
    *value = system_id_stream.str();
  } else if (key == QUERY_KEY_PROVISIONING_ID) {
    std::string provisioning_id;
    if (!crypto_session.GetProvisioningId(&provisioning_id)) {
      LOGW("CdmEngine::QueryStatus: GetProvisioningId failed");
      return UNKNOWN_ERROR;
    }

    *value = provisioning_id;
  } else if (key == QUERY_KEY_CURRENT_HDCP_LEVEL ||
             key == QUERY_KEY_MAX_HDCP_LEVEL) {
    CryptoSession::HdcpCapability current_hdcp;
    CryptoSession::HdcpCapability max_hdcp;
    if (!crypto_session.GetHdcpCapabilities(&current_hdcp, &max_hdcp)) {
      LOGW("CdmEngine::QueryStatus: GetHdcpCapabilities failed");
      return UNKNOWN_ERROR;
    }

    *value = MapHdcpVersion(key == QUERY_KEY_CURRENT_HDCP_LEVEL ? current_hdcp
                                                                : max_hdcp);
  } else if (key == QUERY_KEY_USAGE_SUPPORT) {
    bool supports_usage_reporting;
    if (!crypto_session.UsageInformationSupport(&supports_usage_reporting)) {
      LOGW("CdmEngine::QueryStatus: UsageInformationSupport failed");
      return UNKNOWN_ERROR;
    }

    *value = supports_usage_reporting ? QUERY_VALUE_TRUE : QUERY_VALUE_FALSE;
  } else if (key == QUERY_KEY_NUMBER_OF_OPEN_SESSIONS) {
    size_t number_of_open_sessions;
    if (!crypto_session.GetNumberOfOpenSessions(&number_of_open_sessions)) {
      LOGW("CdmEngine::QueryStatus: GetNumberOfOpenSessions failed");
      return UNKNOWN_ERROR;
    }

    std::ostringstream open_sessions_stream;
    open_sessions_stream << number_of_open_sessions;
    *value = open_sessions_stream.str();
  } else if (key == QUERY_KEY_MAX_NUMBER_OF_SESSIONS) {
    size_t maximum_number_of_sessions;
    if (!crypto_session.GetMaxNumberOfSessions(&maximum_number_of_sessions)) {
      LOGW("CdmEngine::QueryStatus: GetMaxNumberOfOpenSessions failed");
      return UNKNOWN_ERROR;
    }

    std::ostringstream max_sessions_stream;
    max_sessions_stream << maximum_number_of_sessions;
    *value = max_sessions_stream.str();
  } else if (key == QUERY_KEY_OEMCRYPTO_API_VERSION) {
    uint32_t api_version;
    if (!crypto_session.GetApiVersion(&api_version)) {
      LOGW("CdmEngine::QueryStatus: GetApiVersion failed");
      return UNKNOWN_ERROR;
    }

    std::ostringstream api_version_stream;
    api_version_stream << api_version;
    *value = api_version_stream.str();
  } else {
    LOGW("CdmEngine::QueryStatus: Unknown status requested, key = %s",
         key.c_str());
    return INVALID_QUERY_KEY;
  }

  return NO_ERROR;
}

CdmResponseType CdmEngine::QuerySessionStatus(const CdmSessionId& session_id,
                                              CdmQueryMap* key_info) {
  LOGI("CdmEngine::QuerySessionStatus");
  CdmSessionMap::iterator iter = sessions_.find(session_id);
  if (iter == sessions_.end()) {
    LOGE("CdmEngine::QuerySessionStatus: session_id not found = %s",
         session_id.c_str());
    return SESSION_NOT_FOUND_8;
  }
  return iter->second->QueryStatus(key_info);
}

bool CdmEngine::IsReleaseSession(const CdmSessionId& session_id) {
  LOGI("CdmEngine::IsReleaseSession");
  CdmSessionMap::iterator iter = sessions_.find(session_id);
  if (iter == sessions_.end()) {
    LOGE("CdmEngine::IsReleaseSession: session_id not found = %s",
         session_id.c_str());
    return false;
  }
  return iter->second->is_release();
}

bool CdmEngine::IsOfflineSession(const CdmSessionId& session_id) {
  LOGI("CdmEngine::IsOfflineSession");
  CdmSessionMap::iterator iter = sessions_.find(session_id);
  if (iter == sessions_.end()) {
    LOGE("CdmEngine::IsOfflineSession: session_id not found = %s",
         session_id.c_str());
    return false;
  }
  return iter->second->is_offline();
}

CdmResponseType CdmEngine::QueryKeyStatus(const CdmSessionId& session_id,
                                          CdmQueryMap* key_info) {
  LOGI("CdmEngine::QueryKeyStatus");
  CdmSessionMap::iterator iter = sessions_.find(session_id);
  if (iter == sessions_.end()) {
    LOGE("CdmEngine::QueryKeyStatus: session_id not found = %s",
         session_id.c_str());
    return SESSION_NOT_FOUND_9;
  }
  return iter->second->QueryKeyStatus(key_info);
}

CdmResponseType CdmEngine::QueryKeyControlInfo(const CdmSessionId& session_id,
                                               CdmQueryMap* key_info) {
  LOGI("CdmEngine::QueryKeyControlInfo");
  CdmSessionMap::iterator iter = sessions_.find(session_id);
  if (iter == sessions_.end()) {
    LOGE("CdmEngine::QueryKeyControlInfo: session_id not found = %s",
         session_id.c_str());
    return SESSION_NOT_FOUND_10;
  }
  return iter->second->QueryKeyControlInfo(key_info);
}

/*
 * Composes a device provisioning request and output the request in JSON format
 * in *request. It also returns the default url for the provisioning server
 * in *default_url.
 *
 * Returns NO_ERROR for success and CdmResponseType error code if fails.
 */
CdmResponseType CdmEngine::GetProvisioningRequest(
    CdmCertificateType cert_type, const std::string& cert_authority,
    const std::string& origin, CdmProvisioningRequest* request,
    std::string* default_url) {
  if (!request) {
    LOGE("CdmEngine::GetProvisioningRequest: invalid output parameters");
    return INVALID_PROVISIONING_REQUEST_PARAM_1;
  }
  if (!default_url) {
    LOGE("CdmEngine::GetProvisioningRequest: invalid output parameters");
    return INVALID_PROVISIONING_REQUEST_PARAM_2;
  }

  DeleteAllUsageReportsUponFactoryReset();

  if (NULL == cert_provisioning_.get()) {
    cert_provisioning_.reset(new CertificateProvisioning());
  }
  CdmResponseType ret = cert_provisioning_->GetProvisioningRequest(
      cert_provisioning_requested_security_level_, cert_type, cert_authority,
      origin, request, default_url);
  if (ret != NO_ERROR) {
    cert_provisioning_.reset(NULL);  // Release resources.
  }
  return ret;
}

/*
 * The response message consists of a device certificate and the device RSA key.
 * The device RSA key is stored in the T.E.E. The device certificate is stored
 * in the device.
 *
 * Returns NO_ERROR for success and  CdmResponseType error code if fails.
 */
CdmResponseType CdmEngine::HandleProvisioningResponse(
    const std::string& origin, const CdmProvisioningResponse& response,
    std::string* cert, std::string* wrapped_key) {
  if (response.empty()) {
    LOGE("CdmEngine::HandleProvisioningResponse: Empty provisioning response.");
    cert_provisioning_.reset(NULL);
    return EMPTY_PROVISIONING_RESPONSE;
  }
  if (NULL == cert) {
    LOGE(
        "CdmEngine::HandleProvisioningResponse: invalid certificate "
        "destination");
    cert_provisioning_.reset(NULL);
    return INVALID_PROVISIONING_PARAMETERS_1;
  }
  if (NULL == wrapped_key) {
    LOGE("CdmEngine::HandleProvisioningResponse: invalid wrapped key "
         "destination");
    cert_provisioning_.reset(NULL);
    return INVALID_PROVISIONING_PARAMETERS_2;
  }
  if (NULL == cert_provisioning_.get()) {
    // Certificate provisioning object has been released. Check if a concurrent
    // provisioning attempt has succeeded before declaring failure.
    CryptoSession crypto_session;
    CdmResponseType status =
        crypto_session.Open(cert_provisioning_requested_security_level_);
    if (NO_ERROR != status) {
      LOGE(
          "CdmEngine::HandleProvisioningResponse: provisioning object "
          "missing and crypto session open failed.");
      return EMPTY_PROVISIONING_CERTIFICATE_2;
    }
    CdmSecurityLevel security_level = crypto_session.GetSecurityLevel();
    if (!IsProvisioned(security_level, origin)) {
      LOGE(
          "CdmEngine::HandleProvisioningResponse: provisioning object "
          "missing.");
      return EMPTY_PROVISIONING_CERTIFICATE_1;
    }
    return NO_ERROR;
  }

  CdmResponseType ret = cert_provisioning_->HandleProvisioningResponse(
      origin, response, cert, wrapped_key);
  // Release resources only on success. It is possible that a provisioning
  // attempt was made after this one was requested but before the response was
  // received, which will cause this attempt to fail. Not releasing will
  // allow for the possibility that the later attempt succeeds.
  if (NO_ERROR == ret) cert_provisioning_.reset(NULL);
  return ret;
}

bool CdmEngine::IsProvisioned(CdmSecurityLevel security_level,
                              const std::string& origin) {
  DeviceFiles handle;
  if (!handle.Init(security_level)) {
    LOGE("CdmEngine::IsProvisioned: unable to initialize device files");
    return false;
  }
  return handle.HasCertificate(origin);
}

CdmResponseType CdmEngine::Unprovision(CdmSecurityLevel security_level,
                                       const std::string& origin) {
  DeviceFiles handle;
  if (!handle.Init(security_level)) {
    LOGE("CdmEngine::Unprovision: unable to initialize device files");
    return UNPROVISION_ERROR_1;
  }

  if (origin != EMPTY_ORIGIN) {
    if (!handle.RemoveCertificate(origin)) {
      LOGE("CdmEngine::Unprovision: unable to delete certificate for origin %s",
           origin.c_str());
      return UNPROVISION_ERROR_2;
    }
    return NO_ERROR;
  } else {
    if (!handle.DeleteAllFiles()) {
      LOGE("CdmEngine::Unprovision: unable to delete files");
      return UNPROVISION_ERROR_3;
    }

    scoped_ptr<CryptoSession> crypto_session(new CryptoSession());
    CdmResponseType status = crypto_session->Open(
        security_level == kSecurityLevelL3 ? kLevel3 : kLevelDefault);
    if (NO_ERROR != status) {
      LOGE("CdmEngine::Unprovision: error opening crypto session: %d", status);
      return UNPROVISION_ERROR_4;
    }
    status = crypto_session->DeleteAllUsageReports();
    if (status != NO_ERROR) {
      LOGE("CdmEngine::Unprovision: error deleteing usage reports: %d", status);
    }
    return status;
  }
}

CdmResponseType CdmEngine::GetUsageInfo(const std::string& app_id,
                                        const CdmSecureStopId& ssid,
                                        CdmUsageInfo* usage_info) {
  if (NULL == usage_property_set_.get()) {
    usage_property_set_.reset(new UsagePropertySet());
  }
  usage_property_set_->set_security_level(kLevelDefault);
  usage_property_set_->set_app_id(app_id);
  usage_session_.reset(
      new CdmSession(usage_property_set_.get(), EMPTY_ORIGIN, NULL, NULL));
  CdmResponseType status = usage_session_->Init();
  if (NO_ERROR != status) {
    LOGE("CdmEngine::GetUsageInfo: session init error");
    return status;
  }
  DeviceFiles handle;
  if (!handle.Init(usage_session_->GetSecurityLevel())) {
    LOGE("CdmEngine::GetUsageInfo: device file init error");
    return GET_USAGE_INFO_ERROR_1;
  }

  CdmKeyMessage license_request;
  CdmKeyResponse license_response;
  if (!handle.RetrieveUsageInfo(app_id, ssid, &license_request,
                                &license_response)) {
    usage_property_set_->set_security_level(kLevel3);
    usage_property_set_->set_app_id(app_id);
    usage_session_.reset(
        new CdmSession(usage_property_set_.get(), EMPTY_ORIGIN, NULL, NULL));
    status = usage_session_->Init();
    if (NO_ERROR != status) {
      LOGE("CdmEngine::GetUsageInfo: session init error");
      return status;
    }
    if (!handle.Reset(usage_session_->GetSecurityLevel())) {
      LOGE("CdmEngine::GetUsageInfo: device file init error");
      return GET_USAGE_INFO_ERROR_2;
    }
    if (!handle.RetrieveUsageInfo(app_id, ssid, &license_request,
                                  &license_response)) {
      // No entry found for that ssid.
      return USAGE_INFO_NOT_FOUND;
    }
  }

  std::string server_url;
  usage_info->resize(1);
  status =
      usage_session_->RestoreUsageSession(license_request, license_response);
  if (KEY_ADDED != status) {
    LOGE("CdmEngine::GetUsageInfo: restore usage session error %d", status);
    usage_info->clear();
    return status;
  }

  status =
      usage_session_->GenerateReleaseRequest(&(*usage_info)[0], &server_url);

  if (KEY_MESSAGE != status) {
    LOGE("CdmEngine::GetUsageInfo: generate release request error: %d", status);
    usage_info->clear();
    return status;
  }
  return KEY_MESSAGE;
}

CdmResponseType CdmEngine::GetUsageInfo(const std::string& app_id,
                                        CdmUsageInfo* usage_info) {
  // Return a random usage report from a random security level
  SecurityLevel security_level = ((rand() % 2) == 0) ? kLevelDefault : kLevel3;
  CdmResponseType status = UNKNOWN_ERROR;
  do {
    status = GetUsageInfo(app_id, security_level, usage_info);

    if (KEY_MESSAGE == status && !usage_info->empty()) return status;
  } while (KEY_CANCELED == status);

  security_level = (kLevel3 == security_level) ? kLevelDefault : kLevel3;
  do {
    status = GetUsageInfo(app_id, security_level, usage_info);
    if (NEED_PROVISIONING == status)
      return NO_ERROR;  // Valid scenario that one of the security
                        // levels has not been provisioned
  } while (KEY_CANCELED == status);
  return status;
}

CdmResponseType CdmEngine::GetUsageInfo(const std::string& app_id,
                                        SecurityLevel requested_security_level,
                                        CdmUsageInfo* usage_info) {
  if (NULL == usage_property_set_.get()) {
    usage_property_set_.reset(new UsagePropertySet());
  }
  usage_property_set_->set_security_level(requested_security_level);
  usage_property_set_->set_app_id(app_id);

  usage_session_.reset(
      new CdmSession(usage_property_set_.get(), EMPTY_ORIGIN, NULL, NULL));

  CdmResponseType status = usage_session_->Init();
  if (NO_ERROR != status) {
    LOGE("CdmEngine::GetUsageInfo: session init error");
    return status;
  }

  DeviceFiles handle;
  if (!handle.Init(usage_session_->GetSecurityLevel())) {
    LOGE("CdmEngine::GetUsageInfo: unable to initialize device files");
    return GET_USAGE_INFO_ERROR_3;
  }

  std::vector<std::pair<CdmKeyMessage, CdmKeyResponse> > license_info;
  if (!handle.RetrieveUsageInfo(app_id, &license_info)) {
    LOGE("CdmEngine::GetUsageInfo: unable to read usage information");
    return GET_USAGE_INFO_ERROR_4;
  }

  if (0 == license_info.size()) {
    usage_info->resize(0);
    return NO_ERROR;
  }

  std::string server_url;
  usage_info->resize(kUsageReportsPerRequest);

  uint32_t index = rand() % license_info.size();
  status = usage_session_->RestoreUsageSession(license_info[index].first,
                                               license_info[index].second);
  if (KEY_ADDED != status) {
    LOGE("CdmEngine::GetUsageInfo: restore usage session (%d) error %ld", index,
         status);
    usage_info->clear();
    return status;
  }

  status =
      usage_session_->GenerateReleaseRequest(&(*usage_info)[0], &server_url);

  switch (status) {
    case KEY_MESSAGE:
      break;
    case KEY_CANCELED:                  // usage information not present in
      usage_session_->DeleteLicense();  // OEMCrypto, delete and try again
      usage_info->clear();
      break;
    default:
      LOGE("CdmEngine::GetUsageInfo: generate release request error: %d",
           status);
      usage_info->clear();
      break;
  }
  return status;
}

CdmResponseType CdmEngine::ReleaseAllUsageInfo(const std::string& app_id) {
  if (NULL == usage_property_set_.get()) {
    usage_property_set_.reset(new UsagePropertySet());
  }
  usage_property_set_->set_app_id(app_id);

  CdmResponseType status = NO_ERROR;
  for (int j = kSecurityLevelL1; j < kSecurityLevelUnknown; ++j) {
    DeviceFiles handle;
    if (handle.Init(static_cast<CdmSecurityLevel>(j))) {
      std::vector<std::string> provider_session_tokens;
      if (!handle.DeleteAllUsageInfoForApp(app_id, &provider_session_tokens)) {
        LOGE("CdmEngine::ReleaseAllUsageInfo: failed to delete L%d secure"
             "stops", j);
        status = RELEASE_ALL_USAGE_INFO_ERROR_1;
      } else {
        SecurityLevel security_level =
            static_cast<CdmSecurityLevel>(j) == kSecurityLevelL3
                ? kLevel3
                : kLevelDefault;
        usage_property_set_->set_security_level(security_level);
        usage_session_.reset(
            new CdmSession(usage_property_set_.get(),
                           EMPTY_ORIGIN, NULL, NULL));
        CdmResponseType status2 = usage_session_->
            DeleteMultipleUsageInformation(provider_session_tokens);
        if (status2 != NO_ERROR) {
          status = status2;
        }
      }
    } else {
      LOGE("CdmEngine::ReleaseAllUsageInfo: failed to initialize L%d device"
           "files", j);
      status = RELEASE_ALL_USAGE_INFO_ERROR_2;
    }
  }
  usage_session_.reset(NULL);
  return status;
}

CdmResponseType CdmEngine::ReleaseUsageInfo(
    const CdmUsageInfoReleaseMessage& message) {
  if (NULL == usage_session_.get()) {
    LOGE("CdmEngine::ReleaseUsageInfo: cdm session not initialized");
    return RELEASE_USAGE_INFO_ERROR;
  }

  CdmResponseType status = usage_session_->ReleaseKey(message);
  usage_session_.reset(NULL);
  if (NO_ERROR != status) {
    LOGE("CdmEngine::ReleaseUsageInfo: release key error: %d", status);
  }
  return status;
}

CdmResponseType CdmEngine::LoadUsageSession(const CdmKeySetId& key_set_id,
                                            CdmKeyMessage* release_message) {
  LOGI("CdmEngine::LoadUsageSession");
  // This method is currently only used by the CE CDM, in which all session IDs
  // are key set IDs.
  assert(Properties::AlwaysUseKeySetIds());

  if (key_set_id.empty()) {
    LOGE("CdmEngine::LoadUsageSession: invalid key set id");
    return EMPTY_KEYSET_ID_ENG_5;
  }

  CdmSessionMap::iterator iter = sessions_.find(key_set_id);
  if (iter == sessions_.end()) {
    LOGE("CdmEngine::LoadUsageSession: session_id not found = %s ",
         key_set_id.c_str());
    return SESSION_NOT_FOUND_11;
  }

  DeviceFiles handle;
  if (!handle.Init(iter->second->GetSecurityLevel())) {
    LOGE("CdmEngine::LoadUsageSession: unable to initialize device files");
    return LOAD_USAGE_INFO_FILE_ERROR;
  }

  std::string app_id;
  iter->second->GetApplicationId(&app_id);

  CdmKeyMessage key_message;
  CdmKeyResponse key_response;
  if (!handle.RetrieveUsageInfoByKeySetId(app_id, key_set_id, &key_message,
                                          &key_response)) {
    LOGE("CdmEngine::LoadUsageSession: unable to find usage information");
    return LOAD_USAGE_INFO_MISSING;
  }

  CdmResponseType status =
      iter->second->RestoreUsageSession(key_message, key_response);
  if (KEY_ADDED != status) {
    LOGE("CdmEngine::LoadUsageSession: usage session error %ld", status);
    return status;
  }

  std::string server_url;
  status = iter->second->GenerateReleaseRequest(release_message, &server_url);

  switch (status) {
    case KEY_MESSAGE:
      break;
    case KEY_CANCELED:                // usage information not present in
      iter->second->DeleteLicense();  // OEMCrypto, delete and try again
      break;
    default:
      LOGE("CdmEngine::LoadUsageSession: generate release request error: %d",
           status);
      break;
  }
  return status;
}

CdmResponseType CdmEngine::Decrypt(const CdmSessionId& session_id,
                                   const CdmDecryptionParameters& parameters) {
  if (parameters.key_id == NULL) {
    LOGE("CdmEngine::Decrypt: no key_id");
    return INVALID_DECRYPT_PARAMETERS_ENG_1;
  }

  if (parameters.encrypt_buffer == NULL) {
    LOGE("CdmEngine::Decrypt: no src encrypt buffer");
    return INVALID_DECRYPT_PARAMETERS_ENG_2;
  }

  if (parameters.iv == NULL) {
    LOGE("CdmEngine::Decrypt: no iv");
    return INVALID_DECRYPT_PARAMETERS_ENG_3;
  }

  if (parameters.decrypt_buffer == NULL) {
    if (!parameters.is_secure &&
        !Properties::Properties::oem_crypto_use_fifo()) {
      LOGE("CdmEngine::Decrypt: no dest decrypt buffer");
      return INVALID_DECRYPT_PARAMETERS_ENG_4;
    }
    // else we must be level 1 direct and we don't need to return a buffer.
  }

  CdmSessionMap::iterator iter;
  if (session_id.empty()) {
    // Loop through the sessions to find the session containing the key_id.
    for (iter = sessions_.begin(); iter != sessions_.end(); ++iter) {
      if (iter->second->IsKeyLoaded(*parameters.key_id)) {
        break;
      }
    }
  } else {
    iter = sessions_.find(session_id);
  }
  if (iter == sessions_.end()) {
    LOGE("CdmEngine::Decrypt: session not found: id=%s, id size=%d",
         session_id.c_str(), session_id.size());
    return SESSION_NOT_FOUND_FOR_DECRYPT;
  }

  return iter->second->Decrypt(parameters);
}

bool CdmEngine::IsKeyLoaded(const KeyId& key_id) {
  for (CdmSessionMap::iterator iter = sessions_.begin();
       iter != sessions_.end(); ++iter) {
    if (iter->second->IsKeyLoaded(key_id)) {
      return true;
    }
  }
  return false;
}

bool CdmEngine::FindSessionForKey(const KeyId& key_id,
                                  CdmSessionId* session_id) {
  if (NULL == session_id) {
    LOGE("CdmEngine::FindSessionForKey: session id not provided");
    return false;
  }

  CdmSessionMap::iterator iter = sessions_.find(*session_id);
  if (iter != sessions_.end()) {
    if (iter->second->IsKeyLoaded(key_id)) {
      return true;
    }
  }

  uint32_t session_sharing_id = Properties::GetSessionSharingId(*session_id);

  for (iter = sessions_.begin(); iter != sessions_.end(); ++iter) {
    CdmSessionId local_session_id = iter->second->session_id();
    if (Properties::GetSessionSharingId(local_session_id) ==
        session_sharing_id) {
      if (iter->second->IsKeyLoaded(key_id)) {
        *session_id = local_session_id;
        return true;
      }
    }
  }
  return false;
}

void CdmEngine::NotifyResolution(const CdmSessionId& session_id, uint32_t width,
                                 uint32_t height) {
  CdmSessionMap::iterator iter = sessions_.find(session_id);
  if (iter != sessions_.end()) {
    iter->second->NotifyResolution(width, height);
  }
}

bool CdmEngine::ValidateKeySystem(const CdmKeySystem& key_system) {
  return (key_system.find("widevine") != std::string::npos);
}

void CdmEngine::OnTimerEvent() {
  Clock clock;
  uint64_t current_time = clock.GetCurrentTime();
  bool usage_update_period_expired = false;

  if (current_time - last_usage_information_update_time_ >
      kUpdateUsageInformationPeriod) {
    usage_update_period_expired = true;
    last_usage_information_update_time_ = current_time;
  }

  bool is_initial_usage_update = false;
  bool is_usage_update_needed = false;

  AutoLock lock(session_list_lock_);
  for (CdmSessionMap::iterator iter = sessions_.begin();
       iter != sessions_.end(); ++iter) {
    is_initial_usage_update =
        is_initial_usage_update || iter->second->is_initial_usage_update();
    is_usage_update_needed =
        is_usage_update_needed || iter->second->is_usage_update_needed();

    iter->second->OnTimerEvent(usage_update_period_expired);
  }

  if (is_usage_update_needed &&
      (usage_update_period_expired || is_initial_usage_update)) {
    bool has_usage_been_updated = false;
    for (CdmSessionMap::iterator iter = sessions_.begin();
         iter != sessions_.end(); ++iter) {
      iter->second->reset_usage_flags();
      if (!has_usage_been_updated) {
        // usage is updated for all sessions so this needs to be
        // called only once per update usage information period
        CdmResponseType status = iter->second->UpdateUsageInformation();
        if (NO_ERROR != status) {
          LOGW("Update usage information failed: %d", status);
        } else {
          has_usage_been_updated = true;
        }
      }
    }
  }
}

void CdmEngine::OnKeyReleaseEvent(const CdmKeySetId& key_set_id) {
  AutoLock lock(session_list_lock_);
  for (CdmSessionMap::iterator iter = sessions_.begin();
       iter != sessions_.end(); ++iter) {
    iter->second->OnKeyReleaseEvent(key_set_id);
  }
}

std::string CdmEngine::MapHdcpVersion(
    CryptoSession::HdcpCapability version) {
  switch (version) {
    case HDCP_NONE:
      return QUERY_VALUE_UNPROTECTED;
    case HDCP_V1:
      return QUERY_VALUE_HDCP_V1;
    case HDCP_V2:
      return QUERY_VALUE_HDCP_V2_0;
    case HDCP_V2_1:
      return QUERY_VALUE_HDCP_V2_1;
    case HDCP_V2_2:
      return QUERY_VALUE_HDCP_V2_2;
    case HDCP_NO_DIGITAL_OUTPUT:
      return QUERY_VALUE_DISCONNECTED;
  }
  return "";
}

void CdmEngine::DeleteAllUsageReportsUponFactoryReset() {
  std::string device_base_path_level1 = "";
  std::string device_base_path_level3 = "";
  Properties::GetDeviceFilesBasePath(kSecurityLevelL1,
                                     &device_base_path_level1);
  Properties::GetDeviceFilesBasePath(kSecurityLevelL3,
                                     &device_base_path_level3);

  File file;
  if (!file.Exists(device_base_path_level1) &&
      !file.Exists(device_base_path_level3)) {
    scoped_ptr<CryptoSession> crypto_session(new CryptoSession());
    CdmResponseType status = crypto_session->Open(
        cert_provisioning_requested_security_level_);
    if (NO_ERROR == status) {
      status = crypto_session->DeleteAllUsageReports();
      if (NO_ERROR != status) {
        LOGW(
            "CdmEngine::GetProvisioningRequest: "
            "Fails to delete usage reports: %d", status);
      }
    } else {
      LOGW(
          "CdmEngine::GetProvisioningRequest: "
          "Fails to open crypto session: error=%d.\n"
          "Usage reports are not removed after factory reset.", status);
    }
  }
}

}  // namespace wvcdm
