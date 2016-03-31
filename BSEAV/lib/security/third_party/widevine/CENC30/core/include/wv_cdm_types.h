// Copyright 2013 Google Inc. All Rights Reserved.

#ifndef WVCDM_CORE_WV_CDM_TYPES_H_
#define WVCDM_CORE_WV_CDM_TYPES_H_

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

namespace wvcdm {

typedef std::string CdmKeySystem;
typedef std::string CdmInitData;
typedef std::string CdmKeyMessage;
typedef std::string CdmKeyResponse;
typedef std::string KeyId;
typedef std::string CdmSecureStopId;
typedef std::string CdmSessionId;
typedef std::string CdmKeySetId;
typedef std::string RequestId;
typedef uint32_t CryptoResult;
typedef uint32_t CryptoSessionId;
typedef std::string CryptoKeyId;
typedef std::map<std::string, std::string> CdmAppParameterMap;
typedef std::map<std::string, std::string> CdmQueryMap;
typedef std::vector<std::string> CdmUsageInfo;
typedef std::string CdmUsageInfoReleaseMessage;
typedef std::string CdmProvisioningRequest;
typedef std::string CdmProvisioningResponse;

enum CdmKeyRequestType {
  kKeyRequestTypeUnknown,
  kKeyRequestTypeInitial,
  kKeyRequestTypeRenewal,
  kKeyRequestTypeRelease,
};

enum CdmResponseType {
  NO_ERROR,
  UNKNOWN_ERROR,
  KEY_ADDED,
  KEY_ERROR,
  KEY_MESSAGE,
  NEED_KEY,
  KEY_CANCELED,
  NEED_PROVISIONING,
  DEVICE_REVOKED,
  INSUFFICIENT_CRYPTO_RESOURCES,
  ADD_KEY_ERROR,
  CERT_PROVISIONING_GET_KEYBOX_ERROR_1,
  CERT_PROVISIONING_GET_KEYBOX_ERROR_2,
  CERT_PROVISIONING_INVALID_CERT_TYPE,
  CERT_PROVISIONING_REQUEST_ERROR_1,
  CERT_PROVISIONING_REQUEST_ERROR_2,
  CERT_PROVISIONING_REQUEST_ERROR_3,
  CERT_PROVISIONING_REQUEST_ERROR_4,
  CERT_PROVISIONING_RESPONSE_ERROR_1,
  CERT_PROVISIONING_RESPONSE_ERROR_2,
  CERT_PROVISIONING_RESPONSE_ERROR_3,
  CERT_PROVISIONING_RESPONSE_ERROR_4,
  CERT_PROVISIONING_RESPONSE_ERROR_5,
  CERT_PROVISIONING_RESPONSE_ERROR_6,
  CERT_PROVISIONING_RESPONSE_ERROR_7,
  CERT_PROVISIONING_RESPONSE_ERROR_8,
  CRYPTO_SESSION_OPEN_ERROR_1,
  CRYPTO_SESSION_OPEN_ERROR_2,
  CRYPTO_SESSION_OPEN_ERROR_3,
  CRYPTO_SESSION_OPEN_ERROR_4,
  CRYPTO_SESSION_OPEN_ERROR_5,
  DECRYPT_NOT_READY,
  DEVICE_CERTIFICATE_ERROR_1,
  DEVICE_CERTIFICATE_ERROR_2,
  DEVICE_CERTIFICATE_ERROR_3,
  DEVICE_CERTIFICATE_ERROR_4,
  EMPTY_KEY_DATA_1,
  EMPTY_KEY_DATA_2,
  EMPTY_KEYSET_ID,
  EMPTY_KEYSET_ID_ENG_1,
  EMPTY_KEYSET_ID_ENG_2,
  EMPTY_KEYSET_ID_ENG_3,
  EMPTY_KEYSET_ID_ENG_4,
  EMPTY_LICENSE_RENEWAL,
  EMPTY_LICENSE_RESPONSE_1,
  EMPTY_LICENSE_RESPONSE_2,
  EMPTY_PROVISIONING_CERTIFICATE_1,
  EMPTY_PROVISIONING_RESPONSE,
  EMPTY_SESSION_ID,
  GENERATE_DERIVED_KEYS_ERROR,
  LICENSE_RENEWAL_NONCE_GENERATION_ERROR,
  GENERATE_USAGE_REPORT_ERROR,
  GET_LICENSE_ERROR,
  GET_RELEASED_LICENSE_ERROR,
  GET_USAGE_INFO_ERROR_1,
  GET_USAGE_INFO_ERROR_2,
  GET_USAGE_INFO_ERROR_3,
  GET_USAGE_INFO_ERROR_4,
  INIT_DATA_NOT_FOUND,
  INVALID_CRYPTO_SESSION_1,
  INVALID_CRYPTO_SESSION_2,
  INVALID_CRYPTO_SESSION_3,
  INVALID_CRYPTO_SESSION_4,
  INVALID_CRYPTO_SESSION_5,
  INVALID_DECRYPT_PARAMETERS_ENG_1,
  INVALID_DECRYPT_PARAMETERS_ENG_2,
  INVALID_DECRYPT_PARAMETERS_ENG_3,
  INVALID_DECRYPT_PARAMETERS_ENG_4,
  INVALID_DEVICE_CERTIFICATE_TYPE,
  INVALID_KEY_SYSTEM,
  INVALID_LICENSE_RESPONSE,
  INVALID_LICENSE_TYPE,
  INVALID_PARAMETERS_ENG_1,
  INVALID_PARAMETERS_ENG_2,
  INVALID_PARAMETERS_ENG_3,
  INVALID_PARAMETERS_ENG_4,
  INVALID_PARAMETERS_LIC_1,
  INVALID_PARAMETERS_LIC_2,
  INVALID_PROVISIONING_PARAMETERS_1,
  INVALID_PROVISIONING_PARAMETERS_2,
  INVALID_PROVISIONING_REQUEST_PARAM_1,
  INVALID_PROVISIONING_REQUEST_PARAM_2,
  INVALID_QUERY_KEY,
  INVALID_SESSION_ID,
  KEY_REQUEST_ERROR_1,
  UNUSED_1,  /* previously KEY_REQUEST_ERROR_2 */
  KEY_SIZE_ERROR,
  KEYSET_ID_NOT_FOUND_1,
  KEYSET_ID_NOT_FOUND_2,
  KEYSET_ID_NOT_FOUND_3,
  LICENSE_ID_NOT_FOUND,
  LICENSE_PARSER_INIT_ERROR,
  LICENSE_PARSER_NOT_INITIALIZED_1,
  LICENSE_PARSER_NOT_INITIALIZED_2,
  LICENSE_PARSER_NOT_INITIALIZED_3,
  LICENSE_RESPONSE_NOT_SIGNED,
  LICENSE_RESPONSE_PARSE_ERROR_1,
  LICENSE_RESPONSE_PARSE_ERROR_2,
  LICENSE_RESPONSE_PARSE_ERROR_3,
  LOAD_KEY_ERROR,
  NO_CONTENT_KEY,
  REFRESH_KEYS_ERROR,
  RELEASE_ALL_USAGE_INFO_ERROR_1,
  RELEASE_ALL_USAGE_INFO_ERROR_2,
  RELEASE_KEY_ERROR,
  RELEASE_KEY_REQUEST_ERROR,
  RELEASE_LICENSE_ERROR_1,
  RELEASE_LICENSE_ERROR_2,
  RELEASE_USAGE_INFO_ERROR,
  RENEW_KEY_ERROR_1,
  RENEW_KEY_ERROR_2,
  LICENSE_RENEWAL_SIGNING_ERROR,
  RESTORE_OFFLINE_LICENSE_ERROR_1,
  RESTORE_OFFLINE_LICENSE_ERROR_2,
  SESSION_INIT_ERROR_1,
  SESSION_INIT_ERROR_2,
  SESSION_INIT_GET_KEYBOX_ERROR,
  SESSION_NOT_FOUND_1,
  SESSION_NOT_FOUND_2,
  SESSION_NOT_FOUND_3,
  SESSION_NOT_FOUND_4,
  SESSION_NOT_FOUND_5,
  SESSION_NOT_FOUND_6,
  SESSION_NOT_FOUND_7,
  SESSION_NOT_FOUND_8,
  SESSION_NOT_FOUND_9,
  SESSION_NOT_FOUND_10,
  SESSION_NOT_FOUND_FOR_DECRYPT,
  SESSION_KEYS_NOT_FOUND,
  SIGNATURE_NOT_FOUND,
  STORE_LICENSE_ERROR_1,
  STORE_LICENSE_ERROR_2,
  STORE_LICENSE_ERROR_3,
  STORE_USAGE_INFO_ERROR,
  UNPROVISION_ERROR_1,
  UNPROVISION_ERROR_2,
  UNPROVISION_ERROR_3,
  UNPROVISION_ERROR_4,
  UNSUPPORTED_INIT_DATA,
  USAGE_INFO_NOT_FOUND,
  LICENSE_RENEWAL_SERVICE_CERTIFICATE_GENERATION_ERROR,
  PARSE_SERVICE_CERTIFICATE_ERROR,
  SERVICE_CERTIFICATE_TYPE_ERROR,
  CLIENT_ID_GENERATE_RANDOM_ERROR,
  CLIENT_ID_AES_INIT_ERROR,
  CLIENT_ID_AES_ENCRYPT_ERROR,
  CLIENT_ID_RSA_INIT_ERROR,
  CLIENT_ID_RSA_ENCRYPT_ERROR,
  INVALID_QUERY_STATUS,
  UNUSED_3,  /* previously EMPTY_PROVISIONING_CERTIFICATE_2 on mnc-dev, */
             /* DUPLICATE_SESSION_ID_SPECIFIED on master */
  LICENSE_PARSER_NOT_INITIALIZED_4,
  INVALID_PARAMETERS_LIC_3,
  INVALID_PARAMETERS_LIC_4,
  UNUSED_2,  /* previously INVALID_PARAMETERS_LIC_5 */
  INVALID_PARAMETERS_LIC_6,
  INVALID_PARAMETERS_LIC_7,
  LICENSE_REQUEST_SERVICE_CERTIFICATE_GENERATION_ERROR,
  CENC_INIT_DATA_UNAVAILABLE,
  PREPARE_CENC_CONTENT_ID_FAILED,
  WEBM_INIT_DATA_UNAVAILABLE,
  PREPARE_WEBM_CONTENT_ID_FAILED,
  UNSUPPORTED_INIT_DATA_FORMAT,
  LICENSE_REQUEST_NONCE_GENERATION_ERROR,
  LICENSE_REQUEST_SIGNING_ERROR,
  EMPTY_LICENSE_REQUEST,
  SECURE_BUFFER_REQUIRED,
  DUPLICATE_SESSION_ID_SPECIFIED,
  LICENSE_RENEWAL_PROHIBITED,
  EMPTY_PROVISIONING_CERTIFICATE_2,
  OFFLINE_LICENSE_PROHIBITED,
  STORAGE_PROHIBITED,
  EMPTY_KEYSET_ID_ENG_5,
  SESSION_NOT_FOUND_11,
  LOAD_USAGE_INFO_FILE_ERROR,
  LOAD_USAGE_INFO_MISSING,
};

enum CdmKeyStatus {
  kKeyStatusUsable,
  kKeyStatusExpired,
  kKeyStatusOutputNotAllowed,
  kKeyStatusPending,
  kKeyStatusInternalError,
};
typedef std::map<KeyId, CdmKeyStatus> CdmKeyStatusMap;

#define CORE_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);                    \
  void operator=(const TypeName&)

enum CdmLicenseType {
  kLicenseTypeOffline,
  kLicenseTypeStreaming,
  kLicenseTypeRelease,
  // If the original request was saved to make a service certificate request,
  // use Deferred for the license type in the subsequent request.
  kLicenseTypeDeferred,
  // Like Streaming, but stricter.  Does not permit storage of any kind.
  // Named after the 'temporary' session type in EME, which has this behavior.
  kLicenseTypeTemporary,
};

enum SecurityLevel {
  kLevelDefault,
  kLevel3
};

enum CdmSecurityLevel {
  kSecurityLevelUninitialized,
  kSecurityLevelL1,
  kSecurityLevelL2,
  kSecurityLevelL3,
  kSecurityLevelUnknown
};

enum CdmCertificateType {
  kCertificateWidevine,
  kCertificateX509,
};

struct CdmDecryptionParameters {
  bool is_encrypted;
  bool is_secure;
  const KeyId* key_id;
  const uint8_t* encrypt_buffer;
  size_t encrypt_length;
  const std::vector<uint8_t>* iv;
  size_t block_offset;
  void* decrypt_buffer;
  size_t decrypt_buffer_length;
  size_t decrypt_buffer_offset;
  uint8_t subsample_flags;
  bool is_video;
  CdmDecryptionParameters()
      : is_encrypted(true),
        is_secure(true),
        key_id(NULL),
        encrypt_buffer(NULL),
        encrypt_length(0),
        iv(NULL),
        block_offset(0),
        decrypt_buffer(NULL),
        decrypt_buffer_length(0),
        decrypt_buffer_offset(0),
        subsample_flags(0),
        is_video(true) {}
  CdmDecryptionParameters(const KeyId* key, const uint8_t* encrypted_buffer,
                          size_t encrypted_length,
                          const std::vector<uint8_t>* initialization_vector,
                          size_t offset, void* decrypted_buffer)
      : is_encrypted(true),
        is_secure(true),
        key_id(key),
        encrypt_buffer(encrypted_buffer),
        encrypt_length(encrypted_length),
        iv(initialization_vector),
        block_offset(offset),
        decrypt_buffer(decrypted_buffer),
        decrypt_buffer_length(encrypted_length),
        decrypt_buffer_offset(0),
        subsample_flags(0),
        is_video(true) {}
};

// forward class references
class KeyMessage;
class Request;
class Key;

}  // namespace wvcdm

#endif  // WVCDM_CORE_WV_CDM_TYPES_H_
