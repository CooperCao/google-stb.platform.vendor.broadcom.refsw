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

#include "bstd.h"
#include "breg_spi_priv.h"
#include "bspi.h"
#include "bspi_priv.h"
#include "bkni_multi.h"
#include <Windows.h>
#include <stdio.h>

#pragma comment(lib,"FTD2XX.LIB")
#include "ftd2xx.h"

BDBG_MODULE(bspi);

#define DEV_MAGIC_ID            ((BERR_SPI_ID<<16) | 0xFACE)

#define BSPI_CHK_RETCODE( rc, func )        \
do {                                        \
    if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
    {                                       \
        goto done;                          \
    }                                       \
} while(0)

#define	SPI_BAUD_RATE						6000000

#define BSPI_P_ACQUIRE_UPG_MUTEX(handle) BKNI_AcquireMutex((handle)->hUpgMutex)
#define BSPI_P_RELEASE_UPG_MUTEX(handle) BKNI_ReleaseMutex((handle)->hUpgMutex)

static void BSPI_P_EnableInt_isr ( BSPI_ChannelHandle  hChn );
static void BSPI_P_DisableInt_isr( BSPI_ChannelHandle  hChn );

#define SET_DATA_BITS_LOW_BYTE				0x80
#define SET_DATA_BITS_HIGH_BYTE				0x82
#define SET_CLK_DIVISOR						0x86
#define DISABLE_CLK_DIVIDE_BY_5				0x8A
#define DISABLE_3_PHASE_CLOCK				0x8D
#define DISABLE_ADAPTIVE_CLOCKING			0x97
#define	SET_CHIP_MPSSE_RESET				0x00
#define SET_CHIP_MPSSE_MODE					0x02

#define CLK_DATA_MSB_TX_FALLING_EDGE_NO_RX			0x11
#define CLK_DATA_MSB_TX_FALLING_EDGE_RX_RISING_EDGE	0x31

#define SPI_START_CMD_LENGTH					9
#define SPI_STOP_CMD_LENGTH						9
#define SPI_SEND_BUF_SIZE						64

#define BCM9SERIAL_ADP_SPI_CHNL					0x01

#define CALC_READ_BYTE(x)  (x - 1)
#define CALC_WRITE_BYTE(x) (x - 1)

BYTE cmdBeginBuffer[SPI_START_CMD_LENGTH];
BYTE cmdEndBuffer[SPI_STOP_CMD_LENGTH];
/*******************************************************************************
*
*   Private Module Handles
*
*******************************************************************************/

typedef struct BSPI_P_Handle
{
    uint32_t        magicId;                    /* Used to check if structure is corrupt */
    BCHP_Handle     hChip;
    BREG_Handle     hRegister;
    BINT_Handle     hInterrupt;
    unsigned int    maxChnNo;
    BSPI_ChannelHandle hSpiChn[MAX_SPI_CHANNELS + HIF_MSPI_MAX_CHANNELS];
    BKNI_MutexHandle hUpgMutex;                    /* UPG spi mutex handle for serialization */
	FT_HANDLE		ft_handle;

} BSPI_P_Handle;

typedef struct BSPI_P_ChannelHandle
{
    uint32_t            magicId;                    /* Used to check if structure is corrupt */
    BSPI_Handle         hSpi;
    unsigned int        chnNo;
    uint32_t            coreOffset;
    BKNI_EventHandle    hChnEvent;
    BINT_CallbackHandle hChnCallback;
    BSPI_Pcs            pcs;
    uint32_t            baud;
    uint8_t             clkConfig;
    bool                intMode;
    uint8_t             bitsPerTxfr;
    bool                lastByteContinueEnable;     /* Last Byte Contiue Enable Flag */
    bool                useUserDtlAndDsclk;         /* Use User specified DTL and DSCLK */
    BSPI_AssertSSFunc   assertSSFunc;               /* function to assert SS */
    BSPI_DeassertSSFunc deassertSSFunc;             /* function to deassert SS */
    BSPI_EbiCs          ebiCs;                      /* EBI CS line to use */
} BSPI_P_ChannelHandle;



/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/
static const BSPI_Settings defSpiSettings = NULL;

static const BSPI_ChannelSettings defSpiChnSettings =
{
    SPI_BAUD_RATE,
    0,
    false,
    8,
    false,
    false,
    BSPI_SpiCore_Upg,
    BSPI_EbiCs_unused
};

BERR_Code ft_error(FT_STATUS ftstatus)
{
    switch(ftstatus)
    {
    case FT_OK:
        return BERR_SUCCESS;
        break;
    default:
        return BERR_NOT_INITIALIZED;
    }
}

/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/
BERR_Code BSPI_Open(
    BSPI_Handle *pSpi,                  /* [output] Returns handle */
    BCHP_Handle hChip,                  /* Chip handle */
    BREG_Handle hRegister,              /* Register handle */
    BINT_Handle hInterrupt,             /* Interrupt handle */
    const BSPI_Settings *pDefSettings   /* Default settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BSPI_Handle hDev;
    unsigned int chnIdx;
    FT_STATUS ftStatus;

    /* Sanity check on the handles we've been given. */
    BSTD_UNUSED( hChip );
    BSTD_UNUSED( hRegister );
    BSTD_UNUSED( hInterrupt );
    BSTD_UNUSED( pDefSettings );

    if(SPI_BAUD_RATE == 0)
    {
        retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ERR(("BSPI_Open: SPI_BAUD_RATE cannot be 0\n"));
        goto done;
    }

    /* Alloc memory from the system heap */
    hDev = (BSPI_Handle) BKNI_Malloc( sizeof( BSPI_P_Handle ) );
    if( hDev == NULL )
    {
        *pSpi = NULL;
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BSPI_Open: BKNI_malloc() failed\n"));
        goto done;
    }

    hDev->magicId    =   DEV_MAGIC_ID;
    hDev->maxChnNo   =   MAX_SPI_CHANNELS;
    for( chnIdx = 0; chnIdx < hDev->maxChnNo; chnIdx++ )
    {
        hDev->hSpiChn[chnIdx] = NULL;
    }

    ftStatus = FT_Open(BCM9SERIAL_ADP_SPI_CHNL,&(hDev->ft_handle));
    if (ftStatus != FT_OK)
    {
        retCode = ft_error(ftStatus);
        BDBG_ERR(("BSPI_Open: FT_Open() failed\n"));
        goto done;
    }

    *pSpi = hDev;

done:
    if(retCode){
        BKNI_Free( hDev );
        hDev = NULL;
    }
    return( retCode );
}


BERR_Code BSPI_Close(
    BSPI_Handle hDev                    /* Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    FT_Close(hDev->ft_handle);

    BKNI_Free( (void *) hDev );

    return( retCode );
}


BERR_Code BSPI_GetDefaultSettings(
    BSPI_Settings *pDefSettings,        /* [output] Returns default setting */
    BCHP_Handle hChip                   /* Chip handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BSTD_UNUSED(hChip);

    *pDefSettings = defSpiSettings;

    return( retCode );
}


BERR_Code BSPI_GetTotalChannels(
    BSPI_Handle hDev,                   /* Device handle */
    unsigned int *totalChannels         /* [output] Returns total number downstream channels supported */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    *totalChannels = hDev->maxChnNo;

    return( retCode );
}


BERR_Code BSPI_GetTotalUpgSpiChannels(
    BSPI_Handle hDev,                   /* Device handle */
    unsigned int *totalUpgChannels      /* [output] Returns total number of UPG SPI hannels supported */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    *totalUpgChannels = MAX_SPI_CHANNELS;

    return( retCode );
}

BERR_Code BSPI_GetTotalHifSpiChannels(
    BSPI_Handle hDev,                   /* Device handle */
    unsigned int *totalHifChannels      /* [output] Returns total number of HIF SPI hannels supported */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    *totalHifChannels = HIF_MSPI_MAX_CHANNELS;

    return( retCode );
}

BERR_Code BSPI_GetChannelDefaultSettings(
    BSPI_Handle hDev,                   /* Device handle */
    unsigned int channelNo,             /* Channel number to default setting for */
    BSPI_ChannelSettings *pChnDefSettings /* [output] Returns channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

#if !BDBG_DEBUG_BUILD
    BSTD_UNUSED(hDev);
#endif

    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    *pChnDefSettings = defSpiChnSettings;

    return( retCode );
}

BERR_Code BSPI_GetChannelDefaultSettings_Ext(
    BSPI_Handle         hDev,                   /* Device handle */
    BSPI_ChannelInfo    channel,                /* Channel number info */
    BSPI_ChannelSettings *pChnDefSettings       /* [output] Returns channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

#if !BDBG_DEBUG_BUILD
    BSTD_UNUSED(hDev);
#endif

    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    return( BERR_NOT_SUPPORTED );
}

BERR_Code BSPI_OpenChannel(
    BSPI_Handle hDev,                   /* Device handle */
    BSPI_ChannelHandle *phChn,          /* [output] Returns channel handle */
    unsigned int channelNo,             /* Channel number to open */
    const BSPI_ChannelSettings *pChnDefSettings /* Channel default setting */
    )
{
    FT_STATUS	ftStatus;
    BERR_Code	retCode = BERR_SUCCESS;
    /* Buffer to hold MPSSE commands to be sent to the FT2232H */
    BYTE byOutputBuffer[SPI_SEND_BUF_SIZE];
    /* Index to the output buffer */
    DWORD dwNumBytesToSend = 0;
    /* Count of actual bytes sent - used with FT_Write */
    DWORD dwNumBytesSent   = 0;
    DWORD cmdIndex = 0;
    /* SPI Clock */
    DWORD dwClockDivisor   = 0;
    BSPI_ChannelHandle		 hChnDev=NULL;

    /* Allocate memory from the system heap */
    hChnDev = (BSPI_ChannelHandle) BKNI_Malloc( sizeof( BSPI_P_ChannelHandle ) );
    if( hChnDev == NULL )
    {
        *phChn = NULL;
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BSPI_OpenChannel: BKNI_malloc() failed\n"));
        goto done;
    }

    hChnDev->magicId    = DEV_MAGIC_ID;
    hChnDev->hSpi       = hDev;

    /* Reset USB device */
    ftStatus = FT_ResetDevice(hDev->ft_handle);
    /* Purge USB receive and Transmit buffer */
    ftStatus |= FT_Purge(hDev->ft_handle, FT_PURGE_RX|FT_PURGE_TX);
    /* Set USB request transfer sizes to 64K */
    ftStatus |= FT_SetUSBParameters(hDev->ft_handle, 65536, 65535);
    /* Disable event and error characters */
    ftStatus |= FT_SetChars(hDev->ft_handle, false, 0, false, 0);
    /* Sets the read and write timeouts in milliseconds */
    ftStatus |= FT_SetTimeouts(hDev->ft_handle, 0, 5000);
    /* Set the latency timer to 1mS (default is 16mS) */
    ftStatus |= FT_SetLatencyTimer(hDev->ft_handle, 1);
    /* Turn on flow control to synchronize IN requests */
    ftStatus |= FT_SetFlowControl(hDev->ft_handle, FT_FLOW_RTS_CTS, 0x00, 0x00);
    /* Reset controller */
    ftStatus |= FT_SetBitMode(hDev->ft_handle, 0x0, SET_CHIP_MPSSE_RESET);
    /* Enable MPSSE mode */
    ftStatus |= FT_SetBitMode(hDev->ft_handle, 0x0, SET_CHIP_MPSSE_MODE);
    if (ftStatus != FT_OK)
    {
        retCode = ft_error(ftStatus);
        BDBG_ERR(("BSPI_OpenChannel : FT_SetBitMode %d\n", ftStatus));
        goto done;
    }

    /* Reset output buffer index */
    dwNumBytesToSend = 0;
    /* Set up the Hi-Speed specific commands for the FTx232H */
    byOutputBuffer[dwNumBytesToSend++] = DISABLE_CLK_DIVIDE_BY_5;
    byOutputBuffer[dwNumBytesToSend++] = DISABLE_ADAPTIVE_CLOCKING;
    byOutputBuffer[dwNumBytesToSend++] = DISABLE_3_PHASE_CLOCK;
    /* Send off the HS-specific commands */
    ftStatus = FT_Write(hDev->ft_handle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
    if (ftStatus != FT_OK)
    {
        retCode = ft_error(ftStatus);
        BDBG_ERR(("BSPI_OpenChannel : FT_Write(1) %d\n", ftStatus));
        goto done; // Exit with error
    }

    /* Reset output buffer index */
    dwNumBytesToSend = 0;
    /* TCK = 60MHz /((1 + [(1 +0xValueH*256) OR 0xValueL])*2)	*/
    dwClockDivisor = (60000000/(SPI_BAUD_RATE *2))-1;
    /* Command to set clock divisor	*/
    byOutputBuffer[dwNumBytesToSend++] = SET_CLK_DIVISOR;
    /* Set low-value of clock divisor */
    byOutputBuffer[dwNumBytesToSend++] = dwClockDivisor & 0xFF;
    /* Set high-value of clock divisor */
    byOutputBuffer[dwNumBytesToSend++] = (dwClockDivisor >> 8) & 0xFF;
    /* Send off the clock divisor commands */
    ftStatus = FT_Write(hDev->ft_handle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
    if (ftStatus != FT_OK)
    {
        retCode = ft_error(ftStatus);
        BDBG_ERR(("BSPI_OpenChannel : FT_Write(1) %d\n", ftStatus));
        goto done;
    }

    dwNumBytesToSend = 0;
    /* Set initial states -  both pin directions and output values */
    /* Pin name Signal Direction Config Initial State Config */
    /* ADBUS0 SCLK   output 1 high 1 */
    /* ADBUS1 MOSI   output 1 low  1 */
    /* ADBUS2 MISO	 input  0      0 */
    /* ADBUS3 CS	 output 1 high 1 */
    /* ADBUS4 GPIOL0 output 1 high 1 */
    /* ADBUS5 GPIOL1 output 1 high 1 */
    /* ADBUS6 GPIOL2 output 1 high 1 */
    /* ADBUS7 GPIOL3 output 1 high 1 */
    /* Configure data bits  MPSSE port */
    byOutputBuffer[dwNumBytesToSend++] = SET_DATA_BITS_LOW_BYTE;
    /* output state config */
    byOutputBuffer[dwNumBytesToSend++] = 0xFB;
    /* Direction config		*/
    byOutputBuffer[dwNumBytesToSend++] = 0xFB;
    /* Send off the GPIO config commands */
    ftStatus = FT_Write(hDev->ft_handle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
    if (ftStatus != FT_OK)
    {
        retCode = ft_error(ftStatus);
        BDBG_ERR(("BSPI_OpenChannel : FT_Write(1) %d\n", ftStatus));
        goto done;
    }

    *phChn = hChnDev;

    /* setup the SPI start command  bytes */
    cmdIndex = 0;
    cmdBeginBuffer[cmdIndex++] = SET_DATA_BITS_LOW_BYTE;
    /* CS low, MISO low, MOSI high, CLK high */
    cmdBeginBuffer[cmdIndex++] = 0xF3;
    cmdBeginBuffer[cmdIndex++] = 0xFB;

    cmdBeginBuffer[cmdIndex++] = SET_DATA_BITS_LOW_BYTE;
    /* CS low, MISO low, MOSI high, CLK high */
    cmdBeginBuffer[cmdIndex++] = 0xF3;
    cmdBeginBuffer[cmdIndex++] = 0xFB;

    cmdBeginBuffer[cmdIndex++] = SET_DATA_BITS_LOW_BYTE;
    /* CS low, MISO low, MOSI high, CLK low */
    cmdBeginBuffer[cmdIndex++] = 0xF2;
    cmdBeginBuffer[cmdIndex++] = 0xFB;


    /* setup the SPI Stop command  bytes */
    cmdIndex = 0;
    cmdEndBuffer[cmdIndex++] = SET_DATA_BITS_LOW_BYTE;
    /* CS low, MISO low, MOSI high, CLK low */
    cmdEndBuffer[cmdIndex++] = 0xF2;
    cmdEndBuffer[cmdIndex++] = 0xFB;

    cmdEndBuffer[cmdIndex++] = SET_DATA_BITS_LOW_BYTE;
    /* CS high, MISO low, MOSI high, CLK low */
    cmdEndBuffer[cmdIndex++] = 0xFA;
    cmdEndBuffer[cmdIndex++] = 0xFB;

    cmdEndBuffer[cmdIndex++] = SET_DATA_BITS_LOW_BYTE;
    /* CS high, MISO low, MOSI high, CLK high */
    cmdEndBuffer[cmdIndex++] = 0xFB;
    cmdEndBuffer[cmdIndex++] = 0xFB;

done:
    return retCode;

}

BERR_Code BSPI_CloseChannel(
    BSPI_ChannelHandle hChn         /* Device channel handle */
    )
{
   return BERR_NOT_SUPPORTED;
}

BERR_Code BSPI_GetDevice(
    BSPI_ChannelHandle hChn,            /* Device channel handle */
    BSPI_Handle *phDev                  /* [output] Returns Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    *phDev = hChn->hSpi;

    return( retCode );
}


BERR_Code BSPI_CreateSpiRegHandle(
    BSPI_ChannelHandle hChn,            /* Device channel handle */
    BREG_SPI_Handle *pSpiReg            /* [output]  */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    *pSpiReg = (BREG_SPI_Handle)BKNI_Malloc( sizeof(BREG_SPI_Impl) );
    if( *pSpiReg == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BSPI_CreateSpiRegHandle: BKNI_malloc() failed\n"));
        goto done;
    }

    (*pSpiReg)->context                 = (void *)hChn;
    (*pSpiReg)->BREG_SPI_Write_All_Func = BSPI_P_WriteAll;
    (*pSpiReg)->BREG_SPI_Write_Func     = BSPI_P_Write;
    (*pSpiReg)->BREG_SPI_Read_Func      = BSPI_P_Read;

done:
    return( retCode );
}

BERR_Code BSPI_CloseSpiRegHandle(
    BREG_SPI_Handle     hSpiReg         /* SPI register handle */
    )
{

    BDBG_ASSERT( hSpiReg );
    BKNI_Free( (void *) hSpiReg );

    return BERR_SUCCESS ;
}

BERR_Code BSPI_AbortTxfr(
    BSPI_ChannelHandle hChn         /* Device channel handle */
    )
{
   return BERR_NOT_SUPPORTED;
}

BERR_Code BSPI_GetClkConfig(
    BSPI_ChannelHandle hChn,                /* Device channel handle */
    uint8_t *pClkConfig                     /* Pointer to clock config */
    )
{
   return BERR_NOT_SUPPORTED;
}

BERR_Code BSPI_SetClkConfig(
    BSPI_ChannelHandle hChn,            /* Device channel handle */
    uint8_t clkConfig                   /* clock config */
    )
{
   return BERR_NOT_SUPPORTED;
}

BERR_Code BSPI_GetDTLConfig (
    BSPI_ChannelHandle hChn,    /* Device channel handle */
    uint32_t *pDTLConfig        /* pointer to DTLConfig */
    )
{
   return BERR_NOT_SUPPORTED;
}

BERR_Code BSPI_SetDTLConfig (
    BSPI_ChannelHandle context,             /* Device channel handle */
    const uint32_t data         /* data to write */
    )
{
   return BERR_NOT_SUPPORTED;
}


BERR_Code BSPI_GetRDSCLKConfig (
    BSPI_ChannelHandle hChn,    /* Device channel handle */
    uint32_t *pRDSCLKConfig     /* pointer to RDSCLKConfig */
    )
{
   return BERR_NOT_SUPPORTED;
}

BERR_Code BSPI_SetRDSCLKConfig(
    BSPI_ChannelHandle context,             /* Device channel handle */
    const uint32_t data         /* data to write */
    )
{
   return BERR_NOT_SUPPORTED;
}

void BSPI_RegisterSSFunctions (
    BSPI_ChannelHandle hChn,                /* Device channel handle */
    BSPI_AssertSSFunc assertFunc,           /* Assert SS function */
    BSPI_DeassertSSFunc deassertFunc        /* Deassert SS function */
    )
{
}


void BSPI_SetLastByteContinueEnable(
    BSPI_ChannelHandle hChn,
    bool bEnable
)
{
}


/*******************************************************************************
*
*   Private Module Functions
*
*******************************************************************************/
BERR_Code BSPI_P_Read
(
    void *context,					/* Device channel handle */
    const uint8_t *pWriteData,      /* pointer to write data  */
    uint8_t *pReadData,				/* pointer to memory location to store read data  */
    size_t length					/* number of bytes to read + number of bytes to write */
)
{
    BSPI_Handle         hDev;
    BSPI_ChannelHandle  hChn;

    FT_STATUS	ftStatus;
    BERR_Code retCode = BERR_SUCCESS;
    /* Number of bytes available to read in the driver's input buffer */
    DWORD dwNumBytesToRead = 0;
    /* Count of actual bytes read - used with FT_Read */
    DWORD dwNumBytesRead = 0;
    /* Buffer to hold MPSSE commands and data to be sent to the FT2232H */
    BYTE byOutputBuffer[SPI_SEND_BUF_SIZE];
    /* Index to the output buffer */
    DWORD dwNumBytesToSend = 0;
    /* Count of actual bytes sent - used with FT_Write */
    DWORD dwNumBytesSent = 0;

    hChn = (BSPI_ChannelHandle)context;
    hDev = hChn->hSpi;

    if((SPI_START_CMD_LENGTH + 3 + length + SPI_STOP_CMD_LENGTH) > SPI_SEND_BUF_SIZE)
    {
        retCode = BERR_INVALID_PARAMETER;
        BDBG_ERR(("BSPI_P_Read : length exceeds send buffer size\n"));
        goto done;
    }

    memcpy(byOutputBuffer,cmdBeginBuffer,SPI_START_CMD_LENGTH);
    dwNumBytesToSend += SPI_START_CMD_LENGTH;

    byOutputBuffer[dwNumBytesToSend++] =  CLK_DATA_MSB_TX_FALLING_EDGE_RX_RISING_EDGE;

    byOutputBuffer[dwNumBytesToSend++] =  CALC_READ_BYTE(length) &  0xFF ;		  // Length L
    byOutputBuffer[dwNumBytesToSend++] =  (CALC_READ_BYTE(length) >> 8 ) & 0xFF;  // Length H

    memcpy(&byOutputBuffer[dwNumBytesToSend],pWriteData,length);
    dwNumBytesToSend += length;

    memcpy(&byOutputBuffer[dwNumBytesToSend],cmdEndBuffer,SPI_STOP_CMD_LENGTH);
    dwNumBytesToSend += SPI_STOP_CMD_LENGTH;

    /* Send off the command */
    ftStatus = FT_Write(hDev->ft_handle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
    if (ftStatus != FT_OK || dwNumBytesToSend != dwNumBytesSent)
    {
        retCode = ft_error(ftStatus);
        BDBG_ERR(("BSPI_P_Read : FT_Write Error  %d\n", ftStatus));
        goto done;
    }

    /* Wait for data to be transmitted and status to be returned by the device driver */
    Sleep(50);

    ftStatus = FT_GetQueueStatus(hDev->ft_handle, &dwNumBytesToRead);
    if (ftStatus != FT_OK || (dwNumBytesToRead == 0) || (dwNumBytesToRead > length))
    {
        retCode = ft_error(ftStatus);
        BDBG_ERR(("BSPI_P_Read : FT_GetQueueStatus Error  %d\n", ftStatus));
        goto done;
    }

    ftStatus = FT_Read(hDev->ft_handle, pReadData, dwNumBytesToRead, &dwNumBytesRead);
    if (ftStatus != FT_OK || (dwNumBytesRead == 0))
    {
        retCode = ft_error(ftStatus);
        BDBG_ERR(("BSPI_P_Read : FT_Read Error  %d\n", ftStatus));
        goto done;
    }

done:
    return retCode;

}


BERR_Code BSPI_P_WriteAll
(
    void *context,          /* Device channel handle */
    uint16_t chipAddr,      /* Adress */
    uint8_t subAddr,        /* Sub address */
    const uint8_t *pData,   /* pointer to data to write */
    size_t length           /* number of bytes to write */
)
{
   return BERR_NOT_SUPPORTED;
}


BERR_Code BSPI_P_Write
(
    void *context,              /* Device channel handle */
    const uint8_t *pData,       /* pointer to data to write */
    size_t length               /* number of bytes to write */
)
{
    FT_STATUS	ftStatus;
    BERR_Code retCode = BERR_SUCCESS;
    /* Number of bytes available to read in the driver's input buffer */
    DWORD dwNumBytesToRead = 0;
    /* Count of actual bytes read - used with FT_Read */
    DWORD dwNumBytesRead = 0;
    /* Buffer to hold MPSSE commands and data to be sent to the FT2232H */
    BYTE byOutputBuffer[SPI_SEND_BUF_SIZE];
    /* Index to the output buffer */
    DWORD dwNumBytesToSend = 0;
    /* Count of actual bytes sent - used with FT_Write */
    DWORD dwNumBytesSent = 0;
    DWORD bytesToSend = 0;
    DWORD totalBytesSent = 0;
    DWORD totalBytesLeft = 0;

    BSPI_Handle         hDev;
    BSPI_ChannelHandle  hChn;

    hChn = (BSPI_ChannelHandle)context;
    hDev = hChn->hSpi;

    /* send SPI start command */
    ftStatus = FT_Write(hDev->ft_handle, cmdBeginBuffer, SPI_START_CMD_LENGTH, &dwNumBytesSent);
    if (ftStatus != FT_OK || (SPI_START_CMD_LENGTH != dwNumBytesSent))
    {
        retCode = ft_error(ftStatus);
        BDBG_ERR(("BSPI_P_Write 1 : Error  %d\n", ftStatus));
        goto done;
    }

    totalBytesLeft = length;
    do
    {
        /* 3 bytes in the output buffer is reserved for the command and length */
        bytesToSend = (totalBytesLeft > (SPI_SEND_BUF_SIZE - 3) ? (SPI_SEND_BUF_SIZE - 3):totalBytesLeft);

        dwNumBytesToSend = 0;

        byOutputBuffer[dwNumBytesToSend++] =  CLK_DATA_MSB_TX_FALLING_EDGE_NO_RX;
        byOutputBuffer[dwNumBytesToSend++] =  CALC_WRITE_BYTE(bytesToSend) &  0xFF ;		 // Length L
        byOutputBuffer[dwNumBytesToSend++] = (CALC_WRITE_BYTE(bytesToSend) >> 8 ) & 0xFF;    // Length H

        memcpy(&byOutputBuffer[dwNumBytesToSend],&pData[totalBytesSent],bytesToSend);
        dwNumBytesToSend += bytesToSend;

        ftStatus = FT_Write(hDev->ft_handle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
        if (ftStatus != FT_OK || (dwNumBytesToSend != dwNumBytesSent))
        {
            retCode = ft_error(ftStatus);
            /* de-assert the SPI select line */
            FT_Write(hDev->ft_handle, cmdEndBuffer, SPI_STOP_CMD_LENGTH, &dwNumBytesSent);
            BDBG_ERR(("BSPI_P_Write : Error  %d\n", ftStatus));
            goto done;
        }

        totalBytesSent += bytesToSend;
        totalBytesLeft = length - totalBytesSent;

    }while(totalBytesLeft > 0);

    /* send SPI stop command */
    ftStatus = FT_Write(hDev->ft_handle, cmdEndBuffer, SPI_STOP_CMD_LENGTH, &dwNumBytesSent);
    if (ftStatus != FT_OK || (SPI_STOP_CMD_LENGTH != dwNumBytesSent ))
    {
        retCode = ft_error(ftStatus);
        BDBG_ERR(("BSPI_P_Write : Error  %d\n", ftStatus));
    }
    Sleep(20);

done:
    return retCode;

}
