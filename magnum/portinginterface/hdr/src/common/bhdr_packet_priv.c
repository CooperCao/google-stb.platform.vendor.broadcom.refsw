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

#include "bhdr.h"
#include "bhdr_priv.h"
#include "bhdr_debug.h"

BDBG_MODULE(BHDR_PACKET_PRIV) ;


 /******************************************************************************
Summary:
Data structure used to map RAM Packet Types to physical packet memory (registers).
As of 01/01/2007 these are the known Packet Types

Packet Type Value Packet Type
	0x00 Null
	0x01 Audio Clock Regeneration (N/CTS)
	0x02 Audio Sample
	0x03 General Control
	0x04 AudioContentProtection Packet
	0x05 ISRC1 Packet
	0x06 ISRC2 Packet
	0x07 Reserved for DSD Packet

0x80+InfoFrame Type EIA/CEA-861B InfoFrame
	0x81 Vendor-Specific InfoFrame
	0x82 AVI InfoFrame
	0x83 Source Product Descriptor InfoFrame
	0x84 Audio InfoFrame
	0x85 MPEG Source InfoFrame


The list below can be modified, but is best left in its current configuration.  It maps which HDMI
Rx Registers will get which HDMI Packets e.g. Packet 4 will receive the AVI Infoframe Packet.
The eUnused packet is unassigned and therefore no packet will be written to these registers.

In the event of errors in the stream, the packet memory (registers) may have updates since
random data *might* match the packet types listed above.  Infoframe packets types have
checksums which can be used to filter bad packets.
*******************************************************************************/
typedef struct BHDR_P_RamPacketMap
{
	BAVC_HDMI_PacketType ePacketType ;
	bool bMonitorStoppedPackets ;
} BHDR_P_RamPacketMap ;

static const BHDR_P_RamPacketMap RamPacketMap[BHDR_P_NUM_PACKETS] =
{
	/* Packet 00 */ {BAVC_HDMI_PacketType_eGamutMetadataPacket, false},

	/*
	    the clock regeneration packet is ideally used for debug purposes
	    future designs may pass this information directly to the audio core

	    DO NOT MOVE the AudioClockRegeneration Packet from Packet 01
	    other code in this unit depend on this physical location

	    See bhdr_packet_acr.c for usage
	 */

	/* Packet 01 */ {BAVC_HDMI_PacketType_eAudioClockRegeneration, false},

	/* Packet 02 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 03 */ {BAVC_HDMI_PacketType_eGeneralControl, false},

	/* Packet 04 */ {BAVC_HDMI_PacketType_eAviInfoFrame, true},
	/* Packet 05 */ {BAVC_HDMI_PacketType_eSpdInfoFrame, false},
	/* Packet 06 */ {BAVC_HDMI_PacketType_eAudioInfoFrame, false },
	/* Packet 07 */ {BAVC_HDMI_PacketType_eVendorSpecificInfoframe, true},
	/* Packet 08 */ {BAVC_HDMI_PacketType_eMpegInfoFrame, false},

	/* Packet 09 */ {BAVC_HDMI_PacketType_eAudioContentProtection, true},
	/* Packet 10 */ {BAVC_HDMI_PacketType_eISRC1, true},
	/* Packet 11 */ {BAVC_HDMI_PacketType_eISRC2, true},

	/* Packet 12 */ {BAVC_HDMI_PacketType_eDirectStream, false},

	/* Packet 13 */ {BAVC_HDMI_PacketType_eDrmInfoFrame, true},
	/* Packet 14 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 15 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 16 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 17 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 18 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 19 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 20 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 21 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 22 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 23 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 24 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 25 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 26 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 27 */ {BAVC_HDMI_PacketType_eUnused, false}
}  ;



#define BHDR_P_GET_PACKET_RAM_INDEX_BY_TYPE(packetType, index) \
	do \
	{ \
		int i; \
		for (i=0; i<BHDR_P_NUM_PACKETS; i++) \
		{ \
			if (RamPacketMap[i].ePacketType == packetType) \
			{ \
				index = i; \
				break; \
			} \
		} \
	} while(0)

BERR_Code BHDR_P_GetPacketRamIndexByType(
	BHDR_Handle hHDR, BAVC_HDMI_PacketType ePacketType,  uint8_t *PacketIndex)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t i ;

	BSTD_UNUSED(hHDR) ;

	for (i=0; i < BHDR_P_NUM_PACKETS; i++)
	{
		if (RamPacketMap[i].ePacketType == ePacketType)
		{
			*PacketIndex = i ;
			break;
		}
	}

	if (i == BHDR_P_NUM_PACKETS)
	{
		BDBG_ERR(("Invalid Packet Type")) ;
		rc = BERR_INVALID_PARAMETER ;
	}

	return rc ;
}


/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_P_InitializePacketRAM_isr(
   BHDR_Handle hHDR
)
{

	BREG_Handle hRegister  ;
	uint32_t ulOffset  ;

	uint32_t rc = BERR_SUCCESS ;
	uint32_t Register ;
	uint32_t uiPacketBit ;
	uint8_t PacketNum ;

	uint32_t RegAddr ;


	BDBG_ENTER(BHDR_P_InitializePacketRAM_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;


	/* default Packet Stop Detection to use VSync vs HSync */
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_PACKET_STOP_SYNC_SELECT + ulOffset, 0xFFFFFFF) ;

	/* clear any enabled packet stop detection  */
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_ENABLE_PACKET_STOP_DETECT + ulOffset, 0) ;


	BHDR_P_ENABLE_PACKET_WRITES(hRegister, Register, ulOffset) ;

		/* Zero out all Packet Registers */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CFG + ulOffset) ;
		Register |= BCHP_MASK(HDMI_RX_0_PACKET_PROCESSOR_CFG , INIT_RAM_PACKET) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CFG + ulOffset, Register) ;

		Register &= ~ BCHP_MASK(HDMI_RX_0_PACKET_PROCESSOR_CFG , INIT_RAM_PACKET) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CFG + ulOffset, Register) ;


	BHDR_P_DISABLE_PACKET_WRITES(hRegister, Register, ulOffset) ;



	/* assign the Packet Types to the physical packet RAM; use the
	     HDMI Rx Interrupt table to get Phys Packet number and Packet Type
	 */

	for (PacketNum= 0; PacketNum < BHDR_P_NUM_PACKETS ; PacketNum++)
	{
		/* skip Unused Packet Types */
		if (RamPacketMap[PacketNum].ePacketType == BAVC_HDMI_PacketType_eUnused)
			continue ;

		/* assign the Packet Type to the RAM Packet Number using the RamPacketMap */
		/* NOTE: there is a default mapping at reset/power up */

		RegAddr = BCHP_HDMI_RX_0_RAM_PACKET_TYPES_0  + ulOffset
			+ (uint8_t) (PacketNum / 4) * 4 ;

		Register = BREG_Read32(hRegister, RegAddr) ;
		Register &= ~(0xFF << ((PacketNum % 4) * 8)) ;
		Register |= (RamPacketMap[PacketNum].ePacketType << ((PacketNum % 4) * 8)) ;
		BREG_Write32(hRegister, RegAddr, Register) ;

		/* Enable reception of this Packet */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_RAM_PACKET_UPDATE_CONFIG + ulOffset) ;
		uiPacketBit = 1 << PacketNum ;
		Register |= uiPacketBit ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_RAM_PACKET_UPDATE_CONFIG + ulOffset, Register) ;

		/* Enable the IRQ for reception of this Packet */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_RAM_PACKET_IRQ_ENABLE + ulOffset) ;
		uiPacketBit = 1 << PacketNum ;
		Register |= uiPacketBit ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_RAM_PACKET_IRQ_ENABLE + ulOffset, Register) ;


#if 0
		BDBG_MSG(("Mask %#08X for Packet %d",
			(0xFF << ((PacketNum % 4) * 8)), PacketNum)) ;
#endif
	}


	/* clear individual locks */
	for (PacketNum = 0 ; PacketNum < BHDR_P_NUM_PACKETS; PacketNum++)
	{
		BHDR_P_ClearPacketRAMLock_isr(hHDR, PacketNum) ;
	}

	/* clear internal Packet Ram used for status */
	BHDR_P_ClearPacketMemory_isr(hHDR) ;

	BDBG_LEAVE(BHDR_P_InitializePacketRAM_isr) ;
	return rc ;
}




/******************************************************************************
Summary:
Copy the Packet Bytes from the H/W registers
*******************************************************************************/
void BHDR_P_GetPacketRAM_isr(
	BREG_Handle hRegister, uint32_t PacketRegisterOffset, uint8_t *DataBytes)
{
	uint32_t Register ;
	uint8_t i ;

	for (i = 0 ; i < BAVC_HDMI_PB_LENGTH; i = i + 7)
	{
		Register = BREG_Read32(hRegister, PacketRegisterOffset) ;
		DataBytes[i+0] = (uint8_t) (Register & 0x000000FF) ;
		DataBytes[i+1] = (uint8_t) ((Register & 0x0000FF00) >>  8) ;
		DataBytes[i+2] = (uint8_t) ((Register & 0x00FF0000) >> 16) ;
		DataBytes[i+3] = (uint8_t) ((Register & 0xFF000000) >> 24) ;

		PacketRegisterOffset +=  4 ;

		Register = BREG_Read32(hRegister, PacketRegisterOffset) ;
		DataBytes[i+4] = (uint8_t) (Register & 0x000000FF) ;
		DataBytes[i+5] = (uint8_t) ((Register & 0x0000FF00) >>  8) ;
		DataBytes[i+6] = (uint8_t) ((Register & 0x00FF0000) >> 16) ;

		PacketRegisterOffset +=  4 ;
	}
}



/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_P_ClearPacketRAM_isr(
   BHDR_Handle hHDR, uint8_t PacketNum
)
{
	BREG_Handle hRegister  ;
	uint32_t ulOffset  ;

	uint32_t rc = BERR_SUCCESS ;
	uint32_t Register ;
	uint32_t RegAddr ;

	BDBG_ENTER(BHDR_P_ClearPacketRAM_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	if ((RamPacketMap[PacketNum].ePacketType == BAVC_HDMI_PacketType_eUnused)
	|| (RamPacketMap[PacketNum].ePacketType == BAVC_HDMI_PacketType_eNull))
		goto done ;


	BHDR_P_ENABLE_PACKET_WRITES(hRegister, Register, ulOffset) ;

		/* Zero out all Registers for this Packet */
		RegAddr = BCHP_HDMI_RX_0_RAM_PACKET_0_HEADER  + ulOffset
			+ PacketNum * 4 * BHDR_P_PACKET_WORDS  ;

		for (Register = 0 ; Register < BHDR_P_PACKET_WORDS ; Register++)
		{
			BREG_Write32(hRegister, RegAddr, (uint32_t) 0) ;
			RegAddr = RegAddr + 4 ;
		}

	BHDR_P_DISABLE_PACKET_WRITES(hRegister, Register, ulOffset) ;

done:
	BDBG_LEAVE(BHDR_P_ClearPacketRAM_isr) ;
	return rc ;
}


/******************************************************************************
Summary:
*******************************************************************************/
void BHDR_P_ClearPacketMemory_isr(BHDR_Handle hHDR)
{
	BDBG_ENTER(BHDR_P_ClearPacketMemory_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	BKNI_Memset_isr(&hHDR->AviInfoFrame, 0, sizeof(BAVC_HDMI_AviInfoFrame));
	BKNI_Memset_isr(&hHDR->AudioInfoFrame, 0, sizeof(BAVC_HDMI_AudioInfoFrame));
	BKNI_Memset_isr(&hHDR->SPDInfoFrame, 0, sizeof(BAVC_HDMI_SPDInfoFrame));
	BKNI_Memset_isr(&hHDR->AudioContentProtection, 0, sizeof(BAVC_HDMI_ACP)) ;
	BKNI_Memset_isr(&hHDR->AudioClockRegenerationPacket,
		0,  sizeof(BAVC_HDMI_AudioClockRegenerationPacket)) ;
	BKNI_Memset_isr(&hHDR->VSInfoFrame, 0, sizeof(BAVC_HDMI_VendorSpecificInfoFrame));

#if BHDR_CONFIG_GAMUT_PACKET_SUPPORT
	BKNI_Memset_isr(&hHDR->GamutPacket, 0, sizeof(BAVC_HDMI_GamutPacket)) ;
#endif
#if BHDR_CONFIG_ISRC_PACKET_SUPPORT
	BKNI_Memset_isr(&hHDR->ISRC, 0, sizeof(BAVC_HDMI_ISRC)) ;
#endif

	BDBG_LEAVE(BHDR_P_ClearPacketMemory_isr) ;
}


/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_P_ClearPacketRAMLock_isr(
   BHDR_Handle hHDR,	uint8_t PacketNum
)
{

	BREG_Handle hRegister  ;
	uint32_t ulOffset  ;

	uint32_t rc = BERR_SUCCESS ;
	uint32_t Register ;
	uint32_t uiPacketLockedBit ;

	BDBG_ENTER(BHDR_P_ClearPacketRAMLock_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;

	uiPacketLockedBit = 1 << PacketNum ;

	/* Clear Locked Packet by writing 0 followed by 1 */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CONTROL + ulOffset) ;

	Register |= uiPacketLockedBit ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CONTROL + ulOffset, Register) ;

	Register &= ~uiPacketLockedBit ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CONTROL + ulOffset, Register) ;

	BDBG_LEAVE(BHDR_P_ClearPacketRAMLock_isr) ;
	return rc ;
}


/******************************************************************************
Summary:
*******************************************************************************/
static BERR_Code BHDR_P_CalculateInfoframeChecksum_isr(
	BAVC_HDMI_Packet *Packet, uint8_t *checksum)
{
	uint8_t i ;
	uint32_t temp = 0 ;
	uint32_t chksum ;

	temp = Packet->Type + Packet->Length + Packet->Version  ;

	for (i = 0 ; i < BAVC_HDMI_PB_LENGTH; i++)
		temp = temp + Packet->DataBytes[i]  ;

	chksum = temp % 256 ;

	*checksum = (uint8_t) chksum ;

	return BERR_SUCCESS ;
}


/******************************************************************************
Summary:
	Copy Packet Data
*******************************************************************************/
static BERR_Code BHDR_P_GetPacketData_isr(
	BHDR_Handle hHDR, uint8_t PacketNum, BAVC_HDMI_Packet *Packet
)
{
	BREG_Handle hRegister ;
	uint32_t ulOffset ;
	uint32_t rc = BERR_SUCCESS ;
	uint32_t Register ;
	uint32_t PacketRegisterOffset ;


	BDBG_ENTER(BHDR_GetPacketData_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister= hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;


	/* calculate the offset of where the Packet RAM begins */
	PacketRegisterOffset = BCHP_HDMI_RX_0_RAM_PACKET_0_HEADER + ulOffset
		+ BHDR_P_PACKET_WORDS * 4 * PacketNum ;

	/* read the Header Bytes (HBn) first */
	Register = BREG_Read32(hRegister, PacketRegisterOffset) ;
	Packet->Type    = (uint8_t)  (Register & 0x000000FF) ;
#if BHDR_CONFIG_GAMUT_PACKET_SUPPORT
	if (Packet->Type == BAVC_HDMI_PacketType_eGamutMetadataPacket)
	{
		hHDR->GamutPacket.Type = Packet->Type ;

		rc = BHDR_P_GetGamutPacketData_isr(
			hHDR, PacketNum, &hHDR->GamutPacket) ;
		goto done ;
	}
#endif

	Packet->Version = (uint8_t) ((Register & 0x0000FF00) >>  8) ;
	Packet->Length  = (uint8_t) ((Register & 0x00FF0000) >> 16) ;

	/* advance to the next packet register */
	PacketRegisterOffset +=  4 ;

	/* now read the Packet Bytes (PBn) */
	BHDR_P_GetPacketRAM_isr(hRegister, PacketRegisterOffset, (uint8_t *) Packet->DataBytes) ;

#if BHDR_CONFIG_GAMUT_PACKET_SUPPORT
done:
#endif
	BDBG_LEAVE(BHDR_P_GetPacketData_isr) ;
	return rc ;
}


/******************************************************************************
Summary:
	Copy Packet Data
*******************************************************************************/
BERR_Code BHDR_GetPacketData(
	BHDR_Handle hHDR, uint8_t PacketNum, BAVC_HDMI_Packet *Packet
)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDR_GetPacketData) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	BKNI_EnterCriticalSection() ;
		rc = BHDR_P_GetPacketData_isr(hHDR, PacketNum, Packet) ;
	BKNI_LeaveCriticalSection() ;

	return rc ;
}



/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_P_ProcessReceivedPackets_isr(BHDR_Handle hHDR)
{
	BREG_Handle hRegister  ;
	uint32_t ulOffset  ;
	uint32_t rc = BERR_SUCCESS ;
	uint32_t LockedPackets  ;
	uint32_t Register ;
#if BHDR_CONFIG_CLEAR_AVMUTE_AFTER_N_S
	uint8_t AvMute ;
#endif
	uint32_t i ;
	uint8_t checksum ;

	BDBG_ENTER(BHDR_P_ProcessReceivedPackets_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;

	LockedPackets = BREG_Read32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_STATUS + ulOffset) ;
	for (i = 0 ; i < BHDR_P_NUM_PACKETS; i++)
	{
		/* process only packets that are listed as active in the RAM Packet Map */
		if (RamPacketMap[i].ePacketType == BAVC_HDMI_PacketType_eUnused)
			continue ;

		if ((1 << i) & LockedPackets)
		{
			BHDR_P_GetPacketData_isr(hHDR, i, &hHDR->RamPacket) ;

#if BDBG_DEBUG_BUILD
			/* debug support to display received packets -- skip reporting GCP and Clock Recovery Packets; too many! */
			if ((hHDR->RamPacket.Type != BAVC_HDMI_PacketType_eAudioClockRegeneration)
			&& (hHDR->RamPacket.Type != BAVC_HDMI_PacketType_eGeneralControl)
			&& (hHDR->RamPacket.Type != BAVC_HDMI_PacketType_eNull))
			{
				BDBG_MSG(("RAM_PACKET_%02d received '%s' Packet (0x%02x)",
					i, BAVC_HDMI_PacketTypeToStr_isrsafe(hHDR->RamPacket.Type),
					hHDR->RamPacket.Type)) ;
			}
#endif

			if (hHDR->RamPacket.Type > BAVC_HDMI_INFOFRAME_PACKET_TYPE)
			{
				BHDR_P_CalculateInfoframeChecksum_isr(&hHDR->RamPacket, &checksum) ;
				if (checksum)  /* there's an error in the packet */
				{
					BDBG_WRN(("CH%d '%s' Packet (0x%02x) has checksum error",
						hHDR->eCoreId,
						BAVC_HDMI_PacketTypeToStr_isrsafe(hHDR->RamPacket.Type),
						hHDR->RamPacket.Type)) ;

					/*
					ClearLockedPacket may not be needed since the packet
					already contains an error; a clean error-free packet will
					overwrite once the lock is released

					in any case this is a rare occurence

					*/
					BHDR_P_ClearPacketRAMLock_isr(hHDR, i) ;

					continue ;
				}
			}

			switch (hHDR->RamPacket.Type)
			{
			case BAVC_HDMI_PacketType_eNull :
				/* do nothing */
				break ;

			case BAVC_HDMI_PacketType_eSpdInfoFrame    :

				BHDR_P_DEBUG_DumpPacketRam_isr(hHDR, i, &hHDR->RamPacket) ;
				BHDR_P_ParseSPDInfoFrameData_isr(hHDR, &hHDR->RamPacket) ;

				hHDR->SPDInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eUpdated ;

				if (hHDR->pfPacketChangeCallback)
				{
					hHDR->pfPacketChangeCallback(hHDR->pvPacketChangeParm1,
						hHDR->RamPacket.Type, &hHDR->SPDInfoFrame) ;
				}
				break ;

			case BAVC_HDMI_PacketType_eVendorSpecificInfoframe :

				BHDR_P_DEBUG_DumpPacketRam_isr(hHDR, i, &hHDR->RamPacket) ;
				rc = BHDR_P_ParseVendorSpecificInfoFrameData_isr(hHDR, &hHDR->RamPacket) ;
				if (rc) /* error in packet; do not callback with UNKNOWN VSI packet */
					break  ;

				hHDR->VSInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eUpdated ;

				if (hHDR->pfPacketChangeCallback)
				{
					hHDR->pfPacketChangeCallback(hHDR->pvPacketChangeParm1,
						hHDR->RamPacket.Type, &hHDR->VSInfoFrame) ;
				}
				break ;


			case BAVC_HDMI_PacketType_eAviInfoFrame :

				BHDR_P_DEBUG_DumpPacketRam_isr(hHDR, i, &hHDR->RamPacket) ;
				BHDR_P_ParseAviInfoFrameData_isr(hHDR, &hHDR->RamPacket) ;

				hHDR->AviInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eUpdated ;

				if (hHDR->pfPacketChangeCallback)
				{
					hHDR->pfPacketChangeCallback(hHDR->pvPacketChangeParm1,
						hHDR->RamPacket.Type, &hHDR->AviInfoFrame) ;
				}
				else
				{
					BDBG_WRN(("CH%d NO HDMI Packet Change Callback installed!!!",
						hHDR->eCoreId)) ;
				}

				break ;

			case BAVC_HDMI_PacketType_eAudioInfoFrame :

				BHDR_P_DEBUG_DumpPacketRam_isr(hHDR, i, &hHDR->RamPacket) ;
				BHDR_P_ParseAudioInfoFrameData_isr(hHDR, &hHDR->RamPacket) ;

				hHDR->AudioInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eUpdated ;

				if (hHDR->pfPacketChangeCallback)
				{
					hHDR->pfPacketChangeCallback(hHDR->pvPacketChangeParm1,
						hHDR->RamPacket.Type, &hHDR->AudioInfoFrame) ;
				}

				break ;

			case BAVC_HDMI_PacketType_eDrmInfoFrame :

				BHDR_P_DEBUG_DumpPacketRam_isr(hHDR, i, &hHDR->RamPacket) ;
				BHDR_P_ParseDrmInfoFrameData_isr(hHDR, &hHDR->RamPacket) ;

				hHDR->DRMInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eUpdated ;

				if (hHDR->pfPacketChangeCallback)
				{
					hHDR->pfPacketChangeCallback(hHDR->pvPacketChangeParm1,
						hHDR->RamPacket.Type, &hHDR->DRMInfoFrame) ;
				}

				break ;
			case BAVC_HDMI_PacketType_eGeneralControl :
				/* AvMute in the GCP is handled at Isr time */

#if BHDR_CONFIG_CLEAR_AVMUTE_AFTER_N_S
				/* when muted check for continued GCPs every 1s */
				if (hHDR->AvMute)
				{
					Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_MISC_STATUS + ulOffset) ;
					AvMute = (uint8_t) BCHP_GET_FIELD_DATA(Register,
						HDMI_RX_0_MISC_STATUS, PKT_PROCESSOR_AV_MUTE) ;

					/* GCP Packets with AvMute set are still coming; restart our timer */
					BTMR_ReadTimer_isr(hHDR->timerGeneric, &hHDR->AvMuteTimeStamp) ;

					BDBG_MSG(("GCPs with Set_AVMUTE are still active; Restart auto Clear_AVMUTE timer...")) ;
				}
#endif

				BHDR_P_ParseGeneralControlPacket_isr(hHDR, &hHDR->RamPacket) ;

				hHDR->GeneralControlPacket.ePacketStatus = BAVC_HDMI_PacketStatus_eUpdated ;
				break ;

			case BAVC_HDMI_PacketType_eAudioClockRegeneration :
				BHDR_P_ParseAudioClockRegeneration_isr(hHDR, &hHDR->RamPacket) ;

				hHDR->AudioClockRegenerationPacket.ePacketStatus = BAVC_HDMI_PacketStatus_eUpdated ;

				break ;

			case BAVC_HDMI_PacketType_eAudioContentProtection :
				BHDR_P_ParseAudioContentProtection_isr(hHDR, &hHDR->RamPacket) ;

				hHDR->AudioContentProtection.ePacketStatus = BAVC_HDMI_PacketStatus_eUpdated ;

				if (hHDR->pfPacketChangeCallback)
				{
					hHDR->pfPacketChangeCallback(hHDR->pvPacketChangeParm1,
						hHDR->RamPacket.Type, &hHDR->AudioContentProtection) ;
				}

				break ;

	 		case BAVC_HDMI_PacketType_eISRC1                :
			case BAVC_HDMI_PacketType_eISRC2                :
#if BHDR_CONFIG_ISRC_PACKET_SUPPORT
				if (hHDR->RamPacket.Type == BAVC_HDMI_PacketType_eISRC1)
					BHDR_P_ParseISRC1_isr(hHDR, &hHDR->RamPacket) ;
				else
					BHDR_P_ParseISRC2_isr(hHDR, &hHDR->RamPacket) ;


				/* callbacks for ISRC packet change are handled in the parse function */
				hHDR->ISRC.ePacketStatus = BAVC_HDMI_PacketStatus_eUpdated ;
#else
				BDBG_WRN(("****")) ;
				BDBG_WRN(("Support for ISRC packets is disabled; see bhdr_config.h to enable")) ;
				BDBG_WRN(("****")) ;
#endif
				break ;

			case BAVC_HDMI_PacketType_eGamutMetadataPacket :
#if BHDR_CONFIG_GAMUT_PACKET_SUPPORT
				hHDR->GamutPacket.ePacketStatus = BAVC_HDMI_PacketStatus_eUpdated ;

				hHDR->GamutPacket.Type = hHDR->RamPacket.Type ;
				BHDR_P_ParseGamutMetadataPacket_isr(hHDR, &hHDR->GamutPacket) ;

				if (hHDR->pfPacketChangeCallback)
				{
					hHDR->pfPacketChangeCallback(hHDR->pvPacketChangeParm1,
						hHDR->RamPacket.Type, &hHDR->GamutPacket) ;
				}

				break ;
#else
				/* fall through */
#endif

			case BAVC_HDMI_PacketType_eMpegInfoFrame   :
			default :
				BDBG_WRN(("   CH%d No handler for (%s) Packet (0x%02x)",
					hHDR->eCoreId,
					BAVC_HDMI_PacketTypeToStr_isrsafe(hHDR->RamPacket.Type),
					hHDR->RamPacket.Type)) ;
				break ;

			}

			/* enable packet stop detection  */
			if (RamPacketMap[i].bMonitorStoppedPackets)
			{
				BDBG_MSG(("RAM_PACKET_%02d enabled  '%s' Packet (0x%02x) Stop Detection",
					i, BAVC_HDMI_PacketTypeToStr_isrsafe(hHDR->RamPacket.Type),
					hHDR->RamPacket.Type)) ;

				/* clear the packet stop detect status */
				Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_PACKET_STOP_CLEAR_DETECT + ulOffset) ;
				Register |= (uint32_t) 1 << i ;       /* write 1 */
				BREG_Write32(hRegister, BCHP_HDMI_RX_0_PACKET_STOP_CLEAR_DETECT + ulOffset, Register) ;
				Register &= ~((uint32_t) 1 << i) ;  /* write 0 */
				BREG_Write32(hRegister, BCHP_HDMI_RX_0_PACKET_STOP_CLEAR_DETECT + ulOffset, Register) ;

				/* Enable Packet Stop Detection in case the packet stops */
				Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_ENABLE_PACKET_STOP_DETECT + ulOffset) ;
				Register |= (uint32_t) 1 << i ;
				BREG_Write32(hRegister, BCHP_HDMI_RX_0_ENABLE_PACKET_STOP_DETECT  + ulOffset, Register) ;
			}

			/* now that the packet is processed; clear its lock so we get the next change */
			BHDR_P_ClearPacketRAMLock_isr(hHDR, i) ;
		}
	}

	/* always return success */
	BDBG_LEAVE(BHDR_P_ProcessReceivedPackets_isr) ;
	return BERR_SUCCESS ;
}


/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_P_ProcessStoppedPackets_isr(BHDR_Handle hHDR)
{
	BREG_Handle hRegister  ;
	uint32_t ulOffset  ;
	uint32_t rc = BERR_SUCCESS ;
	uint32_t StoppedPackets  ;
	uint32_t PacketNum ;
	uint32_t Register ;

	int ePacketType ;
	void *pvPacketData ;


	BDBG_ENTER(BHDR_P_ProcessStoppedPackets_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;

	StoppedPackets = BREG_Read32(hRegister, BCHP_HDMI_RX_0_PACKET_STOP_DETECT_STATUS + ulOffset) ;
	for (PacketNum = 0 ; PacketNum < BHDR_P_NUM_PACKETS; PacketNum++)
	{
		/* check if monitoring was enabled on the this stopped packet */
		if (!RamPacketMap[PacketNum].bMonitorStoppedPackets)
			continue ;

		/* process only packets that are listed as active in the RAM Packet Map */
		if ((RamPacketMap[PacketNum].ePacketType == BAVC_HDMI_PacketType_eUnused)
		|| (RamPacketMap[PacketNum].ePacketType == BAVC_HDMI_PacketType_eNull))
			continue ;

		/* skip packets which are still coming */
		if (!( ( (uint32_t) 1 << PacketNum) & StoppedPackets))
			continue ;

		ePacketType = (int) RamPacketMap[PacketNum].ePacketType ;
		BDBG_WRN(("CH%d RAM_PACKET_%02d has STOPPED receiving '%s' Packets (0x%02x)",
			hHDR->eCoreId,
			PacketNum, BAVC_HDMI_PacketTypeToStr_isrsafe(ePacketType), ePacketType)) ;

		pvPacketData = NULL ;
		/* handle stopped packets appropriately */
		switch (ePacketType)
		{
		case BAVC_HDMI_PacketType_eAviInfoFrame :
			hHDR->AviInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eStopped ;
			pvPacketData = &hHDR->AviInfoFrame;

			/* call  the callback functions for Format Change notification  */
			if (hHDR->pfVideoFormatChangeCallback)
			{
				hHDR->pfVideoFormatChangeCallback(hHDR->pvVideoFormatChangeParm1,
					BAVC_HDMI_PacketType_eAviInfoFrame, &hHDR->AviInfoFrame) ;
			}
			break ;

		case BAVC_HDMI_PacketType_eAudioInfoFrame :
			hHDR->AudioInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eStopped ;
			pvPacketData = &hHDR->AudioInfoFrame ;
			break ;

		case BAVC_HDMI_PacketType_eSpdInfoFrame       :
			hHDR->SPDInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eStopped ;
			pvPacketData = &hHDR->SPDInfoFrame ;
			break ;

		case BAVC_HDMI_PacketType_eVendorSpecificInfoframe:
			hHDR->VSInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eStopped ;
			hHDR->VSInfoFrame.eHdmiVideoFormat = BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eNone ;
			pvPacketData = &hHDR->VSInfoFrame ;
			break ;

		case BAVC_HDMI_PacketType_eDrmInfoFrame:
			hHDR->DRMInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eStopped ;
			pvPacketData = &hHDR->DRMInfoFrame ;
			break ;

		/* loss of audio ??? */
		case BAVC_HDMI_PacketType_eAudioClockRegeneration :
			hHDR->AudioClockRegenerationPacket.ePacketStatus = BAVC_HDMI_PacketStatus_eStopped ;
			pvPacketData = &hHDR->AudioClockRegenerationPacket ;
			break ;


		/* Audio Not Protected */
		case BAVC_HDMI_PacketType_eAudioContentProtection :
			hHDR->AudioContentProtection.ePacketStatus = BAVC_HDMI_PacketStatus_eStopped ;
			pvPacketData = &hHDR->AudioContentProtection ;
			break ;


		case BAVC_HDMI_PacketType_eAudioSample          :
			/* Audio packets handled by hw */
			continue ;
			break ;

		case BAVC_HDMI_PacketType_eGeneralControl       :
			hHDR->GeneralControlPacket.ePacketStatus = BAVC_HDMI_PacketStatus_eStopped ;
			pvPacketData = &hHDR->GeneralControlPacket ;
			BDBG_WRN(("CH%d General Control Packet stopped...  ", hHDR->eCoreId)) ;

			/* reset the DC count threshold to zero */
			Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDMI_13_FEATURES_CFG_1 + ulOffset) ;
			Register &= ~BCHP_MASK(HDMI_RX_0_HDMI_13_FEATURES_CFG_1, GCP_CD_FRAME_COUNT_THRESHOLD) ;
			BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDMI_13_FEATURES_CFG_1 + ulOffset,  Register) ;

			break ;

		case BAVC_HDMI_PacketType_eISRC1                :
		case BAVC_HDMI_PacketType_eISRC2                :
#if BHDR_CONFIG_ISRC_PACKET_SUPPORT
			hHDR->ISRC.ePacketStatus = BAVC_HDMI_PacketStatus_eStopped ;

			/* reset ISRC1_PacketReceived; it is no longer valid */
			hHDR->ISRC.ISRC1_PacketReceived = false ;
			/* call  the ISRC callback functions for indicate packet has stopped */
			if (hHDR->pfISRCChangeCallback)
			{
				hHDR->pfISRCChangeCallback(hHDR->pvISRCChangeParm1,
					BAVC_HDMI_PacketType_eISRC1, &hHDR->ISRC) ;
			}
#endif
			break ;


		case BAVC_HDMI_PacketType_eDirectStream      :
		case BAVC_HDMI_PacketType_eGamutMetadataPacket   :
		case BAVC_HDMI_PacketType_eMpegInfoFrame   :
			BDBG_WRN(("CH%d HDMI '%s' Packet (%#02x) is not supported",
				hHDR->eCoreId,
				BAVC_HDMI_PacketTypeToStr_isrsafe(ePacketType), ePacketType)) ;
			continue ;
			break ;

		default :
			BDBG_WRN(("CH%d Unknown Packet Type %#02x Stopped",
				hHDR->eCoreId, ePacketType)) ;
			continue ;
		}

		if (hHDR->pfPacketChangeCallback && pvPacketData)
		{
			hHDR->pfPacketChangeCallback(hHDR->pvPacketChangeParm1,
				ePacketType, pvPacketData) ;
		}

		/* clear Packet RAM in case the same packet starts transmitting again */
		/* we will then be notified of the resumption of the packet */

		BHDR_P_ClearPacketRAM_isr(hHDR, PacketNum) ;
		BHDR_P_ClearPacketRAMLock_isr(hHDR, PacketNum) ;

	}

	BDBG_LEAVE(BHDR_P_ProcessStoppedPackets_isr) ;
	return rc ;
}



/******************************************************************************
Summary:
Parse General Control Packet (GCP) data from received packet
*******************************************************************************/
BERR_Code BHDR_P_ParseGeneralControlPacket_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet)
{
	uint8_t temp ;
	BAVC_HDMI_GcpData stNewGcpPacket ;

	BDBG_ENTER(BHDR_P_ParseGeneralControlPacket_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	BKNI_Memset_isr(&stNewGcpPacket, 0, sizeof(BAVC_HDMI_GcpData)) ;

	/* Keep a raw copy of the HDMI Packet structure  */
	BKNI_Memcpy_isr(&stNewGcpPacket.stPacket, Packet, sizeof(BAVC_HDMI_Packet)) ;


	/* check for SetAvMute */
	temp = Packet->DataBytes[0] ;

#if BHDR_CONFIG_DEBUG_GCP_DC || BHDR_CONFIG_DEBUG_GCP_AV_MUTE
	BDBG_LOG(("-------------------- START PARSE GCP PACKET----------------------")) ;
#endif

	if ((temp) && (temp != hHDR->GeneralControlPacket.uiMuteStatus))  /* AvMute Change */
	{
		stNewGcpPacket.uiMuteStatus = (uint32_t) temp ;

		/* check for SetAvMute */
		temp = Packet->DataBytes[0] & 0x01 ;  /* 0x01 = SetAvMute  */
		stNewGcpPacket.SetAvMute = (uint32_t) temp ;

		/* check for ClearAvMute */
		temp = Packet->DataBytes[0] & 0x10 ; /* 0x10 = ClearAvMute */
		stNewGcpPacket.ClearAvMute = (uint32_t) temp ;

#if BHDR_CONFIG_DEBUG_GCP_AV_MUTE
		/* Either Clear_AvMute or Set_AvMute should be set but not both */
		if (stNewGcpPacket.SetAvMute)
		{
			BDBG_WRN(("CH%d Set_AVMUTE (%#02x) is set; PB0: %#02x",
				hHDR->eCoreId, stNewGcpPacket.SetAvMute, Packet->DataBytes[0])) ;
		}

		if (stNewGcpPacket.ClearAvMute)
		{
			BDBG_WRN(("CH%d Clear_AVMUTE (%#02x) is set; PB0: %#02x",
				hHDR->eCoreId, stNewGcpPacket.ClearAvMute, Packet->DataBytes[0])) ;
		}
#endif
	}
	else if (!temp)  /* no AvMute information */
	{
#if BHDR_CONFIG_DEBUG_GCP_AV_MUTE
		BDBG_WRN(("CH%d No AvMute info in current GCP Packet; PB0: %#02x",
				hHDR->eCoreId, Packet->DataBytes[0])) ;
#endif
		/* no mute information in current GCP packet; keep current setting*/
		stNewGcpPacket.uiMuteStatus = hHDR->GeneralControlPacket.uiMuteStatus ;
	}

	/* check for color depth info */
	temp = Packet->DataBytes[1] ;
	temp = temp & 0x0F ;

	/* Ignore GCP Packets that do not contain Color Depth Info  */
	/* hardware handles Mute/Unmute data  */

	stNewGcpPacket.eColorDepth = (uint32_t) temp ;


	temp = Packet->DataBytes[1] ;
	temp = temp & 0xF0 ;
	temp = temp >> 4 ;
	stNewGcpPacket.ePixelPacking = (uint32_t) temp ;

	temp = Packet->DataBytes[2] ;
	temp = temp & 0x01 ;
	stNewGcpPacket.DefaultPhase = (uint32_t) temp ;

	/* Ignore GCP if there is no difference in the Color Depth */
	if ((hHDR->bDeepColorMode) && (!stNewGcpPacket.eColorDepth))  /* packet with no DC info; keep current DC */
	{
#if BHDR_CONFIG_DEBUG_GCP_DC
		/* informative message indicating GCP with no DC message;  which is legal */
		BDBG_LOG(("In %s DC Mode; Received GCP with no DC information",
			BAVC_HDMI_BitsPerPixelToStr(hHDR->GeneralControlPacket.eColorDepth))) ;
#endif
		stNewGcpPacket.eColorDepth = hHDR->GeneralControlPacket.eColorDepth ;
		goto done ;
	}

	if ((!hHDR->bDeepColorMode)
	&& (   (stNewGcpPacket.eColorDepth == BAVC_HDMI_GCP_ColorDepth_e24bpp)
		|| (stNewGcpPacket.eColorDepth == BAVC_HDMI_GCP_ColorDepth_eDepthNotIndicated)))
	{
		stNewGcpPacket.eColorDepth = (uint32_t) BAVC_HDMI_GCP_ColorDepth_e24bpp ;
	}

	if (hHDR->GeneralControlPacket.eColorDepth == stNewGcpPacket.eColorDepth)
		goto done ; /* no color depth change */


	switch (stNewGcpPacket.eColorDepth)
	{
	case BAVC_HDMI_GCP_ColorDepth_eDepthNotIndicated :
	case BAVC_HDMI_GCP_ColorDepth_e24bpp :
		BDBG_MSG(("CH%d Color Depth: 24 bpp", hHDR->eCoreId)) ;
		hHDR->bDeepColorMode = false ;
		/* Pre HDMI 1.3 GCP; no Deep Color Info specified (default 24Bit) ...*/
		goto done ;
		break ;

	case BAVC_HDMI_GCP_ColorDepth_e30bpp :
		BDBG_MSG(("CH%d Color Depth: 30 bpp", hHDR->eCoreId)) ;
		hHDR->bDeepColorMode = true ;
		break ;

	case BAVC_HDMI_GCP_ColorDepth_e36bpp :
		BDBG_MSG(("CH%d Color Depth: 36 bpp", hHDR->eCoreId)) ;
		hHDR->bDeepColorMode = true ;
		break ;

	case BAVC_HDMI_GCP_ColorDepth_e48bpp :
		BDBG_MSG(("CH%d Color Depth: 48 bpp", hHDR->eCoreId)) ;
		hHDR->bDeepColorMode = true ;
		break ;

	default :
		BDBG_MSG(("CH%d Incorrectly Specified Color Depth: %#x",
			hHDR->eCoreId, hHDR->GeneralControlPacket.eColorDepth)) ;
	}


#if BHDR_CONFIG_DEBUG_GCP_DC
	BDBG_LOG(("CH%d Pixel Packing Phase: %#x", hHDR->eCoreId, hHDR->GeneralControlPacket.ePixelPacking)) ;
	switch (hHDR->GeneralControlPacket.ePixelPacking)
	{

	case BAVC_HDMI_GCP_PixelPacking_ePhase4_10P4 :
		BDBG_LOG(("\tCH%d Phase4 Pixel Packing (10P4)", hHDR->eCoreId)) ;
		break ;

	case BAVC_HDMI_GCP_PixelPacking_ePhase1_10P1_12P1_16P1 :
		BDBG_LOG(("\tCH%d Phase1 Pixel Packing (10P1 12P1 16P1)", hHDR->eCoreId)) ;
		break ;

	case BAVC_HDMI_GCP_PixelPacking_ePhase2_10P2_12P2 :
		BDBG_LOG(("\tCH%d Phase2 Pixel Packing (10P2 12P2)", hHDR->eCoreId)) ;
		break ;

	case BAVC_HDMI_GCP_PixelPacking_ePhase3_10P3 :
		BDBG_LOG(("\tCH%d Phase3 Pixel Packing (10P2)", hHDR->eCoreId)) ;
		break ;

	default :
		BDBG_WRN(("CH%d Incorrectly Specified Pixel Packing Phase %#x",
			hHDR->eCoreId, hHDR->GeneralControlPacket.ePixelPacking)) ;
	}
#endif

done :
#if BHDR_CONFIG_DEBUG_GCP_DC || BHDR_CONFIG_DEBUG_GCP_AV_MUTE
	BDBG_LOG(("--------------------  END  PARSE GCP PACKET----------------------")) ;
#endif

	/* copy the new packet to the handle for use later */
	BKNI_Memcpy_isr(&hHDR->GeneralControlPacket, &stNewGcpPacket, sizeof(BAVC_HDMI_GcpData)) ;

	BDBG_LEAVE(BHDR_P_ParseGeneralControlPacket_isr) ;

	return BERR_SUCCESS ;
}
