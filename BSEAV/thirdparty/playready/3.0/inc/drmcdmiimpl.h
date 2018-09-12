/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#ifndef __DRMCDMIIMPL_H__
#define __DRMCDMIIMPL_H__

#include <drmcdmitypes.h>
#include <drmmodulesupport.h>

ENTER_PK_NAMESPACE

/*
** Helper methods
*/
DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_CDMIIMPL_ParseInitDataKeyIdsJson(
    __in                              DRM_DWORD   f_cbInitData,
    __in_ecount( f_cbInitData ) const DRM_BYTE   *f_pbInitData,
    __out                             DRM_DWORD  *f_pcbPRO,
    __deref_out_ecount( *f_pcbPRO )   DRM_BYTE  **f_ppbPRO );

EXIT_PK_NAMESPACE

#endif /*__DRMCDMIIMPL_H__ */
