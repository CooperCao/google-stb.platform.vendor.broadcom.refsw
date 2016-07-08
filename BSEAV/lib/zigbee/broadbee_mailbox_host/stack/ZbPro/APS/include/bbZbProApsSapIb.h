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
 * FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/bbZbProApsSapIb.h $
 *
 * DESCRIPTION:
 *   This header describes API for the ZigBee PRO APS Information Base component.
 *
 * $Revision: 10263 $
 * $Date: 2016-02-29 18:03:06Z $
 *
 ****************************************************************************************/
#ifndef _ZBPRO_APS_SAP_IB_H
#define _ZBPRO_APS_SAP_IB_H

/************************* INCLUDES *****************************************************/
#include "bbZbProApsCommon.h"
#include "bbZbProNwkSapTypesIb.h"
#include "bbZbProApsSapSecurityTypes.h"

/************************* TYPES ********************************************************/
/**//**
 * \brief APS-IB attributes identifiers enumeration (see ZigBee spec r20, Table 2.24 and Table 4.38).
 */
typedef enum _ZbProApsIbAttributeId_t
{
    /* AIB Attributes */
    ZBPRO_APS_IB_FIRST_ID                       = 0xC1,
    ZBPRO_APS_IB_BIDING_TABLE_ID                = ZBPRO_APS_IB_FIRST_ID, //NOTE: table
    ZBPRO_APS_IB_DESIGNATED_COORDINATOR_ID      = 0xC2,
    ZBPRO_APS_IB_CHANNEL_MASK_ID                = 0xC3,
    ZBPRO_APS_IB_USE_EXTENDED_PANID_ID          = 0xC4,
    ZBPRO_APS_IB_GROUP_TABLE_ID                 = 0xC5, //NOTE: table
    ZBPRO_APS_IB_NONMEMBER_RADIUS_ID            = 0xC6,
    ZBPRO_APS_IB_PERMISSIONS_CONFIGURATION_ID   = 0xC7, //NOTE: table
    ZBPRO_APS_IB_USE_INSECURE_JOIN_ID           = 0xC8,
    ZBPRO_APS_IB_INTERFRAME_DELAY_ID            = 0xC9,
    ZBPRO_APS_IB_LAST_CHANNEL_ENERGY_ID         = 0xCA,
    ZBPRO_APS_IB_LAST_CHANNEL_FAILURE_RATE_ID   = 0xCB,
    ZBPRO_APS_IB_CHANNEL_TIMER_ID               = 0xCC,
    ZBPRO_APS_IB_MAX_WINDOW_SIZE_ID             = 0xCD, //NOTE: table
    ZBPRO_APS_IB_LAST_ID                        = ZBPRO_APS_IB_MAX_WINDOW_SIZE_ID,

    /* Attributes which are used by ZDO */
    ZBPRO_APS_IB_ZDO_ID_START                   = 0xCE,
    ZBPRO_APS_IB_MANUFACTURER_CODE_ID           = ZBPRO_APS_IB_ZDO_ID_START, // 2b
    ZBPRO_APS_IB_PERMIT_JOIN_DURATION_ID        = 0xCF, // 1b
    ZBPRO_APS_IB_SCAN_DURATION_ID               = 0xD0, // 1b
    ZBPRO_APS_IB_ZDP_RESPONSE_TIMEOUT_ID        = 0xD1, // 4b
    ZBPRO_APS_IB_END_DEV_BIND_TIMEOUT_ID        = 0xD2, // 4b
    ZBPRO_APS_IB_ZHA_RESPONSE_TIMEOUT_ID        = 0xD3, // 4b
    ZBPRO_APS_IB_ZDO_ID_END,

    /* Additional Security-Related AIB Attributes */
    ZBPRO_APS_IB_FIRST_SECURITY_ID              = 0xE0,
    ZBPRO_APS_IB_DEVICE_KEY_PAIR_SET_ID         = ZBPRO_APS_IB_FIRST_SECURITY_ID, //NOTE: table
    ZBPRO_APS_IB_TRUST_CENTER_ADDRESS_ID        = 0xE1,
    ZBPRO_APS_IB_SECURITY_TIME_OUT_PERIOD_ID    = 0xE2,
    ZBPRO_APS_IB_LAST_SECURITY_ID               = ZBPRO_APS_IB_SECURITY_TIME_OUT_PERIOD_ID,

    ZBPRO_APS_IB_ADDITIONAL_FIRST_SECURITY_ID   = 0xE3,
    ZBPRO_APS_IB_INITIAL_SECURITY_STATUS_ID     = ZBPRO_APS_IB_ADDITIONAL_FIRST_SECURITY_ID,
    ZBPRO_APS_IB_ADDITIONAL_LAST_SECURITY_ID    = ZBPRO_APS_IB_INITIAL_SECURITY_STATUS_ID,

} ZbProApsIbAttributeId_t;

/**//**
 * \brief Checks APS-IB attributes range.
 */
#define ZBPRO_APS_IB_IS_ATTRIBUTE_ID_VALID(attrId)                                                      \
    (                                                                                                   \
            (((attrId >= ZBPRO_APS_IB_FIRST_SECURITY_ID) && (attrId <= ZBPRO_APS_IB_LAST_SECURITY_ID))  \
            || ((attrId >= ZBPRO_APS_IB_ADDITIONAL_FIRST_SECURITY_ID) && (attrId <= ZBPRO_APS_IB_ADDITIONAL_LAST_SECURITY_ID))\
            || ((attrId >= ZBPRO_APS_IB_FIRST_ID) && (attrId <= ZBPRO_APS_IB_LAST_ID))                  \
            || ((attrId >= ZBPRO_APS_IB_ZDO_ID_START) && (attrId <= ZBPRO_APS_IB_ZDO_ID_END)))          \
            && (attrId != ZBPRO_APS_IB_DEVICE_KEY_PAIR_SET_ID)                                          \
            &&(attrId != ZBPRO_APS_IB_BIDING_TABLE_ID)                                                  \
            &&(attrId != ZBPRO_APS_IB_GROUP_TABLE_ID)                                                   \
            &&(attrId != ZBPRO_APS_IB_PERMISSIONS_CONFIGURATION_ID)                                     \
            &&(attrId != ZBPRO_APS_IB_MAX_WINDOW_SIZE_ID)                                               \
    )

/**//**
 * \brief APS-IB private attributes data types list macro.
 */
#define ZBPRO_APS_IB_ARRT_LIST                                                      \
    ZBPRO_APS_ExtAddr_t                             apsTrustCenterAddress;          \
    ZBPRO_APS_ExtPanId_t                            apsUseExtendedPANID;            \
    PHY_ChannelMask_t                               apsChannelMask;                 \
    SYS_Time_t                                      zdoZdpResponseTimeout;          \
    SYS_Time_t                                      zdoEdBindTimeout;               \
    SYS_Time_t                                      zclResponseTimeout;             \
    ZBPRO_APS_ManufacturerCode_t                    zdoManufacturerCode;            \
    ZBPRO_APS_SucurityTimeOutPeriod_t               apsSecurityTimeOutPeriod;       \
    ZBPRO_NWK_PermitJoinDuration_t                  zdoPermitJoinDuration;          \
    ZBPRO_NWK_ScanDuration_t                        zdoScanDuration;                \
    ZBPRO_APS_NwkRadius_t                           apsNonmemberRadius;             \
    ZBPRO_APS_InterframeDelay_t                     apsInterframeDelay;             \
    PHY_ED_t                                        apsLastChannelEnergy;           \
    ZBPRO_APS_FailureRate_t                         apsLastChannelFailureRate;      \
    ZBPRO_APS_ChannelTimer_t                        apsChannelTimer;                \
    Bool8_t                                         apsDesignatedCoordinator;       \
    Bool8_t                                         apsUseInsecureJoin;             \
    uint8_t                                         zdoInitialSecurityStatus;       \
    \
    uint16_t                                        apsReservedAlignment0

/**//**
 * \brief APS-IB private attributes variant data type.
 */
typedef union _ZbProApsIbAttributeValue_t
{
    ZBPRO_APS_IB_ARRT_LIST;
} ZbProApsIbAttributeValue_t;

/**//**
 * \brief APS-IB public attributes data types list macro.
 */
#define ZBPRO_APS_IB_PUBLIC_VARIANT     \
    union                               \
    {                                   \
        ZBPRO_NWK_NIB_PUBLIC_VARIANT;   \
        ZBPRO_APS_IB_ARRT_LIST;         \
    }

/**//**
 * \brief APS-IB public attributes variant data type.
 */
typedef ZBPRO_APS_IB_PUBLIC_VARIANT ZBPRO_APS_IbAttributeValue_t;

/**//**
 * \brief APS-IB attributes default values.
 */
#define ZBPRO_APS_IB_ATTR_DEFAULT_VALUE_TRUST_CENTER_ADDRESS            (0)
#define ZBPRO_APS_IB_ATTR_DEFAULT_VALUE_SECURITY_TIME_OUT_PERIOD        (20000) // NOTE: sets by profile
#define ZBPRO_APS_IB_ATTR_DEFAULT_VALUE_DESIGNATED_COORDINATOR          (true)
#define ZBPRO_APS_IB_ATTR_DEFAULT_VALUE_CHANNEL_MASK                    (0x07FFF800)
#define ZBPRO_APS_IB_ATTR_DEFAULT_VALUE_USE_EXTENDED_PANID              (ZBPRO_NWK_UNASSIGNED_EXTENDED_PANID)
#define ZBPRO_APS_IB_ATTR_DEFAULT_VALUE_NONMEMBER_RADIUS                (2)
#define ZBPRO_APS_IB_ATTR_DEFAULT_VALUE_USE_INSECURE_JOIN               (true)
#define ZBPRO_APS_IB_ATTR_DEFAULT_VALUE_INTERFRAME_DELAY                (0x00) //TODO: set valid value after the fragmentation feature implementation
#define ZBPRO_APS_IB_ATTR_DEFAULT_VALUE_LAST_CHANNEL_ENERGY             (0)
#define ZBPRO_APS_IB_ATTR_DEFAULT_VALUE_LAST_CHANNEL_FAILURE_RATE       (0)
#define ZBPRO_APS_IB_ATTR_DEFAULT_VALUE_CHANNEL_TIMER                   (1)

#define ZBPRO_APS_IB_ATTR_DEFAULT_VALUE_MANUFACTURER_CODE               (0)
#define ZBPRO_APS_IB_ATTR_DEFAULT_VALUE_PERMIT_JOIN_DURATION            (0xff)
#define ZBPRO_APS_IB_ATTR_DEFAULT_VALUE_ZDP_RESPONSE_TIMEOUT            (15000)
#define ZBPRO_APS_IB_ATTR_DEFAULT_VALUE_END_DEV_BIND_TIMEOUT            (60000)
#define ZBPRO_APS_IB_ATTR_DEFAULT_VALUE_ZHA_RESPONSE_TIMEOUT            (5000)
#define ZBPRO_APS_IB_ATTR_DEFAULT_VALUE_SCAN_DURATION                   (5)
#define ZBPRO_APS_IB_ATTR_DEFAULT_VALUE_INITIAL_SECURITY_STATUS         (ZBPRO_APS_PRECONFIGURED_TRUST_CENTER_KEY)

/**//**
 * \brief APS-IB attributes values constraints.
 */
#define ZBPRO_APS_IB_ATTR_MIN_ALLOWED_VALUE_TRUST_CENTER_ADDRESS        (0)
#define ZBPRO_APS_IB_ATTR_MAX_ALLOWED_VALUE_TRUST_CENTER_ADDRESS        (0xfffffffffffffffeull)
#define ZBPRO_APS_IB_ATTR_MIN_ALLOWED_VALUE_SECURITY_TIME_OUT_PERIOD    (0)
#define ZBPRO_APS_IB_ATTR_MAX_ALLOWED_VALUE_SECURITY_TIME_OUT_PERIOD    (0xffff)
#define ZBPRO_APS_IB_ATTR_MIN_ALLOWED_VALUE_INITIAL_SECURITY_STATUS     (0)
#define ZBPRO_APS_IB_ATTR_MAX_ALLOWED_VALUE_INITIAL_SECURITY_STATUS     (3)
#define ZBPRO_APS_IB_ATTR_MIN_ALLOWED_VALUE_DESIGNATED_COORDINATOR      (0)
#define ZBPRO_APS_IB_ATTR_MAX_ALLOWED_VALUE_DESIGNATED_COORDINATOR      (1)
#define ZBPRO_APS_IB_ATTR_MIN_ALLOWED_VALUE_CHANNEL_MASK                (0)
#define ZBPRO_APS_IB_ATTR_MAX_ALLOWED_VALUE_CHANNEL_MASK                (0x07ffffff)
#define ZBPRO_APS_IB_ATTR_MIN_ALLOWED_VALUE_USE_EXTENDED_PANID          (0)
#define ZBPRO_APS_IB_ATTR_MAX_ALLOWED_VALUE_USE_EXTENDED_PANID          (0xfffffffffffffffeull)
#define ZBPRO_APS_IB_ATTR_MIN_ALLOWED_VALUE_NONMEMBER_RADIUS            (0)
#define ZBPRO_APS_IB_ATTR_MAX_ALLOWED_VALUE_NONMEMBER_RADIUS            (7)
#define ZBPRO_APS_IB_ATTR_MIN_ALLOWED_VALUE_USE_INSECURE_JOIN           (0)
#define ZBPRO_APS_IB_ATTR_MAX_ALLOWED_VALUE_USE_INSECURE_JOIN           (1)
#define ZBPRO_APS_IB_ATTR_MIN_ALLOWED_VALUE_INTERFRAME_DELAY            (0)
#define ZBPRO_APS_IB_ATTR_MAX_ALLOWED_VALUE_INTERFRAME_DELAY            (0xff)
#define ZBPRO_APS_IB_ATTR_MIN_ALLOWED_VALUE_LAST_CHANNEL_ENERGY         (0)
#define ZBPRO_APS_IB_ATTR_MAX_ALLOWED_VALUE_LAST_CHANNEL_ENERGY         (0xff)
#define ZBPRO_APS_IB_ATTR_MIN_ALLOWED_VALUE_LAST_CHANNEL_FAILURE_RATE   (0)
#define ZBPRO_APS_IB_ATTR_MAX_ALLOWED_VALUE_LAST_CHANNEL_FAILURE_RATE   (100)
#define ZBPRO_APS_IB_ATTR_MIN_ALLOWED_VALUE_CHANNEL_TIMER               (1)
#define ZBPRO_APS_IB_ATTR_MAX_ALLOWED_VALUE_CHANNEL_TIMER               (24)

#define ZBPRO_APS_IB_ATTR_MIN_ALLOWED_VALUE_MANUFACTURER_CODE           (0)
#define ZBPRO_APS_IB_ATTR_MAX_ALLOWED_VALUE_MANUFACTURER_CODE           (0xffff)
#define ZBPRO_APS_IB_ATTR_MIN_ALLOWED_VALUE_PERMIT_JOIN_DURATION        (0)
#define ZBPRO_APS_IB_ATTR_MAX_ALLOWED_VALUE_PERMIT_JOIN_DURATION        (0xff)
#define ZBPRO_APS_IB_ATTR_MIN_ALLOWED_VALUE_ZDP_RESPONSE_TIMEOUT        (0)
#define ZBPRO_APS_IB_ATTR_MAX_ALLOWED_VALUE_ZDP_RESPONSE_TIMEOUT        (0xffffffff)
#define ZBPRO_APS_IB_ATTR_MIN_ALLOWED_VALUE_END_DEV_BIND_TIMEOUT        (0)
#define ZBPRO_APS_IB_ATTR_MAX_ALLOWED_VALUE_END_DEV_BIND_TIMEOUT        (0xffffffff)
#define ZBPRO_APS_IB_ATTR_MIN_ALLOWED_VALUE_ZHA_RESPONSE_TIMEOUT        (0)
#define ZBPRO_APS_IB_ATTR_MAX_ALLOWED_VALUE_ZHA_RESPONSE_TIMEOUT        (0xffffffff)
#define ZBPRO_APS_IB_ATTR_MIN_ALLOWED_VALUE_SCAN_DURATION               (0)
#define ZBPRO_APS_IB_ATTR_MAX_ALLOWED_VALUE_SCAN_DURATION               (0xff)

#endif /* _ZBPRO_APS_SAP_IB_H */
