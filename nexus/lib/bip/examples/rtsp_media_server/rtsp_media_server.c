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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "bstd.h"
#include "bkni.h"
#include "blst_list.h"
#include "blst_queue.h"

#include "bip.h"
#include "bip_rtsp_server.h"
#if NXCLIENT_SUPPORT
#include "nxclient.h"
#include "nexus_platform_client.h"
#else
#include "nexus_platform.h"
#endif
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_core_utils.h"

#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BDBG_MODULE( rtsp_media_server );
BDBG_FILE_MODULE( rtsp_media_server_list );
#define PRINTMSG_LIST( bdbg_args ) BDBG_MODULE_MSG( rtsp_media_server_list, bdbg_args );
BDBG_FILE_MODULE( rtsp_media_server_params );
#define PRINTMSG_PARAMS( bdbg_args ) BDBG_MODULE_MSG( rtsp_media_server_params, bdbg_args );


#define USE_HARDCODED_PARAMS 0
#define USE_DVB              1                    /* if set to 0 or undefined, use DSS */
#define INTERFACENAME        "eth0"

#if USE_HARDCODED_PARAMS
/* Settings works with Alitronika in San Diego lab */
/* the following define the input and its characteristics -- these will vary by input type */
#if USE_DVB
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_PID      257 /*0x31 */
#define AUDIO_PID      260 /*0x34*/
#define FREQ           1119000000
#define SATELLITE_MODE NEXUS_FrontendSatelliteMode_eDvb
#define TONE_ENABLED   true
#define TONE_MODE      NEXUS_FrontendDiseqcToneMode_eEnvelope
#define VOLTAGE        NEXUS_FrontendDiseqcVoltage_e13v
#else /* if USE_DVB */
#define TRANSPORT_TYPE NEXUS_TransportType_eDssEs
#define VIDEO_PID      0x78
#define AUDIO_PID      0x79
#define FREQ           1396820000
#define SATELLITE_MODE NEXUS_FrontendSatelliteMode_eDss
#define TONE_ENABLED   false
#define TONE_MODE      NEXUS_FrontendDiseqcToneMode_eTone
#define VOLTAGE        NEXUS_FrontendDiseqcVoltage_e18v
#endif /* if USE_DVB */
#define PAT_PID 0
#define PMT_PID 256

#else   /* USE_HARDCODED_PARAMS else */
/* will go away once we have media probe here */
#if USE_DVB
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
/*diseqc settings */
#define TONE_ENABLED true
#define TONE_MODE    NEXUS_FrontendDiseqcToneMode_eEnvelope
#define VOLTAGE      NEXUS_FrontendDiseqcVoltage_e13v
#else /* if USE_DVB */
#define TRANSPORT_TYPE NEXUS_TransportType_eDssEs
/*diseqc settings */
#define TONE_ENABLED false
#define TONE_MODE    NEXUS_FrontendDiseqcToneMode_eTone
#define VOLTAGE      NEXUS_FrontendDiseqcVoltage_e18v
#endif /* if USE_DVB */

#define PAT_PID   0
#define PMT_PID   256
#define VIDEO_PID 257 /*0x31 */
#define AUDIO_PID 260 /*0x34*/

#endif /* USE_HARDCODED_PARAMS end */
#if NEXUS_HAS_FRONTEND
#if 0
/*Sample code for Multiswitch code */
/* Diseqc constants define */
#define DISEQ_RESET                     0x00
#define DISEQ_NO_REPLY                  0xE0
#define DISEQ_REPLY                 0xE2
#define DISEQ_GET_HARDWARE_ID               0x54
#define DISEQ_SWITCH_ADDRESS                0x15
#define DISEQ_SWITCH_ADDRESS_TUNER1     0x17
#define DISEQ_SWITCH_ADDRESS_TUNER2     0x18
#define DISEQ_SWITCH_ADDRESS_TUNER3     0x19
#define DISEQ_GET_SWITCHES              0x14
#define DISEQ_WRITE_SWITCHES                0x38

typedef struct tuner_t
{
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_FrontendHandle frontend;
    NEXUS_CallbackDesc send_disecq_msg_cb_desc;
    BKNI_EventHandle diseqcCallbackEvent;
    BKNI_EventHandle lockCallbackEvent;
    NEXUS_FrontendSatelliteSettings satSettings;
    NEXUS_FrontendDiseqcStatus disqecStatus;
    NEXUS_FrontendAcquireSettings acquireSettings;
} tuner_t;

tuner_t sdsCtrl[NEXUS_MAX_FRONTENDS];
bool has_splitter = false;
#endif
bool diseqcSupport = false;


typedef struct AppRtspSessionCtx
{
    NEXUS_ParserBand                parserBand;
    BIP_RtspSessionHandle           hRtspSession;
    NEXUS_RecpumpHandle             recpumpHandle;
    NEXUS_FrontendHandle            frontend;
    NEXUS_FrontendDiseqcSettings    diseqcSettings;
    NEXUS_FrontendSatelliteSettings satSettings;
    BIP_AdditionalSatelliteSettings addSatSettings;
    BIP_PidInfo             pidInfo;
    NEXUS_PidChannelHandle *pidChannelList;
    BIP_RtspRequestHandle   hRtspRequest;
    BIP_RtspResponseHandle  hRtspResponse;
    BLST_Q_ENTRY( AppRtspSessionCtx ) rtspSessionListNext;    /* list of current rtspSessions */
} AppRtspSessionCtx;

typedef struct AppCtx
{
    bool rtspListenerStarted;
    BIP_RtspListenerHandle hRtspListener;
    BIP_IgmpListenerHandle hIgmpListener;
    BIP_RtspRequestHandle  hRtspRequest;
    BLST_Q_HEAD( rtspSessionListHead, AppRtspSessionCtx ) rtspSessionListHead; /* list of sessions on this listener */

#define MAX_TRACKED_EVENTS 3 /* for RtspListener, RtspSession, & IgmpEvents */
    B_EventGroupHandle hRtspEventGroup;
    B_EventHandle      hRtspListenerEvents;
    B_EventHandle      hRtspSessionEvents;
    B_EventHandle      hRtspSessionIgmpMembershipReportEvents;
    int                maxTriggeredEvents;
} AppCtx;

static void rtspListenerDestroySession( AppRtspSessionCtx *appRtspSession );

static void rtspSessionReportLockStatus(
    BIP_RtspSessionHandle hRtspSession,
    bool                  bLockStatus
    )
{
    BIP_Status rc;

    BDBG_MSG(( "%s: bLockStatus %d", BSTD_FUNCTION, bLockStatus ));

    rc = BIP_RtspSession_ReportLockStatus( hRtspSession, bLockStatus );
    BIP_CHECK_GOTO(( !rc ), ( "BIP_RtspSession_ReportLockStatus Failed" ), error, BIP_ERR_INTERNAL, rc );

error:
    return;
} /* rtspSessionReportLockStatus */

static void lock_callback(
    void *context,
    int   param
    )
{
    AppRtspSessionCtx            *appRtspSession = (AppRtspSessionCtx *)context;
    NEXUS_FrontendHandle          frontend       = NULL;
    NEXUS_FrontendSatelliteStatus status;
    NEXUS_FrontendDiseqcStatus    disqecStatus;

    BSTD_UNUSED( param );

    BDBG_MSG(( "%s: appRtspSession (%p) ", BSTD_FUNCTION, (void *)appRtspSession ));
    BIP_CHECK_PTR_GOTO( appRtspSession, "appRtspSession handle invalid", error, BIP_ERR_INVALID_PARAMETER );

    frontend = appRtspSession->frontend;
    BDBG_MSG(( "%s: Frontend(%p) ", BSTD_FUNCTION, (void *)frontend ));
    BIP_CHECK_PTR_GOTO( frontend, "Frontend handle invalid", error, BIP_ERR_INVALID_PARAMETER );

    NEXUS_Frontend_GetSatelliteStatus( frontend, &status );
    BDBG_MSG(( "%s: demod LOCKED = %d", BSTD_FUNCTION, status.demodLocked ));
    if (diseqcSupport)
    {
        NEXUS_Frontend_GetDiseqcStatus( frontend, &disqecStatus );
        BDBG_MSG(( "%s: diseqc tone = %d, voltage = %d\n", BSTD_FUNCTION, disqecStatus.toneEnabled, disqecStatus.voltage ));
    }

    rtspSessionReportLockStatus( appRtspSession->hRtspSession, status.demodLocked );

    /* TODO: lock_callback doesn't have much meaning for SAT>IP Server as if client tries to SETUP & PLAY a channel and */
    /* for whatever reason (e.g. bad freq) it can't be played, then Server is supposed to send out empty RTP packets to clients! */

error:
    return;
} /* lock_callback */

#if 0
/*Example code for Mutlitswtich control through diseqc. Not tested*/
static void get_diseqc_reply(void *context, int param)
{
    uint8_t rcvBuf[8];
    size_t replyLen, i;
    int ch=0;
    NEXUS_FrontendDiseqcMessageStatus status;

    BSTD_UNUSED(context);

    ch=param;
    NEXUS_Frontend_ResetStatus(sdsCtrl[ch].frontend);

    NEXUS_Frontend_GetDiseqcReply(sdsCtrl[ch].frontend, &status, &rcvBuf[0], sizeof(rcvBuf), &replyLen);
    if (status == NEXUS_FrontendDiseqcMessageStatus_eSuccess)
    {
        printf("Diseqc reply length=%d,ch=%d\n",replyLen,ch);
        if (replyLen > 8)
        {
            printf("replyLen out of range!\n");
            return;
        }
        for (i=0; i<replyLen; i++)
        {
            printf("reply: 0x%02x\n", *(rcvBuf+i));
        }
        BKNI_SetEvent(sdsCtrl[ch].diseqcCallbackEvent);

    }
    else
    {
        printf("get_diseqc_reply, status=%d", status);
    }

    NEXUS_Frontend_ResetStatus(sdsCtrl[ch].frontend);

    NEXUS_Frontend_GetDiseqcStatus(sdsCtrl[ch].frontend, &sdsCtrl[ch].disqecStatus);
    fprintf(stderr, "  diseqc tone = %d, voltage = %d \n", sdsCtrl[ch].disqecStatus.toneEnabled, sdsCtrl[ch].disqecStatus.voltage);

}
#endif
static bool gGotSigInt = 0;
static bool gExitThread = 0;

void signalHandler(
    int signal
    )
{
    BDBG_LOG(( "Got SIGINT (%d): Cleaning up!!!", signal ));
    gGotSigInt = 1;
}
/* returns 1 if there is user input*/
int userInputCheck( void )
{
    int rc;
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); /*STDIN_FILENO is 0*/
    rc= select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    if ( rc < 0 )
    {
        if (errno == EAGAIN || errno == EINTR)
        {  printf("continue %d",rc);  }
        else
        {    perror("ERROR: select(): returning error...");}
        return -1;
    }
    else if ( rc == 0 )
    {
        /* timeout: continue */

    }

    return FD_ISSET(STDIN_FILENO, &fds);
}

/* nexus functions */
NEXUS_Error initNexus(
    AppCtx *appCtx
    )
{

    BIP_Status bipStatus;
    NEXUS_Error nrc = NEXUS_SUCCESS;
    BSTD_UNUSED( appCtx );

#if NXCLIENT_SUPPORT
        {
            NxClient_JoinSettings joinSettings;

            NxClient_GetDefaultJoinSettings(&joinSettings);
            snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "rtsp_media_server");
            nrc = NxClient_Join(&joinSettings);
            BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NxClient_Join Failed" ), error, BIP_ERR_INTERNAL, bipStatus );


            BDBG_ERR(( "%s: Done with nx join ", BSTD_FUNCTION));
            BKNI_Sleep(5000);
        }
#else /* !NXCLIENT_SUPPORT */
        {
            NEXUS_PlatformSettings platformSettings;

            NEXUS_Platform_GetDefaultSettings(&platformSettings);
            platformSettings.mode = NEXUS_ClientMode_eVerified;
            platformSettings.openFrontend = true;
            nrc = NEXUS_Platform_Init(&platformSettings);
            BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Platform_Init Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
        }
#endif /* NXCLIENT_SUPPORT */

    return nrc;

error:
#if NXCLIENT_SUPPORT
    NxClient_Uninit();
#else /* !NXCLIENT_SUPPORT */
    NEXUS_Platform_Uninit();
#endif /* NXCLIENT_SUPPORT */
    return nrc;
}

#if 0
/*Example code for Mutlitswtich control through diseqc. Not tested */
int satmgr_switch(int tuner, int sat, int xpndr)
{

    uint8_t diseqc_send_command[8];
    NEXUS_FrontendDiseqcSettings diseqcSettings;

    NEXUS_Error rc=NEXUS_SUCCESS;
    unsigned i, diseqc_send_length;
    int master = 0;
    NEXUS_FrontendSatelliteStatus   pStatus;

    /* 913CR (4517) uses channel 0 as diseqc master, 913/813 uses channel 2 (4515) as diseqc master */
    rc=NEXUS_Frontend_GetSatelliteStatus(sdsCtrl[tuner].frontend, &pStatus);
    if (rc)
    {
        printf("get tuner status error =%d", rc);
    }
    else
    {
        if (pStatus.version.chipId != 0x4517)
            master = 2;
    }

#if ((NEXUS_PLATFORM==EV9400)||(NEXUS_PLATFORM==XIP112))
    master = tuner;
#endif

    BKNI_Memset(diseqc_send_command, 0, sizeof(diseqc_send_command));
    sdsCtrl[master].send_disecq_msg_cb_desc.callback = get_diseqc_reply;
    sdsCtrl[master].send_disecq_msg_cb_desc.context = sdsCtrl[master].frontend;
    sdsCtrl[master].send_disecq_msg_cb_desc.param = master;

    NEXUS_Frontend_ResetDiseqc(sdsCtrl[master].frontend, 0);

    BKNI_Sleep(300);

    NEXUS_Frontend_GetDiseqcSettings(sdsCtrl[master].frontend, &diseqcSettings);

    diseqcSettings.toneEnabled = true;

    NEXUS_Frontend_SetDiseqcSettings(sdsCtrl[master].frontend,&diseqcSettings);
#if 0
    diseqcSettings.voltage = NEXUS_FrontendDiseqcVoltage_e18v;

    NEXUS_Frontend_SetDiseqcSettings(sdsCtrl[master].frontend,&diseqcSettings);
#endif
    diseqc_send_command[0] = DISEQ_REPLY;

    if (has_splitter)
    {
        diseqc_send_command[1] = (tuner==0)? DISEQ_SWITCH_ADDRESS_TUNER1:((tuner==1)? DISEQ_SWITCH_ADDRESS_TUNER2:DISEQ_SWITCH_ADDRESS_TUNER3);
    }
    else    /* without separator */
        diseqc_send_command[1] = DISEQ_SWITCH_ADDRESS;

    diseqc_send_command[2] = DISEQ_WRITE_SWITCHES;

    if (sat == 119)     /*sat 119*/
    {
        printf("Diseqc switch to SAT 119\n");
        if (xpndr & 0x01)
            diseqc_send_command[3] = 0xA4; /*sat 119 odd*/
        else
            diseqc_send_command[3] = 0x86; /*sat 119 even*/
    }
    else if (sat == 110)        /*sat 110*/
    {
        printf("Diseqc switch to SAT 110\n");
        if (xpndr & 0x01)
            diseqc_send_command[3] = 0xE0;   /*sat 110 odd*/
        else
            diseqc_send_command[3] = 0xC2;   /*sat 110 even*/
    }
    else if (sat == 148)        /*sat 148*/
    {
        printf("Diseqc switch to SAT 148\n");
        if (xpndr & 0x01)
            diseqc_send_command[3] = 0x2C;   /*sat 148 odd*/
        else
            diseqc_send_command[3] = 0x0E;   /*sat 148 even*/
    }
    else if (sat == 129)        /*sat 129*/
    {
        printf("Diseqc switch to SAT 129\n");
        if (xpndr & 0x01)
            diseqc_send_command[3] = 0x68;   /*sat 129 odd*/
        else
            diseqc_send_command[3] = 0x4A;   /*sat 129 even*/
    }
    else
    {
        printf("Unknown SAT\n");
        return rc=1;
    }


    diseqc_send_length = 4;
    BDBG_MSG(("===================== send DISEQ_WRITE_SWITCHES command:\n"));
    for (i=0;i<8;i++)
        BDBG_MSG(("%d 0x%02x\n", i, diseqc_send_command[i]));

    NEXUS_Frontend_SendDiseqcMessage(sdsCtrl[master].frontend,diseqc_send_command,(size_t) diseqc_send_length,&sdsCtrl[master].send_disecq_msg_cb_desc);

    rc = BKNI_WaitForEvent(sdsCtrl[master].diseqcCallbackEvent, 300);

    if (rc)
    {
        printf("DISEQ_WRITE_SWITCHES failed (0x%X)!\n",rc);
        return rc;
    }
    return rc;
}
#endif
NEXUS_Error startNexusSatSrc(
    AppCtx            *appCtx,
    AppRtspSessionCtx *appRtspSession
    )
{
    NEXUS_Error                  rc;
    NEXUS_FrontendCapabilities   capabilities;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_ParserBandSettings     parserBandSettings;

    BSTD_UNUSED( appCtx );
    PRINTMSG_PARAMS(( "SatSettings frequency %d, mode %d pilot %d, symbolRate %d", appRtspSession->satSettings.frequency, appRtspSession->satSettings.mode, appRtspSession->satSettings.ldpcPilot, appRtspSession->satSettings.symbolRate ));

    BIP_CHECK_GOTO(( appRtspSession->frontend ), ( "Frontend handle is NULL" ), error, NEXUS_NOT_INITIALIZED, rc );

#if USE_HARDCODED_PARAMS
    NEXUS_Frontend_GetDefaultSatelliteSettings( &appRtspSession->satSettings );
    appRtspSession->satSettings.frequency = FREQ;
    appRtspSession->satSettings.mode      = SATELLITE_MODE;
#endif
    appRtspSession->satSettings.nyquist20 = true;
    appRtspSession->satSettings.nyquistRolloff = NEXUS_FrontendSatelliteNyquistFilter_e20;

    appRtspSession->satSettings.lockCallback.callback = lock_callback;
    appRtspSession->satSettings.lockCallback.context  = appRtspSession;

    NEXUS_Frontend_GetUserParameters( appRtspSession->frontend, &userParams );
    NEXUS_Frontend_GetCapabilities( appRtspSession->frontend, &capabilities );
    diseqcSupport = capabilities.diseqc;

    NEXUS_ParserBand_GetSettings( appRtspSession->parserBand, &parserBandSettings );
    if (userParams.isMtsif)
    {
        parserBandSettings.sourceType               = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector( appRtspSession->frontend ); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
    }
    else
    {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    }
    parserBandSettings.transportType = TRANSPORT_TYPE;
    rc = NEXUS_ParserBand_SetSettings( appRtspSession->parserBand, &parserBandSettings );
    BIP_CHECK_GOTO(( !rc ), ( "NEXUS_ParserBand_SetSettings Failed" ), error, rc, rc );

    if (diseqcSupport)
    {
#if !USE_HARDCODED_PARAMS
        bool toneEnabled = appRtspSession->diseqcSettings.toneEnabled;
        NEXUS_FrontendDiseqcVoltage voltage = appRtspSession->diseqcSettings.voltage;
#endif
#if 0
    /* TODO: Example code.  Test multiswitch code when we have setup */
    if(appRtspSession->addSatSettings.signalSourceId == 1)
    {
        satmgr_switch(tuner, satellite, transponder);
    }
    else if (appRtspSession->addSatSettings.signalSourceId == 2)
    {
         satmgr_switch(tuner, satellite, transponder);
    }
    else if (appRtspSession->addSatSettings.signalSourceId == 3)
    {
        satmgr_switch(tuner, satellite, transponder);
    }
    else if (appRtspSession->addSatSettings.signalSourceId == 4)
    {
        satmgr_switch(tuner, satellite, transponder);
    }
#endif

    NEXUS_Frontend_GetDiseqcSettings( appRtspSession->frontend, &appRtspSession->diseqcSettings );

#if USE_HARDCODED_PARAMS
        appRtspSession->diseqcSettings.toneEnabled = TONE_ENABLED;
        appRtspSession->diseqcSettings.toneMode    = TONE_MODE;
        appRtspSession->diseqcSettings.voltage     = VOLTAGE;
 #else
        appRtspSession->diseqcSettings.toneEnabled = toneEnabled;
        /**
                * eTone selects tone to be 22 kHz square wave, used when there is a transistor circuit for directly tone injection onto the bus.
                * eEnvelope selects tone envelope signal which drives the 22 kHz oscillator enable of an LNB controller chip. (default)
                **/
        /* appRtspSession->diseqcSettings.toneMode    = TONE_MODE;*/
        appRtspSession->diseqcSettings.voltage = voltage;
 #endif /* if USE_HARDCODED_PARAMS */
        rc = NEXUS_Frontend_SetDiseqcSettings( appRtspSession->frontend, &appRtspSession->diseqcSettings );
        BIP_CHECK_GOTO(( !rc ), ( "NEXUS_Frontend_SetDiseqcSettings Failed" ), error, rc, rc );
    }

    rc = NEXUS_Frontend_TuneSatellite( appRtspSession->frontend, &appRtspSession->satSettings );
    BIP_CHECK_GOTO(( !rc ), ( "NEXUS_Frontend_TuneSatellite Failed" ), error, rc, rc );

    BDBG_MSG(( "%s: Success: frontend (%p)", BSTD_FUNCTION, (void *)appRtspSession->frontend));
error:
    return( rc );
} /* startNexusSatSrc */

void stopNexusSatSrc(
    AppCtx            *appCtx,
    AppRtspSessionCtx *appRtspSession
    )
{
    BSTD_UNUSED( appCtx );

    if (appRtspSession->frontend)
    {
        NEXUS_StopCallbacks( appRtspSession->frontend );
        NEXUS_Frontend_Untune( appRtspSession->frontend );
    }

    BDBG_MSG(( "%s: Done: frontend (%p)", BSTD_FUNCTION, (void *)appRtspSession->frontend ));
} /* stopNexusSatSrc */

NEXUS_Error openNexusSatSrc(
    AppCtx            *appCtx,
    AppRtspSessionCtx *appRtspSession
    )
{
    NEXUS_Error                   rc = NEXUS_SUCCESS;
    NEXUS_PlatformConfiguration   platformConfig;
    NEXUS_FrontendAcquireSettings settings;

    BSTD_UNUSED( appCtx );

    NEXUS_Platform_GetConfiguration( &platformConfig );
    NEXUS_Frontend_GetDefaultAcquireSettings( &settings );
    settings.capabilities.satellite = true;
    appRtspSession->frontend        = NEXUS_Frontend_Acquire( &settings );
    BIP_CHECK_GOTO(( appRtspSession->frontend ), ( "NEXUS_Frontend_Acquire Failed" ), error, NEXUS_NOT_INITIALIZED, rc );

    appRtspSession->parserBand = NEXUS_ParserBand_Open( NEXUS_ANY_ID );
    BIP_CHECK_GOTO(( appRtspSession->parserBand ), ( "NEXUS_ParserBand_Open Failed" ), error, NEXUS_NOT_INITIALIZED, rc );

    BDBG_MSG(( "%s: Success", BSTD_FUNCTION ));
    return( rc );

error:
    return( NEXUS_NOT_INITIALIZED );
} /* openNexusSatSrc */

void closeNexusSatSrc(
    AppCtx            *appCtx,
    AppRtspSessionCtx *appRtspSession
    )
{
    BSTD_UNUSED( appCtx );

    if (appRtspSession->parserBand) {NEXUS_ParserBand_Close( appRtspSession->parserBand ); appRtspSession->parserBand = NEXUS_ParserBand_eInvalid; }
    if (appRtspSession->frontend) {NEXUS_Frontend_Release( appRtspSession->frontend ); appRtspSession->frontend = NULL; }

    BDBG_MSG(( "%s: Done", BSTD_FUNCTION ));
} /* closeNexusSatSrc */

NEXUS_Error openNexusIpDst(
    AppCtx            *appCtx,
    AppRtspSessionCtx *appRtspSession
    )
{
    NEXUS_Error               rc = NEXUS_SUCCESS;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    int factor = 1;

    BSTD_UNUSED( appCtx );

    /* setup the recpump for buffering the live channel */
    NEXUS_Recpump_GetDefaultOpenSettings( &recpumpOpenSettings );
    BDBG_MSG(( "recpump %d: increasing by %d folds, data size %d, threshold %d, atom size %d, index size %d",
               1, factor, factor*recpumpOpenSettings.data.bufferSize, recpumpOpenSettings.data.dataReadyThreshold, recpumpOpenSettings.data.atomSize, factor*recpumpOpenSettings.index.bufferSize ));
    /* increase the default record buffer size as IP streaming can introduce delays */
    recpumpOpenSettings.data.bufferSize         = factor * recpumpOpenSettings.data.bufferSize;
    recpumpOpenSettings.data.dataReadyThreshold = recpumpOpenSettings.data.atomSize * 5; /*RTP 5, eles 22 */
    recpumpOpenSettings.index.bufferSize        = factor * recpumpOpenSettings.index.bufferSize;
    appRtspSession->recpumpHandle               = NEXUS_Recpump_Open( NEXUS_ANY_ID, &recpumpOpenSettings );
    BIP_CHECK_GOTO(( appRtspSession->recpumpHandle ), ( "NEXUS_Recpump_Open Failed" ), error, NEXUS_UNKNOWN, rc );

    BDBG_MSG(( "%s: Success", BSTD_FUNCTION ));

    return( NEXUS_SUCCESS );

error:

    return( rc );
} /* openNexusIpDst */

void closeNexusIpDst(
    AppCtx            *appCtx,
    AppRtspSessionCtx *appRtspSession
    )
{
    BSTD_UNUSED( appCtx );

    if (appRtspSession->recpumpHandle) {NEXUS_Recpump_Close( appRtspSession->recpumpHandle ); }

    BDBG_MSG(( "%s: Done", BSTD_FUNCTION ));
} /* closeNexusIpDst */

NEXUS_Error startNexusIpDst(
    AppCtx            *appCtx,
    AppRtspSessionCtx *appRtspSession
    )
{
    BIP_StreamerTunerInputSettings streamerSettings;
    NEXUS_Error          rc = NEXUS_SUCCESS;
    BIP_Status            brc;

#if USE_HARDCODED_PARAMS
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel, patPidChannel, pmtPidChannel;
#else
    int i;
#endif

    BSTD_UNUSED( appCtx );

    /* Add PidChannels to recpump */
#if  USE_HARDCODED_PARAMS
    videoPidChannel = NEXUS_PidChannel_Open( appRtspSession->parserBand, VIDEO_PID, NULL );
    BIP_CHECK_GOTO(( videoPidChannel ), ( "NEXUS_PidChannel_Open failed for vidoePId Failed" ), error, NEXUS_UNKNOWN, rc );
    rc = NEXUS_Recpump_AddPidChannel( appRtspSession->recpumpHandle, videoPidChannel, NULL );
    BIP_CHECK_GOTO(( !rc ), ( "NEXUS_Recpump_AddPidChannel Failed for videoPid" ), error, NEXUS_UNKNOWN, rc );

    audioPidChannel = NEXUS_PidChannel_Open( appRtspSession->parserBand, AUDIO_PID, NULL );
    BIP_CHECK_GOTO(( audioPidChannel ), ( "NEXUS_PidChannel_Open failed for vidoePId Failed" ), error, NEXUS_UNKNOWN, rc );
    rc = NEXUS_Recpump_AddPidChannel( appRtspSession->recpumpHandle, audioPidChannel, NULL );
    BIP_CHECK_GOTO(( !rc ), ( "NEXUS_Recpump_AddPidChannel Failed for audioPid" ), error, NEXUS_UNKNOWN, rc );

    patPidChannel = NEXUS_PidChannel_Open( appRtspSession->parserBand, PAT_PID, NULL );
    BIP_CHECK_GOTO(( patPidChannel ), ( "NEXUS_PidChannel_Open failed for vidoePId Failed" ), error, NEXUS_UNKNOWN, rc );
    rc = NEXUS_Recpump_AddPidChannel( appRtspSession->recpumpHandle, patPidChannel, NULL );
    BIP_CHECK_GOTO(( !rc ), ( "NEXUS_Recpump_AddPidChannel Failed for patPid" ), error, NEXUS_UNKNOWN, rc );

    pmtPidChannel = NEXUS_PidChannel_Open( appRtspSession->parserBand, PMT_PID, NULL );
    BIP_CHECK_GOTO(( pmtPidChannel ), ( "NEXUS_PidChannel_Open failed for vidoePId Failed" ), error, NEXUS_UNKNOWN, rc );
    rc = NEXUS_Recpump_AddPidChannel( appRtspSession->recpumpHandle, pmtPidChannel, NULL );
    BIP_CHECK_GOTO(( !rc ), ( "NEXUS_Recpump_AddPidChannel Failed for pmtPid" ), error, NEXUS_UNKNOWN, rc );

#else /* if  USE_HARDCODED_PARAMS */
    BDBG_MSG(("%s: pid list count %d ",BSTD_FUNCTION,appRtspSession->pidInfo.pidListCount));
    for (i = 0; i< appRtspSession->pidInfo.pidListCount; i++)
    {
        BDBG_MSG(("%s: Adding pid channel (%d)",BSTD_FUNCTION, appRtspSession->pidInfo.pidList[i]));
        appRtspSession->pidChannelList[i] = NEXUS_PidChannel_Open( appRtspSession->parserBand, appRtspSession->pidInfo.pidList[i], NULL );
        BIP_CHECK_GOTO(( appRtspSession->pidChannelList[i] ), ( "NEXUS_PidChannel_Open failed for pid %d ", appRtspSession->pidInfo.pidList[i] ), error, NEXUS_UNKNOWN, rc );
        rc = NEXUS_Recpump_AddPidChannel( appRtspSession->recpumpHandle, appRtspSession->pidChannelList[i], NULL );
        BIP_CHECK_GOTO(( !rc ), ( "NEXUS_Recpump_AddPidChannel Failed for pid %p", (void *)appRtspSession->pidChannelList[i] ), error, NEXUS_UNKNOWN, rc );
    }
#endif /* if  USE_HARDCODED_PARAMS */


    /* Need to add pids and start recpump first  or Start the streamer will error out as get buffer fails */
    BIP_RtspSession_GetDefaultStartStreamerSettings( &streamerSettings );
    streamerSettings.hRecpump= appRtspSession->recpumpHandle;
    /* SAT>IP only supports MPEG2 TS as stream transport type */
    /*streamerSettings.streamTransportType = NEXUS_TransportType_eTs;*/
    brc = BIP_RtspSession_StartStreamer( appRtspSession->hRtspSession, &streamerSettings );
    BIP_CHECK_GOTO(( !brc ), ( "Unable to Start Streamer on session %p", (void *)appRtspSession->hRtspSession ), error, NEXUS_INVALID_PARAMETER, rc );

    BDBG_MSG(( "%s: SUCCESS", BSTD_FUNCTION ));
    return( rc );

error:
    BDBG_ERR(( "%s: Failed", BSTD_FUNCTION ));
    return( NEXUS_UNKNOWN );
} /* startNexusIpDst */

void stopNexusIpDst(
    AppCtx            *appCtx,
    AppRtspSessionCtx *appRtspSession
    )
{
    BSTD_UNUSED( appCtx );

    /* we should stop the streamer before stopping the recpump */
    if (appRtspSession->hRtspSession) {BIP_RtspSession_StopStreamer( appRtspSession->hRtspSession ); }
    if (appRtspSession->recpumpHandle) {NEXUS_Recpump_Stop( appRtspSession->recpumpHandle ); }
    if (appRtspSession->recpumpHandle) {NEXUS_Recpump_RemoveAllPidChannels( appRtspSession->recpumpHandle ); }
    if (appRtspSession->parserBand) {NEXUS_PidChannel_CloseAll( appRtspSession->parserBand ); }

    BDBG_MSG(( "%s: Done", BSTD_FUNCTION ));
} /* stopNexusIpDst */

static void rtspSessionSendResponse(
    AppRtspSessionCtx     *appRtspSession,
    BIP_RtspSessionHandle  hRtspSession,
    BIP_RtspResponseStatus rtspResponseStatus,
    BIP_RtspRequestHandle  hRtspRequest
    )
{
    BIP_Status rc;

    BDBG_MSG(( "%s: responseStatus %x", BSTD_FUNCTION, rtspResponseStatus ));
    BSTD_UNUSED( hRtspRequest );

    /* Set a successful response */
    BIP_RtspResponse_SetStatus( appRtspSession->hRtspResponse, rtspResponseStatus );

    /* Send out the response */
#if 0
    rc = BIP_RtspSession_SendResponseUsingRequest( hRtspSession, appRtspSession->hRtspResponse, hRtspRequest );
#endif
    rc = BIP_RtspSession_SendResponse( hRtspSession, appRtspSession->hRtspResponse );
    BIP_CHECK_GOTO(( !rc ), ( "BIP_RtspSession_SendResponse Failed" ), error, BIP_ERR_INTERNAL, rc );

error:
    /* Note: we dont care about the case where we failed to send the response */
    /* RTSP Session would be torn-down when either by peer sending us a TEARDOWN message or session itself timing out */
    /* So app will get a callback to Destroy the RTSP Session. */
    return;
} /* rtspSessionSendResponse */

static BIP_Status rtspSessionUpdateTuningParams(
    AppCtx            *appCtx,
    AppRtspSessionCtx *appRtspSession
    )
{
    int       i;
    BIP_Status rc;

    BSTD_UNUSED( appCtx );

    BDBG_MSG(( "%s:", BSTD_FUNCTION ));
    rc = BIP_RtspSession_ParseSatelliteSettings( appRtspSession->hRtspSession, &appRtspSession->satSettings, &appRtspSession->diseqcSettings, &appRtspSession->addSatSettings);
    BIP_CHECK_GOTO(( !rc ), ( "BIP_RtspSession_ParseSatelliteSettings Failed" ), error, rc, rc );
    PRINTMSG_PARAMS(( "SatSettings frequency %d, mode %d pilot %d, symbolRate %d", appRtspSession->satSettings.frequency, appRtspSession->satSettings.mode, appRtspSession->satSettings.ldpcPilot, appRtspSession->satSettings.symbolRate ));

    /* Get Pids info */
    rc = BIP_RtspSession_GetPids( appRtspSession->hRtspSession, &appRtspSession->pidInfo );
    BIP_CHECK_GOTO(( !rc ), ( "BIP_RtspSession_GetPids Failed" ), error, rc, rc );

    PRINTMSG_PARAMS(( "%s:  pidInfo.pidListCount %d", BSTD_FUNCTION, appRtspSession->pidInfo.pidListCount ));
    for (i = 0; i< appRtspSession->pidInfo.pidListCount; i++)
    {
        PRINTMSG_PARAMS(( "%s: pidInfo.pidList[%d] = %d", BSTD_FUNCTION, i, appRtspSession->pidInfo.pidList[i] ));
    }

    /* Free the previous pidChannelList if already allocated */
    if (appRtspSession->pidChannelList) {BKNI_Free( appRtspSession->pidChannelList ); }

    /* Allocate pid channels corresponding to each PID */
    appRtspSession->pidChannelList = (NEXUS_PidChannelHandle *) BKNI_Malloc( appRtspSession->pidInfo.pidListCount * sizeof( NEXUS_PidChannelHandle ));
    BIP_CHECK_GOTO(( appRtspSession->pidChannelList ), ( "BKNI_Malloc Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
    BKNI_Memset( appRtspSession->pidChannelList, 0, appRtspSession->pidInfo.pidListCount * sizeof( NEXUS_PidChannelHandle ) );

    return( BIP_SUCCESS );

error:
    return( BIP_ERR_INTERNAL );
} /* rtspSessionUpdateTuningParams */

static BIP_Status printAppSessions(
    AppCtx *appCtx
    )
{
    AppRtspSessionCtx *appRtspSession = NULL;
    int                i              = 0;

    BDBG_ENTER( printAppSessions );
    PRINTMSG_LIST(( "\n ---------- appRtspSession List Debug Start ------------  " ));
    appRtspSession = BLST_Q_FIRST( &appCtx->rtspSessionListHead );
    if (appRtspSession==NULL)
    {
        PRINTMSG_LIST(( "appRtspSession is empty  \n " ));
    }

    while (appRtspSession != NULL)
    {
        PRINTMSG_LIST(( "appRtspSession List index %d \n ", i++ ));

        PRINTMSG_LIST(( "%s: App Session %p appRtspSession->hRtspSession %p  ", BSTD_FUNCTION, (void *)appRtspSession, (void *)appRtspSession->hRtspSession ));
        /*  BDBG_OBJECT_ASSERT( appRtspSession->hRtspSession, BIP_RtspSession ); */
        PRINTMSG_LIST(( "------------------------\n" ));
        appRtspSession = BLST_Q_NEXT( appRtspSession, rtspSessionListNext );
    }
    PRINTMSG_LIST(( " ---------- appRtspSession List Debug End ------------ \n  " ));

    BDBG_LEAVE( printAppSessions );

    return( 0 );
} /* printAppSessions */

static void rtspSessionProcessEvents(
    AppCtx *appCtx
    )
{
    BIP_RtspRequestHandle hRtspRequest   = NULL;
    BIP_RtspSessionHandle hRtspSession   = NULL;
    AppRtspSessionCtx    *appRtspSession = NULL;
    BIP_Status             rc             = BIP_SUCCESS;
    NEXUS_Error           nrc            = 0;
    bool                  startOver      = false;

    BDBG_MSG(( "%s: ", BSTD_FUNCTION ));

    /* Loop thru the list of sessions and process ones with pending messages */
    appRtspSession = BLST_Q_FIRST( &appCtx->rtspSessionListHead );
    while (appRtspSession && rc==BIP_SUCCESS)
    {
        /* For each session, there may be more than 1 message, process them all. */

        /* Debug */
        printAppSessions( appCtx );

        hRtspRequest = appRtspSession->hRtspRequest;
        /* checking  All Message for a particular Session rc will be BIP Unavaible when out of messages */
        while (hRtspRequest && rc==BIP_SUCCESS)
        {
            BIP_RtspRequestMethod method;

            BKNI_Memset( &method, 0, sizeof( method ));

            /* loop until app receives all pending messages on this hRtspSession */
            hRtspSession = appRtspSession->hRtspSession;

            /* now read the incoming RTSP Request Message */
            rc = BIP_RtspSession_RecvRequest( hRtspSession, hRtspRequest );
            if (rc == BIP_ERR_NOT_AVAILABLE)
            {
                /* no more messages available for this hRtspSession, so break out */
                BDBG_MSG(( "No more message available for hRtspSession %p ...", (void *)hRtspSession ));
                break;
            }
            else if (rc != BIP_SUCCESS)
            {
                /* error while receiving the next RtspRequest */
                BDBG_ERR(( "ERROR while receiving the next RtspRequest %p ...", (void *)hRtspSession ));
                /* shouldn't happen unless there is an internal BIP bug, so catch it early! */
                BDBG_ASSERT( NULL );
                break;
            }

            rc = BIP_RtspResponseStatus_eInvalid; /* force the logic to determine success */

            /* check the message type and take appropriate action */
            BIP_RtspRequest_GetMethod( hRtspRequest, &method );
            switch (method)
            {
                case BIP_RtspRequestMethod_eSetup:
                case BIP_RtspRequestMethod_ePlayWithUrl:
                {
                    BDBG_MSG(( "%s: SETUP or PLAY_WithUrl Method: treat it like Channel Change %p", BSTD_FUNCTION, (void *)appRtspSession->hRtspSession ));
                    stopNexusIpDst( appCtx, appRtspSession );
                    stopNexusSatSrc( appCtx, appRtspSession );

                    /* update the Nexus tuning parameters from this new SETUP or PLAY w/ URL request */
                    rc = rtspSessionUpdateTuningParams( appCtx, appRtspSession );
                    BIP_CHECK_GOTO(( !rc ), ( "rtspSessionUpdateTuningParams Failed" ), errorOnSession, BIP_RtspResponseStatus_eServerError, rc );

                }
                case BIP_RtspRequestMethod_ePlay:
                {
                    BDBG_MSG(( "%s: PLAY Request on app Session %p, hRtspSession %p", BSTD_FUNCTION, (void *)appRtspSession ,(void *)appRtspSession->hRtspSession ));
                    nrc = startNexusSatSrc( appCtx, appRtspSession );
                    BIP_CHECK_GOTO(( !nrc ), ( "startNexusSatSrc Failed" ), errorOnSession, BIP_RtspResponseStatus_eServerError, rc );

                    nrc = startNexusIpDst( appCtx, appRtspSession );
                    BIP_CHECK_GOTO(( !nrc ), ( "startNexusIpDst Failed" ), errorOnSession, BIP_RtspResponseStatus_eServerError, rc );

                    /* all is well, so send a good response */
                    rc = BIP_RtspResponseStatus_eSuccess;
                    break;
                }
                case BIP_RtspRequestMethod_eTeardown:
                {
                    BDBG_MSG(( "%s: TEARDOWN Request on app Session %p, hRtspSession %p", BSTD_FUNCTION,(void *)appRtspSession , (void *)appRtspSession->hRtspSession ));
                    stopNexusIpDst( appCtx, appRtspSession );
                    stopNexusSatSrc( appCtx, appRtspSession );
                    closeNexusIpDst( appCtx, appRtspSession );
                    closeNexusSatSrc( appCtx, appRtspSession );

                    /* all is well, so send a good response */
                    rc = BIP_RtspResponseStatus_eSuccess;

                    /* do not destroy the session here; we need to send the response first! */
                    break;
                }

                default:
                {
                    BDBG_MSG(( "%s: ERROR: method (%d) is not recognized/supported", BSTD_FUNCTION, method ));
                    rc = BIP_RtspResponseStatus_eServerError;
                    break;
                }
            } /* switch */

errorOnSession:
            /* now send the response for this session */
            rtspSessionSendResponse( appRtspSession, appRtspSession->hRtspSession, rc, hRtspRequest );

            if (method == BIP_RtspRequestMethod_eTeardown)
            {
                PRINTMSG_LIST(( "%s: BLST_Q_REMOVE appRtspSession (%p)", BSTD_FUNCTION, (void *)appRtspSession ));
                BLST_Q_REMOVE( &appCtx->rtspSessionListHead, appRtspSession, rtspSessionListNext );

                rtspListenerDestroySession( appRtspSession );
                appRtspSession = NULL;

                startOver      = true;
                appRtspSession = BLST_Q_FIRST( &appCtx->rtspSessionListHead );

                break; /* we are done processing requests for this session; end the inner loop and get request for another session */
            }
            else
            {
                #if 0
                appRtspSession = BLST_Q_NEXT( appRtspSession, rtspSessionListNext );
                BDBG_MSG(( "%s: End of Inner while Loop Sssion id %p", BSTD_FUNCTION, appRtspSession ));
                #endif
            }
        } /* while for processing all events for this session */

        if (startOver)
        {
            appRtspSession = BLST_Q_FIRST( &appCtx->rtspSessionListHead );

            if (appRtspSession !=NULL)
            {
                PRINTMSG_LIST(( "%s: Starting from first AppSession(saw Teardown) App Session %p hRtspSession %p  rc %d ", BSTD_FUNCTION, (void *)appRtspSession, (void *)appRtspSession->hRtspSession,  rc ));
            }
            else
            {
                PRINTMSG_LIST(( "%s: Starting from first AppSession(saw Teardown) App Session %p NULL   rc %d ", BSTD_FUNCTION, (void *)appRtspSession,  rc ));
            }

            startOver = false;
            rc        = BIP_SUCCESS;
        }
        else
        {
            appRtspSession = BLST_Q_NEXT( appRtspSession, rtspSessionListNext );
            rc             = BIP_SUCCESS;
            if (appRtspSession !=NULL)
            {
                PRINTMSG_LIST(( "%s: Moving to Next  App Session %p hRtspSession %p   rc %d ", BSTD_FUNCTION, (void *)appRtspSession, (void *)appRtspSession->hRtspSession,  rc ));
            }
            else
            {
                PRINTMSG_LIST(( "%s: Moving to Next  App Session %p NULL   rc %d ", BSTD_FUNCTION, (void *)appRtspSession, rc ));
            }
        }
    }     /* for loop for processing the next session */

    return;
} /* rtspSessionProcessEvents */

static void rtspSessionIgmpMembershipReportProcessEvents(
    AppCtx *appCtx
    )
{
    BIP_RtspIgmpMemRepStatus igmpStatus;
    BIP_RtspSessionHandle    hRtspSession   = NULL;
    BIP_RtspResponseStatus   responseStatus = BIP_RtspResponseStatus_eSuccess;
    AppRtspSessionCtx       *appRtspSession = NULL;
    BIP_Status                rc;

    /* Loop thru the list of sessions and process ones with pending Igmp events */
    for (appRtspSession = BLST_Q_FIRST( &appCtx->rtspSessionListHead );
         appRtspSession;
         appRtspSession = BLST_Q_NEXT( appRtspSession, rtspSessionListNext ))
    {
        /* Pick up the latest Igmp Status from  hRtspSession */
        hRtspSession = appRtspSession->hRtspSession;
        BIP_RtspSession_GetIgmpStatus( hRtspSession, &igmpStatus );
        switch (igmpStatus)
        {
            case BIP_RtspIgmpMemRepStatus_eJoin:
            {

                BDBG_MSG(( "%s: Join Memebership. appRtspSession %p appRtspSession->hRtspSession %p  ", BSTD_FUNCTION, (void *)appRtspSession, (void *)appRtspSession->hRtspSession ));
                /* Get various SesSatIp specific fields from the URL */
                rc = rtspSessionUpdateTuningParams( appCtx, appRtspSession );
                BIP_CHECK_GOTO(( !rc ), ( "rtspSessionUpdateTuningParams Failed" ), error, rc, rc );

                rc = startNexusSatSrc( appCtx, appRtspSession );
                BIP_CHECK_GOTO(( !rc ), ( "startNexusSatSrc Failed" ), error, BIP_RtspResponseStatus_eServerError, responseStatus );

                rc = startNexusIpDst( appCtx, appRtspSession );
                BIP_CHECK_GOTO(( !rc ), ( "startNexusIpDst Failed" ), error, BIP_RtspResponseStatus_eServerError, responseStatus );

                responseStatus = BIP_RtspResponseStatus_eSuccess;
                break;
            }
            case BIP_RtspIgmpMemRepStatus_eLeave:
            {

                BDBG_MSG(( "%s: Leave Memebership. appRtspSession %p appRtspSession->hRtspSession %p  ", BSTD_FUNCTION, (void *)appRtspSession, (void *)appRtspSession->hRtspSession ));
                stopNexusIpDst( appCtx, appRtspSession );
                stopNexusSatSrc( appCtx, appRtspSession );
                responseStatus = BIP_RtspResponseStatus_eSuccess;
                break;
            }
            default:
            {
                break;
            }
        } /* switch */
error:
        rtspSessionSendResponse( appRtspSession, appRtspSession->hRtspSession, responseStatus, NULL );
    }
    BDBG_MSG(( "%s: Done", BSTD_FUNCTION ));
    return;
} /* rtspSessionIgmpMembershipReportProcessEvents */

static void rtspSessionIgmpMembershipReportCallback(
    void *context,
    int   param
    )
{
    AppCtx *appCtx = context;

    BSTD_UNUSED( param );
    BDBG_MSG(( "%s: B_Event_Set( appCtx->hRtspSessionIgmpMembershipReportEvents )", BSTD_FUNCTION ));
    B_Event_Set( appCtx->hRtspSessionIgmpMembershipReportEvents );
}

static void rtspSessionMessageReceivedCallback(
    void *context,
    int   param
    )
{
    AppCtx *appCtx = context;

    BSTD_UNUSED( param );

    BDBG_MSG(( "%s: B_Event_Set( appCtx->hRtspSessionEvents )", BSTD_FUNCTION ));
    B_Event_Set( appCtx->hRtspSessionEvents );
}

static void rtspListenerDestroySession(
    AppRtspSessionCtx *appRtspSession
    )
{

    if (!appRtspSession) {return; }
    if (appRtspSession->hRtspSession)
    {
        BIP_RtspSession_Destroy( appRtspSession->hRtspSession );
        appRtspSession->hRtspSession = NULL;
    }
    if (appRtspSession->pidChannelList)
    {
        BKNI_Free( appRtspSession->pidChannelList );
        appRtspSession->pidChannelList = NULL;
    }
    if (appRtspSession->hRtspRequest)
    {
        BIP_RtspRequest_Destroy( appRtspSession->hRtspRequest );
        appRtspSession->hRtspRequest = NULL;
    }
    if (appRtspSession->hRtspResponse)
    {
        BIP_RtspResponse_Destroy( appRtspSession->hRtspResponse );
        appRtspSession->hRtspResponse = NULL;
    }
    if (appRtspSession)
    {
        BKNI_Free( appRtspSession );
    }
} /* rtspListenerDestroySession */

static AppRtspSessionCtx *rtspListenerCreateSession(
    AppCtx                *appCtx,
    BIP_RtspListenerHandle hRtspListener,
    BIP_RtspRequestHandle  hRtspRequest
    )
{
    BIP_Status               rc;
    AppRtspSessionCtx      *appRtspSession = NULL;
    BIP_RtspSessionHandle   hRtspSession   = NULL;
    BIP_RtspSessionSettings sessionSettings;

    BDBG_MSG(( "%s: creating session ...", BSTD_FUNCTION ));
    hRtspSession = BIP_RtspListener_CreateSession( hRtspListener, hRtspRequest );
    BIP_CHECK_GOTO(( hRtspSession ), ( "BIP_RtspListener_CreateSession Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    /* Allocate an app specific rtspSession ctx */
    appRtspSession = (AppRtspSessionCtx *)BKNI_Malloc( sizeof( AppRtspSessionCtx ));
    BIP_CHECK_GOTO(( appRtspSession ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
    BKNI_Memset( appRtspSession, 0, sizeof( AppRtspSessionCtx ));
    appRtspSession->hRtspSession = hRtspSession;

    /* maintain a list of rtspSessions */
    BLST_Q_INSERT_TAIL( &appCtx->rtspSessionListHead, appRtspSession, rtspSessionListNext );

    /* Update session settings: add callback for receiving messages on session object */
    BIP_RtspSession_GetSettings( hRtspSession, &sessionSettings );
    sessionSettings.messageReceivedCallback.context            = appCtx;
    sessionSettings.messageReceivedCallback.callback           = rtspSessionMessageReceivedCallback;
    sessionSettings.igmpMembershipReportEventCallback.context  = appCtx;
    sessionSettings.igmpMembershipReportEventCallback.callback = rtspSessionIgmpMembershipReportCallback;
    rc = BIP_RtspSession_SetSettings( hRtspSession, &sessionSettings );
    BIP_CHECK_GOTO(( !rc ), ( "BIP_RtspSession_SetSettings Failed" ), error, rc, rc );

    /* Create a rtsp request message template for reading request messages for this session */
    appRtspSession->hRtspRequest = BIP_RtspRequest_Create( NULL /*&rtspRequestCreateSettings*/ );
    BIP_CHECK_GOTO(( appRtspSession->hRtspRequest ), ( "BIP_RtspRequest_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    /* Create a rtsp response message template for sending response messages for this session */
    appRtspSession->hRtspResponse = BIP_RtspResponse_Create( NULL /*&hRtspResponseCreateSettings*/ );
    BIP_CHECK_GOTO(( appRtspSession->hRtspResponse ), ( "BIP_RtspResponse_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BDBG_MSG(( "%s: returning new session ... %p", BSTD_FUNCTION, (void *)appRtspSession ));
    return( appRtspSession );

error:
    if (appRtspSession && appRtspSession->hRtspSession)
    {
        /* send error Response on this Session before Destroying it */
        rtspSessionSendResponse( appRtspSession, appRtspSession->hRtspSession, BIP_RtspResponseStatus_eServerError, hRtspRequest );
    }
    rtspListenerDestroySession( appRtspSession );
    appRtspSession = NULL;
    return( NULL );
} /* rtspListenerCreateSession */

static void rtspListenerProcessSetupCmd(
    AppCtx                *appCtx,
    BIP_RtspListenerHandle hRtspListener,
    BIP_RtspRequestHandle  hRtspRequest
    )
{
    BIP_Status          rc             = BIP_SUCCESS;
    AppRtspSessionCtx *appRtspSession = NULL;

    /* Create Session */
    appRtspSession = rtspListenerCreateSession( appCtx, hRtspListener, hRtspRequest );
    BIP_CHECK_GOTO(( appRtspSession ), ( "Failed to create Session" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    /* Get various SesSatIp specific fields from the URL */
    rc = rtspSessionUpdateTuningParams( appCtx, appRtspSession );
    BIP_CHECK_GOTO(( !rc ), ( "rtspSessionUpdateTuningParams Failed" ), error, rc, rc );

    /* Now we have valid request */

    /* Open the Nexus Sattelite Src & Dst */
    rc = openNexusSatSrc( appCtx, appRtspSession );
    BIP_CHECK_GOTO(( !rc ), ( "openNexusSatSrc Failed" ), error, rc, rc );

    rc = openNexusIpDst( appCtx, appRtspSession );
    BIP_CHECK_GOTO(( !rc ), ( "openNexusIpDst Failed" ), error, rc, rc );

    /* At this point, we can handle this request, so lets go ahead with the RTSP Session setup & send a successful response */
    rtspSessionSendResponse( appRtspSession, appRtspSession->hRtspSession, BIP_RtspResponseStatus_eSuccess, hRtspRequest );

    return;

error:
    if (appRtspSession && appRtspSession->hRtspSession)
    {
        /* send error Response on this Session before Destroying it */
        rtspSessionSendResponse( appRtspSession, appRtspSession->hRtspSession, BIP_RtspResponseStatus_eServerError, hRtspRequest );
    }
    rtspListenerDestroySession( appRtspSession );
    appRtspSession = NULL;
    return;
} /* rtspListenerProcessSetupCmd */

static void rtspListenerProcessEvents(
    AppCtx *appCtx
    )
{
    BIP_Status             rc = BIP_SUCCESS;
    BIP_RtspRequestHandle hRtspRequest;

    hRtspRequest = appCtx->hRtspRequest;
    BDBG_ASSERT( hRtspRequest );

    /* Debug */
    printAppSessions( appCtx );

    /* loop until app receives all pending messages on this RtspListener */
    while (rc == BIP_SUCCESS)
    {
        BIP_RtspRequestMethod  method;
        BIP_RtspListenerHandle hRtspListener = appCtx->hRtspListener;

        rc = BIP_RtspListener_RecvRequest( hRtspListener, hRtspRequest );
        if (rc == BIP_ERR_NOT_AVAILABLE)
        {
            /* no more pending messages available for this listener, so break out */
            BDBG_MSG(( "%s: No more pending message available for hRtspListener %p",BSTD_FUNCTION,  (void *)hRtspListener ));
            break;
        }
        else if (rc != BIP_SUCCESS)
        {
            /* error while receiving the next RtspRequest */
            BDBG_ERR(( "ERROR while receiving the next RtspRequest %p ...", (void *)hRtspListener ));

            /* shouldn't happen unless there is an internal BIP bug, so catch it early! */
            BDBG_ASSERT( NULL );
            break;
        }

        /* check the message type and take appropriate action */
        BIP_RtspRequest_GetMethod( hRtspRequest, &method );

        switch (method)
        {
            case BIP_RtspRequestMethod_eSetup:
            {
                /* Process SETUP command */
                BDBG_MSG(( "%s: SETUP new session, method %d", BSTD_FUNCTION, method ));
                rtspListenerProcessSetupCmd( appCtx, hRtspListener, hRtspRequest );
                break;
            }
            default:
            {
                BDBG_WRN(( "%s: RtspListener should only get Setup Message, rest are internally handled by BIP!", BSTD_FUNCTION ));
                BDBG_ASSERT( NULL );
                break;
            }
        } /* switch */
    }     /* while */
}         /* rtspListenerProcessEvents */

static void rtspListenerMessageReceivedCallback(
    void *context,
    int   param
    )
{
    AppCtx *appCtx = context;

    BSTD_UNUSED( param );

    BDBG_MSG(( "%s: B_Event_Set( appCtx->hRtspListenerEvents )", BSTD_FUNCTION ));
    B_Event_Set( appCtx->hRtspListenerEvents );
}

void unInitRtspListener(
    AppCtx *appCtx
    )
{
    AppRtspSessionCtx *appRtspSession = NULL;

    if (!appCtx) {return; }
    BDBG_MSG(( "%s: ", BSTD_FUNCTION ));
    if (appCtx->rtspListenerStarted) {BIP_RtspListener_Stop( appCtx->hRtspListener ); }
    appCtx->rtspListenerStarted = false;

    while (( appRtspSession = BLST_Q_FIRST( &appCtx->rtspSessionListHead )) != NULL)
    {
        BLST_Q_REMOVE( &appCtx->rtspSessionListHead, appRtspSession, rtspSessionListNext );
        rtspListenerDestroySession( appRtspSession );
        appRtspSession = NULL;
    }

    if (appCtx->hRtspListener) {BIP_RtspListener_Destroy( appCtx->hRtspListener ); }
    appCtx->hRtspListener = NULL;

    if (appCtx->hRtspRequest) {BIP_RtspRequest_Destroy( appCtx->hRtspRequest ); }
    if (appCtx->hRtspListenerEvents) {B_EventGroup_RemoveEvent( appCtx->hRtspEventGroup, appCtx->hRtspListenerEvents ); }
    if (appCtx->hRtspSessionEvents) {B_EventGroup_RemoveEvent( appCtx->hRtspEventGroup, appCtx->hRtspSessionEvents ); }
    if (appCtx->hRtspSessionIgmpMembershipReportEvents) {B_EventGroup_RemoveEvent( appCtx->hRtspEventGroup, appCtx->hRtspSessionIgmpMembershipReportEvents ); }
    if (appCtx->hRtspListenerEvents) {B_Event_Destroy( appCtx->hRtspListenerEvents ); }
    if (appCtx->hRtspSessionEvents) {B_Event_Destroy( appCtx->hRtspSessionEvents ); }
    if (appCtx->hRtspSessionIgmpMembershipReportEvents) {B_Event_Destroy( appCtx->hRtspSessionIgmpMembershipReportEvents ); }
    if (appCtx->hRtspEventGroup) {B_EventGroup_Destroy( appCtx->hRtspEventGroup ); }
} /* unInitRtspListener */

BIP_Status initRtspListener(
    AppCtx *appCtx
    )
{
    BIP_Status rc;
    BIP_RtspListenerCreateSettings rtspListenerCreateSettings;
    BIP_RtspListenerSettings       rtspListenerSettings;

    /* Create Event group */
    appCtx->hRtspEventGroup = B_EventGroup_Create( NULL );
    BIP_CHECK_GOTO(( appCtx->hRtspEventGroup ), ( "Event Group Creation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    /* Create an event to indicate a callback on the listener */
    appCtx->hRtspListenerEvents = B_Event_Create( NULL );
    BIP_CHECK_GOTO(( appCtx->hRtspListenerEvents ), ( "Event Creation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
    appCtx->maxTriggeredEvents++;
    /* Add this event to the event group */
    rc = B_EventGroup_AddEvent( appCtx->hRtspEventGroup, appCtx->hRtspListenerEvents );
    BIP_CHECK_GOTO(( !rc ), ( "Failed to Add event to group" ), error, BIP_ERR_INTERNAL, rc );

    /* Create an event to indicate a callback on the session */
    appCtx->hRtspSessionEvents = B_Event_Create( NULL );
    BIP_CHECK_GOTO(( appCtx->hRtspSessionEvents ), ( "Event Creation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
    appCtx->maxTriggeredEvents++;
    /* Add this event to the event group */
    rc = B_EventGroup_AddEvent( appCtx->hRtspEventGroup, appCtx->hRtspSessionEvents );
    BIP_CHECK_GOTO(( !rc ), ( "Failed to Add event to group" ), error, BIP_ERR_INTERNAL, rc );

    /* Create an event to indicate a callback from an Igmp Membership Report Event */
    appCtx->hRtspSessionIgmpMembershipReportEvents = B_Event_Create( NULL );
    BIP_CHECK_GOTO(( appCtx->hRtspSessionIgmpMembershipReportEvents ), ( "Event Creation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
    appCtx->maxTriggeredEvents++;
    /* Add this event to the event group */
    rc = B_EventGroup_AddEvent( appCtx->hRtspEventGroup, appCtx->hRtspSessionIgmpMembershipReportEvents );
    BIP_CHECK_GOTO(( !rc ), ( "Failed to Add event to group" ), error, BIP_ERR_INTERNAL, rc );

    BIP_CHECK_GOTO( !( appCtx->maxTriggeredEvents > MAX_TRACKED_EVENTS ), ( "need to increase the MAX_TRACKED_EVENTS to %d", appCtx->maxTriggeredEvents ), error, BIP_ERR_INTERNAL, rc );

    /* create listener for RTSP based streaming sessions */
    BIP_RtspListener_GetDefaultCreateSettings( &rtspListenerCreateSettings );
    rtspListenerCreateSettings.port               = 554;
    rtspListenerCreateSettings.ipAddress          = NULL;
    rtspListenerCreateSettings.rtspSessionTimeout = 60; /* 10sec for testing, SAT>IP spec defaults it to 60 */
    rtspListenerCreateSettings.hIgmpListener      = appCtx->hIgmpListener;
    appCtx->hRtspListener = BIP_RtspListener_Create( &rtspListenerCreateSettings );
    BIP_CHECK_GOTO(( appCtx->hRtspListener ), ( "Failed to create RTSP Listener" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
    BLST_Q_INIT( &appCtx->rtspSessionListHead );

    /* set listener settings */
    BIP_RtspListener_GetSettings( appCtx->hRtspListener, &rtspListenerSettings );
    rtspListenerSettings.messageReceivedCallback.callback = rtspListenerMessageReceivedCallback;
    rtspListenerSettings.messageReceivedCallback.context  = appCtx;
    rc = BIP_RtspListener_SetSettings( appCtx->hRtspListener, &rtspListenerSettings );
    BIP_CHECK_GOTO(( !rc ), ( "BIP_RtspListener_SetSettings Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    /* Create a rtsp request message template for receiving messages on the listener */
    appCtx->hRtspRequest = BIP_RtspRequest_Create( NULL /*&rtspRequestCreateSettings*/ );
    BIP_CHECK_GOTO(( appCtx->hRtspRequest ), ( "BIP_RtspRequest_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    /* start listening for new requests */
    rc = BIP_RtspListener_Start( appCtx->hRtspListener );
    BIP_CHECK_GOTO(( !rc ), ( "BIP_RtspListener_Start Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    appCtx->rtspListenerStarted = true;
    return( rc );

error:
    unInitRtspListener( appCtx );
    return( rc );
} /* initRtspListener */

BIP_Status initIgmpListener(
    AppCtx *appCtx
    )
{
    BIP_Status rc;
    BIP_IgmpListenerCreateSettings igmpListenerCreateSettings;

    BIP_IgmpSesSatIpListener_GetDefaultCreateSettings( &igmpListenerCreateSettings );
    igmpListenerCreateSettings.lan_iface = INTERFACENAME;
    appCtx->hIgmpListener                = BIP_IgmpListener_Create( &igmpListenerCreateSettings );
    BIP_CHECK_GOTO(( appCtx->hIgmpListener ), ( "BIP_IgmpListener_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    return( BIP_SUCCESS );

error:
    return( rc );
}

void unInitIgmpListener(
    AppCtx *appCtx
    )
{
    if (!appCtx->hIgmpListener) {return; }
    BIP_IgmpListener_Stop( appCtx->hIgmpListener );
    BIP_IgmpListener_Destroy( appCtx->hIgmpListener );
}

int main(
    int    argc,
    char **argv
    )
{
    BIP_Status   rc;
    NEXUS_Error nrc;
    AppCtx     *appCtx;

    BSTD_UNUSED( argc );
    BSTD_UNUSED( argv );

    nrc = BKNI_Init();
    BIP_CHECK_GOTO(( !nrc ), ( "BKNI_Init() Failed" ), error_bkni, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    B_Os_Init();
    appCtx = malloc( sizeof( AppCtx ));

    /* register a signal handler to catch ctrl-c and do a clean shutdown of example app */
    signal( SIGINT, signalHandler );

    nrc = initNexus( appCtx );
    BIP_CHECK_GOTO(( !nrc ), ( "initNexus Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    rc = initIgmpListener( appCtx );
    BIP_CHECK_GOTO(( !rc ), ( "initIgmpListener Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    rc = initRtspListener( appCtx );
    BIP_CHECK_GOTO(( !rc ), ( "initRtspListener Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    /* Wait on Event group and process events as they come */
    BDBG_MSG(( "%s: Start monitoring events", BSTD_FUNCTION ));
    BDBG_LOG(( "%s: type 'q' or 'quit' followed by ENTER to exit gracefully", BSTD_FUNCTION ));
    while (!gGotSigInt && !gExitThread)
    {
        unsigned      i;
        unsigned      numTriggeredEvents;
        B_EventHandle triggeredEvents[MAX_TRACKED_EVENTS];

        if(userInputCheck())
        {

           char buffer[256];
           char *buf;

           fgets(buffer, 256, stdin);
           if (feof(stdin)){continue;}
           buffer[strlen(buffer)-1] = 0; /* chop off \n */

           buf = strtok(buffer, ";");
           if (!buf) continue;

           BDBG_MSG(("userInputRecieved"));
           if (!strcmp(buf, "q") || !strcmp(buf, "quit")) {
              BDBG_LOG(("Recieved quit "));
              gExitThread = true;
           }
        }

        /* force the Wait to timeout in 1 second to give us a chance to check the gGotSigInt boolean */
        B_EventGroup_Wait( appCtx->hRtspEventGroup, 1000, triggeredEvents, appCtx->maxTriggeredEvents, &numTriggeredEvents );
        for (i = 0; i < numTriggeredEvents; i++)
        {
            BDBG_MSG(( "%s:  event ARRIVED \n\n", BSTD_FUNCTION ));
            if (triggeredEvents[i] == appCtx->hRtspListenerEvents)
            {
                BDBG_MSG(( "%s: Process RtspListener events", BSTD_FUNCTION ));
                /* process all events on the Rtsp listener */
                rtspListenerProcessEvents( appCtx );
                /* now process the next event */
                continue;
            }
            if (triggeredEvents[i] == appCtx->hRtspSessionEvents)
            {
                BDBG_MSG(( "%s: Process RtspSessions Events", BSTD_FUNCTION ));
                rtspSessionProcessEvents( appCtx );
                continue;
            }
            if (triggeredEvents[i] == appCtx->hRtspSessionIgmpMembershipReportEvents)
            {
                BDBG_MSG(( "%s: Process Igmp Membership Report Events ", BSTD_FUNCTION ));
                rtspSessionIgmpMembershipReportProcessEvents( appCtx );
                continue;
            }
            BDBG_WRN(( "%s: didn't process event (i %d)", BSTD_FUNCTION, i ));
        }
    }
    BDBG_LOG(( "%s: Shutting down", BSTD_FUNCTION ));

error:
    unInitIgmpListener( appCtx );
    unInitRtspListener( appCtx );
#if NXCLIENT_SUPPORT
        NxClient_Uninit();
#else /* !NXCLIENT_SUPPORT */
    NEXUS_Platform_Uninit();
#endif
    /* For debugging any memory leaks, use this */
    B_Os_Uninit();
    BKNI_Uninit();

error_bkni:
    BDBG_LOG(( "All done!" ));
    return( 0 );
} /* main */
#else
int main(
    int    argc,
    char **argv
    )
{
    BSTD_UNUSED( argc );
    BSTD_UNUSED( argv );
    printf("This app is not supported for NON Frontend based platform.");
}

#endif
