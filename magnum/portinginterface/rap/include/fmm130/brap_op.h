/***************************************************************************
*     Copyright (c) 2004-2008, Broadcom Corporation
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
*   Module name: Output
*   This file lists all data structures, macros, enumerations and function 
*   prototypes for the Output ports abstraction, which are exposed to the 
*   application developer.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef _BRAP_OP_H_
#define _BRAP_OP_H_

#include "brap.h"

#ifdef __cplusplus
extern "C" {
#endif

    
/***************************************************************************
Summary:
    This enumeration lists all PLLs that can be used by the Output block.

Description:
     Please note that the application can allocate PLL's as they prefer keeping 
     the following in mind:
    - DAC does not require a PLL.      
    - Output ports driven by different sources must use different PLL's => the 
    application should set aside 1 PLL per source. Note: different PIDs in the 
    same stream are considered as different sources.
    - All Output ports using a common PLL must also 
        - carry data from the same source ie same audio pid etc.
        - use a common timebase
        - be configured for the same sampling rate (or multiples therefore?)
    - Beware that SmartCard may use PLL1 on certain chips/applications
    - If the application is switching HDMI/SPDIF to connect to different audio channels, 
    it will have to switch PLLs too.

Examples:
1. Consider a system with 2 audio channels, one carrying PCM (decode channel) and the other carrying 
the compressed data (passthrough channel).

i) If they both carry audio for the same audio source, then all output ports 
connected to both channels can use a common PLL, provided they are also 
configured to use the same time base (in 7401,7400 such ports will be 
configured for the same sampling rate)

ii) If both channels carry different audio say decode channels carries MPEG, and 
the compressed channel carries AC3 passthrough, then the output ports connected 
to them will have to use configure different PLLs.

2. Examples of some things you cannot do with just one audio PLL
   i) Watch a digital channel with multiple audio PIDs since each PID may have 
   a different sampling rate.
   ii) Decode one audio PID with Fs=48-kHz and send output to I2S. Pass-thru/
   decode a second audio PID with Fs=44.1-kHz and send output to SPDIF.
   iii) Run SPDIF/I2S/HDMI at different sample rates
   iv) Run SPDIF/I2S/HDMI on different timebases
   v) Run HDMI at a sample rate that is not being used by DAC/SPDIF/I2S or 
   some other output port.
   vi) Run HDMI on a timebase that is not being used by DAC/SPDIF/I2S or 
   some other output port.

    
See Also:

***************************************************************************/  
typedef enum BRAP_OP_Pll
{
    BRAP_OP_Pll_ePll0=0,    /* FMM PLL0 */
    BRAP_OP_Pll_ePll1,      /* FMM PLL1 */
    BRAP_OP_Pll_ePll2,      /* FMM PLL2 */
    BRAP_OP_Pll_ePll0Div2,  /* FMM PLL0 with half the frequency */
    BRAP_OP_Pll_ePll1Div2,  /* FMM PLL1 with half the frequency */
    BRAP_OP_Pll_ePll2Div2,  /* FMM PLL2 with half the frequency */
    BRAP_OP_Pll_eHifiDac0,  /* Hi Fi DAC 0 */
    BRAP_OP_Pll_eHifiDac1,  /* Hi Fi DAC 1 */
    BRAP_OP_Pll_eMax
}BRAP_OP_Pll;
  
/***************************************************************************
Summary:
    This enumeration lists the external Master Clocks that can be used by the 
    Output block.

Description:

    
See Also:

***************************************************************************/  
typedef enum BRAP_OP_ExtMClk
{
    BRAP_OP_ExtMClk_eExternal0=0, /* External Master Clock 0 */
    BRAP_OP_ExtMClk_eExternal1      /* External Master Clock 1 */
}BRAP_OP_ExtMClk;

/***************************************************************************
Summary:
    This enumeration lists all SClock (bit clock) rates that can be used.

Description:
    Indicates the frequency of SClock as a multiple of Fs.

See Also:

***************************************************************************/  
typedef enum BRAP_OP_SClkRate
{
    BRAP_OP_SClkRate_e64Fs=1, /* SCLK rate is 64fs */
    BRAP_OP_SClkRate_e128Fs   /* SCLK rate is 128fs */
}BRAP_OP_SClkRate;
    


/***************************************************************************
Summary:
    This enumeration lists all MClock rates that can be used.

Description:
    Indicates the frequency of MCLK as a multiple of Fs.

See Also:

***************************************************************************/  
typedef enum BRAP_OP_MClkRate
{
    BRAP_OP_MClkRate_e128Fs=1, /* MCLK rate is 128fs */
    BRAP_OP_MClkRate_e256Fs,   /* MCLK rate is 256fs */
    BRAP_OP_MClkRate_e384Fs,   /* MCLK rate is 384fs */
    BRAP_OP_MClkRate_e512Fs    /* MCLK rate is 526fs */
}BRAP_OP_MClkRate;
    

/***************************************************************************
Summary:
    This enumeration allows left and right output channels to be normal, 
    swapped, or copies of the left/right input channels.

Description:


See Also:

***************************************************************************/  
typedef enum BRAP_OP_ChannelOrder
{
    BRAP_OP_ChannelOrder_eNormal=0,     /* Normal */
    BRAP_OP_ChannelOrder_eBothLeft,     /* Both get Left */
    BRAP_OP_ChannelOrder_eBothRight,    /* Both get Right */
    BRAP_OP_ChannelOrder_eSwap          /* Swap */
    
}BRAP_OP_ChannelOrder;


/***************************************************************************
Summary:
    Specifies the MAI audio format. 
    
Description:
    This field does not affect the actual data the MAI formatter transmits. 
    The MAI formatter always sends a sample pair. In other words, setting 
    this field to PCM_Mono will not stop the MAI formatter from transmitting 
    the right channel data, but the receiver will think only the left data is 
    valid.

See Also:

***************************************************************************/ 
typedef enum BRAP_OP_MaiAudioFormat
{
    BRAP_OP_MaiAudioFormat_eIdle=0,             /* Idle */
#if (BRAP_7401_FAMILY == 1)
    BRAP_OP_MaiAudioFormat_ePcmMono,            /* PCM Mono */
    BRAP_OP_MaiAudioFormat_ePcmStereo,          /* PCM Stereo */
#else
    /* For 7400 */
    BRAP_OP_MaiAudioFormat_eSpdif2Channel = 0x82,	/* Carries SPDIF 2 channel */
    BRAP_OP_MaiAudioFormat_eSpdif6Channel = 0x86,	/* Carries SPDIF 6 channel */
    BRAP_OP_MaiAudioFormat_eSpdif8Channel = 0x88,	/* Carries SPDIF 8 Channel */
#endif
    BRAP_OP_MaiAudioFormat_eSpdifLinearPcm=254, /* Uncompressed PCM in SPDIF format */
    BRAP_OP_MaiAudioFormat_eSpdifCompressed     /* Compressed SPDIF */
}BRAP_OP_MaiAudioFormat;



/***************************************************************************
Summary:
    This enumeration lists the various Flex formats. 
    
Description:
    Only 6 are used as of now.
    The following should be added later:
    BRAP_OP_FlexFormat_e16BitLsb, // 16 bit samples outputted LSB
    BRAP_OP_FlexFormat_e24BitLsb,  // 24 bit LSB format
    BRAP_OP_FlexFormat_e24BitLsbDelay, // 24 lsb Bits will be delayed by one clock period
    BRAP_OP_FlexFormat_eI2sInvClk, // I2S data outputted at Inverted clock 
    BRAP_OP_FlexFormat_e12BitLsb, //12 bit LSB output 
    BRAP_OP_FlexFormat_e32BitLsb, //32 bit LSB output
    BRAP_OP_FlexFormat_eSync18Bit, // 8 Bit output with sync  signal high for 1 clock period
    BRAP_OP_FlexFormat_eSync1_16Bit, // 16 Bit output with sync  signal high for 1 clock period
    BRAP_OP_FlexFormat_eSync1_16BitLsbFirst,  //16 Bit Lsb first output with sync signal high for 1 clock period
    BRAP_OP_FlexFormat_eSync1_32BitLsbFirst, //32 Bit Lsb first output with sync signal high for 1 clock period
    BRAP_OP_FlexFormat_eSyncAll_32Bit, //   SYNC signal valid for all clock cycle 32BIT
    BRAP_OP_FlexFormat_eSync1_32BitLsb, //   SYNC signal valid for all clock cycle 32BIT LSB first     

See Also:

***************************************************************************/ 
typedef enum BRAP_OP_FlexFormat
{

    BRAP_OP_FlexFormat_eI2s=0,      /* Flex Mimics Normal I2S */
    BRAP_OP_FlexFormat_eAligned,    /* I2S aligned samples with Mclock */
    BRAP_OP_FlexFormat_eCompressed8Bit, 
    BRAP_OP_FlexFormat_eCompressed16Bit,
    BRAP_OP_FlexFormat_eCompressedGap,
    BRAP_OP_FlexFormat_eCompressedContinuous
}BRAP_OP_FlexFormat;

/***************************************************************************
Summary:
    This enumeration lists the types of software mute used for muting the DAC output.

Description:

    
See Also:

***************************************************************************/  
typedef enum BRAP_OP_DacMuteType
{
	BRAP_OP_DacMuteType_ConstantLow,		/* Contant_Low */
	BRAP_OP_DacMuteType_ConstantHigh,		/* Constant_Hight */
	BRAP_OP_DacMuteType_SquareWaveOpp,	/* Square_Wave_Opp */    
	BRAP_OP_DacMuteType_SquareWaveSame	/* Square_Wave_Same */
} BRAP_OP_DacMuteType;

/***************************************************************************
Summary:
    This enumeration lists all MClock rates that can be used for DAC.

Description:
    Indicates the frequency of MCLK as a multiple of Fs.

See Also:

***************************************************************************/  
typedef enum BRAP_OP_DacMClkRate
{
    BRAP_OP_DacMClkRate_e64Fs,   /* MCLK rate is 64fs */	
    BRAP_OP_DacMClkRate_e256Fs  /* MCLK rate is 256fs */
}BRAP_OP_DacMClkRate;


/***************************************************************************
Summary:
    This enumeration lists all the possible ways to output Left and Right Data through DAC.

Description:

    
See Also:

***************************************************************************/  
typedef enum BRAP_OP_DacLROutputCtrl
{
	BRAP_OP_DacLROutputCtrl_eBothLRZeros,	/* Outputs zeros for both Left and Right channel */
	BRAP_OP_DacLROutputCtrl_eOnlyLZeros,	/* Outputs zeros for Left channel only */
	BRAP_OP_DacLROutputCtrl_eOnlyRZeros,	/* Outputs zeros for Right channel only */    
	BRAP_OP_DacLROutputCtrl_eNoZeros		/* Ouputs the same value as it receives */
} BRAP_OP_DacLROutputCtrl;

/***************************************************************************
Summary:
    Parameters to be passed by external application on Opening the MAI output port.
    
Description:
    Note: MAI Format Word parameters are the values that go into 
       the MAI Format Word. These parameters are mainly dependent upon how the 
       MAI client want to receive the data. For example, if input data is 
       Stereo, both Right and Left Channel data will flow on the MAI bus. 
       But if eMAIAudioFormat indicates Left Mono, then the MAI client will 
       only read the Left Mono data. 

See Also:

***************************************************************************/
typedef struct BRAP_OP_MaiSettings
{
    BRAP_OutputPort         eMaiMuxSelector; /* MAI Input: SPDIF or FLEX. 
                                                Start-time changeable */     
    BRAP_OP_ChannelOrder    eChannelOrder;   /* LR output channels to be normal,
                                                swapped, or copies of the LR 
                                                input channels. On-the-fly 
                                                changeable */
    BRAP_OP_MaiAudioFormat  eMaiAudioFormat; /* Specifies the MAI format word. 
                                                Start-time changeable */
    bool                    bSpdifoutput;    /* States if the o/p of MAI should
                                                be of SPDIF format Or PCM. 
                                                TRUE = SPDIF format
                                                FALSE= PCM format. 
                                                Start-time changeable */ 
}BRAP_OP_MaiSettings;  

/***************************************************************************
Summary:
    Parameters to be passed by external application on Opening the SPDIF output port.

Description:
 
See Also:

***************************************************************************/ 
typedef struct BRAP_OP_SpdifSettings
{
    BRAP_OP_ChannelOrder    eChannelOrder;  /* LR output channels to be normal,
                                               swapped, or copies of the LR
                                               input channels. On-the-fly 
                                               changeable.*/
    bool                    bLimitTo16Bits; /* Limit to 16 bits. On-the-fly 
                                               changeable.*/
    BRAP_OP_MClkRate        eMClkRate;      /* MClk Rate selection. Start-time changeable. */
    BRAP_OP_Pll             ePll;           /* PLL selection for SPDIF port. Start-time changeable. */
}BRAP_OP_SpdifSettings;
    
/***************************************************************************
Summary:
    Parameters to be passed by external for the SPDIF channel status bits
    Refer to SPDIF specs for details on the channel status bits.

Description:
    These are Start-time changeable only ie can be set only before starting the 
    output port/channel. Cannot be changed while the output port is running.
    Apply only to SPDIF, MAI and Flex ports.
 
See Also:

***************************************************************************/ 
typedef struct BRAP_OP_SpdifChanStatusParams
{
    bool           bProfessionalMode;    /* [0:0] The professional mode flag.
                                            TRUE: Professional mode. Other user
                                            options will not be considered.
                                            FALSE: Consumer mode.*/
    bool           bLinear_PCM;         /* [1:1] For output Linear  bLinear_PCM=true,
                                                                If output is Compressed bLinear_PCM=false*/
    bool           bSwCopyRight;         /* [2:2] Software CopyRight assert.
                                            TRUE: CopyRight is asserted
                                            FALSE: CopyRight is not asserted */
    uint16_t        uiPre_Emphasis; /*[3:5] pre-emphasis */
    uint16_t        uiChannel_Status_Mode; /* [6:7] Gives channel status */
    uint16_t       ui16CategoryCode;     /* [8:15] Category Code */
    uint16_t        uiSource_Number;/*[16:19] Source  number*/
    bool            bSeparateLRChanNum; /*[20:23]  TRUE:  Left channel num = 0000
                                                   Right Channel Num = 0000
                                            FALSE: Left channel num = 1000
                                                   Right Channel Num = 0100 */
    uint16_t        ui16ClockAccuracy;    /* [28:29] Clock Accuracy */
    bool            bSampleWorldLength; /* [32:32] For setting of 32bit of Channel Status
									Bit 32 0 Maximum audio sample word length is 20 bits
										  1 Maximum audio sample word length is 24 bits */                                                 
    uint16_t        uiSample_Word_Length; /* [33:35] Sample word length */
    uint16_t        uiOriginal_Sampling_Frequency;/* [36:39] Original sampling frequency*/
    uint16_t        uiCGMS_A; /* [40:41] */
} BRAP_OP_SpdifChanStatusParams;


/***************************************************************************
Summary:
    Parameters to be passed by application on Opening the I2S output port.

Description:

See Also:

***************************************************************************/ 
typedef struct BRAP_OP_I2sSettings
{
    BRAP_OP_ChannelOrder     eChannelOrder;      /* LR output channels to be 
                                                    normal, swapped, or copies
                                                    of the LR input channels. 
                                                    On-the-fly changeable.*/
    bool                     bLsbAtLRClk;        /* Data Justification.
                                                    Controls whether LSB or 
                                                    MSB is at LRCK transition
                                                    TRUE: LSB, FALSE: MSB 
                                                    On-the-fly changeable.*/
    bool                     bAlignedToLRClk;    /* Controls whether data is 
                                                    aligned with LRCK or delayed
                                                    by one SCLK period
                                                    FALSE: Delayed. 
                                                    TRUE: Aligned 
                                                    On-the-fly changeable.*/
    bool                     bLRClkPolarity;     /* Sets the polarity of the 
                                                    left/right clock
                                                    TRUE: High for Left
                                                    FALSE: Low for Left 
                                                    On-the-fly changeable.*/
    BRAP_OP_SClkRate         eSClkRate;          /* SClk (bit Clock) rate 
                                                    On-the-fly changeable.*/
    BRAP_OP_MClkRate         eMClkRate;          /* MClk rate. Start-time 
                                                    changeable. */
    BRAP_OP_Pll              ePll;               /* PLL selection. Start-time 
                                                    changeable. */
}BRAP_OP_I2sSettings;

/***************************************************************************
Summary:
   Parameters passed by application on Opening the Flexible output port
   TODO: Other parameters will be added after complete implementation
   of FLEX port.
    
Description:

See Also:

***************************************************************************/ 
typedef struct BRAP_OP_FlexSettings
{
    BRAP_OP_SClkRate         eSClkRate;          /* SClk (bit Clock) rate */
    BRAP_OP_MClkRate        eMClkRate; /* MClk Rate. Start-time 
                                          changeable. */
    BRAP_OP_Pll             ePll;      /* PLL selection. Start-time 
                                          changeable. */
}BRAP_OP_FlexSettings;


/***************************************************************************
Summary:
    Parameters passed by application on Opening the DAC output port
    
Description:

See Also:

***************************************************************************/ 
typedef struct BRAP_OP_DacSettings
{
    BRAP_OP_ChannelOrder     eChannelOrder;      /* LR output channels to be 
                                                    normal, swapped, or copies
                                                    of the LR input channels.
                                                    On-the-fly changeable.*/
    BRAP_OP_DacMuteType     eMuteType;  /* S/W Mute type.
                                           On-the-fly changeable.*/
}BRAP_OP_DacSettings;

/***************************************************************************
Summary:
    Parameters to be passed by external for enabling a test tone throuh DAC.
    
Description:

See Also:

***************************************************************************/
typedef struct BRAP_OP_DacToneInfo
{
	int32_t  aSample[64];  							/* test tone samples */
	BRAP_OP_DacLROutputCtrl	eLROutputControl;	/* 0 = Both L & R Zeros
													1=Left alone zero
													2=Right alone zero
													3=No Zeros on both L and R */
	bool		bSampleBufferMode;	/* True = share;  the same test tone
								is generated for both left and right
								from all samples 0-63.
								false = Dont Share; samples 0-31 for left and
								32-63 for right */
	uint8_t	uiRightRepeat;	/* Number of samples minus 1 in test 
							tone for right channel before repeating. 
							*/
	uint8_t	uiLeftRepeat;	/* Number of samples minus 1 in test 
							tone for left channel before repeating. 
							*/
	BAVC_AudioSamplingRate eSamplingRate;	/* Sampling frequecny
											* of the DAC to be set
											*/
} BRAP_OP_DacToneInfo;

/***************************************************************************
Summary:
    Parameters to be passed for configuring the external Master clocks
    
Description:

See Also:

***************************************************************************/
typedef struct BRAP_OP_ExtMClkSettings
{
	BRAP_OP_Pll			ePll;			/* Desired Pll to be used */
	BRAP_OP_MClkRate	eMClockRate;	/* Desired Master Clock rate */
} BRAP_OP_ExtMClkSettings;


/***************************************************************************
Summary:
    Mute/UnMute data at the output port. 
    The Mute status is maintained over channel change.

Description:
    This Mute can be used for all output ports irrespective of the data type
    ie compressed or PCM data.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_OP_GetMute().
    
**************************************************************************/
BERR_Code BRAP_OP_SetMute ( 
    BRAP_Handle     hRap,    /* [in]Audio Device Handle */
    BRAP_OutputPort eOpType, /* [in]Output Type */
    bool            bMute    /* [in]TRUE: Mute data at the output port
                                    FALSE: UnMute data at the output port */
);

/***************************************************************************
Summary:
    Gets whether output of the mixer is muted or not.

Description:
    It can be used only for uncompressed data.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_OP_SetMute().
    
**************************************************************************/
BERR_Code BRAP_OP_GetMute ( 
    BRAP_Handle     hRap,    /* [in]Audio Device Handle */
    BRAP_OutputPort eOpType, /* [in]Output Type */
    bool *          pMute   /* [out] Pointer to memory where the Mute 
                                status is to be written
                                TRUE: output port is Muted
                                FALSE:  output port is not Muted */
);

/***************************************************************************
Summary:
	Enable/Disable the Test tone

Description:
	This API plays the samples stored in the test tone buffer through
	the specified DAC. This can be used even before a decode channel is 
	opened and just after opening the raptor device. The samples for the
	test tone generaion is given by the application or if application has asked
	to use the default test-tone then PI uses that for test tone generation.
	If bEnable = False, it stops the enabled testtone.

Returns:
	BERR_SUCCESS 

See Also:
	
****************************************************************************/
BERR_Code
BRAP_OP_DacEnableTestTone(
    BRAP_Handle     hRap,				/* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,		/* [in] Output Type */
    bool	bEnable,					/* [in] True = Enable; False = Disable */
    const BRAP_OP_DacToneInfo *pToneInfo	/* [in] Test tone control structure */
);

/***************************************************************************
Summary:
	To get the default testtone info

Description:
	This API gives the default test tone settings for the app.
	
Returns:
	BERR_SUCCESS 

See Also:
	
****************************************************************************/
BERR_Code
BRAP_OP_DacGetDefaultTestTone(
    BRAP_Handle     hRap,					/* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,			/* [in] Output Type */
    BRAP_OP_DacToneInfo *pToneInfo		/* [out] Test tone control structure */
);

/***************************************************************************
Summary:
	Gets the output sampling rate.
Description:
	This API is used to get the output sampling rate and the timebase associated
	with output port for a particular rap channel.
Returns:
	BERR_SUCCESS
See Also:
	BRAP_OP_SetOutputSamplingRate	
****************************************************************************/
BERR_Code BRAP_OP_GetSamplingRate ( 
	BRAP_Handle 			hRap, 				/* [in] The RAP Decode Channel handle */
	BRAP_OutputPort			eOutputPort,		/* [in] Output port */
	BAVC_AudioSamplingRate  *peSamplingRate, 	/* [out] Output Sampling rate */
	BAVC_Timebase			*peTimebase 		/* [out] Timebase to use for the output port */
	);

/***************************************************************************
Summary:
	Sets the sampling rate associated with the output port.
Description:
	This API is used to set the sampling rate associated with a particular 
	output port. Note: This should not be called with an intention to change 
	output sampling rate for an output which gets data from a decode channel. 
	Decode channel decides on its own the output sampling rate.
Returns:
	BERR_SUCCESS on success, else error
See Also:
	BRAP_OP_GetOutputSamplingRate
****************************************************************************/
BERR_Code BRAP_OP_SetSamplingRate ( 
	const BRAP_Handle 		hRap, 			/* [in] The RAP Channel handle */
	const BRAP_OutputPort	eOutputPort, 	/* [in] Output port */
	BAVC_AudioSamplingRate  eSamplingRate 	/* [in] Output Sampling rate */
	);

/***************************************************************************
Summary:
	Configures the external Master Clocks
Description:
	This API is used to configure the external master clock for the specified pll and
	MClock rate.
Returns:
	BERR_SUCCESS 
See Also:

****************************************************************************/
BERR_Code BRAP_OP_ExtMClkConfig ( 
	const BRAP_Handle 		hRap, 				/* [in] The RAP Channel handle */
	const BRAP_OP_ExtMClk	eMClock,			/* [in] The MClock output */
	BRAP_OP_ExtMClkSettings	*pExtMClkSettings	/* [in] External Master clock settings */
	);

#if ( BCHP_CHIP == 7400 )
/***************************************************************************
Summary:
	Selects the source for RFMOD output port
Description:
	This API is used to associate the Rfmod output to the DAC output port
Returns:
	BERR_SUCCESS else error
See Also:

****************************************************************************/
BERR_Code
BRAP_OP_SetRfmodSource(
	BRAP_Handle hRap,
	BRAP_OutputPort eRfmodSource
	);
#endif

#if (BRAP_7401_FAMILY == 1)
/***************************************************************************
Summary:
	Programs the PLL dividers.
	
Description:
	This API is used to program the PLL dividers with the correct values to get the 
	clock frequency mentioned in ulClkFreq. 
	
Returns:
	BERR_SUCCESS on success, else error
	
See Also:
****************************************************************************/
BERR_Code 
BRAP_OP_ProgramPll (
	BRAP_Handle	hRap,		/* [in]Audio Device Handle */
	BRAP_OP_Pll	ePll,		/* [in]PLL device to be programmed */
	unsigned long	ulClkFreq	/* [in]The actual clock frequency. Currently we support only
							24000000 and 36864000 */
	);
#endif

#ifdef __cplusplus
}
#endif


#endif /* _BRAP_OP_H_ */

/* End of File */
