/******************************************************************************
 *    (c)2008 Broadcom Corporation
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
#include "nexus_platform.h"
#include "nexus_pwm.h"

#include "bstd.h"
#include "bkni.h"
#include "prompt.h"
#include <stdlib.h>
#include <stdio.h>

void bcmPwmTest (void)
{
    NEXUS_PwmChannelHandle pwm;
    NEXUS_PwmChannelSettings pwmSettings;
    char str[256];
    char choice;
    int lval,lval2;
    NEXUS_PwmFreqModeType frequencyMode;
    char comma;

    while (1)
    {
        printf("\n\n");
        printf("===================================\n");
        printf("  PWM test\n");
        printf("===================================\n");
        printf("    0) Exit\n");
        printf("    1) Select PWM channel\n");
        printf("    2) Write PWM control word\n");
        printf("    3) Write PWM Freq Mode\n");
        printf("    4) Write PWM On Interval\n");
        printf("    5) Write PWM Period Interval\n");
        printf("    6) Write PWM On and Period Interval\n");
        printf("    7) Read PWM  Freq mode \n");
        printf("    8) Read PWM On and Period Interval\n");
        printf("    9) Start PWM generation\n");
        printf("    a) Stop PWM generation\n");

        choice = PromptChar();

        switch (choice)
        {
            case '0':
                return;

            case '1':
                printf("Enter the PWM channel (1 or 2): \n");
                fgets(str, 256, stdin);
                sscanf (str, "%d", &lval);
                if (lval > 2 || lval == 0)
                {
                    printf("Invalid PWM channel, enter only 1 or 2\n");
                    break;
                }
                NEXUS_Pwm_GetDefaultChannelSettings(&pwmSettings);
                pwm = NEXUS_Pwm_OpenChannel(lval, &pwmSettings);
                break;

            case '2':
                printf("Enter control word value (in hex):\n");
                fgets(str, 256, stdin);
                sscanf(str, "%x", (unsigned int *)&lval);
                NEXUS_Pwm_SetControlWord(pwm, lval);
                break;

            case '3':
                printf("Enter the desired Freq Mode : 0=Variable, 1=Constant\n");
                fgets(str, 256, stdin);
                sscanf (str, "%d", (int *)&frequencyMode);
                if (frequencyMode > 2 )
                {
                    printf("Invalid Fre mode, enter only 0 or 1\n");
                    break;
                }
                NEXUS_Pwm_SetFreqMode(pwm, frequencyMode);
                break;

            case '4':
                printf("Enter On Interval value (in hex):\n");
                fgets(str, 256, stdin);
                sscanf(str, "%x", (unsigned int *)&lval);
                NEXUS_Pwm_SetOnInterval(pwm, lval);
                break;

            case '5':
                printf("Enter Period Interval value (in hex):\n");
                fgets(str, 256, stdin);
                sscanf(str, "%x", (unsigned int *)&lval);
                NEXUS_Pwm_SetPeriodInterval(pwm, lval);
                break;

            case '6':
                printf("Enter On and period intervals seperated by comma (in hex), like this: 40,80\n");
                fgets(str, 256, stdin);
                sscanf(str, "%x %c %x", (unsigned int *)&lval,&comma, (unsigned int *)&lval2);
                NEXUS_Pwm_SetOnAndPeriodInterval(pwm, lval, lval2);
                break;

            case '7':
                NEXUS_Pwm_GetFreqMode(pwm, &frequencyMode);
                if (frequencyMode == 0)
                    printf("The mode is Variable Freq Mode\n");
                else
                    printf("The mode is Contant Freq Mode\n");
                break;

            case '8':
                NEXUS_Pwm_GetOnAndPeriodInterval(pwm, (uint16_t *)&lval, (uint16_t *)&lval2);
                printf("The On Interval is : %x and Period interval is: %x\n", lval, lval2);
                break;

            case '9':
                NEXUS_Pwm_Start(pwm);
                break;

            case 'a':
                NEXUS_Pwm_Stop(pwm);
                break;

            default:
                printf("\nInvalid Choice!\n\n");
                break;
        }
    }

    return;
}
