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
 *      Declaration of the ZigBee PRO APS Tx Rx component.
 *
*******************************************************************************/

#ifndef _ZBPRO_APS_TX_RX_H
#define _ZBPRO_APS_TX_RX_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbSysTimeoutTask.h"

#include "bbZbProNwkCommon.h"
#include "friend/bbZbProNwkNibApiFriend.h"

#include "bbZbProApsCommon.h"
#include "bbZbProApsKeywords.h"

/************************* FUNCTION PROTOTYPES ******************************************/

/* TODO: There should be more records in address map than the number of buffers on APS layer.
         It is needed to prevent errors during APS incoming packet handling, when Extended
         address may be required. */

/**//**
 * \brief Found an Extended address which matches the specified NWK address
 */
APS_PRIVATE bool zbProApsTxRxNwk2ExtAddr(ZBPRO_NWK_NwkAddr_t nwkAddr, ZBPRO_APS_ExtAddr_t *const extAddr);

/**//**
 * \brief Found an NWK address which matches the specified Extended address
 */
APS_PRIVATE bool zbProApsTxRxExt2NwkAddr(ZBPRO_APS_ExtAddr_t extAddr, ZBPRO_NWK_NwkAddr_t *const nwkAddr);

/************************************************************************************//**
  \brief Sets up the timeout timer

  \param[in] timeoutTask - pointer to a SYS_TimeoutTask_t element
  \param[in] delta - timer setting value
****************************************************************************************/
APS_PRIVATE void zbProApsTxRxSetTimer(SYS_TimeoutTask_t *timeoutTask, SYS_Time_t delta);

/**//**
 * \brief Calculates apscAckWaitDuration
 */
INLINE SYS_Time_t zbProApsTxTimeoutEval(void)
{
    ZBPRO_NWK_NibAttributeValue_t attr;

    /* see ZigBee Specification r20, 2.2.7.1 */
    zbProNwkNibApiGet(ZBPRO_NWK_NIB_MAX_DEPTH, &attr, NULL);
    return 50 * (2 * attr.nwkMaxDepth);
}

#endif /* _ZBPRO_APS_TX_RX_H */

/* eof bbZbProApsTxRx.h */