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
 * Module Description: This file contains mode config data structures that shall be shared
 *					with host. 
 *
 ***************************************************************************/
#ifndef BSP_S_MODE_CONFIG_H__
#define BSP_S_MODE_CONFIG_H__


/*

BCMD_cmdType_eMODE_CONFIG  - Use this command to read the customer
mode and further restrict the usage of certain features.

Input Parameters passed:
1) Disabled Crypto Primitive Mode - 
The bit vector defines which crypto Primitive mode is enabled. The final definition should be referred to OTP: MC1[OTP_Ctrl_Bit*] bits.
Bit 0: CA-DVB-Regular/ Conformance: 0 = Enable, 1 = Disable
Bit 1: CA-DES-(ECB/CBC): 0 = Enable, 1 = Disable
Bit 2: CA-Multi2-(ECB/ CBC): 0 = Enable, 1 = Disable
Bit 3: CA-3DES(ECB/CBC): 0 = Enable, 1 = Disable
Bit 4: reserved
Bit 5: reserved
Bit 6: reserved	
	
Bit 7: M2M-3/DES-ECB: 0 = Enable, 1 = Disable
Bit 8: M2M-C2: 0 = Enable, 1 = Disable
Bit 9: M2M-CSS: 0 = Enable, 1 = Disable
Bit 10: M2M-M6: 0 = Enable, 1 = Disable
Bit 11: M2M-AES128-CBC: 0 = Enable, 1 = Disable
Bit 12: M2M-AES128-ECB: 0 = Enable, 1 = Disable
Bit 13: M2M-AES192-CBC: 0 = Enable, 1 = Disable
Bit 14: M2M-AES192-ECB: 0 = Enable, 1 = Disable
Bit 15: reserved
Bit 16: reserved	

Bit 17: HDMI-HDCP: 0 = Enable, 1 = Disable	
Bit 18: Force RMX0 to encrypt content: 0 = Disable, 1 = Enable
Bit 19: Force RMX1 to encrypt content: 0 = Disable, 1 = Enable
Bit 20: reserved
Bit 21: reserved
	
Bit 22: PKE-RSA/DH/DSA: 0 = Enable, 1 = Disable
Bit 23: reserved
Bit 24: MPOD: 0 = Enable, 1 = Disable
Bit 25: reserved
Bit 26: reserved
Bit 27: SAL: 0 = Disable , 1 = Enable

2) Disabled other Mode
The bit vector defines which other mode is enabled..
Bit 0: disable Export Ctrl: 0 = Enable, 1 = Disable
Bit 1: challenge and response bit 0
Bit 2: challenge and response bit 1
Bit 3: PCI Host Enabled by C/R: 1 = Enable, 0 = Disable
Bit 4: PCI Client Enabled by C/R: 1 = Enable, 0 = Disable
Bit 5: Test Port Enabled by C/R: 1 = Enable, 0 = Disable
Bit 6: Force DRAM scrambler: 1 = Enable, 0 = Disab	le
Bit 7: reserved
Bit 8: Disable Debug Cmd: 0 = Enable, 1 = Disable


BCMD_ModeConfig_InCmdField_e: 
|-------------------|-------------------|-------------------|------------------| 
|  31..24           | 23..16            | 15..8             | 7..0             | 
|-------------------|-------------------|-------------------|------------------| 
|  Disabled Crypto Primitive Mode - 32                                         | 
|-------------------|-------------------|-------------------|------------------| 
|  unused           | unused            | Disabled other Mode - 9              | 
|-------------------|-------------------|-------------------|------------------|

Output Parameters:
1) Customer Mode:  
The bit vector defines which customer mode is enabled. 
The final definition should be referred to OTP: MC1[Operation_Mode_Select] bits.

2) Disabled Crypto Primitive Mode: 
The bit vector defines which crypto Primitive mode is enabled. 
The final definition should be referred to OTP: MC1[OTP_Ctrl_Bit*] 
bits and other MSP bits.
Bit 0: CA-DVB-Regular/ Conformance: 0 = Enable, 1 = Disable
Bit 1: CA-DES-(ECB/CBC): 0 = Enable, 1 = Disable
Bit 2: CA-Multi2-(ECB/ CBC): 0 = Enable, 1 = Disable
Bit 3: CA-3DES(ECB/CBC): 0 = Enable, 1 = Disable
Bit 4: reserved
Bit 5: reserved
Bit 6: reserved	
	
Bit 7: M2M-3/DES-ECB: 0 = Enable, 1 = Disable
Bit 8: M2M-C2: 0 = Enable, 1 = Disable
Bit 9: M2M-CSS: 0 = Enable, 1 = Disable
Bit 10: M2M-M6: 0 = Enable, 1 = Disable
Bit 11: M2M-AES128-CBC: 0 = Enable, 1 = Disable
Bit 12: M2M-AES128-ECB: 0 = Enable, 1 = Disable
Bit 13: M2M-AES192-CBC: 0 = Enable, 1 = Disable
Bit 14: M2M-AES192-ECB: 0 = Enable, 1 = Disable
Bit 15: reserved
Bit 16: reserved	

Bit 17: HDMI-HDCP: 0 = Enable, 1 = Disable	
Bit 18: Force RMX0 to encrypt content: 0 = Disable, 1 = Enable
Bit 19: Force RMX1 to encrypt content: 0 = Disable, 1 = Enable
Bit 20: reserved
Bit 21: reserved
	
Bit 22: PKE-RSA/DH/DSA: 0 = Enable, 1 = Disable
Bit 23: reserved
Bit 24: MPOD: 0 = Enable, 1 = Disable
Bit 25: reserved
Bit 26: reserved
Bit 27: SAL: 0 = Disable , 1 = Enable


3) Disabled other Mode:
The bit vector defines which other mode is enabled..
Bit 0: disable Export Ctrl: 0 = Enable, 1 = Disable
Bit 1: challenge and response bit 0
Bit 2: challenge and response bit 1
Bit 3: PCI Host Enabled by C/R: 1 = Enable, 0 = Disable
Bit 4: PCI Client Enabled by C/R: 1 = Enable, 0 = Disable
Bit 5: Test Port Enabled by C/R: 1 = Enable, 0 = Disable
Bit 6: Force DRAM scrambler: 1 = Enable, 0 = Disable
Bit 7: reserved
Bit 8: Disable Debug Cmd: 0 = Enable, 1 = Disable

BCMD_ModeConfig_OutCmdField_e: 
|-------------------|-------------------|-------------------|------------------| 
|  31..24           | 23..16            | 15..8             | 7..0             | 
|-------------------|-------------------|-------------------|------------------| 
|  unused           | unused            | unused            |Customer Mode - 6 | 
|-------------------|-------------------|-------------------|------------------| 
|  Disabled Crypto Primitive Mode - 32                                         | 
|-------------------|-------------------|-------------------|------------------| 
|  unused           | unused            | Disabled other Mode - 16             | 
|-------------------|-------------------|-------------------|------------------| 


 */
 
typedef enum BCMD_ModeConfig_InCmdField_e
{
	BCMD_ModeConfig_InCmdField_eOtpCtrlBits =(5<<2) + 0, /* 32 bit index */		
	BCMD_ModeConfig_InCmdField_eNonOtpCtrlBits = (6<<2) + 0,   /* 9 bit index */
	BCMD_ModeConfig_InCmdField_eOtpCtrlBitsSet2 =(7<<2) + 0 /* 32 bit index */		
} BCMD_ModeConfig_InCmdField_e;

typedef enum BCMD_ModeConfig_OutCmdField_e	
{
	BCMD_ModeConfig_OutCmdField_eCustomerMode = (6<<2) + 3, /* 6 bit index */
	BCMD_ModeConfig_OutCmdField_eOtpCtrlBits24_31 = (7<<2) + 0, /* 8 bit index */
	BCMD_ModeConfig_OutCmdField_eOtpCtrlBits16_23 = (7<<2) + 1, /* 8 bit index */
	BCMD_ModeConfig_OutCmdField_eOtpCtrlBits8_15 = (7<<2) + 2, /* 8 bit index */
	BCMD_ModeConfig_OutCmdField_eOtpCtrlBits0_7 = (7<<2) + 3, /* 8 bit index */
	BCMD_ModeConfig_OutCmdField_eNonOtpCtrlBits8_15 = (8<<2) + 2, /* 8 bit index */
	BCMD_ModeConfig_OutCmdField_eNonOtpCtrlBits0_7 = (8<<2) + 3, /* 8 bit index */
	BCMD_ModeConfig_OutCmdField_eOtpCtrlBitsSet2_24_31 = (9<<2) + 0, /* 8 bit index */
	BCMD_ModeConfig_OutCmdField_eOtpCtrlBitsSet2_16_23 = (9<<2) + 1, /* 8 bit index */
	BCMD_ModeConfig_OutCmdField_eOtpCtrlBitsSet2_8_15 = (9<<2) + 2, /* 8 bit index */
	BCMD_ModeConfig_OutCmdField_eOtpCtrlBitsSet2_0_7 = (9<<2) + 3 /* 8 bit index */	

} BCMD_ModeConfig_OutCmdField_e;

#define BCMD_MODE_CFG_DISABLE_EXPORT_CTRL_SHIFT			0
#define BCMD_MODE_CFG_CR_OTP_BIT_0             					1
#define BCMD_MODE_CFG_CR_OTP_BIT_1             					2
#define BCMD_MODE_CFG_PCI_HOST_ENABLED_BY_CR_SHIFT             	3
#define BCMD_MODE_CFG_PCI_CLIENT_ENABLED_BY_CR_SHIFT		4
#define BCMD_MODE_CFG_TEST_PORT_ENABLED_BY_CR_SHIFT            5
#define BCMD_MODE_CFG_RESERVED_6_SHIFT             	6
#define BCMD_MODE_CFG_RESERVED_7_SHIFT      7
#define BCMD_MODE_CFG_DISABLE_DEBUG_CMD_SHIFT      			8

#endif /* BSP_S_MODE_CONFIG_H__ */

