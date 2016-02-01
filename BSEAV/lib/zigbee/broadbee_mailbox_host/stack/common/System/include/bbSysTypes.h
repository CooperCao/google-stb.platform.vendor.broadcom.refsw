/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/common/System/include/bbSysTypes.h $
*
* DESCRIPTION:
*   Common System Types definition.
*
* $Revision: 3968 $
* $Date: 2014-10-09 02:26:13Z $
*
*****************************************************************************************/


#ifndef _BB_SYS_TYPES_H
#define _BB_SYS_TYPES_H


/************************* INCLUDES *****************************************************/
#include "bbSysOptions.h"           /* Compiler and SYS options setup.   */
#include "bbSysKeywords.h"          /* SYS macro keywords definition.    */
#if !(defined(_SOC_MAC_TEST_) || defined(_SOC_PHY_TEST_))
#include "bbMailConfig.h"           /* Mailbox macro declarations. */
#endif
#include <stdint.h>                 /* C99 compliant integer data types. */
#include <stdbool.h>                /* C99 compliant boolean data type.  */
#include <stddef.h>                 /* ANSI compliant size data type.    */
#include <string.h>                 /* ANSI compliant strings data type. */
#include <limits.h>                 /* Sizes of integer types */

#if defined(__arc__)
# include <alloca.h>                /* Declaration of alloca() function. */
# define ALLOCA(x)          alloca(x)
#else
# include <malloc.h>                /* Declaration of alloca() function. */
# define ALLOCA(x)          __builtin_alloca(x)
#endif


/************************* DEFINITIONS **************************************************/
/*
 * Basic constants.
 */
#undef  UINT8_MAX
#define UINT8_MAX   0xFFU

#undef  INT32_MAX
#define INT32_MAX   0x7FFFFFFFL

#undef  TRUE
#define TRUE    true        /*!< Boolean constant \e true.  */

#undef  FALSE
#define FALSE   false       /*!< Boolean constant \e false. */

/**//**
 * \brief Basic data type for 8-bit binary data fields assembling.
 */
typedef uint8_t  BitField8_t;


/**//**
 * \brief Basic data type for 16-bit binary data fields assembling.
 */
typedef uint16_t  BitField16_t;


/**//**
 * \brief Basic data type for 32-bit binary data fields assembling.
 */
typedef uint32_t  BitField32_t;


/**//**
 * \brief Compact 1-byte size boolean data type.
 */
typedef uint8_t  Bool8_t;


#endif /* _BB_SYS_TYPES_H */