/******************************************************************************
 *  Copyright (C) 2008-2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#include "nexus_platform.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

BDBG_MODULE(spectrumdata);

#define NUM_SAMPLES 200

uint32_t *dataPointer;
uint32_t *currentDataPointer;
unsigned totalDataSamplesRead;

typedef struct callback_data_t {
    NEXUS_FrontendHandle frontend;
    BKNI_EventHandle event;
} callback_data_t;

static void spectrum_data_ready_callback(void *context, int param)
{
    callback_data_t *callbackData = (callback_data_t *)context;
    unsigned dataCount;
    BKNI_EventHandle spectrumEvent;
    BSTD_UNUSED(param);

    BDBG_ASSERT(NULL != context);
    spectrumEvent = callbackData->event;

    for (dataCount=0; dataCount<NUM_SAMPLES; dataCount++){
        printf("Data[0x%x] = 0x%x   ", dataCount, *(dataPointer+dataCount));
    }

    BKNI_SetEvent(spectrumEvent);
}

int main(int argc, char **argv)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_FrontendHandle frontend = NULL;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_FrontendAcquireSettings settings;
    BKNI_EventHandle spectrumEvent = NULL;
    NEXUS_FrontendSpectrumSettings spectrumSettings;
    callback_data_t spectrumCallbackData;
    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
    settings.capabilities.qam = true;
    frontend = NEXUS_Frontend_Acquire(&settings);
    if (!frontend) {
        fprintf(stderr, "Unable to find QAM-capable frontend\n");
        return -1;
    }

    BKNI_CreateEvent(&spectrumEvent);
    NEXUS_Memory_Allocate(NUM_SAMPLES*4, NULL, (void*)&dataPointer);
    currentDataPointer = dataPointer;

    spectrumSettings.data = dataPointer;
    spectrumSettings.binAverage = 2;
    spectrumSettings.numSamples = NUM_SAMPLES;
    spectrumSettings.startFrequency = 0;
    spectrumSettings.stopFrequency = 1200000000;
    spectrumSettings.fftSize = 1024;
    spectrumSettings.dataReadyCallback.callback = spectrum_data_ready_callback;
    spectrumCallbackData.frontend = frontend;
    spectrumCallbackData.event = spectrumEvent;
    spectrumSettings.dataReadyCallback.context = &spectrumCallbackData;
    spectrumSettings.dataReadyCallback.param = 0;

    rc = NEXUS_Frontend_RequestSpectrumData(frontend, &spectrumSettings);
    if (rc){rc = BERR_TRACE(rc);  goto done;}

    rc = BKNI_WaitForEvent(spectrumEvent, 5000);
    if (rc == NEXUS_TIMEOUT) {
        printf("Spectrum data retrieval time out\n");
        goto done;
    }
    printf("Spectrum data read completely\n");

done:
    if (dataPointer) {
        NEXUS_Memory_Free(dataPointer);
    }
    if (spectrumEvent) {
        BKNI_DestroyEvent(spectrumEvent);
    }

    NEXUS_Frontend_Untune(frontend);
    NEXUS_Frontend_Release(frontend);
    NEXUS_Platform_Uninit();
    return 0;
}
#else  /* if NEXUS_HAS_FRONTEND */
#include <stdio.h>
int main(void)
{
    printf("ERROR: This platform doesn't include frontend.inc \n");
    return -1;
}
#endif /* if NEXUS_HAS_FRONTEND */

