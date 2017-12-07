/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
/*=============================================================================
VideoCore OS Abstraction Layer - basic types
=============================================================================*/

#ifndef VCOS_TYPES_H
#define VCOS_TYPES_H

#define VCOS_VERSION   1

#include <assert.h>
#include <stddef.h>
#include "vcos_platform_types.h"
#include "libs/util/common.h"

/* Error return codes - chosen to be similar to errno values */
typedef enum
{
   VCOS_SUCCESS,
   VCOS_EAGAIN,
   VCOS_ENOENT,
   VCOS_ENOSPC,
   VCOS_EINVAL,
   VCOS_EACCESS,
   VCOS_ENOMEM,
   VCOS_ENOSYS,
   VCOS_EEXIST,
   VCOS_ENXIO,
   VCOS_EINTR,
   VCOS_ETIMEDOUT
} VCOS_STATUS_T;

static inline const char *vcos_desc_status(VCOS_STATUS_T status)
{
   switch (status)
   {
   case VCOS_SUCCESS:   return "SUCCESS";
   case VCOS_EAGAIN:    return "EAGAIN";
   case VCOS_ENOENT:    return "ENOENT";
   case VCOS_ENOSPC:    return "ENOSPC";
   case VCOS_EINVAL:    return "EINVAL";
   case VCOS_EACCESS:   return "EACCESS";
   case VCOS_ENOMEM:    return "ENOMEM";
   case VCOS_ENOSYS:    return "ENOSYS";
   case VCOS_EEXIST:    return "EEXIST";
   case VCOS_ENXIO:     return "ENXIO";
   case VCOS_EINTR:     return "EINTR";
   case VCOS_ETIMEDOUT: return "ETIMEDOUT";
   default:             assert(0); return NULL;
   }
}

#endif
