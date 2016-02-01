/***************************************************************************
*     Copyright (c) 2003, Broadcom Corporation
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

#ifndef _BKNI_EMU_H_
#define _BKNI_EMU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "windows.h"

typedef int (*BKNI_EMU_READ_HANDLER_T)( uint32_t address, void* pData, size_t size );
typedef int (*BKNI_EMU_WRITE_HANDLER_T)( uint32_t address, void* pData, size_t size );

typedef struct BKNI_EMU_Settings
{
	void						*bmemAddress;				
	size_t						bmemSize;
	BKNI_EMU_READ_HANDLER_T		bemuReadHandler;
	BKNI_EMU_WRITE_HANDLER_T	bemuWriteHandler;

}BKNI_EMU_Settings;

void BKNI_EMU_Init( BKNI_EMU_Settings Settings );
void BKNI_EMU_Uninit();

int BKNI_EMU_ReadMem( uint32_t mem_addr, void *data, size_t size );
int BKNI_EMU_WriteMem( uint32_t mem_addr, void *data, size_t size );
int BKNI_EMU_ReadReg( uint32_t reg_addr, uint32_t *data);
int BKNI_EMU_WriteReg( uint32_t reg_addr, uint32_t data );

int BKNI_EMU_MemoryHandler( LPEXCEPTION_POINTERS ExceptionPointers );

bool BKNI_EMU_P_PageMap( uintptr_t uipAddress, bool bWrite );
void BKNI_EMU_P_PageUnmapAll();

#ifdef __cplusplus
}
#endif

#endif