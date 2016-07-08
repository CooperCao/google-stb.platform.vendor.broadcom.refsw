/****************************************************************************
 *     Copyright (c) 1999-2016, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Mon Feb  8 12:53:14 2016
 *                 Full Compile MD5 Checksum  7c463a9180016920b3e03273285ff33d
 *                     (minus title and desc)
 *                 MD5 Checksum               30fed0099690880293569d98807ed1d8
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     749
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_FTM_PHY_ANA_H__
#define BCHP_FTM_PHY_ANA_H__

/***************************************************************************
 *FTM_PHY_ANA - FTM PHY ANA Register Set
 ***************************************************************************/
#define BCHP_FTM_PHY_ANA_MISC                    0x01210200 /* [RW] MISC Control */
#define BCHP_FTM_PHY_ANA_CTL0_0                  0x01210204 /* [RW] Analog Control_0_0 */
#define BCHP_FTM_PHY_ANA_CTL0_1                  0x01210208 /* [RW] Analog Control_0_1 */

/***************************************************************************
 *MISC - MISC Control
 ***************************************************************************/
/* FTM_PHY_ANA :: MISC :: reserved0 [31:08] */
#define BCHP_FTM_PHY_ANA_MISC_reserved0_MASK                       0xffffff00
#define BCHP_FTM_PHY_ANA_MISC_reserved0_SHIFT                      8

/* FTM_PHY_ANA :: MISC :: CH0_FSKPHY_DAC_PWRDN [07:07] */
#define BCHP_FTM_PHY_ANA_MISC_CH0_FSKPHY_DAC_PWRDN_MASK            0x00000080
#define BCHP_FTM_PHY_ANA_MISC_CH0_FSKPHY_DAC_PWRDN_SHIFT           7
#define BCHP_FTM_PHY_ANA_MISC_CH0_FSKPHY_DAC_PWRDN_DEFAULT         0x00000000

/* FTM_PHY_ANA :: MISC :: CH0_FSKPHY_ADC_PWRDN [06:06] */
#define BCHP_FTM_PHY_ANA_MISC_CH0_FSKPHY_ADC_PWRDN_MASK            0x00000040
#define BCHP_FTM_PHY_ANA_MISC_CH0_FSKPHY_ADC_PWRDN_SHIFT           6
#define BCHP_FTM_PHY_ANA_MISC_CH0_FSKPHY_ADC_PWRDN_DEFAULT         0x00000000

/* FTM_PHY_ANA :: MISC :: CH0_TX_OUT_EDGE [05:05] */
#define BCHP_FTM_PHY_ANA_MISC_CH0_TX_OUT_EDGE_MASK                 0x00000020
#define BCHP_FTM_PHY_ANA_MISC_CH0_TX_OUT_EDGE_SHIFT                5
#define BCHP_FTM_PHY_ANA_MISC_CH0_TX_OUT_EDGE_DEFAULT              0x00000000

/* FTM_PHY_ANA :: MISC :: reserved1 [04:04] */
#define BCHP_FTM_PHY_ANA_MISC_reserved1_MASK                       0x00000010
#define BCHP_FTM_PHY_ANA_MISC_reserved1_SHIFT                      4

/* FTM_PHY_ANA :: MISC :: FSKPHY_DAC_DYNAMIC_PWR_CTL [03:02] */
#define BCHP_FTM_PHY_ANA_MISC_FSKPHY_DAC_DYNAMIC_PWR_CTL_MASK      0x0000000c
#define BCHP_FTM_PHY_ANA_MISC_FSKPHY_DAC_DYNAMIC_PWR_CTL_SHIFT     2
#define BCHP_FTM_PHY_ANA_MISC_FSKPHY_DAC_DYNAMIC_PWR_CTL_DEFAULT   0x00000001

/* FTM_PHY_ANA :: MISC :: reserved2 [01:01] */
#define BCHP_FTM_PHY_ANA_MISC_reserved2_MASK                       0x00000002
#define BCHP_FTM_PHY_ANA_MISC_reserved2_SHIFT                      1

/* FTM_PHY_ANA :: MISC :: DIGITAL_CLK108_CTL [00:00] */
#define BCHP_FTM_PHY_ANA_MISC_DIGITAL_CLK108_CTL_MASK              0x00000001
#define BCHP_FTM_PHY_ANA_MISC_DIGITAL_CLK108_CTL_SHIFT             0
#define BCHP_FTM_PHY_ANA_MISC_DIGITAL_CLK108_CTL_DEFAULT           0x00000000

/***************************************************************************
 *CTL0_0 - Analog Control_0_0
 ***************************************************************************/
/* FTM_PHY_ANA :: CTL0_0 :: reserved0 [31:31] */
#define BCHP_FTM_PHY_ANA_CTL0_0_reserved0_MASK                     0x80000000
#define BCHP_FTM_PHY_ANA_CTL0_0_reserved0_SHIFT                    31

/* FTM_PHY_ANA :: CTL0_0 :: TXDAC_ctl [30:29] */
#define BCHP_FTM_PHY_ANA_CTL0_0_TXDAC_ctl_MASK                     0x60000000
#define BCHP_FTM_PHY_ANA_CTL0_0_TXDAC_ctl_SHIFT                    29
#define BCHP_FTM_PHY_ANA_CTL0_0_TXDAC_ctl_DEFAULT                  0x00000000

/* FTM_PHY_ANA :: CTL0_0 :: Ifs_ctrl [28:21] */
#define BCHP_FTM_PHY_ANA_CTL0_0_Ifs_ctrl_MASK                      0x1fe00000
#define BCHP_FTM_PHY_ANA_CTL0_0_Ifs_ctrl_SHIFT                     21
#define BCHP_FTM_PHY_ANA_CTL0_0_Ifs_ctrl_DEFAULT                   0x00000000

/* FTM_PHY_ANA :: CTL0_0 :: rstb_crc_lfsr_saw [20:20] */
#define BCHP_FTM_PHY_ANA_CTL0_0_rstb_crc_lfsr_saw_MASK             0x00100000
#define BCHP_FTM_PHY_ANA_CTL0_0_rstb_crc_lfsr_saw_SHIFT            20
#define BCHP_FTM_PHY_ANA_CTL0_0_rstb_crc_lfsr_saw_DEFAULT          0x00000000

/* FTM_PHY_ANA :: CTL0_0 :: en_lfsr [19:19] */
#define BCHP_FTM_PHY_ANA_CTL0_0_en_lfsr_MASK                       0x00080000
#define BCHP_FTM_PHY_ANA_CTL0_0_en_lfsr_SHIFT                      19
#define BCHP_FTM_PHY_ANA_CTL0_0_en_lfsr_DEFAULT                    0x00000000

/* FTM_PHY_ANA :: CTL0_0 :: en_crc [18:18] */
#define BCHP_FTM_PHY_ANA_CTL0_0_en_crc_MASK                        0x00040000
#define BCHP_FTM_PHY_ANA_CTL0_0_en_crc_SHIFT                       18
#define BCHP_FTM_PHY_ANA_CTL0_0_en_crc_DEFAULT                     0x00000000

/* FTM_PHY_ANA :: CTL0_0 :: test_data [17:06] */
#define BCHP_FTM_PHY_ANA_CTL0_0_test_data_MASK                     0x0003ffc0
#define BCHP_FTM_PHY_ANA_CTL0_0_test_data_SHIFT                    6
#define BCHP_FTM_PHY_ANA_CTL0_0_test_data_DEFAULT                  0x00000000

/* FTM_PHY_ANA :: CTL0_0 :: en_saw [05:05] */
#define BCHP_FTM_PHY_ANA_CTL0_0_en_saw_MASK                        0x00000020
#define BCHP_FTM_PHY_ANA_CTL0_0_en_saw_SHIFT                       5
#define BCHP_FTM_PHY_ANA_CTL0_0_en_saw_DEFAULT                     0x00000000

/* FTM_PHY_ANA :: CTL0_0 :: DAC_insel [04:03] */
#define BCHP_FTM_PHY_ANA_CTL0_0_DAC_insel_MASK                     0x00000018
#define BCHP_FTM_PHY_ANA_CTL0_0_DAC_insel_SHIFT                    3
#define BCHP_FTM_PHY_ANA_CTL0_0_DAC_insel_DEFAULT                  0x00000000

/* FTM_PHY_ANA :: CTL0_0 :: BG_adj [02:00] */
#define BCHP_FTM_PHY_ANA_CTL0_0_BG_adj_MASK                        0x00000007
#define BCHP_FTM_PHY_ANA_CTL0_0_BG_adj_SHIFT                       0
#define BCHP_FTM_PHY_ANA_CTL0_0_BG_adj_DEFAULT                     0x00000000

/***************************************************************************
 *CTL0_1 - Analog Control_0_1
 ***************************************************************************/
/* FTM_PHY_ANA :: CTL0_1 :: RXADC_ctrl [31:10] */
#define BCHP_FTM_PHY_ANA_CTL0_1_RXADC_ctrl_MASK                    0xfffffc00
#define BCHP_FTM_PHY_ANA_CTL0_1_RXADC_ctrl_SHIFT                   10
#define BCHP_FTM_PHY_ANA_CTL0_1_RXADC_ctrl_DEFAULT                 0x00000000

/* FTM_PHY_ANA :: CTL0_1 :: RXPGA_gain [09:07] */
#define BCHP_FTM_PHY_ANA_CTL0_1_RXPGA_gain_MASK                    0x00000380
#define BCHP_FTM_PHY_ANA_CTL0_1_RXPGA_gain_SHIFT                   7
#define BCHP_FTM_PHY_ANA_CTL0_1_RXPGA_gain_DEFAULT                 0x00000000

/* FTM_PHY_ANA :: CTL0_1 :: reserved0 [06:00] */
#define BCHP_FTM_PHY_ANA_CTL0_1_reserved0_MASK                     0x0000007f
#define BCHP_FTM_PHY_ANA_CTL0_1_reserved0_SHIFT                    0

#endif /* #ifndef BCHP_FTM_PHY_ANA_H__ */

/* End of File */
