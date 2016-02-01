/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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

/* For bavc.h.  This is a bit of an ugly hack. */
#include "bchp_rdc.h"

#include "bstd.h"			/* standard types */
#include "bdbg.h"			/* Dbglib */
#include "bvbi.h"			/* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bkni.h"			/* For critical sections */
#include "bvbi_priv.h"		/* VBI internal data structures */
#include "bchp_vbi_enc_prim.h"	/* RDB info for VBI_ENC_PRIM registers */
#if (BVBI_NUM_VEC >= 2)
#include "bchp_vbi_enc_sec.h"	/* RDB info for VBI_ENC_SEC registers */
#endif
#if (BVBI_NUM_VEC >= 3)
#include "bchp_vbi_enc_tert.h"	/* RDB info for VBI_ENC_TERT registers */
#endif
#if (BVBI_NUM_PTVEC >= 1)
#if (BCHP_CHIP == 7400) || (BCHP_CHIP == 7405) || \
    (BCHP_CHIP == 7325) || (BCHP_CHIP == 7335) || (BCHP_CHIP==7336)
#include "bchp_vbi_enc_656_ancil.h"	
#else
#include "bchp_vbi_enc_656.h"	
#endif
#endif
#include "bchp_int_id_video_enc_intr2.h"

/* This is a kludge, I think. */
#define P_HAS_MODULAR_VEC (1)

/* Welcome to alias central */
#if (BCHP_CHIP == 7601) && (BCHP_VER >= BCHP_VER_B0)
	#define BCHP_VBI_ENC_PRIM_Interrupt_Control \
		BCHP_VBI_ENC_PRIM_INTERRUPT_CONTROL
	#define BCHP_VBI_ENC_PRIM_Interrupt_Control_INTER1_LINE_MASK \
		BCHP_VBI_ENC_PRIM_INTERRUPT_CONTROL_INTER1_LINE_MASK
	#define BCHP_VBI_ENC_PRIM_Interrupt_Control_INTER0_LINE_MASK \
		BCHP_VBI_ENC_PRIM_INTERRUPT_CONTROL_INTER0_LINE_MASK
	#define BCHP_VBI_ENC_PRIM_Interrupt_Control_INTER1_LINE_SHIFT \
		BCHP_VBI_ENC_PRIM_INTERRUPT_CONTROL_INTER1_LINE_SHIFT
	#define BCHP_VBI_ENC_PRIM_Interrupt_Control_INTER0_LINE_SHIFT \
		BCHP_VBI_ENC_PRIM_INTERRUPT_CONTROL_INTER0_LINE_SHIFT
#endif

BDBG_MODULE(BVBI);

/* Welcome to alias central */
#if (BCHP_CHIP == 7118)
	#define BCHP_TTE_PRIM_Reset_reset_SHIFT BCHP_TTE_PRIM_Reset_reset_SHIFT
#endif
#if (BCHP_CHIP == 7400) && (BCHP_VER == BCHP_VER_A0)
	#define BCHP_VBI_ENC_PRIM_Control_ENABLE_AMOL_MASK \
		BCHP_VBI_ENC_PRIM_Control_ENABLE_AMOLE_MASK
	#define BCHP_VBI_ENC_PRIM_Control_ENABLE_AMOL_BYTE_MASK \
		BCHP_VBI_ENC_PRIM_Control_ENABLE_AMOLE_BYTE_MASK
#endif
#if (BCHP_CHIP == 7400) && (BCHP_VER >= BCHP_VER_B0)
	#define BCHP_VBI_ENC_PRIM_Control_ENABLE_AMOL_MASK \
		BCHP_VBI_ENC_PRIM_Control_ENABLE_AMOLE_MASK
	#define BCHP_VBI_ENC_656_Ancil_Control_ENABLE_AMOL_MASK \
		BCHP_VBI_ENC_656_Ancil_Control_ENABLE_AMOLE_MASK
#endif
#if (BCHP_CHIP == 7403)
	#define BCHP_VBI_ENC_PRIM_Control_ENABLE_AMOL_MASK \
		BCHP_VBI_ENC_PRIM_Control_ENABLE_AMOLE_MASK
	#define BCHP_VBI_ENC_PRIM_Control_ENABLE_AMOL_BYTE_MASK \
		BCHP_VBI_ENC_PRIM_Control_ENABLE_AMOLE_BYTE_MASK
#endif
#if (BCHP_CHIP == 7405) || (BCHP_CHIP == 7325) || (BCHP_CHIP == 7335) || (BCHP_CHIP==7336) || \
	(BCHP_CHIP == 3548) || (BCHP_CHIP == 3556) || (BCHP_CHIP == 7601)
	#define BCHP_VBI_ENC_PRIM_Control_ENABLE_AMOL_MASK \
		BCHP_VBI_ENC_PRIM_Control_ENABLE_AMOLE_MASK
	#define BCHP_VBI_ENC_PRIM_Control_ENABLE_AMOL_BYTE_MASK \
		BCHP_VBI_ENC_PRIM_Control_ENABLE_AMOLE_BYTE_MASK
	#define BCHP_VBI_ENC_656_Ancil_Control_ENABLE_AMOL_MASK \
		BCHP_VBI_ENC_656_Ancil_Control_ENABLE_AMOLE_MASK
#endif

/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/
static void BVBI_P_VE_Enc_Init (BREG_Handle hReg, uint32_t ulCoreOffset);


/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/

BERR_Code BVBI_Encode_GetInterruptName(
    BAVC_VbiPath  eVbiPath,
    BAVC_Polarity eFieldPolarity,
	BINT_Id*      pInterruptName
)
{
	int index;
	BERR_Code eErr = BERR_SUCCESS;
	BINT_Id aEncodeInterruptName[2] = {0, 0};

	BDBG_ENTER(BVBI_Encode_GetInterruptName);

	/* Check for some obvious errors */
	if(!pInterruptName)
	{
		BDBG_ERR(("Invalid parameter"));
		eErr = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BVBI_Encode_GetInterruptName_Done;
	}
	switch (eFieldPolarity)
	{
		case BAVC_Polarity_eFrame:
		case BAVC_Polarity_eTopField:
		case BAVC_Polarity_eBotField:
			break;
		default:
			BDBG_ERR(("Invalid parameter"));
			eErr = BERR_TRACE(BERR_INVALID_PARAMETER);
			goto BVBI_Encode_GetInterruptName_Done;
			break;
	}
	/* Interrupts to use according to VEC path */
	switch (eVbiPath)
	{
	case BAVC_VbiPath_eVec0:
#if   (BCHP_CHIP == 7420) || (BCHP_CHIP == 7340) || \
	  (BCHP_CHIP == 7342) || (BCHP_CHIP == 7550) || \
	  (BCHP_CHIP == 7125) || (BCHP_CHIP == 7408) || \
	  (BCHP_CHIP == 7422) || (BCHP_CHIP == 7425) || \
	  (BCHP_CHIP == 7468) || (BCHP_CHIP == 7358) || (BCHP_CHIP == 7552) || \
	  (BCHP_CHIP == 7344) || (BCHP_CHIP == 7346) || \
	  (BCHP_CHIP == 7231) || (BCHP_CHIP == 7552) || \
	  (BCHP_CHIP == 73465)
		aEncodeInterruptName[0] = BCHP_INT_ID_VBI_0_0_INTR;
		aEncodeInterruptName[1] = BCHP_INT_ID_VBI_0_1_INTR;
#else
		aEncodeInterruptName[0] = BCHP_INT_ID_PRIM_VBI_0_INTR;
		aEncodeInterruptName[1] = BCHP_INT_ID_PRIM_VBI_1_INTR;
#endif
		break;
#if (BVBI_NUM_VEC >= 2) /** { **/
	case BAVC_VbiPath_eVec1:
#if (BCHP_CHIP == 7420) || (BCHP_CHIP == 7340)  || \
	(BCHP_CHIP == 7342) || (BCHP_CHIP == 7550)  || \
	(BCHP_CHIP == 7125) || (BCHP_CHIP == 7408)  || \
	(BCHP_CHIP == 7422) || (BCHP_CHIP == 7425)  || \
	(BCHP_CHIP == 7344) || (BCHP_CHIP == 7346)  || \
	(BCHP_CHIP == 7231) || (BCHP_CHIP == 73465) || \
	(BCHP_CHIP == 7468) || (BCHP_CHIP == 7358)  || \
	(BCHP_CHIP == 7552)
		aEncodeInterruptName[0] = BCHP_INT_ID_VBI_1_0_INTR;
		aEncodeInterruptName[1] = BCHP_INT_ID_VBI_1_1_INTR;
#else
		aEncodeInterruptName[0] = BCHP_INT_ID_SEC_VBI_0_INTR;
		aEncodeInterruptName[1] = BCHP_INT_ID_SEC_VBI_1_INTR;
#endif
		break;
#endif /** } (BVBI_NUM_VEC >= 2) **/
#if (BVBI_NUM_VEC >= 3) /** { **/
	case BAVC_VbiPath_eVec2:
#if (BCHP_CHIP == 7420) || (BCHP_CHIP == 7340) || \
	(BCHP_CHIP == 7342) || (BCHP_CHIP == 7550) || \
	(BCHP_CHIP == 7125) || (BCHP_CHIP == 7408) || \
	(BCHP_CHIP == 7468)
		aEncodeInterruptName[0] = BCHP_INT_ID_VBI_2_0_INTR;
		aEncodeInterruptName[1] = BCHP_INT_ID_VBI_2_1_INTR;
#else
		aEncodeInterruptName[0] = BCHP_INT_ID_TERT_VBI_0_INTR;
		aEncodeInterruptName[1] = BCHP_INT_ID_TERT_VBI_1_INTR;
#endif
		break;
#endif /** } (BVBI_NUM_VEC >= 3) **/
#if (BVBI_NUM_PTVEC > 0) /** { **/
	case BAVC_VbiPath_eBypass0:
#ifdef P_HAS_MODULAR_VEC
#if   (BCHP_CHIP == 7422) || (BCHP_CHIP == 7425)
		aEncodeInterruptName[0] = BCHP_INT_ID_ANCIL_VBI_0_0_INTR;
		aEncodeInterruptName[1] = BCHP_INT_ID_ANCIL_VBI_1_0_INTR;
#elif (BCHP_CHIP == 7344) || (BCHP_CHIP == 7346) || \
	  (BCHP_CHIP == 7231) || (BCHP_CHIP == 73465)
	    /* Weird */
		aEncodeInterruptName[0] = BCHP_INT_ID_ANCIL_VBI_0_0_INTR;
		aEncodeInterruptName[1] = BCHP_INT_ID_ANCIL_VBI_0_1_INTR;
#else
		aEncodeInterruptName[0] = BCHP_INT_ID_ANCIL_VBI_0_INTR;
		aEncodeInterruptName[1] = BCHP_INT_ID_ANCIL_VBI_1_INTR;
#endif
#else
		aEncodeInterruptName[0] = BCHP_INT_ID_BYPASS_VBI_0_INTR;
		aEncodeInterruptName[1] = BCHP_INT_ID_BYPASS_VBI_1_INTR;
#endif
		break;
#endif /** } (BVBI_NUM_PTVEC > 0) **/
	default:
		aEncodeInterruptName[0] = 0;
		aEncodeInterruptName[1] = 0;
		BDBG_ERR (("ERROR: failed to find VEC/VBI interrupts. %s: %d",
			__FILE__, __LINE__));
		eErr = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BVBI_Encode_GetInterruptName_Done;
		break;
	}

	/* Finally, apply field polarity */
	index = (eFieldPolarity == BAVC_Polarity_eBotField ? 1 : 0);
	*pInterruptName = aEncodeInterruptName[index];

BVBI_Encode_GetInterruptName_Done:
	BDBG_LEAVE(BVBI_Encode_GetInterruptName);
	return eErr;
}


/***************************************************************************
* Implementation of supporting VBI_ENC functions that are not in API
***************************************************************************/


BERR_Code BVBI_P_VE_Init( BVBI_P_Handle *pVbi )
{
	uint32_t ulCoreOffset;

	BDBG_ENTER(BVBI_P_VE_Init);

	ulCoreOffset = 0x0;
	BVBI_P_VE_Enc_Init (pVbi->hReg, ulCoreOffset);
#if (BVBI_NUM_VEC >= 2)
	ulCoreOffset = BCHP_VBI_ENC_SEC_RevID - BCHP_VBI_ENC_PRIM_RevID;
	BVBI_P_VE_Enc_Init (pVbi->hReg, ulCoreOffset);
#endif
#if (BVBI_NUM_VEC >= 3)
	ulCoreOffset = BCHP_VBI_ENC_TERT_RevID - BCHP_VBI_ENC_PRIM_RevID;
	BVBI_P_VE_Enc_Init (pVbi->hReg, ulCoreOffset);
#endif
#if (BVBI_NUM_PTVEC >= 1)
	ulCoreOffset = BCHP_VBI_ENC_656_Ancil_RevID - BCHP_VBI_ENC_PRIM_RevID;
	BVBI_P_VE_Enc_Init (pVbi->hReg, ulCoreOffset);
#endif

	BDBG_LEAVE(BVBI_P_VE_Init);
	return BERR_SUCCESS;
}


BERR_Code BVBI_P_VE_Enc_Program (
	BREG_Handle hReg,
	bool is656,
	uint8_t hwCoreIndex,
	uint32_t ulActive_Standards,
	uint32_t ulActive_656_Standards,
	BFMT_VideoFmt eVideoFormat)
{
	uint32_t ulReg;
	uint32_t topLine;
	uint32_t botLine;
	uint32_t vbiBits;
	int      rep;
	uint32_t ulScratchReg =        0x0;
	uint32_t ulOffset = 0xFFFFFFFF;

	BDBG_ENTER(BVBI_P_VE_Enc_Program);

	/* Figure out which encoder core to use */
	switch (hwCoreIndex)
	{
	case 0:
		if (is656)
		{
#if (BVBI_NUM_PTVEC >= 1)
			ulOffset = BCHP_VBI_ENC_656_Ancil_RevID - BCHP_VBI_ENC_PRIM_RevID;
			ulScratchReg = BAVC_VBI_ENC_BP_CTRL_SCRATCH;
#endif
		}
		else
		{
			ulOffset = 0;
			ulScratchReg = BAVC_VBI_ENC_0_CTRL_SCRATCH;
		}
		break;
#if (BVBI_NUM_VEC >= 2)
	case 1:
		ulOffset = BCHP_VBI_ENC_SEC_RevID - BCHP_VBI_ENC_PRIM_RevID;
		ulScratchReg = BAVC_VBI_ENC_1_CTRL_SCRATCH;
		break;
#endif
#if (BVBI_NUM_VEC >= 3)
	case 2:
		ulOffset = BCHP_VBI_ENC_TERT_RevID - BCHP_VBI_ENC_PRIM_RevID;
		ulScratchReg = BAVC_VBI_ENC_2_CTRL_SCRATCH;
		break;
#endif
	default:
		break;
	}
	if (ulOffset == 0xFFFFFFFF)
	{
		/* This should never happen!  This parameter was checked by
		   BVBI_Encode_Create() */
		BDBG_LEAVE(BVBI_P_VE_Enc_Program);
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Prepare to program the interrupt control register */
	ulReg =
		BREG_Read32 ( hReg,  BCHP_VBI_ENC_PRIM_Interrupt_Control + ulOffset );

	/* Select video format */
	switch (eVideoFormat)
	{
    case BFMT_VideoFmt_eNTSC:
    case BFMT_VideoFmt_eNTSC_J:
	case BFMT_VideoFmt_e720x482_NTSC:
	case BFMT_VideoFmt_e720x482_NTSC_J:
    case BFMT_VideoFmt_ePAL_M:
        /* NTSC specific settings */
		topLine = 37;
		botLine = 300;
		break;

	case BFMT_VideoFmt_e1080i:
	case BFMT_VideoFmt_e1080i_50Hz:
		topLine = 21;
		botLine = 584;
		break;

	case BFMT_VideoFmt_e720p:
	case BFMT_VideoFmt_e720p_50Hz:
		topLine = 26;
		botLine = 0;
		break;

	case BFMT_VideoFmt_e480p:
	case BFMT_VideoFmt_e720x483p:
		topLine = 46;
		botLine = 0;
		break;

    case BFMT_VideoFmt_ePAL_B:
    case BFMT_VideoFmt_ePAL_B1:
    case BFMT_VideoFmt_ePAL_D:
    case BFMT_VideoFmt_ePAL_D1:
    case BFMT_VideoFmt_ePAL_G:
    case BFMT_VideoFmt_ePAL_H:
    case BFMT_VideoFmt_ePAL_K:
    case BFMT_VideoFmt_ePAL_I:
    case BFMT_VideoFmt_ePAL_N:
    case BFMT_VideoFmt_ePAL_NC:
    case BFMT_VideoFmt_eSECAM_L:
    case BFMT_VideoFmt_eSECAM_B:
    case BFMT_VideoFmt_eSECAM_G:
    case BFMT_VideoFmt_eSECAM_D:
    case BFMT_VideoFmt_eSECAM_K:
    case BFMT_VideoFmt_eSECAM_H:
		topLine = 47;
		botLine = 360;
		break;

    case BFMT_VideoFmt_e576p_50Hz:
		topLine = 23;
		botLine = 0;
		break;

	default:
		BDBG_LEAVE(BVBI_P_VE_Enc_Program);
		BDBG_ERR(("This video format not supported yet"));
		return BERR_TRACE (BERR_NOT_SUPPORTED);
		break;
	}

	/* Finish programming the interrupt control register */
	ulReg &= ~(
		 BCHP_MASK      (VBI_ENC_PRIM_Interrupt_Control, INTER1_LINE         ) |
		 BCHP_MASK      (VBI_ENC_PRIM_Interrupt_Control, INTER0_LINE         )
	);
	ulReg |= (
		 BCHP_FIELD_DATA(VBI_ENC_PRIM_Interrupt_Control, INTER1_LINE, botLine) |
		 BCHP_FIELD_DATA(VBI_ENC_PRIM_Interrupt_Control, INTER0_LINE, topLine)
	);
	BREG_Write32 (hReg, BCHP_VBI_ENC_PRIM_Interrupt_Control + ulOffset, ulReg);

	/* Determine where VBI goes, analog and/or ITU-R 656 */
	vbiBits = 0x0;
#if (BVBI_NUM_PTVEC >= 1) /** { **/
	if (is656)
	{
		/* only enable these bits for the bypass path
		   (VBI_ENC_656_Ancil_Control differs from PRIM_Control) */
		if (ulActive_656_Standards & BVBI_P_SELECT_CC)
			vbiBits |= BCHP_VBI_ENC_656_Ancil_Control_ENABLE_CC_MASK;
		if (ulActive_656_Standards & BVBI_P_SELECT_MCC)
			vbiBits |= BCHP_VBI_ENC_656_Ancil_Control_ENABLE_CC_MASK;
		if (ulActive_656_Standards & BVBI_P_SELECT_TT)
			vbiBits |= BCHP_VBI_ENC_656_Ancil_Control_ENABLE_TTX_MASK;
		if (ulActive_656_Standards & BVBI_P_SELECT_WSS)
			vbiBits |= BCHP_VBI_ENC_656_Ancil_Control_ENABLE_WSS_MASK;
		if (ulActive_656_Standards & BVBI_P_SELECT_VPS)
			vbiBits |= BCHP_VBI_ENC_656_Ancil_Control_ENABLE_WSS_MASK;
#if (BVBI_NUM_GSE_656 > 0)
		if (ulActive_656_Standards & BVBI_P_SELECT_GS)
			vbiBits |= BCHP_VBI_ENC_656_Ancil_Control_ENABLE_GSE_MASK;
#endif
#if (BVBI_NUM_AMOLE_656 > 0)
		if (ulActive_656_Standards & BVBI_P_SELECT_AMOL)
			vbiBits |= BCHP_VBI_ENC_656_Ancil_Control_ENABLE_AMOL_MASK;
#endif
#if (BVBI_NUM_SCTEE_656 > 0)
		if (ulActive_656_Standards & BVBI_P_SELECT_SCTE)
			vbiBits |= BCHP_VBI_ENC_656_Ancil_Control_ENABLE_SCTE_MASK;
#endif
	}
	else
#endif /** } **/
	{
		/* only enable these bits for the non-bypass path
		   (VBI_ENC_656_Ancil_Control differs from PRIM_Control) */
		if (ulActive_Standards & BVBI_P_SELECT_CC)
			vbiBits |= BCHP_VBI_ENC_PRIM_Control_ENABLE_CC_MASK;
		if (ulActive_Standards & BVBI_P_SELECT_MCC)
			vbiBits |= BCHP_VBI_ENC_PRIM_Control_ENABLE_CC_MASK;
		if (ulActive_Standards & BVBI_P_SELECT_CGMSA)
			vbiBits |= BCHP_VBI_ENC_PRIM_Control_ENABLE_CGMS_MASK;
		if (ulActive_Standards & BVBI_P_SELECT_CGMSB)
			vbiBits |= BCHP_VBI_ENC_PRIM_Control_ENABLE_CGMS_MASK;
		if (ulActive_Standards & BVBI_P_SELECT_TT)
			vbiBits |= BCHP_VBI_ENC_PRIM_Control_ENABLE_TTX_MASK;
		if (ulActive_Standards & BVBI_P_SELECT_WSS)
			vbiBits |= BCHP_VBI_ENC_PRIM_Control_ENABLE_WSS_MASK;
		if (ulActive_Standards & BVBI_P_SELECT_VPS)
			vbiBits |= BCHP_VBI_ENC_PRIM_Control_ENABLE_WSS_MASK;
#if (BVBI_NUM_GSE > 0) /** { **/
		if (ulActive_Standards & BVBI_P_SELECT_GS)
			vbiBits |= BCHP_VBI_ENC_PRIM_Control_ENABLE_GSE_MASK;
#endif /** } BVBI_NUM_GSE **/
#if (BVBI_NUM_AMOLE > 0) /** { **/
		if (ulActive_Standards & BVBI_P_SELECT_AMOL)
			vbiBits |= BCHP_VBI_ENC_PRIM_Control_ENABLE_AMOL_MASK;
#endif /** } BVBI_NUM_AMOLE **/
#if (BVBI_NUM_SCTEE > 0) /** { **/
		if (ulActive_Standards & BVBI_P_SELECT_SCTE)
			vbiBits |= BCHP_VBI_ENC_PRIM_Control_ENABLE_SCTE_MASK;
#endif /** } BVBI_NUM_SCTEE **/
	}
	
	/* This register is shared with VDC. We must enter a critical section
	   so that our read/modify/write operation does not get interrupted by
	   VDC */
	BKNI_EnterCriticalSection();
	ulReg = BREG_Read32 ( hReg,  ulScratchReg );
#if (BVBI_NUM_PTVEC >= 1)
	if (is656)
	{
		/* clear out all Ancil_Control fields except PASS_THROUGH_COUNT */
		ulReg &= BCHP_VBI_ENC_656_Ancil_Control_PASS_THROUGH_COUNT_MASK;
	}
	else
#endif
	{
		/* clear out all PRIM_Control fields except ENABLE_PASS_THROUGH */
		ulReg &= BCHP_VBI_ENC_PRIM_Control_ENABLE_PASS_THROUGH_MASK;
	}
	ulReg |= vbiBits;
	BREG_Write32 (hReg, ulScratchReg, ulReg);
	BKNI_LeaveCriticalSection();

	/* Now poll the real hardware register until it is updated */
	for (rep = 0 ; rep < BVBI_P_MAX_HW_TRIES ; ++rep)
	{
		ulReg =
			BREG_Read32 (hReg, BCHP_VBI_ENC_PRIM_Control + ulOffset);
		if ((ulReg & ~BCHP_VBI_ENC_PRIM_Control_ENABLE_PASS_THROUGH_MASK) ==
			vbiBits)
		{
			break;
		}
		BKNI_Sleep (BVBI_P_SLEEP_HW);
	}

	BDBG_LEAVE(BVBI_P_VE_Enc_Program);
	return BERR_SUCCESS;
}

/***************************************************************************
* Static (private) functions
***************************************************************************/

/***************************************************************************
 *
 */
static void BVBI_P_VE_Enc_Init (BREG_Handle hReg, uint32_t ulCoreOffset)
{
	uint32_t topLine;
	uint32_t botLine;
	uint32_t ulReg;

	BDBG_ENTER(BVBI_P_VE_Enc_Init);

	/* Cause interrupts to occur according to start of NTSC active video
	(default) */
	ulReg = BREG_Read32 ( hReg,
			BCHP_VBI_ENC_PRIM_Interrupt_Control + ulCoreOffset );
	topLine = 23;
	botLine = 286;
	ulReg &= ~(
		 BCHP_MASK      (VBI_ENC_PRIM_Interrupt_Control, INTER1_LINE         ) |
		 BCHP_MASK      (VBI_ENC_PRIM_Interrupt_Control, INTER0_LINE         )
	);
	ulReg |= (
		 BCHP_FIELD_DATA(VBI_ENC_PRIM_Interrupt_Control, INTER1_LINE, botLine) |
		 BCHP_FIELD_DATA(VBI_ENC_PRIM_Interrupt_Control, INTER0_LINE, topLine)
	);
    BREG_Write32 (hReg,
		BCHP_VBI_ENC_PRIM_Interrupt_Control + ulCoreOffset, ulReg);

	BDBG_LEAVE(BVBI_P_VE_Enc_Init);
}

/* End of file */
