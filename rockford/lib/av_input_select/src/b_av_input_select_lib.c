/***************************************************************************
*     (c)2003-2008 Broadcom Corporation
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
* Description:
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "b_av_input_select_lib.h"
#include "bdbg.h"
#include "bkni.h"

BDBG_MODULE(b_av_input_select_lib);


/***************************************************************************
Summary:
Private handle for Av_input_select App Lib
***************************************************************************/
typedef struct B_Avis_P_Struct
{
	B_Avis_Settings settings;    
} B_Avis_P_Struct, *B_Avis_P_Handle;

/***************************************************************************
Summary:
Private function prototypes
***************************************************************************/
static B_Error B_Avis_P_StartInputVideoSource_Common(B_Avis_Handle,
								NEXUS_VideoInput,
								NEXUS_VideoWindowHandle);
static B_Error B_Avis_P_StopInputVideoSource_Common(B_Avis_Handle, 
								 NEXUS_VideoInput,
								 NEXUS_VideoWindowHandle hWindow);

/***************************************************************************
Summary:
This function returns the default and recommended values for the App Lib
public settings. A pointer to a valid B_Avis_Settings structure must be
provide or an error will be returned.
***************************************************************************/
B_Error B_Avis_GetDefaultSettings(B_Avis_Settings *pAv_input_selectSettings)
{
	BDBG_ENTER(B_Avis_GetDefaultSettings);

	if (NULL != pAv_input_selectSettings) return BERR_INVALID_PARAMETER;
	BKNI_Memset( pAv_input_selectSettings, 0, sizeof(B_Avis_Settings) );

	BDBG_LEAVE(B_Avis_GetDefaultSettings);
	return BERR_SUCCESS;
}

/***************************************************************************
Summary:
This function initializes the App Lib based on the settings selected. A shallow
copy of the B_Avis_Settings structure is made in this call. The private App Lib
structure is malloc'ed.
***************************************************************************/
B_Avis_Handle B_Avis_Open(B_Avis_Settings *pAv_input_selectSettings)
{
	B_Avis_P_Handle hAv_input_select = NULL;

	BDBG_ENTER(B_Avis_Open);
	if (NULL == pAv_input_selectSettings)
	{
		BDBG_LEAVE(B_Avis_Open);
		return NULL;
	}

	B_Os_Init();

	hAv_input_select = (B_Avis_P_Handle) B_Os_Calloc(1, sizeof(B_Avis_P_Struct));
	if(NULL == hAv_input_select)
	{
		BDBG_LEAVE(B_Avis_Open);
		return NULL;
	}

	hAv_input_select->settings = *pAv_input_selectSettings;

	BDBG_LEAVE(B_Avis_Open);
	return (B_Avis_Handle) hAv_input_select;
}

/***************************************************************************
Summary:
This function de-initializes the App Lib. The private App Lib structure is freed.
***************************************************************************/
B_Error B_Avis_Close(B_Avis_Handle hAv_input_select)
{
	BDBG_ENTER(B_Avis_Close);
	BDBG_ASSERT(NULL != hAv_input_select);

	B_Os_Free(hAv_input_select);
	B_Os_Uninit();

	BDBG_LEAVE(B_Avis_Close);
	return BERR_SUCCESS;
}

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
							  B_Avis_Source pip)
{
	BSTD_UNUSED(hAv_input_select);
	BDBG_ENTER(B_Avis_IsInputSourcePossible);

	/* Ok if either is not used */
	if(main==B_Avis_eNothing || pip==B_Avis_eNothing) return BERR_SUCCESS;

	/* Cant do two decodes */
	if(main==B_Avis_eDigitalTV && pip==B_Avis_eDigitalTV) return BERR_NOT_SUPPORTED;

	/* Cant do two analog captures to the VDEC */
	if((main==B_Avis_eAnalogTV ||
		main==B_Avis_eComposite ||
		main==B_Avis_eSvideo ||
		main==B_Avis_eComponent ||
		main==B_Avis_eRGB) &&
		(pip==B_Avis_eAnalogTV ||
		pip==B_Avis_eComposite ||
		 pip==B_Avis_eSvideo ||
		pip==B_Avis_eComponent ||
		pip==B_Avis_eRGB)) return BERR_NOT_SUPPORTED;

	BDBG_LEAVE(B_Avis_IsInputSourcePossible);
	return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Setup the source pending and output window based on video input

Description:
Setup the source pending and output window based on video input

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	videoInput - Video input
	hWindow - Active window to attach input to

Returns:
	BERR_SUCCESS - If the video was started.
***************************************************************************/
static B_Error B_Avis_P_StartInputVideoSource_Common(B_Avis_Handle hAv_input_select,
								NEXUS_VideoInput videoInput,
								NEXUS_VideoWindowHandle hWindow)
{
	B_Avis_P_Handle hPAv_input_select = (B_Avis_P_Handle) hAv_input_select;
	NEXUS_VideoInputSettings videoInputSettings;
	NEXUS_VideoWindowSettings 	windowSettings;

	BDBG_ENTER(B_Avis_P_StartInputVideoSource_Common);

	NEXUS_VideoWindow_AddInput(hWindow, videoInput);

	/* enable app-driven source pending */
	if(hPAv_input_select->settings.sourcePending.callback)
	{
		NEXUS_VideoInput_SetResumeMode(videoInput, NEXUS_VideoInputResumeMode_eManual);

		NEXUS_VideoInput_GetSettings(videoInput, &videoInputSettings);
		videoInputSettings.sourcePending = hPAv_input_select->settings.sourcePending;
		NEXUS_VideoInput_SetSettings(videoInput, &videoInputSettings);
	}
	else
	{
		NEXUS_VideoInput_SetResumeMode(videoInput, NEXUS_VideoInputResumeMode_eAuto);
	}

	/* Make window visible */
	NEXUS_VideoWindow_GetSettings(hWindow, &windowSettings);
	windowSettings.visible = true;
	NEXUS_VideoWindow_SetSettings(hWindow, &windowSettings);

	BDBG_LEAVE(B_Avis_P_StartInputVideoSource_Common);
	return BERR_SUCCESS;
}

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
								NEXUS_VideoWindowHandle hWindow)
{
	B_Error err;
	NEXUS_VideoInput videoInput;

	BDBG_ENTER(B_Avis_StartInputVideoSourceDigitalTV);

	if(hWindow==NULL || hVideoDecoder==NULL)
	{
		BDBG_LEAVE(StartInputVideoSourceDigitalTV);
		return BERR_INVALID_PARAMETER;
	}

#if 1
  /* Done in the example decoder lib */
	videoInput = NEXUS_VideoDecoder_GetConnector(hVideoDecoder);
	err = B_Avis_P_StartInputVideoSource_Common(hAv_input_select,
										  videoInput,
										  hWindow);
#endif
	BDBG_LEAVE(B_Avis_StartInputVideoSourceDigitalTV);
	return err;
}

/***************************************************************************
Summary:
Starts the Analog TV source on the given output window

Description:
Starts the Analog TV source on the given output window

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	hAnalogVideoDecoder - Handle representing the source
	hWindow - Active window to attach input to

Returns:
	BERR_SUCCESS - If the video was started.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
	BERR_NOT_SUPPORTED - If the requested configuration is not supported on this platform.
***************************************************************************/
B_Error B_Avis_StartInputVideoSourceAnalogTV(B_Avis_Handle hAv_input_select,
								NEXUS_AnalogVideoDecoderHandle	hAnalogVideoDecoder,
								NEXUS_VideoWindowHandle hWindow)
{
	B_Error err;
	NEXUS_VideoInput videoInput;
	NEXUS_AnalogVideoDecoderSettings analogVideoDecoderSettings;
	NEXUS_PlatformConfiguration	platformConfig;
	NEXUS_FrontendHandle		frontend;
	int i;

	BDBG_ENTER(B_Avis_StartInputVideoSourceAnalogTV);
	if(hWindow==NULL || hAnalogVideoDecoder==NULL)
	{
		BDBG_LEAVE(StartInputVideoSourceAnalogTV);
		return BERR_INVALID_PARAMETER;
	}

	NEXUS_Platform_GetConfiguration(&platformConfig);

	/* Test for an analog TV frontend */
	for ( i = 0; i < NEXUS_MAX_FRONTENDS; i++ )
	{
		NEXUS_FrontendCapabilities capabilities;
		frontend = platformConfig.frontend[i];
		if (frontend) {
			NEXUS_Frontend_GetCapabilities(frontend, &capabilities);
			/* Does this frontend support analog? */
			if ( capabilities.analog && capabilities.ifd )
			{
				break;
			}
		}
	}
	if (NULL == frontend ) return BERR_NOT_SUPPORTED;

	/* connect analog decoder to display */
	NEXUS_AnalogVideoDecoder_GetDefaultSettingsForVideoInput(NEXUS_Frontend_GetAnalogVideoConnector(frontend), 
															 &analogVideoDecoderSettings);
	NEXUS_AnalogVideoDecoder_SetSettings(hAnalogVideoDecoder, &analogVideoDecoderSettings);
	videoInput = NEXUS_AnalogVideoDecoder_GetConnector(hAnalogVideoDecoder);
	err = B_Avis_P_StartInputVideoSource_Common(hAv_input_select,
										  videoInput,
										  hWindow);

	BDBG_LEAVE(B_Avis_StartInputVideoSourceAnalogTV);
	return err;
}

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
								NEXUS_AnalogVideoDecoderHandle	hAnalogVideoDecoder,
								NEXUS_VideoWindowHandle hWindow)
{
	B_Error err;
	NEXUS_VideoInput videoInput;
	NEXUS_AnalogVideoDecoderSettings analogVideoDecoderSettings;
	NEXUS_PlatformConfiguration	platformConfig;

	BDBG_ENTER(B_Avis_StartInputVideoSourceComposite);
	if(hWindow==NULL || hAnalogVideoDecoder==NULL)
	{
		BDBG_LEAVE(B_Avis_StartInputVideoSourceComposite);
		return BERR_INVALID_PARAMETER;
	}

	if(sourceIndex>=NEXUS_NUM_COMPOSITE_INPUTS)
	{
		BDBG_LEAVE(B_Avis_StartInputVideoSourceComposite);
		return BERR_INVALID_PARAMETER;
	}
	NEXUS_Platform_GetConfiguration(&platformConfig);

	if (!platformConfig.inputs.composite[sourceIndex]) return BERR_INVALID_PARAMETER;
	NEXUS_AnalogVideoDecoder_GetDefaultSettingsForVideoInput(
		NEXUS_CompositeInput_GetConnector(platformConfig.inputs.composite[sourceIndex]), 
		&analogVideoDecoderSettings);
	NEXUS_AnalogVideoDecoder_SetSettings(hAnalogVideoDecoder, &analogVideoDecoderSettings);
	videoInput = NEXUS_AnalogVideoDecoder_GetConnector(hAnalogVideoDecoder);
	err = B_Avis_P_StartInputVideoSource_Common(hAv_input_select,
										  videoInput,
										  hWindow);

	BDBG_LEAVE(B_Avis_StartInputVideoSourceComposite);
	return err;
}

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
								NEXUS_AnalogVideoDecoderHandle	hAnalogVideoDecoder,
								NEXUS_VideoWindowHandle hWindow)
{
	B_Error err;
	NEXUS_VideoInput videoInput;
	NEXUS_AnalogVideoDecoderSettings analogVideoDecoderSettings;
	NEXUS_PlatformConfiguration	platformConfig;

	BDBG_ENTER(B_Avis_StartInputVideoSourceSvideo);
	if(hWindow==NULL || hAnalogVideoDecoder==NULL)
	{
		BDBG_LEAVE(B_Avis_StartInputVideoSourceSvideo);
		return BERR_INVALID_PARAMETER;
	}

	if(sourceIndex>=NEXUS_NUM_SVIDEO_INPUTS)
	{
		BDBG_LEAVE(B_Avis_StartInputVideoSourceSvideo);
		return BERR_INVALID_PARAMETER;
	}
	NEXUS_Platform_GetConfiguration(&platformConfig);

	if (!platformConfig.inputs.svideo[sourceIndex]) return BERR_INVALID_PARAMETER;
	NEXUS_AnalogVideoDecoder_GetDefaultSettingsForVideoInput(
		NEXUS_SvideoInput_GetConnector(platformConfig.inputs.svideo[sourceIndex]), 
		&analogVideoDecoderSettings);
	NEXUS_AnalogVideoDecoder_SetSettings(hAnalogVideoDecoder, &analogVideoDecoderSettings);
	videoInput = NEXUS_AnalogVideoDecoder_GetConnector(hAnalogVideoDecoder);
	err = B_Avis_P_StartInputVideoSource_Common(hAv_input_select,
										  videoInput,
										  hWindow);

	BDBG_LEAVE(B_Avis_StartInputVideoSourceSvideo);
	return err;
}

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
								  NEXUS_AnalogVideoDecoderHandle	hAnalogVideoDecoder,
								  NEXUS_VideoWindowHandle hWindow)
{
	B_Error err;
	NEXUS_VideoInput videoInput;
	NEXUS_AnalogVideoDecoderSettings analogVideoDecoderSettings;
	NEXUS_PlatformConfiguration	platformConfig;

	BDBG_ENTER(B_Avis_StartInputVideoSourceComponent);
	if(hWindow==NULL || hAnalogVideoDecoder==NULL)
	{
		BDBG_LEAVE(B_Avis_StartInputVideoSourceComponent);
		return BERR_INVALID_PARAMETER;
	}

	if(sourceIndex>=NEXUS_NUM_COMPONENT_INPUTS)
	{
		BDBG_LEAVE(B_Avis_StartInputVideoSourceComponent);
		return BERR_INVALID_PARAMETER;
	}

	NEXUS_Platform_GetConfiguration(&platformConfig);

	if (!platformConfig.inputs.component[sourceIndex]) return BERR_INVALID_PARAMETER;

	/* This gets VDEC defaults for component input. */
	NEXUS_AnalogVideoDecoder_GetDefaultSettingsForVideoInput(
		NEXUS_ComponentInput_GetConnector(platformConfig.inputs.component[sourceIndex]), 
		&analogVideoDecoderSettings);

	/* analogVideoDecoderSettings.input has been assigned. Now connect to the VDEC. Analog video will be displayed. */
	NEXUS_AnalogVideoDecoder_SetSettings(hAnalogVideoDecoder, &analogVideoDecoderSettings);

	videoInput = NEXUS_AnalogVideoDecoder_GetConnector(hAnalogVideoDecoder);
	err = B_Avis_P_StartInputVideoSource_Common(hAv_input_select,
										  videoInput,
										  hWindow);

	BDBG_LEAVE(B_Avis_StartInputVideoSourceComponent);
	return err;
}

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
							NEXUS_VideoWindowHandle hWindow)
{
	B_Error err;
	NEXUS_VideoInput videoInput;
	NEXUS_AnalogVideoDecoderSettings analogVideoDecoderSettings;
	NEXUS_PlatformConfiguration	platformConfig;
	NEXUS_PcInputHandle			pcInput;
	NEXUS_VideoWindowSettings 	windowSettings;
	NEXUS_PcInputSettings		pcInputSettings;
	NEXUS_PcInputHandle			hInputPc;
	NEXUS_TimebaseSettings timebaseSettings;

	BDBG_ENTER(B_Avis_StartInputVideoSourceRGB);
	if(hWindow==NULL || hAnalogVideoDecoder==NULL)
	{
		BDBG_LEAVE(B_Avis_StartInputVideoSourceRGB);
		return BERR_INVALID_PARAMETER;
	}

	NEXUS_Platform_GetConfiguration(&platformConfig);

	if (!platformConfig.inputs.pc[0]) return BERR_INVALID_PARAMETER;
	pcInput = platformConfig.inputs.pc[0];
	NEXUS_PcInput_GetDefaultSettings( &pcInputSettings );
	pcInputSettings.manualAdjustEnabled                 = true;

	pcInputSettings.adc[NEXUS_AnalogVideoChannel_eR]    = 19; /* TODO: move ADC assignment to platform */
	pcInputSettings.adc[NEXUS_AnalogVideoChannel_eG]    = 20;
	pcInputSettings.adc[NEXUS_AnalogVideoChannel_eB]    = 21;

	hInputPc = NEXUS_PcInput_Open( 0, &pcInputSettings );

	NEXUS_AnalogVideoDecoder_GetDefaultSettingsForVideoInput(NEXUS_PcInput_GetConnector(pcInput), 
															 &analogVideoDecoderSettings);
	/* use VDC default setting for CTI, 3Dcomb*/
	analogVideoDecoderSettings.ctiSettings.useDynamicDefaults = true;
	analogVideoDecoderSettings.s3DComb.useDynamicDefaults = true;    

	NEXUS_AnalogVideoDecoder_SetSettings(hAnalogVideoDecoder, &analogVideoDecoderSettings);

	/* set timebase to vdec */
	NEXUS_AnalogVideoDecoder_GetSettings(hAnalogVideoDecoder, &analogVideoDecoderSettings);
	NEXUS_Timebase_GetSettings(analogVideoDecoderSettings.timebase, &timebaseSettings);
	timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eAnalog;
	NEXUS_Timebase_SetSettings(analogVideoDecoderSettings.timebase, &timebaseSettings);

	videoInput = NEXUS_AnalogVideoDecoder_GetConnector(hAnalogVideoDecoder);
	err = B_Avis_P_StartInputVideoSource_Common(hAv_input_select,
										  videoInput,
										  hWindow);

	NEXUS_VideoWindow_GetSettings(hWindow, &windowSettings);
	windowSettings.letterBoxDetect = true;
	NEXUS_VideoWindow_SetSettings(hWindow, &windowSettings);

	BDBG_LEAVE(B_Avis_StartInputVideoSourceRGB);
	return err;
}

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
							 NEXUS_VideoWindowHandle hWindow)
{
	B_Error err;
	NEXUS_VideoInput videoInput;

	BDBG_ENTER(B_Avis_StartInputVideoSourceHDMI);
	if(hWindow==NULL || hHdmiInput==NULL)
	{
		BDBG_LEAVE(B_Avis_StartInputVideoSourceHDMI);
		return BERR_INVALID_PARAMETER;
	}

	videoInput = NEXUS_HdmiInput_GetVideoConnector(hHdmiInput);
	err = B_Avis_P_StartInputVideoSource_Common(hAv_input_select,
										  videoInput,
										  hWindow);

	BDBG_LEAVE(B_Avis_StartInputVideoSourceHDMI);
	return err;
}

/***************************************************************************
Summary:
Stops the currently active input source in the given output window

Description:
Stops the currently active input source in the given output window

Input:
	hAv_input_select - Handle to the open Av_input_select App Lib
	videoInput - Video input
	hWindow - Active window to remove input from

Returns:
	BERR_SUCCESS - If the video was stopped.
***************************************************************************/
static B_Error B_Avis_P_StopInputVideoSource_Common(B_Avis_Handle hAv_input_select, 
								 NEXUS_VideoInput videoInput,
								 NEXUS_VideoWindowHandle hWindow)
{
	NEXUS_VideoInputSettings videoInputSettings;
	NEXUS_VideoWindowSettings 	windowSettings;
	BSTD_UNUSED(hAv_input_select);

	BDBG_ENTER(B_Avis_P_StopInputVideoSource_Common);

	NEXUS_VideoWindow_RemoveInput(hWindow, videoInput);

	/* disable app-driven source pending */
	NEXUS_VideoInput_SetResumeMode(videoInput, NEXUS_VideoInputResumeMode_eAuto);

	NEXUS_VideoInput_GetSettings(videoInput, &videoInputSettings);
	videoInputSettings.sourcePending.callback = NULL;
	NEXUS_VideoInput_SetSettings(videoInput, &videoInputSettings);

	NEXUS_VideoWindow_GetSettings(hWindow, &windowSettings);
	windowSettings.visible = false;
	NEXUS_VideoWindow_SetSettings(hWindow, &windowSettings);

	NEXUS_VideoInput_Shutdown(videoInput);

	BDBG_LEAVE(B_Avis_P_StopInputVideoSource_Common);
	return BERR_SUCCESS;
}

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
								 NEXUS_VideoWindowHandle hWindow)
{
	B_Error err;
	NEXUS_VideoInput videoInput;

	BDBG_ENTER(B_Avis_StopInputVideoSourceDigitalTV);
	if(hWindow==NULL || hVideoDecoder==NULL)
	{
		BDBG_LEAVE(B_Avis_StopInputVideoSourceDigitalTV);
		return BERR_INVALID_PARAMETER;
	}

	videoInput = NEXUS_VideoDecoder_GetConnector(hVideoDecoder);
	err = B_Avis_P_StopInputVideoSource_Common(hAv_input_select, 
										videoInput,
										hWindow);

	BDBG_LEAVE(B_Avis_StopInputVideoSourceDigitalTV);
	return err;
}

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
								NEXUS_AnalogVideoDecoderHandle	hAnalogVideoDecoder,
								NEXUS_VideoWindowHandle hWindow)
{
	B_Error err;
	NEXUS_VideoInput videoInput;
	NEXUS_AnalogVideoDecoderSettings analogVideoDecoderSettings;

	BDBG_ENTER(B_Avis_StopInputVideoSourceAnalogTV);
	if(hWindow==NULL || hAnalogVideoDecoder==NULL)
	{
		BDBG_LEAVE(B_Avis_StopInputVideoSourceAnalogTV);
		return BERR_INVALID_PARAMETER;
	}

	NEXUS_AnalogVideoDecoder_GetSettings(hAnalogVideoDecoder, &analogVideoDecoderSettings);
	analogVideoDecoderSettings.input = NULL;
	NEXUS_AnalogVideoDecoder_SetSettings(hAnalogVideoDecoder, &analogVideoDecoderSettings);

	videoInput = NEXUS_AnalogVideoDecoder_GetConnector(hAnalogVideoDecoder);
	err = B_Avis_P_StopInputVideoSource_Common(hAv_input_select, 
										videoInput,
										hWindow);
	BDBG_LEAVE(B_Avis_StopInputVideoSourceAnalogTV);
	return err;
}

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
							   NEXUS_AnalogVideoDecoderHandle	hAnalogVideoDecoder,
							   NEXUS_VideoWindowHandle hWindow)
{
	B_Error err;
	NEXUS_VideoInput videoInput;
	NEXUS_AnalogVideoDecoderSettings analogVideoDecoderSettings;

	BDBG_ENTER(B_Avis_StopInputVideoSourceComposite);
	if(hWindow==NULL || hAnalogVideoDecoder==NULL)
	{
		BDBG_LEAVE(B_Avis_StopInputVideoSourceComposite);
		return BERR_INVALID_PARAMETER;
	}

	NEXUS_AnalogVideoDecoder_GetSettings(hAnalogVideoDecoder, &analogVideoDecoderSettings);
	analogVideoDecoderSettings.input = NULL;
	NEXUS_AnalogVideoDecoder_SetSettings(hAnalogVideoDecoder, &analogVideoDecoderSettings);

	videoInput = NEXUS_AnalogVideoDecoder_GetConnector(hAnalogVideoDecoder);
	err = B_Avis_P_StopInputVideoSource_Common(hAv_input_select, 
										videoInput,
										hWindow);
	BDBG_LEAVE(B_Avis_StopInputVideoSourceComposite);
	return err;
}

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
							   NEXUS_AnalogVideoDecoderHandle	hAnalogVideoDecoder,
							   NEXUS_VideoWindowHandle hWindow)
{
	B_Error err;
	NEXUS_VideoInput videoInput;
	NEXUS_AnalogVideoDecoderSettings analogVideoDecoderSettings;

	BDBG_ENTER(B_Avis_StopInputVideoSourceSvideo);
	if(hWindow==NULL || hAnalogVideoDecoder==NULL)
	{
		BDBG_LEAVE(B_Avis_StopInputVideoSourceSvideo);
		return BERR_INVALID_PARAMETER;
	}

	NEXUS_AnalogVideoDecoder_GetSettings(hAnalogVideoDecoder, &analogVideoDecoderSettings);
	analogVideoDecoderSettings.input = NULL;
	NEXUS_AnalogVideoDecoder_SetSettings(hAnalogVideoDecoder, &analogVideoDecoderSettings);

	videoInput = NEXUS_AnalogVideoDecoder_GetConnector(hAnalogVideoDecoder);
	err = B_Avis_P_StopInputVideoSource_Common(hAv_input_select, 
										videoInput,
										hWindow);
	BDBG_LEAVE(B_Avis_StopInputVideoSourceSvideo);
	return err;
}

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
									  NEXUS_AnalogVideoDecoderHandle	hAnalogVideoDecoder,
									  NEXUS_VideoWindowHandle hWindow)
{
	B_Error err;
	NEXUS_VideoInput videoInput;
	NEXUS_AnalogVideoDecoderSettings analogVideoDecoderSettings;

	BDBG_ENTER(B_Avis_StopInputVideoSourceComponent);
	if(hWindow==NULL || hAnalogVideoDecoder==NULL)
	{
		BDBG_LEAVE(B_Avis_StopInputVideoSourceComponent);
		return BERR_INVALID_PARAMETER;
	}

	NEXUS_AnalogVideoDecoder_GetSettings(hAnalogVideoDecoder, &analogVideoDecoderSettings);
	analogVideoDecoderSettings.input = NULL;
	NEXUS_AnalogVideoDecoder_SetSettings(hAnalogVideoDecoder, &analogVideoDecoderSettings);

	videoInput = NEXUS_AnalogVideoDecoder_GetConnector(hAnalogVideoDecoder);
	err = B_Avis_P_StopInputVideoSource_Common(hAv_input_select, 
										videoInput,
										hWindow);
	BDBG_LEAVE(B_Avis_StopInputVideoSourceComponent);
	return err;
}

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
								NEXUS_VideoWindowHandle hWindow)
{
	B_Error err;
	NEXUS_VideoInput videoInput;
	NEXUS_AnalogVideoDecoderSettings analogVideoDecoderSettings;

	BDBG_ENTER(B_Avis_StopInputVideoSourceRGB);
	if(hWindow==NULL || hAnalogVideoDecoder==NULL)
	{
		BDBG_LEAVE(B_Avis_StopInputVideoSourceRGB);
		return BERR_INVALID_PARAMETER;
	}

	analogVideoDecoderSettings.input = NULL;
	NEXUS_AnalogVideoDecoder_SetSettings(hAnalogVideoDecoder, &analogVideoDecoderSettings);

	videoInput = NEXUS_AnalogVideoDecoder_GetConnector(hAnalogVideoDecoder);
	err = B_Avis_P_StopInputVideoSource_Common(hAv_input_select, 
										videoInput,
										hWindow);
	BDBG_LEAVE(B_Avis_StopInputVideoSourceRGB);
	return err;
}

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
								 NEXUS_VideoWindowHandle hWindow)
{
	B_Error err;
	NEXUS_VideoInput videoInput;

	BDBG_ENTER(B_Avis_StopInputVideoSourceHDMI);
	if(hWindow==NULL || hHdmiInput==NULL)
	{
		BDBG_LEAVE(B_Avis_StopInputVideoSourceHDMI);
		return BERR_INVALID_PARAMETER;
	}

	videoInput = NEXUS_HdmiInput_GetVideoConnector(hHdmiInput);
	err = B_Avis_P_StopInputVideoSource_Common(hAv_input_select, 
										videoInput,
										hWindow);
	BDBG_LEAVE(B_Avis_StopInputVideoSourceHDMI);
	return err;
}

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
								unsigned int spdifIndex)
{
	BSTD_UNUSED(hAv_input_select);
	BSTD_UNUSED(audioDacIndex);
	BSTD_UNUSED(spdifIndex);
	BDBG_ENTER(B_Avis_StartInputAudioSourceDigitalTV);
	BDBG_LEAVE(B_Avis_StartInputAudioSourceDigitalTV);
	return BERR_SUCCESS;
}

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
								NEXUS_RfAudioDecoderHandle	hRfAudioDecoder,
								unsigned int audioDacIndex,
								unsigned int spdifIndex)
{
	NEXUS_PlatformConfiguration	platformConfig;
	NEXUS_FrontendHandle		frontend;
	NEXUS_RfAudioDecoderSettings rfAudioSettings;
	int i;
	BSTD_UNUSED(hAv_input_select);

	BDBG_ENTER(B_Avis_StartInputAudioSourceAnalogTV);
	if(hRfAudioDecoder==NULL)
	{
		BDBG_LEAVE(B_Avis_StartInputAudioSourceAnalogTV);
		return BERR_INVALID_PARAMETER;
	}

	NEXUS_Platform_GetConfiguration(&platformConfig);

	/* Test for an analog TV frontend */
	for ( i = 0; i < NEXUS_MAX_FRONTENDS; i++ )
	{
		NEXUS_FrontendCapabilities capabilities;
		frontend = platformConfig.frontend[i];
		if (frontend) {
			NEXUS_Frontend_GetCapabilities(frontend, &capabilities);
			/* Does this frontend support analog? */
			if ( capabilities.analog && capabilities.ifd )
			{
				break;
			}
		}
	}
	if (NULL == frontend ) return BERR_NOT_SUPPORTED;

	if(audioDacIndex>=NEXUS_NUM_AUDIO_DACS)
	{
	   BDBG_LEAVE(B_Avis_StartInputAudioSourceAnalogTV);
	   return BERR_INVALID_PARAMETER;
	}

	if(spdifIndex>=NEXUS_NUM_SPDIF_OUTPUTS)
	{
	   BDBG_LEAVE(B_Avis_StartInputAudioSourceAnalogTV);
	   return BERR_INVALID_PARAMETER;
	}

	NEXUS_RfAudioDecoder_GetSettings(hRfAudioDecoder, &rfAudioSettings);
	rfAudioSettings.input = NEXUS_Frontend_GetAnalogAudioConnector(frontend);
	rfAudioSettings.mode = NEXUS_RfAudioDecoderMode_eUs;
	NEXUS_RfAudioDecoder_SetSettings(hRfAudioDecoder, &rfAudioSettings);

	NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[audioDacIndex]),
							   NEXUS_RfAudioDecoder_GetConnector(hRfAudioDecoder));
	NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[spdifIndex]),
							   NEXUS_RfAudioDecoder_GetConnector(hRfAudioDecoder));
	NEXUS_RfAudioDecoder_Start(hRfAudioDecoder);

	BDBG_LEAVE(B_Avis_StartInputAudioSourceAnalogTV);
	return BERR_SUCCESS;
}

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
									   unsigned int spdifIndex)
{
	B_Error err;
	NEXUS_PlatformConfiguration	platformConfig;
	NEXUS_AnalogAudioDecoderSettings decoderSettings;
	BSTD_UNUSED(hAv_input_select);

	BDBG_ENTER(B_Avis_StartInputAudioSourceComposite);

	if(hAnalogAudioDecoder==NULL)
	{
		BDBG_LEAVE(B_Avis_StartInputAudioSourceComposite);
		return BERR_INVALID_PARAMETER;
	}

	if(sourceIndex>=NEXUS_NUM_COMPOSITE_INPUTS)
	{
		BDBG_LEAVE(B_Avis_StartInputAudioSourceComposite);
		return BERR_INVALID_PARAMETER;
	}

	if(audioDacIndex>=NEXUS_NUM_AUDIO_DACS)
	{
	   BDBG_LEAVE(B_Avis_StartInputAudioSourceComposite);
	   return BERR_INVALID_PARAMETER;
	}

	if(spdifIndex>=NEXUS_NUM_SPDIF_OUTPUTS)
	{
	   BDBG_LEAVE(B_Avis_StartInputAudioSourceComposite);
	   return BERR_INVALID_PARAMETER;
	}

	NEXUS_Platform_GetConfiguration(&platformConfig);

	if (!platformConfig.inputs.composite[sourceIndex]) return BERR_INVALID_PARAMETER;

     NEXUS_AnalogAudioDecoder_GetSettings(hAnalogAudioDecoder, &decoderSettings);
    decoderSettings.input = 
		NEXUS_AnalogAudioInput_GetConnector(platformConfig.inputs.compositeAudio[sourceIndex]);
	NEXUS_AnalogAudioDecoder_SetSettings(hAnalogAudioDecoder, &decoderSettings);

    err = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[audioDacIndex]),
                                         NEXUS_AnalogAudioDecoder_GetConnector(hAnalogAudioDecoder));
    if ( err )
    {
        BDBG_ERR(("Unable to connect DAC to analog audio decoder"));
		BDBG_LEAVE(B_Avis_StartInputAudioSourceComposite);
		return err;
    }

	NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[spdifIndex]),
							   NEXUS_AnalogAudioDecoder_GetConnector(hAnalogAudioDecoder));
	if ( err )
	{
		BDBG_ERR(("Unable to connect SPDIF to analog audio decoder"));
		BDBG_LEAVE(B_Avis_StartInputAudioSourceComposite);
		return err;
	}

    err = NEXUS_AnalogAudioDecoder_Start(hAnalogAudioDecoder);
    if ( err )
    {
        BDBG_ERR(("Unable to start analogDecoder Capture"));
		BDBG_LEAVE(B_Avis_StartInputAudioSourceComposite);
        return err;
    }

	BDBG_LEAVE(B_Avis_StartInputAudioSourceComposite);
	return BERR_SUCCESS;
}

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
									unsigned int spdifIndex)
{
	B_Error err;
	NEXUS_PlatformConfiguration	platformConfig;
	NEXUS_AnalogAudioDecoderSettings decoderSettings;
	BSTD_UNUSED(hAv_input_select);
	BDBG_ENTER(B_Avis_StartInputAudioSourceSvideo);

	if(hAnalogAudioDecoder==NULL)
	{
		BDBG_LEAVE(B_Avis_StartInputAudioSourceSvideo);
		return BERR_INVALID_PARAMETER;
	}

	if(sourceIndex>=NEXUS_NUM_SVIDEO_INPUTS)
	{
		BDBG_LEAVE(B_Avis_StartInputAudioSourceSvideo);
		return BERR_INVALID_PARAMETER;
	}

	if(audioDacIndex>=NEXUS_NUM_AUDIO_DACS)
	{
	   BDBG_LEAVE(B_Avis_StartInputAudioSourceSvideo);
	   return BERR_INVALID_PARAMETER;
	}

	if(spdifIndex>=NEXUS_NUM_SPDIF_OUTPUTS)
	{
	   BDBG_LEAVE(B_Avis_StartInputAudioSourceSvideo);
	   return BERR_INVALID_PARAMETER;
	}

	NEXUS_Platform_GetConfiguration(&platformConfig);

	if (!platformConfig.inputs.svideo[sourceIndex]) return BERR_INVALID_PARAMETER;

	NEXUS_AnalogAudioDecoder_GetSettings(hAnalogAudioDecoder, &decoderSettings);
	decoderSettings.input = 
		NEXUS_AnalogAudioInput_GetConnector(platformConfig.inputs.svideoAudio[sourceIndex]);
	NEXUS_AnalogAudioDecoder_SetSettings(hAnalogAudioDecoder, &decoderSettings);

	err = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[audioDacIndex]),
										 NEXUS_AnalogAudioDecoder_GetConnector(hAnalogAudioDecoder));

	if ( err )
	{
		BDBG_ERR(("Unable to connect DAC to analog audio decoder"));
		BDBG_LEAVE(B_Avis_StartInputAudioSourceSvideo);
		return err;
	}

	NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[spdifIndex]),
							   NEXUS_AnalogAudioDecoder_GetConnector(hAnalogAudioDecoder));
	if ( err )
	{
		BDBG_ERR(("Unable to connect SPDIF to analog audio decoder"));
		BDBG_LEAVE(B_Avis_StartInputAudioSourceSvideo);
		return err;
	}

	err = NEXUS_AnalogAudioDecoder_Start(hAnalogAudioDecoder);
	if ( err )
	{
		BDBG_ERR(("Unable to start analogDecoder Capture"));
		BDBG_LEAVE(StartInputAudioSourceSvideo);
		return err;
	}

	BDBG_LEAVE(B_Avis_StartInputAudioSourceSvideo);
	return BERR_SUCCESS;
}

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
									   unsigned int spdifIndex)
{
	B_Error err;
	NEXUS_PlatformConfiguration	platformConfig;
	NEXUS_AnalogAudioDecoderSettings decoderSettings;
	BSTD_UNUSED(hAv_input_select);
	BDBG_ENTER(B_Avis_StartInputAudioSourceComponent);
	
	if(hAnalogAudioDecoder==NULL)
	{
	   BDBG_LEAVE(B_Avis_StartInputAudioSourceComponent);
	   return BERR_INVALID_PARAMETER;
	}
	
	if(sourceIndex>=NEXUS_NUM_COMPONENT_INPUTS)
	{
	   BDBG_LEAVE(B_Avis_StartInputAudioSourceComponent);
	   return BERR_INVALID_PARAMETER;
	}
	
	if(audioDacIndex>=NEXUS_NUM_AUDIO_DACS)
	{
	   BDBG_LEAVE(B_Avis_StartInputAudioSourceComponent);
	   return BERR_INVALID_PARAMETER;
	}

	if(spdifIndex>=NEXUS_NUM_SPDIF_OUTPUTS)
	{
	   BDBG_LEAVE(B_Avis_StartInputAudioSourceComponent);
	   return BERR_INVALID_PARAMETER;
	}

	NEXUS_Platform_GetConfiguration(&platformConfig);
	
	if (!platformConfig.inputs.component[sourceIndex]) return BERR_INVALID_PARAMETER;
	
	NEXUS_AnalogAudioDecoder_GetSettings(hAnalogAudioDecoder, &decoderSettings);
	decoderSettings.input = 
	   NEXUS_AnalogAudioInput_GetConnector(platformConfig.inputs.componentAudio[sourceIndex]);
	NEXUS_AnalogAudioDecoder_SetSettings(hAnalogAudioDecoder, &decoderSettings);
	
	err = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[audioDacIndex]),
										NEXUS_AnalogAudioDecoder_GetConnector(hAnalogAudioDecoder));
	
	if ( err )
	{
	   BDBG_ERR(("Unable to connect DAC to analog audio decoder"));
	   BDBG_LEAVE(B_Avis_StartInputAudioSourceComponent);
	   return err;
	}

	NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[spdifIndex]),
							   NEXUS_AnalogAudioDecoder_GetConnector(hAnalogAudioDecoder));
	if ( err )
	{
		BDBG_ERR(("Unable to connect SPDIF to analog audio decoder"));
		BDBG_LEAVE(B_Avis_StartInputAudioSourceComponent);
		return err;
	}
	
	err = NEXUS_AnalogAudioDecoder_Start(hAnalogAudioDecoder);
	if ( err )
	{
	   BDBG_ERR(("Unable to start analogDecoder Capture"));
	   BDBG_LEAVE(B_Avis_StartInputAudioSourceComponent);
	   return err;
	}
	
	BDBG_LEAVE(B_Avis_StartInputAudioSourceComponent);
	return BERR_SUCCESS;
}

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

Returns:
	BERR_SUCCESS - If the video was started.
	BERR_INVALID_PARAMETER - Returned if the input arguments are not valid.
	BERR_NOT_SUPPORTED - If the requested configuration is not supported on this platform.
***************************************************************************/
B_Error B_Avis_StartInputAudioSourceHDMI(B_Avis_Handle hAv_input_select,
								  NEXUS_HdmiInputHandle	hHdmiInput,
								  NEXUS_AudioDecoderHandle hAudioDecoder,
								  unsigned int audioDacIndex)
{
	NEXUS_PlatformConfiguration	platformConfig;
	NEXUS_AudioDecoderStartSettings audioProgram;
	NEXUS_StcChannelSettings stcSettings;
	BSTD_UNUSED(hAv_input_select);

	BDBG_ENTER(B_Avis_StartInputAudioSourceHDMI);

	if(hAudioDecoder==NULL || hHdmiInput==NULL)
	{
		BDBG_LEAVE(B_Avis_StartInputAudioSourceHDMI);
		return BERR_INVALID_PARAMETER;
	}

	if(audioDacIndex>=NEXUS_NUM_AUDIO_DACS)
	{
	   BDBG_LEAVE(B_Avis_StartInputAudioSourceHDMI);
	   return BERR_INVALID_PARAMETER;
	}

	NEXUS_Platform_GetConfiguration(&platformConfig);
	NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
	audioProgram.input = NEXUS_HdmiInput_GetAudioConnector(hHdmiInput);
	NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
	stcSettings.timebase = NEXUS_Timebase_e0;
	stcSettings.autoConfigTimebase = false;
	audioProgram.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
	NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[audioDacIndex]),
							   NEXUS_AudioDecoder_GetConnector(hAudioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
	NEXUS_AudioDecoder_Start(hAudioDecoder, &audioProgram);

	BDBG_LEAVE(B_Avis_StartInputAudioSourceHDMI);
	return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Stops the Digital TV audio output

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
									   unsigned int spdifIndex)
{
	BSTD_UNUSED(hAv_input_select);
	BSTD_UNUSED(audioDacIndex);
	BSTD_UNUSED(spdifIndex);
	BDBG_ENTER(B_Avis_StopInputAudioSourceDigitalTV);

	/* For now done in decode lib */

	BDBG_LEAVE(B_Avis_StopInputAudioSourceDigitalTV);
	return BERR_SUCCESS;
}

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
									 unsigned int spdifIndex)
{
	NEXUS_PlatformConfiguration	platformConfig;
	BSTD_UNUSED(hAv_input_select);

	BDBG_ENTER(B_Avis_StopInputAudioSourceAnalogTV);
	if(hRfAudioDecoder==NULL) return BERR_INVALID_PARAMETER;

	if(audioDacIndex>=NEXUS_NUM_AUDIO_DACS)
	{
	   BDBG_LEAVE(B_Avis_StopInputAudioSourceAnalogTV);
	   return BERR_INVALID_PARAMETER;
	}

	if(spdifIndex>=NEXUS_NUM_SPDIF_OUTPUTS)
	{
	   BDBG_LEAVE(B_Avis_StopInputAudioSourceAnalogTV);
	   return BERR_INVALID_PARAMETER;
	}

	NEXUS_Platform_GetConfiguration(&platformConfig);

	NEXUS_RfAudioDecoder_Stop(hRfAudioDecoder);

	NEXUS_AudioOutput_RemoveInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[audioDacIndex]),
							   NEXUS_RfAudioDecoder_GetConnector(hRfAudioDecoder));
	NEXUS_AudioOutput_RemoveInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[spdifIndex]),
							   NEXUS_RfAudioDecoder_GetConnector(hRfAudioDecoder));
	BDBG_LEAVE(B_Avis_StopInputAudioSourceAnalogTV);
	return BERR_SUCCESS;
}

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
									  unsigned int spdifIndex)
{
	NEXUS_PlatformConfiguration	platformConfig;
	BSTD_UNUSED(hAv_input_select);

	BDBG_ENTER(B_Avis_StopInputAudioSourceComposite);

	if(audioDacIndex>=NEXUS_NUM_AUDIO_DACS)
	{
	   BDBG_LEAVE(B_Avis_StopInputAudioSourceComposite);
	   return BERR_INVALID_PARAMETER;
	}

	if(spdifIndex>=NEXUS_NUM_SPDIF_OUTPUTS)
	{
	   BDBG_LEAVE(B_Avis_StopInputAudioSourceComposite);
	   return BERR_INVALID_PARAMETER;
	}

	NEXUS_Platform_GetConfiguration(&platformConfig);

	if(hAnalogAudioDecoder==NULL)
	{
	   BDBG_LEAVE(B_Avis_StopInputAudioSourceComposite);
	   return BERR_INVALID_PARAMETER;
	}

	NEXUS_AnalogAudioDecoder_Stop(hAnalogAudioDecoder);
	NEXUS_AudioOutput_RemoveInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[audioDacIndex]),
								  NEXUS_AnalogAudioDecoder_GetConnector(hAnalogAudioDecoder));

	NEXUS_AudioOutput_RemoveInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[spdifIndex]),
								  NEXUS_AnalogAudioDecoder_GetConnector(hAnalogAudioDecoder));
	BDBG_LEAVE(B_Avis_StopInputAudioSourceComposite);
	return BERR_SUCCESS;
}

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
								   unsigned int spdifIndex)
{
	NEXUS_PlatformConfiguration	platformConfig;
	BSTD_UNUSED(hAv_input_select);
	
	BDBG_ENTER(B_Avis_StopInputAudioSourceSvideo);
	
	if(audioDacIndex>=NEXUS_NUM_AUDIO_DACS)
	{
		BDBG_LEAVE(B_Avis_StopInputAudioSourceSvideo);
		return BERR_INVALID_PARAMETER;
	}
	
	if(spdifIndex>=NEXUS_NUM_SPDIF_OUTPUTS)
	{
		BDBG_LEAVE(B_Avis_StopInputAudioSourceSvideo);
		return BERR_INVALID_PARAMETER;
	}
	
	NEXUS_Platform_GetConfiguration(&platformConfig);
	
	if(hAnalogAudioDecoder==NULL)
	{
		BDBG_LEAVE(B_Avis_StopInputAudioSourceSvideo);
		return BERR_INVALID_PARAMETER;
	}
	
	NEXUS_AnalogAudioDecoder_Stop(hAnalogAudioDecoder);
	NEXUS_AudioOutput_RemoveInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[audioDacIndex]),
							   NEXUS_AnalogAudioDecoder_GetConnector(hAnalogAudioDecoder));
	NEXUS_AudioOutput_RemoveInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[spdifIndex]),
								  NEXUS_AnalogAudioDecoder_GetConnector(hAnalogAudioDecoder));
	
	BDBG_LEAVE(B_Avis_StopInputAudioSourceSvideo);
	return BERR_SUCCESS;
}

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
									  unsigned int spdifIndex)
{
   NEXUS_PlatformConfiguration	platformConfig;
   BSTD_UNUSED(hAv_input_select);

   BDBG_ENTER(B_Avis_StopInputAudioSourceComponent);

   if(audioDacIndex>=NEXUS_NUM_AUDIO_DACS)
   {
	   BDBG_LEAVE(B_Avis_StopInputAudioSourceComponent);
	   return BERR_INVALID_PARAMETER;
   }

   if(spdifIndex>=NEXUS_NUM_SPDIF_OUTPUTS)
   {
	   BDBG_LEAVE(B_Avis_StopInputAudioSourceComponent);
	   return BERR_INVALID_PARAMETER;
   }

   NEXUS_Platform_GetConfiguration(&platformConfig);

   if(hAnalogAudioDecoder==NULL)
   {
	   BDBG_LEAVE(B_Avis_StopInputAudioSourceComponent);
	   return BERR_INVALID_PARAMETER;
   }

   NEXUS_AnalogAudioDecoder_Stop(hAnalogAudioDecoder);
   NEXUS_AudioOutput_RemoveInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[audioDacIndex]),
							  NEXUS_AnalogAudioDecoder_GetConnector(hAnalogAudioDecoder));
   NEXUS_AudioOutput_RemoveInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[spdifIndex]),
								 NEXUS_AnalogAudioDecoder_GetConnector(hAnalogAudioDecoder));

   BDBG_LEAVE(B_Avis_StopInputAudioSourceComponent);
   return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Stops the HDMI audio ouput

Description:
Stops the HDMI audio output

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
								 unsigned int audioDacIndex)
{
	NEXUS_PlatformConfiguration	platformConfig;
	BSTD_UNUSED(hAv_input_select);

	BDBG_ENTER(B_Avis_StopInputAudioSourceHDMI);
	if(hAudioDecoder==NULL)
	{
		BDBG_LEAVE(B_Avis_StopInputAudioSourceHDMI);
		return BERR_INVALID_PARAMETER;
	}

	if(audioDacIndex>=NEXUS_NUM_AUDIO_DACS)
	{
		BDBG_LEAVE(B_Avis_StopInputAudioSourceHDMI);
		return BERR_INVALID_PARAMETER;
	}
	NEXUS_Platform_GetConfiguration(&platformConfig);
	NEXUS_AudioDecoder_Stop(hAudioDecoder);
	NEXUS_AudioOutput_RemoveInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[audioDacIndex]),
							   NEXUS_AudioDecoder_GetConnector(hAudioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

	BDBG_LEAVE(B_Avis_StopInputAudioSourceHDMI);
	return BERR_SUCCESS;
}


