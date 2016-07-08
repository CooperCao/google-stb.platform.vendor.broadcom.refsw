/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 *****************************************************************************/


#ifndef BDSP_COMMON_FW_SETTINGS_H
#define BDSP_COMMON_FW_SETTINGS_H

#include "bdsp_common_fw.h"

typedef enum BDSP_eTsmBool
{
    BDSP_eTsmBool_False,
    BDSP_eTsmBool_True,
    BDSP_eTsmBool_Last,
    BDSP_eTsmBool_Invalid = 0x7FFFFFFF
}BDSP_eTsmBool;


typedef enum BDSP_Audio_WMAIpType
{
    BDSP_Audio_WMAIpType_eASF = 0,
    BDSP_Audio_WMAIpType_eTS,
    BDSP_Audio_WMAIpType_eMax,
    BDSP_Audio_WMAIpType_eInvalid = 0x7FFFFFFF
}BDSP_Audio_WMAIpType;

typedef enum BDSP_Audio_ASFPTSType
{
     BDSP_Audio_ASFPTSType_eInterpolated = 0,
     BDSP_Audio_ASFPTSType_eCoded,
     BDSP_Audio_ASFPTSType_eMax,
     BDSP_Audio_ASFPTSType_eInvalid = 0x7FFFFFFF
}BDSP_Audio_ASFPTSType;

typedef enum BDSP_Audio_LpcmAlgoType
{
    BDSP_Audio_LpcmAlgoType_eDvd,
    BDSP_Audio_LpcmAlgoType_eIeee1394,
    BDSP_Audio_LpcmAlgoType_eBd,
    BDSP_Audio_LpcmAlgoType_eMax,
    BDSP_Audio_LpcmAlgoType_eInvalid = 0x7FFFFFFF
}BDSP_Audio_LpcmAlgoType;

typedef enum BDSP_Audio_DtsEndianType
{
    BDSP_Audio_DtsEndianType_eBIG_ENDIAN = 0,
    BDSP_Audio_DtsEndianType_eLITTLE_ENDIAN,
    BDSP_Audio_DtsEndianType_eINVALID = 0x7FFFFFFF
}BDSP_Audio_DtsEndianType;

typedef enum BDSP_Audio_AudioInputSource
{
    BDSP_Audio_AudioInputSource_eExtI2s0 = 0,     /* External I2S Capture port */
    BDSP_Audio_AudioInputSource_eCapPortRfI2s,   /* BTSC Capture port */
    BDSP_Audio_AudioInputSource_eCapPortSpdif,     /* SPDIF Capture port      */
    BDSP_Audio_AudioInputSource_eCapPortHdmi,      /* HDMI */
    BDSP_Audio_AudioInputSource_eCapPortAdc,
    BDSP_Audio_AudioInputSource_eRingbuffer,      /* This is for Certification needs where PCM Samples are loaded from file to Ring buffers*/
    BDSP_Audio_AudioInputSource_eMax,              /* Invalid/last Entry */
    BDSP_Audio_AudioInputSource_eInvalid = 0x7FFFFFFF

} BDSP_Audio_AudioInputSource;

typedef struct BDSP_AudioTaskDatasyncSettings
{
    BDSP_AF_P_EnableDisable         eEnablePESBasedFrameSync;    /* Default = Disabled */
    BDSP_Audio_AudioInputSource     eAudioIpSourceType;           /* Capture port Type    */

    union
    {
        uint32_t ui32SamplingFrequency;                 /* Will be used if IpPortType is I2S*/
        uint32_t ui32RfI2sCtrlStatusRegAddr;            /* For RfI2s i.e. BTSC */
        uint32_t ui32SpdifCtrlStatusRegAddr;            /* For SPDIF */
        uint32_t ui32MaiCtrlStatusRegAddr;              /* For HDMI */

    }uAudioIpSourceDetail;

    BDSP_AF_P_EnableDisable               eEnableTargetSync;   /* Default = Enabled */

    union
    {
        struct
        {
            BDSP_Audio_ASFPTSType eAsfPtsType;            /* Default = 0 (Use Interpolation always). 1 = Use Coded always. */
            BDSP_Audio_WMAIpType eWMAIpType;              /* Default = 0 (Type ASF). Set to TS only when WMATS is enabled */
        }sWmaConfig;
        struct
        {
            BDSP_Audio_LpcmAlgoType               eLpcmType;
        }sLpcmConfig;
        struct
        {
            BDSP_Audio_DtsEndianType              eDtsEndianType; /* Added for DTS-CD Little Endian Support */
        }sDtsConfig;
    }uAlgoSpecConfigStruct;                                     /* The algo specific structures for configuration */

    BDSP_AF_P_EnableDisable               eForceCompleteFirstFrame;   /* If enabled, the first frame will always be entirely rendered to the
                                                                         output and not partially truncated for TSM computations.  This should
                                                                         be disabled for normal operation, but may be required for some bit-exact
                                                                         certification testing that requires all data to be rendered even with TSM
                                                                         enabled. */

} BDSP_AudioTaskDatasyncSettings ;

typedef struct BDSP_AudioTaskTsmSettings
{
    int32_t                 i32TSMSmoothThreshold;
    int32_t                 i32TSMSyncLimitThreshold;
    int32_t                 i32TSMGrossThreshold;
    int32_t                 i32TSMDiscardThreshold;
    int32_t                 i32TsmTransitionThreshold;     /* Transition threshold is required for the
                                                                                DVD case not required right now*/
    uint32_t                ui32STCAddr;
    uint32_t                ui32AVOffset;
    /* SwSTCOffset. This earlier was ui32PVROffset */
    uint32_t                ui32SwSTCOffset;
    uint32_t                ui32AudioOffset;

    /* For TSM error recovery*/
    BDSP_eTsmBool         eEnableTSMErrorRecovery; /* Whether to go for error recovery
                                                                                            when there are continuous TSM_FAIL */
    BDSP_eTsmBool         eSTCValid;        /* If the STC in valid or not. In NRT case case, this is StcOffsetValid */
    BDSP_eTsmBool         ePlayBackOn;  /* If the play back in on of off */
    BDSP_eTsmBool         eTsmEnable;   /* if the tsm is enable or not*/
    BDSP_eTsmBool         eTsmLogEnable;  /*if the tsm log is enable or not */
    BDSP_eTsmBool         eAstmEnable;  /* if the  Adaptive System Time Management(ASTM) enable or not */

}BDSP_AudioTaskTsmSettings;


/*
    This data structure defines IDS, DS and TSM configuration parameters
*/

typedef struct BDSP_P_Audio_FrameSyncTsmConfigParams
{
    /*
    * Data sync configuration parameters
    */
    BDSP_AudioTaskDatasyncSettings  sFrameSyncConfigParams;

    /*
    * TSM configuration parameters
    */
    BDSP_AudioTaskTsmSettings       sTsmConfigParams;

} BDSP_P_Audio_FrameSyncTsmConfigParams;


#endif /*BDSP_COMMON_FW_SETTINGS_H*/
