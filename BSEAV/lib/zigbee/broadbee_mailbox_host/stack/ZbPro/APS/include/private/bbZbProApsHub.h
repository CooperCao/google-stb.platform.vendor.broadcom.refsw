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
 *      Declaration of the ZigBee PRO APS Hub component.
 *
*******************************************************************************/

#ifndef _ZBPRO_APS_HUB_H
#define _ZBPRO_APS_HUB_H

/************************* INCLUDES ****************************************************/
#include "bbZbProApsCommon.h"
#include "bbZbProApsData.h"

#include "private/bbZbProApsPrivate.h"
#include "private/bbZbProApsTx.h"
#include "private/bbZbProApsRx.h"

/************************* TYPES *******************************************************/

/**//**
 * \brief Descriptor of Hub variables
 */
typedef struct _ZbProApsHubDesc_t
{
    BITMAP_DECLARE(activeRequestors, ZBPRO_APS_ID_AMOUNT);
} ZbProApsHubDesc_t;


/**//**
 * \brief Fill buffer callback function type definition
 * \param[in] buf - transceiver Tx buffer
 */
typedef bool ZbProApsHubTxFillCb_t(ZbProApsTxBuffer_t *const buf);

/**//**
 * \brief Tx confirmation callback function type definition
 * \param[in] status - status of transmission
 * \param[in] txTime - timestamp of the transmission
 *
 * \return true, if the payload from the APS frame has to be not freed
 */
typedef bool ZbProApsHubTxConfCb_t(ZbProApsTxBuffer_t *const buf,
        ZBPRO_APS_Status_t status, ZBPRO_APS_Timestamp_t txTime);

/**//**
 * \brief Rx Indication callback function type definition
 */
typedef bool ZbProApsHubRxIndCb_t(ZbProApsRxBuffer_t *buf);

/**//**
 * \brief Client descriptor type definition
 */
typedef struct _ZbProApsHubRequestor_t
{
    ZbProApsHubTxFillCb_t    *fillHandler;      /* Fill buffer handler */
    ZbProApsHubTxConfCb_t    *confHandler;      /* Tx confirmation handler */
    ZbProApsHubRxIndCb_t     *indHandler;       /* Indication handler */
    uint8_t                  commandSize;       /* size to allocate for a command */
} ZbProApsHubRequestor_t;

/**//**
 * \brief Tx request
 */
APS_PRIVATE void zbProApsHubTxReq(ZbProApsCommandId_t id);

/**//**
 * \brief  Hub Tx request handler function
 */
APS_PRIVATE void zbProApsHubTxReqHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/**//**
 * \brief Tx request confirm callback function
 */
APS_PRIVATE void zbProApsHubTxConfCb(ZbProApsTxBuffer_t *buf,
        ZBPRO_APS_Status_t status, ZBPRO_APS_Timestamp_t txTime);

/**//**
 * \brief Rx indication callback function
 */
APS_PRIVATE bool zbProApsHubRxIndCb(ZbProApsRxBuffer_t *buf);

/**//**
 * \brief Resets the Hub
 */
APS_PRIVATE void zbProApsHubReset(void);

#endif /* _ZBPRO_APS_HUB_H */

/* eof bbZbProApsHub.h */