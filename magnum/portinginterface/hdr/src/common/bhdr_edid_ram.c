/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#if BDBG_DEBUG_BUILD
#if defined LINUX && !defined __KERNEL__
#include "ctype.h"
#endif
#endif

#include "bchp_aon_hdmi_rx.h"

#include "bhdr_fe.h"
#include "bhdr_fe_priv.h"

#include "bhdr.h"
#include "bhdr_priv.h"


BDBG_MODULE(BHDR_EDID_RAM) ;

#define BHDR_DEFAULT_EDID_BLOCKS 2
#define BHDR_DEFAULT_EDID_BLOCKSIZE 128

#define BHDR_DEFAULT_EDID_SIZE (BHDR_DEFAULT_EDID_BLOCKSIZE * BHDR_DEFAULT_EDID_BLOCKS)

static const uint8_t BHDR_DefaultRxEdid[BHDR_DEFAULT_EDID_SIZE] =
{
#if BAVC_HDMI_20_SUPPORT

/* HDMI 2.0 EDID e.g. 2160p 50/60 Support */

	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x08, 0x6D, 0x45, 0x74, 0x67, 0x45, 0x23, 0x01,
	0x18, 0x18, 0x01, 0x03, 0x80, 0x80, 0x48, 0x78, 0x0A, 0xDA, 0xFF, 0xA3, 0x58, 0x4A, 0xA2, 0x29,
	0x17, 0x49, 0x4B, 0x20, 0x00, 0x00, 0x31, 0x40, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x08, 0xE8, 0x00, 0x30, 0xF2, 0x70, 0x5A, 0x80, 0xB0, 0x58,
	0x8A, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x1E, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40,
	0x58, 0x2C, 0x45, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x42,
	0x72, 0x6F, 0x61, 0x64, 0x63, 0x6F, 0x6D, 0x20, 0x32, 0x2E, 0x30, 0x0A, 0x00, 0x00, 0x00, 0xFD,
	0x00, 0x17, 0x3D, 0x0F, 0x88, 0x3C, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x68,

	0x02, 0x03, 0x3D, 0xF0, 0x4D, 0x10, 0x05, 0x20, 0x22, 0x04, 0x03, 0x02, 0x07, 0x06, 0x61, 0x5D,
	0x5F, 0x60, 0x23, 0x09, 0x07, 0x01, 0x77, 0x03, 0x0C, 0x00, 0x10, 0x00, 0xB8, 0x3C, 0x21, 0xC8,
	0x6A, 0x01, 0x03, 0x04, 0x81, 0x41, 0x00, 0x16, 0x06, 0x08, 0x00, 0x56, 0x58, 0x00, 0x67, 0xD8,
	0x5D, 0xC4, 0x01, 0x78, 0x88, 0x03, 0xE2, 0x00, 0x4B, 0xE3, 0x0F, 0x00, 0x12, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55

#else

    /* HDMI 1.x EDID e.g. 1080p Support */
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x08, 0x6D, 0x25, 0x74, 0x67, 0x45, 0x23, 0x01,
	0x18, 0x18, 0x01, 0x03, 0x80, 0x80, 0x48, 0x78, 0x0A, 0xDA, 0xFF, 0xA3, 0x58, 0x4A, 0xA2, 0x29,
	0x17, 0x49, 0x4B, 0x20, 0x00, 0x00, 0x31, 0x40, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C,
	0x45, 0x00, 0x20, 0xC2, 0x31, 0x00, 0x00, 0x1E, 0x04, 0x74, 0x00, 0x30, 0xF2, 0x70, 0x5A, 0x80,
	0xB0, 0x58, 0x8A, 0x00, 0x20, 0xC2, 0x31, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x42,
	0x72, 0x6F, 0x61, 0x64, 0x63, 0x6F, 0x6D, 0x20, 0x31, 0x2E, 0x30, 0x0A, 0x00, 0x00, 0x00, 0xFD,
	0x00, 0x17, 0x3D, 0x0F, 0x88, 0x1E, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0xBF,

	0x02, 0x03, 0x2F, 0xF0, 0x4B, 0x10, 0x05, 0x20, 0x22, 0x04, 0x03, 0x02, 0x07, 0x06, 0x5E, 0x5F,
	0x23, 0x09, 0x07, 0x01, 0x77, 0x03, 0x0C, 0x00, 0x10, 0x00, 0xB8, 0x3C, 0x21, 0xC8, 0x6A, 0x01,
	0x03, 0x04, 0x81, 0x41, 0x00, 0x16, 0x06, 0x08, 0x00, 0x56, 0x58, 0x00, 0xE2, 0x00, 0x4B, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8D

 #endif
} ;

/******************************************************************************
Summary:
	Loads the on-chip EDID RAM for the specified HDR module with 128*Blocks bytes
	stored in the EdidRam array.

	In EDID, two 128 byte Blocks comprise a single segment.  The Broadcom EDIDs supports up
	to  8 Blocks or equivalently, 4 segments.

*******************************************************************************/
BERR_Code BHDR_EDID_RAM_LoadData(
	BHDR_Handle hHDR, const BHDR_EDID_Info *edid)
{
	BERR_Code   rc = BERR_SUCCESS;
	BREG_Handle hRegister ;
	uint32_t ulOffset  ;
	uint32_t Register  ;

	uint32_t i, k	;
	uint8_t block ;
	uint8_t uiNumBlocks ;
	uint32_t checksum;

	BHDR_EDID_Info rxEdid ;

	BDBG_ENTER(BHDR_EDID_RAM_LoadData) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	BKNI_Memset(&rxEdid, 0, sizeof(BHDR_EDID_Info)) ;

	if (!edid->pDataBytes)
	{
		BDBG_LOG(("HDMI EDID bytes not specified; HDMI Rx will use its default EDID")) ;

		rxEdid.pDataBytes = BKNI_Malloc(BHDR_DEFAULT_EDID_SIZE) ;
		BKNI_Memcpy(rxEdid.pDataBytes, BHDR_DefaultRxEdid, BHDR_DEFAULT_EDID_SIZE) ;

		rxEdid.uiNumBytes = BHDR_DEFAULT_EDID_SIZE ;
		uiNumBlocks = BHDR_DEFAULT_EDID_BLOCKS ;
	}
	else
	{
		rxEdid.pDataBytes = edid->pDataBytes ;
		rxEdid.uiNumBytes = edid->uiNumBytes ;
		uiNumBlocks = edid->uiNumBytes / 128 ;
	}

	if (uiNumBlocks < 2)
	{
		BDBG_WRN(("HDMI (CEA 861) EDID requires at least 2 Blocks.  Only '%d' Blocks were specified",
			uiNumBlocks)) ;
	}

	if (uiNumBlocks > 8)
	{
		BDBG_ERR(("HDMI (CEA 861) EDID allows up to 8 Blocks.  %d Blocks were specified.",
			uiNumBlocks )) ;
		rc = BERR_INVALID_PARAMETER ;
		goto BHDR_EDID_RAM_Load_Done ;
	}


	i = 0;

	/* Disable I2c access to the  EDID RAM device */
	Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_CONTROL + ulOffset) ;
	Register &= ~ BCHP_MASK(AON_HDMI_RX_EDID_CONTROL, ENABLE_EDID_DEVICE) ;
	Register |= BCHP_FIELD_DATA(AON_HDMI_RX_EDID_CONTROL, ENABLE_EDID_DEVICE, 0) ;
	BREG_Write32(hRegister, BCHP_AON_HDMI_RX_EDID_CONTROL + ulOffset, Register) ;


	/* enable RBUS access of i2c registers */
	Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_RAM_CONTROL) ;
	Register &= ~(
		  BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, RBUS_ACCESS_ENABLE)
		| BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_WRITE_ENABLE)
		| BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_READ_ENABLE)
		| BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_ACCESS_ADDRESS)
		| BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_WRITE_DATA)) ;

	Register |=
		  BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, RBUS_ACCESS_ENABLE, 0x1)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_WRITE_ENABLE, 0x0)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_READ_ENABLE, 0x0)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_ACCESS_ADDRESS, 0x0)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_WRITE_DATA, 0x0) ;
	BREG_Write32(hRegister, BCHP_AON_HDMI_RX_EDID_RAM_CONTROL, Register) ;



#if BHDR_CONFIG_DEBUG_EDID_RAM
	BDBG_MSG(("Start loading %d EDID Blocks...", uiNumBlocks)) ;
#endif
	for (block = 0 ; block < uiNumBlocks ; block++)
	{
		checksum = 0;
		for (k = 0; k < BHDR_DEFAULT_EDID_BLOCKSIZE; k++, i++)
		{
			/* Mask off the address and data*/
			Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_RAM_CONTROL + ulOffset) ;
			Register &= ~ BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_ACCESS_ADDRESS) ;
			Register &= ~ BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_WRITE_DATA) ;

			/* Write the address and data*/
			Register |= BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_ACCESS_ADDRESS,
				i) ;
			Register |= BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_WRITE_DATA,
				rxEdid.pDataBytes[i]) ;
			BREG_Write32(hRegister, BCHP_AON_HDMI_RX_EDID_RAM_CONTROL + ulOffset, Register) ;

#if BHDR_CONFIG_DEBUG_EDID_RAM
			BDBG_MSG(("%08x Writing %02x to offset %03d", Register,  rxEdid.pDataBytes[i], i)) ;
#endif

			/* Load the data into the RAM */
			Register |= BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_WRITE_ENABLE, 1) ;
			BREG_Write32(hRegister, BCHP_AON_HDMI_RX_EDID_RAM_CONTROL + ulOffset, Register) ;

 			Register &= ~ BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_WRITE_ENABLE) ;
			BREG_Write32(hRegister, BCHP_AON_HDMI_RX_EDID_RAM_CONTROL + ulOffset, Register) ;


			checksum += rxEdid.pDataBytes[i] ;
		}

		if (checksum & 0xFF)
		{
			/* report checksum error and continue */
			BDBG_ERR(("Block %d checksum %#02X is INVALID; Computed %#02X",
				block, rxEdid.pDataBytes[i-1],
				((256 - (checksum - rxEdid.pDataBytes[i-1]) ) & 0xFF)));
		}
		else
		{
			BDBG_MSG(("Block %d checksum %#x", block, rxEdid.pDataBytes[i-1])) ;
		}
	}

	BDBG_MSG(("Finished loading %d EDID Blocks", block));


#if BHDR_CONFIG_DEBUG_EDID_RAM
	{
		uint32_t EdidByte ;

		BDBG_WRN(("Verifying EDID Data."));

		/* enable RBUS access of i2c registers */
		Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_RAM_CONTROL) ;
		Register &= ~(
			  BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, RBUS_ACCESS_ENABLE)
			| BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_WRITE_ENABLE)
			| BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_READ_ENABLE)
			| BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_ACCESS_ADDRESS)
			| BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_WRITE_DATA)) ;

		Register |=
			  BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, RBUS_ACCESS_ENABLE, 0x1)
			| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_WRITE_ENABLE, 0x0)
			| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_READ_ENABLE, 0x0)
			| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_ACCESS_ADDRESS, 0x0)
			| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_WRITE_DATA, 0x0) ;
		BREG_Write32(hRegister, BCHP_AON_HDMI_RX_EDID_RAM_CONTROL, Register) ;



		i = 0 ;
		for (block = 0 ; block < uiNumBlocks ; block++)
		{
			for (k = 0 ; k < BHDR_EDID_BLOCKSIZE ; k++, i++)
			{
				/* Specify the address to be read */
				Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_RAM_CONTROL + ulOffset) ;
				Register &= ~ BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_ACCESS_ADDRESS) ;
				Register |=   BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_ACCESS_ADDRESS, i) ;
				BREG_Write32(hRegister, BCHP_AON_HDMI_RX_EDID_RAM_CONTROL + ulOffset, Register) ;

				/* Enable the EDID for read */
				Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_RAM_CONTROL + ulOffset) ;
				Register |= BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_READ_ENABLE, 1) ;
				BREG_Write32(hRegister, BCHP_AON_HDMI_RX_EDID_RAM_CONTROL + ulOffset, Register) ;


				/* Read the data */
				Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_STATUS + ulOffset) ;
				EdidByte = BCHP_GET_FIELD_DATA(Register, AON_HDMI_RX_EDID_STATUS, EDID_READ_DATA) ;

#if 0
				BDBG_WRN(("Read offset %3d: %3d (%02x)", i, EdidByte, EdidByte));
#endif
				/* Check the data */
				if (EdidByte != rxEdid.pDataBytes[i])
				{
					if (rc == BERR_SUCCESS)
					{
						rc = BERR_UNKNOWN;
					}
					BDBG_ERR(("RAM Byte %03d Block %d Byte %03d write of %02x failed;  Read back %02x",
			                  i, block, k, rxEdid.pDataBytes[i],  EdidByte)) ;
				}
			}
		}
	}
#endif

BHDR_EDID_RAM_Load_Done:

	if (!edid->pDataBytes)
	{
		BKNI_Free(rxEdid.pDataBytes) ;
	}

	/* disable RBUS access of i2c registers */
	Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_RAM_CONTROL) ;
	Register &= ~(
		  BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, RBUS_ACCESS_ENABLE)
		| BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_ACCESS_ADDRESS)
		| BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_WRITE_DATA)) ;

	Register |=
		  BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, RBUS_ACCESS_ENABLE, 0x0)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_ACCESS_ADDRESS, 0x0)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_WRITE_DATA, 0x0) ;
	BREG_Write32(hRegister, BCHP_AON_HDMI_RX_EDID_RAM_CONTROL, Register) ;


	/* Enable I2c access to the  EDID RAM device */
	Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_CONTROL + ulOffset) ;
	Register &= ~ BCHP_MASK(AON_HDMI_RX_EDID_CONTROL, ENABLE_EDID_DEVICE) ;
	Register |= BCHP_FIELD_DATA(AON_HDMI_RX_EDID_CONTROL, ENABLE_EDID_DEVICE, 1) ;
	BREG_Write32(hRegister, BCHP_AON_HDMI_RX_EDID_CONTROL + ulOffset, Register) ;

	BDBG_LEAVE(BHDR_EDID_RAM_LoadData) ;

	return rc ;
}


void BHDR_EDID_RAM_Disable(BHDR_Handle hHDR)
{
	BREG_Handle hRegister ;
	uint32_t ulOffset  ;
	uint32_t Register  ;

	BDBG_ENTER(BHDR_EDID_RAM_Disable) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	/* Disable EDID RAM device */
	Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_CONTROL + ulOffset) ;
	Register &= ~ BCHP_MASK(AON_HDMI_RX_EDID_CONTROL, ENABLE_EDID_DEVICE) ;
	BREG_Write32(hRegister, BCHP_AON_HDMI_RX_EDID_CONTROL + ulOffset, Register) ;

	BDBG_LEAVE(BHDR_EDID_RAM_Disable) ;

}


void BHDR_EDID_RAM_Enable(BHDR_Handle hHDR)
{
	BREG_Handle hRegister ;
	uint32_t ulOffset  ;
	uint32_t Register  ;

	BDBG_ENTER(BHDR_EDID_RAM_Enable) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

 	/* Disable I2c access to the  EDID RAM device */
	Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_CONTROL + ulOffset) ;
	Register &= ~ BCHP_MASK(AON_HDMI_RX_EDID_CONTROL, ENABLE_EDID_DEVICE) ;
	Register |= BCHP_FIELD_DATA(AON_HDMI_RX_EDID_CONTROL, ENABLE_EDID_DEVICE, 0) ;
	BREG_Write32(hRegister, BCHP_AON_HDMI_RX_EDID_CONTROL + ulOffset, Register) ;



	Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_I2C_DELAY_CFG) ;
	Register &= ~ BCHP_MASK(AON_HDMI_RX_EDID_I2C_DELAY_CFG, I2C_WRITE_DELAY) ;
	Register &= ~ BCHP_MASK(AON_HDMI_RX_EDID_I2C_DELAY_CFG, I2C_HOLD_DELAY) ;
	Register |=
		  BCHP_FIELD_DATA(AON_HDMI_RX_EDID_I2C_DELAY_CFG, I2C_WRITE_DELAY, 0xF)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_I2C_DELAY_CFG, I2C_HOLD_DELAY, 0x1C) ;
	BREG_Write32(hRegister,BCHP_AON_HDMI_RX_EDID_I2C_DELAY_CFG, Register) ;


	Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_CONTROL) ;
	Register &= ~ (
		  BCHP_MASK(AON_HDMI_RX_EDID_CONTROL, EDID_I2C_ADDR)
		| BCHP_MASK(AON_HDMI_RX_EDID_CONTROL, EDID_OUTRANGE_DATA)
		| BCHP_MASK(AON_HDMI_RX_EDID_CONTROL, EDID_BLOCK_COUNT)
		| BCHP_MASK(AON_HDMI_RX_EDID_CONTROL, EDID_ENABLE_INC_BB)
		| BCHP_MASK(AON_HDMI_RX_EDID_CONTROL, EDID_ENABLE_INC_SB)
		| BCHP_MASK(AON_HDMI_RX_EDID_CONTROL, FORCE_ACK)) ;
	Register |=
		  BCHP_FIELD_DATA(AON_HDMI_RX_EDID_CONTROL, EDID_I2C_ADDR, 0x50)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_CONTROL, EDID_OUTRANGE_DATA, 0xEE)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_CONTROL, EDID_BLOCK_COUNT, 0x2)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_CONTROL, EDID_ENABLE_INC_BB, 0x1)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_CONTROL, EDID_ENABLE_INC_SB, 0x1)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_CONTROL, FORCE_ACK, 0) ;
	BREG_Write32(hRegister, BCHP_AON_HDMI_RX_EDID_CONTROL, Register) ;

	Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_SEGMENT_POINTER) ;
	Register &= ~ (
		  BCHP_MASK(AON_HDMI_RX_EDID_SEGMENT_POINTER, SP_I2C_ADDR)
		| BCHP_MASK(AON_HDMI_RX_EDID_SEGMENT_POINTER, ENABLE_SP_DEVICE)
		| BCHP_MASK(AON_HDMI_RX_EDID_SEGMENT_POINTER, SP_MAX_RANGE)) ;
	Register |=
		  BCHP_FIELD_DATA(AON_HDMI_RX_EDID_SEGMENT_POINTER, SP_I2C_ADDR, 0x30)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_SEGMENT_POINTER, ENABLE_SP_DEVICE, 0x1)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_SEGMENT_POINTER, SP_MAX_RANGE, 0x3) ;
	BREG_Write32(hRegister, BCHP_AON_HDMI_RX_EDID_SEGMENT_POINTER, Register) ;


	Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_CONFIG) ;
	Register &= ~ (
		  BCHP_MASK(AON_HDMI_RX_EDID_CONFIG, CR_I2C_ADDR)
		| BCHP_MASK(AON_HDMI_RX_EDID_CONFIG, ENABLE_CR_DEVICE)
		| BCHP_MASK(AON_HDMI_RX_EDID_CONFIG, EDID_CONFIGURATION)
		| BCHP_MASK(AON_HDMI_RX_EDID_CONFIG, ENABLE_MASTER_WRITE_EDID)) ;
	Register |=
		  BCHP_FIELD_DATA(AON_HDMI_RX_EDID_CONFIG, CR_I2C_ADDR, 0x31)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_CONFIG, ENABLE_CR_DEVICE, 0x1)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_CONFIG, EDID_CONFIGURATION, 0x1)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_CONFIG, ENABLE_MASTER_WRITE_EDID, 0x0) ;
	BREG_Write32(hRegister, BCHP_AON_HDMI_RX_EDID_CONFIG, Register) ;



	Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_DISPLAYID_CONTROL) ;
	Register &= ~ (
		  BCHP_MASK(AON_HDMI_RX_DISPLAYID_CONTROL, DISPLAYID_I2C_ADDR)
		| BCHP_MASK(AON_HDMI_RX_DISPLAYID_CONTROL, ENABLE_DISPLAYID_DEVICE)
		| BCHP_MASK(AON_HDMI_RX_DISPLAYID_CONTROL, DISPLAYID_OUTRANGE_DATA)
		| BCHP_MASK(AON_HDMI_RX_DISPLAYID_CONTROL, DISPLAYID_START_BLOCK)
		| BCHP_MASK(AON_HDMI_RX_DISPLAYID_CONTROL, DISPLAYID_BLOCK_COUNT)
		| BCHP_MASK(AON_HDMI_RX_DISPLAYID_CONTROL, DISPLAYID_ENABLE_INC_BB)
		| BCHP_MASK(AON_HDMI_RX_DISPLAYID_CONTROL, DISPLAYID_ENABLE_INC_SB)) ;
	Register |=
		  BCHP_FIELD_DATA(AON_HDMI_RX_DISPLAYID_CONTROL, DISPLAYID_I2C_ADDR, 0x32)
		| BCHP_FIELD_DATA(AON_HDMI_RX_DISPLAYID_CONTROL, ENABLE_DISPLAYID_DEVICE, 0x1)
		| BCHP_FIELD_DATA(AON_HDMI_RX_DISPLAYID_CONTROL, DISPLAYID_OUTRANGE_DATA, 0xCC)
		| BCHP_FIELD_DATA(AON_HDMI_RX_DISPLAYID_CONTROL, DISPLAYID_START_BLOCK, 0x6)
		| BCHP_FIELD_DATA(AON_HDMI_RX_DISPLAYID_CONTROL, DISPLAYID_BLOCK_COUNT, 0x4)
		| BCHP_FIELD_DATA(AON_HDMI_RX_DISPLAYID_CONTROL, DISPLAYID_ENABLE_INC_BB, 0x1)
		| BCHP_FIELD_DATA(AON_HDMI_RX_DISPLAYID_CONTROL, DISPLAYID_ENABLE_INC_SB, 0x1) ;
	BREG_Write32(hRegister, BCHP_AON_HDMI_RX_DISPLAYID_CONTROL, Register) ;


	Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_RAM_CONTROL) ;
	Register &= ~ (
		  BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_WRITE_ENABLE)
		| BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_READ_ENABLE)
		| BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_ACCESS_ADDRESS)
		| BCHP_MASK(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_WRITE_DATA)) ;
	Register |=
		  BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_WRITE_ENABLE, 0x0)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_READ_ENABLE, 0x0)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_ACCESS_ADDRESS, 0x0)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_RAM_CONTROL, EDID_WRITE_DATA, 0x0) ;
	BREG_Write32(hRegister, BCHP_AON_HDMI_RX_EDID_RAM_CONTROL, Register) ;



	Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_ENERGYSTAR_CONTROL) ;
	Register &= ~ (
		  BCHP_MASK(AON_HDMI_RX_EDID_ENERGYSTAR_CONTROL, FORCE_EDID_CLK)
		| BCHP_MASK(AON_HDMI_RX_EDID_ENERGYSTAR_CONTROL, ENABLE_EDID_ES)
		| BCHP_MASK(AON_HDMI_RX_EDID_ENERGYSTAR_CONTROL, CLEAR_EDID_ES)) ;
	Register |=
		  BCHP_FIELD_DATA(AON_HDMI_RX_EDID_ENERGYSTAR_CONTROL, FORCE_EDID_CLK, 0x1)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_ENERGYSTAR_CONTROL, ENABLE_EDID_ES, 0x1)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_ENERGYSTAR_CONTROL, CLEAR_EDID_ES, 0x0) ;
	BREG_Write32(hRegister, BCHP_AON_HDMI_RX_EDID_ENERGYSTAR_CONTROL, Register) ;



	Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_I2C_DELAY_CFG) ;
	Register &= ~ BCHP_MASK(AON_HDMI_RX_EDID_I2C_DELAY_CFG, I2C_LPF_FILTER_THRESHOLD) ;
	Register |=
		BCHP_FIELD_DATA(AON_HDMI_RX_EDID_I2C_DELAY_CFG, I2C_LPF_FILTER_THRESHOLD, 0x1) ;
	BREG_Write32(hRegister,BCHP_AON_HDMI_RX_EDID_I2C_DELAY_CFG, Register) ;

	Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_I2C_GEN_START_STOP_CONTROL) ;
	Register &= ~ (
		  BCHP_MASK(AON_HDMI_RX_EDID_I2C_GEN_START_STOP_CONTROL, SCL_DELAY_START)
		| BCHP_MASK(AON_HDMI_RX_EDID_I2C_GEN_START_STOP_CONTROL, SCL_DELAY_STOP)
		| BCHP_MASK(AON_HDMI_RX_EDID_I2C_GEN_START_STOP_CONTROL, SDA_UP_DELAY)
		| BCHP_MASK(AON_HDMI_RX_EDID_I2C_GEN_START_STOP_CONTROL, SDA_DN_DELAY)) ;
	Register |=
		  BCHP_FIELD_DATA(AON_HDMI_RX_EDID_I2C_GEN_START_STOP_CONTROL, SCL_DELAY_START, 0x0)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_I2C_GEN_START_STOP_CONTROL, SCL_DELAY_STOP, 0x0)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_I2C_GEN_START_STOP_CONTROL, SDA_UP_DELAY, 0x0)
		| BCHP_FIELD_DATA(AON_HDMI_RX_EDID_I2C_GEN_START_STOP_CONTROL, SDA_DN_DELAY, 0x0) ;
	BREG_Write32(hRegister, BCHP_AON_HDMI_RX_EDID_I2C_GEN_START_STOP_CONTROL, Register) ;


	/* Enable I2c access to the  EDID RAM device */
	Register = BREG_Read32(hRegister, BCHP_AON_HDMI_RX_EDID_CONTROL + ulOffset) ;
	Register &= ~ BCHP_MASK(AON_HDMI_RX_EDID_CONTROL, ENABLE_EDID_DEVICE) ;
	Register |= BCHP_FIELD_DATA(AON_HDMI_RX_EDID_CONTROL, ENABLE_EDID_DEVICE, 1) ;
	BREG_Write32(hRegister, BCHP_AON_HDMI_RX_EDID_CONTROL + ulOffset, Register) ;


	BDBG_LEAVE(BHDR_EDID_RAM_Enable) ;

}


