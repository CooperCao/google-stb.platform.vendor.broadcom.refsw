/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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

/*= Module Overview *********************************************************
<verbatim>

Overview
Tuner module represents a generalize representatiion of a tuner.  The 
basic function of a tuner is for downconverting Radio Frequency (RF) signal to
a Intermediate Frequency (IF) signal.  For InBand downstream, the tuner module
is responsible for downconverting RF frequencies of 130MHz-860MHz to an
IF signal.  For most reference design, IF frequency is either 43.75 MHz for
Annex B or 36.125 MHz for Annex A.  Annex A support requires a H/W tuner
with 8MHz SAW filter.  Annex B support requires a H/W tuner with 6MHz SAW filter.
For OutOfBand downstream, the tuner module is responsible for downconverting
RF frequencies of 70MHz-130MHz to an IF signal.  For most reference design, 
IF frequency for OutOfBand is 44MHz.
To support the numerous tuners that may appear on for system, the Tuner module
has been designed to allow for customization by the S/W board designer yet
provide a common interface for controlling the operation of the tuner.  To
accomplish the above goals, the Tuner module API is divided into two parts.
First section (called Custom) deals with customization while the other section
(called General) provides control over the operation of a tuner.  The Custom 
section API consists of Open/Close/Create.  The General section API provides
GetRfFreq/SetRfFreq, 


Design
The design for BQDS PI API is broken into two parts.

o Part 1 (open/close/configuration/create):

    These APIs are used for opening and closing a specific BTNR device.  Also
    included in this section is an API to create a general BTNR device handle.

o Part 2 (get/set):

    These APIs are used for getting and setting runtime attributes of BTNR
    using a general BTNR device handle.


Usage
The usage of BTNR involves the following:

   * Configure/Open/Create of BTNR

      * Configure BTNR device for the target system
      * Open BTNR device

   * Tune device channel

      * Set RF frequency using general BTNR handle


Interrupt Requirements:
None


Sample Code
//
// NOTE: The following sample code does not do any error checking.
//
// In this example, the target H/W board contains two tuners. One is a
// Bcm3418 tuner configuried for Annex B application, with IF Frequency
// of 43.75 MHz.  The other tuner is an Alps TDDE tuner, so used Annex B
// application.
//

#include "btnr_3418.h"
#include "btnr_alpstdde.h"

static BTNR_AlpsTdde_Handle hTnrAlpsTdde;
static BTNR_Handle hTnrDev1;
static BTNR_Handle hTnrDev2;
static BCHP_Handle hChip3250;
static BREG_Handle hReg3250;
static BREG_I2C_Handle hBcm7038I2C;


main( void )
{
    BTNR_3418_Settings tnr3418DevSettings;
    BTNR_AlpsTdde_Settings tnrAlpsTddeDevSettings;


    // Initialize hChip3250, hReg3250, hInt3250, and hBcm7038I2C . . .


    // *************************************************************************
    // Start of H/W board specific configuration for BTNR
    // *************************************************************************
    // Configure first tuner, Bcm3418 Device
    BTNR_3418_GetDefaultSettings( &tnr3418DevSettings, hChip3250 );
    // Now initialize Bcm3418 specific data, Bcm3418 needs to know the
    //   If Freq. to use, 43.75 MHz
    //   I2C address of Bcm3418 device, addr=0x10
    tnr3418DevSettings.ifFreq = 43750000;
    tnr3418DevSettings.i2cAddr = 0x10;
    // Open first tuner, Bcm3418 Device
    BTNR_3418_Open( &hTnrDev1, NULL, hBcm7038I2C, &tnr3418DevSettings );

    // Configure second tuner, Alps TDDE Device
    BTNR_AlpsTdde_GetDefaultSettings( &tnrAlpsTddeDevSettings, hChip3250 );
    // Now initialize Alps TDDE specific data
    tnrAlpsTddeDevSettings.ifFreq = 43750000;
    // Open second tuner, Alps TDDE Device
    BTNR_AlpsTdde_Open( &hTnrDev2, NULL, hBcm7038I2C, &tnrAlpsTddeDevSettings );

    // *************************************************************************
    // End of H/W board specific configuration for BTNR
    // *************************************************************************

    // Set the first tuner to 777,000,000 Hz (777 Mhz) for a digital channel
    // using general BTNR handle.
    BTNR_SetTunerRfFreq( hTnrDev1, 777000000, BTNR_TunerMode_eDigital );
    // Set the first tuner to 600,000,000 Hz (600 Mhz) for a analog channel
    // using general BTNR handle.
    BTNR_SetTunerRfFreq( hTnrDev1, 600000000, BTNR_TunerMode_eAnalog );

    // Set the second tuner to 500,000,000 Hz (500 Mhz) for a digital channel
    // using general BTNR handle.
    BTNR_SetTunerRfFreq( hTnrDev2, 500000000, BTNR_TunerMode_eDigital );
}

</verbatim>
***************************************************************************/

/****************************************************************************
    Test Plan

Purpose
    The purpose of this test script is to test Tuner PI as described in the
    Design Requirements in Twiki.

Requirement
    1 Shall use Device Porting Interface device model, supporting 1 or more device channels 
    2 Shall use Set/Get Porting Interface change model 
    3 Shall provide support for Bcm3418 tuner 
    4 Shall provide support for Out-of-Band Bcm3250 tuner 
    5 Shall support configurable board specific Tuner component per device channel 
    6 Shall provide programmable RF Frequency per device channel 
    7 Shall provide programmable Analog/Digital RF signal per device channel 

Test Setup
    See QDS and QOB Test Plans, which tests this PI.


Test Procedure
    See QDS and QOB Test Plans, which tests this PI.


***************************************************************************/

#ifndef BTNR_H__
#define BTNR_H__

#include "bchp.h"
#include "berr_ids.h"
#include "bfec.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    Error Codes specific to BTNR

Description:

See Also:

****************************************************************************/
#define BTNR_ERR_POWER_DOWN       BERR_MAKE_CODE(BERR_TNR_ID, 0)
#define BTNR_ERR_INVALID_CRC      BERR_MAKE_CODE(BERR_TNR_ID, 1)

/***************************************************************************
Summary:
    The handle for Tuner module

Description:
    An opaque handle for BTNR device.

See Also:
    BTNR_xxx_CreateTunerHandle(), 'xxx' is specific to a tuner, for example
    BTNR_3418_CreateTunerHandle() for Bcm3418 tuner.

****************************************************************************/
typedef struct BTNR_P_Handle                *BTNR_Handle;

/***************************************************************************
Summary:
    Enumeration for Tuner mode

Description:
    This enumeration defines the tuner mode for Qam Upstream.

See Also:
    BTNR_SetTunerRfFreq(), BTNR_GetTunerRfFreq()

****************************************************************************/
typedef enum BTNR_TunerMode
{
    BTNR_TunerMode_eDigital,
    BTNR_TunerMode_eAnalog,
    BTNR_TunerMode_eDocsis,   
    BTNR_TunerMode_eLast
} BTNR_TunerMode;

/***************************************************************************
Summary:
    Enumeration for Tuner Application

Description:
    This field controls the target application.

See Also:
    BTNR_SetSettings(), BTNR_GetSettings()

****************************************************************************/
typedef enum BTNR_TunerApplication
{
  BTNR_TunerApplication_eCable,
  BTNR_TunerApplication_eTerrestrial,
  BTNR_TunerApplication_eLast
} BTNR_TunerApplication;

/***************************************************************************
Summary:
    Enumeration for modulation standard

Description:
    This field controls modulation standard to be received.

See Also:
    BTNR_SetSettings(), BTNR_GetSettings()

****************************************************************************/
typedef enum BTNR_Standard
{
  BTNR_Standard_eDvbt,
  BTNR_Standard_eIsdbt,
  BTNR_Standard_eQam,
  BTNR_Standard_eDvbt2,
  BTNR_Standard_eVsb,
  BTNR_Standard_eLast
} BTNR_Standard;

/***************************************************************************
Summary:
    Enumeration for FFT Size for Spetrum Analyzer Data

Description:
    This enumeration represents the FFT Size for Spetrum Analyzer Data.

See Also:

****************************************************************************/
typedef enum BTNR_FftSize
{
    BTNR_FftSize_e64 = 0x6,
    BTNR_FftSize_e128 = 0x7,
    BTNR_FftSize_e256 = 0x8,
    BTNR_FftSize_e512 = 0x9,
    BTNR_FftSize_e1024 = 0x0a,
    BTNR_FftSize_e2048 = 0x0b,
    BTNR_FftSize_e4096 = 0x0c,
    BBTNR_FftSize_eLast
} BTNR_FftSize;

/***************************************************************************
Summary:
    Callback used for event notification.

Description:
    When this PI wants to notify an application, it will call this callback
    function the callback function is registered.

See Also:

****************************************************************************/
typedef BERR_Code (*BTNR_CallbackFunc)(void *pParam );

/***************************************************************************
Summary:
    Enumeration for Callback types

Description:
    This enumeration defines Callback types.

See Also:

****************************************************************************/
typedef enum BTNR_Callback
{
    BTNR_Callback_eSpectrumDataReady,   /* Callback to notify application that spectrum analyzer data is ready */ 
    BTNR_Callback_eIfDacAcquireComplete,   /* Callback to notify application that spectrum analyzer data is ready */ 
    BTNR_Callback_eIfDacStatusReady,   /* Callback to notify application that spectrum analyzer data is ready */ 
    BTNR_Callback_eLast                 /* More may be required */
} BTNR_Callback;

/***************************************************************************
Summary:
    Structure for Tuner Information

Description:
    This defines the tuner information.

See Also:
    BTNR_GetInfo()

****************************************************************************/
#define BRCM_TUNER_MAKER_ID             (1)
typedef struct BTNR_TunerInfo
{
    unsigned int tunerMaker;            /* 0=unknown, 1=BRCM Tuner */
    unsigned int tunerId;
    unsigned int tunerMajorVer;
    unsigned int tunerMinorVer;
} BTNR_TunerInfo;

/***************************************************************************
Summary:
    Structure to set tuner into power saver mode.

Description:
    Structure to set tuner into power saver mode.

See Also:
    BTNR_GetInfo()

****************************************************************************/
typedef struct BTNR_PowerSaverSettings
{
    bool enable; /* 1 =  enable power saver, 0 = disable power saver. */
} BTNR_PowerSaverSettings;

/***************************************************************************
Summary:
    Structure to set tuner settings.

Description:
    Structure to set tuner settings.

See Also:
    BTNR_GetSettings(), BTNR_SetSettings()

****************************************************************************/
typedef struct BTNR_Settings
{
    BTNR_Standard std; /* standard */
    uint32_t bandwidth; /* low pass filter bandwidth in units of Hz */
    uint32_t agcVal; /* AGC value */
} BTNR_Settings;

/***************************************************************************
Summary:
    Structure for IF DAC settings.

Description:
    Structure for IF DAC settings.

See Also:
    BTNR_GetSettings(), BTNR_SetSettings()

****************************************************************************/
typedef struct BTNR_IfDacSettings
{
    BTNR_Standard std; /* standard */
    uint32_t frequency; /* frequency in Hz */
    uint32_t bandwidth; /* bandwidth in units of Hz, supported bandwidth settings are 6 MHz (Annex B) and 8 MHz (Annex A) */
    uint32_t outputFrequency; /* IF DAC output Frequency in Hz. Currently supported output frequencies: 4 MHz, 5 MHz, 36 MHz and 44 MHz */
    int16_t dacAttenuation; /* DAC Attenuation in units of 1/256th of dB, range: 0 to 5120 (20 dB) */
} BTNR_IfDacSettings;

/***************************************************************************
Summary:
    This structure represents the TNR Status.

Description:
    This structure is returned when BTNR_GetStatus() is called.  This
    structure contains the complete status of a TNR channel.

See Also:
    BTNR_GetStatus()

****************************************************************************/
typedef struct BTNR_IfDacStatus
{
    int16_t rssi; /* rssi in units of 1/100th of a dBm */
    BTNR_IfDacSettings ifDacSettings; /* IF DAC Settings */
} BTNR_IfDacStatus;

/***************************************************************************
Summary:
    Structure for Spectrum Analyzer Data

Description:

See Also:
    BTNR_GetSpectrumAnalyzerData()

****************************************************************************/
typedef struct BTNR_SpectrumData
{
    uint32_t *data; /* ptr to the data */
    uint16_t datalength; /* data length in number of words */    
    bool moreData; /* this bit indicates whether there is more data available or not */
} BTNR_SpectrumData;

/***************************************************************************
Summary:
    Spectrum settings for Spectrum Analyzer Data

Description:

See Also:
    BTNR_RequestSpectrumAnalyzerData()

****************************************************************************/
typedef struct BTNR_SpectrumSettings
{
    uint32_t startFreq; /* The start frequency of the spectrum analyzer in Hz; startFreq range: 0 to 1GHz */
    uint32_t stopFreq; /* The stop frequency of the spectrum analyzer in Hz; stopFreq range: 0 to 1GHz  */
    BTNR_FftSize fftSize; /*  FFT Size */
    uint8_t reserved; /*  Reserved for future use */
    uint32_t numSamples; /* Total number of 32-bit frequency values to return. numSamples can range from a value of 1 to 3000 */
} BTNR_SpectrumSettings;

/***************************************************************************
Summary:
    This function sets the Tuner to the requested frequency.

Description:
    This function is responsible for setting Tuner to a frequency.

    This function will call the tuner specific SetRfFreq function.
    
Returns:
    TODO:

See Also:
    BTNR_GetTunerRfFreq()

****************************************************************************/
BERR_Code BTNR_SetTunerRfFreq(
    BTNR_Handle hDev,                   /* [in] Device handle */
    uint32_t freq,                      /* [in] Requested tuner freq., in Hertz */
    BTNR_TunerMode tunerMode            /* [in] Requested tuner mode */
    );

/***************************************************************************
Summary:
    This function gets the current Tuner tuned frequency.

Description:
    This function is responsible for getting the current Tuner tuned frequency.

    This function will call the tuner specific GetRfFreq function.
    
Returns:
    TODO:

See Also:
    BTNR_SetTunerRfFreq()

****************************************************************************/
BERR_Code BTNR_GetTunerRfFreq(
    BTNR_Handle hDev,                   /* [in] Device handle */
    uint32_t *freq,                     /* [out] Returns tuner freq., in Hertz */
    BTNR_TunerMode *tunerMode           /* [out] Returns tuner mode */
    );

/***************************************************************************
Summary:
    This function gets the Tuner Information.

Description:
    This function is responsible for getting Tuner Information.
    
Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTNR_GetInfo(
    BTNR_Handle hDev,                   /* [in] Device handle */
    BTNR_TunerInfo *tnrInfo             /* [out] Tuner information */
    );

/***************************************************************************
Summary:
    This function closes Tuner module.

Description:
    This function is responsible for closing Tuner module.
    
Returns:
    TODO:

See Also:
    BTNR_xxxx_Open()

****************************************************************************/
BERR_Code BTNR_Close(
    BTNR_Handle hDev                    /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function gets the power-saver mode.

Description:
    This function returns the current power-saver mode.
    
Returns:
    TODO:

See Also:
    BTNR_xxxx_Open()

****************************************************************************/
BERR_Code BTNR_GetPowerSaver(
    BTNR_Handle hDev,                   /* [in] Device handle */
    BTNR_PowerSaverSettings *pwrSettings /* [out] Power saver settings. */
    );

/***************************************************************************
Summary:
    This function sets the power-saver mode.

Description:
    This function is responsible for enabling/disabling the downstream tuner
    power-saver mode.  When the power-saver mode is enabled, the
    Qam In-Band Downstream tuner is shutdown. 
    
Returns:
    TODO:

See Also:
    BTNR_xxxx_Open()

****************************************************************************/
BERR_Code BTNR_SetPowerSaver(
    BTNR_Handle hDev,                   /* [in] Device handle */
    BTNR_PowerSaverSettings *pwrSettings /* [in] Power saver settings. */
    );

/***************************************************************************
Summary:
    This function gets the Tuner's AGC register value.

Description:
    This function is responsible for getting the AGC register value using
    register offset.
    
Returns:
    TODO:

See Also:
    BTNR_xxxx_Open()

****************************************************************************/
BERR_Code BTNR_P_GetTunerAgcRegVal(
    BTNR_Handle hDev,                   /* [in] Device handle */
    uint32_t regOffset,                 /* [in] AGC register offset */
    uint32_t *agcVal                    /* [out] output value */
    );

/***************************************************************************
Summary:
    This function sets the Tuner's AGC register value.

Description:
    This function is responsible for setting the AGC register value using
    register offset.
    
Returns:
    TODO:

See Also:
    BTNR_xxxx_Open()

****************************************************************************/
BERR_Code BTNR_SetTunerAgcRegVal(
    BTNR_Handle hDev,                   /* [in] Device handle */
    uint32_t regOffset,                 /* [in] AGC register offset */
    uint32_t *agcVal                    /* [in] input value */
    );

/***************************************************************************
Summary:
    This function gets TNR settings.

Description:
    This function returns the current TNR Settings.
    
Returns:
    TODO:

See Also:
    BTNR_xxxx_Open()

****************************************************************************/
BERR_Code BTNR_GetSettings(
    BTNR_Handle hDev,           /* [in] Device handle */
    BTNR_Settings *settings     /* [out] TNR settings. */
    );
    
/***************************************************************************
Summary:
    This function sets TNR settings.

Description:
    This function sets the current TNR Settings.
    
Returns:
    TODO:

See Also:
    BTNR_xxxx_Open()

****************************************************************************/
BERR_Code BTNR_SetSettings(
    BTNR_Handle hDev,           /* [in] Device handle */
    BTNR_Settings *settings     /* [in] TNR settings. */
    );
    
/******************************************************************************
Summary:
   This function sends request for spectrum analyzer data to the LEAP.
Description:
  
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTNR_RequestSpectrumAnalyzerData(
    BTNR_Handle hDev,     /* [in] Device handle */ 
    BTNR_SpectrumSettings *pSettings /* [in] spectrum settings */
    );

/******************************************************************************
Summary:
   This function gets spectrum analyzer data from the LEAP.
Description:
  
Returns:
   BERR_Code
******************************************************************************/    
BERR_Code BTNR_GetSpectrumAnalyzerData(
    BTNR_Handle hDev,     /* [in] Device handle */ 
    BTNR_SpectrumData  *pSpectrumData /* [out] spectrum Data*/
    );    
    
/***************************************************************************
Summary:
    This function is responsible for installing a callback function.

Description:
    This function installs a callback function.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTNR_InstallCallback(
    BTNR_Handle hDev,     /* [in] Device handle */
    BTNR_Callback callbackType, /* [in] Type of callback */
    BTNR_CallbackFunc pCallback_isr, /* [in] Function Ptr to ISR callback */
    void *pParam                 /* [in] Generic parameter send on callback */
    );

/***************************************************************************
Summary:
    This function returns the version information.

Description:
    This function is responsible for returning the core driver version 
    information. It return the majorVersion and minorVersion of the core
    driver.
Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTNR_GetVersionInfo(
    BTNR_Handle hDev,              /* [in] Device handle */
    BFEC_VersionInfo *pVersionInfo /* [out] Returns version Info */
    );    

/***************************************************************************
Summary:
    This function tunes IF DAC to the requested frequency.

Description:
    This function is responsible for tuning the IF DAC to the requested 
    frequency.
    
Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTNR_TuneIfDac(
    BTNR_Handle hDev,                   /* [in] Device handle */
    BTNR_IfDacSettings *pSettings       /* [in] IF DAC Settings */
    );

/***************************************************************************
Summary:
    This function resets the status of the IF DAC.

Description:
    This function is responsible for resetting If DAC status.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTNR_ResetIfDacStatus(
    BTNR_Handle hDev        /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function requests the status asynchronously of the IF DAC channel.

Description:
    This function is responsible for requesting the status to be calculated 
    asynchronously for an IF DAC channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTNR_RequestIfDacStatus(
    BTNR_Handle hDev        /* [in] Device handle */
    );
    
/***************************************************************************
Summary:
    This function gets the status asynchronously of the IF DAC channel.

Description:
    This function is responsible for asynchronously getting the complete status
    of the IF DAC.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTNR_GetIfDacStatus(
    BTNR_Handle hDev,               /* [in] Device handle */
    BTNR_IfDacStatus *pStatus       /* [out] Returns status */
    );
    
#ifdef __cplusplus
}
#endif
 
#endif
