/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bhdm.h"
#include "bhdm_priv.h"
#include "bavc.h"

BDBG_MODULE(BHDM_PACKET_AUDIO) ;

#define BHDM_REFER_TO_STREAM_HEADER 0

/******************************************************************************
Summary:
Set/Enable the Audio Info Frame packet to be sent to the HDMI Rx
*******************************************************************************/
BERR_Code BHDM_SetAudioInfoFramePacket(
   const BHDM_Handle hHDMI,		  /* [in] HDMI handle */
   BAVC_HDMI_AudioInfoFrame *pstAudioInfoFrame
)
{
	BERR_Code	   rc = BERR_SUCCESS ;

	BHDM_Packet PhysicalHdmiRamPacketId ;

	uint8_t PacketType ;
	uint8_t PacketVersion ;
	uint8_t PacketLength ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* BHDM_SetAudioInfoFramePacket also called from BHDM_EnableDisplay
	    using the AudioInfoFrame stored in the HDMI handle
	*/

	/* initialize packet information to zero */
	BKNI_Memset(hHDMI->PacketBytes, 0, BHDM_NUM_PACKET_BYTES) ;

	PhysicalHdmiRamPacketId = BHDM_PACKET_eAudioFrame_ID ;

	PacketType	  = BAVC_HDMI_PacketType_eAudioInfoFrame ;
	PacketVersion = BAVC_HDMI_PacketType_AudioInfoFrameVersion  ;
	PacketLength = 10 ;

	if ((!pstAudioInfoFrame->bOverrideDefaults)
	&& ((pstAudioInfoFrame->CodingType != BAVC_HDMI_AudioInfoFrame_CodingType_eReferToStream)
	||  (pstAudioInfoFrame->SampleFrequency)
	||  (pstAudioInfoFrame->SampleSize)))
	{
		BDBG_WRN((
			"Tx%d: Audio Coding Type, Sample Size, and Frequency are obtained from stream header",
			 hHDMI->eCoreId)) ;

		/* set the audio coding type, sample frequency, and sample size ALL to 0 */
		pstAudioInfoFrame->CodingType =
		pstAudioInfoFrame->SampleFrequency =
		pstAudioInfoFrame->SampleSize
			=  BHDM_REFER_TO_STREAM_HEADER ;
	}


	hHDMI->PacketBytes[1] =
		   pstAudioInfoFrame->CodingType << 4
		| (pstAudioInfoFrame->ChannelCount) ;
	hHDMI->PacketBytes[2] =
		   pstAudioInfoFrame->SampleFrequency << 2
		| pstAudioInfoFrame->SampleSize ;


	hHDMI->PacketBytes[3] = 0 ; /* Per HDMI Spec... Set to 0  */

	hHDMI->PacketBytes[4] =
		pstAudioInfoFrame->SpeakerAllocation ;

	hHDMI->PacketBytes[5] =
		pstAudioInfoFrame->DownMixInhibit << 7
		| pstAudioInfoFrame->LevelShift << 3 ;

	/* adjust the Audio Input Configuration to reflect any changes */
	BHDM_P_ConfigureInputAudioFmt(hHDMI, pstAudioInfoFrame) ;

	BHDM_CHECK_RC(rc, BHDM_P_WritePacket(
		hHDMI, PhysicalHdmiRamPacketId,
		PacketType, PacketVersion, PacketLength, hHDMI->PacketBytes)) ;

	/* update current device settings with new information on AudioInfoFrame (if external) */
		BKNI_Memcpy(&hHDMI->DeviceSettings.stAudioInfoFrame, pstAudioInfoFrame,
			sizeof(BAVC_HDMI_AudioInfoFrame)) ;


done:
	return rc ;
}



/******************************************************************************
Summary:
Get the Audio Information Info frame sent to the HDMI Rx
*******************************************************************************/
BERR_Code BHDM_GetAudioInfoFramePacket(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   BAVC_HDMI_AudioInfoFrame *stAudioInfoFrame
)
{
	BERR_Code	   rc = BERR_SUCCESS ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_Memset(stAudioInfoFrame, 0, sizeof(BAVC_HDMI_AudioInfoFrame)) ;

	BKNI_Memcpy(stAudioInfoFrame, &(hHDMI->DeviceSettings.stAudioInfoFrame),
		sizeof(BAVC_HDMI_AudioInfoFrame)) ;

	return rc ;
}
