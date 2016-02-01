/***************************************************************************
 *     (c)2005-2014 Broadcom Corporation
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
 * 
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

/***************************************************************************
 * Definitions
 ***************************************************************************/
#ifndef _BTNR_3x7xIB_PRIV_H__
#define _BTNR_3x7xIB_PRIV_H__

#if __cplusplus
extern "C" {
#endif

#include "btnr_struct.h"
#include "bmem.h"
#include "btmr.h"
#include "btnr_def.h"




#define BREG_WriteField(h, Register, Field, Data) \
  BREG_Write32(h, BCHP_##Register, ((BREG_Read32(h, BCHP_##Register) & \
  ~((uint32_t)BCHP_MASK(Register, Field))) | \
  BCHP_FIELD_DATA(Register, Field, Data)))


#define BREG_ReadField(h, Register, Field) \
  ((BREG_Read32(h, BCHP_##Register) & BCHP_MASK(Register,Field)) >> \
  BCHP_SHIFT(Register,Field))



/***************************************************************************
 * Summary:
 *   Structure for chip-specific TNR handle
 *    
 * Description:
 *   This is the chip-specific component of the BTNR_Handle.
 *     
 * See Also:
 *   None.
 *      
 ***************************************************************************/
typedef struct BTNR_P_3x7x_Handle
{
  uint32_t magicId;                   /* Used to check if structure is corrupt */
  BREG_Handle hRegister;
  BTNR_3x7x_TuneParams_t *pTunerParams;
  BTNR_3x7x_TuneStatus_t *pTunerStatus;
  BTMR_TimerHandle hTmr;
  BMEM_Heap_Handle hHeap;
  BKNI_EventHandle hIntEvent;
  BTNR_3x7x_Settings settings;
} BTNR_P_3x7x_Handle;



typedef struct BTNR_P_3x7x_Handle *BTNR_3x7x_Handle;
/*#define BTNR_3x7x_ChnHandle BTNR_3x7x_Handle
#define BTNR_P_3x7x_ChnHandle BTNR_P_3x7x_Handle*/


/***************************************************************************
*	API function interface
 ***************************************************************************/

BERR_Code BTNR_3x7x_Close(
    BTNR_Handle hDev                    /* [in] Device handle */
    );
BERR_Code BTNR_3x7x_SetSettings(
		  BTNR_3x7x_Handle hDev,    /* [in] Device handle */
		  BTNR_Settings *settings     /* [in] TNR settings. */
		);
BERR_Code BTNR_3x7x_GetSettings(
    BTNR_3x7x_Handle hDev,    /* [in] Device handle */
    BTNR_Settings *settings     /* [out] TNR settings. */
    );
BERR_Code BTNR_3x7x_SetRfFreq(
    BTNR_3x7x_Handle hDev,            /* [in] Device handle */
    uint32_t rfFreq,                    /* [in] Requested tuner freq., in Hertz */
    BTNR_TunerMode tunerMode            /* [in] Requested tuner mode */
    );
BERR_Code BTNR_3x7x_GetRfFreq(
    BTNR_3x7x_Handle hDev,            /* [in] Device handle */
    uint32_t *rfFreq,                   /* [output] Returns tuner freq., in Hertz */
    BTNR_TunerMode *tunerMode);

BERR_Code BTNR_3x7x_GetInfo(
    BTNR_3x7x_Handle hDev,            /* [in] Device handle */
    BTNR_TunerInfo *tnrInfo             /* [out] Tuner information */
    );

BERR_Code BTNR_3x7x_PowerUp (BTNR_3x7x_Handle );
BERR_Code BTNR_3x7x_PowerDown(BTNR_3x7x_Handle );
BERR_Code BTNR_3x7x_SetPowerSaver(
    BTNR_3x7x_Handle hDev,                    /* [in] Device handle */
    BTNR_PowerSaverSettings *pwrSettings /* [in] Power saver settings. */
    );
BERR_Code BTNR_3x7x_GetPowerSaver(
    BTNR_3x7x_Handle hDev,                    /* [in] Device handle */
    BTNR_PowerSaverSettings *pwrSettings        /* [in] Power saver settings. */
    );


#ifdef __cplusplus
}
#endif

#endif /* _BTNR_3x7x_PRIV_H__ */