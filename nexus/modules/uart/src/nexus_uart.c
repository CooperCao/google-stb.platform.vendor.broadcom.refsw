/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
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
 **************************************************************************/
#include "nexus_uart_module.h"
#include "burt.h"
#include "priv/nexus_uart_standby_priv.h"
#include "nexus_platform_features.h"

BDBG_MODULE(nexus_uart);

NEXUS_ModuleHandle g_NEXUS_uartModule;
struct {
    NEXUS_UartModuleSettings settings;
    BURT_Handle urt;
} g_NEXUS_uart;

/****************************************
* Module functions
***************/

void NEXUS_UartModule_GetDefaultSettings(NEXUS_UartModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_GetDefaultCommonModuleSettings(&pSettings->common);
}

NEXUS_ModuleHandle NEXUS_UartModule_Init(const NEXUS_UartModuleSettings *pSettings)
{
    NEXUS_ModuleSettings moduleSettings;
    BURT_Settings urtSettings;
    BERR_Code rc;

    BDBG_ASSERT(!g_NEXUS_uartModule);

    if (pSettings) {
        g_NEXUS_uart.settings = *pSettings;
    }
    else {
        NEXUS_UartModule_GetDefaultSettings(&g_NEXUS_uart.settings);
    }

    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_AdjustModulePriority(moduleSettings.priority, &g_NEXUS_uart.settings.common);
    g_NEXUS_uartModule = NEXUS_Module_Create("uart", &moduleSettings);

    BURT_GetDefaultSettings(&urtSettings, g_pCoreHandles->chp);
    rc = BURT_Open(&g_NEXUS_uart.urt, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &urtSettings);
    if (rc) {rc = BERR_TRACE(rc); return NULL;}

    return g_NEXUS_uartModule;
}

void NEXUS_UartModule_Uninit()
{
    BURT_Close(g_NEXUS_uart.urt);
    NEXUS_Module_Destroy(g_NEXUS_uartModule);
    g_NEXUS_uartModule = NULL;
}

/****************************************
* API functions
***************/

typedef struct NEXUS_Uart
{
    NEXUS_OBJECT(NEXUS_Uart);
    bool opened;
    bool txCallbackArmed;
    bool rxCallbackArmed;
    BURT_ChannelHandle urtChannel;
    NEXUS_UartSettings settings;
    uint8_t *pTxFifo;
    unsigned txRead;
    unsigned txWrite;
    uint8_t *pRxFifo;
    unsigned rxRead;
    unsigned rxWrite;
    NEXUS_UartStatus status;

    /* Internal callbacks */
    BKNI_EventHandle txEvent;
    NEXUS_EventCallbackHandle txEventCallback;

    /* User callbacks */
    NEXUS_TaskCallbackHandle txDoneCallback;
    NEXUS_IsrCallbackHandle errorCallback;
    NEXUS_IsrCallbackHandle rxReadyCallback;
} NEXUS_Uart;

static NEXUS_Uart g_NEXUS_uarts[NEXUS_NUM_UARTS];

static void NEXUS_Uart_P_RxCallback_isr(NEXUS_UartHandle uart);
static void NEXUS_Uart_P_TxCallback(void * context);
static void NEXUS_Uart_P_DataReady_isr(void * context, int param);

static int NEXUS_Uart_P_FifoData_isr(int rPtr, int wPtr, int size)
{
    /* if wPtr == rPtr, it is empty. */
    if ( wPtr >= rPtr )
        return wPtr - rPtr;
    else
        return (size - rPtr) + wPtr;
}

static int NEXUS_Uart_P_FifoFreeSpace_isr(int rPtr, int wPtr, int size)
{
    /* -1 is required to prevent overflowing the buffer. */
    if ( wPtr >= rPtr )
        return (size - wPtr) + rPtr - 1;
    else
        return rPtr - wPtr - 1;
}

void NEXUS_Uart_GetDefaultSettings(NEXUS_UartSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->rxBufferSize = 256;
    pSettings->txBufferSize = 256;
    pSettings->baudRate = 38400;
    pSettings->parity = NEXUS_UartParity_eNone;
    pSettings->dataBits = NEXUS_UartDataBits_e8;
    pSettings->stopBits = NEXUS_UartStopBits_e1;
}

NEXUS_UartHandle NEXUS_Uart_Open(unsigned index, const NEXUS_UartSettings *pSettings)
{
    BERR_Code errCode;
    unsigned totalChannels;
    NEXUS_UartSettings settings;
    NEXUS_UartHandle uart;

    BURT_GetTotalChannels(g_NEXUS_uart.urt, &totalChannels);
    if ( index >= totalChannels || index >= NEXUS_NUM_UARTS )
    {
        BDBG_ERR(("unable to open Uart[%d]", index));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }

    if ( g_NEXUS_uarts[index].opened == true )
    {
        BDBG_ERR(("UART %d is already open", index));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        return NULL;
    }

    if ( NULL == pSettings )
    {
        NEXUS_Uart_GetDefaultSettings(&settings);
        pSettings = &settings;
    }

    uart = &g_NEXUS_uarts[index];
    NEXUS_OBJECT_INIT(NEXUS_Uart, uart);
    uart->opened = true;
    uart->errorCallback = NEXUS_IsrCallback_Create(uart, NULL);
    uart->rxReadyCallback = NEXUS_IsrCallback_Create(uart, NULL);
    uart->txDoneCallback = NEXUS_TaskCallback_Create(uart, NULL);

    errCode = BKNI_CreateEvent(&uart->txEvent);
    if (errCode) {
        errCode = BERR_TRACE(errCode);
        goto error;
    }

    uart->txEventCallback = NEXUS_RegisterEvent(uart->txEvent, NEXUS_Uart_P_TxCallback, uart);
    if (NULL == uart->txEventCallback) {
        errCode = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
        goto error;
    }

    if ( pSettings->txBufferSize == 0 && pSettings->rxBufferSize == 0 )
    {
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto error;
    }

    if ( pSettings->txBufferSize > 0 )
    {
        uart->pTxFifo = BKNI_Malloc(pSettings->txBufferSize);
        if ( NULL == uart->pTxFifo )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto error;
        }
    }

    if ( pSettings->rxBufferSize > 0 )
    {
        uart->pRxFifo = BKNI_Malloc(pSettings->rxBufferSize);
        if ( NULL == uart->pRxFifo )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto error;
        }
    }

    errCode = NEXUS_Uart_SetSettings(uart, pSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto error;
    }

    return uart;

error:
    NEXUS_Uart_Close(uart);
    return NULL;
}

static void NEXUS_Uart_P_Finalizer(NEXUS_UartHandle uart)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Uart, uart);

    if (uart->urtChannel)
    {
        BURT_EnableRxInt(uart->urtChannel, false);
        BURT_EnableTxInt(uart->urtChannel, false);
        BURT_CloseChannel(uart->urtChannel);
    }
    if ( uart->pTxFifo )
    {
        BKNI_Free(uart->pTxFifo);
    }
    if ( uart->pRxFifo )
    {
        BKNI_Free(uart->pRxFifo);
    }

    NEXUS_IsrCallback_Destroy(uart->errorCallback);
    NEXUS_IsrCallback_Destroy(uart->rxReadyCallback);
    NEXUS_TaskCallback_Destroy(uart->txDoneCallback);

    if (uart->txEventCallback) {
        NEXUS_UnregisterEvent(uart->txEventCallback);
    }
    if (uart->txEvent) {
        BKNI_DestroyEvent(uart->txEvent);
    }

    NEXUS_OBJECT_DESTROY(NEXUS_Uart, uart);
    uart->opened = false;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_Uart, NEXUS_Uart_Close);

void NEXUS_Uart_GetSettings(NEXUS_UartHandle uart, NEXUS_UartSettings *pSettings)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Uart, uart);
    *pSettings = uart->settings;
}

NEXUS_Error NEXUS_Uart_SetSettings(NEXUS_UartHandle uart, const NEXUS_UartSettings *pSettings)
{
    NEXUS_Error errCode;
    NEXUS_UartSettings settings;

    NEXUS_OBJECT_ASSERT(NEXUS_Uart, uart);

    if ( NULL == pSettings )
    {
        NEXUS_Uart_GetDefaultSettings(&settings);
        pSettings = &settings;
    }

    if ( uart->urtChannel )
    {
        if ( pSettings->baudRate != uart->settings.baudRate ||
             pSettings->dataBits != uart->settings.dataBits ||
             pSettings->stopBits != uart->settings.stopBits ||
             pSettings->parity != uart->settings.parity )
        {
            /* BURT does not support changing these on the fly.  Close the channel and re-open it */
            BURT_CloseChannel(uart->urtChannel);
            uart->urtChannel = NULL;
        }
    }

    if ( NULL == uart->urtChannel )
    {
        BURT_ChannelSettings channelSettings;

        BURT_GetChannelDefaultSettings(g_NEXUS_uart.urt, uart-g_NEXUS_uarts, &channelSettings);
        channelSettings.rxIntMode = true;
        channelSettings.txIntMode = true;
        channelSettings.rxEnable = channelSettings.txEnable = true;
        channelSettings.baud = pSettings->baudRate;
        channelSettings.bits = (pSettings->dataBits == NEXUS_UartDataBits_e7)?BURT_DataBits_eDataBits7:BURT_DataBits_eDataBits8;
        switch ( pSettings->parity )
        {
        case NEXUS_UartParity_eNone:
            channelSettings.parity = BURT_Parity_eNone;
            break;
        case NEXUS_UartParity_eOdd:
            channelSettings.parity = BURT_Parity_eOdd;
            break;
        case NEXUS_UartParity_eEven:
            channelSettings.parity = BURT_Parity_eEven;
            break;
        default:
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        errCode = BURT_OpenChannel(g_NEXUS_uart.urt, &uart->urtChannel, uart-g_NEXUS_uarts, &channelSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        uart->txRead = uart->txWrite = 0;
        uart->rxRead = uart->rxWrite = 0;
        uart->rxCallbackArmed = true;

        /* Register our DataReady isr with BURT. Once we registered an ISR callback,
           BURT will not use the event mechanism.
        */
        BURT_RegisterCallback(uart->urtChannel, NEXUS_Uart_P_DataReady_isr);
    }

    NEXUS_IsrCallback_Set(uart->errorCallback, &pSettings->error);
    NEXUS_IsrCallback_Set(uart->rxReadyCallback, &pSettings->rxReady);
    NEXUS_TaskCallback_Set(uart->txDoneCallback, &pSettings->txDone);

    uart->settings = *pSettings;

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_Uart_Read(NEXUS_UartHandle uart, void *pData, size_t numBytes, size_t *pBytesRead)
{
    size_t rxReadIndex;
    size_t bytesRemaining;
    size_t numBytesRead=0;
    uint8_t *pCharData = (uint8_t *)pData;
    NEXUS_OBJECT_ASSERT(NEXUS_Uart, uart);
    BDBG_ASSERT(NULL != pData);
    BDBG_ASSERT(NULL != pBytesRead);

    /* We need to use the rxWrite pointer to determine bytes remaining. Do it
       in a critical section so we don't collide with the RX isr.
    */
    BKNI_EnterCriticalSection();
    /* Find out how many bytes are in our FIFO */
    bytesRemaining = NEXUS_Uart_P_FifoData_isr(uart->rxRead, uart->rxWrite, uart->settings.rxBufferSize);
    /* Capture the read index pointer while in the critical section */
    rxReadIndex = uart->rxRead;
    /* Let ISR know we're ready to be called again. Do this now because we've just taken
       a snapshot of how much data there is, so we need to be called again when new data
       shows up.
    */
    uart->rxCallbackArmed = true;
    BKNI_LeaveCriticalSection();

    /* If it's more than the caller can take, adjust */
    if (bytesRemaining > numBytes) {
        bytesRemaining = numBytes;
    }

    while (numBytesRead < bytesRemaining)
    {
        pCharData[numBytesRead++] = uart->pRxFifo[rxReadIndex++];
        if (rxReadIndex >= uart->settings.rxBufferSize)
        {
            rxReadIndex = 0;
        }
    }

    *pBytesRead = numBytesRead;

    BKNI_EnterCriticalSection();
    /* We've read all the data we can from the FIFO. Do a block move of our read index pointer */
    uart->rxRead = rxReadIndex;
    BKNI_LeaveCriticalSection();


    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_Uart_Write(NEXUS_UartHandle uart, const void *pData, size_t bufferSize, size_t *pBytesWritten)
{
    size_t txWriteIndex;
    size_t sizeRemaining;
    size_t numBytesWritten=0;
    uint8_t *pCharData = (uint8_t *)pData;
    NEXUS_OBJECT_ASSERT(NEXUS_Uart, uart);
    BDBG_ASSERT(NULL != pData);
    BDBG_ASSERT(NULL != pBytesWritten);

    /* We can't send bytes directly into BURT if there is data in our nexus
       FIFO, otherwise we interrupt the TX Callback sequence. The safest way
       to do this is to push everything into our fifo and let all the TX
       activity get handled by our TX callback.
    */
    BKNI_EnterCriticalSection();
    sizeRemaining = NEXUS_Uart_P_FifoFreeSpace_isr(uart->txRead, uart->txWrite, uart->settings.txBufferSize);
    txWriteIndex = uart->txWrite;
    BKNI_LeaveCriticalSection();

    /* If caller is giving us more than we can take, adjust... */
    if (bufferSize > sizeRemaining) {
        bufferSize = sizeRemaining;
    }

    while (numBytesWritten < bufferSize)
    {
        uart->pTxFifo[txWriteIndex++] = pCharData[numBytesWritten++];
        if ( txWriteIndex >= uart->settings.txBufferSize )
        {
            txWriteIndex = 0;
        }
    }

    *pBytesWritten = numBytesWritten;

    BKNI_EnterCriticalSection();
    uart->txWrite = txWriteIndex;
    /* Arm the callback so user gets notified when output FIFO drains */
    uart->txCallbackArmed = true;
    BKNI_LeaveCriticalSection();

    /* Notify the TX task */
    BKNI_SetEvent(uart->txEvent);


    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_Uart_GetStatus(NEXUS_UartHandle uart, NEXUS_UartStatus *pStatus)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Uart, uart);
    BDBG_ASSERT(NULL != pStatus);

    *pStatus = uart->status;

    return BERR_SUCCESS;
}

static void NEXUS_Uart_P_RxCallback_isr(NEXUS_UartHandle uart)
{
    NEXUS_Error rc;
    unsigned sizeRemaining;
    uint32_t maxRead;
    BERR_Code errCode;

    NEXUS_OBJECT_ASSERT(NEXUS_Uart, uart);

    if (BURT_IsRxDataAvailable_Isr(uart->urtChannel) == false) {
        return;
    }

    sizeRemaining = NEXUS_Uart_P_FifoFreeSpace_isr(uart->rxRead, uart->rxWrite, uart->settings.rxBufferSize);

    if (sizeRemaining == 0) {
        rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
        return;
    }

    if (uart->rxWrite < uart->rxRead) {
        maxRead = sizeRemaining;
    }
    else {
        if (uart->rxRead) {
            maxRead = uart->settings.rxBufferSize - uart->rxWrite;
        }
        else {
            maxRead = uart->settings.rxBufferSize - uart->rxWrite - 1;
        }
    }

    if (maxRead)
    {
        uint32_t bytesRead;
        BURT_RxError error;

        /* This is safe, but could be more optimal if it involved fewer function calls by grouping data */
        errCode = BURT_Read_Isr(uart->urtChannel, uart->pRxFifo + uart->rxWrite, maxRead, &bytesRead, &error);
        /* Ignore the return error code. It only gets set if the RX error code
           is non-zero. We only need to look at the RX error code
        */
        if ( error != BURT_RxError_eNoError )
        {
            switch ( error )
            {
            default:
                break;
            case BURT_RxError_eReceiverOverRun:
                uart->status.rxOverrun++;
                BDBG_WRN(("Receiver Overrun on UART %d", uart-g_NEXUS_uarts));
                break;
            case BURT_RxError_eReceiverFrameError:
                uart->status.rxFrameError++;
                BDBG_WRN(("Receiver Frame Error on UART %d", uart-g_NEXUS_uarts));
                break;
            case BURT_RxError_eReceiverParityError:
                uart->status.rxParityError++;
                BDBG_WRN(("Receiver Parity Error on UART %d", uart-g_NEXUS_uarts));
                break;
            }
            if ( uart->settings.error.callback )
            {
                NEXUS_IsrCallback_Fire_isr(uart->errorCallback);
            }
        }

        uart->rxWrite += bytesRead;
        uart->status.rxBytes += bytesRead;
        if ( uart->rxWrite >= uart->settings.rxBufferSize )
        {
            uart->rxWrite = 0;
        }
    }

    /* Re-enable RX interrupt. We need an ISR version of this call... */
    BURT_EnableRxInt(uart->urtChannel, true);

    if ( uart->rxCallbackArmed && uart->settings.rxReady.callback )
    {
        NEXUS_IsrCallback_Fire_isr(uart->rxReadyCallback);
        /* Don't fire the RX task again until it's ready */
        uart->rxCallbackArmed = false;
    }
}

static void NEXUS_Uart_P_TxCallback(void *context)
{
    NEXUS_UartHandle uart = (NEXUS_UartHandle)context;
    unsigned bytesRemaining;
    unsigned maxWrite;
    uint32_t urtBufferSize=0;
    BERR_Code errCode;

    NEXUS_OBJECT_ASSERT(NEXUS_Uart, uart);

    BKNI_EnterCriticalSection();
    bytesRemaining = NEXUS_Uart_P_FifoData_isr(uart->txRead, uart->txWrite, uart->settings.txBufferSize);

    /* Calculate the max bytes we can send into BURT, up to the end of our
       circular buffer. Trying to handle the wrap would be too complicated.
    */
    maxWrite = (uart->txRead <= uart->txWrite) ? bytesRemaining : (uart->settings.txBufferSize - uart->txRead);
    BKNI_LeaveCriticalSection();

    /* If we have no data to transmit, it means a) we just completed transmitting
       all our data or b) BURT is receiving data -- at which point it always
       informs us of the TX ready interrupt because it doesn't mask properly
    */
    if (maxWrite == 0)
    {
        if (uart->txCallbackArmed && uart->settings.txDone.callback) {
            NEXUS_TaskCallback_Fire(uart->txDoneCallback);
            uart->txCallbackArmed = false;
        }
        return;
    }

    /* Don't loop here. Send what we can and let the tx interrupt tell us when to
       send more.
    */
    if ((urtBufferSize=BURT_GetAvailTxFifoCnt_Isr(uart->urtChannel)) > 0 )
    {
        if (urtBufferSize > maxWrite) {
            urtBufferSize = maxWrite;
        }
        errCode = BURT_Write(uart->urtChannel, uart->pTxFifo + uart->txRead, urtBufferSize);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            return;
        }
        BKNI_EnterCriticalSection();
        uart->txRead += urtBufferSize;
        /* The reason this simple check works is because we limited our transmit size
           above so we didn't have to deal with the buffer wrap.
        */
        if (uart->txRead >= uart->settings.txBufferSize)
        {
            uart->txRead = 0;
        }
        BKNI_LeaveCriticalSection();
        uart->status.txBytes += urtBufferSize;
    }
}

static void NEXUS_Uart_P_DataReady_isr(void * context, int param)
{
    NEXUS_Error rc;
    NEXUS_UartHandle uart;

    /* BURT doesn't let us pass in our own context so we need to search our list */
    {
        int i;
        for (i = 0; i < NEXUS_NUM_UARTS; i++) {
            if (g_NEXUS_uarts[i].urtChannel == context) {
                break;
            }
        }
        if (i == NEXUS_NUM_UARTS) {
            /* Failed to find our uart */
            rc = BERR_TRACE(BERR_INVALID_PARAMETER);
            return;
        }
        uart = &g_NEXUS_uarts[i];
    }

    NEXUS_OBJECT_ASSERT(NEXUS_Uart, uart);

    /* RX: param == 0; TX: param == 1. No enum types in burt.h */
    switch (param) {
        case 0:
            /* Handle RX at ISR time */
            NEXUS_Uart_P_RxCallback_isr(uart);
            break;
        case 1:
            /* Handle TX in deferred task */
            BKNI_SetEvent_isr(uart->txEvent);
            break;
        default:
            rc = BERR_TRACE(param);
    }

    return;
}

NEXUS_Error NEXUS_UartModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    unsigned i;
    NEXUS_Error rc;
    BSTD_UNUSED(pSettings);

    if (enabled) {
        for (i=0; i<NEXUS_NUM_UARTS; i++) {
            NEXUS_UartHandle uart = &g_NEXUS_uarts[i];
            if (uart->opened) {
                BURT_CloseChannel(uart->urtChannel); /* BURT_CloseChannel disables both Rx and Tx interrupts */
                uart->urtChannel = NULL;
            }
        }
    }
    else {
        for (i=0; i<NEXUS_NUM_UARTS; i++) {
            NEXUS_UartHandle uart = &g_NEXUS_uarts[i];
            if (uart->opened) {
                rc = NEXUS_Uart_SetSettings(uart, &uart->settings); /* NEXUS_Uart_SetSettings calls BURT_OpenChannel */
                if (!rc) { return BERR_TRACE(rc); }
            }
        }
    }
    return NEXUS_SUCCESS;
#else
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(enabled);
    return NEXUS_SUCCESS;
#endif
}

