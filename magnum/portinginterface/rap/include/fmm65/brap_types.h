/***************************************************************************
*     Copyright (c) 2004-2010, Broadcom Corporation
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
* Module Description: Public data types common to all the modules of
*					  Raptor PI.
*
* Revision History:
* $brcm_Log: $
* 
***************************************************************************/
#ifndef _BRAP_TYPES_H__
#define _BRAP_TYPES_H__


#include "bstd.h"
#include "bmem.h"			/* Chip memory access. */
#include "bkni.h"			/* The Kernel Interface */
#include "bint.h"			/* The Interrupt Interface */
#include "bdbg.h"			/* debug header file */
#include "berr_ids.h"		/* error code header file */

#ifdef __cplusplus
extern "C" {
#endif

#if ((BCHP_CHIP == 7420)||(BCHP_CHIP == 7340)||(BCHP_CHIP == 7342) || (BCHP_CHIP == 7125) || (BCHP_CHIP == 7468))
#define BRAP_7420_FAMILY    1
#endif

#if (BCHP_CHIP == 7550)
#define BRAP_7550_FAMILY    1
#endif

#if((BCHP_CHIP == 7405)||(BCHP_CHIP == 7325) || (BCHP_CHIP == 7335) || (BCHP_CHIP == 7336) || (BRAP_7420_FAMILY == 1) || (BRAP_7550_FAMILY == 1))
#define BRAP_7405_FAMILY    1
#endif

#if((BCHP_CHIP == 3548)||(BCHP_CHIP == 3556))
#define BRAP_3548_FAMILY    1
#endif

/* Enable/Disable UOD2 Timing Marker Support */
#if (BRAP_TIMING_MARKER == 1)
#define BRAP_NEW_TIMING_MARKER 1
#else
#define BRAP_NEW_TIMING_MARKER 0
#endif



/***************************************************************************
Summary:
	Main Audio Device handle.

Description:
	This is an opaque handle that an application creates with BRAP_Open 
	which holds the device context of the Audio device.
See Also:
	BRAP_Open, BRAP_Close.
***************************************************************************/
typedef struct BRAP_P_Device			*BRAP_Handle;

/***************************************************************************
Summary:
	Audio channel handle.

Description:
	This is an opaque handle for each Audio channel

See Also:
	BRAP_DEC_OpenChannel, BRAP_DEC_CloseChannel.
***************************************************************************/
typedef struct BRAP_P_Channel			*BRAP_ChannelHandle;              
    
/***************************************************************************
Summary:
	Group of Audio channel handles associted together to produce an audio.
Description:
	This is an opaque handle for a group of audio channel associted together.
See Also:
    BRAP_AssociateChannnels, BRAP_DissociateChannnels.
***************************************************************************/
typedef struct BRAP_P_AssociatedChanGrp	*BRAP_AssociatedChannelHandle;              


/***************************************************************************
Summary:
    Destination handle
Description:
	This is an opaque handle for a destination
See Also:
    BRAP_AddDestination
***************************************************************************/
typedef struct BRAP_P_DstDetails	    *BRAP_DestinationHandle;

/***************************************************************************
Summary:
    Audio processing stage handle
Description:
    This is an opaque handle for an audio processing stage. 
See Also:
***************************************************************************/ 
typedef struct BRAP_P_AudioProcessingStage  *BRAP_ProcessingStageHandle;

/***************************************************************************
Summary:
    Audio descriptor fade handle
Description:
    This is an opaque handle for an audio descriptor object 
See Also:
***************************************************************************/ 
typedef struct BRAP_P_AudioDescriptor *BRAP_AudioDescriptorFadeHandle;

/***************************************************************************
Summary:
	Audio equalizer handle.

Description:
	This is an opaque handle for each Equalizer

See Also:
***************************************************************************/
typedef struct BRAP_P_Equalizer			*BRAP_EqualizerHandle;   

/***************************************************************************
Summary:
	Supported Audio Outputs.

Description:
	This enum defines the different types of supported Audio Outputs.
	
See Also:
	
****************************************************************************/
typedef enum BRAP_OutputPort
{
	BRAP_OutputPort_eSpdif = 0,
	BRAP_OutputPort_eDac0,
	BRAP_OutputPort_eI2s0,
	BRAP_OutputPort_eI2s1,
	BRAP_OutputPort_eI2s2,
	BRAP_OutputPort_eDac1,
	BRAP_OutputPort_eMai,
	BRAP_OutputPort_eFlex,
	BRAP_OutputPort_eRfMod,
    BRAP_OutputPort_eI2s3,
    BRAP_OutputPort_eI2s4,
    BRAP_OutputPort_eSpdif1,
    BRAP_OutputPort_eI2s5,
    BRAP_OutputPort_eI2s6,
    BRAP_OutputPort_eI2s7,
    BRAP_OutputPort_eI2s8,
	BRAP_OutputPort_eDac2,
    BRAP_OutputPort_eMaiMulti0,
    BRAP_OutputPort_eMaiMulti1,
    BRAP_OutputPort_eMaiMulti2,
    BRAP_OutputPort_eMaiMulti3,    
	BRAP_OutputPort_eMax
}BRAP_OutputPort;

/***************************************************************************
Summary:
	Supported Audio Capture Inputs.

Description:
	This enum defines the different types of supported Audio Capture Inputs.
	
See Also:
	
****************************************************************************/
typedef enum BRAP_Input
{
	BRAP_Input_eI2s,
	BRAP_Input_RfAudio
}BRAP_Input;

/***************************************************************************
Summary:
	Supported audio output modes.

Description:
	This enumeration defines the user's choice of audio output mode. This 
	enum defines number of front and back channels. eN_M represents
	N number of front channels and M number of back channels.

See Also:

***************************************************************************/
typedef enum BRAP_OutputMode
{
	BRAP_OutputMode_eRightMono = 0, /* Right Mono - Deprecated, Don't use */
	BRAP_OutputMode_eLeftMono = 0,	   	/* Left Mono - Deprecated, Don't use  */
	BRAP_OutputMode_eTrueMono = 2,		/* Left and Right Mono- Deprecated, Don't use  */
	BRAP_OutputMode_eStereo = 3,		/* Stereo - Deprecated, Don't use */
	BRAP_OutputMode_e1_0 = 0,
	BRAP_OutputMode_e1_1 = 1,
	BRAP_OutputMode_e2_0 = 3,
	BRAP_OutputMode_e3_0 = 4,
	BRAP_OutputMode_e2_1 = 5,
	BRAP_OutputMode_e3_1 = 6,
	BRAP_OutputMode_e2_2 = 7,
	BRAP_OutputMode_e3_2 = 8,
	BRAP_OutputMode_e3_3 = 9,
	BRAP_OutputMode_e3_4 = 10,
    BRAP_OutputMode_eLast
}BRAP_OutputMode; 


/***************************************************************************
Summary:
	Format of PCM output mode.
Description:
	Enum defining various PCM output modes - left mono, right mono and 
	stereo. In case of left / right mono, only left / right samples will be
	sent to both the L & R outputs. In case of stereo, both L & R samples 
	are respectively sent to L & R outputs.

	Supported configurations with various BRAP_PcmDataMode:
	------------- --------------------------------------------------	
	|			 |		  	*BRAP_BufDataMode*					   | 
	|O/p Mode 	 | eMono - StereoInterleaved - StereoNonInterleaved|		
	-------------|------   -----------------   --------------------|
	|Left Mono	 | NO	 | YES				 | YES				   |
	|Right Mono	 | NO 	 | YES				 | YES	 			   |
	|Stereo		 | YES	 | YES				 | YES				   |	
	----------------------------------------------------------------
See Also:
	BRAP_PB_ChannelSettings

****************************************************************************/
typedef enum BRAP_PcmOutputMode
{
    BRAP_PcmOutputMode_eLeftMono,   /* Left samples will be playback on both 
    								   the output channels (L and R) */
    BRAP_PcmOutputMode_eRightMono, 	/* Right samples will be playback on 
    								   both the output channels (L and R) */
    BRAP_PcmOutputMode_eStereo,    	/* Left and Right samples will be 
    								   playback on Left and Right output 
    								   channels respectively */
    BRAP_PcmOutputMode_eMaxNum		/* Max number of PcmOutput modes */
}BRAP_PcmOutputMode;


/***************************************************************************
Summary:
	Input bits per sample.

Description:
	This enumeration defines the input bits per sample for the PCM data. 
	This also indicates the way ring buffers are written by the application.
	
	Note: 
	
	For PCM Playback, 18, 20, 24 values are not allowed for the input 
	bits per sample. For such samples, caller has to pad the least 
	significant bits of the samples with zeros and pass input bits per 
	samples as 32 bits.

	8 bits/sample is not supported for capture channels.
	
See Also:
	BRAP_PB_AudioParams, BRAP_CAP_AudioParams
	
**************************************************************************/
typedef enum BRAP_InputBitsPerSample
{
	BRAP_InputBitsPerSample_e8 = 8,		/* 8 bits per input sample:4 samples per
	                                       32-bit. This is only supported for 
	                                       PCM Playback */    
	BRAP_InputBitsPerSample_e16 = 16,	/* 16 bits per input sample: 2 samples 
	                                       per 32-bit */
	BRAP_InputBitsPerSample_e18 = 18,	/* 18 bits per input sample :1 samples 
	                                       per 32-bit, padded LS bits with 0's*/ 
	BRAP_InputBitsPerSample_e20 = 20,	/* 20 bits per input sample :1 samples 
	                                       per 32-bit, padded LS bits with 0's*/ 	
	BRAP_InputBitsPerSample_e24 = 24,	/* 24 bits per input sample :1 samples 
	                                       per 32-bit, padded LS bits with 0's*/ 		
	BRAP_InputBitsPerSample_e32 =32 	/* 32 bits per input sample. For PCM 
	                                       playback, if input PCM data is 24 
	                                       bits per sample then application has 
	                                       to use BRAP_InputBitsPerSample_e32 
	                                       and pad the LSB 8 bits with zeros. */
} BRAP_InputBitsPerSample;


/***************************************************************************
Summary:
	Capture Input Ports.

Description:
	It enumerates various PCM Capture Input Ports present in Raptor.	
	
See Also:
	BRAP_CapInputType
	
**************************************************************************/
typedef enum BRAP_CapInputPort
{
    BRAP_CapInputPort_eIntCapPort0 = 0, /* Internal Capture port 0 */
    BRAP_CapInputPort_eIntCapPort1,     /* Internal Capture port 1 */
    BRAP_CapInputPort_eIntCapPort2,     /* Internal Capture port 2 */
    BRAP_CapInputPort_eIntCapPort3,     /* Internal Capture port 3 */
    BRAP_CapInputPort_eExtI2s0,         /* External I2S Capture port */
    BRAP_CapInputPort_eRfAudio,         /* RF Audio Capture port */
    BRAP_CapInputPort_eSpdif,           /* SPDIF Capture port */
    BRAP_CapInputPort_eHdmi,            /* HDMI */
    BRAP_CapInputPort_eAdc,    
    BRAP_CapInputPort_eIntCapPort4,     /* Internal Capture port 4 */
    BRAP_CapInputPort_eIntCapPort5,     /* Internal Capture port 5 */
    BRAP_CapInputPort_eIntCapPort6,     /* Internal Capture port 6 */
    BRAP_CapInputPort_eIntCapPort7,     /* Internal Capture port 7 */
    BRAP_CapInputPort_eMax              /* Invalid/last Entry */
} BRAP_CapInputPort;

/***************************************************************************
Summary:
	Capture Input Types.

Description:
	It enumerates various types of the PCM Capture Inputs. Each of the PCM 
	Capture input ports indicated by enum BRAP_CapInputPort can carry data 
	from different sources such as External I2S or the I2S and Flex output 
	ports present in the Device can be fed back to the Capture Input. This 
	enum defines such PCM data sources.

	There can be restriction on the Input Type for an Capture Input Port, 
	which can vary from chip to chip. For example, for 7411, Capture Inport 
	Port 1 can't carry External data. So it is used only if any of the I2S 
	or Flex output is looped back to it.
	
See Also:
	BRAP_CapInputPort
	
**************************************************************************/
typedef enum BRAP_CapInputType
{
      BRAP_CapInputType_eExtI2s = 0,  /* The input port carries External 
										 I2S data */
      BRAP_CapInputType_eIntI2s0,     /* I2S0 output is looped back to the 
										 Capture input */
      BRAP_CapInputType_eIntI2s1,     /* I2S1 output is looped back to the 
										 Capture input. */
      BRAP_CapInputType_eIntI2s2,     /* I2S2 output is looped back to the 
										 Capture input.  */
      BRAP_CapInputType_eIntFlex      /* Flex output is looped back to the 
										 Capture input */
} BRAP_CapInputType;

/***************************************************************************
Summary:
	Capture Modes.

Description:
	It enumerates various modes with which a PCM Capture Channels can be 
	configured. Followings are athe meaning of different capture modes.

	Capture Only:	Here PCM Data coming From Capture Input goes to the 
					associated Ring Buffers (DRAM).
	Full Duplex:	Here PCM Data coming From Capture Input goes to the 
					associated Ring Buffers (DRAM) and gets played back 
					from the ring buffer at the desired output port as well. 
					In this mode it is possible to introduce delay in the 
					playback since the data goes via ring buffers.
	By Pass:		Here PCM Data coming From Capture Input gets played back 
					at the desired output port without 
					geting stored in Ring Buffers (DRAM). In this mode it is 
					not possible to introduce delay in the playback since 
					the data doesn't go via ring buffers.
	
See Also:

	
**************************************************************************/
typedef enum BRAP_CaptureMode
{
        BRAP_CaptureMode_eCaptureOnly = 0,	/* Capture Only Mode. Refer to 
											   the comments in the Enum header 
											   for explanation */
        BRAP_CaptureMode_eFullDuplex,		/* Full duplex Mode. Refer to 
											   the comments in the Enum header 
											   for explanation  */
        BRAP_CaptureMode_eByPass,			/* Bypass Mode. Refer to 
											   the comments in the Enum header 
											   for explanation */
        BRAP_CaptureMode_eMaxCaptureMode
} BRAP_CaptureMode;

/***************************************************************************
Summary:
	Audio output channel selects.

Description:
	This enumeration defines different output channels corresponding to 
	various speaker systems.

See Also:

**************************************************************************/
typedef enum BRAP_OutputChannel
{
	BRAP_OutputChannel_eLeft = 0,			/*	Left Streo or Left Mono */
	BRAP_OutputChannel_eRight = 1,			/*	Right Streo or Right Mono */
	BRAP_OutputChannel_eCentreSurround = 2,	/*	Centre Surround, For 3.1 */
	BRAP_OutputChannel_eLeftSurround = 2,	/*	Left Surround,
												For 4.1 and above */
	BRAP_OutputChannel_eRightSurround = 3,	/*	Right Surround,
												For 4.1 and above */
	BRAP_OutputChannel_eCentre = 4,			/*	Centre,  For 5.1 and above */
	BRAP_OutputChannel_eLowFrequency = 5,	/*	Low Frequency */
	BRAP_OutputChannel_eLeftRear = 6,	    /*	Left Rear */
	BRAP_OutputChannel_eRightRear = 7,	    /*	Right Rear */
	BRAP_OutputChannel_eMax
} BRAP_OutputChannel;

/***************************************************************************
Summary:
	Audio output channel pair indexes.

Description:
	This enumeration defines different output channel pairs corresponding to 
	various speaker systems.

See Also:

**************************************************************************/
typedef enum BRAP_OutputChannelPair
{
	BRAP_OutputChannelPair_eLR = 0,				/* Left & Right Streo or Mono */
    BRAP_OutputChannelPair_eCompressed = 0,     /* For compressed channels */
    BRAP_OutputChannelPair_eCaptureLR = 0,     /* For Captured Input channels L/R or Compressed */
	BRAP_OutputChannelPair_eLRSurround = 1,		/* Left & Right Surround */
	BRAP_OutputChannelPair_eCentreSurround = 1,	/* Left channel used for Centre 
												   Surround for 3.1 */
	BRAP_OutputChannelPair_eCentreLF = 2,		/* Centre & Low Frequency */
	BRAP_OutputChannelPair_eLRRear = 3,         /* Rear Left and Rear Right 
	                                               channels */
	BRAP_OutputChannelPair_eMax
} BRAP_OutputChannelPair;

/***************************************************************************
Summary:
	Format of data mode.
Description:
	Enum defining various data mode - mono, stereo interleaved or 
	stereo non-interleaved. 
		In case of PCM Playback and mixing, the ring buffer required for mono 
	and stereo interleaved is 1, while 2 ring buffers are required for stereo 
	non-interleaved.
See Also:
	BRAP_PB_ChannelSettings, BRAP_CAP_ChannelSettings

****************************************************************************/
typedef enum BRAP_BufDataMode
{
    BRAP_BufDataMode_eMono = 0,       		/* Mono (requires 1 buffer) */
    BRAP_BufDataMode_eStereoInterleaved,    /* Stereo Interleaved (requires 1 
                                               buffer) */
    BRAP_BufDataMode_eStereoNoninterleaved, /* Stereo Non-Interleaved (requires 
                                               2 buffers) */
    BRAP_BufDataMode_eMaxNum				/* Max number of buffer data mode */		
} BRAP_BufDataMode;



/***************************************************************************
Summary:
	Endian-ness of data.
Description:
	Enum to specify whether data should be taken same as system endian or swapped 
    In case of PCM Playback, application can specify this mode.
See Also:
	BRAP_SRCCH_P_Params

****************************************************************************/
typedef enum BRAP_DataEndian
{
    BRAP_DataEndian_eSame = 0,           /* Same as System Endian */
    BRAP_DataEndian_eSwap,               /* Swap, little to big or big to little */
    BRAP_DataEndian_eMaxNum			  /* Invalid */		
} BRAP_DataEndian;

#ifdef __cplusplus
}
#endif

#endif /* _BRAP_TYPES_H__ */

