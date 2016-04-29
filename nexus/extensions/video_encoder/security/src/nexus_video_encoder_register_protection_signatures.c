/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#if (NEXUS_NUM_VCE_DEVICES)

static const unsigned char gViceSetVichSignature[NEXUS_NUM_VCE_DEVICES][NEXUS_HMACSHA256_SIGNATURE_SIZE] =
{
#if (BCHP_CHIP == 7445)
	{
		0xaa, 0x8c, 0xd7, 0x26,
		0x7d, 0xd6, 0x64, 0xe9,
		0x13, 0xad, 0x7b, 0xc0,
		0x95, 0xdd, 0xa7, 0x04,
		0x71, 0xc5, 0x87, 0x50,
		0x40, 0x16, 0x2a, 0x2b,
		0x23, 0x78, 0xd4, 0x84,
		0x27, 0x18, 0xbe, 0x5f
	}
#else
	{
		0x87, 0x40, 0x5f, 0xfa,
		0x51, 0x37, 0x74, 0x08,
		0x2c, 0xc0, 0x30, 0x02,
		0x11, 0x43, 0x19, 0x00,
		0x33, 0x54, 0xe5, 0xb3,
		0xf5, 0x26, 0x8d, 0x58,
		0x53, 0xd3, 0xfe, 0x7e,
		0xfb, 0x66, 0xdf, 0xf4
	}
#endif
};



static const unsigned char gViceStartAvdSignature[NEXUS_NUM_VCE_DEVICES][NEXUS_HMACSHA256_SIGNATURE_SIZE] =
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

};



static const unsigned char gViceResetAvdSignature[NEXUS_NUM_VCE_DEVICES][NEXUS_HMACSHA256_SIGNATURE_SIZE] =
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

};

#endif
