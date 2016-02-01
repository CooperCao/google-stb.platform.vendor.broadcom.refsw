/***************************************************************************
*     Copyright (c) 2004-2006, Broadcom Corporation
*     All Rights Reserved
*     Confidential Property of Broadcom Corporation
*
*  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
*  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
*  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
*
* $brcm_Workfile: $
* $brcm_Revision: 
* $brcm_Date: 
*
* Module Description:
*	This file contains structures, enums, macros and function prototypes, 
*	which are exposed to the upper layer by the Raptor Audio Encoder PI. 
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
 
#ifndef _BRAP_ENC_H__
#define _BRAP_ENC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "brap.h"

/*{{{ Defines */

#define MAX_ENC_INPUT_CHANNELS	6 /* Can be increased later for 7.1 encoding */

#define BRAP_ENC_COPY	0
#define BRAP_ENC_ORIG	1

#define BRAP_ENC_COPYRIGHT_ABSENT	0
#define BRAP_ENC_COPYRIGHT_PRESENT	1

#define BRAP_ENC_CRC_OFF	0
#define BRAP_ENC_CRC_ON	1

/*{{{ Typedefs */
/***************************************************************************
Summary:
	This enumeration defines the available audio input channels.

***************************************************************************/
typedef enum BRAP_ENC_Ipstream
{
	BRAP_ENC_Ipstream_eTwoMono_1_1_ch1_ch2 = 0,
	BRAP_ENC_Ipstream_eMono = 1,
	BRAP_ENC_Ipstream_eOneCentre_1_0_C = 1,
	BRAP_ENC_Ipstream_eStereo_2_0_L_R,
	BRAP_ENC_Ipstream_eThree_3_0_L_C_R,
	BRAP_ENC_Ipstream_eThree_2_1_L_R_S,
	BRAP_ENC_Ipstream_eFour_3_1_L_C_R_S,
	BRAP_ENC_Ipstream_eFour_2_2_L_R_SL_SR,
	BRAP_ENC_Ipstream_eFive_3_2_L_C_R_SL_SR
}BRAP_ENC_Ipstream;

/***************************************************************************
Summary:
	This enumeration defines whether the input channels are interleaved or not.

***************************************************************************/
typedef enum BRAP_ENC_Interleaving
{
	BRAP_ENC_Interleaving_eNone = 0, 	/* No Interleaving */
	BRAP_ENC_Interleaving_eLR = 1,		/* Left & Right Channel Interleaved */
	BRAP_ENC_Interleaving_eSLSR = 1,	/* Left Surround & Right Surround Interleaved */
	BRAP_ENC_Interleaving_eCLFE = 1	/* Centre & LFE Interleaved */
}BRAP_ENC_Interleaving;

/***************************************************************************
Summary:
	This enumeration defines the various quality levels for MP3 encoder.

***************************************************************************/
typedef enum BRAP_ENC_MP1L3_Quality 
{
	BRAP_ENC_MP1L3_Quality_e1 = 1,
	BRAP_ENC_MP1L3_Quality_e2,	
	BRAP_ENC_MP1L3_Quality_e3,	
	BRAP_ENC_MP1L3_Quality_e4,	
	BRAP_ENC_MP1L3_Quality_e5,	
	BRAP_ENC_MP1L3_Quality_e6
}BRAP_ENC_MP1L3_Quality;

/***************************************************************************
Summary:
	This enumeration defines the emphasis levels of MP3 encoding.

***************************************************************************/
typedef enum BRAP_ENC_MP1L3_Emphasis
{
	BRAP_ENC_MP1L3_Emphasis_eNone = 0,
	BRAP_ENC_MP1L3_Emphasis_e50_15ms,
	BRAP_ENC_MP1L3_Emphasis_eReserved,	
	BRAP_ENC_MP1L3_Emphasis_eCCITT_J17
}BRAP_ENC_MP1L3_Emphasis;

/***************************************************************************
Summary:
	This enumeration defines the psychoacoustic model to be used in MP3 encoding, might be discontinued.

***************************************************************************/
typedef enum BRAP_ENC_MP1L3_Psycho_Model
{
	BRAP_ENC_MP1L3_Psycho_Model_eReserved0 = 0,
	BRAP_ENC_MP1L3_Psycho_Model_e1,	
	BRAP_ENC_MP1L3_Psycho_Model_e2,	
	BRAP_ENC_MP1L3_Psycho_Model_eReserved3	
}BRAP_ENC_MP1L3_Psycho_Model;

/***************************************************************************
Summary:
	This enumeration defines the supported sampling frequencies for MP3 encoding.

***************************************************************************/
typedef enum BRAP_ENC_MP1L3_Samp_Freq
{
	BRAP_ENC_MP1L3_Samp_Freq_e441 = 0,
	BRAP_ENC_MP1L3_Samp_Freq_e48,		
	BRAP_ENC_MP1L3_Samp_Freq_e32,		
	BRAP_ENC_MP1L3_Samp_Freq_eReserved	
}BRAP_ENC_MP1L3_Samp_Freq;

/***************************************************************************
Summary:
	This enumeration defines the supported bitrates for MP3 encoding.

***************************************************************************/
typedef enum BRAP_ENC_MP1L3_Bitrate
{
	BRAP_ENC_MP1L3_Bitrate_eFree = 0,
	BRAP_ENC_MP1L3_Bitrate_e32,		
	BRAP_ENC_MP1L3_Bitrate_e40,		
	BRAP_ENC_MP1L3_Bitrate_e48,		
	BRAP_ENC_MP1L3_Bitrate_e56,		
	BRAP_ENC_MP1L3_Bitrate_e64,		
	BRAP_ENC_MP1L3_Bitrate_e80,		
	BRAP_ENC_MP1L3_Bitrate_e96,		
	BRAP_ENC_MP1L3_Bitrate_e112,	
	BRAP_ENC_MP1L3_Bitrate_e128,	
	BRAP_ENC_MP1L3_Bitrate_e160,	
	BRAP_ENC_MP1L3_Bitrate_e192,	
	BRAP_ENC_MP1L3_Bitrate_e224,	
	BRAP_ENC_MP1L3_Bitrate_e256,	
	BRAP_ENC_MP1L3_Bitrate_e320,	
	BRAP_ENC_MP1L3_Bitrate_eForbidden = 15
}BRAP_ENC_MP1L3_Bitrate;

/***************************************************************************
Summary:
	This enumeration defines the supported output channel configurations for MP3 encoding.

***************************************************************************/
typedef enum BRAP_ENC_MP1L3_Opmode
{
	BRAP_ENC_MP1L3_Opmode_eStereo = 0,
	BRAP_ENC_MP1L3_Opmode_eJs,	
	BRAP_ENC_MP1L3_Opmode_eDual,	
	BRAP_ENC_MP1L3_Opmode_eSingle,	
	BRAP_ENC_MP1L3_Opmode_eIs,	
	BRAP_ENC_MP1L3_Opmode_eMs		
}BRAP_ENC_MP1L3_Opmode;

/***************************************************************************
Summary:
	This enumeration defines the supported input channel configurations for MP3 encoding.

***************************************************************************/
typedef enum BRAP_ENC_MP1L3_Ipmode
{
	BRAP_ENC_MP1L3_Ipmode_eStereo	 = 0,
	BRAP_ENC_MP1L3_Ipmode_eReserved,	
	BRAP_ENC_MP1L3_Ipmode_eDual,		
	BRAP_ENC_MP1L3_Ipmode_eMono	
}BRAP_ENC_MP1L3_Ipmode;

/***************************************************************************
Summary:
	This enumeration defines the supported bits per sample settings for MP3 encoding.

***************************************************************************/
typedef enum BRAP_ENC_MP1L3_Bpsi
{
	BRAP_ENC_MP1L3_Bpsi_e16 = 0,
	BRAP_ENC_MP1L3_Bpsi_e17,
	BRAP_ENC_MP1L3_Bpsi_e18,
	BRAP_ENC_MP1L3_Bpsi_e19,
	BRAP_ENC_MP1L3_Bpsi_e20,
	BRAP_ENC_MP1L3_Bpsi_e21,
	BRAP_ENC_MP1L3_Bpsi_e22,
	BRAP_ENC_MP1L3_Bpsi_e23,
	BRAP_ENC_MP1L3_Bpsi_e24,
	BRAP_ENC_MP1L3_Bpsi_e25,
	BRAP_ENC_MP1L3_Bpsi_e26,
	BRAP_ENC_MP1L3_Bpsi_e27,
	BRAP_ENC_MP1L3_Bpsi_e28,
	BRAP_ENC_MP1L3_Bpsi_e29,
	BRAP_ENC_MP1L3_Bpsi_e30,
	BRAP_ENC_MP1L3_Bpsi_e31,
	BRAP_ENC_MP1L3_Bpsi_e32
}BRAP_ENC_MP1L3_Bpsi;

/***************************************************************************
Summary:
	This enumeration defines the supported audio encoder types.

***************************************************************************/
typedef enum BRAP_ENC_AudioType
{
	BRAP_ENC_AudioType_eMpeg1Layer3 = 10,				/* MPEG1 Layer 3*/
#if ( BCHP_CHIP == 7440 )
	BRAP_ENC_AudioType_eDTS	= 23	,					/* DTS */
#endif
   	BRAP_ENC_AudioType_eMax,			/* Max value */
   	BRAP_ENC_AudioType_eInvalid	= 0xFF
} BRAP_ENC_AudioType;

/***************************************************************************
Summary:
	Enum describes all the interrupts that can occur in the Audio Encoder. 

Description:
	Each interrupt has a non-trivial mapping to interrupts coming from the Audio
	Encoder. As of now, most of the interrupts are interrupts are generated by the firmware.

See Also:
	BRAP_ENC_InstallInterruptCallback , BRAP_ENC_UnInstallInterruptCallback
****************************************************************************/
typedef enum BRAP_ENC_Interrupt
{
	BRAP_ENC_Interrupt_eInputDataConsumed,	/* PCM Input Data has been consumed, need more data to continue encoding */
	BRAP_ENC_Interrupt_eOutputDataGenerated,	/* Compressed Output Data is generated, pick up to ensure more generation */
	BRAP_ENC_Interrupt_eStopAckReceived,		/* ENC STOP Acknowledgement has been received */
	BRAP_ENC_Interrupt_eMaxInterrupts		/* Not a Real interrupt, just the max no of AUD_ENC interrupts */
} BRAP_ENC_Interrupt;

/***************************************************************************
Summary:
AUD_ENC callback function typedef. This is different from the BINT_CallbackFunc 
definition in that it allows for an extra parameter to be passed to the 
callback which may contain AUD_ENC or application specific data.
****************************************************************************/
typedef void (*BRAP_ENC_CallbackFunc)(void *pParm1, void *pParm2);


/***************************************************************************
Summary:
	Structure to hold the Input Buffer Information for one audio channel

Description:
	This structure will hold the information necessary for the application to 
	access the Input buffers on a per Audio channel basis. 
****************************************************************************/
typedef struct stInputBufferInfoChannel
{
	void *pInputBufferWritePtr1;	/* Input buffer write pointer for a channel Pre Wrap around */
	void *pInputBufferWritePtr2;	/* Input buffer write pointer for a channel post Wrap around 
							     		Some more free space is remaining at the start of the
								     ringbuffer. This pointer is NULL if ther was no wrap around 
									 and uiNumBytesFree2 can be ignored */
	uint32_t uiNumBytesFree1;		/* The number of bytes available free till end of ringbuffer (pre Wrap around) */
	uint32_t uiNumBytesFree2;		/* The number of bytes available free from start of ringbuffer (post Wrap around) */

} stInputBufferInfoChannel;


/***************************************************************************
Summary:
	Structure to hold the Input Buffer Information for all channels

Description:
	This structure will hold the information necessary for the application to 
	access the Input buffers for all Channels. 
****************************************************************************/
/* Instead of having a structure like this, we can also have the application open multiple
instances of stInputBufferStatusChannel for the no. of input channels present */
typedef struct stInputBufferInfo
{
	union inputtype
	{
		stInputBufferInfoChannel sMono ;
		stInputBufferInfoChannel sCentre ;
		stInputBufferInfoChannel sInterleavedStereo ;
		struct sStereo
		{
			stInputBufferInfoChannel sL ;
			stInputBufferInfoChannel sR ;
		}sStereo;
		struct sInterLeavedLRSLSR
		{
			stInputBufferInfoChannel sLR ;
			stInputBufferInfoChannel sSLSR ;
		}sInterLeavedLRSLSR;
		struct sInterLeavedLRC
		{
			stInputBufferInfoChannel sLR ;
			stInputBufferInfoChannel sC ;
		}sInterLeavedLRC;
		struct sInterLeavedLRS
		{
			stInputBufferInfoChannel sLR ;
			stInputBufferInfoChannel sS ;
		}sInterLeavedLRS;
		struct sInterLeavedLRCS
		{
			stInputBufferInfoChannel sLR ;
			stInputBufferInfoChannel sC ;
			stInputBufferInfoChannel sS ;
		}sInterLeavedLRCS;
		struct sInterleavedFiveptone
		{
			stInputBufferInfoChannel sLR ;
			stInputBufferInfoChannel sSLSR ;
			stInputBufferInfoChannel sCLFE;
		}sInterleavedFiveptone;
		struct sLRC
		{
			stInputBufferInfoChannel sL ;
			stInputBufferInfoChannel sR ;
			stInputBufferInfoChannel sC ;
		}sLRC;
		struct sLRS
		{
			stInputBufferInfoChannel sL ;
			stInputBufferInfoChannel sR ;
			stInputBufferInfoChannel sS ;
		}sLRS;
		struct sLRCS
		{
			stInputBufferInfoChannel sL ;
			stInputBufferInfoChannel sR ;
			stInputBufferInfoChannel sC ;
			stInputBufferInfoChannel sS ;
		}sLRCS;
		struct sLRSLSR
		{
			stInputBufferInfoChannel sL ;
			stInputBufferInfoChannel sR ;
			stInputBufferInfoChannel sSL ;
			stInputBufferInfoChannel sSR ;
		}sLRSLSR;
		struct sFiveptone
		{
			stInputBufferInfoChannel sL ;
			stInputBufferInfoChannel sR ;
			stInputBufferInfoChannel sC;
			stInputBufferInfoChannel sSL;
			stInputBufferInfoChannel sSR;
			stInputBufferInfoChannel sLFE;
		}sFiveptone;
	}inputtype;
	uint32_t uiNumInputChannels;	/* No. of input channels actually present */
}stInputBufferInfo;


/***************************************************************************
Summary:
	Structure to hold the Output Buffer Information

Description:
	This structure will hold the information necessary for the application to 
	access the Output buffer and read from them. They have the read and write 
	pointers
****************************************************************************/
/* Diagram showing the concept - TODO */
typedef struct stOutputBufferInfo
{
	void *pOutputBufferReadPtr1;			/* The Ouput Buffer Read Pointer till the end of ringbuffer (pre-wraparound) */
	void *pOutputBufferReadPtr2;			/* The Ouput Buffer Read Pointer from start of ringbuffer
										     after a wraparound. This pointer is NULL if there is no wrap-around & 
										     uiNumBytesFull2 can be ignored */
	uint32_t uiNumBytesFull1;		/* The number of bytes having output data till end of ringbuffer */
	uint32_t uiNumBytesFull2;		/* The number of bytes having output data from start of ringbuffer */

} stOutputBufferInfo;

/***************************************************************************
Summary:
	Channel specific settings for a enocde channel

Description:
	Structure to hold the channel specific settings for a encode channel. 
	This structure is used while opening a encode Audio Channel.
	If only sizes are given by the application & the pointers are NULL, 
	then we need to allocate according to the sizes.

See Also:
	BRAP_ENC_OpenChannel
****************************************************************************/
typedef struct BRAP_ENC_ChannelSettings
{
#if BRAP_ENC_TEST_MODE
	bool	bExtDeviceMem;		/* Test Mode - true = Application provides one
							   chunk of device memory and RAP PI
							   internally manages it.
							   false = Application doesn't provide
							   device memory .RAP PI uses memory 
							   manager to get device memory. */
	uint32_t ui32MemBase;  /* Test Mode - Device memory base to be used.
							   This field is valid only when
							   bExtDeviceMem = true */
	uint32_t	ui32MemSize;	   /* Test Mode - Device Memory size in bytes. 
							   This field is valid only when
							   bExtDeviceMem = true */
#endif
	BRAP_ENC_AudioType eaudtype;			/* Encoding audio algorithm */
	BRAP_ENC_Ipstream eipstream; /* type of input stream e.g. mono, stereo etc. */
	BRAP_ENC_Interleaving eInterLeaving[3]; /* Fill up type of interleaving for all channels, 0 for L & R, 1 for LS & RS, 2 for C & LFE */
	bool bLFEpresent;			/* Whether LFE channel is present or not */
	void *pInputBuffer;			/* Optional externally supplied input
								buffer for input data - sending NULL 
								will result in the PI allocating memory 
								internally. Once this is supplied, multiple
								input streams (as decided by eipstream & eInterLeaving) 
								will be taken as consecutive memory areas
								from this memory based on uiInputPCMBufferSize*/
	void *pOutputBuffer;			/* Optional externally supplied output
								buffer for encoded data - sending NULL 
								will result in the PI allocating memory 
								internally */
	uint32_t  uiInputPCMBufferSize ; /* Optional Size for Input PCM 
								Ring Buffers, set zero for default internal allocation */
	uint32_t  uiOutputBufferSize ; /* Optional Size for Output encoded data
								Ring Buffer, set zero for default internal allocation */
} BRAP_ENC_ChannelSettings;


/***************************************************************************
Summary:
	Parameters for starting encode

Description:
	This structure contains various input parameters required for starting 
	the encode operation for an Audio Channel. These parameters are 
	essentially the stream specific parameters &parameters specific to any 
	other operations to be done for this channel operation.
See Also:
	BRAP_ENC_Start, BRAP_ENC_Stop
	
***************************************************************************/
typedef struct BRAP_ENC_AudioParams
{
/* TODO : Classify once other encoders come in & also more changes might come here once the firmware modes are settled */
/* For other encoders, move encoder specific parameters under an union */
	BRAP_ENC_MP1L3_Bitrate bitrate;
	BRAP_ENC_MP1L3_Samp_Freq sampfreq;
	BRAP_ENC_MP1L3_Ipmode ipmode;
	BRAP_ENC_MP1L3_Opmode opmode;
	BRAP_ENC_MP1L3_Quality quality;
	BRAP_ENC_MP1L3_Bpsi bpsi;
	BRAP_ENC_MP1L3_Psycho_Model psychomod;
	BRAP_ENC_MP1L3_Emphasis emphasis;
}BRAP_ENC_AudioParams;

/*}}}*/


/*{{{ Function prototypes */


/***************************************************************************
Summary:
	API to retrieve maximum number of encode channels supported. 

Description:
	This API used to retrieve the maximum number of encode channels 
Returns:
	BERR_SUCCESS 
	
See Also:
	
	
****************************************************************************/
BERR_Code BRAP_ENC_GetTotalChannels( 
	BRAP_Handle 	hRap,			/* [in] The Raptor Audio device handle*/
	unsigned int	*pTotalChannels	/* [out] Total encode Audio channels 
											supported */
	);

/***************************************************************************
Summary:
	API used to open a encode channel.

Description:
	It is used to instantiate a encode channel. It allocates channel handle 
	and resource required for the channel if any.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_ENC_CloseChannel
****************************************************************************/
BERR_Code BRAP_ENC_OpenChannel( 
	BRAP_Handle 			hRap,			      /* [in] The Raptor Audio device handle*/
	BRAP_ENC_ChannelHandle 		*phRapEncCh,		  /* [out] The RAP Encoder Channel handle */
	unsigned int			uiChannelNo,		  /* [in] the desired channel ID */					
	const BRAP_ENC_ChannelSettings	*pChnSettings /*[in] Channel settings*/ 
	);


/***************************************************************************
Summary:
	Gets the default channel settings of a encode Audio Channel.
Description:
	This API returns the default channel settings of a encode Audio channel.

 Returns:
	BERR_SUCCESS 

See Also:
	BRAP_ENC_OpenChannel, BRAP_ENC_CloseChannel
***************************************************************************/
BERR_Code BRAP_ENC_GetChannelDefaultSettings(
	BRAP_ENC_ChannelSettings *pDefSettings			/* [out] Default channel settings */
	);



/***************************************************************************
Summary:
	API used to close a encode channel.

Description:
	It closes the instance of a encode channel operation. It frees of the 
	channel handle and resources associated with it if any.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_ENC_OpenChannel
****************************************************************************/
BERR_Code BRAP_ENC_CloseChannel( 
	BRAP_ENC_ChannelHandle 	hRapEncCh,	/* [in] The RAP Encoder Channel handle */
	unsigned int			uiChannelNo		  /* [in] the desired channel ID */					
	);


/***************************************************************************
Summary:
	Gets the default audio params for a Encode Channel.
Description:
	This API returns the default parameters for a Encode Audio channel. 
	
Returns:
	BERR_SUCCESS 

See Also:
	BRAP_ENC_Start, BRAP_ENC_Stop
***************************************************************************/
BERR_Code BRAP_ENC_GetDefaultAudioParams(
	BRAP_ENC_ChannelHandle  hRapEncCh,			/* [in] The RAP Encoder channel handle */
	BRAP_ENC_AudioParams *pDefParams,	/* [out] Default channel parameters */
	unsigned int			uiChannelNo		  /* [in] the desired channel ID */					
	);

/***************************************************************************
Summary:
	Starts a encode channel.

Description:
	This API is required to start the encode of the selected 
	channel (stream), as specified by input BRAP_ENC_AudioParams structure.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_ENC_Stop

****************************************************************************/
BERR_Code BRAP_ENC_Start ( 
	BRAP_ENC_ChannelHandle 			hRapEncCh,			/* [in] The RAP Channel handle */
	const BRAP_ENC_AudioParams	*pAudioParams,		/* [in] Audio parameters required 
														for starting this channel */
	unsigned int			uiChannelNo		  /* [in] the desired channel ID */					
	);
/***************************************************************************
Summary:
	Stops a encode channel.

Description:
	This API is required to stop the encode of the selected 
	channel (stream).	

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_ENC_Start
	
****************************************************************************/
BERR_Code BRAP_ENC_Stop ( 
	BRAP_ENC_ChannelHandle 	hRapEncCh,		/* [in] The RAP Encode Channel handle */
	bool					bStopNow,			/* [in] This flag can be used to tell the PI 
										whether to stop then & there or to flush all the 
										encoded data & then stop. Use this to take care of 
										getting all encoded data in an eof condition. */
	unsigned int			uiChannelNo		  /* [in] the desired channel ID */					
	);


/***************************************************************************
Summary:
	Returns the Input Buffer information such as the Read and Write pointers

Description:
	This API is required by the application layer to get the information
	regarding the input buffers (Ring buffers)

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_ENC_UpdateInputBufferInfo, BRAP_ENC_GetOutputBufferInfo, BRAP_ENC_UpdateOutputBufferInfo
	
****************************************************************************/
BERR_Code BRAP_ENC_GetInputBufferInfo ( 
	BRAP_ENC_ChannelHandle 	hRapEncCh,		/* [in] The RAP Encode Channel handle */
	stInputBufferInfo *pInpStatusInfo,
	unsigned int			uiChannelNo		  /* [in] the desired channel ID */					
	);

/***************************************************************************
Summary:
	Updates the input buffer information

Description:
	This API is required by the application layer to update the write 
	pointers of the input buffer (Ring Buffers)
	Only uiNumBytesFree1 & uiNumBytesFree2 parameters are used to update 
	according to uiNumInputChannels.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_ENC_GetInputBufferInfo, BRAP_ENC_GetOutputBufferInfo, BRAP_ENC_UpdateOutputBufferInfo
	
****************************************************************************/
BERR_Code BRAP_ENC_UpdateInputBufferInfo ( 
	BRAP_ENC_ChannelHandle hRapEncCh,		/* [in] The RAP Encode Channel handle */
	stInputBufferInfo *pInpStatusInfo,
	unsigned int			uiChannelNo,		  /* [in] the desired channel ID */					
	uint32_t sizeLeft1[MAX_ENC_INPUT_CHANNELS],		/* Input Data filled pre-wraparound for Channels */
	uint32_t sizeLeft2[MAX_ENC_INPUT_CHANNELS]		/* Input Data filled post-wraparound for Channels */
	);


/***************************************************************************
Summary:
	Returns the Output Buffer information such as the Read and Write pointers

Description:
	This API is required by the application layer to get information regarding
	the output buffer (Ring Buffer)

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_ENC_GetInputBufferInfo, BRAP_ENC_UpdateInputBufferInfo, BRAP_ENC_UpdateOutputBufferInfo
	
****************************************************************************/
BERR_Code BRAP_ENC_GetOutputBufferInfo ( 
	BRAP_ENC_ChannelHandle 	hRapEncCh,		/* [in] The RAP Encode Channel handle */
	stOutputBufferInfo *pOutBuffStatusInfo,
	unsigned int			uiChannelNo		  /* [in] the desired channel ID */					
	);

/***************************************************************************
Summary:
	Updates the output buffer information

Description:
	This API is required by the application layer to update the read pointer
	in the output buffer. This will be done when the application reads the
	converted stream 

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_ENC_GetInputBufferInfo, BRAP_ENC_UpdateInputBufferInfo, BRAP_ENC_GetOutputBufferInfo
	
****************************************************************************/
BERR_Code BRAP_ENC_UpdateOutputBufferInfo ( 
	BRAP_ENC_ChannelHandle 	hRapEncCh,		/* [in] The RAP Encode Channel handle */
	stOutputBufferInfo *pOutBuffStatusInfo,
	unsigned int			uiChannelNo,		  /* [in] the desired channel ID */					
	uint32_t size1,		/* Output Data filled pre-wraparound */
	uint32_t size2		/* Output Data filled post-wraparound */
	);

/***************************************************************************
Summary:
	Used to enable and install an application callback for an Interrupt.

Description:
	This API should be used by the application /syslib to enable a AUD_ENC 
	specific interrupt for a given audio encode channel handle enumerated by the 
	BRAP_ENC_Interrupt enum and can also optionally install a callback for 
	the same.

Returns:
	BERR_SUCCESS 

See Also:
	
****************************************************************************/
BERR_Code BRAP_ENC_InstallInterruptCallback (
	BRAP_ENC_ChannelHandle 		hRapCh, /* [in] The Raptor Audio Encoder channel handle */
	unsigned int			uiChannelNo,		  /* [in] the desired channel ID */					
	BRAP_ENC_Interrupt 			eInterrupt, /* [in] The desired interrupt/Interrupt 
									that needs to be activated */
	BRAP_ENC_CallbackFunc		audEncInterruptCallBack, /* [in] The AUD_ENC callback 
									function which should be registered for the 
									AUD_ENC Interrupt eInterrupt */ 
	void					*pParm1, /* [in] The application specified parameter that 
									needs to be passed unchaned to the callback */
	void					*pParm2 /* [in] The application specified parameter that 
									needs to be passed unchaned to the callback */
	);


/***************************************************************************
Summary:
	Used to disable and un-install an application callback for an Interrupt.

Description:
	This API should be used by the application /syslib to disable an AUD_ENC 
	specific interrupt for a given channel handle enumerated by the 
	BRAP_ENC_Interrupt enum and can also optionally uninstall the callback for 
	the same.

Returns:
	BERR_SUCCESS 

See Also:
	
****************************************************************************/
BERR_Code BRAP_ENC_UnInstallInterruptCallback (
	BRAP_ENC_ChannelHandle 		hRapCh, /* [in] The Raptor Audio Encoder channel handle */
	unsigned int			uiChannelNo,		  /* [in] the desired channel ID */					
	BRAP_ENC_Interrupt 			eInterrupt /* [in] The desired interrupt/Interrupt 
									that needs to be disabled*/
	);


/*}}}*/

#ifdef __cplusplus
}
#endif

#endif /* _BRAP_ENC_H__ */
