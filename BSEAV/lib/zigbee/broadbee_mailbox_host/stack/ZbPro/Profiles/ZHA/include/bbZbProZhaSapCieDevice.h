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

typedef enum _ZBPRO_ZHA_CieEnrollStatus_t
{
    ZBPRO_ZHA_CIE_ENROLL_SUCCESS,
    ZBPRO_ZHA_CIE_ENROLL_CANNOT_RESOLVE_TARGET_ADDR,
} ZBPRO_ZHA_CieEnrollStatus_t;

typedef struct _ZBPRO_ZHA_CieEnrollReqParams_t
{
    ZBPRO_APS_Address_t                 addr;   // NOTE: can be broadcast
    SYS_Time_t                          scanDurationMs;
    SYS_Time_t                          permitEnrollDurationMs;
    Bool8_t                             autoREsponseMode;
} ZBPRO_ZHA_CieEnrollReqParams_t;

typedef struct _ZBPRO_ZHA_CieEnrollConfParams_t
{
    ZBPRO_ZHA_CieEnrollStatus_t status;
} ZBPRO_ZHA_CieEnrollConfParams_t;

typedef struct _ZBPRO_ZHA_CieEnrollReqDescr_t ZBPRO_ZHA_CieEnrollReqDescr_t;

typedef void (*ZBPRO_ZHA_CieEnrollCallback_t)(ZBPRO_ZHA_CieEnrollReqDescr_t *const reqDescr,
        ZBPRO_ZHA_CieEnrollConfParams_t *const confParams);

struct _ZBPRO_ZHA_CieEnrollReqDescr_t
{
    SYS_QueueElement_t   queueElement; // NOTE: may be need to replace ZbProZhaServiceField_t...
    ZBPRO_ZHA_CieEnrollReqParams_t   params;
    ZBPRO_ZHA_CieEnrollCallback_t    callback;
};

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

typedef struct _ZBPRO_ZHA_CieSetPanelStatusReqDescr_t ZBPRO_ZHA_CieSetPanelStatusReqDescr_t;

typedef void (*ZBPRO_ZHA_CieSetPanelStatusCallback_t)(ZBPRO_ZHA_CieSetPanelStatusReqDescr_t *const reqDescr,
        ZBPRO_ZHA_CieSetPanelStatusConfParams_t *const confParams);

/**//**
 * \brief Set Panel Status request descriptor.
*/
struct _ZBPRO_ZHA_CieSetPanelStatusReqDescr_t
{
    ZBPRO_ZHA_CieSetPanelStatusReqParams_t   params;
    ZBPRO_ZHA_CieSetPanelStatusCallback_t    callback;
};

/**//**
 * \brief This function proceed some signal from user to change a CIE Panel Status
 * (it could be some button for example.)
 * \param [in] newStatus - it is a new status which must be set to the CIE Panel.
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





typedef enum _ZBPRO_ZHA_CieZoneBypassState_t
{
    ZBPRO_ZHA_CIE_ZONE_BYPASS_STATE_NOT_BYPASSED = 0,           /*!< Zone is not bypassed now. */
    ZBPRO_ZHA_CIE_ZONE_BYPASS_STATE_BYPASSED,                   /*!< Zone is bypassed now. */
    ZBPRO_ZHA_CIE_ZONE_BYPASS_STATE_NOT_ALLOWED_TO_BYPASS,      /*!< Zone is not allowed to be bypassed. */
    ZBPRO_ZHA_CIE_ZONE_BYPASS_STATE_INVALID
} ZBPRO_ZHA_CieZoneBypassState_t;

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

#endif /* _BB_ZBPRO_ZHA_SAP_CIE_DEVICE_H */
