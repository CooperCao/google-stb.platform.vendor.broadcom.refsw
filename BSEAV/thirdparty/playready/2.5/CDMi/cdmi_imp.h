/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#ifndef __CDMI_IMP_H__
#define __CDMI_IMP_H__

#include "cdmi.h"

// Note: Remove the following block
// if building from Visual Studio is
// unsupported.
#ifdef VISUAL_STUDIO_BUILD
// DRM_BUILD_PROFILE_OEM
#define DRM_BUILD_PROFILE   10
#include <drmbuild_oem.h>
#endif

#include <drmtypes.h>
#include <drmcommon.h>
#include <drmmanager.h>

#include <nexus_memory.h>

namespace PRCDMi
{

// Defines the state of a MediaKeySession.
enum KeyState
{
    // Has been initialized.
    KEY_INIT = 0,
    // Has a key message pending to be processed.
    KEY_PENDING = 1,
    // Has a usable key.
    KEY_READY = 2,
    // Has an error.
    KEY_ERROR = 3,
    // Has been closed.
    KEY_CLOSED = 4
};

// Class that implements the IMediaKeySession interface.
class CMediaKeySession : public IMediaKeySession
{
public:
    // Constructor.
    CMediaKeySession(void);

    // Destructor.
    virtual ~CMediaKeySession(void);

    // Trigger the license acquisition workflow.
    virtual void Run(
        __in const IMediaKeySessionCallback *f_piMediaKeySessionCallback);

    // Process a license acquisition response.
    virtual void Update(
        __in_bcount(f_cbKeyMessageResponse) const uint8_t *f_pbKeyMessageResponse,
        __in uint32_t f_cbKeyMessageResponse);

    // Shutdown the media key session.
    virtual void Close(void);

    // Return a 16-bytes ID of the current media key session.
    virtual const char *GetSessionId(void) const ;

    // Return the key system string.
    virtual const char *GetKeySystem(void) const;

    // Initialize the current media key session instance.
    CDMi_RESULT Init(
        __in_bcount_opt(f_cbInitData) const uint8_t *f_pbInitData,
        __in uint32_t f_cbInitData,
        __in_bcount_opt(f_cbCDMData) const uint8_t *f_pbCDMData,
        __in uint32_t f_cbCDMData);

    // Decrypt is not an IMediaKeySession interface method therefore it can only be
    // accessed from code that has internal knowledge of CMediaKeySession.
    CDMi_RESULT Decrypt(
        __in_bcount(f_cbSessionKey) const DRM_BYTE *f_pbSessionKey,
        __in DRM_DWORD f_cbSessionKey,
        __in_bcount(f_cbIV) const uint8_t *f_pbIV,
        __in uint32_t f_cbIV,
        __inout_bcount(f_cbData) uint8_t *f_pbData,
        __in uint32_t f_cbData,
        __in_ecount_opt(f_cdwSubSampleMapping) const uint32_t *f_pdwSubSampleMapping,
        __in uint32_t f_cdwSubSampleMapping);

private:
    // Retrieve PRO from init data.
    CDMi_RESULT _GetPROFromInitData(
        __in_bcount(f_cbInitData) const DRM_BYTE *f_pbInitData,
        __in DRM_DWORD f_cbInitData,
        __out DRM_DWORD *f_pibPRO,
        __out DRM_DWORD *f_pcbPRO);

    // Parse init data to retrieve PRO from it.
    CDMi_RESULT _ParseInitData(
        __in_bcount(f_cbInitData) const uint8_t *f_pbInitData,
        __in uint32_t f_cbInitData);

    // Parse CDM data to retrieve key ID and custom data (if exists).
    CDMi_RESULT _ParseCDMData(
        __in_bcount(f_cbCDMData) const uint8_t *f_pbCDMData,
        __in uint32_t f_cbCDMData);

    // Build a key message using the supplied license challenge.
    CDMi_RESULT _BuildKeyMessage(
        __in_bcount(f_cbChallenge) const DRM_BYTE *f_pbChallenge,
        __in DRM_DWORD f_cbChallenge,
        __deref_out_bcount(*f_pcbKeyMessage) DRM_BYTE **f_ppbKeyMessage,
        __out DRM_DWORD *f_pcbKeyMessage);

    // Encrypt a block of data in place using the supplied AES session key.
    CDMi_RESULT _EncryptDataUsingSessionKey(
        __in_bcount(f_cbSessionKey) const DRM_BYTE *f_pbSessionKey,
        __in DRM_DWORD f_cbSessionKey,
        __inout_bcount(f_cbData) uint8_t *f_pbData,
        __in uint32_t f_cbData);

    // Map PlayReady specific CDMi error to one of the EME errors.
    int16_t _MapCDMiError(
        __in CDMi_RESULT f_crError);

    static DRM_RESULT DRM_CALL _PolicyCallback(
        __in const DRM_VOID *f_pvOutputLevelsData,
        __in DRM_POLICY_CALLBACK_TYPE f_dwCallbackType,
        __in const DRM_VOID *f_pv);

    bool convertCStringToWString(const char * pCStr, DRM_WCHAR * pWStr, DRM_DWORD * cchStr);
    bool load_revocation_list(const char *revListFile);

private:
    DRM_APP_CONTEXT *m_poAppContext;
    DRM_DECRYPT_CONTEXT m_oDecryptContext;
    DRM_BYTE *m_pbOpaqueBuffer;
    DRM_DWORD m_cbOpaqueBuffer;

    DRM_BYTE *m_pbRevocationBuffer;

    DRM_BYTE *m_pbPRO;
    DRM_DWORD m_cbPRO;

    DRM_ID m_oKeyId;
    DRM_BOOL m_fKeyIdSet;

    DRM_CHAR *m_pchCustomData;
    DRM_DWORD m_cchCustomData;

    IMediaKeySessionCallback *m_piCallback;

    KeyState m_eKeyState;

    DRM_CHAR m_rgchSessionID[CCH_BASE64_EQUIV(SIZEOF(DRM_ID)) + 1];

    DRM_BOOL m_fCommit;
    DRM_VOID *m_pOEMContext;
    NEXUS_MemoryAllocationSettings m_heapSettings;
};

// Class that implements the IMediaKeys interface.
class CMediaKeys : public IMediaKeys
{
public:
    // Constructor.
    CMediaKeys(void);

    // Destructor.
    virtual ~CMediaKeys(void);

    // Return whether a key system and mime type (optional) is supported.
    virtual bool IsTypeSupported(
        __in_z_opt const char *f_pwszMimeType,
        __in_z const char *f_pwszKeySystem) const;

    // Factory method that creates a media key session using the supplied
    // init data and CDM data (both are optional).
    virtual CDMi_RESULT CreateMediaKeySession(
        __in_z_opt const char *f_pwszMimeType,
        __in_bcount_opt(f_cbInitData) const uint8_t *f_pbInitData,
        __in uint32_t f_cbInitData,
        __in_bcount_opt(f_cbCDMData) const uint8_t *f_pbCDMData,
        __in uint32_t f_cbCDMData,
        __deref_out IMediaKeySession **f_ppiMediaKeySession);

    // Close a media key session interface and frees
    // all resources associated with it.
    virtual CDMi_RESULT DestroyMediaKeySession(
        __in IMediaKeySession *f_piMediaKeySession);
};

// Class that implements the IMediaEngineSession interface.
class CMediaEngineSession : public IMediaEngineSession
{
public:
    // Constructor.
    CMediaEngineSession(void);

    // Destructor.
    virtual ~CMediaEngineSession(void);

    // Decrypt a block of data using supplied IV and the optional
    // subsample mapping data. The decrypted data is immediately
    // re-encrypted using the cached session key before being returned
    // to the caller.
    virtual CDMi_RESULT Decrypt(
        __in_bcount(f_cbIV) const uint8_t *f_pbIV,
        __in uint32_t f_cbIV,
        __inout_bcount(f_cbData) uint8_t *f_pbData,
        __in uint32_t f_cbData,
        __in_ecount_opt(f_cdwSubSampleMapping) const uint32_t *f_pdwSubSampleMapping,
        __in uint32_t f_cdwSubSampleMapping);

    // Initialize the current media engine session instance.
    CDMi_RESULT Init(
        __in const IMediaKeySession *f_piMediaKeySession,
        __in_bcount(f_cbSessionKey) const DRM_BYTE *f_pbSessionKey,
        __in DRM_DWORD f_cbSessionKey);

private:
    IMediaKeySession *m_piMediaKeySession;

    DRM_BYTE *m_pbSessionKey;
    DRM_DWORD m_cbSessionKey;
};

} // namespace PRCDMi

#endif  // __CDMI_IMP_H__
