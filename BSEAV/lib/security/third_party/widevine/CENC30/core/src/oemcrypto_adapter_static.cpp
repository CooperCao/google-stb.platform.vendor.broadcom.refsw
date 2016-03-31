// Copyright 2013 Google Inc. All Rights Reserved.
//
// Wrapper of OEMCrypto APIs for platforms that support  Level 1 only.
// This should be used when liboemcrypto.so is linked with the CDM code at
// compile time.
//

#include "oemcrypto_adapter.h"

namespace wvcdm {

OEMCryptoResult OEMCrypto_OpenSession(OEMCrypto_SESSION* session,
                                      SecurityLevel level) {
  return ::OEMCrypto_OpenSession(session);
}

OEMCryptoResult OEMCrypto_InstallKeybox(const uint8_t* keybox,
                                        size_t keyBoxLength,
                                        SecurityLevel level) {
  return ::OEMCrypto_InstallKeybox(keybox, keyBoxLength);
}

OEMCryptoResult OEMCrypto_IsKeyboxValid(SecurityLevel level) {
  return ::OEMCrypto_IsKeyboxValid();
}

OEMCryptoResult OEMCrypto_GetDeviceID(uint8_t* deviceID, size_t* idLength,
                                      SecurityLevel level) {
  return ::OEMCrypto_GetDeviceID(deviceID, idLength);
}

OEMCryptoResult OEMCrypto_GetKeyData(uint8_t* keyData, size_t* keyDataLength,
                                     SecurityLevel level) {
  return ::OEMCrypto_GetKeyData(keyData, keyDataLength);
}

uint32_t OEMCrypto_APIVersion(SecurityLevel level) {
  return ::OEMCrypto_APIVersion();
}

const char* OEMCrypto_SecurityLevel(SecurityLevel level) {
  return ::OEMCrypto_SecurityLevel();
}

OEMCryptoResult OEMCrypto_GetHDCPCapability(
    SecurityLevel level, OEMCrypto_HDCP_Capability* current,
    OEMCrypto_HDCP_Capability* maximum) {
  return ::OEMCrypto_GetHDCPCapability(current, maximum);
}

bool OEMCrypto_SupportsUsageTable(SecurityLevel level) {
  return ::OEMCrypto_SupportsUsageTable();
}

bool OEMCrypto_IsAntiRollbackHwPresent(SecurityLevel level) {
  return ::OEMCrypto_IsAntiRollbackHwPresent();
}

OEMCryptoResult OEMCrypto_GetNumberOfOpenSessions(SecurityLevel level,
                                                  size_t* count) {
  return ::OEMCrypto_GetNumberOfOpenSessions(count);
}

OEMCryptoResult OEMCrypto_GetMaxNumberOfSessions(SecurityLevel level,
                                                 size_t* maximum) {
  return ::OEMCrypto_GetMaxNumberOfSessions(maximum);
}

OEMCryptoResult OEMCrypto_CopyBuffer(
    SecurityLevel level, const uint8_t* data_addr, size_t data_length,
    OEMCrypto_DestBufferDesc* out_buffer, uint8_t subsample_flags) {
  return ::OEMCrypto_CopyBuffer(data_addr, data_length, out_buffer,
                                subsample_flags);
}
}  // namespace wvcdm
