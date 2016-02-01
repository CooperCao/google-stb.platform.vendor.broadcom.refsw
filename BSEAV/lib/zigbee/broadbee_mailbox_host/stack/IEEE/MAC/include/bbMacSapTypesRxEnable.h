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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/bbMacSapTypesRxEnable.h $
*
* DESCRIPTION:
*   MLME-RX-ENABLE service data types definition.
*
* $Revision: 3159 $
* $Date: 2014-08-05 19:11:02Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_SAP_TYPES_RX_ENABLE_H
#define _BB_MAC_SAP_TYPES_RX_ENABLE_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapDefs.h"           /* MAC-SAP common definitions. */
#include "bbMacSapService.h"        /* MAC-SAP service data types. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for parameters of the MLME-RX-ENABLE.request.
 * \details The transceiver will be switched on (or left in RX_ON mode when idle) if the
 *  \c rxOnDuration is not equal to zero when calling the MLME-RX-ENABLE.request. The
 *  transceiver will be switched off if the \c rxOnDuration is equal to zero and there is
 *  no other reason to left the transceiver enabled.
 * \details The next MLME-RX-ENABLE.request cancels the previous one.
 * \details When the time period specified with the \c rxOnDuration elapses, the
 *  transceiver is switched off only if the following conditions are met: (1) the MAC-PIB
 *  attribute macRxOnWhenIdle is equal to FALSE, and (2) the MAC-LE is not currently busy.
 *  If at least one condition is violated, the transceiver will be left enabled; and it
 *  will be switched off as soon as both conditions are met.
 * \details If the \c rxOnDuration is equal to zero, the transceiver will be switched off
 *  only if the following conditions are met: (1) the MAC-PIB attribute macRxOnWhenIdle is
 *  equal to FALSE, and (2) the MAC is not currently busy. If at least one condition is
 *  violated the transceiver will be left enabled; and it will be switched off as soon as
 *  both conditions are met.
 * \details For the case of dual-context MAC with both contexts enabled, the transceiver
 *  will be actually switched off only if the following conditions are met: (1) no
 *  designation to stay in RX_ON when idle was assigned with the MLME-RX-ENABLE.request
 *  for any of two contexts, and (2) MAC-PIB attributes macRxOnWhenIdle are equal to FALSE
 *  for both MAC contexts, and (3) both MAC contexts are not currently busy. If the second
 *  context does not allow to switch off the transceiver while the first context allows,
 *  the transceiver hardware will be actually left enabled, but the MAC-LE will simulate
 *  the switched-off radio for the first context by rejecting all frames addressed to the
 *  first context.
 * \note The following standard parameters are excluded due to the following reasons:
 *  - DeferPermit   is ignored for nonbeacon-enabled PAN,
 *  - RxOnTime      is ignored for nonbeacon-enabled PAN.
 *
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.10.1.1, table 65.
 */
typedef struct _MAC_RxEnableReqParams_t
{
    /* 32-bit data. */
    HAL_SymbolPeriod_t  rxOnDuration;       /*!< The number of symbols for which the receiver is to be enabled. */

} MAC_RxEnableReqParams_t;


/**//**
 * \brief   Maximum allowed value for the RxOnDuration parameter of the
 *  MLME-RX-ENABLE.request.
 * \details The RxOnDuration parameter of the MLME-RX-ENABLE.request shall belong to the
 *  range from 0x00000000 to 0x00FFFFFF.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.10.1.1, table 65.
 */
#define MAC_RX_ON_MAX_DURATION      0x00FFFFFF


/**//**
 * \brief   Special value for the RxOnDuration parameter of the MLME-RX-ENABLE.request to
 *  disable the receiver.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.10.1.1, table 65.
 */
#define MAC_RX_OFF_COMMAND          0x00000000


/**//**
 * \brief   Structure for parameters of the MLME-RX-ENABLE.confirm.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.10.2.1, table 66.
 */
typedef struct _MAC_RxEnableConfParams_t
{
    /* 8-bit data. */
    MAC_Status_t  status;       /*!< The result of the request to enable or disable the receiver. */

} MAC_RxEnableConfParams_t;


/**//**
 * \brief   Structure for descriptor of the MLME-RX-ENABLE.request.
 */
typedef struct _MAC_RxEnableReqDescr_t  MAC_RxEnableReqDescr_t;


/**//**
 * \brief   Template for callback handler-function of the MLME-RX-ENABLE.confirm.
 * \param[in]   reqDescr    Pointer to the confirmed request descriptor.
 * \param[in]   confParams  Pointer to the confirmation parameters object.
 * \details Call functions of this type provided by higher layers of corresponding MAC
 *  contexts, ZigBee PRO and RF4CE, from the MAC to issue the MLME-RX-ENABLE.confirm to
 *  the higher layer that originally issued the request primitive to the MAC.
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
 *  See IEEE 802.15.4-2006, subclause 7.1.10.2.
 */
typedef void MAC_RxEnableConfCallback_t(MAC_RxEnableReqDescr_t *const    reqDescr,
                                        MAC_RxEnableConfParams_t *const  confParams);


/**//**
 * \brief   Structure for descriptor of the MLME-RX-ENABLE.request.
 */
struct _MAC_RxEnableReqDescr_t
{
    /* 32-bit data. */
    MAC_RxEnableConfCallback_t *callback;       /*!< Entry point of the confirmation callback function. */

    /* Structured data. */
    MacServiceField_t           service;        /*!< MAC requests service field. */

    MAC_RxEnableReqParams_t     params;         /*!< Request parameters structured object. */
};


#endif /* _BB_MAC_SAP_TYPES_RX_ENABLE_H */