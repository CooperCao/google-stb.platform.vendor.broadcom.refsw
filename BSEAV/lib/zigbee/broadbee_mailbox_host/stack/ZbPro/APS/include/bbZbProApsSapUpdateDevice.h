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
* FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/bbZbProApsSapUpdateDevice.h $
*
* DESCRIPTION:
*   APSME-UPDATE-DEVICE security service interface.
*
* $Revision: 2223 $
* $Date: 2014-04-18 12:02:47Z $
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_APS_SAP_UPDATE_DEVICE_H
#define _BB_ZBPRO_APS_SAP_UPDATE_DEVICE_H


/************************* INCLUDES *****************************************************/
#include "bbZbProApsSapSecurityTypes.h"     /* APSME Security Services data types definitions. */


/************************* DEFINITIONS **************************************************/

/**//**
 * \brief Updated device statuses enumeration.
 */
typedef enum _ZBPRO_APS_UpdateDeviceStatus_t
{
    ZBPRO_APS_STANDARD_DEVICE_SECURED_REJOIN           = 0x00,      /*!< Standard device secured rejoin. */

    ZBPRO_APS_STANDARD_DEVICE_UNSECURED_JOIN           = 0x01,      /*!< Standard device unsecured join. */

    ZBPRO_APS_DEVICE_LEFT                              = 0x02,      /*!< Device left. */

    ZBPRO_APS_STANDARD_DEVICE_TRUST_CENTER_REJOIN      = 0x03,      /*!< Standard device trust center rejoin. */

    ZBPRO_APS_HIGH_SECURITY_DEVICE_SECURED_REJOIN      = 0x04,      /*!< High security device secured rejoin. */

    ZBPRO_APS_HIGH_SECURITY_DEVICE_TRUST_CENTER_JOIN   = 0x05,      /*!< High security device trust center join. */

    ZBPRO_APS_HIGH_SECURITY_DEVICE_UNSECURED_JOIN      = 0x05,      /*!< High security device unsecured join. */

    /*                                                 = 0x06,         < Reserved. */

    ZBPRO_APS_HIGH_SECURITY_DEVICE_TRUST_CENTER_REJOIN = 0x07,      /*!< High security device trust center rejoin. */

    ZBPRO_APS_UPDATEDEVICE_STATUS_MAX                               /*!< first illegal value */
} ZBPRO_APS_UpdateDeviceStatus_t;

/**//**
 * \brief APSME-UPDATE-DEVICE.request parameters data structure.
 */
typedef struct _ZBPRO_APS_UpdateDeviceReqParams_t
{
    /* 64-bit data. */
    ZBPRO_APS_ExtAddr_t             destAddress;            /*!< The extended 64-bit address of the device
                                                                that shall be sent the update information. */

    ZBPRO_APS_ExtAddr_t             deviceAddress;          /*!< The extended 64-bit address of the device
                                                                whose status is being updated. */
    /* 16-bit data. */
    ZBPRO_APS_ShortAddr_t           deviceShortAddress;     /*!< The 16-bit network address of the device
                                                                whose status is being updated. */
    /* 8-bit data. */
    ZBPRO_APS_UpdateDeviceStatus_t  status;                 /*!< Indicates the updated status of the device
                                                                given by the DeviceAddress parameter. */
} ZBPRO_APS_UpdateDeviceReqParams_t;


/**//**
 * \brief APSME-UPDATE-DEVICE.request descriptor data type declaration.
 */
typedef struct _ZBPRO_APS_UpdateDeviceReqDescr_t  ZBPRO_APS_UpdateDeviceReqDescr_t;


/**//**
 * \brief APSME-UPDATE-DEVICE.confirm callback function data type.
 * \details Call this function to issue APSME-UPDATE-DEVICE.confirm to the higher layer.
 * \param reqDescr Pointer to the confirmed request descriptor data structure.
 * \param confParams Pointer to the confirmation parameters data structure. Treat this
 *  data structure in the confirmation handler-function as it has been allocated in the
 *  program stack by APS before calling this callback-handler and will be destroyed just
 *  after this callback returns.
 * \note According to the Standard there is no confirmation assumed on the primitive
 *  APSME-UPDATE-DEVICE.request. Nevertheless, this function is called with confirmation
 *  status parameter that shows at least successful or unsuccessful result from NWK layer.
 */
typedef void ZBPRO_APS_UpdateDeviceConfCallback_t(ZBPRO_APS_UpdateDeviceReqDescr_t       *const reqDescr,
        ZBPRO_APS_SecurityServicesConfParams_t *const confParams);


/**//**
 * \brief APSME-UPDATE-DEVICE.request descriptor data type.
 */
typedef struct _ZBPRO_APS_UpdateDeviceReqDescr_t
{
    /* Fields are arranged to minimize paddings */
    ZBPRO_APS_UpdateDeviceConfCallback_t *callback;         /*!< Confirm callback function.  */

    struct
    {
        SYS_QueueElement_t                queueElement;     /*!< APS requests service field. */
    } service;

    ZBPRO_APS_UpdateDeviceReqParams_t     params;           /*!< Request parameters set.     */
} ZBPRO_APS_UpdateDeviceReqDescr_t;


/**//**
 * \brief APSME-UPDATE-DEVICE.indication parameters data structure.
 */
typedef struct _ZBPRO_APS_UpdateDeviceIndParams_t
{
    /* 64-bit data. */
    ZBPRO_APS_ExtAddr_t             srcAddress;             /*!< The extended 64-bit address of the device
                                                                originating the update-device command. */

    ZBPRO_APS_ExtAddr_t             deviceAddress;          /*!< The extended 64-bit address of the device
                                                                whose status is being updated. */
    /* 16-bit data. */
    ZBPRO_APS_ShortAddr_t           deviceShortAddress;     /*!< The 16-bit network address of the device
                                                                whose status is being updated. */
    /* 8-bit data. */
    ZBPRO_APS_UpdateDeviceStatus_t  status;                 /*!< Indicates the updated status of the device
                                                                given by the DeviceAddress parameter. */
} ZBPRO_APS_UpdateDeviceIndParams_t;


/**//**
 * \brief APSME-UPDATE-DEVICE.indication callback function data type.
 * \details Call this function to issue APSME-UPDATE-DEVICE.indication to the higher
 *  layer.
 * \param indParams Pointer to the indication parameters data structure. Treat this data
 *  structure in the indication handler-function as it has been allocated in the program
 *  stack by APS before calling this callback-handler and will be destroyed just after
 *  this callback returns.
 */
typedef void ZBPRO_APS_UpdateDeviceInd_t(ZBPRO_APS_UpdateDeviceIndParams_t *const indParams);


/************************* PROTOTYPES ***************************************************/

/*************************************************************************************//**
  \brief
    Accepts APSME-UPDATE-DEVICE.request from ZDO Security Manager to ZigBee Pro APS
    and starts its processing.
  \param    reqDescr
    Pointer to the request descriptor data structure.
  \note
    Data structure pointed by \p reqDescr must reside in global memory space and must be
    preserved by the caller until confirmation from APS. The \c service field of request
    descriptor is used by APS during request processing. The caller shall set the
    \c callback field to the entry point of its APSME-UPDATE-DEVICE.confirm
    handler-function.
  \note
    It is allowed to commence new request to APS directly from the context of the
    confirmation handler. The same request descriptor data object may be used for the new
    request as that one returned with confirmation parameters.
*****************************************************************************************/
APS_PUBLIC void ZBPRO_APS_UpdateDeviceReq(ZBPRO_APS_UpdateDeviceReqDescr_t *reqDescr);


/*************************************************************************************//**
  \brief
    Issues APSME-UPDATE-DEVICE.indication to ZigBee PRO.
  \param    indParams
    Pointer to the indication parameters data structure.
  \note
    ZDO Security Manager shall provide APSME-UPDATE-DEVICE.indication handler-function
    according to the template:
  \code
    void ZBPRO_APS_UpdateDeviceInd(ZBPRO_APS_UpdateDeviceIndParams_t *indParams) { ... }
  \endcode
  \note
    Indication handler-function shall treat the data structure pointed with \p indParams
    as it has been allocated in the program stack by APS before calling this
    callback-handler and will be destroyed by APS just after this callback returns.
*****************************************************************************************/
APS_PUBLIC ZBPRO_APS_UpdateDeviceInd_t ZBPRO_APS_UpdateDeviceInd;


#endif /* _BB_ZBPRO_APS_SAP_UPDATE_DEVICE_H */