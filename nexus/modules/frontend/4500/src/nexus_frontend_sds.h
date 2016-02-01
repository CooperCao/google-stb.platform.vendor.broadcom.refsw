/***************************************************************************
*     (c)2004-2010 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: Frontend SDS
*    Routines to control BCM4500 SDS satellite frontend
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_FRONTEND_SDS_H__
#define NEXUS_FRONTEND_SDS_H__

#include "nexus_frontend_module.h"
#include "nexus_frontend_4500.h"
#include "bsds.h"

/***************************************************************************
Summary:
    SDS Devices
***************************************************************************/
typedef struct NEXUS_FrontendSdsDevices
{
    NEXUS_TunerHandle tuner;
} NEXUS_FrontendSdsDevices;

/***************************************************************************
Summary:
    SDS Channel Settings
***************************************************************************/
typedef struct NEXUS_FrontendSdsSettings
{
	NEXUS_4500Settings devSettings;
    BSDS_Handle sdsHandle;
    NEXUS_FrontendSdsDevices devices;
    unsigned boardRev;	/* reserved */
    void (*closeFunction)(NEXUS_FrontendHandle handle, void *pParam);   /* Called after handle is closed */
    void *pCloseParam;
} NEXUS_FrontendSdsSettings;

/***************************************************************************
Summary:
    Get Default SDS Channel Settings
***************************************************************************/
void NEXUS_Frontend_P_Sds_GetDefaultSettings(
    NEXUS_FrontendSdsSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
    Create an SDS frontend handle
***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_P_Sds_Create(
    const NEXUS_FrontendSdsSettings *pSettings
    );

/***************************************************************************
Summary:
    Set devices for an SDS channel.
Description:
    May be needed if devices are connected to an on-chip I2C master to
    avoid a chicken and egg problem connecting devices.
***************************************************************************/
NEXUS_Error NEXUS_Frontend_P_Sds_SetDevices(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendSdsDevices *pDevices
    );

BDBG_OBJECT_ID_DECLARE(NEXUS_SdsDevice);

/***************************************************************************
Summary:
Internal structure for SDS devices
***************************************************************************/
typedef struct NEXUS_SdsDevice
{
    BDBG_OBJECT(NEXUS_SdsDevice)

    NEXUS_FrontendSdsSettings settings;

    BSDS_Handle sdsHandle;
    NEXUS_FrontendHandle frontendHandle;

    NEXUS_FrontendSatelliteSettings lastSettings;   /* Last tune settings, returned in status */
    NEXUS_FrontendDiseqcSettings diseqcSettings;

	unsigned diseqcAddr;
#define B_SDS_DISEQC_REPLYLEN 64
	uint8_t diseqcReplybuffer[B_SDS_DISEQC_REPLYLEN];
	uint8_t diseqcReplylen;
	NEXUS_FrontendDiseqcMessageStatus diseqcReplyStatus;
    BKNI_EventHandle lockEvent;
    NEXUS_EventCallbackHandle lockEventCallback;
    NEXUS_TaskCallbackHandle lockAppCallback;

    NEXUS_TaskCallbackHandle diseqcAppCallback;

#ifdef NEXUS_FRONTEND_CUSTOMER_EXTENSION
    void *customerData;
#endif
} NEXUS_SdsDevice;

NEXUS_SdsDevice *NEXUS_Frontend_P_GetSdsDevice(
    NEXUS_FrontendHandle handle
    );

#endif /* #ifndef NEXUS_FRONTEND_SDS_H__ */

