/******************************************************************************
 *    (c)2008-2013 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
******************************************************************************/
#include "subtitle_priv.h"

#include "nexus_video_window.h"
#include "nexus_display_vbi.h"
#include "nexus_parser_band.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern Subtitle_Handles gHandles;

static NEXUS_PlatformConfiguration g_platformConfig;

static bool g_initialized = false;
static bool g_usePassthroughAudioDecoder = false;

unsigned cur_audio_track;

BDBG_MODULE(subtitle_priv);


void message_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

NEXUS_TransportType GetTransportType(bstream_mpeg_type mpeg_type)
{
	NEXUS_TransportType transportType;
	
	switch(mpeg_type){	
	case bstream_mpeg_type_es:
		transportType = NEXUS_TransportType_eEs;
		break;
	case bstream_mpeg_type_pes:
		transportType = NEXUS_TransportType_eMpeg2Pes;
		break;
	case bstream_mpeg_type_ts:
		transportType = NEXUS_TransportType_eTs;
		break;
	case bstream_mpeg_type_dss_es:
		transportType = NEXUS_TransportType_eDssEs;
		break;
	case bstream_mpeg_type_dss_pes:
		transportType = NEXUS_TransportType_eDssPes;
		break;
	case bstream_mpeg_type_vob:
		transportType = NEXUS_TransportType_eVob;
		break;
	case bstream_mpeg_type_asf:
		transportType = NEXUS_TransportType_eAsf;
		break;		
	case bstream_mpeg_type_avi:
		transportType = NEXUS_TransportType_eAvi;
		break;
	case bstream_mpeg_type_mpeg1:
		transportType = NEXUS_TransportType_eMpeg1Ps;
		break;
	case bstream_mpeg_type_mp4:
		transportType = NEXUS_TransportType_eMp4;
		break;
	case bstream_mpeg_type_mkv:
		transportType = NEXUS_TransportType_eMkv;
		break;
	case bstream_mpeg_type_wav:
		transportType = NEXUS_TransportType_eWav;
		break;
	case bstream_mpeg_type_flv:
		transportType = NEXUS_TransportType_eFlv;
		break;
	case bstream_mpeg_type_unknown:
	default:       
		transportType = NEXUS_TransportType_eTs;
		break;
	}

	return	transportType;
}

NEXUS_AudioCodec GetAudioCodec(baudio_format settopCodec)
{
    NEXUS_AudioCodec audioCodec;

    switch(settopCodec){
    case baudio_format_mpeg:
        audioCodec = NEXUS_AudioCodec_eMpeg;
        break;
    case baudio_format_mp3:
        audioCodec = NEXUS_AudioCodec_eMp3;
        break;
    case baudio_format_aac:      
        audioCodec = NEXUS_AudioCodec_eAac;
        break;
    case baudio_format_aac_plus:      
        audioCodec = NEXUS_AudioCodec_eAacPlus;
        break;
    case baudio_format_aac_plus_adts:      
        audioCodec = NEXUS_AudioCodec_eAacPlusAdts;
        break;
    case baudio_format_ac3:
        audioCodec = NEXUS_AudioCodec_eAc3;
        break;
    case baudio_format_ac3_plus:
        audioCodec = NEXUS_AudioCodec_eAc3Plus;
        break;
    case baudio_format_dts:
        audioCodec = NEXUS_AudioCodec_eDts;
        break;
    case baudio_format_lpcm_hddvd:
        audioCodec = NEXUS_AudioCodec_eLpcmHdDvd;
        break;
    case baudio_format_lpcm_bluray:
        audioCodec = NEXUS_AudioCodec_eLpcmBluRay;
        break;
    case baudio_format_dts_hd:
        audioCodec = NEXUS_AudioCodec_eDtsHd;
        break;
    case baudio_format_wma_std:
        audioCodec = NEXUS_AudioCodec_eWmaStd;
        break;
    case baudio_format_wma_pro:
        audioCodec = NEXUS_AudioCodec_eWmaPro;
        break;
    case baudio_format_lpcm_dvd:
        audioCodec = NEXUS_AudioCodec_eLpcmDvd;
        break;
    case baudio_format_avs:
        audioCodec = NEXUS_AudioCodec_eAvs;
        break;    
    case baudio_format_unknown:
    default:
        audioCodec = NEXUS_AudioCodec_eUnknown;
        break;
    };

    
    return audioCodec;

}

NEXUS_VideoCodec GetVideoCodec(bvideo_codec settopCodec)
{
    NEXUS_VideoCodec videoCodec;

    /* Converts Settop enum to Nexus-based enum */
    switch(settopCodec){
    case bvideo_codec_mpeg1:
        videoCodec = NEXUS_VideoCodec_eMpeg1;
        break;
    case bvideo_codec_mpeg2:
        videoCodec = NEXUS_VideoCodec_eMpeg2;
        break;
    case bvideo_codec_mpeg4_part2:
        videoCodec = NEXUS_VideoCodec_eMpeg4Part2;
        break;
    case bvideo_codec_h263:
        videoCodec = NEXUS_VideoCodec_eH263;
        break;
    case bvideo_codec_h264:
        videoCodec = NEXUS_VideoCodec_eH264;
        break;
    case bvideo_codec_vc1:
        videoCodec = NEXUS_VideoCodec_eVc1;
        break;
    case bvideo_codec_vc1_sm:
        videoCodec = NEXUS_VideoCodec_eVc1SimpleMain;
        break;
    case bvideo_codec_divx_311:
        videoCodec = NEXUS_VideoCodec_eDivx311;
        break;
    case bvideo_codec_avs:
        videoCodec = NEXUS_VideoCodec_eAvs;
        break;
    case bvideo_codec_unknown:
    default:
        videoCodec = NEXUS_VideoCodec_eUnknown;
        break;
    };

    return videoCodec;
}

void Decode_GetDefaultSettings(Subtitle_DecodeSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));  
}

/* Init display and audio */
NEXUS_Error InitDisplay(void)
{
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplaySettings displayCfg;
    NEXUS_Error rc;
    NEXUS_VideoWindowSettings windowSettings;


    BDBG_ASSERT(false == g_initialized);

    NEXUS_Platform_GetConfiguration(&g_platformConfig);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.modeSettings.Auto.behavior = NEXUS_StcChannelAutoModeBehavior_eVideoMaster;
    gHandles.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
    BDBG_ASSERT(NULL != gHandles.stcChannel);


    /* bring up display */
    NEXUS_Display_GetDefaultSettings(&displayCfg);
    
    displayCfg.format = NEXUS_VideoFormat_e1080i;
    displayCfg.aspectRatio =  NEXUS_DisplayAspectRatio_e16x9;
    gHandles.displayHD = NEXUS_Display_Open(0, &displayCfg);
    BDBG_ASSERT(gHandles.displayHD);    

#if NEXUS_NUM_DISPLAYS > 1 && NEXUS_NUM_COMPOSITE_OUTPUTS
    displayCfg.format  = NEXUS_VideoFormat_eNtsc;
    displayCfg.aspectRatio = NEXUS_DisplayAspectRatio_e4x3;
    gHandles.displaySD = NEXUS_Display_Open(1, &displayCfg);
    BDBG_ASSERT(gHandles.displaySD);
#endif

#if NEXUS_NUM_COMPONENT_OUTPUTS
    rc = NEXUS_Display_AddOutput(gHandles.displayHD, NEXUS_ComponentOutput_GetConnector(g_platformConfig.outputs.component[0]));
    rc = BERR_TRACE(rc);
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS && NEXUS_NUM_DISPLAYS > 1    
    rc = NEXUS_Display_AddOutput(gHandles.displaySD, NEXUS_CompositeOutput_GetConnector(g_platformConfig.outputs.composite[0]));
    rc = BERR_TRACE(rc);
#endif
        
    gHandles.windowHD = NEXUS_VideoWindow_Open(gHandles.displayHD, 0);
    BDBG_ASSERT(gHandles.windowHD);
    NEXUS_VideoWindow_GetSettings(gHandles.windowHD, &windowSettings);
    windowSettings.letterBoxDetect = true;
    windowSettings.contentMode = NEXUS_VideoWindowContentMode_eBox;
    rc = NEXUS_VideoWindow_SetSettings(gHandles.windowHD, &windowSettings);
    BDBG_ASSERT(!rc);

#if NEXUS_NUM_DISPLAYS > 1 && NEXUS_NUM_COMPOSITE_OUTPUTS 
    gHandles.windowSD = NEXUS_VideoWindow_Open(gHandles.displaySD, 0);
    BDBG_ASSERT(gHandles.windowSD);
    NEXUS_VideoWindow_GetSettings(gHandles.windowSD, &windowSettings);
    windowSettings.letterBoxDetect = true;
    windowSettings.contentMode = NEXUS_VideoWindowContentMode_eBox;
    rc = NEXUS_VideoWindow_SetSettings(gHandles.windowSD, &windowSettings);
    BDBG_ASSERT(!rc);
#endif
    
    gHandles.outputProtectionData.enabled = false;

    g_initialized = true;

    return 0;
}

/* This function is only set if CGMSA Bit is detected */
static void cgmsa_line21(void * data)
{
    NEXUS_DisplayHandle displayHandle = data;

    while (1) {
        size_t num;
	
	/* gHandles.outputProtectionData.ccData.field = 0; */
        /* NEXUS_Display_WriteClosedCaption(displayHandle, &gHandles.outputProtectionData.ccData, 1, &num); */
        gHandles.outputProtectionData.ccData.field = 1;
        NEXUS_Display_WriteClosedCaption(displayHandle, &gHandles.outputProtectionData.ccData, 1, &num);
	                           
	/* Give it time to rest */
	B_Thread_Sleep(600);
    }

}

void ictDisplay (void * cnxt, uint8_t ictBit )
{
    BSTD_UNUSED(cnxt);

    BDBG_MSG(("Ict Bit %d", ictBit));
    gHandles.outputProtectionData.ictBit = !ictBit;  /* Invert the logic. Because default is HD ON */

    return; 
}

void outputProtection (void * cnxt, uint32_t macBit, uint32_t cgmsaBit )
{
    BDBG_MSG(("Mac Bit %d, Cgmsa Bit %d", macBit,cgmsaBit));
    
    BSTD_UNUSED(cnxt);
	
    gHandles.outputProtectionData.enabled = true;
    gHandles.outputProtectionData.cgmsaBit = cgmsaBit;
    gHandles.outputProtectionData.cgmsb_data[0] = 0x01100800;		
		
    /* Set the Line 21 Data */	
    gHandles.outputProtectionData.ccData.data[1] = 0x40;
	
    switch(cgmsaBit) {    
    case 1:
	gHandles.outputProtectionData.cgmsb_data[1] = (0x70|2)<<24;						
	gHandles.outputProtectionData.cgmsa_data =0x2<<6;
	gHandles.outputProtectionData.ccData.data[0] = (0x80 | 0x8);
	gHandles.outputProtectionData.wss_data =0x2<<12;
	break;
    case 2: 
	gHandles.outputProtectionData.cgmsb_data[1] = (0x70|1)<<24;			
	gHandles.outputProtectionData.cgmsa_data =0x1<<6;
	gHandles.outputProtectionData.ccData.data[0] = (0x80 | 0x10);			
	gHandles.outputProtectionData.wss_data = 0x1<<12;		
	break;
    case 3:
	gHandles.outputProtectionData.cgmsb_data[1] = (0x70|3)<<24;
	gHandles.outputProtectionData.cgmsa_data =0x3<<6;
	gHandles.outputProtectionData.ccData.data[0] = (0x80 | 0x18);
	gHandles.outputProtectionData.wss_data =0x3<<12;
	break;
    default:
	gHandles.outputProtectionData.cgmsb_data[1] = (0x70)<<24;
	gHandles.outputProtectionData.cgmsa_data =0;
	gHandles.outputProtectionData.ccData.data[0] = 0x80;
	gHandles.outputProtectionData.wss_data =0;
	break;
    }

	
    gHandles.outputProtectionData.macrovision_data = macBit;				

    return; 
}

void Display_SetOutput(NEXUS_DisplaySettings *displayCfg, int display, int pal )
{
    NEXUS_Error rc;   
    NEXUS_DisplayHandle displayHandle;	
    NEXUS_DisplayVbiSettings displayVbiSettings;		

    if (display) {
	displayHandle=gHandles.displaySD;
    } else {
	displayHandle=gHandles.displayHD;
    }	
	
    if(gHandles.outputProtectionData.enabled) {
	if (gHandles.outputProtectionData.ccHandle[display]) {
	    B_Thread_Destroy(gHandles.outputProtectionData.ccHandle[display]);
	    gHandles.outputProtectionData.ccHandle[display] = NULL;
	}			       
		
	NEXUS_Display_GetVbiSettings(displayHandle, &displayVbiSettings);
	displayVbiSettings.teletextEnabled = false;
	displayVbiSettings.wssEnabled = false;
	displayVbiSettings.vpsEnabled = false;
	displayVbiSettings.cgmsEnabled = false;
	displayVbiSettings.closedCaptionEnabled = false;
	displayVbiSettings.macrovisionEnabled = false;
	rc = NEXUS_Display_SetVbiSettings(displayHandle, &displayVbiSettings);
	BDBG_ASSERT(!rc);
    }				

    if((display || gHandles.outputProtectionData.ictBit || gHandles.outputProtectionData.macrovision_data) && (displayCfg->format != NEXUS_VideoFormat_eNtsc && displayCfg->format != NEXUS_VideoFormat_ePal)){			
	displayCfg->format = pal?NEXUS_VideoFormat_ePal:NEXUS_VideoFormat_eNtsc;
	displayCfg->aspectRatio =  NEXUS_DisplayAspectRatio_e4x3;
	BDBG_WRN(("HD Format not allowed in given configuration. Resetting display %d to SD", display));
    }
    NEXUS_Display_SetSettings(displayHandle, displayCfg);

    if(gHandles.outputProtectionData.enabled) {
	NEXUS_Display_GetVbiSettings(displayHandle, &displayVbiSettings);

	displayVbiSettings.wssEnabled = pal?true:false;
	displayVbiSettings.closedCaptionEnabled = pal?false:true;	
	displayVbiSettings.cgmsEnabled = pal?false:true;
	displayVbiSettings.macrovisionEnabled = (displayCfg->format == NEXUS_VideoFormat_eNtsc || 
						 displayCfg->format == NEXUS_VideoFormat_ePal  ||
						 displayCfg->format == NEXUS_VideoFormat_e480p ||
						 displayCfg->format == NEXUS_VideoFormat_e576p ) ? true:false;

	rc = NEXUS_Display_SetVbiSettings(displayHandle, &displayVbiSettings);
	BDBG_ASSERT(!rc);
		
	BKNI_Sleep(100);

	if(displayVbiSettings.wssEnabled) {
	    NEXUS_Display_SetWss(displayHandle, gHandles.outputProtectionData.wss_data);
	}
	if(displayVbiSettings.cgmsEnabled) {	
	    NEXUS_Display_SetCgms(displayHandle, gHandles.outputProtectionData.cgmsa_data); /* CGMSA A is enabled always */
	    if(displayCfg->format != NEXUS_VideoFormat_eNtsc) {	
	    NEXUS_Display_SetCgmsB(displayHandle, gHandles.outputProtectionData.cgmsb_data,5); /* CGMS B is enabled if display format is not Ntsc */
	    }			
	}

	BKNI_Sleep(100);
		
	if(displayVbiSettings.macrovisionEnabled){
	    NEXUS_Display_SetMacrovision(displayHandle, (NEXUS_DisplayMacrovisionType)gHandles.outputProtectionData.macrovision_data , NULL);
	}

	
	/* Launch Closed Caption Thread */
	/* Create Thread for Line 21 if CGMSA is used  */
	/* Only do it for HD dislay */			
	if (displayCfg->format == NEXUS_VideoFormat_eNtsc) {	    
	    gHandles.outputProtectionData.ccHandle[display] = B_Thread_Create("cgmsa_task", cgmsa_line21, displayHandle, NULL);
	}    
    }
 			

    {
	NEXUS_VideoFormatInfo info;
	NEXUS_DisplaySettings displaySettings;    
	NEXUS_GraphicsSettings graphicsSettings;

	if(display) {
	    NEXUS_Display_GetSettings(gHandles.displaySD, &displaySettings);
	    NEXUS_VideoFormat_GetInfo(displaySettings.format, &info);
	    gHandles.displaySDWidth = info.width;
	    gHandles.displaySDHeight = info.height;
		
	    if(gHandles.surfaceSD) {
		NEXUS_Display_GetGraphicsSettings(gHandles.displaySD, &graphicsSettings);			
		graphicsSettings.clip.width = gHandles.displaySDWidth;
		graphicsSettings.clip.height = gHandles.displaySDHeight;
		graphicsSettings.enabled = true;
		graphicsSettings.chromakeyEnabled = false;    
		graphicsSettings.alpha = 0xff;
		NEXUS_Display_SetGraphicsSettings(gHandles.displaySD, &graphicsSettings);
		NEXUS_Display_SetGraphicsFramebuffer(gHandles.displaySD, gHandles.surfaceSD);
	    }
	} else {
	    NEXUS_Display_GetSettings(gHandles.displayHD, &displaySettings);
	    NEXUS_VideoFormat_GetInfo(displaySettings.format, &info);
	    gHandles.displayHDWidth = info.width;
	    gHandles.displayHDHeight = info.height;
		
	    if(gHandles.surfaceHD) {
		NEXUS_Display_GetGraphicsSettings(gHandles.displayHD, &graphicsSettings);			
		graphicsSettings.clip.width = gHandles.displayHDWidth;
		graphicsSettings.clip.height = gHandles.displayHDHeight;
		graphicsSettings.enabled = true;
		graphicsSettings.chromakeyEnabled = false;    
		graphicsSettings.alpha = 0xff;
		NEXUS_Display_SetGraphicsSettings(gHandles.displayHD, &graphicsSettings);
		NEXUS_Display_SetGraphicsFramebuffer(gHandles.displayHD, gHandles.surfaceHD);
	    }
	}
    }

    return; 
}

void SetAudioProgram(Subtitle_DecodeSettings *pSettings, int  totalAudioTracks)
{
    int trackNo=0;
    NEXUS_PlaybackPidChannelSettings pidSettings;

    for (trackNo=0; trackNo < totalAudioTracks; trackNo++)
    {
	NEXUS_AudioDecoder_GetDefaultStartSettings(&gHandles.audioProgram[trackNo]);		
	if ( pSettings->audioPid[trackNo] )
	{			  
	    if (pSettings->playback) {       
		NEXUS_Playback_GetDefaultPidChannelSettings(&pidSettings);
		pidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
		pidSettings.pidSettings.pidTypeSettings.audio.codec = pSettings->audioCodec[trackNo];
		pidSettings.pidTypeSettings.audio.primary = gHandles.audioDecoder;
		if ( g_usePassthroughAudioDecoder )
		{
		    pidSettings.pidTypeSettings.audio.secondary = gHandles.audioPassthrough;
		} else {
		    pidSettings.pidTypeSettings.audio.secondary = NULL;
		}
		gHandles.audioProgram[trackNo].pidChannel = NEXUS_Playback_OpenPidChannel(pSettings->playback, pSettings->audioPid[trackNo], &pidSettings);
		BDBG_MSG(("playback NEXUS_Playback_OpenPidChannel %d",gHandles.audioProgram[trackNo].pidChannel));
		gHandles.audioProgram[trackNo].codec = pSettings->audioCodec[trackNo];				  
	    }
			  		
	    BDBG_ASSERT(gHandles.audioProgram[trackNo].pidChannel);
	}
    }
}

NEXUS_Error InitAudioDecode(Subtitle_DecodeSettings *pSettings, int  totalAudioTracks)
{
    NEXUS_Error rc;		             
    NEXUS_AudioDecoderOpenSettings openSettings;

    NEXUS_AudioDecoder_GetDefaultOpenSettings(&openSettings);
    /* Increase audio CDB size. This is required when using playback accurate seek.
       With files with large gop size accurate seek may cause audio to overflow */ 
    openSettings.fifoSize = 1024*1024; 
    gHandles.audioDecoder = NEXUS_AudioDecoder_Open(0, &openSettings);
    BDBG_ASSERT(NULL != gHandles.audioDecoder);

    rc = NEXUS_AudioOutput_AddInput(
	NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]),
	NEXUS_AudioDecoder_GetConnector(gHandles.audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

    BDBG_ASSERT(!rc);

    if ( g_usePassthroughAudioDecoder )
    {
	gHandles.audioPassthrough = NEXUS_AudioDecoder_Open(1, NULL);
	BDBG_ASSERT(NULL != gHandles.audioPassthrough);
	rc = NEXUS_AudioOutput_AddInput(
	    NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
	    NEXUS_AudioDecoder_GetConnector(gHandles.audioPassthrough, NEXUS_AudioDecoderConnectorType_eCompressed));

	BDBG_ASSERT(!rc);
    } else {
	rc = NEXUS_AudioOutput_AddInput(
	    NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
	    NEXUS_AudioDecoder_GetConnector(gHandles.audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
	
	BDBG_ASSERT(!rc);
    }
	
    SetAudioProgram(pSettings, totalAudioTracks);
    cur_audio_track = 0;
		
    return rc;
}

NEXUS_Error StartAudioDecode(Subtitle_DecodeSettings *pSettings, unsigned track, bool tsm)
{
    NEXUS_Error rc;

    BDBG_MSG((">>StartAudioDecode track %d", track));

    if (!gHandles.audioProgram[track].pidChannel) 
    {
        BDBG_ERR((" Cannot play Audio Pid = 0x%0x", gHandles.audioProgram[track].pidChannel));
        return -1;

    }  

    if (!gHandles.audioProgram[track].codec) 
     {
        BDBG_ERR((" Cannot play Audio Codec Unknown!"));
        return -1;
     }     

    gHandles.audioProgram[track].codec = pSettings->audioCodec[track];
    if(tsm){
	    gHandles.audioProgram[track].stcChannel = gHandles.stcChannel;
    } else {
	    gHandles.audioProgram[track].stcChannel = NULL;
    }

    gHandles.audioPid = gHandles.audioProgram[track].pidChannel;

     
    BDBG_MSG(("Audio Codec =0x%0x , Audio Track No=%d",gHandles.audioProgram[track].codec, track));
    rc = NEXUS_AudioDecoder_Start(gHandles.audioDecoder, &gHandles.audioProgram[track]);
    BDBG_ASSERT(!rc);

    if ( g_usePassthroughAudioDecoder )
    {
        /* Only pass through AC3 */
        rc = NEXUS_AudioDecoder_Start(gHandles.audioPassthrough, &gHandles.audioProgram[track]);
        BDBG_ASSERT(!rc);
    }
    
    cur_audio_track = track;

    return rc;
}

void StopAudioDecode(void)
{  
    NEXUS_AudioDecoder_Stop(gHandles.audioDecoder);
 
    if ( g_usePassthroughAudioDecoder )
    { 
        NEXUS_AudioDecoder_Stop(gHandles.audioPassthrough);
        BDBG_MSG(("NEXUS_AudioDecoder_Stop audioPassthrough"));
    }    
}

void SetVideoProgram(Subtitle_DecodeSettings *pSettings, int  totalVideoTracks)
{
    int trackNo=0;
    NEXUS_PlaybackPidChannelSettings pidSettings;

    for (trackNo=0; trackNo < totalVideoTracks; trackNo++)
    {
	NEXUS_VideoDecoder_GetDefaultStartSettings(&gHandles.videoProgram[trackNo]);    
	if ( pSettings->videoPid[trackNo] )
	{
	    if (pSettings->playback) {
		NEXUS_Playback_GetDefaultPidChannelSettings(&pidSettings);
		pidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
		pidSettings.pidTypeSettings.video.codec = pSettings->videoCodec[trackNo];
		pidSettings.pidTypeSettings.video.decoder = gHandles.videoDecoder;
		pidSettings.pidTypeSettings.video.index = true;
		gHandles.videoProgram[trackNo].pidChannel = NEXUS_Playback_OpenPidChannel(pSettings->playback, pSettings->videoPid[trackNo], &pidSettings);
		BDBG_MSG(("playback NEXUS_Playback_OpenPidChannel %d",gHandles.videoProgram[trackNo].pidChannel));
		gHandles.videoProgram[trackNo].codec = pSettings->videoCodec[trackNo];
		gHandles.videoProgram[trackNo].aspectRatio = gHandles.aspect_ratio;
	    }    
			
	    BDBG_ASSERT(gHandles.videoProgram[trackNo].pidChannel);
	}
    }	
}

NEXUS_Error InitVideoDecode(Subtitle_DecodeSettings *pSettings, int  totalVideoTracks)
{
    NEXUS_Error rc;	
    NEXUS_VideoInput videoInput;
    NEXUS_VideoDecoderOpenSettings openSettings;
    NEXUS_VideoDecoderSettings videoDecoderSettings;

    /* bring up decoder and connect to display */
    NEXUS_VideoDecoder_GetDefaultOpenSettings(&openSettings);
    openSettings.fifoSize = 3*1024*1024;
    gHandles.videoDecoder = NEXUS_VideoDecoder_Open(0, &openSettings); /* take default capabilities */
    BDBG_ASSERT(NULL != gHandles.videoDecoder);

    NEXUS_VideoDecoder_GetSettings(gHandles.videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
    rc=NEXUS_VideoDecoder_SetSettings(gHandles.videoDecoder, &videoDecoderSettings);
    BDBG_ASSERT(!rc);

    videoInput = NEXUS_VideoDecoder_GetConnector(gHandles.videoDecoder);

    rc = NEXUS_VideoWindow_AddInput(gHandles.windowHD, videoInput);
    BDBG_ASSERT(!rc);
#if NEXUS_NUM_DISPLAYS > 1 && NEXUS_NUM_COMPOSITE_OUTPUTS 
    rc = NEXUS_VideoWindow_AddInput(gHandles.windowSD, videoInput);
    BDBG_ASSERT(!rc);
#endif
	
    SetVideoProgram(pSettings, totalVideoTracks);

    return rc;
}


NEXUS_Error StartVideoDecode(Subtitle_DecodeSettings *pSettings, unsigned track, bool tsm)
{    
    NEXUS_Error rc;
    
    if (!gHandles.videoProgram[track].pidChannel) 
    {
        BDBG_ERR((" Cannot play Video Pid = 0x%0x", gHandles.videoProgram[track].pidChannel));
        return -1;

    }  

    if (!gHandles.videoProgram[track].codec) 
     {
        BDBG_ERR((" Cannot play Video Codec Unknown!"));
        return -1;
     }
            

    gHandles.videoProgram[track].codec = pSettings->videoCodec[track];
    if(tsm){
	    gHandles.videoProgram[track].stcChannel = gHandles.stcChannel;
    } else {
	    gHandles.videoProgram[track].stcChannel = NULL;
    }

    gHandles.videoPid = gHandles.videoProgram[track].pidChannel;    
    
    BDBG_MSG(("Video Codec =0x%0x , Video Track No=%d",gHandles.videoProgram[track].codec, track));

    rc = NEXUS_VideoDecoder_Start(gHandles.videoDecoder, &gHandles.videoProgram[track]);
    BDBG_ASSERT(!rc);   

    return rc;
}

void StopVideoDecode(void)
{
	NEXUS_VideoDecoder_Stop(gHandles.videoDecoder);	
}


NEXUS_Error InitSubtitleDecode(Subtitle_DecodeSettings *pSettings, int  totalSubtitleTracks, BKNI_EventHandle event)
{
	NEXUS_Error rc=NEXUS_SUCCESS;
	int trackNo=0;
	NEXUS_MessageSettings settings;
	NEXUS_PlaybackPidChannelSettings pidSettings;	

	NEXUS_Message_GetDefaultSettings(&settings); 
	settings.dataReady.callback = message_callback;
	settings.dataReady.context = event;
	settings.bufferSize = 64*1024;

	NEXUS_Playback_GetDefaultPidChannelSettings(&pidSettings);
	pidSettings.pidSettings.pidType = NEXUS_PidType_eOther;	

	for (trackNo=0; trackNo < totalSubtitleTracks; trackNo++)
	{		
		gHandles.subtitle[trackNo] = NEXUS_Message_Open(&settings);

		NEXUS_Message_GetDefaultStartSettings(gHandles.subtitle[trackNo], &gHandles.subtitleProgram[trackNo]);
		if ( pSettings->subtitlePid[trackNo] )
		{
			if (pSettings->playback) {
				gHandles.subtitleProgram[trackNo].pidChannel = NEXUS_Playback_OpenPidChannel(pSettings->playback, pSettings->subtitlePid[trackNo], &pidSettings);
			}

			BDBG_ASSERT(gHandles.subtitleProgram[trackNo].pidChannel);
		}
	}

	gHandles.totalSubtitleTracks = totalSubtitleTracks;
	
	return rc;
}


NEXUS_Error StartSubtitleDecode(Subtitle_DecodeSettings *pSettings)
{
	NEXUS_Error rc;
	unsigned trackNo=0;
		
	for (trackNo=0; trackNo < gHandles.totalSubtitleTracks; trackNo++)
	{		
		gHandles.subtitleProgram[trackNo].format = NEXUS_MessageFormat_ePes;
		
		if ( pSettings->subtitlePid[trackNo] ) {
			rc = NEXUS_Message_Start(gHandles.subtitle[trackNo], &gHandles.subtitleProgram[trackNo]);   
			BDBG_ASSERT(!rc);
		}
	}

	return rc;
}

void StopSubtitleDecode(void)
{
	unsigned trackNo=0;
	
	for (trackNo=0; trackNo < gHandles.totalSubtitleTracks; trackNo++)
	{
		if(gHandles.subtitle[trackNo]){
			NEXUS_Message_Stop(gHandles.subtitle[trackNo]);			
		}
	}
}

void StopAllDecode(void)
{
	unsigned trackNo=0;

	StopVideoDecode();
	StopAudioDecode();
	StopSubtitleDecode();
	
	if ( gHandles.playback )
	{
		NEXUS_Playback_CloseAllPidChannels(gHandles.playback);		
	}
		
#if NEXUS_HAS_AUDIO
	NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]));
	NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]));
	NEXUS_AudioOutput_Shutdown(NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]));
	NEXUS_AudioOutput_Shutdown(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]));
	NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(gHandles.audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
	NEXUS_AudioDecoder_Close(gHandles.audioDecoder);
	gHandles.audioDecoder = NULL;
	if ( g_usePassthroughAudioDecoder ) {
		NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(gHandles.audioPassthrough, NEXUS_AudioDecoderConnectorType_eCompressed));
		NEXUS_AudioDecoder_Close(gHandles.audioPassthrough);
		gHandles.audioPassthrough = NULL;
	}
	BKNI_Memset(gHandles.audioProgram, 0, sizeof(gHandles.audioProgram));
#endif
	NEXUS_VideoWindow_RemoveAllInputs(gHandles.windowHD);
#if NEXUS_NUM_DISPLAYS > 1 && NEXUS_NUM_COMPOSITE_OUTPUTS 
	NEXUS_VideoWindow_RemoveAllInputs(gHandles.windowSD);
#endif
	NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(gHandles.videoDecoder));
	NEXUS_VideoDecoder_Close(gHandles.videoDecoder);
	gHandles.videoDecoder = NULL;
	BKNI_Memset(gHandles.videoProgram, 0, sizeof(gHandles.videoProgram));

	for (trackNo=0; trackNo < gHandles.totalSubtitleTracks; trackNo++)
	{	
		if(gHandles.subtitle[trackNo]){
			NEXUS_Message_Close(gHandles.subtitle[trackNo]);
			gHandles.subtitle[trackNo] = NULL;	
		}	
	}
	BKNI_Memset(gHandles.subtitleProgram, 0, sizeof(gHandles.subtitleProgram));
}

void UninitDisplay(void)
{    
    if (gHandles.outputProtectionData.ccHandle[0])         
    {        
        B_Thread_Destroy(gHandles.outputProtectionData.ccHandle[0]);
	gHandles.outputProtectionData.ccHandle[0] = NULL;
    }
    if (gHandles.outputProtectionData.ccHandle[1])         
    {        
        B_Thread_Destroy(gHandles.outputProtectionData.ccHandle[1]);
	gHandles.outputProtectionData.ccHandle[1] = NULL;
    }

    NEXUS_VideoWindow_Close(gHandles.windowHD);
    NEXUS_Display_Close(gHandles.displayHD);

#if NEXUS_NUM_DISPLAYS > 1 && NEXUS_NUM_COMPOSITE_OUTPUTS 
    NEXUS_VideoWindow_Close(gHandles.windowSD);
    NEXUS_Display_Close(gHandles.displaySD);
#endif
        
    g_initialized = false;
}

void PrintDecodeStatus(void)
{
    NEXUS_VideoDecoderStatus status;
#if NEXUS_HAS_AUDIO
    NEXUS_AudioDecoderStatus audioStatus;
#endif
    uint32_t stc;

    NEXUS_VideoDecoder_GetStatus(gHandles.videoDecoder, &status);
    NEXUS_StcChannel_GetStc(gHandles.stcChannel, &stc);
    printf("decode %.4dx%.4d, pts %#x, stc %#x (diff %d) fifo=%d%%\n",
        status.source.width, status.source.height, status.pts, stc, status.pts - stc, status.fifoSize?(status.fifoDepth*100)/status.fifoSize:0);
#if NEXUS_HAS_AUDIO
    NEXUS_AudioDecoder_GetStatus(gHandles.audioDecoder, &audioStatus);
    printf("audio0            pts %#x, stc %#x (diff %d) fifo=%d%%\n",
        audioStatus.pts, stc, audioStatus.pts - stc, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0);
    NEXUS_AudioDecoder_GetStatus(gHandles.audioPassthrough, &audioStatus);
    if ( audioStatus.started )
    {
        printf("audio1            pts %#x, stc %#x (diff %d) fifo=%d%%\n",
            audioStatus.pts, stc, audioStatus.pts - stc, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0);
    }
#endif
#if NEXUS_HAS_PLAYBACK
    if (gHandles.playpump) {
        NEXUS_PlaypumpStatus playpumpStatus;
        if(NEXUS_Playpump_GetStatus(gHandles.playpump,&playpumpStatus)==NEXUS_SUCCESS) {
            printf("playpump         fifo=%d%%\n", playpumpStatus.fifoSize?(playpumpStatus.fifoDepth*100)/playpumpStatus.fifoSize:0);
        }
    }
#endif

}

#if defined(LINUX)
#include <sys/time.h>
#elif defined(VXWORKS)
#include <timeCommon.h>
#elif defined (NEXUS_BASE_OS_ucos)
#include "type.h"
#include "ucos.h"
#endif
unsigned b_get_time()
{
#if defined(LINUX)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
#elif defined(VXWORKS)
    struct timespec tv;
    clock_gettime(CLOCK_REALTIME, &tv);
    return tv.tv_sec*1000 + tv.tv_nsec/1000/1000;
#elif defined(NEXUS_BASE_OS_ucos)
    const unsigned msPerTick = 10;
    long osTicks = OSTimeGet();
    return (osTicks * msPerTick);
#else
#error Please implement b_get_time for your OS
#endif
}

