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
#ifndef BSPI_PRIV_H__
#define BSPI_PRIV_H__

#include "bspi.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    This function performs a SPI read operation.

Description:
    This function is used to read data from a SPI slave using a particular
    SPI channel.  The slave select for that SPI channel should already have
    been set when opening the channel.

Returns:
    TODO:

See Also:
    BSPI_AbortTxfr

****************************************************************************/
BERR_Code BSPI_P_Read(
    void *context,              /* Device channel handle */
    const uint8_t *pWriteData,  /* pointer to memory location where data is to sent  */
    uint8_t *pReadData,         /* pointer to memory location to store read data  */
    size_t length               /* number of bytes to read + number of bytes to write */
    );

/***************************************************************************
Summary:
    This function performs a SPI read operation.

Description:
    This function is used to read writeLength amount of data from a SPI slave using a particular
    SPI channel.  The slave select for that SPI channel should already have been set when
    opening the channel.
    NOTE: Internally the spi get clocked for writeLength + readLength cycles.

Returns:
    TODO:

See Also:
    BSPI_AbortTxfr

****************************************************************************/
BERR_Code BSPI_P_ReadAll(
    void *context,              /* Device channel handle */
    const uint8_t *pWriteData,  /* pointer to memory location where data is to sent  */
    size_t writeLength,         /* size of *pWriteData buffer */
    uint8_t *pReadData,         /* pointer to memory location to store read data  */
    size_t readLength           /* size of *pReadData buffer */
    );

/***************************************************************************
Summary:
    This function performs a SPI write operation.

Description:
    This function is used to write data to a SPI slave using a particular
    SPI channel.  The slave select for that SPI channel should already have
    been set when opening the channel.

Returns:
    TODO:

See Also:
    BSPI_AbortTxfr

****************************************************************************/
BERR_Code BSPI_P_Write(
    void *context,              /* Device channel handle */
    const uint8_t *pWriteData,  /* pointer to memory location where data is to sent  */
    size_t length               /* number of bytes to write */
    );

/***************************************************************************
Summary:
    This function performs a complete data SPI write operation.

Description:
    This function writes all the data in an iterative fashion using the
    programmable number of SPI registers.

Returns:
    TODO:

See Also:
    BSPI_P_Write
****************************************************************************/
BERR_Code BSPI_P_WriteAll(
    void *context,                  /* Device channel handle */
    const uint8_t *pWriteData,      /* pointer to memory location where data is to sent  */
    size_t length                   /* number of bytes to write */
    );

/***************************************************************************
Summary:
Description:
Returns:
See Also:
****************************************************************************/
void BSPI_P_SetContinueAfterCommand(
    void *context,              /* Device channel handle */
    bool bEnable
    );

/***************************************************************************
Summary:
    This function sets the clock setting for an SPI channel

Description:
    This function allows the user to change the SPI clock setting.

Returns:
    TODO:

See Also:

****************************************************************************/
void BSPI_P_SetClk (
    BSPI_ChannelHandle channel,         /* Device channel handle */
    uint32_t baud,                      /* serial clock baud setting */
    uint8_t clkConfig                   /* clock polarity/phase setting */
    );

/***************************************************************************
Summary:
    This function sets the EBI CS line to use with HIF MSPI

Description:
    This function allows the user to set the EBI CS line to use with HIF MSPI

Returns:
    TODO:

See Also:

****************************************************************************/
void BSPI_P_SetEbiCs(
    BSPI_ChannelHandle  hChn,           /* Device channel handle */
    BSPI_EbiCs          ebiCs           /* EBI CS to use */
    );

/***************************************************************************
Summary:
    This function waits until a SPI transfer is finished

Description:
    This function is used to wait for SPI to finish.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BSPI_P_WaitForCompletion
(
    BSPI_ChannelHandle  hChn,                           /* Device channel handle */
    uint32_t            numBytes                        /* number of bytes to transfer */
);

/***************************************************************************
Summary:
    This function handles SPI interrupt

Description:
    This function is the SPI ISR.

Returns:
    TODO:

See Also:

****************************************************************************/
void BSPI_P_HandleInterrupt_Isr
(
    void *pParam1,                      /* Device channel handle */
    int parm2                           /* not used */
);

/***************************************************************************
Summary:
    This function gets the total number of bits transferred for every command byte executed.

Returns:
    TODO:

See Also:

****************************************************************************/
void BSPI_P_GetBitsPerTransfer
(
    void *pParam1,                      /* Device channel handle */
    uint8_t *pBitsPerTransfer           /* Number of bits transferred per command byte */
);

#ifdef __cplusplus
}
#endif

#endif



