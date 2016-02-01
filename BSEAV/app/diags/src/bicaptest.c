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

#ifdef DIAGS_BICAP_TEST

#include <stdio.h>
#include "bstd.h"
#include "bkni_multi.h"
#include "bbcp.h"
#include "upg_handles.h"
#include "prompt.h"
#include "bchp_hif_cpu_intr1.h"
#include "priv/nexus_core.h"
#include "nexus_input_capture.h"
#include "nexus_input_capture_init.h"

/***********************************************************************
 *                      Function Prototypes
 ***********************************************************************/

#if __cplusplus
extern "C" {
#endif	   

void bcmBicapTest (void);

#if __cplusplus
}
#endif

static NEXUS_InputCaptureHandle handle[MAX_BCP_CHANNELS];

void capture_callback(void *p1, int p2)
{
    unsigned num_read;
    NEXUS_InputCaptureData data;
    BSTD_UNUSED(p1);

    while (1)
    {
        NEXUS_InputCapture_ReadData(
            handle[p2],
            &data, /* [out] attr{nelem=numEntries;nelem_out=pNumRead} */
            1, /* max number that can be read */
            &num_read /* [out] actual number that were read */
        );
        if (num_read)
            printf("ch%d:  data received from sw fifo = %04xh, polarity=%s\n", p2, data.clockCycles, data.polarity?"positive":"negative");
        else
            break;
    }
}

/***********************************************************************
 *
 *  bcmBicapTest
 * 
 *  BICAP test
 *
 ***********************************************************************/
void bcmBicapTest (void)
{
    NEXUS_InputCaptureTimeoutHandle timeoutHandle[4];
    NEXUS_InputCaptureSettings settings;
/*	NEXUS_InputCaptureFifoInactivityTimeoutSettings inact;*/
    NEXUS_InputCaptureTimeoutSettings timeoutSettings;
    int	choice, chanNo = 0, val;
    char str[20];
    uint32_t i;
    NEXUS_InputCaptureTimeoutSettings timeout;
    uint32_t which;

    NEXUS_InputCapture_GetDefaultSettings(&settings);
    settings.dataReady.callback = capture_callback;
    for (i=0; i<MAX_BCP_CHANNELS; i++)
    {
        settings.dataReady.param = i;	/* channel */
        settings.type = NEXUS_InputCaptureType_eBicap;
        handle[i] = NEXUS_InputCapture_Open(i, &settings);
        if (handle==NULL)
        {
            printf("error from NEXUS_InputCapture_Open\n");
            return;
        }
    }

    /* Setup the timeouts */
    NEXUS_InputCaptureTimeout_GetDefaultSettings(&timeoutSettings);
    timeoutSettings.inputCapture = handle[0];
    for (i=0; i<4; i++)
        timeoutHandle[i] = NEXUS_InputCaptureTimeout_Open(i, &timeoutSettings);

    while(1)
    {
        printf("\n\n BICAP Functions\n");
        printf("\n  Current settings (current chan=%d):\n", chanNo);

        for (i=0; i<MAX_BCP_CHANNELS; i++)
        {
            NEXUS_InputCapture_GetSettings(handle[i], &settings);

            printf("    Input channel %d:\n", i);
            printf("      tout_clk_div=0x%x, sys_clk_div=0x%x, cnt_mode=%d\n", settings.bicap.timeoutClockDivisor, settings.bicap.systemClockDivisor, settings.bicap.countMode);
            printf("      falling edge=%s, rising edge=%s, invert input=%s\n",
                (settings.edgeSelect==NEXUS_InputCaptureEdgeSelect_ePositive) || (settings.edgeSelect==NEXUS_InputCaptureEdgeSelect_eBoth)?"enabled":"disabled",
                (settings.edgeSelect==NEXUS_InputCaptureEdgeSelect_eNegative) || (settings.edgeSelect==NEXUS_InputCaptureEdgeSelect_eBoth)?"enabled":"disabled",
                settings.bicap.invertSignalInput?"true":"false");
            printf("      filter:  bypass=%s, clk_sel=%s, val=%x\n", settings.filter.enable?"not bypassed":"bypassed", (settings.filter.clockSelect==NEXUS_InputCaptureFilterClockSelect_eBicapClock)?"bicap_clk":"sys_clk", settings.filter.value);
        }
        printf("    Timeouts:\n");
        for (i=0; i<4; i++)
        {
            int j;
            NEXUS_InputCaptureTimeoutSettings timeout;
            NEXUS_InputCaptureTimeout_GetSettings(timeoutHandle[i], &timeout);
            for (j=0; j<MAX_BCP_CHANNELS; j++)
            {
                if (timeout.inputCapture == handle[j]) break;
            }
            printf("      Timeout %d:  input_sel=%d, edge_sel=%s, clk_sel=%s, tout=0x%x\n", i, j,
                (timeout.edgeSelect==NEXUS_InputCaptureEdgeSelect_eNegative)?"falling":(timeout.edgeSelect==NEXUS_InputCaptureEdgeSelect_ePositive)?"rising":(timeout.edgeSelect==NEXUS_InputCaptureEdgeSelect_eBoth)?"both":"unknown",
                (timeout.clockSelect==NEXUS_InputCaptureClockSelect_eBicapClock)?"BICAP clk":"TOUT clk", 
                timeout.clockCycles);
        }

#if 0
        if (BERR_SUCCESS == NEXUS_InputCapture_GetFifoInactTimeout(&inact))
        {
            printf("    FIFO Inactivity Timeouts:  \n");
            printf("      bicap_clk_sel=%s\n", inact.bicapClkSel==0?"use bicap clk for input 0":"use bicap clk for input 1");
            printf("      event_sel=%s, clk_sel=%s, tout=0x%x\n", inact.eventSel==0?"no data in or out":inact.eventSel==1?"no data goes in":"no data goes out", inact.clkSel==0?"use BICAP clk":"use TOUT clk", inact.timeout );
        }
        else
            printf("error from BBCP_GetFifoInactTimeout\n");
#endif            

        printf("\n");

        printf("0) Exit\n");;
        printf("1) Set input channel to test\n");
        printf("2) Set filter\n");
        printf("3) Set timeout\n");
/*		printf("4) Set inactivity timeout\n");*/
        printf("4) Set edge detection\n");
        printf("5) Enable BICAP detection\n");

        choice = PromptChar();
        switch (choice)
        {
            case '0':
                return;

            case '1':
                printf("Enter input channel to test:  ");
                chanNo = NoPrompt();
                printf("\n");
                if (chanNo > MAX_BCP_CHANNELS-1)
                {
                    printf("Invalid entry, must be either 0 to %d\n", MAX_BCP_CHANNELS-1);
                    break;
                }

                break;

            case '2':
                NEXUS_InputCapture_GetSettings(handle[chanNo], &settings);

                printf("Enter 0=use filter or 1=bypass filter:  ");
                settings.filter.enable = NoPrompt();
                if (settings.filter.enable > 1)
                {
                    printf("Invalid entry, must be 0 or 1\n");
                    break;
                }

                printf("Enter %d=bicap clk filter or %d=sys clk_filter:  ", NEXUS_InputCaptureFilterClockSelect_eBicapClock, NEXUS_InputCaptureFilterClockSelect_eSystemClock);
                settings.filter.clockSelect = NoPrompt();
                if (settings.filter.clockSelect > 1)
                {
                    printf("Invalid entry, must be either 0 or 1\n");
                    break;
                }

                printf("Enter filter value (0 to 0x7f:  ");
                settings.filter.value = NoPromptHex();
                if (settings.filter.value > 0x7f)
                {
                    printf("Invalid entry, must be either 0 to 3\n");
                    break;
                }

                NEXUS_InputCapture_SetSettings(handle[chanNo], &settings);
                break;

            case '3':
                printf("Enter which timer to use (0-3):  ");
                which = NoPrompt();
                if (which > 3)
                {
                    printf("Invalid entry, must be 0 to 3\n");
                    break;
                }

                printf("Enter which input channel (0 or 1):  ");
                val = NoPrompt();
                if (val > 1)
                {
                    printf("Invalid entry, must be either 0 or 1\n");
                    break;
                }
                timeout.inputCapture = handle[val];

                printf("Enter edge_sel:  %d=falling edge, %d=rising edge, %d=both edges:  ", NEXUS_InputCaptureEdgeSelect_eNegative, NEXUS_InputCaptureEdgeSelect_ePositive, NEXUS_InputCaptureEdgeSelect_eBoth);
                timeout.edgeSelect = NoPrompt();
                if ((timeout.edgeSelect != NEXUS_InputCaptureEdgeSelect_eNegative) && (timeout.edgeSelect != NEXUS_InputCaptureEdgeSelect_ePositive) && (timeout.edgeSelect != NEXUS_InputCaptureEdgeSelect_eBoth))
                {
                    printf("Invalid entry\n");
                    break;
                }

                printf("Timeout counter clock selector:  Enter 0=BICAP clk or 1=TOUT clk:  ");
                timeout.clockSelect = NoPrompt();
                if (timeout.clockSelect > 1)
                {
                    printf("Invalid entry, must be either 0 or 1\n");
                    break;
                }

                printf("Edge timeout value (0-0xfff):  ");
                timeout.clockCycles = NoPromptHex();

                if (timeout.clockCycles & ~0xfff)
                {
                    printf("Invalid entry, must be between 0 and 0xfff\n");
                    break;
                }

                if (NEXUS_SUCCESS != NEXUS_InputCaptureTimeout_SetSettings(timeoutHandle[which], &timeout))
                {
                    printf("Error from NEXUS_InputCapture_SetTimeout\n");
                }
                break;

            case '4':
                NEXUS_InputCapture_GetSettings(handle[chanNo], &settings);

                printf("Enter tout clk div (0-0xff):  ");
                settings.bicap.timeoutClockDivisor = NoPromptHex();

                if (settings.bicap.timeoutClockDivisor & ~0xff)
                {
                    printf("Invalid entry, must be between 0 and 0xff\n");
                    break;
                }

                printf("Enter sys clk div (0-0x3fff):  ");
                settings.bicap.systemClockDivisor = NoPromptHex();

                if (settings.bicap.systemClockDivisor & ~0x3fff)
                {
                    printf("Invalid entry, must be between 0 and 0x3fff\n");
                    break;
                }

                printf("Enter cnt mode (0-3):  ");
                settings.bicap.countMode = NoPrompt();

                if (settings.bicap.countMode > 3)
                {
                    printf("Invalid entry, must be between 0 to 3\n");
                    break;
                }

                printf("0=Negative edge detection, 1=Positive edge detection, 2=Both, 3=None:  ");
                settings.edgeSelect = NoPrompt();

                if (settings.edgeSelect > 3)
                {
                    printf("Invalid entry, must be 0, 1, 2, or 3\n");
                    break;
                }

                printf("Invert signal input: 0=disable, 1=enable):  ");
                settings.bicap.invertSignalInput = NoPrompt();

                if (settings.bicap.invertSignalInput > 1)
                {
                    printf("Invalid entry, must be 0 or 1\n");
                    break;
                }

                NEXUS_InputCapture_SetSettings(handle[chanNo], &settings);
                break;

            case '5':
                NEXUS_InputCapture_Enable(handle[chanNo]);
                printf("\nPress ENTER to exit, waiting for trigger...\n");
                while (1)
                {
                    getchar();
                    NEXUS_InputCapture_Disable(handle[chanNo]);
                    break;
                }
                break;

            default:
                printf("\nInvalid Choice!\n\n");
                break;
        }
    }

    for (i=0; i<4; i++)
        NEXUS_InputCaptureTimeout_Close(timeoutHandle[i]);

    for (i=0; i<MAX_BCP_CHANNELS; i++)
        NEXUS_InputCapture_Close(handle[i]);
}

#else

void bcmBicapTest (void)
{
    printf("Not enabled\n");
    return 0;
}

#endif /* DIAGS_BICAP_TEST */
