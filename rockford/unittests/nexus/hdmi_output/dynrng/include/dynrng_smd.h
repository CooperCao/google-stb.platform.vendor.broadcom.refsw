/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 * Module Description:
 *
******************************************************************************/

#ifndef DYNRNG_SMD_H__
#define DYNRNG_SMD_H__

#include "nexus_hdmi_types.h"

typedef struct SMD_Smd * SMD_SmdHandle;

typedef enum SMD_SmdSource
{
    SMD_SmdSource_eZero,
    SMD_SmdSource_eBt2020,
    SMD_SmdSource_eBt709,
    SMD_SmdSource_eInput,
    SMD_SmdSource_eFile,
    SMD_SmdSource_eUser,
    SMD_SmdSource_eMax
} SMD_SmdSource;

#define SMD_TO_SMPTE_ST2086(X) ((X)/0.00002)

void SMD_Destroy(SMD_SmdHandle smd);
SMD_SmdHandle SMD_Create(void);
int SMD_SetFilePath(SMD_SmdHandle smd, const char * path);
int SMD_GetMetadata(SMD_SmdHandle smd, NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pMetadata);
void SMD_GetBt709Metadata(SMD_SmdHandle smd, NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pMetadata);
void SMD_GetBt2020Metadata(SMD_SmdHandle smd, NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pMetadata);
void SMD_GetFileMetadata(SMD_SmdHandle smd, NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pMetadata);
void SMD_GetInputMetadata(SMD_SmdHandle smd, NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pMetadata);
int SMD_SetInputMetadata(SMD_SmdHandle smd, const NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pMetadata);
void SMD_GetUserMetadata(SMD_SmdHandle smd, NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pMetadata);
int SMD_SetUserMetadata(SMD_SmdHandle smd, const NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pMetadata);
int SMD_SetSmdSource(SMD_SmdHandle smd, SMD_SmdSource source);
SMD_SmdSource SMD_GetSmdSource(SMD_SmdHandle smd);
const char * SMD_GetSmdSourceName(SMD_SmdSource source);
SMD_SmdSource SMD_ParseSmdSource(const char * name);
void SMD_PrintMetadata(const char * source, const NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pMetadata);
void SMD_Print(SMD_SmdHandle smd);
#endif /* DYNRNG_SMD_H__ */
