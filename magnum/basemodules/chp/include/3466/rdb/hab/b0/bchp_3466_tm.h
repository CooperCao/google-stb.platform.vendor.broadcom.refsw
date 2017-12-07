/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/
#ifndef BCHP_TM_H__
#define BCHP_TM_H__

/***************************************************************************
 *TM - TM registers
 ***************************************************************************/
#define BCHP_TM_FAMILY_ID                        0x04820400 /* [RO][32] Chip Family ID */
#define BCHP_TM_PRODUCT_ID                       0x04820404 /* [RO][32] Product ID */
#define BCHP_TM_SFT_RST0                         0x04820410 /* [RW][32] Soft Reset Control Register 0 */
#define BCHP_TM_SFT_RST_CFG0                     0x04820414 /* [RW][32] Soft Reset Configuration Control Register 0 */
#define BCHP_TM_SFT_RST1                         0x04820418 /* [RW][32] Soft Reset Control Register 1 */
#define BCHP_TM_SFT_RST_CFG1                     0x0482041c /* [RW][32] Soft Reset Configuration Control Register 1 */
#define BCHP_TM_CLOCK_ENABLE_CTRL1               0x04820420 /* [RW][32] Clock Enable Control Register 1 */
#define BCHP_TM_AVS_OVERTEMP_CTRL                0x04820434 /* [RW][32] AVS Over Temperature Control */
#define BCHP_TM_UART_ROUTER_SEL                  0x04820444 /* [RW][32] UART Router select */
#define BCHP_TM_EJTAG_INPUT_EN                   0x04820448 /* [RW][32] EJTAG input bus enables */
#define BCHP_TM_EJTAG_INPUT_SEL                  0x0482044c /* [RW][32] EJTAG input select */
#define BCHP_TM_EJTAG_OUTPUT_SEL                 0x04820450 /* [RW][32] EJTAG output select */
#define BCHP_TM_MTSIF0_CTRL                      0x04820460 /* [RW][32] MTSIF0 Pad Control Register */
#define BCHP_TM_MTSIF_RX_CTRL                    0x04820464 /* [RW][32] MTSIF_RX Pad Control Register */
#define BCHP_TM_TEST_MODE                        0x04820520 /* [RW][32] Test Mode Control Register */
#define BCHP_TM_TEST_MISC2                       0x04820524 /* [RW][32] Test Misc2 Control Register */
#define BCHP_TM_TEST_MISC1                       0x04820528 /* [RW][32] Test Misc1 Control Register */
#define BCHP_TM_TEST_MISC0                       0x0482052c /* [RW][32] Test Misc0 Control Register */
#define BCHP_TM_UART_RX_CTRL                     0x04820530 /* [RW][32] UART Control Register */
#define BCHP_TM_GPIO_IODIR                       0x04820540 /* [RW][32] GPIO IO Directional Control Register */
#define BCHP_TM_GPIO_OD                          0x04820544 /* [RW][32] GPIO Open Drain Control Register */
#define BCHP_TM_GPIO_DATA                        0x04820548 /* [RW][32] GPIO Data Control Register */
#define BCHP_TM_GPIO_MUX_CTRL1                   0x0482054c /* [RW][32] GPIO Multiplexing Control 1 Register */
#define BCHP_TM_GPIO_MUX_CTRL2                   0x04820550 /* [RW][32] GPIO Multiplexing Control 2 Register */
#define BCHP_TM_GPIO_MUX_CTRL3                   0x04820554 /* [RW][32] GPIO Multiplexing Control 3 Register */
#define BCHP_TM_GPO_EN                           0x04820560 /* [RW][32] GPO Output Enable Control Register */
#define BCHP_TM_GPO_OD                           0x04820564 /* [RW][32] GPO Open Drain Control Register */
#define BCHP_TM_GPO_DATA                         0x04820568 /* [RW][32] GPO Data Control Register */
#define BCHP_TM_GPO_MUX_CTRL1                    0x0482056c /* [RW][32] GPO Multiplexing Control 1 Register */
#define BCHP_TM_GPO_MUX_CTRL2                    0x04820570 /* [RW][32] GPO Multiplexing Control 2 Register */
#define BCHP_TM_GPO_MUX_CTRL3                    0x04820574 /* [RW][32] GPO Multiplexing Control 3 Register */
#define BCHP_TM_IRQ_CTRL                         0x04820578 /* [RW][32] IRQ Control Register */
#define BCHP_TM_BSC0_CTRL                        0x04820580 /* [RW][32] Master BSC0 Control Register */
#define BCHP_TM_BSC1_CTRL                        0x04820584 /* [RW][32] Master BSC1 Control Register */
#define BCHP_TM_MISC0                            0x048205a8 /* [RW][32] Misc Control Register */
#define BCHP_TM_SFT7                             0x048205ac /* [RW][32] Software Control Register */
#define BCHP_TM_SFT6                             0x048205b0 /* [RW][32] Software Control Register */
#define BCHP_TM_SFT5                             0x048205b4 /* [RW][32] Software Control Register */
#define BCHP_TM_SFT4                             0x048205b8 /* [RW][32] Software Control Register */
#define BCHP_TM_SFT3                             0x048205bc /* [RW][32] Software Control Register */
#define BCHP_TM_SFT2                             0x048205c0 /* [RW][32] Software Control Register */
#define BCHP_TM_SFT1                             0x048205c4 /* [RW][32] Software Control Register */
#define BCHP_TM_SFT0                             0x048205c8 /* [RW][32] Software Control Register */
#define BCHP_TM_TP_DIAG_CTRL                     0x048205cc /* [RW][32] Testport Diagnostic Control Register */
#define BCHP_TM_INT_DIAG_MUX_CTRL8               0x048205d0 /* [RW][32] Internal Diagnostic Mux Control Register */
#define BCHP_TM_INT_DIAG_MUX_CTRL7               0x048205d4 /* [RW][32] Internal Diagnostic Mux Control Register */
#define BCHP_TM_INT_DIAG_MUX_CTRL6               0x048205d8 /* [RW][32] Internal Diagnostic Mux Control Register */
#define BCHP_TM_INT_DIAG_MUX_CTRL5               0x048205dc /* [RW][32] Internal Diagnostic Mux Control Register */
#define BCHP_TM_INT_DIAG_MUX_CTRL4               0x048205e0 /* [RW][32] Internal Diagnostic Mux Control Register */
#define BCHP_TM_INT_DIAG_MUX_CTRL3               0x048205e4 /* [RW][32] Internal Diagnostic Mux Control Register */
#define BCHP_TM_INT_DIAG_MUX_CTRL2               0x048205e8 /* [RW][32] Internal Diagnostic Mux Control Register */
#define BCHP_TM_INT_DIAG_MUX_CTRL1               0x048205ec /* [RW][32] Internal Diagnostic Mux Control Register */
#define BCHP_TM_INT_DIAG_MUX_CTRL0               0x048205f0 /* [RW][32] Internal Diagnostic Mux Control Register */
#define BCHP_TM_INT_DIAG_INV_CTRL1               0x048205f4 /* [RW][32] Internal Diagnostic Inversion Control Register */
#define BCHP_TM_INT_DIAG_INV_CTRL0               0x048205f8 /* [RW][32] Internal Diagnostic Inversion Control Register */
#define BCHP_TM_EXT_DIAG_MUX_CTRL8               0x048205fc /* [RW][32] External Diagnostic Mux Control Register */
#define BCHP_TM_EXT_DIAG_MUX_CTRL7               0x04820600 /* [RW][32] External Diagnostic Mux Control Register */
#define BCHP_TM_EXT_DIAG_MUX_CTRL6               0x04820604 /* [RW][32] External Diagnostic Mux Control Register */
#define BCHP_TM_EXT_DIAG_MUX_CTRL5               0x04820608 /* [RW][32] External Diagnostic Mux Control Register */
#define BCHP_TM_EXT_DIAG_MUX_CTRL4               0x0482060c /* [RW][32] External Diagnostic Mux Control Register */
#define BCHP_TM_EXT_DIAG_MUX_CTRL3               0x04820610 /* [RW][32] External Diagnostic Mux Control Register */
#define BCHP_TM_EXT_DIAG_MUX_CTRL2               0x04820614 /* [RW][32] External Diagnostic Mux Control Register */
#define BCHP_TM_EXT_DIAG_MUX_CTRL1               0x04820618 /* [RW][32] External Diagnostic Mux Control Register */
#define BCHP_TM_EXT_DIAG_MUX_CTRL0               0x0482061c /* [RW][32] External Diagnostic Mux Control Register */
#define BCHP_TM_EXT_DIAG_INV_CTRL1               0x04820620 /* [RW][32] External Diagnostic Inversion Control Register */
#define BCHP_TM_EXT_DIAG_INV_CTRL0               0x04820624 /* [RW][32] External Diagnostic Inversion Control Register */
#define BCHP_TM_EXT_DIAG_TP_OUT_READ             0x04820628 /* [RO][32] Ext. Diag. / TP Out Read Control Register */
#define BCHP_TM_RO_TEST_CTRL0                    0x0482062c /* [RW][32] RO Test Enable Control Register 0 */
#define BCHP_TM_RO_TEST_CTRL1                    0x04820630 /* [RW][32] RO Test Enable Control Register 1 */
#define BCHP_TM_DETECT2P5_CTRL                   0x04820634 /* [RW][32] DETECT2P5 Control Register */
#define BCHP_TM_GPIO_READ                        0x04820638 /* [RO][32] GPIO Read Control Register */
#define BCHP_TM_GPO_READ                         0x0482063c /* [RO][32] GPO Read Control Register */
#define BCHP_TM_SF_READ                          0x04820644 /* [RO][32] SF Read Control Register */
#define BCHP_TM_BSC_READ                         0x04820648 /* [RO][32] BSC Read Control Register */
#define BCHP_TM_EJTAG_READ                       0x0482064c /* [RO][32] EJTAG Read Control Register */
#define BCHP_TM_MISC_READ                        0x04820650 /* [RO][32] Misc Read Control Register */
#define BCHP_TM_PIN_STRAP                        0x04820654 /* [RW][32] Pin Strap Read Control Register */
#define BCHP_TM_DIAG_OUT1                        0x04820658 /* [RO][32] Testport / Diagnostic Read Control Register */
#define BCHP_TM_DIAG_OUT0                        0x0482065c /* [RO][32] Testport / Diagnostic Read Control Register */
#define BCHP_TM_TP_IN_VAL_SEL                    0x04820660 /* [RW][32] TP_IN Enable Control Register */
#define BCHP_TM_TP_IN_VAL                        0x04820664 /* [RW][32] TP_IN Value Control Register */
#define BCHP_TM_DAC_TEST_MODE_CTRL               0x04820668 /* [RW][32] DAC_TEST_MODE_CTRL Control Register */
#define BCHP_TM_TEST_CTRL                        0x04820688 /* [RW][32] Test Control Register */
#define BCHP_TM_WAKEUP_CTRL                      0x04820690 /* [RW][32] Wakeup Control Register */
#define BCHP_TM_RESET_CTRL                       0x04820698 /* [RW][32] Chip Generated Reset Control Register */
#define BCHP_TM_RESET_COUNT                      0x0482069c /* [RW][32] Chip Reset Count Register */
#define BCHP_TM_DEMOD_XPT_CTRL_0                 0x048206e0 /* [RW][32] DEMOD_XPT Control Register 0 */
#define BCHP_TM_DEMOD_XPT_CTRL_1                 0x048206e4 /* [RW][32] DEMOD_XPT Control Register 1 */
#define BCHP_TM_ICID_DATA7                       0x048206e8 /* [RO][32] ICID Data Register */
#define BCHP_TM_ICID_DATA6                       0x048206ec /* [RO][32] ICID Data Register */
#define BCHP_TM_ICID_DATA5                       0x048206f0 /* [RO][32] ICID Data Register */
#define BCHP_TM_ICID_DATA4                       0x048206f4 /* [RO][32] ICID Data Register */
#define BCHP_TM_ICID_DATA3                       0x048206f8 /* [RO][32] ICID Data Register */
#define BCHP_TM_ICID_DATA2                       0x048206fc /* [RO][32] ICID Data Register */
#define BCHP_TM_ICID_DATA1                       0x04820700 /* [RO][32] ICID Data Register */
#define BCHP_TM_ICID_DATA0                       0x04820704 /* [RO][32] ICID Data Register */
#define BCHP_TM_ICID_READ                        0x04820708 /* [RW][32] ICID Read Register */
#define BCHP_TM_ICID_CLK_CTRL                    0x0482070c /* [RW][32] ICID Clock Register */
#define BCHP_TM_ICID_MISC_CTRL                   0x04820710 /* [RW][32] ICID Miscellaneous Control Register */
#define BCHP_TM_SPARE_REG0                       0x04820714 /* [RW][32] Spare Register 0 */
#define BCHP_TM_ANA_XTAL_CONTROL                 0x04820720 /* [RW][32] Xtal Control */
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL         0x04820724 /* [RW][32] Xtal External CML control */
#define BCHP_TM_MEMORY_POWER_CTRL_0              0x04820728 /* [RW][32] MEMORY POWER Control Register 0 */
#define BCHP_TM_MEMORY_POWER_CTRL_1              0x0482072c /* [RW][32] MEMORY POWER Control Register 1 */
#define BCHP_TM_MEMORY_POWER_CTRL_2              0x04820730 /* [RO][32] MEMORY POWER Control Register 2 */

/***************************************************************************
 *FAMILY_ID - Chip Family ID
 ***************************************************************************/
/* TM :: FAMILY_ID :: FAMILY_ID [31:00] */
#define BCHP_TM_FAMILY_ID_FAMILY_ID_MASK                           0xffffffff
#define BCHP_TM_FAMILY_ID_FAMILY_ID_SHIFT                          0
#define BCHP_TM_FAMILY_ID_FAMILY_ID_DEFAULT                        0x34660010

/***************************************************************************
 *PRODUCT_ID - Product ID
 ***************************************************************************/
/* TM :: PRODUCT_ID :: PRODUCT_ID [31:00] */
#define BCHP_TM_PRODUCT_ID_PRODUCT_ID_MASK                         0xffffffff
#define BCHP_TM_PRODUCT_ID_PRODUCT_ID_SHIFT                        0
#define BCHP_TM_PRODUCT_ID_PRODUCT_ID_DEFAULT                      0x34660010

/***************************************************************************
 *SFT_RST0 - Soft Reset Control Register 0
 ***************************************************************************/
/* TM :: SFT_RST0 :: RSVD [31:14] */
#define BCHP_TM_SFT_RST0_RSVD_MASK                                 0xffffc000
#define BCHP_TM_SFT_RST0_RSVD_SHIFT                                14
#define BCHP_TM_SFT_RST0_RSVD_DEFAULT                              0x00000000

/* TM :: SFT_RST0 :: OVERTEMP_RESET_ISLAND_SW_INIT [13:13] */
#define BCHP_TM_SFT_RST0_OVERTEMP_RESET_ISLAND_SW_INIT_MASK        0x00002000
#define BCHP_TM_SFT_RST0_OVERTEMP_RESET_ISLAND_SW_INIT_SHIFT       13
#define BCHP_TM_SFT_RST0_OVERTEMP_RESET_ISLAND_SW_INIT_DEFAULT     0x00000001

/* TM :: SFT_RST0 :: DIAG_CAPT [12:12] */
#define BCHP_TM_SFT_RST0_DIAG_CAPT_MASK                            0x00001000
#define BCHP_TM_SFT_RST0_DIAG_CAPT_SHIFT                           12
#define BCHP_TM_SFT_RST0_DIAG_CAPT_DEFAULT                         0x00000001

/* TM :: SFT_RST0 :: TMT [11:11] */
#define BCHP_TM_SFT_RST0_TMT_MASK                                  0x00000800
#define BCHP_TM_SFT_RST0_TMT_SHIFT                                 11
#define BCHP_TM_SFT_RST0_TMT_DEFAULT                               0x00000001

/* TM :: SFT_RST0 :: OTP [10:10] */
#define BCHP_TM_SFT_RST0_OTP_MASK                                  0x00000400
#define BCHP_TM_SFT_RST0_OTP_SHIFT                                 10
#define BCHP_TM_SFT_RST0_OTP_DEFAULT                               0x00000001

/* TM :: SFT_RST0 :: SBAC [09:09] */
#define BCHP_TM_SFT_RST0_SBAC_MASK                                 0x00000200
#define BCHP_TM_SFT_RST0_SBAC_SHIFT                                9
#define BCHP_TM_SFT_RST0_SBAC_DEFAULT                              0x00000001

/* TM :: SFT_RST0 :: MBAC [08:08] */
#define BCHP_TM_SFT_RST0_MBAC_MASK                                 0x00000100
#define BCHP_TM_SFT_RST0_MBAC_SHIFT                                8
#define BCHP_TM_SFT_RST0_MBAC_DEFAULT                              0x00000001

/* TM :: SFT_RST0 :: MBSC1 [07:07] */
#define BCHP_TM_SFT_RST0_MBSC1_MASK                                0x00000080
#define BCHP_TM_SFT_RST0_MBSC1_SHIFT                               7
#define BCHP_TM_SFT_RST0_MBSC1_DEFAULT                             0x00000001

/* TM :: SFT_RST0 :: MBSC0 [06:06] */
#define BCHP_TM_SFT_RST0_MBSC0_MASK                                0x00000040
#define BCHP_TM_SFT_RST0_MBSC0_SHIFT                               6
#define BCHP_TM_SFT_RST0_MBSC0_DEFAULT                             0x00000001

/* TM :: SFT_RST0 :: WDT [05:05] */
#define BCHP_TM_SFT_RST0_WDT_MASK                                  0x00000020
#define BCHP_TM_SFT_RST0_WDT_SHIFT                                 5
#define BCHP_TM_SFT_RST0_WDT_DEFAULT                               0x00000001

/* TM :: SFT_RST0 :: UPG [04:04] */
#define BCHP_TM_SFT_RST0_UPG_MASK                                  0x00000010
#define BCHP_TM_SFT_RST0_UPG_SHIFT                                 4
#define BCHP_TM_SFT_RST0_UPG_DEFAULT                               0x00000001

/* TM :: SFT_RST0 :: TIMERS [03:03] */
#define BCHP_TM_SFT_RST0_TIMERS_MASK                               0x00000008
#define BCHP_TM_SFT_RST0_TIMERS_SHIFT                              3
#define BCHP_TM_SFT_RST0_TIMERS_DEFAULT                            0x00000001

/* TM :: SFT_RST0 :: DEMOD_XPT [02:02] */
#define BCHP_TM_SFT_RST0_DEMOD_XPT_MASK                            0x00000004
#define BCHP_TM_SFT_RST0_DEMOD_XPT_SHIFT                           2
#define BCHP_TM_SFT_RST0_DEMOD_XPT_DEFAULT                         0x00000001

/* TM :: SFT_RST0 :: AVS [01:01] */
#define BCHP_TM_SFT_RST0_AVS_MASK                                  0x00000002
#define BCHP_TM_SFT_RST0_AVS_SHIFT                                 1
#define BCHP_TM_SFT_RST0_AVS_DEFAULT                               0x00000001

/* TM :: SFT_RST0 :: MTSIF [00:00] */
#define BCHP_TM_SFT_RST0_MTSIF_MASK                                0x00000001
#define BCHP_TM_SFT_RST0_MTSIF_SHIFT                               0
#define BCHP_TM_SFT_RST0_MTSIF_DEFAULT                             0x00000001

/***************************************************************************
 *SFT_RST_CFG0 - Soft Reset Configuration Control Register 0
 ***************************************************************************/
/* TM :: SFT_RST_CFG0 :: RSVD [31:14] */
#define BCHP_TM_SFT_RST_CFG0_RSVD_MASK                             0xffffc000
#define BCHP_TM_SFT_RST_CFG0_RSVD_SHIFT                            14
#define BCHP_TM_SFT_RST_CFG0_RSVD_DEFAULT                          0x00000000

/* TM :: SFT_RST_CFG0 :: OVERTEMP_RESET_ISLAND_SW_INIT [13:13] */
#define BCHP_TM_SFT_RST_CFG0_OVERTEMP_RESET_ISLAND_SW_INIT_MASK    0x00002000
#define BCHP_TM_SFT_RST_CFG0_OVERTEMP_RESET_ISLAND_SW_INIT_SHIFT   13
#define BCHP_TM_SFT_RST_CFG0_OVERTEMP_RESET_ISLAND_SW_INIT_DEFAULT 0x00000001

/* TM :: SFT_RST_CFG0 :: DIAG_CAPT [12:12] */
#define BCHP_TM_SFT_RST_CFG0_DIAG_CAPT_MASK                        0x00001000
#define BCHP_TM_SFT_RST_CFG0_DIAG_CAPT_SHIFT                       12
#define BCHP_TM_SFT_RST_CFG0_DIAG_CAPT_DEFAULT                     0x00000001

/* TM :: SFT_RST_CFG0 :: TMT [11:11] */
#define BCHP_TM_SFT_RST_CFG0_TMT_MASK                              0x00000800
#define BCHP_TM_SFT_RST_CFG0_TMT_SHIFT                             11
#define BCHP_TM_SFT_RST_CFG0_TMT_DEFAULT                           0x00000001

/* TM :: SFT_RST_CFG0 :: OTP [10:10] */
#define BCHP_TM_SFT_RST_CFG0_OTP_MASK                              0x00000400
#define BCHP_TM_SFT_RST_CFG0_OTP_SHIFT                             10
#define BCHP_TM_SFT_RST_CFG0_OTP_DEFAULT                           0x00000001

/* TM :: SFT_RST_CFG0 :: SBAC [09:09] */
#define BCHP_TM_SFT_RST_CFG0_SBAC_MASK                             0x00000200
#define BCHP_TM_SFT_RST_CFG0_SBAC_SHIFT                            9
#define BCHP_TM_SFT_RST_CFG0_SBAC_DEFAULT                          0x00000001

/* TM :: SFT_RST_CFG0 :: MBAC [08:08] */
#define BCHP_TM_SFT_RST_CFG0_MBAC_MASK                             0x00000100
#define BCHP_TM_SFT_RST_CFG0_MBAC_SHIFT                            8
#define BCHP_TM_SFT_RST_CFG0_MBAC_DEFAULT                          0x00000001

/* TM :: SFT_RST_CFG0 :: MBSC1 [07:07] */
#define BCHP_TM_SFT_RST_CFG0_MBSC1_MASK                            0x00000080
#define BCHP_TM_SFT_RST_CFG0_MBSC1_SHIFT                           7
#define BCHP_TM_SFT_RST_CFG0_MBSC1_DEFAULT                         0x00000001

/* TM :: SFT_RST_CFG0 :: MBSC0 [06:06] */
#define BCHP_TM_SFT_RST_CFG0_MBSC0_MASK                            0x00000040
#define BCHP_TM_SFT_RST_CFG0_MBSC0_SHIFT                           6
#define BCHP_TM_SFT_RST_CFG0_MBSC0_DEFAULT                         0x00000001

/* TM :: SFT_RST_CFG0 :: WDT [05:05] */
#define BCHP_TM_SFT_RST_CFG0_WDT_MASK                              0x00000020
#define BCHP_TM_SFT_RST_CFG0_WDT_SHIFT                             5
#define BCHP_TM_SFT_RST_CFG0_WDT_DEFAULT                           0x00000001

/* TM :: SFT_RST_CFG0 :: UPG [04:04] */
#define BCHP_TM_SFT_RST_CFG0_UPG_MASK                              0x00000010
#define BCHP_TM_SFT_RST_CFG0_UPG_SHIFT                             4
#define BCHP_TM_SFT_RST_CFG0_UPG_DEFAULT                           0x00000001

/* TM :: SFT_RST_CFG0 :: TIMERS [03:03] */
#define BCHP_TM_SFT_RST_CFG0_TIMERS_MASK                           0x00000008
#define BCHP_TM_SFT_RST_CFG0_TIMERS_SHIFT                          3
#define BCHP_TM_SFT_RST_CFG0_TIMERS_DEFAULT                        0x00000001

/* TM :: SFT_RST_CFG0 :: DEMOD_XPT [02:02] */
#define BCHP_TM_SFT_RST_CFG0_DEMOD_XPT_MASK                        0x00000004
#define BCHP_TM_SFT_RST_CFG0_DEMOD_XPT_SHIFT                       2
#define BCHP_TM_SFT_RST_CFG0_DEMOD_XPT_DEFAULT                     0x00000001

/* TM :: SFT_RST_CFG0 :: AVS [01:01] */
#define BCHP_TM_SFT_RST_CFG0_AVS_MASK                              0x00000002
#define BCHP_TM_SFT_RST_CFG0_AVS_SHIFT                             1
#define BCHP_TM_SFT_RST_CFG0_AVS_DEFAULT                           0x00000001

/* TM :: SFT_RST_CFG0 :: MTSIF [00:00] */
#define BCHP_TM_SFT_RST_CFG0_MTSIF_MASK                            0x00000001
#define BCHP_TM_SFT_RST_CFG0_MTSIF_SHIFT                           0
#define BCHP_TM_SFT_RST_CFG0_MTSIF_DEFAULT                         0x00000001

/***************************************************************************
 *SFT_RST1 - Soft Reset Control Register 1
 ***************************************************************************/
/* TM :: SFT_RST1 :: RSVD [31:09] */
#define BCHP_TM_SFT_RST1_RSVD_MASK                                 0xfffffe00
#define BCHP_TM_SFT_RST1_RSVD_SHIFT                                9
#define BCHP_TM_SFT_RST1_RSVD_DEFAULT                              0x00000000

/* TM :: SFT_RST1 :: UFE_TOP_1 [08:08] */
#define BCHP_TM_SFT_RST1_UFE_TOP_1_MASK                            0x00000100
#define BCHP_TM_SFT_RST1_UFE_TOP_1_SHIFT                           8
#define BCHP_TM_SFT_RST1_UFE_TOP_1_DEFAULT                         0x00000001

/* TM :: SFT_RST1 :: UFE_TOP_0 [07:07] */
#define BCHP_TM_SFT_RST1_UFE_TOP_0_MASK                            0x00000080
#define BCHP_TM_SFT_RST1_UFE_TOP_0_SHIFT                           7
#define BCHP_TM_SFT_RST1_UFE_TOP_0_DEFAULT                         0x00000001

/* TM :: SFT_RST1 :: ODS_RCVR_TOP_1 [06:06] */
#define BCHP_TM_SFT_RST1_ODS_RCVR_TOP_1_MASK                       0x00000040
#define BCHP_TM_SFT_RST1_ODS_RCVR_TOP_1_SHIFT                      6
#define BCHP_TM_SFT_RST1_ODS_RCVR_TOP_1_DEFAULT                    0x00000001

/* TM :: SFT_RST1 :: ODS_RCVR_TOP_0 [05:05] */
#define BCHP_TM_SFT_RST1_ODS_RCVR_TOP_0_MASK                       0x00000020
#define BCHP_TM_SFT_RST1_ODS_RCVR_TOP_0_SHIFT                      5
#define BCHP_TM_SFT_RST1_ODS_RCVR_TOP_0_DEFAULT                    0x00000001

/* TM :: SFT_RST1 :: T2_BICM_TOP_1 [04:04] */
#define BCHP_TM_SFT_RST1_T2_BICM_TOP_1_MASK                        0x00000010
#define BCHP_TM_SFT_RST1_T2_BICM_TOP_1_SHIFT                       4
#define BCHP_TM_SFT_RST1_T2_BICM_TOP_1_DEFAULT                     0x00000001

/* TM :: SFT_RST1 :: T2_BICM_TOP_0 [03:03] */
#define BCHP_TM_SFT_RST1_T2_BICM_TOP_0_MASK                        0x00000008
#define BCHP_TM_SFT_RST1_T2_BICM_TOP_0_SHIFT                       3
#define BCHP_TM_SFT_RST1_T2_BICM_TOP_0_DEFAULT                     0x00000001

/* TM :: SFT_RST1 :: CORE_MTAP [02:02] */
#define BCHP_TM_SFT_RST1_CORE_MTAP_MASK                            0x00000004
#define BCHP_TM_SFT_RST1_CORE_MTAP_SHIFT                           2
#define BCHP_TM_SFT_RST1_CORE_MTAP_DEFAULT                         0x00000001

/* TM :: SFT_RST1 :: DSTOPA_1 [01:01] */
#define BCHP_TM_SFT_RST1_DSTOPA_1_MASK                             0x00000002
#define BCHP_TM_SFT_RST1_DSTOPA_1_SHIFT                            1
#define BCHP_TM_SFT_RST1_DSTOPA_1_DEFAULT                          0x00000001

/* TM :: SFT_RST1 :: DSTOPA_0 [00:00] */
#define BCHP_TM_SFT_RST1_DSTOPA_0_MASK                             0x00000001
#define BCHP_TM_SFT_RST1_DSTOPA_0_SHIFT                            0
#define BCHP_TM_SFT_RST1_DSTOPA_0_DEFAULT                          0x00000001

/***************************************************************************
 *SFT_RST_CFG1 - Soft Reset Configuration Control Register 1
 ***************************************************************************/
/* TM :: SFT_RST_CFG1 :: RSVD [31:09] */
#define BCHP_TM_SFT_RST_CFG1_RSVD_MASK                             0xfffffe00
#define BCHP_TM_SFT_RST_CFG1_RSVD_SHIFT                            9
#define BCHP_TM_SFT_RST_CFG1_RSVD_DEFAULT                          0x00000001

/* TM :: SFT_RST_CFG1 :: UFE_TOP_1 [08:08] */
#define BCHP_TM_SFT_RST_CFG1_UFE_TOP_1_MASK                        0x00000100
#define BCHP_TM_SFT_RST_CFG1_UFE_TOP_1_SHIFT                       8
#define BCHP_TM_SFT_RST_CFG1_UFE_TOP_1_DEFAULT                     0x00000001

/* TM :: SFT_RST_CFG1 :: UFE_TOP_0 [07:07] */
#define BCHP_TM_SFT_RST_CFG1_UFE_TOP_0_MASK                        0x00000080
#define BCHP_TM_SFT_RST_CFG1_UFE_TOP_0_SHIFT                       7
#define BCHP_TM_SFT_RST_CFG1_UFE_TOP_0_DEFAULT                     0x00000001

/* TM :: SFT_RST_CFG1 :: ODS_RCVR_TOP_1 [06:06] */
#define BCHP_TM_SFT_RST_CFG1_ODS_RCVR_TOP_1_MASK                   0x00000040
#define BCHP_TM_SFT_RST_CFG1_ODS_RCVR_TOP_1_SHIFT                  6
#define BCHP_TM_SFT_RST_CFG1_ODS_RCVR_TOP_1_DEFAULT                0x00000001

/* TM :: SFT_RST_CFG1 :: ODS_RCVR_TOP_0 [05:05] */
#define BCHP_TM_SFT_RST_CFG1_ODS_RCVR_TOP_0_MASK                   0x00000020
#define BCHP_TM_SFT_RST_CFG1_ODS_RCVR_TOP_0_SHIFT                  5
#define BCHP_TM_SFT_RST_CFG1_ODS_RCVR_TOP_0_DEFAULT                0x00000001

/* TM :: SFT_RST_CFG1 :: T2_BICM_TOP_1 [04:04] */
#define BCHP_TM_SFT_RST_CFG1_T2_BICM_TOP_1_MASK                    0x00000010
#define BCHP_TM_SFT_RST_CFG1_T2_BICM_TOP_1_SHIFT                   4
#define BCHP_TM_SFT_RST_CFG1_T2_BICM_TOP_1_DEFAULT                 0x00000001

/* TM :: SFT_RST_CFG1 :: T2_BICM_TOP_0 [03:03] */
#define BCHP_TM_SFT_RST_CFG1_T2_BICM_TOP_0_MASK                    0x00000008
#define BCHP_TM_SFT_RST_CFG1_T2_BICM_TOP_0_SHIFT                   3
#define BCHP_TM_SFT_RST_CFG1_T2_BICM_TOP_0_DEFAULT                 0x00000001

/* TM :: SFT_RST_CFG1 :: CORE_MTAP [02:02] */
#define BCHP_TM_SFT_RST_CFG1_CORE_MTAP_MASK                        0x00000004
#define BCHP_TM_SFT_RST_CFG1_CORE_MTAP_SHIFT                       2
#define BCHP_TM_SFT_RST_CFG1_CORE_MTAP_DEFAULT                     0x00000001

/* TM :: SFT_RST_CFG1 :: DSTOPA_1 [01:01] */
#define BCHP_TM_SFT_RST_CFG1_DSTOPA_1_MASK                         0x00000002
#define BCHP_TM_SFT_RST_CFG1_DSTOPA_1_SHIFT                        1
#define BCHP_TM_SFT_RST_CFG1_DSTOPA_1_DEFAULT                      0x00000001

/* TM :: SFT_RST_CFG1 :: DSTOPA_0 [00:00] */
#define BCHP_TM_SFT_RST_CFG1_DSTOPA_0_MASK                         0x00000001
#define BCHP_TM_SFT_RST_CFG1_DSTOPA_0_SHIFT                        0
#define BCHP_TM_SFT_RST_CFG1_DSTOPA_0_DEFAULT                      0x00000001

/***************************************************************************
 *CLOCK_ENABLE_CTRL1 - Clock Enable Control Register 1
 ***************************************************************************/
/* TM :: CLOCK_ENABLE_CTRL1 :: RSVD [31:01] */
#define BCHP_TM_CLOCK_ENABLE_CTRL1_RSVD_MASK                       0xfffffffe
#define BCHP_TM_CLOCK_ENABLE_CTRL1_RSVD_SHIFT                      1
#define BCHP_TM_CLOCK_ENABLE_CTRL1_RSVD_DEFAULT                    0x00000000

/* TM :: CLOCK_ENABLE_CTRL1 :: PM_DISABLE_ALL_CLOCKS [00:00] */
#define BCHP_TM_CLOCK_ENABLE_CTRL1_PM_DISABLE_ALL_CLOCKS_MASK      0x00000001
#define BCHP_TM_CLOCK_ENABLE_CTRL1_PM_DISABLE_ALL_CLOCKS_SHIFT     0
#define BCHP_TM_CLOCK_ENABLE_CTRL1_PM_DISABLE_ALL_CLOCKS_DEFAULT   0x00000000

/***************************************************************************
 *AVS_OVERTEMP_CTRL - AVS Over Temperature Control
 ***************************************************************************/
/* TM :: AVS_OVERTEMP_CTRL :: RSVD [31:01] */
#define BCHP_TM_AVS_OVERTEMP_CTRL_RSVD_MASK                        0xfffffffe
#define BCHP_TM_AVS_OVERTEMP_CTRL_RSVD_SHIFT                       1
#define BCHP_TM_AVS_OVERTEMP_CTRL_RSVD_DEFAULT                     0x00000000

/* TM :: AVS_OVERTEMP_CTRL :: AVS_RESET_GUARDBAND [00:00] */
#define BCHP_TM_AVS_OVERTEMP_CTRL_AVS_RESET_GUARDBAND_MASK         0x00000001
#define BCHP_TM_AVS_OVERTEMP_CTRL_AVS_RESET_GUARDBAND_SHIFT        0
#define BCHP_TM_AVS_OVERTEMP_CTRL_AVS_RESET_GUARDBAND_DEFAULT      0x00000000

/***************************************************************************
 *UART_ROUTER_SEL - UART Router select
 ***************************************************************************/
/* TM :: UART_ROUTER_SEL :: port_7_cpu_sel [31:28] */
#define BCHP_TM_UART_ROUTER_SEL_port_7_cpu_sel_MASK                0xf0000000
#define BCHP_TM_UART_ROUTER_SEL_port_7_cpu_sel_SHIFT               28
#define BCHP_TM_UART_ROUTER_SEL_port_7_cpu_sel_DEFAULT             0x00000000
#define BCHP_TM_UART_ROUTER_SEL_port_7_cpu_sel_NO_CPU              0
#define BCHP_TM_UART_ROUTER_SEL_port_7_cpu_sel_LEAP_TOP            1
#define BCHP_TM_UART_ROUTER_SEL_port_7_cpu_sel_AVS_TOP             2
#define BCHP_TM_UART_ROUTER_SEL_port_7_cpu_sel_UNUSED_3            3
#define BCHP_TM_UART_ROUTER_SEL_port_7_cpu_sel_UNUSED_4            4
#define BCHP_TM_UART_ROUTER_SEL_port_7_cpu_sel_UNUSED_5            5
#define BCHP_TM_UART_ROUTER_SEL_port_7_cpu_sel_UNUSED_6            6
#define BCHP_TM_UART_ROUTER_SEL_port_7_cpu_sel_UNUSED_7            7
#define BCHP_TM_UART_ROUTER_SEL_port_7_cpu_sel_UNUSED_8            8
#define BCHP_TM_UART_ROUTER_SEL_port_7_cpu_sel_UNUSED_9            9
#define BCHP_TM_UART_ROUTER_SEL_port_7_cpu_sel_UNUSED_10           10
#define BCHP_TM_UART_ROUTER_SEL_port_7_cpu_sel_UNUSED_11           11
#define BCHP_TM_UART_ROUTER_SEL_port_7_cpu_sel_UNUSED_12           12
#define BCHP_TM_UART_ROUTER_SEL_port_7_cpu_sel_UNUSED_13           13
#define BCHP_TM_UART_ROUTER_SEL_port_7_cpu_sel_UNUSED_14           14
#define BCHP_TM_UART_ROUTER_SEL_port_7_cpu_sel_UNUSED_15           15

/* TM :: UART_ROUTER_SEL :: port_6_cpu_sel [27:24] */
#define BCHP_TM_UART_ROUTER_SEL_port_6_cpu_sel_MASK                0x0f000000
#define BCHP_TM_UART_ROUTER_SEL_port_6_cpu_sel_SHIFT               24
#define BCHP_TM_UART_ROUTER_SEL_port_6_cpu_sel_DEFAULT             0x00000000
#define BCHP_TM_UART_ROUTER_SEL_port_6_cpu_sel_NO_CPU              0
#define BCHP_TM_UART_ROUTER_SEL_port_6_cpu_sel_LEAP_TOP            1
#define BCHP_TM_UART_ROUTER_SEL_port_6_cpu_sel_AVS_TOP             2
#define BCHP_TM_UART_ROUTER_SEL_port_6_cpu_sel_UNUSED_3            3
#define BCHP_TM_UART_ROUTER_SEL_port_6_cpu_sel_UNUSED_4            4
#define BCHP_TM_UART_ROUTER_SEL_port_6_cpu_sel_UNUSED_5            5
#define BCHP_TM_UART_ROUTER_SEL_port_6_cpu_sel_UNUSED_6            6
#define BCHP_TM_UART_ROUTER_SEL_port_6_cpu_sel_UNUSED_7            7
#define BCHP_TM_UART_ROUTER_SEL_port_6_cpu_sel_UNUSED_8            8
#define BCHP_TM_UART_ROUTER_SEL_port_6_cpu_sel_UNUSED_9            9
#define BCHP_TM_UART_ROUTER_SEL_port_6_cpu_sel_UNUSED_10           10
#define BCHP_TM_UART_ROUTER_SEL_port_6_cpu_sel_UNUSED_11           11
#define BCHP_TM_UART_ROUTER_SEL_port_6_cpu_sel_UNUSED_12           12
#define BCHP_TM_UART_ROUTER_SEL_port_6_cpu_sel_UNUSED_13           13
#define BCHP_TM_UART_ROUTER_SEL_port_6_cpu_sel_UNUSED_14           14
#define BCHP_TM_UART_ROUTER_SEL_port_6_cpu_sel_UNUSED_15           15

/* TM :: UART_ROUTER_SEL :: port_5_cpu_sel [23:20] */
#define BCHP_TM_UART_ROUTER_SEL_port_5_cpu_sel_MASK                0x00f00000
#define BCHP_TM_UART_ROUTER_SEL_port_5_cpu_sel_SHIFT               20
#define BCHP_TM_UART_ROUTER_SEL_port_5_cpu_sel_DEFAULT             0x00000000
#define BCHP_TM_UART_ROUTER_SEL_port_5_cpu_sel_NO_CPU              0
#define BCHP_TM_UART_ROUTER_SEL_port_5_cpu_sel_LEAP_TOP            1
#define BCHP_TM_UART_ROUTER_SEL_port_5_cpu_sel_AVS_TOP             2
#define BCHP_TM_UART_ROUTER_SEL_port_5_cpu_sel_UNUSED_3            3
#define BCHP_TM_UART_ROUTER_SEL_port_5_cpu_sel_UNUSED_4            4
#define BCHP_TM_UART_ROUTER_SEL_port_5_cpu_sel_UNUSED_5            5
#define BCHP_TM_UART_ROUTER_SEL_port_5_cpu_sel_UNUSED_6            6
#define BCHP_TM_UART_ROUTER_SEL_port_5_cpu_sel_UNUSED_7            7
#define BCHP_TM_UART_ROUTER_SEL_port_5_cpu_sel_UNUSED_8            8
#define BCHP_TM_UART_ROUTER_SEL_port_5_cpu_sel_UNUSED_9            9
#define BCHP_TM_UART_ROUTER_SEL_port_5_cpu_sel_UNUSED_10           10
#define BCHP_TM_UART_ROUTER_SEL_port_5_cpu_sel_UNUSED_11           11
#define BCHP_TM_UART_ROUTER_SEL_port_5_cpu_sel_UNUSED_12           12
#define BCHP_TM_UART_ROUTER_SEL_port_5_cpu_sel_UNUSED_13           13
#define BCHP_TM_UART_ROUTER_SEL_port_5_cpu_sel_UNUSED_14           14
#define BCHP_TM_UART_ROUTER_SEL_port_5_cpu_sel_UNUSED_15           15

/* TM :: UART_ROUTER_SEL :: port_4_cpu_sel [19:16] */
#define BCHP_TM_UART_ROUTER_SEL_port_4_cpu_sel_MASK                0x000f0000
#define BCHP_TM_UART_ROUTER_SEL_port_4_cpu_sel_SHIFT               16
#define BCHP_TM_UART_ROUTER_SEL_port_4_cpu_sel_DEFAULT             0x00000000
#define BCHP_TM_UART_ROUTER_SEL_port_4_cpu_sel_NO_CPU              0
#define BCHP_TM_UART_ROUTER_SEL_port_4_cpu_sel_LEAP_TOP            1
#define BCHP_TM_UART_ROUTER_SEL_port_4_cpu_sel_AVS_TOP             2
#define BCHP_TM_UART_ROUTER_SEL_port_4_cpu_sel_UNUSED_3            3
#define BCHP_TM_UART_ROUTER_SEL_port_4_cpu_sel_UNUSED_4            4
#define BCHP_TM_UART_ROUTER_SEL_port_4_cpu_sel_UNUSED_5            5
#define BCHP_TM_UART_ROUTER_SEL_port_4_cpu_sel_UNUSED_6            6
#define BCHP_TM_UART_ROUTER_SEL_port_4_cpu_sel_UNUSED_7            7
#define BCHP_TM_UART_ROUTER_SEL_port_4_cpu_sel_UNUSED_8            8
#define BCHP_TM_UART_ROUTER_SEL_port_4_cpu_sel_UNUSED_9            9
#define BCHP_TM_UART_ROUTER_SEL_port_4_cpu_sel_UNUSED_10           10
#define BCHP_TM_UART_ROUTER_SEL_port_4_cpu_sel_UNUSED_11           11
#define BCHP_TM_UART_ROUTER_SEL_port_4_cpu_sel_UNUSED_12           12
#define BCHP_TM_UART_ROUTER_SEL_port_4_cpu_sel_UNUSED_13           13
#define BCHP_TM_UART_ROUTER_SEL_port_4_cpu_sel_UNUSED_14           14
#define BCHP_TM_UART_ROUTER_SEL_port_4_cpu_sel_UNUSED_15           15

/* TM :: UART_ROUTER_SEL :: port_3_cpu_sel [15:12] */
#define BCHP_TM_UART_ROUTER_SEL_port_3_cpu_sel_MASK                0x0000f000
#define BCHP_TM_UART_ROUTER_SEL_port_3_cpu_sel_SHIFT               12
#define BCHP_TM_UART_ROUTER_SEL_port_3_cpu_sel_DEFAULT             0x00000000
#define BCHP_TM_UART_ROUTER_SEL_port_3_cpu_sel_NO_CPU              0
#define BCHP_TM_UART_ROUTER_SEL_port_3_cpu_sel_LEAP_TOP            1
#define BCHP_TM_UART_ROUTER_SEL_port_3_cpu_sel_AVS_TOP             2
#define BCHP_TM_UART_ROUTER_SEL_port_3_cpu_sel_UNUSED_3            3
#define BCHP_TM_UART_ROUTER_SEL_port_3_cpu_sel_UNUSED_4            4
#define BCHP_TM_UART_ROUTER_SEL_port_3_cpu_sel_UNUSED_5            5
#define BCHP_TM_UART_ROUTER_SEL_port_3_cpu_sel_UNUSED_6            6
#define BCHP_TM_UART_ROUTER_SEL_port_3_cpu_sel_UNUSED_7            7
#define BCHP_TM_UART_ROUTER_SEL_port_3_cpu_sel_UNUSED_8            8
#define BCHP_TM_UART_ROUTER_SEL_port_3_cpu_sel_UNUSED_9            9
#define BCHP_TM_UART_ROUTER_SEL_port_3_cpu_sel_UNUSED_10           10
#define BCHP_TM_UART_ROUTER_SEL_port_3_cpu_sel_UNUSED_11           11
#define BCHP_TM_UART_ROUTER_SEL_port_3_cpu_sel_UNUSED_12           12
#define BCHP_TM_UART_ROUTER_SEL_port_3_cpu_sel_UNUSED_13           13
#define BCHP_TM_UART_ROUTER_SEL_port_3_cpu_sel_UNUSED_14           14
#define BCHP_TM_UART_ROUTER_SEL_port_3_cpu_sel_UNUSED_15           15

/* TM :: UART_ROUTER_SEL :: port_2_cpu_sel [11:08] */
#define BCHP_TM_UART_ROUTER_SEL_port_2_cpu_sel_MASK                0x00000f00
#define BCHP_TM_UART_ROUTER_SEL_port_2_cpu_sel_SHIFT               8
#define BCHP_TM_UART_ROUTER_SEL_port_2_cpu_sel_DEFAULT             0x00000000
#define BCHP_TM_UART_ROUTER_SEL_port_2_cpu_sel_NO_CPU              0
#define BCHP_TM_UART_ROUTER_SEL_port_2_cpu_sel_LEAP_TOP            1
#define BCHP_TM_UART_ROUTER_SEL_port_2_cpu_sel_AVS_TOP             2
#define BCHP_TM_UART_ROUTER_SEL_port_2_cpu_sel_UNUSED_3            3
#define BCHP_TM_UART_ROUTER_SEL_port_2_cpu_sel_UNUSED_4            4
#define BCHP_TM_UART_ROUTER_SEL_port_2_cpu_sel_UNUSED_5            5
#define BCHP_TM_UART_ROUTER_SEL_port_2_cpu_sel_UNUSED_6            6
#define BCHP_TM_UART_ROUTER_SEL_port_2_cpu_sel_UNUSED_7            7
#define BCHP_TM_UART_ROUTER_SEL_port_2_cpu_sel_UNUSED_8            8
#define BCHP_TM_UART_ROUTER_SEL_port_2_cpu_sel_UNUSED_9            9
#define BCHP_TM_UART_ROUTER_SEL_port_2_cpu_sel_UNUSED_10           10
#define BCHP_TM_UART_ROUTER_SEL_port_2_cpu_sel_UNUSED_11           11
#define BCHP_TM_UART_ROUTER_SEL_port_2_cpu_sel_UNUSED_12           12
#define BCHP_TM_UART_ROUTER_SEL_port_2_cpu_sel_UNUSED_13           13
#define BCHP_TM_UART_ROUTER_SEL_port_2_cpu_sel_UNUSED_14           14
#define BCHP_TM_UART_ROUTER_SEL_port_2_cpu_sel_UNUSED_15           15

/* TM :: UART_ROUTER_SEL :: port_1_cpu_sel [07:04] */
#define BCHP_TM_UART_ROUTER_SEL_port_1_cpu_sel_MASK                0x000000f0
#define BCHP_TM_UART_ROUTER_SEL_port_1_cpu_sel_SHIFT               4
#define BCHP_TM_UART_ROUTER_SEL_port_1_cpu_sel_DEFAULT             0x00000000
#define BCHP_TM_UART_ROUTER_SEL_port_1_cpu_sel_NO_CPU              0
#define BCHP_TM_UART_ROUTER_SEL_port_1_cpu_sel_LEAP_TOP            1
#define BCHP_TM_UART_ROUTER_SEL_port_1_cpu_sel_AVS_TOP             2
#define BCHP_TM_UART_ROUTER_SEL_port_1_cpu_sel_UNUSED_3            3
#define BCHP_TM_UART_ROUTER_SEL_port_1_cpu_sel_UNUSED_4            4
#define BCHP_TM_UART_ROUTER_SEL_port_1_cpu_sel_UNUSED_5            5
#define BCHP_TM_UART_ROUTER_SEL_port_1_cpu_sel_UNUSED_6            6
#define BCHP_TM_UART_ROUTER_SEL_port_1_cpu_sel_UNUSED_7            7
#define BCHP_TM_UART_ROUTER_SEL_port_1_cpu_sel_UNUSED_8            8
#define BCHP_TM_UART_ROUTER_SEL_port_1_cpu_sel_UNUSED_9            9
#define BCHP_TM_UART_ROUTER_SEL_port_1_cpu_sel_UNUSED_10           10
#define BCHP_TM_UART_ROUTER_SEL_port_1_cpu_sel_UNUSED_11           11
#define BCHP_TM_UART_ROUTER_SEL_port_1_cpu_sel_UNUSED_12           12
#define BCHP_TM_UART_ROUTER_SEL_port_1_cpu_sel_UNUSED_13           13
#define BCHP_TM_UART_ROUTER_SEL_port_1_cpu_sel_UNUSED_14           14
#define BCHP_TM_UART_ROUTER_SEL_port_1_cpu_sel_UNUSED_15           15

/* TM :: UART_ROUTER_SEL :: port_0_cpu_sel [03:00] */
#define BCHP_TM_UART_ROUTER_SEL_port_0_cpu_sel_MASK                0x0000000f
#define BCHP_TM_UART_ROUTER_SEL_port_0_cpu_sel_SHIFT               0
#define BCHP_TM_UART_ROUTER_SEL_port_0_cpu_sel_DEFAULT             0x00000000
#define BCHP_TM_UART_ROUTER_SEL_port_0_cpu_sel_NO_CPU              0
#define BCHP_TM_UART_ROUTER_SEL_port_0_cpu_sel_LEAP_TOP            1
#define BCHP_TM_UART_ROUTER_SEL_port_0_cpu_sel_AVS_TOP             2
#define BCHP_TM_UART_ROUTER_SEL_port_0_cpu_sel_UNUSED_3            3
#define BCHP_TM_UART_ROUTER_SEL_port_0_cpu_sel_UNUSED_4            4
#define BCHP_TM_UART_ROUTER_SEL_port_0_cpu_sel_UNUSED_5            5
#define BCHP_TM_UART_ROUTER_SEL_port_0_cpu_sel_UNUSED_6            6
#define BCHP_TM_UART_ROUTER_SEL_port_0_cpu_sel_UNUSED_7            7
#define BCHP_TM_UART_ROUTER_SEL_port_0_cpu_sel_UNUSED_8            8
#define BCHP_TM_UART_ROUTER_SEL_port_0_cpu_sel_UNUSED_9            9
#define BCHP_TM_UART_ROUTER_SEL_port_0_cpu_sel_UNUSED_10           10
#define BCHP_TM_UART_ROUTER_SEL_port_0_cpu_sel_UNUSED_11           11
#define BCHP_TM_UART_ROUTER_SEL_port_0_cpu_sel_UNUSED_12           12
#define BCHP_TM_UART_ROUTER_SEL_port_0_cpu_sel_UNUSED_13           13
#define BCHP_TM_UART_ROUTER_SEL_port_0_cpu_sel_UNUSED_14           14
#define BCHP_TM_UART_ROUTER_SEL_port_0_cpu_sel_UNUSED_15           15

/***************************************************************************
 *EJTAG_INPUT_EN - EJTAG input bus enables
 ***************************************************************************/
/* TM :: EJTAG_INPUT_EN :: reserved0 [31:02] */
#define BCHP_TM_EJTAG_INPUT_EN_reserved0_MASK                      0xfffffffc
#define BCHP_TM_EJTAG_INPUT_EN_reserved0_SHIFT                     2

/* TM :: EJTAG_INPUT_EN :: ejtag_input_enable [01:00] */
#define BCHP_TM_EJTAG_INPUT_EN_ejtag_input_enable_MASK             0x00000003
#define BCHP_TM_EJTAG_INPUT_EN_ejtag_input_enable_SHIFT            0
#define BCHP_TM_EJTAG_INPUT_EN_ejtag_input_enable_DEFAULT          0x00000002
#define BCHP_TM_EJTAG_INPUT_EN_ejtag_input_enable_DO_NOT_USE_CPU_ONE_HOT 1
#define BCHP_TM_EJTAG_INPUT_EN_ejtag_input_enable_LEAP_CPU_ONE_HOT 2

/***************************************************************************
 *EJTAG_INPUT_SEL - EJTAG input select
 ***************************************************************************/
/* TM :: EJTAG_INPUT_SEL :: reserved0 [31:01] */
#define BCHP_TM_EJTAG_INPUT_SEL_reserved0_MASK                     0xfffffffe
#define BCHP_TM_EJTAG_INPUT_SEL_reserved0_SHIFT                    1

/* TM :: EJTAG_INPUT_SEL :: ejtag_input_sel [00:00] */
#define BCHP_TM_EJTAG_INPUT_SEL_ejtag_input_sel_MASK               0x00000001
#define BCHP_TM_EJTAG_INPUT_SEL_ejtag_input_sel_SHIFT              0
#define BCHP_TM_EJTAG_INPUT_SEL_ejtag_input_sel_DEFAULT            0x00000000

/***************************************************************************
 *EJTAG_OUTPUT_SEL - EJTAG output select
 ***************************************************************************/
/* TM :: EJTAG_OUTPUT_SEL :: reserved0 [31:02] */
#define BCHP_TM_EJTAG_OUTPUT_SEL_reserved0_MASK                    0xfffffffc
#define BCHP_TM_EJTAG_OUTPUT_SEL_reserved0_SHIFT                   2

/* TM :: EJTAG_OUTPUT_SEL :: ejtag_output_sel [01:00] */
#define BCHP_TM_EJTAG_OUTPUT_SEL_ejtag_output_sel_MASK             0x00000003
#define BCHP_TM_EJTAG_OUTPUT_SEL_ejtag_output_sel_SHIFT            0
#define BCHP_TM_EJTAG_OUTPUT_SEL_ejtag_output_sel_DEFAULT          0x00000001
#define BCHP_TM_EJTAG_OUTPUT_SEL_ejtag_output_sel_DO_NOT_USE_CPU   0
#define BCHP_TM_EJTAG_OUTPUT_SEL_ejtag_output_sel_LEAP_TOP         1

/***************************************************************************
 *MTSIF0_CTRL - MTSIF0 Pad Control Register
 ***************************************************************************/
/* TM :: MTSIF0_CTRL :: RSVD [31:07] */
#define BCHP_TM_MTSIF0_CTRL_RSVD_MASK                              0xffffff80
#define BCHP_TM_MTSIF0_CTRL_RSVD_SHIFT                             7
#define BCHP_TM_MTSIF0_CTRL_RSVD_DEFAULT                           0x00000000

/* TM :: MTSIF0_CTRL :: MTSIF_0_PAD_INPUT_DISABLE [06:06] */
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_INPUT_DISABLE_MASK         0x00000040
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_INPUT_DISABLE_SHIFT        6
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_INPUT_DISABLE_DEFAULT      0x00000000
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_INPUT_DISABLE_INPUT_ENABLE 0
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_INPUT_DISABLE_INPUT_DISABLE 1

/* TM :: MTSIF0_CTRL :: MTSIF_0_PAD_AMP_EN [05:05] */
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_AMP_EN_MASK                0x00000020
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_AMP_EN_SHIFT               5
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_AMP_EN_DEFAULT             0x00000000

/* TM :: MTSIF0_CTRL :: MTSIF_0_PAD_SEL_GMII [04:04] */
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_SEL_GMII_MASK              0x00000010
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_SEL_GMII_SHIFT             4
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_SEL_GMII_DEFAULT           0x00000001

/* TM :: MTSIF0_CTRL :: MTSIF_0_PAD_MODEHV [03:03] */
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_MODEHV_MASK                0x00000008
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_MODEHV_SHIFT               3
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_MODEHV_DEFAULT             0x00000001

/* TM :: MTSIF0_CTRL :: MTSIF_0_PAD_SEL [02:00] */
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_SEL_MASK                   0x00000007
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_SEL_SHIFT                  0
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_SEL_DEFAULT                0x00000003
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_SEL_DRIVE_2MA              0
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_SEL_DRIVE_4MA              1
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_SEL_DRIVE_6MA              2
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_SEL_DRIVE_8MA              3
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_SEL_DRIVE_10MA             4
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_SEL_DRIVE_12MA             5
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_SEL_DRIVE_14MA             6
#define BCHP_TM_MTSIF0_CTRL_MTSIF_0_PAD_SEL_DRIVE_16MA             7

/***************************************************************************
 *MTSIF_RX_CTRL - MTSIF_RX Pad Control Register
 ***************************************************************************/
/* TM :: MTSIF_RX_CTRL :: RSVD [31:07] */
#define BCHP_TM_MTSIF_RX_CTRL_RSVD_MASK                            0xffffff80
#define BCHP_TM_MTSIF_RX_CTRL_RSVD_SHIFT                           7
#define BCHP_TM_MTSIF_RX_CTRL_RSVD_DEFAULT                         0x00000000

/* TM :: MTSIF_RX_CTRL :: MTSIF_RX_PAD_INPUT_DISABLE [06:06] */
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_INPUT_DISABLE_MASK      0x00000040
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_INPUT_DISABLE_SHIFT     6
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_INPUT_DISABLE_DEFAULT   0x00000000
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_INPUT_DISABLE_INPUT_ENABLE 0
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_INPUT_DISABLE_INPUT_DISABLE 1

/* TM :: MTSIF_RX_CTRL :: MTSIF_RX_PAD_AMP_EN [05:05] */
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_AMP_EN_MASK             0x00000020
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_AMP_EN_SHIFT            5
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_AMP_EN_DEFAULT          0x00000000

/* TM :: MTSIF_RX_CTRL :: MTSIF_RX_PAD_SEL_GMII [04:04] */
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_SEL_GMII_MASK           0x00000010
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_SEL_GMII_SHIFT          4
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_SEL_GMII_DEFAULT        0x00000001

/* TM :: MTSIF_RX_CTRL :: MTSIF_RX_PAD_MODEHV [03:03] */
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_MODEHV_MASK             0x00000008
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_MODEHV_SHIFT            3
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_MODEHV_DEFAULT          0x00000001

/* TM :: MTSIF_RX_CTRL :: MTSIF_RX_PAD_SEL [02:00] */
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_SEL_MASK                0x00000007
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_SEL_SHIFT               0
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_SEL_DEFAULT             0x00000003
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_SEL_DRIVE_2MA           0
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_SEL_DRIVE_4MA           1
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_SEL_DRIVE_6MA           2
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_SEL_DRIVE_8MA           3
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_SEL_DRIVE_10MA          4
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_SEL_DRIVE_12MA          5
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_SEL_DRIVE_14MA          6
#define BCHP_TM_MTSIF_RX_CTRL_MTSIF_RX_PAD_SEL_DRIVE_16MA          7

/***************************************************************************
 *TEST_MODE - Test Mode Control Register
 ***************************************************************************/
/* TM :: TEST_MODE :: RSVD [31:09] */
#define BCHP_TM_TEST_MODE_RSVD_MASK                                0xfffffe00
#define BCHP_TM_TEST_MODE_RSVD_SHIFT                               9
#define BCHP_TM_TEST_MODE_RSVD_DEFAULT                             0x00000000

/* TM :: TEST_MODE :: EN [08:08] */
#define BCHP_TM_TEST_MODE_EN_MASK                                  0x00000100
#define BCHP_TM_TEST_MODE_EN_SHIFT                                 8
#define BCHP_TM_TEST_MODE_EN_DEFAULT                               0x00000000

/* TM :: TEST_MODE :: MODE [07:00] */
#define BCHP_TM_TEST_MODE_MODE_MASK                                0x000000ff
#define BCHP_TM_TEST_MODE_MODE_SHIFT                               0
#define BCHP_TM_TEST_MODE_MODE_DEFAULT                             0x00000000

/***************************************************************************
 *TEST_MISC2 - Test Misc2 Control Register
 ***************************************************************************/
/* TM :: TEST_MISC2 :: MISC [31:00] */
#define BCHP_TM_TEST_MISC2_MISC_MASK                               0xffffffff
#define BCHP_TM_TEST_MISC2_MISC_SHIFT                              0
#define BCHP_TM_TEST_MISC2_MISC_DEFAULT                            0x00000000

/***************************************************************************
 *TEST_MISC1 - Test Misc1 Control Register
 ***************************************************************************/
/* TM :: TEST_MISC1 :: MISC [31:00] */
#define BCHP_TM_TEST_MISC1_MISC_MASK                               0xffffffff
#define BCHP_TM_TEST_MISC1_MISC_SHIFT                              0
#define BCHP_TM_TEST_MISC1_MISC_DEFAULT                            0x00000000

/***************************************************************************
 *TEST_MISC0 - Test Misc0 Control Register
 ***************************************************************************/
/* TM :: TEST_MISC0 :: MISC [31:00] */
#define BCHP_TM_TEST_MISC0_MISC_MASK                               0xffffffff
#define BCHP_TM_TEST_MISC0_MISC_SHIFT                              0
#define BCHP_TM_TEST_MISC0_MISC_DEFAULT                            0x00000000

/***************************************************************************
 *UART_RX_CTRL - UART Control Register
 ***************************************************************************/
/* TM :: UART_RX_CTRL :: RSVD [31:06] */
#define BCHP_TM_UART_RX_CTRL_RSVD_MASK                             0xffffffc0
#define BCHP_TM_UART_RX_CTRL_RSVD_SHIFT                            6
#define BCHP_TM_UART_RX_CTRL_RSVD_DEFAULT                          0x00000000

/* TM :: UART_RX_CTRL :: EN_RX [05:05] */
#define BCHP_TM_UART_RX_CTRL_EN_RX_MASK                            0x00000020
#define BCHP_TM_UART_RX_CTRL_EN_RX_SHIFT                           5
#define BCHP_TM_UART_RX_CTRL_EN_RX_DEFAULT                         0x00000000

/* TM :: UART_RX_CTRL :: RSVD_0 [04:02] */
#define BCHP_TM_UART_RX_CTRL_RSVD_0_MASK                           0x0000001c
#define BCHP_TM_UART_RX_CTRL_RSVD_0_SHIFT                          2
#define BCHP_TM_UART_RX_CTRL_RSVD_0_DEFAULT                        0x00000000

/* TM :: UART_RX_CTRL :: GPIO_SEL_RX [01:00] */
#define BCHP_TM_UART_RX_CTRL_GPIO_SEL_RX_MASK                      0x00000003
#define BCHP_TM_UART_RX_CTRL_GPIO_SEL_RX_SHIFT                     0
#define BCHP_TM_UART_RX_CTRL_GPIO_SEL_RX_DEFAULT                   0x00000000

/***************************************************************************
 *GPIO_IODIR - GPIO IO Directional Control Register
 ***************************************************************************/
/* TM :: GPIO_IODIR :: RSVD [31:20] */
#define BCHP_TM_GPIO_IODIR_RSVD_MASK                               0xfff00000
#define BCHP_TM_GPIO_IODIR_RSVD_SHIFT                              20
#define BCHP_TM_GPIO_IODIR_RSVD_DEFAULT                            0x00000000

/* TM :: GPIO_IODIR :: IODIR [19:00] */
#define BCHP_TM_GPIO_IODIR_IODIR_MASK                              0x000fffff
#define BCHP_TM_GPIO_IODIR_IODIR_SHIFT                             0
#define BCHP_TM_GPIO_IODIR_IODIR_DEFAULT                           0x000fffff

/***************************************************************************
 *GPIO_OD - GPIO Open Drain Control Register
 ***************************************************************************/
/* TM :: GPIO_OD :: RSVD [31:20] */
#define BCHP_TM_GPIO_OD_RSVD_MASK                                  0xfff00000
#define BCHP_TM_GPIO_OD_RSVD_SHIFT                                 20
#define BCHP_TM_GPIO_OD_RSVD_DEFAULT                               0x00000000

/* TM :: GPIO_OD :: OD [19:00] */
#define BCHP_TM_GPIO_OD_OD_MASK                                    0x000fffff
#define BCHP_TM_GPIO_OD_OD_SHIFT                                   0
#define BCHP_TM_GPIO_OD_OD_DEFAULT                                 0x00000000

/***************************************************************************
 *GPIO_DATA - GPIO Data Control Register
 ***************************************************************************/
/* TM :: GPIO_DATA :: RSVD [31:20] */
#define BCHP_TM_GPIO_DATA_RSVD_MASK                                0xfff00000
#define BCHP_TM_GPIO_DATA_RSVD_SHIFT                               20
#define BCHP_TM_GPIO_DATA_RSVD_DEFAULT                             0x00000000

/* TM :: GPIO_DATA :: VAL [19:00] */
#define BCHP_TM_GPIO_DATA_VAL_MASK                                 0x000fffff
#define BCHP_TM_GPIO_DATA_VAL_SHIFT                                0
#define BCHP_TM_GPIO_DATA_VAL_DEFAULT                              0x00000000

/***************************************************************************
 *GPIO_MUX_CTRL1 - GPIO Multiplexing Control 1 Register
 ***************************************************************************/
/* TM :: GPIO_MUX_CTRL1 :: RSVD_7 [31:31] */
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_7_MASK                         0x80000000
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_7_SHIFT                        31
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_7_DEFAULT                      0x00000000

/* TM :: GPIO_MUX_CTRL1 :: GPIO_7 [30:28] */
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_7_MASK                         0x70000000
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_7_SHIFT                        28
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_7_DEFAULT                      0x00000000

/* TM :: GPIO_MUX_CTRL1 :: RSVD_6 [27:27] */
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_6_MASK                         0x08000000
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_6_SHIFT                        27
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_6_DEFAULT                      0x00000000

/* TM :: GPIO_MUX_CTRL1 :: GPIO_6 [26:24] */
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_6_MASK                         0x07000000
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_6_SHIFT                        24
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_6_DEFAULT                      0x00000000

/* TM :: GPIO_MUX_CTRL1 :: RSVD_5 [23:23] */
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_5_MASK                         0x00800000
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_5_SHIFT                        23
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_5_DEFAULT                      0x00000000

/* TM :: GPIO_MUX_CTRL1 :: GPIO_5 [22:20] */
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_5_MASK                         0x00700000
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_5_SHIFT                        20
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_5_DEFAULT                      0x00000000

/* TM :: GPIO_MUX_CTRL1 :: RSVD_4 [19:19] */
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_4_MASK                         0x00080000
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_4_SHIFT                        19
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_4_DEFAULT                      0x00000000

/* TM :: GPIO_MUX_CTRL1 :: GPIO_4 [18:16] */
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_4_MASK                         0x00070000
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_4_SHIFT                        16
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_4_DEFAULT                      0x00000000

/* TM :: GPIO_MUX_CTRL1 :: RSVD_3 [15:15] */
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_3_MASK                         0x00008000
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_3_SHIFT                        15
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_3_DEFAULT                      0x00000000

/* TM :: GPIO_MUX_CTRL1 :: GPIO_3 [14:12] */
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_3_MASK                         0x00007000
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_3_SHIFT                        12
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_3_DEFAULT                      0x00000000

/* TM :: GPIO_MUX_CTRL1 :: RSVD_2 [11:11] */
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_2_MASK                         0x00000800
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_2_SHIFT                        11
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_2_DEFAULT                      0x00000000

/* TM :: GPIO_MUX_CTRL1 :: GPIO_2 [10:08] */
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_2_MASK                         0x00000700
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_2_SHIFT                        8
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_2_DEFAULT                      0x00000000

/* TM :: GPIO_MUX_CTRL1 :: RSVD_1 [07:07] */
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_1_MASK                         0x00000080
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_1_SHIFT                        7
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_1_DEFAULT                      0x00000000

/* TM :: GPIO_MUX_CTRL1 :: GPIO_1 [06:04] */
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_1_MASK                         0x00000070
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_1_SHIFT                        4
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_1_DEFAULT                      0x00000000

/* TM :: GPIO_MUX_CTRL1 :: RSVD_0 [03:03] */
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_0_MASK                         0x00000008
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_0_SHIFT                        3
#define BCHP_TM_GPIO_MUX_CTRL1_RSVD_0_DEFAULT                      0x00000000

/* TM :: GPIO_MUX_CTRL1 :: GPIO_0 [02:00] */
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_0_MASK                         0x00000007
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_0_SHIFT                        0
#define BCHP_TM_GPIO_MUX_CTRL1_GPIO_0_DEFAULT                      0x00000000

/***************************************************************************
 *GPIO_MUX_CTRL2 - GPIO Multiplexing Control 2 Register
 ***************************************************************************/
/* TM :: GPIO_MUX_CTRL2 :: RSVD_15 [31:31] */
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_15_MASK                        0x80000000
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_15_SHIFT                       31
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_15_DEFAULT                     0x00000000

/* TM :: GPIO_MUX_CTRL2 :: GPIO_15 [30:28] */
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_15_MASK                        0x70000000
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_15_SHIFT                       28
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_15_DEFAULT                     0x00000000

/* TM :: GPIO_MUX_CTRL2 :: RSVD_14 [27:27] */
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_14_MASK                        0x08000000
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_14_SHIFT                       27
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_14_DEFAULT                     0x00000000

/* TM :: GPIO_MUX_CTRL2 :: GPIO_14 [26:24] */
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_14_MASK                        0x07000000
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_14_SHIFT                       24
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_14_DEFAULT                     0x00000000

/* TM :: GPIO_MUX_CTRL2 :: RSVD_13 [23:23] */
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_13_MASK                        0x00800000
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_13_SHIFT                       23
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_13_DEFAULT                     0x00000000

/* TM :: GPIO_MUX_CTRL2 :: GPIO_13 [22:20] */
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_13_MASK                        0x00700000
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_13_SHIFT                       20
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_13_DEFAULT                     0x00000000

/* TM :: GPIO_MUX_CTRL2 :: RSVD_12 [19:19] */
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_12_MASK                        0x00080000
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_12_SHIFT                       19
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_12_DEFAULT                     0x00000000

/* TM :: GPIO_MUX_CTRL2 :: GPIO_12 [18:16] */
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_12_MASK                        0x00070000
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_12_SHIFT                       16
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_12_DEFAULT                     0x00000000

/* TM :: GPIO_MUX_CTRL2 :: RSVD_11 [15:15] */
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_11_MASK                        0x00008000
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_11_SHIFT                       15
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_11_DEFAULT                     0x00000000

/* TM :: GPIO_MUX_CTRL2 :: GPIO_11 [14:12] */
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_11_MASK                        0x00007000
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_11_SHIFT                       12
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_11_DEFAULT                     0x00000000

/* TM :: GPIO_MUX_CTRL2 :: RSVD_10 [11:11] */
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_10_MASK                        0x00000800
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_10_SHIFT                       11
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_10_DEFAULT                     0x00000000

/* TM :: GPIO_MUX_CTRL2 :: GPIO_10 [10:08] */
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_10_MASK                        0x00000700
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_10_SHIFT                       8
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_10_DEFAULT                     0x00000000

/* TM :: GPIO_MUX_CTRL2 :: RSVD_9 [07:07] */
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_9_MASK                         0x00000080
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_9_SHIFT                        7
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_9_DEFAULT                      0x00000000

/* TM :: GPIO_MUX_CTRL2 :: GPIO_9 [06:04] */
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_9_MASK                         0x00000070
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_9_SHIFT                        4
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_9_DEFAULT                      0x00000000

/* TM :: GPIO_MUX_CTRL2 :: RSVD_8 [03:03] */
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_8_MASK                         0x00000008
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_8_SHIFT                        3
#define BCHP_TM_GPIO_MUX_CTRL2_RSVD_8_DEFAULT                      0x00000000

/* TM :: GPIO_MUX_CTRL2 :: GPIO_8 [02:00] */
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_8_MASK                         0x00000007
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_8_SHIFT                        0
#define BCHP_TM_GPIO_MUX_CTRL2_GPIO_8_DEFAULT                      0x00000000

/***************************************************************************
 *GPIO_MUX_CTRL3 - GPIO Multiplexing Control 3 Register
 ***************************************************************************/
/* TM :: GPIO_MUX_CTRL3 :: RSVD_19 [31:15] */
#define BCHP_TM_GPIO_MUX_CTRL3_RSVD_19_MASK                        0xffff8000
#define BCHP_TM_GPIO_MUX_CTRL3_RSVD_19_SHIFT                       15
#define BCHP_TM_GPIO_MUX_CTRL3_RSVD_19_DEFAULT                     0x00000000

/* TM :: GPIO_MUX_CTRL3 :: GPIO_19 [14:12] */
#define BCHP_TM_GPIO_MUX_CTRL3_GPIO_19_MASK                        0x00007000
#define BCHP_TM_GPIO_MUX_CTRL3_GPIO_19_SHIFT                       12
#define BCHP_TM_GPIO_MUX_CTRL3_GPIO_19_DEFAULT                     0x00000000

/* TM :: GPIO_MUX_CTRL3 :: RSVD_18 [11:11] */
#define BCHP_TM_GPIO_MUX_CTRL3_RSVD_18_MASK                        0x00000800
#define BCHP_TM_GPIO_MUX_CTRL3_RSVD_18_SHIFT                       11
#define BCHP_TM_GPIO_MUX_CTRL3_RSVD_18_DEFAULT                     0x00000000

/* TM :: GPIO_MUX_CTRL3 :: GPIO_18 [10:08] */
#define BCHP_TM_GPIO_MUX_CTRL3_GPIO_18_MASK                        0x00000700
#define BCHP_TM_GPIO_MUX_CTRL3_GPIO_18_SHIFT                       8
#define BCHP_TM_GPIO_MUX_CTRL3_GPIO_18_DEFAULT                     0x00000000

/* TM :: GPIO_MUX_CTRL3 :: RSVD_17 [07:07] */
#define BCHP_TM_GPIO_MUX_CTRL3_RSVD_17_MASK                        0x00000080
#define BCHP_TM_GPIO_MUX_CTRL3_RSVD_17_SHIFT                       7
#define BCHP_TM_GPIO_MUX_CTRL3_RSVD_17_DEFAULT                     0x00000000

/* TM :: GPIO_MUX_CTRL3 :: GPIO_17 [06:04] */
#define BCHP_TM_GPIO_MUX_CTRL3_GPIO_17_MASK                        0x00000070
#define BCHP_TM_GPIO_MUX_CTRL3_GPIO_17_SHIFT                       4
#define BCHP_TM_GPIO_MUX_CTRL3_GPIO_17_DEFAULT                     0x00000000

/* TM :: GPIO_MUX_CTRL3 :: RSVD_16 [03:03] */
#define BCHP_TM_GPIO_MUX_CTRL3_RSVD_16_MASK                        0x00000008
#define BCHP_TM_GPIO_MUX_CTRL3_RSVD_16_SHIFT                       3
#define BCHP_TM_GPIO_MUX_CTRL3_RSVD_16_DEFAULT                     0x00000000

/* TM :: GPIO_MUX_CTRL3 :: GPIO_16 [02:00] */
#define BCHP_TM_GPIO_MUX_CTRL3_GPIO_16_MASK                        0x00000007
#define BCHP_TM_GPIO_MUX_CTRL3_GPIO_16_SHIFT                       0
#define BCHP_TM_GPIO_MUX_CTRL3_GPIO_16_DEFAULT                     0x00000000

/***************************************************************************
 *GPO_EN - GPO Output Enable Control Register
 ***************************************************************************/
/* TM :: GPO_EN :: RSVD [31:19] */
#define BCHP_TM_GPO_EN_RSVD_MASK                                   0xfff80000
#define BCHP_TM_GPO_EN_RSVD_SHIFT                                  19
#define BCHP_TM_GPO_EN_RSVD_DEFAULT                                0x00000000

/* TM :: GPO_EN :: EN [18:00] */
#define BCHP_TM_GPO_EN_EN_MASK                                     0x0007ffff
#define BCHP_TM_GPO_EN_EN_SHIFT                                    0
#define BCHP_TM_GPO_EN_EN_DEFAULT                                  0x0007ffff

/***************************************************************************
 *GPO_OD - GPO Open Drain Control Register
 ***************************************************************************/
/* TM :: GPO_OD :: RSVD [31:19] */
#define BCHP_TM_GPO_OD_RSVD_MASK                                   0xfff80000
#define BCHP_TM_GPO_OD_RSVD_SHIFT                                  19
#define BCHP_TM_GPO_OD_RSVD_DEFAULT                                0x00000000

/* TM :: GPO_OD :: OD [18:00] */
#define BCHP_TM_GPO_OD_OD_MASK                                     0x0007ffff
#define BCHP_TM_GPO_OD_OD_SHIFT                                    0
#define BCHP_TM_GPO_OD_OD_DEFAULT                                  0x00000000

/***************************************************************************
 *GPO_DATA - GPO Data Control Register
 ***************************************************************************/
/* TM :: GPO_DATA :: RSVD [31:19] */
#define BCHP_TM_GPO_DATA_RSVD_MASK                                 0xfff80000
#define BCHP_TM_GPO_DATA_RSVD_SHIFT                                19
#define BCHP_TM_GPO_DATA_RSVD_DEFAULT                              0x00000000

/* TM :: GPO_DATA :: VAL [18:00] */
#define BCHP_TM_GPO_DATA_VAL_MASK                                  0x0007ffff
#define BCHP_TM_GPO_DATA_VAL_SHIFT                                 0
#define BCHP_TM_GPO_DATA_VAL_DEFAULT                               0x00000000

/***************************************************************************
 *GPO_MUX_CTRL1 - GPO Multiplexing Control 1 Register
 ***************************************************************************/
/* TM :: GPO_MUX_CTRL1 :: RSVD_7 [31:31] */
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_7_MASK                          0x80000000
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_7_SHIFT                         31
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_7_DEFAULT                       0x00000000

/* TM :: GPO_MUX_CTRL1 :: GPO_7 [30:28] */
#define BCHP_TM_GPO_MUX_CTRL1_GPO_7_MASK                           0x70000000
#define BCHP_TM_GPO_MUX_CTRL1_GPO_7_SHIFT                          28
#define BCHP_TM_GPO_MUX_CTRL1_GPO_7_DEFAULT                        0x00000000

/* TM :: GPO_MUX_CTRL1 :: RSVD_6 [27:27] */
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_6_MASK                          0x08000000
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_6_SHIFT                         27
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_6_DEFAULT                       0x00000000

/* TM :: GPO_MUX_CTRL1 :: GPO_6 [26:24] */
#define BCHP_TM_GPO_MUX_CTRL1_GPO_6_MASK                           0x07000000
#define BCHP_TM_GPO_MUX_CTRL1_GPO_6_SHIFT                          24
#define BCHP_TM_GPO_MUX_CTRL1_GPO_6_DEFAULT                        0x00000000

/* TM :: GPO_MUX_CTRL1 :: RSVD_5 [23:23] */
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_5_MASK                          0x00800000
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_5_SHIFT                         23
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_5_DEFAULT                       0x00000000

/* TM :: GPO_MUX_CTRL1 :: GPO_5 [22:20] */
#define BCHP_TM_GPO_MUX_CTRL1_GPO_5_MASK                           0x00700000
#define BCHP_TM_GPO_MUX_CTRL1_GPO_5_SHIFT                          20
#define BCHP_TM_GPO_MUX_CTRL1_GPO_5_DEFAULT                        0x00000000

/* TM :: GPO_MUX_CTRL1 :: RSVD_4 [19:19] */
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_4_MASK                          0x00080000
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_4_SHIFT                         19
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_4_DEFAULT                       0x00000000

/* TM :: GPO_MUX_CTRL1 :: GPO_4 [18:16] */
#define BCHP_TM_GPO_MUX_CTRL1_GPO_4_MASK                           0x00070000
#define BCHP_TM_GPO_MUX_CTRL1_GPO_4_SHIFT                          16
#define BCHP_TM_GPO_MUX_CTRL1_GPO_4_DEFAULT                        0x00000000

/* TM :: GPO_MUX_CTRL1 :: RSVD_3 [15:15] */
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_3_MASK                          0x00008000
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_3_SHIFT                         15
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_3_DEFAULT                       0x00000000

/* TM :: GPO_MUX_CTRL1 :: GPO_3 [14:12] */
#define BCHP_TM_GPO_MUX_CTRL1_GPO_3_MASK                           0x00007000
#define BCHP_TM_GPO_MUX_CTRL1_GPO_3_SHIFT                          12
#define BCHP_TM_GPO_MUX_CTRL1_GPO_3_DEFAULT                        0x00000000

/* TM :: GPO_MUX_CTRL1 :: RSVD_2 [11:11] */
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_2_MASK                          0x00000800
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_2_SHIFT                         11
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_2_DEFAULT                       0x00000000

/* TM :: GPO_MUX_CTRL1 :: GPO_2 [10:08] */
#define BCHP_TM_GPO_MUX_CTRL1_GPO_2_MASK                           0x00000700
#define BCHP_TM_GPO_MUX_CTRL1_GPO_2_SHIFT                          8
#define BCHP_TM_GPO_MUX_CTRL1_GPO_2_DEFAULT                        0x00000000

/* TM :: GPO_MUX_CTRL1 :: RSVD_1 [07:07] */
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_1_MASK                          0x00000080
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_1_SHIFT                         7
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_1_DEFAULT                       0x00000000

/* TM :: GPO_MUX_CTRL1 :: GPO_1 [06:04] */
#define BCHP_TM_GPO_MUX_CTRL1_GPO_1_MASK                           0x00000070
#define BCHP_TM_GPO_MUX_CTRL1_GPO_1_SHIFT                          4
#define BCHP_TM_GPO_MUX_CTRL1_GPO_1_DEFAULT                        0x00000000

/* TM :: GPO_MUX_CTRL1 :: RSVD_0 [03:03] */
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_0_MASK                          0x00000008
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_0_SHIFT                         3
#define BCHP_TM_GPO_MUX_CTRL1_RSVD_0_DEFAULT                       0x00000000

/* TM :: GPO_MUX_CTRL1 :: GPO_0 [02:00] */
#define BCHP_TM_GPO_MUX_CTRL1_GPO_0_MASK                           0x00000007
#define BCHP_TM_GPO_MUX_CTRL1_GPO_0_SHIFT                          0
#define BCHP_TM_GPO_MUX_CTRL1_GPO_0_DEFAULT                        0x00000000

/***************************************************************************
 *GPO_MUX_CTRL2 - GPO Multiplexing Control 2 Register
 ***************************************************************************/
/* TM :: GPO_MUX_CTRL2 :: RSVD_15 [31:31] */
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_15_MASK                         0x80000000
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_15_SHIFT                        31
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_15_DEFAULT                      0x00000000

/* TM :: GPO_MUX_CTRL2 :: GPO_15 [30:28] */
#define BCHP_TM_GPO_MUX_CTRL2_GPO_15_MASK                          0x70000000
#define BCHP_TM_GPO_MUX_CTRL2_GPO_15_SHIFT                         28
#define BCHP_TM_GPO_MUX_CTRL2_GPO_15_DEFAULT                       0x00000000

/* TM :: GPO_MUX_CTRL2 :: RSVD_14 [27:27] */
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_14_MASK                         0x08000000
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_14_SHIFT                        27
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_14_DEFAULT                      0x00000000

/* TM :: GPO_MUX_CTRL2 :: GPO_14 [26:24] */
#define BCHP_TM_GPO_MUX_CTRL2_GPO_14_MASK                          0x07000000
#define BCHP_TM_GPO_MUX_CTRL2_GPO_14_SHIFT                         24
#define BCHP_TM_GPO_MUX_CTRL2_GPO_14_DEFAULT                       0x00000000

/* TM :: GPO_MUX_CTRL2 :: RSVD_13 [23:23] */
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_13_MASK                         0x00800000
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_13_SHIFT                        23
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_13_DEFAULT                      0x00000000

/* TM :: GPO_MUX_CTRL2 :: GPO_13 [22:20] */
#define BCHP_TM_GPO_MUX_CTRL2_GPO_13_MASK                          0x00700000
#define BCHP_TM_GPO_MUX_CTRL2_GPO_13_SHIFT                         20
#define BCHP_TM_GPO_MUX_CTRL2_GPO_13_DEFAULT                       0x00000000

/* TM :: GPO_MUX_CTRL2 :: RSVD_12 [19:19] */
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_12_MASK                         0x00080000
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_12_SHIFT                        19
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_12_DEFAULT                      0x00000000

/* TM :: GPO_MUX_CTRL2 :: GPO_12 [18:16] */
#define BCHP_TM_GPO_MUX_CTRL2_GPO_12_MASK                          0x00070000
#define BCHP_TM_GPO_MUX_CTRL2_GPO_12_SHIFT                         16
#define BCHP_TM_GPO_MUX_CTRL2_GPO_12_DEFAULT                       0x00000000

/* TM :: GPO_MUX_CTRL2 :: RSVD_11 [15:15] */
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_11_MASK                         0x00008000
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_11_SHIFT                        15
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_11_DEFAULT                      0x00000000

/* TM :: GPO_MUX_CTRL2 :: GPO_11 [14:12] */
#define BCHP_TM_GPO_MUX_CTRL2_GPO_11_MASK                          0x00007000
#define BCHP_TM_GPO_MUX_CTRL2_GPO_11_SHIFT                         12
#define BCHP_TM_GPO_MUX_CTRL2_GPO_11_DEFAULT                       0x00000000

/* TM :: GPO_MUX_CTRL2 :: RSVD_10 [11:11] */
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_10_MASK                         0x00000800
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_10_SHIFT                        11
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_10_DEFAULT                      0x00000000

/* TM :: GPO_MUX_CTRL2 :: GPO_10 [10:08] */
#define BCHP_TM_GPO_MUX_CTRL2_GPO_10_MASK                          0x00000700
#define BCHP_TM_GPO_MUX_CTRL2_GPO_10_SHIFT                         8
#define BCHP_TM_GPO_MUX_CTRL2_GPO_10_DEFAULT                       0x00000000

/* TM :: GPO_MUX_CTRL2 :: RSVD_9 [07:07] */
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_9_MASK                          0x00000080
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_9_SHIFT                         7
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_9_DEFAULT                       0x00000000

/* TM :: GPO_MUX_CTRL2 :: GPO_9 [06:04] */
#define BCHP_TM_GPO_MUX_CTRL2_GPO_9_MASK                           0x00000070
#define BCHP_TM_GPO_MUX_CTRL2_GPO_9_SHIFT                          4
#define BCHP_TM_GPO_MUX_CTRL2_GPO_9_DEFAULT                        0x00000000

/* TM :: GPO_MUX_CTRL2 :: RSVD_8 [03:03] */
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_8_MASK                          0x00000008
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_8_SHIFT                         3
#define BCHP_TM_GPO_MUX_CTRL2_RSVD_8_DEFAULT                       0x00000000

/* TM :: GPO_MUX_CTRL2 :: GPO_8 [02:00] */
#define BCHP_TM_GPO_MUX_CTRL2_GPO_8_MASK                           0x00000007
#define BCHP_TM_GPO_MUX_CTRL2_GPO_8_SHIFT                          0
#define BCHP_TM_GPO_MUX_CTRL2_GPO_8_DEFAULT                        0x00000000

/***************************************************************************
 *GPO_MUX_CTRL3 - GPO Multiplexing Control 3 Register
 ***************************************************************************/
/* TM :: GPO_MUX_CTRL3 :: RSVD_18 [31:11] */
#define BCHP_TM_GPO_MUX_CTRL3_RSVD_18_MASK                         0xfffff800
#define BCHP_TM_GPO_MUX_CTRL3_RSVD_18_SHIFT                        11
#define BCHP_TM_GPO_MUX_CTRL3_RSVD_18_DEFAULT                      0x00000000

/* TM :: GPO_MUX_CTRL3 :: GPO_18 [10:08] */
#define BCHP_TM_GPO_MUX_CTRL3_GPO_18_MASK                          0x00000700
#define BCHP_TM_GPO_MUX_CTRL3_GPO_18_SHIFT                         8
#define BCHP_TM_GPO_MUX_CTRL3_GPO_18_DEFAULT                       0x00000000

/* TM :: GPO_MUX_CTRL3 :: RSVD_17 [07:07] */
#define BCHP_TM_GPO_MUX_CTRL3_RSVD_17_MASK                         0x00000080
#define BCHP_TM_GPO_MUX_CTRL3_RSVD_17_SHIFT                        7
#define BCHP_TM_GPO_MUX_CTRL3_RSVD_17_DEFAULT                      0x00000000

/* TM :: GPO_MUX_CTRL3 :: GPO_17 [06:04] */
#define BCHP_TM_GPO_MUX_CTRL3_GPO_17_MASK                          0x00000070
#define BCHP_TM_GPO_MUX_CTRL3_GPO_17_SHIFT                         4
#define BCHP_TM_GPO_MUX_CTRL3_GPO_17_DEFAULT                       0x00000000

/* TM :: GPO_MUX_CTRL3 :: RSVD_16 [03:03] */
#define BCHP_TM_GPO_MUX_CTRL3_RSVD_16_MASK                         0x00000008
#define BCHP_TM_GPO_MUX_CTRL3_RSVD_16_SHIFT                        3
#define BCHP_TM_GPO_MUX_CTRL3_RSVD_16_DEFAULT                      0x00000000

/* TM :: GPO_MUX_CTRL3 :: GPO_16 [02:00] */
#define BCHP_TM_GPO_MUX_CTRL3_GPO_16_MASK                          0x00000007
#define BCHP_TM_GPO_MUX_CTRL3_GPO_16_SHIFT                         0
#define BCHP_TM_GPO_MUX_CTRL3_GPO_16_DEFAULT                       0x00000000

/***************************************************************************
 *IRQ_CTRL - IRQ Control Register
 ***************************************************************************/
/* TM :: IRQ_CTRL :: RSVD_2 [31:08] */
#define BCHP_TM_IRQ_CTRL_RSVD_2_MASK                               0xffffff00
#define BCHP_TM_IRQ_CTRL_RSVD_2_SHIFT                              8
#define BCHP_TM_IRQ_CTRL_RSVD_2_DEFAULT                            0x00000000

/* TM :: IRQ_CTRL :: RSVD_1 [07:06] */
#define BCHP_TM_IRQ_CTRL_RSVD_1_MASK                               0x000000c0
#define BCHP_TM_IRQ_CTRL_RSVD_1_SHIFT                              6
#define BCHP_TM_IRQ_CTRL_RSVD_1_DEFAULT                            0x00000000

/* TM :: IRQ_CTRL :: OD [05:05] */
#define BCHP_TM_IRQ_CTRL_OD_MASK                                   0x00000020
#define BCHP_TM_IRQ_CTRL_OD_SHIFT                                  5
#define BCHP_TM_IRQ_CTRL_OD_DEFAULT                                0x00000001

/* TM :: IRQ_CTRL :: GPO_EN [04:04] */
#define BCHP_TM_IRQ_CTRL_GPO_EN_MASK                               0x00000010
#define BCHP_TM_IRQ_CTRL_GPO_EN_SHIFT                              4
#define BCHP_TM_IRQ_CTRL_GPO_EN_DEFAULT                            0x00000000

/* TM :: IRQ_CTRL :: RSVD_0 [03:01] */
#define BCHP_TM_IRQ_CTRL_RSVD_0_MASK                               0x0000000e
#define BCHP_TM_IRQ_CTRL_RSVD_0_SHIFT                              1
#define BCHP_TM_IRQ_CTRL_RSVD_0_DEFAULT                            0x00000000

/* TM :: IRQ_CTRL :: GPO_VAL [00:00] */
#define BCHP_TM_IRQ_CTRL_GPO_VAL_MASK                              0x00000001
#define BCHP_TM_IRQ_CTRL_GPO_VAL_SHIFT                             0
#define BCHP_TM_IRQ_CTRL_GPO_VAL_DEFAULT                           0x00000000

/***************************************************************************
 *BSC0_CTRL - Master BSC0 Control Register
 ***************************************************************************/
/* TM :: BSC0_CTRL :: RSVD_2 [31:08] */
#define BCHP_TM_BSC0_CTRL_RSVD_2_MASK                              0xffffff00
#define BCHP_TM_BSC0_CTRL_RSVD_2_SHIFT                             8
#define BCHP_TM_BSC0_CTRL_RSVD_2_DEFAULT                           0x00000000

/* TM :: BSC0_CTRL :: RSVD_1 [07:06] */
#define BCHP_TM_BSC0_CTRL_RSVD_1_MASK                              0x000000c0
#define BCHP_TM_BSC0_CTRL_RSVD_1_SHIFT                             6
#define BCHP_TM_BSC0_CTRL_RSVD_1_DEFAULT                           0x00000000

/* TM :: BSC0_CTRL :: REF_LVL [05:05] */
#define BCHP_TM_BSC0_CTRL_REF_LVL_MASK                             0x00000020
#define BCHP_TM_BSC0_CTRL_REF_LVL_SHIFT                            5
#define BCHP_TM_BSC0_CTRL_REF_LVL_DEFAULT                          0x00000000

/* TM :: BSC0_CTRL :: GPO_EN [04:04] */
#define BCHP_TM_BSC0_CTRL_GPO_EN_MASK                              0x00000010
#define BCHP_TM_BSC0_CTRL_GPO_EN_SHIFT                             4
#define BCHP_TM_BSC0_CTRL_GPO_EN_DEFAULT                           0x00000000

/* TM :: BSC0_CTRL :: RSVD_0 [03:02] */
#define BCHP_TM_BSC0_CTRL_RSVD_0_MASK                              0x0000000c
#define BCHP_TM_BSC0_CTRL_RSVD_0_SHIFT                             2
#define BCHP_TM_BSC0_CTRL_RSVD_0_DEFAULT                           0x00000000

/* TM :: BSC0_CTRL :: SCL_GPO_VAL [01:01] */
#define BCHP_TM_BSC0_CTRL_SCL_GPO_VAL_MASK                         0x00000002
#define BCHP_TM_BSC0_CTRL_SCL_GPO_VAL_SHIFT                        1
#define BCHP_TM_BSC0_CTRL_SCL_GPO_VAL_DEFAULT                      0x00000000

/* TM :: BSC0_CTRL :: SDA_GPO_VAL [00:00] */
#define BCHP_TM_BSC0_CTRL_SDA_GPO_VAL_MASK                         0x00000001
#define BCHP_TM_BSC0_CTRL_SDA_GPO_VAL_SHIFT                        0
#define BCHP_TM_BSC0_CTRL_SDA_GPO_VAL_DEFAULT                      0x00000000

/***************************************************************************
 *BSC1_CTRL - Master BSC1 Control Register
 ***************************************************************************/
/* TM :: BSC1_CTRL :: RSVD_2 [31:08] */
#define BCHP_TM_BSC1_CTRL_RSVD_2_MASK                              0xffffff00
#define BCHP_TM_BSC1_CTRL_RSVD_2_SHIFT                             8
#define BCHP_TM_BSC1_CTRL_RSVD_2_DEFAULT                           0x00000000

/* TM :: BSC1_CTRL :: RSVD_1 [07:06] */
#define BCHP_TM_BSC1_CTRL_RSVD_1_MASK                              0x000000c0
#define BCHP_TM_BSC1_CTRL_RSVD_1_SHIFT                             6
#define BCHP_TM_BSC1_CTRL_RSVD_1_DEFAULT                           0x00000000

/* TM :: BSC1_CTRL :: REF_LVL [05:05] */
#define BCHP_TM_BSC1_CTRL_REF_LVL_MASK                             0x00000020
#define BCHP_TM_BSC1_CTRL_REF_LVL_SHIFT                            5
#define BCHP_TM_BSC1_CTRL_REF_LVL_DEFAULT                          0x00000000

/* TM :: BSC1_CTRL :: GPO_EN [04:04] */
#define BCHP_TM_BSC1_CTRL_GPO_EN_MASK                              0x00000010
#define BCHP_TM_BSC1_CTRL_GPO_EN_SHIFT                             4
#define BCHP_TM_BSC1_CTRL_GPO_EN_DEFAULT                           0x00000000

/* TM :: BSC1_CTRL :: RSVD_0 [03:02] */
#define BCHP_TM_BSC1_CTRL_RSVD_0_MASK                              0x0000000c
#define BCHP_TM_BSC1_CTRL_RSVD_0_SHIFT                             2
#define BCHP_TM_BSC1_CTRL_RSVD_0_DEFAULT                           0x00000000

/* TM :: BSC1_CTRL :: SCL_GPO_VAL [01:01] */
#define BCHP_TM_BSC1_CTRL_SCL_GPO_VAL_MASK                         0x00000002
#define BCHP_TM_BSC1_CTRL_SCL_GPO_VAL_SHIFT                        1
#define BCHP_TM_BSC1_CTRL_SCL_GPO_VAL_DEFAULT                      0x00000000

/* TM :: BSC1_CTRL :: SDA_GPO_VAL [00:00] */
#define BCHP_TM_BSC1_CTRL_SDA_GPO_VAL_MASK                         0x00000001
#define BCHP_TM_BSC1_CTRL_SDA_GPO_VAL_SHIFT                        0
#define BCHP_TM_BSC1_CTRL_SDA_GPO_VAL_DEFAULT                      0x00000000

/***************************************************************************
 *MISC0 - Misc Control Register
 ***************************************************************************/
/* TM :: MISC0 :: MISC [31:01] */
#define BCHP_TM_MISC0_MISC_MASK                                    0xfffffffe
#define BCHP_TM_MISC0_MISC_SHIFT                                   1
#define BCHP_TM_MISC0_MISC_DEFAULT                                 0x00000000

/* TM :: MISC0 :: JTAG_DISABLE [00:00] */
#define BCHP_TM_MISC0_JTAG_DISABLE_MASK                            0x00000001
#define BCHP_TM_MISC0_JTAG_DISABLE_SHIFT                           0
#define BCHP_TM_MISC0_JTAG_DISABLE_DEFAULT                         0x00000001

/***************************************************************************
 *SFT7 - Software Control Register
 ***************************************************************************/
/* TM :: SFT7 :: SFT [31:00] */
#define BCHP_TM_SFT7_SFT_MASK                                      0xffffffff
#define BCHP_TM_SFT7_SFT_SHIFT                                     0
#define BCHP_TM_SFT7_SFT_DEFAULT                                   0x00000000

/***************************************************************************
 *SFT6 - Software Control Register
 ***************************************************************************/
/* TM :: SFT6 :: SFT [31:00] */
#define BCHP_TM_SFT6_SFT_MASK                                      0xffffffff
#define BCHP_TM_SFT6_SFT_SHIFT                                     0
#define BCHP_TM_SFT6_SFT_DEFAULT                                   0x00000000

/***************************************************************************
 *SFT5 - Software Control Register
 ***************************************************************************/
/* TM :: SFT5 :: SFT [31:00] */
#define BCHP_TM_SFT5_SFT_MASK                                      0xffffffff
#define BCHP_TM_SFT5_SFT_SHIFT                                     0
#define BCHP_TM_SFT5_SFT_DEFAULT                                   0x00000000

/***************************************************************************
 *SFT4 - Software Control Register
 ***************************************************************************/
/* TM :: SFT4 :: SFT [31:00] */
#define BCHP_TM_SFT4_SFT_MASK                                      0xffffffff
#define BCHP_TM_SFT4_SFT_SHIFT                                     0
#define BCHP_TM_SFT4_SFT_DEFAULT                                   0x00000000

/***************************************************************************
 *SFT3 - Software Control Register
 ***************************************************************************/
/* TM :: SFT3 :: SFT [31:00] */
#define BCHP_TM_SFT3_SFT_MASK                                      0xffffffff
#define BCHP_TM_SFT3_SFT_SHIFT                                     0
#define BCHP_TM_SFT3_SFT_DEFAULT                                   0x00000000

/***************************************************************************
 *SFT2 - Software Control Register
 ***************************************************************************/
/* TM :: SFT2 :: SFT [31:00] */
#define BCHP_TM_SFT2_SFT_MASK                                      0xffffffff
#define BCHP_TM_SFT2_SFT_SHIFT                                     0
#define BCHP_TM_SFT2_SFT_DEFAULT                                   0x00000000

/***************************************************************************
 *SFT1 - Software Control Register
 ***************************************************************************/
/* TM :: SFT1 :: SFT [31:00] */
#define BCHP_TM_SFT1_SFT_MASK                                      0xffffffff
#define BCHP_TM_SFT1_SFT_SHIFT                                     0
#define BCHP_TM_SFT1_SFT_DEFAULT                                   0x00000000

/***************************************************************************
 *SFT0 - Software Control Register
 ***************************************************************************/
/* TM :: SFT0 :: SFT [31:00] */
#define BCHP_TM_SFT0_SFT_MASK                                      0xffffffff
#define BCHP_TM_SFT0_SFT_SHIFT                                     0
#define BCHP_TM_SFT0_SFT_DEFAULT                                   0x00000000

/***************************************************************************
 *TP_DIAG_CTRL - Testport Diagnostic Control Register
 ***************************************************************************/
/* TM :: TP_DIAG_CTRL :: EN [31:31] */
#define BCHP_TM_TP_DIAG_CTRL_EN_MASK                               0x80000000
#define BCHP_TM_TP_DIAG_CTRL_EN_SHIFT                              31
#define BCHP_TM_TP_DIAG_CTRL_EN_DEFAULT                            0x00000000

/* TM :: TP_DIAG_CTRL :: RSVD [30:06] */
#define BCHP_TM_TP_DIAG_CTRL_RSVD_MASK                             0x7fffffc0
#define BCHP_TM_TP_DIAG_CTRL_RSVD_SHIFT                            6
#define BCHP_TM_TP_DIAG_CTRL_RSVD_DEFAULT                          0x00000000

/* TM :: TP_DIAG_CTRL :: SRC_SEL [05:00] */
#define BCHP_TM_TP_DIAG_CTRL_SRC_SEL_MASK                          0x0000003f
#define BCHP_TM_TP_DIAG_CTRL_SRC_SEL_SHIFT                         0
#define BCHP_TM_TP_DIAG_CTRL_SRC_SEL_DEFAULT                       0x00000000

/***************************************************************************
 *INT_DIAG_MUX_CTRL8 - Internal Diagnostic Mux Control Register
 ***************************************************************************/
/* TM :: INT_DIAG_MUX_CTRL8 :: SEL_35 [31:24] */
#define BCHP_TM_INT_DIAG_MUX_CTRL8_SEL_35_MASK                     0xff000000
#define BCHP_TM_INT_DIAG_MUX_CTRL8_SEL_35_SHIFT                    24
#define BCHP_TM_INT_DIAG_MUX_CTRL8_SEL_35_DEFAULT                  0x00000023

/* TM :: INT_DIAG_MUX_CTRL8 :: SEL_34 [23:16] */
#define BCHP_TM_INT_DIAG_MUX_CTRL8_SEL_34_MASK                     0x00ff0000
#define BCHP_TM_INT_DIAG_MUX_CTRL8_SEL_34_SHIFT                    16
#define BCHP_TM_INT_DIAG_MUX_CTRL8_SEL_34_DEFAULT                  0x00000022

/* TM :: INT_DIAG_MUX_CTRL8 :: SEL_33 [15:08] */
#define BCHP_TM_INT_DIAG_MUX_CTRL8_SEL_33_MASK                     0x0000ff00
#define BCHP_TM_INT_DIAG_MUX_CTRL8_SEL_33_SHIFT                    8
#define BCHP_TM_INT_DIAG_MUX_CTRL8_SEL_33_DEFAULT                  0x00000021

/* TM :: INT_DIAG_MUX_CTRL8 :: SEL_32 [07:00] */
#define BCHP_TM_INT_DIAG_MUX_CTRL8_SEL_32_MASK                     0x000000ff
#define BCHP_TM_INT_DIAG_MUX_CTRL8_SEL_32_SHIFT                    0
#define BCHP_TM_INT_DIAG_MUX_CTRL8_SEL_32_DEFAULT                  0x00000020

/***************************************************************************
 *INT_DIAG_MUX_CTRL7 - Internal Diagnostic Mux Control Register
 ***************************************************************************/
/* TM :: INT_DIAG_MUX_CTRL7 :: SEL_31 [31:24] */
#define BCHP_TM_INT_DIAG_MUX_CTRL7_SEL_31_MASK                     0xff000000
#define BCHP_TM_INT_DIAG_MUX_CTRL7_SEL_31_SHIFT                    24
#define BCHP_TM_INT_DIAG_MUX_CTRL7_SEL_31_DEFAULT                  0x0000001f

/* TM :: INT_DIAG_MUX_CTRL7 :: SEL_30 [23:16] */
#define BCHP_TM_INT_DIAG_MUX_CTRL7_SEL_30_MASK                     0x00ff0000
#define BCHP_TM_INT_DIAG_MUX_CTRL7_SEL_30_SHIFT                    16
#define BCHP_TM_INT_DIAG_MUX_CTRL7_SEL_30_DEFAULT                  0x0000001e

/* TM :: INT_DIAG_MUX_CTRL7 :: SEL_29 [15:08] */
#define BCHP_TM_INT_DIAG_MUX_CTRL7_SEL_29_MASK                     0x0000ff00
#define BCHP_TM_INT_DIAG_MUX_CTRL7_SEL_29_SHIFT                    8
#define BCHP_TM_INT_DIAG_MUX_CTRL7_SEL_29_DEFAULT                  0x0000001d

/* TM :: INT_DIAG_MUX_CTRL7 :: SEL_28 [07:00] */
#define BCHP_TM_INT_DIAG_MUX_CTRL7_SEL_28_MASK                     0x000000ff
#define BCHP_TM_INT_DIAG_MUX_CTRL7_SEL_28_SHIFT                    0
#define BCHP_TM_INT_DIAG_MUX_CTRL7_SEL_28_DEFAULT                  0x0000001c

/***************************************************************************
 *INT_DIAG_MUX_CTRL6 - Internal Diagnostic Mux Control Register
 ***************************************************************************/
/* TM :: INT_DIAG_MUX_CTRL6 :: SEL_27 [31:24] */
#define BCHP_TM_INT_DIAG_MUX_CTRL6_SEL_27_MASK                     0xff000000
#define BCHP_TM_INT_DIAG_MUX_CTRL6_SEL_27_SHIFT                    24
#define BCHP_TM_INT_DIAG_MUX_CTRL6_SEL_27_DEFAULT                  0x0000001b

/* TM :: INT_DIAG_MUX_CTRL6 :: SEL_26 [23:16] */
#define BCHP_TM_INT_DIAG_MUX_CTRL6_SEL_26_MASK                     0x00ff0000
#define BCHP_TM_INT_DIAG_MUX_CTRL6_SEL_26_SHIFT                    16
#define BCHP_TM_INT_DIAG_MUX_CTRL6_SEL_26_DEFAULT                  0x0000001a

/* TM :: INT_DIAG_MUX_CTRL6 :: SEL_25 [15:08] */
#define BCHP_TM_INT_DIAG_MUX_CTRL6_SEL_25_MASK                     0x0000ff00
#define BCHP_TM_INT_DIAG_MUX_CTRL6_SEL_25_SHIFT                    8
#define BCHP_TM_INT_DIAG_MUX_CTRL6_SEL_25_DEFAULT                  0x00000019

/* TM :: INT_DIAG_MUX_CTRL6 :: SEL_24 [07:00] */
#define BCHP_TM_INT_DIAG_MUX_CTRL6_SEL_24_MASK                     0x000000ff
#define BCHP_TM_INT_DIAG_MUX_CTRL6_SEL_24_SHIFT                    0
#define BCHP_TM_INT_DIAG_MUX_CTRL6_SEL_24_DEFAULT                  0x00000018

/***************************************************************************
 *INT_DIAG_MUX_CTRL5 - Internal Diagnostic Mux Control Register
 ***************************************************************************/
/* TM :: INT_DIAG_MUX_CTRL5 :: SEL_23 [31:24] */
#define BCHP_TM_INT_DIAG_MUX_CTRL5_SEL_23_MASK                     0xff000000
#define BCHP_TM_INT_DIAG_MUX_CTRL5_SEL_23_SHIFT                    24
#define BCHP_TM_INT_DIAG_MUX_CTRL5_SEL_23_DEFAULT                  0x00000017

/* TM :: INT_DIAG_MUX_CTRL5 :: SEL_22 [23:16] */
#define BCHP_TM_INT_DIAG_MUX_CTRL5_SEL_22_MASK                     0x00ff0000
#define BCHP_TM_INT_DIAG_MUX_CTRL5_SEL_22_SHIFT                    16
#define BCHP_TM_INT_DIAG_MUX_CTRL5_SEL_22_DEFAULT                  0x00000016

/* TM :: INT_DIAG_MUX_CTRL5 :: SEL_21 [15:08] */
#define BCHP_TM_INT_DIAG_MUX_CTRL5_SEL_21_MASK                     0x0000ff00
#define BCHP_TM_INT_DIAG_MUX_CTRL5_SEL_21_SHIFT                    8
#define BCHP_TM_INT_DIAG_MUX_CTRL5_SEL_21_DEFAULT                  0x00000015

/* TM :: INT_DIAG_MUX_CTRL5 :: SEL_20 [07:00] */
#define BCHP_TM_INT_DIAG_MUX_CTRL5_SEL_20_MASK                     0x000000ff
#define BCHP_TM_INT_DIAG_MUX_CTRL5_SEL_20_SHIFT                    0
#define BCHP_TM_INT_DIAG_MUX_CTRL5_SEL_20_DEFAULT                  0x00000014

/***************************************************************************
 *INT_DIAG_MUX_CTRL4 - Internal Diagnostic Mux Control Register
 ***************************************************************************/
/* TM :: INT_DIAG_MUX_CTRL4 :: SEL_19 [31:24] */
#define BCHP_TM_INT_DIAG_MUX_CTRL4_SEL_19_MASK                     0xff000000
#define BCHP_TM_INT_DIAG_MUX_CTRL4_SEL_19_SHIFT                    24
#define BCHP_TM_INT_DIAG_MUX_CTRL4_SEL_19_DEFAULT                  0x00000013

/* TM :: INT_DIAG_MUX_CTRL4 :: SEL_18 [23:16] */
#define BCHP_TM_INT_DIAG_MUX_CTRL4_SEL_18_MASK                     0x00ff0000
#define BCHP_TM_INT_DIAG_MUX_CTRL4_SEL_18_SHIFT                    16
#define BCHP_TM_INT_DIAG_MUX_CTRL4_SEL_18_DEFAULT                  0x00000012

/* TM :: INT_DIAG_MUX_CTRL4 :: SEL_17 [15:08] */
#define BCHP_TM_INT_DIAG_MUX_CTRL4_SEL_17_MASK                     0x0000ff00
#define BCHP_TM_INT_DIAG_MUX_CTRL4_SEL_17_SHIFT                    8
#define BCHP_TM_INT_DIAG_MUX_CTRL4_SEL_17_DEFAULT                  0x00000011

/* TM :: INT_DIAG_MUX_CTRL4 :: SEL_16 [07:00] */
#define BCHP_TM_INT_DIAG_MUX_CTRL4_SEL_16_MASK                     0x000000ff
#define BCHP_TM_INT_DIAG_MUX_CTRL4_SEL_16_SHIFT                    0
#define BCHP_TM_INT_DIAG_MUX_CTRL4_SEL_16_DEFAULT                  0x00000010

/***************************************************************************
 *INT_DIAG_MUX_CTRL3 - Internal Diagnostic Mux Control Register
 ***************************************************************************/
/* TM :: INT_DIAG_MUX_CTRL3 :: SEL_15 [31:24] */
#define BCHP_TM_INT_DIAG_MUX_CTRL3_SEL_15_MASK                     0xff000000
#define BCHP_TM_INT_DIAG_MUX_CTRL3_SEL_15_SHIFT                    24
#define BCHP_TM_INT_DIAG_MUX_CTRL3_SEL_15_DEFAULT                  0x0000000f

/* TM :: INT_DIAG_MUX_CTRL3 :: SEL_14 [23:16] */
#define BCHP_TM_INT_DIAG_MUX_CTRL3_SEL_14_MASK                     0x00ff0000
#define BCHP_TM_INT_DIAG_MUX_CTRL3_SEL_14_SHIFT                    16
#define BCHP_TM_INT_DIAG_MUX_CTRL3_SEL_14_DEFAULT                  0x0000000e

/* TM :: INT_DIAG_MUX_CTRL3 :: SEL_13 [15:08] */
#define BCHP_TM_INT_DIAG_MUX_CTRL3_SEL_13_MASK                     0x0000ff00
#define BCHP_TM_INT_DIAG_MUX_CTRL3_SEL_13_SHIFT                    8
#define BCHP_TM_INT_DIAG_MUX_CTRL3_SEL_13_DEFAULT                  0x0000000d

/* TM :: INT_DIAG_MUX_CTRL3 :: SEL_12 [07:00] */
#define BCHP_TM_INT_DIAG_MUX_CTRL3_SEL_12_MASK                     0x000000ff
#define BCHP_TM_INT_DIAG_MUX_CTRL3_SEL_12_SHIFT                    0
#define BCHP_TM_INT_DIAG_MUX_CTRL3_SEL_12_DEFAULT                  0x0000000c

/***************************************************************************
 *INT_DIAG_MUX_CTRL2 - Internal Diagnostic Mux Control Register
 ***************************************************************************/
/* TM :: INT_DIAG_MUX_CTRL2 :: SEL_11 [31:24] */
#define BCHP_TM_INT_DIAG_MUX_CTRL2_SEL_11_MASK                     0xff000000
#define BCHP_TM_INT_DIAG_MUX_CTRL2_SEL_11_SHIFT                    24
#define BCHP_TM_INT_DIAG_MUX_CTRL2_SEL_11_DEFAULT                  0x0000000b

/* TM :: INT_DIAG_MUX_CTRL2 :: SEL_10 [23:16] */
#define BCHP_TM_INT_DIAG_MUX_CTRL2_SEL_10_MASK                     0x00ff0000
#define BCHP_TM_INT_DIAG_MUX_CTRL2_SEL_10_SHIFT                    16
#define BCHP_TM_INT_DIAG_MUX_CTRL2_SEL_10_DEFAULT                  0x0000000a

/* TM :: INT_DIAG_MUX_CTRL2 :: SEL_09 [15:08] */
#define BCHP_TM_INT_DIAG_MUX_CTRL2_SEL_09_MASK                     0x0000ff00
#define BCHP_TM_INT_DIAG_MUX_CTRL2_SEL_09_SHIFT                    8
#define BCHP_TM_INT_DIAG_MUX_CTRL2_SEL_09_DEFAULT                  0x00000009

/* TM :: INT_DIAG_MUX_CTRL2 :: SEL_08 [07:00] */
#define BCHP_TM_INT_DIAG_MUX_CTRL2_SEL_08_MASK                     0x000000ff
#define BCHP_TM_INT_DIAG_MUX_CTRL2_SEL_08_SHIFT                    0
#define BCHP_TM_INT_DIAG_MUX_CTRL2_SEL_08_DEFAULT                  0x00000008

/***************************************************************************
 *INT_DIAG_MUX_CTRL1 - Internal Diagnostic Mux Control Register
 ***************************************************************************/
/* TM :: INT_DIAG_MUX_CTRL1 :: SEL_07 [31:24] */
#define BCHP_TM_INT_DIAG_MUX_CTRL1_SEL_07_MASK                     0xff000000
#define BCHP_TM_INT_DIAG_MUX_CTRL1_SEL_07_SHIFT                    24
#define BCHP_TM_INT_DIAG_MUX_CTRL1_SEL_07_DEFAULT                  0x00000007

/* TM :: INT_DIAG_MUX_CTRL1 :: SEL_06 [23:16] */
#define BCHP_TM_INT_DIAG_MUX_CTRL1_SEL_06_MASK                     0x00ff0000
#define BCHP_TM_INT_DIAG_MUX_CTRL1_SEL_06_SHIFT                    16
#define BCHP_TM_INT_DIAG_MUX_CTRL1_SEL_06_DEFAULT                  0x00000006

/* TM :: INT_DIAG_MUX_CTRL1 :: SEL_05 [15:08] */
#define BCHP_TM_INT_DIAG_MUX_CTRL1_SEL_05_MASK                     0x0000ff00
#define BCHP_TM_INT_DIAG_MUX_CTRL1_SEL_05_SHIFT                    8
#define BCHP_TM_INT_DIAG_MUX_CTRL1_SEL_05_DEFAULT                  0x00000005

/* TM :: INT_DIAG_MUX_CTRL1 :: SEL_04 [07:00] */
#define BCHP_TM_INT_DIAG_MUX_CTRL1_SEL_04_MASK                     0x000000ff
#define BCHP_TM_INT_DIAG_MUX_CTRL1_SEL_04_SHIFT                    0
#define BCHP_TM_INT_DIAG_MUX_CTRL1_SEL_04_DEFAULT                  0x00000004

/***************************************************************************
 *INT_DIAG_MUX_CTRL0 - Internal Diagnostic Mux Control Register
 ***************************************************************************/
/* TM :: INT_DIAG_MUX_CTRL0 :: SEL_03 [31:24] */
#define BCHP_TM_INT_DIAG_MUX_CTRL0_SEL_03_MASK                     0xff000000
#define BCHP_TM_INT_DIAG_MUX_CTRL0_SEL_03_SHIFT                    24
#define BCHP_TM_INT_DIAG_MUX_CTRL0_SEL_03_DEFAULT                  0x00000003

/* TM :: INT_DIAG_MUX_CTRL0 :: SEL_02 [23:16] */
#define BCHP_TM_INT_DIAG_MUX_CTRL0_SEL_02_MASK                     0x00ff0000
#define BCHP_TM_INT_DIAG_MUX_CTRL0_SEL_02_SHIFT                    16
#define BCHP_TM_INT_DIAG_MUX_CTRL0_SEL_02_DEFAULT                  0x00000002

/* TM :: INT_DIAG_MUX_CTRL0 :: SEL_01 [15:08] */
#define BCHP_TM_INT_DIAG_MUX_CTRL0_SEL_01_MASK                     0x0000ff00
#define BCHP_TM_INT_DIAG_MUX_CTRL0_SEL_01_SHIFT                    8
#define BCHP_TM_INT_DIAG_MUX_CTRL0_SEL_01_DEFAULT                  0x00000001

/* TM :: INT_DIAG_MUX_CTRL0 :: SEL_00 [07:00] */
#define BCHP_TM_INT_DIAG_MUX_CTRL0_SEL_00_MASK                     0x000000ff
#define BCHP_TM_INT_DIAG_MUX_CTRL0_SEL_00_SHIFT                    0
#define BCHP_TM_INT_DIAG_MUX_CTRL0_SEL_00_DEFAULT                  0x00000000

/***************************************************************************
 *INT_DIAG_INV_CTRL1 - Internal Diagnostic Inversion Control Register
 ***************************************************************************/
/* TM :: INT_DIAG_INV_CTRL1 :: RSVD [31:04] */
#define BCHP_TM_INT_DIAG_INV_CTRL1_RSVD_MASK                       0xfffffff0
#define BCHP_TM_INT_DIAG_INV_CTRL1_RSVD_SHIFT                      4
#define BCHP_TM_INT_DIAG_INV_CTRL1_RSVD_DEFAULT                    0x00000000

/* TM :: INT_DIAG_INV_CTRL1 :: INV [03:00] */
#define BCHP_TM_INT_DIAG_INV_CTRL1_INV_MASK                        0x0000000f
#define BCHP_TM_INT_DIAG_INV_CTRL1_INV_SHIFT                       0
#define BCHP_TM_INT_DIAG_INV_CTRL1_INV_DEFAULT                     0x00000000

/***************************************************************************
 *INT_DIAG_INV_CTRL0 - Internal Diagnostic Inversion Control Register
 ***************************************************************************/
/* TM :: INT_DIAG_INV_CTRL0 :: INV [31:00] */
#define BCHP_TM_INT_DIAG_INV_CTRL0_INV_MASK                        0xffffffff
#define BCHP_TM_INT_DIAG_INV_CTRL0_INV_SHIFT                       0
#define BCHP_TM_INT_DIAG_INV_CTRL0_INV_DEFAULT                     0x00000000

/***************************************************************************
 *EXT_DIAG_MUX_CTRL8 - External Diagnostic Mux Control Register
 ***************************************************************************/
/* TM :: EXT_DIAG_MUX_CTRL8 :: SEL_35 [31:24] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL8_SEL_35_MASK                     0xff000000
#define BCHP_TM_EXT_DIAG_MUX_CTRL8_SEL_35_SHIFT                    24
#define BCHP_TM_EXT_DIAG_MUX_CTRL8_SEL_35_DEFAULT                  0x00000023

/* TM :: EXT_DIAG_MUX_CTRL8 :: SEL_34 [23:16] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL8_SEL_34_MASK                     0x00ff0000
#define BCHP_TM_EXT_DIAG_MUX_CTRL8_SEL_34_SHIFT                    16
#define BCHP_TM_EXT_DIAG_MUX_CTRL8_SEL_34_DEFAULT                  0x00000022

/* TM :: EXT_DIAG_MUX_CTRL8 :: SEL_33 [15:08] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL8_SEL_33_MASK                     0x0000ff00
#define BCHP_TM_EXT_DIAG_MUX_CTRL8_SEL_33_SHIFT                    8
#define BCHP_TM_EXT_DIAG_MUX_CTRL8_SEL_33_DEFAULT                  0x00000021

/* TM :: EXT_DIAG_MUX_CTRL8 :: SEL_32 [07:00] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL8_SEL_32_MASK                     0x000000ff
#define BCHP_TM_EXT_DIAG_MUX_CTRL8_SEL_32_SHIFT                    0
#define BCHP_TM_EXT_DIAG_MUX_CTRL8_SEL_32_DEFAULT                  0x00000020

/***************************************************************************
 *EXT_DIAG_MUX_CTRL7 - External Diagnostic Mux Control Register
 ***************************************************************************/
/* TM :: EXT_DIAG_MUX_CTRL7 :: SEL_31 [31:24] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL7_SEL_31_MASK                     0xff000000
#define BCHP_TM_EXT_DIAG_MUX_CTRL7_SEL_31_SHIFT                    24
#define BCHP_TM_EXT_DIAG_MUX_CTRL7_SEL_31_DEFAULT                  0x0000001f

/* TM :: EXT_DIAG_MUX_CTRL7 :: SEL_30 [23:16] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL7_SEL_30_MASK                     0x00ff0000
#define BCHP_TM_EXT_DIAG_MUX_CTRL7_SEL_30_SHIFT                    16
#define BCHP_TM_EXT_DIAG_MUX_CTRL7_SEL_30_DEFAULT                  0x0000001e

/* TM :: EXT_DIAG_MUX_CTRL7 :: SEL_29 [15:08] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL7_SEL_29_MASK                     0x0000ff00
#define BCHP_TM_EXT_DIAG_MUX_CTRL7_SEL_29_SHIFT                    8
#define BCHP_TM_EXT_DIAG_MUX_CTRL7_SEL_29_DEFAULT                  0x0000001d

/* TM :: EXT_DIAG_MUX_CTRL7 :: SEL_28 [07:00] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL7_SEL_28_MASK                     0x000000ff
#define BCHP_TM_EXT_DIAG_MUX_CTRL7_SEL_28_SHIFT                    0
#define BCHP_TM_EXT_DIAG_MUX_CTRL7_SEL_28_DEFAULT                  0x0000001c

/***************************************************************************
 *EXT_DIAG_MUX_CTRL6 - External Diagnostic Mux Control Register
 ***************************************************************************/
/* TM :: EXT_DIAG_MUX_CTRL6 :: SEL_27 [31:24] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL6_SEL_27_MASK                     0xff000000
#define BCHP_TM_EXT_DIAG_MUX_CTRL6_SEL_27_SHIFT                    24
#define BCHP_TM_EXT_DIAG_MUX_CTRL6_SEL_27_DEFAULT                  0x0000001b

/* TM :: EXT_DIAG_MUX_CTRL6 :: SEL_26 [23:16] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL6_SEL_26_MASK                     0x00ff0000
#define BCHP_TM_EXT_DIAG_MUX_CTRL6_SEL_26_SHIFT                    16
#define BCHP_TM_EXT_DIAG_MUX_CTRL6_SEL_26_DEFAULT                  0x0000001a

/* TM :: EXT_DIAG_MUX_CTRL6 :: SEL_25 [15:08] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL6_SEL_25_MASK                     0x0000ff00
#define BCHP_TM_EXT_DIAG_MUX_CTRL6_SEL_25_SHIFT                    8
#define BCHP_TM_EXT_DIAG_MUX_CTRL6_SEL_25_DEFAULT                  0x00000019

/* TM :: EXT_DIAG_MUX_CTRL6 :: SEL_24 [07:00] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL6_SEL_24_MASK                     0x000000ff
#define BCHP_TM_EXT_DIAG_MUX_CTRL6_SEL_24_SHIFT                    0
#define BCHP_TM_EXT_DIAG_MUX_CTRL6_SEL_24_DEFAULT                  0x00000018

/***************************************************************************
 *EXT_DIAG_MUX_CTRL5 - External Diagnostic Mux Control Register
 ***************************************************************************/
/* TM :: EXT_DIAG_MUX_CTRL5 :: SEL_23 [31:24] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL5_SEL_23_MASK                     0xff000000
#define BCHP_TM_EXT_DIAG_MUX_CTRL5_SEL_23_SHIFT                    24
#define BCHP_TM_EXT_DIAG_MUX_CTRL5_SEL_23_DEFAULT                  0x00000017

/* TM :: EXT_DIAG_MUX_CTRL5 :: SEL_22 [23:16] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL5_SEL_22_MASK                     0x00ff0000
#define BCHP_TM_EXT_DIAG_MUX_CTRL5_SEL_22_SHIFT                    16
#define BCHP_TM_EXT_DIAG_MUX_CTRL5_SEL_22_DEFAULT                  0x00000016

/* TM :: EXT_DIAG_MUX_CTRL5 :: SEL_21 [15:08] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL5_SEL_21_MASK                     0x0000ff00
#define BCHP_TM_EXT_DIAG_MUX_CTRL5_SEL_21_SHIFT                    8
#define BCHP_TM_EXT_DIAG_MUX_CTRL5_SEL_21_DEFAULT                  0x00000015

/* TM :: EXT_DIAG_MUX_CTRL5 :: SEL_20 [07:00] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL5_SEL_20_MASK                     0x000000ff
#define BCHP_TM_EXT_DIAG_MUX_CTRL5_SEL_20_SHIFT                    0
#define BCHP_TM_EXT_DIAG_MUX_CTRL5_SEL_20_DEFAULT                  0x00000014

/***************************************************************************
 *EXT_DIAG_MUX_CTRL4 - External Diagnostic Mux Control Register
 ***************************************************************************/
/* TM :: EXT_DIAG_MUX_CTRL4 :: SEL_19 [31:24] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL4_SEL_19_MASK                     0xff000000
#define BCHP_TM_EXT_DIAG_MUX_CTRL4_SEL_19_SHIFT                    24
#define BCHP_TM_EXT_DIAG_MUX_CTRL4_SEL_19_DEFAULT                  0x00000013

/* TM :: EXT_DIAG_MUX_CTRL4 :: SEL_18 [23:16] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL4_SEL_18_MASK                     0x00ff0000
#define BCHP_TM_EXT_DIAG_MUX_CTRL4_SEL_18_SHIFT                    16
#define BCHP_TM_EXT_DIAG_MUX_CTRL4_SEL_18_DEFAULT                  0x00000012

/* TM :: EXT_DIAG_MUX_CTRL4 :: SEL_17 [15:08] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL4_SEL_17_MASK                     0x0000ff00
#define BCHP_TM_EXT_DIAG_MUX_CTRL4_SEL_17_SHIFT                    8
#define BCHP_TM_EXT_DIAG_MUX_CTRL4_SEL_17_DEFAULT                  0x00000011

/* TM :: EXT_DIAG_MUX_CTRL4 :: SEL_16 [07:00] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL4_SEL_16_MASK                     0x000000ff
#define BCHP_TM_EXT_DIAG_MUX_CTRL4_SEL_16_SHIFT                    0
#define BCHP_TM_EXT_DIAG_MUX_CTRL4_SEL_16_DEFAULT                  0x00000010

/***************************************************************************
 *EXT_DIAG_MUX_CTRL3 - External Diagnostic Mux Control Register
 ***************************************************************************/
/* TM :: EXT_DIAG_MUX_CTRL3 :: SEL_15 [31:24] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL3_SEL_15_MASK                     0xff000000
#define BCHP_TM_EXT_DIAG_MUX_CTRL3_SEL_15_SHIFT                    24
#define BCHP_TM_EXT_DIAG_MUX_CTRL3_SEL_15_DEFAULT                  0x0000000f

/* TM :: EXT_DIAG_MUX_CTRL3 :: SEL_14 [23:16] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL3_SEL_14_MASK                     0x00ff0000
#define BCHP_TM_EXT_DIAG_MUX_CTRL3_SEL_14_SHIFT                    16
#define BCHP_TM_EXT_DIAG_MUX_CTRL3_SEL_14_DEFAULT                  0x0000000e

/* TM :: EXT_DIAG_MUX_CTRL3 :: SEL_13 [15:08] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL3_SEL_13_MASK                     0x0000ff00
#define BCHP_TM_EXT_DIAG_MUX_CTRL3_SEL_13_SHIFT                    8
#define BCHP_TM_EXT_DIAG_MUX_CTRL3_SEL_13_DEFAULT                  0x0000000d

/* TM :: EXT_DIAG_MUX_CTRL3 :: SEL_12 [07:00] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL3_SEL_12_MASK                     0x000000ff
#define BCHP_TM_EXT_DIAG_MUX_CTRL3_SEL_12_SHIFT                    0
#define BCHP_TM_EXT_DIAG_MUX_CTRL3_SEL_12_DEFAULT                  0x0000000c

/***************************************************************************
 *EXT_DIAG_MUX_CTRL2 - External Diagnostic Mux Control Register
 ***************************************************************************/
/* TM :: EXT_DIAG_MUX_CTRL2 :: SEL_11 [31:24] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL2_SEL_11_MASK                     0xff000000
#define BCHP_TM_EXT_DIAG_MUX_CTRL2_SEL_11_SHIFT                    24
#define BCHP_TM_EXT_DIAG_MUX_CTRL2_SEL_11_DEFAULT                  0x0000000b

/* TM :: EXT_DIAG_MUX_CTRL2 :: SEL_10 [23:16] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL2_SEL_10_MASK                     0x00ff0000
#define BCHP_TM_EXT_DIAG_MUX_CTRL2_SEL_10_SHIFT                    16
#define BCHP_TM_EXT_DIAG_MUX_CTRL2_SEL_10_DEFAULT                  0x0000000a

/* TM :: EXT_DIAG_MUX_CTRL2 :: SEL_09 [15:08] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL2_SEL_09_MASK                     0x0000ff00
#define BCHP_TM_EXT_DIAG_MUX_CTRL2_SEL_09_SHIFT                    8
#define BCHP_TM_EXT_DIAG_MUX_CTRL2_SEL_09_DEFAULT                  0x00000009

/* TM :: EXT_DIAG_MUX_CTRL2 :: SEL_08 [07:00] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL2_SEL_08_MASK                     0x000000ff
#define BCHP_TM_EXT_DIAG_MUX_CTRL2_SEL_08_SHIFT                    0
#define BCHP_TM_EXT_DIAG_MUX_CTRL2_SEL_08_DEFAULT                  0x00000008

/***************************************************************************
 *EXT_DIAG_MUX_CTRL1 - External Diagnostic Mux Control Register
 ***************************************************************************/
/* TM :: EXT_DIAG_MUX_CTRL1 :: SEL_07 [31:24] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL1_SEL_07_MASK                     0xff000000
#define BCHP_TM_EXT_DIAG_MUX_CTRL1_SEL_07_SHIFT                    24
#define BCHP_TM_EXT_DIAG_MUX_CTRL1_SEL_07_DEFAULT                  0x00000007

/* TM :: EXT_DIAG_MUX_CTRL1 :: SEL_06 [23:16] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL1_SEL_06_MASK                     0x00ff0000
#define BCHP_TM_EXT_DIAG_MUX_CTRL1_SEL_06_SHIFT                    16
#define BCHP_TM_EXT_DIAG_MUX_CTRL1_SEL_06_DEFAULT                  0x00000006

/* TM :: EXT_DIAG_MUX_CTRL1 :: SEL_05 [15:08] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL1_SEL_05_MASK                     0x0000ff00
#define BCHP_TM_EXT_DIAG_MUX_CTRL1_SEL_05_SHIFT                    8
#define BCHP_TM_EXT_DIAG_MUX_CTRL1_SEL_05_DEFAULT                  0x00000005

/* TM :: EXT_DIAG_MUX_CTRL1 :: SEL_04 [07:00] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL1_SEL_04_MASK                     0x000000ff
#define BCHP_TM_EXT_DIAG_MUX_CTRL1_SEL_04_SHIFT                    0
#define BCHP_TM_EXT_DIAG_MUX_CTRL1_SEL_04_DEFAULT                  0x00000004

/***************************************************************************
 *EXT_DIAG_MUX_CTRL0 - External Diagnostic Mux Control Register
 ***************************************************************************/
/* TM :: EXT_DIAG_MUX_CTRL0 :: SEL_03 [31:24] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL0_SEL_03_MASK                     0xff000000
#define BCHP_TM_EXT_DIAG_MUX_CTRL0_SEL_03_SHIFT                    24
#define BCHP_TM_EXT_DIAG_MUX_CTRL0_SEL_03_DEFAULT                  0x00000003

/* TM :: EXT_DIAG_MUX_CTRL0 :: SEL_02 [23:16] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL0_SEL_02_MASK                     0x00ff0000
#define BCHP_TM_EXT_DIAG_MUX_CTRL0_SEL_02_SHIFT                    16
#define BCHP_TM_EXT_DIAG_MUX_CTRL0_SEL_02_DEFAULT                  0x00000002

/* TM :: EXT_DIAG_MUX_CTRL0 :: SEL_01 [15:08] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL0_SEL_01_MASK                     0x0000ff00
#define BCHP_TM_EXT_DIAG_MUX_CTRL0_SEL_01_SHIFT                    8
#define BCHP_TM_EXT_DIAG_MUX_CTRL0_SEL_01_DEFAULT                  0x00000001

/* TM :: EXT_DIAG_MUX_CTRL0 :: SEL_00 [07:00] */
#define BCHP_TM_EXT_DIAG_MUX_CTRL0_SEL_00_MASK                     0x000000ff
#define BCHP_TM_EXT_DIAG_MUX_CTRL0_SEL_00_SHIFT                    0
#define BCHP_TM_EXT_DIAG_MUX_CTRL0_SEL_00_DEFAULT                  0x00000000

/***************************************************************************
 *EXT_DIAG_INV_CTRL1 - External Diagnostic Inversion Control Register
 ***************************************************************************/
/* TM :: EXT_DIAG_INV_CTRL1 :: RSVD [31:04] */
#define BCHP_TM_EXT_DIAG_INV_CTRL1_RSVD_MASK                       0xfffffff0
#define BCHP_TM_EXT_DIAG_INV_CTRL1_RSVD_SHIFT                      4
#define BCHP_TM_EXT_DIAG_INV_CTRL1_RSVD_DEFAULT                    0x00000000

/* TM :: EXT_DIAG_INV_CTRL1 :: INV [03:00] */
#define BCHP_TM_EXT_DIAG_INV_CTRL1_INV_MASK                        0x0000000f
#define BCHP_TM_EXT_DIAG_INV_CTRL1_INV_SHIFT                       0
#define BCHP_TM_EXT_DIAG_INV_CTRL1_INV_DEFAULT                     0x00000000

/***************************************************************************
 *EXT_DIAG_INV_CTRL0 - External Diagnostic Inversion Control Register
 ***************************************************************************/
/* TM :: EXT_DIAG_INV_CTRL0 :: INV [31:00] */
#define BCHP_TM_EXT_DIAG_INV_CTRL0_INV_MASK                        0xffffffff
#define BCHP_TM_EXT_DIAG_INV_CTRL0_INV_SHIFT                       0
#define BCHP_TM_EXT_DIAG_INV_CTRL0_INV_DEFAULT                     0x00000000

/***************************************************************************
 *EXT_DIAG_TP_OUT_READ - Ext. Diag. / TP Out Read Control Register
 ***************************************************************************/
/* TM :: EXT_DIAG_TP_OUT_READ :: EXT_DIAG_OUT [31:00] */
#define BCHP_TM_EXT_DIAG_TP_OUT_READ_EXT_DIAG_OUT_MASK             0xffffffff
#define BCHP_TM_EXT_DIAG_TP_OUT_READ_EXT_DIAG_OUT_SHIFT            0
#define BCHP_TM_EXT_DIAG_TP_OUT_READ_EXT_DIAG_OUT_DEFAULT          0x00000000

/***************************************************************************
 *RO_TEST_CTRL0 - RO Test Enable Control Register 0
 ***************************************************************************/
/* TM :: RO_TEST_CTRL0 :: RSVD [31:24] */
#define BCHP_TM_RO_TEST_CTRL0_RSVD_MASK                            0xff000000
#define BCHP_TM_RO_TEST_CTRL0_RSVD_SHIFT                           24
#define BCHP_TM_RO_TEST_CTRL0_RSVD_DEFAULT                         0x00000000

/* TM :: RO_TEST_CTRL0 :: SELECT [23:12] */
#define BCHP_TM_RO_TEST_CTRL0_SELECT_MASK                          0x00fff000
#define BCHP_TM_RO_TEST_CTRL0_SELECT_SHIFT                         12
#define BCHP_TM_RO_TEST_CTRL0_SELECT_DEFAULT                       0x00000000

/* TM :: RO_TEST_CTRL0 :: ENABLE [11:00] */
#define BCHP_TM_RO_TEST_CTRL0_ENABLE_MASK                          0x00000fff
#define BCHP_TM_RO_TEST_CTRL0_ENABLE_SHIFT                         0
#define BCHP_TM_RO_TEST_CTRL0_ENABLE_DEFAULT                       0x00000000

/***************************************************************************
 *RO_TEST_CTRL1 - RO Test Enable Control Register 1
 ***************************************************************************/
/* TM :: RO_TEST_CTRL1 :: RSVD [31:24] */
#define BCHP_TM_RO_TEST_CTRL1_RSVD_MASK                            0xff000000
#define BCHP_TM_RO_TEST_CTRL1_RSVD_SHIFT                           24
#define BCHP_TM_RO_TEST_CTRL1_RSVD_DEFAULT                         0x00000000

/* TM :: RO_TEST_CTRL1 :: SELECT [23:12] */
#define BCHP_TM_RO_TEST_CTRL1_SELECT_MASK                          0x00fff000
#define BCHP_TM_RO_TEST_CTRL1_SELECT_SHIFT                         12
#define BCHP_TM_RO_TEST_CTRL1_SELECT_DEFAULT                       0x00000000

/* TM :: RO_TEST_CTRL1 :: ENABLE [11:00] */
#define BCHP_TM_RO_TEST_CTRL1_ENABLE_MASK                          0x00000fff
#define BCHP_TM_RO_TEST_CTRL1_ENABLE_SHIFT                         0
#define BCHP_TM_RO_TEST_CTRL1_ENABLE_DEFAULT                       0x00000000

/***************************************************************************
 *DETECT2P5_CTRL - DETECT2P5 Control Register
 ***************************************************************************/
/* TM :: DETECT2P5_CTRL :: RSVD [31:09] */
#define BCHP_TM_DETECT2P5_CTRL_RSVD_MASK                           0xfffffe00
#define BCHP_TM_DETECT2P5_CTRL_RSVD_SHIFT                          9
#define BCHP_TM_DETECT2P5_CTRL_RSVD_DEFAULT                        0x00000000

/* TM :: DETECT2P5_CTRL :: 2P5_COMP [08:08] */
#define BCHP_TM_DETECT2P5_CTRL_2P5_COMP_MASK                       0x00000100
#define BCHP_TM_DETECT2P5_CTRL_2P5_COMP_SHIFT                      8
#define BCHP_TM_DETECT2P5_CTRL_2P5_COMP_DEFAULT                    0x00000000

/* TM :: DETECT2P5_CTRL :: CTRL [07:00] */
#define BCHP_TM_DETECT2P5_CTRL_CTRL_MASK                           0x000000ff
#define BCHP_TM_DETECT2P5_CTRL_CTRL_SHIFT                          0
#define BCHP_TM_DETECT2P5_CTRL_CTRL_DEFAULT                        0x00000000

/***************************************************************************
 *GPIO_READ - GPIO Read Control Register
 ***************************************************************************/
/* TM :: GPIO_READ :: RSVD [31:20] */
#define BCHP_TM_GPIO_READ_RSVD_MASK                                0xfff00000
#define BCHP_TM_GPIO_READ_RSVD_SHIFT                               20
#define BCHP_TM_GPIO_READ_RSVD_DEFAULT                             0x00000000

/* TM :: GPIO_READ :: GPIO_19_00 [19:00] */
#define BCHP_TM_GPIO_READ_GPIO_19_00_MASK                          0x000fffff
#define BCHP_TM_GPIO_READ_GPIO_19_00_SHIFT                         0
#define BCHP_TM_GPIO_READ_GPIO_19_00_DEFAULT                       0x00000000

/***************************************************************************
 *GPO_READ - GPO Read Control Register
 ***************************************************************************/
/* TM :: GPO_READ :: RSVD [31:15] */
#define BCHP_TM_GPO_READ_RSVD_MASK                                 0xffff8000
#define BCHP_TM_GPO_READ_RSVD_SHIFT                                15
#define BCHP_TM_GPO_READ_RSVD_DEFAULT                              0x00000000

/* TM :: GPO_READ :: GPO_14_00 [14:00] */
#define BCHP_TM_GPO_READ_GPO_14_00_MASK                            0x00007fff
#define BCHP_TM_GPO_READ_GPO_14_00_SHIFT                           0
#define BCHP_TM_GPO_READ_GPO_14_00_DEFAULT                         0x00000000

/***************************************************************************
 *SF_READ - SF Read Control Register
 ***************************************************************************/
/* TM :: SF_READ :: RSVD [31:05] */
#define BCHP_TM_SF_READ_RSVD_MASK                                  0xffffffe0
#define BCHP_TM_SF_READ_RSVD_SHIFT                                 5
#define BCHP_TM_SF_READ_RSVD_DEFAULT                               0x00000000

/* TM :: SF_READ :: SF_SCK [04:04] */
#define BCHP_TM_SF_READ_SF_SCK_MASK                                0x00000010
#define BCHP_TM_SF_READ_SF_SCK_SHIFT                               4
#define BCHP_TM_SF_READ_SF_SCK_DEFAULT                             0x00000000

/* TM :: SF_READ :: SF_HOLDB [03:03] */
#define BCHP_TM_SF_READ_SF_HOLDB_MASK                              0x00000008
#define BCHP_TM_SF_READ_SF_HOLDB_SHIFT                             3
#define BCHP_TM_SF_READ_SF_HOLDB_DEFAULT                           0x00000000

/* TM :: SF_READ :: SF_MOSI [02:02] */
#define BCHP_TM_SF_READ_SF_MOSI_MASK                               0x00000004
#define BCHP_TM_SF_READ_SF_MOSI_SHIFT                              2
#define BCHP_TM_SF_READ_SF_MOSI_DEFAULT                            0x00000000

/* TM :: SF_READ :: SF_MISO [01:01] */
#define BCHP_TM_SF_READ_SF_MISO_MASK                               0x00000002
#define BCHP_TM_SF_READ_SF_MISO_SHIFT                              1
#define BCHP_TM_SF_READ_SF_MISO_DEFAULT                            0x00000000

/* TM :: SF_READ :: SF_WPB [00:00] */
#define BCHP_TM_SF_READ_SF_WPB_MASK                                0x00000001
#define BCHP_TM_SF_READ_SF_WPB_SHIFT                               0
#define BCHP_TM_SF_READ_SF_WPB_DEFAULT                             0x00000000

/***************************************************************************
 *BSC_READ - BSC Read Control Register
 ***************************************************************************/
/* TM :: BSC_READ :: RSVD [31:10] */
#define BCHP_TM_BSC_READ_RSVD_MASK                                 0xfffffc00
#define BCHP_TM_BSC_READ_RSVD_SHIFT                                10
#define BCHP_TM_BSC_READ_RSVD_DEFAULT                              0x00000000

/* TM :: BSC_READ :: BAC_RSTB [09:09] */
#define BCHP_TM_BSC_READ_BAC_RSTB_MASK                             0x00000200
#define BCHP_TM_BSC_READ_BAC_RSTB_SHIFT                            9
#define BCHP_TM_BSC_READ_BAC_RSTB_DEFAULT                          0x00000000

/* TM :: BSC_READ :: BAC_SS [08:08] */
#define BCHP_TM_BSC_READ_BAC_SS_MASK                               0x00000100
#define BCHP_TM_BSC_READ_BAC_SS_SHIFT                              8
#define BCHP_TM_BSC_READ_BAC_SS_DEFAULT                            0x00000000

/* TM :: BSC_READ :: BAC_MOSI [07:07] */
#define BCHP_TM_BSC_READ_BAC_MOSI_MASK                             0x00000080
#define BCHP_TM_BSC_READ_BAC_MOSI_SHIFT                            7
#define BCHP_TM_BSC_READ_BAC_MOSI_DEFAULT                          0x00000000

/* TM :: BSC_READ :: BAC_SCK [06:06] */
#define BCHP_TM_BSC_READ_BAC_SCK_MASK                              0x00000040
#define BCHP_TM_BSC_READ_BAC_SCK_SHIFT                             6
#define BCHP_TM_BSC_READ_BAC_SCK_DEFAULT                           0x00000000

/* TM :: BSC_READ :: BAC_SCL [05:05] */
#define BCHP_TM_BSC_READ_BAC_SCL_MASK                              0x00000020
#define BCHP_TM_BSC_READ_BAC_SCL_SHIFT                             5
#define BCHP_TM_BSC_READ_BAC_SCL_DEFAULT                           0x00000001

/* TM :: BSC_READ :: BAC_SDA [04:04] */
#define BCHP_TM_BSC_READ_BAC_SDA_MASK                              0x00000010
#define BCHP_TM_BSC_READ_BAC_SDA_SHIFT                             4
#define BCHP_TM_BSC_READ_BAC_SDA_DEFAULT                           0x00000001

/* TM :: BSC_READ :: BSC1_SCL [03:03] */
#define BCHP_TM_BSC_READ_BSC1_SCL_MASK                             0x00000008
#define BCHP_TM_BSC_READ_BSC1_SCL_SHIFT                            3
#define BCHP_TM_BSC_READ_BSC1_SCL_DEFAULT                          0x00000001

/* TM :: BSC_READ :: BSC1_SDA [02:02] */
#define BCHP_TM_BSC_READ_BSC1_SDA_MASK                             0x00000004
#define BCHP_TM_BSC_READ_BSC1_SDA_SHIFT                            2
#define BCHP_TM_BSC_READ_BSC1_SDA_DEFAULT                          0x00000001

/* TM :: BSC_READ :: BSC0_SCL [01:01] */
#define BCHP_TM_BSC_READ_BSC0_SCL_MASK                             0x00000002
#define BCHP_TM_BSC_READ_BSC0_SCL_SHIFT                            1
#define BCHP_TM_BSC_READ_BSC0_SCL_DEFAULT                          0x00000001

/* TM :: BSC_READ :: BSC0_SDA [00:00] */
#define BCHP_TM_BSC_READ_BSC0_SDA_MASK                             0x00000001
#define BCHP_TM_BSC_READ_BSC0_SDA_SHIFT                            0
#define BCHP_TM_BSC_READ_BSC0_SDA_DEFAULT                          0x00000001

/***************************************************************************
 *EJTAG_READ - EJTAG Read Control Register
 ***************************************************************************/
/* TM :: EJTAG_READ :: RSVD [31:06] */
#define BCHP_TM_EJTAG_READ_RSVD_MASK                               0xffffffc0
#define BCHP_TM_EJTAG_READ_RSVD_SHIFT                              6
#define BCHP_TM_EJTAG_READ_RSVD_DEFAULT                            0x00000000

/* TM :: EJTAG_READ :: EJTAG_TDO [05:05] */
#define BCHP_TM_EJTAG_READ_EJTAG_TDO_MASK                          0x00000020
#define BCHP_TM_EJTAG_READ_EJTAG_TDO_SHIFT                         5
#define BCHP_TM_EJTAG_READ_EJTAG_TDO_DEFAULT                       0x00000000

/* TM :: EJTAG_READ :: EJTAG_TDI [04:04] */
#define BCHP_TM_EJTAG_READ_EJTAG_TDI_MASK                          0x00000010
#define BCHP_TM_EJTAG_READ_EJTAG_TDI_SHIFT                         4
#define BCHP_TM_EJTAG_READ_EJTAG_TDI_DEFAULT                       0x00000000

/* TM :: EJTAG_READ :: ETJAG_TMS [03:03] */
#define BCHP_TM_EJTAG_READ_ETJAG_TMS_MASK                          0x00000008
#define BCHP_TM_EJTAG_READ_ETJAG_TMS_SHIFT                         3
#define BCHP_TM_EJTAG_READ_ETJAG_TMS_DEFAULT                       0x00000000

/* TM :: EJTAG_READ :: EJTAG_TRSTb [02:02] */
#define BCHP_TM_EJTAG_READ_EJTAG_TRSTb_MASK                        0x00000004
#define BCHP_TM_EJTAG_READ_EJTAG_TRSTb_SHIFT                       2
#define BCHP_TM_EJTAG_READ_EJTAG_TRSTb_DEFAULT                     0x00000000

/* TM :: EJTAG_READ :: EJTAG_CE [01:01] */
#define BCHP_TM_EJTAG_READ_EJTAG_CE_MASK                           0x00000002
#define BCHP_TM_EJTAG_READ_EJTAG_CE_SHIFT                          1
#define BCHP_TM_EJTAG_READ_EJTAG_CE_DEFAULT                        0x00000000

/* TM :: EJTAG_READ :: EJTAG_TCK [00:00] */
#define BCHP_TM_EJTAG_READ_EJTAG_TCK_MASK                          0x00000001
#define BCHP_TM_EJTAG_READ_EJTAG_TCK_SHIFT                         0
#define BCHP_TM_EJTAG_READ_EJTAG_TCK_DEFAULT                       0x00000000

/***************************************************************************
 *MISC_READ - Misc Read Control Register
 ***************************************************************************/
/* TM :: MISC_READ :: RSVD [31:05] */
#define BCHP_TM_MISC_READ_RSVD_MASK                                0xffffffe0
#define BCHP_TM_MISC_READ_RSVD_SHIFT                               5
#define BCHP_TM_MISC_READ_RSVD_DEFAULT                             0x00000000

/* TM :: MISC_READ :: BYP_OSC [04:04] */
#define BCHP_TM_MISC_READ_BYP_OSC_MASK                             0x00000010
#define BCHP_TM_MISC_READ_BYP_OSC_SHIFT                            4
#define BCHP_TM_MISC_READ_BYP_OSC_DEFAULT                          0x00000000

/* TM :: MISC_READ :: TEST_MODE_3 [03:03] */
#define BCHP_TM_MISC_READ_TEST_MODE_3_MASK                         0x00000008
#define BCHP_TM_MISC_READ_TEST_MODE_3_SHIFT                        3
#define BCHP_TM_MISC_READ_TEST_MODE_3_DEFAULT                      0x00000000

/* TM :: MISC_READ :: TEST_MODE_2 [02:02] */
#define BCHP_TM_MISC_READ_TEST_MODE_2_MASK                         0x00000004
#define BCHP_TM_MISC_READ_TEST_MODE_2_SHIFT                        2
#define BCHP_TM_MISC_READ_TEST_MODE_2_DEFAULT                      0x00000000

/* TM :: MISC_READ :: TEST_MODE_1 [01:01] */
#define BCHP_TM_MISC_READ_TEST_MODE_1_MASK                         0x00000002
#define BCHP_TM_MISC_READ_TEST_MODE_1_SHIFT                        1
#define BCHP_TM_MISC_READ_TEST_MODE_1_DEFAULT                      0x00000000

/* TM :: MISC_READ :: TEST_MODE_0 [00:00] */
#define BCHP_TM_MISC_READ_TEST_MODE_0_MASK                         0x00000001
#define BCHP_TM_MISC_READ_TEST_MODE_0_SHIFT                        0
#define BCHP_TM_MISC_READ_TEST_MODE_0_DEFAULT                      0x00000000

/***************************************************************************
 *PIN_STRAP - Pin Strap Read Control Register
 ***************************************************************************/
/* TM :: PIN_STRAP :: RSVD [31:11] */
#define BCHP_TM_PIN_STRAP_RSVD_MASK                                0xfffff800
#define BCHP_TM_PIN_STRAP_RSVD_SHIFT                               11
#define BCHP_TM_PIN_STRAP_RSVD_DEFAULT                             0x00000000

/* TM :: PIN_STRAP :: OVR_STRAP_OSC_CML_OUT [10:10] */
#define BCHP_TM_PIN_STRAP_OVR_STRAP_OSC_CML_OUT_MASK               0x00000400
#define BCHP_TM_PIN_STRAP_OVR_STRAP_OSC_CML_OUT_SHIFT              10
#define BCHP_TM_PIN_STRAP_OVR_STRAP_OSC_CML_OUT_DEFAULT            0x00000000

/* TM :: PIN_STRAP :: OVR_STRAP_WBRX_LDO_EN [09:09] */
#define BCHP_TM_PIN_STRAP_OVR_STRAP_WBRX_LDO_EN_MASK               0x00000200
#define BCHP_TM_PIN_STRAP_OVR_STRAP_WBRX_LDO_EN_SHIFT              9
#define BCHP_TM_PIN_STRAP_OVR_STRAP_WBRX_LDO_EN_DEFAULT            0x00000000

/* TM :: PIN_STRAP :: STRAP_WBRX_LDO_EN [08:08] */
#define BCHP_TM_PIN_STRAP_STRAP_WBRX_LDO_EN_MASK                   0x00000100
#define BCHP_TM_PIN_STRAP_STRAP_WBRX_LDO_EN_SHIFT                  8
#define BCHP_TM_PIN_STRAP_STRAP_WBRX_LDO_EN_DEFAULT                0x00000000

/* TM :: PIN_STRAP :: STRAP_OSC_HIGHPASS [07:07] */
#define BCHP_TM_PIN_STRAP_STRAP_OSC_HIGHPASS_MASK                  0x00000080
#define BCHP_TM_PIN_STRAP_STRAP_OSC_HIGHPASS_SHIFT                 7
#define BCHP_TM_PIN_STRAP_STRAP_OSC_HIGHPASS_DEFAULT               0x00000001

/* TM :: PIN_STRAP :: STRAP_XCORE_BIAS [06:04] */
#define BCHP_TM_PIN_STRAP_STRAP_XCORE_BIAS_MASK                    0x00000070
#define BCHP_TM_PIN_STRAP_STRAP_XCORE_BIAS_SHIFT                   4
#define BCHP_TM_PIN_STRAP_STRAP_XCORE_BIAS_DEFAULT                 0x00000002

/* TM :: PIN_STRAP :: STARP_SPI_BSCB [03:03] */
#define BCHP_TM_PIN_STRAP_STARP_SPI_BSCB_MASK                      0x00000008
#define BCHP_TM_PIN_STRAP_STARP_SPI_BSCB_SHIFT                     3
#define BCHP_TM_PIN_STRAP_STARP_SPI_BSCB_DEFAULT                   0x00000001

/* TM :: PIN_STRAP :: STRAP_HIF_RATE [02:01] */
#define BCHP_TM_PIN_STRAP_STRAP_HIF_RATE_MASK                      0x00000006
#define BCHP_TM_PIN_STRAP_STRAP_HIF_RATE_SHIFT                     1
#define BCHP_TM_PIN_STRAP_STRAP_HIF_RATE_DEFAULT                   0x00000000

/* TM :: PIN_STRAP :: STRAP_ADDR2 [00:00] */
#define BCHP_TM_PIN_STRAP_STRAP_ADDR2_MASK                         0x00000001
#define BCHP_TM_PIN_STRAP_STRAP_ADDR2_SHIFT                        0
#define BCHP_TM_PIN_STRAP_STRAP_ADDR2_DEFAULT                      0x00000000

/***************************************************************************
 *DIAG_OUT1 - Testport / Diagnostic Read Control Register
 ***************************************************************************/
/* TM :: DIAG_OUT1 :: RSVD [31:04] */
#define BCHP_TM_DIAG_OUT1_RSVD_MASK                                0xfffffff0
#define BCHP_TM_DIAG_OUT1_RSVD_SHIFT                               4
#define BCHP_TM_DIAG_OUT1_RSVD_DEFAULT                             0x00000000

/* TM :: DIAG_OUT1 :: DIAG_OUT [03:00] */
#define BCHP_TM_DIAG_OUT1_DIAG_OUT_MASK                            0x0000000f
#define BCHP_TM_DIAG_OUT1_DIAG_OUT_SHIFT                           0
#define BCHP_TM_DIAG_OUT1_DIAG_OUT_DEFAULT                         0x00000000

/***************************************************************************
 *DIAG_OUT0 - Testport / Diagnostic Read Control Register
 ***************************************************************************/
/* TM :: DIAG_OUT0 :: DIAG_OUT [31:00] */
#define BCHP_TM_DIAG_OUT0_DIAG_OUT_MASK                            0xffffffff
#define BCHP_TM_DIAG_OUT0_DIAG_OUT_SHIFT                           0
#define BCHP_TM_DIAG_OUT0_DIAG_OUT_DEFAULT                         0x00000000

/***************************************************************************
 *TP_IN_VAL_SEL - TP_IN Enable Control Register
 ***************************************************************************/
/* TM :: TP_IN_VAL_SEL :: SEL [31:00] */
#define BCHP_TM_TP_IN_VAL_SEL_SEL_MASK                             0xffffffff
#define BCHP_TM_TP_IN_VAL_SEL_SEL_SHIFT                            0
#define BCHP_TM_TP_IN_VAL_SEL_SEL_DEFAULT                          0xffffffff

/***************************************************************************
 *TP_IN_VAL - TP_IN Value Control Register
 ***************************************************************************/
/* TM :: TP_IN_VAL :: VAL [31:00] */
#define BCHP_TM_TP_IN_VAL_VAL_MASK                                 0xffffffff
#define BCHP_TM_TP_IN_VAL_VAL_SHIFT                                0
#define BCHP_TM_TP_IN_VAL_VAL_DEFAULT                              0x00000000

/***************************************************************************
 *DAC_TEST_MODE_CTRL - DAC_TEST_MODE_CTRL Control Register
 ***************************************************************************/
/* TM :: DAC_TEST_MODE_CTRL :: RSVD [31:02] */
#define BCHP_TM_DAC_TEST_MODE_CTRL_RSVD_MASK                       0xfffffffc
#define BCHP_TM_DAC_TEST_MODE_CTRL_RSVD_SHIFT                      2
#define BCHP_TM_DAC_TEST_MODE_CTRL_RSVD_DEFAULT                    0x00000000

/* TM :: DAC_TEST_MODE_CTRL :: CLK_EDGE_SEL [01:01] */
#define BCHP_TM_DAC_TEST_MODE_CTRL_CLK_EDGE_SEL_MASK               0x00000002
#define BCHP_TM_DAC_TEST_MODE_CTRL_CLK_EDGE_SEL_SHIFT              1
#define BCHP_TM_DAC_TEST_MODE_CTRL_CLK_EDGE_SEL_DEFAULT            0x00000000

/* TM :: DAC_TEST_MODE_CTRL :: EN [00:00] */
#define BCHP_TM_DAC_TEST_MODE_CTRL_EN_MASK                         0x00000001
#define BCHP_TM_DAC_TEST_MODE_CTRL_EN_SHIFT                        0
#define BCHP_TM_DAC_TEST_MODE_CTRL_EN_DEFAULT                      0x00000000

/***************************************************************************
 *TEST_CTRL - Test Control Register
 ***************************************************************************/
/* TM :: TEST_CTRL :: MISC_CTRL [31:16] */
#define BCHP_TM_TEST_CTRL_MISC_CTRL_MASK                           0xffff0000
#define BCHP_TM_TEST_CTRL_MISC_CTRL_SHIFT                          16
#define BCHP_TM_TEST_CTRL_MISC_CTRL_DEFAULT                        0x00000000

/* TM :: TEST_CTRL :: FSK_CLK_BYP_SEL [15:15] */
#define BCHP_TM_TEST_CTRL_FSK_CLK_BYP_SEL_MASK                     0x00008000
#define BCHP_TM_TEST_CTRL_FSK_CLK_BYP_SEL_SHIFT                    15
#define BCHP_TM_TEST_CTRL_FSK_CLK_BYP_SEL_DEFAULT                  0x00000000

/* TM :: TEST_CTRL :: OPU2_CLK_BYP_SEL [14:14] */
#define BCHP_TM_TEST_CTRL_OPU2_CLK_BYP_SEL_MASK                    0x00004000
#define BCHP_TM_TEST_CTRL_OPU2_CLK_BYP_SEL_SHIFT                   14
#define BCHP_TM_TEST_CTRL_OPU2_CLK_BYP_SEL_DEFAULT                 0x00000000

/* TM :: TEST_CTRL :: OPU1_CLK_BYP_SEL [13:13] */
#define BCHP_TM_TEST_CTRL_OPU1_CLK_BYP_SEL_MASK                    0x00002000
#define BCHP_TM_TEST_CTRL_OPU1_CLK_BYP_SEL_SHIFT                   13
#define BCHP_TM_TEST_CTRL_OPU1_CLK_BYP_SEL_DEFAULT                 0x00000000

/* TM :: TEST_CTRL :: OPU0_CLK_BYP_SEL [12:12] */
#define BCHP_TM_TEST_CTRL_OPU0_CLK_BYP_SEL_MASK                    0x00001000
#define BCHP_TM_TEST_CTRL_OPU0_CLK_BYP_SEL_SHIFT                   12
#define BCHP_TM_TEST_CTRL_OPU0_CLK_BYP_SEL_DEFAULT                 0x00000000

/* TM :: TEST_CTRL :: CHAN3_CLK_BYP_SEL [11:11] */
#define BCHP_TM_TEST_CTRL_CHAN3_CLK_BYP_SEL_MASK                   0x00000800
#define BCHP_TM_TEST_CTRL_CHAN3_CLK_BYP_SEL_SHIFT                  11
#define BCHP_TM_TEST_CTRL_CHAN3_CLK_BYP_SEL_DEFAULT                0x00000000

/* TM :: TEST_CTRL :: CHAN2_CLK_BYP_SEL [10:10] */
#define BCHP_TM_TEST_CTRL_CHAN2_CLK_BYP_SEL_MASK                   0x00000400
#define BCHP_TM_TEST_CTRL_CHAN2_CLK_BYP_SEL_SHIFT                  10
#define BCHP_TM_TEST_CTRL_CHAN2_CLK_BYP_SEL_DEFAULT                0x00000000

/* TM :: TEST_CTRL :: CHAN1_CLK_BYP_SEL [09:09] */
#define BCHP_TM_TEST_CTRL_CHAN1_CLK_BYP_SEL_MASK                   0x00000200
#define BCHP_TM_TEST_CTRL_CHAN1_CLK_BYP_SEL_SHIFT                  9
#define BCHP_TM_TEST_CTRL_CHAN1_CLK_BYP_SEL_DEFAULT                0x00000000

/* TM :: TEST_CTRL :: CHAN0_CLK_BYP_SEL [08:08] */
#define BCHP_TM_TEST_CTRL_CHAN0_CLK_BYP_SEL_MASK                   0x00000100
#define BCHP_TM_TEST_CTRL_CHAN0_CLK_BYP_SEL_SHIFT                  8
#define BCHP_TM_TEST_CTRL_CHAN0_CLK_BYP_SEL_DEFAULT                0x00000000

/* TM :: TEST_CTRL :: BAND_CLK_BYP_SEL [07:07] */
#define BCHP_TM_TEST_CTRL_BAND_CLK_BYP_SEL_MASK                    0x00000080
#define BCHP_TM_TEST_CTRL_BAND_CLK_BYP_SEL_SHIFT                   7
#define BCHP_TM_TEST_CTRL_BAND_CLK_BYP_SEL_DEFAULT                 0x00000000

/* TM :: TEST_CTRL :: PIPE_CLK_BYP_SEL [06:06] */
#define BCHP_TM_TEST_CTRL_PIPE_CLK_BYP_SEL_MASK                    0x00000040
#define BCHP_TM_TEST_CTRL_PIPE_CLK_BYP_SEL_SHIFT                   6
#define BCHP_TM_TEST_CTRL_PIPE_CLK_BYP_SEL_DEFAULT                 0x00000000

/* TM :: TEST_CTRL :: XBAR_CLK_BYP_SEL [05:05] */
#define BCHP_TM_TEST_CTRL_XBAR_CLK_BYP_SEL_MASK                    0x00000020
#define BCHP_TM_TEST_CTRL_XBAR_CLK_BYP_SEL_SHIFT                   5
#define BCHP_TM_TEST_CTRL_XBAR_CLK_BYP_SEL_DEFAULT                 0x00000000

/* TM :: TEST_CTRL :: AIF_MDAC_CLK_BYP_SEL [04:04] */
#define BCHP_TM_TEST_CTRL_AIF_MDAC_CLK_BYP_SEL_MASK                0x00000010
#define BCHP_TM_TEST_CTRL_AIF_MDAC_CLK_BYP_SEL_SHIFT               4
#define BCHP_TM_TEST_CTRL_AIF_MDAC_CLK_BYP_SEL_DEFAULT             0x00000000

/* TM :: TEST_CTRL :: AIF3_CLK_BYP_SEL [03:03] */
#define BCHP_TM_TEST_CTRL_AIF3_CLK_BYP_SEL_MASK                    0x00000008
#define BCHP_TM_TEST_CTRL_AIF3_CLK_BYP_SEL_SHIFT                   3
#define BCHP_TM_TEST_CTRL_AIF3_CLK_BYP_SEL_DEFAULT                 0x00000000

/* TM :: TEST_CTRL :: AIF2_CLK_BYP_SEL [02:02] */
#define BCHP_TM_TEST_CTRL_AIF2_CLK_BYP_SEL_MASK                    0x00000004
#define BCHP_TM_TEST_CTRL_AIF2_CLK_BYP_SEL_SHIFT                   2
#define BCHP_TM_TEST_CTRL_AIF2_CLK_BYP_SEL_DEFAULT                 0x00000000

/* TM :: TEST_CTRL :: AIF1_CLK_BYP_SEL [01:01] */
#define BCHP_TM_TEST_CTRL_AIF1_CLK_BYP_SEL_MASK                    0x00000002
#define BCHP_TM_TEST_CTRL_AIF1_CLK_BYP_SEL_SHIFT                   1
#define BCHP_TM_TEST_CTRL_AIF1_CLK_BYP_SEL_DEFAULT                 0x00000000

/* TM :: TEST_CTRL :: AIF0_CLK_BYP_SEL [00:00] */
#define BCHP_TM_TEST_CTRL_AIF0_CLK_BYP_SEL_MASK                    0x00000001
#define BCHP_TM_TEST_CTRL_AIF0_CLK_BYP_SEL_SHIFT                   0
#define BCHP_TM_TEST_CTRL_AIF0_CLK_BYP_SEL_DEFAULT                 0x00000000

/***************************************************************************
 *WAKEUP_CTRL - Wakeup Control Register
 ***************************************************************************/
/* TM :: WAKEUP_CTRL :: RSVD [31:04] */
#define BCHP_TM_WAKEUP_CTRL_RSVD_MASK                              0xfffffff0
#define BCHP_TM_WAKEUP_CTRL_RSVD_SHIFT                             4
#define BCHP_TM_WAKEUP_CTRL_RSVD_DEFAULT                           0x00000000

/* TM :: WAKEUP_CTRL :: WAKEUP_EVENT_EN [03:03] */
#define BCHP_TM_WAKEUP_CTRL_WAKEUP_EVENT_EN_MASK                   0x00000008
#define BCHP_TM_WAKEUP_CTRL_WAKEUP_EVENT_EN_SHIFT                  3
#define BCHP_TM_WAKEUP_CTRL_WAKEUP_EVENT_EN_DEFAULT                0x00000000

/* TM :: WAKEUP_CTRL :: WAKEUP_INTR_EN [02:02] */
#define BCHP_TM_WAKEUP_CTRL_WAKEUP_INTR_EN_MASK                    0x00000004
#define BCHP_TM_WAKEUP_CTRL_WAKEUP_INTR_EN_SHIFT                   2
#define BCHP_TM_WAKEUP_CTRL_WAKEUP_INTR_EN_DEFAULT                 0x00000000

/* TM :: WAKEUP_CTRL :: EVENT_POL [01:01] */
#define BCHP_TM_WAKEUP_CTRL_EVENT_POL_MASK                         0x00000002
#define BCHP_TM_WAKEUP_CTRL_EVENT_POL_SHIFT                        1
#define BCHP_TM_WAKEUP_CTRL_EVENT_POL_DEFAULT                      0x00000000

/* TM :: WAKEUP_CTRL :: INTR_POL [00:00] */
#define BCHP_TM_WAKEUP_CTRL_INTR_POL_MASK                          0x00000001
#define BCHP_TM_WAKEUP_CTRL_INTR_POL_SHIFT                         0
#define BCHP_TM_WAKEUP_CTRL_INTR_POL_DEFAULT                       0x00000000

/***************************************************************************
 *RESET_CTRL - Chip Generated Reset Control Register
 ***************************************************************************/
/* TM :: RESET_CTRL :: RSVD_2 [31:11] */
#define BCHP_TM_RESET_CTRL_RSVD_2_MASK                             0xfffff800
#define BCHP_TM_RESET_CTRL_RSVD_2_SHIFT                            11
#define BCHP_TM_RESET_CTRL_RSVD_2_DEFAULT                          0x00000000

/* TM :: RESET_CTRL :: WATCHDOG_NMI_CLEAR [10:10] */
#define BCHP_TM_RESET_CTRL_WATCHDOG_NMI_CLEAR_MASK                 0x00000400
#define BCHP_TM_RESET_CTRL_WATCHDOG_NMI_CLEAR_SHIFT                10
#define BCHP_TM_RESET_CTRL_WATCHDOG_NMI_CLEAR_DEFAULT              0x00000000

/* TM :: RESET_CTRL :: WATCHDOG_RESET_CLEAR [09:09] */
#define BCHP_TM_RESET_CTRL_WATCHDOG_RESET_CLEAR_MASK               0x00000200
#define BCHP_TM_RESET_CTRL_WATCHDOG_RESET_CLEAR_SHIFT              9
#define BCHP_TM_RESET_CTRL_WATCHDOG_RESET_CLEAR_DEFAULT            0x00000000

/* TM :: RESET_CTRL :: FTM_RST_CMD_CLEAR [08:08] */
#define BCHP_TM_RESET_CTRL_FTM_RST_CMD_CLEAR_MASK                  0x00000100
#define BCHP_TM_RESET_CTRL_FTM_RST_CMD_CLEAR_SHIFT                 8
#define BCHP_TM_RESET_CTRL_FTM_RST_CMD_CLEAR_DEFAULT               0x00000000

/* TM :: RESET_CTRL :: RSVD_1 [07:06] */
#define BCHP_TM_RESET_CTRL_RSVD_1_MASK                             0x000000c0
#define BCHP_TM_RESET_CTRL_RSVD_1_SHIFT                            6
#define BCHP_TM_RESET_CTRL_RSVD_1_DEFAULT                          0x00000000

/* TM :: RESET_CTRL :: WATCHDOG_RESET_ENABLE [05:05] */
#define BCHP_TM_RESET_CTRL_WATCHDOG_RESET_ENABLE_MASK              0x00000020
#define BCHP_TM_RESET_CTRL_WATCHDOG_RESET_ENABLE_SHIFT             5
#define BCHP_TM_RESET_CTRL_WATCHDOG_RESET_ENABLE_DEFAULT           0x00000000

/* TM :: RESET_CTRL :: FTM_RST_CMD_ENABLE [04:04] */
#define BCHP_TM_RESET_CTRL_FTM_RST_CMD_ENABLE_MASK                 0x00000010
#define BCHP_TM_RESET_CTRL_FTM_RST_CMD_ENABLE_SHIFT                4
#define BCHP_TM_RESET_CTRL_FTM_RST_CMD_ENABLE_DEFAULT              0x00000000

/* TM :: RESET_CTRL :: RSVD_0 [03:03] */
#define BCHP_TM_RESET_CTRL_RSVD_0_MASK                             0x00000008
#define BCHP_TM_RESET_CTRL_RSVD_0_SHIFT                            3
#define BCHP_TM_RESET_CTRL_RSVD_0_DEFAULT                          0x00000000

/* TM :: RESET_CTRL :: WATCHDOG_NMI_STAT [02:02] */
#define BCHP_TM_RESET_CTRL_WATCHDOG_NMI_STAT_MASK                  0x00000004
#define BCHP_TM_RESET_CTRL_WATCHDOG_NMI_STAT_SHIFT                 2
#define BCHP_TM_RESET_CTRL_WATCHDOG_NMI_STAT_DEFAULT               0x00000000

/* TM :: RESET_CTRL :: WATCHDOG_RESET_STAT [01:01] */
#define BCHP_TM_RESET_CTRL_WATCHDOG_RESET_STAT_MASK                0x00000002
#define BCHP_TM_RESET_CTRL_WATCHDOG_RESET_STAT_SHIFT               1
#define BCHP_TM_RESET_CTRL_WATCHDOG_RESET_STAT_DEFAULT             0x00000000

/* TM :: RESET_CTRL :: FTM_RST_CMD_STAT [00:00] */
#define BCHP_TM_RESET_CTRL_FTM_RST_CMD_STAT_MASK                   0x00000001
#define BCHP_TM_RESET_CTRL_FTM_RST_CMD_STAT_SHIFT                  0
#define BCHP_TM_RESET_CTRL_FTM_RST_CMD_STAT_DEFAULT                0x00000000

/***************************************************************************
 *RESET_COUNT - Chip Reset Count Register
 ***************************************************************************/
/* TM :: RESET_COUNT :: RSVD [31:24] */
#define BCHP_TM_RESET_COUNT_RSVD_MASK                              0xff000000
#define BCHP_TM_RESET_COUNT_RSVD_SHIFT                             24
#define BCHP_TM_RESET_COUNT_RSVD_DEFAULT                           0x00000000

/* TM :: RESET_COUNT :: chip_rst_cnter [23:00] */
#define BCHP_TM_RESET_COUNT_chip_rst_cnter_MASK                    0x00ffffff
#define BCHP_TM_RESET_COUNT_chip_rst_cnter_SHIFT                   0
#define BCHP_TM_RESET_COUNT_chip_rst_cnter_DEFAULT                 0x0003ffff

/***************************************************************************
 *DEMOD_XPT_CTRL_0 - DEMOD_XPT Control Register 0
 ***************************************************************************/
/* TM :: DEMOD_XPT_CTRL_0 :: reserved0 [31:01] */
#define BCHP_TM_DEMOD_XPT_CTRL_0_reserved0_MASK                    0xfffffffe
#define BCHP_TM_DEMOD_XPT_CTRL_0_reserved0_SHIFT                   1

/* TM :: DEMOD_XPT_CTRL_0 :: demod_xpt_tb_disable [00:00] */
#define BCHP_TM_DEMOD_XPT_CTRL_0_demod_xpt_tb_disable_MASK         0x00000001
#define BCHP_TM_DEMOD_XPT_CTRL_0_demod_xpt_tb_disable_SHIFT        0
#define BCHP_TM_DEMOD_XPT_CTRL_0_demod_xpt_tb_disable_DEFAULT      0x00000001

/***************************************************************************
 *DEMOD_XPT_CTRL_1 - DEMOD_XPT Control Register 1
 ***************************************************************************/
/* TM :: DEMOD_XPT_CTRL_1 :: demod_xpt_chip_signature [31:00] */
#define BCHP_TM_DEMOD_XPT_CTRL_1_demod_xpt_chip_signature_MASK     0xffffffff
#define BCHP_TM_DEMOD_XPT_CTRL_1_demod_xpt_chip_signature_SHIFT    0
#define BCHP_TM_DEMOD_XPT_CTRL_1_demod_xpt_chip_signature_DEFAULT  0x00000000

/***************************************************************************
 *ICID_DATA7 - ICID Data Register
 ***************************************************************************/
/* TM :: ICID_DATA7 :: DATA [31:00] */
#define BCHP_TM_ICID_DATA7_DATA_MASK                               0xffffffff
#define BCHP_TM_ICID_DATA7_DATA_SHIFT                              0
#define BCHP_TM_ICID_DATA7_DATA_DEFAULT                            0x00000000

/***************************************************************************
 *ICID_DATA6 - ICID Data Register
 ***************************************************************************/
/* TM :: ICID_DATA6 :: DATA [31:00] */
#define BCHP_TM_ICID_DATA6_DATA_MASK                               0xffffffff
#define BCHP_TM_ICID_DATA6_DATA_SHIFT                              0
#define BCHP_TM_ICID_DATA6_DATA_DEFAULT                            0x00000000

/***************************************************************************
 *ICID_DATA5 - ICID Data Register
 ***************************************************************************/
/* TM :: ICID_DATA5 :: DATA [31:00] */
#define BCHP_TM_ICID_DATA5_DATA_MASK                               0xffffffff
#define BCHP_TM_ICID_DATA5_DATA_SHIFT                              0
#define BCHP_TM_ICID_DATA5_DATA_DEFAULT                            0x00000000

/***************************************************************************
 *ICID_DATA4 - ICID Data Register
 ***************************************************************************/
/* TM :: ICID_DATA4 :: DATA [31:00] */
#define BCHP_TM_ICID_DATA4_DATA_MASK                               0xffffffff
#define BCHP_TM_ICID_DATA4_DATA_SHIFT                              0
#define BCHP_TM_ICID_DATA4_DATA_DEFAULT                            0x00000000

/***************************************************************************
 *ICID_DATA3 - ICID Data Register
 ***************************************************************************/
/* TM :: ICID_DATA3 :: DATA [31:00] */
#define BCHP_TM_ICID_DATA3_DATA_MASK                               0xffffffff
#define BCHP_TM_ICID_DATA3_DATA_SHIFT                              0
#define BCHP_TM_ICID_DATA3_DATA_DEFAULT                            0x00000000

/***************************************************************************
 *ICID_DATA2 - ICID Data Register
 ***************************************************************************/
/* TM :: ICID_DATA2 :: DATA [31:00] */
#define BCHP_TM_ICID_DATA2_DATA_MASK                               0xffffffff
#define BCHP_TM_ICID_DATA2_DATA_SHIFT                              0
#define BCHP_TM_ICID_DATA2_DATA_DEFAULT                            0x00000000

/***************************************************************************
 *ICID_DATA1 - ICID Data Register
 ***************************************************************************/
/* TM :: ICID_DATA1 :: DATA [31:00] */
#define BCHP_TM_ICID_DATA1_DATA_MASK                               0xffffffff
#define BCHP_TM_ICID_DATA1_DATA_SHIFT                              0
#define BCHP_TM_ICID_DATA1_DATA_DEFAULT                            0x00000000

/***************************************************************************
 *ICID_DATA0 - ICID Data Register
 ***************************************************************************/
/* TM :: ICID_DATA0 :: DATA [31:00] */
#define BCHP_TM_ICID_DATA0_DATA_MASK                               0xffffffff
#define BCHP_TM_ICID_DATA0_DATA_SHIFT                              0
#define BCHP_TM_ICID_DATA0_DATA_DEFAULT                            0x00000000

/***************************************************************************
 *ICID_READ - ICID Read Register
 ***************************************************************************/
/* TM :: ICID_READ :: RSVD [31:01] */
#define BCHP_TM_ICID_READ_RSVD_MASK                                0xfffffffe
#define BCHP_TM_ICID_READ_RSVD_SHIFT                               1
#define BCHP_TM_ICID_READ_RSVD_DEFAULT                             0x00000000

/* TM :: ICID_READ :: EN [00:00] */
#define BCHP_TM_ICID_READ_EN_MASK                                  0x00000001
#define BCHP_TM_ICID_READ_EN_SHIFT                                 0
#define BCHP_TM_ICID_READ_EN_DEFAULT                               0x00000000

/***************************************************************************
 *ICID_CLK_CTRL - ICID Clock Register
 ***************************************************************************/
/* TM :: ICID_CLK_CTRL :: RSVD [31:08] */
#define BCHP_TM_ICID_CLK_CTRL_RSVD_MASK                            0xffffff00
#define BCHP_TM_ICID_CLK_CTRL_RSVD_SHIFT                           8
#define BCHP_TM_ICID_CLK_CTRL_RSVD_DEFAULT                         0x00000000

/* TM :: ICID_CLK_CTRL :: DIV [07:00] */
#define BCHP_TM_ICID_CLK_CTRL_DIV_MASK                             0x000000ff
#define BCHP_TM_ICID_CLK_CTRL_DIV_SHIFT                            0
#define BCHP_TM_ICID_CLK_CTRL_DIV_DEFAULT                          0x00000064

/***************************************************************************
 *ICID_MISC_CTRL - ICID Miscellaneous Control Register
 ***************************************************************************/
/* TM :: ICID_MISC_CTRL :: RSVD_2 [31:08] */
#define BCHP_TM_ICID_MISC_CTRL_RSVD_2_MASK                         0xffffff00
#define BCHP_TM_ICID_MISC_CTRL_RSVD_2_SHIFT                        8
#define BCHP_TM_ICID_MISC_CTRL_RSVD_2_DEFAULT                      0x00000000

/* TM :: ICID_MISC_CTRL :: RSVD_1 [07:06] */
#define BCHP_TM_ICID_MISC_CTRL_RSVD_1_MASK                         0x000000c0
#define BCHP_TM_ICID_MISC_CTRL_RSVD_1_SHIFT                        6
#define BCHP_TM_ICID_MISC_CTRL_RSVD_1_DEFAULT                      0x00000000

/* TM :: ICID_MISC_CTRL :: NUM_DUMMY_BITS_INIT [05:05] */
#define BCHP_TM_ICID_MISC_CTRL_NUM_DUMMY_BITS_INIT_MASK            0x00000020
#define BCHP_TM_ICID_MISC_CTRL_NUM_DUMMY_BITS_INIT_SHIFT           5
#define BCHP_TM_ICID_MISC_CTRL_NUM_DUMMY_BITS_INIT_DEFAULT         0x00000001

/* TM :: ICID_MISC_CTRL :: NUM_DUMMY_BITS_DATA [04:04] */
#define BCHP_TM_ICID_MISC_CTRL_NUM_DUMMY_BITS_DATA_MASK            0x00000010
#define BCHP_TM_ICID_MISC_CTRL_NUM_DUMMY_BITS_DATA_SHIFT           4
#define BCHP_TM_ICID_MISC_CTRL_NUM_DUMMY_BITS_DATA_DEFAULT         0x00000001

/* TM :: ICID_MISC_CTRL :: RSVD_0 [03:03] */
#define BCHP_TM_ICID_MISC_CTRL_RSVD_0_MASK                         0x00000008
#define BCHP_TM_ICID_MISC_CTRL_RSVD_0_SHIFT                        3
#define BCHP_TM_ICID_MISC_CTRL_RSVD_0_DEFAULT                      0x00000000

/* TM :: ICID_MISC_CTRL :: DIS_SELF_CLR_READ_EN [02:02] */
#define BCHP_TM_ICID_MISC_CTRL_DIS_SELF_CLR_READ_EN_MASK           0x00000004
#define BCHP_TM_ICID_MISC_CTRL_DIS_SELF_CLR_READ_EN_SHIFT          2
#define BCHP_TM_ICID_MISC_CTRL_DIS_SELF_CLR_READ_EN_DEFAULT        0x00000000

/* TM :: ICID_MISC_CTRL :: TP_EN [01:01] */
#define BCHP_TM_ICID_MISC_CTRL_TP_EN_MASK                          0x00000002
#define BCHP_TM_ICID_MISC_CTRL_TP_EN_SHIFT                         1
#define BCHP_TM_ICID_MISC_CTRL_TP_EN_DEFAULT                       0x00000000

/* TM :: ICID_MISC_CTRL :: SW_EN [00:00] */
#define BCHP_TM_ICID_MISC_CTRL_SW_EN_MASK                          0x00000001
#define BCHP_TM_ICID_MISC_CTRL_SW_EN_SHIFT                         0
#define BCHP_TM_ICID_MISC_CTRL_SW_EN_DEFAULT                       0x00000000

/***************************************************************************
 *SPARE_REG0 - Spare Register 0
 ***************************************************************************/
/* TM :: SPARE_REG0 :: RSVD_1 [31:04] */
#define BCHP_TM_SPARE_REG0_RSVD_1_MASK                             0xfffffff0
#define BCHP_TM_SPARE_REG0_RSVD_1_SHIFT                            4
#define BCHP_TM_SPARE_REG0_RSVD_1_DEFAULT                          0x0f0f0f0f

/* TM :: SPARE_REG0 :: RSVD [03:03] */
#define BCHP_TM_SPARE_REG0_RSVD_MASK                               0x00000008
#define BCHP_TM_SPARE_REG0_RSVD_SHIFT                              3
#define BCHP_TM_SPARE_REG0_RSVD_DEFAULT                            0x00000000

/* TM :: SPARE_REG0 :: MODEHV [02:02] */
#define BCHP_TM_SPARE_REG0_MODEHV_MASK                             0x00000004
#define BCHP_TM_SPARE_REG0_MODEHV_SHIFT                            2
#define BCHP_TM_SPARE_REG0_MODEHV_DEFAULT                          0x00000000

/* TM :: SPARE_REG0 :: ANA_2P5_COMP [01:01] */
#define BCHP_TM_SPARE_REG0_ANA_2P5_COMP_MASK                       0x00000002
#define BCHP_TM_SPARE_REG0_ANA_2P5_COMP_SHIFT                      1
#define BCHP_TM_SPARE_REG0_ANA_2P5_COMP_DEFAULT                    0x00000000

/* TM :: SPARE_REG0 :: MTSIF_GATEOFF [00:00] */
#define BCHP_TM_SPARE_REG0_MTSIF_GATEOFF_MASK                      0x00000001
#define BCHP_TM_SPARE_REG0_MTSIF_GATEOFF_SHIFT                     0
#define BCHP_TM_SPARE_REG0_MTSIF_GATEOFF_DEFAULT                   0x00000000

/***************************************************************************
 *ANA_XTAL_CONTROL - Xtal Control
 ***************************************************************************/
/* TM :: ANA_XTAL_CONTROL :: reserved0 [31:12] */
#define BCHP_TM_ANA_XTAL_CONTROL_reserved0_MASK                    0xfffff000
#define BCHP_TM_ANA_XTAL_CONTROL_reserved0_SHIFT                   12

/* TM :: ANA_XTAL_CONTROL :: osc_ldo_ctrl [11:08] */
#define BCHP_TM_ANA_XTAL_CONTROL_osc_ldo_ctrl_MASK                 0x00000f00
#define BCHP_TM_ANA_XTAL_CONTROL_osc_ldo_ctrl_SHIFT                8
#define BCHP_TM_ANA_XTAL_CONTROL_osc_ldo_ctrl_DEFAULT              0x00000005

/* TM :: ANA_XTAL_CONTROL :: osc_select_current_gisb_control [07:07] */
#define BCHP_TM_ANA_XTAL_CONTROL_osc_select_current_gisb_control_MASK 0x00000080
#define BCHP_TM_ANA_XTAL_CONTROL_osc_select_current_gisb_control_SHIFT 7
#define BCHP_TM_ANA_XTAL_CONTROL_osc_select_current_gisb_control_DEFAULT 0x00000000

/* TM :: ANA_XTAL_CONTROL :: osc_cml_sel_pd [06:03] */
#define BCHP_TM_ANA_XTAL_CONTROL_osc_cml_sel_pd_MASK               0x00000078
#define BCHP_TM_ANA_XTAL_CONTROL_osc_cml_sel_pd_SHIFT              3
#define BCHP_TM_ANA_XTAL_CONTROL_osc_cml_sel_pd_DEFAULT            0x00000000

/* TM :: ANA_XTAL_CONTROL :: osc_d2cbias_gisb_control [02:00] */
#define BCHP_TM_ANA_XTAL_CONTROL_osc_d2cbias_gisb_control_MASK     0x00000007
#define BCHP_TM_ANA_XTAL_CONTROL_osc_d2cbias_gisb_control_SHIFT    0
#define BCHP_TM_ANA_XTAL_CONTROL_osc_d2cbias_gisb_control_DEFAULT  0x00000004

/***************************************************************************
 *ANA_XTAL_EXT_CML_CONTROL - Xtal External CML control
 ***************************************************************************/
/* TM :: ANA_XTAL_EXT_CML_CONTROL :: reserved0 [31:09] */
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_reserved0_MASK            0xfffffe00
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_reserved0_SHIFT           9

/* TM :: ANA_XTAL_EXT_CML_CONTROL :: xtal_ctrl [08:06] */
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_xtal_ctrl_MASK            0x000001c0
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_xtal_ctrl_SHIFT           6
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_xtal_ctrl_DEFAULT         0x00000001

/* TM :: ANA_XTAL_EXT_CML_CONTROL :: osc_PMSM_S3_pd_buffer [05:05] */
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_osc_PMSM_S3_pd_buffer_MASK 0x00000020
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_osc_PMSM_S3_pd_buffer_SHIFT 5
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_osc_PMSM_S3_pd_buffer_DEFAULT 0x00000000

/* TM :: ANA_XTAL_EXT_CML_CONTROL :: osc_override_strap [04:04] */
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_osc_override_strap_MASK   0x00000010
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_osc_override_strap_SHIFT  4
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_osc_override_strap_DEFAULT 0x00000000

/* TM :: ANA_XTAL_EXT_CML_CONTROL :: osc_pd_buffer [03:03] */
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_osc_pd_buffer_MASK        0x00000008
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_osc_pd_buffer_SHIFT       3
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_osc_pd_buffer_DEFAULT     0x00000000

/* TM :: ANA_XTAL_EXT_CML_CONTROL :: osc_div2_sel [02:02] */
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_osc_div2_sel_MASK         0x00000004
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_osc_div2_sel_SHIFT        2
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_osc_div2_sel_DEFAULT      0x00000000

/* TM :: ANA_XTAL_EXT_CML_CONTROL :: osc_current [01:00] */
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_osc_current_MASK          0x00000003
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_osc_current_SHIFT         0
#define BCHP_TM_ANA_XTAL_EXT_CML_CONTROL_osc_current_DEFAULT       0x00000001

/***************************************************************************
 *MEMORY_POWER_CTRL_0 - MEMORY POWER Control Register 0
 ***************************************************************************/
/* TM :: MEMORY_POWER_CTRL_0 :: RSVD [31:09] */
#define BCHP_TM_MEMORY_POWER_CTRL_0_RSVD_MASK                      0xfffffe00
#define BCHP_TM_MEMORY_POWER_CTRL_0_RSVD_SHIFT                     9
#define BCHP_TM_MEMORY_POWER_CTRL_0_RSVD_DEFAULT                   0x00000000

/* TM :: MEMORY_POWER_CTRL_0 :: MEMORY_STANDBY_PERIPH [08:08] */
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_PERIPH_MASK     0x00000100
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_PERIPH_SHIFT    8
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_PERIPH_DEFAULT  0x00000000

/* TM :: MEMORY_POWER_CTRL_0 :: MEMORY_STANDBY_DEMOD_XPT [07:07] */
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_DEMOD_XPT_MASK  0x00000080
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_DEMOD_XPT_SHIFT 7
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_DEMOD_XPT_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_0 :: MEMORY_STANDBY_ODS_RCVR_TOP_1 [06:06] */
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_ODS_RCVR_TOP_1_MASK 0x00000040
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_ODS_RCVR_TOP_1_SHIFT 6
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_ODS_RCVR_TOP_1_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_0 :: MEMORY_STANDBY_T2_BICM_1 [05:05] */
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_T2_BICM_1_MASK  0x00000020
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_T2_BICM_1_SHIFT 5
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_T2_BICM_1_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_0 :: MEMORY_STANDBY_DSTOPA_1 [04:04] */
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_DSTOPA_1_MASK   0x00000010
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_DSTOPA_1_SHIFT  4
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_DSTOPA_1_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_0 :: MEMORY_STANDBY_ODS_RCVR_TOP_0 [03:03] */
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_ODS_RCVR_TOP_0_MASK 0x00000008
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_ODS_RCVR_TOP_0_SHIFT 3
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_ODS_RCVR_TOP_0_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_0 :: MEMORY_STANDBY_T2_BICM_0 [02:02] */
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_T2_BICM_0_MASK  0x00000004
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_T2_BICM_0_SHIFT 2
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_T2_BICM_0_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_0 :: MEMORY_STANDBY_DSTOPA_0 [01:01] */
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_DSTOPA_0_MASK   0x00000002
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_DSTOPA_0_SHIFT  1
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_DSTOPA_0_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_0 :: MEMORY_STANDBY_LEAP [00:00] */
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_LEAP_MASK       0x00000001
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_LEAP_SHIFT      0
#define BCHP_TM_MEMORY_POWER_CTRL_0_MEMORY_STANDBY_LEAP_DEFAULT    0x00000000

/***************************************************************************
 *MEMORY_POWER_CTRL_1 - MEMORY POWER Control Register 1
 ***************************************************************************/
/* TM :: MEMORY_POWER_CTRL_1 :: RSVD [31:09] */
#define BCHP_TM_MEMORY_POWER_CTRL_1_RSVD_MASK                      0xfffffe00
#define BCHP_TM_MEMORY_POWER_CTRL_1_RSVD_SHIFT                     9
#define BCHP_TM_MEMORY_POWER_CTRL_1_RSVD_DEFAULT                   0x00000000

/* TM :: MEMORY_POWER_CTRL_1 :: SRAM_POWER_GATE_IN_PERIPH [08:08] */
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_PERIPH_MASK 0x00000100
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_PERIPH_SHIFT 8
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_PERIPH_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_1 :: SRAM_POWER_GATE_IN_DEMOD_XPT [07:07] */
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_DEMOD_XPT_MASK 0x00000080
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_DEMOD_XPT_SHIFT 7
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_DEMOD_XPT_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_1 :: SRAM_POWER_GATE_IN_ODS_RCVR_TOP_1 [06:06] */
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_ODS_RCVR_TOP_1_MASK 0x00000040
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_ODS_RCVR_TOP_1_SHIFT 6
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_ODS_RCVR_TOP_1_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_1 :: SRAM_POWER_GATE_IN_T2_BICM_1 [05:05] */
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_T2_BICM_1_MASK 0x00000020
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_T2_BICM_1_SHIFT 5
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_T2_BICM_1_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_1 :: SRAM_POWER_GATE_IN_DSTOPA_1 [04:04] */
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_DSTOPA_1_MASK 0x00000010
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_DSTOPA_1_SHIFT 4
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_DSTOPA_1_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_1 :: SRAM_POWER_GATE_IN_ODS_RCVR_TOP_0 [03:03] */
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_ODS_RCVR_TOP_0_MASK 0x00000008
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_ODS_RCVR_TOP_0_SHIFT 3
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_ODS_RCVR_TOP_0_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_1 :: SRAM_POWER_GATE_IN_T2_BICM_0 [02:02] */
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_T2_BICM_0_MASK 0x00000004
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_T2_BICM_0_SHIFT 2
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_T2_BICM_0_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_1 :: SRAM_POWER_GATE_IN_DSTOPA_0 [01:01] */
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_DSTOPA_0_MASK 0x00000002
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_DSTOPA_0_SHIFT 1
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_DSTOPA_0_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_1 :: SRAM_POWER_GATE_IN_LEAP [00:00] */
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_LEAP_MASK   0x00000001
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_LEAP_SHIFT  0
#define BCHP_TM_MEMORY_POWER_CTRL_1_SRAM_POWER_GATE_IN_LEAP_DEFAULT 0x00000000

/***************************************************************************
 *MEMORY_POWER_CTRL_2 - MEMORY POWER Control Register 2
 ***************************************************************************/
/* TM :: MEMORY_POWER_CTRL_2 :: RSVD [31:09] */
#define BCHP_TM_MEMORY_POWER_CTRL_2_RSVD_MASK                      0xfffffe00
#define BCHP_TM_MEMORY_POWER_CTRL_2_RSVD_SHIFT                     9
#define BCHP_TM_MEMORY_POWER_CTRL_2_RSVD_DEFAULT                   0x00000000

/* TM :: MEMORY_POWER_CTRL_2 :: SRAM_POWER_GATE_OUT_PERIPH [08:08] */
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_PERIPH_MASK 0x00000100
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_PERIPH_SHIFT 8
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_PERIPH_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_2 :: SRAM_POWER_GATE_OUT_DEMOD_XPT [07:07] */
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_DEMOD_XPT_MASK 0x00000080
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_DEMOD_XPT_SHIFT 7
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_DEMOD_XPT_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_2 :: SRAM_POWER_GATE_OUT_ODS_RCVR_TOP_1 [06:06] */
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_ODS_RCVR_TOP_1_MASK 0x00000040
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_ODS_RCVR_TOP_1_SHIFT 6
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_ODS_RCVR_TOP_1_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_2 :: SRAM_POWER_GATE_OUT_T2_BICM_1 [05:05] */
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_T2_BICM_1_MASK 0x00000020
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_T2_BICM_1_SHIFT 5
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_T2_BICM_1_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_2 :: SRAM_POWER_GATE_OUT_OUT_DSTOPA_1 [04:04] */
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_OUT_DSTOPA_1_MASK 0x00000010
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_OUT_DSTOPA_1_SHIFT 4
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_OUT_DSTOPA_1_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_2 :: SRAM_POWER_GATE_OUT_ODS_RCVR_TOP_0 [03:03] */
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_ODS_RCVR_TOP_0_MASK 0x00000008
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_ODS_RCVR_TOP_0_SHIFT 3
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_ODS_RCVR_TOP_0_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_2 :: SRAM_POWER_GATE_OUT_T2_BICM_0 [02:02] */
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_T2_BICM_0_MASK 0x00000004
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_T2_BICM_0_SHIFT 2
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_T2_BICM_0_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_2 :: SRAM_POWER_GATE_OUT_DSTOPA_0 [01:01] */
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_DSTOPA_0_MASK 0x00000002
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_DSTOPA_0_SHIFT 1
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_DSTOPA_0_DEFAULT 0x00000000

/* TM :: MEMORY_POWER_CTRL_2 :: SRAM_POWER_GATE_OUT_LEAP [00:00] */
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_LEAP_MASK  0x00000001
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_LEAP_SHIFT 0
#define BCHP_TM_MEMORY_POWER_CTRL_2_SRAM_POWER_GATE_OUT_LEAP_DEFAULT 0x00000000

#endif /* #ifndef BCHP_TM_H__ */

/* End of File */
