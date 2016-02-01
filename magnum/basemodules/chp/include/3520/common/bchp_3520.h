/***************************************************************************
 *     Copyright (c) 2003-2009, Broadcom Corporation
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

#ifndef BCHP_3520_H__
#define BCHP_3520_H__

/* BCM3520 host registers */
#define BCM3520_SH_SFR_H_ADR_16_15     0x0084
#define BCM3520_SH_SFR_H_ADR_14_7      0x0085
#define BCM3520_SH_SFR_IO_MBOX_STATUS  0x0086
#define BCM3520_SH_SFR_IO_MBOX_A_15_8  0x0087
#define BCM3520_SH_SFR_IO_MBOX_D_31_24 0x0088
#define BCM3520_SH_SFR_IO_MBOX_D_23_16 0x0089
#define BCM3520_SH_SFR_IO_MBOX_D_15_8  0x008a
#define BCM3520_SH_SFR_IO_MBOX_D_7_0   0x008b
#define BCM3520_SH_SFR_IO_MBOX_CMD     0x008c
#define BCM3520_SH_AP_SFR_H_MSG1       0x0093
#define BCM3520_SH_AP_SFR_H_MSG2       0x0094
#define BCM3520_SH_AP_SFR_JDEC         0x0095
#define BCM3520_SH_AP_SFR_H_CTL1       0x009a
#define BCM3520_SH_AP_SFR_H_STAT1      0x009b
#define BCM3520_SH_AP_SFR_H_STAT2      0x009c
#define BCM3520_SH_AP_SFR_H_STAT3      0x009d
#define BCM3520_SH_AP_SFR_H_STAT4      0x009e
#define BCM3520_SH_AP_SFR_H_IE1        0x009f
#define BCM3520_SH_AP_SFR_H_IE2        0x00a0
#define BCM3520_SH_AP_SFR_H_IE3        0x00a1
#define BCM3520_SH_AP_SFR_H_IE4        0x00a2
#define BCM3520_SH_AP_SFR_H_FSTAT1     0x00a3
#define BCM3520_SH_AP_SFR_H_FSTAT2     0x00a4
#define BCM3520_SH_AP_SFR_H_FSTAT3     0x00a5
#define BCM3520_SH_AP_SFR_H_FSTAT4     0x00a6
#define BCM3520_SH_AP_SFR_GPIN_1       0x00a9
#define BCM3520_SH_AP_SFR_GPIN_0       0x00aa
#define BCM3520_SH_AP_SFR_GPOUT_1      0x00ab
#define BCM3520_SH_AP_SFR_GPOUT_0      0x00ac
#define BCM3520_SH_AP_SFR_GPIOENB_1    0x00ad
#define BCM3520_SH_AP_SFR_GPIOENB_0    0x00ae
#define BCM3520_SH_AP_SFR_GPIO_OW_1    0x00bc
#define BCM3520_SH_AP_SFR_GPIO_OW_0    0x00bd
#define BCM3520_SH_AP_SFR_HCTL_DBG     0x00be
#define BCM3520_SH_AP_SFR_TMUX         0x00c9
#define BCM3520_SH_AP_SFR_AP_CMD       0x00e4
#define BCM3520_SH_AP_SFR_CP_CFG       0x00ef

/* BSC IO_MBOX registers */
#define BCM3520_BSC_CHIP_ADDRESS       0x0800
#define BCM3520_BSC_DATA_IN0_3         0x0804
#define BCM3520_BSC_DATA_IN4_7         0x0808
#define BCM3520_BSC_CNT_REG            0x080C
#define BCM3520_BSC_CTL_REG            0x0810
#define BCM3520_BSC_IIC_ENABLE         0x0814
#define BCM3520_BSC_DATA_OUT0_3        0x0818
#define BCM3520_BSC_DATA_OUT4_7        0x081C
#define BCM3520_BSC_IRQEN              0x0820
#define BCM3520_BSC_IRQSTS             0x0824

/* Antenna IO_MBOX registers */
#define BCM3520_ANT_CNTL               0x0900
#define BCM3520_ANT_TX_CNTL            0x0904
#define BCM3520_ANT_TX_LOGIC           0x0908
#define BCM3520_ANT_TX_DATA            0x090C
#define BCM3520_ANT_TX_2_TX_WAIT       0x0910
#define BCM3520_ANT_RX_LOGIC1          0x0914
#define BCM3520_ANT_RX_LOGIC0          0x0918
#define BCM3520_ANT_RX_PERIOD          0x091C
#define BCM3520_ANT_RX_STATUS          0x0920
#define BCM3520_ANT_RX_ST_COND         0x0924
#define BCM3520_ANT_IRQSTS             0x0928
#define BCM3520_ANT_IRQEN              0x092C

/* TM registers */
#define BCM3520_TM_CLK_SEL             0x0000
#define BCM3520_TM_SFT_RST             0x0004
#define BCM3520_TM_OUTPUT_PAD_CTRL     0x0008
#define BCM3520_TM_DS_OB_XPT_PAD_CTRL  0x000C
#define BCM3520_TM_GPO_AUDIO_PAD_CTRL  0x0010
#define BCM3520_TM_DIAG_CTRL           0x0014
#define BCM3520_TM_MBIST_CTRL          0x0018
#define BCM3520_TM_PWRDN_CTRL          0x001C
#define BCM3520_TM_PIN_STRAP           0x0020
#define BCM3520_TM_PIN_STRAP_OVRD      0x0024
#define BCM3520_TM_SCAN_TRI_ISB        0x0028
#define BCM3520_TM_SCAN_TRI_RBUS       0x002C
#define BCM3520_TM_TMODE_CTRL          0x0030
#define BCM3520_TM_OSC_CTRL_REG0       0x0034
#define BCM3520_TM_OSC_CTRL_REG1       0x0038
#define BCM3520_TM_DDAC_CTRL_REG       0x003C
#define BCM3520_TM_GENPLL_CTRL_REG     0x0040
#define BCM3520_TM_CHIP_ID             0x0044
#define BCM3520_TM_CHIP_ID_INT         0x0048
#define BCM3520_TM_TP_IN_CTRL          0x004C
#define BCM3520_TM_TP_OUT_REG          0x0050
#define BCM3520_TM_DAC_TEST_CTRL       0x0054
#define BCM3520_TM_SS_AGC_HSYNC_REG    0x0058
#define BCM3520_TM_SPARE_REGISTER_0    0x005C
#define BCM3520_TM_SPARE_REGISTER_1    0x0060
 
/* H_CTL1 register bits */
#define BCM3520_AP_RUN           0x00
#define BCM3520_AP_RESET         0x01
#define BCM3520_AP_IDLE          0x02
#define BCM3520_AP_MASK          0x03
#define BCM3520_AP_HABR          0x04
#define BCM3520_AP_HAB_MASK      0x07
#define BCM3520_AP_HAB_AVAILABLE 0x00

/* JDEC register bits */
#define BCM3520_JDEC_EEPROM      0x00
#define BCM3520_JDEC_ROM         0x01
#define BCM3520_JDEC_RAM         0x02
#define BCM3520_JDEC_EXTRAM      0x03

/* H_STAT1 register bits */
#define BCM3520_STAT1_H_ER       0x80
#define BCM3520_STAT1_IOMB_ER    0x40
#define BCM3520_STAT1_MEM_ER     0x20
#define BCM3520_STAT1_H_HAB_ER   0x10
#define BCM3520_STAT1_HAB_DONE   0x04
#define BCM3520_STAT1_IOMB_DONE  0x02
#define BCM3520_STAT1_AP_OP_CHG  0x01
#define BCM3520_STAT1_ERROR_MASK 0xF0

/* H_STAT2 register bits */
#define BCM3520_STAT2_INIT_DONE   0x80
#define BCM3520_STAT2_RT_LOCK     0x40
#define BCM3520_STAT2_IN_LOCK     0x20
#define BCM3520_STAT2_OUT_OF_LOCK 0x10   
#define BCM3520_STAT2_LOCK_MASK   0x30  
#define BCM3520_STAT2_HAB_ERROR   0x08
#define BCM3520_STAT2_CRIT_ERROR  0x04
#define BCM3520_STAT2_ANT         0x02
#define BCM3520_STAT2_BSC         0x01
#define BCM3520_STAT2_ERROR_MASK  0x0C

#define BCM3520_STAT3_ACQ_FINISHED		 0x01
#define BCM3520_STAT3_ACQ_OOB_FINISHED	 0x02
#define BCM3520_STAT3_OOB_LOCKED		 0x08
#define BCM3520_STAT3_OOB_NOT_LOCKED	 0x10
#define BCM3520_STAT3_OOB_LOCK_MASK      0x18
#define BCM3520_STAT3_OOB_RT_LOCK_STATUS 0x20


#endif /* BCHP_3520_H__ */
