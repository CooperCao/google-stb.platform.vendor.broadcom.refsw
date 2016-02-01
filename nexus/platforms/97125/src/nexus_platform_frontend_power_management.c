/******************************************************************************
 *    (c)2008-2012 Broadcom Corporation
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
#include "nexus_platform_priv.h"
#include "nexus_platform_frontend_power_management.h"
#include "bchp_sub_bnmctl.h"
#include "bchp_sub_ubus_sgisb.h"
#include "nexus_frontend_3255.h"
#include "bchp_common.h"


BDBG_MODULE(nexus_platform_frontend_power_management);

#if NEXUS_POWER_MANAGEMENT

static NEXUS_Error NEXUS_Platform_P_FrontendControlEnterNormalOperation(void);
static NEXUS_Error NEXUS_Platform_P_FrontendControlEnterActiveStandby(void);
static NEXUS_Error NEXUS_Platform_P_FrontendControlEnterPassiveStandby(void);


static NEXUS_PlatformFrontendStandbyState g_PlatformFrontendPowerManagementState;

NEXUS_Error NEXUS_Platform_SetFrontendStandbySetting(const NEXUS_PlatformFrontendStandby_Settings *pSettings /* [in] */)
{
    BDBG_ASSERT(NULL != pSettings);

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Platform_GetFrontendStandbySetting(NEXUS_PlatformFrontendStandby_Settings *pSettings/* [out] */)
{
    BDBG_ASSERT(NULL != pSettings);

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Platform_GetDefaultFrontendStandbySetting(NEXUS_PlatformFrontendStandby_Settings *pSettings/* [out] */)
{
    BDBG_ASSERT(NULL != pSettings);

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Platform_GetFrontendStandbyMode(NEXUS_PlatformFrontendStandbyMode * pMode /* [out] */)
{
    BDBG_ASSERT(NULL != pMode);
    
    *pMode = g_PlatformFrontendPowerManagementState.mode;

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Platform_SetFrontendStandbyMode(const NEXUS_PlatformFrontendStandbyMode Mode /* [in] */)
{

    switch(Mode)
    {
        case NEXUS_PlatformFrontendStandbyMode_eFullPower:
            NEXUS_Platform_P_FrontendControlEnterNormalOperation();
            break;
        case NEXUS_PlatformFrontendStandbyMode_eActive:
            NEXUS_Platform_P_FrontendControlEnterActiveStandby();
            break;
        case NEXUS_PlatformFrontendStandbyMode_ePassive:
            NEXUS_Platform_P_FrontendControlEnterPassiveStandby();
            break;
        default:
	    return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            break;
    }

    return NEXUS_SUCCESS;
}


static NEXUS_Error NEXUS_Platform_P_FrontendControlEnterNormalOperation(void)
{
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Error rc = NEXUS_NOT_SUPPORTED;
    NEXUS_FrontendHandle handle = platformConfig.frontend[1];
    bool isOperational = false;
    int count = 0;

    NEXUS_Platform_GetConfiguration(&platformConfig);
    handle = platformConfig.frontend[1];

    if (NEXUS_Platform_P_FrontendType(handle)==3255)
    {

        NEXUS_Frontend_3255_EnableRPC(handle, true);
    
        /*because this is after wakeup, to receive notification to set operational, may there be a timing issue potentially?*/
        do{
            BKNI_Sleep(1000);
    
            NEXUS_Frontend_3255_IsCMOperational( handle, &isOperational);
            if(count > 10) /*assume BNM bootup takes 10 s*/
                NEXUS_Frontend_3255_EnableHeartBeat(handle, true);                
            count ++;
        }while(isOperational != true); 
    
        NEXUS_Platform_P_HostFrontendPinmux(false); /* set pinmux for BNM to use BSC_M3 (I2C) bus for 3112 */
    
        rc = NEXUS_Frontend_3255_TransferFrontendControlToCM(handle);
    
        if(rc != NEXUS_SUCCESS){
            BDBG_WRN(("not right here! eCM on, but tansfer frontend control rmagnum failed"));
            return NEXUS_UNKNOWN;
        }
    
        NEXUS_Platform_P_HostFrontendPinmux(false); /* set pinmux for BNM to use BSC_M3 (I2C) bus for 3112 */
    
        g_PlatformFrontendPowerManagementState.mode = NEXUS_PlatformFrontendStandbyMode_eFullPower;
    }

    return NEXUS_SUCCESS;
		
}

static NEXUS_Error NEXUS_Platform_P_FrontendControlEnterActiveStandby(void)
{
    NEXUS_PlatformConfiguration platformConfig;
    bool isOperational = false;
    NEXUS_Error rc = NEXUS_NOT_SUPPORTED;

    NEXUS_Platform_GetConfiguration(&platformConfig);

    if (NEXUS_Platform_P_FrontendType(platformConfig.frontend[1])==3255)
    {
        rc = NEXUS_Frontend_3255_IsCMOperational( platformConfig.frontend[1], &isOperational);
    
        if(isOperational != true) {
            BDBG_WRN(("eCM is not power ON mode, trying to enter active standby"));
            return NEXUS_UNKNOWN;
        }
    
    
        NEXUS_Platform_P_HostFrontendPinmux(true);    /* only switch this control should not bring glitch of data on this channel*/
    
    
        {
            BDBG_MSG((" NEXUS_Platform_FrontendControlEnterActiveStandby will shutdown CM"));
    
            NEXUS_Platform_ShutdownCM();
    
            g_PlatformFrontendPowerManagementState.mode = NEXUS_PlatformFrontendStandbyMode_eActive;
            
        }
    }
    
    return NEXUS_SUCCESS;


}

static NEXUS_Error NEXUS_Platform_P_FrontendControlEnterPassiveStandby(void)
{
    NEXUS_PlatformConfiguration platformConfig;

    NEXUS_Platform_GetConfiguration(&platformConfig);

    BDBG_MSG((" NEXUS_Platform_FrontendControlEnterPassiveStandby will shutdown CM"));
    NEXUS_Platform_ShutdownCM();

    g_PlatformFrontendPowerManagementState.mode = NEXUS_PlatformFrontendStandbyMode_ePassive;
    
    return NEXUS_SUCCESS;

}

NEXUS_Error NEXUS_Platform_WakeupCM(void)
{
    /*Platform dependent code to wake up eCM*/
#if 0
    volatile unsigned int *bnmctl = (unsigned int *)0xb4000028;

    *bnmctl &= ~3;  /* Reset the BNM UBUS and MIPS*/
#else

    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    BDBG_MSG((" NEXUS_Platform_WakeupCM will write 0x%x", BCHP_SUB_BNMCTL_HOSTBNMCTL));

#if 0
    reg = BREG_Read32(hReg, BCHP_SUB_BNMCTL_HOSTBNMCTL);
    reg &= ~(
            BCHP_MASK(SUB_BNMCTL_HOSTBNMCTL, MIPS_RELEASE ) |
            BCHP_MASK(SUB_BNMCTL_HOSTBNMCTL, BNM_SYS_RESET_N )
            );
    reg |= 
            BCHP_FIELD_DATA(SUB_BNMCTL_HOSTBNMCTL, MIPS_RELEASE,  1 ) | 
            BCHP_FIELD_DATA(SUB_BNMCTL_HOSTBNMCTL, BNM_SYS_RESET_N,  1 )	
    ;
    BREG_Write32(hReg, BCHP_SUB_BNMCTL_HOSTBNMCTL, reg);
#else

    reg = BREG_Read32(hReg, BCHP_SUB_BNMCTL_HOSTBNMCTL);
    reg &= ~(
            BCHP_MASK(SUB_BNMCTL_HOSTBNMCTL, BNM_SYS_RESET_N )
            );
    reg |= 
            BCHP_FIELD_DATA(SUB_BNMCTL_HOSTBNMCTL, BNM_SYS_RESET_N,  0 )	
    ;
    BREG_Write32(hReg, BCHP_SUB_BNMCTL_HOSTBNMCTL, reg);

    reg = BREG_Read32(hReg, BCHP_SUB_BNMCTL_HOSTBNMCTL);
    reg &= ~(
            BCHP_MASK(SUB_BNMCTL_HOSTBNMCTL, BNM_SYS_RESET_N )
            );
    reg |= 
            BCHP_FIELD_DATA(SUB_BNMCTL_HOSTBNMCTL, BNM_SYS_RESET_N,  1 )	
    ;
    BREG_Write32(hReg, BCHP_SUB_BNMCTL_HOSTBNMCTL, reg);

    BKNI_Sleep(1);

    reg = BREG_Read32(hReg, BCHP_SUB_BNMCTL_HOSTBNMCTL);
    reg &= ~(
            BCHP_MASK(SUB_BNMCTL_HOSTBNMCTL, MIPS_RELEASE )
            );
    reg |= 
            BCHP_FIELD_DATA(SUB_BNMCTL_HOSTBNMCTL, MIPS_RELEASE,  1 )
    ;
    BREG_Write32(hReg, BCHP_SUB_BNMCTL_HOSTBNMCTL, reg);

#endif

    BDBG_MSG((" NEXUS_Platform_WakeupCM write done 0x%x", BCHP_SUB_BNMCTL_HOSTBNMCTL));
#endif

    return NEXUS_SUCCESS;

}

NEXUS_Error NEXUS_Platform_P_ShutdownCM(void)
{
    /*Platform dependent code to shutdown eCM*/

#if 0
    volatile unsigned int *bnmctl = (unsigned int *)0xb4000028;

    *bnmctl |= 1;   /* Release the UBUS*/
    *bnmctl |= 2;   /* Release the MIPS*/
#endif

    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    BDBG_MSG((" NEXUS_Platform_ShutdownCM will write 0x%x", BCHP_SUB_BNMCTL_HOSTBNMCTL));
    reg = BREG_Read32(hReg, BCHP_SUB_BNMCTL_HOSTBNMCTL);
    
#if 0
    reg &= ~(
            BCHP_MASK(SUB_BNMCTL_HOSTBNMCTL, MIPS_RELEASE ) |
            BCHP_MASK(SUB_BNMCTL_HOSTBNMCTL, BNM_SYS_RESET_N )
            );
    reg |= 
            BCHP_FIELD_DATA(SUB_BNMCTL_HOSTBNMCTL, MIPS_RELEASE,  0 ) | 
            BCHP_FIELD_DATA(SUB_BNMCTL_HOSTBNMCTL, BNM_SYS_RESET_N,  0 )	
    ;
#else 
    reg &= ~(
            BCHP_MASK(SUB_BNMCTL_HOSTBNMCTL, MIPS_RELEASE )
            );
    reg |= 
            BCHP_FIELD_DATA(SUB_BNMCTL_HOSTBNMCTL, MIPS_RELEASE,  0 )	
    ;
#endif

    BREG_Write32(hReg, BCHP_SUB_BNMCTL_HOSTBNMCTL, reg);
    BDBG_MSG((" BCHP_SUB_BNMCTL_HOSTBNMCTL 0x%x", BREG_Read32(hReg, BCHP_SUB_BNMCTL_HOSTBNMCTL)));

    return NEXUS_SUCCESS;
}

bool NEXUS_Platform_P_CMPowerIsOn(void)
{
    /*Platform dependent code to shutdown eCM*/

    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;
    bool on;

    reg = BREG_Read32(hReg, BCHP_SUB_BNMCTL_HOSTBNMCTL); 
    reg &= (BCHP_MASK(SUB_BNMCTL_HOSTBNMCTL, MIPS_RELEASE));

    on = (reg == BCHP_FIELD_DATA(SUB_BNMCTL_HOSTBNMCTL, MIPS_RELEASE, 1 ));
    BDBG_MSG(("%s() = %d",__FUNCTION__,on));

    return on;
}

NEXUS_Error NEXUS_Platform_ReadFrontendControlSharedMemory(uint32_t *pHostControlledFrontend)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    /* Map physical memory into uncached local address space */
    uint32_t bnmDestBase;
    uint32_t bnmDestMask;
    uint32_t memSize;
    uint32_t memBase;
    NEXUS_Error errCode = NEXUS_SUCCESS;

    bnmDestBase = BREG_Read32(hReg, BCHP_SUB_UBUS_SGISB_BNM_DDR_DEST_BASE);   /*(unsigned int *)0xb4880950;*/
    bnmDestMask = BREG_Read32(hReg, BCHP_SUB_UBUS_SGISB_BNM_DDR_WMASK);   /*(unsigned int *)0xb4880958;*/

    memSize = ~(bnmDestMask | 0xe0000000) + 1;
    memBase = (bnmDestBase & 0xfffffff8);
    
    memBase = (uint32_t)NEXUS_Platform_P_MapMemory(memBase, memSize, false);
    if (memBase)
    {
        *pHostControlledFrontend = *((uint32_t *)(memBase + memSize - 8));
        BDBG_MSG((" Read from shared memory: memBase 0x%x memSize 0x%x = 0x%x", memBase, memSize, *pHostControlledFrontend));
        NEXUS_Platform_P_UnmapMemory((void*)memBase, memSize, NEXUS_MemoryMapType_eUncached);
    }
    else
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    return errCode;
}

NEXUS_Error NEXUS_Platform_WriteFrontendControlSharedMemory(uint32_t hostControlledFrontend)
{
    BREG_Handle hReg = g_pCoreHandles->reg;

    /* Map physical memory into uncached local address space */
    uint32_t bnmDestBase;
    uint32_t bnmDestMask;
    uint32_t memSize;
    uint32_t memBase;
    NEXUS_Error errCode = NEXUS_SUCCESS;


    bnmDestBase = BREG_Read32(hReg, BCHP_SUB_UBUS_SGISB_BNM_DDR_DEST_BASE);   /*(unsigned int *)0xb4880950;*/
    bnmDestMask = BREG_Read32(hReg, BCHP_SUB_UBUS_SGISB_BNM_DDR_WMASK);   /*(unsigned int *)0xb4880958;*/

    memSize = ~(bnmDestMask | 0xe0000000) + 1;
    memBase = (bnmDestBase & 0xfffffff8);
    
    BDBG_MSG((" Write to shared memory: memBase 0x%x memSize 0x%x", memBase, memSize));
    memBase = (uint32_t)NEXUS_Platform_P_MapMemory(memBase, memSize, false);
    if (memBase)
    {
        *((uint32_t *)(memBase + memSize - 8)) = hostControlledFrontend;    
        NEXUS_Platform_P_UnmapMemory((void*)memBase,memSize,NEXUS_MemoryMapType_eUncached);
    }
    else
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    return errCode;
}


#else
NEXUS_Error NEXUS_Platform_SetFrontendStandbySetting(const NEXUS_PlatformFrontendStandby_Settings *pSettings /* [in] */)
{
	BSTD_UNUSED(pSettings);
	return NEXUS_NOT_SUPPORTED;
}

NEXUS_Error NEXUS_Platform_GetFrontendStandbySetting(NEXUS_PlatformFrontendStandby_Settings *pSettings/* [out] */)
{
	BSTD_UNUSED(pSettings);

	return NEXUS_NOT_SUPPORTED;
}

NEXUS_Error NEXUS_Platform_GetDefaultFrontendStandbySetting(NEXUS_PlatformFrontendStandby_Settings *pSettings/* [out] */)
{
	BSTD_UNUSED(pSettings);

	return NEXUS_NOT_SUPPORTED;
}

NEXUS_Error NEXUS_Platform_GetFrontendStandbyMode(NEXUS_PlatformFrontendStandbyMode * pMode /* [out] */)
{
	BSTD_UNUSED(pMode);
    
	return NEXUS_NOT_SUPPORTED;
}

NEXUS_Error NEXUS_Platform_SetFrontendStandbyMode(const NEXUS_PlatformFrontendStandbyMode Mode /* [in] */)
{

    BSTD_UNUSED(Mode);
    return NEXUS_NOT_SUPPORTED;
}
NEXUS_Error NEXUS_Platform_WakeupCM(void)
{
    return NEXUS_NOT_SUPPORTED;
}

NEXUS_Error NEXUS_Platform_ReadFrontendControlSharedMemory(uint32_t *pHostControlledFrontend)
{
    *pHostControlledFrontend = 0;
    return NEXUS_NOT_SUPPORTED;
}

NEXUS_Error NEXUS_Platform_WriteFrontendControlSharedMemory(uint32_t hostControlledFrontend)
{
    BSTD_UNUSED(hostControlledFrontend);
    return NEXUS_NOT_SUPPORTED;
}

bool NEXUS_Platform_P_CMPowerIsOn(void)
{
    return true;
}

#endif

