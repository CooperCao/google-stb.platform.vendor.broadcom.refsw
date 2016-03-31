// Copyright 2013 Google Inc. All Rights Reserved.

#ifndef WVCDM_CORE_CDM_ENGINE_H_
#define WVCDM_CORE_CDM_ENGINE_H_

#include <string>

#include "certificate_provisioning.h"
#include "crypto_session.h"
#include "initialization_data.h"
#include "lock.h"
#include "oemcrypto_adapter.h"
#include "scoped_ptr.h"
#include "wv_cdm_types.h"

namespace wvcdm {

class CdmClientPropertySet;
class CdmSession;
class CryptoEngine;
class UsagePropertySet;
class WvCdmEventListener;

typedef std::map<CdmSessionId, CdmSession*> CdmSessionMap;
typedef std::map<CdmKeySetId, CdmSessionId> CdmReleaseKeySetMap;

class CdmEngine {
 public:
  CdmEngine();
  virtual ~CdmEngine();

  // Session related methods
  virtual CdmResponseType OpenSession(const CdmKeySystem& key_system,
                                      CdmClientPropertySet* property_set,
                                      const std::string& origin,
                                      WvCdmEventListener* event_listener,
                                      const CdmSessionId* forced_session_id,
                                      CdmSessionId* session_id);
  virtual CdmResponseType CloseSession(const CdmSessionId& session_id);
  virtual bool IsOpenSession(const CdmSessionId& session_id);

  virtual CdmResponseType OpenKeySetSession(
      const CdmKeySetId& key_set_id, CdmClientPropertySet* property_set,
      const std::string& origin, WvCdmEventListener* event_listener);
  virtual CdmResponseType CloseKeySetSession(const CdmKeySetId& key_set_id);

  // License related methods

  // Construct a valid license request. The arguments are used as follows:
  // session_id: The Session ID of the session the request is being generated
  //             for. This is ignored for license release requests.
  // key_set_id: The Key Set ID of the key set the request is being generated
  //             for. This is ignored except for license release requests.
  // init_data: The initialization data from the media file, which is used to
  //            build the key request. This is ignored for release and renewal
  //            requests.
  // license_type: The type of license being requested. Never ignored.
  // app_parameters: Additional, application-specific parameters that factor
  //                 into the request generation. This is ignored for release
  //                 and renewal requests.
  // key_request: This must be non-null and point to a CdmKeyMessage. The buffer
  //              will have its contents replaced with the key request.
  // key_request_type: May be null. If it is non-null, it will be filled with
  //                   key request type, whether it is an initial request,
  //                   renewal request or release request etc.
  // server_url: This must be non-null and point to a string. The string will
  //             have its contents replaced with the default URL (if one is
  //             known) to send this key request to.
  // key_set_id_out: May be null. If it is non-null, the CdmKeySetId pointed to
  //                 will have its contents replaced with the key set ID of the
  //                 session. Note that for non-offline license requests, the
  //                 key set ID is empty, so the CdmKeySetId will be cleared.
  // TODO(kqyang): Consider refactor GenerateKeyRequest to reduce the number of
  // parameters.
  virtual CdmResponseType GenerateKeyRequest(
      const CdmSessionId& session_id, const CdmKeySetId& key_set_id,
      const InitializationData& init_data, const CdmLicenseType license_type,
      CdmAppParameterMap& app_parameters, CdmKeyMessage* key_request,
      CdmKeyRequestType* key_request_type, std::string* server_url,
      CdmKeySetId* key_set_id_out);

  // Accept license response and extract key info.
  virtual CdmResponseType AddKey(const CdmSessionId& session_id,
                                 const CdmKeyResponse& key_data,
                                 CdmKeySetId* key_set_id);

  virtual CdmResponseType RestoreKey(const CdmSessionId& session_id,
                                     const CdmKeySetId& key_set_id);

  virtual CdmResponseType RemoveKeys(const CdmSessionId& session_id);

  // Construct valid renewal request for the current session keys.
  virtual CdmResponseType GenerateRenewalRequest(const CdmSessionId& session_id,
                                                 CdmKeyMessage* key_request,
                                                 std::string* server_url);

  // Accept renewal response and update key info.
  virtual CdmResponseType RenewKey(const CdmSessionId& session_id,
                                   const CdmKeyResponse& key_data);

  // Query system information
  virtual CdmResponseType QueryStatus(SecurityLevel security_level,
                                      const std::string& key,
                                      std::string* value);

  // Query session information
  virtual CdmResponseType QuerySessionStatus(const CdmSessionId& session_id,
                                             CdmQueryMap* key_info);
  virtual bool IsReleaseSession(const CdmSessionId& session_id);
  virtual bool IsOfflineSession(const CdmSessionId& session_id);

  // Query license information
  virtual CdmResponseType QueryKeyStatus(const CdmSessionId& session_id,
                                         CdmQueryMap* key_info);

  // Query session control information
  virtual CdmResponseType QueryKeyControlInfo(const CdmSessionId& session_id,
                                              CdmQueryMap* key_info);

  // Provisioning related methods
  virtual CdmResponseType GetProvisioningRequest(
      CdmCertificateType cert_type, const std::string& cert_authority,
      const std::string& origin, CdmProvisioningRequest* request,
      std::string* default_url);

  virtual CdmResponseType HandleProvisioningResponse(
      const std::string& origin, const CdmProvisioningResponse& response,
      std::string* cert, std::string* wrapped_key);

  virtual bool IsProvisioned(CdmSecurityLevel security_level,
                             const std::string& origin);

  virtual CdmResponseType Unprovision(CdmSecurityLevel security_level,
                                      const std::string& origin);

  // Usage related methods for streaming licenses
  // Retrieve a random usage info from the list of all usage infos for this app
  // id.
  virtual CdmResponseType GetUsageInfo(const std::string& app_id,
                                       CdmUsageInfo* usage_info);
  // Retrieve the usage info for the specified pst.
  // Returns UNKNOWN_ERROR if no usage info was found.
  virtual CdmResponseType GetUsageInfo(const std::string& app_id,
                                       const CdmSecureStopId& ssid,
                                       CdmUsageInfo* usage_info);
  virtual CdmResponseType ReleaseAllUsageInfo(const std::string& app_id);
  virtual CdmResponseType ReleaseUsageInfo(
      const CdmUsageInfoReleaseMessage& message);
  virtual CdmResponseType LoadUsageSession(const CdmKeySetId& key_set_id,
                                           CdmKeyMessage* release_message);

  // Decryption and key related methods
  // Accept encrypted buffer and return decrypted data.
  virtual CdmResponseType Decrypt(const CdmSessionId& session_id,
                                  const CdmDecryptionParameters& parameters);

  virtual size_t SessionSize() const { return sessions_.size(); }

  // Is the key known to any session?
  virtual bool IsKeyLoaded(const KeyId& key_id);
  virtual bool FindSessionForKey(const KeyId& key_id, CdmSessionId* sessionId);

  // Used for notifying the Max-Res Engine of resolution changes
  virtual void NotifyResolution(const CdmSessionId& session_id, uint32_t width,
                                uint32_t height);

  // Timer expiration method. This method is not re-entrant -- there can be
  // only one timer.
  // This method triggers appropriate event callbacks from |event_listener_|,
  // which is assumed to be asynchronous -- i.e. an event should be dispatched
  // to another thread which does the actual work. In particular, if a
  // synchronous listener calls OpenSession or CloseSession, the thread will
  // dead lock.
  virtual void OnTimerEvent();

 private:
  // private methods
  void DeleteAllUsageReportsUponFactoryReset();
  bool ValidateKeySystem(const CdmKeySystem& key_system);
  CdmResponseType GetUsageInfo(const std::string& app_id,
                               SecurityLevel requested_security_level,
                               CdmUsageInfo* usage_info);

  void OnKeyReleaseEvent(const CdmKeySetId& key_set_id);

  std::string MapHdcpVersion(CryptoSession::HdcpCapability version);

  // instance variables
  CdmSessionMap sessions_;
  CdmReleaseKeySetMap release_key_sets_;
  scoped_ptr<CertificateProvisioning> cert_provisioning_;
  SecurityLevel cert_provisioning_requested_security_level_;

  static bool seeded_;

  // usage related variables
  scoped_ptr<CdmSession> usage_session_;
  scoped_ptr<UsagePropertySet> usage_property_set_;
  int64_t last_usage_information_update_time_;

  // Locks the list of sessions, |sessions_|, for the event timer.  It will be
  // locked in OpenSession, CloseSession. It is also locked in OnTimerEvent and
  // OnKeyReleaseEvent while the list of event listeners is being generated.
  // The layer above the CDM implementation is expected to handle thread
  // synchronization to make sure other functions that access sessions do not
  // occur simultaneously with OpenSession or CloseSession.
  Lock session_list_lock_;

  CORE_DISALLOW_COPY_AND_ASSIGN(CdmEngine);
};

}  // namespace wvcdm

#endif  // WVCDM_CORE_CDM_ENGINE_H_
