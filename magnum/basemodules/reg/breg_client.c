/***************************************************************************
 *	   Copyright (c) 2003-2010, Broadcom Corporation
 *	   All Rights Reserved
 *	   Confidential Property of Broadcom Corporation
 *
 *	THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *	AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *	EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
 ***************************************************************************/
#include "bstd.h"

#include "bkni.h"

#include "breg_mem.h"
#include "client.h"

BDBG_MODULE(BREG_CLIENT);

static void BREG_P_systemUpdate32_isrsafe(void *context, uint32_t reg, uint32_t mask, uint32_t value, bool atomic);
static bool BREG_P_isRegisterAtomic_isrsafe(void *unused, uint32_t reg );

static const BREG_OpenSettings BREG_OpenSettings_Default = {
    NULL,
    BREG_P_isRegisterAtomic_isrsafe,
    BREG_P_systemUpdate32_isrsafe
};

void BREG_GetDefaultOpenSettings(BREG_OpenSettings *pSettings)
{
    *pSettings = BREG_OpenSettings_Default;
    return;
}

BERR_Code BREG_Open( BREG_Handle *pRegHandle, void *Address, size_t MaxRegOffset, const BREG_OpenSettings *pSettings)
{
    BREG_Handle regHandle;
    *pRegHandle = NULL;

    if(pSettings==NULL) {
        pSettings = &BREG_OpenSettings_Default;
    }
    if(pSettings->isRegisterAtomic_isrsafe == NULL || pSettings->systemUpdate32_isrsafe == NULL) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    regHandle = BKNI_Malloc( sizeof(*regHandle) );
    if(regHandle==NULL) {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    regHandle->openSettings = *pSettings;
    if(pSettings->systemUpdate32_isrsafe == BREG_P_systemUpdate32_isrsafe) {
        regHandle->openSettings.callbackContext = regHandle;
    }
    regHandle->MaxRegOffset = MaxRegOffset;
    regHandle->BaseAddr = Address;
    *pRegHandle = regHandle;
    return BERR_SUCCESS;
}

void BREG_Close( BREG_Handle RegHandle )
{
    BDBG_ASSERT(RegHandle != NULL );
    BKNI_Free(RegHandle);
}

/* compile the register access functions even for the release build */
#undef	BREG_Write32
#undef	BREG_Write16
#undef	BREG_Write8

#undef	BREG_Read32
#undef	BREG_Read16
#undef	BREG_Read8

uint32_t BREG_Read32(BREG_Handle RegHandle, uint32_t reg)
{
	uint32_t data ;

	BDBG_ASSERT(reg < RegHandle->MaxRegOffset);
	BEMU_Client_ReadRegister(reg, &data);
	return data ;
}

uint16_t BREG_Read16(BREG_Handle RegHandle, uint32_t reg)
{
	uint32_t data ;

	BDBG_ASSERT(reg < RegHandle->MaxRegOffset);
	BEMU_Client_ReadRegister(reg, &data);
	return (data & 0xffff) ;
}

uint8_t BREG_Read8(BREG_Handle RegHandle, uint32_t reg)
{
	uint32_t data ;

	BDBG_ASSERT(reg < RegHandle->MaxRegOffset);
	BEMU_Client_ReadRegister(reg, &data);
	return (data & 0xff) ;
}

void BREG_Write32(BREG_Handle RegHandle, uint32_t reg, uint32_t data)
{
	BDBG_ASSERT(reg < RegHandle->MaxRegOffset);
	BEMU_Client_WriteRegister(reg, data) ;
}

void BREG_Write16(BREG_Handle RegHandle, uint32_t reg, uint16_t data)
{
	BSTD_UNUSED(data);
	BDBG_ASSERT(reg < RegHandle->MaxRegOffset);
	BDBG_ASSERT(0);
}

void BREG_Write8(BREG_Handle RegHandle, uint32_t reg, uint8_t data)
{
	BSTD_UNUSED(data);
	BDBG_ASSERT(reg < RegHandle->MaxRegOffset);
	BDBG_ASSERT(0);
}


static void BREG_P_systemUpdate32_isrsafe(void *context, uint32_t addr, uint32_t mask, uint32_t value, bool atomic)
{
	uint32_t temp;

#if 0
    addr = ((BREG_Handle)context)->BaseAddr + addr;
    temp = *(volatile uint32_t *)addr;
    temp = (temp&~mask)|value;
    *(volatile uint32_t *)addr = temp;
#else
/*    addr = ((BREG_Handle)context)->BaseAddr + addr;*/
    BSTD_UNUSED(atomic);
	temp = BREG_Read32((BREG_Handle)context, addr);
    temp = (temp&~mask)|value;
	BREG_Write32((BREG_Handle)context, addr, temp);
#endif
    return;
}


#include "bchp_sun_top_ctrl.h"
#define BREG_P_ATOMIC_REG(reg) case reg: regAtomic=true;break
static bool BREG_P_isRegisterAtomic_isrsafe(void *unused, uint32_t reg )
{
    bool regAtomic;
    BSTD_UNUSED(unused);
    switch(reg) {
#ifdef BCHP_SUN_TOP_CTRL_SW_RESET		
    BREG_P_ATOMIC_REG(BCHP_SUN_TOP_CTRL_SW_RESET);
#else
    BREG_P_ATOMIC_REG(BCHP_SUN_TOP_CTRL_SW_INIT_0_SET);
#endif
#if (BCHP_CHIP==7405)
#include "bchp_decode_sd_0.h"
#include "bchp_decode_ip_shim_0.h"
    BREG_P_ATOMIC_REG(BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH);
    BREG_P_ATOMIC_REG(BCHP_DECODE_IP_SHIM_0_PFRI_REG);
#if BCHP_VER >= BCHP_VER_B0
    BREG_P_ATOMIC_REG(BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH);
#endif
#elif (BCHP_CHIP==3556 || BCHP_CHIP==3548)
#include "bchp_decode_sd_0.h"
    BREG_P_ATOMIC_REG(BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH);
    BREG_P_ATOMIC_REG(BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH);
#include "bchp_clkgen.h"
    BREG_P_ATOMIC_REG(BCHP_CLKGEN_PWRDN_CTRL_0);
    BREG_P_ATOMIC_REG(BCHP_CLKGEN_PWRDN_CTRL_1);
    BREG_P_ATOMIC_REG(BCHP_CLKGEN_PWRDN_CTRL_2);
    BREG_P_ATOMIC_REG(BCHP_CLKGEN_PWRDN_CTRL_3);
#include "bchp_vcxo_ctl_misc.h"
    BREG_P_ATOMIC_REG(BCHP_VCXO_CTL_MISC_AVD_CTRL);
#elif (BCHP_CHIP==7325 || BCHP_CHIP==7335)
#include "bchp_decode_sd_0.h"
#include "bchp_decode_ip_shim_0.h"
    BREG_P_ATOMIC_REG(BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH);
    BREG_P_ATOMIC_REG(BCHP_DECODE_IP_SHIM_0_PFRI_REG);
#endif
    default:
        regAtomic = false;
        break;
    }
    return regAtomic;
}

#if BDBG_DEBUG_BUILD
static void BREG_P_CheckAtomicRegister_isrsafe(BREG_Handle regHandle, uint32_t reg, const char *function, bool atomic )
{
    bool regAtomic = regHandle->openSettings.isRegisterAtomic_isrsafe(regHandle->openSettings.callbackContext, reg);

    if(regAtomic!=atomic) {
        if(!atomic) {
            BDBG_ERR(("%s: register %#lx should only be used with atomic access", function, (unsigned long)reg));
        } else {
            BDBG_ERR(("%s: register %#lx shouldn't be used for atomic access", function, (unsigned long)reg));
        }
    }
    return;
}
#else
#define BREG_P_CheckAtomicRegister_isrsafe(regHandle, reg, function, atomic)
#endif

void BREG_AtomicUpdate32_isrsafe(BREG_Handle RegHandle, uint32_t reg, uint32_t mask, uint32_t value)
{
    BDBG_ASSERT(reg < RegHandle->MaxRegOffset);
    BREG_P_CheckAtomicRegister_isrsafe(RegHandle, reg, "BREG_AtomicUpdate32_isr", true);
    RegHandle->openSettings.systemUpdate32_isrsafe(RegHandle->openSettings.callbackContext, reg, mask, value, true);
#if BREG_CAPTURE
    APP_BREG_Write32(RegHandle,reg,BREG_Read32(RegHandle,reg));
#endif
}

void BREG_Update32_isrsafe(BREG_Handle RegHandle, uint32_t reg, uint32_t mask, uint32_t value)
{
    RegHandle->openSettings.systemUpdate32_isrsafe(RegHandle->openSettings.callbackContext, reg, mask, value, false);
    return;
}
/* End of File */

