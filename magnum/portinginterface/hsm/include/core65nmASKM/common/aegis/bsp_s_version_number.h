/***************************************************************************
 *     Copyright (c) 2005-2009, Broadcom Corporation
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
 * Security Processor Code for Register Access
 ***************************************************************************/

#ifndef BSP_S_VERSION_NUMBER_H__
#define BSP_S_VERSION_NUMBER_H__

/* 
 * 32-bit Version Number for Aegis: 
 * bit [31 -24] is the chip number. For eg, 0x0 = 7401 , 0x1 = 7400, 0x2  = 7118 
 * bit [23 -16] is the chip version. For eg, 0xA0 = A0, 0xB1 = B1 
 * bit [15 -12] is the major version number. For eg, 0x1, 0x2.
 * bit [11 -06] is the minor version number. For eg, 0x1, 0x2.
 * bit [05] is reserved
 * bit [04 - 00] is the customer mode. For eg, All customers = 0x1F, GENERIC = 0x00, SVPDSS = 0x0B

 * 32-bit Version Number for Host: 
 * bit [31 -24] is the chip number. For eg, 0x0 = 7401 , 0x1 = 7400, 0x2  = 7118 
 * bit [23 -16] is the chip version. For eg, 0xA0 = A0, 0xB1 = B1 
 * bit [15 -12] is the major version number. For eg, 0x1, 0x2.
 * bit [11 -06] is the minor version number. For eg, 0x1, 0x2.
 * bit [05 - 00] is reserved.	


 1)	Aegis and Host both increase major number by one if we decide to have a different command structure.

 2)	Aegis should increment the minor number of specific chip revision by 1, if we decide to release a ROM or patch
 	codes for that specific chip revision.  Host can have a different minor number from Aegis.
 	
 3) 	Host will check if bit[31-12] of Aegis verision number matches with it version number.  
 	If so, then host will continue 	process the next command. Otherwise, host stop processing command.
 	Aegis does not have to perform the any version number checking.

 4)	Host should not have those customer codes in any header files. 	

 	By default, the customer mode is ALL since the code is built for all customers.
 	So we #define BSP_S_CUSTOMER_MODE			0x1F
 	
 	If the PATCH codes is built for certain customer, then someone has to
 	change the definition of BSP_S_CUSTOMER_MODE by modifying
 	"bsp_customer_code.h"

 5)	For ROM codes, bit[15 - 00] should be all 0s.  For PATCH codes, bit[15 - 00]
 	should contain major, minor and customer codes.

 6)	Label for ClearCase:
 	ChipNum_ChipVersion_MajorNum_MinorNum_ROM/PATCH_CustomerMode
 	For Eg:
 	7401_A0_1_0_ROM_ALL (32 bit value = 0x00A00000)
 	7401_B0_1_0_ROM_ALL (32 bit value = 0x00B00000)
 	7401_B0_1_0_PATCH_ALL (32 bit value = 0x00B0103F)
 	7400_A0_1_0_ROM_ALL (32 bit value = 0x01A00000)
 	7400_A0_1_0_PATCH_ALL (32 bit value = 0x01A0103F)

 	7401_B0_1_0_PATCH_SVPDSS (32 bit value = 0x00B0102B)
 	7401_B0_1_1_PATCH_SVPDSS (32 bit value = 0x00B0106B)

	 For each label, we should be able to refer to a list of PR or release note. 

 */

#define BSP_S_CHIP_NUMBER_SHIFT		24
#define BSP_S_CHIP_VERSION_SHIFT		16
#define BSP_S_MAJOR_VERSION_SHIFT		12
#define BSP_S_MINOR_VERSION_SHIFT		6
#define BSP_S_CUSTOMER_MODE_SHIFT	0

 
#define BSP_S_CHIP_NUMBER_7401		0x00L
#define BSP_S_CHIP_NUMBER_7400		0x01L
#define BSP_S_CHIP_NUMBER_7118		0x02L
#define BSP_S_CHIP_NUMBER_7440		0x03L
#define BSP_S_CHIP_NUMBER_3563		0x04L
#define BSP_S_CHIP_NUMBER_7405		0x07L
#define BSP_S_CHIP_NUMBER_7325		0x08L
#define BSP_S_CHIP_NUMBER_7335		0x09L
#define BSP_S_CHIP_NUMBER_3573		0x0AL
#define BSP_S_CHIP_NUMBER_3548      0x0bL
#define BSP_S_CHIP_NUMBER_7420		0x0BL
#define BSP_S_CHIP_NUMBER_7340		0x0CL
#define BSP_S_CHIP_NUMBER_7342	    0x0DL
#define BSP_S_CHIP_NUMBER_7125      0x0EL
#define BSP_S_CHIP_NUMBER_7468      0x0FL


#define BSP_S_A0_CHIP_VERSION			0xA0L
#define BSP_S_A1_CHIP_VERSION			0xA1L
#define BSP_S_A2_CHIP_VERSION			0xA2L
#define BSP_S_A3_CHIP_VERSION			0xA3L
#define BSP_S_B0_CHIP_VERSION			0xB0L
#define BSP_S_B1_CHIP_VERSION			0xB1L
#define BSP_S_B2_CHIP_VERSION			0xB2L
#define BSP_S_B3_CHIP_VERSION			0xB3L
#define BSP_S_C0_CHIP_VERSION			0xC0L
#define BSP_S_C1_CHIP_VERSION			0xC1L
#define BSP_S_C2_CHIP_VERSION			0xC2L
#define BSP_S_C3_CHIP_VERSION			0xC3L
#define BSP_S_D0_CHIP_VERSION			0xD0L
#define BSP_S_D1_CHIP_VERSION			0xD1L
#define BSP_S_D2_CHIP_VERSION			0xD2L
#define BSP_S_D3_CHIP_VERSION			0xD3L
#define BSP_S_E0_CHIP_VERSION			0xE0L
#define BSP_S_E1_CHIP_VERSION			0xE1L
#define BSP_S_E2_CHIP_VERSION			0xE2L
#define BSP_S_E3_CHIP_VERSION			0xE3L
#define BSP_S_F0_CHIP_VERSION			0xF0L
#define BSP_S_F1_CHIP_VERSION			0xF1L
#define BSP_S_F2_CHIP_VERSION			0xF2L
#define BSP_S_F3_CHIP_VERSION			0xF3L

#define BSP_S_BSECK_GENERIC				0x00
#define BSP_S_BSECK_ESTAR				0x05
#define BSP_S_BSECK_DCAS				0x08 /* This is the NGNA customer mode and is used for DCAS */
#define BSP_S_BSECK_SVP					0x07 /* This includes ICAM + VGS + SVP mode */
#define BSP_S_BSECK_SVP_DSS				0x0B /* This includes DSS + SVP_DSS mode */
#define BSP_S_BSECK_SVP_VGS_DSS_GENERIC_ESTAR	0x1C /* This includes SVP + VGS + DSS + GENERIC + ESTAR */
#define BSP_S_BSECK_MSIPTV_VGS			0x1D /* This includes GENERIC + VGS + MSIPTV */
#define BSP_S_BSECK_SVP_ICAM_VGS_DSS	0x1E /* This includes ICAM + VGS + DSS + SVP + SVP_DSS mode */
#define BSP_S_BSECK_ALL_CUSTOMER		0x1F

#endif /* BSP_S_VERSION_NUMBER_H__ */

