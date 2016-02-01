/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/



#ifndef BCHP_PWR_RESOURCES_PRIV_H__
#define BCHP_PWR_RESOURCES_PRIV_H__

#include "bchp_pwr.h"

/* Private power resource IDs */
#define BCHP_PWR_HW_AVD0_CLK        0xff000001
#define BCHP_PWR_HW_AVD0_PWR        0xff000002
#define BCHP_PWR_HW_VEC_AIO         0xff000003
#define BCHP_PWR_HW_RAAGA0_CLK      0xff000004
#define BCHP_PWR_HW_RAAGA0_DSP      0xff000005
#define BCHP_PWR_HW_RAAGA0_SRAM     0xff000006
#define BCHP_PWR_HW_HDMI_TX_CLK     0xff000007
#define BCHP_PWR_HW_BVN             0xff000008
#define BCHP_PWR_HW_BVN_108M        0xff000009
#define BCHP_PWR_HW_BVN_SRAM        0xff00000a
#define BCHP_PWR_HW_VDC_DAC         0xff00000b
#define BCHP_PWR_HW_VEC_SRAM        0xff00000c
#define BCHP_PWR_HW_XPT_108M        0xff00000d
#define BCHP_PWR_HW_XPT_XMEMIF      0xff00000e
#define BCHP_PWR_HW_XPT_RMX         0xff00000f
#define BCHP_PWR_HW_XPT_SRAM        0xff000010
#define BCHP_PWR_HW_XPT_WAKEUP      0xff000011
#define BCHP_PWR_HW_HDMI_TX_SRAM    0xff000012
#define BCHP_PWR_HW_HDMI_TX_108M    0xff000013
#define BCHP_PWR_HW_HDMI_TX_CEC     0xff000014
#define BCHP_PWR_HW_M2MC            0xff000015
#define BCHP_PWR_HW_GFX_SRAM        0xff000016
#define BCHP_PWR_HW_GFX_108M        0xff000017
#define BCHP_PWR_HW_DMA             0xff000018
#define BCHP_PWR_HW_SCD0            0xff000019
#define BCHP_PWR_HW_SCD1            0xff00001a
#define BCHP_PWR_HW_MDM             0xff00001b
#define BCHP_PWR_HW_PLL_AVD_CH1     0xff00001c
#define BCHP_PWR_HW_PLL_AVD_CH2     0xff00001d
#define BCHP_PWR_HW_PLL_AVD_CH3     0xff00001e
#define BCHP_PWR_HW_AUD_PLL0        0xff00001f
#define BCHP_PWR_HW_AUD_PLL1        0xff000020
#define BCHP_PWR_HW_PLL_SCD         0xff000021
#define BCHP_PWR_HW_PLL_VCXO_CH0    0xff000022
#define BCHP_PWR_HW_PLL_SCD_CH0     0xff000023
#define BCHP_PWR_HW_PLL_SCD_CH1     0xff000024
#define BCHP_PWR_HW_PLL_VCXO_CH1    0xff000025
#define BCHP_PWR_HW_PLL_VCXO        0xff000026

/* This is the link between the public and private interface */
void BCHP_PWR_P_HW_Control(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, bool activate);
void BCHP_PWR_P_HW_ControlId(BCHP_Handle handle, unsigned id, bool activate);

#define BCHP_PWR_P_NUM_NONLEAFS   38
#define BCHP_PWR_P_NUM_NONLEAFSHW 13
#define BCHP_PWR_P_NUM_LEAFS      25
#define BCHP_PWR_P_NUM_ALLNODES   76

#endif
