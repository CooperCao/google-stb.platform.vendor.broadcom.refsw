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
 *
 * FILENAME: $Workfile: trunk/stack/ZbPro/ZDO/include/private/bbZbProZdoIb.h $
 *
 * DESCRIPTION:
 *   ZDO information base private declarations.
 *
 * $Revision: 10263 $
 * $Date: 2016-02-29 18:03:06Z $
 *
 ****************************************************************************************/
#ifndef _ZBPRO_ZDO_IB_H
#define _ZBPRO_ZDO_IB_H

/************************* INCLUDES *****************************************************/
#include "bbZbProZdoCommon.h"

/************************* TYPES ********************************************************/
/**//**
 * \brief Descriptor capability mask type, the node descriptor part. For more detailed please see spec R20 chapter 2.3.2.3.12 p.86
 */
typedef union _ZbProZdoDescriptorCapability_t
{
    BitField8_t    plain;
    struct
    {
        BitField8_t    extendedActiveEndpointListAvailable : 1;
        BitField8_t    extendedSimpleDescriptorListAvailable : 1;
        BitField8_t    reserved : 6;
    };
} ZbProZdoDescriptorCapability_t;
/**//**
 * \brief Node descriptor. For more detailed please see spec R20 chapter 2.3.2.3 p.82
 */
typedef struct _ZbProZdoNodeDescriptor_t
{
    ZBPRO_APS_ManufacturerCode_t    manufacturerCode;
    ZBPRO_ZDO_ServerMask_t          serverMask;
    ZBPRO_APS_MaxDuSize_t           maximumIncomingTransferSize;
    ZBPRO_APS_MaxDuSize_t           maximumOutgoingTransferSize;
    ZBPRO_NWK_Capability_t          macCapabilityFlags;
    ZBPRO_NWK_MaxDuSize_t           maximumBufferSize;
    ZbProZdoDescriptorCapability_t  descriptorCapabilityField;
    PHY_Page_t                      frequenceBand;
    ZBPRO_NWK_DeviceType_t          logicalType;
    Bool8_t                         complexDescriptorAvailable;
    Bool8_t                         userDescriptorAvailable;
    // APS flags - not used in this specification
} ZbProZdoNodeDescriptor_t;

/**//**
 * \brief Power mode enumeration, part of power descriptor. For more detailed please see spec R20 chapter 2.3.2.4.1 p.87
 */
typedef enum _ZbProZdoPowerMode_t
{
    ZBPRO_ZDO_RECEIVER_SYNCHRONIZED_WHEN_ON_IDLE    = 0,
    ZBPRO_ZDO_RECEIVER_COMES_PERIODICALLY           = 1,
    ZBPRO_ZDO_RECEIVER_COMESWHEN_STIMULATED         = 2,
} ZbProZdoPowerMode_t;

/**//**
 * \brief Values of the Current Power source field of the power descriptor.
 *        Values are deduced from the appropriate bit numbers in the spec R20 chapter 2.3.2.4.3 p.87
 */
typedef enum _ZbProZdoPowerSource_t
{
    ZBPRO_ZDO_CONSTANT_POWER        = 1,
    ZBPRO_ZDO_RECHARGEABLE_POWER    = 2,
    ZBPRO_ZDO_DISPOSABLE_POWER      = 4
} ZbProZdoPowerSource_t;

/**//**
 * \brief Power source level enumeration, part of power descriptor. For more detailed please see spec R20 chapter 2.3.2.4.4 p.88
 */
typedef enum _ZbProZdoPowerLevel_t
{
    ZBPRO_ZDO_POWER_LEVEL_CRITICAL              = 0U,
    ZBPRO_ZDO_POWER_LEVEL_33_PERCENTS           = 4U,
    ZBPRO_ZDO_POWER_LEVEL_66_PERCENTS           = 8U,
    ZBPRO_ZDO_POWER_LEVEL_100_PERCENTS          = 12U,
} ZbProZdoPowerLevel_t;

/**//**
 * \brief Power descriptor. For more detailed please see spec R20 chapter 2.3.2.4 p.86
 */
typedef struct _ZbProZdoPowerDescriptor_t
{
    ZbProZdoPowerMode_t         currentPowerMode;
    ZbProZdoPowerSource_t       currentPowerSource;
    ZbProZdoPowerLevel_t        currentPowerLevel;
    Bool8_t                     constantPowerAvailable;
    Bool8_t                     rechargeableBatteryAvailable;
    Bool8_t                     disposableBatteryAvailable;
} ZbProZdoPowerDescriptor_t;

/**//**
 * \brief ZDO-IB attributes storage type.
 */
typedef struct _ZbProZdoIb_t
{
    ZbProZdoNodeDescriptor_t    nodeDescriptor;
    ZbProZdoPowerDescriptor_t   powerDescriptor;
} ZbProZdoIb_t;

/*************************************************************************************//**
  \brief Sets Zdo attributes to their default values (compiles their from information base attributes).
*****************************************************************************************/
void zbProZdoIbCompouseDeviceDescriptors(void);

#endif /* _ZBPRO_ZDO_IB_H */
