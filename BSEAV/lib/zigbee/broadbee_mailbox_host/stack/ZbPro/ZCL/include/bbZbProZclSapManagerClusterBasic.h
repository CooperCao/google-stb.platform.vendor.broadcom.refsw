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
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   Basic cluster ZCL Manager SAP interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZCL_SAP_MANAGER_CLUSTER_BASIC_H
#define _BB_ZBPRO_ZCL_SAP_MANAGER_CLUSTER_BASIC_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"
#include "bbZbProZclSapClusterBasic.h"

/************************* DEFINITIONS **************************************************/

/**//**
 * \brief Declarations of ZCL Attributes
 * \note: use ZBPRO_ZCL_ATTR_DECLARE(clusterId, attrId, ZBPRO_ZCL_AttrDataType, actualType, defaultValue, zbProZclAttrFlagMask, accessFunc)
 */
#define ZBPRO_ZCL_CLUSTER_BASIC_ATTR_DECLARATIONS \
    ZBPRO_ZCL_ATTR_DECLARE( \
            ZBPRO_ZCL_CLUSTER_ID_BASIC, \
            ZBPRO_ZCL_CLUSTER_BASIC_ATTR_ID_ZCL_VERSION, \
            ZBPRO_ZCL_ATTR_DATA_TYPE_UINT_8BIT, \
            ZBPRO_ZCL_ClusterBasicAttrZclVersion_t, \
            0x01, \
            ZBPRO_ZCL_ATTR_FLAG_MASK_DECLARE( \
                ZBPRO_ZCL_ATTR_FLAG_BIT_PERMISSION_READ, \
                ZBPRO_ZCL_ATTR_FLAG_BIT_PERMISSION_READ, \
                ZBPRO_ZCL_ATTR_FLAG_BIT_PERMISSION_READ, \
                ZBPRO_ZCL_ATTR_FLAG_BIT_PER_DEVICE \
            ), \
            NULL) \
    ZBPRO_ZCL_ATTR_DECLARE( \
            ZBPRO_ZCL_CLUSTER_ID_BASIC, \
            ZBPRO_ZCL_CLUSTER_BASIC_ATTR_ID_POWER_SOURCE, \
            ZBPRO_ZCL_ATTR_DATA_TYPE_UINT_8BIT, \
            ZBPRO_ZCL_ClusterBasicAttrPowerSource_t, \
            1 /* Mains (single phase) */, \
            ZBPRO_ZCL_ATTR_FLAG_MASK_DECLARE( \
                ZBPRO_ZCL_ATTR_FLAG_BIT_PERMISSION_READ | ZBPRO_ZCL_ATTR_FLAG_BIT_PERMISSION_WRITE, \
                ZBPRO_ZCL_ATTR_FLAG_BIT_PERMISSION_READ, \
                ZBPRO_ZCL_ATTR_FLAG_BIT_PERMISSION_READ, \
                ZBPRO_ZCL_ATTR_FLAG_BIT_PER_DEVICE \
            ), \
            NULL) \

/************************* TYPES *******************************************************/

/**//**
 * \brief ZCLVersion attribute type
 */
typedef uint8_t ZBPRO_ZCL_ClusterBasicAttrZclVersion_t;

/**//**
 * \brief PowerSource attribute type
 */
typedef uint8_t ZBPRO_ZCL_ClusterBasicAttrPowerSource_t;

/**//**
 * \brief Power Source enumeration
 */
typedef enum _ZBPRO_ZCL_PowerSource_t
{
    ZBPRO_ZCL_POWER_SOURCE_UNKNOWN                  = 0x00,
    ZBPRO_ZCL_POWER_SOURCE_MAINS_1PHASE             = 0x01,
    ZBPRO_ZCL_POWER_SOURCE_MAINS_3PHASE             = 0x02,
    ZBPRO_ZCL_POWER_SOURCE_BATTERY                  = 0x03,
    ZBPRO_ZCL_POWER_SOURCE_DC                       = 0x04,
    ZBPRO_ZCL_POWER_SOURCE_EMERGENCY_MAINS_ALWAYS   = 0x05,
    ZBPRO_ZCL_POWER_SOURCE_EMERGENCY_MAINS_SWITCH   = 0x06,
    ZBPRO_ZCL_POWER_SOURCE_LAST
} ZBPRO_ZCL_PowerSource_t;

/**//**
 * \brief HA Basic Cluster Power request parameters
 */
typedef struct _ZBPRO_ZCL_SetPowerSourceReqParams_t
{
    Bool8_t                     hasBatteryBackup;
    ZBPRO_ZCL_PowerSource_t     source;
} ZBPRO_ZCL_SetPowerSourceReqParams_t;

/**//**
 * \brief HA Basic Cluster Power request confirmation parameters
 */
typedef struct _ZBPRO_ZCL_SetPowerSourceConfParams_t
{
    ZBPRO_ZCL_Status_t          status;
} ZBPRO_ZCL_SetPowerSourceConfParams_t;

/**//**
 * \brief HA Basic Cluster Power request descriptor
 */
typedef struct _ZBPRO_ZCL_SetPowerSourceReqDescr_t  ZBPRO_ZCL_SetPowerSourceReqDescr_t;

/**//**
 * \brief HA Basic Cluster Power request confirmation callback
 */
typedef void (ZBPRO_ZCL_SetPowerSourceConfCallback_t)(ZBPRO_ZCL_SetPowerSourceReqDescr_t      *const reqDescr,
                                                    ZBPRO_ZCL_SetPowerSourceConfParams_t    *const confParams);

/**//**
 * \brief HA Basic Cluster Power request descriptor structure
 */
struct _ZBPRO_ZCL_SetPowerSourceReqDescr_t
{
    /* service field */

    ZBPRO_ZCL_SetPowerSourceConfCallback_t   *callback;
    ZBPRO_ZCL_SetPowerSourceReqParams_t      params;
};

/************************* PROTOTYPES ***************************************************/

/**//**
 * \brief ZCL HA Basic Cluster Power request
 */
void ZBPRO_ZCL_SetPowerSourceReq(ZBPRO_ZCL_SetPowerSourceReqDescr_t *const reqDescr);


#endif /* _BB_ZBPRO_ZCL_SAP_MANAGER_CLUSTER_BASIC_H */
