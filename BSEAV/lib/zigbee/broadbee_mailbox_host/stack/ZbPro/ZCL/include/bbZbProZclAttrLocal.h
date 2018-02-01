/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

/******************************************************************************
*
* DESCRIPTION:
*       ZCL Attribute interface
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZCL_ATTR_LOCAL_H
#define _BB_ZBPRO_ZCL_ATTR_LOCAL_H

/************************* INCLUDES *****************************************************/
#include "bbZbProZclSapProfileWideAttributes.h"
#include "bbZbProZclSapManagerClusterBasic.h"
#include "bbZbProZclSapManagerClusterIdentify.h"
#include "bbZbProZclSapManagerClusterDiagnostic.h"

/************************* DEFINITIONS **************************************************/

/**//**
 * \brief Declarations of ZCL Attributes
 * \note: use ZBPRO_ZCL_ATTR_DECLARE(clusterId, attrId, ZBPRO_ZCL_AttrDataType, actualType,
 *                                   defaultValue, zbProZclAttrFlagMask, accessFunc)
 */
#ifdef _CLUSTER_DIAGNOSTIC_
#define ZBPRO_ZCL_ATTR_DECLARATIONS \
        ZBPRO_ZCL_CLUSTER_BASIC_ATTR_DECLARATIONS \
        ZBPRO_ZCL_CLUSTER_IDENTIFY_ATTR_DECLARATIONS \
        ZBPRO_ZCL_CLUSTER_DIAGNOSTIC_ATTR_DECLARATIONS
#else
#define ZBPRO_ZCL_ATTR_DECLARATIONS \
        ZBPRO_ZCL_CLUSTER_BASIC_ATTR_DECLARATIONS \
        ZBPRO_ZCL_CLUSTER_IDENTIFY_ATTR_DECLARATIONS
#endif
/************************* TYPES *******************************************************/

/**//**
 * \brief Definition to declare ZCL attributes
 */
#define ZBPRO_ZCL_ATTR_DECLARE(clusterId, attrId, ZBPRO_ZCL_AttrDataType, actualType, defaultValue, zbProZclAttrFlagMask, accessFunc) \
        actualType var##attrId;

/**//**
 * \brief Type representing every supported attribute
 */
typedef union _ZbProZclbAttrValue_t
{
    ZBPRO_ZCL_ATTR_DECLARATIONS                /*!< All supported attributes */
} ZbProZclAttrValue_t;
#undef ZBPRO_ZCL_ATTR_DECLARE

/**//**
 * \brief HA Attributes access notification indication parameters
 * \ingroup ZBPRO_ZCL_AttrAccessInd
 */
typedef struct _ZBPRO_ZCL_AttrAccessIndParams_t
{
    ZBPRO_APS_EndpointId_t          endpoint;  /*!< Endpoint ID */
    Bool8_t                         isWrite;   /*!< Could be written? */
    ZBPRO_ZCL_AttributeId_t         attrId;    /*!< Attribute ID */
    ZbProZclAttrValue_t             value;     /*!< Attribute value */
} ZBPRO_ZCL_AttrAccessIndParams_t;

/************************* PROTOTYPES ***************************************************/

/**//**
 * \brief ZCL HA Attributes access notification
 * \ingroup ZBPRO_ZCL_Functions
 *
 * \note Access of the Identify time attribute of the Identify Cluster attributes
 *       is notified via dedicated Identify/Identify Query command indications
 * \param[in] indParams - pointer to the indication parameters.
 * \return Nothing.
 */
void ZBPRO_ZCL_AttrAccessInd(ZBPRO_ZCL_AttrAccessIndParams_t *const indParams);

#endif /* _BB_ZBPRO_ZCL_ATTR_LOCAL_H */

/* eof bbZbProZclAttrLocal.h */