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
 
#ifndef _BRAP_ENC_PRIV_H__
#define _BRAP_ENC_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "brap.h"
#include "brap_enc.h"

/*{{{ Defines */

#define TOTAL_ENCODER_CHANNELS	1	/* Not used as of now, might be needed later */

/*{{{ Typedefs */
/***************************************************************************
Summary:
	Enum describes all the interrupt frequencies supported for I/O data in the Audio Encoder. 

Description:
	This determines the rate at which the audio encoder firmware generates interrupts for 
	input data consumption & output data generation.

See Also:
	
****************************************************************************/
typedef enum BRAP_ENC_P_InterruptFreq
{
	BRAP_ENC_P_InterruptFreq_eEachFrame,
	BRAP_ENC_P_InterruptFreq_eSecondFrame,
	BRAP_ENC_P_InterruptFreq_eFourthFrame,
	BRAP_ENC_P_InterruptFreq_eReserved
}BRAP_ENC_P_InterruptFreq;

/***************************************************************************
Summary:
	Structure to keep all the AUD ENCODER interrupt callbacks and user params. 

Description:
	These are basically application specified callbacks. Used by app to hook up necessary 
	routines to pass input pcm data, collect encoded data & get information abt. eof.

See Also:
	
****************************************************************************/
typedef struct BRAP_ENC_P_InterruptCallbackInfo
{
	BRAP_ENC_CallbackFunc BRAP_ENC_P_pAppIntCallbackPtr ;
	void              *pParm1 ;
	void              *pParm2 ;
} BRAP_ENC_P_InterruptCallbackInfo ;

/***************************************************************************
Summary:
	Structure defining the state of a Raptor Audio encoder Channel. 

Description:
	This is an internal structure containing specifics about an encoder channel.

See Also:
	
****************************************************************************/
typedef struct BRAP_ENC_P_Channel
{
	
	BCHP_Handle		hChip;			/* Handle to chip object */	
	BREG_Handle		hRegister;		/* Handle to register object */
	BMEM_Handle		hHeap;			/* Handle to memory object */
	BINT_Handle		hInt;			/* Handle to interrupt object */
	BRAP_Handle		hRap;	/* Parent Audio Device handle */

	BRAP_P_ChannelType	eChannelType;/*Channel type: Decode, PCM Playback, 
									   PCM Capture, Encode */

	BRAP_DSP_Handle		hDsp[BRAP_RM_P_MAX_DSPS];		/* DSP Device handles */
	BRAP_RM_Handle		hRm;		/* Resource Manager handle */

	BRAP_ENC_AudioType eaudtype;			/* Encoding audio algorithm */
	BRAP_EncBufDataMode eEncBufDataMode;  /* Buffer Data Mode. This will decide the 
										number of ring buffers required to be 
										allocated. This will be provided during 
										channel start time for AudioEncode. It should be less then or 
										equal to eMaxEncBufDataMode.  */									   	
	BRAP_ENC_Ipstream eipstream; /* type of input stream e.g. mono, stereo etc. */
	BRAP_ENC_Interleaving eInterLeaving[MAX_ENC_INPUT_CHANNELS/2]; 
								/* Fill up type of interleaving for all channels - L/R, SL/SR & C/LFE */
	bool bLFEpresent;			/* Whether LFE channel is present or not */
	uint32_t		uiInputFrameSize;	/* Input frame size for the currently chosen encoder */
	uint32_t		uiOutputFrameSize;	/* Output frame size for the currently chosen encoder, 
									this might not be fixed from frame to frame */
	BRAP_RM_P_EncResrcGrant	sEncRsrcGrnt;	/* Resource granted by the 
									   resource manager */
       BINT_CallbackHandle hDSPCallback;  /* Handle for AUD_DSP_INTH0_R5F_STATUS L2 RAPTOR
    									interrupt callbacks */
       uint32_t        ui32DspIntMask; /* Bits in AUD_DSP_INTH0_R5F that are unmasked */
	bool			bInternalCallFromRap;	/* Flag to distinguish whether an API
											   is getting called from external
											   application or from within RAP modules */
	unsigned int	uiChannelNo;	/* Channel id of the the type of 
									   channel specified by eChannelType */
	BRAP_RBUF_P_Handle	hRBuf[MAX_ENC_INPUT_CHANNELS];	/* Ring buffer 
									   handles for different input channels. */
	bool bEncoderStopped; /* To check whether firmware has stopped encoding & the flushing out of data is complete */
	/* Structure to keep all the AUD_ENC interrupt callbacks and its params */
	BRAP_ENC_P_InterruptCallbackInfo stInterruptCallbackInfo[BRAP_ENC_Interrupt_eMaxInterrupts] ;
/*	BRAP_ENC_P_InterruptCallbackInfo stInterruptCallbackInfo[3] ;*/
}BRAP_ENC_P_Channel;

/***************************************************************************
Summary:
	Internal function to hook up main audio encoder ISR. 

Description:
	Once this is done, the main audio encoder ISR is up & thus it enables all app specific interrupts.

See Also:
	
****************************************************************************/
BERR_Code BRAP_P_InterruptInstallEnc (
	BRAP_ENC_ChannelHandle hRapCh		/* [in] Raptor Channel handle */
);

/***************************************************************************
Summary:
	Internal function to un-hook main audio encoder ISR. 

Description:
	Uninstalls main audio encoder ISR.

See Also:
	
****************************************************************************/
BERR_Code BRAP_P_InterruptUnInstallEnc (
	BRAP_ENC_ChannelHandle 		hRapCh		/* [in] Raptor Channel handle */
);

/***************************************************************************
Summary:
	Main audio encoder ISR. 

Description:
	This calls the application specific callbacks depending on firmware interrupt generations.

See Also:
	
****************************************************************************/
void BRAP_P_DSP2HostEnc_isr (
        void * pParm1, /* [in] Raptor channel handle */
        int    iParm2  /* [in] Not used */        
);

#endif /* _BRAP_ENC_PRIV_H__ */

