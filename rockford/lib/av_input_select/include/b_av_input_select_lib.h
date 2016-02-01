/***************************************************************************
*     (c)2008 Broadcom Corporation
*  
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
*   
*  Except as expressly set forth in the Authorized License,
*   
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*   
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
*  USE OR PERFORMANCE OF THE SOFTWARE.
*  
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
*  ANY LIMITED REMEDY.
* 
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Description: Av_input_select header file for an App Lib
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef B_Avis_LIB_H__
#define B_Avis_LIB_H__

#include "bstd.h"
#include "b_os_lib.h"		/* Used for error codes */

/* Nexus includes */
#include "nexus_platform.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_video_input.h"
#include "nexus_audio_mixer.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_spdif_output.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_video_window.h"
#include "nexus_video_input_vbi.h"
#include "nexus_rf_audio_decoder.h"
#include "nexus_hdmi_input.h"
#include "nexus_analog_audio_input.h"
#include "nexus_analog_audio_decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=Module Overview: ********************************************************
The purpose of this module is to provide support for changing the input source
to a video window.  This applies to both main and pip.  The sources that are
supported are digital TV, analog TV, composite video input, S-video input,
component video input, RGB(PC) input, and HDMI input.  In cases where there are
multiple inputs for a source, for example, composite, component, and HDMI, a
index is provided to enumerate the desired input. APIs deals with stopping an
input do not normally require the source index because the logic to stop the
input source is the same independent of the index.

In the case of multiple inputs for a source type, the number of such inputs
is controlled by define parameters from the platform configuration in Nexus.
***************************************************************************/

/***************************************************************************
Summary:
Av_input_select Sources
***************************************************************************/
typedef enum B_Avis_Source
{
	B_Avis_eNothing,
	B_Avis_eDigitalTV,
	B_Avis_eAnalogTV,
	B_Avis_eComposite,
	B_Avis_eSvideo,
	B_Avis_eComponent,
	B_Avis_eRGB,
	B_Avis_eHDMI,
	B_Avis_eLast_Source
} B_Avis_Source;

/***************************************************************************
Summary:
 Av_input_select App Lib settings structure
***************************************************************************/
typedef struct B_Avis_Settings
{
	NEXUS_CallbackDesc sourcePending;
} B_Avis_Settings;

/***************************************************************************
Summary:
Opaque public handle for Av_input_select App Lib
***************************************************************************/
typedef void * B_Avis_Handle;

/***************************************************************************
Summary:
Returns the default and recommended values for the App Lib public settings

Description:
This function returns the default and recommended values for the App Lib
public settings. A pointer to a valid B_Avis_Settings structure must be
provided or an error will be returned.

Input:
	pAv_input_selectSettings - pointer to an existing settings structure

Returns:
	BERR_SUCCESS - If settings where able to be set
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
***************************************************************************/
B_Error B_Avis_GetDefaultSettings(B_Avis_Settings *pAv_input_selectSettings);


/***************************************************************************
Summary:
This function initializes the Av_input_select App Lib 

Description:
This function initializes the App Lib based on the settings selected. A shallow
copy of the B_Avis_Settings structure is made in this call. The private App Lib
structure is malloc'ed.

Input:
	pAv_input_selectSettings - pointer to an existing settings structure

Returns:
	Opaque module handle used in subsequent API calls to this module
	NULL - If pAv_input_selectSettings was NULL or module structure could not be allocated
***************************************************************************/
B_Avis_Handle B_Avis_Open(B_Avis_Settings *pAv_input_selectSettings);

/***************************************************************************
Summary:
This function de-initializes the Av_input_select App Lib.

Description:
This function de-initializes the App Lib. The private App Lib structure is freed.

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib

Returns:
	BERR_SUCCESS - If the module was able to de-initialize.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
***************************************************************************/
B_Error B_Avis_Close(B_Avis_Handle hAv_input_select);

/***************************************************************************
Summary:
Validates a combination of source inputs and video outputs.

Description:
Validates a combination of source inputs and video outputs.

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	main - Type of input the main window will display
	pip - Type of input the pip window will display

Returns:
	BERR_SUCCESS - If the configuration of inputs is allowed.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
	BERR_NOT_SUPPORTED - If the configuration of inputs is not allowed.
***************************************************************************/
B_Error B_Avis_IsInputSourcePossible(B_Avis_Handle hAv_input_select,
							  B_Avis_Source main, 
							  B_Avis_Source pip);

/***************************************************************************
Summary:
Starts the Digital TV source on the given output window

Description:
Starts the Digital TV source on the given output window

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	hVideoDecoder - Handle representing the source
	hWindow - Active window to attach input to

Returns:
	BERR_SUCCESS - If the video was started.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
	BERR_NOT_SUPPORTED - If the requested configuration is not supported on this platform.
***************************************************************************/
B_Error B_Avis_StartInputVideoSourceDigitalTV(B_Avis_Handle hAv_input_select,
								NEXUS_VideoDecoderHandle hVideoDecoder,
								NEXUS_VideoWindowHandle hWindow);

/***************************************************************************
Summary:
Starts the Analog TV source on the given output window

Description:
Starts the Analog TV source on the given output window

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	hAnalogVideoDecoder - Handle representing the analog video decoder
	hWindow - Active window to attach input to

Returns:
	BERR_SUCCESS - If the video was started.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
	BERR_NOT_SUPPORTED - If the requested configuration is not supported on this platform.
***************************************************************************/
B_Error B_Avis_StartInputVideoSourceAnalogTV(B_Avis_Handle hAv_input_select,
								NEXUS_AnalogVideoDecoderHandle hAnalogVideoDecoder,
								NEXUS_VideoWindowHandle hWindow);

/***************************************************************************
Summary:
Starts a composite source on the given output window

Description:
Starts an composite source on the given output window

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	sourceIndex - sub index of the source input type
	hAnalogVideoDecoder - Handle representing the analog video decoder
	hWindow - Active window to attach input to

Returns:
	BERR_SUCCESS - If the video was started.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
	BERR_NOT_SUPPORTED - If the requested configuration is not supported on this platform.
***************************************************************************/
B_Error B_Avis_StartInputVideoSourceComposite(B_Avis_Handle hAv_input_select,
							    unsigned int sourceIndex,
								NEXUS_AnalogVideoDecoderHandle hAnalogVideoDecoder,
								NEXUS_VideoWindowHandle hWindow);

/***************************************************************************
Summary:
Starts a s-video source on the given output window

Description:
Starts an s-video source on the given output window

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	sourceIndex - sub index of the source input type
	hAnalogVideoDecoder - Handle representing the analog video decoder
	hWindow - Active window to attach input to

Returns:
	BERR_SUCCESS - If the video was started.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
	BERR_NOT_SUPPORTED - If the requested configuration is not supported on this platform.
***************************************************************************/
B_Error B_Avis_StartInputVideoSourceSvideo(B_Avis_Handle hAv_input_select,
							    unsigned int sourceIndex,
								NEXUS_AnalogVideoDecoderHandle hAnalogVideoDecoder,
								NEXUS_VideoWindowHandle hWindow);

/***************************************************************************
Summary:
Starts the given component source on the given output window

Description:
Starts the given component source on the given output window

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	sourceIndex - sub index of the source input type
	hAnalogVideoDecoder - Handle representing the analog video decoder
	window - Active window to attach input to

Returns:
	BERR_SUCCESS - If the video was started.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
	BERR_NOT_SUPPORTED - If the requested configuration is not supported on this platform.
***************************************************************************/
B_Error B_Avis_StartInputVideoSourceComponent(B_Avis_Handle hAv_input_select,
								  unsigned int sourceIndex,
								  NEXUS_AnalogVideoDecoderHandle hAnalogVideoDecoder,
								  NEXUS_VideoWindowHandle hWindow);

/***************************************************************************
Summary:
Starts the RGB source on the given output window

Description:
Starts the RGB source on the given output window

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	hAnalogVideoDecoder - Handle representing the analog video decoder
	window - Active window to attach input to

Returns:
	BERR_SUCCESS - If the video was started.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
	BERR_NOT_SUPPORTED - If the requested configuration is not supported on this platform.
***************************************************************************/
B_Error B_Avis_StartInputVideoSourceRGB(B_Avis_Handle hAv_input_select,
							NEXUS_AnalogVideoDecoderHandle	hAnalogVideoDecoder,
							NEXUS_VideoWindowHandle hWindow);

/***************************************************************************
Summary:
Starts the given HDMI source on the given output window

Description:
Starts the given HDMI source on the given output window

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	hHdmiInput - Handle to Nexus HDMI input
	window - Active window to attach input to

Returns:
	BERR_SUCCESS - If the video was started.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
	BERR_NOT_SUPPORTED - If the requested configuration is not supported on this platform.
***************************************************************************/
B_Error B_Avis_StartInputVideoSourceHDMI(B_Avis_Handle hAv_input_select,
							 NEXUS_HdmiInputHandle	hHdmiInput,
							 NEXUS_VideoWindowHandle hWindow);

/***************************************************************************
Summary:
Stops the currently active digital TV input source in the given output window

Description:
Stops the currently active digital TV input source in the given output window

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	hVideoDecoder - Handle to the Nexus video decoder
	hWindow - Active window to remove input from

Returns:
	BERR_SUCCESS - If the video was stopped.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
***************************************************************************/
B_Error B_Avis_StopInputVideoSourceDigitalTV(B_Avis_Handle hAv_input_select, 
								 NEXUS_VideoDecoderHandle hVideoDecoder,
								 NEXUS_VideoWindowHandle hWindow);

/***************************************************************************
Summary:
Stops the currently active analog TV input for the given output window

Description:
Stops the currently active analog TV input for the given output window

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	hAnalogVideoDecoder - Handle to the Nexus analog video decoder
	hWindow - Active window to remove input from

Returns:
	BERR_SUCCESS - If the video was stopped.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
***************************************************************************/
B_Error B_Avis_StopInputVideoSourceAnalogTV(B_Avis_Handle hAv_input_select, 
								NEXUS_AnalogVideoDecoderHandle hAnalogVideoDecoder,
								NEXUS_VideoWindowHandle hWindow);

/***************************************************************************
Summary:
Stops the currently active composite video input source on the given output window

Description:
Stops the currently active composite video input source on the given output window

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	hAnalogVideoDecoder - Handle to the Nexus analog video decoder
	hWindow - Active window to remove input from

Returns:
	BERR_SUCCESS - If the video was stopped.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
***************************************************************************/
B_Error B_Avis_StopInputVideoSourceComposite(B_Avis_Handle hAv_input_select, 
							   NEXUS_AnalogVideoDecoderHandle hAnalogVideoDecoder,
							   NEXUS_VideoWindowHandle hWindow);

/***************************************************************************
Summary:
Stops the currently active s-video video input source on the given output window

Description:
Stops the currently active s-video video input source on the given output window

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	hAnalogVideoDecoder - Handle to the Nexus analog video decoder
	hWindow - Active window to remove input from

Returns:
	BERR_SUCCESS - If the video was stopped.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
***************************************************************************/
B_Error B_Avis_StopInputVideoSourceSvideo(B_Avis_Handle hAv_input_select, 
							   NEXUS_AnalogVideoDecoderHandle hAnalogVideoDecoder,
							   NEXUS_VideoWindowHandle hWindow);

/***************************************************************************
Summary:
Stops the currently active component video input for the given output window

Description:
Stops the currently active component video input for the given output window

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	hAnalogVideoDecoder - Handle to the Nexus analog video decoder
	hWindow - Active window to remove input from

Returns:
	BERR_SUCCESS - If the video was stopped.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
***************************************************************************/
B_Error B_Avis_StopInputVideoSourceComponent(B_Avis_Handle hAv_input_select, 
									  NEXUS_AnalogVideoDecoderHandle hAnalogVideoDecoder,
									  NEXUS_VideoWindowHandle hWindow);

/***************************************************************************
Summary:
Stops the currently active RGB video input on the given output window

Description:
Stops the currently active RGB video input on the given output window

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	hAnalogVideoDecoder - Handle to the Nexus analog video decoder
	hWindow - Active window to remove input from

Returns:
	BERR_SUCCESS - If the video was stopped.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
***************************************************************************/
B_Error B_Avis_StopInputVideoSourceRGB(B_Avis_Handle hAv_input_select, 
								NEXUS_AnalogVideoDecoderHandle	hAnalogVideoDecoder,
								NEXUS_VideoWindowHandle hWindow);

/***************************************************************************
Summary:
Stops the currently active HDMI video input on the given output window

Description:
Stops the currently active HDMI video input on the given output window

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	hHdmiInput - Handle to Nexus HDMI input
	hWindow - Active window to remove input from

Returns:
	BERR_SUCCESS - If the video was stopped.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
***************************************************************************/
B_Error B_Avis_StopInputVideoSourceHDMI(B_Avis_Handle hAv_input_select, 
								 NEXUS_HdmiInputHandle	hHdmiInput,
								 NEXUS_VideoWindowHandle hWindow);

/***************************************************************************
Summary:
Starts the digital TV audio source

Description:
Currently performed by decode app lib so this is a no-op.

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	audioDacIndex - Audio output DAC to start the audio on
	spdifIndex - spdif channel to start the audio on

Returns:
	BERR_SUCCESS - If the audio was started.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
	BERR_NOT_SUPPORTED - If the requested configuration is not supported on this platform.
***************************************************************************/
B_Error B_Avis_StartInputAudioSourceDigitalTV(B_Avis_Handle hAv_input_select,
								unsigned int audioDacIndex,
								unsigned int spdifIndex);

/***************************************************************************
Summary:
Starts the Analog TV audio source

Description:
Starts the Analog TV audio source

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	hRfAudioDecoder - Handle to Nexus RF audio decoder
	audioDacIndex - Audio output DAC to start the audio on
	spdifIndex - spdif channel to start the audio on

Returns:
	BERR_SUCCESS - If the audio was started.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
	BERR_NOT_SUPPORTED - If the requested configuration is not supported on this platform.
***************************************************************************/
B_Error B_Avis_StartInputAudioSourceAnalogTV(B_Avis_Handle hAv_input_select,
									  NEXUS_RfAudioDecoderHandle hRfAudioDecoder,
									  unsigned int audioDacIndex,
									  unsigned int spdifIndex);

/***************************************************************************
Summary:
Starts the composite audio source

Description:
Starts the composite audio source

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	sourceIndex - sub index of the source input type
	hAnalogAudioDecoder - Handle to Nexus analog audio decoder
	audioDacIndex - Audio output DAC to start the audio on
	spdifIndex - spdif channel to start the audio on

Returns:
	BERR_SUCCESS - If the audio was started.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
	BERR_NOT_SUPPORTED - If the requested configuration is not supported on this platform.
***************************************************************************/
B_Error B_Avis_StartInputAudioSourceComposite(B_Avis_Handle hAv_input_select,
									   unsigned int sourceIndex,
									   NEXUS_AnalogAudioDecoderHandle hAnalogAudioDecoder,
									   unsigned int audioDacIndex,
									   unsigned int spdifIndex);

/***************************************************************************
Summary:
Starts the svideo audio source

Description:
Starts the svideo audio source

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	sourceIndex - sub index of the source input type
	hAnalogAudioDecoder - Handle to Nexus analog audio decoder
	audioDacIndex - Audio output DAC to start the audio on
	spdifIndex - spdif channel to start the audio on

Returns:
	BERR_SUCCESS - If the audio was started.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
	BERR_NOT_SUPPORTED - If the requested configuration is not supported on this platform.
***************************************************************************/
B_Error B_Avis_StartInputAudioSourceSvideo(B_Avis_Handle hAv_input_select,
									unsigned int sourceIndex,
									NEXUS_AnalogAudioDecoderHandle hAnalogAudioDecoder,
									unsigned int audioDacIndex,
									unsigned int spdifIndex);

/***************************************************************************
Summary:
Starts the component audio source

Description:
Starts the composite audio source

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	sourceIndex - sub index of the source input type
	hAnalogAudioDecoder - Handle to Nexus analog audio decoder
	audioDacIndex - Audio output DAC to start the audio on
	spdifIndex - spdif channel to start the audio on

Returns:
	BERR_SUCCESS - If the audio was started.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
	BERR_NOT_SUPPORTED - If the requested configuration is not supported on this platform.
***************************************************************************/
B_Error B_Avis_StartInputAudioSourceComponent(B_Avis_Handle hAv_input_select,
									   unsigned int sourceIndex,
									   NEXUS_AnalogAudioDecoderHandle hAnalogAudioDecoder,
									   unsigned int audioDacIndex,
									   unsigned int spdifIndex);

/***************************************************************************
Summary:
Starts the HDMI audio source

Description:
Starts the HDMI audio source

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	hHdmiInput - Nexus Handle representing the source HDMI input
	hAudioDecoder - Represents the Nexus audio decorder handle
	audioDacIndex - Audio output DAC to start the audio on
	spdifIndex - spdif channel to start the audio on

Returns:
	BERR_SUCCESS - If the audio was started.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
	BERR_NOT_SUPPORTED - If the requested configuration is not supported on this platform.
***************************************************************************/
B_Error B_Avis_StartInputAudioSourceHDMI(B_Avis_Handle hAv_input_select,
								  NEXUS_HdmiInputHandle	hHdmiInput,
								  NEXUS_AudioDecoderHandle hAudioDecoder,
								  unsigned int audioDacIndex);

/***************************************************************************
Summary:
Stops the Digital TV audio input

Description:
Currently performed by decode app lib so this is a no-op.

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	audioDacIndex - Audio output DAC the output is playing on
	spdifIndex - spdif channel the audio is playing on

Returns:
	BERR_SUCCESS - If the audio was stopped.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
***************************************************************************/
B_Error B_Avis_StopInputAudioSourceDigitalTV(B_Avis_Handle hAv_input_select,
									   unsigned int audioDacIndex,
									   unsigned int spdifIndex);

/***************************************************************************
Summary:
Stops the Analog TV audio output

Description:
Stops the Analog TV audio output

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	hRfAudioDecoder - Handle to Nexus RF audio decoder
	audioDacIndex - Audio output DAC the output is playing on
	spdifIndex - spdif channel the audio is playing on

Returns:
	BERR_SUCCESS - If the audio was stopped.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
***************************************************************************/
B_Error B_Avis_StopInputAudioSourceAnalogTV(B_Avis_Handle hAv_input_select,
									 NEXUS_RfAudioDecoderHandle	hRfAudioDecoder,
									 unsigned int audioDacIndex,
									 unsigned int spdifIndex);

/***************************************************************************
Summary:
Stops the composite audio source

Description:
Stops the composite audio source

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	hAnalogAudioDecoder - Handle to Nexus analog audio decoder
	audioDacIndex - Audio output DAC the output is playing on
	spdifIndex - spdif channel the audio is playing on

Returns:
	BERR_SUCCESS - If the audio was stopped.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
***************************************************************************/
B_Error B_Avis_StopInputAudioSourceComposite(B_Avis_Handle hAv_input_select,
									  NEXUS_AnalogAudioDecoderHandle hAnalogAudioDecoder,
									  unsigned int audioDacIndex,
									  unsigned int spdifIndex);

/***************************************************************************
Summary:
Stops the svideo audio source

Description:
Stops the svideo audio source

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	hAnalogAudioDecoder - Handle to Nexus analog audio decoder
	audioDacIndex - Audio output DAC the output is playing on
	spdifIndex - spdif channel the audio is playing on

Returns:
	BERR_SUCCESS - If the audio was stopped.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
***************************************************************************/
B_Error B_Avis_StopInputAudioSourceSvideo(B_Avis_Handle hAv_input_select,
								   NEXUS_AnalogAudioDecoderHandle hAnalogAudioDecoder,
								   unsigned int audioDacIndex,
								   unsigned int spdifIndex);

/***************************************************************************
Summary:
Stops the component audio source

Description:
Stops the component audio source

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	hAnalogAudioDecoder - Handle to Nexus analog audio decoder
	audioDacIndex - Audio output DAC the output is playing on
	spdifIndex - spdif channel the audio is playing on

Returns:
	BERR_SUCCESS - If the audio was stopped.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
***************************************************************************/
B_Error B_Avis_StopInputAudioSourceComponent(B_Avis_Handle hAv_input_select,
									  NEXUS_AnalogAudioDecoderHandle hAnalogAudioDecoder,
									  unsigned int audioDacIndex,
									  unsigned int spdifIndex);

/***************************************************************************
Summary:
Stops the HDMI audio input

Description:
Stops the HDMI audio input

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	hAudioDecoder - Represents the Nexus audio decorder handle
	audioDacIndex - Audio output DAC the output is playing on

Returns:
	BERR_SUCCESS - If the audio was stopped.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
***************************************************************************/
B_Error B_Avis_StopInputAudioSourceHDMI(B_Avis_Handle hAv_input_select,
								 NEXUS_AudioDecoderHandle hAudioDecoder,
								 unsigned int audioDacIndex);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef B_Avis_LIB_H__ */
