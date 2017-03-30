/*******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/


#define BDSP_IMG_AACHEADTS_IDS_INTER_FRAME_SIZE                      ((uint32_t)16928)
#define BDSP_IMG_AACHEADTS_IDS_SIZE                                  ((uint32_t)28780)
#define BDSP_IMG_AACHELOAS_IDS_INTER_FRAME_SIZE                      ((uint32_t)16928)
#define BDSP_IMG_AACHELOAS_IDS_SIZE                                  ((uint32_t)30204)
#define BDSP_IMG_CDB_PASSTHRU_CODE_SIZE                              ((uint32_t)31144)
#define BDSP_IMG_CDB_PASSTHRU_INTER_FRAME_SIZE                       ((uint32_t)852)
#define BDSP_IMG_CDB_PASSTHRU_TABLES_SIZE                            ((uint32_t)640)
#define BDSP_IMG_DDP_ENCODE_CODE_SIZE                                ((uint32_t)298520)
#define BDSP_IMG_DDP_ENCODE_INTER_FRAME_SIZE                         ((uint32_t)217536)
#define BDSP_IMG_DDP_ENCODE_TABLES_SIZE                              ((uint32_t)640)
#define BDSP_IMG_DOLBY_AACHE_DECODE_INTER_FRAME_SIZE                 ((uint32_t)211772)
#define BDSP_IMG_DOLBY_AACHE_DECODE_SIZE                             ((uint32_t)243748)
#define BDSP_IMG_DOLBY_AACHE_DECODE_TABLES_SIZE                      ((uint32_t)640)
#define BDSP_IMG_DPCMR_CODE_SIZE                                     ((uint32_t)52904)
#define BDSP_IMG_DPCMR_INTER_FRAME_SIZE                              ((uint32_t)173020)
#define BDSP_IMG_DPCMR_TABLES_SIZE                                   ((uint32_t)640)
#define BDSP_IMG_MIXER_DAPV2_CODE_SIZE                               ((uint32_t)405196)
#define BDSP_IMG_MIXER_DAPV2_IDS_INTER_FRAME_SIZE                    ((uint32_t)4016)
#define BDSP_IMG_MIXER_DAPV2_IDS_SIZE                                ((uint32_t)11852)
#define BDSP_IMG_MIXER_DAPV2_INTER_FRAME_SIZE                        ((uint32_t)712320)
#define BDSP_IMG_MIXER_DAPV2_TABLES_SIZE                             ((uint32_t)640)
#define BDSP_IMG_MPEG1_DECODE_INTER_FRAME_SIZE                       ((uint32_t)9300)
#define BDSP_IMG_MPEG1_DECODE_SIZE                                   ((uint32_t)54828)
#define BDSP_IMG_MPEG1_DECODE_TABLES_SIZE                            ((uint32_t)640)
#define BDSP_IMG_MPEG1_IDS_INTER_FRAME_SIZE                          ((uint32_t)16952)
#define BDSP_IMG_MPEG1_IDS_SIZE                                      ((uint32_t)39440)
#define BDSP_IMG_SRC_CODE_SIZE                                       ((uint32_t)23352)
#define BDSP_IMG_SRC_INTER_FRAME_SIZE                                ((uint32_t)13028)
#define BDSP_IMG_SRC_TABLES_SIZE                                     ((uint32_t)640)
#define BDSP_IMG_SYSTEM_KERNEL_SIZE                                  ((uint32_t)181248)
#define BDSP_IMG_SYSTEM_ROMFS_SIZE                                   ((uint32_t)803332)
#define BDSP_IMG_UDC_DECODE_INTER_FRAME_SIZE                         ((uint32_t)566488)
#define BDSP_IMG_UDC_DECODE_SIZE                                     ((uint32_t)236384)
#define BDSP_IMG_UDC_DECODE_TABLES_SIZE                              ((uint32_t)640)
#define BDSP_IMG_UDC_IDS_INTER_FRAME_SIZE                            ((uint32_t)16952)
#define BDSP_IMG_UDC_IDS_SIZE                                        ((uint32_t)40772)
#define BDSP_IMG_VIDIDSCOMMON_CODE_SIZE                              ((uint32_t)11684)
#define BDSP_IMG_VIDIDSCOMMON_INTER_FRAME_SIZE                       ((uint32_t)2208)
#define BDSP_IMG_SYSTEM_RDBVARS_SIZE                               	 ((uint32_t)1024)
#define BDSP_IMG_KERNEL_RW_IMG_SIZE                               	 ((uint32_t)4321280)
#define BDSP_TB_BUF_START_ADDR                               		 ((uint32_t)0x10001a70)
#define BDSP_TB_BUF_MEM_SIZE                               			 ((uint32_t)4194304)
