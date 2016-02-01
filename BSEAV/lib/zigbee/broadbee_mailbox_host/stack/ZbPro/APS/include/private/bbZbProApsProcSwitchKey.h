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
* FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/private/bbZbProApsProcSwitchKey.h $
*
* DESCRIPTION:
*   APSME-SWITCH-KEY security service processor interface.
*
* $Revision: 2491 $
* $Date: 2014-05-22 11:01:34Z $
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_APS_PROC_SWITCH_KEY_H
#define _BB_ZBPRO_APS_PROC_SWITCH_KEY_H


/************************* INCLUDES *****************************************************/
#include "bbZbProSsp.h"
#include "bbZbProApsSapSwitchKey.h"     /* APSME-SWITCH-KEY security service interface. */
#include "bbZbProApsHub.h"              /* ZigBee PRO APS Hub interface.                */


/************************* DEFINITIONS **************************************************/

/**//**
 * \brief APSME-SWITCH-KEY security service descriptor
 */
typedef struct _ZbProApsSwitchKeyDescr_t
{
    SYS_QueueDescriptor_t  fillQueue;       /*!< To fill buffers queue.   */
} ZbProApsSwitchKeyDescr_t;

/**//**
 * \brief APS_CMD_SWITCH_KEY command frame
 */
typedef struct PACKED _ZbProApsSwitchKeyFrame_t
{
    ZbProApsCommandId_t             cmdId;
    ZbProSspNwkKeySeqNum_t          keySeqNum;
} ZbProApsSwitchKeyFrame_t;


/************************* PROTOTYPES ***************************************************/

/*************************************************************************************//**
  \brief
    Fills specified transceiver buffers for APSME-SWITCH-KEY.request.
*****************************************************************************************/
APS_PRIVATE ZbProApsHubTxFillCb_t zbProApsProcSwitchKeyFill;


/*************************************************************************************//**
  \brief
    Commences issuing APSME-SWITCH-KEY.confirm.
*****************************************************************************************/
APS_PRIVATE ZbProApsHubTxConfCb_t zbProApsProcSwitchKeyConf;


/*************************************************************************************//**
  \brief
    Commences issuing APSME-SWITCH-KEY.indication.
*****************************************************************************************/
APS_PRIVATE ZbProApsHubRxIndCb_t zbProApsProcSwitchKeyInd;


/*************************************************************************************//**
  \brief
    Resets APSME-SWITCH-KEY.request processor.
*****************************************************************************************/
APS_PRIVATE void zbProApsProcSwitchKeyReset(void);


#endif /* _BB_ZBPRO_APS_PROC_SWITCH_KEY_H */