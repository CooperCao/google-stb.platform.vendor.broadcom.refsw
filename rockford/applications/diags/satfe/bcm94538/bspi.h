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

/*= Module Overview *********************************************************
<verbatim>

Overview
The SPI PI module controls the SPI master core within the BCM7038.
The SPI master core supports several slave devices as selected by the
slave select signals.  For the BCM7038, the SPI core supports 2 slave
selects.  The SPI clock is configurable upon channel opening.  It can
be set from 53 Khz to 1.6875 Mhz.  By default, the SPI PI will set the
baud rate to the maximum value, 1.6875 Mhz.

Design
The design for BSPI PI API is broken into two parts.

o Part 1 (open/close/configuration):

  These APIs are used for opening and closing BSPI device/device channel.
  When a device/device channel is opened, the device/device channel can be
  configured for transfer speed.

o Part 2 (command):

  These APIs are sending read and write commands using the SPI master controller.

Usage
The usage of BSPI involves the following:

* Configure/Open of BSPI

  * Configure BSPI device for the target system
  * Open BSPI device
  * Configure BSPI device channel for the target system
  * Open BSPI device channel
  * Create BSPI register handle

* Sending commands

  * Send read/write commands using BSPI register handle.

Sample Code
void main( void )
{
    BSPI_Handle         hSpi;
    BSPI_ChannelHandle   hSpiChan;
    BSPI_ChannelSettings defChnSettings;
    BREG_Handle         hReg;
    BCHP_Handle         hChip;
    BINT_Handle         hInt;
    unsigned int        chanNo;
    uint8_t             writeBuf[8];
    uint8_t             readBuf[8];
    BREG_SPI_Handle     hSpiReg;

    // Do other initialization, i.e. for BREG, BCHP, etc.
    BSPI_Open( &hSpi, hChip, hReg, hInt, (BSPI_Settings *)NULL );

    chanNo= 0;  // example for channel 0
    BSPI_GetChannelDefaultSettings( hSpi, chanNo, &defChnSettings );

    // Make any changes required from the default values
    defChnSettings.baud = 1200000;      // set clock to 1.2 Mhz
    defChnSettings.clkConfig = (SPI_SPCR0_MSB_CPOL | SPI_SPCR0_MSB_CPHA);
    defChnSettings.intMode = TRUE;      // enable interrupt

    BSPI_OpenChannel( hSpi, &hSpiChan, chanNo, &defChnSettings );

    BSPI_CreateSpiRegHandle (hSpiChan, &hSpiReg);

    //
    // Do a read of 2 bytes from register 0x04 of SPI device whose
    // chip address is 0x28.
    //
    writeBuf[0] = 0x28;                             // chip address
    writeBuf[1] = 0x04;                             // register offset
    BREG_SPI_Read (hSpiReg, writeBuf, readBuf, 4);   // results in readBuf[2] and readBuf[3]

    printf("Data = %x %x \n", readBuf[2], readBuf[3]);
    //
    // Do a write of 1 byte from register 0x1C of SPI device whose
    // chip ID is 0x28.
    //
   writeBuf[0] = 0x28;
   writeBuf[1] = 0x1C;
   writeBuf[2] = 0x30;               // value to write
   BREG_SPI_Write (hSpiReg,  writeBuf, 3);
}


</verbatim>
***************************************************************************/


#ifndef BSPI_H__
#define BSPI_H__

#include "bchp.h"
#include "breg_mem.h"
#include "breg_spi.h"
#include "bint.h"
#include "bkni.h"
#include "berr_ids.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    Number of SPI channels in BCM7038

Description:

See Also:

****************************************************************************/
#define MAX_SPI_CHANNELS        1
#define HAS_HIF_MSPI            0
#define HIF_MSPI_MAX_CHANNELS   0
#define HAS_UPG_MSPI            0
#define UPG_MSPI_SUPPORTS_16BIT_TRANSFER    0


/***************************************************************************
Summary:
    Error Codes specific to BSPI

Description:

See Also:

****************************************************************************/
#define BSPI_ERR_NOTAVAIL_CHN_NO            BERR_MAKE_CODE(BERR_SPI_ID, 0)

/***************************************************************************
Summary:
    The handles for spi module.

Description:
    Since BSPI is a device channel, it has main device handle as well
    as a device channel handle.

See Also:
    BSPI_Open(), BSPI_OpenChannel()

****************************************************************************/
typedef struct BSPI_P_Handle                *BSPI_Handle;

/***************************************************************************
Summary:
    The handles for spi module.

Description:
    Since BSPI is a device channel, it has main device handle as well
    as a device channel handle.

See Also:
    BSPI_Open(), BSPI_OpenChannel()

****************************************************************************/
typedef struct BSPI_P_ChannelHandle         *BSPI_ChannelHandle;

/***************************************************************************
Summary:
    Enumeration for slave selects

Description:
    This enumeration defines the slave select setting for a SPI channel

See Also:
    None.

****************************************************************************/
typedef enum BSPI_Pcs
{
   BSPI_Pcs_eUpgSpiPcs0 = 0,
   BSPI_Pcs_eUpgSpiPcs1,
   BSPI_Pcs_eUpgSpiPcs2
} BSPI_Pcs;

/***************************************************************************
Summary:
    Required default settings structure for SPI module.

Description:
    The default setting structure defines the default configure of
    SPI when the device is opened.  Since BSPI is a device
    channel, it also has default settings for a device channel.
    Currently there are no parameters for device setting.

See Also:
    BSPI_Open(), BSPI_OpenChannel()

****************************************************************************/
typedef void *BSPI_Settings;

/***************************************************************************
Summary:
    Required default settings structure for SPI module.

Description:
    The default setting structure defines the default configure of
    SPI when the device is opened.  Since BSPI is a device
    channel, it also has default settings for a device channel.
    Currently there are no parameters for device setting.

See Also:
    BSPI_Open(), BSPI_OpenChannel()

****************************************************************************/
typedef enum BSPI_SpiCore
{
    BSPI_SpiCore_Upg = 0,                   /* UPG SPI core */
    BSPI_SpiCore_Hif                        /* HIF SPI core */
} BSPI_SpiCore;

typedef enum BSPI_EbiCs
{
    BSPI_EbiCs_0 = 0,
    BSPI_EbiCs_1,
    BSPI_EbiCs_2,
    BSPI_EbiCs_3,
    BSPI_EbiCs_4,
    BSPI_EbiCs_5,
    BSPI_EbiCs_6,
    BSPI_EbiCs_7,
    BSPI_EbiCs_unused
} BSPI_EbiCs;

typedef struct BSPI_ChannelSettings
{
    uint32_t    baud;                       /* SPI baud rate */
    uint8_t     clkConfig;                  /* SPI clock configuration */
    bool        intMode;                    /* interrupt enable flag */
    uint8_t     bitsPerTxfr;                /* number of bits per transfer */
    bool        lastByteContinueEnable;     /* Last Byte Contiue Enable Flag */
    bool        useUserDtlAndDsclk;         /* Use User specified DTL and DSCLK */
    BSPI_SpiCore spiCore;                   /* SPI core to use */
    BSPI_EbiCs  ebiCs;                      /* EBI CS line to use as slave select */
} BSPI_ChannelSettings;

typedef void (*BSPI_AssertSSFunc)(void);
typedef void (*BSPI_DeassertSSFunc)(void);

typedef struct BSPI_ChannelInfo
{
    BSPI_SpiCore    spiCore;                /* SPI core to use */
    uint32_t        channelNo;              /* Channel number */
} BSPI_ChannelInfo;


/***************************************************************************
Summary:
    This function opens SPI module.

Description:
    This function is responsible for opening BSPI module. When BSPI is
    opened, it will create a module handle and configure the module based
    on the default settings. Once the device is opened, it must be closed
    before it can be opened again.

Returns:
    TODO:

See Also:
    BSPI_Close(), BSPI_OpenChannel(), BSPI_CloseChannel(),
    BSPI_GetDefaultSettings()

****************************************************************************/
BERR_Code BSPI_Open(
    BSPI_Handle *pSPI,                  /* [out] Returns handle */
    BCHP_Handle hChip,                  /* [in] Chip handle */
    BREG_Handle hRegister,              /* [in] Register handle */
    BINT_Handle hInterrupt,             /* [in] Interrupt handle */
    const BSPI_Settings *pDefSettings   /* [in] Default settings */
    );

/***************************************************************************
Summary:
    This function closes SPI module.

Description:
    This function is responsible for closing BSPI module. Closing BSPI
    will free main BSPI handle. It is required that all opened
    BSPI channels must be closed before calling this function. If this
    is not done, the results will be unpredicable.

Returns:
    TODO:

See Also:
    BSPI_Open(), BSPI_CloseChannel()

****************************************************************************/
BERR_Code BSPI_Close(
    BSPI_Handle hDev                    /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function returns the default settings for SPI module.

Description:
    This function is responsible for returns the default setting for
    BSPI module. The returning default setting should be when
    opening the device.

Returns:
    TODO:

See Also:
    BSPI_Open()

****************************************************************************/
BERR_Code BSPI_GetDefaultSettings(
    BSPI_Settings *pDefSettings,        /* [out] Returns default setting */
    BCHP_Handle hChip                   /* [in] Chip handle */
    );

/***************************************************************************
Summary:
    This function returns the total number of channels supported by
    SPI module.

Description:
    This function is responsible for getting total number of channels
    supported by BSPI module, since BSPI device is implemented as a
    device channel.

Returns:
    TODO:

See Also:
    BSPI_OpenChannel(), BSPI_ChannelDefaultSettings()

****************************************************************************/
BERR_Code BSPI_GetTotalChannels(
    BSPI_Handle hDev,                   /* [in] Device handle */
    unsigned int *totalChannels         /* [out] Returns total number downstream channels supported */
    );

/***************************************************************************
Summary:
    This function returns the total number of UPG channels supported by
    SPI module.

Description:
    This function is responsible for getting total number of UPG channels
    supported by BSPI module, since BSPI device is implemented as a
    device channel.

Returns:
    TODO:

See Also:
    BSPI_OpenChannel(), BSPI_ChannelDefaultSettings()

****************************************************************************/
BERR_Code BSPI_GetTotalUpgSpiChannels(
    BSPI_Handle hDev,                   /* Device handle */
    unsigned int *totalUpgChannels      /* [output] Returns total number of UPG SPI hannels supported */
    );

/***************************************************************************
Summary:
    This function returns the total number of HIF channels supported by
    SPI module.

Description:
    This function is responsible for getting total number of HIF channels
    supported by BSPI module, since BSPI device is implemented as a
    device channel.

Returns:
    TODO:

See Also:
    BSPI_OpenChannel(), BSPI_ChannelDefaultSettings()

****************************************************************************/
BERR_Code BSPI_GetTotalHifSpiChannels(
    BSPI_Handle hDev,                   /* Device handle */
    unsigned int *totalHifChannels      /* [output] Returns total number of HIF SPI hannels supported */
    );

/***************************************************************************
Summary:
    This function gets default setting for a SPI module channel.

Description:
    This function is responsible for returning the default setting for
    channel of BSPI. The return default setting is used when opening
    a channel.

Returns:
    TODO:

See Also:
    BSPI_OpenChannel()

****************************************************************************/
BERR_Code BSPI_GetChannelDefaultSettings(
    BSPI_Handle hDev,                   /* [in] Device handle */
    unsigned int channelNo,             /* [in] Channel number to default setting for */
    BSPI_ChannelSettings *pChnDefSettings /* [out] Returns channel default setting */
    );

/***************************************************************************
Summary:
    This function gets default setting for a SPI module channel.

Description:
    This function is responsible for returning the default setting for
    channel of BSPI. The caller sets the BSPI_ChannelInfo input information.

Returns:
    TODO:

See Also:
    BSPI_OpenChannel()

****************************************************************************/
BERR_Code BSPI_GetChannelDefaultSettings_Ext(
    BSPI_Handle         hDev,                   /* Device handle */
    BSPI_ChannelInfo    channel,                /* Channel number info */
    BSPI_ChannelSettings *pChnDefSettings       /* [output] Returns channel default setting */
    );

/***************************************************************************
Summary:
    This function opens SPI module channel.

Description:
    This function is responsible for opening BSPI module channel. When a
    BSPI channel is opened, it will create a module channel handle and
    configure the module based on the channel default settings. Once a
    channel is opened, it must be closed before it can be opened again.

Returns:
    TODO:

See Also:
    BSPI_CloseChannel(), BSPI_GetChannelDefaultSettings()

****************************************************************************/
BERR_Code BSPI_OpenChannel(
    BSPI_Handle hDev,                   /* [in] Device handle */
    BSPI_ChannelHandle *phChn,          /* [out] Returns channel handle */
    unsigned int channelNo,             /* [in] Channel number to open */
    const BSPI_ChannelSettings *pChnDefSettings /* [in] Channel default setting */
    );

/***************************************************************************
Summary:
    This function closes SPI module channel.

Description:
    This function is responsible for closing BSPI module channel. Closing
    BSPI channel it will free BSPI channel handle. It is required that all
    opened BSPI channels must be closed before closing BSPI.

Returns:
    TODO:

See Also:
    BSPI_OpenChannel(), BSPI_CloseChannel()

****************************************************************************/
BERR_Code BSPI_CloseChannel(
    BSPI_ChannelHandle hChn             /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function gets SPI module device handle based on
    the device channel handle.

Description:
    This function is responsible returning BSPI module handle based on the
    BSPI module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BSPI_GetDevice(
    BSPI_ChannelHandle hChn,            /* [in] Device channel handle */
    BSPI_Handle *pSPI                   /* [out] Returns Device handle */
    );


/***************************************************************************
Summary:
    This function creates an SPI register handle.

Description:
    This function is responsible for creating an SPI register handle for
    master SPI controller.  It fills in the BREG_SPI_Handle structure with
    pointers to the SPI PI functions.  The application can then use this
    SPI register interface to perform read and write operations.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BSPI_CreateSpiRegHandle(
    BSPI_ChannelHandle hChn,            /* [in] Device channel handle */
    BREG_SPI_Handle *pSpiRegHandle      /* [out] register handle */
    );

/***************************************************************************
Summary:
    This function closes SPI register handle

Description:
    This function is responsible for closing the SPI register handle.
    This function frees BSPI register handle.

Returns:
    TODO:

See Also:
    BSPI_CreateSpiRegHandle()

****************************************************************************/
BERR_Code BSPI_CloseSpiRegHandle(
    BREG_SPI_Handle     hSpiReg             /* [in] SPI register handle */
    );

/***************************************************************************
Summary:
    This function aborts the current SPI transfer

Description:
    This function allows the user to abort an on-going SPI transfer.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BSPI_AbortTxfr (
    BSPI_ChannelHandle channel          /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function sets the SPI clock config

Description:

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BSPI_GetClkConfig(
    BSPI_ChannelHandle hChn,            /* Device channel handle */
    uint8_t *pClkConfig                 /* pointer to clock config */
    );

/***************************************************************************
Summary:
    This function gets the SPI clock clock config

Description:

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BSPI_SetClkConfig(
    BSPI_ChannelHandle hChn,            /* Device channel handle */
    uint8_t clkConfig                   /* clock config */
    );

/***************************************************************************
Summary:
    This function gets length of delay after transfer

Description:

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BSPI_GetDTLConfig (
    BSPI_ChannelHandle hChn,                /* Device channel handle */
    uint32_t *pDTLConfig                    /* pointer to DTLConfig */
    );

/***************************************************************************
Summary:
    This function sets length of delay after transfer

Description:

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BSPI_SetDTLConfig (
    BSPI_ChannelHandle context,             /* Device channel handle */
    const uint32_t data                     /* data to write */
    );

/***************************************************************************
Summary:
    This function gets the length of delay from PCS valid to SCK transition

Description:

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BSPI_GetRDSCLKConfig(
    BSPI_ChannelHandle hChn,                /* Device channel handle */
    uint32_t *pRDSCLKConfig                 /* pointer to RDSCLKConfig */
    );

/***************************************************************************
Summary:
    This function sets the length of delay from PCS valid to SCK transition

Description:

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BSPI_SetRDSCLKConfig(
    BSPI_ChannelHandle context,             /* Device channel handle */
    const uint32_t data                     /* data to write */
    );

/***************************************************************************
Summary:
    This function sets the functions to assert SS and deassert SS for HIF MSPI

Description:

Returns:
    TODO:

See Also:

****************************************************************************/
void BSPI_RegisterSSFunctions (
    BSPI_ChannelHandle hChn,                /* Device channel handle */
    BSPI_AssertSSFunc assertFunc,           /* Assert SS function */
    BSPI_DeassertSSFunc deassertFunc        /* Deassert SS function */
    );


/***************************************************************************
Summary:
    This function sets the flag which controls whether to continue driving
    SS after the last byte of transfer.

Description:

Returns:
    TODO:

See Also:

****************************************************************************/
void BSPI_SetLastByteContinueEnable(
    BSPI_ChannelHandle hChn,                /* Device channel handle */
    bool bEnable                            /* Enable or disable last byte cont flag */
);

#ifdef __cplusplus
}
#endif

#endif
