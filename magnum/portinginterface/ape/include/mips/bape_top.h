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
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BAPE_H_
#define BAPE_H_

#include "bchp.h"
#include "bint.h"
#include "bmem.h"
#include "breg_mem.h"
#include "btmr.h"
#include "bimg.h"
#include "bape_types.h"
#include "bape_mixer.h"
#include "bape_decoder.h"
#include "bape_playback.h"
#include "bape_processor.h"
#include "bape_input.h"
#include "bape_output.h"
#include "bape_pll.h"
#include "bape_input_group.h"
#include "bape_input_capture.h"
#include "bape_output_capture.h"

/*=************************ Module Overview ******************************** 
APE (Audio Processing Engine) is a porting interface (PI) module that 
controls the audio subsystem.  This includes the AIO (mixer, inputs, outputs) 
as well as the decode processor (DSP or CPU). 
 
Device: 
-------
An APE Device is identified by a device handle (BAPE_Handle).  This is the 
top-level interface to the audio subsystem. 
 
Decoder: 
-------- 
A decoder is capable of decoding digital audio into PCM samples and also 
providing compressed passthrough of input data.  This is identified by 
a BAPE_DecoderHandle.  Decoding is done in a decode processor, and the 
output can be fed into a mixer.  Optionally, post-processing (BAPE_Processor) 
may be inserted between the decoder and the mixer.   Decoders can read 
from a RAVE context (BAVC_XptContextMap) or on some systems can read 
from an input port (BAPE_InputPort).  The latter is useful for external 
inputs that provide compressed data (e.g. SPDIF) or inputs that require 
post processing.   

Playback: 
--------- 
Playback is used for playing host-generated PCM data.  It is also an 
input to a mixer. 
 
InputCapture: 
------------- 
InputCapture is used to capture PCM input from an input port (BAPE_InputPort) 
and feeds data into a mixer. 
 
InputGroup: 
----------- 
Input groups allow for multiple stereo inputs to be grouped together 
and treated as multichannel data.  This is useful if you want to combine 
three stereo playback channels to generate 5.1 audio for example.  Grouped 
inputs will always start synchronously in the mixer. 
 
Mixer: 
------ 
Mixers are used to mix one or more inputs to one or more outputs.  A 
mixer is always required to bind an output to its input(s).   Mixers 
provide input volume scaling as well as output volume scaling.  All 
inputs to a mixer must run at the same sample rate, and correspondingly 
they will be sample rate converted to either a fixed sample rate value 
or they can slave to a master input's sampling rate.  It is possible 
for an input to be routed to more than one mixer.  For example, the 
application may want different input volume levels applied to one 
output vs. another one.  For example, you could use two mixers to 
send a decode input mixed with a playback input to one output setting 
both inputs to half volume.  You could simultaneously route the same 
decode input to another output at full volume with no playback 
by using two mixers and applying different volume levels to each. 
Inputs to a mixer are connected using a generic connector type 
(BAPE_MixerInput), and similarly outputs are connected using 
a generic connector type (BAPE_MixerOutput). 
 
Outputs:
-------- 
Several different output types are supported by this module.  They include 
audio DACs (BAPE_DacHandle), SPDIF transmitters (BAPE_SpdifOutputHandle), 
I2S transmitters (BAPE_I2sOutputHandle) and the MAI interface to HDMI 
(BAPE_MaiOutputHandle).  Outputs must always be connected to a single 
mixer to receive data, even if they are receiving compressed output. 
Output data can also be captured by the host by connecting an 
BAPE_OutputCaptureHandle. 
 
Inputs: 
------- 
External audio inputs are supported by this module.  BAPE_I2sInput 
represents an I2S receiver.  In the future, other inputs may be 
added as well. 
 
Example Connection Diagrams: 
---------------------------- 
1) Single decoder routed to DAC + SPDIF 
BAPE_DecoderHandle (stereo data) -> BAPE_MixerHandle -> BAPE_DacHandle + BAPE_SpdifOutputHandle 
 
2) Decoder + host data mixed to DAC, Decoder data routed directly to SPDIF (PCM) 
BAPE_PlaybackHandle --------------\  
BAPE_DecoderHandle (stereo data) --> BAPE_MixerHandle1 -> BAPE_DacHandle 
                                 \-> BAPE_MixerHandle2 -> BAPE_SpdifOutputHandle
 
3) Decode to DAC with compressed passthrough to SPDIF 
 
BAPE_DecoderHandle1 (stereo data)     -> BAPE_MixerHandle1 -> BAPE_DacHandle 
BAPE_DecoderHandle2 (compressed data) -> BAPE_MixerHandle2 -> BAPE_SpdifOutputHandle 
 
4) Decode to DAC and I2S with DAC delayed to compensate for an external 
audio processor on I2S. 
BAPE_DecoderHandle (stereo data) --> BAPE_MixerHandle1 (delay=0) -> BAPE_DacHandle 
                                 \-> BAPE_MixerHandle2 (delay=n) -> BAPE_I2sOutputHandle
 
5) I2S stereo input routed to DAC 
BAPE_I2sInput -> BAPE_InputCapture -> BAPE_MixerHandle -> BAPE_DacHandle 
 
6) Decode of primary audio mixed with decode of secondary audio (e.g. MP3 sound effect) 
BAPE_DecoderHandle1 (stereo data) --> BAPE_MixerHandle -> BAPE_DacHandle 
BAPE_DecoderHandle2 (stereo data) -/
 
***************************************************************************/

/***************************************************************************
Summary:
Device Settings
***************************************************************************/
typedef struct BAPE_Settings
{
    const BIMG_Interface *pImageInterface;      /* Interface to access firmware image. This interface must be
                                                   implemented and the function pointers must be stored here. */

    void **pImageContext;                       /* Context for the Image Interface. This is also provided by
                                                   the implementation of the Image interface */

    /* These callback routines are used to communicate with the audio processor/DSP. */
    uint32_t (*readReserved)(void *pParam1, int param2, uint32_t physicalAddress);              /* Read 4 bytes of data starting at physicalAddress */
    void (*writeReserved)(void *pParam1, int param2, uint32_t physicalAddress, uint32_t data);  /* Write 4 bytes of data starting at physicalAddress */
    uint32_t (*readCoreRegister)(void *pParam1, int param2, uint32_t offset);                   /* Read 4 bytes of data from the core register offset specified */
    void (*writeCoreRegister)(void *pParam1, int param2, uint32_t offset, uint32_t data);       /* Write 4 bytes of data to the core register offset specified */
    uint32_t (*readCmtControl)(void *pParam1, int param2);                                      /* Read the CMT control register (CP0 22 sel 2) */
    void (*writeCmtControl)(void *pParam1, int param2, uint32_t data);                          /* Write the CMT control register (CP0 22 sel 2) */
    void *pParam1;      /* First parameter passed to DSP callbacks above */
    int param2;         /* Second parameter passed to DSP callbacks above */

    unsigned maxIndependentDelay;       /* Max independent delay value in ms */
    unsigned maxPcmSampleRate;          /* Max PCM sample rate in Hz */
    bool rampPcmSamples;                /* If true (default), PCM samples will be ramped up/down on startup, shutdown, and underflow conditions.  
                                           Set to false if you want to disable this feature for testing or verification purposes. */
    bool highBitRateEnabled;            /* If set to true, compressed buffers are large enough to accomodate 4x the decode
                                           sample rate.  This is required for AC3+ Passthrough over HDMI. */
    unsigned numCompressedBuffers;      /* Number of discrete compressed decoder output buffers required.  This is the
                                           number of outputs that will receive discrete compressed content.
                                           Independent delay does not affect this number, only different content sources. */ 
    unsigned numPcmBuffers;             /* Number of discrete PCM decoder output buffers required.  This is the
                                           number of outputs that will receive discrete stereo content plus the
                                           number of outputs that will receive discrete multichannel output * the number
                                           of channel pairs involved in multichannel (3 for 5.1, 4 for 7.1).  
                                           Independent delay does not affect this number, only different content sources. */ 
} BAPE_Settings;

/***************************************************************************
Summary:
Get default settings for an audio processor
***************************************************************************/
void BAPE_GetDefaultSettings(
    BAPE_Settings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Open an audio processor
***************************************************************************/
BERR_Code BAPE_Open(
    BAPE_Handle *pHandle,   /* [out] returned handle */
    BCHP_Handle chpHandle,
    BREG_Handle regHandle,
    BMEM_Handle memHandle,
    BINT_Handle intHandle,
    BTMR_Handle tmrHandle,
    const BAPE_Settings *pSettings  /* NULL will use default settings */
    );

/***************************************************************************
Summary:
Close an audio processor
***************************************************************************/
void BAPE_Close(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Device Level Interrupts
***************************************************************************/
typedef struct BAPE_InterruptHandlers
{
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;        
    } watchdog;
} BAPE_InterruptHandlers;

/***************************************************************************
Summary:
Get Currently Registered Interrupt Handlers
***************************************************************************/
void BAPE_GetInterruptHandlers(
    BAPE_Handle handle,
    BAPE_InterruptHandlers *pInterrupts     /* [out] */
    );

/***************************************************************************
Summary:
Set Interrupt Handlers 
 
Description: 
To disable any unwanted interrupt, pass NULL for its callback routine
***************************************************************************/
BERR_Code BAPE_SetInterruptHandlers(
    BAPE_Handle handle,
    const BAPE_InterruptHandlers *pInterrupts
    );

/***************************************************************************
Summary:
Process a watchdog interrupt
***************************************************************************/
BERR_Code BAPE_ProcessWatchdogInterrupt(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Retrieve the audio path delay value in ms.
***************************************************************************/
void BAPE_GetDecoderPathDelay(
    BAPE_Handle handle,
    unsigned *pDelay    /* [out] in ms */
    );

/***************************************************************************
Summary:
Returns single null terminated line from the debug log
***************************************************************************/
BERR_Code BAPE_GetDebugLog(
    BAPE_Handle handle,
    char *buf,
    size_t buf_len
    );

/***************************************************************************
Summary:
Put APE in standby
***************************************************************************/
BERR_Code BAPE_Standby(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Resume APE from standby
***************************************************************************/
BERR_Code BAPE_Resume(
    BAPE_Handle handle
    );

#endif

