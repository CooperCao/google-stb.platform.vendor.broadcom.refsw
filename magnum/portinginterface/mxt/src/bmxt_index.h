/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#ifndef BMXT_INDEX_H__
#define BMXT_INDEX_H__

#define BMXT_RESOURCE_IB0_CTRL                    0
#define BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1      1
#define BMXT_RESOURCE_TSMF0_CTRL                  2
#define BMXT_RESOURCE_MTSIF_RX0_CTRL1             3
#define BMXT_RESOURCE_MTSIF_TX0_CTRL1             4
#define BMXT_RESOURCE_MINI_PID_PARSER0_TB_CTRL1   5

#define BCHP_DEMOD_XPT_FE_FE_CTRL                                                           0
#define BCHP_DEMOD_XPT_FE_PWR_CTRL                                                          1
#define BCHP_DEMOD_XPT_FE_MAX_PID_CHANNEL                                                   2
#define BCHP_DEMOD_XPT_FE_IB_SYNC_DETECT_CTRL                                               3
#define BCHP_DEMOD_XPT_FE_INTR_STATUS0_REG                                                  4
#define BCHP_DEMOD_XPT_FE_INTR_STATUS1_REG                                                  5
#define BCHP_DEMOD_XPT_FE_INTR_STATUS2_REG                                                  6
#define BCHP_DEMOD_XPT_FE_INTR_STATUS0_REG_EN                                               7
#define BCHP_DEMOD_XPT_FE_INTR_STATUS1_REG_EN                                               8
#define BCHP_DEMOD_XPT_FE_INTR_STATUS2_REG_EN                                               9
#define BCHP_DEMOD_XPT_FE_TSMF_INTR_STATUS0_REG                                            10
#define BCHP_DEMOD_XPT_FE_TSMF_INTR_STATUS0_REG_EN                                         11
#define BCHP_DEMOD_XPT_FE_MTSIF_RX_INTR_STATUS0_REG                                        12
#define BCHP_DEMOD_XPT_FE_MTSIF_RX_INTR_STATUS0_REG_EN                                     13
#define BCHP_DEMOD_XPT_FE_PARSERS_TSMF_FRAME_ERROR_INTR_STATUS0_REG                        14
#define BCHP_DEMOD_XPT_FE_PARSERS_TSMF_SYNC_ERROR_INTR_STATUS0_REG                         15
#define BCHP_DEMOD_XPT_FE_PARSERS_INBUFF_OVFL_ERROR_INTR_STATUS0_REG                       16
#define BCHP_DEMOD_XPT_FE_PARSERS_TSMF_FRAME_ERROR_INTR_STATUS0_REG_EN                     17
#define BCHP_DEMOD_XPT_FE_PARSERS_TSMF_SYNC_ERROR_INTR_STATUS0_REG_EN                      18
#define BCHP_DEMOD_XPT_FE_PARSERS_INBUFF_OVFL_ERROR_INTR_STATUS0_REG_EN                    19
#define BCHP_DEMOD_XPT_FE_INBUF_MEM_PWR_DN_GLOBAL_CTRL                                     20
#define BCHP_DEMOD_XPT_FE_INBUF_MEM0_MEM31_PWR_DN_CTRL                                     21
#define BCHP_DEMOD_XPT_FE_INBUF_MEM0_MEM31_PWR_DN_STATUS                                   22
#define BCHP_DEMOD_XPT_FE_MEM_INIT_CTRL                                                    23
#define BCHP_DEMOD_XPT_FE_PARSER_BAND0_BAND31_SRC                                          24
#define BCHP_DEMOD_XPT_FE_PARSER_BAND0_BAND15_MTSIF_RX_SRC                                 25
#define BCHP_DEMOD_XPT_FE_PARSER_BAND16_BAND31_MTSIF_RX_SRC                                26
#define BCHP_DEMOD_XPT_FE_ATS_COUNTER_CTRL                                                 27
#define BCHP_DEMOD_XPT_FE_ATS_TS_MOD300                                                    28
#define BCHP_DEMOD_XPT_FE_ATS_TS_BINARY                                                    29
#define BCHP_DEMOD_XPT_FE_TV_STATUS_0                                                      30
#define BCHP_DEMOD_XPT_FE_MTSIF_TX_TV_CTRL                                                 31
#define BCHP_DEMOD_XPT_FE_IB0_CTRL                                                         32
#define BCHP_DEMOD_XPT_FE_IB0_CTRL2                                                        33
#define BCHP_DEMOD_XPT_FE_IB0_SYNC_COUNT                                                   34
#define BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1                                           35
#define BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL2                                           36
#define BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_ALL_PASS_CTRL                                   37
#define BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID                              38
#define BCHP_DEMOD_XPT_FE_TSMF0_CTRL                                                       39
#define BCHP_DEMOD_XPT_FE_TSMF0_SLOT_MAP_LO                                                40
#define BCHP_DEMOD_XPT_FE_TSMF0_SLOT_MAP_HI                                                41
#define BCHP_DEMOD_XPT_FE_TSMF0_STATUS                                                     42
#define BCHP_DEMOD_XPT_FE_MTSIF_RX0_CTRL1                                                  43
#define BCHP_DEMOD_XPT_FE_MTSIF_RX0_SECRET_WORD                                            44
#define BCHP_DEMOD_XPT_FE_MTSIF_RX0_BAND0_BAND31_ID_DROP                                   45
#define BCHP_DEMOD_XPT_FE_MTSIF_RX0_BAND0_BAND3_ID                                         46
#define BCHP_DEMOD_XPT_FE_MTSIF_RX0_PKT_BAND0_BAND31_DETECT                                47
#define BCHP_DEMOD_XPT_FE_MTSIF_TX0_CTRL1                                                  48
#define BCHP_DEMOD_XPT_FE_MTSIF_TX0_BLOCK_OUT                                              49
#define BCHP_DEMOD_XPT_FE_MTSIF_TX0_STATUS                                                 50
#define BCHP_DEMOD_XPT_FE_MTSIF_TX0_SECRET_WORD                                            51
#define BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND0_BAND3_ID                                         52
#define BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_TB_CTRL1                                        53
#define BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_SYNC_COUNT                                      54
#define BCHP_DEMOD_XPT_FE_TB_GLOBAL_CTRL1                                                  55
#define BCHP_DEMOD_XPT_FE_TB_GLOBAL_CTRL2                                                  56
#define BCHP_DEMOD_XPT_FE_CHIP_SUB_VARIANT_ID                                              57
#define BCHP_DEMOD_XPT_FE_TB_PARSERS_PRIVATE_DATA_LENGTH_ERROR_STATUS0_REG                 58
#define BCHP_DEMOD_XPT_FE_TB_PARSERS_PRIVATE_DATA_FIELD_LENGTH_ERROR_STATUS0_REG           59
#define BCHP_DEMOD_XPT_FE_PID_TABLE_i_ARRAY_BASE                                           60
#define BCHP_DEMOD_XPT_FE_SPID_TABLE_i_ARRAY_BASE                                          61

#endif
