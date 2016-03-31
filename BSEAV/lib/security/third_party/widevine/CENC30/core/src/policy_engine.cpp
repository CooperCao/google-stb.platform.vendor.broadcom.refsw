// Copyright 2013 Google Inc. All Rights Reserved.

#include "policy_engine.h"

#include <limits.h>

#include <sstream>
#include <string>
#include <vector>

#include "clock.h"
#include "log.h"
#include "properties.h"
#include "string_conversions.h"
#include "wv_cdm_constants.h"
#include "wv_cdm_event_listener.h"

using video_widevine_server::sdk::License;

namespace wvcdm {

PolicyEngine::PolicyEngine(CdmSessionId session_id,
                           WvCdmEventListener* event_listener,
                           CryptoSession* crypto_session)
    : license_state_(kLicenseStateInitial),
      license_start_time_(0),
      playback_start_time_(0),
      last_playback_time_(0),
      last_expiry_time_(0),
      last_expiry_time_set_(false),
      next_renewal_time_(0),
      policy_max_duration_seconds_(0),
      session_id_(session_id),
      event_listener_(event_listener),
      max_res_engine_(new MaxResEngine(crypto_session)),
      clock_(new Clock) {}

PolicyEngine::~PolicyEngine() {}

bool PolicyEngine::CanDecrypt(const KeyId& key_id) {
  if (keys_status_.find(key_id) == keys_status_.end()) {
    LOGE("PolicyEngine::CanDecrypt Key '%s' not in license.",
         b2a_hex(key_id).c_str());
    return false;
  }
  return keys_status_[key_id] == kKeyStatusUsable;
}

void PolicyEngine::OnTimerEvent() {
  int64_t current_time = clock_->GetCurrentTime();

  // License expiration trumps all.
  if (IsLicenseOrPlaybackDurationExpired(current_time) &&
      license_state_ != kLicenseStateExpired) {
    license_state_ = kLicenseStateExpired;
    NotifyKeysChange(kKeyStatusExpired);
    return;
  }

  max_res_engine_->OnTimerEvent();

  bool renewal_needed = false;

  // Test to determine if renewal should be attempted.
  switch (license_state_) {
    case kLicenseStateCanPlay: {
      if (IsRenewalDelayExpired(current_time)) renewal_needed = true;
      // HDCP may change, so force a check.
      NotifyKeysChange(kKeyStatusUsable);
      break;
    }

    case kLicenseStateNeedRenewal: {
      renewal_needed = true;
      break;
    }

    case kLicenseStateWaitingLicenseUpdate: {
      if (IsRenewalRetryIntervalExpired(current_time)) renewal_needed = true;
      break;
    }

    case kLicenseStatePending: {
      if (current_time >= license_start_time_) {
        license_state_ = kLicenseStateCanPlay;
        NotifyKeysChange(kKeyStatusUsable);
      }
      break;
    }

    case kLicenseStateInitial:
    case kLicenseStateExpired: {
      break;
    }

    default: {
      license_state_ = kLicenseStateExpired;
      NotifyKeysChange(kKeyStatusInternalError);
      break;
    }
  }

  if (renewal_needed) {
    UpdateRenewalRequest(current_time);
    if (event_listener_) event_listener_->OnSessionRenewalNeeded(session_id_);
  }
}

void PolicyEngine::SetLicense(const License& license) {
  license_id_.Clear();
  license_id_.CopyFrom(license.id());
  policy_.Clear();

  // Extract content key ids.
  keys_status_.clear();
  for (int key_index = 0; key_index < license.key_size(); ++key_index) {
    const License::KeyContainer& key = license.key(key_index);
    if (key.type() == License::KeyContainer::CONTENT && key.has_id())
      keys_status_[key.id()] = kKeyStatusInternalError;
  }

  UpdateLicense(license);
  max_res_engine_->SetLicense(license);
}

void PolicyEngine::SetLicenseForRelease(const License& license) {
  license_id_.Clear();
  license_id_.CopyFrom(license.id());
  policy_.Clear();

  // Expire any old keys.
  NotifyKeysChange(kKeyStatusExpired);

  UpdateLicense(license);
}

void PolicyEngine::UpdateLicense(const License& license) {
  if (!license.has_policy()) return;

  if (kLicenseStateExpired == license_state_) {
    LOGD("PolicyEngine::UpdateLicense: updating an expired license");
  }

  policy_.MergeFrom(license.policy());

  // some basic license validation
  // license start time needs to be specified in the initial response
  if (!license.has_license_start_time()) return;

  // if renewal, discard license if version has not been updated
  if (license_state_ != kLicenseStateInitial) {
    if (license.id().version() > license_id_.version())
      license_id_.CopyFrom(license.id());
    else
      return;
  }

  // Update time information
  license_start_time_ = license.license_start_time();
  next_renewal_time_ = license_start_time_ + policy_.renewal_delay_seconds();

  // Calculate policy_max_duration_seconds_. policy_max_duration_seconds_
  // will be set to the minimum of the following policies :
  // rental_duration_seconds and license_duration_seconds.
  // The value is used to determine when the license expires.
  policy_max_duration_seconds_ = 0;

  if (policy_.has_rental_duration_seconds())
    policy_max_duration_seconds_ = policy_.rental_duration_seconds();

  if ((policy_.license_duration_seconds() > 0) &&
      ((policy_.license_duration_seconds() < policy_max_duration_seconds_) ||
       policy_max_duration_seconds_ == 0)) {
    policy_max_duration_seconds_ = policy_.license_duration_seconds();
  }

  int64_t current_time = clock_->GetCurrentTime();
  if (!policy_.can_play() || IsLicenseOrPlaybackDurationExpired(current_time)) {
    license_state_ = kLicenseStateExpired;
    NotifyKeysChange(kKeyStatusExpired);
    return;
  }

  // Update state
  if (current_time >= license_start_time_) {
    license_state_ = kLicenseStateCanPlay;
    NotifyKeysChange(kKeyStatusUsable);
  } else {
    license_state_ = kLicenseStatePending;
    NotifyKeysChange(kKeyStatusPending);
  }
  NotifyExpirationUpdate();
}

void PolicyEngine::BeginDecryption() {
  if (playback_start_time_ == 0) {
    switch (license_state_) {
      case kLicenseStateCanPlay:
      case kLicenseStateNeedRenewal:
      case kLicenseStateWaitingLicenseUpdate:
        playback_start_time_ = clock_->GetCurrentTime();
        last_playback_time_ = playback_start_time_;

        if (policy_.renew_with_usage()) {
          license_state_ = kLicenseStateNeedRenewal;
        }
        NotifyExpirationUpdate();
        break;
      case kLicenseStateInitial:
      case kLicenseStatePending:
      case kLicenseStateExpired:
      default:
        break;
    }
  }
}

void PolicyEngine::DecryptionEvent() {
  last_playback_time_ = clock_->GetCurrentTime();
}

void PolicyEngine::NotifyResolution(uint32_t width, uint32_t height) {
  max_res_engine_->SetResolution(width, height);
}

void PolicyEngine::NotifySessionExpiration() {
  license_state_ = kLicenseStateExpired;
  NotifyKeysChange(kKeyStatusExpired);
}

CdmResponseType PolicyEngine::Query(CdmQueryMap* key_info) {
  std::stringstream ss;
  int64_t current_time = clock_->GetCurrentTime();

  if (license_state_ == kLicenseStateInitial) {
    key_info->clear();
    return NO_ERROR;
  }

  (*key_info)[QUERY_KEY_LICENSE_TYPE] =
      license_id_.type() == video_widevine_server::sdk::STREAMING
          ? QUERY_VALUE_STREAMING
          : QUERY_VALUE_OFFLINE;
  (*key_info)[QUERY_KEY_PLAY_ALLOWED] =
      policy_.can_play() ? QUERY_VALUE_TRUE : QUERY_VALUE_FALSE;
  (*key_info)[QUERY_KEY_PERSIST_ALLOWED] =
      policy_.can_persist() ? QUERY_VALUE_TRUE : QUERY_VALUE_FALSE;
  (*key_info)[QUERY_KEY_RENEW_ALLOWED] =
      policy_.can_renew() ? QUERY_VALUE_TRUE : QUERY_VALUE_FALSE;
  ss << GetLicenseDurationRemaining(current_time);
  (*key_info)[QUERY_KEY_LICENSE_DURATION_REMAINING] = ss.str();
  ss.str("");
  ss << GetPlaybackDurationRemaining(current_time);
  (*key_info)[QUERY_KEY_PLAYBACK_DURATION_REMAINING] = ss.str();
  (*key_info)[QUERY_KEY_RENEWAL_SERVER_URL] = policy_.renewal_server_url();

  return NO_ERROR;
}

bool PolicyEngine::GetSecondsSinceStarted(int64_t* seconds_since_started) {
  if (playback_start_time_ == 0) return false;

  *seconds_since_started = clock_->GetCurrentTime() - playback_start_time_;
  return (*seconds_since_started >= 0) ? true : false;
}

bool PolicyEngine::GetSecondsSinceLastPlayed(
    int64_t* seconds_since_last_played) {
  if (last_playback_time_ == 0) return false;

  *seconds_since_last_played = clock_->GetCurrentTime() - last_playback_time_;
  return (*seconds_since_last_played >= 0) ? true : false;
}

void PolicyEngine::RestorePlaybackTimes(int64_t playback_start_time,
                                        int64_t last_playback_time) {
  playback_start_time_ = (playback_start_time > 0) ? playback_start_time : 0;
  last_playback_time_ = (last_playback_time > 0) ? last_playback_time : 0;
  NotifyExpirationUpdate();
}

void PolicyEngine::UpdateRenewalRequest(int64_t current_time) {
  license_state_ = kLicenseStateWaitingLicenseUpdate;
  next_renewal_time_ = current_time + policy_.renewal_retry_interval_seconds();
}

bool PolicyEngine::IsLicenseOrPlaybackDurationExpired(int64_t current_time) {
  int64_t expiry_time =
      IsPlaybackStarted() ? GetPlaybackExpiryTime() : GetLicenseExpiryTime();
  return (expiry_time == NEVER_EXPIRES) ? false : (expiry_time <= current_time);
}

// For the policy time fields checked in the following methods, a value of 0
// indicates that there is no limit to the duration. These methods
// will always return false if the value is 0.
int64_t PolicyEngine::GetLicenseExpiryTime() {
  return policy_max_duration_seconds_ > 0
             ? license_start_time_ + policy_max_duration_seconds_
             : NEVER_EXPIRES;
}

int64_t PolicyEngine::GetPlaybackExpiryTime() {
  return (playback_start_time_ > 0 && policy_.playback_duration_seconds() > 0)
             ? (playback_start_time_ + policy_.playback_duration_seconds())
             : NEVER_EXPIRES;
}

int64_t PolicyEngine::GetLicenseDurationRemaining(int64_t current_time) {
  int64_t license_expiry_time = GetLicenseExpiryTime();
  if (license_expiry_time == NEVER_EXPIRES) return LLONG_MAX;
  if (license_expiry_time < current_time) return 0;
  return std::min(license_expiry_time - current_time,
                  policy_max_duration_seconds_);
}

int64_t PolicyEngine::GetPlaybackDurationRemaining(int64_t current_time) {
  int64_t playback_expiry_time = GetPlaybackExpiryTime();
  if (playback_expiry_time == NEVER_EXPIRES) {
    return (policy_.playback_duration_seconds() != 0)
               ? policy_.playback_duration_seconds()
               : LLONG_MAX;
  }

  if (playback_expiry_time < current_time) return 0;
  return std::min(playback_expiry_time - current_time,
                  policy_.playback_duration_seconds());
}

bool PolicyEngine::IsRenewalDelayExpired(int64_t current_time) {
  return policy_.can_renew() && (policy_.renewal_delay_seconds() > 0) &&
         license_start_time_ + policy_.renewal_delay_seconds() <= current_time;
}

bool PolicyEngine::IsRenewalRecoveryDurationExpired(int64_t current_time) {
  // NOTE: Renewal Recovery Duration is currently not used.
  return (policy_.renewal_recovery_duration_seconds() > 0) &&
         license_start_time_ + policy_.renewal_recovery_duration_seconds() <=
             current_time;
}

bool PolicyEngine::IsRenewalRetryIntervalExpired(int64_t current_time) {
  return policy_.can_renew() &&
         (policy_.renewal_retry_interval_seconds() > 0) &&
         next_renewal_time_ <= current_time;
}

void PolicyEngine::NotifyKeysChange(CdmKeyStatus new_status) {
  bool keys_changed = false;
  bool has_new_usable_key = false;
  for (std::map<KeyId, CdmKeyStatus>::iterator it = keys_status_.begin();
       it != keys_status_.end(); ++it) {
    const KeyId key_id = it->first;
    CdmKeyStatus& key_status = it->second;
    CdmKeyStatus updated_status = new_status;
    if (updated_status == kKeyStatusUsable) {
      if (!max_res_engine_->CanDecrypt(key_id))
        updated_status = kKeyStatusOutputNotAllowed;
    }
    if (key_status != updated_status) {
      key_status = updated_status;
      if (updated_status == kKeyStatusUsable) has_new_usable_key = true;
      keys_changed = true;
    }
  }
  if (keys_changed && event_listener_) {
    event_listener_->OnSessionKeysChange(session_id_, keys_status_,
                                         has_new_usable_key);
  }
}

void PolicyEngine::NotifyExpirationUpdate() {
  int64_t expiry_time =
      IsPlaybackStarted() ? GetPlaybackExpiryTime() : GetLicenseExpiryTime();
  if (!last_expiry_time_set_ || expiry_time != last_expiry_time_) {
    last_expiry_time_ = expiry_time;
    if (event_listener_)
      event_listener_->OnExpirationUpdate(session_id_, expiry_time);
  }
  last_expiry_time_set_ = true;
}

void PolicyEngine::set_clock(Clock* clock) { clock_.reset(clock); }

void PolicyEngine::set_max_res_engine(MaxResEngine* max_res_engine) {
  max_res_engine_.reset(max_res_engine);
}

}  // wvcdm
