/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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


/*= Module Overview *********************************************************
<verbatim>

Overview
The URT PI module controls the UART core within the BCM7038.
There are two independent UARTs that are separately configurable for
baud rate, number of bits per character (7 or 8), number of stop bits
(0, 1, or 2), and parity bit (none, odd, or even).

Design
The design for BURT PI API is broken into two parts.
  Part 1 (open/close/configuration):
    These APIs are used for opening and closing BURT device/device channel.
  Part 2 (receive data or send data)
    These APIs are used to send/receive data.

Usage
The usage of BURT involves the following:
   * Configure/Open of BURT
      * Configure BURT device for the target system
      * Open BURT device
      * Configure BURT device channel for the target system
      * Open BURT device channel
   * Receive/Send data.

Sample Code
void main( void )
{
    BURT_Handle hUrt;
    BURT_ChannelHandle hUrtChan;
    BURT_ChannelSettings defChnSettings;
    BREG_Handle hReg;
    BCHP_Handle hChip;
	BINT_Handle hInt;
    int chanNo;
	BKNI_EventHandle rxEventHandle;
	uint8_t		data[32];
	BURT_RxError		rxError;
	uint32_t			bytesRead;

    // Do other initialization, i.e. for BREG, BCHP, etc.

    BURT_Open (&hUrt, hChip, hReg, hInt, (BURT_Settings *)NULL);

    chnIdx = 0; // example for channel 0
    BURT_GetChannelDefaultSettings( hUrt, chanNo, &defChnSettings );

    // Make any changes required from the default values
    defChnSettings.rxEnable = true;
    defChnSettings.txEnable = true;
    defChnSettings.baud     = 115200;
    defChnSettings.bits     = BURT_DataBits_eDataBits8;
    defChnSettings.parity   = BURT_Parity_eNone;
    defChnSettings.intMode	= true;
    BURT_OpenChannel( hUrt, &hUrtChan, chnIdx, &defChnSettings );

	BURT_GetRxEventHandle(hUrtChan, &rxEventHandle);

	// Wait to receive a character
	BKNI_WaitForEvent(rxEventHandle, BKNI_INFINITE);

    if (BURT_Read (hUrtChan, data, 32, &bytesRead, &rxError) == BURT_ERR_RX_ERROR)
		printf("Error code is %d\n", rxError);
	else
		printf("Number of bytes read was %d\n", bytesRead);
}

</verbatim>
***************************************************************************/


#ifndef BURT_H__
#define BURT_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "berr_ids.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
	Error Codes specific to BURT

Description:

See Also:

****************************************************************************/
#define BURT_ERR_NOTAVAIL_CHN_NO		BERR_MAKE_CODE(BERR_URT_ID, 0)
#define BURT_ERR_RX_ERROR				BERR_MAKE_CODE(BERR_URT_ID, 1)
#define BURT_ERR_TX_FIFO_NOT_AVAIL		BERR_MAKE_CODE(BERR_URT_ID, 2)

/***************************************************************************
Summary:
	The handles for urt module.

Description:
	Since BURT is a device channel, it has main device handle as well
	as a device channel handle.

See Also:
	BURT_Open(), BURT_OpenChannel()

****************************************************************************/
typedef struct BURT_P_Handle				*BURT_Handle;
typedef struct BURT_P_ChannelHandle			*BURT_ChannelHandle;

/***************************************************************************
Summary:
	Enumeration for URT parity setting

Description:
	This enumeration defines the parity setting option for URT

See Also:
	None.

****************************************************************************/
typedef enum
{
   BURT_Parity_eNone,
   BURT_Parity_eOdd,
   BURT_Parity_eEven
} BURT_Parity;

/***************************************************************************
Summary:
	Enumeration for URT number of bits setting

Description:
	This enumeration defines the number of bits setting for UART

See Also:
	None.

****************************************************************************/
typedef enum
{
	BURT_DataBits_eDataBits5,
	BURT_DataBits_eDataBits6,
	BURT_DataBits_eDataBits7,
	BURT_DataBits_eDataBits8
} BURT_DataBits;

/***************************************************************************
Summary:
	Enumeration for URT number of stop bits setting

Description:
	This enumeration defines the number of stop bits setting for UART

See Also:
	None.

****************************************************************************/
typedef enum
{
	BURT_StopBits_eOneBit,
	BURT_StopBits_eTwoBit
} BURT_StopBits;

/***************************************************************************
Summary:
	Enumeration for URT receiver error

Description:
	This enumeration defines the receiver errors

See Also:
	None.

****************************************************************************/
typedef enum
{
   BURT_RxError_eNoError,
   BURT_RxError_eReceiverOverRun,
   BURT_RxError_eReceiverFrameError,
   BURT_RxError_eReceiverParityError
} BURT_RxError;

/***************************************************************************
Summary:
	Required default settings structure for URT module.

Description:
	The default setting structure defines the default configure of
	URT when the device is opened.  Since BURT is a device
	channel, it also has default settings for a device channel.
	Currently there are no parameters for device setting.

See Also:
	BURT_Open(), BURT_OpenChannel()

****************************************************************************/
typedef void *BURT_Settings;

typedef struct BURT_ChannelSettings
{
   bool      	rxEnable;				/* receiver enable */
   bool      	txEnable;				/* transmitter enable */
   uint32_t		baud;					/* baud rate */
   BURT_DataBits bits;					/* number of bits */
   BURT_Parity	parity;					/* parity setting */
   bool			intMode;				/* interrupt enable flag. If enable, rxIntMode and txIntMode are not used */
   bool			rxIntMode;				/* rx interrupt enable flag */
   bool			txIntMode;				/* tx interrupt enable flag */
   BURT_StopBits stopBits;				/* number of stop bit */
} BURT_ChannelSettings;

/***************************************************************************
Summary:
	This function opens URT module.

Description:
	This function is responsible for opening BURT module. When BURT is
	opened, it will create a module handle and configure the module based
	on the default settings. Once the device is opened, it must be closed
	before it can be opened again.

Returns:
	TODO:

See Also:
	BURT_Close(), BURT_OpenChannel(), BURT_CloseChannel(),
	BURT_GetDefaultSettings()

****************************************************************************/
BERR_Code BURT_Open(
	BURT_Handle *pURT,					/* [out] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_Handle hRegister,				/* [in] Register handle */
	BINT_Handle hInterrupt,				/* [in] Interrupt handle */
	const BURT_Settings *pDefSettings	/* [in] Default settings */
	);

/***************************************************************************
Summary:
	This function closes URT module.

Description:
	This function is responsible for closing BURT module. Closing BURT
	will free main BURT handle. It is required that all opened
	BURT channels must be closed before calling this function. If this
	is not done, the results will be unpredicable.

Returns:
	TODO:

See Also:
	BURT_Open(), BURT_CloseChannel()

****************************************************************************/
BERR_Code BURT_Close(
	BURT_Handle hDev					/* [in] Device handle */
	);

/***************************************************************************
Summary:
	This function returns the default settings for URT module.

Description:
	This function is responsible for returns the default setting for
	BURT module. The returning default setting should be when
	opening the device.

Returns:
	TODO:

See Also:
	BURT_Open()

****************************************************************************/
BERR_Code BURT_GetDefaultSettings(
	BURT_Settings *pDefSettings,		/* [out] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	);

/***************************************************************************
Summary:
	This function returns the total number of channels supported by
	URT module.

Description:
	This function is responsible for getting total number of channels
	supported by BURT module, since BURT device is implemented as a
	device channel.

Returns:
	TODO:

See Also:
	BURT_OpenChannel(), BURT_ChannelDefaultSettings()

****************************************************************************/
BERR_Code BURT_GetTotalChannels(
	BURT_Handle hDev,					/* [in] Device handle */
	unsigned int *totalChannels			/* [out] Returns total number downstream channels supported */
	);

/***************************************************************************
Summary:
	This function gets default setting for a URT module channel.

Description:
	This function is responsible for returning the default setting for
	channel of BURT. The return default setting is used when opening
	a channel.

Returns:
	TODO:

See Also:
	BURT_OpenChannel()

****************************************************************************/
BERR_Code BURT_GetChannelDefaultSettings(
	BURT_Handle hDev,					/* [in] Device handle */
	unsigned int channelNo,				/* [in] Channel number to default setting for */
    BURT_ChannelSettings *pChnDefSettings /* [out] Returns channel default setting */
    );



/***************************************************************************
Summary:
	This function resets the Tx and Rx Fifo's

Description:

Returns:
	TODO:

****************************************************************************/
void BURT_ResetTxRx(
	BURT_ChannelHandle 	hChn			/* Device channel handle */
	);

/***************************************************************************
Summary:
	This function opens URT module channel.

Description:
	This function is responsible for opening BURT module channel. When a
	BURT channel is	opened, it will create a module channel handle and
	configure the module based on the channel default settings. Once a
	channel is opened, it must be closed before it can be opened again.

Returns:
	TODO:

See Also:
	BURT_CloseChannel(), BURT_GetChannelDefaultSettings()

****************************************************************************/
BERR_Code BURT_OpenChannel(
	BURT_Handle hDev,					/* [in] Device handle */
	BURT_ChannelHandle *phChn,			/* [out] Returns channel handle */
	unsigned int channelNo,				/* [in] Channel number to open */
	const BURT_ChannelSettings *pChnDefSettings /* [in] Channel default setting */
	);

/***************************************************************************
Summary:
	This function closes URT module channel.

Description:
	This function is responsible for closing BURT module channel. Closing
	BURT channel it will free BURT channel handle. It is required that all
	opened BURT channels must be closed before closing BURT.

Returns:
	TODO:

See Also:
	BURT_OpenChannel(), BURT_CloseChannel()

****************************************************************************/
BERR_Code BURT_CloseChannel(
	BURT_ChannelHandle hChn				/* [in] Device channel handle */
	);

/***************************************************************************
Summary:
	This function gets URT module device handle based on
	the device channel handle.

Description:
	This function is responsible returning BURT module handle based on the
	BURT module channel.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BURT_GetDevice(
	BURT_ChannelHandle hChn,			/* [in] Device channel handle */
	BURT_Handle *pURT					/* [out] Returns Device handle */
	);

/***************************************************************************
Summary:
	This function checks to see if data is received

Description:
	This function is used to poll for data received

Returns:
	TODO:

See Also:


****************************************************************************/
bool BURT_IsRxDataAvailable(
	BURT_ChannelHandle	hChn			/* [in] Device channel handle */
	);

/***************************************************************************
Summary:
	This function checks to see if data is received, called in ISR

Description:
	This function is used to poll for data received, called in ISR

Returns:
	TODO:

See Also:


****************************************************************************/
bool BURT_IsRxDataAvailable_Isr(
	BURT_ChannelHandle	hChn			/* Device channel handle */
	);

/***************************************************************************
Summary:
	This function returns the TX fifo size available

Description:
	This function gets the available TX fifo count.  If the TX fifo is empty
	the count will be the maximum size of FIFO (in this case, it's 32 bytes).
	If the TX fifo is not empty, it will return 0 since it doesn't know
	exactly how many bytes are left in FIFO.  It only knows that the FIFO
	is not empty.

Returns:
	TODO:

See Also:


****************************************************************************/
uint32_t BURT_GetAvailTxFifoCnt (
	BURT_ChannelHandle	hChn			/* [in] Device channel handle */
	);

/***************************************************************************
Summary:
	This function returns the TX fifo size available, called in ISR

Description:
	This function gets the available TX fifo count.  If the TX fifo is empty
	the count will be the maximum size of FIFO (in this case, it's 32 bytes).
	If the TX fifo is not empty, it will return 0 since it doesn't know
	exactly how many bytes are left in FIFO.  It only knows that the FIFO
	is not empty. Called in ISR.

Returns:
	TODO:

See Also:

****************************************************************************/
uint32_t BURT_GetAvailTxFifoCnt_Isr(
	BURT_ChannelHandle	hChn			/* Device channel handle */
	);

/***************************************************************************
Summary:
	This function enables or disables TX interrupt

Description:
	This function is used to turn on and off TX interrupt

Returns:
	TODO:

See Also:


****************************************************************************/
void BURT_EnableTxInt(
	BURT_ChannelHandle 	hChn,			/* [in] Device channel handle */
	bool				enableTxInt		/* [in] enable flag for transmitter interrupt */
	);

/***************************************************************************
Summary:
	This function enables or disables RX interrupt

Description:
	This function is used to turn on and off RX interrupt

Returns:
	TODO:

See Also:


****************************************************************************/
void BURT_EnableRxInt(
	BURT_ChannelHandle 	hChn,			/* [in] Device channel handle */
	bool				enableRxInt		/* [in] enable flag for receiver interrupt */
	);

/***************************************************************************
Summary:
	This function reads a byte from UART fifo

Description:
	This function is used to get a data byte from the UART fifo.

Returns:
	TODO:

See Also:


****************************************************************************/
BERR_Code BURT_Read_Isr (
	BURT_ChannelHandle	hChn,			/* [in] Device channel handle */
	uint8_t				*data,			/* [in] pointer to memory to store data */
	uint32_t			numBytes,		/* [in] number of bytes to read */
	uint32_t			*bytesRead,		/* [out] number of actual bytes read */
	BURT_RxError		*rxError		/* [out] receive error code */
	);

/***************************************************************************
Summary:
	This function reads a byte from UART fifo

Description:
	This function is used to get a data byte from the UART fifo.

Returns:
	TODO:

See Also:


****************************************************************************/
BERR_Code BURT_Read (
	BURT_ChannelHandle	hChn,			/* [in] Device channel handle */
	uint8_t				*data,			/* [in] pointer to memory to store data */
	uint32_t			numBytes,		/* [in] number of bytes to read */
	uint32_t			*bytesRead,		/* [out] number of actual bytes read */
	BURT_RxError		*rxError		/* [out] receive error code */
	);

/***************************************************************************
Summary:
	This function write a byte to the UART fifo

Description:
	This function is used to send a byte out of the UART.

Returns:
	TODO:

See Also:


****************************************************************************/
BERR_Code BURT_Write (
	BURT_ChannelHandle	hChn,			/* [in] Device channel handle */
	uint8_t			*data,				/* [in] pointers to data to send */
	uint32_t		numBytes			/* [in] number of bytes to write */
	);

/***************************************************************************
Summary:
	This function gets the receive error

Description:
	This function is used to get the receive error status after an error occurs.

Returns:
	TODO:

See Also:


****************************************************************************/
BURT_RxError BURT_GetRxError (
	BURT_ChannelHandle	hChn			/* [in] Device channel handle */
	);

/***************************************************************************
Summary:
	This function gets the receive error, called in ISR

Description:
	This function is used to get the receive error status after an error occurs. Called in ISR.

Returns:
	TODO:

See Also:


****************************************************************************/
BURT_RxError BURT_GetRxError_Isr (
	BURT_ChannelHandle	hChn			/* Device channel handle */
	);

/***************************************************************************
Summary:
	This function gets the transmit event handle for BURT module.

Description:
	This function is responsible for getting the event handle. The
	application code should use this function get BURT's event handle,
	which the application should use to pend on.  The URT ISR will
	set the event if a transmit interrupt happens.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BURT_GetTxEventHandle(
	BURT_ChannelHandle	hChn,			/* [in] Device channel handle */
	BKNI_EventHandle 	*phEvent		/* [out] Returns event handle */
	);

/***************************************************************************
Summary:
	This function gets the receive event handle for BURT module.

Description:
	This function is responsible for getting the event handle. The
	application code should use this function get BURT's event handle,
	which the application should use to pend on.  The URT ISR will
	set the event if a receive interrupt happens.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BURT_GetRxEventHandle(
	BURT_ChannelHandle	hChn,			/* [in] Device channel handle */
	BKNI_EventHandle 	*phEvent		/* [out] Returns event handle */
	);

/***************************************************************************
Summary:
	This function registers callback function.

Description:
	This function registers callback function used during interrupt handling.
	Normally, event is posted to notify the ocurrance of the interrupt.
	If callback is registered, then callback function is used instead
	of posting event.

Returns:
	TODO:

See Also:

****************************************************************************/
void BURT_RegisterCallback(
	BURT_ChannelHandle	hChn,			/* [in] Device channel handle */
	BINT_CallbackFunc callbackFunc	/* [in] callback function to register */
	);

#ifdef __cplusplus
}
#endif

#endif



