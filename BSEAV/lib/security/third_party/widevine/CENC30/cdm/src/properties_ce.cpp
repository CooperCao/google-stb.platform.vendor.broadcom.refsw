// Copyright 2015 Google Inc. All Rights Reserved.
#include "properties.h"
#include "properties_ce.h"

#include "cdm_version.h"
#include "log.h"

// This anonymous namespace is shared between both the widevine namespace and
// wvcdm namespace objects below.
namespace {

bool use_secure_buffers_ = false;
bool use_fifo_ = false;
bool use_userspace_buffers_ = true;

widevine::Cdm::SecureOutputType secure_output_type_ =
    widevine::Cdm::kNoSecureOutput;
widevine::Cdm::ClientInfo client_info_;

bool GetValue(const std::string& source, std::string* output) {
  if (!output) {
    return false;
  }
  *output = source;
  return source.size() != 0;
}

}  // namespace

namespace widevine {

// static
void PropertiesCE::SetSecureOutputType(
    Cdm::SecureOutputType secure_output_type) {
  secure_output_type_ = secure_output_type;

  switch (secure_output_type) {
    case Cdm::kOpaqueHandle:
      use_secure_buffers_ = true;
      use_fifo_ = false;
      use_userspace_buffers_ = false;
      break;
    case Cdm::kDirectRender:
      use_secure_buffers_ = false;
      use_fifo_ = true;
      use_userspace_buffers_ = false;
      break;
    case Cdm::kNoSecureOutput:
    default:
      use_secure_buffers_ = false;
      use_fifo_ = false;
      use_userspace_buffers_ = true;
      break;
  }
}

// static
Cdm::SecureOutputType PropertiesCE::GetSecureOutputType() {
  return secure_output_type_;
}

// static
void PropertiesCE::SetClientInfo(const Cdm::ClientInfo& client_info) {
  client_info_ = client_info;
}

// static
Cdm::ClientInfo PropertiesCE::GetClientInfo() {
  return client_info_;
}

}  // namespace widevine

namespace wvcdm {

// static
void Properties::Init() {
  oem_crypto_use_secure_buffers_ = use_secure_buffers_;
  oem_crypto_use_fifo_ = use_fifo_;
  oem_crypto_use_userspace_buffers_ = use_userspace_buffers_;
  use_certificates_as_identification_ = true;
  security_level_path_backward_compatibility_support_ = false;
  session_property_set_.reset(new CdmClientPropertySetMap());
}

// static
bool Properties::GetCompanyName(std::string* company_name) {
  return GetValue(client_info_.company_name, company_name);
}

// static
bool Properties::GetModelName(std::string* model_name) {
  return GetValue(client_info_.model_name, model_name);
}

// static
bool Properties::GetArchitectureName(std::string* arch_name) {
  return GetValue(client_info_.arch_name, arch_name);
}

// static
bool Properties::GetDeviceName(std::string* device_name) {
  return GetValue(client_info_.device_name, device_name);
}

// static
bool Properties::GetProductName(std::string* product_name) {
  return GetValue(client_info_.product_name, product_name);
}

// static
bool Properties::GetBuildInfo(std::string* build_info) {
  return GetValue(client_info_.build_info, build_info);
}

// static
bool Properties::GetWVCdmVersion(std::string* version) {
  return GetValue(CDM_VERSION, version);
}

// static
bool Properties::GetDeviceFilesBasePath(CdmSecurityLevel security_level,
                                        std::string* base_path) {
  // A no-op, but successful.
  base_path->clear();
  return true;
}

// static
bool Properties::GetFactoryKeyboxPath(std::string* keybox) {
  // Unused on CE devices.
  return false;
}

// static
bool Properties::GetOEMCryptoPath(std::string* library_name) {
  // Unused on CE devices.
  return false;
}

// static
bool Properties::AlwaysUseKeySetIds() {
  return true;
}

}  // namespace wvcdm
