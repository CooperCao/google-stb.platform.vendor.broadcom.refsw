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
* FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/bbZbProApsSapBindUnbind.h $
*
* DESCRIPTION:
*   APSME-(UN)BIND service interface.
*
* $Revision: 2437 $
* $Date: 2014-05-19 13:56:29Z $
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_APS_SAP_BIND_UNBIND_H
#define _BB_ZBPRO_APS_SAP_BIND_UNBIND_H


/************************* INCLUDES *****************************************************/
#include "bbZbProApsKeywords.h"     /* ZigBee PRO APS macro keywords definition. */
#include "bbZbProApsCommon.h"       /* ZigBee PRO APS general types definitions. */

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief APSME-(UN)BIND.request parameters data structure.
 */
typedef struct _ZBPRO_APS_BindUnbindReqParams_t
{
    ZBPRO_APS_ExtAddr_t     srcAddress;
    /* Structured / 96-bit data. */
    ZBPRO_APS_Address_t     dstAddr;       /*!< The destination address mode and address
                                                for the binding entry. */
    /* 16-bit data. */
    ZBPRO_APS_ClusterId_t   clusterId;     /*!< The identifier of the cluster on the source device
                                                that is to be (un)bound to/from the destination device. */
    /* 8-bit data. */
    ZBPRO_APS_EndpointId_t  srcEndpoint;   /*!< The source endpoint for the binding entry. */

    ZBPRO_APS_EndpointId_t  dstEndpoint;   /*!< The destination endpoint for the binding entry. */

} ZBPRO_APS_BindUnbindReqParams_t;


/**//**
 * \brief APSME-(UN)BIND.confirm parameters data structure.
 */
typedef struct _ZBPRO_APS_BindUnbindConfParams_t
{
    ZBPRO_APS_Status_t       status;        /*!< The results of the (un)binding request. */
} ZBPRO_APS_BindUnbindConfParams_t;


/**//**
 * \brief APSME-(UN)BIND.request descriptor data type declaration.
 */
typedef struct _ZBPRO_APS_BindUnbindReqDescr_t  ZBPRO_APS_BindUnbindReqDescr_t;


/**//**
 * \brief APSME-(UN)BIND.confirm callback function data type.
 * \details Call this function to issue APSME-(UN)BIND.confirm to the higher layer.
 * \param reqDescr Pointer to the confirmed request descriptor data structure.
 * \param confParams Pointer to the confirmation parameters data structure. Treat this
 *  data structure in the confirmation handler-function as it has been allocated in the
 *  program stack by APS before calling this callback-handler and will be destroyed just
 *  after this callback returns.
 */
typedef void ZBPRO_APS_BindUnbindConfCallback_t(ZBPRO_APS_BindUnbindReqDescr_t   *const reqDescr,
                                                ZBPRO_APS_BindUnbindConfParams_t *const confParams);


/**//**
 * \brief APSME-(UN)BIND.request descriptor data type.
 */
struct _ZBPRO_APS_BindUnbindReqDescr_t
{
    /* 32-bit data. */
    ZBPRO_APS_BindUnbindConfCallback_t *callback;           /*!< Confirm callback function. */

    /* Structured / 32-bit data. */
    struct
    {
        SYS_QueueElement_t              queueElement;       /*!< APS requests service field. */

        Bool8_t                         isBindRequest;      /*!< TRUE for the case of APSME-BIND.request; FALSE for the
                                                                case of APSME-UNBIND.request. */

    } service;                                              /*!< Service field. */

    /* Structured data. */
    ZBPRO_APS_BindUnbindReqParams_t     params;             /*!< Request parameters set. */

};


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
  \brief
    Accepts APSME-BIND.request from ZDO Binding Manager to ZigBee Pro APS and starts its
    processing.
  \param    reqDescr
    Pointer to the request descriptor data structure.
  \note
    Data structure pointed by \p reqDescr must reside in global memory space and must be
    preserved by the caller until confirmation from APS. The \c service field of request
    descriptor is used by APS during request processing. The caller shall set the
    \c callback field to the entry point of its APSME-BIND.confirm handler-function.
  \note
    It is allowed to commence new request to APS directly from the context of the
    confirmation handler. The same request descriptor data object may be used for the new
    request as that one returned with confirmation parameters.
*****************************************************************************************/
APS_PUBLIC void ZBPRO_APS_BindReq(ZBPRO_APS_BindUnbindReqDescr_t *reqDescr);


/*************************************************************************************//**
  \brief
    Accepts APSME-UNBIND.request from ZDO Binding Manager to ZigBee Pro APS and starts its
    processing.
  \param    reqDescr
    Pointer to the request descriptor data structure.
  \note
    Data structure pointed by \p reqDescr must reside in global memory space and must be
    preserved by the caller until confirmation from APS. The \c service field of request
    descriptor is used by APS during request processing. The caller shall set the
    \c callback field to the entry point of its APSME-UNBIND.confirm handler-function.
  \note
    It is allowed to commence new request to APS directly from the context of the
    confirmation handler. The same request descriptor data object may be used for the new
    request as that one returned with confirmation parameters.
*****************************************************************************************/
APS_PUBLIC void ZBPRO_APS_UnbindReq(ZBPRO_APS_BindUnbindReqDescr_t *reqDescr);


#endif /* _BB_ZBPRO_APS_SAP_BIND_UNBIND_H */