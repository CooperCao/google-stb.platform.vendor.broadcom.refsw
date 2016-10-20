/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#if (NEXUS_NUM_XVD_DEVICES)

static const unsigned char gAVD_SetVichSignature[NEXUS_NUM_XVD_DEVICES][NEXUS_HMACSHA256_SIGNATURE_SIZE] =
{
	{
		0xa7, 0x3b, 0xb3, 0x8a,
		0x9d, 0x40, 0x1d, 0xde,
		0x5c, 0x6c, 0x56, 0x45,
		0x8b, 0x6e, 0x8f, 0xd0,
		0x6a, 0xf7, 0x1c, 0x1f,
		0xa1, 0x8a, 0x3e, 0x02,
		0x80, 0x59, 0xb8, 0xdb,
		0x38, 0x74, 0x4f, 0xfe
	}
#if (NEXUS_NUM_XVD_DEVICES>1)
	,
	{
		0x0f, 0x4a, 0x32, 0x9c,
		0x69, 0x18, 0xcc, 0x5b,
		0x3a, 0x51, 0x50, 0xa1,
		0x0e, 0x4c, 0x24, 0x41,
		0x11, 0x95, 0xdd, 0x0a,
		0x03, 0xed, 0xed, 0x8a,
		0x64, 0x38, 0x96, 0x1d,
		0xe0, 0x62, 0xb4, 0x0e
	}
#endif
};



static const unsigned char gAVD_StartAvdSignature[NEXUS_NUM_XVD_DEVICES][NEXUS_HMACSHA256_SIGNATURE_SIZE] =
{
	{
		0xab, 0xc7, 0x6e, 0xd2,
		0x0b, 0xe9, 0x94, 0x5e,
		0xc9, 0x2e, 0x94, 0xf9,
		0x88, 0x38, 0x87, 0x0a,
		0xe3, 0x83, 0xc2, 0xe3,
		0x41, 0xb8, 0xe8, 0x8f,
		0x83, 0xc8, 0x3c, 0x55,
		0x8c, 0x8e, 0x04, 0x1e
	}
#if (NEXUS_NUM_XVD_DEVICES>1)
	,
	{
		0x21, 0x74, 0xe2, 0x54,
		0xca, 0xee, 0x5f, 0x09,
		0xee, 0x12, 0xdb, 0xd6,
		0xf9, 0x77, 0x32, 0xdb,
		0x8b, 0x46, 0xe6, 0x21,
		0x73, 0x3d, 0xc1, 0xfc,
		0x9e, 0x63, 0xbb, 0x18,
		0x7a, 0x1b, 0x20, 0x50
	}
#endif
};



static const unsigned char gAVD_ResetAvdSignature[NEXUS_NUM_XVD_DEVICES][NEXUS_HMACSHA256_SIGNATURE_SIZE] =
{
	{
		0x30, 0x23, 0xC4, 0x60,
		0xA2, 0x80, 0x69, 0xE7,
		0x8B, 0x17, 0xC8, 0x3C,
		0xD3, 0x82, 0x5C, 0x17,
		0x78, 0x87, 0x9E, 0xE3,
		0xC4, 0xCD, 0x81, 0xAB,
		0x5C, 0x5E, 0xE1, 0xE7,
		0x2B, 0xA8, 0x9B, 0x29
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
