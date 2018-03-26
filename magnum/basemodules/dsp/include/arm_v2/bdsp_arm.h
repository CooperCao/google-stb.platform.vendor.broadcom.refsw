/******************************************************************************
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
 *****************************************************************************/
#ifndef BDSP_ARM_H_
#define BDSP_ARM_H_

#include "bchp.h"
#include "bint.h"
#include "bbox.h"
#include "bmma.h"
#include "btmr.h"
#include "bimg.h"
#include "btee.h"
#include "bdsp_task.h"
#include "bdsp_common.h"

#define BDSP_ARM_ADDRESS_ALIGN_CDB  12 /* CDB is aligned to 2^12 Bytes*/
#define BDSP_ARM_ADDRESS_ALIGN_ITB  12 /* ITB is aligned to 2^12 Bytes*/

#define BDSP_ARM_ADDRESS_ALIGN      4096
#define BDSP_ARM_SIZE_ALIGN(x)  	\
	{                               \
		unsigned power = 2;         \
		x = (((x+4095)/4096)*4096);\
		while (x >>= 1) power <<= 1;\
		x=power;                    \
	}

#define BDSP_ARM_MAX_DSP 1
#define BDSP_ARM_MAX_CORE_PER_DSP 1

/***************************************************************************
Summary:
Arm DSP Settings
***************************************************************************/
typedef struct BDSP_ArmSettings
{
    bool authenticationEnabled; /* If authentication is enabled, all the firmware execs needs to be downloaded at open time and
                                   the DSP must be booted via a separate call to BDSP_Arm_Boot() after HSM has been authenticated. */
	BDSP_DebugTypeSettings debugSettings[BDSP_DebugType_eLast]; /* Debug information for the different types of debug logs */
    const BIMG_Interface *pImageInterface;      /* Interface to access firmware image. This interface must be
                                                   implemented and the function pointers must be stored here. */
    void **pImageContext;                       /* Context for the Image Interface. This is also provided by
                                                   the implementation of the Image interface */
    unsigned NumDevices;    /*Number of DSP supported in the System, currently used for estimation of Memory*/
    bool preloadImages;         /* If true, all firmware images will be loaded on startup.  Default=false. */
    unsigned maxAlgorithms[BDSP_AlgorithmType_eMax] ;
    BTEE_InstanceHandle hBteeInstance;
} BDSP_ArmSettings;

/***************************************************************************
Summary:
Use case scenario provided by APE
***************************************************************************/
typedef struct BDSP_ArmUsageOptions
{
    bool           Codeclist[BDSP_Algorithm_eMax];  /* Total list containing the Codecs enabled or disabled */
    BDSP_AudioDolbyCodecVersion DolbyCodecVersion;
    BDSP_DataType IntertaskBufferDataType;
    unsigned NumAudioDecoders;
    unsigned NumAudioPostProcesses;
    unsigned NumAudioEncoders;
    unsigned NumAudioMixers;
    unsigned NumAudioPassthru;
    unsigned NumAudioEchocancellers;
    unsigned NumVideoDecoders;
    unsigned NumVideoEncoders;
} BDSP_ArmUsageOptions;

void BDSP_Arm_GetDefaultSettings(
    BDSP_ArmSettings *pSettings     /* [out] */
);
BERR_Code BDSP_Arm_Open(
    BDSP_Handle             *pDsp,     /* [out] */
    BCHP_Handle              chpHandle,
    BREG_Handle              regHandle,
    BMMA_Heap_Handle         memHandle,
    BINT_Handle              intHandle,
    BTMR_Handle              tmrHandle,
    const BDSP_ArmSettings  *pSettings
);

#endif /*BDSP_ARM_H_*/
