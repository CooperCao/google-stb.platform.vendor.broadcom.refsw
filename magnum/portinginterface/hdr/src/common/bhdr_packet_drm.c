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

BDBG_MODULE(BHDR_PACKET_DRM) ;

#define BHDR_DRM_PACKET_NUM_DISPLAY_PRIMARIES 3

#if BHDR_CONFIG_DEBUG_PACKET_DRM
static void BHDR_P_DEBUG_DrmInfoFrame_isr(
	BAVC_HDMI_DRMInfoFrame *OldDrmInfoFrame,
	BAVC_HDMI_DRMInfoFrame *NewDrmInfoFrame)
{
	uint8_t i ;
	BSTD_UNUSED(OldDrmInfoFrame) ;

	BDBG_LOG(("=== NEW DRM IF ===")) ;

	BDBG_LOG(("DRM EOTF     %s",
		BAVC_HDMI_DRMInfoFrame_EOTFToStr(NewDrmInfoFrame->eEOTF))) ;

	BDBG_LOG(("DRM Descriptor ID   %s",
		BAVC_HDMI_DRMInfoFrame_DescriptorIdToStr(NewDrmInfoFrame->eDescriptorId))) ;
	if (NewDrmInfoFrame->eDescriptorId == BAVC_HDMI_DRM_DescriptorId_eType1)
	{
		for (i = 0 ; i < BHDR_DRM_PACKET_NUM_DISPLAY_PRIMARIES; i++)
		{
			BDBG_LOG(("Display_primaries[%d] x: %d, y: %d", i,
				NewDrmInfoFrame->Type1.DisplayPrimaries[i].X,
				NewDrmInfoFrame->Type1.DisplayPrimaries[i].Y)) ;
		}

		BDBG_LOG(("White Point x: %d, y: %d",
			NewDrmInfoFrame->Type1.WhitePoint.X ,
			NewDrmInfoFrame->Type1.WhitePoint.Y)) ;

		BDBG_LOG(("Display Mastering Luminance MAX: %d",
			NewDrmInfoFrame->Type1.DisplayMasteringLuminance.Max)) ;

		BDBG_LOG(("Display Mastering Luminance MIN: %d",
			NewDrmInfoFrame->Type1.DisplayMasteringLuminance.Min)) ;

		BDBG_LOG(("Max Content Light Level: %d",
			NewDrmInfoFrame->Type1.MaxContentLightLevel)) ;

		BDBG_LOG(("Frame-average Light Level: %d",
			NewDrmInfoFrame->Type1.MaxFrameAverageLightLevel)) ;
	}
	else
	{
		BDBG_WRN(("Unknown/Unsupported DRM packet %d",
			NewDrmInfoFrame->eDescriptorId)) ;
	}


	/* use a marker to separate packet data */
	BDBG_LOG(("=== END DRM IF ===")) ;
}
#endif

BERR_Code BHDR_P_ParseDrmInfoFrameData_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet)
{
	uint8_t i ;

	BAVC_HDMI_DRMInfoFrame stNewDrmInfoFrame ;

	BDBG_ENTER(BHDR_P_ParseDrmInfoFrameData_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	BKNI_Memset_isr(&stNewDrmInfoFrame, 0, sizeof(BAVC_HDMI_DRMInfoFrame)) ;

	/* Keep a raw copy of the SPD packet  */
	BKNI_Memcpy_isr(&stNewDrmInfoFrame.stPacket, Packet, sizeof(BAVC_HDMI_Packet)) ;

	/* parse the various fields in the packet */

	/* DRM EOTF and Descriptor */
	stNewDrmInfoFrame.eEOTF = Packet->DataBytes[1] ;
	stNewDrmInfoFrame.eDescriptorId = Packet->DataBytes[2] ;

	if (stNewDrmInfoFrame.eDescriptorId == BAVC_HDMI_DRM_DescriptorId_eType1)
	{
		/* Display Primaries */
		for (i = 0; i < BHDR_DRM_PACKET_NUM_DISPLAY_PRIMARIES ; i++)
		{
			stNewDrmInfoFrame.Type1.DisplayPrimaries[i].X =
				  Packet->DataBytes[3 + i * 4 ]
				| (Packet->DataBytes[4 + i * 4] & 0x00FF) << 8 ;

			stNewDrmInfoFrame.Type1.DisplayPrimaries[i].Y =
				  Packet->DataBytes[5 + i * 4 ]
				| (Packet->DataBytes[6 + i * 4] & 0x00FF) << 8 ;
		}

		/* White Point */
		stNewDrmInfoFrame.Type1.WhitePoint.X =
			  Packet->DataBytes[15]
			| (Packet->DataBytes[16] & 0x00FF) << 8 ;

		stNewDrmInfoFrame.Type1.WhitePoint.Y =
			  Packet->DataBytes[17]
			| (Packet->DataBytes[18] & 0x00FF) << 8 ;


		/* Max Display Mastering Luminance */
		stNewDrmInfoFrame.Type1.DisplayMasteringLuminance.Max =
			  Packet->DataBytes[19]
			| (Packet->DataBytes[20] & 0x00FF) << 8 ;

		/* Min Display Mastering Luminance */
		stNewDrmInfoFrame.Type1.DisplayMasteringLuminance.Min =
			  Packet->DataBytes[21]
			| (Packet->DataBytes[22] & 0x00FF) << 8 ;

		/* Max Content Light Level */
		stNewDrmInfoFrame.Type1.MaxContentLightLevel =
			  Packet->DataBytes[23]
			| (Packet->DataBytes[24] & 0x00FF) << 8 ;

		/* Max Frame Average Light Level */
		stNewDrmInfoFrame.Type1.MaxFrameAverageLightLevel =
			  Packet->DataBytes[23]
			| (Packet->DataBytes[24] & 0x00FF) << 8 ;

	}
	else
	{
		BDBG_WRN(("Unsupported Descriptor (TypeId) : %d",
			stNewDrmInfoFrame.eDescriptorId )) ;
	}

#if BHDR_CONFIG_DEBUG_PACKET_DRM
	BHDR_P_DEBUG_DrmInfoFrame_isr(& hHDR->DRMInfoFrame, &stNewDrmInfoFrame) ;
#endif

	/* copy the new packet to the handle for use later */
	BKNI_Memcpy_isr(&hHDR->DRMInfoFrame, &stNewDrmInfoFrame,
		sizeof(BAVC_HDMI_DRMInfoFrame)) ;

	BDBG_LEAVE(BHDR_P_ParseDrmInfoFrameData_isr) ;

	return BERR_SUCCESS ;
}

/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_GetDrmiInfoFrameData(BHDR_Handle hHDR,
	BAVC_HDMI_DRMInfoFrame *pDrmInfoFrame)
{
	BKNI_Memcpy(pDrmInfoFrame, &hHDR->DRMInfoFrame, sizeof(BAVC_HDMI_DRMInfoFrame)) ;
	return BERR_SUCCESS ;
}
