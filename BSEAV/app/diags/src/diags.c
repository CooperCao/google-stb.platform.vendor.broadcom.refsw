/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "version.h"
#include "sys_handles.h"
#include "upg_handles.h"
#include "frontend.h"
#include "nexus_platform.h"
#include "prompt.h"
#include "colorbar.h"

/***********************************************************************
 *                       Global Variables
 ***********************************************************************/

/***********************************************************************
 *                      External References
 ***********************************************************************/
#if __cplusplus
extern "C" {
#endif

extern void bcmUpgTests(void);
extern void MemoryTestMenu(void);
extern void bcmInitBackend(void);
extern void bcmBackendTest(void);
extern void bcmMiscTest(void);
extern int bcmAudioTest(void);

int run_dma_memory_test;

#if __cplusplus
}
#endif

/***********************************************************************
 *                        Local Functions
 ***********************************************************************/

/***********************************************************************
 *
 *  BcmDiags()
 * 
 *  Main diagnostics menu
 *
 ***********************************************************************/
void BcmDiags(void)
{
    static char command_id;

    while (1)
    {
        printf( "\n\nBCM%d Set Top Evaluation Code - Version %s, Compiled on %s, %s\n",
                        BCM_BOARD, EVAL_VERSION, __DATE__, __TIME__);
        printf("    0) Exit\n");
        printf("    1) Memory Functions\n");
        printf("    2) Front End Functions\n");
        printf("    3) Back End Functions\n");
        printf("    4) Peripheral Functions\n");
        printf("    5) Audio Functions\n");
        printf("    6) SATA Functions\n");
        printf("    7) USB Functions\n");
        printf("    8) Front End Daughtercard Functions\n");
        printf("    9) RFM Functions\n");
        printf("    a) HDMI Functions\n"); 
        printf("    b) Ethernet Functions\n"); 
        printf("    c) Softmodem Tests\n"); 
        printf("    d) Misc Tests\n"); 
        printf("    e) IP Client Functions\n"); 
        printf("    f) Transcode\n"); 
        printf("    g) Display Colorbar\n"); 

        command_id = PromptChar();

        switch(command_id)
        {
            case '0':
                printf("Exiting diags ...\n");
                return;
                break;

            case '1':
                printf("Not enabled\n");
                break;

            case '2':
                #ifdef DIAGS_FRONTEND_TEST
                    bcmFrontendTest();
                #else
                    printf("Not enabled\n");
                #endif
                break;

            case '3':
                #ifdef DIAGS_BACKEND_TEST
                    bcmBackendTest();
                #else
                    printf("Not enabled\n");
                #endif
                break;

            case '4':
                bcmUpgTests();
                break;
                    
            case '5':
                bcmAudioTest();
                break;
                    
            case '6':
                #ifdef DIAGS_SATA_TEST
                    bcmSataTest();
                #else
                    printf("Not enabled\n");
                #endif
                break;
                    
            case '7':
                #if defined(DIAGS_CFE_SUPPORT) && defined(DIAGS_USB_TEST)
                    bcmUsbTest();
                #else
                    printf("Not enabled\n");
                #endif
                break;
                    
            case '8':
                #ifdef DIAGS_DAUGHTERCARD_TEST
                bcmDaughterCardTest();
                #else
                    printf("Not enabled\n");
                #endif
                break;

            case '9':
                #ifdef DIAGS_RFM_TEST
                    bcmRfmTest();
                #else
                    printf("Not enabled\n");
                #endif
                break;

            case 'a': 
                #ifdef DIAGS_HDMI_TEST
                    bcmHdmiTest() ;
                #else
                    printf("Not enabled\n");
                #endif
                break ;
                    
            case 'b': 
                #ifdef DIAGS_ETHERNET_TEST
                    bcmEthernetTest();
                #else
                    printf("Not enabled\n");
                #endif
                break ;

            case 'c':
                #ifdef DIAGS_SOFTMODEM_TEST
                    bcmSoftModemTest();
                #else
                    printf("Softmodem test not enabled\n");
                #endif
                break;

            case 'd':
                #ifdef DIAGS_MISC_TEST
                    bcmMiscTest();
                #else
                    printf("Not enabled\n");
                #endif
                break;

            case 'e':
                #ifdef DIAGS_IP_CLIENT_TEST
                    /* nexus ip_client -p 5000 -t 3 -d 192.168.254.10 -u /AbcMPEG2HD.mpg -l 1 */
                    IpClientCmd("-p 5000 -t 3 -d 192.168.254.10 -u /AbcMPEG2HD.mpg -l 1");
                #else
                    printf("Not enabled\n");
                #endif
                break;
                    
            case 'f':
                #ifdef DIAGS_TRANSCODE_TEST
                    bcmTranscodeTest();
                #else
                    printf("Not enabled\n");
                #endif
                break;

            case 'g':
                bcmColorbarTest();
                break;

            default:
                printf("Please enter a valid choice\n");
                break;
        }
    }
}

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    int curarg = 0;
    bool useFrontend=true;

    #if NEXUS_HAS_FILE
        int i;
    #endif

    run_dma_memory_test = false;

    while ( curarg < argc ) {
        if (!strcmp(argv[curarg], "-notuner")) {
            useFrontend = false;
        }

        curarg++;
    }
    /* Initialize SETTOP API */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);

    #ifdef DIAGS_CRC_TEST
        platformSettings.displayModuleSettings.crcQueueSize = 600; /* This enables the CRC capture */
    #endif

    #ifdef DIAGS_SUPPORT_MVC
        platformSettings.transportModuleSettings.maxDataRate.parserBand[NEXUS_ParserBand_e0] = 50000000;
    #endif

    platformSettings.openFrontend = useFrontend;

    #if NEXUS_HAS_FILE
        for (i=0; i<NEXUS_FILE_MAX_IOWORKERS; i++)
        {
            platformSettings.fileModuleSettings.schedulerSettings[i].priority = i+210;
        }
        platformSettings.fileModuleSettings.workerThreads=2;
    #endif

    #ifdef DIAGS_TRANSCODE_TEST
        /* audio PI supports 4 by default; we need one extra mixers for each transcoders; */
        #ifdef NEXUS_HAS_VIDEO_ENCODER
            platformSettings.audioModuleSettings.maxAudioDspTasks += NEXUS_NUM_VIDEO_ENCODERS;
            platformSettings.audioModuleSettings.numCompressedBuffers += NEXUS_NUM_VIDEO_ENCODERS;
        #endif
    #endif

    NEXUS_Platform_Init(&platformSettings);

    #ifdef DIAGS_SATA_SUPPORT
        #ifdef DIAGS_SATA_AHCI
            ahci_init_one(0, SPEED_GEN2);
            ahci_init_one(1, SPEED_GEN2);
        #else
            ATAInit();
        #endif
    #endif

    /* Initialize diagnostics backend handles */
    /*bcmInitBackend();*/

    /* Diagnostics Menu */
    BcmDiags();
    NEXUS_Platform_Uninit();
}
