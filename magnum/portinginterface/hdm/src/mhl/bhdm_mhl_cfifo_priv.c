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
#include "bhdm_mhl_cfifo_priv.h"
#include "bhdm_mhl_debug_priv.h"

BDBG_MODULE(BHDM_MHL_CFIFO);
BDBG_OBJECT_ID(BHDM_MHL_CFIFO);


#ifdef BHDM_MHL_ENABLE_DEBUG_FIFO
struct BHDM_P_Mhl_Circular_Fifo sDebugFifo;
#endif

void BHDM_P_Mhl_Fifo_Init
	( struct BHDM_P_Mhl_Circular_Fifo *psFifo,
	  unsigned long ulBaseAddr,
	  uint32_t ulSize )
{
	psFifo->ulBaseAddr = ulBaseAddr;
	psFifo->ulEndAddr = psFifo->ulBaseAddr + ulSize - 1;
	psFifo->ulReadAddr = psFifo->ulBaseAddr;
	psFifo->ulWriteAddr = psFifo->ulBaseAddr;

	BKNI_Memset((void *)psFifo->ulBaseAddr, 0, ulSize);
}

void BHDM_P_Mhl_Fifo_Uninit
	( struct BHDM_P_Mhl_Circular_Fifo *psFifo )
{
	psFifo->ulBaseAddr = 0;
	psFifo->ulEndAddr = 0;
	psFifo->ulReadAddr = 0;
	psFifo->ulWriteAddr = 0;
}


void BHDM_P_Mhl_Fifo_GetInfo_isr
	( struct BHDM_P_Mhl_Circular_Fifo *psFifo,
	  struct BHDM_P_Mhl_Circular_Fifo_Info *psBfrInfo )
{
	unsigned long ulReadAddr;
	unsigned long ulWriteAddr;
	unsigned long ulBaseAddr;
	unsigned long ulEndAddr;

	uint32_t ulSize;
	uint32_t ulDataNumBytes = 0;

	/* Get the Circular Buffer pointers from the physical address*/
	ulReadAddr	=	psFifo->ulReadAddr;
	ulWriteAddr	=	psFifo->ulWriteAddr;
	ulBaseAddr	=	psFifo->ulBaseAddr;
	ulEndAddr	=	psFifo->ulEndAddr;

	ulSize  = ulEndAddr - ulBaseAddr + 1;
	if( ulReadAddr <= ulWriteAddr )
	{
		ulDataNumBytes = (ulWriteAddr - ulReadAddr) ;
	}
	else
	{
		ulDataNumBytes =	ulSize + ulWriteAddr - ulReadAddr;
	}


	psBfrInfo->ulDataPresentInBytes	= ulDataNumBytes;

	/* sizeof(uint32_t) is added to maintain
	one empty space b/wn Rd and Wr pointer; else
	Wr and Rd will get to same point. It will be difficult
	to find whether full or empty in case of normal buffer */
	psBfrInfo->ulFreeSpaceNumBytes	= ulSize - ulDataNumBytes - sizeof(uint32_t);

	BDBG_MSG(("R[0x%lu], W[0x%lu], Base[0x%lu], End[0x%lu], size=%d, data=%d, free space = %d",
		ulReadAddr, ulWriteAddr, ulBaseAddr, ulEndAddr, ulSize, ulDataNumBytes, psBfrInfo->ulFreeSpaceNumBytes));

}

bool BHDM_P_Mhl_Fifo_IsEmpty_isr
	( struct BHDM_P_Mhl_Circular_Fifo *psFifo )
{
	struct BHDM_P_Mhl_Circular_Fifo_Info sFifoInfo;
	bool ret;
	BHDM_P_Mhl_Fifo_GetInfo_isr(psFifo, &sFifoInfo);
	ret = (sFifoInfo.ulDataPresentInBytes == 0)? true : false;
	return ret;
}

bool BHDM_P_Mhl_Fifo_IsFull_isr
	( struct BHDM_P_Mhl_Circular_Fifo *psFifo )
{
	struct BHDM_P_Mhl_Circular_Fifo_Info sFifoInfo;
	bool ret;
	BHDM_P_Mhl_Fifo_GetInfo_isr(psFifo, &sFifoInfo);
	ret = (sFifoInfo.ulFreeSpaceNumBytes == 0)? true : false;
	return ret;
}

bool BHDM_P_Mhl_Fifo_WriteData_isr
	( struct BHDM_P_Mhl_Circular_Fifo *psFifo,
	  uint8_t *pusSource,
	  uint32_t ulSize )
{
	struct BHDM_P_Mhl_Circular_Fifo_Info sFifoInfo;
	uint32_t    ulSizeToEnd;
	uint32_t    ulWriteSize = 0;

	unsigned long    ulWriteAddr;
	unsigned long    ulEndAddr;
	unsigned long    ulBaseAddr;
	unsigned long    ulDestinationAddr;

	BHDM_P_Mhl_Fifo_GetInfo_isr(psFifo, &sFifoInfo);

	if(sFifoInfo.ulFreeSpaceNumBytes < ulSize)
	{
		return true; /* Fifo is going to be full, avoid any more writes */
	}

	ulWriteAddr   = psFifo->ulWriteAddr;
	ulBaseAddr    = psFifo->ulBaseAddr;
	ulEndAddr    = psFifo->ulEndAddr;

	/* Writing to FIFO */
	ulSizeToEnd   = ulEndAddr - ulWriteAddr + 1;
	ulWriteSize   = (ulSizeToEnd > ulSize) ? ulSize : ulSizeToEnd;

	ulDestinationAddr = ulWriteAddr;
	ulSize    -= (uint32_t)ulWriteSize;

	BKNI_Memcpy((void *)ulDestinationAddr, (void *)pusSource, ulWriteSize);

	psFifo->ulWriteAddr = ulDestinationAddr + ulWriteSize;

	/* excess if ulSize is greater than ulSizeToEnd */
	if(ulSize)
	{
		pusSource = (uint8_t *)pusSource + (uint8_t)ulWriteSize;
		ulDestinationAddr = ulBaseAddr;

		BKNI_Memcpy((void *)ulDestinationAddr, (void *)pusSource, ulSize);

		psFifo->ulWriteAddr = ulDestinationAddr + ulSize;

	}

#if BHDM_MHL_ENABLE_DEBUG
	{
		char acStr[40] = "";
		char acTemp[10] = "";
		strcat(acStr, "WRITE: R:0x");
		sprintf(acTemp, "%X", psFifo->ulReadAddr);
		strcat(acStr, acTemp);
		strcat(acStr, ", W:0x");
		sprintf(acTemp, "%X ", psFifo->ulWriteAddr);
		strcat(acStr, acTemp);
		strcat(acStr, "\n");
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
	}
#else
	BDBG_MSG(("WRITE: R[0x%lu], W[0x%lu]", psFifo->ulReadAddr, psFifo->ulWriteAddr));
#endif

	return false;

}

bool BHDM_P_Mhl_Fifo_ReadData_isr
	( struct BHDM_P_Mhl_Circular_Fifo *psFifo,
	  uint8_t *pusDest,
	  uint32_t ulSize )
{
	struct BHDM_P_Mhl_Circular_Fifo_Info sFifoInfo;
	uint32_t    ulSizeToEnd;
	uint32_t    ulReadSize = 0;

	unsigned long    ulReadAddr;
	unsigned long    ulEndAddr;
	unsigned long    ulBaseAddr;
	unsigned long    ulSourceAddr;

	BHDM_P_Mhl_Fifo_GetInfo_isr(psFifo, &sFifoInfo);

	if(sFifoInfo.ulDataPresentInBytes < ulSize)
	{
		return true; /* Fifo is either already empty or does not have enough bytes to be read, avoid any reads and return error */
	}

	ulReadAddr    = psFifo->ulReadAddr;
	ulBaseAddr    = psFifo->ulBaseAddr;
	ulEndAddr    = psFifo->ulEndAddr;

	/* Reading From FIFO */
	ulSizeToEnd   = ulEndAddr - ulReadAddr + 1;
	ulReadSize   = (ulSizeToEnd > ulSize) ? ulSize : ulSizeToEnd;

	ulSourceAddr = ulReadAddr;
	ulSize    -= (uint32_t)ulReadSize;

	BKNI_Memcpy((void *)pusDest, (void *)ulSourceAddr, ulReadSize);
	psFifo->ulReadAddr = ulSourceAddr + ulReadSize;


	/* excess if ulSize is greater than ulSizeToEnd */
	if(ulSize)
	{
		pusDest = (uint8_t *)pusDest + (uint8_t)ulReadSize;
		ulSourceAddr = ulBaseAddr;

		BKNI_Memcpy((void *)pusDest, (void *)ulSourceAddr, ulSize);

		psFifo->ulReadAddr = ulSourceAddr + ulSize;

	}

#if BHDM_MHL_ENABLE_DEBUG
	{
		char acStr[40] = "";
		char acTemp[10] = "";
		strcat(acStr, "READ: R:0x");
		sprintf(acTemp, "%X", psFifo->ulReadAddr);
		strcat(acStr, acTemp);
		strcat(acStr, ", W:0x");
		sprintf(acTemp, "%X ", psFifo->ulWriteAddr);
		strcat(acStr, acTemp);
		strcat(acStr, "\n");
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
	}
#else
	BDBG_MSG(("READ: R[0x%lu], W[0x%lu]", psFifo->ulReadAddr, psFifo->ulWriteAddr));
#endif
	return false;

}

uint32_t BHDM_P_Mhl_Fifo_GetFreeSpace_isr
	( struct BHDM_P_Mhl_Circular_Fifo *psFifo )
{
	struct BHDM_P_Mhl_Circular_Fifo_Info sFifoInfo;
	BHDM_P_Mhl_Fifo_GetInfo_isr(psFifo, &sFifoInfo);

	return (sFifoInfo.ulFreeSpaceNumBytes);
}
