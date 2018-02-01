/***************************************************************************
 * Copyright (C) 2006-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 * Perfomance counter module
 *
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bperf_counter.h"
#include "biobits.h"

BDBG_MODULE(bperf_counter);


#if B_PERF_BMIPS3300
/* bcmips3300-arch.pdf pages 63..66 */
#define B_ICACHE_MOD_ID	0x06
#define B_ICACHE_ID		0x00
#define B_ICACHE_HIT	0x06
#define B_ICACHE_MISS	0x05

#define B_DCACHE_MOD_ID	0x04
#define B_DCACHE_ID		0x01
#define B_DCACHE_HIT	0x0A
#define B_DCACHE_MISS	0x09

#define B_RAC_MOD_ID	0x0B
#define B_RAC_ACCESS_ID	0x01
#define B_RAC_ACCESS	0x43
#define B_RAC_PREFETCH_ID 0x00
#define B_RAC_PREFETCH 0x47

#define B_RAC_HIT_ID	0x01
#define B_RAC_HIT		0x40

#define B_CYCLES		0x12
#define B_INSTRUCTIONS	0x11

const  bperf_counter_mode bperf_counter_dcache  = {
	{
		"D-Cache miss",
		"D-Cache hit",
        "Instructions",
		"Cycles"
	},
	{
		/* $25 #6 */
		B_SET_BIT(PCE, 1 /* enable */, 31) |
		B_SET_BIT(magic, 1 /* enable */, 9) |
		B_SET_BIT(PCSD, 0 /* enable */, 8) |
		B_SET_BITS(ModID, B_DCACHE_MOD_ID, 5, 2) |
		B_SET_BITS(SetID, B_DCACHE_ID, 1, 0),

		/* $25 #4 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(EVT, B_DCACHE_MISS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(EVT, B_DCACHE_HIT,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16),

		/* $25 #5 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(EVT, B_INSTRUCTIONS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(EVT, B_CYCLES,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16)
	}
};

const  bperf_counter_mode bperf_counter_instructions = {
	{
        "Instructions",
		"D-Cache miss",
        "Instructions",
		"Cycles"
	},
	{
		/* $25 #6 */
		B_SET_BIT(PCE, 1 /* enable */, 31) |
		B_SET_BIT(magic, 1 /* enable */, 9) |
		B_SET_BIT(PCSD, 0 /* enable */, 8) |
		B_SET_BITS(ModID, B_DCACHE_MOD_ID, 5, 2) |
		B_SET_BITS(SetID, B_DCACHE_ID, 1, 0),

		/* $25 #4 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(EVT, B_INSTRUCTIONS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(EVT, B_DCACHE_MISS,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16),

		/* $25 #5 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(EVT, B_INSTRUCTIONS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(EVT, B_CYCLES,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16)
	}
};

const  bperf_counter_mode bperf_counter_icache  = {
	{
		"I-Cache miss",
		"I-Cache hit",
        "Instructions",
		"Cycles"
	},
	{
		/* $25 #6 */
		B_SET_BIT(PCE, 1 /* enable */, 31) |
		B_SET_BIT(magic, 1 /* enable */, 9) |
		B_SET_BIT(PCSD, 0 /* enable */, 8) |
		B_SET_BITS(ModID, B_ICACHE_MOD_ID, 5, 2) |
		B_SET_BITS(SetID, B_ICACHE_ID, 1, 0),

		/* $25 #4 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(EVT, B_ICACHE_MISS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(EVT, B_ICACHE_HIT,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16),

		/* $25 #5 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(EVT, B_INSTRUCTIONS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(EVT, B_CYCLES,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16)
	}
};

const  bperf_counter_mode bperf_counter_rac_access = {
	{
		"RAC access",
		"RAC hits",
        "Instructions",
		"Cycles"
	},
	{
		/* $25 #6 */
		B_SET_BIT(PCE, 1 /* enable */, 31) |
		B_SET_BIT(magic, 1 /* enable */, 9) |
		B_SET_BIT(PCSD, 0 /* enable */, 8) |
		B_SET_BITS(ModID, B_RAC_MOD_ID, 5, 2) |
		B_SET_BITS(SetID, B_RAC_ACCESS_ID, 1, 0),

		/* $25 #4 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(EVT, B_RAC_ACCESS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(EVT, B_RAC_HIT,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16),

		/* $25 #5 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(EVT, B_INSTRUCTIONS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(EVT, B_CYCLES,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16)
	}
};

const  bperf_counter_mode bperf_counter_rac_prefetch = {
	{
		"RAC prefetch", 
        "Instructions",
        "Instructions",
		"Cycles"
	},
	{
		/* $25 #6 */
		B_SET_BIT(PCE, 1 /* enable */, 31) |
		B_SET_BIT(magic, 1 /* enable */, 9) |
		B_SET_BIT(PCSD, 0 /* enable */, 8) |
		B_SET_BITS(ModID, B_RAC_MOD_ID, 5, 2) |
		B_SET_BITS(SetID, B_RAC_PREFETCH_ID, 1, 0),

		/* $25 #4 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(EVT, B_RAC_PREFETCH,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(EVT, B_INSTRUCTIONS,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16),

		/* $25 #5 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(EVT, B_INSTRUCTIONS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(EVT, B_CYCLES,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16)
	}
};

const  bperf_counter_mode bperf_counter_rac_hit = {
	{
		"RAC hits", 
		"RAC access",
        "Instructions",
		"Cycles"
	},
	{
		/* $25 #6 */
		B_SET_BIT(PCE, 1 /* enable */, 31) |
		B_SET_BIT(magic, 1 /* enable */, 9) |
		B_SET_BIT(PCSD, 0 /* enable */, 8) |
		B_SET_BITS(ModID, B_RAC_MOD_ID, 5, 2) |
		B_SET_BITS(SetID, B_RAC_HIT_ID, 1, 0),

		/* $25 #4 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(EVT, B_RAC_HIT,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(EVT, B_RAC_ACCESS,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16),

		/* $25 #5 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(EVT, B_INSTRUCTIONS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(EVT, B_CYCLES,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16)
	}
};
#elif B_PERF_BMIPS4380
/* bcmips4380-Programmin-v1.7.pdf pages 56..66 */
#define B_ICACHE_MOD_ID	0x06
#define B_ICACHE_ID		0x00
#define B_ICACHE_HIT	0x06
#define B_ICACHE_MISS	0x05

#define B_DCACHE_MOD_ID	0x04
#define B_DCACHE_ID		0x01
#define B_DCACHE_HIT	0x0A
#define B_DCACHE_MISS	0x09

#define B_RAC_MOD_ID	0x0B
#define B_RAC_ACCESS_ID	0x01
#define B_RAC_ACCESS	0x43
#define B_RAC_HIT_ID	0x01
#define B_RAC_HIT		0x40
#define B_DRAC_HIT		0x23

#define B_RAC_PREFETCH_ID 0x00
#define B_RAC_PREFETCH  0x47

#define B_BIU_MOD_ID	0x01

#define B_CYCLES		0x12
#define B_INSTRUCTIONS	0x11

const  bperf_counter_mode bperf_counter_dcache  = {
	{
		"D-Cache miss",
		"D-Cache hit",
        "Instructions",
		"Cycles"
	},
	{
		/* 11F2_0000 */
		B_SET_BIT(PCE, 1 /* enable */, 31) |
		B_SET_BIT(PCSD, 0 /* enable */, 8) |
		B_SET_BITS(ModID, B_DCACHE_MOD_ID, 5, 2) |
		B_SET_BITS(SetID, B_DCACHE_ID, 1, 0),

		/* 11F2_0004 */
		B_SET_BIT(PCE, 1 /* enable */, 15) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14,13) |
		B_SET_BITS(EVT, B_DCACHE_MISS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14+16,13+16) |
		B_SET_BITS(EVT, B_DCACHE_HIT,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16),

		/* 11F2_0008 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14,13) |
		B_SET_BITS(EVT, B_INSTRUCTIONS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14+16,13+16) |
		B_SET_BITS(EVT, B_CYCLES,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16)
	}
};

const  bperf_counter_mode bperf_counter_instructions = {
	{
        "Instructions",
		"D-Cache miss",
        "Instructions",
		"Cycles"
	},
	{
		/* $25 #6 */
		B_SET_BIT(PCE, 1 /* enable */, 31) |
		B_SET_BIT(PCSD, 0 /* enable */, 8) |
		B_SET_BITS(ModID, B_DCACHE_MOD_ID, 5, 2) |
		B_SET_BITS(SetID, B_DCACHE_ID, 1, 0),

		/* $25 #4 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14,13) |
		B_SET_BITS(EVT, B_INSTRUCTIONS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14+16,13+16) |
		B_SET_BITS(EVT, B_DCACHE_MISS,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16),

		/* $25 #5 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14,13) |
		B_SET_BITS(EVT, B_INSTRUCTIONS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14+16,13+16) |
		B_SET_BITS(EVT, B_CYCLES,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16)
	}
};

const  bperf_counter_mode bperf_counter_icache  = {
	{
		"I-Cache miss",
		"I-Cache hit",
        "Instructions",
		"Cycles"
	},
	{
		/* $25 #6 */
		B_SET_BIT(PCE, 1 /* enable */, 31) |
		B_SET_BIT(PCSD, 0 /* enable */, 8) |
		B_SET_BITS(ModID, B_ICACHE_MOD_ID, 5, 2) |
		B_SET_BITS(SetID, B_ICACHE_ID, 1, 0),

		/* $25 #4 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14,13) |
		B_SET_BITS(EVT, B_ICACHE_MISS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14+16,13+16) |
		B_SET_BITS(EVT, B_ICACHE_HIT,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16),

		/* $25 #5 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14,13) |
		B_SET_BITS(EVT, B_INSTRUCTIONS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14+16,13+16) |
		B_SET_BITS(EVT, B_CYCLES,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16)
	}
};

const  bperf_counter_mode bperf_counter_rac_access = {
	{
		"RAC access",
		"RAC hits",
        "Instructions",
		"Cycles"
	},
	{
		/* $25 #6 */
		B_SET_BIT(PCE, 1 /* enable */, 31) |
		B_SET_BIT(PCSD, 0 /* enable */, 8) |
		B_SET_BITS(ModID, B_RAC_MOD_ID, 5, 2) |
		B_SET_BITS(SetID, B_RAC_ACCESS_ID, 1, 0),

		/* $25 #4 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14,13) |
		B_SET_BITS(EVT, B_RAC_ACCESS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14+16,13+16) |
		B_SET_BITS(EVT, B_RAC_HIT,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16),

		/* $25 #5 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14,13) |
		B_SET_BITS(EVT, B_INSTRUCTIONS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14+16,13+16) |
		B_SET_BITS(EVT, B_CYCLES,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16)
	}
};

const  bperf_counter_mode bperf_counter_rac_prefetch = {
	{
		"RAC prefetch", 
        "Instructions",
        "Instructions",
		"Cycles"
	},
	{
		/* $25 #6 */
		B_SET_BIT(PCE, 1 /* enable */, 31) |
		B_SET_BIT(PCSD, 0 /* enable */, 8) |
		B_SET_BITS(ModID, B_RAC_MOD_ID, 5, 2) |
		B_SET_BITS(SetID, B_RAC_PREFETCH_ID, 1, 0),

		/* $25 #4 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14,13) |
		B_SET_BITS(EVT, B_RAC_PREFETCH,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14+16,13+16) |
		B_SET_BITS(EVT, B_INSTRUCTIONS,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16),

		/* $25 #5 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14,13) |
		B_SET_BITS(EVT, B_INSTRUCTIONS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14+16,13+16) |
		B_SET_BITS(EVT, B_CYCLES,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16)
	}
};

const  bperf_counter_mode bperf_counter_rac_hit = {
	{
		"RAC hits", 
		"D-RAC hits",
        "Instructions",
		"Cycles"
	},
	{
		/* $25 #6 */
		B_SET_BIT(PCE, 1 /* enable */, 31) |
		B_SET_BIT(PCSD, 0 /* enable */, 8) |
		B_SET_BITS(ModID, B_RAC_MOD_ID, 5, 2) |
		B_SET_BITS(SetID, B_RAC_HIT_ID, 1, 0),

		/* $25 #4 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14,13) |
		B_SET_BITS(EVT, B_RAC_HIT,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14+16,13+16) |
		B_SET_BITS(EVT, B_DRAC_HIT,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16),

		/* $25 #5 */
		B_SET_BIT(CE, 1 /* enable */, 15) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14,13) |
		B_SET_BITS(EVT, B_INSTRUCTIONS,  8, 2) |
		B_SET_BIT(TIE, 0 /* disable */, 0) |

		B_SET_BIT(CE, 1 /* enable */, 15+16) |
		B_SET_BITS(TPID, 0 /* TP0 */, 14+16,13+16) |
		B_SET_BITS(EVT, B_CYCLES,  8+16, 2+16) |
		B_SET_BIT(TIE, 0 /* disable */, 0+16)
	}
};
#elif B_PERF_BMIPS5000

/* Zephyr Processor Specification, Page 128 */
#define B_0_2_CYCLES        0
#define B_1_3_CYCLES        0

#define B_0_2_INSTRUCTIONS  1
#define B_1_3_INSTRUCTIONS  1

/* No #define B_0_2_FETCH_BRANCH_INST  2 */
#define B_1_3_BRANCH_MISPREDICT  2

#define B_0_2_BHT_REFERENCE 3
#define B_1_3_BHT_MISPREDICT 3

#define B_0_2_BTB_MISPREDICT 4
#define B_1_3_BTB_REFERENCE 4

#define B_0_2_CRS_MISPREDICT 5
#define B_1_3_CRS_REFERENCE 5

#define B_0_2_ITLB_REFERENCE 6
#define B_1_3_ITLB_MISS      6

/* No #define B_0_2_DTLB_REFERENCE 7 */
/* No #define B_1_3_DTLB_MISS 7 */

/* No #define B_0_2_JTLB_I_REFERENCE 8 */
/* No #define B_1_3_JTLB_I_MISS 8 */

/* No #define B_0_2_JTLB_D_REFERENCE 9 */
/* No #define B_1_3_JTLB_D_MISS 9 */

#define B_0_2_I_CACHE_REFERENCE 10
#define B_1_3_I_CACHE_MISS 10

/* ---                      11 */
/* No #define B_1_3_I_CACHE_INVALIDATE 11 */

#define B_0_2_D_CACHE_REFERENCE 12
#define B_1_3_D_CACHE_MISS 12

#define B_0_2_D_CACHE_EVICTION 13
#define B_1_3_D_CACHE_INVALIDATE 13

#define B_0_2_D_CACHE_LOAD_MISS 14
#define B_1_3_D_CACHE_LOAD  14

#define B_0_2_D_CACHE_STORE_MISS 15
#define B_1_3_D_CACHE_STORE 15

#define B_0_2_LOAD_REPLAY 16
#define B_1_3_STORE_REPLAY 16

/* No #define B_0_2_LOAD_REPLAY_RES_UNAVAIL 17 */
/* No #define B_1_3_STORE_REPLAY_RES_UNAVAIL 17 */

/* No #define B_0_2_LOAD_REPLAY_HIT_STORE 18 */
/* No #define B_1_3_STORE_REPLAY_FULL 18 */

/* ---                      19 */
/* ---                      19 */

/* ---                      20 */
/* ---                      20 */

#define B_0_2_RETIRED_LOAD 21
#define B_1_3_RETIRED_ALU 21

#define B_0_2_RETIRED_STORE 22
#define B_1_3_RETIRED_MUL_DIVIDE 23

#define B_0_2_RETIRED_PREFETCH 23
#define B_1_3_RETIRED_FPU 23

#define B_0_2_RETIRED_PREFETCH_NOOP 24
#define B_1_3_RETIRED_DSP 24

#define B_0_2_RETIRED_JPIPE 25
#define B_1_3_RETIRED_KPIPE 25

#define B_0_2_EXCEPTION 26
#define B_1_3_INTERRUPT 26

/* ---                      27 */
/* ---                      27 */

#define B_0_2_L2_CACHE_REFERENCE  28
#define B_1_3_L2_CACHE_MISS 28

#define B_0_2_L2_CACHE_DIRTY_EVICTION 29
#define B_1_3_L2_CACHE_INVALIDATION 29

#define B_0_2_L2_CACHE_HW_PREFETCH 30
#define B_1_3_L2_CACHE_PREFETCH_USED 30

#define B_0_2_L2_CACHE_I_MISS 31
/* No #define B_1_3_L2_CACHE_I_REFERENCE 31 */

#define B_0_2_L2_CACHE_D_MISS 32
#define B_1_3_L2_CACHE_D_REFERENCE 32

/* ---                      33 */
/* ---                      33 */

/* ---                      34 */
/* ---                      34 */

#define B_0_2_BIU_NON_CACHEABLE_LOAD 35
#define B_1_3_BIU_NON_CACHEABLE_STORE 35

/* No #define B_0_2_BIU_CACHEABLE_LOAD 36 */
/* No #define B_1_3_BIU_CACHEABLE_STORE 36 */

/* No #define B_0_2_BIU_PREFETCH  37 */
/* No #define B_1_3_BIU_DOUBLE_PREFETCH  37 */

/* ---                      38 */
#define B_1_3_BIU_CACHEABLE_PARTIAL_STORE 38

/* ---                      39 */
/* ---                      39 */

/* No #define B_0_2_NO_INST_FETCH 40 */
#define B_1_3_STANDBY       40

/* No #define B_0_2_IBUF_BOTH_EMPTY 41 */
/* No #define B_1_3_IBUF_BOTH_EMPTY 41 */

/* No #define B_0_2_IBUF_EMPTY 42 */
/* No #define B_0_2_IBUF_EMPTY 42 */

#define B_0_2_BRANCH_MISPREDICT_ON  43
/* No #define B_1_3_ITLB_MISS_IBUF    43 */

/* ---                      44 */
/* No #define B_1_3_ICACHE_MISS_IBUF  44 */

#define B_0_2_JPIPE_IDLE    45
#define B_1_3_JPIPE_REPLAY  45

#define B_0_2_IQUEUE_EMPTY 46
#define B_1_3_JPIPE_IDLE_OPERAND 46

#define B_0_2_JPIPE_IDLE_LOAD 47
#define B_1_3_JPIPE_IDLE_CONFLICT 47

/* No #define B_0_2_KPIPE_IDLE    48 */
/* No #define B_1_3_KPIPE_REPLAY  48 */

/* #define B_0_2_KPIPE_IDLE_IBUF    49 */
#define B_1_3_KPIPE_IDLE_LOAD   49

/* #define B_0_2_KPIPE_IDLE_LOAD 50 */
#define B_1_3_KPIPE_IDLE_CONFLICT 50

#define B_0_2_FPU_IDLE    51
#define B_1_3_FPU_REPLAY  51

/* No #define B_0_2_ICACHE_MISS_L2 52 */
/* No #define B_1_3_ICACHE_MISS   52 */

/* No #define B_0_2_DCACHE_MISS_L2 53 */
/* No #define B_1_3_DCACHE_MISS   53 */

#define B_0_2_L2_BLOCKED 54
/* No #define B_1_3_L2_MISS    54 */

/* No #define B_0_2_L2_BLOCKED_EVICTION 55 */
/* No #define B_1_3_L2_BLOCKED_REFILL   55 */

/* No #define B_0_2_L2_BLOCKED_CREDIT 56 */
/* No #define B_1_3_L2_BLOCKED_BANK   56 */

/* No #define B_0_2_LOAD_L2_BLOCKED_CREDIT 57 */
/* No #define B_1_3_STORE_L2_BLOCKED_CREDIT 57 */

#define B_0_2_CYCLES_BIU_CACHEABLE_LOAD 58
#define B_1_3_CYCLES_BIU_CACHEABLE_STORE 58

#define B_0_2_BIU_UNCACHEABLE_LOAD 59
#define B_1_3_BIU_UNCACHEABLE_STORE 59

#define B_0_2_BIU_CACHEABLE_LOAD_SENT 60
#define B_1_3_BIU_CACHEABLE_STORE_SENT 60

#define B_0_2_BIU_UNCACHEABLE_LOAD_SENT 61
#define B_0_2_BIU_UNCACHEABLE_STORE_SENT 61

#define B_0_2_ALU_SKEWED_1_CYCLE    62
#define B_1_3_ALU_SKEWED_2_CYCLE    62

#define B_0_2_ALU_SKEWED_3_CYCLE    63

#define B_ONE_COUNTER(event, more) \
		B_SET_BIT(M, more , 31) | \
		B_SET_BIT(TC, 1 /* both threads */, 30) | \
		B_SET_BITS(IdxEn, 0 /* The cache index field is ignored */, 29, 28) | \
		B_SET_BITS(ASID, 3 /* Both threads */, 19, 12) | \
		B_SET_BITS(EVENT, event, 10, 5) | \
		B_SET_BIT(IE, 0 /* interrupt disable */, 30) | \
		B_SET_BIT(U, 1 /* enable user mode */, 3) | \
		B_SET_BIT(S, 1 /* enable supervisor mode */, 2) | \
		B_SET_BIT(K, 1 /* enable kernel mode */, 1) | \
		B_SET_BIT(EXL, 1 /* enable exception mode */, 0) 


#define B_COUNTERS(c0,c1,c2,c3) B_ONE_COUNTER(B_0_2_##c0, 1), B_ONE_COUNTER(B_1_3_##c1, 1) , B_ONE_COUNTER(B_0_2_##c2, 1), B_ONE_COUNTER(B_1_3_##c3, 0)

const  bperf_counter_mode bperf_counter_dcache  = {
	{
		"D-Cache reference",
		"D-Cache miss",
        "L2 reference",
		"L2 miss",
        "Cycles/8"
	},
	{
        B_COUNTERS(D_CACHE_REFERENCE, D_CACHE_MISS, L2_CACHE_REFERENCE, L2_CACHE_MISS)
	}
};

const  bperf_counter_mode bperf_counter_instructions = {
	{
		"J-Pipe",
		"K-Pipe",
        "Branches",
		"Instructions",
        "Cycles/8"
	},
	{
        B_COUNTERS(RETIRED_JPIPE, RETIRED_KPIPE, BHT_REFERENCE, INSTRUCTIONS)
	}
};

const  bperf_counter_mode bperf_counter_icache = {
	{
		"I-Cache reference",
		"I-Cache miss",
        "L2/I reference",
		"L2 miss",
        "Cycles/8"
	},
	{
        B_COUNTERS(I_CACHE_REFERENCE, I_CACHE_MISS, L2_CACHE_I_MISS, L2_CACHE_MISS)
	}
};

const  bperf_counter_mode bperf_counter_idle = {
	{
		"I-Queue empty",
		"K-Retired",
   		"J-Idle",
		"Standby",
        "Cycles/8"
	},
	{
        B_COUNTERS(IQUEUE_EMPTY, RETIRED_KPIPE, JPIPE_IDLE, STANDBY)
	}
};

const  bperf_counter_mode bperf_counter_memory = {
	{
		"uncached Load",
		"uncached Store",
   		"Load Instructions",
		"ALU Instructions",
        "Cycles/8"
	},
	{
        B_COUNTERS(BIU_NON_CACHEABLE_LOAD, BIU_NON_CACHEABLE_STORE, RETIRED_LOAD, RETIRED_ALU)
	}
};

const  bperf_counter_mode bperf_counter_memory_cycles = {
	{
		"uncached Load",
		"uncached Store",
   		"cached Load",
		"cached Store",
        "Cycles/8"
	},
	{
        B_COUNTERS(BIU_UNCACHEABLE_LOAD, BIU_UNCACHEABLE_STORE, CYCLES_BIU_CACHEABLE_LOAD, CYCLES_BIU_CACHEABLE_STORE)
	}
};


#elif defined(B_PERF_MIPSR5K) || defined(B_PERF_MIPS34K)
/* MD00012-2B-5K-SUM-02.08.pdf pages 146..150 */
#define B_EVENT_CYCLES	0
#define B_EVENT_0_INST_FETCHED 1
#define B_EVENT_1_INST_EXECUTED 1
#define B_EVENT_CACHE_OPS	2
#define B_EVENT_STORES	3
#define B_EVENT_COND_STORES	4
#define B_EVENT_0_FAIL_STORES	5
#define B_EVENT_1_FPU_OPS	5
#define B_EVENT_0_BRANCHES	6
#define B_EVENT_1_DCACHE_EVICTED 6
#define B_EVENT_0_ITLB_MISS	7
#define B_EVENT_1_TLB_MISS_EXC	7
#define B_EVENT_0_DTLB_MISS	8
#define B_EVENT_1_BRANCH_MISS 8
#define B_EVENT_0_ICACHE_MISS	9
#define B_EVENT_1_DCACHE_MISS	9
#define B_EVENT_0_INST_SCHED 10
#define B_EVENT_1_INST_STALL 10
#define B_EVENT_0_DUAL_ISSUE	14
#define B_EVENT_0_INST_EXECUTED 15
#define B_EVENT_1_INST_COP2 15

const  bperf_counter_mode bperf_counter_dcache  = {
	{
		"I-Cache miss",
		"D-Cache miss"
	}, {
		/* $25 0 */
		B_SET_BITS(Event, B_EVENT_0_ICACHE_MISS, 8, 5) |
		B_SET_BIT(IE, 1, 4) |
		B_SET_BIT(U, 1, 3) |
		B_SET_BIT(S, 1, 2) |
		B_SET_BIT(K, 1, 1) |
		B_SET_BIT(EXL, 1, 0),

		/* $25 2 */
		B_SET_BITS(Event, B_EVENT_1_DCACHE_MISS, 8, 5) |
		B_SET_BIT(IE, 1, 4) |
		B_SET_BIT(U, 1, 3) |
		B_SET_BIT(S, 1, 2) |
		B_SET_BIT(K, 1, 1) |
		B_SET_BIT(EXL, 1, 0),

		/* unused */
		0
	}
};

const  bperf_counter_mode bperf_counter_icache  = {
	{
		"I-Cache miss",
		"D-Cache miss"
	}, {
		/* $25 0 */
		B_SET_BITS(Event, B_EVENT_0_ICACHE_MISS, 8, 5) |
		B_SET_BIT(IE, 1, 4) |
		B_SET_BIT(U, 1, 3) |
		B_SET_BIT(S, 1, 2) |
		B_SET_BIT(K, 1, 1) |
		B_SET_BIT(EXL, 1, 0),

		/* $25 2 */
		B_SET_BITS(Event, B_EVENT_1_DCACHE_MISS, 8, 5) |
		B_SET_BIT(IE, 1, 4) |
		B_SET_BIT(U, 1, 3) |
		B_SET_BIT(S, 1, 2) |
		B_SET_BIT(K, 1, 1) |
		B_SET_BIT(EXL, 1, 0),

		/* unused */
		0
	}
};

const  bperf_counter_mode bperf_counter_instructions = {
	{
        "Instructions",
		"Cycles"
	},
	{
		/* $25 0 */
		B_SET_BITS(Event, B_EVENT_0_INST_EXECUTED, 8, 5) |
		B_SET_BIT(IE, 1, 4) |
		B_SET_BIT(U, 1, 3) |
		B_SET_BIT(S, 1, 2) |
		B_SET_BIT(K, 1, 1) |
		B_SET_BIT(EXL, 1, 0),

		/* $25 2 */
		B_SET_BITS(Event, B_EVENT_CYCLES, 8, 5) |
		B_SET_BIT(IE, 1, 4) |
		B_SET_BIT(U, 1, 3) |
		B_SET_BIT(S, 1, 2) |
		B_SET_BIT(K, 1, 1) |
		B_SET_BIT(EXL, 1, 0),

		/* unused */
		0
	}
};
#elif B_PERF_LINUX
#if !(defined(B_REFSW_OS_BKERNEL) || defined(__KERNEL__))
#include <sys/time.h>
#elif defined(__KERNEL__)
#include <linux/timer.h>
#endif
#define DONT_PROFILE __attribute__((no_instrument_function))
unsigned  DONT_PROFILE
b_perf_read_time(void)
{
    unsigned time;
    struct timeval tv;
#if defined(__KERNEL__)
	do_gettimeofday(&tv);
#else
	gettimeofday(&tv, NULL);
#endif

	time = tv.tv_usec+((unsigned)tv.tv_sec)*1000000;
    return time;
}

const  bperf_counter_mode bperf_counter_dcache  = {
	{ "Time, us" },
	{ 0,0,0}
};

const  bperf_counter_mode bperf_counter_icache  = {
	{ "Time, us" },
	{ 0,0,0}
};

const  bperf_counter_mode bperf_counter_instructions = {
	{ "Time, us" },
	{ 0,0,0}
};

#endif

static const bperf_counter_mode *b_perf_mode = NULL;

#if defined(b_perf_write_cfg)
#define bcm_write_perf(sel, value) b_perf_write_cfg(sel, value)
#else
#define bcm_write_perf(sel, value)					\
{		__asm__ __volatile__(".set\tpush\n\t"			\
			".set\tmips32\n\t"							\
			"mtc0\t%0, $25, " #sel "\n\t"	\
			".set\tpop\n\t"							\
			: /* none */								\
			: "r" ((unsigned int)value));				\
}
#endif

int 
b_perf_init(const bperf_counter_mode *mode)
{
	BDBG_ASSERT(mode);
#if  defined(B_PERF_BMIPS3300) || defined(B_PERF_BMIPS4380)
	BDBG_LOG(("perf_init %#x %#x %#x",mode->config[0],mode->config[1],mode->config[2]));
#if B_PERF_BMIPS4380
	b_perf_4380_base[0] = mode->config[0];
	b_perf_4380_base[1] = mode->config[1];
	b_perf_4380_base[2] = mode->config[2];
	b_perf_read_one(0)	= 0;
	b_perf_read_one(1)	= 0;
	b_perf_read_one(2)	= 0;
	b_perf_read_one(3)	= 0;
#else
	bcm_write_perf(6, mode->config[0]);
	bcm_write_perf(4, mode->config[1]);
	bcm_write_perf(5, mode->config[2]);

	bcm_write_perf(0, 0);  /* PerfCount0 reset to 0 (it is a decrementing count) */
	bcm_write_perf(1, 0);  /* PerfCount1 reset to 0 (it is a decrementing count) */
	bcm_write_perf(2, 0);  /* PerfCount2 reset to 0 (it is a decrementing count) */
	bcm_write_perf(3, 0);  /* PerfCount3 reset to 0 (it is a decrementing count) */
#endif
#elif defined(B_PERF_BMIPS5000)
	BDBG_LOG(("perf_init %#x %#x %#x %#x",mode->config[0],mode->config[1],mode->config[2],mode->config[3]));
	bcm_write_perf(0, mode->config[0]);
	bcm_write_perf(2, mode->config[1]);
	bcm_write_perf(4, mode->config[2]);
	bcm_write_perf(6, mode->config[3]);

	bcm_write_perf(1, 0);  /* PerfCount0 reset to 0 */
	bcm_write_perf(3, 0);  /* PerfCount1 reset to 0 */
	bcm_write_perf(5, 0);  /* PerfCount2 reset to 0 */
	bcm_write_perf(7, 0);  /* PerfCount3 reset to 0 */

#elif defined(B_PERF_MIPSR5K) || defined(B_PERF_MIPS34K)
	BDBG_MSG(("perf_init %#x %#x",mode->config[0],mode->config[1]));
	bcm_write_perf(0, mode->config[0]);
	bcm_write_perf(2, mode->config[1]);

	bcm_write_perf(1, 0)  /* PerfCount0 reset to 0 */
	bcm_write_perf(3, 0)  /* PerfCount1 reset to 0 */
#endif

	b_perf_mode = mode;

	return 0;
}


void 
bperf_print(const bperf_counter_mode *mode, const bperf_sample *stop, const bperf_sample *start)
{
	unsigned i;
    char str[64];
    unsigned pos=0;
	if (mode==NULL) {
		mode = b_perf_mode;
	}
	BDBG_ASSERT(mode);
    str[0] = 0;
	for(i=0;i<BPERF_N_COUNTERS;i++) {
		unsigned diff = bperf_sample_diff(stop->data[i], start->data[i]);
        int rc = BKNI_Snprintf(str+pos, sizeof(str)-pos, " %s(%u) ", mode->counter_names[i], diff);
        if(rc>0) {
            if((unsigned)rc<sizeof(str)-pos) {
                pos -= rc;
                BDBG_ASSERT(pos<=sizeof(str));
            } else {
                pos = sizeof(str);
            }
        }
	}
	BDBG_LOG(("%s", str));
	return;
}

const bperf_counter_mode *
bperf_get_mode(void)
{
	return b_perf_mode;
}

