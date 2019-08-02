/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/


#ifndef BDSP_RAAGA_H_
#define BDSP_RAAGA_H_

#include "bchp.h"
#include "bint.h"
#include "bbox.h"
#include "bmma.h"
#include "btmr.h"
#include "bimg.h"
#include "bdsp_task.h"
#include "bdsp.h"
#include "bdsp_common.h"

/*#define RAAGA_UART_ENABLE*/

#define BDSP_RAAGA_ADDRESS_ALIGN_CDB  12 /* CDB is aligned to 2^12 Bytes*/
#define BDSP_RAAGA_ADDRESS_ALIGN_ITB  12 /* ITB is aligned to 2^12 Bytes*/

#define BDSP_RAAGA_ADDRESS_ALIGN      4096
#if 1
#define BDSP_RAAGA_SIZE_ALIGN(x)  	\
	{                               \
		unsigned power = 2;         \
		x = (((x+4095)/4096)*4096);\
		while (x >>= 1) power <<= 1;\
		x=power;                    \
	}
#else
#define BDSP_RAAGA_SIZE_ALIGN(x) \
	{                            \
		unsigned min=0,max=4096; \
		while(x){                \
		if((min<x)&&(x<=max)){   \
			x = max;break;}      \
		min=max; max<<=1;}       \
	}
#endif

#if defined BCHP_RAAGA_DSP_RGR_1_REG_START
#define BDSP_RAAGA_MAX_DSP 2
#else
#define BDSP_RAAGA_MAX_DSP 1
#endif
#define BDSP_RAAGA_MAX_CORE_PER_DSP 2

/***************************************************************************
Summary:
Raaga DSP Settings
***************************************************************************/
typedef struct BDSP_RaagaSettings
{
    bool authenticationEnabled; /* If authentication is enabled, all the firmware execs needs to be downloaded at open time and
                                   the DSP must be booted via a separate call to BDSP_Raaga_Boot() after HSM has been authenticated. */

	bool preloadImages;		   /* If true, all firmware images will be loaded on startup.  Default=false. */

    BDSP_DebugTypeSettings debugSettings[BDSP_DebugType_eLast]; /* Debug information for the different types of debug logs */

    const BIMG_Interface *pImageInterface;      /* Interface to access firmware image. This interface must be
                                                   implemented and the function pointers must be stored here. */

    void **pImageContext;                       /* Context for the Image Interface. This is also provided by
                                                   the implementation of the Image interface */
    unsigned maxAlgorithms[BDSP_AlgorithmType_eMax] ;

    unsigned NumDsp;    /*Number of DSP supported in the System, currently used for estimation of Memory*/

    unsigned numCorePerDsp;  /* Number of cores per DSP supported in the System, Used for estimation of Memory for newer chips */

    struct {
        BSTD_DeviceOffset baseAddress; /* Physical base address accessible heap */
        uint64_t size;
    } heap[BDSP_RAAGA_MAX_NUM_HEAPS]; /* heaps that DSP is expected to access */
    BCHP_MemoryLayout memoryLayout; /* all addressable memory per MEMC */
	bool disableSyncCommmand;
} BDSP_RaagaSettings;

void BDSP_Raaga_GetDefaultSettings(
	BDSP_RaagaSettings *pSettings
);

BERR_Code BDSP_Raaga_Open(
   BDSP_Handle *pDsp,                      /* [out] */
   BCHP_Handle chpHandle,
   BREG_Handle regHandle,
   BMMA_Heap_Handle memHandle,
   BINT_Handle intHandle,
   BTMR_Handle tmrHandle,
   BBOX_Handle boxHandle,
   const BDSP_RaagaSettings *pSettings
);
BERR_Code BDSP_Raaga_GetMemoryEstimate(
    const BDSP_RaagaSettings     *pSettings,
    const BDSP_UsageOptions      *pUsage,
    BBOX_Handle                   boxHandle,
    BDSP_MemoryEstimate          *pEstimate /*[out]*/
);

/***************************************************************************
Summary:
Trigger DBG_service as part of on-chip debug support
***************************************************************************/
BERR_Code BDSP_Raaga_RunDebugService(
    BDSP_Handle hDsp,
    uint32_t dspIndex
);
#endif /* BDSP_RAAGA_H_ */
