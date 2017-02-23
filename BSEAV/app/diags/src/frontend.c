/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bstd.h"
#include "prompt.h"
#include "nexus_platform.h"
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
