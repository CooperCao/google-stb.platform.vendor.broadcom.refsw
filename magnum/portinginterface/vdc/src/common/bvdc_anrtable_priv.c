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
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include "bstd.h"
#include "bvdc.h"
#include "bvdc_anr_priv.h"

BDBG_MODULE(BVDC_ANR_TBL);

/****************************************************************************/
/* Making an entry for ANR table                                            */
/****************************************************************************/
#define BVDC_P_MAKE_ANR(ALTdMC, ALTdNMC, McK0, McK1, NonMcK0, NonMcK1, \
	FinalK0, FinalK1, Bypass)                                          \
{                                                                      \
	(ALTdMC), (ALTdNMC), (McK0), (McK1), (NonMcK0), (NonMcK1),         \
	(0), (0), (0), (0),                                                \
	(FinalK0), (FinalK1), (0), (0), (Bypass), 0                        \
}
/***********************************************************************
 *
 *  This table could be replaced by customer to achieve different ANR
 *  filtering strength. If you do so, please make sure that macro
 *  BVDC_P_ANR_MIN_SN and BVDC_P_ANR_MAX_SN are defined to match the
 *  number of table entries
 *
 */
#define BVDC_P_ANR_MIN_SN     20
#define BVDC_P_ANR_MAX_SN     60

/* K table for RF */
static const BVDC_P_AnrKValue s_ulAnrKValueForRf[] = {
	/*ALTdMC ALTdNMC    McK0  McK1 NonMcK0 NonMcK1 FinalK0 FinalK1 bBypass*/
	BVDC_P_MAKE_ANR( 50,     50,    260,  1200,    260,   750,    260,   180,  false),    /* S/N: 20 */
	BVDC_P_MAKE_ANR( 50,     50,    260,  1150,    260,   700,    260,   175,  false),    /* S/N: 21 */
	BVDC_P_MAKE_ANR( 50,     50,    260,  1100,    260,   650,    260,   170,  false),    /* S/N: 22 */
	BVDC_P_MAKE_ANR( 50,     50,    260,  1050,    260,   600,    260,   165,  false),    /* S/N: 23 */
	BVDC_P_MAKE_ANR( 50,     50,    260,  1000,    260,   550,    260,   160,  false),    /* S/N: 24 */
	BVDC_P_MAKE_ANR( 50,     50,    260,   950,    260,   500,    260,   155,  false),    /* S/N: 25 */
	BVDC_P_MAKE_ANR( 50,     50,    260,   900,    260,   450,    260,   150,  false),    /* S/N: 26 */
	BVDC_P_MAKE_ANR( 50,     50,    260,   850,    260,   400,    260,   145,  false),    /* S/N: 27 */
	BVDC_P_MAKE_ANR( 50,     50,    260,   800,    260,   350,    260,   140,  false),    /* S/N: 28 */
	BVDC_P_MAKE_ANR( 53,     53,    260,   750,    260,   310,    260,   135,  false),    /* S/N: 29 */
	BVDC_P_MAKE_ANR( 56,     56,    260,   700,    260,   270,    260,   130,  false),    /* S/N: 30 */
	BVDC_P_MAKE_ANR( 59,     59,    260,   650,    260,   230,    260,   125,  false),    /* S/N: 31 */
	BVDC_P_MAKE_ANR( 62,     62,    260,   600,    260,   200,    260,   120,  false),    /* S/N: 32 */
	BVDC_P_MAKE_ANR( 65,     65,    260,   550,    260,   170,    260,   116,  false),    /* S/N: 33 */
	BVDC_P_MAKE_ANR( 68,     68,    260,   500,    260,   140,    260,   112,  false),    /* S/N: 34 */
	BVDC_P_MAKE_ANR( 71,     71,    260,   450,    260,   120,    260,   108,  false),    /* S/N: 35 */
	BVDC_P_MAKE_ANR( 74,     74,    260,   400,    260,   110,    260,   104,  false),    /* S/N: 36 */
	BVDC_P_MAKE_ANR( 77,     77,    260,   350,    260,   100,    260,   100,  false),    /* S/N: 37 */
	BVDC_P_MAKE_ANR( 80,     80,    260,   320,    260,    90,    260,    96,  false),    /* S/N: 38 */
	BVDC_P_MAKE_ANR( 83,     83,    260,   300,    260,    80,    260,    92,  false),    /* S/N: 39 */
	BVDC_P_MAKE_ANR( 86,     86,    260,   280,    260,    70,    260,    88,  false),    /* S/N: 40 */
	BVDC_P_MAKE_ANR( 89,     89,    260,   250,    260,    64,    260,    84,  false),    /* S/N: 41 */
	BVDC_P_MAKE_ANR( 92,     92,    260,   225,    260,    58,    260,    80,  false),    /* S/N: 42 */
	BVDC_P_MAKE_ANR(101,    101,    260,   200,    260,    52,    260,    76,  false),    /* S/N: 43 */
	BVDC_P_MAKE_ANR(104,    104,    260,   170,    260,    46,    260,    72,  false),    /* S/N: 44 */
	BVDC_P_MAKE_ANR(107,    107,    260,   140,    260,    40,    260,    68,  false),    /* S/N: 45 */
	BVDC_P_MAKE_ANR(110,    110,    260,   120,    260,    34,    260,    64,  false),    /* S/N: 46 */
	BVDC_P_MAKE_ANR(113,    113,    260,   100,    260,    28,    260,    60,  false),    /* S/N: 47 */
	BVDC_P_MAKE_ANR(116,    116,    260,    75,    260,    22,    260,    56,  false),    /* S/N: 48 */
	BVDC_P_MAKE_ANR(119,    119,    260,    58,    260,    16,    260,    52,  false),    /* S/N: 49 */
	BVDC_P_MAKE_ANR(122,    122,    260,    40,    260,    10,    260,    48,  false),    /* S/N: 50 */
	BVDC_P_MAKE_ANR(125,    125,    260,    28,    260,     8,    260,    44,  false),    /* S/N: 51 */
	BVDC_P_MAKE_ANR(128,    128,    260,    20,    260,     6,    260,    40,  false),    /* S/N: 52 */
	BVDC_P_MAKE_ANR(131,    131,    260,    14,    260,     5,    260,    36,  false),    /* S/N: 53 */
	BVDC_P_MAKE_ANR(134,    134,    260,     8,    260,     4,    260,    32,  false),    /* S/N: 54 */
	BVDC_P_MAKE_ANR(137,    137,    260,     5,    260,     3,    260,    28,  false),    /* S/N: 55 */
	BVDC_P_MAKE_ANR(140,    140,    260,     3,    260,     2,    260,    24,  false),    /* S/N: 56 */
	BVDC_P_MAKE_ANR(143,    143,    260,     1,    260,     1,    260,    20,  false),    /* S/N: 57 */
	BVDC_P_MAKE_ANR(146,    146,    260,     1,    260,     1,    260,    16,  false),    /* S/N: 58 */
	BVDC_P_MAKE_ANR(149,    149,    260,     1,    260,     1,    260,    12,  false),    /* S/N: 59 */
	BVDC_P_MAKE_ANR(152,    152,    260,     1,    260,     1,    260,     8,  false),    /* S/N: 60 */
};

/* K table for non-RF SD */
static const BVDC_P_AnrKValue s_ulAnrKValueForSd[] = {
	/*ALTdMC ALTdNMC    McK0  McK1 NonMcK0 NonMcK1 FinalK0 FinalK1 bBypass*/
	BVDC_P_MAKE_ANR( 50,     50,    260,   840,    260,  1900,    260,   142,  false),    /* S/N: 20 */
	BVDC_P_MAKE_ANR( 50,     50,    260,   810,    260,  1700,    260,   132,  false),    /* S/N: 21 */
	BVDC_P_MAKE_ANR( 50,     50,    260,   780,    260,  1500,    260,   122,  false),    /* S/N: 22 */
	BVDC_P_MAKE_ANR( 50,     50,    260,   750,    260,  1300,    260,   112,  false),    /* S/N: 23 */
	BVDC_P_MAKE_ANR( 53,     53,    260,   720,    260,  1100,    260,   102,  false),    /* S/N: 24 */
	BVDC_P_MAKE_ANR( 56,     56,    260,   690,    260,   900,    260,    92,  false),    /* S/N: 25 */
	BVDC_P_MAKE_ANR( 59,     59,    260,   660,    260,   700,    260,    82,  false),    /* S/N: 26 */
	BVDC_P_MAKE_ANR( 62,     62,    260,   630,    260,   600,    260,    72,  false),    /* S/N: 27 */
	BVDC_P_MAKE_ANR( 65,     65,    260,   600,    260,   500,    260,    62,  false),    /* S/N: 28 */
	BVDC_P_MAKE_ANR( 68,     68,    260,   560,    260,   400,    260,    52,  false),    /* S/N: 29 */
	BVDC_P_MAKE_ANR( 71,     71,    260,   530,    260,   300,    260,    42,  false),    /* S/N: 30 */
	BVDC_P_MAKE_ANR( 74,     74,    260,   500,    260,   250,    260,    32,  false),    /* S/N: 31 */
	BVDC_P_MAKE_ANR( 77,     77,    260,   470,    260,   200,    260,    22,  false),    /* S/N: 32 */
	BVDC_P_MAKE_ANR( 80,     80,    260,   440,    260,   150,    260,    21,  false),    /* S/N: 33 */
	BVDC_P_MAKE_ANR( 83,     83,    260,   410,    260,   140,    260,    21,  false),    /* S/N: 34 */
	BVDC_P_MAKE_ANR( 86,     86,    260,   380,    260,   130,    260,    20,  false),    /* S/N: 35 */
	BVDC_P_MAKE_ANR( 89,     89,    260,   360,    260,   120,    260,    20,  false),    /* S/N: 36 */
	BVDC_P_MAKE_ANR( 92,     92,    260,   340,    260,   110,    260,    19,  false),    /* S/N: 37 */
	BVDC_P_MAKE_ANR(101,    101,    260,   320,    260,   100,    260,    19,  false),    /* S/N: 38 */
	BVDC_P_MAKE_ANR(104,    104,    260,   300,    260,    90,    260,    18,  false),    /* S/N: 39 */
	BVDC_P_MAKE_ANR(107,    107,    260,   280,    260,    80,    260,    18,  false),    /* S/N: 40 */
	BVDC_P_MAKE_ANR(110,    110,    260,   250,    260,    70,    260,    16,  false),    /* S/N: 41 */
	BVDC_P_MAKE_ANR(115,    115,    260,   225,    260,    65,    260,    13,  false),    /* S/N: 42 */
	BVDC_P_MAKE_ANR(118,    118,    260,   200,    260,    60,    260,    11,  false),    /* S/N: 43 */
	BVDC_P_MAKE_ANR(122,    122,    260,   170,    260,    55,    260,     9,  false),    /* S/N: 44 */
	BVDC_P_MAKE_ANR(128,    128,    260,   140,    260,    50,    260,     7,  false),    /* S/N: 45 */
	BVDC_P_MAKE_ANR(135,    135,    260,   120,    260,    40,    260,     7,  false),    /* S/N: 46 */
	BVDC_P_MAKE_ANR(138,    138,    260,   100,    260,    30,    260,     7,  false),    /* S/N: 47 */
	BVDC_P_MAKE_ANR(141,    141,    260,    75,    260,    20,    260,     6,  false),    /* S/N: 48 */
	BVDC_P_MAKE_ANR(144,    144,    260,    58,    260,    10,    260,     5,  false),    /* S/N: 49 */
	BVDC_P_MAKE_ANR(147,    147,    260,    40,    260,     9,    260,     5,  false),    /* S/N: 50 */
	BVDC_P_MAKE_ANR(150,    150,    260,    28,    260,     8,    260,     4,  false),    /* S/N: 51 */
	BVDC_P_MAKE_ANR(153,    153,    260,    20,    260,     7,    260,     4,  false),    /* S/N: 52 */
	BVDC_P_MAKE_ANR(156,    156,    260,    14,    260,     6,    260,     4,  false),    /* S/N: 53 */
	BVDC_P_MAKE_ANR(159,    159,    260,     8,    260,     5,    260,     4,  false),    /* S/N: 54 */
	BVDC_P_MAKE_ANR(162,    162,    260,     5,    260,     4,    260,     3,  false),    /* S/N: 55 */
	BVDC_P_MAKE_ANR(165,    165,    260,     3,    260,     3,    260,     3,  false),    /* S/N: 56 */
	BVDC_P_MAKE_ANR(168,    168,    260,     1,    260,     2,    260,     2,  false),    /* S/N: 57 */
	BVDC_P_MAKE_ANR(168,    168,    260,     1,    260,     1,    260,     2,  false),    /* S/N: 58 */
	BVDC_P_MAKE_ANR(168,    168,    260,     1,    260,     1,    260,     1,  false),    /* S/N: 59 */
	BVDC_P_MAKE_ANR(168,    168,    260,     1,    260,     1,    260,     1,  false),    /* S/N: 60 */
};

/* K table for ED */
static const BVDC_P_AnrKValue s_ulAnrKValueForEd[] = {
	/*ALTdMC ALTdNMC    McK0  McK1 NonMcK0 NonMcK1 FinalK0 FinalK1 bBypass*/
	BVDC_P_MAKE_ANR( 50,     50,    260,  1000,    260,  1300,    260,   172,  false),    /* S/N: 20 */
	BVDC_P_MAKE_ANR( 50,     50,    260,  1000,    260,  1200,    260,   162,  false),    /* S/N: 21 */
	BVDC_P_MAKE_ANR( 50,     50,    260,  1000,    260,  1100,    260,   152,  false),    /* S/N: 22 */
	BVDC_P_MAKE_ANR( 50,     50,    260,  1000,    260,  1000,    260,   142,  false),    /* S/N: 23 */
	BVDC_P_MAKE_ANR( 50,     50,    260,  1000,    260,   900,    260,   132,  false),    /* S/N: 24 */
	BVDC_P_MAKE_ANR( 50,     50,    260,  1000,    260,   800,    260,   122,  false),    /* S/N: 25 */
	BVDC_P_MAKE_ANR( 53,     53,    260,  1000,    260,   700,    260,   112,  false),    /* S/N: 26 */
	BVDC_P_MAKE_ANR( 56,     56,    260,  1000,    260,   600,    260,   102,  false),    /* S/N: 27 */
	BVDC_P_MAKE_ANR( 59,     59,    260,  1000,    260,   500,    260,    92,  false),    /* S/N: 28 */
	BVDC_P_MAKE_ANR( 62,     62,    260,  1000,    260,   400,    260,    82,  false),    /* S/N: 29 */
	BVDC_P_MAKE_ANR( 65,     65,    260,  1000,    260,   350,    260,    72,  false),    /* S/N: 30 */
	BVDC_P_MAKE_ANR( 68,     68,    260,   950,    260,   300,    260,    62,  false),    /* S/N: 31 */
	BVDC_P_MAKE_ANR( 71,     71,    260,   900,    260,   250,    260,    52,  false),    /* S/N: 32 */
	BVDC_P_MAKE_ANR( 74,     74,    260,   850,    260,   200,    260,    42,  false),    /* S/N: 33 */
	BVDC_P_MAKE_ANR( 77,     77,    260,   800,    260,   170,    260,    32,  false),    /* S/N: 34 */
	BVDC_P_MAKE_ANR( 80,     80,    260,   750,    260,   140,    260,    22,  false),    /* S/N: 35 */
	BVDC_P_MAKE_ANR( 83,     83,    260,   700,    260,   130,    260,    21,  false),    /* S/N: 36 */
	BVDC_P_MAKE_ANR( 86,     86,    260,   650,    260,   125,    260,    21,  false),    /* S/N: 37 */
	BVDC_P_MAKE_ANR( 89,     89,    260,   600,    260,   120,    260,    20,  false),    /* S/N: 38 */
	BVDC_P_MAKE_ANR( 92,     92,    260,   550,    260,   115,    260,    20,  false),    /* S/N: 39 */
	BVDC_P_MAKE_ANR(101,    101,    260,   500,    260,   110,    260,    19,  false),    /* S/N: 40 */
	BVDC_P_MAKE_ANR(104,    104,    260,   450,    260,   105,    260,    19,  false),    /* S/N: 41 */
	BVDC_P_MAKE_ANR(107,    107,    260,   400,    260,   100,    260,    18,  false),    /* S/N: 42 */
	BVDC_P_MAKE_ANR(110,    110,    260,   350,    260,    90,    260,    18,  false),    /* S/N: 43 */
	BVDC_P_MAKE_ANR(113,    113,    260,   300,    260,    80,    260,    16,  false),    /* S/N: 44 */
	BVDC_P_MAKE_ANR(116,    116,    260,   250,    260,    70,    260,    13,  false),    /* S/N: 45 */
	BVDC_P_MAKE_ANR(119,    119,    260,   200,    260,    60,    260,    11,  false),    /* S/N: 46 */
	BVDC_P_MAKE_ANR(122,    122,    260,   150,    260,    50,    260,     9,  false),    /* S/N: 47 */
	BVDC_P_MAKE_ANR(125,    125,    260,   100,    260,    40,    260,     7,  false),    /* S/N: 48 */
	BVDC_P_MAKE_ANR(128,    128,    260,    75,    260,    35,    260,     7,  false),    /* S/N: 49 */
	BVDC_P_MAKE_ANR(131,    131,    260,    50,    260,    30,    260,     7,  false),    /* S/N: 50 */
	BVDC_P_MAKE_ANR(134,    134,    260,    40,    260,    25,    260,     6,  false),    /* S/N: 51 */
	BVDC_P_MAKE_ANR(137,    137,    260,    30,    260,    20,    260,     5,  false),    /* S/N: 52 */
	BVDC_P_MAKE_ANR(140,    140,    260,    20,    260,    15,    260,     5,  false),    /* S/N: 53 */
	BVDC_P_MAKE_ANR(143,    143,    260,    15,    260,    10,    260,     4,  false),    /* S/N: 54 */
	BVDC_P_MAKE_ANR(146,    146,    260,    11,    260,     6,    260,     4,  false),    /* S/N: 55 */
	BVDC_P_MAKE_ANR(149,    149,    260,     9,    260,     5,    260,     4,  false),    /* S/N: 56 */
	BVDC_P_MAKE_ANR(152,    152,    260,     7,    260,     4,    260,     4,  false),    /* S/N: 57 */
	BVDC_P_MAKE_ANR(155,    155,    260,     5,    260,     3,    260,     3,  false),    /* S/N: 58 */
	BVDC_P_MAKE_ANR(158,    158,    260,     3,    260,     2,    260,     3,  false),    /* S/N: 59 */
	BVDC_P_MAKE_ANR(161,    161,    260,     1,    260,     1,    260,     2,  false),    /* S/N: 60 */
};



/* K table for HD */
static const BVDC_P_AnrKValue s_ulAnrKValueForHd[] = {
	/*ALTdMC ALTdNMC    McK0  McK1 NonMcK0 NonMcK1 FinalK0 FinalK1 bBypass*/
	BVDC_P_MAKE_ANR( 50,     50,    260,   390,    260,  1100,    260,   132,  false),    /* S/N: 20*/
	BVDC_P_MAKE_ANR( 50,     50,    260,   380,    260,  1000,    260,   127,  false),    /* S/N: 21 */
	BVDC_P_MAKE_ANR( 50,     50,    260,   370,    260,   900,    260,   112,  false),    /* S/N: 22 */
	BVDC_P_MAKE_ANR( 50,     50,    260,   360,    260,   800,    260,   107,  false),    /* S/N: 23 */
	BVDC_P_MAKE_ANR( 50,     50,    260,   350,    260,   700,    260,   102,  false),    /* S/N: 24 */
	BVDC_P_MAKE_ANR( 50,     50,    260,   340,    260,   600,    260,    97,  false),    /* S/N: 25 */
	BVDC_P_MAKE_ANR( 50,     50,    260,   330,    260,   550,    260,    92,  false),    /* S/N: 26 */
	BVDC_P_MAKE_ANR( 50,     50,    260,   320,    260,   500,    260,    87,  false),    /* S/N: 27 */
	BVDC_P_MAKE_ANR( 50,     50,    260,   310,    260,   450,    260,    82,  false),    /* S/N: 28 */
	BVDC_P_MAKE_ANR( 50,     50,    260,   300,    260,   400,    260,    77,  false),    /* S/N: 29 */
	BVDC_P_MAKE_ANR( 53,     53,    260,   290,    260,   350,    260,    72,  false),    /* S/N: 30 */
	BVDC_P_MAKE_ANR( 56,     56,    260,   280,    260,   300,    260,    62,  false),    /* S/N: 31 */
	BVDC_P_MAKE_ANR( 59,     59,    260,   270,    260,   250,    260,    52,  false),    /* S/N: 32 */
	BVDC_P_MAKE_ANR( 62,     62,    260,   260,    260,   200,    260,    42,  false),    /* S/N: 33 */
	BVDC_P_MAKE_ANR( 65,     65,    260,   250,    260,   150,    260,    32,  false),    /* S/N: 34 */
	BVDC_P_MAKE_ANR( 68,     68,    260,   240,    260,   120,    260,    22,  false),    /* S/N: 35 */
	BVDC_P_MAKE_ANR( 71,     71,    260,   230,    260,   110,    260,    21,  false),    /* S/N: 36 */
	BVDC_P_MAKE_ANR( 74,     74,    260,   220,    260,   102,    260,    21,  false),    /* S/N: 37 */
	BVDC_P_MAKE_ANR( 77,     77,    260,   210,    260,    96,    260,    20,  false),    /* S/N: 38 */
	BVDC_P_MAKE_ANR( 80,     80,    260,   200,    260,    90,    260,    20,  false),    /* S/N: 39 */
	BVDC_P_MAKE_ANR( 83,     83,    260,   190,    260,    84,    260,    19,  false),    /* S/N: 40 */
	BVDC_P_MAKE_ANR( 86,     86,    260,   180,    260,    78,    260,    19,  false),    /* S/N: 41 */
	BVDC_P_MAKE_ANR( 89,     89,    260,   166,    260,    72,    260,    18,  false),    /* S/N: 42 */
	BVDC_P_MAKE_ANR( 92,     92,    260,   153,    260,    66,    260,    18,  false),    /* S/N: 43 */
	BVDC_P_MAKE_ANR(101,    101,    260,   139,    260,    60,    260,    16,  false),    /* S/N: 44 */
	BVDC_P_MAKE_ANR(104,    104,    260,   126,    260,    54,    260,    13,  false),    /* S/N: 45 */
	BVDC_P_MAKE_ANR(107,    107,    260,   112,    260,    48,    260,    11,  false),    /* S/N: 46 */
	BVDC_P_MAKE_ANR(110,    110,    260,   100,    260,    42,    260,     9,  false),    /* S/N: 47 */
	BVDC_P_MAKE_ANR(113,    113,    260,    82,    260,    36,    260,     7,  false),    /* S/N: 48 */
	BVDC_P_MAKE_ANR(116,    116,    260,    70,    260,    30,    260,     7,  false),    /* S/N: 49 */
	BVDC_P_MAKE_ANR(119,    119,    260,    60,    260,    24,    260,     7,  false),    /* S/N: 50 */
	BVDC_P_MAKE_ANR(122,    122,    260,    52,    260,    18,    260,     6,  false),    /* S/N: 51 */
	BVDC_P_MAKE_ANR(125,    125,    260,    42,    260,    12,    260,     5,  false),    /* S/N: 52 */
	BVDC_P_MAKE_ANR(128,    128,    260,    32,    260,     8,    260,     5,  false),    /* S/N: 52 */
	BVDC_P_MAKE_ANR(131,    131,    260,    22,    260,     7,    260,     4,  false),    /* S/N: 54 */
	BVDC_P_MAKE_ANR(134,    134,    260,    16,    260,     6,    260,     4,  false),    /* S/N: 55 */
	BVDC_P_MAKE_ANR(137,    137,    260,    12,    260,     5,    260,     4,  false),    /* S/N: 56 */
	BVDC_P_MAKE_ANR(140,    140,    260,     7,    260,     4,    260,     4,  false),    /* S/N: 57 */
	BVDC_P_MAKE_ANR(143,    143,    260,     4,    260,     3,    260,     3,  false),    /* S/N: 58 */
	BVDC_P_MAKE_ANR(146,    146,    260,     2,    260,     2,    260,     3,  false),    /* S/N: 59 */
	BVDC_P_MAKE_ANR(149,    149,    260,     1,    260,     1,    260,     2,  false),    /* S/N: 60 */
};

#if !B_REFSW_MINIMAL
/***************************************************************************
 * {private}
 *
 * BVDC_P_Anr_GetKValue_isr
 *
 * called by BVDC_P_Anr_BuildRul_StatisRead_isr to look-up K values from
 * SN number (db), at every vsync when RUL is built.
 *
 * This subroutine could be replaced by customer to customize the K values.
 *
 * pFmtInfo represents source video format that might change dynamically
 * and get detected automatically by VDC. It could be NULL if MPEG input
 * source is used. pFmtInfo might be used for customer to fine tune the K
 * values.
 *
 * pvUserInfo is a pointer to a user defined struct. It is passed to VDC as
 * a member of BVDC_Anr_Configure from app with BVDC_Source_ConfigureAnr.
 * It could be used to pass any information that customer want to use to
 * decide the K values.
 */
const BVDC_P_AnrKValue * BVDC_P_Anr_GetKValue_isr(
	uint32_t                       ulSnrDb,
	BVDC_P_CtInput                 eCtInputType,
	const BFMT_VideoInfo          *pFmtInfo,
	void                          *pvUserInfo )
{
	uint32_t  ulTruncSnDb, ulIdx;

	BSTD_UNUSED(pvUserInfo);

	/* xfer SnDb into lookup index and truncate it into valid range */
	ulTruncSnDb = BVDC_P_MAX(BVDC_P_ANR_MIN_SN,
							 BVDC_P_MIN(BVDC_P_ANR_MAX_SN, ulSnrDb));
	ulIdx = ulTruncSnDb - BVDC_P_ANR_MIN_SN;

	if (BVDC_P_CtInput_eIfd == eCtInputType)
	{
		BDBG_MSG(("K-lookup: RF table, snrDb %d, ulIdx %d", ulSnrDb, ulTruncSnDb));
		BDBG_MSG(("ALwThdMc %d, ALwThdNMc %d, McK0 %d, McK1 %d, NonMcK0 %d, NonMcK1 %d, FinalK0 %d, FinalK1 %d",
			s_ulAnrKValueForRf[ulIdx].ulAlphLowThdMC,
			s_ulAnrKValueForRf[ulIdx].ulAlphLowThdNMC,
			s_ulAnrKValueForRf[ulIdx].ulMcK0, s_ulAnrKValueForRf[ulIdx].ulMcK1,
			s_ulAnrKValueForRf[ulIdx].ulNonMcK0, s_ulAnrKValueForRf[ulIdx].ulNonMcK1,
			s_ulAnrKValueForRf[ulIdx].ulFinalK0, s_ulAnrKValueForRf[ulIdx].ulFinalK1));
		return &(s_ulAnrKValueForRf[ulIdx]);
	}
	else if (BFMT_720P_WIDTH <= pFmtInfo->ulWidth)
	{
		BDBG_MSG(("K-lookup: HD table, snrDb %d, ulIdx %d", ulSnrDb, ulTruncSnDb));
		BDBG_MSG(("ALwThdMc %d, ALwThdNMc %d, McK0 %d, McK1 %d, NonMcK0 %d, NonMcK1 %d, FinalK0 %d, FinalK1 %d",
			s_ulAnrKValueForHd[ulIdx].ulAlphLowThdMC,
			s_ulAnrKValueForHd[ulIdx].ulAlphLowThdNMC,
			s_ulAnrKValueForHd[ulIdx].ulMcK0, s_ulAnrKValueForHd[ulIdx].ulMcK1,
			s_ulAnrKValueForHd[ulIdx].ulNonMcK0, s_ulAnrKValueForHd[ulIdx].ulNonMcK1,
			s_ulAnrKValueForHd[ulIdx].ulFinalK0, s_ulAnrKValueForHd[ulIdx].ulFinalK1));
		return &(s_ulAnrKValueForHd[ulIdx]);
	}
	else if (pFmtInfo->bInterlaced)
	{
		BDBG_MSG(("K-lookup: SD table, snrDb %d, ulIdx %d", ulSnrDb, ulTruncSnDb));
		BDBG_MSG(("ALwThdMc %d, ALwThdNMc %d, McK0 %d, McK1 %d, NonMcK0 %d, NonMcK1 %d, FinalK0 %d, FinalK1 %d",
			s_ulAnrKValueForSd[ulIdx].ulAlphLowThdMC,
			s_ulAnrKValueForSd[ulIdx].ulAlphLowThdNMC,
			s_ulAnrKValueForSd[ulIdx].ulMcK0, s_ulAnrKValueForSd[ulIdx].ulMcK1,
			s_ulAnrKValueForSd[ulIdx].ulNonMcK0, s_ulAnrKValueForSd[ulIdx].ulNonMcK1,
			s_ulAnrKValueForSd[ulIdx].ulFinalK0, s_ulAnrKValueForSd[ulIdx].ulFinalK1));
		return &(s_ulAnrKValueForSd[ulIdx]);
	}
	else
	{
		BDBG_MSG(("K-lookup: ED table, snrDb %d, ulIdx %d", ulSnrDb, ulTruncSnDb));
		BDBG_MSG(("ALwThdMc %d, ALwThdNMc %d, McK0 %d, McK1 %d, NonMcK0 %d, NonMcK1 %d, FinalK0 %d, FinalK1 %d",
			s_ulAnrKValueForEd[ulIdx].ulAlphLowThdMC,
			s_ulAnrKValueForEd[ulIdx].ulAlphLowThdNMC,
			s_ulAnrKValueForEd[ulIdx].ulMcK0, s_ulAnrKValueForEd[ulIdx].ulMcK1,
			s_ulAnrKValueForEd[ulIdx].ulNonMcK0, s_ulAnrKValueForEd[ulIdx].ulNonMcK1,
			s_ulAnrKValueForEd[ulIdx].ulFinalK0, s_ulAnrKValueForEd[ulIdx].ulFinalK1));
		return &(s_ulAnrKValueForEd[ulIdx]);
	}
}
#endif

/* End of file. */
