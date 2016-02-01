/***************************************************************************
*     Copyright (c) 2004-2009, Broadcom Corporation
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

#if ( BCHP_CHIP == 7401 )
#include "bchp_7401.h"		/* Chip information */
#elif ( BCHP_CHIP == 7403 )
#include "bchp_7403.h"		/* Chip information */
#elif ( BCHP_CHIP == 7118)
#include "bchp_7118.h"		/* Chip information */
#elif ( BCHP_CHIP == 7400 )
#include "bchp_7400.h"
#endif

#include "bstd.h"
#include "bmem.h"			/* Chip memory access. */
#include "bkni.h"			/* The Kernel Interface */
#include "bint.h"			/* The Interrupt Interface */
#include "bdbg.h"			/* debug header file */
#include "berr_ids.h"		/* error code header file */

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
	Raptor Audio Device handle.

Description:
	This is an opaque handle that an application creates with BRAP_Open 
	which holds the device context of the Audio device.
	
See Also:
	BRAP_GetDefaultSettings(), BRAP_Open(), BRAP_Close()
***************************************************************************/
typedef struct BRAP_P_Device			*BRAP_Handle;

/***************************************************************************
Summary:
	Raptor channel handle.

Description:
	This is an opaque handle for each Audio channel

See Also:
	BRAP_DEC_OpenChannel(), BRAP_DEC_CloseChannel()
	BRAP_PB_OpenChannel(), BRAP_PB_CloseChannel()
	BRAP_CAP_OpenChannel(), BRAP_CAP_CloseChannel()	
***************************************************************************/
typedef struct BRAP_P_Channel			*BRAP_ChannelHandle;              

#ifndef BCHP_7411_VER /* For chips other than 7411 */
/***************************************************************************
Summary:
	Audio encoder channel handle.

Description:
	This is an opaque handle for each Audio encoder channel

See Also:
	BRAP_ENC_OpenChannel(), BRAP_ENC_CloseChannel().
***************************************************************************/
typedef struct BRAP_ENC_P_Channel			*BRAP_ENC_ChannelHandle;              
#endif

#if (BCHP_CHIP == 7401)
/***************************************************************************
Summary:
    Audio descriptor fade handle
Description:
    This is an opaque handle for an audio descriptor object 
See Also:
***************************************************************************/ 
typedef struct BRAP_AudioDescriptorObject *BRAP_AudioDescriptorFadeHandle;
#endif

typedef struct BRAP_P_GetInterruptCount *BRAP_GetInterruptCount;

/***************************************************************************
Summary:
	Supported Audio Outputs ports

Description:
	This enum defines the different types of supported Audio Outputs.
	
See Also:
	BRAP_AddOutputport(), BRAP_RemoveOutputPort()
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
	BRAP_OutputPort_eMax
}BRAP_OutputPort;


/***************************************************************************
Summary:
	Supported audio output modes.

Description:
	This enumeration defines the user's choice of audio output mode. This 
	enum defines number of front and back channels. eN_M represents
	N number of front channels and M number of back channels.

See Also:
    BRAP_OP_ChannelOrder, BRAP_PcmOutputMode, BRAP_MIXER_InputAudioMode
    BRAP_DEC_ConfigParams, BRAP_DEC_SetConfig()
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
	BRAP_OutputMode_e3_2 = 8
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
    BRAP_OP_ChannelOrder, BRAP_OutputMode, BRAP_MIXER_InputAudioMode
	BRAP_PB_AudioParams, BRAP_PB_Start()

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
	Audio output channel selects.

Description:
	This enumeration defines different output channels corresponding to 
	various speaker systems.

See Also:
    BRAP_OutputChannelPair
    BRAP_RBUF_GetBaseAndEndAddress()
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
	BRAP_OutputChannel_eLowFrequency = 5	/*	Low Frequency */
} BRAP_OutputChannel;

/***************************************************************************
Summary:
	Audio output channel pair indexes.

Description:
	This enumeration defines different output channel pairs corresponding to 
	various speaker systems.

See Also:
    BRAP_OutputChannel
	BRAP_AddOutputport(), BRAP_RemoveOutputPort()    
**************************************************************************/
typedef enum BRAP_OutputChannelPair
{
	BRAP_OutputChannelPair_eLR = 0,				/*	Left & Right Streo or Mono */
	BRAP_OutputChannelPair_eLRSurround = 1,		/*	Left & Right Surround */
	BRAP_OutputChannelPair_eCentreLF = 2,		/*	Centre & Low Frequency */
	BRAP_OutputChannelPair_eDownMixedLR = 3
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
	BRAP_PB_ChannelSettings, BRAP_PB_AudioParams, BRAP_PB_Open(), BRAP_PB_Start()
	BRAP_CAP_ChannelSettings, BRAP_CAP_AudioParams, BRAP_CAP_Open(), BRAP_CAP_Start()
****************************************************************************/
typedef enum BRAP_BufDataMode
{
        BRAP_BufDataMode_eMono = 0,       		/* Mono (requires 1 buffer) */
        BRAP_BufDataMode_eStereoInterleaved,    /* Stereo Interleaved (requires 1 buffer) */
        BRAP_BufDataMode_eStereoNoninterleaved, /* Stereo Non-Interleaved (requires 2 buffers) */
        BRAP_BufDataMode_eMaxNum				/* Max number of buffer data mode */		
} BRAP_BufDataMode;


#ifndef BCHP_7411_VER /* For chips other than 7411 */
/* Encoder support in RM */
/***************************************************************************
Summary:
	Format of data mode for encoder input data.
Description:
	Enum defining various data mode - mono, stereo interleaved or 
	stereo non-interleaved & further. 
See Also:

****************************************************************************/
typedef enum BRAP_EncBufDataMode
{
        BRAP_EncBufDataMode_eMono = 1,       		/* Mono (requires 1 buffer) */
        BRAP_EncBufDataMode_eStereoInterleaved = 1,    /* Stereo Interleaved (requires 1 buffer) */
        BRAP_EncBufDataMode_eStereoNoninterleaved = 2, /* Stereo Non-Interleaved (requires 2 buffers) or L/R & SL/SR */
        BRAP_EncBufDataMode_eFiveptoneInterLeaved = 3, /* 3 Ring buffers, L/R, SL/SR, C/LFE */
        BRAP_EncBufDataMode_eStereoSurround = 4, /* 4 Ring buffers, L/R, SL/SR */
        BRAP_EncBufDataMode_eFiveptoneNoLFENonInterLeaved = 5, /* 5 Ring buffers, L, R, SL, SR, C */
        BRAP_EncBufDataMode_eFiveptoneLFENonInterLeaved = 6, /* 6 Ring buffers, L, R, SL, SR, C, LFE */
        /* TODO - Add for other encoder input data combinations */
        BRAP_EncBufDataMode_eMaxNum				/* Max number of buffer data mode */		
} BRAP_EncBufDataMode;
#endif


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
	BRAP_Pb_Start(), BRAP_CAP_Start()
	
**************************************************************************/
typedef enum BRAP_InputBitsPerSample
{
	BRAP_InputBitsPerSample_e8 = 8,		/* 8 bits per input sample:4 samples per 32-bit. 
										   This is only supported for PCM Playback */    
	BRAP_InputBitsPerSample_e16 = 16,	/* 16 bits per input sample: 2 samples per 32-bit */
	BRAP_InputBitsPerSample_e18 = 18,	/* 18 bits per input sample :1 samples per 32-bit, padded LS bits with zero*/ 
	BRAP_InputBitsPerSample_e20 = 20,	/* 20 bits per input sample :1 samples per 32-bit, padded LS bits with zero*/ 	
	BRAP_InputBitsPerSample_e24 = 24,	/* 24 bits per input sample :1 samples per 32-bit, padded LS bits with zero */ 		
	BRAP_InputBitsPerSample_e32 =32 	/* 32 bits per input sample. 
			                           For PCM playback, if input PCM data 
			                           is 24 bits per sample then 
			                           application has to use BRAP_InputBitsPerSample_e32 and 
			                           pad the LSB 8 bits with zeros. */
} BRAP_InputBitsPerSample;


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
	BRAP_Input_Btsc
}BRAP_Input;


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
        BRAP_CapInputPort_eInput0 = 0,    /* Capture Hardware Input Port 0 */
#if ( BCHP_CHIP == 7400 )
		BRAP_CapInputPort_eInput1,    	  /* Capture Hardware Input Port 1 */
#endif
        BRAP_CapInputPort_eMaxInputPorts
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
#if ( BCHP_CHIP == 7400 )
      BRAP_CapInputType_eIntI2s1,     /* I2S1 output is looped back to the 
										 Capture input. */
      BRAP_CapInputType_eIntI2s2,     /* I2S2 output is looped back to the 
										 Capture input.  */
#endif
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



#ifdef __cplusplus
}
#endif

#endif /* _BRAP_TYPES_H__ */

