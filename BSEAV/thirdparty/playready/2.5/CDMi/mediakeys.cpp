/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#include "cdmi_imp.h"

#include <string.h>

namespace PRCDMi
{

extern const char *g_pszPLAYREADY_KEYSYSTEM = "com.microsoft.playready";

// Constructor
CMediaKeys::CMediaKeys(void)
{
}

// Destructor
CMediaKeys::~CMediaKeys(void)
{
}

// Return whether a key system and mime type (optional) is supported.
bool CMediaKeys::IsTypeSupported(
    __in_z_opt const char *f_pszMimeType,
    __in_z const char *f_pszKeySystem) const
{
    UNREFERENCED_PARAMETER(f_pszMimeType);

    bool isSupported = false;
    if (f_pszKeySystem != NULL)
    {
        isSupported = strcmp(g_pszPLAYREADY_KEYSYSTEM, f_pszKeySystem) == 0;
    }
    return isSupported;
}

// Factory method that creates a media key session using the supplied
// init data and CDM data (both are optional).
// If the returned media key session interface is not needed then
// it must be released via the call of IMediaKeys::DestroyMediaKeySession.
CDMi_RESULT CMediaKeys::CreateMediaKeySession(
    __in_z_opt const char *f_pszMimeType,
    __in_bcount_opt(f_cbInitData) const uint8_t *f_pbInitData,
    __in uint32_t f_cbInitData,
    __in_bcount_opt(f_cbCDMData) const uint8_t *f_pbCDMData,
    __in uint32_t f_cbCDMData,
    __deref_out IMediaKeySession **f_ppiMediaKeySession)
{
    UNREFERENCED_PARAMETER(f_pszMimeType);

    DRM_RESULT dr = DRM_SUCCESS;
    CMediaKeySession *poMediaKeySession = NULL;

    ChkArg((f_pbInitData == NULL) == (f_cbInitData == 0));
    ChkArg((f_pbCDMData == NULL) == (f_cbCDMData == 0));
    ChkArg(f_ppiMediaKeySession != NULL);

    *f_ppiMediaKeySession = NULL;

    ChkMem(poMediaKeySession = new CMediaKeySession());

    ChkDR(poMediaKeySession->Init(f_pbInitData,
                                  f_cbInitData,
                                  f_pbCDMData,
                                  f_cbCDMData));

    *f_ppiMediaKeySession = static_cast<IMediaKeySession *>(poMediaKeySession);

ErrorExit:
    if (DRM_FAILED(dr))
    {
        delete poMediaKeySession;
    }
    return dr;
}

// Close a media key session interface and frees
// all resources associated with it.
CDMi_RESULT CMediaKeys::DestroyMediaKeySession(
    __in IMediaKeySession *f_piMediaKeySession)
{
    DRM_RESULT dr = DRM_SUCCESS;

    ChkArg(f_piMediaKeySession != NULL);

    delete f_piMediaKeySession;

ErrorExit:
    return dr;
}

// Factory method that creates a media keys.
// If the returned media keys interface is not needed then
// it must be released via the call of DestroyMediaKeys.
CDMi_RESULT CreateMediaKeys(
    __deref_out IMediaKeys **f_ppiMediaKeys)
{
    DRM_RESULT dr = DRM_SUCCESS;
    CMediaKeys *pMediaKeys = NULL;

    ChkArg(f_ppiMediaKeys != NULL);

    *f_ppiMediaKeys = NULL;

    ChkMem(pMediaKeys = new CMediaKeys());

    *f_ppiMediaKeys = static_cast<IMediaKeys *>(pMediaKeys);

ErrorExit:
    return dr;
}

// Close a media keys interface and frees
// all resources associated with it.
CDMi_RESULT DestroyMediaKeys(
    __in IMediaKeys *f_piMediaKeys)
{
    DRM_RESULT dr = DRM_SUCCESS;

    ChkArg(f_piMediaKeys != NULL);

    delete f_piMediaKeys;

ErrorExit:
    return dr;
}

} // namespace PRCDMi
