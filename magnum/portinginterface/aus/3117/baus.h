/***************************************************************************
 *     Copyright (c) 2003-2009, Broadcom Corporation
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
The upstream transmitter incorporates burst formatting and encoding logic,
an all-digital QAM modulator, a 10-bit D/A converter with output power control,
and an upstream power amplifier. It supports DQPSK, QPSK, and 16-QAM modulation.
The transmitter takes burst or continuous data, provides FEC encoding and
pre-equalization, filters and modulates the data stream, and provides
a direct 0-65 MHz analog output.  The upstream transmitter module also includes
power amplifier.


TODO TODO For Bcm3117 core, the power amplifier is an external module.
The power amplifier is responsible for controlling the power output level.


Design
The design for BAUS PI API is broken into two parts.

o Part 1 (open/close/configuration):

    These APIs are used for opening and closing BAUS device.
    When a device is opened, the device can be configured.

o Part 2 (get/set):

    These APIs are used for getting and setting runtime attributes of BAUS.


Usage
The usage of BAUS involves the following:

   * Configure/Open of BAUS

      * Configure BAUS device for the target system
      * Open BAUS device

   * Program device

      * Program Operation Mode ( Curently Annex A mode is not supported)
      * Program Symbol Rate
      * Program Rf Frequency
      * Program Rf Power Level

Interrupt Requirements
None


Sample Code
///
// NOTE: The following sample code does not do any error checking.
//
// In this example, the target H/W board is configured for Upstream
// of Multistream CableCard application, with a Crystal Frequency of 27.00 MHz.
//
static BAUS_Handle ausDev;
static BCHP_Handle hChip3117;
static BREG_Handle hReg3117;
static BRPC_Handle hRpc;

main( void )
{
    BERR_Code retCode = BERR_SUCCESS;
    BAUS_Settings ausDevSettings;

    // Initialize hChip3117, hReg3117, and hRpc . . .

    // Now startup the RPC Notificiation Backup Ground thread/task . . .

    // *************************************************************************
    // Start of H/W board specific configuration for BAUS
    // *************************************************************************
    // Configure BAUS Device
    BAUS_GetDefaultSettings( &ausDevSettings, hChip3117 );
    ausDevSettings.xtalFreq = 27000000;
    // Open BAUS Device
    BAUS_Open( ausDev, hChip3117, hReg3117, &ausDevSettings );

    // Setup Upstream for CableCard Application, with a symbol rate of 772 KBaud
    // at 10.00 MHz and with a power level of 30.0 dBmV.
    BAUS_SetOperationMode( ausDev, BAUS_OperationMode_ePodDvs178 );
    BAUS_SetSymbolRate( ausDev, 772000 );
    BAUS_SetRfFreq( ausDev, 10000000 );
    BAUS_SetPowerLevel( ausDev, 300 );
}

</verbatim>
***************************************************************************/

#ifndef BAUS_H__
#define BAUS_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bkni.h"
#include "bchp.h"
#include "bint.h"
#include "berr_ids.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    Error Codes specific to BAUS

Description:

See Also:

****************************************************************************/


/***************************************************************************
Summary:
    The handles for Qam Upstream module.

Description:

See Also:
    BAUS_Open()

****************************************************************************/
typedef struct BAUS_P_Handle            *BAUS_Handle;

/***************************************************************************
Summary:
    Enumeration for operation mode of Qam Upstream.

Description:
    This enumeration defines the operation mode for upstream.

See Also:
    BAUS_SetOperationMode(), BAUS_GetOperationMode()

****************************************************************************/
typedef enum BAUS_OperationMode
{
    BAUS_OperationMode_eAnnexA,
    BAUS_OperationMode_eDvs167 = BAUS_OperationMode_eAnnexA,
    BAUS_OperationMode_eDavic = BAUS_OperationMode_eAnnexA,
    BAUS_OperationMode_eDvs178,
    BAUS_OperationMode_eStarvue = BAUS_OperationMode_eDvs178,
    BAUS_OperationMode_eDocsis,
    BAUS_OperationMode_eTestSingleCw,
    BAUS_OperationMode_eTestDoubleCw,
    BAUS_OperationMode_eTestPn23,
    BAUS_OperationMode_ePodAnnexA,
    BAUS_OperationMode_ePodDvs167 = BAUS_OperationMode_ePodAnnexA,
    BAUS_OperationMode_ePodDavic = BAUS_OperationMode_ePodAnnexA,
    BAUS_OperationMode_ePodDvs178,
    BAUS_OperationMode_ePodStarvue = BAUS_OperationMode_ePodDvs178,
    BAUS_OperationMode_eLast
} BAUS_OperationMode;

/***************************************************************************
Summary:
    Enumeration for burst bank number of Qam Upstream.

Description:
    This enumeration defines the burst bank number for upstream.
    Bcm3117 only supports burst bank 0 to 6.

See Also:
    BAUS_SetBurstProfile()

****************************************************************************/
typedef enum BAUS_BurstBank
{
    BAUS_BurstBank_eNbr0 = 0,
    BAUS_BurstBank_eNbr1,
    BAUS_BurstBank_eNbr2,
    BAUS_BurstBank_eNbr3,
    BAUS_BurstBank_eNbr4,
    BAUS_BurstBank_eNbr5,
    BAUS_BurstBank_eLast
} BAUS_BurstBank;

/***************************************************************************
Summary:
    Structure for burst bank of Qam Upstream.

Description:
    This structure defines the burst bank for upstream.  Please see
    Docsis 1.1 Radio Frequency Interface Specification (SP-RFIv1.1-103-991105)
    documentation under 'Upstream Physical-Layer Burst Attributes'
    for detail information of burst profile.

See Also:
    BAUS_SetBurstProfile()

****************************************************************************/
typedef struct BAUS_BurstProfile
{
    uint8_t modulationType;         /* 1=QPSK, 2=16Qam */
    uint8_t diffEncodingOnOff;      /* 1=On, 2=Off */
    uint16_t preambleLength;        /* 9:0 bits */
    uint16_t preambleValOffset;     /* 8:0 bits */
    uint8_t fecBytes;               /* 0-10 */
    uint8_t fecCodewordInfoBytes;   /* 0, 16-253 */
    uint16_t scramblerSeed;         /* 14:0 bits */
    uint8_t maxBurstSize;           /* TODO: */
    uint8_t guardTimeSize;          /* TODO: */
    uint8_t lastCodewordLength;     /* 1=fixed, 2=shortened */
    uint8_t scramblerOnOff;         /* 1=on, 2=off */
    uint8_t nbrPreambleDiffEncoding; /* Number of Preamble different encoding */
} BAUS_BurstProfile;

/***************************************************************************
Summary:
    This structure represents the QUS Status for a Qam Upstream.

Description:
    This structure is returned when BAUS_GetStatus() is called.

See Also:
    BAUS_GetStatus()

****************************************************************************/
typedef struct BAUS_Status
{
    bool isPowerSaverEnabled;           /* Eanble=1, Disable=0 */
    BAUS_OperationMode operationMode;   /* Current Operation mode */
    uint32_t sysXtalFreq;               /* in Hertz, Sys. Xtal freq */
    unsigned int powerLevel;            /* Current Power level, in hundredth of dBmV */
    unsigned long rfFreq;               /* Current RF frequency, in Hertz */
    unsigned long symbolRate;           /* Current symbol rate, in baud */
} BAUS_Status;

#define BAUS_SETTINGS_XTALFREQ                  (27000000)  /* 27.00 MHz */
#define BAUS_SETTINGS_OPERATIONMODE             (BAUS_OperationMode_eDvs178)

/***************************************************************************
Summary:
    Used to configure the upstream moule for test modes.

Description:
    Used to configure the upstream moule for test modes.

See Also:
    BAUS_Open()

****************************************************************************/

typedef struct BAUS_TestModeSettings
{   
    BAUS_OperationMode operationMode;   /* Mode of operation */
    bool bypassFilters;                 /* 0 = Bypass filter. 1 = Don't bypass filter. Applies only to single tone and double tone test modes. */
    bool nyquistAlphaValue;             /* 0 = 25% Nyquist Alpha. 1 = 50% Nyquist Alpha. Applies to all test modes. */
    bool spectrumInverted;              /* 0 = Spectrum not inverted. 1 = Spectrum inverted. Applies only to pn23 test mode. */
    unsigned int qamMode;               /* 0 = QAM2; 1 = QAM4; 2 = QAM16; 3 = QAM64; 4= QAM256. Applies only to pn23 test mode.*/
    unsigned long rfFreq2;              /* Sets the second Rf frequency for two tone testing. */
} BAUS_TestModeSettings;

/***************************************************************************
Summary:
    Used to for the upstream moule's serial to parallel configuration.

Description:
    Used to for the upstream moule's serial to parallel configuration.

See Also:
    BAUS_Open()

****************************************************************************/

typedef struct BAUS_S2PSettiongs
{   
    bool clockSource;           /* 0 = Tx serial clock src is external. 1 = Internal. */
    bool clockPolarity;         /* 0 = Data output on Falling Edge of Tx Serial Clock.  1 = Raising edge. */ 
    bool dataValidPolarity;     /* 0 = Active low data valid.  1 = Active high data valid. */ 
    bool bitOrder;              /* 0 = LSB bit transmitted first. 1= MSB transmitted first. */
    bool cellAvailable;         /* 0 = CLAV signal is based on ATM cell. 1 = CLAV signal is based on byte availablitlity. */
    bool enableS2P;             /* 0 = Disable s2p. 1 = Enable. */
} BAUS_S2PSettiongs;

/***************************************************************************
Summary:
    Required default settings structure for Qam Upstream module.

Description:
    The default setting structure defines the default configuration of
    Qam Upstream when the device is opened.
    clkSrc, polarity, dataValidPolarity, bitOrder, cellAvailable and enableS2P are used to 
    for the upstream moule's serial to parallel configuration.

See Also:
    BAUS_Open()

****************************************************************************/
typedef struct BAUS_Settings
{
    unsigned int devId;                 /* generic device ID */
    void *hGeneric;                     /* generic handle can be used for anything */
    unsigned long xtalFreq;             /* Crystal Freqency in Hertz */
    BAUS_OperationMode operationMode;   /* Mode of operation */
    BAUS_S2PSettiongs s2PSettings;      /* Serial to Prallel settings. */ 
} BAUS_Settings;

/***************************************************************************
Summary:
    This function opens Qam Upstream module.

Description:
    This function is responsible for opening BAUS module. When BAUS is
    opened, it will create a module handle and configure the module based
    on the default settings. Once the device is opened, it must be closed
    before it can be opened again.

Returns:
    TODO:

See Also:
    BAUS_Close(), BAUS_GetDefaultSettings()

****************************************************************************/
BERR_Code BAUS_Open(
    BAUS_Handle *pAus,                      /* [out] Returns handle */
    BCHP_Handle hChip,                      /* [in] Chip handle */
    BREG_Handle hRegister,                  /* [in] Register handle */
    const BAUS_Settings *pDefSettings       /* [in] Default settings */
    );

/***************************************************************************
Summary:
    This function closes Qam Upstream module.

Description:
    This function is responsible for closing BAUS module. Closing BAUS
    will free main BAUS handle. It is required that all opened

Returns:
    TODO:

See Also:
    BAUS_Open()

****************************************************************************/
BERR_Code BAUS_Close(
    BAUS_Handle hDev                        /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function returns the default settings for Qam Upstream module.

Description:
    This function is responsible for returns the default setting for
    BAUS module. The returning default setting should be when
    opening the device.

Returns:
    TODO:

See Also:
    BAUS_Open()

****************************************************************************/
BERR_Code BAUS_GetDefaultSettings(
    BAUS_Settings *pDefSettings,            /* [out] Returns default setting */
    BCHP_Handle hChip                       /* [in] Chip handle */
    );

/***************************************************************************
Summary:
    This function sets operation mode of Qam Upstream module.

Description:
    This function is responsible for setting operationn mode of BAUS
    module.

    NOTE: If transmitter is disabled, calling this function will
    automatically enable the transmitter.

Returns:
    TODO:

See Also:
    BAUS_GetOperationMode(), BAUS_EnableTransmitter(), BAUS_DisableTransmitter()

****************************************************************************/
BERR_Code BAUS_SetOperationMode(
    BAUS_Handle hDev,                   /* [in] Device handle */
    BAUS_OperationMode operationMode    /* [in] Requested operation mode */    
    );

/***************************************************************************
Summary:
    This function gets the current operation mode of Qam Upstream module.

Description:
    This function is responsible for getting the current operationn mode
    of BAUS module.

Returns:
    TODO:

See Also:
    BAUS_SetOperationMode()

****************************************************************************/
BERR_Code BAUS_GetOperationMode(
    BAUS_Handle hDev,                   /* [in] Device handle */
    BAUS_OperationMode *operationMode   /* [out] Current operation mode */    
    );

/***************************************************************************
Summary:
    This function sets the symbol rate of Qam Upstream module.

Description:
    This function is responsible for setting the symbol rate
    of BAUS module.

Returns:
    TODO:

See Also:
    BAUS_GetSymbolRate()

****************************************************************************/
BERR_Code BAUS_SetSymbolRate(
    BAUS_Handle hDev,                       /* [in] Device handle */
    unsigned long symbolRate                /* [in] Requested symbol rate, in baud */
    );

/***************************************************************************
Summary:
    This function gets the current symbol rate of Qam Upstream module.

Description:
    This function is responsible for getting the current symbol rate
    of BAUS module.

Returns:
    TODO:

See Also:
    BAUS_SetSymbolRate()

****************************************************************************/
BERR_Code BAUS_GetSymbolRate(
    BAUS_Handle hDev,                       /* [in] Device handle */
    unsigned long *symbolRate               /* [out] Current symbol rate, in baud */
    );

/***************************************************************************
Summary:
    This function sets the Rf frequency of Qam Upstream module.

Description:
    This function is responsible for setting the Rf frequency
    of BAUS module.

Returns:
    TODO:

See Also:
    BAUS_GetRfFreq()

****************************************************************************/
BERR_Code BAUS_SetRfFreq(
    BAUS_Handle hDev,                       /* [in] Device handle */
    unsigned long rfFreq                    /* [in] Requested RF frequency, in Hertz */
    );

/***************************************************************************
Summary:
    This function gets the current Rf frequency of Qam Upstream module.

Description:
    This function is responsible for getting the current Rf frequency
    of BAUS module.

Returns:
    TODO:

See Also:
    BAUS_SetRfFreq()

****************************************************************************/
BERR_Code BAUS_GetRfFreq(
    BAUS_Handle hDev,                       /* [in] Device handle */
    unsigned long *rfFreq                   /* [out] Current RF frequency, in Hertz */
    );

/***************************************************************************
Summary:
    This function sets power level for Qam upstream to H/W.

Description:
    This function is responsible for setting the requested power level
    for BAUS.

Returns:
    TODO:

See Also:
    BAUS_GetPowerLevel()

****************************************************************************/
BERR_Code BAUS_SetPowerLevel(
    BAUS_Handle hDev,                       /* [in] Device handle */
    unsigned int powerLevel                 /* [in] Requested power level, in hundredth of dBmV */
    );

/***************************************************************************
Summary:
    This function gets the current power level of Qam Upstream module.

Description:
    This function is responsible for getting the current power level
    of BAUS module.

Returns:
    TODO:

See Also:
    BAUS_SetPowerLevel()

****************************************************************************/
BERR_Code BAUS_GetPowerLevel(
    BAUS_Handle hDev,                       /* [in] Device handle */
    unsigned int *powerLevel                /* [out] Current power level, in hundredth of dBmV */
    );

/***************************************************************************
Summary:
    This function gets the status of Qam Upstream module.

Description:
    This function is responsible for getting the status for
    a Qam In-Band Upstream module.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BAUS_GetStatus(
    BAUS_Handle hDev,                   /* [in] Device handle */
    BAUS_Status *pStatus                /* [out] Returns status */
    );

/***************************************************************************
Summary:
    This function transmits a single 54 byte StarVue mode data packet.
    The contents of the packet are (header+payload) are set by the calling
    routine.

Description:
    Transmit a DVS178 upstream packet

Returns:
    BERR_SUCCESS, BERR_OUT_OF_DEVICE_MEMORY

See Also:

****************************************************************************/
BERR_Code BAUS_TransmitStarvuePkt(
    BAUS_Handle hDev,                       /* [in] Device handle */
    uint8_t *ucXmitBuffer,                  /* [in] Buffer to transmit */
    unsigned int size                       /* [in] Size of Buffer (in bytes) */
    );


/***************************************************************************
Summary:
    This function enables transmitter.

Description:
    This function is responsible for enabling the transmitter.

Returns:
    TODO:

See Also:
    BAUS_DisableTransmitter()

****************************************************************************/
BERR_Code BAUS_EnableTransmitter(
    BAUS_Handle hDev                        /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function disables transmitter.

Description:
    This function is responsible for diabling transmitter.
    This is also useful when BAUS needs to set into a power-saver mode or
    if the transmitter needs to shutdown.

Returns:
    TODO:

See Also:
    BAUS_EnableTransmitter()

****************************************************************************/
BERR_Code BAUS_DisableTransmitter(
    BAUS_Handle hDev                        /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function disables transmitter.

Description:
    This function is responsible for diabling transmitter.
    This is also useful when BAUS needs to set into a power-saver mode or
    if the transmitter needs to shutdown.

Returns:
    TODO:

See Also:
    BAUS_EnableTransmitter()

****************************************************************************/

BERR_Code BAUS_SetTestModeParams(
    BAUS_Handle hDev,                       /* [in] Device handle */
    BAUS_TestModeSettings *testModeSettings  /* [in] Requested operation mode settings. */
    );


#ifdef __cplusplus
}
#endif

#endif



