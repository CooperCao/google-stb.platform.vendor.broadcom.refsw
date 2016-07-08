/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#ifndef __DRMMANAGERTYPES_H__
#define __DRMMANAGERTYPES_H__

#include <drmcontextsizes.h>

ENTER_PK_NAMESPACE

#define DRM_REAL_TIME_EXPIRATION_CHECK_BYTE_COUNT   0x100000  /* Check for expiration every 1MB (0x100000 == 1024*1024 == 1,048,578 bytes) of data processed */

typedef struct
{
    DRM_BYTE rgbBuffer[ sizeof( DRM_CIPHER_CONTEXT ) ];
} DRM_DECRYPT_CONTEXT;

typedef enum
{
    DRM_DGP_DEVICE_CERT_MD    = 1,
    DRM_DGP_DEVICE_CERT_PD    = 2,
    DRM_DGP_RESERVED3         = 3,
    DRM_DGP_PLAYREADY_VERSION = 4,
    DRM_DGP_RESERVED5         = 5,
    DRM_DGP_RESERVED6         = 6
} DRM_DEVICE_GET_PROPERTY;

/*
** pbKeyID:      Array of bytes that contains the KeyID from the V1 ASF header object.
** cbKeyID:      Count of bytes in pbKeyID.
** pbSecretData: Array of bytes that contains the SecretData from the V1 ASF header.
** cbSecretData: Count of bytes in pbSecretData.
** pbURL:        Array of bytes that contains the license acquisition URL from the V1 ASF header.
** cbURL:        Count of bytes in pbURL.
*/
typedef struct
{
    const DRM_BYTE *pbKeyID;

    DRM_DWORD       cbKeyID;

    const DRM_BYTE *pbSecretData;

    DRM_DWORD       cbSecretData;

    const DRM_BYTE *pbURL;

    DRM_DWORD       cbURL;

} DRM_CSP_V1_HEADER_DATA;

typedef struct
{
    const DRM_BYTE *pbKeyID;        /* For this structure, pbKeyID is a base64-encoded UTF16 string */
    DRM_DWORD       cbKeyID;

    const DRM_BYTE *pbHeaderData;
    DRM_DWORD       cbHeaderData;

} DRM_CSP_PLAYREADY_OBJ_WITH_KID_DATA;

typedef struct
{
    DRM_DWORD      cKIDs;
    DRM_WCHAR     *rgpwszKIDs[1];

} DRM_CGP_HEADER_KIDS_DATA;

typedef struct
{
    eDRM_HEADER_VERSION     eHeaderVersion; /* Only supported values are: DRM_HEADER_VERSION_4, DRM_HEADER_VERSION_4_1, DRM_HEADER_VERSION_4_2 */
    DRM_SUPPORTED_CIPHERS   eCipherType;
    DRM_CONST_STRING        dstrKID;        /* dstrKID is a base64-encoded UTF16 string */
    DRM_CONST_STRING        dstrCheckSum;
    DRM_CONST_STRING        dstrPlayReadySilentURL;
    DRM_CONST_STRING        dstrPlayReadyNonSilentURL;
    DRM_CONST_STRING        dstrServiceID;
    DRM_CONST_STRING        dstrCustomAttributesXML;
    DRM_CONST_STRING        dstrDecryptorSetup;         /* If set: L"STANDARD" or L"ONDEMAND" */
} DRM_CSP_HEADER_COMPONENTS_DATA;

typedef enum
{
    DRM_LGP_MIN_APP_SEC = 1,
    DRM_LGP_SOURCE_ID,
    DRM_LGP_REVINFO_VERSION,
    DRM_LGP_MID,
    DRM_LGP_HAS_INCLUSION_GUID,
    DRM_LGP_ROOT_LOCAL_LICENSE_POLICY_DESCRIPTOR,
    DRM_LGP_LEAF_LOCAL_LICENSE_POLICY_DESCRIPTOR,
} DRM_LICENSE_GET_PROPERTY;

typedef struct
{
    DRM_CHAR szMID[ DRM_MAX_LICENSE_CHAIN_DEPTH ][ CCH_BASE64_EQUIV( sizeof( DRM_MID ) ) ] ;
} DRM_LGP_MID_EXTRA_DATA;

typedef struct
{
    DRM_GUID guid;
} DRM_LGP_HAS_INCLUSION_GUID_EXTRA_DATA;

/* Strings to be used from DRMManager */
extern DRM_GLOBAL_CONST DRM_CONST_STRING      g_dstrWMDRM_RIGHT_PLAYBACK;

typedef struct
{
    /* This data is Opaque.  Do not set any value in it. */
    DRM_BYTE rgbOpaqueBuffer[ DRM_APP_CONTEXT_BUFFER_SIZE ];
} DRM_APP_CONTEXT;

#define DRM_STORE_CLEANUP_ALL                           0xFFFFFFFF
#define DRM_STORE_CLEANUP_DELETE_EXPIRED_LICENSES       0x1
#define DRM_STORE_CLEANUP_DELETE_REMOVAL_DATE_LICENSES  0x2

EXIT_PK_NAMESPACE

#endif /* __DRMMANAGERTYPES_H__ */