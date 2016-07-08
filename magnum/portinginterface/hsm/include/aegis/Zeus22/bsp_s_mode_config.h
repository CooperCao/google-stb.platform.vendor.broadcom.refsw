/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#ifndef BSP_S_MODE_CONFIG_H__
#define BSP_S_MODE_CONFIG_H__



typedef enum BCMD_ModeConfig_InCmdField_e
{
	BCMD_ModeConfig_InCmdField_eOtpCtrlBits =(5<<2) + 0,
	BCMD_ModeConfig_InCmdField_eNonOtpCtrlBits = (6<<2) + 0,
	BCMD_ModeConfig_InCmdField_eOtpCtrlBitsSet2 =(7<<2) + 0
} BCMD_ModeConfig_InCmdField_e;

typedef enum BCMD_ModeConfig_OutCmdField_e
{
	BCMD_ModeConfig_OutCmdField_eCustomerMode = (6<<2) + 3,
	BCMD_ModeConfig_OutCmdField_eOtpCtrlBits24_31 = (7<<2) + 0,
	BCMD_ModeConfig_OutCmdField_eOtpCtrlBits16_23 = (7<<2) + 1,
	BCMD_ModeConfig_OutCmdField_eOtpCtrlBits8_15 = (7<<2) + 2,
	BCMD_ModeConfig_OutCmdField_eOtpCtrlBits0_7 = (7<<2) + 3,
	BCMD_ModeConfig_OutCmdField_eNonOtpCtrlBits8_15 = (8<<2) + 2,
	BCMD_ModeConfig_OutCmdField_eNonOtpCtrlBits0_7 = (8<<2) + 3,
	BCMD_ModeConfig_OutCmdField_eOtpCtrlBitsSet2_24_31 = (9<<2) + 0,
	BCMD_ModeConfig_OutCmdField_eOtpCtrlBitsSet2_16_23 = (9<<2) + 1,
	BCMD_ModeConfig_OutCmdField_eOtpCtrlBitsSet2_8_15 = (9<<2) + 2,
	BCMD_ModeConfig_OutCmdField_eOtpCtrlBitsSet2_0_7 = (9<<2) + 3

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

#endif
