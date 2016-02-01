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


#if (NEXUS_NUM_XVD_DEVICES)

static const unsigned char gAVD_SetVichSignature[NEXUS_NUM_XVD_DEVICES][NEXUS_HMACSHA256_SIGNATURE_SIZE] =
{
	{
		0xB9, 0xF6, 0xB2, 0xA9,
		0x4A, 0x56, 0x12, 0xBE,
		0x24, 0xB8, 0x9C, 0x20,
		0x77, 0x01, 0xCC, 0x06,
		0x9B, 0x2A, 0x4E, 0xC7,
		0x11, 0xF1, 0x17, 0x4D,
		0x6A, 0x2E, 0x0C, 0xD6,
		0xFF, 0x50, 0xBF, 0x93
	}
#if (NEXUS_NUM_XVD_DEVICES>1)
	,
	{
		0xD1, 0x16, 0x5B, 0x16,
		0xBB, 0x42, 0x64, 0xDF,
		0x5B, 0x7A, 0x99, 0x02,
		0x81, 0x7E, 0x08, 0xF6,
		0x4E, 0x47, 0x12, 0xBF,
		0xC9, 0xF5, 0x25, 0x04,
		0xA8, 0xE4, 0x2C, 0xD3,
		0x3A, 0x12, 0xD7, 0x8C
	}
#endif
};



static const unsigned char gAVD_StartAvdSignature[NEXUS_NUM_XVD_DEVICES][NEXUS_HMACSHA256_SIGNATURE_SIZE] =
{
	{
		0x82, 0x4B, 0xE8, 0xCA,
		0x05, 0xFE, 0x3C, 0xE8,
		0xA1, 0xDE, 0xE0, 0xFF,
		0xAA, 0xF8, 0x9F, 0xD1,
		0xC2, 0x7C, 0x93, 0x20,
		0xA4, 0x2B, 0xB3, 0xD0,
		0xFB, 0x10, 0x9A, 0x38,
		0x10, 0xDB, 0x1A, 0xF7
	}
#if (NEXUS_NUM_XVD_DEVICES>1)
	,
	{
		0x82, 0x4B, 0xE8, 0xCA,
		0x05, 0xFE, 0x3C, 0xE8,
		0xA1, 0xDE, 0xE0, 0xFF,
		0xAA, 0xF8, 0x9F, 0xD1,
		0xC2, 0x7C, 0x93, 0x20,
		0xA4, 0x2B, 0xB3, 0xD0,
		0xFB, 0x10, 0x9A, 0x38,
		0x10, 0xDB, 0x1A, 0xF7
	}
#endif
};



static const unsigned char gAVD_ResetAvdSignature[NEXUS_NUM_XVD_DEVICES][NEXUS_HMACSHA256_SIGNATURE_SIZE] =
{
	{
		0xB3, 0xCD, 0xC0, 0x57,
		0xD1, 0x54, 0xA2, 0x98,
		0x28, 0x3D, 0x1C, 0x58,
		0x39, 0x9D, 0x83, 0x8B,
		0x55, 0xB8, 0xD9, 0x04,
		0x6E, 0x82, 0xC3, 0xE3,
		0xCA, 0x36, 0x23, 0xD2,
		0x75, 0xF4, 0x54, 0xEA
	}
#if (NEXUS_NUM_XVD_DEVICES>1)
	,
	{
		0xFB, 0x32, 0xAB, 0x4A,
		0x68, 0x5B, 0xF5, 0xC9,
		0xE1, 0x84, 0x19, 0x54,
		0xAB, 0x8C, 0xF1, 0x57,
		0xC9, 0x7E, 0xF9, 0xC7,
		0x25, 0x48, 0x2A, 0x96,
		0x99, 0x01, 0xA2, 0x6A,
		0x5A, 0x39, 0x94, 0xCB 
	}
#endif
};

#endif
