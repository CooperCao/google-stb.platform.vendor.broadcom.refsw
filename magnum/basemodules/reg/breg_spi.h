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
 ******************************************************************************/

/*= Module Overview ********************************************************
This module provides a standard abstraction API for accessing SPI registers.
Several SPI busses may exist on a single platform. Some may be hardware based, 
while others software based (using GPIO pins). In order to hide the 
specific implementation of the SPI bus from a PortingInterface this SPI
register access abstraction is used.

---++++ SPI Register Handles
Hardware and software SPI implementations must follow the defined function 
prototype definitions. All SPI PortingInterfaces supply the a function 
that initializes a standard BREG_SPI_Handle (and fills it with function 
pointers provided by the specific SPI PortingInterfaces implementation).
Please refer to the specific SPI PortingInterface implementations for more
details.

An example of this type of function would be:

<verbatim>
BERR_Code BSPI_CreateSpiRegHandle( BSPI_Handle SpiHandle, BREG_SPI_Handle *pSpiRegHandle );
</verbatim>

This handle should be created during initialization and passed to the 
appropriate porting interfaces.  The porting interfaces in turn pass 
this handle to the abstract SPI functions when then call the appropriate 
SPI implementation though function pointers contained in the BREG_SPI_Handle.

---++++ Handling ThreadSafe with SPI
Since multiple modules (PortingInterface and otherwise) may be sharing the
same SPI bus, any function calls to the modules in question must be serialized
to prevent thread collisions.

    1 All calls to the modules sharing the same SPI handle must be protected
      through the use of a mutex.  This protection must be handled by the
      upper level software (driver, middleware, etc).
    2 Upper level software can create an additional layer between the SPI
      RegisterInterface and the corresponding SPI PortingInterface instance
      that implements the SPI register access.  In this layer a mutex is
      used to protect all SPI accesses.  This could easily be done
      using the BREG_SPI_Impl structure.
***************************************************************************/

#ifndef BREG_SPI_H
#define BREG_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

/*
Summary:
This is an opaque handle that is used for SPI read and write functions.
*/
typedef struct BREG_SPI_Impl *BREG_SPI_Handle;

typedef struct BREG_SPI_Data
{
    const void *data;
    unsigned length;
} BREG_SPI_Data;

/*
Summary:
This function gets the total number of bits transferred for every command byte executed.
*/
void BREG_SPI_GetBitsPerTransfer(
                    BREG_SPI_Handle spiHandle,      /* Device channel handle */
                    uint8_t *pBitsPerTransfer       /* Number of bits transferred per command byte. */
                    );

/*
Summary:
This function writes a programmable number of SPI registers.
*/
BERR_Code BREG_SPI_Write(
                    BREG_SPI_Handle spiHandle,      /* Device channel handle */
                    const uint8_t *pWriteData,      /* pointer to write memory location */
                    size_t length                   /* size of *pWriteData  buffers (number of bytes to write ) */
                    );

/*
Summary:
This function writes data of size length, where length is generally greater than MAX_SPI_XFER, in an iterative fashion.
*/
BERR_Code BREG_SPI_WriteAll(
                    BREG_SPI_Handle spiHandle,      /* Device channel handle */
                    const uint8_t *pWriteData,      /* pointer to write memory location */
                    size_t length                   /* size of *pWriteData  buffers (number of bytes to write ) */
                    );

/*
Summary:
This function iteratively writes data pointed to by multiple buffers.

Example:
If you need to write buf0 and buf1 of lengths len1 and len2, do the following:

BREG_SPI_Data writeData[2];

writeData[0].data = buf1;
writeData[0].length = len1;
writeData[1].data = buf2;
writeData[1].length = len2;

BREG_SPI_Multiple_Write(spiHandle, &writeData, 2)
*/

BERR_Code BREG_SPI_Multiple_Write(
                    BREG_SPI_Handle spiHandle,       /* Device channel handle */
                    const BREG_SPI_Data *pWriteData, /* Pointer to array of BREG_SPI_Data structs */
                    size_t count                     /* BREG_SPI_Data struct count */
                    );

/*
Summary:
This function reads a programmable number of SPI registers.

The SPI protocol always writes data out as it is reading data in.
The incoming data is stored in pReadData while the data
going out is sourced from pWriteData.
*/
BERR_Code BREG_SPI_Read(
                    BREG_SPI_Handle spiHandle,      /* Device channel handle */
                    const uint8_t *pWriteData,      /* pointer to memory location where data is to sent  */
                    uint8_t *pReadData,     /* pointer to memory location to store read data  */
                    size_t length                   /* size of *pWriteData and *pReadData buffers (number of bytes to read + number of bytes to write) */
                    );

/*
Summary:
This function reads readLength amound of data from the SPI slave.

The SPI protocol always writes data out as it is reading data in.
The incoming data is stored in pReadData while the data
going out is sourced from pWriteData.
NOTE: Internally the spi get clocked for writeLength + readLength cycles.
*/
BERR_Code BREG_SPI_ReadAll(
                    BREG_SPI_Handle spiHandle,      /* Device channel handle */
                    const uint8_t *pWriteData,      /* pointer to memory location where data is to sent  */
                    size_t writeLength,             /* size of *pWriteData buffer */
                    uint8_t *pReadData,             /* pointer to memory location to store read data  */
                    size_t readLength               /* size of *pReadData buffer */
                    );

/*
Summary:
This function writes a programmable number of SPI registers in isr.
*/
BERR_Code BREG_SPI_Write_isr(
                    BREG_SPI_Handle spiHandle,      /* Device channel handle */
                    const uint8_t *pWriteData,      /* pointer to write memory location */
                    size_t length                   /* size of *pWriteData  buffers (number of bytes to write ) */
                    );

/*
Summary:
This function reads a programmable number of SPI registers in isr.

The SPI protocol always writes data out as it is reading data in.
The incoming data is stored in pReadData while the data
going out is sourced from pWriteData.
*/
BERR_Code BREG_SPI_Read_isr(
                    BREG_SPI_Handle spiHandle,      /* Device channel handle */
                    const uint8_t *pWriteData,      /* pointer to memory location where data is to sent  */
                    uint8_t *pReadData,     /* pointer to memory location to store read data  */
                    size_t length                   /* size of *pWriteData and *pReadData buffers (number of bytes to read + number of bytes to write) */
                    );


#ifdef __cplusplus
}
#endif
 
#endif
/* End of File */



