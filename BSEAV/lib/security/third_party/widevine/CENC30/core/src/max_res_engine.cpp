// Copyright 2014 Google Inc. All Rights Reserved.

#include "max_res_engine.h"

#include "clock.h"
#include "log.h"

namespace {

const int64_t kHdcpCheckInterval = 10;
const uint32_t kNoResolution = 0;

}  // namespace

namespace wvcdm {

MaxResEngine::MaxResEngine(CryptoSession* crypto_session) {
  Init(crypto_session, new Clock());
}

MaxResEngine::MaxResEngine(CryptoSession* crypto_session, Clock* clock) {
  Init(crypto_session, clock);
}

MaxResEngine::~MaxResEngine() {
  AutoLock lock(status_lock_);
  DeleteAllKeys();
}

bool MaxResEngine::CanDecrypt(const KeyId& key_id) {
  AutoLock lock(status_lock_);
  if (keys_.count(key_id) > 0) {
    return keys_[key_id]->can_decrypt();
  } else {
    // If a Key ID is unknown to us, we don't know of any constraints for it,
    // so never block decryption.
    return true;
  }
}

void MaxResEngine::Init(CryptoSession* crypto_session, Clock* clock) {
  AutoLock lock(status_lock_);
  current_resolution_ = kNoResolution;
  clock_.reset(clock);
  next_check_time_ = clock_->GetCurrentTime();
  crypto_session_ = crypto_session;
}

void MaxResEngine::SetLicense(
    const video_widevine_server::sdk::License& license) {
  AutoLock lock(status_lock_);
  DeleteAllKeys();
  for (int32_t key_index = 0; key_index < license.key_size(); ++key_index) {
    const KeyContainer& key = license.key(key_index);
    if (key.type() == KeyContainer::CONTENT && key.has_id() &&
        key.video_resolution_constraints_size() > 0) {
      const ConstraintList& constraints = key.video_resolution_constraints();
      const KeyId& key_id = key.id();
      if (key.has_required_protection()) {
        keys_[key_id] =
            new KeyStatus(constraints, key.required_protection().hdcp());
      } else {
        keys_[key_id] = new KeyStatus(constraints);
      }
    }
  }
}

void MaxResEngine::SetResolution(uint32_t width, uint32_t height) {
  AutoLock lock(status_lock_);
  current_resolution_ = width * height;
}

void MaxResEngine::OnTimerEvent() {
  AutoLock lock(status_lock_);
  int64_t current_time = clock_->GetCurrentTime();
  if (!keys_.empty() && current_resolution_ != kNoResolution &&
      current_time >= next_check_time_) {
    CryptoSession::HdcpCapability current_hdcp_level;
    CryptoSession::HdcpCapability ignored;
    if (!crypto_session_->GetHdcpCapabilities(&current_hdcp_level, &ignored)) {
      current_hdcp_level = HDCP_NONE;
    }
    for (KeyIterator i = keys_.begin(); i != keys_.end(); ++i) {
      i->second->Update(current_resolution_, current_hdcp_level);
    }
    next_check_time_ = current_time + kHdcpCheckInterval;
  }
}

void MaxResEngine::DeleteAllKeys() {
  // This helper method assumes that status_lock_ is already held.
  for (KeyIterator i = keys_.begin(); i != keys_.end(); ++i) delete i->second;
  keys_.clear();
}

MaxResEngine::KeyStatus::KeyStatus(const ConstraintList& constraints)
    : default_hdcp_level_(HDCP_NONE) {
  Init(constraints);
}

MaxResEngine::KeyStatus::KeyStatus(
    const ConstraintList& constraints,
    const OutputProtection::HDCP& default_hdcp_level)
    : default_hdcp_level_(ProtobufHdcpToOemCryptoHdcp(default_hdcp_level)) {
  Init(constraints);
}

void MaxResEngine::KeyStatus::Init(const ConstraintList& constraints) {
  constraints_.Clear();
  constraints_.MergeFrom(constraints);
  can_decrypt_ = true;
}

void MaxResEngine::KeyStatus::Update(
    uint32_t res, CryptoSession::HdcpCapability current_hdcp_level) {
  VideoResolutionConstraint* current_constraint = GetConstraintForRes(res);

  if (current_constraint == NULL) {
    can_decrypt_ = false;
    return;
  }

  CryptoSession::HdcpCapability desired_hdcp_level;
  if (current_constraint->has_required_protection()) {
    desired_hdcp_level = ProtobufHdcpToOemCryptoHdcp(
        current_constraint->required_protection().hdcp());
  } else {
    desired_hdcp_level = default_hdcp_level_;
  }
  can_decrypt_ = (current_hdcp_level >= desired_hdcp_level);
}

MaxResEngine::VideoResolutionConstraint*
MaxResEngine::KeyStatus::GetConstraintForRes(uint32_t res) {
  typedef ConstraintList::pointer_iterator Iterator;
  for (Iterator i = constraints_.pointer_begin();
       i != constraints_.pointer_end(); ++i) {
    VideoResolutionConstraint* constraint = *i;
    if (constraint->has_min_resolution_pixels() &&
        constraint->has_max_resolution_pixels() &&
        res >= constraint->min_resolution_pixels() &&
        res <= constraint->max_resolution_pixels()) {
      return constraint;
    }
  }
  return NULL;
}

CryptoSession::HdcpCapability
MaxResEngine::KeyStatus::ProtobufHdcpToOemCryptoHdcp(
    const OutputProtection::HDCP& input) {
  switch (input) {
    case OutputProtection::HDCP_NONE:
      return HDCP_NONE;
    case OutputProtection::HDCP_V1:
      return HDCP_V1;
    case OutputProtection::HDCP_V2:
      return HDCP_V2;
    case OutputProtection::HDCP_V2_1:
      return HDCP_V2_1;
    case OutputProtection::HDCP_V2_2:
      return HDCP_V2_2;
    case OutputProtection::HDCP_NO_DIGITAL_OUTPUT:
      return HDCP_NO_DIGITAL_OUTPUT;
    default:
      LOGE("MaxResEngine::KeyStatus::ProtobufHdcpToOemCryptoHdcp: "
           "Unknown HDCP Level");
      return HDCP_NO_DIGITAL_OUTPUT;
  }
}

}  // wvcdm
