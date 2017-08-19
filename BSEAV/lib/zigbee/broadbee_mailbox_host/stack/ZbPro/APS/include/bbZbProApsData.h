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
 *      This header describes API for the ZigBee PRO APS Data Service component.
 *
*******************************************************************************/

#ifndef _ZBPRO_APS_DATA_SERVICE_H
#define _ZBPRO_APS_DATA_SERVICE_H

/************************* INCLUDES ****************************************************/
#include "bbSysPayload.h"
#include "bbZbProNwkCommon.h"
#include "bbZbProApsCommon.h"


/************************* TYPES *******************************************************/

/**//**
 * \brief ZigBeePRO APS Data request type declaration.
 * \ingroup ZBPRO_APS_DataReq
 */
typedef struct _ZBPRO_APS_DataReqDescr_t ZBPRO_APS_DataReqDescr_t;

/**//**
 * \brief Type is used to store transmission options.
 */
typedef struct _ZBPRO_APS_TxOptions_t
{
    BitField8_t security                : 1;       /*!< Security enabled transmission */
    BitField8_t nwkKey                  : 1;       /*!< Use NWK key */
    BitField8_t ack                     : 1;       /*!< Acknowledged transmission */
    BitField8_t fragmentationPermitted  : 1;       /*!< Fragmentation permitted */
    BitField8_t extNonce                : 1;       /*!< Include extended nonce in APS security frame */
    BitField8_t updateExistingRoute     : 1;       /*!< \note APS force NWK to update existing route if it is possible */
} ZBPRO_APS_TxOptions_t;

/**//**
 * \brief ZigBeePRO APS Data request parameters. Refer to ZigBee Spec r20 table 2.2.
 * \ingroup ZBPRO_APS_DataReq
 */
typedef struct _ZBPRO_APS_DataReqParams_t
{
    ZBPRO_APS_Address_t         dstAddress;        /*!< The individual device or group "to" address */
    ZBPRO_APS_ProfileId_t       profileId;         /*!< Frame profile ID */
    ZBPRO_APS_ClusterId_t       clusterId;         /*!< Frame object ID */
    ZBPRO_APS_EndpointId_t      dstEndpoint;       /*!< Either individual or broadcast (0xff) "to" endpoint */
    ZBPRO_APS_EndpointId_t      srcEndpoint;       /*!< The individual "from" endpoint */
    ZBPRO_APS_TxOptions_t       txOptions;         /*!< The transmission options */
    ZBPRO_APS_NwkRadius_t       radius;            /*!< The maximum distance, in hops */
    SYS_DataPointer_t           payload;           /*!< Request data pointer */
} ZBPRO_APS_DataReqParams_t;

/**//**
 * \brief ZigBeePRO APS APSDE-DATA.confirm parameters type. Refer to ZigBee Spec r20 table 2.3.
 * \ingroup ZBPRO_APS_DataConf
 */
typedef struct _ZBPRO_APS_DataConfParams_t
{
    ZBPRO_APS_Address_t     dstAddress;            /*!< The individual device or group "to" address */
    ZBPRO_APS_Timestamp_t   txTime;                /*!< A packet time indication based on the local clock
                                                        \note Implementation specific */
    ZBPRO_APS_EndpointId_t  dstEndpoint;           /*!< The individual "to" endpoint */
    ZBPRO_APS_EndpointId_t  srcEndpoint;           /*!< The individual "from" endpoint */
    ZBPRO_APS_Status_t      status;                /*!< The status of the corresponding request */
    Bool8_t                 isNhlUnicast;          /*!< False, if there was more than one recipient */
} ZBPRO_APS_DataConfParams_t;

/**//**
 * \brief ZigBeePRO APS APSDE-DATA.confirm primitive's type.
 * \ingroup ZBPRO_APS_DataConf
 */
typedef void ZBPRO_APS_DataConfCallback_t(ZBPRO_APS_DataReqDescr_t *const reqDescr,
        ZBPRO_APS_DataConfParams_t *const confParams);

/**//**
 * \brief ZigBeePRO APS Data request type
 * \ingroup ZBPRO_APS_DataReq
 */
struct _ZBPRO_APS_DataReqDescr_t
{
    struct
    {
        SYS_QueueElement_t          next;          /*!< Service queue field */
        ZBPRO_APS_Timestamp_t       txTime;        /*!< Transmit Time */
        ZbProApsPeer_t              peer;          /*!< Peer description */
        uint8_t                     peerCnt;       /*!< Peers counter */
        ZBPRO_APS_Status_t          overallStatus; /*!< Overall status of the corresponding request */
        Bool8_t                     isNhlUnicast;  /*!< False, if there was more than one recipient */
    } service;                                     /*!< Service data container */
    ZBPRO_APS_DataReqParams_t       params;        /*!< Request parameters */
    ZBPRO_APS_DataConfCallback_t    *callback;     /*!< Confirmation callback */
};

/**//**
 * \brief ZigBeePRO APS APSDE-DATA.indication primitive's parameters type.
 * \ingroup ZBPRO_APS_DataInd
 */
typedef struct _ZBPRO_APS_DataIndParams_t
{
    ZBPRO_APS_Address_t     dstAddress;            /*!< The individual device or group "to" address */
    ZBPRO_APS_Address_t     srcAddress;            /*!< The individual device or group "from" address */
    ZBPRO_APS_ProfileId_t   profileId;             /*!< Frame profile ID */
    ZBPRO_APS_ClusterId_t   clusterId;             /*!< Frame object ID */
    ZBPRO_APS_EndpointId_t  dstEndpoint;           /*!< The individual "to" endpoint */
    ZBPRO_APS_EndpointId_t  srcEndpoint;           /*!< The individual "from" endpoint */
    SYS_DataPointer_t       payload;               /*!< Request data pointer */
    ZBPRO_APS_Timestamp_t   rxTime;                /*!< Received packet timestamp based on local clock
                                                        \note Implementation specific */
    ZBPRO_APS_Status_t      status;                /*!< The status of the corresponding request */
    ZBPRO_APS_Status_t      securityStatus;        /*!< Unsecured/NWK secured/LinkKey Secured */
    PHY_LQI_t               lqi;                   /*!< The link quality indication */
    ZBPRO_APS_EndpointId_t  localEndpoint;         /*!< Local endpoint ID */
} ZBPRO_APS_DataIndParams_t;

/************************************************************************************//**
 \brief Type definition of an NHL function which is notified of the arrival of
    the APSDE-DATA indication primitive. Refer to ZigBee Spec r20 paragraph 2.2.4.1.3.
 \ingroup ZBPRO_APS_Functions

 \param[in] indParams - pointer to the indication parameters.
 \return Nothing.
 ***************************************************************************************/
typedef void ZBPRO_APS_DataInd_t(ZBPRO_APS_DataIndParams_t *const indParams);

/************************* FUNCTIONS PROTOTYPES ****************************************/

/************************************************************************************//**
 \brief Function implements APSDE-DATA request primitive. Refer to ZigBee Spec r20
        paragraph 2.2.4.1.1.
 \ingroup ZBPRO_APS_Functions

 \param[in] reqDescr - pointer to the request parameters.
 \return Nothing.
 ***************************************************************************************/
void ZBPRO_APS_DataReq(ZBPRO_APS_DataReqDescr_t *const reqDescr);

/************************************************************************************//**
 \brief Function implements APSDE-DATA indication primitive on ZDO endpoint(0x00).
  ***************************************************************************************/
ZBPRO_APS_DataInd_t ZBPRO_ZDO_DataInd;

#endif /* _ZBPRO_APS_DATA_SERVICE_H */

/* eof bbZbProApsData.h */