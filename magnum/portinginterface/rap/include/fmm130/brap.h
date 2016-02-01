/***************************************************************************
*     Copyright (c) 2004-2011, Broadcom Corporation
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
* Hydra_Software_Devel/88   9/27/06 * $brcm_Revision: $
* $brcm_Date: $
5:45p rjain
* PR 22087: PI changes for independent output delay
* 
* Hydra_Software_Devel/87   9/25/06 7:41p gautamk
* PR 23493: Playback of stream causes Error: Unknown AAC sampling rate
* 
* Hydra_Software_Devel/86   9/12/06 4:18p rjain
* PR 22760: Support for unsigned data in raptor
* 
* Hydra_Software_Devel/85   9/8/06 10:57a bhanus
* PR 22760: Added support for 8Bit Unsigned Playback
* 
* Hydra_Software_Devel/84   9/7/06 11:32a rjain
* PR 22868: changing SECURE_HEAP to BRAP_SECURE_HEAP
* 
* Hydra_Software_Devel/83   8/31/06 12:27p rjain
* PR 22868: Moving the hSecureMem handle into BRAP_Settings()
* 
* Hydra_Software_Devel/82   8/23/06 4:29p rjain
* PR 22868: added code to pass a 2nd memory handle (for secure region).
* this 2nd handle is used for RBUF memory allocation. Note: application
* has to take care to pass correct secure memory handle
* 
* Hydra_Software_Devel/81   8/2/06 5:58p bselva
* PR 22486: Merged the changes for multichannel and concurrent stereo
* downmix support for 7400 to main branch
* 
* Hydra_Software_Devel/BLR_RAP_7400_MULTICH/3   7/19/06 1:30p bselva
* PR 22486: Checkin the changes for concurrent stereo downmix code.
* 
* Hydra_Software_Devel/BLR_RAP_7400_MULTICH/2   7/13/06 1:33p bselva
* PR 22486: Merging with the latest code
* 
* Hydra_Software_Devel/80   7/12/06 4:28p bselva
* PR 22288: Checking in the usage of timer for full mark interrupt
* 
* Hydra_Software_Devel/79   7/12/06 4:12p sushmit
* PR18604: Removing BCHP_7411_VER
* 
* Hydra_Software_Devel/78   7/7/06 2:56p rjain
* PR22466: adding description
* 
* Hydra_Software_Devel/77   7/5/06 11:52a rjain
* PR22466: adding module overview
* 
* Hydra_Software_Devel/76   7/3/06 8:27p nitinb
* PR 22443: Replace AC3 decoder with DDP decoder in AC3 Lossless codec
* 
* Hydra_Software_Devel/75   6/27/06 1:58p bhanus
* PR 22288 : Checking in timer workaround for water mark interrupt bug
* 
* Hydra_Software_Devel/74   6/25/06 3:40p kagrawal
* PR 22297: Added support for stereo downmix path concurrent with
* multichannel path
* 
* Hydra_Software_Devel/73   6/19/06 2:23p rjain
* PR 22179: adding description for start-time / on-the-fly changeable
* paramters
* 
* Hydra_Software_Devel/72   6/13/06 2:53p nitinb
* PR 22083: Add support for MLP algorithm in 7411D0 RAP PI
* 
* Hydra_Software_Devel/71   5/17/06 11:31a sushmit
* PR18604: Updating Encoder PI for 7401.
* 
* Hydra_Software_Devel/70   5/10/06 8:47p nitinb
* PR 20117: Added configuration parameter structure for DDBM
* 
* Hydra_Software_Devel/69   5/4/06 6:15p rjain
* PR 21403 : Removing eOutputLR, eOutputLRSurround, eOutputCntrLF,
* bSimulModeConfig, eSimulModePtOutput from BRAP_DEC_ChannelSettings for
* 7401 onwards
* 
* Hydra_Software_Devel/68   5/4/06 2:32p nitinb
* PR 21400: Fixing compilation error for 7400
* 
* Hydra_Software_Devel/67   4/12/06 1:37p bselva
* PR 20318: Implemented the Audio Manager review coments
* 
* Hydra_Software_Devel/66   4/12/06 10:23a nitinb
* PR 20736: PR for Raptor Interrupt code review related changes
* 
* Hydra_Software_Devel/65   4/11/06 5:42p rjain
* PR 20752: fixing file inclusion warning
* 
* Hydra_Software_Devel/64   4/3/06 6:32p rjain
* PR 20318: Implementing code review comments for AM
* 
* Hydra_Software_Devel/63   3/28/06 12:03p rnayak
* PR20439: Restore Bangalore Raptor files after magnum vob corruption on
* 17th March 2006
* 
* Hydra_Software_Devel/63   3/21/06 2:40p nitinb
* PR 20287: Fixing compilation error on 7401
* 
* Hydra_Software_Devel/62   2/27/06 2:18p rjain
* PR 19593: changing BRAP_MAX_AUDIO_TYPES to BRAP_DSPCHN_AudioType_eMax
* 
* Hydra_Software_Devel/61   2/27/06 2:12p rjain
* PR 19593: removing unsupported algo types
* BRAP_DSPCHN_AudioType_ePcmPlayback and BRAP_DSPCHN_AudioType_eDv25
* from enum  BRAP_DSPCHN_AudioType.
* 
* Hydra_Software_Devel/60   2/17/06 9:53a sushmit
* PR18604: Merging Encoder to latest.
* 
* Hydra_Software_Devel/RAP_AUD_ENC/2   2/6/06 4:16p sushmit
* PR18604: Merging latest Decoder PI to Encoder PI branch
* 
* 
* Hydra_Software_Devel/55   1/4/06 1:39p bselva
* PR 18863: Added BRAP_DEC_DisableForFlush for 7401
* 
* Hydra_Software_Devel/54   1/3/06 4:33p bselva
* PR 18167: Changed BRAP_Settings.pImgInterface to const
* 
* Hydra_Software_Devel/53   12/19/05 2:36p kagrawal
* PR 18672: Capture start param eInputBitsPerSample and
* eInputSamplingRate will be used only when the capture port type is
* external (ExtI2s)
* 
* Hydra_Software_Devel/52   12/16/05 6:16p rjain
* PR 18277: Changes to add  & remove an output port to a channel after
* chanenl open / before Close
* 
* Hydra_Software_Devel/51   12/13/05 3:24p kagrawal
* PR 18312: Added comments on uiWaterMark
* 
* Hydra_Software_Devel/50   12/13/05 1:49p kagrawal
* PR 18578: Added support for dither and burst mode programming for
* SPDIFFM
* 
* Hydra_Software_Devel/49   12/9/05 6:37p rjain
* PR 18264: Fixing 7401 compilation
* 
* Hydra_Software_Devel/48   12/9/05 5:28p rjain
* PR 18264: Moving PB and CAP related public structure etc from
* brap_pcm.h .
* 
* Hydra_Software_Devel/47   12/5/05 3:40p nitinb
* PR 17900: Fixing compilation warning on 7401
* 
* Hydra_Software_Devel/46   12/2/05 4:56p nitinb
* PR 18061: Raptor memory allocation must be less than 0.5M for MPEG/AC-3
* only
* 
* Hydra_Software_Devel/45   12/2/05 1:24p bselva
* PR 18312: Added support for BD-LPCM
* 
* Hydra_Software_Devel/44   11/30/05 5:35p kagrawal
* PR 18312: Fixed compilation break in 7401
* 
* Hydra_Software_Devel/43   11/30/05 4:10p kagrawal
* PR 18279: Rearchitected output sampling rate change for a PCM channel
* 
* Hydra_Software_Devel/42   11/16/05 9:45p rjain
* PR 18129: updating comments for AddOutputport
* 
* Hydra_Software_Devel/41   11/15/05 4:14p rjain
* PR 18129: Cloned port now get stopped/restarted on a normal channel
* stop/start
* 
* Hydra_Software_Devel/40   11/14/05 9:53a rjain
* PR 18129: Adding support for routing to multiple output ports
* 
* Hydra_Software_Devel/39   11/10/05 11:51a kagrawal
* PR 17590: Merging PCM Playback and Mixing changes to
* Hydra_Software_Devel
* 
* Hydra_Software_Devel/Kapil_PCM_Playback/2   11/1/05 9:40a kagrawal
* 1. Implemented review comments on PCM playback and mixing
* 2. Added a new enum BRAP_BufDataMode
* 
* Hydra_Software_Devel/Kapil_PCM_Playback/1   10/24/05 4:35p kagrawal
* PR 17590: Initial check in for PCM Playback and Mixing
*
* Hydra_Software_Devel/38   11/2/05 3:23p bselva
* PR 17590: Added watchdog support for 7401
* 
* Hydra_Software_Devel/37   10/26/05 2:30p nitinb
* PR 17415: Added more comments
* 
* Hydra_Software_Devel/36   10/24/05 5:33p nitinb
* PR 17415: Added prototypes of APIs BRAP_DEC_SetConfig,
* BRAP_DEC_GetDefaultConfig, BRAP_DEC_GetCurrentConfig
* 
* Hydra_Software_Devel/35   10/14/05 5:40p nitinb
* PR 17420: Fixing compilation warning for 7401
* 
* Hydra_Software_Devel/34   10/14/05 3:31p nitinb
* PR 17420: Request for Audio Flush feature, for wrap-around conditions
* 
* Hydra_Software_Devel/33   9/30/05 6:33p nitinb
* PR 16982: Audio Raptor PI shall use BIMG interface to access firmware
* interface
* 
* Hydra_Software_Devel/32   9/27/05 8:07p nitinb
* PR 16706: Raptor should not fire StreamInfo interrupt when only chMod
* field changes
* 
* Hydra_Software_Devel/31   9/21/05 5:04p rjain
* PR 16783: adding description for the test tone PI
* 
* Hydra_Software_Devel/30   9/20/05 4:03p nitinb
* PR 16899: Added prototype of API BRAP_ProcessWatchdogInterrupt
* Removed Watchdog recovery mode enum
* 
* Hydra_Software_Devel/29   9/20/05 1:14p nitinb
* PR 16899: Added prototype of API BRAP_ProcessWatchdogInterrupt
* 
* Hydra_Software_Devel/28   9/19/05 7:14p bmishra
* PR 16148: Removed chip specific code from interface files
* 
* Hydra_Software_Devel/27   9/14/05 7:38p nitinb
* PR 17042: Raptor does not support sample rate change in the middle of a
* stream
* 
* Hydra_Software_Devel/26   9/13/05 11:58a rjain
* PR 16783: additions for raptor test tone.
* 
* Hydra_Software_Devel/25   9/13/05 7:42p bmishra
* PR 16148: Merged 7401 related changes
* 
* Hydra_Software_Devel/BLR_RAP_7401_Bringup/5   8/31/05 6:39p bselva
* PR 16148: Adding DAC support
* 
* Hydra_Software_Devel/BLR_RAP_7401_Bringup/4   8/19/05 12:02p bmishra
* PR 16148: Mergeded the latest changes in the main branch.
* 
* Hydra_Software_Devel/BLR_RAP_7401_Bringup/3   8/1/05 5:07p bmishra
* PR 16148: Added API to query CDB/ITB sizes required
* 
* Hydra_Software_Devel/24   8/3/05 9:15p bselva
* PR 15766: Support for BRAP_GetRevision API added
* 
* Hydra_Software_Devel/23   7/21/05 12:57p rjain
* PR 16354: FMM related changes for Pause/Resume
* 
* Hydra_Software_Devel/BLR_RAP_7401_Bringup/2   7/14/05 11:10a bmishra
* PR 16148: Removed references to ARC
* 
* Hydra_Software_Devel/BLR_RAP_7401_Bringup/1   7/12/05 8:13p bmishra
* PR 16148: Modified the file for 7401 compatibility
* 
* Hydra_Software_Devel/22   7/5/05 3:04p nitinb
* PR 15765: Added decoder lock and decoder unlock interrupts
* 
* Hydra_Software_Devel/21   6/24/05 3:28p bmishra
* PR 14143: Implemented code review comments
* 
* Hydra_Software_Devel/20   6/18/05 3:29p rjain
* PR 15925: PI changes required for routing decoded data to I2S and SPDIF
* ports simultanesouly
* 
* Hydra_Software_Devel/19   6/10/05 8:22p nitinb
* PR 15837: Feature development for the Minor Release of June 2005
* (Implemented TSM log)
* 
* Hydra_Software_Devel/18   6/7/05 11:43a rjain
* PR 14143: Implemented FMM Code review comments.
* 
* Hydra_Software_Devel/17   5/25/05 7:29p nitinb
* PR 15272: Adding interrupts FirstPTSReady, PTSError and StreamInfo
* 
* Hydra_Software_Devel/16   5/25/05 3:58p rjain
* PR 15272: Adding some code for interrupt handling framework.
* 
* Hydra_Software_Devel/15   5/13/05 2:40p rjain
* PR 15160: Need ability to set various fields in the SPDIF channel
* status
* 
* Hydra_Software_Devel/14   5/12/05 6:25p bmishra
* PR 14143: Added more error codes
* 
* Hydra_Software_Devel/13   4/20/05 3:29p bmishra
* PR 14507: Added support for DSP Channel settings geting passed from the
* user
* 
* Hydra_Software_Devel/12   4/15/05 2:58p bmishra
* PR 14507: Simultaneous mode related changes
* 
* Hydra_Software_Devel/11   3/11/05 1:49p nitinb
* PR 13082: API BRAP_DEC_GetDefaultAudioParams should take
* BRAP_ChannelHandle (not BRAP_Handle)
* 
* Hydra_Software_Devel/10   3/10/05 3:23p rjain
* PR 13082:
* - Implemented the new Start-Stop scheme where Hosts prgms all
* registers. DSP uses Start_WRPoint and RBUF_EndAddress to start decode.
* - removed bMixingEnable from BRAP_MIXER_Params
* - removed sRBufParams from BRAP_DEC_AudioParams.
* - Added BRAP_RM_P_GetDpStreamId
* - Corrected programming of Stream Id in DP registers
* - Added BRAP_SRCCH_P_WaitForRampDown()
* - Other minor changes
* 
* Hydra_Software_Devel/8   2/26/05 4:53p nitinb
* PR 14143: Created new RAP error BRAP_ERR_OUTPUT_NOT_CONFIGURED
* 
* Hydra_Software_Devel/7   2/17/05 7:34p bmishra
* PR 14143: Changes related to passing BARC Handle in place of BREG
* Handle to BRAP_open
* 
* Hydra_Software_Devel/6   2/16/05 8:30p bmishra
* PR 14143: Implemented some Phase 1 code review comments for the
* interfaces only
* 
* Hydra_Software_Devel/5   2/16/05 1:47p rjain
* PR 14143: Implementing some comments from Phase 1 Code review done till
* date
* 
* Hydra_Software_Devel/4   2/15/05 8:08p bmishra
* PR 14143: Implemented Phase 1 code review comments for the interfaces
* only
* 
* Hydra_Software_Devel/3   1/25/05 9:52a rjain
* PR13082: Modifying according to settings/bugs highlighted in emulation
* so far.
* 
* Hydra_Software_Devel/2   1/12/05 5:01p rjain
* PR13082: Updating with changes after compilation of FMM, AM and RM
* together.
* 
* Hydra_Software_Devel/1   1/10/05 8:27p nitinb
* PR 13082: Moved to new directory structure
* 
* Hydra_Software_Devel/5   1/3/05 3:04p bmishra
* PR13082: Implemented PI Interface External Review comments
* 
* Hydra_Software_Devel/4   12/9/04 8:35p bmishra
* PR13082: Reformatting for proper DOCJET output
* 
* Hydra_Software_Devel/3   12/9/04 8:11p bmishra
* PR13082: Implemented review comments from f/w team
* 
* Hydra_Software_Devel/2   12/9/04 1:20p bmishra
* PR13082: Implemented internal design review comments
* 
* Hydra_Software_Devel/1   12/9/04 10:53a rnayak
* PR13082: Initial Version
* 
***************************************************************************/
 
#ifndef _BRAP_H__
#define _BRAP_H__

/*=************************ Module Overview ********************************

Overview
--------

The Broadcom Audio API is a porting interface (PI) module which
allows status/control of all audio components in the system. 

This Broadcom Audio core is called Raptor, abbreviated as RAP.

The core or the audio hardware is reffered to as the 'Raptor device'.

All audio services on this device are provided on logical channels depending
on the functionality, which in turn contains multiple audio channels. Each 
logical channel is called a Raptor Channel. A Raptor channel allows the 
HW/FW to receive a single audio stream from the transport layer.
Depending on the functionality, these are classified as :

1.	Decode channel (DEC Channel): Decode channel uses DSP processor and firmware. 
       Various operations performed by decode channel are - decode and post processing, 
       pass through, sample rate conversion (SRC).
2.	Playback channel (PB Channel): PCM Playback channel
3.  Capture channel (CAP Channel): PCM Capture channel
4.	Encode channel (ENC Channel): Encoder channel

Clients of the audio PI are expected to open the Raptor device and then 
open audio channels of the specific type, as needed. Opening the Raptor 
device itself results in initialization of all configuration that applies 
globally. Opening a Raptor channel configures the only the features of the 
channel that are constant during the lifetime of the channel. Complete 
configuration of an individual channel is completed when the channel is 
started by the client software.

All Raptor APIs that operate on the entire device or across multiple Raptor 
channels use a BRAP_Handle.  This handle maintains the hardware and software 
states for this device.

All Raptor APIs that operate on a specific channel use a BRAP_ChannelHandle.
This handle maintains the hardware and software states for each logical channel.


Raptor HW overview:

    Raptor comprises of the following HW modules
    - DSP : Does the actual processing
    - FMM : Flexible Mixer and Multiplexer.

    The FMM in turn comprises of
    - Ring buffers (BRAP_RBUF_): This is the DRAM buffer memory. Controls all 
    AUD_FMM_BF_CTRL_RINGBUF_XXX registers.
    - Destination Channels (BRAP_DSTCH_) : This provides a local fifo in the 
    Capture path. Controls all AUD_FMM_BF_CTRL_DESTCH_XXX registers.
    - Source Channels (BRAP_SRCCH_) : This provides a local fifo to route 
    the data from the DRAM (ring buffer) to the Mixers. Controls all 
    AUD_FMM_BF_CTRL_SOURCECH_XXX registers.
    - Mixer (BRAP_MIXER_) : This is where the actual mixing of data takes 
    place. Volume control, scaling etc are also done here. Controls all 
    AUD_FMM_DP_XXX registers.
    - SPDIF Formatter (BRAP_SPDIFFM_): Formats streams to be output on the 
    SPDIF, HDMI or Flex ports in SPDIF format. Controls all AUD_FMM_MS_XXX 
    registers.
    - Output ports (BRAP_OP_): The actual output ports of the system like the 
    Spdif, I2S, DAC etc. Controls all AUD_FMM_OP_XXX registers.
 

    These HW resources are combined in various ways to form Raptor Channels.
    The SW entity managing these resources is called the 'Audio Manager'.

    For details about each module, refer to their respective header files.


Raptor Software Design
The design for Raptor API is broken into the following parts:

 o Raptor Device

   The Raptor Device (BRAP), which controls common parts of audio channels and
   its main APIs

 o Decode Channels

   Decode channels(BRAP_DEC_) which decode an audio stream. These use one or more 
   DSP context to decode the audio stream and then route it via the FMM to the 
   output ports. 

   Possible decode modes are enumerated in BRAP_DSPCHN_DecodeMode.
   Decode   : Decode an audio stream
   Passthru : Pass the audio stream through as compressed data. Dont process it.
   Simul    : Decode the audio stream and parallelly send it out in the compressed 
              form too.

   The Data Flow for Decode channels is:
   input stream -> DSP -> RBUF -> SRCCH -> MIXER -> SPDIFFM (optional) -> OUTPUT PORT

   Various Raptor chips support different combinations of these modes:
    For 7411: 
    Max number of DEC channels supported = 2
    DEC channel1        DEC channel2
    -----------         ------------
    Decode              Passthru
    Passthru            Decode
    Simul mode          Not supported
    Not supported       Simul mode

    For 7401:
    Max number of DEC channels supported = 2
    DEC channel1        DEC channel2
    -----------         ------------
    Decode              Passthru
    Passthru            Decode
    Simul mode          Not supported
    Not supported       Simul mode

    For 7400:
    Max number of DEC channels supported = 3
    Worst case combinations: 
        2 Decode + 1 PassThru
        1 Simul + 1 Decode
        1 Simul + 1 Pass
   Limitation: Only 2 PCR blocks are available so we can have combinations that 
   use use upto 2 different timebases only

 o Playback Channels 

   PCM Playback channels(BRAP_PB_) which controls PCM playback. PCM data is 
   directly read from DRAM and played out through the FMM.
   The DSP does not come into play at all.

   The Data Flow for Playback channels is:
   PCM data -> RBUF -> SRCCH -> MIXER -> SPDIFFM (optional) -> OUTPUT PORT

    <TODO>


 o Capture Channels

   PCM Capture channels(BRAP_CAP_) which capture PCM data. 
   PCM data can be captured from an external source, or looped back from some 
   output ports of the Raptor device itself. Possible Capture modes are listed 
   in the enumeration BRAP_CaptureMode

   The Data Flow for Capture channels depends on the capture mode.

      
    CaptureOnly : Data is only saved into the Ring Buffers (DRAM).
                  PCM data -> DSTCH -> RBUF 
    ByPass      : Data is not saved into the DRAM, but is directly routed to 
                  the output ports via the FMM.        
                  PCM data -> DSTCH -> SRCCH -> MIXER -> SPDIFFM (optional) -> OUTPUT PORT                  
    Full Duplex : Data is saved into the DRAM AND also routed to the output 
                  ports via the FMM. 
                  PCM data -> DSTCH -> RBUF -> SRCCH -> MIXER -> SPDIFFM (optional) -> OUTPUT PORT                  
                  
   The DSP does not come into play at all.

 o Encode Channels 

    <TODO>


Usage: 

Steps 1 through 3 are required to open and partially configure a Raptor 
channel. These are usually only done once at system initialisation time.

Steps 4 and 5 are required to fully configure a Raptor Channel and start the 
Raptor channel. Channels can be stopped and restarted multiple times.

After this, various other APIs can be called to change the configuration, stop 
& restart the channel or what ever the application requires. The other APIs 
are described in the respective header files and broadly fall into the 
following categories
    - APIs to change/update configuration
    - APIs to get status or information
    - Miscellaneous

Step 6 shows how to stop a Raptor channel.

Steps 7 and 8 show how to properly shut down the Raptor device.


Step 1: Opening the Raptor device

        BRAP_Settings sRapSettings;
        BRAP_GetDefaultSettings(&sRapSettings);
        // Modify sRapSettings as required.
        // Get other handles required to open the audio device.
        BRAP_Open(&hRap, hChip, hReg, hMemory, hInt, &sRapSettings);

Step 2: Configure output ports 

    Configure all output ports that may be used. This doesnt Open or start 
    the port. It simply saves the configuration to be used
    later when any channel starts to use this port.

        BRAP_OutputSettings  sSettings;
        // Get default configuration for an output port, say I2S0 
        BRAP_GetOutputDefaultConfig (hRap, BRAP_OutputPort_eI2s0, sSettings);    
        // Modify as required. Remember to fill in the output port type
        sSettings.eOutput = BRAP_OutputPort_eI2s0;
        // Configure the port
        BRAP_SetOutputConfig (hRap, sSettings);

   - This has to be done for all output ports you want to use.
   - Note that a Raptor channel can send out audio data onto various output 
   ports. Each output port can carry a PCM data channel pair. Some ports like 
   the HDMI or SPDIF can carry compressed data.
   - Multiple Raptor channels can share an output port ie multiple Raptor channels 
   can be mixed and the mixed data can be routed to an output port.

Step 3: Open the Raptor channel. 
 
        // Get the default channel  settings
        BRAP_XXX_GetChannelDefaultSettings(hRap, <pointer to settings variable>);
        // Modify the configuration as required.
        // Open the channel
        BRAP_XXX_OpenChannel (hRap, &hRapCh, <settings >); 

    Where _XXX_ is
        _DEC_ for a Decode channel
        _PB_  for a PCM Playback channel
        _CAP_ for a PCM Capture channel


Step 4: Add output ports as required 
Configure which output port should carry which audio channel pair of each 
Raptor channel.

        BRAP_AddOutputPort (hRapCh, false, BRAP_OutputChannelPair_eLR, BRAP_OutputPort_eSpdif);

    Eg1. For a given Raptor channel that contains multiple channel pairs, to route the LR channel pair 
    to the DAC and the LsRs channel pair to I2S0, use the following:
        BRAP_AddOutputPort (hRapCh, false, BRAP_OutputChannelPair_eLR, BRAP_OutputPort_eDac);
        BRAP_AddOutputPort (hRapCh, false, BRAP_OutputChannelPair_eLRSurround, BRAP_OutputPort_eI2S0);        

    Eg2. Lets say you now want to mix a PCM sound effect from a Raptor Playback 
    channel to the LR channel pair of the channels from the previous example.
        BRAP_AddOutputPort (hPbRapCh, false, BRAP_OutputChannelPair_eLR, BRAP_OutputPort_eDac);

    This would mix the the LR channel pair from hRapCh with the LR channel pair 
    of hPbRapCh and cause the mixed data to be sent out on the DAC.

    Note that at this point the channels have not yet been started. So there is 
    no actual audio to be heard yet.

Step 5: Start the channel/s
    
        // Get the default parameters required to start the Raptor channel
        BRAP_XXX_GetDefaultAudioParams(hRap, <pointer to settings variable>);
        // Modify the parameters as required.
        // Start the channel
        BRAP_XXX_Start (hRap, hRapCh, <settings >); 

    Where _XXX_ is
        _DEC_ for a Decode channel
        _PB_  for a PCM Playback channel
        _CAP_ for a PCM Capture channel


Step 6: Stop the Raptor channel

        BRAP_XXX_Stop (hRap, hRapCh); 

    Where _XXX_ is
        _DEC_ for a Decode channel
        _PB_  for a PCM Playback channel
        _CAP_ for a PCM Capture channel

Step 7: Close the Raptor channel

        BRAP_XXX_CloseChannel (hRap, hRapCh); 

    Where _XXX_ is
        _DEC_ for a Decode channel
        _PB_  for a PCM Playback channel
        _CAP_ for a PCM Capture channel


Step 8: Close the Raptor device

        BRAP_Close(hRap)


Miscellaneous:

Q: How to mix 2 Raptor channels?
Ans: There is no seperate PI to mix 2 Raptor channels. If you connect multiple 
channels to the same output port, they will automatically be mixed when you 
start them. Note that mixing of 2 Decode channels is not allowed. Any other 
combination of channels can be mixed.





***************************************************************************/

/* Include bstd.h before using any BCHP_ macros. */
#include "bstd.h"


/* Defines to include/exclude certain features. These need to be define BEFORE 
including any other files which might require these - effectively all the 
raptor internal modules' header files. */
#if (BCHP_CHIP == 7401 && BCHP_VER >= BCHP_VER_C0) || (BCHP_CHIP == 7403) || (BCHP_CHIP == 7400)  
#define BRAP_UNSIGNED_PCM_SUPPORTED 1
#else
#define BRAP_UNSIGNED_PCM_SUPPORTED 0
#endif

#if (BCHP_CHIP == 7401) || (BCHP_CHIP == 7118) || (BCHP_CHIP == 7403) || ( BCHP_CHIP == 7400 )
#define BRAP_INDEP_OP_DELAY_SUPPORTED 1
#else
#define BRAP_INDEP_OP_DELAY_SUPPORTED 0
#endif

/* For adding or removing DTS or DRA or AD or DoblyVolume features, 
need modify the brap.inc and brap_img.h files along with brap.h */
#if (BCHP_CHIP == 7403) 
#define BRAP_DTS_SUPPORTED 1 
#else
#define BRAP_DTS_SUPPORTED 0
#endif

#if (BCHP_CHIP == 7400) 
#define BRAP_DTS_PASSTHRU_SUPPORTED 1 
#else
#define BRAP_DTS_PASSTHRU_SUPPORTED 0
#endif

#if RAP_AD_SUPPORT 
#define BRAP_AD_SUPPORTED 1 
#else
#define BRAP_AD_SUPPORTED 0
#endif


#if RAP_DOLBYVOLUME_SUPPORT 
#define BRAP_DOLBYVOLUME_SUPPORTED 1
#else
#define BRAP_DOLBYVOLUME_SUPPORTED 0
#endif

#ifdef RAP_SRSTRUVOL_SUPPORT
#define BRAP_SRS_TRUVOL_SUPPORTED 1 
#else
#define BRAP_SRS_TRUVOL_SUPPORTED 0 
#endif

#if (BCHP_CHIP == 7401) || (BCHP_CHIP == 7118) || (BCHP_CHIP == 7403)
#define BRAP_7401_FAMILY 1 
#else
#define BRAP_7401_FAMILY 0
#endif

#define BRAP_SECURE_HEAP 1     /* For chips other than 7411, use Secure Heap for DEC RBUFS */


/* Including header files */

#include "brap_types.h"
#include "brap_dspchn.h"
#include "brap_fmm.h"
#include "brap_dstch.h"
#include "brap_rbuf.h"
#include "brap_srcch.h"
#include "brap_mixer.h"
#include "brap_spdiffm.h"
#include "brap_op.h"
#include "brap_pvr.h"
#include "brap_hifidac.h"


#include "brap_pcm.h"
#include "bavc.h"
#include "bimg.h"
#include "btmr.h"

#ifdef __cplusplus
extern "C" {
#endif

/*{{{ Defines */

#define BRAP_INVALID_VALUE			((unsigned int)(-1))
#define	BRAP_MAX_AUDIO_TYPES			(BRAP_DSPCHN_AudioType_eMax) /* Max audio types in  RAP PI. Corresponds to
											    number of enum values in BRAP_DSPCHN_AudioType */
#define BRAP_MAX_PP_ALGOS           (BRAP_DSPCHN_PP_Algo_eMax) /* Max number of post processing
                                                                 algorithms supported in RAP PI */
 
#define	BRAP_MAX_ENC_AUDIO_TYPES		(BRAP_DSPCHN_ENC_AudioType_eMax) /* Max audio encode algorithms */

#define	BRAP_MAX_DSP_DECODE_ALGO		20	/* Max supported DSP Decode Algoritms */
#define	BRAP_MAX_DSP_FS_ALGO			20	/* Max supported DSP Frame Sync Algoritms */
#define	BRAP_MAX_DSP_PP_ALGO			12	/* Max supported DSP Post Processing Algoritms */

#if (BRAP_AD_SUPPORTED == 1)
/* Max number of associated channel groups */
#define BRAP_P_MAX_AUDIO_DESC_OBJECTS           1
#endif

/* Error codes */
/***************************************************************************
Summary:
RAP error defines: Device not initialized.
****************************************************************************/
#define BRAP_ERR_DEVICE_UNINTIALIZED	BERR_MAKE_CODE(BERR_RAP_ID, 1)

/***************************************************************************
Summary:
RAP error defines: Fatal error, the device is in a bad state.
****************************************************************************/
#define BRAP_ERR_BAD_DEVICE_STATE	BERR_MAKE_CODE(BERR_RAP_ID, 2)

/***************************************************************************
Summary:
RAP error defines: Device resources exhausted 
****************************************************************************/
#define BRAP_ERR_RESOURCE_EXHAUSTED			BERR_MAKE_CODE(BERR_RAP_ID, 3)

/***************************************************************************
Summary:
RAP error defines: Output port(s) and associated properties are not configured.
****************************************************************************/
#define BRAP_ERR_OUTPUT_NOT_CONFIGURED		BERR_MAKE_CODE(BERR_RAP_ID, 4)

/***************************************************************************
Summary:
RAP error defines: Channel already opened.
****************************************************************************/
#define BRAP_ERR_CH_ALREADY_OPENED		BERR_MAKE_CODE(BERR_RAP_ID, 5)

/***************************************************************************
Summary:
RAP error defines: Channel already started.
****************************************************************************/
#define BRAP_ERR_CH_ALREADY_STARTED		BERR_MAKE_CODE(BERR_RAP_ID, 6)

/***************************************************************************
Summary:
RAP error defines: The requested Output port is not supported in this device.
****************************************************************************/
#define BRAP_ERR_OUPUT_PORT_NOT_SUPPORTED		BERR_MAKE_CODE(BERR_RAP_ID, 7)
    
/***************************************************************************
Summary:
RAP error defines: The link from the Source Channel to mixer is broken
****************************************************************************/
#define BRAP_ERR_INVALID_SRCCH_MIXER_CONNECTION		BERR_MAKE_CODE(BERR_RAP_ID, 8)
    
/***************************************************************************
Summary:
RAP error defines: Compressed data on SPDIF output port is not a supported option
for this algorithm
****************************************************************************/
#define BRAP_ERR_COMP_SPDIF_NOT_ALLOWED		BERR_MAKE_CODE(BERR_RAP_ID, 9)

/***************************************************************************
Summary:
RAP error defines: Valid Stream Info values are not avialable (yet not set by Firmware)
in the registers AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT{0-3}
****************************************************************************/
#define BRAP_ERR_STREAMINFO_NOT_AVAILABLE		BERR_MAKE_CODE(BERR_RAP_ID, 10)



/*}}}*/

/*{{{ Typedefs */



/*****************************************************************************
Summary:
	This enum describes Raptor device level interrupts.

Description:    
	These interrupts are for Raptor device as a whole. These interrupts 
	function across all modules and channels opened in the Raptor device.

See Also:
    BRAP_Interrupt
    BRAP_CallbackFunc
******************************************************************************/
typedef enum BRAP_DeviceLevelInterrupt
{
	BRAP_DeviceLevelInterrupt_eWatchdog,		/* Raptor watchdog interrupt */
	BRAP_Interrupt_eMaxDeviceLevelInterrupts	/* Place holder */
} BRAP_DeviceLevelInterrupt;

/*****************************************************************************
Summary:
    This enumeration lists the channel level interrupts that are currently handled in 
    Raptor PI.
    
Description:    
    In case of similar interrupts that occur for different DSP contexts, or 
    underflows for various RBufs etc, a common enum will be used.
    eg BRAP_Interrupt_eSrcChUnderflow will be the common enum used for
    SOURCE_FIFO_X_UNDERFLOW.

See Also:
    BRAP_DeviceLevelInterrupt
    BRAP_CallbackFunc    
******************************************************************************/
typedef enum BRAP_Interrupt
{
    /* TODO: list other interrupts */
	BRAP_Interrupt_eFmmRbufFreeByte,
    BRAP_Interrupt_eFmmDstRbufFullMark,
    BRAP_Interrupt_eFmmRbufUnderflow,
	BRAP_Interrupt_eFirstPtsReady,
	BRAP_Interrupt_ePtsError,
	BRAP_Interrupt_eTsmLog,
	BRAP_Interrupt_eDecLock,
	BRAP_Interrupt_eDecUnlock,
	BRAP_Interrupt_eSampleRateChange,
	BRAP_Interrupt_eBitRateChange,
	BRAP_Interrupt_eModeChange,
	BRAP_Interrupt_eCrcError,
	BRAP_Interrupt_eMpegLayerChange,	
    BRAP_Interrupt_eFmmCrc,	
    BRAP_Interrupt_eAstmTsmPass,       
    BRAP_Interrupt_eCdbItbUnderflow,
    BRAP_Interrupt_eCdbReady,    
    BRAP_Interrupt_eStreamInfoAvailable,       
    BRAP_Interrupt_eMaxInterrupts
} BRAP_Interrupt;


typedef struct BRAP_InterruptCount
{
	uint32_t    uiFmmDstRbufFullWaterMark;	
    uint32_t    uiPtsError;
    uint32_t    uiDecoderUnlock;    
    uint32_t    uiFmmCRC;
    uint32_t    uiCrcError;
    uint32_t    uiTotalFrameDecoded;
    uint32_t    uiFramesInError;
} BRAP_InterruptCount;

/*****************************************************************************
Summary:
Raptor Application callback function typedef. This is different from the 
BINT_CallbackFunc definition in that it allows for an extra parameter to be passed to the 
callback which contains Raptor specific data. All application ie user level interrupt callback 
functions have to be of this type.

    void * pParm1: Application specified parameter that needs to be passed 
                   unchanged to the callback
    int    iParm2: Application specified parameter that needs to be passed 
                   unchanged to the callback 
    void * pRap_data: Resevered for internal Raptor PI usage.

This can be used for callbacks to both the Device level and Channel level 
interrupts    

See Also:
    BRAP_DeviceLevelInterrupt
    BRAP_Interrupt   
******************************************************************************/
typedef void (*BRAP_CallbackFunc) (void *pParm1, int parm2, void *pRap_data);


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
	BRAP_BufferCfgMode_eStdAudioMode,	/* Standard Audio Mode (for algorithm like AC3, MPEG, AAC etc) */
	BRAP_BufferCfgMode_eAdvAudioMode,	/* Advanced Audio Mode (for algorithm like AAC-HE, DD+ etc)*/
	BRAP_BufferCfgMode_eIpStbMode,		/* IP Settop Box Mode */
	BRAP_BufferCfgMode_eHdDvdMode,		/* HD DVD Mode */
	BRAP_BufferCfgMode_eBlueRayMode,	/* Blue Ray Mode */
	BRAP_BufferCfgMode_eMaxMode
}BRAP_BufferCfgMode;

/***************************************************************************
Summary: 
	This enum defines the various downmix paths present in a decode channel

Description:
	In a decode channel, there can be multiple downmix paths. Each downmix path has
	its own output configuration parameters. Output configuration parameters are
	following.
	Output Mode
	Dual Mono Mode
	LFE ON/OFF
	Stereo Mode
	Downmix Coefficients
	Karaoke Level Pan Coefficients etc
	However, other decoder configurtion parameters (like Dynamic Range Compression 
	parameters, PCM scale factor etc ) are common for all the downmix paths. If 
	common parameters are changed for one downmix path then it affects other
	downmix paths as well.

See Also:	
****************************************************************************/
typedef enum BRAP_DEC_DownmixPath
{
	BRAP_DEC_DownmixPath_eMain,			/* Main downmix path. This path 
											    supports multiple channels */
	BRAP_DEC_DownmixPath_eStereoDownmix,  /* Stereo downmix path */
	BRAP_DEC_DownmixPath_eMax
} BRAP_DEC_DownmixPath;


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
	This Structure is used to return the DSP Firmware Algorithm
	version information.

Description:
	This structure contains the Major and Minor version, Date and Year 
	of DSP Firmware Algorithm.
	
See Also:
	BRAP_GetRevision  
	
****************************************************************************/
typedef struct BRAP_DSPVersionInfo
{
	
	BRAP_FwVersionInfo	sDecAlgoVersion[BRAP_MAX_DSP_DECODE_ALGO];	/* Decode Algorithm version information */
	BRAP_FwVersionInfo	sDecFsVersion[BRAP_MAX_DSP_FS_ALGO];		/* Frame Sync Algorithm version information */
	BRAP_FwVersionInfo	sPPAlgoVersion[BRAP_MAX_DSP_PP_ALGO];		/* Frame Sync Algorithm version information */
} BRAP_DSPVersionInfo;

/***************************************************************************
Summary:
	This Structure is used to return the Raptor Audio PI & firmware 
	version information.

Description:
	This structure contains the Major and Minor version, Date, Year and 
	Chip Id (if applicable) of both PI and firmwares.
	
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

/***************************************************************************
Summary:
	Structure to hold the device specific settings
	
Description:
	Currently the Audio module supports the option to use static memory 
	allocation wherein the application needs to pass a the start address 
	of the buffer and its size.
	
See Also:
	BRAP_GetDefaultSettings(), BRAP_Open(), BRAP_Close()
****************************************************************************/
typedef struct BRAP_Settings
{
#if (BRAP_SECURE_HEAP==1)
 	BMEM_Handle 			hSecureMemory;	/* [in] Secure Memory heap handle */
#endif
	bool 				bFwAuthEnable;	/* TRUE = All the supported audio codecs
										will be downloaded at BRAP_Open time.
										FALSE = The audio codec will be 
										downloaded at start time.*/
										
	bool			bExtDeviceMem;		/* TRUE = Application provides one
							   			chunk of device memory and RAP PI
							   			internally manages it.
							   			FALSE = Application doesn't provide
							   			device memory .RAP PI uses memory 
							   			manager to get device memory. */
							   			
	unsigned int	ui32MemBase; 		/* Device memory base to be used.
							   			This field is valid only when
							   			bExtDeviceMem = TRUE */
							   			
	size_t			ui32MemSize; 	/* Device Memory size in bytes. 
							   			This field is valid only when
							   			bExtDeviceMem = TRUE */
							   			
	size_t			ui32MaxRbufSize;/* Worst case size of a ring buffer in 
							   			bytes for all ring buffers to be used. 
							   			If this value is zero, then default 
							   			worst case ring buffer size is used 
							   			for allocating all ring buffers. */

	const BIMG_Interface *pImgInterface;       /* Interface to access firmware image 
									      	This interface	must be implemented 
									      	and the function pointers must be stored
									      	here. */
							   
	void                 **pImgContext;         /* Context for the Image Interface 
										This is also given by the implementation of 
										the Image interface */
							   
	bool		bSupportAlgos[BRAP_MAX_AUDIO_TYPES];         /* Select algorithms to be supported in RAP PI. Enum
        													BRAP_DSPCHN_AudioType is used as an index
        													in this array.
        													TRUE = Support algorithm corresponding to the 
        													given index in the array.
        													e.g. bSuppportAlgo[BRAP_DSPCHN_AudioType_eAc3] = true
        													means AC3 algorithm to be supported.
        													FALSE = Don't support algorithm corresponding 
        													to given index in the array.
       													 e.g. bSuppportAlgo[BRAP_DSPCHN_AudioType_eAc3] = false
        													means AC3 algorithm not to be supported. 
        													Device memory allocation will be done in RAP PI
        													according to the algorithms selected here. */
#if (BRAP_7401_FAMILY == 1) || ( BCHP_CHIP == 7400 )
	bool		bSupportPpAlgos[BRAP_MAX_PP_ALGOS];		/* Select post processing algorithms to be supported
														     in RAP PI. Enum BRAP_DSPCHN_PP_Algo is used
														     as an index in this array.
														     TRUE = Support algorithm corresponding to the
														     given index in the array.
														     e.g. bSupportPpAlgos[BRAP_DSPCHN_PP_Algo_eDdbm] = true
														     means Dolby Digital Bass Management algorithm to be
														     supported.
														     FALSE = Don't support algorithm corresponding to the
														     given index in the array.
														     e.g. bSupportPpAlgos[BRAP_DSPCHN_PP_Algo_eDdbm] = false
														     means Dolby Digital Bass Management algorithm not to be
														     supported. */
#endif
        bool bSupportENCAlgos[BRAP_MAX_ENC_AUDIO_TYPES];	/* Select encoding algos */

} BRAP_Settings;

/***************************************************************************
Summary:
	Structure to hold the output port related settings. 
	Some of the parameters can be changed on-the-fly ie while the channel is 
	running, while others can be changed only if the channel has been stopped.
	Refer to each BRAP_OP_XXXSettings description for further details.

See Also:
	BRAP_GetOutputDefaultConfig(), BRAP_SetOutputConfig()
****************************************************************************/
typedef struct BRAP_OutputSettings
{
	BRAP_OutputPort					eOutput;	/* Output type */

	union {										
		BRAP_OP_SpdifSettings	sSpdifSettings;	/* Spdif settings. It is used 
												   only if the output port is 
												   SPDIF */ 
		BRAP_OP_I2sSettings		sI2sSettings;	/* I2S settings. It is used 
												   only if the output port is 
												   I2S */ 
		BRAP_OP_FlexSettings	sFlexSettings;	/* Flex settings. It is used 
												   only if the output port is 
												   Flex */ 
		BRAP_OP_MaiSettings		sMaiSettings;	/* Mai settings. It is used 
												   only if the output port is 
												   MAI */ 
        BRAP_OP_DacSettings     sDacSettings;	/* Dac settings. It is used 
												   only if the output port is 
												   DAC */ 
	}uOutputPortSettings;						

	BAVC_Timebase		eOutputTimebase;		/* Timebase to use for the output port */

    BAVC_AudioSamplingRate 		eOutputSR;		/* Sampling rate of the output
                                                   port. This is used only for 
                                                   a PB/CAP channel. For a 
                                                   decode channel, the output 
                                                   sampling rate is determined 
                                                   by the DSP from the stream */		

    BRAP_OP_SpdifChanStatusParams sSpdifChanStatusParams;   /* Contains user 
                                                   parameters required for the
                                                   SPDIF channel status bits.  
                                                   Used only if output port is 
                                                   SPDIF, Mai or Flex */                                                   

    BRAP_SPDIFFM_Settings       sSpdiffmSettings;
                                                /* Spdiff FM settings. It is 
                                                   used only if output port is 
                                                   SPDIF, Mai or Flex */                                                          
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)
	bool bIndpDlyRqd; 	/* TRUE: This output ports may require to have and independent delay at some time.
                           FALSE: This output port will NEVER have independent delay wrt to the other ports*/



    int iDelay;         /* The independent delay to be applied to this  output 
                                  port in units of milliseconds. 
                                  Note: This delay value is used only if the 
                                  channel is opened with bSelectDelay as TRUE  */                             
#endif
}BRAP_OutputSettings;

/***************************************************************************
Summary:
	Parameters specific to the FMM Output path which are required for starting 
	a Raptor Channel

Description:
	This structure contains various input parameters specific to the output 
	blocks such as Mixer, Spdif Formater and Output port
	
See Also:
	BRAP_DEC_AudioParams, BRAP_PB_AudioParams, BRAP_CAP_AudioParams
	BRAP_DEC_GetDefaultAudioParams(), BRAP_DEC_Start()
	BRAP_PB_GetDefaultAudioParams(), BRAP_PB_Start()
	BRAP_CAP_GetDefaultAudioParams(), BRAP_CAP_Start()	
	
****************************************************************************/
typedef struct BRAP_AudioOutputParams
{
	unsigned int  uiOutputBitsPerSample; /* Bits per sample to be 
										 output at the output port.
										 The legal values for 
										 this field are:
										 16 to 32, for I2S outputs
										 16 to 24, for all other outputs */
 
	BRAP_MIXER_Params   sMixerParams;   /* Mixer Parameters */
   
	BRAP_SPDIFFM_Params sSpdifFmParams; /* SPDIF Formater Parameters */
 
} BRAP_AudioOutputParams;

/***************************************************************************
Summary:
	This structure holds the settings required to open a Raptor Decode Channel.

See Also:
	BRAP_DEC_GetChannelDefaultSettings(), BRAP_DEC_OpenChannel()
****************************************************************************/
typedef struct BRAP_DEC_ChannelSettings
{
	unsigned int		uiWaterMark;		/* Free byte water mark in percentage. 
											   When the percentage of free space 
											   in the ring buffers become less 
											   than this, interrupt is generated 
											   by the hardware. */
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)
   bool                    bLargeRbuf;     /* TRUE: Output ports connected to this 
                                            channel require larger 
                                            rbufs in order to support independent delay.
                                            FALSE: Normal RBUF sixe will be used 
                                            => No output port connected to this 
                                            channel can have an independent delay */
#endif

	BRAP_DSPCHN_Settings	sDspChSettings[BRAP_DEC_DownmixPath_eMax];	/* DSP Channel Settings */
	
}BRAP_DEC_ChannelSettings;

/***************************************************************************
Summary:
	This structure contains  input parameters required to start a Raptor Decode 
	Channel. These parameters are essentially the some stream details, output 
	mode selection etc.

See Also:
	BRAP_DEC_GetDefaultAudioParams(), BRAP_DEC_Start(), BRAP_DEC_Stop()
	
****************************************************************************/
typedef struct BRAP_DEC_AudioParams
{
	BAVC_Timebase		eTimebase;		/* Timebase to use for decoder */

	BRAP_DSPCHN_AudioParams	sDspChParams;/* DSP Context specific parameters */
	
	BRAP_AudioOutputParams	sAudioOutputParams;	/* Parameters specific to the 
											output blocks such as Mixer, Spdif 
											Formater and Output port etc. This 
											structure is used for Decode and 
											Pass-thru mode and for the Decode 
											context of Simultaneous mode. */

	BRAP_AudioOutputParams	sSimulPtAudioOutputParams; /* This structure is 
											used for the pass-thru context 
											of the simultaneous mode or the 
                                            clone output port in Decode mode. It  
											contains parameters specific to the 
											output blocks such as Mixer, Spdif 
											Formater and Output port etc. */

    BRAP_OP_SpdifChanStatusParams sSpdifChanStatusParams;   /* This is used 
                                            only when SPDIF/HDMI output port is 
                                            configured for this audio channel.
                                            It contains user parameters required
                                            for the SPDIF channel status bits*/

	BAVC_XptContextMap	sXptContextMap;	 /* This structure holds the CDB and 
											ITB register addresses for the 
											corresponding Rave Context 
											allocated by the Xpt PI.*/

	bool	bPlayback;                   /* TRUE: input transport channel
                                        	is a playback channel
                							FALSE: input transport channel
                							is a broadcast channel */ 
#if (BRAP_AD_SUPPORTED == 1)
    BRAP_AudioDescriptorFadeHandle hAdFade;     
                                        /* Audio descriptor fade table handle. This field should be set
                                           only if this channel belongs to audio descriptor channel 
                                           pair. Application should send the same audio
                                           descriptor handle to both (audio descriptor channel and
                                           primary channel) channels in an audio descriptor channel
                                           pair. For non audio descriptor channel this field should be
                                           set to NULL. */ 
#endif
#ifdef RAP_SRSTRUVOL_CERTIFICATION
	BAVC_AudioSamplingRate 			eInputSamplingRate; /*used for SRS certification. Input sample rate of PCM samples.*/
	unsigned int					uiNumChannels;
#endif
									/* Sampling rate of PCM Input */	
}BRAP_DEC_AudioParams;


typedef struct BRAP_PP_ConfigParams
{	
	unsigned int				uiPpBranchId;	
	unsigned int				uiPpStage;	
	BRAP_DSPCHN_PP_Algo	ePpAlgo;	
	union	
	{	
		BRAP_DSPCHN_DolbyVolumeConfigParams   sDolbyVolumeConfigParams;          /* Dolby Volume config params */	
		BRAP_DSPCHN_SRSTruVolConfigParams   sSRSTruVolConfigParams;          /* SRS TruVol Config params */	
		BRAP_DSPCHN_ADPanCtrlConfigParams   sAdPanConfigParams;          /* AD Pan Control config params */
		BRAP_DSPCHN_ADFadeCtrlConfigParams  sAdFadeConfigParams;        /* AD Fade Control config params */
	} uConfigParams;
} BRAP_PP_ConfigParams;



/***************************************************************************
Summary:
	This structure contains decoder configuration parameters that can be
	changed on the fly.
	
See Also:
	BRAP_DEC_SetConfig()
	BRAP_DEC_GetDefaultConfig()
	BRAP_DEC_GetCurrentConfig()
****************************************************************************/
typedef struct BRAP_DEC_ConfigParams
{
	BRAP_OutputMode	eOutputMode;	/* Output mode */
	BRAP_DSPCHN_DualMonoMode eDualMonoMode;	/* Dual mono mode, applies only when
									   stream has two independent mono
									   channel */
    bool                    bMonoToAll;	 /* TRUE: Downmixed mono data is routed 
                            to all speakers in the system.
                            Output mode must be 1_0 for this to take effect.*/			   
                                       
	BRAP_DSPCHN_AudioType	eType;	/* Audio type for which this structure
									   is getting initialized */
	BRAP_DEC_DownmixPath	eDownmixPath;	/* Decode path to which the output mode
											    parameters are applied. This is valid only
												for 7400 and not for 7401 */
#if (BRAP_AD_SUPPORTED == 1) || (BRAP_DOLBYVOLUME_SUPPORTED == 1) || (BRAP_SRS_TRUVOL_SUPPORTED == 1)
	BRAP_PP_ConfigParams		sPpConfigParams;
#endif

	union 
	{
		BRAP_DSPCHN_Ac3ConfigParams		sAc3ConfigParams; /* AC3 config params */
		BRAP_DSPCHN_Ac3PlusConfigParams			sAc3LosslessConfigParams;	/* AC3_LOSSLESS config params */
		BRAP_DSPCHN_Ac3PlusConfigParams		sAc3PlusConfigParams; /* AC3_PLUS config params */
		BRAP_DSPCHN_AacConfigParams		sAacConfigParams; /* AAC config params */
		BRAP_DSPCHN_AacSbrConfigParams		sAacSbrConfigParams;  /* AAC SBR config params */
		BRAP_DSPCHN_DtsConfigParams			sDtsConfigParams; /* DTS config params */
#if (BRAP_DTS_SUPPORTED == 1)		
		BRAP_DSPCHN_DtsCoreConfigParams			sDtsCoreConfigParams; /* DTS Core config params - Used for 7401 and 7403*/
#endif        
   		BRAP_DSPCHN_DtshdConfigParams		sDtshdConfigParams;  /* DTS HD config params */
		BRAP_DSPCHN_BdlpcmConfigParams		sBdlpcmConfigParams;	/* BD_LPCM config params */	
        	BRAP_DSPCHN_LpcmHdDvdConfigParams	sLpcmHdDvdConfigParams;	  /* HD_DVD_LPCM config params */       
		BRAP_DSPCHN_LpcmDvdConfigParams	sLpcmDvdConfigParams;	       /* DVD_LPCM config params */    
		BRAP_DSPCHN_MlpConfigParams			sMlpConfigParams;			/* MLP config params */
		BRAP_DSPCHN_WmaProConfigParams		sWmaProConfigParams;	
		BRAP_DSPCHN_WmaStdConfigParams		sWmaStdConfigParams;		
		BRAP_DSPCHN_DraConfigParams		sDraConfigParams;
        BRAP_DSPCHN_PcmWavConfigParams  sPcmWavConfigParams;                
	} uConfigParams;				/* Decoder configuration parameters */

	BRAP_DSPCHN_DdbmConfigParams			sDdbmConfigParams;		/* DDBM config params */									   
} BRAP_DEC_ConfigParams;


/***************************************************************************
Summary:
	Channel specific settings required to open a Raptor PCM playback channel
	
Description:
	Structure to hold the channel specific settings for a PCM playback 
	channel. This structure is used while opening a PCM playback Audio 
	Channel.

	Note: The uiWaterMark element of BRAP_RBUF_Settings structure is used as
          a free byte water mark for PCM playback channel. Free watermark 
          percentage, say x, implies that if free number of bytes in the 
          associated ring buffer(s) exceeds x% of total buffer size, a ring 
          buffer free water mark interrupt is generated.
See Also:
	BRAP_PB_GetChannelDefaultSettings(), BRAP_PB_OpenChannel() 

****************************************************************************/
typedef struct BRAP_PB_ChannelSettings
{
	BRAP_OutputPort				eOutputPort;	
									/* Output port for LR channels */

	BRAP_BufDataMode 			eBufDataMode; 	
									/* PCM Data Mode. This will decide the 
									   number of ring buffers required to be 
									   allocated */
									   
    BRAP_RBUF_Settings 			sLtOrSnglRbufSettings;
									/* Left or single ring buffer settings 
									   structure. This will be used for all 
									   ePcmDataMode that need either 1/2 buffers */

    BRAP_RBUF_Settings 			sRtRbufSettings; 
									/* Right ring buffer settings structure. 
									   This will be used only when ePcmDataMode 
									   indicates stereo non-interleaved PCM 
									   type that needs 2 buffers */
}BRAP_PB_ChannelSettings;

/***************************************************************************
Summary:
	Parameters for starting a Raptor PCM playback channel
	
Description:
	This structure contains various input parameters required for starting 
	the PCM playback operation for an Audio Channel. These parameters are 
	essentially the stream specific parameters / parameters specific to any 
	other operations to be done for this channel operation.
	
See Also:
	BRAP_PB_GetDefaultAudioParams(), BRAP_PB_Start(), BRAP_PB_Stop()

****************************************************************************/
typedef struct BRAP_PB_AudioParams
{
	BRAP_AudioOutputParams 			sAudioOutputParams;	
									/* Parameters specific to the Output 
									   blocks such as Mixer, Spdif  Formater 
									   and Output port etc. */

	unsigned int    				uiDelayRbuf;    
									/* Starting delay in micro-seconds. This 
    							   	   parameter is internally converted to
    							   	   mark the start of read point in the 
    							   	   ring buffer. This can be used to 
    							   	   achieve lip-sync. 
    							   	   Note: The same delay will be applicable 
    							   	   to both the ring buffers for a stereo
    							   	   non-interleaved PCM data mode indicated
    							   	   by eBufDataMode */

	size_t   						ui32RbufMemSize;    
									/* Ring buffer memory size in bytes for 
									   left or mono ring buffer. It should be 
									   less than or equal to the ring buffer 
									   size provided during channel open time 
									   i.e. ui32RbufMemSize of 
									   BRAP_PB_ChannelSettings. 
    							   	   Note: The same buffer size will be 
    							   	   applicable to both the ring buffers for 
    							   	   a stereo non-interleaved PCM data mode 
    							   	   indicated by eBufDataMode */

	BRAP_OP_SpdifChanStatusParams 	sSpdifChanStatusParams;   
									/* This is used only when SPDIF/HDMI output 
									   port is configured for this audio 
									   channel. It contains user parameters 
									   required for the SPDIF channel status 
									   bits*/

	BAVC_AudioSamplingRate 			eInputSamplingRate; 
									/* Sampling rate of PCM Input */		

	BRAP_InputBitsPerSample			eInputBitsPerSample;
									/* Input bits per sample of PCM data*/
									
    bool  							bLoopAroundEn;   
									/* TRUE: Play the data in ring buffer 
									   over and over again. In this mode, the 
	        						   application can write the ring buffer 
	        						   once and then start the playback channel, 
        							   so that the data can be played back over 
        							   and over again. FALSE: Otherwise */

    BRAP_PcmOutputMode			    ePcmOutputMode;	
									/* Output Mode: Number of output 
									   channels to be used are determined 
									   from this. */

	BRAP_BufDataMode 			    eBufDataMode; 	
									/* PCM Data Mode. This will decide the 
									   number of ring buffers to be used. 
									   This should be compatible with 
									   eBufDataMode passed during channel 
									   open */
#if BRAP_UNSIGNED_PCM_SUPPORTED
    bool                            bIsSigned;
                                    /* PCM Data Format. 
                                       TRUE: The PCM data is signed
                                       FALSE: The PCM data is unsigned */
#endif
}BRAP_PB_AudioParams;

/***************************************************************************
Summary:
	Channel specific settings required to open a Raptor PCM capture channel
	
Description:
	Structure to hold the channel specific settings for a PCM capture 
	channel. This structure is used while opening a PCM capture Audio 
	Channel.
	
See Also:
	BRAP_CAP_GetChannelDefaultSettings(), BRAP_CAP_OpenChannel()

****************************************************************************/
typedef struct BRAP_CAP_ChannelSettings
{
	BRAP_CaptureMode	eCapMode;		/* Capture mode: Whether this channel 
										   is used for capture only or for 
										   full duplex or bypass mode */

	BRAP_OutputPort		eOutputPort;	/* Output port for playback. This 
										   is ignored if eCapMode ==  
										   BRAP_CAP_CaptureMode_eCaptureOnly */										   
	BRAP_BufDataMode	eMaxBufDataMode;/* Max PCM Data Mode. It defines 
										   how the capture PCM data will be stored in 
										   the ring buffers. This will decide 
										   whether 1 or 2 ring buffers 
										   required to be allocated. It is 
										   ignored if eCapMode ==  
										   BRAP_CaptureMode_eByPass 
										   This is used to allocate resources.*/
	BRAP_RBUF_Settings	sLtOrSnglRbufSettings; /* Left or single ring buffer 
										   settings structure. This will be 
										   used for all ePcmDataMode that need 
										   either 1/2 buffers. This variable is 
										   ignored if eCapMode ==  
										   BRAP_CaptureMode_eByPass */
	BRAP_RBUF_Settings	sRtRbufSettings;/* Right ring buffer settings structure. 
										   This will be used only when ePcmDataMode 
										   indicates stereo non-interleaved PCM 
										   type that needs 2 buffers. This 
										   variable is ignored if eCapMode ==  
										   BRAP_CaptureMode_eByPass */

}BRAP_CAP_ChannelSettings;

/***************************************************************************
Summary:
	Parameters for starting a Raptor PCM Capture channel
	
Description:
	This structure contains various input parameters required for starting 
	the PCM capture operation for an Audio Channel. These parameters are 
	essentially the stream specific parameters / parameters specific to any 
	other operations to be done for this channel operation.
	
See Also:
	BRAP_CAP_GetDefaultAudioParams(), BRAP_CAP_Start(), BRAP_CAP_Stop()

****************************************************************************/
typedef struct BRAP_CAP_AudioParams
{

	BRAP_CapInputType	eCapInputType;	/* Type of PCM Capture Input Source */
    
	BRAP_BufDataMode	eBufDataMode;	/* PCM Data Mode. It defines how the 
										   capture PCM data will be stored in 
										   the ring buffers. This will decide 
										   whether 1 or 2 ring buffers are to be
										   used. This is ignored in
										   BRAP_CaptureMode_eByPass 
										   mode. Should be less than or equal to
										    the Max mode specified at Channel 
										    Open */									   
    BRAP_PcmOutputMode    ePcmOutputMode;	
									/* Output Mode: Number of output 
									   channels to be used are determined 
									   from this. */


	unsigned int		uiDelayRbuf; /* Starting delay in micro-seconds. This 
    							   	   parameter is internally converted to
    							   	   mark the start of read point in the 
    							   	   ring buffer. This can be used to 
    							   	   achieve lip-sync. 
    							   	   Note: The same delay will be applicable 
    							   	   to both the ring buffers for a stereo
    							   	   non-interleaved PCM data mode indicated
    							   	   by eBufDataMode
    							   	   This is used only in BRAP_CAP_CaptureMode_eFullDuplex
							          mode*/

	size_t				ui32RbufMemSize;    /* Ring buffer memory size in bytes for 
									   left or mono ring buffer. It should be 
									   less than or equal to the ring buffer 
									   size provided during channel open time 
    							   	   Note: The same buffer size will be 
    							   	   applicable to both the ring buffers for 
    							   	   a stereo non-interleaved PCM data mode 
    							   	   indicated by eBufDataMode. This is ignored in
										   BRAP_CaptureMode_eByPass */

	BAVC_AudioSamplingRate 			eInputSamplingRate; 
                                       /* Sampling rate of PCM Input. Note: This 
                                       param will be used only when eCapInputType 
                                       is BRAP_CapInputType_eExtI2s. For all other
                                       internal capture input types, input sampling 
                                       rate will be derived internally from the 
                                       captured output port's output sampling rate */
                                       
	BRAP_InputBitsPerSample			eInputBitsPerSample;
                                       /* Input bits per sample of PCM data
                                       Note: This param will be used only when 
                                       eCapInputType is BRAP_CapInputType_eExtI2s. 
                                       For all other internal capture input types, 
                                       input bits/sample will be derived internally 
                                       from the captured output port's output 
                                       bits/sample */                                    

	BRAP_DSTCH_Params	sDstChParams;/* Parameters requires for Destination 
									   channel start */

	BRAP_AudioOutputParams 	sAudioOutputParams;/* Parameters specific to the Output 
									   blocks such as Mixer, Spdif  Formater 
									   and Output port etc. This is ignored in
										   BRAP_CaptureMode_eCaptureOnly mode*/									   

	BRAP_OP_SpdifChanStatusParams 	sSpdifChanStatusParams;  /* This is used 
                                       only when SPDIF/HDMI output 
									   port is configured for this audio 
									   channel. It contains user parameters 
									   required for the SPDIF channel status 
									   bits. This is ignored in
										   BRAP_CaptureMode_eCaptureOnly mode*/
#if BRAP_UNSIGNED_PCM_SUPPORTED
    bool                            bIsSigned;
                                    /* PCM Data Format. 
                                       TRUE: The PCM data is signed
                                       FALSE: The PCM data is unsigned 
                                       This is ignored in
									   BRAP_CaptureMode_eCaptureOnly mode */
#endif
}BRAP_CAP_AudioParams;

#ifdef RAP_SRSTRUVOL_CERTIFICATION  

typedef struct BRAP_BufInfo
{
    unsigned int uiSize; /* Size of the Buffer*/
    unsigned int LeftDramBufferBaseAddr; /* Base Address of Left PCM Buffer*/
    unsigned int LeftDramBufferReadAddr; /* Read Address of Left PCM Buffer*/
    unsigned int LeftDramBufferWriteAddr; /* Write Address of Left PCM Buffer*/
    unsigned int LeftDramBufferEndAddr;    /* End Address of Left PCM Buffer*/
    unsigned int RightDramBufferBaseAddr; /* Base Address of Right PCM Buffer*/
    unsigned int RightDramBufferReadAddr; /* Read Address of Right PCM Buffer*/
    unsigned int RightDramBufferWriteAddr;/* Write Address of Right PCM Buffer*/
    unsigned int RightDramBufferEndAddr;  /* End Address of Right PCM Buffer*/      
	bool bLeftRdWrapAround;
	bool bLeftWrWrapAround;    
	bool bRightRdWrapAround;
	bool bRightWrWrapAround;     
}BRAP_BufInfo;
#endif


/**********************************************************************
Summary:
	Enum describe the polarity to set in CRC register before starting the CRC
***********************************************************************/
typedef enum BRAP_CrcPolarity
{
	BRAP_CrcPolarity_0=0,
	BRAP_CrcPolarity_1
}BRAP_CrcPolarity;


#if (BRAP_AD_SUPPORTED == 1)

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

#endif
/*}}}*/


/*{{{ Function prototypes */

/***************************************************************************
Summary:
	Gets the default settings for Raptor Audio device.
	
Description:
	This API returns the default settings of the Raptor Audio device. These can 
	be modified as required and used to open the Raptor Device
	
Returns:
	BERR_SUCCESS 

See Also:
	BRAP_Open()
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
	A handle to the Raptor Audio Device is returned by this API, 
	which represents the instance of this device. This Device handle needs 
	to be passed to PIs operating at device level.
	This API must be called prior to any other Raptor PI call requiring a 
	device handle.	
	Note that this PI alone is not sufficient to get audio.
    Raptor audio channels will have to be opened and started individually.
	
Returns:
	BERR_SUCCESS on success, error on failure

See Also:
	BRAP_GetDefaultSettings(), BRAP_Close()
	BRAP_DEC_OpenChannel(), BRAP_DEC_Start()
	BRAP_PB_OpenChannel(), BRAP_PB_Start()
	BRAP_CAP_OpenChannel(), BRAP_CAP_Start()	
***************************************************************************/
BERR_Code BRAP_Open(
	BRAP_Handle 			*phRap,			/* [out] The RAP handle returned*/
	BCHP_Handle 			hChip,			/* [in] Chip handle */
	BREG_Handle				hRegister,		/* [in] Register handle */
	BMEM_Handle 			hMemory,		/* [in] Memory heap handle */
	BINT_Handle				hInt,			/* [in] Interrupt module handle */
	BTMR_Handle			    hTmr,			/* [in]Timer modules handle */
	const BRAP_Settings		*pSettings		/* [in] Device settings */
	);


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

/***************************************************************************
Summary:
	Closes the Raptor Audio Device.

Description:
	This function should be called when the application is exiting or the 
	application doesn't intend to use Raptor Audio PI further. This call 
	destroys the instance of the Raptor Audio Device handle. 

	Note: all Raptor channels must be stopped and closed before this PI is 
	called.

Returns:
	BERR_SUCCESS on success, error on failure

See Also:
	BRAP_Open()
	BRAP_DEC_Stop(), BRAP_DEC_CloseChannel()
	BRAP_PB_Stop(), BRAP_PB_CloseChannel()
	BRAP_CAP_Stop(), BRAP_CAP_CloseChannel()	
***************************************************************************/
BERR_Code BRAP_Close(
	BRAP_Handle 			hRap	/* [in] The Raptor Audio device handle*/
	
	);

/***************************************************************************
Summary:
	API returns revision information.

Description:
	Gets the revision information of all the firmwares executing on Raptor 
	DSP and FMM hardware as well as the revision information of the Raptor 
	Audio PI.
	
Returns:
	BERR_SUCCESS 

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


/***************************************************************************
Summary:
	Sends out a pregenerated, 1KHz test tone on the I2S and SPDIF interfaces

Returns:
	BERR_SUCCESS on success
	Error value on failure

Note:
	Before calling this PI the Raptor device should have been opened, and all 
	Raptor channels closed. This PI does not save / restore state of any open
	channels.

See Also:
	BRAP_TestToneDisable
****************************************************************************/
BERR_Code BRAP_TestToneEnable(
     BRAP_Handle 			hRap		/* [in] The RAP device handle */
);

/***************************************************************************
Summary:
	Stops the test tone started by BRAP_TestToneEnable on the I2S and SPDIF interfaces
	
Returns:
	BERR_SUCCESS on success
	Error value on failure

Note:
	Before calling this PI the Raptor device should have been opened, and all 
	Raptor channels closed. This PI does not save / restore state of any open
	channels.
	
See Also:
	BRAP_TestToneEnable
****************************************************************************/
BERR_Code BRAP_TestToneDisable (
     BRAP_Handle 			hRap		/* [in] The RAP device handle */
);


/***************************************************************************
Summary:
	This API returns the default settings for a particular output port.
	
Returns:
	BERR_SUCCESS  on success,
	error value on failure
	
See Also:
	BRAP_SetOutputConfig(), BRAP_GetOutputConfig()
***************************************************************************/
BERR_Code BRAP_GetOutputDefaultConfig(
	BRAP_Handle 		hRap,			/* [in] The RAP device handle */
	BRAP_OutputPort		eOutput,		/* [in] Output port */
	BRAP_OutputSettings *pDefSettings	/* [out] Default output settings */
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

    1. 'start-time only parameters: can be reprogrammed can be reprogrammed every 
    time an output port is stopped and restarted. These can be modified only when 
    the output port is stopped. and will take effect the next time the output port 
    is started. Note that an output port may be in use by mutliple channels.
    => an output port is 'stopped' when ALL the channels using it are stopped.


    2. 'on-the-fly parameters: can be reprogrammed any time ie even while the 
    output port is running, without stopping and restarting the channel.

    This is a device level PI. The configuration paramters will take effect for 
    ALL channels that are currently using this output port.

    Refer to the description of the output configuration paramters for details 
    on which of them are start-time paramters and which can be changed 
    on-the-fly.  

Returns:
	BERR_SUCCESS on success
	BRAP_ERR_OUPUT_PORT_NOT_SUPPORTED if the output port is not supported
	BERR_NOT_SUPPORTED if there is an attempt to change the start-time 
	paramters while ie output port is running - ie if the output port is in use 
	by ANY running channel

See Also:
	BRAP_GetOutputDefaultConfig(), BRAP_GetOutputConfig()
****************************************************************************/
BERR_Code BRAP_SetOutputConfig( 
	BRAP_Handle 				hRap,				/* [in] The Raptor Audio device handle*/
	const BRAP_OutputSettings	*pOutputSettings	/* [in] Output port settings*/ 
	);

/***************************************************************************
Summary:
	API used to get the current output port configuration.

Description:
	This API is used to get the output configuration of an output port. 
	If the caller has not configured the output port before, this API 
	will return error. 

Returns:
	BERR_SUCCESS on success, error on failure

See Also:
	BRAP_SetOutputConfig(), BRAP_GetOutputDefaultConfig()
****************************************************************************/
BERR_Code BRAP_GetOutputConfig( 
	BRAP_Handle 		hRap,			/* [in] The Raptor Audio device handle*/
	BRAP_OutputPort		eOutput,		/* [in] Output port */
	BRAP_OutputSettings	*pOutputSettings/* [out] Output port settings*/ 
	);


/***************************************************************************
Summary:
	Add an output port to an existing Channel. 

Description:
o  Note that output ports can be specified directly at a channel open.
    But if the application requires to add an output port after the channel
    has already been opened, it should use this PI.

o  This PI is channel specific and that data from many channels 
       may be mixed at an output port.

o  This PI reserves and opens the internal resources required to associate an output port 
    with a channel.

o  This PI only "links" the resources for an output port to the channel.  
    It does not cofigure the port properties nor does it start data flow to the port.
    Therefore, the following apply
            1) The output port should have been configured beforehand by calling 
            BRAP_SetOutputConfig()
            2) This PI can be called only AFTER the channel has been opened.
            3) This PI has to be called BEFORE starting the channel for the  
                addition to take effect. This channel's data will starting 
                flowing on this output port only after an explicit call to 
                BRAP_DEC_Start() [or BRAP_PB_Start()/BRAP_CAP_Start() for 
                playback/capture channels]

o  An output port added by calling BRAP_AddOutputPort() is at par with other 
    output ports that were specified for this channel in the channel open. 
    All output ports port - irrespective of whether specified at channel open 
    or by calling BRAP_AddOutputPort() are treated alike. 

o  If the New output port is already in use by this channel, calling this PI 
    will return an error.

o  If the current channel is a decoded channel, and the New output port is 
    already in use by another decoded channel, this PI will return an error, 
    since we cannot mix two decoded channels. However, we can mix a decoded 
    channel with playback/capture channels.   

o  A decode channel behaves differently depending on the mode in which it is started

    In Decode mode the channel will carry only PCM data. This PCM data will be 
    sent out on all output ports associated with this channel.
    
    In PassThru mode the channel will carry only Compressed data. 
    This Compressed data will be sent out on all output ports associated with this channel    
    
    In Simul mode the channel carries both PCM as well as Compressed data. 
    The data flow/path for the compressed data in Simul mode is different and 
    requires special treatment. The application has to reserve resources 
    apriory for this. 
    BRAP_DEC_OpenChannel() this is done by specifying bSimulModeConfig = TRUE
    and eSimulModePtOutput.
    Similarly for this PI the application has to set bSimulPt=TRUE to reserve 
    this port for the Compressed context in Simul mode.

Returns:
	BERR_SUCCESS if successful 
	else error value.

See Also:
    BRAP_RemoveOutputPort() - to delink/remove an output port from this channel
    BRAP_GetOutputDefaultConfig(), BRAP_SetOutputConfig(),  BRAP_GetOutputConfig() - to set/get the output port configuration
    BRAP_DEC_OpenChannel(), BRAP_PB_OpenChannel(), BRAP_CAP_OpenChannel() - to open channels
    BRAP_DEC_Start(), BRAP_PB_Start(), BRAP_CAP_Start() - to start a channel


Usage:
In the most common usage mode, the channel would be opened with DAC specified as the LR output port.
With respect to SPDIF usage, there are two possibilities - either it is reserved for Simul mode at channel open time.. or not
Both are discussed below.

Step 1: 
    Configure all output ports that may be used. This doesnt Open or start the port. It simply saves the configuration to be used
    later when the channel/output port is actually started.


    BRAP_SetOutputConfig (hRap, <config for DAC>);
    BRAP_SetOutputConfig (hRap, <config for SPDIF>);
    BRAP_SetOutputConfig (hRap, <config for I2S0>);    

Step 2:
    Open the channel.

    o Nothing is reserved for Simul mode ie bSimulModeConfig == false.    

    BRAP_DEC_OpenChannel (hRap, &hRapCh, <settings>); // Where settings 
            have eOutputLR = DAC0, bSimulModeConfig = false, eSimulModePtOutput = Doesnt matter 

OR 

    o SPDIF is reserved for Simul mode ie bSimulModeConfig == true.    

    BRAP_DEC_OpenChannel (hRap, &hRapCh, <settings>); // Where settings 
            have eOutputLR = DAC0, bSimulModeConfig = true, eSimulModePtOutput = SPDIF            

Step 3: 
    Add output ports as required and start the channel
    
    i) To Add SPDIF - to carry compressed data if used in Simul mode.
        BRAP_AddOutputPort(rhRapCh,<bSimulPt = true>, BRAP_OutputChannelPair_eLR, BRAP_OutputPort_eSpdif);
        Note: if channel was opened with bSimulModeConfig = true, eSimulModePtOutput = SPDIF and you try to 
        add SPDIF again, BRAP_AddOutputPort() will return an error saying this output port is already linked with the channel.

        Start channel:
        
            o   BRAP_DEC_Start (hRapCh, <paramters>) // where paramters have Mode= decode
                => You will get PCM audio only on the DAC. no audio on SPDIF (since SPDIF has been reserved for compressed data in SimulMode)

                OR
                
            o   BRAP_DEC_Start (hRapCh, <paramters>) // where paramters have Mode= Simul mode
                => You will get PCM audio on the DAC and compressed data on SPDIF
                
    ii) To Add I2S 0     
        BRAP_AddOutputPort(rhRapCh,<bSimulPt = false>, BRAP_OutputChannelPair_eLR, BRAP_OutputPort_eI2s0);

        Note: if you make bSimulPt = true for I2S (or other ports that can carry only PCM data) the PI will return an error 

        Start channel:
        
            o   BRAP_DEC_Start (hRapCh, <paramters>) // where paramters have Mode= decode
                => You will get PCM audio on the DAC + I2S0. No audio on SPDIF (since SPDIF has been reserved for compressed data in SimulMode)

                OR
                
            o   BRAP_DEC_Start (hRapCh, <paramters>) // where paramters have Mode= Simul mode
                => You will get PCM audio on the DAC + I2S0 and compressed data on SPDIF                  

    iii) If SPDIF is configured for compresses data (either by calling AddOutputPOrt() or by setting bSimulModeConfig = true in BRAP_DEC_Open) 
    and you want to switch to PCM

            BRAP_DEC_Stop (hRapCh);
            BRAP_RemoveOutputPort (hRapCh, BRAP_OutputPort_eSpdif)
            BRAP_AddOutputPort(rhRapCh,<bSimulPt = false>, BRAP_OutputChannelPair_eLR, BRAP_OutputPort_eSpdif); //Add SPDIF with simulPt == false
            BRAP_DEC_Start (hRapCh, <paramters>) // where paramters have Mode= decode

    iv) If SPDIF is configured for PCM data and you want to switch to compressed

            BRAP_DEC_Stop (hRapCh);
            BRAP_RemoveOutputPort (hRapCh, BRAP_OutputPort_eSpdif)
            BRAP_AddOutputPort (hRapCh,<bSimulPt = true>, BRAP_OutputChannelPair_eLR, BRAP_OutputPort_eSpdif); //Add SPDIF with simulPt == true
            BRAP_DEC_Start (hRapCh, <paramters>) // where paramters have Mode= Simul mode

    
    v) If the channel was opened with DAC as the main output port, but you want to start the channel 
    in PassThrout mode with compressed data on SPDIF and no data on DAC

            BRAP_DEC_Stop (hRapCh);
            BRAP_RemoveOutputPort (hRapCh, BRAP_OutputPort_eDac0)
            BRAP_AddOutputPort(rhRapCh,<bSimulPt = false>, BRAP_OutputChannelPair_eLR, BRAP_OutputPort_eSpdif); //Add SPDIF with simulPt == false
            BRAP_DEC_Start (hRapCh, <paramters>) // where paramters have Mode= PassThru

    vi) Hypothetical situation with two channels, Channel1 and Channel2, and many changes of output ports
    Disclaimer: Some of the output ports and audio modes used in this example are not yet supported in Raptor.
    
    - Configure all output ports (irrespective of which channel uses them)            
            BRAP_SetOutputConfig (hRap, <config for DAC0>);
            BRAP_SetOutputConfig (hRap, <config for DAC1>);
            BRAP_SetOutputConfig (hRap, <config for SPDIF>);
            BRAP_SetOutputConfig (hRap, <config for I2S0>);    
            BRAP_SetOutputConfig (hRap, <config for I2S1>);                
            BRAP_SetOutputConfig (hRap, <config for Mai - with Flex>);    

    - Open and start Channel1 for stereo sound with PCM data going out on DAC + Mai and compressed data on SPDIF
            BRAP_DEC_OpenChannel (hRap, &hRapCh1, <settings>); // Where settings 
                have eOutputLR = DAC0, bSimulModeConfig = false, eSimulModePtOutput = Doesnt matter 
            BRAP_AddOutputPort (hRapCh1,<bSimulPt = false>, BRAP_OutputChannelPair_eLR, BRAP_OutputPort_eMai); // Add Mai for L/R PCM data
            BRAP_AddOutputPort (hRapCh1,<bSimulPt = true>, BRAP_OutputChannelPair_eLR, BRAP_OutputPort_eSpdif); //Add SPDIF with simulPt == true
            BRAP_DEC_Start (hRapCh1, <paramters>) // where paramters have Mode= Simul mode    

            => L/R data on DAC+MAI, compressed on SPDIF

    -User1 decides to switch to 3/2.1 analog out  on Channel1   
            BRAP_DEC_Stop (hRapCh1);
            BRAP_AddOutputPort (hRapCh1,<bSimulPt = false>, BRAP_OutputChannelPair_eLRSurround, BRAP_OutputPort_eI2s0); // Add I2S0 for Ls/Rs
            BRAP_AddOutputPort (hRapCh1,<bSimulPt = false>, BRAP_OutputChannelPair_eCentreLF, BRAP_OutputPort_eI2s1); // Add I2S1 for centre
            BRAP_DEC_Start (hRapCh1, <paramters>) // where paramters have Mode= Simul mode    

            => L/R data on DAC+MAI, Ls/Rs on I2S0, Centre on I2S1, compressed on SPDIF

    - User1 wants to switch back to 2/0 out on Channel1
            BRAP_DEC_Stop (hRapCh1);
            BRAP_RemoveOutputPort (hRapCh1, BRAP_OutputPort_eI2s0);
            BRAP_RemoveOutputPort (hRapCh1, BRAP_OutputPort_eI2s2);
            BRAP_DEC_Start (hRapCh1, <paramters>) // where paramters have Mode= Simul mode   

            => L/R data on DAC+MAI, compressed on SPDIF

    - User1 decides to do PCM on the SPDIF port and CMP on MAI while still carrying PCM on DAC
            BRAP_DEC_Stop (hRapCh1);
            BRAP_RemoveOutputPort (hRapCh1, BRAP_OutputPort_eSpdif);
            BRAP_RemoveOutputPort (hRapCh1, BRAP_OutputPort_eMai);
            BRAP_AddOutputPort (hRapCh1,<bSimulPt = false>, BRAP_OutputChannelPair_eLR, BRAP_OutputPort_eSpdif);
            BRAP_AddOutputPort (hRapCh1,<bSimulPt = true>, BRAP_OutputChannelPair_eLR, BRAP_OutputPort_eMai);
            BRAP_DEC_Start (hRapCh1, <paramters>) // where paramters have Mode= Simul mode   
    
    - User2  starts a new channel with L/R PCM data going out to DAC1 and using the 3/2.1 analog outputs      
            BRAP_DEC_OpenChannel (hRap, &hRapCh2, <settings>); // Where settings 
                have eOutputLR = DAC1, bSimulModeConfig = false, eSimulModePtOutput = Doesnt matter 
            BRAP_AddOutputPort (hRapCh2, <bSimulPt = false>, BRAP_OutputChannelPair_eLRSurround, BRAP_OutputPort_eI2s0); // Add I2S0 for Ls/Rs
            BRAP_AddOutputPort (hRapCh2 ,<bSimulPt = false>, BRAP_OutputChannelPair_eCentreLF, BRAP_OutputPort_eI2s1); // Add I2S1 for centre
            BRAP_DEC_Start (hRapCh2, <paramters>) // where paramters have Mode= decode mode 
            
            => L/R data on DAC, Ls/Rs on I2S0, Centre on I2S1

    -User1 decides to switch to 3/2.1 analog out  on Channel1   
            BRAP_DEC_Stop (hRapCh1);    
            BRAP_AddOutputPort (hRapCh1,<bSimulPt = false>, BRAP_OutputChannelPair_eLRSurround, BRAP_OutputPort_eI2s0); // Add I2S0 for Ls/Rs

            Here the PI will return an ERROR since Decode Channel 2 is already using this port.            

            User1 can then choose to restart with the earlier configuration for Channel 1.           

****************************************************************************/
BERR_Code BRAP_AddOutputPort (
    BRAP_ChannelHandle          hRapCh,         /* [in] The Raptor channel handle */

    bool                                   bSimulPt,          /* [in] TRUE: Reserve this port for the PassThru context of Simul Mode.
                                                                             => This port will be used ONLY if the channel is started in Simul Mode. 
                                                                              It will be used for the compressed data ie Pass Thru context of the Simul mode.   
                                                                              This port will NOT be used at all if the channel is started in Decode/Passthrough mode.

                                                                              FALSE: This port will carry PCM data if the channel is started in Decode/Simul mode
                                                                              or compressed data if the channel is started in PassThru mode 

                                                                              This field is applicable only to decode channels */    
    
    BRAP_OutputChannelPair     eChanPair,    /* [in]  The channel pair -LR/LsRs/C - data to send to the new outport.
                                                                               This field is ignored if bSimulPt is TRUE 

                                                                               This field is applicable only to decode channels */    
                                                                              
    BRAP_OutputPort                eNewOpPort   /* [in] New output port */
);

/***************************************************************************
Summary:
    Removes/de-links an output port from a channel
    
Description:
o   This PI can be used to remove any of the Output ports associated with a channel
        those that were specified at channel open time or added later by calling BRAP_AddOutputPort().

o   This PI should be called only after the channel has been stopped.
     It should not be called while the channel is still running.

o   It will close and free all the resources associated with this output port for this channel.
     Other channels using the same output port can continue to do so

Returns:
	BERR_SUCCESS if successful else error value.

See Also:
    BRAP_AddOutputPort() - to add an output port to a channel
    BRAP_DEC_OpenChannel(), BRAP_PB_OpenChannel(), BRAP_CAP_OpenChannel() - to open channels
    BRAP_DEC_Start(), BRAP_PB_Start(), BRAP_CAP_Start() - to start a channel
    BRAP_DEC_Stop(), BRAP_PB_Stop(), BRAP_CAP_Stop() - to stop a channel    

Usage:
    Typical usage would be
            BRAP_DEC_Stop (hRapCh);
            BRAP_RemoveOutputPort (hRapCh, BRAP_OutputPort_eXXXX);   

    If output port BRAP_OutputPort_eXXXX is not associated with the channel, then the PI will return an error.            

            
****************************************************************************/
BERR_Code BRAP_RemoveOutputPort (
	BRAP_ChannelHandle hRapCh,	/* [in] The Raptor channel handle */
	BRAP_OutputPort eOpPort          /* [in] Output port to be removed from this channel */
);


/*****************************************************************************
Summary:
    Used to install an application callback for channel specific interrupts.

Description:
    Channel specific interrupts that are handled by Raptor PI and can have 
    application callbacks installed are listed in BRAP_Interrupt

    The application speficied callback gets called from the internal handler 
    that the Raptor PI has installed for that particular interrupt.
    
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_RemoveAppInterruptCb()
    BRAP_InstallDeviceLevelAppInterruptCb()
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
    Used to remove the application callback for channel specific interrupts.

Description:
    This PI should be used by the application to remove an application specified 
    callback that has been installed in addition to the RAptor PI interrupt handler 
    for that channel interrupt. Can be used only for callbacks that were installed with 
    BRAP_InstallAppInterruptCb().

Returns:
    BERR_SUCCESS on success
     Error code on failure

See Also:
    BRAP_InstallAppInterruptCb()
******************************************************************************/
BERR_Code BRAP_RemoveAppInterruptCb (
	BRAP_ChannelHandle hRapCh,	/* [in] The RAP channel handle */
    BRAP_Interrupt  eInterrupt  /* [in] The interrupt that needs to be activated */
);

/*****************************************************************************
Summary:
	Install application callback function for device level interrupts
	
Description:
    Device level interrupts are listed in BRAP_DeviceLevelInterrupt.
    
    The application speficied callback gets called from the internal handler 
    that the Raptor PI has installed for that particular interrupt.

Returns:
    BERR_SUCCESS on success
     Error code on failure

See Also:
    BRAP_InstallAppInterruptCb()
    BRAP_RemoveDeviceLevelAppInterruptCb()    
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
	level Raptor interrupts. Can be used only for callbacks that were installed with 
    BRAP_InstallDeviceLevelAppInterruptCb().

Returns:
    BERR_SUCCESS on success
     Error code on failure

See Also:
    BRAP_InstallDeviceLevelAppInterruptCb()
******************************************************************************/
BERR_Code BRAP_RemoveDeviceLevelAppInterruptCb (
	BRAP_Handle hRap,	/* [in] The RAP channel handle */
    BRAP_DeviceLevelInterrupt  eInterrupt  /* [in] The device level interrupt 
								that needs to be removed */
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
	API to retrieve maximum number of decode channels supported. 

Description:
	This API used to retrieve the maximum number of decode channels (channels 
	where either decode, or pass-through or simultaneous mode or SRC can be 
	done) supported by the Raptor Audio PI.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_OpenChannel()

****************************************************************************/
BERR_Code BRAP_DEC_GetTotalChannels( 
	BRAP_Handle 	hRap,			/* [in] The Raptor Audio device handle*/
	unsigned int	*pTotalChannels	/* [out] Total decode Audio channels 
											supported */
	);


/***************************************************************************
Summary:
	API to retrieve Interrupt occurs count. 

Description:


Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_GetInterruptCount()

****************************************************************************/
BERR_Code BRAP_DEC_GetInterruptCount( 
	BRAP_ChannelHandle 		hRapCh,			/* [in] The Raptor Channel handle*/
	BRAP_InterruptCount	*pInterruptCount/* [out] Interrupt Count */
	);

/***************************************************************************
Summary:
	Gets the default channel settings required to open a Raptor Decode Channel.

Description:
	
	Default values of BRAP_DEC_ChannelSettings are:
        eOutputMode = BRAP_OutputMode_e2_0
        uiWaterMark = BRAP_RBUF_P_DEFAULT_FREE_BYTE_MARK

        The DSP/Algorithm settings are not listed here since they are too numerous.

        Note that this PI returns default values for all fields in the 
        BRAP_DEC_ChannelSettings structure, but depending on the usage, 
        not all of them may be used/required.

Returns:
	BERR_SUCCESS on success, error on failure

See Also:
	BRAP_DEC_OpenChannel(), BRAP_DEC_CloseChannel()
***************************************************************************/
BERR_Code BRAP_DEC_GetChannelDefaultSettings(
	BRAP_Handle 			 hRap,			/* [in] The RAP device handle */
	BRAP_DEC_ChannelSettings *pDefSettings	/* [out] Default channel settings */
	);

/***************************************************************************
Summary:
	API used to open a decode channel.
	
Description:
	It is used to instantiate a decode channel. It allocates channel handle 
	and initial resources required for the channel.
	
    This PI merely 'opens' a channel ie sets up the initial channel 
    configuration. It is not sufficient to get audio.
    You will hear audio only after the channel is started.	

Returns:
	BERR_SUCCESS on success, error on failure

See Also:
	BRAP_DEC_GetChannelDefaultSettings(), BRAP_AddOutputPort()
	BRAP_DEC_Start(), BRAP_DEC_Stop(), BRAP_DEC_CloseChannel()
****************************************************************************/
BERR_Code BRAP_DEC_OpenChannel( 
	BRAP_Handle 			hRap,			/* [in] The Raptor Audio device handle*/
	BRAP_ChannelHandle 		*phRapCh,		/* [out] The RAP Channel handle */
	unsigned int			uiChannelNo,	/* [in] the desired channel ID */					
	const BRAP_DEC_ChannelSettings	*pChnSettings /*[in] Channel settings*/ 
	);

/***************************************************************************
Summary:
	API used to close a decode channel.

Description:
	It closes the instance of a decode channel operation. It frees of the 
	channel handle and resources associated with it if any.
	
	Note: 
	1. The channel must first be stopped, before it can be closed.
	2. After stopping the channel, BRAP_RemoveOutputPort() must be called to 
	free the output port and other resources linked to the channel.
	
Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_OpenChannel(), BRAP_DEC_Start(), BRAP_DEC_Stop(), 
	BRAP_RemoveOutputPort()
****************************************************************************/
BERR_Code BRAP_DEC_CloseChannel( 
	BRAP_ChannelHandle 	hRapCh	/* [in] The RAP Channel handle */
	);


/***************************************************************************
Summary:
	Gets the default params for a Decode Channel.
Description:
	This API returns the default parameters for starting a Decode Audio channel. 

	"AC3 stereo decode on I2S0 output port without Simul Mode and Cloning, 
	 with 24 bits/sample output and Timebase 0" configuration is supported 
	 as default.
Returns:
	BERR_SUCCESS on success, error on failure

See Also:
	BRAP_DEC_Start(), BRAP_DEC_Stop()
***************************************************************************/
BERR_Code BRAP_DEC_GetDefaultAudioParams(
	BRAP_ChannelHandle  hRapCh,		/* [in] The RAP channel handle */
	BRAP_DEC_AudioParams *pDefParams	/* [out] Default channel parameters */
	);

/***************************************************************************
Summary:
	Starts a decode channel.

Description:
	This API is required to start the decode/pass-through/SRC of the selected 
	channel (stream), as specified by input BRAP_DEC_AudioParams structure.
	Warning: eOutputMode field of structure BRAP_DEC_AudioParams is ignored
	in BRAP_DEC_Start. Please use API BRAP_DEC_SetConfig to set outputmode.

    Note that before starting an audio channel, BRAP_AddOutputPort() has to be 
    called to link the channel to an output port.

Returns:
	BERR_SUCCESS on success, error on failure

See Also:
	BRAP_DEC_GetDefaultAudioParams(), BRAP_DEC_Stop()
	BRAP_AddOutputPort()
****************************************************************************/
BERR_Code BRAP_DEC_Start ( 
	BRAP_ChannelHandle 			hRapCh,			/* [in] The RAP Channel handle */
	const BRAP_DEC_AudioParams	*pAudioParams	/* [in] Audio parameters required 
														for starting this channel */
	);
/***************************************************************************
Summary:
	Stops a decode channel.

Description:
	This API is required to stop the decode/pass-through/SRC of the selected 
	channel (stream).	
	Note that this does not free the resources associated with the channel. 
	BRAP_RemoveOutputPort() and BRAP_DEC_CloseChannel() have to be explictly 
	called for that.

Returns:
	BERR_SUCCESS on success, error on failure 

See Also:
	BRAP_DEC_Start(), BRAP_RemoveOutputPort(), BRAP_DEC_CloseChannel()
****************************************************************************/
BERR_Code BRAP_DEC_Stop ( 
	BRAP_ChannelHandle 	hRapCh		/* [in] The RAP Decode Channel handle */
	);
									   
/***************************************************************************
Summary:
	This API flushes CDB/ITB buffers associated with a decode channel for 7411 only.
	And for other chips this fucntion clears the itnernal buffers owned by PI and FW.

Description:
	Along with CDB/ITB buffers(7411), various buffers present in the decode pipeline also get flushed.
	BRAP_DEC_DisableForFlush() API should be called before calling BRAP_DEC_Flush() API.

Returns:
	BERR_SUCCESS on success
	Error value on failure

See Also:
     BRAP_DEC_DisableForFlush()
****************************************************************************/
BERR_Code	BRAP_DEC_Flush (
	BRAP_ChannelHandle	hRapCh		/* [in] The RAP Decode Channel handle */
	);

/***************************************************************************
Summary:
	This API is used to inform the decoder to stop reading data from RAVE in
	preparation for flush.

Description:
	This API just stops the decoder channel from reading data from CDB/ITB.
	This API should be called before calling BRAP_DEC_Flush() PI.

Returns:
	BERR_SUCCESS on success
	Error value on failure

See Also:
    BRAP_DEC_Flush()
****************************************************************************/
BERR_Code  BRAP_DEC_DisableForFlush ( 
	BRAP_ChannelHandle	hRapCh		/* [in] The RAP Decode Channel handle */
	);


/***************************************************************************
Summary:
	Change decoder configuration parameters on the fly for a Raptor decode 
	channel
    
Description:
	This function sets decoder configuration parameters that can be set
	on the fly for audio type (eType) mentioned in the configuration
	parameter structure (psDecConfigParams). Decoder configuration parameteters
	contain audio-type specific parameters (elements in union) and common
	parameters for all the audio types (elements outside union).
	Change in configuration parameters depend on the timing of calling
	this API. There are following three cases.
            
	1. If currently decode is not happening on this channel then all configuration
	parameters are stored in PI. When next decode starts, common parameters are applied
	irrespective of audio type of next decode, but audio-type specific parameters
	are applied only on next decode with same audio type.

	2. If currently channel is decoding other audio type,then only common parameters 
	are applied immediately and audio-type specific parameters are stored in PI. 
	Audio-type specific parameters are applied on next decode start with same audio 
	type.

	3. If decode is running with same audio type, all the configuration parameters are 
	applied immediately.

Returns:
	BERR_SUCCESS on success, error on failure

See Also:
	BRAP_DEC_GetDefaultConfig()
	BRAP_DEC_GetCurrentConfig()
****************************************************************************/
BERR_Code BRAP_DEC_SetConfig (
	BRAP_ChannelHandle		hRapCh,		/* [in] RAP channel handle */
	BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */
	);
/***************************************************************************
Summary:
	Change Post Processing configuration parameters on the fly

Description:
	This function sets post processing configuration parameters that can be set
	on the fly for PP algo type  mentioned in the configuration
	parameter structure (psPpConfigParsms). Pp configuration parameteters
	contain pp-type specific parameters (elements in union) and common
	parameters for all the pp types (elements outside union).
	Change in configuration parameters depend on the timing of calling
	this API. There are following three cases.
	1. If currently decode is not happening on this channel then all configuration
	parameters are stored in PI. When next decode starts, common parameters are applied
	irrespective of audio type of next decode, but pp-type specific parameters
	are applied only on next decode with same audio type.
	2. If currently channel is decoding other audio type,then only common parameters 
	are applied immediately and audio-type specific parameters are stored in PI. 
	Audio-type specific parameters are applied on next decode start with same audio 
	type.
	3. If decode is running with same audio type, all the configuration parameters are 
	applied immediately.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_P_GetDefaultPPConfig
	BRAP_DEC_GetPpCurrentConfig
****************************************************************************/

BERR_Code BRAP_DEC_SetPPConfig (
	BRAP_ChannelHandle		hRapCh,		        /* [in] RAP channel handle */
	BRAP_PP_ConfigParams		*psPpConfigParsms	/* [in] Post processing configuration
												    parameters */
	);

/***************************************************************************
Summary:
	Get default decoder configuration parameters for a raptor decode channel

Description:
	This function gets default configuration parameters for given audio 
	type (eType) parameter. 
    
Returns:
	BERR_SUCCESS on success, error on failure

See Also:
	BRAP_DEC_SetConfig()
	BRAP_DEC_GetCurrentConfig()
****************************************************************************/
BERR_Code BRAP_DEC_GetDefaultConfig (
	BRAP_ChannelHandle	hRapCh,		/* [in] RAP channel handle */
	BRAP_DSPCHN_AudioType	eType,	/* [in] Audio type for which to get
									   default configuration parameters */
	BRAP_DEC_ConfigParams	*psDecConfigParams	/* [out] Decoder configuration
												   parameters */
	);

/***************************************************************************
Summary:
	Get default post processing configuration parameters

Description:
	This function gets default configuration parameters for given post 
	processing type (ePpType) parameter. 

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_SetPPConfig
	BRAP_DEC_GetPpCurrentConfig
****************************************************************************/
BERR_Code BRAP_DEC_GetDefaultPPconfig (
	BRAP_ChannelHandle	hRapCh,		/* [in] RAP channel handle */
	BRAP_DSPCHN_PP_Algo	ePpType,	/* [in] Audio type for which to get
									   default configuration parameters */
	BRAP_PP_ConfigParams		*psPpConfigParsms	/* [Out] Post processing configuration
												    parameters */							   
	);
																			  

/***************************************************************************
Summary:
	Get current decoder configuration parameters for a raptor decode channel
    
Description:
	This function gets current configuration parameters for given audio 
	type (eType) parameter.

Returns:
	BERR_SUCCESS on success, error on failure

See Also:
	BRAP_DEC_SetConfig()
	BRAP_DEC_GetDefaultConfig()
****************************************************************************/
BERR_Code BRAP_DEC_GetCurrentConfig (
	BRAP_ChannelHandle	hRapCh,		/* [in] RAP channel handle */
	BRAP_DSPCHN_AudioType	eType,	/* [in] Audio type for which to get
									   Current configuration parameters */
	BRAP_DEC_ConfigParams	*psDecConfigParams	/* [out] Decoder configuration
												   parameters */
	);

/***************************************************************************
Summary:
	Get current Post Processing configuration parameters

Description:
	This function gets current configuration parameters for given Pp 
	type (uiPpAlgo) parameter.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_SetPPConfig
	BRAP_DEC_GetDefaultPPConfig
****************************************************************************/
BERR_Code BRAP_DEC_GetPpCurrentConfig (
	BRAP_ChannelHandle	hRapCh,		/* [in] RAP channel handle */
	BRAP_DSPCHN_PP_Algo	uiPpAlgo,	/* [in] Audio type for which to get
									   Current configuration parameters */
	unsigned int				uiPpBranchId,	/* [in] Branch Id of the post
											processing stage for which to get
											current counfiguration parameters.
											If current configuration is not
											required for any post processing
											stage, this field should be set to
											BRAP_MAX_PP_BRANCH_SUPPORTED */
	unsigned int				uiPpStageId,		/* [in] Stage Id of the post
											processing stage for which to get
											current counfiguration parameters.
											If current configuration is not
											required for any post processing
											stage, this field should be set to
											BRAP_MAX_PP_PER_BRANCH_SUPPORTED */																		   
	BRAP_PP_ConfigParams	*psDecConfigParams	/* [out] Decoder configuration
												   parameters */
	);


/***************************************************************************
Summary:
	Gets the default settings of the PCM Playback Channel.
Description:
	This API returns the default settings of the PCM Playback Channel.
	These can be modified as required and used to open a Playback channel.
Returns:
	BERR_SUCCESS 
See Also:
	BRAP_PB_Open()
***************************************************************************/
BERR_Code BRAP_PB_GetChannelDefaultSettings(
	BRAP_Handle 			hRap,		 	/* [in] The RAP device handle */
	BRAP_PB_ChannelSettings *pDefSettings	/* [out] Default channel settings */
);

/***************************************************************************
Summary:
	API used to open a PCM playback channel.
Description:
	It is used to instantiate a PCM playback channel. It allocates channel 
	handle and resource required for the channel, if any.
    This PI merely 'opens' a channel ie sets up the initial channel 
    configuration. It is not sufficient to get audio.
    You will hear audio only after the channel is started.	

Returns:
	BERR_SUCCESS on success, error on failure

See Also:
	BRAP_PB_GetChannelDefaultSettings(), BRAP_AddOutputPort()
	BRAP_PB_Start(), BRAP_PB_Stop(), BRAP_PB_CloseChannel()
****************************************************************************/
BERR_Code BRAP_PB_OpenChannel( 
	const BRAP_Handle 				hRap,			/* [in] The Raptor Audio device handle*/
	BRAP_ChannelHandle 				*phRapCh,		/* [out] The RAP Channel handle */
	const unsigned int				uiChannelNo,	/* [in] the desired channel ID */	
	const BRAP_PB_ChannelSettings 	*pChnSettings 	/* [in] Channel settings*/ 
	);

/***************************************************************************
Summary:
	API used to close a PCM playback channel.
	
Description:
	It closes the instance of a PCM playback channel operation. It frees the 
	channel handle and resources associated with it, if any.
	
	Note: 
	1. The channel must first be stopped, before it can be closed.
	2. After stopping the channel, BRAP_RemoveOutputPort() must be called to 
	free the output port and other resources linked to the channel.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_PB_OpenChannel(), BRAP_PB_Start(), BRAP_PB_Stop(), 
	BRAP_RemoveOutputPort()
****************************************************************************/
BERR_Code BRAP_PB_CloseChannel( 
	BRAP_ChannelHandle 	hRapCh	/* [in] The RAP Channel handle */
	);

/***************************************************************************
Summary:
	Gets the default audio params for a PCM Playback Channel.
	
Description:
	This API returns the default parameters for a PCM Playback channel. 
	These can be modified as required and used to start a Playback channel
	
Returns:
	BERR_SUCCESS 
	
See Also:
	BRAP_PB_Start(), BRAP_PB_Stop()
***************************************************************************/
BERR_Code BRAP_PB_GetDefaultAudioParams(
	BRAP_ChannelHandle  hRapCh,		/* [in] The RAP channel handle */
	BRAP_PB_AudioParams *pDefParams /* [out] Default channel parameters */
	);

/***************************************************************************
Summary:
	Starts a PCM playback channel.
Description:
	This API is required to start the PCM playback of the selected stream, 
	as specified by input BRAP_PB_AudioParams structure.

Returns:
	BERR_SUCCESS on success, error on failure

See Also:
	BRAP_PB_GetDefaultAudioParams(), BRAP_PB_Stop()
	BRAP_AddOutputPort()
****************************************************************************/
BERR_Code BRAP_PB_Start ( 
	BRAP_ChannelHandle 		  hRapCh,		/* [in] The RAP Channel handle */
	const BRAP_PB_AudioParams *pAudioParams /* [in] Audio parameters required 
												for starting this channel */
	);

/***************************************************************************
Summary:
	Stops a PCM playback channel.
Description:
	This API is required to stop the PCM playback channel.
	Note that this does not free the resources associated with the channel. 
	BRAP_RemoveOutputPort() and BRAP_PB_CloseChannel() have to be explictly 
	called for that.	
Returns:
	BERR_SUCCESS on success, error on failure
See Also:
	BRAP_PB_Start(), BRAP_RemoveOutputPort(), BRAP_PB_CloseChannel()
****************************************************************************/
BERR_Code BRAP_PB_Stop ( 
	BRAP_ChannelHandle	hRapCh 	/* [in] The RAP Decode Channel handle */
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
	Function that updates sampling rate of SRC.
Description:
	Function that updates sampling rate of SRC.This function must be called
	for PCM playback channels only.
Returns:
	BERR_SUCCESS 
See Also:
	BRAP_PB_SetSRCInputFrequency
****************************************************************************/
BERR_Code 
BRAP_PB_SetSRCFrequency(
		BRAP_ChannelHandle hRapCh,
		BRAP_OutputPort eOutputPort, 
		unsigned int uiSROut, 
		unsigned int uiSRIn);

/***************************************************************************
Summary:
	Returns the default settings of the PCM Capture Channel.
	These can be modified as required and used to open a Capture channel.
	
Returns:
	BERR_SUCCESS on success, error on failure
	
See Also:
	BRAP_CAP_OpenChannel()
***************************************************************************/
BERR_Code BRAP_CAP_GetChannelDefaultSettings (
        BRAP_Handle         hRap,        /* [in] The Raptor device handle */
        BRAP_CAP_ChannelSettings *pDefSettings    /* [out] Pointer to memory where the Default channel settings are returned */
);

/***************************************************************************
Summary:
	API used to open a PCM Capture channel.

Description:
	It is used to instantiate a PCM Capture channel. It allocates channel 
	handle and resource required for the channel.
	
    This PI merely 'opens' a channel ie sets up the initial channel 
    configuration. It is not sufficient to get audio.
    You will hear audio only after the channel is started.	

Returns:
	BERR_SUCCESS on success, error on failure

See Also:
	BRAP_CAP_GetChannelDefaultSettings(), BRAP_AddOutputPort()
	BRAP_CAP_Start(), BRAP_CAP_Stop(), BRAP_CAP_CloseChannel()	
****************************************************************************/
BERR_Code BRAP_CAP_OpenChannel( 
        BRAP_Handle             hRap,               /* [in] The Raptor device handle */
        BRAP_ChannelHandle *phRapCh,        /* [out] The Raptor Capture Channel handle */
        unsigned int        uiChannelNo, /* [in] the desired channel ID */
        const BRAP_CAP_ChannelSettings    *pChnSettings /*[in] Pointer to Capture Channel settings*/ 
);

/***************************************************************************
Summary:
	API used to close a PCM Capture channel.
	
Description:
	It closes the instance of a PCM Capture channel operation. It frees the 
	channel handle and resources associated with it
		
	Note: 
	1. The channel must first be stopped, before it can be closed.
	2. After stopping the channel, BRAP_RemoveOutputPort() must be called to 
	free the output port and other resources linked to the channel.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_CAP_OpenChannel(), BRAP_CAP_Start(), BRAP_CAP_Stop(), 
	BRAP_RemoveOutputPort()
****************************************************************************/
BERR_Code BRAP_CAP_CloseChannel ( 
        BRAP_ChannelHandle     hRapCh    /* [in] The Raptor Capture Channel handle */
);


/***************************************************************************
Summary:
	Returns the default audio params for a PCM Capture Channel.
	These can be modified as required and used to start a Capture channel.
	
Returns:
	BERR_SUCCESS on success, error on failure
	
See Also:
	BRAP_CAP_Start(), BRAP_CAP_Stop()
***************************************************************************/
BERR_Code BRAP_CAP_GetDefaultAudioParams(
        BRAP_ChannelHandle     hRapCh,    /* [in] The Raptor Capture Channel handle */
        BRAP_CAP_AudioParams *pDefParams  /* [out] Pointer to memory where the Default channel parameters are returned */
);


/***************************************************************************
Summary:
	Starts a PCM Capture channel.
	
Description:
	This API is required to start the PCM Capture of the selected stream, 
	as specified by input BRAP_CAP_AudioParams structure.

Returns:
	BERR_SUCCESS on success, error on failure

See Also:
	BRAP_CAP_GetDefaultAudioParams(), BRAP_CAP_Stop()
	BRAP_AddOutputPort()
****************************************************************************/
BERR_Code BRAP_CAP_Start ( 
        BRAP_ChannelHandle     hRapCh,    /* [in] The Raptor Capture Channel handle */
        const BRAP_CAP_AudioParams    *pAudioParams /*[in] Audio parameters required for starting this channel */
);


/***************************************************************************
Summary:
	Stops a PCM Capture channel.
	
Description:
	Note that this does not free the resources associated with the channel. 
	BRAP_RemoveOutputPort() and BRAP_CAP_CloseChannel() have to be explictly 
	called for that.	
	
Returns:
	BERR_SUCCESS on success, error on failure
	
See Also:
	BRAP_CAP_Start(), BRAP_RemoveOutputPort(), BRAP_CAP_CloseChannel()
****************************************************************************/
BERR_Code BRAP_CAP_Stop ( 
        BRAP_ChannelHandle     hRapCh    /* [in] The Raptor Capture Channel handle */
);


/***************************************************************************
Summary:	
	Updates the read pointers for the associated ring buffers as per the 
	ring buffer usage provided.
Description:
	After reading PCM data from the ring buffer, an application can use this 
	routine to update the read pointer associated to that ring buffer. 
Returns:
	BERR_SUCCESS
See Also:
	BRAP_CAP_GetBufInfo
****************************************************************************/
BERR_Code BRAP_CAP_UpdateBufUsg( 
	BRAP_ChannelHandle 	hRapCh,			/* [in] Audio channel handle */
    const BRAP_PcmBufSzUpdt	*psPcmBufSzUpdt	/* [in] Ring buffer read 
    											   information */
);

/***************************************************************************
Summary:	
	Updates the read pointers for the associated ring buffers as per the 
	ring buffer usage provided.
Description:
	After reading PCM data from the ring buffer, an application can use this 
	routine to update the read pointer associated to that ring buffer. 
	This is an ISR version of BRAP_CAP_UpdateBufUsg().
Returns:
	BERR_SUCCESS
See Also:
	BRAP_CAP_GetBufInfo
****************************************************************************/
BERR_Code BRAP_CAP_UpdateBufUsg_isr( 
	BRAP_ChannelHandle 	hRapCh,			/* [in] Audio channel handle */
    const BRAP_PcmBufSzUpdt	*psPcmBufSzUpdt	/* [in] Ring buffer usage 
    											   information */
);

/***************************************************************************
Summary:	
	Gets the associated ring buffers's information.
Description:
	This API returns the associated ring buffers's information.
	Note: BRAP_PcmBufInfo has left and right ring buffer info structure. Each 
      info structure has both pWritePtr and pReadPtr elements. This routine 
      returns pWritePtr = NULL and pReadPtr = valid address (if any) as an
      application requires only a readPtr to copy captured PCM data	from 
      ring buffer to some local memory.
Returns:
	BERR_SUCCESS
See Also:
	BRAP_CAP_UpdateBufUsg
****************************************************************************/
BERR_Code BRAP_CAP_GetBufInfo(
	BRAP_ChannelHandle 	hRapCh,		   /* [in] Audio channel handle */
	BRAP_PcmBufInfo		*psPcmBufInfo  /* [Out] Associated Pcm buffer 
													 information */		
);

/***************************************************************************
Summary:	Gets the associated ring buffers's information.
Description:
	This API returns the associated ring buffers's information. This is an 
	ISR version of BRAP_CAP_GetBufInfo(). 
Returns:
	BERR_SUCCESS
See Also:
****************************************************************************/
BERR_Code BRAP_CAP_GetBufInfo_isr(
	BRAP_ChannelHandle 	hRapCh,		   /* [in] Audio channel handle */
	BRAP_PcmBufInfo		*psPcmBufInfo  /* [Out] Associated Pcm buffer 
													 information */		
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
	BRAP_CAP_BufferReadDone_isr
****************************************************************************/
BERR_Code BRAP_CAP_BufferReadDone(
	BRAP_ChannelHandle 	hRapCh		   /* [in] Audio channel handle */
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
	BRAP_CAP_BufferReadDone
****************************************************************************/
BERR_Code BRAP_CAP_BufferReadDone_isr(
	BRAP_ChannelHandle 	hRapCh		   /* [in] Audio channel handle */
);

#if (BRAP_AD_SUPPORTED == 1)

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
    BRAP_ConfigureAudioDescriptorFadeTable
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
    BRAP_ConfigureAudioDescriptorFadeTable
**************************************************************************/
BERR_Code BRAP_DestroyAudioDescriptorFadeTable(
    BRAP_AudioDescriptorFadeHandle hAdFade
    );

/***************************************************************************
Summary:
    Configures audio descriptor fade table object.
Description:
    This function configures audio descriptor fade table object. For a given audio
    descriptor feature, this function identifies which RAP channel is a primary channel
    and which RAP channel is audio descriptor channel.
Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
    BRAP_CreateAudioDescriptorFadeTable
    BRAP_DestroyAudioDescriptorFadeTable
**************************************************************************/
BERR_Code BRAP_ConfigureAudioDescriptorFadeTable(
    BRAP_AudioDescriptorFadeHandle hAdFade, 
    BRAP_AudioDescriptorFadeTableParams sParams
    );
#endif

#ifdef RAP_SRSTRUVOL_CERTIFICATION  
/**********************************************************************
Summary: Allocates 2 PCM Buffer of size uiSize. These Buffers are used for SRS
Certification.RD/WR/Base/End values are programmed in psBufInfo.
***********************************************************************/

BERR_Code BRAP_AllocatePCMBuffers(
    BRAP_ChannelHandle      hRapCh,
    unsigned int   uiSize,
	unsigned int   ui32Context,    
    BRAP_BufInfo    *psBufInfo);


/**********************************************************************
Summary: Free  PCM Buffer allocated  for SRS
Certification.
***********************************************************************/

BERR_Code BRAP_FreePCMBuffers(
    BRAP_ChannelHandle      hRapCh);


/**********************************************************************
Summary: Returns current PCM Buffer Info. It returns updated
 Read pointer as used by FW at that instant.
***********************************************************************/

BERR_Code BRAP_GetPCMBufferInfo(
    BRAP_ChannelHandle      hRapCh,
    unsigned int   ui32Context,       
    BRAP_BufInfo    *psBufInfo);


/**********************************************************************
Summary: Updates current PCM Buffer Info. It will update write 
pointer in the register
***********************************************************************/
BERR_Code BRAP_UpdatePCMBufferInfo(
    BRAP_ChannelHandle      hRapCh,
    unsigned int   ui32Context,        
    BRAP_BufInfo    *psBufInfo);

BERR_Code BRAP_GetRbufPtr(
	BRAP_ChannelHandle      hRapCh,
	BRAP_RingBufInfo  *psLeftRbuf,
	BRAP_RingBufInfo  *psRightRbuf);

BERR_Code BRAP_GetRbufPtr_isr(
	BRAP_ChannelHandle      hRapCh,
	BRAP_RingBufInfo  *psLeftRbuf,
	BRAP_RingBufInfo  *psRightRbuf);

BERR_Code BRAP_UpdateRbufReadPtr(
	BRAP_ChannelHandle      hRapCh,
	BRAP_RingBufInfo  *psLeftRbuf,
	BRAP_RingBufInfo  *psRightRbuf);
BERR_Code BRAP_UpdateRbufReadPtr_isr(
	BRAP_ChannelHandle      hRapCh,
	BRAP_RingBufInfo  *psLeftRbuf,
	BRAP_RingBufInfo  *psRightRbuf);

#endif


/*}}}*/

#ifdef __cplusplus
}
#endif

#endif /* _BRAP_H__ */
