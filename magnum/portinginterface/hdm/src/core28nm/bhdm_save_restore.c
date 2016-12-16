/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/

#include "../common/bhdm_priv.h"

BDBG_MODULE(BHDM_SAVE_RESTORE) ;


/***********************************
** HDMI RW registers that are saved and restored
** are generated via the following command:
**    grep -F "[RW][32]" magnum/basemodules/chp/include/7268/rdb/b0/bchp_hdmi.h | cut -d ' ' -f 2
**
**
** NOTE: To pass HDCP 1.4 1B-01 on Simplay tester (FW 2.0.1),
** we need to skip registers from HDMI_BKSV0..HDMI_HDCP_KEY_2
** the following registers should not appear in the BHDM_P_SaveRestoreRegList
** list below
**
** 	BCHP_HDMI_BKSV0,
** 	BCHP_HDMI_BKSV1,
** 	BCHP_HDMI_KSV_FIFO_0,
** 	BCHP_HDMI_KSV_FIFO_1,
** 	BCHP_HDMI_V,
** 	BCHP_HDMI_HDCP_KEY_1,
** 	BCHP_HDMI_HDCP_KEY_2,
***********************************/

#if BHDM_CONFIG_HAS_HDCP22
static const uint32_t BHDM_P_SaveRestoreRegList[] =
{
	BCHP_HDMI_HDCP_CTL,
	BCHP_HDMI_CP_INTEGRITY_CFG,
	BCHP_HDMI_CP_INTEGRITY_CHK_CFG_1,
	BCHP_HDMI_CP_INTEGRITY_CHK_CFG_2,
	BCHP_HDMI_CP_INTEGRITY_CHK_CFG_3,
	BCHP_HDMI_CP_INTEGRITY_CHK_CFG_4,
	BCHP_HDMI_CP_INTEGRITY_CHK_CFG_5,
	BCHP_HDMI_CP_INTEGRITY_CHK_CFG_6,
	BCHP_HDMI_CP_INTEGRITY_CHK_CFG_7,
	BCHP_HDMI_CP_INTEGRITY_CHK_CFG_8,
#if BCHP_HDMI_CP_INTEGRITY_CHK_CFG_9
	BCHP_HDMI_CP_INTEGRITY_CHK_CFG_9,
	BCHP_HDMI_CP_INTEGRITY_CHK_CFG_10,
#endif
	BCHP_HDMI_CP_CONFIG,
	BCHP_HDMI_CP_TST,
	BCHP_HDMI_CP_CONSTRAIN,
	BCHP_HDMI_FIFO_CTL,
	BCHP_HDMI_ENCODER_CTL,
	BCHP_HDMI_PERT_CONFIG,
	BCHP_HDMI_PERT_LFSR_PRELOAD,
	BCHP_HDMI_PERT_LFSR_FEEDBACK_MASK,
	BCHP_HDMI_PERT_INSERT_ERR,
	BCHP_HDMI_PERT_INSERT_ERR_SEPARATION,
	BCHP_HDMI_PERT_TEST_LENGTH,
	BCHP_HDMI_PERT_DATA,
	BCHP_HDMI_MAI_CHANNEL_MAP,
	BCHP_HDMI_MAI_CONFIG,
	BCHP_HDMI_HDMI_HBR_AUDIO_PACKET_HEADER,
	BCHP_HDMI_MAI_BUS_MONITOR_1,
	BCHP_HDMI_MAI_BUS_MONITOR_2,
	BCHP_HDMI_AUDIO_PACKET_CONFIG,
	BCHP_HDMI_RAM_PACKET_CONFIG,
	BCHP_HDMI_RAM_PACKET_CONFIG_2,
	BCHP_HDMI_CRP_CFG,
	BCHP_HDMI_CTS_0,
	BCHP_HDMI_CTS_1,
	BCHP_HDMI_CTS_PERIOD_0,
	BCHP_HDMI_CTS_PERIOD_1,
	BCHP_HDMI_BCH_CONFIGURATION,
	BCHP_HDMI_SCHEDULER_CONTROL,
	BCHP_HDMI_HORZA,
	BCHP_HDMI_HORZB,
	BCHP_HDMI_VERTA0,
	BCHP_HDMI_VERTB0,
	BCHP_HDMI_VERTA1,
	BCHP_HDMI_VERTB1,
	BCHP_HDMI_TEST,
	BCHP_HDMI_MISC_CONTROL,
	BCHP_HDMI_MISC_CONTROL_1,
	BCHP_HDMI_PACKET_FIFO_CTL,
	BCHP_HDMI_PACKET_FIFO_CFG,
	BCHP_HDMI_HDMI_13_AUDIO_CFG_1,
	BCHP_HDMI_HDMI_POSTING_MASTER,
	BCHP_HDMI_FORMAT_DET_CFG,
	BCHP_HDMI_FORMAT_DET_UPDATE_CLEAR,
	BCHP_HDMI_FORMAT_DET_UPDATE_CLEAR_1,
	BCHP_HDMI_DEEP_COLOR_CONFIG_1,
	BCHP_HDMI_GCP_CONFIG,
	BCHP_HDMI_GCP_WORD_1,
	BCHP_HDMI_GCP_WORD_2,
	BCHP_HDMI_RGB_MONITOR_CFG_1,
	BCHP_HDMI_RGB_MONITOR_CFG_2,
	BCHP_HDMI_RGB_MONITOR_CFG_3,
	BCHP_HDMI_RGB_MONITOR_CFG_4,
	BCHP_HDMI_VMS_CFG_0,
	BCHP_HDMI_VMS_CFG_1,
	BCHP_HDMI_SCRAMBLER_LFSR_0_INIT,
	BCHP_HDMI_SCRAMBLER_LFSR_1_INIT,
	BCHP_HDMI_SCRAMBLER_LFSR_2_INIT,
	BCHP_HDMI_SCRAMBLER_LFSR_REVERSE,
	BCHP_HDMI_SCRAMBLER_CTL,
	BCHP_HDMI_CTL_UNSCRAMBLED_CFG_0,
	BCHP_HDMI_CTL_UNSCRAMBLED_CFG_1,
	BCHP_HDMI_CTL_SCRAMBLED_CFG_0,
	BCHP_HDMI_CTL_SCRAMBLED_CFG_1,
	BCHP_HDMI_CTL_SCRAMBLED_CFG_2,
	BCHP_HDMI_CTL_SCRAMBLED_CFG_3,
	BCHP_HDMI_CTL_SCRAMBLED_CFG_4,
	BCHP_HDMI_CTL_SCRAMBLED_CFG_5,
	BCHP_HDMI_CTL_SCRAMBLED_CFG_6,
	BCHP_HDMI_CTL_SCRAMBLED_CFG_7,
	BCHP_HDMI_CTL_SCRAMBLED_CFG_8,
	BCHP_HDMI_CTL_SCRAMBLED_CFG_9,
	BCHP_HDMI_CTL_SCRAMBLED_CFG_10,
	BCHP_HDMI_MHL_CTL,
	BCHP_HDMI_HDCP2TX_CFG0,
	BCHP_HDMI_HDCP2TX_MBOX_0,
	BCHP_HDMI_HDCP2TX_MBOX_1,
	BCHP_HDMI_HDCP2TX_AUTH_CTL,
	BCHP_HDMI_HDCP2TX_CTRL0,
#if BCHP_HDMI_ERR_INSERT_CTRL
	BCHP_HDMI_ERR_INSERT_CTRL,
	BCHP_HDMI_ERR_INSERT_CFG_0,
	BCHP_HDMI_ERR_INSERT_CFG_1,
	BCHP_HDMI_ERR_INSERT_CFG_2,
	BCHP_HDMI_ERR_INSERT_CFG_3,
	BCHP_HDMI_ERR_INSERT_CFG_4,
	BCHP_HDMI_ERR_INSERT_CFG_5,
	BCHP_HDMI_ERR_INSERT_CFG_6,
	BCHP_HDMI_ERR_INSERT_CFG_7,
	BCHP_HDMI_ERR_INSERT_CFG_8,
#endif
#if	BCHP_HDMI_HDR_DEBUG0
	BCHP_HDMI_HDR_DEBUG0,
	BCHP_HDMI_HDR_DEBUG2,
#endif
	BCHP_HDMI_SPARE_REG2,
	0
} ;


void BHDM_P_ResetHDCPI2C_isr(const BHDM_Handle hHDMI)
{
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;
	uint16_t i=0 ;
	uint16_t index = 0 ;
	uint32_t regAddress = BCHP_HDMI_REG_START;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/***********************************
	** Save register values before resetting **
	***********************************/
	{
		BDBG_MSG(("Num HDMI Regs (size) : %3d (%d bytes)",
			hdmiRegBuffSize, (unsigned) sizeof(hHDMI->hdmiRegBuff))) ;
		BDBG_MSG((" RW HDMI Regs (size) : %3d (%d bytes)",
			(unsigned) (sizeof(BHDM_P_SaveRestoreRegList) / 4), (unsigned) sizeof(BHDM_P_SaveRestoreRegList))) ;

		/* make sure the size of the BHDM RW reg list is <= to the size total number of HDMI registers */
		BDBG_CASSERT(sizeof(BHDM_P_SaveRestoreRegList) <= sizeof(hHDMI->hdmiRegBuff)) ;

		/* clear buffer before starting */
		BKNI_Memset(hHDMI->hdmiRegBuff, 0, sizeof(hHDMI->hdmiRegBuff));

		i=0 ; index = 0 ;
		while (BHDM_P_SaveRestoreRegList[index])
		{
			hHDMI->hdmiRegBuff[i] = BREG_Read32(hRegister, BHDM_P_SaveRestoreRegList[index] + ulOffset);
			BDBG_MSG(("Save RegAddr: %x = %x",
				BHDM_P_SaveRestoreRegList[index] + ulOffset, hHDMI->hdmiRegBuff[i])) ;
			i++ ; index++ ;
		}

		/* clear buffer before starting */
		BKNI_Memset(hHDMI->autoI2cRegBuff, 0, autoI2cRegBuffSize);
		i=0;
		regAddress = BCHP_HDMI_TX_AUTO_I2C_CH0_CFG;
		while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH0_REG13_WD)
		{
			Register = BREG_Read32(hRegister, regAddress + ulOffset);
			hHDMI->autoI2cRegBuff[i++] = Register;
			regAddress+=4;
		}

		regAddress = BCHP_HDMI_TX_AUTO_I2C_CH1_CFG;
		while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH1_REG13_WD)
		{
			Register = BREG_Read32(hRegister, regAddress + ulOffset);
			hHDMI->autoI2cRegBuff[i++] = Register;
			regAddress+=4;
		}

		regAddress = BCHP_HDMI_TX_AUTO_I2C_CH2_CFG;
		while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH2_REG13_WD)
		{
			Register = BREG_Read32(hRegister, regAddress + ulOffset);
			hHDMI->autoI2cRegBuff[i++] = Register;
			regAddress+=4;
		}

		/* CH3 has some address holes */
		regAddress = BCHP_HDMI_TX_AUTO_I2C_CH3_CFG;
		while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH3_REG3_CFG)
		{
			Register = BREG_Read32(hRegister, regAddress + ulOffset);
			hHDMI->autoI2cRegBuff[i++] = Register;
			regAddress+=4;
		}

		regAddress = BCHP_HDMI_TX_AUTO_I2C_CH3_REG3_WD;
		while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH3_REG13_WD)
		{
			Register = BREG_Read32(hRegister, regAddress + ulOffset);
			hHDMI->autoI2cRegBuff[i++] = Register;
			regAddress+=4;
		}

		Register = BREG_Read32(hRegister, BCHP_HDMI_TX_AUTO_I2C_START + ulOffset);
		hHDMI->autoI2cRegBuff[i++] = Register;

		/******************************
		** NOTE: To pass 1B-07 on QD980, DO NOT save/restore registers
		** from BCHP_HDMI_TX_AUTO_I2C_TRANSACTION_DONE_STAT_CLEAR
		**   to BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_FSM_DEBUG
		******************************/
    }


	/******************************
	** All the register values are saved.
	** Now reset the cores
	******************************/
	Register = BREG_Read32(hRegister, BCHP_DVP_HT_SW_INIT + ulOffset);
	Register |= 0x000000C0; /* write 1 to HDCP and HDCP2_I2C - private fields */
	BREG_Write32(hRegister, BCHP_DVP_HT_SW_INIT + ulOffset, Register);

	Register &= 0x0000013F; /* write 0 to HDCP and HDCP2_I2C - private fields */
	BREG_Write32(hRegister, BCHP_DVP_HT_SW_INIT + ulOffset, Register) ;


	/********************************
	** Reload register values after reset  **
	*********************************/
	{
		i=0 ; index = 0 ;
		while (BHDM_P_SaveRestoreRegList[index])
		{
			BREG_Write32(hRegister, BHDM_P_SaveRestoreRegList[index] + ulOffset, hHDMI->hdmiRegBuff[i]) ;
			BDBG_MSG(("Write RegAddr: %x = %x",
				BHDM_P_SaveRestoreRegList[index] + ulOffset, hHDMI->hdmiRegBuff[i])) ;
			i++ ; index++ ;
		}

		i=0;
		regAddress = BCHP_HDMI_TX_AUTO_I2C_CH0_CFG;
		while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH0_REG13_WD)
		{
			Register = hHDMI->autoI2cRegBuff[i++];
			BREG_Write32(hRegister, regAddress + ulOffset, Register) ;
			regAddress+=4;
		}

		regAddress = BCHP_HDMI_TX_AUTO_I2C_CH1_CFG;
		while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH1_REG13_WD)
		{
			Register = hHDMI->autoI2cRegBuff[i++];
			BREG_Write32(hRegister, regAddress + ulOffset, Register) ;
			regAddress+=4;
		}

		regAddress = BCHP_HDMI_TX_AUTO_I2C_CH2_CFG;
		while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH2_REG13_WD)
		{
			Register = hHDMI->autoI2cRegBuff[i++];
			BREG_Write32(hRegister, regAddress + ulOffset, Register) ;
			regAddress+=4;
		}

		regAddress = BCHP_HDMI_TX_AUTO_I2C_CH3_CFG;
		while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH3_REG3_CFG)
		{
			Register = hHDMI->autoI2cRegBuff[i++];
			BREG_Write32(hRegister, regAddress + ulOffset, Register) ;
			regAddress+=4;
		}

		regAddress = BCHP_HDMI_TX_AUTO_I2C_CH3_REG3_WD;
		while (regAddress <= BCHP_HDMI_TX_AUTO_I2C_CH3_REG13_WD)
		{
			Register = hHDMI->autoI2cRegBuff[i++];
			BREG_Write32(hRegister, regAddress + ulOffset, Register) ;
			regAddress+=4;
		}

		Register = hHDMI->autoI2cRegBuff[i++];
		BREG_Write32(hRegister, BCHP_HDMI_TX_AUTO_I2C_START + ulOffset, Register);

		/******************************
		** NOTE: To pass 1B-07 on QD980, DO NOT save/restore registers
		** from BCHP_HDMI_TX_AUTO_I2C_TRANSACTION_DONE_STAT_CLEAR
		**   to BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_FSM_DEBUG
		******************************/
    }
}
#endif
