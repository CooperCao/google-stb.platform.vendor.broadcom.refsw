/***************************************************************************
*     Copyright (c) 2003-2011, Broadcom Corporation
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
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description:	This is the header file that contains the interface
*						between PI and firmware for updating the status of 
*						decode,encode or audio processing stage at any time 
*						within a task. The CIT module will allocate memory 
*						for decode/encode/audio processing stage in a task 
*						and send the DRAM address of that stage status through 
*						CIT output structure to the PI.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef BRAP_FWSTATUS_PRIV_H__

#define BRAP_FWSTATUS_PRIV_H__

#define BRAP_FWSTATUS_STATUS_INVALID    0



/*********************************************************************
Summary:
    This structure defines all the stream information parameters of 
	DTSHD Broadcast  Decoder which is required by PI at every frame boundary.
Description:

See Also:
**********************************************************************/ 
typedef struct BRAP_FWIF_P_DtsHdFrameInfo			
{			
	uint32_t	ui32NumOfChannels;		
	uint32_t	ui32SampleRate;	
	/*
		3	:	32 Khz
		8	:	44.1 KHz
		13	:	48Khz
		12	:	24Khz
		11	:	12Khz
	*/
	uint32_t	ui32ChannelAllocation;	
	uint32_t	ui32LFEPresent;	
	uint32_t	ui32BitsPerSample;	
	uint32_t	ui32ErrorStatus;	
	uint32_t	ui32CoreExtensionMask;	
	uint32_t	ui32DTSNeoEnable;	
	uint32_t	ui32EsFlag;	
	uint32_t	ui32ExtendedCodingFlag;	
	uint32_t	ui32ExtensionAudioDescriptorFlag;	
	uint32_t	ui32PCMFrameSize;	

} BRAP_FWIF_P_DtsHdFrameInfo;	

typedef struct BRAP_FWIF_P_DtsHdStreamInfo	
{		
	uint32_t	  ui32CrcFlag;
	uint32_t	  ui32NBlocks;
	uint32_t	  ui32Fsize;
	uint32_t	  ui32Amode;
	uint32_t	  ui32TransRate;
	uint32_t	  ui32DownMix;
	uint32_t	  ui32DynRangeCoeff;
	uint32_t	  ui32HdcdFormat;
	uint32_t	  ui32ExtAudioId;
	uint32_t	  ui32ExtAudioFlag;
	uint32_t	  ui32VerNum;
	uint32_t	  ui32CopyHist;
	uint32_t	  ui32PcmResolution;

	BRAP_FWIF_P_DtsHdFrameInfo	sDtsFrameInfo;	
/*
	This is the cumulative count for number of Valid frames decoded
*/
	uint32_t	ui32TotalFramesDecoded;
/*
	This is the cumulative count for number of Dummy decodes done by decoder
*/

	uint32_t	ui32TotalFramesDummy;
/*
	This is the cumulative count for number of erroneous frames detected by decoder.
*/

	uint32_t	ui32TotalFramesInError;

	uint32_t		ui32StatusValid;	

} BRAP_FWIF_P_DtsHdStreamInfo;	


/*********************************************************************
Summary:
    This structure defines all the stream information parameters of 
	DTS Broadcast  Encoder which is required by PI at every frame boundary.
Description:

See Also:
**********************************************************************/ 
typedef struct BRAP_FWIF_P_DTSBroadcastEncoderStreamInfo
{
	
/*	 0 = A (Mono) 
	 1 = A + B (dual mono)
	 2 = L + R (stereo) 
	 3 = (L+R) + (L-R) (sum-difference)
	 4 = LT +RT (left and right total)
	 5 = C + L + R 
	 6 = L + R+ S 
	 7 = C + L + R+ S 
	 8 = L + R+ SL+SR 
	 9 = C + L + R+ SL+SR 
	 10 = CL + CR + L + R + SL + SR 
	 11 = C + L + R+ LR + RR + OV 
	 12 = CF+ CR+LF+ RF+LR + RR 
	 13 = CL + C + CR + L + R + SL + SR 
	 14 = CL + CR + L + R + SL1 + SL2+ SR1 + SR2 
	 15 = CL + C+ CR + L + R + SL + S+ SR
*/ 
	uint32_t ui32AudioMode;

/* Sampling frequency */
	uint32_t ui32SamplingFreq;

/* Bit Rate */
	uint32_t ui32BitRate;

/* LFE Flag
	1 = LFE ON
	0 = LFE OFF
*/
	uint32_t ui32LFEFlag;


/* This field tells whether the stream info fields are valid or not */
	uint32_t ui32StatusValid;
 
}BRAP_FWIF_P_DTSBroadcastEncoderStreamInfo;
/*********************************************************************
Summary:
    This structure defines all the stream information parameters of 
	WMA decoder which is required by PI at every frame boundary.

Description:

See Also:
**********************************************************************/ 

typedef struct BRAP_FWIF_P_WmaStreamInfo
{


/*
	The field tells the presence of original copy
	0 = Copy
	1 = Original
*/
    uint32_t    ui32OriginalCopy;
/*
	The field tells the presence of copyright
	0 = Copyright_Absent
	1 = Copyright_Present
*/
    uint32_t    ui32Copyright;
/*
	The stereo mode as defined by the ISO 11172-3 audio specification.
*/
    uint32_t    ui32Mode;
/*
	The sampling frequency value as defined by the ISO 11172-3 audio specification.
	0 = Sampling_44_1_kHz
	1 = Sampling_48_kHz
	2 = Sampling_32_kHz
	3 = Reserved
*/
    uint32_t    ui32SamplingFreq;
/*
	The bitrate value as defined by the ISO 11172-3 audio specification.
*/
    uint32_t    ui32BitRate;
/*
	The field tells the presence of CRC
	0 = CRC_Absent
	1 = CRC_Present
*/
    uint32_t    ui32CrcPresent;

/* ACMOD
	
	ui32Acmod =1 One Channel	
	ui32Acmod =2 Two Channel
	
*/
	uint32_t        ui32Acmod;

/* Version

	0 = Version1	
	1 = Version2
  
*/
    uint32_t        ui32Version;

/*
	Low Frequency Effects Channel 
	0 = LFE_channel_off
	1 = LFE_channel_ON
*/
	uint32_t    ui32LfeOn;

/*
	This is the cumulative number of samples decoded by any decoder from the
	channel open time. Decoder should accumulate number of samples decoded for
	every frame. Dummy samples are not included.
*/
	uint32_t	ui32NumSamplesDecoded;

/*
	This is the cumulative count for number of Valid frames decoded
*/
	uint32_t	ui32TotalFramesDecoded;
/*
	This is the cumulative count for number of Dummy decodes done by decoder
*/

	uint32_t	ui32TotalFramesDummy;
/*
	This is the cumulative count for number of erroneous frames detected by decoder.
*/

	uint32_t	ui32TotalFramesInError;

/* This field tells whether the stream info fields are valid or not */
    uint32_t	ui32StatusValid;


}BRAP_FWIF_P_WmaStreamInfo;



/*********************************************************************
Summary:
    This structure defines all the stream information parameters of 
	WMAPro decoder which is required by PI at every frame boundary.

Description:

See Also:
**********************************************************************/ 

typedef struct BRAP_FWIF_P_WmaProStreamInfo
{


/*
	The field tells the presence of original copy
	0 = Copy
	1 = Original
*/
    uint32_t    ui32OriginalCopy;
/*
	The field tells the presence of copyright
	0 = Copyright_Absent
	1 = Copyright_Present
*/
    uint32_t    ui32Copyright;
/*
	The stereo mode as defined by the ISO 11172-3 audio specification.
*/
    uint32_t    ui32Mode;
/*
	The sampling frequency value as defined by the ISO 11172-3 audio specification.
	0 = Sampling_44_1_kHz
	1 = Sampling_48_kHz
	2 = Sampling_32_kHz
	3 = Reserved
*/
    uint32_t    ui32SamplingFreq;
/*
	The bitrate value as defined by the ISO 11172-3 audio specification.
*/
    uint32_t    ui32BitRate;
/*
	The field tells the presence of CRC
	0 = CRC_Absent
	1 = CRC_Present
*/
    uint32_t    ui32CrcPresent;

/* ACMOD
	
	ui32Acmod =0 to 7
	
*/
	uint32_t        ui32Acmod;

/* Version

	0 = Version1	
	1 = Version2
  
*/
    uint32_t        ui32Version;

/*
	Low Frequency Effects Channel 
	0 = LFE_channel_off
	1 = LFE_channel_ON
*/
	uint32_t    ui32LfeOn;

/*
	This is the cumulative number of samples decoded by any decoder from the
	channel open time. Decoder should accumulate number of samples decoded for
	every frame. Dummy samples are not included.
*/
	uint32_t	ui32NumSamplesDecoded;

/*
	This is the cumulative count for number of Valid frames decoded
*/
	uint32_t	ui32TotalFramesDecoded;
/*
	This is the cumulative count for number of Dummy decodes done by decoder
*/

	uint32_t	ui32TotalFramesDummy;
/*
	This is the cumulative count for number of erroneous frames detected by decoder.
*/

	uint32_t	ui32TotalFramesInError;

/* This field tells whether the stream info fields are valid or not */
    uint32_t	ui32StatusValid;


}BRAP_FWIF_P_WmaProStreamInfo;



/*********************************************************************
Summary:
    This structure defines all the stream information parameters of AC-3
	decoder which is required by PI at every frame boundary.

Description:

See Also:
**********************************************************************/ 

typedef struct BRAP_FWIF_P_Ac3StreamInfo
{

/*
	This field decides the sampling frequency of the AC-3 stream. 
	0 = Sampling_48_kHz
	1 = Sampling_44_1_kHz
	2 = Sampling_32_kHz		
*/
    uint32_t    ui32SamplingFrequency;
/*
	The bit stream identification value. Bit stream identification
	value in this standard is 8. The future modifications of this 
	standard may define other values.
*/
    uint32_t    ui32BitStreamIdentification;
/*
	Transmission mode 
	0 = main_audio_service_complete_main_CM
	1 = main_audio_service_music_and_effects_ME
	2 = associated_service_visually_impaired_VI
	3 = associated_service_hearing_impaired_HI
	4 = associated_service_dialogue_D
	5 = associated_service_commentary_C
	6 = associated_service_emergency_E
	7 = associated_service_stream_identification_along_with_ACMOD_value
*/
    uint32_t    ui32BsmodValue;
/*
	Audio coding mode which describes the number of input channels. 
	0 = Two_mono_channels_1_1_ch1_ch2
	1 = One_centre_channel_1_0_C
	2 = Two_channels_2_0_L__R
	3 = Three_channels_3_0_L_C_R
	4 = Three_chanels_2_1_L_R_S
	5 = Four_channels_3_1_L_C_R_S
	6 = Four_channels_2_2_L_R_SL_SR
	7 = Five_channels_3_2_L_C_R_SL_SR
*/
	uint32_t    ui32AcmodValue;
/*
	Dolby Surround Mode (dsurmod)  
	0 = Not_indicated
	1 = NOT_Dolby_Surround_encoded
	2 = Dolby_Surround_encoded
*/
	uint32_t    ui32DsurmodValue;
/*
	Low Frequency Effects Channel 
	0 = LFE_channel_off
	1 = LFE_channel_ON
*/
	uint32_t    ui32LfeOn;
/*
	This field indicates whether the AC-3 language code exists or not. 
	0 = Langauge_code_not_exists
	1 = Language_code_exists
*/
	uint32_t    ui32LangCodeExists;
	/* This field indicates the language code value. */
	uint32_t    ui32LangCodeValue;
/*
	This field indicates the room type value. 
	0 = Not_indicated
	1 = Large_room
	2 = Small_room
*/
	uint32_t    ui32RoomTypeValue;
/*
	This field indicates the room type2 value. 
	0 = Not_indicated
	1 = Large_room
	2 = Small_room
*/
	uint32_t	ui32RoomType2Value;
/* 
	This field indicates that stream is protected by copyright. 
	0 = stream_not_copyright
	1 = stream_copyright
*/
	uint32_t    ui32CopyrightBit;
/*
	This field indicates the stream is original bit stream. 
	0 = Stream_not_original
	1 = Stream_original
*/
	uint32_t	ui32OriginalBitStream;

/*
	AC-3 centre mix level value 
	0 = three_dB_below
	1 = four_and_half_dB_below
	2 = six_dB_below
	3 = RESERVED
*/
	uint32_t	ui32CmixLevel;
/*
	AC-3 surround mix level value 
	0 = three_dB_below
	1 = six_dB_below
	2 = zero_dB_below
	3 = RESERVED
*/
	uint32_t	ui32SurmixLevel;
/*
	This field indicates whether the AC-3 language code for the second channel exists or not. 
	0 = Langauge_code_not_exists
	1 = Language_code_exists
*/
	uint32_t	ui32Langcode2Exists;
/*
	This field indicates the language code value for the second channel.
*/
	uint32_t	ui32Langcode2Value;
/*
	This field indicates whether the time code exits or not. 
	0 = Not_present
	1 = First_half_present
	2 = Second_half_present
	3 = Both_halves_present
*/
	uint32_t	ui32TimeCodeExists;
/*
This is time code for low resolution in 8 seconds increments up to 24 hours. First 5 bits 
indicates the time in hours. Next 6 bits represents the time in minutes. Final 3 bits 
represents the time in 8 seconds increments.
*/
	uint32_t	ui32TimeCodeFirstHalf;
/*
	Output LFE Channel Configuration
	1 = LFE_ON
	0 = LFE_OFF
*/
	uint32_t	ui32OutputLfeMode;
/*
	Ouput channel Configuration
	0 = Two_mono_channels_1_1_ch1_ch2
	1 = One_centre_channel_1_0_C
	2 = Two_channels_2_0_L__R
	3 = Three_channels_3_0_L_C_R
	4 = Three_chanels_2_1_L_R_S
	5 = Four_channels_3_1_L_C_R_S
	6 = Four_channels_2_2_L_R_SL_SR
	7 = Five_channels_3_2_L_C_R_SL_SR
*/
	uint32_t	ui32OutputMode;
/*
	The frame size code is used in conjunction with the sample rate code to determine 
	the number of 16-bit words before the next syncword.
*/
	uint32_t	ui32FrmSizeCod;
/*
	This is time code for high resolution in 1/64th frame increments up to 8 seconds. 
	The first 3 bits indicate the time in seconds. The next 5 bits represent the time 
	in frames. The final 6 bits represent the time in fractions of 1/64th of a frame.
*/
	uint32_t	ui32TimeCodeSecondHalf;
/*
	This is the cumulative number of samples decoded by any decoder from the
	channel open time. Decoder should accumulate number of samples decoded for
	every frame. Dummy samples are not included.
*/
	uint32_t	ui32NumSamplesDecoded;

/*
	This is the cumulative count for number of Valid frames decoded
*/
	uint32_t	ui32TotalFramesDecoded;
/*
	This is the cumulative count for number of Dummy decodes done by decoder
*/

	uint32_t	ui32TotalFramesDummy;
/*
	This is the cumulative count for number of erroneous frames detected by decoder.
*/

	uint32_t	ui32TotalFramesInError;

/* This field tells whether the stream info fields are valid or not */
	uint32_t	ui32StatusValid;

}BRAP_FWIF_P_Ac3StreamInfo;


/*********************************************************************
Summary:
     This structure defines all the stream information parameters of DDP
	decoder which is required by PI at every frame boundary.
Description:

See Also:
**********************************************************************/


typedef struct BRAP_FWIF_P_DdpStreamInfo
{

/*
	This field decides the sampling frequency of the AC-3 stream. 
	0 = Sampling_48_kHz
	1 = Sampling_44_1_kHz
	2 = Sampling_32_kHz		
*/
    uint32_t    ui32SamplingFrequency;
/*
	The bit stream identification value. Bit stream identification
	value in this standard is 8. The future modifications of this 
	standard may define other values.
*/
    uint32_t    ui32BitStreamIdentification;
/*
	Transmission mode 
	0 = main_audio_service_complete_main_CM
	1 = main_audio_service_music_and_effects_ME
	2 = associated_service_visually_impaired_VI
	3 = associated_service_hearing_impaired_HI
	4 = associated_service_dialogue_D
	5 = associated_service_commentary_C
	6 = associated_service_emergency_E
	7 = associated_service_stream_identification_along_with_ACMOD_value
*/
    uint32_t    ui32BsmodValue;
/*
	Audio coding mode which describes the number of input channels. 
	0 = Two_mono_channels_1_1_ch1_ch2
	1 = One_centre_channel_1_0_C
	2 = Two_channels_2_0_L__R
	3 = Three_channels_3_0_L_C_R
	4 = Three_chanels_2_1_L_R_S
	5 = Four_channels_3_1_L_C_R_S
	6 = Four_channels_2_2_L_R_SL_SR
	7 = Five_channels_3_2_L_C_R_SL_SR
*/
	uint32_t    ui32AcmodValue;
/*
	Dolby Surround Mode (dsurmod)  
	0 = Not_indicated
	1 = NOT_Dolby_Surround_encoded
	2 = Dolby_Surround_encoded
*/
	uint32_t    ui32DsurmodValue;
/*
	Low Frequency Effects Channel 
	0 = LFE_channel_off
	1 = LFE_channel_ON
*/
	uint32_t    ui32LfeOn;
/*
	This field indicates whether the AC-3 language code exists or not. 
	0 = Langauge_code_not_exists
	1 = Language_code_exists
*/
	uint32_t    ui32LangCodeExists;
	/* This field indicates the language code value. */
	uint32_t    ui32LangCodeValue;
/*
	This field indicates the room type value. 
	0 = Not_indicated
	1 = Large_room
	2 = Small_room
*/
	uint32_t    ui32RoomTypeValue;
/*
	This field indicates the room type2 value. 
	0 = Not_indicated
	1 = Large_room
	2 = Small_room
*/
	uint32_t	ui32RoomType2Value;
/* 
	This field indicates that stream is protected by copyright. 
	0 = stream_not_copyright
	1 = stream_copyright
*/
	uint32_t    ui32CopyrightBit;
/*
	This field indicates the stream is original bit stream. 
	0 = Stream_not_original
	1 = Stream_original
*/
	uint32_t	ui32OriginalBitStream;

/*
	AC-3 centre mix level value 
	0 = three_dB_below
	1 = four_and_half_dB_below
	2 = six_dB_below
	3 = RESERVED
*/
	uint32_t	ui32CmixLevel;
/*
	AC-3 surround mix level value 
	0 = three_dB_below
	1 = six_dB_below
	2 = zero_dB_below
	3 = RESERVED
*/
	uint32_t	ui32SurmixLevel;
/*
	This field indicates whether the AC-3 language code for the second channel exists or not. 
	0 = Langauge_code_not_exists
	1 = Language_code_exists
*/
	uint32_t	ui32Langcode2Exists;
/*
	This field indicates the language code value for the second channel.
*/
	uint32_t	ui32Langcode2Value;
/*
	This field indicates whether the time code exits or not. 
	0 = Not_present
	1 = First_half_present
	2 = Second_half_present
	3 = Both_halves_present
*/
	uint32_t	ui32TimeCodeExists;
/*
This is time code for low resolution in 8 seconds increments up to 24 hours. First 5 bits 
indicates the time in hours. Next 6 bits represents the time in minutes. Final 3 bits 
represents the time in 8 seconds increments.
*/
	uint32_t	ui32TimeCodeFirstHalf;
/*
	Output LFE Channel Configuration
	1 = LFE_ON
	0 = LFE_OFF
*/
	uint32_t	ui32OutputLfeMode;
/*
	Ouput channel Configuration
	0 = Two_mono_channels_1_1_ch1_ch2
	1 = One_centre_channel_1_0_C
	2 = Two_channels_2_0_L__R
	3 = Three_channels_3_0_L_C_R
	4 = Three_chanels_2_1_L_R_S
	5 = Four_channels_3_1_L_C_R_S
	6 = Four_channels_2_2_L_R_SL_SR
	7 = Five_channels_3_2_L_C_R_SL_SR
*/
	uint32_t	ui32OutputMode;
/*
	The frame size code is used in conjunction with the sample rate code to determine 
	the number of 16-bit words before the next syncword.
*/
	uint32_t	ui32FrmSizeCod;
/*
	This is time code for high resolution in 1/64th frame increments up to 8 seconds. 
	The first 3 bits indicate the time in seconds. The next 5 bits represent the time 
	in frames. The final 6 bits represent the time in fractions of 1/64th of a frame.
*/
	uint32_t	ui32TimeCodeSecondHalf;

/*
	This is the cumulative number of samples decoded by any decoder from the
	channel open time. Decoder should accumulate number of samples decoded for
	every frame. Dummy samples are not included.
*/
	uint32_t	ui32NumSamplesDecoded;

/*
	This is the cumulative count for number of Valid frames decoded
*/
	uint32_t	ui32TotalFramesDecoded;
/*
	This is the cumulative count for number of Dummy decodes done by decoder
*/

	uint32_t	ui32TotalFramesDummy;
/*
	This is the cumulative count for number of erroneous frames detected by decoder.
*/

	uint32_t	ui32TotalFramesInError;
/*	
	This field tells whether the stream info fields are valid or not 
*/
	uint32_t	ui32StatusValid;

}BRAP_FWIF_P_DdpStreamInfo;


/*********************************************************************
Summary:
     This structure defines all the stream information parameters of 
	 MPEG-1 L1/L2/L3 decoder which is required by PI at every frame 
	 boundary.

Description:

See Also:
**********************************************************************/

typedef struct BRAP_FWIF_P_MpegStreamInfo
{

/*
	This field indicates the type of de-emphasis that shall be used.
	0 = None
	1 = Emph_50_15ms
	2 = Reserved
	3 = CCITT_J17
*/
	uint32_t    ui32Emphasis;
/*
	The field tells the presence of original copy
	0 = Copy
	1 = Original
*/
    uint32_t    ui32OriginalCopy;
/*
	The field tells the presence of copyright
	0 = Copyright_Absent
	1 = Copyright_Present
*/
    uint32_t    ui32Copyright;
/*
	The stereo mode as defined by the ISO 11172-3 audio specification.
*/
    uint32_t    ui32Mode;
/*
	The sampling frequency value as defined by the ISO 11172-3 audio specification.
	0 = Sampling_44_1_kHz
	1 = Sampling_48_kHz
	2 = Sampling_32_kHz
	3 = Reserved
*/
    uint32_t    ui32SamplingFreq;
/*
	The bitrate value as defined by the ISO 11172-3 audio specification.
*/
    uint32_t    ui32BitRate;
/*
	The field tells the presence of CRC
	0 = CRC_Absent
	1 = CRC_Present
*/
    uint32_t    ui32CrcPresent;
/*
	This field tells the MPEG layer type used
	1 = Layer3
	2 = Layer2
	3 = Layer1
*/
    uint32_t    ui32MpegLayer;
/*
	The value of this field is 
	1 = ISO11172_Audio
*/
	uint32_t    ui32AlgorithmId;

/*
	This is the cumulative number of samples decoded by any decoder from the
	channel open time. Decoder should accumulate number of samples decoded for
	every frame. Dummy samples are not included.
*/
	uint32_t	ui32NumSamplesDecoded;
/*
	This is the cumulative count for number of Valid frames decoded
*/
	uint32_t	ui32TotalFramesDecoded;
/*
	This is the cumulative count for number of Dummy decodes done by decoder
*/
	uint32_t	ui32TotalFramesDummy;
/*
	This is the cumulative count for number of erroneous frames detected by decoder.
*/
	uint32_t	ui32TotalFramesInError;
/* This field tells whether the stream info fields are valid or not */
    uint32_t	ui32StatusValid;

}BRAP_FWIF_P_MpegStreamInfo;

/*********************************************************************
Summary:
     This structure defines all the stream information parameters of 
	 AAC-HE  decoder which is required by PI at every frame 
	 boundary.

Description:

See Also:
**********************************************************************/
typedef struct BRAP_FWIF_P_AacheStreamInfo
{
	

/*
	The bitrate value as defined by the ISO 11172-3 audio specification.
*/
    uint32_t    ui32BitRate;


/*	AAC Profile: 
	0 = Main_profile
	1 = Low_complexity_profile
	2 = Scaleable_Sampling_Rate_profile
	3 = RESERVED

*/

	uint32_t	ui32Profile;

/*
	The sampling frequency value as defined by the ISO 11172-3 audio specification.
	0 = Sampling_96_kHz
	1 = Sampling_88_2_kHz
	2 = Sampling_64_kHz
	3 = Sampling_48_kHz
	4 = Sampling_44_1_kHz
	5 = Sampling_32_kHz
	6 = Sampling_24_kHz
	7 = Sampling_22_05_kHz
	8 = Sampling_16_kHz
	9 = Sampling_12_kHz
	10 = Sampling_11_025_kHz
	11 = Sampling_8_kHz
	12 = RESERVED_12
	13 = RESERVED_13
	14 = RESERVED_14
	15 = RESERVED_15
*/
    uint32_t    ui32SamplingFreq;

/*
	The number of LFE channel elements.
*/
	uint32_t	ui32NumLfeChannels;
/*
	The number of audio syntactic elements for the back channels.
*/
	uint32_t	ui32NumBackChannels;

/*
	The number of audio syntactic elements in the side.
*/
	uint32_t	ui32NumSideChannels;

/*
	The number of audio syntactic elements in the front channels, front center to 
	back center, symmetrically by left and right, or alternating by left and right
	in the case of single channel element.
*/
	uint32_t	ui32NumFrontChannels;

/*
	Output LFE Channel Configuration
	1 = LFE_ON
	0 = LFE_OFF
*/
	uint32_t	ui32OutputLfeMode;
/*
	Ouput channel Configuration
	0 = Two_mono_channels_1_1_ch1_ch2
	1 = One_centre_channel_1_0_C
	2 = Two_channels_2_0_L__R
	3 = Three_channels_3_0_L_C_R
	4 = Three_chanels_2_1_L_R_S
	5 = Four_channels_3_1_L_C_R_S
	6 = Four_channels_2_2_L_R_SL_SR
	7 = Five_channels_3_2_L_C_R_SL_SR
*/
	uint32_t	ui32OutputMode;

/*
	Low Frequency Effects Channel 
	0 = LFE_channel_off
	1 = LFE_channel_ON
*/
	uint32_t    ui32LfeOn;

/*
	Audio coding mode which describes the number of input channels. 
	0 = Two_mono_channels_1_1_ch1_ch2
	1 = One_centre_channel_1_0_C
	2 = Two_channels_2_0_L__R
	3 = Three_channels_3_0_L_C_R
	4 = Three_chanels_2_1_L_R_S
	5 = Four_channels_3_1_L_C_R_S
	6 = Four_channels_2_2_L_R_SL_SR
	7 = Five_channels_3_2_L_C_R_SL_SR
*/
	uint32_t    ui32AcmodValue;

/*
	A 1 bit field indicating that pseudo surround decoding is possible.
	0 = Disabled
	1 = Enabled
*/
	uint32_t		ui32PseudoSurroundEnable;

/*
	A 2 bit field that indicates the coefficient to be used in 5 to 2 matrix mixdown.
*/
	uint32_t		ui32MatrixMixdownIndex;

/*
	One bit indicating that a stereo matrix coefficient is present in the stream.
	0 = Mixdown_Absent
	1 = Mixdown_Present
*/
	uint32_t	ui32MatrixMixdownPresent;

/*
	This is a one bit flag that indicates whether DRC is present in the stream.
	0 = DRC_Absent
	1 = DRC_Present
*/
	uint32_t	ui32DrcPresent;

/*
	This is the cumulative number of samples decoded by any decoder from the
	channel open time. Decoder should accumulate number of samples decoded for
	every frame. Dummy samples are not included.
*/
	uint32_t	ui32NumSamplesDecoded;
/*
	This is the cumulative count for number of Valid frames decoded
*/
	uint32_t	ui32TotalFramesDecoded;
/*
	This is the cumulative count for number of Dummy decodes done by decoder
*/
	uint32_t	ui32TotalFramesDummy;
/*
	This is the cumulative count for number of erroneous frames detected by decoder.
*/
	uint32_t	ui32TotalFramesInError;
/* This field tells whether the stream info fields are valid or not */
    uint32_t	ui32StatusValid;

}BRAP_FWIF_P_AacheStreamInfo;


/*********************************************************************
Summary:
    This structure holds all the TSM related field values which is 
	required by PI at every frame boundary.

Description:

See Also:
**********************************************************************/ 

typedef struct BRAP_FWIF_P_TsmInfo
{
    uint32_t    ui32RunningPts;
    int32_t     i32PtsToStcPhase;
    uint32_t    ui32PtsType;
	uint32_t    ui32StatusValid;
	int32_t     i32TSMUpperThreshold;
}BRAP_FWIF_P_TsmInfo;


/*********************************************************************
Summary:
     This structure defines all the stream information parameters of 
	 DTS Core decoder which is required by PI at every frame 
	 boundary.

Description:

See Also:
**********************************************************************/

typedef struct BRAP_FWIF_P_DtsCoreStreamInfo
{
	/*
	This field specifies the sampling frequency of the audio samples in
	the DTS core encoder.
	0 = Invalid_0
	1 = Sampling_8_kHz
	2 = Sampling_16_kHz
	3 = Sampling_32_kHz
	4 = Invalid_4
	5 = Invalid_5
	6 = Sampling_11_025_kHz
	7 = Sampling_22_05_kHz
	8 = Sampling_44_1_kHz
	9 = Invalid_9
	10 = Invalid_10
	11 = Sampling_12_kHz
	12 = Sampling_24_kHz
	13 = Sampling_48_kHz
	14 = Invalid_14
	15 = Invalid_15
	*/
	uint32_t    ui32SamplingFreq;
	/*
	This field specifies the number of audio channels and the audio playback
	arrangement.
	0 = One_ch_A
	1 = Two_ch_A_B
	2 = Two_ch_L_R
	3 = Two_ch_LpR_LmR
	4 = Two_ch_LT_RT
	5 = Three_ch_C_L_R
	6 = Three_ch_L_R_S
	7 = Four_ch_C_L_R_S
	8 = Four_ch_L_R_SL_SR
	9 = Five_ch_C_L_R_SL_SR
	10 = Six_ch_CL_CR_L_R_SL_SR
	11 = Six_ch_C_L_R_LR_RR_OV
	12 = Six_ch_CF_CR_LF_RF_Lr_Rr
	13 = Seven_ch_CL_C_CR_L_R_SL_SR
	14 = Eight_ch_CL_CR_L_R_SL1_SL2_SR1_SR2
	15 = Eight_ch_CL_C_CR_L_R_SL_S_SR
	16 = User_Defined_16
	
	*/
	uint32_t ui32AcmodValue;
	/*
	This field contains the total byte size of the current frame
	including primary and any extension audio data.Valid range
	is 95 to 16383. Invalid range is 0 to 94.
	*/
	uint32_t ui32FrameSize;
	/*
	This field specifies the number block of 32 PCM samples per channel.
	Invalid range is 0 to 4.
	*/
	uint32_t ui32NumPcmBlocks;
	/*
	This field indicates the presence of CRC in the bitstream. 
	0 = Not_present
	1 = Present
	*/
	uint32_t ui32CrcFlag;
	/*
	This field indicates the quantization resolution of source PCM samples.
	The left and right surround channels of the source material are mastered
	in DTS ES format if ES = 1. 
	0 = ES0_16_bits
	1 = ES1_16_bits
	2 = ES0_20_bits
	3 = ES1_20_bits
	4 = ES0_24_bits
	5 = ES1_24_bits
	6 = Invalid_6
	7 = Invalid_7
	
	*/
	uint32_t ui32SourcePcm;
	/*
	This field indicates the copy history of the audio. 
	0 = Copy_prohibited
	1 = First_generation
	2 = Second_generation
	3 = Original_material
	*/
	uint32_t ui32CopyHistory;
	/*
	This field indicates the revision status of the encoder software.
	In addition it is used to indicate the presence of the dialog 
	normalization parameters.
	A value of 0-6 indicates a future (compatible) revision.
	A value of 7 indicates the current version.
	A value of 8-15 indicates a future (incompatible) revision.
	*/
	uint32_t ui32VersionNumber;
	/*
	Indicates if the LFE channel is present and the choice of the 
	interpolation factor.
	0 = Not_present
	1 = Present_128
	2 = Present_64
	3 = Invalid
	*/
	uint32_t ui32LfePresentFlag;
	/*
	This field indicates if extended audio coding data are present after 
	the core audio data.
	0 = Not_present
	1 = Present
	*/
	uint32_t ui32ExtAudio;
		/*
		This flag has meaning only if EXT_AUDIO = 1 and then it indicates 
		the type of data that has been placed in the extension stream(s).
		0 = XCh
		1 = XXCh
		2 = X96k
		3 = Reserved_3
		4 = Reserved_4
		5 = Reserved_5
		6 = Reserved_6
		7 = Reserved_7
	*/
	uint32_t ui32ExtAudioId;
	/*
	The source material is mastered in HDCD format if HDCD=1, 
	and otherwise HDCD=0.
	*/
	uint32_t ui32HdCdFormat;
	
	/*
	The embedded dynamic range coefficients are included at the start of 
	each subframe.
	0 = Not_present
	1 = Present
	*/
	uint32_t ui32EmbDrc;
	
	/*
	The embedded downmixing coefficients are included at the start of each 
	subframe.
	0 = Not_present
	1 = Present
	*/
	uint32_t ui32EmbDownmixEnable;
	
	/*
	The average data rate for the current frame of audio.
	0 = kbps_32
	1 = kbps_56
	2 = kbps_64
	3 = kbps_96
	4 = kbps_112
	5 = kbps_128
	6 = kbps_192
	7 = kbps_224
	8 = kbps_256
	9 = kbps_320
	10 = kbps_384
	11 = kbps_448
	12 = kbps_512
	13 = kbps_576
	14 = kbps_640
	15 = kbps_768
	16 = kbps_960
	17 = kbps_1024
	18 = kbps_1152
	19 = kbps_1280
	20 = kbps_1344
	21 = kbps_1408
	22 = kbps_1411_2
	23 = kbps_1472
	24 = kbps_1536
	25 = kbps_1920
	26 = kbps_2048
	27 = kbps_3072
	28 = kbps_3840
	29 = Open
	30 = Variable
	31 = Lossless
	*/
	uint32_t ui32BitRate;
	
	/*
	Output LFE Channel Configuration
	1 = LFE_ON
	0 = LFE_OFF
	*/
	uint32_t	ui32OutputLfeMode;
	/*
	Ouput channel Configuration
	0 = Two_mono_channels_1_1_ch1_ch2
	1 = One_centre_channel_1_0_C
	2 = Two_channels_2_0_L__R
	3 = Three_channels_3_0_L_C_R
	4 = Three_chanels_2_1_L_R_S
	5 = Four_channels_3_1_L_C_R_S
	6 = Four_channels_2_2_L_R_SL_SR
	7 = Five_channels_3_2_L_C_R_SL_SR
	*/
	uint32_t	ui32OutputMode;

	/*
	This is the cumulative number of samples decoded by any decoder from the
	channel open time. Decoder should accumulate number of samples decoded for
	every frame. Dummy samples are not included.
	*/
	uint32_t	ui32NumSamplesDecoded;
	/*
	This is the cumulative count for number of Valid frames decoded
	*/
	uint32_t	ui32TotalFramesDecoded;
	/*
	This is the cumulative count for number of Dummy decodes done by decoder
	*/

	uint32_t	ui32TotalFramesDummy;
	/*
	This is the cumulative count for number of erroneous frames detected by decoder.
	*/

	uint32_t	ui32TotalFramesInError;

	
	/* This field tells whether the stream info fields are valid or not */
    uint32_t	ui32StatusValid;

}BRAP_FWIF_P_DtsCoreStreamInfo;
/*********************************************************************
Summary:
     This structure defines all the stream information parameters of 
	 LPCM decoder which is required by PI at every frame 
	 boundary.

Description:

See Also:
**********************************************************************/

typedef struct BRAP_FWIF_P_LpcmStreamInfo
{
	
  /*This field indicates the size in bytes of the LPCM audio frame*/
   uint32_t    ui32FrameSize;	
	
  /*This field specifies the channel assignment for the channel configuration in the LPCM audio stream.
	0 = Reserved_0
	1 = Mono
	2 = Reserved_2
	3 = L_R_Stereo
	4 = L_C_R_3_0
	5 = L_R_S_2_1
	6 = L_C_R_S_3_1
	7 = L_R_Ls_Rs_2_2
	8 = L_C_R_Ls_Rs_3_2
	9 = L_C_R_Ls_Rs_LFE_3_2
	10 = L_C_R_Ls_Cs1_Cs2_Rs_3_4
	11 = L_C_R_Ls_Cs1_Cs2_Rs_LFE_3_4
	12 = Reserved_12
	13 = Reserved_13
	14 = Reserved_14
	15 = Reserved_15  */
    uint32_t   ui32ChannelAssignment;
  

   /*This field specifies the sampling frequency of the LPCM audio stream.
	0 = Reserved_0
	1 = Sampling_48_kHz
	2 = Reserved_2
	3 = Reserved_3
	4 = Sampling_96_kHz
	5 = Sampling_192_kHz
	6 = Reserved_6
	15 = Reserved_15 */
   	uint32_t    ui32SamplingFreq;

    /*This field specifies the sampling resolution of the audio samples for all channels in the LPCM audio stream.
	0 = Reserved
	1 = bits_16
	2 = bits_20
	3 = bits_24 */
	uint32_t    ui32BitsPerSample;

	/* This field indicates the track-start or index-point. 
	0 = Not_present
	1 = Present*/
	uint32_t    ui32StartFlag;


   /* This field indicates Ouput LFE channel Configuration
  	1 = LFE_ON
    0 = LFE_OFF*/
	uint32_t   ui32OutputLfeMode;
  
	/* This field indicates Ouput channel Configuration
	0 = Two_mono_channels_1_1_ch1_ch2
	1 = One_centre_channel_1_0_C
	2 = Two_channels_2_0_L__R
	3 = Three_channels_3_0_L_C_R
	4 = Three_chanels_2_1_L_R_S
	5 = Four_channels_3_1_L_C_R_S
	6 = Four_channels_2_2_L_R_SL_SR
	7 = Five_channels_3_2_L_C_R_SL_SR */
	uint32_t   ui32OutMode;

	/*
	This is the cumulative number of samples decoded by any decoder from the
	channel open time. Decoder should accumulate number of samples decoded for
	every frame. Dummy samples are not included.
	*/
	uint32_t	ui32NumSamplesDecoded;
	/*
	This is the cumulative count for number of Valid frames decoded
	*/
	uint32_t	ui32TotalFramesDecoded;
	/*
	This is the cumulative count for number of Dummy decodes done by decoder
	*/
	uint32_t	ui32TotalFramesDummy;
	/*
	This is the cumulative count for number of erroneous frames detected by decoder.
	*/

	uint32_t	ui32TotalFramesInError;

	/* This field tells whether the stream info fields are valid or not */
    uint32_t ui32StatusValid;


}BRAP_FWIF_P_LpcmStreamInfo;

/*********************************************************************
Summary:
     This structure defines all the stream information parameters of 
	 Custom voice PP which is required by PI at every frame 
	 boundary.

Description:

See Also:
**********************************************************************/
typedef struct BRAP_FWIF_P_CustomVoiceStatusInfo
{
	int32_t i32PVCMonitorLevel;
 
	/* This field tells whether the stream info fields are valid or not */
	uint32_t ui32StatusValid;

} BRAP_FWIF_P_CustomVoiceStatusInfo;


 



/*********************************************************************
Summary:
     This structure defines all the stream information parameters of 
	 PCM WAV decoder which is required by PI at every frame 
	 boundary.

Description:

See Also:
**********************************************************************/

typedef struct BRAP_FWIF_P_PcmWavStreamInfo
{
	 uint32_t	ui32SamplingFreq;

	 uint32_t	ui32NumChannels;

	 /*
	This is the cumulative number of samples decoded by any decoder from the
	channel open time. Decoder should accumulate number of samples decoded for
	every frame. Dummy samples are not included.
	*/
	uint32_t	ui32NumSamplesDecoded;
	/*
		This is the cumulative count for number of Valid frames decoded
	*/
	uint32_t	ui32TotalFramesDecoded;
	/*
		This is the cumulative count for number of Dummy decodes done by decoder
	*/
	uint32_t	ui32TotalFramesDummy;
	/*
		This is the cumulative count for number of erroneous frames detected by decoder.
	*/
	uint32_t	ui32TotalFramesInError;
	/* This field tells whether the stream info fields are valid or not */
    uint32_t	ui32StatusValid;

}BRAP_FWIF_P_PcmWavStreamInfo;


/*********************************************************************
Summary:
     This structure defines all the stream information parameters of 
	 AMR decoder which is required by PI at every frame 
	 boundary.

Description:

See Also:
**********************************************************************/

typedef struct BRAP_FWIF_P_AmrStreamInfo
{
/*
	The bitrate of the stream.
	0 = 4.75kbps
	1 = 5.15kbps
	2 = 5.90kbps
	3 = 6.70kbps
	4 = 7.40kbps
	5 = 7.95kbps
	6 = 10.2kbps
	7 = 12.2kbps
	8 = Silence Frame
*/
	uint32_t    ui32BitRate;

/*
	The sampling frequency of the stream, 8kHz always.
*/
	uint32_t    ui32SamplingFreq;

/*
	Audio coding mode which describes the number of input channels.
	1 = One_centre_channel_1_0_C, always.
*/
	uint32_t    ui32AcmodValue;

/*
	Ouput channel configuration.
	0 = Two_mono_channels_1_1_ch1_ch2
	1 = One_centre_channel_1_0_C
	2 = Two_channels_2_0_L__R
	3 = Three_channels_3_0_L_C_R
	4 = Three_chanels_2_1_L_R_S
	5 = Four_channels_3_1_L_C_R_S
	6 = Four_channels_2_2_L_R_SL_SR
	7 = Five_channels_3_2_L_C_R_SL_SR
*/
	uint32_t	ui32OutputMode;

/*
	This is the cumulative number of samples decoded by any decoder from the
	channel open time. Decoder should accumulate number of samples decoded for
	every frame. Dummy samples are not included.
*/
	uint32_t	ui32NumSamplesDecoded;

/*
	This is the cumulative count for number of Valid frames decoded.
*/
	uint32_t	ui32TotalFramesDecoded;

/*
	This is the cumulative count for number of Dummy decodes done by decoder.
*/
	uint32_t	ui32TotalFramesDummy;

/*
	This is the cumulative count for number of Erroneous frames detected by decoder.
*/
	uint32_t	ui32TotalFramesInError;

/*
	This field tells whether the stream info fields are valid or not.
*/
    uint32_t	ui32StatusValid;

} BRAP_FWIF_P_AmrStreamInfo;


/*********************************************************************
Summary:
     This structure defines all the stream information parameters of 
	 DRA decoder which is required by PI at every frame 
	 boundary.

Description:

See Also:
**********************************************************************/
typedef struct BRAP_FWIF_P_DraStreamInfo
{
	uint32_t ui32SamplingFrequency;
		/*
		The sampling frequency value as defined by DRA Spec. v1.0
		0 = Sampling_8_kHz 
		1 = Sampling_11_025_kHz 
		2 = Sampling_12_kHz 
		3 = Sampling_16_kHz 
		4 = Sampling_22_05_kHz 
		5 = Sampling_24_kHz 
		6 = Sampling_32_kHz
		7 = Sampling_44_1_kHz
		8 = Sampling_48_kHz
		9 = Sampling_88_2_kHz
		10 = Sampling_96_kHz
		11 = Sampling_176_4_kHz
		12 = Sampling_192_kHz
		13 = RESERVED_13
		14 = RESERVED_14
		15 = RESERVED_15
		*/
	
	uint32_t ui32FrameSize;
		/*
		This field contains the total frame size in bytes of the current frame.
		Max frame size is 4096 bytes
		*/

	uint32_t ui32NumBlocks;
		/*
		No. of short window mdct blocks in the frame. Num samples = 128 * ui32NumBlocks = 1024. 
		Hence ui32NumBlocks has a value of 8
		*/

	uint32_t ui32AcmodValue;
		/*
		Input primary channels configuration
		1 = Cf
		2 = Lf, Rf
		3 = Lf, Rf, Cr
		4 = Lf, Rf, Lr, Rr
		5 = Lf, Rf, Lr, Rr, Cf
		6 = Lf, Rf, Lr, Rr, Cr, Cf
		7 = Lf, Rf, Lr, Rr, Ls, Rs, Cf
		8 = Lf, Rf, Lr, Rr, Ls, Rs, Cr, Cf
		*/

	uint32_t ui32LFEOn;
		/* 
		Input LFE present
		0 - LFE Not Present
		1 - LFE Present
		*/

	uint32_t ui32OutputMode;
		/*
		User defined Output configuration
		0 - Mono
		1 - Stereo, LoRo
		2 - Stereo, LtRt
		3 - 5.1
		4 - Invalid
		*/

	uint32_t ui32OutputLFEMode;
		/*
		User prompted LFE present on Output
		0 - LFE not required on OutPut
		1 - LFE required on Output
		*/
	
	uint32_t ui32ErrorStatus;
		/*
		Error status as sent by the decoder
		0 - Frame is sane
		1 - Frame is corrupt. O/P has been zero filled
		*/
	
	uint32_t	 ui32NumSamplesDecoded;
		/*
		This is the cumulative number of samples decoded by any decoder from the
		channel open time. Decoder should accumulate number of samples decoded for
		every frame. Dummy samples are not included.
		*/

	uint32_t	  ui32TotalFramesDecoded;
		/*
			This is the cumulative count for number of valid frames decoded
		*/

	uint32_t  ui32TotalFramesDummy;
		/*
			This is the cumulative count for number of Dummy decodes done by decoder
		*/

	uint32_t  ui32TotalFramesInError;
		/*
			This is the cumulative count for number of erroneous frames detected by decoder.	
		*/

	uint32_t  ui32StatusValid;
		/* This field tells whether the stream info fields are valid or not */

}BRAP_FWIF_P_DraStreamInfo;

/*********************************************************************
Summary:
     This structure defines all the stream information parameters of 
	 Real Audio LBR decoder which is required by PI at every frame 
	 boundary.

Description:

See Also:
**********************************************************************/

typedef struct BRAP_FWIF_P_RalbrStreamInfo
{
	uint32_t	ui32SamplingFrequency;
		/*	The sampling frequnecy should be in absolute value but not index.
			The samplign frequency support for RA is 8000Hz tp 44100Hz		
			The sampling frequency value, 0 in case of error.
			
		*/
	
	uint32_t	ui32FrameSize;
		/*	This field contains the total frame size
			in bytes of the current frame.		
		*/

	uint32_t	ui32StreamAcmod;
		/*	Acmod of input stream, 
			possible value [0: Error, 
							1: C, 
							2: (Lf, Rf)
						   ]	
		*/

	uint32_t	ui32UserOutputMode;	
		/*	User defined Output configuration, same as eBufferOutputMode
			Syntex according to BCOM_eACMOD		

					0, Invalid
					1,  Centre (C)
					2,	L,R -->To be used for BufferOutputMode only.
					3,	L,C,R
					4,	L,R,S
					5,	L,C,R,S
					6,  L,R,LS,RS
					7,	L,C,R,LS,RS

		*/

	uint32_t	ui32ErrorStatus;
		/*	Error status as sent by the decoder, 
			0 - Frame is sane
			1 - Frame is corrupt. O/P has been Interpolated based on prev values
		*/
	
	uint32_t	ui32NumSamplesDecoded;
		/*	This is the cumulative number of samples decoded by any decoder from the channel open time.
			Decoder should accumulate number of samples decoded for every frame. Dummy samples are not included.		
		*/

	uint32_t	ui32TotalFramesDecoded;
		/*	This is the cumulative count for number of valid frames decoded
		*/

	uint32_t	ui32TotalFramesDummy;
		/*	This is the cumulative count for number of Dummy decodes done by decoder
		*/

	uint32_t	ui32TotalFramesInError;
		/*	This is the cumulative count for number of erroneous frames detected by decoder.
		*/

	uint32_t	ui32StatusValid;
		/*	This field tells whether the stream info fields are valid or not range according to BCOM_eSUCCESS_FAIL
			eFAIL => invalid, eSUCCESS => valid
		*/

}BRAP_FWIF_P_RalbrStreamInfo;


/*********************************************************************
Summary:
     This structure defines all the stream information parameters of 
	 Dolby Pulse decoder which is required by PI at every frame 
	 boundary.

Description:

See Also:
**********************************************************************/
typedef struct BRAP_FWIF_P_DolbyPulseStreamInfo
{
	/*
		The bitrate value as defined by the ISO 11172-3 audio specification.
	*/
	uint32_t    ui32BitRate;

	/*
		AAC Profile: 
		0 = Main_profile
		1 = Low_complexity_profile
		2 = Scaleable_Sampling_Rate_profile
		3 = RESERVED
	*/
	uint32_t	ui32Profile;

	/*
	The sampling frequency value as defined by the ISO 11172-3 audio specification:
		 0 = Sampling_96_kHz
		 1 = Sampling_88_2_kHz
		 2 = Sampling_64_kHz
		 3 = Sampling_48_kHz
		 4 = Sampling_44_1_kHz
		 5 = Sampling_32_kHz
		 6 = Sampling_24_kHz
		 7 = Sampling_22_05_kHz
		 8 = Sampling_16_kHz
		 9 = Sampling_12_kHz
		10 = Sampling_11_025_kHz
		11 = Sampling_8_kHz
		12 = RESERVED_12
		13 = RESERVED_13
		14 = RESERVED_14
		15 = RESERVED_15
	*/
    uint32_t    ui32SamplingFreq;

	/*
		The number of LFE channel elements.
	*/
	uint32_t	ui32NumLfeChannels;

	/*
		The number of audio syntactic elements for the back channels.
	*/
	uint32_t	ui32NumBackChannels;

	/*
		The number of audio syntactic elements in the side.
	*/
	uint32_t	ui32NumSideChannels;

	/*
	The number of audio syntactic elements in the front channels, front center to back center, symmetrically by left and right, or alternating by left and right in the case of single channel element.
	*/
	uint32_t	ui32NumFrontChannels;

	/*
		Output LFE Channel Configuration:
		1 = LFE_ON
		0 = LFE_OFF
	*/
	uint32_t	ui32OutputLfeMode;

	/*
		Ouput channel Configuration:
		0 = Two_mono_channels_1_1_ch1_ch2
		1 = One_centre_channel_1_0_C
		2 = Two_channels_2_0_L__R
		3 = Three_channels_3_0_L_C_R
		4 = Three_chanels_2_1_L_R_S
		5 = Four_channels_3_1_L_C_R_S
		6 = Four_channels_2_2_L_R_SL_SR
		7 = Five_channels_3_2_L_C_R_SL_SR
	*/
	uint32_t	ui32OutputMode;

	/*
		Low Frequency Effects Channel:
		0 = LFE_channel_off
		1 = LFE_channel_ON
	*/
	uint32_t    ui32LfeOn;

	/*
		Audio coding mode which describes the number of input channels.
		0 = Two_mono_channels_1_1_ch1_ch2
		1 = One_centre_channel_1_0_C
		2 = Two_channels_2_0_L__R
		3 = Three_channels_3_0_L_C_R
		4 = Three_chanels_2_1_L_R_S
		5 = Four_channels_3_1_L_C_R_S
		6 = Four_channels_2_2_L_R_SL_SR
		7 = Five_channels_3_2_L_C_R_SL_SR
	*/
	uint32_t    ui32AcmodValue;

	/*
		A 1 bit field indicating that pseudo surround decoding is possible.
		0 = Disabled
		1 = Enabled
	*/
	uint32_t	ui32PseudoSurroundEnable;

	/*
	A 2 bit field that indicates the coefficient to be used in 5 to 2 matrix mixdown.
	*/
	uint32_t	ui32MatrixMixdownIndex;

	/*
	One bit indicating that a stereo matrix coefficient is present in the stream.
		0 = Mixdown_Absent
		1 = Mixdown_Present
	*/
	uint32_t	ui32MatrixMixdownPresent;

	/*
	This is a one bit flag that indicates whether DRC is present in the stream.
		0 = DRC_Absent
		1 = DRC_Present
	*/
	uint32_t	ui32DrcPresent;

	/* Total number of Pulse IDs found in the bitstream till the current frame */
	uint32_t	ui32DolbyPulseIdCnt;

	/*
		This is the cumulative number of samples decoded by any decoder from the
	channel open time. Decoder should accumulate number of samples decoded for every frame. Dummy samples are not included.
	*/
	uint32_t	ui32NumSamplesDecoded;

	/*
		This is the cumulative count for number of Valid frames decoded.
	*/
	uint32_t	ui32TotalFramesDecoded;

	/*
		This is the cumulative count for number of Dummy decodes done by decoder.
	*/
	uint32_t	ui32TotalFramesDummy;

	/*
	This is the cumulative count for number of erroneous frames detected by decoder.
	*/
	uint32_t	ui32TotalFramesInError;

	/*
	This field tells whether the stream info fields are valid or not.
	*/
    uint32_t	ui32StatusValid;

} BRAP_FWIF_P_DolbyPulseStreamInfo;


/*********************************************************************
Summary:
    This structure defines all the stream information parameters of 
	DDP MS10 decoder which is required by PI at every frame boundary.

Description:

See Also:
**********************************************************************/ 

typedef struct BRAP_FWIF_P_DDPMS10StreamInfo
{

/*
	This field decides the sampling frequency of the AC-3 stream. 
	0 = Sampling_48_kHz
	1 = Sampling_44_1_kHz
	2 = Sampling_32_kHz		
*/
    uint32_t    ui32SamplingFrequency;
/*
	The bit stream identification value. Bit stream identification
	value in this standard is 8. The future modifications of this 
	standard may define other values.
*/
    uint32_t    ui32BitStreamIdentification;
/*
	Transmission mode 
	0 = main_audio_service_complete_main_CM
	1 = main_audio_service_music_and_effects_ME
	2 = associated_service_visually_impaired_VI
	3 = associated_service_hearing_impaired_HI
	4 = associated_service_dialogue_D
	5 = associated_service_commentary_C
	6 = associated_service_emergency_E
	7 = associated_service_stream_identification_along_with_ACMOD_value
*/
    uint32_t    ui32BsmodValue;
/*
	Audio coding mode which describes the number of input channels. 
	0 = Two_mono_channels_1_1_ch1_ch2
	1 = One_centre_channel_1_0_C
	2 = Two_channels_2_0_L__R
	3 = Three_channels_3_0_L_C_R
	4 = Three_chanels_2_1_L_R_S
	5 = Four_channels_3_1_L_C_R_S
	6 = Four_channels_2_2_L_R_SL_SR
	7 = Five_channels_3_2_L_C_R_SL_SR
*/
	uint32_t    ui32AcmodValue;
/*
	Dolby Surround Mode (dsurmod)  
	0 = Not_indicated
	1 = NOT_Dolby_Surround_encoded
	2 = Dolby_Surround_encoded
*/
	uint32_t    ui32DsurmodValue;
/*
	Low Frequency Effects Channel 
	0 = LFE_channel_off
	1 = LFE_channel_ON
*/
	uint32_t    ui32LfeOn;
/*
	This field indicates whether the AC-3 language code exists or not. 
	0 = Langauge_code_not_exists
	1 = Language_code_exists
*/
	uint32_t    ui32LangCodeExists;
	/* This field indicates the language code value. */
	uint32_t    ui32LangCodeValue;
/*
	This field indicates the room type value. 
	0 = Not_indicated
	1 = Large_room
	2 = Small_room
*/
	uint32_t    ui32RoomTypeValue;
/*
	This field indicates the room type2 value. 
	0 = Not_indicated
	1 = Large_room
	2 = Small_room
*/
	uint32_t	ui32RoomType2Value;
/* 
	This field indicates that stream is protected by copyright. 
	0 = stream_not_copyright
	1 = stream_copyright
*/
	uint32_t    ui32CopyrightBit;
/*
	This field indicates the stream is original bit stream. 
	0 = Stream_not_original
	1 = Stream_original
*/
	uint32_t	ui32OriginalBitStream;

/*
	AC-3 centre mix level value 
	0 = three_dB_below
	1 = four_and_half_dB_below
	2 = six_dB_below
	3 = RESERVED
*/
	uint32_t	ui32CmixLevel;
/*
	AC-3 surround mix level value 
	0 = three_dB_below
	1 = six_dB_below
	2 = zero_dB_below
	3 = RESERVED
*/
	uint32_t	ui32SurmixLevel;
/*
	This field indicates whether the AC-3 language code for the second channel exists or not. 
	0 = Langauge_code_not_exists
	1 = Language_code_exists
*/
	uint32_t	ui32Langcode2Exists;
/*
	This field indicates the language code value for the second channel.
*/
	uint32_t	ui32Langcode2Value;
/*
	This field indicates whether the time code exits or not. 
	0 = Not_present
	1 = First_half_present
	2 = Second_half_present
	3 = Both_halves_present
*/
	uint32_t	ui32TimeCodeExists;
/*
This is time code for low resolution in 8 seconds increments up to 24 hours. First 5 bits 
indicates the time in hours. Next 6 bits represents the time in minutes. Final 3 bits 
represents the time in 8 seconds increments.
*/
	uint32_t	ui32TimeCodeFirstHalf;
/*
	Output LFE Channel Configuration
	1 = LFE_ON
	0 = LFE_OFF
*/
	uint32_t	ui32OutputLfeMode;
/*
	Ouput channel Configuration
	0 = Two_mono_channels_1_1_ch1_ch2
	1 = One_centre_channel_1_0_C
	2 = Two_channels_2_0_L__R
	3 = Three_channels_3_0_L_C_R
	4 = Three_chanels_2_1_L_R_S
	5 = Four_channels_3_1_L_C_R_S
	6 = Four_channels_2_2_L_R_SL_SR
	7 = Five_channels_3_2_L_C_R_SL_SR
*/
	uint32_t	ui32OutputMode;
/*
	The frame size code is used in conjunction with the sample rate code to determine 
	the number of 16-bit words before the next syncword.
*/
	uint32_t	ui32FrmSizeCod;
/*
	This is time code for high resolution in 1/64th frame increments up to 8 seconds. 
	The first 3 bits indicate the time in seconds. The next 5 bits represent the time 
	in frames. The final 6 bits represent the time in fractions of 1/64th of a frame.
*/
	uint32_t	ui32TimeCodeSecondHalf;
/*
	This is the cumulative number of samples decoded by any decoder from the
	channel open time. Decoder should accumulate number of samples decoded for
	every frame. Dummy samples are not included.
*/
	uint32_t	ui32NumSamplesDecoded;

/*
	This is the cumulative count for number of Valid frames decoded
*/
	uint32_t	ui32TotalFramesDecoded;
/*
	This is the cumulative count for number of Dummy decodes done by decoder
*/

	uint32_t	ui32TotalFramesDummy;
/*
	This is the cumulative count for number of erroneous frames detected by decoder.
*/

	uint32_t	ui32TotalFramesInError;

/* This field tells whether the stream info fields are valid or not */
	uint32_t	ui32StatusValid;

}BRAP_FWIF_P_DDPMS10StreamInfo;


typedef struct BRAP_FWIF_P_MultiStreamDDPStreamInfo
{

/*
	This field decides the sampling frequency of the AC-3 stream. 
	0 = Sampling_48_kHz
	1 = Sampling_44_1_kHz
	2 = Sampling_32_kHz		
*/
    uint32_t    ui32SamplingFrequency;
/*
	The bit stream identification value. Bit stream identification
	value in this standard is 8. The future modifications of this 
	standard may define other values.
*/
    uint32_t    ui32BitStreamIdentification;
/*
	Transmission mode 
	0 = main_audio_service_complete_main_CM
	1 = main_audio_service_music_and_effects_ME
	2 = associated_service_visually_impaired_VI
	3 = associated_service_hearing_impaired_HI
	4 = associated_service_dialogue_D
	5 = associated_service_commentary_C
	6 = associated_service_emergency_E
	7 = associated_service_stream_identification_along_with_ACMOD_value
*/
    uint32_t    ui32BsmodValue;
/*
	Audio coding mode which describes the number of input channels. 
	0 = Two_mono_channels_1_1_ch1_ch2
	1 = One_centre_channel_1_0_C
	2 = Two_channels_2_0_L__R
	3 = Three_channels_3_0_L_C_R
	4 = Three_chanels_2_1_L_R_S
	5 = Four_channels_3_1_L_C_R_S
	6 = Four_channels_2_2_L_R_SL_SR
	7 = Five_channels_3_2_L_C_R_SL_SR
*/
	uint32_t    ui32AcmodValue;
/*
	Dolby Surround Mode (dsurmod)  
	0 = Not_indicated
	1 = NOT_Dolby_Surround_encoded
	2 = Dolby_Surround_encoded
*/

	uint32_t    ui32DepFrameChanmapMode;
/*  This Mode represent channel ordering index
	0  - Reserved 
	1  - C, 
	2  - L, R, 
	3  - L, C, R, 
	4  - L, R, l, 
	5  - L, C, R, l 
	6  - L, R, l, r 
	7  - L, C, R, l, r 
	8  - L, C, R, Cvh 
	9  - L, R, l, r, Ts 
	10 - L, C, R, l, r, Ts 
	11 - L, C, R, l, r, Cvh 
	12 - L, C, R, Lc, Rc 
	13 - L, R, l, r, Lw, Rw 
	14 - L, R, l, r, Lvh, Rvh 
	15 - L, R, l, r, Lsd, Rsd 
	16 - L, R, l, r, Lrs, Rrs 
	17 - L, C, R, l, r, Lc, Rc 
	18 - L, C, R, l, r, Lw, Rw 
	19 - L, C, R, l, r, Lvh, Rvh 
	20 - L, C, R, l, r, Lsd, Rsd 
	21 - L, C, R, l, r, Lrs, Rrs 
	22 - L, C, R, l, r, Ts, Cvh 
	23 - L, R, l, r, Cs 
	24 - L, C, R, l, r, Cs 
	25 - L, R, l, r, Cs, Ts 
	26 - L, C, R, l, r, Cs, Cvh 
	27 - L, C, R, l, r, Cs, Ts 
*/

	uint32_t    ui32DsurmodValue;
/*
	Low Frequency Effects Channel 
	0 = LFE_channel_off
	1 = LFE_channel_ON
*/
	uint32_t    ui32LfeOn;
/*
	This field indicates whether the AC-3 language code exists or not. 
	0 = Langauge_code_not_exists
	1 = Language_code_exists
*/
	uint32_t    ui32LangCodeExists;
	/* This field indicates the language code value. */
	uint32_t    ui32LangCodeValue;
/*
	This field indicates the room type value. 
	0 = Not_indicated
	1 = Large_room
	2 = Small_room
*/
	uint32_t    ui32RoomTypeValue;
/*
	This field indicates the room type2 value. 
	0 = Not_indicated
	1 = Large_room
	2 = Small_room
*/
	uint32_t	ui32RoomType2Value;
/* 
	This field indicates that stream is protected by copyright. 
	0 = stream_not_copyright
	1 = stream_copyright
*/
	uint32_t    ui32CopyrightBit;
/*
	This field indicates the stream is original bit stream. 
	0 = Stream_not_original
	1 = Stream_original
*/
	uint32_t	ui32OriginalBitStream;

/*
	AC-3 centre mix level value 
	0 = three_dB_below
	1 = four_and_half_dB_below
	2 = six_dB_below
	3 = RESERVED
*/
	uint32_t	ui32CmixLevel;
/*
	AC-3 surround mix level value 
	0 = three_dB_below
	1 = six_dB_below
	2 = zero_dB_below
	3 = RESERVED
*/
	uint32_t	ui32SurmixLevel;
/*
	This field indicates whether the AC-3 language code for the second channel exists or not. 
	0 = Langauge_code_not_exists
	1 = Language_code_exists
*/
	uint32_t	ui32Langcode2Exists;
/*
	This field indicates the language code value for the second channel.
*/
	uint32_t	ui32Langcode2Value;
/*
	This field indicates whether the time code exits or not. 
	0 = Not_present
	1 = First_half_present
	2 = Second_half_present
	3 = Both_halves_present
*/
	uint32_t	ui32TimeCodeExists;
/*
This is time code for low resolution in 8 seconds increments up to 24 hours. First 5 bits 
indicates the time in hours. Next 6 bits represents the time in minutes. Final 3 bits 
represents the time in 8 seconds increments.
*/
	uint32_t	ui32TimeCodeFirstHalf;
/*
	Output LFE Channel Configuration
	1 = LFE_ON
	0 = LFE_OFF
*/
	uint32_t	ui32OutputLfeMode;
/*
	Ouput channel Configuration
	0 = Two_mono_channels_1_1_ch1_ch2
	1 = One_centre_channel_1_0_C
	2 = Two_channels_2_0_L__R
	3 = Three_channels_3_0_L_C_R
	4 = Three_chanels_2_1_L_R_S
	5 = Four_channels_3_1_L_C_R_S
	6 = Four_channels_2_2_L_R_SL_SR
	7 = Five_channels_3_2_L_C_R_SL_SR
*/
	uint32_t	ui32OutputMode;
/*
	The frame size code is used in conjunction with the sample rate code to determine 
	the number of 16-bit words before the next syncword.
*/
	uint32_t	ui32FrmSizeCod;
/*
	This is time code for high resolution in 1/64th frame increments up to 8 seconds. 
	The first 3 bits indicate the time in seconds. The next 5 bits represent the time 
	in frames. The final 6 bits represent the time in fractions of 1/64th of a frame.
*/
	uint32_t	ui32TimeCodeSecondHalf;
/*
	This is the cumulative number of samples decoded by any decoder from the
	channel open time. Decoder should accumulate number of samples decoded for
	every frame. Dummy samples are not included.
*/
	uint32_t	ui32NumSamplesDecoded;

/*
	This is the cumulative count for number of Valid frames decoded
*/
	uint32_t	ui32TotalFramesDecoded;
/*
	This is the cumulative count for number of Dummy decodes done by decoder
*/

	uint32_t	ui32TotalFramesDummy;
/*
	This is the cumulative count for number of erroneous frames detected by decoder.
*/

	uint32_t	ui32TotalFramesInError;

	uint32_t	ui32NumIndepStreamPresent;
	uint32_t	ui32IndepStreamId[8]; 
/* This field tells whether the stream info fields are valid or not */
	uint32_t	ui32StatusValid;

}BRAP_FWIF_P_MultiStreamDDPStreamInfo;





/*********************************************************************
Summary:
     This structure defines all the stream information parameters of 
	 Audessey Volume PP which is required by PI at every frame 
	 boundary.

Description:

See Also:
**********************************************************************/
typedef struct BRAP_FWIF_P_AudysseyVolStatusInfo
{
    int32_t      i32chCalcLevel[8];		/* q22 chCalcLevel in dB in floating format Default 0 */


    int32_t		 i32Vol_Applied_dB;		/*q22 DynamicEQ  Vol_Applied_dB in dB*/

    int32_t      i32SwDynEQ;			/* Dynamic EQ On/Off/Ramp off  3 modes 0, 1 and 2 */
										/* Default : 0, range [0,1,2]*/
										/* 0 -> off, 1-> on, 2 -> Ramp off */

    uint32_t	 ui32NumChannels;		/*	No of channels */
    uint32_t     ui32AudysseyVolBypass;	/*	Default : 0, range [0,1]	*/
										/*	0 -> Processing enable, 1-> total disable of the processing*/

    int32_t      i32ChannelMask;		/* Specifying the channels where gain is to be applied*/
										/* Default:    0xFF	All channels */
										/* Meaning of bits

											0 -> L, 1 -> R, 2 -> C, 3 -> LFE
											4 -> Ls, 5-> Rs, 6 -> Lb, 7 -> Rb
										*/

    int32_t      i32AudysseyVolInit;    /* Default : 0, range [0,1]*/
										/* 0 -> Do nothing, 1-> Reset(re-initialize) ADV*/
            
    int32_t      i32HeadroomOffset;		/* In q22 format. headroom_offset in dB */
										/* Default : 0, range -256 to 256*/

    int32_t      i32SwAudysseyVol;		/* ADV On/Off */
										/* Default : 1, range [0,1]*/
										/* 0 -> off, 1-> on*/

    int32_t      i32SwDynSurrGain;		/* Switch for surround channel */
										/* Default : 1, range [0,1]*/
										/* 0 -> off, 1-> on*/
    int32_t      i32SwHiLoCmpress;		/* Switch for having boost in calc cntr gain    */
										/* Default : 1, range [0,1]*/
										/* 0 -> off, 1-> on */

    int32_t      i32dBVolSetting;		/* In q22 format for gain function and Dynamic EQ */
										/* Default : 0, float range [0, -50] */
    int32_t      i32GCF;				/* In q22 format	Gain Compression Factor */
										/* Default : 0x733333 (1.8 in float), float range [1, 2.4] */

    int32_t      i32chCalbGain[8];		/* q22 chCAlbGain in dB in floating format. Default 0 */

 
	/* This field tells whether the stream info fields are valid or not */
	uint32_t ui32StatusValid;

} BRAP_FWIF_P_AudysseyVolStatusInfo;



/*********************************************************************
Summary:
    This union  holds all the stream info related field values for all 
	audio decoders in a common memory which is required by PI at every 
	frame boundary.

Description:

See Also:
**********************************************************************/ 


typedef union BRAP_FWIF_P_StreamInfo
{
		BRAP_FWIF_P_Ac3StreamInfo		sAc3StreamInfo;
		BRAP_FWIF_P_DdpStreamInfo		sDdpStreamInfo;
		BRAP_FWIF_P_MpegStreamInfo		sMpegStreamInfo;
        BRAP_FWIF_P_AacheStreamInfo		sAacheStreamInfo;		
		BRAP_FWIF_P_WmaStreamInfo		sWmaStreamInfo;
		BRAP_FWIF_P_WmaProStreamInfo	sWmaProStreamInfo;
		BRAP_FWIF_P_DtsCoreStreamInfo	sDtsCoreStreamInfo;
		BRAP_FWIF_P_LpcmStreamInfo		sLpcmStreamInfo;
		BRAP_FWIF_P_DtsHdStreamInfo		sDtsHdStreamInfo;
		BRAP_FWIF_P_PcmWavStreamInfo	sPcmWavStreamInfo;
		BRAP_FWIF_P_AmrStreamInfo		sAmrStreamInfo;
		BRAP_FWIF_P_DraStreamInfo		sDraStreamInfo;
		BRAP_FWIF_P_RalbrStreamInfo		sRalbrStreamInfo;
		BRAP_FWIF_P_DolbyPulseStreamInfo sDolbyPulseStreamInfo;
		BRAP_FWIF_P_DDPMS10StreamInfo	 sDDPMS10StreamInfo;
		BRAP_FWIF_P_MultiStreamDDPStreamInfo	sMSDDPStreamInfo;

		/* Other decoders info */
} BRAP_FWIF_P_StreamInfo;
 
typedef union BRAP_FWIF_P_DecodeStageStatus
{
        BRAP_FWIF_P_TsmInfo     sTsmInfo;
        BRAP_FWIF_P_StreamInfo  sStreamInfo;
}BRAP_FWIF_P_DecodeStageStatus;
 

/*********************************************************************
Summary:
    This union  holds all the stream info related field values for all 
	audio encoders/PPS/Graphics in a common memory which is required by PI at every 
	frame boundary.

Description:

See Also:
**********************************************************************/ 

typedef struct BRAP_FWIF_P_EncodeStageStatus
{
    BRAP_FWIF_P_DTSBroadcastEncoderStreamInfo sDTSBroadcastEncStreamInfo;
    uint32_t    ui32StatusValid;

} BRAP_FWIF_P_EncodeStageStatus;


typedef union BRAP_FWIF_P_ProcessingStageStatus
{
   BRAP_FWIF_P_CustomVoiceStatusInfo	sCustomVoiceStatusInfo;
   BRAP_FWIF_P_AudysseyVolStatusInfo	sAudysseyVolStatusInfo;

} BRAP_FWIF_P_ProcessingStageStatus;


typedef struct BRAP_FWIF_P_GfxStageStatus
{
    uint32_t    ui32LastCompletedGfxCmdId;    

    uint32_t    ui32LastReadGfxCmdId;    
    
    /* This field tells whether the stream info fields are valid or not */
    uint32_t	ui32StatusValid;   
	
} BRAP_FWIF_P_GfxStageStatus;

/* 
	All the data structure which is going to defined in this union mush have 
    last field as ui32StatusValid 
*/
typedef union BRAP_FWIF_P_StageStatus
{
    BRAP_FWIF_P_EncodeStageStatus       sEncodeStage;
    BRAP_FWIF_P_ProcessingStageStatus   sProcessingStage;
    BRAP_FWIF_P_DecodeStageStatus       sDecodeStage;
    BRAP_FWIF_P_GfxStageStatus       sGfxStage;    
}BRAP_FWIF_P_StageStatus;
    
#endif
