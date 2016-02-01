/******************************************************************************
 *    (c)2011-2012 Broadcom Corporation
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
		0xD4, 0xEB, 0xE1, 0x3A,
		0x73, 0x82, 0x9C, 0x06,
		0xC1, 0x87, 0xC0, 0x65,
		0x7F, 0x13, 0x4F, 0xB4,
		0x7B, 0x65, 0x5F, 0x07,
		0x71, 0xAC, 0x1E, 0xA1,
		0x83, 0x62, 0xBE, 0x48,
		0x08, 0x63, 0x95, 0x95
	}
#if (NEXUS_NUM_XVD_DEVICES>1)
	,
	{
		0x08, 0xB9, 0x56, 0x9D,
		0x99, 0x1B, 0xC5, 0x2E,
		0x39, 0xE0, 0x24, 0x0C,
		0x81, 0x85, 0x27, 0x87,
		0x4B, 0x4B, 0xEE, 0x3A,
		0x46, 0x4E, 0xFE, 0xA4,
		0xA5, 0x3C, 0x48, 0xDA,
		0xC7, 0x3A, 0x29, 0x17
	}
#endif
};



static const unsigned char gAVD_StartAvdSignature[NEXUS_NUM_XVD_DEVICES][NEXUS_HMACSHA256_SIGNATURE_SIZE] =
{
	{
		0x2F, 0x46, 0x6F, 0xCC,
		0xDC, 0xE7, 0x60, 0xD1,
		0x17, 0xAA, 0x61, 0x36,
		0x8F, 0x28, 0x71, 0x58,
		0x1D, 0xA2, 0xFE, 0x7B,
		0xEB, 0x58, 0x19, 0x5E,
		0x4F, 0x88, 0x03, 0x52,
		0xB2, 0x4B, 0x26, 0x44
	}
#if (NEXUS_NUM_XVD_DEVICES>1)
	,
	{
		0xFF, 0x9A, 0x6A, 0x79,
		0xBC, 0x31, 0x0C, 0xDB,
		0xA5, 0xA9, 0x80, 0xC1,
		0x2C, 0x83, 0x5A, 0xDB,
		0xB7, 0x99, 0x01, 0x7D,
		0xD9, 0xBF, 0xF9, 0xDD,
		0x5B, 0xE9, 0xA3, 0x53,
		0x8E, 0x18, 0x21, 0x3A
	}
#endif
};



static const unsigned char gAVD_ResetAvdSignature[NEXUS_NUM_XVD_DEVICES][NEXUS_HMACSHA256_SIGNATURE_SIZE] =
{
	{
		0xBE, 0x65, 0x85, 0x21,
		0xD1, 0xD9, 0x6A, 0x7C,
		0xC4, 0x9A, 0xD5, 0x34,
		0xD1, 0x14, 0xFC, 0x22,
		0xD5, 0x20, 0x13, 0xD3,
		0x5F, 0x0E, 0x92, 0xDC,
		0x97, 0x72, 0x00, 0x93,
		0xA5, 0x84, 0x5E, 0x0B
	}
#if (NEXUS_NUM_XVD_DEVICES>1)
	,
	{
		0x25, 0x03, 0xF0, 0x65,
		0x47, 0xC2, 0x16, 0x21,
		0x70, 0x27, 0x02, 0x99,
		0x27, 0xB4, 0x02, 0xF6,
		0xF1, 0xF8, 0x74, 0xEE,
		0x54, 0x59, 0x50, 0x3E,
		0xB6, 0xAA, 0xA8, 0x9D,
		0xBD, 0x04, 0x3A, 0xE4 
	}
#endif
};

#endif
