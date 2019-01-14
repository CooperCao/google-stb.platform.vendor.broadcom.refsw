/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include "bdsp_raaga_priv_include.h"

BDBG_MODULE(bdsp_raaga_hw);

/**********************************************************
Function: BDSP_Raaga_P_ResetRaagaCore_isr

Description : Reset's Raaga core.
				It masks watchdog interrupt too.

**********************************************************/
static BERR_Code BDSP_Raaga_P_ResetRaagaCore_isr(
	BDSP_Raaga *pRaaga,
	unsigned uiDspIndex
)
{
	uint32_t regVal = 0, ui32dspOffset =0;
    BDBG_ENTER(BDSP_Raaga_P_ResetRaagaCore_isr);

	ui32dspOffset = pRaaga->dspOffset[uiDspIndex];

    BDSP_WriteReg32(pRaaga->regHandle,
		BCHP_RAAGA_DSP_FP_MISC_0_CORECTRL_CORE_ENABLE+ui32dspOffset,
		0x0);
    BDSP_WriteReg32(pRaaga->regHandle,
		BCHP_RAAGA_DSP_FP_MISC_1_CORECTRL_CORE_ENABLE+ui32dspOffset,
		0x0);
    BDSP_WriteReg32(pRaaga->regHandle,
		BCHP_RAAGA_DSP_MISC_SOFT_INIT+ui32dspOffset,
		0x100);

    /*RDB says no need of Read modify write.*/
    regVal = 0;
    regVal = (BCHP_FIELD_DATA(SUN_TOP_CTRL_SW_INIT_0_SET,    raaga0_sw_init,1));
    BDSP_WriteReg32(pRaaga->regHandle,
		BCHP_SUN_TOP_CTRL_SW_INIT_0_SET+ui32dspOffset,
		regVal);

    BKNI_Delay(2);

    /*RDB says no need of Read modify write.*/
    regVal = 0;
    regVal = (BCHP_FIELD_DATA(SUN_TOP_CTRL_SW_INIT_0_CLEAR, raaga0_sw_init,1));
    BDSP_WriteReg32(pRaaga->regHandle,
		BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR+ui32dspOffset,
		regVal);

	BDBG_LEAVE(BDSP_Raaga_P_ResetRaagaCore_isr);
	return BERR_SUCCESS;
}

/**********************************************************
Function: BDSP_Raaga_P_ResetHardware_isr

Description : To be called inside an isr-
				Reset's Raaga core and restores interrupts
				to the state it was before the reset

**********************************************************/
BERR_Code BDSP_Raaga_P_ResetHardware_isr(
	BDSP_Raaga *pRaaga,
	unsigned    uiDspIndex
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDBG_ENTER(BDSP_Raaga_P_ResetHardware_isr);

	errCode = BDSP_Raaga_P_ResetRaagaCore_isr(pRaaga, uiDspIndex );
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ResetHardware_isr: Error in Reset Hardware for DSP %d",uiDspIndex));
		goto end;
	}

	errCode = BDSP_Raaga_P_RestoreInterrupts_isr((void *)pRaaga, uiDspIndex );
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ResetHardware_isr: Error in Restoring Interrupt for DSP %d",uiDspIndex));
		goto end;
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_ResetHardware_isr);
	return errCode;
}

/**********************************************************
Function: BDSP_Raaga_P_ResetHardware

Description : Reset's Raaga core and restores interrupts
		 to the state it was before the reset

**********************************************************/
static BERR_Code BDSP_Raaga_P_ResetHardware(
	BDSP_Raaga 	*pRaaga,
	unsigned     uiDspIndex
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDBG_ENTER(BDSP_Raaga_P_ResetHardware);

	BKNI_EnterCriticalSection();
	errCode = BDSP_Raaga_P_ResetRaagaCore_isr(pRaaga, uiDspIndex);
	BKNI_LeaveCriticalSection();
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ResetHardware: Error in Reset Hardware for DSP %d",uiDspIndex));
		goto end;
	}

	BKNI_EnterCriticalSection();
	errCode = BDSP_Raaga_P_RestoreInterrupts_isr((void *)pRaaga, uiDspIndex);
	BKNI_LeaveCriticalSection();
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ResetHardware: Error in Restoring Interrupt for DSP %d",uiDspIndex));
		goto end;
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_ResetHardware);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_SetSCBConfig (
	BDSP_Raaga *pDevice,
	unsigned uiDspIndex
)
{
	BERR_Code errCode = BERR_SUCCESS;
	uint32_t regVal =0, ui32dspOffset =0;

	BDBG_ENTER(BDSP_Raaga_P_SetSCBConfig);

	ui32dspOffset = pDevice->dspOffset[uiDspIndex];

	BDBG_MSG(("Programming SCB Registers"));
	BDBG_MSG(("NEED TO PROGRAM FROM MEMORY CONTROLLER"));
	/* These values are currently as per the current proposed values
	   These values shall be programmed based on pDevice->settings.memc[i] values */

        regVal = BDSP_ReadReg32(pDevice->regHandle, BCHP_RAAGA_DSP_MISC_SCB0_BASE_CONFIG+ui32dspOffset);
	BCHP_SET_FIELD_DATA(regVal, RAAGA_DSP_MISC_SCB0_BASE_CONFIG, BASE_START,
	                                     (pDevice->deviceSettings.memoryLayout.memc[0].region[0].addr >> SCB_REG_ADDR_WRITE_SHIFT_VALUE));
	BCHP_SET_FIELD_DATA(regVal, RAAGA_DSP_MISC_SCB0_BASE_CONFIG, BASE_END, ((pDevice->deviceSettings.memoryLayout.memc[0].region[0].addr +
                                             pDevice->deviceSettings.memoryLayout.memc[0].region[0].size) >> SCB_REG_ADDR_WRITE_SHIFT_VALUE));
        BDBG_MSG(("BCHP_RAAGA_DSP_MISC_SCB0_BASE_CONFIG : regVal : 0x%x", regVal));
	BDSP_WriteReg32(pDevice->regHandle, BCHP_RAAGA_DSP_MISC_SCB0_BASE_CONFIG+ui32dspOffset, regVal);

	regVal = BDSP_ReadReg32(pDevice->regHandle, BCHP_RAAGA_DSP_MISC_SCB0_EXT_CONFIG+ui32dspOffset);
	BCHP_SET_FIELD_DATA(regVal, RAAGA_DSP_MISC_SCB0_EXT_CONFIG, EXT_START,
	                                     (pDevice->deviceSettings.memoryLayout.memc[0].region[1].addr >> SCB_REG_ADDR_WRITE_SHIFT_VALUE));
	BCHP_SET_FIELD_DATA(regVal, RAAGA_DSP_MISC_SCB0_EXT_CONFIG, EXT_END, ((pDevice->deviceSettings.memoryLayout.memc[0].region[1].addr +
                                             pDevice->deviceSettings.memoryLayout.memc[0].region[1].size) >> SCB_REG_ADDR_WRITE_SHIFT_VALUE));
        BDBG_MSG(("BCHP_RAAGA_DSP_MISC_SCB0_EXT_CONFIG : regVal : 0x%x", regVal));
	BDSP_WriteReg32(pDevice->regHandle, BCHP_RAAGA_DSP_MISC_SCB0_EXT_CONFIG+ui32dspOffset, regVal);

        regVal = BDSP_ReadReg32(pDevice->regHandle, BCHP_RAAGA_DSP_MISC_SCB1_EXT_CONFIG+ui32dspOffset);
	BCHP_SET_FIELD_DATA(regVal, RAAGA_DSP_MISC_SCB1_BASE_CONFIG, BASE_START,
	                                     (pDevice->deviceSettings.memoryLayout.memc[1].region[0].addr >> SCB_REG_ADDR_WRITE_SHIFT_VALUE));
	BCHP_SET_FIELD_DATA(regVal, RAAGA_DSP_MISC_SCB1_BASE_CONFIG, BASE_END, ((pDevice->deviceSettings.memoryLayout.memc[1].region[0].addr +
                                             pDevice->deviceSettings.memoryLayout.memc[1].region[0].size) >> SCB_REG_ADDR_WRITE_SHIFT_VALUE));
        BDBG_MSG(("BCHP_RAAGA_DSP_MISC_SCB1_BASE_CONFIG : regVal : 0x%x", regVal));
	BDSP_WriteReg32(pDevice->regHandle, BCHP_RAAGA_DSP_MISC_SCB1_BASE_CONFIG+ui32dspOffset, regVal);

        regVal = BDSP_ReadReg32(pDevice->regHandle, BCHP_RAAGA_DSP_MISC_SCB1_EXT_CONFIG+ui32dspOffset);
	BCHP_SET_FIELD_DATA(regVal, RAAGA_DSP_MISC_SCB1_EXT_CONFIG, EXT_START,
	                                    (pDevice->deviceSettings.memoryLayout.memc[1].region[1].addr >> SCB_REG_ADDR_WRITE_SHIFT_VALUE));
	BCHP_SET_FIELD_DATA(regVal, RAAGA_DSP_MISC_SCB1_EXT_CONFIG, EXT_END, ((pDevice->deviceSettings.memoryLayout.memc[1].region[1].addr +
                                             pDevice->deviceSettings.memoryLayout.memc[1].region[1].size) >> SCB_REG_ADDR_WRITE_SHIFT_VALUE));
        BDBG_MSG(("BCHP_RAAGA_DSP_MISC_SCB1_EXT_CONFIG : regVal : 0x%x", regVal));
	BDSP_WriteReg32(pDevice->regHandle, BCHP_RAAGA_DSP_MISC_SCB1_EXT_CONFIG+ui32dspOffset, regVal);

        BDBG_LEAVE(BDSP_Raaga_P_SetSCBConfig);
	return errCode;
}
static BERR_Code BDSP_Raaga_P_SetupDspBoot(
	BDSP_Raaga *pDevice,
	unsigned uiDspIndex
)
{
	BERR_Code errCode = BERR_SUCCESS;
	unsigned regVal=0;

	BDBG_ENTER(BDSP_Raaga_P_SetupDspBoot);

	errCode = BDSP_Raaga_P_SetSCBConfig (pDevice, uiDspIndex);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_SetupDspBoot: Error in Initing Core for DSP %d",uiDspIndex));
		goto err;
	}

	/* Initialize Mailbox5 register to A5A5 */
	BDSP_WriteReg32(pDevice->regHandle,BCHP_RAAGA_DSP_PERI_SW_MAILBOX5 + pDevice->dspOffset[uiDspIndex],
							BDSP_RAAGA_PREBOOT_MAILBOX_PATTERN);
	regVal = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_PERI_SW_MAILBOX5 + pDevice->dspOffset[uiDspIndex]);
	if(regVal != BDSP_RAAGA_PREBOOT_MAILBOX_PATTERN)
	{
		BDBG_ERR(("BDSP_Raaga_P_SetupDspBoot: Error in Validating MailBox 5 before BOOT "));
		BDBG_ASSERT(0);
	}

	BDSP_WriteReg32(pDevice->regHandle,BCHP_RAAGA_DSP_PERI_SW_MAILBOX0 + pDevice->dspOffset[uiDspIndex],
							BDSP_RAAGA_PREBOOT_MAILBOX_PATTERN);
	regVal = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_PERI_SW_MAILBOX0 + pDevice->dspOffset[uiDspIndex]);
	if(regVal != BDSP_RAAGA_PREBOOT_MAILBOX_PATTERN)
	{
		BDBG_ERR(("BDSP_Raaga_P_SetupDspBoot: Error in Validating MailBox 0 before BOOT"));
		BDBG_ASSERT(0);
	}

	regVal = BDSP_ReadFIFOReg(pDevice->regHandle,BCHP_RAAGA_DSP_FW_CFG_HOST2DSPCMD_FIFO_ID + pDevice->dspOffset[uiDspIndex]);
	if(regVal != pDevice->hCmdQueue[uiDspIndex]->ui32FifoId)
	{
		BDBG_ERR(("BDSP_Raaga_P_SetupDspBoot: Error in Validating the CMD Q FIFO ID before BOOT"));
		BDBG_ASSERT(0);
	}
	regVal = BDSP_ReadFIFOReg(pDevice->regHandle,BCHP_RAAGA_DSP_FW_CFG_SW_UNUSED1 + pDevice->dspOffset[uiDspIndex]);
	if(regVal != pDevice->hGenRespQueue[uiDspIndex]->ui32FifoId)
	{
		BDBG_ERR(("BDSP_Raaga_P_SetupDspBoot: Error in Validating the GEN RESP Q FIFO ID before BOOT"));
		BDBG_ASSERT(0);
	}
err:
	BDBG_LEAVE(BDSP_Raaga_P_SetupDspBoot);
	return errCode;
}

static void BDSP_Raaga_P_DisableCacheWays(BDSP_Raaga *pDevice, int32_t i32CacheWayNum)
{
	uint32_t ui32ReleaseDone = 0;
	uint32_t ui32CacheWayEnable = 0;
	uint32_t ui32PowerDownEnable = 0;

	BDSP_WriteReg32(pDevice->regHandle, BCHP_RAAGA_DSP_L2C_CMD_RELEASE_WAY, i32CacheWayNum);

	while(1)
	{
		ui32ReleaseDone = 0;
		ui32ReleaseDone = BDSP_ReadReg32(pDevice->regHandle, BCHP_RAAGA_DSP_L2C_RELEASE_WAY_DONE);
		if(ui32ReleaseDone & BCHP_RAAGA_DSP_L2C_RELEASE_WAY_DONE_RELEASE_WAY_DONE_MASK)
		{
			break;/*Release way successful*/
		}
		else if(ui32ReleaseDone & BCHP_RAAGA_DSP_L2C_RELEASE_WAY_DONE_RELEASE_WAY_ERROR_MASK)
		{
			BDBG_ERR(("ERROR Powering down cache ways"));
			return;
		}
	}

	ui32CacheWayEnable = BDSP_ReadReg32(pDevice->regHandle, BCHP_RAAGA_DSP_L2C_CACHE_WAY_ENABLE);
	BDSP_WriteReg32(pDevice->regHandle, BCHP_RAAGA_DSP_L2C_CACHE_WAY_ENABLE, (ui32CacheWayEnable & ~(1 << i32CacheWayNum)));

	/*BDSP_WriteReg32(pDevice->regHandle, BCHP_RAAGA_DSP_L2C_RELEASE_WAY_CLEAR, i32CacheWayNum);*/

	ui32PowerDownEnable = BDSP_ReadReg32(pDevice->regHandle, BCHP_RAAGA_DSP_L2C_POWERDN_WAY_ENABLE);
	BDSP_WriteReg32(pDevice->regHandle, BCHP_RAAGA_DSP_L2C_POWERDN_WAY_ENABLE, (ui32PowerDownEnable | (1 << i32CacheWayNum)));
}


static BERR_Code BDSP_Raaga_P_L2C_Config(
	BDSP_Raaga *pDevice,
	unsigned uiDspIndex
)
{
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BDSP_Raaga_P_L2C_Config);

	/* Disabling Cache ways */
#if 1
	if(pDevice->numCorePerDsp == 1)
	{
		/*Disable the Cacheways only when single core is present */
		BDSP_Raaga_P_DisableCacheWays(pDevice, 0);
		BDSP_Raaga_P_DisableCacheWays(pDevice, 1);
		BDSP_Raaga_P_DisableCacheWays(pDevice, 2);
		BDSP_Raaga_P_DisableCacheWays(pDevice, 3);
		BDSP_Raaga_P_DisableCacheWays(pDevice, 4);
	}
#else
	BDSP_WriteReg32(pDevice->regHandle,
		BCHP_RAAGA_DSP_L2C_CACHE_WAY_ENABLE+pDevice->dspOffset[uiDspIndex],
		0x3E0);
	BDSP_WriteReg32(pDevice->regHandle,
		BCHP_RAAGA_DSP_L2C_POWERDN_WAY_ENABLE+pDevice->dspOffset[uiDspIndex],
		0x1F);
#endif

    BDSP_WriteReg32(pDevice->regHandle,
		BCHP_RAAGA_DSP_L2C_CTRL0+pDevice->dspOffset[uiDspIndex],
		0x402);
    BDSP_WriteReg32(pDevice->regHandle,
		BCHP_RAAGA_DSP_L2C_CTRL1+pDevice->dspOffset[uiDspIndex],
		0x10C434);

	BDSP_WriteReg32(pDevice->regHandle,
		BCHP_RAAGA_DSP_L2C_ERROR_INTERRUPT_MASK_SET+pDevice->dspOffset[uiDspIndex],
		0x1ff);

	BDSP_WriteReg32(pDevice->regHandle,
		BCHP_RAAGA_DSP_L2C_ERROR_INTERRUPT_CLEAR+pDevice->dspOffset[uiDspIndex],
		0x1ff);

	BDBG_LEAVE(BDSP_Raaga_P_L2C_Config);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_UnresetRaagaCore(
	BDSP_Raaga *pDevice,
	unsigned uiDspIndex
)
{
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BDSP_Raaga_P_UnresetRaagaCore);

	BDSP_WriteReg32(pDevice->regHandle,
		BCHP_RAAGA_DSP_FW_INTH_HOST_CLEAR+pDevice->dspOffset[uiDspIndex],
		0X7);
	BDSP_WriteReg32(pDevice->regHandle,
		BCHP_RAAGA_DSP_INTH_HOST_CLEAR+pDevice->dspOffset[uiDspIndex],
		0x7ff);
	/*BDSP_WriteReg32(pDevice->regHandle,
		BCHP_RAAGA_DSP_FP_MISC_0_CORECTRL_CORE_ENABLE+pDevice->dspOffset[uiDspIndex],
		0x1);*/
	errCode = DSP_enable(&pDevice->sLibDsp,DSP_CORE_0);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_UnresetRaagaCore: DSP Enable failed for core 0"));
	}
	BDBG_LEAVE(BDSP_Raaga_P_UnresetRaagaCore);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_BootDsp(
	BDSP_Raaga *pDevice,
	unsigned uiDspIndex
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDBG_ENTER(BDSP_Raaga_P_BootDsp);

	errCode = BDSP_Raaga_P_SetupDspBoot(pDevice, uiDspIndex);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_BootDsp: Error in Setting up the DSP %d for BOOT",uiDspIndex));
		goto err_boot;
	}

	errCode = BDSP_Raaga_P_L2C_Config(pDevice,uiDspIndex);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_BootDsp: Error in L2C Cache configuration for DSP %d",uiDspIndex));
		goto err_boot;
	}

	errCode = BDSP_Raaga_P_UnresetRaagaCore (pDevice,uiDspIndex);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_BootDsp: Error in UNRESETTING the DSP %d",uiDspIndex));
		goto err_boot;
	}

err_boot:
	BDBG_LEAVE(BDSP_Raaga_P_BootDsp);
	return errCode;
}

/**********************************************************
Function: BDSP_Raaga_P_Reset

Description : Reset's Raaga for each DSP separately

**********************************************************/
BERR_Code BDSP_Raaga_P_Reset(BDSP_Raaga *pDevice)
{
	unsigned uiDspIndex;
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BDSP_Raaga_P_Reset);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

	for(uiDspIndex = 0; uiDspIndex < pDevice->numDsp; uiDspIndex++)
	{
		errCode = BDSP_Raaga_P_ResetHardware(pDevice, uiDspIndex);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_Reset: Error in Reset for DSP %d",uiDspIndex));
			goto err;
		}
	}

err:
	BDBG_LEAVE(BDSP_Raaga_P_Reset);
	return errCode;
}

/**********************************************************
Function: BDSP_Raaga_P_Boot

Description : Downloads the resident code, Init Raaga hardware with
			the settings which are different from default and Unresets
			Raaga.

**********************************************************/
BERR_Code BDSP_Raaga_P_Boot(BDSP_Raaga *pDevice)
{

	BERR_Code errCode = BERR_SUCCESS;
	unsigned uiDspIndex = 0;

	BDBG_ENTER(BDSP_Raaga_P_Boot);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

	for(uiDspIndex = 0; uiDspIndex < pDevice->numDsp; uiDspIndex++)
	{
		errCode = BDSP_Raaga_P_BootDsp( pDevice, uiDspIndex);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_Boot: Error in BOOTING DSP %d",uiDspIndex));
            BDBG_ASSERT(0);
		}
	}
	BDBG_LEAVE(BDSP_Raaga_P_Boot);
	return errCode;
}

void BDSP_Raaga_P_EnableDspWatchdogTimer (
		BDSP_Raaga *pDevice,
		uint32_t	dspIndex,
		bool		bEnable
)
{
    uint32_t regVal = 0;

	BDBG_ENTER(BDSP_Raaga_P_EnableDspWatchdogTimer);
    BDBG_MSG(("%s watchdog: DSP %d", bEnable?"Enable":"Disable", dspIndex));

    if (bEnable)
    {
        /* Program default watchdog count */
        regVal = (regVal & ~(BCHP_MASK(RAAGA_DSP_TIMERS_WATCHDOG_TIMER, COUNT)))
                        | BCHP_FIELD_DATA(RAAGA_DSP_TIMERS_WATCHDOG_TIMER, COUNT, BDSP_RAAGA_DEFAULT_WATCHDOG_COUNT);
        /* Disable auto reload of count */
        regVal = regVal & ~(BCHP_MASK(RAAGA_DSP_TIMERS_WATCHDOG_TIMER, CM));
        /* Enable timer bit */
        regVal = regVal | BCHP_MASK(RAAGA_DSP_TIMERS_WATCHDOG_TIMER, ET);
    }
    else {
        /* Program default watchdog count */
        regVal = (regVal & ~(BCHP_MASK(RAAGA_DSP_TIMERS_WATCHDOG_TIMER, COUNT)))
                        | BCHP_FIELD_DATA(RAAGA_DSP_TIMERS_WATCHDOG_TIMER, COUNT, BDSP_RAAGA_DEFAULT_WATCHDOG_COUNT);
        /* Disable timer bit */
        regVal = regVal & ~(BCHP_MASK(RAAGA_DSP_TIMERS_WATCHDOG_TIMER, ET));
    }
    BDBG_ASSERT((dspIndex + 1) <= BDSP_RAAGA_MAX_DSP);
    BDSP_WriteReg32(pDevice->regHandle, BCHP_RAAGA_DSP_TIMERS_WATCHDOG_TIMER + pDevice->dspOffset[dspIndex], regVal);
	BDBG_LEAVE(BDSP_Raaga_P_EnableDspWatchdogTimer);
}

BERR_Code BDSP_Raaga_P_WriteAtuEntry(
        BREG_Handle hRegister,
        BDSP_Raaga_P_ATUInfo *psATUInfo
)
{
    uint64_t ui64AtuVrtlEntry = 0;
    uint32_t ui32VrtlEntryOffset = 0;
    uint32_t ui32AtuPhysEntry = 0;
    uint32_t ui32PhysEntryOffset = 0;
    uint64_t ui64Temp = 0;
	uint32_t ui32EndAddr= 0;


	ui32EndAddr = (psATUInfo->ui32StartAddr + psATUInfo->size - 1);
    BDBG_MSG(("---------------------------------------------------------------------------------------------------------------------------"));
    BDBG_MSG(("StartAddr : %x \t EndAddr  : %x \t PhysAddr : " BDBG_UINT64_FMT"", psATUInfo->ui32StartAddr, ui32EndAddr, BDBG_UINT64_ARG(psATUInfo->offset)));
    /* Add Virtual start and end addresses to ATU table */
    ui32VrtlEntryOffset = BCHP_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLEi_ARRAY_BASE + (psATUInfo->eATUIndex*8);

    ui64Temp = (uint64_t)((ui32EndAddr >> 12) & 0xfffff);
    ui64AtuVrtlEntry = (uint64_t)((ui64Temp << 44) |
            (psATUInfo->ui32StartAddr & 0xfffff000) | 0x001);

    BDSP_WriteReg64(hRegister, ui32VrtlEntryOffset, ui64AtuVrtlEntry);

    /* Add Physical start and end addresses to ATU table */
    ui32PhysEntryOffset = BCHP_RAAGA_DSP_L2C_PHYSICAL_40B_BASE_ADDRi_ARRAY_BASE + (psATUInfo->eATUIndex * 4);

    ui32AtuPhysEntry = BDSP_ReadReg32(hRegister, ui32PhysEntryOffset);
    BCHP_SET_FIELD_DATA(ui32AtuPhysEntry, RAAGA_DSP_L2C_PHYSICAL_40B_BASE_ADDRi, BASE_ADDR, ((uint32_t)(psATUInfo->offset >> 9)));
    BDSP_WriteReg32(hRegister, ui32PhysEntryOffset, ui32AtuPhysEntry);

    BDBG_MSG(("---------------------------------------------------------------------------------------------------------------------------"));
    BDBG_MSG(("ui32PhysEntryOffset : %x \t ui32ATUPhysicalEntry : %x \t ui32VrtlEntryOffset : %x \t ui64AtuVrtlEntry : " BDBG_UINT64_FMT"",
            ui32PhysEntryOffset, ui32AtuPhysEntry, ui32VrtlEntryOffset, BDBG_UINT64_ARG(ui64AtuVrtlEntry)));

	return BERR_SUCCESS;
}

static void BDSP_Raaga_P_DisableAtuEntry(
        BREG_Handle hRegister,
        uint32_t ATU_index
)
{
    uint32_t ui32VrtlEntryOffset = 0;

    /* Add Virtual start and end addresses to ATU table */
    ui32VrtlEntryOffset = BCHP_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLEi_ARRAY_BASE + (ATU_index*8);

    BDSP_WriteReg64(hRegister, ui32VrtlEntryOffset, 0);
}

BERR_Code BDSP_Raaga_P_ProgramAtuEntries(
    BDSP_Raaga *pDevice,
    unsigned dspindex
)
{
	unsigned index;
	unsigned num_atu_entry, previous_start;
    BERR_Code errCode = BERR_SUCCESS;
	BDSP_Raaga_P_ATUInfo sATUInfo;
    BDBG_ENTER(BDSP_Raaga_P_ProgramAtuEntries);

#if (BCHP_VER == BCHP_VER_A0)
  num_atu_entry = 16;
#else
  num_atu_entry = 128;
	BDSP_WriteReg32(pDevice->regHandle,BCHP_RAAGA_DSP_L2C_CTRL5, BCHP_RAAGA_DSP_L2C_CTRL5_COREID_PROTECTION_DISABLE_MASK);
#endif

	for (index=0; index < num_atu_entry; index++)
	{
		BDBG_MSG(("BDSP_Raaga_P_DisableAtuEntry: disabling ATU  %d",index));
		BDSP_Raaga_P_DisableAtuEntry(pDevice->regHandle, index);
	}

	BKNI_Sleep(5);

	/* Program the RO Region */
	sATUInfo.eATUIndex = BDSP_Raaga_P_ATUEntry_ROMem;
	sATUInfo.offset    = pDevice->memInfo.sROMemoryPool.Memory.offset;
	sATUInfo.size      = pDevice->memInfo.sROMemoryPool.ui32Size;
	sATUInfo.ui32StartAddr = BDSP_ATU_VIRTUAL_RO_MEM_START_ADDR;
	errCode = BDSP_Raaga_P_WriteAtuEntry(pDevice->regHandle, &sATUInfo);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ProgramAtuEntries: Error in programming ATU for RO Memory"));
		goto end;
	}

	/* Program the RW Region */
	sATUInfo.eATUIndex = BDSP_Raaga_P_ATUEntry_RWMem_Kernel;
	sATUInfo.offset    = pDevice->memInfo.sKernelRWMemoryPool[dspindex].Memory.offset;
	sATUInfo.size      = pDevice->memInfo.sKernelRWMemoryPool[dspindex].ui32Size;
	sATUInfo.ui32StartAddr = BDSP_ATU_VIRTUAL_RW_MEM_START_ADDR;
	errCode = BDSP_Raaga_P_WriteAtuEntry(pDevice->regHandle, &sATUInfo);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ProgramAtuEntries: Error in programming ATU for RW Memory"));
		goto end;
	}
  previous_start = sATUInfo.ui32StartAddr;

	sATUInfo.eATUIndex = BDSP_Raaga_P_ATUEntry_RWMem_HostFWShared;
	sATUInfo.offset    = pDevice->memInfo.sHostSharedRWMemoryPool[dspindex].Memory.offset;
	sATUInfo.size      = pDevice->memInfo.sHostSharedRWMemoryPool[dspindex].ui32Size;
	sATUInfo.ui32StartAddr = previous_start +  pDevice->memInfo.sKernelRWMemoryPool[dspindex].ui32Size;
	errCode = BDSP_Raaga_P_WriteAtuEntry(pDevice->regHandle, &sATUInfo);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ProgramAtuEntries: Error in programming ATU for RW Memory"));
		goto end;
	}
  previous_start = sATUInfo.ui32StartAddr;
	sATUInfo.eATUIndex = BDSP_Raaga_P_ATUEntry_RWMem_FW;
	sATUInfo.offset    = pDevice->memInfo.sRWMemoryPool[dspindex].Memory.offset;
	sATUInfo.size      = pDevice->memInfo.sRWMemoryPool[dspindex].ui32Size;
	sATUInfo.ui32StartAddr = previous_start + pDevice->memInfo.sHostSharedRWMemoryPool[dspindex].ui32Size;
	errCode = BDSP_Raaga_P_WriteAtuEntry(pDevice->regHandle, &sATUInfo);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ProgramAtuEntries: Error in programming ATU for RW Memory"));
		goto end;
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_ProgramAtuEntries);
    return errCode;
}

/* Enable RAAGA_UART_ENABLE in bdsp_raaga.h to route UARTS to live terminal */
void BDSP_Raaga_P_DirectRaagaUartToPort(BREG_Handle regHandle)
{
#ifdef RAAGA_UART_ENABLE
	uint32_t regVal;

	regVal = BDSP_ReadReg32(regHandle, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
	regVal = (regVal & ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel)))
					| BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_1_cpu_sel_AUDIO_FP0);
	BDSP_WriteReg32(regHandle, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, regVal);

	regVal = BDSP_ReadReg32(regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
	regVal = (regVal & ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_006)))
					| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_006, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_006_TP_IN_18);
	BDSP_WriteReg32(regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, regVal);

	regVal = BDSP_ReadReg32(regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
	regVal = (regVal & ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_007)))
					| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_007, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_007_TP_OUT_19);
	BDSP_WriteReg32(regHandle, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, regVal);

	regVal = BDSP_ReadReg32(regHandle, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
	regVal = (regVal & ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable)))
					| BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS);
	BDSP_WriteReg32(regHandle, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, regVal);
#else
	BSTD_UNUSED(regHandle);
#endif /*RAAGA_UART_ENABLE*/
}

/***********************************************************************
Name        :   BDSP_Raaga_P_GetDefaultClkRate

Input       :   pDeviceHandle
				dsp Index

Output      :   Default Dsp Clock Rate

Function    :   This function is used for dynamic frequency scaling.
				Here we obtain the default clock rate of the DSP.
***********************************************************************/
BERR_Code BDSP_Raaga_P_GetDefaultClkRate(
	BDSP_Raaga *pRaaga,
	unsigned dspIndex,
	unsigned *pDefaultDspClkRate
)
{
	BERR_Code err = BERR_NOT_INITIALIZED;

	BDBG_ASSERT((dspIndex + 1) <= BDSP_RAAGA_MAX_DSP);

	if( dspIndex == 0 )
	{
		#ifdef BCHP_PWR_RESOURCE_RAAGA0_DSP
			err = BCHP_PWR_GetDefaultClockRate(pRaaga->chpHandle, BCHP_PWR_RESOURCE_RAAGA0_DSP, pDefaultDspClkRate );
		#else
			BSTD_UNUSED(pRaaga);
			BSTD_UNUSED(pDefaultDspClkRate);
		#endif
	}
	else if( dspIndex == 1 )
	{
		#ifdef BCHP_PWR_RESOURCE_RAAGA1_DSP
			err = BCHP_PWR_GetDefaultClockRate(pRaaga->chpHandle, BCHP_PWR_RESOURCE_RAAGA1_DSP, pDefaultDspClkRate );
		#endif
	}

	return err;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_SetDspClkRate

Input       :   pRaagaHandle
				expectedDspClkRate
				dsp Index

Function    :   This function is used for dynamic frequency scaling.
				This will be required as part of power management until we
				decouple the AIO RBUS bridge from the  DSP0 and have
				independent control.

				dspIndex decides which DSPs frequency is being updated.
				In case BCHP_PWR doesn't have support for dynamic freq
				scaling, the apis will return BERR_NOT_SUPPORTED.

Return      :   void
***********************************************************************/
void BDSP_Raaga_P_SetDspClkRate(
		BDSP_Raaga *pRaaga,
		unsigned expectedDspClkRate,
		unsigned dspIndex
)
{
	BERR_Code err = BERR_NOT_SUPPORTED;
	unsigned currentDspClkRate=0;

	BDBG_ASSERT((dspIndex + 1) <= BDSP_RAAGA_MAX_DSP);

	if( dspIndex == 0 )
	{
		#ifdef BCHP_PWR_RESOURCE_RAAGA0_DSP
			/* GetClk is done before SetClk to detect if DSP is already running at expected frequency,
				to avoid invoking the glitchless circuit */
			err = BCHP_PWR_GetClockRate(pRaaga->chpHandle, BCHP_PWR_RESOURCE_RAAGA0_DSP, &currentDspClkRate );
			if( (err == BERR_SUCCESS) && \
				( currentDspClkRate !=  expectedDspClkRate))
			{
				err = BCHP_PWR_SetClockRate(pRaaga->chpHandle, BCHP_PWR_RESOURCE_RAAGA0_DSP, expectedDspClkRate );
			}
		#else
			BSTD_UNUSED(pRaaga);
			BSTD_UNUSED(expectedDspClkRate);
			BSTD_UNUSED(currentDspClkRate);
			BSTD_UNUSED(dspIndex);
		#endif
	}
	else if( dspIndex == 1 )
	{
		#ifdef BCHP_PWR_RESOURCE_RAAGA1_DSP
			err = BCHP_PWR_GetClockRate(pRaaga->chpHandle, BCHP_PWR_RESOURCE_RAAGA1_DSP, &currentDspClkRate );
			if( (err == BERR_SUCCESS) && \
				( currentDspClkRate !=  expectedDspClkRate))
			{
				err = BCHP_PWR_SetClockRate(pDevice->chpHandle, BCHP_PWR_RESOURCE_RAAGA1_DSP, expectedDspClkRate );
			}
		#endif
	}

	if( err == BERR_SUCCESS)
			BDBG_MSG(("PWR: DSP%d current ClkRate = %d Hz ",dspIndex, expectedDspClkRate));
	return;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_GetLowerDspClkRate

Input       :   Default Raaga DSP frequency

Output      :   1/16th the input parameter-defaultDspClkRate

Funtion     :   After measuring power at different clock frequencies it
				was decided to have a 1/16th frequency for lower power
				consumption. So whenever DSP is not in use, before taking
				off the clock we will reduce the Clock frequency to
				1/16th the default.

***********************************************************************/

void  BDSP_Raaga_P_GetLowerDspClkRate(
		unsigned dspClkRate,
		unsigned *lowerDspClkRate
)
{
	BSTD_UNUSED(dspClkRate);
	*lowerDspClkRate = 46875000;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_EnableAllPwrResource

Input       :   pDeviceHandle
				Enable/Disable

Function    :   Enable/Disable the power resources Raaga,Raaga_sram and
				DSP.

Return      :   void

***********************************************************************/
void BDSP_Raaga_P_EnableAllPwrResource(
		void *pDeviceHandle,
		bool bEnable
)
{
	BDSP_Raaga *pDevice = pDeviceHandle;

	if( bEnable == true )
	{
		#ifdef  BCHP_PWR_RESOURCE_RAAGA0_SRAM
		BCHP_PWR_AcquireResource(pDevice->chpHandle, BCHP_PWR_RESOURCE_RAAGA0_SRAM);
		#endif
		#ifdef BCHP_PWR_RESOURCE_RAAGA1_SRAM
		BCHP_PWR_AcquireResource(pDevice->chpHandle, BCHP_PWR_RESOURCE_RAAGA1_SRAM);
		#endif
		#ifdef BCHP_PWR_RESOURCE_RAAGA0_CLK
		BCHP_PWR_AcquireResource(pDevice->chpHandle, BCHP_PWR_RESOURCE_RAAGA0_CLK);
		#else
		BSTD_UNUSED(pDevice);
		#endif
		#ifdef BCHP_PWR_RESOURCE_RAAGA1_CLK
		BCHP_PWR_AcquireResource(pDevice->chpHandle, BCHP_PWR_RESOURCE_RAAGA1_CLK);
		#endif
		#ifdef BCHP_PWR_RESOURCE_RAAGA0_DSP
		BCHP_PWR_AcquireResource(pDevice->chpHandle, BCHP_PWR_RESOURCE_RAAGA0_DSP);
		#endif
		#ifdef BCHP_PWR_RESOURCE_RAAGA1_DSP
		BCHP_PWR_AcquireResource(pDevice->chpHandle, BCHP_PWR_RESOURCE_RAAGA1_DSP);
		#endif
	}
	else
	{
		#ifdef  BCHP_PWR_RESOURCE_RAAGA0_SRAM
		BCHP_PWR_ReleaseResource(pDevice->chpHandle, BCHP_PWR_RESOURCE_RAAGA0_SRAM);
		#endif
		#ifdef BCHP_PWR_RESOURCE_RAAGA1_SRAM
		BCHP_PWR_ReleaseResource(pDevice->chpHandle, BCHP_PWR_RESOURCE_RAAGA1_SRAM);
		#endif
		#ifdef BCHP_PWR_RESOURCE_RAAGA0_CLK
		BCHP_PWR_ReleaseResource(pDevice->chpHandle, BCHP_PWR_RESOURCE_RAAGA0_CLK);
		#else
		BSTD_UNUSED(pDevice);
		#endif
		#ifdef BCHP_PWR_RESOURCE_RAAGA1_CLK
		BCHP_PWR_ReleaseResource(pDevice->chpHandle, BCHP_PWR_RESOURCE_RAAGA1_CLK);
		#endif
		#ifdef BCHP_PWR_RESOURCE_RAAGA0_DSP
		BCHP_PWR_ReleaseResource(pDevice->chpHandle, BCHP_PWR_RESOURCE_RAAGA0_DSP);
		#endif
		#ifdef BCHP_PWR_RESOURCE_RAAGA1_DSP
		BCHP_PWR_ReleaseResource(pDevice->chpHandle, BCHP_PWR_RESOURCE_RAAGA1_DSP);
		#endif
	}
}

/***********************************************************************
Name        :   BDSP_Raaga_P_PowerStandby

Type        :   PI Interface

Input       :   pDeviceHandle -Device Handle which needs to be closed.
				pSettings - pointer where the Default Data will be filled and returned to PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   To check if all the task for the DSP is closed.
				Then disable the Watchdog timer, Reset the Hardware and relanquish the resources.
***********************************************************************/
BERR_Code BDSP_Raaga_P_PowerStandby(
	void 					*pDeviceHandle,
	BDSP_StandbySettings    *pSettings
)
{
	BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
	BERR_Code errCode=BERR_SUCCESS;
	unsigned index = 0;

	BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);
	BDBG_ENTER(BDSP_Raaga_P_PowerStandby);

	if (pSettings)
		BSTD_UNUSED(pSettings);

	if (!pDevice->hardwareStatus.powerStandby)
	{
		BDSP_RaagaContext *pRaagaContext=NULL;
		BDSP_RaagaTask    *pRaagaTask=NULL;

		for (pRaagaContext = BLST_S_FIRST(&pDevice->contextList);
			pRaagaContext != NULL;
			pRaagaContext = BLST_S_NEXT(pRaagaContext, node) )
		{
			for (pRaagaTask = BLST_S_FIRST(&pRaagaContext->taskList);
				pRaagaTask != NULL;
				pRaagaTask = BLST_S_NEXT(pRaagaTask, node) )
			{
				if (pRaagaTask->taskParams.isRunning == true)
				{
					BDBG_ERR(("BDSP_Raaga_P_PowerStandby: Task %d is not stopped. Cannot go in standby",pRaagaTask->taskParams.taskId));
					errCode = BERR_INVALID_PARAMETER;
					goto end;
				}
			}
		}
		/*Disable watchdog*/
		for(index=0; index<pDevice->numDsp; index++)
		{
			BDSP_Raaga_P_EnableDspWatchdogTimer(pDeviceHandle,index,false);
		}

		errCode = BDSP_Raaga_P_Reset(pDeviceHandle);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_PowerStandby: Error in Reset of the Raaga"));
			goto end;
		}
		BDSP_Raaga_P_EnableAllPwrResource(pDeviceHandle, false);
		pDevice->hardwareStatus.powerStandby = true;
	}
	else
	{
		BDBG_WRN(("BDSP_Raaga_P_PowerStandby: Already in standby mode"));
		errCode = BERR_INVALID_PARAMETER;
		goto end;
	}

end:

	BDBG_LEAVE(BDSP_Raaga_P_PowerStandby);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_PowerResume

Type        :   PI Interface

Input       :   pDeviceHandle -Device Handle which needs to be closed.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Acquire the resources for the DSP. Reboot and initialize DSP, then Enable the Watchdog timer.
***********************************************************************/
BERR_Code BDSP_Raaga_P_PowerResume(
	void *pDeviceHandle)
{
	BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
	BERR_Code errCode=BERR_SUCCESS;

	BDBG_ENTER(BDSP_Raaga_P_PowerResume);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

	if (pDevice->hardwareStatus.powerStandby)
	{
		BDSP_Raaga_P_EnableAllPwrResource(pDeviceHandle, true);
        pDevice->hardwareStatus.deviceWatchdogFlag = true;
        BDSP_Raaga_P_Reset(pDevice);
        /*Raaga_Open enables Watchdog*/
		BDSP_Raaga_P_Open(pDevice);
		if(pDevice->deviceSettings.authenticationEnabled == false)
		{
			errCode = BDSP_Raaga_P_Boot(pDevice);
			if (errCode !=BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Raaga_P_PowerResume: Error in Boot Sequence in Power Resume"));
				errCode = BERR_TRACE(errCode);
				goto end;
			}

			errCode = BDSP_Raaga_P_CheckDspAlive(pDevice);
			if (errCode!=BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Raaga_P_PowerResume: DSP not alive"));
				errCode= BERR_TRACE(errCode);
				goto end;
			}
			else
			{
				BDBG_MSG(("BDSP_Raaga_P_PowerResume: DSP is alive"));
			}
		}

        pDevice->hardwareStatus.deviceWatchdogFlag = false;
        pDevice->hardwareStatus.powerStandby = false;
	}
	else
	{
		BDBG_WRN(("BDSP_Raaga_P_PowerResume: Not in standby mode"));
		errCode = BERR_INVALID_PARAMETER;
		goto end;
	}
end:

	BDBG_LEAVE(BDSP_Raaga_P_PowerResume);
	return errCode;
}

BERR_Code BDSP_Raaga_P_Initialize(
	void  *pDeviceHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;

    BDBG_ENTER(BDSP_Raaga_P_Initialize);
    /* Assert the function arguments*/
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    /*If Firmware authentication is Disabled*/
    if(pDevice->deviceSettings.authenticationEnabled==false)
    {
        BDBG_ERR(("BDSP_Raaga_P_Initialize should be called only if bFwAuthEnable is true"));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto end;
    }

    errCode = BDSP_Raaga_P_Boot(pDevice);
    if (errCode!=BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_Initialize: Error in Booting Raaga"));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Raaga_P_CheckDspAlive(pDevice);
    if (errCode!=BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_Initialize: DSP not alive"));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

end:
    BDBG_LEAVE(BDSP_Raaga_P_Initialize);
    return errCode;
}
