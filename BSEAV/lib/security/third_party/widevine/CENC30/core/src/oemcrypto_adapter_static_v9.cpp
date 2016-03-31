// Copyright 2013 Google Inc. All Rights Reserved.
//
// Wrapper of OEMCrypto APIs for platforms that support Level 1 only.
// This should be used when liboemcrypto.so is linked with the CDM code at
// compile time.
//
// An implementation should compile either oemcrypto_adapter_dynamic.cpp or
// oemcrypto_adapter_static.cpp, but not both.
//
// Defines APIs introduced in newer version (v9) which is not available in v8
// to allow an older oemcrypto implementation to be linked with CDM.

#include "OEMCryptoCENC.h"

extern "C" OEMCryptoResult OEMCrypto_LoadKeys_V8(
    OEMCrypto_SESSION session, const uint8_t* message, size_t message_length,
    const uint8_t* signature, size_t signature_length,
    const uint8_t* enc_mac_keys_iv, const uint8_t* enc_mac_keys,
    size_t num_keys, const OEMCrypto_KeyObject* key_array);

extern "C" OEMCryptoResult OEMCrypto_GenerateRSASignature_V8(
    OEMCrypto_SESSION session, const uint8_t* message, size_t message_length,
    uint8_t* signature, size_t* signature_length);

extern "C" OEMCryptoResult OEMCrypto_LoadKeys(
    OEMCrypto_SESSION session, const uint8_t* message, size_t message_length,
    const uint8_t* signature, size_t signature_length,
    const uint8_t* enc_mac_key_iv, const uint8_t* enc_mac_key, size_t num_keys,
    const OEMCrypto_KeyObject* key_array, const uint8_t* pst,
    size_t pst_length) {
  return OEMCrypto_LoadKeys_V8(session, message, message_length, signature,
                               signature_length, enc_mac_key_iv, enc_mac_key,
                               num_keys, key_array);
}

extern "C" OEMCryptoResult OEMCrypto_GenerateRSASignature(
    OEMCrypto_SESSION session, const uint8_t* message, size_t message_length,
    uint8_t* signature, size_t* signature_length,
    RSA_Padding_Scheme padding_scheme) {
  return OEMCrypto_GenerateRSASignature_V8(session, message, message_length,
                                           signature, signature_length);
}

extern "C" OEMCryptoResult OEMCrypto_GetHDCPCapability_V9(
    OEMCrypto_HDCP_Capability* current, OEMCrypto_HDCP_Capability* maximum) {
  return OEMCrypto_ERROR_NOT_IMPLEMENTED;
}

extern "C" bool OEMCrypto_SupportsUsageTable() { return false; }

extern "C" OEMCryptoResult OEMCrypto_UpdateUsageTable() {
  return OEMCrypto_ERROR_NOT_IMPLEMENTED;
}

extern "C" OEMCryptoResult OEMCrypto_DeactivateUsageEntry(const uint8_t* pst,
                                                          size_t pst_length) {
  return OEMCrypto_ERROR_NOT_IMPLEMENTED;
}

extern "C" OEMCryptoResult OEMCrypto_ReportUsage(OEMCrypto_SESSION session,
                                                 const uint8_t* pst,
                                                 size_t pst_length,
                                                 OEMCrypto_PST_Report* buffer,
                                                 size_t* buffer_length) {
  return OEMCrypto_ERROR_NOT_IMPLEMENTED;
}

extern "C" OEMCryptoResult OEMCrypto_DeleteUsageEntry(
    OEMCrypto_SESSION session, const uint8_t* pst, size_t pst_length,
    const uint8_t* message, size_t message_length, const uint8_t* signature,
    size_t signature_length) {
  return OEMCrypto_ERROR_NOT_IMPLEMENTED;
}

extern "C" OEMCryptoResult OEMCrypto_DeleteUsageTable(){
  return OEMCrypto_ERROR_NOT_IMPLEMENTED;
}
