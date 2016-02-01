/******************************************************************************* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant* to the terms and conditions of a separate, written license agreement executed
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
******************************************************************************/

#ifndef BI2C_PRIVATE_H__
#define BI2C_PRIVATE_H__

#include "bi2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Data Transfer Format */
typedef enum
{
	BI2C_P_eWriteOnly = 0,
	BI2C_P_eReadOnly  = 1,
	BI2C_P_eReadWrite = 2,
	BI2C_P_eWriteRead = 3
} BI2C_P_XferType;

/***************************************************************************
Summary:
	This function performs a read operation with 8-bit target sub address.

Description:
	This function is used to read from a target with 8-bit sub address.

Returns:
	TODO:

See Also:
	BI2C_P_ReadNoAck(), BI2C_P_ReadA16(), BI2C_P_ReadNoAddr(), BI2C_EDDCRead().

****************************************************************************/
BERR_Code BI2C_P_Read(
	void *context,				/* Device channel handle */
	uint16_t chipAddr,					/* chip address */
	uint32_t subAddr,					/* 8-bit sub address */
	uint8_t *pData,				/* pointer to memory location to store read data  */
	size_t length						/* number of bytes to read */
	);

/***************************************************************************
Summary:
	This function performs a read operation with 8-bit target sub address
	and no ack.

Description:
	This function is used to read from a target with 8-bit sub address.

Returns:
	TODO:

See Also:
	BI2C_P_Read(), BI2C_P_ReadA16(), BI2C_P_ReadNoAddr(), BI2C_EDDCRead().

****************************************************************************/
BERR_Code BI2C_P_ReadNoAck(
	void *context,				/* Device channel handle */
	uint16_t chipAddr,					/* chip address */
	uint32_t subAddr,					/* 8-bit sub address */
	uint8_t *pData,				/* pointer to memory location to store read data  */
	size_t length						/* number of bytes to read */
	);

/***************************************************************************
Summary:
	This function performs a read operation with 16-bit target sub address.

Description:
	This function is used to read from a target with 16-bit sub address.

Returns:
	TODO:

See Also:
	BI2C_P_Read(), BI2C_P_ReadNoAck(), BI2C_P_ReadNoAddr(), BI2C_EDDCRead().

****************************************************************************/
BERR_Code BI2C_P_ReadA16(
	void *context,				/* Device channel handle */
	uint16_t chipAddr,					/* chip address */
	uint32_t subAddr,					/* 16-bit sub address */
	uint8_t *pData,				/* pointer to memory location to store read data  */
	size_t length						/* number of bytes to read */
	);

/***************************************************************************
Summary:
	This function performs a read operation with 24-bit target sub address.

Description:
	This function is used to read from a target with 24-bit sub address.

Returns:
	TODO:

See Also:
	BI2C_P_Read(), BI2C_P_ReadNoAck(), BI2C_P_ReadNoAddr(), BI2C_EDDCRead().

****************************************************************************/
BERR_Code BI2C_P_ReadA24(
	void *context,				/* Device channel handle */
	uint16_t chipAddr,					/* chip address */
	uint32_t subAddr,					/* 24-bit sub address */
	uint8_t *pData,				/* pointer to memory location to store read data  */
	size_t length						/* number of bytes to read */
	);

/***************************************************************************
Summary:
	This function performs a read operation with no target sub address.

Description:
	This function is used to read from a target without specifying sub
	address.  This is normally used when the user wants to continue reading
	from where the last read command left off.

Returns:
	TODO:

See Also:
	BI2C_P_Read(), BI2C_P_ReadNoAck(), BI2C_P_ReadA16(), BI2C_EDDCRead().

****************************************************************************/
BERR_Code BI2C_P_ReadNoAddr(
	void *context,				/* Device channel handle */
	uint16_t chipAddr,					/* chip address */
	uint8_t *pData,				/* pointer to memory location to store read data  */
	size_t length						/* number of bytes to read */
	);

/***************************************************************************
Summary:
	This function performs a write operation with 8-bit target sub address.

Description:
	This function is used to write data to a target with 8-bit sub address.

Returns:
	TODO:

See Also:
	BI2C_P_WriteA16(), BI2C_P_WriteNoAck(), BI2C_P_WriteNoAddr(), BI2C_EDDCwrite().

****************************************************************************/
BERR_Code BI2C_P_Write(
	void *context,				/* Device channel handle */
	uint16_t chipAddr,					/* chip address */
	uint32_t subAddr,					/* 8-bit sub address */
	const uint8_t *pData,				/* pointer to data to write */
	size_t length						/* number of bytes to write */
	);

/***************************************************************************
Summary:
	This function performs a write operation with 8-bit target sub address
	and no ack.

Description:
	This function is used to write data to a target with 8-bit sub address.

Returns:
	TODO:

See Also:
	BI2C_P_WriteA16(), BI2C_P_WriteA16(), BI2C_P_WriteNoAddr(), BI2C_EDDCwrite().

****************************************************************************/
BERR_Code BI2C_P_WriteNoAck(
	void *context,				/* Device channel handle */
	uint16_t chipAddr,					/* chip address */
	uint32_t subAddr,					/* 8-bit sub address */
	const uint8_t *pData,				/* pointer to data to write */
	size_t length						/* number of bytes to write */
	);

/***************************************************************************
Summary:
	This function performs a write operation with 16-bit target sub address.

Description:
	This function is used to write data to a target with 16-bit sub address.

Returns:
	TODO:

See Also:
	BI2C_P_Write(), BI2C_P_WriteNoAck(), BI2C_P_WriteNoAddr(), BI2C_EDDCwrite().

****************************************************************************/
BERR_Code BI2C_P_WriteA16(
	void *context,				/* Device channel handle */
	uint16_t chipAddr,					/* chip address */
	uint32_t subAddr,					/* 16-bit sub address */
	const uint8_t *pData,				/* pointer to data to write */
	size_t length						/* number of bytes to write */
	);

/***************************************************************************
Summary:
	This function performs a write operation with 24-bit target sub address.

Description:
	This function is used to write data to a target with 16-bit sub address.

Returns:
	TODO:

See Also:
	BI2C_P_Write(), BI2C_P_WriteNoAck(), BI2C_P_WriteNoAddr(), BI2C_EDDCwrite().

****************************************************************************/
BERR_Code BI2C_P_WriteA24(
	void *context,				/* Device channel handle */
	uint16_t chipAddr,					/* chip address */
	uint32_t subAddr,					/* 24-bit sub address */
	const uint8_t *pData,				/* pointer to data to write */
	size_t length						/* number of bytes to write */
	);

/***************************************************************************
Summary:
	This function performs a write operation with no target sub address.

Description:
	This function is used to write from a target without specifying sub
	address.  This is normally used when the user wants to continue writing
	from where the last write command left off.

Returns:
	TODO:

See Also:
	BI2C_P_Write(), BI2C_P_WriteNoAck(), BI2C_P_WriteA16(), BI2C_EDDCwrite().

****************************************************************************/
BERR_Code BI2C_P_WriteNoAddr(
	void *context,				/* Device channel handle */
	uint16_t chipAddr,					/* chip address */
	const uint8_t *pData,				/* pointer to data to write */
	size_t length						/* number of bytes to write */
	);

/***************************************************************************
Summary:
	This function performs a write operation with no target sub address
	and no ack.

Description:
	This function is used to write from a target without specifying sub
	address and without waiting for an ack.  This is normally used when
	the user wants to continue writing from where the last write command
	left off.

Returns:
	TODO:

See Also:
	BI2C_P_Write(), BI2C_P_WriteNoAck(), BI2C_P_WriteA16(), BI2C_EDDCwrite().

****************************************************************************/
BERR_Code BI2C_P_WriteNoAddrNoAck(
	void *context,				/* Device channel handle */
	uint16_t chipAddr,					/* chip address */
	const uint8_t *pData,				/* pointer to data to write */
	size_t length						/* number of bytes to write */
	);

/***************************************************************************
Summary:
	This function performs a write operation to an nvram device.

Description:
	This function is used to write an nvram device.

Returns:
	TODO:

See Also:
	BI2C_P_Write(), BI2C_P_WriteNoAck(), BI2C_P_WriteA16(), BI2C_EDDCwrite().

****************************************************************************/
BERR_Code BI2C_P_WriteNvram(
	void *context,						/* Device channel handle */
	uint16_t chipAddr,					/* chip address */
	uint32_t subAddr,					/* 8-bit sub address */
	const uint8_t *pData,				/* pointer to data to write */
	size_t length						/* number of bytes to write */
	);

/***************************************************************************
Summary:
	This function performs a EDDC read operation.

Description:
	This function is used to perform an Enhanced Display Data Channel read
	protocol.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BI2C_P_ReadEDDC(
	void *context,				/* Device channel handle */
	uint8_t	chipAddr,			/* chip address */
	uint8_t segment,			/* EDDC segment pointer */
	uint32_t subAddr,			/* 8-bit sub address */
	uint8_t *pData,				/* pointer to memory location to store read data  */
	size_t length				/* number of bytes to read */
	);

/***************************************************************************
Summary:
	This function performs a EDDC write operation.

Description:
	This function is used to perform an Enhanced Display Data Channel write
	protocol.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BI2C_P_WriteEDDC(
	void *context,				/* Device channel handle */
	uint8_t	chipAddr,			/* chip address */
	uint8_t segment,			/* EDDC segment pointer */
	uint32_t subAddr,			/* 8-bit sub address */
	const uint8_t *pData,		/* pointer to data to write */
	size_t length				/* number of bytes to write */
	);

/***************************************************************************
Summary:
	This function is the main read function

Description:
	This function is called by public read functions

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BI2C_P_ReadCmd
(
	BI2C_ChannelHandle 	hChn,			/* Device channel handle */
	uint16_t		chipAddr,			/* i2c chip address.  this is unshifted */
	void 			*pSubAddr,			/* pointer to register address */
	uint8_t 		numSubAddrBytes,	/* number of bytes in register address */
	uint8_t			*pData,				/* storage */
	size_t			numBytes			/* number of bytes to read */
);

/***************************************************************************
Summary:
	This function is the main read function

Description:
	This function is called by public read functions

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BI2C_P_ReadCmdNoAck
(
	BI2C_ChannelHandle 	hChn,			/* Device channel handle */
	uint16_t		chipAddr,			/* i2c chip address.  this is unshifted */
	void 			*pSubAddr,			/* pointer to register address */
	uint8_t 		numSubAddrBytes,	/* number of bytes in register address */
	uint8_t			*pData,				/* storage */
	size_t			numBytes			/* number of bytes to read */
);

/***************************************************************************
Summary:
	This function is the main write function

Description:
	This function is called by public write functions

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BI2C_P_WriteCmd
(
	BI2C_ChannelHandle	hChn,			/* Device channel handle */
	uint16_t			chipAddr,		/* i2c chip address.  this is unshifted */
	void				*pSubAddr,		/* pointer to register address */
	uint8_t				numSubAddrBytes,	/* number of bytes in register address */
	const uint8_t		*pData,			/* storage */
	size_t				numBytes,		/* number of bytes to write */
	bool				isNvram			/* is this a nvram access? */
);

/***************************************************************************
Summary:
	This function is the main write function

Description:
	This function is called by public write functions

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BI2C_P_WriteCmdNoAck
(
	BI2C_ChannelHandle 	hChn,			/* Device channel handle */
	uint16_t			chipAddr,		/* i2c chip address.  this is unshifted */
	void 				*pSubAddr,		/* pointer to register address */
	uint8_t 			numSubAddrBytes,	/* number of bytes in register address */
	const uint8_t		*pData,			/* storage */
	size_t				numBytes		/* number of bytes to write */
);

/***************************************************************************
Summary:
	This function waits for I2C transfer to complete

Description:
	This function waits for event if interrupt is enabled.  If interrupt is
	not enabled, it stays in a loop and polls until timeout occurs.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BI2C_P_WaitForCompletion
(
	BI2C_ChannelHandle 	hChn,				/* Device channel handle */
	uint32_t			numBytes			/* number of bytes to transfer */
);

/***************************************************************************
Summary:
	This function waits for an NVRAM I2C slave device transfer to complete

Description:
	This function waits a calculated amount of time for an NVRAM I2C
	slave device to respond with an acknowledgement.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BI2C_P_WaitForNVMToAcknowledge
(
	BI2C_ChannelHandle  hChn,				/* Device channel handle */
	uint16_t			chipAddr,			/* i2c chip address.  this is unshifted */
	void				*pSubAddr,			/* pointer to register address */
	uint8_t				numSubAddrBytes,	/* number of bytes in register address */
	size_t				numBytes			/* number of bytes to write */
);

/***************************************************************************
Summary:
	This function waits for data to leave the 8 byte I2C fifo.

Description:
	This function waits a calculated amount of time for the 8 byte I2C fifo
	to empty rather than rely on waiting for an ack.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BI2C_P_WaitForDataToLeaveFIFO
(
	BI2C_ChannelHandle	hChn,				/* Device channel handle */
	uint32_t			numBytes			/* number of bytes to transfer */
);

#ifdef __cplusplus
}
#endif

#endif
