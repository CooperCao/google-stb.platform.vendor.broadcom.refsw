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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/bbMacSapTypesStart.h $
*
* DESCRIPTION:
*   MLME-START service data types definition.
*
* $Revision: 2722 $
* $Date: 2014-06-24 19:37:15Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_SAP_TYPES_START_H
#define _BB_MAC_SAP_TYPES_START_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapPib.h"            /* MAC-PIB for MAC-SAP definitions. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for parameters of the MLME-START.request.
 * \note The following standard parameters are excluded due to the following reasons:
 *  - BeaconOrder           is constantly equal to 15 for non-beacon-enabled PAN,
 *  - StartTime             is ignored if BeaconOrder = 15,
 *  - SuperframeOrder       is ignored (equal to 15) if BeaconOrder = 15,
 *  - BatteryLifeExtension  is ignored if BeaconOrder = 15,
 *  - CoordRealignment      is not used by ZigBee PRO and RF4CE (equal to FALSE).
 * \note    Security parameters are excluded because MAC security is not implemented.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.14.1, table 72.
 */
typedef struct _MAC_StartReqParams_t
{
    /* TODO: This field is redundant. Wrap it with a conditional build key. */
    /* 32-bit data. */
    HAL_SymbolTimestamp_t   startTime;                      /*!< The time at which to begin transmitting beacons. */

    /* 16-bit data. */
    MAC_PanId_t             panId;                          /*!< The PAN identifier to be used by the device. */

    /* 8-bit data. */
    PHY_LogicalChannelId_t  logicalChannel;                 /*!< The logical channel on which to start using the new
                                                                superframe configuration. */

    PHY_ChannelPageId_t     channelPage;                    /*!< The channel page on which to begin using the new
                                                                superframe configuration. */

    /* TODO: This field is redundant. Wrap it with a conditional build key. */
    MAC_BeaconOrder_t       beaconOrder;                    /*!< How often the beacon is to be transmitted. A value of
                                                                15 indicates that the coordinator will not transmit
                                                                periodic beacons. */

    /* TODO: This field is redundant. Wrap it with a conditional build key. */
    MAC_SuperframeOrder_t   superframeOrder;                /*!< The length of the active portion of the superframe,
                                                                including the beacon frame. */

    MAC_PanCoordinator_t    panCoordinator;                 /*!< If this value is TRUE, the device will become the PAN
                                                                coordinator of a new PAN. */

    /* TODO: This field is redundant. Wrap it with a conditional build key. */
    MAC_BattLifeExt_t       batteryLifeExtension;           /*!< If this value is FALSE, the receiver of the beaconing
                                                                device remains enabled for the entire CAP. */

    /* TODO: This field is redundant. Wrap it with a conditional build key. */
    Bool8_t                 coordRealignment;               /*!< TRUE if a coordinator realignment command is to be
                                                                transmitted prior to changing the superframe
                                                                configuration or FALSE otherwise. */

    /* TODO: This field is redundant. Wrap it with a conditional build key. */
    MAC_SecurityLevel_t     coordRealignSecurityLevel;      /*!< The security level to be used for coordinator
                                                                realignment command frames. */

    /* TODO: This field is redundant. Wrap it with a conditional build key. */
    MAC_SecurityLevel_t     beaconSecurityLevel;            /*!< The security level to be used for beacon frames. */

} MAC_StartReqParams_t;


/**//**
 * \brief   Structure for parameters of the MLME-START.confirm.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.14.2, table 73.
 */
typedef struct _MAC_StartConfParams_t
{
    /* 8-bit data. */
    MAC_Status_t  status;       /*!< The result of the attempt to start using an updated superframe configuration. */

} MAC_StartConfParams_t;


/**//**
 * \brief   Structure for descriptor of the MLME-START.request.
 */
typedef struct _MAC_StartReqDescr_t  MAC_StartReqDescr_t;


/**//**
 * \brief   Template for callback handler-function of the MLME-START.confirm.
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
 */
struct _MAC_StartReqDescr_t
{
    /* 32-bit data. */
    MAC_StartConfCallback_t *callback;      /*!< Entry point of the confirmation callback function. */

    /* Structured data. */
    MacServiceField_t        service;       /*!< MAC requests service field. */

    MAC_StartReqParams_t     params;        /*!< Request parameters structured object. */
};


#endif /* _BB_MAC_SAP_TYPES_START_H */