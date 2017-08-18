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
*       APSME-UPDATE-DEVICE security service processor interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_APS_PROC_UPDATE_DEVICE_H
#define _BB_ZBPRO_APS_PROC_UPDATE_DEVICE_H


/************************* INCLUDES *****************************************************/
#include "bbZbProApsSapUpdateDevice.h"  /* APSME-UPDATE-DEVICE security service interface. */
#include "bbZbProApsHub.h"              /* ZigBee PRO APS Hub interface.                   */


/******************************* TYPES **************************************************/

/**//**
 * \brief APSME-UPDATE-DEVICE security service descriptor
 */
typedef struct _ZbProApsUpdateDeviceDescr_t
{
    SYS_QueueDescriptor_t  fillQueue;       /*!< To fill buffers queue.   */
} ZbProApsUpdateDeviceDescr_t;

/**//**
 * \brief APS_CMD_UPDATE command frame
 */
typedef struct PACKED _ZbProApsUpdateDeviceFrame_t
{
    ZbProApsCommandId_t             cmdId;
    ZBPRO_APS_ExtAddr_t             deviceAddress;
    ZBPRO_APS_ShortAddr_t           deviceShortAddress;
    ZBPRO_APS_UpdateDeviceStatus_t  status;
} ZbProApsUpdateDeviceFrame_t;

/************************* PROTOTYPES ***************************************************/

/*************************************************************************************//**
  \brief
    Fills specified transceiver buffers for APSME-UPDATE-DEVICE.request.
*****************************************************************************************/
APS_PRIVATE ZbProApsHubTxFillCb_t zbProApsProcUpdateDeviceFill;


/*************************************************************************************//**
  \brief
    Commences issuing APSME-UPDATE-DEVICE.confirm.
*****************************************************************************************/
APS_PRIVATE ZbProApsHubTxConfCb_t zbProApsProcUpdateDeviceConf;


/*************************************************************************************//**
  \brief
    Commences issuing APSME-UPDATE-DEVICE.indication.
*****************************************************************************************/
APS_PRIVATE ZbProApsHubRxIndCb_t zbProApsProcUpdateDeviceInd;


/*************************************************************************************//**
  \brief
    Resets APSME-UPDATE-DEVICE.request processor.
*****************************************************************************************/
APS_PRIVATE void zbProApsProcUpdateDeviceReset(void);


#endif /* _BB_ZBPRO_APS_PROC_UPDATE_DEVICE_H */

/* eof bbZbProApsProcUpdateDevice.h */