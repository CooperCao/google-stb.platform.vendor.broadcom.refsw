/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: satfe_platform.h $
 * $brcm_Revision: Hydra_Software_Devel/7 $
 * $brcm_Date: 12/8/11 9:46a $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: /rockford/applications/diags/satfe/bcm97358/satfe_platform.h $
 *
 * Hydra_Software_Devel/7   12/8/11 9:46a ronchan
 * SWSATFE-88: add config L1 interrupt parameter to SATFE_Diags_config
 *
 * Hydra_Software_Devel/6   1/7/11 2:32p ronchan
 * SWSATFE-88: added btmr handle to SATFE_Diags_Config struct
 *
 * Hydra_Software_Devel/5   1/7/11 2:24p ronchan
 * SWSATFE-88: moved main on-idle function to satfe level
 *
 * Hydra_Software_Devel/4   11/19/10 3:53p enavarro
 * SWSATFE-88: added SATFE_Diags_Config struct
 *
 * Hydra_Software_Devel/3   10/15/10 8:39a enavarro
 * SWSATFE-75: fixed compiler warnings
 *
 * Hydra_Software_Devel/2   9/27/10 5:28p enavarro
 * SWSATFE-75: fixed compiler warning
 *
 * Hydra_Software_Devel/1   9/15/10 4:39p enavarro
 * SWSATFE-75: initial version
 *
 ***************************************************************************/
#ifndef _SATFE_PLATFORM_H_
#define _SATFE_PLATFORM_H_

#include "satfe.h"
#include "btmr.h"


/* set SATFE_NUM_CHIPS to the number of Broadcom satellite frontend chips to be
   controlled on the board */
#define SATFE_NUM_CHIPS 1

/* define SATFE_MUTEX */
#define SATFE_MUTEX(x) x

/* required functions to implement */
extern BERR_Code SATFE_Platform_Init(void *pParam);
extern BERR_Code SATFE_Platform_Shutdown(void);
extern void SATFE_Platform_InitDiags(void *pParam);
extern char SATFE_Platform_GetChar(bool bBlocking);
extern void SATFE_Platform_Backspace(void);
extern void SATFE_Platform_GetInputPower(SATFE_Chip *pChip, uint32_t rfagc,
                                         uint32_t ifagc, uint32_t agf, uint32_t tuner_freq,
                                         float *pPower);
extern void SATFE_Platform_StartTimer(void);
extern void SATFE_Platform_KillTimer(void);
extern uint32_t SATFE_Platform_GetTimerCount(void);
extern uint32_t SATFE_Platform_Rand(void);

/* required data structure to initialize */
extern SATFE_ChipDescriptor SATFE_chips[SATFE_NUM_CHIPS];

/* define any other platform-specific data types, variables, etc */
extern BKNI_MutexHandle SATFE_Platform_hFtmMessageMutex;

/* pointer to SATFE_Diags_Config should be passed into SATFE_Diags() by the app */
typedef struct
{
   BREG_Handle hReg;
   BINT_Handle hInt;
   BTMR_Handle hTimer;
   bool        bAstOpenExternal; /* true if AST was openned outside of satfe */
   bool        bConfigL1Int;     /* connect and enable L1 interrupts */
   BAST_Handle        hAst; /* AST handle if bAstOpenExternal is true */
   BAST_ChannelHandle *hAstChannel; /* AST channel handles if bAstOpenExternal is true */
} SATFE_Diags_Config;


#endif
