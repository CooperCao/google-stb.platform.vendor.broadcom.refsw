/***************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 * 
 * 
 ***************************************************************************/
#ifndef __NEXUS_SWFILTER_PRIV_H__
#define __NEXUS_SWFILTER_PRIV_H__

/***************************************************************************
Summary:
Callback, called when complete message is received.
***************************************************************************/
typedef void * (*NEXUS_SwFilter_Msg_P_callback_t)(void * context, size_t NEXUS_SwFilter_Msg_P_size);

struct NEXUS_SwFilter_FilterState;

/***************************************************************************
Summary:
Parameters for starting message capture.
***************************************************************************/
typedef struct NEXUS_SwFilter_MsgParams 
{
    uint16_t pid;
    NEXUS_MessageFilter filt;
    bool capture_ts;
    uint8_t * buffer;
    size_t buffer_size;
    bool disable_crc_check;
    NEXUS_SwFilter_Msg_P_callback_t callback; 
    void * context;
} NEXUS_SwFilter_MsgParams_t;

/***************************************************************************
Summary:
Initialize message filter.
***************************************************************************/

typedef struct parser_state_t *NEXUS_SwMsgFilterHandle;

NEXUS_SwMsgFilterHandle NEXUS_SwFilter_Msg_P_Init(void);
void NEXUS_SwFilter_Msg_P_Uninit(NEXUS_SwMsgFilterHandle handle);

/***************************************************************************
Summary:
Process buffer with transport packets. Buffer must be aligned on start of
transport packet.
***************************************************************************/
size_t NEXUS_SwFilter_Msg_P_Feed(NEXUS_SwMsgFilterHandle handle, uint8_t * buffer, size_t size);
size_t NEXUS_SwFilter_Msg_P_FeedPes(NEXUS_SwMsgFilterHandle handle, uint8_t * buffer, size_t size);
size_t NEXUS_SwFilter_Msg_P_FeedDss(NEXUS_SwMsgFilterHandle handle, uint8_t * buffer, size_t size);

/***************************************************************************
Summary:
Create and start filter.
***************************************************************************/
struct NEXUS_SwFilter_FilterState * NEXUS_SwFilter_Msg_P_SetFilter(NEXUS_SwMsgFilterHandle handle, NEXUS_SwFilter_MsgParams_t * params);
struct NEXUS_SwFilter_FilterState * NEXUS_SwFilter_Msg_P_SetPesFilter(NEXUS_SwMsgFilterHandle handle, NEXUS_SwFilter_MsgParams_t * params);
struct NEXUS_SwFilter_FilterState * NEXUS_SwFilter_Msg_P_SetDssFilter(NEXUS_SwMsgFilterHandle handle, NEXUS_SwFilter_MsgParams_t * params, NEXUS_DssMessageType dssMsgType , NEXUS_DssMessageMptFlags dssMptFlags );

/***************************************************************************
Summary:
Disable and remove filter.
***************************************************************************/
void NEXUS_SwFilter_Msg_P_RemoveFilter(NEXUS_SwMsgFilterHandle handle, struct NEXUS_SwFilter_FilterState *filter);
void NEXUS_SwFilter_Msg_P_RemovePesFilter(NEXUS_SwMsgFilterHandle handle, struct NEXUS_SwFilter_FilterState *filter);

#endif /*__NEXUS_SWFILTER_PRIV_H__*/
