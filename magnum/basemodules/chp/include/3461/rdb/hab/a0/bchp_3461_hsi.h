/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2005-2011 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef BCHP_HSI_H__
#define BCHP_HSI_H__

/**
 * m = memory, c = core, r = register, f = field, d = data.
 */
#if !defined(GET_FIELD) && !defined(SET_FIELD)
#define BRCM_ALIGN(c,r,f)   c##_##r##_##f##_ALIGN
#define BRCM_BITS(c,r,f)    c##_##r##_##f##_BITS
#define BRCM_MASK(c,r,f)    c##_##r##_##f##_MASK
#define BRCM_SHIFT(c,r,f)   c##_##r##_##f##_SHIFT

#define GET_FIELD(m,c,r,f) \
	((((m) & BRCM_MASK(c,r,f)) >> BRCM_SHIFT(c,r,f)) << BRCM_ALIGN(c,r,f))

#define SET_FIELD(m,c,r,f,d) \
	((m) = (((m) & ~BRCM_MASK(c,r,f)) | ((((d) >> BRCM_ALIGN(c,r,f)) << \
	 BRCM_SHIFT(c,r,f)) & BRCM_MASK(c,r,f))) \
	)

#define SET_TYPE_FIELD(m,c,r,f,d) SET_FIELD(m,c,r,f,c##_##d)
#define SET_NAME_FIELD(m,c,r,f,d) SET_FIELD(m,c,r,f,c##_##r##_##f##_##d)
#define SET_VALUE_FIELD(m,c,r,f,d) SET_FIELD(m,c,r,f,d)

#endif /* GET & SET */

/***************************************************************************
 *CSR - BBSI Control & Status Registers
 ***************************************************************************/
#define CSR_SER_PROT_REV                         0x00 /* Serial protocol revision ID */
#define CSR_CHIP_FAM0                            0x01 /* Bits [15:08] of the 16-bit chip family ID */
#define CSR_CHIP_FAM1                            0x02 /* Bits [07:00] of the 16-bit chip family ID */
#define CSR_CHIP_REV0                            0x03 /* Bits [15:08] of the 16-bit chip revision ID */
#define CSR_CHIP_REV1                            0x04 /* Bits [07:00] of the 16-bit chip revision ID */
#define CSR_STATUS                               0x05 /* Status register of the serial interface */
#define CSR_CONFIG                               0x06 /* Configuration register of the serial interface */
#define CSR_RBUS_ADDR0                           0x08 /* Bits [31:24] of the 32-bit RBUS address */
#define CSR_RBUS_ADDR1                           0x09 /* Bits [23:16] of the 32-bit RBUS address */
#define CSR_RBUS_ADDR2                           0x0a /* Bits [15:08] of the 32-bit RBUS address */
#define CSR_RBUS_ADDR3                           0x0b /* Bits [07:00] of the 32-bit RBUS address */
#define CSR_RBUS_DATA0                           0x0c /* Bits [31:24] of the 32-bit RBUS data */
#define CSR_RBUS_DATA1                           0x0d /* Bits [23:16] of the 32-bit RBUS data */
#define CSR_RBUS_DATA2                           0x0e /* Bits [15:08] of the 32-bit RBUS data */
#define CSR_RBUS_DATA3                           0x0f /* Bits [07:00] of the 32-bit RBUS data */

/***************************************************************************
 *HIF - BBSI Host Interface Registers
 ***************************************************************************/
#define HIF_SFT_RST                              0x40 /* Software Reset Control Register */
#define HIF_SFT_RST_CFG                          0x41 /* Soft Reset Configuration Control Register */
#define HIF_PWRDN                                0x42 /* Power Down Control Register */
#define HIF_MEM_CTRL                             0x43 /* Memory Power Control Register */
#define HIF_OSC_LDO_CTRL                         0x44 /* XTAL Oscillator LDO Control */
#define HIF_OSC_BIAS_CTRL                        0x45 /* XTAL Oscillator Bias Control */
#define HIF_OSC_CML_CTRL                         0x46 /* XTAL Oscillator CML Control */
#define HIF_OSC_MISC_CTRL                        0x47 /* XTAL Oscillator Bias Control */
#define HIF_OSC_STRAP_OVRD_XCORE_BIAS            0x48 /* XTAL Oscillator Stap Override Control for xcore_bias */
#define HIF_OSC_STRAP_OVRD_HIGHPASS              0x49 /* XTAL Oscillator Stap Override Control for highpass */
#define HIF_REG_PLL_RST                          0x4a /* Register PLL Reset Control */
#define HIF_REG_PLL_PDIV                         0x4b /* Register PLL PDIV Divider Control */
#define HIF_REG_PLL_NDIV_INT                     0x4c /* Register PLL NDIV_INT Divider Control */
#define HIF_REG_PLL_MDIV_CLK_108                 0x4d /* Register PLL 108 MHz Clock Divider Control */
#define HIF_REG_PLL_MDIV_CLK_054                 0x4e /* Register PLL 54 MHz Clock Divider Control */
#define HIF_REG_PLL_MDEL_CLK_108                 0x4f /* Register PLL Output Delay Control */
#define HIF_REG_PLL_MDEL_CLK_054                 0x50 /* Register PLL Output Delay Control */
#define HIF_REG_PLL_MISC_CLK_108                 0x51 /* Register PLL 108 MHz Clock Miscellaneous Control */
#define HIF_REG_PLL_MISC_CLK_054                 0x52 /* Register PLL 54 MHz Clock Miscellaneous Control */
#define HIF_REG_PLL_GAIN_KA                      0x53 /* Register PLL Ka Gain Control */
#define HIF_REG_PLL_GAIN_KI                      0x54 /* Register PLL Ki Gain Control */
#define HIF_REG_PLL_GAIN_KP                      0x55 /* Register PLL Kp Gain Control */
#define HIF_REG_PLL_DCO_BYP_EN                   0x56 /* Register PLL DCO Bypass Enable Control */
#define HIF_REG_PLL_DCO_CTRL1                    0x57 /* Register PLL DCO Control */
#define HIF_REG_PLL_DCO_CTRL0                    0x58 /* Register PLL DCO Control */
#define HIF_REG_PLL_FB_EN                        0x59 /* Register PLL Feedback Enable Control */
#define HIF_REG_PLL_FB_OFFSET1                   0x5a /* Register PLL Feedback Offset Control */
#define HIF_REG_PLL_FB_OFFSET0                   0x5b /* Register PLL Feedback Offset Control */
#define HIF_REG_PLL_SS_CTRL                      0x5c /* System PLL Spread Spectrum Control */
#define HIF_REG_PLL_SS_STEP1                     0x5d /* System PLL Spread Spectrum Step Size Control */
#define HIF_REG_PLL_SS_STEP0                     0x5e /* System PLL Spread Spectrum Step Size Control */
#define HIF_REG_PLL_SS_LIMIT3                    0x5f /* System PLL Spread Spectrum Limit Control */
#define HIF_REG_PLL_SS_LIMIT2                    0x60 /* System PLL Spread Spectrum Limit Control */
#define HIF_REG_PLL_SS_LIMIT1                    0x61 /* System PLL Spread Spectrum Limit Control */
#define HIF_REG_PLL_SS_LIMIT0                    0x62 /* System PLL Spread Spectrum Limit Control */
#define HIF_REG_PLL_MISC_CTRL1                   0x63 /* Register PLL Miscellaneous Control */
#define HIF_REG_PLL_MISC_CTRL0                   0x64 /* Register PLL Miscellaneous Control */
#define HIF_REG_PLL_STAT_CTRL                    0x65 /* Register PLL Status Control */
#define HIF_REG_PLL_STATUS2                      0x66 /* Register PLL Status */
#define HIF_REG_PLL_STATUS1                      0x67 /* Register PLL Status */
#define HIF_REG_PLL_STATUS0                      0x68 /* Register PLL Status */
#define HIF_REG_CLK_EN                           0x69 /* Register Clock Enable */
#define HIF_SYS_CLK_EN                           0x6a /* System Clock Enable */
#define HIF_MISC_CTRL                            0x6b /* Miscellaneous Control */
#define HIF_SPARE3                               0x78 /* Spare Register */
#define HIF_SPARE2                               0x79 /* Spare Register */
#define HIF_SPARE1                               0x7a /* Spare Register */
#define HIF_SPARE0                               0x7b /* Spare Register */
#define HIF_SFT3                                 0x7c /* Software Register */
#define HIF_SFT2                                 0x7d /* Software Register */
#define HIF_SFT1                                 0x7e /* Software Register */
#define HIF_SFT0                                 0x7f /* Software Register */

/***************************************************************************
 *SER_PROT_REV - Serial protocol revision ID
 ***************************************************************************/
/* CSR :: SER_PROT_REV :: SER_PROT_REV [07:00] */
#define CSR_SER_PROT_REV_SER_PROT_REV_MASK                         0xff
#define CSR_SER_PROT_REV_SER_PROT_REV_ALIGN                        0
#define CSR_SER_PROT_REV_SER_PROT_REV_BITS                         8
#define CSR_SER_PROT_REV_SER_PROT_REV_SHIFT                        0

/***************************************************************************
 *CHIP_FAM0 - Bits [15:08] of the 16-bit chip family ID
 ***************************************************************************/
/* CSR :: CHIP_FAM0 :: CHIP_FAMILY [07:00] */
#define CSR_CHIP_FAM0_CHIP_FAMILY_MASK                             0xff
#define CSR_CHIP_FAM0_CHIP_FAMILY_ALIGN                            0
#define CSR_CHIP_FAM0_CHIP_FAMILY_BITS                             8
#define CSR_CHIP_FAM0_CHIP_FAMILY_SHIFT                            0

/***************************************************************************
 *CHIP_FAM1 - Bits [07:00] of the 16-bit chip family ID
 ***************************************************************************/
/* CSR :: CHIP_FAM1 :: CHIP_FAMILY [07:00] */
#define CSR_CHIP_FAM1_CHIP_FAMILY_MASK                             0xff
#define CSR_CHIP_FAM1_CHIP_FAMILY_ALIGN                            0
#define CSR_CHIP_FAM1_CHIP_FAMILY_BITS                             8
#define CSR_CHIP_FAM1_CHIP_FAMILY_SHIFT                            0

/***************************************************************************
 *CHIP_REV0 - Bits [15:08] of the 16-bit chip revision ID
 ***************************************************************************/
/* CSR :: CHIP_REV0 :: CHIP_REV [07:00] */
#define CSR_CHIP_REV0_CHIP_REV_MASK                                0xff
#define CSR_CHIP_REV0_CHIP_REV_ALIGN                               0
#define CSR_CHIP_REV0_CHIP_REV_BITS                                8
#define CSR_CHIP_REV0_CHIP_REV_SHIFT                               0

/***************************************************************************
 *CHIP_REV1 - Bits [07:00] of the 16-bit chip revision ID
 ***************************************************************************/
/* CSR :: CHIP_REV1 :: CHIP_REV [07:00] */
#define CSR_CHIP_REV1_CHIP_REV_MASK                                0xff
#define CSR_CHIP_REV1_CHIP_REV_ALIGN                               0
#define CSR_CHIP_REV1_CHIP_REV_BITS                                8
#define CSR_CHIP_REV1_CHIP_REV_SHIFT                               0

/***************************************************************************
 *STATUS - Status register of the serial interface
 ***************************************************************************/
/* CSR :: STATUS :: reserved0 [07:07] */
#define CSR_STATUS_reserved0_MASK                                  0x80
#define CSR_STATUS_reserved0_ALIGN                                 0
#define CSR_STATUS_reserved0_BITS                                  1
#define CSR_STATUS_reserved0_SHIFT                                 7

/* CSR :: STATUS :: CPU_RUNNING [06:06] */
#define CSR_STATUS_CPU_RUNNING_MASK                                0x40
#define CSR_STATUS_CPU_RUNNING_ALIGN                               0
#define CSR_STATUS_CPU_RUNNING_BITS                                1
#define CSR_STATUS_CPU_RUNNING_SHIFT                               6

/* CSR :: STATUS :: HAB_REQ [05:05] */
#define CSR_STATUS_HAB_REQ_MASK                                    0x20
#define CSR_STATUS_HAB_REQ_ALIGN                                   0
#define CSR_STATUS_HAB_REQ_BITS                                    1
#define CSR_STATUS_HAB_REQ_SHIFT                                   5

/* CSR :: STATUS :: BUSY [04:04] */
#define CSR_STATUS_BUSY_MASK                                       0x10
#define CSR_STATUS_BUSY_ALIGN                                      0
#define CSR_STATUS_BUSY_BITS                                       1
#define CSR_STATUS_BUSY_SHIFT                                      4

/* CSR :: STATUS :: RBUS_UNEXP_TX [03:03] */
#define CSR_STATUS_RBUS_UNEXP_TX_MASK                              0x08
#define CSR_STATUS_RBUS_UNEXP_TX_ALIGN                             0
#define CSR_STATUS_RBUS_UNEXP_TX_BITS                              1
#define CSR_STATUS_RBUS_UNEXP_TX_SHIFT                             3

/* CSR :: STATUS :: RBUS_TIMEOUT [02:02] */
#define CSR_STATUS_RBUS_TIMEOUT_MASK                               0x04
#define CSR_STATUS_RBUS_TIMEOUT_ALIGN                              0
#define CSR_STATUS_RBUS_TIMEOUT_BITS                               1
#define CSR_STATUS_RBUS_TIMEOUT_SHIFT                              2

/* CSR :: STATUS :: RBUS_ERR_ACK [01:01] */
#define CSR_STATUS_RBUS_ERR_ACK_MASK                               0x02
#define CSR_STATUS_RBUS_ERR_ACK_ALIGN                              0
#define CSR_STATUS_RBUS_ERR_ACK_BITS                               1
#define CSR_STATUS_RBUS_ERR_ACK_SHIFT                              1

/* CSR :: STATUS :: ERROR [00:00] */
#define CSR_STATUS_ERROR_MASK                                      0x01
#define CSR_STATUS_ERROR_ALIGN                                     0
#define CSR_STATUS_ERROR_BITS                                      1
#define CSR_STATUS_ERROR_SHIFT                                     0

/***************************************************************************
 *CONFIG - Configuration register of the serial interface
 ***************************************************************************/
/* CSR :: CONFIG :: reserved0 [07:05] */
#define CSR_CONFIG_reserved0_MASK                                  0xe0
#define CSR_CONFIG_reserved0_ALIGN                                 0
#define CSR_CONFIG_reserved0_BITS                                  3
#define CSR_CONFIG_reserved0_SHIFT                                 5

/* CSR :: CONFIG :: TRANSFER_MODE [04:03] */
#define CSR_CONFIG_TRANSFER_MODE_MASK                              0x18
#define CSR_CONFIG_TRANSFER_MODE_8BIT                              3
#define CSR_CONFIG_TRANSFER_MODE_16BIT                             2
#define CSR_CONFIG_TRANSFER_MODE_24BIT                             1
#define CSR_CONFIG_TRANSFER_MODE_32BIT                             0
#define CSR_CONFIG_TRANSFER_MODE_ALIGN                             0
#define CSR_CONFIG_TRANSFER_MODE_BITS                              2
#define CSR_CONFIG_TRANSFER_MODE_SHIFT                             3

/* CSR :: CONFIG :: NO_RBUS_ADDR_INC [02:02] */
#define CSR_CONFIG_NO_RBUS_ADDR_INC_MASK                           0x04
#define CSR_CONFIG_NO_RBUS_ADDR_INC_ALIGN                          0
#define CSR_CONFIG_NO_RBUS_ADDR_INC_BITS                           1
#define CSR_CONFIG_NO_RBUS_ADDR_INC_SHIFT                          2

/* CSR :: CONFIG :: SPECULATIVE_READ_EN [01:01] */
#define CSR_CONFIG_SPECULATIVE_READ_EN_MASK                        0x02
#define CSR_CONFIG_SPECULATIVE_READ_EN_ALIGN                       0
#define CSR_CONFIG_SPECULATIVE_READ_EN_BITS                        1
#define CSR_CONFIG_SPECULATIVE_READ_EN_SHIFT                       1

/* CSR :: CONFIG :: READ_RBUS [00:00] */
#define CSR_CONFIG_READ_RBUS_MASK                                  0x01
#define CSR_CONFIG_READ_RBUS_READ                                  0x01
#define CSR_CONFIG_READ_RBUS_WRITE                                 0x00
#define CSR_CONFIG_READ_RBUS_ALIGN                                 0
#define CSR_CONFIG_READ_RBUS_BITS                                  1
#define CSR_CONFIG_READ_RBUS_SHIFT                                 0

/***************************************************************************
 *RBUS_ADDR0 - Bits [31:24] of the 32-bit RBUS address
 ***************************************************************************/
/* CSR :: RBUS_ADDR0 :: RBUS_ADDR0 [07:00] */
#define CSR_RBUS_ADDR0_RBUS_ADDR0_MASK                             0xff
#define CSR_RBUS_ADDR0_RBUS_ADDR0_ALIGN                            0
#define CSR_RBUS_ADDR0_RBUS_ADDR0_BITS                             8
#define CSR_RBUS_ADDR0_RBUS_ADDR0_SHIFT                            0

/***************************************************************************
 *RBUS_ADDR1 - Bits [23:16] of the 32-bit RBUS address
 ***************************************************************************/
/* CSR :: RBUS_ADDR1 :: RBUS_ADDR1 [07:00] */
#define CSR_RBUS_ADDR1_RBUS_ADDR1_MASK                             0xff
#define CSR_RBUS_ADDR1_RBUS_ADDR1_ALIGN                            0
#define CSR_RBUS_ADDR1_RBUS_ADDR1_BITS                             8
#define CSR_RBUS_ADDR1_RBUS_ADDR1_SHIFT                            0

/***************************************************************************
 *RBUS_ADDR2 - Bits [15:08] of the 32-bit RBUS address
 ***************************************************************************/
/* CSR :: RBUS_ADDR2 :: RBUS_ADDR2 [07:00] */
#define CSR_RBUS_ADDR2_RBUS_ADDR2_MASK                             0xff
#define CSR_RBUS_ADDR2_RBUS_ADDR2_ALIGN                            0
#define CSR_RBUS_ADDR2_RBUS_ADDR2_BITS                             8
#define CSR_RBUS_ADDR2_RBUS_ADDR2_SHIFT                            0

/***************************************************************************
 *RBUS_ADDR3 - Bits [07:00] of the 32-bit RBUS address
 ***************************************************************************/
/* CSR :: RBUS_ADDR3 :: RBUS_ADDR3 [07:00] */
#define CSR_RBUS_ADDR3_RBUS_ADDR3_MASK                             0xff
#define CSR_RBUS_ADDR3_RBUS_ADDR3_ALIGN                            0
#define CSR_RBUS_ADDR3_RBUS_ADDR3_BITS                             8
#define CSR_RBUS_ADDR3_RBUS_ADDR3_SHIFT                            0

/***************************************************************************
 *RBUS_DATA0 - Bits [31:24] of the 32-bit RBUS data
 ***************************************************************************/
/* CSR :: RBUS_DATA0 :: RBUS_DATA0 [07:00] */
#define CSR_RBUS_DATA0_RBUS_DATA0_MASK                             0xff
#define CSR_RBUS_DATA0_RBUS_DATA0_ALIGN                            0
#define CSR_RBUS_DATA0_RBUS_DATA0_BITS                             8
#define CSR_RBUS_DATA0_RBUS_DATA0_SHIFT                            0

/***************************************************************************
 *RBUS_DATA1 - Bits [23:16] of the 32-bit RBUS data
 ***************************************************************************/
/* CSR :: RBUS_DATA1 :: RBUS_DATA1 [07:00] */
#define CSR_RBUS_DATA1_RBUS_DATA1_MASK                             0xff
#define CSR_RBUS_DATA1_RBUS_DATA1_ALIGN                            0
#define CSR_RBUS_DATA1_RBUS_DATA1_BITS                             8
#define CSR_RBUS_DATA1_RBUS_DATA1_SHIFT                            0

/***************************************************************************
 *RBUS_DATA2 - Bits [15:08] of the 32-bit RBUS data
 ***************************************************************************/
/* CSR :: RBUS_DATA2 :: RBUS_DATA2 [07:00] */
#define CSR_RBUS_DATA2_RBUS_DATA2_MASK                             0xff
#define CSR_RBUS_DATA2_RBUS_DATA2_ALIGN                            0
#define CSR_RBUS_DATA2_RBUS_DATA2_BITS                             8
#define CSR_RBUS_DATA2_RBUS_DATA2_SHIFT                            0

/***************************************************************************
 *RBUS_DATA3 - Bits [07:00] of the 32-bit RBUS data
 ***************************************************************************/
/* CSR :: RBUS_DATA3 :: RBUS_DATA3 [07:00] */
#define CSR_RBUS_DATA3_RBUS_DATA3_MASK                             0xff
#define CSR_RBUS_DATA3_RBUS_DATA3_ALIGN                            0
#define CSR_RBUS_DATA3_RBUS_DATA3_BITS                             8
#define CSR_RBUS_DATA3_RBUS_DATA3_SHIFT                            0

/***************************************************************************
 *SFT_RST - Software Reset Control Register
 ***************************************************************************/
/* HIF :: SFT_RST :: RSVD [07:06] */
#define HIF_SFT_RST_RSVD_MASK                                      0xc0
#define HIF_SFT_RST_RSVD_ALIGN                                     0
#define HIF_SFT_RST_RSVD_BITS                                      2
#define HIF_SFT_RST_RSVD_SHIFT                                     6

/* HIF :: SFT_RST :: DS_TOPB [05:05] */
#define HIF_SFT_RST_DS_TOPB_MASK                                   0x20
#define HIF_SFT_RST_DS_TOPB_ALIGN                                  0
#define HIF_SFT_RST_DS_TOPB_BITS                                   1
#define HIF_SFT_RST_DS_TOPB_SHIFT                                  5

/* HIF :: SFT_RST :: DS_TOPA [04:04] */
#define HIF_SFT_RST_DS_TOPA_MASK                                   0x10
#define HIF_SFT_RST_DS_TOPA_ALIGN                                  0
#define HIF_SFT_RST_DS_TOPA_BITS                                   1
#define HIF_SFT_RST_DS_TOPA_SHIFT                                  4

/* HIF :: SFT_RST :: WFE [03:03] */
#define HIF_SFT_RST_WFE_MASK                                       0x08
#define HIF_SFT_RST_WFE_ALIGN                                      0
#define HIF_SFT_RST_WFE_BITS                                       1
#define HIF_SFT_RST_WFE_SHIFT                                      3

/* HIF :: SFT_RST :: CG [02:02] */
#define HIF_SFT_RST_CG_MASK                                        0x04
#define HIF_SFT_RST_CG_ALIGN                                       0
#define HIF_SFT_RST_CG_BITS                                        1
#define HIF_SFT_RST_CG_SHIFT                                       2

/* HIF :: SFT_RST :: PERIPH [01:01] */
#define HIF_SFT_RST_PERIPH_MASK                                    0x02
#define HIF_SFT_RST_PERIPH_ALIGN                                   0
#define HIF_SFT_RST_PERIPH_BITS                                    1
#define HIF_SFT_RST_PERIPH_SHIFT                                   1

/* HIF :: SFT_RST :: LEAP [00:00] */
#define HIF_SFT_RST_LEAP_MASK                                      0x01
#define HIF_SFT_RST_LEAP_ALIGN                                     0
#define HIF_SFT_RST_LEAP_BITS                                      1
#define HIF_SFT_RST_LEAP_SHIFT                                     0

/***************************************************************************
 *SFT_RST_CFG - Soft Reset Configuration Control Register
 ***************************************************************************/
/* HIF :: SFT_RST_CFG :: RSVD [07:06] */
#define HIF_SFT_RST_CFG_RSVD_MASK                                  0xc0
#define HIF_SFT_RST_CFG_RSVD_ALIGN                                 0
#define HIF_SFT_RST_CFG_RSVD_BITS                                  2
#define HIF_SFT_RST_CFG_RSVD_SHIFT                                 6

/* HIF :: SFT_RST_CFG :: DS_TOPB [05:05] */
#define HIF_SFT_RST_CFG_DS_TOPB_MASK                               0x20
#define HIF_SFT_RST_CFG_DS_TOPB_ALIGN                              0
#define HIF_SFT_RST_CFG_DS_TOPB_BITS                               1
#define HIF_SFT_RST_CFG_DS_TOPB_SHIFT                              5

/* HIF :: SFT_RST_CFG :: DS_TOPA [04:04] */
#define HIF_SFT_RST_CFG_DS_TOPA_MASK                               0x10
#define HIF_SFT_RST_CFG_DS_TOPA_ALIGN                              0
#define HIF_SFT_RST_CFG_DS_TOPA_BITS                               1
#define HIF_SFT_RST_CFG_DS_TOPA_SHIFT                              4

/* HIF :: SFT_RST_CFG :: WFE [03:03] */
#define HIF_SFT_RST_CFG_WFE_MASK                                   0x08
#define HIF_SFT_RST_CFG_WFE_ALIGN                                  0
#define HIF_SFT_RST_CFG_WFE_BITS                                   1
#define HIF_SFT_RST_CFG_WFE_SHIFT                                  3

/* HIF :: SFT_RST_CFG :: CG [02:02] */
#define HIF_SFT_RST_CFG_CG_MASK                                    0x04
#define HIF_SFT_RST_CFG_CG_ALIGN                                   0
#define HIF_SFT_RST_CFG_CG_BITS                                    1
#define HIF_SFT_RST_CFG_CG_SHIFT                                   2

/* HIF :: SFT_RST_CFG :: PERIPH [01:01] */
#define HIF_SFT_RST_CFG_PERIPH_MASK                                0x02
#define HIF_SFT_RST_CFG_PERIPH_ALIGN                               0
#define HIF_SFT_RST_CFG_PERIPH_BITS                                1
#define HIF_SFT_RST_CFG_PERIPH_SHIFT                               1

/* HIF :: SFT_RST_CFG :: LEAP [00:00] */
#define HIF_SFT_RST_CFG_LEAP_MASK                                  0x01
#define HIF_SFT_RST_CFG_LEAP_ALIGN                                 0
#define HIF_SFT_RST_CFG_LEAP_BITS                                  1
#define HIF_SFT_RST_CFG_LEAP_SHIFT                                 0

/***************************************************************************
 *PWRDN - Power Down Control Register
 ***************************************************************************/
/* HIF :: PWRDN :: PWRDN_07 [07:07] */
#define HIF_PWRDN_PWRDN_07_MASK                                    0x80
#define HIF_PWRDN_PWRDN_07_ALIGN                                   0
#define HIF_PWRDN_PWRDN_07_BITS                                    1
#define HIF_PWRDN_PWRDN_07_SHIFT                                   7

/* HIF :: PWRDN :: PWRDN_06 [06:06] */
#define HIF_PWRDN_PWRDN_06_MASK                                    0x40
#define HIF_PWRDN_PWRDN_06_ALIGN                                   0
#define HIF_PWRDN_PWRDN_06_BITS                                    1
#define HIF_PWRDN_PWRDN_06_SHIFT                                   6

/* HIF :: PWRDN :: PWRDN_05 [05:05] */
#define HIF_PWRDN_PWRDN_05_MASK                                    0x20
#define HIF_PWRDN_PWRDN_05_ALIGN                                   0
#define HIF_PWRDN_PWRDN_05_BITS                                    1
#define HIF_PWRDN_PWRDN_05_SHIFT                                   5

/* HIF :: PWRDN :: PWRDN_04 [04:04] */
#define HIF_PWRDN_PWRDN_04_MASK                                    0x10
#define HIF_PWRDN_PWRDN_04_ALIGN                                   0
#define HIF_PWRDN_PWRDN_04_BITS                                    1
#define HIF_PWRDN_PWRDN_04_SHIFT                                   4

/* HIF :: PWRDN :: PWRDN_03 [03:03] */
#define HIF_PWRDN_PWRDN_03_MASK                                    0x08
#define HIF_PWRDN_PWRDN_03_ALIGN                                   0
#define HIF_PWRDN_PWRDN_03_BITS                                    1
#define HIF_PWRDN_PWRDN_03_SHIFT                                   3

/* HIF :: PWRDN :: OSC_LDO [02:02] */
#define HIF_PWRDN_OSC_LDO_MASK                                     0x04
#define HIF_PWRDN_OSC_LDO_ALIGN                                    0
#define HIF_PWRDN_OSC_LDO_BITS                                     1
#define HIF_PWRDN_OSC_LDO_SHIFT                                    2

/* HIF :: PWRDN :: OSC_XTAL [01:01] */
#define HIF_PWRDN_OSC_XTAL_MASK                                    0x02
#define HIF_PWRDN_OSC_XTAL_ALIGN                                   0
#define HIF_PWRDN_OSC_XTAL_BITS                                    1
#define HIF_PWRDN_OSC_XTAL_SHIFT                                   1

/* HIF :: PWRDN :: REG_PLL [00:00] */
#define HIF_PWRDN_REG_PLL_MASK                                     0x01
#define HIF_PWRDN_REG_PLL_ALIGN                                    0
#define HIF_PWRDN_REG_PLL_BITS                                     1
#define HIF_PWRDN_REG_PLL_SHIFT                                    0

/***************************************************************************
 *MEM_CTRL - Memory Power Control Register
 ***************************************************************************/
/* HIF :: MEM_CTRL :: RSVD [07:04] */
#define HIF_MEM_CTRL_RSVD_MASK                                     0xf0
#define HIF_MEM_CTRL_RSVD_ALIGN                                    0
#define HIF_MEM_CTRL_RSVD_BITS                                     4
#define HIF_MEM_CTRL_RSVD_SHIFT                                    4

/* HIF :: MEM_CTRL :: ROM_STBY_LEAP [03:03] */
#define HIF_MEM_CTRL_ROM_STBY_LEAP_MASK                            0x08
#define HIF_MEM_CTRL_ROM_STBY_LEAP_ALIGN                           0
#define HIF_MEM_CTRL_ROM_STBY_LEAP_BITS                            1
#define HIF_MEM_CTRL_ROM_STBY_LEAP_SHIFT                           3

/* HIF :: MEM_CTRL :: RAM_STBY_LEAP [02:02] */
#define HIF_MEM_CTRL_RAM_STBY_LEAP_MASK                            0x04
#define HIF_MEM_CTRL_RAM_STBY_LEAP_ALIGN                           0
#define HIF_MEM_CTRL_RAM_STBY_LEAP_BITS                            1
#define HIF_MEM_CTRL_RAM_STBY_LEAP_SHIFT                           2

/* HIF :: MEM_CTRL :: PSM_LEAP [01:00] */
#define HIF_MEM_CTRL_PSM_LEAP_MASK                                 0x03
#define HIF_MEM_CTRL_PSM_LEAP_ALIGN                                0
#define HIF_MEM_CTRL_PSM_LEAP_BITS                                 2
#define HIF_MEM_CTRL_PSM_LEAP_SHIFT                                0

/***************************************************************************
 *OSC_LDO_CTRL - XTAL Oscillator LDO Control
 ***************************************************************************/
/* HIF :: OSC_LDO_CTRL :: RSVD [07:02] */
#define HIF_OSC_LDO_CTRL_RSVD_MASK                                 0xfc
#define HIF_OSC_LDO_CTRL_RSVD_ALIGN                                0
#define HIF_OSC_LDO_CTRL_RSVD_BITS                                 6
#define HIF_OSC_LDO_CTRL_RSVD_SHIFT                                2

/* HIF :: OSC_LDO_CTRL :: OUTPUT [01:00] */
#define HIF_OSC_LDO_CTRL_OUTPUT_MASK                               0x03
#define HIF_OSC_LDO_CTRL_OUTPUT_ALIGN                              0
#define HIF_OSC_LDO_CTRL_OUTPUT_BITS                               2
#define HIF_OSC_LDO_CTRL_OUTPUT_SHIFT                              0

/***************************************************************************
 *OSC_BIAS_CTRL - XTAL Oscillator Bias Control
 ***************************************************************************/
/* HIF :: OSC_BIAS_CTRL :: RSVD_1 [07:07] */
#define HIF_OSC_BIAS_CTRL_RSVD_1_MASK                              0x80
#define HIF_OSC_BIAS_CTRL_RSVD_1_ALIGN                             0
#define HIF_OSC_BIAS_CTRL_RSVD_1_BITS                              1
#define HIF_OSC_BIAS_CTRL_RSVD_1_SHIFT                             7

/* HIF :: OSC_BIAS_CTRL :: D2C [06:04] */
#define HIF_OSC_BIAS_CTRL_D2C_MASK                                 0x70
#define HIF_OSC_BIAS_CTRL_D2C_ALIGN                                0
#define HIF_OSC_BIAS_CTRL_D2C_BITS                                 3
#define HIF_OSC_BIAS_CTRL_D2C_SHIFT                                4

/* HIF :: OSC_BIAS_CTRL :: RSVD_0 [03:03] */
#define HIF_OSC_BIAS_CTRL_RSVD_0_MASK                              0x08
#define HIF_OSC_BIAS_CTRL_RSVD_0_ALIGN                             0
#define HIF_OSC_BIAS_CTRL_RSVD_0_BITS                              1
#define HIF_OSC_BIAS_CTRL_RSVD_0_SHIFT                             3

/* HIF :: OSC_BIAS_CTRL :: MASTER [02:00] */
#define HIF_OSC_BIAS_CTRL_MASTER_MASK                              0x07
#define HIF_OSC_BIAS_CTRL_MASTER_ALIGN                             0
#define HIF_OSC_BIAS_CTRL_MASTER_BITS                              3
#define HIF_OSC_BIAS_CTRL_MASTER_SHIFT                             0

/***************************************************************************
 *OSC_CML_CTRL - XTAL Oscillator CML Control
 ***************************************************************************/
/* HIF :: OSC_CML_CTRL :: RSVD_1 [07:05] */
#define HIF_OSC_CML_CTRL_RSVD_1_MASK                               0xe0
#define HIF_OSC_CML_CTRL_RSVD_1_ALIGN                              0
#define HIF_OSC_CML_CTRL_RSVD_1_BITS                               3
#define HIF_OSC_CML_CTRL_RSVD_1_SHIFT                              5

/* HIF :: OSC_CML_CTRL :: CUR [04:04] */
#define HIF_OSC_CML_CTRL_CUR_MASK                                  0x10
#define HIF_OSC_CML_CTRL_CUR_ALIGN                                 0
#define HIF_OSC_CML_CTRL_CUR_BITS                                  1
#define HIF_OSC_CML_CTRL_CUR_SHIFT                                 4

/* HIF :: OSC_CML_CTRL :: RSVD_0 [03:02] */
#define HIF_OSC_CML_CTRL_RSVD_0_MASK                               0x0c
#define HIF_OSC_CML_CTRL_RSVD_0_ALIGN                              0
#define HIF_OSC_CML_CTRL_RSVD_0_BITS                               2
#define HIF_OSC_CML_CTRL_RSVD_0_SHIFT                              2

/* HIF :: OSC_CML_CTRL :: DRIVE [01:00] */
#define HIF_OSC_CML_CTRL_DRIVE_MASK                                0x03
#define HIF_OSC_CML_CTRL_DRIVE_ALIGN                               0
#define HIF_OSC_CML_CTRL_DRIVE_BITS                                2
#define HIF_OSC_CML_CTRL_DRIVE_SHIFT                               0

/***************************************************************************
 *OSC_MISC_CTRL - XTAL Oscillator Bias Control
 ***************************************************************************/
/* HIF :: OSC_MISC_CTRL :: RSVD_0 [07:01] */
#define HIF_OSC_MISC_CTRL_RSVD_0_MASK                              0xfe
#define HIF_OSC_MISC_CTRL_RSVD_0_ALIGN                             0
#define HIF_OSC_MISC_CTRL_RSVD_0_BITS                              7
#define HIF_OSC_MISC_CTRL_RSVD_0_SHIFT                             1

/* HIF :: OSC_MISC_CTRL :: LPG [00:00] */
#define HIF_OSC_MISC_CTRL_LPG_MASK                                 0x01
#define HIF_OSC_MISC_CTRL_LPG_ALIGN                                0
#define HIF_OSC_MISC_CTRL_LPG_BITS                                 1
#define HIF_OSC_MISC_CTRL_LPG_SHIFT                                0

/***************************************************************************
 *OSC_STRAP_OVRD_XCORE_BIAS - XTAL Oscillator Stap Override Control for xcore_bias
 ***************************************************************************/
/* HIF :: OSC_STRAP_OVRD_XCORE_BIAS :: RSVD_0 [07:05] */
#define HIF_OSC_STRAP_OVRD_XCORE_BIAS_RSVD_0_MASK                  0xe0
#define HIF_OSC_STRAP_OVRD_XCORE_BIAS_RSVD_0_ALIGN                 0
#define HIF_OSC_STRAP_OVRD_XCORE_BIAS_RSVD_0_BITS                  3
#define HIF_OSC_STRAP_OVRD_XCORE_BIAS_RSVD_0_SHIFT                 5

/* HIF :: OSC_STRAP_OVRD_XCORE_BIAS :: EN [04:04] */
#define HIF_OSC_STRAP_OVRD_XCORE_BIAS_EN_MASK                      0x10
#define HIF_OSC_STRAP_OVRD_XCORE_BIAS_EN_ALIGN                     0
#define HIF_OSC_STRAP_OVRD_XCORE_BIAS_EN_BITS                      1
#define HIF_OSC_STRAP_OVRD_XCORE_BIAS_EN_SHIFT                     4

/* HIF :: OSC_STRAP_OVRD_XCORE_BIAS :: VAL [03:00] */
#define HIF_OSC_STRAP_OVRD_XCORE_BIAS_VAL_MASK                     0x0f
#define HIF_OSC_STRAP_OVRD_XCORE_BIAS_VAL_ALIGN                    0
#define HIF_OSC_STRAP_OVRD_XCORE_BIAS_VAL_BITS                     4
#define HIF_OSC_STRAP_OVRD_XCORE_BIAS_VAL_SHIFT                    0

/***************************************************************************
 *OSC_STRAP_OVRD_HIGHPASS - XTAL Oscillator Stap Override Control for highpass
 ***************************************************************************/
/* HIF :: OSC_STRAP_OVRD_HIGHPASS :: RSVD_1 [07:05] */
#define HIF_OSC_STRAP_OVRD_HIGHPASS_RSVD_1_MASK                    0xe0
#define HIF_OSC_STRAP_OVRD_HIGHPASS_RSVD_1_ALIGN                   0
#define HIF_OSC_STRAP_OVRD_HIGHPASS_RSVD_1_BITS                    3
#define HIF_OSC_STRAP_OVRD_HIGHPASS_RSVD_1_SHIFT                   5

/* HIF :: OSC_STRAP_OVRD_HIGHPASS :: EN [04:04] */
#define HIF_OSC_STRAP_OVRD_HIGHPASS_EN_MASK                        0x10
#define HIF_OSC_STRAP_OVRD_HIGHPASS_EN_ALIGN                       0
#define HIF_OSC_STRAP_OVRD_HIGHPASS_EN_BITS                        1
#define HIF_OSC_STRAP_OVRD_HIGHPASS_EN_SHIFT                       4

/* HIF :: OSC_STRAP_OVRD_HIGHPASS :: RSVD_0 [03:01] */
#define HIF_OSC_STRAP_OVRD_HIGHPASS_RSVD_0_MASK                    0x0e
#define HIF_OSC_STRAP_OVRD_HIGHPASS_RSVD_0_ALIGN                   0
#define HIF_OSC_STRAP_OVRD_HIGHPASS_RSVD_0_BITS                    3
#define HIF_OSC_STRAP_OVRD_HIGHPASS_RSVD_0_SHIFT                   1

/* HIF :: OSC_STRAP_OVRD_HIGHPASS :: VAL [00:00] */
#define HIF_OSC_STRAP_OVRD_HIGHPASS_VAL_MASK                       0x01
#define HIF_OSC_STRAP_OVRD_HIGHPASS_VAL_ALIGN                      0
#define HIF_OSC_STRAP_OVRD_HIGHPASS_VAL_BITS                       1
#define HIF_OSC_STRAP_OVRD_HIGHPASS_VAL_SHIFT                      0

/***************************************************************************
 *REG_PLL_RST - Register PLL Reset Control
 ***************************************************************************/
/* HIF :: REG_PLL_RST :: RSVD_0 [07:02] */
#define HIF_REG_PLL_RST_RSVD_0_MASK                                0xfc
#define HIF_REG_PLL_RST_RSVD_0_ALIGN                               0
#define HIF_REG_PLL_RST_RSVD_0_BITS                                6
#define HIF_REG_PLL_RST_RSVD_0_SHIFT                               2

/* HIF :: REG_PLL_RST :: POST [01:01] */
#define HIF_REG_PLL_RST_POST_MASK                                  0x02
#define HIF_REG_PLL_RST_POST_ALIGN                                 0
#define HIF_REG_PLL_RST_POST_BITS                                  1
#define HIF_REG_PLL_RST_POST_SHIFT                                 1

/* HIF :: REG_PLL_RST :: GLOBAL [00:00] */
#define HIF_REG_PLL_RST_GLOBAL_MASK                                0x01
#define HIF_REG_PLL_RST_GLOBAL_ALIGN                               0
#define HIF_REG_PLL_RST_GLOBAL_BITS                                1
#define HIF_REG_PLL_RST_GLOBAL_SHIFT                               0

/***************************************************************************
 *REG_PLL_PDIV - Register PLL PDIV Divider Control
 ***************************************************************************/
/* HIF :: REG_PLL_PDIV :: RSVD [07:04] */
#define HIF_REG_PLL_PDIV_RSVD_MASK                                 0xf0
#define HIF_REG_PLL_PDIV_RSVD_ALIGN                                0
#define HIF_REG_PLL_PDIV_RSVD_BITS                                 4
#define HIF_REG_PLL_PDIV_RSVD_SHIFT                                4

/* HIF :: REG_PLL_PDIV :: DIV [03:00] */
#define HIF_REG_PLL_PDIV_DIV_MASK                                  0x0f
#define HIF_REG_PLL_PDIV_DIV_ALIGN                                 0
#define HIF_REG_PLL_PDIV_DIV_BITS                                  4
#define HIF_REG_PLL_PDIV_DIV_SHIFT                                 0

/***************************************************************************
 *REG_PLL_NDIV_INT - Register PLL NDIV_INT Divider Control
 ***************************************************************************/
/* HIF :: REG_PLL_NDIV_INT :: DIV [07:00] */
#define HIF_REG_PLL_NDIV_INT_DIV_MASK                              0xff
#define HIF_REG_PLL_NDIV_INT_DIV_ALIGN                             0
#define HIF_REG_PLL_NDIV_INT_DIV_BITS                              8
#define HIF_REG_PLL_NDIV_INT_DIV_SHIFT                             0

/***************************************************************************
 *REG_PLL_MDIV_CLK_108 - Register PLL 108 MHz Clock Divider Control
 ***************************************************************************/
/* HIF :: REG_PLL_MDIV_CLK_108 :: DIV [07:00] */
#define HIF_REG_PLL_MDIV_CLK_108_DIV_MASK                          0xff
#define HIF_REG_PLL_MDIV_CLK_108_DIV_ALIGN                         0
#define HIF_REG_PLL_MDIV_CLK_108_DIV_BITS                          8
#define HIF_REG_PLL_MDIV_CLK_108_DIV_SHIFT                         0

/***************************************************************************
 *REG_PLL_MDIV_CLK_054 - Register PLL 54 MHz Clock Divider Control
 ***************************************************************************/
/* HIF :: REG_PLL_MDIV_CLK_054 :: DIV [07:00] */
#define HIF_REG_PLL_MDIV_CLK_054_DIV_MASK                          0xff
#define HIF_REG_PLL_MDIV_CLK_054_DIV_ALIGN                         0
#define HIF_REG_PLL_MDIV_CLK_054_DIV_BITS                          8
#define HIF_REG_PLL_MDIV_CLK_054_DIV_SHIFT                         0

/***************************************************************************
 *REG_PLL_MDEL_CLK_108 - Register PLL Output Delay Control
 ***************************************************************************/
/* HIF :: REG_PLL_MDEL_CLK_108 :: RSVD [07:03] */
#define HIF_REG_PLL_MDEL_CLK_108_RSVD_MASK                         0xf8
#define HIF_REG_PLL_MDEL_CLK_108_RSVD_ALIGN                        0
#define HIF_REG_PLL_MDEL_CLK_108_RSVD_BITS                         5
#define HIF_REG_PLL_MDEL_CLK_108_RSVD_SHIFT                        3

/* HIF :: REG_PLL_MDEL_CLK_108 :: DLY [02:00] */
#define HIF_REG_PLL_MDEL_CLK_108_DLY_MASK                          0x07
#define HIF_REG_PLL_MDEL_CLK_108_DLY_ALIGN                         0
#define HIF_REG_PLL_MDEL_CLK_108_DLY_BITS                          3
#define HIF_REG_PLL_MDEL_CLK_108_DLY_SHIFT                         0

/***************************************************************************
 *REG_PLL_MDEL_CLK_054 - Register PLL Output Delay Control
 ***************************************************************************/
/* HIF :: REG_PLL_MDEL_CLK_054 :: RSVD [07:03] */
#define HIF_REG_PLL_MDEL_CLK_054_RSVD_MASK                         0xf8
#define HIF_REG_PLL_MDEL_CLK_054_RSVD_ALIGN                        0
#define HIF_REG_PLL_MDEL_CLK_054_RSVD_BITS                         5
#define HIF_REG_PLL_MDEL_CLK_054_RSVD_SHIFT                        3

/* HIF :: REG_PLL_MDEL_CLK_054 :: DLY [02:00] */
#define HIF_REG_PLL_MDEL_CLK_054_DLY_MASK                          0x07
#define HIF_REG_PLL_MDEL_CLK_054_DLY_ALIGN                         0
#define HIF_REG_PLL_MDEL_CLK_054_DLY_BITS                          3
#define HIF_REG_PLL_MDEL_CLK_054_DLY_SHIFT                         0

/***************************************************************************
 *REG_PLL_MISC_CLK_108 - Register PLL 108 MHz Clock Miscellaneous Control
 ***************************************************************************/
/* HIF :: REG_PLL_MISC_CLK_108 :: RSVD [07:03] */
#define HIF_REG_PLL_MISC_CLK_108_RSVD_MASK                         0xf8
#define HIF_REG_PLL_MISC_CLK_108_RSVD_ALIGN                        0
#define HIF_REG_PLL_MISC_CLK_108_RSVD_BITS                         5
#define HIF_REG_PLL_MISC_CLK_108_RSVD_SHIFT                        3

/* HIF :: REG_PLL_MISC_CLK_108 :: LOAD_DIS [02:02] */
#define HIF_REG_PLL_MISC_CLK_108_LOAD_DIS_MASK                     0x04
#define HIF_REG_PLL_MISC_CLK_108_LOAD_DIS_ALIGN                    0
#define HIF_REG_PLL_MISC_CLK_108_LOAD_DIS_BITS                     1
#define HIF_REG_PLL_MISC_CLK_108_LOAD_DIS_SHIFT                    2

/* HIF :: REG_PLL_MISC_CLK_108 :: PWRUP [01:01] */
#define HIF_REG_PLL_MISC_CLK_108_PWRUP_MASK                        0x02
#define HIF_REG_PLL_MISC_CLK_108_PWRUP_ALIGN                       0
#define HIF_REG_PLL_MISC_CLK_108_PWRUP_BITS                        1
#define HIF_REG_PLL_MISC_CLK_108_PWRUP_SHIFT                       1

/* HIF :: REG_PLL_MISC_CLK_108 :: EN [00:00] */
#define HIF_REG_PLL_MISC_CLK_108_EN_MASK                           0x01
#define HIF_REG_PLL_MISC_CLK_108_EN_ALIGN                          0
#define HIF_REG_PLL_MISC_CLK_108_EN_BITS                           1
#define HIF_REG_PLL_MISC_CLK_108_EN_SHIFT                          0

/***************************************************************************
 *REG_PLL_MISC_CLK_054 - Register PLL 54 MHz Clock Miscellaneous Control
 ***************************************************************************/
/* HIF :: REG_PLL_MISC_CLK_054 :: RSVD [07:03] */
#define HIF_REG_PLL_MISC_CLK_054_RSVD_MASK                         0xf8
#define HIF_REG_PLL_MISC_CLK_054_RSVD_ALIGN                        0
#define HIF_REG_PLL_MISC_CLK_054_RSVD_BITS                         5
#define HIF_REG_PLL_MISC_CLK_054_RSVD_SHIFT                        3

/* HIF :: REG_PLL_MISC_CLK_054 :: LOAD_DIS [02:02] */
#define HIF_REG_PLL_MISC_CLK_054_LOAD_DIS_MASK                     0x04
#define HIF_REG_PLL_MISC_CLK_054_LOAD_DIS_ALIGN                    0
#define HIF_REG_PLL_MISC_CLK_054_LOAD_DIS_BITS                     1
#define HIF_REG_PLL_MISC_CLK_054_LOAD_DIS_SHIFT                    2

/* HIF :: REG_PLL_MISC_CLK_054 :: PWRUP [01:01] */
#define HIF_REG_PLL_MISC_CLK_054_PWRUP_MASK                        0x02
#define HIF_REG_PLL_MISC_CLK_054_PWRUP_ALIGN                       0
#define HIF_REG_PLL_MISC_CLK_054_PWRUP_BITS                        1
#define HIF_REG_PLL_MISC_CLK_054_PWRUP_SHIFT                       1

/* HIF :: REG_PLL_MISC_CLK_054 :: EN [00:00] */
#define HIF_REG_PLL_MISC_CLK_054_EN_MASK                           0x01
#define HIF_REG_PLL_MISC_CLK_054_EN_ALIGN                          0
#define HIF_REG_PLL_MISC_CLK_054_EN_BITS                           1
#define HIF_REG_PLL_MISC_CLK_054_EN_SHIFT                          0

/***************************************************************************
 *REG_PLL_GAIN_KA - Register PLL Ka Gain Control
 ***************************************************************************/
/* HIF :: REG_PLL_GAIN_KA :: RSVD [07:03] */
#define HIF_REG_PLL_GAIN_KA_RSVD_MASK                              0xf8
#define HIF_REG_PLL_GAIN_KA_RSVD_ALIGN                             0
#define HIF_REG_PLL_GAIN_KA_RSVD_BITS                              5
#define HIF_REG_PLL_GAIN_KA_RSVD_SHIFT                             3

/* HIF :: REG_PLL_GAIN_KA :: KA [02:00] */
#define HIF_REG_PLL_GAIN_KA_KA_MASK                                0x07
#define HIF_REG_PLL_GAIN_KA_KA_ALIGN                               0
#define HIF_REG_PLL_GAIN_KA_KA_BITS                                3
#define HIF_REG_PLL_GAIN_KA_KA_SHIFT                               0

/***************************************************************************
 *REG_PLL_GAIN_KI - Register PLL Ki Gain Control
 ***************************************************************************/
/* HIF :: REG_PLL_GAIN_KI :: RSVD [07:03] */
#define HIF_REG_PLL_GAIN_KI_RSVD_MASK                              0xf8
#define HIF_REG_PLL_GAIN_KI_RSVD_ALIGN                             0
#define HIF_REG_PLL_GAIN_KI_RSVD_BITS                              5
#define HIF_REG_PLL_GAIN_KI_RSVD_SHIFT                             3

/* HIF :: REG_PLL_GAIN_KI :: KI [02:00] */
#define HIF_REG_PLL_GAIN_KI_KI_MASK                                0x07
#define HIF_REG_PLL_GAIN_KI_KI_ALIGN                               0
#define HIF_REG_PLL_GAIN_KI_KI_BITS                                3
#define HIF_REG_PLL_GAIN_KI_KI_SHIFT                               0

/***************************************************************************
 *REG_PLL_GAIN_KP - Register PLL Kp Gain Control
 ***************************************************************************/
/* HIF :: REG_PLL_GAIN_KP :: RSVD [07:04] */
#define HIF_REG_PLL_GAIN_KP_RSVD_MASK                              0xf0
#define HIF_REG_PLL_GAIN_KP_RSVD_ALIGN                             0
#define HIF_REG_PLL_GAIN_KP_RSVD_BITS                              4
#define HIF_REG_PLL_GAIN_KP_RSVD_SHIFT                             4

/* HIF :: REG_PLL_GAIN_KP :: KP [03:00] */
#define HIF_REG_PLL_GAIN_KP_KP_MASK                                0x0f
#define HIF_REG_PLL_GAIN_KP_KP_ALIGN                               0
#define HIF_REG_PLL_GAIN_KP_KP_BITS                                4
#define HIF_REG_PLL_GAIN_KP_KP_SHIFT                               0

/***************************************************************************
 *REG_PLL_DCO_BYP_EN - Register PLL DCO Bypass Enable Control
 ***************************************************************************/
/* HIF :: REG_PLL_DCO_BYP_EN :: RSVD [07:01] */
#define HIF_REG_PLL_DCO_BYP_EN_RSVD_MASK                           0xfe
#define HIF_REG_PLL_DCO_BYP_EN_RSVD_ALIGN                          0
#define HIF_REG_PLL_DCO_BYP_EN_RSVD_BITS                           7
#define HIF_REG_PLL_DCO_BYP_EN_RSVD_SHIFT                          1

/* HIF :: REG_PLL_DCO_BYP_EN :: EN [00:00] */
#define HIF_REG_PLL_DCO_BYP_EN_EN_MASK                             0x01
#define HIF_REG_PLL_DCO_BYP_EN_EN_ALIGN                            0
#define HIF_REG_PLL_DCO_BYP_EN_EN_BITS                             1
#define HIF_REG_PLL_DCO_BYP_EN_EN_SHIFT                            0

/***************************************************************************
 *REG_PLL_DCO_CTRL1 - Register PLL DCO Control
 ***************************************************************************/
/* HIF :: REG_PLL_DCO_CTRL1 :: RSVD [07:04] */
#define HIF_REG_PLL_DCO_CTRL1_RSVD_MASK                            0xf0
#define HIF_REG_PLL_DCO_CTRL1_RSVD_ALIGN                           0
#define HIF_REG_PLL_DCO_CTRL1_RSVD_BITS                            4
#define HIF_REG_PLL_DCO_CTRL1_RSVD_SHIFT                           4

/* HIF :: REG_PLL_DCO_CTRL1 :: VAL [03:00] */
#define HIF_REG_PLL_DCO_CTRL1_VAL_MASK                             0x0f
#define HIF_REG_PLL_DCO_CTRL1_VAL_ALIGN                            0
#define HIF_REG_PLL_DCO_CTRL1_VAL_BITS                             4
#define HIF_REG_PLL_DCO_CTRL1_VAL_SHIFT                            0

/***************************************************************************
 *REG_PLL_DCO_CTRL0 - Register PLL DCO Control
 ***************************************************************************/
/* HIF :: REG_PLL_DCO_CTRL0 :: VAL [07:00] */
#define HIF_REG_PLL_DCO_CTRL0_VAL_MASK                             0xff
#define HIF_REG_PLL_DCO_CTRL0_VAL_ALIGN                            0
#define HIF_REG_PLL_DCO_CTRL0_VAL_BITS                             8
#define HIF_REG_PLL_DCO_CTRL0_VAL_SHIFT                            0

/***************************************************************************
 *REG_PLL_FB_EN - Register PLL Feedback Enable Control
 ***************************************************************************/
/* HIF :: REG_PLL_FB_EN :: RSVD [07:01] */
#define HIF_REG_PLL_FB_EN_RSVD_MASK                                0xfe
#define HIF_REG_PLL_FB_EN_RSVD_ALIGN                               0
#define HIF_REG_PLL_FB_EN_RSVD_BITS                                7
#define HIF_REG_PLL_FB_EN_RSVD_SHIFT                               1

/* HIF :: REG_PLL_FB_EN :: EN [00:00] */
#define HIF_REG_PLL_FB_EN_EN_MASK                                  0x01
#define HIF_REG_PLL_FB_EN_EN_ALIGN                                 0
#define HIF_REG_PLL_FB_EN_EN_BITS                                  1
#define HIF_REG_PLL_FB_EN_EN_SHIFT                                 0

/***************************************************************************
 *REG_PLL_FB_OFFSET1 - Register PLL Feedback Offset Control
 ***************************************************************************/
/* HIF :: REG_PLL_FB_OFFSET1 :: RSVD [07:04] */
#define HIF_REG_PLL_FB_OFFSET1_RSVD_MASK                           0xf0
#define HIF_REG_PLL_FB_OFFSET1_RSVD_ALIGN                          0
#define HIF_REG_PLL_FB_OFFSET1_RSVD_BITS                           4
#define HIF_REG_PLL_FB_OFFSET1_RSVD_SHIFT                          4

/* HIF :: REG_PLL_FB_OFFSET1 :: OFFSET [03:00] */
#define HIF_REG_PLL_FB_OFFSET1_OFFSET_MASK                         0x0f
#define HIF_REG_PLL_FB_OFFSET1_OFFSET_ALIGN                        0
#define HIF_REG_PLL_FB_OFFSET1_OFFSET_BITS                         4
#define HIF_REG_PLL_FB_OFFSET1_OFFSET_SHIFT                        0

/***************************************************************************
 *REG_PLL_FB_OFFSET0 - Register PLL Feedback Offset Control
 ***************************************************************************/
/* HIF :: REG_PLL_FB_OFFSET0 :: OFFSET [07:00] */
#define HIF_REG_PLL_FB_OFFSET0_OFFSET_MASK                         0xff
#define HIF_REG_PLL_FB_OFFSET0_OFFSET_ALIGN                        0
#define HIF_REG_PLL_FB_OFFSET0_OFFSET_BITS                         8
#define HIF_REG_PLL_FB_OFFSET0_OFFSET_SHIFT                        0

/***************************************************************************
 *REG_PLL_SS_CTRL - System PLL Spread Spectrum Control
 ***************************************************************************/
/* HIF :: REG_PLL_SS_CTRL :: RSVD [07:01] */
#define HIF_REG_PLL_SS_CTRL_RSVD_MASK                              0xfe
#define HIF_REG_PLL_SS_CTRL_RSVD_ALIGN                             0
#define HIF_REG_PLL_SS_CTRL_RSVD_BITS                              7
#define HIF_REG_PLL_SS_CTRL_RSVD_SHIFT                             1

/* HIF :: REG_PLL_SS_CTRL :: EN [00:00] */
#define HIF_REG_PLL_SS_CTRL_EN_MASK                                0x01
#define HIF_REG_PLL_SS_CTRL_EN_ALIGN                               0
#define HIF_REG_PLL_SS_CTRL_EN_BITS                                1
#define HIF_REG_PLL_SS_CTRL_EN_SHIFT                               0

/***************************************************************************
 *REG_PLL_SS_STEP1 - System PLL Spread Spectrum Step Size Control
 ***************************************************************************/
/* HIF :: REG_PLL_SS_STEP1 :: SIZE [07:00] */
#define HIF_REG_PLL_SS_STEP1_SIZE_MASK                             0xff
#define HIF_REG_PLL_SS_STEP1_SIZE_ALIGN                            0
#define HIF_REG_PLL_SS_STEP1_SIZE_BITS                             8
#define HIF_REG_PLL_SS_STEP1_SIZE_SHIFT                            0

/***************************************************************************
 *REG_PLL_SS_STEP0 - System PLL Spread Spectrum Step Size Control
 ***************************************************************************/
/* HIF :: REG_PLL_SS_STEP0 :: SIZE [07:00] */
#define HIF_REG_PLL_SS_STEP0_SIZE_MASK                             0xff
#define HIF_REG_PLL_SS_STEP0_SIZE_ALIGN                            0
#define HIF_REG_PLL_SS_STEP0_SIZE_BITS                             8
#define HIF_REG_PLL_SS_STEP0_SIZE_SHIFT                            0

/***************************************************************************
 *REG_PLL_SS_LIMIT3 - System PLL Spread Spectrum Limit Control
 ***************************************************************************/
/* HIF :: REG_PLL_SS_LIMIT3 :: RSVD [07:02] */
#define HIF_REG_PLL_SS_LIMIT3_RSVD_MASK                            0xfc
#define HIF_REG_PLL_SS_LIMIT3_RSVD_ALIGN                           0
#define HIF_REG_PLL_SS_LIMIT3_RSVD_BITS                            6
#define HIF_REG_PLL_SS_LIMIT3_RSVD_SHIFT                           2

/* HIF :: REG_PLL_SS_LIMIT3 :: LIMIT [01:00] */
#define HIF_REG_PLL_SS_LIMIT3_LIMIT_MASK                           0x03
#define HIF_REG_PLL_SS_LIMIT3_LIMIT_ALIGN                          0
#define HIF_REG_PLL_SS_LIMIT3_LIMIT_BITS                           2
#define HIF_REG_PLL_SS_LIMIT3_LIMIT_SHIFT                          0

/***************************************************************************
 *REG_PLL_SS_LIMIT2 - System PLL Spread Spectrum Limit Control
 ***************************************************************************/
/* HIF :: REG_PLL_SS_LIMIT2 :: LIMIT [07:00] */
#define HIF_REG_PLL_SS_LIMIT2_LIMIT_MASK                           0xff
#define HIF_REG_PLL_SS_LIMIT2_LIMIT_ALIGN                          0
#define HIF_REG_PLL_SS_LIMIT2_LIMIT_BITS                           8
#define HIF_REG_PLL_SS_LIMIT2_LIMIT_SHIFT                          0

/***************************************************************************
 *REG_PLL_SS_LIMIT1 - System PLL Spread Spectrum Limit Control
 ***************************************************************************/
/* HIF :: REG_PLL_SS_LIMIT1 :: LIMIT [07:00] */
#define HIF_REG_PLL_SS_LIMIT1_LIMIT_MASK                           0xff
#define HIF_REG_PLL_SS_LIMIT1_LIMIT_ALIGN                          0
#define HIF_REG_PLL_SS_LIMIT1_LIMIT_BITS                           8
#define HIF_REG_PLL_SS_LIMIT1_LIMIT_SHIFT                          0

/***************************************************************************
 *REG_PLL_SS_LIMIT0 - System PLL Spread Spectrum Limit Control
 ***************************************************************************/
/* HIF :: REG_PLL_SS_LIMIT0 :: LIMIT [07:00] */
#define HIF_REG_PLL_SS_LIMIT0_LIMIT_MASK                           0xff
#define HIF_REG_PLL_SS_LIMIT0_LIMIT_ALIGN                          0
#define HIF_REG_PLL_SS_LIMIT0_LIMIT_BITS                           8
#define HIF_REG_PLL_SS_LIMIT0_LIMIT_SHIFT                          0

/***************************************************************************
 *REG_PLL_MISC_CTRL1 - Register PLL Miscellaneous Control
 ***************************************************************************/
/* HIF :: REG_PLL_MISC_CTRL1 :: RSVD_1 [07:04] */
#define HIF_REG_PLL_MISC_CTRL1_RSVD_1_MASK                         0xf0
#define HIF_REG_PLL_MISC_CTRL1_RSVD_1_ALIGN                        0
#define HIF_REG_PLL_MISC_CTRL1_RSVD_1_BITS                         4
#define HIF_REG_PLL_MISC_CTRL1_RSVD_1_SHIFT                        4

/* HIF :: REG_PLL_MISC_CTRL1 :: RSVD_0 [03:02] */
#define HIF_REG_PLL_MISC_CTRL1_RSVD_0_MASK                         0x0c
#define HIF_REG_PLL_MISC_CTRL1_RSVD_0_ALIGN                        0
#define HIF_REG_PLL_MISC_CTRL1_RSVD_0_BITS                         2
#define HIF_REG_PLL_MISC_CTRL1_RSVD_0_SHIFT                        2

/* HIF :: REG_PLL_MISC_CTRL1 :: NDIV_RELOCK [01:01] */
#define HIF_REG_PLL_MISC_CTRL1_NDIV_RELOCK_MASK                    0x02
#define HIF_REG_PLL_MISC_CTRL1_NDIV_RELOCK_ALIGN                   0
#define HIF_REG_PLL_MISC_CTRL1_NDIV_RELOCK_BITS                    1
#define HIF_REG_PLL_MISC_CTRL1_NDIV_RELOCK_SHIFT                   1

/* HIF :: REG_PLL_MISC_CTRL1 :: FAST_LOCK [00:00] */
#define HIF_REG_PLL_MISC_CTRL1_FAST_LOCK_MASK                      0x01
#define HIF_REG_PLL_MISC_CTRL1_FAST_LOCK_ALIGN                     0
#define HIF_REG_PLL_MISC_CTRL1_FAST_LOCK_BITS                      1
#define HIF_REG_PLL_MISC_CTRL1_FAST_LOCK_SHIFT                     0

/***************************************************************************
 *REG_PLL_MISC_CTRL0 - Register PLL Miscellaneous Control
 ***************************************************************************/
/* HIF :: REG_PLL_MISC_CTRL0 :: PWM_RATE [07:06] */
#define HIF_REG_PLL_MISC_CTRL0_PWM_RATE_MASK                       0xc0
#define HIF_REG_PLL_MISC_CTRL0_PWM_RATE_ALIGN                      0
#define HIF_REG_PLL_MISC_CTRL0_PWM_RATE_BITS                       2
#define HIF_REG_PLL_MISC_CTRL0_PWM_RATE_SHIFT                      6

/* HIF :: REG_PLL_MISC_CTRL0 :: VCO_DLY [05:04] */
#define HIF_REG_PLL_MISC_CTRL0_VCO_DLY_MASK                        0x30
#define HIF_REG_PLL_MISC_CTRL0_VCO_DLY_ALIGN                       0
#define HIF_REG_PLL_MISC_CTRL0_VCO_DLY_BITS                        2
#define HIF_REG_PLL_MISC_CTRL0_VCO_DLY_SHIFT                       4

/* HIF :: REG_PLL_MISC_CTRL0 :: VCO_DIV2 [03:03] */
#define HIF_REG_PLL_MISC_CTRL0_VCO_DIV2_MASK                       0x08
#define HIF_REG_PLL_MISC_CTRL0_VCO_DIV2_ALIGN                      0
#define HIF_REG_PLL_MISC_CTRL0_VCO_DIV2_BITS                       1
#define HIF_REG_PLL_MISC_CTRL0_VCO_DIV2_SHIFT                      3

/* HIF :: REG_PLL_MISC_CTRL0 :: VCO_DIV2_POST [02:02] */
#define HIF_REG_PLL_MISC_CTRL0_VCO_DIV2_POST_MASK                  0x04
#define HIF_REG_PLL_MISC_CTRL0_VCO_DIV2_POST_ALIGN                 0
#define HIF_REG_PLL_MISC_CTRL0_VCO_DIV2_POST_BITS                  1
#define HIF_REG_PLL_MISC_CTRL0_VCO_DIV2_POST_SHIFT                 2

/* HIF :: REG_PLL_MISC_CTRL0 :: AUX [01:01] */
#define HIF_REG_PLL_MISC_CTRL0_AUX_MASK                            0x02
#define HIF_REG_PLL_MISC_CTRL0_AUX_ALIGN                           0
#define HIF_REG_PLL_MISC_CTRL0_AUX_BITS                            1
#define HIF_REG_PLL_MISC_CTRL0_AUX_SHIFT                           1

/* HIF :: REG_PLL_MISC_CTRL0 :: EN_REF_OUT [00:00] */
#define HIF_REG_PLL_MISC_CTRL0_EN_REF_OUT_MASK                     0x01
#define HIF_REG_PLL_MISC_CTRL0_EN_REF_OUT_ALIGN                    0
#define HIF_REG_PLL_MISC_CTRL0_EN_REF_OUT_BITS                     1
#define HIF_REG_PLL_MISC_CTRL0_EN_REF_OUT_SHIFT                    0

/***************************************************************************
 *REG_PLL_STAT_CTRL - Register PLL Status Control
 ***************************************************************************/
/* HIF :: REG_PLL_STAT_CTRL :: UPDATE [07:07] */
#define HIF_REG_PLL_STAT_CTRL_UPDATE_MASK                          0x80
#define HIF_REG_PLL_STAT_CTRL_UPDATE_ALIGN                         0
#define HIF_REG_PLL_STAT_CTRL_UPDATE_BITS                          1
#define HIF_REG_PLL_STAT_CTRL_UPDATE_SHIFT                         7

/* HIF :: REG_PLL_STAT_CTRL :: MODE [06:05] */
#define HIF_REG_PLL_STAT_CTRL_MODE_MASK                            0x60
#define HIF_REG_PLL_STAT_CTRL_MODE_ALIGN                           0
#define HIF_REG_PLL_STAT_CTRL_MODE_BITS                            2
#define HIF_REG_PLL_STAT_CTRL_MODE_SHIFT                           5

/* HIF :: REG_PLL_STAT_CTRL :: RST [04:04] */
#define HIF_REG_PLL_STAT_CTRL_RST_MASK                             0x10
#define HIF_REG_PLL_STAT_CTRL_RST_ALIGN                            0
#define HIF_REG_PLL_STAT_CTRL_RST_BITS                             1
#define HIF_REG_PLL_STAT_CTRL_RST_SHIFT                            4

/* HIF :: REG_PLL_STAT_CTRL :: RSVD [03:03] */
#define HIF_REG_PLL_STAT_CTRL_RSVD_MASK                            0x08
#define HIF_REG_PLL_STAT_CTRL_RSVD_ALIGN                           0
#define HIF_REG_PLL_STAT_CTRL_RSVD_BITS                            1
#define HIF_REG_PLL_STAT_CTRL_RSVD_SHIFT                           3

/* HIF :: REG_PLL_STAT_CTRL :: SEL [02:00] */
#define HIF_REG_PLL_STAT_CTRL_SEL_MASK                             0x07
#define HIF_REG_PLL_STAT_CTRL_SEL_ALIGN                            0
#define HIF_REG_PLL_STAT_CTRL_SEL_BITS                             3
#define HIF_REG_PLL_STAT_CTRL_SEL_SHIFT                            0

/***************************************************************************
 *REG_PLL_STATUS2 - Register PLL Status
 ***************************************************************************/
/* HIF :: REG_PLL_STATUS2 :: RSVD_0 [07:01] */
#define HIF_REG_PLL_STATUS2_RSVD_0_MASK                            0xfe
#define HIF_REG_PLL_STATUS2_RSVD_0_ALIGN                           0
#define HIF_REG_PLL_STATUS2_RSVD_0_BITS                            7
#define HIF_REG_PLL_STATUS2_RSVD_0_SHIFT                           1

/* HIF :: REG_PLL_STATUS2 :: LOCK [00:00] */
#define HIF_REG_PLL_STATUS2_LOCK_MASK                              0x01
#define HIF_REG_PLL_STATUS2_LOCK_ALIGN                             0
#define HIF_REG_PLL_STATUS2_LOCK_BITS                              1
#define HIF_REG_PLL_STATUS2_LOCK_SHIFT                             0

/***************************************************************************
 *REG_PLL_STATUS1 - Register PLL Status
 ***************************************************************************/
/* HIF :: REG_PLL_STATUS1 :: RSVD_0 [07:04] */
#define HIF_REG_PLL_STATUS1_RSVD_0_MASK                            0xf0
#define HIF_REG_PLL_STATUS1_RSVD_0_ALIGN                           0
#define HIF_REG_PLL_STATUS1_RSVD_0_BITS                            4
#define HIF_REG_PLL_STATUS1_RSVD_0_SHIFT                           4

/* HIF :: REG_PLL_STATUS1 :: STAT_OUT [03:00] */
#define HIF_REG_PLL_STATUS1_STAT_OUT_MASK                          0x0f
#define HIF_REG_PLL_STATUS1_STAT_OUT_ALIGN                         0
#define HIF_REG_PLL_STATUS1_STAT_OUT_BITS                          4
#define HIF_REG_PLL_STATUS1_STAT_OUT_SHIFT                         0

/***************************************************************************
 *REG_PLL_STATUS0 - Register PLL Status
 ***************************************************************************/
/* HIF :: REG_PLL_STATUS0 :: STAT_OUT [07:00] */
#define HIF_REG_PLL_STATUS0_STAT_OUT_MASK                          0xff
#define HIF_REG_PLL_STATUS0_STAT_OUT_ALIGN                         0
#define HIF_REG_PLL_STATUS0_STAT_OUT_BITS                          8
#define HIF_REG_PLL_STATUS0_STAT_OUT_SHIFT                         0

/***************************************************************************
 *REG_CLK_EN - Register Clock Enable
 ***************************************************************************/
/* HIF :: REG_CLK_EN :: RSVD [07:02] */
#define HIF_REG_CLK_EN_RSVD_MASK                                   0xfc
#define HIF_REG_CLK_EN_RSVD_ALIGN                                  0
#define HIF_REG_CLK_EN_RSVD_BITS                                   6
#define HIF_REG_CLK_EN_RSVD_SHIFT                                  2

/* HIF :: REG_CLK_EN :: PERIPH [01:01] */
#define HIF_REG_CLK_EN_PERIPH_MASK                                 0x02
#define HIF_REG_CLK_EN_PERIPH_ALIGN                                0
#define HIF_REG_CLK_EN_PERIPH_BITS                                 1
#define HIF_REG_CLK_EN_PERIPH_SHIFT                                1

/* HIF :: REG_CLK_EN :: LEAP [00:00] */
#define HIF_REG_CLK_EN_LEAP_MASK                                   0x01
#define HIF_REG_CLK_EN_LEAP_ALIGN                                  0
#define HIF_REG_CLK_EN_LEAP_BITS                                   1
#define HIF_REG_CLK_EN_LEAP_SHIFT                                  0

/***************************************************************************
 *SYS_CLK_EN - System Clock Enable
 ***************************************************************************/
/* HIF :: SYS_CLK_EN :: RSVD [07:01] */
#define HIF_SYS_CLK_EN_RSVD_MASK                                   0xfe
#define HIF_SYS_CLK_EN_RSVD_ALIGN                                  0
#define HIF_SYS_CLK_EN_RSVD_BITS                                   7
#define HIF_SYS_CLK_EN_RSVD_SHIFT                                  1

/* HIF :: SYS_CLK_EN :: LEAP [00:00] */
#define HIF_SYS_CLK_EN_LEAP_MASK                                   0x01
#define HIF_SYS_CLK_EN_LEAP_ALIGN                                  0
#define HIF_SYS_CLK_EN_LEAP_BITS                                   1
#define HIF_SYS_CLK_EN_LEAP_SHIFT                                  0

/***************************************************************************
 *MISC_CTRL - Miscellaneous Control
 ***************************************************************************/
/* HIF :: MISC_CTRL :: RSVD [07:03] */
#define HIF_MISC_CTRL_RSVD_MASK                                    0xf8
#define HIF_MISC_CTRL_RSVD_ALIGN                                   0
#define HIF_MISC_CTRL_RSVD_BITS                                    5
#define HIF_MISC_CTRL_RSVD_SHIFT                                   3

/* HIF :: MISC_CTRL :: OSC_CML_CTRL_SRC [02:02] */
#define HIF_MISC_CTRL_OSC_CML_CTRL_SRC_MASK                        0x04
#define HIF_MISC_CTRL_OSC_CML_CTRL_SRC_ALIGN                       0
#define HIF_MISC_CTRL_OSC_CML_CTRL_SRC_BITS                        1
#define HIF_MISC_CTRL_OSC_CML_CTRL_SRC_SHIFT                       2

/* HIF :: MISC_CTRL :: RBUS_CLK_SRC [01:01] */
#define HIF_MISC_CTRL_RBUS_CLK_SRC_MASK                            0x02
#define HIF_MISC_CTRL_RBUS_CLK_SRC_ALIGN                           0
#define HIF_MISC_CTRL_RBUS_CLK_SRC_BITS                            1
#define HIF_MISC_CTRL_RBUS_CLK_SRC_SHIFT                           1

/* HIF :: MISC_CTRL :: REG_PLL_STAT_CTRL_SRC [00:00] */
#define HIF_MISC_CTRL_REG_PLL_STAT_CTRL_SRC_MASK                   0x01
#define HIF_MISC_CTRL_REG_PLL_STAT_CTRL_SRC_ALIGN                  0
#define HIF_MISC_CTRL_REG_PLL_STAT_CTRL_SRC_BITS                   1
#define HIF_MISC_CTRL_REG_PLL_STAT_CTRL_SRC_SHIFT                  0

/***************************************************************************
 *SPARE3 - Spare Register
 ***************************************************************************/
/* HIF :: SPARE3 :: SPARE [07:00] */
#define HIF_SPARE3_SPARE_MASK                                      0xff
#define HIF_SPARE3_SPARE_ALIGN                                     0
#define HIF_SPARE3_SPARE_BITS                                      8
#define HIF_SPARE3_SPARE_SHIFT                                     0

/***************************************************************************
 *SPARE2 - Spare Register
 ***************************************************************************/
/* HIF :: SPARE2 :: SPARE [07:00] */
#define HIF_SPARE2_SPARE_MASK                                      0xff
#define HIF_SPARE2_SPARE_ALIGN                                     0
#define HIF_SPARE2_SPARE_BITS                                      8
#define HIF_SPARE2_SPARE_SHIFT                                     0

/***************************************************************************
 *SPARE1 - Spare Register
 ***************************************************************************/
/* HIF :: SPARE1 :: SPARE [07:00] */
#define HIF_SPARE1_SPARE_MASK                                      0xff
#define HIF_SPARE1_SPARE_ALIGN                                     0
#define HIF_SPARE1_SPARE_BITS                                      8
#define HIF_SPARE1_SPARE_SHIFT                                     0

/***************************************************************************
 *SPARE0 - Spare Register
 ***************************************************************************/
/* HIF :: SPARE0 :: SPARE [07:00] */
#define HIF_SPARE0_SPARE_MASK                                      0xff
#define HIF_SPARE0_SPARE_ALIGN                                     0
#define HIF_SPARE0_SPARE_BITS                                      8
#define HIF_SPARE0_SPARE_SHIFT                                     0

/***************************************************************************
 *SFT3 - Software Register
 ***************************************************************************/
/* HIF :: SFT3 :: SFT [07:00] */
#define HIF_SFT3_SFT_MASK                                          0xff
#define HIF_SFT3_SFT_ALIGN                                         0
#define HIF_SFT3_SFT_BITS                                          8
#define HIF_SFT3_SFT_SHIFT                                         0

/***************************************************************************
 *SFT2 - Software Register
 ***************************************************************************/
/* HIF :: SFT2 :: SFT [07:00] */
#define HIF_SFT2_SFT_MASK                                          0xff
#define HIF_SFT2_SFT_ALIGN                                         0
#define HIF_SFT2_SFT_BITS                                          8
#define HIF_SFT2_SFT_SHIFT                                         0

/***************************************************************************
 *SFT1 - Software Register
 ***************************************************************************/
/* HIF :: SFT1 :: SFT [07:00] */
#define HIF_SFT1_SFT_MASK                                          0xff
#define HIF_SFT1_SFT_ALIGN                                         0
#define HIF_SFT1_SFT_BITS                                          8
#define HIF_SFT1_SFT_SHIFT                                         0

/***************************************************************************
 *SFT0 - Software Register
 ***************************************************************************/
/* HIF :: SFT0 :: SFT [07:00] */
#define HIF_SFT0_SFT_MASK                                          0xff
#define HIF_SFT0_SFT_ALIGN                                         0
#define HIF_SFT0_SFT_BITS                                          8
#define HIF_SFT0_SFT_SHIFT                                         0

#endif /* #ifndef BCHP_HSI_H__ */

/* End of File */
