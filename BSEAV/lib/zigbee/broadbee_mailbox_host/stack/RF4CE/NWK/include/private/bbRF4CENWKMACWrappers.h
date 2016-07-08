/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 ******************************************************************************
/*****************************************************************************
*
* FILENAME: $Workfile: $
*
* DESCRIPTION:
*   RF4CE NWK MAC requests wrapper functions.
*
* $Revision: $
* $Date: $
*
****************************************************************************************/
#ifndef _RF4CE_NWK_MAC_WRAPPERS_H
#define _RF4CE_NWK_MAC_WRAPPERS_H
/************************* INCLUDES ****************************************************/
#include "bbMacSapForRF4CE.h"

#ifdef RF4CE_ENABLE_MAC_STATS

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Wrapper for RF4CE_MAC_DataReq().
 ****************************************************************************************/
void rf4ceMAC_DataReq(MAC_DataReqDescr_t *const reqDescr);

/************************************************************************************//**
 \brief Wrapper for RF4CE_MAC_GetReq().
 ****************************************************************************************/
void rf4ceMAC_GetReq(MAC_GetReqDescr_t *const reqDescr);

/************************************************************************************//**
 \brief Wrapper for RF4CE_MAC_ResetReq().
 ****************************************************************************************/
void rf4ceMAC_ResetReq(MAC_ResetReqDescr_t *const reqDescr);

#ifdef RF4CE_TARGET
/************************************************************************************//**
 \brief Wrapper for RF4CE_MAC_ScanReq().
 ****************************************************************************************/
void rf4ceMAC_ScanReq(MAC_ScanReqDescr_t *const reqDescr);
#endif /* RF4CE_TARGET */

/************************************************************************************//**
 \brief Wrapper for RF4CE_MAC_SetReq().
 ****************************************************************************************/
void rf4ceMAC_SetReq(MAC_SetReqDescr_t *const reqDescr);

/************************************************************************************//**
 \brief Wrapper for RF4CE_MAC_StartReq().
 ****************************************************************************************/
void rf4ceMAC_StartReq(MAC_StartReqDescr_t *const reqDescr);

#else /* RF4CE_ENABLE_MAC_STATS */

#define rf4ceMAC_DataReq RF4CE_MAC_DataReq
#define rf4ceMAC_GetReq RF4CE_MAC_GetReq
#define rf4ceMAC_ResetReq RF4CE_MAC_ResetReq
#define rf4ceMAC_ScanReq RF4CE_MAC_ScanReq
#define rf4ceMAC_SetReq RF4CE_MAC_SetReq
#define rf4ceMAC_StartReq RF4CE_MAC_StartReq

#endif /* RF4CE_ENABLE_MAC_STATS */

#endif /* _RF4CE_NWK_MAC_WRAPPERS_H */
