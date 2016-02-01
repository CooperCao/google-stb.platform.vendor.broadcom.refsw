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

#include "nexus_platform.h"
#include "nexus_ir_blaster.h"
#include "nexus_ir_input.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "prompt.h"

static void irCallback(void *pParam, int iParam)
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

void bcmIrb(NEXUS_IrBlasterMode mode)
{
    NEXUS_Error rc;
    NEXUS_IrBlasterHandle irBHandle;
    NEXUS_IrBlasterSettings irBSettings;
    NEXUS_IrBlasterStatus irBStatus;
    NEXUS_IrInputHandle irIHandle;
    NEXUS_IrInputSettings irISettings;
    uint32_t data;
    uint8_t numBits = 12;
    bool headerPulse = true;
    uint8_t trailer = 0;
    int choice;

    NEXUS_IrInput_GetDefaultSettings(&irISettings);

    switch (mode) {
        case NEXUS_IrBlasterMode_eSony:
            irISettings.mode = NEXUS_IrInputMode_eCirSony;
            break;
        case NEXUS_IrBlasterMode_eXmp2:
            irISettings.mode = NEXUS_IrInputMode_eCirXmp;
            break;
        case NEXUS_IrBlasterMode_eGI:
            irISettings.mode = NEXUS_IrInputMode_eCirGI;
            break;
        case NEXUS_IrBlasterMode_eRC6:
            irISettings.mode = NEXUS_IrInputMode_eCirRC6;
            break;
        default:
            printf("Unsupported IR input mode (%d)\n", mode);
        break;
    }

    irISettings.dataReady.callback = irCallback;
    irISettings.dataReady.context = &irIHandle;

    irIHandle = NEXUS_IrInput_Open(0, &irISettings);
    if (!irIHandle) {
        printf("Failed to open irInput\n");
        exit(1);
    }

    NEXUS_IrBlaster_GetDefaultSettings(&irBSettings);
    irBSettings.mode = mode;

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

    while (1)
    {
        printf("\n\n");
        printf("===================================\n");
        printf("  IR Blaster mode %d\n", mode);
        printf("===================================\n");
        printf("    0) Exit\n");
        printf("    1) Toggle Power\n");
        printf("    2) Channel Up\n");
        printf("    3) Channel Down\n");
        printf("    4) Volume Up\n");
        printf("    5) Volume Down\n");
        printf("    6) Repeat Last IR Blaster Command\n");

        choice = Prompt();

        if (choice == 7) {
            NEXUS_IrBlaster_Resend(irBHandle);
        }
        else {
            switch(choice) {
                case 0:
                    NEXUS_IrBlaster_Close(irBHandle);
                    NEXUS_IrInput_Close(irIHandle);
                    return;
                case 1: /* Toggle Power */
                    switch (mode) {
                    case NEXUS_IrBlasterMode_eSony:
                        data = 0x095;
                        break;
                    case NEXUS_IrBlasterMode_eGI:
                        data = 0x600a;
                        break;
                    default:
                        printf("Unsupported IR blaster mode (%d)\n", mode);
                        break;
                    }
                    break;
                case 2: /* channel up */
                    switch (mode) {
                    case NEXUS_IrBlasterMode_eSony:
                        data = 0x090;
                        break;
                    case NEXUS_IrBlasterMode_eGI:
                        data = 0x500b;
                        break;
                    default:
                        printf("Unsupported IR blaster mode (%d)\n", mode);
                        break;
                    }
                    break;
                case 3: /* channel down */
                    switch (mode) {
                    case NEXUS_IrBlasterMode_eSony:
                        data = 0x091;
                        break;
                    case NEXUS_IrBlasterMode_eGI:
                        data = 0x400c;
                        break;
                    default:
                        printf("Unsupported IR blaster mode (%d)\n", mode);
                        break;
                    }
                    break;
                case 4: /* volume up */
                    switch (mode) {
                    case NEXUS_IrBlasterMode_eSony:
                        data = 0x092;
                        break;
                    case NEXUS_IrBlasterMode_eGI:
                        data = 0x300d;
                        break;
                    default:
                        printf("Unsupported IR blaster mode (%d)\n", mode);
                        break;
                    }
                    break;
                case 5: /* volume down */
                    switch (mode) {
                    case NEXUS_IrBlasterMode_eSony:
                        data = 0x093;
                        break;
                    case NEXUS_IrBlasterMode_eGI:
                        data = 0x200e;
                        break;
                    default:
                        printf("Unsupported IR blaster mode (%d)\n", mode);
                        break;
                    }
                    break;
                default:
                    break;
            }

            switch (mode) {
                case NEXUS_IrBlasterMode_eSony:
                    numBits = 12;
                    break;
                case NEXUS_IrBlasterMode_eRC6:
                    numBits = 16;
                    break;
                case NEXUS_IrBlasterMode_eGI:
                    numBits = 17;
                    break;
                default:
                    printf("Unsupported IR blaster mode (%d)\n", mode);
                    break;
            }

            if (mode == NEXUS_IrBlasterMode_eRC6)
                rc = NEXUS_IrBlaster_SendRC6(irBHandle, &data, numBits, 6 /* mode */, trailer ^= 1);
            else
                rc = NEXUS_IrBlaster_Send(irBHandle, &data, numBits, headerPulse);

            if (rc) {
                printf("Failed to send data!\n");
                return;
            } 
            while (1) {
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
        }
    }
}

void bcmIrbTest(void)
{
    while (1)
    {
        printf("\n\n");
        printf("========================\n");
        printf("  IR Blaster Test Menu  \n");
        printf("========================\n");
        printf("    0) Exit\n");
        printf("    1) Sony\n");
        printf("    2) GI\n");
        printf("    3) RC6\n");
        printf("    4) XMP2\n");

        switch(Prompt()) {
            case 0:
                return;

            case 1:
                bcmIrb(NEXUS_IrBlasterMode_eSony);
                break;

            case 2:
                bcmIrb(NEXUS_IrBlasterMode_eGI);
                break;

            case 3:
                bcmIrb(NEXUS_IrBlasterMode_eRC6);
                break;

            case 4:
                bcmIrb(NEXUS_IrBlasterMode_eXmp2);
                break;

            default:
                printf("not implemented\n");
                break;
        }
    }

    return;
}

