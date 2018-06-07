/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bdsp_raaga_fw.h"
#include "bdsp_raaga_img_sizes.h"
#include "bdsp_raaga_version.h"
#include "bdsp_raaga_fw_settings.h"
#include "bdsp_raaga_fw_algo.h"
#include "bdsp.h"

BDBG_MODULE(bdsp_raaga_fw_algo);

#define BDSP_AF_P_EXTRA_SAMPLES (8) /* used to put some extra buffer */

const uint32_t BDSP_SystemID_MemoryReqd[BDSP_SystemImgId_eMax] = {
    BDSP_IMG_SYSTEM_CODE_SIZE, /*BDSP_SystemImgId_eSystemCode*/
    /*BDSP_IMG_SYSTEM_DATA_SIZE,*/ /*BDSP_SystemImgId_eSystemData*/
    BDSP_IMG_SYSTEM_RDBVARS_SIZE, /*BDSP_SystemImgId_eSystemRdbvars*/
    BDSP_IMG_SYSLIB_CODE_SIZE, /*BDSP_SystemImgId_eSyslibCode*/
    BDSP_IMG_ALGOLIB_CODE_SIZE, /*BDSP_SystemImgId_eAlgolibCode*/
    BDSP_IMG_IDSCOMMON_CODE_SIZE, /*BDSP_SystemImgId_eCommonIdsCode*/
    BDSP_IMG_VIDIDSCOMMON_CODE_SIZE, /*BDSP_SystemImgId_eCommonVideoEncodeIdsCode*/
    BDSP_IMG_VIDIDSCOMMON_INTER_FRAME_SIZE, /*BDSP_SystemImgId_eCommonVideoEncodeIdsInterframe*/
    BDSP_IMG_SCM_TASK_CODE_SIZE, /*BDSP_SystemImgId_eScm_Task_Code*/
    BDSP_IMG_VIDEO_DECODE_TASK_CODE_SIZE, /*BDSP_SystemImgId_eVideo_Decode_Task_Code*/
    BDSP_IMG_VIDEO_ENCODE_TASK_CODE_SIZE, /*BDSP_SystemImgId_eVideo_Encode_Task_Code*/
    BDSP_IMG_SYSTEM_SCM1_DIGEST_SIZE, /*BDSP_SystemImgId_eScm1_Digest*/
    BDSP_IMG_SYSTEM_SCM2_DIGEST_SIZE, /*BDSP_SystemImgId_eScm2_Digest*/
};

const BDSP_AF_P_sNODE_INFO BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMax] =
{
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMpegDecode] =  */
    {
        BDSP_IMG_MPEG1_DECODE_SIZE,                     /*  ui32CodeSize */
        BDSP_IMG_MPEG1_DECODE_TABLES_SIZE,              /*  ui32RomTableSize */
        BDSP_IMG_MPEG1_DECODE_INTER_FRAME_SIZE,         /*  ui32InterFrameBuffSize */
        (1152+BDSP_AF_P_EXTRA_SAMPLES)*4*2,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_MpegConfigParams),      /*  ui32UserCfgBuffSize */
        (1152+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eStatusBuffType */
        sizeof(BDSP_Raaga_Audio_MpegStreamInfo)         /*  FwStatusBuffSize */

        /*BDSP_AF_P_NodeStatusBuffType_ePresent */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAc3Decode] =  */
    {
    #ifdef BDSP_DOLBY_DCV_SUPPORT

        #ifdef BDSP_MS10_SUPPORT
            BDSP_IMG_DOLBY_MS_DDP_DECODE_SIZE,              /*  ui32CodeSize */
            BDSP_IMG_DOLBY_MS_DDP_DECODE_TABLES_SIZE,       /*  ui32RomTableSize */
            /* Interframe size with Dolby MS 10 Changes */
            BDSP_IMG_DOLBY_MS_DDP_DECODE_INTER_FRAME_SIZE,  /*  ui32InterFrameBuffSize */
        #else
            BDSP_IMG_AC3_DECODE_SIZE,                       /*  ui32CodeSize */
            BDSP_IMG_AC3_DECODE_TABLES_SIZE,                /*  ui32RomTableSize */
            /* Interframe size with Dolby MS 10 Changes */
            BDSP_IMG_AC3_DECODE_INTER_FRAME_SIZE,           /*  ui32InterFrameBuffSize */
        #endif
            (2560+BDSP_AF_P_EXTRA_SAMPLES)*4*8,             /*  ui32InterStageIoBuffSize */
            20000,                                          /*  ui32InterStageGenericBuffSize */
            82600,                                          /*  ui32ScratchBuffSize */
            sizeof(BDSP_Raaga_Audio_DDPMultiStreamConfigParams),    /*  ui32UserCfgBuffSize                   */
            (2560+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
            8,                                              /*  ui32MaxNumChansSupported */
            BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
            BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
            sizeof(BDSP_Raaga_Audio_MultiStreamDDPStreamInfo)   /*  FwStatusBuffSize */
    #else

        #ifdef BDSP_MS10_SUPPORT
            BDSP_IMG_DOLBY_MS_DDP_DECODE_SIZE,              /*  ui32CodeSize */
            BDSP_IMG_DOLBY_MS_DDP_DECODE_TABLES_SIZE,       /*  ui32RomTableSize */
            /* Interframe size with Dolby MS 10 Changes */
            BDSP_IMG_DOLBY_MS_DDP_DECODE_INTER_FRAME_SIZE,  /*  ui32InterFrameBuffSize */
            (2560+BDSP_AF_P_EXTRA_SAMPLES)*4*8,             /*  ui32InterStageIoBuffSize */
            20000,                                          /*  ui32InterStageGenericBuffSize */
            82600,                                          /*  ui32ScratchBuffSize */
            sizeof(BDSP_Raaga_Audio_DDPMultiStreamConfigParams),    /*  ui32UserCfgBuffSize                   */
            (2560+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
            8,                                              /*  ui32MaxNumChansSupported */
            BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
            BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
            sizeof(BDSP_Raaga_Audio_MultiStreamDDPStreamInfo)   /*  FwStatusBuffSize */
        #else
            BDSP_IMG_AC3_DECODE_SIZE,                       /*  ui32CodeSize */
            BDSP_IMG_AC3_DECODE_TABLES_SIZE,                /*  ui32RomTableSize */
            /* Interframe size with Dolby MS 10 Changes */
            BDSP_IMG_AC3_DECODE_INTER_FRAME_SIZE,           /*  ui32InterFrameBuffSize */
            (2560+BDSP_AF_P_EXTRA_SAMPLES)*4*8,             /*  ui32InterStageIoBuffSize */
            20000,                                          /*  ui32InterStageGenericBuffSize */
            462144,                                          /*  ui32ScratchBuffSize */
            sizeof(BDSP_Raaga_Audio_UdcdecConfigParams),    /*  ui32UserCfgBuffSize                   */
            (2560+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
            8,                                              /*  ui32MaxNumChansSupported */
            BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
            BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
            sizeof(BDSP_Raaga_Audio_UdcStreamInfo)          /*  FwStatusBuffSize */
        #endif
    #endif
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAacDecode] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAacHeLpSbrDecode] =  */
    {
        BDSP_IMG_AACHE_DECODE_SIZE,                     /*  ui32CodeSize */
        BDSP_IMG_AACHE_DECODE_TABLES_SIZE,              /*  ui32RomTableSize */
        BDSP_IMG_AACHE_DECODE_INTER_FRAME_SIZE,         /*  ui32InterFrameBuffSize */
        (12300+BDSP_AF_P_EXTRA_SAMPLES)*4*6,            /*  ui32InterStageIoBuffSize */
        41984,                                          /*  ui32InterStageGenericBuffSize */
        81920,                                          /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_AacheConfigParams),     /*  ui32UserCfgBuffSize      */
        (12300+BDSP_AF_P_EXTRA_SAMPLES)*4,              /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_AacStreamInfo)          /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDdpDecode] =  */
    {
    #ifdef BDSP_DOLBY_DCV_SUPPORT

        #ifdef BDSP_MS10_SUPPORT
            BDSP_IMG_DOLBY_MS_DDP_DECODE_SIZE,              /*  ui32CodeSize */
            BDSP_IMG_DOLBY_MS_DDP_DECODE_TABLES_SIZE,       /*  ui32RomTableSize */
            /* Interframe size with Dolby MS 10 Changes */
            BDSP_IMG_DOLBY_MS_DDP_DECODE_INTER_FRAME_SIZE,  /*  ui32InterFrameBuffSize */
        #else
            BDSP_IMG_DDP_DECODE_SIZE,                       /*  ui32CodeSize */
            BDSP_IMG_DDP_DECODE_TABLES_SIZE,                /*  ui32RomTableSize */
            /* Interframe size with Dolby MS 10 Changes */
            BDSP_IMG_DDP_DECODE_INTER_FRAME_SIZE,           /*  ui32InterFrameBuffSize */
        #endif
            (2560+BDSP_AF_P_EXTRA_SAMPLES)*4*8,                                     /*  ui32InterStageIoBuffSize */
            20000,                                          /*  ui32InterStageGenericBuffSize */
            82600,                                          /*  ui32ScratchBuffSize */
            sizeof(BDSP_Raaga_Audio_DDPMultiStreamConfigParams),    /*  ui32UserCfgBuffSize  */
            (2560+BDSP_AF_P_EXTRA_SAMPLES)*4,                                           /*  ui32MaxSizePerChan */
            8,                                              /*  ui32MaxNumChansSupported */
            BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
            BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
            sizeof(BDSP_Raaga_Audio_MultiStreamDDPStreamInfo)   /*  FwStatusBuffSize */
    #else

        #ifdef BDSP_MS10_SUPPORT
            BDSP_IMG_DOLBY_MS_DDP_DECODE_SIZE,              /*  ui32CodeSize */
            BDSP_IMG_DOLBY_MS_DDP_DECODE_TABLES_SIZE,       /*  ui32RomTableSize */
            /* Interframe size with Dolby MS 10 Changes */
            BDSP_IMG_DOLBY_MS_DDP_DECODE_INTER_FRAME_SIZE,  /*  ui32InterFrameBuffSize */
            (2560+BDSP_AF_P_EXTRA_SAMPLES)*4*8,                                     /*  ui32InterStageIoBuffSize */
            20000,                                          /*  ui32InterStageGenericBuffSize */
            82600,                                          /*  ui32ScratchBuffSize */
            sizeof(BDSP_Raaga_Audio_DDPMultiStreamConfigParams),    /*  ui32UserCfgBuffSize  */
            (2560+BDSP_AF_P_EXTRA_SAMPLES)*4,                                           /*  ui32MaxSizePerChan */
            8,                                              /*  ui32MaxNumChansSupported */
            BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
            BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
            sizeof(BDSP_Raaga_Audio_MultiStreamDDPStreamInfo)   /*  FwStatusBuffSize */
        #else
            BDSP_IMG_DDP_DECODE_SIZE,                       /*  ui32CodeSize */
            BDSP_IMG_DDP_DECODE_TABLES_SIZE,                /*  ui32RomTableSize */
            /* Interframe size with Dolby MS 10 Changes */
            BDSP_IMG_DDP_DECODE_INTER_FRAME_SIZE,           /*  ui32InterFrameBuffSize */
            (2560+BDSP_AF_P_EXTRA_SAMPLES)*4*8,                                     /*  ui32InterStageIoBuffSize */
            20000,                                          /*  ui32InterStageGenericBuffSize */
            462144,                                          /*  ui32ScratchBuffSize */
            sizeof(BDSP_Raaga_Audio_UdcdecConfigParams),    /*  ui32UserCfgBuffSize  */
            (2560+BDSP_AF_P_EXTRA_SAMPLES)*4,                                           /*  ui32MaxSizePerChan */
            8,                                              /*  ui32MaxNumChansSupported */
            BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
            BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
            sizeof(BDSP_Raaga_Audio_UdcStreamInfo)   /*  FwStatusBuffSize */
        #endif
    #endif
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDdLosslessDecode] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eLpcmCustomDecode] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eBdLpcmDecode] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDvdLpcmDecode] =  */
    {
        BDSP_IMG_LPCM_DECODE_CODE_SIZE,                 /*  ui32CodeSize */
        BDSP_IMG_LPCM_DECODE_TABLES_SIZE,               /*  ui32RomTableSize */
        BDSP_IMG_LPCM_DECODE_INTER_FRAME_SIZE,          /*  ui32InterFrameBuffSize */
        (5000+BDSP_AF_P_EXTRA_SAMPLES)*4*8,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        100,                                            /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_LpcmUserConfig),        /*  ui32UserCfgBuffSize      */
        (5000+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        8,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_LpcmStreamInfo)         /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eHdDvdLpcmDecode] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMpegMcDecode] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eWmaStdDecode] =  */
    {
        BDSP_IMG_WMA_DECODE_SIZE,                       /*  ui32CodeSize */
        BDSP_IMG_WMA_DECODE_TABLES_SIZE,                /*  ui32RomTableSize */
        BDSP_IMG_WMA_DECODE_INTER_FRAME_SIZE,           /*  ui32InterFrameBuffSize */
        (4096+BDSP_AF_P_EXTRA_SAMPLES)*4*2,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        7000,                                           /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_WmaConfigParams),       /*  ui32UserCfgBuffSize      */
        (4096+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_WmaStreamInfo)          /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eWmaProStdDecode] =  */
     {
        BDSP_IMG_WMA_PRO_DECODE_SIZE,                   /*  ui32CodeSize */
        BDSP_IMG_WMA_PRO_DECODE_TABLES_SIZE,            /*  ui32RomTableSize */
        BDSP_IMG_WMA_PRO_DECODE_INTER_FRAME_SIZE,       /*  ui32InterFrameBuffSize */
        (4500+BDSP_AF_P_EXTRA_SAMPLES)*4*6,                /*  ui32InterStageIoBuffSize */
        160000*2,                                          /*  ui32InterStageGenericBuffSize */
        250000,                                          /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_WmaProConfigParams),    /*  ui32UserCfgBuffSize */
        (4500+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_WmaProStreamInfo)       /*  FwStatusBuffSize */
     },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMlpDecode] =  */
    {
        BDSP_IMG_TRUEHD_DECODE_SIZE,                        /*  ui32CodeSize */
        BDSP_IMG_TRUEHD_DECODE_TABLES_SIZE,             /*  ui32RomTableSize */
        BDSP_IMG_TRUEHD_DECODE_INTER_FRAME_SIZE,            /*  ui32InterFrameBuffSize */
        (4500+BDSP_AF_P_EXTRA_SAMPLES)*4*6,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        100,                                            /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_MlpConfigParams),       /*  ui32UserCfgBuffSize */
        (4500+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_MlpStreamInfo)      /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDdp71Decode] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDtsDecode] =  */
    {
        BDSP_IMG_DTSHD_DECODE_SIZE,                     /*  ui32CodeSize */
        BDSP_IMG_DTSHD_DECODE_TABLES_SIZE,              /*  ui32RomTableSize */
        BDSP_IMG_DTSHD_DECODE_INTER_FRAME_SIZE,         /*  ui32InterFrameBuffSize */
        (4500+BDSP_AF_P_EXTRA_SAMPLES)*4*6,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        100,                                            /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_DtsHdConfigParams),     /*  ui32UserCfgBuffSize */
        (4500+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_DtsHdStreamInfo)        /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDtsLbrDecode] =  */
    {
        BDSP_IMG_DTS_EXPRESS_DECODE_SIZE,                              /*      ui32CodeSize */
        BDSP_IMG_DTS_EXPRESS_DECODE_TABLES_SIZE,                       /*      ui32RomTableSize */
        BDSP_IMG_DTS_EXPRESS_DECODE_INTER_FRAME_SIZE,                  /*      ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*4*8,                      /*      ui32InterStageIoBuffSize */
        54000,                                                   /*      ui32InterStageGenericBuffSize */
        716,                                                     /*      ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_DtslbrConfigParams),             /*      ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*4,                        /*      ui32MaxSizePerChan */
        8,                                                       /*      ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,                   /*      eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                             /*      eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_DtslbrStreamInfo)                /*      FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDtsHdDecode] =  */
    {
        BDSP_IMG_DTSHD_DECODE_SIZE,                     /*  ui32CodeSize */
        BDSP_IMG_DTSHD_DECODE_TABLES_SIZE,              /*  ui32RomTableSize */
        BDSP_IMG_DTSHD_DECODE_INTER_FRAME_SIZE,         /*  ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*4*6,             /*  ui32InterStageIoBuffSize */
        54000,                                          /*  ui32InterStageGenericBuffSize */
        174000  ,                       /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_DtsHdConfigParams),     /*  ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_DtsHdStreamInfo)        /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_ePcmWavDecode] =  */
    {
        BDSP_IMG_PCMWAV_DECODE_SIZE,                    /*  ui32CodeSize */
        BDSP_IMG_PCMWAV_DECODE_TABLES_SIZE,             /*  ui32RomTableSize */
        BDSP_IMG_PCMWAV_DECODE_INTER_FRAME_SIZE,        /*  ui32InterFrameBuffSize */
        (4096+BDSP_AF_P_EXTRA_SAMPLES)*4*8,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        1024,                                           /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_PcmWavConfigParams),    /*  ui32UserCfgBuffSize */
        (4096+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_PcmWavStreamInfo)       /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAmrDecode] =  */
    {
        BDSP_IMG_AMR_DECODE_SIZE,                       /*  ui32CodeSize */
        BDSP_IMG_AMR_DECODE_TABLES_SIZE,                /*  ui32RomTableSize */
        BDSP_IMG_AMR_DECODE_INTER_FRAME_SIZE,           /*  ui32InterFrameBuffSize */
        (160+BDSP_AF_P_EXTRA_SAMPLES)*4*6,              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_AmrConfigParams),       /*  ui32UserCfgBuffSize */
        (160+BDSP_AF_P_EXTRA_SAMPLES)*4,                /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_AmrStreamInfo)          /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDraDecode] =  */
    {
        BDSP_IMG_DRA_DECODE_SIZE,                       /*  ui32CodeSize */
        BDSP_IMG_DRA_DECODE_TABLES_SIZE,                /*  ui32RomTableSize */
        BDSP_IMG_DRA_DECODE_INTER_FRAME_SIZE,           /*  ui32InterFrameBuffSize */
        (1024+BDSP_AF_P_EXTRA_SAMPLES)*4*6,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        38912,                                          /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_DraConfigParams),       /*  ui32UserCfgBuffSize */
        (1024+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_DraStreamInfo)          /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eRealAudioLbrDecode] =  */
    {
        BDSP_IMG_RALBR_DECODE_SIZE,                     /*  ui32CodeSize */
        BDSP_IMG_RALBR_DECODE_TABLES_SIZE,              /*  ui32RomTableSize */
        BDSP_IMG_RALBR_DECODE_INTER_FRAME_SIZE,         /*  ui32InterFrameBuffSize */
        (1024+BDSP_AF_P_EXTRA_SAMPLES)*4*6,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        100,                                            /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_RalbrConfigParams),     /*  ui32UserCfgBuffSize */
        (1024+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_RalbrStreamInfo)        /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDolbyPulseDecode] =  */
    {
        BDSP_IMG_DOLBY_PULSE_DECODE_SIZE,               /*  ui32CodeSize */
        BDSP_IMG_DOLBY_PULSE_DECODE_TABLES_SIZE,        /*  ui32RomTableSize */
        BDSP_IMG_DOLBY_PULSE_DECODE_INTER_FRAME_SIZE,   /*  ui32InterFrameBuffSize */
        (3072+BDSP_AF_P_EXTRA_SAMPLES)*4*8,             /*  ui32InterStageIoBuffSize */
        80240,                                          /*  ui32InterStageGenericBuffSize */
        185088,                                         /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_DolbyPulseUserConfig),  /*  ui32UserCfgBuffSize */
        (3072+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        8,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_DolbyPulseStreamInfo)   /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMs10DdpDecode] =  */
    {
        BDSP_IMG_DOLBY_MS_DDP_DECODE_SIZE,              /*  ui32CodeSize */
        BDSP_IMG_DOLBY_MS_DDP_DECODE_TABLES_SIZE,       /*  ui32RomTableSize */
        BDSP_IMG_DOLBY_MS_DDP_DECODE_INTER_FRAME_SIZE,  /*  ui32InterFrameBuffSize */
        (2560+BDSP_AF_P_EXTRA_SAMPLES)*4*8,             /*  ui32InterStageIoBuffSize */
        87000,                                          /*  ui32InterStageGenericBuffSize */
        6584,                                           /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_DDPMultiStreamConfigParams),/*  ui32UserCfgBuffSize */
        (2560+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        8,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_MultiStreamDDPStreamInfo)   /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAdpcmDecode] =  */
    {
        BDSP_IMG_ADPCM_DECODE_SIZE,                     /*  ui32CodeSize */
        BDSP_IMG_ADPCM_DECODE_TABLES_SIZE,              /*  ui32RomTableSize */
        BDSP_IMG_ADPCM_DECODE_INTER_FRAME_SIZE,         /*  ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*4*2,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_AdpcmConfigParams),     /*  ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_AdpcmStreamInfo)        /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eG711G726Decode] =  */
    {
        BDSP_IMG_G711_G726_DECODE_SIZE,                 /*  ui32CodeSize */
        BDSP_IMG_G711_G726_DECODE_TABLES_SIZE,          /*  ui32RomTableSize */
        BDSP_IMG_G711_G726_DECODE_INTER_FRAME_SIZE,     /*  ui32InterFrameBuffSize */
        (568+BDSP_AF_P_EXTRA_SAMPLES)*4*2,              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_G726ConfigParams),      /*  ui32UserCfgBuffSize */
        (568+BDSP_AF_P_EXTRA_SAMPLES)*4,                /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_G726StreamInfo)         /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eG729Decode] =  */
    {
        BDSP_IMG_G729_DECODE_SIZE,                      /*  ui32CodeSize */
        BDSP_IMG_G729_DECODE_TABLES_SIZE,               /*  ui32RomTableSize */
        BDSP_IMG_G729_DECODE_INTER_FRAME_SIZE,          /*  ui32InterFrameBuffSize */
        (256+BDSP_AF_P_EXTRA_SAMPLES)*4*2,              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        12604,                                          /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_G729DecConfigParams),   /*  ui32UserCfgBuffSize */
        (256+BDSP_AF_P_EXTRA_SAMPLES)*4,                /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_G729DecStreamInfo)      /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eVorbisDecode] =  */
    {
        BDSP_IMG_VORBIS_DECODE_SIZE,                    /*  ui32CodeSize */
        BDSP_IMG_VORBIS_DECODE_TABLES_SIZE,             /*  ui32RomTableSize */
        BDSP_IMG_VORBIS_DECODE_INTER_FRAME_SIZE,            /*  ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*4*8,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        73728,                                          /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_VorbisDecConfigParams), /*  ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        8,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_VorbisDecStreamInfo)        /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eG723_1Decode] =  */
    {
        BDSP_IMG_G723_1_DECODE_SIZE,                    /*  ui32CodeSize */
        BDSP_IMG_G723_1_DECODE_TABLES_SIZE,             /*  ui32RomTableSize */
        BDSP_IMG_G723_1_DECODE_INTER_FRAME_SIZE,        /*  ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*4*8,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        73728,                                          /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_G723_1DEC_ConfigParams),    /*  ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        8,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_G723_1_StreamInfo)      /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eFlacDecode] =  */
    {
        BDSP_IMG_FLAC_DECODE_SIZE,                      /*  ui32CodeSize */
        BDSP_IMG_FLAC_DECODE_TABLES_SIZE,               /*  ui32RomTableSize */
        BDSP_IMG_FLAC_DECODE_INTER_FRAME_SIZE,          /*  ui32InterFrameBuffSize */
        (18*1024+BDSP_AF_P_EXTRA_SAMPLES)*4*8,          /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        (18*1024+BDSP_AF_P_EXTRA_SAMPLES)*4*8,          /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_FlacDecConfigParams),   /*  ui32UserCfgBuffSize */
        (18*1024+BDSP_AF_P_EXTRA_SAMPLES)*4,            /*  ui32MaxSizePerChan */
        8,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_FlacDecStreamInfo)      /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMacDecode] =  */
    {
        BDSP_IMG_MAC_DECODE_SIZE,                       /*  ui32CodeSize */
        BDSP_IMG_MAC_DECODE_TABLES_SIZE,                /*  ui32RomTableSize */
        BDSP_IMG_MAC_DECODE_INTER_FRAME_SIZE,           /*  ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*4*8,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        73728,                                          /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_MacDecConfigParams),    /*  ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        8,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_MacDecStreamInfo)       /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAmrWbDecode] =  */
    {
        BDSP_IMG_AMRWB_DECODE_SIZE,                     /*  ui32CodeSize */
        BDSP_IMG_AMRWB_DECODE_TABLES_SIZE,              /*  ui32RomTableSize */
        BDSP_IMG_AMRWB_DECODE_INTER_FRAME_SIZE,         /*  ui32InterFrameBuffSize */
        (320+BDSP_AF_P_EXTRA_SAMPLES)*4*6,              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_AmrwbdecConfigParams),      /*  ui32UserCfgBuffSize */
        (320+BDSP_AF_P_EXTRA_SAMPLES)*4,                /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_AmrWbStreamInfo)            /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eiLBCDecode] =  */
    {
        BDSP_IMG_ILBC_DECODE_SIZE,                      /*  ui32CodeSize */
        BDSP_IMG_ILBC_DECODE_TABLES_SIZE,               /*  ui32RomTableSize */
        BDSP_IMG_ILBC_DECODE_INTER_FRAME_SIZE,          /*  ui32InterFrameBuffSize */
        (240+BDSP_AF_P_EXTRA_SAMPLES)*4*6,              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_iLBCdecConfigParams),       /*  ui32UserCfgBuffSize */
        (240+BDSP_AF_P_EXTRA_SAMPLES)*4,                /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_iLBCStreamInfo)         /*  FwStatusBuffSize */
    },
            /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eiSACDecode] =  */
    {
        BDSP_IMG_ISAC_DECODE_SIZE,                      /*  ui32CodeSize */
        BDSP_IMG_ISAC_DECODE_TABLES_SIZE,               /*  ui32RomTableSize */
        BDSP_IMG_ISAC_DECODE_INTER_FRAME_SIZE,          /*  ui32InterFrameBuffSize */
        (240+BDSP_AF_P_EXTRA_SAMPLES)*4*6,              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_iSACdecConfigParams),       /*  ui32UserCfgBuffSize */
        (240+BDSP_AF_P_EXTRA_SAMPLES)*4,                /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_iSACStreamInfo)         /*  FwStatusBuffSize */
    },

    {
        BDSP_IMG_UDC_DECODE_SIZE,                       /*  ui32CodeSize */
        BDSP_IMG_UDC_DECODE_TABLES_SIZE,                /*  ui32RomTableSize */
        BDSP_IMG_UDC_DECODE_INTER_FRAME_SIZE,           /*  ui32InterFrameBuffSize */
        (1536+BDSP_AF_P_EXTRA_SAMPLES)*4*8,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        462144,                                         /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_UdcdecConfigParams),    /*  ui32UserCfgBuffSize */
        (1536+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        8,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_UdcStreamInfo)          /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDolbyAacheDecode] =  */
    {
        BDSP_IMG_DOLBY_AACHE_DECODE_SIZE,               /*  ui32CodeSize */
        BDSP_IMG_DOLBY_AACHE_DECODE_TABLES_SIZE,        /*  ui32RomTableSize */
        BDSP_IMG_DOLBY_AACHE_DECODE_INTER_FRAME_SIZE,   /*  ui32InterFrameBuffSize */
        (3072+BDSP_AF_P_EXTRA_SAMPLES)*4*8,             /*  ui32InterStageIoBuffSize */
        80240,                                          /*  ui32InterStageGenericBuffSize */
        185088,                                         /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_DolbyAacheUserConfig),  /*  ui32UserCfgBuffSize */
        (3072+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        8,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_DolbyAacheStreamInfo)   /*  FwStatusBuffSize */
    },

    {
      BDSP_IMG_OPUS_DECODE_SIZE,                        /*  ui32CodeSize */
      BDSP_IMG_OPUS_DECODE_TABLES_SIZE,             /*  ui32RomTableSize */
      BDSP_IMG_OPUS_DECODE_INTER_FRAME_SIZE,            /*  ui32InterFrameBuffSize */
      (5760+BDSP_AF_P_EXTRA_SAMPLES)*4*8,               /*  ui32InterStageIoBuffSize */
      20000,                                            /*  ui32InterStageGenericBuffSize */
      40000,                                            /*  ui32ScratchBuffSize */
      sizeof(BDSP_Raaga_Audio_OpusDecConfigParams), /*  ui32UserCfgBuffSize */
      (5760+BDSP_AF_P_EXTRA_SAMPLES)*4,             /*  ui32MaxSizePerChan */
      8,                                                /*  ui32MaxNumChansSupported */
      BDSP_AF_P_InterFrameBuffType_ePresent,            /*  eInterFrameBuffType */
      BDSP_AF_P_FwStatus_ePresent,                  /*  eFwStatusBuffType */
      sizeof(BDSP_Raaga_Audio_OpusDecStreamInfo)        /*  FwStatusBuffSize */
    },

    {
      BDSP_IMG_ALS_DECODE_SIZE,                     /*  ui32CodeSize */
      BDSP_IMG_ALS_DECODE_TABLES_SIZE,              /*  ui32RomTableSize */
      BDSP_IMG_ALS_DECODE_INTER_FRAME_SIZE,         /*  ui32InterFrameBuffSize */
      (4096+BDSP_AF_P_EXTRA_SAMPLES)*4*6,               /*  ui32InterStageIoBuffSize */
      20000,                                            /*  ui32InterStageGenericBuffSize */
      40000,                                            /*  ui32ScratchBuffSize */
      sizeof(BDSP_Raaga_Audio_ALSDecConfigParams),  /*  ui32UserCfgBuffSize */
      (4096+BDSP_AF_P_EXTRA_SAMPLES)*4,             /*  ui32MaxSizePerChan */
      6,                                                /*  ui32MaxNumChansSupported */
      BDSP_AF_P_InterFrameBuffType_ePresent,            /*  eInterFrameBuffType */
      BDSP_AF_P_FwStatus_ePresent,                  /*  eFwStatusBuffType */
      sizeof(BDSP_Raaga_Audio_ALSDecStreamInfo)         /*  FwStatusBuffSize */
    },

    {
      BDSP_IMG_AC4_DECODE_SIZE,                     /*  ui32CodeSize */
      BDSP_IMG_AC4_DECODE_TABLES_SIZE,              /*  ui32RomTableSize */
      BDSP_IMG_AC4_DECODE_INTER_FRAME_SIZE,         /*  ui32InterFrameBuffSize */
      (4096+BDSP_AF_P_EXTRA_SAMPLES)*4*6,               /*  ui32InterStageIoBuffSize */
      20000,                                            /*  ui32InterStageGenericBuffSize */
      8388608,                                      /*  ui32ScratchBuffSize */
      sizeof(BDSP_Raaga_Audio_AC4DecConfigParams),  /*  ui32UserCfgBuffSize */
      (4096+BDSP_AF_P_EXTRA_SAMPLES)*4,             /*  ui32MaxSizePerChan */
      6,                                                /*  ui32MaxNumChansSupported */
      BDSP_AF_P_InterFrameBuffType_ePresent,            /*  eInterFrameBuffType */
      BDSP_AF_P_FwStatus_ePresent,                  /*  eFwStatusBuffType */
      sizeof(BDSP_Raaga_Audio_AC4StreamInfo)        /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eEndOfAudioDecodeAlgos] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eRealVideo9Decode] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eVP6Decode] =  */
    {
        BDSP_IMG_VP6_DECODE_SIZE,                       /*  ui32CodeSize */
        BDSP_IMG_VP6_DECODE_TABLES_SIZE,                /*  ui32RomTableSize */
        BDSP_IMG_VP6_DECODE_INTER_FRAME_SIZE,           /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000,                                              /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        0,                                              /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        0,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_P_Vp6StreamInfo)              /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eEndOfDecodeAlgos] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  Algo inits for the frame syncs of the decoder algorithms */
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMpegFrameSync] =  */
    {
        BDSP_IMG_MPEG1_IDS_SIZE,                        /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_MPEG1_IDS_INTER_FRAME_SIZE,            /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMpegMcFrameSync] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAdtsFrameSync] =  */
    {
        BDSP_IMG_AACHEADTS_IDS_SIZE,                    /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_AACHEADTS_IDS_INTER_FRAME_SIZE,        /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eLoasFrameSync] =  */
    {
        BDSP_IMG_AACHELOAS_IDS_SIZE,                    /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_AACHELOAS_IDS_INTER_FRAME_SIZE,        /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eWmaStdFrameSync] =  */
    {
        BDSP_IMG_WMA_IDS_SIZE,                          /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_WMA_IDS_INTER_FRAME_SIZE,              /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eWmaProFrameSync] =  */
    {
        BDSP_IMG_WMA_PRO_IDS_SIZE,                          /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_WMA_PRO_IDS_INTER_FRAME_SIZE,              /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAc3FrameSync] =  */
    {
        BDSP_IMG_AC3_IDS_SIZE,                          /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_AC3_IDS_INTER_FRAME_SIZE,              /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDdpFrameSync] =  */
    {
        BDSP_IMG_DDP_IDS_SIZE,                          /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_DDP_IDS_INTER_FRAME_SIZE,              /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDdp71FrameSync] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDtsFrameSync] =  */
    {
        BDSP_IMG_DTS_IDS_SIZE,                          /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_DTS_IDS_INTER_FRAME_SIZE,              /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        0,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDtsLbrFrameSync] =  */
    {
          BDSP_IMG_DTS_EXPRESS_IDS_DECODE_SIZE,             /*  ui32CodeSize */
          0,                                            /*  ui32RomTableSize */
          BDSP_IMG_DTS_EXPRESS_IDS_INTER_FRAME_SIZE,    /*  ui32InterFrameBuffSize */
          0,                                            /*  ui32InterStageIoBuffSize */
          20000,                                        /*  ui32InterStageGenericBuffSize */
          0,                                            /*  ui32ScratchBuffSize */
          sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),    /*  ui32UserCfgBuffSize */
          0,                                            /*  ui32MaxSizePerChan */
          0,                                            /*  ui32MaxNumChansSupported */
          BDSP_AF_P_InterFrameBuffType_ePresent,            /*  eInterFrameBuffType */
          BDSP_AF_P_FwStatus_ePresent,                  /*  eFwStatusBuffType */
          sizeof(BDSP_Raaga_Audio_IdsTsmInfo)           /*  FwStatusBuffSize */
     },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDtsHdFrameSync] =  */
    {
        BDSP_IMG_DTSHD_IDS_SIZE,                        /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_DTSHD_IDS_INTER_FRAME_SIZE,            /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        0,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDtsHdFrameSync_1] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDtsHdHdDvdFrameSync] =  */
        {
            BDSP_IMG_DTSHD_IDS_SIZE,                        /*  ui32CodeSize */
            0,                                              /*  ui32RomTableSize */
            BDSP_IMG_DTSHD_IDS_INTER_FRAME_SIZE ,           /*  ui32InterFrameBuffSize */
            0,                                              /*  ui32InterStageIoBuffSize */
            20000 ,                                         /*  ui32InterStageGenericBuffSize */
            0,                                              /*  ui32ScratchBuffSize */
            sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
            0,                                              /*  ui32MaxSizePerChan */
            0,                                              /*  ui32MaxNumChansSupported */
            BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
            BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
            sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
        },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDdLosslessFrameSync] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMlpFrameSync] =  */
        {
            BDSP_IMG_MLP_IDS_SIZE,                      /*  ui32CodeSize */
            0,                                              /*  ui32RomTableSize */
            BDSP_IMG_MLP_IDS_INTER_FRAME_SIZE ,         /*  ui32InterFrameBuffSize */
            0,                                              /*  ui32InterStageIoBuffSize */
            20000 ,                                         /*  ui32InterStageGenericBuffSize */
            0,                                              /*  ui32ScratchBuffSize */
            sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
            0,                                              /*  ui32MaxSizePerChan */
            0,                                              /*  ui32MaxNumChansSupported */
            BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
            BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
            sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
        },


    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMlpHdDvdFrameSync] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_ePesFrameSync] =  */
    {
        BDSP_IMG_PCM_IDS_SIZE,                          /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_PCM_IDS_INTER_FRAME_SIZE,              /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        0,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eBdLpcmFrameSync] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eHdDvdLpcmFrameSync] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDvdLpcmFrameSync] =  */
    {
        BDSP_IMG_DVDLPCM_IDS_SIZE,                      /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_DVDLPCM_IDS_INTER_FRAME_SIZE,          /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        0,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDvdLpcmFrameSync_1] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },


    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_ePcmWavFrameSync] =  */
    {
        BDSP_IMG_PCMWAV_IDS_SIZE,                       /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_PCMWAV_IDS_INTER_FRAME_SIZE,           /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        0,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDraFrameSync] =  */
    {
        BDSP_IMG_DRA_IDS_SIZE,                          /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_DRA_IDS_INTER_FRAME_SIZE ,             /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000 ,                                         /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        0,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eRealAudioLbrFrameSync] =  */
    {
        BDSP_IMG_RALBR_IDS_SIZE,                        /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_RALBR_IDS_INTER_FRAME_SIZE ,           /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000 ,                                         /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        0,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMs10DdpFrameSync] =  */
    {
        BDSP_IMG_DDP_IDS_SIZE,                          /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_DDP_IDS_INTER_FRAME_SIZE,              /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        0,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eVorbisFrameSync] =  */
    {
        BDSP_IMG_VORBIS_IDS_SIZE,                           /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_VORBIS_IDS_INTER_FRAME_SIZE ,          /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000 ,                                         /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        0,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eFlacFrameSync] =  */
    {
        BDSP_IMG_FLAC_IDS_SIZE,                         /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_FLAC_IDS_INTER_FRAME_SIZE ,            /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000 ,                                         /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        0,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMacFrameSync] =  */
    {
        BDSP_IMG_MAC_IDS_SIZE,                          /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_MAC_IDS_INTER_FRAME_SIZE ,         /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000 ,                                         /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        0,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },
    {
        BDSP_IMG_UDC_IDS_SIZE,                          /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_UDC_IDS_INTER_FRAME_SIZE ,             /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000 ,                                         /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },
    {
        BDSP_IMG_AC4_IDS_SIZE,                          /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_AC4_IDS_INTER_FRAME_SIZE ,             /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000 ,                                         /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },
    {
        BDSP_IMG_ALS_IDS_SIZE,                          /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_ALS_IDS_INTER_FRAME_SIZE ,             /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000 ,                                         /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eEndOfAudioDecFsAlgos] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eRealVideo9FrameSync] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eVP6FrameSync] =  */
    {
        BDSP_IMG_VP6_IDS_SIZE,                          /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_VP6_IDS_INTER_FRAME_SIZE ,             /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000 ,                                         /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        0,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eEndOfDecFsAlgos] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAc3Encode] =  */
    {   44000,                                          /*  ui32CodeSize */
        12000,                                          /*  ui32RomTableSize */
        56000,                                          /*  ui32InterFrameBuffSize */
        (12000 +BDSP_AF_P_EXTRA_SAMPLES)*4*2,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        85000,                                          /*  ui32ScratchBuffSize */
        100,                                            /*  ui32UserCfgBuffSize */
        12000*4,                                        /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
     },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMpegL2Encode] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMpegL3Encode] =  */
    {
        BDSP_IMG_MP3_ENCODE_SIZE,                       /*  ui32CodeSize */
        BDSP_IMG_MP3_ENCODE_TABLES_SIZE,                /*  ui32RomTableSize */
        BDSP_IMG_MP3_ENCODE_INTER_FRAME_SIZE,           /*  ui32InterFrameBuffSize */
        (1024+BDSP_AF_P_EXTRA_SAMPLES)*4*2,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_Mpeg1L3EncConfigParams),/*  ui32UserCfgBuffSize */
        1024*4,                                         /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
     },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAacLcEncode] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAacHeEncode] =  */
    {
        BDSP_IMG_AACHE_ENCODE_SIZE,                     /*  ui32CodeSize */
        BDSP_IMG_AACHE_ENCODE_TABLES_SIZE,              /*  ui32RomTableSize */
        BDSP_IMG_AACHE_ENCODE_INTER_FRAME_SIZE,         /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*6,           /*  ui32InterStageIoBuffSize */
        16384,                                          /*  ui32InterStageGenericBuffSize */
        2000 ,                                          /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_AacheEncConfigParams),  /*  ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDtsEncode] =  */
    {
        BDSP_IMG_DTS_ENCODE_SIZE,                       /* ui32CodeSize */
        BDSP_IMG_DTS_ENCODE_TABLES_SIZE,                /* ui32RomTableSize */
        BDSP_IMG_DTS_ENCODE_INTER_FRAME_SIZE,           /* ui32InterFrameBuffSize */
        (1024+BDSP_AF_P_EXTRA_SAMPLES)*4*1,             /* ui32InterStageIoBuffSize */
        20000,                                          /* ui32InterStageGenericBuffSize */
        0,                                              /* ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_DtsBroadcastEncConfigParams),/* ui32UserCfgBuffSize */
        (1024+BDSP_AF_P_EXTRA_SAMPLES)*4,               /* ui32MaxSizePerChan */
        1,                                              /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /* eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_DTSBroadcastEncoderStreamInfo)  /* FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDtsBroadcastEncode] =  */
    {
        BDSP_IMG_DTS_ENCODE_SIZE,                       /* ui32CodeSize */
        BDSP_IMG_DTS_ENCODE_TABLES_SIZE,                /* ui32RomTableSize */
        BDSP_IMG_DTS_ENCODE_INTER_FRAME_SIZE,           /* ui32InterFrameBuffSize */
        (1024+BDSP_AF_P_EXTRA_SAMPLES)*4*1,             /* ui32InterStageIoBuffSize */
        20000,                                              /* ui32InterStageGenericBuffSize */
        0,                                              /* ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_DtsBroadcastEncConfigParams),/* ui32UserCfgBuffSize */
        (1024+BDSP_AF_P_EXTRA_SAMPLES)*4,               /* ui32MaxSizePerChan */
        1,                                              /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /* eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_DTSBroadcastEncoderStreamInfo)  /* FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eSbcEncode] =  */
    {
        10240,                                          /* ui32CodeSize */
        1024,                                           /* ui32RomTableSize */
        2048,                                           /* ui32InterFrameBuffSize */
        (37400+BDSP_AF_P_EXTRA_SAMPLES)*1,              /* ui32InterStageIoBuffSize */
        20000,                                          /* ui32InterStageGenericBuffSize */
        256,                                            /* ui32ScratchBuffSize */
        0,                                              /* ui32UserCfgBuffSize */
        (37400+BDSP_AF_P_EXTRA_SAMPLES),                /* ui32MaxSizePerChan */
        1,                                              /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /* eFwStatusBuffType */
        0                                               /* FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMs10DDTranscode] =  */
    {
        BDSP_IMG_DD_TRANSCODE_SIZE,                     /*  ui32CodeSize */
        BDSP_IMG_DD_TRANSCODE_TABLES_SIZE,              /*  ui32RomTableSize */
        BDSP_IMG_DD_TRANSCODE_INTER_FRAME_SIZE,         /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*6,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        85000 ,                                         /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_DDTranscodeConfigParams),/* ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eG711G726Encode] =  */
    {
        BDSP_IMG_G711_G726_ENCODE_SIZE,                     /* ui32CodeSize */
        BDSP_IMG_G711_G726_ENCODE_TABLES_SIZE,              /* ui32RomTableSize */
        BDSP_IMG_G711_G726_ENCODE_INTER_FRAME_SIZE,         /* ui32InterFrameBuffSize */
        (2272+BDSP_AF_P_EXTRA_SAMPLES)*1,                   /* ui32InterStageIoBuffSize */
        20000,                                              /* ui32InterStageGenericBuffSize */
        0,                                                  /* ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_G711_G726EncConfigParams),  /* ui32UserCfgBuffSize */
        (1136+BDSP_AF_P_EXTRA_SAMPLES),                     /* ui32MaxSizePerChan */
        1,                                                  /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,              /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                         /* eFwStatusBuffType */
        0                                                   /* FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eG729Encode] =  */
    {
        BDSP_IMG_G729_ENCODE_SIZE,                      /* ui32CodeSize */
        BDSP_IMG_G729_ENCODE_TABLES_SIZE,               /* ui32RomTableSize */
        BDSP_IMG_G729_ENCODE_INTER_FRAME_SIZE,          /* ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*1,               /* ui32InterStageIoBuffSize */
        20000,                                          /* ui32InterStageGenericBuffSize */
        0,                                              /* ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_G729EncoderUserConfig), /* ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES),                 /* ui32MaxSizePerChan */
        1,                                              /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /* eFwStatusBuffType */
        0                                               /* FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eG723_1Encode] =    */
    {
        BDSP_IMG_G723_1_ENCODE_SIZE,                    /* ui32CodeSize */
        BDSP_IMG_G723_1_ENCODE_TABLES_SIZE,             /* ui32RomTableSize */
        BDSP_IMG_G723_1_ENCODE_INTER_FRAME_SIZE,        /* ui32InterFrameBuffSize */
        4800,                                           /* ui32InterStageIoBuffSize */
        2000,                                           /* ui32InterStageGenericBuffSize */
        0,                                              /* ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_G723EncoderUserConfig), /* ui32UserCfgBuffSize */
        2400,                                           /* ui32MaxSizePerChan */
        2,                                              /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /* eFwStatusBuffType */
        0                                               /* FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eG722Encode] =  */
    {
        BDSP_IMG_G722_ENCODE_SIZE,                      /* ui32CodeSize */
        BDSP_IMG_G722_ENCODE_TABLES_SIZE,               /* ui32RomTableSize */
        BDSP_IMG_G722_ENCODE_INTER_FRAME_SIZE,          /* ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*1,               /* ui32InterStageIoBuffSize */
        20000,                                          /* ui32InterStageGenericBuffSize */
        0,                                              /* ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_G722EncConfigParams),   /* ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES),                 /* ui32MaxSizePerChan */
        1,                                              /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /* eFwStatusBuffType */
        0                                               /* FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAmrEncode] =  */
    {
        BDSP_IMG_AMR_ENCODE_SIZE,                       /* ui32CodeSize */
        BDSP_IMG_AMR_ENCODE_TABLES_SIZE,                /* ui32RomTableSize */
        BDSP_IMG_AMR_ENCODE_INTER_FRAME_SIZE,           /* ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*1,               /* ui32InterStageIoBuffSize */
        20000,                                          /* ui32InterStageGenericBuffSize */
        0,                                              /* ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_AmrEncoderUserConfig),  /* ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES),                 /* ui32MaxSizePerChan */
        1,                                              /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /* eFwStatusBuffType */
        0                                               /* FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAmrwbEncode] =  */
    {
        BDSP_IMG_AMRWB_ENCODE_SIZE,                     /* ui32CodeSize */
        BDSP_IMG_AMRWB_ENCODE_TABLES_SIZE,              /* ui32RomTableSize */
        BDSP_IMG_AMRWB_ENCODE_INTER_FRAME_SIZE,         /* ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*1,               /* ui32InterStageIoBuffSize */
        20000,                                          /* ui32InterStageGenericBuffSize */
        0,                                              /* ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_AMRWBConfigParams), /* ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES),                 /* ui32MaxSizePerChan */
        1,                                              /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /* eFwStatusBuffType */
        0                                               /* FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eiLBCEncode] =  */
    {
        BDSP_IMG_ILBC_ENCODE_SIZE,                      /* ui32CodeSize */
        BDSP_IMG_ILBC_ENCODE_TABLES_SIZE,               /* ui32RomTableSize */
        BDSP_IMG_ILBC_ENCODE_INTER_FRAME_SIZE,          /* ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*1,               /* ui32InterStageIoBuffSize */
        20000,                                          /* ui32InterStageGenericBuffSize */
        0,                                              /* ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_ILBCConfigParams),  /* ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES),                 /* ui32MaxSizePerChan */
        1,                                              /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /* eFwStatusBuffType */
        0                                               /* FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eiSACEncode] =  */
    {
        BDSP_IMG_ISAC_ENCODE_SIZE,                      /* ui32CodeSize */
        BDSP_IMG_ISAC_ENCODE_TABLES_SIZE,               /* ui32RomTableSize */
        BDSP_IMG_ISAC_ENCODE_INTER_FRAME_SIZE,          /* ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*1,               /* ui32InterStageIoBuffSize */
        20000,                                          /* ui32InterStageGenericBuffSize */
        0,                                              /* ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_ISACConfigParams),  /* ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES),                 /* ui32MaxSizePerChan */
        1,                                              /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /* eFwStatusBuffType */
        0                                               /* FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eLpcmEncode] =  */
    {
        BDSP_IMG_LPCM_ENCODE_CODE_SIZE,                     /* ui32CodeSize */
        BDSP_IMG_LPCM_ENCODE_TABLES_SIZE,               /* ui32RomTableSize */
        BDSP_IMG_LPCM_ENCODE_INTER_FRAME_SIZE,          /* ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*1,               /* ui32InterStageIoBuffSize */
        20000,                                          /* ui32InterStageGenericBuffSize */
        0,                                              /* ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_LpcmEncConfigParams),   /* ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES),                 /* ui32MaxSizePerChan */
        1,                                              /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /* eFwStatusBuffType */
        0                                               /* FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eOpusEncode] =  */
    {
      BDSP_IMG_OPUS_ENCODE_CODE_SIZE,                   /* ui32CodeSize */
      BDSP_IMG_OPUS_ENCODE_TABLES_SIZE,                 /* ui32RomTableSize */
      BDSP_IMG_OPUS_ENCODE_INTER_FRAME_SIZE,            /* ui32InterFrameBuffSize */
      (4000+BDSP_AF_P_EXTRA_SAMPLES)*1,                 /* ui32InterStageIoBuffSize */
      20000,                                            /* ui32InterStageGenericBuffSize */
      12000,                                            /* ui32ScratchBuffSize */
      sizeof(BDSP_Raaga_Audio_OpusEncConfigParams),     /* ui32UserCfgBuffSize */
      (4000+BDSP_AF_P_EXTRA_SAMPLES),                   /* ui32MaxSizePerChan */
      1,                                                /* ui32MaxNumChansSupported*/
      BDSP_AF_P_InterFrameBuffType_ePresent,            /* eInterFrameBuffType*/
      BDSP_AF_P_FwStatus_eAbsent,                       /* eFwStatusBuffType*/
      0                                                 /* FwStatusBuffSize*/
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDDPEncode] =  */
    {
        BDSP_IMG_DDP_ENCODE_CODE_SIZE,                      /* ui32CodeSize */
        BDSP_IMG_DDP_ENCODE_TABLES_SIZE,                /* ui32RomTableSize */
        BDSP_IMG_DDP_ENCODE_INTER_FRAME_SIZE,           /* ui32InterFrameBuffSize */
        (1536+BDSP_AF_P_EXTRA_SAMPLES)*4*8,             /* ui32InterStageIoBuffSize */
        20000,                                          /* ui32InterStageGenericBuffSize */
        240000,                                         /* ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_DDPEncConfigParams),    /* ui32UserCfgBuffSize */
        (1536+BDSP_AF_P_EXTRA_SAMPLES)*4,               /* ui32MaxSizePerChan */
        8,                                              /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /* eFwStatusBuffType */
        0                                               /* FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eEndOfEncodeAlgos] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_VF_P_AlgoId_eH264Encode] =  */
    {
        BDSP_IMG_H264_ENCODE_SIZE,                      /* ui32CodeSize */
        BDSP_IMG_H264_ENCODE_TABLES_SIZE,               /* ui32RomTableSize */
        BDSP_IMG_H264_ENCODE_INTER_FRAME_SIZE,          /* ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*1,               /* ui32InterStageIoBuffSize */
        1024,                                           /* ui32InterStageGenericBuffSize */
        0,                                              /* ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_VideoBH264UserConfig), /* ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES),                 /* ui32MaxSizePerChan */
        1,                                              /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /* eFwStatusBuffType */
        sizeof(BDSP_Raaga_VideoH264EncoderInfo)         /* FwStatusBuffSize */
    },

    {
        BDSP_IMG_X264_ENCODE_SIZE,                      /* ui32CodeSize */
        BDSP_IMG_X264_ENCODE_TABLES_SIZE,               /* ui32RomTableSize */
        BDSP_IMG_X264_ENCODE_INTER_FRAME_SIZE,          /* ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*1,               /* ui32InterStageIoBuffSize */
        1024,                                           /* ui32InterStageGenericBuffSize */
        0,                                              /* ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_VideoBX264UserConfig), /* ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES),                 /* ui32MaxSizePerChan */
        1,                                              /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /* eFwStatusBuffType */
        sizeof(BDSP_Raaga_VideoX264EncoderInfo)         /* FwStatusBuffSize */
    },
    {
        BDSP_IMG_XVP8_ENCODE_SIZE,                      /* ui32CodeSize */
        BDSP_IMG_XVP8_ENCODE_TABLES_SIZE,               /* ui32RomTableSize */
        BDSP_IMG_XVP8_ENCODE_INTER_FRAME_SIZE,          /* ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*1,               /* ui32InterStageIoBuffSize */
        1024,                                           /* ui32InterStageGenericBuffSize */
        0,                                              /* ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_VideoBXVP8UserConfig),        /* ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES),                 /* ui32MaxSizePerChan */
        1,                                              /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /* eFwStatusBuffType */
        sizeof(BDSP_Raaga_VideoXVP8EncoderInfo)         /* FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eEndOfEncodeAlgos] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAc3EncFrameSync] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMpegL3EncFrameSync] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMpegL2EncFrameSync] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAacLcEncFrameSync] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAacHeEncFrameSync] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDtsEncFrameSync] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eEndOfEncFsAlgos] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_ePassThru] =  */
    {
        BDSP_IMG_CDB_PASSTHRU_CODE_SIZE,                /*  ui32CodeSize */
        BDSP_IMG_CDB_PASSTHRU_TABLES_SIZE,              /*  ui32RomTableSize */
        BDSP_IMG_CDB_PASSTHRU_INTER_FRAME_SIZE,         /*  ui32InterFrameBuffSize */
        (4096+BDSP_AF_P_EXTRA_SAMPLES)*4*6,             /*  ui32InterStageIoBuffSize */
        32768,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_PassthruConfigParams),  /*  ui32UserCfgBuffSize */
        (4096+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        400                                             /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMLPPassThru] =  */
    {
        BDSP_IMG_MLP_PASSTHROUGH_CODE_SIZE,             /*  ui32CodeSize */
        BDSP_IMG_MLP_PASSTHROUGH_TABLES_SIZE,           /*  ui32RomTableSize */
        BDSP_IMG_MLP_PASSTHROUGH_INTER_FRAME_SIZE,      /*  ui32InterFrameBuffSize */
        (4096+BDSP_AF_P_EXTRA_SAMPLES)*4*6,             /*  ui32InterStageIoBuffSize */
        32768,                                          /*  ui32InterStageGenericBuffSize */
        1024,                                           /*  ui32ScratchBuffSize */
        0,                                              /*  ui32UserCfgBuffSize */
        (4096+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eEndOfAuxAlgos] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eSrsTruSurroundPostProc] =  */
    {
        16000,                                          /*  ui32CodeSize */
        8000,                                           /*  ui32RomTableSize */
        2000,                                           /*  ui32InterFrameBuffSize */
        (4096+BDSP_AF_P_EXTRA_SAMPLES)*4*2,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        100,                                            /*  ui32UserCfgBuffSize */
        (4096+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eSrcPostProc] =  */
    {
        BDSP_IMG_SRC_CODE_SIZE,                         /*  ui32CodeSize */
        BDSP_IMG_SRC_TABLES_SIZE,                       /*  ui32RomTableSize */
        BDSP_IMG_SRC_INTER_FRAME_SIZE,                  /*  ui32InterFrameBuffSize */
        (8*1024+BDSP_AF_P_EXTRA_SAMPLES)*4*8,               /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        64*1024*4+1000,                                             /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_SRCUserConfigParams),   /*  ui32UserCfgBuffSize */
        (8*1024+BDSP_AF_P_EXTRA_SAMPLES)*4,             /*  ui32MaxSizePerChan */
        8,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDdbmPostProc] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDownmixPostProc] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eCustomSurroundPostProc] =  */
    {
        13000,                                          /*  ui32CodeSize */
        8000,                                           /*  ui32RomTableSize */
        1500,                                           /*  ui32InterFrameBuffSize */
        (4096+BDSP_AF_P_EXTRA_SAMPLES)*4*2,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        0,                                              /*  ui32UserCfgBuffSize  */
        (4096+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eCustomBassPostProc] =  */
    {
        13000,                                          /*  ui32CodeSize */
        5900 ,                                          /*  ui32RomTableSize */
        800,                                            /*  ui32InterFrameBuffSize */
        (4096+BDSP_AF_P_EXTRA_SAMPLES)*4*2,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        0,                                              /*  ui32UserCfgBuffSize  */
        (4096+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eKaraokeCapablePostProc] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eCustomVoicePostProc] =  */
    {
        BDSP_IMG_CUSTOMVOICE_CODE_SIZE,                 /*  ui32CodeSize */
        BDSP_IMG_CUSTOMVOICE_TABLES_SIZE,               /*  ui32RomTableSize */
        BDSP_IMG_CUSTOMVOICE_INTER_FRAME_SIZE,          /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_CustomVoiceConfigParams),/* ui32UserCfgBuffSize  */
        0,                                              /*  ui32MaxSizePerChan */
        0,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_CustomVoiceStatusInfo)  /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_ePeqPostProc] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAvlPostProc] =  */
    {
        BDSP_IMG_BRCM_AVL_CODE_SIZE,                    /*  ui32CodeSize */
        BDSP_IMG_BRCM_AVL_TABLES_SIZE,                  /*  ui32RomTableSize */
        BDSP_IMG_BRCM_AVL_INTER_FRAME_SIZE,             /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*8,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_AVLConfigParams),       /*  ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        BDSP_MAX_AVL_CHANNLES,                          /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_AvlPPStatusInfo)        /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_ePl2PostProc] =  */
    {
        18000,                                          /*  ui32CodeSize */
        1500,                                           /*  ui32RomTableSize */
        3000,                                           /*  ui32InterFrameBuffSize */
        (1152*4+BDSP_AF_P_EXTRA_SAMPLES)*4*6,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        0,                                               /*  ui32UserCfgBuffSize */
        (1152*4+BDSP_AF_P_EXTRA_SAMPLES)*4,             /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eXenPostProc] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eBbePostProc] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDsolaPostProc] =  */
    {
        BDSP_IMG_DSOLA_CODE_SIZE,                       /*  ui32CodeSize */
        BDSP_IMG_DSOLA_TABLES_SIZE,                     /*  ui32RomTableSize */
        BDSP_IMG_DSOLA_INTER_FRAME_SIZE,                /*  ui32InterFrameBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4*6,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        156000,                                          /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_DsolaConfigParams),     /*  ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDtsNeoPostProc] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDDConvert] =  */
    {
        14336,                                          /*  ui32CodeSize */
        4096,                                           /*  ui32RomTableSize */
        2048,                                           /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000   ,                                           /*  ui32InterStageGenericBuffSize */
        1024,                                           /*  ui32ScratchBuffSize */
        0,                                              /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        0,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAudioDescriptorFadePostProc] =  */
    {
        12000,                                          /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        0,                                              /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        0,                                              /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_AudioDescFadeConfigParams), /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        0,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_eAbsent,           /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAudioDescriptorPanPostProc] =  */
    {
        12000,                                          /*  ui32CodeSize */
        7000,                                           /*  ui32RomTableSize */
        1024,                                           /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        0,                                              /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        0,                                               /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        0,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_ePCMRouterPostProc] =  */
    {
        12000,                                          /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        0,                                              /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        0,                                              /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        0,                                              /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        0,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_eAbsent,           /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },

   /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eWMAPassThrough] =  */
    {
        6000,                                           /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        20,                                             /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        13000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        0,                                              /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        0,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eSrsTruSurroundHDPostProc] =  */
    {
        BDSP_IMG_SRS_TRUSURROUNDHD_CODE_SIZE,           /*  ui32CodeSize */
        BDSP_IMG_SRS_TRUSURROUNDHD_TABLES_SIZE,         /*  ui32RomTableSize */
        BDSP_IMG_SRS_TRUSURROUNDHD_INTER_FRAME_SIZE,    /*  ui32InterFrameBuffSize */
        (4096+BDSP_AF_P_EXTRA_SAMPLES)*4*6,             /*  ui32InterStageIoBuffSize */
        20000,                                              /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_TruSurrndHDConfigParams),/* ui32UserCfgBuffSize */
        (4096+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eSrsTruVolumePostProc] =  */
    {
        BDSP_IMG_SRS_TVOL_CODE_SIZE,                    /*  ui32CodeSize */
        BDSP_IMG_SRS_TVOL_TABLES_SIZE,                  /*  ui32RomTableSize */
        BDSP_IMG_SRS_TVOL_INTER_FRAME_SIZE,             /*  ui32InterFrameBuffSize */
        (4096+BDSP_AF_P_EXTRA_SAMPLES)*4*2,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_TruVolumeUserConfig),   /*  ui32UserCfgBuffSize */
        (4096+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDolbyVolumePostProc] =  */
    {
        40000,                                          /*  ui32CodeSize */
        200000,                                         /*  ui32RomTableSize */
        20000,                                          /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*2,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        85000,                                          /*  ui32ScratchBuffSize */
        0,                                              /*  ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eBrcm3DSurroundPostProc] =  */
    {
        BDSP_IMG_BRCM_3DSURROUND_CODE_SIZE,             /*  ui32CodeSize */
        BDSP_IMG_BRCM_3DSURROUND_TABLES_SIZE,           /*  ui32RomTableSize */
        BDSP_IMG_BRCM_3DSURROUND_INTER_FRAME_SIZE,      /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*6,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_Brcm3DSurroundConfigParams),/*  ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eFWMixerPostProc] =  */
    {
        BDSP_IMG_FW_MIXER_CODE_SIZE,                    /*  ui32CodeSize */
        BDSP_IMG_FW_MIXER_TABLES_SIZE,                  /*  ui32RomTableSize */
        BDSP_IMG_FW_MIXER_INTER_FRAME_SIZE,             /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*6,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        16384*6,                                                /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_MixerConfigParams),     /*  ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMonoDownMixPostProc] =  */
    {
        6000,                                           /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        0,                                              /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*2,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        20,                                         /*  ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMs10DDConvert] =  */
    {
        22500,                                          /*  ui32CodeSize */
        4096,                                           /*  ui32RomTableSize */
        2048,                                           /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*2,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        20,                                             /*  ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDdrePostProc] =  */
    {
        BDSP_IMG_DDRE_CODE_SIZE,                        /*  ui32CodeSize */
        BDSP_IMG_DDRE_TABLES_SIZE,                      /*  ui32RomTableSize */
        BDSP_IMG_DDRE_INTER_FRAME_SIZE,                 /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*6,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        100,                                            /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_DDReencodeConfigParams),/*  ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDv258PostProc] =  */
    {
        BDSP_IMG_DV258_CODE_SIZE,                       /*  ui32CodeSize */
        BDSP_IMG_DV258_TABLES_SIZE,                     /*  ui32RomTableSize */
        BDSP_IMG_DV258_INTER_FRAME_SIZE,                /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*6,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        100,                                            /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_DV258ConfigParams),     /*  ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },
  /*    BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eDpcmrPostProc] =  */
    {
        BDSP_IMG_DPCMR_CODE_SIZE,                       /*  ui32CodeSize */
        BDSP_IMG_DPCMR_TABLES_SIZE,                     /*  ui32RomTableSize */
        BDSP_IMG_DPCMR_INTER_FRAME_SIZE,                /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*6,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        100,                                            /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_DpcmrConfigParams),     /*  ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eGenCdbItbPostProc] =  */
    {
        BDSP_IMG_GEN_CDBITB_CODE_SIZE,                  /*  ui32CodeSize */
        BDSP_IMG_GEN_CDBITB_TABLES_SIZE,                /*  ui32RomTableSize */
        BDSP_IMG_GEN_CDBITB_INTER_FRAME_SIZE,           /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*6,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_GenCdbItbConfigParams), /*  ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                        /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_GenCdbItbStreamInfo)    /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eBtscEncoderPostProc] =  */
    {
        BDSP_IMG_BTSCENC_CODE_SIZE,                 /*  ui32CodeSize */
        BDSP_IMG_BTSCENC_TABLES_SIZE,               /*  ui32RomTableSize */
        BDSP_IMG_BTSCENC_INTER_FRAME_SIZE,          /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*6,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_BtscEncoderConfigParams),   /*  ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eSpeexAECPostProc] =  */
    {
        BDSP_IMG_SPEEXAEC_CODE_SIZE,                    /*  ui32CodeSize */
        BDSP_IMG_SPEEXAEC_TABLES_SIZE,                  /*  ui32RomTableSize */
        BDSP_IMG_SPEEXAEC_INTER_FRAME_SIZE,             /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*2,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_SpeexAECConfigParams),  /*  ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eKaraokePostProc] =  */
    {
        BDSP_IMG_KARAOKE_CODE_SIZE,                 /*  ui32CodeSize */
        BDSP_IMG_KARAOKE_TABLES_SIZE,                   /*  ui32RomTableSize */
        BDSP_IMG_KARAOKE_INTER_FRAME_SIZE,              /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*2,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_KaraokeConfigParams),  /*   ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /*  eFwStatusBuffType */
        0                                               /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMixerDapv2PostProc] =  */
    {
        BDSP_IMG_MIXER_DAPV2_CODE_SIZE,                 /*  ui32CodeSize */
        BDSP_IMG_MIXER_DAPV2_TABLES_SIZE,               /*  ui32RomTableSize */
        BDSP_IMG_MIXER_DAPV2_INTER_FRAME_SIZE,          /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*8,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        341380,                                     /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_MixerDapv2ConfigParams),  /*    ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                     /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_MixerDapv2StatusInfo)   /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eOutputFormatterPostProc] =  */
    {
        BDSP_IMG_OUTPUTFORMATTER_CODE_SIZE,                 /*  ui32CodeSize */
        BDSP_IMG_OUTPUTFORMATTER_TABLES_SIZE,                   /*  ui32RomTableSize */
        BDSP_IMG_OUTPUTFORMATTER_INTER_FRAME_SIZE,          /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*2,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_OutputFormatterConfigParams), /*    ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_OutputFormatterStatus)                /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eVocalPostProc] =  */
    {
        BDSP_IMG_VOCALS_CODE_SIZE,                      /*  ui32CodeSize */
        BDSP_IMG_VOCALS_TABLES_SIZE,                    /*  ui32RomTableSize */
        BDSP_IMG_VOCALS_INTER_FRAME_SIZE,               /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*2,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_VocalPPConfigParams),   /*  ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_VocalPPStatus)                /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eFadeCtrlPostProc] =  */
    {
        BDSP_IMG_FADECTRL_CODE_SIZE,                    /*  ui32CodeSize */
        BDSP_IMG_FADECTRL_TABLES_SIZE,                  /*  ui32RomTableSize */
        BDSP_IMG_FADECTRL_INTER_FRAME_SIZE,             /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*2,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_FadeCtrlConfigParams), /*   ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_FadeCtrlPPStatusInfo)   /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAmbisonicsPostProc] =  */
    {
        BDSP_IMG_AMBISONICS_CODE_SIZE,                    /*  ui32CodeSize */
        BDSP_IMG_AMBISONICS_TABLES_SIZE,                  /*  ui32RomTableSize */
        BDSP_IMG_AMBISONICS_INTER_FRAME_SIZE,             /*  ui32InterFrameBuffSize */
        (6144 + BDSP_AF_P_EXTRA_SAMPLES)*4*2,           /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_AmbisonicsConfigParams), /*   ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_AmbisonicsPPStatusInfo)   /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eTsmCorrectionPostProc] =  */
    {
        BDSP_IMG_TSMCORRECTION_CODE_SIZE,               /*  ui32CodeSize */
        BDSP_IMG_TSMCORRECTION_TABLES_SIZE,             /*  ui32RomTableSize */
        BDSP_IMG_TSMCORRECTION_INTER_FRAME_SIZE,        /*  ui32InterFrameBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4*6,             /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        156000,                                          /*  ui32ScratchBuffSize */
        sizeof(BDSP_Raaga_Audio_TsmCorrectionConfigParams),     /*  ui32UserCfgBuffSize */
        (6144+BDSP_AF_P_EXTRA_SAMPLES)*4,               /*  ui32MaxSizePerChan */
        6,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                     /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_TsmCorrectionPPStatusInfo)   /*  FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eEndOfPpAlgos] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMixerFrameSync] =  */
    {
        BDSP_IMG_MIXER_IDS_SIZE,                        /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_MIXER_IDS_INTER_FRAME_SIZE,            /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eMixerDapv2FrameSync] =  */
    {
        BDSP_IMG_MIXER_DAPV2_IDS_SIZE,                      /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        BDSP_IMG_MIXER_DAPV2_IDS_INTER_FRAME_SIZE,      /*  ui32InterFrameBuffSize */
        0,                                              /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        0,                                              /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),  /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        sizeof(BDSP_Raaga_Audio_IdsTsmInfo)             /*  FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eEndOfPpFsAlgos] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eSyslib] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eAlgoLib] =  */
        { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eIDSCommonLib] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eVidIDSCommonLib] =  */
    {
        BDSP_IMG_VIDIDSCOMMON_CODE_SIZE,                        /*  ui32CodeSize */
        0,                                                      /*  ui32RomTableSize */
        BDSP_IMG_VIDIDSCOMMON_INTER_FRAME_SIZE,                 /*  ui32InterFrameBuffSize */
        0,                                                      /*  ui32InterStageIoBuffSize */
        8192,                                                   /*  ui32InterStageGenericBuffSize */
        0,                                                      /*  ui32ScratchBuffSize */
        sizeof(BDSP_VideoEncodeTaskDatasyncSettings),                                                   /*  ui32UserCfgBuffSize */
        0,                                                      /*  ui32MaxSizePerChan */
        2,                                                      /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,                  /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                            /*  eFwStatusBuffType */
        160                                                     /*  FwStatusBuffSize */

    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eEndOfLibAlgos] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eScm1] =    */
    {
        BDSP_IMG_SCM1_DECODE_SIZE,                      /* ui32CodeSize */
        BDSP_IMG_SCM1_DECODE_TABLES_SIZE,               /* ui32RomTableSize */
        BDSP_IMG_SCM1_DECODE_INTER_FRAME_SIZE,          /* ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*1,       /* ui32InterStageIoBuffSize */
        1024,                                   /* ui32InterStageGenericBuffSize */
        0,                                      /* ui32ScratchBuffSize */
        0,                                      /* ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES),         /* ui32MaxSizePerChan */
        1,                                      /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,  /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,             /* eFwStatusBuffType */
        0                                       /* FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eScm2] =    */
    {
        BDSP_IMG_SCM2_DECODE_SIZE,                      /* ui32CodeSize */
        BDSP_IMG_SCM2_DECODE_TABLES_SIZE,               /* ui32RomTableSize */
        BDSP_IMG_SCM2_DECODE_INTER_FRAME_SIZE,          /* ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*1,       /* ui32InterStageIoBuffSize */
        1024,                                   /* ui32InterStageGenericBuffSize */
        0,                                      /* ui32ScratchBuffSize */
        0,                                      /* ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES),         /* ui32MaxSizePerChan */
        1,                                      /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,  /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,             /* eFwStatusBuffType */
        0                                       /* FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eScm3] =    */
    {
        BDSP_IMG_SCM3_DECODE_SIZE,                      /* ui32CodeSize */
        BDSP_IMG_SCM3_DECODE_TABLES_SIZE,               /* ui32RomTableSize */
        BDSP_IMG_SCM3_DECODE_INTER_FRAME_SIZE,          /* ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*1,       /* ui32InterStageIoBuffSize */
        1024,                                   /* ui32InterStageGenericBuffSize */
        0,                                      /* ui32ScratchBuffSize */
        0,                                      /* ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES),         /* ui32MaxSizePerChan */
        1,                                      /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,  /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,             /* eFwStatusBuffType */
        0                                       /* FwStatusBuffSize */
    },
        /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eEndOfScmAlgos] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eScmTask] =     */
    {
        BDSP_IMG_SCM_TASK_CODE_SIZE,            /* ui32CodeSize */
        0,              /* ui32RomTableSize */
        0,          /* ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*1,       /* ui32InterStageIoBuffSize */
        1024,                                   /* ui32InterStageGenericBuffSize */
        0,                                      /* ui32ScratchBuffSize */
        0,                                      /* ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES),         /* ui32MaxSizePerChan */
        1,                                      /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_eAbsent,   /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,             /* eFwStatusBuffType */
        0                                       /* FwStatusBuffSize */
    },
    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eVideoDecodeTask] =     */
    {
        BDSP_IMG_VIDEO_DECODE_TASK_CODE_SIZE,           /* ui32CodeSize */
        0,              /* ui32RomTableSize */
        0,          /* ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*1,       /* ui32InterStageIoBuffSize */
        1024,                                   /* ui32InterStageGenericBuffSize */
        0,                                      /* ui32ScratchBuffSize */
        0,                                      /* ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES),         /* ui32MaxSizePerChan */
        1,                                      /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_eAbsent,   /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,             /* eFwStatusBuffType */
        0                                       /* FwStatusBuffSize */
    },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eVideoEncodeTask] =     */
    {
        BDSP_IMG_VIDEO_ENCODE_TASK_CODE_SIZE,           /* ui32CodeSize */
        0,              /* ui32RomTableSize */
        0,          /* ui32InterFrameBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES)*1,       /* ui32InterStageIoBuffSize */
        1024,                                   /* ui32InterStageGenericBuffSize */
        0,                                      /* ui32ScratchBuffSize */
        0,                                      /* ui32UserCfgBuffSize */
        (2048+BDSP_AF_P_EXTRA_SAMPLES),         /* ui32MaxSizePerChan */
        1,                                      /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_eAbsent,   /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,             /* eFwStatusBuffType */
        0                                       /* FwStatusBuffSize */
    },


    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eEndOfTaskAlgos] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },

    /*  BDSP_sNodeInfo[BDSP_AF_P_AlgoId_eEndOfAlgos] =  */
    { 0,0,0,0,0,0,0,0,0,0,0,0 }

};

/*****************************************************************************/
/**************Audio Codec structure details ****************************/
/*****************************************************************************/

static const BDSP_Raaga_P_AlgorithmSupportInfo BDSP_sAlgorithmSupportInfo[]=
{
    {
        BDSP_Algorithm_eMpegAudioDecode,        /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "MPEG Audio Decode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,  /* Number of nodes in Mpeg Decode */
            {
                BDSP_AF_P_AlgoId_eMpegFrameSync,
                BDSP_AF_P_AlgoId_eMpegDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
    {
        BDSP_Algorithm_eMpegAudioPassthrough,   /* Algorithm */
        BDSP_AlgorithmType_eAudioPassthrough,
        "MPEG Audio Passthru",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,  /* Number of nodes in Mpeg pass through */
            {
                BDSP_AF_P_AlgoId_eMpegFrameSync,
                BDSP_AF_P_AlgoId_ePassThru,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
    {
        BDSP_Algorithm_eMpegAudioEncode,        /* Algorithm */
        BDSP_AlgorithmType_eAudioEncode,
        "MPEG Audio Encode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,  /* Number of nodes in MPEG1 L3 Encode */
            {
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eMpegL3Encode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
    {
        BDSP_Algorithm_eAacAdtsDecode,          /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "AAC ADTS Audio Decode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,  /* Number of nodes in AAC ADTS Decode */
            {
                BDSP_AF_P_AlgoId_eAdtsFrameSync,
                BDSP_AF_P_AlgoId_eAacHeLpSbrDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
            },
        }
    },
    {
        BDSP_Algorithm_eAacLoasDecode,          /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "AAC LOAS Audio Decode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,
            {
                BDSP_AF_P_AlgoId_eLoasFrameSync,
                BDSP_AF_P_AlgoId_eAacHeLpSbrDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
    {
        BDSP_Algorithm_eAacAdtsPassthrough,     /* Algorithm */
        BDSP_AlgorithmType_eAudioPassthrough,
        "AAC ADTS Audio Passthru",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,  /* Number of nodes in AAC ADTS pass thru */
            {
                BDSP_AF_P_AlgoId_eAdtsFrameSync,
                BDSP_AF_P_AlgoId_ePassThru,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
            },
        }
    },
    {
        BDSP_Algorithm_eAacLoasPassthrough,     /* Algorithm */
        BDSP_AlgorithmType_eAudioPassthrough,
        "AAC LOAS Audio Passthru",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,  /* Number of nodes in AAC LOAS pass thru */
            {
                BDSP_AF_P_AlgoId_eLoasFrameSync,
                BDSP_AF_P_AlgoId_ePassThru,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
    {
        BDSP_Algorithm_eAacEncode,              /* Algorithm */
        BDSP_AlgorithmType_eAudioEncode,
        "AAC Audio Encode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,  /* Number of nodes in AAC-HE Encode */
            {
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eAacHeEncode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
            },
        }
    },
    {
        BDSP_Algorithm_eDolbyPulseAdtsDecode,   /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "PULSE ADTS Audio Decode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,
            {
                BDSP_AF_P_AlgoId_eAdtsFrameSync,
                BDSP_AF_P_AlgoId_eDolbyPulseDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
    {
        BDSP_Algorithm_eDolbyPulseLoasDecode,   /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "PULSE LOAS Audio Decode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,
            {
                BDSP_AF_P_AlgoId_eLoasFrameSync,
                BDSP_AF_P_AlgoId_eDolbyPulseDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
    {
        BDSP_Algorithm_eAc3Decode,              /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "AC3 Audio Decode",
        true,                                   /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMS10,      /* License Type */
        {
            2,  /* Number of nodes in AC3 Plus Decode */
            {
                BDSP_AF_P_AlgoId_eMs10DdpFrameSync,
                BDSP_AF_P_AlgoId_eMs10DdpDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
    {
        BDSP_Algorithm_eAc3Decode,              /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "AC3 Audio Decode",
        true,                                   /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eDDP,       /* License Type */
        {
            2,  /* Number of nodes in AC3 Plus Decode */
            {
                BDSP_AF_P_AlgoId_eDdpFrameSync,
                BDSP_AF_P_AlgoId_eDdpDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
            },
        }
    },
    {
        BDSP_Algorithm_eAc3Decode,              /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "AC3 Audio Decode",
        true,                                   /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eAC3,       /* License Type */
        {
            2,  /* Number of nodes in AC3 Decode */
            {
                BDSP_AF_P_AlgoId_eAc3FrameSync,
                BDSP_AF_P_AlgoId_eAc3Decode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid

            }
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
    {
        BDSP_Algorithm_eAc3Passthrough,         /* Algorithm */
        BDSP_AlgorithmType_eAudioPassthrough,
        "AC3 Audio Passthrough",
        true,                                   /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMS10,      /* License Type */
        {
            2,  /* Number of nodes in AC3 Plus Decode */
            {
                BDSP_AF_P_AlgoId_eMs10DdpFrameSync,
                BDSP_AF_P_AlgoId_ePassThru,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
            },
        }
    },
    {
        BDSP_Algorithm_eAc3Passthrough,         /* Algorithm */
        BDSP_AlgorithmType_eAudioPassthrough,
        "AC3 Audio Passthrough",
        true,                                   /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eDDP,       /* License Type */
        {
            2,  /* Number of nodes in AC3 Plus Decode */
            {
                BDSP_AF_P_AlgoId_eDdpFrameSync,

                BDSP_AF_P_AlgoId_ePassThru,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
    },
    {
        BDSP_Algorithm_eAc3Passthrough,         /* Algorithm */
        BDSP_AlgorithmType_eAudioPassthrough,
        "AC3 Audio Passthrough",
        true,                                   /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eAC3,       /* License Type */
        {
            2,  /* Number of nodes in AC3 Decode */
            {
                BDSP_AF_P_AlgoId_eAc3FrameSync,
                BDSP_AF_P_AlgoId_ePassThru,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid

            }
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
    {
        BDSP_Algorithm_eAc3Encode,              /* Algorithm */
        BDSP_AlgorithmType_eAudioEncode,
        "AC3 Audio Encode",
        true,                                   /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMS10,      /* License Type */
        {
            2,  /* Number of nodes in DD Transcode */
            {
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eMs10DDTranscode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
            },
        }
    },
    {
        BDSP_Algorithm_eAc3PlusDecode,          /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "AC3 Plus Audio Decode",
        true,                                   /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMS10,      /* License Type */
        {
            2,  /* Number of nodes in AC3 Plus Decode */
            {
                BDSP_AF_P_AlgoId_eMs10DdpFrameSync,
                BDSP_AF_P_AlgoId_eMs10DdpDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
    {
        BDSP_Algorithm_eAc3PlusDecode,          /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "AC3 Plus Audio Decode",
        true,                                   /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eDDP,       /* License Type */
        {
            2,  /* Number of nodes in AC3 Plus Decode */
            {
                BDSP_AF_P_AlgoId_eDdpFrameSync,
                BDSP_AF_P_AlgoId_eDdpDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
            },
        }
    },
    {
        BDSP_Algorithm_eAc3PlusPassthrough,     /* Algorithm */
        BDSP_AlgorithmType_eAudioPassthrough,
        "AC3 Plus Audio Passthru",
        true,                                   /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMS10,      /* License Type */
        {
            2,  /* Number of nodes in AC3 Plus Pass Thru */
            {
                BDSP_AF_P_AlgoId_eMs10DdpFrameSync,
                BDSP_AF_P_AlgoId_ePassThru,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
    },
    {
        BDSP_Algorithm_eAc3PlusPassthrough,     /* Algorithm */
        BDSP_AlgorithmType_eAudioPassthrough,
        "AC3 Plus Audio Passthru",
        true,                                   /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eDDP,       /* License Type */
        {
            2,  /* Number of nodes in AC3 Plus Decode */
            {
                BDSP_AF_P_AlgoId_eDdpFrameSync,
                BDSP_AF_P_AlgoId_ePassThru,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
            },
        }
    },
    {
        BDSP_Algorithm_eDtsCoreEncode,          /* Algorithm */
        BDSP_AlgorithmType_eAudioEncode,
        "DTS Audio Encode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,  /* Number of nodes in DTS Encode */
            {
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eDtsEncode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
    },
    {
        BDSP_Algorithm_eDtsHdDecode,            /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "DTS HD Audio Decode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,
            {
                BDSP_AF_P_AlgoId_eDtsHdFrameSync,
                BDSP_AF_P_AlgoId_eDtsHdDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
            },
        }
    },
    {
        BDSP_Algorithm_eDtsHdPassthrough,       /* Algorithm */
        BDSP_AlgorithmType_eAudioPassthrough,
        "DTS HD Audio Passthru",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,
            {
                BDSP_AF_P_AlgoId_eDtsHdFrameSync,
                BDSP_AF_P_AlgoId_ePassThru,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
    {
        BDSP_Algorithm_eDts14BitDecode,         /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "DTS 14bit Audio Decode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,
            {
                BDSP_AF_P_AlgoId_eDtsFrameSync,
                BDSP_AF_P_AlgoId_eDtsHdDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
            },
        }
    },
    {
        BDSP_Algorithm_eDts14BitPassthrough,    /* Algorithm */
        BDSP_AlgorithmType_eAudioPassthrough,
        "DTS 14bit Audio Passthru",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,
            {
                BDSP_AF_P_AlgoId_eDtsFrameSync,
                BDSP_AF_P_AlgoId_ePassThru,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
    {
        BDSP_Algorithm_eDtsLbrDecode,           /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "DTS-LBR Audio Decode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,
            {
                    BDSP_AF_P_AlgoId_eDtsLbrFrameSync,
                    BDSP_AF_P_AlgoId_eDtsLbrDecode,
                    BDSP_AF_P_AlgoId_eInvalid,
                    BDSP_AF_P_AlgoId_eInvalid,
                    BDSP_AF_P_AlgoId_eInvalid,
                    BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
            },
        }
    },
    {
        BDSP_Algorithm_eWmaStdDecode,           /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "WMA STD Audio Decode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,  /* Number of nodes in WMA-STD Decode */
            {
                BDSP_AF_P_AlgoId_eWmaStdFrameSync,
                BDSP_AF_P_AlgoId_eWmaStdDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
    {
        BDSP_Algorithm_eWmaProDecode,           /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "WMA PRO Audio Decode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,  /* Number of nodes in WMA-pro Decode */
            {
                BDSP_AF_P_AlgoId_eWmaProFrameSync,
                BDSP_AF_P_AlgoId_eWmaProStdDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
            },
        }
    },
    {
        BDSP_Algorithm_eMlpDecode,              /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "MLP Audio Decode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,
            {
                BDSP_AF_P_AlgoId_eMlpFrameSync,
                BDSP_AF_P_AlgoId_eMlpDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
        },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
    },
    {
        BDSP_Algorithm_eMlpPassthrough,         /* Algorithm */
        BDSP_AlgorithmType_eAudioPassthrough,
        "MLP Audio Passthru",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,
            {
                BDSP_AF_P_AlgoId_eMlpFrameSync,
                BDSP_AF_P_AlgoId_eMLPPassThru,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
            },
        }
    },
    {
        BDSP_Algorithm_eAmrNbDecode,            /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "AMR NB Audio Decode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,
            {
                BDSP_AF_P_AlgoId_ePcmWavFrameSync,
                BDSP_AF_P_AlgoId_eAmrDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
    {
        BDSP_Algorithm_eAmrNbEncode,            /* Algorithm */
        BDSP_AlgorithmType_eAudioEncode,
        "AMR NB Audio Encode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,  /* Number of nodes in Amr Encode */
            {
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eAmrEncode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
    {
        BDSP_Algorithm_eAmrWbDecode,            /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "AMR WB Audio Decode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,
            {
                BDSP_AF_P_AlgoId_ePcmWavFrameSync,
                BDSP_AF_P_AlgoId_eAmrWbDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
    {
        BDSP_Algorithm_eAmrWbEncode,            /* Algorithm */
        BDSP_AlgorithmType_eAudioEncode,
        "AMR WB Audio Encode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,  /* Number of nodes in Amrwb Encode */
            {
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eAmrwbEncode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
    {
        BDSP_Algorithm_eDraDecode,              /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "DRA Audio Decode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,
            {
                BDSP_AF_P_AlgoId_eDraFrameSync,
                BDSP_AF_P_AlgoId_eDraDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
    {
        BDSP_Algorithm_eCookDecode,             /* Algorithm */
        BDSP_AlgorithmType_eAudioDecode,
        "COOK Audio Decode",
        false,                                  /* Dolby License Present */
        BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
        {
            2,
            {
                BDSP_AF_P_AlgoId_eRealAudioLbrFrameSync,
                BDSP_AF_P_AlgoId_eRealAudioLbrDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
            },
        }
    },
    {
    BDSP_Algorithm_eVorbisDecode,           /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "VORBIS Audio Decode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_eVorbisFrameSync,
        BDSP_AF_P_AlgoId_eVorbisDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eFlacDecode,             /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "FLAC Audio Decode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_eFlacFrameSync,
        BDSP_AF_P_AlgoId_eFlacDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eMacDecode,              /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "MAC Audio Decode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_eMacFrameSync,
        BDSP_AF_P_AlgoId_eMacDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eG711Decode,             /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "G711 Audio Decode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eG711G726Decode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eG726Decode,             /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "G726 Audio Decode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eG711G726Decode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eG711Encode,             /* Algorithm */
    BDSP_AlgorithmType_eAudioEncode,
    "G711 Audio Encode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in G711 G726 Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eG711G726Encode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eG726Encode,             /* Algorithm */
    BDSP_AlgorithmType_eAudioEncode,
    "G726 Audio Encode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in G711 G726 Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eG711G726Encode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eG723_1Decode,           /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "G723_1 Audio Decode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eG723_1Decode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eG723_1Encode,           /* Algorithm */
    BDSP_AlgorithmType_eAudioEncode,
    "G723_1 Audio Encode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eG723_1Encode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eG729Decode,             /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "G729 Audio Decode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eG729Decode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eG729Encode,             /* Algorithm */
    BDSP_AlgorithmType_eAudioEncode,
    "G729 Audio Decode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in G729 Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eG729Encode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eLpcmDvdDecode,          /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "LPCM DVD Audio Decode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_eDvdLpcmFrameSync,
        BDSP_AF_P_AlgoId_eDvdLpcmDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eLpcm1394Decode,         /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "LPCM 1394 Audio Decode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_eDvdLpcmFrameSync,
        BDSP_AF_P_AlgoId_eDvdLpcmDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eLpcmBdDecode,           /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "LPCM BD Audio Decode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_eDvdLpcmFrameSync,
        BDSP_AF_P_AlgoId_eDvdLpcmDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_ePcmWavDecode,           /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "PCM WAV Audio Decode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,

        BDSP_AF_P_AlgoId_ePcmWavDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_ePcmDecode,              /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "PCM Audio Decode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePesFrameSync,

        BDSP_AF_P_AlgoId_ePassThru,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eAdpcmDecode,            /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "ADPCM Audio Decode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eAdpcmDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eiLBCDecode,             /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "iLBC Audio Decode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eiLBCDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eiSACDecode,             /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "iSAC Audio Decode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eiSACDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eUdcDecode,              /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "UDC Audio Decode",
    true,                                   /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMS12,      /* License Type */
    {
      2,  /* Number of nodes in UDC Decode */
      {
        BDSP_AF_P_AlgoId_eUdcFrameSync,
        BDSP_AF_P_AlgoId_eUdcDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eUdcPassthrough,             /* Algorithm */
    BDSP_AlgorithmType_eAudioPassthrough,
    "UDC Audio Passthru",
    true,                                   /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMS12,      /* License Type */
    {
      2,  /* Number of nodes in UDC Passthru */
      {
        BDSP_AF_P_AlgoId_eUdcFrameSync,
        BDSP_AF_P_AlgoId_ePassThru,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eDolbyAacheAdtsDecode,   /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "DOLBY AACHE ADTS Audio Decode",
    true,                                   /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMS12,      /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_eAdtsFrameSync,
        BDSP_AF_P_AlgoId_eDolbyAacheDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eDolbyAacheLoasDecode,   /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "DOLBY AACHE LOAS Audio Decode",
    true,                                   /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMS12,      /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_eLoasFrameSync,
        BDSP_AF_P_AlgoId_eDolbyAacheDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eOpusDecode,               /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "Opus Audio Decode",
    false,                                     /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,         /* License Type */
    {
      2,  /* Number of nodes in Opus Decode */
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eOpusDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eALSDecode,              /* Algorithm - ALS BCMA  */
    BDSP_AlgorithmType_eAudioDecode,
    "ALS BCMA Audio Decode",
    false,                                   /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,        /* License Type */
    {
      2,  /* Number of nodes in ALS Decode */
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eALSDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eALSLoasDecode,         /* Algorithm - ALS LOAS */
    BDSP_AlgorithmType_eAudioDecode,
    "ALS LOAS Audio Decode",
    false,                                   /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,        /* License Type */
    {
      2,  /* Number of nodes in ALS Decode */
      {
        BDSP_AF_P_AlgoId_eALSFrameSync,
        BDSP_AF_P_AlgoId_eALSDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },

  {
    BDSP_Algorithm_eAC4Decode,              /* Algorithm */
    BDSP_AlgorithmType_eAudioDecode,
    "AC4 Audio Decode",
    true,                                   /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMS12,      /* License Type */
    {
      2,  /* Number of nodes in AC4 Decode */
      {
        BDSP_AF_P_AlgoId_eAC4FrameSync,
        BDSP_AF_P_AlgoId_eAC4Decode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eiLBCEncode,             /* Algorithm */
    BDSP_AlgorithmType_eAudioEncode,
    "iLBC Audio Encode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in iLBC Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eiLBCEncode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eiSACEncode,             /* Algorithm */
    BDSP_AlgorithmType_eAudioEncode,
    "iSAC Audio Encode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in iLBC Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eiSACEncode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eLpcmEncode,             /* Algorithm */
    BDSP_AlgorithmType_eAudioEncode,
    "LPCM Audio Encode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in LPCM Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eLpcmEncode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eOpusEncode,             /* Algorithm */
    BDSP_AlgorithmType_eAudioEncode,
    "OPUS Audio Encode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in OPUS Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eOpusEncode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eDDPEncode,              /* Algorithm */
    BDSP_AlgorithmType_eAudioEncode,
    "DDP Audio Encode",
    true,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMS12,      /* License Type */
    {
      2,  /* Number of nodes in DDP Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eDDPEncode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eGenericPassthrough,     /* Algorithm */
    BDSP_AlgorithmType_eAudioPassthrough,
    "Generic Audio Passthru",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in Generic Pass through  */
      {
        BDSP_AF_P_AlgoId_eInvalid,

        BDSP_AF_P_AlgoId_ePassThru,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eSrc,                    /* Algorithm */
    BDSP_AlgorithmType_eAudioProcessing,
    "Sample Rate Converter",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in SRC Post Proc */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eSrcPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eDsola,                  /* Algorithm */
    BDSP_AlgorithmType_eAudioProcessing,
    "DSOLA",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in DSOLA  */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eDsolaPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eMixer,                  /* Algorithm */
    BDSP_AlgorithmType_eAudioMixer,
    "FIRMWARE MIXER",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in FWMixer */
      {
        BDSP_AF_P_AlgoId_eMixerFrameSync,
        BDSP_AF_P_AlgoId_eFWMixerPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eGenCdbItb,              /* Algorithm */
    BDSP_AlgorithmType_eAudioProcessing,
    "GEN CDB/ITB",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in GEN CDB ITB */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eGenCdbItbPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eBrcmAvl,                /* Algorithm */
    BDSP_AlgorithmType_eAudioProcessing,
    "BRCM AVL",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in AVL Post Proc */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eAvlPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eBrcm3DSurround,         /* Algorithm */
    BDSP_AlgorithmType_eAudioProcessing,
    "BRCM 3D SURROUND",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in AudysseyV olume */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eBrcm3DSurroundPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eSrsTruSurroundHd,       /* Algorithm */
    BDSP_AlgorithmType_eAudioProcessing,
    "SRS TRUE SURROUND HD",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in SrsHd  */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eSrsTruSurroundHDPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eSrsTruVolume,           /* Algorithm */
    BDSP_AlgorithmType_eAudioProcessing,
    "SRS TRUE VOLUME",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in Srs VolumeIq  */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eSrsTruVolumePostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eDdre,                   /* Algorithm */
    BDSP_AlgorithmType_eAudioProcessing,
    "Dolby Digital Re-Encode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in DDRE */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eDdrePostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eDv258,                  /* Algorithm */
    BDSP_AlgorithmType_eAudioProcessing,
    "Dolby Volume 258",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in DV258 */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eDv258PostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eDpcmr,
    BDSP_AlgorithmType_eAudioProcessing,
    "Dolby PCM Renderer",
    true,
    BDSP_AudioDolbyCodecVersion_eMS12,      /* License Type */
    {
      2,  /* Number of nodes in Dpcmr */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eDpcmrPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eCustomVoice,            /* Algorithm */
    BDSP_AlgorithmType_eAudioProcessing,
    "CUSTOM VOICE",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in CUSTOM Voice Post Proc */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eCustomVoicePostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eBtscEncoder,            /* Algorithm */
    BDSP_AlgorithmType_eAudioProcessing,
    "BTSC Encoder",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in BTSC Encoder */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eBtscEncoderPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eSpeexAec,               /* Algorithm */
    BDSP_AlgorithmType_eAudioEchoCanceller,
    "SPEEX AEC",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eSpeexAECPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eKaraoke,                /* Algorithm */
    BDSP_AlgorithmType_eAudioProcessing,
    "KARAOKE",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eKaraokePostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eMixerDapv2,             /* Algorithm */
    BDSP_AlgorithmType_eAudioMixer,
    "MIXER DAPV2",
    true,                                   /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMS12,      /* License Type */
    {
      2,  /* Number of nodes in FWMixer */
      {
        BDSP_AF_P_AlgoId_eMixerDapv2FrameSync,
        BDSP_AF_P_AlgoId_eMixerDapv2PostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eOutputFormatter,                /* Algorithm */
    BDSP_AlgorithmType_eAudioProcessing,
    "OUTPUTFORMATTER",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eOutputFormatterPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eVocalPP,              /* Algorithm */
    BDSP_AlgorithmType_eAudioProcessing,
    "VOCALPP",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,     /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eVocalPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eFadeCtrl,               /* Algorithm */
    BDSP_AlgorithmType_eAudioProcessing,
    "FadeControl",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eFadeCtrlPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },

  {
    BDSP_Algorithm_eAmbisonics,               /* Algorithm */
    BDSP_AlgorithmType_eAudioProcessing,
    "Ambisonics",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
        2,
        {
            BDSP_AF_P_AlgoId_eInvalid,
            BDSP_AF_P_AlgoId_eAmbisonicsPostProc,
            BDSP_AF_P_AlgoId_eInvalid,
            BDSP_AF_P_AlgoId_eInvalid,
            BDSP_AF_P_AlgoId_eInvalid,
            BDSP_AF_P_AlgoId_eInvalid
        },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },

  {
    BDSP_Algorithm_eTsmCorrection,                  /* Algorithm */
    BDSP_AlgorithmType_eAudioProcessing,
    "TSMCORRECTION",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in TSMCORRECTION  */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eTsmCorrectionPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eVp6Decode,              /* Algorithm */
    BDSP_AlgorithmType_eVideoDecode,
    "VP6 Video Decode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,
      {
        BDSP_VF_P_AlgoId_eVP6FrameSync,
        BDSP_VF_P_AlgoId_eVP6Decode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eH264Encode,             /* Algorithm */
    BDSP_AlgorithmType_eVideoEncode,
    "H.264 Video Encode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in H264 Video Encode  */
      {
        BDSP_AF_P_AlgoId_eVidIDSCommonLib,
        BDSP_VF_P_AlgoId_eH264Encode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  }, /* vijay new addition */
  {
    BDSP_Algorithm_eX264Encode,             /* Algorithm */
    BDSP_AlgorithmType_eVideoEncode,
    "H.264 Video Encode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      2,  /* Number of nodes in H264 Video Encode  */
      {
        BDSP_AF_P_AlgoId_eVidIDSCommonLib,
        BDSP_VF_P_AlgoId_eX264Encode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
#if defined BDSP_INTR_MODE_AX_VIDEO_ENCODE /* Interrupt Mode Ax Video Encode Support */
        false,
        true
#else
          true,
        false
#endif
      },
    }
  },
  {
    BDSP_Algorithm_eXVP8Encode,             /* Algorithm */
    BDSP_AlgorithmType_eVideoEncode,
    "VP8 Video Encode",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,      /* License Type */
    {
      2,/* Number of nodes in VP8 Video Encode  */
      {
        BDSP_AF_P_AlgoId_eVidIDSCommonLib,
        BDSP_VF_P_AlgoId_eXVP8Encode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eSecurityA,              /* Algorithm */
    BDSP_AlgorithmType_eSecurity,
    "Security SCM 1",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      1,  /* Number of nodes in SCM 1 */
      {
        BDSP_AF_P_AlgoId_eScm1,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eSecurityB,              /* Algorithm */
    BDSP_AlgorithmType_eSecurity,
    "Security SCM 2",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      1,  /* Number of nodes in SCM 2 */
      {
        BDSP_AF_P_AlgoId_eScm2,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eSecurityC,              /* Algorithm */
    BDSP_AlgorithmType_eSecurity,
    "Security SCM 3",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      1,  /* Number of nodes in SCM 3 */
      {
        BDSP_AF_P_AlgoId_eScm3,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  /* This entry must always be last */
  {
    BDSP_Algorithm_eMax,                    /* Algorithm */
    BDSP_AlgorithmType_eMax,
    "Invalid",
    false,                                  /* Dolby License Present */
    BDSP_AudioDolbyCodecVersion_eMax,       /* License Type */
    {
      0,
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      0,
      {
        false,
        false
      },
    }
  },
};

#define BDSP_RAAGA_STREAMINFO_VALID_OFFSET(typ,var) (unsigned)((unsigned long)(&((typ *)NULL)->var))

static const BDSP_Raaga_P_AlgorithmInfo BDSP_sAlgorithmInfo[] =
{
#if BDSP_MPEG_SUPPORT
  {
    /* Algorithm */                  /* Type */                       /* Name */           /* Supported */
    BDSP_Algorithm_eMpegAudioDecode, BDSP_AlgorithmType_eAudioDecode, "MPEG Audio Decode", true,
    /* Default User Config */     /* User config size */
    &BDSP_sMpegDefaultUserConfig, sizeof(BDSP_Raaga_Audio_MpegConfigParams),
    /* Stream Info Size */                   /* Valid offset */
    sizeof(BDSP_Raaga_Audio_MpegStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_MpegStreamInfo, ui32StatusValid),
    /* Default IDS Config */            /* IDS config size */
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    /* FW Algorithms Required */
    {
      2,  /* Number of nodes in Mpeg Decode */
      {
        BDSP_AF_P_AlgoId_eMpegFrameSync,
        BDSP_AF_P_AlgoId_eMpegDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#if BDSP_MPEG_PASSTHRU_SUPPORT
  {
    BDSP_Algorithm_eMpegAudioPassthrough, BDSP_AlgorithmType_eAudioPassthrough, "MPEG Passthrough", true,
    &BDSP_sDefaultPassthruSettings, sizeof(BDSP_Raaga_Audio_PassthruConfigParams),
    0, 0xffffffff,
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,  /* Number of nodes in Mpeg pass through */
      {
        BDSP_AF_P_AlgoId_eMpegFrameSync,

        BDSP_AF_P_AlgoId_ePassThru,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#if BDSP_MP3ENC_SUPPORT
  {
    BDSP_Algorithm_eMpegAudioEncode, BDSP_AlgorithmType_eAudioEncode, "MPEG Audio Encode", true,
    &BDSP_sDefMpeg1L3EncConfigSettings, sizeof(BDSP_Raaga_Audio_Mpeg1L3EncConfigParams),
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in MPEG1 L3 Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eMpegL3Encode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#if defined BDSP_AACSBR_SUPPORT && !defined BDSP_MS10_SUPPORT
  {
    BDSP_Algorithm_eAacAdtsDecode, BDSP_AlgorithmType_eAudioDecode, "AAC ADTS Decode", true,
    &BDSP_sAacheDefaultUserConfig, sizeof(BDSP_Raaga_Audio_AacheConfigParams),
    sizeof(BDSP_Raaga_Audio_AacheStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_AacheStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,  /* Number of nodes in AAC ADTS Decode */
      {
        BDSP_AF_P_AlgoId_eAdtsFrameSync,

        BDSP_AF_P_AlgoId_eAacHeLpSbrDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eAacLoasDecode, BDSP_AlgorithmType_eAudioDecode, "AAC LOAS Decode", true,
    &BDSP_sAacheDefaultUserConfig, sizeof(BDSP_Raaga_Audio_AacheConfigParams),
    sizeof(BDSP_Raaga_Audio_AacheStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_AacheStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eLoasFrameSync,
        BDSP_AF_P_AlgoId_eAacHeLpSbrDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_AACSBR_PASSTHRU_SUPPORT
  {
    BDSP_Algorithm_eAacAdtsPassthrough, BDSP_AlgorithmType_eAudioPassthrough, "AAC ADTS Passthrough", true,
    &BDSP_sDefaultPassthruSettings, sizeof(BDSP_Raaga_Audio_PassthruConfigParams),
    0, 0xffffffff,
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,  /* Number of nodes in AAC ADTS  pass thru */
      {
        BDSP_AF_P_AlgoId_eAdtsFrameSync,

        BDSP_AF_P_AlgoId_ePassThru,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eAacLoasPassthrough, BDSP_AlgorithmType_eAudioPassthrough, "AAC LOAS Passthrough", true,
    &BDSP_sDefaultPassthruSettings, sizeof(BDSP_Raaga_Audio_PassthruConfigParams),
    0, 0xffffffff,
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,  /* Number of nodes in AAC LOAS pass thru */
      {
        BDSP_AF_P_AlgoId_eLoasFrameSync,

        BDSP_AF_P_AlgoId_ePassThru,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#if BDSP_AACHEENC_SUPPORT
  {
    BDSP_Algorithm_eAacEncode, BDSP_AlgorithmType_eAudioEncode, "AAC Audio Encode", true,
    &BDSP_sDefAacHeENCConfigSettings, sizeof(BDSP_Raaga_Audio_AacheEncConfigParams),
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in AAC-HE Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eAacHeEncode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_MS10_SUPPORT
  {
    BDSP_Algorithm_eDolbyPulseAdtsDecode, BDSP_AlgorithmType_eAudioDecode, "Dolby Pulse ADTS Decode", true,
    &BDSP_sDolbyPulseDefaultUserConfig, sizeof(BDSP_Raaga_Audio_DolbyPulseUserConfig),
    sizeof(BDSP_Raaga_Audio_DolbyPulseStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_DolbyPulseStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eAdtsFrameSync,
        BDSP_AF_P_AlgoId_eDolbyPulseDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eDolbyPulseLoasDecode, BDSP_AlgorithmType_eAudioDecode, "Dolby Pulse LOAS Decode", true,
    &BDSP_sDolbyPulseDefaultUserConfig, sizeof(BDSP_Raaga_Audio_DolbyPulseUserConfig),
    sizeof(BDSP_Raaga_Audio_DolbyPulseStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_DolbyPulseStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eLoasFrameSync,
        BDSP_AF_P_AlgoId_eDolbyPulseDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
#endif
#ifdef BDSP_AC3_SUPPORT
    {
        #if defined BDSP_MS10_SUPPORT || defined BDSP_DOLBY_DCV_SUPPORT
        BDSP_Algorithm_eAc3Decode, BDSP_AlgorithmType_eAudioDecode, "AC3 Decode", true,
        &BDSP_sDDPDefaultUserConfig, sizeof(BDSP_Raaga_Audio_DDPMultiStreamConfigParams),
        sizeof(BDSP_Raaga_Audio_MultiStreamDDPStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_MultiStreamDDPStreamInfo, ui32StatusValid),
        #else
        BDSP_Algorithm_eAc3Decode, BDSP_AlgorithmType_eAudioDecode, "AC3 Decode", true,
        &BDSP_sUdcdecDefaultUserConfig, sizeof(BDSP_Raaga_Audio_UdcdecConfigParams),
        sizeof(BDSP_Raaga_Audio_UdcStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_UdcStreamInfo, ui32StatusValid),
        #endif
        &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
        #if defined BDSP_MS10_SUPPORT
        {
            2,  /* Number of nodes in AC3 Plus Decode */
            {
                BDSP_AF_P_AlgoId_eMs10DdpFrameSync,
                BDSP_AF_P_AlgoId_eMs10DdpDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
        #elif defined BDSP_DDP_SUPPORT
        {
            2,  /* Number of nodes in AC3 Plus Decode */
            {
                BDSP_AF_P_AlgoId_eDdpFrameSync,

                BDSP_AF_P_AlgoId_eDdpDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
        #else
        {
            2,  /* Number of nodes in AC3 Decode */
            {
                BDSP_AF_P_AlgoId_eAc3FrameSync,

                BDSP_AF_P_AlgoId_eAc3Decode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid

            }
    },
        #endif
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_AC3_PASSTHRU_SUPPORT
    {
        BDSP_Algorithm_eAc3Passthrough, BDSP_AlgorithmType_eAudioPassthrough, "AC3 Passthrough", true,
        &BDSP_sDefaultPassthruSettings, sizeof(BDSP_Raaga_Audio_PassthruConfigParams),
        sizeof(BDSP_Raaga_Audio_Ac3StreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_Ac3StreamInfo, ui32StatusValid),
        &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
        #if defined BDSP_MS10_SUPPORT
        {
            2,  /* Number of nodes in AC3 Plus Decode */
            {
                BDSP_AF_P_AlgoId_eMs10DdpFrameSync,
                BDSP_AF_P_AlgoId_ePassThru,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
        #elif defined BDSP_DDP_SUPPORT
        {
            2,  /* Number of nodes in AC3 Plus Decode */
            {
                BDSP_AF_P_AlgoId_eDdpFrameSync,

                BDSP_AF_P_AlgoId_ePassThru,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
        #else
        {
            2,  /* Number of nodes in AC3 Decode */
            {
                BDSP_AF_P_AlgoId_eAc3FrameSync,

                BDSP_AF_P_AlgoId_ePassThru,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid

            }
    },
        #endif
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
    },
#endif
#ifdef BDSP_MS10_SUPPORT
    {
        BDSP_Algorithm_eAc3Encode, BDSP_AlgorithmType_eAudioEncode, "Dolby AC3 Transcode", true,
        &BDSP_sDefDDTranscodeConfigSettings, sizeof(BDSP_Raaga_Audio_DDTranscodeConfigParams),
        0, 0xffffffff,
        NULL, 0,
        {
            2,  /* Number of nodes in DD Transcode */
            {
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eMs10DDTranscode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
    },
#endif
#ifdef BDSP_DDP_SUPPORT
    {
        #if defined BDSP_MS10_SUPPORT || defined BDSP_DOLBY_DCV_SUPPORT
        BDSP_Algorithm_eAc3PlusDecode, BDSP_AlgorithmType_eAudioDecode, "AC3+ Decode", true,
        &BDSP_sDDPDefaultUserConfig, sizeof(BDSP_Raaga_Audio_DDPMultiStreamConfigParams),
        sizeof(BDSP_Raaga_Audio_MultiStreamDDPStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_MultiStreamDDPStreamInfo, ui32StatusValid),
        #else
        BDSP_Algorithm_eAc3PlusDecode, BDSP_AlgorithmType_eAudioDecode, "AC3+ Decode", true,
        &BDSP_sUdcdecDefaultUserConfig, sizeof(BDSP_Raaga_Audio_UdcdecConfigParams),
        sizeof(BDSP_Raaga_Audio_UdcStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_UdcStreamInfo, ui32StatusValid),
        #endif
        &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
        #if defined BDSP_MS10_SUPPORT
        {
            2,  /* Number of nodes in AC3 Plus Decode */
            {
                BDSP_AF_P_AlgoId_eMs10DdpFrameSync,
                BDSP_AF_P_AlgoId_eMs10DdpDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
        #else
        {
            2,  /* Number of nodes in AC3 Plus Decode */
            {
                BDSP_AF_P_AlgoId_eDdpFrameSync,

                BDSP_AF_P_AlgoId_eDdpDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
        #endif
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_DDP_PASSTHRU_SUPPORT
    {
        BDSP_Algorithm_eAc3PlusPassthrough, BDSP_AlgorithmType_eAudioPassthrough, "AC3+ Passthrough", true,
        &BDSP_sDefaultPassthruSettings, sizeof(BDSP_Raaga_Audio_PassthruConfigParams),
        sizeof(BDSP_Raaga_Audio_DdpStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_DdpStreamInfo, ui32StatusValid),
        &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
        #if defined BDSP_MS10_SUPPORT
        {
            2,  /* Number of nodes in AC3 Plus Decode */
            {
                BDSP_AF_P_AlgoId_eMs10DdpFrameSync,
                BDSP_AF_P_AlgoId_ePassThru,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
        #else
        {
            2,  /* Number of nodes in AC3 Plus Decode */
            {
                BDSP_AF_P_AlgoId_eDdpFrameSync,

                BDSP_AF_P_AlgoId_ePassThru,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
        #endif
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
    },
#endif
#ifdef BDSP_DTSENC_SUPPORT
    {
        BDSP_Algorithm_eDtsCoreEncode, BDSP_AlgorithmType_eAudioEncode, "DTS Core Encode", true,
        &BDSP_sDefDTSENCConfigSettings, sizeof(BDSP_Raaga_Audio_DtsBroadcastEncConfigParams),
        0, 0xffffffff,
        NULL, 0,
        {
            2,  /* Number of nodes in DTS Encode */
            {
                BDSP_AF_P_AlgoId_eInvalid,

                BDSP_AF_P_AlgoId_eDtsEncode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
            },
        }
    },
#endif
#ifdef BDSP_DTSHD_SUPPORT
    {
        BDSP_Algorithm_eDtsHdDecode, BDSP_AlgorithmType_eAudioDecode, "DTS-HD Decode", true,
        &BDSP_sDtsHdDefaultUserConfig, sizeof(BDSP_Raaga_Audio_DtsHdConfigParams),
        sizeof(BDSP_Raaga_Audio_DtsHdStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_DtsHdStreamInfo, ui32StatusValid),
        &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
        {
            2,
            {
                BDSP_AF_P_AlgoId_eDtsHdFrameSync,
                BDSP_AF_P_AlgoId_eDtsHdDecode,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
#endif
#ifdef BDSP_DTSHD_PASSTHRU_SUPPORT
    {
        BDSP_Algorithm_eDtsHdPassthrough, BDSP_AlgorithmType_eAudioPassthrough, "DTS-HD Passthrough", true,
        &BDSP_sDefaultPassthruSettings, sizeof(BDSP_Raaga_Audio_PassthruConfigParams),
        sizeof(BDSP_Raaga_Audio_DtsHdStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_DtsHdStreamInfo, ui32StatusValid),
        &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
        {
            2,
            {
                BDSP_AF_P_AlgoId_eDtsHdFrameSync,
                BDSP_AF_P_AlgoId_ePassThru,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_DTSBROADCAST_SUPPORT
  {
    BDSP_Algorithm_eDts14BitDecode, BDSP_AlgorithmType_eAudioDecode, "DTS 14bit Decode", true,
    &BDSP_sDtsHdDefaultUserConfig, sizeof(BDSP_Raaga_Audio_DtsHdConfigParams),
    sizeof(BDSP_Raaga_Audio_DtsHdStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_DtsHdStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eDtsFrameSync,

        BDSP_AF_P_AlgoId_eDtsHdDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_DTSBROADCAST_PASSTHRU_SUPPORT
  {
    BDSP_Algorithm_eDts14BitPassthrough, BDSP_AlgorithmType_eAudioPassthrough, "DTS 14bit Passthrough", true,
    &BDSP_sDefaultPassthruSettings, sizeof(BDSP_Raaga_Audio_PassthruConfigParams),
    sizeof(BDSP_Raaga_Audio_DtsHdStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_DtsHdStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eDtsFrameSync,

        BDSP_AF_P_AlgoId_ePassThru,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_DTSLBR_SUPPORT
  {
    BDSP_Algorithm_eDtsLbrDecode, BDSP_AlgorithmType_eAudioDecode, "DTS LBR Decode", true,
    &BDSP_sDtsLbrDefaultUserConfig, sizeof(BDSP_Raaga_Audio_DtslbrConfigParams),
    sizeof(BDSP_Raaga_Audio_DtslbrStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_DtslbrStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eDtsLbrFrameSync,

        BDSP_AF_P_AlgoId_eDtsLbrDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_WMA_SUPPORT
  {
    BDSP_Algorithm_eWmaStdDecode, BDSP_AlgorithmType_eAudioDecode, "WMA Decode", true,
    &BDSP_sWmaDefaultUserConfig, sizeof(BDSP_Raaga_Audio_WmaConfigParams),
    sizeof(BDSP_Raaga_Audio_WmaStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_WmaStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,  /* Number of nodes in WMA-STD Decode */
      {
        BDSP_AF_P_AlgoId_eWmaStdFrameSync,

        BDSP_AF_P_AlgoId_eWmaStdDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_WMAPRO_SUPPORT
  {
    BDSP_Algorithm_eWmaProDecode, BDSP_AlgorithmType_eAudioDecode, "WMA Pro Decode", true,
    &BDSP_sWmaProDefaultUserConfig, sizeof(BDSP_Raaga_Audio_WmaProConfigParams),
    sizeof(BDSP_Raaga_Audio_WmaProStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_WmaProStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,  /* Number of nodes in WMA-pro Decode */
      {
        BDSP_AF_P_AlgoId_eWmaProFrameSync,

        BDSP_AF_P_AlgoId_eWmaProStdDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_MLP_SUPPORT
  {
    BDSP_Algorithm_eMlpDecode, BDSP_AlgorithmType_eAudioDecode, "MLP Decode", true,
    &BDSP_sMlpUserConfig, sizeof(BDSP_Raaga_Audio_MlpConfigParams),
    sizeof(BDSP_Raaga_Audio_MlpStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_MlpStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eMlpFrameSync,
        BDSP_AF_P_AlgoId_eMlpDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_MLP_PASSTHROUGH_SUPPORT
  {
    BDSP_Algorithm_eMlpPassthrough, BDSP_AlgorithmType_eAudioPassthrough, "MLP Passthrough", true,
    NULL, 0,
    0, 0xffffffff,
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eMlpFrameSync,
        BDSP_AF_P_AlgoId_eMLPPassThru,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_AMR_SUPPORT
  {
    BDSP_Algorithm_eAmrNbDecode, BDSP_AlgorithmType_eAudioDecode, "AMR-NB Decode", true,
    &BDSP_sAmrDefaultUserConfig, sizeof(BDSP_Raaga_Audio_AmrConfigParams),
    sizeof(BDSP_Raaga_Audio_AmrStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_AmrStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eAmrDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_AMRENC_SUPPORT
  {
    BDSP_Algorithm_eAmrNbEncode, BDSP_AlgorithmType_eAudioEncode, "AMR-NB Encode", true,
    &BDSP_sDefAmrEncConfigSettings, sizeof(BDSP_Raaga_Audio_AmrEncoderUserConfig),
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in Amr Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eAmrEncode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_AMRWB_SUPPORT
  {
    BDSP_Algorithm_eAmrWbDecode, BDSP_AlgorithmType_eAudioDecode, "AMR-WB Decode", true,
    &BDSP_sAmrwbdecDefaultUserConfig, sizeof(BDSP_Raaga_Audio_AmrwbdecConfigParams),
    sizeof(BDSP_Raaga_Audio_AmrWbStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_AmrWbStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eAmrWbDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_AMRWBENC_SUPPORT
  {
    BDSP_Algorithm_eAmrWbEncode, BDSP_AlgorithmType_eAudioEncode, "AMR-WB Encode", true,
    &BDSP_sDefAmrwbEncConfigSettings, sizeof(BDSP_Raaga_Audio_AMRWBConfigParams) /*?*/,
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in Amrwb Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eAmrwbEncode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_DRA_SUPPORT
  {
    BDSP_Algorithm_eDraDecode, BDSP_AlgorithmType_eAudioDecode, "DRA Decode", true,
    &BDSP_sDraDefaultUserConfig, sizeof(BDSP_Raaga_Audio_DraConfigParams),
    sizeof(BDSP_Raaga_Audio_DraStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_DraStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eDraFrameSync,
        BDSP_AF_P_AlgoId_eDraDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_REALAUDIOLBR_SUPPORT
  {
    BDSP_Algorithm_eCookDecode, BDSP_AlgorithmType_eAudioDecode, "Cook Decode", true,
    &BDSP_sRalbrDefaultUserConfig, sizeof(BDSP_Raaga_Audio_RalbrConfigParams),
    sizeof(BDSP_Raaga_Audio_RalbrStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_RalbrStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eRealAudioLbrFrameSync,
        BDSP_AF_P_AlgoId_eRealAudioLbrDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_VORBIS_SUPPORT
  {
    BDSP_Algorithm_eVorbisDecode, BDSP_AlgorithmType_eAudioDecode, "Vorbis Decode", true,
    &BDSP_sVorbisDecUserConfig, sizeof(BDSP_Raaga_Audio_VorbisDecConfigParams),
    sizeof(BDSP_Raaga_Audio_VorbisDecStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_VorbisDecStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eVorbisFrameSync,
        BDSP_AF_P_AlgoId_eVorbisDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_FLAC_SUPPORT
  {
    BDSP_Algorithm_eFlacDecode, BDSP_AlgorithmType_eAudioDecode, "FLAC Decode", true,
    &BDSP_sFlacDecUserConfig, sizeof(BDSP_Raaga_Audio_FlacDecConfigParams),
    sizeof(BDSP_Raaga_Audio_FlacDecStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_FlacDecStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eFlacFrameSync,
        BDSP_AF_P_AlgoId_eFlacDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_MAC_SUPPORT
  {
    BDSP_Algorithm_eMacDecode, BDSP_AlgorithmType_eAudioDecode, "MAC Decode", true,
    &BDSP_sMacDecUserConfig, sizeof(BDSP_Raaga_Audio_MacDecConfigParams),
    sizeof(BDSP_Raaga_Audio_MacDecStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_MacDecStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eMacFrameSync,
        BDSP_AF_P_AlgoId_eMacDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_G711G726_SUPPORT
  {
    BDSP_Algorithm_eG711Decode, BDSP_AlgorithmType_eAudioDecode, "G.711 Decode", true,
    &BDSP_sG711G726DecUserConfig, sizeof(BDSP_Raaga_Audio_G726ConfigParams),
    sizeof(BDSP_Raaga_Audio_G726StreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_G726StreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eG711G726Decode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eG726Decode, BDSP_AlgorithmType_eAudioDecode, "G.726 Decode", true,
    &BDSP_sG711G726DecUserConfig, sizeof(BDSP_Raaga_Audio_G726ConfigParams),
    sizeof(BDSP_Raaga_Audio_G726StreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_G726StreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eG711G726Decode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_G711G726ENC_SUPPORT
  {
    BDSP_Algorithm_eG711Encode, BDSP_AlgorithmType_eAudioEncode, "G.711 Encode", true,
    &BDSP_sDefG711G726EncConfigSettings, sizeof(BDSP_Raaga_Audio_G711_G726EncConfigParams) /*?*/,
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in G711 G726 Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eG711G726Encode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eG726Encode, BDSP_AlgorithmType_eAudioEncode, "G.726 Encode", true,
    &BDSP_sDefG711G726EncConfigSettings, sizeof(BDSP_Raaga_Audio_G711_G726EncConfigParams) /*?*/,
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in G711 G726 Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eG711G726Encode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_G723_1_SUPPORT
  {
    BDSP_Algorithm_eG723_1Decode, BDSP_AlgorithmType_eAudioDecode, "G.723.1 Decode", true,
    &BDSP_sG723_1_Configsettings, sizeof(BDSP_Raaga_Audio_G723_1DEC_ConfigParams),
    sizeof(BDSP_Raaga_Audio_G723_1_StreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_G723_1_StreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eG723_1Decode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_G723_1ENC_SUPPORT
  {
    BDSP_Algorithm_eG723_1Encode, BDSP_AlgorithmType_eAudioEncode, "G.723.1 Encode", true,
    &BDSP_sDefG723_1EncodeConfigSettings, sizeof(BDSP_Raaga_Audio_G723EncoderUserConfig),
    0, 0xffffffff,
    NULL, 0,
    {
      2,
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eG723_1Encode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_G729_SUPPORT
  {
    BDSP_Algorithm_eG729Decode, BDSP_AlgorithmType_eAudioDecode, "G.729 Decode", true,
    &BDSP_sG729DecUserConfig, sizeof(BDSP_Raaga_Audio_G729DecConfigParams),
    sizeof(BDSP_Raaga_Audio_G729DecStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_G729DecStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eG729Decode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_G729ENC_SUPPORT
  {
    BDSP_Algorithm_eG729Encode, BDSP_AlgorithmType_eAudioEncode, "G.729 Encode", true,
    &BDSP_sDefG729EncConfigSettings, sizeof(BDSP_Raaga_Audio_G729EncoderUserConfig),
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in G729 Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eG729Encode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_LPCMDVD_SUPPORT
  {
    BDSP_Algorithm_eLpcmDvdDecode, BDSP_AlgorithmType_eAudioDecode, "LPCM-DVD Decode", true,
    &BDSP_sLcpmDvdDefaultUserConfig, sizeof(BDSP_Raaga_Audio_LpcmUserConfig),
    sizeof(BDSP_Raaga_Audio_LpcmStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_LpcmStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eDvdLpcmFrameSync,

        BDSP_AF_P_AlgoId_eDvdLpcmDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eLpcm1394Decode, BDSP_AlgorithmType_eAudioDecode, "LPCM-1394 Decode", true,
    &BDSP_sLcpmDvdDefaultUserConfig, sizeof(BDSP_Raaga_Audio_LpcmUserConfig),
    sizeof(BDSP_Raaga_Audio_LpcmStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_LpcmStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eDvdLpcmFrameSync,

        BDSP_AF_P_AlgoId_eDvdLpcmDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eLpcmBdDecode, BDSP_AlgorithmType_eAudioDecode, "LPCM-BD Decode", true,
    &BDSP_sLcpmDvdDefaultUserConfig, sizeof(BDSP_Raaga_Audio_LpcmUserConfig),
    sizeof(BDSP_Raaga_Audio_LpcmStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_LpcmStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eDvdLpcmFrameSync,

        BDSP_AF_P_AlgoId_eDvdLpcmDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_PCMWAV_SUPPORT
  {
    BDSP_Algorithm_ePcmWavDecode, BDSP_AlgorithmType_eAudioDecode, "PCM-WAV Decode", true,
    &BDSP_sPcmWavDefaultUserConfig, sizeof(BDSP_Raaga_Audio_PcmWavConfigParams),
    sizeof(BDSP_Raaga_Audio_PcmWavStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_PcmWavStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,

        BDSP_AF_P_AlgoId_ePcmWavDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_PCM_SUPPORT
  {
    BDSP_Algorithm_ePcmDecode, BDSP_AlgorithmType_eAudioDecode, "RAW PCM Decode", true,
    &BDSP_sPcmDefaultUserConfig, sizeof(BDSP_Raaga_Audio_PassthruConfigParams),
    sizeof(BDSP_Raaga_Audio_PassthruStreamInfo), 0xffffffff,
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePesFrameSync,

        BDSP_AF_P_AlgoId_ePassThru,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_ADPCM_SUPPORT
  {
    BDSP_Algorithm_eAdpcmDecode, BDSP_AlgorithmType_eAudioDecode, "ADPCM Decode", true,
    &BDSP_sAdpcmDefaultUserConfig, sizeof(BDSP_Raaga_Audio_AdpcmConfigParams),
    sizeof(BDSP_Raaga_Audio_AdpcmStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_AdpcmStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eAdpcmDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_ILBC_SUPPORT
  {
    BDSP_Algorithm_eiLBCDecode, BDSP_AlgorithmType_eAudioDecode, "iLBC Decode", true,
    &BDSP_siLBCdecDefaultUserConfig, sizeof(BDSP_Raaga_Audio_iLBCdecConfigParams),
    sizeof(BDSP_Raaga_Audio_iLBCStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_iLBCStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eiLBCDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_ISAC_SUPPORT
  {
    BDSP_Algorithm_eiSACDecode, BDSP_AlgorithmType_eAudioDecode, "iSAC Decode", true,
    &BDSP_siSACdecDefaultUserConfig, sizeof(BDSP_Raaga_Audio_iSACdecConfigParams),
    sizeof(BDSP_Raaga_Audio_iSACStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_iSACStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eiSACDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_UDC_SUPPORT
  {
    BDSP_Algorithm_eUdcDecode, BDSP_AlgorithmType_eAudioDecode, "UDC Decode", true,
    &BDSP_sUdcdecDefaultUserConfig, sizeof(BDSP_Raaga_Audio_UdcdecConfigParams),
    sizeof(BDSP_Raaga_Audio_UdcStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_UdcStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eUdcFrameSync,
        BDSP_AF_P_AlgoId_eUdcDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_UDC_PASSTHRU_SUPPORT
  {
    BDSP_Algorithm_eUdcPassthrough, BDSP_AlgorithmType_eAudioPassthrough, "UDC Passthrough", true,
    &BDSP_sDefaultPassthruSettings, sizeof(BDSP_Raaga_Audio_PassthruConfigParams),
    sizeof(BDSP_Raaga_Audio_DdpStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_DdpStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eUdcFrameSync,
        BDSP_AF_P_AlgoId_ePassThru,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_DOLBY_AACHE_SUPPORT
  {
    BDSP_Algorithm_eDolbyAacheAdtsDecode, BDSP_AlgorithmType_eAudioDecode, "Dolby Aache ADTS Decode", true,
    &BDSP_sDolbyAacheDefaultUserConfig, sizeof(BDSP_Raaga_Audio_DolbyAacheUserConfig),
    sizeof(BDSP_Raaga_Audio_DolbyAacheStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_DolbyAacheStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eAdtsFrameSync,
        BDSP_AF_P_AlgoId_eDolbyAacheDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eDolbyAacheLoasDecode, BDSP_AlgorithmType_eAudioDecode, "Dolby Aache LOAS Decode", true,
    &BDSP_sDolbyAacheDefaultUserConfig, sizeof(BDSP_Raaga_Audio_DolbyAacheUserConfig),
    sizeof(BDSP_Raaga_Audio_DolbyAacheStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_DolbyAacheStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eLoasFrameSync,
        BDSP_AF_P_AlgoId_eDolbyAacheDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_OPUSDEC_SUPPORT
  {
    BDSP_Algorithm_eOpusDecode, BDSP_AlgorithmType_eAudioDecode, "Opus Decode", true,
    &BDSP_sOpusDecDefaultUserConfig, sizeof(BDSP_Raaga_Audio_OpusDecConfigParams),
    sizeof(BDSP_Raaga_Audio_OpusDecStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_OpusDecStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eOpusDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_ALS_SUPPORT
  {
    BDSP_Algorithm_eALSDecode, BDSP_AlgorithmType_eAudioDecode, "ALS BCMA Decode", true,
    &BDSP_sALSDecDefaultUserConfig, sizeof(BDSP_Raaga_Audio_ALSDecConfigParams),
    sizeof(BDSP_Raaga_Audio_ALSDecStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_ALSDecStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_ePcmWavFrameSync,
        BDSP_AF_P_AlgoId_eALSDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
  {
    BDSP_Algorithm_eALSLoasDecode, BDSP_AlgorithmType_eAudioDecode, "ALS LOAS Decode", true,
    &BDSP_sALSDecDefaultUserConfig, sizeof(BDSP_Raaga_Audio_ALSDecConfigParams),
    sizeof(BDSP_Raaga_Audio_ALSDecStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_ALSDecStreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eALSFrameSync,
        BDSP_AF_P_AlgoId_eALSDecode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_AC4_SUPPORT
  {
    BDSP_Algorithm_eAC4Decode, BDSP_AlgorithmType_eAudioDecode, "AC4 Decode", true,
    &BDSP_sAC4DecDefaultUserConfig, sizeof(BDSP_Raaga_Audio_AC4DecConfigParams),
    sizeof(BDSP_Raaga_Audio_AC4StreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_AC4StreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_AF_P_AlgoId_eAC4FrameSync,
        BDSP_AF_P_AlgoId_eAC4Decode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_ILBCENC_SUPPORT
  {
    BDSP_Algorithm_eiLBCEncode, BDSP_AlgorithmType_eAudioEncode, "iLBC Encode", true,
    &BDSP_sDefiLBCEncConfigSettings, sizeof(BDSP_Raaga_Audio_ILBCConfigParams) /*?*/,
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in iLBC Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eiLBCEncode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_ISACENC_SUPPORT
  {
    BDSP_Algorithm_eiSACEncode, BDSP_AlgorithmType_eAudioEncode, "iSAC Encode", true,
    &BDSP_sDefiSACEncConfigSettings, sizeof(BDSP_Raaga_Audio_ISACConfigParams) /*?*/,
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in iLBC Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eiSACEncode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_LPCMENC_SUPPORT
  {
    BDSP_Algorithm_eLpcmEncode, BDSP_AlgorithmType_eAudioEncode, "LPCM Encode", true,
    &BDSP_sDefLpcmEncConfigSettings, sizeof(BDSP_Raaga_Audio_LpcmEncConfigParams) /*?*/,
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in LPCM Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eLpcmEncode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif

#ifdef BDSP_OPUS_ENC_SUPPORT
  {
    BDSP_Algorithm_eOpusEncode, BDSP_AlgorithmType_eAudioEncode, "OPUS Encode", true,
    &BDSP_sDefOpusEncConfigSettings, sizeof(BDSP_Raaga_Audio_OpusEncConfigParams) /*?*/,
    0, 0xffffffff,
    NULL, 0,
    {
      2,      /* Number of nodes in OPUS Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eOpusEncode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_DDPENC_SUPPORT
  {
    BDSP_Algorithm_eDDPEncode, BDSP_AlgorithmType_eAudioEncode, "DDP Encode", true,
    &BDSP_sDefDDPEncConfigSettings, sizeof(BDSP_Raaga_Audio_DDPEncConfigParams) /*?*/,
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in DDP Encode */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eDDPEncode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
  {
    BDSP_Algorithm_eGenericPassthrough, BDSP_AlgorithmType_eAudioPassthrough, "Generic Passthrough", true,
    &BDSP_sDefaultPassthruSettings, sizeof(BDSP_Raaga_Audio_PassthruConfigParams),
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in Generic Pass through  */
      {
        BDSP_AF_P_AlgoId_eInvalid,

        BDSP_AF_P_AlgoId_ePassThru,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#ifdef BDSP_SRC_SUPPORT
  {
    BDSP_Algorithm_eSrc, BDSP_AlgorithmType_eAudioProcessing, "Sample Rate Conversion", true,
    &BDSP_sDefaultSrcSettings, sizeof(BDSP_Raaga_Audio_SRCUserConfigParams),
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in SRC Post Proc */
      {
        BDSP_AF_P_AlgoId_eInvalid,

        BDSP_AF_P_AlgoId_eSrcPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_DSOLA_SUPPORT
  {
    BDSP_Algorithm_eDsola, BDSP_AlgorithmType_eAudioProcessing, "DSOLA", true,
    &BDSP_sDefDsolaConfigSettings, sizeof(BDSP_Raaga_Audio_DsolaConfigParams),
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in DSOLA  */
      {
        BDSP_AF_P_AlgoId_eInvalid,

        BDSP_AF_P_AlgoId_eDsolaPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_FWMIXER_SUPPORT
  {
    BDSP_Algorithm_eMixer, BDSP_AlgorithmType_eAudioMixer, "FW Mixer", true,
    &BDSP_sDefFwMixerConfigSettings, sizeof(BDSP_Raaga_Audio_MixerConfigParams),
    0, 0xffffffff,
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,  /* Number of nodes in FWMixer */
      {
        BDSP_AF_P_AlgoId_eMixerFrameSync,
        BDSP_AF_P_AlgoId_eFWMixerPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_GENCDBITB_SUPPORT
  {
    BDSP_Algorithm_eGenCdbItb, BDSP_AlgorithmType_eAudioProcessing, "Gen CDB/ITB", true,
    &BDSP_sDefGenCdbItbConfigSettings, sizeof(BDSP_Raaga_Audio_GenCdbItbConfigParams),
    sizeof(BDSP_Raaga_Audio_GenCdbItbStreamInfo), 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in GEN CDB ITB */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eGenCdbItbPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_AVL_SUPPORT
  {
    BDSP_Algorithm_eBrcmAvl, BDSP_AlgorithmType_eAudioProcessing, "BRCM AVL", true,
    &BDSP_sDefAVLConfigSettings, sizeof(BDSP_Raaga_Audio_AVLConfigParams),
    sizeof(BDSP_Raaga_Audio_AvlPPStatusInfo), 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in AVL Post Proc */
      {
        BDSP_AF_P_AlgoId_eInvalid,

        BDSP_AF_P_AlgoId_eAvlPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_BRCM3DSURROUND_SUPPORT
  {
    BDSP_Algorithm_eBrcm3DSurround, BDSP_AlgorithmType_eAudioProcessing, "BRCM 3D Surround", true,
    &BDSP_sDefBrcm3DSurroundConfigSettings, sizeof(BDSP_Raaga_Audio_Brcm3DSurroundConfigParams),
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in AudysseyV olume */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eBrcm3DSurroundPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_SRSHD_SUPPORT
  {
    BDSP_Algorithm_eSrsTruSurroundHd, BDSP_AlgorithmType_eAudioProcessing, "SRS TruSurround HD", true,
    &BDSP_sDefTruSurrndHDConfigSettings, sizeof(BDSP_Raaga_Audio_TruSurrndHDConfigParams),
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in SrsHd  */
      {
        BDSP_AF_P_AlgoId_eInvalid,

        BDSP_AF_P_AlgoId_eSrsTruSurroundHDPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_SRSTRUVOL_SUPPORT
  {
    BDSP_Algorithm_eSrsTruVolume, BDSP_AlgorithmType_eAudioProcessing, "SRS TruVolume", true,
    &BDSP_sDefSrsTruVolumeUserConfig, sizeof(BDSP_Raaga_Audio_TruVolumeUserConfig),
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in Srs VolumeIq  */
      {
        BDSP_AF_P_AlgoId_eInvalid,

        BDSP_AF_P_AlgoId_eSrsTruVolumePostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_DDRE_SUPPORT
  {
    BDSP_Algorithm_eDdre, BDSP_AlgorithmType_eAudioProcessing, "Dolby Digital Re-Encode", true,
    &BDSP_sDefDDReencodeUserConfig, sizeof(BDSP_Raaga_Audio_DDReencodeConfigParams),
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in DDRE */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eDdrePostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_DV258_SUPPORT
  {
    BDSP_Algorithm_eDv258, BDSP_AlgorithmType_eAudioProcessing, "Dolby Volume 258", true,
    &BDSP_sDefDV258UserConfig, sizeof(BDSP_Raaga_Audio_DV258ConfigParams),
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in DV258 */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eDv258PostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_DPCMR_SUPPORT
  {
    BDSP_Algorithm_eDpcmr, BDSP_AlgorithmType_eAudioProcessing, "Dolby PCM Renderer", true,
    &BDSP_sDefDpcmrUserConfig, sizeof(BDSP_Raaga_Audio_DpcmrConfigParams),
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in Dpcmr */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eDpcmrPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_CUSTOMVOICE_SUPPORT
  {
    BDSP_Algorithm_eCustomVoice, BDSP_AlgorithmType_eAudioProcessing, "Custom Voice", true,
    &BDSP_sDefCustomVoiceConfigSettings, sizeof(BDSP_Raaga_Audio_CustomVoiceConfigParams),
    sizeof(BDSP_Raaga_Audio_CustomVoiceStatusInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_CustomVoiceStatusInfo, ui32StatusValid),
    NULL, 0,
    {
      2,  /* Number of nodes in CUSTOM Voice Post Proc */
      {
        BDSP_AF_P_AlgoId_eInvalid,

        BDSP_AF_P_AlgoId_eCustomVoicePostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_BTSCENC_SUPPORT
  {
    BDSP_Algorithm_eBtscEncoder, BDSP_AlgorithmType_eAudioProcessing, "BTSC Encoder", true,
    &BDSP_sDefBtscEncoderConfigSettings, sizeof(BDSP_Raaga_Audio_BtscEncoderConfigParams),
    0, 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in BTSC Encoder */
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eBtscEncoderPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_SPEEXAEC_SUPPORT
  {
    BDSP_Algorithm_eSpeexAec, BDSP_AlgorithmType_eAudioEchoCanceller, "SPEEX AEC", true,
    &BDSP_sDefSpeexAECConfigParams, sizeof(BDSP_Raaga_Audio_SpeexAECConfigParams),
    0, 0xffffffff,
    NULL, 0,
    {
      2,
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eSpeexAECPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif

#ifdef BDSP_KARAOKE_SUPPORT
  {
    BDSP_Algorithm_eKaraoke, BDSP_AlgorithmType_eAudioProcessing, "KARAOKE PP", true,
    &BDSP_sDefKaraokeConfigSettings, sizeof(BDSP_Raaga_Audio_KaraokeConfigParams),
    0, 0xffffffff,
    NULL, 0,
    {
      2,
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eKaraokePostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_OUTPUTFORMATTER_SUPPORT
  {
    BDSP_Algorithm_eOutputFormatter, BDSP_AlgorithmType_eAudioProcessing, "OUTPUT FORMATTER PP", true,
    &BDSP_sDefOutputFormatterConfigSettings, sizeof(BDSP_Raaga_Audio_OutputFormatterConfigParams),
    0, 0xffffffff,
    NULL, 0,
    {
      2,
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eOutputFormatterPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
        }
    },
#endif
#ifdef BDSP_MIXERDAPV2_SUPPORT
    {
        BDSP_Algorithm_eMixerDapv2, BDSP_AlgorithmType_eAudioMixer, "Mixer Dapv2", true,
        &BDSP_sDefMixerDapv2ConfigParams, sizeof(BDSP_Raaga_Audio_MixerDapv2ConfigParams),
        sizeof(BDSP_Raaga_Audio_MixerDapv2StatusInfo), 0xffffffff,
        &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
        {
            2,  /* Number of nodes in MixerDapv2 */
            {
                BDSP_AF_P_AlgoId_eMixerDapv2FrameSync,
                BDSP_AF_P_AlgoId_eMixerDapv2PostProc,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
            },
        }
    },
#endif

#ifdef BDSP_VOCALPP_SUPPORT
    {
        BDSP_Algorithm_eVocalPP, BDSP_AlgorithmType_eAudioProcessing, "VOCALPP", true,
        &BDSP_sDefVocalPPConfigSettings, sizeof(BDSP_Raaga_Audio_VocalPPConfigParams),
        0, 0xffffffff,
        NULL, 0,
        {
            2,
            {
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eVocalPostProc,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_FADECTRL_SUPPORT
  {
    BDSP_Algorithm_eFadeCtrl, BDSP_AlgorithmType_eAudioProcessing, "FadeControl", true,
    &BDSP_sDefFadeCtrlConfigSettings, sizeof(BDSP_Raaga_Audio_FadeCtrlConfigParams),
    sizeof(BDSP_Raaga_Audio_FadeCtrlPPStatusInfo), 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in FadeControl Post Proc */
      {
        BDSP_AF_P_AlgoId_eInvalid,

        BDSP_AF_P_AlgoId_eFadeCtrlPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif

#ifdef BDSP_AMBISONICS_SUPPORT
    {
        BDSP_Algorithm_eAmbisonics, BDSP_AlgorithmType_eAudioProcessing, "Ambisonics", true,
        &BDSP_sDefAmbisonicsConfigSettings, sizeof(BDSP_Raaga_Audio_AmbisonicsConfigParams),
        sizeof(BDSP_Raaga_Audio_AmbisonicsPPStatusInfo), 0xffffffff,
        NULL, 0,
        {
            2,  /* Number of nodes in Ambisonics Post Proc */
            {
                BDSP_AF_P_AlgoId_eInvalid,

                BDSP_AF_P_AlgoId_eAmbisonicsPostProc,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid,
                BDSP_AF_P_AlgoId_eInvalid
            },
        },
        /* Scheduling Groups Info */
        {
          1,
          {
            true,
            false
          },
        }
    },
#endif

#ifdef BDSP_TSMCORRECTION_SUPPORT
  {
    BDSP_Algorithm_eTsmCorrection, BDSP_AlgorithmType_eAudioProcessing, "TSMCORRECTION", true,
    &BDSP_sDefTsmCorrectionConfigSettings, sizeof(BDSP_Raaga_Audio_TsmCorrectionConfigParams),
    sizeof(BDSP_Raaga_Audio_TsmCorrectionPPStatusInfo), 0xffffffff,
    NULL, 0,
    {
      2,  /* Number of nodes in TSMCORRECTION  */
      {
        BDSP_AF_P_AlgoId_eInvalid,

        BDSP_AF_P_AlgoId_eTsmCorrectionPostProc,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_VP6_SUPPORT
  {
    BDSP_Algorithm_eVp6Decode, BDSP_AlgorithmType_eVideoDecode, "VP6 Decoder", true,
    NULL, 0,
    sizeof(BDSP_Raaga_P_Vp6StreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_P_Vp6StreamInfo, ui32StatusValid),
    &BDSP_sDefaultFrameSyncTsmSettings, sizeof(BDSP_sDefaultFrameSyncTsmSettings),
    {
      2,
      {
        BDSP_VF_P_AlgoId_eVP6FrameSync,
        BDSP_VF_P_AlgoId_eVP6Decode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_H264_ENCODE_SUPPORT
  {
    BDSP_Algorithm_eH264Encode, BDSP_AlgorithmType_eVideoEncode, "H.264 Encoder", true,
    &BDSP_sBH264EncodeUserConfigSettings, sizeof(BDSP_sBH264EncodeUserConfigSettings),
    sizeof(BDSP_Raaga_VideoH264EncoderInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_VideoH264EncoderInfo, ui32StatusValid),
    &BDSP_sDefaultVideoEncodeFrameSyncSettings, sizeof(BDSP_VideoEncodeTaskDatasyncSettings),
    {
      2,  /* Number of nodes in H264 Video Encode  */
      {
        BDSP_AF_P_AlgoId_eVidIDSCommonLib,
        BDSP_VF_P_AlgoId_eH264Encode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_X264_ENCODE_SUPPORT
  {
    BDSP_Algorithm_eX264Encode, BDSP_AlgorithmType_eVideoEncode, "X.264 Encoder", true,
    &BDSP_sBX264EncodeUserConfigSettings, sizeof(BDSP_sBX264EncodeUserConfigSettings),
    sizeof(BDSP_Raaga_VideoX264EncoderInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_VideoX264EncoderInfo, ui32StatusValid),
    &BDSP_sDefaultVideoEncodeFrameSyncSettings, sizeof(BDSP_VideoEncodeTaskDatasyncSettings),
    {
      2,  /* Number of nodes in H264 Video Encode  */
      {
        BDSP_AF_P_AlgoId_eVidIDSCommonLib,
        BDSP_VF_P_AlgoId_eX264Encode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
#if defined BDSP_INTR_MODE_AX_VIDEO_ENCODE /* Interrupt Mode Ax Video Encode Support */
        false,
        true
#else
          true,
        false
#endif
      },
    }
  },
#endif
#ifdef BDSP_XVP8_ENCODE_SUPPORT
  {
    BDSP_Algorithm_eXVP8Encode, BDSP_AlgorithmType_eVideoEncode, "VP8 Encoder", true,
    &BDSP_sBXVP8EncodeUserConfigSettings, sizeof(BDSP_sBXVP8EncodeUserConfigSettings),
    sizeof(BDSP_Raaga_VideoXVP8EncoderInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_VideoXVP8EncoderInfo, ui32StatusValid),
    &BDSP_sDefaultVideoEncodeFrameSyncSettings, sizeof(BDSP_VideoEncodeTaskDatasyncSettings),
    {
      2,  /* Number of nodes in VP8 Video Encode  */
      {
        BDSP_AF_P_AlgoId_eVidIDSCommonLib,
        BDSP_VF_P_AlgoId_eXVP8Encode,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif


#ifdef BDSP_SCM1_SUPPORT
  {
    BDSP_Algorithm_eSecurityA, BDSP_AlgorithmType_eSecurity, "Security SCM 1", true,
    NULL, 0,
    0, 0xffffffff,
    NULL, 0,
    {
      1,  /* Number of nodes in SCM 1 */
      {
        BDSP_AF_P_AlgoId_eScm1,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_SCM2_SUPPORT
  {
    BDSP_Algorithm_eSecurityB, BDSP_AlgorithmType_eSecurity, "Security SCM 2", true,
    NULL, 0,
    0, 0xffffffff,
    NULL, 0,
    {
      1,  /* Number of nodes in SCM 2 */
      {
        BDSP_AF_P_AlgoId_eScm2,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
#ifdef BDSP_SCM3_SUPPORT
  {
    BDSP_Algorithm_eSecurityC, BDSP_AlgorithmType_eSecurity, "Security SCM 3", true,
    NULL, 0,
    0, 0xffffffff,
    NULL, 0,
    {
      1,  /* Number of nodes in SCM 3 */
      {
        BDSP_AF_P_AlgoId_eScm3,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      1,
      {
        true,
        false
      },
    }
  },
#endif
  /* This entry must always be last */
  {
    BDSP_Algorithm_eMax, BDSP_AlgorithmType_eMax, "Invalid", false,
    NULL, 0,
    0, 0xffffffff,
    NULL, 0,
    {
      0,
      {
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid,
        BDSP_AF_P_AlgoId_eInvalid
      },
    },
    /* Scheduling Groups Info */
    {
      0,
      {
        false,
        false
      },
    }
  },
};

const BDSP_Raaga_P_AlgorithmSupportInfo *BDSP_Raaga_P_LookupAlgorithmSupported(
    BDSP_Algorithm              algorithm,
    BDSP_AudioDolbyCodecVersion DolbyCodecVersion
    )
{
  unsigned i;
  /* From BDSP side there is no difference between MS10 and MS11.
     Since proper mapping is required from NEXUS, we use this logic. MS11 decision making is done as part of APE */
  if(DolbyCodecVersion == BDSP_AudioDolbyCodecVersion_eMS11)
  {
    DolbyCodecVersion = BDSP_AudioDolbyCodecVersion_eMS10;
  }
  for ( i = 0; BDSP_sAlgorithmSupportInfo[i].algorithm != BDSP_Algorithm_eMax; i++ )
  {
    if (BDSP_sAlgorithmSupportInfo[i].algorithm == algorithm)
    {
      if(BDSP_sAlgorithmSupportInfo[i].DolbyLicensePresent == false)
      {
        break;
      }
      else if(DolbyCodecVersion == BDSP_sAlgorithmSupportInfo[i].DolbyCodecVersion)
      {
        break;
      }
      else
      {
        /*Continue the loop */
      }
    }
  }
  return &BDSP_sAlgorithmSupportInfo[i];
}

const BDSP_Raaga_P_AlgorithmInfo *BDSP_Raaga_P_LookupAlgorithmInfo_isrsafe(
    BDSP_Algorithm algorithm
    )
{
  unsigned i;

  for ( i = 0; BDSP_sAlgorithmInfo[i].algorithm != BDSP_Algorithm_eMax; i++ )
  {
    if ( BDSP_sAlgorithmInfo[i].algorithm == algorithm )
    {
      break;
    }
    }

    return &BDSP_sAlgorithmInfo[i];
}

BERR_Code BDSP_Raaga_P_GetFWSize (
    const BIMG_Interface *iface,
    void *context,
    unsigned firmware_id,
    uint32_t *size
    )
{
    void *image;
    BERR_Code rc;
    const void *data;
    uint32_t ui32DataSize = 0;

    BDBG_ASSERT(NULL != iface);
    BDBG_ASSERT(NULL != context);

    *size=0;
    rc = iface->open (context, &image, (uint32_t)firmware_id);

    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error in Opening the Image Interface for FW_ID =%d ",firmware_id));;
        return BERR_TRACE(rc);
    }

    rc = iface->next(image, 0, &data, 8);

    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error in fetching next chunk in Image Interface"));;
        iface->close(image);
        return BERR_TRACE(rc);
    }

    ui32DataSize = ((uint32_t *) data)[0];

    *size = ui32DataSize;

    iface->close(image);
    return rc;

}

bool BDSP_Raaga_P_AlgoHasTables(
    BDSP_AF_P_AlgoId algorithm
    )
{
    BDBG_CASSERT(BDSP_AF_P_AlgoId_eEndOfDecodeAlgos < BDSP_AF_P_AlgoId_eEndOfDecFsAlgos);
    BDBG_CASSERT(BDSP_AF_P_AlgoId_eEndOfDecFsAlgos < BDSP_AF_P_AlgoId_eEndOfEncodeAlgos);
    BDBG_CASSERT(BDSP_AF_P_AlgoId_eEndOfEncodeAlgos < BDSP_AF_P_AlgoId_eEndOfEncFsAlgos);
    BDBG_CASSERT(BDSP_AF_P_AlgoId_eEndOfEncFsAlgos < BDSP_AF_P_AlgoId_eEndOfAuxAlgos);
    BDBG_CASSERT(BDSP_AF_P_AlgoId_eEndOfAuxAlgos < BDSP_AF_P_AlgoId_eEndOfPpAlgos);
    BDBG_CASSERT(BDSP_AF_P_AlgoId_eEndOfPpAlgos < BDSP_AF_P_AlgoId_eEndOfPpFsAlgos);

    if ( algorithm <= BDSP_AF_P_AlgoId_eEndOfDecodeAlgos )
    {
        switch ( algorithm )
        {
        case BDSP_AF_P_AlgoId_ePcmWavDecode:
            /* PCM WAV is the only decoder without a table image */
            return false;
        default:
            return true;
        }
    }
    else if ( algorithm <= BDSP_AF_P_AlgoId_eEndOfDecFsAlgos )
    {
        /* Framesync does not have tables */
        return false;
    }
    else if ( algorithm <= BDSP_AF_P_AlgoId_eEndOfEncodeAlgos )
    {
        /* All encoders have tables */
        return true;
    }
    else if ( algorithm <= BDSP_AF_P_AlgoId_eEndOfEncFsAlgos )
    {
        /* Framesync does not have tables */
        return false;
    }
    else if ( algorithm <= BDSP_AF_P_AlgoId_eEndOfPpAlgos )
    {
        /* All aux/pp have tables */
        return true;
    }
    else if ( (algorithm >= BDSP_AF_P_AlgoId_eScm1 )&& (algorithm < BDSP_AF_P_AlgoId_eEndOfScmAlgos) )
    {
        /* All scm algos have tables */
        return true;
    }
    else
    {
        /* Remaining are framesync */
        return false;
    }
}
