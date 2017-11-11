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

/*******************************************************************************
 *
 * DESCRIPTION:
 *      MLME-START service data types definition.
 *
*******************************************************************************/

#ifndef _BB_MAC_SAP_TYPES_START_H
#define _BB_MAC_SAP_TYPES_START_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapPib.h"            /* MAC-PIB for MAC-SAP definitions. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for parameters of the MLME-START.request.
 * \ingroup StartReq
 * \note    The following parameters are expected to be equal to the following values:
 *  - BeaconOrder                   15, means nonbeacon-enabled PAN,
 *  - CoordRealignment              FALSE, is not used by ZigBee PRO and RF4CE,
 *  - CoordRealignSecurityLevel     0x00 'None', MAC security is not implemented,
 *  - BeaconSecurityLevel           0x00 'None', MAC security is not implemented.
 *
 *  All these parameters are ignored and assumed to be equal to their default values
 *  mentioned here; or they shall be assigned with these values directly by the higher
 *  layer if _MAC_SAP_PROCESS_REDUNDANT_PARAMS_ conditional build key is defined by the
 *  project make configuration file, otherwise INVALID_PARAMETER status will be returned.
 * \note    The following parameters are ignored for the case of a nonbeacon-enabled PAN
 *  (note that BeaconOrder = 15 means nonbeacon-enabled PAN):
 *  - StartTime             periodic beacons are not transmitted,
 *  - SuperframeOrder       assumed to be equal 15 as the BeaconOrder,
 *  - BatteryLifeExtension  assumed to be equal False, it means that coordinator remains
 *      enabled during the entire CAP period (i.e., during the whole time),
 *
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.14.1.1, table 72.
 */
typedef struct _MAC_StartReqParams_t
{
    /* 32-bit data. */
    HAL_Symbol__Tstamp_t    startTime;                      /*!< The time at which to begin transmitting beacons. */

    /* 16-bit data. */
    MAC_PanId_t             panId;                          /*!< The PAN identifier to be used by the device. */

    /* 8-bit data. */
    PHY_Channel_t           logicalChannel;                 /*!< The logical channel on which to start using the new
                                                                superframe configuration. */

    PHY_Page_t              channelPage;                    /*!< The channel page on which to begin using the new
                                                                superframe configuration. */

    MAC_BeaconOrder_t       beaconOrder;                    /*!< How often the beacon is to be transmitted. A value of
                                                                15 indicates that the coordinator will not transmit
                                                                periodic beacons. */

    MAC_SuperframeOrder_t   superframeOrder;                /*!< The length of the active portion of the superframe,
                                                                including the beacon frame. */

    MAC_PanCoordinator_t    panCoordinator;                 /*!< If this value is TRUE, the device will become the PAN
                                                                coordinator of a new PAN. */

    MAC_BattLifeExt_t       batteryLifeExtension;           /*!< If this value is FALSE, the receiver of the beaconing
                                                                device remains enabled for the entire CAP. */

    Bool8_t                 coordRealignment;               /*!< TRUE if a coordinator realignment command is to be
                                                                transmitted prior to changing the superframe
                                                                configuration or FALSE otherwise. */

    MAC_SecurityLevel_t     coordRealignSecurityLevel;      /*!< The security level to be used for coordinator
                                                                realignment command frames. */

    MAC_SecurityLevel_t     beaconSecurityLevel;            /*!< The security level to be used for beacon frames. */

} MAC_StartReqParams_t;


/**//**
 * \brief   Structure for parameters of the MLME-START.confirm.
 * \ingroup StartConf
 * \details Possible values for the \c status parameter are the following:
 *  - SUCCESS                   The requested operation was completed successfully.
 *  - INVALID_PARAMETER         A parameter in the primitive is either not supported or is
 *      out of the valid range.
 *  - NO_SHORT_ADDRESS          The operation failed because a 16-bit short address was
 *      not allocated.
 *  - UNSUPPORTED_SECURITY      The requested security level is not supported.
 *  - RESET                     An MLME-RESET.request was issued prior to execution of the
 *      MLME-START.request being confirmed.
 *
 * \note    The following values for the \c status parameter are not used due to listed
 *  reasons:
 *  - CHANNEL_ACCESS_FAILURE                            This implementation of the MAC
 *      does not support True value for the CoordRealignment parameter and doesn't issue
 *      the MAC Coordinator Realignment command prior to start a PAN.
 *  - COUNTER_ERROR, FRAME_TOO_LONG, UNAVAILABLE_KEY    This implementation of the MAC
 *      does not support the MAC Security.
 *  - SUPERFRAME_OVERLAP, TRACKING_OFF                  This implementation of the MAC is
 *      not able to establish a beacon-enabled PAN.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.14.2.1, table 73.
 */
typedef struct _MAC_StartConfParams_t
{
    /* 8-bit data. */
    MAC_Status_t  status;       /*!< The result of the attempt to start using an updated superframe configuration. */

} MAC_StartConfParams_t;


/**//**
 * \brief   Structure for descriptor of the MLME-START.request.
 * \ingroup StartReq
 */
typedef struct _MAC_StartReqDescr_t  MAC_StartReqDescr_t;


/**//**
 * \brief   Template for callback handler-function of the MLME-START.confirm.
 * \ingroup StartConf
 * \param[in]   reqDescr    Pointer to the confirmed request descriptor.
 * \param[in]   confParams  Pointer to the confirmation parameters object.
 * \details Call functions of this type provided by higher layers of corresponding MAC
 *  contexts, ZigBee PRO and RF4CE, from the MAC to issue the MLME-START.confirm to the
 *  higher layer that originally issued the request primitive to the MAC.
 * \details To issue the confirmation primitive the MAC calls the confirmation callback
 *  handler-function that was specified with the \c callback parameter of the original
 *  request primitive descriptor that is pointed here by the \p reqDescr argument.
 * \details The request descriptor object that was originally used to issue the request to
 *  the MAC and is pointed here with the \p reqDescr is released by the MAC for random
 *  use by the higher layer (the owner of the request descriptor object) when this
 *  confirmation callback handler-function is called by the MAC.
 * \details Treat the parameters structure pointed by the \p confParams and passed into
 *  the confirmation callback handler-function as it has been allocated in the program
 *  stack by the MAC before calling this callback handler and will be destroyed just after
 *  this callback function returns.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.14.2.
 */
typedef void MAC_StartConfCallback_t(MAC_StartReqDescr_t *const reqDescr, MAC_StartConfParams_t *const confParams);


/**//**
 * \brief   Structure for descriptor of the MLME-START.request.
 * \ingroup StartReq
 */
struct _MAC_StartReqDescr_t
{
    /* 32-bit data. */
    MAC_StartConfCallback_t *callback;      /*!< Entry point of the confirmation callback function. */

#ifndef _HOST_
    /* Structured data. */
    MacServiceField_t        service;       /*!< MAC requests service field. */
#endif

    MAC_StartReqParams_t     params;        /*!< Request parameters structured object. */
};


#endif /* _BB_MAC_SAP_TYPES_START_H */

/* eof bbMacSapTypesStart.h */