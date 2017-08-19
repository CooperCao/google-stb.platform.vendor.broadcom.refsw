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
*       Network Layer Data Entity declarations
*
*******************************************************************************/

#ifndef _ZBPRO_NWK_DATA_H
#define _ZBPRO_NWK_DATA_H

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkCommon.h"

/************************* TYPES *******************************************************/

/**//**
 * \brief NLME-DATA request parameters structure. See ZigBee Spec r20, 3.2.1.1.
 * \ingroup ZBPRO_NWK_DataReq
 */
typedef struct _ZBPRO_NWK_DataReqParams_t
{
    ZBPRO_NWK_AddrMode_t    dstAddrMode;         /*!< NWK Destination address mode */
    ZBPRO_NWK_NwkAddr_t     dstAddr;             /*!< NWK Destination address */
    SYS_DataPointer_t       payload;             /*!< Request payload */
    uint8_t                 nsduHandle;          /*!< NSDU handle */
    uint8_t                 radius;              /*!< Request radius */
    uint8_t                 nonmemberRadius;     /*!< Nonmember radius */
    Bool8_t                 discoverRoute;       /*!< Is discover route? */
    Bool8_t                 securityEnable;      /*!< Is security enabled? */
    Bool8_t                 forceRouteDiscovery; /*!< \note NWK will force the route discovery
                                                      procedure to update current route */
} ZBPRO_NWK_DataReqParams_t;


/**//**
 * \brief NLME-DATA confirm parameters structure. See ZigBee Spec r20, 3.2.1.2.
 * \ingroup ZBPRO_NWK_DataConf
 */
typedef struct _ZBPRO_NWK_DataConfParams_t
{
    ZBPRO_NWK_Status_t      status;              /*!< Confirmation status */
    uint8_t                 nsduHandle;          /*!< NSDU handle */
} ZBPRO_NWK_DataConfParams_t;

/**//**
 * \brief NLME-DATA.request descriptor data type declaration.
 * \ingroup ZBPRO_NWK_DataReq
 */
typedef struct _ZBPRO_NWK_DataReqDescr_t  ZBPRO_NWK_DataReqDescr_t;

/**//**
 * \brief NLME-DATA confirm primitive callback function type
 * \ingroup ZBPRO_NWK_DataConf
 */
typedef void (*ZBPRO_NWK_DataConfCallback_t)(ZBPRO_NWK_DataReqDescr_t *const reqDescr,
                                             ZBPRO_NWK_DataConfParams_t *const conf);

/**//**
 * \brief NLME-DATA.request descriptor data type definition.
 * \ingroup ZBPRO_NWK_DataReq
 */
struct _ZBPRO_NWK_DataReqDescr_t
{
    ZbProNwkServiceField_t          service;     /*!< Request service field */
    ZBPRO_NWK_DataReqParams_t       params;      /*!< Request parameters */
    ZBPRO_NWK_DataConfCallback_t    callback;    /*!< Confirmation callback */
};

/**//**
 * \brief NLDE-DATA indication parameters structure. See ZigBee Spec r20, 3.2.1.3.
 * \ingroup ZBPRO_NWK_DataInd
 */
typedef struct _ZBPRO_NWK_DataIndParams_t
{
    ZBPRO_NWK_AddrMode_t    dstAddrMode;         /*!< NWK Destination address mode */
    ZBPRO_NWK_NwkAddr_t     dstAddr;             /*!< NWK Destination address */
    ZBPRO_NWK_NwkAddr_t     srcAddr;             /*!< NWK Source address */
    SYS_DataPointer_t       payload;             /*!< Indication payload */
    PHY_LQI_t               linkQuality;         /*!< Link quality */
    bool                    secured;             /*!< Is secured? */
} ZBPRO_NWK_DataIndParams_t;

/************************* PROTOTYPES **************************************************/

/************************************************************************************//**
    \brief NLME-DATA request primitive function.
    \ingroup ZBPRO_NWK_Functions

    \param[in] req - pointer to the request structure.
    \return Nothing.
****************************************************************************************/
NWK_PUBLIC void ZBPRO_NWK_DataReq(ZBPRO_NWK_DataReqDescr_t *const req);

/************************************************************************************//**
    \brief NLME-DATA indication primitive function.
    \ingroup ZBPRO_NWK_Functions

    \param[in] ind - pointer to the indication parameters.
    \return Nothing.
****************************************************************************************/
extern void ZBPRO_NWK_DataInd(ZBPRO_NWK_DataIndParams_t *const ind);

#endif /* _ZBPRO_NWK_DATA_H */

/* eof bbZbProNwkSapTypesData.h */