/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description: Audio Decoder Interface
 *
 ***************************************************************************/

#include "bape.h"
#include "bape_priv.h"

#if defined BCHP_HIFIDAC_CTRL_1_REG_START
    #include "bchp_hifidac_ctrl_1.h"
#endif
#if defined BCHP_HIFIDAC_CTRL_0_REG_START
    #include "bchp_hifidac_ctrl_0.h"
#endif

#if defined BCHP_HIFIDAC_CTRL1_REG_START
    #include "bchp_hifidac_ctrl1.h"
#endif
#if defined BCHP_HIFIDAC_CTRL0_REG_START
    #include "bchp_hifidac_ctrl0.h"
#endif

BDBG_MODULE(bape_rfmod);

#if BAPE_CHIP_MAX_RFMODS > 0   /* If no RF modulator outputs, then skip all of this and just put in stub funcs at bottom of file. */

BDBG_OBJECT_ID(BAPE_RfMod);

typedef struct BAPE_RfMod
{
    BDBG_OBJECT(BAPE_RfMod)
    BAPE_Handle deviceHandle;
    BAPE_RfModSettings settings;
    unsigned index;
    char name[16];   /* RF Mod %d */
} BAPE_RfMod;


/* Static function prototypes */
static void BAPE_RfMod_P_SetDacMute(BAPE_RfModHandle handle, unsigned dacIndex, bool muteRfm);

/* Output port callbacks */
    /* None */


/***************************************************************************
        Public APIs: From bape_output.h
***************************************************************************/

void BAPE_RfMod_GetDefaultSettings(
    BAPE_RfModSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

/**************************************************************************/

BERR_Code BAPE_RfMod_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_RfModSettings *pSettings,
    BAPE_RfModHandle *pHandle             /* [out] */
    )
{
    BERR_Code errCode;
    BAPE_RfModHandle handle;
    BAPE_RfModSettings defaultSettings;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    BDBG_MSG(("%s: Opening RFMOD Output: %u", BSTD_FUNCTION, index));

    *pHandle = NULL;    /* Set up to return null handle in case of error. */

    if ( index >= BAPE_CHIP_MAX_RFMODS )
    {
        BDBG_ERR(("Request to open RFMOD %d but chip only has %u RFMODs", index, BAPE_CHIP_MAX_RFMODS));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( deviceHandle->rfmods[index] )
    {
        BDBG_ERR(("RFMOD %u already open", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Allocate the device structure, then fill in all the fields. */
    handle = BKNI_Malloc(sizeof(BAPE_RfMod));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_RfMod));
    BDBG_OBJECT_SET(handle, BAPE_RfMod);
    handle->deviceHandle = deviceHandle;
    handle->index = index;
    BKNI_Snprintf(handle->name, sizeof(handle->name), "RF Mod %u", index);

    /* Init to specified settings */
    if ( NULL == pSettings )
    {
        BAPE_RfMod_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    errCode = BAPE_RfMod_SetSettings(handle, pSettings);
    if ( errCode )
    {
        BAPE_RfMod_Close(handle);
        return BERR_TRACE(errCode);
    }

    deviceHandle->rfmods[index] = handle;
    *pHandle = handle;
    return BERR_SUCCESS;
}

/**************************************************************************/

void BAPE_RfMod_Close(
    BAPE_RfModHandle handle
    )
{
    BAPE_RfModSettings defaults;

    BDBG_OBJECT_ASSERT(handle, BAPE_RfMod);

    BAPE_RfMod_GetDefaultSettings(&defaults);
    BAPE_RfMod_SetSettings(handle, &defaults);

    handle->deviceHandle->rfmods[handle->index] = NULL;
    BDBG_OBJECT_DESTROY(handle, BAPE_RfMod);
    BKNI_Free(handle);
}

/**************************************************************************/

void BAPE_RfMod_GetSettings(
    BAPE_RfModHandle handle,
    BAPE_RfModSettings *pSettings     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_RfMod);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

/**************************************************************************/

BERR_Code BAPE_RfMod_SetSettings(
    BAPE_RfModHandle handle,
    const BAPE_RfModSettings *pSettings
    )
{
    BAPE_OutputPort     oldMaster = NULL;

    BDBG_OBJECT_ASSERT(handle, BAPE_RfMod);
    BDBG_ASSERT(NULL != pSettings);

    /* See if they want to change the  master. */
    if (pSettings->master != handle->settings.master)
    {
        oldMaster = handle->settings.master;    /* Save this so we can mute the old master later */

        /* If they've passed in a new master, make sure it's valid. */
        if (pSettings->master)
        {
            if ( pSettings->master->type != BAPE_OutputPortType_eDac ) {
                BDBG_ERR(("Master: %s is not a DAC, giving up.", pSettings->master->pName ));
                return (BERR_INVALID_PARAMETER);
            }
            BDBG_ASSERT(  pSettings->master->index < BAPE_CHIP_MAX_DACS );
        }

        /* Print the old master if there was one. */
        if (handle->settings.master)
        {
            BDBG_OBJECT_ASSERT(handle->settings.master, BAPE_OutputPort);
            BDBG_MSG(("Removing master: %s from %s", handle->settings.master->pName, handle->name ));
        }

        /* Print the new master if there is one. */
        if (pSettings->master)
        {
            BDBG_OBJECT_ASSERT(pSettings->master, BAPE_OutputPort);
            BDBG_MSG(("Assigning master: %s to %s", pSettings->master->pName, handle->name ));
        }

        /* Now update the master in the device handle. */
        handle->settings.master = pSettings->master;
    }

    /* See if they want to change the mute setting. */
    if (pSettings->muted != handle->settings.muted)
    {
        /* Now update the setting in the device handle. */
        handle->settings.muted = pSettings->muted;
    }

    BDBG_ASSERT(NULL==handle->settings.master || BAPE_CHIP_MAX_DACS > handle->settings.master->index);

    /* Now do the register updates. */ 
    /* If the master DAC is changing, and the previous one was non-NULL, then mute it. */
    if (oldMaster)
    {
        BAPE_RfMod_P_SetDacMute(handle, oldMaster->index, true);
    }

    /* If we have a master, then go ahead and set the muting and source selection as appropriate.
     * But if there's no master, then there's nothing to do... the source selection is irrelevant,
     * and the mute bit is on the master DAC (so there's nothing to mute). 
     */ 
    if (handle->settings.master)
    {
        bool muteRfm = true;       /* Assume that we'll be muting the RF Mod */

        /* Determine the mute state... Only unmute if the RFMod has a master (DAC) and has
         * been requested to unmute.  Otherwise, keep it muted.
         */ 
        if (!handle->settings.muted)
        {
            muteRfm = false;
        }

        /* If there's more than one DAC, then we'll have to tell the RfMod which DAC  to take its
         * input from.  So see if there are any RfMod source selection register fields defined.
         */
        #if defined (BCHP_AUD_FMM_MISC_HIFIOUT_SEL_RFMOD_SEL_MASK) || defined (BCHP_AUD_FMM_MISC_HIFIOUT_SEL_RFMOD0_SEL_MASK)
            {
                uint32_t srcSelRegAddr,  srcSelRegVal;

                /* If we are muting, do it now, before selecting the source. If unmuting, don't do it
                 * until after we do the source selection.
                 */
                if (muteRfm)
                {
                    BAPE_RfMod_P_SetDacMute(handle, handle->settings.master->index, muteRfm);
                }
     
                srcSelRegAddr = BCHP_AUD_FMM_MISC_HIFIOUT_SEL;
                srcSelRegVal = BREG_Read32(handle->deviceHandle->regHandle, srcSelRegAddr);
        
                #if BAPE_CHIP_MAX_DACS > 3
                    #error "Check this code to see if srcSelVal is being computed correctly"
                #endif
        
                #if defined  BCHP_AUD_FMM_MISC_HIFIOUT_SEL_RFMOD_SEL_MASK  /* one RFMod */
                BDBG_ASSERT(0 == handle->index);
    
                    srcSelRegVal &= ~ BCHP_MASK(AUD_FMM_MISC_HIFIOUT_SEL, RFMOD_SEL);
                    srcSelRegVal |=   BCHP_FIELD_DATA(AUD_FMM_MISC_HIFIOUT_SEL, RFMOD_SEL, handle->settings.master->index );
    
                #elif defined BCHP_AUD_FMM_MISC_HIFIOUT_SEL_RFMOD0_SEL_MASK  /* Multiple RFMods*/
    
                    if (0 == handle->index)
                    {
                        srcSelRegVal &= ~ BCHP_MASK(AUD_FMM_MISC_HIFIOUT_SEL, RFMOD0_SEL);
                        srcSelRegVal |=   BCHP_FIELD_DATA(AUD_FMM_MISC_HIFIOUT_SEL, RFMOD0_SEL, handle->settings.master->index );
                    }
                    #if BAPE_CHIP_MAX_RFMODS > 1
                        else if (1 == handle->index)
                        {
                            srcSelRegVal &= ~ BCHP_MASK(AUD_FMM_MISC_HIFIOUT_SEL, RFMOD1_SEL);
                            srcSelRegVal |=   BCHP_FIELD_DATA(AUD_FMM_MISC_HIFIOUT_SEL, RFMOD1_SEL, handle->settings.master->index );
                        }
                    #endif  /*  BAPE_CHIP_MAX_RFMODS > 1 */
                    #if BAPE_CHIP_MAX_RFMODS > 2
                        else if (2 == handle->index)
                        {
                            srcSelRegVal &= ~ BCHP_MASK(AUD_FMM_MISC_HIFIOUT_SEL, RFMOD2_SEL);
                            srcSelRegVal |=   BCHP_FIELD_DATA(AUD_FMM_MISC_HIFIOUT_SEL, RFMOD2_SEL, handle->settings.master->index );
                        }
                    #endif  /*  BAPE_CHIP_MAX_RFMODS > 2 */
                    #if BAPE_CHIP_MAX_RFMODS > 3
                        #error "Need to add support for more than 3 RF modulators"
                    #endif /* BAPE_CHIP_MAX_RFMODS > 3 */
                    else
                    {
                        BDBG_ERR(("RF Mod index:%u is invalid"));
                        BDBG_ASSERT(false);
                    }
                #else
                    #error "I don't know how to fill in the RFMOD_SEL field!"
                #endif
        
                BREG_Write32(handle->deviceHandle->regHandle, srcSelRegAddr, srcSelRegVal);
    
                /* If we're unmuting, then do it now... after the source selection. */
                if (! muteRfm)
                {
                    BAPE_RfMod_P_SetDacMute(handle, handle->settings.master->index, muteRfm);
                }
            }

        #else /* There aren't any RF Mod source selection bits, just write the mute/unmute register */
            BAPE_RfMod_P_SetDacMute(handle, handle->settings.master->index, muteRfm);
        #endif /* defined (BCHP_AUD_FMM_MISC_HIFIOUT_SEL_RFMOD_SEL_MASK) || defined (BCHP_AUD_FMM_MISC_HIFIOUT_SEL_RFMOD0_SEL_MASK) */
    
    }

    handle->settings = *pSettings;
    return BERR_SUCCESS;
}

/***************************************************************************
        BAPE Internal APIs: From bape_fmm_priv.h
***************************************************************************/

BERR_Code BAPE_RfMod_P_ResumeFromStandby(BAPE_Handle bapeHandle)
{
    BERR_Code   errCode = BERR_SUCCESS;
    unsigned    rfModIndex;

    BDBG_OBJECT_ASSERT(bapeHandle, BAPE_Device);

    /* For each opened RfMod, call the functions necessary to restore the hardware to it's appropriate state. */
    for ( rfModIndex=0 ; rfModIndex<BAPE_CHIP_MAX_RFMODS ; rfModIndex++ )
    {
        if ( bapeHandle->rfmods[rfModIndex] )       /* If this RfMod is open... */
        {
            BAPE_RfModHandle hRfMod = bapeHandle->rfmods[rfModIndex];

            /* Put the HW into the generic open state. */
                /* Nothing to do here for RfMods. */
            
            /* Now apply changes for the settings struct. */
            errCode = BAPE_RfMod_SetSettings(hRfMod, &hRfMod->settings);
            if ( errCode ) return BERR_TRACE(errCode);

            /* Now restore the dynamic stuff from the values saved in the device struct. */
                /* Nothing to do here for RfMods. */
        }
    }
    return errCode;
}

static void BAPE_RfMod_P_SetDacMute(BAPE_RfModHandle handle, unsigned dacIndex, bool muteRfm)
{
    uint32_t regAddr;

#if defined BCHP_HIFIDAC_CTRL0_CONFIG
    regAddr = BCHP_HIFIDAC_CTRL0_CONFIG;
    #ifdef BCHP_HIFIDAC_CTRL1_CONFIG
    regAddr += dacIndex * (BCHP_HIFIDAC_CTRL1_CONFIG - BCHP_HIFIDAC_CTRL0_CONFIG);
    #else
    BDBG_ASSERT(dacIndex == 0);
    #endif

    BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, HIFIDAC_CTRL0_CONFIG, RFMOD_MUTE, muteRfm?1:0);
#elif defined BCHP_HIFIDAC_CTRL_0_CONFIG
    regAddr = BCHP_HIFIDAC_CTRL_0_CONFIG;
    #ifdef BCHP_HIFIDAC_CTRL_1_CONFIG
    regAddr += dacIndex * (BCHP_HIFIDAC_CTRL_1_CONFIG - BCHP_HIFIDAC_CTRL_0_CONFIG);
    #else
    BDBG_ASSERT(dacIndex == 0);
    #endif

    BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, HIFIDAC_CTRL_0_CONFIG, RFMOD_MUTE, muteRfm?1:0);
#else
    #warning Unknown DAC register layout 
#endif
}

/***************************************************************************
    Define stub functions for when there are no RF modulator audio outputs. 
***************************************************************************/
#else /* BAPE_CHIP_MAX_RFMODS <= 0 */
    /* No RF modulator audio outputs, just use stubbed out functions. */

/**************************************************************************/
void BAPE_RfMod_GetDefaultSettings(
    BAPE_RfModSettings *pSettings
    )
{
    BSTD_UNUSED(pSettings);
}

/**************************************************************************/

BERR_Code BAPE_RfMod_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_RfModSettings *pSettings,
    BAPE_RfModHandle *pHandle             /* [out] */
    )
{
    BSTD_UNUSED(deviceHandle);
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);

    *pHandle = NULL;

    return BERR_NOT_SUPPORTED;
}

/**************************************************************************/

void BAPE_RfMod_Close(
    BAPE_RfModHandle handle
    )
{
    BSTD_UNUSED(handle);
}

/**************************************************************************/

void BAPE_RfMod_GetSettings(
    BAPE_RfModHandle handle,
    BAPE_RfModSettings *pSettings     /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

/**************************************************************************/

BERR_Code BAPE_RfMod_SetSettings(
    BAPE_RfModHandle handle,
    const BAPE_RfModSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);

    return BERR_NOT_SUPPORTED;
}

/**************************************************************************/

BERR_Code BAPE_RfMod_P_ResumeFromStandby(BAPE_Handle bapeHandle)
{
    BSTD_UNUSED(bapeHandle);
    return BERR_SUCCESS;
}


#endif /* BAPE_CHIP_MAX_RFMODS */
