/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 ******************************************************************************
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
/**//**
 * \name    Basic constants.
 */
/**@{*/
#undef  UINT8_MAX
#define UINT8_MAX   0xFFU

#undef  INT32_MAX
#define INT32_MAX   0x7FFFFFFFL

#undef  TRUE
#define TRUE    true        /*!< Boolean constant \e true.  */

#undef  FALSE
#define FALSE   false       /*!< Boolean constant \e false. */
/**@}*/


/**//**
 * \name    Data types for binary values of different widths.
 */
/**@{*/
typedef uint32_t  BitField32_t;     /*!< Data type for 32-bit binary value. */
typedef uint16_t  BitField16_t;     /*!< Data type for 16-bit binary value. */
typedef uint8_t   BitField8_t;      /*!< Data type for 8-bit binary value. */
/**@}*/


/**//**
 * \brief   Compact 1-byte size boolean data type.
 */
typedef uint8_t  Bool8_t;


/**//**
 * \name    Data types for hardware registers declaration.
 * \details Use the appropriate \t regNN_t type according to the actual register width: 8, 16 or 32 bits. Use the
 *  \t regV_t type for converting pointers to registers of different widths between each other.
 */
/**@{*/
typedef volatile uint32_t  reg32_t;     /*!< Data type for 32-bit hardware register value. */
typedef volatile uint16_t  reg16_t;     /*!< Data type for 16-bit hardware register value. */
typedef volatile uint8_t   reg8_t;      /*!< Data type for 8-bit hardware register value. */
typedef volatile void      regV_t;      /*!< Data type for converting pointers to registers of different widths. */
/**@}*/


/**//**
 * \name    Macros for converting register address into the pointer to register of the specified width, and for
 *  converting pointers to registers of different widths between each other.
 * \param[in]   reg     Address of a register (value of the \t size_t type), or pointer to register (value of one of
 *  \t regNN_t types).
 * \return  Pointer to the register of the specified width: 8, 16, or 32 bits.
 */
#define SYS_REG32(reg)      ((reg32_t*)(regV_t*)(reg))
#define SYS_REG16(reg)      ((reg16_t*)(regV_t*)(reg))
#define SYS_REG8(reg)       ((reg8_t*)(regV_t*)(reg))


#endif /* _BB_SYS_TYPES_H */