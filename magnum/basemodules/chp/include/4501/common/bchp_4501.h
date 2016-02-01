/***************************************************************************
 *     Copyright (c) 2005-2006, Broadcom Corporation
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

#ifndef BCHP_4501_H__
#define BCHP_4501_H__

/* BCM4501 host registers */
#define BCM4501_SH_SFR_H_ADR_16_15     0x0084
#define BCM4501_SH_SFR_H_ADR_14_7      0x0085
#define BCM4501_SH_SFR_IO_MBOX_STATUS  0x0086
#define BCM4501_SH_SFR_IO_MBOX_A_15_8  0x0087
#define BCM4501_SH_SFR_IO_MBOX_D_31_24 0x0088
#define BCM4501_SH_SFR_IO_MBOX_D_23_16 0x0089
#define BCM4501_SH_SFR_IO_MBOX_D_15_8  0x008a
#define BCM4501_SH_SFR_IO_MBOX_D_7_0   0x008b
#define BCM4501_SH_SFR_IO_MBOX_CMD     0x008c
#define BCM4501_SH_AP_SFR_H_MSG1       0x0093
#define BCM4501_SH_AP_SFR_H_MSG2       0x0094
#define BCM4501_SH_AP_SFR_JDEC         0x0095
#define BCM4501_SH_AP_SFR_H_CTL1       0x009a
#define BCM4501_SH_AP_SFR_H_STAT1      0x009b
#define BCM4501_SH_AP_SFR_H_STAT2      0x009c
#define BCM4501_SH_AP_SFR_H_STAT3      0x009d
#define BCM4501_SH_AP_SFR_H_STAT4      0x009e
#define BCM4501_SH_AP_SFR_H_IE1        0x009f
#define BCM4501_SH_AP_SFR_H_IE2        0x00a0
#define BCM4501_SH_AP_SFR_H_IE3        0x00a1
#define BCM4501_SH_AP_SFR_H_IE4        0x00a2
#define BCM4501_SH_AP_SFR_H_FSTAT1     0x00a3
#define BCM4501_SH_AP_SFR_H_FSTAT2     0x00a4
#define BCM4501_SH_AP_SFR_H_FSTAT3     0x00a5
#define BCM4501_SH_AP_SFR_H_FSTAT4     0x00a6
#define BCM4501_SH_AP_SFR_GPIN_1       0x00a9
#define BCM4501_SH_AP_SFR_GPIN_0       0x00aa
#define BCM4501_SH_AP_SFR_GPOUT_1      0x00ab
#define BCM4501_SH_AP_SFR_GPOUT_0      0x00ac
#define BCM4501_SH_AP_SFR_GPIOENB_1    0x00ad
#define BCM4501_SH_AP_SFR_GPIOENB_0    0x00ae
#define BCM4501_SH_AP_SFR_A_STAT1      0x00b0
#define BCM4501_SH_AP_SFR_A_STAT2      0x00b1
#define BCM4501_SH_AP_SFR_A_IE2        0x00b2
#define BCM4501_SH_AP_SFR_HABADR       0x00b9
#define BCM4501_SH_AP_SFR_HABDATA      0x00ba
#define BCM4501_SH_AP_SFR_GPIO_OW_1    0x00bc
#define BCM4501_SH_AP_SFR_GPIO_OW_0    0x00bd
#define BCM4501_SH_AP_SFR_HCTL_DBG     0x00be
#define BCM4501_SH_AP_SFR_WDTRST       0x00c5
#define BCM4501_SH_AP_SFR_WDTSTAT      0x00c6
#define BCM4501_SH_AP_SFR_TMUX         0x00c9
#define BCM4501_SH_AP_SFR_IRAM_PAGE    0x00ce
#define BCM4501_SH_AP_SFR_XDATA_PAGE   0x00cf
#define BCM4501_SH_AP_SFR_ISB_STATUS   0x00e2
#define BCM4501_SH_AP_SFR_AP_CMD       0x00e4
#define BCM4501_SH_AP_SFR_CP_CFG       0x00ef

/* TM registers */
#define BCM4501_TM_CLK_SEL             0x0000
#define BCM4501_TM_SFT_RST             0x0004
#define BCM4501_TM_PAD_CTRL            0x0008
#define BCM4501_TM_SDS_XPT_PAD_CTRL    0x000C
#define BCM4501_TM_TMODE_CTRL          0x0010
#define BCM4501_TM_TP_CTRL             0x0014
#define BCM4501_TM_TPOUT_REG           0x0018
#define BCM4501_TM_MBIST_CTRL          0x001C
#define BCM4501_TM_SCAN_TRI_ISB        0x0020
#define BCM4501_TM_SCAN_TRI_RBUS       0x0024
#define BCM4501_TM_PIN_STRAP           0x0028
#define BCM4501_TM_PIN_STRAP_OVRD      0x002C
#define BCM4501_TM_VREG_CTRL           0x0030
#define BCM4501_TM_OSC_CTRL_REG        0x0038
#define BCM4501_TM_CHIP_ID             0x003C
#define BCM4501_TM_CHIP_ID_INT         0x0040
#define BCM4501_TM_BOND_OPTION         0x0044
#define BCM4501_TM_AP_TIMER_CTRL       0x0048
#define BCM4501_TM_FTM_IRQ             0x004C
#define BCM4501_TM_AP_PLL_M2CNTL       0x0050
#define BCM4501_TM_SPARE_REGISTER_0    0x0054
#define BCM4501_TM_SPARE_REGISTER_1    0x0058
 
/* SDS registers */
#define BCM4501_SDS_EQSFT              0x0550

/* BB registers */
#define BCM4501_BB_CFG                 0x0400
#define BCM4501_BB_SER                 0x0404
#define BCM4501_BB_ISR                 0x0408
#define BCM4501_BB_IE                  0x040C
#define BCM4501_BB_T_PTRN              0x0414
#define BCM4501_BB_LOCK                0x0418
#define BCM4501_BB_TST                 0x041C
#define BCM4501_BB_ISB_ERR_STAT0       0x0420
#define BCM4501_BB_ISB_ERR_STAT1       0x0424
#define BCM4501_BB_ISB_ERR_STAT2       0x0428
#define BCM4501_BB_ISB_ERR_STAT_CLR    0x042C
#define BCM4501_BB_FISR                0x0430

/* H_CTL1 register bits */
#define BCM4501_AP_RUN           0x00
#define BCM4501_AP_RESET         0x01
#define BCM4501_AP_IDLE          0x02
#define BCM4501_AP_MASK          0x03
#define BCM4501_AP_HABR          0x04
#define BCM4501_AP_SOFT_RESET    0x10
#define BCM4501_AP_HAB_MASK      0x27
#define BCM4501_AP_HAB_AVAILABLE 0x20

/* JDEC register bits */
#define BCM4501_JDEC_ROM         (0x01 | 0x04)
#define BCM4501_JDEC_RAM         (0x02 | 0x04)
#define BCM4501_JDEC_EXTRAM      (0x03 | 0x04)
#define BCM4501_JDEC_RAM2        (BCM4501_JDEC_RAM | 0x10)

/* H_STAT1 register bits */
#define BCM4501_STAT1_H_ER       0x80
#define BCM4501_STAT1_IOMB_ER    0x40
#define BCM4501_STAT1_MEM_ER     0x20
#define BCM4501_STAT1_H_HAB_ER   0x10
#define BCM4501_STAT1_HAB_DONE   0x04
#define BCM4501_STAT1_IOMB_DONE  0x02
#define BCM4501_STAT1_AP_OP_CHG  0x01
#define BCM4501_STAT1_ERROR_MASK 0xF0

/* H_STAT2 register bits */
#define BCM4501_STAT2_INIT_DONE    0x80
#define BCM4501_STAT2_FTM          0x40
#define BCM4501_STAT2_FTM_ERROR    0x20
#define BCM4501_STAT2_TUNER_1_DONE 0x08
#define BCM4501_STAT2_TUNER_0_DONE 0x04
#define BCM4501_STAT2_HAB_ERROR    0x02
#define BCM4501_STAT2_CRIT_ERROR   0x01
#define BCM4501_STAT2_ERROR_MASK   0x23

/* H_STAT3 register bits */
#define BCM4501_STAT3_CH1_RT_LOCK  0x80
#define BCM4501_STAT3_CH1_LOCK     0x40
#define BCM4501_STAT3_CH1_NOT_LOCK 0x20
#define BCM4501_STAT3_CH1_DS_DONE  0x10
#define BCM4501_STAT3_CH0_RT_LOCK  0x08
#define BCM4501_STAT3_CH0_LOCK     0x04
#define BCM4501_STAT3_CH0_NOT_LOCK 0x02
#define BCM4501_STAT3_CH0_DS_DONE  0x01

/* H_STAT4 register bits */
#define BCM4501_STAT4_MI2C_ERROR_1 0x02
#define BCM4501_STAT4_MI2C_ERROR_0 0x01

#endif /* BCHP_4501_H__ */
