/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#ifndef __CDMI_H__
#define __CDMI_H__

// For the support of portable data types such as uint8_t.
#include <stdint.h>
#include "drmtypes.h"

namespace PRCDMi
{

// EME error code to which CDMi errors are mapped. Please
// refer to the EME spec for details of the errors
// https://dvcs.w3.org/hg/html-media/raw-file/tip/encrypted-media/encrypted-media.html
#define MEDIA_KEYERR_UNKNOWN        1
#define MEDIA_KEYERR_CLIENT         2
#define MEDIA_KEYERR_SERVICE        3
#define MEDIA_KEYERR_OUTPUT         4
#define MEDIA_KEYERR_HARDWARECHANGE 5
#define MEDIA_KEYERR_DOMAIN         6

// The status code returned by CDMi APIs.
typedef int32_t CDMi_RESULT;

#define CDMi_SUCCESS            ((CDMi_RESULT)0)
#define CDMi_S_FALSE            ((CDMi_RESULT)1)
#define CDMi_E_OUT_OF_MEMORY    ((CDMi_RESULT)0x80000002)
#define CDMi_E_FAIL             ((CDMi_RESULT)0x80004005)
#define CDMi_E_INVALID_ARG      ((CDMi_RESULT)0x80070057)

#define CDMi_E_SERVER_INTERNAL_ERROR    ((CDMi_RESULT)0x8004C600)
#define CDMi_E_SERVER_INVALID_MESSAGE   ((CDMi_RESULT)0x8004C601)
#define CDMi_E_SERVER_SERVICE_SPECIFIC  ((CDMi_RESULT)0x8004C604)

// More CDMi status codes can be defined. In general
// CDMi status codes should use the same PK error codes.

#define CDMi_FAILED(Status)     ((CDMi_RESULT)(Status)<0)
#define CDMi_SUCCEEDED(Status)  ((CDMi_RESULT)(Status) >= 0)

// IMediaKeySessionCallback defines the callback interface to receive
// events originated from MediaKeySession.
class IMediaKeySessionCallback
{
public:
    virtual ~IMediaKeySessionCallback(void) {}

    // Event fired when a key message is successfully created.
    virtual void OnKeyMessage(
        __in_bcount(f_cbKeyMessage) const uint8_t *f_pbKeyMessage,
        __in uint32_t f_cbKeyMessage,
        __in_z_opt char *f_pszUrl) = 0;

    // Event fired when MediaKeySession has found a usable key.
    virtual void OnKeyReady(void) = 0;

    // Event fired when MediaKeySession encounters an error.
    virtual void OnKeyError(
        __in int16_t f_nError,
        __in CDMi_RESULT f_crSysError) = 0;
};

// IMediaKeySession defines the MediaKeySession interface.
class IMediaKeySession
{
public:
    virtual ~IMediaKeySession(void) {}

    // Kicks off the process of acquiring a key. A MediaKeySession callback is supplied
    // to receive notifications during the process.
    virtual void Run(
        __in const IMediaKeySessionCallback *f_piMediaKeySessionCallback) = 0;

    // Process a key message response.
    virtual void Update(
        __in_bcount(f_cbKeyMessageResponse) const uint8_t *f_pbKeyMessageResponse,
        __in uint32_t f_cbKeyMessageResponse) = 0;

    // Explicitly release all resources associated with the MediaKeySession.
    virtual void Close(void) = 0;

    // Return the session ID of the MediaKeySession. The returned pointer
    // is valid as long as the associated MediaKeySession still exists.
    virtual const char *GetSessionId(void) const = 0;

    // Return the key system of the MediaKeySession.
    virtual const char *GetKeySystem(void) const = 0;
};

// IMediaKeys defines the MediaKeys interface.
class IMediaKeys
{
public:
    virtual ~IMediaKeys(void) {}

    // Check whether the MediaKey supports a specific mime type (optional)
    // and a key system.
    virtual bool IsTypeSupported(
        __in_z_opt const char *f_pszMimeType,
        __in_z const char *f_pszKeySystem) const = 0;

    // Create a MediaKeySession using the supplied init data and CDM data.
    // If the returned media key session interface is not needed then it must
    // be released via the call of IMediaKeys::DestroyMediaKeySession.
    virtual CDMi_RESULT CreateMediaKeySession(
        __in_z_opt const char *f_pszMimeType,
        __in_bcount_opt(cbInitData) const uint8_t *f_pbInitData,
        __in uint32_t f_cbInitData,
        __in_bcount_opt(f_cbCDMData) const uint8_t *f_pbCDMData,
        __in uint32_t f_cbCDMData,
        __deref_out IMediaKeySession **f_ppiMediaKeySession) = 0;

    // Destroy a MediaKeySession instance.
    virtual CDMi_RESULT DestroyMediaKeySession(
        __in IMediaKeySession *f_piMediaKeySession) = 0;
};

// Global factory method that creates a MediaKeys instance.
// If the returned media keys interface is not needed then it must be released
// via the call of DestroyMediaKeys.
CDMi_RESULT CreateMediaKeys(
    __deref_out IMediaKeys **f_ppiMediaKeys);

// Global method that destroys a MediaKeys instance.
CDMi_RESULT DestroyMediaKeys(
    __in IMediaKeys *f_piMediaKeys);

// IMediaEngineSession represents a secure channel between media engine and CDMi
// for the purpose of sample decryption.
//
// Note: This interface below is at the prototype stage and it will undergo further design
// review to be finalized.
class IMediaEngineSession
{
public:

    virtual ~IMediaEngineSession(void) {}

    // Decrypt a buffer in-place and internally immediately re-encrypt it using a
    // session key. In other words the returned buffer can only be encrypted by
    // a party that knows the session key.
    virtual CDMi_RESULT Decrypt(
        __in_bcount(f_cbIV) const uint8_t *f_pbIV,
        __in uint32_t f_cbIV,
        __inout_bcount(f_cbData) uint8_t *f_pbData,
        __in uint32_t f_cbData,
        __in_ecount_opt(f_cdwSubSampleMapping) const uint32_t *f_pdwSubSampleMapping,
        __in uint32_t f_cdwSubSampleMapping) = 0;
};

// Global factory method that creates a MediaEngineSession instance.
// pbCert/cbCert is a sample protection certificate chain from a trusted authority.
// After the certificate chain is validated a random session key is generated and then
// encrypted using the public key extracted from the leaf certificate of the sample
// protection certificate chain before sending back to the caller. The session key is
// later used by media engine to decrypt samples that are re-encrypted by the call of
// IMediaEngineSession::decrypt.
//
// If the returned media engine session interface is not needed then it must be released
// via the call of DestroyMediaEngineSession. The returned encrypted session key should
// be released via delete[] if it is not used.
//
// Note: This interface below is at the prototype stage and it will undergo further design
// review to be finalized.
CDMi_RESULT CreateMediaEngineSession(
    __in const IMediaKeySession *f_piMediaKeySession,
    __in_bcount(f_cbCertChain) const uint8_t *f_pbCertChain,
    __in uint32_t f_cbCertChain,
    __deref_out_bcount(*f_pcbSessionKey) uint8_t **f_ppbEncryptedSessionKey,
    __out uint32_t *f_pcbEncryptedSessionKey,
    __deref_out IMediaEngineSession **f_ppiMediaEngineSession);

// Global method that destroys a MediaEngineSession instance.
CDMi_RESULT DestroyMediaEngineSession(
    __in IMediaEngineSession *f_piMediaEngineSession);

} // namespace PRCDMi

#endif  // __CDMI_H__
