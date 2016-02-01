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
#include "bstd.h"
#include "prompt.h"
#include "nexus_platform.h"
#include "priv/nexus_core.h"
#include "sys_handles.h"

#ifdef DIAGS_OFDM
    #include "ofdmtest.h"
#endif

#ifdef DIAGS_QAM
    #include "qamtest.h"
#endif

#ifdef DIAGS_SAT
    #include "sattest.h"
#endif

/***********************************************************************
 *                       Global Variables
 ***********************************************************************/

/***********************************************************************
 *                      External References
 ***********************************************************************/

/***********************************************************************
 *                      Function Prototypes
 ***********************************************************************/
#if __cplusplus
extern "C" {
#endif

void bcmFrontendTest (void);

#if __cplusplus
}
#endif

/***********************************************************************
 *                      Global variables
 ***********************************************************************/

 /***********************************************************************
 *
 *  bcmFrontendTest()
 * 
 *  Back end test function
 *
 ***********************************************************************/
void bcmFrontendTest (void)
{
    uint32_t command_id;

    enum  menu 
    {
        EXIT_FRONTEND_TEST
        #ifdef DIAGS_SAT
            ,FRONTEND_SAT_TEST
        #endif
        #ifdef DIAGS_QAM
            ,FRONTEND_QAM_TEST
        #endif
        #ifdef DIAGS_QAM_CANOPENER
            ,FRONTEND_QAM_CANOPENER_TEST
        #endif
        #ifdef DIAGS_OFDM
            ,FRONTEND_OFDM_TEST
        #endif
        #ifdef DIAGS_MOCA_TEST
            ,FRONTEND_MOCA_TEST
        #endif
    };

    while (1)
    {
        printf("\n\n");
        printf("================\n");
        printf("  FRONTEND MENU  \n");
        printf("================\n");
        printf("    0) Exit\n");
        #ifdef DIAGS_SAT
            printf("    1) Satellite\n");
        #endif
        #ifdef DIAGS_QAM
            printf("    2) QAM\n");
        #endif
        #ifdef DIAGS_QAM_CANOPENER
            printf("    3) QAM - Can-opener\n");
        #endif
        #ifdef DIAGS_OFDM
            printf("    4) OFDM\n");
        #endif
        #ifdef DIAGS_MOCA_TEST
            printf("    5) MoCA\n");
        #endif

        command_id = PromptChar();

        switch(command_id)
        {
            case '0':
                return;

            #ifdef DIAGS_SAT
                case '1':
                    SatTest();
                    break;
            #endif

            #ifdef DIAGS_QAM
                case '2':
                    QamTest();
                    break;
            #endif

            #ifdef DIAGS_QAM_CANOPENER
                case '3':
                    QamTestCanopener();
                    break;
            #endif

            #ifdef DIAGS_OFDM
                case '4':
                    OfdmTest();
                    break;
            #endif

            #ifdef DIAGS_MOCA_TEST
                case '5':
                    bcmMocaTest();
                    break;
            #endif

            default:
                printf("Invalid selection\n");
                break;
        }
    }
}
