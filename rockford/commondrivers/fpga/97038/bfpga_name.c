/***************************************************************************
 *     Copyright (c) 2002-2007, Broadcom Corporation
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
#include "bstd.h"
#include "bfpga_name.h"

BDBG_MODULE(BFPGA_NAME);

static const char * BFPGA_TsSelectNames[] = {
#if BCHP_CHIP==7401 || BCHP_CHIP == 7403
	"MB 4500SDS", 	/* 0 */
	"MB 3517VSB", 	/* 1 */
	"RMX 0", 	  	/* 2 */
	"RMX 1", 	  	/* 3 */
	"Reserved 4", 	/* 4 */
	"1394", 		/* 5 */ 
	"Reserved 6", 	/* 6 */
	"Streamer 1", 	/* 7 */
	"Slot0 TS2", 	/* 8 */
	"Slot0 TS3", 	/* 9 */
	"POD", 			/* 10 */
	"Slot0 TS4", 	/* 11 */
	"Slot0 TS5", 	/* 12 */
	"Unknown 13", 	/* 13 */
	"Reserved 14",  /* 14 */
	"Disabled"		/* 15 */
#elif BCHP_CHIP==7400
	"MB 4500SDS", 	/* 0 */
	"MB 3517VSB", 	/* 1 */
	"RMX 0", 	  	/* 2 */
	"RMX 1", 	  	/* 3 */
	"Reserved 4", 	/* 4 */
	"1394", 		/* 5 */ 
	"Reserved 6", 	/* 6 */
	"Streamer 1", 	/* 7 */
	"Slot0 TS2", 	/* 8 */
	"Slot0 TS3", 	/* 9 */
	"POD", 			/* 10 */
	"Slot1 TS4", 	/* 11 */
	"Slot1 TS5", 	/* 12 */
	"Unknown 13", 	/* 13 */
	"Slot 2",       /* 14 */
	"Disabled"		/* 15 */
#else
	"3250 DS 1",
	"3250 OOB",
	"7038 HSX 1",
	"7038 HSX 2",
	"7041 TS 0",
	"1394",
	"Streamer 2",
	"Streamer 1",
	"3250 DS 2",
	"7041 TS 1",
	"POD",
	"VSB/SDS 1",
	"VSB/SDS 2",
	"Reserved 1",
    "MS-POD",
	"Disabled"
#endif
};

#if BCHP_CHIP == 7401 || BCHP_CHIP == 7400 || BCHP_CHIP == 7403
/* PKT4 is available on 97038V4 boards. We have no way to detect version of board. Customers can turn this
on if they wish. For other boards, default it on. */
#define B_HAS_PKT4 1
#endif

static const char * BFPGA_OutputSelectNames[] = {
	"Band 0",
	"Band 1",
	"Band 2",
	"Band 3",
	"1394",
	"Test",
	"POD",
	"AVC"
#if B_HAS_PKT4
	,"Band 4" /* 0x0B */
#endif
};

const char * BFPGA_GetTsSelectName( BFPGA_TsSelect tsSelect )
{
	if (tsSelect >= sizeof(BFPGA_TsSelectNames)/sizeof(BFPGA_TsSelectNames[0])) return "unknown";
	
	return BFPGA_TsSelectNames[tsSelect];
}

const char * BFPGA_GetOutputSelectName( BFPGA_OutputSelect outSelect )
{
	if (outSelect >= sizeof(BFPGA_OutputSelectNames)/sizeof(BFPGA_OutputSelectNames[0])) return "unknown";
	
	return BFPGA_OutputSelectNames[outSelect];
}

void BFPGA_DumpConfiguration( BFPGA_Handle hFpga )
{
#if BDBG_DEBUG_BUILD
	BFPGA_info fpgaInfo;
#endif
	unsigned i;
	BFPGA_TsSelect tsSelect;
	BFPGA_ClockPolarity inClock, outClock;
	bool softConfig;

	BDBG_ASSERT(!BFPGA_GetInfo( 
		hFpga,
		&fpgaInfo
		));

	BDBG_WRN(("FPGA Version: 0x%X, Board Cfg: 0x%X, Strap: 0x%X", fpgaInfo.fpga_ver, fpgaInfo.board_cfg, fpgaInfo.strap_pins ));

	for( i = 0; i < sizeof(BFPGA_OutputSelectNames)/sizeof(BFPGA_OutputSelectNames[0]); i++ )
	{
		BFPGA_GetTsOutput( hFpga, i, &tsSelect, &softConfig );
		BFPGA_GetClockPolarity( hFpga, i, &inClock, &outClock );
		
		BDBG_WRN(("%-12s (clk %s) ==> %-8s (clk %s) %s", 
			BFPGA_GetTsSelectName(tsSelect), inClock?"-":"+", 
			BFPGA_GetOutputSelectName(i), outClock?"-":"+",
			softConfig?"custom":"default"));
	}
	return;
}
 
