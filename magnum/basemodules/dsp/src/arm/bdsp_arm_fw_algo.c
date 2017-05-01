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

#include "bdsp_arm_priv_include.h"
#include "bdsp_arm_img_sizes.h"

#define BDSP_AF_P_EXTRA_SAMPLES (8) /* used to put some extra buffer */

BDBG_MODULE(bdsp_arm_fw_algo);

const uint32_t BDSP_ARM_SystemID_MemoryReqd[BDSP_ARM_SystemImgId_eMax] = {
    BDSP_ARM_IMG_SYSTEM_CODE_SIZE, /*BDSP_SystemImgId_eSystemCode*/

};

const char * BDSP_ARM_AlgoFileName[BDSP_ARM_AF_P_AlgoId_eMax] =
{
    "/DdpEncoder.so",  /*  BDSP_ARM_AF_P_AlgoId_eDDPEncode */
    "/DdpEncoderFrameSync.so" /*BDSP_ARM_AF_P_AlgoId_eDDPEncFrameSync */
};

const BDSP_Arm_P_AlgorithmInfo BDSP_ARM_sAlgorithmInfo[] =
{
    {
        /* Algorithm */                   /* Type */                   /* Name */   /* Supported */
        BDSP_Algorithm_eDDPEncode, BDSP_AlgorithmType_eAudioEncode, "DDP ARM Encode",      true,
        /* Default User Config */           /* User config size */
        &BDSP_ARM_sDefDDPEncConfigSettings, sizeof(BDSP_Arm_Audio_DDPEncConfigParams),
        /* Stream Info Size */             /* Valid offset */
        0,                                      0xffffffff,
        /* Default IDS Config */            /* IDS config size */
        &BDSP_ARM_sDefaultFrameSyncTsmSettings, sizeof(BDSP_ARM_sDefaultFrameSyncTsmSettings),
        /* FW Algorithms Required */
        {
            2,  /* Number of nodes in DDP Encode */
            {
                BDSP_ARM_AF_P_AlgoId_eDDPEncFrameSync,
                BDSP_ARM_AF_P_AlgoId_eDDPEncode,
                BDSP_ARM_AF_P_AlgoId_eInvalid,
                BDSP_ARM_AF_P_AlgoId_eInvalid,
                BDSP_ARM_AF_P_AlgoId_eInvalid,
                BDSP_ARM_AF_P_AlgoId_eInvalid
            },
        }
    },

    /* This entry must always be last */
    {
        BDSP_Algorithm_eMax, BDSP_AlgorithmType_eMax, "Invalid", false,
        NULL, 0,
        0, 0xffffffff,
        NULL, 0,
        {
            0,
            {
                BDSP_ARM_AF_P_AlgoId_eInvalid,
                BDSP_ARM_AF_P_AlgoId_eInvalid,
                BDSP_ARM_AF_P_AlgoId_eInvalid,
                BDSP_ARM_AF_P_AlgoId_eInvalid,
                BDSP_ARM_AF_P_AlgoId_eInvalid,
                BDSP_ARM_AF_P_AlgoId_eInvalid
            },
        }
    },
};

const BDSP_AF_P_sNODE_INFO BDSP_ARM_sNodeInfo[BDSP_ARM_AF_P_AlgoId_eMax+1] =
{
    /*  BDSP_ARM_sNodeInfo[BDSP_ARM_AF_P_AlgoId_eDDPEncode] =   */
    /*{ 0,0,0,0,0,0,0,0,0,0,0,0 },*/
    {
        BDSP_ARM_IMG_DDP_ENCODE_CODE_SIZE,              /* ui32CodeSize */
        BDSP_ARM_IMG_DDP_ENCODE_TABLES_SIZE,            /* ui32RomTableSize */
        BDSP_ARM_IMG_DDP_ENCODE_INTER_FRAME_SIZE,       /* ui32InterFrameBuffSize */
        (1536+BDSP_AF_P_EXTRA_SAMPLES)*4*8,             /* ui32InterStageIoBuffSize */
        20000,                                          /* ui32InterStageGenericBuffSize */
        240000,                                         /* ui32ScratchBuffSize */
        sizeof(BDSP_Arm_Audio_DDPEncConfigParams),  /* ui32UserCfgBuffSize */
        (1536+BDSP_AF_P_EXTRA_SAMPLES)*4,               /* ui32MaxSizePerChan */
        8,                                              /* ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /* eInterFrameBuffType */
        BDSP_AF_P_FwStatus_eAbsent,                     /* eFwStatusBuffType */
        100
    },

    /*  BDSP_ARM_sNodeInfo[BDSP_ARM_AF_P_AlgoId_eDDPEncFrameSync] = */
    /*{ 0,0,0,0,0,0,0,0,0,0,0,0 },*/
    {
        0,                                              /*  ui32CodeSize */
        0,                                              /*  ui32RomTableSize */
        0,                                              /*  ui32InterFrameBuffSize */
        100,                                            /*  ui32InterStageIoBuffSize */
        20000,                                          /*  ui32InterStageGenericBuffSize */
        100,                                                /*  ui32ScratchBuffSize */
        sizeof(BDSP_P_Audio_FrameSyncTsmConfigParams),    /*  ui32UserCfgBuffSize */
        0,                                              /*  ui32MaxSizePerChan */
        2,                                              /*  ui32MaxNumChansSupported */
        BDSP_AF_P_InterFrameBuffType_ePresent,          /*  eInterFrameBuffType */
        BDSP_AF_P_FwStatus_ePresent,                    /*  eFwStatusBuffType */
        100                                             /*  FwStatusBuffSize */
    },

    /*  BDSP_ARM_sNodeInfo[BDSP_ARM_AF_P_AlgoId_eEndOfEncFsAlgos] = */
    { 0,0,0,0,0,0,0,0,0,0,0,0 },
};

static const BDSP_Arm_P_AlgorithmSupportInfo BDSP_Arm_sAlgorithmSupportInfo[]=
{
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
        }
    },
};
const BDSP_Arm_P_AlgorithmSupportInfo *BDSP_Arm_P_LookupAlgorithmSupported(
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
    for ( i = 0; BDSP_Arm_sAlgorithmSupportInfo[i].algorithm != BDSP_Algorithm_eMax; i++ )
    {
        if (BDSP_Arm_sAlgorithmSupportInfo[i].algorithm == algorithm)
        {
            if(BDSP_Arm_sAlgorithmSupportInfo[i].DolbyLicensePresent == false)
            {
                break;
            }
            else if(DolbyCodecVersion == BDSP_Arm_sAlgorithmSupportInfo[i].DolbyCodecVersion)
            {
                break;
            }
            else
            {
                /*Continue the loop */
            }
        }
    }
    return &BDSP_Arm_sAlgorithmSupportInfo[i];
}


const BDSP_Arm_P_AlgorithmInfo *BDSP_Arm_P_LookupAlgorithmInfo_isrsafe(
    BDSP_Algorithm algorithm
    )
{
    unsigned i;

    for ( i = 0; BDSP_ARM_sAlgorithmInfo[i].algorithm != BDSP_Algorithm_eMax; i++ )
    {
        if ( BDSP_ARM_sAlgorithmInfo[i].algorithm == algorithm )
        {
            break;
        }
    }

    return &BDSP_ARM_sAlgorithmInfo[i];
}

BERR_Code BDSP_Arm_P_GetFWSize (
    const BIMG_Interface *iface,
    void *context,
    unsigned firmware_id,
    uint32_t *size
    )
{
    void *image;
    BERR_Code rc;
    const void *data;

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

    *size =((uint32_t *) data)[0];

    iface->close(image);
    return rc;

}


bool BDSP_Arm_P_AlgoHasTables(
    BDSP_ARM_AF_P_AlgoId algorithm
    )
{

/*	BDBG_CASSERT(BDSP_ARM_AF_P_AlgoId_eEndOfEncodeAlgos < BDSP_ARM_AF_P_AlgoId_eEndOfEncFsAlgos);*/


	if ( algorithm <= BDSP_ARM_AF_P_AlgoId_eAudioAlgoEndIdx )
    {
        /* All encoders have tables */
        return true;
    }
#if 0
	else if ( algorithm <= BDSP_ARM_AF_P_AlgoId_eEndOfEncFsAlgos )
    {
        /* Framesync does not have tables */
        return false;
    }
#endif
    else
    {
        /* Remaining are framesync */
        return false;
    }
}
