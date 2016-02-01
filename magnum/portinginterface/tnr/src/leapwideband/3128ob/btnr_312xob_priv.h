/***************************************************************************
 *     Copyright (c) 2005-2012, Broadcom Corporation
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
 ****************************************************************************/
#ifndef BTNR_312XOB_PRIV_H__
#define BTNR_312XOB_PRIV_H__

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

#define CHK_RETCODE( rc, func )         \
do {                                        \
if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
{                                       \
    goto done;                          \
}                                       \
} while(0)

#define HAB_MSG_HDR(OPCODE,N,CORE_TYPE,CORE_ID) \
    { ((uint8_t)(((uint16_t)(OPCODE)) >> 2)), \
    (((uint8_t)(0x03 & (OPCODE)) << 6) | ((uint8_t)((N)>>4))), \
    ((((uint8_t)(((N)& 0x0F) << 4))) | ((uint8_t)(0x0F & (CORE_TYPE)))), \
    ((uint8_t)(CORE_ID)) }

#define BTNR_CORE_TYPE		        0xE
#define BTNR_CORE_TYPE_TO_FEED      0x2
#define BTNR_CORE_ID		        0x0
#define BTNR_CHANNEL_ID		        0x8
#define BTNR_ACQUIRE		        0x10
#define BTNR_ACQUIRE_PARAMS_WRITE 	0x11
#define BTNR_POWER_OFF            	0x18
#define BTNR_POWER_ON            	0x19
#define BTNR_POWER_STATUS_READ     	0x98
#define BTNR_ACQUIRE_PARAMS_READ 	0x91
#define BTNR_SYS_VERSION   	        0XBA

/*******************************************************************************
*
*   Private Module Handles
*
*******************************************************************************/

typedef struct BTNR_P_312xOb_Handle     *BTNR_312xOb_Handle;

typedef struct BTNR_P_312xOb_Settings
{
    unsigned long rfFreq;
    unsigned long ifFreq;
    int iRevLetter, iRevNumber, iType;      /* Saved chip information */
    BTNR_TunerMode tunerMode;   
} BTNR_P_312xOb_Settings;

typedef struct BTNR_P_312xOb_Handle
{
    uint32_t magicId;                   /* Used to check if structure is corrupt */
    BHAB_Handle hHab;
    BTNR_P_312xOb_Settings settings;
    bool bPowerdown;    
} BTNR_P_312xOb_Handle;

/*******************************************************************************
*
*   Private Module Data
*
*******************************************************************************/
BERR_Code BTNR_312xOb_Close(
    BTNR_Handle hDev                        /* [in] Device handle */
    );

BERR_Code BTNR_312xOb_SetRfFreq(
    BTNR_312xOb_Handle hDev,                /* [in] Device handle */
    uint32_t rfFreq,                        /* [in] Requested tuner freq., in Hertz */
    BTNR_TunerMode tunerMode                /* [in] Requested tuner mode */
    );

BERR_Code BTNR_312xOb_GetRfFreq(
    BTNR_312xOb_Handle hDev,                /* [in] Device handle */
    uint32_t *rfFreq,                       /* [output] Returns tuner freq., in Hertz */
    BTNR_TunerMode *tunerMode               /* [output] Returns tuner mode */
    );

BERR_Code BTNR_P_312xOb_GetAgcRegVal(
    BTNR_312xOb_Handle hDev,                /* [in] Device handle */
    uint32_t regOffset,                     /* [in] AGC register offset */
    uint32_t *agcVal                        /* [out] output value */
    );

BERR_Code BTNR_312xOb_SetAgcRegVal(
    BTNR_312xOb_Handle hDev,                /* [in] Device handle */
    uint32_t regOffset,                     /* [in] AGC register offset */
    uint32_t *agcVal                        /* [in] input value */
    );

BERR_Code BTNR_312xOb_GetInfo(
    BTNR_312xOb_Handle hDev,                /* [in] Device handle */
    BTNR_TunerInfo *tnrInfo                 /* [out] Tuner information */
    );

BERR_Code BTNR_312xOb_GetPowerSaver(
    BTNR_312xOb_Handle hDev,                /* [in] Device handle */
    BTNR_PowerSaverSettings *pwrSettings    /* [in] Power saver settings. */
    );

BERR_Code BTNR_312xOb_SetPowerSaver(
    BTNR_312xOb_Handle hDev,                /* [in] Device handle */
    BTNR_PowerSaverSettings *pwrSettings    /* [in] Power saver settings. */
    );

BERR_Code BTNR_312xOb_GetSettings(
    BTNR_312xOb_Handle hDev,                /* [in] Device handle */
    BTNR_Settings *settings                 /* [out] TNR settings. */
    );

BERR_Code BTNR_312xOb_SetSettings(
    BTNR_312xOb_Handle hDev,                /* [in] Device handle */
    BTNR_Settings *settings                 /* [in] TNR settings. */
    );
    
BERR_Code BTNR_312xOb_GetVersionInfo(
    BTNR_312xOb_Handle hDev,        /* [in] Device handle */
    BFEC_VersionInfo *pVersionInfo  /* [out] Returns version Info */
    );
    
#ifdef __cplusplus
}
#endif
 
#endif



