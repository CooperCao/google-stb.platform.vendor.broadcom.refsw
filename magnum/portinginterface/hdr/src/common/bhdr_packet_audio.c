/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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

 ******************************************************************************/
#include "bavc_hdmi.h"

#include "bhdr_priv.h"

BDBG_MODULE(BHDR_PACKET_AUDIO) ;


#if BHDR_CONFIG_DEBUG_PACKET_AUDIO
static void BHDR_P_DEBUG_AudioInfoFrame(
	BAVC_HDMI_AudioInfoFrame *OldAudioInfoFrame,
	BAVC_HDMI_AudioInfoFrame *NewAudioInfoFrame)
{
	BDBG_MSG(("=== NEW AUDIO IF ===")) ;

	if (OldAudioInfoFrame->CodingType != NewAudioInfoFrame->CodingType)
	{
		BDBG_MSG(("(CT3_CT0)       Coding Type          : %s (%d) was %s (%d)",
			BAVC_HDMI_AudioInfoFrame_CodingTypeToStr(NewAudioInfoFrame->CodingType),
			NewAudioInfoFrame->CodingType,
			BAVC_HDMI_AudioInfoFrame_CodingTypeToStr(OldAudioInfoFrame->CodingType),
			OldAudioInfoFrame->CodingType)) ;
	}
	else
	{
		BDBG_MSG(("(CT3_CT0)       Coding Type         : %s (%d)",
		BAVC_HDMI_AudioInfoFrame_CodingTypeToStr(NewAudioInfoFrame->CodingType),
		NewAudioInfoFrame->CodingType)) ;
	}


	if (OldAudioInfoFrame->ChannelCount != NewAudioInfoFrame->ChannelCount)
	{
		BDBG_MSG(("(CC2_CC0)       Channel Count       : %s (%d) was %s (%d)",
			BAVC_HDMI_AudioInfoFrame_ChannelCountToStr(NewAudioInfoFrame->ChannelCount),
			NewAudioInfoFrame->ChannelCount,
			BAVC_HDMI_AudioInfoFrame_ChannelCountToStr(OldAudioInfoFrame->ChannelCount),
			OldAudioInfoFrame->ChannelCount)) ;
	}
	else
	{
		BDBG_MSG(("(CC2_CC0)       Channel Count       : %s (%d)",
			BAVC_HDMI_AudioInfoFrame_ChannelCountToStr(NewAudioInfoFrame->ChannelCount),
			NewAudioInfoFrame->ChannelCount)) ;
	}


	if (OldAudioInfoFrame->SampleFrequency != NewAudioInfoFrame->SampleFrequency)
	{
		BDBG_MSG(("(SF2_SF0)       Sample Frequency    : %s (%d) was %s (%d)",
			BAVC_HDMI_AudioInfoFrame_SampleFrequencyToStr(NewAudioInfoFrame->SampleFrequency),
			NewAudioInfoFrame->SampleFrequency,
			BAVC_HDMI_AudioInfoFrame_SampleFrequencyToStr(OldAudioInfoFrame->SampleFrequency),
			OldAudioInfoFrame->SampleFrequency)) ;
	}
	else
	{
		BDBG_MSG(("(SF2_SF0)       Sample Frequency    : %s (%d)",
			BAVC_HDMI_AudioInfoFrame_SampleFrequencyToStr(NewAudioInfoFrame->SampleFrequency),
			NewAudioInfoFrame->SampleFrequency)) ;
	}


	if (OldAudioInfoFrame->SampleSize != NewAudioInfoFrame->SampleSize)
	{
		BDBG_MSG(("(SS1SS0)        Sample Size         : %s (%d) was %s (%d)",
			BAVC_HDMI_AudioInfoFrame_SampleSizeToStr(NewAudioInfoFrame->SampleSize),
			NewAudioInfoFrame->SampleSize,
			BAVC_HDMI_AudioInfoFrame_SampleSizeToStr(OldAudioInfoFrame->SampleSize),
			OldAudioInfoFrame->SampleSize)) ;
	}
	else
	{
		BDBG_MSG(("(SS1SS0)        Sample Size         : %s (%d)",
			BAVC_HDMI_AudioInfoFrame_SampleSizeToStr(NewAudioInfoFrame->SampleSize),
			NewAudioInfoFrame->SampleSize)) ;
	}


	if (OldAudioInfoFrame->LevelShift != NewAudioInfoFrame->LevelShift)
	{
		BDBG_MSG(("(LSV3_LSV0)     Level Shift Value   : %s (%d) was %s (%d)",
			BAVC_HDMI_AudioInfoFrame_LevelShiftToStr(NewAudioInfoFrame->LevelShift),
			NewAudioInfoFrame->LevelShift,
			BAVC_HDMI_AudioInfoFrame_LevelShiftToStr(OldAudioInfoFrame->LevelShift),
			OldAudioInfoFrame->LevelShift)) ;
	}
	else
	{	BDBG_MSG(("(LSV3_LSV0)     Level Shift Value   : %s (%d)",
		BAVC_HDMI_AudioInfoFrame_LevelShiftToStr(NewAudioInfoFrame->LevelShift),
		NewAudioInfoFrame->LevelShift)) ;
	}


	if (OldAudioInfoFrame->DownMixInhibit != NewAudioInfoFrame->DownMixInhibit)
	{
		BDBG_MSG(("(DM)            Down Mix            : %s (%d) was %s (%d)",
			BAVC_HDMI_AudioInfoFrame_DownMixInhibitToStr(NewAudioInfoFrame->DownMixInhibit),
			NewAudioInfoFrame->DownMixInhibit,
			BAVC_HDMI_AudioInfoFrame_DownMixInhibitToStr(OldAudioInfoFrame->DownMixInhibit),
			OldAudioInfoFrame->DownMixInhibit)) ;
	}
	else
	{
		BDBG_MSG(("(DM)            Down Mix            : %s (%d)",
			BAVC_HDMI_AudioInfoFrame_DownMixInhibitToStr(NewAudioInfoFrame->DownMixInhibit),
			NewAudioInfoFrame->DownMixInhibit)) ;
	}
	BDBG_MSG(("=== END AUDIO IF ===")) ;
}
#endif

/******************************************************************************
Summary:
Parse Audio Info Frame data from received packet
*******************************************************************************/
BERR_Code BHDR_P_ParseAudioInfoFrameData_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet)
{
	uint8_t temp ;
	BAVC_HDMI_AudioInfoFrame stNewAudioInfoFrame ;

	BDBG_ENTER(BHDR_P_ParseAudioInfoFrameData_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	/* zero out the declared Audio Infoframe Structure */
	BKNI_Memset_isr(&stNewAudioInfoFrame, 0, sizeof(stNewAudioInfoFrame)) ;

	/* keep a copy of the raw HDMI Packet structure */
	BKNI_Memcpy_isr(&stNewAudioInfoFrame.stPacket, Packet, sizeof(BAVC_HDMI_Packet));

	/* parse the various fields in the packet */
	temp = Packet->DataBytes[1] ;
	temp = temp & 0x07 ;
	stNewAudioInfoFrame.ChannelCount = temp ;

	temp = Packet->DataBytes[1] ;
	temp = temp & 0xF0 ;
	temp = temp >> 4 ;
	stNewAudioInfoFrame.CodingType = temp ;


	temp = Packet->DataBytes[2] ;
	temp = temp & 0x03,
	 stNewAudioInfoFrame.SampleSize = temp ;

	temp = Packet->DataBytes[2] ;
	temp = temp & 0x1C ;
	temp = temp >> 2 ;
	stNewAudioInfoFrame.SampleFrequency = temp ;


	temp = Packet->DataBytes[4] ;
	temp = temp & 0x1F ;
	stNewAudioInfoFrame.SpeakerAllocation = temp ;


	temp = Packet->DataBytes[5] ;
	temp = temp & 0x78 ;
	temp = temp >> 3 ;
	stNewAudioInfoFrame.LevelShift = temp ;

	temp = Packet->DataBytes[5] ;
	temp = temp & 0x80 ;
	temp = temp >> 7 ;
	stNewAudioInfoFrame.DownMixInhibit = temp ;

#if BHDR_CONFIG_DEBUG_PACKET_AUDIO
	BHDR_P_DEBUG_AudioInfoFrame(&hHDR->AudioInfoFrame, &stNewAudioInfoFrame) ;
#endif

	/* copy the new packet to the handle for use later */
	BKNI_Memcpy_isr(&hHDR->AudioInfoFrame, &stNewAudioInfoFrame, sizeof(BAVC_HDMI_AudioInfoFrame)) ;

	/* make sure MAI bus reflects any updated channel count info */
	BHDR_P_ConfigureAudioMaiBus_isr(hHDR) ;

	/* call  the callback functions for Audio Change notification  */
	if (hHDR->pfAudioFormatChangeCallback)
	{
		hHDR->pfAudioFormatChangeCallback(hHDR->pvAudioFormatChangeParm1,
			hHDR->iAudioFormatChangeParm2, &hHDR->AudioData);
	}

	BDBG_LEAVE(BHDR_P_ParseAudioInfoFrameData_isr) ;
	return BERR_SUCCESS ;
}


/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_GetAudioInfoFrameData(BHDR_Handle hHDR,
	BAVC_HDMI_AudioInfoFrame *AudioInfoFrame)
{
	BKNI_Memcpy(AudioInfoFrame, &hHDR->AudioInfoFrame, sizeof(BAVC_HDMI_AudioInfoFrame)) ;
	return BERR_SUCCESS ;
}
