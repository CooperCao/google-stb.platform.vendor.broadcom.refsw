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
 * FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/bbZbProApsSapEndpoint.h $
 *
 * DESCRIPTION:
 *   Declarations of the Service Access point of the APS Endpoint component.
 *
 * $Revision: 3281 $
 * $Date: 2014-08-15 12:08:21Z $
 *
 ****************************************************************************************/

#ifndef _ZBPRO_APS_SAP_ENDPOINT_H
#define _ZBPRO_APS_SAP_ENDPOINT_H

/************************* INCLUDES ****************************************************/
#include "bbZbProApsCommon.h"
#include "bbZbProApsKeywords.h"
#include "bbZbProApsData.h"

/**************************** TYPES ****************************************************/

/*
 * \brief Simple descriptor type. For more information please see spec R20 chapter 2.3.2.5 p.88
 */
typedef struct _ZBPRO_APS_SimpleDescriptor_t
{
    ZBPRO_APS_DeviceId_t        deviceId;
    ZBPRO_APS_ProfileId_t       profileId;
    ZBPRO_APS_EndpointId_t      endpoint;
    ZBPRO_APS_DeviceVersion_t   deviceVersion;
    ZBPRO_APS_ClusterAmount_t   inClusterAmount;
    ZBPRO_APS_ClusterAmount_t   outClusterAmount;
    SYS_DataPointer_t           inOutClusterList;   /* List includes input Clusters Ids from the beginning
                                                     * and output Clusters Ids after that.
                                                     */
} ZBPRO_APS_SimpleDescriptor_t;

/**//**
 * \brief APS Endpoint_Register request parameters
 */
typedef struct _zbProApsEndpointRegisterReqParams_t
{
    ZBPRO_APS_SimpleDescriptor_t simpleDescriptor;
    Bool8_t                      useInternalHandler;
    ZBPRO_APS_DataInd_t         *dataInd;
} zbProApsEndpointRegisterReqParams_t;
SYS_DbgAssertStatic(0 == offsetof(zbProApsEndpointRegisterReqParams_t, simpleDescriptor));

/**//**
 * \brief APS Endpoint_Register request parameters for host side.
 */
typedef struct _ZBPRO_APS_EndpointRegisterReqParams_t
{
    ZBPRO_APS_SimpleDescriptor_t simpleDescriptor;
    Bool8_t                      useInternalHandler;
} ZBPRO_APS_EndpointRegisterReqParams_t;

SYS_DbgAssertStatic(offsetof(ZBPRO_APS_EndpointRegisterReqParams_t, simpleDescriptor)
                 == offsetof(zbProApsEndpointRegisterReqParams_t,   simpleDescriptor));
SYS_DbgAssertStatic(offsetof(ZBPRO_APS_EndpointRegisterReqParams_t, useInternalHandler)
                 == offsetof(zbProApsEndpointRegisterReqParams_t,   useInternalHandler));

/**//**
 * \brief APS Endpoint_Register request confirmation parameters
 */
typedef struct _ZBPRO_APS_EndpointRegisterConfParams_t
{
    ZBPRO_APS_Status_t  status;
} ZBPRO_APS_EndpointRegisterConfParams_t;

/**//**
 * \brief APS Endpoint_Register request descriptor prototype
 */
typedef struct _ZBPRO_APS_EndpointRegisterReqDescr_t ZBPRO_APS_EndpointRegisterReqDescr_t;

/**//**
 * \brief APS Endpoint_Register request confirmation callback function type
 */
typedef void ZBPRO_APS_EndpointRegisterConfCallback_t(ZBPRO_APS_EndpointRegisterReqDescr_t *const reqDescr,
                                                        ZBPRO_APS_EndpointRegisterConfParams_t *const confParams);

/**//**
 * \brief APS Endpoint_Register request descriptor
 */
struct _ZBPRO_APS_EndpointRegisterReqDescr_t
{
    struct
    {
        SYS_QueueElement_t  qElem;
    } service;

    ZBPRO_APS_EndpointRegisterConfCallback_t    *callback;
#ifndef MAILBOX_HOST_SIDE
    zbProApsEndpointRegisterReqParams_t         params;
#else
    ZBPRO_APS_EndpointRegisterReqParams_t       params;
#endif
};

/**//**
 * \brief APS Endpoint_Unregister request parameters
 */
typedef struct _ZBPRO_APS_EndpointUnregisterReqParams_t
{
    ZBPRO_APS_EndpointId_t      endpoint;
} ZBPRO_APS_EndpointUnregisterReqParams_t;

/**//**
 * \brief APS Endpoint_Unregister request descriptor prototype
 */
typedef struct _ZBPRO_APS_EndpointUnregisterReqDescr_t ZBPRO_APS_EndpointUnregisterReqDescr_t;

/**//**
 * \brief APS Endpoint_Unregister request confirmation callback function type
 */
typedef void ZBPRO_APS_EndpointUnregisterConfCallback_t(ZBPRO_APS_EndpointUnregisterReqDescr_t *const reqDescr,
                                                        ZBPRO_APS_EndpointRegisterConfParams_t *const confParams);

/**//**
 * \brief APS Endpoint_Unregister request descriptor
 */
struct _ZBPRO_APS_EndpointUnregisterReqDescr_t
{
    struct
    {
        SYS_QueueElement_t  qElem;
    } service;

    ZBPRO_APS_EndpointUnregisterConfCallback_t    *callback;
    ZBPRO_APS_EndpointUnregisterReqParams_t       params;
};

/************************* PROTOTYPES ***************************************************/

/*************************************************************************************//**
  \brief APS Endpoint_Register request function
  \param[in] reqDescr - pointer to the request structure

  \note Endpoints SHALL be registered in the ascending order of their Endpoint Ids.
*****************************************************************************************/
void zbProApsEndpointRegisterReq(ZBPRO_APS_EndpointRegisterReqDescr_t *const reqDescr);

/*************************************************************************************//**
  \brief APS Public function, bind ZBPRO_APS_DataInd function with a given simple descriptor(endpoint).
  \param[in] reqDescr - pointer to the request structure

  \note Endpoints SHALL be registered in the ascending order of their Endpoint Ids.
*****************************************************************************************/
void ZBPRO_APS_EndpointRegisterReq(ZBPRO_APS_EndpointRegisterReqDescr_t *const reqDescr);

/*************************************************************************************//**
  \brief APS Public function, unregister the specified endpoint
  \param[in] reqDescr - pointer to the request structure
*****************************************************************************************/
void ZBPRO_APS_EndpointUnregisterReq(ZBPRO_APS_EndpointUnregisterReqDescr_t *const reqDescr);

/*************************************************************************************//**
  \brief This is function shall be declared from host side.
*****************************************************************************************/
ZBPRO_APS_DataInd_t ZBPRO_APS_DataInd;

/**//**
 * \brief Prototype of the function which accepts APSDE-DATA.indication if stack ZCL/HA support is enabled.
 */
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_)
ZBPRO_APS_DataInd_t ZBPRO_ZCL_DataInd;
#endif

#endif /* _ZBPRO_APS_SAP_ENDPOINT_H */
