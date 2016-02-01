/***************************************************************************
*     Copyright (c) 2003-2014, Broadcom Corporation
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

#include "bhdm_mhl_mailbox_priv.h"

BDBG_MODULE(BHDM_MHL_MAILBOX);
BDBG_OBJECT_ID(HDMI_MHL_MAILBOX);

uint8_t BHDM_P_Mhl_Mailbox_Read
	( BREG_Handle hRegister,
	  uint32_t    ulOffset )
{
	uint8_t ucData;

	BKNI_EnterCriticalSection();
	ucData = BHDM_P_Mhl_Mailbox_Read_isr(hRegister, ulOffset);
	BKNI_LeaveCriticalSection();

	return (ucData);
}

void BHDM_P_Mhl_Mailbox_Write
	( BREG_Handle hRegister,
	  uint32_t    ulOffset,
	  uint8_t     ucValue )
{
	BKNI_EnterCriticalSection();
	BHDM_P_Mhl_Mailbox_Write_isr(hRegister, ulOffset, ucValue);
	BKNI_LeaveCriticalSection();

}

uint8_t BHDM_P_Mhl_Mailbox_Read_isr
	( BREG_Handle hRegister,
	  uint32_t    ulOffset )
{
	uint32_t ulData;
	uint16_t usBytePos;
	uint32_t ulBase;
	uint8_t ucData = 0;

	usBytePos = ulOffset%4; /* byte position in 32-bit word */
	ulBase = ulOffset - usBytePos; /* 32-bit aligned offset from MBOX base address */

	BDBG_ASSERT(ulOffset < BHDM_P_MHL_MAILBOX_SIZE);

	ulData = BREG_Read32(hRegister, BCHP_MPM_CPU_DATA_MEM_WORDi_ARRAY_BASE + ulBase);
	ucData = (ulData >> (usBytePos*8)) & 0xFF;
	return ucData;
}

void BHDM_P_Mhl_Mailbox_Write_isr
	( BREG_Handle hRegister,
	  uint32_t    ulOffset,
	  uint8_t     ucValue )
{
	uint32_t ulData, ulTemp;
	uint16_t usBytePos;
	uint32_t ulBase;
	uint32_t ulMask = 0xFF;

	usBytePos = ulOffset%4; /* byte position in 32-bit word */
	ulBase = (uint32_t)(ulOffset - usBytePos); /* 32-bit aligned offset from MBOX base address */
	ulTemp = (uint32_t)ucValue;

	/* host access to MBOX is 32-bits */
	BDBG_ASSERT(ulOffset < BHDM_P_MHL_MAILBOX_SIZE);

	ulData = BREG_Read32(hRegister, BCHP_MPM_CPU_DATA_MEM_WORDi_ARRAY_BASE + ulBase);
	ulData &= ~(ulMask << (usBytePos*8));
	ulData |= (ulTemp << (usBytePos*8));

	BREG_Write32(hRegister, BCHP_MPM_CPU_DATA_MEM_WORDi_ARRAY_BASE + ulBase, ulData);
}


uint8_t BHDM_P_Mhl_Mailbox_GetField
	( BREG_Handle hRegister,
	  uint32_t    ulOffset,
	  uint8_t     ucMask,
	  uint8_t     ucLsb )
{
	uint8_t ucData;

	BKNI_EnterCriticalSection();
	ucData = BHDM_P_Mhl_Mailbox_GetField_isr(hRegister, ulOffset, ucMask, ucLsb);
	BKNI_LeaveCriticalSection();

	return (ucData);
}

void BHDM_P_Mhl_Mailbox_SetField
	( BREG_Handle hRegister,
	  uint32_t    ulOffset,
	  uint8_t     ucMask,
	  uint8_t     ucLsb,
	  uint8_t     ucValue )
{
	BKNI_EnterCriticalSection();
	BHDM_P_Mhl_Mailbox_SetField_isr(hRegister, ulOffset, ucMask, ucLsb, ucValue);
	BKNI_LeaveCriticalSection();
}


/* Return a particular field in mailbox */
uint8_t BHDM_P_Mhl_Mailbox_GetField_isr
	( BREG_Handle hRegister,
	  uint32_t    ulOffset,
	  uint8_t     ucMask,
	  uint8_t     ucLsb )
{
	uint8_t ucData, ucField;

	ucData = BHDM_P_Mhl_Mailbox_Read_isr(hRegister, ulOffset);
	ucField = (ucData >> ucLsb) & ucMask;
	return ucField;
}

/* Set a particular field in mailbox */
void BHDM_P_Mhl_Mailbox_SetField_isr
	( BREG_Handle hRegister,
	  uint32_t    ulOffset,
	  uint8_t     ucMask,
	  uint8_t     ucLsb,
	  uint8_t     ucValue )
{
	uint8_t ucData;
	ucData = BHDM_P_Mhl_Mailbox_Read_isr(hRegister, ulOffset);
	ucData &= ~(ucMask << ucLsb);
	ucData |= ((ucValue & ucMask) << ucLsb);
	BHDM_P_Mhl_Mailbox_Write_isr(hRegister, ulOffset, ucData);
}

/* Get the mailbox addr to a particular offset */
uint32_t BHDM_P_Mhl_Mailbox_GetFieldAddr
	( BREG_Handle hRegister,
	  uint32_t    ulOffset )
{
	uint32_t ulAddr;

	BKNI_EnterCriticalSection();
	ulAddr = BHDM_P_Mhl_Mailbox_GetFieldAddr_isr(hRegister, ulOffset);
	BKNI_LeaveCriticalSection();

	return (ulAddr);
}

/* Get the mailbox addr to a particular offset */
uint32_t BHDM_P_Mhl_Mailbox_GetFieldAddr_isr
	( BREG_Handle hRegister,
	  uint32_t ulOffset )
{
	uint32_t ulAddr = 0;
	BSTD_UNUSED(hRegister);

	if(ulOffset < BHDM_P_MHL_MAILBOX_SIZE)
	{
		ulAddr = BCHP_MPM_CPU_DATA_MEM_WORDi_ARRAY_BASE + ulOffset;
	}
	else
	{
		BDBG_ERR(("Mailbox out-of-bounds read."));
		BDBG_ASSERT(ulAddr);
	}
	return ulAddr;
}

/* Return the EDID addr */
uint32_t BHDM_P_Mhl_Mailbox_GetEdidAddr
	( void )
{
	uint32_t ulAddr;

	BKNI_EnterCriticalSection();
	ulAddr = BHDM_P_Mhl_Mailbox_GetEdidAddr_isr();
	BKNI_LeaveCriticalSection();

	return ulAddr;
}

/* Return the EDID addr */
uint32_t BHDM_P_Mhl_Mailbox_GetEdidAddr_isr
	( void )
{
	return BCHP_MPM_CPU_DATA_MEM_WORDi_ARRAY_BASE + BHDM_P_Mhl_MailboxField_eEdid;
}


/*
 * Source/sink DCAP API,
 * Note there is nothing to stop you from overwriting sink DCAP.
 * Use get_field/set_field to interact with one DCAP byte at a time.
 */
void BHDM_P_Mhl_Mailbox_SetSrcDcap
	( BREG_Handle   hRegister,
	  uint8_t      *pucDcap )
{
	BKNI_EnterCriticalSection();
	BHDM_P_Mhl_Mailbox_SetSrcDcap_isr(hRegister, pucDcap);
	BKNI_LeaveCriticalSection();
}

/* Set the source DCAP in one go */
void BHDM_P_Mhl_Mailbox_SetSrcDcap_isr
	( BREG_Handle   hRegister,
	  uint8_t      *pucDcap )
{
	if(pucDcap)
	{
		uint8_t i;

		for (i=0; i<BHDM_P_MHL_CBUS_CAP_REG_SIZE; i++)
		{
			BHDM_P_Mhl_Mailbox_Write_isr(hRegister, BHDM_P_Mhl_MailboxField_eSrcDcapDevState + i, pucDcap[i]);
		}
	}
}


void BHDM_P_Mhl_Mailbox_GetSrcDcap
	( BREG_Handle   hRegister,
	  uint8_t      *pucDcap )
{
	BKNI_EnterCriticalSection();
	BHDM_P_Mhl_Mailbox_GetSrcDcap_isr(hRegister, pucDcap);
	BKNI_LeaveCriticalSection();
}

/* Get the source DCAP in one go */
void BHDM_P_Mhl_Mailbox_GetSrcDcap_isr
	( BREG_Handle   hRegister,
	  uint8_t      *pucDcap )
{
	if(pucDcap)
	{
		uint8_t i;

		for (i=0; i<BHDM_P_MHL_CBUS_CAP_REG_SIZE; i++)
		{
			pucDcap[i] = BHDM_P_Mhl_Mailbox_Read_isr(hRegister, BHDM_P_Mhl_MailboxField_eSrcDcapDevState + i);
		}
	}
}

void BHDM_P_Mhl_Mailbox_GetSinkDcap
	( BREG_Handle   hRegister,
	  uint8_t      *pucDcap )
{
	BKNI_EnterCriticalSection();
	BHDM_P_Mhl_Mailbox_GetSinkDcap_isr(hRegister, pucDcap);
	BKNI_LeaveCriticalSection();
}

/* Get the sink DCAP in one go */
void BHDM_P_Mhl_Mailbox_GetSinkDcap_isr
	( BREG_Handle   hRegister,
	  uint8_t      *pucDcap )
{
	if(pucDcap)
	{
		uint8_t i;

		for (i=0; i<BHDM_P_MHL_CBUS_CAP_REG_SIZE; i++)
		{
			pucDcap[i] = BHDM_P_Mhl_Mailbox_Read_isr(hRegister, BHDM_P_Mhl_MailboxField_eSinkDcapDevState + i);
		}
	}
}

/* Getting/setting scratchpad
   Arguments: buffer to copy to/copy from
              offset within the scratchpad registers where copy starts
	      how many bytes to copy, up to scratchpad size
   Returns zero if no error
*/
int BHDM_P_Mhl_Mailbox_GetScratchpad_isr
	( BREG_Handle  hRegister,
	  uint8_t     *pucScratchpad,
	  uint8_t      ucOffset,
	  uint8_t      ucSize )
{
	uint8_t i;

	if(ucOffset >= BHDM_P_MHL_MAILBOX_SCRATCHPAD_SIZE ||
	   ucOffset + ucSize > BHDM_P_MHL_MAILBOX_SCRATCHPAD_SIZE)
	{
		BDBG_ERR(("Invalid scratchpad offset/size: 0x%x/%d ", ucOffset, ucSize));
		return -1;
	}

	for (i=0; i<ucSize; i++)
	{
		pucScratchpad[i] = BHDM_P_Mhl_Mailbox_Read_isr(hRegister, BHDM_P_Mhl_MailboxField_eScratchpad + ucOffset + i);
	}
	return 0;
}

int BHDM_P_Mhl_Mailbox_SetScratchpad_isr
	( BREG_Handle  hRegister,
	  uint8_t     *pucScratchpad,
	  uint8_t      ucOffset,
	  uint8_t      ucSize )

{
	uint8_t i;

	if(ucOffset >= BHDM_P_MHL_MAILBOX_SCRATCHPAD_SIZE ||
	   ucOffset + ucSize > BHDM_P_MHL_MAILBOX_SCRATCHPAD_SIZE)
	{
		BDBG_ERR(("Invalid scratchpad offset/size: 0x%x/%d ", ucOffset, ucSize));
		return -1;
	}

	for (i=0; i<ucSize; i++)
	{
		BHDM_P_Mhl_Mailbox_Write_isr(hRegister, BHDM_P_Mhl_MailboxField_eScratchpad + ucOffset + i, pucScratchpad[i]);
	}
	return 0;
}

/* Process incoming SET_INT by setting a bit, if you want to clear
   a bit, call BHDM_P_Mhl_Mailbox_ClearInt_isr.
   returns zero if success (offset is valid)
   if the return code is < 0, the host should
   reply with ABORT.
 */
BERR_Code BHDM_P_Mhl_Mailbox_ProcessInt_isr
	( BREG_Handle  hRegister,
	  uint8_t      ucOffset,
	  uint8_t      ucVal )
{
	BERR_Code ret = BHDM_P_MHL_CBUS_SUCCESS;
	uint8_t ucData;

	switch(ucOffset)
	{
	case BHDM_P_Mhl_IntAddr_eRchangeIntAddr:
		ucData = BHDM_P_Mhl_Mailbox_Read_isr(hRegister, BHDM_P_Mhl_MailboxField_eSinkRchangeInt);
		ucData |= ucVal;
		BHDM_P_Mhl_Mailbox_Write_isr(hRegister, BHDM_P_Mhl_MailboxField_eSinkRchangeInt, ucData);

		/* Auto clear DCAP valid on DCAP_CHG interrupt */
		if(ucVal & BHDM_P_MHL_INT_RCHANGE_DCAP_CHG)
			BHDM_P_MHL_MAILBOX_CLR_FIELD_ISR(hRegister, DCAP_VALID);
		break;

	case BHDM_P_Mhl_IntAddr_eDchangeIntAddr:
		ucData = BHDM_P_Mhl_Mailbox_Read_isr(hRegister, BHDM_P_Mhl_MailboxField_eSinkDchangeInt);
		ucData |= ucVal;
		BHDM_P_Mhl_Mailbox_Write_isr(hRegister, BHDM_P_Mhl_MailboxField_eSinkDchangeInt, ucData);

		/* Auto clear EDID valid on EDID_CHG interrupt */
		if(ucVal & BHDM_P_MHL_INT_DCHANGE_EDID_CHG)
			BHDM_P_MHL_MAILBOX_CLR_FIELD_ISR(hRegister, EDID_VALID);
		break;

		/* Just ACK reserved addresses */
	case BHDM_P_Mhl_IntAddr_eRsvrInt1Addr:
	case BHDM_P_Mhl_IntAddr_eRsvrInt2Addr:
		break;

	default:
		ret = BHDM_P_MHL_CBUS_INVALID;
	}
	return ret;
}

/* Clear one of the bits in the sink INT register
   after the host has processed it
   val should only have 1 bit set

   Returns 0 if the offset is valid
*/
BERR_Code BHDM_P_Mhl_Mailbox_ClearInt_isr
	( BREG_Handle  hRegister,
	  uint8_t      ucOffset,
	  uint8_t      ucVal )
{
	BERR_Code ret = BHDM_P_MHL_CBUS_SUCCESS;
	uint8_t ucData;

	switch(ucOffset)
	{
	case BHDM_P_Mhl_IntAddr_eRchangeIntAddr:
		ucData = BHDM_P_Mhl_Mailbox_Read_isr(hRegister, BHDM_P_Mhl_MailboxField_eSinkRchangeInt);
		ucData &= ~ucVal;
		BHDM_P_Mhl_Mailbox_Write_isr(hRegister, BHDM_P_Mhl_MailboxField_eSinkRchangeInt, ucData);
		break;

	case BHDM_P_Mhl_IntAddr_eDchangeIntAddr:
		ucData = BHDM_P_Mhl_Mailbox_Read_isr(hRegister, BHDM_P_Mhl_MailboxField_eSinkDchangeInt);
		ucData &= ~ucVal;
		BHDM_P_Mhl_Mailbox_Write_isr(hRegister, BHDM_P_Mhl_MailboxField_eSinkDchangeInt, ucData);
		break;

		/* Nothing happens */
	case BHDM_P_Mhl_IntAddr_eRsvrInt1Addr:
	case BHDM_P_Mhl_IntAddr_eRsvrInt2Addr:
		break;

	default:
		ret = BHDM_P_MHL_CBUS_INVALID;
	}
	return ret;
}

/* Process incoming WRITE_STAT command. Note this does NOT update
   the source's STAT registers, which should be changed
   with the set_field function.
   Returns 0 if the offset is valid,
   if return < 0, the host should reply with ABORT
*/
BERR_Code BHDM_P_Mhl_Mailbox_ProcessStat_isr
	( BREG_Handle  hRegister,
	  uint8_t      ucOffset,
	  uint8_t      ucVal )
{
	BERR_Code ret = BHDM_P_MHL_CBUS_SUCCESS;

	switch(ucOffset)
	{

	case BHDM_P_Mhl_StatusAddr_eConnectedRdyAddr:
		BHDM_P_Mhl_Mailbox_Write_isr(hRegister, BHDM_P_Mhl_MailboxField_eSinkConnectedRdy, ucVal);
		break;

	case BHDM_P_Mhl_StatusAddr_eLinkModeAddr:
		BHDM_P_Mhl_Mailbox_Write_isr(hRegister, BHDM_P_Mhl_MailboxField_eSinkLinkMode, ucVal);
		break;

		/* Just ACK reserved addresses */
	case BHDM_P_Mhl_StatusAddr_eRsvrStat1Addr:
	case BHDM_P_Mhl_StatusAddr_eRsvrStat2Addr:
		break;

	default:
		ret = BHDM_P_MHL_CBUS_INVALID;
		break;
	}
	return ret;
}

#if BHDM_MHL_CTS
/* Scratchpad updated */
void BHDM_P_Mhl_Mailbox_ScratchpadUpdateDone_isr
	( BREG_Handle hRegister )
{
	BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, SCRATCHPAD_VALID, 1);
	BHDM_P_MHL_MAILBOX_CLR_FIELD_ISR(hRegister, SINK_DSCR_CHG);
}

/* Process DCAP change / EDID change
   if EDID and DCAP has been reread,
   this mark the VALID bit and clear the corresponding
   bit in the INT registers at the same time */
void BHDM_P_Mhl_Mailbox_DcapUpdateDone_isr
	( BREG_Handle hRegister )
{
	BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, DCAP_VALID, 1);
	BHDM_P_MHL_MAILBOX_CLR_FIELD_ISR(hRegister, SINK_DCAP_CHG);
}

void BHDM_P_Mhl_Mailbox_EdidUpdateDone_isr
	( BREG_Handle hRegister )
{
	BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, EDID_VALID, 1);
	BHDM_P_MHL_MAILBOX_CLR_FIELD_ISR(hRegister, SINK_EDID_CHG);
}
#endif

/* Initialise the mailbox from fresh start,
   do NOT call this if initialising from scratchpad.
   Argument is mailbox revision and source DCAP
 */
void BHDM_P_Mhl_Mailbox_Init
	( BREG_Handle  hRegister,
	  uint8_t      ucRev,
	  uint8_t     *pucDcap,
	  uint8_t      ucSrcLinkMode )
{

	BSTD_UNUSED(ucRev);
	BSTD_UNUSED(ucSrcLinkMode);
	BHDM_P_Mhl_Mailbox_SetSrcDcap(hRegister, pucDcap);
	BDBG_MSG(("MAILBOX ADDRESS: 0x%x", BCHP_MPM_CPU_DATA_MEM_WORDi_ARRAY_BASE));
}
