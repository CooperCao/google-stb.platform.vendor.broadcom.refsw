/******************************************************************************
 * Copyright (C) 2006-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/
#if 0
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h> /*for abs*/
#endif

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "bchp.h"
#include "bchp_priv.h"
#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#include "bchp_pwr_resources.h"
#include "bchp_aon_ctrl.h"
#endif

BDBG_MODULE(BCHP_AVS);

#include "bchp_avs.h"
#include "bchp_avs_priv.h"

/* TODO:
** 2) Add support for saving and checking registers on each "monitor" call (using AvsCheckSaveRegisters).
** 3) Add more parameter checking (now that we're exporting more functions for testing).
*/

/* Compile time options: */
/*#define ALLOW_CONSTANT_VOLTAGE*/ /* define to allow the use of constant voltage process for FF parts */

/* There are two generations of AVS hardware that require different handling. */
/* Statically decide which part uses which generation of AVS hardware. */
/* Handle older parts or parts that have not updated to 2nd generation AVS hardware. */
#if (((BCHP_CHIP==7231 || BCHP_CHIP==7344 || BCHP_CHIP==7346 || BCHP_CHIP==7422 || BCHP_CHIP==7425) && (BCHP_VER < BCHP_VER_B0)) \
    || (BCHP_CHIP==7358 && BCHP_VER < BCHP_VER_B0)  /* we're at A2 -- they should use 2nd bump in Bx revs */ \
    || (BCHP_CHIP==7552 && BCHP_VER <= BCHP_VER_B0) /* we're already at B0 revision using 1st bump */ \
    )
  #define AVS_GENERATION 1
#else
  #define AVS_GENERATION 2 /* all other parts default to 2nd generation */
#endif

/* The 7358 uses different design for AON registers (leave out until they decide how to use them) */
#if (BCHP_CHIP!=7358)
#define SAVE_TO_AON /* save the DAC_CODE and PVT_MON_CTRL registers to AON memory space as part of S3 shutdown */
#endif

/* We need to store four things across re-starts of the API:
** 1) the original predicted voltage (also set by CFE)
** 2) the fact that the thresolds were set by CFE
** 3) the original DAC value set by CFE
** 4) the final DAC value that was being used before the close
**
** We reserve the first two sets (four registers) of the central oscillator thresholds for storing information across restarts of the PI.
** CFE uses the first lower threshold (threshold1_0) for storing the predicted voltage value.  This is used to identify FF parts.
** FF parts requires a special initialization process so that we can still run the part at a lower voltage than if we used the default.
** The second lower threshold (threshold1_1) is used by CFE to store a flag to indicate that it already initialized the threshold registers at startup.
**
** Because we always bump the starting DAC value by a margin on initialization we would be bumping the voltage on each subsequent
** run allowing the voltage to "creep" up un-necessarily.  In order to correct for this, two central oscillator are reserved.
** We use the first upper threshold (threshold2_0) for storing the original DAC value and restore it on each subsequent run.
** If we closed properly we'll restart at the voltage value (DAC value) before the close.
** We use the second upper threshold (threshold2_1) for storing the close DAC value and restore it on each subsequent run.
** If we didn't close properly then we didn't get a chance to save the DAC so we'll revert to the DAC value set by CFE.
**
*/
#define STARTING_OSCILLATOR 2 /* skip oscillator sets 0 and 1 when using central oscillator thresholds */

/* This is where CFE stored the predicted voltage value on startup.
** This is used to determine if the part is FF part (in whic case we process the voltage differently.
** THIS MUST MATCH THE REGISTER AND VALUE FROM CFE (so if you change it then you'll need to change CFE too!).
*/
#define PREDICTED_VOLTAGE BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_0

/* This is where CFE set the flag that indicates that it set the thresholds already (and we don't need to do that here).
** THIS MUST MATCH THE REGISTER AND VALUE FROM CFE (so if you change it then you'll need to change CFE too!).
** The value MUST match or we'll re-do the setting of the thresholds.
** That means backward compatibility with CFEs that don't have this update.
*/
#define THRESHOLDS_SET BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_1

/* This is where we're going to save the starting DAC value for subsequent restarts */
#define SAVED_DAC  BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_0

/* This is where we saved the closing DAC value when we (cleanly) shutdown */
#define CLOSED_DAC BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_1

/* We need to get some data from CFE and also provide some data to CFE.
** We use some of the unused threshold registers to pass data (CFE->PI) and some AON registers to pass data back (PI->CFE).
** This magic value is used to indicate that the data is valid and not some random junk written to a register.
** This MUST match the value used by CFE -- do not change it!
*/
#define AVS_CFE_MAGIC 0x7735 /* ("sell" upside-down) this MUST match the value in CFE */
#define AVS_MPI_MAGIC 0x5377 /* use this version to know WE set the thresholds */

/*#include "bchp_avs_asb_registers.h"*/
#include "bchp_avs_hw_mntr.h"
#include "bchp_avs_pvt_mntr_config.h"
#include "bchp_avs_ro_registers_0.h"
#include "bchp_avs_ro_registers_1.h"
#include "bchp_avs_rosc_threshold_1.h"
#include "bchp_avs_rosc_threshold_2.h"

#if (BCHP_CHIP == 7358)
  #define MAX_REMOTE_OSCILLATORS 32 /* RDB shows 38 but really only 32 working */
#endif

/* Dynamically configure the number of central oscillators based on the shifts available */
#ifndef MAX_CENTRAL_OSCILLATORS
#if defined(BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_CEN_ROSC_1) /* defined if > 32 */
  #if defined(BCHP_AVS_HW_MNTR_ENABLE_DEFAULT_CEN_ROSC_1_reserved0_SHIFT) /* defined if > 32 and < 64 */
    #define MAX_CENTRAL_OSCILLATORS (32 + BCHP_AVS_HW_MNTR_ENABLE_DEFAULT_CEN_ROSC_1_reserved0_SHIFT)
  #else
    #define MAX_CENTRAL_OSCILLATORS 64 /* is exactly 64 */
  #endif
#elif defined(BCHP_AVS_HW_MNTR_ENABLE_DEFAULT_CEN_ROSC_0_reserved0_SHIFT) /* defined if < 32 */
  #define MAX_CENTRAL_OSCILLATORS BCHP_AVS_HW_MNTR_ENABLE_DEFAULT_CEN_ROSC_0_reserved0_SHIFT
#else
  #define MAX_CENTRAL_OSCILLATORS 32 /* is exactly 32 */
#endif
#endif

/* Dynamically configure the number of remote oscillators based on the shifts available */
#ifndef MAX_REMOTE_OSCILLATORS
#if defined(BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_RMT_ROSC_1) /* defined if > 32 */
  #if defined(BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_RMT_ROSC_1_reserved0_SHIFT) /* defined if > 32 and < 64 */
    #define MAX_REMOTE_OSCILLATORS (32 + BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_RMT_ROSC_1_reserved0_SHIFT)
  #else
    #define MAX_REMOTE_OSCILLATORS 64 /* is exactly 64 */
  #endif
#elif defined(BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_RMT_ROSC_0_reserved0_SHIFT) /* defined if < 32 */
  #define MAX_REMOTE_OSCILLATORS BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_RMT_ROSC_0_reserved0_SHIFT
#else
  #define MAX_REMOTE_OSCILLATORS 32 /* is exactly 32 */
#endif
#endif

/* Note: the above operations will not work if number of oscillators is greater than 64. Use same technique if that ever happens. */

/* We're not allowed to use any of the standard library macros or functions due to build conflicts */
#define AvsAbs(x) (((x)<0)?-(x):(x))

/* In order to get rid of the floating points, do the math and print the parts */
#define sign(f) ((f)<0)?'-':' ' /*space means positive*/
#define mantissa(f) (AvsAbs((int)(f)/1000))
#define fraction(f) (AvsAbs((int)((f) - (f)/1000*1000)))

BDBG_OBJECT_ID(bchp_avs_t);

/* This is the contect for this driver -- users use an opaque handle */
struct BCHP_P_AvsContext {
    BDBG_OBJECT(bchp_avs_t)    /* used to check if structure is valid */

    BCHP_Handle hChip;     /* the handle for the chip open */
    BREG_Handle hRegister; /* the register handle provided on open */

    int dac_step_size; /* this is the maximum amount (+ or -) we'll change the DAC on each pass */

    uint32_t last_dac;  /* keep track of the last value written to the DAC */
    uint32_t last_temp; /* last data read from temperature register */
    uint32_t last_voltage_1p1_0, last_voltage_1p1_1, last_voltage_0p99, last_voltage_2p75, last_voltage_3p63;

    uint32_t V_0p99, V_1p1_0, V_1p1_1, V_2p75, V_3p63; /* last values read from voltage registers */

    uint32_t original_dac; /* value of DAC set by CFE (or by our last run if run multiple times) */
    uint32_t saved_predicted; /* value of predicted voltage saved by CFE */

    uint64_t central_exclude_mask; /* running exclude lists for this part */
    uint64_t remote_exclude_mask;

    bool ff_part; /* flag indicating that part requires special processing */
    bool enabled;  /* flag to indicate that AVS-Enable process in CFE completed */
    bool tracking; /* flag to indicate AVS completed initialization and is now tracking */
    bool thresholdsSet; /* flag to indicate that the threshold values were set in CFE */

    bool initialized; /* flag telling me that this structure has been properly initialized */
    unsigned initialization_step; /* need to break down initialization into multiple steps */

    bool standby; /* set to true to pause the AVS processing (low-power mode) */
    AvsLockType_t lock_type; /* flag to indicate AVS lock is enabled and what type of lock to be processed */

    bool doOnce; /* if printing the "beginning" status (debug) */

    bool outOfBounds; /* temp -- so we only print this once */

    /* These are the registers that need to be saved/restored when entering/exiting low power mode */
    struct {
        uint32_t dac, min_dac, max_dac;
        uint32_t central_thresholds_min[MAX_CENTRAL_OSCILLATORS];
        uint32_t central_thresholds_max[MAX_CENTRAL_OSCILLATORS];
        uint32_t remote_hvt_min, remote_hvt_max;
        uint32_t remote_svt_min, remote_svt_max;

        uint32_t sw_controls;
        uint32_t default_cent0;
        uint32_t default_cent1;
        uint32_t measurement;

        bool valid;
    } saved_registers;
};

/* This defines which of the voltages we're using for measurements (and to return) */
#define last_voltage last_voltage_1p1_0

/* forward references: */
static void AvsInitialize(BCHP_P_AvsHandle handle);
static void AvsUpdate(BCHP_P_AvsHandle handle);
static void AvsSetOverTempThreshold(BREG_Handle handle);
static void AvsSaveClosedDAC(BCHP_P_AvsHandle handle);
static void AvsSaveRegisters(BCHP_P_AvsHandle handle);
static void AvsRestoreRegisters(BCHP_P_AvsHandle handle, bool restore);
/*static int AvsCheckSaveRegisters(BCHP_P_AvsHandle handle);*/

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

    /* This is the amount we'll change the value written to the DAC.
    ** The step is programmable and, if optimizations are enabled, changes once we converge.
    ** This ensures that we should converge or recover quickly (or more quickly) and then adjust more slowly after we converge.
    ** With optimization enabled, the dac_step_size will change when required.
    */
#define DEFAULT_DAC_STEP_SIZE 1
    handle->dac_step_size = DEFAULT_DAC_STEP_SIZE;

    /* We're not initialized until we complete the initialization process */
    handle->initialized = false;

    *phHandle = handle; /*success -- return the handle*/

    /* Always print something so we know that AVS is enabled in this build! */
    /* The problem with this is that even if its included it doesn't mean that nexus has it enabled -- needs to be part of Monitor function */
    /*BDBG_WRN(("AVS support included!"));*/

    /* Handle older parts or parts that have not updated to 2nd generation AVS hardware. */
    BDBG_MSG(("AVS_Open: Part uses generation %d of AVS", AVS_GENERATION));
    BDBG_MSG(("AVS_Open: Number of oscillators: central=%d, remote=%d", MAX_CENTRAL_OSCILLATORS, MAX_REMOTE_OSCILLATORS));

    AvsSetOverTempThreshold(handle->hRegister); /* make sure threshold is enabled */

    BDBG_LEAVE(BCHP_AvsOpen);
    return rc;
}

BERR_Code BCHP_P_AvsClose ( BCHP_P_AvsHandle hHandle )
{
    BDBG_ENTER(BCHP_AvsClose);
    BDBG_OBJECT_ASSERT(hHandle, bchp_avs_t);

    BDBG_MSG(("AVS Close called"));

    AvsSaveClosedDAC(hHandle);
#ifdef BCHP_PWR_RESOURCE_AVS
    BCHP_PWR_ReleaseResource(hHandle->hChip, BCHP_PWR_RESOURCE_AVS);
#endif
    BDBG_OBJECT_DESTROY(hHandle, bchp_avs_t);
    BKNI_Free(hHandle);

    BDBG_LEAVE(BCHP_AvsClose);
    return BERR_SUCCESS;
}

/* This gets called once a second to monitor the voltage and temperatures */
BERR_Code BCHP_P_AvsMonitorPvt ( BCHP_P_AvsHandle hHandle )
{
    BDBG_ENTER(BCHP_Monitor_Pvt);
    BDBG_OBJECT_ASSERT(hHandle, bchp_avs_t);

    /*BDBG_MSG(("Entered AVS Processing routine (initialized=%s)", hHandle->initialized?"true":"false"));*/

    /* If we have been placed in stand-by mode, we don't touch any registers */
    /*if (hHandle->standby) return BERR_TRACE(BERR_UNKNOWN);*/
    if (hHandle->standby) return BERR_UNKNOWN;

    if (!hHandle->initialized)
    {
        /* Prime everything until initialization completes */
        AvsInitialize(hHandle);
    }
    else
    {
        /* Update on every other call */
        AvsUpdate(hHandle);
    }

    BDBG_LEAVE(BCHP_Monitor_Pvt);
    return BERR_SUCCESS;
}

BERR_Code BCHP_P_AvsGetData (
    BCHP_P_AvsHandle hHandle, /* [in] handle supplied from open */
    BCHP_AvsData *pData )     /* [out] location to put data */
{
    uint32_t voltage, temperature;

    BDBG_ASSERT(pData);

    BDBG_ENTER(BCHP_AvsGetData);

    voltage = BREG_Read32(hHandle->hRegister, BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_0_MNTR_STATUS);
    voltage = BCHP_GET_FIELD_DATA(voltage, AVS_RO_REGISTERS_0_PVT_1P10V_0_MNTR_STATUS, data);
    pData->voltage = (990 * voltage * 8) / (7*1024);

    temperature = BREG_Read32(hHandle->hRegister, BCHP_AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS);
    temperature = BCHP_GET_FIELD_DATA(temperature, AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS, data);
    pData->temperature = 418000 - (556 * temperature);

    /* If the DAC is non-zero then CFE enabled AVS */
    pData->enabled = hHandle->enabled;

    /* If we got past the initialization then we're tracking the AVS data */
    /* If lock is in place then we won't track until lock removed */
    pData->tracking = hHandle->tracking;

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

    BDBG_MSG(("%s AVS standby-mode (AVS %s)", activate?"Entering":"Exiting", hHandle->initialized?"active":"inactive"));

    /* If the AVS system was never initialized then nothing to do here */
    if (!hHandle->initialized) return BERR_SUCCESS;

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

    BDBG_LEAVE(BCHP_P_AvsLowPowerMode);
    return rc;
}

/*\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\*/

/* Background: There are two types of oscillators: central and remote.
** The central ones are a block of different kinds with different threshold values.  These are packed close together in a
** central area of the part.  The remote ones are a scattering of oscillators spread out on the part.  They are all the
** same kind and the number may differ from chip to chip.  The central oscillators (because they are of different types)
** will have a threshold setting for each one.  The remotes will all share the same set.
** The object is to set some threshold values that can be used to tell if the voltage is too high or low.  When the voltage
** is too low the oscillators will run slower and speed will drop below the lower threshold.  The voltage is raised to compensate.
** When the voltage is too high the oscillators will run faster and speed will rise above upper threshold.  The voltage is lowered.
** Note that monitoring temperature is not required because the effects of increased or decreased temperature will be reflected
** in the oscillator speeds.
*/

typedef enum { Remote, Central } oscillator_t;

/* Nice defines for debug prints */
#define TorF(x) (((x)==true)?"true":"false")
#define CentralOrRemote(x) (((x)==Remote)?"Remote":"Central")

/* These are a bit mask of the oscillators to exclude from the processing */
/* These were identified by the hardware team to be ineffectual representation of the speed of the processor */
/* These are fixed across all instances so they don't need to be a part of the allocated data */
static const uint64_t default_central_exclude_mask = ~((uint64_t)1<<44 | (uint64_t)1<<45); /* we are only using oscillators 44 & 45 */
static const uint64_t default_remote_exclude_mask  = 0; /* we're using all the remotes */

/*\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\*/

/* Read the status of the specified central oscillator */
uint32_t AvsReadCentralOscillator(BREG_Handle hRegister, unsigned oscillator)
{
    uint32_t cent;
    cent = BREG_Read32(hRegister, BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_0 + (oscillator * 4)) & BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_0_data_MASK;
    return cent;
}

/* Read the status of the specified remote oscillator */
uint32_t AvsReadRemoteOscillator(BREG_Handle hRegister, unsigned oscillator)
{
    uint32_t rmt;
    rmt = BREG_Read32(hRegister, BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_0 + (oscillator * 4)) & BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_0_data_MASK;
    return rmt;
}

unsigned AvsGetNumberCentrals(void) { return MAX_CENTRAL_OSCILLATORS; }
unsigned AvsGetNumberRemotes(void)  { return MAX_REMOTE_OSCILLATORS; }

/* We have some parts that did not implement some oscillators.
** These are identified by reading the status as zero.
** Check all the oscillators (not already excluded) and add the ones we find.
** This procedure could be done statically, but this does it dynamically.
*/
static void AvsUpdateExcludeLists(BCHP_P_AvsHandle handle)
{
    BREG_Handle hRegister = handle->hRegister;
    uint32_t current;
    unsigned i;

    /* Start with the defaults and update as required */
    handle->central_exclude_mask = default_central_exclude_mask;
    handle->remote_exclude_mask  = default_remote_exclude_mask;

    for (i=STARTING_OSCILLATOR; i<MAX_CENTRAL_OSCILLATORS; i++)
    {
        if (((uint64_t)1<<i) & handle->central_exclude_mask) continue; /* skip items we're already excluding */

        current = AvsReadCentralOscillator(hRegister, i);
        if (!current)
            handle->central_exclude_mask |= (uint64_t)1<<i; /* add this oscillator to the exclude list */
    }

    for (i=0; i<MAX_REMOTE_OSCILLATORS; i++)
    {
        if (((uint64_t)1<<i) & handle->remote_exclude_mask) continue; /* skip items we're already excluding */

        current = AvsReadRemoteOscillator(hRegister, i);
        if (!current)
            handle->remote_exclude_mask |= (uint64_t)1<<i; /* add this oscillator to the exclude list */
    }

    if (handle->central_exclude_mask != default_central_exclude_mask)
        BDBG_MSG(("Using corrected central exclude mask of: " BDBG_UINT64_FMT "", BDBG_UINT64_ARG(handle->central_exclude_mask)));
    if (handle->remote_exclude_mask != default_remote_exclude_mask)
        BDBG_MSG(("Using corrected remote exclude mask of: " BDBG_UINT64_FMT "", BDBG_UINT64_ARG(handle->remote_exclude_mask)));
}

/* This performs some pre-initialization cleanup from stuff passed from CFE */
static void AvsCleanup(BCHP_P_AvsHandle handle)
{
    BREG_Handle hRegister = handle->hRegister;

    /* A newer version of the CFE sets the MIN & MAX DAC values to prevent accidental DAC writes. */
    /* This is identified by a MAX value that is different from the default. */
    /* If the MAX is the default then this could be an old version of CFE. */
    if (BREG_Read32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_MAX_DAC_CODE) == 0x3FF)
       {
        /* There is a CFE version out that only stores the predicted voltage in the MIN_DAC_CODE register.
        ** Need to save this off BEFORE we step on it.
        */
        handle->saved_predicted = BREG_Read32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_MIN_DAC_CODE);

        /* A version of CFE saves values to the MIN_DAC_CODE register; don't leave these set to non-valid numbers */
        /* Note: you do NOT need to have the programming enable bit set in order to write these as it says in th4e RDB */
        BREG_Write32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_MIN_DAC_CODE, 0);
        BREG_Write32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_MAX_DAC_CODE, 0x3FF);

        /*
        ** WARNING: If you DON'T reset the registers above and attempt to either enable the PROGRAMMING_ENABLE
        ** or write a DAC value that is lower/higher than the min/max, then the part will reset.
        */
    }
}

#if AVS_GENERATION==1
  #define PVT_MON_CNTRL 0x001D0C03  /* see PVTMon manual for description of these bits */
#else
  #define PVT_MON_CNTRL 0x007D2683  /* definitions for this register changed in the second generation parts */
#endif

static void AvsInitializeRegisters(BCHP_P_AvsHandle handle)
{
    BREG_Handle hRegister = handle->hRegister;
    uint32_t low_32, high_16;
    uint64_t out;

    /* Enable the DAC for writing */
    BREG_Write32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL, PVT_MON_CNTRL);

    /* Set the interrupt flag when ANY (?) oscillator is BELOW the threshold value */
    BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_DIRECTION, 0);
    BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_DIRECTION, 0);

#if 0
    /* Tell the sequencer to skip items we're not using */
    out = handle->central_exclude_mask;
    low_32  = out & 0xFFFFFFFF;
    BREG_Write32(hRegister, BCHP_AVS_HW_MNTR_SEQUENCER_MASK_CEN_ROSC_0, low_32);
#if (MAX_CENTRAL_OSCILLATORS > 32)
    high_16 = (out >> 32);
    BREG_Write32(hRegister, BCHP_AVS_HW_MNTR_SEQUENCER_MASK_CEN_ROSC_1, high_16);
#endif
#else
    /* If we tell the sequencer to skip these items then we won't get accurate data when trying to set the
    ** thresholds based on the current values during the new setup process.  So its okay to tell the sequencer to
    ** use all the oscillators, but we'll tell him to not use them for the threshold processing below.
    */
    low_32 = high_16 = 0;
    BREG_Write32(hRegister, BCHP_AVS_HW_MNTR_SEQUENCER_MASK_CEN_ROSC_0, low_32);
    BREG_Write32(hRegister, BCHP_AVS_HW_MNTR_SEQUENCER_MASK_CEN_ROSC_1, high_16);
#endif

    /* And exclude the thresholds for items we're not using */
    out = ~handle->central_exclude_mask; /* we enable the items we're NOT masking */
    low_32  = out & 0xFFFFFFFF;
    BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_1_CEN_ROSC_THRESHOLD1_EN_0, low_32);
    BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_2_CEN_ROSC_THRESHOLD2_EN_0, low_32);
#if (MAX_CENTRAL_OSCILLATORS > 32)
    high_16 = (out >> 32);
    BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_1_CEN_ROSC_THRESHOLD1_EN_1, high_16);
    BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_2_CEN_ROSC_THRESHOLD2_EN_1, high_16);
#endif
/*BDBG_MSG(("Central: For mask of: %016llx; Using enable mask of: %016llx (high=%04x, low=%08x)", handle->central_exclude_mask, out, high_16, low_32));*/

    /* Tell the sequencer to skip items we're not using */
    out = handle->remote_exclude_mask;
    low_32  = out & 0xFFFFFFFF;
    BREG_Write32(hRegister, BCHP_AVS_HW_MNTR_SEQUENCER_MASK_RMT_ROSC_0, low_32);
#if (MAX_REMOTE_OSCILLATORS > 32)
    high_16 = (out >> 32);
    BREG_Write32(hRegister, BCHP_AVS_HW_MNTR_SEQUENCER_MASK_RMT_ROSC_1, high_16);
#endif

    /* And exclude the thresholds for items we're not using */
    out = ~handle->remote_exclude_mask; /* we enable the items we're NOT masking */
    low_32  = out & 0xFFFFFFFF;
    BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_1_RMT_ROSC_THRESHOLD1_EN_0, low_32);
    BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_2_RMT_ROSC_THRESHOLD2_EN_0, low_32);
#if (MAX_REMOTE_OSCILLATORS > 32)
    high_16 = (out >> 32);
    BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_1_RMT_ROSC_THRESHOLD1_EN_1, high_16);
    BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_2_RMT_ROSC_THRESHOLD2_EN_1, high_16);
#endif
/*BDBG_MSG(("Remote: For mask of: %016llx; Using enable mask of: %016llx (high=%04x, low=%08x)", handle->remote_exclude_mask, out, high_16, low_32));*/
}

/* We set the voltage by controlling the DAC.  A larger DAC value means a lower voltage and vice versa. */
void AvsSetDAC(BCHP_P_AvsHandle handle, uint32_t dac_code)
{
    BREG_Handle hRegister = handle->hRegister;

    if (!dac_code) return; /* NEVER write 0 to DAC */

    /* Found a part that was forcing us to try to change the voltage past a point where changing the DAC yielded no change in the voltage.
    ** That is, we lowered the DAC (which was supposed to raise the voltage) only the voltage didn't change.
    ** When this happened, the DAC continued to go lower on each pass until the value wrapped.
    ** Note that we don't need a cap for the max value as writing a large DAC will generate a low voltage and cause the board to reset.
    ** Since I don't know where that value is, there is no way to test for this. Using arbitrary value could limit the ability to set a specific low voltage.
    ** WARNING: don't print an error message or we'll continue to print on EVERY update (solved this by adding a flag to only print once).
    */
#define MINIMUM_DAC 10
    if (dac_code < MINIMUM_DAC) {
        if (!handle->outOfBounds) {
            BDBG_ERR(("DAC went lower than %d -- this should not happen!", MINIMUM_DAC));
            handle->outOfBounds = true;
        }
        return; /* don't let the DAC ever go lower than this */
    }
#define MAXIMUM_DAC 0x3FF /* this is the maximum value allowed by the register -- if this is too high then we're out of control */
    if (dac_code > MAXIMUM_DAC) {
        if (!handle->outOfBounds) {
            BDBG_ERR(("DAC went higher than %d -- this should not happen!", MAXIMUM_DAC));
            handle->outOfBounds = true;
        }
        return; /* don't let the DAC ever go above this */
    }
    handle->outOfBounds = false; /* it is back in bounds so print again next time it goes out */

    /* There is a shadow copy of this register that gets loaded on the enable.
    ** If we enable first we could be loading something unintended.
    */
    BREG_Write32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE, dac_code);
    BREG_Write32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE_PROGRAMMING_ENABLE, 1); /* enable writing */
    BKNI_Delay(10); /* delay just a bit before disable */
    BREG_Write32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE_PROGRAMMING_ENABLE, 0); /* disable writing */

    /* We're supposed to wait awhile before reading any of the values provided by the PVTMON after changing the DAC.
    ** But that's just not something we can can do here (sleep for 60 milliseconds that is).
    ** Instead, we just make sure that we change the DAC as the last thing before exiting the poll loop.
    ** The poll loop gets executed once a second so the values will have settled before we get called again.
    ** Don't do this -> BKNI_Sleep(60);
    */

    /*BDBG_MSG(("Set DAC value of %x", dac_code));*/
    handle->last_dac = dac_code; /* this is the last value we set the DAC to */
}

uint32_t AvsGetDAC(BCHP_P_AvsHandle handle)
{
    uint32_t dac_code;
    dac_code = BREG_Read32(handle->hRegister, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE);
    /*BDBG_MSG(("Got DAC value of %x", dac_code));*/
    return dac_code;
}

/* These are fixed definitions provided by the developers for converting oscillator frequencies */
#define DIVIDER_DEFINE 1000
#define COUNTER_MAX    0x7FFF
#define EDGE_COUNT     2

/*
** These functions are used when writing/reading the thresholds from supplied constants.
** It puts the data into a format used by the hardware or into a format that is easier to read as a frequency.
** The "toRegister" is needed when we used fixed constants for the thresholds (no longer used).
** The "fromRegister" is needed when dumping the data in debug mode.
*/

#if 0
/* We always convert the threshold values before writing the registers (turns KHz into correct format) */
static uint32_t ConvertThresholdValue_toRegister(oscillator_t oscillator, unsigned long value)
{
    if (oscillator == Central)
        return value * (COUNTER_MAX * EDGE_COUNT) / (2 * 108 * DIVIDER_DEFINE);
    else
        return value * (COUNTER_MAX * EDGE_COUNT) / (108 * DIVIDER_DEFINE);
}
/* We always convert the threshold values after reading the registers (turns KHz into correct format) */
static unsigned long ConvertThresholdValue_fromRegister(oscillator_t oscillator, uint32_t reg)
{
    if (oscillator == Central)
        return reg * (2 * 108 * DIVIDER_DEFINE) / (COUNTER_MAX * EDGE_COUNT);
    else
        return reg * (108 * DIVIDER_DEFINE) / (COUNTER_MAX * EDGE_COUNT);
}
#endif

/* Return the threshold values for the specified central oscillator */
void AvsReadCentralOscThresholds(BREG_Handle hRegister, unsigned oscillator, uint32_t *reg_min, uint32_t *reg_max)
{
    *reg_min = BREG_Read32(hRegister, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_0 + (oscillator*4));
    *reg_max = BREG_Read32(hRegister, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_0 + (oscillator*4));
}

/* Return the threshold values for the specified remote oscillator */
void AvsReadRemoteOscThresholds(BREG_Handle hRegister, unsigned oscillator, uint32_t *reg_min, uint32_t *reg_max)
{
    /* The even status values are for the GS thresholds and the odd are for the GH (not documented in the RDB!) */
    /* This might need adjusting by the TPYE of the oscillator... */
    if (oscillator & 1) {
        *reg_min = BREG_Read32(hRegister, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_G8H);
        *reg_max = BREG_Read32(hRegister, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_G8H);
    } else {
        *reg_min = BREG_Read32(hRegister, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_G8S);
        *reg_max = BREG_Read32(hRegister, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_G8S);
    }
}

/* This returns the current value from the register.  This needs converting before it reflects a temperature in centigrade units (see include file). */
static uint32_t AvsGetTemperature(BCHP_P_AvsHandle handle)
{
    handle->last_temp = BREG_Read32(handle->hRegister, BCHP_AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS);
    handle->last_temp &= BCHP_AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS_data_MASK;
    return handle->last_temp;
}

#if 0
static void printLastVoltages(BCHP_P_AvsHandle handle)
{
    BDBG_MSG(("voltage_1p1_0=%d, voltage_1p1_1=%d, voltage_0p99=%d, voltage_2p75=%d, voltage_3p63=%d (DAC=%d)",
        handle->last_voltage_1p1_0, handle->last_voltage_1p1_1, handle->last_voltage_0p99, handle->last_voltage_2p75, handle->last_voltage_3p63,
        handle->last_dac));
}

static uint32_t GetPVTmonitorData(BREG_Handle hRegister, unsigned control)
{
    uint32_t data;

    /* Select the data to read, wait for it to settle, and Read the result */
    BREG_Write32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL, control);
    data = BREG_Read32(hRegister, BCHP_AVS_RO_REGISTERS_0_PVT_PROCESS_MNTR_STATUS);
    return (data & BCHP_AVS_RO_REGISTERS_0_PVT_PROCESS_MNTR_STATUS_data_MASK);
}
#endif

/* This reads all of the voltage data provided by the PVT Monitor */
static uint32_t AvsReadPvt(BCHP_P_AvsHandle handle)
{
    BREG_Handle hRegister = handle->hRegister;
#ifdef USE_AVERAGE_VOLTAGE
    unsigned i;
    unsigned long temp;

    /* There is a question as to whether we're getting the correct voltage value on a single read.
    ** So instead of relying on a single read, read it a bunch of times and use an average.
    */
#define READ_VOLTAGE_THIS_MANY_TIMES 5
    for (i=0; i<READ_VOLTAGE_THIS_MANY_TIMES; i++)
        temp += BREG_Read32(hRegister, BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_0_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_0_MNTR_STATUS_data_MASK;
    temp /= READ_VOLTAGE_THIS_MANY_TIMES;
#endif

    /*Read voltages*/
    handle->V_0p99  = BREG_Read32(hRegister, BCHP_AVS_RO_REGISTERS_0_PVT_0P99V_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_PVT_0P99V_MNTR_STATUS_data_MASK;
#ifdef USE_AVERAGE_VOLTAGE
    handle->V_1p1_0 = temp;
#else
    handle->V_1p1_0 = BREG_Read32(hRegister, BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_0_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_0_MNTR_STATUS_data_MASK;
#endif
    handle->V_1p1_1 = BREG_Read32(hRegister, BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_1_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_1_MNTR_STATUS_data_MASK;
    handle->V_2p75  = BREG_Read32(hRegister, BCHP_AVS_RO_REGISTERS_0_PVT_2p75V_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_PVT_2p75V_MNTR_STATUS_data_MASK;
    handle->V_3p63  = BREG_Read32(hRegister, BCHP_AVS_RO_REGISTERS_0_PVT_3p63V_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_PVT_3p63V_MNTR_STATUS_data_MASK;

#define bg_ref 990  /* make it 1000 bigger */

    handle->last_voltage_0p99  = (bg_ref * handle->V_0p99 ) / 1024;
    handle->last_voltage_1p1_0 = (bg_ref * handle->V_1p1_0 * 8) / (7*1024);
    handle->last_voltage_1p1_1 = (bg_ref * handle->V_1p1_1 * 8) / (7*1024);
    handle->last_voltage_2p75  = (bg_ref * handle->V_2p75 * 3) / 1024;
    handle->last_voltage_3p63  = (bg_ref * handle->V_3p63 * 4) / 1024;

/*printLastVoltages();*/

    return handle->last_voltage;
}

#if 0
/* Reset the PVT Monitor Sequencer.
** Note: there needs to be a delay before reading all the values for them to become valid.
** We do that by performing the reset after completing the convergence process, before we get called back in for a second pass.
*/
static void AvsResetSequencers(BREG_Handle hRegister)
{
    BREG_Write32(hRegister, BCHP_AVS_HW_MNTR_SEQUENCER_INIT, 1);
    BREG_Write32(hRegister, BCHP_AVS_HW_MNTR_SEQUENCER_INIT, 0);
}
#endif

#if 0
static void AvsClearInterruptFlags(BREG_Handle hRegister)
{
    BREG_Write32(hRegister, BCHP_AVS_HW_MNTR_AVS_INTERRUPT_FLAGS_CLEAR, 0xF);
    BREG_Write32(hRegister, BCHP_AVS_HW_MNTR_AVS_INTERRUPT_FLAGS_CLEAR, 0x0);
}

static unsigned AvsReadInterruptFlags(BREG_Handle hRegister)
{
    /*AvsClearInterruptFlags(hRegister);*/
    /* Do I need a delay here? */
    return (3 & BREG_Read32(hRegister, BCHP_AVS_HW_MNTR_AVS_INTERRUPT_FLAGS));
}
#endif

/* Adjust the DAC (and therefore the voltage) by the specified step value */
static bool AvsAdjustDacCode(BCHP_P_AvsHandle handle, int adjustment_step)
{
    uint32_t cur_val, new_val;
    uint32_t voltage_1p1_0;
    bool result = false;

    new_val = cur_val = AvsGetDAC(handle);

    /*
    ** We created a locking system to allow others to muck with registers without telling us.
    ** Before they do anything they are supposed to create the lock.
    ** When the lock is enabled we won't do anything to prevent them from doing whatever they want.
    */
    if (handle->lock_type != eAvsLockRegisters)
    {
        /* HACK: Seeing this value come back incorrect sometimes (???) */
        /* If we see this then something is wrong... (and we'll use the last [saved] value instead of the one we just read) */
        if (handle->last_dac && handle->last_dac != cur_val) {
            BDBG_ERR(("DAC current value (%d) not same as last (%d)", cur_val, handle->last_dac));
            cur_val = handle->last_dac;
        }
    }

/* These define the MIN and MAX values we'll let the voltage go to */
#define VMIN_ABS 860   /*0.86V*/
#define VMAX_ABS 1020  /*1.02V*/

    voltage_1p1_0 = AvsReadPvt(handle);
    /*BDBG_MSG(("Voltage = %d (min=%d, max=%d)", voltage_1p1_0, VMIN_ABS, VMAX_ABS));*/
    /*BDBG_MSG(("DAC: current value (%d) last value (%d)", cur_val, handle->last_dac));*/

    /* Make sure we never violate the voltage max and min values */
    /* Note that the adjustment step can be negative to adjust the other way */
    if ((VMIN_ABS <= voltage_1p1_0) && (voltage_1p1_0 <= VMAX_ABS))
    {
        new_val = cur_val + adjustment_step;
        /*BDBG_MSG(("Setting new DAC value = %d (was %d, step=%d)", new_val, cur_val, adjustment_step));*/
        result = true;
    }
    else
    {
        /* new algorithm says we shouldn't ALLOW the voltage to exceed its limits */
        /* So we know we're outside the safe voltage range -- bring it back */
        if (voltage_1p1_0 > VMAX_ABS)
            new_val = cur_val + AvsAbs(adjustment_step); /* increase DAC to lower voltage */
        else
            new_val = cur_val - AvsAbs(adjustment_step); /* decrease DAC to increase voltage */

        BDBG_MSG(("Voltage exceeding limit -- correcting (voltage=%d)", voltage_1p1_0));
        /*BDBG_MSG(("Setting new DAC value = %d (was %d, step=%d)", new_val, cur_val, adjustment_step));*/
        result = true;
    }

    if (new_val == 512) new_val += 1; /* apparently using 512 is bad */
    if (new_val == cur_val) return result; /* optimization -- don't write register with same value */

    AvsSetDAC(handle, new_val);
    return result;
}

#if 0
static void get_oscillator_values(BREG_Handle hRegister, oscillator_t oscillator, unsigned which_one, uint32_t *current, uint32_t *lower, uint32_t *upper)
{
    if (oscillator == Remote)
    {
        *current = AvsReadRemoteOscillator(hRegister, which_one);
        AvsReadRemoteOscThresholds(hRegister, which_one, lower, upper);
    }
    else
    {
        *current = AvsReadCentralOscillator(hRegister, which_one);
        AvsReadCentralOscThresholds(hRegister, which_one, lower, upper);
    }
}

static void print_oscillator(char *what_kind, oscillator_t oscillator, unsigned which_one, uint32_t current, uint32_t lower, uint32_t upper)
{
#if 1
    signed f_current = ConvertThresholdValue_fromRegister(oscillator, current);
    signed f_lower   = ConvertThresholdValue_fromRegister(oscillator, lower);
    signed f_upper   = ConvertThresholdValue_fromRegister(oscillator, upper);
    BDBG_MSG(("%s %2d %08x %08x %08x (%c %c) c=%c%2d.%03d l=%c%2d.%03d u=%c%2d.%03d (ld=%c%2d.%03d, ud=%c%2d.%03d)",
        what_kind, which_one, current, lower, upper, (current<=lower)?'L':'H', (current<=upper)?'L':'H',
        sign(f_current), mantissa(f_current), fraction(f_current),
        sign(f_lower), mantissa(f_lower), fraction(f_lower),
        sign(f_upper), mantissa(f_upper), fraction(f_upper),
        sign(f_current-f_lower), mantissa(f_current-f_lower), fraction(f_current-f_lower),
        sign(f_current-f_upper), mantissa(f_current - f_upper), fraction(f_current - f_upper)));
#else
    BDBG_MSG(("%s %2d %08x %08x %08x (%c %c)", what_kind, which_one, current, lower, upper, (current<=lower)?'L':'H', (current<=upper)?'L':'H'));
#endif
}

static void get_and_print_oscillator(BCHP_P_AvsHandle handle, oscillator_t oscillator, unsigned which_one)
{
    uint32_t current;
    uint32_t lower, upper;

    get_oscillator_values(handle->hRegister, oscillator, which_one, &current, &lower, &upper);
    print_oscillator(oscillator==Remote?"R:":"C:", oscillator, which_one, current, lower, upper);
}

static void printOscillators(BCHP_P_AvsHandle handle, char *text, bool include_excludes)
{
    unsigned i;

    if (text) BDBG_MSG(("%s", text));

    /* The number of central oscillators is set by the defaults or configuration file */
    for (i=STARTING_OSCILLATOR; i<MAX_CENTRAL_OSCILLATORS; i++)
    {
        if (!include_excludes && (((uint64_t)1<<i) & handle->central_exclude_mask)) continue; /* skip items we're excluding */
        get_and_print_oscillator(handle, Central, i);
    }

    /* The number of remote oscillators is fixed based on the part type (we dynamically set based on part) */
    for (i=0; i<MAX_REMOTE_OSCILLATORS; i++)
    {
        if (!include_excludes && (((uint64_t)1<<i) & handle->remote_exclude_mask)) continue; /* skip items we're excluding */
        get_and_print_oscillator(handle, Remote, i);
    }
}
#endif

static void AvsSetNewThresholds_0(BCHP_P_AvsHandle handle)
{
    BREG_Handle hRegister = handle->hRegister;
    unsigned current_dac, closed_dac=0, saved_dac=0, thresholds=0, predicted;

    current_dac = AvsGetDAC(handle);

#if 1
    /* We reserve a threshold value for saving the DAC value set by us when we closed */
    closed_dac = BREG_Read32(hRegister, CLOSED_DAC);
    if (closed_dac)
    {
        /* use the original DAC value that CFE chose that we saved on the first run */
        AvsSetDAC(handle, closed_dac);
        BDBG_MSG(("Using saved DAC value of %d (0x%x) from close for starting DAC value", closed_dac, closed_dac));
        /* Don't use it again unless we close properly again */
        BREG_Write32(hRegister, CLOSED_DAC, 0);
    }
#endif

#if 1
    /* We reserve a threshold value for saving the original DAC value set by CFE */
    if (!closed_dac)
    {
        saved_dac = BREG_Read32(hRegister, SAVED_DAC);
        if (saved_dac)
        {
            /* use the original DAC value that CFE chose that we saved on the first run */
            AvsSetDAC(handle, saved_dac);
            BDBG_MSG(("Using saved DAC value of %d (0x%x) for starting DAC value", saved_dac, saved_dac));
        }
        else
        {
            /* else this is our first run.  Save the CFE DAC value for future runs. */
            BREG_Write32(hRegister, SAVED_DAC, current_dac);
            BDBG_MSG(("Using CFE DAC value of %d (0x%x) for starting DAC value (saving)", current_dac, current_dac));
        }
    }
#endif

    /* CFE stored the predicted voltage value (before applying margins, etc.) */
    /* If this one is set, ignore the one in the DAC_MIN register */
    predicted = BREG_Read32(hRegister, PREDICTED_VOLTAGE);

    /* If it isn't in the one above then look in the secondary location  */
    if (!predicted) {
        predicted = handle->saved_predicted;
        BREG_Write32(hRegister, PREDICTED_VOLTAGE, predicted);
        BDBG_MSG(("Saving predicted voltage (%d) to save location", predicted));
    }

    /* Either way, predicted is now the right one.  Dump and save a copy! */
    BDBG_MSG(("Predicted voltage (from CFE) = %d (%x)", predicted, predicted));
    handle->saved_predicted = predicted;

/* A FF part is identified as having a predicted voltage lower the 0.730V */
#define FF_PART 730
    handle->ff_part = false;

#ifdef ALLOW_CONSTANT_VOLTAGE
    /* We never want the voltage to go above a specific value for FF parts.
    ** Since CFE always uses the minimum voltage for a FF part this means we never need to change it.
    ** We can just not do ANY AVS processing for these parts (or use alternate method).
    ** If predicted is zero then this CFE never saved the predicted voltage value -- this is wrong CFE!
    */
    if (predicted && predicted < FF_PART)
    {
        BDBG_MSG(("FF part identified -- using alternate AVS processing (predicted=%d)!", predicted));
        handle->ff_part = true;
    }
#endif

#if 1
    /* Code to set threshold registers was copied to CFE in order to set the thresholds based on the status with
    ** the voltage set by CFE.  There is older code that doesn't do this so we have to "know" if CFE did this or not.
    ** CFE writes a magic flag in the following register if it did this operation.
    */
    handle->thresholdsSet = false;
    thresholds = BREG_Read32(hRegister, THRESHOLDS_SET);
    if (thresholds == AVS_CFE_MAGIC)
    {
        BDBG_MSG(("Theshold values were set by CFE and will not be re-set here!"));
        handle->thresholdsSet = true;
    }
    if (thresholds == AVS_MPI_MAGIC)
    {
        BDBG_MSG(("Theshold values were set by this PI and will not be re-set here!"));
        handle->thresholdsSet = true;
    }
#endif
}

/* Save the current DAC value on a clean shutdown to restore on restart */
static void AvsSaveClosedDAC(BCHP_P_AvsHandle handle)
{
    BREG_Handle hRegister = handle->hRegister;
    unsigned current_dac;

    current_dac = AvsGetDAC(handle);

    BREG_Write32(hRegister, CLOSED_DAC, current_dac);
    BDBG_MSG(("Using current DAC value of %d (0x%x) for next starting DAC value (saving)", current_dac, current_dac));
}

/*
** New Threshold Procedure (7/15/11):
** We're assuming that CFE has already started and has completed the open-loop processing resulting in a voltage value.
** This procedure uses that voltage value as a starting point for calculating new threshold values before beginning the convergence process.
**
** 1. For oscillators 44 and 45 of the central group (IROSCX2 oscillators) and the remote oscillators, find slowest oscillator for each vt
**    type and store its register value and its number.
** 2. Because the register values of remotes and central of the same IROSCX2 oscillator cell differ by a factor of 2 (extra divider on the
**    central oscillators), the min register value found in step 1 above is loaded into the corresponding threshold_1 registers depending
**    on whether the corresponding oscillator is in the central or the remote group.
** 3. Read the status registers of all the remaining central oscillators and store them as the lower threshold, on oscillator by oscillator basis.
** 4. Read the current dac code and decrement it by 16 codes.  This should result between 7 and 14mV, or an average of ~10mV.
**    The number 16 may still need to be refined based on actual test results.
** 5. Similar to step #1, for oscillators 44 and 45 and the remotes, find the slowest oscillator for each vt type and store its register value and its number.
** 6. Similar to step #2, load the register value in step 5 into the upper threshold (threshold_2), taking into account the factor of 2 mentioned above.
** 7. Read the status registers of all the remaining central oscillators and store them as the upper threshold, on oscillator by oscillator basis.
**
** Then, Run the standard convergence code that you should already have.
**
** The threshold procedure had to be broken into two steps because of the delay needed to get accurate readings for the voltages and oscillators.
** When we set a new voltage, by writing the DAC, there is a delay needed to allow the sequencer time to process all the new data (make another pass).
** This can be forced by resetting the sequencer (AvsResetSequencers) but this also requires a delay to allow the sequencer to process all the items.
** By splitting this process into two steps we'll process the first part in one call-back, and set the new voltage, and then process the rest in
** the second call-back (after a delay).
*/
static void AvsSetNewThresholds_1(BCHP_P_AvsHandle handle)
{
    BREG_Handle hRegister = handle->hRegister;
    unsigned i, current_dac, lowest_hvt, lowest_svt, temp;
    oscillator_t which_h_type, which_s_type; unsigned which_h, which_s;

    /* 0) Use the current DAC value to choose the lower threshold value */
    current_dac = AvsGetDAC(handle);

    /* 1) Find the lowest threshold for both the central and remote oscillators */
    lowest_hvt = AvsReadCentralOscillator(hRegister, 44);
    lowest_svt = AvsReadCentralOscillator(hRegister, 45);
    which_s_type = which_h_type = Central; which_h = 44; which_s = 45;

    lowest_hvt *= 2; /* normalizing the central to the remotes because central are divided by 2 relative to remotes */
    lowest_svt *= 2;

    for (i=0; i<MAX_REMOTE_OSCILLATORS; i++)
    {
        if (((uint64_t)1<<i) & handle->remote_exclude_mask) continue; /* skip items we're excluding */
        if (i&1) {
            if ((temp = AvsReadRemoteOscillator(hRegister, i)) < lowest_hvt) { lowest_hvt = temp; which_h_type = Remote; which_h = i; }
        } else {
            if ((temp = AvsReadRemoteOscillator(hRegister, i)) < lowest_svt) { lowest_svt = temp; which_s_type = Remote; which_s = i; }
        }
    }

    {
        temp = AvsReadPvt(handle);
        BDBG_MSG(("Lower: For DAC = %d (0x%x) and voltage = %d (0x%x)", current_dac, current_dac, temp, temp));
        BDBG_MSG(("Lowest HVT = %s:%d = %d (0x%x)", CentralOrRemote(which_h_type), which_h, lowest_hvt, lowest_hvt));
        BDBG_MSG(("Lowest SVT = %s:%d = %d (0x%x)", CentralOrRemote(which_s_type), which_s, lowest_svt, lowest_svt));
    }

    /* 2) Set the lower threshold with the smallest value found above. */
    BREG_Write32(hRegister, (BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_0 + (44*4)), lowest_hvt/2);
    BREG_Write32(hRegister, (BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_0 + (45*4)), lowest_svt/2);
    BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_G8H, lowest_hvt);
    BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_G8S, lowest_svt);

#if 0 /* if we're not using these then don't write anything there */
    /* 3) Write the current status values to the lower threshold register */
    for (i=STARTING_OSCILLATOR; i<MAX_CENTRAL_OSCILLATORS; i++)
    {
        if (i==44 || i==45) continue; /* already did these above */
        BREG_Write32(hRegister, (BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_0 + (i*4)), AvsReadCentralOscillator(hRegister, i));
    }
#endif

    /* 4) Now, lower the DAC (raise the voltage) to choose the upper threshold value */
#define DAC_OFFSET 16
    current_dac -= DAC_OFFSET;
    AvsSetDAC(handle, current_dac);
}

static void AvsSetNewThresholds_2(BCHP_P_AvsHandle handle)
{
    BREG_Handle hRegister = handle->hRegister;
    unsigned i, current_dac, lowest_hvt, lowest_svt, temp;
    oscillator_t which_h_type, which_s_type; unsigned which_h, which_s;

    current_dac = AvsGetDAC(handle);

    /* 5) Find the lowest threshold for both the central and remote oscillators at the new voltage */
    lowest_hvt = AvsReadCentralOscillator(hRegister, 44);
    lowest_svt = AvsReadCentralOscillator(hRegister, 45);
    which_s_type = which_h_type = Central; which_h = 44; which_s = 45;

    lowest_hvt *= 2; /* normalizing the central to the remotes because central are divided by 2 relative to remotes */
    lowest_svt *= 2;

    for (i=0; i<MAX_REMOTE_OSCILLATORS; i++)
    {
        if (((uint64_t)1<<i) & handle->remote_exclude_mask) continue; /* skip items we're excluding */
        if (i&1) {
            if ((temp = AvsReadRemoteOscillator(hRegister, i)) < lowest_hvt) { lowest_hvt = temp; which_h_type = Remote; which_h = i; }
        } else {
            if ((temp = AvsReadRemoteOscillator(hRegister, i)) < lowest_svt) { lowest_svt = temp; which_s_type = Remote; which_s = i; }
        }
    }

    {
        temp = AvsReadPvt(handle);
        BDBG_MSG(("Upper: For DAC = %d (0x%x) and voltage = %d (0x%x)", current_dac, current_dac, temp, temp));
        BDBG_MSG(("Lowest HVT = %s:%d = %d (0x%x)", CentralOrRemote(which_h_type), which_h, lowest_hvt, lowest_hvt));
        BDBG_MSG(("Lowest SVT = %s:%d = %d (0x%x)", CentralOrRemote(which_s_type), which_s, lowest_svt, lowest_svt));
    }

    /* 6) Set the upper threshold with the smallest value found above. */
    BREG_Write32(hRegister, (BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_0 + (44*4)), lowest_hvt/2);
    BREG_Write32(hRegister, (BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_0 + (45*4)), lowest_svt/2);
    BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_G8H, lowest_hvt);
    BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_G8S, lowest_svt);

#if 0 /* if we're not using these then don't write anything there */
    /* 7) Write the current status values to the upper threshold register */
    for (i=STARTING_OSCILLATOR; i<MAX_CENTRAL_OSCILLATORS; i++)
    {
        if (i==44 || i==45) continue; /* already did these above */
        BREG_Write32(hRegister, (BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_0 + (i*4)), AvsReadCentralOscillator(hRegister, i));
    }
#endif

#if 1
    /* The thresholds have now been set so we don't need to re-do this on restart */
    BREG_Write32(hRegister, THRESHOLDS_SET, AVS_MPI_MAGIC);
#endif
}

#ifdef ALLOW_CONSTANT_VOLTAGE
/* This is a special version for the FF parts.
** This is meant to maintain the voltage at specific value regardless of the temperature.
** For FF parts, this is run in place of the convergence algorithm.
*/
static void AvsConstantVoltageProcess(BCHP_P_AvsHandle handle)
{
    uint32_t dac, saved, voltage;

    voltage = AvsReadPvt(handle);
    saved = dac = AvsGetDAC(handle);

#define FF_Voltage_Norm 860 /* 0.860V */
#define ConstantMargin 4 /* millivolts */
#define DACchange 1 /* amount to change DAC on each pass */

    if (voltage < (FF_Voltage_Norm - ConstantMargin)) dac -= DACchange; /* raise the voltage by lowering the DAC */
    if (voltage > (FF_Voltage_Norm + ConstantMargin)) dac += DACchange; /* lower the voltage by raising the DAC */

    /* Only make changes when the value has changed */
    if (dac != saved) AvsSetDAC(handle, dac);
}
#endif /*ALLOW_CONSTANT_VOLTAGE*/

/* This is the convergence algorithm.
** It checks the value of each oscillator against the threshold values.
** It moves the voltage by a fixed amount (on each call) until at least one of the oscillartors is inside the threshold boundaries.
** We can be unconverged by being outside the threshold ranges on all oscillators or by one or more oscillators being below the lower threshold.
** If all oscillators are above the upper threshold we lower the voltage (by raising the DAC).
** If even one oscillator is below the lower threshold we raise the voltage (by lowering the DAC).
** Return false if we failed to change the DAC due to voltage threshold violations (min/max).
*/
static bool AvsConvergeProcess(BCHP_P_AvsHandle handle)
{
    BREG_Handle hRegister = handle->hRegister;
    uint32_t current, lower=0, upper=0;
    bool increase=false, found_one=false, failed=false;
    unsigned i;

    for (i=STARTING_OSCILLATOR; i<MAX_CENTRAL_OSCILLATORS; i++)
    {
        if (((uint64_t)1<<i) & handle->central_exclude_mask) continue; /* skip items we're excluding */

        current = AvsReadCentralOscillator(hRegister, i);
        if (!current) continue; /* this should have already been handled by exclude list, but ... */
        AvsReadCentralOscThresholds(hRegister, i, &lower, &upper);

        /*print_oscillator("CENTRAL:", Central, i, current, lower, upper);*/

            if (lower && current < lower) {
                increase = true;
            } else if (upper && current < upper) {
                found_one = true;
        }
    }

    for (i=0; i<MAX_REMOTE_OSCILLATORS; i++)
    {
        if (((uint64_t)1<<i) & handle->remote_exclude_mask) continue; /* skip items we're excluding */

        current = AvsReadRemoteOscillator(hRegister, i);
        if (!current) continue; /* this should have already been handled by exclude list, but ... */
        AvsReadRemoteOscThresholds(hRegister, i, &lower, &upper);

        /*print_oscillator("REMOTE: ", Remote, i, current, lower, upper);*/

            if (lower && current < lower) {
                increase = true;
            } else if (upper && current < upper) {
                found_one = true;
        }
    }

    /*
    ** Note: this algorithm ensures that if we're below the lower threshold that we increase the voltage.
    ** If we don't find any oscillator below the upper threshold then we decrease the voltage.
    ** There is a swing to the values read so that sometimes we'll read a value under the lower threshold.
    ** We'll compensate for this by raising the voltage another step.  The swing should never be less than
    ** the threshold range or we'll end up vasilating between the two thresholds.
    ** You might think that its a good idea to increase the voltage by a larger amount then when lowering.
    ** This will bring us above the lower threshold quicker, but might allow us to increase past the upper
    ** threshold causing a vasilating situation.  Don't do this!
    */

    if (increase)
    {
        /*BDBG_MSG(("** DAC High (voltage too low) **"));*/

        /* we raise the voltage by lowering the DAC */
        failed = AvsAdjustDacCode(handle, -(handle->dac_step_size*2));
    }
    else if (!found_one)
    {
        /*BDBG_MSG(("** DAC low (voltage too high) **"));*/

        /* we lower the voltage by raising the DAC */
        failed = AvsAdjustDacCode(handle, handle->dac_step_size);
    }

    /*if (!handle->doOnce) printOscillators(handle, "Converged:", true);*/
    handle->doOnce = true;
    return failed;
}

/*\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\*/

static void AvsInitComplete(BCHP_P_AvsHandle handle)
{
    BDBG_MSG(("AVS initialized and processing started (AVS enabled)!"));
    handle->initialized = true;
    handle->tracking = true;
}

static void AvsInitialize(BCHP_P_AvsHandle handle)
{
    uint32_t current_dac;

    /* This checks to make sure that CFE did what it was supposed to do (set the voltage as part of the open-loop process).
    ** If this wasn't done than we can't do anything more here -- trying to would be bad.
    ** The reason is that changing the voltage by too much here would change the rate at which we access memory.
    ** This has been tuned (shmoo) to provide the optimim access speed.  We could move outside the memory optimization window
    ** and cause memory access issues if we were to change the voltage too much.
    */
    current_dac = AvsGetDAC(handle);

    if (!current_dac) {
        if (!handle->doOnce) {
            BDBG_WRN(("Booted with CFE that doesn't include AVS support --- disabling AVS!"));
            handle->doOnce = true;
        }
        return;
    }

    /* Always print something so we know that AVS is enabled in this build! */
    /* This is here (instead of in the Open) so that we know that support is included in the application (and that its being called) */
    if (!handle->doOnce) {
        BDBG_WRN(("AVS support enabled!"));
        handle->doOnce = true;
    }

    handle->enabled = true; /* AVS was enabled in CFE */
    handle->original_dac = current_dac; /* this was the value when we first started (set from CFE?) */

    /* Process the initialization in multiple steps, where each step ends in setting a new voltage.  This way each subsequent step (after the first)
    ** has an automatic delay that occured before our next call-back allowing the sequencer to make an additional pass and get new data.
    ** The third case is only necessary in order to allow everything to settle before (debug) printing the final register values.
    */
    BDBG_MSG(("Starting initialization step %d:", handle->initialization_step));
    switch (handle->initialization_step++)
    {
        case 0:
            /* Make sure this part has all the oscillators its supposed to */
            AvsUpdateExcludeLists(handle);

            /* Perform some cleanup from CFE */
            AvsCleanup(handle);

            /* Make sure the registers are prepared */
            AvsInitializeRegisters(handle);

            /* Set the starting DAC from the saved value from first run (if any) */
            AvsSetNewThresholds_0(handle);

            /* We don't do any further initialization for FF parts */
            if (handle->ff_part) AvsInitComplete(handle);

            /* If the thresholds were already set in CFE then we don't need to set them here */
            if (handle->thresholdsSet) AvsInitComplete(handle);
            break;

        case 1:
            /* Set the new lower thresholds based on the current voltage */
            AvsSetNewThresholds_1(handle);
            break;

        case 2:
            /* Set the new upper thresholds based on the current voltage */
            AvsSetNewThresholds_2(handle);
            /*break; <-- don't need to break if NOT printing the new threshold values */

        case 3:
            /*printOscillators(handle, "New Thresholds Set:", true);*/
            AvsInitComplete(handle);
            break;
    }
}

static void AvsUpdate(BCHP_P_AvsHandle handle)
{
    /* The lock is supposed to allow someone to stop the AVS process for a period while they play with the registers.
    ** If they enabled it, then I don't do any processing.
    */
    if (handle->lock_type == eAvsPause) return;

    /* Note: we read and print the values before we make any changes as the change can cause mis-reads on voltage values */
    AvsReadPvt(handle);

    if (!handle->last_dac) handle->last_dac = AvsGetDAC(handle);
    handle->last_temp = AvsGetTemperature(handle);

#if BDBG_DEBUG_BUILD
    {
        /* The formula is: centigrade = 418 - (.5556 * temp_reg) [multiplied everything by 1000 in order to get fractional part] */
        signed temperature = 418*1000 - (556 * handle->last_temp);
        BDBG_MSG(("Voltage = %d.%03dV (0x%x), DAC = %d (0x%x), Temp = [%c%d.%03dC] (0x%x)",
            mantissa(handle->last_voltage), fraction(handle->last_voltage), handle->last_voltage,
            handle->last_dac, handle->last_dac,
            sign(temperature), mantissa(temperature), fraction(temperature), handle->last_temp));
    }
#endif

    /*
    ** Note: you'd think we could use the interrupt value to tell us whether all the oscillators are above the upper threshold
    ** or if one is below the lower, but in the current part the interrupt doesn't appear to work if using remote oscillators.
    ** So converge or check the voltage on every pass...
    */

#ifdef ALLOW_CONSTANT_VOLTAGE
    if (handle->ff_part)
        AvsConstantVoltageProcess(handle);
    else
#endif
        AvsConvergeProcess(handle);
}

/* Maximum supported temperature */
#define MAX_TEMPERATURE 130 /* note: 150C = register value of 0x1E2 which is the default for this register */

/* This enables a fail-safe temperature monitor.  If the temperature ever reaches this value the part will reset.
** There This is really just a last ditch effort to save the part from burning up.
** Note:
**   reg = BREG_Read32(BCHP_AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS);
**   reg &= BCHP_AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS_data_MASK;
**   temperature = 418*1000 - (556 * reg);
** Note that the part is guaranteed to run up to 125C so using 150C is just to catch thermal run-away temperature situations.
** The result of a reset due to over-temp can be read in "BCHP_AON_CTRL_RESET_HISTORY" with bit: AON_CTRL :: RESET_HISTORY :: overtemp_reset [13:13]
*/
static void AvsSetOverTempThreshold(BREG_Handle hRegister)
{
	uint32_t reg;
	reg = BREG_Read32(hRegister, BCHP_AVS_HW_MNTR_TEMPERATURE_RESET_ENABLE);
	if (!reg) {
            reg = (418 - MAX_TEMPERATURE) * 1000 / 556;
            BREG_Write32(hRegister, BCHP_AVS_HW_MNTR_TEMPERATURE_THRESHOLD, reg);
            BREG_Write32(hRegister, BCHP_AVS_HW_MNTR_TEMPERATURE_RESET_ENABLE, 1);
	}
}

/* TODO: update this to use 'rmt' common utils code to do the save and restore for me */

/* Save a copy of the registers we set.
** This can be used to ensure that none of our registers were stepped on by someother code.
** This can also be used to save a copy of the registers in memory prior to entering low-power mode.
*/
static void AvsSaveRegisters(BCHP_P_AvsHandle handle)
{
    BREG_Handle hRegister = handle->hRegister;
    unsigned int i;

    handle->saved_registers.min_dac = BREG_Read32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_MIN_DAC_CODE);
    handle->saved_registers.max_dac = BREG_Read32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_MAX_DAC_CODE);
    handle->saved_registers.dac = BREG_Read32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE);
    /* Note: we save them ALL (including the ones we reserved and the ones we're not using) */
    for (i=0; i<MAX_CENTRAL_OSCILLATORS; i++)
    {
        handle->saved_registers.central_thresholds_min[i] = BREG_Read32(hRegister, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_0 + (i*4));
        handle->saved_registers.central_thresholds_max[i] = BREG_Read32(hRegister, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_0 + (i*4));
    }
    handle->saved_registers.remote_hvt_min = BREG_Read32(hRegister, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_G8H);
    handle->saved_registers.remote_hvt_max = BREG_Read32(hRegister, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_G8H);
    handle->saved_registers.remote_svt_min = BREG_Read32(hRegister, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_G8S);
    handle->saved_registers.remote_svt_max = BREG_Read32(hRegister, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_G8S);

    handle->saved_registers.sw_controls   = BREG_Read32(hRegister, BCHP_AVS_HW_MNTR_SW_CONTROLS);
    handle->saved_registers.default_cent0 = BREG_Read32(hRegister, BCHP_AVS_HW_MNTR_ENABLE_DEFAULT_CEN_ROSC_0);
    handle->saved_registers.default_cent1 = BREG_Read32(hRegister, BCHP_AVS_HW_MNTR_ENABLE_DEFAULT_CEN_ROSC_1);
    handle->saved_registers.measurement   = BREG_Read32(hRegister, BCHP_AVS_HW_MNTR_ROSC_MEASUREMENT_TIME_CONTROL);

    handle->saved_registers.valid = true; /* okay to use the saved set */

#if defined(BCHP_PWR_SUPPORT) && defined(SAVE_TO_AON)
    /* We need to save the current DAC and PCT_MON_CTRL values so CFE can restore them as part of a warm boot */
    {
        uint32_t pvt_ctrl = BREG_Read32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL);
        uint32_t dac_code = BREG_Read32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE);
        dac_code &= 0xffff;
        dac_code |= (AVS_CFE_MAGIC<<16); /* tell CFE that these values are valid */
#define AON_DAC_CODE_REGISTER 83 /* these were negotiated with Linux and Power Management guys */
        BREG_Write32(hRegister, BCHP_AON_CTRL_SYSTEM_DATA_RAMi_ARRAY_BASE + (AON_DAC_CODE_REGISTER*4), dac_code);
#define AON_PVT_CTRL_REGISTER 84
        BREG_Write32(hRegister, BCHP_AON_CTRL_SYSTEM_DATA_RAMi_ARRAY_BASE + (AON_PVT_CTRL_REGISTER*4), pvt_ctrl);
#define AON_PV_SAVE_REGISTER 85
        BREG_Write32(hRegister, BCHP_AON_CTRL_SYSTEM_DATA_RAMi_ARRAY_BASE + (AON_PV_SAVE_REGISTER*4), handle->saved_predicted);
    }
#endif
}

/* This is used to restore the saved set of registers.
** This is used to fix an issue where our registers were stepped on (restore=false).
** This is also used to restore the registers when exiting out of low-power mode (restore=true).
*/
static void AvsRestoreRegisters(BCHP_P_AvsHandle handle, bool restore)
{
    BREG_Handle hRegister = handle->hRegister;
    unsigned int i;

    if (!handle->saved_registers.valid) {
        BDBG_ERR(("Attempt to restore saved AVS registers from invalid save set!"));
        return;
    }

    if (restore) AvsInitializeRegisters(handle);

    /* This version sets the value (in shadow copies) which get loaded on the enable */
    BREG_Write32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_MIN_DAC_CODE, handle->saved_registers.min_dac);
    BREG_Write32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_MAX_DAC_CODE, handle->saved_registers.max_dac);
    BREG_Write32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE, handle->saved_registers.dac);

    BREG_Write32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE_PROGRAMMING_ENABLE, 1); /* enable writing */
    BKNI_Delay(10); /* delay just a bit before disable */
    BREG_Write32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE_PROGRAMMING_ENABLE, 0); /* disable writing */

    /* Note: we saved them ALL so restore them ALL */
    for (i=0; i<MAX_CENTRAL_OSCILLATORS; i++)
    {
        BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_0 + (i*4), handle->saved_registers.central_thresholds_min[i]);
        BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_0 + (i*4), handle->saved_registers.central_thresholds_max[i]);
    }
    BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_G8H, handle->saved_registers.remote_hvt_min);
    BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_G8H, handle->saved_registers.remote_hvt_max);
    BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_G8S, handle->saved_registers.remote_svt_min);
    BREG_Write32(hRegister, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_G8S, handle->saved_registers.remote_svt_max);

    BREG_Write32(hRegister, BCHP_AVS_HW_MNTR_SW_CONTROLS, handle->saved_registers.sw_controls);
    BREG_Write32(hRegister, BCHP_AVS_HW_MNTR_ENABLE_DEFAULT_CEN_ROSC_0, handle->saved_registers.default_cent0);
    BREG_Write32(hRegister, BCHP_AVS_HW_MNTR_ENABLE_DEFAULT_CEN_ROSC_1, handle->saved_registers.default_cent1);
    BREG_Write32(hRegister, BCHP_AVS_HW_MNTR_ROSC_MEASUREMENT_TIME_CONTROL, handle->saved_registers.measurement);

    BREG_Write32(hRegister, SAVED_DAC, handle->original_dac);
    BREG_Write32(hRegister, PREDICTED_VOLTAGE, handle->saved_predicted);
    BREG_Write32(hRegister, CLOSED_DAC, 0); /* make sure this is zero until we make it not zero */

    handle->saved_registers.valid = false; /* technically its still okay to use but this will find a restore without a save */

    AvsSetOverTempThreshold(handle->hRegister); /* make sure threshold is enabled */
}

#if 0
/* Test all of my "important" registers to see if they got stepped on.
** We can do a save before leaving on each pass of the monitoring function and check on entry.
** IF there is a verify error, the restore can put the registers back to what they should be.
*/
static int AvsCheckSaveRegisters(BCHP_P_AvsHandle handle)
{
    BREG_Handle hRegister = handle->hRegister;
    unsigned int i, result=0;

    if ((result |= handle->saved_registers.min_dac == BREG_Read32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_MIN_DAC_CODE)))
        BDBG_ERR(("Check: MIN_DAC stepped on!"));
    if ((result |= handle->saved_registers.max_dac == BREG_Read32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_MAX_DAC_CODE)))
        BDBG_ERR(("Check: MAX_DAC stepped on!"));
    if ((result |= handle->saved_registers.dac == BREG_Read32(hRegister, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE)))
        BDBG_ERR(("Check: DAC_CODE stepped on!"));
    for (i=STARTING_OSCILLATOR; i<MAX_CENTRAL_OSCILLATORS; i++)
    {
        if ((result |= handle->saved_registers.central_thresholds_min[i] == BREG_Read32(hRegister, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_0 + (i*4))))
            BDBG_ERR(("Check: Central threshold MIN %d stepped on!", i));
        if ((result |= handle->saved_registers.central_thresholds_max[i] == BREG_Read32(hRegister, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_0 + (i*4))))
            BDBG_ERR(("Check: Central threshold MAX %d stepped on!", i));
    }
    if ((result |= handle->saved_registers.remote_hvt_min == BREG_Read32(hRegister, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_G8H)))
        BDBG_ERR(("Check: Remote threshold HVT min stepped on!"));
    if ((result |= handle->saved_registers.remote_hvt_max == BREG_Read32(hRegister, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_G8H)))
        BDBG_ERR(("Check: Remote threshold HVT max stepped on!"));
    if ((result |= handle->saved_registers.remote_svt_min == BREG_Read32(hRegister, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_G8S)))
        BDBG_ERR(("Check: Remote threshold SVT min stepped on!"));
    if ((result |= handle->saved_registers.remote_svt_max == BREG_Read32(hRegister, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_G8S)))
        BDBG_ERR(("Check: Remote threshold SVT max stepped on!"));

    return result;
}
#endif

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
    handle->tracking = handle->initialized; /* if we were successfully initialized then we should be tracking */
    BDBG_LEAVE(BCHP_AvsUnlock);
    return BERR_SUCCESS;
}

void AvsGetTestData(BCHP_P_AvsHandle handle, AvsTestData *data)
{
    BDBG_ASSERT(handle);
    BDBG_ASSERT(data);

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
}
