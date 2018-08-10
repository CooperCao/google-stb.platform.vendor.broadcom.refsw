/***************************************************************************
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
 *
 * Module Description:
 *   See Module Overview below.
 *
 ***************************************************************************/

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "bchp.h"
#include "bchp_priv.h"
#include "bchp_common.h"

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#include "bchp_pwr_resources.h"
#include "bchp_aon_ctrl.h"
#endif

BDBG_MODULE(BCHP_AVS);

#include "bchp_avs.h"
#include "bchp_avs_priv.h"
#ifdef BCHP_AVS_CPU_DATA_MEM_REG_START
#include "bchp_avs_cpu_data_mem.h"
#endif
#ifdef BCHP_AVS_RO_REGISTERS_0_1_REG_START
#include "bchp_avs_pvt_mntr_config_1.h"
#include "bchp_avs_ro_registers_0_1.h"
#else
#include "bchp_avs_pvt_mntr_config.h"
#include "bchp_avs_ro_registers_0.h"
#endif
#ifdef BCHP_AVS_TMON_REG_START
#include "bchp_avs_tmon.h"
#endif

/* Make sure this is undefined until we're ready... */
#undef BCHP_PWR_RESOURCE_AVS

/* The conversion formulas changed on newer products */
#if (BCHP_CHIP==7445 && BCHP_VER >= BCHP_VER_D0)
#define USE_NEW_CONVERSION_FORMULAS
#endif

BDBG_OBJECT_ID(bchp_avs_t);

/* This is the contect for this driver -- users use an opaque handle */
struct BCHP_P_AvsContext {
    BDBG_OBJECT(bchp_avs_t)    /* used to check if structure is valid */

    BCHP_Handle hChip;     /* the handle for the chip open */
    BREG_Handle hRegister; /* the register handle provided on open */

    bool standby;   /* set to true to pause the AVS processing (low-power mode) */
    bool tracking;  /* flag to tell if AVS code is tracking */
    AvsLockType_t lock_type;
};

/* forward references: */
#ifdef BCHP_PWR_RESOURCE_AVS
static void AvsSaveRegisters(BCHP_P_AvsHandle handle);
static void AvsRestoreRegisters(BCHP_P_AvsHandle handle, bool restore);
/*static int AvsCheckSaveRegisters(BCHP_P_AvsHandle handle);*/
#endif

/* We're not allowed to use any of the standard library macros or functions due to build conflicts */
#define AvsAbs(x) (((x)<0)?-(x):(x))

/* In order to get rid of the floating points, do the math and print the parts */
#define sign(f) ((f)<0)?'-':' ' /*space means positive*/
#define mantissa(f) (AvsAbs((int)(f)/1000))
#define fraction(f) (AvsAbs((int)((f) - (f)/1000*1000)))

/*\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\*/

BERR_Code BCHP_P_AvsOpen (
    BCHP_P_AvsHandle *phHandle,   /* [out] returns new handle on success */
    BCHP_Handle       hChip)      /* [in] handle for chip data */
{
    BCHP_P_AvsHandle handle;
    BERR_Code rc = BERR_SUCCESS;

    /* Make sure they gave me a place to return the handle and valid handles I'll need */
    BDBG_ASSERT(phHandle);
    BDBG_ASSERT(hChip);
    BDBG_ASSERT(hChip->regHandle);

    BDBG_ENTER(BCHP_AvsOpen);

    /* If error ocurr user get a NULL *phHandle */
    *phHandle = NULL;

    /* Alloc the base chip context. */
    handle = (BCHP_P_AvsHandle) (BKNI_Malloc(sizeof(struct BCHP_P_AvsContext)));
    if (!handle)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)handle, 0x0, sizeof(struct BCHP_P_AvsContext));
    BDBG_OBJECT_SET(handle, bchp_avs_t);

    /* Need register handle for accessing my registers. */
    handle->hRegister = hChip->regHandle;
    handle->hChip     = hChip;

#ifdef BCHP_PWR_RESOURCE_AVS
    rc = BCHP_PWR_AcquireResource(hChip, BCHP_PWR_RESOURCE_AVS);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("Failed to acquire the AVS resource"));
        BKNI_Free(handle);
        return BERR_TRACE(BERR_UNKNOWN);
    }
#endif

    handle->tracking = true;  /* we're only NOT tracking when we asked FW to pause */

    *phHandle = handle; /*success -- return the handle*/

    BDBG_LEAVE(BCHP_AvsOpen);
    return rc;
}

BERR_Code BCHP_P_AvsClose ( BCHP_P_AvsHandle hHandle )
{
    BDBG_ENTER(BCHP_AvsClose);
    BDBG_OBJECT_ASSERT(hHandle, bchp_avs_t);

    BDBG_MSG(("AVS Close called"));

#ifdef BCHP_PWR_RESOURCE_AVS
    BCHP_PWR_ReleaseResource(hHandle->hChip, BCHP_PWR_RESOURCE_AVS);
#endif
    BDBG_OBJECT_DESTROY(hHandle, bchp_avs_t);
    BKNI_Free(hHandle);

    BDBG_LEAVE(BCHP_AvsClose);
    return BERR_SUCCESS;
}

/* This is where these values come from IF the AVS firmware is running */
/* The firmware updates these values every second */
#if (BCHP_CHIP==7260) || (BCHP_CHIP==7268) || (BCHP_CHIP==7271) || (BCHP_CHIP==7278) || (BCHP_CHIP==7255)
#define AVS_FW_INTERFACE_DVFS
#endif

#ifdef BCHP_AVS_RO_REGISTERS_0_1_PVT_1V_0_MNTR_STATUS /* dual AVS_MONITORs, i.e. 7145 */
#define AVS_DUAL_MONITOR
#endif

#define MAX_AVS_DOMAIN_CNT 2

typedef enum {
    AVS_MSG_IDX_COMMAND,    /*  0 */
    AVS_MSG_IDX_STATUS,     /*  1 */
    AVS_MSG_IDX_VOLT0,      /*  2 */
    AVS_MSG_IDX_TEMP0,      /*  3 */
    AVS_MSG_IDX_PV0,        /*  4 */
    AVS_MSG_IDX_MV0,        /*  5 */
#ifdef AVS_FW_INTERFACE_DVFS
    AVS_MSG_IDX_COMMAND_P0, /*  6 */
    AVS_MSG_IDX_COMMAND_P1, /*  7 */
    AVS_MSG_IDX_COMMAND_P2, /*  8 */
    AVS_MSG_IDX_COMMAND_P3, /*  9 */
#else
    AVS_MSG_IDX_VOLT1,      /*  6 */
    AVS_MSG_IDX_TEMP1,      /*  7 */
    AVS_MSG_IDX_PV1,        /*  8 */
    AVS_MSG_IDX_MV1,        /*  9 */
#endif
    AVS_MSG_IDX_REVISION,   /* 10 */
    AVS_MSG_IDX_STATE,      /* 11 */
    AVS_MSG_IDX_HEARTBEAT,  /* 12 */
    AVS_MSG_IDX_AVS_MAGIC,  /* 13 */
    AVS_MSG_IDX_SIGMA_HVT,  /* 14 */
    AVS_MSG_IDX_SIGMA_SVT   /* 15 */
#ifdef AVS_FW_INTERFACE_DVFS
    ,
    AVS_MSG_IDX_VOLT1,      /* 16 */
    AVS_MSG_IDX_TEMP1,      /* 17 */
    AVS_MSG_IDX_PV1,        /* 18 */
    AVS_MSG_IDX_MV1         /* 19 */
#endif
} AVS_MSG_IDX;


#ifndef BCHP_AVS_CPU_DATA_MEM_REG_START
static const uint32_t avs_cpu_memory[128];
#define BCHP_AVS_CPU_DATA_MEM_WORDi_ARRAY_BASE (*(uint32_t*)&avs_cpu_memory)
#endif

#define VREG_ADDR0 (BCHP_AVS_CPU_DATA_MEM_WORDi_ARRAY_BASE + 4*AVS_MSG_IDX_VOLT0)
#define VREG_ADDR1 (BCHP_AVS_CPU_DATA_MEM_WORDi_ARRAY_BASE + 4*AVS_MSG_IDX_VOLT1)

#define TREG_ADDR0 (BCHP_AVS_CPU_DATA_MEM_WORDi_ARRAY_BASE + 4*AVS_MSG_IDX_TEMP0)
#define TREG_ADDR1 (BCHP_AVS_CPU_DATA_MEM_WORDi_ARRAY_BASE + 4*AVS_MSG_IDX_TEMP1)

#define HB_ADDR0 (BCHP_AVS_CPU_DATA_MEM_WORDi_ARRAY_BASE + 4*AVS_MSG_IDX_HEARTBEAT)

#if 0
static const uint32_t vreg_addr[] = {
    (BCHP_AVS_CPU_DATA_MEM_WORDi_ARRAY_BASE + 4*AVS_MSG_IDX_VOLT0),
    (BCHP_AVS_CPU_DATA_MEM_WORDi_ARRAY_BASE + 4*AVS_MSG_IDX_VOLT1)
};
static const uint32_t treg_addr[] = {
    (BCHP_AVS_CPU_DATA_MEM_WORDi_ARRAY_BASE + 4*AVS_MSG_IDX_TEMP0),
    (BCHP_AVS_CPU_DATA_MEM_WORDi_ARRAY_BASE + 4*AVS_MSG_IDX_TEMP1)
};
#endif


/* Because we may have started up without AVS firmware running, we need to provide data in both situations.
 * If the firmware is running then it is updating the current data in the above locations.
 * If it is not running (i.e. above locations are always zero) then get the data ourselves.
 */
static void GetAvsData_isrsafe(BCHP_P_AvsHandle hHandle, unsigned *voltage0, unsigned *voltage1, signed *temperature, bool *firmware_running, unsigned *heartbeat)
{
#if (BCHP_CHIP != 7255) /* TODO : Detect this at runtime */
    uint32_t v_reg0;
    uint32_t v_reg1;
    uint32_t t_reg;

    *firmware_running = true; /* assume its running */

    *voltage0 = BREG_Read32(hHandle->hRegister, VREG_ADDR0);
    *voltage1 = BREG_Read32(hHandle->hRegister, VREG_ADDR1);
    *temperature = BREG_Read32(hHandle->hRegister, TREG_ADDR0);
    *heartbeat = BREG_Read32(hHandle->hRegister, HB_ADDR0);

    {
        BDBG_MSG(("v0=%08x v1=%08x  t0=%08x t1=%08x  rev=%08x  beat=%08x",
            *voltage0,
            *voltage1,
            *temperature,
            BREG_Read32(hHandle->hRegister, TREG_ADDR1),
            BREG_Read32(hHandle->hRegister, BCHP_AVS_CPU_DATA_MEM_WORDi_ARRAY_BASE + 4*AVS_MSG_IDX_REVISION),
            *heartbeat
        ));
    }

    if (*voltage0 == 0)
    {
#ifdef AVS_DUAL_MONITOR
        v_reg0 = BREG_Read32(hHandle->hRegister, BCHP_AVS_RO_REGISTERS_0_1_PVT_1V_0_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_1_PVT_1V_0_MNTR_STATUS_data_MASK;
        v_reg1 = BREG_Read32(hHandle->hRegister, BCHP_AVS_RO_REGISTERS_0_1_PVT_1V_1_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_1_PVT_1V_0_MNTR_STATUS_data_MASK;
        t_reg  = BREG_Read32(hHandle->hRegister, BCHP_AVS_RO_REGISTERS_0_1_PVT_TEMPERATURE_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_1_PVT_TEMPERATURE_MNTR_STATUS_data_MASK;
#else
        v_reg0 = BREG_Read32(hHandle->hRegister, BCHP_AVS_RO_REGISTERS_0_PVT_1V_0_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_PVT_1V_0_MNTR_STATUS_data_MASK;
        v_reg1 = BREG_Read32(hHandle->hRegister, BCHP_AVS_RO_REGISTERS_0_PVT_1V_1_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_PVT_1V_0_MNTR_STATUS_data_MASK;
        t_reg  = BREG_Read32(hHandle->hRegister, BCHP_AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS_data_MASK;
#endif

#ifdef USE_NEW_CONVERSION_FORMULAS
        /* formula is: voltage=((v/1024)*880/0.7) */
        /* formula is: temperature=410.04-(t*0.48705) */
        *voltage0 = (v_reg0 * 8800U) / 7168U;
        *voltage1 = (v_reg1 * 8800U) / 7168U;
        *temperature = (4100400 - (signed)(t_reg * 4870)) / 10;
#else
        /* formula is: voltage=((v/1024)*877.9/0.7) */
        /* formula is: temperature=(854.8-t)/2.069 */
        *voltage0 = (v_reg0 * 8779U) / 7168U;
        *voltage1 = (v_reg1 * 8779U) / 7168U;
        *temperature = 1000 * (854800 - (signed)(t_reg*1000)) / 2069;
#endif

        *firmware_running = false;
    }

    if (*voltage1 == 0)
        *voltage1 = 0xffffffff;

#ifdef BCHP_AVS_TMON_REG_START
    /* In newer products we have a AVS_TMON block that returns a different temperature than AVS_MONITOR */
    /* This temperature reads from a hotter location on the chip so we want this value to be used if available. */
    t_reg = (BREG_Read32(hHandle->hRegister, BCHP_AVS_TMON_TEMPERATURE_MEASUREMENT_STATUS) & BCHP_AVS_TMON_TEMPERATURE_MEASUREMENT_STATUS_data_MASK) >> 1;
    /* formula is: temperature=410.04-(t*0.48705) */
    *temperature = (4100400 - (signed)(t_reg * 4870)) / 10;
#endif

#else /* (BCHP_CHIP != 7255) */

#if BCHP_UNIFIED_IMPL
    BERR_Code rc;
    BCHP_Handle hChip = hHandle->hChip;
    if (hChip->openSettings.getAvsData) {
        rc = hChip->openSettings.getAvsData(AVS_MSG_IDX_VOLT0, voltage0);
        if (rc) {rc = BERR_TRACE(rc);}
        rc = hChip->openSettings.getAvsData(AVS_MSG_IDX_VOLT1, voltage1);
        if (rc) {rc = BERR_TRACE(rc);}
        rc = hChip->openSettings.getAvsData(AVS_MSG_IDX_TEMP0, (unsigned*)temperature);
        if (rc) {rc = BERR_TRACE(rc);}
        rc = hChip->openSettings.getAvsData(AVS_MSG_IDX_HEARTBEAT, heartbeat);
        if (rc) {rc = BERR_TRACE(rc);}
        *firmware_running = *voltage0 == 0?false:true;
    }
#endif
#endif
}

/* This gets called once a second to monitor the voltage and temperatures */
BERR_Code BCHP_P_AvsMonitorPvt ( BCHP_P_AvsHandle hHandle )
{
    unsigned voltage0;
    unsigned voltage1;
    signed temperature;
    bool firmware_running;
    unsigned heartbeat;
#if BDBG_DEBUG_BUILD
    unsigned dac;
#endif

    BDBG_ENTER(BCHP_Monitor_Pvt);
    BDBG_OBJECT_ASSERT(hHandle, bchp_avs_t);

    /* If we have been placed in stand-by mode, we don't touch any registers */
    /*if (hHandle->standby) return BERR_TRACE(BERR_UNKNOWN);*/
    if (hHandle->standby) return BERR_UNKNOWN;

#if BDBG_DEBUG_BUILD
#ifdef BCHP_AVS_PVT_MNTR_CONFIG_1_DAC_CODE /* i.e. 7145 */
    dac = BREG_Read32(hHandle->hRegister, BCHP_AVS_PVT_MNTR_CONFIG_1_DAC_CODE);
#else
    dac = BREG_Read32(hHandle->hRegister, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE);
#endif
#endif

    GetAvsData_isrsafe(hHandle, &voltage0, &voltage1, &temperature, &firmware_running, &heartbeat);

    /* We don't do any "processing", just report the current status */
    /* This is to help people to build reports containing periodic temperature and voltage status */
    BDBG_MSG(("Voltage0 = %d.%03dV, Voltage1 = %d.%03dV, DAC = %d (0x%x), Temp = [%c%d.%03dC] %s",
        mantissa(voltage0), fraction(voltage0),
        mantissa(voltage1), fraction(voltage1),
        dac, dac,
        sign(temperature), mantissa(temperature), fraction(temperature),
        firmware_running?"alive":"dead"));

    BDBG_LEAVE(BCHP_Monitor_Pvt);
    return BERR_SUCCESS;
}

BERR_Code BCHP_P_GetAvsData_isrsafe (
    BCHP_P_AvsHandle hHandle, /* [in] handle supplied from open */
    BCHP_AvsData *pData )     /* [out] location to put data */
{
    unsigned voltage0;
    unsigned voltage1;
    signed temperature;
    bool firmware_running = false;
    unsigned heartbeat;

    BDBG_ASSERT(pData);

    BDBG_ENTER(BCHP_P_GetAvsData_isrsafe);

    GetAvsData_isrsafe(hHandle, &voltage0, &voltage1, &temperature, &firmware_running, &heartbeat);

    pData->voltage = voltage0;
    pData->temperature = temperature;
    pData->enabled  = firmware_running?true:false;
    pData->tracking = firmware_running?true:false;
    pData->voltage1 = voltage1;
    pData->temperature1 = temperature;
    pData->heartbeat = heartbeat;

    BDBG_MSG(("voltage0=%d  voltage1=%d  temperature=%d  heartbeat=%d", pData->voltage, pData->voltage1, pData->temperature, heartbeat));

    BDBG_LEAVE(BCHP_P_GetAvsData_isrsafe);
    return BERR_SUCCESS;
}

BERR_Code BCHP_P_AvsStandbyMode(
    BCHP_P_AvsHandle hHandle, /* [in] handle supplied from open */
    bool activate)            /* [in] true to enter low power mode */
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER(BCHP_P_AvsLowPowerMode);
    BDBG_OBJECT_ASSERT(hHandle, bchp_avs_t);

#ifndef BCHP_PWR_RESOURCE_AVS
    BSTD_UNUSED(activate);
#else
    BDBG_MSG(("%s AVS standby-mode", activate?"Entering":"Exiting"));

    if (activate)
    {
        hHandle->standby = true; /* stop register accesses BEFORE we go to low power mode */
        AvsSaveRegisters(hHandle);
#ifdef BCHP_PWR_RESOURCE_AVS
        rc = BCHP_PWR_ReleaseResource(hHandle->hChip, BCHP_PWR_RESOURCE_AVS);
#endif
    }
    else
    {
#ifdef BCHP_PWR_RESOURCE_AVS
        rc = BCHP_PWR_AcquireResource(hHandle->hChip, BCHP_PWR_RESOURCE_AVS);
#endif
        AvsRestoreRegisters(hHandle, true);
        hHandle->standby = false;  /* re-enable register accesses AFTER we return from low power mode */
    }

#endif /*BCHP_PWR_RESOURCE_AVS*/

    BDBG_LEAVE(BCHP_P_AvsLowPowerMode);
    return rc;
}

/*\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\*/

#ifdef BCHP_PWR_RESOURCE_AVS
static void AvsSaveRegisters(BCHP_P_AvsHandle handle)
{
}

static void AvsRestoreRegisters(BCHP_P_AvsHandle handle, bool restore)
{
}
#endif /*BCHP_PWR_RESOURCE_AVS*/

/*\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\*/

/* The following are included for test purposes */

BERR_Code AvsLock(BCHP_P_AvsHandle handle, AvsLockType_t type)
{
    BDBG_ENTER(BCHP_AvsLock);
    if (!type) AvsUnlock(handle); /* make eNoLock case synonymous with unlock */
    handle->lock_type = type;
    if (type == eAvsLockRegisters) handle->tracking = false; /* not tracking when locked */
    BDBG_LEAVE(BCHP_AvsLock);
    return BERR_SUCCESS;
}

BERR_Code AvsUnlock(BCHP_P_AvsHandle handle)
{
    BDBG_ENTER(BCHP_AvsUnlock);
    handle->lock_type = eAvsNoLock;
    handle->tracking = true;
    BDBG_LEAVE(BCHP_AvsUnlock);
    return BERR_SUCCESS;
}

void AvsGetTestData(BCHP_P_AvsHandle handle, AvsTestData *data)
{
    BDBG_ASSERT(handle);
    BDBG_ASSERT(data);

#if 0
    data->valid = false;
    if (!handle->initialized) return;

    data->last_dac  = handle->last_dac;
    data->last_temp = handle->last_temp;

    data->V_0p99  = handle->V_0p99;
    data->V_1p1_0 = handle->V_1p1_0;
    data->V_1p1_1 = handle->V_1p1_1;
    data->V_2p75  = handle->V_2p75;
    data->V_3p63  = handle->V_3p63;

    data->last_voltage_1p1_0 = handle->last_voltage_1p1_0;
    data->last_voltage_1p1_1 = handle->last_voltage_1p1_1;
    data->last_voltage_0p99  = handle->last_voltage_0p99;
    data->last_voltage_2p75  = handle->last_voltage_2p75;
    data->last_voltage_3p63  = handle->last_voltage_3p63;

    data->valid = true;
#else
    BSTD_UNUSED (handle);

    /* zero it (in case they didn't) so they don't get junk */
    BKNI_Memset((void*)data, 0x0, sizeof(*data));
#endif
}
