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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include <stdio.h>

#if NEXUS_HAS_UART
#include "nexus_platform.h"
#include "nexus_uart.h"

BDBG_MODULE(uart);

static size_t UartWrite(NEXUS_UartHandle uart, BKNI_EventHandle txEvent, const char *pData, size_t dataSize);

static void event_callback(void *pContext, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent(pContext);
}

int uartTest(void)
{
    NEXUS_UartSettings uartSettings;
    NEXUS_UartHandle uart;
    BKNI_EventHandle rxEvent;
    BKNI_EventHandle txEvent;
#define BUFSIZE 1024
    char buf[BUFSIZE];
    NEXUS_Error rc;

    BKNI_CreateEvent(&rxEvent);
    BKNI_CreateEvent(&txEvent);

    /* This will implement a simple loopback with a serial port.  */
    NEXUS_Uart_GetDefaultSettings(&uartSettings);
    uartSettings.baudRate = 115200;
    uartSettings.parity = NEXUS_UartParity_eNone;
    uartSettings.dataBits = NEXUS_UartDataBits_e8;
    uartSettings.stopBits = NEXUS_UartStopBits_e1;

    uartSettings.rxReady.callback = event_callback;
    uartSettings.rxReady.context = rxEvent;
    uartSettings.txDone.callback = event_callback;
    uartSettings.txDone.context = txEvent;
    printf("Opening UART2\n");
    uart = NEXUS_Uart_Open(2, &uartSettings);
    if ( NULL == uart )
    {
        BDBG_ERR(("Unable to open UART2"));
        return -1;
    }

    printf("This application assumes that you have set the pinmuxing to enable UART2 on your platform.\n"
           "This is typically not enabled by default.\n");
    /* Hints:
    See SUN_TOP_CTRL_UART_ROUTER_SEL in RDB. port_2_cpu_sel needs to be set to NO_CPU.
    The RDB comments for port_2_cpu_sel should tell you the pinmuxing required for uart_txd_2 and uart_rxd_2.
    uart_cts_2 and uart_rts_2 are for hardware flow control. This app does not use that.
    See nexus code in nexus_platform_pinmux.c. You can add a run time or compile time option there if desired.
    */
#if 0
    /* manual option */
    printf("Please manually set your pinmuxing now. Press ENTER when that's done.\n");
    getchar();
#endif

    printf("Printing 'hello' to terminal.\n");
    sprintf(buf, "hello\r\n");
    UartWrite(uart, txEvent, buf, 7);

    printf("Starting loopback. Please type into the UART2 console. It should be echoed to that console and also printed on the OS console.  Press 'CTRL-C' to quit.\n");
    for ( ;; )
    {
        unsigned numRead;

        rc = NEXUS_Uart_Read(uart, buf, BUFSIZE-1, (size_t *)&numRead);
        BDBG_ASSERT(!rc);

        if (!numRead) {
            BKNI_WaitForEvent(rxEvent, 1000 /*BKNI_INFINITE*/);
/*            if (det_in_char() == 'q') break; */
            continue;
        }

        buf[numRead] = 0; /* terminate the string */
        printf("%s\n", buf); /* print the string to console (i.e. UART0) */

        /* echo the string back to UART2 */
        UartWrite(uart, txEvent, buf, numRead);
    }

    NEXUS_Uart_Close(uart);

    return 0;
}

/**
helper function - blocking uart write
**/
static size_t UartWrite(NEXUS_UartHandle uart, BKNI_EventHandle txEvent, const char *pData, size_t dataSize)
{
    unsigned totalWritten = 0;
    while (totalWritten < dataSize) {
        NEXUS_Error rc;
        unsigned n;

        rc = NEXUS_Uart_Write(uart, &pData[totalWritten], dataSize - totalWritten, (size_t *)&n);
        BDBG_ASSERT(!rc);

        if (!n) {
            BKNI_WaitForEvent(txEvent, BKNI_INFINITE);
            continue;
        }
        totalWritten += n;
    }
    return 0;
}
#else
int uartTest(void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif
