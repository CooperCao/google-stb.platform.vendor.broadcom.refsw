/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#ifndef __DRMCDMI_H__
#define __DRMCDMI_H__

#include <drmcdmitypes.h>
#include <drmmodulesupport.h>

ENTER_PK_NAMESPACE

extern DRM_GLOBAL_CONST DRM_CHAR g_rgchPlayReadyCDMSystemID[];

/*
** Helper methods
*/
DRM_NO_INLINE DRM_API_VOID DRM_CDMI_EXCEPTION DRM_CALL DRM_CDMI_MapErrorCodeToExceptionType(
    __in                                                  DRM_RESULT                              f_dr );

/*
** MediaKeySystemAccess methods
*/
DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMI_CreateMediaKeySystemAccess(
    __in_z                                                  const DRM_CHAR                               *f_pszKeySystem,
    __inout                                                       DRM_DWORD                              *f_pcInitDataTypes,
    __inout_ecount( *f_pcInitDataTypes ) _Deref_prepost_z_        DRM_CHAR                              **f_rgpszInitDataTypes,
    __in_z_opt                                              const DRM_CHAR                               *f_pszDistinctiveIdentifierRequested,
    __out_z_opt                                                   DRM_CHAR                              **f_ppszDistinctiveIdentifierUsed,
    __in_z_opt                                              const DRM_CHAR                               *f_pszPersistedStateRequested,
    __out_z_opt                                                   DRM_CHAR                              **f_ppszPersistedStateUsed,
    __in                                                          DRM_DWORD                               f_cSessionTypes,
    __in_ecount_opt( f_cSessionTypes ) _Deref_pre_opt_z_    const DRM_CHAR                              **f_rgpszSessionTypes,
    __out_ecount( 1 )                                             DRM_CDMI_MEDIA_KEY_SYSTEM_ACCESS       *f_pMediaKeySystemAccess );

DRM_NO_INLINE DRM_API_VOID DRM_VOID DRM_CALL DRM_CDMI_DestroyMediaKeySystemAccess(
    __inout_ecount( 1 )                                           DRM_CDMI_MEDIA_KEY_SYSTEM_ACCESS       *f_pMediaKeySystemAccess );

DRM_NO_INLINE DRM_API_VOID const DRM_CHAR* DRM_CALL DRM_CDMI_GetKeySystem(
    __in_ecount( 1 )                                        const DRM_CDMI_MEDIA_KEY_SYSTEM_ACCESS       *f_pMediaKeySystemAccess );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMI_GetConfiguration(
    __inout_ecount( 1 )                                           DRM_CDMI_MEDIA_KEY_SYSTEM_ACCESS       *f_pMediaKeySystemAccess,
    __out                                                         DRM_DWORD                              *f_pcInitDataTypes,
    __deref_out_ecount( *f_pcInitDataTypes )                      DRM_CHAR                             ***f_prgpszInitDataTypes,
    __out                                                         DRM_CHAR                              **f_ppszDistinctiveIdentifier,
    __out                                                         DRM_CHAR                              **f_ppszPersistedState,
    __out                                                         DRM_DWORD                              *f_pcSessionTypes,
    __deref_out_ecount( *f_pcSessionTypes )                       DRM_CHAR                             ***f_prgpszSessionTypes );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMI_CreateMediaKeys(
    __in_ecount( 1 )                                        const DRM_CDMI_MEDIA_KEY_SYSTEM_ACCESS       *f_pMediaKeySystemAccess,
    __out_ecount( 1 )                                             DRM_CDMI_MEDIA_KEYS                    *f_pMediaKeys );

/*
** MediaKeys methods
*/
DRM_NO_INLINE DRM_API_VOID DRM_VOID DRM_CALL DRM_CDMI_DestroyMediaKeys(
    __inout_ecount( 1 )                                           DRM_CDMI_MEDIA_KEYS                    *f_pMediaKeys );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMI_SetServerCertificate(
    __inout_ecount( 1 )                                           DRM_CDMI_MEDIA_KEYS                    *f_pMediaKeys,
    __in                                                          DRM_DWORD                               f_cbServerCertificate,
    __in_ecount( f_cbServerCertificate )                    const DRM_BYTE                               *f_pbServerCertificate );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMI_CreateMediaKeySession(
    __in_ecount( 1 )                                        const DRM_CDMI_MEDIA_KEYS                    *f_pMediaKeys,
    __in_z_opt                                              const DRM_CHAR                               *f_pszSessionType,
    __in_opt                                                      DRM_VOID                               *f_pOEMContext,
    __in_z                                                  const DRM_CHAR                               *f_pszDeviceStoreName,
    __out_ecount( 1 )                                             DRM_CDMI_MEDIA_KEY_SESSION             *f_pMediaKeySession );

/*
** MediaKeySession methods
*/
DRM_NO_INLINE DRM_API_VOID DRM_VOID DRM_CALL DRM_CDMI_DestroyMediaKeySession(
    __inout_ecount( 1 )                                           DRM_CDMI_MEDIA_KEY_SESSION             *f_pMediaKeySession );

DRM_NO_INLINE DRM_API_VOID const DRM_CHAR* DRM_CALL DRM_CDMI_GetSessionId(
    __in_ecount( 1 )                                        const DRM_CDMI_MEDIA_KEY_SESSION             *f_pMediaKeySession );

DRM_NO_INLINE DRM_API_VOID DRM_VOID DRM_CALL DRM_CDMI_SetSessionCallbacks(
    __inout_ecount( 1 )                                           DRM_CDMI_MEDIA_KEY_SESSION             *f_pMediaKeySession,
    __in_ecount_opt( 1 )                                          PFN_DRM_CDMI_OnCloseCallback            f_pfnOnClose,
    __in_ecount_opt( 1 )                                          PFN_DRM_CDMI_OnKeyStatusChangeCallback  f_pfnOnKeyStatusChange,
    __in_ecount_opt( 1 )                                          PFN_DRM_CDMI_OnKeyMessageCallback       f_pfnOnKeyMessage );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMI_GetExpiration(
    __in_ecount( 1 )                                        const DRM_CDMI_MEDIA_KEY_SESSION             *f_pMediaKeySession,
    __out                                                         DRM_UINT64                             *f_pui64Expiration );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMI_GetMediaKeyCount(
    __in_ecount( 1 )                                        const DRM_CDMI_MEDIA_KEY_SESSION             *f_pMediaKeySession,
    __out                                                         DRM_DWORD                              *f_pcMediaKeys );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMI_GetMediaKeyStatus(
    __in_ecount( 1 )                                        const DRM_CDMI_MEDIA_KEY_SESSION             *f_pMediaKeySession,
    __inout_opt                                                   DRM_DWORD                              *f_piMediaKey,
    __inout_opt                                                   DRM_ID                                 *f_pKeyId,
    __out_opt                                                     DRM_BOOL                               *f_pfHasKey,
    __out_opt                                                     DRM_CDMI_KEY_STATUS                    *f_peKeyStatus );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMI_GenerateRequest(
    __inout_ecount( 1 )                                           DRM_CDMI_MEDIA_KEY_SESSION             *f_pMediaKeySession,
    __in_z                                                  const DRM_CHAR                               *f_pszInitDataType,
    __in                                                          DRM_DWORD                               f_cbInitData,
    __in_ecount( f_cbInitData )                             const DRM_BYTE                               *f_pbInitData );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMI_Load(
    __inout_ecount( 1 )                                           DRM_CDMI_MEDIA_KEY_SESSION             *f_pMediaKeySession,
    __in_z                                                  const DRM_CHAR                               *f_pszSessionId,
    __in_opt                                                      DRMPFNPOLICYCALLBACK                    f_pfnPolicyCallback,
    __in_opt                                                const DRM_VOID                               *f_pvPolicyCallbackContext );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMI_Update(
    __inout_ecount( 1 )                                           DRM_CDMI_MEDIA_KEY_SESSION             *f_pMediaKeySession,
    __in_opt                                                      DRMPFNPOLICYCALLBACK                    f_pfnPolicyCallback,
    __in_opt                                                const DRM_VOID                               *f_pvPolicyCallbackContext,
    __in                                                          DRM_DWORD                               f_cbResponse,
    __in_ecount( f_cbResponse )                             const DRM_BYTE                               *f_pbResponse );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMI_Close(
    __inout_ecount( 1 )                                           DRM_CDMI_MEDIA_KEY_SESSION             *f_pMediaKeySession );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMI_Remove(
    __inout_ecount( 1 )                                           DRM_CDMI_MEDIA_KEY_SESSION             *f_pMediaKeySession );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMI_DecryptOpaque(
    __inout_ecount( 1 )                                           DRM_CDMI_MEDIA_KEY_SESSION             *f_pMediaKeySession,
    __in                                                    const DRM_ID                                 *f_pKeyId,
    __in                                                          DRM_DWORD                               f_cEncryptedRegionMappings,
    __in_ecount( f_cEncryptedRegionMappings )               const DRM_DWORD                              *f_pdwEncryptedRegionMappings,
    __in                                                          DRM_UINT64                              f_ui64InitializationVector,
    __in                                                          DRM_DWORD                               f_cbEncryptedContent,
    __in_bcount( f_cbEncryptedContent )                     const DRM_BYTE                               *f_pbEncryptedContent,
    __out                                                         DRM_DWORD                              *f_pcbOpaqueClearContent,
    __deref_out_bcount( *f_pcbOpaqueClearContent )                DRM_BYTE                              **f_ppbOpaqueClearContent );

DRM_NO_INLINE DRM_API_VOID DRM_VOID DRM_CALL DRM_CDMI_FreeOpaqueDecryptedContent(
    __inout_ecount( 1 )                                           DRM_CDMI_MEDIA_KEY_SESSION             *f_pMediaKeySession,
    __in                                                    const DRM_ID                                 *f_pKeyId,
    __in                                                          DRM_DWORD                               f_cbOpaqueClearContent,
    __inout_bcount( f_cbOpaqueClearContent )                      DRM_BYTE                               *f_pbOpaqueClearContent );

DRM_NO_INLINE DRM_API_VOID DRM_APP_CONTEXT* DRM_CALL DRM_CDMI_GetSessionAppContext(
    __inout_ecount( 1 )                                           DRM_CDMI_MEDIA_KEY_SESSION             *f_pMediaKeySession );

EXIT_PK_NAMESPACE

#endif /*__DRMCDMI_H__ */
