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
    This function writes data of size length, where length is generally greater than MAX_SPI_XFER, in an iterative fashion.

Description:
    This function writes data of size length, where length is generally greater than MAX_SPI_XFER, in an iterative fashion.

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
    This function iteratively writes data pointed to by multiple buffers.

Description:
    This function iteratively writes data pointed to by multiple buffers.

Returns:
    TODO:

See Also:
    BSPI_P_Write, BSPI_P_WriteAll
****************************************************************************/
BERR_Code BSPI_P_Multiple_Write(
    void *context,                   /* Device channel handle */
    const BREG_SPI_Data *pWriteData, /* Pointer to array of BREG_SPI_Data structs */
    size_t count);                   /* BREG_SPI_Data struct count */

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



