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
*       Contains ZigBee PRO ZDO layer configuration.
*
*******************************************************************************/

#ifndef _ZBPRO_ZDO_CONFIG_H
#define _ZBPRO_ZDO_CONFIG_H

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief The total amount of buffers for zdp requests.
 */
#define ZBPRO_ZDO_MAX_ZDP_REQUEST_AMOUNT 10

/**//**
 * \brief The timeout before restart Intra-PAN Portability procedure.
 */
#define ZBPRO_ZDO_INTRA_PAN_PORT_TIMEOUT_MSEC 5000

/**//**
 * \brief The timeout before call nvm request to store attributes.
 */
#define ZBPRO_ZDO_PERSISTENT_STORE_DELAY_MS 1000

/**//**
 * \brief Minimal amount of reports to start channel change procedure.
 */
#define ZBPRO_ZDO_CHANNEL_CHANGE_REPORTS_AMOUNT 3

/**//**
 * \brief   Threshold for nwkTxTotal attribute to activate ZDO Frequency Agility.
 * \details
 *  For the Frequency Agility to be activated, the value of nwkTxTotal must be greater
 *  than this threshold.
 * \par     Documentation
 *  See ZigBee Document 053474r20, annex E.
 */
#define ZBPRO_ZDO_FREQUENCY_AGILITY_THRESHOLD_TX_TOTAL          20


/**//**
 * \brief   Threshold for Transmission Error Rate to trigger ZDO Frequency Agility, in
 *  percentage points.
 * \details
 *  For the Frequency Agility to be triggered, the percentage of total transmission
 *  failures over all the actual neighbors to the total number of unicast transactions
 *  shall be greater than this threshold.
 * \par     Documentation
 *  See ZigBee Document 053474r20, annex E.
 */
#define ZBPRO_ZDO_FREQUENCY_AGILITY_THRESHOLD_TX_ERROR_RATE     25

#endif /* _ZBPRO_ZDO_CONFIG_H */

/* eof bbZbProZdoConfig.h */