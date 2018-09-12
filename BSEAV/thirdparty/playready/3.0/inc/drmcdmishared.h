/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#ifndef __DRMCDMISHARED_H__
#define __DRMCDMISHARED_H__

#include <drmcdmitypes.h>
#include <drmlicacqv3.h>

ENTER_PK_NAMESPACE

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMI_GetPROFromCencInitData(
    __in                                              DRM_DWORD                   f_cbInitData,
    __in_bcount( f_cbInitData )                 const DRM_BYTE                   *f_pbInitData,
    __out                                             DRM_DWORD                  *f_pibPRO,
    __out                                             DRM_DWORD                  *f_pcbPRO );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMI_HDS_WriteSession(
    __inout                                           DRM_DST                    *f_pDST,
    __in                                        const DRM_ID                     *f_pidSession,
    __in                                              DRM_CDMI_SESSION_TYPE       f_eSessionType,
    __in                                              DRM_DWORD                   f_cbPRO,
    __in_ecount_opt( f_cbPRO )                  const DRM_BYTE                   *f_pbPRO,
    __in                                              DRM_DWORD                   f_cLicenseAcks,
    __in_ecount( f_cLicenseAcks )               const DRM_LICENSE_ACK            *f_pLicenseAcks );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMI_HDS_ReadSession(
    __inout                                           DRM_DST                    *f_pDST,
    __inout                                           DRM_SECURECORE_CONTEXT     *f_pSecureCore,
    __inout                                           DRM_SECURESTOP_CONTEXT     *f_pSecureStop,
    __in                                        const DRM_ID                     *f_pidSession,
    __in                                              DRM_CDMI_SESSION_TYPE       f_eSessionType,
    __in                                              DRM_DWORD                   f_cbServerCertificate,
    __in_ecount_opt( f_cbServerCertificate )    const DRM_BYTE                   *f_pbServerCertificate,
    __out_opt                                         DRM_BOOL                   *f_pfHasPersistentLicenses,
    __out_opt                                         DRM_BOOL                   *f_pfHasSecureStopLicenses,
    __out                                             DRM_DWORD                  *f_pcLicenseAcks,
    __deref_out_ecount( *f_pcLicenseAcks )            DRM_LICENSE_ACK           **f_ppLicenseAcks,
    __out_opt                                         DRM_DWORD                  *f_pcbPRO,
    __deref_out_bcount_opt( *f_pcbPRO )               DRM_BYTE                  **f_ppbPRO,
    __out                                             DRM_BOOL                   *f_pfRemoveCalled );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMI_HDS_SetRemoveCalledOnSession(
    __inout                                           DRM_DST                    *f_pDST,
    __in                                        const DRM_ID                     *f_pidSession,
    __in                                              DRM_CDMI_SESSION_TYPE       f_eSessionType );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMI_HDS_DeleteSession(
    __inout                                           DRM_DST                    *f_pDST,
    __in                                        const DRM_ID                     *f_pidSession,
    __in                                              DRM_CDMI_SESSION_TYPE       f_eSessionType );

EXIT_PK_NAMESPACE

#endif /*__DRMCDMISHARED_H__ */
