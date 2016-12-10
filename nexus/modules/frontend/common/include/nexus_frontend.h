/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ***************************************************************************/
#ifndef NEXUS_FRONTEND_H__
#define NEXUS_FRONTEND_H__

#include "nexus_types.h"
#include "nexus_input_band.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/**
Summary:
Handle for a generic frontend object, which typically corresponds to a single input/tuner/demod path (or channel)
**/
typedef struct NEXUS_Frontend *NEXUS_FrontendHandle;
/**
Summary:
FrontendDevice is a single frontend chip which can have one or more channels
**/
typedef struct NEXUS_FrontendDevice *NEXUS_FrontendDeviceHandle;

#include "nexus_frontend_qam.h"
#include "nexus_frontend_vsb.h"
#include "nexus_frontend_satellite.h"
#include "nexus_frontend_ofdm_types.h"
#include "nexus_frontend_ofdm.h"
#include "nexus_frontend_oob.h"
#include "nexus_frontend_upstream.h"
#include "nexus_frontend_analog.h"
#include "nexus_tuner.h"
#include "nexus_amplifier.h"
#include "nexus_ifd.h"
#include "nexus_frontend_card.h"
#include "nexus_frontend_t2_types.h"
#include "nexus_frontend_dvb_t2.h"
#include "nexus_frontend_dvb_c2.h"
#include "nexus_frontend_dvbt.h"
#include "nexus_frontend_isdbt.h"

#include "nexus_i2c.h"
#include "nexus_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Interrupt Mode -- Used to choose the notification method from frontend to the backedn.
***************************************************************************/
typedef enum NEXUS_FrontendInterruptMode
{
    NEXUS_FrontendInterruptMode_eInterrupt, /* default, use either gpioInterrupt or isrNumber */
    NEXUS_FrontendInterruptMode_ePolling,
    NEXUS_FrontendInterruptMode_eMax
} NEXUS_FrontendInterruptMode;

/***************************************************************************
Summary:
Frontend Temperature -- Allows frontend temperature to be queried by the application.
***************************************************************************/
typedef struct NEXUS_FrontendTemperature
{
    unsigned temperature;                           /* units of 1/100 degree celcius */
} NEXUS_FrontendTemperature;


/***************************************************************************
Summary:
Frontend Capabilities -- Allows capabilities of generic frontend objects to be queried by the application.
***************************************************************************/
typedef struct NEXUS_FrontendCapabilities
{
    bool powerDownSupported;                        /* If true, this tuner supports a power-down mode */
    bool analog;                                    /* If true, this tuner can support analog/NTSC tuning */
    bool ifd;                                       /* If true, this tuner can demodulate an analog signal with the IFD */
    bool ifdac;                                     /* If true, this tuner channel if-dac. There is no demod connected to this tuner output. */
    bool qam;                                       /* If true, at least one QAM mode is supported */
    bool docsis;                                    /* If true, DOCSIS is supported */
    bool vsb;                                       /* If true, at least one VSB mode is supported */
    bool satellite;                                 /* If true, at least one satellite mode is supported */
    bool outOfBand;                                 /* If true, at least one out of band mode is supported */
    bool upstream;                                  /* If true, at least one upstream mode is supported */
    bool ofdm;                                      /* If true, OFDM modulation is supported */
    bool scan;                                      /* If true, NEXUS_Frontend_ScanFrequency is supported for this frontend */
    bool qamModes[NEXUS_FrontendQamMode_eMax];      /* For each QAM mode, the mode is supported if true and unsupported if false */
    bool vsbModes[NEXUS_FrontendVsbMode_eMax];      /* For each VSB mode, the mode is supported if true and unsupported if false */
    bool satelliteModes[NEXUS_FrontendSatelliteMode_eMax];  /* For each satellite mode, the mode is supported if true and unsupported if false */
    bool outOfBandModes[NEXUS_FrontendOutOfBandMode_eMax];  /* For each out-of-band mode, the mode is supported if true and unsupported if false */
    bool upstreamModes[NEXUS_FrontendUpstreamMode_eMax];    /* For each upstream mode, the mode is supported if true and unsupported if false */
    bool ofdmModes[NEXUS_FrontendOfdmMode_eMax];    /* For each OFDM mode, the mode is supported if true and unsupported if false */
    bool diseqc;
} NEXUS_FrontendCapabilities;

/***************************************************************************
Summary:
    Return a list of capabilities for a given frontend object.

Description:
    This call returns a list of capabilities for a frontend object.  Because
    many frontends support multiple types of modulation schemes, this
    allows the application to distinguish the capabilities of one tuner
    versus another.  If a tuner has multiple capabilities, only one of
    the modes may be used at any one time.
See Also:
    NEXUS_Frontend_Get
***************************************************************************/
void NEXUS_Frontend_GetCapabilities(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendCapabilities *pCapabilities   /* [out] */
    );

/***************************************************************************
Summary:
Frontend Soft Decisions
***************************************************************************/
typedef struct NEXUS_FrontendSoftDecision
{
    int16_t i; /* ranges from 32767 to -32768 for all devices, however precision may vary per device. */
    int16_t q; /* ranges from 32767 to -32768 for all devices, however precision may vary per device. */
} NEXUS_FrontendSoftDecision;

/***************************************************************************
Summary:
    Get a fixed-sized array of soft decisions for a constellation.

Description:
    Deprecated. Use NEXUS_Frontend_ReadSoftDecisions instead.
***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetSoftDecisions(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendSoftDecision *pDecisions, /* attr{nelem=length;reserved=100} array of soft decisions */
    size_t length                           /* number of NEXUS_FrontendSoftDecision's to get */
    );

/***************************************************************************
Summary:
    Read an array of soft decisions for a constellation.

Description:
    Like NEXUS_Frontend_GetSoftDecisions, but will only do a single read and return pNunRead.
***************************************************************************/
NEXUS_Error NEXUS_Frontend_ReadSoftDecisions(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendSoftDecision *pDecisions, /* attr{nelem=length;nelem_out=pNumRead;reserved=100} array of soft decisions */
    size_t length,                          /* maximum number of NEXUS_FrontendSoftDecision's to read */
    size_t *pNumRead                        /* actual number read */
    );

/***************************************************************************
Summary:
Frontend User Parameters -- Optional values that platform may bind to a
                            specific device/channel to pass information
                            to the application (such as input band)

This struct is deprecated. It should not be extended. If additional frontend
information is needed from the platform, the NEXUS_PlatformConfiguration struct should
be extended so the info can be communicated directly, not through a tunnel.
***************************************************************************/
typedef struct NEXUS_FrontendUserParameters
{
    int param1; /* if isMtsif==false, this is the SoC's input band number that the frontend is connected to.
                   if isMtsif==true, it's the demod's input band number. */
    unsigned pParam2;
    unsigned id; /* platform-assigned id for this frontend instance */
    uint16_t chipId; /* demod chip, in hex, e.g. 0x3117 */
    uint16_t chipRevision; /* demod chip revision, in hex, e.g. 0x0010 = A0, 0x0021 = B1
                              0 = "not needed" or "not available" */
    bool isMtsif; /* if true, uses MTSIF connection between demod and SoC. if false, legacy connection */
} NEXUS_FrontendUserParameters;

/***************************************************************************
Summary:
    Get user parameters provided by NEXUS_Frontend_SetUserParameters.

Description:
    This is an optional call that allows the platform layer to attach some
    user parameters to a frontend object.  The application can, in turn,
    retrieve these parameters to get some data from platform such as
    input band.
See Also:
    NEXUS_Frontend_SetUserParameters
***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetUserParameters(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendUserParameters *pParams   /* [out] */
    );

/***************************************************************************
Summary:
    Set user parameters for a frontend.

Description:
    This is an optional call that allows the platform layer to attach some
    user parameters to a frontend object.  The application can, in turn,
    retrieve these parameters to get some data from platform such as
    input band.
See Also:
    NEXUS_Frontend_GetUserParameters
***************************************************************************/
NEXUS_Error NEXUS_Frontend_SetUserParameters(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendUserParameters *pParams
    );


/***************************************************************************
Summary:
Major/minor version code

Examples:
For silicon,
major=0x01 and minor=0x00, denotes A0.
major=0x01 and minor=0x01, denotes A1.
major=0x02 and minor=0x02, denotes B2.

For firmware:
major=0x00 and minor=0x05, denotes 0.5.
major=0x01 and minor=0x01, denotes 1.1.
major=0x02 and minor=0x02, denotes 2.2.
***************************************************************************/
typedef struct NEXUS_FrontendVersion
{
    unsigned major;
    unsigned minor;
    unsigned buildType; /* only for FW */
    unsigned buildId;   /* only for FW */
} NEXUS_FrontendVersion;

#define NEXUS_FRONTEND_MAX_BONDOUT_OPTIONS 2

/***************************************************************************
Summary: Frontend Chip Info.
***************************************************************************/
typedef struct NEXUS_FrontendChipType
{
    unsigned familyId; /* Chip's family Id. In hex, e.g. 0x3128. */
    unsigned id;       /* Chip's Id or Bondout Id. In hex, e.g. 0x3128/ 0x3124/ 0x3123. */
    NEXUS_FrontendVersion version;
    unsigned bondoutOptions[NEXUS_FRONTEND_MAX_BONDOUT_OPTIONS];
} NEXUS_FrontendChipType;

/***************************************************************************
Summary: Frontend Chip and Firmware Info.
***************************************************************************/
typedef struct NEXUS_FrontendType
{
    NEXUS_FrontendChipType chip;
    NEXUS_FrontendVersion firmwareVersion;
} NEXUS_FrontendType;

/***************************************************************************
Summary:
    Retrieve the chip family id, chip id, chip version and firmware version.
***************************************************************************/
void NEXUS_Frontend_GetType(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendType *pType
    );

/***************************************************************************
Summary:
Do not use this function. This is a stub function to create the Nexus
class for systems with frontend module, but no chip-specific open
***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_OpenStub( /* attr{destructor=NEXUS_Frontend_Close} */
    unsigned index
    );

/***************************************************************************
Summary:
    Close a frontend handle
***************************************************************************/
void NEXUS_Frontend_Close(
    NEXUS_FrontendHandle handle
    );

/***************************************************************************
Summary:
    Reset state after a tune

Description:
    After this function is called, no new lock callback will be generated.
    It's possible that a pending lock callback will still be issued. If you cannot process another
    callback after NEXUS_Frontend_Untune, call NEXUS_StopCallbacks before calling NEXUS_Frontend_Untune and
    call NEXUS_StartCallbacks from any subsequent Tune function.

    In wideband frontend devices (for example 3128, 7584, 4538, 45218, 45308), which have wideband ADC, the first tune takes
    more time (at least a second, as many as four seconds) than subsequent tunes do.  This delay comes from powering up the wideband
    cores, waiting for a stable state, and running a calibration routine.

    The last frontend handle using a given ADC which untunes will turn off the wideband ADC. Any subsequent tune on that
    wideband ADC will again require the (potentially long) powerup time.

    If an app wants lowest power, it must untune all frontend handles after use and the incur the long powerup time.

    If an app wants faster tuning, it must monitor all frontend handles and not untune the last channel. This will keep
    the hardware powered up and reduce the time taken to tune and acquire the first channel.

    Alternatively, the various NEXUS_Frontend_TuneXX commands can be called with frequency==0. This is a special case,
    and causes the frontend handle in question to break the existing tune while leaving the associated frontend handle marked
    as in use. This will stop transport data from flowing from the frontend but leave the associated tuner, demod, and wideband
    ADC cores powered up. This is NOT a lower power state, but allows one to trade higher power for speedier retuning.
***************************************************************************/
void NEXUS_Frontend_Untune(
    NEXUS_FrontendHandle handle
    );

/***************************************************************************
Summary:
    Reset accumulated status values for a frontend
***************************************************************************/
void NEXUS_Frontend_ResetStatus(
    NEXUS_FrontendHandle handle
    );

/***************************************************************************
Summary:
Frontend Lock Status
****************************************************************************/
typedef enum NEXUS_FrontendLockStatus
{
    NEXUS_FrontendLockStatus_eUnknown,  /* no status available */
    NEXUS_FrontendLockStatus_eUnlocked, /* not locked, but there is a signal. */
    NEXUS_FrontendLockStatus_eLocked,   /* locked */
    NEXUS_FrontendLockStatus_eNoSignal, /* no signal; therefore not locked */
    NEXUS_FrontendLockStatus_eMax
} NEXUS_FrontendLockStatus;

/***************************************************************************
Summary:
Status returned by NEXUS_Frontend_GetFastStatus
***************************************************************************/
typedef struct NEXUS_FrontendFastStatus {
    NEXUS_FrontendLockStatus lockStatus;
    bool acquireInProgress; /* if true, you can wait for another lockCallback */
} NEXUS_FrontendFastStatus;

/***************************************************************************
Summary:
Get fast status for the frontend

Description:
Frontends may not populate all status.
Usually fast status happens from direct register reads from the SoC,
not FW transactions.
***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetFastStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendFastStatus *pStatus /* [out] */
    );

/***************************************************************************
Summary:
Re-apply transport settings to frontend

Description:
MTSIF-based frontends are routed to host parserbands using NEXUS_ParserBandSourceType_eMtsif.
NEXUS_ParserBand_SetSettings must be called prior to calling the tune function.

NEXUS_Frontend_ReapplyTransportSettings re-applies the transport settings to the frontend
after the tune.

This function is called automatically by the tune function.
***************************************************************************/
NEXUS_Error NEXUS_Frontend_ReapplyTransportSettings(
    NEXUS_FrontendHandle handle
    );

/***************************************************************************
The NEXUS_FrontendBandwidth enum has been deprecated. It is an unsigned integer in units of Hz now.
The following #defines are for backward compatibility only.
***************************************************************************/
#define NEXUS_FrontendBandwidth_e5Mhz 5000000
#define NEXUS_FrontendBandwidth_e6Mhz 6000000
#define NEXUS_FrontendBandwidth_e7Mhz 7000000
#define NEXUS_FrontendBandwidth_e8Mhz 8000000
#define NEXUS_FrontendBandwidth_eMax  4

/***************************************************************************
Summary:
***************************************************************************/
typedef struct NEXUS_FrontendSpectrumSettings
{
    void *data;              /* attr{memory=cached} ptr to the data. You must allocate this buffer with NEXUS_Memory_Allocate */
    unsigned dataLength;     /* max data length in bytes, must be >= (numSamples*4) */
    unsigned startFrequency; /* in Hz */
    unsigned stopFrequency;  /* in Hz */
    unsigned binAverage;     /* Deprecated. */
    unsigned numSamples;     /* Total number of 32-bit frequency values to return */
    unsigned fftSize;        /* Current supported FFT sizes are 64, 128, 256, 512, 1024 and 2048. */
    NEXUS_CallbackDesc dataReadyCallback;   /* Callback will be called when the spectrum data is available to be read. */
} NEXUS_FrontendSpectrumSettings;

/***************************************************************************
Summary:
***************************************************************************/
void NEXUS_Frontend_GetDefaultSpectrumSettings(
    NEXUS_FrontendSpectrumSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
***************************************************************************/
NEXUS_Error NEXUS_Frontend_RequestSpectrumData(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendSpectrumSettings *pSettings  /* spectrum settings */
    );

/***************************************************************************
Summary:
Read a frontend register for debug
***************************************************************************/
NEXUS_Error NEXUS_Frontend_WriteRegister(
    NEXUS_FrontendHandle handle,
    unsigned address, /* offset from demod's register base address */
    uint32_t value
    );

/***************************************************************************
Summary:
Write a frontend register for debug

Description:
Be aware that direct register writes can cause serious system failures. This should
only be used for debug and only as directed.
***************************************************************************/
NEXUS_Error NEXUS_Frontend_ReadRegister(
    NEXUS_FrontendHandle handle,
    unsigned address, /* offset from demod's register base address */
    uint32_t *pValue
    );

/***************************************************************************
Summary:
Mode for NEXUS_FrontendAcquireSettings
***************************************************************************/
typedef enum NEXUS_FrontendAcquireMode
{
    NEXUS_FrontendAcquireMode_eByCapabilities,
    NEXUS_FrontendAcquireMode_eByIndex,
    NEXUS_FrontendAcquireMode_eMax
} NEXUS_FrontendAcquireMode;

/***************************************************************************
Summary:
Settings for NEXUS_Frontend_Acquire
***************************************************************************/
typedef struct NEXUS_FrontendAcquireSettings
{
    NEXUS_FrontendAcquireMode mode; /* this determines which of the following fields are used */
    struct {
        bool qam;
        bool vsb;
        bool ofdm;
        bool satellite;
        bool outOfBand;
        bool ifd;
    } capabilities; /* used if mode == eByCapabilities. at least one field must be set true.
        if more than one is true, then the frontend must meet all requested capabilities. */

    unsigned index; /* used if mode == eByIndex. this is a platform-wide index of frontend typically corresponding
        to the index of NEXUS_PlatformConfiguration.frontend[]. NEXUS_ANY_ID supported.
        it may differ if the application opens/closes frontends outside of platform. */
} NEXUS_FrontendAcquireSettings;

/***************************************************************************
Summary:
Get default acquire settings
***************************************************************************/
void NEXUS_Frontend_GetDefaultAcquireSettings(
    NEXUS_FrontendAcquireSettings *pSettings
    );

/***************************************************************************
Summary:
Find a frontend based on capabilities and register handle for protected client use
***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Acquire( /* attr{release=NEXUS_Frontend_Release} */
    const NEXUS_FrontendAcquireSettings *pSettings
    );

/***************************************************************************
Summary:
Release an acquired client
***************************************************************************/
void NEXUS_Frontend_Release(
    NEXUS_FrontendHandle handle
    );

/***************************************************************************
Summary:
Returns the frontend die temperature
***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetTemperature(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendTemperature *pTemp
    );

/***************************************************************************
Summary:
Frontend debug packet type
***************************************************************************/
typedef enum NEXUS_FrontendDebugPacketType {
    NEXUS_FrontendDebugPacketType_eOob,
    NEXUS_FrontendDebugPacketType_eMax
} NEXUS_FrontendDebugPacketType;

/***************************************************************************
Summary:
Transmit a debug packet
***************************************************************************/
NEXUS_Error NEXUS_Frontend_TransmitDebugPacket(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDebugPacketType type,
    const uint8_t *pBuffer,
    size_t size
    );

/***************************************************************************
Summary:
    Current BERT status

Description:
    This struct holds the BERT status data, and is used only with a special
pseudo-random bit stream test input.
    If NEXUS_FrontendBertStatus.enabled = false, the frontend was not tuned
with BERT enabled, and the rest of the values are invalid.

See also:
    NEXUS_Frontend_GetBertStatus
***************************************************************************/
typedef struct NEXUS_FrontendBertStatus {
    bool enabled;
    bool locked;          /* current BERT lock status */
    bool syncAcquired;    /* BERT acquired sync since last NEXUS_Frontend_GetBertStatus() or NEXUS_Frontend_ResetStatus() */
    bool syncLost;        /* BERT lost sync since last NEXUS_Frontend_GetBertStatus() or NEXUS_Frontend_ResetStatus() */
    uint64_t bitCount;    /* Cumulative bit error count since last NEXUS_Frontend_ResetStatus () */
    uint64_t errorCount;  /* Total bit count since last NEXUS_Frontend_ResetStatus() */
                          /* BER = (float) errorCount / bitCount. */
} NEXUS_FrontendBertStatus;

/***************************************************************************
Summary:
    Get the current BERT status

Description:
    This function returns the BERT status for a given frontend.  For the values
to be valid, the frontend needs to have been tuned with BERT enabled.
    This call was introduced for greater precision in the bit and error counts,
plus support for new fields.  As a separate call, frontends which have support
to provide just BERT data can also return the data faster than via the larger
get status structures.

See also:
    NEXUS_FrontendBertStatus,
    NEXUS_FrontendSettings
***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetBertStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendBertStatus *pStatus
    );

/***************************************************************************
Summary:
Returns connector for this frontend

Description:
There is no attr{shutdown} needed because the frontend connector is passive in the transport module.
***************************************************************************/
NEXUS_FrontendConnectorHandle NEXUS_Frontend_GetConnector(
    NEXUS_FrontendHandle frontend
    );

/**
Summary:
**/
void NEXUS_FrontendDevice_Close(
    NEXUS_FrontendDeviceHandle handle
    );

/**
Summary:
**/
typedef enum NEXUS_FrontendDeviceRfInput
{
    NEXUS_FrontendDeviceRfInput_eOff,         /* There is no RF connection between the devices. */
    NEXUS_FrontendDeviceRfInput_eDaisy,       /* Device Rf input through parent device's daisy path. */
    NEXUS_FrontendDeviceRfInput_eLoopThrough, /* Device Rf input through parent device's loop through path. */
    NEXUS_FrontendDeviceRfInput_eMax
} NEXUS_FrontendDeviceRfInput;

/**
Summary:
**/
typedef enum NEXUS_FrontendDeviceMtsifOutput
{
    NEXUS_FrontendDeviceMtsifOutput_eDefault, /* connected directly to host, or no MTSIF output */
    NEXUS_FrontendDeviceMtsifOutput_eDaisy,   /* daisy-chained to parent frontend device */
    NEXUS_FrontendDeviceMtsifOutput_eMax
} NEXUS_FrontendDeviceMtsifOutput;

/**
Summary:
Settings used for NEXUS_FrontendDevice_Link
**/
typedef struct NEXUS_FrontendDeviceLinkSettings
{
    NEXUS_FrontendDeviceRfInput rfInput;
    NEXUS_FrontendDeviceMtsifOutput mtsif;
} NEXUS_FrontendDeviceLinkSettings;

/**
Summary:
**/
void NEXUS_FrontendDevice_GetDefaultLinkSettings(
    NEXUS_FrontendDeviceLinkSettings *pSettings
    );

/**
Summary:
Make a link between two devices
**/
NEXUS_Error NEXUS_FrontendDevice_Link(
    NEXUS_FrontendDeviceHandle parentHandle,
    NEXUS_FrontendDeviceHandle childHandle,
    const NEXUS_FrontendDeviceLinkSettings *pSettings
    );

/**
Summary:
Unlink a child or all children from the parent
**/
void NEXUS_FrontendDevice_Unlink(
    NEXUS_FrontendDeviceHandle parentHandle,
    NEXUS_FrontendDeviceHandle childHandle /* if NULL, unlink all children */
    );

/**
Summary:
Get the device for a channel
**/
NEXUS_FrontendDeviceHandle NEXUS_Frontend_GetDevice(
    NEXUS_FrontendHandle handle
    );

/**
Summary:
Make a link between a device and an amplifier
**/
NEXUS_Error NEXUS_FrontendDevice_LinkAmplifier(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_AmplifierHandle amplifierHandle
    );

/**
Summary:
Frontend Device Status
**/
typedef struct NEXUS_FrontendDeviceStatus
{
    struct {
        bool enabled;     /* If true, AVS(Automatic Voltage Scaling) is enabled on the device. */
        unsigned voltage; /* chip voltage in units of millivolts. This is only valid if enabled= true. */
    } avs;
    int temperature;      /* chip temperature in units of 1/1000 degrees Celsius. */
    bool openPending; /* Set true if async device open/resume is pending */
    bool openFailed; /* Set true if async device open/resume has failed */
} NEXUS_FrontendDeviceStatus;

/**
Summary:
Get status for the device
**/
NEXUS_Error NEXUS_FrontendDevice_GetStatus(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_FrontendDeviceStatus *pStatus
    );

typedef struct NEXUS_FrontendDeviceRecalibrateSettings
{
    struct {
        bool enabled;     /* If set to true when calling NEXUS_FrontendDevice_Recalibrate, CPPM is triggered in addition to setting the threshold settings. */
                          /* If set to false, threshold settings are updated, but CPPM is not triggered. */
        signed threshold; /* Power threshold used by CPPM algorithm to determine signal saturation. In units of 1/10 dBmV */
        unsigned thresholdHysteresis; /* Amount that composite power can crossover the Power threshold before an interrupt can be triggered. In units of 1/10 dB */
        NEXUS_CallbackDesc powerLevelChange;   /* This callback will be called when the cable-plant composite power changes, so that CPPM may be recalibrated
                                                  by the application. Since the recalibration disables all WFE/demods channels, the application needs to decide
                                                  the appropriate time to recalibrate CPPM. Application can recalibrate CPPM when all channels are unused/powered-down.
                                                  For example, the application can call NEXUS_FrontendDevice_Recalibrate() at the next channel change. */
        NEXUS_CallbackDesc calibrationComplete;/* This callback will be called once CPPM is complete. */
    } cppm; /* Cable Plant Power Management(CPPM) measures Composite Power of all input RF signals and adjusts Gains including external LNA. */
} NEXUS_FrontendDeviceRecalibrateSettings;

void NEXUS_FrontendDevice_GetDefaultRecalibrateSettings(
    NEXUS_FrontendDeviceRecalibrateSettings *pSettings
    );

/*
If performance degrades, the board frontend devices may need to be recalibrated for optimal performance.
Internally, this may include triggering the composite plant power measurement.
*/
NEXUS_Error NEXUS_FrontendDevice_Recalibrate(
    NEXUS_FrontendDeviceHandle handle,
    const NEXUS_FrontendDeviceRecalibrateSettings *pSettings
    );

/**
Summary:
Get all the frontend capabilites for the indexed tuner. Asingle tuner can have multiple demod capabilities or could just be used as only a tuner in case of ifdac.
**/
void NEXUS_FrontendDevice_GetTunerCapabilities(
    NEXUS_FrontendDeviceHandle handle,
    unsigned tunerIndex, /* corresponds to NEXUS_FrontendDeviceCapabilities.numTuners */
    NEXUS_FrontendCapabilities *pCapabilities /* capabilities for tuner */
    );

/**
Summary:
Get total number of tuners present on the frontend device.
**/
typedef struct NEXUS_FrontendDeviceCapabilities
{
    unsigned numTuners;
} NEXUS_FrontendDeviceCapabilities;

void NEXUS_FrontendDevice_GetCapabilities(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_FrontendDeviceCapabilities *pCapabilities
    );

/***************************************************************************
Summary:
    MTSIF settings that can be configured by the frontend.

Description:
    Some frontends allow certain MTSIF configuration parameters to be changed.
This is a shared structure for NEXUS_Frontend structures and code to use.  It
is initially targeted at NEXUS_FrontendDevice_OpenXXXX calls for frontends
which support this.
    The details on how this maps to a given frontend's MTSIFx is detailed in
the individual frontend's header.
***************************************************************************/
typedef struct NEXUS_FrontendDeviceMtsifSettings {
    bool enabled;           /* If true, enable this MTSIF channel. Typically defaults to 'true', but can be set to 'false' to disable. */
    unsigned clockRate;     /* in Hz */
    unsigned driveStrength; /* in mA */
} NEXUS_FrontendDeviceMtsifSettings;

/**
Summary:
Get inputband status of a MTSIF frontend
**/
NEXUS_Error NEXUS_Frontend_GetInputBandStatus(
    NEXUS_FrontendHandle frontend,
    NEXUS_InputBandStatus *pStatus /* [out] */
    );


/***************************************************************************
Summary:
    Channel type for a frontend handle

See Also:
    NEXUS_Frontend_Open
***************************************************************************/
typedef enum NEXUS_FrontendChannelType
{
    NEXUS_FrontendChannelType_eCable,
    NEXUS_FrontendChannelType_eCableOutOfBand,
    NEXUS_FrontendChannelType_eTerrestrial,
    NEXUS_FrontendChannelType_eSatellite,
    NEXUS_FrontendChannelType_eMax
} NEXUS_FrontendChannelType;

/***************************************************************************
Summary:
Out of band settings for a frontend handle

See Also:
    NEXUS_FrontendDevice_Open
***************************************************************************/
typedef enum NEXUS_FrontendOutOfBandInput
{
    NEXUS_OutOfBandInput_eNarrowBandAtoD, /* out of band uses its own dedicated analog to digital converter. */
    NEXUS_OutOfBandInput_eWideBandAtoD,   /* out of band uses the wideband analog to digital converter. */
    NEXUS_OutOfBandInput_eBandPassFilteredNarrowBandAtoD, /* out of band uses its own dedicated analog to digital converter.  The signal is also band pass filtered
                                                                 to filter out frequencies <70Mhz as normal out of band range is from 70-130 Mhz. This should help save some power.*/
    NEXUS_OutOfBandInput_eMax
} NEXUS_FrontendOutOfBandInput;

/***************************************************************************
Summary:
Cable-specific settings for a frontend handle

Description:
    When opening a frontend device using the generic function NEXUS_FrontendDevice_Open,
this structure (in NEXUS_FrontendDeviceOpenSettings) contains the settings specific to
a frontend.
    There are also general settings in the main structure which can apply to any of the
frontend types, such as daisy chain configuration.

See Also:
    NEXUS_FrontendDevice_Open,
    NEXUS_FrontendDeviceOpenSettings
***************************************************************************/
typedef struct NEXUS_FrontendCableConfiguration
{
    bool enabled;

    struct {
        unsigned ifFrequency;       /* IF frequency used for OOB module. */
        NEXUS_FrontendOutOfBandInput useWidebandAtoD; /* Specify which A to D converter is used and how. See the enum for more comments. */
        NEXUS_FrontendOutOfBandNyquistFilter nyquist; /* Nyquist filter used for out of band. */
    } outOfBand;

    struct {
        bool enabled;               /* If true, set the pinmux registers withs values as explained in the specific frontend. */
        unsigned data[4];           /* pin mux data to be applied, data[n]=register, data[n+1]=value. */
    } pinmux;
} NEXUS_FrontendCableConfiguration;

/***************************************************************************
Summary:
Terrestrial-specific settings for a frontend handle

Description:
    When opening a frontend device using the generic function NEXUS_FrontendDevice_Open,
this structure (in NEXUS_FrontendDeviceOpenSettings) contains the settings specific to
a terrestrial frontend.
    There are also general settings in the main structure which can apply to any of the
frontend types, such as daisy chain configuration.

See Also:
    NEXUS_FrontendDevice_Open,
    NEXUS_FrontendDeviceOpenSettings
***************************************************************************/
typedef struct NEXUS_FrontendTerrestrialConfiguration
{
    bool enabled;
} NEXUS_FrontendTerrestrialConfiguration;

/***************************************************************************
Summary:
Terrestrial-specific settings for a frontend handle

Description:
    When opening a frontend device using the generic function NEXUS_FrontendDevice_Open,
this structure (in NEXUS_FrontendDeviceOpenSettings) contains the settings specific to
a satellite frontend.
    There are also general settings in the main structure which can apply to any of the
frontend types, such as daisy chain configuration.

See Also:
    NEXUS_FrontendDevice_Open,
    NEXUS_FrontendDeviceOpenSettings
***************************************************************************/
typedef struct NEXUS_FrontendSatelliteConfiguration
{
    bool enabled;

    struct {
        uint16_t i2cAddress;            /* I2C address to communicate with diseqc */
        NEXUS_I2cHandle i2cDevice;      /* I2C device to communicate with diseqc */
    } diseqc;
} NEXUS_FrontendSatelliteConfiguration;

/***************************************************************************
Summary:
Reset GPIO control information for a frontend device handle

Description:
    This structure provides the information to allow the frontend to directly control its own
reset GPIO. This is required for platforms which support S3 standby and the reset line is not
an AON GPIO.
    If this is supported by the frontend and provided to the matching NEXUS_FrontendDevice_Open
call, then the frontend device is responsible for handling the reset GPIO and platform code does
not need to.

See Also:
    NEXUS_FrontendDevice_Open,
    NEXUS_FrontendDeviceOpenSettings
***************************************************************************/
typedef struct NEXUS_FrontendDeviceReset{
    bool enable;           /* If true, reset settings are valid and the frontend device needs to be released out of reset. */
    unsigned type;         /* Upon boot up, these gpio settings control the gpio line that holds the frontend device in hardware reset.
                                                       Using settings.value we can either pull the line high or low to bring the frontend device out of reset as applicable.
                                                       Also, when the system goes into S3 standby, the frontend device needs to be put in reset and brought out of reset upon resuming. */
    unsigned pin;          /*  What gpio pin/number is used to hold the frontend chipe in reset. */
    NEXUS_GpioValue value; /*  Set to low or high depending on what the gpio line needs to be, to bring the frontend chip out of reset. */
} NEXUS_FrontendDeviceReset;

/***************************************************************************
Summary:
    Settings for a frontend device

Description:
    This structure provides the settings when opening a generic frontend device. (A FrontendDeviceHandle
maps to the frontend chip, a FrontendHandle maps to a specific demod or tuner path.)
    The structure can be used for a frontend of any of the supported types (cable, terrestrial,
or satellite) with NEXUS_FrontendDevice_Open, and is required for frontends with multiple capability
(such as 7364).
    It is similar to chip-specific NEXUS_FrontendDevice_OpenXXXX calls.
    To configure the frontend device's interrupt:
        External interrupt: Set isrNumber
        GPIO interrupt: configure a NEXUS_GpioHandle and use gpioInterrupt
    To configure I2C communication:
        Set i2cDevice, i2cAddress.  If required by the platform, configure i2cSlaveAddr.
        Leave spiDevice NULL.
    To configure SPI communication:
        Set spiDevice.
        Leave i2cDevice NULL.
    For integrated frontends:
        Both spiDevice and i2cDevice should be set to NULL.

    The structure should be initialized with NEXUS_FrontendDevice_GetDefaultOpenSettings before
setting any member variables.

    Support for older frontends via the generic open is not guaranteed.

See Also:
    NEXUS_FrontendDevice_GetDefaultOpenSettings,
    NEXUS_FrontendDevice_Open,
    NEXUS_FrontendCableConfiguration,
    NEXUS_FrontendTerrestrialConfiguration,
    NEXUS_FrontendSatelliteConfiguration
 ***************************************************************************/
typedef struct NEXUS_FrontendDeviceOpenSettings
{
    /* General configuration options */
    NEXUS_FrontendDeviceReset reset; /* Information required for controlling the GPIO reset for S3, if necessary. */
    NEXUS_FrontendInterruptMode interruptMode; /* Either GPIO or an L1 can be used for notification via interrupts from the frontend to the host.
                                                  If not, polling can be used instead to check the status of frontend. */
    NEXUS_GpioHandle gpioInterrupt; /* GPIO pin for interrupt. If not NULL, isrNumber is ignored. If NULL, isrNumber is used. */
    unsigned isrNumber;             /* L1 interrupt number.  See gpioInterrupt for other interrupt option. */

    /* either I2C or SPI is used for the host to control the frontend. */
    NEXUS_I2cHandle i2cDevice;      /* I2C device to use. spiDevice should be NULL and this set to use I2C. */
    uint16_t i2cAddress;            /* master device I2C Address */
    uint8_t i2cSlaveAddress;        /* slave device I2C Address */

    NEXUS_SpiHandle spiDevice;      /* SPI device to use. i2cDevice should be NULL and this set to use SPI. */

    bool loadAP;                    /* Load Acquisition Processor microcode when starting. */
    bool configureWatchdog;         /* Depending on the platform decide whether to configure watchdog or not. */

    struct {
        int bypassable;             /* in units of 1/100 db. */
        int total;                  /* includes the bypassable and all non-bypassable fixed gains before this device. in units of 1/100 db. */
    } externalFixedGain;

    struct {
        bool enableDaisyChain;      /* enable clock to the next frontend device. */
    } crystalSettings;

    NEXUS_FrontendDeviceMtsifSettings mtsif[NEXUS_MAX_MTSIF]; /* Configure MTSIF TX enable, rate, and drive strength at open time. If values are 0, defaults are used. */

    NEXUS_CallbackDesc updateGainCallback;  /* Callback will be called when the gain from the lna needs to be updated. */

    /* Cable/Terrestrial configuration options */
    NEXUS_FrontendCableConfiguration cable;

    /* Terrestrial configuration options */
    NEXUS_FrontendTerrestrialConfiguration terrestrial;

    /* Satellite configuration options */
    NEXUS_FrontendSatelliteConfiguration satellite;
} NEXUS_FrontendDeviceOpenSettings;

/***************************************************************************
Summary:
    Get default settings for a frontend device

See Also:
    NEXUS_FrontendDeviceOpenSettings,
    NEXUS_FrontendDevice_Open
***************************************************************************/
void NEXUS_FrontendDevice_GetDefaultOpenSettings(
    NEXUS_FrontendDeviceOpenSettings *pSettings
    );

/***************************************************************************
Summary:
    Open a handle to a frontend device

Description:
    This function opens a frontend device and returns a handle.  The device maps to
the chip, and each chip only needs to be opened once.  It will then be used to open
the individual channels (demods) via NEXUS_Frontend_Open.

See Also:
    NEXUS_FrontendDeviceOpenSettings,
    NEXUS_FrontendDevice_GetDefaultOpenSettings
***************************************************************************/
NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_Open( /* attr{destructor=NEXUS_FrontendDevice_Close} */
    unsigned index,
    const NEXUS_FrontendDeviceOpenSettings *pSettings
    );

/***************************************************************************
Summary:
Power mode configuration of the tuner.
***************************************************************************/
typedef enum NEXUS_TunerRfInput
{
    NEXUS_TunerRfInput_eOff,         /* Tuner is off. */
    NEXUS_TunerRfInput_eExternalLna, /* Tuner Rf input through UHF path. This Rf path does not use internal LNA. */
    NEXUS_TunerRfInput_eInternalLna, /* Tuner Rf input through UHF path. This Rf path does uses internal LNA. */
    NEXUS_TunerRfInput_eStandardIf,  /* Tuner input is not RF. Insted a (36 or 44MHz) standard IF signal is input.  */
    NEXUS_TunerRfInput_eLowIf,       /* Tuner input is not RF. Insted a (4 to 5MHz) Low IF signal is input.  */
    NEXUS_TunerRfInput_eBaseband,    /* Tuner input is baseband.  */
    NEXUS_TunerRfInput_eMax
} NEXUS_TunerRfInput;

/***************************************************************************
Summary:
    Enumeration for Tuner Application

****************************************************************************/
typedef enum NEXUS_RfDaisyChain
{
    NEXUS_RfDaisyChain_eOff,         /* Daisychaining is turned off. */
    NEXUS_RfDaisyChain_eExternalLna, /* Daisychaining through UHF path. This Rf daisychaining path does not use internal LNA. */
    NEXUS_RfDaisyChain_eInternalLna, /* Daisychaining through VHF path. This Rf daisychaining path uses internal LNA. */
    NEXUS_RfDaisyChain_eMax
} NEXUS_RfDaisyChain;

/***************************************************************************
Summary:
Config settings for a BCM Device
***************************************************************************/
typedef struct NEXUS_FrontendDeviceCableSettings
{
    struct {
        NEXUS_FrontendOutOfBandOutputMode outputMode;
    } outOfBand;
} NEXUS_FrontendDeviceCableSettings;

typedef struct NEXUS_FrontendDeviceTerrestrialSettings
{
    bool dummy;
} NEXUS_FrontendDeviceTerrestrialSettings;

typedef enum NEXUS_FrontendLoopthroughGain {
    NEXUS_FrontendLoopthroughGain_eLow,     /* lower gain mode on loopthrough output */
    NEXUS_FrontendLoopthroughGain_eHigh,    /* high gain mode on loopthrough output */
    NEXUS_FrontendLoopthroughGain_eMax
} NEXUS_FrontendLoopthroughGain;

typedef struct NEXUS_FrontendDeviceSettings
{
    NEXUS_TunerRfInput rfInput;              /* Determines how Rf is input to the tuner. */
    NEXUS_RfDaisyChain rfDaisyChain;         /* Determines if the Rf daisychain is on/off/uses internal LNA/ does not use internal LNA. */
    bool enableRfLoopThrough;                /* True = Enables RF loop through. */
    NEXUS_FrontendLoopthroughGain loopthroughGain; /* Loopthrough gain output. This is frontend chip-specific. */
    NEXUS_FrontendDeviceCableSettings cable;

} NEXUS_FrontendDeviceSettings;

/* Device API. */
/***************************************************************************
Summary:
Get the default config settings to a BCM device.
***************************************************************************/
void NEXUS_FrontendDevice_GetDefaultSettings(
    NEXUS_FrontendDeviceSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Get the config settings to a BCM device.
***************************************************************************/
NEXUS_Error NEXUS_FrontendDevice_GetSettings(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_FrontendDeviceSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Set the config settings to a BCM device.
***************************************************************************/
NEXUS_Error NEXUS_FrontendDevice_SetSettings(
    NEXUS_FrontendDeviceHandle handle,
    const NEXUS_FrontendDeviceSettings *pSettings
    );

/***************************************************************************
Summary:
    Settings for a frontend channel

Description:
    These are the settings passed into NEXUS_Frontend_Open, and are used to configure
a specific demod or tuner path.  The device is required to be opened by an earlier
call.
    NEXUS_FrontendChannelType should be set to the appropriate frontend category,
and needs to match the portions of the frontend device which were opened.

See Also:
    NEXUS_FrontendDeviceOpenSettings
 ***************************************************************************/
typedef struct NEXUS_FrontendChannelSettings
{
    NEXUS_FrontendDeviceHandle device;  /* Previously opened device to use */
    NEXUS_FrontendChannelType type;     /* Channel type to open from this device */
    unsigned channelNumber;             /* Which channel of the specified type to open from this device */
} NEXUS_FrontendChannelSettings;

/***************************************************************************
Summary:
    Get the default settings for a frontend channel

Description:
See Also:
    NEXUS_Frontend_OpenChannel,
    NEXUS_FrontendChannelSettings
 ***************************************************************************/
void NEXUS_Frontend_GetDefaultOpenSettings(
    NEXUS_FrontendChannelSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Open a handle to a frontend device.

Description:
    This function opens a channel on a frontend device and returns a frontend handle.
The channel maps to an individual demod of the device (which may contain more than one
demod).  The total number of channels (demods) available can be retrieved with
NEXUS_FrontendDevice_GetCapabilities.

See Also:
    NEXUS_Frontend_Device_Open,
    NEXUS_Frontend_Close
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open(  /* attr{destructor=NEXUS_Frontend_Close} */
    const NEXUS_FrontendChannelSettings *pSettings
    );

/***************************************************************************
Summary:
    Hold the results of a frontend device probe.

Description:
    This structure returns the address of a frontend device probe.

See Also:
    NEXUS_FrontendDevice_Probe
 ***************************************************************************/
typedef struct NEXUS_FrontendProbeResults
{
    NEXUS_FrontendChipType chip;
} NEXUS_FrontendProbeResults;

/***************************************************************************
Summary:
    Probe for the chip identifiers for a frontend device.

Description:
    This function will probe a specific I2C or SPI address to find out what (if any)
frontend chip is available at the address.  The only members of pSettings which are used
are i2cDevice and i2cAddress (to probe I2C), or spiDevice (to probe SPI).

See Also:
    NEXUS_FrontendDevice_Open,
    NEXUS_FrontendProbeResults
 ***************************************************************************/
NEXUS_Error NEXUS_FrontendDevice_Probe(
    const NEXUS_FrontendDeviceOpenSettings *pSettings,
    NEXUS_FrontendProbeResults *pResults    /* [out] */
    );


/***************************************************************************
Summary:
    Enum to represent possible states for a frontend device amplifier.

Description:
    This enum represents the amplifier state.
 ***************************************************************************/
typedef enum NEXUS_FrontendDeviceAmplifierState
{
    NEXUS_FrontendDeviceAmplifierState_eBypass,
    NEXUS_FrontendDeviceAmplifierState_eEnabled,
    NEXUS_FrontendDeviceAmplifierState_eMax
} NEXUS_FrontendDeviceAmplifierState;

/***************************************************************************
Summary:
    Structure to hold the amplifier status for a frontend device.

Description:
    This structure provides amplifier status.
 ***************************************************************************/
typedef struct NEXUS_FrontendDeviceAmplifierStatus {
    NEXUS_FrontendDeviceAmplifierState externalFixedGain;
} NEXUS_FrontendDeviceAmplifierStatus;

/***************************************************************************
Summary:
    Retrieve amplifier state information for a frontend device.

Description:
    This function reads the amplifier status from a given frontend device.
 ***************************************************************************/
NEXUS_Error NEXUS_FrontendDevice_GetAmplifierStatus(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_FrontendDeviceAmplifierStatus *pStatus
    );

#include "nexus_transport_wakeup.h"
/***************************************************************************
Summary:
    Read the wakeup packet settings for a frontend device.

Description:
    This function allows the caller to read the current wakeup packet configuration
for a given frontend device.

See Also:
    nexus_transport_wakeup.h
    NEXUS_FrontendDevice_SetWakeupSettings
 ***************************************************************************/
void NEXUS_FrontendDevice_GetWakeupSettings(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_TransportWakeupSettings *pSettings
    );

/***************************************************************************
Summary:
    Configure the wakeup packet settings for a frontend device.

Description:
    This function allows the caller to set or clear the wakeup packet configuration
for a given frontend device. To properly enable backend wakeup in S3 requires that
the board be designed to support it, that the frontend device support it, and that
the frontend device has only the tuner which will be listening tuned (and all other
tuners untuned) prior to entering S3. The wakeup will be returned on a GPIO interrupt,
and entering S3 requires the xpt and gpio flags to both be set.
    There are board design considerations regarding what is left powered when the
frontend has power and the backend is in S3, and platform code must configure a
power mode transition callback in the frontend in order to make any board-level
changes required.
    Incorrectly handling what is on/off can result in damage to the backend or
or frontend pins (e.g. i2c/spi, mtsif).
    Only some frontends support this feature.

See Also:
    nexus_transport_wakeup.h
    NEXUS_FrontendDevice_GetWakeupSettings
 ***************************************************************************/
NEXUS_Error NEXUS_FrontendDevice_SetWakeupSettings(
    NEXUS_FrontendDeviceHandle handle,
    const NEXUS_TransportWakeupSettings *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_H__ */

