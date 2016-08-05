/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * THE SOURCE CODE CONTAINS BROADCOM HIGHLY CONFIDENTIAL
 * INFORMATION AND MUST BE HANDLED AS AGREED UPON IN THE
 * HIGHLY CONFIDENTIAL SOFTWARE LICENSE AGREEMENT (HC-SLA).
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
 *****************************************************************************/


#if (NEXUS_NUM_XVD_DEVICES)

static const unsigned char gAVD_SetVichSignature[NEXUS_NUM_XVD_DEVICES][NEXUS_HMACSHA256_SIGNATURE_SIZE] =
{
	{
        0x37, 0x31, 0x4a, 0xc4,
        0xf7, 0x72, 0xf9, 0xaf,
        0x8a, 0x31, 0x96, 0x08,
        0x39, 0x78, 0x8e, 0xe3,
        0x5d, 0xb7, 0xae, 0xbe,
        0x67, 0x90, 0x21, 0xf3,
        0x74, 0x1f, 0xa6, 0x61,
        0x9c, 0x3e, 0x31, 0x43
	}
};



static const unsigned char gAVD_StartAvdSignature[NEXUS_NUM_XVD_DEVICES][NEXUS_HMACSHA256_SIGNATURE_SIZE] =
{
	{
        0x06, 0xd1, 0x47, 0x31,
        0x25, 0xa3, 0x23, 0x4d,
        0xd4, 0xe5, 0x2d, 0x38,
        0xba, 0x2b, 0xc2, 0x90,
        0xad, 0xde, 0x4e, 0xc9,
        0xed, 0x3c, 0x7a, 0x12,
        0xe9, 0xfe, 0x68, 0xbd,
        0x1d, 0x8b, 0x78, 0x8e
	}
};



static const unsigned char gAVD_ResetAvdSignature[NEXUS_NUM_XVD_DEVICES][NEXUS_HMACSHA256_SIGNATURE_SIZE] =
{
	{ /* the signature is not correct, but is also not used. */
        0x30, 0x23, 0xC4, 0x60,
        0xA2, 0x80, 0x69, 0xE7,
        0x8B, 0x17, 0xC8, 0x3C,
        0xD3, 0x82, 0x5C, 0x17,
        0x78, 0x87, 0x9E, 0xE3,
        0xC4, 0xCD, 0x81, 0xAB,
        0x5C, 0x5E, 0xE1, 0xE7,
        0x2B, 0xA8, 0x9B, 0x29
	}
};

#endif
