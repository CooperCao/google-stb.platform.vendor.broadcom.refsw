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
 * FILENAME: $Workfile: trunk/stack/common/System/include/bbSysMemManData.h $
 *
 * DESCRIPTION:
 *   This is the header file for the Memory Manager component internal data structure
 *   definition and access macros.
 *
 * $Revision: 1195 $
 * $Date: 2014-01-23 13:03:59Z $
 *
 ****************************************************************************************/
#ifndef _SYS_MEM_MAN_DATA_H
#define _SYS_MEM_MAN_DATA_H

/************************* INCLUDES ****************************************************/
#include "bbSysBasics.h"            /* Basic system environment set. */
#include "bbSysMemMan.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief Type of the Memory Manager internal data structure.
 */
typedef struct _MM_InternalData_t
{
    Mm_Char_t *startAddress;            /**!< A starting address of the memory pool. */
    Mm_Char_t blockSize;                /**!< A size of the memory block. */
    Mm_Size_t numBlocks;                /**!< A total amount of memory blocks available. */
    Mm_Char_t numExtraBitmaps;          /**!< A number of additional bits in the memory block ID. */
} MM_InternalData_t;

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief Common Stack Static Structure member.
 */
#define SYS_STACK_DATA_MM_FIELD()       MM_InternalData_t MM_StackDataField;

/**//**
 * \brief Common Stack Static Structure member access.
 */
#define GET_SYS_STACK_DATA_MM_FIELD() (&sysCommonStackData.MM_StackDataField)

/**//**
 * \brief Common Stack Static Structure member initialization.
 */

#define INIT_SYS_STACK_DATA_MM_FIELD() \
.MM_StackDataField = \
{ \
    .startAddress = (Mm_Char_t *)NULL, \
    .blockSize = 0, \
    .numBlocks = 0, \
    .numExtraBitmaps = 0, \
},

#endif /* _SYS_MEM_MAN_DATA_H */