/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date:
 *
 * Module Description:
 *   See Module Overview below.
 *
 * Revision History:
 *
 * $brcm_Log: $
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
#include "bchp_avs_cpu_data_mem.h"
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
#if (BCHP_CHIP==7145 && BCHP_VER >= BCHP_VER_B0) || \
    (BCHP_CHIP==7445 && BCHP_VER >= BCHP_VER_D0)
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
#define voltage_register     (BCHP_AVS_CPU_DATA_MEM_WORDi_ARRAY_BASE + (2*4))
#define temperature_register (BCHP_AVS_CPU_DATA_MEM_WORDi_ARRAY_BASE + (3*4))

/* Because we may have started up without AVS firmware running, we need to provide data in both situations.
 * If the firmware is running then it is updating the current data in the above locations.
 * If it is not running (i.e. above locations are always zero) then get the data ourselves.
 */
static void AvsGetData(BCHP_P_AvsHandle hHandle, unsigned *voltage, signed *temperature, bool *firmware_running)
{
    uint32_t v_reg, t_reg;

    *voltage = BREG_Read32(hHandle->hRegister, voltage_register);
    *temperature = BREG_Read32(hHandle->hRegister, temperature_register);
    *firmware_running = true; /* assume its running */

    if (!*voltage)
    {
#ifdef BCHP_AVS_RO_REGISTERS_0_1_PVT_1V_0_MNTR_STATUS /* dual AVS_MONITORs, i.e. 7145 */
        v_reg = BREG_Read32(hHandle->hRegister, BCHP_AVS_RO_REGISTERS_0_1_PVT_1V_0_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_1_PVT_1V_0_MNTR_STATUS_data_MASK;
        t_reg = BREG_Read32(hHandle->hRegister, BCHP_AVS_RO_REGISTERS_0_1_PVT_TEMPERATURE_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_1_PVT_TEMPERATURE_MNTR_STATUS_data_MASK;
#else
        v_reg = BREG_Read32(hHandle->hRegister, BCHP_AVS_RO_REGISTERS_0_PVT_1V_0_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_PVT_1V_0_MNTR_STATUS_data_MASK;
        t_reg = BREG_Read32(hHandle->hRegister, BCHP_AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS_data_MASK;
#endif
#ifdef USE_NEW_CONVERSION_FORMULAS
        /* formula is: voltage=((v/1024)*880/0.7) */
        *voltage = (v_reg * 8800U) / 7168U;
        /* formula is: temperature=410.04-(t*0.48705) */
        *temperature = (4100400 - (signed)(t_reg * 4870)) / 10;
#else
        /* formula is: voltage=((v/1024)*877.9/0.7) */
        *voltage = (v_reg * 8779U) / 7168U;
        /* formula is: temperature=(854.8-t)/2.069 */
        *temperature = 1000 * (854800 - (signed)(t_reg*1000)) / 2069;
#endif
        *firmware_running = false;
    }

#ifdef BCHP_AVS_TMON_REG_START
    /* In newer products we have a AVS_TMON block that returns a different temperature than AVS_MONITOR */
    /* This temperature reads from a hotter location on the chip so we want this value to be used if available. */
    t_reg = (BREG_Read32(hHandle->hRegister, BCHP_AVS_TMON_TEMPERATURE_MEASUREMENT_STATUS) & BCHP_AVS_TMON_TEMPERATURE_MEASUREMENT_STATUS_data_MASK) >> 1;
    /* formula is: temperature=410.04-(t*0.48705) */
    *temperature = (4100400 - (signed)(t_reg * 4870)) / 10;
#endif
}

/* This gets called once a second to monitor the voltage and temperatures */
BERR_Code BCHP_P_AvsMonitorPvt ( BCHP_P_AvsHandle hHandle )
{
    unsigned voltage, dac;
    signed temperature;
    bool firmware_running;

    BDBG_ENTER(BCHP_Monitor_Pvt);
    BDBG_OBJECT_ASSERT(hHandle, bchp_avs_t);

    AvsGetData(hHandle, &voltage, &temperature, &firmware_running);

#ifdef BCHP_AVS_PVT_MNTR_CONFIG_1_DAC_CODE /* i.e. 7145 */
    dac = BREG_Read32(hHandle->hRegister, BCHP_AVS_PVT_MNTR_CONFIG_1_DAC_CODE);
#else
    dac = BREG_Read32(hHandle->hRegister, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE);
#endif

    /* If we have been placed in stand-by mode, we don't touch any registers */
    /*if (hHandle->standby) return BERR_TRACE(BERR_UNKNOWN);*/
    if (hHandle->standby) return BERR_UNKNOWN;

    /* We don't do any "processing", just report the current status */
    /* This is to help people to build reports containing periodic temperature and voltage status */
    BDBG_MSG(("Voltage = %d.%03dV, DAC = %d (0x%x), Temp = [%c%d.%03dC] %s",
        mantissa(voltage), fraction(voltage), dac, dac,
        sign(temperature), mantissa(temperature), fraction(temperature),
        firmware_running?"alive":"dead"));

    BDBG_LEAVE(BCHP_Monitor_Pvt);
    return BERR_SUCCESS;
}

BERR_Code BCHP_P_AvsGetData (
    BCHP_P_AvsHandle hHandle, /* [in] handle supplied from open */
    BCHP_AvsData *pData )     /* [out] location to put data */
{
    unsigned voltage;
    signed temperature;
    bool firmware_running;

    BDBG_ASSERT(pData);

    BDBG_ENTER(BCHP_AvsGetData);

    AvsGetData(hHandle, &voltage, &temperature, &firmware_running);

    pData->voltage = voltage;
    pData->temperature = temperature;
    pData->enabled  = firmware_running?true:false;
    pData->tracking = firmware_running?true:false;

    BDBG_LEAVE(BCHP_AvsGetData);
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
    /* zero it (in case they didn't) so they don't get junk */
    BKNI_Memset((void*)data, 0x0, sizeof(*data));
#endif
}
