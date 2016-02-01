/***************************************************************************
*     Copyright (c) 2006-2011, Broadcom Corporation
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
*	This file contains structures, enums, macros and function prototypes, 
*	which are exposed to the upper layer by the Raptor Audio PI. In other 
*	words this file defines the interfaces between the upper layer and the 
*	Raptor Audio PI layer. This file belongs to the Audio Manager module 
*	of the Raptor Audio PI.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
 
#ifndef _BRAP_H__
#define _BRAP_H__

/*=************************ Module Overview ********************************
Raptor is a porting interface (PI) module which allows status/control of
all audio components in the system including Audio DSP, Flexibile Mixer and 
Muxer (FMM), Output Ports(SPDIF, I2s, DAC, HDMI, etc.), Capture Ports (both 
internal and external), DRAM Memory (through ring buffers), etc.

Raptor Device:
--------------
A Raptor Device is identified by a device handle - BRAP_Handle. This device 
handle maintains the hardware and software states for Raptor module for a chip.

Raptor Channel:
---------------
A subset of hardware and software resources available in a Raptor device 
constitutes a Raptor Channel. A Raptor channel is a logical entity (containing 
hardware and software resource for processing audio data) that carries audio 
data from a source to its destination. A Raptor Channel is identified by a 
channel handle - BRAP_ChannelHandle.

Raptor Channel Types:
---------------------
These are the various supported channel types with their details:

A) Decode Channel:
    The primary function of a decode channel is to 
    - decode a compressed audio data into the PCM data
    - pass through the compressed data as such to the external decoder
    - Or do both decoding and pass-through

    A decode channel internally utilizes a DSP. This compressed data can come 
    from one of the various sources - e.g. transport interface (as CDB-ITB), 
    external capture port capable of capturing compressed data (as SPDIF 
    capture), etc. The decoded PCM data then can be sent to a number of 
    destinations - e.g. Output ports, DRAM memory (a.k.a. Ring Buffers), etc. 
    Also, note the decoded data can be post-processed or transcoded before 
    sending out to the desired destinations.

    Following are the currently supported features:
    
    o Valid Sources: 
        - CDB/ITB

    o Valid Destinations: 
        - Output Ports
        - DRAM Memory (or Ring Buffers)
        
    o Valid Post-Processing: 
        - Transcoding to some other algorithms before sending out to compressed 
          output ports (SPDIF, HDMI)
        - Speaker Management for multi-channel audio
        - Verance Watermark Detection for multi-channel audio        
        - Delay Balancing: Required for lip-syncing transcoded audio with the 
          PCM audio at different output ports    

B) Encode Channel:
    The primary function of an encode channel is to encode a PCM audio data into
    a compressed audio data format. For encoding, it internally utilizes a 
    DSP. This PCM data can come from one of the various sources - e.g. DRAM 
    transport interface (as CDB-ITB), external capture port capable of capturing 
    PCM data (as I2s capture), etc. The encoded compressed data then can be 
    sent to a number of destinations - e.g. Output ports, DRAM memory (a.k.a. 
    Ring Buffers), etc. 

    Following are the currently supported features:
    
    o Valid Sources: 
        - DRAM Memory (or Ring Buffers)

    o Valid Destinations: 
        - DRAM Memory (or Ring Buffers)
        
    o Valid Post-Processing: 
        - None

C) PCM Playback Channel:
    The primary function of a PCM playback channel is to read PCM data provided
    by the application and route it to the its destination. This channel is 
    mostly used to provide sound effects which can either be mixed with other 
    channels in the system or can standalone played out from the output ports.
    Application can provide the PCM data by writing into the DRAM memory (a.k.a.
    ring buffers). This PCM data can be sent to a number of destinations - e.g. 
    Output ports, DRAM memory (a.k.a. Ring Buffers), etc. 

    Note: 
    - Once a PCM playback channel audio data is mixed to a decode channel audio 
    data, all the post mixing processing done on the decode channel will also be
    applied to PCM playback channel audio data.

    - It is invalid to mix a PCM audio data to a compressed audio data.
    
    Following are the currently supported features:
    
    o Valid Sources: 
        - DRAM Memory (or Ring Buffers)

    o Valid Destinations: 
        - Output Ports
        - DRAM Memory (or Ring Buffers)
        
    o Valid Post-Processing: 
        - All applicable to a decode channel after a PCM playback channel gets 
          mixed with that decode channel

D) PCM Capture Channel:
    The primary function of a PCM capture channel is to capture a PCM data 
    from a port and route it to the its destination. This channel is mostly used
    to capture microphone input and can be used to achieve karoake effect. This
    can also be used to capture audio data from an external chip. 
    This PCM data can be sent to a number of destinations - e.g. Output ports, 
    DRAM memory (a.k.a. Ring Buffers), etc. 

    Note: 
    - Once a PCM capture channel audio data is mixed to a decode channel audio 
    data, all the post mixing processing done on the decode channel will also be
    applied to PCM capture channel audio data.
    
    Following are the currently supported features:
    
    o Valid Sources: 
        - External and internal capture ports

    o Valid Destinations: 
        - Output Ports
        - DRAM Memory (or Ring Buffers)
        
    o Valid Post-Processing: 
        - All applicable to a decode channel after a PCM capture channel gets 
          mixed with that decode channel

Raptor APIs:
------------
There are two sets of Raptor APIs - 
o APIs that operate on device level, and
o APIs that operate on channel level

All public APIs, data structues, enums and defines has a form of BRAP_XXX, while
all the private ones have a form of BRAP_<SubModule>_P_XXX. All public functions
do not test for NULL or invalid handle passed to these functions, therefore the 
caller is responsible to make sure that the handles are valid.

Example Usage Case:
-------------------
Description of BD_FULL1_1 (as mentioned in FMM_7440_usage.doc):
o The input streams:
    - Primary audio input of 96 KHz 7.1 channels of  LPCM, DTS-HD, or MLP
    - Secondary audio input of 48 KHz 5.1 channels of either DD+ or DTS-HD 
    - IM input of 48 KHz 8 channels mono or mono and stereo combined sound 
      effects

o Post Processing Requirement:
    - Verance Water Mark Detection and Speaker Management (both input of 7.1 
      channels at 96 KHz)
o Outputs      
    - SPDIF output of 48 KHz down-mixed PCM, 
    - HDMI output of 96 KHz 7.1 channels PCM after SM processing,
    - I2S0-3 output of 96 KHz 7.1 channels PCM after SM processing,
    - I2S4 output of 48 KHz stereo
    - DAC output of 96 KHz stereo

Let's see the sequence of API calls to support BD_FULL1_1:

1) First of all set up a valid Raptor Device Handle.
    o Get the default Raptor Device Settings
        - BRAP_GetDefaultSettings(&sDefRapDeviceSettings);
    o Modify the default settings as per your need
    o Open the Raptor Device Handle (hRap)
        - BRAP_Open(&hRap, 
                     hChip,
                     hRegister,
                     hMemory,
                     hInt,
                     hTimer,
                     &sDefRapDeviceSettings);

2) Configure the destination
   In this example, destination is output port.
   So, configure all output ports that may be used. This doesn't open or start
   the port. It simply saves the configuration to be used later when any 
   channel starts to use this port. 

    o Get default configuration for an output port, say I2S0
        BRAP_OutputPortConfig  sOpConfig;        
        BRAP_GetDefaultOutputConfig (hRap, BRAP_OutputPort_eI2s0, sOpConfig);
    o Modify as required. Remember to fill in the output port type
        sSettings.eOutput = BRAP_OutputPort_eI2s0;        
    o Configure the port        
        BRAP_SetOutputConfig (hRap, sOpConfig);   
    o If required, the current output configuration can be retrieved for an 
      output port using following API:
        BRAP_GetCurrentOutputConfig (hRap, BRAP_OutputPort_eI2s0, sOpConfig);   

   This has to be done for all output ports you want to use.

3) Open the required number and types of channels.
   In this example, we need 2 decode channels (1 primary and 1 secondary) and 
   4 playback channels.
    o Get the default channel settings
        - BRAP_GetDefaultChannelSettings(&sDefChannelSettings);
    o For each of the channels, modify the default settings 
        Ex: For primary decode channel, 
            eChType = BRAP_ChannelType_eDecode;
            eChSubType = BRAP_ChannelSubType_ePrimary;
            eAudioSource = BRAP_AudioSource_eXptInterface; 
        (Similarly for other channels)
    o Open each of the channels and get hRapCh
        - BRAP_OpenChannel(hRap, &hRapCh, &sDefChannelSettings);

4) Associate channels to be mixed together before sending to the destination
    (Background: This is required because in 7440, all the channel pairs from 
     secondary decode and PCM playback channels are mixed to each of the channel
     pairs of the primary decode channel. This association is used internally in 
     the PI to allocate proper hardware resources.)
     
    o Prepare the BRAP_AssociatedChanSettings structure
      Ex: sAssociatedChSettings.sChDetail[0].hRapCh = hPriDecCh; for primary DEC
          sAssociatedChSettings.sChDetail[1].hRapCh = hSecDecCh; for secondary DEC
          sAssociatedChSettings.sChDetail[2].hRapCh = hPBCh1;   for PB1
          sAssociatedChSettings.sChDetail[3].hRapCh = hPBCh2;   for PB2
          sAssociatedChSettings.sChDetail[4].hRapCh = hPBCh3;   for PB3
          sAssociatedChSettings.sChDetail[5].hRapCh = hPBCh4;   for PB4
          
    o Associate the primary, secondary and PCM playback channels and get an
       hAssociateChHandle.
       - BRAP_CreateAssociation(&hAssociateChHandle, sAssociatedChSettings);

5) Add destinations as required
   In this example, the destination is output ports. 
    o Configure which output port should carry which audio channel pair.
    Ex: I2s0-3 carrying corresponding channel pairs for 7.1 output
        BRAP_DstDetails         sDstDetails;
        BRAP_OutputPortDetails  sOpDetails;
        for(eChPair: 0-4)
        {
            sOpDetails.eOutput[eChPair] = BRAP_OutputPort_eI2si
        }

        Also, enable/disble bDelayBalance, bPostProcessing and bTranscoding.

        sOpDetails.bDelayBalance = true;
        sOpDetails.bPostProcessing = true;
        sOpDetails.bTranscoding = false;
        
        sDstDetails.eAudioDst = BRAP_AudioDst_eOutputPort;
        sDstDetails.uDstDetails.sOpDetails = sOpDetails;
        - BRAP_AddDestination(hAssociatedCh, &sDstDetails); 
        
     Similarly, add all other output ports.   

   Note: No audio will be heard from these output ports as at this point the 
   channels have not been started.

6) Set channel configuration
   (This configuration parameters are mandatory to be set for a channel before 
    starting a channel. But, once started these configuration parameters can be
    changed for an on-the-fly effect.)
    Ex. For primary decode channel,
    o Get the default configuration parameters for the channel
        BRAP_ChannelConfigParams	sChConfigParams;
        - BRAP_GetDefaultChannelConfig( hRapCh, 
                                        BRAP_DSPCHN_AudioType_eAc3, 
                                        &sChConfigParams);
    o Modify as per your requirement
    o Set the channel configuration for the channel
        - BRAP_SetChannelConfig(hRapCh, &sChConfigParams);

7) Start a Raptor channel
    o Get the default parameters required to start a Raptor channel
        - BRAP_GetDefaulChannelParams(hRapCh, &sDefChParams);  
    o Modify the parameters as required
    o Start the channel
        - BRAP_StartChannel(hRapCh, &sDefChParams);

    Note: If a primary and secondary channels have been associated together, 
    a secondary channel can be started only after starting the primary channel.

8) Stop a Raptor channel
        - BRAP_StopChannel(hRapCh); 

9) Destroy the channel association 
        - BRAP_DestroyAssociation(hAssociateChHandle);

9) Close a Raptor channel
        - BRAP_CloseChannel(hRapCh); 

10) Close the Raptor device
        - BRAP_Close(hRap);

Important Notes:
----------------
1) If an association has a primary decode channel, its OpMode should be informed
   to the PI before starting any channel in the association. This can be done 
   through BRAP_SetChannelConfig().
   
2) If user doesn't have OpMode for a primary decode channel, he can not use any 
   channel in the association. In such scenario, user should dissociate and form
   another association with no primary decode channel.
   
***************************************************************************/
#include "brap_types.h"
#include "brap_dspchn.h"
#include "brap_fmm.h"
#include "brap_rbuf.h"
#include "brap_srcch.h"
#include "brap_mixer.h"
#include "brap_spdiffm.h"
#include "brap_op.h"
#if (BRAP_7550_FAMILY !=1)
#include "brap_capport.h"
#endif
#include "bavc.h"
#include "bimg.h"
#include "btmr.h"

#ifdef RAP_REALVIDEO_SUPPORT
#include "brap_videodecode.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*{{{ Defines */

#define BRAP_VER 3              /* RAP PI Version No. */

#define BRAP_INVALID_VALUE			((unsigned int)(-1))
#define	BRAP_MAX_AUDIO_TYPES		(BRAP_DSPCHN_AudioType_eMax) 
                                        /* Max audio types in  RAP PI. 
                                           Corresponds to number of enum 
                                           values in BRAP_DSPCHN_AudioType*/
#define BRAP_MAX_PP_ALGO_TYPES      (BRAP_ProcessingType_eMax)
						/* Max number of post processing
						algorithms supported in RAP PI */                                           
#define	BRAP_MAX_ENC_AUDIO_TYPES	(BRAP_DSPCHN_ENC_AudioType_eMax) 
                                            /* Max audio encode algorithms */
#define	BRAP_MAX_DSP_DECODE_ALGO	20	/* Max supported DSP Decode Algoritms */
#define	BRAP_MAX_DSP_FS_ALGO		20	/* Max supported DSP Frame Sync 
                                           Algoritms */
#define	BRAP_MAX_DSP_PP_ALGO		12	/* Max supported DSP Post Processing 
                                           Algoritms */
#define BRAP_MAX_OP_IN_A_GRP        4   /* Max output ports in a group - 
                                           corresponding to 7.1 */
/* Max primay decode channels associated in a group */

/*[TODO] Decode channel of "None" type is also treated as primary channel currently, Since the
no. of "None" type channel can be more than 1, So increasing it from 1 to 3. Now if this code will
be used by DVD Chips then a new macro specifically for "None" type needs to be defined,
because in DVD application no of primary dec channel can only be 1. In that case an array of 
"None" type channel handles needs to be added in BRAP_P_AssociatedChanGrp (similar to hPriDecCh[])*/

#define BRAP_MAX_PRI_DEC_CHAN_IN_ASSOCIATED_GRP 3   

/* Max secondary decode channels associated in a group*/
#define BRAP_MAX_SEC_DEC_CHAN_IN_ASSOCIATED_GRP 1   

/* Max PCM playback channels associated in a group*/             
#define BRAP_MAX_PB_CHAN_IN_ASSOCIATED_GRP      4   
/* Max PCM Capture channels associated in a group*/             
#define BRAP_MAX_CAP_CHAN_IN_ASSOCIATED_GRP 1


/* Max associated channels in a group */
#define BRAP_MAX_ASSOCIATED_CHANNELS_IN_GRP     ( \
                                 (BRAP_MAX_PRI_DEC_CHAN_IN_ASSOCIATED_GRP) + \
                                 (BRAP_MAX_SEC_DEC_CHAN_IN_ASSOCIATED_GRP) + \
                                 (BRAP_MAX_PB_CHAN_IN_ASSOCIATED_GRP) + \
                                 (BRAP_MAX_CAP_CHAN_IN_ASSOCIATED_GRP) \
                                 )   
                                 
/* Max number of associated channel groups */
#define BRAP_MAX_ASSOCIATED_GROUPS              4   

/* Max firmware processing branches for a decode channel */
#define BRAP_MAX_FW_PROC_BRANCHES               2


/***************************************************************************
Summary:
    RAP processing stage define: 
****************************************************************************/

/* Max Post-processing branches & stages */
#define BRAP_MAX_PP_BRANCH_SUPPORTED	    3
#if (BRAP_7405_FAMILY ==1)   
#define BRAP_MAX_PP_PER_BRANCH_SUPPORTED    4
#else
#define BRAP_MAX_PP_PER_BRANCH_SUPPORTED    7
#endif
#define BRAP_MAX_INTERNAL_STAGE_SUPPORTED    (1+1+1) /* Decode + SRC + DSOLA*/
#define BRAP_MAX_STAGE_PER_BRANCH_SUPPORTED    (BRAP_MAX_PP_PER_BRANCH_SUPPORTED + BRAP_MAX_INTERNAL_STAGE_SUPPORTED) /*MAX PP stage added externally + MAX internal stage added*/

/* Max number of decode channels with external capport as the source */
#define BRAP_MAX_DEC_CHAN_WITH_CAPPORT_SOURCE   2

#define BRAP_MAX_OUT_CHPAIRS_PER_FW_PROC_BRANCH BRAP_MAX_PP_BRANCH_SUPPORTED*3 /* 3 (for 5.1) for each path */
#define BRAP_MAX_IN_CHPAIRS_PER_FW_PROC_BRANCH  4

#define BRAP_MAX_OUT_CH_PER_FW_PROC_BRANCH (BRAP_MAX_OUT_CHPAIRS_PER_FW_PROC_BRANCH*2)
#define BRAP_MAX_IN_CH_PER_FW_PROC_BRANCH  (BRAP_MAX_IN_CHPAIRS_PER_FW_PROC_BRANCH*2)

#ifdef RAP_INDEP_DELAY_DISABLED
#define BRAP_INDEP_OP_DELAY_SUPPORTED 0
#else
#define BRAP_INDEP_OP_DELAY_SUPPORTED 1
#endif



/****************************************************************************/


/* Max limit of interchannel delay in milli-second */
#define BRAP_MAX_INTERCHANNEL_DELAY         20

/* Error codes */
/***************************************************************************
Summary:
    RAP error defines: Device not initialized.
****************************************************************************/
#define BRAP_ERR_DEVICE_UNINTIALIZED	    BERR_MAKE_CODE(BERR_RAP_ID, 1)

/***************************************************************************
Summary:
    RAP error defines: Fatal error, the device is in a bad state.
****************************************************************************/
#define BRAP_ERR_BAD_DEVICE_STATE	        BERR_MAKE_CODE(BERR_RAP_ID, 2)

/***************************************************************************
Summary:
    RAP error defines: Device resources exhausted 
****************************************************************************/
#define BRAP_ERR_RESOURCE_EXHAUSTED			BERR_MAKE_CODE(BERR_RAP_ID, 3)

/***************************************************************************
Summary:
    RAP error defines: Output port(s) and associated properties are not 
    configured.
****************************************************************************/
#define BRAP_ERR_OUTPUT_NOT_CONFIGURED		BERR_MAKE_CODE(BERR_RAP_ID, 4)

/***************************************************************************
Summary:
    RAP error defines: Channel already opened.
****************************************************************************/
#define BRAP_ERR_CH_ALREADY_OPENED		    BERR_MAKE_CODE(BERR_RAP_ID, 5)

/***************************************************************************
Summary:
    RAP error defines: Channel already started.
****************************************************************************/
#define BRAP_ERR_CH_ALREADY_STARTED		    BERR_MAKE_CODE(BERR_RAP_ID, 6)

/***************************************************************************
Summary:
    RAP error defines: The requested Output port is not supported in this 
    device.
****************************************************************************/
#define BRAP_ERR_OUPUT_PORT_NOT_SUPPORTED	BERR_MAKE_CODE(BERR_RAP_ID, 7)
    
/***************************************************************************
Summary:
    RAP error defines: The link from the Source Channel to mixer is broken
****************************************************************************/
#define BRAP_ERR_INVALID_SRCCH_MIXER_CONNECTION	BERR_MAKE_CODE(BERR_RAP_ID, 8)
    
/***************************************************************************
Summary:
    RAP error defines: Compressed data on SPDIF output port is not a 
    supported option for this algorithm
****************************************************************************/
#define BRAP_ERR_COMP_SPDIF_NOT_ALLOWED		BERR_MAKE_CODE(BERR_RAP_ID, 9)

/***************************************************************************
Summary:
    RAP error defines: Compressed data on SPDIF output port is not a 
    supported option for this algorithm
****************************************************************************/
#define BRAP_ERR_BUFFER_FULL	BERR_MAKE_CODE(BERR_RAP_ID, 10)

/***************************************************************************
Summary:
    RAP error defines: Compressed data on SPDIF output port is not a 
    supported option for this algorithm
****************************************************************************/
#define BRAP_ERR_BUFFER_EMPTY		BERR_MAKE_CODE(BERR_RAP_ID, 11)

/***************************************************************************
Summary:
    RAP error defines: The buffer is not in vailid due to its size of its 
    address
    
****************************************************************************/
#define BRAP_ERR_BUFFER_INVALID     BERR_MAKE_CODE(BERR_RAP_ID, 12)

/***************************************************************************
Summary:
    RAP error defines: Thiss error will come when the task will not be 
    present in the channel handle.
****************************************************************************/
#define BRAP_ERR_INVALID_TASK		BERR_MAKE_CODE(BERR_RAP_ID, 13)

/***************************************************************************
Summary:
    RAP error defines: Thiss error will come when there are some outstanding blit and/or fill 
    operations in the ZSP queue
****************************************************************************/
#define BRAP_GFX_BUSY		BERR_MAKE_CODE(BERR_RAP_ID, 14)


/***************************************************************************
Summary:
    RAP error defines: This error will come when There are some outstanding blit and/or fill 
    operations in the ZSP queue.In this case when these blits/fills are completed Callback installed
    on BRAP_Interrupt_eGfxOperationComplete will be fired.

****************************************************************************/
#define BRAP_GFX_QUEUED		BERR_MAKE_CODE(BERR_RAP_ID, 15)


/***************************************************************************
Summary:
    RAP error defines: This error will come when The GFX QUEUE is full. And it can not take any furthur command.

****************************************************************************/
#define BRAP_GFX_QUEUE_FULL		BERR_MAKE_CODE(BERR_RAP_ID, 16)



/*}}}*/

/*{{{ Typedefs */

/*****************************************************************************
Summary:
	This enum describes various Raptor audio channel types.

Description:    
    These are the various possible audio channel types:
    TODO: Copy details from the excel sheet for each channel 
******************************************************************************/
typedef enum BRAP_ChannelType
{
    BRAP_ChannelType_eDecode,       /* Decode channel */   
    BRAP_ChannelType_ePcmPlayback,  /* PCM playback channel */       
    BRAP_ChannelType_ePcmCapture,   /* PCM capture channel */       
    BRAP_ChannelType_eGfx,   /* Graphics channel */ 
    BRAP_ChannelType_eVideoDecode,	/* Real Video channel */              
#ifdef RAP_SCM_SUPPORT    
    BRAP_ChannelType_eScm,   /* SCM channel */ 
#endif    
    BRAP_ChannelType_eMax           /* Invalid value */
}BRAP_ChannelType;


/*****************************************************************************
Summary:
	This enum describes various sub channel classifications. 

Description:    
    These are the various possible audio channel types: Primary, Secondary and 
    None.

    As per the BD-DVD spec:
    Primary Stream: Doesn't carry meta data information.
    Secondary Stream: Carries the meta data information that can be applied to 
    the primary stream and the secondary stream. 

    Note: Primary and Secondary sub channel concept is applicable only to BD-DVD 
    and HD-DVD decode channel types. For all other products and channel types
    None should be used. 
******************************************************************************/
typedef enum BRAP_ChannelSubType
{
    BRAP_ChannelSubType_ePrimary,   /* Primary sub channel type */
    BRAP_ChannelSubType_eSecondary, /* Secondary sub channel type */
    BRAP_ChannelSubType_eNone,      /* Neither primary nor secondary */
    BRAP_ChannelSubType_eMax        /* Invalid value */
}BRAP_ChannelSubType;


/*****************************************************************************
Summary:
	This enum describes various audio data sources for RAPTOR.

Description:    
    These are the various audio data sources to Raptor:
    1. Transport/CDB-ITB: External modules like RAVE, Transport, etc. write 
       audio data to the CDB-ITB. Currently, only a DSP in Raptor can read from 
       CDB-ITB.
    2. Ring Buffer: An application can provide audio data to Raptor through the 
       ring buffers. Currently, an EncodeOnly or PcmPlayback channel can use 
       this audio data source.
    3. External Capture Ports: Audio data can also be captured from any external
       device through the external capture ports.	
******************************************************************************/
typedef enum BRAP_AudioSource
{
    BRAP_AudioSource_eXptInterface,  /* Transport interface / CDB-ITB */    
    BRAP_AudioSource_eRingBuffer,    /* Ring buffer(DRAM) written by the user */
    BRAP_AudioSource_eExtCapPort,    /* External capture port connected to 
                                        the external device */
    BRAP_AudioSource_eMax            /* Invalid value */
}BRAP_AudioSource;


/*****************************************************************************
Summary:
	This enum describes various destinations for audio data processed in RAPTOR.

Description:    
    These are the various destinations for audio data from Raptor:
    1. Output Ports: Audio data can be sent to the output ports which can be
       connected to speakers or other external devices like SPDIF receivers.	
    2. Ring Buffer: Raptor can be configured to output processed audio data into
       the ring buffers. Then an application can read this data to its memory.   
******************************************************************************/
typedef enum BRAP_AudioDst
{
    BRAP_AudioDst_eOutputPort,      /* Output ports */
    BRAP_AudioDst_eRingBuffer,      /* Ring buffer */
    BRAP_AudioDst_eMax              /* Invalid value */
}BRAP_AudioDst;


/*****************************************************************************
Summary:
	This enum describes Raptor device level interrupts.

Description:    
	These interrupts are for Raptor device. They are not at RAP channel level.
	Handlers for these interrupts should be at Raptor device level and not
	at RAP channel level.
******************************************************************************/
typedef enum BRAP_DeviceLevelInterrupt
{
	BRAP_DeviceLevelInterrupt_eWatchdog,    /* Raptor watchdog interrupt */
	BRAP_DeviceLevelInterrupt_eMax	        /* Invalid value */
} BRAP_DeviceLevelInterrupt;


/*****************************************************************************
Summary:
    Enum describes all the interrupts that are currently handled in 
    Raptor PI. These interrupts are channel specific.
    
Description:    
    In case of similar interrupts that occur for different DSP contexts, or 
    underflows for various RBufs etc, a common enum will be used.
    E.g. BRAP_Interrupt_eSrcChUnderflow will be the common enum used for
    SOURCE_FIFO_X_UNDERFLOW.

Note:    
    Whenever we define any new interrupt we have to define it on the basics of
    DSP interrupt or FMM interrupt. If the interrupt is related to DSP we have 
    to define it in DSP interrupt and if interrupt is related to FMM then we 
    habe to define in FMM interrupts. Check BRAP_Interrupt_eDspLastInterrupt
    and BRAP_Interrupt_eFMMLastInterrupt interrupt values after adding any new 
    interrupts.
******************************************************************************/
typedef enum BRAP_Interrupt
{
    BRAP_Interrupt_eDspFirstInterrupt   = 0,
    BRAP_Interrupt_eFirstPtsReady       = BRAP_Interrupt_eDspFirstInterrupt,
    BRAP_Interrupt_ePtsError,
    BRAP_Interrupt_eStreamInfo,
    BRAP_Interrupt_eTsmLog,
    BRAP_Interrupt_eDecLock,
    BRAP_Interrupt_eDecUnlock,
    BRAP_Interrupt_eSampleRateChange,
    BRAP_Interrupt_eBitRateChange,
    BRAP_Interrupt_eModeChange,
    BRAP_Interrupt_eCrcError,
    BRAP_Interrupt_eStartPtsReached,
    BRAP_Interrupt_eStopPtsReached,	
    BRAP_Interrupt_eMpegLayerChange,
    BRAP_Interrupt_eAstmTsmPass,
    BRAP_Interrupt_eTsmFail,    
    BRAP_Interrupt_eStreamInfoAvailable,        
    BRAP_Interrupt_eCdbItbOverflow,  
    BRAP_Interrupt_eCdbItbUnderflow, 
    BRAP_Interrupt_eUnlicensedAlgo,     
    BRAP_Interrupt_eGfxOperationComplete,
#ifdef RAP_SCM_SUPPORT    
    BRAP_Interrupt_eScmResponse,
    BRAP_Interrupt_eDspLastInterrupt    = BRAP_Interrupt_eScmResponse,
#else
    BRAP_Interrupt_eDspLastInterrupt    = BRAP_Interrupt_eGfxOperationComplete,
#endif
    BRAP_Interrupt_eFMMFirstInterrupt   = BRAP_Interrupt_eDspLastInterrupt+1,
    BRAP_Interrupt_eFmmRbufFreeByte     = BRAP_Interrupt_eFMMFirstInterrupt,
    BRAP_Interrupt_eFmmCrc,
    BRAP_Interrupt_eFMMLastInterrupt   = BRAP_Interrupt_eFmmCrc,

    BRAP_Interrupt_eMaxInterrupts = BRAP_Interrupt_eFMMLastInterrupt+1
} BRAP_Interrupt;


/*****************************************************************************
Summary:
   This enum describes destination level interrupts.

Description:    
   These interrupts are related to destinations. Handlers for these interrupts 
   should be at destination level.
******************************************************************************/
typedef enum BRAP_DestinationInterrupt
{
   BRAP_DestinationInterrupt_eFmmRbufFullMark,    /* Full Mark Interrupt */
   BRAP_DestinationInterrupt_eFmmRbufOverflow,    /* Overflow Interrupt */       
   BRAP_DestinationInterrupt_eSampleRateChange,    /* Overflow Interrupt */          
   BRAP_DestinationInterrupt_eMax                 /* Invalid value */
} BRAP_DestinationInterrupt;


/*****************************************************************************
Summary:
    Enum describes various CDB/ITB configuration modes for Raptor.
    
Description:    
    Based on the buffer configuration mode, the required CDB and ITB size 
	information is reqtured to the application, which in turn used for 
	allocating the CDB and ITB in Rave. 
******************************************************************************/
typedef enum BRAP_BufferCfgMode
{
	BRAP_BufferCfgMode_eStdAudioMode,	/* Standard Audio Mode (for algorithm 
	                                       like AC3, MPEG, AAC etc) */
	BRAP_BufferCfgMode_eAdvAudioMode,	/* Advanced Audio Mode (for algorithm 
	                                       like AAC-HE, DD+ etc)*/
	BRAP_BufferCfgMode_eIpStbMode,		/* IP Settop Box Mode */
	BRAP_BufferCfgMode_eHdDvdMode,		/* HD DVD Mode */
	BRAP_BufferCfgMode_eBlueRayMode,	/* Blue Ray Mode */
	BRAP_BufferCfgMode_eMaxMode         /* Illegal last value */
}BRAP_BufferCfgMode;


/**********************************************************************
Summary:
	Enum describe the polarity to set in CRC register before starting the CRC
***********************************************************************/
typedef enum BRAP_CrcPolarity
{
	BRAP_CrcPolarity_0=0,
	BRAP_CrcPolarity_1
}BRAP_CrcPolarity;

/*****************************************************************************
Summary:
    Raptor Application callback function typedef. This is different from the 
    BINT_CallbackFunc definition in that it allows for an extra parameter to 
    be passed to the callback which contains Raptor specific data. All 
    application ie user level interrupt callback functions have to be of this 
    type.

    void * pParm1: Application specified parameter that needs to be passed 
                   unchanged to the callback
    int    iParm2: Application specified parameter that needs to be passed 
                   unchanged to the callback 
    void * pRap_data: Resevered for internal Raptor PI usage.

******************************************************************************/
typedef void (*BRAP_CallbackFunc) (void *pParm1, int parm2, void *pRap_data);


/***************************************************************************
Summary:
	Version information of PI or firmware.

Description:
	This structure contains the Major and Minor version, Date & Year.

See Also:
	BRAP_RevisionInfo
****************************************************************************/
typedef struct BRAP_VersionInfo
{
	uint32_t		ui32MajorVersion;	/* major version */
	uint32_t		ui32MinorVersion;	/* minor version */
	uint32_t		ui32Date;			/* date and month */
	uint32_t		ui32Year;			/* year */
	char			*pCustomerID;
} BRAP_VersionInfo;

/***************************************************************************
Summary:
	Version information of firmware.

Description:
	This structure contains the Major and Minor version, Date & Year.
	
See Also:
	BRAP_RevisionInfo
****************************************************************************/
typedef struct BRAP_FwVersionInfo
{
	uint32_t		ui32FwVersionField1;
	uint32_t		ui32FwVersionField2;
	uint32_t		ui32CustomerID;
	uint32_t		ui32MajorVersion;	/* major version */
	uint32_t		ui32MinorVersion;	/* minor version */
	uint32_t		ui32Date;			/* date and month */
	uint32_t		ui32Year;			/* year */
} BRAP_FwVersionInfo;

/***************************************************************************
Summary:
	This Structure is used to return the DSP Firmware Algorithm	version 
	information.

Description:
	This structure contains the Major and Minor version, Date and Year of DSP 
	Firmware Algorithm.
	
See Also:
	BRAP_GetRevision  
****************************************************************************/
typedef struct BRAP_DSPVersionInfo
{
	BRAP_FwVersionInfo	sDecAlgoVersion[BRAP_MAX_DSP_DECODE_ALGO];	
                                /* Decode Algorithm version information */
	BRAP_FwVersionInfo	sDecFsVersion[BRAP_MAX_DSP_FS_ALGO];		
                                /* Frame Sync Algorithm version information */
	BRAP_FwVersionInfo	sPPAlgoVersion[BRAP_MAX_DSP_PP_ALGO];		
                                /* Frame Sync Algorithm version information */
} BRAP_DSPVersionInfo;

/***************************************************************************
Summary:
	This Structure is used to return the Raptor Audio PI & firmware version 
	information.

Description:
	This structure contains the Major and Minor version, Date, Year and Chip 
	Id (if applicable) of both PI and firmwares.
	
See Also:
	BRAP_GetRevision  
****************************************************************************/
typedef struct BRAP_RevisionInfo
{
	uint32_t			ui32ChipId;		/* Chip ID */
	BRAP_VersionInfo	sPIVersion;		/* Audio PI version information */
	BRAP_DSPVersionInfo	sDSPVersion;	/* DSP Firmware version information */
} BRAP_RevisionInfo;

/***************************************************************************
Summary:
	Common External Buffer params structure.

Description:
	

See Also:
	BRAP_ChannelParams
****************************************************************************/

typedef struct BRAP_BufferParams
{
    /* Rbuf Settings */
    void *                      pLeftBufferStart; 
                                  /* Pointer to the start of this RBUF.
                                     If it is NULL, internally allocated memory
                                     is used. It has to be 256 byte aligned. */
    void *                      pRightBufferStart; 
                                  /* Pointer to the start of this RBUF.
                                     If it is NULL, internally allocated memory
                                     is used. It has to be 256 byte aligned. 
                                     Based on eBufDataMode, this field would be 
                                     used. If eBufDataMode is Stereo Non 
                                     Interleaved, Two ring buffers are used. */
    size_t                      uiSize;       
                                  /* Ring Buffer size must be multiple of 
                                     256 bytes. If passed as 0, default value 
                                     indicated by BRAP_RBUF_P_DEFAULT_SIZE 
                                     will be used. 
                                     Note: Even if pBufferStart is NULL,
                                     uiSize can be passed for internal use */
    unsigned int                uiWaterMark;  
                                  /* Water Mark: Percentage of Free/Full Bytes.
                                     If passed as 0, default value indicated 
                                     by BRAP_RBUF_P_DEFAULT_WATER_MARK will 
                                     be used. The Water Mark Level depends on 
                                     the type of the System. 
                                     If Edge Triggered system, the application 
                                     provided Water Mark is Ignored and internally
                                     forced to be (RBuf Size-FrameSize), where 
                                     FrameSize is one-fourth of the Rbuf Size.
                                     If Level Triggered System, the user provided 
                                     Water Mark is Used.
                                     Note: Even if pBufferStart is NULL,
                                     uiWaterMark can be passed for internal use */                               
}BRAP_BufferParams;

/***************************************************************************
Summary:
	Ring buffer information required.
Description:
	This structure contains various ring buffer information viz- 
	buffer base pointer, buffer end pointer, continuous free size in bytes
	without considering the wrap-around and total free size in bytes 
	considering the wrap-around.
See Also:
	BRAP_PcmBufInfo
****************************************************************************/
typedef struct BRAP_RingBufInfo {
	void 			*pBasePtr;			/* Base pointer of the associated
										   ring buffer */
	void 			*pEndPtr;			/* End pointer of the associated
										   ring buffer */
	void 			*pWritePtr;			/* Current write pointer in the 
										   ring buffer */
	void 			*pReadPtr;			/* Current read pointer in the 
										   ring buffer */
   	unsigned int	uiContiguousSize;	/* Contiguous free size in 
   					    				   bytes without wrap around */
   	unsigned int 	uiTotalSize;	  	/* Total free size in bytes 
   										   including wrap around */
    unsigned int    uiRBufId;           /* RBuf Id */
}BRAP_RingBufInfo;


/***************************************************************************
Summary:
	PCM Ring buffer information required to write PCM data into the 
	associated ring buffers.
Description:
	This structure contains the PCM Ring buffer information required to write 
	PCM data into the associated ring buffers. At one go, application can at
	max write uiContiguousSize number of bytes in the ring buffer. The onus 
	of maintaining the write current pointer lies with the application.

	Hint: Application can decide the current write pointer by as follows:
			curWrPtr = pEndPtr - uiContiguousSize;
See Also:
	BRAP_PB_GetBufInfo, BRAP_RingBufInfo.
****************************************************************************/
typedef struct BRAP_PcmBufInfo{
 	BRAP_RingBufInfo 	sLtOrSnglBufInfo;/* Ring buffer info associated with
 										   left stereo or mono or stereo 
 										   interleaved ring buffer */
 	BRAP_RingBufInfo	sRtBufInfo;		 /* Ring buffer info associated with
 										   right stereo ring buffer */                                            
}BRAP_PcmBufInfo;


/***************************************************************************
Summary:
	PCM Ring buffer information required to write PCM data into the 
	associated ring buffers.
Description:
	This structure contains the size in bytes of the newly written data in the 
	corresponding ring buffer, separately for the left/single and right ring
	buffers.
See Also:
	BRAP_PB_UpdateBufUsg
****************************************************************************/
typedef struct BRAP_PcmBufSzUpdt{
	unsigned int uiLtOrSnglBufSzUpdt; 	/* Size of the newly written data in
										   left or mono or stereo interleaved
 										   ring buffer in bytes */
	unsigned int uiRtBufSzUpdt;			/* Size of the newly written data in
										   right ring buffer in bytes */                                              
}BRAP_PcmBufSzUpdt;

/*********************************************************************
Summary:
	Structure for informing the maximum number of input and output buffer pairs 
	for each audio processing.

Description:
	This structure indicates the possible maximum number of input and output 
	buffer pairs for each of the audio processing types that can exist through 
	out the life time of the raptor device. This information is internally used 
	by the RAP PI to pre-allocate the corresponding number of ring buffers 
	(hence the DRAM). Once these audio processings are enabled, the 
	corresponding pre-allocated ring-buffers are used by the PI. This prevents 
	the possible error due to memory fragmentation in the DRAM.
	
See Also:
	BRAP_Settings
**********************************************************************/	
typedef struct BRAP_DeviceRBufPool 
{
    unsigned                uiNumPostMixingInputBuffers; /* Number of input buffers required by
        processing done post-mixing.  two buffer is required per PCM channel-pair for each 
        FMM stream, And one buffer is required for Compressed channel input(can be either playback
        or hardware mixed data) */
    unsigned                uiNumPostMixingOutputBuffers; /* Number of output buffers required 
        between post-processing done after mixing and the FMM hardware.  One buffer is required 
        per channel pair of output from any processing done after mixing (includes FwMixer or other 
        post processing such as AVL) */
    unsigned int        iMaxDelay[BRAP_ProcessingType_eMax]; 
                            /* Max possible delay in milli-second. Valid only SM
                               audio processing. This should be less than or 
                               equal to BRAP_MAX_INTERCHANNEL_DELAY */

    /* Buffer pool required for ring buffer as the destination in the system */
    unsigned int	    uiMaxNumRBuf[BRAP_OutputChannelPair_eMax];
    BRAP_BufferParams   sDstBufSettings[BRAP_OutputChannelPair_eMax];

    /* Buffer pool required for high bit rate outputs */
    bool                        bHbrMode;
}BRAP_DeviceRBufPool;

/*********************************************************************
Summary:
	Structure for informing the maximum number of input and output buffer pairs
	for each audio processing.

Description:
	This structure indicates the possible maximum number of input and output 
	buffer pairs that can be used through out the life time of a particular 
	channel. This information is internally used by the RAP PI to pre-allocate 
	the corresponding number of ring buffers (hence the DRAM) during the channel
	open time. These buffers are freed during the corresponding channel close 
	time.
	
See Also:
	BRAP_ChannelSettings
**********************************************************************/	
typedef struct BRAP_ChannelRBufPool
{
	/* For a decode channel */
    unsigned int	    uiMaxNumOutChPairs[BRAP_MAX_FW_PROC_BRANCHES]; 
    unsigned int	    uiMaxNumInChPairs[BRAP_MAX_DEC_CHAN_WITH_CAPPORT_SOURCE]; 

    /* For a PCM playback channel where application directly reads or writes to
       the DRAM */
    unsigned int	    uiMaxNumRBuf; 
    BRAP_BufferParams	sBufSettings;
}BRAP_ChannelRBufPool;

/***************************************************************************
Summary:
	Structure to hold the SCM specific settings
	
Description:
	This structure holds SCM specific settings like Resident Image Info, Authentication Signatures,
	all other firmware image signatures
****************************************************************************/
#ifdef RAP_SCM_SUPPORT

typedef enum BRAP_HashImageTypes
{
	BRAP_HashImageTypes_eIboot = 0x0,
	BRAP_HashImageTypes_eSystemCode,
	BRAP_HashImageTypes_eSystemRdb,
	BRAP_HashImageTypes_eSystemData,	
	BRAP_HashImageTypes_eTableOfHashes,
	BRAP_HashImageTypes_eScm_1_Code,
	BRAP_HashImageTypes_eScm_2_Code,
	BRAP_HashImageTypes_eScmTask,	
	BRAP_HashImageTypes_eMax
}BRAP_HashImageTypes;


typedef struct BRAP_HashTable
{
	uint32_t ui32HashValue [5];
}BRAP_HashTable;


typedef struct BRAP_SCM_Settings
{
	bool bSCMEnabled;
	uint32_t ui32CaSystemId;
	uint32_t ui32RefHashDigest[5];
	BERR_Code (*pfCallbackOtpProgramming)(void * pScmVersionId, uint32_t ui32CaSystemId);

}BRAP_SCM_Settings;
#endif
/***************************************************************************
Summary:
	Structure to hold the device specific settings
	
Description:
	Currently the Audio module supports the option to use static memory 
	allocation wherein the application needs to pass a the start address 
	of the buffer and its size.
	
See Also:
	BRAP_Open, BRAP_Close
****************************************************************************/
typedef struct BRAP_Settings
{
	bool 				bFwAuthEnable;	/* If Fw authentication is enabled, all the Fw 
	                                                                  execs needs to be downloaded at open time 
	                                                                  i.e bOpenTimeFwDownload should be true. 
	                                                                  Also If its true then the DSP should be in reset
	                                                                  condition in DSP_Open. And the booting of 
	                                                                  DSP should be explicitely called from App 
	                                                                  via a seperate API after HSM Authentication.*/

	bool					bOpenTimeFwDownload; /* 
												bOpenTimeFwDownload = True, If 
												All the FW Execs needs to be 
												download at open time.
												otherwise,
												bOpenTimeFwDownload = False
												*/
	bool			        bExtDeviceMem;		
                                    /* TRUE = Application provides one
							   		   chunk of device memory and RAP PI
							   		   internally manages it.
							   		   FALSE = Application doesn't provide
							   		   device memory .RAP PI uses memory 
							   		   manager to get device memory. */
							   			
	unsigned int	        ui32MemBase;
                                    /* Device memory base to be used.
							   		   This field is valid only when
							   		   bExtDeviceMem = TRUE */
							   			
	size_t			        ui32MemSize;
                                    /* Device Memory size in bytes. 
							   		   This field is valid only when
							   		   bExtDeviceMem = TRUE */
							   		   
    BRAP_DeviceRBufPool 	sDeviceRBufPool;
                                    /* Device memory requirement details */

	const BIMG_Interface    *pImgInterface;
                                    /* Interface to access firmware image 
									   This interface	must be implemented 
									   and the function pointers must be stored
									   here. */
							   
	void                    **pImgContext; 
                                    /* Context for the Image Interface 
									   This is also given by the implementation 
									   of the Image interface */
							   
	bool                   	bEnableSoftLimit; 
									/* TRUE: All mixers carrying PCM data will
									   have Soft-Limiting enabled.
									   FALSE: Soft-Limiting is disabled for all
									   the mixers. */

    bool                    bIndOpVolCtrl;
                                    /* Not used in 3548/3556 currently. Can be ignored.
                                       It has been added here to maintain uniformity
                                       with the general code and with the intention of adding it in future.
                                       3548/3556 uses one mixer per channel already as default
                                       
                                       TRUE: Volume can be controlled per channel
                                       per output port.
                                       FALSE: Same mixer can be used to drive
                                       multiple output ports. */       
#ifdef RAP_SCM_SUPPORT 
	BRAP_SCM_Settings sScmSettings; /*This Structure holds SCM specific settings */
#endif

} BRAP_Settings;

/***************************************************************************
Summary:
	Common channel settings structure.

Description:
	Structure to hold the channel open settings. This structure is used while 
	opening real videochannel.


See Also:
	BRAP_OpenChannel, BRAP_GetDefaultChannelSettings
****************************************************************************/

typedef struct BRAP_RealVideoSettings
{
	unsigned int uiTbd; /*TBD*/
} BRAP_RealVideoSettings;


/***************************************************************************
Summary:
	Common channel settings structure.

Description:
	Structure to hold the channel open settings. This structure is used while 
	opening any RAPTOR audio channel.

    General Notes:
    1. A channel will have a fixed source. If user wants to change the source
       he has to open a new channel.   
    2. eChSubType is required here as this will enable grouping of channels. 

See Also:
	BRAP_OpenChannel, BRAP_GetDefaultChannelSettings
****************************************************************************/
typedef struct BRAP_ChannelSettings
{
    BRAP_ChannelType        eChType;      /* Channel type */

    BRAP_ChannelSubType     eChSubType;     /* Channel Sub-type:
                                               NOTE:
                                               1. This is valid only for a 
                                               decode channel type. For all 
                                               other channel types this should
                                               be set to eNone.
                                               2. For a BD-DVD and HD-DVD 
                                               product, valid values for a 
                                               decode channel are:
                                               Primary and Secondary. */
    BRAP_ChannelRBufPool 	        sChnRBufPool;   
                                        /* Channel DRAM requirement details */
    bool    bEnaIndDly;         /* Enable Independent Delay Support */

    bool                    bOpenTimeWrToRbuf;/*True: If Playback channel can write in to Rbuf at OpenTime*/

#ifdef RAP_REALVIDEO_SUPPORT                                                                    
    /*Real video Settings*/
    BRAP_RealVideoSettings		sRealVideoSettings; 
#endif
}BRAP_ChannelSettings;

/***************************************************************************
Summary:
	Structure that holds the details of the channel

Description:
    Channel detail currently has channel handle only.

See Also:
    BRAP_AssociateChannels()
****************************************************************************/
typedef struct BRAP_ChannelDetail
{
    BRAP_ChannelHandle          hRapCh;         /* [in] Channel handle */
}BRAP_ChannelDetail;

/***************************************************************************
Summary:
	Settings structure required while associating channels together.

Description:
	This structure is an array of channel details structure. While associating
	channels, this settings is required. This settings structure holds the 
	details of each of the channel to be associated in a group.

See Also:
    BRAP_CreateAssociation()
****************************************************************************/
typedef struct BRAP_AssociatedChanSettings
{
    BRAP_ChannelDetail      sChDetail[BRAP_MAX_ASSOCIATED_CHANNELS_IN_GRP];
    BRAP_ChannelHandle     hMasterChannel;
    
}BRAP_AssociatedChanSettings;

/***************************************************************************
Summary:
	Output specific configuration parameters structure

Description:
	Structure to hold the output specific configuration params. This structure 
	is used while configuring an output port.

    Note: 
    7440 A0 FMM architecture mandates a singe setting for I2s0-3. App 
    should explicitly provide the same settings for any subset of I2s from this 
    group.
    
See Also:
    BRAP_SetOutputConfig 
    BRAP_GetDefaultOutputConfig
    BRAP_GetCurrentOutputConfig
****************************************************************************/
typedef struct BRAP_OutputPortConfig
{
    BRAP_OutputPort             eOutputPort;    /* Output port type */
    union 
	{										
		BRAP_OP_SpdifSettings	sSpdifSettings;	/* Spdif settings. It is used 
												   only if the output port is 
												   SPDIF */ 
		BRAP_OP_I2sSettings		sI2sSettings;	/* I2S settings. It is used 
												   only if the output port is 
												   I2S */ 
		BRAP_OP_MaiSettings		sMaiSettings;	/* Mai settings. It is used 
												   only if the output port is 
												   MAI */ 
        BRAP_OP_DacSettings     sDacSettings;	/* Dac settings. It is used 
												   only if the output port is 
												   DAC */ 
        BRAP_OP_MaiSettings		sMaiMultiSettings;	/* Mai settings. It is used 
        										   only if the output port is 
        										   MAI */ 												   
	}uOutputPortSettings;

    	BAVC_Timebase		eOutputTimebase;		/* Timebase to use for the output port */

    BAVC_AudioSamplingRate 		eOutputSR;		/* Sampling rate of the output
                                                   port */		
    bool                	    bCompressed;    /* TRUE: Output port will carry 
                                       			   compressed data. 
                	                               FALSE: Output port will carry 
                	                               PCM data */
    BRAP_SPDIFFM_Settings       sSpdiffmSettings;
                                                /* Spdiff FM settings. It is 
                                                   used only if output port is 
                                                   SPDIF or Mai */

	bool				bUseSpdifPackedChanStatusBits;
							/* Applies only when output port is SPDIF.
							    This field decides whether to use SPDIF channel 
							    status bits packed in two 32-bit words
							    OR seperated into individual parameters.
							    TRUE = Use field sSpdifPackedChanStatusBits
							    FALSE = Use field sSpdifChanStatusParams 
							    When TRUE, all the channel status bits can be
							    set by application. If a decode channel is using
							    this port, then firmware is disabled from programming
							    any of the channel status bits. 
							    This field can be changed only when output port is
							    in "Stop" state. */

	BRAP_OP_SpdifPackedChannelStatusBits		sSpdifPackedChanStatusBits;
							/* Applies only when output port is SPDIF.
							    This field is valid only when
							    bUseSpdifPackedChanStatusBits = TRUE. */
							    
    BRAP_OP_SpdifChanStatusParams sSpdifChanStatusParams;   
                                                /* This is used only when SPDIF 
                                                   output port is configured for
                                                   this audio channel. 
                                                   This field is valid only when
                                                   bUseSpdifPackedChanStatusBits = FALSE.
                                                   It contains user parameters 
                                                   required for the SPDIF 
                                                   channel status bits*/
    unsigned int                uiCpToggleRate;
                                    /* Allows toggling of the copy protection bit 
                                       (bit 2, Cp) according to section A.2 of IEC 
                                       60958-3. When enabled, valid value should 
                                       be between 4 to 10 (in Hz). 0 indicates 
                                       it is disabled. */

	unsigned int                uiOutputBitsPerSample; 
                                                /* Bits per sample to be 
            									 output at the output port.
            									 The possible legal values for 
            									 this field are:
            									 16 to 32, for I2S outputs
            									 16 to 24, for all other outputs
            									 */
	bool                        bHbrEnable;
                                    /* This flag is honoured only for HDMI and 
                                       I2sMulti_1 (I2s5 - I2s8) output ports.
                                       TRUE: Port is running on High Bit Rate
                                       FALSE: Port is running on normal rate */
            									 
/*    bool bIndpDlyRqd; 	*//* TRUE: This output ports may require to have and independent delay at some time.
                        FALSE: This output port will NEVER have independent delay wrt to the other ports*/
    int iDelay;         /* The independent delay to be applied to this  outputport in ms.
                        Note: This delay value is used only if the channel is opened with bSelectDelay as TRUE  */
}BRAP_OutputPortConfig;

#if (BRAP_7550_FAMILY !=1)
/***************************************************************************
Summary:
	Input (Capture Port) specific configuration parameters structure

Description:
	Structure to hold the Capture specific configuration params. This structure 
	is used while configuring an External Capture port.

See Also:
    BRAP_SetInputConfig 
    BRAP_GetDefaultInputConfig
    BRAP_GetCurrentInputConfig
****************************************************************************/
typedef struct BRAP_InputPortConfig
{
    BRAP_CapInputPort       eCapPort;   /* Input Capture Port.
                                           Note: Application can set the Input
                                           config only for External capture 
                                           port as I2S0-in. The Internal capture
                                           ports are configured internally by 
                                           RAP PI*/
     BRAP_BufDataMode	        eBufDataMode;	
                                     /* PCM Data Mode. It defines how the 
									    capture PCM data will be stored in 
									    the ring buffers. This will decide 
									    whether 1 or 2 ring buffers are to be
									    used. Should be less than or equal to
                                        the Max mode specified at Channel 
									    Open */
  	
    BRAP_InputBitsPerSample	    eInputBitsPerSample;
                                    /* Input bits per sample of PCM data
                                       Note: This param will be used only when 
                                       eCapInputType is BRAP_CapInputType_eExtI2s. 
                                       For all other internal capture input types, 
                                       input bits/sample will be derived internally 
                                       from the captured output port's output 
                                       bits/sample */
    BAVC_AudioSamplingRate      eSampleRate;
                                    /* Input sample rate. When capture
                                       port is BRAP_CapInputPort_eBtsc, the
                                       output sample rate of BTSC decoder would
                                       be set equal to this rate */
    BRAP_CAPPORT_Params         sCapPortParams;
                                    /* External capture input Params */
}BRAP_InputPortConfig;
#endif


/***************************************************************************
Summary:
	DTS-Neo configuration details

Description:
	Structure to hold the DTS-Neo specific configuration details. This 
	configuration structure is used when a destination has DTS-Neo as the 
	audio processing.
	
See Also:

****************************************************************************/
typedef struct BRAP_DtsNeoConfig
{
    BRAP_OutputMode                 eOpMode;
    BRAP_DSPCHN_FidelitySubBands    eFidelitySubBands; 
                                        /* Fidelity sub-bands */
    BRAP_DSPCHN_CinemaMode          eCinemaMode;       
                                        /* Cinema mode */   
	unsigned int	                uiCenterGain;       
                                        /* Center gain. Input value range = 0% 
                                           to 100%. 100% corresponds to value of
                                           unity. Any value above 100% is 
                                           considered as unity */
}BRAP_DtsNeoConfig;



/***************************************************************************
Summary:
	Output port details used while adding an output port

Description:
	Structure to hold the output group specific details. This output group 
	structure is used while adding an output port type destination to an 
	associated group of channels.
	
See Also:
    BRAP_AddDestDetails, BRAP_DstDetails
****************************************************************************/
typedef struct BRAP_OutputPortDetails
{
	BRAP_OutputPort             eOutput[BRAP_OutputChannelPair_eMax]; 
                                                /* Output type */
	
    BRAP_OutputMode             eAudioMode;     /* Audio Mode for this 
                                                   output group */
    bool                        bLfeOn;         /* Whether LFE exists for this 
                                                   output group */
    bool                        bDownmixedOpPort; /* TRUE , If the destination added will carry
                                                                    Downmixed data */
}BRAP_OutputPortDetails;

/***************************************************************************
Summary:
	RBUF details used while adding an RBUF as Destination.

Description:
	Structure to hold the RBUF specific Settings. These settings are used while
	adding a RBUF as a destination to an associated group of channels.
	
See Also:
    BRAP_DstDetails
****************************************************************************/
typedef struct BRAP_RbufDetails
{
	/* Used for storing granted resources */
    unsigned int                uiRBufId[2 * BRAP_OutputChannelPair_eMax]; 
    unsigned int                uiDstChId[BRAP_OutputChannelPair_eMax]; 
    BRAP_CapInputPort           eCapPort[BRAP_OutputChannelPair_eMax];

    bool                        bDontPauseWhenFull;
                                /* 
                                    FALSE: stop when the rbuf is full, wait for app to consume it
                                    TRUE:  continue consumption even when rbuf is full
                                */
    

	/* Should we move the next 3 fields to common destination details? */
    BRAP_OutputMode             eAudioMode;     /* Audio Mode for this 
                                                   output group */
    bool                        bLfeOn;         /* Whether LFE exists for this 
                                                   output group */
    BAVC_AudioSamplingRate      eSampleRate;    /* Sample rate of the data
                                                   coming in RBUFs */
    bool                    bCompress;
                                    /* Type of data coming to the Ring Buffer
                                       True for SPDFIF Compressed input */
    
    BRAP_BufDataMode        eBufDataMode[BRAP_OutputChannelPair_eMax];
                                     /* PCM Data Mode. It defines how the 
									    capture PCM data will be stored in 
									    the ring buffers. This will decide 
									    whether 1 or 2 ring buffers are to be
									    used */

    BRAP_RBUF_Settings	    sLtOrSnglRbufSettings[BRAP_OutputChannelPair_eMax];
                                    /* Left or single ring buffer 
									   settings structure. This will be 
									   used for all ePcmDataMode that need 
									   either 1/2 buffers. This variable is 
									   ignored if eCapMode ==  
									   BRAP_CaptureMode_eByPass */

	BRAP_RBUF_Settings	    sRtRbufSettings[BRAP_OutputChannelPair_eMax];
                                    /* Right ring buffer settings structure. 
									   This will be used only when ePcmDataMode 
									   indicates stereo non-interleaved PCM 
									   type that needs 2 buffers. This 
									   variable is ignored if eCapMode ==  
									   BRAP_CaptureMode_eByPass */

#ifdef  RAP_AUTOMATED_UNIT_TEST
    unsigned int                uiDelay;	
#endif
}BRAP_RbufDetails;

/***************************************************************************
Summary:
	Structure containing the details of the audio destination used while adding 
	a destination.

Description:
	Structure to supply the corresponding details of the destination type to be 
	added to an associated group of audio channels.

See Also:
    BRAP_AddDestination, BRAP_OutputPortDetails
****************************************************************************/
typedef struct BRAP_DstDetails
{
    BRAP_AudioDst               eAudioDst;     /* Destination of audio data 
                                                   from the Raptor channel */
    union 
    {
        BRAP_OutputPortDetails  sOpDetails;
        BRAP_RbufDetails        sRBufDetails;
    }uDstDetails;
}BRAP_DstDetails;

/***************************************************************************
Summary:
	Structure containing the details of the audio destination used while 
	removing a destination.

Description:
	Structure to supply the corresponding details of the destination type to be 
	removed from an associated group of audio channels.

See Also:
    BRAP_RemoveDestination
****************************************************************************/
typedef struct BRAP_RemoveDstDetails
{
    BRAP_AudioDst               eAudioDst;      /* Destination of audio data 
                                                   from the Raptor channel */
    union 
    {
      	BRAP_OutputPort         eOutput[BRAP_OutputChannelPair_eMax]; 
                                                /* Output port group */
    }uDstDetails;
}BRAP_RemoveDstDetails;



/***************************************************************************
Summary:
	Mixing Coefficients.

Description:
	

See Also:
	BRAP_ChannelParams
****************************************************************************/
typedef struct BRAP_MixingCoef
{
    uint32_t    ui32Coef[BRAP_OutputChannel_eMax][BRAP_OutputChannel_eMax];
} BRAP_MixingCoef;


/***************************************************************************
Summary:
	Common channel params structure.

Description:
	Structure to hold the channel start parameters. This structure is used 
	while starting any RAPTOR audio channel.

See Also:
	BRAP_StartChannel, BRAP_GetDefaultChannelParams
****************************************************************************/
typedef struct BRAP_ChannelParams
{
    BAVC_Timebase		        eTimebase;		
                                        /* Timebase to use */
                                        
    BRAP_AudioSource            eAudioSource;   
                                        /* Source of audio data to the Raptor 
                                           channel */

	BAVC_XptContextMap	        sXptContextMap;	 
                                        /* This structure holds the CDB and 
										   ITB register addresses for the 
										   corresponding Rave Context 
										   allocated by the Xpt PI.*/

	bool	                    bPlayback;			
                                        /* TRUE: If the input transport channel 
                                           is a playback channel
							               FALSE: If the input transport channel 
							               is a broadcast channel */
							               
    BRAP_OutputMode             eInputAudMode;
                                        /* Input mode of the incoming data for a
                                           primary or secondary decode channel. 
                                           This parameter is ignored for all 
                                           other channels and channel sub types
                                        */
                                           
    bool                        bInputLfePresent;
                                        /* TRUE: If incoming data for a primary
                                           or secondary decode channel contains
                                           LFE audio channel. This parameter is 
                                           ignored for all other channels and 
                                           channel sub types */

    BAVC_AudioSamplingRate      eInputSR;
                                        /* Sampling rate of the decoded audio 
                                           data expected at the ring buffer 
                                           (input to the FMM). For a decode 
                                           channel, valid value should be passed
                                           for DVD application and for all other
                                           applications(DTV, STB, etc) pass 
                                           BAVC_AudioSamplingRate_eUnknown. */

	BRAP_DSPCHN_AudioParams	    sDspChParams;
                                        /* DSP Context specific parameters */

    BRAP_CaptureMode	        eCapMode;		
                                    /* Capture mode: Whether this channel 
									   is used for capture only or for 
									   full duplex or bypass mode. This field
									   is considered only if channel type is
									   Capture */

    BRAP_CapInputPort	        eCapInputPort;	
                                    /* Type of PCM Capture Input Source.This 
                                       field is considered only if channel type
                                       is Capture */

    BRAP_BufDataMode	        eBufDataMode;	
                                     /* PCM Data Mode. It is used for PCM PB 
                                        channel. This will decide whether 1 
                                        or 2 ring buffers are to be used */
    BRAP_InputBitsPerSample     eInputBitsPerSample;
								    /* Bit resolution of the Source Stream */
                                    
    bool  						bLoopAroundEn;   
									/* TRUE: Play the data in ring buffer 
									   over and over again. In this mode, the 
	        						   application can write the ring buffer 
	        						   once and then start the playback channel, 
        							   so that the data can be played back over 
        							   and over again. FALSE: Otherwise */

    BRAP_BufferParams           sBufParams;
                                    /* This structure holds external buffer 
                                       allocation specific parameters */

    unsigned int                      uiStartWRPtrLocation;                                            
                                  /* It specifies StartWrite pointer for the Ringbuffer of PCM playback channel. 
                                  In default its value is -1, which will program the startWR pointer to End -256*/                                       
                                       
    BRAP_OutputChannelPair      eInputChPair;
                                    /* Channel pair data carried by a PCM 
                                       playback channel. Note: This param is 
                                       used only for a PCM playback channel */
    BRAP_MixingCoef             sMixingCoeff;
                                    /* Mixing coefficients to be applied to 
                                       mixer when mixing with other RAP Channel*/

    bool                        bIsPcmSigned;
                                    /* PCM Data Format. It is used for PCM PB 
                                       channel only. This will decide whether
                                       content is signed or unsigned */

 
    BRAP_AudioDescriptorFadeHandle hAdFade;     
                                        /* Audio descriptor fade table handle. This field should be set
                                           only if this channel belongs to audio descriptor channel 
                                           pair. Application should send the same audio
                                           descriptor handle to both (audio descriptor channel and
                                           primary channel) channels in an audio descriptor channel
                                           pair. For non audio descriptor channel this field should be
                                           set to NULL. */      
    BRAP_DataEndian     eDataSwap;  /* To select whether FMM takes data as per
                                       system endian-ness or swapped. This field
                                       is valid only for PCM Playback channels. */
    BRAP_DSPCHN_LowDelayEnableMode	eLowDelayEnableMode;
                                        /* To enable or disable the low delay mode */                                                                         
    bool    bEnableFixedSampleRateOutput;/*True:    Audio output will always be  at 48Khz, 
                                                    HW SRC will be done based on stream sample rate.
                                                    Only applicable for Decode Channel Outputting PCM
                                                    data. Because HW SRC of Compressed data will be garbage*/                                                                                  
#ifdef RAP_REALVIDEO_SUPPORT                                                                    
    BRAP_VideoParams		    sVideoParams; /*Real Video Params*/
#endif
}BRAP_ChannelParams;


/***************************************************************************
Summary:
	Structure for channel configuration parameters

Description:
	This structure contains channel configuration parameters that can be
	changed on the fly.
	
See Also:
	BRAP_SetChannelConfig
	BRAP_GetDefaultChannelConfig
	BRAP_GetCurrentChannelConfig
****************************************************************************/
typedef struct BRAP_ChannelConfigParams
{
    BRAP_ChannelType                eChType;
                                        /* Channel type */
    BRAP_MixingCoef                 sMixingCoef;
                                        
	union 
	{
	    BRAP_DEC_ConfigParams     sDecConfigParams;
	} uChConfigParams;				/* Decode configuration parameters */
} BRAP_ChannelConfigParams;

/***************************************************************************
Summary:
	Structure for channel gain and pan control coefficients.

Description:
    This structure contains the info about the four scaling coefficients used
    to control channel input gain and pan. They are used to scale the input 
    stream pair before being mix.

    In the scaling coefficients, the 23 LSB of this field is the fractional 
    part and the 5 MSB is the integer part.  This is a 2'compliment number 
    range from -16 to 16.

    The mixing equation for each input stream pair is as follow:

    Lout = Left_Coef_0 * Lin + Right_Coef_0 * Rin
    Rout = Left_Coef_1 * Lin + Right_Coef_1 * Rin.
	
See Also:
****************************************************************************/
typedef struct BRAP_Coefficient
{
    uint32_t  ui32Left;     /* Left Coefficent */
    uint32_t  ui32Right;    /* Right Coefficent */
}BRAP_Coefficient;

typedef struct BRAP_GainControl
{
    /* The gain values for the BRAP_GainControl are taken in 5.23 format */
    
    uint32_t  ui32LeftGain[BRAP_OutputChannelPair_eMax];    /* Left Gain */
    uint32_t  ui32RightGain[BRAP_OutputChannelPair_eMax];   /* Right Gain */
}BRAP_GainControl;

/***************************************************************************
Summary:
	Structure for Setting the static downmixinng coefficients

Description:
	This structure contains the information about the Static Downmixing 
	Coefficients that are passed by the application to the PI.

	These coefficients are exposed as uint32_t in 5.23 format (5 integer 
	and 23 fractional) where bits 0 - 22 (23 LSB) represents the fractional 
	part and bits 24 - 27 represent the integer part. 
	
See Also:
    BRAP_SetDownMixCoef
****************************************************************************/
typedef struct BRAP_DownMixingCoef
{
    BRAP_OutputMode	    eInputMode;
    bool				bInputLfePresent;
    BRAP_OutputMode	    eOutputMode;
    bool				bOutputLfePresent;
    union 
    {	
    	uint32_t        ui32DnCoef5_1To2_0[2][6];
	    uint32_t        ui32DnCoef7_1To2_0[2][8];
        uint32_t        ui32DnCoef7_1To5_1[6][8];
    } uDnCoeff;
    
}BRAP_DownMixingCoef;

/***************************************************************************
Summary:
	Structure for configuring the audio processing stages for a destination
	
Description:
	This structure contains the information about the audio processing stages that need to 
	be added for a destination.
	
See Also: 
    BRAP_SetPostProcessingStages
    BRAP_GetPostProcessingStages
    BRAP_SetPreProcessingStages
    BRAP_GetPreProcessingStages
****************************************************************************/
typedef struct BRAP_ProcessingSettings
{
    BRAP_ProcessingStageHandle          hAudProcessing[BRAP_MAX_PP_PER_BRANCH_SUPPORTED];
                                         /* Audio processing stages for this destination handle. 
                                            For a given hRapCh, if initial stage handles 
                                            are common with other destination, then those
                                            stages are shared between destinations.
                                            Uncommon stages are implemented on
                                            seperate audio processing branches. Unused
                                            entries in hAudProcessing should be initialized
                                            to NULL. */
} BRAP_ProcessingSettings;

/***************************************************************************
Summary:
	Structure will contian the bebug info of FW params depening on algo type.
****************************************************************************/

typedef struct BRAP_FwDebugInfo
{
    BRAP_DSPCHN_AudioType    eAlgoType; 
    union
    {
        BRAP_DSPCHN_DtsDebugInfo    sDtsDebugInfo; /* For DTS/DTS-HD */
        BRAP_DSPCHN_DdpDebugInfo    sDdpDebugInfo; /* For AC3/AC3+ */
    }uFwAlgoInfo;
}BRAP_FwDebugInfo;


/*********************************************************************
Summary: Structure to hold configuration of audio descriptor fade table object.
Description:
    This structure contains configuration of audio descriptor fade table object. 
See Also:
**********************************************************************/ 
typedef struct BRAP_AudioDescriptorFadeTableParams
{
    BRAP_ChannelHandle hPrimaryChannel;     /* Audio Descriptor Primary Channel handle.
                                                Not programmable on-the-fly */
    BRAP_ChannelHandle hDescriptorChannel; /* Audio Descriptor channel handle.
                                                Not programmable on-the-fly */
} BRAP_AudioDescriptorFadeTableParams;

/*}}}*/

/*{{{ Function prototypes */

/***************************************************************************
Summary:
	Gets the default settings of the Raptor Audio device.
Description:
	This API returns the default settings of the Raptor Audio device.
Returns:
	BERR_SUCCESS 
See Also:
	BRAP_Open.
***************************************************************************/
BERR_Code BRAP_GetDefaultSettings(
	BRAP_Settings 			*pDefaultSettings /* [out] Default device 
													   settings */
	);

/***************************************************************************
Summary:
	Initializes the Raptor Audio Device.

Description:
	This function should be called to initialize the Raptor Audio Device. 
	This API must be called prior to any other Raptor PI call.
	As a result a handle to the Raptor Audio Device is returned by this API, 
	which represents the instance of this device. This Device handle needs 
	to be passed to APIs operating at device level.
	
Returns:
	BERR_SUCCESS - If open is successful else error	

See Also:
	BRAP_Close, BRAP_GetDefaultSettings.
***************************************************************************/
BERR_Code BRAP_Open(
	BRAP_Handle 			*phRap,			/* [out] The RAP handle returned*/
	BCHP_Handle 			hChip,			/* [in] Chip handle */
	BREG_Handle				hRegister,		/* [in] Register handle */
	BMEM_Handle 			hMemory,		/* [in] Memory heap handle */
	BINT_Handle				hInt,			/* [in] Interrupt module handle */
	BTMR_Handle             hTimer,         /* [in] Timer handle */
	const BRAP_Settings		*pSettings		/* [in] Device settings */
	);

/***************************************************************************
Summary:
	Closes the Raptor Audio Device.

Description:
	This function should be called when the application is exiting or the 
	application doesn't intend to use Raptor Audio PI further. This call 
	destroys the instance of the Raptor Audio Device handle. 

Returns:
	BERR_SUCCESS - If open is successful else error	

See Also:
	BRAP_Open.
***************************************************************************/
BERR_Code BRAP_Close(
	BRAP_Handle 			hRap	/* [in] The Raptor Audio device handle*/
	
	);

/***************************************************************************
Summary:
	Standby settings. Currently unused.
****************************************************************************/
typedef struct BRAP_StandbySettings
{
    bool dummy; /* placeholder to avoid compiler warning */
} BRAP_StandbySettings;


typedef struct BRAP_Capabilities
{ 
    /* If decode of a codec is supported, decodeSupported[codec] is true. */ 
    /* Compressed passthrough is available for all codecs */ 
    bool bDecodeSupported[BRAP_DSPCHN_AudioType_eMax]; 
    /* If a particular processing type is supported, decodeSupported[processingType] is true. */ 
    bool bProcessingSupported[BRAP_ProcessingType_eMax]; /* TODO: Could be extended further to reflect other runtime capabilities */ 
} BRAP_Capabilities;


/***************************************************************************
Summary:
	Enter standby mode with the Raptor Audio Device.

Description:
	This function puts the Raptor Audio Device into standby mode, 
	if supported. All Raptor channels must be in a stopped state 
	in order to successfully enter standby mode.
	If standby mode is not supported, calling function has no effect.

	When in standby mode, the device clocks are turned off,
	resulting in a minimal power state.

	No BRAP_ calls should be made until standby mode exitted by calling
	BRAP_Resume().

Returns:
	BERR_SUCCESS - If standby is successful, otherwise error

See Also:
	BRAP_Resume
***************************************************************************/
BERR_Code BRAP_Standby(
	BRAP_Handle 			hRap, 		/* [in] RAP device handle */
	BRAP_StandbySettings 	*pSettings 	/* [in] standby settings */
);

/***************************************************************************
Summary:
	Exit standby mode with the Raptor Audio Device.

Description:
	This function exits the Raptor Audio Device from standby mode. 
	After exitting standby mode, upper-level SW is free to call
	BRAP_ functions.

Returns:
	BERR_SUCCESS - If resume is successful, otherwise error

See Also:
	BRAP_Standby
***************************************************************************/
BERR_Code BRAP_Resume(
	BRAP_Handle 			hRap 		/* [in] RAP device handle */
	);

/***************************************************************************
Summary:
	Gets the default channel settings for an audio channel type.
	
Description:
	This API returns the default channel settings of a channel type.
	The default channel settings is -
	"Decode channel with eNone channel sub-type and CDB/ITB (transport) as the 
	audio data source"
	
Returns:
	BERR_SUCCESS on success else error
	
See Also:
	BRAP_OpenChannel
***************************************************************************/
BERR_Code BRAP_GetDefaultChannelSettings(
	BRAP_Handle 			hRap,			/* [in] The RAP device handle */
	BRAP_ChannelSettings    *pDefSettings	/* [out] Default channel settings */
	);

/***************************************************************************
Summary:
	API used to open a channel.
	
Description:
	It is used to instantiate a channel. It allocates channel handle 
	and resource required for the channel if any.
	
	Note: This should be called only after the device has been opened with 
	BRAP_Open().
	
Returns:
	BERR_SUCCESS on success else error
	
See Also:
	BRAP_CloseChannel
****************************************************************************/
BERR_Code BRAP_OpenChannel( 
	BRAP_Handle 			    hRap,		    /* [in] The Raptor Audio 
	                                               device handle*/
	BRAP_ChannelHandle 		    *phRapCh,		/* [out] The RAP Channel 
	                                               handle */
	const BRAP_ChannelSettings	*pChnSettings   /* [in] Channel settings*/ 
	);


/***************************************************************************
Summary:
	API used to close a Raptor Audio Channel.

Description:
	It closes the instance of a raptor channel operation. It frees the channel
	handle and resources associated with it if any.

Returns:
	BERR_SUCCESS on success else error

See Also:
	BRAP_OpenChannel
****************************************************************************/
BERR_Code BRAP_CloseChannel( 
	BRAP_ChannelHandle 	hRapCh	                /* [in]The RAP Channel handle */
	);


/***************************************************************************
Summary:
	Associates/Groups a set of audio channels together.
	
Description:
	This API associates/groups a set of audio channel handles together. All 
	those channnels that are required to be mixed together to produce an 
	audio, should be associated/grouped together using this API.

	E.g. Primary decode, secondary decode and PCM playback (sound effect) 
	channels are mixed together to produce the desired audio output. So,
	these channels are required to be associated together.

	Note: 
	1. In case, just one channels is sufficient to produce the desired
	   audio then just a single channel can be added to the association. 
	2. This operation is permissible only when all the channels are in 
	   the stop state.
	
Returns:
	BERR_SUCCESS for a successful grouping, else returns an error.
		
See Also:
	BRAP_DestroyAssociation.
***************************************************************************/
BERR_Code BRAP_CreateAssociation(
    BRAP_AssociatedChannelHandle    *phAssociatedCh,     
                                                /* [out] Handle to the group
                                                   of associated audio 
                                                   channels */
    BRAP_AssociatedChanSettings     *pAssociatedChSettings        
                                                /* [in] Pointer to a stucture
                                                   that holds the details of 
                                                   channels to be grouped 
                                                   together */
	);

/***************************************************************************
Summary:
	Ungroups/dissociates a group of audio channels.
	
Description:
	This API ungroups/dissociates a group of audio channels. Note: All the 
	channel handles will still be valid after the execution of this API.
	
Returns:
	BERR_SUCCESS for a successful ungrouping else returns an error. Also,
	the phAssociatedChan is made NULL in this API.
	
See Also:
	BRAP_CreateAssociation.
***************************************************************************/
BERR_Code BRAP_DestroyAssociation(
    BRAP_AssociatedChannelHandle    hAssociatedCh     
                                                /* [in] Handle to the 
                                                   group of audio channels */
	);

/***************************************************************************
Summary:
	API used to configure an output port. This API needs to be called 
	seperately for each output port that is in use by the system.    

Description:
    This API needs to be called atleast once before any channel using this 
    output port can be started.

    Note that an output port may be in use by mutliple channels.
    => an output port is 'stopped' when ALL the channels using it are stopped.
    => an output port is 'running' when one or more channels using it have been 
    started.

    Configuration parameters fall into 2 types:

    1. 'start-time only parameters can be reprogrammed every time an output 
    port is stopped and restarted. These can be modified only when the output 
    port is stopped and will take effect the next time the output port is 
    started.

    2. 'on-the-fly parameters can be reprogrammed any time ie even while the 
    output port is running, without stopping and restarting the channel.

    The configuration paramters will take effect for ALL channels that are 
    currently using this output port.

    Refer to the description of the output configuration paramters for details 
    on which of them are start-time paramters and which can be changed 
    on-the-fly.  

Returns:
	BERR_SUCCESS on success
	BRAP_ERR_OUPUT_PORT_NOT_SUPPORTED if the output port is not supported
	BERR_NOT_SUPPORTED if there is an attempt to change the start-time 
	paramters while the output port is running - ie if the output port is in 
	use by ANY running channel.

See Also:
	BRAP_GetDefaultOutputConfig
	BRAP_GetCurrentOutputConfig
****************************************************************************/
BERR_Code BRAP_SetOutputConfig (
	BRAP_Handle		            hRap,		    /* [in] RAP device handle */
	const BRAP_OutputPortConfig	*pOpConfig	    /* [in] Output port 
	                                               configuration */
	);

/***************************************************************************
Summary:
	Get default output port configuration

Description:
	This function gets the default output port configuration 
	
Returns:
	BERR_SUCCESS if successful else error value

See Also:
	BRAP_SetOutputConfig
	BRAP_GetCurrentOutputConfig
****************************************************************************/
BERR_Code BRAP_GetDefaultOutputConfig (
	BRAP_Handle	            hRap,	        /* [in] RAP device handle */
	BRAP_OutputPort         eOutput,        /* [in] Output port */
	BRAP_OutputPortConfig	*pOpConfig      /* [out] Default configuration for
	                                           the output port */
	);

/***************************************************************************
Summary:
	Get current output port configuration

Description:
	This function gets the current output port configuration. If the caller has 
	not configured the output port before, this API will return error. 
 	
Returns:
	BERR_SUCCESS if successful else error value

See Also:
	BRAP_SetOutputConfig
	BRAP_GetDefaultOutputConfig
****************************************************************************/
BERR_Code BRAP_GetCurrentOutputConfig (
	BRAP_Handle	            hRap,	        /* [in] RAP device handle */
	BRAP_OutputPort         eOutput,        /* [in] Output port */
	BRAP_OutputPortConfig	*pOpConfig      /* [out] Current configuration for
	                                           the output port */
    );

/***************************************************************************
Summary:
	API to add destination to a group of raptor channel(s). 

Description:
o	This PI is used to add destination to a channel group.

o   This PI should be called only after the channel has been stopped. It 
    should not be called while the channel is still running.

o	Usage Example1: A group of channels sending data to 
	a) I2s0-3 7.1 mode 
	b) DAC 2.0 mode
	c) I2s4 2.0 mode
	Call sequence:
	After forming the associated channel handle(hAsctdChan), 
	a) BRAP_AddDestination(hAsctdChan, settings for I2s0-3);
	b) BRAP_AddDestination(hAsctdChan, settings for DAC);
	c) BRAP_AddDestination(hAsctdChan, settings for I2s4);

o	Hardware Limitation: Can't group other than I2s0-3 or one of its subset.
	
	TODO: Add more limitations and usage cases.
	
Returns:
	BERR_SUCCESS on success else error
	
See Also:
    BRAP_RemoveDestination
    BRAP_CreateAssociation
****************************************************************************/
BERR_Code BRAP_AddDestination(
    BRAP_AssociatedChannelHandle    hAssociatedCh,     
                                            /* [in] Handle to the group of 
                                               associated audio channels */
    BRAP_DstDetails                 *pDstDetails,
                                            /* [in] Details of the destination 
                                               to be added to the channel group 
                                            */
    BRAP_DestinationHandle          *hDstHandle
                                            /* [out] Destination handle that
                                               represents this distination */                                            
    );

/***************************************************************************
Summary:
    Removes/de-links a destination from a group of associated channels
   
Description:
o   This PI can be used to remove any of the destinations associated with a 
    channel group.

o   This PI should be called only after the channel has been stopped. It 
    should not be called while the channel is still running.

o   It will close and free all the resources opened during the corresponding
    call to the BRAP_AddDestination().

Returns:
	BERR_SUCCESS if successful else error value.            

See Also:
    BRAP_AddDestination
    BRAP_CreateAssociation
    BRAP_DestroyAssociation
****************************************************************************/
BERR_Code BRAP_RemoveDestination(
    BRAP_AssociatedChannelHandle    hAssociatedCh,     
                                            /* [in] Handle to the group
                                               of audio channels */

	BRAP_DestinationHandle          hDestHandle
	                                        /* [in] Destination handle to be 
	                                           removed from this group of 
	                                           channels */
    );

/***************************************************************************
Summary:
	Gets the default params for a channel type.
	
Description:
	This API returns the default parameters for an audio channel type. 
	
Returns:
	BERR_SUCCESS if successful else error value.            
	
See Also:
	BRAP_StartChannel
***************************************************************************/
BERR_Code BRAP_GetDefaultChannelParams(
	BRAP_ChannelHandle      hRapCh,		/* [in] The RAP channel handle */
	BRAP_ChannelParams      *pDefParams	/* [out] Default channel parameters */
	);

/***************************************************************************
Summary:
	Starts an audio channel.
	
Description:
	This API is required to start the selected channel.
	
Returns:
	BERR_SUCCESS if successful else error value.            
	
See Also:
	BRAP_StopChannel
	BRAP_GetDefaultChannelParams
****************************************************************************/
BERR_Code BRAP_StartChannel ( 
	BRAP_ChannelHandle 			hRapCh,		    /* [in] RAP Channel handle */
	const BRAP_ChannelParams	*pAudioParams	/* [in] Audio parameters for 
	                                               starting this channel */
	);

/***************************************************************************
Summary:
	Stops a channel

Description:
	This API is required to stop a channel.	

Returns:
	BERR_SUCCESS if successful else error value.            

See Also:
	BRAP_StartChannel
****************************************************************************/
BERR_Code BRAP_StopChannel ( 
	BRAP_ChannelHandle 	        hRapCh		    /* [in] RAP channel handle */
	);

/***************************************************************************
Summary:
	Change channel configuration parameters on the fly. 

Description:
	This function sets an audio channel configuration parameters that can be 
	set	on the fly.
	Currently, this is applicable only for a decode channel, for all others
	it will return an error. 

	Change in configuration parameters depend on the timing of calling
	this API. There are following three cases.
	1. If currently decode is not happening on this channel then all 
	configuration parameters are stored in PI. When next decode starts, common 
	parameters are applied	irrespective of audio type of next decode, but 
	audio-type specific parameters are applied only on next decode with same 
	audio type.
	2. If currently channel is decoding other audio type,then only common 
	parameters are applied immediately and audio-type specific parameters are 
	stored in PI. Audio-type specific parameters are applied on next decode 
	start with same audio type.
	3. If decode is running with the same audio type, all the configuration 
	parameters are applied immediately.

	Note: It is mandatory to call this API atleast once before starting a decode
	channel.
    
Returns:
	BERR_SUCCESS if successful else error value.            

See Also:
	BRAP_GetDefaultChannelConfig
	BRAP_GetCurrentChannelConfig
****************************************************************************/
BERR_Code BRAP_SetChannelConfig (
	BRAP_ChannelHandle		    hRapCh,		        /* [in] RAP channel handle */
	BRAP_ChannelConfigParams	*psChConfigParams	/* [in] Channel configuration
											           parameters */
	);

/***************************************************************************
Summary:
	Get default channel configuration parameters

Description:
	This function gets the default configuration parameters for given 
	channel type. 

Returns:
	BERR_SUCCESS if successful else error value.            

See Also:
	BRAP_SetChannelConfig
	BRAP_GetCurrentChannelConfig
****************************************************************************/
BERR_Code BRAP_GetDefaultChannelConfig (
	BRAP_ChannelHandle	        hRapCh,	/* [in] RAP channel handle */
	BRAP_DSPCHN_AudioType	    eType,	/* [in] Audio type for which to get
									   default configuration parameters.
									   This field is valid only if the 
									   hRapCh is of decode type. For 
									   all others this is ignored */
	BRAP_ChannelConfigParams	*psChConfigParams	
	                                /* [out] Channel configuration
									   parameters */
	);
									   
/***************************************************************************
Summary:
	Get current channel configuration parameters

Description:
	This function gets the current configuration parameters for given channel 
	type.

Returns:
	BERR_SUCCESS if successful else error value.            

See Also:
	BRAP_SetChannelConfig
	BRAP_GetDefaultChannelConfig
****************************************************************************/
BERR_Code BRAP_GetCurrentChannelConfig (
	BRAP_ChannelHandle	        hRapCh,	/* [in] RAP channel handle */
	BRAP_DSPCHN_AudioType	    eType,	/* [in] Audio type for which to get
									       Current configuration parameters.
                					       This field is valid only if the 
									       hRapCh is of decode type. For 
									       all others this is ignored */									   
	BRAP_ChannelConfigParams    *psChConfigParams	
	                                    /* [out] Channel configuration
									       parameters */
	);

/***************************************************************************
Summary:
	API to retrieve maximum number of channels supported for a channel type.

Description:
	This API can be used to retrieve the maximum number of channels supported 
	for a particular channel type by the Raptor Audio PI.
	
Returns:
	BERR_SUCCESS if successful else error value.            
	
See Also:
	
****************************************************************************/
BERR_Code BRAP_GetTotalChannels( 
	BRAP_Handle 	    hRap,			/* [in] Raptor Audio device handle */
	BRAP_ChannelType    eChanType,      /* [in] Channel type for which the max
	                                       number of channels are sought */
	unsigned int	    *pTotalChannels	/* [out] Total decode Audio channels 
											supported */
	);

/***************************************************************************
Summary:
	API returns revision information.

Description:
	Gets the revision information of all the firmwares executing on Raptor 
	DSP and FMM hardware as well as the revision information of the Raptor 
	Audio PI.
Returns:
	BERR_SUCCESS if successful else error value.            

See Also:
	None
	
****************************************************************************/
BERR_Code BRAP_GetRevision ( 	
	BRAP_Handle 			hRap,		/* [in] The RAP device handle */
	BRAP_RevisionInfo		*pRevInfo	/* [out]Version infomation of Raptor 
												Audio PI, the microcode 
												running on DSP and 
												Microsequencer */ 
);

/***************************************************************************
Summary:
    Sets the output volume for left and right channels.

Description:
    This function sets the output left and right channel volume. In hardware,
    the output volume control is done at the mixer feeding data to an output 
    port. The mixer output volume control is linear in hardware, ranging from 
    0 min to 8000 max volume. This API has done a mapping from this linear range
    to 1/100 DB. This function gets values in 1/100 DB from 0 (max) to 9000 
    1/100 DB (min), and for all values above 9000 the volume will be set to 0 
    linear or 90 DB. 

    Note: This API can be used only for an output port carrying PCM data.
    
Returns:
	BERR_SUCCESS if successful else error value.            
	
See Also:
    BRAP_GetOutputVolume().
**************************************************************************/
BERR_Code BRAP_SetOutputVolume ( 
    BRAP_Handle     hRap,       /* [in] Audio Device Handle */
    BRAP_OutputPort eOpType,    /* [in] Output Port */
    unsigned int    uiLVolume,  /* [in] Left channel volume in 1/100 DB */
    unsigned int    uiRVolume   /* [in] Right channel volume in 1/100 DB*/
    );

/***************************************************************************
Summary:
    Gets the output volume for left and right channels

Description:
    This function returns left and right volume channel settings for an output.
    Currently, volume control is done at the mixer feeding data to an output
    port, so the associated mixer's output volume level is returned by this 
    API. Values are returned in 1/100 DB. 

    Note: This API can be used only for an output port carrying PCM data.

Returns:
   	BERR_SUCCESS if successful else error value.            

See Also:
    BRAP_SetOutputVolume().
**************************************************************************/
BERR_Code BRAP_GetOutputVolume( 
    BRAP_Handle     hRap,       /* [in] Audio Device Handle */
    BRAP_OutputPort eOpType,    /* [in] Output Type */
    unsigned int *  pLVolume,   /* [out] Pointer to memory where left channel 
                                   volume should be stored. Units: 1/100 DB */
    unsigned int *  pRVolume    /* [out] Pointer to memory where right channel 
                                   volume should be stored. Units: 1/100 DB */
    );

/***************************************************************************
Summary:
    Mute/UnMute data at the output port. 

Description:
    This API mutes/unmutes an output port irrespective of the data type is
    compressed or PCM.

Returns:
	BERR_SUCCESS if successful else error value.            
	
See Also:
    BRAP_GetOutputMute().
**************************************************************************/
BERR_Code BRAP_SetOutputMute ( 
    BRAP_Handle     hRap,    /* [in] Audio Device Handle */
    BRAP_OutputPort eOpType, /* [in] Output Type */
    bool            bMute    /* [in] TRUE: Mute output
                                     FALSE: UnMute output */
    );

/***************************************************************************
Summary:
    Gets whether an output is muted or not.

Description:
    This API returns the mute status for an output port.

Returns:
	BERR_SUCCESS if successful else error value.            
	
See Also:
    BRAP_SetOutputMute().    
**************************************************************************/
BERR_Code BRAP_GetOutputMute ( 
    BRAP_Handle     hRap,    /* [in] Audio Device Handle */
    BRAP_OutputPort eOpType, /* [in] Output Type */
    bool            *pMute   /* [out] Pointer to memory where the Mute 
                                status is to be written */            
    );

/***************************************************************************
Summary:
    Sets the coefficients for controlling the Scaling of the input audio stream.

Description:
    This function programs the mixer gain with the provided value.
	
Returns:
	BERR_SUCCESS if successful else error value.            
	
See Also:
    BRAP_GetGain().
**************************************************************************/
BERR_Code BRAP_SetGain(
    BRAP_ChannelHandle  hRapCh,     /* [in] Rap channel Handle */
    BRAP_GainControl    *psGain     /* [in] Gain coefficients Structure */
    );

/***************************************************************************
Summary:
    Gets the coefficients for controlling the Scaling and Panning of the 
    Input audio stream.

Description:
    This function gets the mixer coefficients value.

Returns:
	BERR_SUCCESS if successful else error value.            
	
See Also:
    BRAP_SetGain().
**************************************************************************/
BERR_Code BRAP_GetGain(
    BRAP_ChannelHandle  hRapCh,     /* [in] Rap channel Handle */
    BRAP_GainControl    *psGain     /* [out] Gain coefficients Structure */
    );

/***************************************************************************
Summary:	
    Gets the associated ring buffers's information.
Description:
	This API returns the associated ring buffers's information.
Returns:
	BERR_SUCCESS on success, error on failure
See Also:
    BRAP_PB_GetBufInfo_isr()
****************************************************************************/
BERR_Code BRAP_PB_GetBufInfo(
	BRAP_ChannelHandle 	hRapCh,		   /* [in] Audio channel handle */
	BRAP_PcmBufInfo     *psPcmBufInfo  /* [out] Associated Pcm buffer 
													 information */		
	);

/***************************************************************************
Summary:	
    Gets the associated ring buffers's information.
Description:
	This API returns the associated ring buffers's information. This is an 
	ISR version of BRAP_PB_GetBufInfo(). If ring buffer is written with PCM
    data from an ISR context, this API should be used to get the rbuf info.
Returns:
	BERR_SUCCESS on success, error on failure
See Also:
    BRAP_PB_GetBufInfo()
****************************************************************************/
BERR_Code BRAP_PB_GetBufInfo_isr(
	BRAP_ChannelHandle 	hRapCh,		   /* [in] Audio channel handle */
	BRAP_PcmBufInfo     *psPcmBufInfo  /* [out] Associated Pcm buffer 
													 information */		
	);

/***************************************************************************
Summary:	
    Updates the write pointers for the associated ring buffers as 
    per the ring buffer usage provided.
    
Description:
	This API updates the write pointer associated to the ring-buffer as per 
	the information related to the data rwritten to the ring buffer, which 
	is provided by the caller through the structure BRAP_PcmBufSzUpdt.

	Note: BRAP_PB_BufferWriteDone() must be called after this PI to ensure that 
	the interrupt triggers. BRAP_PB_BufferWriteDone() can be called once after 
	multiple calls to BRAP_PB_UpdateBufUsg()
	
Returns:
	BERR_SUCCESS on success, error on failure
	
See Also:
	BRAP_PB_UpdateBufUsg_isr(), BRAP_PB_BufferWriteDone()
****************************************************************************/
BERR_Code BRAP_PB_UpdateBufUsg( 
	BRAP_ChannelHandle 	hRapCh,			/* [in] Audio channel handle */
    const BRAP_PcmBufSzUpdt	*psPcmBufSzUpdt	/* [in] Ring buffer usage 
    											   information */
    );

/***************************************************************************
Summary:	
    Updates the write pointers for the associated ring buffers as 
    per the ring buffer usage provided.
    
Description:
	This API updates the write pointer associated to the ring-buffer as per 
	the information related to the data rwritten to the ring buffer, which 
	is provided by the caller through the structure BRAP_PcmBufSzUpdt.
	This is an ISR version of BRAP_PB_UpdateBufUsg(). If ring buffer is 
	written with PCM data from an ISR context, this API should be used to 
	update the ring buffer usage info.
	
	Note: BRAP_PB_BufferWriteDone_isr() must be called after this PI to ensure that 
	the interrupt triggers. BRAP_PB_BufferWriteDone_isr() can be called once after 
	multiple calls to BRAP_PB_UpdateBufUsg_isr()
	
Returns:
	BERR_SUCCESS on success, error on failure
	
See Also:
	BRAP_PB_UpdateBufUsg(), BRAP_PB_BufferWriteDone_isr()
****************************************************************************/
BERR_Code BRAP_PB_UpdateBufUsg_isr( 
	BRAP_ChannelHandle 	hRapCh,			/* [in] Audio channel handle */
    const BRAP_PcmBufSzUpdt	*psPcmBufSzUpdt	/* [in] Ring buffer usage 
    											   information */
    );

/***************************************************************************
Summary:	
	Informs the PI about the completion of pumping of data to ring buffers.
	
Description:
	This API is required to be called by the application after it has completed
	writing PCM data to ring buffers. This call is mandatory to ensure that
	it gets further interrupts in future.

	Note: BRAP_PB_BufferWriteDone() must be called after BRAP_PB_UpdateBufUsg()
	to ensure that the interrupt triggers. BRAP_PB_BufferWriteDone() can be 
	called once after multiple calls to BRAP_PB_UpdateBufUsg()
	
Returns:
    Returns error on failure or BERR_SUCCESS on success
    
See Also:
	BRAP_PB_UpdateBufUsg(), BRAP_PB_BufferWriteDone_isr()
****************************************************************************/
BERR_Code BRAP_PB_BufferWriteDone(
	BRAP_ChannelHandle 	hRapCh		   /* [in] Audio channel handle */
	);

/***************************************************************************
Summary:	
	Informs the PI about the completion of pumping of data to ring buffers.
	
Description:
	This API is required to be called by the application after it has completed
	writing of PCM data to ring buffers. This call is mandatory to ensure that
	it gets further interrupts in future. This API is used within the ISR context
	
	Note: BRAP_PB_BufferWriteDone_isr() must be called after 
	BRAP_PB_UpdateBufUsg_isr() to ensure that the interrupt triggers. 
	BRAP_PB_BufferWriteDone_isr() can be called once after multiple 
	calls to BRAP_PB_UpdateBufUsg()	
	
Returns:
    Returns error on failure or BERR_SUCCESS on success
    
See Also:
	BRAP_PB_BufferWriteDone(), BRAP_PB_UpdateBufUsg_isr()
****************************************************************************/
BERR_Code BRAP_PB_BufferWriteDone_isr(
	BRAP_ChannelHandle 	hRapCh		   /* [in] Audio channel handle */
	);

/***************************************************************************
Summary:   
   Updates the read pointers for the destination ring buffers as per the 
   ring buffer usage provided.
Description:
   After reading PCM data from the ring buffer, an application can use this 
   routine to update the read pointer associated to that ring buffer. 
Returns:
   BERR_SUCCESS
See Also:
   BRAP_GetRingBufInfo
****************************************************************************/
BERR_Code BRAP_UpdateRingBufUsg( 
    BRAP_DestinationHandle          hDstHandle, /* [in] Destination Handle */
    const BRAP_PcmBufSzUpdt         *psPcmBufSzUpdt,  /* [in] Ring buffer read 
                                                         information */
    BRAP_OutputChannelPair          eChPair
);


/***************************************************************************
Summary:   
   Updates the read pointers for the destination ring buffers as per the 
   ring buffer usage provided.
Description:
   After reading PCM data from the ring buffer, an application can use this 
   routine to update the read pointer associated to that ring buffer. 
   This is an ISR version of BRAP_UpdateRingBufUsg().
Returns:
   BERR_SUCCESS
See Also:
   BRAP_GetRingBufInfo
****************************************************************************/
BERR_Code BRAP_UpdateRingBufUsg_isr( 
    BRAP_DestinationHandle          hDstHandle, /* [in] Destination Handle */
    const BRAP_PcmBufSzUpdt         *psPcmBufSzUpdt,   /* [in] Ring buffer usage 
                                                          information */
    BRAP_OutputChannelPair          eChPair
);


/***************************************************************************
Summary:   
   Gets the destination ring buffers's information.
Description:
   This API returns the destination ring buffers's information.
   Note: BRAP_PcmBufInfo has left and right ring buffer info structure. Each 
      info structure has both pWritePtr and pReadPtr elements. This routine 
      returns pWritePtr = NULL and pReadPtr = valid address (if any) as an
      application requires only a readPtr to copy captured PCM data   from 
      ring buffer to some local memory.
Returns:
   BERR_SUCCESS
See Also:
   BRAP_UpdateRingBufUsg
****************************************************************************/
BERR_Code BRAP_GetRingBufInfo(
    BRAP_DestinationHandle          hDstHandle,      /* [in] Destination Handle */
    BRAP_PcmBufInfo                 *psPcmBufInfo,  /* [Out] Associated Pcm buffer 
                                                         information */
    BRAP_OutputChannelPair          eChPair
);

/***************************************************************************
Summary:   Gets the destination ring buffers's information.
Description:
   This API returns the destination ring buffers's information. This is an 
   ISR version of BRAP_GetRingBufInfo(). 
Returns:
   BERR_SUCCESS
See Also:
    BRAP_UpdateRingBufUsg_isr
****************************************************************************/
BERR_Code BRAP_GetRingBufInfo_isr(
    BRAP_DestinationHandle          hDstHandle,     /* [in] Destination Handle */
    BRAP_PcmBufInfo                 *psPcmBufInfo,  /* [Out] Associated Pcm buffer 
                                                         information */
    BRAP_OutputChannelPair          eChPair
);


/***************************************************************************
Summary:   
   Informs the PI about the completion of reading of data from ring buffers.
Description:
   This API is required to be called by the application after it has completed
   reading of PCM data from ring buffers. This call is mandatory to ensure that
   it gets further interrupts in future.
Returns:
    Returns error on failure or BERR_SUCCESS on success
See Also:
   BRAP_RingBufferReadDone_isr
****************************************************************************/
BERR_Code BRAP_RingBufferReadDone(
   BRAP_DestinationHandle          hDstHandle /* [in] Destination Handle */
);

/***************************************************************************
Summary:   
   Informs the PI about the completion of reading of data from ring buffers.
Description:
   This API is required to be called by the application after it has completed
   reading of PCM data from ring buffers. This call is mandatory to ensure that
   it gets further interrupts in future. This API is used within the ISR context
Returns:
    Returns error on failure or BERR_SUCCESS on success
See Also:
   BRAP_RingBufferReadDone
****************************************************************************/
BERR_Code BRAP_RingBufferReadDone_isr(
   BRAP_DestinationHandle          hDstHandle /* [in] Destination Handle */
);


/***************************************************************************
Summary:   
   Flushes the Ring Buffer.
Description:
   This API is required to be called by the application after it has received
   an Overflow interrupt. This call makes the Read pointer equal the write pointer
   and hence flushes the data out
Returns:
    Returns error on failure or BERR_SUCCESS on success
See Also:
   BRAP_FlushRingBuffer_isr
****************************************************************************/
BERR_Code BRAP_FlushRingBuffer(
   BRAP_DestinationHandle          hDstHandle /* [in] Destination Handle */
);

/***************************************************************************
Summary:   
   Flushes the Ring Buffer - ISR version
Description:
   This API is required to be called by the application after it has received
   an Overflow interrupt. This call makes the Read pointer equal the write pointer
   and hence flushes the data out
Returns:
    Returns error on failure or BERR_SUCCESS on success
See Also:
   BRAP_FlushRingBuffer
****************************************************************************/
BERR_Code BRAP_FlushRingBuffer_isr(
   BRAP_DestinationHandle          hDstHandle /* [in] Destination Handle */
);


/***************************************************************************
Summary:
	API returns the buffer requirements for CDB and ITB.

Description:
	This API returns the size required for the CDB and ITB buffers
	based upon the algorithm type to be used.
	
Returns:
	BERR_SUCCESS 

See Also:
	None
****************************************************************************/
BERR_Code BRAP_GetBufferConfig ( 	
	BRAP_Handle 		hRap,				/* [in] The RAP device handle */
	BRAP_BufferCfgMode	eCfgMode,			/* [in] CDB/ITB Configuration Mode */
	BAVC_CdbItbConfig	*pCdbItbConfigInfo	/* [out] Buffer information of CDB and ITB*/ 
	);
#if (BRAP_7550_FAMILY !=1)
/***************************************************************************
Summary:
	Sets the config for input ports e.g. I2s In.
	
Description:
    
Returns:

See Also:
	BRAP_GetDefaultInputConfig
	BRAP_GetCurrentInputConfig
****************************************************************************/
BERR_Code BRAP_SetInputConfig (
	BRAP_Handle		            hRap,		    /* [in] RAP device handle */
	const BRAP_InputPortConfig	*pInConfig	    /* [in] Input port 
	                                               configuration */
	);

/***************************************************************************
Summary:
	Get default Input port configuration

Description:
	This function gets the default Input port configuration 
	
Returns:
	BERR_SUCCESS if successful else error value

See Also:
	BRAP_SetInputConfig
	BRAP_GetCurrentInputConfig
****************************************************************************/
BERR_Code BRAP_GetDefaultInputConfig (
	BRAP_Handle	            hRap,	        /* [in] RAP device handle */
	BRAP_CapInputPort       eCapPort,       /* [in] Input port */
	BRAP_InputPortConfig	*pInConfig      /* [out] Default configuration for
	                                           the Input port */
	);

/***************************************************************************
Summary:
	Get current Input port configuration

Description:
	This function gets the current Input port configuration. If the caller has 
	not configured the output port before, this API will return error. 
 	
Returns:
	BERR_SUCCESS if successful else error value

See Also:
	BRAP_SetInputConfig
	BRAP_GetDefaultinputConfig
****************************************************************************/
BERR_Code BRAP_GetCurrentInputConfig (
	BRAP_Handle	            hRap,	        /* [in] RAP device handle */
	BRAP_CapInputPort       eCapPort,       /* [in] Input port */
	BRAP_InputPortConfig	*pInConfig      /* [out] Current configuration for
	                                           the input port */
    );
#endif
/*****************************************************************************
Summary:
    Used to install an application callback.

Description:
    This PI should be used by the application to install an application 
    specified callback in addition to the RAptor PI interrupt handler for that 
    interrupt. The speficied Application callback gets called from the interrupt
    handler that the Raptor PI already has installed. 
    This can be done only for the interrupts that are handled by Raptor PI and
    listed in BRAP_Interrupt. For all other interrupts, application has to use 
    the standard INT interface.
    
Returns:
    BERR_SUCCESS on success
    Error code on failure
******************************************************************************/
BERR_Code BRAP_InstallAppInterruptCb (
	BRAP_ChannelHandle hRapCh,	   /* [in] The RAP channel handle */
    BRAP_Interrupt     eInterrupt, /* [in] The interrupt that needs to 
                                      be activated */
    BRAP_CallbackFunc  pfAppInterruptCb, /* [in] The RAP callback function 
                                      to be registered for eInterrupt*/ 
    void               *pParm1,    /* [in] The application specified parameter 
                                      that needs to be passed unchanged to the 
                                      callback */
    int                iParm2      /* [in] The application specified parameter 
                                      that needs to be passed unchanged to the 
                                      callback */
    );

/*****************************************************************************
Summary:
    Used to remove the application callback.

Description:
    This PI should be used by the application to remove an application specified 
    callback that has been installed in addition to the Raptor PI interrupt handler 
    for that interrupt. 

Returns:
    BERR_SUCCESS on success
    Error code on failure
******************************************************************************/
BERR_Code BRAP_RemoveAppInterruptCb (
	BRAP_ChannelHandle hRapCh,	/* [in] The RAP channel handle */
    BRAP_Interrupt  eInterrupt  /* [in] The interrupt that needs to be deactivated */
    );

/*****************************************************************************
Summary:
    Used to install an application callback.

Description:
    This PI should be used by the application to install an application 
    specified callback in addition to the RAptor PI interrupt handler for that 
    interrupt. The speficied Application callback gets called from the interrupt
    handler that the Raptor PI already has installed. 
    This can be done only for the interrupts that are handled by Raptor PI and
    listed in BRAP_Interrupt. For all other interrupts, application has to use 
    the standard INT interface.
    
Returns:
    BERR_SUCCESS on success
    Error code on failure
******************************************************************************/
BERR_Code BRAP_InstallDestinationLevelAppInterruptCb (
	BRAP_DestinationHandle        hDstHandle,	   /* [in] The RAP destination handle */
    BRAP_DestinationInterrupt     eInterrupt, /* [in] The interrupt that needs to 
                                      be activated */
    BRAP_CallbackFunc             pfAppInterruptCb, /* [in] The RAP callback function 
                                      to be registered for eInterrupt*/ 
    void                         *pParm1,    /* [in] The application specified parameter 
                                      that needs to be passed unchanged to the 
                                      callback */
    int                           iParm2      /* [in] The application specified parameter 
                                      that needs to be passed unchanged to the 
                                      callback */
);

/*****************************************************************************
Summary:
    Used to remove the application callback.

Description:
    This PI should be used by the application to remove an application specified 
    callback that has been install in addition to the Raptor PI interrupt handler 
    for that interrupt. 

Returns:
    BERR_SUCCESS on success
    Error code on failure
******************************************************************************/
BERR_Code BRAP_RemoveDestinationLevelAppInterruptCb (
   BRAP_DestinationHandle          hDstHandle, /* [in] Destination Handle */
   BRAP_DestinationInterrupt       eInterrupt  /* [in] The destination level interrupt 
            that needs to be removed */
    );

/*****************************************************************************
Summary:
	Install application callback function for device level interrupts
	
Description:
	This API is used to install application callback function for device
	level Raptor interrupts. Application callback function is called when
	these interrupts are triggered.

Returns:
    BERR_SUCCESS on success
     Error code on failure
******************************************************************************/
BERR_Code BRAP_InstallDeviceLevelAppInterruptCb (
	BRAP_Handle hRap,	   /* [in] The RAP handle */
    BRAP_DeviceLevelInterrupt     eInterrupt, /* [in] The device level interrupt
										that needs to be activated */
    BRAP_CallbackFunc  pfAppInterruptCb, /* [in] The RAP callback function 
                                      to be registered for eInterrupt*/ 
    void               *pParm1,    /* [in] The application specified parameter 
                                      that needs to be passed unchanged to the 
                                      callback */
    int                iParm2      /* [in] The application specified parameter 
                                      that needs to be passed unchanged to the 
                                      callback */
    );

/*****************************************************************************
Summary:
	Remove application callback function for device level interrupts

Description:
	This API is used to remove application callback function for device
	level Raptor interrupts. 

Returns:
    BERR_SUCCESS on success
     Error code on failure
******************************************************************************/
BERR_Code BRAP_RemoveDeviceLevelAppInterruptCb (
	BRAP_Handle hRap,	/* [in] The RAP channel handle */
    BRAP_DeviceLevelInterrupt  eInterrupt  /* [in] The device level interrupt 
								that needs to be removed */
    );

/***************************************************************************
Summary:
	This API is used to inform the decoder to stop reading data from RAVE in
	preparation for flush. This API should be called before calling 
	BRAP_FlushChannel().
	
Description:
	This API just stops the decoder channel from reading data from CDB/ITB.
	This API should be called before calling BRAP_FlushChannel() PI. This API is
	valid only for a decode channel.

Returns:
	BERR_SUCCESS on success
	Error value on failure

See Also:
    BRAP_FlushChannel
****************************************************************************/
BERR_Code  BRAP_DisableForFlush ( 
	BRAP_ChannelHandle	hRapCh		/* [in] The RAP Decode Channel handle */
	);

/***************************************************************************
Summary:
	This API flushes CDB/ITB buffers associated with a decode channel for 7411 only.
	And for other chips this fucntion clears the itnernal buffers owned by PI and FW.

Description:
	Along with CDB/ITB buffers(7411), various buffers present in the decode pipeline also get flushed.
	BRAP_DisableForFlush() API should be called before calling BRAP_FlushChannel() API.

Returns:
	BERR_SUCCESS on success
	Error value on failure

See Also:
    BRAP_DisableForFlush
****************************************************************************/
BERR_Code BRAP_FlushChannel (
	BRAP_ChannelHandle	        hRapCh		/* [in] The Decode Channel handle */
	);

/***************************************************************************
Summary:
	Sets the Static Downmixing coefficients to be used in the system.
	
Description:
    
Returns:

See Also:
    BRAP_DownMixingCoef
****************************************************************************/
BERR_Code BRAP_SetDownMixCoef(
    BRAP_Handle            		hRap,
    BRAP_DownMixingCoef        	*pDnMixingCoeff
    );


/**********************************************************************
Summary:
    Sets various audio post processing stages

Description:
    This function sets the desired audio post processing stages on a given destination. 

    A post processing stage is one that operates on output of a decoder or encoder.
    For PCM playback and capture channels, post processing stages operate on
    PCM samples played from memory or captured from hardware input.
    
    A destination can take audio data from a channel or mixed output of multiple
    channels. If a destination is receiving data from multiple channels after mixing,
    audio post processing stages can be added to specific channel or to the mixed output
    of multiple channels. If audio processing stages are getting added to a specifc
    channel, then this function should be called only when that channel is in "stop"
    state. If audio processing stages are getting added on the mixed output of the
    channels then this function should be called only when all the channels are in
    "stop" stage.

    Usage Examples: 
    Example 1: Consider following two data flows in a single RAP decode channel
    ----------

    Decode -> PL-II -> SRS-XT -> AVL -> DAC
                              -> BBE -> I2S
                
    Here PL-II and SRS-XT post processing stages are common for both DAC
    and I2S destinations. Hence for DAC we need to add post processing stages
    PL-II, SRS-XT and AVL. For I2S we need to add post processing stages
    PL-II, SRS-XT and BBE.
    
    BRAP_ProcessingSettings        sPostProcessingSettings;
    BRAP_ProcessingStageHandle     hPl2, hSrsXt, hAvl, hBbe;
    BRAP_ProcessingStageSettings   sProcessingStageSettings;
    BRAP_ProcessingAlgorithm       sProcessingAlgorithm;
    BERR_Code err;

    sProcessingAlgorithm.eAudioProcessing = BRAP_ProcessingType_ePl2;
    err = BRAP_GetDefaultProcessingStageSettings( &sProcessingAlgorithm,
            &sProcessingStageSettings );
    err = BRAP_CreateProcessingStage( &sProcessingStageSettings,
            &hPl2 );

    sProcessingAlgorithm.eAudioProcessing = BRAP_ProcessingType_eSrsXt;
    err = BRAP_GetDefaultProcessingStageSettings( &sProcessingAlgorithm,
            &sProcessingStageSettings );
    err = BRAP_CreateProcessingStage( &sProcessingStageSettings,
            &hSrsXt );
            
    sProcessingAlgorithm.eAudioProcessing = BRAP_ProcessingType_eAvl;
    err = BRAP_GetDefaultProcessingStageSettings( &sProcessingAlgorithm,
            &sProcessingStageSettings );
    err = BRAP_CreateProcessingStage( &sProcessingStageSettings,
            &hAvl );

    sProcessingAlgorithm.eAudioProcessing = BRAP_ProcessingType_eBbe;
    err = BRAP_GetDefaultProcessingStageSettings( &sProcessingAlgorithm,
            &sProcessingStageSettings );
    err = BRAP_CreateProcessingStage( &sProcessingStageSettings,
            &hBbe );
 
    err = BRAP_GetDefaultPostProcessingStages( &sPostProcessingSettings );
    sPostProcessingSettings.hAudProcessing[0] = hPl2;
    sPostProcessingSettings.hAudProcessing[1] = hSrsXt;
    sPostProcessingSettings.hAudProcessing[2] = hAvl;

    err = BRAP_SetPostProcessingStages( hDac, hDecCh, &sPostProcessingSettings );

    err = BRAP_GetDefaultPostProcessingStages( &sPostProcessingSettings );
    sPostProcessingSettings.hAudProcessing[0] = hPl2;
    sPostProcessingSettings.hAudProcessing[1] = hSrsXt;
    sPostProcessingSettings.hAudProcessing[2] = hBbe;

    err = BRAP_SetPostProcessingStages( hI2s, hDecCh, &sPostProcessingSettings );

    Example 2: Seperate post processing stages for two channels getting mixed.
    ----------
    Consider following data flows for two channels

    Decode -> PL-II -> SRS-XT -> DAC
    Capture -> XEN -> BBE -> DAC

    Decode and capture channels are getting mixed and going to DAC output port.
    Before mixing, decode channel has PL-II, SRS-XT post processing stages and
    capture channel has XEN, BBE processing stages.
     
    BRAP_ProcessingSettings        sPostProcessingSettings;
    BRAP_ProcessingStageHandle     hPl2, hSrsXt, hXen, hBbe;
    BRAP_ProcessingStageSettings   sProcessingStageSettings;
    BRAP_ProcessingAlgorithm       sProcessingAlgorithm;
    BERR_Code err;

    sProcessingAlgorithm.eAudioProcessing = BRAP_ProcessingType_ePl2;
    err = BRAP_GetDefaultProcessingStageSettings( &sProcessingAlgorithm,
            &sProcessingStageSettings );
    err = BRAP_CreateProcessingStage( &sProcessingStageSettings,
            &hPl2 );
            
    sProcessingAlgorithm.eAudioProcessing = BRAP_ProcessingType_eSrsXt;
    err = BRAP_GetDefaultProcessingStageSettings( &sProcessingAlgorithm,
            &sProcessingStageSettings );
    err = BRAP_CreateProcessingStage( &sProcessingStageSettings,
            &hSrsXt );
            
    sProcessingAlgorithm.eAudioProcessing = BRAP_ProcessingType_eXen;
    err = BRAP_GetDefaultProcessingStageSettings( &sProcessingAlgorithm,
            &sProcessingStageSettings );
    err = BRAP_CreateProcessingStage( &sProcessingStageSettings,
            &hXen );
               
    sProcessingAlgorithm.eAudioProcessing = BRAP_ProcessingType_eBbe;
    err = BRAP_GetDefaultProcessingStageSettings( &sProcessingAlgorithm,
            &sProcessingStageSettings );
    err = BRAP_CreateProcessingStage( &sProcessingStageSettings,
            &hBbe );

    err = BRAP_GetDefaultPostProcessingStages( &sPostProcessingSettings );
    sPostProcessingSettings.hAudProcessing[0] = hPl2;
    sPostProcessingSettings.hAudProcessing[1] = hSrsXt;

    err = BRAP_SetPostProcessingStages( hDac, hDecCh, &sPostProcessingSettings );

    err = BRAP_GetDefaultPostProcessingStages( &sPostProcessingSettings );
    sPostProcessingSettings.hAudProcessing[0] = hXen;
    sPostProcessingSettings.hAudProcessing[2] = hBbe;

    err = BRAP_SetPostProcessingStages( hI2s, hCapCh, &sPostProcessingSettings );

    Example 3: Common post processing stages for two channels getting mixed
    ----------

    (Decode + Capture ) -> PL-II -> SRS-XT -> DAC 

    BRAP_ProcessingSettings            sPostProcessingSettings;
    BRAP_ProcessingStageHandle         hPl2, hSrsXt;
    BRAP_ProcessingStageSettings       sProcessingStageSettings;
    BRAP_ProcessingAlgorithm           sProcessingAlgorithm;
    BERR_Code err;

    sProcessingAlgorithm.eAudioProcessing = BRAP_ProcessingType_ePl2;
    err = BRAP_GetDefaultProcessingStageSettings( &sProcessingAlgorithm,
            &sProcessingStageSettings );
    err = BRAP_CreateProcessingStage( &sProcessingStageSettings,
            &hPl2 );
            
    sProcessingAlgorithm.eAudioProcessing = BRAP_ProcessingType_eSrsXt;
    err = BRAP_GetDefaultProcessingStageSettings( &sProcessingAlgorithm,
            &sProcessingStageSettings );
    err = BRAP_CreateProcessingStage( &sProcessingStageSettings,
            &hSrsXt );

    err = BRAP_GetDefaultPostProcessingStages( &sPostProcessingSettings );
    sPostProcessingSettings.hAudProcessing[0] = hPl2;
    sPostProcessingSettings.hAudProcessing[1] = hSrsXt;

    err = BRAP_SetPostProcessingStages( hDac, NULL, &sPostProcessingSettings );

    Example 4: Decode with post processing and passthru in same decode channel
    ----------

    Decode -> PL-II -> SRS-XT -> DAC
           -> Convert -> SPDIF

    BRAP_ProcessingSettings            sPostProcessingSettings;
    BRAP_ProcessingStageHandle         hPl2, hSrsXt, hConvert;
    BRAP_ProcessingStageSettings       sProcessingStageSettings;
    BRAP_ProcessingAlgorithm           sProcessingAlgorithm;
    BERR_Code err;
    
    sProcessingAlgorithm.eAudioProcessing = BRAP_ProcessingType_ePl2;
    err = BRAP_GetDefaultProcessingStageSettings( &sProcessingAlgorithm,
            &sProcessingStageSettings );
    err = BRAP_CreateProcessingStage( &sProcessingStageSettings,
            &hPl2 );
            
    sProcessingAlgorithm.eAudioProcessing = BRAP_ProcessingType_eSrsXt;
    err = BRAP_GetDefaultProcessingStageSettings( &sProcessingAlgorithm,
            &sProcessingStageSettings );
    err = BRAP_CreateProcessingStage( &sProcessingStageSettings,
            &hSrsXt );

    sProcessingAlgorithm.eAudioProcessing = BRAP_ProcessingType_eConvert;
    sProcessingAlgorithm.uAlgorithm.eConverterType = BRAP_ConverterType_ePassthru;
    err = BRAP_GetDefaultProcessingStageSettings( &sProcessingAlgorithm,
            &sProcessingStageSettings );
    err = BRAP_CreateProcessingStage( &sProcessingStageSettings,
            &hConvert );


    err = BRAP_GetDefaultPostProcessingStages( &sPostProcessingSettings );
    sPostProcessingSettings.hAudProcessing[0] = hPl2;
    sPostProcessingSettings.hAudProcessing[1] = hSrsXt;

    err = BRAP_SetPostProcessingStages( hDac, hDecCh, &sPostProcessingSettings );
    
    err = BRAP_GetDefaultPostProcessingStages( &sPostProcessingSettings );
    sPostProcessingSettings.hAudProcessing[0] = hConvert;

    err = BRAP_SetPostProcessingStages( hSpdif, hDecCh, &sPostProcessingSettings );

    Example 5: Generic Transcode case
    ----------
    Decode -> DAC
           -> encode -> SPDIF
    Decoded PCM samples go to DAC whereas transcoded data goes to SPDIF
    
    BRAP_ProcessingSettings            sPostProcessingSettings;
    BRAP_ProcessingStageHandle         hEncode;
    BRAP_ProcessingStageSettings       sProcessingStageSettings;
    BRAP_ProcessingAlgorithm           sProcessingAlgorithm;
    BERR_Code err;
    
    sProcessingAlgorithm.eAudioProcessing = BRAP_ProcessingType_eEncode;
    sProcessingAlgorithm.uAlgorithm.eEncodeType = BRAP_EncodeType_eDts;
    err = BRAP_GetDefaultProcessingStageSettings( &sProcessingAlgorithm,
            &sProcessingStageSettings );
    err = BRAP_CreateProcessingStage( &sProcessingStageSettings,
            &hEncode );

    err = BRAP_GetDefaultPostProcessingStages( &sPostProcessingSettings );
    sPostProcessingSettings.hAudProcessing[0] = hEncode;

    err = BRAP_SetPostProcessingStages( hSpdif, hDecCh, &sPostProcessingSettings );
    
Returns:
    BERR_SUCCESS - If function is successful.
    BERR_INVALID_PARAMETER - On bad input parameters.
    BERR_NOT_SUPPORTED - If this function is called when channel is in
	"start" state.

See Also:
    BRAP_AddDestination
    BRAP_GetAudioProcessingStages
***********************************************************************/
BERR_Code BRAP_SetPostProcessingStages (
    BRAP_DestinationHandle          hDestHandle, /* [in] Destination handle */
    BRAP_ChannelHandle              hRapCh,    /* [in] If audio processing branch is specific to 
                                                  one of the channels feeding data to this 
                                                  destination, handle of that channel.
                                                  If set to NULL, audio processing branch
                                                  becomes common ( starts after mixing
                                                  of channels ) to all the channels
                                                  feeding data to this destination. */
    const BRAP_ProcessingSettings    *psStgSettings  
                                               /* [in] Settings for all the stages in 
                                                  the branch */
);

/**********************************************************************
Summary:
    Gets audio post processing stages added for a destination

Description:
    This function returns the post processing stages added for a destination by API
    BRAP_SetPostProcessingStages().
    
Returns:
    BERR_SUCCESS - If function is successful.
    BERR_INVALID_PARAMETER - On bad input parameters.
See Also:
    BRAP_SetPostProcessingStages
***********************************************************************/
BERR_Code BRAP_GetCurrentPostProcessingStages (
    BRAP_DestinationHandle          hDestHandle,
                                               /* [in] Destination handle */    
    BRAP_ChannelHandle              hRapCh,    /* [in] If audio processing branch is specific to 
                                                  one of the channels feeding data to this 
                                                  destination, handle of that channel.
                                                  If set to NULL, audio processing branch
                                                  becomes common ( starts after mixing
                                                  of channels ) to all the channels
                                                  feeding data to this destination. */
    BRAP_ProcessingSettings    *psStgSettings 
	                                           /* [out] Settings for all the stages in 
	                                              the branch */
);

/**********************************************************************
Summary:
    Gets default audio post processing stages added for a destination
    
Description:
    This function initializes fields of BRAP_ProcessingSettings to NULL values. Application
    should populate this structure properly before passing on to API
    BRAP_SetPostProcessingStages().
    
Returns:
    BERR_SUCCESS - If function is successful.
    BERR_INVALID_PARAMETER - On bad input parameters.
See Also:
    BRAP_SetPostProcessingStages
***********************************************************************/
BERR_Code BRAP_GetDefaultPostProcessingStages (
    BRAP_ProcessingSettings    *psStgSettings 
	                                                       /* [out] Settings for all the stages in 
	                                                            the branch */
);

/**********************************************************************
Summary:
    Sets audio pre-processing stages for a given decode/encode channel

Description:
    This function sets the pre-processing stages for a given channel. A pre-processing
    stage is one that preceeds encoder or decoder. This function is supported only for
    encode and decode channels. This function should be called only when channel is
    in "stop" condition.
    
Returns:
    BERR_SUCCESS - If function is successful.
    BERR_INVALID_PARAMETER - On bad input parameters.
    BERR_NOT_SUPPORTED - If channel is not in "stop" condition or if channel is
        different than encode/decode.

see Also:
    BRAP_GetPreProcessingStages
***********************************************************************/
BERR_Code BRAP_SetPreProcessingStages (
    BRAP_ChannelHandle          hRapCh,
                                                                /* [in] Destination handle */    
    const BRAP_ProcessingSettings    *psStgSettings 
	                                                        /* [in] Settings for all the stages in 
	                                                             the branch */
);

/**********************************************************************
Summary:
    Gets current audio pre-processing stages

Description:
    This function returns the pre-processing stages by API
    BRAP_SetPreProcessingStages().
    
Returns:
    BERR_SUCCESS - If function is successful.
    BERR_INVALID_PARAMETER - On bad input parameters.
See Also:
    BRAP_SetPreProcessingStages
***********************************************************************/
BERR_Code BRAP_GetCurrentPreProcessingStages (
    BRAP_ChannelHandle          hRapCh,
                                     /* [in] Destination handle */    
    BRAP_ProcessingSettings    *psStgSettings 
	                                 /* [out] Settings for all the stages in 
	                                    the branch */
);

/**********************************************************************
Summary:
    Gets default audio pre-processing stages

Description:
    This function initializes fields of BRAP_ProcessingSettings to NULL values. Application
    should populate this structure properly before passing on to API
    BRAP_SetPreProcessingStages().
    
Returns:
    BERR_SUCCESS - If function is successful.
    BERR_INVALID_PARAMETER - On bad input parameters.
See Also:
    BRAP_SetPrePostProcessingStages
***********************************************************************/
BERR_Code BRAP_GetDefaultPreProcessingStages (
    BRAP_ProcessingSettings    *psStgSettings 
	                                 /* [out] Settings for all the stages in 
	                                    the branch */
);

/***************************************************************************
Summary: 
	Pause/resume audio

Description: 
	This function pauses/resumes audio. When unpaused (resumed), audio
	starts from the point it paused. This function works only for decode 
	channel in playback mode.

	Note: This API is valid only for a decode channel.

Returns:
	BERR_SUCCESS - If pause/resume operation is successful.

See Also: 
	BRAP_GetPvrPauseStatus
****************************************************************************/
BERR_Code BRAP_EnablePvrPause(
	BRAP_ChannelHandle		hRapCh,     /* [In] The RAP Channel handle */
	bool					bOnOff	    /* [In] true = Pause audio
											    false = Resume audio */
	);

/***************************************************************************
Summary: 
	Get Pause/resume audio status

Description: 
	This function gets the current pause/resume status of the audio. When 
	audio is paused it returns TRUE and when audio is playing it returns 
	FALSE.
	
	Note: This API is valid only for a decode channel.
	
Returns:
	BERR_SUCCESS - If pause/resume information is retrieved successfully.

See Also: 
	BRAP_EnablePvrPause
****************************************************************************/
BERR_Code BRAP_GetPvrPauseStatus(
	BRAP_ChannelHandle		hRapCh,		    /* [In] The RAP Channel handle */
	bool					*pbPauseStatus  /* [Out] true = Audio paused
										             false = Audio not paused */
	);

/***************************************************************************
Summary: 
	Frame advance trick mode

Description: 
	This function programs audio decoder to perform frame advance trick 
	mode. During frame advance trick mode,audio is muted at the output port.
	Parameter uiFrameAdvTime tells audio decoder how many frames to advance
	in terms of time (in millisecond).
	
	Note: This API is valid only for a decode channel running in playback mode.
	
Returns:
	BERR_SUCCESS - If frame advance mode is programmed successfully.

See Also:
****************************************************************************/
BERR_Code BRAP_PvrFrameAdvance (
	BRAP_ChannelHandle		hRapCh,			/* [In] The RAP Channel handle */
	unsigned int			uiFrameAdvTime	/* [In] Frame advance time in msec */
	);

/***************************************************************************
Summary:
	Process Raptor watchdog interrupt

Description:
	This function processes Raptor watchdog interrupt. This is not _isr safe
	function and should be called only from task context. Call to this function
	should be treated as critical section code.

Returns:
	BERR_SUCCESS 

See Also:
	None
****************************************************************************/
BERR_Code BRAP_ProcessWatchdogInterrupt(
     BRAP_Handle 			hRap		/* [in] The RAP device handle */
    );

/***************************************************************************
Summary:
    Sets the ramping step size for scale and volume change for all output ports.

Description:
    This step size represent the volume/scale change per FS. 

    Scale/Volume Ramp step size control is linear in hardware, ranging from 0
    min to 0x8000 max volume.  This API has done a mapping from this linear 
    range to 1/100 DB.  This function gets values in 1/100 DB from 0 (max) to 
    9000 1/100 DB (min), and for all values above 9000 the step size 
    will be set to 0 linear or 90 DB.  

    Please note: Ramp Step size is common to all output ports. It cannot be 
    changed on a per-output-port basis.

    At device open, the Ramp Step is set to a default of 36 dB => transitioning 
    from full volume (ie 0x8000) to muted (ie 0x0) in 64 fs.
    If the user wants to change this default, they need to explcitly call this 
    PI after device open.
    
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_GetRampStepSize().
    
**************************************************************************/

BERR_Code BRAP_SetRampStepSize ( 
    BRAP_Handle     hRap,       /* [in] Audio Device Handle */
    unsigned int    uiRampStep  /* [in] Ramp Step size in 1/100 DB */
);


/**********************************************************************
Summary:
	This function will configure the CRC registers

Description:
	This function will configure the CRC registers and give the crc configuration
	to install the inerrupt for respective output port. 
Returns:
    BERR_SUCCESS - If function is successful.
    BERR_NOT_SUPPORTED - On bad input parameters.
***********************************************************************/
BERR_Code BRAP_ConfigCrc
(
	BRAP_ChannelHandle	hRapCh,	/* [in] The RAP Channel handle */
	uint32_t				uiSample_period, /* [in] The sample period of Crc */
	uint32_t				*pCrc_cfg /*[Out] the crc configuration*/
);

/***************************************************************************
Summary:
    Gets the ramping step size for scale and volume change 

Description:
    This function returns ramping step size for scale and volume change.
    Values are returned in 1/100 DB.
    
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_SetRampStepSize().
    
**************************************************************************/
BERR_Code BRAP_GetRampStepSize( 
    BRAP_Handle     hRap,       /* [in] Audio Device Handle */
    unsigned int *  pRampStepSize /* [out] Pointer to memory where ramp 
    								size should be stored. Units: 1/100 DB */
);

/***************************************************************************
Summary:
    Gets default audio processing stage settings.

Description:
    This function gets the default processing stage settings for a given processing algorithm.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_CreateProcessingStage
    BRAP_DestroyProcessingStage
    BRAP_GetCurrentProcessingStageSettings
    BRAP_SetProcessingStageSettings
    
**************************************************************************/
BERR_Code BRAP_GetDefaultProcessingStageSettings(
    BRAP_ProcessingType                     eProcessingType,
                                                                /* [in] Audio processing
                                                                    algorithm */
    BRAP_ProcessingStageSettings       *psProcessingStageSettings 
                                                                /* [out] Audio Processing Stage
                                                                     settings */
);                                                                                

/***************************************************************************
Summary:
    Gets current audio processing stage settings.

Description:
    This function gets the current processing stage settings for a given processing stage
    handle.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_CreateProcessingStage
    BRAP_DestroyProcessingStage
    BRAP_GetDefaultProcessingStageSettings
    BRAP_SetProcessingStageSettings
    
**************************************************************************/
BERR_Code BRAP_GetCurrentProcessingStageSettings(
    BRAP_ProcessingStageHandle   hAudioProcessingStageHandle,  
                                     /* [in] Handle for the audio processing
                                        stage */
    BRAP_ProcessingStageSettings       *psProcessingStageSettings 
                                     /* [out] Audio Processing Stage
                                        settings */
);                                                                                

/***************************************************************************
Summary:
    Sets audio processing stage settings.

Description:
    This function sets  the processing stage settings for a given processing stage handle.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_CreateProcessingStage
    BRAP_DestroyProcessingStage
    BRAP_GetDefaultProcessingStageSettings
    BRAP_GetCurrentProcessingStageSettings
    
**************************************************************************/
BERR_Code BRAP_SetProcessingStageSettings(
    BRAP_ProcessingStageHandle   hAudioProcessingStageHandle,  
                                     /* [in] Handle for the audio processing
                                        stage */
    const BRAP_ProcessingStageSettings       *psProcessingStageSettings 
                                    /* [in] Audio Processing Stage
                                       settings */
);                                                                                

/***************************************************************************
Summary:
    Creates audio processing stage.

Description:
    This function creates an audio processing stage.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_DestroyProcessingStage
    BRAP_SetProcessingStageSettings
    BRAP_GetDefaultProcessingStageSettings
    BRAP_GetCurrentProcessingStageSettings
    
**************************************************************************/
BERR_Code BRAP_CreateProcessingStage(
    BRAP_Handle                 hRap,   /* [in] The Raptor Audio 
                                                           device handle*/                                                                    
    const BRAP_ProcessingStageSettings    *psProcessingStageSettings,   
                                                                        /* [in] Audio processing to be executed
                                                                             in this stage */
    BRAP_ProcessingStageHandle   *hAudioProcessingStageHandle  
                                                                        /* [out] Handle for the audio processing
                                                                             stage */
);
                                                                               
/***************************************************************************
Summary:
    Destroys audio processing stage.

Description:
    This function destroys an audio processing stage.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_CreateProcessingStage
    BRAP_SetProcessingStageSettings
    BRAP_GetDefaultProcessingStageSettings
    BRAP_GetCurrentProcessingStageSettings
    
**************************************************************************/
BERR_Code BRAP_DestroyProcessingStage(
    BRAP_ProcessingStageHandle   hAudioProcessingStageHandle  
                                  /* [in] Handle for the audio processing
                                     stage */
);


/*}}}*/

/***************************************************************************
Summary:
	This function takes out DSP processor from reset.

Description:
	When firmware authentication is enabled, BRAP_Open keeps DSP processor in 
	reset condition for security software module to authenticate the Raptor 
	firmware. After firmware authentication is done, application needs to call
	this function to take out DSP processor from reset. After BRAP_Open this 
	function should be called first before calling any other RAP PI functions.

Returns:
	BERR_SUCCESS - If unresetting DSP is successful.

See Also:
***************************************************************************/

BERR_Code BRAP_StartDsp(
	BRAP_Handle hRap); 	/* The RAP handle */

/***************************************************************************
Summary:
	Returns physical memory offset and size of firmware executables.
	
Description:
	Returns the physical memory offset where firmware executables of all the 
	supported audio codecs are downloaded in contiguous memory locations. It 
	also returns the total size of the downloaded firmware executables. This 
	function is supported only when firmware authentication is enabled.

Returns:
	BERR_SUCCESS - If valid offset is set.
	BERR_NOT_SUPPORTED - If Firmware authentication is Disabled.

See Also:	
***************************************************************************/
BERR_Code BRAP_GetFwDwnldInfo( 
	BRAP_Handle 		hRap,			/* [in] The Raptor Audio device handle*/
	uint32_t		*ui32FwDwnldOffset,	/* [out] Physical memory offset of 
											downloaded firmware executables  */
	uint32_t		*ui32FwDwnldAddress,	/* [out] Virtual memory offset of 	
											downloaded firmware executables  */											
	uint32_t		*ui32FwDwnldSize	/* [out] Total size of downloaded 
											firmware executables */ 
);

/*}}}*/

/***************************************************************************
Summary:
	Gets the sampling rate and time base associated with the output port.
Description:
	This API is used to get the sampling rate and time base associated with 
	a particular output port.
Returns:
	BERR_SUCCESS 
See Also:
****************************************************************************/
BERR_Code BRAP_GetSamplingRate ( 
	const BRAP_Handle 		hRap, 			/* [in] The RAP Channel handle */
	const BRAP_OutputPort	eOutputPort, 	/* [in] Output port */
	BAVC_AudioSamplingRate  *pSamplingRate, /* [out] Output Sampling rate */
	BAVC_Timebase			*pTimebase 		/* [out] Timebase to use for the output port */
	);

/***************************************************************************
Summary:
    Sets the coefficients for controlling the Scaling of the Input audio stream.
    This is set per channel per output port.

Description:
	
Returns:
	BERR_SUCCESS if successful else error value.            
	
See Also:
    BRAP_GetScaling().
**************************************************************************/
BERR_Code BRAP_SetScaling(
    BRAP_ChannelHandle  hRapCh,     /* [in] Rap channel Handle */
    BRAP_GainControl    *psGain,    /* [in] Gain coefficients Structure */
    BRAP_OutputPort     eOpType     /* [in] Output Port */
);

/***************************************************************************
Summary:
    Gets the coefficients for controlling the Scaling of the Input audio stream.

Description:
    This function gets the mixer coefficients value.
    Note: This API returns only the gain coefficients programmed by the 
    application. 

Returns:
	BERR_SUCCESS if successful else error value.            
	
See Also:
    BRAP_SetScaling().
**************************************************************************/
BERR_Code BRAP_GetScaling(
    BRAP_ChannelHandle  hRapCh,     /* [in] Rap channel Handle */
    BRAP_GainControl    *psGain,    /* [out] Gain coefficients Structure */
    BRAP_OutputPort     eOpType     /* [in] Output Port */
	);


/***************************************************************************
Summary:
    Creates audio descriptor fade table object.
Description:
    This function creates audio descriptor fade table object. For every audio 
    descriptor channel pair, there needs to be one audio descriptor fade table.
Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
    BRAP_DestroyAudioDescriptorFadeTable
    BRAP_SetAudioDescriptorFadeTable
**************************************************************************/
BERR_Code BRAP_CreateAudioDescriptorFadeTable(
    BRAP_Handle hRap, 
    BRAP_AudioDescriptorFadeHandle *phAdFade
    );

/***************************************************************************
Summary:
    Destroys audio descriptor fade table object.
Description:
    This function destroys audio descriptor fade table object. 
Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
    BRAP_CreateAudioDescriptorFadeTable
    BRAP_SetAudioDescriptorFadeTable
**************************************************************************/
BERR_Code BRAP_DestroyAudioDescriptorFadeTable(
    BRAP_AudioDescriptorFadeHandle hAdFade
    );

/***************************************************************************
Summary:
    Sets the audio descriptor fade table object.
Description:
    This function sets the audio descriptor fade table object. For a given audio
    descriptor feature, this function identifies which RAP channel is a primary channel
    and which RAP channel is audio descriptor channel.
Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
    BRAP_GetAudioDescriptorFadeTable
    BRAP_CreateAudioDescriptorFadeTable
    BRAP_DestroyAudioDescriptorFadeTable
**************************************************************************/
BERR_Code BRAP_SetAudioDescriptorFadeTable(
    BRAP_AudioDescriptorFadeHandle hAdFade,     /* [in] Audio Desc handle */ 
    BRAP_AudioDescriptorFadeTableParams *psParams /* [in] Audio Desc Params */
    );

/***************************************************************************
Summary:
    Gets the audio descriptor fade table object.
Description:
    This function gets the audio descriptor fade table object. For a given audio
    descriptor feature, this function provides which RAP channel is a primary channel
    and which RAP channel is audio descriptor channel. Get returns null if no Set
    has been performed earlier
Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
    BRAP_SetAudioDescriptorFadeTable
    BRAP_CreateAudioDescriptorFadeTable
    BRAP_DestroyAudioDescriptorFadeTable
**************************************************************************/
BERR_Code BRAP_GetAudioDescriptorFadeTable(
    BRAP_AudioDescriptorFadeHandle hAdFade,     /* [in] Audio Desc handle */ 
    BRAP_AudioDescriptorFadeTableParams *psParams /* [out] Audio Desc Params */
    );

/***************************************************************************
Summary:
	Gets default association settings.
	
Description:
	
Returns:
	BERR_SUCCESS for a successful fillup, else returns an error.
		
See Also:
	BRAP_CreateAssociation.
***************************************************************************/
BERR_Code BRAP_GetDefaultAssociationSettings(
    BRAP_Handle    hRap,                        /* [in] Raptor Device Handle */
    BRAP_AssociatedChanSettings     *pAssociatedChSettings        
                                                /* [out] Pointer to a stucture
                                                   that holds the details of 
                                                   channels to be grouped 
                                                   together */
	);

/***************************************************************************
Summary:
	Gets default destination details.
	
Description:
	
Returns:
	BERR_SUCCESS for a successful fillup, else returns an error.
		
See Also:
	BRAP_AddDestination.
***************************************************************************/
BERR_Code BRAP_GetDefaultDstDetails(
    BRAP_Handle    hRap,                    /* [in] Raptor Device Handle */
    BRAP_AudioDst   eAudioDst,              /* [in] Destination Type */
    BRAP_DstDetails *pDstDetails
                                            /* [out] Details of the destination 
                                               to be added to the channel group 
                                            */
    );


/***************************************************************************
Summary:
	Sets the Destination volume level for left and right channels.

Description:
    This API has the volume control on the Destination.
    The default value written will be 0x800000
    
Note: This API can be used  for both output port and ring buffers carrying PCM data.
    
Returns:
	BERR_SUCCESS if successful else error value.            
	
See Also:
    BRAP_GetDestinationVolume()

**************************************************************************/
BERR_Code BRAP_SetDestinationVolume ( 
    BRAP_DestinationHandle	hDstHandle, /* [in] Destination Handle */
    unsigned int    		uiLVolume,  /* [in] Left channel volume in 0.23 format */
    unsigned int    		uiRVolume   /* [in] Right channel volume in 0.23 format */
    );


/***************************************************************************
Summary:
	Gets the Destination volume level for left and right channels.

Description:
    This API gets the present Destination volume level. 
    
Note: This API can be used  for both output port and ring buffers carrying PCM data.
    
Returns:
	BERR_SUCCESS if successful else error value.            
	
See Also:
    BRAP_SetDestinationVolumeGain()

**************************************************************************/
BERR_Code BRAP_GetDestinationVolume ( 
    BRAP_DestinationHandle	hDstHandle, /* [in] Destination Handle */
    unsigned int *  		pLVolume,  	/* [out] Left channel volume in 0.23 format */
    unsigned int *  		pRVolume    /* [out] Right channel volume in 0.23 format */
    );

/***************************************************************************
Summary:
    Mute/UnMute data at the output port. 

Description:
    This API mutes/unmutes an output port irrespective of the data type is
    compressed or PCM.

Returns:
	BERR_SUCCESS if successful else error value.            
	
See Also:
    BRAP_GetDestinationMute().
**************************************************************************/
BERR_Code BRAP_SetDestinationMute ( 
    BRAP_DestinationHandle	hDstHandle, /* [in] Destination Handle */
    bool            bMute    /* [in] TRUE: Mute output
                                     FALSE: UnMute output */
    );

/***************************************************************************
Summary:
    Gets whether an output is muted or not.

Description:
    This API returns the mute status for an output port.

Returns:
	BERR_SUCCESS if successful else error value.            
	
See Also:
    BRAP_SetDestinationMute().    
**************************************************************************/
BERR_Code BRAP_GetDestinationMute ( 
    BRAP_DestinationHandle	hDstHandle, /* [in] Destination Handle */
    bool            *pMute   /* [out] Pointer to memory where the Mute 
                                status is to be written */            
    );

void BRAP_ReadRbuf(BRAP_Handle hRapCh);

/************************************************************************
Summary: Returns the sum of following errors
        PTS_Error + Crc_Error + Dec_unlock +TSM_Fail
************************************************************************/
BERR_Code   BRAP_GetTotalErrorCount(BRAP_ChannelHandle  hRapCh,unsigned int *uiTotalCount);


/***************************************************************************
Summary:
    Sets the ramping step size for volume change for all output ports.

Description:
The mixer output volume value is changed by this step size every fs during ramping. 
The 23 LSB of this field is the fractional part and the [26:23] MSB is the integer part. 
This step range from 16 to 0.  bit[27] should be set to zero.

Ramping here is on the change in the Mixer output volume level. Below is the
ramp time calculation.

Ramp Time = [|cur_vol - new_vol| / vol_ramp_step] / Fs
Assume, 
cur_vol = 0x80_0000 = 8388608 (dec), 
new_vol = 0
vol_ramp_step = 0xA00 (hex) = 2560 (dec)
Fs = 48000 Hz

Ramp Time = [(8388608 - 0 ) / 2560 ] / 48000 =  0.06826(sec)

The range is 0  to 0x7FFFFFF

To have a smooth ramping minimum of 1024 samples are required, 
for 48khz sample rate this accounts to 21.33ms ramp time so any ramp step size 
below 0x2000 will provide smooth ramping.

Default value is 0xA00 (Ramp time ~68ms)
    
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_GetMixerRampStepSize().
    
**************************************************************************/
BERR_Code BRAP_SetMixerRampStepSize ( 
    BRAP_Handle     hRap,       /* [in] Audio Device Handle */
    unsigned int    uiRampStep  /* [in] Ramp Step size  */
);

/***************************************************************************
Summary:
    Gets the ramping step size for volume change 

Description:
    This function returns ramping step size for scale and volume change.
     Its range is 0  to 0x7FFFFFF
    
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_SetMixerRampStepSize().
    
**************************************************************************/
BERR_Code BRAP_GetMixerRampStepSize( 
    BRAP_Handle     hRap,       /* [in] Audio Device Handle */
    unsigned int *  pRampStepSize /* [out] Pointer to memory where ramp size should be stored.*/
);

#if (BRAP_7550_FAMILY != 1)
/***************************************************************************
Summary:
    Sets the ramping step size for scale change for all output ports.

Description:
The scaling value is changed by this step size every fs during ramping. 
The 15 LSB of this field is the is fractional part and MSB is integer part.  
The maximum step size is 0x8000 (1).

Ramping here is on the change of audio sample value under the conditions of
Startup/Underflow/MUTE_ENA. Below is the ramp time calculation.

Ramp Time = [|old_sample_value - new_sample_value| / vol_ramp_step] / Fs
Assume,
old_sample_value = 0, 
new_sample_value = 0x7F_FFFF = 8388607 (dec)
vol_ramp_step = 0xA00 (hex) = 2560 (dec)
Fs = 48000 Hz

Ramp Time = [|0 - 8388607| / 2560 ] / 48000 =  0.06825(sec)

The range is 0  to 0x8000

To have a smooth ramping minimum of 1024 samples are required, 
for 48khz sample rate this accounts to 21.33ms ramp time so any ramp step size 
below 0x2000 will provide smooth ramping.

Default value is 0xA00 (Ramp time < 100ms)

    
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_GetSrcRampStepSize().
    
**************************************************************************/
BERR_Code BRAP_SetSrcRampStepSize ( 
    BRAP_Handle     hRap,       /* [in] Audio Device Handle */
    unsigned int    uiRampStep  /* [in] Ramp Step size  */
);


/***************************************************************************
Summary:
    Gets the ramping step size for scale change 

Description:
    This function returns ramping step size for scale and volume change.
Its range is 0  to 0x8000
    
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_SetRampStepSize().
    
**************************************************************************/
BERR_Code BRAP_GetSrcRampStepSize( 
    BRAP_Handle     hRap,       /* [in] Audio Device Handle */
    unsigned int *  pRampStepSize /* [out] Pointer to memory where ramp size should be stored.*/
);
#endif
/***************************************************************************
Summary:
    Sets the ramping step size for Ping Pong coefficients for all mixers.

Description:
The Ping coefficients value is changed by this step size every fs during ramping. 
The 23 LSB of this field is the fractional part and the bit[26:23] MSB is the integer part. T
his step ranges from 16 to 0.  Bit [27] should always set to zero.
0x80_0000 = 1
0x40_0000 = 0.5

default is 0x00100000

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_GetSrcRampStepSize().
    
**************************************************************************/
BERR_Code BRAP_SetScalingCoefRampStepSize ( 
    BRAP_Handle     hRap,       /* [in] Audio Device Handle */
    unsigned int    uiRampStep  /* [in] Ramp Step size  */
);

/***************************************************************************
Summary:
    Gets the ramping step size for scale change 

Description:
    This function returns ramping step size for ping/pong coefficents
Its range is 0  to 0x8000
    
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_SetSrcRampStepSize().
    
**************************************************************************/
BERR_Code BRAP_GetScalingCoefRampStepSize( 
    BRAP_Handle     hRap,       /* [in] Audio Device Handle */
    unsigned int *  pRampStepSize /* [out] Pointer to memory where ramp size should be stored. */
);

BERR_Code BRAP_FlushPbChannel (
	BRAP_ChannelHandle	        hRapCh		/* [in] The PB Channel handle */
	);


BERR_Code BRAP_GetCurrentRfModSource(BRAP_Handle hRap,unsigned int uiRfModIndex, BRAP_OutputPort    *eSource);
BERR_Code BRAP_SetRfModSource(BRAP_Handle hRap,unsigned int uiRfModIndex, BRAP_OutputPort    eSource);

/***************************************************************************
Summary:
	Function that updates input sampling rate of PCM Plaback channel on the fly.
Description:
	FFunction that updates input sampling rate of PCM Plaback channel on the fly..This function must be called
	for PCM playback channels only.
Returns:
	BERR_SUCCESS 
****************************************************************************/
BERR_Code 
BRAP_PB_SetInputSampleRate(
		BRAP_ChannelHandle hRapCh,/*PCM PB Channel Handle*/
		unsigned int uiInputSampleRate/*Input Sample rate of PCM PB content.
		                                                   It can have any +ve possible value. UNit should be Hz.
		                                                   e.g. 44100 for 44.1K , 9000 for 9k. etc.*/
		);

#ifdef RAP_VIDEOONDSP_SUPPORT
/***************************************************************************
Video PVR related calls.
***************************************************************************/
BERR_Code BRAP_PvrSetSlowMotionRate
(
    BRAP_ChannelHandle hRapCh,
    unsigned long ulRate
);

BERR_Code BRAP_PvrGetSlowMotionRate
(
    BRAP_ChannelHandle hRapCh,
    unsigned long *pSMRate
);

BERR_Code BRAP_PvrEnableReverseFields(
    BRAP_ChannelHandle hRapCh,
    bool bEnable
);

BERR_Code BRAP_PvrGetReverseFieldStatus
(
    BRAP_ChannelHandle hRapCh,
    bool *pbEnable
);

BERR_Code BRAP_PvrSetHostSparseMode(
    BRAP_ChannelHandle hRapCh,
    bool bSparseMode
);

BERR_Code BRAP_PvrGetHostSparseMode
(
    BRAP_ChannelHandle hRapCh,
    bool *pbSparseMode
);

BERR_Code BRAP_PvrSetGopTrickMode
(
    BRAP_ChannelHandle hRapCh,
    bool bEnableTrickMode
);

BERR_Code BRAP_PvrGetGopTrickMode
(
    BRAP_ChannelHandle hRapCh,
    bool *pbEnableTrickMode
);

BERR_Code BRAP_PvrSetAutoValidateStcOnPause
(
    BRAP_ChannelHandle hRapCh,
    bool bAutoValidateStcOnPause
);

BERR_Code BRAP_PvrGetAutoValidateStcOnPause
(
    BRAP_ChannelHandle hRapCh,
    bool *pbAutoValidateStcOnPause
);

#endif

void BRAP_GetCapabilities(
    BRAP_Handle hRap, 
    BRAP_Capabilities *pCapabilities /* [out] */ 
    );

/***************************************************************************
Summary:
	Returns current audio rate of the Raptor Channel.

Description:
	This function returns current audio rate of the Raptor Channel.
	Valid only for decode channel

Returns:
	BERR_SUCCESS - if passed parameters are correct.

See Also:
	BRAP_SetAudioRate
***************************************************************************/
BERR_Code BRAP_GetAudioPlayRate(
	BRAP_ChannelHandle		    hRapCh,		        /* [in] RAP channel handle */
	unsigned int                            *uiPBRate 	/* [out] Current audio rate*/
	);

/***************************************************************************
Summary:
	Sets  audio rate of the Raptor Channel on the fly.

Description:
	This function sets  audio rate of the Raptor Channel on the fly. This API should be called only 
	1) when decode Channel is in started case, 
	2) the decode channel has been started with non-1x rate.
	Valid only for decode channel

Returns:
	BERR_SUCCESS - if passed parameters are correct.

See Also:
	BRAP_GetAudioRate
***************************************************************************/

BERR_Code BRAP_SetAudioPlayRate(
	BRAP_ChannelHandle		    hRapCh,		        /* [in] RAP channel handle */
	unsigned int                            uiPBRate 	/* [in]  audio rate*/
	);



#ifdef __cplusplus
}
#endif

#endif /* _BRAP_H__ */



