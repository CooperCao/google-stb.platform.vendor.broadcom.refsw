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
 *      Declarations of the Service Access point of the APS Endpoint component.
 *
*******************************************************************************/

#ifndef _ZBPRO_APS_SAP_ENDPOINT_H
#define _ZBPRO_APS_SAP_ENDPOINT_H

/************************* INCLUDES ****************************************************/
#include "bbZbProApsCommon.h"
#include "bbZbProApsKeywords.h"
#include "bbZbProApsData.h"

/**************************** TYPES ****************************************************/

/**//**
 * \brief Simple descriptor type. For more information please see spec R20 chapter 2.3.2.5 p.88
 */
typedef struct _ZBPRO_APS_SimpleDescriptor_t
{
    ZBPRO_APS_DeviceId_t        deviceId;                 /*!< Device description supported on this endpoint */
    ZBPRO_APS_ProfileId_t       profileId;                /*!< Profile that is supported on this endpoint */
    ZBPRO_APS_EndpointId_t      endpoint;                 /*!< Endpoint within the node */
    ZBPRO_APS_DeviceVersion_t   deviceVersion;            /*!< Device version */
    ZBPRO_APS_ClusterAmount_t   inClusterAmount;          /*!< Number of input clusters supported on this endpoint */
    ZBPRO_APS_ClusterAmount_t   outClusterAmount;         /*!< Number of output clusters, supported on this endpoint */
    SYS_DataPointer_t           inOutClusterList;         /*!< List includes input Clusters Ids from the beginning
                                                               and output Clusters Ids after that. */
} ZBPRO_APS_SimpleDescriptor_t;

/**//**
 * \brief APS Endpoint_Register request parameters
 * \ingroup ZBPRO_APS_EndpointRegisterReq
 * \details The apsMaxWindowSize specifies whether this endpoint allows fragmented APS Data frames or not, and if it
 *  allows then this value specifies the fragmentation window size, in fragments. Value 0 stays for 'fragmentation is
 *  disabled'; values from 1 to 8 stay for 'fragmentation is enabled' and specify the window size.
 */
typedef struct _zbProApsEndpointRegisterReqParams_t
{
    ZBPRO_APS_SimpleDescriptor_t        simpleDescriptor;      /*!< Simple descriptor (endpoint) to register */
    Bool8_t                             useInternalHandler;    /*!< Should use internal handler? */
    ZBPRO_APS_EndpointMaxWindowSize_t   apsMaxWindowSize;      /*!< Value of the apsMaxWindowSize attribute, 0..8. */
    ZBPRO_APS_DataInd_t                 *dataInd;              /*!< Data indication pointer */
} zbProApsEndpointRegisterReqParams_t;

SYS_DbgAssertStatic(0 == offsetof(zbProApsEndpointRegisterReqParams_t, simpleDescriptor));

/**//**
 * \brief APS Endpoint_Register request parameters for host side.
 * \ingroup ZBPRO_APS_EndpointRegisterReq
 * \details The apsMaxWindowSize specifies whether this endpoint allows fragmented APS Data frames or not, and if it
 *  allows then this value specifies the fragmentation window size, in fragments. Value 0 stays for 'fragmentation is
 *  disabled'; values from 1 to 8 stay for 'fragmentation is enabled' and specify the window size.
 */
typedef struct _ZBPRO_APS_EndpointRegisterReqParams_t
{
    ZBPRO_APS_SimpleDescriptor_t        simpleDescriptor;      /*!< Simple descriptor (endpoint) to register */
    Bool8_t                             useInternalHandler;    /*!< Should use internal handler? */
    ZBPRO_APS_EndpointMaxWindowSize_t   apsMaxWindowSize;      /*!< Value of the apsMaxWindowSize attribute, 0..8. */
} ZBPRO_APS_EndpointRegisterReqParams_t;

SYS_DbgAssertStatic(offsetof(ZBPRO_APS_EndpointRegisterReqParams_t, simpleDescriptor)
                 == offsetof(zbProApsEndpointRegisterReqParams_t,   simpleDescriptor));
SYS_DbgAssertStatic(offsetof(ZBPRO_APS_EndpointRegisterReqParams_t, useInternalHandler)
                 == offsetof(zbProApsEndpointRegisterReqParams_t,   useInternalHandler));
SYS_DbgAssertStatic(offsetof(ZBPRO_APS_EndpointRegisterReqParams_t, apsMaxWindowSize)
                 == offsetof(zbProApsEndpointRegisterReqParams_t,   apsMaxWindowSize));

/**//**
 * \brief APS Endpoint_Register request confirmation parameters
 * \ingroup ZBPRO_APS_EndpointRegisterConf
 */
typedef struct _ZBPRO_APS_EndpointRegisterConfParams_t
{
    ZBPRO_APS_Status_t  status;                           /*!< Confirmation status */
} ZBPRO_APS_EndpointRegisterConfParams_t;

/**//**
 * \brief APS Endpoint_Register request descriptor prototype
 * \ingroup ZBPRO_APS_EndpointRegisterReq
 */
typedef struct _ZBPRO_APS_EndpointRegisterReqDescr_t ZBPRO_APS_EndpointRegisterReqDescr_t;

/**//**
 * \brief APS Endpoint_Register request confirmation callback function type
 * \ingroup ZBPRO_APS_EndpointRegisterConf
 */
typedef void ZBPRO_APS_EndpointRegisterConfCallback_t(ZBPRO_APS_EndpointRegisterReqDescr_t *const reqDescr,
                                                        ZBPRO_APS_EndpointRegisterConfParams_t *const confParams);

/**//**
 * \brief APS Endpoint_Register request descriptor
 * \ingroup ZBPRO_APS_EndpointRegisterReq
 */
struct _ZBPRO_APS_EndpointRegisterReqDescr_t
{
    struct
    {
        SYS_QueueElement_t  qElem;                        /*!< Service queue field */
    } service;                                            /*!< Service container */

    ZBPRO_APS_EndpointRegisterConfCallback_t *callback;   /*!< Confirmation callback */
#ifndef MAILBOX_HOST_SIDE
    zbProApsEndpointRegisterReqParams_t      params;      /*!< Request parameters */
#else
    ZBPRO_APS_EndpointRegisterReqParams_t    params;      /*!< Request parameters */
#endif
};

/**//**
 * \brief APS Endpoint_Unregister request parameters
 * \ingroup ZBPRO_APS_EndpointUnregisterReq
 */
typedef struct _ZBPRO_APS_EndpointUnregisterReqParams_t
{
    ZBPRO_APS_EndpointId_t      endpoint;                 /*!< Endpoint ID */
} ZBPRO_APS_EndpointUnregisterReqParams_t;

/**//**
 * \brief APS Endpoint_Unregister request descriptor prototype
 * \ingroup ZBPRO_APS_EndpointUnregisterReq
 */
typedef struct _ZBPRO_APS_EndpointUnregisterReqDescr_t ZBPRO_APS_EndpointUnregisterReqDescr_t;

/**//**
 * \brief APS Endpoint_Unregister request confirmation callback function type
 * \ingroup ZBPRO_APS_EndpointUnregisterConf
 */
typedef void ZBPRO_APS_EndpointUnregisterConfCallback_t(ZBPRO_APS_EndpointUnregisterReqDescr_t *const reqDescr,
                                                        ZBPRO_APS_EndpointRegisterConfParams_t *const confParams);

/**//**
 * \brief APS Endpoint_Unregister request descriptor
 * \ingroup ZBPRO_APS_EndpointUnregisterReq
 */
struct _ZBPRO_APS_EndpointUnregisterReqDescr_t
{
    struct
    {
        SYS_QueueElement_t  qElem;                        /*!< Service queue field */
    } service;                                            /*!< Service container */

    ZBPRO_APS_EndpointUnregisterConfCallback_t *callback; /*!< Confirmation callback */
    ZBPRO_APS_EndpointUnregisterReqParams_t    params;    /*!< Request parameters */
};

/************************* PROTOTYPES ***************************************************/

/*************************************************************************************//**
  \brief Internal APS Endpoint_Register request function
  \param[in] reqDescr - pointer to the request structure
  \return Nothing.
*****************************************************************************************/
void zbProApsEndpointRegisterReq(ZBPRO_APS_EndpointRegisterReqDescr_t *const reqDescr);

/*************************************************************************************//**
  \brief APS Public function, register endpoint via binding ZBPRO_APS_DataInd function with
         a given simple descriptor (endpoint).
  \ingroup ZBPRO_APS_Functions

  \param[in] reqDescr - pointer to the request structure
  \return Nothing.
*****************************************************************************************/
void ZBPRO_APS_EndpointRegisterReq(ZBPRO_APS_EndpointRegisterReqDescr_t *const reqDescr);

/*************************************************************************************//**
  \brief APS Public function, unregister the specified endpoint
  \ingroup ZBPRO_APS_Functions

  \param[in] reqDescr - pointer to the request structure
  \return Nothing.
*****************************************************************************************/
void ZBPRO_APS_EndpointUnregisterReq(ZBPRO_APS_EndpointUnregisterReqDescr_t *const reqDescr);

/*************************************************************************************//**
  \brief This function shall be declared from host side.
*****************************************************************************************/
ZBPRO_APS_DataInd_t ZBPRO_APS_DataInd;

/**//**
 * \brief Prototype of the function which accepts APSDE-DATA.indication if stack ZCL/HA support is enabled.
 */
#if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_)
ZBPRO_APS_DataInd_t ZBPRO_ZCL_DataInd;
#endif

#endif /* _ZBPRO_APS_SAP_ENDPOINT_H */

/* eof bbZbProApsSapEndpoint.h */