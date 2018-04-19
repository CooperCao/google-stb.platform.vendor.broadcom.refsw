/***************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 * Module Description:
 *
 ************************************************************/
#include "nexus_platform_module.h"
#include "nexus_platform_audio_log.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

BDBG_MODULE(nexus_platform_audio_log);

#if NEXUS_HAS_AUDIO && NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
#include "nexus_audio.h"
#include "nexus_audio_dsp_private.h"
#define NEXUS_PLATFORM_AUDIO_LOG 1
#endif

#if NEXUS_PLATFORM_AUDIO_LOG
#include "bape.h"
static bool g_audioLogThreadDone;
static pthread_t g_audioLogThread;

static void *NEXUS_Platform_P_AudioLogThread(void *pParam)
{
    unsigned dev, devType, type;
    const char *pEnv;
    char *pCoreFileName = "audio_core";
    const void *pBuffer;
    size_t bufferSize;
    NEXUS_Error errCode;
    NEXUS_AudioCapabilities audioCaps;
    unsigned numDevices;

    struct
    {
        bool enabled;
        struct
        {
            FILE *pFile;
            unsigned index;
        } dsp[BAPE_DEVICE_DSP_NUM];
        struct
        {
            FILE *pFile;
            unsigned index;
        } arm[BAPE_DEVICE_ARM_NUM];
    } debugData[NEXUS_AudioDspDebugType_eMax];

    BKNI_Memset(&debugData, 0, sizeof(debugData));
    BSTD_UNUSED(pParam);

    NEXUS_GetAudioCapabilities(&audioCaps);

    /* Initialize */
    for ( type = 0; type < NEXUS_AudioDspDebugType_eMax; type++ )
    {
        pEnv = NULL;
        switch ( type )
        {
        case NEXUS_AudioDspDebugType_eDramMessage:
            pEnv = NEXUS_GetEnv("audio_debug_file");
            if ( NULL == pEnv && NEXUS_GetEnv("audio_logs_enabled") )
            {
                pEnv = "audio_debug";
            }
            break;
        case NEXUS_AudioDspDebugType_eUartMessage:
            pEnv = NEXUS_GetEnv("audio_uart_file");
            if ( NULL == pEnv && NEXUS_GetEnv("audio_logs_enabled") )
            {
                pEnv = "audio_uart";
            }
            break;
        case NEXUS_AudioDspDebugType_eCoreDump:
            pEnv = NEXUS_GetEnv("audio_core_file");
            if ( NULL == pEnv && NEXUS_GetEnv("audio_logs_enabled") )
            {
                pEnv = pCoreFileName;
            }
            pCoreFileName = (char*)pEnv;
            break;
        case NEXUS_AudioDspDebugType_eTargetPrint:
            pEnv = NEXUS_GetEnv("audio_target_print_file");
            if ( NULL == pEnv && NEXUS_GetEnv("audio_logs_enabled") )
            {
                pEnv = "audio_target_print";
            }
            break;
        }
        if ( NULL != pEnv )
        {
            debugData[type].enabled = true;

            for ( devType = 0; devType < BAPE_DEVICE_TYPE_MAX; devType++ )
            {
                FILE * pFile = NULL;
                const char *devTypeName;

                switch ( devType )
                {
                case BAPE_DEVICE_TYPE_DSP:
                    devTypeName = "dsp";
                    numDevices = audioCaps.numDsps;
                    break;
                case BAPE_DEVICE_TYPE_ARM:
                    devTypeName = "arm";
                    numDevices = audioCaps.numSoftAudioCores;
                    break;
                }

                for ( dev = 0; dev < numDevices; dev++ )
                {
    #define AUDIO_LOG_PATH_MAX 128
                    char pathname[AUDIO_LOG_PATH_MAX];
                    if ( type == NEXUS_AudioDspDebugType_eCoreDump )
                    {
                        snprintf(pathname, sizeof(pathname), "%s.%s.%u.0", pEnv, devTypeName, dev);
                    }
                    else
                    {
                        snprintf(pathname, sizeof(pathname), "%s.%s.%u", pEnv, devTypeName, dev);
                    }
                    /* coverity[tainted_string] */
                    pFile = fopen(pathname, "wb+");
                    if ( NULL == pFile )
                    {
                        goto exit;
                    }

                    switch ( devType )
                    {
                    case BAPE_DEVICE_TYPE_DSP:
                        debugData[type].dsp[dev].pFile = pFile;
                        debugData[type].dsp[dev].index = 0;
                        break;
                    case BAPE_DEVICE_TYPE_ARM:
                        debugData[type].arm[dev].pFile = pFile;
                        debugData[type].arm[dev].index = 0;
                        break;
                    }
                }
            }
        }
    }

    while ( false == g_audioLogThreadDone )
    {
        for ( type = 0; type < NEXUS_AudioDspDebugType_eMax; type++ )
        {
            if ( debugData[type].enabled )
            {
                for ( devType = 0; devType < BAPE_DEVICE_TYPE_MAX; devType++ )
                {
                    FILE * pFile = NULL;
                    unsigned index = 0;
                    unsigned offset = 0;

                    switch ( devType )
                    {
                    case BAPE_DEVICE_TYPE_DSP:
                        numDevices = audioCaps.numDsps;
                        break;
                    case BAPE_DEVICE_TYPE_ARM:
                        numDevices = audioCaps.numSoftAudioCores;
                        break;
                    }

                    for ( dev = 0; dev < numDevices; dev++ )
                    {
                        const char *devTypeName;
                        bool logged=false;

                        switch ( devType )
                        {
                        case BAPE_DEVICE_TYPE_DSP:
                            devTypeName = "dsp";
                            pFile = debugData[type].dsp[dev].pFile;
                            index = debugData[type].dsp[dev].index;
                            break;
                        case BAPE_DEVICE_TYPE_ARM:
                            devTypeName = "arm";
                            pFile = debugData[type].arm[dev].pFile;
                            index = debugData[type].arm[dev].index;
                            offset = BAPE_DEVICE_ARM_FIRST;
                            break;
                        }

                        do
                        {
                            errCode = NEXUS_AudioDsp_GetDebugBuffer(dev+offset, type, &pBuffer, &bufferSize);
                            if ( errCode != BERR_SUCCESS || bufferSize == 0 )
                            {
                                break;
                            }
                            (void)fwrite(pBuffer, bufferSize, 1, pFile);
                            fflush(pFile);
                            NEXUS_AudioDsp_DebugReadComplete(dev+offset, type, bufferSize);
                            BDBG_MSG(("Wrote %u bytes of debug type %u", (unsigned)bufferSize, type));
                            logged = true;
                        } while ( 1 );

                        if ( logged == true && type == NEXUS_AudioDspDebugType_eCoreDump )
                        {
                            char pathname[AUDIO_LOG_PATH_MAX];
                            /* Increment file name for each core dump */
                            fclose(pFile);
                            index++;
                            snprintf(pathname, sizeof(pathname), "%s.%s.%u.%u", pCoreFileName, devTypeName, dev, index);
                            BDBG_MSG(("Opening audio core dump file %s", pathname));
                            /* coverity[tainted_string] */
                            pFile = fopen(pathname, "wb+");
                            if ( NULL == pFile )
                            {
                                BDBG_ERR(("Unable to open audio debug logfile %s", pathname));
                                goto exit;
                            }

                            switch ( devType )
                            {
                            case BAPE_DEVICE_TYPE_DSP:
                                debugData[type].dsp[dev].index = index;
                                debugData[type].dsp[dev].pFile = pFile;
                                break;
                            case BAPE_DEVICE_TYPE_ARM:
                                debugData[type].arm[dev].index = index;
                                debugData[type].arm[dev].pFile = pFile;
                                break;
                            }
                        }
                    }
                }
            }
        }

        /* global debug processes */
        for ( devType = 0; devType < BAPE_DEVICE_TYPE_MAX; devType++ )
        {
            switch ( devType )
            {
            case BAPE_DEVICE_TYPE_DSP:
                numDevices = audioCaps.numDsps;
                break;
            case BAPE_DEVICE_TYPE_ARM:
                numDevices = audioCaps.numSoftAudioCores;
                break;
            }

            for ( dev = 0; dev < numDevices; dev++ )
            {
                unsigned offset = 0;

                switch ( devType )
                {
                case BAPE_DEVICE_TYPE_DSP:
                    break;
                case BAPE_DEVICE_TYPE_ARM:
                    offset = BAPE_DEVICE_ARM_FIRST;
                    break;
                }

                if ( NEXUS_GetEnv("audio_debug_service_enabled") )
                {
                    errCode = NEXUS_AudioDspPrivate_RunDebugService(dev+offset);
                    if ( errCode != BERR_SUCCESS )
                    {
                        break;
                    }
                }
            }
        }
        BKNI_Sleep(100);
    }

exit:
    for ( type = 0; type < NEXUS_AudioDspDebugType_eMax; type++ )
    {
        for ( devType = 0; devType < BAPE_DEVICE_TYPE_MAX; devType++ )
        {
            switch ( devType )
            {
            case BAPE_DEVICE_TYPE_DSP:
                numDevices = audioCaps.numDsps;
                break;
            case BAPE_DEVICE_TYPE_ARM:
                numDevices = audioCaps.numSoftAudioCores;
                break;
            }

            for ( dev = 0; dev < numDevices; dev++ )
            {
                FILE * pFile = NULL;

                switch ( devType )
                {
                case BAPE_DEVICE_TYPE_DSP:
                    pFile = debugData[type].dsp[dev].pFile;
                    break;
                case BAPE_DEVICE_TYPE_ARM:
                    pFile = debugData[type].arm[dev].pFile;
                    break;
                }

                if ( pFile != NULL )
                {
                    fclose(pFile);
                }
            }
        }
    }

    pthread_exit(NULL);
    return NULL;
}
#endif

NEXUS_Error NEXUS_Platform_P_InitAudioLog(void)
{
#if NEXUS_PLATFORM_AUDIO_LOG
    if ( NEXUS_GetEnv("audio_uart_file") ||
         NEXUS_GetEnv("audio_debug_file") ||
         NEXUS_GetEnv("audio_core_file") ||
         NEXUS_GetEnv("audio_target_print_file") ||
         NEXUS_GetEnv("audio_logs_enabled") ||
         NEXUS_GetEnv("audio_debug_service_enabled") )
    {
        if ( pthread_create(&g_audioLogThread, NULL, NEXUS_Platform_P_AudioLogThread, NULL) )
        {
            return BERR_TRACE(BERR_OS_ERROR);
        }
    }
#endif

    return BERR_SUCCESS;
}

void NEXUS_Platform_P_UninitAudioLog(void)
{
#if NEXUS_PLATFORM_AUDIO_LOG
    if ( (pthread_t) NULL != g_audioLogThread )
    {
        g_audioLogThreadDone = true;
        pthread_join(g_audioLogThread, NULL);
        g_audioLogThread = (pthread_t)NULL;
        g_audioLogThreadDone = false;
    }
#endif
}
