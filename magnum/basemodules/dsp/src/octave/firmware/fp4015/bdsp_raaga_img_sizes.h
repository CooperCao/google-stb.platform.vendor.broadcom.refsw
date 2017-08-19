/*******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/


/****Kernel specific parameters for the host ****/
#define BDSP_IMG_KERNEL_RW_IMG_SIZE                               	 ((uint32_t)4260352)
#define BDSP_IMG_TB_BUF_START_ADDR                                   ((uint32_t)0x10000040)
#define BDSP_IMG_TB_BUF_MEM_SIZE                                     ((uint32_t)4194304)

/****MM Server specific parameters for the host ****/
#define BDSP_IMG_INIT_PROCESS_MEM_SIZE                               ((uint32_t)256000)
#define BDSP_IMG_TOPLEVEL_PROCESS_MEM_SIZE                           ((uint32_t)2048000)
#define BDSP_IMG_DEFAULT_MM_PROC_HEAP_SIZE                           ((uint32_t)256000)
#define BDSP_IMG_USER_PROCESS_SPAWN_MEM_SIZE                         ((uint32_t)409600)

/****Size of RDB vars ****/
#define BDSP_IMG_SYSTEM_RDBVARS_SIZE                               	 ((uint32_t)1024)

/****Size of Shared memory between MP and AP ****/
#define BDSP_IMG_MP_AP_SHARED_MEMORY_SIZE                            ((uint32_t)4096)

/****Details of Kernel and ROMFS for the host ****/


/*FPOS_KERNEL Size requirement*/
#define BDSP_IMG_SYSTEM_KERNEL_SIZE                                  ((uint32_t)138752)

/*INIT_ROMFS Size requirement*/
#define BDSP_IMG_INIT_ROMFS_SIZE                                     ((uint32_t)655360)

/*SYSTEM_CODE Size requirement*/
#define BDSP_IMG_SYSTEM_CODE_SIZE                                    ((uint32_t)393216)

/*SYSTEM_LIB Size requirement*/
#define BDSP_IMG_SYSTEM_LIB_SIZE                                     ((uint32_t)440320)
/****Codec specific parameters for the host ****/


/*ADEC_MPEG1 Size requirement*/
#define BDSP_IMG_ADEC_MPEG1_SCRATCH_SIZE                             ((uint32_t)36864)
#define BDSP_IMG_ADEC_MPEG1_SIZE                                     ((uint32_t)40960)
#define BDSP_IMG_ADEC_MPEG1_INTER_FRAME_SIZE                         ((uint32_t)10752)
#define BDSP_IMG_ADEC_MPEG1_INTER_FRAME_ENCODED_SIZE                 ((uint32_t)512)
#define BDSP_IMG_ADEC_MPEG1_TABLES_SIZE                              ((uint32_t)15360)

/*AIDS_MPEG1 Size requirement*/
#define BDSP_IMG_AIDS_MPEG1_SIZE                                     ((uint32_t)13824)

/*APP_SRC Size requirement*/
#define BDSP_IMG_APP_SRC_SCRATCH_SIZE                                ((uint32_t)45056)
#define BDSP_IMG_APP_SRC_SIZE                                        ((uint32_t)19456)
#define BDSP_IMG_APP_SRC_INTER_FRAME_SIZE                            ((uint32_t)14336)
#define BDSP_IMG_APP_SRC_INTER_FRAME_ENCODED_SIZE                    ((uint32_t)512)
#define BDSP_IMG_APP_SRC_TABLES_SIZE                                 ((uint32_t)4096)

/*ADEC_UDC Size requirement*/
#define BDSP_IMG_ADEC_UDC_SCRATCH_SIZE                               ((uint32_t)217088)
#define BDSP_IMG_ADEC_UDC_SIZE                                       ((uint32_t)629760)
#define BDSP_IMG_ADEC_UDC_INTER_FRAME_SIZE                           ((uint32_t)1049600)
#define BDSP_IMG_ADEC_UDC_INTER_FRAME_ENCODED_SIZE                   ((uint32_t)512)
#define BDSP_IMG_ADEC_UDC_TABLES_SIZE                                ((uint32_t)49664)

/*ADEC_DDP Size requirement*/
#define BDSP_IMG_ADEC_DDP_SCRATCH_SIZE                               ((uint32_t)217088)
#define BDSP_IMG_ADEC_DDP_SIZE                                       ((uint32_t)629760)
#define BDSP_IMG_ADEC_DDP_INTER_FRAME_SIZE                           ((uint32_t)1049600)
#define BDSP_IMG_ADEC_DDP_INTER_FRAME_ENCODED_SIZE                   ((uint32_t)512)
#define BDSP_IMG_ADEC_DDP_TABLES_SIZE                                ((uint32_t)49664)

/*ADEC_AC3 Size requirement*/
#define BDSP_IMG_ADEC_AC3_SCRATCH_SIZE                               ((uint32_t)217088)
#define BDSP_IMG_ADEC_AC3_SIZE                                       ((uint32_t)630272)
#define BDSP_IMG_ADEC_AC3_INTER_FRAME_SIZE                           ((uint32_t)1049600)
#define BDSP_IMG_ADEC_AC3_INTER_FRAME_ENCODED_SIZE                   ((uint32_t)512)
#define BDSP_IMG_ADEC_AC3_TABLES_SIZE                                ((uint32_t)49664)

/*AIDS_DDP Size requirement*/
#define BDSP_IMG_AIDS_DDP_SIZE                                       ((uint32_t)19456)

/*AIDS_ADTS Size requirement*/
#define BDSP_IMG_AIDS_ADTS_SIZE                                      ((uint32_t)9216)

/*AIDS_LOAS Size requirement*/
#define BDSP_IMG_AIDS_LOAS_SIZE                                      ((uint32_t)10752)

/*ADEC_AACHE Size requirement*/
#define BDSP_IMG_ADEC_AACHE_SCRATCH_SIZE                             ((uint32_t)249856)
#define BDSP_IMG_ADEC_AACHE_SIZE                                     ((uint32_t)156672)
#define BDSP_IMG_ADEC_AACHE_INTER_FRAME_SIZE                         ((uint32_t)185344)
#define BDSP_IMG_ADEC_AACHE_INTER_FRAME_ENCODED_SIZE                 ((uint32_t)512)
#define BDSP_IMG_ADEC_AACHE_TABLES_SIZE                              ((uint32_t)71680)

/*ADEC_PASSTHRU Size requirement*/
#define BDSP_IMG_ADEC_PASSTHRU_SCRATCH_SIZE                          ((uint32_t)36864)
#define BDSP_IMG_ADEC_PASSTHRU_SIZE                                  ((uint32_t)30208)
#define BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_SIZE                      ((uint32_t)2560)
#define BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_ENCODED_SIZE              ((uint32_t)512)
#define BDSP_IMG_ADEC_PASSTHRU_TABLES_SIZE                           ((uint32_t)1536)

/*AIDS_WAVFORMATEX Size requirement*/
#define BDSP_IMG_AIDS_WAVFORMATEX_SIZE                               ((uint32_t)10240)

/*ADEC_PCMWAV Size requirement*/
#define BDSP_IMG_ADEC_PCMWAV_SCRATCH_SIZE                            ((uint32_t)32768)
#define BDSP_IMG_ADEC_PCMWAV_SIZE                                    ((uint32_t)18944)
#define BDSP_IMG_ADEC_PCMWAV_INTER_FRAME_SIZE                        ((uint32_t)1536)
#define BDSP_IMG_ADEC_PCMWAV_INTER_FRAME_ENCODED_SIZE                ((uint32_t)512)
#define BDSP_IMG_ADEC_PCMWAV_TABLES_SIZE                             ((uint32_t)1536)