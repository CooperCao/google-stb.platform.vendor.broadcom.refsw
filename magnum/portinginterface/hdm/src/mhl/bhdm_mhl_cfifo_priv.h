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
#ifndef BHDM_MHL_CFIFO_PRIV_H__
#define BHDM_MHL_CFIFO_PRIV_H__

#include "bstd.h"
#include "bdbg.h"       /* Debug Support */
#include "bkni.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifdef BHDM_MHL_ENABLE_DEBUG_FIFO
extern unsigned long _DEBUG_FIFO_BASE_ADDR[];
#define DEBUG_FIFO_SIZE	2048
#define DEBUG_FIFO_BASE_ADDR	0xFFC03800
#define DEBUG_FIFO_END_ADDR	(DEBUG_FIFO_BASE_ADDR + DEBUG_FIFO_SIZE - 1)
#endif


/* Circular Fifo Structure to hold the pointers for the Command Q FIFOs which are circular FIFOs */
struct BHDM_P_Mhl_Circular_Fifo
{
	unsigned long ulBaseAddr;
	unsigned long ulEndAddr;
	unsigned long ulReadAddr;
	unsigned long ulWriteAddr;
};

struct BHDM_P_Mhl_Circular_Fifo_Info {
	uint32_t ulDataPresentInBytes;
	uint32_t ulFreeSpaceNumBytes;
};

struct BHDM_P_Mhl_Circular_Fifo;
struct BHDM_P_Mhl_Circular_Fifo_Info;


/* Helper macros */
#define READ_CMD_FROM_Q_ISR(psFifo, pusDest, ulSize) BHDM_P_Mhl_Fifo_ReadData_isr(psFifo, pusDest, ulSize)
#define WRITE_CMD_TO_Q(_ISRpsFifo, pusSource, ulSize) BHDM_P_Mhl_Fifo_WriteData_isr(psFifo, pusSource, ulSize)

#define IS_CMD_Q_EMPTY_ISR(psFifo) BHDM_P_Mhl_Fifo_IsEmpty_isr(psFifo)
#define IS_CMD_Q_FULL_ISR(psFifo) BHDM_P_Mhl_Fifo_IsFull_isr(psFifo)


void BHDM_P_Mhl_Fifo_Init
	( struct BHDM_P_Mhl_Circular_Fifo *psFifo,
	  unsigned long ulBaseAddr,
	  uint32_t ulSize );

void BHDM_P_Mhl_Fifo_Uninit
	( struct BHDM_P_Mhl_Circular_Fifo *psFifo );

void BHDM_P_Mhl_Fifo_GetInfo_isr
	( struct BHDM_P_Mhl_Circular_Fifo *psFifo,
	  struct BHDM_P_Mhl_Circular_Fifo_Info *psFifoInfo );

bool BHDM_P_Mhl_Fifo_IsEmpty_isr
	( struct BHDM_P_Mhl_Circular_Fifo *psFifo );

bool BHDM_P_Mhl_Fifo_IsFull_isr
	( struct BHDM_P_Mhl_Circular_Fifo *psFifo );

bool BHDM_P_Mhl_Fifo_WriteData_isr
	( struct BHDM_P_Mhl_Circular_Fifo *psFifo,
	  uint8_t *pusSource,
	  uint32_t ulSize );

bool BHDM_P_Mhl_Fifo_ReadData_isr
	( struct BHDM_P_Mhl_Circular_Fifo *psFifo,
	  uint8_t *pusDest,
	  uint32_t ulSize );

uint32_t BHDM_P_Mhl_Fifo_GetFreeSpace_isr
	( struct BHDM_P_Mhl_Circular_Fifo *psFifo );

#ifdef __cplusplus
}
#endif

#endif /* BHDM_MHL_CFIFO_PRIV_H__*/
