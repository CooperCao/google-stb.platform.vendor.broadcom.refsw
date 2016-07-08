/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *****************************************************************************/
/* Example to tune a OOB channel using nexus */

#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#include "nexus_platform.h"
#include "nexus_parser_band.h"
#if NEXUS_HAS_RECORD
#include "nexus_record.h"
#include "nexus_recpump.h"
#endif
#include "nexus_pid_channel.h"
#include "bdbg.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define ENABLE_BERT_TESTING 0

BDBG_MODULE(tune_oob);

static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;    
#if ENABLE_BERT_TESTING 
    NEXUS_FrontendOutOfBandStatus oobStatus;
#else
    NEXUS_FrontendFastStatus status;
#endif

    BSTD_UNUSED(param);

#if ENABLE_BERT_TESTING
    NEXUS_Frontend_GetOutOfBandStatus(frontend, &oobStatus);
    BDBG_WRN(("OOB Lock callback, frontend 0x%08x - lock status: QamLock = %d, Fec lock = %d, Bert Lock = %d\n", (unsigned)frontend, oobStatus.isQamLocked, oobStatus.isFecLocked, oobStatus.isBertLocked));
#else
    NEXUS_Frontend_GetFastStatus(frontend, &status);
    if(status.lockStatus == NEXUS_FrontendLockStatus_eUnlocked)
        BDBG_WRN(("OOB Lock callback: Fast lock status = Unlocked.\n"));
    else if(status.lockStatus == NEXUS_FrontendLockStatus_eLocked)
        BDBG_WRN(("OOB Lock callback: Fast lock status = Locked.\n"));
    else if(status.lockStatus == NEXUS_FrontendLockStatus_eNoSignal)
        BDBG_WRN(("OOB Lock callback: Fast lock status = NoSignal.\n"));
    else
        BDBG_WRN(("OOB Lock callback: Fast lock status = Unknown.\n"));
    BDBG_WRN(("OOB Lock callback: acquireInProgress = %d.\n", status.acquireInProgress));
#endif
}

int main(int argc, char **argv)
{
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendHandle frontend = NULL;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_FrontendAcquireSettings settings;
    unsigned symbolrate=0, loops = 2;
    NEXUS_FrontendOutOfBandSettings oobSettings;
    NEXUS_Error rc;
    NEXUS_FrontendOutOfBandStatus oobStatus;
    NEXUS_FrontendOutOfBandMode mode = NEXUS_FrontendOutOfBandMode_eDvs167Qpsk;
    NEXUS_PidChannelHandle allpassPidChannel;
    NEXUS_PidChannelSettings pidChannelSettings;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
#if NEXUS_HAS_RECORD
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecordHandle record;
    NEXUS_RecordSettings recordCfg;
    NEXUS_RecordPidChannelSettings pidCfg;
    NEXUS_FileRecordHandle file;
    const char *fname = "/mnt/hd/videos/oobstream.ts";
#endif

    /* default freq & qam mode */
    unsigned freq = 357;

    /* allow cmdline freq & qam mode for simple test */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        symbolrate = atoi(argv[2]);
    }

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
    settings.capabilities.outOfBand = true;
    frontend = NEXUS_Frontend_Acquire(&settings);
    if (!frontend) {
        BDBG_ERR(("Unable to find OOB-capable frontend\n"));
        return -1;
    }

    
    NEXUS_Frontend_GetDefaultOutOfBandSettings(&oobSettings);
    switch (mode) {
    case NEXUS_FrontendOutOfBandMode_eDvs167Qpsk:
    case NEXUS_FrontendOutOfBandMode_ePod_Dvs167Qpsk:
#if ENABLE_BERT_TESTING
        freq = 75000000; 
        symbolrate = 1544000;

        BDBG_WRN(("Make sure to set outOfBand.nyquist in NEXUS_FrontendDevice3128OpenSettings structure to NEXUS_NyquistFilter_eRootRaisedCosine30.\n"));
        oobSettings.bert.enabled = false;
        oobSettings.bert.polynomial = NEXUS_FrontendOutOfBandBertPolynomial_e15;
        oobSettings.spectrum = NEXUS_FrontendOutOfBandSpectrum_eInverted;
        oobSettings.autoAcquire = false;
        oobSettings.bertSource = NEXUS_FrontendOutOfBandBertSource_eQChOutput;
#else
        freq = 102500000;
        symbolrate = 772000;
#endif
        break;
    case NEXUS_FrontendOutOfBandMode_eDvs178Qpsk:
    case NEXUS_FrontendOutOfBandMode_ePod_Dvs178Qpsk:
        freq = 75250000; /* symbolrate = 1024000 */
        symbolrate = 1024000;
    break;
    default:
        BDBG_ERR(("Unsupported modulation type.\n"));
        return 0;
    }

    oobSettings.mode = mode;
    oobSettings.symbolRate = symbolrate;
    oobSettings.frequency = freq;
    oobSettings.lockCallback.callback = lock_callback;
    oobSettings.lockCallback.context = frontend;

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
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    parserBandSettings.allPass = true;
    parserBandSettings.acceptNullPackets = true;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    rc = NEXUS_Frontend_TuneOutOfBand(frontend, &oobSettings);
    BDBG_ASSERT(!rc); 
    
    while (loops--) {
        BDBG_WRN(("Press enter to get status. \n"));
        getchar();
        rc = NEXUS_Frontend_GetOutOfBandStatus(frontend, &oobStatus);
        BDBG_ASSERT(!rc);       
        BDBG_WRN(("OOB Status, frontend %p - lock status: QamLock = %d, Fec lock = %d, Bert Lock = %d\n", (void*)frontend, oobStatus.isQamLocked, oobStatus.isFecLocked, oobStatus.isBertLocked));
    }

    NEXUS_PidChannel_GetDefaultSettings(&pidChannelSettings);
    pidChannelSettings.pidChannelIndex = NEXUS_ParserBand_e0;
    allpassPidChannel = NEXUS_PidChannel_Open(parserBand, 0, &pidChannelSettings);

#if NEXUS_HAS_RECORD
    recpump = NEXUS_Recpump_Open(0, NULL);
    record = NEXUS_Record_Create();
    NEXUS_Record_GetSettings(record, &recordCfg);
    recordCfg.recpump = recpump;
    NEXUS_Record_SetSettings(record, &recordCfg);
    
    file = NEXUS_FileRecord_OpenPosix(fname,  NULL);
    
    NEXUS_Record_GetDefaultPidChannelSettings(&pidCfg);
    pidCfg.recpumpSettings.pidType = NEXUS_PidType_eUnknown;

    /* Do not generate an index */
    NEXUS_Record_AddPidChannel(record,allpassPidChannel,NULL);

    NEXUS_Record_Start(record, file);
    BDBG_WRN(("press ENTER to stop record\n"));
    getchar();
    
    NEXUS_Record_Stop(record);
    NEXUS_FileRecord_Close(file);
#endif

    BDBG_WRN(("Press enter to stop. \n"));
    getchar();

    /* Shutdown */
    NEXUS_Record_Destroy(record);
    NEXUS_Recpump_Close(recpump);
    NEXUS_PidChannel_Close(allpassPidChannel);
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

