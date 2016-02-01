/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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
 * Module Description: Audio Equalizer Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BAPE_EQUALIZER_H_
#define BAPE_EQUALIZER_H_

/*******************************************************************************
Summary:
    Type of Equalization for Tone Control
   
Description:
    This enum describes the type of equalization that is to be applied in a 
    Tone control mode.
*******************************************************************************/

typedef enum BAPE_ToneControlEqType
{
    BAPE_ToneControlEqType_eShelving,
    BAPE_ToneControlEqType_eNotch    
} BAPE_ToneControlEqType;

typedef struct BAPE_EqualizerToneControlSettings
{
    /* Bass */
    int                             bassGain;           /* Gain Specified in 1/100 dB.  Values range from -10dB (-1000) to +10dB (+1000) */
    unsigned                        bassFreq;           /* Frequency */    
    BAPE_ToneControlEqType          bassEqType;         /* Equalizer Type */    
    unsigned                        bassBandwidthFreq;  /* Frequency Bandwidth */

    /* Treble */
    int                             trebleGain;         /* Gain Specified in 1/100 dB.  Values range from -10dB (-1000) to +10dB (+1000) */    
    unsigned                        trebleFreq;         /* Frequency */    
    BAPE_ToneControlEqType          trebleEqType;       /* Equalizer Type */    
    unsigned                        trebleBandwidthFreq;/* Frequency Bandwidth */
} BAPE_EqualizerToneControlSettings;


/**********************************************************************
Summary:
    Settings for 5 band graphic equalizer
    
Description:
    This structure holds the settings of an equalizer when it is configured in 
    5 band equalizer mode. The values can range from -120 to +120 for the range 
    of -12dB to +12dB for each band in the hardware equalizer.
***********************************************************************/
typedef struct BAPE_EqualizerFiveBandSettings
{
    int gain100Hz;  /* Specified in 1/100 dB.  Values range from -10dB (-1000) to +10dB (+1000) */
    int gain300Hz;  /* Specified in 1/100 dB.  Values range from -10dB (-1000) to +10dB (+1000) */
    int gain1000Hz; /* Specified in 1/100 dB.  Values range from -10dB (-1000) to +10dB (+1000) */
    int gain3000Hz; /* Specified in 1/100 dB.  Values range from -10dB (-1000) to +10dB (+1000) */
    int gain10000Hz;/* Specified in 1/100 dB.  Values range from -10dB (-1000) to +10dB (+1000) */
}BAPE_EqualizerFiveBandSettings;


/**********************************************************************
Summary:
    Settings for PEQ parametric equalizer
    
Description:
    This structure holds the settings of an equalizer when it is configured in 
    PEQ equalizer mode.
***********************************************************************/

/***************************************************************************
Summary:
    Seven Band EQ window step - time for phase in/out for filter change
***************************************************************************/
typedef enum BAPE_EqualizerWindowStep
{
    BAPE_EqualizerWindowStep_eNone,   /* No window */
    BAPE_EqualizerWindowStep_e170_6,  /* 170.6ms at 48kHz */
    BAPE_EqualizerWindowStep_e85_3,   /* 85.3ms at 48kHz */
    BAPE_EqualizerWindowStep_e42_6,   /* 42.6ms at 48kHz */
    BAPE_EqualizerWindowStep_e21_3,   /* 21.3ms at 48kHz */
    BAPE_EqualizerWindowStep_e10_6,   /* 10.6ms at 48kHz */
    BAPE_EqualizerWindowStep_e5_3,    /* 5.3ms at 48kHz */
    BAPE_EqualizerWindowStep_e2_6,    /* 2.6ms at 48kHz */
    BAPE_EqualizerWindowStep_eMax
} BAPE_EqualizerWindowStep;

typedef struct BAPE_EqualizerSevenBandSettings
{
    struct
    {
        unsigned peak;  /* Peak frequency in Hz */
        int gain;       /* Specified in 1/100 dB.  Values range from -12dB (-1200) to +12dB (+1200) */
        unsigned q;     /* Peak Frequency Q gain specified in 1/100 (e.g. .33 = 33) */
    } bandSettings[7];

    BAPE_EqualizerWindowStep  windowStep; /* time for phase in/out for filter change */
}BAPE_EqualizerSevenBandSettings;


/*******************************************************************************
Summary:
    Type of Filter for Low/High Pass Filtering in Subsonic/Subwoofer modes
   
Description:
    This enum describes the type of filtering to be applied when the mode is
    subwoofer or subsonic
*******************************************************************************/
typedef enum BAPE_EqualizerFilterType
{
    BAPE_EqualizerFilterType_eButterworth, 
    BAPE_EqualizerFilterType_eLinkwitzRiley,
    BAPE_EqualizerFilterType_eMax
}BAPE_EqualizerFilterType;

/*******************************************************************************
Summary:
    Filter order for Low/High Pass Filtering in Subsonic/Subwoofer modes
   
Description:
    This enum describes the order of the filter to be employed when the mode is
    subwoofer or subsonic
*******************************************************************************/
typedef enum BAPE_EqualizerFilterOrder
{
    BAPE_EqualizerFilterOrder_e2 = 2,
    BAPE_EqualizerFilterOrder_e4 = 4,
    BAPE_EqualizerFilterOrder_e6 = 6
}BAPE_EqualizerFilterOrder;


/*******************************************************************************
Summary:
    Settings for Subsonic Filter
   
Description:
    This structure holds the settings of an equalizer when it is configured in 
    Subsonic filter mode.
*******************************************************************************/
typedef struct BAPE_EqualizerSubsonicSettings
{
    unsigned            frequency;  /* Subsonic filter frequency (in Hz)  Ranges from 40..315Hz */
    BAPE_EqualizerFilterOrder    filterOrder;      /* Possible Values: 2, 4, 6 */
    BAPE_EqualizerFilterType     filterType;        /* Filter type for Subsonic Mode */    
}BAPE_EqualizerSubsonicSettings;


/*******************************************************************************
Summary:
    Settings for Subwoofer Filter
   
Description:
    This structure holds the settings of an equalizer when it is configured in 
    Subwoofer filter mode.
*******************************************************************************/
typedef struct BAPE_EqualizerSubwooferSettings
{
    unsigned            frequency; /* Subwoofer filter frequency (in Hz) Ranges from 40..315Hz */
    BAPE_EqualizerFilterOrder    filterOrder;      /* Possible Values: 2, 4, 6 */
    BAPE_EqualizerFilterType     filterType;        /* Filter type for Subwoofer Mode */        
}BAPE_EqualizerSubwooferSettings;


/**********************************************************************
Summary:
    Settings for a custom equalizer mode
   
Description:
    This structure holds the settings of an equalizer when it is configured in 
    custom equalizer mode. 
***********************************************************************/
typedef struct BAPE_EqualizerIIRFilterCoefficients
{
    int32_t b0;
    int32_t b1;
    int32_t b2;
    int32_t a1;
    int32_t a2;    
}BAPE_EqualizerIIRFilterCoefficients;

#define BAPE_EQUALIZER_MAX_CUSTOM_FILTERS (8)
typedef struct BAPE_EqualizerCustomSettings
{
    unsigned numFilters;        /* Number of biquad IIR filters to be programmed.  Ranges from 0..8. */
    BAPE_EqualizerIIRFilterCoefficients filterCoefficients[BAPE_EQUALIZER_MAX_CUSTOM_FILTERS];
}BAPE_EqualizerCustomSettings;


/***************************************************************************
Summary:
Equalizer Stage Types
***************************************************************************/
typedef enum BAPE_EqualizerStageType
{
    BAPE_EqualizerStageType_eToneControl,
    BAPE_EqualizerStageType_eFiveBand,
    BAPE_EqualizerStageType_eSevenBand,
    BAPE_EqualizerStageType_eSubsonic,
    BAPE_EqualizerStageType_eSubwoofer,
    BAPE_EqualizerStageType_eCustom,
    BAPE_EqualizerStageType_eMax
} BAPE_EqualizerStageType;

/***************************************************************************
Summary:
Equalizer Stage Settings
***************************************************************************/
typedef struct BAPE_EqualizerRampSettings
{
    bool enable;                       /* If true (default), the coefficients will be double-buffered for on-the-fly settings changes.
                                          If false, the coefficients are not double-buffered to save coefficient memory space.
                                          In case there is a conflict among Stages that are packed into single SRC, 
                                          the take is rampEnabled as 'true' for all stages. It is not programmable on the fly. */ 
    unsigned stepSize;                  /* Valid if ramping is enabled. 
                                           The Equalizer will ramp between two banks using 2^stepSize samples. */                                           
}BAPE_EqualizerRampSettings;

/***************************************************************************
Summary:
Equalizer Stage Settings
***************************************************************************/
typedef struct BAPE_EqualizerStageSettings
{
    BAPE_EqualizerStageType type;           /* Type of equalizer stage.  May only be set at create time. */
    BAPE_EqualizerRampSettings   rampSettings;
    bool bypassEnabled;                     /* If true, the values specified in modeSettings will be ignored and the data
                                               will bypass this stage */

    union
    {
        BAPE_EqualizerToneControlSettings toneControl;
        BAPE_EqualizerFiveBandSettings fiveBand;
        BAPE_EqualizerSevenBandSettings sevenBand;
        BAPE_EqualizerSubsonicSettings subsonic;
        BAPE_EqualizerSubwooferSettings subwoofer;
        BAPE_EqualizerCustomSettings custom;
    } modeSettings;
} BAPE_EqualizerStageSettings;

/***************************************************************************
Summary:
Equalizer Stage Handle 
 
Description: 
An Equalizer stage represents a single operation to be performed in one or 
more equalizers.  A stage handle may be passed to more than one equalizer if 
the same operation is to be performed in more than one equalizer, however 
the equalizers must run at the same sample rate. 
 
See Also: 
BAPE_Equalizer 
***************************************************************************/
typedef struct BAPE_EqualizerStage *BAPE_EqualizerStageHandle;

/***************************************************************************
Summary:
Equalizer Handle
***************************************************************************/
typedef struct BAPE_Equalizer *BAPE_EqualizerHandle;

/***************************************************************************
Summary:
Equalizer Settings
***************************************************************************/
typedef struct BAPE_EqualizerSettings
{
    int tbd;
} BAPE_EqualizerSettings;

/***************************************************************************
Summary:
Get default open settings for an equalizer stage
***************************************************************************/
void BAPE_EqualizerStage_GetDefaultSettings(
    BAPE_EqualizerStageType type,
    BAPE_EqualizerStageSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Create an audio equalizer stage
***************************************************************************/
BERR_Code BAPE_EqualizerStage_Create(
    BAPE_Handle deviceHandle,
    const BAPE_EqualizerStageSettings *pSettings, 
    BAPE_EqualizerStageHandle *pHandle                 /* [out] */
    );

/***************************************************************************
Summary:
Destroy an audio equalizer stage
***************************************************************************/
void BAPE_EqualizerStage_Destroy(
    BAPE_EqualizerStageHandle handle
    );

/***************************************************************************
Summary:
Get settings for an audio equalizer stage
***************************************************************************/
void BAPE_EqualizerStage_GetSettings(
    BAPE_EqualizerStageHandle handle,
    BAPE_EqualizerStageSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Set settings for an audio equalizer stage
***************************************************************************/
BERR_Code BAPE_EqualizerStage_SetSettings(
    BAPE_EqualizerStageHandle handle,
    const BAPE_EqualizerStageSettings *pSettings
    );

/***************************************************************************
Summary:
Get Default Equalizer Settings
***************************************************************************/
void BAPE_Equalizer_GetDefaultSettings(
    BAPE_EqualizerSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Create an equalizer
***************************************************************************/
BERR_Code BAPE_Equalizer_Create(
    BAPE_Handle deviceHandle,
    const BAPE_EqualizerSettings *pSettings,
    BAPE_EqualizerHandle *pHandle               /* [out] */
    );

/***************************************************************************
Summary:
Destroy an equalizer
***************************************************************************/
void BAPE_Equalizer_Destroy(
    BAPE_EqualizerHandle handle
    );

/***************************************************************************
Summary:
Set stages for an equalizer 
 
Description: 
An Equalizer is capable of processing one or more stages.  The array of stages 
provided in pStages will be executed in order.  A stage may be used simultaneously 
by one or more equalizers, provided those equalizers run at the same sample 
rate. 
 
See Also: 
BAPE_EqualizerStage 
BAPE_Equalizer_ClearStages 
***************************************************************************/
BERR_Code BAPE_Equalizer_SetStages(
    BAPE_EqualizerHandle handle,
    BAPE_EqualizerStageHandle *pStages,
    unsigned numStages
    );

/***************************************************************************
Summary:
Remove all stages from an equalizer
***************************************************************************/
void BAPE_Equalizer_ClearStages(
    BAPE_EqualizerHandle handle
    );

/***************************************************************************
Summary:
Add an input to an equalizer
***************************************************************************/
BERR_Code BAPE_Equalizer_AddInput(
    BAPE_EqualizerHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove an input from an equalizer
***************************************************************************/
BERR_Code BAPE_Equalizer_RemoveInput(
    BAPE_EqualizerHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove all inputs from an equalizer
***************************************************************************/
BERR_Code BAPE_Equalizer_RemoveAllInputs(
    BAPE_EqualizerHandle handle
    );

/***************************************************************************
Summary:
Get a connection handle for use downstream
***************************************************************************/
void BAPE_Equalizer_GetConnector(
    BAPE_EqualizerHandle handle,
    BAPE_Connector *pConnector     /* [out] */
    );

#endif

