/***************************************************************************
 *     Copyright (c) 2006-2012, Broadcom Corporation
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
#ifndef BCHP_AVS_PRIV_H__
#define BCHP_AVS_PRIV_H__

#include "bstd.h"
#include "bchp.h"

/* 
** The functions in this file are used to provide dymanic testing (or checking) of the AVS module.
** These functions should only be used for testing/validation purposes.  They can be destructive
** and cause the system to crash.  Use these functions with extreme caution.
*/

typedef struct {
	bool valid; /* true if data filled in */

	uint32_t last_dac;  /* the last value written to the DAC */
	uint32_t last_temp; /* last data read from temperature register */

	uint32_t V_0p99, V_1p1_0, V_1p1_1, V_2p75, V_3p63; /* last values read from voltage registers */
	/* below are the converted (to voltages) values of above */
	uint32_t last_voltage_1p1_0, last_voltage_1p1_1, last_voltage_0p99, last_voltage_2p75, last_voltage_3p63;
} AvsTestData;

#ifdef __cplusplus
extern "C" {
#endif

/* These are the different type of "lock" requests permitted */
typedef enum {
	eAvsNoLock,
	eAvsLockRegisters, /* do not verify/restore registers during normal processing */
	eAvsPause          /* pause AVS processing until unlock request */
} AvsLockType_t;

/* The number of oscillators varies from platform to platform -- here is the number on this platform */
unsigned AvsGetNumberCentrals(void);
unsigned AvsGetNumberRemotes(void);

/* Get/Set the DAC */
uint32_t AvsGetDAC(BCHP_P_AvsHandle hHandle);
void AvsSetDAC(BCHP_P_AvsHandle hHandle, uint32_t dac_code);

/* Get status value of a specific oscillator */
uint32_t AvsReadCentralOscillator(BREG_Handle hRegister, unsigned oscillator);
uint32_t AvsReadRemoteOscillator(BREG_Handle hRegister, unsigned oscillator);

/* Get threshold values for a specific oscillator */
void AvsReadCentralOscThresholds(BREG_Handle hRegister, unsigned oscillator, uint32_t *reg_min, uint32_t *reg_max);
void AvsReadRemoteOscThresholds(BREG_Handle hRegister, unsigned oscillator, uint32_t *reg_min, uint32_t *reg_max);

/* This is used to prevent the AVS processing from verifying any of its registers */
BERR_Code AvsLock(BCHP_P_AvsHandle hHandle, AvsLockType_t type);

/* This un-does the Lock operation */
BERR_Code AvsUnlock(BCHP_P_AvsHandle hHandle);

/* This provides miscellaneous AVS data for test purposes */
void AvsGetTestData(BCHP_P_AvsHandle hHandle, AvsTestData *data);

#ifdef __cplusplus
}
#endif

#endif /* BCHP_AVS_PRIV_H__ */

