/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
* Module Description:
*
***************************************************************************/
#include "bstd.h"

#include "bkni.h"
#include "bchp_common.h"
#include "breg_mem.h"
#include "client.h"

BDBG_MODULE(BREG_CLIENT);

static void BREG_P_systemUpdate32_isrsafe(void *context, uint32_t reg, uint32_t mask, uint32_t value, bool atomic);
static bool BREG_P_isRegisterAtomic_isrsafe(void *unused, uint32_t reg );

static const BREG_OpenSettings BREG_OpenSettings_Default = {
    NULL,
    BREG_P_isRegisterAtomic_isrsafe,
    BREG_P_systemUpdate32_isrsafe,
    NULL,
    0
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

#if defined(BCHP_HIF_MSAT_REG_START) /* MSAT is hw atomizer for 64-bit register access from 32-bit host */
#if defined(BREG_64_NATIVE_SUPPORT) /* native 64-bit register access from 64-bit host interface */
void BREG_Write64_isrsafe(BREG_Handle regHandle, uint32_t reg, uint64_t data)
{
    BDBG_ASSERT(reg < regHandle->MaxRegOffset);
    if(reg%8!=0) {
        BDBG_ASSERT(0);
    }
    BEMU_Client_WriteRegister64(reg, data);
    return;
}

uint64_t BREG_Read64_isrsafe(BREG_Handle regHandle, uint32_t reg)
{
    uint64_t data;
    BDBG_ASSERT(reg < regHandle->MaxRegOffset);
    if(reg%8!=0) {
        BDBG_ASSERT(0);
    }
    BEMU_Client_ReadRegister64(reg, &data);
    return data;
}
#else /*  #if !defined(BCHP_HIF_MSAT_REG_START)  */
#include "bchp_hif_msat.h"
    /* 64-bits Migration: Control/Busses/Register Update */
    /* 5.7  Managed SAT */

static void BREG_P_Msat_SetAddress_isrsafe(BREG_Handle regHandle, unsigned offset, uint32_t reg)
{
    uint64_t deviceOffset = reg + (uint64_t)BCHP_PHYSICAL_OFFSET;
    BREG_Write32(regHandle, BCHP_HIF_MSAT_CHANNEL_0_LO_ADDR + offset, (uint32_t)deviceOffset);
    BREG_Write32(regHandle, BCHP_HIF_MSAT_CHANNEL_0_HI_ADDR + offset, (uint32_t)(deviceOffset>>32));
    return;
}

struct BREG_P_Msat_ActionContext {
    uint32_t reg;
    uint64_t data;
};

static void BREG_P_Msat_Write64_isrsafe(BREG_Handle regHandle, unsigned index, uint32_t reg, uint64_t data)
{
    unsigned offset = (BCHP_HIF_MSAT_CHANNEL_1_LO_ADDR - BCHP_HIF_MSAT_CHANNEL_0_LO_ADDR)*index;
    BREG_P_Msat_SetAddress_isrsafe(regHandle, offset, reg);
    BREG_Write32(regHandle, BCHP_HIF_MSAT_CHANNEL_0_LO_DATA + offset, (uint32_t)data);
    BREG_Write32(regHandle, BCHP_HIF_MSAT_CHANNEL_0_HI_DATA + offset, (uint32_t)(data>>32));
    BREG_Write32(regHandle, BCHP_HIF_MSAT_CHANNEL_0_ACTION + offset, BCHP_FIELD_ENUM(HIF_MSAT_CHANNEL_0_ACTION, WRITE, WRITE_CYC));
    return;
}

static void BREG_P_Msat_Action_Write64_isrsafe(BREG_Handle regHandle, void *_context)
{
    struct BREG_P_Msat_ActionContext *context = _context;
    BREG_P_Msat_Write64_isrsafe(regHandle, regHandle->openSettings.msatChannel, context->reg, context->data);
    return;
}


void BREG_Write64_isrsafe(BREG_Handle regHandle, uint32_t reg, uint64_t data)
{
    uint32_t tmp;
    unsigned index;

    BDBG_ASSERT(reg < regHandle->MaxRegOffset);
    tmp = BREG_Read32(regHandle, BCHP_HIF_MSAT_ACQUIRE);
    index = BCHP_GET_FIELD_DATA(tmp, HIF_MSAT_ACQUIRE, CHANNEL_INDEX);
    if(index!=0xFF) {
        BREG_P_Msat_Write64_isrsafe(regHandle, index, reg, data);
        BREG_Write32(regHandle, BCHP_HIF_MSAT_RELEASE, BCHP_FIELD_DATA(HIF_MSAT_RELEASE, CHANNEL_INDEX, index));
    } else {
        struct BREG_P_Msat_ActionContext context;
        context.reg = reg;
        context.data = data;
        regHandle->openSettings.runSerialized_isrsafe(regHandle->openSettings.callbackContext, BREG_P_Msat_Action_Write64_isrsafe, &context);
    }
    return ;
}

static uint64_t BREG_P_Msat_Read64_isrsafe(BREG_Handle regHandle, unsigned index, uint32_t reg)
{
    unsigned offset = (BCHP_HIF_MSAT_CHANNEL_1_LO_ADDR - BCHP_HIF_MSAT_CHANNEL_0_LO_ADDR)*index;
    uint64_t data;
    uint32_t tmp;
    BREG_P_Msat_SetAddress_isrsafe(regHandle, offset, reg);
    BREG_Write32(regHandle, BCHP_HIF_MSAT_CHANNEL_0_ACTION + offset, BCHP_FIELD_ENUM(HIF_MSAT_CHANNEL_0_ACTION, READ, READ_CYC));
    tmp = BREG_Read32(regHandle, BCHP_HIF_MSAT_CHANNEL_0_LO_DATA + offset);
    data = BREG_Read32(regHandle, BCHP_HIF_MSAT_CHANNEL_0_HI_DATA + offset);
    data = (data<<32) | tmp;
    return data;
}

static void BREG_P_Msat_Action_Read64_isrsafe(BREG_Handle regHandle, void *_context)
{
    struct BREG_P_Msat_ActionContext *context = _context;
    context->data = BREG_P_Msat_Read64_isrsafe(regHandle, regHandle->openSettings.msatChannel, context->reg);
    return;
}

uint64_t BREG_Read64_isrsafe(BREG_Handle regHandle, uint32_t reg)
{
    uint64_t data;
    uint32_t tmp;
    unsigned index;

    BDBG_ASSERT(reg < regHandle->MaxRegOffset);

    tmp = BREG_Read32(regHandle, BCHP_HIF_MSAT_ACQUIRE);
    index = BCHP_GET_FIELD_DATA(tmp, HIF_MSAT_ACQUIRE, CHANNEL_INDEX);
    if(index!=0xFF) {
        data = BREG_P_Msat_Read64_isrsafe(regHandle, index, reg);
        BREG_Write32(regHandle, BCHP_HIF_MSAT_RELEASE, BCHP_FIELD_DATA(HIF_MSAT_RELEASE, CHANNEL_INDEX, index));
    } else {
        struct BREG_P_Msat_ActionContext context;
        context.reg = reg;
        regHandle->openSettings.runSerialized_isrsafe(regHandle->openSettings.callbackContext, BREG_P_Msat_Action_Read64_isrsafe, &context);
        data = context.data;
    }
    return data;
}
#endif /* #if defined(BREG_64_NATIVE_SUPPORT) */
#else /* only allow 32-bit */
void BREG_Write64_isrsafe(BREG_Handle regHandle, uint32_t reg, uint64_t data)
{
    BDBG_ASSERT(reg < regHandle->MaxRegOffset);
    if(reg%8!=0) { /* only allow access to low 32-bits */
        BDBG_ASSERT(0);
    }
    BEMU_Client_WriteRegister(reg, (uint32_t)data);
    return;
}

uint64_t BREG_Read64_isrsafe(BREG_Handle regHandle, uint32_t reg)
{
    uint64_t data;
    BDBG_ASSERT(reg < regHandle->MaxRegOffset);
    if(reg%8!=0) { /* only allow access to low 32-bits */
        BDBG_ASSERT(0);
    }
    BEMU_Client_ReadRegister(reg, &data);
    return data;
}
#endif /* #if defined(BCHP_HIF_MSAT_REG_START) */
/* End of File */

