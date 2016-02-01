/***************************************************************************
*     (c)2004-2011 Broadcom Corporation
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
* API Description:
*   API name: Uart
*    Specific APIs related to UART Control.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_UART_H__
#define NEXUS_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Handle for the Uart Interface.

Description:

See Also:
NEXUS_Uart_Open
***************************************************************************/
typedef struct NEXUS_Uart *NEXUS_UartHandle;

/***************************************************************************
Summary:
UART parity

Description:

See Also:
NEXUS_UartSettings
***************************************************************************/
typedef enum NEXUS_UartParity
{
    NEXUS_UartParity_eNone,
    NEXUS_UartParity_eOdd,
    NEXUS_UartParity_eEven,
    NEXUS_UartParity_eMax
} NEXUS_UartParity;

/***************************************************************************
Summary:
UART data bits

Description:

See Also:
NEXUS_UartSettings
***************************************************************************/
typedef enum NEXUS_UartDataBits
{
    NEXUS_UartDataBits_e7,
    NEXUS_UartDataBits_e8,
    NEXUS_UartDataBits_eMax
} NEXUS_UartDataBits;

/***************************************************************************
Summary:
UART data bits

Description:

See Also:
NEXUS_UartSettings
***************************************************************************/
typedef enum NEXUS_UartStopBits
{
    NEXUS_UartStopBits_e0,
    NEXUS_UartStopBits_e1,
    NEXUS_UartStopBits_e2,
    NEXUS_UartStopBits_eMax
} NEXUS_UartStopBits;

/***************************************************************************
Summary:
UART Settings

Description:

See Also:
NEXUS_Uart_Open
NEXUS_Uart_GetDefaultSettings
***************************************************************************/
typedef struct NEXUS_UartSettings
{
    unsigned rxBufferSize;
    unsigned txBufferSize;

    unsigned baudRate;
    NEXUS_UartParity parity;
    NEXUS_UartDataBits dataBits;
    NEXUS_UartStopBits stopBits;

    NEXUS_CallbackDesc rxReady;
    NEXUS_CallbackDesc txDone;
    NEXUS_CallbackDesc error;
} NEXUS_UartSettings;

/***************************************************************************
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.

See Also:
NEXUS_Uart_Open
***************************************************************************/
void NEXUS_Uart_GetDefaultSettings(
    NEXUS_UartSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Open a serial port handle.

Description:

See Also:
NEXUS_Uart_Close
***************************************************************************/
NEXUS_UartHandle NEXUS_Uart_Open( /* attr{destructor=NEXUS_Uart_Close}  */
    unsigned index,
    const NEXUS_UartSettings *pSettings
    );

/***************************************************************************
Summary:
Close a serial port handle.

Description:

See Also:
NEXUS_Uart_Open
***************************************************************************/
void NEXUS_Uart_Close(
    NEXUS_UartHandle handle
    );

/***************************************************************************
Summary:
Get the settings for a UART

Description:

See Also:
NEXUS_Uart_SetSettings
***************************************************************************/
void NEXUS_Uart_GetSettings(
    NEXUS_UartHandle handle,
    NEXUS_UartSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Set the settings for a UART

Description:

See Also:
NEXUS_Uart_GetSettings
***************************************************************************/
NEXUS_Error NEXUS_Uart_SetSettings(
    NEXUS_UartHandle handle,
    const NEXUS_UartSettings *pSettings
    );

/***************************************************************************
Summary:
Read data from a serial port.

Description:
This function will read data from a serial port, up to the number of bytes
specified.  The application should continue to call this function until it
returns 0 bytes read before waiting for another callback.

See Also:
NEXUS_Uart_Write
***************************************************************************/
NEXUS_Error NEXUS_Uart_Read(
    NEXUS_UartHandle handle,
    void *pData, /* [out] attr{nelem=numBytes;nelem_out=pBytesRead;reserved=64} */
    size_t numBytes,
    size_t *pBytesRead /* [out] */
    );

/***************************************************************************
Summary:
Write data to a serial port.

Description:
This function will write data to a serial port, up to the number of bytes
specified.  If the bytes specified exceeds available buffer space, the
number of bytes written will be smaller than the number requested.  In this
case, the application should wait for a TxCallback and then continue to write
the remaining bytes.

See Also:
NEXUS_Uart_Read
***************************************************************************/
NEXUS_Error NEXUS_Uart_Write(
    NEXUS_UartHandle handle,
    const void *pData, /* [out] attr{nelem=bufferSize;reserved=64} */
    size_t bufferSize,
    size_t *pBytesWritten /* [out] */
    );

/***************************************************************************
Summary:
UART status

Description:

See Also:
NEXUS_Uart_GetStatus
***************************************************************************/
typedef struct NEXUS_UartStatus
{
    bool rxOverrun;
    bool rxFrameError;
    bool rxParityError;
    unsigned rxBytes;
    unsigned txBytes;
} NEXUS_UartStatus;

/***************************************************************************
Summary:
Get the current status of a UART.
***************************************************************************/
NEXUS_Error NEXUS_Uart_GetStatus(
    NEXUS_UartHandle handle,
    NEXUS_UartStatus *pStatus   /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_UART_H__ */

