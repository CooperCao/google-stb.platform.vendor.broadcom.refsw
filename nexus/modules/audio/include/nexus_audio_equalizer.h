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
*   API name: AudioEqualizer
*    Generic APIs for audio equalizer control.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_AUDIO_EQUALIZER_H__
#define NEXUS_AUDIO_EQUALIZER_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"
#include "nexus_audio_output.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=************************************
Interface: AudioOutput

Header file: nexus_audio_equalizer.h

Module: Audio

Description: Interface for audio output equalizer settings

**************************************/

/***************************************************************************
Summary:
	Equalizer Stage Handle 

Description: 
	An Equalizer stage represents a single operation to be performed in one or 
	more equalizers.  A stage handle may be passed to more than one equalizer if 
	the same operation is to be performed in more than one equalizer, however 
	the equalizers must run at the same sample rate. 

See Also: 
	NEXUS_AudioEqualizerHandle 
***************************************************************************/
typedef struct NEXUS_AudioEqualizerStage *NEXUS_AudioEqualizerStageHandle;

/*******************************************************************************
Summary:
    Type of Equalization for Tone Control
   
Description:
    This enum describes the type of equalization that is to be applied in a 
    Tone control mode.
*******************************************************************************/
typedef enum NEXUS_AudioToneControlType
{
    NEXUS_AudioToneControlType_eShelving,
    NEXUS_AudioToneControlType_eNotch,
    NEXUS_AudioToneControlType_eMax
} NEXUS_AudioToneControlType;

/*******************************************************************************
Summary:
    Tone Control Equaliizer Settings
*******************************************************************************/
typedef struct NEXUS_AudioEqualizerToneControlSettings
{
    /* Bass */
	struct{
		int                             gain;           /* Gain Specified in 1/100 dB.  Values range from -10dB (-1000) to +10dB (+1000) */
		unsigned						freq;			/* Frequency in Hz */    
		NEXUS_AudioToneControlType		eqType; 		/* Equalizer Type */	
		unsigned						bandwidthFreq;	/* Frequency Bandwidth in Hz */
	}bassSettings;

    /* Treble */
    struct{
	   	int                             gain;         /* Gain Specified in 1/100 dB.  Values range from -10dB (-1000) to +10dB (+1000) */    
	    unsigned                        freq;         /* Frequency in Hz */    
	    NEXUS_AudioToneControlType      eqType;       /* Equalizer Type */    
	    unsigned                        bandwidthFreq;/* Frequency Bandwidth in Hz */
	}trebleSettings;
} NEXUS_AudioEqualizerToneControlSettings;


/**********************************************************************
Summary:
    Settings for 5 band graphic equalizer
    
Description:
    This structure holds the settings of an equalizer when it is configured in 
    5 band equalizer mode. The values can range from -120 to +120 for the range 
    of -12dB to +12dB for each band in the hardware equalizer.
***********************************************************************/
typedef struct NEXUS_AudioEqualizerFiveBandSettings
{
    int gain100Hz;  /* Specified in 1/100 dB.  Values range from -10dB (-1000) to +10dB (+1000) */
    int gain300Hz;  /* Specified in 1/100 dB.  Values range from -10dB (-1000) to +10dB (+1000) */
    int gain1000Hz; /* Specified in 1/100 dB.  Values range from -10dB (-1000) to +10dB (+1000) */
    int gain3000Hz; /* Specified in 1/100 dB.  Values range from -10dB (-1000) to +10dB (+1000) */
    int gain10000Hz;/* Specified in 1/100 dB.  Values range from -10dB (-1000) to +10dB (+1000) */
}NEXUS_AudioEqualizerFiveBandSettings;

/***************************************************************************
Summary:
    Seven Band EQ window step - time for phase in/out for filter change
***************************************************************************/
typedef enum NEXUS_AudioEqualizerWindowStep
{
    NEXUS_AudioEqualizerWindowStep_eNone,   /* No window */
    NEXUS_AudioEqualizerWindowStep_e170_6,  /* 170.6ms at 48kHz */
    NEXUS_AudioEqualizerWindowStep_e85_3,   /* 85.3ms at 48kHz */
    NEXUS_AudioEqualizerWindowStep_e42_6,   /* 42.6ms at 48kHz */
    NEXUS_AudioEqualizerWindowStep_e21_3,   /* 21.3ms at 48kHz */
    NEXUS_AudioEqualizerWindowStep_e10_6,   /* 10.6ms at 48kHz */
    NEXUS_AudioEqualizerWindowStep_e5_3,    /* 5.3ms at 48kHz */
    NEXUS_AudioEqualizerWindowStep_e2_6,    /* 2.6ms at 48kHz */
    NEXUS_AudioEqualizerWindowStep_eMax
} NEXUS_AudioEqualizerWindowStep;

#define NEXUS_AUDIO_MAX_PEQ_BANDS (7)

/**********************************************************************
Summary:
    Settings for seven band parametric equalizer
    
Description:
    This structure holds the settings of an equalizer when it is configured in 
    seven band equalizer mode.
***********************************************************************/
typedef struct NEXUS_AudioEqualizerSevenBandSettings
{
    struct
    {
        unsigned peak;  /* Peak frequency in Hz */
        int gain;       /* Specified in 1/100 dB.  Values range from -12dB (-1200) to +12dB (+1200) */
        unsigned q;     /* Peak Frequency Q gain specified in 1/100 (e.g. .33 = 33) */
    } bandSettings[NEXUS_AUDIO_MAX_PEQ_BANDS];

    NEXUS_AudioEqualizerWindowStep  windowStep; /* time for phase in/out for filter change */
}NEXUS_AudioEqualizerSevenBandSettings;


/*******************************************************************************
Summary:
    Type of Filter for Low/High Pass Filtering in Subsonic/Subwoofer modes
   
Description:
    This enum describes the type of filtering to be applied when the mode is
    subwoofer or subsonic
*******************************************************************************/
typedef enum NEXUS_AudioEqualizerFilterType
{
    NEXUS_AudioEqualizerFilterType_eButterworth,    
    NEXUS_AudioEqualizerFilterType_eLinkwitzRiley,  
    NEXUS_AudioEqualizerFilterType_eMax
}NEXUS_AudioEqualizerFilterType;


/*******************************************************************************
Summary:
    Settings for Subsonic Filter
   
Description:
    This structure holds the settings of an equalizer when it is configured in 
    Subsonic filter mode.
*******************************************************************************/
typedef struct NEXUS_AudioEqualizerSubsonicSettings
{
    unsigned            frequency;   /* Subsonic filter frequency (in Hz)  Ranges from 40..315Hz */
    unsigned            filterOrder; /* Possible Values: 2, 4, 6 */
    NEXUS_AudioEqualizerFilterType     filterType;        /* Filter type for Subsonic Mode */    
}NEXUS_AudioEqualizerSubsonicSettings;


/*******************************************************************************
Summary:
    Settings for Subwoofer Filter
   
Description:
    This structure holds the settings of an equalizer when it is configured in 
    Subwoofer filter mode.
*******************************************************************************/
typedef struct NEXUS_AudioEqualizerSubwooferSettings
{
    unsigned            frequency;        /* Subwoofer filter frequency (in Hz) Ranges from 40..315Hz */
    unsigned            filterOrder;      /* Possible Values: 2, 4, 6 */
    NEXUS_AudioEqualizerFilterType     filterType;        /* Filter type for Subwoofer Mode */        
}NEXUS_AudioEqualizerSubwooferSettings;


/***************************************************************************
Summary:
Equalizer Stage Types
***************************************************************************/
typedef enum NEXUS_AudioEqualizerStageType
{
    NEXUS_AudioEqualizerStageType_eToneControl,
    NEXUS_AudioEqualizerStageType_eFiveBand,
    NEXUS_AudioEqualizerStageType_eSevenBand,
    NEXUS_AudioEqualizerStageType_eSubsonic,
    NEXUS_AudioEqualizerStageType_eSubwoofer,
    NEXUS_AudioEqualizerStageType_eMax
} NEXUS_AudioEqualizerStageType;

/***************************************************************************
Summary:
Equalizer Ramp Settings
***************************************************************************/
typedef struct NEXUS_AudioEqualizerRampSettings
{
    bool enable;                       /* If true (default), the coefficients will be double-buffered for on-the-fly settings changes.
                                          If false, the coefficients are not double-buffered to save coefficient memory space.
                                          In case there is a conflict among Stages that are packed into single SRC, 
                                          the take is rampEnabled as 'true' for all stages. It is not programmable on the fly. */ 
    unsigned stepSize;                  /* Valid if ramping is enabled. 
                                           The Equalizer will ramp between two banks using 2^stepSize samples. */                                           
}NEXUS_AudioEqualizerRampSettings;


/***************************************************************************
Summary:
Equalizer Stage Settings
***************************************************************************/
typedef struct NEXUS_AudioEqualizerStageSettings
{
    NEXUS_AudioEqualizerStageType type;     /* Type of equalizer stage.  May only be set at create time. */
    NEXUS_AudioEqualizerRampSettings rampSettings;
    bool enabled;                           /* If true, equalizer stage will be enabled, if false data
                                               will bypass this stage */

    union
    {
        NEXUS_AudioEqualizerToneControlSettings toneControl;
        NEXUS_AudioEqualizerFiveBandSettings fiveBand;
        NEXUS_AudioEqualizerSevenBandSettings sevenBand;
        NEXUS_AudioEqualizerSubsonicSettings subsonic;
        NEXUS_AudioEqualizerSubwooferSettings subwoofer;
    } modeSettings;
} NEXUS_AudioEqualizerStageSettings;


/***************************************************************************
Summary:
Get default open settings for an equalizer stage
***************************************************************************/
void NEXUS_AudioEqualizerStage_GetDefaultSettings(
    NEXUS_AudioEqualizerStageType type,
    NEXUS_AudioEqualizerStageSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Create an audio equalizer stage
***************************************************************************/
NEXUS_AudioEqualizerStageHandle NEXUS_AudioEqualizerStage_Create(	/* attr{destructor=NEXUS_AudioEqualizerStage_Destroy} */
    const NEXUS_AudioEqualizerStageSettings *pSettings	            /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
Destroy an audio equalizer stage
***************************************************************************/
void NEXUS_AudioEqualizerStage_Destroy(
    NEXUS_AudioEqualizerStageHandle handle
    );

/***************************************************************************
Summary:
Get settings for an audio equalizer stage
***************************************************************************/
void NEXUS_AudioEqualizerStage_GetSettings(
    NEXUS_AudioEqualizerStageHandle handle,
    NEXUS_AudioEqualizerStageSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Set settings for an audio equalizer stage
***************************************************************************/
NEXUS_Error NEXUS_AudioEqualizerStage_SetSettings(
    NEXUS_AudioEqualizerStageHandle handle,
    const NEXUS_AudioEqualizerStageSettings *pSettings
    );

/***************************************************************************
Summary:
Equalizer Handle
***************************************************************************/
typedef struct NEXUS_AudioEqualizer *NEXUS_AudioEqualizerHandle;

/***************************************************************************
Summary:
Equalizer Settings
***************************************************************************/
typedef struct NEXUS_AudioEqualizerSettings
{
    int tbd;
} NEXUS_AudioEqualizerSettings;

/***************************************************************************
Summary:
Get Default Equalizer Settings
***************************************************************************/
void NEXUS_AudioEqualizer_GetDefaultSettings(
    NEXUS_AudioEqualizerSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Create an equalizer
***************************************************************************/
NEXUS_AudioEqualizerHandle NEXUS_AudioEqualizer_Create( /* attr{destructor=NEXUS_AudioEqualizer_Destroy} */
    const NEXUS_AudioEqualizerSettings *pSettings	    /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
Destroy an equalizer
***************************************************************************/
void NEXUS_AudioEqualizer_Destroy(
    NEXUS_AudioEqualizerHandle handle
    );       

/***************************************************************************
Summary:
Remove a stage from an equalizer
***************************************************************************/
NEXUS_Error NEXUS_AudioEqualizer_AddStage(
	NEXUS_AudioEqualizerHandle equalizer,
	NEXUS_AudioEqualizerStageHandle stage
	);
 
/***************************************************************************
Summary:
Remove a stage from an equalizer
***************************************************************************/
NEXUS_Error NEXUS_AudioEqualizer_RemoveStage(
	NEXUS_AudioEqualizerHandle equalizer,
	NEXUS_AudioEqualizerStageHandle stage
    );

/***************************************************************************
Summary:
Remove all stages from an equalizer
***************************************************************************/
NEXUS_Error NEXUS_AudioEqualizer_RemoveAllStages(
	NEXUS_AudioEqualizerHandle equalizer
    );

/***************************************************************************
Summary:
Connect an equalizer to an output

Description:
This can only be called when all inputs to the specified output are 
stopped.
***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_SetEqualizer(
    NEXUS_AudioOutputHandle output,
    NEXUS_AudioEqualizerHandle equalizer    /* attr{null_allowed=y} Pass NULL to remove any equalizer connected to this output */
    );

/***************************************************************************
Summary:
Remove equalizer from the output.
***************************************************************************/
void NEXUS_AudioOutput_ClearEqualizer(
	NEXUS_AudioOutputHandle output
	);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUDIO_EQUALIZER_H__ */

