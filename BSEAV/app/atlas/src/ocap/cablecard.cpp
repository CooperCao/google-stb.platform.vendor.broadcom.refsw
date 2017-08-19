/***************************************************************************
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
 *
 * Module Description:
 *
 ****************************************************************************/
#ifdef MPOD_SUPPORT
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <ctype.h>

#include "bstd.h"
#include "bkni.h"

#include "cablecard.h"
#include "band.h"
#include "nexus_parser_band.h"
#include "tuner_oob.h"
#include "tuner_upstream.h"
#include "channel.h"
#include "band.h"
#include "channelmgr.h"
#if NEXUS_FRONTEND_DOCSIS
#include "nexus_docsis.h"
#elif NEXUS_FRONTEND_3255
#include "nexus_frontend_3255.h"
#else
#endif

#ifdef SOFTWARE_RS_MPOD
#include "mpod_rate_smooth.h"
#endif
#if NEXUS_HAS_SECURITY
#include "nexus_security.h"
#endif

#define CABLECARD_DEVICE        "/dev/pod"
#ifdef CDL_SUPPORT
#include "b_cdl_lib.h"
#include "cdl_adpu.h"
#include "cdl_dsg.h"
#endif
#ifdef ESTB_CFG_SUPPORT
#include "b_estb_cfg_lib.h"
#endif

#ifdef LINUX
static pthread_mutex_t g_cablecard_mutex = PTHREAD_MUTEX_INITIALIZER;
#define cablecard_lock() pthread_mutex_lock(&g_cablecard_mutex)
#define cablecard_unlock() pthread_mutex_unlock(&g_cablecard_mutex)
#else
error "Not Supported"
#endif

CTunerOob * _pTunerOob;
CTunerUpstream * _pTunerUpstream;
CChannelMgr * g_pChannelMgr;

CCablecard * g_pCableCard;
static Cablecard g_cablecard;
#if NEXUS_FRONTEND_DOCSIS
NEXUS_FrontendDeviceHandle g_oobHFrontendDeviceHandle=NULL;
#endif
NEXUS_FrontendHandle g_oobHFrontendHandle=NULL;
NEXUS_FrontendHandle g_upstreamHFrontendHandle=NULL;
static B_MPOD_SAS_HANDLE SasConnections[32]; /* max 32 connections allowed by the Host 2.0 spec */
static uint8_t SasData[8] = {0xde, 0xad, 0xbe, 0xef, 0x01, 0x23, 0x45, 0x67};
static CChannel *pCableCardChannels[MAX_CABLECARD_ROUTE];
/* TODO: move pmt info into CChannel class later to make it more generic */
typedef struct channelPmtInfo_t
{
  uint8_t pmt[4096];
  uint32_t pmtSize;
}channelPmtInfo_t;
static channelPmtInfo_t channelPmtInfo[MAX_CABLECARD_ROUTE];
static B_EventHandle keySlotRemoved[MAX_CABLECARD_ROUTE];
/* generic CabelCARD information */
static CableCARDInfo info;
static B_EventHandle MpegFlowHandlerEvent;
static B_ThreadHandle MpegFlowHandler = NULL;
static uint32_t MpegFlowId = 0;
static bool MpegFlowReq = false;


BDBG_MODULE(atlas_cablecard);
#if !defined(DSG_SUPPORT) && defined(MPOD_SUPPORT)
 void CableCardCallbackSetDSGMode( unsigned char * data, unsigned short len ){BSTD_UNUSED(data);BSTD_UNUSED(len);}
 void CableCardCallbackDSG_Directory( unsigned char *data, unsigned short len ){BSTD_UNUSED(data);BSTD_UNUSED(len);}
 void CableCardCallbackDSGPacketError( unsigned char err_status ){BSTD_UNUSED(err_status);}
 void CableCardCallbackConfig_Advanced_DSG( unsigned char *pkt_obj_ptr, unsigned short len ){BSTD_UNUSED(pkt_obj_ptr);BSTD_UNUSED(len);}
 void CableCardCallbackDSGFlowID( unsigned long flow_id ){BSTD_UNUSED(flow_id);}
 unsigned long CableCardCallbackIPUDhcp( unsigned long flowid, unsigned char *mac, unsigned char opt_len, unsigned char *opt_data, unsigned char *ret_option, int *ropt_len )
			{BSTD_UNUSED(flowid);BSTD_UNUSED(mac);BSTD_UNUSED(opt_len);BSTD_UNUSED(opt_data);BSTD_UNUSED(ret_option);BSTD_UNUSED(ropt_len);return 0;}
 unsigned char CableCardCallbackSocketFlowConfig(unsigned long id, unsigned char *opt_data, unsigned long opt_len){BSTD_UNUSED(id);BSTD_UNUSED(opt_data);BSTD_UNUSED(opt_len);return 0;}
 unsigned char CableCardCallbackDeleteFlowReq(unsigned long flowid){BSTD_UNUSED(flowid);return 0;}
 void CableCardCallbackSendIPUData( unsigned char *pdata, unsigned long len ){BSTD_UNUSED(pdata);BSTD_UNUSED(len);}
 void CableCardCallbackSendSocketFlowUsData( unsigned long id, unsigned char *opt_data, unsigned long opt_len ){BSTD_UNUSED(id);BSTD_UNUSED(opt_data);BSTD_UNUSED(opt_len);}
 void CableCardRemovedCallbackCleanUp( void ){}

#else
extern "C"
{
// external call to DSGCC lib
 int InitializeDsgCcApplication(void);
 void CableCardCallbackSetDSGMode( unsigned char * data, unsigned short len );
 void CableCardCallbackDSG_Directory( unsigned char *data, unsigned short len );
 void CableCardCallbackDSGPacketError( unsigned char err_status );
 void CableCardCallbackConfig_Advanced_DSG( unsigned char *pkt_obj_ptr, unsigned short len );
 void CableCardCallbackDSGFlowID( unsigned long flow_id );
 unsigned long CableCardCallbackIPUDhcp( unsigned long flowid, unsigned char *mac, unsigned char opt_len, unsigned char *opt_data, unsigned char *ret_option, int *ropt_len );
 unsigned char CableCardCallbackSocketFlowConfig(unsigned long id, unsigned char *opt_data,
													   unsigned long opt_len);
 unsigned char CableCardCallbackDeleteFlowReq(unsigned long flowid);
 void CableCardCallbackSendIPUData( unsigned char *pdata, unsigned long len );
 void CableCardCallbackSendSocketFlowUsData( unsigned long id, unsigned char *opt_data, unsigned long opt_len );
 void CableCardRemovedCallbackCleanUp( void );

}

#endif

static void cablecard_error(B_MPOD_IF_ERROR error);
static void cablecard_reset(void);
static void OOB_Tx_Docsis(void);
static void page_cleanup();
void removeKeyCb(
    uint16_t programNumber,
    uint8_t ltsid
    );

CableCARDInfo * CCablecard::cablecard_get_info()
{
	return &info;
}
static void b_reset_mpod_route(NEXUS_MpodHandle mpod)
{

	NEXUS_MpodInputSettings settings;
	unsigned pr_band;
	NEXUS_MpodInputHandle mpod_input;

	NEXUS_MpodInput_GetDefaultSettings(&settings);
	settings.bandType = NEXUS_MpodBandType_eParserBand;
	settings.allPass = false;

	for ( pr_band = 0; pr_band < MAX_CABLECARD_ROUTE; pr_band++)
	{
		settings.band.parserBand = (NEXUS_ParserBand)pr_band;
#ifdef SOFTWARE_RS_MPOD
		mpod_input = B_Mpod_InputOpen(mpod, &settings);
		B_Mpod_InputClose(mpod_input);
#else
		mpod_input = NEXUS_MpodInput_Open(mpod, &settings);
		NEXUS_MpodInput_Close(mpod_input);
#endif
	}

}

static void MpegFlowReqThread(
    void *cntx
    )
{
	cablecard_t cablecard = (cablecard_t)cntx;
	int retval;

	/* allow card to establish sessions */
	BKNI_Sleep(10000);
	while(true)
	{
		retval = B_Event_Wait(MpegFlowHandlerEvent, B_WAIT_NONE);
		if (retval == B_ERROR_SUCCESS)
			break;

		/* if there is no MPEG flow and no outstanding request */
		if( cablecard->cablecard_in && !(MpegFlowId) && !(MpegFlowReq))
		{
			if(B_Mpod_ExtChOpenMpegFlow(0x1ffc) == MPOD_SUCCESS)
			{
				MpegFlowReq = true;
			}
			else
			{
				//BDBG_WRN(("Attempt to open MPEG Flow failed: Card may not be ready\n"));
			}
		}
	}
	B_Event_Destroy(MpegFlowHandlerEvent);
}

CCablecard::CCablecard() :
	CMvcModel("cableCard"),
	cableCardIn(false),
	chMgrUninitialized(false)
{
}

CCablecard::~CCablecard()
{
}
eRet CCablecard::initialize(CTunerOob * pTuner)
{
     eRet ret = eRet_Ok;
    _pTunerOob = pTuner;
#if NEXUS_FRONTEND_DOCSIS
    g_oobHFrontendDeviceHandle = NEXUS_Frontend_GetDevice(_pTunerOob->getFrontend());
#endif

	g_oobHFrontendHandle =_pTunerOob->getFrontend();
    return ret;
}

eRet CCablecard::initializeUpstream(CTunerUpstream * pTuner)
{
     eRet ret = eRet_Ok;
    _pTunerUpstream = pTuner;

	g_upstreamHFrontendHandle =_pTunerUpstream->getFrontend();
    return ret;
}


/* bwin io callback that is triggered when it is safe to handle callbacks */
static void bwinChannelMapUpdateCallback(
        void *       pObject,
        const char * strCallback
        )
{
    eRet           ret          = eRet_Ok;
    CChannelMgr * pChannelMgr = (CChannelMgr *)pObject;

    BDBG_ASSERT(NULL != pChannelMgr);
    BSTD_UNUSED(strCallback);
    ret = pChannelMgr->notifyObservers(eNotify_ChannelMapUpdate, pChannelMgr);
    CHECK_ERROR("error notifying observers", ret);
} /* bwinChannelMapUpdateCallback */

/* bwin io callback that is triggered when it is safe to handle callbacks */
static void bwinChannelScanStartCallback(
        void *       pObject,
        const char * strCallback
        )
{
    eRet           ret          = eRet_Ok;
    CChannelMgr * pChannelMgr = (CChannelMgr *)pObject;

    BDBG_ASSERT(NULL != pChannelMgr);
    BSTD_UNUSED(strCallback);
    ret = pChannelMgr->notifyObservers(eNotify_ScanStart, pChannelMgr->getTunerScanData());
    CHECK_ERROR("error notifying observers", ret);
} /* bwinChannelScanStartCallback */

/* bwin io callback that is triggered when it is safe to handle callbacks */
static void bwinCableCardInCallback(
        void *       pObject,
        const char * strCallback
        )
{
    eRet           ret          = eRet_Ok;
    CChannelMgr * pChannelMgr = (CChannelMgr *)pObject;

    BDBG_ASSERT(NULL != pChannelMgr);
    BSTD_UNUSED(strCallback);
    ret = pChannelMgr->notifyObservers(eNotify_CableCardIn, pChannelMgr);
    CHECK_ERROR("error notifying observers", ret);
} /* bwinCableCardInCallback */

/* bwin io callback that is triggered when it is safe to handle callbacks */
static void bwinCableCardOutCallback(
        void *       pObject,
        const char * strCallback
        )
{
    eRet           ret          = eRet_Ok;
    CChannelMgr * pChannelMgr = (CChannelMgr *)pObject;

    BDBG_ASSERT(NULL != pChannelMgr);
    BSTD_UNUSED(strCallback);
    ret = pChannelMgr->notifyObservers(eNotify_CableCardOut, pChannelMgr);
    CHECK_ERROR("error notifying observers", ret);
} /* bwinCableCardOutCallback */

void cablecard_in(void)
{
	cablecard_t cablecard = cablecard_get_instance();
    CCablecard * param = g_pCableCard;
    CWidgetEngine * pWidgetEngine = g_pChannelMgr->getWidgetEngine();
    /* sync with bwin loop */
    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(g_pChannelMgr, CALLBACK_CABLECARD_IN);
    }
	BDBG_WRN((" CableCard has been plugged in\n"));
	param->bcm_3255_enable_oob_pins();
	cablecard->cablecard_in = true;
	/* initialize channelmgr for SI and scte callbacks */
	g_pChannelMgr->initialize();
	param->cableCardIn = true;
	param->chMgrUninitialized = false;

	if ( (pCableCardChannels[0] == NULL) && ( param->_pModel->getCurrentChannel() != NULL) ) {
		param->cablecard_route_add_tuner(cablecard, param->_pModel->getCurrentChannel());
	}
}

void cablecard_out(void)
{
	cablecard_t cablecard = cablecard_get_instance();
    CCablecard * param = g_pCableCard;
	CWidgetEngine * pWidgetEngine = g_pChannelMgr->getWidgetEngine();
	BDBG_WRN((" CableCard has been plugged out\n"));
	cablecard->cablecard_in = false;
 	cablecard_reset();
    param->bcm_3255_tristate_oob_pins();
	g_pChannelMgr->unInitialize();
	param->chMgrUninitialized = true;
	if (param->_pChannel) {
	  param->cablecard_route_remove_tuner(cablecard, param->_pChannel);
	}
	else {
	  param->cablecard_route_remove_tuner(cablecard, param->_pModel->getCurrentChannel());
	}
    /* sync with bwin loop */
    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(g_pChannelMgr, CALLBACK_CABLECARD_OUT);
    }
}

cablecard_t CCablecard::cablecard_open(cablecard_setting_t setting, CChannelMgr * pChannelMgr, CModel *  pModel)
{
	int pod, i;
	cablecard_t cablecard = &g_cablecard;
    B_MPOD_IF_SETTINGS IfSettings;
	NEXUS_MpodOpenSettings Mpodsetting;
	#ifdef DSG_SUPPORT
	pthread_t dsgcc_thread;
	#endif
	g_pChannelMgr = pChannelMgr;
	_pModel = pModel;
    CWidgetEngine * pWidgetEngine = pChannelMgr->getWidgetEngine();

    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->addCallback(pChannelMgr, CALLBACK_CABLECARD_IN, bwinCableCardInCallback);
        pWidgetEngine->addCallback(pChannelMgr, CALLBACK_CABLECARD_OUT, bwinCableCardOutCallback);
        pWidgetEngine->addCallback(pChannelMgr, CALLBACK_CHANNEL_MAP_UPDATE, bwinChannelMapUpdateCallback);
        pWidgetEngine->addCallback(pChannelMgr, CALLBACK_CHANNEL_SCAN_START, bwinChannelScanStartCallback);
    }

	memset(cablecard, 0, sizeof(struct Cablecard));
	cablecard->oob_mode = setting->oob_mode;
	cablecard->us_mode = setting->us_mode;

	/* open POD driver device*/
    pod = open(CABLECARD_DEVICE, O_RDWR);
    if (pod < 0) {
        BDBG_ERR(("Unable to open cablecard device: %s\n", CABLECARD_DEVICE));
        return NULL;
    }
	cablecard->pod = pod;
#ifdef DSG_SUPPORT
	printf("Lauching dsgcc-thread in library\n");
	pthread_create(&dsgcc_thread, NULL, (void *(*)(void *))(InitializeDsgCcApplication), NULL);
#endif

	/* open MPOD XPT module*/
    NEXUS_Mpod_GetDefaultOpenSettings(&Mpodsetting);
    Mpodsetting.mpodMode = NEXUS_MpodMode_eMpod;
    Mpodsetting.bandType = NEXUS_MpodBandType_eParserBand;

	Mpodsetting.byteSync = 1;
	Mpodsetting.clockDelay = 1;
	Mpodsetting.invertClock = 0;

    cablecard->mpod = NEXUS_Mpod_Open(0, &Mpodsetting);
	if ( cablecard->mpod == NULL)
	{
        BDBG_ERR(("Unable to open NEXUS mpod "));
		return NULL;
	}

	b_reset_mpod_route(cablecard->mpod );

    for(i=0;i<MAX_CABLECARD_ROUTE;i++)
    {
        pCableCardChannels[i] = NULL;
        keySlotRemoved[i] = NULL;
        keySlotRemoved[i] = B_Event_Create(NULL);
        if(!keySlotRemoved[i])
        {
            break;
        }
    }
    if(i < MAX_CABLECARD_ROUTE)
    {
         BDBG_ERR(("keySlot event creation failed"));
         return NULL;
    }

	IfSettings.cardInsertedCb = cablecard_in;
	IfSettings.cardRemovedCb = cablecard_out;
	IfSettings.cardErrorCb = cablecard_error;
	IfSettings.cardResetCb = cablecard_reset;
	B_Mpod_Init(&IfSettings);

	/* Initializing MPOD stack */
	cablecard_init();
	B_Mpod_TestCpInit();
	MpegFlowId = 0;
	MpegFlowReq = false;

	MpegFlowHandlerEvent = B_Event_Create(NULL);
	if((MpegFlowHandler = B_Thread_Create("MpegFlowReqThread", MpegFlowReqThread, (void *)cablecard, NULL)) == NULL)
	{
		BDBG_ERR(("Unable to create MpegFlowReqThread\n"));
		exit(1);
	}

	/* clear all Card related Info*/
	memset(&info, 0, sizeof(info));
	info.openedResrc = B_MPOD_RESRC_FEATURE_CTL_ID;

	CableCardRemovedCallbackCleanUp();
	bcm_3255_tristate_oob_pins();

    /* Maintain global object for now to access from friend methods */
	g_pCableCard = this;
	return cablecard;
}


void CCablecard::cablecard_close()
{
	int i;
	cablecard_t cablecard = cablecard_get_instance();
    CWidgetEngine * pWidgetEngine = g_pChannelMgr->getWidgetEngine();
	if (cablecard == NULL) return;

	if (cableCardIn && !chMgrUninitialized)
		g_pChannelMgr->unInitialize();
	B_Event_Set(MpegFlowHandlerEvent);
	B_Thread_Destroy(MpegFlowHandler); /* implicitly waits for termination */

	if (cablecard->mpod)
	{
		for (i=0; i<MAX_CABLECARD_ROUTE; i++)
		{
			if ( NULL != cablecard->mpod_input[i])
				NEXUS_MpodInput_Close(cablecard->mpod_input[i]);
#if 0 /*temp remove*/
			removeKeyCb(pg_info[i].program_number, pg_info[i].ltsid);
#endif
		}
		NEXUS_Mpod_Close(cablecard->mpod);
	}

	if (cablecard->pod) {
		B_Mpod_Done(cablecard->pod);
		B_Mpod_TestCpDone();
		B_Mpod_AppInfoShutdown();
		B_Mpod_MmiShutdown();
		B_Mpod_SasShutdown();
		B_Mpod_DownloadShutdown();
		B_Mpod_FeatureShutdown();
		B_Mpod_DsgShutdown();
		B_Mpod_HostControlShutdown();
		B_Mpod_DiagsShutdown();
        B_Mpod_CaShutdown();
		B_Mpod_CpShutdown();
        B_Mpod_HeadendCommShutdown();
		B_Mpod_SnmpShutdown();
		B_Mpod_HostPropertiesShutdown();
		B_Mpod_SystimeShutdown();
		B_Mpod_HomingShutdown();
		B_Mpod_ExtendedChShutdown();

		close(cablecard->pod);
	}

	if (cablecard->mpeg_section)
		BKNI_Free(cablecard->mpeg_section);

	cablecard->mpeg_section = NULL;
	cablecard->cablecard_in = false;
    for(i=0;i<MAX_CABLECARD_ROUTE;i++)
    {
        B_Event_Destroy(keySlotRemoved[i]);
    }
	CableCardRemovedCallbackCleanUp();
	bcm_3255_tristate_oob_pins();

    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->removeCallback(this, CALLBACK_CHANNEL_MAP_UPDATE);
        pWidgetEngine->removeCallback(this, CALLBACK_CHANNEL_SCAN_START);
        pWidgetEngine->removeCallback(this, CALLBACK_CABLECARD_IN);
        pWidgetEngine->removeCallback(this, CALLBACK_CABLECARD_OUT);
    }
}

/*
Summary:
   Enable routing tuner's transport output to CableCARD.
Description:
*/
int CCablecard::cablecard_route_add_tuner(cablecard_t cablecard, CChannel *pChannel)
{
    NEXUS_MpodInputSettings settings;
	int input_index = 0;
	CParserBand * pBand;

    pBand = pChannel->getParserBand();

    if (cablecard == NULL) return -1;

	NEXUS_MpodInput_GetDefaultSettings(&settings);
	settings.bandType = NEXUS_MpodBandType_eParserBand;
	settings.allPass = false;
	settings.band.parserBand = pBand->getBand();

	/*find the first NULL spot*/
	while(cablecard->mpod_input[input_index]){
		input_index ++;
		if(input_index >= MAX_CABLECARD_ROUTE) goto error_index;
	}
	{
#ifdef SOFTWARE_RS_MPOD
		cablecard->mpod_input[input_index] = B_Mpod_InputOpen(cablecard->mpod, &settings);
#else
		cablecard->mpod_input[input_index] = NEXUS_MpodInput_Open(cablecard->mpod, &settings);
#endif
		if (cablecard->mpod_input[input_index] == NULL) return -1;
	}
	return 0;

error_index:
	BDBG_WRN((" cannot find routing resource"));

	return -1;
}

/*
Summary:
   Disable routing tuner's transport output to CableCARD.
Description:
*/
int CCablecard::cablecard_route_remove_tuner(cablecard_t cablecard,  CChannel *pChannel)
{
	NEXUS_ParserBand pr_band;
	NEXUS_MpodInputSettings settings;
	int input_index = 0;

	CParserBand * pBand = pChannel->getParserBand();

	if (cablecard == NULL) return -1;

	pr_band = pBand->getBand();
	while(input_index < MAX_CABLECARD_ROUTE){
		if(cablecard->mpod_input[input_index]){
			NEXUS_MpodInput_GetSettings(cablecard->mpod_input[input_index], &settings);
			BDBG_MSG(("\n close MPOD input settings pr_band 0x%x, frontend pr_band %d\n", settings.band.parserBand, pr_band));
		}
		if (settings.band.parserBand == pr_band)
		{
#ifdef SOFTWARE_RS_MPOD
			B_Mpod_InputClose(cablecard->mpod_input[input_index]);
#else
			BDBG_MSG(("\n close MPOD input 0x%x, input_index %d\n", cablecard->mpod_input[input_index], input_index));
			NEXUS_MpodInput_Close(cablecard->mpod_input[input_index]);
#endif
			cablecard->mpod_input[input_index] = NULL;
			break;
		}
		input_index++;
		if(input_index == MAX_CABLECARD_ROUTE)
			BDBG_WRN((" traverse the array but not found, shouldn't be"));
	}
	return 0;
}

int CCablecard::cablecard_go(cablecard_t cablecard)
{
	/* start POD detection thread*/
	B_Mpod_Go(cablecard->pod);
	return 0;
}

cablecard_t cablecard_get_instance(void)
{
	return (&g_cablecard);
}

int CCablecard::cablecard_get(cablecard_t cablecard, cablecard_setting_t setting)
{
    if (cablecard == NULL || setting == NULL) return -1;
	setting->oob_mode = cablecard->oob_mode;
	setting->us_mode = cablecard->us_mode;
	return 0;
}

int CCablecard::cablecard_set(cablecard_t cablecard, cablecard_setting_t setting)
{
    if (cablecard == NULL|| setting == NULL) return -1;
	cablecard->oob_mode = setting->oob_mode;
	cablecard->us_mode = setting->us_mode;
	return 0;
}



int CCablecard::cablecard_inquire_program(cablecard_t cablecard, CChannel *pChannel)
{
    BSTD_UNUSED(pChannel);
    BSTD_UNUSED(cablecard);
   #if 0
	MPOD_RET_CODE	ret;

	if (cablecard == NULL) return -1;
	BDBG_MSG((" inquire program ltsid %d PG num %d video_pid 0x%x", info->ltsid, info->program_number, info->video_pid));
	ret = B_Mpod_CaSendPmt((unsigned char *)info->pmt, B_MPOD_CA_QUERY, info->program_index, info->ltsid, info->source_id
				 );
    return (ret != MPOD_SUCCESS);
    #endif
    return 0;
}

int CCablecard::cablecard_enable_program(CChannel *pChannel)
{
	MPOD_RET_CODE	ret;
    CParserBand * pBand = pChannel->getParserBand();
    unsigned ltsid=0;
    unsigned programIndex=0;
    unsigned programNumber=0;
    unsigned sourceId=0;
    PROGRAM_INFO_T programInfo;
	cablecard_t cablecard = cablecard_get_instance();
	_pChannel = pChannel;

    if (!g_cablecard.cablecard_in)
    {
        BDBG_WRN(("cable card not inserted yet"));
        return -1;
    }
	cablecard_route_add_tuner(cablecard, pChannel);
    for(programIndex=0;programIndex<MAX_CABLECARD_ROUTE;programIndex++)
    {
        if (!pCableCardChannels[programIndex])
            break;
    }
    if(programIndex >= MAX_CABLECARD_ROUTE)
    {
         BDBG_ERR(("channels added to cable card exceeded the max limit "));
         return -1;
    }

    pCableCardChannels[programIndex] = pChannel;
    sourceId = pChannel->getSourceId();
    programNumber = pChannel->getProgramNum();
    memset(channelPmtInfo[programIndex].pmt,0,sizeof(channelPmtInfo[programIndex].pmt));
    channelPmtInfo[programIndex].pmtSize = 0;
	tsPsi_setTimeout(800,800);
	memset(&programInfo, 0, sizeof(PROGRAM_INFO_T));
    tsPsi_getProgramInfo(&programInfo,programNumber, pBand->getBand(),
                         channelPmtInfo[programIndex].pmt,
                         &channelPmtInfo[programIndex].pmtSize );
    if(!channelPmtInfo[programIndex].pmtSize)
    {
        BDBG_ERR(("%s: not PMT found",BSTD_FUNCTION));
        goto error;
    }
    ltsid = pBand->getBand();
    BDBG_WRN(("%s programIndex %u ltsid %u sourceId %u programNumber %u caPid %u pmtSize %u",
              BSTD_FUNCTION,programIndex,ltsid, sourceId, programNumber, programInfo.ca_pid,
              channelPmtInfo[programIndex].pmtSize));
    ret = B_Mpod_CaSendPmt((unsigned char *)channelPmtInfo[programIndex].pmt,
                           B_MPOD_CA_OK_DESCRAMBLE, programIndex, ltsid, sourceId);
	if (ret == MPOD_SUCCESS)
    {
		B_Mpod_CpAddProgram(programIndex,programNumber, programInfo.ca_pid,ltsid);
	}
    return 0;
error:
    pCableCardChannels[programIndex] = NULL;
    return 0;
}


int CCablecard::cablecard_disable_program(class CChannel *pChannel)
{
	MPOD_RET_CODE	ret;
    CParserBand * pBand = pChannel->getParserBand();
    unsigned ltsid=0;
    unsigned programIndex=0;
    unsigned programNumber=0 ;
    unsigned sourceId=0;

	cablecard_t cablecard = cablecard_get_instance();
	_pChannel = pChannel;

	cablecard_route_remove_tuner(cablecard, pChannel);

    for(programIndex=0;programIndex<MAX_CABLECARD_ROUTE;programIndex++)
    {
        if (pCableCardChannels[programIndex] == pChannel)
            break;
    }

    if(programIndex >= MAX_CABLECARD_ROUTE)
    {
         BDBG_ERR(("channel not available"));
         return -1;
    }

    sourceId = pChannel->getSourceId();
    programNumber = pChannel->getProgramNum();
    ltsid = pBand->getBand();

    BDBG_WRN(("%s programIndex %u ltsid %u sourceId %u programNumber %u pmtSize %u",
              BSTD_FUNCTION,programIndex,ltsid, sourceId, programNumber,channelPmtInfo[programIndex].pmtSize));
	ret = B_Mpod_CaSendPmt((unsigned char *)channelPmtInfo[programIndex].pmt,
                            B_MPOD_CA_NOT_SELECTED, programIndex, ltsid, sourceId);
	if (ret == MPOD_SUCCESS)
	{
       B_Mpod_CpRemoveProgram(programIndex);
	}

#ifdef NEXUS_HAS_SECURITY
    if(pChannel->getKeySlot())
    {
        B_Event_Wait(keySlotRemoved[programIndex],1000);
    }
#endif

    pCableCardChannels[programIndex] = NULL;
    return 0;
}


int CCablecard::cablecard_set_mpeg_section_callback(cablecard_t cablecard, cablecard_mpeg_callback callback)
{
    if (cablecard == NULL) return -1;

    if (!cablecard->mpeg_section) {
		cablecard->mpeg_section = (unsigned char *)BKNI_Malloc(MAX_MPEG_SECTION_LEN);
        cablecard->mpeg_section_len = 0;
    }
    cablecard->si_callback = callback;

	return 0;
}


int cablecard_get_mpeg_section(cablecard_t cablecard, void *buffer, size_t size)
{
	if (cablecard == NULL) return -1;

    if (cablecard->mpeg_section) {
        cablecard_lock();
        size = (cablecard->mpeg_section_len > size)? size : cablecard->mpeg_section_len;
        BKNI_Memcpy(buffer, cablecard->mpeg_section, size);
        cablecard_unlock();
        return size;
    }

    return -1;
}

#if NEXUS_FRONTEND_DOCSIS

int CCablecard::bcm_3255_tristate_oob_pins()
{
#if defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
    return NEXUS_Docsis_EnableCableCardOutOfBandPins(g_oobHFrontendDeviceHandle, false);
#else
    return -1;
#endif
}

int CCablecard::bcm_3255_enable_oob_pins()
{
#if defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
    return NEXUS_Docsis_EnableCableCardOutOfBandPins(g_oobHFrontendDeviceHandle, true);
#else
    return -1;
#endif
}

#elif NEXUS_FRONTEND_3255
int CCablecard::bcm_3255_tristate_oob_pins()
{
#if defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
#if (BCHP_CHIP == 7425 || BCHP_CHIP == 7429 || BCHP_CHIP == 7435 || BCHP_CHIP == 7445)
	NEXUS_3255DeviceGpioPinSettings settings;
	settings.mode = NEXUS_GpioMode_eOutputOpenDrain;
	return NEXUS_Frontend_Set3255DeviceGpioPinSettings(g_oobHFrontendHandle, NEXUS_3255DeviceGpioPin_eOob, &settings);
#else
	NEXUS_3255GpioPinSettings settings;
	settings.mode = NEXUS_GpioMode_eOutputOpenDrain;
	return NEXUS_Frontend_3255_SetGpioPinSettings(g_oobHFrontendHandle, NEXUS_3255GpioPin_eOob, &settings);
#endif
#endif
}

int CCablecard::bcm_3255_enable_oob_pins()

{
#if defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
#if (BCHP_CHIP == 7425 || BCHP_CHIP == 7429 || BCHP_CHIP == 7435 || BCHP_CHIP == 7445)
	NEXUS_3255DeviceGpioPinSettings settings;
	settings.mode = NEXUS_GpioMode_eOutputPushPull;
	return NEXUS_Frontend_Set3255DeviceGpioPinSettings(g_oobHFrontendHandle, NEXUS_3255DeviceGpioPin_eOob, &settings);
#else
	NEXUS_3255GpioPinSettings settings;
	settings.mode = NEXUS_GpioMode_eOutputPushPull;
	return NEXUS_Frontend_3255_SetGpioPinSettings(g_oobHFrontendHandle, NEXUS_3255GpioPin_eOob, &settings);
#endif
#endif
}
#else
int CCablecard::bcm_3255_tristate_oob_pins()
{
	return 0;
}

int CCablecard::bcm_3255_enable_oob_pins()
{
	return 0;
}
#endif

void cablecard_error(
    B_MPOD_IF_ERROR error
    )
{
	char strings[][256] = {"B_MPOD_IF_ERROR_CARD_UNKNOWN","B_MPOD_IF_ERROR_CARD_RCV","B_MPOD_IF_ERROR_CARD_TX"};
	BDBG_ASSERT(error < B_MPOD_IF_ERROR_CARD_MAX);

	BDBG_WRN((" CableCard Error %s\n", strings[error]));
}


void cablecard_reset(void)
{
    unsigned int  cdl_group_id = 0;
    CableCardRemovedCallbackCleanUp();
	memset(&info, 0, sizeof(info));
#if 0
	channel_mgr_reset_map();
#endif
	page_cleanup();
#ifdef ESTB_CFG_SUPPORT
	B_Estb_cfg_Set_uint32((char*)"/dyn/estb/group_id", cdl_group_id);
#endif
}
void CCablecard::getProperties(
        MStringHash * pProperties,
        int page,
        bool bClear
        )
{
    if (true == bClear)
    {
        pProperties->clear();
    }
	char str[101];
	if (page == 1)
	{
		pProperties->clear();
		if (info.card_mmi_message1 != NULL)
			{
			snprintf(str, 101, "%s", info.card_mmi_message1);
			pProperties->add("1st line", str);
			}
		if (info.card_mmi_message2 != NULL)
			{
			snprintf(str, 101, "%s", info.card_mmi_message2);
			pProperties->add("2nd line", str);
			}
		if (info.card_mmi_message3 != NULL)
			{
			snprintf(str, 101, "%s", info.card_mmi_message3);
			pProperties->add("3rd line", str);
			}
		if (info.card_mmi_message4 != NULL)
			{
			snprintf(str, 101, "%s", info.card_mmi_message4);
			pProperties->add("4th line", str);
			}
	}
} /* getProperties */

int CCablecard::cablecard_set_dsgtunnel_handler(cablecard_t cablecard,  cablecard_dsg_handler  *dsg_handler)
{
	if (cablecard->num_of_dsg_handler >= MAX_DSGTUNNEL_HANDLER) return -1;
	cablecard->dsg_handler[cablecard->num_of_dsg_handler++] = *dsg_handler;
	return 0;
}

unsigned char BcmSendDSGTunnelDataToHost( unsigned char *pBufData, unsigned int pktlen, unsigned long client_type, unsigned long client_id)
{

	cablecard_t cablecard = cablecard_get_instance();
	unsigned short     ether_type, GutsLen;
	unsigned char     *pFirstBuffData = pBufData;
	int i;

	if (cablecard == NULL) return -1;

   /*
     * Only forward these types (0800,0806) to TCPIP all others return.
     */
    ether_type = ((unsigned char *)pFirstBuffData)[12];
    ether_type <<= 8;
    ether_type |= ((unsigned char *)pFirstBuffData)[13];

    /*
     * discard non-IP ehternet packet data
     */
    if (ether_type != 0x0800) {
        printf("err1, ehter_type=%04x(%04x)\n", ether_type, 0x0800);
        return -1;
    }

    /*
     * Passed IP checking, now the protocol type is farther in to the packet.
     * Evaluate it to make checking for UDP.
     */
    if (((unsigned char *)pFirstBuffData)[14 + 9] != 17/*IPPROTO_UDP*/) {
        printf("err2, Not UDP packet=%02x(%02x)\n", ((unsigned char *)pFirstBuffData)[14 + 9], 17/*IPPROTO_UDP*/ );
        return -1;
    }

    /*
     * get the UDP packet length, subtract 8 for UDP header
     */
    GutsLen = ((unsigned char *)pFirstBuffData)[14 + 20 + 4];
    GutsLen <<= 8;
    GutsLen |= ((unsigned char *)pFirstBuffData)[14 + 20 + 5];
    pktlen = GutsLen - 8;

    /*
     * offset to start of UDP data
     */
    pBufData = pFirstBuffData + 14 + 20 + 8;

	// we have NOT seen any HE to send out new Broadcasting tunnels with BT header
	if (getenv("BT_HEADER") != NULL) {
		if (client_type == DSG_CLIENT_TYPE_BROADCAST) {
			/* TODO:: process BT header. Skip now*/
    		pBufData += 4;
    		pktlen -= 4;
		}
	}

	if (pktlen <= 0) {
		printf(" error in DSG tunnel data %d\n", pktlen);
		return -1;
	}

	// first dispatch the DSG tunnel to registered external handlers
	for (i=0;i<cablecard->num_of_dsg_handler;i++)
	{
		if (cablecard->dsg_handler[i].callback != NULL
			&& client_id == cablecard->dsg_handler[i].client_id
			&& client_type == cablecard->dsg_handler[i].client_type)
		{
			(*cablecard->dsg_handler[i].callback)(pBufData, pktlen);
			return 0;
		}
	}


	if (client_type == DSG_CLIENT_TYPE_BROADCAST) {
		BDBG_MSG((" Got Broadcast DSG tunnel!\n"));

		switch ( client_id ) {
			case DSG_CLIENT_BROADCAST_TYPE_SCTE65:
				BDBG_MSG((" Got SCTE 65 DSG tunnel! \n"));
		    	if (cablecard->mpeg_section) {
        			cablecard_lock();
        			cablecard->mpeg_section_len = (pktlen > MAX_MPEG_SECTION_LEN)? MAX_MPEG_SECTION_LEN : pktlen;
        			BKNI_Memcpy(cablecard->mpeg_section, pBufData, cablecard->mpeg_section_len);
        			cablecard_unlock();
    			}
				/* This callback will pass SI data (MPEG2 section) to SI parsing library*/
				if (cablecard->si_callback)
			    	(*cablecard->si_callback)(cablecard, cablecard->mpeg_section_len);
				break;
			case DSG_CLIENT_BROADCAST_TYPE_SCTE18:
				BDBG_MSG((" Got SCTE 18 DSG tunnel! \n"));
				break;
			case DSG_CLIENT_BROADCAST_TYPE_XAIT_CVT:
				BDBG_MSG((" Got CVT or XAIT DSG tunnel! ID 0x%2x\n", pBufData[0]));
#ifdef CDL_SUPPORT
				/* CVT: table ID is 0xd9; XAIT: table ID is 0x74*/
				/*fixme: how to tell it's CVT or XAIT? */
				/* debug only, use a cvt captured in file */
#if 0
				{
					int fp;
					char fn_cvt[128] ="./signed_cvt_capture.bin";
					if ((fp  = open(fn_cvt, O_RDONLY)) < 0) {
						BDBG_ERR(("cannot open %s", fn_cvt));
					} else {
						pktlen = read(fp, pBufData, pktlen);
						BDBG_ERR(("cvt pktlen %d bytes", pktlen));
						close(fp);
					}
				}
#endif
                if (pBufData[0] == 0xd9) {
                    BDBG_ERR(("pktlen %d", pktlen));
					cdl_dsg_cvt_process(NULL, pBufData, pktlen);
                }
#endif
				break;
 		case DSG_CLIENT_BROADCAST_TYPE_OCCD:
				BDBG_MSG((" Got OCAP Comon Download DSG tunnel!"));
				break;
		case DSG_CLIENT_BROADCAST_TYPE_OBJECT_CAROUSEL:
				BDBG_MSG((" Got Object Carousel DSG tunnel!"));
				break;
			default:
				BDBG_MSG((" Got unknown type Broadcast tunnel!"));
				break;
		}
	} else if (client_type ==  DSG_CLIENT_TYPE_APPLICATION ){
		BDBG_MSG((" Got Application type DSG tunnel!\n"));
	} else if (client_type ==  DSG_CLIENT_TYPE_WELLKNOWN_MACADDR){
		BDBG_MSG((" Got Well-Known Macaddr  DSG tunnel!  \n"));
	} else if (client_type ==  DSG_CLIENT_TYPE_CAS){
		BDBG_WRN((" Got CA DSG tunnel! Should NOT be terminated at Host!\n"));
	} else {
		BDBG_WRN((" Got unknown type DSG tunnel!\n"));
	}

    return 0;
}

unsigned char BcmSendDSGTunnelDataToPOD( unsigned char *pBufData, unsigned int pktlen, unsigned long flow_id )
{
	unsigned char *ptr=NULL, ret_val;

	BDBG_MSG((" BcmSendDSGTunnelDataToPOD len %d  flowID 0x%x", pktlen, flow_id));

	if (pktlen>=65535)
	{
		printf("too big %d\n", pktlen);
		return 1; /* Change the return value */
	}

	/* Need to stick 2 bytes of length count infront of the packet per Opencable spec...
	 The buffer will be Queued and sent to cablecard in link layer Send thread
	 The buffer will be freed in link layer.
	 So, have to allocate buffer and copy the entire packet.
	 Not the best solution in town, will optimize later.*/
	ptr = (unsigned char*)BKNI_Malloc(pktlen+4);
	if( ptr )
		BKNI_Memcpy(ptr+2, pBufData, pktlen);
	else
		BDBG_ASSERT((0));

	/* add 2 byte length */
	*ptr = pktlen >> 8;
	*(ptr+1) = pktlen & 0xff;

    ret_val = (unsigned char) B_Mpod_LinkH2CSendExtData(flow_id, pktlen+2, ptr);
	if( ret_val != 0 )
		printf("\n!CableCard Not Rdy! RX DSG tunnel packet with MacAddr=0x%08lx_%04x, pktlen=%d\n", *(unsigned long *)(pBufData), *(unsigned short *)(pBufData+4), pktlen );

	/* free(ptr); free at link layer  */

    return ret_val;

}


/*for IP-U and socket flow*/
unsigned char BcmSendDataToPOD( unsigned char *pBufData, unsigned int pktlen, unsigned long flow_id )
{

	unsigned char *ptr=NULL, ret_val;

	if (pktlen>=65535)
	{
		printf("too big %d\n", pktlen);
		return 1;
	}

	BDBG_MSG((" BcmSendDataToPOD len %d  flowID 0x%x", pktlen, flow_id));

/*	// Need to have a check here if the CableCard is present/absent,
	// if cablecard is not present, no need to waste time copy packet, just return
	// if( CableCard is not present )
	// 	return POD_NOT_READY;

    // No Need to stick 2 bytes of length count infront of the packet per Opencable spec...
    // The buffer will be Queued and sent to cablecard in link layer Send thread
    // The buffer will be freed in link layer.
    // So, have to allocate buffer and copy the entire packet.
    // Not the best solution in town, will optimize later.*/
    ptr =(unsigned char*)BKNI_Malloc(pktlen);
    if( ptr )
        BKNI_Memcpy (ptr, pBufData, pktlen);
    else
        return -1;

	/*printf("************** BcmSendDataToPOD **************\n"); */

    ret_val = (unsigned char) B_Mpod_LinkH2CSendExtData(flow_id, pktlen, ptr);
	if( ret_val )
		printf("\n!CableCard Not Rdy! RX DSG tunnel packet with MacAddr=0x%08lx_%04x, pktlen=%d\n", *(unsigned long *)(pBufData), *(unsigned short *)(pBufData+4), pktlen );

	/* free(ptr); free at link layer  */
	return ret_val;

}


void POD_Api_Lost_Flow_Ind(unsigned long id,unsigned char status)
{
    BDBG_MSG(("\nsent to Cablecard: lost_flow_ind id=0x%x, status=%d!\n", id, status));
    B_Mpod_ExtChLostFlow(id, (B_MPOD_EXT_LOST_FLOW_REAS)status);
}


void POD_Api_Send_DCD_Info(void *dcd_ptr, unsigned short dcd_len)
{
    /* skip first three bytes to comform latest CCIF spec*/
	/* TODO:: wait for DSG-CC final fix*/
	if (info.extSession > 4)
	{
    	B_Mpod_DsgSendDcdInfo((uint8_t*)dcd_ptr, (uint32_t)dcd_len);
	} else 	if (info.extSession) {
		B_Mpod_ExtChSendDcdInfo((uint8_t*)dcd_ptr, (uint32_t)dcd_len);
	} else
		BDBG_ERR((" both DSG and extended channel have NOT opened yet"));
}
void POD_Api_Send_DSG_Message(void *dsg_message_ptr, unsigned short dsg_message_len)
{
	if (info.extSession > 4)
	{
    	B_Mpod_DsgSendDSGMessage((uint8_t *)dsg_message_ptr,(uint32_t)dsg_message_len);
	} else if (info.extSession) {
		B_Mpod_ExtChSendDSGMessage((uint8_t *)dsg_message_ptr,(uint32_t)dsg_message_len);
	} else
		BDBG_ERR((" both DSG and extended channel have NOT opened yet"));
}

/* TODO: Temporarily removed all APIs for DSG hookup */
/* For the DSG and Ext Channel Settings */

void CableCardCallback_DSG_Packet_Error(uint8_t *data, uint32_t len)
{
    BSTD_UNUSED(len);
	CableCardCallbackDSGPacketError((unsigned char)*data);
}

void CableCardCallback_ExtSet_DSG_Mode(uint8_t *data, uint32_t len)
{
	info.mode = (B_MPOD_EXT_OP_MODE)data[0];
	info.extSession = 4;

	/* HPNX pro send us basic DSG mode without mac_addr*/
	if ( (data[0] == B_MPOD_EXT_DSG || data[0] == B_MPOD_EXT_DSG_ONE_WAY) && len == 1) return;

	/* if switch to DSG mode, we need to switch docsis Upstream mode*/
	if ( data[0] != B_MPOD_EXT_OOB) OOB_Tx_Docsis();

	CableCardCallbackSetDSGMode((unsigned char *)data, (unsigned short)len);
}

void CableCardCallback_DSGSet_DSG_Mode(uint8_t *data, uint32_t len)
{
	info.mode = (B_MPOD_EXT_OP_MODE)data[0];
	info.extSession = 5;

	/* HPNX pro send us basic DSG mode without mac_addr*/
	if ( (data[0] == B_MPOD_EXT_DSG || data[0] == B_MPOD_EXT_DSG_ONE_WAY) && len == 1) return;

	/* if switch to DSG mode, we need to switch docsis Upstream mode*/
	if ( data[0] != B_MPOD_EXT_OOB) OOB_Tx_Docsis();

	CableCardCallbackSetDSGMode((unsigned char *)data, (unsigned short)len);
	}


void CableCardCallback_DSG_Directory(uint8_t *data, uint32_t len)
{
	int vct_id_included, vct_id = 0;

	unsigned char *ptr = data;

	/* get VCT id if possible*/
    vct_id_included = (*ptr++ & 0x01);

	if (vct_id_included) {
		vct_id = (data[len-2]<<8)|data[len-1];

		BDBG_MSG((" VCI_ID included in DSG_directory %d", vct_id));

		info.vctId = vct_id;
		/*TODO:: notify application for SI data filtering*/
	}

	CableCardCallbackDSG_Directory((unsigned char *)data, (unsigned short)len);
}


void CableCardCallback_IPU_Dhcp(uint32_t flowId, uint8_t *data, uint32_t len)
{
	unsigned char status = 0, flowtype = 0x00, flags = 1;
	unsigned short max_pdu_size = 1500;
	unsigned long ipaddr = 0xc0a80001;
    unsigned char *ropt_data = NULL;
	int ropt_len = 128;	/*Guessing: max return option length*/
    uint8_t *a = &(data[1]);
    unsigned int optLen = data[7];
    unsigned char *optBytes = &(data[8]);

    BDBG_MSG(("MacAddr: %02x:%02x:%02x:%02x:%02x:%02x, option length: %d",
                  a[0],a[1],a[2],a[3],a[4],a[5], optLen));

	memcpy(info.macaddr, a, 6);
	if( len == (optLen + 8) && optLen && optBytes )
	{
		printf("\n dhcp option field (hex):");
		for(ipaddr=0; ipaddr<optLen; ipaddr++ )
		{
			if( (ipaddr % 32) == 0 )
				printf("\n");
			printf(" %02x", optBytes[ipaddr] );
		}
		printf("\n");
	}
	else {
		optLen = 0;
		printf(" No dhcp option field!");
	}

	ropt_data = (unsigned char *)malloc( ropt_len );
	if( ropt_data == NULL )
	{
		printf("Error! Cannot allocate space for dhcp option data\n");
		return;
	}

	/*Call Host to send DHCP and WAIT for the response*/
	ipaddr = CableCardCallbackIPUDhcp( flowId, a, optLen, optBytes, ropt_data, &ropt_len );

    ipaddr = htonl(ipaddr);
    BDBG_MSG(("DHCP reply with IPAddress 0x%08lx with option-field data len=%d\n\n", ipaddr, ropt_len));

	/* Network is unavailable*/
	if (ipaddr == 0) {
		status =  B_MPOD_EXT_DEL_FLOW_NETUNAVAIL;
		ropt_len = 0;
	}
	memcpy(info.ipaddr, &ipaddr, 4); //TODO:: ipv6

  	B_Mpod_ExtChIpUnicastFlowCnf( flowId, (B_MPOD_EXT_NEW_FLOW_CNF_STAT)status, (unsigned char *)&ipaddr, flowtype, flags, max_pdu_size, ropt_len, ropt_data);

	/*When this function return, the ropt_data pointer can be freed.*/

	free(ropt_data);
}

B_MPOD_EXT_NEW_FLOW_CNF_STAT CableCardCallback_Req_Ipm_Flow(uint32_t flowId, uint32_t mgid)
{
	/* dummy */
    BSTD_UNUSED(flowId);
    BSTD_UNUSED(mgid);
    return B_MPOD_EXT_NEW_FLOW_GRANTED;
}

B_MPOD_EXT_NEW_FLOW_CNF_STAT CableCardCallback_DSG_Flow_Id(uint32_t flowId)
{
	CableCardCallbackDSGFlowID(flowId); /* For DSG flow */
	return B_MPOD_EXT_NEW_FLOW_GRANTED;
}

void CableCardCallback_Socket_Flow_Config(uint32_t flowId, uint8_t *opt_data, uint32_t opt_len)

{
   	unsigned short max_pdu_size = 1500;
	unsigned char status;

	status = CableCardCallbackSocketFlowConfig(flowId, opt_data, opt_len);
    BDBG_MSG(("-----New_flow_req type Socket: id=%d, opt_len=%d", flowId, opt_len));

 	B_Mpod_ExtChSocketFlowCnf(flowId, (B_MPOD_EXT_NEW_FLOW_CNF_STAT)status, max_pdu_size, opt_len, opt_data);

}
static void CableCardCallback_Flow_Req_Failed(B_MPOD_EXT_SERV_TYPE serviceType, B_MPOD_EXT_NEW_FLOW_CNF_STAT status)
{
    BSTD_UNUSED(status);
	if(MpegFlowReq && (serviceType == B_MPOD_EXT_FLOW_MPEG)) MpegFlowReq = false;
}


static void CableCardCallback_New_Flow_Cnf_Cb(uint32_t flowId, B_MPOD_EXT_SERV_TYPE serviceType, uint16_t pid)
{
	if(MpegFlowReq && (serviceType == B_MPOD_EXT_FLOW_MPEG) &&(pid == 0x1ffc))
	{
		/* keep the following order otherwise need a semaphore */
		MpegFlowId = flowId;
		MpegFlowReq = false;
	}

	BDBG_MSG(("received %s flowId=%d serviceType=%d pid=%d\n", BSTD_FUNCTION, flowId, (uint32_t) serviceType, pid));
}

B_MPOD_EXT_DEL_FLOW_CNF_STAT CableCardCallback_Delete_Flow_Req(uint32_t flowId, B_MPOD_EXT_SERV_TYPE serviceType)
{
	if (serviceType == B_MPOD_EXT_FLOW_IP_U || serviceType == 	B_MPOD_EXT_FLOW_IP_M || serviceType == B_MPOD_EXT_FLOW_SOCKET)
	{
		CableCardCallbackDeleteFlowReq(flowId);
		return B_MPOD_EXT_DEL_FLOW_GRANTED;
	} else
		return B_MPOD_EXT_DEL_FLOW_UNAUTH;
}

static void CableCardCallback_Del_Flow_Cnf_Cb(uint32_t flowId, B_MPOD_EXT_SERV_TYPE serviceType)
{
	/* dummy */
    BSTD_UNUSED(flowId);
    BSTD_UNUSED(serviceType);
    return;
}

static void CableCardCallback_Lost_Flow_Cb(uint32_t flowId, B_MPOD_EXT_SERV_TYPE serviceType, B_MPOD_EXT_LOST_FLOW_REAS reason)
{
    BSTD_UNUSED(serviceType);
    BSTD_UNUSED(reason);
	if((MpegFlowId == flowId)) MpegFlowId = 0;
}

void CableCardCallback_Config_Advanced_DSG(uint8_t *data, uint32_t len)
{
	CableCardCallbackConfig_Advanced_DSG(data, len);
}

void CableCardCallback_Rcv_Flow_Data(uint32_t flowId, B_MPOD_EXT_SERV_TYPE serviceType, uint8_t *data, uint32_t len)
{
	if(serviceType == B_MPOD_EXT_FLOW_IP_U) {
		CableCardCallbackSendIPUData(data, len );
	}
	else if(serviceType == B_MPOD_EXT_FLOW_SOCKET) {
		CableCardCallbackSendSocketFlowUsData( flowId, data, len );
	}
	else if (serviceType == B_MPOD_EXT_FLOW_MPEG) {
		cablecard_t cablecard = cablecard_get_instance();

    	if (cablecard->mpeg_section) {
        	cablecard_lock();
        	cablecard->mpeg_section_len = (len  > MAX_MPEG_SECTION_LEN)? MAX_MPEG_SECTION_LEN : len;
        	BKNI_Memcpy(cablecard->mpeg_section, data, cablecard->mpeg_section_len);
        	BDBG_MSG((" \n**** received service type %d flow data 0x%x, len %d \n", serviceType, data, cablecard->mpeg_section_len ));
        	cablecard_unlock();
    	}
		/* This callback will notify SI parser thread to process new SI data*/
    	if (cablecard->si_callback)
        	(*cablecard->si_callback)(cablecard, cablecard->mpeg_section_len);
	}
	else {
		/* do nothing */
		BDBG_WRN((" unhandled service type flow data %d", serviceType ));
	}
}

static void apInfoInfo_Changed_Cb(
    uint8_t *apInfoLoop,
    uint32_t len
    )
{
	int id;
    BSTD_UNUSED(apInfoLoop);
    BSTD_UNUSED(len);
    BDBG_MSG(("apInfoInfoRdyCb Callback Called\n"));
	B_Mpod_AppInfoGetManuID(&info.vendorId);
	id = ((info.vendorId>>8)&0xff);
	B_Mpod_AppInfoGetVersionNum(&info.version);
	B_Mpod_AppInfoGetSerialNum(info.serialNum, &info.serialLen);
	B_Mpod_AppInfoGetMacAddr(info.macaddr);
	if (info.serialLen == 0)
	{
		memset(info.serialNum, 0, sizeof(info.serialNum));
		memset(info.macaddr, 0, sizeof(info.macaddr));
	}
	/* set correct OOB upstream mode according to card vendor ID*/
	BDBG_MSG((" CableCARD manufacture_id 0x%x",info.vendorId));
	// use symbolrate to automatically set correct OOB mode
#if 0 /*temporarily remove*/
	cablecard_get(cablecard, &s);
	s.us_mode = (id == 0) ? NEXUS_FrontendUpstreamMode_ePodDvs178: NEXUS_FrontendUpstreamMode_ePodDvs167;
	s.oob_mode = (id == 0) ? NEXUS_FrontendOutOfBandMode_ePod_Dvs178Qpsk: NEXUS_FrontendOutOfBandMode_ePod_Dvs167Qpsk;
	BDBG_WRN((" set OOB mode to %s", ((id==0)? "DVS178" : "DVS167")));
	cablecard_set(cablecard,&s);
#endif

}


static uint32_t dialogs[8];
#define MARK_AS_VALID(number)   (dialogs[(number) >> 5] |=  (1 << ((number) & 0x1f)))
#define MARK_AS_INVALID(number) (dialogs[(number) >> 5] &= ~(1 << ((number) & 0x1f)))
#define CHECK_IF_VALID(number)  (dialogs[(number) >> 5] &   (1 << ((number) & 0x1f)))

struct cablecard_message {
	char diag_message[NUM_CABLECARD_LINES][NUM_CABLECARD_CHARS]; /* for the app info recieved when we send the URLs */
	unsigned int diag_message_filled; 	/* no: of lines on the page filled */
};
static cablecard_message cablecard_page;

static void page_cleanup(void)
{
	int j=0;
	for(j=0;j<NUM_CABLECARD_LINES;j++) {
		memset(cablecard_page.diag_message[j], 0, NUM_CABLECARD_CHARS);
	}
}

/* No of lines to be updated */
unsigned int CCablecard::get_num_lines_filled(void){
	return cablecard_page.diag_message_filled;
}

/* To recieve the various App info thro a callback for the URLs sent  */
void cablecard_receive_app_info_query(uint8_t *html, unsigned len, uint8_t dialogNb, uint8_t fileStatus)
{
	int i=0, cnt=0, j=0;
    BSTD_UNUSED(dialogNb);
    BSTD_UNUSED(fileStatus);
	for (i=0;i<NUM_CABLECARD_LINES;i++)
	{
		while (j < NUM_CABLECARD_CHARS-1 && cnt <(int)len)
		{
			if ((html[cnt] != '<') && (html[cnt] != '&') && (html[cnt] != '\n') && (html[cnt] != '\0'))
			{
				cablecard_page.diag_message[i][j++] = html[cnt++];
			} else if (html[cnt] == '<'){
				// check the next three characters
				if(((html[cnt+1] == 'b')||(html[cnt+1] == 'B')) && ((html[cnt+2] == 'r')|| (html[cnt+2] == 'R'))
				   && (html[cnt+3] == '>')){
					cnt = cnt + 4;
					cablecard_page.diag_message[i][j] = 0;
					goto next_line;
				}
				else{
					while (html[cnt] != '>'){
						cnt++;
					}
					cnt++;
				}
			} else if(html[cnt] == '&') {
				cnt+=6;
			} else
				cnt++;
		}
		if (cnt >=(int)len) break;
next_line:

		j = 0;
	}


	cablecard_page.diag_message_filled = i;
}

void CCablecard::parse_app_info_query(uint8_t *html, unsigned len)
{
	cablecard_receive_app_info_query(html, len,  0, 0);
}
int CCablecard::cablecard_get_page(int app_num)
{
	unsigned short length = 0;
	char url_query[100];
	char *url_ptr;
	uint8_t dialogNb;
	uint8_t urlLen;
   	uint8_t max_num_pages;
	MPOD_RET_CODE err;
	cablecard_t cablecard = cablecard_get_instance();

	err = B_Mpod_AppInfoGetNumApps(&max_num_pages);
	if(err || app_num >= max_num_pages ){
		BDBG_ERR(("\n!!!!ERROR retrieving application dialogs\n\n"));
		return -1;
	}

	err = B_Mpod_AppInfoGetURL(app_num, &url_ptr, &urlLen);
	if (err) return -1;
	strcpy(url_query, url_ptr);

	length = strlen(url_query);
	cablecard_page.diag_message_filled = 0;
	cablecard_set_html_callback(cablecard, cablecard_receive_app_info_query);
	err = B_Mpod_MMIHostOpenDialog(url_query, length, &dialogNb);

	return (err? -1 : 0);
}

/* poorman's HTML parser*/
int cablecard_receive_MMI(char *mmi_message, int len)
{
  int length;
  int i = 30;
  int j = 0;
  int k=0, l=0, m=0, n=0;
  int cnt;
  char  *new_mmi_message, *tmp_str1, *tmp_str2, *tmp_str3, *tmp_str4;

   if (len <=0) return -1;
  new_mmi_message = (char *)malloc(len);
  tmp_str1 = (char *)malloc(100);
  tmp_str2 = (char *)malloc(100);
  tmp_str3 = (char *)malloc(100);
  tmp_str4 = (char *)malloc(100);
  memset(new_mmi_message, 0, len);
  memset(tmp_str1, 0, 100);
  memset(tmp_str2, 0, 100);
  memset(tmp_str3, 0, 100);
  memset(tmp_str4, 0, 100);
  length = len;

  for (cnt=0 ; cnt < (length) ; )
    {
      if (mmi_message[cnt] != '<' && mmi_message[cnt] != '&'
	  	 && (mmi_message[cnt] != '\n') && (mmi_message[cnt] != '\0') )
      {
      	if (k < 100) {
     		tmp_str1[k++] = new_mmi_message[j++] = mmi_message[cnt++];
      		}
		else if (l < 100) {
			tmp_str2[l++] = new_mmi_message[j++] = mmi_message[cnt++];
			}
		else if (m < 100) {
			tmp_str3[m++] = new_mmi_message[j++] = mmi_message[cnt++];
			}
		else if (n < 100) {
			tmp_str4[n++] = new_mmi_message[j++] = mmi_message[cnt++];
			}
		else {
     	new_mmi_message[j++] = mmi_message[cnt++];
			}
      } else if (mmi_message[cnt] == '<')
	  {
		while (mmi_message[cnt] != '>')
	  	{
	    	cnt++;
        }
		cnt++;
		if (k < 100) {
			tmp_str1[k++] = new_mmi_message[j++] = ' ';
			}
		else if (l < 100) {
			tmp_str2[l++] = new_mmi_message[j++] = ' ';
			}
		else if (m < 100) {
			tmp_str3[m++] = new_mmi_message[j++] = ' ';
			}
		else if (n < 100) {
			tmp_str4[n++] = new_mmi_message[j++] = ' ';
			}
	  } else if (mmi_message[cnt] == '&')
	  {
	  	cnt+=6;
	  }
	  else
	  	cnt++;
    }

  while (i <= j)
    {
    if (new_mmi_message[i] == ' ')
      {
      new_mmi_message[i] = '\n';
      i+=30;
      }
      else
	{
	  i+=1;
	}
  }
	new_mmi_message[j+1] = '\0';
	info.mmi_message_length = length;
	info.card_mmi_message1 = tmp_str1;
	info.card_mmi_message2 = tmp_str2;
	info.card_mmi_message3 = tmp_str3;
	info.card_mmi_message4 = tmp_str4;


  printf("\n\n*************************************************************************************************************\n");
  printf("*************************************MMI MESSAGE FROM CABLE CARD ************************************************\n");
  printf("*****************************************************************************************************************\n");
  printf("\n %s \n", new_mmi_message);
  printf("\n****************************************************************************************************************\n");
  free(new_mmi_message);
  return 0;
}


int CCablecard::cablecard_set_html_callback(cablecard_t cablecard, cablecard_html_callback callback)
{
	if (cablecard == NULL) return -1;
    cablecard->html_callback = callback;
	return 0;
}

static void mmiHtmlRdyCb(
    uint8_t *html,
    uint16_t len,
    uint8_t dialogNb,
    bool hostDialog,
    uint8_t fileStatus
    )
{
	cablecard_t cablecard = cablecard_get_instance();

    /* add NULL termination */
    html[len - 1] = '\0';
	BDBG_MSG(("mmiHtmlRdyCb Dialog %d diaglog %d len %d", hostDialog, dialogNb, len));
	if (hostDialog == true)
	{
		if (cablecard->html_callback)
			(cablecard->html_callback)(html, len, dialogNb, fileStatus);
	}
	else {
		cablecard_receive_MMI((char*)html, (int)len);
		}
/* TODO: Add MMI to atlas cfg later on */

}


static B_MPOD_MMI_OPEN_STATUS mmiDialogRequestCb(
    B_MPOD_MMI_DISPLAY_TYPE displayType,
    uint8_t dialogNb
    )
{
    BDBG_MSG(("mmiDialogRequestCb Callback Called with dialog number %d for display type %d\n", dialogNb, displayType));
    return B_MPOD_MMI_OK;
}


static void mmiDialogCloseCb(
    uint8_t dialogNb
    )
{
    BDBG_MSG(("mmiDialogCloseCb Callback Called for dialog %d\n", dialogNb));
    MARK_AS_INVALID(dialogNb);
}

#if 0
static void mmiAPDURcvCb(
    uint8_t *data,
    uint32_t tag,
    uint32_t len
    )
{
    BDBG_MSG(("mmiAPDURcvCb Callback Called\n"));
}
#endif


static void mmiSessionOpenCb(
    uint16_t sessionNb
    )
{
    BSTD_UNUSED(sessionNb);
    BDBG_MSG(("mmiSessionOpenCb Callback Called\n"));
    return;
}


static void mmiSessionCloseCb(
    uint16_t sessionNb
    )
{
    BSTD_UNUSED(sessionNb);
    BDBG_MSG(("mmiSessionCloseCb Callback Called\n"));
    return;
}

static void dlNewCodeVersionTableType1Cb(
    uint8_t * data,
	uint32_t len,
    B_MPOD_DL_CVT_HOST_RESPONSE *cvtResponse
    )
{
	#ifdef CDL_SUPPORT
		*cvtResponse = (B_MPOD_DL_CVT_HOST_RESPONSE)cdl_adpu_cvt_type1_process(NULL, data, len);
	#else
		BDBG_MSG(("MPOD_TEST-DL: Got new code version table callback"));
		*cvtResponse = B_MPOD_DL_ACT_NO_ERROR;
        BSTD_UNUSED(data);
        BSTD_UNUSED(len);
	#endif
}

static void dlNewCodeVersionTableType2Cb(
    uint8_t * data,
	uint32_t len,
    B_MPOD_DL_CVT_HOST_RESPONSE *cvtResponse
    )
{
	#ifdef CDL_SUPPORT
		*cvtResponse = (B_MPOD_DL_CVT_HOST_RESPONSE)cdl_adpu_cvt_type2_process(NULL, data, len);
	#else
		BDBG_MSG(("MPOD_TEST-DL: Got new code version table callback"));
		*cvtResponse = B_MPOD_DL_ACT_NO_ERROR;
        BSTD_UNUSED(data);
        BSTD_UNUSED(len);
	#endif
}

uint8_t descriptorBlock[] =
/* tag length                      data                       */
{
    0,  12,      0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
    1,  11,     12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
    2,  10,     23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    3,   9,     33, 34, 35, 36, 37, 38, 39, 40, 41,
    4,   8,     42, 43, 44, 45, 46, 47, 48, 49,
    5,   7,     50, 51, 52, 53, 54, 55, 56,
    6,   6,     57, 58, 59, 60, 61, 62,
    7,   5,     63, 64, 65, 66, 67,
    8,   4,     68, 69, 70, 71,
    9,   3,     72, 73, 74,
    10,  2,     75, 76,
    11,  1,     77
};


static void dlHostInfoCb(
    B_MPOD_DL_SUPPORTED_DL_TYPE supportedDlType,
    B_MPOD_DL_HOST_INFO *hostInfo
    )
{

	#ifdef CDL_SUPPORT
		cdl_adpu_get_host_info((void*)supportedDlType, hostInfo);
	#else
		BDBG_MSG(("MPOD_TEST-DL: Got Host Info callback"));
        BSTD_UNUSED(supportedDlType);
		hostInfo->vendorId = 0x3a3a3a;
		hostInfo->hardwareVersionId = 0x8c8c8c8c;
		hostInfo->numDescriptors = 12;
		hostInfo->descriptors = descriptorBlock;
	#endif
}

static const B_MPOD_FEATURE_ID TestFeatureList[] =
{
    B_MPOD_FEATURE_ID_RF_OUTPUT_CHANNEL,
    B_MPOD_FEATURE_ID_PARENTIAL_CTL_PIN,
    B_MPOD_FEATURE_ID_PARENTIAL_CTL_SETTING,
    B_MPOD_FEATURE_ID_IPPV_PIN,
    B_MPOD_FEATURE_ID_TIME_ZONE,
    B_MPOD_FEATURE_ID_DAYLIGHT_SAVING,
    B_MPOD_FEATURE_ID_AC_OUTLET,
    B_MPOD_FEATURE_ID_LANGUAGE,
    B_MPOD_FEATURE_ID_RATING_REGION,
    B_MPOD_FEATURE_ID_RESET_PINS,
    B_MPOD_FEATURE_ID_CABLE_URL,
    B_MPOD_FEATURE_ID_EAS_LOCATION_CODE,
    B_MPOD_FEATURE_ID_VCT_ID,
    B_MPOD_FEATURE_ID_TURN_ON_CHANNEL,
    B_MPOD_FEATURE_ID_TERMINAL_ASSOC,
    B_MPOD_FEATURE_ID_DOWNLOAD_GRP_ID,
    B_MPOD_FEATURE_ID_ZIP_CODE
};

static void featureReqHostListCb(
    B_MPOD_FEATURE_ID *hostFeatures,
    uint8_t *hostNumFeatures
    )
{
    *hostNumFeatures = sizeof(TestFeatureList)/sizeof(TestFeatureList[0]);
    BKNI_Memcpy(hostFeatures, TestFeatureList, sizeof(TestFeatureList));
}


static B_MPOD_FEATURE_PARAM TestParams[B_MPOD_FEATURE_ID_MAX]; /* adjust for 0 based below */
static B_MPOD_FEATURE_PARAM *RfOutput          = &TestParams[B_MPOD_FEATURE_ID_RF_OUTPUT_CHANNEL-1];
static B_MPOD_FEATURE_PARAM *ParentalPin       = &TestParams[B_MPOD_FEATURE_ID_PARENTIAL_CTL_PIN-1];
static B_MPOD_FEATURE_PARAM *ParentalSettings  = &TestParams[B_MPOD_FEATURE_ID_PARENTIAL_CTL_SETTING-1];
static B_MPOD_FEATURE_PARAM *PurchasePin       = &TestParams[B_MPOD_FEATURE_ID_IPPV_PIN-1];
static B_MPOD_FEATURE_PARAM *TimeZone          = &TestParams[B_MPOD_FEATURE_ID_TIME_ZONE-1];
static B_MPOD_FEATURE_PARAM *DaylightSavings   = &TestParams[B_MPOD_FEATURE_ID_DAYLIGHT_SAVING-1];
static B_MPOD_FEATURE_PARAM *AcOutlet          = &TestParams[B_MPOD_FEATURE_ID_AC_OUTLET-1];
static B_MPOD_FEATURE_PARAM *Language          = &TestParams[B_MPOD_FEATURE_ID_LANGUAGE-1];
static B_MPOD_FEATURE_PARAM *RatingRegion      = &TestParams[B_MPOD_FEATURE_ID_RATING_REGION-1];
static B_MPOD_FEATURE_PARAM *ResetPin          = &TestParams[B_MPOD_FEATURE_ID_RESET_PINS-1];
static B_MPOD_FEATURE_PARAM *CableUrls         = &TestParams[B_MPOD_FEATURE_ID_CABLE_URL-1];
static B_MPOD_FEATURE_PARAM *EmergencyAlertLoc = &TestParams[B_MPOD_FEATURE_ID_EAS_LOCATION_CODE-1];
static B_MPOD_FEATURE_PARAM *VirtualChannel    = &TestParams[B_MPOD_FEATURE_ID_VCT_ID-1];
static B_MPOD_FEATURE_PARAM *TurnOnChan        = &TestParams[B_MPOD_FEATURE_ID_TURN_ON_CHANNEL-1];
static B_MPOD_FEATURE_PARAM *TerminalAssoc     = &TestParams[B_MPOD_FEATURE_ID_TERMINAL_ASSOC-1];
static B_MPOD_FEATURE_PARAM *CommonDownload    = &TestParams[B_MPOD_FEATURE_ID_DOWNLOAD_GRP_ID-1];
static B_MPOD_FEATURE_PARAM *ZipCode           = &TestParams[B_MPOD_FEATURE_ID_ZIP_CODE-1];

static B_MPOD_FEATURE_VIRTUAL_CHANNEL VirtualChannels[10];
static B_MPOD_FEATURE_CABLE_URL Urls[3];


static void featureReqParamsCb(
    void
    )
{
    uint32_t i;
    int len;
    char parentalPin[256];
    char purchasePin[256];
    char url0[256];
    char url1[256];
    char url2[256];
    char terminal[256];
    char zipCode[256];

    /* set up the ID's */
    for (i = 0; i < (int)B_MPOD_FEATURE_ID_MAX; i++) TestParams[i].feature = (B_MPOD_FEATURE_ID)(i+1);
    RfOutput->param.rfOutput.channel = 0x3;
    RfOutput->param.rfOutput.channelUi = B_MPOD_FEATURE_ENABLE_RF_CH_UI;

    sprintf(parentalPin,"%s","09080706");
    ParentalPin->param.parentalPin.chr = (char *) parentalPin;
    ParentalPin->param.parentalPin.length = 8;

    ParentalSettings->param.parentalSettings.factoryReset = 0;
    ParentalSettings->param.parentalSettings.chanCount = 10;
    ParentalSettings->param.parentalSettings.virtualChannels = VirtualChannels;

    for (i = 0; i < ParentalSettings->param.parentalSettings.chanCount; i++)
    {
        ParentalSettings->param.parentalSettings.virtualChannels[i].channelMajorMinor[2] = i;
        ParentalSettings->param.parentalSettings.virtualChannels[i].channelMajorMinor[1] = i * 5;
        ParentalSettings->param.parentalSettings.virtualChannels[i].channelMajorMinor[0] = i * 10;
    }

    sprintf(purchasePin,"%s","0504030");
    PurchasePin->param.purchasePin.chr = (char *)purchasePin;
    PurchasePin->param.purchasePin.length = 7;

    TimeZone->param.timeZone.offset = 27;

    DaylightSavings->param.daylightSavings.ctrl = B_MPOD_FEATURE_USE_DST; /* use daylight savings */
    DaylightSavings->param.daylightSavings.delta = 128;
    DaylightSavings->param.daylightSavings.entry = 0xffff0000;
    DaylightSavings->param.daylightSavings.exit =0x0000ffff;

    AcOutlet->param.acOutlet.ctrl = B_MPOD_FEATURE_AC_USER_SETTING;

#ifdef ESTB_CFG_SUPPORT
	B_Estb_cfg_Get_bin((char*)"/dyn/estb/lang_code", Language->param.language.ctrl, &len );
#else
    Language->param.language.ctrl[0] = 'e';
    Language->param.language.ctrl[1] = 'n';
    Language->param.language.ctrl[2] = 'g';
#endif

    RatingRegion->param.ratingRegion.region = B_MPOD_FEATURE_REGION_US;

    ResetPin->param.resetPin.ctrl = B_MPOD_FEATURE_RESET_P_C_PURCHASED_PINS;

    CableUrls->param.cableUrls.numberOfUrls = 3;
    CableUrls->param.cableUrls.urls = Urls;

    sprintf(url0,"%s","http://www.broadcom.com");
    CableUrls->param.cableUrls.urls[0].length = 23;
    CableUrls->param.cableUrls.urls[0].type = B_MPOD_FEATURE_WEB_PORTAL; /* Web Portal URL */
    CableUrls->param.cableUrls.urls[0].url = (char *)url0;

    sprintf(url1,"%s","http://epg.broadcomm.com");
    CableUrls->param.cableUrls.urls[1].length = 24;
    CableUrls->param.cableUrls.urls[1].type = B_MPOD_FEATURE_EPG; /* EPG URL */
    CableUrls->param.cableUrls.urls[1].url = (char *) url1;

    sprintf(url2,"%s","http://vod.broadcommm.com");
    CableUrls->param.cableUrls.urls[2].length = 25;
    CableUrls->param.cableUrls.urls[2].type = B_MPOD_FEATURE_VOD; /* VOD URL */
    CableUrls->param.cableUrls.urls[2].url = (char *) url2;

    EmergencyAlertLoc->param.emergencyAlertLoc.stateCode = 101;
    EmergencyAlertLoc->param.emergencyAlertLoc.countySubdivision = 102;
    EmergencyAlertLoc->param.emergencyAlertLoc.countyCode = 103;

    VirtualChannel->param.virtualChannel.vctId = 234;

    TurnOnChan->param.turnOnChan.virtualChannel = 88;
    TurnOnChan->param.turnOnChan.defined  = 1;

    sprintf(terminal,"%s","This Is My Terminal");
    TerminalAssoc->param.terminalAssoc.length = 19;
    TerminalAssoc->param.terminalAssoc.identifier = (char *)terminal;

#ifdef ESTB_CFG_SUPPORT
	{
		unsigned int group_id = 0;
		B_Estb_cfg_Get_uint32((char*)"/dyn/estb/group_id", &group_id);
	    CommonDownload->param.commonDownload.groupId = group_id;
	}
#else
    CommonDownload->param.commonDownload.groupId = 56;
#endif
    sprintf(zipCode,"%s","95118-9446");
    ZipCode->param.zipCode.chr = (char *)zipCode;
    ZipCode->param.zipCode.length = 10;

    B_Mpod_FeatureSendUpdatedParams(TestParams, 17);
}


static const char *FeatureString[] =
{
    "dummy", /* features are 1-based */
    "rf_output_channel",
    "parental_control_pin",
    "parental_control_settings",
    "purchase_pin",
    "time_zone",
    "daylight_savings",
    "ac_outlet",
    "language",
    "rating_region",
    "reset_pin",
    "cable_URLs",
    "EAS_location_code",
    "VCT_ID",
    "turn_on_channel",
    "terminal_association",
    "download_group_id",
    "zip_code"
};

static void featureRcvCardListCb(
    B_MPOD_FEATURE_ID *cardFeatures,
    uint8_t cardNumFeatures
    )
{
    uint32_t i;

    BDBG_MSG(("%s list of Card supported features", BSTD_FUNCTION));
    for(i = 0; i < cardNumFeatures; i++)
    {
    	if (cardFeatures[i] < (sizeof(FeatureString)/sizeof(const char*)))
			BDBG_MSG(("Feature %d = %s", i, FeatureString[cardFeatures[i]]));
		else
			BDBG_MSG(("Unknown Feature %d", i));
    }
}


static void featureRcvParamsCb(
    B_MPOD_FEATURE_PARAM *featureParams,
    uint8_t numFeatures
    )
{
    uint32_t i, j;
    unsigned int group_id;
    BDBG_MSG(("Received the following feature params from the CableCard\n\n"));

    for(i = 0; i < numFeatures; i++)
    {
        switch(featureParams[i].feature)
        {
            case B_MPOD_FEATURE_ID_RF_OUTPUT_CHANNEL:
                BDBG_MSG(("RF_OUTPUT_CHANNEL\n\n"));
                BDBG_MSG(("Channel = %d, Channel UI is %s\n",
                featureParams[i].param.rfOutput.channel,
                (featureParams[i].param.rfOutput.channelUi) == 0x1 ? "enabled" : "disabled"));
            break;

            case B_MPOD_FEATURE_ID_PARENTIAL_CTL_PIN:
                BDBG_MSG(("PARENTIAL_CTL_PIN\n\n"));
                featureParams[i].param.parentalPin.chr[featureParams[i].param.parentalPin.length + 1] = '\0';
                BDBG_MSG(("Pin is %s\n", featureParams[i].param.parentalPin.chr));
            break;

            case B_MPOD_FEATURE_ID_PARENTIAL_CTL_SETTING:
                BDBG_MSG(("PARENTIAL_CTL_SETTING\n\n"));
                BDBG_MSG(("%s Factory Reset, Channel Count = %d\n",
                (featureParams[i].param.parentalSettings.factoryReset == 0xa7) ? "Perform" : "Don't Perform",
                featureParams[i].param.parentalSettings.chanCount));
                for(j = 0; j < featureParams[i].param.parentalSettings.chanCount; j++)
                {
                    unsigned major, minor;

                    major = ((featureParams[i].param.parentalSettings.virtualChannels[j].channelMajorMinor[0] & 0xf) << 6) |
                            ((featureParams[i].param.parentalSettings.virtualChannels[j].channelMajorMinor[1] & 0xfc) >> 2);
                    minor = ((featureParams[i].param.parentalSettings.virtualChannels[j].channelMajorMinor[1] & 0x3) << 8) |
                            featureParams[i].param.parentalSettings.virtualChannels[j].channelMajorMinor[2];

                    BDBG_MSG(("Virtual Channel %d %d included\n", major, minor));
                }
            break;

            case B_MPOD_FEATURE_ID_IPPV_PIN:
                BDBG_MSG(("IPPV_PIN\n\n"));
                featureParams[i].param.purchasePin.chr[featureParams[i].param.purchasePin.length + 1] = '\0';
                BDBG_MSG(("Pin is %s\n", featureParams[i].param.purchasePin.chr));
            break;

            case B_MPOD_FEATURE_ID_TIME_ZONE:
                BDBG_MSG(("TIME_ZONE\n\n"));
                BDBG_MSG(("Time Zone Offset = %d\n", featureParams[i].param.timeZone.offset));
				info.timezoneOffset = featureParams[i].param.timeZone.offset;
            break;

            case B_MPOD_FEATURE_ID_DAYLIGHT_SAVING:
                BDBG_MSG(("DAYLIGHT_SAVING\n\n"));
                BDBG_MSG(("%s use Daylight Savings\n",
                    (featureParams[i].param.daylightSavings.ctrl == B_MPOD_FEATURE_USE_DST) ? "Do" : "Don't"));
                if(featureParams[i].param.daylightSavings.ctrl == B_MPOD_FEATURE_USE_DST)
                {
                    BDBG_MSG(("Delta = %d, Entry = %d, Exit = %d\n",
                    featureParams[i].param.daylightSavings.delta,
                    featureParams[i].param.daylightSavings.entry,
                    featureParams[i].param.daylightSavings.exit));
					info.DSTimeDelta = featureParams[i].param.daylightSavings.delta;
					info.DSTimeEntry = featureParams[i].param.daylightSavings.entry;
					info.DSTimeExit = featureParams[i].param.daylightSavings.exit;
                }
            break;

            case B_MPOD_FEATURE_ID_AC_OUTLET:
            {
                char ACOutletStrings[][256] = {"Use User Setting",
                                           "Switched AC Outlet",
                                           "Unswitched AC Outlet",
                                           "Reserved"};
                BDBG_MSG(("AC_OUTLET\n\n"));
                BDBG_MSG(("AC Outlet setting %s\n", ACOutletStrings[featureParams[i].param.acOutlet.ctrl & 0x3]));
            }
            break;

            case B_MPOD_FEATURE_ID_LANGUAGE:
                BDBG_MSG(("LANGUAGE\n\n"));
                BDBG_MSG(("Language code is %d %d %d\n",
                featureParams[i].param.language.ctrl[0],
                featureParams[i].param.language.ctrl[1],
                featureParams[i].param.language.ctrl[2]));
            break;

            case B_MPOD_FEATURE_ID_RATING_REGION:
                BDBG_MSG(("RATING_REGION\n\n"));
                BDBG_MSG(("Rating Region is %d\n",
                featureParams[i].param.ratingRegion.region));
            break;

            case B_MPOD_FEATURE_ID_RESET_PINS:
            {
                char resetPinsString[][256] = {"Don't reset any pin",
                                            "Reset parental control pin",
                                            "Reset purchase pin",
                                            "Reset parental control and purchase pin"};
                BDBG_MSG(("RESET_PINS\n\n"));
                BDBG_MSG(("Reset Pin Setting is %s\n",
                resetPinsString[featureParams[i].param.resetPin.ctrl & 0x3]));
            }
            break;

            case B_MPOD_FEATURE_ID_CABLE_URL:
            {
                BDBG_MSG(("CABLE_URL\n\n"));

                /* populate the array with type, length and the pointer to the url */
                for (j = 0; j < featureParams[i].param.cableUrls.numberOfUrls; j++)
                {
                    char urlTypeString[][256] = {"undefined", "Web Portal URL", "EPG URL", "VOD URL"};

                    featureParams[i].param.cableUrls.urls[j].url[featureParams[i].param.cableUrls.urls[j].length + 1] = '\0';

                    BDBG_MSG(("Type %s, URL = %s\n",
                    urlTypeString[featureParams[i].param.cableUrls.urls[j].type & 0x3],
                    featureParams[i].param.cableUrls.urls[j].url));
                }
            }
            break;

            case B_MPOD_FEATURE_ID_EAS_LOCATION_CODE:
                BDBG_MSG(("EAS_LOCATION_CODE\n\n"));
                BDBG_MSG(("State Code = %d, County Subdvsn = %d, County Code = %d\n",
                featureParams[i].param.emergencyAlertLoc.stateCode,
                featureParams[i].param.emergencyAlertLoc.countySubdivision,
                featureParams[i].param.emergencyAlertLoc.countyCode));
				info.EALocCode[0] = featureParams[i].param.emergencyAlertLoc.stateCode;
				info.EALocCode[1] = (featureParams[i].param.emergencyAlertLoc.countySubdivision<<4)
									|(featureParams[i].param.emergencyAlertLoc.countyCode>>8);
				info.EALocCode[2] = featureParams[i].param.emergencyAlertLoc.countyCode&0xff;
            break;

            case B_MPOD_FEATURE_ID_VCT_ID:
                BDBG_MSG(("VCT_ID\n\n"));
                BDBG_MSG(("Virtual Channel ID = %d\n",
                    featureParams[i].param.virtualChannel.vctId));
				info.vctId = featureParams[i].param.virtualChannel.vctId;
            break;

            case B_MPOD_FEATURE_ID_TURN_ON_CHANNEL:
                BDBG_MSG(("TURN_ON_CHANNEL\n\n"));
                BDBG_MSG(("Turn-On Channel %d is %s\n", featureParams[i].param.turnOnChan.virtualChannel,
                featureParams[i].param.turnOnChan.defined ? "defined" : "not defined"));
            break;

            case B_MPOD_FEATURE_ID_TERMINAL_ASSOC:
                BDBG_MSG(("TERMINAL_ASSOCIATION\n\n"));
                featureParams[i].param.terminalAssoc.identifier[featureParams[i].param.terminalAssoc.length + 1] = '\0';
                BDBG_MSG(("Terminal Association ID = %s\n", featureParams[i].param.terminalAssoc.identifier));
            break;

            case B_MPOD_FEATURE_ID_DOWNLOAD_GRP_ID:
                BDBG_MSG(("DOWNLOAD_GROUP_ID\n\n"));
                BDBG_MSG(("Download Group ID is %d\n", featureParams[i].param.commonDownload.groupId));
#ifdef ESTB_CFG_SUPPORT
				group_id = featureParams[i].param.commonDownload.groupId;
				B_Estb_cfg_Set_uint32((char*)"/dyn/estb/group_id", group_id);
#endif
            break;

            case B_MPOD_FEATURE_ID_ZIP_CODE:
                BDBG_MSG(("ZIP_CODE\n\n"));
                featureParams[i].param.zipCode.chr[featureParams[i].param.zipCode.length + 1] = '\0';
                BDBG_MSG(("Zip Code is %s\n", featureParams[i].param.zipCode.chr));
            break;
        default:
            BDBG_MSG(("%s invalid feature \n", BSTD_FUNCTION));
        }

        featureParams[i].featureStatus = 0; /* feature param accepted */

        BDBG_MSG(("\n\n\n"));

    }
}


static void featureParamDeniedCb(
    B_MPOD_FEATURE_ID feature,
    B_MPOD_FEATURE_STATUS status
    )
{
    BDBG_MSG(("Param %s denied with status %d",FeatureString[feature], status ));
}

static B_MPOD_HC_IB_TUNE_STATUS inbandTuneCb(
    uint32_t freqHz,
    B_MPOD_HC_IB_MOD_TYPE modulation,
    uint8_t *ltsid
    )
{
#if 1
    BSTD_UNUSED(freqHz);
    BSTD_UNUSED(modulation);
    BSTD_UNUSED(ltsid);
	BDBG_WRN(("HC inbandTuneCb not currently supported"));
	return B_MPOD_HC_IB_TUNE_ACCEPTED;
#else
    char *modStrings[] = {"QAM64","QAM256"};
	int rc;
	NEXUS_FrontendHandle frontend;
	tune_params tune_param;
	int tuner_ltsid;
	cablecard_t cablecard = cablecard_get_instance();

	if (cablecard == NULL) return B_MPOD_HC_IB_TUNER_BUSY;
	if (Trinity_GetInstance()->_tuner[1])
		frontend = Trinity_GetInstance()->_tuner[1];
	else
		frontend = Trinity_GetInstance()->_tuner[0];

    if(B_Mpod_HomingIsHomingActive())/*TODO*/
    {
        *ltsid = 0x80;
        return B_MPOD_HC_IB_TUNE_ACCEPTED;
    }

    BDBG_MSG(("Freq: %dHz  modulation: %s\n",
            freqHz, modStrings[modulation & 0x1]));
	tune_param.callback.callback = NULL;
	tune_param.annex = NEXUS_FrontendQamAnnex_eB;
	tune_param.frequency = freqHz;
	if (modulation == B_MPOD_HC_IB_QAM64) {
		tune_param.modulation = NEXUS_FrontendQamMode_e64;
		tune_param.symbolrate = 5056941;
	} else if (modulation == B_MPOD_HC_IB_QAM256) {
		tune_param.modulation = NEXUS_FrontendQamMode_e256;
		tune_param.symbolrate = 5360537;
	} else
		return B_MPOD_HC_IB_INVALID_MODULATION;
	channel_tune_getLtsid(frontend, &tuner_ltsid);
	*ltsid = tuner_ltsid;

	rc = channel_tune(frontend, &tune_param);
	return (rc)?  B_MPOD_HC_IB_HARDWARE_ERROR : B_MPOD_HC_IB_TUNE_ACCEPTED ;
#endif
}


static B_MPOD_HC_OOB_TX_TUNE_STATUS oobTxTuneCb(
    uint32_t freqHz,
    uint32_t powerLevel,
    uint32_t rate
    )
{
#ifndef VMS93380_SUPPORT
#if defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT) || defined(CABLE_SUPPORT)

	NEXUS_FrontendUpstreamSettings setting;
	NEXUS_Error rc;
	cablecard_t cablecard = cablecard_get_instance();

	NEXUS_Frontend_GetDefaultUpstreamSettings(&setting);
	setting.frequency = freqHz;
	setting.mode = (freqHz==0|| rate==0) ? NEXUS_FrontendUpstreamMode_eDocsis : cablecard->us_mode;
	setting.powerLevel = powerLevel*50; /* in hundredth of dBmV*/
	setting.symbolRate = rate;

    BDBG_MSG(("Freq: %dHz  PowerLevel: %d.%ddBmV  Rate: %dbaud\n",
            freqHz, powerLevel >> 1, (powerLevel & 0x1) ? 5 : 0, rate));

	rc = NEXUS_Frontend_TuneUpstream(g_upstreamHFrontendHandle, &setting);

	return (rc)?  B_MPOD_HC_OOB_TX_OTHER_ERROR : B_MPOD_HC_OOB_TX_TUNE_GRANTED ;
#else
	/* no OOB US support */
	BSTD_UNUSED(freqHz);
	BSTD_UNUSED(powerLevel);
	BSTD_UNUSED(rate);

	BKNI_Sleep(500);
	return B_MPOD_HC_OOB_TX_XMIT_PHYSICAL_NA ;
#endif
#else
	/* still waiting for OOB US support */
	BKNI_Sleep(500);
	return B_MPOD_HC_OOB_TX_XMIT_PHYSICAL_NA ;
#endif
}

/* switch frontend OOB TX mode to Docsis UPstream mode*/
static void OOB_Tx_Docsis(void)
{
	oobTxTuneCb(0, 0, 0);
}



static B_MPOD_HC_OOB_RX_TUNE_STATUS oobRxTuneCb(
    uint32_t freqHz,
    uint32_t rate,
    bool spectralInv
    )
{
	NEXUS_FrontendOutOfBandSettings setting;
	int  rc;
	cablecard_t cablecard = cablecard_get_instance();

//use OOB RX as the final reference to choose correct OOB mode
	if (rate == 1024000)
	{
		cablecard->us_mode = NEXUS_FrontendUpstreamMode_ePodDvs178;
		/*xy, epod is not used anymore*/
		cablecard->oob_mode = NEXUS_FrontendOutOfBandMode_eDvs178Qpsk;
		BDBG_MSG(("Set to DVS178 OOB mode"));
	} else if (rate == 1544000/2 || rate == 3088000/2)
	{
		cablecard->us_mode = NEXUS_FrontendUpstreamMode_ePodDvs167;
		cablecard->oob_mode = NEXUS_FrontendOutOfBandMode_eDvs167Qpsk;
		BDBG_MSG(("Set to DVS167 OOB mode"));
	} else
		BDBG_WRN((" wrong OOB symbolrate!"));
	NEXUS_Frontend_GetDefaultOutOfBandSettings(&setting);
	setting.mode = cablecard->oob_mode;
	setting.frequency = freqHz;
	setting.spectrum = /*NEXUS_FrontendOutOfBandSpectrum_eAuto*/(spectralInv) ? NEXUS_FrontendOutOfBandSpectrum_eInverted : NEXUS_FrontendOutOfBandSpectrum_eNonInverted;
	setting.symbolRate = rate;

	BDBG_MSG(("Freq: %dHz  Rate: %dbps  Spectral Inversion: %s, OOB tuner 0x%x\n", setting.frequency/* freqHz*/, rate, spectralInv ? "true" : "false", _pTunerOob->getFrontend()));
	rc = _pTunerOob->tune(setting);
	BKNI_Sleep(500);
	return (rc)?  B_MPOD_HC_OOB_RX_OTHER_ERROR : B_MPOD_HC_OOB_RX_TUNE_GRANTED ;
}

static void sourceIdToFreqCb(
    uint16_t sourceId,
    uint32_t *freqHz,
    B_MPOD_HC_IB_MOD_TYPE *modulation
    )
{
#if 1
    BSTD_UNUSED(sourceId);
    BSTD_UNUSED(freqHz);
    BSTD_UNUSED(modulation);
	BDBG_WRN(("sourceIdToFreqCb not currently supported"));
#else
	channel_info_t *info;
	info = NULL;
	info = channel_mgr_get_ch_bysource(sourceId);


	if (info)
	{
		BDBG_MSG((" Find the channel by source ID %d", sourceId));
    	*freqHz = info->frequency;
    	*modulation = (info->modulation == NEXUS_FrontendQamMode_e256) ? B_MPOD_HC_IB_QAM256 : B_MPOD_HC_IB_QAM64;
	}
	else
	{
		BDBG_ERR((" can't find the channel by source ID %d", sourceId));
    	*freqHz = 0;
    	*modulation = B_MPOD_HC_IB_QAM64;
	}
#endif
}

static void delayedDownloadReqCb(
    void
    )
{
    BDBG_MSG(("received %s\n", BSTD_FUNCTION));
}


static void homingCompleteCb(
    void
    )
{
    BDBG_MSG(("received %s\n", BSTD_FUNCTION));
}


/* the host should NOT interrupt the download */
static void downloadStartingCb(
    B_MPOD_HOMING_DOWNLOAD_INFO *downloadInfo
    )
{
    char notifyString[257];
    char sourceStrings[][256] = {"unknown", "QAM Inband", "QPSK OOB", "reserved"};
    char timeoutTypeStrings[][256] = {"both timeouts", "transport timeout",
                                            "download timeout", "no_timeout"};

    BDBG_MSG(("received %s\n", BSTD_FUNCTION));

    if(downloadInfo->notifyTextLength)
    {
        strncpy(notifyString, downloadInfo->notifyText, downloadInfo->notifyTextLength);
        notifyString[downloadInfo->notifyTextLength] = '\0';
        BDBG_MSG(("Notify Message: %s\n", notifyString));
    }

    BDBG_MSG(("Upgrade Source: %s,  Download Time %d\n",
            sourceStrings[downloadInfo->source & 0x3],
            downloadInfo->downloadTime));

    BDBG_MSG(("Timeout Type: %s,  Timeout Period: %d\n",
            timeoutTypeStrings[downloadInfo->timeoutType & 0x3],
            downloadInfo->downloadTimeoutPeriod));
}


static void downloadCompleteCb(
    B_MPOD_HOMING_UPGRADE_RESET_REQUEST resetType
    )
{
    char resetTypeStrings[][256] = {"PCMCIA Reset", "Card Reset", "No Reset", "Reserved"};

    BDBG_MSG(("received %s\n", BSTD_FUNCTION));
    BDBG_MSG(("requested %s\n", resetTypeStrings[resetType & 0x3]));
}

static void homingTimeoutCb(
    void
    )
{
    BDBG_MSG(("received %s\n", BSTD_FUNCTION));
    BDBG_MSG(("Resetting CableCard\n"));
}


/* tells the app that the requested connection for privateAppId has been established */
static void sasConnectCnfCallback(
    uint8_t *privateAppId,
    B_MPOD_SAS_HANDLE newSasConnection
    )
{
    int i;

    for(i = 0; i < 32; i++)
    {
        if(SasConnections[i] == 0)
        {
            SasConnections[i] = newSasConnection;
		    BDBG_MSG(("SAS connection %d assigned for priv SAS app id %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    i, privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
                    privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]));
            return;
        }
    }

    BDBG_MSG(("Unable to open new SAS Connection. Connection limit has been reached\n"));
}


/* tells the app that the card is ready for syncrhonous communication for this privateAppId */
static void sasConnectionRdyCallback(
    B_MPOD_SAS_HANDLE sasConnection,
    uint8_t *privateAppId
    )
{

    BDBG_MSG(("Recieved connection rdy for connection %#x assigned for priv SAS app id %02x %02x %02x %02x %02x %02x %02x %02x\n",
        sasConnection, privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
        privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]
    ));
}


/* delivers data to the application from the card */
static void sasSynchDataRcvCallback(
    B_MPOD_SAS_HANDLE sasConnection,
    uint8_t *privateAppId,
    uint8_t *msg,
    uint32_t len
    )
{
    uint32_t i = 0;
    BSTD_UNUSED(sasConnection);
    BDBG_MSG(("Received new data through synchronous transmission from private app %02x %02x %02x %02x %02x %02x %02x %02x\n",
        privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
        privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]
    ));

    BDBG_MSG(("msg: \n"));
    while(len--) BDBG_MSG(("%x ", msg[i++]));
    BDBG_MSG(("\n"));
}


/* delivers data to the application from the card */
static void sasAsynchDataRcvCallback(
    B_MPOD_SAS_HANDLE sasConnection,
    uint8_t *privateAppId,
    uint8_t *msg,
    uint32_t len)
{
    uint32_t i = 0;
    BSTD_UNUSED(sasConnection);
    BDBG_MSG(("Received new data through asynchronous transmission from private app %02x %02x %02x %02x %02x %02x %02x %02x\n",
        privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
        privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]
    ));

    BDBG_MSG(("msg: \n"));
    while(len--) BDBG_MSG(("%x ", msg[i++]));
    BDBG_MSG(("\n"));

}


/* retrieves data from the app to be sent to the card (app previously requested a syncrhonous transfer of data) */
static void sasGetSynchDataCallback(
    B_MPOD_SAS_HANDLE sasConnection,
    uint8_t *privateAppId,
    uint8_t transactionNb,
    uint8_t **sasMsg,
    uint32_t *sasLen
    )
{
    BSTD_UNUSED(sasConnection);
    BDBG_MSG(("Received request for data for transaction number %d for private app %02x %02x %02x %02x %02x %02x %02x %02x\n",
        transactionNb, privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
        privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]
    ));

    /* Preliminary handshake is over, allow the data to be transmitted */
    *sasMsg = SasData;
    *sasLen = 8;
}


/* tells the app that the card has closed the connection for privateAppId */
static void sasConnectionClosedCallback(
    B_MPOD_SAS_HANDLE sasConnection,
    uint8_t *privateAppId
    )
{
    int i;

    for(i = 0; i < 32; i++)
    {
        if(SasConnections[i] == sasConnection)
        {
            SasConnections[i] = 0;
		    BDBG_MSG(("Closing SAS connection %#x assigned for priv SAS app id %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    sasConnection, privateAppId[0], privateAppId[1], privateAppId[2], privateAppId[3],
                    privateAppId[4], privateAppId[5], privateAppId[6], privateAppId[7]
            ));
            return;
        }
    }

    BDBG_MSG(("Unable to close SAS Connection. Connection not found\n"));
}

void diagReqCb(
    uint8_t numDiags,
    B_MPOD_DIAG_DATA *diagRequests
    )
{
#if 1
    BSTD_UNUSED(numDiags);
    BSTD_UNUSED(diagRequests);
	BDBG_WRN(("diagReqCb not currently supported"));
#else
    uint8_t i, j;
	NEXUS_Error rc;

    for(i = 0; i < numDiags; i++)
    {
		BDBG_MSG((" NUM: %d  Diag_id %d", i, diagReqests[i].id));
		if (Diag_Update(&diagReqests[i]))
			diagReqests[i].status = B_MPOD_DIAG_DENIED_OTHER;
		else
			diagReqests[i].status = B_MPOD_DIAG_GRANTED;
    }
#endif
}


void caInfoCb(
    uint16_t *caSystemId,
    uint32_t numSystemId
    )
{
    uint32_t i;
    for(i = 0; i < numSystemId; i++)
        BDBG_MSG(("CA System Id [%d]: %x\n", i, caSystemId[i]));
	if (numSystemId) info.CAId = caSystemId[0];
	info.CPId = CP_SYSTEM2_ID;
}


void caPmtUpdateReply(
    B_MPOD_CA_PMT_UPDATE_INFO *replyInfo
    )
{
    uint32_t i;
    unsigned pid;
    bool caEnableFlag;
    int caEnable;
    BDBG_MSG(("%s\n", BSTD_FUNCTION));
    BDBG_MSG(("\nProg Idx: %d  Prog Num: %d, Src Id: %d\n",
            replyInfo->progIndex, replyInfo->progNum, replyInfo->srcId));
    BDBG_MSG(("Trans Id: %d  LTSID: %d\n", replyInfo->transId, replyInfo->ltsid));
    if(replyInfo->caEnaFlag)
        BDBG_MSG(("Program Level CA Enable: %d\n", replyInfo->caEna));
    else
        BDBG_MSG(("No Program Level CA Enable\n"));

    for(i = 0; i < replyInfo->numElem; i++)
    {
        B_MPOD_CA_GET_PMT_ES_INFO(replyInfo->esInfo, i, pid, caEnableFlag, caEnable);
        if(caEnableFlag)
            BDBG_MSG(("Es Level CA Enable:  pid:%02x  CA Enable:%d\n", pid, (uint8_t)caEnable));
        else
            BDBG_MSG(("No Es Level CA Enable for pid %02x", pid));
    }

}


void caPmtUpdate(
    B_MPOD_CA_PMT_UPDATE_INFO *updateInfo
    )
{
    BDBG_MSG(("%s\n", BSTD_FUNCTION));
    caPmtUpdateReply(updateInfo);
}


void caPmtReply(
    B_MPOD_CA_PMT_REPLY_INFO *replyInfo
    )
{
    BDBG_MSG(("%s\n", BSTD_FUNCTION));
    caPmtUpdateReply(replyInfo);
}


void getAuthKeyCb(
    uint8_t *authKey,
    bool *authKeyExists
    )
{
	unsigned char hostId[5];
    BDBG_MSG(("%s\n", BSTD_FUNCTION));
    B_Mpod_TestCpGetAuthKey(authKey, authKeyExists);
	info.authKeyStatus = true;

	if (*authKeyExists)
	{
		B_Mpod_TestCpGetID(hostId, info.cardId);
		BDBG_MSG((" Get host ID and card ID"));
	}
}


void cardAuthMsgCb(
    uint8_t *cardDevCert,
    uint8_t *cardManCert,
    uint8_t *dhPubKeyC,
    uint8_t *signC,
    uint8_t *hostDevCert,
    uint8_t *hostManCert,
    uint8_t *dhPubKeyH,
    uint8_t *signH
    )
{
	int ret;
    BDBG_MSG(("%s\n", BSTD_FUNCTION));
    BDBG_MSG(("Getting Host CP Authentication Parameters\n"));
    B_Mpod_TestCpGetHostAuthParams(hostDevCert, hostManCert, dhPubKeyH, signH);
    BDBG_MSG(("Checking Card CP Authorization, generating AuthKey\n"));
    ret = B_Mpod_TestCpGenAuthKeyH(cardDevCert, cardManCert, dhPubKeyC, signC);
	if (ret&cablecard_cardcert_error)
	{
		info.certCheck = true;
		BDBG_ERR((" card certificate error"));
	}
	if (ret&cablecard_hostId_error)
	{
		BDBG_ERR((" HOST ID error"));
	}
	if (ret&cablecard_cardId_error)
	{
		BDBG_ERR((" card ID error"));
	}
}


void getNonceCb(
    uint8_t *nonce
    )
{
    BDBG_MSG(("%s\n", BSTD_FUNCTION));
    B_Mpod_TestCpGenerateNonce(nonce);
}

void getIDCb(
	uint8_t * hostId,
	uint8_t * cardId)
{
    BDBG_MSG(("%s\n", BSTD_FUNCTION));
	B_Mpod_TestCpGetID(hostId, cardId);
	memcpy(info.cardId, cardId, 8);
}

#if NEXUS_HAS_SECURITY
NEXUS_Error configureCAKeySlot(uint8_t *keyData, CChannel *pChannel)
{
    NEXUS_SecurityClearKey clearKey;
	NEXUS_SecurityKeySlotSettings keySlotSettings;
	NEXUS_SecurityAlgorithmSettings algorithmSettings;
	NEXUS_Error rc;
    NEXUS_KeySlotHandle keySlot = NULL;
    CPid * pVideoPid = NULL;
    CPid * pAudioPid = NULL;

    NEXUS_Security_GetDefaultAlgorithmSettings(&algorithmSettings);
	algorithmSettings.terminationMode	= NEXUS_SecurityTerminationMode_eCipherStealing;
	algorithmSettings.algorithm			= NEXUS_SecurityAlgorithm_e3DesAba;
	algorithmSettings.algorithmVar		= NEXUS_SecurityAlgorithmVariant_eEcb;

#if BCHP_CHIP == 7420 || BCHP_CHIP == 7125 || BCHP_CHIP == 7425 || BCHP_CHIP == 7429
	algorithmSettings.ivMode 			 = NEXUS_SecurityIVMode_eRegular;
	algorithmSettings.solitarySelect 	 = NEXUS_SecuritySolitarySelect_eClear;
	algorithmSettings.caVendorID 		 = 0x1234;
	algorithmSettings.askmModuleID		 = NEXUS_SecurityAskmModuleID_eModuleID_4;
	algorithmSettings.otpId				 = NEXUS_SecurityOtpId_eOtpVal;
	algorithmSettings.testKey2Select 	 = 0;
	algorithmSettings.dest                = NEXUS_SecurityAlgorithmConfigDestination_eCa;
	algorithmSettings.modifyScValue[NEXUS_SecurityPacketType_eRestricted] = false;
	algorithmSettings.modifyScValue[NEXUS_SecurityPacketType_eGlobal]     = false;
	algorithmSettings.operation           = NEXUS_SecurityOperation_eDecrypt;
#endif

	/* Route a clear key */
	NEXUS_Security_GetDefaultClearKey(&clearKey);
	clearKey.keySize = 16; /* 16 bytes (128 bits) */
	clearKey.dest = NEXUS_SecurityAlgorithmConfigDestination_eCa;
	clearKey.keyIVType = NEXUS_SecurityKeyIVType_eNoIV;
	BKNI_Memcpy(clearKey.keyData, keyData, clearKey.keySize);

    NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
	keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCa;
	BDBG_MSG(("Allocating new key slot"));
	keySlot = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
	if(keySlot== NULL)
    {
        BDBG_ERR(("%s key slot allocation failed",BSTD_FUNCTION));
        goto error;
    }
    pChannel->setKeySlot(keySlot);
	rc = NEXUS_Security_ConfigAlgorithm(keySlot, &algorithmSettings);
	if (rc != 0)
    {
        BDBG_ERR(("%s security alogorithm apply settings failed",BSTD_FUNCTION));
        goto error;
    }
    clearKey.keyEntryType = NEXUS_SecurityKeyType_eOdd;
	rc = NEXUS_Security_LoadClearKey(keySlot, &clearKey);
	if (rc != 0)
    {
        BDBG_ERR(("%s load odd clear key failed",BSTD_FUNCTION));
        goto error;
    }
	clearKey.keyEntryType = NEXUS_SecurityKeyType_eEven;
	rc = NEXUS_Security_LoadClearKey(keySlot, &clearKey);
	if (rc != 0)
    {
        BDBG_ERR(("%s load even clear key failed",BSTD_FUNCTION));
        goto error;
    }

    pVideoPid = pChannel->getPid(0, ePidType_Video);
    pAudioPid = pChannel->getPid(0, ePidType_Audio);
    rc = NEXUS_KeySlot_AddPidChannel(keySlot, pVideoPid->getPidChannel());
	if (rc != 0)
    {
        BDBG_ERR(("%s add video pidChannel to keySlot failed ",BSTD_FUNCTION));
        goto error;
    }

    rc = NEXUS_KeySlot_AddPidChannel(keySlot, pAudioPid->getPidChannel());
	if (rc != 0)
    {
        BDBG_ERR(("%s add video pidChannel to keySlot failed ",BSTD_FUNCTION));
        goto error;
    }
error:
	return rc;
}
#endif

void removeKeyCb(
    uint16_t programNumber,
    uint8_t ltsid
    )
{

    BDBG_MSG(("%s\n", BSTD_FUNCTION));
#ifdef NEXUS_HAS_SECURITY
    unsigned programIndex=0;
    CChannel *pChannel=NULL;
    CPid * pVideoPid = NULL;
    CPid * pAudioPid = NULL;

    BDBG_MSG(("%s programNumber %u ltsid %u", BSTD_FUNCTION, programNumber, ltsid));

    for(programIndex=0;programIndex<MAX_CABLECARD_ROUTE;programIndex++)
    {
		if (pCableCardChannels[programIndex] == NULL)
		{
			continue;
		}
		else
		{
          CParserBand * pBand = pCableCardChannels[programIndex]->getParserBand();
          if( (ltsid == pBand->getBand()) &&
              (pCableCardChannels[programIndex]->getProgramNum() == programNumber))
          {
              BDBG_WRN(("%s found programNumber %u ltsid %u", BSTD_FUNCTION, programNumber, ltsid));
              pChannel = pCableCardChannels[programIndex];
              break;
          }
		}

    }
    if(!pChannel || !pChannel->getKeySlot())
    {
       BDBG_ERR(("%s no channel found with programNumber %u ltsid %u", BSTD_FUNCTION, programNumber, ltsid));
       return;
    }

    pVideoPid = pChannel->getPid(0, ePidType_Video);
    pAudioPid = pChannel->getPid(0, ePidType_Audio);
    NEXUS_KeySlot_RemovePidChannel(pChannel->getKeySlot(),pVideoPid->getPidChannel());
    NEXUS_KeySlot_RemovePidChannel(pChannel->getKeySlot(),pAudioPid->getPidChannel());
    NEXUS_Security_FreeKeySlot(pChannel->getKeySlot());
    pChannel->setKeySlot(NULL);
    B_Event_Set(keySlotRemoved[programIndex]);
#else
	BSTD_UNUSED(programNumber);
	BSTD_UNUSED(ltsid);
#endif
    return;
}

void progKeyCb(
    uint8_t *desABAKey,
    uint16_t programNumber,
    uint8_t ltsid
    )
{
#ifdef NEXUS_HAS_SECURITY
    unsigned programIndex=0;
    CChannel *pChannel=NULL;
    BDBG_MSG(("%s programNumber %u ltsid %u", BSTD_FUNCTION, programNumber, ltsid));
    for(programIndex=0;programIndex<MAX_CABLECARD_ROUTE;programIndex++)
    {
        CParserBand * pBand = pCableCardChannels[programIndex]->getParserBand();
        if( (ltsid == pBand->getBand()) &&
            (pCableCardChannels[programIndex]->getProgramNum() == programNumber))
        {
            BDBG_WRN(("%s found programNumber %u ltsid %u", BSTD_FUNCTION, programNumber, ltsid));
            pChannel = pCableCardChannels[programIndex];
            break;
        }

    }
    if(!pChannel)
{
       BDBG_ERR(("%s no channel found with programNumber %u ltsid %u", BSTD_FUNCTION, programNumber, ltsid));
       return;
    }
    configureCAKeySlot(desABAKey,pChannel);
#else
    BDBG_MSG(("%s\n", BSTD_FUNCTION));
    BSTD_UNUSED(desABAKey);
    BSTD_UNUSED(programNumber);
    BSTD_UNUSED(ltsid);
#endif
    return;
}


void calcCciAckCb(
    uint8_t cpVersion,
    uint8_t cci,
    uint8_t *cpKeyA,
    uint8_t *cpKeyB,
    uint8_t *cciNCard,
    uint8_t *cciNHost,
    uint16_t programNumber,
    uint8_t ltsid,
    uint16_t ecmpid,
    uint8_t *cciAuth,
    uint8_t *cciAck
    )
{
    BDBG_MSG(("%s\n", BSTD_FUNCTION));
    BDBG_MSG(("Calculating new CP cci ack value\n"));
    B_Mpod_TestCpGenCciAck(cpVersion, cci, cpKeyA, cpKeyB, cciNCard,
                cciNHost, programNumber, ltsid, ecmpid, cciAuth, cciAck);
	info.cciCount++;
}


void enforceCciCb(
    uint8_t cci_data, uint16_t prog_num, uint8_t cci_ltsid
    )
{
    BDBG_MSG(("%s\n", BSTD_FUNCTION));
    BSTD_UNUSED(cci_ltsid);
    BSTD_UNUSED(prog_num);
    BDBG_MSG(("Received new CP cci status from the card, cci is 0x%x\n", cci_data));

#if SNMP_SUPPORT
{
    eWindowType type[2] = { eWindowType_Main, eWindowType_Pip };
    CCablecard * param = g_pCableCard;
    CChannel *pChannel = NULL;
    CParserBand * pBand = NULL;
    int i;
    for (i = 0; i < 2; i++)
    {
        if ((pChannel = param->_pModel->getCurrentChannel(type[i])) != NULL)
        {
            if ((pBand = pChannel->getParserBand()) != NULL)
            {
                if ((pBand->getBand() == cci_ltsid) && (pChannel->getProgramNum() == prog_num))
                {
                    pChannel->setCci(cci_data);
                    break;
                }
            }
        }
    }
}
#endif

    return;
}


void cpkeyGenCb(
    uint8_t *nHost,
    uint8_t *nCard,
    uint8_t *cpKeyA,
    uint8_t *cpKeyB
    )
{
    BDBG_MSG(("%s\n", BSTD_FUNCTION));
    BDBG_MSG(("Generating New CPKey"));

    B_Mpod_TestCpGenCPKey(nHost, nCard, cpKeyA, cpKeyB);
	info.CPkeyCount++;
}

void newValidationStatusCb(
    B_MPOD_CP_VALIDATION_STATUS validationStatus
    )
{

    BDBG_MSG(("%s\n", BSTD_FUNCTION));
    BDBG_MSG(("Received new CP validation status from the card, status is %d\n", validationStatus));
    info.status = validationStatus;
}

void resProfileInfoCb(
    uint8_t numStreams,
    uint8_t numProgs,
    uint8_t numEs
    )
{
   BDBG_MSG(("%s streams=%d  progs=%d  elem streams=%d\n", BSTD_FUNCTION, numStreams, numProgs, numEs));
}

static void getRootOID( uint8_t *data, uint32_t len)
{
    uint32_t i, j;
    char c, *rootoid, *suboid, *begin, *end;

    rootoid = (char *) malloc(len+2);

    info.rootoidLen = 0;

    for (i=0;i<len;i++)
    {
        c = data[i];
        if (isdigit(c))
        {
            sprintf((rootoid+i), "%d", c-'0');
        }
        else if (c == '.')
        {
            sprintf((rootoid+i), ".");
            info.rootoidLen++;
        }
        else
        {
            BDBG_WRN(("!!! not a valid CableCard Root OID !!!"));
            free(rootoid);
            return;
        }
    }

    *(rootoid+i) = '.';
    *(rootoid+i+1) = '\0';
    info.rootoidLen++;

    if (info.rootoid) free(info.rootoid);
    info.rootoid = (unsigned long *) malloc(info.rootoidLen*sizeof(unsigned long));

    j = 0;
    begin = end = rootoid;

    for (i = 0; i < len+1; i++)
    {
        if (*end == '.')
        {
            suboid = strndup(begin, (size_t)(end++-begin));
            *(info.rootoid+j++) = (unsigned long) atol(suboid);
            free(suboid);
            begin = end;
        }
        else
        {
            end++;
        }
    }

    printf("CableCard Root OID: ");
    for (i = 0; i < info.rootoidLen; i++)
    {
        printf("%lu", *(info.rootoid+i));
        if (i < (info.rootoidLen-1)) printf(".");
    }
    printf("\n");

    free(rootoid);
}

static void getSnmpReply( uint8_t *data,    uint32_t len)
{
    BSTD_UNUSED(data);
    BSTD_UNUSED(len);
	// will be passed back to SNMP proxy
	// by calling MPOD_RET_CODE B_Mpod_SnmpReq(uint8_t *snmpMsg, uint32_t len)

}

/*****************
** Host Properties
*****************/
static void hostPropertiesReplyCb(
    B_MPOD_PROP_HOST_PROPS *hostProperties
    )
{
    int i, j;

    BDBG_MSG(("%s\n\n", BSTD_FUNCTION));
    BDBG_MSG(("%d properties sent from the card\n\n", hostProperties->numOfProperties));

    for(i = 0; i < hostProperties->numOfProperties; i++)
    {
        BDBG_MSG(("Key: "));
        for(j = 0; j < hostProperties->properties[i].keyLength; j++)
            BDBG_MSG(("%02x ", hostProperties->properties[i].keyByte[j]));

        BDBG_MSG(("= "));
        for(j = 0; j < hostProperties->properties[i].valueLength; j++)
            BDBG_MSG(("%02x ", hostProperties->properties[i].valueByte[j]));

        BDBG_MSG(("\n"));
    }
}

/**************
** Headend Comm
**************/
static void rcvHostResetVectorCb(
    B_MPOD_HEADEND_HOST_RESET_VECTOR *hostResetVector
    )
{
    BDBG_MSG(("%s\n\n", BSTD_FUNCTION));

    BDBG_MSG(("Delay = %d\n", hostResetVector->delay));

    BDBG_MSG(("resetEcm = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESET_FIELD(hostResetVector, B_MPOD_HEADEND_RESET_ECM) ? "true" : "false"));
    BDBG_MSG(("resetSecurityElem = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESET_FIELD(hostResetVector, B_MPOD_HEADEND_RESET_SECURITY_ELEM) ? "true" : "false"));
    BDBG_MSG(("resetHost = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESET_FIELD(hostResetVector, B_MPOD_HEADEND_RESET_HOST) ? "true" : "false"));
    BDBG_MSG(("resetExternalDevices = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESET_FIELD(hostResetVector, B_MPOD_HEADEND_RESET_EXTERNAL_DEVICES) ? "true" : "false"));
    BDBG_MSG(("resetAll = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESET_FIELD(hostResetVector, B_MPOD_HEADEND_RESET_ALL) ? "true" : "false"));

    BDBG_MSG(("restartOcapStack = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESTART_FIELD(hostResetVector, B_MPOD_HEADEND_RESTART_OCAP_STACK) ? "true" : "false"));
    BDBG_MSG(("restartAll = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RESTART_FIELD(hostResetVector, B_MPOD_HEADEND_RESTART_ALL) ? "true" : "false"));

    BDBG_MSG(("reloadAllOcapApps = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RELOAD_APP_FIELD(hostResetVector, B_MPOD_HEADEND_RELOAD_ALL_OCAP_APPS) ? "true" : "false"));
    BDBG_MSG(("reloadOcapStack = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RELOAD_APP_FIELD(hostResetVector, B_MPOD_HEADEND_RELOAD_OCAP_STACK) ? "true" : "false"));

    BDBG_MSG(("reloadHostFirmware = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_RELOAD_FW_FIELD(hostResetVector, B_MPOD_HEADEND_RELOAD_HOST_FIRMWARE) ? "true" : "false"));

    BDBG_MSG(("clearPersistentGetFeatParams = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_PERSISTENT_GEN_FEAT_PARAMS) ? "true" : "false"));
    BDBG_MSG(("clearOrgDvbPersistentFs = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_ORG_DVB_PERSISTENT_FS) ? "true" : "false"));
    BDBG_MSG(("clearCachedUnboundApps = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_CACHED_UNBOUND_APPS) ? "true" : "false"));
    BDBG_MSG(("clearRegisteredLibs = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_REGISTERED_LIBS) ? "true" : "false"));
    BDBG_MSG(("clearPersistentHostMem = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_PERSISTENT_HOST_MEM) ? "true" : "false"));
    BDBG_MSG(("clearSecElemPassedValues = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_SEC_ELEM_PASSED_VALUES) ? "true" : "false"));
    BDBG_MSG(("clearNonAsdDvrContent = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_NON_ASD_DVR_CONTENT) ? "true" : "false"));
    BDBG_MSG(("clearAsdDvrContent = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_ASD_DVR_CONTENT) ? "true" : "false"));
    BDBG_MSG(("clearNetworkDvrContent = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_NETWORK_DVR_CONTENT) ? "true" : "false"));
    BDBG_MSG(("clearMediaVolInternalHdd = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_MEDIA_VOL_INTERNAL_HDD) ? "true" : "false"));
    BDBG_MSG(("clearMediaVolExternalHdd = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_MEDIA_VOL_EXTERNAL_HDD) ? "true" : "false"));
    BDBG_MSG(("clearGpfsInternalHdd = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_GPFS_INTERNAL_HDD) ? "true" : "false"));
    BDBG_MSG(("clearGpfsExternalHdd = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_GPFS_EXTERNAL_HDD) ? "true" : "false"));
    BDBG_MSG(("clearAllStorage = %s\n",
        B_MPOD_HEADEND_GET_HOST_VEC_CLEARING_FIELD(hostResetVector, B_MPOD_HEADEND_CLEAR_ALL_STORAGE) ? "true" : "false"));

    BDBG_MSG(("\n"));
}
void CCablecard::cablecard_init()
{
	B_MPOD_APINFO_SETTINGS apInfoSettings;
	B_MPOD_DSG_SETTINGS dsgSettings;
	B_MPOD_EXT_CH_SETTINGS extChSettings;
   	B_MPOD_RES_SETTINGS resSettings = {5 /* max simul xpt streams*/,&resProfileInfoCb};
    B_MPOD_MMI_SETTINGS mmiSettings = {&mmiDialogRequestCb, &mmiDialogCloseCb,
        &mmiHtmlRdyCb, (B_MPOD_MMI_CB_EXTERN_RDY)NULL, (B_MPOD_MMI_CB_EXTERN_RCV)NULL, &mmiSessionOpenCb,
        &mmiSessionCloseCb};

    B_MPOD_SAS_SETTINGS sasSettings = {&sasConnectCnfCallback, &sasConnectionRdyCallback,
        &sasSynchDataRcvCallback, &sasAsynchDataRcvCallback, &sasGetSynchDataCallback,
        NULL, NULL, &sasConnectionClosedCallback};

    B_MPOD_DL_SETTINGS dlSettings = {&dlHostInfoCb,&dlNewCodeVersionTableType1Cb, &dlNewCodeVersionTableType2Cb};

    B_MPOD_FEATURE_SETTINGS featureSettings = {&featureReqHostListCb, &featureReqParamsCb,
                &featureRcvCardListCb, &featureRcvParamsCb, &featureParamDeniedCb};

    B_MPOD_HOST_CONTROL_SETTNINGS hostControlSettings = {&inbandTuneCb, &oobTxTuneCb,
                                                &oobRxTuneCb, &sourceIdToFreqCb};

    B_MPOD_HOMING_SETTINGS homingSettings = {&homingTimeoutCb, &delayedDownloadReqCb,
                    &homingCompleteCb, &downloadStartingCb, &downloadCompleteCb};
#if 0
    B_MPOD_DIAG_SETTINGS diagSettings = {&diagReqCb};
#endif
    B_MPOD_SYSTIME_SETTINGS systimeSettings = {NULL};

    B_MPOD_CA_SETTINGS caSettings = {&caInfoCb, &caPmtUpdate, &caPmtReply};
    B_MPOD_CP_SETTINGS cpSettings = {&getAuthKeyCb, &cardAuthMsgCb,
        &getNonceCb, &cpkeyGenCb, &removeKeyCb, &progKeyCb, &calcCciAckCb,
        &enforceCciCb, &newValidationStatusCb, &getIDCb};
	B_MPOD_SNMP_SETTINGS snmpSettings = { &getRootOID, &getSnmpReply};
    B_MPOD_HEADEND_COMM_SETTINGS headendCommSettings = {&rcvHostResetVectorCb};

    B_MPOD_PROP_HOST_PROPS_SETTINGS hostPropertySettings = {&hostPropertiesReplyCb};


 	BKNI_Memset(SasConnections, 0, sizeof(SasConnections));

/* Initialize the resource manager */
	if(B_Mpod_ResrcMgrInit())
	{
		printf("Unable to initialize resource manager\n");
		exit(1);
	}


    if(B_Mpod_SasInit(&sasSettings))
    {
        printf("Unable to initialize SAS resource\n");
        exit(1);
    }


/* DSG Settings */
	dsgSettings.dsgErrorCb = &CableCardCallback_DSG_Packet_Error;
	dsgSettings.dsgRcvSetModeCb = &CableCardCallback_DSGSet_DSG_Mode;
	dsgSettings.dsgRcvDirectoryCb = &CableCardCallback_DSG_Directory;
    if(B_Mpod_DsgInit(&dsgSettings))
    {
        printf("Unable to initialize DSG resource\n");
        exit(1);
    }

/* Ext Channel Settings */
	extChSettings.reqIpUnicastFlowCbThread = &CableCardCallback_IPU_Dhcp;
	extChSettings.reqIpMulticastFlowCb = &CableCardCallback_Req_Ipm_Flow;
	extChSettings.reqDsgFlowCb = &CableCardCallback_DSG_Flow_Id;
	extChSettings.reqSocketFlowCbThread = &CableCardCallback_Socket_Flow_Config;
	extChSettings.flowReqFailedCb = &CableCardCallback_Flow_Req_Failed;
	extChSettings.newFlowCnfCb = &CableCardCallback_New_Flow_Cnf_Cb;
	extChSettings.delFlowReqCb = &CableCardCallback_Delete_Flow_Req;
	extChSettings.delFlowCnfCb = &CableCardCallback_Del_Flow_Cnf_Cb;
	extChSettings.lostFlowIndCb = &CableCardCallback_Lost_Flow_Cb;
	extChSettings.rcvSetDsgModeCb = &CableCardCallback_ExtSet_DSG_Mode;
	extChSettings.dsgErrorCb = &CableCardCallback_DSG_Packet_Error;
	extChSettings.configAdvDsgCb = &CableCardCallback_Config_Advanced_DSG;
	extChSettings.rcvFlowDataCb = &CableCardCallback_Rcv_Flow_Data;

    if(B_Mpod_ExtendedChInit(&extChSettings))
    {
        printf("Unable to initialize Extended Channel resource\n");
        exit(1);
    }

/* App Info */
    B_Mpod_AppInfoGetDefaultCapabilities(&apInfoSettings);

	apInfoSettings.apInfoExternalHandlerRcvCb   = NULL;
	apInfoSettings.apInfoChangedCb              = &apInfoInfo_Changed_Cb;
    if(B_Mpod_AppInfoInit(&apInfoSettings))
    {
        printf("Unable to initialize ApInfo resource\n");
        exit(1);
    }

    if(B_Mpod_MmiInit(&mmiSettings))
    {
        printf("Unable to initialize MMI resource\n");
        exit(1);
    }

    if(B_Mpod_DownloadInit(&dlSettings))
    {
        printf("Unable to initialize Download resource\n");
        exit(1);
    }

    if(B_Mpod_FeatureInit(&featureSettings))
    {
        printf("Unable to initialize Feature resource\n");
        exit(1);
    }

    if(B_Mpod_HostControlInit(&hostControlSettings))
    {
        printf("Unable to initialize Host Control resource\n");
        exit(1);
    }

    if(B_Mpod_HomingInit(&homingSettings))
    {
        printf("Unable to initialize Homing resource\n");
        exit(1);
    }
#if 0
    if(B_Mpod_DiagsInit(&diagSettings))
    {
        printf("Unable to initialize General Diagnostic resource\n");
        exit(1);
    }
#endif

    if(B_Mpod_SystimeInit(&systimeSettings))
    {
        printf("Unable to initialize Systime resource\n");
        exit(1);
    }

    if(B_Mpod_CaInit(&caSettings))
    {
        printf("Unable to initialize Systime resource\n");
        exit(1);
    }

    if(B_Mpod_CpInit(&cpSettings))
    {
        printf("Unable to initialize Systime resource\n");
        exit(1);
    }

    if(B_Mpod_ResInit(&resSettings))
    {
        printf("Unable to initialize Res resource\n");
        exit(1);
    }

    if(B_Mpod_SnmpInit(&snmpSettings))
    {
        printf("Unable to initialize SNMP resource\n");
        exit(1);
    }

    if(B_Mpod_HostPropertiesInit(&hostPropertySettings))
    {
        printf("Unable to initialize Host Property resource\n");
        exit(1);
    }

    if(B_Mpod_HeadendCommInit(&headendCommSettings))
    {
        printf("Unable to initialize HE resource\n");
        exit(1);
    }
}


#endif
