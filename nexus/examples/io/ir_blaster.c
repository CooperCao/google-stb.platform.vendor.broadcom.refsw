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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/
/* Example to send IR output and receive IR input using nexus */
/* This sends output using the IR blaster port and receives that data via the IR receiver port. */
#if NEXUS_HAS_IR_BLASTER
#include "nexus_platform.h"
#include "nexus_ir_blaster.h"
#include "nexus_ir_input.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define GI          0
#define RC6         1
#define SONY        2

#define IRB_TEST_OPTION GI

void irCallback(void *pParam, int iParam)
{
    NEXUS_IrInputHandle irHandle = *(NEXUS_IrInputHandle *)pParam;
    size_t numEvents = 1;
    NEXUS_Error rc = 0;
    bool overflow;

    BSTD_UNUSED(iParam);

    while (numEvents && !rc) 
    {
        NEXUS_IrInputEvent irEvent;
        rc = NEXUS_IrInput_GetEvents(irHandle, &irEvent, 1, &numEvents, &overflow);
        if (numEvents)
            printf("irCallback: rc: %d, code: %08x, repeat: %s\n", rc, irEvent.code, irEvent.repeat ? "true" : "false");
    }
}

int main(void)
{
    NEXUS_Error rc;
    NEXUS_IrInputHandle irIHandle;
    NEXUS_IrInputSettings irISettings;
    NEXUS_IrBlasterHandle irBHandle;
    NEXUS_IrBlasterSettings irBSettings;
    NEXUS_IrBlasterStatus irBStatus;

#if IRB_TEST_OPTION==RC6
    uint32_t data = 0x270c; /* Toggle Power command for a RC6 remote */
    uint8_t numBits = 16;
    uint8_t trailer = 0;
#elif IRB_TEST_OPTION==SONY
    uint32_t data = 0x095;  /* Toggle Power command for a Sony remote */
    uint8_t numBits = 12;
#else
    uint32_t data = 0x600a; /* Toggle Power command for a GI remote */
    uint8_t numBits = 17;
#endif

    bool headerPulse = true;

    NEXUS_Platform_Init(NULL);

    NEXUS_IrInput_GetDefaultSettings(&irISettings);
#if IRB_TEST_OPTION==RC6
    irISettings.mode = NEXUS_IrInputMode_eCirRC6;
#elif IRB_TEST_OPTION==SONY
    irISettings.mode = NEXUS_IrInputMode_eCirSony;
#else
    irISettings.mode = NEXUS_IrInputMode_eCirGI;
#endif
    irISettings.dataReady.callback = irCallback;
    irISettings.dataReady.context = &irIHandle;

    irIHandle = NEXUS_IrInput_Open(0, &irISettings);
    if (!irIHandle) {
        printf("Failed to open irInput\n");
        exit(1);
    }

    NEXUS_IrBlaster_GetDefaultSettings(&irBSettings);
#if IRB_TEST_OPTION==RC6
    irBSettings.mode = NEXUS_IrBlasterMode_eRC6;
#elif IRB_TEST_OPTION==SONY
    irBSettings.mode = NEXUS_IrBlasterMode_eSony;
#else
    irBSettings.mode = NEXUS_IrBlasterMode_eGI;
#endif

    irBHandle = NEXUS_IrBlaster_Open(0, &irBSettings);
    if (!irBHandle) {
        printf("Failed to open irBlaster\n");
        exit(1);
    }

    rc = NEXUS_IrBlaster_GetStatus(irBHandle, &irBStatus);
    if (rc)
        printf("Get Status failed!\n");
    else
        printf("Initial Blaster is status is %sbusy!\n", irBStatus.busy?"":"not ");

    while (1) /* run forever */
    {
        printf("Sending next command...\n");
#if IRB_TEST_OPTION==RC6
        rc = NEXUS_IrBlaster_SendRC6(irBHandle, &data, numBits, 6 /* mode */, trailer ^= 1);
        BSTD_UNUSED(headerPulse);
#else
        rc = NEXUS_IrBlaster_Send(irBHandle, &data, numBits, headerPulse);
#endif
        if (rc) {
            printf("Failed to send data!\n");
            goto done;
        } 

        while (1)
        {
            rc = NEXUS_IrBlaster_GetStatus(irBHandle, &irBStatus);
            if (rc) {
                printf("Get Status failed!\n");
                break;
            }
    
            if (!irBStatus.busy) {
                printf("Data sent...\n");
                break; /* stop when complete */
            }
        }

        sleep(2); /* pause a bit before sending next command */
    }

done:
    NEXUS_IrInput_Close(irIHandle);
    NEXUS_IrBlaster_Close(irBHandle);
    NEXUS_Platform_Uninit();

    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif
