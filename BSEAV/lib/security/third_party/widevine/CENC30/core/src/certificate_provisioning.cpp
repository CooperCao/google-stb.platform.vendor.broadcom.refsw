// Copyright 2013 Google Inc. All Rights Reserved.

#include "certificate_provisioning.h"
#include "device_files.h"
#include "file_store.h"
#include "license_protocol.pb.h"
#include "log.h"
#include "string_conversions.h"
#include "wv_cdm_constants.h"

namespace {

// URL for Google Provisioning Server.
// The provisioning server supplies the certificate that is needed
// to communicate with the License Server.
const std::string kProvisioningServerUrl =
    "https://www.googleapis.com/"
    "certificateprovisioning/v1/devicecertificates/create"
    "?key=AIzaSyB-5OLKTx2iU5mko18DfdwK5611JIjbUhE";
}

namespace wvcdm {
// Protobuf generated classes.
using video_widevine_server::sdk::ClientIdentification;
using video_widevine_server::sdk::ProvisioningOptions;
using video_widevine_server::sdk::ProvisioningRequest;
using video_widevine_server::sdk::ProvisioningResponse;
using video_widevine_server::sdk::SignedProvisioningMessage;

/*
 * This function converts SignedProvisioningRequest into base64 string. It then
 * wraps it in JSON format expected by the frontend. This server requires a
 * "web-safe" base 64 encoding, where '+' becomes '-' and '/' becomes '_'.
 *
 * Returns the JSON formated string in *request. The JSON string will be
 * appended as a query parameter, i.e. signedRequest=<base 64 encoded
 * SignedProvisioningRequest>. All base64 '=' padding chars must be removed.
 *
 * The JSON formated request takes the following format:
 *
 * base64 encoded message
 */
void CertificateProvisioning::ComposeJsonRequestAsQueryString(
    const std::string& message, CdmProvisioningRequest* request) {
  // Performs base64 encoding for message
  std::vector<uint8_t> message_vector(message.begin(), message.end());
  std::string message_b64 = Base64SafeEncodeNoPad(message_vector);
  request->assign(message_b64);
}

/*
 * Composes a device provisioning request and output the request in JSON format
 * in *request. It also returns the default url for the provisioning server
 * in *default_url.
 *
 * Returns NO_ERROR for success and CERT_PROVISIONING_REQUEST_ERROR_? if fails.
 */
CdmResponseType CertificateProvisioning::GetProvisioningRequest(
    SecurityLevel requested_security_level, CdmCertificateType cert_type,
    const std::string& cert_authority, const std::string& origin,
    CdmProvisioningRequest* request, std::string* default_url) {
  if (!default_url) {
    LOGE("GetProvisioningRequest: pointer for returning URL is NULL");
    return CERT_PROVISIONING_REQUEST_ERROR_1;
  }

  default_url->assign(kProvisioningServerUrl);

  CdmResponseType sts = crypto_session_.Open(requested_security_level);
  if (NO_ERROR != sts) {
    LOGE("GetProvisioningRequest: fails to create a crypto session");
    return sts;
  }

  // Prepares device provisioning request.
  ProvisioningRequest provisioning_request;
  ClientIdentification* client_id = provisioning_request.mutable_client_id();
  client_id->set_type(ClientIdentification::KEYBOX);
  std::string token;
  if (!crypto_session_.GetToken(&token)) {
    LOGE("GetProvisioningRequest: fails to get token");
    return CERT_PROVISIONING_GET_KEYBOX_ERROR_1;
  }
  client_id->set_token(token);

  uint32_t nonce;
  if (!crypto_session_.GenerateNonce(&nonce)) {
    LOGE("GetProvisioningRequest: fails to generate a nonce");
    return CERT_PROVISIONING_REQUEST_ERROR_2;
  }

  // The provisioning server does not convert the nonce to uint32_t, it just
  // passes the binary data to the response message.
  std::string the_nonce(reinterpret_cast<char*>(&nonce), sizeof(nonce));
  provisioning_request.set_nonce(the_nonce);

  ProvisioningOptions* options = provisioning_request.mutable_options();
  switch (cert_type) {
    case kCertificateWidevine:
      options->set_certificate_type(
          video_widevine_server::sdk::
              ProvisioningOptions_CertificateType_WIDEVINE_DRM);
      break;
    case kCertificateX509:
      options->set_certificate_type(
          video_widevine_server::sdk::ProvisioningOptions_CertificateType_X509);
      break;
    default:
      LOGE("GetProvisioningRequest: unknown certificate type %ld", cert_type);
      return CERT_PROVISIONING_INVALID_CERT_TYPE;
  }

  cert_type_ = cert_type;
  options->set_certificate_authority(cert_authority);

  if (origin != EMPTY_ORIGIN) {
    std::string device_unique_id;
    if (!crypto_session_.GetDeviceUniqueId(&device_unique_id)) {
      LOGE("GetProvisioningRequest: fails to get device unique ID");
      return CERT_PROVISIONING_GET_KEYBOX_ERROR_2;
    }
    provisioning_request.set_stable_id(device_unique_id + origin);
  }

  std::string serialized_message;
  provisioning_request.SerializeToString(&serialized_message);

  // Derives signing and encryption keys and constructs signature.
  std::string request_signature;
  if (!crypto_session_.PrepareRequest(serialized_message, true,
                                      &request_signature)) {
    LOGE("GetProvisioningRequest: fails to prepare request");
    return CERT_PROVISIONING_REQUEST_ERROR_3;
  }
  if (request_signature.empty()) {
    LOGE("GetProvisioningRequest: request signature is empty");
    return CERT_PROVISIONING_REQUEST_ERROR_4;
  }

  SignedProvisioningMessage signed_provisioning_msg;
  signed_provisioning_msg.set_message(serialized_message);
  signed_provisioning_msg.set_signature(request_signature);

  std::string serialized_request;
  signed_provisioning_msg.SerializeToString(&serialized_request);

  // Converts request into JSON string
  ComposeJsonRequestAsQueryString(serialized_request, request);
  return NO_ERROR;
}

/*
 * Parses the input json_str and locates substring using start_substr and
 * end_stubstr. The found base64 substring is then decoded and returns
 * in *result.
 *
 * Returns true for success and false if fails.
 */
bool CertificateProvisioning::ParseJsonResponse(
    const CdmProvisioningResponse& json_str, const std::string& start_substr,
    const std::string& end_substr, std::string* result) {
  std::string b64_string;
  size_t start = json_str.find(start_substr);
  if (start == json_str.npos) {
    LOGE("ParseJsonResponse: cannot find start substring");
    return false;
  }
  size_t end = json_str.find(end_substr, start + start_substr.length());
  if (end == json_str.npos) {
    LOGE("ParseJsonResponse cannot locate end substring");
    return false;
  }

  size_t b64_string_size = end - start - start_substr.length();
  b64_string.assign(json_str, start + start_substr.length(), b64_string_size);

  // Decodes base64 substring and returns it in *result
  std::vector<uint8_t> result_vector = Base64SafeDecode(b64_string);
  result->assign(result_vector.begin(), result_vector.end());

  return true;
}

/*
 * The response message consists of a device certificate and the device RSA key.
 * The device RSA key is stored in the T.E.E. The device certificate is stored
 * in the device.
 *
 * Returns NO_ERROR for success and CERT_PROVISIONING_RESPONSE_ERROR_? if fails.
 */
CdmResponseType CertificateProvisioning::HandleProvisioningResponse(
    const std::string& origin, const CdmProvisioningResponse& response,
    std::string* cert, std::string* wrapped_key) {
  // Extracts signed response from JSON string, decodes base64 signed response
  const std::string kMessageStart = "\"signedResponse\": \"";
  const std::string kMessageEnd = "\"";
  std::string serialized_signed_response;
  if (!ParseJsonResponse(response, kMessageStart, kMessageEnd,
                         &serialized_signed_response)) {
    LOGE("Fails to extract signed serialized response from JSON response");
    return CERT_PROVISIONING_RESPONSE_ERROR_1;
  }

  // Authenticates provisioning response using D1s (server key derived from
  // the provisioing request's input). Validate provisioning response and
  // stores private device RSA key and certificate.
  SignedProvisioningMessage signed_response;
  if (!signed_response.ParseFromString(serialized_signed_response)) {
    LOGE("HandleProvisioningResponse: fails to parse signed response");
    return CERT_PROVISIONING_RESPONSE_ERROR_2;
  }

  bool error = false;
  if (!signed_response.has_signature()) {
    LOGE("HandleProvisioningResponse: signature not found");
    error = true;
  }

  if (!signed_response.has_message()) {
    LOGE("HandleProvisioningResponse: message not found");
    error = true;
  }

  if (error) return CERT_PROVISIONING_RESPONSE_ERROR_3;

  const std::string& signed_message = signed_response.message();
  ProvisioningResponse provisioning_response;

  if (!provisioning_response.ParseFromString(signed_message)) {
    LOGE("HandleProvisioningResponse: Fails to parse signed message");
    return CERT_PROVISIONING_RESPONSE_ERROR_4;
  }

  if (!provisioning_response.has_device_rsa_key()) {
    LOGE("HandleProvisioningResponse: key not found");
    return CERT_PROVISIONING_RESPONSE_ERROR_5;
  }

  const std::string& enc_rsa_key = provisioning_response.device_rsa_key();
  const std::string& nonce = provisioning_response.nonce();
  const std::string& rsa_key_iv = provisioning_response.device_rsa_key_iv();
  const std::string& signature = signed_response.signature();
  std::string wrapped_rsa_key;
  if (!crypto_session_.RewrapDeviceRSAKey(signed_message, signature, nonce,
                                          enc_rsa_key, rsa_key_iv,
                                          &wrapped_rsa_key)) {
    LOGE("HandleProvisioningResponse: RewrapDeviceRSAKey fails");
    return CERT_PROVISIONING_RESPONSE_ERROR_6;
  }

  crypto_session_.Close();

  if (cert_type_ == kCertificateX509) {
    *cert = provisioning_response.device_certificate();
    *wrapped_key = wrapped_rsa_key;
    return NO_ERROR;
  }

  const std::string& device_certificate =
      provisioning_response.device_certificate();

  DeviceFiles handle;
  if (!handle.Init(crypto_session_.GetSecurityLevel())) {
    LOGE("HandleProvisioningResponse: failed to init DeviceFiles");
    return CERT_PROVISIONING_RESPONSE_ERROR_7;
  }
  if (!handle.StoreCertificate(origin, device_certificate, wrapped_rsa_key)) {
    LOGE("HandleProvisioningResponse: failed to save provisioning certificate");
    return CERT_PROVISIONING_RESPONSE_ERROR_8;
  }

  return NO_ERROR;
}

}  // namespace wvcdm
