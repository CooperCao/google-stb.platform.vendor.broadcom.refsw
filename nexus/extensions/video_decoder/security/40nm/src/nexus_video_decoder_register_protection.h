/******************************************************************************
 *    (c)2011 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * 
 * Module Description:
 * 
 * Revision History:
 * 
 * $brcm_Log: $
 * 
 *****************************************************************************/

#ifndef NEXUS_VIDEO_DECODER_REGISTER_PROTECTION_H__
#define NEXUS_VIDEO_DECODER_REGISTER_PROTECTION_H__


#ifdef __cplusplus
extern "C" {
#endif


typedef struct AVDRegRegion
{
	unsigned int startPhysicalAddress;
	unsigned int endPhysicalAddress;
} AVDRegRegion;

typedef struct AVDRegAddrValue
{
	unsigned int regPhysicalAddress;
	unsigned int regValue;
} AVDRegAddrValue;

#define NEXUS_VIDEO_REG_PROTECTION_NUM_START_AVD_REGISTERS 3
#define NEXUS_VIDEO_REG_PROTECTION_NUM_RESET_AVD1_REGISTERS 2
#ifndef NEXUS_VIDEO_DECODER_SECURITY_NO_BLD
#define NEXUS_VIDEO_REG_PROTECTION_NUM_RESET_AVD0_REGISTERS 6
#else
#define NEXUS_VIDEO_REG_PROTECTION_NUM_RESET_AVD0_REGISTERS 4
#endif

typedef struct NEXUS_VideoDecoderRegisterProtectionConfig
{
    unsigned int index;
}   NEXUS_VideoDecoderRegisterProtectionConfig;

BERR_Code NEXUS_VideoDecoder_P_EnableRegisterProtection (
    const NEXUS_VideoDecoderRegisterProtectionConfig *pConfig
    );

BERR_Code NEXUS_VideoDecoder_P_SecureStart(
    const NEXUS_VideoDecoderRegisterProtectionConfig *pConfig
    );

BERR_Code NEXUS_VideoDecoder_P_SecureReset(
    const NEXUS_VideoDecoderRegisterProtectionConfig *pConfig
    );

BERR_Code NEXUS_VideoDecoder_P_AVDResetCallback (void * pPrivateData);

#ifdef __cplusplus
}
#endif

#endif	/*NEXUS_VIDEO_DECODER_REGISTER_PROTECTION_H__*/
