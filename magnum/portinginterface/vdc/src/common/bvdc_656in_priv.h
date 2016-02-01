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
#ifndef BVDC_656IN_PRIV_H__
#define BVDC_656IN_PRIV_H__

#include "bvdc.h"
#include "bvdc_common_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * Defines
 ***************************************************************************/
/* 7038x-ish, 3560x-ish, 7401Ax, 7118, 7440 */
#define BVDC_P_656IN_NEW_VER_0                   (0)
/* 7401Bx:
 * Use EXT_656_TOP_0_ext_656_count */
#define BVDC_P_656IN_NEW_VER_1                   (1)
/* 3563, 7405:
 * Use EXT_656_TOP_0_ext_656_count.
 * No  EXT_656_TOP_0_ext_656_reset */
#define BVDC_P_656IN_NEW_VER_2                   (2)
/* 7400-ish:
 * Use EXT_656_TOP_0_ext_656_short_count & EXT_656_TOP_0_ext_656_long_count */
#define BVDC_P_656IN_NEW_VER_3                   (3)

#if (BVDC_P_SUPPORT_NEW_656_IN_VER == BVDC_P_656IN_NEW_VER_3)
#define BVDC_P_NUM_656IN_SUPPORT                 (2)
#else
#define BVDC_P_NUM_656IN_SUPPORT                 (1)
#endif

/* Trigger offset from the picture height. */
#define BVDC_P_656IN_TRIGGER_OFFSET              (6)


/***************************************************************************
 * Private macros
 ***************************************************************************/

/***************************************************************************
 * Private enums
 ***************************************************************************/
typedef enum BVDC_P_656Id
{
	BVDC_P_656Id_e656In0 = 0,
	BVDC_P_656Id_e656In1
} BVDC_P_656Id;

/***************************************************************************
 * 656 Context
 ***************************************************************************/
typedef struct BVDC_P_656InContext
{
	BDBG_OBJECT(BVDC_656)

	BVDC_Source_Handle             hSource;
	BVDC_P_656Id                   eId;
	uint32_t                       ulOffset;

	bool                           bVideoDetected;

	/* 656 output slow start countdown to avoid prematurely output garbage */
	uint32_t                       ulDelayStart;

	/* 656 frame rate code */
	BAVC_FrameRateCode             eFrameRateCode;

} BVDC_P_656InContext;


/***************************************************************************
 * Private function prototypes
 ***************************************************************************/
BERR_Code BVDC_P_656In_Create
	( BVDC_P_656In_Handle             *ph656In,
	  BVDC_P_656Id                     e656Id,
	  BVDC_Source_Handle               hSource );

void BVDC_P_656In_Destroy
	( BVDC_P_656In_Handle              h656In );

void BVDC_P_656In_Init
	( BVDC_P_656In_Handle              h656In );

void BVDC_P_656In_UpdateStatus_isr
	( BVDC_P_656In_Handle              h656In );

void BVDC_P_656In_GetStatus_isr
	( const BVDC_P_656In_Handle        h656In,
	  bool                            *pbVideoDetected );

void BVDC_P_656In_Bringup_isr
	( BVDC_P_656In_Handle              h656In );

void BVDC_P_656In_BuildRul_isr
	( const BVDC_P_656In_Handle        h656In,
	  BVDC_P_ListInfo                 *pList,
	  BAVC_Polarity                    eFieldId );


#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_656IN_PRIV_H__ */
/* End of file. */
