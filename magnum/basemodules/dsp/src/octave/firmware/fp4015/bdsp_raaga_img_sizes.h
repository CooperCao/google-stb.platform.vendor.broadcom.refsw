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
#ifndef BCHP_RAAGA_DSP_L2C_HW_PREFETCH_CTRL
#define BDSP_IMG_KERNEL_RW_IMG_SIZE                               	 ((uint32_t)59904)
#define BDSP_IMG_TB_BUF_START_ADDR                                   ((uint32_t)0x1000ea00)
#else
#define BDSP_IMG_KERNEL_RW_IMG_SIZE                               	 ((uint32_t)60416)
#define BDSP_IMG_TB_BUF_START_ADDR                                   ((uint32_t)0x1000ec00)
#endif

#define BDSP_IMG_TB_BUF_MEM_SIZE                                     ((uint32_t)0)

/****MM Server specific parameters for the host ****/
#define BDSP_IMG_INIT_PROCESS_MEM_SIZE                               ((uint32_t)256000)
#define BDSP_IMG_TOPLEVEL_PROCESS_MEM_SIZE                           ((uint32_t)1536000)
#define BDSP_IMG_DEFAULT_MM_PROC_HEAP_SIZE                           ((uint32_t)256000)
#define BDSP_IMG_USER_PROCESS_SPAWN_MEM_SIZE                         ((uint32_t)210000)

/****Size of RDB vars ****/
#define BDSP_IMG_SYSTEM_RDBVARS_SIZE                               	 ((uint32_t)1024)

/****Size of Shared memory between MP and AP ****/
#define BDSP_IMG_MP_AP_SHARED_MEMORY_SIZE                            ((uint32_t)4096)

/****Details of Kernel and ROMFS for the host ****/
#if (BCHP_VER == BCHP_VER_A0)


/*FPOS_KERNEL_A0 Size requirement*/
#define BDSP_IMG_SYSTEM_KERNEL_SIZE                                  ((uint32_t)140800)
#else


/*FPOS_KERNEL_B0 Size requirement*/
#define BDSP_IMG_SYSTEM_KERNEL_SIZE                                  ((uint32_t)139264)
#endif


/*INIT_ROMFS Size requirement*/
#define BDSP_IMG_INIT_ROMFS_SIZE                                     ((uint32_t)655360)

/*SYSTEM_CODE Size requirement*/
#define BDSP_IMG_SYSTEM_CODE_SIZE                                    ((uint32_t)393216)

/*SYSTEM_LIB Size requirement*/
#define BDSP_IMG_SYSTEM_LIB_SIZE                                     ((uint32_t)601600)
/****Codec specific parameters for the host ****/


/*ADEC_MPEG1 Size requirement*/
#define BDSP_IMG_ADEC_MPEG1_SCRATCH_SIZE                             ((uint32_t)36864)
#define BDSP_IMG_ADEC_MPEG1_SIZE                                     ((uint32_t)43008)
#define BDSP_IMG_ADEC_MPEG1_INTER_FRAME_SIZE                         ((uint32_t)10752)
#define BDSP_IMG_ADEC_MPEG1_INTER_FRAME_ENCODED_SIZE                 ((uint32_t)512)
#define BDSP_IMG_ADEC_MPEG1_TABLES_SIZE                              ((uint32_t)15360)

/*AIDS_MPEG1 Size requirement*/
#define BDSP_IMG_AIDS_MPEG1_SIZE                                     ((uint32_t)14336)

/*APP_SRC Size requirement*/
#define BDSP_IMG_APP_SRC_SCRATCH_SIZE                                ((uint32_t)45056)
#define BDSP_IMG_APP_SRC_SIZE                                        ((uint32_t)20480)
#define BDSP_IMG_APP_SRC_INTER_FRAME_SIZE                            ((uint32_t)14336)
#define BDSP_IMG_APP_SRC_INTER_FRAME_ENCODED_SIZE                    ((uint32_t)512)
#define BDSP_IMG_APP_SRC_TABLES_SIZE                                 ((uint32_t)4096)

/*APP_FW_MIXER Size requirement*/
#define BDSP_IMG_APP_FW_MIXER_SCRATCH_SIZE                           ((uint32_t)102400)
#define BDSP_IMG_APP_FW_MIXER_SIZE                                   ((uint32_t)69120)
#define BDSP_IMG_APP_FW_MIXER_INTER_FRAME_SIZE                       ((uint32_t)33280)
#define BDSP_IMG_APP_FW_MIXER_INTER_FRAME_ENCODED_SIZE               ((uint32_t)512)
#define BDSP_IMG_APP_FW_MIXER_TABLES_SIZE                            ((uint32_t)6144)

/*APP_MIXER_DAPV2 Size requirement*/
#define BDSP_IMG_APP_MIXER_DAPV2_SCRATCH_SIZE                        ((uint32_t)1597440)
#define BDSP_IMG_APP_MIXER_DAPV2_SIZE                                ((uint32_t)178688)
#define BDSP_IMG_APP_MIXER_DAPV2_INTER_FRAME_SIZE                    ((uint32_t)523776)
#define BDSP_IMG_APP_MIXER_DAPV2_INTER_FRAME_ENCODED_SIZE            ((uint32_t)512)
#define BDSP_IMG_APP_MIXER_DAPV2_TABLES_SIZE                         ((uint32_t)208896)

/*ADEC_UDC Size requirement*/
#define BDSP_IMG_ADEC_UDC_SCRATCH_SIZE                               ((uint32_t)217088)
#define BDSP_IMG_ADEC_UDC_SIZE                                       ((uint32_t)261120)
#define BDSP_IMG_ADEC_UDC_INTER_FRAME_SIZE                           ((uint32_t)1049600)
#define BDSP_IMG_ADEC_UDC_INTER_FRAME_ENCODED_SIZE                   ((uint32_t)512)
#define BDSP_IMG_ADEC_UDC_TABLES_SIZE                                ((uint32_t)177152)

/*ADEC_DDP Size requirement*/
#define BDSP_IMG_ADEC_DDP_SCRATCH_SIZE                               ((uint32_t)217088)
#define BDSP_IMG_ADEC_DDP_SIZE                                       ((uint32_t)261120)
#define BDSP_IMG_ADEC_DDP_INTER_FRAME_SIZE                           ((uint32_t)1049600)
#define BDSP_IMG_ADEC_DDP_INTER_FRAME_ENCODED_SIZE                   ((uint32_t)512)
#define BDSP_IMG_ADEC_DDP_TABLES_SIZE                                ((uint32_t)177152)

/*ADEC_AC3 Size requirement*/
#define BDSP_IMG_ADEC_AC3_SCRATCH_SIZE                               ((uint32_t)217088)
#define BDSP_IMG_ADEC_AC3_SIZE                                       ((uint32_t)261120)
#define BDSP_IMG_ADEC_AC3_INTER_FRAME_SIZE                           ((uint32_t)1049600)
#define BDSP_IMG_ADEC_AC3_INTER_FRAME_ENCODED_SIZE                   ((uint32_t)512)
#define BDSP_IMG_ADEC_AC3_TABLES_SIZE                                ((uint32_t)177152)

/*ADEC_DOLBY_AACHE Size requirement*/
#define BDSP_IMG_ADEC_DOLBY_AACHE_SCRATCH_SIZE                       ((uint32_t)528384)
#define BDSP_IMG_ADEC_DOLBY_AACHE_SIZE                               ((uint32_t)193024)
#define BDSP_IMG_ADEC_DOLBY_AACHE_INTER_FRAME_SIZE                   ((uint32_t)525312)
#define BDSP_IMG_ADEC_DOLBY_AACHE_INTER_FRAME_ENCODED_SIZE           ((uint32_t)512)
#define BDSP_IMG_ADEC_DOLBY_AACHE_TABLES_SIZE                        ((uint32_t)158720)

/*AIDS_DDP Size requirement*/
#define BDSP_IMG_AIDS_DDP_SIZE                                       ((uint32_t)20992)

/*AIDS_ADTS Size requirement*/
#define BDSP_IMG_AIDS_ADTS_SIZE                                      ((uint32_t)9728)

/*AIDS_LOAS Size requirement*/
#define BDSP_IMG_AIDS_LOAS_SIZE                                      ((uint32_t)11264)

/*ADEC_AACHE Size requirement*/
#define BDSP_IMG_ADEC_AACHE_SCRATCH_SIZE                             ((uint32_t)249856)
#define BDSP_IMG_ADEC_AACHE_SIZE                                     ((uint32_t)157696)
#define BDSP_IMG_ADEC_AACHE_INTER_FRAME_SIZE                         ((uint32_t)185344)
#define BDSP_IMG_ADEC_AACHE_INTER_FRAME_ENCODED_SIZE                 ((uint32_t)512)
#define BDSP_IMG_ADEC_AACHE_TABLES_SIZE                              ((uint32_t)71680)

/*ADEC_PASSTHRU Size requirement*/
#define BDSP_IMG_ADEC_PASSTHRU_SCRATCH_SIZE                          ((uint32_t)36864)
#define BDSP_IMG_ADEC_PASSTHRU_SIZE                                  ((uint32_t)44544)
#define BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_SIZE                      ((uint32_t)2048)
#define BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_ENCODED_SIZE              ((uint32_t)512)
#define BDSP_IMG_ADEC_PASSTHRU_TABLES_SIZE                           ((uint32_t)1536)

/*AIDS_WAVFORMATEX Size requirement*/
#define BDSP_IMG_AIDS_WAVFORMATEX_SIZE                               ((uint32_t)10752)

/*ADEC_PCMWAV Size requirement*/
#define BDSP_IMG_ADEC_PCMWAV_SCRATCH_SIZE                            ((uint32_t)32768)
#define BDSP_IMG_ADEC_PCMWAV_SIZE                                    ((uint32_t)18432)
#define BDSP_IMG_ADEC_PCMWAV_INTER_FRAME_SIZE                        ((uint32_t)1536)
#define BDSP_IMG_ADEC_PCMWAV_INTER_FRAME_ENCODED_SIZE                ((uint32_t)512)
#define BDSP_IMG_ADEC_PCMWAV_TABLES_SIZE                             ((uint32_t)1536)

/*APP_DPCMR Size requirement*/
#define BDSP_IMG_APP_DPCMR_SCRATCH_SIZE                              ((uint32_t)40960)
#define BDSP_IMG_APP_DPCMR_SIZE                                      ((uint32_t)56320)
#define BDSP_IMG_APP_DPCMR_INTER_FRAME_SIZE                          ((uint32_t)175616)
#define BDSP_IMG_APP_DPCMR_INTER_FRAME_ENCODED_SIZE                  ((uint32_t)512)
#define BDSP_IMG_APP_DPCMR_TABLES_SIZE                               ((uint32_t)3584)

/*AENC_DDP Size requirement*/
#define BDSP_IMG_AENC_DDP_SCRATCH_SIZE                               ((uint32_t)503808)
#define BDSP_IMG_AENC_DDP_SIZE                                       ((uint32_t)343552)
#define BDSP_IMG_AENC_DDP_INTER_FRAME_SIZE                           ((uint32_t)801280)
#define BDSP_IMG_AENC_DDP_INTER_FRAME_ENCODED_SIZE                   ((uint32_t)512)
#define BDSP_IMG_AENC_DDP_TABLES_SIZE                                ((uint32_t)155136)

/*AENC_AACHE Size requirement*/
#define BDSP_IMG_AENC_AACHE_SCRATCH_SIZE                             ((uint32_t)110592)
#define BDSP_IMG_AENC_AACHE_SIZE                                     ((uint32_t)256000)
#define BDSP_IMG_AENC_AACHE_INTER_FRAME_SIZE                         ((uint32_t)180224)
#define BDSP_IMG_AENC_AACHE_INTER_FRAME_ENCODED_SIZE                 ((uint32_t)512)
#define BDSP_IMG_AENC_AACHE_TABLES_SIZE                              ((uint32_t)25088)

/*APP_GEN_CDBITB Size requirement*/
#define BDSP_IMG_APP_GEN_CDBITB_SCRATCH_SIZE                         ((uint32_t)4096)
#define BDSP_IMG_APP_GEN_CDBITB_SIZE                                 ((uint32_t)17408)
#define BDSP_IMG_APP_GEN_CDBITB_INTER_FRAME_SIZE                     ((uint32_t)2048)
#define BDSP_IMG_APP_GEN_CDBITB_INTER_FRAME_ENCODED_SIZE             ((uint32_t)512)
#define BDSP_IMG_APP_GEN_CDBITB_TABLES_SIZE                          ((uint32_t)1536)

/*APP_DSOLA Size requirement*/
#define BDSP_IMG_APP_DSOLA_SCRATCH_SIZE                              ((uint32_t)4096)
#define BDSP_IMG_APP_DSOLA_SIZE                                      ((uint32_t)10752)
#define BDSP_IMG_APP_DSOLA_INTER_FRAME_SIZE                          ((uint32_t)78336)
#define BDSP_IMG_APP_DSOLA_INTER_FRAME_ENCODED_SIZE                  ((uint32_t)512)
#define BDSP_IMG_APP_DSOLA_TABLES_SIZE                               ((uint32_t)2048)