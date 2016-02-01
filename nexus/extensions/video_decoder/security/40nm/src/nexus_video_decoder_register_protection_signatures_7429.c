/******************************************************************************
 *    (c)2011-2013 Broadcom Corporation
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
		0x1C, 0x7C, 0x72, 0xAD,
		0xAF, 0x67, 0x1C, 0xDA,
		0x04, 0x3C, 0xEA, 0x63,
		0x0D, 0x11, 0xB7, 0xBA,
		0x0A, 0x14, 0x5D, 0x37,
		0xD3, 0xC9, 0xD9, 0x7F,
		0x1E, 0x20, 0xC2, 0x98,
		0x70, 0x7B, 0xF8, 0x5A
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
		0x6D, 0xC2, 0xBC, 0xC9,
		0xDD, 0xD7, 0x00, 0x1B,
		0x71, 0x61, 0xAC, 0x03,
		0x8D, 0x21, 0x20, 0xAC,
		0x20, 0xA3, 0x22, 0xF4,
		0xB9, 0x1F, 0xA5, 0x7F,
		0x4D, 0x89, 0x63, 0x24,
		0x37, 0x4B, 0x86, 0xFC
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
		0x62, 0xB2, 0xD5, 0x55,
		0xAE, 0x92, 0x30, 0xC4,
		0xC1, 0x8F, 0xD0, 0x83,
		0x35, 0x62, 0xD7, 0xC2,
		0x08, 0xA7, 0x46, 0xBF,
		0x4C, 0x31, 0x09, 0xD2,
		0x32, 0x2E, 0x10, 0x35,
		0xAC, 0x2A, 0x5D, 0xF1

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
