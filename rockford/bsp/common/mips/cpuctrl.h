/**********************************************************************
**
** 	Broadcom Corp. Confidential
** 	Copyright 2000 Broadcom Corp.  All Rights Reserved.
**
**
**	File:         cpuctrl.h
**	Description:  MIPS CPU  Interface 
**	Created:      
**
**
**	REVISION:
**
**		$Log: $
**
**
**********************************************************************/

void ExceptReport(uint32_t *GenRegs, uint32_t *Cp0Regs, uint32_t *Cp1Regs);
void InterruptHandler(void);

unsigned long CpuIdGet(void);
unsigned long CpuCountGet(void);
void CpuCountSet(unsigned long value);

unsigned long CpuCompareGet(void);
void CpuCompareSet(unsigned long value);

unsigned long CpuIntGet(void);
unsigned long CpuIntSet(unsigned long imask);

unsigned long CpuStatusGet(void);
void CpuStatusSet(unsigned long);

void CpuBevSet(unsigned long val);

void AddTBLEntry(unsigned int entry_hi, unsigned int entry_lo_0, unsigned int entry_lo_1);


