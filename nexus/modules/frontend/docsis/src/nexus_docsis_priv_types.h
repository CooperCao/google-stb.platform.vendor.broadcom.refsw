/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef NEXUS_DOCSIS_PRIV_TYPES_H__
#define NEXUS_DOCSIS_PRIV_TYPES_H__
#include "nexus_frontend.h"
#include "bdcm_device.h"
#include "bdcm_ads.h"
#include "bdcm_aob.h"
#include "bdcm_aus.h"
#include "bdcm_tnr.h"
#include "brpc.h"
#include "brpc_docsis.h"
#include "brpc_plat.h"
#include "nexus_tsmf.h"

#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(NEXUS_DocsisDevice);
BDBG_OBJECT_ID_DECLARE(NEXUS_DocsisChannel);

#define DOCSIS_DS_CHANNEL_INVALID 0xffffffff
#define DOCSIS_OPERATIONAL_RPC_TIMEOUT 3000 /* units ms */
typedef struct NEXUS_DocsisDevice
{
    BDBG_OBJECT(NEXUS_DocsisDevice)
    BDCM_DeviceHandle hDocsis;
    NEXUS_StandbyMode standbyMode;
    BDCM_Version version;
    NEXUS_ThreadHandle heartBeatThread;
    bool heartBeatEnabled;
    BKNI_EventHandle heartBeatEvent;
    bool notificationEnabled;
    unsigned notificationCount;
    NEXUS_ThreadHandle notificationThread;
    NEXUS_FrontendHandle hDsChannel[BDCM_MAX_ADS_CHANNELS];
    NEXUS_FrontendHandle hOutOfBandChannel;
    NEXUS_FrontendHandle hUpStreamChannel;
    unsigned numChannels;     /* numChannels = numDsChannels + 1 OOB + 1 US */
    unsigned numDsChannels;   /* numDsChannels = numQamChannels + numDataChannels */
    unsigned numDataChannels;
    unsigned numQamChannels;
    bool outOfBandChannelEnabled;
    bool upStreamEnabled;
    NEXUS_DocsisDeviceStatus status;
    NEXUS_TaskCallbackHandle statusCallback;
    unsigned numTsmfParsers;
    unsigned numUsedTsmfParsers;
    NEXUS_FrontendDevice *pGenericDeviceHandle;
    bool hasMxt;
    NEXUS_DocsisDeviceSettings settings;
    #ifdef NEXUS_FRONTEND_REVERSE_RMAGNUM_SUPPORT
    NEXUS_TunerHandle dataTuner;
    BKNI_EventHandle tunerStatusEvent;
    #endif
} NEXUS_DocsisDevice;

/***************************************************************************
Summary:
    An unique handle for a DOCSIS device.
***************************************************************************/
typedef struct NEXUS_DocsisDevice *NEXUS_DocsisDeviceHandle;


typedef struct NEXUS_DocsisChannel
{
    BDBG_OBJECT(NEXUS_DocsisChannel)
    NEXUS_DocsisDeviceHandle hDevice;
    int hostParserBand;     /* channel dev ID on Host */
    uint32_t dsChannelNum;    /*channel dev ID on DOCSIS */
    NEXUS_DocsisChannelType type;
    BDCM_AdsChannelHandle qam;
    BDCM_AobChannelHandle outOfBand;
    BDCM_AusChannelHandle upStream;
    BDCM_TnrHandle hTnr;
    int docsisTsmfParser;
    NEXUS_CallbackHandler lockDriverCBHandler;
    NEXUS_TaskCallbackHandle lockAppCallback;
    NEXUS_TaskCallbackHandle lockDriverCallback;
    NEXUS_TaskCallbackHandle asyncStatusAppCallback;
    NEXUS_TimerHandle retuneTimer;
    NEXUS_FrontendQamStatus qamStatus;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_FrontendOutOfBandSettings outOfBandSettings;
    NEXUS_FrontendOutOfBandStatus outOfBandStatus;
    NEXUS_FrontendUpstreamSettings upStreamSettings;
    NEXUS_FrontendUpstreamStatus upStreamStatus;
    bool tuneStarted;
    bool fastAcquire;
    bool autoAcquire;
    NEXUS_TsmfSettings tsmfSettings;
    NEXUS_FrontendConnectorHandle connector;
    NEXUS_DocsisOpenChannelSettings settings;
} NEXUS_DocsisChannel;

/***************************************************************************
Summary:
    An unqiue handle for a DOCSIS channel. This handle shall encapsulate
    host controlled DOCSIS device's channels like QAM, OOB, Data or UpStream channel,
    but not a DOCSIS data channel.
***************************************************************************/
typedef struct NEXUS_DocsisChannel *NEXUS_DocsisChannelHandle;


#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_DOCSIS_PRIV_TYPES_H__ */
