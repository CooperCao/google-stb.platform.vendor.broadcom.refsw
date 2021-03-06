/****************************************************************************
 *     Copyright (c) 1999-2014, Broadcom Corporation
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
 * Date:           Generated on               Mon Nov 17 11:53:47 2014
 *                 Full Compile MD5 Checksum  7c712d144f2919fdac508431993b36d7
 *                     (minus title and desc)
 *                 MD5 Checksum               bc950b877a17d97d6325a810f8bd832e
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     15193
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *
 *
 ***************************************************************************/

#ifndef BCHP_HDMI_TX_PHY_1_H__
#define BCHP_HDMI_TX_PHY_1_H__

/***************************************************************************
 *HDMI_TX_PHY_1 - HDMI Tx Analog Phy
 ***************************************************************************/
#define BCHP_HDMI_TX_PHY_1_RESET_CTL             0x006d0a80 /* [RW] HDMI TX Analog Phy Reset Control */
#define BCHP_HDMI_TX_PHY_1_POWERDOWN_CTL         0x006d0a84 /* [RW] HDMI TX Analog Phy Powerdown Control */
#define BCHP_HDMI_TX_PHY_1_CTL_0                 0x006d0a88 /* [RW] HDMI TX Analog Phy Configuration and Control 0 */
#define BCHP_HDMI_TX_PHY_1_CTL_1                 0x006d0a8c /* [RW] HDMI TX Analog Phy Configuration and Control 1 */
#define BCHP_HDMI_TX_PHY_1_CTL_2                 0x006d0a90 /* [RW] HDMI TX Analog Phy Configuration and Control 2 */
#define BCHP_HDMI_TX_PHY_1_CTL_3                 0x006d0a94 /* [RW] HDMI TX Analog Phy Configuration and Control 3 */
#define BCHP_HDMI_TX_PHY_1_CTL_4                 0x006d0a98 /* [RW] HDMI TX Analog Phy Configuration and Control 4 */
#define BCHP_HDMI_TX_PHY_1_PLL_CTL_0             0x006d0a9c /* [RW] HDMI TX Analog Phy PLL Configuration and Control 0 */
#define BCHP_HDMI_TX_PHY_1_PLL_CTL_1             0x006d0aa0 /* [RW] HDMI TX Analog Phy PLL Configuration and Control 1 */
#define BCHP_HDMI_TX_PHY_1_PLL_CLK_DISABLE       0x006d0aa4 /* [RW] HDMI TX Analog Phy output clock enable */
#define BCHP_HDMI_TX_PHY_1_CLK_DIV               0x006d0aa8 /* [RW] HDMI TX Analog Phy output clock divider */
#define BCHP_HDMI_TX_PHY_1_SSC_CFG_0             0x006d0aac /* [RW] HDMI TX Analog Phy Spread Spectrum configuration register 0 */
#define BCHP_HDMI_TX_PHY_1_SSC_CFG_1             0x006d0ab0 /* [RW] HDMI TX Analog Phy Spread Spectrum configuration register 1 */
#define BCHP_HDMI_TX_PHY_1_PLL_CFG               0x006d0ab4 /* [RW] HDMI TX PLL Configuration Register */
#define BCHP_HDMI_TX_PHY_1_TMDS_CFG_0            0x006d0ab8 /* [RW] HDMI TX Phy TMDS Configuration Register 0 */
#define BCHP_HDMI_TX_PHY_1_TMDS_CFG_1            0x006d0abc /* [RW] HDMI TX Phy TMDS Configuration Register 1 */
#define BCHP_HDMI_TX_PHY_1_TMDS_CFG_2            0x006d0ac0 /* [RW] HDMI TX Phy TMDS Configuration Register 2 */
#define BCHP_HDMI_TX_PHY_1_TMDS_CLK_WORD_SEL     0x006d0ac4 /* [RW] HDMI TX TMDS CLK WORD select */
#define BCHP_HDMI_TX_PHY_1_STATUS                0x006d0ac8 /* [RO] HDMI TX Status register */
#define BCHP_HDMI_TX_PHY_1_CHANNEL_SWAP          0x006d0acc /* [RW] HDMI TX Phy Channel Swap and Invert Control Register. */
#define BCHP_HDMI_TX_PHY_1_CLK_MEASURE_TIMER     0x006d0ad0 /* [RW] HDMI TX Phy Clock Measure Timer Value Register. */
#define BCHP_HDMI_TX_PHY_1_CLK_MEASURE_CTL       0x006d0ad4 /* [RW] HDMI TX Phy Clock Measure Control Register. */
#define BCHP_HDMI_TX_PHY_1_CLK_MEASURED          0x006d0ad8 /* [RO] HDMI TX Phy Measured Clock Frequency Register. */
#define BCHP_HDMI_TX_PHY_1_CLK_MAX_MEASURED      0x006d0adc /* [RO] HDMI TX Phy Measured Maximum Clock Frequency Register. */
#define BCHP_HDMI_TX_PHY_1_CLK_MIN_MEASURED      0x006d0ae0 /* [RO] HDMI TX Phy Measured Minimum Clock Frequency Register. */
#define BCHP_HDMI_TX_PHY_1_RSEN_CTL              0x006d0ae4 /* [RW] HDMI TX PHY RSEN Control Register */
#define BCHP_HDMI_TX_PHY_1_RSEN_STATUS           0x006d0ae8 /* [RO] HDMI TX PHY RSEN Status Register */
#define BCHP_HDMI_TX_PHY_1_SPARE                 0x006d0aec /* [RW] Spare bits for ECO's. */

#endif /* #ifndef BCHP_HDMI_TX_PHY_1_H__ */

/* End of File */
