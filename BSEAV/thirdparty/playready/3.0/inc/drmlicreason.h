/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/


/* Description: The error codes to indicate the reason why a license is not usable.
              The error codes are used to give a better feedback to the user.
              These codes are used in the license to indicate the reason.
              drm.reason variable is set to this reason.
*/

#ifndef __LICREASON_H__
#define __LICREASON_H__

#include <drmnamespace.h>

ENTER_PK_NAMESPACE

/* Server can help with these values. These are reasons why a license that was just obtained is not usable. */
enum
{
    LR_LICENSE_SUCCESS = 0,     /* MUST be zero so that zero-initialized value is "success" */
    LR_LICENSE_EXPIRED,
    LR_LICENSE_NOTENABLED,
    LR_LICENSE_APPSECLOW,
    LR_LICENSE_STORE_NOT_ALLOWED,
    LR_LICENSE_RESTRICTED_SOURCE,
    LR_LICENSE_SERVER_MAX
};

/* Client side detectable reasons */
enum
{
    LR_LICENSE_CLOCK_NOT_SET = LR_LICENSE_SERVER_MAX,   /* MUST not overlap with previous enum */
    LR_LICENSE_TIME_CHECK_FAILURE,         /* special case for if the current time is before the first use/store time, rollback is detected, etc */
    LR_LICENSE_EXTENSIBLE_RIGHT_NOT_FOUND
};

EXIT_PK_NAMESPACE

#endif /* __LICREASON_H__ */