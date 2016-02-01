/***************************************************************************
 *     (c)2004-2010 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
 *   
 *  Except as expressly set forth in the Authorized License,
 *   
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *   
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *  
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
 *  ANY LIMITED REMEDY.
 * 
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:  SCD API for controlling BRCM demodulator chips/cores
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ************************************************************/

/****************************************************************************/
/*
 *  SCD API
 */
/****************************************************************************/

#include "btfe_scd_priv.h"
#include "btfe_scd_hal_priv.h"
#include "btfe_scd_int_priv.h"
#include "btfe_scd_version_priv.h"
#include "bstd.h"
#include "bkni.h"
#include "btfe.h"

BDBG_MODULE(btfe_scd);


#define API_MAJOR  1
#define API_MINOR  28
#define API_BUILD  42

/****************************************************************************/
/* local types */
/****************************************************************************/

/****************************************************************************/
/* global variables */
/****************************************************************************/

/****************************************************************************/
/* local variables */
/****************************************************************************/
#define DEFAULT_IF_FREQUENCY        (44000000)

static SCD_CHIP scd_chip[SCD_MAX_CHIP];
static SCD_DEMOD scd_fat[SCD_MAX_FAT];
static SCD_DEMOD scd_fdc[SCD_MAX_FDC];

static uint32_t fat_count;
static uint32_t fdc_count;

static uint32_t scd_initialized;

/****************************************************************************/
/* local functions */
/****************************************************************************/

static int BTFE_P_ScdIsHandleClosed(SCD_HANDLE handle)
{
    SCD_OBJECT object;

    object.handle = handle;

    switch(GET_HANDLE_TYPE(handle))
    {
        case SCD_HANDLE_FAT:
        case SCD_HANDLE_FDC:
            if(object.demod->open_count != 0) return 0;
                break;

        case SCD_HANDLE_CHIP:
            if(object.chip->open_count != 0) return 0;
                break; 
    }

    return 1;
}

/****************************************************************************/
/* API functions */
/****************************************************************************/

SCD_RESULT BTFE_P_ScdInitialize(uint32_t flags, void *reg_handle)
{
    int i;

    API_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdInitialize(flags=%08X)", flags)));

#ifdef BBS_SCD_PERL
	if(!scd_initialized)
	{
#endif
		/* flags not currently implemented */
		if(flags)
		{
			return SCD_RESULT__ARG_OUT_OF_RANGE;
		}

		for(i=0; i<SCD_MAX_CHIP; i++)
		{
			BKNI_Memset(&scd_chip[i], 0, sizeof(SCD_CHIP));

			scd_chip[i].state = SCD_STATE_UNINITIALIZED;
		}

		for(i=0; i<SCD_MAX_FAT; i++)
		{
			BKNI_Memset(&scd_fat[i], 0, sizeof(SCD_DEMOD));

			scd_fat[i].state = SCD_STATE_CLOSED;
		}

		for(i=0; i<SCD_MAX_FDC; i++)
		{
			BKNI_Memset(&scd_fdc[i], 0, sizeof(SCD_DEMOD));

			scd_fdc[i].state = SCD_STATE_CLOSED;
		}
#ifdef BBS_SCD_PERL
	}
#endif
    /* initialize HAL */
    fat_count = 0;
    fdc_count = 0;

    scd_initialized = 1;

    return BTFE_P_HalInitialize(0, reg_handle);
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdCleanup(void)
{
    int i;

    API_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdCleanup()")));

    if(!scd_initialized)
    {
        BDBG_ERR(("BTFE_P_ScdCleanup: SCD not initialized"));
        return SCD_RESULT__SCD_NOT_INITIALIZED;
    }

    /* close any open FATs */
    for(i=0; i<SCD_MAX_FAT; i++)
    {
        if(scd_fat[i].handle_id == SCD_HANDLE_FAT)
        {
            if(scd_fat[i].open_count)
            {
                BDBG_MSG(("BTFE_P_ScdCleanup: closing FAT %u", i));
                scd_fat[i].open_count = 1;
                BTFE_P_ScdCloseFat(&scd_fat[i]);
            }

            scd_fat[i].handle_id = 0;
        }
    }

    /* close any open chips and remove */
    for(i=0; i<SCD_MAX_CHIP; i++)
    {
        if(scd_chip[i].handle_id == SCD_HANDLE_CHIP)
        {
            if(scd_chip[i].open_count)
            {
                BDBG_MSG(("BTFE_P_ScdCleanup: closing chip %u", i));
                scd_chip[i].open_count = 1;
                BTFE_P_ScdCloseChip(&scd_chip[i]);
            }

            scd_chip[i].handle_id = 0;
        }
    }

    /* do HAL cleanup */
    BTFE_P_HalCleanup();

    fat_count = 0;
    fdc_count = 0;

    scd_initialized = 0;

    return SCD_RESULT__OK;
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdGetVersion(SCD_ITEM item, uint32_t instance, SCD_VERSION *version)
{
    if(!scd_initialized)
    {
        BDBG_ERR(("BTFE_P_ScdGetVersion: SCD not initialized"));
        return SCD_RESULT__SCD_NOT_INITIALIZED;
    }

    if(version)
    {
        switch(item)
        {
        case SCD_ITEM__API:
            version->name = "SCD";
            version->major = API_MAJOR;
            version->customer = CUSTOMER;
            version->minor = API_MINOR;
            version->device_id = API_BUILD;
            return SCD_RESULT__OK;

        case SCD_ITEM__FAT:
            if((instance < SCD_MAX_FAT) && (scd_fat[instance].handle_id))
            {
                uint32_t chip_instance = scd_fat[instance].chip_instance;

                version->major = 0;
                version->customer = 0;
                version->minor = 0;
                version->device_id = 0;

                if((chip_instance < SCD_MAX_CHIP) && (scd_chip[chip_instance].handle_id))
                {
                    if(scd_chip[chip_instance].function.get_version)
                    {
                        scd_chip[chip_instance].function.get_version(&scd_chip[chip_instance], item, version);
                    }
                }

                version->name = scd_fat[instance].name;

                return SCD_RESULT__OK;
            }
            break;
        
        case SCD_ITEM__CHIP:
            if((instance < SCD_MAX_CHIP) && (scd_chip[instance].handle_id))
            {
                if(scd_chip[instance].function.get_version)
                {
                    return scd_chip[instance].function.get_version(&scd_chip[instance], item, version);
                }
                else
                {
                    return SCD_RESULT__CHIP_FEATURE_NOT_IMPLEMENTED;
                }
            }
            break;           

        case SCD_ITEM__FIRMWARE: 
            if((instance < SCD_MAX_CHIP) && (scd_chip[instance].handle_id))
            {
                if(scd_chip[instance].function.get_version)
                {
                    return scd_chip[instance].function.get_version(&scd_chip[instance], item, version);
                }
                else
                {
                    return SCD_RESULT__CHIP_FEATURE_NOT_IMPLEMENTED;
                }
            }
            break;

        default:
            break;
        }
    }

    return SCD_RESULT__ARG_OUT_OF_RANGE;
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdOpenFat(uint32_t instance, SCD_HANDLE *fat_handle)
{
    SCD_OBJECT chip;
    SCD_OBJECT fat;    
    SCD_RESULT r;
    SCD_RESULT r2;

    API_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdOpenFat(instance=%u, &fat_handle=%08X)", instance, fat_handle)));

    if(fat_handle)
    {
        *fat_handle = 0;
    }

    if(!scd_initialized)
    {
        BDBG_ERR(("BTFE_P_ScdOpenFat: SCD not initialized"));
        return SCD_RESULT__SCD_NOT_INITIALIZED;
    }

    /* check args */
    if(instance >= SCD_MAX_FAT)
    {
        BDBG_ERR(("BTFE_P_ScdOpenFat: instance %u out of range", instance));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    /* check data */
    fat.demod = &scd_fat[instance];

    if(fat.demod->handle_id != SCD_HANDLE_FAT)
    {
        BDBG_ERR(("BTFE_P_ScdOpenFat: instance %u not available", instance));
        return SCD_RESULT__FAT_NOT_AVAILABLE;
    }

    if(fat.demod->open_count == 0)
    {
        /* open chip */
        if((r = BTFE_P_ScdOpenChip(fat.demod->chip_instance, &chip.handle)) != SCD_RESULT__OK)
        {
            BDBG_ERR(("BTFE_P_ScdOpenFat: open chip failed"));
            return r;
        }
    }

    /* open FAT */
    chip.chip = &scd_chip[fat.demod->chip_instance];

    if(chip.chip->function.open)
    {
        r2 = chip.chip->function.open(chip.handle, fat.handle);
        if(r2 != SCD_RESULT__OK)
        {
            BDBG_ERR(("BTFE_P_ScdOpenFat: error opening chip FAT"));

            BTFE_P_ScdCloseChip(&scd_chip[fat.demod->chip_instance]);

            return r2;
        }
    }

    if(fat.demod->open_count == 0)
    {
        fat.demod->state = SCD_STATE_OPEN;
        fat.demod->last_mod = SCD_MOD_FORMAT__UNKNOWN;
    }

    fat.demod->open_count += 1;

    OPEN_DEBUG(BDBG_MSG(("BTFE_P_ScdOpenFat: open_count=%u", fat.demod->open_count)));

    if(fat_handle)
    {
        *fat_handle = fat.handle;
    }

    return SCD_RESULT__OK;
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdCloseFat(SCD_HANDLE handle)
{
    SCD_OBJECT fat;
    SCD_RESULT r2;
    SCD_OBJECT chip;

    API_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdCloseFat(handle=%08X)", handle)));

    if(!scd_initialized)
    {
        BDBG_ERR(("BTFE_P_ScdCloseFat: SCD not initialized"));
        return SCD_RESULT__SCD_NOT_INITIALIZED;
    }

    /* check args */
    if(IS_BAD_HANDLE(handle))
    {
        BDBG_ERR(("BTFE_P_ScdCloseFat: bad handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(IS_NOT_HANDLE_TYPE(handle, SCD_HANDLE_FAT))
    {
        BDBG_ERR(("BTFE_P_ScdCloseFat: bad handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    fat.handle = handle;

    /* if open */
    if(fat.demod->open_count)
    {
        /* stop if last */
        if(fat.demod->open_count == 1)
        {
            if(fat.demod->state == SCD_STATE_RUNNING)
            {
                BDBG_MSG(("BTFE_P_ScdCloseFat: stopping"));
                BTFE_P_ScdStop(handle);
            }
        }

        /* close FAT */
        fat.demod->open_count -= 1;

        OPEN_DEBUG(BDBG_MSG(("BTFE_P_ScdCloseFat: open_count=%u", fat.demod->open_count)));

        chip.chip = &scd_chip[fat.demod->chip_instance];

        if(chip.chip->function.close)
        {
            r2 = chip.chip->function.close(chip.chip, fat.handle);
            if(r2 != SCD_RESULT__OK)
            {
                BDBG_ERR(("BTFE_P_ScdCloseFat: error closing chip FAT"));
                return r2;
            }
        }

        /* close chip if last */
        if(fat.demod->open_count == 0)
        {
            BTFE_P_ScdCloseChip(&scd_chip[fat.demod->chip_instance]);
            fat.demod->state = SCD_STATE_CLOSED;
        }
    }
    else
    {
        BDBG_ERR(("BTFE_P_ScdCloseFat: FAT not open"));
        return SCD_RESULT__FAT_NOT_OPEN;
    }

    return SCD_RESULT__OK;
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdOpenChip(uint32_t instance, SCD_HANDLE *chip_handle)
{
    SCD_OBJECT chip;
    SCD_RESULT r;
    SCD_RESULT r2;

    API_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdOpenChip(instance=%u, &chip_handle)", instance, chip_handle)));

    if(chip_handle)
    {
        *chip_handle = 0;
    }

    if(!scd_initialized)
    {
        BDBG_ERR(("BTFE_P_ScdOpenChip: SCD not initialized"));
        return SCD_RESULT__SCD_NOT_INITIALIZED;
    }

    /* check args */
    if(instance >= SCD_MAX_CHIP)
    {
        BDBG_ERR(("BTFE_P_ScdOpenChip: instance %u out of range", instance));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    /* open chip */
    chip.chip = &scd_chip[instance];

    if(chip.chip->handle_id != SCD_HANDLE_CHIP)
    {
        BDBG_ERR(("BTFE_P_ScdOpenChip: chip instance %u not available", instance));
        return SCD_RESULT__CHIP_NOT_AVAILABLE;
    }

    chip.chip->open_count += 1;

    OPEN_DEBUG(BMSG_MSG(("BTFE_P_ScdOpenChip: open_count=%u", chip.chip->open_count)));

    /* if first time open */
    if(chip.chip->open_count == 1)
    {
        /* HAL open chip */
        if((r = BTFE_P_HalOpenChip(instance)) != SCD_RESULT__OK)
        {
            BDBG_ERR(("BTFE_P_ScdOpenChip: error in BTFE_P_HalOpenChip"));
            chip.chip->open_count = 0;
            return r;
        }

        /* initialize chip */
        if(chip.chip->function.open)
        {
            r2 = chip.chip->function.open(chip.handle, chip.handle);
            if(r2 != SCD_RESULT__OK)
            {
                BDBG_ERR(("BTFE_P_ScdOpenChip: error opening chip instance %u", instance));
                chip.chip->open_count = 0;
                return r2;
            }
        }
    }

    OPEN_DEBUG(BDBG_MSG(("BTFE_P_ScdOpenChip: open_count=%u", chip.chip->open_count)));

    chip.chip->state = SCD_STATE_OPEN;

    if(chip_handle)
    {
        *chip_handle = chip.handle;
    }

    return SCD_RESULT__OK;
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdCloseChip(SCD_HANDLE handle)
{
    SCD_OBJECT chip;
    SCD_RESULT r;
    SCD_RESULT r2;

    API_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdCloseChip(handle=%08X)", handle)));

    if(!scd_initialized)
    {
        BDBG_ERR(("BTFE_P_ScdCloseChip: SCD not initialized"));
        return SCD_RESULT__SCD_NOT_INITIALIZED;
    }

    /* check args */
    if(IS_BAD_HANDLE(handle))
    {
        BDBG_ERR(("BTFE_P_ScdCloseChip: bad handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(IS_NOT_HANDLE_TYPE(handle, SCD_HANDLE_CHIP))
    {
        BDBG_ERR(("BTFE_P_ScdCloseChip: bad handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    /* if open */
    chip.handle = handle;

    if(chip.chip->open_count)
    {
        /* if last close */
        if(chip.chip->open_count == 1)
        {
            if(chip.chip->function.close)
            {
                r2 = chip.chip->function.close(chip.chip, chip.chip);
                if(r2 != SCD_RESULT__OK)
                {
                    BDBG_ERR(("BTFE_P_ScdCloseChip: error closing chip"));
                    return r2;
                }
            }

            chip.chip->state = SCD_STATE_CLOSED;

            /* HAL close chip */
            if((r = BTFE_P_HalCloseChip(chip.chip->chip_instance)) != SCD_RESULT__OK)
            {
                BDBG_ERR(("BTFE_P_ScdCloseChip: error in BTFE_P_HalCloseChip"));
                return r;
            }
        }

        /* close chip */
        chip.chip->open_count -= 1;

        OPEN_DEBUG(BDBG_MSG(("BTFE_P_ScdCloseChip: open_count=%u", chip.chip->open_count)));
    }
    else
    {
        BDBG_ERR(("BTFE_P_ScdCloseChip: chip not open"));
        return SCD_RESULT__CHIP_NOT_OPEN;
    }

    return SCD_RESULT__OK;
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdStart(SCD_HANDLE handle, SCD_MOD_FORMAT format)
{
    SCD_OBJECT chip;
    SCD_OBJECT demod;
    SCD_RESULT r=SCD_RESULT__OK;
    SCD_RESULT r2;

    API_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdStart(handle=%08X, format=%u)", handle, format)));

    if(!scd_initialized)
    {
        BDBG_ERR(("BTFE_P_ScdStart: SCD not initialized"));
        return SCD_RESULT__SCD_NOT_INITIALIZED;
    }

    /* check args */
    if(IS_BAD_HANDLE(handle))
    {
        BDBG_ERR(("BTFE_P_ScdStart: bad handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(!IS_DEMOD_HANDLE(handle))
    {
        BDBG_ERR(("BTFE_P_ScdStart: bad handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(BTFE_P_ScdIsHandleClosed(handle))
    {
        BDBG_ERR(("BTFE_P_ScdStart: handle is not open"));
        return SCD_RESULT__HANDLE_NOT_OPEN;
    }

    demod.handle = handle;

    if(format == SCD_MOD_FORMAT__LAST)
    {
        format = (SCD_MOD_FORMAT)demod.demod->last_mod;
    }
    else
    {
        demod.demod->last_mod = format;
    }

    /* chip interface start */
    if(BTFE_P_ScdGetChip(handle, &chip.handle) == SCD_RESULT__OK)
    {
        if(chip.chip->function.start)
        {
            r2 = chip.chip->function.start(chip.chip, handle, format);
            if(r2 != SCD_RESULT__OK)
            {
                BDBG_ERR(("BTFE_P_ScdStart: error starting chip"));
                return r2;
            }
        }
    }

    demod.demod->state = SCD_STATE_RUNNING;

    return r;
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdStop(SCD_HANDLE handle)
{
    SCD_OBJECT chip;
    SCD_OBJECT demod;
    SCD_RESULT r=SCD_RESULT__OK;
    SCD_RESULT r2;

    API_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdStop(handle=%08X)", handle)));

    if(!scd_initialized)
    {
        BDBG_ERR(("BTFE_P_ScdStop: SCD not initialized"));
        return SCD_RESULT__SCD_NOT_INITIALIZED;
    }

    /* check args */
    if(IS_BAD_HANDLE(handle))
    {
        BDBG_ERR(("BTFE_P_ScdStop: bad handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(!IS_DEMOD_HANDLE(handle))
    {
        BDBG_ERR(("BTFE_P_ScdStop: bad handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(BTFE_P_ScdIsHandleClosed(handle))
    {
        BDBG_ERR(("BTFE_P_ScdStop: handle is not open"));
        return SCD_RESULT__HANDLE_NOT_OPEN;
    }

    /* chip interface stop */
    if(BTFE_P_ScdGetChip(handle, &chip.handle) == SCD_RESULT__OK)
    {
        if(chip.chip->function.stop)
        {
            r2 = chip.chip->function.stop(chip.chip, handle);
            if(r2 != SCD_RESULT__OK)
            {
                BDBG_ERR(("BTFE_P_ScdStart: error stopping chip"));
                return r2;
            }
        }
    }

    demod.handle = handle;
    demod.demod->state = SCD_STATE_OPEN;

    return r;
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdSetConfig(SCD_HANDLE handle, SCD_CONFIG_ITEM item, void *data, uint32_t size)
{
    SCD_OBJECT chip;
    SCD_RESULT r = SCD_RESULT__OK;
    SCD_RESULT r2 = SCD_RESULT__OK;
    BSTD_UNUSED(item);
    BSTD_UNUSED(data);
    BSTD_UNUSED(size);
	
#ifdef ENABLE_TUNER_FUNCS
    SCD_STATUS__TUNER_AGC tuner_agc;
	SCD_CONFIG__AGC_SCRIPT agc_script;
#endif

    API_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdSetConfig(handle=%08X, item=%u, data=%08X, size=%u)", handle, item, data, size)));

    if(!scd_initialized)
    {
        BDBG_ERR(("BTFE_P_ScdSetConfig: SCD not initialized"));
        return SCD_RESULT__SCD_NOT_INITIALIZED;
    }

    /* check args */
    if(IS_BAD_HANDLE(handle))
    {
        BDBG_ERR(("BTFE_P_ScdSetConfig: bad handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(BTFE_P_ScdIsHandleClosed(handle))
    {
        BDBG_ERR(("BTFE_P_ScdSetConfig: handle is not open"));
        return SCD_RESULT__HANDLE_NOT_OPEN;
    }

    if((r = BTFE_P_ScdGetChip(handle, &chip.handle)) != SCD_RESULT__OK)
    {
        BDBG_ERR(("BTFE_P_ScdSetConfig: no chip"));
        return r;
    }

    if(chip.chip == SCD_NULL)
    {
        BDBG_ERR(("BTFE_P_ScdSetConfig: bad chip handle"));
        return SCD_RESULT__CHIP_NOT_AVAILABLE;
    }

#ifdef ENABLE_TUNER_FUNCS

    if(item == SCD_CONFIG_ITEM__SET_TUNER)
    {
        SCD_CONFIG__SET_TUNER *config_set_tuner = (SCD_CONFIG__SET_TUNER *)data;

        if(config_set_tuner)
        {
            CONFIG_DEBUG(BDBG_MSG(("BTFE_P_ScdSetConfig(handle=%08X, item=SCD_CONFIG_ITEM__SET_TUNER, tuner_instance=%u)", handle, config_set_tuner->tuner_instance)));

            if(IS_DEMOD_HANDLE(handle))
            {
                SCD_DEMOD *demod = (SCD_DEMOD *) handle;

                if(config_set_tuner->tuner_instance < SCD_MAX_TUNER)
                {
                    /* add tuner to demod */
                    demod->tuner_instance = config_set_tuner->tuner_instance;

                    /* set AGC */
					if((r = (SCD_RESULT)scd_tuner[demod->tuner_instance].function.GetAgcScript((TfeTuner_Handle)handle, &(agc_script.pdata))) == SCD_RESULT__OK)
					{
                        if((r = BTFE_P_ScdSetConfig(demod, SCD_CONFIG_ITEM__AGC_SCRIPT, &agc_script, sizeof(SCD_CONFIG__AGC_SCRIPT))) != SCD_RESULT__OK)
                        {
                            BDBG_ERR(("BTFE_P_ScdSetConfig: set AGC script failed"));

                            return r;
                        }
                    }
                    else
                    {
                        BDBG_ERR(("BTFE_P_ScdSetConfig: get tuner AGC failed"));

                        return r;
                    }
                }
                else
                {
                    BDBG_ERR(("BTFE_P_ScdSetConfig: tuner instance out of range"));

                    return SCD_RESULT__TUNER_NOT_AVAILABLE;
                }
            }
        }
        else
        {
            BDBG_ERR(("BTFE_P_ScdSetConfig: arg out of range"));

            return SCD_RESULT__ARG_OUT_OF_RANGE;
        }
    }

	#endif

    /* do chip interface set config */
    if(chip.chip->function.set_config)
    {
        r2 = chip.chip->function.set_config(chip.chip, handle, item, data, size);
        if(r2 != SCD_RESULT__OK)
        {
            BDBG_ERR(("BTFE_P_ScdSetConfig: error setting chip config"));
            return r2;
        }

        return SCD_RESULT__OK;
    }
    else
    {
        return SCD_RESULT__CHIP_FEATURE_NOT_IMPLEMENTED;
    }
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdGetStatus(SCD_HANDLE handle, SCD_STATUS_ITEM item, void *data, uint32_t size)
{
    SCD_OBJECT chip;
    SCD_OBJECT demod;
    SCD_RESULT r;
    SCD_RESULT r2;

	/* #pragma message ("----------------> BTFE_P_ScdGetStatus:  Put the following commented line back in the code !! ") */

    API_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdGetStatus(handle=%08X, item=%u, data=%08X, size=%u)", handle, item, data, size)));

    if(!scd_initialized)
    {
        BDBG_ERR(("BTFE_P_ScdGetStatus: SCD not initialized"));
        return SCD_RESULT__SCD_NOT_INITIALIZED;
    }

    /* check args */
    if(IS_BAD_HANDLE(handle))
    {
        BDBG_ERR(("BTFE_P_ScdGetStatus: bad handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(BTFE_P_ScdIsHandleClosed(handle))
    {
        BDBG_ERR(("BTFE_P_ScdGetStatus: handle is not open"));
        return SCD_RESULT__HANDLE_NOT_OPEN;
    }

    if((r = BTFE_P_ScdGetChip(handle, &chip.handle)) != SCD_RESULT__OK)
    {
        BDBG_ERR(("BTFE_P_ScdGetStatus: no chip"));
        return r;
    }

    if(chip.chip == SCD_NULL)
    {
        BDBG_ERR(("BTFE_P_ScdGetStatus: bad chip handle"));
        return SCD_RESULT__CHIP_NOT_AVAILABLE;
    }

    /* chip interface get status */
    if(chip.chip->function.get_status)
    {
        r2 = chip.chip->function.get_status(chip.chip, handle, item, data, size);
        if(r2 != SCD_RESULT__OK)
        {
            BDBG_ERR(("BTFE_P_ScdGetStatus: error %u getting chip status", r2));
            return r2;
        }
    }
    else
    {
        return SCD_RESULT__CHIP_FEATURE_NOT_IMPLEMENTED;
    }

    if(item == SCD_STATUS_ITEM__FAT)
    {
        if(size != sizeof(SCD_STATUS__FAT))
        {
            BDBG_ERR(("BTFE_P_ScdGetStatus: bad size"));
            return SCD_RESULT__ARG_OUT_OF_RANGE;
        }

        if(GET_HANDLE_TYPE(handle) != SCD_HANDLE_FAT)
        {
            BDBG_ERR(("BTFE_P_ScdGetStatus: bad handle type"));
            return SCD_RESULT__ARG_OUT_OF_RANGE;
        }

        demod.handle = handle;

        ((SCD_STATUS__FAT *) data)->Started = (demod.demod->state == SCD_STATE_RUNNING);
    }
    else if(item == SCD_STATUS_ITEM__FDC)
    {
        if(size != sizeof(SCD_STATUS__FDC))
        {
            BDBG_ERR(("BTFE_P_ScdGetStatus: bad size"));
            return SCD_RESULT__ARG_OUT_OF_RANGE;
        }

        if(GET_HANDLE_TYPE(handle) != SCD_HANDLE_FDC)
        {
            BDBG_ERR(("BTFE_P_ScdGetStatus: bad handle type"));
            return SCD_RESULT__ARG_OUT_OF_RANGE;
        }

        demod.handle = handle;

        ((SCD_STATUS__FDC *) data)->Started = (demod.demod->state == SCD_STATE_RUNNING);
    }

    return SCD_RESULT__OK;
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdGetChip(SCD_HANDLE handle, SCD_HANDLE *chip_handle)
{

	/* #pragma message ("----------------> BTFE_P_ScdGetChip:  Put the following commented line back in the code !! ") */

    API_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdGetChip(handle=%08X, &chip_handle=%08X)", handle, chip_handle)));

    if(!scd_initialized)
    {
        BDBG_ERR(("BTFE_P_ScdGetChip: SCD not initialized"));
        return SCD_RESULT__SCD_NOT_INITIALIZED;
    }

    /* check args */
    if(IS_BAD_HANDLE(handle))
    {
        BDBG_ERR(("BTFE_P_ScdGetChip: bad handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    /* get chip handle */
    switch(GET_HANDLE_TYPE(handle))
    {
        case SCD_HANDLE_FAT: *chip_handle = &scd_chip[((SCD_DEMOD *) handle)->chip_instance]; break;
        case SCD_HANDLE_FDC: *chip_handle = &scd_chip[((SCD_DEMOD *) handle)->chip_instance]; break;
        case SCD_HANDLE_CHIP: *chip_handle = handle; break;

        default: BDBG_ERR(("BTFE_P_ScdGetChip: invalid handle type")); return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(IS_BAD_HANDLE(*chip_handle))
    {
        BDBG_ERR(("BTFE_P_ScdGetChip: bad chip handle"));
        return SCD_RESULT__CHIP_NOT_AVAILABLE;
    }

    return SCD_RESULT__OK;
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdWriteGpio(SCD_HANDLE handle, uint32_t mask, uint32_t value)
{
    SCD_OBJECT chip;
    SCD_RESULT r;
    SCD_RESULT r2;

    API_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdWriteGpio(handle=%08X, mask=%08X, value=%08X)", handle, mask, value)));

    if(!scd_initialized)
    {
        BDBG_ERR(("BTFE_P_ScdWriteGpio: SCD not initialized"));
        return SCD_RESULT__SCD_NOT_INITIALIZED;
    }

    /* check args */
    if(IS_BAD_HANDLE(handle))
    {
        BDBG_ERR(("BTFE_P_ScdWriteGpio: bad handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(BTFE_P_ScdIsHandleClosed(handle))
    {
        BDBG_ERR(("BTFE_P_ScdWriteGpio: handle is not open"));
        return SCD_RESULT__HANDLE_NOT_OPEN;
    }

    if((r = BTFE_P_ScdGetChip(handle, &chip.handle)) != SCD_RESULT__OK)
    {
        BDBG_ERR(("BTFE_P_ScdWriteGpio: no chip"));
        return r;
    }

    if(chip.chip == SCD_NULL)
    {
        BDBG_ERR(("BTFE_P_ScdWriteGpio: bad chip handle"));
        return SCD_RESULT__CHIP_NOT_AVAILABLE;
    }

    /* chip interface write gpio */
    if(chip.chip->function.write_gpio)
    {
        r2 = chip.chip->function.write_gpio(chip.chip, mask, value);
        if(r2 != SCD_RESULT__OK)
        {
            BDBG_ERR(("BTFE_P_ScdWriteGpio: error writing chip GPIO"));
            return r2;
        }

        return SCD_RESULT__OK;
    }
    else
    {
        return SCD_RESULT__CHIP_FEATURE_NOT_IMPLEMENTED;
    }
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdReadGpio(SCD_HANDLE handle, uint32_t mask, uint32_t *value)
{
    SCD_OBJECT chip;
    SCD_RESULT r;
    SCD_RESULT r2;

    API_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdReadGpio(handle=%08X, mask=%08X, &value=%08X)", handle, mask, value)));

    if(!scd_initialized)
    {
        BDBG_ERR(("BTFE_P_ScdReadGpio: SCD not initialized"));
        return SCD_RESULT__SCD_NOT_INITIALIZED;
    }

    /* check args */
    if(IS_BAD_HANDLE(handle))
    {
        BDBG_ERR(("BTFE_P_ScdReadGpio: bad handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(BTFE_P_ScdIsHandleClosed(handle))
    {
        BDBG_ERR(("BTFE_P_ScdReadGpio: handle is not open"));
        return SCD_RESULT__HANDLE_NOT_OPEN;
    }

    if((r = BTFE_P_ScdGetChip(handle, &chip.handle)) != SCD_RESULT__OK)
    {
        BDBG_ERR(("BTFE_P_ScdReadGpio: no chip"));
        return r;
    }

    if(chip.chip == SCD_NULL)
    {
        BDBG_ERR(("BTFE_P_ScdReadGpio: bad chip handle"));
        return SCD_RESULT__CHIP_NOT_AVAILABLE;
    }

    /* chip interface read gpio */
    if(chip.chip->function.read_gpio)
    {
        r2 = chip.chip->function.read_gpio(chip.chip, mask, value);
        if(r2 != SCD_RESULT__OK)
        {
            BDBG_ERR(("BTFE_P_ScdReadGpio: error reading chip GPIO"));
            return r2;
        }

        return SCD_RESULT__OK;
    }
    else
    {
        return SCD_RESULT__CHIP_FEATURE_NOT_IMPLEMENTED;
    }
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdWriteChip(SCD_HANDLE handle, uint32_t aper, uint32_t offset, uint32_t length, uint8_t *buffer)
{
    SCD_OBJECT chip;
    SCD_RESULT r;
    SCD_RESULT r2;

    API_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdWriteChip(handle=%08X, aper=%u, offset=%08X, &buffer=%08X, length=%u)", handle, aper, offset, buffer, length)));

    if(!scd_initialized)
    {
        BDBG_ERR(("BTFE_P_ScdWriteChip: SCD not initialized"));
        return SCD_RESULT__SCD_NOT_INITIALIZED;
    }

    /* check args */
    if(IS_BAD_HANDLE(handle))
    {
        BDBG_ERR(("BTFE_P_ScdWriteChip: bad handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(BTFE_P_ScdIsHandleClosed(handle))
    {
        BDBG_ERR(("BTFE_P_ScdWriteChip: handle is not open"));
        return SCD_RESULT__HANDLE_NOT_OPEN;
    }

    if((r = BTFE_P_ScdGetChip(handle, &chip.handle)) != SCD_RESULT__OK)
    {
        BDBG_ERR(("BTFE_P_ScdWriteChip: no chip"));
        return r;
    }

    if(chip.chip == SCD_NULL)
    {
        BDBG_ERR(("BTFE_P_ScdWriteChip: bad chip handle"));
        return SCD_RESULT__CHIP_NOT_AVAILABLE;
    }

    if(chip.chip->function.write)
    {
        r2 = chip.chip->function.write(chip.chip, aper, offset, length, buffer);
    }
    else
    {
        r2 = BTFE_P_HalWriteChip(chip.chip->chip_instance, aper, offset, length, buffer);
    }

    if(r2 != SCD_RESULT__OK)
    {
        BDBG_ERR(("BTFE_P_ScdWriteChip: error writing chip"));
        return r2;
    }

    return SCD_RESULT__OK;
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdReadChip(SCD_HANDLE handle, uint32_t aper, uint32_t offset, uint32_t length, uint8_t *buffer)
{
    SCD_OBJECT chip;
    SCD_RESULT r;
    SCD_RESULT r2;

    API_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdReadChip(handle=%08X, aper=%u, offset=%08X, &buffer=%08X, length=%u)", handle, aper, offset, buffer, length)));

    if(!scd_initialized)
    {
        BDBG_ERR(("BTFE_P_ScdReadChip: SCD not initialized"));
        return SCD_RESULT__SCD_NOT_INITIALIZED;
    }

    /* check args */
    if(IS_BAD_HANDLE(handle))
    {
        BDBG_ERR(("BTFE_P_ScdReadChip: bad handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(BTFE_P_ScdIsHandleClosed(handle))
    {
        BDBG_ERR(("BTFE_P_ScdReadChip: handle is not open"));
        return SCD_RESULT__HANDLE_NOT_OPEN;
    }

    if((r = BTFE_P_ScdGetChip(handle, &chip.handle)) != SCD_RESULT__OK)
    {
        BDBG_ERR(("BTFE_P_ScdReadChip: no chip"));
        return r;
    }

    if(chip.chip == SCD_NULL)
    {
        BDBG_ERR(("BTFE_P_ScdReadChip: bad chip handle"));
        return SCD_RESULT__CHIP_NOT_AVAILABLE;
    }

    if(chip.chip->function.read)
    {
        r2 = chip.chip->function.read(chip.chip, aper, offset, length, buffer);
    }
    else
    {
        r2 = BTFE_P_HalReadChip(chip.chip->chip_instance, aper, offset, length, buffer);
    }

    if(r2 != SCD_RESULT__OK)
    {
        BDBG_ERR(("BTFE_P_ScdReadChip: error reading chip"));
        return r2;
    }

    return SCD_RESULT__OK;
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdWriteI2C(SCD_HANDLE handle, uint32_t i2c_addr, uint32_t i2c_subaddr, uint32_t subaddr_len, uint32_t data_len, uint8_t *buffer)
{
    SCD_OBJECT chip;
    SCD_RESULT r;
    SCD_RESULT r2;

    API_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdWriteI2C(handle=%08X, i2c_addr=%08X, i2c_subaddr=%08X, subaddr_len=%u, data_len=%u, &buffer=%08X)", handle, i2c_addr, i2c_subaddr, subaddr_len, data_len, buffer)));

    if(!scd_initialized)
    {
        BDBG_ERR(("BTFE_P_ScdWriteI2C: SCD not initialized"));
        return SCD_RESULT__SCD_NOT_INITIALIZED;
    }

    /* check args */
    if(IS_BAD_HANDLE(handle))
    {
        BDBG_ERR(("BTFE_P_ScdWriteI2C: bad handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(BTFE_P_ScdIsHandleClosed(handle))
    {
        BDBG_ERR(("BTFE_P_ScdWriteI2C: handle is not open"));
        return SCD_RESULT__HANDLE_NOT_OPEN;
    }

    if((r = BTFE_P_ScdGetChip(handle, &chip.handle)) != SCD_RESULT__OK)
    {
        BDBG_ERR(("BTFE_P_ScdWriteI2C: no chip"));
        return r;
    }

    if(chip.chip == SCD_NULL)
    {
        BDBG_ERR(("BTFE_P_ScdWriteI2C: bad chip handle"));
        return SCD_RESULT__CHIP_NOT_AVAILABLE;
    }

    /* chip interface write i2c */
    if(chip.chip->function.write_i2c) 
    {
        r2 = chip.chip->function.write_i2c(chip.chip, i2c_addr, i2c_subaddr, subaddr_len, data_len, buffer);
        if(r2 != SCD_RESULT__OK)
        {
            BDBG_ERR(("BTFE_P_ScdWriteI2C: error writing chip I2C"));
            return r2;
        }

        return SCD_RESULT__OK;
    }
    else
    {
        return SCD_RESULT__CHIP_FEATURE_NOT_IMPLEMENTED;
    }
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdReadI2C(SCD_HANDLE handle, uint32_t i2c_addr, uint32_t i2c_subaddr, uint32_t subaddr_len, uint32_t data_len, uint8_t *buffer)
{
    SCD_OBJECT chip;
    SCD_RESULT r;
    SCD_RESULT r2;

    API_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdReadI2C(handle=%08X, i2c_addr=%08X, i2c_subaddr=%08X, subaddr_len=%u, data_len=%u, &buffer=%08X)", handle, i2c_addr, i2c_subaddr, subaddr_len, data_len, buffer)));

    if(!scd_initialized)
    {
        BDBG_ERR(("BTFE_P_ScdReadI2C: SCD not initialized"));
        return SCD_RESULT__SCD_NOT_INITIALIZED;
    }

    /* check args */
    if(IS_BAD_HANDLE(handle))
    {
        BDBG_ERR(("BTFE_P_ScdReadI2C: bad handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(BTFE_P_ScdIsHandleClosed(handle))
    {
        BDBG_ERR(("BTFE_P_ScdReadI2C: handle is not open"));
        return SCD_RESULT__HANDLE_NOT_OPEN;
    }

    if((r = BTFE_P_ScdGetChip(handle, &chip.handle)) != SCD_RESULT__OK)
    {
        BDBG_ERR(("BTFE_P_ScdReadI2C: no chip"));
        return r;
    }

    if(chip.chip == SCD_NULL)
    {
        BDBG_ERR(("BTFE_P_ScdReadI2C: bad chip handle"));
        return SCD_RESULT__CHIP_NOT_AVAILABLE;
    }

    /* chip interface read i2c */
    if(chip.chip->function.read_i2c)
    {
        r2 = chip.chip->function.read_i2c(chip.chip, i2c_addr, i2c_subaddr, subaddr_len, data_len, buffer);
        if(r2 != SCD_RESULT__OK)
        {
            BDBG_ERR(("BTFE_P_ScdReadI2C: error reading chip I2C"));
            return r2;
        }

        return SCD_RESULT__OK;
    }
    else
    {
        return SCD_RESULT__CHIP_FEATURE_NOT_IMPLEMENTED;
    }
}

/****************************************************************************/
/* internal functions */
/****************************************************************************/

SCD_RESULT BTFE_P_ScdAddChip(uint32_t chip_instance, SCD_CHIP_FUNCTIONS *functions, SCD_HANDLE *chip_handle, void *user)
{
    SCD_OBJECT chip;

    INT_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdAddChip(chip_instance=%u, &functions=%08X)", chip_instance, functions)));

    /* check args */
    if(chip_instance >= SCD_MAX_CHIP)
    {
        BDBG_ERR(("BTFE_P_ScdAddChip: chip instance %u out of range", chip_instance));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(functions == NULL)
    {
        BDBG_ERR(("BTFE_P_ScdAddChip: functions %08X out of range", functions));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    /* add chip */
    chip.chip = &scd_chip[chip_instance];

    if(chip.chip->handle_id == SCD_HANDLE_CHIP)
    {
        BDBG_ERR(("BTFE_P_ScdAddChip: chip instance %u already added", chip_instance));
        return SCD_RESULT__CHIP_NOT_AVAILABLE;
    }

    BKNI_Memcpy(&chip.chip->function, functions, sizeof(SCD_CHIP_FUNCTIONS));

    chip.chip->chip_instance = chip_instance;
    chip.chip->handle_id = SCD_HANDLE_CHIP;
    chip.chip->user = user;
    chip.chip->mutex_handle = SCD_NULL;

    if(chip_handle)
    {
        *chip_handle = chip.handle;
    }

    return SCD_RESULT__OK;
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdAddFat(uint32_t chip_instance, uint32_t tuner_instance, const char *name, SCD_HANDLE *fat_handle)
{
    SCD_OBJECT fat;
    uint32_t fat_instance;

    BSTD_UNUSED(tuner_instance);
	
    INT_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdAddFat(chip_instance =%u)", chip_instance)));

    /* check args */
    if(chip_instance >= SCD_MAX_CHIP)
    {
        BDBG_ERR(("BTFE_P_ScdAddFat: chip instance %u out of range", chip_instance));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(fat_count >= SCD_MAX_FAT)
    {
        BDBG_ERR(("BTFE_P_ScdAddFat: no FAT available"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    /* add FAT */
    fat_instance = fat_count;
    fat_count += 1;

    fat.demod = &scd_fat[fat_instance];

    if(fat.demod->handle_id == SCD_HANDLE_FAT)
    {
        BDBG_ERR(("BTFE_P_ScdAddFat: fat instance %u already added", fat_instance));
        return SCD_RESULT__FAT_NOT_AVAILABLE;
    }

    fat.demod->demod_instance = fat_instance;
    fat.demod->chip_instance = chip_instance;   
    fat.demod->name = name;
    fat.demod->handle_id = SCD_HANDLE_FAT;

    if(fat_handle)
    {
        *fat_handle = fat.handle;
    }

    return SCD_RESULT__OK;
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdGetChipFromInstance(uint32_t chip_instance, SCD_HANDLE *chip_handle)
{
    INT_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdGetChipFromInstance(chip_instance=%u, &chip_handle=%08X)", chip_instance, chip_handle)));

    /* check args */
    if(chip_instance >= SCD_MAX_CHIP)
    {
        BDBG_ERR(("BTFE_P_ScdGetChipFromInstance: chip instance %u out of range", chip_instance));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    if(chip_handle == NULL)
    {
        BDBG_ERR(("BTFE_P_ScdGetChipFromInstance: chip handle %08X out of range", chip_handle));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    *chip_handle = &scd_chip[chip_instance];

    return SCD_RESULT__OK;
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdGetInstance(SCD_HANDLE handle, uint32_t *instance)
{
    SCD_OBJECT object;

    if(!scd_initialized)
    {
        BDBG_ERR(("BTFE_P_ScdGetInstance: SCD not initialized"));
        return SCD_RESULT__SCD_NOT_INITIALIZED;
    }

    /* check args */
    if(IS_BAD_HANDLE(handle))
    {
        BDBG_ERR(("BTFE_P_ScdGetInstance: bad handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    object.handle = handle;

    switch(GET_HANDLE_TYPE(handle))
    {
        case SCD_HANDLE_FAT:   *instance = object.demod->demod_instance;  break;
        case SCD_HANDLE_FDC:   *instance = object.demod->demod_instance;  break;
        case SCD_HANDLE_CHIP:  *instance = object.chip->chip_instance;    break;

        default: BDBG_ERR(("BTFE_P_ScdGetInstance: invalid handle type")); return SCD_RESULT__ARG_OUT_OF_RANGE;
    }

    return SCD_RESULT__OK;
}

/****************************************************************************/

SCD_RESULT BTFE_P_ScdGetUser(SCD_HANDLE handle, void **user)
{
    SCD_OBJECT object;

    INT_FUNC_DEBUG(BDBG_MSG(("BTFE_P_ScdGetUser(handle=%08X, &user=%u)", handle, user)));

    /* check args */
    if(IS_BAD_HANDLE(handle))
    {
        BDBG_ERR(("BTFE_P_ScdGetUser: bad handle"));
        return SCD_RESULT__ARG_OUT_OF_RANGE;
    }
    
    object.handle = handle;

    if(GET_HANDLE_TYPE(handle) == SCD_HANDLE_CHIP)
    {
        *user = object.chip->user;

        return SCD_RESULT__OK;
    }

    return SCD_RESULT__ERROR;
}

/****************************************************************************/

const char *scd_c_ver_str = "$ID$";
/****************************************************************************/
