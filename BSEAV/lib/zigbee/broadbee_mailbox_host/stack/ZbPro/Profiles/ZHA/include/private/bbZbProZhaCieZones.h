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
* FILENAME: $Workfile: $
*
* DESCRIPTION:
*   ZHA CIE device zones table definitions.
*
* $Revision: 7361 $
* $Date: 2015-07-08 17:17:17Z $
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZHA_CIE_ZONES_TABLE_H_
#define _BB_ZBPRO_ZHA_CIE_ZONES_TABLE_H_

/************************* INCLUDES *****************************************************/
#include "bbZbProZclSapClusterIasZone.h"
#include "bbZbProZclSapClusterIasAce.h"
#include "bbZbProZhaSapCieDevice.h"
#include "bbZbProZhaCommon.h"

typedef  ZBPRO_ZCL_SapIASZoneAttributeZoneStatus_t  ZbProZhaCieZoneStatus_t;

typedef  ZBPRO_ZCL_SapIasAceAudibleNotification_t  ZbProZhaCieZoneAudibleNotification_t;

typedef struct
{
    ZBPRO_APS_ExtAddr_t                     addr;
    ZBPRO_ZCL_SapIASZoneAttributeZoneType_t type;
    ZBPRO_APS_EndpointId_t                  ep;
    ZBPRO_ZHA_CieZoneBypassState_t          bypassState;
    ZbProZhaCieZoneStatus_t                 status;
    SYS_DataPointer_t                       label;  /*!< Zone Label -  character string without first
                                                         length byte. SYS_EMPTY_PAYLOAD means, that zone
                                                         have no label. */
    ZbProZhaCieZoneAudibleNotification_t    audibleNotification;

} ZbProZhaCieZoneTableEntry_t;

typedef struct
{
    ZbProZhaCieZoneTableEntry_t table[ZBPRO_ZHA_CIE_ZONES_TABLE_SIZE];
} ZbProZhaCieZonesDescriptor_t;

typedef uint8_t ZbProZhaCieZoneId_t;
SYS_DbgAssertStatic(UINT8_MAX >= ZBPRO_ZHA_CIE_ZONES_TABLE_SIZE);

/************************* PROTOTYPES ***************************************************/
void zbProZhaCieZonesReset(void);
bool zbProZhaCieZonesUpdateTable(ZbProZhaCieZoneTableEntry_t *const newZone, ZbProZhaCieZoneId_t *zoneId);
ZbProZhaCieZoneTableEntry_t *zbProZhaCieZonesGetByZone(ZbProZhaCieZoneTableEntry_t *const newZone);
ZbProZhaCieZoneTableEntry_t *zbProZhaCieZonesGetById(const ZbProZhaCieZoneId_t id);

/**//**
 * \brief By specification arming/disarming must reset the bypassed status of the all zones which were bypassed to
 * NOT_BYPASSED state.
*/
void zbProZhaCieZonesResetAllBypassedZones(void);

/**//**
 * \brief Checking if zone with Zone ID == id could be bypassed.
 * \note It must be defined by some user configuration or by the policy.
*/
bool zbProZhaCieZonesCouldBeBypassed(const ZbProZhaCieZoneId_t id);

#endif /* _BB_ZBPRO_ZHA_CIE_ZONES_TABLE_H_ */