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

#include "bstd.h"

#include <stdio.h>
#include <signal.h>

#ifdef STACK_TRACE
#include <sys/resource.h>
#include "brcm_sig_hdl.h"
#endif

BDBG_MODULE(atlas_main);

#ifdef NXCLIENT_SUPPORT
#include "atlas_main_nx.h"
CConfigNx * g_pConfig = NULL;
#else
#include "atlas_main.h"
CConfig * g_pConfig = NULL;
#endif /* ifdef NXCLIENT_SUPPORT */

B_ThreadHandle    gSchedulerThread;
B_MutexHandle     gLock;
B_SchedulerHandle gScheduler;

#ifdef CDL_SUPPORT
static bool cdl_started = false;
static bool CDL_ENABLED = false;
#endif

/**
 * main.cpp must be recompiled every time in order to make sure this
 * function returns the last build date. Other files can use this
 * and so don't have to be recompiled every time.
 **/
const char * buildDate()
{
    return(__DATE__ " " __TIME__);
}

#define STACKGUARD_FIX()        \
    uint8_t stackReserve[8192]; \
    BKNI_Memset(stackReserve, 0, sizeof(stackReserve));

#ifdef NXCLIENT_SUPPORT
eRet initializeNexus()
{
    eRet             ret    = eRet_Ok;
    NEXUS_Error      nerror = NEXUS_SUCCESS;
    CConfiguration * pCfg   = NULL;

    BDBG_ASSERT(NULL != g_pConfig);

    pCfg = g_pConfig->getCfg();
    CHECK_PTR_ERROR_GOTO("configuration failure", pCfg, ret, eRet_InvalidParameter, error);

    STACKGUARD_FIX() /* DTT do we really need this? */

    {
        NxClient_JoinSettings joinSettings;

        NxClient_GetDefaultJoinSettings(&joinSettings);
        snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", GET_STR(pCfg, ATLAS_NAME));

        /* dac bandgap, dac detection, and max transport rate options can be set using nxserver command line options */
        if (0 != GET_INT(pCfg, DAC_BANDGAP_ADJUST))
        {
            BDBG_WRN(("DAC bandgap adjustments in atlas.cfg have no effect in nxclient mode.  Use nxserver command line options instead."));
        }

        if (0 != GET_INT(pCfg, DAC_DETECTION))
        {
            BDBG_WRN(("DAC detection in atlas.cfg has no effect in nxclient mode.  Use nxserver command line options instead."));
        }

        if (0 != GET_INT(pCfg, MAXDATARATE_PARSERBAND))
        {
            BDBG_WRN(("Parserband max data rate adjustements in atlas.cfg have no effect in nxclient mode.  Use nxserver command line options instead."));
        }

        if (0 != GET_INT(pCfg, MAXDATARATE_PLAYBACK))
        {
            BDBG_WRN(("Playback max data rate adjustements in atlas.cfg have no effect in nxclient mode.  Use nxserver command line options instead."));
        }

        nerror = NxClient_Join(&joinSettings);
        CHECK_NEXUS_ERROR_GOTO("error calling NxClientJoin()", ret, nerror, error);

        /* enable audio decoder output delay for SPDIF/HDMI/DAC */
        {
            NxClient_AudioSettings settings;

            NxClient_GetAudioSettings(&settings);
            settings.hdmi.additionalDelay  = GET_INT(pCfg, HDMI_OUTPUT_DELAY);
            settings.spdif.additionalDelay = GET_INT(pCfg, SPDIF_OUTPUT_DELAY);
            settings.dac.additionalDelay   = GET_INT(pCfg, DAC_OUTPUT_DELAY);
            NxClient_SetAudioSettings(&settings);
        }
    }

error:
    return(ret);
} /* initializeNexus */

#else /* NXCLIENT_SUPPORT */

eRet initializeNexus()
{
    eRet             ret    = eRet_Ok;
    NEXUS_Error      nerror = NEXUS_SUCCESS;
    CConfiguration * pCfg   = NULL;

    BDBG_ASSERT(NULL != g_pConfig);

    pCfg = g_pConfig->getCfg();
    CHECK_PTR_ERROR_GOTO("configuration failure", pCfg, ret, eRet_InvalidParameter, error);

#if 0
    STACKGUARD_FIX() /* DTT do we really need this? */
#endif

    {
        /* coverity[stack_use_local_overflow] */
        NEXUS_PlatformSettings platformSettings;

        NEXUS_Platform_GetDefaultSettings(&platformSettings);
        /* DTT make modification to platformSettings based on config */

        /* initializing the frontend is slow so we'll do this after we have everything else in
         * the app up and running */
        /*platformSettings.openFrontend = false;*//* dtt todo - change to false and then init in idle loop */

        /* modify DAC bandgap */
        {
            uint32_t dacBandGapAdjust = GET_INT(pCfg, DAC_BANDGAP_ADJUST);

            if (0 < dacBandGapAdjust)
            {
                for (int i = 0; i < NEXUS_MAX_VIDEO_DACS; i++)
                {
                    BDBG_WRN(("DAC%d Bandgap Adjustment old:%d new:%d", i, platformSettings.displayModuleSettings.dacBandGapAdjust[i], dacBandGapAdjust));
                    platformSettings.displayModuleSettings.dacBandGapAdjust[i] = dacBandGapAdjust;
                }
            }
        }

        /* turn on/off optional video DAC detection */
        {
            NEXUS_VideoDacDetection dacDetection = (NEXUS_VideoDacDetection)GET_INT(pCfg, DAC_DETECTION);
            if (NEXUS_VideoDacDetection_eAuto < dacDetection)
            {
                BDBG_WRN(("Video DAC detection %s", (NEXUS_VideoDacDetection_eOn == dacDetection) ? "ON" : "OFF"));
                platformSettings.displayModuleSettings.dacDetection = dacDetection;
            }
        }

        /* modify transport max data rate if specified in atlas.cfg */
        {
            uint32_t maxDataRateParserBand = GET_INT(pCfg, MAXDATARATE_PARSERBAND);
            uint32_t maxDataRatePlayback   = GET_INT(pCfg, MAXDATARATE_PLAYBACK);

            if (0 < maxDataRateParserBand)
            {
                for (int i = 0; i < NEXUS_MAX_PARSER_BANDS; i++)
                {
                    BDBG_WRN(("Max Data Rate for PARSER BAND %d changed to:%d", i, maxDataRateParserBand));
                    platformSettings.transportModuleSettings.maxDataRate.parserBand[i] = maxDataRateParserBand;
                }
            }
            if (0 < maxDataRatePlayback)
            {
                for (int i = 0; i < NEXUS_MAX_PLAYPUMPS; i++)
                {
                    BDBG_MSG(("Max Data Rate for PLAYPUMP %d changed to:%d", i, maxDataRatePlayback));
                    platformSettings.transportModuleSettings.maxDataRate.playback[i] = maxDataRatePlayback;
                }
            }
        }

        /* enable audio decoder output delay for SPDIF/HDMI/DAC */
        {
            uint32_t delaySpdif = GET_INT(pCfg, SPDIF_OUTPUT_DELAY);
            uint32_t delayHdmi  = GET_INT(pCfg, HDMI_OUTPUT_DELAY);
            uint32_t delayDac   = GET_INT(pCfg, DAC_OUTPUT_DELAY);

            if (0 < (delaySpdif + delayHdmi + delayDac))
            {
                platformSettings.audioModuleSettings.independentDelay = true;
#if NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
                platformSettings.audioModuleSettings.maxIndependentDelay = MAX(delaySpdif, MAX(delayHdmi, delayDac));
                BDBG_ERR(("maxIndependentDelay:%d", platformSettings.audioModuleSettings.maxIndependentDelay));
#endif
            }
        }
#ifdef B_HAS_DTCP_IP
        /* Due to latest SAGE restrictions EXPORT_HEAP needs to be initialized even if we are not using SVP/EXPORT_HEAP(XRR).
         * It could be any small size heap.
         * Configure export heap since it's not allocated by nexus by default */
        platformSettings.heap[NEXUS_EXPORT_HEAP].size = 32*1024;
#endif

#ifdef MPOD_SUPPORT
#if ((NEXUS_PLATFORM == 97278) || (NEXUS_PLATFORM == 97271))
        for (int i = 0; i < MAX_CABLECARD_ROUTE; i++)
        {
            /* enable mpodRs only for parsers that need to send data to the cablecard, which will cause an additional 175k or 200k per parserBand to be allocated from the device heap. */
            platformSettings.transportModuleSettings.clientEnabled.parserBand[i].mpodRs = true;
            platformSettings.transportModuleSettings.maxDataRate.parserBand[i]          = 108000000;         /* 54000000;*/
        }
#endif /* if ((NEXUS_PLATFORM == 97278) || (NEXUS_PLATFORM == 97271)) */
#endif /* ifdef MPOD_SUPPORT */

        /* coverity[stack_use_overflow] */
        nerror = NEXUS_Platform_Init(&platformSettings);
        CHECK_NEXUS_ERROR_GOTO("unable to initialize nexus", ret, nerror, error);
    }

error:
    return(ret);
} /* initializeNexus */

#endif /* NXCLIENT_SUPPORT */

void uninitializeNexus()
{
#ifdef NXCLIENT_SUPPORT
    NxClient_Uninit();
#else /* NXCLIENT_SUPPORT */
    NEXUS_Platform_Uninit();
#endif /* NXCLIENT_SUPPORT */
} /* uninitializeNexus */

static void runScheduler(void * pParam)
{
    B_SchedulerHandle scheduler = (B_SchedulerHandle)pParam;

    B_Scheduler_Run(scheduler);
}

eRet initializeOsLib()
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    nerror = B_Os_Init();
    CHECK_NEXUS_ERROR_GOTO("Failed to initialize os library", ret, nerror, err_os);

    gLock = B_Mutex_Create(NULL);
    CHECK_PTR_ERROR_GOTO("Failed to create mutex", gLock, ret, eRet_ExternalError, err_mutex);

    gScheduler = B_Scheduler_Create(NULL);
    CHECK_PTR_ERROR_GOTO("failed to create scheduler", gScheduler, ret, eRet_ExternalError, err_scheduler);

    /* run scheduler with thread */
    {
        B_ThreadSettings settings;

        B_Thread_GetDefaultSettings(&settings);

        gSchedulerThread = B_Thread_Create("b_event", runScheduler, gScheduler, &settings);
        CHECK_PTR_ERROR_GOTO("failed to create scheduler thread", gSchedulerThread, ret, eRet_ExternalError, err_thread);

        /* yield so sheduler can get up and running */
        B_Thread_Sleep(1);
    }

    goto done;

err_thread:
    B_Scheduler_Stop(gScheduler);
    B_Scheduler_Destroy(gScheduler);
err_scheduler:
    B_Mutex_Destroy(gLock);
err_mutex:
    B_Os_Uninit();
err_os:
done:
    return(ret);
} /* initializeOsLib */

void uninitializeOsLib()
{
    if (gScheduler)
    {
        B_Scheduler_Stop(gScheduler);
        B_Scheduler_Destroy(gScheduler);
    }

    if (gSchedulerThread)
    {
        B_Thread_Destroy(gSchedulerThread);
    }

    if (gLock)
    {
        B_Mutex_Destroy(gLock);
    }

    B_Os_Uninit();
} /* uninitializeOsLib */

eRet initializeAtlas()
{
    eRet      ret  = eRet_Ok;
    BERR_Code berr = BERR_SUCCESS;

#ifdef PLAYBACK_IP_SUPPORT
    BIP_Status bipStatus = BIP_SUCCESS;
#endif
    berr = BKNI_Init();
    BDBG_ASSERT(BERR_SUCCESS == berr);

#ifdef NXCLIENT_SUPPORT
    g_pConfig = new CConfigNx();
#else
    g_pConfig = new CConfig();
#endif /* ifdef NXCLIENT_SUPPORT */

    BDBG_MSG(("*** initialize nexus"));
    ret = initializeNexus();
    CHECK_ERROR_GOTO("Nexus failed to initialize", ret, err_initNexus);

    BDBG_MSG(("*** initialize os lib"));
    ret = initializeOsLib();
    CHECK_ERROR_GOTO("OS lib failed to initialize", ret, err_initOsLib);

#ifdef PLAYBACK_IP_SUPPORT
    {
        {
            bipStatus = BIP_Init(NULL);
            CHECK_BIP_ERROR_GOTO("BIP lib failed to initialize", ret, bipStatus, err_initBipLib);
        }
#ifdef B_HAS_DTCP_IP
        {
            bipStatus = BIP_DtcpIpClientFactory_Init(NULL);
            CHECK_BIP_ERROR_GOTO("BIP_DtcpIpClientFactoryInit() failed to initialize", ret, bipStatus, err_initBipLib);
        }
#endif /* ifdef B_HAS_DTCP_IP */
#ifdef B_HAS_SSL
        {
            #define TEST_ROOT_CA_PATH  "./host.cert"
            BIP_SslClientFactoryInitSettings settings;

            BIP_SslClientFactory_GetDefaultInitSettings(&settings);
            settings.pRootCaCertPath = TEST_ROOT_CA_PATH;
            bipStatus                = BIP_SslClientFactory_Init(&settings);
            CHECK_BIP_ERROR_GOTO("BIP_SslClientFactory_Init() failed to initialize", ret, bipStatus, err_initDtcpIp);
        }
#endif /* ifdef B_HAS_SSL */
    }
#endif /* ifdef PLAYBACK_IP_SUPPORT */
    goto done;

#ifdef PLAYBACK_IP_SUPPORT
#ifdef B_HAS_SSL
err_initDtcpIp:
#endif
#ifdef B_HAS_DTCP_IP
    BIP_DtcpIpClientFactory_Uninit();
#endif
err_initBipLib:
    BIP_Uninit();
#endif /* ifdef PLAYBACK_IP_SUPPORT */
err_initOsLib:
    uninitializeOsLib();
err_initNexus:
    uninitializeNexus();
done:
    return(ret);
} /* initializeAtlas */

void uninitializeAtlas()
{
#ifdef PLAYBACK_IP_SUPPORT
    {
#ifdef B_HAS_SSL
        BIP_SslClientFactory_Uninit();
#endif
#ifdef B_HAS_DTCP_IP
        BIP_DtcpIpClientFactory_Uninit();
#endif
        BIP_Uninit();
    }
#endif /* ifdef PLAYBACK_IP_SUPPORT */
    uninitializeOsLib();
    uninitializeNexus();

    DEL(g_pConfig);
    BKNI_Uninit();
} /* uninitializeAtlas */

/**
 * This is the universal entry point for Atlas and
 * is entry point for Linux. Other operating systems (like VxWorks)
 * have other entry points which then call main().
 **/

static void cleanExit(int sig)
{
    CConfiguration * pCfg = g_pConfig->getCfg();

    BDBG_ASSERT(NULL != pCfg);
    BSTD_UNUSED(sig);

    SET(pCfg, EXIT_APPLICATION, "true");
}

static void readCmdLineParams(
        int              argc,
        char **          argv,
        CConfiguration * pCfg
        )
{
    for (int i = 1; i < argc; i++)
    {
        if ((i < argc) && !strcasecmp(argv[i], "-script"))
        {
            SET(pCfg, LUA_SCRIPT, argv[++i]);
            BDBG_WRN(("-script specified:%s", GET_STR(pCfg, LUA_SCRIPT)));

            /* since we are running a lua script, don't bother with initial tune and idle tuning */
            SET(pCfg, FIRST_TUNE, false);
            SET(pCfg, ENABLE_IDLE_TUNE, false);
        }

        if ((i < argc) && !strcasecmp(argv[i], "-noprompt"))
        {
            SET(pCfg, ENABLE_LUA, false);
            i++;
        }

        if ((i < argc) && !strcasecmp(argv[i], "-cfg"))
        {
            BDBG_WRN(("-cfg specified:%s", argv[i+1]));
            pCfg->read(argv[++i]);
        }
    }
} /* readCmdLineParams */

int main(
        int     argc,
        char ** argv
        )
{
    eRet ret = eRet_Ok;

    if (SIG_ERR == signal(SIGINT, cleanExit))
    {
        BDBG_WRN(("Error setting signal handler %d", SIGINT));
    }

    fprintf(stderr,
            "Brutus 2.0/Atlas (C)2016, Broadcom\n"
            "  Built on %s, BCHP_CHIP %d\n", buildDate(), BCHP_CHIP);

    ret = initializeAtlas();
    CHECK_ERROR_GOTO("atlas initialization failed", ret, error);

    {
        eRet ret = eRet_Ok;
        CChannelMgr channelMgr;
#if DVR_LIB_SUPPORT
        char virtualDevice[] = "/data";
        CDvrMgr dvrMgr;
        bool vsfsEnabled = true;
#endif
        CConfiguration * pCfg = NULL;
        char * pListName      = NULL;

#if DVR_LIB_SUPPORT
        BDBG_MSG(("unmounting the loop devices in case application wasn't shutdown properly"));
        system("umount /dev/loop0");
        system("umount /dev/loop1");
        system("umount /dev/loop2");
        dvrMgr.init(eB_DVR_MediaStorageTypeLoopDevice);
        ret = dvrMgr.registerStorageDevice(0, virtualDevice, 50*1024);
        if (ret != eRet_Ok)
        {
            vsfsEnabled = false;
            dvrMgr.unInit();
        }
        else
        {
            dvrMgr.mountStorageDevice(0);
            dvrMgr.reserveTsbSpace(0, 2, 30, 40);
        }
#endif /* if DVR_LIB_SUPPORT */
        /* read chips info, board features, and board resources */
        ret = g_pConfig->initialize();
        CHECK_ERROR_GOTO("failed to initialize model config", ret, error);
        /*g_pConfig->dump();*/ /* debug */

        pCfg = g_pConfig->getCfg();
        CHECK_PTR_ERROR_GOTO("configuration failure", pCfg, ret, eRet_InvalidParameter, error);
        channelMgr.setCfg(pCfg);
        /* Enable DTCP , then Disable in the end ! */
        readCmdLineParams(argc, argv, pCfg);

        /* load default channel list */
        pListName = (char *)GET_STR(pCfg, CHANNELS_LIST);
        if ((NULL != pListName) && (0 < strlen(pListName)))
        {
            if (eRet_Ok != channelMgr.loadChannelList(pListName))
            {
                BDBG_WRN(("channel list XML version MISmatch (%s)", pListName));
            }
            channelMgr.dumpChannelList(); /* debug - set parameter to true (force) or add "atlas_channel" to msg_modules */
        }

        {
            CAtlas * pAtlas0 = NULL;
            unsigned number  = 0;
            CLua * pLua      = NULL;

            if (true == GET_BOOL(pCfg, ENABLE_LUA))
            {
                pLua = new CLua();
                CHECK_PTR_ERROR_GOTO("unable to allocate pLua", pLua, ret, eRet_OutOfMemory, error);

                ret = pLua->initialize(g_pConfig);
                CHECK_ERROR_GOTO("lua failed to initialize", ret, error);
            }

#ifdef NXCLIENT_SUPPORT
            pAtlas0 = new CAtlasNx(number, eAtlasMode_SingleDisplay, &channelMgr, pLua);
            CHECK_PTR_ERROR_GOTO("unable to instantiate CAtlas", pAtlas0, ret, eRet_OutOfMemory, error);
#else
#if DVR_LIB_SUPPORT
            pAtlas0 = new CAtlas(number, eAtlasMode_SingleDisplay, &channelMgr, vsfsEnabled ? &dvrMgr : NULL, pLua);
#else
            pAtlas0 = new CAtlas(number, eAtlasMode_SingleDisplay, &channelMgr, pLua);
#endif
            CHECK_PTR_ERROR_GOTO("unable to instantiate CAtlas", pAtlas0, ret, eRet_OutOfMemory, error);
#endif /* ifdef NXCLIENT_SUPPORT */

            number++;

#ifndef NXCLIENT_SUPPORT
            /*
             * some resources are pre-reserved for each instance of atlas.  other resources will be
             * dynamically checked out by each atlas instance based on given eAtlasMode.
             */
            {
                CResource * pResource             = NULL;
                CBoardResources * pBoardResources = NULL;

                pBoardResources = g_pConfig->getBoardResources();

                pResource = pBoardResources->reserveResource(pAtlas0, eBoardResource_outputComponent);
                CHECK_PTR_MSG("component output resource unavailable for reservation", pResource, ret, eRet_NotSupported);
                pResource = pBoardResources->reserveResource(pAtlas0, eBoardResource_outputComposite);
                CHECK_PTR_MSG("composite output resource unavailable for reservation", pResource, ret, eRet_NotSupported);
                pResource = pBoardResources->reserveResource(pAtlas0, eBoardResource_outputRFM);
                CHECK_PTR_MSG("RFM output resource unavailable for reservation", pResource, ret, eRet_NotSupported);
                pResource = pBoardResources->reserveResource(pAtlas0, eBoardResource_outputHdmi);
                CHECK_PTR_MSG("hdmi output resource unavailable for reservation", pResource, ret, eRet_NotSupported);
            }
#endif /* NXCLIENT_SUPPORT */

            pAtlas0->initialize(g_pConfig);
            BKNI_Printf("\nAtlas is running.\n\n");
            pAtlas0->run();

            pAtlas0->uninitialize();
            DEL(pAtlas0);
            DEL(pLua);
        }

        g_pConfig->uninitialize();
#if DVR_LIB_SUPPORT
        if (vsfsEnabled)
        {
            dvrMgr.unReserveTsbSpace(0);
            dvrMgr.unMountStorageDevice(0);
            dvrMgr.unInit();
        }
#endif /* if DVR_LIB_SUPPORT */
    }

    uninitializeAtlas();
    signal(SIGINT, SIG_DFL);

    return(0);

error:
    return(-1);
} /* main */