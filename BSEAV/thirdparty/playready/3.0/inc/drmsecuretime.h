/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#ifndef __DRMSECURETIME_H__
#define __DRMSECURETIME_H__

#include <drmsecuretimetypes.h>
#include <drmmanagertypes.h>
#include <drmmodulesupport.h>

ENTER_PK_NAMESPACE

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_SECURETIME_GenerateChallenge(
    __inout                                         DRM_SECURECORE_CONTEXT      *f_pSecureCoreCtx,
    __out                                           DRM_DWORD                   *f_pcbChallenge,
    __deref_out_bcount( *f_pcbChallenge )           DRM_BYTE                   **f_ppbChallenge );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_SECURETIME_ProcessResponse(
    __inout                                         DRM_SECURECORE_CONTEXT      *f_pSecureCoreCtx,
    __in                                            DRM_DWORD                    f_cbRevocationBuffer,
    __inout_bcount_opt( f_cbRevocationBuffer )      DRM_BYTE                    *f_pbRevocationBuffer,
    __in                                            DRM_REVOCATIONSTORE_CONTEXT *f_pRevContext,
    __inout_opt                                     DRM_SECSTORE_CONTEXT        *f_pSecStoreGlobalContext,
    __inout_opt                                     DRM_DST                     *f_pDatastoreHDS,
    __inout                                         DRM_BOOL                    *f_pfClockSet,
    __inout                                         DRM_LICEVAL_CONTEXT         *f_pLicEvalContext,
    __in                                            DRM_DWORD                    f_cbResponse,
    __in_bcount( f_cbResponse )               const DRM_BYTE                    *f_pbResponse );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_SECURETIME_GetValue(
    __inout                                         DRM_SECURECORE_CONTEXT      *f_pSecureCoreCtx,
    __out                                           DRMFILETIME                 *f_pftSystemTime,
    __out                                           DRM_SECURETIME_CLOCK_TYPE   *f_peClockType );

EXIT_PK_NAMESPACE

#endif /*__DRMSECURETIME_H__ */