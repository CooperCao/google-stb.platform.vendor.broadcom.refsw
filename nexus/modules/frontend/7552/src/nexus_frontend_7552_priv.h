/***************************************************************************
*     (c) 2011 Broadcom Corporation
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
* Module Description:
*       The frontend_7552 module defines the common functions in signal
*   demodulation process, including opening or closing a frontend_7552 device,
*   acquire a DVB-T or ISDB-T signal, getting the status of the acquired
*   channel, setting the inband transport output interface settings, detecting
*   certain type of signal, install signal lock callback, install signal
*   unlock callback, and etc.
*
* Revision History:
*
* $brcm_Log: $
* 
**************************************************************************/
#ifndef NEXUS_FRONTEND_7552_PRIV_H__
#define NEXUS_FRONTEND_7552_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nexus_frontend_module.h"
#include "bthd.h"
#include "7552/bthd_3x7x.h"
#include "bads.h"
#include "bads_priv.h"
#include "7552/bads_api.h"
#include "7552/bads_3x7x.h"
#include "btnr_3x7x.h"

BDBG_MODULE(nexus_frontend_7552);
BDBG_OBJECT_ID_DECLARE(NEXUS_7552);

static const unsigned J83A_SYMBOLRATE_UPPER = 7200000;
static const unsigned J83A_SYMBOLRATE_LOWER = 4000000;
static const unsigned J83A_SYMBOLRATE_DEFAULT = 6952000;
static const unsigned J83B_SYMBOLRATE_QAM64 = 5056941;
static const unsigned J83B_SYMBOLRATE_QAM256 = 5360537;
static const unsigned CHANNEL_SCAN_TIMEOUT_DEFAULT = 5000; /* 5 seconds */

typedef struct NEXUS_7552
{
    BDBG_OBJECT(NEXUS_7552)
    union
    {
        NEXUS_FrontendQamSettings qam;
        NEXUS_FrontendOfdmSettings ofdm;
        NEXUS_FrontendScanSettings scan;
    } lastSettings;
    enum
    {
        UNKNOWN_7552,
        QAM_7552,
        OFDM_7552,
        SCAN_7552
    } currentMode;
    BTNR_Handle tnrHandle;
    BTHD_Handle thdHandle;
    BADS_Handle adsHandle;
    BADS_ChannelHandle channelHandle; /* 7552 has only one channel */
    BKNI_EventHandle tnrIsrEvent;
    BKNI_EventHandle thdIsrEvent;
    BKNI_EventHandle adsIsrEvent;
    BKNI_EventHandle thdLockEvent;
    BKNI_EventHandle adsLockEvent;
    BKNI_EventHandle ewsEvent;
    BKNI_EventHandle bbsEvent;
    BKNI_EventHandle channelScanEvent;
    NEXUS_EventCallbackHandle tnrIsrEventCallback;
    NEXUS_EventCallbackHandle thdIsrEventCallback;
    NEXUS_EventCallbackHandle adsIsrEventCallback;
    NEXUS_EventCallbackHandle thdLockEventCallback;
    NEXUS_EventCallbackHandle adsLockEventCallback;
    NEXUS_EventCallbackHandle ewsEventCallback;
    NEXUS_EventCallbackHandle bbsEventCallback;
    NEXUS_TaskCallbackHandle thdLockAppCallback;
    NEXUS_IsrCallbackHandle  adsLockAppCallback;
    NEXUS_TaskCallbackHandle ewsAppCallback;
    NEXUS_7552OfdmFrontendSettings ofdmSettings;
    NEXUS_7552QamFrontendSettings  qamSettings;
    NEXUS_IsrCallbackHandle tunerIsrCallback;
    bool untuned;
    unsigned channelScanTimeout; /* in milliseconds */
    unsigned upperBaudSearch; /* in symbols per sec */
    unsigned lowerBaudSearch; /* in symbols per sec */
    unsigned frontendcount;
    NEXUS_7552TunerRfInput rfInput;
    bool enableRfLoopThrough;
} NEXUS_7552;

extern NEXUS_7552 *p_7552device;

NEXUS_Error NEXUS_Frontend_P_Open7552(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef 7552_H__ */
