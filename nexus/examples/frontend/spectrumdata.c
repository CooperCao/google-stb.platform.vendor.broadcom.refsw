/******************************************************************************
 *    (c)2008-2013 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Example code to demonstrate spectrum data retieval.
 * NEXUS_FrontendSpectrumSettings is used to specify the input parameters for requesting the spectrum data.
 * The memory needs to be allocated such that it is >= numSamples*4 as numSamples is 4byte long each.
 * NUM_SAMPLES can be changed to requested the required number of frequency samples from the FFT output.
 * Then call NEXUS_Frontend_RequestSpectrumData(). Once the requested frequency data is ready for the application to be used,
 * Nexus fires a callback registered in the input parameter dataReadyCallback.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
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

static void spectrum_data_ready_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend;
    unsigned dataCount;
    BKNI_EventHandle spectrumEvent = (BKNI_EventHandle)param;

    BDBG_ASSERT(NULL != context);
    frontend = (NEXUS_FrontendHandle)context;

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
    unsigned freq;
    NEXUS_FrontendQamMode qamMode;
    BKNI_EventHandle spectrumEvent = NULL;
    unsigned codec = -1;
    NEXUS_FrontendSpectrumSettings spectrumSettings;
    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    codec = NEXUS_VideoCodec_eMpeg2;
    qamMode = NEXUS_FrontendQamMode_e64;
    freq = 765;
    
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
    spectrumSettings.dataReadyCallback.context = frontend;
    spectrumSettings.dataReadyCallback.param = (int)spectrumEvent;
    
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

