/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bstd.h"
#include "bkni.h"
#include "brsp.h"
#include "brsp_mbyte.h"
#include "brsp_priv.h"




BDBG_MODULE(brsp_mbyte);

#if (BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE)
#define PCI_ADDR8(x)	(x^3)	/* fix addresses for char access  */
#define PCI_ADDR16(x)	(x^2)	/* fix addresses for short access */
#else
#define	PCI_ADDR8(x)	(x)
#define	PCI_ADDR16(x)	(x)
#endif

typedef BERR_Code (*BRSP_P_CallbackFunc) ( void *, uint32_t, uint32_t, uint32_t, uint32_t);

typedef struct BRSP_P_SimpleScript
{
	uint32_t offset;
	uint32_t value;
} BRSP_P_SimpleScript;

typedef struct BRSP_P_RdModWrSimpleScript
{
	uint32_t offset;
	uint32_t mask;
	uint32_t value;
	uint32_t filler0;
	uint32_t filler1;
} BRSP_P_RdModWrSimpleScript;

typedef struct BRSP_P_FuncScript
{
	BRSP_P_CallbackFunc func;
	uint32_t arg1;
	uint32_t arg2;
	uint32_t arg3;
	uint32_t arg4;
} BRSP_P_FuncScript;

typedef struct BRSP_P_NestedScript
{
	BRSP_ScriptTbl *pScriptTbl;
	uint32_t notused;
} BRSP_P_NestedScript;

typedef struct BRSP_P_ScriptEntry
{
	BRSP_ScriptType	scriptType;
	union
	{
		BRSP_P_SimpleScript	regData;
		BRSP_P_RdModWrSimpleScript	regRdModWrData;
		BRSP_P_FuncScript funcData;
		BRSP_P_NestedScript nestedData;
	} scriptInfo;
} BRSP_P_ScriptEntry;



BERR_Code BRSP_MByte_ParseRegScript(
	BRSP_MByte_Handle hRsp,					/* [in] Handle to Register Script */
	const BRSP_ScriptTbl *pScriptTbl,		/* [in] Pointer to Register Script Tbl */
	void *phDev								/* [in] Handle of calling device */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BRSP_P_ScriptEntry *pScript;
	uint32_t ulVal;


	/* Sanity check on the handles we've been given. */
	BDBG_ENTER(BRSP_MByte_ParseRegScript);
	BDBG_ASSERT( hRsp );
	BDBG_ASSERT( hRsp->magicId == BRSP_MAGIC_ID );

	pScript = (BRSP_P_ScriptEntry *) pScriptTbl;

	for(; pScript->scriptType != BRSP_ScriptType_eEndOfScript && retCode == BERR_SUCCESS; )
	{
		switch( pScript->scriptType )
		{
			case BRSP_ScriptType_eWrite8:
				BREG_Write8( hRsp->hReg, (pScript->scriptInfo.regData.offset + hRsp->coreOffset),
					(uint8_t) pScript->scriptInfo.regData.value );
				break;
			case BRSP_ScriptType_eWrite16:
				BREG_Write16( hRsp->hReg, (pScript->scriptInfo.regData.offset + hRsp->coreOffset),
					(uint16_t) pScript->scriptInfo.regData.value );
				break;
			case BRSP_ScriptType_eWrite32:
				BREG_Write32( hRsp->hReg, (pScript->scriptInfo.regData.offset + hRsp->coreOffset),
					(uint32_t) pScript->scriptInfo.regData.value );
				break;
			case BRSP_ScriptType_eRdModWr32:
				ulVal = BREG_Read32( hRsp->hReg, (pScript->scriptInfo.regRdModWrData.offset + hRsp->coreOffset) );
				ulVal &= ~pScript->scriptInfo.regRdModWrData.mask;
				ulVal |= pScript->scriptInfo.regRdModWrData.value;
				BREG_Write32( hRsp->hReg, (pScript->scriptInfo.regRdModWrData.offset + hRsp->coreOffset),
					ulVal );
				break;
			case BRSP_ScriptType_eMbWrite32:
				BRSP_MByte_Write32( hRsp, pScript->scriptInfo.regData.offset,
					pScript->scriptInfo.regData.value );
				break;
			case BRSP_ScriptType_eDelay:
				/* Convert Milliseconds into Microseconds
				BKNI_Delay( pScript->scriptInfo.regData.value * 1000 ); */
				BKNI_Sleep( pScript->scriptInfo.regData.value );
				break;
			case BRSP_ScriptType_eFunc:
				retCode = BERR_TRACE((*(BRSP_P_CallbackFunc)(pScript->scriptInfo.funcData.func))
				    ( phDev, 
				     pScript->scriptInfo.funcData.arg1, 
				     pScript->scriptInfo.funcData.arg2,
				     pScript->scriptInfo.funcData.arg3,
				     pScript->scriptInfo.funcData.arg4 ));
				break;
			case BRSP_ScriptType_eNestedScript:
				retCode = BRSP_MByte_ParseRegScript( hRsp, 
					pScript->scriptInfo.nestedData.pScriptTbl, phDev );
				break;
			case BRSP_ScriptType_eEndOfScript:
				break;
			default:
				retCode = BERR_UNKNOWN;
				break;
		}
		if( retCode == BERR_SUCCESS )
		{
			/* Goto next entry in the script */
			if( pScript->scriptType == BRSP_ScriptType_eFunc || pScript->scriptType == BRSP_ScriptType_eRdModWr32 )
			{
				pScript++;
			}
			else
			{
				pScript = (BRSP_P_ScriptEntry *) ((uint32_t *)pScript + 
					(sizeof(BRSP_P_FuncScript)-sizeof(BRSP_P_SimpleScript))/sizeof(uint32_t));
			}
		}
	}
	BDBG_MSG(("%s: ScriptType=BRSP_ScriptType_eEndOfScript", BSTD_FUNCTION));

	BDBG_LEAVE(BRSP_MByte_ParseRegScript);
	return( retCode );
}


BERR_Code BRSP_MByte_Open(
	BRSP_MByte_Handle *phRsp,				/* [out] MBtye Register Handle */
	BREG_Handle hReg,						/* [in] Register Handle to use */
	uint32_t coreOffset,					/* [in] Core Offset */
	uint32_t nbrWriteOnlyShadowReg			/* [in] Number of Write Only Register, 0 if none */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
 	BRSP_MByte_Handle hRsp;


	BDBG_ENTER(BRSP_MByte_Open);

	hRsp = (BRSP_MByte_Handle) BKNI_Malloc( sizeof( BRSP_P_MByte_Handle ) );
	if( hRsp == NULL )
	{
		*phRsp = NULL;
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BRSP_MByte_Open: BKNI_malloc() failed"));
		goto done;
	}
	BKNI_Memset( hRsp, 0x00, sizeof( BRSP_P_MByte_Handle ) );

	hRsp->magicId = BRSP_MAGIC_ID;
	hRsp->hReg = hReg;
	hRsp->coreOffset = coreOffset;
	hRsp->nbrShadowReg = nbrWriteOnlyShadowReg;
	if( hRsp->nbrShadowReg != 0 )
	{
		hRsp->pShadowRegs = (uint32_t *) BKNI_Malloc( sizeof(uint32_t) * hRsp->nbrShadowReg );
		BDBG_ASSERT( hRsp->pShadowRegs );
		BKNI_Memset( hRsp->pShadowRegs, 0x00, sizeof(uint32_t) * hRsp->nbrShadowReg );
	}
	
	*phRsp = hRsp;

done:
	BDBG_LEAVE(BRSP_MByte_Open);
	return( retCode );
}

BERR_Code BRSP_MByte_Close(
	BRSP_MByte_Handle hRsp					/* [in] MBtye Register Handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ENTER(BRSP_MByte_Close);
	BDBG_ASSERT( hRsp );
	BDBG_ASSERT( hRsp->magicId == BRSP_MAGIC_ID );

	if( hRsp->pShadowRegs != NULL )
	{
		BKNI_Free( (void *) hRsp->pShadowRegs );
	}
	hRsp->magicId = 0x00;		/* clear it to catch inproper use */
	BKNI_Free( (void *) hRsp );

	BDBG_LEAVE(BRSP_MByte_Close);
	return( retCode );
}


void BRSP_MByte_Write32(
	BRSP_MByte_Handle hRsp,					/* [in] MBtye Register Handle */
	uint32_t opCode,						/* [in] MByte Operation Code to write to (see chip doc.) */
	uint32_t value							/* [in] Value to write to MByte Register */
	)
{
	BDBG_ENTER(BRSP_MByte_Write32);
	BDBG_ASSERT( hRsp );
	BDBG_ASSERT( hRsp->magicId == BRSP_MAGIC_ID );


	BREG_Write32( hRsp->hReg, hRsp->coreOffset, value );
	BREG_Write8( hRsp->hReg, (hRsp->coreOffset + PCI_ADDR8(4)), (uint8_t) opCode );
	if( hRsp->pShadowRegs != NULL )
	{
		if( hRsp->nbrShadowReg > opCode )
		{
			hRsp->pShadowRegs[opCode] = value;
		}
	}

	BDBG_LEAVE(BRSP_MByte_Write32);
	return;
}

uint32_t BRSP_MByte_Read32(
	BRSP_MByte_Handle hRsp,					/* [in] MByte Register Handle */
	uint32_t opCode							/* [in] MByte Operation Code to read from (see chip doc.) */
	)
{
	uint32_t value;


	BDBG_ENTER(BRSP_MByte_Read32);
	BDBG_ASSERT( hRsp );
	BDBG_ASSERT( hRsp->magicId == BRSP_MAGIC_ID );

	BREG_Write8( hRsp->hReg, (hRsp->coreOffset + PCI_ADDR8(4)), (uint8_t) opCode );
	value = BREG_Read32( hRsp->hReg, hRsp->coreOffset );

	BDBG_LEAVE(BRSP_MByte_Read32);
	return( value );
}

uint32_t BRSP_MByte_Read32WriteOnly(
	BRSP_MByte_Handle hRsp,					/* [in] MByte Register Handle */
	uint32_t opCode							/* [in] MByte Operation Code to read from (see chip doc.) */
	)
{
	uint32_t value;


	BDBG_ENTER(BRSP_MByte_Read32WriteOnly);
	BDBG_ASSERT( hRsp );
	BDBG_ASSERT( hRsp->magicId == BRSP_MAGIC_ID );

	value = 0;
	if( hRsp->pShadowRegs != NULL )
	{
		if( hRsp->nbrShadowReg > opCode )
		{
			value = hRsp->pShadowRegs[opCode];
		}
	}

	BDBG_LEAVE(BRSP_MByte_Read32WriteOnly);
	return( value );
}

