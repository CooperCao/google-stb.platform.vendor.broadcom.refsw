/******************************************************************************
 *    (c)2013 Broadcom Corporation
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

#if (NEXUS_NUM_VCE_DEVICES)

#include "bchp_common.h"
#include "nexus_bsp_config.h"
#include "nexus_video_encoder_register_protection.h"

#if defined BCHP_VICE2_ARCSS_ESS_CTRL_1_REG_START
    #include "bchp_vice2_arcss_ess_ctrl_1.h"
    #define VICE2_ARCSS_ESS_DEFINED 1
#else
    #define VICE2_ARCSS_ESS_DEFINED 0
#endif



#define VICE_RESGISTER_ADDRESS(x) (0x10000000+x)


static const ViceRegRegion gVice_Regions[NEXUS_NUM_VCE_DEVICES] =
{

#if (NEXUS_NUM_VCE_DEVICES==1)
  #if (VICE2_ARCSS_ESS_DEFINED)
     {
        VICE_RESGISTER_ADDRESS(BCHP_VICE2_ARCSS_ESS_CTRL_1_INIT_SYS_HOST_IF+4), /* 0x10750004 on 7425 */
        VICE_RESGISTER_ADDRESS(BCHP_VICE2_ARCSS_ESS_CTRL_1_INIT_SYS_HOST_IF+7)  /* 0x10750007 on 7425 */
     }
  #else
    {
        0,
        0
    }
  #endif
#elif (NEXUS_NUM_VCE_DEVICES==2)
	{
		0,
		0
	}
#endif
};


#if 0

static const ViceRegAddrValue gStartViceRegAddrValues[NEXUS_NUM_VCE_DEVICES][NEXUS_VIDEO_REG_PROTECTION_NUM_START_Vice_REGISTERS] =
{
	{
		{
			VICE_RESGISTER_ADDRESS(BCHP_DECODE_IND_SDRAM_REGS2_0_REG_CPU_DBG), /* 0x10051010 */
			0x00000001,

		},
		{
			VICE_RESGISTER_ADDRESS(BCHP_DECODE_CPUAUX2_0_CPUAUX_REG+0x18), /* 0x10055018 */
			0x00000000,

		},

		{
			VICE_RESGISTER_ADDRESS(BCHP_DECODE_CPUAUX2_0_CPUAUX_REG+0x28), /* 0x10055028 */
			0x00000000,
		}
	}

};


static const ViceRegAddrValue gResetViceRegAddrValues[NEXUS_NUM_VCE_DEVICES][NEXUS_VIDEO_REG_PROTECTION_NUM_RESET_Vice0_REGISTERS] =
{
	{
		{
			VICE_RESGISTER_ADDRESS(BCHP_DECODE_IND_SDRAM_REGS_0_REG_CPU_DBG),
			0x00000001,

		},
		{
			VICE_RESGISTER_ADDRESS(BCHP_DECODE_CPUAUX_0_CPUAUX_REG+0x28),
			0x00000001,

		},
		{
			VICE_RESGISTER_ADDRESS(BCHP_DECODE_IND_SDRAM_REGS2_0_REG_CPU_DBG),
			0x00000001,

		},
		{
			VICE_RESGISTER_ADDRESS(BCHP_DECODE_CPUAUX2_0_CPUAUX_REG+0x28),
			0x00000001,

		},

	}

};

#endif

#endif
