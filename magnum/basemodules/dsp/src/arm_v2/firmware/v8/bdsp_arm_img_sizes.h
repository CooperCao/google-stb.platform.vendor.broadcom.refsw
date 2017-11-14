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

/****Size of RDB vars ****/
#define BDSP_ARM_IMG_SYSTEM_RDBVARS_SIZE                               ((uint32_t)1024)

/**** Size of RDB mapped space ****/
#define BDSP_ARM_IMG_SYSTEM_IO_SIZE                                   ((uint32_t)8192)

/**** Size of Shared memory between MP and AP ****/
#define BDSP_ARM_IMG_MP_AP_SHARED_MEMORY_SIZE                          ((uint32_t)4096)

/****Details of Kernel and ROMFS for the host ****/

/****Codec specific parameters for the host ****/


/*COMMON Size requirement*/
#define BDSP_ARM_IMG_COMMON_SIZE                                     ((uint32_t)28672)

/*MM_CLIENT Size requirement*/
#define BDSP_ARM_IMG_MM_CLIENT_SIZE                                  ((uint32_t)90624)

/*KERNEL_OPS Size requirement*/
#define BDSP_ARM_IMG_KERNEL_OPS_SIZE                                 ((uint32_t)73728)

/*STAGE_IO Size requirement*/
#define BDSP_ARM_IMG_STAGE_IO_SIZE                                   ((uint32_t)189440)

/*COMMON_IDS Size requirement*/
#define BDSP_ARM_IMG_COMMON_IDS_SIZE                                 ((uint32_t)236544)

/*INIT_PROCESS Size requirement*/
#define BDSP_ARM_IMG_INIT_PROCESS_SIZE                               ((uint32_t)28160)

/*SCHEDULING_PROCESS Size requirement*/
#define BDSP_ARM_IMG_SCHEDULING_PROCESS_SIZE                         ((uint32_t)122368)

/*ROUTING_PROCESS Size requirement*/
#define BDSP_ARM_IMG_ROUTING_PROCESS_SIZE                            ((uint32_t)66560)

/*MESSAGING_PROCESS Size requirement*/
#define BDSP_ARM_IMG_MESSAGING_PROCESS_SIZE                          ((uint32_t)96768)

/*ALGO_PROCESS Size requirement*/
#define BDSP_ARM_IMG_ALGO_PROCESS_SIZE                               ((uint32_t)95744)

/*IDLE_PROCESS Size requirement*/
#define BDSP_ARM_IMG_IDLE_PROCESS_SIZE                               ((uint32_t)11264)

/*MM_SERVER Size requirement*/
#define BDSP_ARM_IMG_MM_SERVER_SIZE                                  ((uint32_t)150528)

/*AIDS_WAVFORMATEX Size requirement*/
#define BDSP_ARM_IMG_AIDS_WAVFORMATEX_SIZE                           ((uint32_t)24576)

/*AIDS_MPEG1 Size requirement*/
#define BDSP_ARM_IMG_AIDS_MPEG1_SIZE                                 ((uint32_t)27648)

/*ADEC_PCMWAV Size requirement*/
#define BDSP_ARM_IMG_ADEC_PCMWAV_SCRATCH_SIZE                        ((uint32_t)32768)
#define BDSP_ARM_IMG_ADEC_PCMWAV_SIZE                                ((uint32_t)34304)
#define BDSP_ARM_IMG_ADEC_PCMWAV_INTER_FRAME_SIZE                    ((uint32_t)1536)
#define BDSP_ARM_IMG_ADEC_PCMWAV_INTER_FRAME_ENCODED_SIZE            ((uint32_t)512)
#define BDSP_ARM_IMG_ADEC_PCMWAV_TABLES_SIZE                         ((uint32_t)1536)

/*ADEC_MPEG1 Size requirement*/
#define BDSP_ARM_IMG_ADEC_MPEG1_SCRATCH_SIZE                         ((uint32_t)36864)
#define BDSP_ARM_IMG_ADEC_MPEG1_SIZE                                 ((uint32_t)130048)
#define BDSP_ARM_IMG_ADEC_MPEG1_INTER_FRAME_SIZE                     ((uint32_t)10752)
#define BDSP_ARM_IMG_ADEC_MPEG1_INTER_FRAME_ENCODED_SIZE             ((uint32_t)512)
#define BDSP_ARM_IMG_ADEC_MPEG1_TABLES_SIZE                          ((uint32_t)15360)

/*APP_SRC Size requirement*/
#define BDSP_ARM_IMG_APP_SRC_SCRATCH_SIZE                            ((uint32_t)45056)
#define BDSP_ARM_IMG_APP_SRC_SIZE                                    ((uint32_t)31232)
#define BDSP_ARM_IMG_APP_SRC_INTER_FRAME_SIZE                        ((uint32_t)14336)
#define BDSP_ARM_IMG_APP_SRC_INTER_FRAME_ENCODED_SIZE                ((uint32_t)512)
#define BDSP_ARM_IMG_APP_SRC_TABLES_SIZE                             ((uint32_t)4096)