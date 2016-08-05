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
#include "nexus_audio_types.h"
#include "nexus_video_types.h"

#ifndef NEXUS_IFD_H__
#define NEXUS_IFD_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
IF Demodulator Handle
***************************************************************************/
typedef struct NEXUS_Ifd *NEXUS_IfdHandle;

/***************************************************************************
Summary:
IF Modulation Types
***************************************************************************/
typedef enum NEXUS_IfdInputType
{
    NEXUS_IfdInputType_eNone,       /* Default, Audio and Video carried on IF input */
    NEXUS_IfdInputType_eIf,         /* Default, Audio and Video carried on IF input */
    NEXUS_IfdInputType_eSif,        /* Sound IF - Audio input typically from an off-chip IF Demodulator */
    NEXUS_IfdInputType_eBaseband,   /* Baseband signal, not modulated.  Data will bypass IF Demodulator */
    NEXUS_IfdInputType_eMax
} NEXUS_IfdInputType;

/***************************************************************************
Summary:
IF Modulation Types
***************************************************************************/
typedef enum NEXUS_IfdAudioMode
{
    NEXUS_IfdAudioMode_eUs,              /* BTSC US */
    NEXUS_IfdAudioMode_eKorea,           /* Korea (A2) */
    NEXUS_IfdAudioMode_eJapan,           /* Japan CPZ503 */
    NEXUS_IfdAudioMode_eIndia,           /* India High Deviation FM Mono */
    NEXUS_IfdAudioMode_eNicam,           /* NICAM */
    NEXUS_IfdAudioMode_ePalA2,           /* PAL A2 */
    NEXUS_IfdAudioMode_eAutoNicamPalA2,  /* Auto switch for NICAM and PAL-A2*/
    NEXUS_IfdAudioMode_eMax
} NEXUS_IfdAudioMode;

/***************************************************************************
Summary:
US RF audio decoder modes
***************************************************************************/
typedef enum NEXUS_UsIfdAudioMode
{
    NEXUS_UsIfdAudioMode_eMono,       
    NEXUS_UsIfdAudioMode_eStereo,     
    NEXUS_UsIfdAudioMode_eSap,        
    NEXUS_UsIfdAudioMode_eSapMono,
    NEXUS_UsIfdAudioMode_eMax
} NEXUS_UsIfdAudioMode;

/***************************************************************************
Summary:
Korea A2 RF audio decoder modes
***************************************************************************/
typedef enum NEXUS_KoreaIfdAudioMode
{
    NEXUS_KoreaIfdAudioMode_eMono,    /* Mono - L=Main, R=Main */
    NEXUS_KoreaIfdAudioMode_eStereo,  /* Stereo - L=L, R=R */
    NEXUS_KoreaIfdAudioMode_eSub,     /* Special Mode - L=Sub, R=Sub */
    NEXUS_KoreaIfdAudioMode_eDualMono,/* Dual Mono - L=Main, r=Sub */
    NEXUS_KoreaIfdAudioMode_eMax
} NEXUS_KoreaIfdAudioMode;

/***************************************************************************
Summary:
Japan RF audio decoder modes
***************************************************************************/
typedef enum NEXUS_JapanIfdAudioMode
{
    NEXUS_JapanIfdAudioMode_eMono,    /* Mono - L=Main, R=Main */
    NEXUS_JapanIfdAudioMode_eStereo,  /* Stereo - L=L, R=R */
    NEXUS_JapanIfdAudioMode_eSub,     /* Special Mode - L=Sub, R=Sub */
    NEXUS_JapanIfdAudioMode_eDualMono,/* Dual Mono - L=Main, r=Sub */
    NEXUS_JapanIfdAudioMode_eMax
} NEXUS_JapanIfdAudioMode;

/***************************************************************************
Summary:
NICAM RF audio decoder modes
***************************************************************************/
typedef enum NEXUS_NicamIfdAudioMode
{
    NEXUS_NicamIfdAudioMode_eFmAmMono,      /* AM or FM mono audio */
    NEXUS_NicamIfdAudioMode_eMono,          /* NICAM Mono - L=Main, R=Main */
    NEXUS_NicamIfdAudioMode_eStereo,        /* NICAM Stereo - L=L, R=R */
    NEXUS_NicamIfdAudioMode_eDualMono1,     /* NICAM Dual Mono - L=Main, r=Main */
    NEXUS_NicamIfdAudioMode_eDualMono2,     /* NICAM Dual Mono - L=Sub, r=Sub */
    NEXUS_NicamIfdAudioMode_eDualMono1And2, /* NICAM Dual Mono - L=Main, r=Sub */
    NEXUS_NicamIfdAudioMode_eMax
} NEXUS_NicamIfdAudioMode;

/***************************************************************************
Summary:
PAL A2 RF audio decoder modes
***************************************************************************/
typedef enum NEXUS_PalA2IfdAudioMode
{
    NEXUS_PalA2IfdAudioMode_eMono,    /* Mono - L=Main, R=Main */
    NEXUS_PalA2IfdAudioMode_eStereo,  /* Stereo - L=L, R=R */
    NEXUS_PalA2IfdAudioMode_eSub,     /* Special Mode - L=Sub, R=Sub */
    NEXUS_PalA2IfdAudioMode_eDualMono,/* Dual Mono - L=Main, r=Sub */
    NEXUS_PalA2IfdAudioMode_eMax
} NEXUS_PalA2IfdAudioMode;

/***************************************************************************
Summary:
IFD Audio deviation settings
***************************************************************************/
typedef enum NEXUS_IfdAudioDeviation 
{
    NEXUS_IfdAudioDeviation_eNormal,
    NEXUS_IfdAudioDeviation_eMedium,
    NEXUS_IfdAudioDeviation_eHigh,
    NEXUS_IfdAudioDeviation_eMax
} NEXUS_IfdAudioDeviation;

/***************************************************************************
Summary:
IFD Pull In Range
***************************************************************************/
typedef enum NEXUS_IfdPullInRange
{
    NEXUS_IfdPullInRange_eDefault,      /* Use defaults from the hardware */
    NEXUS_IfdPullInRange_e32kHz,
    NEXUS_IfdPullInRange_e250kHz,
    NEXUS_IfdPullInRange_e500kHz,
    NEXUS_IfdPullInRange_e750kHz,
    NEXUS_IfdPullInRange_e1000kHz,
    NEXUS_IfdPullInRange_e1100kHz,
    NEXUS_IfdPullInRange_eMax
} NEXUS_IfdPullInRange;

/***************************************************************************
Summary:
Generic IFD Settings
***************************************************************************/
typedef struct NEXUS_IfdSettings
{
    bool enabled;                           /* Set to true to enable the IFD, otherwise set to false */
    
    bool spectrumInverted;                  /* Set to true if the spectrum is inverted (SECAM-L' requires this) */
    NEXUS_IfdInputType videoInputType;      /* Video Input to IFD.  Can select None, IF or Baseband. */
    unsigned videoInputIndex;               /* Video Input index.  Specifies which specific input of type inputType will be selected */

    NEXUS_IfdInputType audioInputType;      /* Audio Input to IFD.  Can select IF or SIF. */
    unsigned audioInputIndex;               /* Audio Input index.  Specifies which specific input of type inputType will be selected */

    unsigned carrierFrequency;              /* Video carrier frequency, in Hz.  Default=45.75MHz */
    NEXUS_VideoFormat videoFormat;          /* Default=NEXUS_VideoFormat_eNtsc */
    NEXUS_IfdAudioMode audioMode;           /* Audio Mode.  Default = NEXUS_IfdAudioMode_eUs */
    NEXUS_IfdAudioDeviation audioDeviation; /* Audio Deviation mode.  Default = NEXUS_IfdAudioDeviation_eNormal */
    NEXUS_IfdPullInRange pullInRange;       /* Pull In Range.  Default = NEXUS_IfdPullInRange_eDefault */
    union
    {   
        struct
        {
           NEXUS_UsIfdAudioMode mode; 
        } us;
        struct
        {
            NEXUS_KoreaIfdAudioMode mode; 
        } korea;
        struct
        {
            NEXUS_JapanIfdAudioMode mode; 
        } japan;
        struct
        {
            NEXUS_NicamIfdAudioMode mode; 
        } nicam;
        struct
        {
            NEXUS_PalA2IfdAudioMode mode; 
        } palA2;
        struct
        {
            NEXUS_NicamIfdAudioMode nicamMode; 
            NEXUS_PalA2IfdAudioMode palA2Mode; 
        } autoNicamPalA2;
    } audioModeSettings;

    NEXUS_CallbackDesc lockCallback;        /* Callback will be called when lock status changes */
} NEXUS_IfdSettings;

/***************************************************************************
Summary:
    Get IFD Settings
***************************************************************************/
void NEXUS_Ifd_GetSettings(
    NEXUS_IfdHandle handle,
    NEXUS_IfdSettings *pSettings        /* [out] */
    );

/***************************************************************************
Summary:
    Set IFD Settings
***************************************************************************/
void NEXUS_Ifd_SetSettings(
    NEXUS_IfdHandle handle,
    const NEXUS_IfdSettings *pSettings
    );

/***************************************************************************
Summary:
IF Modulation Types
***************************************************************************/
typedef struct NEXUS_IfdStatus
{
    bool locked;        /* lock=true, unlocked=false */
    int carrierOffset;  /* Carrier Frequency Offset, in Hertz */
    int rfAgcLevel;     /* RF AGC level, in 1/10 percent */
    int ifAgcLevel;     /* IF AGC level, in 1/10 percent */
} NEXUS_IfdStatus;

/***************************************************************************
Summary:
    Get current IFD status
***************************************************************************/
void NEXUS_Ifd_GetStatus(
    NEXUS_IfdHandle handle,
    NEXUS_IfdStatus *pStatus    /* [out] */
    );

/***************************************************************************
Summary:
Do not use this function. This is a stub function
***************************************************************************/
NEXUS_IfdHandle NEXUS_Ifd_OpenStub( /* attr{destructor=NEXUS_Ifd_Close} */
    unsigned index
    );

/***************************************************************************
Summary:
    Close an IFD handle
***************************************************************************/
void NEXUS_Ifd_Close(
    NEXUS_IfdHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_IFD_H__ */

