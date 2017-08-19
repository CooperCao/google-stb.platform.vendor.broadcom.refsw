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
*       ZDP Dispatcher private interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZDP_DISPATCHER_H
#define _BB_ZBPRO_ZDP_DISPATCHER_H


/************************* INCLUDES *****************************************************/
#include "private/bbZbProZdpTransaction.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Data type for ZDP Transaction Sequence Number.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.4.2.8, figure 2.19.
 */
typedef uint8_t  ZbProZdpTransSeqNum_t;


/**//**
 * \brief   Size of ZDP Frame Header, in octets.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.4.2.8, figure 2.19.
 */
#define ZBPRO_ZDP_FRAME_HEADER_SIZE     (sizeof(ZbProZdpTransSeqNum_t))


/************************* PROTOTYPES ***************************************************/

/**//**
 * \brief   Initializes ZDP Dispatcher on startup.
 * \note
 *  Do not ever call this function during the ZDP Dispatcher work even if it is itself and
 *  all its linked Services are temporarily idle; use this function only during the
 *  startup sequence for memory initialization.
 */
void zbProZdpDispatcherInit(void);


/**//**
 * \brief   Accepts New Local Client ZDO Request for processing with ZDP Dispatcher.
 * \param[in]   zdoLocalRequest     Pointer to Local Client ZDO Request service field.
 */
void zbProZdpDispatcherAcceptNewLocalRequest(
                const ZbProZdoLocalRequest_t *const  zdoLocalRequest);


/**//**
 * \brief   Accepts COMPLETED signal from the ZDP Service being waited for completion.
 * \param[in]   zdpTransaction          Pointer to the service field of executed
 *  ZDP Transaction.
 * \param[in]   completedDirective      Numeric identifier of the Directive reported by
 *  the ZDP Service being completed.
 */
void zbProZdpDispatcherAcceptServiceCompleted(
                ZbProZdpTransaction_t *const  zdpTransaction,
                const RpcServiceDirective_t   completedDirective);


/**//**
 * \brief   Handles the APSDE-DATA.indication for the case of ZDP Profile and ZDP
 *  Destination Endpoint.
 * \param[in]   indParams       Pointer to APSDE-DATA.indication parameters structure.
 */
void zbProZdpDispatcherApsDataInd(
                ZBPRO_APS_DataIndParams_t *const  indParams);


#endif /* _BB_ZBPRO_ZDP_DISPATCHER_H */

/* eof bbZbProZdpDispatcher.h */