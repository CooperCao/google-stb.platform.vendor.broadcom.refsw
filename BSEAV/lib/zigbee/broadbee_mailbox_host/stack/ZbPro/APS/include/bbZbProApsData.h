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
 * FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/bbZbProApsData.h $
 *
 * DESCRIPTION:
 *   This header describes API for the ZigBee PRO APS Data Service component.
 *
 * $Revision: 2491 $
 * $Date: 2014-05-22 11:01:34Z $
 *
 ***************************************************************************************/
#ifndef _ZBPRO_APS_DATA_SERVICE_H
#define _ZBPRO_APS_DATA_SERVICE_H

/************************* INCLUDES ****************************************************/
#include "bbSysPayload.h"
#include "bbZbProNwkCommon.h"
#include "bbZbProApsCommon.h"


/************************* TYPES *******************************************************/

/**//**
 * \brief ZigBeePRO APS Data request type declaration.
 */
typedef struct _ZBPRO_APS_DataReqDescr_t ZBPRO_APS_DataReqDescr_t;

/**//**
 * \brief Type is used to store transmission options.
 */
typedef struct _ZBPRO_APS_TxOptions_t
{
    BitField8_t security                : 1;
    BitField8_t nwkKey                  : 1;
    BitField8_t ack                     : 1;
    BitField8_t fragmentationPermitted  : 1;    /* not supported */
    BitField8_t extNonce                : 1;
} ZBPRO_APS_TxOptions_t;

/**//**
 * \brief ZigBeePRO APS Data request parameters. Refer to ZigBee Spec r20 table 2.2.
 */
typedef struct _ZBPRO_APS_DataReqParams_t
{
    ZBPRO_APS_Address_t         dstAddress;
    ZBPRO_APS_ProfileId_t       profileId;
    ZBPRO_APS_ClusterId_t       clusterId;
    ZBPRO_APS_EndpointId_t      dstEndpoint;
    ZBPRO_APS_EndpointId_t      srcEndpoint;
    ZBPRO_APS_TxOptions_t       txOptions;
    ZBPRO_APS_NwkRadius_t       radius;
    SYS_DataPointer_t           payload;
} ZBPRO_APS_DataReqParams_t;

/**//**
 * \brief ZigBeePRO APS APSDE-DATA.confirm parameters type. Refer to ZigBee Spec r20 table 2.3.
 */
typedef struct _ZBPRO_APS_DataConfParams_t
{
    ZBPRO_APS_Address_t     dstAddress;
    ZBPRO_APS_Timestamp_t   txTime;
    ZBPRO_APS_EndpointId_t  dstEndpoint;
    ZBPRO_APS_EndpointId_t  srcEndpoint;
    ZBPRO_APS_Status_t      status;
    Bool8_t                 isNhlUnicast;   /*<! false, if there was more than one recipient */
} ZBPRO_APS_DataConfParams_t;

/**//**
 * \brief ZigBeePRO APS APSDE-DATA.confirm primitive's type.
 */
typedef void ZBPRO_APS_DataConfCallback_t(ZBPRO_APS_DataReqDescr_t *const reqDescr,
        ZBPRO_APS_DataConfParams_t *const confParams);

/**//**
 * \brief ZigBeePRO APS Data request type
 */
struct _ZBPRO_APS_DataReqDescr_t
{
    struct
    {
        SYS_QueueElement_t          next;
        ZBPRO_APS_Timestamp_t       txTime;
        ZbProApsPeer_t              peer;
        uint8_t                     peerCnt;
        ZBPRO_APS_Status_t          overallStatus;
        Bool8_t                     isNhlUnicast;
    } service;

    ZBPRO_APS_DataReqParams_t       params;

    ZBPRO_APS_DataConfCallback_t    *callback;
};

/**//**
 * \brief ZigBeePRO APS APSDE-DATA.indication primitive's parameters type.
 */
typedef struct _ZBPRO_APS_DataIndParams_t
{
    ZBPRO_APS_Address_t     dstAddress;
    ZBPRO_APS_Address_t     srcAddress;
    ZBPRO_APS_ProfileId_t   profileId;
    ZBPRO_APS_ClusterId_t   clusterId;
    ZBPRO_APS_EndpointId_t  dstEndpoint;
    ZBPRO_APS_EndpointId_t  srcEndpoint;
    SYS_DataPointer_t       payload;
    ZBPRO_APS_Timestamp_t   rxTime;
    ZBPRO_APS_Status_t      status;
    ZBPRO_APS_Status_t      securityStatus;
    ZBPRO_NWK_Lqi_t         lqi;
    ZBPRO_APS_EndpointId_t  localEndpoint;
} ZBPRO_APS_DataIndParams_t;

/************************************************************************************//**
 \brief Type definition of an NHL function which is notified of the arrival of
    the APSDE-DATA indication primitive. Refer to ZigBee Spec r20 paragraph 2.2.4.1.3.
 \param[in] ind - pointer to the indication parameters.
 ***************************************************************************************/
typedef void ZBPRO_APS_DataInd_t(ZBPRO_APS_DataIndParams_t *const indParams);

/************************* FUNCTIONS PROTOTYPES ****************************************/

/************************************************************************************//**
 \brief Function implements APSDE-DATA request primitive. Refer to ZigBee Spec r20
        paragraph 2.2.4.1.1.
 \param[in] req - pointer to the request parameters.
 ***************************************************************************************/
void ZBPRO_APS_DataReq(ZBPRO_APS_DataReqDescr_t *const reqDescr);

/************************************************************************************//**
 \brief Function implements APSDE-DATA indication primitive on ZDO endpoint(0x00).
  ***************************************************************************************/
ZBPRO_APS_DataInd_t ZBPRO_ZDO_DataInd;

#endif /* _ZBPRO_APS_DATA_SERVICE_H */