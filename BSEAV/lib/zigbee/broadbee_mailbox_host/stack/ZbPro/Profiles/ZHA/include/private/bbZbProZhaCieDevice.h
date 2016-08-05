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
/*
* FILENAME: $Workfile: $
*
* DESCRIPTION:
*   ZHA Profile CIE device handler.
*
* $Revision: $
* $Date: $
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZHA_CIE_DEVICE_H
#define _BB_ZBPRO_ZHA_CIE_DEVICE_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZhaCommon.h"
#include "bbZbProZhaSapCieDevice.h"
#include "bbZbProNwkSapTypesGetSet.h"
#include "bbZbProZdoSapTypesGetSimpleDescHandler.h"
#include "private/bbZbProZhaCieZones.h"
#include "private/bbZbProZhaCieCmdPull.h"
#include "private/bbZbProZhaCieZoneEnrollment.h"
#include "private/bbZbProZhaCiePanel.h"
#include "private/bbZbProZhaCieUserRules.h"

/************************* TYPEDEFS *****************************************************/

/**//**
 * \brief Structure for the CIE Device static data.
*/
typedef struct _ZbProZhaCieDeviceMemDescr_t
{
    SYS_QueueDescriptor_t reqQueue;

    ZbProZhaCieZonesDescriptor_t            zhaCieZonesDescr;
    ZbProZhaCieCmdPullDescriptor_t          zhaCieCmdPullDescr;
    ZbProZhaCieZoneEnrollmentDescriptor_t   zhaCieZoneEnrollmentDescr;
    ZbProZhaCiePanelDescriptor_t            zhaCiePanelDescr;
    ZbProZhaCieUserRulesDescriptor_t        zhaCieUserRulesDescr;
    Bool8_t                                 zhaCieIsRegistered;
    ZBPRO_APS_EndpointId_t                  zhaCieEndpoint;
    ZBPRO_ZDO_SimpleDescReqDescr_t          zhaCieSimpleDescr;
    ZBPRO_NWK_GetReqDescr_t                 zhaCieNwkGetDescr;
} ZbProZhaCieDeviceMemDescr_t;

/************************* FUNCTION PROTOTYPES ******************************************/
/**//**
 * \brief Handler to proceed the CIE Device events.
*/
void zbProZhaCieDeviceHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/**//**
 * \brief Reset function for the CIE Device. Should be called at the start
 * of the stack after RESET.
*/
void zbProZhaCieDeviceReset(void);

#endif /* _BB_ZBPRO_ZHA_CIE_DEVICE_H */
