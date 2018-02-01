/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/

/*= Module Overview ************************************************************
<verbatim>

Overview
The RF Modulator (RFM) converts a NTSC/PAL compliant digital video source
and a Pulse Code Modulated (PCM) audio source into an analog composite
television signal which is suitable for demodulation by a television
demodulator. The digital video source (video input into RFM) is assumed
to come from Video Encoder (VEC) core. The PCM audio source
(audio input into RFM) is assumed to come from HiFiDAC core.


Design
The design for BRFM PI API is broken into two parts.

o Part 1 (open/close/configuration):

    These APIs are used for opening and closing BRFM device/device channel.
    When a device/device channel is opened, the device/device channel can be
    configured.

o Part 2 (get/set):

    These APIs are used for getting and setting runtime attributes of BRFM.


Usage
The usage of BRFM involves the following:

   * Configure/Open of BRFM

      * Configure BRFM device for the target system
      * Open BRFM device

   * Program device channel

      * Program output channel
      * Program output modulation type
      * Program enable output


Interrupt Requirements
This module requires L2 interrupts. They are handled internally 
by the RFM PI.


Sample Code

//
// NOTE: The following sample code does not do any error checking.
//
// In this example, the target H/W board is configured for NTSC application.
//
static BRFM_Handle hRfmDev;
static BCHP_Handle hChip7038;
static BREG_Handle hReg7038;
static BINT_Handle hInt7038;

main( void )
{
    BRFM_Settings rfmDevSettings;

    // Initialize hChip7038, hReg7038, and hInt7038...

    // *************************************************************************
    // Start of H/W board specific configuration for BRFM
    // *************************************************************************

    // Configure BRFM Device
    BRFM_GetDefaultSettings(&rfmDevSettings, hChip7038);
    rfmDevSettings.audioEncoding = BRFM_AudioEncoding_eStereo;

    // Open BRFM Device
    BRFM_Open(hRfmDev, hChip7038, hReg7038, hInt7038, &rfmDevSettings);

    // *************************************************************************
    // End of H/W board specific configuration for BRFM
    // *************************************************************************

    // Setup Rf Modulator to use Channel 3 and NTSC Open Cable modulation
    BRFM_SetModulationType(hRfmDev, BRFM_ModulationType_eNtscOpenCable, BRFM_OutputChannel_eCh3);

    while(1)
    {
        BRFM_Status status;

        // Check status every 5 seconds
        BKNI_Sleep(5000);
        BRFM_GetStatus(hRfmDev, &status);
        printf("  OutputChannel=%d, mute=%d\n", status.outputChannel+3, status.mute);
        printf("  rfmStatusSignals=%08x\n", status.rfmStatusSignals);
    }
}

</verbatim>
*******************************************************************************/

#ifndef BRFM_H__
#define BRFM_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "berr_ids.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
Summary:
    Error Codes specific to BRFM
*******************************************************************************/
#define BRFM_ERR_VOLUME_RANGE    BERR_MAKE_CODE(BERR_RFM_ID, 0)
#define BRFM_ERR_CARR_AMPL_RANGE BERR_MAKE_CODE(BERR_RFM_ID, 0)

/*******************************************************************************
Summary:
    Max and Min amplitude for NICAM carrier
*******************************************************************************/
#define BRFM_NICAM_CARR_AMPL_MAX BRFM_NicamCarrierAmpl_eNeg13p5
#define BRFM_NICAM_CARR_AMPL_MIN BRFM_NicamCarrierAmpl_eNeg37p0

/*******************************************************************************
Summary:
    Audio Video Ratio Selection Values
*******************************************************************************/
#define BRFM_AUDIO_VIDEO_RATIO_17 (-1700)
#define BRFM_AUDIO_VIDEO_RATIO_16 (-1600)
#define BRFM_AUDIO_VIDEO_RATIO_15 (-1500)
#define BRFM_AUDIO_VIDEO_RATIO_14 (-1400)

/*******************************************************************************
Summary:
    The handles for Rf Modulator module.

Description:
    An opaque handle for BRFM device.

See Also:
    BRFM_Open()
*******************************************************************************/
typedef struct BRFM_P_Handle *BRFM_Handle;


/*******************************************************************************
Summary:
    Enumeration for Rf Modulator audio/video output channel.

Description:
    This enumeration defines the output channel (Ch. 3 or Ch. 4).

See Also:
    BRFM_SetModulationType(), BRFM_GetStatus()
*******************************************************************************/
typedef enum BRFM_OutputChannel
{
    /* Do not change the order of this enumeration; tables are dependent on this order */
    BRFM_OutputChannel_eBaseband, /* Baseband bypass, BTSC routed to baseband output. Baseband audio only */
    BRFM_OutputChannel_eCh3,      /* Channel 3 */
    BRFM_OutputChannel_eCh4,      /* Channel 4 */
    BRFM_OutputChannel_eSIF,      /* Sound Intermediate Frequency mode. Frequency modulated audio only */
    BRFM_OutputChannel_eLast
} BRFM_OutputChannel;

/*******************************************************************************
Summary:
    Enumeration for output modulation type.

Description:
    This enumeration defines the modulation type, not all are supported.

    The following information is from "Video Demystified Third Edition"

    NSTC by countries:
    The following countries use the (M) NTSC standard.
        Antigua             Japan (NTSCJ, color-space conversion is different)
        Aruba               South Korea
        Bahamas             Mexico
        Barbados            Montserrat
        Belize              Myanmar
        Bermuda             Nicaragua
        Bolivia             Panama
        Canada              Peru
        Chile               Philippines
        Colombia            Puerto Rico
        Costa Rica          St. Kitts and Nevis
        Cuba                Samoa
        Curacao             Suriname
        Taiwan              Dominican Republic
        Ecuador             Trinidad/Tobago
        El Salvador         United States of America
        Guatemala           Venezuela
        Honduras            Virgin Islands
        Guam                Jamaica

    PAL by countries:
    The following countries use the (N) PAL standard. Note that Argentina
    uses a modified PAL standard, called N C .
        Argentina           Paraguay
        Aryenuna            Uruguay
    The following countries use the (M) PAL standard.
        Brazil
    The following countries use the (B) PAL standard.
        Australia           Indonesia
        Bangladesh          Maldives
        Belgium             Malta
        Nigeria             Brunei Darussalam
        Estonia             Rwandese Republic
        Ghana               Sao Tome & Principe
        India               Seychelles
    The following countries use the (G) PAL standard.
        Hungary             Zambia
        Mozambique          Zimbabwe
        Romania
    The following countries use the (D) PAL standard.
        China               Latvia
        Czech Republic      Poland
        Hungary             Romania
        North Korea
    The following countries use the (I) PAL standard.
        Angola              Malawi
        Botswana            Namibia
        China               Nigeria
        Gambia              South Africa
        Guinea-Bissau       Tanzania
        Ireland             United Kingdom
        Lesotho             Zanzibar
        Macau
    The following countries use the (B) and (G) PAL standards.
        Albania             Kenya
        Algeria             Kuwait
        Austria             Liberia
        Bahrain             Libya
        Cambodia            Lithuania
        Cameroon            Luxembourg
        Croatia             Malaysia
        Cyprus              Netherlands
        Denmark             New Zealand
        Egypt               Norway
        Equatorial Guinea   Oman
        Ethiopia            Pakistan
        Finland             Papua New Guinea
        Germany             Portugal
        Iceland             Qatar
        Israel              Sierra Leone
        Italy               Singapore
        Jordan              Slovenia
        Somalia             Syria
        Spain               Thailand
        Sri Lanka           Turkey
        Sudan               Yemen
        Sweden              Yugoslavia
        Switzerland
    The following countries use the (H) PAL standard.
        Belgium

    SECAM by countries:
    The following countries use the (B) and (G) SECAM standards.
        Greece              Mauritius
        Iran                Morocco
        Iraq                Saudi Arabia
        Lebanon             Tunisia
        Mali
    The following countries use the (D) and (K) SECAM standards.
        Azerbaijan          Lithuania
        Belarus             Moldova
        Bulgaria            Russia
        Georgia             Ukraine
        Kazakhstan          VietNam
        Latvia
    The following countries use the (B) SECAM standard.
        Mauritania          Djibouti
    The following countries use the (D) SECAM standard.
        Afghanistan         Mongolia

See Also:
    BRFM_SetModulationType(), BRFM_GetStatus()
*******************************************************************************/
typedef enum BRFM_ModulationType
{
    /* Do not change the order of this enumeration; tables are dependent on this order */
    BRFM_ModulationType_eNtscCustom = 0,     /* Not supported */
    BRFM_ModulationType_ePalCustom = 1,      /* Not supported */
    BRFM_ModulationType_eNtscOpenCable = 10, /* Supported */

    BRFM_ModulationType_eSecamBG = 30,       /* Not supported */
    BRFM_ModulationType_eSecamDK = 31,       /* Not supported */
    BRFM_ModulationType_eSecamK1 = 32,       /* Not supported */

    BRFM_ModulationType_ePalBG = 40,         /* Supported */
    BRFM_ModulationType_ePalD = 41,          /* Supported */
    BRFM_ModulationType_ePalH = 42,          /* Not supported */
    BRFM_ModulationType_ePalI = 43,          /* Supported, but only in baseband */
    BRFM_ModulationType_ePalM = 44,          /* Supported */
    BRFM_ModulationType_ePalN = 45,          /* Supported */
    BRFM_ModulationType_ePalDChina = 46      /* Supported */
} BRFM_ModulationType;

/*******************************************************************************
Summary:
    Enumeration for Rf Modulator audio encoding.

Description:
    This enumeration defines the audio encoding.
    Audio encoding supports BTSC analog standard.  BTSC analog standard
    contains three BTSC variations: BTSC-USA, BTSC-EIA-J, and BTSC-Zweiton-M.
    Rf Modulator only support BTSC-USA.  Countries that use BTSC USA
    standard are: United States, Canada, Mexico, Brazil, and Taiwan.
    BTSC-EIA-J is used in Japan, BTSC-Zweiton-M is used in South Korea.

    Pal audio encoding only support mono.

    The following table highlighting the recommend settings for Rfm based on
    inputs into the Rfm.


                             BTSC         Rfm Audio          Rfm            Rfm Audio
      Input      Signal     Decoder      Input Ch.        Mode of        Output Spectrum
    Standard      Type        Mode         A    B         Operation      L+R  L-R  SAP
    ----------------------------------------------------------------------------------
    NTSC-USA    Digital    Don't Care     L    R        Stereo           L+R  L-R
    NTSC-USA    Analog     Stereo         L    R        Stereo           L+R  L-R
    NTSC-USA    Analog     Mono          L+R  L+R       Mono             L+R
 ***NTSC-USA    Analog     Mono+SAP      L+R  SAP       Sap and Mono     L+R       SAP
    NTSC-USA    Analog     SAP Only      SAP  SAP       Sap                        SAP 

    NTSC-Japan  Digital    Don't Care     L    R        Japanese Mono2   L+R
    NTSC-Japan  Analog     Stereo         L    R        Japanese Mono2   L+R
    NTSC-Japan  Analog     Main          main main      Japanese Mono1   main
    NTSC-Japan  Analog     Main Sub      main sub       Japanese Mono1   main
    NTSC-Japan  Analog     Sub           sub  sub       Japanese Mono1   sub

    PAL         Digital    Don't Care     L    R        Mono             L+R
    PAL         Analog     Mono          L+R  L+R       Mono             L+R

    Rfm Mono Audio Encoding uses:
        75us, 25kHz: NTSC-USA and NTSC-Japan
        50us, 50kHz: PAL

    ====================== Application Notes ==================================
    *** Note:It is recommended NOT to use Mono+SAP mode of BTSC Decoder.
    From a STB System point-of-view, Mono+SAP mode can cause problems.  If you
    use Mono+SAP mode, the STB System software must make sure to route either 
    main audio program (Mono component) or secondary audio program (SAP component)
    to the audio DAC.  Only one of the audio program can be routed to audio DAC,
    otherwise the output on the audio DAC will be unpredictable.  If you do use
    Mono+SAP mode, which we don't recommend, then Rfm should be set to
    BRFM_AudioEncoding_eSapMono.
    -------------------- For NTSC-USA applications: ---------------------------
    We recommend keeping Rfm Audio Encoding in BRFM_AudioEncoding_eStereo mode
    for NTSC-USA applications.  In this mode, Rfm output would work for
    Digital Signal and Analog Signal (BTSC Stereo, BTSC Mono, BTSC SAP only).
    -------------------- For NTSC-Japan applications: --------------------------
    We recommend keeping Rfm Audio Encoding in BRFM_AudioEncoding_eJpMono2 mode.
    -------------------- For PAL applications: ---------------------------------
    We recommend keeping Rfm Audio Encoding in BRFM_AudioEncoding_eMono mode.


    BTSC_CTRL0_    BAUD BTSC                           BRFM BTSC
    BASIC_CTRL     Decoder Setting                     Encoder Setting
    ---------------------------------------------------------------------------
    [18:16]=0      BAUD_BTSC_USOutputMode_eMono        BRFM_AudioEncoding_eMono
    [18:16]=1      BAUD_BTSC_USOutputMode_eStereo      BRFM_AudioEncoding_eStereo
    [18:16]=6      BAUD_BTSC_USOutputMode_eSap         BRFM_AudioEncoding_eSap
    [18:16]=7      BAUD_BTSC_USOutputMode_eSapMono     BRFM_AudioEncoding_eSapMono
    [15:14]=0      BAUD_BTSC_JpOutputMode_eMain        BRFM_AudioEncoding_eJpMono1
    [15:14]=1      BAUD_BTSC_JpOutputMode_eStereo      BRFM_AudioEncoding_eJpMono2
    [15:14]=2      BAUD_BTSC_JpOutputMode_eSub         BRFM_AudioEncoding_eJpMono1
    [15:14]=3      BAUD_BTSC_JpOutputMode_eMainSub     BRFM_AudioEncoding_eJpMono1


See Also:
    BRFM_SetAudioEncoding(), BRFM_GetStatus()
*******************************************************************************/
typedef enum BRFM_AudioEncoding
{
    /* Do not change the order of this enumeration; tables are dependent on this order
       Note that Nicam audio encoding modes are not supported for 97038 and 97401 platforms */
    BRFM_AudioEncoding_eMono,               /* Monaural transmission of 2nd audio channel */
    BRFM_AudioEncoding_eStereo,             /* BTSC stereo encoding & transmission of 2nd audio channel */
    BRFM_AudioEncoding_eStereoExtDeviation, /* BTSC stereo with extended deviation */
    BRFM_AudioEncoding_eSap,                /* BTSC SAP encoding, see above for more details */
    BRFM_AudioEncoding_eSapMono,            /* Mono and BTSC SAP encoding, see above for more details */
    BRFM_AudioEncoding_eJpMono1,            /* Japanese Mono1, see above for more details */
    BRFM_AudioEncoding_eJpMono2,            /* Japanese Mono2, see above for more details */
    BRFM_AudioEncoding_eNicamStereo,        /* NICAM Stereo encoding */
    BRFM_AudioEncoding_eNicamDualMono,      /* NICAM Dual Mono encoding mode */
    BRFM_AudioEncoding_eNicamMono,          /* NICAM Mono encoding mode */
    BRFM_AudioEncoding_eLast
} BRFM_AudioEncoding;

/*******************************************************************************
Summary:
    Enumeration for BTSC encoder type

Description:
    This enumeration defines the two encoder types that can be used.
*******************************************************************************/
typedef enum BRFM_EncoderType
{
    BRFM_EncoderType_singleBTSC, /* First BTSC-encoder */
    BRFM_EncoderType_dualBTSC    /* Second BTSC-encoder for dual RFM encoder chips. 
                                    Only supports baseband and SIF audio, no NICAM support */
} BRFM_EncoderType;

/*******************************************************************************
Summary:
    Enumeration for NICAM carrier amplitude level

Description:
    This enumeration defines the different amplitudes that can be used to set NICAM 
    carrier levels.

See Also:
    BRFM_SetNicamCarrierLevel(), BRFM_GetStatus()
*******************************************************************************/
typedef enum BRFM_NicamCarrierAmpl
{
    BRFM_NicamCarrierAmpl_eNeg37p0 = 1,
    BRFM_NicamCarrierAmpl_eNeg31p0,
    BRFM_NicamCarrierAmpl_eNeg27p5,
    BRFM_NicamCarrierAmpl_eNeg25p0,
    BRFM_NicamCarrierAmpl_eNeg23p0,
    BRFM_NicamCarrierAmpl_eNeg21p5,
    BRFM_NicamCarrierAmpl_eNeg20p0,
    BRFM_NicamCarrierAmpl_eNeg19p0,
    BRFM_NicamCarrierAmpl_eNeg18p0,
    BRFM_NicamCarrierAmpl_eNeg17p0,
    BRFM_NicamCarrierAmpl_eNeg16p0,
    BRFM_NicamCarrierAmpl_eNeg15p5,
    BRFM_NicamCarrierAmpl_eNeg14p5,
    BRFM_NicamCarrierAmpl_eNeg14p0,
    BRFM_NicamCarrierAmpl_eNeg13p5
} BRFM_NicamCarrierAmpl;

/*******************************************************************************
Summary:
    This structure represents the Config Settings for RFM module.

Description:
    This structure represents the Config Settings for RFM module.

See Also:
    BRFM_GetConfigSettings(), BRFM_SetConfigSettings()
*******************************************************************************/
typedef struct BRFM_ConfigSettings
{
    int audioVideoRatio; /* RFM Audio-Video Ratio selection: In units of 1/100th dB.
                            For instance: -1700 is -17.00 and -1650 is -16.50.
                            [-17,-16,-15,-14] dB supported for NTSC only */
}BRFM_ConfigSettings;

/*******************************************************************************
Summary:
    This structure represents the RFM Status for a Rf Modulator module.

Description:
    This structure is returned when BRFM_GetStatus() is called.  This
    structure contains the complete status of Rf Modulator module.

See Also:
    BRFM_GetStatus()
*******************************************************************************/
typedef struct BRFM_Status
{
    BRFM_OutputChannel outputChannel; /* Output Channel number */
    BRFM_ModulationType modType;      /* Modulation Type */
    BRFM_AudioEncoding audioEncoding; /* Audio Encoding mode */
    bool isMuteEnable;                /* Enable=1, Disable=0 */
    bool isOutputEnable;              /* Enable=1, Disable=0 */
    bool isPowerSaverEnable;          /* Enable=1, Disable=0 */
    int volume;                       /* in 1 dB steps */
    uint32_t rfmStatusSignals;
} BRFM_Status;

/*******************************************************************************
Summary:
    Required default settings structure for Rf Modulator module.

Description:
    The default setting structure defines the default configuration of
    Rf Modulator when the device is opened.

See Also:
    BRFM_Open()
*******************************************************************************/
#define BRFM_SETTINGS_MODTYPE       (BRFM_ModulationType_eNtscOpenCable)
#define BRFM_SETTINGS_CHNBR         (BRFM_OutputChannel_eCh3)
#define BRFM_SETTINGS_AUDIOENCODING (BRFM_AudioEncoding_eStereo)
#define BRFM_SETTINGS_VOLUME        (6)
#define BRFM_SETTINGS_ENCTYPE       (BRFM_EncoderType_singleBTSC)

typedef struct BRFM_Settings
{
    BRFM_ModulationType modType;      /* Modulation Type */
    BRFM_OutputChannel chNbr;         /* Channel number to use */
    BRFM_AudioEncoding audioEncoding; /* Audio Encoding mode */
    int volume;                       /* in 1 dB steps */
    BRFM_EncoderType encType;         /* Encoder type (first or second BTSC encoder) */
}  BRFM_Settings;

/*******************************************************************************
Summary:
    This function opens Rf Modulator module.

Description:
    This function is responsible for opening BRFM module. When BRFM is
    opened, it will create a module handle and configure the module based
    on the default settings. Once the device is opened, it must be closed
    before it can be opened again.

See Also:
    BRFM_Close(), BRFM_GetDefaultSettings()
*******************************************************************************/
BERR_Code BRFM_Open(
    BRFM_Handle *phRfm,               /* [out] Returns handle */
    BCHP_Handle hChip,                /* [in] Chip handle */
    BREG_Handle hRegister,            /* [in] Register handle */
    BINT_Handle hInterrupt,           /* [in] Interrupt handle */
    const BRFM_Settings *pDefSettings /* [in] Default settings */
    );

/*******************************************************************************
Summary:
    This function closes Rf Modulator module.

Description:
    This function is responsible for closing BRFM module. Closing BRFM 
    will free BRFM handle.

See Also:
    BRFM_Open()
*******************************************************************************/
void BRFM_Close(
    BRFM_Handle hDev /* [in] Device handle */
    );

/*******************************************************************************
Summary:
    This function returns the default settings for Rf Modulator module.

Description:
    This function is responsible for returning the default setting for 
    BRFM module. The returning default setting should be when
    opening the device.

See Also:
    BRFM_Open()
*******************************************************************************/
void BRFM_GetDefaultSettings(
    BRFM_Settings *pDefSettings, /* [out] Returns default setting */
    BCHP_Handle hChip            /* [in] Chip handle */
    );

/*******************************************************************************
Summary:
    This function enables output of Rf Modulator module.

Description:
    This function is responsible for enabling the Rf Modulator. Enabling
    the output will using the current setting, such as modulation type,
    output channel, etc.

See Also:
    BRFM_DisableRfOutput()
*******************************************************************************/
BERR_Code BRFM_EnableRfOutput(
    BRFM_Handle hDev /* [in] Device handle */
    );

/*******************************************************************************
Summary:
    This function disables output of Rf Modulator module.

Description:
    This function is responsible for disabling the Rf Modulator. Disabling
    the output will shutdown Rf output of the Rf Modulator.

    Calling this function will disable the Rf output, disabling both
    video and audio outputs.
    
See Also:
    BRFM_EnableRfOutput()
*******************************************************************************/
void BRFM_DisableRfOutput(
    BRFM_Handle hDev /* [in] Device handle */
    );

/*******************************************************************************
Summary:
    This function sets the current audio output volume of Rf Modulator
    module.

Description:
    This function is responsible for setting the current audio output
    volume of BRFM module. Volume changes are adjusted using a ramping
    function.

    Note: Volume setting is relative to the audio source, therefore
          the 0 dB setting is 0 dB from the audio source.  Audio source
          for BRFM comes from HiFiDAC core.

See Also:
    BRFM_GetStatus(), BRFM_SetAudioMute()
*******************************************************************************/
BERR_Code BRFM_SetAudioVolume(
    BRFM_Handle hDev, /* [in] Device handle */
    int volume        /* [in] Requested audio volume, in 1 dB steps */
    );

/*******************************************************************************
Summary:
    This function sets the current audio mute mode of Rf Modulator module.

Description:
    This function is responsible for setting the current audio mute mode
    BRFM module. Mute changes are adjusted using a ramping function.
    
See Also:
    BRFM_GetStatus()
*******************************************************************************/
BERR_Code BRFM_SetAudioMute(
    BRFM_Handle hDev, /* [in] Device handle */
    bool mute         /* [in] 0=disable mute, 1=enable mute */
    );

/*******************************************************************************
Summary:
    This function sets the current audio encoding of Rf Modulator module.

Description:
    This function is responsible for setting the current audio encoding mode
    BRFM module.  The selected encoding mode should match the source of
    audio channel.  If the audio source to Rf Modulator is outputting
    Stereo, then Rf Modulator audio encoding should also be set to Stereo.
    Please BRFM_AudioEncoding description for a more detail discussion
    regarding audio input and output.

See Also:
    BRFM_GetStatus(), BRFM_AudioEncoding.
*******************************************************************************/
BERR_Code BRFM_SetAudioEncoding(
    BRFM_Handle hDev,                /* [in] Device handle */
    BRFM_AudioEncoding audioEncoding /* [in] Requested audio encoding */
    );

/*******************************************************************************
Summary:
    This function sets the current modulation type of Rf Modulator module.

Description:
    This function is responsible for setting the current modulation type of
    BRFM module. When setting the modulation type, one must also provide the
    channel number to use.

    NOTE: If BRFM output is disabled, this function will enable BRFM output.
    
See Also:
    BRFM_GetStatus(), BRFM_SetAudioMute(), BRFM_SetAudioVolume(),
    BRFM_EnableRfOutput(), BRFM_DisableRfOutput()
*******************************************************************************/
BERR_Code BRFM_SetModulationType(
    BRFM_Handle hDev,            /* [in] Device handle */
    BRFM_ModulationType modType, /* [in] Requested modulation type */
    BRFM_OutputChannel chNbr     /* [in] Requested channel number */
    );

/*******************************************************************************
Summary:
    This function enable the power-saver mode.

Description:
    This function is responsible for enabling the RFM power-saver mode.  
    To exist power-saver mode, calling any of the "setting" functions in 
    BRFM will disable power-saver mode with one exception, calling 
    BRFM_DisableRfOutput() will not disable power-saver mode automatically.
        
See Also:
    BRFM_SetAudioMute(), BRFM_SetAudioVolume(), BRFM_SetAudioEncoding(),
    BRFM_EnableRfOutput(), BRFM_DisableRfOutput()
*******************************************************************************/
BERR_Code BRFM_EnablePowerSaver(
    BRFM_Handle hDev /* [in] Device handle */
    );

/*******************************************************************************
Summary:
    Enter standby mode
*******************************************************************************/
void BRFM_Standby(
    BRFM_Handle hDev /* [in] Device handle */
    );

/*******************************************************************************
Summary:
    Resume from standby mode
********************************************************************************/
void BRFM_Resume(
    BRFM_Handle hDev /* [in] Device handle */
    );

/*******************************************************************************
Summary:
    This function gets the current Rf Modulator status.

Description:
    This function is responsible for getting the current Rf Modulator status.

See Also:
    BRFM_Status
*******************************************************************************/
void BRFM_GetStatus(
    BRFM_Handle hDev,   /* [in] Device handle */
    BRFM_Status *status /* [out] Returns status */
    );

/*******************************************************************************
Summary:
    This function sets the RFM audio standard register

Description:
    This function is responsible for setting the correct audio standard (Mono, Nicam, etc.)

See Also:
    BRFM_SetAudioEncoding
*******************************************************************************/
BERR_Code BRFM_SetAudioStandard(
    BRFM_Handle hDev,
    BRFM_AudioEncoding audioEncoding);

/*******************************************************************************
Summary:
    This function sets the NICAM carrier amplitude values

Description:
    This function is responsible for setting the amplitude for NICAM carrier

See Also:
    BRFM_SetNicamVolume
*******************************************************************************/
BERR_Code BRFM_SetNicamCarrierLevel(
    BRFM_Handle hDev,
    uint32_t eNicamCarrAmpl);

/*******************************************************************************
Summary:
    This function set the volume for NICAM encoder

Description:
    This function is responsible for setting the volume scaling at input to NICAM audio encoder.

See Also:
    BRFM_SetNicamCarrierLevel
*******************************************************************************/
BERR_Code BRFM_SetNicamVolume(
    BRFM_Handle hDev,
    int volumeDbLeft,
    int volumeDbRight);

/*******************************************************************************
Summary:
    This function returns the config settings for RFM module.

Description:
    This function is responsible for returning the config settings for
    RFM module.

See Also:
    BRFM_SetConfigSettings()
*******************************************************************************/
void BRFM_GetConfigSettings(
    BRFM_Handle hDev,
    BRFM_ConfigSettings *pConfigSettings);

/*******************************************************************************
Summary:
    This function sets the config settings for RFM module.

Description:
    This function is responsible for setting the config settings for
    RFM module.

See Also:
    BRFM_GetConfigSettings()
*******************************************************************************/
BERR_Code BRFM_SetConfigSettings(
    BRFM_Handle hDev,
    const BRFM_ConfigSettings *pConfigSettings);

#ifdef __cplusplus
}
#endif
 
#endif /* #ifndef BRFM_H__ */
