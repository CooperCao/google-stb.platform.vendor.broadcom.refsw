/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
*   API name: Spi
*    Specific APIs related to SPI Control.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_SPI_H__
#define NEXUS_SPI_H__

#include "nexus_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NEXUS_Spi *NEXUS_SpiHandle;

/***************************************************************************
Summary:
Settings structure for SPI.
****************************************************************************/
typedef struct NEXUS_SpiSettings
{
    /* the following can only be set at Open time */
    uint32_t      baud;                       /* SPI baud rate */
    uint8_t       bitsPerTransfer;            /* number of bits per transfer */
    bool          lastByteContinueEnable;     /* Deprecated. */
    bool          useUserDtlAndDsclk;         /* Use User specified DTL and DSCLK */
    bool          interruptMode;              /* If true (default), transfers will wait for interrupt, if false transfers will poll for completion */
    unsigned      ebiChipSelect;              /* EBI chip select line to use as slave select */ 

    /* the following can be set at Open or SetSettings */
    bool          clockActiveLow;             /* If true, the SCK will be active low. */
    bool          txLeadingCapFalling;        /* If true, data will be transmitted on the leading edge and captured on the falling edge */
    uint8_t       dtl;                        /* DTL field - length of delay after transfer. See RDB documentation for usage. */
    uint8_t       rdsclk;                     /* RDSCLK field - the length of delay from PCS valid to SCK transition. See RDB documentation for usage. */
	unsigned      delayAfterTransfer;         /* in units of nano seconds. A value of 0 (default) uses 'dtl', but may be limited in range. If non-zero, ignore dtl. */
	unsigned      delayChipSelectToClock;     /* in units of nano seconds. A value of 0 (default) uses 'rdsclk', but may be limited in range. If non-zero, ignore rdsclk. */
} NEXUS_SpiSettings;

/***************************************************************************
Summary:
Get default settings for a SPI channel.
****************************************************************************/
void NEXUS_Spi_GetDefaultSettings(
    NEXUS_SpiSettings *pSettings    /* [out] Default Settings */
    );

/**
deprecated
**/
typedef enum NEXUS_SpiCore
{
    NEXUS_SpiCore_eUpg = 0,                  /* UPG SPI core */
    NEXUS_SpiCore_eMax 
} NEXUS_SpiCore;
#define NEXUS_SPI_INDEX(TYPE,INDEX) (NEXUS_SpiCore_e##TYPE<<16|(INDEX))

/***************************************************************************
Summary:
Open a SPI channel
****************************************************************************/
NEXUS_SpiHandle NEXUS_Spi_Open(  /* attr{destructor=NEXUS_Spi_Close} */
    unsigned index, /* channel index */
    const NEXUS_SpiSettings *pSettings
    );

/***************************************************************************
Summary:
Close a SPI channel
****************************************************************************/
void NEXUS_Spi_Close(
    NEXUS_SpiHandle handle
    );

/***************************************************************************
Summary:
Set settings for a SPI channel.
****************************************************************************/
NEXUS_Error NEXUS_Spi_SetSettings(
    NEXUS_SpiHandle handle,
    const NEXUS_SpiSettings *pSettings
    );

/***************************************************************************
Summary:
Get settings for a SPI channel.
****************************************************************************/
void NEXUS_Spi_GetSettings(
    NEXUS_SpiHandle handle,
    NEXUS_SpiSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Write out data over SPI.

Description:

To write 100 bytes to register at offset 0xC on a SPI slave device which needs a initial starting byte of 0x81, do the following:

NEXUS_Spi_GetDefaultSettings(&spiSettings);

This holds the slave device active after the first transfer of two bytes.

spiSettings.lastByteContinueEnable = true;
NEXUS_Spi_SetSettings(spiHandle, &spiSettings);

writebuf[0] = 0x81;
writebuf[1] = 0xC;

NEXUS_Spi_Write(spiHandle, writebuf, 2);

Here buf is the member holding 100 bytes of data that needs to be transferred next.
NEXUS_Spi_Write(spiHandle, buf, 100));

This deasserts the slave device.

spiSettings.lastByteContinueEnable = false;
NEXUS_Spi_SetSettings(spiHandle, &spiSettings);
****************************************************************************/
NEXUS_Error NEXUS_Spi_Write(
    NEXUS_SpiHandle handle,
    const uint8_t *pWriteData,      /* attr{nelem=length;reserved=8} pointer to write memory location */
    size_t length                   /* size of data in pWriteData[] in bytes */
    );

/***************************************************************************
Summary:
Read in data from SPI.

Description:
The SPI protocol always writes data out as it is reading data in.
The incoming data is stored in pReadData while the data
going out is sourced from pWriteData.
****************************************************************************/
NEXUS_Error NEXUS_Spi_Read(
    NEXUS_SpiHandle handle,
    const uint8_t *pWriteData,      /* attr{nelem=length;reserved=8} pointer to memory location where data is to sent  */
    uint8_t *pReadData,             /* [out] attr{nelem=length;reserved=8} pointer to memory location to store read data  */
    size_t length                   /* length is the size of pWriteData[] in bytes and is also the size of pReadData[] in bytes. They have the same size. */
    );

/***************************************************************************
Summary:
Read all the requested data continuously from SPI.

Description:
The SPI protocol always writes data out as it is reading data in.
But, NEXUS_Spi_ReadAll handles all this internally and return all the data reuested from the offset.

Example:
If data 0x01, 0x56, 0x3, 0x87 is to be read from offset 0x30. Lets assume a "read sync" byte of 0x99 was sent before the offset.
That is 0x9, 0x30 and written first and 0x01, 0x56, 0x3, 0x87 is read back.

So, NEXUS_Spi_Read() would return in pReadData the following byte stream:  0xdontCare, 0xdontCare, 0x01, 0x56, 0x3, 0x87.
But, NEXUS_Spi_ReadAll would return just the  0x01, 0x56, 0x3, 0x87 in pReadData.

Also, NEXUS_Spi_Read() is limited to read 16, 32, 64 bytes read at a time depending on the hardwared.
But, NEXUS_Spi_ReadAll() can read any amount of data iteratively, provided sufficient buffer is provided.
****************************************************************************/
NEXUS_Error NEXUS_Spi_ReadAll(
    NEXUS_SpiHandle handle,
    const uint8_t *pWriteData,      /* attr{nelem=writeLength;reserved=8} pointer to memory location where data is to sent  */
    size_t writeLength,             /* Total number of bytes to write to slave from pWriteData, in bytes */
    uint8_t *pReadData,             /* [out] attr{nelem=readLength;nelem_out=pActualReadLength;reserved=8} pointer to memory location to store read data  */
    size_t readLength,              /* Total number of bytes to read from slave into pReadData, in bytes */
    size_t *pActualReadLength       /* [out] actual number. will be less than readLength on timeout. */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_SPI_H__ */

