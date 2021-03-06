//****************************************************************************
//
// Copyright (c) 2009-2012 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.  
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//****************************************************************************
//    Description:                                                          
//                                                                          
//****************************************************************************
//    Revision History:                                                     
//                                                                          
//****************************************************************************
//    Filename: mocaBridge.h
/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:license-public
 *
 ************************************************************************/

#ifndef __OS_DEFS_H__
#define __OS_DEFS_H__

#if defined(__gnu_linux__)
#include <unistd.h>  /* for getopt */
#include <syslog.h>  /* for syslog */
#endif
#include <stdio.h>   /* for snprintf */
#include <stdint.h>  /* for the various integer types */
#include <stdlib.h>  /* for NULL */
#include <string.h>  /* for strlen, strncpy */
#include <ctype.h>   /* for isdigit */

#include <stdarg.h>  /* for va_list */
#include "cms_params.h"

/*!\file os_defs.h
 * \brief Various commonly used, but OS dependent definitions are defined here.
 *
 *  This file is for Linux.
 */

#ifndef NUMBER_TYPES_ALREADY_DEFINED
#define NUMBER_TYPES_ALREADY_DEFINED

/** Unsigned 64 bit integer.
 * This data type was introduced in TR-106 Issue 1, Admendment 2, Sept. 2008
 */
typedef uint64_t   UINT64;

/** Signed 64 bit integer.
 * This data type was introduced in TR-106 Issue 1, Admendment 2, Sept. 2008
 */
typedef int64_t    SINT64;

#ifndef BCMTYPES_H
/** Unsigned 32 bit integer. */
typedef uint32_t   UINT32;
#endif

/** Signed 32 bit integer. */
typedef int32_t    SINT32;

/** Unsigned 16 bit integer. */
typedef uint16_t   UINT16;

/** Signed 16 bit integer. */
typedef int16_t    SINT16;

/** Unsigned 8 bit integer. */
typedef uint8_t    UINT8;

/** Signed 8 bit integer. */
typedef int8_t     SINT8;

#endif /* NUMBER_TYPES_ALREADY_DEFINED */


#ifndef BOOL_TYPE_ALREADY_DEFINED
#define BOOL_TYPE_ALREADY_DEFINED

/**Boolean type; use 1 byte only, possible values are TRUE(1) or FALSE(0) only.
 *
 * TRUE/FALSE defined in cms.h
 */
typedef uint8_t    UBOOL8;

#endif /* BOOL_TYPE_ALREADY_DEFINED */


/** Base64 encoded string representation of binary data.
 *
 * This is to support TR69 data types.
 */
typedef char *     BASE64;


/** Hex encoded string representation of binary data.
 * This data type was introduced in TR-106 Issue 1, Admendment 2, Sept. 2008
 *
 * This is to support TR69 data types.
 */
typedef char *     HEXBINARY;


/** String representation of date and time.
 *
 * This is to support TR69 data types.
 */
typedef char *     DATETIME;


/** Invalid file descriptor number */
#define CMS_INVALID_FD  (-1)


/** Invalid process id.
 *
 * Management entities should not need to use this constant.  It is used
 * by OS dependent code in the OAL layer.  But I put this constant here
 * to make it easier to find.
 */
#define CMS_INVALID_PID   0


/** A number to identify a MdmObject (but not the specific instance of
 *  the object).
 *
 * MdmObjectId's are defined in mdm_oid.h.
 */
typedef UINT16 MdmObjectId;


/** A structure to keep track of instance information.
 *
 * External callers can treat this as an opaque handle.
 * Note the instance array must be of type UINT32 because
 * the instance id's are constantly increasing, so we
 * cannot save space by defining instance to be
 * an array of UINT8's.
 */
typedef struct
{
   UINT8 currentDepth;                     /**< next index in the instance array 
                                            *   to fill.  0 means empty. */
   UINT32 instance[MAX_MDM_INSTANCE_DEPTH];/**< Array of instance id's. */
} InstanceIdStack;



#endif /* __OS_DEFS_H__ */

