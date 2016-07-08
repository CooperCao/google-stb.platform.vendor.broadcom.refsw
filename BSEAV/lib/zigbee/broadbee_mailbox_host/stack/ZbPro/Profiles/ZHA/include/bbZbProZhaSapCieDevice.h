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
* FILENAME: $Workfile: $
*
* DESCRIPTION:
*   ZHA Profile CIE device interface.
*
* $Revision: $
* $Date: $
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZHA_SAP_CIE_DEVICE_H
#define _BB_ZBPRO_ZHA_SAP_CIE_DEVICE_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZhaCommon.h"
#include "bbZbProZclSapClusterIasAce.h"
#include "bbZbProZclSapClusterIasZone.h"

/**//**
 * \brief Enumeration for the CIE Enroll Request status.
*/
typedef enum _ZBPRO_ZHA_CieEnrollStatus_t
{
    ZBPRO_ZHA_CIE_ENROLL_SUCCESS = 0,               /*!< Successfully done. */
    ZBPRO_ZHA_CIE_ENROLL_NOT_REGISTERED,            /*!< CIE Device was not registered. */
    ZBPRO_ZHA_CIE_ENROLL_CANNOT_RESOLVE_TARGET_ADDR, /*!< Address resolving failed. */
} ZBPRO_ZHA_CieEnrollStatus_t;

/**//**
 * \brief CIE Device Enroll Request parameters.
 * \note The \p addr can be broadcast.
*/
typedef struct _ZBPRO_ZHA_CieEnrollReqParams_t
{
    ZBPRO_APS_Address_t                 addr;
    SYS_Time_t                          scanDurationMs;
    SYS_Time_t                          permitEnrollDurationMs;
    Bool8_t                             autoREsponseMode;
} ZBPRO_ZHA_CieEnrollReqParams_t;

/**//**
 * \brief   Structure for parameters of CIE Device Enroll confirmation.
 * \details Possible values for the \c status parameter are the following:
 *  - SUCCESS                     The Enroll procedure finished successfully.
 *  - NOT_REGISTERED              The CIE Device was not already registered.
 *  - CANNOT_RESOLVE_TARGET_ADDR  Address resolving failed.
 */
typedef struct _ZBPRO_ZHA_CieEnrollConfParams_t
{
    ZBPRO_ZHA_CieEnrollStatus_t status;
} ZBPRO_ZHA_CieEnrollConfParams_t;

/**//**
 * \brief   Structure for descriptor of CIE Device Enroll request
 *  command.
 */
typedef struct _ZBPRO_ZHA_CieEnrollReqDescr_t ZBPRO_ZHA_CieEnrollReqDescr_t;

/**//**
 * \brief   Data type for Local Confirmation callback function of CIE Device Enroll
 *  command.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void (*ZBPRO_ZHA_CieEnrollCallback_t)(
                    ZBPRO_ZHA_CieEnrollReqDescr_t *const reqDescr,
                    ZBPRO_ZHA_CieEnrollConfParams_t *const confParams);

/**//**
 * \brief   Structure for descriptor of CIE Device Enroll request
 *  command.
 */
struct _ZBPRO_ZHA_CieEnrollReqDescr_t
{
    SYS_QueueElement_t               queueElement;
    ZBPRO_ZHA_CieEnrollReqParams_t   params;
    ZBPRO_ZHA_CieEnrollCallback_t    callback;
};

/**//**
 * \brief Privitive for the CIE Device Enroll procedure executing.
*/
void ZBPRO_ZHA_CieDeviceEnrollReq(ZBPRO_ZHA_CieEnrollReqDescr_t *reqDescr);

/**//**
 * \brief One Zone enroll indication parameters.
*/
typedef struct _ZBPRO_ZHA_CieEnrollIndParams_t
{
    uint8_t zoneID;                                         /*!< ZoneID. */
    ZBPRO_ZCL_SapIASZoneAttributeZoneType_t zoneType;       /*!< Type of zone. */
    ZBPRO_APS_ExtAddr_t zoneAddr;                           /*!< Zone device extended address. */
    ZBPRO_APS_EndpointId_t ep;                              /*!< Zone device endpoint */
} ZBPRO_ZHA_CieEnrollIndParams_t;

/**//**
 * \brief Indication for finishing enrolling process for one zone.
*/
void ZBPRO_ZHA_CieDeviceEnrollInd(ZBPRO_ZHA_CieEnrollIndParams_t *const indParams);


/**//**
 * \brief Type for the Panel Status.
*/
typedef  ZBPRO_ZCL_SapIasAcePanelStatus_t  ZbProZhaCiePanelStatus_t;

/**//**
 * \brief Struct for Set Panel Status request parameters.
 * \param status - new status for the CIE Panel.
*/
typedef struct _ZBPRO_ZHA_CieSetPanelStatusReqParams_t
{
    ZbProZhaCiePanelStatus_t  status;
} ZBPRO_ZHA_CieSetPanelStatusReqParams_t;

/**//**
 * \brief Struct for Set Panel Status request parameters.
 * \param status - current status of the CIE Panel (it could be differ from the status from request).
*/
typedef struct _ZBPRO_ZHA_CieSetPanelStatusConfParams_t
{
    ZbProZhaCiePanelStatus_t  status;
} ZBPRO_ZHA_CieSetPanelStatusConfParams_t;

/**//**
 * \brief Type for the Set Panel Status request descriptor.
*/
typedef struct _ZBPRO_ZHA_CieSetPanelStatusReqDescr_t ZBPRO_ZHA_CieSetPanelStatusReqDescr_t;

/**//**
 * \brief   Data type for Local Confirmation callback function of CIE Device Set Panel Status
 *  command.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void (*ZBPRO_ZHA_CieSetPanelStatusCallback_t)(ZBPRO_ZHA_CieSetPanelStatusReqDescr_t *const reqDescr,
        ZBPRO_ZHA_CieSetPanelStatusConfParams_t *const confParams);

/**//**
 * \brief CIE Device Set Panel Status request descriptor.
*/
struct _ZBPRO_ZHA_CieSetPanelStatusReqDescr_t
{
    ZBPRO_ZHA_CieSetPanelStatusReqParams_t   params;
    ZBPRO_ZHA_CieSetPanelStatusCallback_t    callback;
};

/**//**
 * \brief This function proceed some signal from user to change a CIE Panel Status
 * (it could be some button for example.)
 * \param [in] reqDescr - it is a new status which must be set to the CIE Panel.
*/
void ZBPRO_ZHA_CieDeviceSetPanelStatusReq(ZBPRO_ZHA_CieSetPanelStatusReqDescr_t * const reqDescr);


/**//**
 * \brief Enumerator for the Set Panel Status command results.
*/
typedef enum _ZBPRO_ZHA_CieSetPanelStatusResult_t
{
    ZBPRO_ZHA_CIE_SET_PANEL_STATUS_SUCCESS = 0,     /*<! Panel state was set successfully. */
    ZBPRO_ZHA_CIE_SET_PANEL_STATUS_FAILED,          /*<! Failed. For example because of the state NOT_READY. */
    ZBPRO_ZHA_CIE_SET_PANEL_STATUS_NOT_IMPLEMENTED  /*<! Set Panel Status with not implemented state. */
} ZBPRO_ZHA_CieSetPanelStatusResult_t;

/**//**
 * \brief Set Panel Status command indication parameters.
*/
typedef struct _ZBPRO_ZHA_CieSetPanelStatusIndParams_t
{
    ZbProZhaCiePanelStatus_t             currentStatus;
    ZBPRO_ZHA_CieSetPanelStatusResult_t  result;

} ZBPRO_ZHA_CieSetPanelStatusIndParams_t;

/**//**
 * \brief Indication of the Set Panel Status command finished.
*/
void ZBPRO_ZHA_CieDeviceSetPanelStatusInd(
    ZBPRO_ZHA_CieSetPanelStatusIndParams_t   *const   indParams);


/**//**
 * \brief Enumerator for the Bypass State of some Zone.
*/
typedef enum _ZBPRO_ZHA_CieZoneBypassState_t
{
    ZBPRO_ZHA_CIE_ZONE_BYPASS_STATE_NOT_BYPASSED = 0,           /*!< Zone is not bypassed now. */
    ZBPRO_ZHA_CIE_ZONE_BYPASS_STATE_BYPASSED,                   /*!< Zone is bypassed now. */
    ZBPRO_ZHA_CIE_ZONE_BYPASS_STATE_NOT_ALLOWED_TO_BYPASS,      /*!< Zone is not allowed to be bypassed. */
    ZBPRO_ZHA_CIE_ZONE_BYPASS_STATE_INVALID
} ZBPRO_ZHA_CieZoneBypassState_t;

/**//**
 * \brief Enumerator for the CIE Device Set Bypass State request results.
*/
typedef enum _ZBPRO_ZHA_CieSetBypassStateResult_t
{
    ZBPRO_ZHA_CIE_SET_BYPASS_STATE_RESULT_SUCCESS = 0,          /*!< Bypass state was set successfully. */
    ZBPRO_ZHA_CIE_SET_BYPASS_STATE_RESULT_UNKNOWN_ZONE_ID,      /*!< There is no Zone with specified ZoneID. */
    ZBPRO_ZHA_CIE_SET_BYPASS_STATE_RESULT_UNKNOWN_STATE         /*!< There is unknown Bypass State in the request. */
} ZBPRO_ZHA_CieSetBypassStateResult_t;

/**//**
 * \brief Set Zone Bypass status request parameters.
*/
typedef struct _ZBPRO_ZHA_CieZoneSetBypassStateReqParams_t
{
    uint8_t                            zoneID;          /*!< ID of the zone, which state must be changed. */
    ZBPRO_ZHA_CieZoneBypassState_t     bypassState;     /*!< Bypass Status, which must be set. */
} ZBPRO_ZHA_CieZoneSetBypassStateReqParams_t;

/**//**
 * \brief Set Zone Bypass status request confirmation parameters.
*/
typedef struct _ZBPRO_ZHA_CieZoneSetBypassStateConfParams_t
{
    ZBPRO_ZHA_CieSetBypassStateResult_t  result;         /*!< Result of the Set Bypass State request. */
} ZBPRO_ZHA_CieZoneSetBypassStateConfParams_t;


/**//**
 * \brief Type for the CIE Device Set Bypass State request descriptor.
*/
typedef struct _ZBPRO_ZHA_CieZoneSetBypassStateReqDescr_t   ZBPRO_ZHA_CieZoneSetBypassStateReqDescr_t;

/**//**
 * \brief Set Zone Bypass status request callback.
*/
typedef void (*ZBPRO_ZHA_CieZoneSetBypassStateCallback_t)(ZBPRO_ZHA_CieZoneSetBypassStateReqDescr_t *const reqDescr,
        ZBPRO_ZHA_CieZoneSetBypassStateConfParams_t *const confParams);

/**//**
 * \brief Set Zone Bypass status request descriptor.
*/
struct _ZBPRO_ZHA_CieZoneSetBypassStateReqDescr_t
{
    ZBPRO_ZHA_CieZoneSetBypassStateReqParams_t   params;
    ZBPRO_ZHA_CieZoneSetBypassStateCallback_t    callback;
};


/**//**
 * \brief Set Zone Bypass status command. It is used to set Bypass status to the Zone from Zone Table.
*/
void ZBPRO_ZHA_CieZoneSetBypassStateReq(
    ZBPRO_ZHA_CieZoneSetBypassStateReqDescr_t   *const descr);


/**//**
 * \brief Enumerator for the CIE Device requests status.
*/
typedef enum _ZBPRO_ZHA_CieDeviceStatus_t
{
    ZBPRO_ZHA_CieDevice_SUCCESS = 0,
    ZBPRO_ZHA_CieDevice_NOT_PERMITTED,
    ZBPRO_ZHA_CieDevice_FAIL
} ZBPRO_ZHA_CieDeviceStatus_t;


/**//**
 * \brief Enumerator for the CIE Device requests ID.
 * \note   It is an internal enumerator.
*/
typedef enum _ZbProZhaCieDeviceRequestID_t
{
    ZBPRO_ZHA_CieDevice_REGISTER=0,
    ZBPRO_ZHA_CieDevice_UNREGISTER
} ZbProZhaCieDeviceRequestID_t;

/**//**
 * \brief  Descriptor for the CIE Device requests
 * \note   It is an internal structure.
*/
typedef struct _ZbProZhaCieDeviceReqDesr_t
{
    SYS_QueueElement_t             serviceData; /*!< Helper field to allow that structure object to be queued. */
    ZbProZhaCieDeviceRequestID_t   requestID;   /*!< Request ID. */
} ZbProZhaCieDeviceReqDesr_t;


/**//**
 * \brief   Structure for parameters of ZCL CIE Device Register confirmation.
 * \details Possible values for the \c status parameter are the following:
 *  - SUCCESS                   The CIE Device was successfully registered.
 *  - NOT_PERMITTED             The CIE Device was already registered - double registration
                                is not permitted.
 *                              channel, i.e., the CSMA-CA mechanism has failed.
 *  - FAIL                      The CIE Device was not registered because of some fail
                                (The endpoin in the Register request is incorrect - it should be
                                registered with the clusters required by CIE Device:
 *                               - IAS ACE server side,
 *                               - IAS WD client side,
 *                               - Identify client side,
 *                               - IAS Zones client side.
                                )
 */
typedef struct _ZBPRO_ZHA_CieDeviceRegisterConfParams_t
{
    ZBPRO_ZHA_CieDeviceStatus_t  status;
} ZBPRO_ZHA_CieDeviceRegisterConfParams_t;


/**//**
 * \brief  ZCL CIE Device Register request parameters.
 * \note   The \c endpoint parameter should contain the Endpoint ID wich was registered
 *         with the next clusters:
 *                               - IAS ACE server side,
 *                               - IAS WD client side,
 *                               - Identify client side,
 *                               - IAS Zones client side.
*/
typedef struct _ZBPRO_ZHA_CieDeviceRegisterReqParams_t
{
    ZBPRO_APS_EndpointId_t       endpoint;
} ZBPRO_ZHA_CieDeviceRegisterReqParams_t;


/**//**
 * \brief   Structure for descriptor of ZCL CIE Device Register request
 *  command.
 */
typedef struct _ZBPRO_ZHA_CieDeviceRegisterReqDescr_t ZBPRO_ZHA_CieDeviceRegisterReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of ZCL CIE Device Register
 *  command.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZHA_CieDeviceRegisterConfCallback_t(
                   ZBPRO_ZHA_CieDeviceRegisterReqDescr_t   *const  reqDescr,
                   ZBPRO_ZHA_CieDeviceRegisterConfParams_t *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL CIE Device Register request
 *  command.
 */
struct _ZBPRO_ZHA_CieDeviceRegisterReqDescr_t
{
    /* 32-bit data. */

    ZbProZhaCieDeviceReqDesr_t                     service;    /*!< Internal field. Shoul not be used by Application. */

    ZBPRO_ZHA_CieDeviceRegisterConfCallback_t     *callback;   /*!< ZCL Confirmation callback handler entry point. */

    ZBPRO_ZHA_CieDeviceRegisterReqParams_t         params;     /*!< ZCL Request parameters structure. */
};


/**//**
 * \brief Privitive for the CIE Device registration.
 * \note  Only one CIE Device could be registered. All other
 *        Register Requests will return the status NOT_PERMITTED
 *        in the confirmation.
*/
void ZBPRO_ZHA_CieDeviceRegisterReq(
    ZBPRO_ZHA_CieDeviceRegisterReqDescr_t * const descr);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on ZCL CIE Device Unregister request.
 * \details Possible values for the \c status parameter are the following:
 *  - SUCCESS                   The CIE Device was successfully unregistered.
 *  - FAIL                      The CIE Device was not unregistered because it was not already registered.
 */
typedef struct _ZBPRO_ZHA_CieDeviceUnregisterConfParams_t
{
    ZBPRO_ZHA_CieDeviceStatus_t  status;
} ZBPRO_ZHA_CieDeviceUnregisterConfParams_t;


/**//**
 * \brief  ZCL CIE Device Unregister request parameters.
 * \note   The \c endpoint parameter should contain the Endpoint ID wich was used
 *         for the CIE Device registration.
*/
typedef struct _ZBPRO_ZHA_CieDeviceUnregisterReqParams_t
{
    ZBPRO_APS_EndpointId_t       endpoint;
} ZBPRO_ZHA_CieDeviceUnregisterReqParams_t;


/**//**
 * \brief   Structure for descriptor of ZCL CIE Device Unregister request command.
 */
typedef struct _ZBPRO_ZHA_CieDeviceUnregisterReqDescr_t ZBPRO_ZHA_CieDeviceUnregisterReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of ZCL CIE Device Unregister
 *  command.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZHA_CieDeviceUnregisterConfCallback_t(
                   ZBPRO_ZHA_CieDeviceUnregisterReqDescr_t   *const  reqDescr,
                   ZBPRO_ZHA_CieDeviceUnregisterConfParams_t *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL CIE Device Register request
 *  command.
 */
struct _ZBPRO_ZHA_CieDeviceUnregisterReqDescr_t
{
    /* 32-bit data. */
    ZbProZhaCieDeviceReqDesr_t                       service;    /*!< Internal field. Shoul not be used
                                                                      by Application. */

    ZBPRO_ZHA_CieDeviceUnregisterConfCallback_t     *callback;   /*!< ZCL Confirmation callback handler entry point. */

    ZBPRO_ZHA_CieDeviceUnregisterReqParams_t         params;     /*!< ZCL Request parameters structure. */
};


/**//**
 * \brief Privitive for the CIE Device unregistration.
 * \note  If the CIE Device was not already registered OR if the enpoint in the request
 *        is not equal with endpoint of the CIE Device,
 *        this request will return FAIL status.
*/
void ZBPRO_ZHA_CieDeviceUnregisterReq(
                   ZBPRO_ZHA_CieDeviceUnregisterReqDescr_t * const descr);

#endif /* _BB_ZBPRO_ZHA_SAP_CIE_DEVICE_H */