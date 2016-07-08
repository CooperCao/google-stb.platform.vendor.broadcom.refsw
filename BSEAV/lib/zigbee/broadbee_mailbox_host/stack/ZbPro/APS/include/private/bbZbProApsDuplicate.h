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
 * FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/private/bbZbProApsDuplicate.h $
 *
 * DESCRIPTION:
 *   Declaration of the ZigBee PRO Duplicate Rejection Table component.
 *
 * $Revision: 2437 $
 * $Date: 2014-05-19 13:56:29Z $
 *
 ****************************************************************************************/

#ifndef _ZBPRO_APS_DUPLICATE_H
#define _ZBPRO_APS_DUPLICATE_H

/************************* INCLUDES ****************************************************/
#include "bbSysTable.h"

#include "bbZbProApsCommon.h"
#include "bbZbProApsKeywords.h"

#define ZBPRO_APS_DUPLICATE_ROWS        10
#define ZBPRO_APS_DUPLICATE_MASK_BITS   16
#define ZBPRO_APS_DUPLICATE_BUSY_BIT    7
#define ZBPRO_APS_DUPLICATE_MAX_TTL     127

/* These values specify how long Duplicate Rejection Table remembers received frames
 * (if there is enough memory). The ZigBee specification does not mention any value.
 * It depends on MaxDepth, Number of retries, random jitter delays, Indirect Poll Rate
 * TODO
 */
#define ZBPRO_APS_DUPLICATE_TIME_STEP   512 /* ms */
#define ZBPRO_APS_DUPLICATE_NEW_TTL     10  /* steps */

/************************* TYPES *******************************************************/

/**//**
 * \brief Rejection table entry
 */
typedef struct _ZbProApsDuplicateEntry_t
{
    ZBPRO_APS_ShortAddr_t   nwkAddr;
    ZbProApsCounter_t       counter;

    union
    {
        struct
        {
            BitField8_t     ttl     : ZBPRO_APS_DUPLICATE_BUSY_BIT;
            BitField8_t     isBusy  : 1;
        };
        uint8_t             rawBits;
    };

} ZbProApsDuplicateEntry_t;

/**//**
 * \brief Rejection table service descriptor.
 */
typedef struct _ZbProApsDuplicateDescriptor_t
{
    SYS_Time_t                  lastTimeStamp;
    ZbProApsDuplicateEntry_t    table[ZBPRO_APS_DUPLICATE_ROWS];
    BITMAP_DECLARE(bitmap, ZBPRO_APS_DUPLICATE_ROWS * ZBPRO_APS_DUPLICATE_MASK_BITS);
} ZbProApsDuplicateDescriptor_t;

/************************* FUNCTION PROTOTYPES ******************************************/

/**//**
 * \brief Initializes Rejection table
 */
APS_PRIVATE void zbProApsDuplicateReset(void);

/**//**
 * \brief Returns true if the specified Frame Data already set in the Duplicate Rejection Table. \n
 *        Otherwise adds new entry into the Rejection Table.
 */
APS_PRIVATE bool zbProApsDuplicated(ZBPRO_APS_ShortAddr_t nwkSrcAddr, ZbProApsCounter_t apsCounter);

/**//**
 * \brief Makes the rejection table forgets the specified frame
 */
APS_PRIVATE void zbProApsDuplicatedForget(ZBPRO_APS_ShortAddr_t nwkSrcAddr, ZbProApsCounter_t apsCounter);

#endif /* _ZBPRO_APS_DUPLICATE_H */