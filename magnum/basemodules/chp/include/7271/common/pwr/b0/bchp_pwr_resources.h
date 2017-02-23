 /******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

 ******************************************************************************/

/***************************************************************************
*
* This file is auto-generated
*
* This file contains a list of power resource IDs that can be
* acquired and released.
*
***************************************************************************/

#ifndef BCHP_PWR_RESOURCES_H__
#define BCHP_PWR_RESOURCES_H__

#define BCHP_PWR_RESOURCE_AIO_CLK              0x00000001
#define BCHP_PWR_RESOURCE_AIO_SRAM             0x00000002
#define BCHP_PWR_RESOURCE_AUD_AIO              0x00000003
#define BCHP_PWR_RESOURCE_AUD_DAC              0x00000004
#define BCHP_PWR_RESOURCE_AUD_PLL0             0x00000005
#define BCHP_PWR_RESOURCE_AUD_PLL1             0x00000006
#define BCHP_PWR_RESOURCE_AVD                  0x00000007
#define BCHP_PWR_RESOURCE_AVD0                 0x00000008
#define BCHP_PWR_RESOURCE_AVD0_CLK             0x00000009
#define BCHP_PWR_RESOURCE_AVD0_PWR             0x0000000a
#define BCHP_PWR_RESOURCE_BINT_OPEN            0x0000000b
#define BCHP_PWR_RESOURCE_BVN                  0x0000000c
#define BCHP_PWR_RESOURCE_BVN_SRAM             0x0000000d
#define BCHP_PWR_RESOURCE_DMA                  0x0000000e
#define BCHP_PWR_RESOURCE_GRAPHICS3D           0x0000000f
#define BCHP_PWR_RESOURCE_GRAPHICS3D_PLL_CH    0x00000010
#define BCHP_PWR_RESOURCE_HDMI_RX0_CLK         0x00000011
#define BCHP_PWR_RESOURCE_HDMI_RX0_PHY         0x00000012
#define BCHP_PWR_RESOURCE_HDMI_RX0_SRAM        0x00000013
#define BCHP_PWR_RESOURCE_HDMI_TX0_CLK         0x00000014
#define BCHP_PWR_RESOURCE_HDMI_TX0_PHY         0x00000015
#define BCHP_PWR_RESOURCE_HSM                  0x00000016
#define BCHP_PWR_RESOURCE_M2MC                 0x00000017
#define BCHP_PWR_RESOURCE_M2MC0                0x00000018
#define BCHP_PWR_RESOURCE_M2MC0_SRAM           0x00000019
#define BCHP_PWR_RESOURCE_M2MC_SRAM            0x0000001a
#define BCHP_PWR_RESOURCE_MAGNUM_CONTROLLED    0x0000001b
#define BCHP_PWR_RESOURCE_RAAGA                0x0000001c
#define BCHP_PWR_RESOURCE_RAAGA0_CLK           0x0000001d
#define BCHP_PWR_RESOURCE_RAAGA0_DSP           0x0000001e
#define BCHP_PWR_RESOURCE_RAAGA0_SRAM          0x0000001f
#define BCHP_PWR_RESOURCE_SECURE_ACCESS        0x00000020
#define BCHP_PWR_RESOURCE_SID                  0x00000021
#define BCHP_PWR_RESOURCE_SID_SRAM             0x00000022
#define BCHP_PWR_RESOURCE_SMARTCARD0           0x00000023
#define BCHP_PWR_RESOURCE_SMARTCARD1           0x00000024
#define BCHP_PWR_RESOURCE_VDC                  0x00000025
#define BCHP_PWR_RESOURCE_VDC_656_OUT          0x00000026
#define BCHP_PWR_RESOURCE_VDC_DAC              0x00000027
#define BCHP_PWR_RESOURCE_VDC_HDMI_RX_CLK0     0x00000028
#define BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY0     0x00000029
#define BCHP_PWR_RESOURCE_VDC_STG0             0x0000002a
#define BCHP_PWR_RESOURCE_VDC_VEC              0x0000002b
#define BCHP_PWR_RESOURCE_VDC_VEC_SRAM         0x0000002c
#define BCHP_PWR_RESOURCE_VIP                  0x0000002d
#define BCHP_PWR_RESOURCE_VIP_SRAM             0x0000002e
#define BCHP_PWR_RESOURCE_XPT                  0x0000002f
#define BCHP_PWR_RESOURCE_XPT_PACKETSUB        0x00000030
#define BCHP_PWR_RESOURCE_XPT_PARSER           0x00000031
#define BCHP_PWR_RESOURCE_XPT_PLAYBACK         0x00000032
#define BCHP_PWR_RESOURCE_XPT_RAVE             0x00000033
#define BCHP_PWR_RESOURCE_XPT_REMUX            0x00000034
#define BCHP_PWR_RESOURCE_XPT_WAKEUP           0x00000035
#define BCHP_PWR_RESOURCE_XPT_XMEMIF           0x00000036

#endif
