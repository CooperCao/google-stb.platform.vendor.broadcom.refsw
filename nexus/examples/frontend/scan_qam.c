/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *   Frontend QAM scan
 *
 *   This sample app demonstrates how to use qam scan feature.
 *   1. Frequencies.txt file populated with the frequencies inteded to scan must be copied into nexus/bin folder.
 *   2. Frequencies.txt file should have one frequency(in Hz) per line.
 *   3. NEXUS_Frontend_TuneQam() frontend api is used to scan one frequency at a time.
 *   4. NEXUS_FrontendQamSettings structure is used to define all the required scan parameters.
 *   5. Every frequency tuned/scanned, irrespective of the outcome generates atleast TWO callbacks.
 *   6. First callback is to denote that a new frequency is being tuned/scanned.
 *   7. Time to wait for the first callback can be varied using UNLOCK_TIMEOUT.
 *   8. Second callback denotes the completion of the scanning for the frequence and mode once.
 *   8. NEXUS_Frontend_GetFastStatus() is used to determine if the channel is locked/unlocked/no_signal.
 *   9. If the channel is found, the NEXUS_Frontend_GetFastStatus() returns locked status.
 * 10. Then the application calls NEXUS_Frontend_GetQamScanStatus() to retreive complete qam scan results and then proceeds to scan the next frequency.
 * 11. If the channel is not found, the NEXUS_Frontend_GetFastStatus() returns unlocked/no_signal.
 * 11. Then the application can either wait to see if the scan algorithm can reacquire or it can proceed to scan for the next frequency/mode.
 * 12. In this app, we proceed to scan for the next frequency/mode.
 * 13. The results are printed in results.txt file in nexus/bin folder and also displayed on screen.
 *
 ***************************************************************************/
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#include "nexus_platform.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

BDBG_MODULE(tune_qam);

#define LINE_BUF_SIZE 256

/* Max scan wait time in ms. */
#define SCAN_TIMEOUT 2000

unsigned int lockCount;
unsigned int unLockCount;
unsigned int noSginalCount;
FILE *pFileResults;
uint32_t start_time, end_time, start, end;

unsigned long gettime(void)
{
    int rc;
    struct timespec now;
    /* It's ok to use clock_gettime even without NPTL. */
    rc = clock_gettime(CLOCK_MONOTONIC, &now);
    if (rc) {
        rc = BERR_TRACE(BERR_OS_ERROR);
        return 0;
    }
    return now.tv_sec * 1000000 + now.tv_nsec / 1000;
}

static void lock_callback(void *context, int param)
{
    BKNI_EventHandle statusEvent = (BKNI_EventHandle)context;
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)statusEvent);
}

int main(int argc, char **argv)
{
    NEXUS_FrontendHandle frontend=NULL;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_FrontendAcquireSettings settings;
    unsigned tot_chn = 0;
    BKNI_EventHandle statusEvent = NULL;
    FILE *pFileFreq;
    NEXUS_Error rc, retValue;
    char freqBuf[LINE_BUF_SIZE];
    NEXUS_FrontendQamScanStatus scanStatus;
    NEXUS_FrontendFastStatus fastStatus;

    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    pFileFreq = fopen("frequencies.txt", "r");
    if ( NULL == pFileFreq )
    {
        printf("frequencies.txt file not found. Copy this file into nexus/bin folder.\n");
        goto done;
    }

    pFileResults = fopen("results.txt", "w");
    if ( NULL == pFileResults )
    {
        printf("results.txt file not found.\n");
        goto done;
    }

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);
    BKNI_CreateEvent(&statusEvent);

    NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
    settings.capabilities.qam = true;
    frontend = NEXUS_Frontend_Acquire(&settings);
    if (!frontend) {
        fprintf(stderr, "Unable to find QAM-capable frontend\n");
        goto done;
    }


    NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
    qamSettings.frequency = 765000000;
    qamSettings.mode = NEXUS_FrontendQamMode_e64;
    switch (qamSettings.mode) {
        default:
        case NEXUS_FrontendQamMode_e64: qamSettings.symbolRate = 5056900; break;
        case NEXUS_FrontendQamMode_e256: qamSettings.symbolRate = 5360537; break;
        case NEXUS_FrontendQamMode_e1024: qamSettings.symbolRate = 0; break;
    }
    qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
    qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
    qamSettings.lockCallback.callback = lock_callback;
    qamSettings.lockCallback.context = statusEvent;
    qamSettings.lockCallback.param = 0;
    qamSettings.autoAcquire = true;
    qamSettings.enablePowerMeasurement = false;
    qamSettings.frequencyOffset = 250000;
    qamSettings.acquisitionMode = NEXUS_FrontendQamAcquisitionMode_eScan;
    qamSettings.spectrumMode = NEXUS_FrontendQamSpectrumMode_eAuto;
    qamSettings.enableNullPackets = false;
    qamSettings.scan.upperBaudSearch = 5360537;
    qamSettings.scan.lowerBaudSearch = 5056941;
    qamSettings.scan.mode[NEXUS_FrontendQamAnnex_eB][NEXUS_FrontendQamMode_e64] = true;
    qamSettings.scan.mode[NEXUS_FrontendQamAnnex_eB][NEXUS_FrontendQamMode_e256] = true;
    qamSettings.scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e64] = true;
    qamSettings.scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e256] = true;

    fprintf(stderr, "\n\nStart Channel Scan.\n");
    start = (uint32_t) gettime();
    while ( fgets(freqBuf, LINE_BUF_SIZE, pFileFreq) )
    {
        tot_chn++;
        qamSettings.frequency = atoi(freqBuf);
        fprintf(stderr, "Tuning frequency %d\n", qamSettings.frequency);

        fastStatus.lockStatus = NEXUS_FrontendLockStatus_eUnknown;

        rc = NEXUS_Frontend_TuneQam(frontend, &qamSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        while(1) {
            /* Waiting for the lock change callback for dvbt2. */
            rc = BKNI_WaitForEvent(statusEvent, SCAN_TIMEOUT);

            retValue = NEXUS_Frontend_GetFastStatus(frontend, &fastStatus);
            if(retValue){retValue = BERR_TRACE(retValue); rc = retValue; goto done;}

            if(rc){
                if(rc == NEXUS_TIMEOUT){
                    BDBG_WRN(("Scan timed out. Continuing to next frequency."));
                    break;
                }
                else {
                    rc = BERR_TRACE(rc); goto done;
                }
            }

            BDBG_MSG(("fastStatus.lockStatus = %d, fastStatus.acquireInProgress = %d", fastStatus.lockStatus, fastStatus.acquireInProgress));

            if((fastStatus.lockStatus == NEXUS_FrontendLockStatus_eNoSignal) || (fastStatus.lockStatus == NEXUS_FrontendLockStatus_eLocked))break;
            if((!fastStatus.acquireInProgress) && (fastStatus.lockStatus != NEXUS_FrontendLockStatus_eUnlocked))break;
        }

        if(rc == NEXUS_TIMEOUT) {
            if(fastStatus.lockStatus == NEXUS_FrontendLockStatus_eUnlocked)unLockCount++;
            continue;
        }
        else if(fastStatus.lockStatus == NEXUS_FrontendLockStatus_eLocked){
            lockCount++;
            fprintf(pFileResults, "\nFrequency  %dHz, Locked\n",qamSettings.frequency);

            rc = NEXUS_Frontend_GetQamScanStatus(frontend, &scanStatus);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            else {
                fprintf(pFileResults, "Scan Status: \n");
                fprintf(pFileResults, "SymbolRate = %d\n", scanStatus.symbolRate);
                fprintf(pFileResults, "Mode = %d\n", scanStatus.mode);
                fprintf(pFileResults, "Annex = %d\n", scanStatus.annex);
                fprintf(pFileResults, "Interleaver = %d\n", scanStatus.interleaver);
                fprintf(pFileResults, "Spectrum inverted = %s\n", scanStatus.spectrumInverted?"True":"False");
                fprintf(pFileResults, "Aacquisition status = %d\n\n\n", scanStatus.acquisitionStatus);
            }
        }
        else if(fastStatus.lockStatus == NEXUS_FrontendLockStatus_eUnlocked){
            unLockCount++;
            fprintf(pFileResults, "\nFrequency  %dHz, Unlocked\n",qamSettings.frequency);
        }
        else if(fastStatus.lockStatus == NEXUS_FrontendLockStatus_eNoSignal){
            noSginalCount++;
            fprintf(pFileResults, "\nNo Signal at frequency  %dHz\n",qamSettings.frequency);
        }
        else{
            fprintf(stderr, "Unrecognized lock status. lockStatus = %d\n", fastStatus.lockStatus);
        }
    }
    end = (uint32_t) gettime();
    fprintf(pFileResults, "\n\nTotal scan time: %d seconds\n", (end - start)/1000000);
    fprintf(stderr, "\n\nTotal scan time: %d milli-seconds\n", (end - start)/1000);

    fprintf(pFileResults, "Total Channels Scanned: %d, locked channels: %d, unlocked channels: %d, no signal channels: %d\n", tot_chn, lockCount, unLockCount, noSginalCount);
    fprintf(stderr, "Total Channels Scanned: %d, locked channels: %d, unlocked channels: %d, no signal channels: %d\n", tot_chn, lockCount, unLockCount, noSginalCount);

done:
    if(pFileFreq)fclose (pFileFreq);
    if(pFileResults)fclose (pFileResults);
    if (frontend) {
        NEXUS_Frontend_Untune(frontend);
        NEXUS_Frontend_Release(frontend);
    }
    if (statusEvent) BKNI_DestroyEvent(statusEvent);
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

