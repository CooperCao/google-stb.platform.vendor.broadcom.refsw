/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#ifndef __DRMSECURETIMETYPES_H__
#define __DRMSECURETIMETYPES_H__

ENTER_PK_NAMESPACE

typedef enum _DRM_SECURETIME_CLOCK_TYPE
{
    DRM_SECURETIME_CLOCK_TYPE_INVALID = 0,
    DRM_SECURETIME_CLOCK_TYPE_TEE     = 1,    /* SecureTime - TEE clock           */
    DRM_SECURETIME_CLOCK_TYPE_UM      = 2     /* SecureTime - User-mode clock     */
} DRM_SECURETIME_CLOCK_TYPE;

EXIT_PK_NAMESPACE

#endif /*__DRMSECURETIMETYPES_H__ */