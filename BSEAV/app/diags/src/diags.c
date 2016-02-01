/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
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
        printf( "Copyright (C) Broadcom Corporation 2003. All rights reserved.\n");

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

    #if NEXUS_HAS_FILE
        int i;
    #endif

    run_dma_memory_test = false;

    /* Initialize SETTOP API */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);

    #ifdef DIAGS_CRC_TEST
        platformSettings.displayModuleSettings.crcQueueSize = 600; /* This enables the CRC capture */
    #endif

    #ifdef DIAGS_SUPPORT_MVC
        platformSettings.transportModuleSettings.maxDataRate.parserBand[NEXUS_ParserBand_e0] = 50000000; 
    #endif

    platformSettings.openFrontend = true;

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
}
