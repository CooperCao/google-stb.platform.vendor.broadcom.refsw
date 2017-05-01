/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef BIP_TYPES_H
#define BIP_TYPES_H

#include "bstd.h"
#include "nexus_platform.h"

/* This include file defines global BIP datatypes and enums required by the
   public interfaces to the BIP (Broadcom IPtv Library)
*/

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
*  Define some convenience macros.
*
*  Caution!  Beware of side-effects. Don't do BIP_MIN(i++,j++) *
**************************************************************************/
#define BIP_ABS(num)                ((num) < 0 ? -(num) : (num))    /* Returns absolute value of num. */
#define BIP_NEG( num )              ((num) > 0 ? -(num) : (num))    /* Returns negative absolute value of num. */
#define BIP_MIN( x, y )             ((x)<(y) ? (x) : (y))           /* Returns maximum of two numbers. */
#define BIP_MAX( x, y )             ((x)>(y) ? (x) : (y))           /* Returns minimum of two numbers. */
#define BIP_ABSDIFF(x, y)           ((x)>(y) ? ((x)-(y)) : ((y)-(x)))  /* Absolute value of difference.  */
#define BIP_N_ELEMENTS( array )     (sizeof(array)/sizeof(array[0]))

#define   BIP_P_1E9    ((int64_t)1000000000)                         /* 1x10**9  */
#define   BIP_P_1E18U ((uint64_t)1000000000 * 1000000000)   /* Unsigned 1*10**18 */
#define   BIP_P_1E18S ((int64_t)1000000000 * 1000000000)      /* Signed 1*10**18 */

#define  BIP_I64_MAX  ((int64_t)(((uint64_t)-1)>>1))  /* Largest signed 64-bit integer. Equivalent to LLONG_MAX. */
#define  BIP_U64_MAX  ((uint64_t)-1)        /* Largest unsigned 64-bit integer. Equivalent to ULLONG_MAX. */

/* Helper macro for defining a uint64_t constant in a C89 build env.
 * BIP_U64_CONST(12345,678901234)  makes 12345678901234
 *   But be sure to remove any leading zeroes from "low9digits" and "mid9digits" (or C will treat it as octal). */
#define  BIP_I64_CONST(high1digit, mid9digits, low9digits)  (((high1digit)*BIP_P_1E18S) + ((mid9digits)*BIP_P_1E9)+(low9digits))
#define  BIP_U64_CONST(high2digits, mid9digits, low9digits)  (((high2digits)*BIP_P_1E18U) + ((mid9digits)*BIP_P_1E9)+(low9digits))


/* Some macros for printing out 64-bit integers with C89 (because it doesn't have the %lld type of format. */

/* Example:
 *      \* C89 also doesn't have "LL" (64-bit) constants, so  do this to initialize
 *       * u64var to 9223372036854775807 (largest positive signed 64-bit integer): *\
 *      uint64_t    u64var =  9*BIP_P_1E18U +_223372036*BIP_P_1E9 + 854775807) ;
 *      printf( "u64var: "BIP_U64_FMT"\n", BIP_U64_ARG(u64var));
 *
 * Will print: "u64var: 9223372036854775807"
 **/
/* Unsigned versions: */
#define   BIP_U64_FMT  "%.*u%.*u%0*u"
#define   BIP_U64_ARG(num)                                                                                                         \
            (num>=BIP_P_1E18U ? 1 : 0),                         (num>=BIP_P_1E18U ? (uint32_t)(num/BIP_P_1E18U) : 0),              \
            (num>=BIP_P_1E18U ? 9 : (num>=BIP_P_1E9 ? 1 : 0)) , (num>=BIP_P_1E9   ? (uint32_t)((num%BIP_P_1E18U)/BIP_P_1E9) : 0),  \
            (num>=BIP_P_1E9   ? 9 : 1) ,                        (num>=BIP_P_1E9   ? (uint32_t)(num%BIP_P_1E9) : (uint32_t)(num))

/* Signed versions: */
#define   BIP_I64_FMT  "%.*s%.*u%.*u%0*u"
#define   BIP_I64_ARG(num)                                                                                                                                               \
            (num<0 ? 1 : 0),                                                      (num<0 ? "-" : ""),                                                                    \
            (BIP_NEG(num)<=-BIP_P_1E18S ? 1 : 0),                                  (BIP_NEG(num)<=-BIP_P_1E18S ? -(uint32_t)(BIP_NEG(num)/BIP_P_1E18S) : 0),             \
            (BIP_NEG(num)<=-BIP_P_1E18S ? 9 : (BIP_NEG(num)<=-BIP_P_1E9 ? 1 : 0)), (BIP_NEG(num)<=-BIP_P_1E9   ? -(uint32_t)((BIP_NEG(num)%BIP_P_1E18S)/BIP_P_1E9) : 0), \
            (BIP_NEG(num)<=-BIP_P_1E9   ? 9 : 1),                                  (BIP_NEG(num)<=-BIP_P_1E9   ? -(uint32_t)(BIP_NEG(num)%BIP_P_1E9) : -(uint32_t)(BIP_NEG(num)))

/* Hexidecimal versions: */
#define   BIP_X64_FMT  "%.*x%0*x"
#define   BIP_X64_ARG(num)                                                                                            \
            (num>0xffffffff ? 1 : 0),                         (num>0xffffffff ? (uint32_t)(num>>32) : 0),             \
            (num>0xffffffff ? 8 : 1) ,                        (num>0xffffffff ? (uint32_t)(num&0xffffffff) : (uint32_t)(num))

/*************************************************************************
*  Define global BIP datatypes and enums.
**************************************************************************/

/**
Summary:
BIP_Status is the common return type for BIP Interfaces. Normal successful
completion is indicated by a BIP_Status value of zero.  A non-zero BIP_Status
value indicates either exception or failure.

The BIP-specific status code names and values are defined in "bip_statuscodes.h".

Description:
An Interface may return one of the standard codes defined below.
**/
typedef unsigned  BIP_Status;  /* Or use the BIP_StatusCode typedef'd enum defined below. */

/**
 * Summary:
 * Function prototype used by BIP_CallbackDesc.
**/
typedef void (*BIP_Callback)(void *context, int param);

/**
 * Summary:
 * Standard definition of a callback in BIP.
**/
typedef struct BIP_CallbackDesc {
    BIP_Callback callback; /* Function pointer */
    void *context;           /* First parameter to callback function. */
    int param;               /* Second parameter to callback function. */
} BIP_CallbackDesc;


/**
 * Summary:
 * Some settings common to many BIP APIs.
 *
 * Description:
 *
**/
typedef struct BIP_ApiSettings
{
    int               timeout;         /* 0: non-blocking; -1: blocking, > 0: timeout in milliseconds */
    BIP_CallbackDesc  asyncCallback;   /* async completion callback: must be set if async usage of API is desired */
    BIP_Status       *pAsyncStatus;    /* status of async API.  */
                                       /* status = BIP_INF_IN_PROGRESS when API returns BIP_SUCCESS */
                                       /* status = BIP_ERR_ALREADY_IN_PROGRESS if API is retried before async completion callback */
} BIP_ApiSettings;

/**
 * Summary:
 * NetworkAddress Types
 *
**/
typedef enum BIP_NetworkAddressType
{
    BIP_NetworkAddressType_eIpV4,                      /* Use IPv4 Address. */
    BIP_NetworkAddressType_eIpV6,                      /* Use IPv6 Address. */
    BIP_NetworkAddressType_eIpV6_and_IpV4,             /* Use IPv6 address and IPv4 address. */
    BIP_NetworkAddressType_eIpV6_over_IpV4,            /* Use IPv6 address, if not available, then use IPv4 Address. */
    BIP_NetworkAddressType_eMax
} BIP_NetworkAddressType;

#define BIP_INFINITE (-1)

/* TODO: Temp place holder: need to move them out */
typedef enum BIP_RtspLiveMediaStreamingMode
{
    BIP_StreamingMode_eRTP_UDP,
    BIP_StreamingMode_eRTP_TCP,
    BIP_StreamingMode_eRAW_UDP
} BIP_RtspLiveMediaStreamingMode;

/* Stores additional Satellite Settings that can't be stored in Nexus Satellite or Diseqc Settings */
typedef struct BIP_AdditionalSatelliteSettings
{
    int signalSourceId; /*SATIP specific.   a satellite oribital positions. Usually a fixed mapping of srcId to Multiswitch Setting controlled by diseqc*/
} BIP_AdditionalSatelliteSettings;

typedef struct BIP_RtspTransportStatus
{
    BIP_RtspLiveMediaStreamingMode streamingMode;
    char         *clientAddressStr;
    unsigned char clientTTL;
    int           clientRTPPortNum;
    char         *serverAddressStr;
    bool          isMulticast;
    unsigned char multicastOctet;
    int           serverRTPPortNum;
} BIP_RtspTransportStatus;

typedef struct BIP_PidInfo
{
    int pidListCount;
    int* pidList;
} BIP_PidInfo;


#ifdef __cplusplus
}
#endif

#endif /* !defined BIP_TYPES_H */
