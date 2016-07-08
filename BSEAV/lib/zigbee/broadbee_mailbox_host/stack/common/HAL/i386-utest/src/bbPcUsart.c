/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/common/HAL/i386-utest/src/bbPcUsart.c $
*
* DESCRIPTION:
*   pc usart implementation.
*
* $Revision: 1195 $
* $Date: 2014-01-23 13:03:59Z $
*
****************************************************************************************/
/* NOTE: For more information see http://msdn.microsoft.com/en-us/library/aa450602.aspx */
/************************* INCLUDES ****************************************************/
// #include "bbPcDbg.h"
#include "bbPcUsart.h"
#include "private/bbPcPrivateUsart.h"
#include <pthread.h>

#include <stdio.h>

/************************* IMPLEMENTATION **********************************************/
static PC_UsartDescriptor_t pcUsart[MAX_PC_USART_CHANNEL_AMOUNT];

static void openSerialPort(UsartChannel_t channel);
static void configureSerialPort(UsartChannel_t channel);
static void setSerialPortTimeouts(UsartChannel_t channel);
static void *eventHandler(void *arg);

static void openSerialPort(UsartChannel_t channel)
{
    static uint8_t comPortNumber[3] = {0, 0, 0};
    uint8_t i = 2;

    /* converting COM port number to string */
    {
        UsartChannel_t copyChannel = channel;
        while (copyChannel)
        {
            comPortNumber[i--] = '0' + copyChannel % 10;
            copyChannel /= 10;
        }
    }
    // creating string like "\\\\.\\COM255" for WinAPI function
    memcpy(pcUsart[channel - 1].portName, "\\\\.\\COM", sizeof("\\\\.\\COM"));
    // (void)comPortNumber;
    memcpy(pcUsart[channel - 1].portName + sizeof("\\\\.\\COM") - 1, comPortNumber + i + 1, sizeof(comPortNumber) - i - 1);

    pcUsart[channel - 1].hPort = CreateFile(pcUsart[channel - 1].portName,    // Pointer to the name of the port
                                            GENERIC_READ | GENERIC_WRITE,    // Access (read-write) mode
                                            0,                               // Share mode
                                            NULL,                            // Pointer to the security attribute
                                            OPEN_EXISTING,                   // How to open the serial port
                                            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // Port attributes
                                            NULL);                           // Handle to port with attribute to copy

    if (INVALID_HANDLE_VALUE == pcUsart[channel - 1].hPort)
    {
        fprintf(stderr, "Failed to open serial port %s. Error %ld\n", pcUsart[channel - 1].portName, GetLastError());
        SYS_DbgAssert(false, PCUSART_OPENSERIALPORT_0);
    }
}

static void configureSerialPort(UsartChannel_t channel)
{
    (void)channel;
    DCB PortDCB;

    // Initialize the DCBlength member.
    PortDCB.DCBlength = sizeof(DCB);

    // Get the default port setting information.
    SYS_DbgAssert(GetCommState(pcUsart[channel - 1].hPort, &PortDCB), PCUSART_CONFIGURESERIALPORT_0);

    // Change the DCB structure settings.
    PortDCB.BaudRate = CBR_115200;        // Current baud

    PortDCB.fBinary = TRUE;               // Binary mode; no EOF check
    PortDCB.fParity = TRUE;               // Enable parity checking
    PortDCB.fOutxCtsFlow = FALSE;         // No CTS output flow control
    PortDCB.fOutxDsrFlow = FALSE;         // No DSR output flow control
    PortDCB.fDtrControl = DTR_CONTROL_ENABLE; // DTR flow control type
    PortDCB.fDsrSensitivity = FALSE;      // DSR sensitivity
    PortDCB.fTXContinueOnXoff = TRUE;     // XOFF continues Tx
    PortDCB.fOutX = FALSE;                // No XON/XOFF out flow control
    PortDCB.fInX = FALSE;                 // No XON/XOFF in flow control
    PortDCB.fErrorChar = FALSE;           // Disable error replacement
    PortDCB.fNull = FALSE;                // Disable null stripping
    PortDCB.fRtsControl = RTS_CONTROL_ENABLE; // RTS flow control
    PortDCB.fAbortOnError = FALSE;        // Do not abort reads/writes on error

    PortDCB.ByteSize = 8;                 // Number of bits/byte, 4-8
    PortDCB.Parity = NOPARITY;            // 0-4=no,odd,even,mark,space
    PortDCB.StopBits = ONESTOPBIT;        // 0,1,2 = 1, 1.5, 2

    // Configure the port according to the specifications of the DCB structure.
    SYS_DbgAssert(SetCommState(pcUsart[channel - 1].hPort, &PortDCB), PCUSART_CONFIGURESERIALPORT_1);
}

static void setSerialPortTimeouts(UsartChannel_t channel)
{
    (void)channel;
    // Retrieve the timeout parameters for all read and write operations on the port.
    COMMTIMEOUTS CommTimeouts;
    SYS_DbgAssert(GetCommTimeouts(pcUsart[channel - 1].hPort, &CommTimeouts), PCUSART_SETSERIALPORTTIMEOUTS_0);

    // Change the COMMTIMEOUTS structure settings.
    CommTimeouts.ReadIntervalTimeout = MAXDWORD;
    CommTimeouts.ReadTotalTimeoutMultiplier = 0;
    CommTimeouts.ReadTotalTimeoutConstant = 0;
    CommTimeouts.WriteTotalTimeoutMultiplier = 10;
    CommTimeouts.WriteTotalTimeoutConstant = 10;

    // Set the timeout parameters for all read and write operations on the port.
    SYS_DbgAssert(SetCommTimeouts(pcUsart[channel - 1].hPort, &CommTimeouts), PCUSART_SETSERIALPORTTIMEOUTS_1);
}

/************************************************************************************//**
    \brief initializes usart
    \param[in] channel - USART channel
    \param[in] txComplete - pointer to the tx interrupt handler
    \param[in] rxComplete - pointer to the rx interrupt handler
    \param[in] link - pointer to the upper descriptor.
****************************************************************************************/
void PC_UsartEnable(UsartChannel_t channel, UsartVector_t txComplete, UsartVector_t rxComplete, void *link)
{
    SYS_DbgAssert(txComplete, PCUSART_PCUSARTENABLE_0);
    SYS_DbgAssert(rxComplete, PCUSART_PCUSARTENABLE_1);
    SYS_DbgAssert(link, PCUSART_PCUSARTENABLE_2);
    SYS_DbgAssert(!pcUsart[channel - 1].isOpened, PCUSART_PCUSARTENABLE_3);

    pcUsart[channel - 1].txComplete = txComplete;
    pcUsart[channel - 1].rxComplete = rxComplete;
    pcUsart[channel - 1].parrentLink = link;

    openSerialPort(channel);
    configureSerialPort(channel);
    setSerialPortTimeouts(channel);

    {
        pthread_t thread;
        pthread_attr_t threadAttr;
        pthread_attr_init(&threadAttr);
        pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
        {
            int result = pthread_create(&thread, &threadAttr, eventHandler, (void *) &pcUsart[channel - 1]);
            SYS_DbgAssert(0 <= result, PCUSART_PCUSARTENABLE_4);
        }
    }
    /* waiting for thread start */
    while (!pcUsart[channel - 1].isOpened)
        Sleep(20);
}

/************************************************************************************//**
    \brief disables usart
    \param[in] channel - USART channel
****************************************************************************************/
void PC_UsartDisable(UsartChannel_t channel)
{
    SYS_DbgAssert(pcUsart[channel - 1].isOpened, PCUSART_PCUSARTDISABLE_0);
    // SetCommMask(pcUsart[channel - 1].hPort, 0);
    PurgeComm(pcUsart[channel - 1].hPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
    CloseHandle(pcUsart[channel - 1].hPort);
    while (pcUsart[channel - 1].isOpened)
        Sleep(20);
    /* workaround: we need spend time until port is closed */
    Sleep(300);
}

static void *eventHandler(void *arg)
{
    PC_UsartDescriptor_t *const descr = (PC_UsartDescriptor_t *)arg;
    DWORD  dwCommMask;
    descr->isOpened = true;

    while (descr->isOpened && INVALID_HANDLE_VALUE != descr->hPort)
    {
        SetCommMask(descr->hPort, EV_RXCHAR | EV_TXEMPTY);
        /* Wait for an event to occur for the port. */
        if (!WaitCommEvent(descr->hPort, &dwCommMask, 0))
        {
            break;
        }
        else
        {
            if (EV_TXEMPTY & dwCommMask)
            {
                if (descr->txComplete && descr->parrentLink)
                    descr->txComplete(descr->parrentLink);
                else
                    SYS_DbgAssert(false, PCUSART_EVENTHANDLER_0);
            }

            if (EV_RXCHAR & dwCommMask)
            {
                DWORD bytesRead;
                do
                {
                    uint8_t value;
                    bool isEnded = ReadFile(descr->hPort, &value, sizeof(value), &bytesRead, &descr->readOverlapped);

                    if (!isEnded)
                    {
                        SYS_DbgAssert(ERROR_IO_PENDING != GetLastError(), PCUSART_EVENTHANDLER_1);
                        if (!GetOverlappedResult(descr->hPort, &descr->readOverlapped, &bytesRead, true))
                            SYS_DbgAssert(false, PCUSART_EVENTHANDLER_2);
                    }
                    if (sizeof(value) == bytesRead)
                    {
                        descr->receivedByte = value;

                        if (descr->rxComplete && descr->parrentLink)
                            descr->rxComplete(descr->parrentLink);
                        else
                            SYS_DbgAssert(false, PCUSART_EVENTHANDLER_3);
                    }
                }
                while (bytesRead == 1);
            }

        }
    }
    descr->isOpened = false;
    pthread_exit(NULL);
    return NULL;
}

/************************************************************************************//**
    \brief Sends one byte
    \param[in] channel - USART channel
    \param[in] value - byte value
****************************************************************************************/
void PC_UsartSendByte(UsartChannel_t channel, uint8_t value)
{
    (void)channel;
    DWORD dwNumBytesWritten;

    SYS_DbgAssert(pcUsart[channel - 1].isOpened, PCUSART_PCUSARTSENDBYTE_0);

    pcUsart[channel - 1].byteToTransmit = value;
    bool isEnded = WriteFile(pcUsart[channel - 1].hPort,             // Port handle
                             &pcUsart[channel - 1].byteToTransmit,  // Pointer to the data to write
                             sizeof(pcUsart[channel - 1].byteToTransmit), // Number of bytes to write
                             &dwNumBytesWritten,        // Pointer to the number of bytes written
                             &pcUsart[channel - 1].writeOverlapped  // Pointer to the an OVERLAPPED structure
                            );
    if (isEnded)
    {
        if (pcUsart[channel - 1].txComplete && pcUsart[channel - 1].parrentLink)
            pcUsart[channel - 1].txComplete(pcUsart[channel - 1].parrentLink);
        else
            SYS_DbgAssert(false, PCUSART_PCUSARTSENDBYTE_1);
    }
    else
        SYS_DbgAssert(ERROR_IO_PENDING == GetLastError(), PCUSART_PCUSARTSENDBYTE_2);

}

/************************************************************************************//**
    \brief Read one byte
    \param[in] channel - USART channel
    \return read value
****************************************************************************************/
uint8_t PC_UsartReceiveByte(UsartChannel_t channel)
{
    (void)channel;
    SYS_DbgAssert(pcUsart[channel - 1].isOpened, PCUSART_PCUSARTRECEIVEBYTE_0);
    return pcUsart[channel - 1].receivedByte;
}

/************************************************************************************//**
    \brief Gets number of received bytes
    \param[in] channel - USART channel
    \return bytes number
****************************************************************************************/
uint8_t PC_UsartGetRxDataSize(UsartChannel_t channel)
{
    (void)channel;
    return 0;
}
/* eof bbPcUsart.c */