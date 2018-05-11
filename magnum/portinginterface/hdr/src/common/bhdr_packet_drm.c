/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bavc_hdmi.h"

#include "bhdr_priv.h"

BDBG_MODULE(BHDR_PACKET_DRM) ;

#if BHDR_CONFIG_DEBUG_PACKET_DRM
static void BHDR_P_DEBUG_DrmInfoFrame_isr(
	BAVC_HDMI_DRMInfoFrame *OldDrmInfoFrame,
	BAVC_HDMI_DRMInfoFrame *NewDrmInfoFrame)
{
	uint8_t i ;
	BSTD_UNUSED(OldDrmInfoFrame) ;
	static const char * colorimetryNames[] =
	{
	    "Green Primary",
	    "Blue Primary",
	    "Red Primary",
	    "White Point",
	    NULL
	};

	BDBG_LOG(("=== NEW DRM IF ===")) ;

	BDBG_LOG(("DRM EOTF     %s",
		BAVC_HDMI_DRMInfoFrame_EOTFToStr(NewDrmInfoFrame->eEOTF))) ;

	BDBG_LOG(("DRM Descriptor ID   %s",
		BAVC_HDMI_DRMInfoFrame_DescriptorIdToStr(NewDrmInfoFrame->eDescriptorId))) ;
	if (NewDrmInfoFrame->eDescriptorId == BAVC_HDMI_DRM_DescriptorId_eType1)
	{
        const BAVC_Point * points[BAVC_NUM_COLORIMETRY_POINTS];

        /* this maps GBRW -> 0123 per original CEA/SMPTE guidelines */
        points[0] = &NewDrmInfoFrame->stType1.stMasteringDisplayColorVolume.stColorVolume.stPrimaries.stGreen;
        points[1] = &NewDrmInfoFrame->stType1.stMasteringDisplayColorVolume.stColorVolume.stPrimaries.stBlue;
        points[2] = &NewDrmInfoFrame->stType1.stMasteringDisplayColorVolume.stColorVolume.stPrimaries.stRed;
        points[3] = &NewDrmInfoFrame->stType1.stMasteringDisplayColorVolume.stColorVolume.stWhitePoint;

		for (i = 0 ; i < BAVC_NUM_COLORIMETRY_POINTS; i++)
		{
			BDBG_LOG(("%s x: %d, y: %d", i, points[i]->ulX, points[i]->ulY)) ;
		}

		BDBG_LOG(("Display Mastering Luminance MAX: %d",
			NewDrmInfoFrame->stType1.stMasteringDisplayColorVolume.stLuminance.uiMax)) ;

		BDBG_LOG(("Display Mastering Luminance MIN: %d",
			NewDrmInfoFrame->stType1.stMasteringDisplayColorVolume.stLuminance.uiMin)) ;

		BDBG_LOG(("Max Content Light Level: %d",
			NewDrmInfoFrame->stType1.stContentLightLevel.ulMax)) ;

		BDBG_LOG(("Frame-average Light Level: %d",
			NewDrmInfoFrame->stType1.stContentLightLevel.ulAvg)) ;
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
	    /* DRMIF-to-struct parser was moved to AVC because it is also used in VDC to get data from DBV layer */
	    BAVC_DeserializeStaticHdrMetadata_isrsafe(&stNewDrmInfoFrame.stType1, BAVC_TYPE1_STATIC_HDR_METADATA_LEN, Packet->DataBytes + 3) ;
	    /* convert from uint16_t-stored 1 nits units to 100 micronits units now that we have more space */
	    stNewDrmInfoFrame.stType1.stMasteringDisplayColorVolume.stLuminance.uiMax *= 10000;
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
