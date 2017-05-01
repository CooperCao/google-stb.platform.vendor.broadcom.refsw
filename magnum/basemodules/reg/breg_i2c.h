/***************************************************************************
 *  Copyright (C) 2003-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 ***************************************************************************/

/*= Module Overview ********************************************************
This module provides a standard abstraction API for accessing I2C registers.
Several I2C busses may exist on a single platform. Some may be hardware based, 
while others software based (using GPIO pins). In order to hide the 
specific implementation of the I2C bus from a PortingInterface this I2C 
register access abstraction is used.

---++++ 7 or 10 Bit I2C Chip Addresses 
For all I2C functions, the chipAddr parameter is always used as the 
*un-shifted* 7 bit (bits 15:7 = 0) or 10 bit (bits 15:10 = 0) chip address. 
If (bits 9:7 == 0) then 7 bit addressing is used by default. Some I2C
implementations will return errors for 10 bit chip addresses (for example
if a hardware I2C implementation does not support 10 bit chip addresses).

---++++ Non 8-Bit Sub-Address Handling 
Although most I2C devices use 8 or 16 bit sub-addresses, some may require 
larger sub addresses. These devices must be use the BREG_I2C_WriteNoAddr and 
BREG_I2C_ReadNoAddr in order to form larger sub-addresses. 

The following example shows how to access read from a device with a 32 bit sub 
address: 

<verbatim>
BERR_Code BXXX_ReadRegister( 
			BREG_I2C_Handle cntx, 
			uint16_t chipAddr, 
			uint32_t subAddr, 
			uint8_t *pData, 
			size_t length )
{
   BREG_I2C_WriteNoAddr(cntx, chipAddr, &subAddr, sizeof(subAddr) );
   BREG_I2C_ReadNoAddr(cntx, chipAddr, pData, length);
}
</verbatim>

Writing to a device with a non 8 or 16 bit sub address is slightly different; 
you need to pre-pend your data with the sub address you wish to write. 

---++++ I2C Register Handles
Hardware and software I2C implementations must follow the defined function 
prototype definitions. All I2C PortingInterfaces supply the a function 
that initializes a standard BREG_I2C_Handle (and fills it with function 
pointers provided by the specific I2C PortingInterfaces implementation).
Please refer to the specific I2C PortingInterface implementations for more
details.

An example of this type of function would be:

<verbatim>
BERR_Code BI2C_Open( BI2C_Handle I2cHandle, BREG_I2C_Handle *pI2cRegHandle );
</verbatim>

This handle should be created during initialization and passed to the 
appropriate porting interfaces.  The porting interfaces in turn pass 
this handle to the abstract I2C functions when then call the appropriate 
I2C implementation though function pointers contained in the BREG_I2C_Handle.

---++++ Handling ThreadSafe with I2C
Since multiple modules (PortingInterface and otherwise) may be sharing the
same I2C bus, any function calls to the modules in question must be serialized
to prevent thread collisions.

	1 All calls to the modules sharing the same I2C handle must be protected
	  through the use of a mutex.  This protection must be handled by the
	  upper level software (driver, middleware, etc).
	1 Upper level software can create an additional layer between the I2C
	  RegisterInterface and the corresponding I2C PortingInterface instance
	  that implements the I2C register access.  In this layer a mutex is
	  used to protect all I2C accesses.  This could easily be done
	  using the BREG_I2C_Impl structure.
***************************************************************************/

#ifndef BREG_I2C_H
#define BREG_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

/*
Summary:
This is an opaque handle that is used for I2C register functions.
*/
typedef struct BREG_I2C_Impl *BREG_I2C_Handle;

/*
Summary:
This function writes a programmable number of I2C registers using an 8 bit
sub address.
*/
BERR_Code BREG_I2C_Write(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
						 uint8_t subAddr, /* 8 bit sub address */
						 const uint8_t *pData, /* pointer to data to write */
						 size_t length /* number of bytes to write */
						 );

/*
Summary:
This function uses software "bit-bang" to write a programmable number of I2C registers using an 8 bit
sub address.
*/
BERR_Code BREG_I2C_WriteSw(
                            BREG_I2C_Handle i2cHandle, /* I2C Handle */
                            uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
                            uint8_t subAddr, /* 8 bit sub address */
                            const uint8_t *pData, /* pointer to data to write */
                            size_t length /* number of bytes to write */
                          );

/*
Summary:
This function writes a programmable number of I2C registers using an 8 bit
sub address and no ack.
*/
BERR_Code BREG_I2C_WriteNoAck(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
						 uint8_t subAddr, /* 8 bit sub address */
						 const uint8_t *pData, /* pointer to data to write */
						 size_t length /* number of bytes to write */
						 );

/*
Summary:
This function writes a programmable number of I2C registers using a 16 bit
sub address.
*/
BERR_Code BREG_I2C_WriteA16(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
						 uint16_t subAddr, /* 16 bit sub address */
						 const uint8_t *pData, /* pointer to data to write */
						 size_t length /* number of bytes to write */
						 );

/*
Summary:
This function uses software "bit-bang" to write a programmable number of I2C registers using a 16 bit
sub address.
*/
BERR_Code BREG_I2C_WriteSwA16(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
						 uint16_t subAddr, /* 16 bit sub address */
						 const uint8_t *pData, /* pointer to data to write */
						 size_t length /* number of bytes to write */
						 );

/*
Summary:
This function writes a programmable number of I2C registers using a 24 bit
sub address.
*/
BERR_Code BREG_I2C_WriteA24(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
						 uint32_t subAddr, /* 24 bit sub address */
						 const uint8_t *pData, /* pointer to data to write */
						 size_t length /* number of bytes to write */
						 );

/*
Summary:
This function uses software "bit-bang" to write a programmable number of I2C registers using a 24 bit
sub address.
*/
BERR_Code BREG_I2C_WriteSwA24(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
						 uint32_t subAddr, /* 24 bit sub address */
						 const uint8_t *pData, /* pointer to data to write */
						 size_t length /* number of bytes to write */
						 );

/*
Summary:
This function writes a programmable number of I2C registers without a sub address
(raw write).
*/
BERR_Code BREG_I2C_WriteNoAddr(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
						 const uint8_t *pData, /* pointer to data to write */
						 size_t length /* number of bytes to write */
					     );

/*
Summary:
This function uses software "bit-bang" to write a programmable number of I2C registers without a sub address
(raw write).
*/
BERR_Code BREG_I2C_WriteNoAddr(
                               BREG_I2C_Handle i2cHandle, /* I2C Handle */
                               uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
                               const uint8_t *pData, /* pointer to data to write */
                               size_t length /* number of bytes to write */
                              );

/*
Summary:
This function writes a programmable number of I2C registers without a sub address
(raw write), and without waiting for an ack.
*/
BERR_Code BREG_I2C_WriteNoAddrNoAck(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
						 const uint8_t *pData, /* pointer to data to write */
						 size_t length /* number of bytes to write */
					     );

/*
Summary:
This function writes an I2C NVRAM device using an 8 bit sub address.
*/
BERR_Code BREG_I2C_WriteNvram(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
						 uint8_t subAddr, /* 8 bit sub address */
						 const uint8_t *pData, /* pointer to data to write */
						 size_t length /* number of bytes to write */
						 );

/*
Summary:
This function reads a programmable number of I2C registers using an 8 bit
sub address.
*/
BERR_Code BREG_I2C_Read(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
						 uint8_t subAddr, /* 8 bit sub address */
						 uint8_t *pData, /* pointer to memory location to store read data */
						 size_t length /* number of bytes to read */
						 );

/*
Summary:
This function uses software "bit-bang" to read a programmable number of I2C registers using an 8 bit
sub address.
*/
BERR_Code BREG_I2C_ReadSw(
                           BREG_I2C_Handle i2cHandle, /* I2C Handle */
                           uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
                           uint8_t subAddr, /* 8 bit sub address */
                           uint8_t *pData, /* pointer to memory location to store read data */
                           size_t length /* number of bytes to read */
                          );

/*
Summary:
This function reads a programmable number of I2C registers using an 8 bit
sub address and no ack..
*/
BERR_Code BREG_I2C_ReadNoAck(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
						 uint8_t subAddr, /* 8 bit sub address */
						 uint8_t *pData, /* pointer to memory location to store read data */
						 size_t length /* number of bytes to read */
						 );

/*
Summary:
This function reads a programmable number of I2C registers using a 16 bit
sub address.
*/
BERR_Code BREG_I2C_ReadA16(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
						 uint16_t subAddr, /* 16 bit sub address */
						 uint8_t *pData, /* pointer to memory location to store read data */
						 size_t length /* number of bytes to read */
						 );

/*
Summary:
This function uses software "bit-bang" to read a programmable number of I2C registers using a 16 bit
sub address.
*/
BERR_Code BREG_I2C_ReadSwA16(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
						 uint16_t subAddr, /* 16 bit sub address */
						 uint8_t *pData, /* pointer to memory location to store read data */
						 size_t length /* number of bytes to read */
						 );

/*
Summary:
This function reads a programmable number of I2C registers using a 24 bit
sub address.
*/
BERR_Code BREG_I2C_ReadA24(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
						 uint32_t subAddr, /* 32 bit sub address */
						 uint8_t *pData, /* pointer to memory location to store read data */
						 size_t length /* number of bytes to read */
						 );

/*
Summary:
This function uses software "bit-bang" to read a programmable number of I2C registers using a 24 bit
sub address.
*/
BERR_Code BREG_I2C_ReadSwA24(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
						 uint32_t subAddr, /* 32 bit sub address */
						 uint8_t *pData, /* pointer to memory location to store read data */
						 size_t length /* number of bytes to read */
						 );

/*
Summary:
This function reads a programmable number of I2C registers without a sub address
(raw read).
*/
BERR_Code BREG_I2C_ReadNoAddr(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
						 uint8_t *pData, /* pointer to memory location to store read data */
						 size_t length /* number of bytes to read */
						 );

/*
Summary:
This function uses software "bit-bang" to read a programmable number of I2C registers without a sub address
(raw read).
*/
BERR_Code BREG_I2C_ReadSwNoAddr(
                                 BREG_I2C_Handle i2cHandle, /* I2C Handle */
                                 uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
                                 uint8_t *pData, /* pointer to memory location to store read data */
                                 size_t length /* number of bytes to read */
                                );

/*
Summary:
This function reads a programmable number of I2C registers without a sub address and no Ack
(raw read).
*/

BERR_Code BREG_I2C_ReadNoAddrNoAck(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
						 uint8_t *pData, /* pointer to memory location to store read data */
						 size_t length /* number of bytes to read */
						 );
/*
Summary:
This function is used to perform an Enhanced Display Data Channel read protocol.
*/
BERR_Code BREG_I2C_ReadEDDC(
						BREG_I2C_Handle i2cHandle,	/* I2C Handle */
						uint8_t chipAddr,			/* chip address */
						uint8_t segment,			/* EDDC segment pointer */
						uint8_t subAddr,			/* 8-bit sub address */
						uint8_t *pData,				/* pointer to memory location to store read data  */
						size_t length				/* number of bytes to read */
						);

/*
Summary:
This function uses software "bit-bang" to read an Enhanced Display Data Channel read protocol.
*/
BERR_Code BREG_I2C_ReadSwEDDC(
						BREG_I2C_Handle i2cHandle,	/* I2C Handle */
						uint8_t chipAddr,			/* chip address */
						uint8_t segment,			/* EDDC segment pointer */
						uint8_t subAddr,			/* 8-bit sub address */
						uint8_t *pData, 			/* pointer to memory location to store read data  */
						size_t length				/* number of bytes to read */
						);

/*
Summary:
This function is used to perform an Enhanced Display Data Channel write protocol.
*/
BERR_Code BREG_I2C_WriteEDDC(
						BREG_I2C_Handle i2cHandle,	/* I2C Handle */
						uint8_t chipAddr,			/* chip address */
						uint8_t segment,			/* EDDC segment pointer */
						uint8_t subAddr,			/* 8-bit sub address */
						const uint8_t *pData,		/* pointer to data to write */
						size_t length				/* number of bytes to write */
						);


/*
Summary:
This function set up i2c for HDMI HDCP HW Ri/Pj Link integrity check
*/
BERR_Code BREG_I2C_SetupHdmiHwAccess(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint32_t dataTransferFormat, 	/* Data Transfer Format */
						 uint32_t cnt1,   /* Counter 1 value */
						 uint32_t cnt2    /* Counter 2 value */
						 );


/*
Summary:
This function is used to perform write and read as an atomic operation.
*/
BERR_Code BREG_I2C_WriteRead(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint16_t chipAddr,         /* 7 or 10 bit chip address (_unshifted_) */
						 uint8_t subAddr,           /* 8 bit sub address */
						 const uint8_t *pWriteData, /* pointer to data to write */
						 size_t writeLength,        /* number of bytes to write */
						 uint8_t *pReadData,        /* pointer to data to write */
						 size_t readLength          /* number of bytes to write */
						 );

/*
Summary:
This function is used to perform write and read as an atomic operation, with no sub address
*/
BERR_Code BREG_I2C_WriteReadNoAddr(
						 BREG_I2C_Handle i2cHandle, /* I2C Handle */
						 uint16_t chipAddr,         /* 7 or 10 bit chip address (_unshifted_) */
						 const uint8_t *pWriteData, /* pointer to data to write */
						 size_t writeLength,        /* number of bytes to write */
						 uint8_t *pReadData,        /* pointer to data to write */
						 size_t readLength          /* number of bytes to write */
						 );

#ifdef __cplusplus
}
#endif
 
#endif
/* End of File */
