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


BDBG_MODULE(BHDR_PACKET_SPD) ;


#if BHDR_CONFIG_DEBUG_PACKET_SPD
static void BHDR_P_DEBUG_SpdInfoFrame(
	BAVC_HDMI_SPDInfoFrame *OldSpdInfoFrame,
	BAVC_HDMI_SPDInfoFrame *NewSpdInfoFrame)
{
	BSTD_UNUSED(OldSpdInfoFrame) ;

	BDBG_LOG(("=== NEW SPD IF ===")) ;
	BDBG_LOG(("Vendor Name: <%s>", NewSpdInfoFrame->VendorName)) ;
	BDBG_LOG(("Description: <%s>",  NewSpdInfoFrame->ProductDescription)) ;
	BDBG_LOG(("Type: %s (%d)",
		BAVC_HDMI_SpdInfoFrame_SourceTypeToStr(NewSpdInfoFrame->SourceDeviceInfo),
		NewSpdInfoFrame->SourceDeviceInfo)) ;

	/* use a marker to separate packet data */
	BDBG_LOG(("=== END SPD IF ===")) ;
}
#endif


/******************************************************************************
Summary:
Parse SPD Info Frame data from received packet
*******************************************************************************/
BERR_Code BHDR_P_ParseSPDInfoFrameData_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet)
{
	BAVC_HDMI_SPDInfoFrame stNewSpdInfoFrame ;

	BDBG_ENTER(BHDR_P_ParseSPDInfoFrameData_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	BKNI_Memset_isr(&stNewSpdInfoFrame, 0, sizeof(BAVC_HDMI_SPDInfoFrame)) ;

	/* Keep a raw copy of the SPD packet  */
	BKNI_Memcpy(&stNewSpdInfoFrame.stPacket, Packet, sizeof(BAVC_HDMI_Packet)) ;


	BKNI_Memcpy_isr(stNewSpdInfoFrame.VendorName,
		&Packet->DataBytes[BAVC_HDMI_SPD_IF_VENDOR_OFFSET],
		BAVC_HDMI_SPD_IF_VENDOR_LEN) ;

	stNewSpdInfoFrame.VendorName[BAVC_HDMI_SPD_IF_VENDOR_LEN] = '\0' ;

	BKNI_Memcpy_isr(stNewSpdInfoFrame.ProductDescription,
		&Packet->DataBytes[BAVC_HDMI_SPD_IF_DESC_OFFSET],
		BAVC_HDMI_SPD_IF_DESC_LEN) ;

	stNewSpdInfoFrame.ProductDescription[BAVC_HDMI_SPD_IF_DESC_LEN] = '\0' ;

	stNewSpdInfoFrame.SourceDeviceInfo =
		Packet->DataBytes[BAVC_HDMI_SPD_IF_DEVICE_INFO_OFFSET] ;

#if BHDR_CONFIG_DEBUG_PACKET_SPD
	BHDR_P_DEBUG_SpdInfoFrame(&hHDR->SPDInfoFrame, &stNewSpdInfoFrame) ;
#endif

	/* copy the new packet struct data */
	BKNI_Memcpy_isr(&hHDR->SPDInfoFrame, &stNewSpdInfoFrame, sizeof(BAVC_HDMI_SPDInfoFrame)) ;


	BDBG_LEAVE(BHDR_P_ParseSPDInfoFrameData_isr) ;
	return BERR_SUCCESS ;
}

/******************************************************************************
Summary:
**7*****************************************************************************/
BERR_Code BHDR_GetSPDInfoFrameData(BHDR_Handle hHDR,
	BAVC_HDMI_SPDInfoFrame *SPDInfoFrame)
{
	BKNI_Memcpy(SPDInfoFrame, &hHDR->SPDInfoFrame, sizeof(BAVC_HDMI_SPDInfoFrame)) ;
	return BERR_SUCCESS ;
}
