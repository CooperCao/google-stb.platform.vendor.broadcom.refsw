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

#ifdef DIAGS_ICAP_TEST

#include <stdio.h>
#include "bstd.h"
#include "bkni_multi.h"
#include "bicp.h"
#include "upg_handles.h"
#include "prompt.h"
#include "bchp_hif_cpu_intr1.h"
#include "priv/nexus_core.h"

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
 *  bcmCallBack
 * 
 *  ICAP callback function
 *
 ***********************************************************************/
void bcmCallBack(uint16_t ctrl, uint8_t mode, uint16_t data)
{
    printf("RC6:  ctrlbits=0x%x, modebits=0x%x, data=0x%03x (%d)\n", ctrl, mode, data, data);
}

/***********************************************************************
 *
 *  bcmIcapTest
 * 
 *  ICAP test
 *
 ***********************************************************************/
void bcmIcapTest (void)
{
    int			choice, chanNo = 3;
    unsigned int		lval;
    char				str[20];
    BICP_ChannelHandle	hIcpChan;
    uint16_t			count;
    BKNI_EventHandle	hEvent;
    double freq=0.0;
    char key;
    extern unsigned long Timer0IntCount;
    extern void TimerISR(void);

    hIcpChan = bcmGetIcpChannelHandle (chanNo);
    BICP_GetEventHandle (hIcpChan, &hEvent);

    while(1)
    {
        printf("\n\n ICAP Functions\n");
        printf("1) Select ICAP pin to test\n");
        printf("2) Enable ICAP trigger\n");
        printf("3) Disable ICAP trigger\n");
        printf("4) Get counter value\n");
        printf("5) Display interrupt frequency\n");
        printf("6) Enable RC6 processing\n");
        printf("7) Disable RC6 processing\n");
        printf("0) Exit\n\n");;

        choice = Prompt();
        switch (choice)
        {
            case 0:
                return;

            case 1:
                printf("Current pin selected = %d\n", chanNo + 1);
                printf("ICAP pin 3 is connected to IR2_IN, ICAP pin 4 is connected to IR1_IN\n");
                printf("Enter the ICAP pin you want to test (3 or 4):\n");
                rdstr(str);
                sscanf (str, "%d", &lval);
                if (lval != 3 && lval != 4)
                {
                    printf("Invalid ICAP channel, enter only 3 or 4\n");
                    break;
                }
                chanNo = lval - 1;
                hIcpChan = bcmGetIcpChannelHandle (chanNo);
                BICP_GetEventHandle (hIcpChan, &hEvent);
                break;

            case 2:
                printf("Enter 0 for negative edge, 1 for positive edge, 2 for both:\n");
                rdstr(str);
                sscanf (str, "%d", &lval);
                if (lval > 2)
                    printf("Invalid entry, must be either 0, 1, or 2\n");
                else
                    BICP_EnableEdge (hIcpChan, (BICP_EdgeConfig)lval);

                printf ("\nWaiting for trigger...\n");
                
                /* Wait for interrupt to trigger */
                BKNI_WaitForEvent (hEvent, BKNI_INFINITE);
                
                printf("Interrupt triggered\n");
                BICP_GetTimerCnt (hIcpChan, &count);
                printf("Count for ICAP pin %d = %04xh\n", chanNo + 1, count);
                break;

            case 3:
                printf("Enter 0 for negative edge, 1 for positive edge, 2 for both:\n");
                rdstr(str);
                sscanf (str, "%d", &lval);
                if (lval > 2)
                    printf("Invalid entry, must be either 0, 1, or 2\n");
                else
                    BICP_DisableEdge (hIcpChan, (BICP_EdgeConfig)lval);

                break;

            case 4:
                printf("Getting counter value for ICAP pin %d\n", chanNo + 1);
                BICP_GetTimerCnt (hIcpChan, &count);
                printf("Count for ICAP pin %d = %04xh\n", chanNo + 1, count);
                break;

            case 5:
#if 0
                NEXUS_Core_ConnectInterrupt(INT1_ID_UPG_TMR_CPU_INTR, (NEXUS_Core_InterruptFunction)TimerISR, (void *)0, 0);
                NEXUS_Core_DisableInterrupt (INT1_ID_UPG_TMR_CPU_INTR);
                TIMER->TimerCtl0 = 0x6978; /* 1 ms */ 
                TIMER->TimerMask = 0x01;
                Timer0IntCount = 0;

                printf("Press 'r' to reset, press any other key to quit\n");
                TIMER->TimerCtl0 |= TIMERENABLE;
                NEXUS_Core_EnableInterrupt (INT1_ID_UPG_TMR_CPU_INTR);
                BICP_ResetIntCount(hIcpChan);

                while (1)
                {
                    key = det_in_char();                  
                    if (key == 'r')
                    {
                        BICP_DisableInt(hIcpChan);
                        NEXUS_Core_DisableInterrupt (INT1_ID_UPG_TMR_CPU_INTR);
                        Timer0IntCount = 0;
                        BICP_ResetIntCount(hIcpChan);
                        BICP_EnableInt(hIcpChan);
                        NEXUS_Core_EnableInterrupt (INT1_ID_UPG_TMR_CPU_INTR);
                    }
                    else if (key > 0)
                        break;
                    
                    if (Timer0IntCount > 0)
                    {
                        uint32_t freqCount;
                        BICP_GetIntCount(hIcpChan, &freqCount);
                        freq = (double)freqCount * 1000.0 / (double)Timer0IntCount;
                        printf("isr freq = %f \t\r", freq);
                    }
                }

                NEXUS_Core_DisableInterrupt (INT1_ID_UPG_TMR_CPU_INTR);
                TIMER->TimerCtl0 &= ~TIMERENABLE;            
#else
                printf("Not enabled\n");
#endif
                break;

            case 6:
                printf("RC6 processing now enabled.  Current pin selected = %d.  To change pin selection, use option 1.  To disable RC6 processing, use option 7.\n", chanNo + 1);
                BICP_EnableEdge (hIcpChan, BICP_EdgeConfig_eBoth);
                BICP_EnableRC6(hIcpChan, bcmCallBack);
                break;

            case 7:
                BICP_DisableRC6(hIcpChan);
                break;

            default:
                printf("\nInvalid Choice!\n\n");
                break;
        }
    }
}

#else

#include <stdio.h>

void bcmIcapTest (void)
{
    printf("Not enabled\n");
}

#endif
