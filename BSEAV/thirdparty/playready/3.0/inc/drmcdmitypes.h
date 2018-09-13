/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#ifndef __DRMCDMITYPES_H__
#define __DRMCDMITYPES_H__

#include <drmmanagertypes.h>
#include <drmlicacqv3.h>
#include <drmbcertformatparser.h>

ENTER_PK_NAMESPACE

#define DRM_CDMI_MAX_SUPPORTED_INIT_DATA_TYPES  2
#define DRM_CDMI_MAX_SUPPORTED_SESSION_TYPES    3
#define DRM_CDMI_CHARS_IN_B64_DRM_ID            ( CCH_BASE64_EQUIV( sizeof( DRM_ID ) ) )

typedef DRM_API_VOID DRM_VOID( DRM_CALL *PFN_DRM_CDMI_OnCloseCallback )( DRM_VOID );
typedef DRM_API_VOID DRM_VOID( DRM_CALL *PFN_DRM_CDMI_OnKeyStatusChangeCallback )( DRM_VOID );
typedef DRM_API_VOID DRM_VOID( DRM_CALL *PFN_DRM_CDMI_OnKeyMessageCallback )(
    __in_z                              const DRM_CHAR      *f_pszMediaKeyMessageType,
    __in                                      DRM_DWORD      f_cbMediaKeyMessage,
    __in_ecount( f_cbMediaKeyMessage )        DRM_BYTE      *f_pbMediaKeyMessage );

/*
** Note: PlayReady never returns an error which should map to QuotaExceededError.
** Refer to DRM_CDMI_Load documentation for more information.
*/
typedef enum __tag_DRM_CDMI_EXCEPTION
{
    DRM_CDMI_EXCEPTION_NOT_SUPPORTED_ERROR,
    DRM_CDMI_EXCEPTION_INVALID_STATE_ERROR,
    DRM_CDMI_EXCEPTION_TYPE_ERROR
} DRM_CDMI_EXCEPTION;

/* Note: used as a bitmask. */
typedef enum __tag_DRM_CDMI_SESSION_TYPE
{
    DRM_CDMI_SESSION_TYPE_TEMPORARY                 = 0x1,
    DRM_CDMI_SESSION_TYPE_PERSISTENT_USAGE_RECORD   = 0x2,
    DRM_CDMI_SESSION_TYPE_PERSISTENT_LICENSE        = 0x4,
} DRM_CDMI_SESSION_TYPE;

#define DRM_CDMI_SESSION_TYPE_COUNT 3
#define DRM_CDMI_SESSION_TYPE_MASK_TEMPORARY                                    ( DRM_CDMI_SESSION_TYPE_TEMPORARY )
#define DRM_CDMI_SESSION_TYPE_MASK_TEMPORARY_AND_USAGE_RECORD                   ( DRM_CDMI_SESSION_TYPE_TEMPORARY | DRM_CDMI_SESSION_TYPE_PERSISTENT_USAGE_RECORD )
#define DRM_CDMI_SESSION_TYPE_MASK_TEMPORARY_AND_USAGE_RECORD_AND_PERSISTENT    ( DRM_CDMI_SESSION_TYPE_TEMPORARY | DRM_CDMI_SESSION_TYPE_PERSISTENT_USAGE_RECORD | DRM_CDMI_SESSION_TYPE_PERSISTENT_LICENSE )

/* Note: These macros work on either an enum or a bitmask */
#define DRM_CDMI_SESSION_TYPE_ALLOW_TEMPORARY_LICENSES( __eSessionType ) ( ( ( __eSessionType ) & DRM_CDMI_SESSION_TYPE_TEMPORARY ) != 0 )
#define DRM_CDMI_SESSION_TYPE_ALLOW_SECURESTOP_LICENSES( __eSessionType ) ( ( ( __eSessionType ) & DRM_CDMI_SESSION_TYPE_PERSISTENT_USAGE_RECORD ) != 0 )
#define DRM_CDMI_SESSION_TYPE_ALLOW_PERSISTENT_LICENSES( __eSessionType ) ( ( ( __eSessionType ) & DRM_CDMI_SESSION_TYPE_PERSISTENT_LICENSE ) != 0 )
#define DRM_CDMI_SESSION_TYPE_HAS_PERSISTENT_STATE( __eSessionType ) \
    ( DRM_CDMI_SESSION_TYPE_ALLOW_SECURESTOP_LICENSES( __eSessionType ) || DRM_CDMI_SESSION_TYPE_ALLOW_PERSISTENT_LICENSES( __eSessionType ) )

/*
** Note: PlayReady never returns emun values which are equivalent
** to the following strings.
**
** "output-restricted"
** "output-downscaled"
** "status-pending"
**
** However, when PlayReady returns DRM_CDMI_KEY_STATUS_USABLE,
** the caller should factor in the output protection information
** sent via the DRMPFNPOLICYCALLBACK parameter passed to other
** CDMI functions.
** For example, if PlayReady returns DRM_CDMI_KEY_STATUS_USABLE
** but the output protection information requires the caller
** to downscale content based on the caller's knowledge of
** the state of output protection enforcement in the media
** pipeline, then instead of sending "usable" to the application,
** the caller should instead send "output-restricted".
**
** Refer to the PlayReady Compliance and Robustness Rules
** and the EME specification for more information.
*/
typedef enum __tag_DRM_CDMI_KEY_STATUS
{
    DRM_CDMI_KEY_STATUS_UNDEFINED           = 0,    /*
                                                    ** An invalid argument, e.g. an unknown KID, was passed to DRM_CDMI_GetMediaKeyStatus.
                                                    ** Do not include this KID's information in the MediaKeyStatusMap.
                                                    */
    DRM_CDMI_KEY_STATUS_USABLE              = 1,    /* Maps to "usable" */
    DRM_CDMI_KEY_STATUS_EXPIRED             = 2,    /* Maps to "expired" */
    DRM_CDMI_KEY_STATUS_RELEASED            = 3,    /* Maps to "released" */
    DRM_CDMI_KEY_STATUS_INTERNAL_ERROR      = 4,    /* Maps to "internal-error" */
} DRM_CDMI_KEY_STATUS;

typedef struct __tag_DRM_CDMI_KEY_STATUS_WITH_ERROR
{
    DRM_CDMI_KEY_STATUS eStatus;
    DRM_RESULT          drStatus;
} DRM_CDMI_KEY_STATUS_WITH_ERROR;

typedef struct __tag_DRM_CDMI_MEDIA_KEY_SYSTEM_ACCESS
{
    DRM_DWORD    dwSessionTypesAllowedMask;
    DRM_DWORD    cSupportedInitDataTypes;
    DRM_CHAR    *rgszSupportedInitDataTypes[ DRM_CDMI_MAX_SUPPORTED_INIT_DATA_TYPES ];
    DRM_DWORD    cSupportedSessionTypes;
    DRM_CHAR    *rgszSupportedSessionTypes[ DRM_CDMI_MAX_SUPPORTED_SESSION_TYPES ];
} DRM_CDMI_MEDIA_KEY_SYSTEM_ACCESS;

typedef struct __tag_DRM_CDMI_CERT_DATA
{
    DRM_DWORD                        cbServerCertificate;
    DRM_BYTE                        *pbServerCertificate;
} DRM_CDMI_CERT_DATA;

typedef struct __tag_DRM_CDMI_MEDIA_KEYS
{
    DRM_DWORD           dwSessionTypesAllowedMask;
    DRM_CDMI_CERT_DATA  oCertData;
} DRM_CDMI_MEDIA_KEYS;

typedef struct __tag_DRM_CDMI_MEDIA_KEY_SESSION_CALLBACKS
{
    PFN_DRM_CDMI_OnCloseCallback            pfnOnClose;
    PFN_DRM_CDMI_OnKeyStatusChangeCallback  pfnKeyStatusChange;
    PFN_DRM_CDMI_OnKeyMessageCallback       pfnOnKeyMessage;
} DRM_CDMI_MEDIA_KEY_SESSION_CALLBACKS;

typedef struct __tag_DRM_CDMI_APP_CONTEXT_STATE
{
    DRM_APP_CONTEXT         oAppContext;
    DRM_VOID               *pOEMContext;
    DRM_BYTE                rgbAppContextOpaqueBuffer[ MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE ];
    DRM_BYTE                rgbAppContextRevocationBuffer[ REVOCATION_BUFFER_SIZE ];
} DRM_CDMI_APP_CONTEXT_STATE;

typedef struct __tag_DRM_CDMI_DECRYPTOR
{
    DRM_DECRYPT_CONTEXT     oDecryptContext;
    DRM_BOOL                fCommitted;
} DRM_CDMI_DECRYPTOR;

typedef struct __tag_DRM_CDMI_LICENSE_STATE
{
    DRM_LICENSE_RESPONSE                 oLicenseResponse;          /* oLicenseResponse.m_pAcks member may be allocated, if so will be freed when session is destroyed */
    DRM_LICENSE_ACK                     *pAcks;                     /* Will equal oLicenseResponse.m_rgoAcks or oLicenseResponse.m_pAcks, should not be freed */
    DRM_DWORD                            cLicenses;                 /* Will equal oLicenseResponse.m_cAcks */
    DRM_BOOL                             fBound;                    /* TRUE if currently bound to a license, FALSE otherwise */
    DRM_DWORD                            iBound;                    /* if( fBound), index of license currently bound */
    DRM_CDMI_DECRYPTOR                  *pDecryptors;               /* Size will equal cLicenses, will be freed when session is destroyed */
    DRMPFNPOLICYCALLBACK                 pfnPolicyCallback;
    const DRM_VOID                      *pvPolicyCallbackContext;
    DRM_UINT64                           ui64Expiration;
    DRM_CDMI_KEY_STATUS_WITH_ERROR      *pKeyStatusesWithErrors;    /* Size will equal cLicenses, will be freed when session is destroyed */
    DRM_BOOL                             fRemoveCalled;
} DRM_CDMI_LICENSE_STATE;

typedef struct __tag_DRM_CDMI_MEDIA_KEY_SESSION_STATE
{
    DRM_BOOL                    fInitialized;
    DRM_BOOL                    fCloseExecuted;
    DRM_BOOL                    fCallable;
    DRM_BOOL                    fAllSessionsForSecureStop;
    DRM_CDMI_APP_CONTEXT_STATE  oAppContextState;
    DRM_CDMI_LICENSE_STATE      oLicenseState;
    DRM_CDMI_CERT_DATA          oCertData;
} DRM_CDMI_MEDIA_KEY_SESSION_STATE;

typedef struct __tag_DRM_CDMI_MEDIA_KEY_SESSION
{
    DRM_ID                                  idSession;
    DRM_CHAR                                rgchSessionId[ DRM_CDMI_CHARS_IN_B64_DRM_ID + 1 ];     /* +1 for null terminator */
    DRM_CDMI_SESSION_TYPE                   eSessionType;

    DRM_CDMI_MEDIA_KEY_SESSION_CALLBACKS    oCallbacks;
    DRM_CDMI_MEDIA_KEY_SESSION_STATE        oSessionState;
} DRM_CDMI_MEDIA_KEY_SESSION;

EXIT_PK_NAMESPACE

#endif /*__DRMCDMITYPES_H__ */
