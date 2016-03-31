// Copyright 2013 Google Inc. All Rights Reserved.
//
// Wrapper of OEMCrypto APIs for platforms that support  Level 1 only.
// This should be used when liboemcrypto.so is linked with the CDM code at
// compile time.
//
// Defines APIs introduced in newer version (v10) which is not available in v9
// to allow an older oemcrypto implementation to be linked with CDM.

#include "OEMCryptoCENC.h"

extern "C" OEMCryptoResult OEMCrypto_GetHDCPCapability_V9(uint8_t* current,
                                                          uint8_t* maximum);

extern "C" OEMCryptoResult OEMCrypto_LoadTestKeybox() {
  return OEMCrypto_ERROR_NOT_IMPLEMENTED;
}

extern "C" OEMCryptoResult OEMCrypto_LoadTestRSAKey() {
  return OEMCrypto_ERROR_NOT_IMPLEMENTED;
}

extern "C" OEMCryptoResult OEMCrypto_QueryKeyControl(
    OEMCrypto_SESSION session, const uint8_t* key_id, size_t key_id_length,
    uint8_t* key_control_block, size_t* key_control_block_length) {
  return OEMCrypto_ERROR_NOT_IMPLEMENTED;
}

extern "C" OEMCryptoResult OEMCrypto_CopyBuffer(
    const uint8_t* data_addr, size_t data_length,
    OEMCrypto_DestBufferDesc* out_buffer, uint8_t subsample_flags) {
  return OEMCrypto_ERROR_NOT_IMPLEMENTED;
}

extern "C" OEMCryptoResult OEMCrypto_GetHDCPCapability(
    OEMCrypto_HDCP_Capability* current, OEMCrypto_HDCP_Capability* maximum) {
  if (current == NULL) return OEMCrypto_ERROR_UNKNOWN_FAILURE;
  if (maximum == NULL) return OEMCrypto_ERROR_UNKNOWN_FAILURE;
  uint8_t current_byte, maximum_byte;
  OEMCryptoResult sts = OEMCrypto_GetHDCPCapability_V9(&current_byte,
                                                       &maximum_byte);
  *current = static_cast<OEMCrypto_HDCP_Capability>(current_byte);
  *maximum = static_cast<OEMCrypto_HDCP_Capability>(maximum_byte);
  return sts;
}

extern "C" bool OEMCrypto_IsAntiRollbackHwPresent() {
  return false;
}

extern "C" OEMCryptoResult OEMCrypto_GetNumberOfOpenSessions(size_t* count) {
  return OEMCrypto_ERROR_NOT_IMPLEMENTED;
}

extern "C" OEMCryptoResult OEMCrypto_GetMaxNumberOfSessions(size_t* max) {
  return OEMCrypto_ERROR_NOT_IMPLEMENTED;
}

extern "C" OEMCryptoResult OEMCrypto_ForceDeleteUsageEntry(
    const uint8_t* pst, size_t pst_length) {
  return OEMCrypto_ERROR_NOT_IMPLEMENTED;
}
