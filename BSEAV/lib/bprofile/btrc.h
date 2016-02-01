/***************************************************************************
 *     Copyright (c) 2006-2009, Broadcom Corporation
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
 * Perfomance counter module
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef __BTRC_H__
#define __BTRC_H__

#include "bperf_counter.h"
#include "blst_slist.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define B_TRC_N_SAMPLES 4
#define B_TRC_N_CHANNELS 1

typedef struct BTRC_P_Sample {
	/* unsigned samples[B_TRC_N_SAMPLES]; */
	bperf_sample perf;
} BTRC_P_Sample;

#define BTRC_P_SAMPLE_INITIALIZER { /*{0,0,0,0},*/ BPERF_SAMPLE_INITIALIZER }

typedef struct BTRC_P_Stats {
	unsigned count;
	BTRC_P_Sample last;
	BTRC_P_Sample max;
	BTRC_P_Sample min;
	BTRC_P_Sample total; /* 31..0 bits of total */
	BTRC_P_Sample total_hi; /* 63..32 bits of total */
} BTRC_P_Stats;

#define BTRC_P_STATS_INITIALIZER {0, BTRC_P_SAMPLE_INITIALIZER, BTRC_P_SAMPLE_INITIALIZER, BTRC_P_SAMPLE_INITIALIZER, BTRC_P_SAMPLE_INITIALIZER, BTRC_P_SAMPLE_INITIALIZER}

typedef struct BTRC_Module {
	bool b_trc_enable;
    BLST_S_ENTRY(BTRC_Module) link;
	const char *name;
	BTRC_P_Stats stats[B_TRC_N_CHANNELS];
} BTRC_Module;

BLST_S_HEAD(BTRC_ModuleList, BTRC_Module);
typedef struct BTRC_ModuleList BTRC_ModuleList;

typedef struct BTRC_P_Entry {
	unsigned event;
	unsigned data;
	unsigned samples[B_TRC_N_SAMPLES];
	bperf_sample perf;
}BTRC_P_Entry;


typedef enum BTRC_P_State {
	BTRC_P_State_DISABLE,
	BTRC_P_State_ENABLE
}BTRC_P_State;

void BTRC_P_DoTrace_START(BTRC_Module *module);
void BTRC_P_DoTrace_STOP(BTRC_Module *module);
void BTRC_P_DoTrace_COUNT(BTRC_Module *module);
/*
#define BTRC_P_Encode_Event(inst, event) (((unsigned)(&(inst).b_trc_enable)&(~3))|(event))
*/

#define BTRC_P_Encode_Event(inst, event) (inst)

#define BTRC_P_Encode_Data(chn, us20, us16)  ((((us16)&0xFFFF)<<24) | (((us20)&0xFFFFF)<<4) | ((chn)&0x0F))

#define BTRC_P_MODULE_HANDLE(module) (b_trc_mod_##module)
#define BTRC_MODULE_HANDLE(module) (&BTRC_P_MODULE_HANDLE(module))

#define BTRC_P_TRACE(module, event, data) BTRC_P_DoTrace_##event(BTRC_MODULE_HANDLE(module))

#define BTRC_TRACE(module, event) BTRC_P_TRACE(module, event, 0)
#define BTRC_CHN_TRACE(module, chn, event) BTRC_P_TRACE(module, event, BTRC_P_Encode_Data(chn, 0, 0))
#define BTRC_TRACE_DATA20(module, event, data20) BTRC_P_TRACE(module, event, BTRC_P_Encode_Data(0, data20, 0))
#define BTRC_TRACE_DATA20_16(module, event, data20, data16) BTRC_P_TRACE(module, event, BTRC_P_Encode_Data(0, data20,data16))

#define BTRC_MODULE(module,enable) BTRC_Module BTRC_P_MODULE_HANDLE(module) = {((BTRC_P_State_##enable)!=BTRC_P_State_DISABLE) , BLST_S_INITIALIZER(unused), #module,\
    { /* BTRC_P_STATS_INITIALIZER, */ BTRC_P_STATS_INITIALIZER}}

#define BTRC_MODULE_DECLARE(module) extern BTRC_Module BTRC_P_MODULE_HANDLE(module)

void BTRC_Module_Reset(BTRC_Module *module);
void BTRC_Module_Enable(BTRC_Module *module, bool enable);
void BTRC_Module_Report(const BTRC_Module *modile);
void BTRC_Module_Register(BTRC_Module *module, BTRC_ModuleList *list);

void BTRC_List_Init(BTRC_ModuleList *list);
void BTRC_List_Report(BTRC_ModuleList *list);
																																													

#ifdef __cplusplus
}
#endif

#endif /* __BTRC_H__ */

