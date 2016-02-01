/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $ 
 * 
  ***************************************************************************/
#ifndef BCHP_31xx_H__
#define BCHP_31xx_H__

#define DEVICE(val)           BCM31xx_##val

/*****************************************************
These are the Host Indirect Registers written using the IOMbox.
This is just a hack for now.
*****************************************************/
#define BCM31xx_US_TOP_PLL_DIV1        0x20c0
#define BCM31xx_US_TOP_PLL_DIV2        0x20c4
#define BCM31xx_US_TOP_PLL_CLK_US      0x20c8
#define BCM31xx_US_TOP_PLL_CLK_OB      0x20cc
#define BCM31xx_US_TOP_PLL_CLK_DS      0x20d0
#define BCM31xx_US_TOP_PLL_CLK_DS_AFE  0x20d4
#define BCM31xx_US_TOP_PLL_CLK_AP      0x20d8
#define BCM31xx_US_TOP_PLL_CLK_REF2    0x20dc
#define BCM31xx_US_TOP_PLL_CLK_REF3    0x20e0
#define BCM31xx_US_TOP_PLL_BYP         0x20e4
#define BCM31xx_US_TOP_PLL_RST         0x20e8
#define BCM31xx_US_TOP_PLL_MISC        0x20ec
#define BCM31xx_US_TOP_PLL_CTRL_1      0x20f0
#define BCM31xx_US_TOP_PLL_CTRL_0      0x20f4
#define BCM31xx_US_TOP_PLL_PHASE_1     0x20f8
#define BCM31xx_US_TOP_PLL_PHASE_0     0x20fc
#define BCM31xx_US_TOP_BIAS_CTRL       0x2100
#define BCM31xx_US_ANALOG_CTRL_1       0x2104
#define BCM31xx_US_ANALOG_CTRL_0       0x2108
#define BCM31xx_TIMER_TIMER_IS         0x4080
#define BCM31xx_TIMER_TIMER_IE0        0x4084
#define BCM31xx_TIMER_WDTIMEOUT        0x40a8
#define BCM31xx_TIMER_WDCMD            0x40ac
#define BCM31xx_TIMER_WDCHIPRST_CNT    0x40b0
#define BCM31xx_TIMER_WDCRS            0x40b4
#define BCM31xx_TIMER_WDCTRL           0x40bc
#define BCM31xx_TM_TOP_GPIO_MUX_CTRL0  0x4818
#define BCM31xx_TM_TOP_GPIO_MUX_CTRL1  0x4814


#define BCM31xx_TC_MAIN_CONFIG         0x1000
#define BCM31xx_TC_SERIF_CONFIG        0x1024
#define BCM31xx_TC_SERIF_S2P_CONFIG    0x1020
#define BCM31xx_TC_SCR_PRBS_CFG        0x100c
#define BCM31xx_TC_BB0_RS_QAM          0x1100
#define BCM31xx_TC_BB0_PREAMBLE_LEN    0x1104
#define BCM31xx_TC_BB0_SCR_SEED        0x1108
#define BCM31xx_TC_PREAMBLE_RAM        0x1240

#define BCM31xx_US_TOP_CORE_EN         0x2044
#define BCM31xx_US_TOP_PDAC_EN         0x2048

#define BCM31xx_TC_STATUS_WORD_1       0x1300
#define BCM31xx_TC_STS_BURST_LENGTH    0x130c
#define BCM31xx_TC_STATUS_IFIFO_CNTS   0x1314
#define BCM31xx_TC_STATUS_BRST_TSYMB_REM 0x1318
#define BCM31xx_TC_NUMOF_CODEWORDS     0x131c
#define BCM31xx_TC_NUMOF_PYLD_SYMBOLS  0x1320
#define BCM31xx_TC_NUMOF_RS_SYMBOLS    0x1324
#define BCM31xx_TC_NUMOF_TOTAL_SYMBOLS 0x1328
#define BCM31xx_DS_TUNER_REF_DB0_WDATA_01 0x3101c
#define BCM31xx_DS_TUNER_ANACTL_WDATA_13_SPARE0   0x32038



/*****************************************************/

/* BCM31xx host registers */
#define BCM31xx_SH_SFR_H_LOCAL_ADR     0x0082
#define BCM31xx_SH_SFR_H_LOCAL_DAT     0x0083
#define BCM31xx_SH_SFR_H_ADR_16_15     0x0084
#define BCM31xx_SH_SFR_H_ADR_14_7      0x0085
#define BCM31xx_SH_SFR_IO_MBOX_STATUS  0x0086
#define BCM31xx_SH_SFR_IO_MBOX_A_15_8  0x0087
#define BCM31xx_SH_SFR_IO_MBOX_D_31_24 0x0088
#define BCM31xx_SH_SFR_IO_MBOX_D_23_16 0x0089
#define BCM31xx_SH_SFR_IO_MBOX_D_15_8  0x008a
#define BCM31xx_SH_SFR_IO_MBOX_D_7_0   0x008b
#define BCM31xx_SH_SFR_IO_MBOX_CMD     0x008c
#define BCM31xx_SH_SFR_IO_MBOX_A_31_24 0x008d
#define BCM31xx_SH_SFR_IO_MBOX_A_23_16 0x008e
#define BCM31xx_SH_AP_SFR_H_MSG1       0x0093
#define BCM31xx_SH_AP_SFR_H_MSG2       0x0094
#define BCM31xx_SH_AP_SFR_JDEC         0x0095
#define BCM31xx_SH_AP_SFR_H_CTL1       0x009a
#define BCM31xx_SH_AP_SFR_H_STAT1      0x009b
#define BCM31xx_SH_AP_SFR_H_STAT2      0x009c
#define BCM31xx_SH_AP_SFR_H_STAT3      0x009d
#define BCM31xx_SH_AP_SFR_H_STAT4      0x009e
#define BCM31xx_SH_AP_SFR_H_IE1        0x009f
#define BCM31xx_SH_AP_SFR_H_IE2        0x00a0
#define BCM31xx_SH_AP_SFR_H_IE3        0x00a1
#define BCM31xx_SH_AP_SFR_H_IE4        0x00a2
#define BCM31xx_SH_AP_SFR_H_FSTAT1     0x00a3
#define BCM31xx_SH_AP_SFR_H_FSTAT2     0x00a4
#define BCM31xx_SH_AP_SFR_H_FSTAT3     0x00a5
#define BCM31xx_SH_AP_SFR_H_FSTAT4     0x00a6
#define BCM31xx_SH_AP_SFR_GPIN_1       0x00a9
#define BCM31xx_SH_AP_SFR_GPIN_0       0x00aa
#define BCM31xx_SH_AP_SFR_GPOUT_1      0x00ab
#define BCM31xx_SH_AP_SFR_GPOUT_0      0x00ac
#define BCM31xx_SH_AP_SFR_GPIOENB_1    0x00ad
#define BCM31xx_SH_AP_SFR_GPIOENB_0    0x00ae
#define BCM31xx_SH_AP_SFR_GPIO_OW_1    0x00bc
#define BCM31xx_SH_AP_SFR_GPIO_OW_0    0x00bd
#define BCM31xx_SH_AP_SFR_HCTL_DBG     0x00be
#define BCM31xx_SH_AP_SFR_TMUX         0x00c9
#define BCM31xx_SH_AP_SFR_AP_CMD       0x00e4
#define BCM31xx_SH_AP_SFR_CP_CFG       0x00ef

/* BSC IO_MBOX registers */
#define BCM31xx_BSC_CHIP_ADDRESS       0x0800
#define BCM31xx_BSC_DATA_IN0_3         0x0804
#define BCM31xx_BSC_DATA_IN4_7         0x0808
#define BCM31xx_BSC_CNT_REG            0x080C
#define BCM31xx_BSC_CTL_REG            0x0810
#define BCM31xx_BSC_IIC_ENABLE         0x0814
#define BCM31xx_BSC_DATA_OUT0_3        0x0818
#define BCM31xx_BSC_DATA_OUT4_7        0x081C
#define BCM31xx_BSC_IRQEN              0x0820
#define BCM31xx_BSC_IRQSTS             0x0824

/* TM registers */
#define BCM31xx_TM_CLK_SEL             0x0000
#define BCM31xx_TM_SFT_RST             0x0004
#define BCM31xx_TM_OUTPUT_PAD_CTRL     0x0008
#define BCM31xx_TM_DS_OB_XPT_PAD_CTRL  0x000C
#define BCM31xx_TM_GPO_AUDIO_PAD_CTRL  0x0010
#define BCM31xx_TM_DIAG_CTRL           0x0014
#define BCM31xx_TM_MBIST_CTRL          0x0018
#define BCM31xx_TM_PWRDN_CTRL          0x001C
#define BCM31xx_TM_PIN_STRAP           0x0020
#define BCM31xx_TM_PIN_STRAP_OVRD      0x0024
#define BCM31xx_TM_SCAN_TRI_ISB        0x0028
#define BCM31xx_TM_SCAN_TRI_RBUS       0x002C
#define BCM31xx_TM_TMODE_CTRL          0x0030
#define BCM31xx_TM_OSC_CTRL_REG0       0x0034
#define BCM31xx_TM_OSC_CTRL_REG1       0x0038
#define BCM31xx_TM_DDAC_CTRL_REG       0x003C
#define BCM31xx_TM_GENPLL_CTRL_REG     0x0040
#define BCM31xx_TM_CHIP_ID             0x0044
#define BCM31xx_TM_CHIP_ID_INT         0x0048
#define BCM31xx_TM_TP_IN_CTRL          0x004C
#define BCM31xx_TM_TP_OUT_REG          0x0050
#define BCM31xx_TM_DAC_TEST_CTRL       0x0054
#define BCM31xx_TM_SS_AGC_HSYNC_REG    0x0058
#define BCM31xx_TM_SPARE_REGISTER_0    0x005C
#define BCM31xx_TM_SPARE_REGISTER_1    0x0060
 
/* H_CTL1 register bits */
#define BCM31xx_AP_RUN           0x00
#define BCM31xx_AP_RESET         0x01
#define BCM31xx_AP_IDLE          0x02
#define BCM31xx_AP_MASK          0x03
#define BCM31xx_AP_HABR          0x04
#define BCM31xx_AP_HAB_MASK      0x07
#define BCM31xx_AP_HAB_AVAILABLE 0x00

/* JDEC register bits */
#define BCM31xx_JDEC_EEPROM      0x00
#define BCM31xx_JDEC_ROM         0x01
#define BCM31xx_JDEC_RAM         0x02
#define BCM31xx_JDEC_EXTRAM      0x03

/* H_STAT1 register bits */
#define BCM31xx_STAT1_H_ER       0x80
#define BCM31xx_STAT1_IOMB_ER    0x40
#define BCM31xx_STAT1_MEM_ER     0x20
#define BCM31xx_STAT1_H_HAB_ER   0x10
#define BCM31xx_STAT1_HAB_DONE   0x04
#define BCM31xx_STAT1_IOMB_DONE  0x02
#define BCM31xx_STAT1_AP_OP_CHG  0x01
#define BCM31xx_STAT1_ERROR_MASK 0xF0

/* H_STAT2 register bits */
#define BCM31xx_STAT2_INIT_DONE   0x80
#define BCM31xx_STAT2_RT_LOCK     0x40
#define BCM31xx_STAT2_IN_LOCK     0x20
#define BCM31xx_STAT2_OUT_OF_LOCK 0x10   
#define BCM31xx_STAT2_LOCK_MASK   0x30  
#define BCM31xx_STAT2_HAB_ERROR   0x08
#define BCM31xx_STAT2_CRIT_ERROR  0x04
#define BCM31xx_STAT2_ANT         0x02
#define BCM31xx_STAT2_BSC         0x01
#define BCM31xx_STAT2_ERROR_MASK  0x0C

/* H_STAT3 register bits */
#define BCM31xx_STAT3_NO_SIGNAL   0x02
#define BCM31xx_STAT3_GET_GAIN    0x04
#define BCM31xx_STAT3_GAIN_MASK   0x06
#define BCM31xx_STAT3_IN_LOCK     0x08
#define BCM31xx_STAT3_OUT_OF_LOCK 0x10   
#define BCM31xx_STAT3_LOCK_MASK   0x18

#define BCM31xx_STAT3_QAM_STATUS_READY   0x20
#define BCM31xx_STAT3_OOB_STATUS_READY   0x40

#endif /* BCHP_31xx_H__ */
