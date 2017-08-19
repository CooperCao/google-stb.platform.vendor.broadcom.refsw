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
 *      MAC configuration file.
 *
*******************************************************************************/

#ifndef _BB_MAC_CONFIG_H
#define _BB_MAC_CONFIG_H


/******************************** MAC DOCUMENTATION STRUCTURE ***************************/
/**//**
 * \defgroup MAC (MAC API)
 @{
 * \defgroup Types (MAC Types)
 @{
 * \defgroup AssociateReq (Associate Request)
 * \defgroup AssociateConf (Associate Confirmation)
 * \defgroup AssociateInd (Associate Indication)
 * \defgroup AssociateResp (Associate Response)
 * \defgroup BeaconNotifyInd (Beacon Notify Indication)
 * \defgroup CommStatusInd (CommStatus Indication)
 * \defgroup DataReq (Data Request)
 * \defgroup DataConf (Data Confirmation)
 * \defgroup DataInd (Data Indication)
 * \defgroup GetReq (Get Request)
 * \defgroup GetConf (Get Confirmation)
 * \defgroup OrphanInd (Orphan Indication)
 * \defgroup OrphanResp (Orphan Response)
 * \defgroup PollReq (Poll Request)
 * \defgroup PollConf (Poll Confirmation)
 * \defgroup PollInd (Poll Indication)
 * \defgroup PurgeReq (Purge Request)
 * \defgroup PurgeConf (Purge Confirmation)
 * \defgroup ResetReq (Reset Request)
 * \defgroup ResetConf (Reset Confirmation)
 * \defgroup RxEnableReq (RxEnable Request)
 * \defgroup RxEnableConf (RxEnable Confirmation)
 * \defgroup ScanReq (Scan Request)
 * \defgroup ScanConf (Scan Confirmation)
 * \defgroup SetReq (Set Request)
 * \defgroup SetConf (Set Confirmation)
 * \defgroup StartReq (Start Request)
 * \defgroup StartConf (Start Confirmation)
 @}
 * \defgroup Functions (MAC Routines)
 @}
 */


/************************* CONFIGURATION ************************************************/
/*
 * Implement MAC context for ZigBee PRO.
 */
#undef   _MAC_CONTEXT_ZBPRO_
#
#if defined(_ZBPRO_)
# define _MAC_CONTEXT_ZBPRO_
#endif


/*
 * Implement MAC context for RF4CE.
 */
#undef   _MAC_CONTEXT_RF4CE_
#undef   _MAC_CONTEXT_RF4CE_TARGET_
#undef   _MAC_CONTEXT_RF4CE_CONTROLLER_
#
#if defined(_RF4CE_)
# define _MAC_CONTEXT_RF4CE_
#
# if defined(RF4CE_TARGET)
#  define _MAC_CONTEXT_RF4CE_TARGET_
# else
#  define _MAC_CONTEXT_RF4CE_CONTROLLER_
# endif
#
# if defined(MAILBOX_UNIT_TEST)
#  undef  _MAC_CONTEXT_RF4CE_TARGET_
#  define _MAC_CONTEXT_RF4CE_TARGET_
#  undef  _MAC_CONTEXT_RF4CE_CONTROLLER_
#  define _MAC_CONTEXT_RF4CE_CONTROLLER_
# endif
#endif


#endif /* _BB_MAC_CONFIG_H */

/* eof bbMacConfig.h */