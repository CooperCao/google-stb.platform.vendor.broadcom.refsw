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

BDBG_MODULE(BHDR_PACKET_VSI) ;

#if BHDR_CONFIG_DEBUG_PACKET_VSI
static void BHDR_P_DEBUG_VsInfoFrame_isr(
	BAVC_HDMI_VendorSpecificInfoFrame *OldVsInfoFrame,
	BAVC_HDMI_VendorSpecificInfoFrame *NewVsInfoFrame)
{
	BSTD_UNUSED(OldVsInfoFrame) ;

	BDBG_LOG(("=== NEW VS IF ===")) ;

	BDBG_LOG(("IEEE RegId: <0x%02x%02X%02X>",
		NewVsInfoFrame->uIEEE_RegId[2],
		NewVsInfoFrame->uIEEE_RegId[1],
		NewVsInfoFrame->uIEEE_RegId[0])) ;

	BDBG_LOG(("HDMI Video Format: <%s> (%d)",
		BAVC_HDMI_VsInfoFrame_HdmiVideoFormatToStr(
			NewVsInfoFrame->eHdmiVideoFormat),
			NewVsInfoFrame->eHdmiVideoFormat)) ;

	switch (NewVsInfoFrame->eHdmiVideoFormat)
	{
	default :
	case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eNone :
		break ;

	case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eExtendedResolution:
		BDBG_LOG(("HDMI VIC: <%s> (%d)",
			BAVC_HDMI_VsInfoFrame_HdmiVicToStr(NewVsInfoFrame->eHdmiVic),
				NewVsInfoFrame->eHdmiVic)) ;
		break ;

	case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_e3DFormat :
		BDBG_LOG(("3D Struct: <%s> (%d)",
			BAVC_HDMI_VsInfoFrame_3DStructureToStr(NewVsInfoFrame->e3DStructure),
			NewVsInfoFrame->e3DStructure)) ;

		if ((NewVsInfoFrame->e3DStructure <=  BAVC_HDMI_VSInfoFrame_3DStructure_eSidexSideHalf)
		&& (NewVsInfoFrame->e3DStructure !=  BAVC_HDMI_VSInfoFrame_3DStructure_eReserved))
		{
			BDBG_LOG(("3D Ext Data: <%s> (%d)",
				BAVC_HDMI_VsInfoFrame_3DExtDataToStr(NewVsInfoFrame->e3DExtData),
				NewVsInfoFrame->e3DExtData)) ;
		}
		else
		{
			BDBG_LOG(("3D Ext Data: %d is not used", NewVsInfoFrame->e3DExtData)) ;
		}
		break ;

	}

	/* use a marker to separate packet data */
	BDBG_LOG(("=== END VS IF ===")) ;
}
#endif

BERR_Code BHDR_P_ParseVendorSpecificInfoFrameData_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet)
{
	uint8_t temp ;
	static const uint8_t HDMI_IEEE_0x00C003[] = {0x03, 0x0C, 0x00} ;

	BAVC_HDMI_VendorSpecificInfoFrame stNewVsInfoFrame ;

	BDBG_ENTER(BHDR_P_ParseVendorSpecificInfoFrameData_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	BKNI_Memset_isr(&stNewVsInfoFrame, 0, sizeof(BAVC_HDMI_VendorSpecificInfoFrame)) ;

	/* Keep a raw copy of the SPD packet  */
	BKNI_Memcpy_isr(&stNewVsInfoFrame.stPacket, Packet, sizeof(BAVC_HDMI_Packet)) ;

	/* copy the IEEE Reg ID value */
	BKNI_Memcpy_isr(&stNewVsInfoFrame.uIEEE_RegId,
		&Packet->DataBytes[ BAVC_HDMI_VS_IEEE_REGID_OFFSET],
		BAVC_HDMI_IEEE_REGID_LEN) ;

	if (BKNI_Memcmp_isr(&stNewVsInfoFrame.uIEEE_RegId, HDMI_IEEE_0x00C003,
		BAVC_HDMI_IEEE_REGID_LEN))
	{
		BDBG_WRN(("Proprietary IEEE RegId: <0x%02X%02X%02X> is UNKNOWN/UNSUPPORTED",
			stNewVsInfoFrame.uIEEE_RegId[2], stNewVsInfoFrame.uIEEE_RegId[1],
			stNewVsInfoFrame.uIEEE_RegId[0])) ;
		goto done ;
	}

	/* parse the various fields in the packet */

	/* HDMI Video Format */
	temp = Packet->DataBytes[4] ;
	temp = temp & 0xE0 ;
	temp = temp >> 5 ;
	stNewVsInfoFrame.eHdmiVideoFormat = temp ;

	switch(stNewVsInfoFrame.eHdmiVideoFormat)
	{
	case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eExtendedResolution :
	/* HDMI VIC */
		temp = Packet->DataBytes[5] ;
		stNewVsInfoFrame.eHdmiVic = temp ;
		break ;

	case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_e3DFormat :
	/* OR  HDMI 3D Structure */
		temp = Packet->DataBytes[5] ;
		temp = temp & 0xF0 ;
		temp = temp >> 4 ;
		stNewVsInfoFrame.e3DStructure = temp ;

		/* get the 3D Ext Data regardless if we know it is valid or not */
		temp = Packet->DataBytes[6] ;
		temp = temp & 0xF0 ;
		temp = temp >> 4 ;
		stNewVsInfoFrame.e3DExtData = temp ;

		break ;

	case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eNone :
		/* do nothing */
		break ;

	default :
		BDBG_WRN(("VSI HDMI Video Format Reserved (%d)",
			stNewVsInfoFrame.eHdmiVideoFormat)) ;
	}

done:
#if BHDR_CONFIG_DEBUG_PACKET_VSI
	BHDR_P_DEBUG_VsInfoFrame_isr(& hHDR->VSInfoFrame, &stNewVsInfoFrame) ;
#endif

	/* copy the new packet to the handle for use later */
	BKNI_Memcpy_isr(&hHDR->VSInfoFrame, &stNewVsInfoFrame,
		sizeof(BAVC_HDMI_VendorSpecificInfoFrame)) ;

	BDBG_LEAVE(BHDR_P_ParseVendorSpecificInfoFrameData_isr) ;

	return BERR_SUCCESS ;
}

/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_GetVendorSpecificInfoFrameData(BHDR_Handle hHDR,
	BAVC_HDMI_VendorSpecificInfoFrame *VendorSpecificInfoFrame)
{
	BKNI_Memcpy(VendorSpecificInfoFrame, &hHDR->VSInfoFrame, sizeof(BAVC_HDMI_VendorSpecificInfoFrame)) ;
	return BERR_SUCCESS ;
}
