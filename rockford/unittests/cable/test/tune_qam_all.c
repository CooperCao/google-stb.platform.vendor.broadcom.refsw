/******************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/* Example to tune a VSB channel using nexus */

#include "nexus_frontend.h"
#include "nexus_platform.h"
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
#if NEXUS_DTV_PLATFORM
#include "nexus_platform_boardcfg.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

BDBG_MODULE(tune_qam);

static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;

    BSTD_UNUSED(param);

    /* BDBG_WRN(("Lock callback, frontend 0x%08x", (unsigned)frontend)); */

}

int main(int argc, char **argv) /*  */
{
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendHandle frontend=NULL;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_FrontendQamStatus qamStatus;
	NEXUS_Error rc;
	bool locked;
    char ch;
    unsigned i;
    /* default freq & qam mode */
    unsigned freq = 759;
    NEXUS_FrontendQamMode qamMode = NEXUS_FrontendQamMode_e256;
    FILE * fp = NULL;
#define FREQ_FILE "./freq.txt"
    char line[128];
    unsigned mode = 256;
    int count = 0;

    printf("\n ");
    printf("%s :  %s \n", __FILE__, __TIMESTAMP__);
    printf("\n");

    /* allow cmdline freq & qam mode for simple test */
    if (argc > 1) {
        freq = atoi(argv[1]);
        if (argc > 2) {
            mode = atoi(argv[2]);
        }
    } else {
        fp = fopen(FREQ_FILE, "rt");
        if (!fp) {
            printf("Cannot open %s, abort\n", FREQ_FILE);
            return 1;
        }
        fgets(line, 128, fp);
        sscanf(line, "%u %u", &freq, &mode);
        printf("Freq %uMHz, QAM%u\n", freq,mode);
    }

    switch (mode) {
    case 64: qamMode = NEXUS_FrontendQamMode_e64; break;
    case 256: qamMode = NEXUS_FrontendQamMode_e256; break;
    case 1024: qamMode = NEXUS_FrontendQamMode_e1024; break;
    default: BDBG_ERR(("unknown qam mode %d\n", mode)); return -1;
    }
    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    for ( i = 0; i < NEXUS_MAX_FRONTENDS; i++ )
    {
        NEXUS_FrontendCapabilities capabilities;
        frontend = platformConfig.frontend[i];
        if (frontend) {
            NEXUS_Frontend_GetCapabilities(frontend, &capabilities);
            /* Does this frontend support qam? */
            if ( capabilities.qam )
            {
                BDBG_WRN(("Frontend %d (0x%08x) is QAM capable!", i, (unsigned)frontend ));
            }
        }
    }

    if (NULL == frontend )
    {
        BDBG_ERR(("Unable to find QAM-capable frontend\n"));
        return -1;
    }

    while (count < 100) {
        NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
        qamSettings.frequency = freq * 1000000;
        qamSettings.mode = qamMode;
        switch (qamMode) {
        default:
        case NEXUS_FrontendQamMode_e64: qamSettings.symbolRate = 5056900; break;
        case NEXUS_FrontendQamMode_e256: qamSettings.symbolRate = 5360537; break;
        case NEXUS_FrontendQamMode_e1024: qamSettings.symbolRate = 0; /* TODO */
		break;
        }
        printf("Tuning freq %u QAM mode %u\n", qamSettings.frequency, qamMode);
        qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
		qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
		qamSettings.lockCallback.callback = lock_callback;

        NEXUS_Frontend_GetUserParameters(frontend, &userParams);


        for ( i = 0; i < NEXUS_MAX_FRONTENDS; i++  )
        {
            NEXUS_FrontendCapabilities capabilities;
            frontend = platformConfig.frontend[i];
            if (frontend) {
                NEXUS_Frontend_GetCapabilities(frontend, &capabilities);
                /* only tune if the frontend supports QAM */
                if ( capabilities.qam ) {
                    /* printf("\n\n");
                     BDBG_WRN(("tuning frontend %d (0x%08x)", i, (unsigned)frontend ));;
                    */
                    qamSettings.lockCallback.context = frontend;
                    NEXUS_Frontend_TuneQam(frontend, &qamSettings);
                }
            }
		}

        sleep(1);

#if 1
    status:
        for ( i = 0; i < NEXUS_MAX_FRONTENDS; i++  )
        {
            NEXUS_FrontendCapabilities capabilities;
            frontend = platformConfig.frontend[i];
            if (frontend) {
                NEXUS_Frontend_GetCapabilities(frontend, &capabilities);
				NEXUS_Frontend_GetUserParameters(frontend, &userParams);
				BDBG_WRN(("Frontend type is %x", userParams.chipId));
                /* Does this frontend support qam? */
                if ( capabilities.qam )
                {
					rc = NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
					if(rc) {
						BDBG_ERR(("Error in tuning the frontend"));
					}
					else{
						BDBG_WRN(("QAM Status, frontend %d (0x%08x):", i, (unsigned)frontend ));
						BDBG_WRN(("      receiverLock [%d]= %d"  ,i, qamStatus.receiverLock    ));  /*  bool     receiverLock;      Do we have QAM lock? */
						BDBG_WRN(("           fecLock [%d]= %d"  ,i, qamStatus.fecLock         ));  /*  bool     fecLock;           Is the FEC locked? */
						BDBG_WRN(("       snrEstimate [%d]= %u"  ,i, qamStatus.snrEstimate     ));  /*  unsigned snrEstimate;       snr estimate in 1/100 dB (in-Band). */
					}
                }
            }
        }

        printf("\n");
        sleep(1);

#endif
        printf("sleep for 1 second, ctrl-c to quit\n");
        sleep(1);
#if 0
            switch (ch)
            {
                case 'q':
					goto quit;
                case 'u':
                    NEXUS_Frontend_Untune(frontend);
                    printf("Untuned.  Press a key to continue\n");
                    break;
                case 'd':
                {
                    unsigned symbolRate = qamSettings.symbolRate;
                    qamSettings.symbolRate = 1;
                    NEXUS_Frontend_TuneQam(frontend, &qamSettings);
                    qamSettings.symbolRate = symbolRate;
                    break;
                }
                case 'a':
                {
                    unsigned symbolRate = qamSettings.symbolRate;
                    qamSettings.symbolRate = 2;
                    NEXUS_Frontend_TuneQam(frontend, &qamSettings);
                    qamSettings.symbolRate = symbolRate;
                    break;
                }
                case 's':
                    goto status;
                case '\n':
                    break;
                default:
                    goto again;
            }
#endif
        printf("Tune again.\n");
        if (fgets(line, 128, fp) == NULL) {
            printf("%s wrapped\n", FREQ_FILE);
            fseek(fp, 0, SEEK_SET);
            fgets(line, 128, fp);
        }

        sscanf(line, "%u %u", &freq, &mode);
        printf("Freq %uMHz, QAM%u\n", freq,mode);
        switch (mode) {
        case 64: qamMode = NEXUS_FrontendQamMode_e64; break;
        case 256: qamMode = NEXUS_FrontendQamMode_e256; break;
        case 1024: qamMode = NEXUS_FrontendQamMode_e1024; break;
        default: BDBG_ERR(("unknown qam mode %d\n", mode)); return -1;
		}

        count++;
        printf("======= Count %d\n", count);
    }
quit:
    NEXUS_Platform_Uninit();

    return 0;
}
