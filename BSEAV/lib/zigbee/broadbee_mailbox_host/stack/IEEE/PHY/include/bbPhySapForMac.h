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
 *      PHY-SAP interface for the MAC layer.
 *
*******************************************************************************/

#ifndef _BB_PHY_SAP_FOR_MAC_H
#define _BB_PHY_SAP_FOR_MAC_H

/************************* INCLUDES ***********************************************************************************/
#include "bbPhySapDefs.h"
#include "bbPhySapTypesData.h"
#include "bbPhySapTypesState.h"
#include "bbPhySapTypesChannel.h"
#include "bbPhySapTypesEnergy.h"
#include "bbPhySapPib.h"

/************************* PROTOTYPES *********************************************************************************/
/**//**
 * \name    Aliases for Radio Driver interface to TX and RX Frame Buffers.
 */
/**{@*/
#define PHY_FrmBuf__TX_BDC_PHR      (HAL_Radio_FrmBuf__TX_BDC_PHR)
#define PHY_FrmBuf__TX_BDC_PSDU     (HAL_Radio_FrmBuf__TX_BDC_PSDU)
#define PHY_FrmBuf__TX_ACK_PSDU     (HAL_Radio_FrmBuf__TX_ACK_PSDU)
#define PHY_FrmBuf__RX_PPDU         (HAL_Radio_FrmBuf__RX_PPDU)
/**}@*/

/**//**
 * \name    Aliases for Radio Driver interface to Auxiliary Data of Frame Buffer.
 */
/**{@*/
#define PHY_FrmBuf__RX_Stuff        (HAL_Radio_FrmBuf__RX_Stuff)
#define PHY_FrmBuf__TX_Tstamps      (HAL_Radio_FrmBuf__TX_Tstamps)
#define PHY_FrmBuf__RX_Tstamps      (HAL_Radio_FrmBuf__RX_Tstamps)
#define PHY_FrmBuf__Status          (HAL_Radio_FrmBuf__Status)
/**}@*/

/**//**
 * \name    Aliases for Radio Driver interface to PHY Primitives.
 * \note    PHY's Confirmation and Indication callback functions are aliased only for conventional build.
 */
/**{@*/
#define PHY__DATA_req               (HAL_Radio__DATA_req)
#define PHY__DATA_ACK_req           (HAL_Radio__DATA_ACK_req)
#define PHY__STATE_req              (HAL_Radio__STATE_req)
#define PHY__CCA_req                (HAL_Radio__CCA_req)
#define PHY__ED_req                 (HAL_Radio__ED_req)
#define PHY__CHANNEL_req            (HAL_Radio__CHANNEL_req)
#define PHY__CCA_MODE_set           (HAL_Radio__CCA_MODE_set)
#define PHY__TX_POWER_set           (HAL_Radio__TX_POWER_set)
#define PHY__RSSI_get               (HAL_Radio__RSSI_get)
#define PHY__STATE_get              (HAL_Radio__STATE_get)
#define PHY__IS_BUSY_get            (HAL_Radio__IS_BUSY_get)
/**}@*/

/**//**
 * \brief Phy initialization routine
 */
void PHY__Init(void);

/**//**
 * \name    PHY's Confirmation and Indication callback functions for MAC in the case of Host-test build.
 */
/**@{*/
extern void PHY__DATA_conf(void);
extern void PHY__DATA_ind(void);
extern void PHY__STATE_conf(void);
extern void PHY__CCA_conf(void);
extern void PHY__ED_conf(void);
extern void PHY__CHANNEL_conf(void);
/**@}*/

#endif /* _BB_PHY_SAP_FOR_MAC_H */

/* eof bbPhySapForMac.h */