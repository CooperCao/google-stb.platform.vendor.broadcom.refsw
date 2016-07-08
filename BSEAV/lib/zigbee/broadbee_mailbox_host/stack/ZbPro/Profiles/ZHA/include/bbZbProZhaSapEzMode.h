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
* FILENAME: $Workfile: trunk/stack/ZbPro/Profiles/ZHA/include/bbZbProZhaSapEzMode.h $
*
* DESCRIPTION:
*   ZHA Profile configuration.
*
* $Revision: 8021 $
* $Date: 2015-08-11 12:05:24Z $
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZHA_SAP_EZ_MODE_H
#define _BB_ZBPRO_ZHA_SAP_EZ_MODE_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZhaCommon.h"

typedef enum
{
    ZBPRO_EZ_SUCCESS,
    ZBPRO_EZ_CANNOT_START,
    ZBPRO_EZ_WRONG_EP,
} ZBPRO_ZHA_EzModeStatus_t;

typedef enum
{
    ZBPRO_EZ_STEERING_ONLY,
    ZBPRO_EZ_TARGET,
    ZBPRO_EZ_INITIATOR,
} ZBPRO_ZHA_EzModeRole_t;

typedef struct _ZBPRO_ZHA_EzModeReqParams_t
{
    uint32_t                roundTimeMs;
    uint8_t                 times;
    ZBPRO_ZHA_EzModeRole_t  ezRole;
    Bool8_t                 factoryFresh;
    ZBPRO_APS_EndpointId_t  endPoint;
} ZBPRO_ZHA_EzModeReqParams_t;

typedef struct _ZBPRO_ZHA_EzModeConfParams_t
{
    ZBPRO_ZHA_EzModeStatus_t status;
} ZBPRO_ZHA_EzModeConfParams_t;

typedef struct _ZBPRO_ZHA_EzModeReqDescr_t ZBPRO_ZHA_EzModeReqDescr_t;

typedef void (*ZBPRO_ZHA_EzModeCallback_t)(ZBPRO_ZHA_EzModeReqDescr_t *const reqDescr,
        ZBPRO_ZHA_EzModeConfParams_t *const confParams);

struct _ZBPRO_ZHA_EzModeReqDescr_t
{
    SYS_QueueElement_t   queueElement; // NOTE: may be need to replace ZbProZhaServiceField_t...
    ZBPRO_ZHA_EzModeReqParams_t   params;
    ZBPRO_ZHA_EzModeCallback_t    callback;
};

void ZBPRO_ZHA_EzModeReq(ZBPRO_ZHA_EzModeReqDescr_t *reqDescr);

#endif /* _BB_ZBPRO_ZHA_SAP_EZ_MODE_H */
