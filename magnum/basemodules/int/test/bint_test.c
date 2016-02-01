/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
#include "bkni.h"

#include "bint_plat.h"
#include "bint_7038.h"

#ifndef BINT_UNIT_TEST
#error BINT_UNIT_TEST must be defined before compiling Interrupt Interface Unit Test!
#endif

BDBG_MODULE(ii_test_module);

const BINT_P_L2Impl bint_test[] =
{
	/* Bit 00 */ 
	0x0 * 8, 
	{
		{ "", false },
	},
	/* Bit 01 */ 
	1 * 8,
	{
		/* Bit 00 */ {"AUD_DSP1_HI00",	true},
		/* Bit 01 */ {"AUD_DSP1_HI01",	true},
		/* Bit 02 */ {"AUD_DSP1_HI02",	true},
		/* Bit 03 */ {"AUD_DSP1_HI03",	true},
		/* Bit 04 */ {"",				true},
		/* Bit 05 */ {"",				true},
		/* Bit 06 */ {"AUD_DSP1_HI06",	true},
		/* Bit 07 */ {"",				true},
		/* Bit 08 */ {"AUD_DSP1_HI08",	true},
		/* Bit 09 */ {"AUD_DSP1_HI09",	true},
		/* Bit 10 */ {"AUD_DSP1_HI10",	true},
		/* Bit 11 */ {"AUD_DSP1_HI11",	true},
		/* Bit 12 */ {"AUD_DSP1_HI12",	true},
		/* Bit 13 */ {"AUD_DSP1_HI13",	true},
		/* Bit 14 */ {"AUD_DSP1_HI14",	true},
		/* Bit 15 */ {"AUD_DSP1_HI15",	true}
	},
	/* Bit 02 */ 
	2 * 8,
	{
		/* Bit 00 */ {"AUD_FMM_00",		true}
	},
	/* Bit 03 */ 
	3 * 8,
	{
		/* Bit 00 */ {"BVNF_RGR_ERR",	true},
		/* Bit 01 */ {"BVNF_SCL_0_ERR",	true},
		/* Bit 02 */ {"BVNF_SCL_1_ERR",	true},
		/* Bit 03 */ {"BVNF_SCL_2_ERR",	true}
	},
	/* Bit 04 */ 
	4 * 8, 
	{
		/* Bit 00 */ {"DVI_HOTPLUG",		true},
		/* Bit 01 */ {"DVI_ALMOST_EMPTY",	true},
		/* Bit 02 */ {"DVI_EMPTY_MINUS",	true},
		/* Bit 03 */ {"DVI_ALMOST_FULL",	true},
		/* Bit 04 */ {"DVI_FULL_MINUS",		true}
	},
	/* Bit 05 */ 
	5 * 8,
	{
		/* Bit 00 */ {"IFD0_COEF_W",		true},
		/* Bit 01 */ {"IFD0_COEF_R",		true},
		/* Bit 02 */ {"IFD0_FE_AGCLP_LKG",	true},
		/* Bit 03 */ {"IFD0_FE_AGCLP_LKR",	true},
		/* Bit 04 */ {"IFD0_FE_AGCLP_IND",	true},
		/* Bit 05 */ {"IFD0_FE_AGCLP_INI",	true},
		/* Bit 06 */ {"IFD0_FE_AGCLP_INR",	true},
		/* Bit 07 */ {"IFD0_FE_AGCLP_SM",	true},
		/* Bit 08 */ {"IFD0_FE_AGCLP_SAT",	true},
		/* Bit 09 */ {"IFD0_FE_PRCLP",		true},
		/* Bit 10 */ {"IFD0_FE_DCOLP",		true},
		/* Bit 11 */ {"IFD0_FE_FIFO_OF",	true},
		/* Bit 12 */ {"IFD0_FE_FIFO_UF",	true},
		/* Bit 13 */ {"IFD0_AP_CICAGC_IN",	true},
		/* Bit 14 */ {"IFD0_AP_CORAGC_IN",	true},
		/* Bit 15 */ {"IFD0_AP_TB0_SAT",	true},
		/* Bit 16 */ {"IFD0_AP_TB1_SAT",	true},
		/* Bit 17 */ {"IFD0_AP_LP0_SAT",	true},
		/* Bit 18 */ {"IFD0_AP_LP1_SAT",	true},
		/* Bit 19 */ {"IFD0_VP_QBND_SAT",	true},
		/* Bit 20 */ {"IFD0_VP_NQST_SAT",	true},
		/* Bit 21 */ {"IFD0_VP_GDLY_SAT",	true},
		/* Bit 22 */ {"IFD0_VP_TRAP_SAT",	true},
		/* Bit 23 */ {"IFD0_VP_HBND_SAT",	true},
		/* Bit 24 */ {"IFD0_VP_CLAMP_SAT",	true},
		/* Bit 25 */ {"IFD0_VP_AGC_SAT",	true},
		/* Bit 26 */ {"IFD0_DE_FIL0_SAT",	true},
		/* Bit 27 */ {"IFD0_DE_FIL1_SAT",	true},
		/* Bit 28 */ {"IFD0_DE_LK",			true},
		/* Bit 29 */ {"IFD0_DE_SM",			true},
		/* Bit 30 */ {"",					true},
		/* Bit 31 */ {"IFD0_GR_BRIDGE_ERROR", true}
	},
	/* Bit 06 */ 
	6 * 8,
	{
		{ "", false },
	},
	/* Bit 07 */ 
	7 * 8,
	{
		/* Bit 00 */ {"MTP0_WATCHDOG",		true},
		/* Bit 01 */ {"MTP0_API1",			true},
		/* Bit 02 */ {"MTP0_API2",			true},
		/* Bit 03 */ {"MTP0_RGR_ERROR",		true}
	},
	/* Bit 08 */ 
	8 * 8, 
	{
		/* Bit 00 */ {"MTP1_WATCHDOG",		true},
		/* Bit 01 */ {"MTP1_API1",			true},
		/* Bit 02 */ {"MTP1_API2",			true},
		/* Bit 03 */ {"MTP1_RGR_ERROR",		true}
	},
	/* Bit 09 */ 
	9 * 8,
	{
		/* Bit 00 */ {"RFM_AUDIO_FIFO_OF",	true},
		/* Bit 01 */ {"RFM_AUDIO_FIFO_UF",	true},
		/* Bit 02 */ {"RFM_VIDEO_FIFO_OF",	true},
		/* Bit 03 */ {"RFM_VIDEO_FIFO_UF",	true}
	},
	/* Bit 10 */ 
	10 * 8,
	{
		{ "", false },
	},
	11 * 8,
	{
		{ "", false },
	},
	12 * 8,
	{
		{ "", false },
	},
	13 * 8,
	{
		/* Bit 00 */ {"SUN_GISB_TIMEOUT",	true},
		/* Bit 01 */ {"SUN_GISB_DEBUG_ERR",	true},
		/* Bit 02 */ {"SUN_GISB_TEA",		true},
		/* Bit 03 */ {"SUN_RGR_ERROR",		true}
	},
	BINT_P_L2_END /* end of array */
};

#define II_TEST_L2_SIZE (14 * sizeof(uint32_t) * 2 )


#define IIT_CHECK_RC( a ) \
	do { \
		rc = a; \
		if( rc != BERR_SUCCESS ) { \
			BDBG_ERR(( "Return Code %d Error in %s: Line %i", (int)rc, __FILE__, __LINE__ )); \
			goto done; \
		} \
	} while(0)

typedef struct
{
	BINT_CallbackHandle hCallback;
	int actualIntCount;
	int expectedIntCount;
} IIT_P_Callback;

static void cbFunc( void * pParm, int value )
{
	((IIT_P_Callback *)pParm)->actualIntCount++;
	if( ((IIT_P_Callback *)pParm)->actualIntCount != ((IIT_P_Callback *)pParm)->expectedIntCount )
	{
		BDBG_ERR(( "InterruptInteface failure."));
		BDBG_ERR(( "Expected ints (%d), Actual ints (%d)!", ((IIT_P_Callback *)pParm)->expectedIntCount, ((IIT_P_Callback *)pParm)->actualIntCount ));
	}
}

void triggerAllInts( BINT_Handle hInt )
{
	int i;

	for( i = 0; i < 64; i++ )
	{
		BINT_Isr( hInt, i );
	}
}


int main( void )
{
	uint32_t * pL2IntMemory;
	BERR_Code rc;
	BINT_Handle hInt = NULL;
    BREG_Handle hRegister = NULL;
	IIT_P_Callback iitCallback[4] = {{NULL,0,0},{NULL,0,0},{NULL,0,0},{NULL,0,0}};
	BINT_Settings bintSettings;
	const BINT_Settings *p_bint7038Settings;

    BKNI_Init();
    BDBG_Init();

	pL2IntMemory = BKNI_Malloc( II_TEST_L2_SIZE ); 
	if( !pL2IntMemory )
	{
		BDBG_ERR(( "Unable to allocate memory for simulated L2 interrupts!" ));
		return -1;
	}

	BKNI_Memset( pL2IntMemory, 0, II_TEST_L2_SIZE );

	BREG_Open( &hRegister, pL2IntMemory, II_TEST_L2_SIZE, NULL );

	BDBG_ERR(( "Opening InterruptInteface and creating callbacks." ));

	/* Need to override default 7038 settings with internal test interrupt implementation */
	p_bint7038Settings = BINT_7038_GetSettings();
	memcpy( &bintSettings, p_bint7038Settings, sizeof(BINT_Settings) );
	bintSettings.pIntImpl = bint_test;	
	
	IIT_CHECK_RC( BINT_Open( &hInt, hRegister, &bintSettings ) );
	IIT_CHECK_RC( BINT_CreateCallback( &iitCallback[0].hCallback, hInt, "AUD_DSP1_HI00", cbFunc, &iitCallback[0], 1 ) );
	IIT_CHECK_RC( BINT_CreateCallback( &iitCallback[1].hCallback, hInt, "AUD_DSP1_HI00", cbFunc, &iitCallback[1], 2 ) );
	
	IIT_CHECK_RC( BINT_TriggerInterruptByHandle( iitCallback[0].hCallback ) );
	triggerAllInts( hInt );

	IIT_CHECK_RC( BINT_EnableCallback( iitCallback[0].hCallback ) );
	IIT_CHECK_RC( BINT_TriggerInterruptByHandle( iitCallback[1].hCallback ) );

	iitCallback[0].expectedIntCount = 1; iitCallback[3].expectedIntCount = 1;
	triggerAllInts( hInt );

	IIT_CHECK_RC( BINT_TriggerInterruptByHandle( hInt, "AUD_DSP1_HI00" ) );

	iitCallback[0].expectedIntCount = 2;
	triggerAllInts( hInt );

	IIT_CHECK_RC( BINT_EnableCallback( iitCallback[1].hCallback ) );

	IIT_CHECK_RC( BINT_TriggerInterruptByHandle( iitCallback[0].hCallback ) );

	iitCallback[0].expectedIntCount = 3; iitCallback[1].expectedIntCount = 1;
	triggerAllInts( hInt );

	IIT_CHECK_RC( BINT_TriggerInterruptByHandle( hInt, "AUD_DSP1_HI00" ) );

	iitCallback[0].expectedIntCount = 4; iitCallback[1].expectedIntCount = 2;
	triggerAllInts( hInt );

	IIT_CHECK_RC( BINT_DisableCallback( iitCallback[0].hCallback ) );

	IIT_CHECK_RC( BINT_TriggerInterruptByHandle( iitCallback[0].hCallback ) );

	iitCallback[0].expectedIntCount = 4; iitCallback[1].expectedIntCount = 3;
	triggerAllInts( hInt );

	IIT_CHECK_RC( BINT_TriggerInterruptByHandle( hInt, "AUD_DSP1_HI00" ) );

	iitCallback[0].expectedIntCount = 4; iitCallback[1].expectedIntCount = 4;
	triggerAllInts( hInt );

	IIT_CHECK_RC( BINT_DisableCallback( iitCallback[1].hCallback ) );

	IIT_CHECK_RC( BINT_TriggerInterruptByHandle( iitCallback[1].hCallback ) );
	triggerAllInts( hInt );

	IIT_CHECK_RC( BINT_TriggerInterruptByHandle( hInt, "AUD_DSP1_HI00" ) );
	triggerAllInts( hInt );

	BDBG_ERR(( "Destroying callbacks." ));
	IIT_CHECK_RC( BINT_DestroyCallback( iitCallback[0].hCallback ) );
	IIT_CHECK_RC( BINT_DestroyCallback( iitCallback[1].hCallback ) );

	IIT_CHECK_RC( BINT_Close( hInt ) );

done:
    BKNI_Uninit();
    BDBG_Uninit();
    return 0;
}
