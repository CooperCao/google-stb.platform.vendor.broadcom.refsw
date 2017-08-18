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
 *      MAC-SAP interface for the ZigBee PRO stack.
 *
*******************************************************************************/

#ifndef _BB_MAC_SAP_FOR_ZBPRO_H
#define _BB_MAC_SAP_FOR_ZBPRO_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapTypesData.h"          /* MCPS-DATA service data types. */
#include "bbMacSapTypesPurge.h"         /* MCPS-PURGE service data types. */
#include "bbMacSapTypesAssociate.h"     /* MLME-ASSOCIATE service data types. */
#include "bbMacSapTypesBeacon.h"        /* MLME-BEACON service data types. */
#include "bbMacSapTypesCommStatus.h"    /* MLME-COMM-STATUS service data types. */
#include "bbMacSapTypesGet.h"           /* MLME-GET service data types. */
#include "bbMacSapTypesOrphan.h"        /* MLME-ORPHAN service data types. */
#include "bbMacSapTypesPoll.h"          /* MLME-POLL service data types. */
#include "bbMacSapTypesReset.h"         /* MLME-RESET service data types. */
#include "bbMacSapTypesRxEnable.h"      /* MLME-RX-ENABLE service data types. */
#include "bbMacSapTypesScan.h"          /* MLME-SCAN service data types. */
#include "bbMacSapTypesSet.h"           /* MLME-SET service data types. */
#include "bbMacSapTypesStart.h"         /* MLME-START service data types. */
#include "bbHalRandom.h"                /* Random Number Generator Hardware interface. */


/************************* VALIDATIONS **************************************************/
#if !defined(_MAC_CONTEXT_ZBPRO_)
# error This header shall be compiled only if the ZigBee PRO context is included into the build.
#endif


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Accepts MCPS-DATA.request for the ZigBee PRO context of the MAC.
 * \ingroup Functions
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
MAC_PUBLIC void ZBPRO_MAC_DataReq(MAC_DataReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Issues MCPS-DATA.indication to the ZigBee PRO Stack from the MAC.
 * \ingroup Functions
 * \param[in]   indParams   Pointer to the indication parameters object.
 * \details
 *  ZigBee PRO shall provide MCPS-DATA.indication handler-function according to the
 *  template:
 * \code
 *  void ZBPRO_MAC_DataInd(MAC_DataIndParams_t *const indParams) { ... }
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
MAC_PUBLIC MAC_DataIndCallback_t ZBPRO_MAC_DataInd;


/*************************************************************************************//**
 * \brief   Accepts MCPS-PURGE.request for the ZigBee PRO context of the MAC.
 * \ingroup Functions
 * \param[in]   reqDescr    Pointer to the request descriptor object.
 * \note
 *  The descriptor pointed by \p reqDescr must reside in the global memory space and must
 *  be preserved by the caller until confirmation from the MAC is received. The \c service
 *  field of the request descriptor is used by the MAC during request processing. The
 *  caller shall set the \c callback field to the entry point of its MCPS-PURGE.confirm
 *  handler-function.
 * \note
 *  It is allowed to commence a new request to the MAC directly from the context of the
 *  confirmation handler. The same request descriptor object may be used for the new
 *  request to MAC just after it is freed with the confirmation on the previous request
 *  (of the same or different MAC primitive).
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.1.1.4, 7.1.1.5.
*****************************************************************************************/
MAC_PUBLIC void ZBPRO_MAC_PurgeReq(MAC_PurgeReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Accepts MLME-ASSOCIATE.request for the ZigBee PRO context of the MAC.
 * \ingroup Functions
 * \param[in]   reqDescr    Pointer to the request descriptor object.
 * \note
 *  The descriptor pointed by \p reqDescr must reside in the global memory space and must
 *  be preserved by the caller until confirmation from the MAC is received. The \c service
 *  field of the request descriptor is used by the MAC during request processing. The
 *  caller shall set the \c callback field to the entry point of its
 *  MLME-ASSOCIATE.confirm handler-function.
 * \note
 *  It is allowed to commence a new request to the MAC directly from the context of the
 *  confirmation handler. The same request descriptor object may be used for the new
 *  request to MAC just after it is freed with the confirmation on the previous request
 *  (of the same or different MAC primitive).
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.1.3.1, 7.1.3.4.
*****************************************************************************************/
MAC_PUBLIC void ZBPRO_MAC_AssociateReq(MAC_AssociateReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Issues MLME-ASSOCIATE.indication to the ZigBee PRO Stack from the MAC.
 * \ingroup Functions
 * \param[in]   indParams   Pointer to the indication parameters object.
 * \details
 *  ZigBee PRO shall provide MLME-ASSOCIATE.indication handler-function according to the
 *  template:
 * \code
 *  void ZBPRO_MAC_AssociateInd(MAC_AssociateIndParams_t *const indParams) { ... }
 * \endcode
 * \note
 *  Treat the parameters structure pointed by the \p indParams and passed into the
 *  indication callback handler-function as it has been allocated in the program stack by
 *  the MAC before calling this callback handler and will be destroyed just after this
 *  callback returns.
 * \note
 *  It is allowed to commence a new request to the MAC directly from the context of this
 *  indication handler.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.1.3.2.
*****************************************************************************************/
MAC_PUBLIC MAC_AssociateIndCallback_t ZBPRO_MAC_AssociateInd;


/*************************************************************************************//**
 * \brief   Accepts MLME-ASSOCIATE.response for the ZigBee PRO context of the MAC.
 * \ingroup Functions
 * \param[in]   respDescr   Pointer to the response descriptor object.
 * \note
 *  The descriptor pointed by \p respDescr must reside in the global memory space and must
 *  be preserved by the caller until confirmation from the MAC is received. The \c service
 *  field of the response descriptor is used by the MAC during response processing. The
 *  caller shall set the \c callback field to the entry point of its
 *  MLME-COMM-STATUS-ASSOCIATE.indication handler-function that plays the role of
 *  confirmation handler for the MLME-ASSOCIATE.response primitive.
 * \note
 *  It is allowed to commence a new request to the MAC directly from the context of the
 *  confirmation handler. The same request descriptor object may be used for the new
 *  request to MAC just after it is freed with the confirmation on the previous request
 *  (of the same or different MAC primitive).
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.1.3.3, 7.1.12.1.
*****************************************************************************************/
MAC_PUBLIC void ZBPRO_MAC_AssociateResp(MAC_AssociateRespDescr_t *const respDescr);


#if defined(_MAC_TESTER_)
/*************************************************************************************//**
 * \brief   Accepts MLME-BEACON.response for the ZigBee PRO context of the MAC.
 * \ingroup Functions
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
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.5.2.4.
*****************************************************************************************/
MAC_PUBLIC void ZBPRO_MAC_BeaconResp(MAC_BeaconRespDescr_t *const respDescr);
#endif /* _MAC_TESTER_ */


/*************************************************************************************//**
 * \brief   Issues MLME-BEACON-NOTIFY.indication to the ZigBee PRO Stack from the MAC.
 * \ingroup Functions
 * \param[in]   indParams   Pointer to the indication parameters object.
 * \details
 *  ZigBee PRO shall provide MLME-BEACON-NOTIFY.indication handler-function according to
 *  the template:
 * \code
 *  void ZBPRO_MAC_BeaconNotifyInd(MAC_BeaconNotifyIndParams_t *const indParams) { ... }
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
 *  See IEEE Std 802.15.4-2006, subclause 7.1.5.1.
*****************************************************************************************/
MAC_PUBLIC MAC_BeaconNotifyIndCallback_t ZBPRO_MAC_BeaconNotifyInd;


/*************************************************************************************//**
 * \brief   Issues MLME-COMM-STATUS.indication to the ZigBee PRO Stack from the MAC.
 * \ingroup Functions
 * \param[in]   indParams   Pointer to the indication parameters object.
 * \details
 *  ZigBee PRO shall provide MLME-COMM-STATUS.indication handler-function according to
 *  the template:
 * \code
 *  void ZBPRO_MAC_CommStatusInd(MAC_CommStatusIndParams_t *const indParams) { ... }
 * \endcode
 * \details
 *  The MAC issues the MLME-COMM-STATUS.indication only for the case when a MAC secured
 *  frame is received. The MAC issues MLME-COMM-STATUS-ASSOCIATE.indication and
 *  MLME-COMM-STATUS-ORPHAN.indication instead of MLME-COMM-STATUS.indication following
 *  just processed MLME-ASSOCIATE.response and MLME-ORPHAN.response respectively; both of
 *  these dedicated indications are issued as confirmations on responses unlike the true
 *  MLME-COMM-STATUS.indication that is issued as an indication.
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
MAC_PUBLIC MAC_CommStatusIndCallback_t ZBPRO_MAC_CommStatusInd;


/*************************************************************************************//**
 * \brief   Accepts MLME-GET.request for the ZigBee PRO context of the MAC.
 * \ingroup Functions
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
MAC_PUBLIC void ZBPRO_MAC_GetReq(MAC_GetReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Issues MLME-ORPHAN.indication to the ZigBee PRO Stack from the MAC.
 * \ingroup Functions
 * \param[in]   indParams   Pointer to the indication parameters object.
 * \details
 *  ZigBee PRO shall provide MLME-ORPHAN.indication handler-function according to the
 *  template:
 * \code
 *  void ZBPRO_MAC_OrphanInd(MAC_OrphanIndParams_t *const indParams) { ... }
 * \endcode
 * \note
 *  Treat the parameters structure pointed by the \p indParams and passed into the
 *  indication callback handler-function as it has been allocated in the program stack by
 *  the MAC before calling this callback handler and will be destroyed just after this
 *  callback returns.
 * \note
 *  It is allowed to commence a new request to the MAC directly from the context of this
 *  indication handler.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.1.8.1.
*****************************************************************************************/
MAC_PUBLIC MAC_OrphanIndCallback_t ZBPRO_MAC_OrphanInd;


/*************************************************************************************//**
 * \brief   Accepts MLME-ORPHAN.response for the ZigBee PRO context of the MAC.
 * \ingroup Functions
 * \param[in]   respDescr   Pointer to the response descriptor object.
 * \note
 *  The descriptor pointed by \p respDescr must reside in the global memory space and must
 *  be preserved by the caller until confirmation from the MAC is received. The \c service
 *  field of the response descriptor is used by the MAC during response processing. The
 *  caller shall set the \c callback field to the entry point of its
 *  MLME-COMM-STATUS-ORPHAN.indication handler-function that plays the role of
 *  confirmation handler for the MLME-ORPHAN.response primitive.
 * \note
 *  It is allowed to commence a new request to the MAC directly from the context of the
 *  confirmation handler. The same request descriptor object may be used for the new
 *  request to MAC just after it is freed with the confirmation on the previous request
 *  (of the same or different MAC primitive).
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.1.8.2.
*****************************************************************************************/
MAC_PUBLIC void ZBPRO_MAC_OrphanResp(MAC_OrphanRespDescr_t *const respDescr);


/*************************************************************************************//**
 * \brief   Accepts MLME-POLL.request for the ZigBee PRO context of the MAC.
 * \ingroup Functions
 * \param[in]   reqDescr    Pointer to the request descriptor object.
 * \note
 *  The descriptor pointed by \p reqDescr must reside in the global memory space and must
 *  be preserved by the caller until confirmation from the MAC is received. The \c service
 *  field of the request descriptor is used by the MAC during request processing. The
 *  caller shall set the \c callback field to the entry point of its
 *  MLME-POLL.confirm handler-function.
 * \note
 *  It is allowed to commence a new request to the MAC directly from the context of the
 *  confirmation handler. The same request descriptor object may be used for the new
 *  request to MAC just after it is freed with the confirmation on the previous request
 *  (of the same or different MAC primitive).
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.1.16.1, 7.1.16.2.
*****************************************************************************************/
MAC_PUBLIC void ZBPRO_MAC_PollReq(MAC_PollReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Issues MLME-POLL.indication to the ZigBee PRO Stack from the MAC.
 * \ingroup Functions
 * \param[in]   indParams   Pointer to the indication parameters object.
 * \details
 *  ZigBee PRO shall provide MLME-POLL.indication handler-function according to the
 *  template:
 * \code
 *  void ZBPRO_MAC_PollInd(MAC_PollIndParams_t *const indParams) { ... }
 * \endcode
 * \details
 *  This nonstandard primitive is used by the MAC to indicate reception of Data Request
 *  MAC Command frame from the media in the case when there is no pending data for that
 *  device in this device transactions queue. It is helpful for the ZigBee PRO NWK layer
 *  of coordinator or router, because by means of this primitive it is possible to see the
 *  activity (and presence in the POS) of an associated device, especially when it is in
 *  the Rx-Off-When-Idle mode (the sleeping end-device).
 * \note
 *  Treat the parameters structure pointed by the \p indParams and passed into the
 *  indication callback handler-function as it has been allocated in the program stack by
 *  the MAC before calling this callback handler and will be destroyed just after this
 *  callback returns.
 * \note
 *  It is allowed to commence a new request to the MAC directly from the context of this
 *  indication handler.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.5.5.
*****************************************************************************************/
MAC_PUBLIC MAC_PollIndCallback_t ZBPRO_MAC_PollInd;


/*************************************************************************************//**
 * \brief   Accepts MLME-RESET.request for the ZigBee PRO context of the MAC.
 * \ingroup Functions
 * \param[in]   reqDescr    Pointer to the request descriptor object.
 * \note
 *  The descriptor pointed by \p reqDescr must reside in the global memory space and must
 *  be preserved by the caller until confirmation from the MAC is received. The \c service
 *  field of the request descriptor is used by the MAC during request processing. The
 *  caller shall set the \c callback field to the entry point of its MLME-RESET.confirm
 *  handler-function.
 * \note
 *  This primitive resets only the ZigBee PRO context of the MAC, it does not interfere
 *  with the RF4CE context of the MAC.
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
MAC_PUBLIC void ZBPRO_MAC_ResetReq(MAC_ResetReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Accepts MLME-RX-ENABLE.request for the ZigBee PRO context of the MAC.
 * \ingroup Functions
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
MAC_PUBLIC void ZBPRO_MAC_RxEnableReq(MAC_RxEnableReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Accepts MLME-SCAN.request for the ZigBee PRO context of the MAC.
 * \ingroup Functions
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
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.1.11.1, 7.1.11.2.
*****************************************************************************************/
MAC_PUBLIC void ZBPRO_MAC_ScanReq(MAC_ScanReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Accepts MLME-SET.request for the ZigBee PRO context of the MAC.
 * \ingroup Functions
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
MAC_PUBLIC void ZBPRO_MAC_SetReq(MAC_SetReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Accepts MLME-START.request for the ZigBee PRO context of the MAC.
 * \ingroup Functions
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
MAC_PUBLIC void ZBPRO_MAC_StartReq(MAC_StartReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Returns pointer to the Descriptor of the PRNG (Pseudo-Random Number Generator)
 *  of the ZigBee PRO context.
 * \return  Pointer to the Descriptor of the PRNG structured object.
*****************************************************************************************/
#if defined(_HAL_USE_PRNG_)
MAC_PUBLIC HAL_PrngDescr_t* ZBPRO_MAC_PrngDescr(void);
#else
#define ZBPRO_MAC_PrngDescr()       NULL
#endif


#endif /* _BB_MAC_SAP_FOR_ZBPRO_H */

/* eof bbMacSapForZBPRO.h */