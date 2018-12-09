/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef NEXUS_AUDIO_IMAGE_PRIV_H__
#define NEXUS_AUDIO_IMAGE_PRIV_H__
#include "bimg.h"

typedef enum NEXUS_AudioImage
{
    NEXUS_AudioImage_ePak,
    NEXUS_AudioImage_eDrm,
    NEXUS_AudioImage_ePakDev,
    NEXUS_AudioImage_eDrmDev,
    NEXUS_AudioImage_ePakDev_72501,
    NEXUS_AudioImage_ePakDev_72502,
    NEXUS_AudioImage_ePakDev_7250,
    NEXUS_AudioImage_ePakDev_72511,
    NEXUS_AudioImage_ePakDev_72521,
    NEXUS_AudioImage_ePakDev_72525,
    NEXUS_AudioImage_ePakDev_72603,
    NEXUS_AudioImage_ePakDev_72604,
    NEXUS_AudioImage_ePakDev_7260,
    NEXUS_AudioImage_ePakDev_7268,
    NEXUS_AudioImage_ePakDev_7271,
    NEXUS_AudioImage_ePakDev_73649,
    NEXUS_AudioImage_ePakDev_7364,
    NEXUS_AudioImage_ePakDev_7366,
    NEXUS_AudioImage_ePakDev_7367,
    NEXUS_AudioImage_ePakDev_74381,
    NEXUS_AudioImage_ePakDev_7444,
    NEXUS_AudioImage_ePakDev_7445,
    NEXUS_AudioImage_ePakDev_74481,
    NEXUS_AudioImage_ePakDev_74491,
    NEXUS_AudioImage_ePakDev_74495,
    NEXUS_AudioImage_eMax
} NEXUS_AudioImage;

extern BIMG_Interface NEXUS_AUDIO_IMG_Interface;
extern void *NEXUS_AUDIO_IMG_Context;

#endif /* NEXUS_AUDIO_IMAGE_PRIV_H__ */
/* End of file. */
