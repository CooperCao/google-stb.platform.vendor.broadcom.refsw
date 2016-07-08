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
* FILENAME: $Workfile: trunk/stack/common/HAL/include/bbHalCcmData.h $
*
* DESCRIPTION:
*   CCM* routines HAL handler static data.
*
* $Revision: 2595 $
* $Date: 2014-06-03 15:11:16Z $
*
*****************************************************************************************/


#ifndef _BB_HAL_CCM_DATA_H
#define _BB_HAL_CCM_DATA_H


/************************* INCLUDES *****************************************************/
#include "bbHalCcm.h"            /* HAL CCM* header. */

/************************* TYPES *******************************************************/
/**//**
 * \brief Type of the HAL CCM* internal data structure.
 */
typedef struct _HAL_CcmInternalData_t
{
    const uint8_t *key;     /*!< The used security key. */
    uint8_t data[16];       /*!< The internal IO buffer. */
} HAL_CcmInternalData_t;

/************************* DEFINITIONS *************************************************/
#ifdef SECURITY_EMU

/**//**
 * \brief Common HAL CCM Structure member.
 */
#define HAL_CCM_DATA_FIELD()       HAL_CcmInternalData_t HAL_CcmDataField;

/**//**
 * \brief Common HAL CCM Structure member access.
 */
#define GET_HAL_CCM_DATA_FIELD() (&sysCommonStackData.HAL_CcmDataField)

/**//**
 * \brief Common HAL CCM Structure member initialization.
 */

#define INIT_HAL_CCM_DATA_FIELD() \
.HAL_CcmDataField = \
{ \
    .key = NULL, \
},

#else /* SECURITY_EMU */

#define HAL_CCM_DATA_FIELD()
#define GET_HAL_CCM_DATA_FIELD()
#define INIT_HAL_CCM_DATA_FIELD()

#endif /* SECURITY_EMU */

#endif /* _BB_HAL_CCM_DATA_H */