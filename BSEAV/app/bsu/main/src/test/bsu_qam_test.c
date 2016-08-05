/***************************************************************************
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
 *
 * Module Description:
 *
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <os.h>
#include "bsu_prompt.h"

#include "nexus_frontend.h"
#include "nexus_platform.h"
#include "nexus_parser_band.h"

BDBG_MODULE(tune_qam);

/* the following define the input and its characteristics -- these will vary by input stream */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define VIDEO_PID 0x11
#define AUDIO_PID 0x14

/***********************************************************************
 *                      External References
 ***********************************************************************/
extern bool frontend_already_opened;

static void lock_changed_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void async_status_ready_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

void bsu_qam_test(void)
{
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendHandle frontend = NULL;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_Error rc;
    /* default freq & qam mode */
    unsigned freq = 765;
    NEXUS_FrontendQamMode qamMode = NEXUS_FrontendQamMode_e64;
    BKNI_EventHandle statusEvent, lockChangedEvent;
    bool done = false;

    if (frontend_already_opened==false) {
        rc = NEXUS_Platform_InitFrontend();
        if(rc){rc = BERR_TRACE(rc); goto done;}
    }

    {
        NEXUS_FrontendAcquireSettings settings;
        NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
        settings.capabilities.qam = true;
        frontend = NEXUS_Frontend_Acquire(&settings);
        if (!frontend) {
            BDBG_ERR(("Unable to find QAM-capable frontend"));
            return;
        }
    }

    BKNI_CreateEvent(&statusEvent);
    BKNI_CreateEvent(&lockChangedEvent);

    while (!done) {
        bool acquired = false;

        NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
        qamSettings.frequency = freq * 1000000;
        qamSettings.mode = qamMode;
        switch (qamMode) {
        default:
        case NEXUS_FrontendQamMode_e64: qamSettings.symbolRate = 5056900; break;
        case NEXUS_FrontendQamMode_e256: qamSettings.symbolRate = 5360537; break;
        case NEXUS_FrontendQamMode_e1024: qamSettings.symbolRate = 0; /* TODO */ break;
        }
        qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
        qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
        qamSettings.lockCallback.callback = lock_changed_callback;
        qamSettings.lockCallback.context = lockChangedEvent;
        qamSettings.asyncStatusReadyCallback.callback = async_status_ready_callback;
        qamSettings.asyncStatusReadyCallback.context = statusEvent;

        NEXUS_Frontend_GetUserParameters(frontend, &userParams);

        /* Map a parser band to the demod's input band. */
        parserBand = NEXUS_ParserBand_e0;
        NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
        if (userParams.isMtsif) {
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
            parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(frontend); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
        }
        else {
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
            parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
        }
        parserBandSettings.transportType = TRANSPORT_TYPE;
        NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

        BKNI_ResetEvent(lockChangedEvent);

        BDBG_WRN(("tuning %d MHz...", freq));
        rc = NEXUS_Frontend_TuneQam(frontend, &qamSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        /* in a real-world app, we don't start decode until we are locked and have scanned for PSI */
        {
            OS_ERR err;
            unsigned start_time = OSTimeGet(&err);
            while (1) {
                unsigned current_time = OSTimeGet(&err);
                NEXUS_FrontendFastStatus status;

/* MAX_ACQUIRE_TIME is the application timeout */
#define MAX_ACQUIRE_TIME 2000
                if (current_time - start_time > MAX_ACQUIRE_TIME) {
                    BDBG_WRN(("application timeout. cannot acquire."));
                    break;
                }
                rc = BKNI_WaitForEvent(lockChangedEvent, MAX_ACQUIRE_TIME - (current_time - start_time));
                if (rc == BERR_TIMEOUT) {
                    BDBG_WRN(("application timeout. cannot acquire."));
                    break;
                }
                BDBG_ASSERT(!rc);

                rc = NEXUS_Frontend_GetFastStatus(frontend, &status);
                if (rc == NEXUS_SUCCESS) {
                    if (status.lockStatus == NEXUS_FrontendLockStatus_eLocked) {
                        BDBG_WRN(("frontend locked"));
                        acquired = true;
                        break;
                    }
                    else if (status.lockStatus == NEXUS_FrontendLockStatus_eUnlocked) {
                        BDBG_WRN(("frontend unlocked (acquireInProgress=%d)", status.acquireInProgress));
                        if (!status.acquireInProgress) {
                            break;
                        }
                        /* if acquisition is still in progress, we have to wait more.
                        we get a locked changed callback when transitioning from previous acquire to current acquire,
                        even when lockStatus is unchanged. */
                    }
                    else if (status.lockStatus == NEXUS_FrontendLockStatus_eNoSignal) {
                        BDBG_WRN(("No signal at the tuned frequency"));
                        break;
                    }
                    else {
                        BDBG_ERR(("unknown status: %d", status.lockStatus));
                    }
                }
                else if (rc == NEXUS_NOT_SUPPORTED) {
                    NEXUS_FrontendQamStatus qamStatus;
                    rc = NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
                    if (rc) {rc = BERR_TRACE(rc); return;}

                    BDBG_WRN(("frontend %s (fecLock=%d)", qamStatus.receiverLock?"locked":"unlocked", qamStatus.fecLock));
                    if (qamStatus.receiverLock) {
                        acquired = true;
                        break;
                    }
                    /* we can't conclude no lock until application timeout */
                }
                else {
                     BERR_TRACE(rc);
                }
            }
        }

        if (acquired) {
            break;
        }
    }

done:
    BKNI_DestroyEvent(statusEvent);
    BKNI_DestroyEvent(lockChangedEvent);
    return;
}
