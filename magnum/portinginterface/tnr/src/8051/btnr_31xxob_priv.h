/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
#ifndef BTNR_31XXOB_PRIV_H__
#define BTNR_31XXOB_PRIV_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "berr_ids.h"
#include "btnr.h"
#include "bhab.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*
*   Private Module Handles
*
*******************************************************************************/

typedef struct BTNR_P_31xxOb_Handle     *BTNR_31xxOb_Handle;

typedef struct BTNR_P_31xxOb_Settings
{
    unsigned long rfFreq;
    unsigned long ifFreq;
    int iRevLetter, iRevNumber, iType;      /* Saved chip information */
    BTNR_TunerMode tunerMode;
    bool powerSaver;
} BTNR_P_31xxOb_Settings;

typedef struct BTNR_P_31xxOb_Handle
{
    uint32_t magicId;                   /* Used to check if structure is corrupt */
    BHAB_Handle hHab;
    BTNR_P_31xxOb_Settings settings;
} BTNR_P_31xxOb_Handle;

/*******************************************************************************
*
*   Private Module Data
*
*******************************************************************************/
BERR_Code BTNR_31xxOb_Close(
    BTNR_Handle hDev                        /* [in] Device handle */
    );

BERR_Code BTNR_31xxOb_SetRfFreq(
    BTNR_31xxOb_Handle hDev,                /* [in] Device handle */
    uint32_t rfFreq,                        /* [in] Requested tuner freq., in Hertz */
    BTNR_TunerMode tunerMode                /* [in] Requested tuner mode */
    );

BERR_Code BTNR_31xxOb_GetRfFreq(
    BTNR_31xxOb_Handle hDev,                /* [in] Device handle */
    uint32_t *rfFreq,                       /* [output] Returns tuner freq., in Hertz */
    BTNR_TunerMode *tunerMode               /* [output] Returns tuner mode */
    );

BERR_Code BTNR_P_31xxOb_GetAgcRegVal(
    BTNR_31xxOb_Handle hDev,                /* [in] Device handle */
    uint32_t regOffset,                     /* [in] AGC register offset */
    uint32_t *agcVal                        /* [out] output value */
    );

BERR_Code BTNR_31xxOb_SetAgcRegVal(
    BTNR_31xxOb_Handle hDev,                /* [in] Device handle */
    uint32_t regOffset,                     /* [in] AGC register offset */
    uint32_t *agcVal                        /* [in] input value */
    );

BERR_Code BTNR_31xxOb_GetInfo(
    BTNR_31xxOb_Handle hDev,                /* [in] Device handle */
    BTNR_TunerInfo *tnrInfo                 /* [out] Tuner information */
    );

BERR_Code BTNR_31xxOb_GetPowerSaver(
    BTNR_31xxOb_Handle hDev,                /* [in] Device handle */
    BTNR_PowerSaverSettings *pwrSettings    /* [in] Power saver settings. */
    );

BERR_Code BTNR_31xxOb_SetPowerSaver(
    BTNR_31xxOb_Handle hDev,                /* [in] Device handle */
    BTNR_PowerSaverSettings *pwrSettings    /* [in] Power saver settings. */
    );

#ifdef __cplusplus
}
#endif
 
#endif



