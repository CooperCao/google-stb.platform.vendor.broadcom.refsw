/******************************************************************************
 *  Copyright (C) 2008-2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

/* Example to do a satellite peak scan followed by a blind acquisition */

#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#include "nexus_platform.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MANUAL_MODE 0

BKNI_EventHandle psCallbackEvent;
BKNI_EventHandle lockCallbackEvent;
NEXUS_FrontendSatellitePeakscanResult psResult;

/* maximum room for list of detected signals */
#define LIST_SIZE 100
typedef struct list_node {
    unsigned freq;
    unsigned sym;
    bool locked;
    bool timing_loop_locked;
    bool no_acquire;
} list_node;
static int num_signals = 0;
static list_node list_of_signals[LIST_SIZE];

static void print_usage(void)
{
    printf(
        "Usage: scan_satellite\n"
        "  --help or -h for help\n"
        "  -freq   #     frequency in Hz, e.g. 1119000000\n"
        "  -minsym #     min symbol rate in baud, e.g. 15000000\n"
        "  -maxsym #     max symbol rate in baud, e.g. 30000000\n"
        "  -range  #     frequency range, e.g. 5000000\n"
        "  -step   #     frequency step in Hz, e.g. 1000000\n"
        "  -psd          enable PSD\n"
        );
    printf(
        "  -debug        enable debug messages for peakscan code\n"
        "  -manual       manual mode\n"
        );
    printf(
        "  -list         continual peakscan over full range, listing signals found and locked\n"
        "  -sweep        sweep full satellite range (950MHz to 2150MHz)\n"
        "  -noacq        skip blind acquisition on a detected signal\n"
        "  -timing       check timing lock\n"
        );
}

static void peakscan_callback(void *context, int param) 
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendSatellitePeakscanResult result;
    NEXUS_Error rc;

    BSTD_UNUSED(param);

    rc = NEXUS_Frontend_GetSatellitePeakscanResult(frontend, &result);
    BDBG_ASSERT(!rc);
    fprintf(stderr, "Peak scan callback. freq=%u, symRate=%u, power=%#x\n", 
        result.peakFrequency, result.symbolRate, result.peakPower);
    psResult = result;
    BKNI_SetEvent(psCallbackEvent);
}

static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendSatelliteStatus status;

    BSTD_UNUSED(param);

    NEXUS_Frontend_GetSatelliteStatus(frontend, &status);
    fprintf(stderr, "Lock callback. symRate=%u, locked=%u, mode=%u\n", 
        status.settings.symbolRate, status.demodLocked, status.mode);
    BKNI_SetEvent(lockCallbackEvent);
}

static void update_signal_list(NEXUS_FrontendSatellitePeakscanResult *pResult, bool locked, bool timing_loop_locked, bool no_acquire)
{
    list_node *p = &list_of_signals[num_signals];
    p->freq = pResult->peakFrequency;
    p->sym = pResult->symbolRate;
    p->locked = locked;
    p->timing_loop_locked = timing_loop_locked;
    p->no_acquire = no_acquire;
    num_signals++;
    if (num_signals > LIST_SIZE) {
        printf("Error -- too many signals. Recompile after increasing LIST_SIZE.\n");
        BKNI_Fail();
    }
}

static void update_scan_parameters(NEXUS_FrontendSatellitePeakscanSettings *pSettings, NEXUS_FrontendSatellitePeakscanResult *pResult)
{
    unsigned range;
    unsigned new_middle;
    unsigned old_top;
    unsigned last_frequency;

    last_frequency = pResult->lastFrequency;
    last_frequency -= 750000; /* back up very slightly in case of overlap at the edge */
    old_top = pSettings->frequency + pSettings->frequencyRange;
    range = (old_top - last_frequency)/2;
    new_middle = last_frequency + range;

    pSettings->frequency = new_middle;
    pSettings->frequencyRange = range;
}

#define TIMING_LOOP_TIMEOUT_MS 2000

int main(int argc, char **argv)
{
    NEXUS_FrontendHandle frontend=NULL;
    NEXUS_FrontendSatelliteSettings satSettings;
    NEXUS_FrontendSatellitePeakscanSettings psSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_FrontendAcquireSettings settings;
    NEXUS_Error rc=NEXUS_SUCCESS;
    unsigned input;
    int i;

    int curarg = 1;
    unsigned frequency = 1120 * 1000000;
    unsigned range = 5 * 1000000;
    unsigned step = 1 * 1000000;
    unsigned minsym = 15 * 1000000;
    unsigned maxsym = 30 * 1000000;
    bool psd = false;
    bool manual = false;
    bool debug = false;
    bool list = false;
    bool sweep = false;
    bool no_acquire = false;
    bool check_timing_lock = false;

#if MANUAL_MODE
    manual = 1;
#endif

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-freq") && argc>curarg+1) {
            frequency = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-minsym") && argc>curarg+1) {
            minsym = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-maxsym") && argc>curarg+1) {
            maxsym = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-range") && argc>curarg+1) {
            range = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-step") && argc>curarg+1) {
            step = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-psd")) {
            psd = true;
        }
        else if (!strcmp(argv[curarg], "-debug")) {
            debug = true;
        }
        else if (!strcmp(argv[curarg], "-nodebug")) {
            debug = false;
        }
        else if (!strcmp(argv[curarg], "-manual")) {
            manual = true;
        }
        else if (!strcmp(argv[curarg], "-sweep")) {
            sweep = true;
        }
        else if (!strcmp(argv[curarg], "-list")) {
            list = true;
        }
        else if (!strcmp(argv[curarg], "-timing")) {
            check_timing_lock = true;
        }
        else if (!strcmp(argv[curarg], "-noacq") || !strcmp(argv[curarg], "-noacquire") || !strcmp(argv[curarg], "-no_acquire")) {
            no_acquire = true;
        }
        else {
            print_usage();
            return 1;
        }
        curarg++;
    }

    if (sweep) {
        frequency = 1550000000;
        range = 600000000;
        minsym =  1500000;
        maxsym = 35000000;
        if (psd) {
            /* PSD requires a smaller step */
            step = 250000;
        }
    }

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);
    
    printf("fetching frontend...\n"); fflush(stdout);

    NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
    settings.capabilities.satellite = true;
    frontend = NEXUS_Frontend_Acquire(&settings);
    if (!frontend) {
        fprintf(stderr, "Unable to find satellite-capable frontend\n");
        return -1;
    }
    if (debug) {
        BDBG_SetModuleLevel("nexus_frontend_ast",BDBG_eMsg);
        BDBG_SetModuleLevel("nexus_frontend_sat",BDBG_eMsg);
    }

    BKNI_CreateEvent(&psCallbackEvent);
    BKNI_CreateEvent(&lockCallbackEvent);

    printf("Starting scan...\n"); fflush(stdout);

if (!manual) {
    NEXUS_Frontend_GetDefaultSatellitePeakscanSettings(&psSettings);
    psSettings.frequency = frequency;
    psSettings.frequencyRange = range;
    psSettings.frequencyStep = step;
    psSettings.minSymbolRate = minsym;
    psSettings.maxSymbolRate = maxsym;
    psSettings.peakscanCallback.callback = peakscan_callback;
    psSettings.peakscanCallback.context = frontend;
    psSettings.mode = psd ? NEXUS_FrontendSatellitePeakscanMode_ePowerSpectrumDensity : NEXUS_FrontendSatellitePeakscanMode_eSymbolRateScan;

    while (1) {
        if (!list) {
            printf("Press Enter to start %s peak scan\n", psd ? "psd" : "symbol rate");
            getchar();
        }
        if ((psSettings.frequencyRange <= psSettings.frequencyStep) || (psSettings.frequency > 2148000000U)) {
            printf("Trying to peakscan off the end...\n");
            goto done;
        }
        rc = NEXUS_Frontend_SatellitePeakscan(frontend, &psSettings);
        BDBG_ASSERT(!rc);
        rc = BKNI_WaitForEvent(psCallbackEvent, 60000);
        if (rc) {
            printf("Callback not fired. Aborting\n");
            rc = 1;
            goto done;
        }
        else {
            bool timing_loop_locked = false;
            unsigned acq_timeout = 5000;

            if (!psResult.peakFrequency) {
                printf("No signal found. Continuing peak scan...\n");
                update_scan_parameters(&psSettings, &psResult);
                continue;
            }
            if (check_timing_lock) {
                NEXUS_FrontendSatelliteSignalDetectStatus signalStatus;

                NEXUS_Frontend_GetDefaultSatelliteSettings(&satSettings);
                satSettings.mode = NEXUS_FrontendSatelliteMode_e8pskLdpc; /* not used */
                satSettings.codeRate.numerator = 2;
                satSettings.codeRate.denominator = 3;
                satSettings.lockCallback.callback = NULL;
                satSettings.lockCallback.context = NULL;
                satSettings.frequency = psResult.peakFrequency;
                satSettings.symbolRate = psResult.symbolRate;
                satSettings.carrierOffset = 0;
                satSettings.signalDetectMode = true;

                rc = NEXUS_Frontend_TuneSatellite(frontend, &satSettings);
                if (rc) {
                    printf("signal detect tune attempt failed (error on call: %x)...\n",rc);
                }

                for (i = TIMING_LOOP_TIMEOUT_MS; i; i--) {
                    rc = NEXUS_Frontend_GetSatelliteSignalDetectStatus(frontend, &signalStatus);
                    if (rc) {
                        printf("signal detect status call failed (error on call: %x)...\n",rc);
                    }

                    /* sanity check */
                    if (signalStatus.enabled == false)
                    {
                       printf("NEXUS_Frontend_GetSatelliteSignalDetectStatus() error: enabled should not be false!\n");
                       break;
                    }

                    if (signalStatus.detectionComplete)
                       break;

                    BKNI_Sleep(1);
                }
                if (i > 0)
                {
                    if (!signalStatus.signalDetected)
                    {
                        printf("Unable to lock timing loop\n");
                        update_signal_list(&psResult, false, timing_loop_locked, no_acquire);
                        goto skip_acquire;
                    }
                    else
                    {
                        timing_loop_locked = true;
                        printf("Timing loop is locked (%d ms)\n", TIMING_LOOP_TIMEOUT_MS-i);
                    }
                }
                else
                   printf("ERROR: signal detect function timeout\n");
            }

            if (no_acquire) {
                update_signal_list(&psResult, false, timing_loop_locked, no_acquire);
                goto skip_acquire;
            }

            /* found an interesting peak frequency. try blind acquisition */
            if (!list) {
                printf("Press Enter to attempt blind acquisition\n");
                getchar();
            }
            NEXUS_Frontend_GetDefaultSatelliteSettings(&satSettings);
            satSettings.mode = NEXUS_FrontendSatelliteMode_eBlindAcquisition;
            satSettings.lockCallback.callback = lock_callback;
            satSettings.lockCallback.context = frontend;
            satSettings.frequency = psResult.peakFrequency;
            satSettings.symbolRate = psResult.symbolRate;
            satSettings.carrierOffset = 0;
            if (psd)
                satSettings.bypassFrequencyEstimation = true;

            /* limit search range for low symbol rates */
            if (psResult.symbolRate > 8000000) {
                satSettings.searchRange = 5000000;
                acq_timeout = 7500;
            } else {
                satSettings.searchRange = psResult.symbolRate / 2;
                if (satSettings.searchRange > 3000000)
                    satSettings.searchRange = 3000000;
                else if (satSettings.searchRange < 1000000)
                    satSettings.searchRange = 1000000;
                acq_timeout = 5000;
            }

            rc = NEXUS_Frontend_TuneSatellite(frontend, &satSettings);
            BDBG_ASSERT(!rc);

            rc = BKNI_WaitForEvent(lockCallbackEvent, acq_timeout);
            if (rc) {
                printf("Could not lock. Continuing peak scan...\n");
                update_signal_list(&psResult, false, timing_loop_locked, no_acquire);
                update_scan_parameters(&psSettings, &psResult);
                continue;
            }
            else {
                /* finished */
                input = 1;
                if (!list) {
                    printf("Enter 0 to quit, 1 to continue\n");
                    scanf("%u", &input);
                }

                {
                    NEXUS_FrontendSatelliteStatus satStatus;
                    rc = NEXUS_Frontend_GetSatelliteStatus(frontend, &satStatus);
                    if (!rc) {
                        printf("freq: %4.2f, error: %8u", (double)(psResult.lastFrequency)/1000000.0, satStatus.carrierError);
                        psResult.lastFrequency += satStatus.carrierError;
                        printf(", new freq: %4.2f\n", (double)psResult.lastFrequency/1000000.0);
                    }
                }

                update_signal_list(&psResult, true, timing_loop_locked, no_acquire);
                if (input==0) {
                    rc = 0;
                    goto done;
                }
                else {
skip_acquire:
                    update_scan_parameters(&psSettings, &psResult);
                    continue;
                }
            }
        }
    }
} else {
    NEXUS_Frontend_GetDefaultSatellitePeakscanSettings(&psSettings);
    psSettings.frequencyRange = 0;
    psSettings.peakscanCallback.callback = peakscan_callback;
    psSettings.peakscanCallback.context = frontend;
    psSettings.frequency = 1110 * 1000000;

    printf("Enter 0 to quit, 1 to continue to next frequency, 2 to try blind acquisition\n");
    for (i=0; i<20; i++) {
        scanf("%u", &input);
        switch (input) {
            case 0:
                rc = 0;
                goto done;
                break;
            case 1:
                psSettings.frequency += 1 * 1000000;
                rc = NEXUS_Frontend_SatellitePeakscan(frontend, &psSettings);
                BDBG_ASSERT(!rc);
                rc = BKNI_WaitForEvent(psCallbackEvent, 3000);
                if (rc) {
                    printf("Callback not fired. Aborting\n");
                    rc = 1;
                    goto done;
                }
                continue;
                break;
            case 2:
            default:
                NEXUS_Frontend_GetDefaultSatelliteSettings(&satSettings);
                satSettings.mode = NEXUS_FrontendSatelliteMode_eBlindAcquisition;
                satSettings.lockCallback.callback = lock_callback;
                satSettings.lockCallback.context = frontend;
                satSettings.frequency = psResult.peakFrequency;
                satSettings.symbolRate = psResult.symbolRate;
                satSettings.carrierOffset = 0;
                rc = NEXUS_Frontend_TuneSatellite(frontend, &satSettings);
                BDBG_ASSERT(!rc);

                rc = BKNI_WaitForEvent(lockCallbackEvent, 3000);
                if (rc) {
                    printf("Could not lock. Press Enter to continue\n");
                    getchar();
                }
                break;
        }
    }

}

done:
    BKNI_DestroyEvent(psCallbackEvent);
    BKNI_DestroyEvent(lockCallbackEvent);
    NEXUS_Frontend_Release(frontend);
    NEXUS_Platform_Uninit();


    if (list) {
        printf("\n\n");
        printf("Signals found:\n");
        for (i=0;i<LIST_SIZE;i++) {
            list_node *p = &list_of_signals[i];
            if (p->freq) {
                double f,s;
                f = (double)p->freq / 1000000.0;
                s = (double)p->sym / 1000000.0;
                printf("%3d: freq: %4.2f\tsym: %2.1f\tlock: %s\n",i,f,s,p->no_acquire ? "did not try" : p->locked ? "yes" : "no");
            }
        }
    }

    return rc;
}
#else  /* if NEXUS_HAS_FRONTEND */
#include <stdio.h>
int main(void)
{
    printf("ERROR: This platform doesn't include frontend.inc \n");
    return -1;
}
#endif /* if NEXUS_HAS_FRONTEND */

