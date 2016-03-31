// Copyright 2012 Google Inc. All Rights Reserved.

#include "cdm_session.h"

#include <assert.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>

#include "cdm_engine.h"
#include "clock.h"
#include "crypto_session.h"
#include "device_files.h"
#include "file_store.h"
#include "log.h"
#include "properties.h"
#include "string_conversions.h"
#include "wv_cdm_constants.h"
#include "wv_cdm_event_listener.h"

namespace {
const size_t kKeySetIdLength = 14;
}  // namespace

namespace wvcdm {

CdmSession::CdmSession(CdmClientPropertySet* cdm_client_property_set,
                       const std::string& origin,
                       WvCdmEventListener* event_listener,
                       const CdmSessionId* forced_session_id)
    : initialized_(false),
      session_id_(GenerateSessionId()),
      origin_(origin),
      crypto_session_(new CryptoSession),
      file_handle_(new DeviceFiles),
      license_received_(false),
      is_offline_(false),
      is_release_(false),
      is_temporary_(false),
      security_level_(kSecurityLevelUninitialized),
      requested_security_level_(kLevelDefault),
      is_initial_decryption_(true),
      has_decrypted_since_last_report_(false),
      is_initial_usage_update_(true),
      is_usage_update_needed_(false) {
  if (Properties::AlwaysUseKeySetIds()) {
    if (forced_session_id) {
      key_set_id_ = *forced_session_id;
    } else {
      bool ok = GenerateKeySetId(&key_set_id_);
      (void)ok;  // ok is now used when assertions are turned off.
      assert(ok);
    }
    session_id_ = key_set_id_;
  }
  license_parser_.reset(new CdmLicense(session_id_));
  policy_engine_.reset(new PolicyEngine(
      session_id_, event_listener, crypto_session_.get()));
  if (cdm_client_property_set) {
    if (cdm_client_property_set->security_level() ==
        QUERY_VALUE_SECURITY_LEVEL_L3) {
      requested_security_level_ = kLevel3;
      security_level_ = kSecurityLevelL3;
    }
    Properties::AddSessionPropertySet(session_id_, cdm_client_property_set);
  }
}

CdmSession::~CdmSession() {
  if (!key_set_id_.empty()) {
    // Unreserve the license ID.
    file_handle_->UnreserveLicenseId(key_set_id_);
  }
  Properties::RemoveSessionPropertySet(session_id_);
}

CdmResponseType CdmSession::Init() {
  if (session_id_.empty()) {
    LOGE("CdmSession::Init: Failed, session not properly constructed");
    return SESSION_INIT_ERROR_1;
  }
  if (initialized_) {
    LOGE("CdmSession::Init: Failed due to previous initialization");
    return SESSION_INIT_ERROR_2;
  }
  CdmResponseType sts = crypto_session_->Open(requested_security_level_);
  if (NO_ERROR != sts) return sts;
  security_level_ = crypto_session_->GetSecurityLevel();

  std::string token;
  if (Properties::use_certificates_as_identification()) {
    std::string wrapped_key;
    if (!file_handle_->Init(security_level_) ||
        !file_handle_->RetrieveCertificate(origin_, &token, &wrapped_key) ||
        !crypto_session_->LoadCertificatePrivateKey(wrapped_key)) {
      return NEED_PROVISIONING;
    }
  } else {
    if (!crypto_session_->GetToken(&token))
      return SESSION_INIT_GET_KEYBOX_ERROR;
  }

  if (!license_parser_->Init(token, crypto_session_.get(),
                             policy_engine_.get()))
    return LICENSE_PARSER_INIT_ERROR;

  license_received_ = false;
  is_initial_decryption_ = true;
  initialized_ = true;
  return NO_ERROR;
}

CdmResponseType CdmSession::RestoreOfflineSession(
    const CdmKeySetId& key_set_id, const CdmLicenseType license_type) {
  key_set_id_ = key_set_id;

  // Retrieve license information from persistent store
  if (!file_handle_->Reset(security_level_))
    return RESTORE_OFFLINE_LICENSE_ERROR_1;

  DeviceFiles::LicenseState license_state;
  int64_t playback_start_time;
  int64_t last_playback_time;

  if (!file_handle_->RetrieveLicense(
          key_set_id, &license_state, &offline_init_data_, &key_request_,
          &key_response_, &offline_key_renewal_request_,
          &offline_key_renewal_response_, &offline_release_server_url_,
          &playback_start_time, &last_playback_time, &app_parameters_)) {
    LOGE("CdmSession::Init failed to retrieve license. key set id = %s",
         key_set_id.c_str());
    return GET_LICENSE_ERROR;
  }

  // Do not restore a released offline license, unless a release retry
  if (!(license_type == kLicenseTypeRelease ||
        license_state == DeviceFiles::kLicenseStateActive)) {
    LOGE("CdmSession::Init invalid offline license state = %d, type = %d",
         license_state, license_type);
    return GET_RELEASED_LICENSE_ERROR;
  }

  if (license_type == kLicenseTypeRelease) {
    if (!license_parser_->RestoreLicenseForRelease(key_request_,
                                                   key_response_)) {
      return RELEASE_LICENSE_ERROR_1;
    }
  } else {
    if (!license_parser_->RestoreOfflineLicense(
            key_request_, key_response_, offline_key_renewal_response_,
            playback_start_time, last_playback_time)) {
      return RESTORE_OFFLINE_LICENSE_ERROR_2;
    }
  }

  license_received_ = true;
  is_offline_ = true;
  is_release_ = license_type == kLicenseTypeRelease;
  return KEY_ADDED;
}

CdmResponseType CdmSession::RestoreUsageSession(
    const CdmKeyMessage& key_request, const CdmKeyResponse& key_response) {
  key_request_ = key_request;
  key_response_ = key_response;

  if (!license_parser_->RestoreLicenseForRelease(key_request_, key_response_)) {
    return RELEASE_LICENSE_ERROR_2;
  }

  license_received_ = true;
  is_offline_ = false;
  is_release_ = true;
  return KEY_ADDED;
}

CdmResponseType CdmSession::GenerateKeyRequest(
    const InitializationData& init_data, CdmLicenseType license_type,
    const CdmAppParameterMap& app_parameters, CdmKeyMessage* key_request,
    CdmKeyRequestType* key_request_type, std::string* server_url,
    CdmKeySetId* key_set_id) {
  if (crypto_session_.get() == NULL) {
    LOGW("CdmSession::GenerateKeyRequest: Invalid crypto session");
    return INVALID_CRYPTO_SESSION_1;
  }

  if (!crypto_session_->IsOpen()) {
    LOGW("CdmSession::GenerateKeyRequest: Crypto session not open");
    return CRYPTO_SESSION_OPEN_ERROR_1;
  }

  switch (license_type) {
    case kLicenseTypeTemporary:
      is_temporary_ = true;
      break;
    case kLicenseTypeStreaming:
      is_offline_ = false;
      break;
    case kLicenseTypeOffline:
      is_offline_ = true;
      break;
    case kLicenseTypeRelease:
      is_release_ = true;
      break;
    case kLicenseTypeDeferred:
      // If you're going to pass Deferred, you must have empty init data in
      // this call and stored init data from the previous call.
      if (!init_data.IsEmpty() || !license_parser_->HasInitData()) {
        return INVALID_LICENSE_TYPE;
      }
      // The arguments check out.
      // The is_release_ and is_offline_ flags were already set last time based
      // on the original license type.  Do not change them, and use them to
      // re-derive the original license type.
      if (is_release_) {
        license_type = kLicenseTypeRelease;
      } else if (is_offline_) {
        license_type = kLicenseTypeOffline;
      } else if (is_temporary_) {
        license_type = kLicenseTypeTemporary;
      } else {
        license_type = kLicenseTypeStreaming;
      }
      break;
    default:
      LOGE("CdmSession::GenerateKeyRequest: unrecognized license type: %ld",
           license_type);
      return INVALID_LICENSE_TYPE;
  }

  if (is_release_) {
    if (key_request_type) *key_request_type = kKeyRequestTypeRelease;
    return GenerateReleaseRequest(key_request, server_url);
  } else if (license_received_) {  // renewal
    if (key_request_type) *key_request_type = kKeyRequestTypeRenewal;
    return GenerateRenewalRequest(key_request, server_url);
  } else {
    if (key_request_type) *key_request_type = kKeyRequestTypeInitial;
    if (!license_parser_->HasInitData()) {
      if (!init_data.is_supported()) {
        LOGW("CdmSession::GenerateKeyRequest: unsupported init data type (%s)",
             init_data.type().c_str());
        return UNSUPPORTED_INIT_DATA;
      }
      if (init_data.IsEmpty()) {
        LOGW("CdmSession::GenerateKeyRequest: init data absent");
        return INIT_DATA_NOT_FOUND;
      }
    }
    if (is_offline_ && key_set_id_.empty() &&
        !GenerateKeySetId(&key_set_id_)) {
      LOGE("CdmSession::GenerateKeyRequest: Unable to generate key set ID");
      return KEY_REQUEST_ERROR_1;
    }

    app_parameters_ = app_parameters;
    CdmResponseType status = license_parser_->PrepareKeyRequest(
                                 init_data, license_type,
                                 app_parameters, key_request, server_url);

    if (KEY_MESSAGE != status) return status;

    key_request_ = *key_request;
    if (is_offline_) {
      offline_init_data_ = init_data.data();
      offline_release_server_url_ = *server_url;
    }

    if (key_set_id) *key_set_id = key_set_id_;
    return KEY_MESSAGE;
  }
}

// AddKey() - Accept license response and extract key info.
CdmResponseType CdmSession::AddKey(const CdmKeyResponse& key_response,
                                   CdmKeySetId* key_set_id) {
  if (crypto_session_.get() == NULL) {
    LOGW("CdmSession::AddKey: Invalid crypto session");
    return INVALID_CRYPTO_SESSION_2;
  }

  if (!crypto_session_->IsOpen()) {
    LOGW("CdmSession::AddKey: Crypto session not open");
    return CRYPTO_SESSION_OPEN_ERROR_2;
  }

  if (is_release_) {
    CdmResponseType sts = ReleaseKey(key_response);
    return (NO_ERROR == sts) ? KEY_ADDED : sts;
  } else if (license_received_) {  // renewal
    return RenewKey(key_response);
  } else {
    CdmResponseType sts = license_parser_->HandleKeyResponse(key_response);

    if (sts != KEY_ADDED) return (KEY_ERROR == sts) ? ADD_KEY_ERROR : sts;

    license_received_ = true;
    key_response_ = key_response;

    if (is_offline_ || !license_parser_->provider_session_token().empty()) {
      sts = StoreLicense();
      if (sts != NO_ERROR) return sts;
    }

    if (key_set_id) *key_set_id = key_set_id_;
    return KEY_ADDED;
  }
}

CdmResponseType CdmSession::QueryStatus(CdmQueryMap* key_info) {
  if (crypto_session_.get() == NULL) {
    LOGE("CdmSession::QueryStatus: Invalid crypto session");
    return INVALID_CRYPTO_SESSION_3;
  }

  if (!crypto_session_->IsOpen()) {
    LOGE("CdmSession::QueryStatus: Crypto session not open");
    return CRYPTO_SESSION_OPEN_ERROR_3;
  }

  switch (security_level_) {
    case kSecurityLevelL1:
      (*key_info)[QUERY_KEY_SECURITY_LEVEL] = QUERY_VALUE_SECURITY_LEVEL_L1;
      break;
    case kSecurityLevelL2:
      (*key_info)[QUERY_KEY_SECURITY_LEVEL] = QUERY_VALUE_SECURITY_LEVEL_L2;
      break;
    case kSecurityLevelL3:
      (*key_info)[QUERY_KEY_SECURITY_LEVEL] = QUERY_VALUE_SECURITY_LEVEL_L3;
      break;
    case kSecurityLevelUninitialized:
    case kSecurityLevelUnknown:
      (*key_info)[QUERY_KEY_SECURITY_LEVEL] =
          QUERY_VALUE_SECURITY_LEVEL_UNKNOWN;
      break;
    default:
      return INVALID_QUERY_KEY;
  }
  return NO_ERROR;
}

CdmResponseType CdmSession::QueryKeyStatus(CdmQueryMap* key_info) {
  return policy_engine_->Query(key_info);
}

CdmResponseType CdmSession::QueryKeyControlInfo(CdmQueryMap* key_info) {
  if (crypto_session_.get() == NULL) {
    LOGW("CdmSession::QueryKeyControlInfo: Invalid crypto session");
    return INVALID_CRYPTO_SESSION_4;
  }

  if (!crypto_session_->IsOpen()) {
    LOGW("CdmSession::QueryKeyControlInfo: Crypto session not open");
    return CRYPTO_SESSION_OPEN_ERROR_4;
  }

  std::stringstream ss;
  ss << crypto_session_->oec_session_id();
  (*key_info)[QUERY_KEY_OEMCRYPTO_SESSION_ID] = ss.str();
  return NO_ERROR;
}

// Decrypt() - Accept encrypted buffer and return decrypted data.
CdmResponseType CdmSession::Decrypt(const CdmDecryptionParameters& params) {
  if (crypto_session_.get() == NULL) {
    LOGW("CdmSession::Decrypt: Invalid crypto session");
    return INVALID_CRYPTO_SESSION_5;
  }

  if (!crypto_session_->IsOpen()) {
    LOGW("CdmSession::Decrypt: Crypto session not open");
    return CRYPTO_SESSION_OPEN_ERROR_5;
  }

  // Playback may not begin until either the start time passes or the license
  // is updated, so we treat this Decrypt call as invalid.
  if (params.is_encrypted && !policy_engine_->CanDecrypt(*params.key_id)) {
    return policy_engine_->IsLicenseForFuture() ? DECRYPT_NOT_READY : NEED_KEY;
  }

  CdmResponseType status = crypto_session_->Decrypt(params);

  if (status == NO_ERROR) {
    if (is_initial_decryption_) {
      policy_engine_->BeginDecryption();
      is_initial_decryption_ = false;
    }
    has_decrypted_since_last_report_ = true;
    if (!is_usage_update_needed_) {
      is_usage_update_needed_ =
          !license_parser_->provider_session_token().empty();
    }
  } else {
    Clock clock;
    int64_t current_time = clock.GetCurrentTime();
    if (policy_engine_->IsLicenseOrPlaybackDurationExpired(current_time)) {
      return NEED_KEY;
    }
  }

  return status;
}

// License renewal
// GenerateRenewalRequest() - Construct valid renewal request for the current
// session keys.
CdmResponseType CdmSession::GenerateRenewalRequest(CdmKeyMessage* key_request,
                                                   std::string* server_url) {
  CdmResponseType status = license_parser_->PrepareKeyUpdateRequest(
      true, app_parameters_, key_request, server_url);

  if (KEY_MESSAGE != status) return status;

  if (is_offline_) {
    offline_key_renewal_request_ = *key_request;
  }
  return KEY_MESSAGE;
}

// RenewKey() - Accept renewal response and update key info.
CdmResponseType CdmSession::RenewKey(const CdmKeyResponse& key_response) {
  CdmResponseType sts =
      license_parser_->HandleKeyUpdateResponse(true, key_response);
  if (sts != KEY_ADDED) return (KEY_ERROR == sts) ? RENEW_KEY_ERROR_1 : sts;

  if (is_offline_) {
    offline_key_renewal_response_ = key_response;
    if (!StoreLicense(DeviceFiles::kLicenseStateActive))
      return RENEW_KEY_ERROR_2;
  }
  return KEY_ADDED;
}

CdmResponseType CdmSession::GenerateReleaseRequest(CdmKeyMessage* key_request,
                                                   std::string* server_url) {
  is_release_ = true;
  CdmResponseType status = license_parser_->PrepareKeyUpdateRequest(
      false, app_parameters_, key_request, server_url);

  if (KEY_MESSAGE != status) return status;

  if (is_offline_) {  // Mark license as being released
    if (!StoreLicense(DeviceFiles::kLicenseStateReleasing))
      return RELEASE_KEY_REQUEST_ERROR;
  }
  return KEY_MESSAGE;
}

// ReleaseKey() - Accept release response and  release license.
CdmResponseType CdmSession::ReleaseKey(const CdmKeyResponse& key_response) {
  CdmResponseType sts =
      license_parser_->HandleKeyUpdateResponse(false, key_response);
  if (KEY_ADDED != sts) return (KEY_ERROR == sts) ? RELEASE_KEY_ERROR : sts;

  if (is_offline_ || !license_parser_->provider_session_token().empty()) {
    DeleteLicense();
  }
  return NO_ERROR;
}

bool CdmSession::IsKeyLoaded(const KeyId& key_id) {
  return license_parser_->IsKeyLoaded(key_id);
}

CdmSessionId CdmSession::GenerateSessionId() {
  static int session_num = 1;
  return SESSION_ID_PREFIX + IntToString(++session_num);
}

bool CdmSession::GenerateKeySetId(CdmKeySetId* key_set_id) {
  if (!key_set_id) {
    LOGW("CdmSession::GenerateKeySetId: key set id destination not provided");
    return false;
  }

  std::vector<uint8_t> random_data(
      (kKeySetIdLength - sizeof(KEY_SET_ID_PREFIX)) / 2, 0);

  if (!file_handle_->Reset(security_level_)) return false;

  while (key_set_id->empty()) {
    if (!crypto_session_->GetRandom(random_data.size(), &random_data[0]))
      return false;

    *key_set_id = KEY_SET_ID_PREFIX + b2a_hex(random_data);

    // key set collision
    if (file_handle_->LicenseExists(*key_set_id)) {
      key_set_id->clear();
    }
  }
  // Reserve the license ID to avoid collisions.
  file_handle_->ReserveLicenseId(*key_set_id);
  return true;
}

CdmResponseType CdmSession::StoreLicense() {
  if (is_temporary_) {
    LOGE("CdmSession::StoreLicense: Session type prohibits storage.");
    return STORAGE_PROHIBITED;
  }

  if (is_offline_) {
    if (key_set_id_.empty()) {
      LOGE("CdmSession::StoreLicense: No key set ID");
      return EMPTY_KEYSET_ID;
    }

    if (!license_parser_->is_offline()) {
      LOGE("CdmSession::StoreLicense: License policy prohibits storage.");
      return OFFLINE_LICENSE_PROHIBITED;
    }

    if (!StoreLicense(DeviceFiles::kLicenseStateActive)) {
      LOGE("CdmSession::StoreLicense: Unable to store license");
      CdmResponseType sts = Init();
      if (sts != NO_ERROR) {
        LOGW("CdmSession::StoreLicense: Reinitialization failed");
        return sts;
      }

      key_set_id_.clear();
      return STORE_LICENSE_ERROR_1;
    }
    return NO_ERROR;
  }  // if (is_offline_)

  std::string provider_session_token =
      license_parser_->provider_session_token();
  if (provider_session_token.empty()) {
    LOGE("CdmSession::StoreLicense: No provider session token and not offline");
    return STORE_LICENSE_ERROR_2;
  }

  if (!file_handle_->Reset(security_level_)) {
    LOGE("CdmSession::StoreLicense: Unable to initialize device files");
    return STORE_LICENSE_ERROR_3;
  }

  std::string app_id;
  GetApplicationId(&app_id);
  if (!file_handle_->StoreUsageInfo(provider_session_token, key_request_,
                                    key_response_, app_id, key_set_id_)) {
    LOGE("CdmSession::StoreLicense: Unable to store usage info");
    return STORE_USAGE_INFO_ERROR;
  }
  return NO_ERROR;
}

bool CdmSession::StoreLicense(DeviceFiles::LicenseState state) {
  if (!file_handle_->Reset(security_level_)) return false;

  return file_handle_->StoreLicense(
      key_set_id_, state, offline_init_data_, key_request_, key_response_,
      offline_key_renewal_request_, offline_key_renewal_response_,
      offline_release_server_url_, policy_engine_->GetPlaybackStartTime(),
      policy_engine_->GetLastPlaybackTime(), app_parameters_);
}

bool CdmSession::DeleteLicense() {
  if (!is_offline_ && license_parser_->provider_session_token().empty())
    return false;

  if (!file_handle_->Reset(security_level_)) {
    LOGE("CdmSession::DeleteLicense: Unable to initialize device files");
    return false;
  }
  if (is_offline_) {
    return file_handle_->DeleteLicense(key_set_id_);
  } else {
    std::string app_id;
    GetApplicationId(&app_id);
    return file_handle_->DeleteUsageInfo(
        app_id, license_parser_->provider_session_token());
  }
}

void CdmSession::NotifyResolution(uint32_t width, uint32_t height) {
  policy_engine_->NotifyResolution(width, height);
}

void CdmSession::OnTimerEvent(bool update_usage) {
  if (update_usage && has_decrypted_since_last_report_) {
    policy_engine_->DecryptionEvent();
    has_decrypted_since_last_report_ = false;
    if (is_offline_ && !is_release_) {
      StoreLicense(DeviceFiles::kLicenseStateActive);
    }
  }
  policy_engine_->OnTimerEvent();
}

void CdmSession::OnKeyReleaseEvent(const CdmKeySetId& key_set_id) {
  if (key_set_id_ == key_set_id) {
    policy_engine_->NotifySessionExpiration();
  }
}

void CdmSession::GetApplicationId(std::string* app_id) {
  if (app_id && !Properties::GetApplicationId(session_id_, app_id)) {
    *app_id = "";
  }
}

CdmResponseType CdmSession::DeleteMultipleUsageInformation(
    const std::vector<std::string>& provider_session_tokens) {
  return crypto_session_->DeleteMultipleUsageInformation(
      provider_session_tokens);
}

CdmResponseType CdmSession::UpdateUsageInformation() {
  return crypto_session_->UpdateUsageInformation();
}

CdmResponseType CdmSession::ReleaseCrypto() {
  crypto_session_->Close();
  return NO_ERROR;
}

void CdmSession::set_license_parser(CdmLicense* license_parser) {
  license_parser_.reset(license_parser);
}

void CdmSession::set_crypto_session(CryptoSession* crypto_session) {
  crypto_session_.reset(crypto_session);
}

void CdmSession::set_policy_engine(PolicyEngine* policy_engine) {
  policy_engine_.reset(policy_engine);
}

void CdmSession::set_file_handle(DeviceFiles* file_handle) {
  file_handle_.reset(file_handle);
}

}  // namespace wvcdm
