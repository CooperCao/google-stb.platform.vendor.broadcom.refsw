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
* FILENAME: $Workfile: trunk/stack/ZbPro/TC/include/private/bbZbProTcMem.h $
*
* DESCRIPTION:
*   Trust Center Memory
*
* $Revision: 2286 $
* $Date: 2014-04-25 12:12:32Z $
*
*****************************************************************************************/

#ifndef _BB_ZBPRO_TC_MEMORY_H
#define _BB_ZBPRO_TC_MEMORY_H

/************************* INCLUDES *****************************************************/
#include "bbZbProTcPolicy.h"
#include "bbZbProTcKeywords.h"

#include "private/bbZbProTcAuthenticate.h"
#include "private/bbZbProTcNwkKeyUpdatePrivate.h"

/************************** DEFINITIONS *************************************************/
#define ZBPRO_TC_MEMORY_POOL_SIZE       4

/***************************** TYPES ****************************************************/

/**//**
 * \brief TC handler buffer
 */
typedef struct _ZbProTcMemBuf_t
{
    union
    {
        ZbProTcAuthenticateBuffer_t     authenticateBuffer;
        ZbProTcNwkKeyUpdateBuffer_t     nwkKeyUpdateBuffer;
    };
    Bool8_t busy;
} ZbProTcMemBuf_t;

/**//**
 * \brief TC Memory
 */
typedef struct _ZbProTcMemDescr_t
{
    SYS_SchedulerTaskDescriptor_t   taskDescriptor;
    ZbProTcPolicy_t                 policy;
    ZbProTcNwkKeyUpdateDesc_t       nwkKeyUpdateDesc;
    ZbProTcMemBuf_t                 bufferPool[ZBPRO_TC_MEMORY_POOL_SIZE];
} ZbProTcMemDescr_t;

extern ZbProTcMemDescr_t zbProTcMemDescr;

/*********************** INLINE FUNCTIONS ***********************************************/

/**//**
 * \brief Gets the TC memory descriptor
 */
INLINE ZbProTcMemDescr_t *ZbProTcMemGet(void)
{
    return &zbProTcMemDescr;
}

/************************ PROTOTYPES ****************************************************/

/**//**
 * \brief Allocates a TC buffer
 */
TC_PRIVATE ZbProTcMemBuf_t *ZbProTcMemBufferGet(void);

/**//**
 * \brief Frees a TC buffer
 */
TC_PRIVATE void ZbProTcMemBufferFree(ZbProTcMemBuf_t *buffer);

#endif /* _BB_ZBPRO_TC_MEMORY_H */