/***************************************************************************
*     Copyright (c) 2004-2011, Broadcom Corporation
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
*	This file contains the common APIs used internally by different 
*	modules of the Raptor Audio PI. 
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "brap.h"
#include "brap_priv.h"

BDBG_MODULE(rap_priv);

#define BRAP_P_DEC_PT_MODE_COUNT		(2) /* Decode mode and pass thru mode - 2 in count */

BRAP_OP_P_SpdifParams	sSpdifParams;
BRAP_OP_P_I2sParams		sI2sParams;
BRAP_OP_P_MaiParams		sMaiParams;
BRAP_OP_P_FlexParams	sFlexParams;
BRAP_OP_P_DacParams	sDacParams;

/* Static array sAlgoMemRequirementArray contains buffer size requirements for all the algorithms. 
 * Application selects the algorithms to be supported during BRAP_Open. While allocating these
 * buffers, PI finds out worst case size requirement for each of the buffer for selected algorithms
 * using this array and makes buffer allocation accordingly.
 */
 
/*
For 7401: 
Copied from 
http://twiki-01.broadcom.com/bin/viewfile/Raptor/WebHome?rev=1;filename=
7401MemoryUsage.xls
Dated: 20th July 06
Decoder specific memory estimates		
Algo        Code size	Tables	scratch 	interframe 	Total
                                space       buffer

MPEG1	    59392	    23432	8	        10916	    93748	
AC3 5.1	    28672	    7560	110	        31588	    67930	
AAC 5.1	    28672	    29612	123368	    14336	    195988	
AAC HE stereo	57344	44084	90600	    52180	    244208	
Dolby plus 5.1	45056	20620	79060	    31562	    176298	
*/

/*
For 7411: 
Copied from 
 http://twiki-01.broadcom.com/bin/viewfile/Bseavsw/ReferencePhases97398?rev=1.
11;filename=7411MemoryUsage.xls
Dated: 20th July 06
Decoder specific memory estimates		
Algo        Code size	Tables	scratch 	interframe 	Total
                                space       buffer

MPEG1	    86016	    23432	8	        10916	120372
AC3 5.1	    28672	    7560	110	        3376	39718
AAC 5.1	    28672	    29612	123368	    14336	195988
AAC HE stereo	57344	44084	90600	    52180	244208
Dolby plus 5.1	45056	20620	79060	    31512	176248
DTS core	19770	    86092	67668	    13488	187018

*/


#if (BRAP_DSP_P_7401_NEWMIT==1)
static const 
BRAP_P_Dec_AlgoMemRequirement sAlgoMemRequirementArray[BRAP_MAX_AUDIO_TYPES] = {
		{	/* MPEG */
			{
				BRAP_P_ALIGN((59392 + 24250),2), /* Code + Table */
				BRAP_P_ALIGN(11172,2),  /* Interframe buf */
				BRAP_P_ALIGN(0,2),      /* Interstage I/P */
				BRAP_P_ALIGN(0,2),      /* Interstate IF I/P */
				BRAP_P_ALIGN(147456,2),      /* Interstage O/P */
				BRAP_P_ALIGN(0,2),      /* Interstate IF O/P */
				BRAP_P_ALIGN(0,2),      /* DEC scratch buf */
				BRAP_P_ALIGN(84,2),     /* DEC config */
			},
			BRAP_P_ALIGN(4096,2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)       /* Ring Buffer */
		},	
#if ( BCHP_CHIP == 7400 )
		{	/* AAC */
			{
				BRAP_P_ALIGN((149504 +71680),2), /* Code + Table */
				BRAP_P_ALIGN(187392,2),  /* Interframe buf */
				BRAP_P_ALIGN(73728,2),  /* Interstage I/P */
				BRAP_P_ALIGN(41984 ,2),   /* Interstate IF I/P */
				BRAP_P_ALIGN(147728,2),  /* Interstage O/P */
				BRAP_P_ALIGN(41984 ,2),   /* Interstate IF O/P */
				BRAP_P_ALIGN(81920,2),  /* DEC scratch buf */
				BRAP_P_ALIGN(1024,2),    /* DEC config */
			},
			BRAP_P_ALIGN(4096,2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)       /* Ring Buffer */
		},	
		{	/* AAC_SBR */
			{
				BRAP_P_ALIGN((149504 +71680),2), /* Code + Table */
				BRAP_P_ALIGN(187392,2),  /* Interframe buf */
				BRAP_P_ALIGN(73728,2),  /* Interstage I/P */
				BRAP_P_ALIGN(41984 ,2),   /* Interstate IF I/P */
				BRAP_P_ALIGN(147728,2),  /* Interstage O/P */
				BRAP_P_ALIGN(41984 ,2),   /* Interstate IF O/P */
				BRAP_P_ALIGN(81920,2),  /* DEC scratch buf */
				BRAP_P_ALIGN(1024,2),    /* DEC config */
			},
			BRAP_P_ALIGN(4096,2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)       /* Ring Buffer */
		},	
#else
		{	/* AAC */
			{
				BRAP_P_ALIGN((155000 +76000),2), /* Code + Table */
				BRAP_P_ALIGN(333000,2),  /* Interframe buf */
				BRAP_P_ALIGN(56320,2),  /* Interstage I/P */
				BRAP_P_ALIGN(3000 ,2),   /* Interstate IF I/P */
				BRAP_P_ALIGN(131072,2),  /* Interstage O/P */
				BRAP_P_ALIGN(3000 ,2),   /* Interstate IF O/P */
				BRAP_P_ALIGN(74752,2),  /* DEC scratch buf */
				BRAP_P_ALIGN(1024,2),    /* DEC config */
			},
			BRAP_P_ALIGN(4096,2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)       /* Ring Buffer */
		},	
		{	/* AAC_SBR */
			{
				BRAP_P_ALIGN((155000 + 76000),2), /* Code + Table */
				BRAP_P_ALIGN(333000,2),  /* Interframe buf */
				BRAP_P_ALIGN(56320,2),  /* Interstage I/P */
				BRAP_P_ALIGN(3000 ,2),   /* Interstate IF I/P */
				BRAP_P_ALIGN(131072,2),  /* Interstage O/P */
				BRAP_P_ALIGN(3000 ,2),   /* Interstate IF O/P */
				BRAP_P_ALIGN(74752,2),  /* DEC scratch buf */
				BRAP_P_ALIGN(1024,2),    /* DEC config */
			},
			BRAP_P_ALIGN(4096,2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)       /* Ring Buffer */
		},	
#endif		
#if (BRAP_7401_FAMILY == 1)		
		{	/* AC3 */
			{
				BRAP_P_ALIGN((28672 + 7560),2), /* Code + Table */ 
				BRAP_P_ALIGN(31588,2),  /* Interframe buf */
				BRAP_P_ALIGN(0,2),      /* Interstage I/P */ 
				BRAP_P_ALIGN(0,2),      /* Interstate IF I/P */ 
				BRAP_P_ALIGN(49152,2),      /* Interstage O/P */ 
				BRAP_P_ALIGN(0,2),      /* Interstate IF O/P */ 
				BRAP_P_ALIGN(0,2),      /* DEC scratch buf */ 
				BRAP_P_ALIGN(130,2),    /* DEC config */
			},
			BRAP_P_ALIGN(4200,2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)       /* Ring Buffer */
		},	
#else
		/* We use DDP executables for AC3 decoding */
		{	/* AC3 */
			{
				BRAP_P_ALIGN((47000+21740),2), /* Code + Table */ 
				BRAP_P_ALIGN(33804,2),  /* Interframe buf */
				BRAP_P_ALIGN(0,2),      /* Interstage I/P */ 
				BRAP_P_ALIGN(78688,2),  /* Interstate IF I/P */ 
				BRAP_P_ALIGN(49152,2),      /* Interstage O/P */ 
				BRAP_P_ALIGN(78688,2),  /* Interstate IF O/P */ 
				BRAP_P_ALIGN(3584,2),   /* DEC scratch buf */ 
				BRAP_P_ALIGN(520,2),    /* DEC config */
			},
			BRAP_P_ALIGN((28672 + 0),2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)  /* Ring Buffer */
		},
#endif
		{	/* AC3 Plus  aka DDP  */
			{
				BRAP_P_ALIGN((47000+21740),2), /* Code + Table */ 
				BRAP_P_ALIGN(33804,2),  /* Interframe buf */
				BRAP_P_ALIGN(0,2),      /* Interstage I/P */ 
				BRAP_P_ALIGN(78688,2),  /* Interstate IF I/P */ 
				BRAP_P_ALIGN(49152,2),      /* Interstage O/P */ 
				BRAP_P_ALIGN(78688,2),  /* Interstate IF O/P */ 
				BRAP_P_ALIGN(3584,2),   /* DEC scratch buf */ 
				BRAP_P_ALIGN(520,2),    /* DEC config */
			},
			BRAP_P_ALIGN(28672,2),/* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)       /* Ring Buffer */
		},	
#if ((BRAP_DTS_SUPPORTED == 1) || (BRAP_DTS_PASSTHRU_SUPPORTED==1))		
		{	/* DTS */
			{
				BRAP_P_ALIGN((30720 + 102400),2), /* Code + Table */ 
				BRAP_P_ALIGN(15000,2),  /* Interframe buf */
				BRAP_P_ALIGN(33000,2),      /* Interstage I/P */ 
				BRAP_P_ALIGN(19000,2),  /* Interstate IF I/P */ 
				BRAP_P_ALIGN(65536,2),      /* Interstage O/P */ 
				BRAP_P_ALIGN(0,2),  /* Interstate IF O/P */ 
				BRAP_P_ALIGN(50000,2),      /* DEC scratch buf */ 
				BRAP_P_ALIGN(250,2)     /* DEC config */

			},
			BRAP_P_ALIGN(5000,2),          /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)       /* Ring Buffer */
		},	
#else		
		{	/* DTS */
			{
				BRAP_P_ALIGN((28672 + 87090),2), /* Code + Table */ 
				BRAP_P_ALIGN(13488,2),  /* Interframe buf */
				BRAP_P_ALIGN(8,2),      /* Interstage I/P */ 
				BRAP_P_ALIGN(67488,2),  /* Interstate IF I/P */ 
				BRAP_P_ALIGN(65536,2),      /* Interstage O/P */ 
				BRAP_P_ALIGN(67488,2),  /* Interstate IF O/P */ 
				BRAP_P_ALIGN(8,2),      /* DEC scratch buf */ 
				BRAP_P_ALIGN(164,2)     /* DEC config */

			},
			BRAP_P_ALIGN(0,2),          /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)       /* Ring Buffer */
		},	
#endif		
		{	/* LPCM BD */
			{
				BRAP_P_ALIGN((28672 + 23432),2), /* Code + Table */ 
				BRAP_P_ALIGN(964,2),    /* Interframe buf */
				BRAP_P_ALIGN(30720,2),  /* Interstage I/P */ 
				BRAP_P_ALIGN(28,2),     /* Interstate IF I/P */
				BRAP_P_ALIGN(30720,2),  /* Interstage O/P */ 
				BRAP_P_ALIGN(28,2),     /* Interstate IF O/P */
				BRAP_P_ALIGN(0,2),      /* DEC scratch buf */ 
				BRAP_P_ALIGN(540,2),    /* DEC config */
			},
			BRAP_P_ALIGN(0,2),          /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)       /* Ring Buffer */
		},	
		{	/* LPCM HD-DVD */
			{
				BRAP_P_ALIGN((28672 + 23432),2), /* Code + Table */ 
				BRAP_P_ALIGN(968,2),    /* Interframe buf */
				BRAP_P_ALIGN(32128,2),  /* Interstage I/P */ 
				BRAP_P_ALIGN(28,2),     /* Interstate IF I/P */ 
				BRAP_P_ALIGN(32128,2),  /* Interstage O/P */ 
				BRAP_P_ALIGN(28,2),     /* Interstate IF O/P */
				BRAP_P_ALIGN(0,2),      /* DEC scratch buf */ 
				BRAP_P_ALIGN(540,2),    /* DEC config */
			},
			BRAP_P_ALIGN(0,2),          /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)       /* Ring Buffer */
		},	
            {	/* DTS HD */
			{
				BRAP_P_ALIGN((28672 + 87090),2), /* Code + Table */ 
				BRAP_P_ALIGN(13488,2),  /* Interframe buf */
				BRAP_P_ALIGN(8,2),      /* Interstage I/P */ 
				BRAP_P_ALIGN(67488,2),  /* Interstate IF I/P */ 
				BRAP_P_ALIGN(8,2),      /* Interstage O/P */ 
				BRAP_P_ALIGN(67488,2),  /* Interstate IF O/P */ 
				BRAP_P_ALIGN(8,2),      /* DEC scratch buf */ 
				BRAP_P_ALIGN(164,2),    /* DEC config */
			},
			BRAP_P_ALIGN(0,2),          /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)       /* Ring Buffer */
            },
		{	/* LPCM DVD */
			{
				BRAP_P_ALIGN((28672 + 23432),2), /* Code + Table */ 
				BRAP_P_ALIGN(964,2),    /* Interframe buf */
				BRAP_P_ALIGN(32128,2),  /* Interstage I/P */ 
				BRAP_P_ALIGN(28,2),     /* Interstate IF I/P */ 
				BRAP_P_ALIGN(32128,2),  /* Interstage O/P */ 
				BRAP_P_ALIGN(28,2),     /* Interstate IF O/P */ 
				BRAP_P_ALIGN(0,2),      /* DEC scratch buf */ 
				BRAP_P_ALIGN(612,2),    /* DEC config */
			},
			BRAP_P_ALIGN(0,2),          /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)       /* Ring Buffer */
		},	
		{	/* WMA_STD */
			{
				BRAP_P_ALIGN((40960 + 36000),2), /* Code + Table */ 
				BRAP_P_ALIGN(32840,2),  /* Interframe buf */
				BRAP_P_ALIGN(0,2),      /* Interstage I/P */ 
				BRAP_P_ALIGN(0,2),      /* Interstate IF I/P */ 
				BRAP_P_ALIGN(96256,2),      /* Interstage O/P */ 
				BRAP_P_ALIGN(0,2),      /* Interstate IF O/P */ 
				BRAP_P_ALIGN(22908,2),   /* DEC scratch buf */ 
				BRAP_P_ALIGN(60,2)       /* DEC config */
			},
			BRAP_P_ALIGN(0,2),          /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)       /* Ring Buffer */
		},
		{	/* AC3_LOSSLESS */
			{
				BRAP_P_ALIGN((47000 + 20620),2), /* Code + Table */ 
				BRAP_P_ALIGN(33280,2),  /* Interframe buf */
				BRAP_P_ALIGN(49152,2),  /* Interstage I/P */ 
				BRAP_P_ALIGN(75292,2),  /* Interstate IF I/P */ 
				BRAP_P_ALIGN(49152,2),  /* Interstage O/P */ 
				BRAP_P_ALIGN(75292,2),  /* Interstate IF O/P */ 
				BRAP_P_ALIGN(3584,2),   /* DEC scratch buf */ 
				BRAP_P_ALIGN(412,2)     /* DEC config */
			},
			BRAP_P_ALIGN(28672,2),      /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)       /* Ring Buffer */
		},
		{	/* MLP */
			{
				BRAP_P_ALIGN((14336 + 4608),2), /* Code + Table */ 
				BRAP_P_ALIGN(1024,2),   /* Interframe buf */
				BRAP_P_ALIGN(0,2),      /* Interstage I/P */ 
				BRAP_P_ALIGN(0,2),      /* Interstate IF I/P */ 
				BRAP_P_ALIGN(0,2),      /* Interstage O/P */ 
				BRAP_P_ALIGN(0,2),      /* Interstate IF O/P */ 
				BRAP_P_ALIGN(0,2),      /* DEC scratch buf */ 
				BRAP_P_ALIGN(40,2)      /* DEC config */
			},
			BRAP_P_ALIGN(0,2),          /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)       /* Ring Buffer */
		},
		{	/* WMA_PRO */
			{
				BRAP_P_ALIGN((26624 +22320),2), /* Code + Table */ 
				BRAP_P_ALIGN(133500,2), /* Interframe buf */
				BRAP_P_ALIGN(152500,2), /* Interstage I/P */ 
				BRAP_P_ALIGN(128,2), /* Interstate IF I/P*/ 
				BRAP_P_ALIGN(152500,2), /* Interstage O/P */ 
				BRAP_P_ALIGN(128,2), /* Interstate IF O/P*/ 
				BRAP_P_ALIGN(174080,2), /* DEC scratch buf */ 
				BRAP_P_ALIGN(118,2)  /* DEC config */
			},
			BRAP_P_ALIGN(0,2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2), /* Ring Buffer */
		},
		{	/* PCM Wav */
			{
				BRAP_P_ALIGN((25000+0),2), /* Code + Table */ 
				BRAP_P_ALIGN(480,2), /* Interframe buf */
				BRAP_P_ALIGN(24576,2), /* Interstage I/P */ 
				BRAP_P_ALIGN(0,2), /* Interstate IF I/P*/ 
				BRAP_P_ALIGN(24576,2), /* Interstage O/P */ 
				BRAP_P_ALIGN(0,2), /* Interstate IF O/P*/ 
				BRAP_P_ALIGN(0,2), /* DEC scratch buf */ 
				BRAP_P_ALIGN(20,2) /* DEC config */
			},
			BRAP_P_ALIGN(0,2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2), /* Ring Buffer */
		}		
		,{	/* DRA */
			{
				BRAP_P_ALIGN((20000 + 40000),2), /* Code + Table */
				BRAP_P_ALIGN(35000,2),  /* Interframe buf */
				BRAP_P_ALIGN(0,2),  /* Interstage I/P */
				BRAP_P_ALIGN(0,2),   /* Interstate IF I/P */
				BRAP_P_ALIGN(0,2),  /* Interstage O/P */
				BRAP_P_ALIGN(0,2),   /* Interstate IF O/P */
				BRAP_P_ALIGN(30000,2),  /* DEC scratch buf */
				BRAP_P_ALIGN(32,2),    /* DEC config */
			},
			BRAP_P_ALIGN(4096,2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)       /* Ring Buffer */
		}
#ifdef RAP_SRSTRUVOL_CERTIFICATION
		,{	/* PCM Passthru */
			{
				BRAP_P_ALIGN((87040 + 28672),2), /* Code + Table */
				BRAP_P_ALIGN(8,2),  /* Interframe buf */
				BRAP_P_ALIGN(8,2),  /* Interstage I/P */
				BRAP_P_ALIGN(8,2),   /* Interstate IF I/P */
				BRAP_P_ALIGN(26576,2),  /* Interstage O/P */
				BRAP_P_ALIGN(8,2),   /* Interstate IF O/P */
				BRAP_P_ALIGN(8,2),  /* DEC scratch buf */
				BRAP_P_ALIGN(8,2),    /* DEC config */
			},
			BRAP_P_ALIGN(0,2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)       /* Ring Buffer */
		}
#endif
};
#else

static const 
BRAP_P_AlgoMemRequirement sAlgoMemRequirementArray[BRAP_MAX_AUDIO_TYPES] = {
		{	/* MPEG */
			{
#if defined ( BCHP_7411_VER ) 
				BRAP_P_ALIGN((86016 /* 28672 */ + 23432),2), /* Code + Table */
				BRAP_P_ALIGN(10916,2), /* Interframe buf */
				BRAP_P_ALIGN(0,2), /* Interstage */
				BRAP_P_ALIGN(0,2), /* Interstate IF */
				BRAP_P_ALIGN(0,2), /* DEC scratch buf */
				BRAP_P_ALIGN(8,2)  /* DEC config */
#else
				BRAP_P_ALIGN((59392 /* 28672 */ + 24250),2), /* Code + Table */
				BRAP_P_ALIGN(11172,2), /* Interframe buf */
				BRAP_P_ALIGN(0,2), /* Interstage */
				BRAP_P_ALIGN(0,2), /* Interstate IF */
				BRAP_P_ALIGN(0,2), /* DEC scratch buf */
				BRAP_P_ALIGN(20,2)  /* DEC config */
#endif
			},
			BRAP_P_ALIGN((2048 + 0),2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2),  /* Ring Buffer */
			BRAP_P_ALIGN(12000,2) /*Frame Sync Size*/
		},	
		{	/* AAC */
			{
				BRAP_P_ALIGN((28672 + 29612 + 10240),2), /* Code + Table + Downmix code */
				BRAP_P_ALIGN(14336,2), /* Interframe buf */
				BRAP_P_ALIGN(49152,2), /* Interstage */
				BRAP_P_ALIGN(1500,2), /* Intersate IF */
				BRAP_P_ALIGN(73088,2), /* DEC scratch buf */
				BRAP_P_ALIGN(340,2)  /* DEC config */
			},
			BRAP_P_ALIGN((2048 + 0),2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2),  /* Ring Buffer */
			BRAP_P_ALIGN(12000,2) /*Frame Sync Size*/
		},	
		{	/* AAC_SBR */
			{
				BRAP_P_ALIGN((57344 + 44084 + 10000),2), /* Code + Table */
				BRAP_P_ALIGN(52180,2), /* Interframe buf */
				BRAP_P_ALIGN(16384,2), /* Interstage */
				BRAP_P_ALIGN(1104,2), /* Interstate IF */ 
				BRAP_P_ALIGN(73088,2), /* DEC scratch buf */ 
				BRAP_P_ALIGN(340,2)   /* DEC config */
			},
			BRAP_P_ALIGN((2048 + 0),2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2),  /* Ring Buffer */
			BRAP_P_ALIGN(12000,2) /*Frame Sync Size*/
		},	
#if defined ( BCHP_7411_VER ) 
		{	/* AC3 */
			{
				BRAP_P_ALIGN((28672 + 7560),2), /* Code + Table */ 
				BRAP_P_ALIGN(3376,2), /* Interframe buf */
				BRAP_P_ALIGN(0,2), /* Interstage */ 
				BRAP_P_ALIGN(0,2), /* Interstate IF */ 
				BRAP_P_ALIGN(0,2), /* DEC scratch buf */ 
				BRAP_P_ALIGN(110,2)   /* DEC config */
			},
			BRAP_P_ALIGN((2292 + 0),2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2),  /* Ring Buffer */
			BRAP_P_ALIGN(12000,2) /*Frame Sync Size*/
		},
#elif (BRAP_7401_FAMILY == 1)
		{	/* AC3 */
			{
				BRAP_P_ALIGN((28672 + 7560),2), /* Code + Table */ 
				BRAP_P_ALIGN(31588,2), /* Interframe buf */
				BRAP_P_ALIGN(0,2), /* Interstage */ 
				BRAP_P_ALIGN(0,2), /* Interstate IF */ 
				BRAP_P_ALIGN(0,2), /* DEC scratch buf */ 
				BRAP_P_ALIGN(110,2)   /* DEC config */
			},
			BRAP_P_ALIGN((2292 + 0),2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2),  /* Ring Buffer */
			BRAP_P_ALIGN(12000,2) /*Frame Sync Size*/
		},                
#else
		
		/* We use DDP executables for AC3 decoding */
		{	/* AC3 */
			{
				BRAP_P_ALIGN((47000+21740),2), /* Code + Table */ 
				BRAP_P_ALIGN(0x840C,2), /* Interframe buf */
				BRAP_P_ALIGN(0,2), /* Interstage */ 
				BRAP_P_ALIGN(0x13360,2), /* Interstate IF */ 
				BRAP_P_ALIGN(0xe00,2), /* DEC scratch buf */ 
				BRAP_P_ALIGN(0x19c,2)  /* DEC config */
			},
			BRAP_P_ALIGN((28672 + 0),2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2),  /* Ring Buffer */
			BRAP_P_ALIGN(12000,2) /*Frame Sync Size*/
		},	
#endif		
		{	/* AC3 Plus */
			{
#if defined (BCHP_7411_VER ) /* 7114 C0 and D0 */
				BRAP_P_ALIGN((45056/*47000*/+20620),2), /* Code + Table */ 
				BRAP_P_ALIGN(31512/*0x8200*/,2), /* Interframe buf */
				BRAP_P_ALIGN(0/*0xc000*/,2), /* Interstage */ 
				BRAP_P_ALIGN(75264/*0x1261c*/,2), /* Interstate IF */ 
				BRAP_P_ALIGN(3584/*0xe00*/,2), /* DEC scratch buf */ 
				BRAP_P_ALIGN(212/*0x19c*/,2)  /* DEC config */
#else	/* For 7400 or 7401 or 7118 */			
				BRAP_P_ALIGN((47000+21740),2), /* Code + Table */ 
				BRAP_P_ALIGN(0x840C,2), /* Interframe buf */
				BRAP_P_ALIGN(0,2), /* Interstage */ 
				BRAP_P_ALIGN(0x13360,2), /* Interstate IF */ 
				BRAP_P_ALIGN(0xe00,2), /* DEC scratch buf */ 
				BRAP_P_ALIGN(0x19c,2)  /* DEC config */
#endif				
			},
			BRAP_P_ALIGN((28672 + 0),2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2),  /* Ring Buffer */
			BRAP_P_ALIGN(12000,2) /*Frame Sync Size*/
		},	
		{	/* DTS */
			{
#if BCHP_7411_VER > BCHP_VER_C0 /* Only for 7411D0 */
				BRAP_P_ALIGN((28672 + 87242),2), /* Code + Table */ 
				BRAP_P_ALIGN(17848,2), /* Interframe buf */
				BRAP_P_ALIGN(16384,2), /* Interstage */ 
				BRAP_P_ALIGN(18336,2), /* Interstate IF */ 
				BRAP_P_ALIGN(8,2), /* DEC scratch buf */ 
				BRAP_P_ALIGN(168,2)  /* DEC config */
#else
				BRAP_P_ALIGN((28672 + 87090),2), /* Code + Table */ 
				BRAP_P_ALIGN(13488,2), /* Interframe buf */
				BRAP_P_ALIGN(8,2), /* Interstage */ 
				BRAP_P_ALIGN(67488,2), /* Interstate IF */ 
				BRAP_P_ALIGN(8,2), /* DEC scratch buf */ 
				BRAP_P_ALIGN(164,2)  /* DEC config */
#endif /* BCHP_7411_VER > BCHP_VER_C0 */								
			},
			BRAP_P_ALIGN(0,2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2), /* Ring Buffer */
			BRAP_P_ALIGN(12000,2) /*Frame Sync Size*/
		},	
		{	/* LPCM BD */
			{
				BRAP_P_ALIGN((28672 + 23432),2), /* Code + Table */ 
				BRAP_P_ALIGN(964,2), /* Interframe buf */
				BRAP_P_ALIGN(30720,2), /* Interstage */ 
				BRAP_P_ALIGN(28,2), /* Interstate IF */
				BRAP_P_ALIGN(0,2), /* DEC scratch buf */ 
				BRAP_P_ALIGN(540,2)  /* DEC config */
			},
			BRAP_P_ALIGN(0,2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2),  /* Ring Buffer */
			BRAP_P_ALIGN(12000,2) /*Frame Sync Size*/
		},	
		{	/* LPCM HD-DVD */
			{
				BRAP_P_ALIGN((28672 + 23432),2), /* Code + Table */ 
				BRAP_P_ALIGN(968,2), /* Interframe buf */
				BRAP_P_ALIGN(32128 ,2), /* Interstage */ 
				BRAP_P_ALIGN(28,2), /* Interstate IF */ 
				BRAP_P_ALIGN(0,2), /* DEC scratch buf */ 
				BRAP_P_ALIGN(540,2)  /* DEC config */
			},
			BRAP_P_ALIGN(0,2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2), /* Ring Buffer */
			BRAP_P_ALIGN(12000,2) /*Frame Sync Size*/
		},	
            {	/* DTS HD */
			{
#if BCHP_7411_VER > BCHP_VER_C0 /* Only for 7411D0 */
				BRAP_P_ALIGN((28672 + 87242),2), /* Code + Table */ 
				BRAP_P_ALIGN(17848,2), /* Interframe buf */
				BRAP_P_ALIGN(16384,2), /* Interstage */ 
				BRAP_P_ALIGN(18336,2), /* Interstate IF */ 
				BRAP_P_ALIGN(8,2), /* DEC scratch buf */ 
				BRAP_P_ALIGN(168,2)  /* DEC config */
#else
				BRAP_P_ALIGN((28672 + 87090),2), /* Code + Table */ 
				BRAP_P_ALIGN(13488,2), /* Interframe buf */
				BRAP_P_ALIGN(8,2), /* Interstage */ 
				BRAP_P_ALIGN(67488,2), /* Interstate IF */ 
				BRAP_P_ALIGN(8,2), /* DEC scratch buf */ 
				BRAP_P_ALIGN(164,2)  /* DEC config */
#endif /* BCHP_7411_VER > BCHP_VER_C0 */				
			},
			BRAP_P_ALIGN(0,2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2), /* Ring Buffer */
			BRAP_P_ALIGN(12000,2) /*Frame Sync Size*/
            },
		{	/* LPCM DVD */
			{
				BRAP_P_ALIGN((28672 + 23432),2), /* Code + Table */ 
				BRAP_P_ALIGN(964,2), /* Interframe buf */
				BRAP_P_ALIGN(32128,2), /* Interstage */ 
				BRAP_P_ALIGN(28,2), /* Interstate IF */ 
				BRAP_P_ALIGN(0,2), /* DEC scratch buf */ 
				BRAP_P_ALIGN(540,2)  /* DEC config */
			},
			BRAP_P_ALIGN(0,2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2), /* Ring Buffer */
			BRAP_P_ALIGN(12000,2) /*Frame Sync Size*/
		},
		{	/* WMA_STD */
			{
				BRAP_P_ALIGN((40960 + 36000),2), /* Code + Table */ 
				BRAP_P_ALIGN(32840,2), /* Interframe buf */
				BRAP_P_ALIGN(0,2), /* Interstage */ 
				BRAP_P_ALIGN(0,2), /* Interstate IF */ 
				BRAP_P_ALIGN(6524,2), /* DEC scratch buf */ 
				BRAP_P_ALIGN(0,2)  /* DEC config */
			},
			BRAP_P_ALIGN(0,2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2), /* Ring Buffer */
			BRAP_P_ALIGN(12000,2) /*Frame Sync Size*/
		},
		{	/* AC3_LOSSLESS */
			{
				BRAP_P_ALIGN((47000 + 20620),2), /* Code + Table */ 
				BRAP_P_ALIGN(0x8200,2), /* Interframe buf */
				BRAP_P_ALIGN(0xc000,2), /* Interstage */ 
				BRAP_P_ALIGN(0x1261c,2), /* Interstate IF */ 
				BRAP_P_ALIGN(0xe00,2), /* DEC scratch buf */ 
				BRAP_P_ALIGN(0x19c,2)   /* DEC config */
			},
			BRAP_P_ALIGN((28672 + 0),2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2),  /* Ring Buffer */
			BRAP_P_ALIGN(12000,2) /*Frame Sync Size*/
		},
		{	/* MLP */
			{
				BRAP_P_ALIGN((14336 + 4608),2), /* Code + Table */ 
				BRAP_P_ALIGN(1024,2), /* Interframe buf */
				BRAP_P_ALIGN(0,2), /* Interstage */ 
				BRAP_P_ALIGN(0,2), /* Interstate IF */ 
				BRAP_P_ALIGN(0,2), /* DEC scratch buf */ 
				BRAP_P_ALIGN(40,2)   /* DEC config */
			},
			BRAP_P_ALIGN((0 + 0),2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2),  /* Ring Buffer */
			BRAP_P_ALIGN(12000,2) /*Frame Sync Size*/
		},
		{	/* WMA_PRO */
			{
				BRAP_P_ALIGN((26624 +16384 ),2), /* Code + Table */ 
				BRAP_P_ALIGN(133000,2), /* Interframe buf */
				BRAP_P_ALIGN(152500,2), /* Interstage */ 
				BRAP_P_ALIGN(128,2), /* Interstate IF */ 
				BRAP_P_ALIGN(174080,2), /* DEC scratch buf */ 
				BRAP_P_ALIGN(48,2)  /* DEC config */
			},
			BRAP_P_ALIGN(0,2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2), /* Ring Buffer */
			BRAP_P_ALIGN(12000,2) /*Frame Sync Size*/
		},
		{	/* DRA */
			{
				BRAP_P_ALIGN((20000 + 40000),2), /* Code + Table */
				BRAP_P_ALIGN(35000,2),  /* Interframe buf */
				BRAP_P_ALIGN(0,2),  /* Interstage I/P */
				BRAP_P_ALIGN(0,2),   /* Interstate IF I/P */
				BRAP_P_ALIGN(9000,2),  /* Interstage O/P */
				BRAP_P_ALIGN(0,2),   /* Interstate IF O/P */
				BRAP_P_ALIGN(30000,2),  /* DEC scratch buf */
				BRAP_P_ALIGN(32,2),    /* DEC config */
			},
			BRAP_P_ALIGN(4096,2), /* Pass Thru Exec Size */
			BRAP_P_ALIGN(24576,2)       /* Ring Buffer */
		}
};
#endif

static const
uint32_t	ui32MultiChannelInterstageBufReqArray[BRAP_DSPCHN_AudioType_eMax] = {
	BRAP_P_ALIGN(0, 2),			/* MPEG */
	BRAP_P_ALIGN(49152, 2),		/* AAC */
	BRAP_P_ALIGN(16384,2),		/* AAC-SBR */
	BRAP_P_ALIGN(0x9000,2),		/* AC3 */
	BRAP_P_ALIGN(0xc000 /*0x9000*/,2),		/* AC3 Plus */
	BRAP_P_ALIGN(16384,2),		/* DTS */
	BRAP_P_ALIGN(30720,2),		/* LPCM-BD */
	BRAP_P_ALIGN(32128,2),		/* LPCM HD-DVD */
	BRAP_P_ALIGN(16384,2),		/* DTS-HD */
	BRAP_P_ALIGN(32128,2),		/* LPCM-DVD */
	BRAP_P_ALIGN(0,2),			/* WMA-STD */
	BRAP_P_ALIGN(0xc000,2),		/* AC3 LOSSLESS */
	BRAP_P_ALIGN(0,2),           /* MLP */
	BRAP_P_ALIGN(51200,2)           /* WMA PRO */
};


#if (BRAP_DSP_P_7401_NEWMIT==1)
static const
BRAP_P_Pp_AlgoMemRequirement sPpAlgoMemRequirementArray[BRAP_DSPCHN_PP_Algo_eMax] = {
		{	/* BRAP_DSPCHN_PP_Algo_eDdbm */ /*note DDBM is not used for 7401 */
			{
			BRAP_P_ALIGN((8192 + 8192), 2), /* FW Code + Table size */ 
				BRAP_P_ALIGN(0,2),			/* Interframe buf size */  
				BRAP_P_ALIGN(0,2), 			/* Interstage Input Buf size */ 
				BRAP_P_ALIGN(0,2),			/* Interstate IF Input */ 
				BRAP_P_ALIGN(0,2), 			/* Interstage Output Buf size */ 
				BRAP_P_ALIGN(0,2),			/* Interstate IF Output */ 
				BRAP_P_ALIGN(0,2),          /* PP scratch buf */
				BRAP_P_ALIGN(0,2)           /* PP config */ 
			}
		},	
		{	/* BRAP_DSPCHN_PP_Algo_eSrc */
			{
    			BRAP_P_ALIGN((4600 + 700), 2), /* FW Code + Table size */ 
				BRAP_P_ALIGN(4352,2),	    /* Interframe buf size */ 
				BRAP_P_ALIGN(0,2), 			/* Interstage Input Buf size */  
				BRAP_P_ALIGN(0,2),			/* Interstate IF Input */ 
				BRAP_P_ALIGN(0,2), 			/* Interstage Output Buf size */ 
				BRAP_P_ALIGN(0,2),			/* Interstate IF Output */ 
				BRAP_P_ALIGN(0,2),          /* PP scratch buf */
				BRAP_P_ALIGN(0,2)           /* PP config */  
			}
		},
		{	/* BRAP_DSPCHN_PP_Algo_eAacDownmix */
			{
    			BRAP_P_ALIGN(10240, 2), /* FW Code + Table size */ 
				BRAP_P_ALIGN(0,2),	    /* Interframe buf size */ 
				BRAP_P_ALIGN(0,2), 			/* Interstage Input Buf size */  
				BRAP_P_ALIGN(0,2),			/* Interstate IF Input */ 
				BRAP_P_ALIGN(0,2), 			/* Interstage Output Buf size */ 
				BRAP_P_ALIGN(0,2),			/* Interstate IF Output */ 
				BRAP_P_ALIGN(0,2),          /* PP scratch buf */
				BRAP_P_ALIGN(0,2)           /* PP config */  
			}
        },
        {   /* BRAP_DSPCHN_PP_Algo_eDsola */
            {   /* FIXME: get correct sizes */
                BRAP_P_ALIGN(6800+1500, 2),     /* FW Code + Table size */ 
                BRAP_P_ALIGN(15000,2),      /* Interframe buf size */ 
                BRAP_P_ALIGN(0,2),     /* Interstage Input Buf size */  
                BRAP_P_ALIGN(0,2),         /* Interstate IF Input */ 
                BRAP_P_ALIGN(0,2),     /* Interstage Output Buf size */ 
                BRAP_P_ALIGN(0,2),         /* Interstate IF Output */ 
                BRAP_P_ALIGN(60000,2),         /* PP scratch buf */
                BRAP_P_ALIGN(0,2)          /* PP config */  
            }
		}		
#if (BRAP_AD_SUPPORTED == 1)
		,
		{	/* BRAP_DSPCHN_PP_Algo_eAD_FadeCtrl */
			{
    			BRAP_P_ALIGN((4600 + 2048), 2), /* FW Code + Table size */ 
				BRAP_P_ALIGN(4352,2),	    /* Interframe buf size */ 
				BRAP_P_ALIGN(0,2), 			/* Interstage Input Buf size */  
				BRAP_P_ALIGN(0,2),			/* Interstate IF Input */ 
				BRAP_P_ALIGN(0,2), 			/* Interstage Output Buf size */ 
				BRAP_P_ALIGN(0,2),			/* Interstate IF Output */ 
				BRAP_P_ALIGN(0,2),          /* PP scratch buf */
				BRAP_P_ALIGN(0,2)           /* PP config */  
			}
		},
		{	/* BRAP_DSPCHN_PP_Algo_eAD_PanCtrl */
			{
    			BRAP_P_ALIGN((6000 + 2048), 2), /* FW Code + Table size */ 
				BRAP_P_ALIGN(4352,2),	    /* Interframe buf size */ 
				BRAP_P_ALIGN(0,2), 			/* Interstage Input Buf size */  
				BRAP_P_ALIGN(0,2),			/* Interstate IF Input */ 
				BRAP_P_ALIGN(0,2), 			/* Interstage Output Buf size */ 
				BRAP_P_ALIGN(0,2),			/* Interstate IF Output */ 
				BRAP_P_ALIGN(0,2),          /* PP scratch buf */
				BRAP_P_ALIGN(0,2)           /* PP config */  
			}
		}		
#endif
#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)
		,{	/* BRAP_DSPCHN_PP_Algo_eDolbyVolume */
			{
    			BRAP_P_ALIGN((44000 + 200000), 2), /* FW Code + Table size */ 
				BRAP_P_ALIGN(18000,2),	    /* Interframe buf size */ 
				BRAP_P_ALIGN(0,2), 			/* Interstage Input Buf size */  
				BRAP_P_ALIGN(0,2),			/* Interstate IF Input */ 
				BRAP_P_ALIGN(0,2), 			/* Interstage Output Buf size */ 
				BRAP_P_ALIGN(0,2),			/* Interstate IF Output */ 
				BRAP_P_ALIGN(50000,2),          /* PP scratch buf */
				BRAP_P_ALIGN(110,2)           /* PP config */  
			}
		}
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)
		,{	/* BRAP_DSPCHN_PP_Algo_eSRS_TruVol */
			{
    			BRAP_P_ALIGN((20000 + 8000), 2), /* FW Code + Table size */ 
				BRAP_P_ALIGN(18000,2),	    /* Interframe buf size */ 
				BRAP_P_ALIGN(0,2), 			/* Interstage Input Buf size */  
				BRAP_P_ALIGN(0,2),			/* Interstate IF Input */ 
				BRAP_P_ALIGN(0,2), 			/* Interstage Output Buf size */ 
				BRAP_P_ALIGN(0,2),			/* Interstate IF Output */ 
				BRAP_P_ALIGN(8,2),          /* PP scratch buf */
				BRAP_P_ALIGN(112,2)           /* PP config */  
			}
		}
#endif
};
#else
static const
BRAP_P_DecodeModeMemRequirement sPpAlgoMemRequirementArray[BRAP_DSPCHN_PP_Algo_eMax] = {
		{ /* BRAP_DSPCHN_PP_Algo_eDdbm */
			BRAP_P_ALIGN((8192 + 8192), 2), 		/* FW Code + Table size */ 
				BRAP_P_ALIGN(0,2),			/* Interframe buf size */
				BRAP_P_ALIGN(0,2), 			/* Interstage Buf size */ 
				BRAP_P_ALIGN(0,2),			/* Interstate IF */ 
				BRAP_P_ALIGN(0,2), 			/* DEC scratch buf */ 
				BRAP_P_ALIGN(0, 2)			/* DEC config */
		},
		{ /* BRAP_DSPCHN_PP_Algo_eSrc */
			BRAP_P_ALIGN((4600 + 700), 2), 		/* FW Code + Table size */ 
				BRAP_P_ALIGN(0,2),			/* Interframe buf size */
				BRAP_P_ALIGN(0,2), 			/* Interstage Buf size */ 
				BRAP_P_ALIGN(0,2),			/* Interstate IF */ 
				BRAP_P_ALIGN(0,2), 			/* DEC scratch buf */ 
				BRAP_P_ALIGN(0, 2)			/* DEC config */
		}
};
#endif

/* This function gives the absolute value corresponding to the enum 
   BAVC_AudioSamplingRate */
BERR_Code BRAP_P_ConvertSR (
    BAVC_AudioSamplingRate   eSR,   /* [in] samping rate enum */
    unsigned int            *uiSR   /* [out] sampling rate unsigned value */
)
{
    BERR_Code err = BERR_SUCCESS;
    BDBG_ASSERT(uiSR);

    switch(eSR)
    {
    	case BAVC_AudioSamplingRate_e32k:   *uiSR = 32000; break;
        case BAVC_AudioSamplingRate_e44_1k: *uiSR = 44100; break;
        case BAVC_AudioSamplingRate_e48k:   *uiSR = 48000; break;
        case BAVC_AudioSamplingRate_e96k:   *uiSR = 96000; break;
        case BAVC_AudioSamplingRate_e16k:   *uiSR = 16000; break;
        case BAVC_AudioSamplingRate_e22_05k:*uiSR = 22050; break;
        case BAVC_AudioSamplingRate_e24k:   *uiSR = 24000; break;
        case BAVC_AudioSamplingRate_e64k:   *uiSR = 64000; break;
        case BAVC_AudioSamplingRate_e88_2k: *uiSR = 88200; break;
        case BAVC_AudioSamplingRate_e128k:  *uiSR = 128000; break;
        case BAVC_AudioSamplingRate_e176_4k:*uiSR = 176400; break;
        case BAVC_AudioSamplingRate_e192k:  *uiSR = 192000; break;
        case BAVC_AudioSamplingRate_e12k:  *uiSR = 12000; break;
        case BAVC_AudioSamplingRate_e8k:  *uiSR = 8000; break;
        case BAVC_AudioSamplingRate_e11_025k:  *uiSR = 11025; break;
        case BAVC_AudioSamplingRate_eUnknown: 
        default:            
            *uiSR = 0; err = BERR_INVALID_PARAMETER; break;
    }
    return err;        
}

/* BRAP_P_AlignAddress : Aligns the address to the specified bit position
 */
uint32_t BRAP_P_AlignAddress(
		uint32_t	ui32Address,	/* [in] size in bytes of block to allocate */
		uint32_t	uiAlignBits		/* [in] alignment for the block */
		)
{
	return (ui32Address+((1<<uiAlignBits)-1)) & ~((1<<uiAlignBits)-1) ;
}

/* BRAP_P_GetAlgoMemRequirement: This function gets total memory required for an
 * algorithm. This memory requirement is not considering following buffers
 * CDB/ITB
 * Common Post Processing buffers (For downloading FW image)
 * Firmware Resident code
 * Master Index Table structure */

BERR_Code BRAP_P_GetAlgoMemRequirement (
				BRAP_Handle hRap, 	/* [in] RAP Handle */
				BRAP_DSPCHN_AudioType eAudioType,	/* [in] Audio Type */
#if (BRAP_DSP_P_7401_NEWMIT==1)
				BRAP_P_Dec_AlgoMemRequirement *psAlgoMemReq /* [out] pointer to struct to be
														   initialized with memory requirement
														   for the given algorithm */
#else
				BRAP_P_AlgoMemRequirement *psAlgoMemReq /* [out] pointer to struct to be
														   initialized with memory requirement
														   for the given algorithm */
#endif
)
{
	BERR_Code err = BERR_SUCCESS;
	
	BDBG_ASSERT(hRap);
	BDBG_ASSERT(psAlgoMemReq);
	BSTD_UNUSED(hRap);

	*psAlgoMemReq = sAlgoMemRequirementArray[eAudioType];

	return err;
}

/* BRAP_P_GetPpAlgoMemRequirement: This function gets memory requirement for a
 * post processing algorithm. This function essentially gets the split-up of scratch buffer
 * as required by post processing algorithm.
 */


BERR_Code BRAP_P_GetPpAlgoMemRequirement(
				BRAP_Handle hRap, 							/* [in] RAP Handle */
				BRAP_DSPCHN_PP_Algo	ePpAlgo,			/* [in] Post processing algorithm */
#if (BRAP_DSP_P_7401_NEWMIT==1)				
				BRAP_P_Pp_AlgoMemRequirement * psPpAlgoMemReq	/* [out] pointer to struct to be
														   initialized with memory requirement
														   for the given post processing algorithm */

#else
				BRAP_P_AlgoMemRequirement * psPpAlgoMemReq	/* [out] pointer to struct to be
														   initialized with memory requirement
														   for the given post processing algorithm */
#endif														   
)
{
	BERR_Code err = BERR_SUCCESS;
	
	BDBG_ASSERT(hRap);
	BDBG_ASSERT(psPpAlgoMemReq);
	BSTD_UNUSED(hRap);

#if (BRAP_DSP_P_7401_NEWMIT==1)
	BKNI_Memset(psPpAlgoMemReq, 0x0, sizeof(BRAP_P_Pp_AlgoMemRequirement));
	*psPpAlgoMemReq = sPpAlgoMemRequirementArray[ePpAlgo];

    BDBG_MSG(("For PP algo %d:", ePpAlgo));
    BDBG_MSG(("ui32PpFirmwareExecutablesSize :0x%x", sPpAlgoMemRequirementArray[ePpAlgo].sPpMemReq.ui32PpFirmwareExecutablesSize));
    BDBG_MSG(("ui32PpInterframeBufSize :0x%x", sPpAlgoMemRequirementArray[ePpAlgo].sPpMemReq.ui32PpInterframeBufSize));
    BDBG_MSG(("ui32PpInterstageInputBufSize :0x%x", sPpAlgoMemRequirementArray[ePpAlgo].sPpMemReq.ui32PpInterstageInputBufSize));
    BDBG_MSG(("ui32PpInterstageInputInterfaceBufSize :0x%x", sPpAlgoMemRequirementArray[ePpAlgo].sPpMemReq.ui32PpInterstageInputInterfaceBufSize));
    BDBG_MSG(("ui32PpInterstageOutputBufSize :0x%x", sPpAlgoMemRequirementArray[ePpAlgo].sPpMemReq.ui32PpInterstageOutputBufSize));
    BDBG_MSG(("ui32PpInterstageOutputInterfaceBufSize :0x%x", sPpAlgoMemRequirementArray[ePpAlgo].sPpMemReq.ui32PpInterstageOutputInterfaceBufSize));
    BDBG_MSG(("ui32PpScratchBufSize :0x%x", sPpAlgoMemRequirementArray[ePpAlgo].sPpMemReq.ui32PpScratchBufSize));
    BDBG_MSG(("ui32PpConfigParamStructSize :0x%x", sPpAlgoMemRequirementArray[ePpAlgo].sPpMemReq.ui32PpConfigParamStructSize));  
#else
	BKNI_Memset(psPpAlgoMemReq, 0x0, sizeof(BRAP_P_AlgoMemRequirement));
	psPpAlgoMemReq->sDecodeMemReq = sPpAlgoMemRequirementArray[ePpAlgo];
#endif


    
    

	return err;
}

/* BRAP_P_GetSelectedAlgos: This function returns audio types selected by application
 * during BRAP_Open call */
BERR_Code BRAP_P_GetSelectedAlgos(
			BRAP_Handle	hRap,		/* [in] RAP Handle */
			bool *pbSelectedAlgos	/* [out] pointer to bSupportAlgos[BRAP_MAX_AUDIO_TYPES] array */
)
{
	unsigned int i;
	BERR_Code err = BERR_SUCCESS;
	
	BDBG_ASSERT(hRap);
	BDBG_ASSERT(pbSelectedAlgos);

	for (i = 0; i < BRAP_MAX_AUDIO_TYPES; i++) {
		*(pbSelectedAlgos + i) = hRap->sSettings.bSupportAlgos[i];
	}

	return err;
}

#if (BCHP_7411_VER) || (BRAP_7401_FAMILY == 1) || ( BCHP_CHIP == 7400 )
/* BRAP_P_GetSelectedPpAlgos: This function returns post processing algorithms selected
  * by application during BRAP_Open call */
 BERR_Code BRAP_P_GetSelectedPpAlgos(
			BRAP_Handle	hRap,		/* [in] RAP Handle */
			bool *pbSelectedPpAlgos	/* [out] pointer to bSupportPpAlgos[BRAP_MAX_PP_ALGOS] array */
)
{
	unsigned int i;
	BERR_Code err = BERR_SUCCESS;
	
	BDBG_ASSERT(hRap);
	BDBG_ASSERT(pbSelectedPpAlgos);

	for (i = 0; i < BRAP_MAX_PP_ALGOS; i++) {
		*(pbSelectedPpAlgos + i) = hRap->sSettings.bSupportPpAlgos[i];
	}

	return err;

}
#endif
 
/* BRAP_P_GetDecodeBufferSizes : This function finds out worst case sizes of various buffers
    required for combination of algorithms selected by application for a RAP decode
    channel.
    */

#if (BRAP_DSP_P_7401_NEWMIT==1)
BERR_Code BRAP_P_GetDecChannelBufferSizes(
						bool *pbSupportAlgos,	/* [in] pointer to bSupportAlgos[BRAP_MAX_AUDIO_TYPES] array */
						BRAP_DSPCHN_DecodeMode eDecodeMode, /* [in] Decode mode */
						BRAP_DEC_P_ChannelMemReq	*psChMemReq, /* [out] Dec channel memory requirement */
            			bool bFwAuthEnable	 /*Firmware authentication Enable bit */
			
						)
{
	int i = 0;
	unsigned int maxInterstageBuf = 0, maxInterstageInterfaceBuf = 0;
	BRAP_P_Dec_AlgoMemRequirement	sAlgoBufSizes;
	BERR_Code err = BERR_SUCCESS;
	uint32_t	ui32FrameSyncSize = 0;

	BDBG_ASSERT(psChMemReq);
	
	BKNI_Memset(&sAlgoBufSizes, 0, sizeof(BRAP_P_Dec_AlgoMemRequirement));
	BKNI_Memset(psChMemReq, 0, sizeof(BRAP_DEC_P_ChannelMemReq));
	
	if (eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode) {
		BDBG_ERR(("This function finds buffer sizes only for decode and passthru modes"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	for (i = 0; i < BRAP_MAX_AUDIO_TYPES; i++) {
		/* If algorithm is not selected by application, go to next algorithm */
		if (*(pbSupportAlgos + i)==false)
			continue;
		
		if (eDecodeMode==BRAP_DSPCHN_DecodeMode_eDecode) {	
			
	
			if(bFwAuthEnable==false)
			{
			
			if (sAlgoMemRequirementArray[i].sDecodeMemReq.ui32FirmwareExecutablesSize > 
				sAlgoBufSizes.sDecodeMemReq.ui32FirmwareExecutablesSize)
				sAlgoBufSizes.sDecodeMemReq.ui32FirmwareExecutablesSize = 
					sAlgoMemRequirementArray[i].sDecodeMemReq.ui32FirmwareExecutablesSize;
								
			}
			else
			{
				sAlgoBufSizes.sDecodeMemReq.ui32FirmwareExecutablesSize+=sAlgoMemRequirementArray[i].sDecodeMemReq.ui32FirmwareExecutablesSize;		
				ui32FrameSyncSize += BRAP_DSP_P_CTXT_FS_EXE_SIZE;
			}
		
			if (sAlgoMemRequirementArray[i].sDecodeMemReq.ui32InterstageOutputBufSize > maxInterstageBuf)
				maxInterstageBuf = sAlgoMemRequirementArray[i].sDecodeMemReq.ui32InterstageOutputBufSize;

			if (sAlgoMemRequirementArray[i].sDecodeMemReq.ui32InterstageInputInterfaceBufSize > maxInterstageInterfaceBuf)
				maxInterstageInterfaceBuf = sAlgoMemRequirementArray[i].sDecodeMemReq.ui32InterstageInputInterfaceBufSize;

			if (sAlgoMemRequirementArray[i].sDecodeMemReq.ui32InterstageOutputInterfaceBufSize > maxInterstageInterfaceBuf)
				maxInterstageInterfaceBuf = sAlgoMemRequirementArray[i].sDecodeMemReq.ui32InterstageOutputInterfaceBufSize;
			
			if (sAlgoMemRequirementArray[i].sDecodeMemReq.ui32DecScratchBufSize > 
				sAlgoBufSizes.sDecodeMemReq.ui32DecScratchBufSize)
				sAlgoBufSizes.sDecodeMemReq.ui32DecScratchBufSize =
					sAlgoMemRequirementArray[i].sDecodeMemReq.ui32DecScratchBufSize;
	

			if (sAlgoMemRequirementArray[i].sDecodeMemReq.ui32InterframeBufSize > 
				sAlgoBufSizes.sDecodeMemReq.ui32InterframeBufSize)
				sAlgoBufSizes.sDecodeMemReq.ui32InterframeBufSize = 
					sAlgoMemRequirementArray[i].sDecodeMemReq.ui32InterframeBufSize;

			if (sAlgoMemRequirementArray[i].sDecodeMemReq.ui32DecoderConfigParamStructSize >
				sAlgoBufSizes.sDecodeMemReq.ui32DecoderConfigParamStructSize)
				sAlgoBufSizes.sDecodeMemReq.ui32DecoderConfigParamStructSize = 
					sAlgoMemRequirementArray[i].sDecodeMemReq.ui32DecoderConfigParamStructSize;
		
		}
		else if (eDecodeMode==BRAP_DSPCHN_DecodeMode_ePassThru) {
						
			if(bFwAuthEnable==false)
			{
			
    			if (sAlgoMemRequirementArray[i].ui32PassThruExecSize > 
    				sAlgoBufSizes.ui32PassThruExecSize)
    				sAlgoBufSizes.ui32PassThruExecSize = 
    				sAlgoMemRequirementArray[i].ui32PassThruExecSize;			
			}
			else
			{	
				sAlgoBufSizes.ui32PassThruExecSize += sAlgoMemRequirementArray[i].ui32PassThruExecSize;
			}
			
		}
		

		if (sAlgoMemRequirementArray[i].ui32RingBufSize > 
			sAlgoBufSizes.ui32RingBufSize)
			sAlgoBufSizes.ui32RingBufSize = 
			sAlgoMemRequirementArray[i].ui32RingBufSize;

	}

	if (eDecodeMode==BRAP_DSPCHN_DecodeMode_eDecode)
	{
		psChMemReq->ui32FwCodeSize = sAlgoBufSizes.sDecodeMemReq.ui32FirmwareExecutablesSize;
		psChMemReq->ui32FsSize=ui32FrameSyncSize;				
	}
	else
		psChMemReq->ui32FwCodeSize = sAlgoBufSizes.ui32PassThruExecSize;
    
	psChMemReq->ui32ScratchBufSize = sAlgoBufSizes.sDecodeMemReq.ui32DecScratchBufSize;
	psChMemReq->ui32InterframeBufSize = sAlgoBufSizes.sDecodeMemReq.ui32InterframeBufSize;
	psChMemReq->uiInterstageBufSize = maxInterstageBuf;
	psChMemReq->uiInterstageInterfaceBufSize = maxInterstageInterfaceBuf;
	psChMemReq->ui32DecConfigParamBufSize = sAlgoBufSizes.sDecodeMemReq.ui32DecoderConfigParamStructSize;
	psChMemReq->ui32RingBufSize = sAlgoBufSizes.ui32RingBufSize;


	BDBG_MSG(("Buffer sizes requirement for decode mode %d are\n"
		"Firmware image download buffer size = %d, \n"
		"Scratch Buffer Size = %d, \n"
		"Interframe Buffer Size = %d, \n"
		"Decoder Config Params Buffer Size = %d, \n"
		"Ring Buffer Size = %d\n"
		"Frame sync Size = %d\n"	
		"Interstage Buffer Size = %d,\n"
		"Interstage Interface Buffer Size = %d,\n"
		"ui32PpFwCodeSize = %d\n"
		"ui32PpScratchBufSize = %d\n"
		"ui32PpInterframeBufSize = %d\n"
		"ui32PpConfigParamBufSize = %d\n",
		eDecodeMode,
		psChMemReq->ui32FwCodeSize, 
		psChMemReq->ui32ScratchBufSize, 
		psChMemReq->ui32InterframeBufSize,
		psChMemReq->ui32DecConfigParamBufSize, 
		psChMemReq->ui32RingBufSize,
		psChMemReq->ui32FsSize,
		psChMemReq->uiInterstageBufSize,
		psChMemReq->uiInterstageInterfaceBufSize,
		psChMemReq->ui32PpFwCodeSize,
		psChMemReq->ui32PpScratchBufSize,
		psChMemReq->ui32PpInterframeBufSize,
		psChMemReq->ui32PpConfigParamBufSize));
	
	return (err);

}

#else
BERR_Code BRAP_P_GetDecChannelBufferSizes(
						bool *pbSupportAlgos,	/* [in] pointer to bSupportAlgos[BRAP_MAX_AUDIO_TYPES] array */
						BRAP_DSPCHN_DecodeMode eDecodeMode, /* [in] Decode mode */
#ifdef BCHP_7411_VER
						BRAP_AudioMemAllocation	eAudioMemAllocation, /* [in] Audio memory allocation type */
#endif
						BRAP_DEC_P_ChannelMemReq	*psChMemReq /* [out] Dec channel memory requirement */
#ifndef BCHP_7411_VER			
			, 
			bool bFwAuthEnable	 /*Firmware authentication Enable bit */
#endif			
						)
{
	int i = 0;
	unsigned int temp = 0, maxScratchBuf = 0;
	BRAP_P_AlgoMemRequirement	sAlgoBufSizes;
	BERR_Code err = BERR_SUCCESS;
	unsigned int uiInterstageBufSize = 0;

	BDBG_ASSERT(psChMemReq);
	
	BKNI_Memset(&sAlgoBufSizes, 0, sizeof(BRAP_P_AlgoMemRequirement));
	
	if (eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode) {
		BDBG_ERR(("This function finds buffer sizes only for decode and passthru modes"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	for (i = 0; i < BRAP_MAX_AUDIO_TYPES; i++) {
		/* If algorithm is not selected by application, go to next algorithm */
		if (*(pbSupportAlgos + i)==false)
			continue;
		
		if (eDecodeMode==BRAP_DSPCHN_DecodeMode_eDecode) {	
			
#ifndef BCHP_7411_VER		
			if(bFwAuthEnable==false)
			{
#endif			
			if (sAlgoMemRequirementArray[i].sDecodeMemReq.ui32FirmwareExecutablesSize > 
				sAlgoBufSizes.sDecodeMemReq.ui32FirmwareExecutablesSize)
				sAlgoBufSizes.sDecodeMemReq.ui32FirmwareExecutablesSize = 
					sAlgoMemRequirementArray[i].sDecodeMemReq.ui32FirmwareExecutablesSize;

					sAlgoBufSizes.ui32FrameSyncSize=0;
#ifndef BCHP_7411_VER								
			}
			else
			{
				sAlgoBufSizes.sDecodeMemReq.ui32FirmwareExecutablesSize+=sAlgoMemRequirementArray[i].sDecodeMemReq.ui32FirmwareExecutablesSize;
				sAlgoBufSizes.ui32FrameSyncSize+=sAlgoMemRequirementArray[i].ui32FrameSyncSize;
		
			}
#endif			

#ifdef BCHP_7411_VER
			if ((eAudioMemAllocation==BRAP_AudioMemAllocation_eMultiChannel) 
				|| (eAudioMemAllocation==BRAP_AudioMemAllocation_eMultiChannelSrc)) {
				uiInterstageBufSize = ui32MultiChannelInterstageBufReqArray[i];
			}
			else {
				uiInterstageBufSize = sAlgoMemRequirementArray[i].sDecodeMemReq.ui32InterstageBufSize;
			}
#else
			uiInterstageBufSize = sAlgoMemRequirementArray[i].sDecodeMemReq.ui32InterstageBufSize;
#endif

			temp = uiInterstageBufSize
				+ sAlgoMemRequirementArray[i].sDecodeMemReq.ui32InterstageInterfaceBufSize
				+ sAlgoMemRequirementArray[i].sDecodeMemReq.ui32DecScratchBufSize;

			if (temp > maxScratchBuf)
				maxScratchBuf = temp;

			if (sAlgoMemRequirementArray[i].sDecodeMemReq.ui32InterframeBufSize > 
				sAlgoBufSizes.sDecodeMemReq.ui32InterframeBufSize)
				sAlgoBufSizes.sDecodeMemReq.ui32InterframeBufSize = 
					sAlgoMemRequirementArray[i].sDecodeMemReq.ui32InterframeBufSize;

			if (sAlgoMemRequirementArray[i].sDecodeMemReq.ui32DecoderConfigParamStructSize >
				sAlgoBufSizes.sDecodeMemReq.ui32DecoderConfigParamStructSize)
				sAlgoBufSizes.sDecodeMemReq.ui32DecoderConfigParamStructSize = 
					sAlgoMemRequirementArray[i].sDecodeMemReq.ui32DecoderConfigParamStructSize;
		
			}
		else if (eDecodeMode==BRAP_DSPCHN_DecodeMode_ePassThru) {
#ifndef BCHP_7411_VER						
			if(bFwAuthEnable==false)
			{
#endif			
				if (sAlgoMemRequirementArray[i].ui32PassThruExecSize > sAlgoBufSizes.ui32PassThruExecSize)
				sAlgoBufSizes.ui32PassThruExecSize = sAlgoMemRequirementArray[i].ui32PassThruExecSize;
#ifndef BCHP_7411_VER							
			}
			else
			{
				sAlgoBufSizes.ui32PassThruExecSize += sAlgoMemRequirementArray[i].ui32PassThruExecSize;
			}
#endif			
		}
		

		if (sAlgoMemRequirementArray[i].ui32RingBufSize > 
			sAlgoBufSizes.ui32RingBufSize)
			sAlgoBufSizes.ui32RingBufSize = 
			sAlgoMemRequirementArray[i].ui32RingBufSize;

	}

	if (eDecodeMode==BRAP_DSPCHN_DecodeMode_eDecode)
		psChMemReq->ui32FwCodeSize = sAlgoBufSizes.sDecodeMemReq.ui32FirmwareExecutablesSize;
	else
		psChMemReq->ui32FwCodeSize = sAlgoBufSizes.ui32PassThruExecSize;
	psChMemReq->ui32ScratchBufSize = maxScratchBuf;
	psChMemReq->ui32InterframeBufSize = sAlgoBufSizes.sDecodeMemReq.ui32InterframeBufSize;
	psChMemReq->ui32DecConfigParamBufSize = sAlgoBufSizes.sDecodeMemReq.ui32DecoderConfigParamStructSize;
	psChMemReq->ui32RingBufSize = sAlgoBufSizes.ui32RingBufSize;
	psChMemReq->ui32FsSize=sAlgoBufSizes.ui32FrameSyncSize;

	BDBG_MSG(("Buffer sizes requirement for decode mode %d are\n"
		"Firmware image download buffer size = %d, "
		"Scratch Buffer Size = %d, "
		"Interframe Buffer Size = %d, "
		"Decoder Config Params Buffer Size = %d, "
		"Ring Buffer Size = %d",
		eDecodeMode,
		psChMemReq->ui32FwCodeSize, psChMemReq->ui32ScratchBufSize, 
		psChMemReq->ui32InterframeBufSize, psChMemReq->ui32DecConfigParamBufSize, psChMemReq->ui32RingBufSize));
	
	return (err);

}
#endif /* end of BRAP_DSP_P_7401_NEWMIT*/

#if (BCHP_7411_VER) || (BRAP_7401_FAMILY == 1) || ( BCHP_CHIP == 7400 )
/* BRAP_P_GetPpBufferSizes : This function finds out total memory
    required for combination of post processing algorithms selected by 
    application.
    */
#if (BRAP_DSP_P_7401_NEWMIT==1)
void  BRAP_P_GetPpBufferSizes(
						bool *pbSupportPpAlgos,	/* [in] pointer to bSupportPpAlgos[BRAP_MAX_PP_ALGOS] array */
						BRAP_DEC_P_ChannelMemReq	*psPpMemReq /* [out] Post processing memory requirement */
						)
{
	int i = 0;
	unsigned int maxInterstageBuf = 0, maxInterstageInterfaceBuf = 0;
	BRAP_P_Pp_AlgoMemRequirement	sAlgoBufSizes;
	
	BDBG_ASSERT(pbSupportPpAlgos);
	BSTD_UNUSED(psPpMemReq);

	BKNI_Memset(&sAlgoBufSizes, 0x0, sizeof(BRAP_P_Pp_AlgoMemRequirement));
	BKNI_Memset(psPpMemReq, 0, sizeof(BRAP_DEC_P_ChannelMemReq));
    
	for (i = 0; i < BRAP_MAX_PP_ALGOS; i++) {
		/* If algorithm is not selected by application, go to next algorithm */
		if (*(pbSupportPpAlgos + i)==false)
			continue;
		
		sAlgoBufSizes.sPpMemReq.ui32PpFirmwareExecutablesSize += 
			sPpAlgoMemRequirementArray[i].sPpMemReq.ui32PpFirmwareExecutablesSize;

	if (sPpAlgoMemRequirementArray[i].sPpMemReq.ui32PpInterframeBufSize > 
		sAlgoBufSizes.sPpMemReq.ui32PpInterframeBufSize)
		sAlgoBufSizes.sPpMemReq.ui32PpInterframeBufSize =
			sPpAlgoMemRequirementArray[i].sPpMemReq.ui32PpInterframeBufSize;
	
	if (sPpAlgoMemRequirementArray[i].sPpMemReq.ui32PpInterstageInputBufSize > maxInterstageBuf)
		maxInterstageBuf = sPpAlgoMemRequirementArray[i].sPpMemReq.ui32PpInterstageInputBufSize;
	
	if (sPpAlgoMemRequirementArray[i].sPpMemReq.ui32PpInterstageInputInterfaceBufSize > maxInterstageInterfaceBuf)
		maxInterstageInterfaceBuf = sPpAlgoMemRequirementArray[i].sPpMemReq.ui32PpInterstageInputInterfaceBufSize;
	
	if (sPpAlgoMemRequirementArray[i].sPpMemReq.ui32PpInterstageOutputBufSize > maxInterstageBuf )
		maxInterstageBuf = sPpAlgoMemRequirementArray[i].sPpMemReq.ui32PpInterstageOutputBufSize;
	
	if (sPpAlgoMemRequirementArray[i].sPpMemReq.ui32PpInterstageOutputInterfaceBufSize > maxInterstageInterfaceBuf)
		maxInterstageInterfaceBuf = sPpAlgoMemRequirementArray[i].sPpMemReq.ui32PpInterstageOutputInterfaceBufSize;
	
	if (sPpAlgoMemRequirementArray[i].sPpMemReq.ui32PpScratchBufSize > 
		sAlgoBufSizes.sPpMemReq.ui32PpScratchBufSize)
		sAlgoBufSizes.sPpMemReq.ui32PpScratchBufSize =
			sPpAlgoMemRequirementArray[i].sPpMemReq.ui32PpScratchBufSize;

	sAlgoBufSizes.sPpMemReq.ui32PpConfigParamStructSize += 
		sPpAlgoMemRequirementArray[i].sPpMemReq.ui32PpConfigParamStructSize;

	}

	psPpMemReq->ui32PpFwCodeSize = sAlgoBufSizes.sPpMemReq.ui32PpFirmwareExecutablesSize;
	psPpMemReq->ui32PpScratchBufSize = sAlgoBufSizes.sPpMemReq.ui32PpScratchBufSize;
	psPpMemReq->ui32PpInterframeBufSize = sAlgoBufSizes.sPpMemReq.ui32PpInterframeBufSize;
	psPpMemReq->uiInterstageBufSize = maxInterstageBuf;
	psPpMemReq->uiInterstageInterfaceBufSize = maxInterstageInterfaceBuf;
	psPpMemReq->ui32PpConfigParamBufSize = sAlgoBufSizes.sPpMemReq.ui32PpConfigParamStructSize;

	BDBG_MSG(("Buffer requirements for post processing algorithms \n"
		"Firmware image download buffer size = %d, \n"
		"Scratch Buffer Size = %d, \n"
		"Interframe Buffer Size = %d, \n"
		"Decoder Config Params Buffer Size = %d, \n"
		"Ring Buffer Size = %d\n"
		"Frame sync Size = %d\n"	
		"Interstage Buffer Size = %d,\n"
		"Interstage Interface Buffer Size = %d,\n"
		"ui32PpFwCodeSize = %d\n"
		"ui32PpScratchBufSize = %d\n"
		"ui32PpInterframeBufSize = %d\n"
		"ui32PpConfigParamBufSize = %d\n",
		psPpMemReq->ui32FwCodeSize, 
		psPpMemReq->ui32ScratchBufSize, 
		psPpMemReq->ui32InterframeBufSize,
		psPpMemReq->ui32DecConfigParamBufSize, 
		psPpMemReq->ui32RingBufSize,
		psPpMemReq->ui32FsSize,
		psPpMemReq->uiInterstageBufSize,
		psPpMemReq->uiInterstageInterfaceBufSize,
		psPpMemReq->ui32PpFwCodeSize,
		psPpMemReq->ui32PpScratchBufSize,
		psPpMemReq->ui32PpInterframeBufSize,
		psPpMemReq->ui32PpConfigParamBufSize));
	
	
}
#else
void  BRAP_P_GetPpBufferSizes(
						bool *pbSupportPpAlgos,	/* [in] pointer to bSupportPpAlgos[BRAP_MAX_PP_ALGOS] array */
						BRAP_DEC_P_ChannelMemReq	*psPpMemReq /* [out] Post processing memory requirement */
						)
{
	int i = 0;
	
	BDBG_ASSERT(pbSupportPpAlgos);
	BSTD_UNUSED(pbSupportPpAlgos);

	BKNI_Memset(psPpMemReq, 0x0, sizeof(BRAP_DEC_P_ChannelMemReq));
	for (i = 0; i < BRAP_MAX_PP_ALGOS; i++) {
		psPpMemReq->ui32FwCodeSize += sPpAlgoMemRequirementArray[i].ui32FirmwareExecutablesSize;
	}
}
#endif /* New MIT */




#endif

BERR_Code BRAP_P_OpenOpPathFmmModules
(
	BRAP_ChannelHandle 		hRapCh,			/* [in] The RAP Channel handle */
	BRAP_RBUF_P_Settings	*pRbufSettings,	/* [in] Pointer to an array of 2 
													ring buffer settings */
	BRAP_SRCCH_P_Settings	*pSrcChSettings,/* [in] Source channel settings */
	BRAP_MIXER_P_Settings	*pMixerSettings,/* [in] Mixer settings */
	BRAP_SPDIFFM_P_Settings	*pSpdfFmSettings,/* [in] Spdif Formater settings */
	void					*pOpSettings,	/* [in] Output settings */
	BRAP_RM_P_OpPathResrc	*pOpPathResrc,	/* [in] The FMM Module ids to be opened */
	BRAP_OutputChannelPair	eOpChannel,		/* [in] Output Audio channel type */
	BRAP_P_ObjectHandles	*pModuleHandles /* [out] Internal Module Handles */
)
{
	BERR_Code				ret = BERR_SUCCESS;
	BERR_Code				rc = BERR_SUCCESS;
	int						i;

#ifndef BCHP_7411_VER
	BRAP_OutputChannelPair	eMaxOpChannelPair = BRAP_OutputChannelPair_eDownMixedLR;
#else
	BRAP_OutputChannelPair	eMaxOpChannelPair = BRAP_OutputChannelPair_eCentreLF;
#endif

	BDBG_ENTER(BRAP_P_OpenOpPathFmmModules);

	BDBG_ASSERT(hRapCh);

    if (BRAP_P_GetWatchdogRecoveryFlag (hRapCh->hRap) == false)
    {   
    	/* If not in WatchDog recovery */  
		BDBG_ASSERT(pRbufSettings);
    	BDBG_ASSERT(pSrcChSettings);
    	BDBG_ASSERT(pSpdfFmSettings);
	    BDBG_ASSERT(pOpSettings); 
    } 
	
    BDBG_ASSERT(pMixerSettings);      

	BDBG_ASSERT(pOpPathResrc);
	BDBG_ASSERT(pModuleHandles);


#if (BRAP_7401_FAMILY ==1)	
    if (eOpChannel >= eMaxOpChannelPair)
    {
        BDBG_ERR(("BRAP_P_OpenOpPathFmmModules: Invalid Output Audio channel type!!! "));
        return BERR_TRACE(BERR_INVALID_PARAMETER); 
    }
#else 
    if (eOpChannel > eMaxOpChannelPair)
    {
        BDBG_ERR(("BRAP_P_OpenOpPathFmmModules: Invalid Output Audio channel type!!! "));
        return BERR_TRACE(BERR_INVALID_PARAMETER); 
    }
#endif

	for(i=0; i < BRAP_RM_P_MAX_RBUFS_PER_PORT; i++)
	{
		if(pOpPathResrc->uiRbufId[i] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
		{
			/* Instantiate the ring buffer */
			ret = BRAP_RBUF_P_Open (
					pModuleHandles->hFmm,
					&(pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i]),
					pOpPathResrc->uiRbufId[i],
					&pRbufSettings[i]);
			if(ret != BERR_SUCCESS){goto end_fmm_open;}
			BDBG_MSG(("Ring buffer %d opened", pOpPathResrc->uiRbufId[i]));
		}
		else
		{
			/* RBuf is not needed for current channel */
			pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i] = NULL;
		}
	}


	if(pOpPathResrc->uiSrcChId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
	{
		/* Instantiate the Source Channel */
		ret = BERR_TRACE(BRAP_SRCCH_P_Open (
				pModuleHandles->hFmm,
				&(pModuleHandles->hSrcCh[eOpChannel]),
				pOpPathResrc->uiSrcChId,
				pSrcChSettings));
		if(ret != BERR_SUCCESS) {goto close_rbuf;}
		BDBG_MSG(("Source channel %d opened", pOpPathResrc->uiSrcChId));
	}
	else
		pModuleHandles->hSrcCh[eOpChannel] = NULL;


    /* Fill up the mixer input id */
	pModuleHandles->uiMixerInputIndex[eOpChannel] = pOpPathResrc->uiMixerInputId;
	
	if(pOpPathResrc->uiMixerId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
	{
		/* Instantiate the Mixer */
		ret = BERR_TRACE(BRAP_MIXER_P_Open (
				pModuleHandles->hFmm,
				&(pModuleHandles->hMixer[eOpChannel]),
				pOpPathResrc->uiMixerId,
				pMixerSettings));
		if(ret != BERR_SUCCESS){goto close_srcch;}
		BDBG_MSG(("Mixer %d - inpur %d opened", 
				  pOpPathResrc->uiMixerId, 
				  pOpPathResrc->uiMixerInputId));
	}
	else
		pModuleHandles->hMixer[eOpChannel] = NULL;

    if(pOpPathResrc->uiSpdiffmStreamId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
	{
		/* SPDIF Formater is used */

		/* Instantiate the Micro-sequencer */
		/* Instantiate the Micro-sequencer */
		ret = BERR_TRACE(BRAP_SPDIFFM_P_Open (
				pModuleHandles->hFmm,
				&(pModuleHandles->hSpdifFm[eOpChannel]),
				pOpPathResrc->uiSpdiffmStreamId,
				pSpdfFmSettings));
		if(ret != BERR_SUCCESS){goto close_mixer;}
		BDBG_MSG(("Spdiffm %d - stream %d opened", 
				  pOpPathResrc->uiSpdiffmId, 
				  pOpPathResrc->uiSpdiffmStreamId));
	}
	else
	{
		pModuleHandles->hSpdifFm[eOpChannel] = NULL;
	}

	if(pOpPathResrc->eOutputPortType != (unsigned int)BRAP_RM_P_INVALID_INDEX)
	{
			/* Instantiate the output port */
			ret = BERR_TRACE(BRAP_OP_P_Open(pModuleHandles->hFmm,
								 &(pModuleHandles->hOp[eOpChannel]),
								 pOpPathResrc->eOutputPortType,
								 pOpSettings));
			if(ret != BERR_SUCCESS){goto close_spdiffm;}
			BDBG_MSG(("Output port type %d opened", 
					  pOpPathResrc->eOutputPortType));


	}
	else
	{
		pModuleHandles->hOp[eOpChannel] = NULL;
	}

	goto end_fmm_open;

close_spdiffm:
	/* Close the SPDIF Formator objects */
	if(pModuleHandles->hSpdifFm[eOpChannel] != NULL)
	{
		rc = BERR_TRACE(BRAP_SPDIFFM_P_Close(pModuleHandles->hSpdifFm[eOpChannel]));
	        if (rc != BERR_SUCCESS)
	        {
	            BDBG_ERR (("BRAP_P_OpenOpPathFmmModules: call to BRAP_SPDIFFM_P_Close() failed. Ignoring error!!!!!"));
	        }           
		BDBG_MSG(("Spdiffm %d - stream %d closed", 
				  pModuleHandles->hSpdifFm[eOpChannel]->uiIndex, 
				  pModuleHandles->hSpdifFm[eOpChannel]->uiStreamIndex));
	}
	
close_mixer:
	/* Close the Mixer objects */
	if(pModuleHandles->hMixer[eOpChannel] != NULL)
	{
		rc = BERR_TRACE(BRAP_MIXER_P_Close(pModuleHandles->hMixer[eOpChannel], 
								pOpPathResrc->uiMixerInputId));
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR (("BRAP_P_OpenOpPathFmmModules: call to BRAP_MIXER_P_Close() failed. Ignoring error!!!!!"));
        }     
		BDBG_MSG(("Mixer %d - input %d closed", 
				  pModuleHandles->hMixer[eOpChannel]->uiMixerIndex, 
				  pOpPathResrc->uiMixerInputId));
	}
	
close_srcch:
	/* Close the Source Channel objects */
	if(pModuleHandles->hSrcCh[eOpChannel] != NULL)
	{
		rc = BERR_TRACE(BRAP_SRCCH_P_Close(pModuleHandles->hSrcCh[eOpChannel]));
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR (("BRAP_P_OpenOpPathFmmModules: call to BRAP_SRCCH_P_Close() failed. Ignoring error!!!!!"));
        }     
		BDBG_MSG(("Source channel %d closed", 
				  pModuleHandles->hSrcCh[eOpChannel]->uiIndex));
	}
	
close_rbuf:
	/* Close the Ring buffer objects */
	for(i=0; i < BRAP_RM_P_MAX_RBUFS_PER_PORT; i++)
	{
		if(pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i] != NULL)
		{
			rc = BERR_TRACE(BRAP_RBUF_P_Close(pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i]));
            if (rc != BERR_SUCCESS)
            {
                BDBG_ERR (("BRAP_P_OpenOpPathFmmModules: call to BRAP_RBUF_P_Close() failed. Ignoring error!!!!!"));
            }  
			BDBG_MSG(("Ring buffer %d closed", 
					  pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i]->uiIndex));
		}
	}
	
end_fmm_open:
	BDBG_LEAVE(BRAP_P_OpenOpPathFmmModules);
	
	return (ret);
}

BERR_Code BRAP_P_CloseOpPathFmmModules
(
	BRAP_OutputChannelPair	eOpChannel,		/* [in] Output Audio channel type */
	BRAP_P_ObjectHandles	*pModuleHandles /* [in] Internal Module Handles */
)
{
	BERR_Code ret = BERR_SUCCESS;
	unsigned int i=0;

#ifndef BCHP_7411_VER
	BRAP_OutputChannelPair	eMaxOpChannelPair = BRAP_OutputChannelPair_eDownMixedLR;
#else
	BRAP_OutputChannelPair	eMaxOpChannelPair = BRAP_OutputChannelPair_eCentreLF;
#endif
	

	BDBG_ENTER(BRAP_P_CloseOpPathFmmModules);

	BDBG_ASSERT(pModuleHandles);
#if (BRAP_7401_FAMILY ==1)	
    if (eOpChannel >= eMaxOpChannelPair)
    {
        BDBG_ERR(("BRAP_P_CloseOpPathFmmModules: Invalid Output Audio channel type!!! "));
        return BERR_TRACE(BERR_INVALID_PARAMETER); 
    }
#else 
    if (eOpChannel > eMaxOpChannelPair)
    {
        BDBG_ERR(("BRAP_P_CloseOpPathFmmModules: Invalid Output Audio channel type!!! "));
        return BERR_TRACE(BERR_INVALID_PARAMETER); 
    }
#endif
	
	/* Close the outputs */
	if(pModuleHandles->hOp[eOpChannel] != NULL)
	{
		ret = BERR_TRACE(BRAP_OP_P_Close(pModuleHandles->hOp[eOpChannel]));
		if(ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_P_CloseOpPathFmmModules: call to BRAP_OP_P_Close() failed!!! Ignoring error!!! "));
        }
		BDBG_MSG(("Output port type %d closed", 
				  pModuleHandles->hOp[eOpChannel]->eOutputPort));
		pModuleHandles->hOp[eOpChannel] = NULL;
	}

	/* Close the SPDIF Formaters */
	if(pModuleHandles->hSpdifFm[eOpChannel] != NULL)
	{
		ret = BERR_TRACE(BRAP_SPDIFFM_P_Close(pModuleHandles->hSpdifFm[eOpChannel]));
		if(ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_P_CloseOpPathFmmModules: call to BRAP_SPDIFFM_P_Close() failed!!! Ignoring error!!! "));
        }
		BDBG_MSG(("Spdiffm %d - stream %d closed", 
				  pModuleHandles->hSpdifFm[eOpChannel]->uiIndex, 
				  pModuleHandles->hSpdifFm[eOpChannel]->uiStreamIndex));
		pModuleHandles->hSpdifFm[eOpChannel] = NULL;
	}

	/* Close the Mixers */
	if(pModuleHandles->hMixer[eOpChannel] != NULL)
	{
		ret = BERR_TRACE(BRAP_MIXER_P_Close(pModuleHandles->hMixer[eOpChannel], 
								 pModuleHandles->uiMixerInputIndex[eOpChannel]));
		if(ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_P_CloseOpPathFmmModules: call to BRAP_MIXER_P_Close() failed!!! Ignoring error!!! "));
        }
		BDBG_MSG(("Mixer %d - input %d closed", 
				  pModuleHandles->hMixer[eOpChannel]->uiMixerIndex, 
				  pModuleHandles->uiMixerInputIndex[eOpChannel]));
		pModuleHandles->hMixer[eOpChannel] = NULL;
	}

	/* Close the source channels */
	if(pModuleHandles->hSrcCh[eOpChannel] != NULL)
	{
		ret = BERR_TRACE(BRAP_SRCCH_P_Close(pModuleHandles->hSrcCh[eOpChannel]));
		if(ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_P_CloseOpPathFmmModules: call to BRAP_SRCCH_P_Close() failed!!! Ignoring error!!! "));
        }
		BDBG_MSG(("Source channel %d closed", 
				  pModuleHandles->hSrcCh[eOpChannel]->uiIndex));
		pModuleHandles->hSrcCh[eOpChannel] = NULL;
	}

	/* Close the ring buffers */
	for(i=0; i < BRAP_RM_P_MAX_RBUFS_PER_PORT; i++)    
	{
		if(pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i] != NULL)
		{
			ret = BERR_TRACE(BRAP_RBUF_P_Close(pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i]));
    		if(ret != BERR_SUCCESS)
            {
                BDBG_ERR(("BRAP_P_CloseOpPathFmmModules: call to BRAP_RBUF_P_Close() failed!!! Ignoring error!!! "));
            }
			BDBG_MSG(("Ring buffer %d closed", 
					  pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i]->uiIndex));
			pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i] = NULL;
		}
	}
	BDBG_LEAVE(BRAP_P_CloseOpPathFmmModules);
	
	return (ret);
}

/*
 The entire DEC_Start sequence is:

1. Mute associate channel in DP (was Muted in _Stop()) 

2. Start the associated Output operation

3. Start the associated Microsequencer operation 

4. Start the associated Mixer operation

5. Unmute the channel (taken care of in BRAP_MIXER_P_Start() which starts with SCALE_MUTE_ENA & VOLUME_MUTE_ENA both unMuted)

6. Start the associated Ring buffers' operation

7. Start the associated Source FIFOs' operation

8. Start the associated DSP Context operation (Program audio PLL, open the gate)


*/    

BERR_Code BRAP_P_StartOpPathFmmModules
(
	BRAP_ChannelHandle 		hRapCh,			/* [in] The RAP Channel handle */
	BRAP_RBUF_P_Params		*pRBufParams,	/* [in] Pointer to an array of 2 
													ring buffer settings */
	BRAP_SRCCH_P_Params		*pSrcChParams,	/* [in] Source channel start params */
	BRAP_MIXER_P_Params		*pMixerParams,	/* [in] Mixer start params */
	BRAP_SPDIFFM_P_Params	*pSpdifFmParams,/* [in] Spdif Formater start params */
	void					*pOpParams,		/* [in] Output params */
	BRAP_OutputChannelPair	eOpChannel,		/* [in] Output Audio channel type */
	BRAP_P_ObjectHandles	*pModuleHandles /* [in] Internal Module Handles */
)
{
	BERR_Code				ret = BERR_SUCCESS;
	int						i;
#ifndef BCHP_7411_VER
	BRAP_OutputChannelPair	eMaxOpChannelPair = BRAP_OutputChannelPair_eDownMixedLR;
#else
	BRAP_OutputChannelPair	eMaxOpChannelPair = BRAP_OutputChannelPair_eCentreLF;
#endif
	
	BDBG_ENTER(BRAP_P_StartOpPathFmmModules);

	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(pModuleHandles);
    
    if (BRAP_P_GetWatchdogRecoveryFlag (hRapCh->hRap) == false)
    {   /* If not in WatchDog recovery */  
	    BDBG_ASSERT(pRBufParams);
    	BDBG_ASSERT(pSrcChParams);
	    BDBG_ASSERT(pMixerParams);
    	BDBG_ASSERT(pSpdifFmParams);
	    BDBG_ASSERT(pOpParams);
    }    

    if (eOpChannel > eMaxOpChannelPair)
    {
        BDBG_ERR(("BRAP_P_StartOpPathFmmModules: Invalid Output Audio channel type!!! "));
        return BERR_TRACE(BERR_INVALID_PARAMETER); 
    }

	/* Start the associated output block */
	if(pModuleHandles->hOp[eOpChannel] != NULL)
	{
		ret = BERR_TRACE(BRAP_OP_P_Start(pModuleHandles->hOp[eOpChannel], 
							  pOpParams));
		if(ret != BERR_SUCCESS){goto end_fmm_start;}
		BDBG_MSG(("Output port type %d started", 
				  pModuleHandles->hOp[eOpChannel]->eOutputPort));
	}
	
	/* TODO: Do Output alignment if required. It is not needed for stereo. */
	
	/* Start the associated SPDIF Formaters */
	if(pModuleHandles->hSpdifFm[eOpChannel] != NULL)
	{
	    pSpdifFmParams->eSamplingRate = pModuleHandles->hOp[eOpChannel]->eSamplingRate;
		/* Start the SPDIF Formater */
		ret = BERR_TRACE(BRAP_SPDIFFM_P_Start(	hRapCh, 
									pModuleHandles->hSpdifFm[eOpChannel], 
									pSpdifFmParams));
		if(ret != BERR_SUCCESS){goto stop_op;}
		BDBG_MSG(("Spdiffm %d - stream %d started", 
				  pModuleHandles->hSpdifFm[eOpChannel]->uiIndex, 
				  pModuleHandles->hSpdifFm[eOpChannel]->uiStreamIndex));
	}

	if(hRapCh->eChannelType != BRAP_P_ChannelType_eDecode)
	{
		int ui32RegVal=0x0;

	    ui32RegVal = BRAP_Read32 (hRapCh->hRegister,BCHP_AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP);
		
	    ui32RegVal &= ~(BCHP_MASK (    
	                    AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP, 
	                    SCALE_RAMP_STEP_SIZE));
	    
	    ui32RegVal |= (BCHP_FIELD_DATA (    
	                        AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP, 
	                        SCALE_RAMP_STEP_SIZE, 
	                        0x20));

	    BRAP_Write32 (hRapCh->hRegister, BCHP_AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP, ui32RegVal);
#if ( BCHP_CHIP == 7400 )
	    BRAP_Write32 (hRapCh->hRegister, BCHP_AUD_FMM_DP_CTRL1_FMM_SCALE_VOL_STEP, ui32RegVal);
#endif		
	}
		
	/* Start the associated mixer */
	if(pModuleHandles->hMixer[eOpChannel] != NULL)
	{
		ret = BERR_TRACE(BRAP_MIXER_P_Start(pModuleHandles->hMixer[eOpChannel], 
								 pModuleHandles->uiMixerInputIndex[eOpChannel], 
								 pMixerParams));
		if(ret != BERR_SUCCESS){goto stop_spdiffm;}
		BDBG_MSG(("Mixer %d - input %d started", 
				  pModuleHandles->hMixer[eOpChannel]->uiMixerIndex, 
				  pModuleHandles->uiMixerInputIndex[eOpChannel]));
	}
    /* Start the associated ring buffers */
    
    for(i=0; i < BRAP_RM_P_MAX_RBUFS_PER_PORT; i++)
    {
        if(pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i] != NULL)
        {
    
            /* Start the ring buffer */
            ret = BERR_TRACE(BRAP_RBUF_P_Start(pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i], 
                                    &pRBufParams[i]));
            if(ret != BERR_SUCCESS){goto stop_mixer;}
            BDBG_MSG(("Ring buffer %d started", 
                      pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i]->uiIndex));

	    /* If it is a playback channel then we need to program the end address properly based
	    on the size given during start time */
	    if ( hRapCh->eChannelType == BRAP_P_ChannelType_ePcmPlayback )
	    {
	    	ret = BERR_TRACE(BRAP_RBUF_P_ProgramEndAddress(
				pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i]));
		if(ret != BERR_SUCCESS){goto stop_rbuf;}
		BDBG_MSG(("Ring buffer %d End address programmed", 
		          pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i]->uiIndex));

	    }
        }
    }
#ifndef RAP_SRSTRUVOL_CERTIFICATION 	
	/* Start the associated source channels */
	if(pModuleHandles->hSrcCh[eOpChannel] != NULL)
	{
		ret = BERR_TRACE(BRAP_SRCCH_P_Start(pModuleHandles->hSrcCh[eOpChannel],
								 pSrcChParams));
		if(ret != BERR_SUCCESS){goto stop_rbuf;}
		BDBG_MSG(("Source channel %d started", 
				  pModuleHandles->hSrcCh[eOpChannel]->uiIndex));
	}
#endif	

	goto end_fmm_start;
	
stop_rbuf:
	/* Stop the Ring buffer objects */
	for(i=0; i < BRAP_RM_P_MAX_OP_CHANNELS; i++)
	{
		if(pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i] != NULL)
		{
			ret = BERR_TRACE(BRAP_RBUF_P_Stop(pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i]));
		    if(ret != BERR_SUCCESS)
            {
                BDBG_ERR(("BRAP_P_StartOpPathFmmModules: call to BRAP_RBUF_P_Stop() failed!!! Ignoring error!!! "));
            }
			BDBG_MSG(("Ring buffer %d stopped", 
					  pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i]->uiIndex));
		}
	}
	
stop_mixer:
	/* Stop the Mixer objects */
	if(pModuleHandles->hMixer[eOpChannel] != NULL)
	{
		ret = BERR_TRACE(BRAP_MIXER_P_Stop(pModuleHandles->hMixer[eOpChannel], 
								pModuleHandles->uiMixerInputIndex[eOpChannel]));
	    if(ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_P_StartOpPathFmmModules: call to BRAP_MIXER_P_Stop() failed!!! Ignoring error!!! "));
        }
		BDBG_MSG(("Mixer %d - input %d stopped", 
				  pModuleHandles->hMixer[eOpChannel]->uiMixerIndex, 
				  pModuleHandles->uiMixerInputIndex[eOpChannel]));
	}
	
stop_spdiffm:
	if(pModuleHandles->hSpdifFm[eOpChannel] != NULL)
	{
		/* Stop the Spdif Formater objects */
		ret = BERR_TRACE(BRAP_SPDIFFM_P_Stop(pModuleHandles->hSpdifFm[eOpChannel]));
	    if(ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_P_StartOpPathFmmModules: call to BRAP_SPDIFFM_P_Stop() failed!!! Ignoring error!!! "));
        }
		BDBG_MSG(("Spdiffm %d - stream %d stopped", 
				  pModuleHandles->hSpdifFm[eOpChannel]->uiIndex, 
				  pModuleHandles->hSpdifFm[eOpChannel]->uiStreamIndex));
	}
	
stop_op:
	if(pModuleHandles->hOp[eOpChannel] != NULL)
	{
		/* Stop the Output objects */
		ret = BERR_TRACE(BRAP_OP_P_Stop(pModuleHandles->hOp[eOpChannel]));
	    if(ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_P_StartOpPathFmmModules: call to BRAP_OP_P_Stop() failed!!! Ignoring error!!! "));
        }
		BDBG_MSG(("Output port type %d stopped", 
				  pModuleHandles->hOp[eOpChannel]->eOutputPort));
	}
	
end_fmm_start:
	
	BDBG_LEAVE(BRAP_P_StartOpPathFmmModules);
	
	return (ret);
}
/*

    The entire DEC_Stop sequence is
    
1a. (PCM mode)Stop the associated DSP Context operation(stop decoding but ring buffer still playing)

1b. (Compress SPDIF mode)Stop the associated DSP Context operation, insert one pause/null burst in ring buffer

2a. (PCM mode)Mute associated PCM output stream in DP and wait for DP volume ramp down done interrupt  

2b. (Compressed SPDIF mode)Play until ring buffer empty 

4. *Stop the associated Microsequencer operation

5. Stop the associated Mixer operation

3. *Stop the associated Output operation.

5. Stop the associated Source FIFOs' operation(close gate)

6. Stop the associated Ring buffers' operation

At step 2 ring buffer should have more than 1 frame of data left.

*Never shut down SPDIF output.

For solving the PR 23121 we need to disable the DP first because during the next start up of multichannel 
	  with alignment between pairs can no longer be guaranteed due to the leftover sample in DP. It looks like the
	 only way to flush data out of DP is to leave the corresponding OP stream enabled for at least one additional sample after
	 DP is disabled. If this happens, then on the next restart, all streams will be waiting for the correct first sample
	 like they are the first time.


*/
BERR_Code BRAP_P_StopOpPathFmmModules
(
	BRAP_OutputChannelPair	eOpChannel,		/* [in] Output Audio channel type */
	BRAP_P_ObjectHandles	*pModuleHandles /* [in] Internal Module Handles */
)
{
	BERR_Code	ret = BERR_SUCCESS;
	int			i;
#ifndef BCHP_7411_VER
	BRAP_OutputChannelPair	eMaxOpChannelPair = BRAP_OutputChannelPair_eDownMixedLR;
#else
	BRAP_OutputChannelPair	eMaxOpChannelPair = BRAP_OutputChannelPair_eCentreLF;
#endif

	BDBG_ENTER(BRAP_P_StopOpPathFmmModules);

	BDBG_ASSERT(pModuleHandles);

#if (BRAP_7401_FAMILY ==1)	
    if (eOpChannel >= eMaxOpChannelPair)
    {
        BDBG_ERR(("BRAP_P_StopOpPathFmmModules: Invalid Output Audio channel type!!! "));
        return BERR_TRACE(BERR_INVALID_PARAMETER); 
    }
#else 
    if (eOpChannel > eMaxOpChannelPair)
    {
        BDBG_ERR(("BRAP_P_StopOpPathFmmModules: Invalid Output Audio channel type!!! "));
        return BERR_TRACE(BERR_INVALID_PARAMETER); 
    }
#endif


#if 1 
	if((pModuleHandles->hMixer[eOpChannel] != NULL)
            && (pModuleHandles->hSrcCh[eOpChannel] !=NULL) 
            && (pModuleHandles->hDspCh !=NULL))
    {
        /* We can mute/unmute only for uncompressed data */    
        if (pModuleHandles->hSrcCh[eOpChannel]->sParams.bCompress == false)        
        { 
    	    /* Mute the channel */ 
	    	ret = BRAP_SRCCH_P_SetMute(pModuleHandles->hSrcCh[eOpChannel], 
									   pModuleHandles->hMixer[eOpChannel], 
									   true);
		    if(ret != BERR_SUCCESS)
    		{
	    		return BERR_TRACE(ret);
		    }

            if (pModuleHandles->hDspCh->bDecLocked ==true)
            {
    			/* Wait for ramp done only if decoder is locked.*/
            ret = BERR_TRACE(BRAP_SRCCH_P_WaitForRampDown (pModuleHandles->hSrcCh[eOpChannel], 
										pModuleHandles->hMixer[eOpChannel]));
            if (ret != BERR_SUCCESS)
            {
                BDBG_ERR (("BRAP_P_StopOpPathFmmModules: call to BRAP_SRCCH_P_WaitForRampDown() failed. Ignoring error!!!!!"));
            }            
            }
           
        }
    }
#endif
    
	/* Stop the SPDIF Formater */
	if(pModuleHandles->hSpdifFm[eOpChannel] != NULL)
	{
		ret = BERR_TRACE(BRAP_SPDIFFM_P_Stop(pModuleHandles->hSpdifFm[eOpChannel]));
	    if(ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_P_StopOpPathFmmModules: call to BRAP_SPDIFFM_P_Stop() failed!!! Ignoring error!!! "));
        }
		BDBG_MSG(("Spdiffm %d - stream %d stopped", 
				  pModuleHandles->hSpdifFm[eOpChannel]->uiIndex, 
				  pModuleHandles->hSpdifFm[eOpChannel]->uiStreamIndex));
	}

	/* Stop the Mixer */
	if(pModuleHandles->hMixer[eOpChannel] != NULL)
	{
		ret = BERR_TRACE(BRAP_MIXER_P_Stop(pModuleHandles->hMixer[eOpChannel],	
								pModuleHandles->uiMixerInputIndex[eOpChannel]));
	    if(ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_P_StopOpPathFmmModules: call to BRAP_MIXER_P_Stop() failed!!! Ignoring error!!! "));
        }
		BDBG_MSG(("Mixer %d - input %d stopped", 
				  pModuleHandles->hMixer[eOpChannel]->uiMixerIndex, 
				  pModuleHandles->uiMixerInputIndex[eOpChannel]));
	}

	/* For solving the PR 23121 we need to disable the DP first because during the next start up of multichannel 
	* with alignment between pairs can no longer be guaranteed due to the leftover sample in DP. It looks like the
	* only way to flush data out of DP is to leave the corresponding OP stream enabled for at least one additional sample after
	* DP is disabled. If this happens, then on the next restart, all streams will be waiting for the correct first sample
	* like they are the first time.
	*/
	/* Stop the output */
	if(pModuleHandles->hOp[eOpChannel] != NULL)
	{
		ret = BERR_TRACE(BRAP_OP_P_Stop(pModuleHandles->hOp[eOpChannel]));
	    if(ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_P_StopOpPathFmmModules: call to BRAP_OP_P_Stop() failed!!! Ignoring error!!! "));
        }
		BDBG_MSG(("Output port type %d stopped", 
				  pModuleHandles->hOp[eOpChannel]->eOutputPort));
	}

	BKNI_Delay(3000);  /*Added 3msec delay - PR38305*/

#ifndef RAP_SRSTRUVOL_CERTIFICATION 
	/* Stop the source channel */
	if(pModuleHandles->hSrcCh[eOpChannel] != NULL)
	{
		ret = BERR_TRACE(BRAP_SRCCH_P_Stop(pModuleHandles->hSrcCh[eOpChannel]));
	    if(ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_P_StopOpPathFmmModules: call to BRAP_SRCCH_P_Stop() failed!!! Ignoring error!!! "));
        }
		BDBG_MSG(("Source channel %d stopped", 
				  pModuleHandles->hSrcCh[eOpChannel]->uiIndex));
	}
#endif

	/* Stop the ring buffers */
	for(i=0; i < BRAP_RM_P_MAX_RBUFS_PER_PORT; i++)
	{
		if(pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i] != NULL)
		{
			ret = BERR_TRACE(BRAP_RBUF_P_Stop(pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i]));
	        if(ret != BERR_SUCCESS)
            {
                BDBG_ERR(("BRAP_P_StopOpPathFmmModules: call to BRAP_RBUF_P_Stop() failed!!! Ignoring error!!! "));
            }
			BDBG_MSG(("Ring buffer %d stopped", 
					  pModuleHandles->hRBuf[eOpChannel * BRAP_RM_P_MAX_RBUFS_PER_PORT + i]->uiIndex));
		}
	}
	

	
	BDBG_LEAVE(BRAP_P_StopOpPathFmmModules);
	
	return (ret);
}




BERR_Code BRAP_P_FormOpParams
(
	BRAP_ChannelHandle     hRapCh,		/* [in] The RAP decode channel handle */ 
	const BRAP_OutputPort	    eOutputPort,/* [in] Output port */
	BAVC_Timebase                   eTimebase, /* [in] Timebase */
	unsigned int uiOutputBitsPerSample, /* [in] Bits per sample */
	void * * 			pParamAddr		/* [out] pointer of type void * to 
                                            return a pointer to the output parameter struct */	
)
{
	BERR_Code				ret = BERR_SUCCESS;

    BDBG_MSG(("hRapCh=0x%x", hRapCh));
    BDBG_MSG(("eOutputPort=0x%x", eOutputPort));
    BDBG_MSG(("eTimebase=0x%x", eTimebase));
    BDBG_MSG(("uiOutputBitsPerSample=0x%x", uiOutputBitsPerSample));
    BDBG_MSG(("pParamAddr=0x%x", pParamAddr));

		/* Output block params */
		switch(eOutputPort)
		{
			case BRAP_OutputPort_eSpdif:
                            BRAP_OP_P_GetDefaultParams (BRAP_OutputPort_eSpdif, (void *)&sSpdifParams);
				sSpdifParams.eTimebase = eTimebase;
				*pParamAddr = &sSpdifParams;
				break;
			case BRAP_OutputPort_eI2s0:
			case BRAP_OutputPort_eI2s1:
			case BRAP_OutputPort_eI2s2:
                            BRAP_OP_P_GetDefaultParams (eOutputPort, (void *)&sI2sParams);               
				sI2sParams.eTimebase = eTimebase;
				sI2sParams.uiBitsPerSample 
					= uiOutputBitsPerSample;
				*pParamAddr = &sI2sParams;
				break;
			case BRAP_OutputPort_eMai:
                            BRAP_OP_P_GetDefaultParams (BRAP_OutputPort_eMai, (void *)&sMaiParams);               
				sMaiParams.uiSampleWidth 
					= uiOutputBitsPerSample;
                            sMaiParams.eTimebase = eTimebase;
				*pParamAddr = &sMaiParams;
				/* Populate the SPDIF settings properly if MAI input is SPDIF */
				if (hRapCh->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector 
                        == BRAP_OutputPort_eSpdif )
				{
					hRapCh->sModuleHandles.hFmm->hOp[BRAP_OutputPort_eSpdif]->
						uOpParams.sSpdif.eTimebase = eTimebase;
				}
				else if ( hRapCh->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector 
                            == BRAP_OutputPort_eFlex)
				{
					hRapCh->sModuleHandles.hFmm->hOp[BRAP_OutputPort_eFlex]->
						uOpParams.sFlex.eTimebase = eTimebase;
				}
				else
				{
					BDBG_ERR(("Invalid MAI mux selector port "));
				}
				break;
			case BRAP_OutputPort_eFlex:
				sFlexParams.eTimebase = eTimebase;			
				*pParamAddr = &sFlexParams;	
				break;
			case BRAP_OutputPort_eDac0:
			case BRAP_OutputPort_eDac1:
                            BRAP_OP_P_GetDefaultParams (eOutputPort, (void *)&sDacParams); 
				sDacParams.eTimebase = eTimebase;
				*pParamAddr = &sDacParams;		
				break;
			default:
				BDBG_ERR(("BRAP_P_FormOpParams: Output port type %d not supported", 
						  eOutputPort));
				ret = BERR_TRACE(BRAP_ERR_OUPUT_PORT_NOT_SUPPORTED);
		}

	BDBG_LEAVE (BRAP_P_FormOpParams);
	return (ret);

}



/* BRAP_P_MuteChannelOutputOnSr_isr: Mutes the output ports associated with
 * a decode channel at output port level on sample rate change. Returns the
 * previous mute status of output ports in psOpPortPrevMuteStatus.
 */
BERR_Code 
BRAP_P_MuteChannelOutputOnSr_isr
(
	BRAP_ChannelHandle hRapCh,		/* [in] The RAP decode channel handle */ 
	BRAP_P_OpPortPrevMuteStatus *psOpPortPrevMuteStatus /* [out] Returns previous mute states of output ports */
)
{
	unsigned int count;
	BERR_Code rc = BERR_SUCCESS;
#ifndef 	BCHP_7411_VER
	unsigned int i;
#endif

	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(psOpPortPrevMuteStatus);

	BKNI_Memset_isr(psOpPortPrevMuteStatus, 0x0, sizeof(BRAP_P_OpPortPrevMuteStatus));

	for (count = 0; count < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; count++) {
		if (hRapCh->sModuleHandles.hOp[count]) {
			rc = BRAP_OP_GetMute(hRapCh->hRap, hRapCh->sModuleHandles.hOp[count]->eOutputPort, 
					&psOpPortPrevMuteStatus->bDecMuteStatus[count]);
			if (rc != BERR_SUCCESS)
				return BERR_TRACE(rc);
			rc = BRAP_OP_P_SetMute_isr(hRapCh->hRap, hRapCh->sModuleHandles.hOp[count]->eOutputPort, true);
			if (rc != BERR_SUCCESS)
				return BERR_TRACE(rc);
			BDBG_MSG(("Muted outputport %d, mute status = %d", hRapCh->sModuleHandles.hOp[count]->eOutputPort, 
				psOpPortPrevMuteStatus->bDecMuteStatus[count]));
		}
#ifdef BCHP_7411_VER  /* only for 7411 */    	
		if (hRapCh->eClone!=BRAP_P_CloneState_eInvalid) {
#endif     
			if (hRapCh->sSimulPtModuleHandles.hOp[count]) {
				rc = BRAP_OP_GetMute(hRapCh->hRap, hRapCh->sSimulPtModuleHandles.hOp[count]->eOutputPort, 
						&psOpPortPrevMuteStatus->bSimulPtMuteStatus[count]);
				if (rc != BERR_SUCCESS)
					return BERR_TRACE(rc);
				rc = BRAP_OP_P_SetMute_isr(hRapCh->hRap, hRapCh->sSimulPtModuleHandles.hOp[count]->eOutputPort, true);
				if (rc != BERR_SUCCESS)
					return BERR_TRACE(rc);
				BDBG_MSG(("Muted simulpt outputport %d, mute status = %d", hRapCh->sSimulPtModuleHandles.hOp[count]->eOutputPort, 
				psOpPortPrevMuteStatus->bSimulPtMuteStatus[count]));
			}
#ifdef BCHP_7411_VER  /* only for 7411 */                  
		}	
#endif       
#ifndef 	BCHP_7411_VER /* Other than 7411 chips */
		for (i = 0; i < BRAP_RM_P_MAX_OUTPUTS; i++) {
			if (hRapCh->sCloneOpPathHandles[count][i].hOp) {
				rc = BRAP_OP_GetMute(hRapCh->hRap, hRapCh->sCloneOpPathHandles[count][i].hOp->eOutputPort, 
						&psOpPortPrevMuteStatus->bCloneMuteStates[count][i]);
				if (rc != BERR_SUCCESS)
					return BERR_TRACE(rc);
				rc = BRAP_OP_P_SetMute_isr(hRapCh->hRap, hRapCh->sCloneOpPathHandles[count][i].hOp->eOutputPort, true);
				if (rc != BERR_SUCCESS)
					return BERR_TRACE(rc);
				BDBG_MSG(("Muted cloned outputport %d, mute status = %d", hRapCh->sCloneOpPathHandles[count][i].hOp->eOutputPort, 
					psOpPortPrevMuteStatus->bCloneMuteStates[count][i]));
			}
		}
#endif
	}
	return rc;
}

/* BRAP_P_UnMuteChannelOutputOnSr_isr: Unmutes the output ports associated with
 * a decode channel at output port level on sample rate change. Unmutes only those
 * output ports for which previous mute status was unmute. Previous mute states of
 * output ports are passed in parameter psOpPortPrevMuteStatus.
 */
BERR_Code 
BRAP_P_UnMuteChannelOutputOnSr_isr
(
	BRAP_ChannelHandle hRapCh, 		/* [in] The RAP decode channel handle */ 
	BRAP_P_OpPortPrevMuteStatus *psOpPortPrevMuteStatus /* [in] Previous mute states of output ports */
)
{
	unsigned int count;
	BERR_Code rc = BERR_SUCCESS;
#ifndef 	BCHP_7411_VER
	unsigned int i;
#endif

	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(psOpPortPrevMuteStatus);
	for (count = 0; count < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; count++) {
		if (hRapCh->sModuleHandles.hOp[count]) {
			rc = BRAP_OP_P_SetMute_isr(hRapCh->hRap, hRapCh->sModuleHandles.hOp[count]->eOutputPort, 
					psOpPortPrevMuteStatus->bDecMuteStatus[count]);
			if (rc != BERR_SUCCESS)
				return BERR_TRACE(rc);
			BDBG_MSG(("Unmuted outputport %d, mute status = %d", hRapCh->sModuleHandles.hOp[count]->eOutputPort, 
				psOpPortPrevMuteStatus->bDecMuteStatus[count]));
		}
#ifdef BCHP_7411_VER  /* only for 7411 */    	
		if (hRapCh->eClone!=BRAP_P_CloneState_eInvalid) {
#endif     
			if (hRapCh->sSimulPtModuleHandles.hOp[count]) {
				rc = BRAP_OP_P_SetMute_isr(hRapCh->hRap, hRapCh->sSimulPtModuleHandles.hOp[count]->eOutputPort, 
						psOpPortPrevMuteStatus->bSimulPtMuteStatus[count]);
				if (rc != BERR_SUCCESS)
					return BERR_TRACE(rc);
			BDBG_MSG(("Unmuted simulpt outputport %d, mute status = %d", hRapCh->sSimulPtModuleHandles.hOp[count]->eOutputPort, 
				psOpPortPrevMuteStatus->bSimulPtMuteStatus[count]));
			}
#ifdef BCHP_7411_VER  /* only for 7411 */                  
		}	
#endif       
#ifndef 	BCHP_7411_VER /* Other than 7411 chips */
		for (i = 0; i < BRAP_RM_P_MAX_OUTPUTS; i++) {
			if (hRapCh->sCloneOpPathHandles[count][i].hOp) {
				rc = BRAP_OP_P_SetMute_isr(hRapCh->hRap, hRapCh->sCloneOpPathHandles[count][i].hOp->eOutputPort, 
						psOpPortPrevMuteStatus->bCloneMuteStates[count][i]);
				if (rc != BERR_SUCCESS)
					return BERR_TRACE(rc);
				BDBG_MSG(("Unmuted cloned outputport %d, mute status = %d", hRapCh->sCloneOpPathHandles[count][i].hOp->eOutputPort, 
					psOpPortPrevMuteStatus->bCloneMuteStates[count][i]));
			}
		}
#endif
	}
	return rc;
}

/* BRAP_P_GetWatchdogRecoveryFlag: Returns watchdog recovery flag */
bool BRAP_P_GetWatchdogRecoveryFlag(BRAP_Handle hRap)
{
	return hRap->bWatchdogRecoveryOn;
}

/* BRAP_P_GetInternalCallFlag: Returns internal call flag. This flag is set when a RAP API
 * gets called from within PI. The called API checks for this flag and depending on its status
 * executes/skips updation of internal state variables. This function is used in decoder flush
 * operaton. */
bool BRAP_P_GetInternalCallFlag (BRAP_ChannelHandle hRapCh)
{
	return hRapCh->bInternalCallFromRap;
}

/* BRAP_P_AllocAligned : A wrapper around the BMEM module to 
 * include static memory allocation 
 */
void *BRAP_P_AllocAligned(
		BRAP_Handle  hRap,        /* [in] The RAP device handle */
		size_t       ulSize,      /* [in] size in bytes of block to allocate */
		unsigned int uiAlignBits, /* [in] alignment for the block */
		unsigned int uiBoundary   /* [in] boundary restricting allocated value */
#if (BRAP_SECURE_HEAP==1)
		,bool bAllocSecureMem		/*[in ] Enabled if the memory is allocated from secure memory*/
#endif		
		)
{
	uint32_t ui32_AllocMem ;
	uint32_t ui32_adjSize;
	void *ptr;

#ifndef BCHP_7411_VER /* For chips other than 7411 */
	uint32_t ui32PhyAddr ;
	BSTD_UNUSED(ui32_AllocMem);
	BSTD_UNUSED(ui32_adjSize);
#if (BRAP_SECURE_HEAP==1)
	if(hRap->sSettings.hSecureMemory!=NULL && bAllocSecureMem==true)
	{
		ptr = BMEM_AllocAligned(
				hRap->sSettings.hSecureMemory, 
				ulSize, 
				uiAlignBits, 
				uiBoundary) ;
	}
	else
	{
			ptr = BMEM_AllocAligned(
			hRap->hHeap, 
			ulSize, 
			uiAlignBits, 
			uiBoundary);

	}
#else
			ptr = BMEM_AllocAligned(
			hRap->hHeap, 
			ulSize, 
			uiAlignBits, 
			uiBoundary);
#endif

		BRAP_P_ConvertAddressToOffset(hRap->hHeap, ptr, &ui32PhyAddr);

		BDBG_MSG(("Allocated Memory : 0x%08lx (physical addr=0x%08lx), size 0x%08lx", ptr, ui32PhyAddr, ulSize));

		hRap->uiTotalMemUsed+=ulSize;
		return (ptr == 0)? (void *)0xFFFFFFFF : ptr ;
#else /* other than 7411 */
	BSTD_UNUSED(uiBoundary);
	BSTD_UNUSED(ptr);
		ui32_adjSize = ulSize + (1<<uiAlignBits)-1 ;

		/* The simple static memory allocator works as follows :
		 * The running pointer always points to the starting address
		 * of the remaining and free portion of the static memory block.
		 */
		ui32_AllocMem = hRap->sMemAllocInfo.currentStaticMemoryPointer ;

		BDBG_MSG(("Current Mem Pointer = 0x%08x, Base = 0x%08x, Size = 0x%08x",
		hRap->sMemAllocInfo.currentStaticMemoryPointer,
		hRap->sMemAllocInfo.currentStaticMemoryBase,
		hRap->sMemAllocInfo.staticMemorySize));

		/* Point the running pointer to the end+1 of the current buffer *
		 * but first check if the static memory block is not exhausted */
		if( (hRap->sMemAllocInfo.currentStaticMemoryPointer+ulSize) > 
			(hRap->sMemAllocInfo.currentStaticMemoryBase +
			hRap->sMemAllocInfo.staticMemorySize) )
		{
			BDBG_ERR(("BRAP_P_AllocAligned : No more Static Memory available"));
			return (void *) 0xFFFFFFFF ;
		}

		/* Align the address to the specified Bit position */
		ui32_AllocMem = BRAP_P_AlignAddress(ui32_AllocMem, uiAlignBits) ;

		hRap->sMemAllocInfo.currentStaticMemoryPointer += ui32_adjSize;

		/* Set the memory that is allocate to zero, this is asure allocated memory is a known value */
		{
			uint32_t offset;
			
			for(offset = 0; offset <= ui32_adjSize; offset += 4)
			{
				/* may over the allocated buffer by upto 3 bytes, but this should be okay 
				   since the override will occur to unallocated memory, due to the simple
				   memory allocator that currently exist */
				BRAP_P_DRAMWRITE((BARC_Handle)hRap->hRegister, (ui32_AllocMem + offset), 0x00);
			}
		}


		/* TODO : Add Boundary checks */

		hRap->uiTotalMemUsed+=ulSize;
		BDBG_MSG(("Allocated Memory : %08lx, size %08lx, prealigned = %08lx", ui32_AllocMem, ulSize,hRap->sMemAllocInfo.currentStaticMemoryPointer ));


		return (void *)ui32_AllocMem ;	
#endif
}

/* BRAP_P_Free : A wrapper around the BMEM module to  free device memory.
 * Device memory is actually freed only for non-7411 chips. For 7411 chips, this
 * function just returns.
 */
BERR_Code BRAP_P_Free(
        BRAP_Handle  hRap,      /* [in] The RAP device handle */
        void *pMemPtr           /* [in] Pointer to device memory to be freed */
#if (BRAP_SECURE_HEAP==1)    
        , bool bAllocSecureMem  /*[in ] Enabled if the memory is allocated from secure memory*/
#endif      
        )           
{
    BERR_Code err = BERR_SUCCESS;
#ifndef BCHP_7411_VER
    uint32_t ui32PhyAddr=0;
#endif
    BDBG_ASSERT(hRap);

#ifdef BCHP_7411_VER
    BSTD_UNUSED(hRap);
    BSTD_UNUSED(pMemPtr);
    BSTD_UNUSED(err);
    return BERR_SUCCESS;
#else
    /* For non-7411 chips */

    if((pMemPtr !=NULL) && (pMemPtr!=(void *)BRAP_P_INVALID_DRAM_ADDRESS))
    {
#if (BRAP_SECURE_HEAP==1)
        if(hRap->sSettings.hSecureMemory!=NULL && bAllocSecureMem==true)
        {
            BDBG_MSG(("Freeing memory [0x%x] from Secure region", pMemPtr));        
            err = BMEM_Free(hRap->sSettings.hSecureMemory, pMemPtr);    
        }
        else
        {
#endif  
		BRAP_P_ConvertAddressToOffset(hRap->hHeap, pMemPtr, &ui32PhyAddr);


            BDBG_MSG(("Freeing memory 0x%x (physical addr=0x%x)", pMemPtr, ui32PhyAddr));
            err = BMEM_Free(hRap->hHeap, pMemPtr);

#if (BRAP_SECURE_HEAP==1)
        }
#endif
    }
    
    return err;
#endif /* For non-7411 chips */
}


/* BRAP_P_ConvertAddressToOffset: Wrapper on BMEM function BMEM_ConvertAddressToOffset.
 * Since for 7411 chips, RAP PI doesn't get virtual pointer to device memory and access
 * device memory using physical address only, this function returns device offset equal
 * to virtual pointer to device memory for these chips. For all other chips it calls 
 * BMEM function BMEM_ConvertAddressToOffset for this conversion.
 */

BERR_Code BRAP_P_ConvertAddressToOffset(
	BMEM_Handle  Heap,    /* Heap that contains the memory block. */
	void        *pvAddress, /* Address of the memory block */
	uint32_t    *pulOffset)   /* [out] Returned device offset. */
{
	BERR_Code err = BERR_SUCCESS;
	
#ifndef BCHP_7411_VER /* For chips other than 7411 */
	err = BMEM_ConvertAddressToOffset(Heap, pvAddress, pulOffset);
#else
	BSTD_UNUSED(Heap);
	*pulOffset = (uint32_t) pvAddress;
#endif
	return err;
}

/* BRAP_P_ConvertAddressToOffset_isr: Isr version of BRAP_P_ConvertAddressToOffset */

BERR_Code BRAP_P_ConvertAddressToOffset_isr(
	BMEM_Handle  Heap,    /* Heap that contains the memory block. */
	void        *pvAddress, /* Address of the memory block */
	uint32_t    *pulOffset)   /* [out] Returned device offset. */
{
	BERR_Code err = BERR_SUCCESS;
	
#ifndef BCHP_7411_VER /* For chips other than 7411 */
	err = BMEM_ConvertAddressToOffset_isr(Heap, pvAddress, pulOffset);
#else
	BSTD_UNUSED(Heap);
	*pulOffset = (uint32_t) pvAddress;
#endif
	return err;
}

/* BRAP_P_ConvertOffsetToAddress: Wrapper on BMEM function BMEM_ConvertOffsetToAddress.
 * Since for 7411 chips, RAP PI doesn't get virtual pointer to device memory and access
 * device memory using physical address only, this function returns device offset equal
 * to virtual pointer to device memory for these chips. For all other chips it calls 
 * BMEM function BMEM_ConvertOffsetToAddress for this conversion.
 */
BERR_Code BRAP_ConvertOffsetToAddress
(
	BMEM_Handle   hHeap,    /* Heap that contains the memory block */
	uint32_t      ulOffset,  /* Device offset within the heap. */
	void        **ppvAddress)  /* [out] Returned address. */
{
	BERR_Code err = BERR_SUCCESS;
	
#ifndef BCHP_7411_VER /* For chips other than 7411 */
	err = BMEM_ConvertOffsetToAddress(hHeap, ulOffset, ppvAddress);
#else
	BSTD_UNUSED(hHeap);
	*ppvAddress = (void *)ulOffset;
#endif
	return err;
}

void BRAP_P_DownloadDspFwInMem(BARC_Handle hArc, uint32_t *data,
									uint32_t memAdr,
									uint32_t size)
{
	uint32_t index;

	if (data==NULL) {
		/* Initialize this buffer with zeros */
		for (index = 0; index < size/4; index++) {
			BRAP_P_DRAMWRITE(hArc, memAdr,0x0);
			memAdr += 4;
		}
	}
	else {
#ifndef BCHP_7411_VER /* For chips other than 7411 */	
		for (index = 0; index < size/4; index++) {
			BRAP_P_DRAMWRITE(hArc, memAdr,*(data + index));
			memAdr += 4;
		}
#else
              /* For 7411, we can use a BARC_Mem_Write to do a bulk write and speed thigns up */
		BARC_Mem_Write(hArc, memAdr, (void *) data,  size);
#endif
	}

	BRAP_P_DummyMemRead32(hArc, memAdr);
}

#ifndef BCHP_7411_VER /* For chips other than 7411 */	
/* We can use this function to change the endian-ness of the contents
  * and download to DRAM.This Function is added for debug pupose
  * for swapping the F/W contents and not used by any one
  */
void BRAP_P_DownloadDspFwInMemSwapped(BARC_Handle hArc, 
									uint32_t *data,
									uint32_t memAdr,
									uint32_t size)
{
	uint32_t index;
	uint32_t tempValue;

	if (data==NULL) {
		/* Initialize this buffer with zeros */
		for (index = 0; index < size/4; index++) {
			BRAP_P_DRAMWRITE(hArc, memAdr,0x0);
			memAdr += 4;
		}
	}
	else {
		for (index = 0; index < size/4; index++) {
			tempValue = *(data + index );
			tempValue = ( ( tempValue & 0x000000FF ) << 24 ) |
					      ( ( tempValue & 0x0000FF00 ) << 8 ) |
					      ( ( tempValue & 0x00FF0000 ) >> 8 ) |
					      ( ( tempValue & 0xFF000000 ) >> 24 );
			BRAP_P_DRAMWRITE(hArc, memAdr,tempValue);
			memAdr += 4;
		}
	}

	BRAP_P_DummyMemRead32(hArc, memAdr);
}
#endif

#ifndef EMULATION
void BRAP_MemWrite32(BARC_Handle hArc,   /* ARC Handle */
					uint32_t	offset, /* Memory offset to write */
					uint32_t	data   /* Data to write */
					)
{
#if BRAP_P_USE_BARC ==0
	BSTD_UNUSED(hArc);
	*((volatile uint32_t *)offset) = data;
#else
	BARC_Mem_Write(hArc, offset, &data, 4);
#endif
}

void BRAP_MemWrite32_isr(BARC_Handle hArc,   /* ARC Handle */
					uint32_t	offset, /* Memory offset to write */
					uint32_t	data   /* Data to write */
					)
{
#if BRAP_P_USE_BARC ==0
	BSTD_UNUSED(hArc);
	*((volatile uint32_t *)offset) = data;
#else
	BARC_Mem_Write_isr(hArc, offset, &data, 4);
#endif
}

void BRAP_MemWrite8(BARC_Handle hArc,   /* ARC Handle */
					uint32_t	offset, /* Memory offset to write */
					uint8_t	data   /* Data to write */
					)
{
#if BRAP_P_USE_BARC ==0
	BSTD_UNUSED(hArc);
	*((volatile uint8_t *)offset) = data;
#else
	BARC_Mem_Write(hArc, offset, &data, 1);
#endif
}

uint32_t BRAP_MemRead32(BARC_Handle hArc,      /* ARC Handle */
						uint32_t	offset /* Memory offset to write */
					   )
{
	uint32_t ui32ValRead;

#if BRAP_P_USE_BARC ==0
	BSTD_UNUSED(hArc);
	ui32ValRead = *((volatile uint32_t *)offset);
#else
	BARC_Mem_Read(hArc, offset, &ui32ValRead, 4);
#endif

	return ui32ValRead;
}

uint32_t BRAP_MemRead32_isr(BARC_Handle hArc,      /* ARC Handle */
						uint32_t	offset /* Memory offset to write */
					   )
{
	uint32_t ui32ValRead;

#if BRAP_P_USE_BARC ==0
	BSTD_UNUSED(hArc);
	ui32ValRead = *((volatile uint32_t *)offset);
#else
	BARC_Mem_Read_isr(hArc, offset, &ui32ValRead, 4);
#endif

	return ui32ValRead;
}

uint8_t BRAP_MemRead8(	BARC_Handle hArc,      /* ARC Handle */
						uint32_t	offset /* Memory offset to write */
					   )
{
	uint8_t ui8ValRead;

#if BRAP_P_USE_BARC ==0
	BSTD_UNUSED(hArc);
	ui8ValRead = *((volatile uint8_t *)offset);
#else
	BARC_Mem_Read(hArc, offset, &ui8ValRead, 1);
#endif

	return ui8ValRead;

}

void BRAP_DSP_Write32(BARC_Handle	hArc,
					uint32_t		offset,
					uint32_t		data,
					bool			bIsrCxt)
{
	if (bIsrCxt)
		BRAP_Write32_isr(hArc, offset, data);
	else
		BRAP_Write32(hArc, offset, data);
}

uint32_t BRAP_DSP_Read32(BARC_Handle hArc,
					uint32_t		offset,
					bool			bIsrCxt)
{
	uint32_t regVal;

	if(bIsrCxt)
		regVal = BRAP_Read32_isr(hArc, offset);
	else
		regVal = BRAP_Read32(hArc, offset);

	return regVal;
}

#endif /* EMULATION */


#ifndef BCHP_7411_VER /* For chips other than 7411 */

/* Read Physical Memory through MSA Client.
   It reads 8 32-bit words at a time.
   So uiPhyAddr should be a OWORD address */
/* This PI is not actively used. It is meant to be used only for debug purposes */
void BRAP_P_ReadPhyMem( BREG_Handle hReg,
						uint32_t	ui32PhyAddr,
						uint32_t*	pData)
{
	uint32_t ui32RegVal = 0;
	int iLoopCnt = 0;

	BDBG_ASSERT(hReg);
	BDBG_ASSERT(pData);

	/* write the command */
	BRAP_Write32(hReg, BCHP_MEMC_0_MSA_CMD_TYPE, 1);

	/* write the physical address */
	BRAP_Write32(hReg, BCHP_MEMC_0_MSA_CMD_ADDR, ui32PhyAddr);
	
	/* Wait till MSA is busy */
	do{
		ui32RegVal = BRAP_Read32(hReg, BCHP_MEMC_0_MSA_STATUS);
		iLoopCnt++;
		if(iLoopCnt > 1000)
		{
			BDBG_ERR(("\nMSA Read: Timeout!!!\n"));
			break;
		}
	}while(BCHP_GET_FIELD_DATA(ui32RegVal, MEMC_0_MSA_STATUS, BUSY) != 0);

	/* read back the data */
	*pData = BRAP_Read32(hReg, BCHP_MEMC_0_MSA_RD_DATA7);
	*(pData+1) = BRAP_Read32(hReg, BCHP_MEMC_0_MSA_RD_DATA6);
	*(pData+2) = BRAP_Read32(hReg, BCHP_MEMC_0_MSA_RD_DATA5);
	*(pData+3) = BRAP_Read32(hReg, BCHP_MEMC_0_MSA_RD_DATA4);
	*(pData+4) = BRAP_Read32(hReg, BCHP_MEMC_0_MSA_RD_DATA3);
	*(pData+5) = BRAP_Read32(hReg, BCHP_MEMC_0_MSA_RD_DATA2);
	*(pData+6) = BRAP_Read32(hReg, BCHP_MEMC_0_MSA_RD_DATA1);
	*(pData+7) = BRAP_Read32(hReg, BCHP_MEMC_0_MSA_RD_DATA0);
	
	return;
}

/* Write Physical Memory through MSA Client.
   It writess 8 32-bit words at a time.
   So uiPhyAddr should be a OWORD address */
/* This PI is not actively used. It is meant to be used only for debug purposes */   
void BRAP_P_WritePhyMem( BREG_Handle hReg,
						 uint32_t	ui32PhyAddr,
						 uint32_t*	pData)
{
	uint32_t ui32RegVal = 0;
	int iLoopCnt = 0;

	BDBG_ASSERT(hReg);
	BDBG_ASSERT(pData);

	/* read back the data */
	BRAP_Write32(hReg, BCHP_MEMC_0_MSA_WR_DATA7, *pData);
	BRAP_Write32(hReg, BCHP_MEMC_0_MSA_WR_DATA6, *(pData+1));
	BRAP_Write32(hReg, BCHP_MEMC_0_MSA_WR_DATA5, *(pData+2));
	BRAP_Write32(hReg, BCHP_MEMC_0_MSA_WR_DATA4, *(pData+3));
	BRAP_Write32(hReg, BCHP_MEMC_0_MSA_WR_DATA3, *(pData+4));
	BRAP_Write32(hReg, BCHP_MEMC_0_MSA_WR_DATA2, *(pData+5));
	BRAP_Write32(hReg, BCHP_MEMC_0_MSA_WR_DATA1, *(pData+6));
	BRAP_Write32(hReg, BCHP_MEMC_0_MSA_WR_DATA0, *(pData+7));

	/* write the command */
	BRAP_Write32(hReg, BCHP_MEMC_0_MSA_CMD_TYPE, 0x11);

	/* write the physical address */
	BRAP_Write32(hReg, BCHP_MEMC_0_MSA_CMD_ADDR, ui32PhyAddr);
	
	/* Wait till MSA is busy */
	do{
		ui32RegVal = BRAP_Read32(hReg, BCHP_MEMC_0_MSA_STATUS);
		iLoopCnt++;
		if(iLoopCnt > 1000)
		{
			BDBG_ERR(("\nMSA Write: Timeout!!!\n"));
			break;
		}
	}while(BCHP_GET_FIELD_DATA(ui32RegVal, MEMC_0_MSA_STATUS, BUSY) != 0);
		
	return;
}
#endif 



/* This is a static routine that checks if an output port is used 
   in a RAP channel. This routine checks sModuleHandles, 
   sSimulPtModuleHandles and sCloneOpPathHandles to decide if an 
   output port is in use by this channel or not.
   If yes, it returns output params that indicate 
   i) channel pair 
   ii) whether cloned port
   iii) whether used for simulPt mode
   If neither from sClone or sSimulPt handles, then it is from a
   normal module handle.

   Note: To get eChanPair, the caller of this API should also pass 
   valid pClone and pSimulPt.*/
static BERR_Code
BRAP_P_GetInfoForOpPortUsedbyChannel(
	const BRAP_ChannelHandle 	hRapCh,     /* [in] The RAP Channel handle */
	const BRAP_OutputPort	    eOutputPort,/* [in] Output port */
    bool                        *pFlag,     /* [out] TRUE: output port eOutputPort 
                                                    is used by the channel hRapCh 
                                                FALSE: is not used by this channel*/
    BRAP_OutputChannelPair      *pChanPair, /* [out] Channel pair if output port is used */
    bool                        *pClone,    /* [out] TRUE: if output port is cloned port
                                                     FALSE: otherwise.
                                               This output param should be used by the 
                                               caller only if pFlag is TRUE.
                                               This is an optional output parameter. */
    bool                        *pSimulPt   /* [out] TRUE: if output port is used for 
                                               simulPt mode
                                                     FALSE: otherwise.
                                               This is an optional output parameter. */
)
{
    unsigned int i;
#ifndef BCHP_7411_VER
    unsigned int j;
#endif
    BERR_Code ret = BERR_SUCCESS;
    
    BDBG_ENTER(BRAP_P_GetInfoForOpPortUsedbyChannel);
	
    /* Validate input params */
    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(pFlag);

    /* Pre-init out params */
    *pFlag = false;
    if(pClone != NULL)
    {   
        *pClone = false;
    }
    if(pSimulPt != NULL)
    {
        *pSimulPt = false;
    }

    /* If user is interested in pChanPair, he should also
       pass a valid pClone and pSimulPt */
    if((pChanPair != NULL) && 
       ((pClone == NULL) || (pSimulPt == NULL)))
    {
        BDBG_ERR(("BRAP_P_GetInfoForOpPortUsedbyChannel: All pChanPair, pClone and pSimulPt be valid"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
        
    for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    {
        /* Check if this port is the main port for this channel pair */
        if (hRapCh->sModuleHandles.hOp[i] != NULL)
        {
            if (hRapCh->sModuleHandles.hOp[i]->eOutputPort == eOutputPort)
            {
                *pFlag = true;
                if(pChanPair != NULL)
                {
                    *pChanPair = (BRAP_OutputChannelPair)i;
                }

                BDBG_MSG(("Output port %d is used as the main port for channel pair %d in this channel.", eOutputPort, i));
                return ret;
            }
            /* If this port is the Flex or SPDIF, then it may not show up in the stored handles if it is used in conjunction with the Mai. 
            So check the Mai Mux Select */            
            else if ((hRapCh->sModuleHandles.hOp[i]->eOutputPort == BRAP_OutputPort_eMai)
                        && (hRapCh->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector == eOutputPort))
            {
                *pFlag = true;
                if(pChanPair != NULL)
                {
                    *pChanPair = (BRAP_OutputChannelPair)i;
                }

                BDBG_MSG(("Output port %d is used in conjunction with Mai as the main port for channel pair %d in this channel.", eOutputPort, i));
                return ret;
            }
        } /* end of main ports */

        /* Check if this port is the simulPt port for this channel pair */        
        if (hRapCh->sSimulPtModuleHandles.hOp[i] != NULL)
        {
            if (hRapCh->sSimulPtModuleHandles.hOp[i]->eOutputPort == eOutputPort)
            {
                *pFlag = true;
                if(pChanPair != NULL)
                {
                    *pChanPair = (BRAP_OutputChannelPair)i;
                }
                if(pSimulPt != NULL)
                {
                    *pSimulPt = true;
                }

                BDBG_MSG(("Output port %d is used as the simul mode (pass thru context) port for channel pair %d in this channel.", eOutputPort, i));
                return ret;
            }
            /* If this port is the Flex or SPDIF, then it may not show up in the stored handles if it is used in conjunction with the Mai. 
            So check the Mai Mux Select */            
            else if ((hRapCh->sSimulPtModuleHandles.hOp[i]->eOutputPort == BRAP_OutputPort_eMai)
                        && (hRapCh->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector == eOutputPort))
            {
                *pFlag = true;
                if(pChanPair != NULL)
                {
                    *pChanPair = (BRAP_OutputChannelPair)i;
                }
                if(pSimulPt != NULL)
                {
                    *pSimulPt = true;
                }

                BDBG_MSG(("Output port %d is used in conjunction with Mai as the simul port for channel pair %d in this channel.", eOutputPort, i));
                return ret;
            }            
        } /* end of simulPt ports */

#ifndef BCHP_7411_VER /* For chips other than 7411 */
        /* Check if this port is cloned for this channel pair */
        for(j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
        {
            if (hRapCh->sCloneOpPathHandles[i][j].hOp != NULL)
            {
                if (hRapCh->sCloneOpPathHandles[i][j].hOp->eOutputPort == eOutputPort)
                {
                    *pFlag = true;
                    if(pClone != NULL)
                    {   
                        *pClone = true;
                    }
                    if(pChanPair != NULL)
                    {
                        *pChanPair = (BRAP_OutputChannelPair)i;
                    }

                    BDBG_MSG(("Output port %d is used as the cloned port for channel pair %d in this channel.", eOutputPort, i));
                    return ret;
                } 
                /* If this port is the Flex or SPDIF, then it may not show up in the stored handles if it is used in conjunction with the Mai. 
                So check the Mai Mux Select */            
                else if ((hRapCh->sCloneOpPathHandles[i][j].hOp->eOutputPort == BRAP_OutputPort_eMai)
                            && (hRapCh->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector == eOutputPort))
                {
                    *pFlag = true;
                    if(pClone != NULL)
                    {   
                        *pClone = true;
                    }
                    if(pChanPair != NULL)
                    {
                        *pChanPair = (BRAP_OutputChannelPair)i;
                    }

                    BDBG_MSG(("Output port %d is used in conjunction with Mai as the cloned port for channel pair %d in this channel.", eOutputPort, i));
                    return ret;
                }                         
             }
        }/* end of cloned ports */
#endif /* Not for 7411 */
    }

    BDBG_LEAVE(BRAP_P_GetInfoForOpPortUsedbyChannel);
    return ret;
}

/* This routine checks if an output port is used in a RAP channel.
   This routine checks sModuleHandles, sSimulPtModuleHandles and
   sCloneOpPathHandles to decide if an output port is in use by 
   this channel or not.
   If an output port is used by a channel, this routine also informs
   the caller if the output port is a cloned port or not. This is an
   optional output parameter. */    
BERR_Code
BRAP_P_IsOpPortUsedbyChannel(
	const BRAP_ChannelHandle 	hRapCh,     /* [in] The RAP Channel handle */
	const BRAP_OutputPort	    eOutputPort,/* [in] Output port */
    bool                        *pFlag,     /* [out] TRUE: output port eOutputPort 
                                                    is used by the channel hRapCh 
                                                FALSE: is not used by this channel*/
    bool                        *pClone     /* [out] TRUE: if output port is cloned port
                                                     FALSE: otherwise.
                                               This output param should be used by the 
                                               caller only if pFlag is TRUE.
                                               This is an optional output parameter. */
)
{
    BERR_Code ret = BERR_SUCCESS;
    
    BDBG_ENTER(BRAP_P_IsOpPortUsedbyChannel);
	
    /* Validate input params */
    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(pFlag);

    *pFlag = false;
    if(pClone != NULL)
    {   
        *pClone = false;
    }

    ret = BRAP_P_GetInfoForOpPortUsedbyChannel(
                        hRapCh, 
                        eOutputPort, 
                        pFlag, 
                        NULL /*pChanPair*/, 
                        pClone, 
                        NULL /*pSimulPt*/
                        );
    BDBG_LEAVE(BRAP_P_IsOpPortUsedbyChannel);
    return BERR_TRACE(ret);    
}

/* Returns the channel pair along with used for whether clone/SimulPt info if 
   the output is used by the channel */
BERR_Code
BRAP_P_GetChannelPairUsingOpPort_isr(
	const BRAP_ChannelHandle hRapCh,        /* [in] The RAP Channel handle */
	const BRAP_OutputPort	 eOutputPort, 	/* [in] Output port */
    unsigned int             *pChnPair,     /* [out] channel pair if eOutputPort 
                                                is used by the channel hRapCh */ 
    bool                     *pClone,       /* [out]TRUE: chan pair corresponds to 
                                               sCloneHandles */
    bool                     *pSimulPt      /* [out]TRUE: chan pair corresponds to 
                                               sSimulPtHandles */
                                            /* Note: if valid chan pair and both 
                                               pClone and pSimulPt are false, 
                                               then this is from sModuleHandle */
)
{
    BERR_Code   ret = BERR_SUCCESS;
    bool        bFlag = false;
    
    BDBG_ENTER(BRAP_P_GetChannelPairUsingOpPort_isr);
	
    /* Validate input params */
    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(pChnPair);
    BDBG_ASSERT(pClone);
    BDBG_ASSERT(pSimulPt);
    
    *pClone = false;
    *pSimulPt = false;
    *pChnPair = BRAP_INVALID_VALUE;

    ret = BRAP_P_GetInfoForOpPortUsedbyChannel(
                        hRapCh, 
                        eOutputPort, 
                        &bFlag, 
                        (BRAP_OutputChannelPair *)pChnPair,
                        pClone, 
                        pSimulPt);
    if(ret == BERR_SUCCESS)
    {
        if(bFlag == false)
        {
            ret = BERR_NOT_INITIALIZED;
        }
    }

    BDBG_LEAVE(BRAP_P_GetChannelPairUsingOpPort_isr);
    return BERR_TRACE(ret);    
}

BERR_Code
BRAP_P_GetPllForOp(
	const BRAP_Handle 	hRap, /* [in] The RAP handle */
	const BRAP_OutputPort	eOutputPort,	/* [in] Output port */
       BRAP_OP_Pll  * pPll  /* [out] the Pll associated with this output port */
)
{
    BERR_Code ret = BERR_SUCCESS;
    
    BDBG_ENTER(BRAP_P_GetPllForOp);
    BDBG_ASSERT(hRap);
    BDBG_ASSERT(pPll);
    
    /* Make sure  port is configured */
    if(hRap->bOpSettingsValid[eOutputPort] == false)    
    {
        BDBG_ERR(("BRAP_P_GetPllForOp: Output port %d is not configured. Please configure before calling this PI.", eOutputPort));
        return BERR_TRACE(BRAP_ERR_OUTPUT_NOT_CONFIGURED);
    }       
        switch(eOutputPort)
        {
            case BRAP_OutputPort_eSpdif:
                *pPll = hRap->sOutputSettings[eOutputPort].uOutputPortSettings.sSpdifSettings.ePll;                            
                break;
            case BRAP_OutputPort_eI2s0:
            case BRAP_OutputPort_eI2s1:
            case BRAP_OutputPort_eI2s2:              
                *pPll = hRap->sOutputSettings[eOutputPort].uOutputPortSettings.sI2sSettings.ePll;                             
                break;
            case BRAP_OutputPort_eMai:
                    if (hRap->sOutputSettings[eOutputPort].uOutputPortSettings.sMaiSettings.eMaiMuxSelector == BRAP_OutputPort_eSpdif )
                    {
                        *pPll =  hRap->sOutputSettings[BRAP_OutputPort_eSpdif].uOutputPortSettings.sSpdifSettings.ePll; 
                     }
                    else if (hRap->sOutputSettings[eOutputPort].uOutputPortSettings.sMaiSettings.eMaiMuxSelector == BRAP_OutputPort_eFlex )
                    {
                        *pPll = hRap->sOutputSettings[BRAP_OutputPort_eFlex].uOutputPortSettings.sFlexSettings.ePll;                   
                    }                               
                break;
            case BRAP_OutputPort_eFlex:
                *pPll = hRap->sOutputSettings[eOutputPort].uOutputPortSettings.sFlexSettings.ePll; 
                break;
            case BRAP_OutputPort_eDac0:
            case BRAP_OutputPort_eDac1:
                *pPll = BRAP_RM_P_INVALID_INDEX;
                break;
            default:
                    BDBG_ERR(("BRAP_P_GetPllForOp:  Output port type %d not supported",  eOutputPort ));
                    return BERR_TRACE(BRAP_ERR_OUPUT_PORT_NOT_SUPPORTED);   
        }

    BDBG_MSG(("BRAP_P_GetPllForOp: Pll %d is associated with ouput port %d", *pPll, eOutputPort));

    BDBG_LEAVE (BRAP_P_GetPllForOp);        
    return ret;
}

BERR_Code BRAP_P_ConvertDelayToStartWRPoint(
	unsigned int 			uiDelay /* micro sec */,
	BRAP_BufDataMode 		eBufDataMode,
	BAVC_AudioSamplingRate 	eSamplingRate,
	unsigned int			uiIpBitsPerSample,
    unsigned int	        *pStartWRPoint
	)
{
	unsigned int uiDelayInBytes = 0;
	unsigned int uiSR = 0;
    BERR_Code err = BERR_SUCCESS;
    
	BDBG_ENTER(BRAP_P_ConvertDelayToStartWRPoint);
    BDBG_ASSERT(pStartWRPoint);

	err = BERR_TRACE(BRAP_P_ConvertSR(eSamplingRate, &uiSR));
    if(err != BERR_SUCCESS)
    {
        return err;
    }
    
	if(eBufDataMode == BRAP_BufDataMode_eStereoInterleaved)
	{
	 	/* For Stereo Interleaved case, 
 			delayInBytes = 2 * Delay * Sampling rate * Bits per Sample / 8 */
 		uiDelayInBytes = (((2 * uiSR * uiIpBitsPerSample)/1000) * uiDelay)/1000;
		uiDelayInBytes >>= 3; /* Divide by 8: bits to byte conversion */
	}
	else
	{
		/* For Mono or Stereo Non-interleaved case,
 			delayInBytes = Delay * Sampling rate * Bits per Sample / 8; */
 		uiDelayInBytes = (((uiSR * uiIpBitsPerSample)/1000) * uiDelay)/1000;
		uiDelayInBytes >>= 3; /* Divide by 8: bits to byte conversion */
	}

    *pStartWRPoint = uiDelayInBytes;
        
	BDBG_LEAVE(BRAP_P_ConvertDelayToStartWRPoint);
	return err;
}

/* 
    Recommended Burst Repetition Periods for various algo types are as follows:
    AC3 and DTS = PER_3
    MPEG-2 Layers 1, 2, and 3 < 32kHz = PER_64
    MPEG-1, MPEG-2/AAC, and any other MPEG-2 formats = PER_32
*/
BERR_Code BRAP_P_GetBurstRepetitionPeriodForAlgo(
    BAVC_StreamType                 eStreamType,
                                        /* [in] audio stream type*/
    BRAP_DSPCHN_AudioType           eAudioType, 
                                        /* [in] audio type for which 
                                           burst repetition period is 
                                           sought */
    BRAP_SPDIFFM_P_BurstRepPeriod   *pBurstRepPeriod
                                        /* [out] corresponding burst 
                                           repetition period for the 
                                           algo type passed */
    )
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER(BRAP_P_GetBurstRepetitionPeriodForAlgo);

    /* validate input */
    BDBG_ASSERT(pBurstRepPeriod);

    switch(eAudioType)
    {
        /* Here we return PER_32 for both MPEG-1 and MPEG-2. For
           MPEG-2 < 32KHz, PER_64 is set from SamplingRateChange_isr */
        case BRAP_DSPCHN_AudioType_eMpeg:
            if((eStreamType == BAVC_StreamType_ePS) ||
               (eStreamType == BAVC_StreamType_eTsMpeg)) 
                *pBurstRepPeriod = BRAP_SPDIFFM_P_BurstRepPeriod_ePer32;
            else
                *pBurstRepPeriod = BRAP_SPDIFFM_P_BurstRepPeriod_eNone;
            break;

    	case BRAP_DSPCHN_AudioType_eAc3:
    	case BRAP_DSPCHN_AudioType_eDra:				
    	case BRAP_DSPCHN_AudioType_eAc3Lossless:
       	case BRAP_DSPCHN_AudioType_eDts:
       	case BRAP_DSPCHN_AudioType_eDtshd:
            *pBurstRepPeriod = BRAP_SPDIFFM_P_BurstRepPeriod_ePer3;
            break;
            
        /*Note: AC3+ simul mode uses SPDIFFM */
        case BRAP_DSPCHN_AudioType_eAc3Plus:
            *pBurstRepPeriod = BRAP_SPDIFFM_P_BurstRepPeriod_ePer4;
        break;
        
       	case BRAP_DSPCHN_AudioType_eAac:
       	case BRAP_DSPCHN_AudioType_eAacSbr:
            *pBurstRepPeriod = BRAP_SPDIFFM_P_BurstRepPeriod_ePer32;
            break;

        /* Not supported / un-compressed algo types */
       	case BRAP_DSPCHN_AudioType_eLpcmBd:
       	case BRAP_DSPCHN_AudioType_eLpcmHdDvd:            
       	case BRAP_DSPCHN_AudioType_eLpcmDvd:
		case BRAP_DSPCHN_AudioType_eWmaStd:
		case BRAP_DSPCHN_AudioType_eWmaPro:
		case BRAP_DSPCHN_AudioType_ePcmWav:            
		case BRAP_DSPCHN_AudioType_eMlp:
#ifdef RAP_SRSTRUVOL_CERTIFICATION  
		case BRAP_DSPCHN_AudioType_ePCM:          
#endif			
            *pBurstRepPeriod = BRAP_SPDIFFM_P_BurstRepPeriod_eNone;
            break;
            
        case BRAP_DSPCHN_AudioType_eInvalid:
        default: 
            return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_LEAVE(BRAP_P_GetBurstRepetitionPeriodForAlgo);
    return ret;
}
    
BERR_Code BRAP_P_GetCurrentMemoryUsage(
	BRAP_Handle 			hRap,		/* [in] The RAP device handle */
	unsigned int			*puiTotalMemUsed	/* [out] Total device memory used by RAP PI */
	)
{
	BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BRAP_P_GetCurrentMemoryUsage);
    
	BDBG_ASSERT(hRap);
	BDBG_ASSERT(puiTotalMemUsed);

	*puiTotalMemUsed = hRap->uiTotalMemUsed;

    BDBG_LEAVE(BRAP_P_GetCurrentMemoryUsage);
	return err;	
}


/***************************************************************************
Summary:
    This private routine is used to check if a new channel is compatible with
    existing channels.
Description:
    This private routine is used to check if a new channel is compatible with
    existing channels. This routine should be called at the very beginning of a 
    channel open(DEC/PB/CAP).

    The non support combinations of channels are as follows:
    1) Two DEC channels on the same output port i.e. 2 direct DEC channels
    2) Two DEC channels on different output ports where one is captured and
       mixed to another i.e. one direct DEC channel and another indirect DEC
       channel associated to an output port through a CAP channel.
    3) A CAP channel getting captured again by another CAP channel i.e. 
       one direct CAP channel and another indirect CAP channel

Returns:
	BERR_SUCCESS if compatible else error
See Also:
    BRAP_OP_P_GetAssociatedChannels
****************************************************************************/
BERR_Code BRAP_P_IsNewChanCompatible(
	BRAP_Handle 			hRap,		/* [in] The RAP device handle */
    BRAP_P_ChannelType      eNewChannelType,/*[in] The new RAP Channel type */
    BRAP_OutputPort         eOutputPort     /*[in] output port for the new channel */  
    )
{
	BERR_Code ret = BERR_SUCCESS;
    BRAP_ChannelHandle hDirChannels[BRAP_RM_P_MAX_MIXER_INPUTS];
    BRAP_ChannelHandle hIndirChannels[BRAP_RM_P_MAX_MIXER_INPUTS];
    int i;

    BDBG_ENTER(BRAP_P_IsNewChanCompatible);
    BDBG_ASSERT(hRap);

    ret = BRAP_OP_P_GetAssociatedChannels(hRap, &hDirChannels[0], &hIndirChannels[0], 
            eOutputPort, false /* bCalledFromISR */);
    if(ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_OP_P_SetSamplingRate: Can't get associated channel handles"));
        return BERR_TRACE (ret);
    }

    if(eNewChannelType == BRAP_P_ChannelType_eDecode)
    {
        /* Invalid case#1: New chan = DEC and an already DEC chan in direct path */
        for(i=0; i<BRAP_RM_P_MAX_MIXER_INPUTS && hDirChannels[i]!=NULL; i++)
        {
            if(hDirChannels[i]->eChannelType == BRAP_P_ChannelType_eDecode)
            {
                BDBG_ERR(("BRAP_P_IsNewChanCompatible: Two direct DEC channels are not supported"));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
        }

        /* Invalid case#2: New chan = DEC and an already DEC chan in indirect path */
        for(i=0; i<BRAP_RM_P_MAX_MIXER_INPUTS && hIndirChannels[i]!=NULL; i++)
        {
            if(hIndirChannels[i]->eChannelType == BRAP_P_ChannelType_eDecode)
            {
                BDBG_ERR(("BRAP_P_IsNewChanCompatible: One direct DEC channels captured and"
                          "linked to another indirect DEC channel is not supported"));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
        }
    }

    /* Invalid case#3: A CAP channel getting captured again by another CAP channel */
    if(eNewChannelType == BRAP_P_ChannelType_eCapture)
    {
        for(i=0; i<BRAP_RM_P_MAX_MIXER_INPUTS && hIndirChannels[i]!=NULL; i++)
        {
            if(hIndirChannels[i]->eChannelType == BRAP_P_ChannelType_eCapture)
            {
                BDBG_ERR(("BRAP_P_IsNewChanCompatible: A CAP channel getting captured again"
                          "by another CAP channel is not supported"));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
        }    
    }        

    BDBG_LEAVE(BRAP_P_IsNewChanCompatible);
    return ret;
}


/***************************************************************************
Summary:
	Checks if Compressed channel being Mixed with PCM channel
Description:
 	This functions returns true of false depending on if compressed 
 	data on a channel is being mixed with PCm dat on other channel.
Returns:
        BERR_SUCCESS on success, else error.
        bIsCompressedMixedWithPCM would be true or false.
See Also:
 	None
**************************************************************************/
BERR_Code BRAP_P_CheckIfCompressedMixedWithPCM(
	const BRAP_Handle hRap,			/*[in] The Rap Handle*/
	const BRAP_OutputPort eOutputPort, /*[in] The output port for the current channel*/
	bool bNewChannelCompressed,	/*[in] Current channel compressed or not*/
	bool *bIsCompressedMixedWithPCM	/*[out] TRUE: if compressed and PCM are being mixed 
										     FALSE: If not*/
	)
{
	BERR_Code 			ret = BERR_SUCCESS;
	unsigned int			uiMixerId;
	BRAP_ChannelHandle	phChannels[BRAP_RM_P_MAX_MIXER_INPUTS];
	BRAP_ChannelHandle 	hRapCh = NULL;
	unsigned int 			i,j;

	BDBG_ENTER(BRAP_P_CheckIfCompressedMixedWithPCM);

	/*Validate the Input Parameters*/
	BDBG_ASSERT(hRap);

	/*Initialize the output value to false*/
	*bIsCompressedMixedWithPCM = false;

	/*Intialize the Array to Null*/
	for(i=0; i<BRAP_RM_P_MAX_MIXER_INPUTS; i++)
		phChannels[i]=NULL;

	/*Get the list of channels on the given Output Port*/
	ret = BRAP_P_GetChannelsUsingOpPort(hRap, eOutputPort, phChannels);
	if(BERR_SUCCESS == ret)
	{
		for(i=0; i<BRAP_RM_P_MAX_MIXER_INPUTS && phChannels[i];i++)
		{
			hRapCh = phChannels[i];
			
			/*If current channel has compressed data*/
			if(bNewChannelCompressed)
			{
				/* Since the compressed data can come only on decode channel and bNewChannelCompressed 
				     is true that means the current channel is Decode and since two decode channels can not
				     go on one Mixer input , so checking only for PlayBack and Capture Channel.*/
				if((hRapCh->bStarted == true)&&
                   ((hRapCh->eChannelType ==BRAP_P_ChannelType_eCapture) || 
				   (hRapCh->eChannelType ==BRAP_P_ChannelType_ePcmPlayback)))
			   	{
					*bIsCompressedMixedWithPCM = true;
					BDBG_ERR(("BRAP_P_CheckIfCompressedMixedWithPCM(): Channel cant Be mixed"));
					goto end;
				}
			}
			else	
			{
				/*Check for a Decode Channel on which Compressed Data can be present*/
				if(hRapCh->eChannelType ==BRAP_P_ChannelType_eDecode)
				{
					/*Get the Mixer being used by getting the Mixer Id*/
					ret = BRAP_RM_P_GetMixerForOpPort(hRap->hRm,eOutputPort, &uiMixerId);
					if (ret != BERR_SUCCESS)
					{
					        BDBG_ERR(("BRAP_P_CheckIfCompressedMixedWithPCM(): Cant get Mixer ID"));
					        return BERR_TRACE(ret);
	    				}

					for(j=0; j<BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; j++)
					{
						/*Check the Mixer Handle*/
						if((hRapCh->bStarted == true)&&
                           (hRapCh->sModuleHandles.hMixer[j] != NULL) && 
						   (hRapCh->sModuleHandles.hMixer[j]->uiMixerIndex == uiMixerId))
						{
							/*Check if Decode Channel has Compressed Data or not*/
							if(hRapCh->sModuleHandles.hMixer[j]->bCompress == true)
							{
								*bIsCompressedMixedWithPCM = true;
								BDBG_ERR(("BRAP_P_CheckIfCompressedMixedWithPCM(): Channel cant Be mixed"));
								goto end;
							}
						}

						/*Check the Mixer Handle for Simul Mode*/
						if((hRapCh->bStarted == true)&&
                           (hRapCh->sSimulPtModuleHandles.hMixer[j] != NULL )&& 
						   (hRapCh->sSimulPtModuleHandles.hMixer[j]->uiMixerIndex == uiMixerId))
						{
							/*Check if Decode Channel has Compressed Data or not*/
							if(hRapCh->sSimulPtModuleHandles.hMixer[j]->bCompress == true)
							{
								*bIsCompressedMixedWithPCM = true;
								BDBG_ERR(("BRAP_P_CheckIfCompressedMixedWithPCM(): Channel cant Be mixed"));
								goto end;
							}
						}
						#ifndef BCHP_7411_VER
						{
							unsigned int k;

							/*Check the Mixer Handle for Clonned outputs*/
							for(k=0;k<BRAP_RM_P_MAX_OUTPUTS;k++)
							{
								if((hRapCh->bStarted == true)&&
                                   (hRapCh->sCloneOpPathHandles[j][k].hMixer != NULL)&&
                                   (hRapCh->sCloneOpPathHandles[j][k].hMixer->uiMixerIndex == uiMixerId))
								{
									/*Check if Decode Channel has Compressed Data or not*/
									if(hRapCh->sCloneOpPathHandles[j][k].hMixer->bCompress == true)
									{
										*bIsCompressedMixedWithPCM = true;
										BDBG_ERR(("BRAP_P_CheckIfCompressedMixedWithPCM(): Channel cant Be mixed"));
										goto end;
									}
								}
							}
						}
						#endif
					}
				}
			}
		}
	}
		
end:	
	BDBG_LEAVE(BRAP_P_CheckIfCompressedMixedWithPCM);
	return ret;
}


#ifndef BCHP_7411_VER /* For chips other than 7411 */

/***************************************************************************
Summary:
    This API should be called for setting the Channel status params on-the-fly for SPDIF and HDMI.
    It should be called only after atleast one channel using this output port 
    is already running since it assumes that the sample rate info /dsp channel handle is 
    already available
    
    If there is any decode channel channel using this port, this PI will write the 
    new values to DRAM and the Fw will then program the CBIT registers.
    If there is no currently running decode channel using tihs port, the PI will 
    directly write to the CBIT registers.

    Note: if a channel using this port is started/restarted AFTER calling this PI, then the Channel status 
    params passed in BRAP_DEC_Start() [or corresponding PB/CAP start] will take effect.

See Also:
	BRAP_SetOutputConfig, BRAP_GetOutputDefaultConfig, BRAP_GetOutputConfig
****************************************************************************/
BERR_Code BRAP_P_SetCbit( 
	BRAP_Handle 				hRap,				/* [in] The Raptor Audio device handle*/
	const BRAP_OutputSettings	*pOutputSettings	/* [in] Output port settings*/ 
	)
{
    BERR_Code ret = BERR_SUCCESS;
    int i;
    BRAP_ChannelHandle hDirChannels[BRAP_RM_P_MAX_MIXER_INPUTS];
    bool bDone=false;
    unsigned int uiSpdiffmId, uiSpdiffmStreamId;

    BDBG_ENTER (BRAP_P_SetCbit);
    BDBG_ASSERT (hRap);   

    BDBG_MSG(("Entering BRAP_P_SetCbit"));        

    BDBG_MSG(("bProfessionalMode=%d", pOutputSettings->sSpdifChanStatusParams.bProfessionalMode)); 
    BDBG_MSG(("bLinear_PCM=%d", pOutputSettings->sSpdifChanStatusParams.bLinear_PCM));     
    BDBG_MSG(("bSwCopyRight=%d", pOutputSettings->sSpdifChanStatusParams.bSwCopyRight)); 
    BDBG_MSG(("uiPre_Emphasis=%d", pOutputSettings->sSpdifChanStatusParams.uiPre_Emphasis)); 
    BDBG_MSG(("uiChannel_Status_Mode=%d", pOutputSettings->sSpdifChanStatusParams.uiChannel_Status_Mode)); 
    BDBG_MSG(("ui16CategoryCode=%d", pOutputSettings->sSpdifChanStatusParams.ui16CategoryCode)); 
    BDBG_MSG(("uiSource_Number=%d", pOutputSettings->sSpdifChanStatusParams.uiSource_Number)); 
    BDBG_MSG(("bSeparateLRChanNum=%d", pOutputSettings->sSpdifChanStatusParams.bSeparateLRChanNum)); 
    
    BDBG_MSG(("ui16ClockAccuracy=%d", pOutputSettings->sSpdifChanStatusParams.ui16ClockAccuracy));
    BDBG_MSG(("bSampleWorldLength=%d", pOutputSettings->sSpdifChanStatusParams.bSampleWorldLength)); 
    BDBG_MSG(("uiSample_Word_Length=%d", pOutputSettings->sSpdifChanStatusParams.uiSample_Word_Length)); 
    BDBG_MSG(("uiOriginal_Sampling_Frequency=%d", pOutputSettings->sSpdifChanStatusParams.uiOriginal_Sampling_Frequency));
    BDBG_MSG(("uiCGMS_A =%d", pOutputSettings->sSpdifChanStatusParams.uiCGMS_A));	

    /* Intializing the hDirChannels to NULL*/
    for(i = 0; i < BRAP_RM_P_MAX_MIXER_INPUTS; i++)
    {
        hDirChannels[i] = NULL;
    }

    /* Find the relevant SPDIFFM stream and save the new channel status params*/
    switch (pOutputSettings->eOutput)
    {
         case BRAP_OutputPort_eSpdif:
         case BRAP_OutputPort_eFlex:
    BRAP_RM_P_GetSpdifFmForOpPort (hRap->hRm, 
                                   pOutputSettings->eOutput, 
                                   &uiSpdiffmId, 
                                   &uiSpdiffmStreamId);  
            break;
        case BRAP_OutputPort_eMai:
            BRAP_RM_P_GetSpdifFmForOpPort (hRap->hRm, 
                                       pOutputSettings->uOutputPortSettings.sMaiSettings.eMaiMuxSelector, 
                                       &uiSpdiffmId, 
                                       &uiSpdiffmStreamId); 
            break;
        default:
            BDBG_ERR(("Channel status bits can not be programmed for output port %d", pOutputSettings->eOutput));
            BDBG_ERR(("The only valid ports are SPDIF, Flex and Mai"));         
            return BERR_TRACE (BERR_INVALID_PARAMETER);
    }
    BDBG_MSG(("uiSpdiffmId=%d, uiSpdiffmStreamId=%d", uiSpdiffmId, uiSpdiffmStreamId));    
    hRap->hFmm[0]->hSpdifFm[uiSpdiffmStreamId]->sParams.sChanStatusParams = pOutputSettings->sSpdifChanStatusParams;
    
    /* Get channel handles using this output port. there can be upto 3 channels coz a mixer can take upto 3 inputs */
    ret = BRAP_P_GetChannelsUsingOpPort(hRap, pOutputSettings->eOutput, &hDirChannels[0]);
    if(ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_P_SetCbit: No channels are connected to this output port. Please connect channels and then try again."));
        return BERR_TRACE (ret);
    }    

    /* If there's a decode channel running, write the new CBIT values to DRAM.
    If not => there's only a PB/CAP channel using this port (since this function 
    is called only if the port is already running), directly write to the CBIT registers.
    */

    /* Programming the CBIT for one channel is sufficent even if multiple channels use the same port*/

    for(i=0; i<BRAP_RM_P_MAX_MIXER_INPUTS; i++)
    {
        if((hDirChannels[i] != NULL)  
           && (hDirChannels[i]->eChannelType == BRAP_P_ChannelType_eDecode)
           && (hDirChannels[i]->bStarted == true))
        {
            /* If this is a decode channel and it is running, write the new CBITs to DRAM */
            BRAP_DSPCHN_P_InitSpdifChanStatusParams (
                hDirChannels[i]->sModuleHandles.hDspCh,
                pOutputSettings->sSpdifChanStatusParams);       
            BDBG_MSG(("Decode channel 0x%x is using this port. Wrote new CBIT params to the DRAM.", hDirChannels[i])); 
            /* There can be just one decode channel for this port*/
            bDone=true;
            break;
        }
    }    
    if (bDone == false)
    {
        /* There is no decode channel currently running. Directly write to the CBIT registers */
        for(i=0; i<BRAP_RM_P_MAX_MIXER_INPUTS; i++)
        {
            if((hDirChannels[i] != NULL)  
               && (hDirChannels[i]->eChannelType != BRAP_P_ChannelType_eDecode)
               && (hDirChannels[i]->bStarted == true))
            {
  

                /* BRAP_SPDIFFM_P_ProgramChanStatusBits() reads the new params from the hSpdifFm->sParams*/
                BRAP_SPDIFFM_P_ProgramChanStatusBits(hRap->hFmm[0]->hSpdifFm[uiSpdiffmStreamId]);
                BDBG_MSG(("PB/CAP channel 0x%x is using this port. Programmed CBIT registers for SPDIFFM Stream Id %d", 
                    hDirChannels[i],uiSpdiffmStreamId)); 
                bDone=true;
                break;
            }
        }         
    }

    BDBG_MSG(("Leaving BRAP_P_SetCbit"));   
    BDBG_LEAVE (BRAP_P_SetCbit);
    return ret;        

}
#endif

#if (BCHP_CHIP == 7400)
#if (BRAP_DSPCHN_P_HAS_NEW_TSM_SCHEME==1)
BERR_Code BRAP_P_ProgramAdaptRateCtrl (
		BRAP_ChannelHandle		hRapCh
		)
{

    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegVal = 0,ui32RegVal1=0, ui32Offset=0,ui32Offset1=0,Index=0;   
    uint32_t i=0, uiCount=0,k=0;

    BDBG_ENTER (BRAP_P_ProgramAdaptRateCtrl);
    BDBG_ASSERT (hRapCh);    

    /* If this channel has a SrcCh which carries PCM data, program the 
    AUD_FMM_BF_CTRL_ADAPTRATE_X accordingly */
    ui32Offset = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_1_CFG - 
    BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG;

    
    ui32Offset1=BCHP_AUD_DSP_CFG0_PPM_CONFIG_REG_CXT1 - BCHP_AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0;
    
    Index =hRapCh->sRsrcGrnt.uiDspContextId;

    BRAP_Write32(hRapCh->hRegister, 
          BCHP_AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0+(Index*ui32Offset1), 0xFFFFFFFF); 
    
    
    for(i=0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    {
    	if (hRapCh->sModuleHandles.hSrcCh[i] != NULL) 
    	{
            /* Make sure the SrcCh the SrcCh is carrying PCM data.
            Note: whether the srcch is carrying PCM or Comp is determined only 
            at start time. So make sure the Srcch has been started.*/
            if ((hRapCh->sModuleHandles.hSrcCh[i]->eState == BRAP_SRCCH_P_State_eRunning) 
                && (hRapCh->sModuleHandles.hSrcCh[i]->sParams.bCompress == false))
            {
                 uiCount = hRapCh->sRsrcGrnt.sOpResrcId[i].uiPpmId;  
                k++;

                 ui32RegVal1 = BRAP_Read32(hRapCh->hRegister, 
                            BCHP_AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0 + (Index*ui32Offset1));
                 
                 switch(i)
                {
                    case 0:
                        ui32RegVal1 &= ~(BCHP_MASK (AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0,PPM_MODULE0));                
                        ui32RegVal1 |= (BCHP_FIELD_DATA (AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0, 
                                        PPM_MODULE0,  hRapCh->sRsrcGrnt.sOpResrcId[i].uiPpmId));
                        break;
                    case 1:
                        ui32RegVal1 &= ~(BCHP_MASK (AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0,PPM_MODULE1));                
                        ui32RegVal1 |= (BCHP_FIELD_DATA (AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0, 
                                        PPM_MODULE1,  hRapCh->sRsrcGrnt.sOpResrcId[i].uiPpmId));
                        break;                        
                    case 2:
                        ui32RegVal1 &= ~(BCHP_MASK (AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0,PPM_MODULE2));                
                        ui32RegVal1 |= (BCHP_FIELD_DATA (AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0, 
                                        PPM_MODULE2,  hRapCh->sRsrcGrnt.sOpResrcId[i].uiPpmId));
                        break;                        
                    case 3:
                        ui32RegVal1 &= ~(BCHP_MASK (AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0,PPM_MODULE3));                
                        ui32RegVal1 |= (BCHP_FIELD_DATA (AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0, 
                                        PPM_MODULE3,  hRapCh->sRsrcGrnt.sOpResrcId[i].uiPpmId));
                        
                } 

                BRAP_Write32(hRapCh->hRegister, 
                   BCHP_AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0+(Index*ui32Offset1), ui32RegVal1);                

                ui32RegVal = BRAP_Read32(hRapCh->hRegister, 
                BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG + (uiCount*ui32Offset));

                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_SFIFO_SEL));                
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                ADAPTIVE_SFIFO_SEL, 
                                hRapCh->sModuleHandles.hSrcCh[i]->uiIndex));            

#if (BCHP_CHIP == 7401 && BCHP_VER >= BCHP_VER_C0) || (BCHP_CHIP == 7403) ||(BCHP_CHIP == 7400)
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                TRIWINDOW_WIDTH_SEL));
                /* TODO: hardcoding window width field to 8 ie actual window 
                width of 256. This has to be changed later to be taken from 
                application */
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                TRIWINDOW_WIDTH_SEL, 8));
#else
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                    TRIWINDOW_WIDTH_SEL));
                /* TODO: hardcoding window width field to 8 ie actual window 
                width of 256. This has to be changed later to be taken from 
                application */
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                TRIWINDOW_WIDTH_SEL, 8));
#endif
                /* Earlier, for a decode channel FW was setting the enable flag. 
                Now we're doing AdaptRate control for PCM channels also. So let the 
                PI set this flag always */
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_ENABLE));                
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                ADAPTIVE_RATE_ENABLE, 1));  


                /* Program ADAPTIVE_RATE_THRESHOLD required */
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_THRESHOLD));                
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                ADAPTIVE_RATE_THRESHOLD, 5));  



                BDBG_MSG(("For AdaptRateControl: channel pair %d, SrcFifo %d"
                    ", AdaptRateThreshold=5 and WindowWidth = 256 ", i, 
                    hRapCh->sModuleHandles.hSrcCh[i]->uiIndex));
                
                BRAP_Write32(hRapCh->hRegister, 
                            BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG + (uiCount*ui32Offset), 
                            ui32RegVal);    
                uiCount ++;
                if (uiCount == 4)
                { 
                    BDBG_ERR(("uiCount cant exceed 4. There are only 4 SrcCh on 7401!")); 
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
                }
            }
    	}
    }  

    /* also program it for all the cloned srcCh */
    for(i=0; i < BRAP_RM_P_MAX_OUTPUTS; i++)
    {
    	if (hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][i].hSrcCh!= NULL) 
    	{  
            
    	  
            /* Make sure the SrcCh the SrcCh is carrying PCM data.
            Note: whether the srcch is carrying PCM or Comp is determined only 
            at start time. So make sure the Srcch has been started.*/
            if ((hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][i].hSrcCh->eState == BRAP_SRCCH_P_State_eRunning) 
                && (hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][i].hSrcCh->sParams.bCompress == false))
            {    
                uiCount = hRapCh->sRsrcGrnt.sOpResrcId[k].uiPpmId;

                 ui32RegVal1 = BRAP_Read32(hRapCh->hRegister, 
                            BCHP_AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0 + (Index*ui32Offset1));
                 switch(k)
                {
                    case 0:
                        ui32RegVal1 &= ~(BCHP_MASK (AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0,PPM_MODULE0));                
                        ui32RegVal1 |= (BCHP_FIELD_DATA (AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0, 
                                        PPM_MODULE0,  hRapCh->sRsrcGrnt.sOpResrcId[k].uiPpmId));
                        break;
                    case 1:
                        ui32RegVal1 &= ~(BCHP_MASK (AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0,PPM_MODULE1));                
                        ui32RegVal1 |= (BCHP_FIELD_DATA (AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0, 
                                        PPM_MODULE1,  hRapCh->sRsrcGrnt.sOpResrcId[k].uiPpmId));
                        break;                        
                    case 2:
                        ui32RegVal1 &= ~(BCHP_MASK (AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0,PPM_MODULE2));                
                        ui32RegVal1 |= (BCHP_FIELD_DATA (AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0, 
                                        PPM_MODULE2,  hRapCh->sRsrcGrnt.sOpResrcId[k].uiPpmId));
                        break;                        
                    case 3:
                        ui32RegVal1 &= ~(BCHP_MASK (AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0,PPM_MODULE3));                
                        ui32RegVal1 |= (BCHP_FIELD_DATA (AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0, 
                                        PPM_MODULE3,  hRapCh->sRsrcGrnt.sOpResrcId[k].uiPpmId));
                        
                } 

                BRAP_Write32(hRapCh->hRegister, 
                   BCHP_AUD_DSP_CFG0_PPM_CONFIG_REG_CXT0+(Index*ui32Offset1), ui32RegVal1); 

                ui32RegVal = BRAP_Read32(hRapCh->hRegister, 
                BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG + (uiCount*ui32Offset));

                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_SFIFO_SEL));                
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                ADAPTIVE_SFIFO_SEL, 
                                hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][i].hSrcCh->uiIndex));            

#if (BCHP_CHIP == 7401 && BCHP_VER >= BCHP_VER_C0) || (BCHP_CHIP == 7403) || (BCHP_CHIP == 7400 )
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                TRIWINDOW_WIDTH_SEL));
                /* TODO: hardcoding window width field to 8 ie actual window 
                width of 256. This has to be changed later to be taken from 
                application */
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                TRIWINDOW_WIDTH_SEL, 8));
#else
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                    TRIWINDOW_WIDTH_SEL));
                /* TODO: hardcoding window width field to 8 ie actual window 
                width of 256. This has to be changed later to be taken from 
                application */
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                TRIWINDOW_WIDTH_SEL, 8));
#endif

                /* Earlier, for a decode channel FW was setting the enable flag. 
                Now we're doing AdaptRate control for PCM channels also. So let the 
                PI set this flag always */
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_ENABLE));                
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                ADAPTIVE_RATE_ENABLE, 1));  


                /* Program ADAPTIVE_RATE_THRESHOLD if required */
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_THRESHOLD));                
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                ADAPTIVE_RATE_THRESHOLD, 5));  


                BDBG_MSG(("For AdaptRateControl: channel pair %d, cloned SrcFifo %d"
                    ", AdaptRateThreshold=5 and WindowWidth=256 ", i, 
                    hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][i].hSrcCh->uiIndex));
                
                BRAP_Write32(hRapCh->hRegister, 
                            BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG + (uiCount*ui32Offset), 
                            ui32RegVal);                
            }
    	}
    }  
    
    BDBG_LEAVE (BRAP_P_ProgramAdaptRateCtrl);
    return ret;    
}
#endif

#else /* For 7401,7403 and 7118*/

#if (BRAP_DSPCHN_P_HAS_NEW_TSM_SCHEME==1)
BERR_Code BRAP_P_ProgramAdaptRateCtrl (
		BRAP_ChannelHandle		hRapCh
		)
{

    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegVal = 0, ui32Offset=0;   
    int i, count=0;

    BDBG_ENTER (BRAP_P_ProgramAdaptRateCtrl);
    BDBG_ASSERT (hRapCh);    

    /* If this channel has a SrcCh which carries PCM data, program the 
    AUD_FMM_BF_CTRL_ADAPTRATE_X accordingly */
    ui32Offset = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_1_CFG - 
    BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG;

    for(i=0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    {
    	if (hRapCh->sModuleHandles.hSrcCh[i] != NULL) 
    	{
            /* Make sure the SrcCh the SrcCh is carrying PCM data.
            Note: whether the srcch is carrying PCM or Comp is determined only 
            at start time. So make sure the Srcch has been started.*/
            if ((hRapCh->sModuleHandles.hSrcCh[i]->eState == BRAP_SRCCH_P_State_eRunning) 
                && (hRapCh->sModuleHandles.hSrcCh[i]->sParams.bCompress == false))
            {
                ui32RegVal = BRAP_Read32(hRapCh->hRegister, 
                BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG + (count*ui32Offset));

                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_SFIFO_SEL));                
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                ADAPTIVE_SFIFO_SEL, 
                                hRapCh->sModuleHandles.hSrcCh[i]->uiIndex));            

#if (BCHP_CHIP == 7401 && BCHP_VER >= BCHP_VER_C0) || (BCHP_CHIP == 7403)
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                TRIWINDOW_WIDTH_SEL));
                /* TODO: hardcoding window width field to 8 ie actual window 
                width of 256. This has to be changed later to be taken from 
                application */
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                TRIWINDOW_WIDTH_SEL, 8));
#else
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                    TRIWINDOW_WDITH_SEL));
                /* TODO: hardcoding window width field to 8 ie actual window 
                width of 256. This has to be changed later to be taken from 
                application */
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                TRIWINDOW_WDITH_SEL, 8));
#endif
                /* Earlier, for a decode channel FW was setting the enable flag. 
                Now we're doing AdaptRate control for PCM channels also. So let the 
                PI set this flag always */
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_ENABLE));                
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                ADAPTIVE_RATE_ENABLE, 1));  


                /* Program ADAPTIVE_RATE_THRESHOLD required */
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_THRESHOLD));                
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                ADAPTIVE_RATE_THRESHOLD, 5));  



                BDBG_MSG(("For AdaptRateControl: channel pair %d, SrcFifo %d"
                    ", AdaptRateThreshold=5 and WindowWidth = 256 ", i, 
                    hRapCh->sModuleHandles.hSrcCh[i]->uiIndex));
                
                BRAP_Write32(hRapCh->hRegister, 
                            BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG + (count*ui32Offset), 
                            ui32RegVal);                
                count ++;
                if (count == 4)
                { 
                    BDBG_ERR(("Count cant exceed 4. There are only 4 SrcCh on 7401!")); 
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
                }
            }
    	}
    }  

    /* also program it for all the cloned srcCh */
    for(i=0; i < BRAP_RM_P_MAX_OUTPUTS; i++)
    {
    	if (hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][i].hSrcCh!= NULL) 
    	{
            /* Make sure the SrcCh the SrcCh is carrying PCM data.
            Note: whether the srcch is carrying PCM or Comp is determined only 
            at start time. So make sure the Srcch has been started.*/
            if ((hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][i].hSrcCh->eState == BRAP_SRCCH_P_State_eRunning) 
                && (hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][i].hSrcCh->sParams.bCompress == false))
            {
                ui32RegVal = BRAP_Read32(hRapCh->hRegister, 
                BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG + (count*ui32Offset));

                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_SFIFO_SEL));                
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                ADAPTIVE_SFIFO_SEL, 
                                hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][i].hSrcCh->uiIndex));            

#if (BCHP_CHIP == 7401 && BCHP_VER >= BCHP_VER_C0) || (BCHP_CHIP == 7403)
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                TRIWINDOW_WIDTH_SEL));
                /* TODO: hardcoding window width field to 8 ie actual window 
                width of 256. This has to be changed later to be taken from 
                application */
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                TRIWINDOW_WIDTH_SEL, 8));
#else
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                    TRIWINDOW_WDITH_SEL));
                /* TODO: hardcoding window width field to 8 ie actual window 
                width of 256. This has to be changed later to be taken from 
                application */
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                TRIWINDOW_WDITH_SEL, 8));
#endif

                /* Earlier, for a decode channel FW was setting the enable flag. 
                Now we're doing AdaptRate control for PCM channels also. So let the 
                PI set this flag always */
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_ENABLE));                
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                ADAPTIVE_RATE_ENABLE, 1));  


                /* Program ADAPTIVE_RATE_THRESHOLD if required */
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_THRESHOLD));                
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, 
                                ADAPTIVE_RATE_THRESHOLD, 5));  


                BDBG_MSG(("For AdaptRateControl: channel pair %d, cloned SrcFifo %d"
                    ", AdaptRateThreshold=5 and WindowWidth=256 ", i, 
                    hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][i].hSrcCh->uiIndex));
                
                BRAP_Write32(hRapCh->hRegister, 
                            BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG + (count*ui32Offset), 
                            ui32RegVal);                
            }
    	}
    }  
    
    BDBG_LEAVE (BRAP_P_ProgramAdaptRateCtrl);
    return ret;    
}
#endif

#endif

#if BCHP_7411_VER > BCHP_VER_C0
#define BRAP_P_I2S2_CFG_OFFSET	(0x8)
BERR_Code BRAP_P_I2sAlignmentWorkaround (
		BRAP_ChannelHandle		hRapCh
		)
{
	unsigned int regVal = 0;
	
	/* For I2S alignment to work, all three I2S ports should be enabled and drawing data. For
	 * 7411D multichannel mode, we get two I2S ports from application. Start third I2S port 
	 * internally. We need to start corresponding mixer also.
	 * The steps are
	 * Enable I2S2 port
	 * Enable Mixer 1 of DP1
	 * Share source channel corresponding to I2S1 with I2S2
	 */
	 BDBG_WRN(("For I2S alignment to work, all three I2S ports needs to be enabled"
	 			" Enabling I2S 2 internally."));

	/* Enable I2S 2 port */
	regVal = BRAP_Read32(hRapCh->hRegister, BCHP_AUD_FMM_OP_CTRL_I2S_CFG_i_ARRAY_BASE + BRAP_P_I2S2_CFG_OFFSET);
	
	regVal = (regVal & ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_i, CLOCK_ENABLE)))
			| BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_i, CLOCK_ENABLE, Enable);

	regVal = (regVal & ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_i, DATA_ENABLE)))
			| BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_i, DATA_ENABLE, Enable);

	regVal = (regVal & ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_i, MCLK_RATE)))
			| BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_i, MCLK_RATE, MCLK_128fs_SCLK_64fs);

	regVal = (regVal & ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_i, BITS_PER_SAMPLE)))
			| BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_i, BITS_PER_SAMPLE, Bitwidth24);
	
	regVal = (regVal & ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_i, BITS_PER_SAMPLE)))
			| BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_i, BITS_PER_SAMPLE, Bitwidth24);

	BDBG_ERR(("BCHP_AUD_FMM_OP_CTRL_I2S_CFG_i_ARRAY_BASE = 0x%08x", regVal));
	BRAP_Write32(hRapCh->hRegister, BCHP_AUD_FMM_OP_CTRL_I2S_CFG_i_ARRAY_BASE + BRAP_P_I2S2_CFG_OFFSET, regVal);

	BRAP_Write32(hRapCh->hRegister, BCHP_AUD_FMM_OP_CTRL_ENABLE_SET, 0x10);
	
	/* Enable Mixer1 of DP1 */
	regVal = BRAP_Read32(hRapCh->hRegister, BCHP_AUD_FMM_DP_CTRL1_MS_CLIENT1_MIX1_CONFIG);

	regVal = (regVal & ~(BCHP_MASK(AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, MIX_PB_ID)))
			| BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, MIX_PB_ID, 0x0);

	regVal = (regVal & ~(BCHP_MASK(AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, MIX_ENABLE)))
			| BCHP_FIELD_ENUM(AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, MIX_ENABLE, Enable);

	BDBG_ERR(("BCHP_AUD_FMM_DP_CTRL1_MS_CLIENT1_MIX1_CONFIG = 0x%08x", regVal));
	BRAP_Write32(hRapCh->hRegister, BCHP_AUD_FMM_DP_CTRL1_MS_CLIENT1_MIX1_CONFIG, regVal);

	regVal = BRAP_Read32(hRapCh->hRegister, BCHP_AUD_FMM_DP_CTRL1_STRM_FORMAT);

	regVal = (regVal & ~(BCHP_MASK(AUD_FMM_DP_CTRL0_STRM_FORMAT, STREAM0_BIT_RESOLUTION)))
			| BCHP_FIELD_ENUM(AUD_FMM_DP_CTRL0_STRM_FORMAT, STREAM0_BIT_RESOLUTION, RES_24);

	BRAP_Write32(hRapCh->hRegister, BCHP_AUD_FMM_DP_CTRL1_STRM_FORMAT, regVal);

	regVal = BRAP_Read32(hRapCh->hRegister, BCHP_AUD_FMM_DP_CTRL1_STRM_ENA);

	regVal = (regVal & ~(BCHP_MASK(AUD_FMM_DP_CTRL0_STRM_ENA, STREAM1_ENA)))
			| BCHP_FIELD_ENUM(AUD_FMM_DP_CTRL0_STRM_ENA, STREAM1_ENA, Enable);

	BDBG_ERR(("BCHP_AUD_FMM_DP_CTRL1_STRM_ENA = 0x%08x", regVal));
	BRAP_Write32(hRapCh->hRegister, BCHP_AUD_FMM_DP_CTRL1_STRM_ENA, regVal);

	return BERR_SUCCESS;
}

BERR_Code
BRAP_P_I2sAlignmentWorkaroundStop	( BRAP_ChannelHandle		hRapCh
		)
{
	unsigned int regVal = 0;

	regVal = BRAP_Read32(hRapCh->hRegister, BCHP_AUD_FMM_DP_CTRL1_STRM_ENA);

	regVal = (regVal & ~(BCHP_MASK(AUD_FMM_DP_CTRL0_STRM_ENA, STREAM1_ENA)))
			| BCHP_FIELD_ENUM(AUD_FMM_DP_CTRL0_STRM_ENA, STREAM1_ENA, Disable);

	BDBG_ERR(("BCHP_AUD_FMM_DP_CTRL1_STRM_ENA = 0x%08x", regVal));
	BRAP_Write32(hRapCh->hRegister, BCHP_AUD_FMM_DP_CTRL1_STRM_ENA, regVal);

	regVal = BRAP_Read32(hRapCh->hRegister, BCHP_AUD_FMM_DP_CTRL1_MS_CLIENT1_MIX1_CONFIG);

	regVal = (regVal & ~(BCHP_MASK(AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, MIX_PB_ID)))
			| BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, MIX_PB_ID, 0xF);
#if 0 
	regVal = (regVal & ~(BCHP_MASK(AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, MIX_ENABLE)))
			| BCHP_FIELD_ENUM(AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, MIX_ENABLE, Disable);
#endif

	BDBG_ERR(("BCHP_AUD_FMM_DP_CTRL1_MS_CLIENT1_MIX1_CONFIG = 0x%08x", regVal));
	BRAP_Write32(hRapCh->hRegister, BCHP_AUD_FMM_DP_CTRL1_MS_CLIENT1_MIX1_CONFIG, regVal);
#if 0
	regVal = BRAP_Read32(hRapCh->hRegister, BCHP_AUD_FMM_OP_CTRL_I2S_CFG_i_ARRAY_BASE + BRAP_P_I2S2_CFG_OFFSET);
	
	regVal = (regVal & ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_i, CLOCK_ENABLE)))
			| BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_i, CLOCK_ENABLE, Enable);

	regVal = (regVal & ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_i, DATA_ENABLE)))
			| BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_i, DATA_ENABLE, Disable);

	regVal = (regVal & ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_i, MCLK_RATE)))
			| BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_i, MCLK_RATE, MCLK_128fs_SCLK_64fs);

	regVal = (regVal & ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_i, BITS_PER_SAMPLE)))
			| BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_i, BITS_PER_SAMPLE, Bitwidth24);
	
	regVal = (regVal & ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_i, BITS_PER_SAMPLE)))
			| BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_i, BITS_PER_SAMPLE, Bitwidth24);

	BDBG_ERR(("BCHP_AUD_FMM_OP_CTRL_I2S_CFG_i_ARRAY_BASE = 0x%08x", regVal));
	BRAP_Write32(hRapCh->hRegister, BCHP_AUD_FMM_OP_CTRL_I2S_CFG_i_ARRAY_BASE + BRAP_P_I2S2_CFG_OFFSET, regVal);


	regVal = BRAP_Read32(hRapCh->hRegister, BCHP_AUD_FMM_OP_CTRL_I2S_CFG_i_ARRAY_BASE + BRAP_P_I2S2_CFG_OFFSET);

	regVal = (regVal & ~(BCHP_MASK(AUD_FMM_OP_CTRL_I2S_CFG_i, DATA_ENABLE)))
	| BCHP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG_i, DATA_ENABLE, Disable);


	BDBG_ERR(("BCHP_AUD_FMM_OP_CTRL_I2S_CFG_i_ARRAY_BASE = 0x%08x", regVal));
	BRAP_Write32(hRapCh->hRegister, BCHP_AUD_FMM_OP_CTRL_I2S_CFG_i_ARRAY_BASE + BRAP_P_I2S2_CFG_OFFSET, regVal);


#endif


	regVal = 0x80010; /* To disable the Alignment at stop */
	BRAP_Write32(hRapCh->hRegister, BCHP_AUD_FMM_OP_CTRL_ENABLE_CLEAR, regVal );


	return BERR_SUCCESS;

}

BRAP_AudioMemAllocation BRAP_P_GetMemoryAllocationType (
							BRAP_Handle			hRap		/* [in] RAP Handle */
							)
{
	return hRap->sSettings.eMemAllocation;
}
#endif

#if ( BCHP_CHIP == 7400 )
/**************************************************************************
Summary:
    Private function that Resets the CHP_AUD_FMM_BF_CTRL_REARM_FREEFULL_MARK
    bit for the SrcCh which is unmasked to recieve the interrupt.
    Resetting this bit is a part of hardware workaround. For resetting the 
    bit, First set it to 0, then set the bit to 1 again
**************************************************************************/

void BRAP_P_ReArmSrcChInterrupt(
	BRAP_ChannelHandle	hRapch
	)
{
	BDBG_ENTER(BRAP_P_ReArmSrcChInterrupt);
	BDBG_ASSERT(hRapch);

    BKNI_EnterCriticalSection();
    BRAP_P_ReArmSrcChInterrupt_isr(hRapch);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_P_ReArmSrcChInterrupt);
    return;
}

/**************************************************************************
Summary:
	Isr version of BRAP_P_ReArmSrcChInterrupt.
**************************************************************************/

void BRAP_P_ReArmSrcChInterrupt_isr(
	BRAP_ChannelHandle	hRapch
	)
{
	uint32_t 	ui32RegVal =0x0,ui32ReArmBit=0x0;
	
	BDBG_ENTER(BRAP_P_ReArmSrcChInterrupt_isr);
	BDBG_ASSERT(hRapch);
	
	ui32ReArmBit |= (1 << hRapch->sRsrcGrnt.sOpResrcId[0].uiSrcChId);

	BDBG_MSG(("ui32ReArmBit = %d",ui32ReArmBit));
	/* First set 0 to Re-Arm bit of source channel, then set it to 1 */
	
	ui32RegVal = BRAP_Read32_isr(hRapch->hRegister,BCHP_AUD_FMM_BF_CTRL_REARM_FREEFULL_MARK );
	ui32RegVal = (~ui32ReArmBit) & ui32RegVal;
	BRAP_Write32_isr (hRapch->hRegister, BCHP_AUD_FMM_BF_CTRL_REARM_FREEFULL_MARK,ui32RegVal);
	
	ui32RegVal = BRAP_Read32_isr(hRapch->hRegister,BCHP_AUD_FMM_BF_CTRL_REARM_FREEFULL_MARK );
	ui32RegVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_REARM_FREEFULL_MARK, 
                        REARM_FREEMARK, ui32ReArmBit);
	BRAP_Write32_isr (hRapch->hRegister,  
              BCHP_AUD_FMM_BF_CTRL_REARM_FREEFULL_MARK,ui32RegVal);
	
	BDBG_LEAVE(BRAP_P_ReArmSrcChInterrupt_isr);
	return;
}
#endif

/* End Of File */

