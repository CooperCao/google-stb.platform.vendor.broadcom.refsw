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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/bbMacSapForRF4CE.h $
*
* DESCRIPTION:
*   MAC-SAP interface for the ZigBee RF4CE stack.
*
* $Revision: 3536 $
* $Date: 2014-09-11 07:21:52Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_SAP_FOR_RF4CE_H
#define _BB_MAC_SAP_FOR_RF4CE_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapTypesData.h"          /* MCPS-DATA service data types. */
#include "bbMacSapTypesGet.h"           /* MLME-GET service data types. */
#include "bbMacSapTypesReset.h"         /* MLME-RESET service data types. */
#include "bbMacSapTypesRxEnable.h"      /* MLME-RX-ENABLE service data types. */
#include "bbMacSapTypesSet.h"           /* MLME-SET service data types. */
#include "bbMacSapTypesStart.h"         /* MLME-START service data types. */

#if defined(_MAC_CONTEXT_RF4CE_TARGET_)
# include "bbMacSapTypesBeacon.h"       /* MLME-BEACON service data types. */
# include "bbMacSapTypesCommStatus.h"   /* MLME-COMM-STATUS service data types. */
# include "bbMacSapTypesScan.h"         /* MLME-SCAN service data types. */
#endif

#include "bbHalRandom.h"                /* Random Number Generator Hardware interface. */


/************************* VALIDATIONS **************************************************/
#if !defined(_MAC_CONTEXT_RF4CE_)
# error This header shall be compiled only if the RF4CE context is included into the build.
#endif


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Accepts MCPS-DATA.request for the RF4CE context of the MAC.
 * \param[in]   reqDescr    Pointer to the request descriptor object.
 * \note
 *  The descriptor pointed by \p reqDescr must reside in the global memory space and must
 *  be preserved by the caller until confirmation from the MAC is received. The \c service
 *  field of the request descriptor is used by the MAC during request processing. The
 *  caller shall set the \c callback field to the entry point of its MCPS-DATA.confirm
 *  handler-function.
 * \note
 *  It is allowed to commence a new request to the MAC directly from the context of the
 *  confirmation handler. The same request descriptor object may be used for the new
 *  request to MAC just after it is freed with the confirmation on the previous request
 *  (of the same or different MAC primitive).
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.1.1.1, 7.1.1.2.
*****************************************************************************************/
MAC_PUBLIC void RF4CE_MAC_DataReq(MAC_DataReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Issues MCPS-DATA.indication to the RF4CE Stack from the MAC.
 * \param[in]   indParams   Pointer to the indication parameters object.
 * \details
 *  RF4CE shall provide MCPS-DATA.indication handler-function according to the template:
 * \code
 *  void RF4CE_MAC_DataInd(MAC_DataIndParams_t *const indParams) { ... }
 * \endcode
 * \note
 *  Treat the parameters structure pointed by the \p indParams and passed into the
 *  indication callback handler-function as it has been allocated in the program stack by
 *  the MAC before calling this callback handler and will be destroyed just after this
 *  callback returns.
 * \note
 *  The payload pointed by \p indParams->payload is allocated by the MAC in the dynamic
 *  memory and must be dismissed by the higher layer; for the case of dual-context MAC
 *  each stack higher layer must dismiss its own copy of payload. The payload may be
 *  empty (i.e., its length may be equal to zero).
 * \note
 *  It is allowed to commence a new request to the MAC directly from the context of this
 *  indication handler.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.1.1.3.
*****************************************************************************************/
MAC_PUBLIC MAC_DataIndCallback_t RF4CE_MAC_DataInd;


#if defined(_MAC_CONTEXT_RF4CE_TARGET_)
# if defined(_MAC_TESTER_)
/*************************************************************************************//**
 * \brief   Accepts MLME-BEACON.response for the RF4CE context of the MAC.
 * \param[in]   respDescr   Pointer to the response descriptor object.
 * \details
 *  This nonstandard primitive is used by the Test Harness to instigate the MAC to
 *  transmit a Beacon frame. Normally MAC transmits Beacons only automatically when the
 *  Beacon Request MAC Command is received.
 * \note
 *  The descriptor pointed by \p respDescr must reside in the global memory space and must
 *  be preserved by the caller until confirmation from the MAC is received. The \c service
 *  field of the response descriptor is used by the MAC during response processing. The
 *  caller shall set the \c callback field to the entry point of its MLME-BEACON.confirm
 *  handler-function.
 * \note
 *  It is allowed to commence a new request to the MAC directly from the context of the
 *  confirmation handler. The same request descriptor object may be used for the new
 *  request to MAC just after it is freed with the confirmation on the previous request
 *  (of the same or different MAC primitive).
 * \note
 *  This primitive is not implemented for the RF4CE-Controller.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.5.2.4.
*****************************************************************************************/
/* TODO: Rename to REQUEST. */
MAC_PUBLIC void RF4CE_MAC_BeaconResp(MAC_BeaconRespDescr_t *const respDescr);
# endif /* _MAC_TESTER_ */


/*************************************************************************************//**
 * \brief   Issues MLME-BEACON-NOTIFY.indication to the RF4CE Stack from the MAC.
 * \param[in]   indParams   Pointer to the indication parameters object.
 * \details
 *  RF4CE shall provide MLME-BEACON-NOTIFY.indication handler-function according to the
 *  template:
 * \code
 *  void RF4CE_MAC_BeaconNotifyInd(MAC_BeaconNotifyIndParams_t *const indParams) { ... }
 * \endcode
 * \note
 *  Treat the parameters structure pointed by the \p indParams and passed into the
 *  indication callback handler-function as it has been allocated in the program stack by
 *  the MAC before calling this callback handler and will be destroyed just after this
 *  callback returns.
 * \note
 *  The payload pointed by \p indParams->payload is allocated by the MAC in the dynamic
 *  memory and must be dismissed by the higher layer; for the case of dual-context MAC
 *  each stack higher layer must dismiss its own copy of payload. The payload may be
 *  empty (i.e., its length may be equal to zero).
 * \note
 *  It is allowed to commence a new request to the MAC directly from the context of this
 *  indication handler.
 * \note
 *  This primitive is not implemented for the RF4CE-Controller.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.1.5.1.
*****************************************************************************************/
MAC_PUBLIC MAC_BeaconNotifyIndCallback_t RF4CE_MAC_BeaconNotifyInd;


/*************************************************************************************//**
 * \brief   Issues MLME-COMM-STATUS.indication to the RF4CE Stack from the MAC.
 * \param[in]   indParams   Pointer to the indication parameters object.
 * \details
 *  RF4CE shall provide MLME-COMM-STATUS.indication handler-function according to the
 *  template:
 * \code
 *  void RF4CE_MAC_CommStatusInd(MAC_CommStatusIndParams_t *const indParams) { ... }
 * \endcode
 * \details
 *  The MAC issues the MLME-COMM-STATUS.indication only for the case when a MAC secured
 *  frame is received. The RF4CE stack does not implement the MLME-ASSOCIATE.response and
 *  the MLME-ORPHAN.response; consequently the MLME-COMM-STATUS.indication is not ever
 *  issued by the MAC to the RF4CE stack following these two responses.
 * \note
 *  Treat the parameters structure pointed by the \p indParams and passed into the
 *  indication callback handler-function as it has been allocated in the program stack by
 *  the MAC before calling this callback handler and will be destroyed just after this
 *  callback returns.
 * \note
 *  It is allowed to commence a new request to the MAC directly from the context of this
 *  indication handler.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.1.12.1.
*****************************************************************************************/
MAC_PUBLIC MAC_CommStatusIndCallback_t RF4CE_MAC_CommStatusInd;
#endif /* _MAC_CONTEXT_RF4CE_TARGET_ */


/*************************************************************************************//**
 * \brief   Accepts MLME-GET.request for the RF4CE context of the MAC.
 * \param[in]   reqDescr    Pointer to the request descriptor object.
 * \note
 *  The descriptor pointed by \p reqDescr must reside in the global memory space and must
 *  be preserved by the caller until confirmation from the MAC is received. The \c service
 *  field of the request descriptor is used by the MAC during request processing. The
 *  caller shall set the \c callback field to the entry point of its MLME-GET.confirm
 *  handler-function.
 * \note
 *  The payload pointed by \p confParams->payload is allocated by the MAC or PHY in the
 *  static memory; still it is recommended to dismiss it by the higher layer. The payload
 *  may be empty (i.e., its length may be equal to zero).
 * \note
 *  It is allowed to commence a new request to the MAC directly from the context of the
 *  confirmation handler. The same request descriptor object may be used for the new
 *  request to MAC just after it is freed with the confirmation on the previous request
 *  (of the same or different MAC primitive).
 * \note
 *  This primitive may be called for the MAC Context that is still disabled.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.1.6.1, 7.1.6.2.
*****************************************************************************************/
MAC_PUBLIC void RF4CE_MAC_GetReq(MAC_GetReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Accepts MLME-RESET.request for the RF4CE context of the MAC.
 * \param[in]   reqDescr    Pointer to the request descriptor object.
 * \note
 *  The descriptor pointed by \p reqDescr must reside in the global memory space and must
 *  be preserved by the caller until confirmation from the MAC is received. The \c service
 *  field of the request descriptor is used by the MAC during request processing. The
 *  caller shall set the \c callback field to the entry point of its MLME-RESET.confirm
 *  handler-function.
 * \note
 *  This primitive resets only the RF4CE context of the MAC, it does not interfere with
 *  the ZigBee PRO context of the MAC.
 * \note
 *  It is allowed to commence a new request to the MAC directly from the context of the
 *  confirmation handler. The same request descriptor object may be used for the new
 *  request to MAC just after it is freed with the confirmation on the previous request
 *  (of the same or different MAC primitive).
 * \note
 *  This primitive may be called for the MAC Context that is still disabled.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.1.9.1, 7.1.9.2.
*****************************************************************************************/
MAC_PUBLIC void RF4CE_MAC_ResetReq(MAC_ResetReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Accepts MLME-RX-ENABLE.request for the RF4CE context of the MAC.
 * \param[in]   reqDescr    Pointer to the request descriptor object.
 * \note
 *  The descriptor pointed by \p reqDescr must reside in the global memory space and must
 *  be preserved by the caller until confirmation from the MAC is received. The \c service
 *  field of the request descriptor is used by the MAC during request processing. The
 *  caller shall set the \c callback field to the entry point of its
 *  MLME-RX-ENABLE.confirm handler-function.
 * \note
 *  For the case of dual-context MAC when both contexts are enabled, the MAC will switch
 *  off the radio hardware only if both contexts issued the MLME-RX-ENABLE.request to
 *  disable the radio, and if macRxOnWhenIdle attribute of both contexts are equal to
 *  FALSE, and if there are no requests to the MAC that use the radio hardware. If at
 *  least one of MAC Contexts demands the radio to stay enabled the radio hardware will be
 *  left switched on; still the off state of the radio hardware will be simulated by the
 *  MAC for the second MAC Context (that demanded the radio to be disabled), i.e. the
 *  corresponding MAC Context will reject all incoming frames and will not indicate them.
 * \note
 *  It is allowed to commence a new request to the MAC directly from the context of the
 *  confirmation handler. The same request descriptor object may be used for the new
 *  request to MAC just after it is freed with the confirmation on the previous request
 *  (of the same or different MAC primitive).
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.1.10.1, 7.1.10.2.
*****************************************************************************************/
MAC_PUBLIC void RF4CE_MAC_RxEnableReq(MAC_RxEnableReqDescr_t *const reqDescr);


#if defined(_MAC_CONTEXT_RF4CE_TARGET_)
/*************************************************************************************//**
 * \brief   Accepts MLME-SCAN.request for the RF4CE context of the MAC.
 * \param[in]   reqDescr    Pointer to the request descriptor object.
 * \note
 *  The descriptor pointed by \p reqDescr must reside in the global memory space and must
 *  be preserved by the caller until confirmation from the MAC is received. The \c service
 *  field of the request descriptor is used by the MAC during request processing. The
 *  caller shall set the \c callback field to the entry point of its MLME-SCAN.confirm
 *  handler-function.
 * \note
 *  The payload pointed by \p confParams->payload is allocated by the MAC in the dynamic
 *  memory and must be dismissed by the higher layer. The payload may be empty (i.e., its
 *  length may be equal to zero).
 * \note
 *  It is allowed to commence a new request to the MAC directly from the context of the
 *  confirmation handler. The same request descriptor object may be used for the new
 *  request to MAC just after it is freed with the confirmation on the previous request
 *  (of the same or different MAC primitive).
 * \note
 *  This primitive is not implemented for the RF4CE-Controller.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.1.11.1, 7.1.11.2.
*****************************************************************************************/
MAC_PUBLIC void RF4CE_MAC_ScanReq(MAC_ScanReqDescr_t *const reqDescr);
#endif /* _MAC_CONTEXT_RF4CE_TARGET_ */


/*************************************************************************************//**
 * \brief   Accepts MLME-SET.request for the RF4CE context of the MAC.
 * \param[in]   reqDescr    Pointer to the request descriptor object.
 * \note
 *  The descriptor pointed by \p reqDescr must reside in the global memory space and must
 *  be preserved by the caller until confirmation from the MAC is received. The \c service
 *  field of the request descriptor is used by the MAC during request processing. The
 *  caller shall set the \c callback field to the entry point of its MLME-SET.confirm
 *  handler-function.
 * \note
 *  The payload pointed by \p reqDescr->params.payload must be allocated by the caller in
 *  the dynamic or static memory and must be dismissed by it after the confirmation from
 *  the MAC is received. The payload may be empty (i.e., its length may be equal to zero).
 * \note
 *  It is allowed to commence a new request to the MAC directly from the context of the
 *  confirmation handler. The same request descriptor object may be used for the new
 *  request to MAC just after it is freed with the confirmation on the previous request
 *  (of the same or different MAC primitive).
 * \note
 *  This primitive may be called for the MAC Context that is still disabled.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.1.13.1, 7.1.13.2.
*****************************************************************************************/
MAC_PUBLIC void RF4CE_MAC_SetReq(MAC_SetReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Accepts MLME-START.request for the RF4CE context of the MAC.
 * \param[in]   reqDescr    Pointer to the request descriptor object.
 * \note
 *  The descriptor pointed by \p reqDescr must reside in the global memory space and must
 *  be preserved by the caller until confirmation from the MAC is received. The \c service
 *  field of the request descriptor is used by the MAC during request processing. The
 *  caller shall set the \c callback field to the entry point of its MLME-START.confirm
 *  handler-function.
 * \note
 *  It is allowed to commence a new request to the MAC directly from the context of the
 *  confirmation handler. The same request descriptor object may be used for the new
 *  request to MAC just after it is freed with the confirmation on the previous request
 *  (of the same or different MAC primitive).
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.1.14.1, 7.1.14.2.
*****************************************************************************************/
MAC_PUBLIC void RF4CE_MAC_StartReq(MAC_StartReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Returns pointer to the Descriptor of the PRNG (Pseudo-Random Number Generator)
 *  of the ZigBee RF4CE context.
 * \return  Pointer to the Descriptor of the PRNG structured object.
*****************************************************************************************/
#if defined(_HAL_USE_PRNG_)
MAC_PUBLIC HAL_PrngDescr_t* RF4CE_MAC_PrngDescr(void);
#else
#define RF4CE_MAC_PrngDescr()       NULL
#endif /* _ML507_OR_SOC_USES_PRNG_ */


#endif /* _BB_MAC_SAP_FOR_RF4CE_H */