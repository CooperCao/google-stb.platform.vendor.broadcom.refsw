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
#include "test.h"

#ifdef DIAGS_PWM_TEST

#include <stdio.h>
#include "bpwm.h"
#include "upg_handles.h"
#include "prompt.h"

/***********************************************************************
 *                      Function Prototypes
 ***********************************************************************/

#if __cplusplus
extern "C" {
#endif	   

#if __cplusplus
}
#endif

/***********************************************************************
 *
 *  bcmPwmTestBcmDiags()
 * 
 *  PWM test
 *
 ***********************************************************************/
void bcmPwmTest (void)
{
    int     	     	choice;
    uint16_t	      	lval,lval2;
    char 				str[20];
    BPWM_ChannelHandle	hPwmChan = NULL;
    BPWM_FreqModeType	Frequeny_Mode;	
    char 				comma;

    choice = 1;

    while(choice != 0xb)
    {
        printf("\n\n PWM Functions\n");
        printf("1) Select PWM channel\n");
        printf("2) Write PWM control word\n");
        printf("3) Write PWM Freq Mode\n");
        printf("4) Write PWM On Interval\n");
        printf("5) Write PWM Period Interval\n");
        printf("6) Write PWM On and Period Interval\n");
        printf("7) Read PWM  Freq mode \n");
        printf("8) Read PWM On and Period Interval\n");
        printf("9) Start PWM generation\n");
        printf("a) Stop PWM generation\n");
        printf("b) Exit\n\n");;

        choice = Prompt();
        switch (choice)
        {
            case 0:
                return;

            case 1:
                printf("Enter the PWM channel (1 or 2): \n");
                rdstr(str);
                sscanf (str, "%d", &lval);
                if (lval > 2 || lval == 0)
                {
                    printf("Invalid PWM channel, enter only 1 or 2\n");
                    break;
                }
                hPwmChan = bcmGetPwmChannelHandle (lval - 1);
                break;

            case 2:
                printf("Enter control word value (in hex):\n");
                         rdstr(str);
                      sscanf(str, "%x", &lval);
                BPWM_SetControlWord (hPwmChan, (uint16_t)lval);
                break;

            case 3:
                printf("Enter the desired Freq Mode : 0=Variable, 1=Constant\n");
                rdstr(str);
                sscanf (str, "%d", &Frequeny_Mode);
                if (Frequeny_Mode > 2 )
                {
                    printf("Invalid Fre mode, enter only 0 or 1\n");
                    break;
                }
                 BPWM_SetFreqMode (hPwmChan, Frequeny_Mode);
                break;

            case 4:
                printf("Enter On Interval value (in hex):\n");
                rdstr(str);
                sscanf(str, "%x", &lval);
                BPWM_SetOnInterval (hPwmChan, lval);
                break;

            case 5:
                printf("Enter Period Interval value (in hex):\n");
                rdstr(str);
                sscanf(str, "%x", &lval);
                BPWM_SetPeriodInterval (hPwmChan, lval);
                break;
                
            case 6:
                printf("Enter On and period intervals seperated by comma (in hex), like this: 40,80\n");
                rdstr(str);
                sscanf(str, "%x %c %x", &lval,&comma, &lval2); 
                BPWM_SetOnAndPeriodInterval (hPwmChan, lval, lval2);
                break;

            case 7:
                BPWM_GetFreqMode (hPwmChan, &Frequeny_Mode);
                if (Frequeny_Mode == 0)
                    printf("The mode is Variable Freq Mode\n");
                else
                    printf("The mode is Contant Freq Mode\n");	
                break;
                    
            case 8:
                BPWM_GetOnAndPeriodInterval (hPwmChan, &lval, &lval2);
                printf("The On Interval is : %x and Period interval is: %x\n", lval, lval2);
                break;

            case 9:
                BPWM_Start (hPwmChan);
                break;

            case 0xa:
                BPWM_Stop (hPwmChan);
                break;

            default:
                printf("\nInvalid Choice!\n\n");
                break;
        }
    }
}

#else

void bcmPwmTest (void)
{
    printf("Not enabled\n");
}

#endif