/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/

/* Used for the interrupt decoupling tasks */
#include <msgQLib.h>
#include <taskLib.h>

#include "simpleinit.h"

#include "bchp_hif_cpu_intr1.h" /* Interrupt IDs */

#include "int1_api.h"
#include "boardmap.h"

#include "bchp_vcxo_0_rm.h"
#include "bchp_clk.h"

#include "bkni_multi.h"
#include "bkni_event_group.h"
#include "berr.h"
#include "bdbg.h"

#include "bxvd_priv.h"
#include "bpcrlib_xvd.h"
#include "bvdc.h"
#include "bcmtm.h"
#if BCHP_CHIP==7401
#include "bhsm_misc.h"
#endif
#if BCHP_CHIP==7400
#include "bchp_sun_gisb_arb.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_clk.h"
#include "bchp_vcxo_0_rm.h"
#include "bchp_vcxo_1_rm.h"
#include "bchp_vcxo_ctl_misc.h"
#endif

BDBG_MODULE("simpleinit");

/*---------------------------------------------------
	External functions
  ---------------------------------------------------*/

/* This starts a task, which then invokes BINT_Isr (since there are
 * restrictions on what VxWorks can do in interrupt context, and we need to do
 * more than it will allow).
 */
extern int b_intmgr_vxInit( void );
extern int CPUINT1_ConnectIsr_vxWorks(unsigned long intId, void *pfunc, void *param1, int param2);

/* This function signals the interrupt task (via a message queue at the
 * moment). It's used instead of BINT_Isr when connecting interrupts up.
 */
extern void b_vxworks_Isr( BINT_Handle intHandle, int intId );

static void PictureReady_isr(void *pvSourceHandle, int iParm2, void *pvFieldData);


/*---------------------------------------------------
	External variables
  ---------------------------------------------------*/

/*---------------------------------------------------
	Constants
  ---------------------------------------------------*/

#define MAX_TUNERS 2

#ifndef MAX_INT_MSG
#define MAX_INT_MSG			32
#endif
#ifndef INT_MSG_START_ID
#define INT_MSG_START_ID	1234
#endif
#ifndef INT_MSG_END_ID
#define INT_MSG_END_ID		6789
#endif

#if BCHP_CHIP==7401
#define B_DAC_Pr_0 BVDC_Dac_5
#define B_DAC_Y_0  BVDC_Dac_4
#define B_DAC_Pb_0 BVDC_Dac_3

#define B_DAC_C_0  	BVDC_Dac_2
#define B_DAC_YC_C_0 BVDC_Dac_1
#define B_DAC_YC_Y_0 BVDC_Dac_0
#endif

#if BCHP_CHIP==7400
#define B_DAC_Pr_0 BVDC_Dac_2
#define B_DAC_Y_0  BVDC_Dac_0
#define B_DAC_Pb_0 BVDC_Dac_1

#define B_DAC_C_0  	BVDC_Dac_4
#define B_DAC_YC_C_0 BVDC_Dac_5
#define B_DAC_YC_Y_0 BVDC_Dac_6

#define B_DAC_C_1  	BVDC_Dac_3
#endif

#if BCHP_CHIP == 7401
#define HEAP_OFFSET				(48 * 1024 * 1024)
#define HEAP_XVD_SIZE           (1 * 1024 * 1024)
#define HEAP_SIZE               (0x08000000 - HEAP_OFFSET)
#endif

#if BCHP_CHIP == 7400
#define HEAP_OFFSET				(48 * 1024 * 1024)
#define HEAP_XVD_SIZE           (0)
#define HEAP_SIZE               (0x08000000 - HEAP_OFFSET)
#endif

#define HEAP_START				(0xa0000000 | HEAP_OFFSET)
#if BCHP_CHIP == 7401
#define HEAP_ALIGNMENT			4
#endif
#if BCHP_CHIP == 7400
#define HEAP_ALIGNMENT			6
#endif


#define MAX_APP_SRCWND 6
#define MAX_MI 2


/*---------------------------------------------------
	Local types
  ---------------------------------------------------*/

/*---------------------------------------------------
	Local Variables
  ---------------------------------------------------*/


/*---------------------------------------------------
	Local Routines
  ---------------------------------------------------*/
static BERR_Code InitSystem(void);
static BERR_Code InitUPGModules(void);
static BERR_Code OpenI2cDevice(void);
static BERR_Code OpenSpiDevice(void);
static BERR_Code OpenIcpDevice(void);
static BERR_Code OpenAudDevice(void);
static BERR_Code OpenRapDevice(void);
void InitVideoData(void);
void GetHeapConfig(uint32_t* heapStart, uint32_t* heapOffset, uint32_t* heapSize, uint32_t* heapXvdSize, uint32_t* heapAlignment);
BERR_Code ConfigurePinMux(void);
void InitTransport(void);
BERR_Code SetDisplayFormat(BVDC_CompositorId theCompositor, BFMT_VideoFmt theFormat, bool applyNow);



static void bcmMapL1Ints(void);
static void bcmEnableL1Ints(void);

BCHP_Handle GetBCHP(void);
BREG_Handle GetBREG(void);
BMEM_Handle GetBMEM(void);
BINT_Handle GetBINT(void);
BTMR_Handle GetBTMR(void);

typedef struct
{
	uint32_t	clipLeft;
	uint32_t	clipRight;
	uint32_t	clipTop;
	uint32_t	clipBottom;

	uint32_t	scalerLeft;
	uint32_t	scalerTop;
	uint32_t	scalerWidth;
	uint32_t	scalerHeight;

	uint32_t	destLeft;
	uint32_t	destTop;
	uint32_t	destWidth;
	uint32_t	destHeight;

} WndPlacement_t;

typedef struct MpegIntercept_FrameBuffer_t
{
	uint32_t           uiRequestedWidth;    /* The size requested */
	uint32_t           uiRequestedHeight;
	uint32_t           uiAllocWidth;        /* The size allocated */
	uint32_t           uiAllocHeight;
	uint8_t            *pucLumaAddr;		/* Luma buffer data */
	uint8_t            *pucChromaAddr;		/* Chroma buffer data */
	uint32_t           uiLumaRowStride;		/* Stride for luma buffer */
	uint32_t           uiChromaRowStride;	/* Stride for chroma data */
	uint32_t           uiFrames;			/* Number of frames stored in the buffers */
	uint32_t           uiFrameOffset;		/* Bytes per row */
	uint32_t           uiLumaBufSize;
	uint32_t           uiChromaBufSize;
} MpegIntercept_FrameBuffer_t;



typedef struct MpegIntercept_Field_t
{
	BMEM_Handle                  hMem;
	BINT_Handle                  hInt;
	BAVC_MVD_Field               stAVC_MVD_Field;
	MpegIntercept_FrameBuffer_t  stFrameBuffer;
	BVDC_Source_Handle           hSource;
	bool                         bCapturePending;
	BINT_CallbackHandle          hCallback[3];
	BERR_Code                    captureErr;
} MpegIntercept_Field_t;

typedef struct MpegIntercept_Field_t *MpegIntercept_Handle;


/*---------------------------------------------------
	Porting Interface Handles
  ---------------------------------------------------*/

struct
{
	BVDC_Compositor_Handle	hCompositor[2];
	BVDC_Display_Handle		hDisplay[2];
	/* Graphics */
	BVDC_Source_Handle		hGfxSrc;
	BVDC_Window_Handle		hGfxWnd;
	/* Mpeg */
	BVDC_Source_Handle		hMpegSrcList[MAX_APP_SRCWND];
	BVDC_Window_Handle		hMpegWndList[MAX_APP_SRCWND];
    BPCRlib_Xvd_Decoder     asDec[2];

	/* Mpeg Intercept */
	MpegIntercept_Handle	hMI[MAX_MI];
} sVideoData;

struct
{
	/* Base Modules */
	BCHP_Handle			hBCHP;
	BREG_Handle			hBREG;
	BINT_Handle			hBINT;
	BMEM_Handle			hBMEM;
	BMEM_ModuleHandle	hBMEM_ModuleHandle;
    BTMR_Handle         hBTMR;
#if BCHP_CHIP == 7401
    BMEM_Handle         hBMEM_XVD;
#endif
#if BCHP_CHIP == 7400
    BMEM_Handle         hBMEM_XVD[2];
#endif

	/* UPG Modules */
	BI2C_Handle			hBI2C;
	BI2C_ChannelHandle	hBI2C_Chan[BI2C_MAX_I2C_CHANNELS];
	BREG_I2C_Handle		hBI2C_Reg[BI2C_MAX_I2C_CHANNELS];

	BSPI_Handle			hBSPI;
	BSPI_ChannelHandle	hBSPI_Chan[MAX_SPI_CHANNELS];
	BREG_SPI_Handle		hBSPI_Reg[MAX_SPI_CHANNELS];

	BICP_Handle			hBICP;
	BICP_ChannelHandle	hBICP_Chan[MAX_ICP_CHANNELS];

	BIRB_Handle			hBIRB;

	BKIR_Handle			hBKIR;
	BKIR_ChannelHandle	hBKIR_Chan;

	BKPD_Handle			hBKPD;

	BLED_Handle			hBLED;

	/* Porting Interface Modules */
	BVDC_Handle			hBVDC;
	BRDC_Handle			hBRDC;
	BGRC_Handle			hBGRC;
	BGIO_Handle			hBGIO;
//	BURT_Handle			hBURT;
	BXPT_Handle			hBXPT;
	BHDM_Handle			hBHDM;
#if BCHP_CHIP == 7401
	BXVD_Handle			hBXVD;
#endif
#if BCHP_CHIP == 7400
	BXVD_Handle			hBXVD[2];
#endif
	BRAP_Handle			hBRAP;

    BXPT_PCR_Handle     ahBXPT_PCR[BXPT_P_MAX_PCRS];
    BPCRlib_Handle      hBPCRlib;
    BPCRlib_Channel     ahBPCRlib_Channel[BXPT_P_MAX_PCRS];
    BXPT_Playback_Handle ahBXPT_Playback[BXPT_P_MAX_PLAYBACKS];
    BXPT_PcrOffset_Handle ahBXPT_PcrOffset[BXPT_P_MAX_PCR_OFFSET_CHANNELS];
    BXPT_Rave_Handle    hBXPT_Rave;
#if BCHP_CHIP == 7400
    BMEM_Handle	        ahBMEM_XVD_CHPmem[2];
    BMEM_Handle         hBMEM_XVD_FWmem;
    BMEM_Handle         ahBMEM_XVD[2];
#endif
#if BCHP_CHIP==7401
    BHSM_Handle         hBHSM;
#endif
} sAppData;

void SetUnscaledPlacement(WndPlacement_t* placement, uint32_t width, uint32_t height);
BVDC_Display_Handle GetDisplay(BVDC_CompositorId theCompositor);
BERR_Code MakeMpegSrc(BVDC_CompositorId theCompositor, uint32_t id, uint32_t mvdChan, BVDC_Source_Handle* phSrc);
BERR_Code OpenVideoChan(uint32_t channelNum, BVDC_Source_Handle src, bool bInstallPTSCallbacks);
BERR_Code MakeMpegWnd(BVDC_CompositorId theCompositor, uint32_t id, BVDC_Source_Handle src, BVDC_Window_Handle *phWnd);
BERR_Code PlaceWindow(BVDC_Window_Handle hWnd, WndPlacement_t* placement, bool applyNow);
BERR_Code OpenXvChan(uint32_t channelNum, BVDC_Source_Handle src, BXVD_ChannelHandle* phChannel);
BVDC_Compositor_Handle GetCompositor(BVDC_CompositorId theCompositor);
bool MpegIntercept_CapturePending_isr(MpegIntercept_Handle	hMI);
void MpegIntercept_CopyFieldData_isr(MpegIntercept_Handle	hMI, BAVC_MVD_Field*         pFieldData	);



/*---------------------------------------------------
	Entry Point: mainApp
  ---------------------------------------------------*/

BERR_Code InitApp(void)
{
	BERR_Code	status = BERR_SUCCESS;

	memset(&sAppData, 0, sizeof(sAppData));

	if (status == BERR_SUCCESS)
		status = InitSystem();

	if (status == BERR_SUCCESS)
	{
		InitVideoData();
	}

	BERR_TRACE(status);
	return status;
}

/*---------------------------------------------------
	InitSystem
  ---------------------------------------------------*/
BERR_Code InitSystem(void)
{
	BERR_Code				status = BERR_SUCCESS;
	const BINT_Settings 	*pINTSettings;
	BRDC_Settings		 RDCSettings;
	BVDC_Settings			VDCSettings;
	BIRB_Settings			IRBSettings;
	BHDM_Settings			HDMSettings;
	BMEM_Settings			defMemSettings;
	BMEM_Heap_Settings		stHeapSettings;

	uint32_t				heapStart;
	uint32_t				heapOffset;
	uint32_t				heapSize;
	uint32_t				heapAlignment;
    uint32_t                heapXvdSize;
#if BCHP_CHIP == 7400
	BXVD_Settings            cfg;
#endif /* BCHP_CHIP*/


    int ii;

	/*	Initialize KernelInterface */
	if (status == BERR_SUCCESS)
		status = BKNI_Init();

    /*	Initialize DebugInterface */
	if (status == BERR_SUCCESS)
		status = BDBG_Init();

	if (status == BERR_SUCCESS)
		status = BDBG_SetLevel(BDBG_eWrn);

    if (b_intmgr_vxInit())
    {
        BDBG_ERR(("Error initialising VxWorks interrupt task"));
        status = BERR_UNKNOWN;
    }

	/*	Initialize RegisterInterface(s) */
	if (status == BERR_SUCCESS)
		BREG_Open(&sAppData.hBREG, (void *)(BCM_PHYS_TO_K1(BCHP_PHYSICAL_OFFSET)), (size_t)(0x0fffffff), NULL );

	/* CHP */
	if (status == BERR_SUCCESS)
#if (BCHP_CHIP == 7401)
        status = BCHP_Open7401(&sAppData.hBCHP, sAppData.hBREG);

    /*	Initialize InterruptInterface(s) */
    if (status == BERR_SUCCESS)
    {
        pINTSettings = BINT_7401_GetSettings();

#endif

#if (BCHP_CHIP == 7400)
		status = BCHP_Open7400(&sAppData.hBCHP, sAppData.hBREG);

        /*	Initialize InterruptInterface(s) */
        if (status == BERR_SUCCESS)
        {
            pINTSettings = BINT_7400_GetSettings();

#endif
        status = BINT_Open(&sAppData.hBINT, sAppData.hBREG, pINTSettings, NULL);
	}

	/*	Initialize MemoryManager(s) */
	GetHeapConfig(&heapStart, &heapOffset, &heapSize, &heapXvdSize, &heapAlignment);

    BMEM_GetDefaultSettings(&defMemSettings);
    if (status == BERR_SUCCESS) {
        status = BMEM_Open(&sAppData.hBMEM_ModuleHandle, &defMemSettings);
    }

#if (BCHP_CHIP == 7401)

    sAppData.hBMEM_XVD = NULL;

    if (status == BERR_SUCCESS) {
        status = BMEM_Heap_GetDefaultSettings(&stHeapSettings);
    }
	if (status == BERR_SUCCESS)
        stHeapSettings.uiAlignment = heapAlignment;
        stHeapSettings.eBookKeeping = BMEM_BOOKKEEPING_LOCAL;

		status = BMEM_Heap_Create(
            sAppData.hBMEM_ModuleHandle,
            (void*)heapStart,
            heapOffset,
            heapSize,
            &stHeapSettings,
            &sAppData.hBMEM
            );
#endif

#if (BCHP_CHIP == 7400)
if (status == BERR_SUCCESS) {
#if 0
            status = BMEM_Heap_GetDefaultSettings(&stHeapSettings);

            stHeapSettings.uiAlignment = 4;


            status = BMEM_Heap_Create(
                sAppData.hBMEM_ModuleHandle,
                (void*)heapStart,
                heapOffset,
                0x03E00000 - heapOffset,
                &stHeapSettings,
                &sAppData.hBMEM_XVD_FWmem
                );

            status = BMEM_Heap_GetDefaultSettings(&stHeapSettings);

            stHeapSettings.uiAlignment = 0;
            stHeapSettings.eBookKeeping = BMEM_BOOKKEEPING_SYSTEM;

            status = BMEM_Heap_Create(
                sAppData.hBMEM_ModuleHandle,
                (void*)0xA3E00000,
                0x03E00000,
                0x00100000,
                &stHeapSettings,
                &sAppData.hBMEM_XVD
                );
#endif
            sAppData.ahBMEM_XVD[0] = sAppData.ahBMEM_XVD[1] = 0;
            heapStart += heapXvdSize;
            heapOffset += heapXvdSize;
            heapSize -= heapXvdSize;

            BXVD_GetDefaultSettings(&cfg);

            stHeapSettings.uiAlignment = 8;
            stHeapSettings.pCachedAddress = NULL;
            stHeapSettings.flush = NULL;
            stHeapSettings.flush_isr = NULL;
            stHeapSettings.eBookKeeping = BMEM_BookKeeping_eSystem;	/* These can not be accessed via the CPU on the 7400 */
            stHeapSettings.eSafetyConfig = BMEM_SafetyConfig_eFastest;	/* These can not be accessed via the CPU on the 7400 */


            /* Memory heap for 16-bit DDR_1: 1-on-1 mapping, 256-byte aligned */
            status = BMEM_Heap_Create(sAppData.hBMEM_ModuleHandle, (void *)0x60000000, 0x60000000, (64*1024*1024),
                                      &stHeapSettings, &sAppData.ahBMEM_XVD_CHPmem[0]);


            /* Memory heap for 16-bit DDR_2: 1-on-1 mapping, 256-byte aligned */
            status = BMEM_Heap_Create(sAppData.hBMEM_ModuleHandle, (void *)0x70000000, 0x70000000, (64*1024*1024),
                                      &stHeapSettings, &sAppData.ahBMEM_XVD_CHPmem[1]);

}
        if (status == BERR_SUCCESS) {
            status = BMEM_Heap_GetDefaultSettings(&stHeapSettings);
        }
        if (status == BERR_SUCCESS)
        {
            stHeapSettings.uiAlignment = heapAlignment;
            stHeapSettings.eBookKeeping = BMEM_BOOKKEEPING_LOCAL;
            status = BMEM_Heap_Create(
                sAppData.hBMEM_ModuleHandle,
                (void*)heapStart,
                heapOffset,
                heapSize,
                &stHeapSettings,
                &sAppData.hBMEM
                );
        }
#endif

    if (status == BERR_SUCCESS)
    {
        BTMR_DefaultSettings settings;

        status = BTMR_GetDefaultSettings(&settings);

        if (status == BERR_SUCCESS)
            status = BTMR_Open(&sAppData.hBTMR, sAppData.hBCHP, sAppData.hBREG, sAppData.hBINT, &settings);
    }

	/* Configure PIN muxing */
	if (status == BERR_SUCCESS)
		status = ConfigurePinMux();

	/*	Initialize UPG modules */
	if (status == BERR_SUCCESS)
		status = InitUPGModules();

	/*	Initialize other PortingInterface modules */
	/*	RDC */
	BRDC_GetDefaultSettings(sAppData.hBCHP, &RDCSettings);
	if (status == BERR_SUCCESS)
		status = BRDC_Open(&sAppData.hBRDC, sAppData.hBCHP, sAppData.hBREG, sAppData.hBMEM, &RDCSettings);

	/*	GRC */
	if (status == BERR_SUCCESS)
		status = BGRC_Open(&sAppData.hBGRC, sAppData.hBCHP, sAppData.hBREG, sAppData.hBMEM, sAppData.hBINT, NULL);

	/*	VDC */
	if (status == BERR_SUCCESS)
		BVDC_GetDefaultSettings(&VDCSettings);

#if (BCHP_CHIP == 7400)
    VDCSettings.bVecSwap = false;
#endif

	if (status == BERR_SUCCESS)
	{
		status = BVDC_Open(&sAppData.hBVDC, sAppData.hBCHP, sAppData.hBREG, sAppData.hBMEM, sAppData.hBINT, sAppData.hBRDC, sAppData.hBTMR, &VDCSettings);
	}

	/* GIO */
	if (status == BERR_SUCCESS)
		status = BGIO_Open(&sAppData.hBGIO, sAppData.hBCHP, sAppData.hBREG);
#if 0
	/* URT */
	if (status == BERR_SUCCESS)
		status = BURT_Open(&sAppData.hBURT, sAppData.hBCHP, sAppData.hBREG, sAppData.hBINT, NULL);
#endif
	/* IRB */
	if (status == BERR_SUCCESS)
		status = BIRB_GetDefaultSettings(&IRBSettings, sAppData.hBCHP);

    if (status == BERR_SUCCESS)
    	status = BIRB_Open( &sAppData.hBIRB,sAppData.hBCHP, sAppData.hBREG, sAppData.hBINT, &IRBSettings);

    if (status == BERR_SUCCESS) {
        status = OpenAudDevice();
    }


	/* XPT */
	if (status == BERR_SUCCESS) {
        BXPT_DefaultSettings sDefaults;
        BXPT_GetDefaultSettings(&sDefaults, sAppData.hBCHP);

        BKNI_Memset(&sDefaults.DramBuffers, 0, sizeof(sDefaults.DramBuffers));

        /* For each configured input band */
        for (ii = 0; ii < 5; ii++)
        {
            sDefaults.DramBuffers.IbParserRsSize[ii] = 200;
            sDefaults.DramBuffers.RaveXcCfg.IbParserXcSize[ii] = 200;
            sDefaults.DramBuffers.MesgBufXcCfg.IbParserXcSize[ii] = 8;
        }
        /* For each configured playback parser - need to do two for 1 PB right now */
        for (ii = 0; ii < 2; ii++)
        {
            sDefaults.DramBuffers.PbParserRsSize[ii] = 8;
            sDefaults.DramBuffers.RaveXcCfg.PbParserXcSize[ii] = 8;
            sDefaults.DramBuffers.MesgBufXcCfg.IbParserXcSize[ii] = 8;
        }
		status = BXPT_Open(&sAppData.hBXPT, sAppData.hBCHP, sAppData.hBREG, sAppData.hBMEM, sAppData.hBINT, &sDefaults);
    }


	/* HDM */
	if (status == BERR_SUCCESS)
		status = BHDM_GetDefaultSettings(&HDMSettings);

	if (status == BERR_SUCCESS)
		status = BHDM_Open(&sAppData.hBHDM, sAppData.hBCHP, sAppData.hBREG, sAppData.hBINT, sAppData.hBI2C_Reg[3], &HDMSettings);

    /* open PCR channels */
    for(ii = 0; ii < BXPT_P_MAX_PCRS && status == BERR_SUCCESS; ii++)
    {
        BXPT_PCR_DefSettings sCfg;
        BXPT_PCR_GetChannelDefSettings(sAppData.hBXPT, ii, &sCfg);
        status = BXPT_PCR_Open(sAppData.hBXPT, ii, &sCfg, &sAppData.ahBXPT_PCR[ii]);
    }

    if (status == BERR_SUCCESS)
        status = BPCRlib_Open(&sAppData.hBPCRlib, sAppData.hBCHP);

    /* open PCRlib channels */
    for(ii = 0; ii < BXPT_P_MAX_PCRS && status == BERR_SUCCESS; ii++)
    {
        BPCRlib_ChannelSettings sLibCfg;
        BPCRlib_Channel_GetChannelDefSettings(sAppData.hBPCRlib, &sLibCfg);
        status = BPCRlib_Channel_Create(sAppData.hBPCRlib, sAppData.ahBXPT_PCR[ii], &sAppData.ahBPCRlib_Channel[ii], &sLibCfg);
    }

    /* open Playback channels */
    for(ii = 0; ii < BXPT_P_MAX_PLAYBACKS && status == BERR_SUCCESS; ii++)
    {
        BXPT_Playback_ChannelSettings sSettings;
        BXPT_Playback_GetChannelDefaultSettings(sAppData.hBXPT, ii, &sSettings);
        status = BXPT_Playback_OpenChannel(sAppData.hBXPT, sAppData.ahBXPT_Playback + ii, ii, &sSettings);
    }

    /* open PCRoffset channels */
    for(ii = 0; ii < BXPT_P_MAX_PCR_OFFSET_CHANNELS && status == BERR_SUCCESS; ii++)
    {
        BXPT_PcrOffset_Defaults sOffsetDefaults;
        BXPT_PcrOffset_GetChannelDefSettings(sAppData.hBXPT, ii, &sOffsetDefaults);
        sOffsetDefaults.UsePcrTimeBase = true;
        sOffsetDefaults.WhichPcr = 0;
        status = BXPT_PcrOffset_Open(sAppData.hBXPT, ii, &sOffsetDefaults, &sAppData.ahBXPT_PcrOffset[ii]);
    }

    if (status == BERR_SUCCESS) {
        BXPT_Rave_ChannelSettings sRaveDefaults;
        BXPT_Rave_GetChannelDefaultSettings(sAppData.hBXPT, 0, &sRaveDefaults);
        sRaveDefaults.TpitEventTimeout = TPIT_PACKET_TIMEOUT_MS * TPIT_TICKS_PER_MS;
        status = BXPT_Rave_OpenChannel(sAppData.hBXPT, &sAppData.hBXPT_Rave, 0, &sRaveDefaults);
    }

	/*	Enable system interrupts (L1 handlers) */
	if (status == BERR_SUCCESS)
		bcmMapL1Ints();

	if (status == BERR_SUCCESS)
		bcmEnableL1Ints();
#if (BCHP_CHIP == 7401)
    /*open and setup hsm. This MUST be done after rave channel is open*/
    do {
        BHSM_Settings hsm_config;
        BHSM_ChannelSettings    hsmch_config;
        BHSM_ChannelHandle      BHSM_ChannelHandle[BHSM_HwModule_eMax];
        BHSM_SetMiscBitsIO_t setMiscBitsIO;

        if (status == BERR_SUCCESS)
            status = BHSM_GetDefaultSettings(&hsm_config, sAppData.hBCHP);
        if (status == BERR_SUCCESS)
            status = BHSM_Open(&sAppData.hBHSM, sAppData.hBREG, sAppData.hBCHP, sAppData.hBINT, &hsm_config);
        if (status == BERR_SUCCESS)
            status = BHSM_GetChannelDefaultSettings(sAppData.hBHSM, BHSM_HwModule_eCmdInterface1, &hsmch_config);
        if (status == BERR_SUCCESS)
            status = BHSM_Channel_Open(sAppData.hBHSM, &BHSM_ChannelHandle[BHSM_HwModule_eCmdInterface1], BHSM_HwModule_eCmdInterface1, &hsmch_config);
        if (status == BERR_SUCCESS)
            status = BHSM_Channel_Open(sAppData.hBHSM, &BHSM_ChannelHandle[BHSM_HwModule_eCmdInterface2], BHSM_HwModule_eCmdInterface2, &hsmch_config);

        /*for everything to work correctly please preserve these bits until
          noted otherwise*/
        setMiscBitsIO.setMiscBitsSubCmd = BCMD_SetMiscBitsSubCmd_eRaveBits;
        setMiscBitsIO.bEnableWriteIMem = 1;
        setMiscBitsIO.bEnableReadIMem = 1;
        setMiscBitsIO.bEnableReadDMem = 1;
        setMiscBitsIO.bEnableEncBypass = 0;
        setMiscBitsIO.bDisableClear = 1;

        if (status == BERR_SUCCESS)
            status = BHSM_SetMiscBits (sAppData.hBHSM, &setMiscBitsIO);
    }while(0);

#endif
    InitTransport();

	/* XVD */
	if (status == BERR_SUCCESS)
#if BCHP_CHIP == 7400
        cfg.hFrmMemory = sAppData.hBMEM;
		status = BXVD_Open(&sAppData.hBXVD[0], sAppData.hBCHP, sAppData.hBREG, sAppData.hBMEM_XVD[0], sAppData.ahBMEM_XVD_CHPmem[0], sAppData.hBINT, &cfg );
        cfg.uiAVDInstance = 1;
        status = BXVD_Open(&sAppData.hBXVD[1], sAppData.hBCHP, sAppData.hBREG, sAppData.hBMEM_XVD[1], sAppData.ahBMEM_XVD_CHPmem[1], sAppData.hBINT, &cfg );
#endif

#if BCHP_CHIP == 7401
		status = BXVD_Open(&sAppData.hBXVD, sAppData.hBCHP, sAppData.hBREG, sAppData.hBMEM_XVD, sAppData.hBMEM, sAppData.hBINT, NULL );
#endif

    if (status == BERR_SUCCESS) {
        status = OpenRapDevice();
    }

	if (status != BERR_SUCCESS)
		BDBG_ERR(("InitSystem: failed error %d", (int)status));
    else
        BDBG_WRN(("InitSystem: success"));

	return status;
}

#include "brap_img.h"

BERR_Code
OpenRapDevice(void)
{
    BERR_Code eStatus;
	BRAP_Settings sRapCfg;
	eStatus = BRAP_GetDefaultSettings(&sRapCfg);
	if (eStatus != BERR_SUCCESS) { return eStatus; }
	sRapCfg.bExtDeviceMem = false;
	sRapCfg.pImgInterface = &BRAP_IMG_Interface;
	sRapCfg.pImgContext = BRAP_IMG_Context;

	sRapCfg.bSupportAlgos[BRAP_DSPCHN_AudioType_eMpeg] = true;
	sRapCfg.bSupportAlgos[BRAP_DSPCHN_AudioType_eAac] = true;
	sRapCfg.bSupportAlgos[BRAP_DSPCHN_AudioType_eAacSbr] = true;
    sRapCfg.bSupportAlgos[BRAP_DSPCHN_AudioType_eAc3] = true;
	sRapCfg.bSupportAlgos[BRAP_DSPCHN_AudioType_eAc3Plus] = true;
//    BDBG_SetModuleLevel("rap_dsp", BDBG_eMsg);
    /* Open the RAP device */
    eStatus = BRAP_Open(&sAppData.hBRAP,
                   GetBCHP(),
                   GetBREG(),
                   GetBMEM(),
                   GetBINT(),
                   GetBTMR(),
                   &sRapCfg);
	return eStatus;
}


BERR_Code OpenAudDevice(void)
{
    return BERR_SUCCESS;
}

 /***********************************************************************
 *
 *  OpenI2cDevice()
 *
 *  Open I2C device and create I2C reg handle
 *
 ***********************************************************************/
BERR_Code OpenI2cDevice(void)
{
    BI2C_ChannelSettings	defChnSettings;
	BERR_Code 				status = BERR_SUCCESS;
	uint32_t				i;

    BDBG_SetModuleLevel("bi2c", BDBG_eTrace);

    status = BI2C_Open(&sAppData.hBI2C, sAppData.hBCHP, sAppData.hBREG,
    						sAppData.hBINT, NULL);

	for (i=0; i < BI2C_MAX_I2C_CHANNELS; i++)
	{
    	status = BI2C_GetChannelDefaultSettings(sAppData.hBI2C, i, &defChnSettings );

    	defChnSettings.clkRate = BI2C_Clk_eClk100Khz;

	    status = BI2C_OpenChannel(sAppData.hBI2C, &sAppData.hBI2C_Chan[i], i, &defChnSettings);
	    status = BI2C_CreateI2cRegHandle (sAppData.hBI2C_Chan[i], &sAppData.hBI2C_Reg[i]);
	}

    return status ;
}

 /***********************************************************************
 *
 *  OpenSpiDevice()
 *
 *  Open SPI device and create SPI reg handle
 *
 ***********************************************************************/
BERR_Code OpenSpiDevice(void)
{
    BSPI_ChannelSettings	defChnSettings;
	BERR_Code 				status = BERR_SUCCESS;
	uint32_t				i;

    status = BSPI_Open(&sAppData.hBSPI, sAppData.hBCHP,
    						sAppData.hBREG, sAppData.hBINT, NULL);


	for (i=0; i < MAX_SPI_CHANNELS; i++)
	{
	    status = BSPI_GetChannelDefaultSettings(sAppData.hBSPI, i, &defChnSettings);
    	status = BSPI_OpenChannel(sAppData.hBSPI, &sAppData.hBSPI_Chan[i], i, &defChnSettings);
	    status = BSPI_CreateSpiRegHandle (sAppData.hBSPI_Chan[i], &sAppData.hBSPI_Reg[i]);
    }

    return status;
}


 /***********************************************************************
 *
 *  OpenIcpDevice()
 *
 *  Open ICP device.
 *
 ***********************************************************************/
BERR_Code OpenIcpDevice(void)
{
	BERR_Code 				status = BERR_SUCCESS;
	unsigned int			i;
    BICP_ChannelSettings	defChnSettings;

    status = BICP_Open(&sAppData.hBICP, sAppData.hBCHP,
    						sAppData.hBREG, sAppData.hBINT, NULL);

	for (i=0; i < MAX_ICP_CHANNELS; i++)
	{
	    status = BICP_GetChannelDefaultSettings(sAppData.hBICP, i, &defChnSettings);
    	status = BICP_OpenChannel(sAppData.hBICP, &sAppData.hBICP_Chan[i], i, &defChnSettings);
	}

	return status;
}

/*
	Closes and Re-opens the GPIO module.
	This is only here for testing purposes.
	The module must already be open, otherwise an error is returned.
*/
BERR_Code CloseAndReopenGIO(void)
{
	BERR_Code status = BERR_SUCCESS;

	/* Must already be opened for this test */
	if (sAppData.hBGIO == NULL)
		status = BERR_INVALID_PARAMETER;

	/* Close the module */
	if (status == BERR_SUCCESS)
		status = BGIO_Close(sAppData.hBGIO);

	/* Re-open */
	if (status == BERR_SUCCESS)
	{
		sAppData.hBGIO = NULL;
		status = BGIO_Open(&sAppData.hBGIO, sAppData.hBCHP, sAppData.hBREG);
	}

	return status;
}

#if 0
/*
	Closes and Re-opens the UART module.
	This is only here for testing purposes.
	The module must already be open, otherwise an error is returned.
*/
BERR_Code CloseAndReopenURT(void)
{
	BERR_Code status = BERR_SUCCESS;

	/* Must already be opened for this test */
	if (sAppData.hBURT == NULL)
		status = BERR_INVALID_PARAMETER;

	/* Close the module */
	if (status == BERR_SUCCESS)
		status = BURT_Close(sAppData.hBURT);

	/* Re-open */
	if (status == BERR_SUCCESS)
	{
		sAppData.hBURT = NULL;
		status = BURT_Open(&sAppData.hBURT, sAppData.hBCHP, sAppData.hBREG, sAppData.hBINT, NULL);
	}

	return status;
}
#endif
#define B_IRQ_ID_1_N(id,n) BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_##id##_CPU_INTR_##n##_SHIFT + 32
#if BCHP_CHIP==7400
#define INT1_ID_AVD1_CPU_INTR B_IRQ_ID_1_N(AVD1, 1)
#endif

/***********************************************************************
 *
 *  bcmMapL1Ints: Map all L1 interrupts here
 *
 *
 ***********************************************************************/
void bcmMapL1Ints (void)
{
	FN_L1_ISR	f;
	void*		v;

	#define UL unsigned long
	f = (FN_L1_ISR)BINT_Isr;

	/* Standard 7401 interrupts */
	v = (void *)sAppData.hBINT;

	/* XPT Interrupt */
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_XPT_MSG_STAT_CPU_INTR, f,
						v, INT1_ID_XPT_MSG_STAT_CPU_INTR);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_XPT_FE_CPU_INTR, f,
						v, INT1_ID_XPT_FE_CPU_INTR);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_XPT_PCR_CPU_INTR, f,
						v, INT1_ID_XPT_PCR_CPU_INTR);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_XPT_RAV_CPU_INTR, f,
						v, INT1_ID_XPT_RAV_CPU_INTR);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_XPT_STATUS_CPU_INTR, f,
						v, INT1_ID_XPT_STATUS_CPU_INTR);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_XPT_OVFL_CPU_INTR, f,
						v, INT1_ID_XPT_OVFL_CPU_INTR);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_XPT_MSG_CPU_INTR, f,
						v, INT1_ID_XPT_MSG_CPU_INTR);

	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_BSP_CPU_INTR, f,
						v, INT1_ID_BSP_CPU_INTR);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_AIO_CPU_INTR, f,
						v, INT1_ID_AIO_CPU_INTR);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_GFX_CPU_INTR, f,
						v, INT1_ID_GFX_CPU_INTR);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_HDMI_CPU_INTR, f,
						v, INT1_ID_HDMI_CPU_INTR);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_RPTD_CPU_INTR, f,
						v, INT1_ID_RPTD_CPU_INTR);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_VEC_CPU_INTR, f,
						v, INT1_ID_VEC_CPU_INTR);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_BVNB_CPU_INTR, f,
						v, INT1_ID_BVNB_CPU_INTR);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_BVNF_CPU_INTR_0, f,
						v, INT1_ID_BVNF_CPU_INTR_0);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_BVNF_CPU_INTR_1, f,
						v, INT1_ID_BVNF_CPU_INTR_1);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_BVNF_CPU_INTR_2, f,
						v, INT1_ID_BVNF_CPU_INTR_2);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_BVNF_CPU_INTR_3, f,
						v, INT1_ID_BVNF_CPU_INTR_3);

	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_RFM_CPU_INTR, f,
						v, INT1_ID_RFM_CPU_INTR);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_MC_CPU_INTR, f,
						v, INT1_ID_MC_CPU_INTR);
	/* AVD0 */
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_AVD0_CPU_INTR, f,
						v, INT1_ID_AVD0_CPU_INTR);
#if BCHP_CHIP == 7400
	/* AVD1 */
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_AVD1_CPU_INTR, f,
						v, INT1_ID_AVD1_CPU_INTR);
#endif
	/*
	 * Add more here...
	 */

	/* UPG/BSC/SPI interrupts */
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_UPG_CPU_INTR, f,
						v, INT1_ID_UPG_CPU_INTR);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_UPG_BSC_CPU_INTR, f,
						v, INT1_ID_UPG_BSC_CPU_INTR);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_UPG_SPI_CPU_INTR, f,
						v, INT1_ID_UPG_SPI_CPU_INTR);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_UPG_SC_CPU_INTR, f,
						v, INT1_ID_UPG_SC_CPU_INTR);
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_UPG_TMR_CPU_INTR, f,
						v, INT1_ID_UPG_TMR_CPU_INTR);

	/* SATA */
//	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_PCI_SATA_CPU_INTR, f,
//						v, INT1_ID_PCI_SATA_CPU_INTR);
	/* UHF */
	CPUINT1_ConnectIsr_vxWorks((unsigned long)INT1_ID_UHF1_CPU_INTR, f,
						v, INT1_ID_UHF1_CPU_INTR);
}

/***********************************************************************
 *
 *  bcmEnableL1Ints: Enable L1 interrupts here
 *
 *
 ***********************************************************************/
void bcmEnableL1Ints (void)
{
	CPUINT1_Enable (INT1_ID_UPG_CPU_INTR);
	CPUINT1_Enable (INT1_ID_UPG_BSC_CPU_INTR);
	CPUINT1_Enable (INT1_ID_UPG_SPI_CPU_INTR);
 	CPUINT1_Enable (INT1_ID_UPG_SC_CPU_INTR);
	CPUINT1_Enable (INT1_ID_UPG_TMR_CPU_INTR);
	CPUINT1_Enable (INT1_ID_UHF1_CPU_INTR);
#if 1
	CPUINT1_Enable (INT1_ID_PCI_SATA_CPU_INTR);
#endif
#if 1
 	CPUINT1_Enable (INT1_ID_BSP_CPU_INTR);
	CPUINT1_Enable (INT1_ID_AIO_CPU_INTR);
	CPUINT1_Enable (INT1_ID_GFX_CPU_INTR);
	CPUINT1_Enable (INT1_ID_HDMI_CPU_INTR);
	CPUINT1_Enable (INT1_ID_RPTD_CPU_INTR);

	CPUINT1_Enable (INT1_ID_VEC_CPU_INTR);
	CPUINT1_Enable (INT1_ID_BVNB_CPU_INTR);
	CPUINT1_Enable (INT1_ID_BVNF_CPU_INTR_0);
	CPUINT1_Enable (INT1_ID_BVNF_CPU_INTR_1);
	CPUINT1_Enable (INT1_ID_BVNF_CPU_INTR_2);
	CPUINT1_Enable (INT1_ID_BVNF_CPU_INTR_3);
	CPUINT1_Enable (INT1_ID_RFM_CPU_INTR);
	CPUINT1_Enable (INT1_ID_MC_CPU_INTR);

	/* AVD0 */
	CPUINT1_Enable (INT1_ID_AVD0_CPU_INTR);
#if BCHP_CHIP==7400
	CPUINT1_Enable (INT1_ID_AVD1_CPU_INTR);
#endif
	CPUINT1_Enable (INT1_ID_XPT_MSG_STAT_CPU_INTR);
	CPUINT1_Enable (INT1_ID_XPT_FE_CPU_INTR);
	CPUINT1_Enable (INT1_ID_XPT_PCR_CPU_INTR);
	CPUINT1_Enable (INT1_ID_XPT_RAV_CPU_INTR);
	CPUINT1_Enable (INT1_ID_XPT_STATUS_CPU_INTR);
	CPUINT1_Enable (INT1_ID_XPT_OVFL_CPU_INTR);
	CPUINT1_Enable (INT1_ID_XPT_MSG_CPU_INTR);
#endif
}

BCHP_Handle GetBCHP(void)
{
	BDBG_ASSERT(sAppData.hBCHP);
	return sAppData.hBCHP;
}
BREG_Handle GetBREG(void)
{
	BDBG_ASSERT(sAppData.hBREG);
	return sAppData.hBREG;
}
BINT_Handle GetBINT(void)
{
	BDBG_ASSERT(sAppData.hBINT);
	return sAppData.hBINT;
}
BMEM_Handle GetBMEM(void)
{
	BDBG_ASSERT(sAppData.hBMEM);
	return sAppData.hBMEM;
}

/* Introduced post 7401A0 phase 2 */
BMEM_ModuleHandle GetBMEM_ModuleHandle(void)
{
	BDBG_ASSERT(sAppData.hBMEM_ModuleHandle);
	return sAppData.hBMEM_ModuleHandle;
}

BTMR_Handle GetBTMR(void)
{
	BDBG_ASSERT(sAppData.hBTMR);
	return sAppData.hBTMR;
}

BI2C_Handle GetBI2C(void)
{
	BDBG_ASSERT(sAppData.hBI2C);
	return sAppData.hBI2C;
}

BI2C_ChannelHandle GetBI2C_Chan(uint32_t index)
{
	BDBG_ASSERT(index < BI2C_MAX_I2C_CHANNELS && sAppData.hBI2C_Chan[index]);
	return sAppData.hBI2C_Chan[index];
}

BREG_I2C_Handle GetBI2C_Reg(uint32_t index)
{
	BDBG_ASSERT(index < BI2C_MAX_I2C_CHANNELS && sAppData.hBI2C_Reg[index]);
	return sAppData.hBI2C_Reg[index];
}

BSPI_Handle GetBSPI(void)
{
	BDBG_ASSERT(sAppData.hBSPI);
	return sAppData.hBSPI;
}

BSPI_ChannelHandle GetBSPI_Chan(uint32_t index)
{
	BDBG_ASSERT(index < MAX_SPI_CHANNELS && sAppData.hBSPI_Chan[index]);
	return sAppData.hBSPI_Chan[index];
}

BREG_SPI_Handle GetBSPI_Reg(uint32_t index)
{
	BDBG_ASSERT(index < MAX_SPI_CHANNELS && sAppData.hBSPI_Reg[index]);
	return sAppData.hBSPI_Reg[index];
}

BICP_Handle GetBICP(void)
{
	BDBG_ASSERT(sAppData.hBICP);
	return sAppData.hBICP;
}

BICP_ChannelHandle GetBICP_Chan(uint32_t index)
{
	BDBG_ASSERT(index < MAX_ICP_CHANNELS && sAppData.hBICP_Chan[index]);
	return sAppData.hBICP_Chan[index];
}

BIRB_Handle GetBIRB(void)
{
	BDBG_ASSERT(sAppData.hBIRB);
	return sAppData.hBIRB;
}

BVDC_Handle GetBVDC(void)
{
	BDBG_ASSERT(sAppData.hBVDC);
	return sAppData.hBVDC;
}

BRDC_Handle GetBRDC(void)
{
	BDBG_ASSERT(sAppData.hBRDC);
	return sAppData.hBRDC;
}

BGRC_Handle GetBGRC(void)
{
	BDBG_ASSERT(sAppData.hBGRC);
	return sAppData.hBGRC;
}

BGIO_Handle GetBGIO(void)
{
	BDBG_ASSERT(sAppData.hBGIO);
	return sAppData.hBGIO;
}
#if 0
BURT_Handle GetBURT(void)
{
	BDBG_ASSERT(sAppData.hBURT);
	return sAppData.hBURT;
}
#endif
BXPT_Handle GetBXPT(void)
{
	BDBG_ASSERT(sAppData.hBXPT);
	return sAppData.hBXPT;
}

BHDM_Handle GetBHDM(void)
{
	BDBG_ASSERT(sAppData.hBHDM);
	return sAppData.hBHDM;
}

#if BCHP_CHIP == 7401
BXVD_Handle GetBXVD(uint32_t index)
{
	BDBG_ASSERT(sAppData.hBXVD);
	return sAppData.hBXVD;
}
#endif

#if BCHP_CHIP == 7400
BXVD_Handle GetBXVD(uint32_t index)
{
	BDBG_ASSERT(sAppData.hBXVD);
	return sAppData.hBXVD[index];
}
#endif

BRAP_Handle GetBRAP(void)
{
	BDBG_ASSERT(sAppData.hBRAP);
	return sAppData.hBRAP;
}

BPCRlib_Handle
GetBPCRlib(void)
{
	BDBG_ASSERT(sAppData.hBPCRlib);
	return sAppData.hBPCRlib;
}

BPCRlib_Channel
GetBPCRlib_Channel(uint32_t uIndex)
{
	BDBG_ASSERT(uIndex < BXPT_P_MAX_PCRS && sAppData.ahBPCRlib_Channel[uIndex]);
    return sAppData.ahBPCRlib_Channel[uIndex];
}

BXPT_PCR_Handle
GetBXPT_PCR(uint32_t uIndex)
{
	BDBG_ASSERT(uIndex < BXPT_P_MAX_PCRS && sAppData.ahBXPT_PCR[uIndex]);
    return sAppData.ahBXPT_PCR[uIndex];
}

BXPT_Playback_Handle
GetBXPT_Playback(uint32_t uIndex)
{
	BDBG_ASSERT(uIndex < BXPT_P_MAX_PLAYBACKS && sAppData.ahBXPT_Playback[uIndex]);
    return sAppData.ahBXPT_Playback[uIndex];
}


BXPT_PcrOffset_Handle
GetBXPT_PcrOffset(uint32_t uIndex)
{
	BDBG_ASSERT(uIndex < BXPT_P_MAX_PCR_OFFSET_CHANNELS && sAppData.ahBXPT_PcrOffset[uIndex]);
    return sAppData.ahBXPT_PcrOffset[uIndex];
}

BXPT_Rave_Handle
GetBXPT_Rave(void)
{
	BDBG_ASSERT(sAppData.hBXPT_Rave);
	return sAppData.hBXPT_Rave;
}

BERR_Code InitUPGModules(void)
{
	BERR_Code	status = BERR_SUCCESS;

	if (status == BERR_SUCCESS)
		status = OpenI2cDevice();

	if (status == BERR_SUCCESS)
		status = OpenSpiDevice();

	if (status == BERR_SUCCESS)
		status = OpenIcpDevice();


	return status;
}

/*------------------------------------------------------------
	stubs to avoid compile errors
------------------------------------------------------------*/
void InitAudioData(void)
{
}

void
InitTransport(void)
{
}

static BVDC_Source_Handle	src;
static BVDC_Window_Handle	wnd;

BERR_Code TestVideoDisplayForFormat(uint32_t uDecoder, BFMT_VideoFmt vidfmt, bool bInstallPTSCallbacks)
{
	BERR_Code			status = BERR_SUCCESS;
	uint32_t			width, height;
	WndPlacement_t		unclipped;
	BFMT_VideoInfo		vidinfo;
	uint32_t			channelNum;
	uint32_t			srcwndId;

	channelNum = uDecoder;
	srcwndId   = uDecoder;

    if (uDecoder >= 2) {
        return BERR_INVALID_PARAMETER;
    }

	if (status == BERR_SUCCESS)
		status = BFMT_GetVideoFormatInfo(vidfmt, &vidinfo);

	if (status == BERR_SUCCESS)
	{
		width  = vidinfo.ulWidth;
		height = vidinfo.ulHeight;
		SetUnscaledPlacement(&unclipped, width, height);
	}

	/* Set video to requested format */
	if (uDecoder ==0 && status == BERR_SUCCESS)
		status = SetDisplayFormat(BVDC_CompositorId_eCompositor0, vidfmt, true);

	if (status == BERR_SUCCESS)
		status = MakeMpegSrc(BVDC_CompositorId_eCompositor0, srcwndId, uDecoder, &src);

	/* Open and configure the vd chan */
	if (status == BERR_SUCCESS)
		status = OpenVideoChan(channelNum, src, bInstallPTSCallbacks);

	if (status == BERR_SUCCESS)
		status = MakeMpegWnd(BVDC_CompositorId_eCompositor0, srcwndId, src, &wnd);

	if (status == BERR_SUCCESS)
		status = PlaceWindow(wnd, &unclipped, true);

	return status;
}
#if (BCHP_CHIP == 7400)
/* B0 and beyond */
static void bsettop_p_init_pinmuxes_b0(BREG_Handle hReg)
{
	uint32_t reg;

	/* Writing pinmuxes in order from pg. 14 of the schematic */

	/* GPIO 000..001 set by OS */

	/* GPIO 002 */
	reg = BREG_Read32(hReg,
					  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);

	reg &= ~(
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_002)
			);

	reg |=
#if BCM_BOARD == 97400
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_002, 0) /* GPIO_002 -> POD_EN */
#else
	BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_002, 1) /* MII_RX_CLK */
#endif
	;

	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2,
				 reg);

	/* GPIO 003..006 set by OS */

	/* GPIO 007..013 */
	reg = BREG_Read32(hReg,
					  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
	reg &= ~(
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_007 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_008 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_009 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_010 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_011 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_012 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_013 )
			);
	reg |=
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_007, 4 ) | /* UART 1 RX -> tp_in_07 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_008, 4 ) | /* UART 1 TX -> tp_out_08 */
#if BCM_BOARD == 97400
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_009, 3 ) | /* IRQ1 [3517 IRQ] */
#else
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_009, 0 ) | /* POD_VS1 */
#endif
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_010, 3 ) | /* IRQ2 [Test] */
#ifdef AVD_UART
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_011, 4 ) | /* UART 2 RX -> tp_in_11 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_012, 4 ) | /* UART 2 TX -> tp_out_12 */
#else
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_011, 2 ) | /* UART 2 RX -> uart_rxd_2 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_012, 2 ) | /* UART 2 TX -> uart_txd_2 */
#endif
#if BCM_BOARD == 97400
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_013, 4 )	 /* IRQ3 [POD] */
#else
	BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_013, 0 )	 /* POD_CD1 */
#endif
	;

	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3,
				 reg);

	/* GPIO 014..023 */
	reg =
#if BCM_BOARD == 97400
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_014, 4 ) | /* IRQ4 [Slot 2] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_015, 2 ) | /* IRQ5 [Diseqc] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_016, 2 ) | /* IRQ6 [4500 IRQ] */
#else
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_014, 0 ) | /* POD_CD2 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_015, 0 ) | /* POD_MDET */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_016, 0 ) | /* POD_VS2 */
#endif
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_017, 3 ) | /* AUD_FS_CLK0 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_018, 3 ) | /* AUD_FS_CLK1 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_019, 3 ) | /* SC_CLK_OUT_1 */
#if BCM_BOARD == 97400
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_020, 2 ) | /* IRQ13 [Slot 1a] */
#else
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_020, 0 ) | /* POD_VPP1_MSEL */
#endif
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_021, 5 ) | /* SC_CLK_OUT_2 */
#if BCM_BOARD == 97400
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_022, 2 ) | /* IRQ 11 [Slot 0b] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_023, 2 )	 /* IRQ 12 [Slot 0a] */
#else
	BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_022, 0 ) | /* POD_RST */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_023, 0 )	 /* POD_VPP2_ON */
#endif
	;

	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4,
				 reg);

	/* GPIO 024..033 */
	reg =
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_024, 1 ) | /* VI0_656_CLK */
#if BCM_BOARD == 97400
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_025, 1 ) | /* MPOD_SCLK */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_026, 1 ) | /* MPOD_SDI */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_027, 1 ) | /* MPOD_SDO */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_028, 1 ) | /* MPOD_SCTL */
#else
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_025, 2 ) | /* SPI_M_SCK */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_026, 2 ) | /* SPI_M_MOSI */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_027, 2 ) | /* SPI_M_MISO */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_028, 2 ) | /* SPI_M_SS0b */
#endif
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_029, 2 ) | /* PPKT_IN_SYNC */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_030, 2 ) | /* PPKT_IN_VALID */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_031, 2 ) | /* PPKT_IN_DATA0 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_032, 2 ) | /* PPKT_IN_DATA1 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_033, 2 )	 /* PPKT_IN_DATA2 */
	;

	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5,
				 reg);

	/* GPIO 034..043 */
	reg =
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_034, 2 ) | /* PPKT_IN_DATA3 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_035, 2 ) | /* PPKT_IN_DATA4 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_036, 2 ) | /* PPKT_IN_DATA5 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_037, 2 ) | /* PPKT_IN_DATA6 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_038, 2 ) | /* PPKT_IN_DATA7 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_039, 2 ) | /* PPKT_IN_CLK */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_040, 2 ) | /* RMXP_OUT_VALID */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_041, 2 ) | /* RMXP_OUT_SYNC */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_042, 2 ) | /* RMXP_OUT_DATA0 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_043, 4 )	 /* NDS_SC_AUX0 */
	;

	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6,
				 reg);

	/* GPIO 044..053 */
	reg =
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_044, 4 ) | /* NDS_SC_AUX1 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_045, 4 ) | /* NDS_SC_VCTRL1 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_046, 4 ) | /* SC_IO_2 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_047, 4 ) | /* SC_CLK_2 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_048, 4 ) | /* SC_RST_2 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_049, 4 ) | /* SC_PRES_2 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_050, 4 ) | /* SC_VCC_2 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_051, 1 ) | /* PKT_ERROR0 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_052, 1 ) | /* PKT_ERROR1 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_053, 1 )	 /* PKT_ERROR2 */
	;

	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7,
				 reg);

	/* GPIO 054..064 */
	reg =
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_054, 1 ) | /* PKT_ERROR3 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_055, 2 ) | /* VI0_656_D0 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_056, 2 ) | /* VI0_656_D1 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_057, 2 ) | /* VI0_656_D2 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_058, 2 ) | /* VI0_656_D3 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_059, 2 ) | /* VI0_656_D4 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_060, 2 ) | /* VI0_656_D5 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_061, 2 ) | /* VI0_656_D6 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_062, 2 ) | /* VI0_656_D7 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_063, 1 ) | /* PKT_CLK_0 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_064, 1 )	 /* PKT_CLK_1 */
	;

	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8,
				 reg);

	/* GPIO 065..078 */
	reg =
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_065, 1 ) | /* PKT_CLK_2 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_066, 1 ) | /* PKT_CLK_3 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_067, 1 ) | /* PKT_CLK_4 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_068, 1 ) | /* PKT_CLK_5 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_069, 1 ) | /* PKT_DATA0 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_070, 1 ) | /* PKT_DATA1 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_071, 1 ) | /* PKT_DATA2 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_072, 1 ) | /* PKT_DATA3 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_073, 1 ) | /* PKT_DATA4 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_074, 1 ) | /* PKT_DATA5 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_075, 1 ) | /* PKT_SYNC0 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_076, 1 ) | /* PKT_SYNC1 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_077, 1 ) | /* PKT_SYNC2 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_078, 1 )	 /* PKT_SYNC3 */
	;

	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9,
				 reg);

	/* GPIO 079..088 */
	reg =
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_079, 1 ) | /* PKT_SYNC4 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_080, 1 ) | /* PKT_SYNC5 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_081, 1 ) | /* SC_IO_1 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_082, 1 ) | /* SC_CLK_1 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_083, 1 ) | /* SC_RST_1 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_084, 1 ) | /* SC_PRES_1 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_085, 1 ) | /* SC_VCC_1 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_086, 1 ) | /* GP_LED_KD_0 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_087, 1 ) | /* GP_LED_KD_1 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_088, 1 )	  /* GP_LED_KD_2 */
	;

	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,
				 reg);

	/* GPIO 089..098 */
	reg =
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_089, 1 ) | /* GP_LED_KD_3 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_090, 1 ) | /* GP_LED_LS_0 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_091, 1 ) | /* GP_LED_LS_1 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_092, 1 ) | /* GP_LED_LS_2 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_093, 1 ) | /* GP_LED_LS_3 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_094, 1 ) | /* GP_LED_LS_4 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_095, 1 ) | /* GP_LED_LD_0 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_096, 1 ) | /* GP_LED_LD_1 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_097, 1 ) | /* GP_LED_LD_2 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_098, 1 )	  /* GP_LED_LD_3 */
	;

	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,
				 reg);

	/* GPIO 099..109 */
	reg =
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_099, 1 ) | /* GP_LED_LD_4 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_100, 1 ) | /* GP_LED_LD_5 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101, 1 ) | /* GP_LED_LD_6 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102, 1 ) | /* GP_LED_LD_7 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103, 1 ) | /* SC_EXT_CLK */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104, 1 ) | /* SC_IO_0 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_105, 1 ) | /* SC_CLK_0 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_106, 1 ) | /* SC_RST_0 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_107, 1 ) | /* SC_PRES_0 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_108, 1 ) | /* SC_VCC_0 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_109, 3 )	  /* NDS_SC_AUX0_2 */
	;

	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12,
				 reg);

	/* GPIO 110..112 SGPIO 00..09 */
	reg = BREG_Read32(hReg,
					  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);

	reg = reg & BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, byp_sys9_clk);

	reg |=
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_110, 3 ) | /* NDS_SC_AUX1_2 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_111, 3 ) | /* NDS_SC_VCTRL_2 */
#if BCM_BOARD == 97400
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_112, 2 ) | /* IRQ14 [Slot 1b] */
#else
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_112, 0 ) | /* POD_VCC_ON */
#endif
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_00, 1 ) | /* BSC_M0_SCL */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_01, 1 ) | /* BSC_M0_SDA */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_02, 1 ) | /* BSC_M1_SCL */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_03, 1 ) | /* BSC_M1_SDA */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_04, 1 ) | /* BSC_M2_SCL */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_05, 1 ) | /* BSC_M2_SDA */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_06, 1 ) | /* BSC_M3_SCL */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_07, 1 ) | /* BSC_M3_SDA */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_08, 1 ) | /* BSC_M4_SCL */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_09, 1 )	  /* BSC_M4_SDA */
	;

	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13,
				 reg);
}

static void bsettop_p_init_vcxo_b0(BREG_Handle hRegister, unsigned index)
{
	uint32_t ui32RegVal;
	unsigned offset = (BCHP_VCXO_1_RM_CONTROL - BCHP_VCXO_0_RM_CONTROL)*index;

	ui32RegVal = BCHP_FIELD_DATA(VCXO_0_RM_CONTROL, RESET, 1);
	BREG_Write32(hRegister, BCHP_VCXO_0_RM_CONTROL+offset, ui32RegVal);

	/* assert reset */
	if ( index == 0 )
	{
		ui32RegVal = BREG_Read32(hRegister, BCHP_VCXO_CTL_MISC_VC0_CTRL);
		ui32RegVal &= ~BCHP_MASK(VCXO_CTL_MISC_VC0_CTRL, DRESET);
		ui32RegVal |= BCHP_FIELD_DATA(VCXO_CTL_MISC_VC0_CTRL, DRESET, 1);
		BREG_Write32(hRegister, BCHP_VCXO_CTL_MISC_VC0_CTRL, ui32RegVal);
	}
	else
	{
		ui32RegVal = BREG_Read32(hRegister, BCHP_VCXO_CTL_MISC_VC1_CTRL);
		ui32RegVal &= ~BCHP_MASK(VCXO_CTL_MISC_VC1_CTRL, DRESET);
		ui32RegVal |= BCHP_FIELD_DATA(VCXO_CTL_MISC_VC1_CTRL, DRESET, 1);
		BREG_Write32(hRegister, BCHP_VCXO_CTL_MISC_VC1_CTRL, ui32RegVal);
	}

	ui32RegVal = BCHP_FIELD_DATA(VCXO_0_RM_RATE_RATIO, DENOMINATOR, 32);
	BREG_Write32(hRegister, BCHP_VCXO_0_RM_RATE_RATIO+offset, ui32RegVal);

	ui32RegVal =
		BCHP_FIELD_DATA(VCXO_0_RM_SAMPLE_INC, NUMERATOR, 29) |
		BCHP_FIELD_DATA(VCXO_0_RM_SAMPLE_INC, SAMPLE_INC, 3);
	BREG_Write32(hRegister, BCHP_VCXO_0_RM_SAMPLE_INC+offset, ui32RegVal);

	ui32RegVal = BCHP_FIELD_DATA(VCXO_0_RM_PHASE_INC, PHASE_INC, 0x7d634);
	BREG_Write32(hRegister, BCHP_VCXO_0_RM_PHASE_INC+offset, ui32RegVal);

	BREG_Write32(hRegister, BCHP_VCXO_0_RM_INTEGRATOR+offset, 0x0);

	ui32RegVal =
		BCHP_FIELD_DATA(VCXO_0_RM_FORMAT, SHIFT, 2) |
		BCHP_FIELD_DATA(VCXO_0_RM_FORMAT, STABLE_COUNT, 10000);
	BREG_Write32(hRegister, BCHP_VCXO_0_RM_FORMAT+offset, ui32RegVal);

	ui32RegVal =
		BCHP_FIELD_DATA(VCXO_0_RM_OFFSET, OFFSET_ONLY, 0) |
		BCHP_FIELD_DATA(VCXO_0_RM_OFFSET, OFFSET, 0x10000000);
	BREG_Write32(hRegister, BCHP_VCXO_0_RM_OFFSET+offset, ui32RegVal);

	ui32RegVal =
		BCHP_FIELD_DATA(VCXO_0_RM_CONTROL, TIMEBASE, index) |	/* Timebase 0 */
		BCHP_FIELD_DATA(VCXO_0_RM_CONTROL, FREE_RUN, 0) |
		BCHP_FIELD_DATA(VCXO_0_RM_CONTROL, DITHER, 1) |
		BCHP_FIELD_DATA(VCXO_0_RM_CONTROL, DIRECT_GAIN, 2) |
		BCHP_FIELD_DATA(VCXO_0_RM_CONTROL, INT_GAIN, 4) |
		BCHP_FIELD_DATA(VCXO_0_RM_CONTROL, RESET, 0);
	BREG_Write32(hRegister, BCHP_VCXO_0_RM_CONTROL+offset, ui32RegVal);

	/* Deassert reset */
	if ( index == 0 )
	{
		ui32RegVal = BREG_Read32(hRegister, BCHP_VCXO_CTL_MISC_VC0_CTRL);
		ui32RegVal &= ~BCHP_MASK(VCXO_CTL_MISC_VC0_CTRL, DRESET);
		BREG_Write32(hRegister, BCHP_VCXO_CTL_MISC_VC0_CTRL, ui32RegVal);
	}
	else
	{
		ui32RegVal = BREG_Read32(hRegister, BCHP_VCXO_CTL_MISC_VC1_CTRL);
		ui32RegVal &= ~BCHP_MASK(VCXO_CTL_MISC_VC1_CTRL, DRESET);
		BREG_Write32(hRegister, BCHP_VCXO_CTL_MISC_VC1_CTRL, ui32RegVal);
	}
}
#endif
BERR_Code ConfigurePinMux(void)
{
    BREG_Handle hReg = GetBREG();
    uint32_t reg;
	bcmConfigureTm(hReg);

#if (BCHP_CHIP == 7401)
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_22 ) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_23 )
            );
    reg |=
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_22, 6 ) |
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_23, 5 )
        ;
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, reg);


    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_48 ) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_49 )
            );
    reg |=
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_48, 4 ) |
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_49, 4 )
        ;
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, reg);
#endif
#if (BCHP_CHIP == 7400)
    // settop implementation

	/* Reseet is triggered on a rising-edge pulse.  Must write 0,1 */
	BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_SW_RESET, 0);
	BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_SW_RESET, BCHP_SUN_TOP_CTRL_SW_RESET_ext_sys_reset_1shot_MASK);

	bsettop_p_init_pinmuxes_b0(hReg);
	bsettop_p_init_vcxo_b0(hReg, 0);
	bsettop_p_init_vcxo_b0(hReg, 1);

#endif
	return BERR_SUCCESS;
}

#if (BCHP_CHIP == 7401)
void GetDacWiring(uint32_t* dacBluePb, uint32_t* dacGreenY, uint32_t* dacRedPr,
				  uint32_t* dacLuma,   uint32_t* dacChroma, uint32_t* dacComp
                  )
{
	*dacBluePb = B_DAC_Pb_0;
	*dacGreenY = B_DAC_Y_0;
	*dacRedPr  = B_DAC_Pr_0;
	*dacLuma   = B_DAC_YC_Y_0;
	*dacChroma = B_DAC_YC_C_0;
	*dacComp   = B_DAC_C_0;
}
#endif
#if (BCHP_CHIP == 7400)
void GetDacWiring(uint32_t* dacBluePb, uint32_t* dacGreenY, uint32_t* dacRedPr,
				  uint32_t* dacLuma,   uint32_t* dacChroma, uint32_t* dacComp, uint32_t *dacComp1)
{
	*dacBluePb = B_DAC_Pb_0;
	*dacGreenY = B_DAC_Y_0;
	*dacRedPr  = B_DAC_Pr_0;
	*dacLuma   = B_DAC_YC_Y_0;
	*dacChroma = B_DAC_YC_C_0;
	*dacComp   = B_DAC_C_0;
    *dacComp1  = B_DAC_C_1;
}


#endif

void GetHeapConfig(uint32_t* heapStart, uint32_t* heapOffset, uint32_t* heapSize, uint32_t* heapXvdSize, uint32_t* heapAlignment)
{
	*heapStart     = HEAP_START;
	*heapOffset    = HEAP_OFFSET;
	*heapSize      = HEAP_SIZE;
    *heapXvdSize   = HEAP_XVD_SIZE;
	*heapAlignment = HEAP_ALIGNMENT;
}

BERR_Code SetDisplayFormat(BVDC_CompositorId theCompositor, BFMT_VideoFmt theFormat, bool applyNow)
{
	BERR_Code		status = BERR_SUCCESS;
	uint32_t		dacBluePb, dacGreenY, dacRedPr;
	uint32_t		dacLuma,   dacChroma, dacComp;
    uint32_t        dacComp1;
    BVDC_Display_Handle hDisplay;

    hDisplay = GetDisplay(theCompositor);
#if (BCHP_CHIP == 7401)
    GetDacWiring(&dacBluePb, &dacGreenY, &dacRedPr, &dacLuma, &dacChroma, &dacComp);
#endif
#if (BCHP_CHIP == 7400)
    GetDacWiring(&dacBluePb, &dacGreenY, &dacRedPr, &dacLuma, &dacChroma, &dacComp, &dacComp1);
#endif

	/* Set Format */
	if (status == BERR_SUCCESS)
		status = BVDC_Display_SetVideoFormat(hDisplay, theFormat);

	/* Configure DACs */
	if (theFormat == BFMT_VideoFmt_eNTSC  || theFormat == BFMT_VideoFmt_ePAL_G ||
	    theFormat == BFMT_VideoFmt_ePAL_H || theFormat == BFMT_VideoFmt_ePAL_K || theFormat == BFMT_VideoFmt_ePAL_I)
	{
#if 0
		if (status == BERR_SUCCESS)
			status = BVDC_Display_SetDacConfiguration(hDisplay, dacRedPr,   BVDC_DacOutput_ePr);

		if (status == BERR_SUCCESS)
			status = BVDC_Display_SetDacConfiguration(hDisplay, dacGreenY,  BVDC_DacOutput_eY);

		if (status == BERR_SUCCESS)
			status = BVDC_Display_SetDacConfiguration(hDisplay, dacBluePb,  BVDC_DacOutput_ePb);

		if (status == BERR_SUCCESS)
			status = BVDC_Display_SetDacConfiguration(hDisplay, dacLuma,    BVDC_DacOutput_eSVideo_Luma);

		if (status == BERR_SUCCESS)
			status = BVDC_Display_SetDacConfiguration(hDisplay, dacChroma,  BVDC_DacOutput_eSVideo_Chroma);
#endif
        if (theCompositor == BVDC_CompositorId_eCompositor0) {
            /* 1st decoder */
            if (status == BERR_SUCCESS)
                status = BVDC_Display_SetDacConfiguration(hDisplay, dacComp,    BVDC_DacOutput_eComposite);

        } else {
            /* 2nd decoder */
            if (status == BERR_SUCCESS)
                status = BVDC_Display_SetDacConfiguration(hDisplay, dacComp1,    BVDC_DacOutput_eComposite);

            if (status == BERR_SUCCESS)
                status = BVDC_Display_SetDacConfiguration(hDisplay, dacLuma,    BVDC_DacOutput_eSVideo_Luma);

            if (status == BERR_SUCCESS)
                status = BVDC_Display_SetDacConfiguration(hDisplay, dacChroma,    BVDC_DacOutput_eSVideo_Chroma);
        }

	}
	else if (theFormat == BFMT_VideoFmt_e1080i)
	{
		if (status == BERR_SUCCESS)
			status = BVDC_Display_SetDacConfiguration(hDisplay, dacRedPr,   BVDC_DacOutput_ePr);

		if (status == BERR_SUCCESS)
			status = BVDC_Display_SetDacConfiguration(hDisplay, dacGreenY,  BVDC_DacOutput_eY);

		if (status == BERR_SUCCESS)
			status = BVDC_Display_SetDacConfiguration(hDisplay, dacBluePb,  BVDC_DacOutput_ePb);

		if (status == BERR_SUCCESS)
#if (BCHP_CHIP == 7401)
			status = BVDC_Display_SetDacConfiguration(hDisplay, dacLuma | dacChroma | dacComp,    BVDC_DacOutput_eUnused);
#endif
#if (BCHP_CHIP == 7400)
			status = BVDC_Display_SetDacConfiguration(hDisplay, dacLuma | dacChroma | dacComp | dacComp1,    BVDC_DacOutput_eUnused);
#endif
	}
	else
	{
		if (status == BERR_SUCCESS)
			status = BVDC_Display_SetDacConfiguration(hDisplay, dacRedPr,   BVDC_DacOutput_ePr);

		if (status == BERR_SUCCESS)
			status = BVDC_Display_SetDacConfiguration(hDisplay, dacGreenY,  BVDC_DacOutput_eY);

		if (status == BERR_SUCCESS)
			status = BVDC_Display_SetDacConfiguration(hDisplay, dacBluePb,  BVDC_DacOutput_ePb);

		if (status == BERR_SUCCESS)
			status = BVDC_Display_SetDacConfiguration(hDisplay, dacLuma | dacChroma | dacComp,    BVDC_DacOutput_eUnused);
	}

	/* Make it so */
	if (status == BERR_SUCCESS && applyNow)
		status = BVDC_ApplyChanges(GetBVDC());

	BERR_TRACE(status);

	return status;
}

void SetUnscaledPlacement(WndPlacement_t* placement, uint32_t width, uint32_t height)
{
	BDBG_ASSERT(placement);

	placement->clipLeft		= 0;
	placement->clipTop		= 0;
	placement->clipRight	= 0;
	placement->clipBottom	= 0;

	placement->scalerLeft	= 0;
	placement->scalerTop	= 0;
	placement->scalerWidth	= width;
	placement->scalerHeight = height;

	placement->destLeft		= 0;
	placement->destTop		= 0;
	placement->destWidth	= width;
	placement->destHeight	= height;
}

BERR_Code MakeMpegSrc(BVDC_CompositorId theCompositor, uint32_t id, uint32_t mvdChan, BVDC_Source_Handle* phSrc)
{
	BERR_Code				status = BERR_SUCCESS;
	BAVC_SourceId			sourceId;

	BDBG_ASSERT(phSrc && id < MAX_APP_SRCWND);


	sourceId = (mvdChan == 0) ? BAVC_SourceId_eMpeg0 : BAVC_SourceId_eMpeg1;

	/* Make an Mpeg source and window */
	if (status == BERR_SUCCESS)
        if (!sVideoData.hMpegSrcList[id])
        {
#if BCHP_CHIP == 7401
            status = BVDC_Source_Create(GetBVDC(), &sVideoData.hMpegSrcList[id], sourceId);
#endif
#if BCHP_CHIP == 7400
            status = BVDC_Source_Create(GetBVDC(), &sVideoData.hMpegSrcList[id], sourceId, NULL);
#endif
        }

	if (status == BERR_SUCCESS)
		*phSrc = sVideoData.hMpegSrcList[id];

	BERR_TRACE(status);
	return status;
}

BERR_Code
OpenVideoChan(uint32_t channelNum, BVDC_Source_Handle src, bool bInstallPTSCallbacks)
{

	BERR_Code					status = BERR_SUCCESS;

    if (status == BERR_SUCCESS)
        status = OpenXvChan(channelNum, src, 0);

    return status;
}

BERR_Code MakeMpegWnd(BVDC_CompositorId theCompositor, uint32_t id, BVDC_Source_Handle src, BVDC_Window_Handle *phWnd)
{
	BERR_Code				status = BERR_SUCCESS;
	BVDC_Compositor_Handle	hComp;

	BDBG_ASSERT(phWnd && (id < MAX_APP_SRCWND));

	BDBG_ASSERT(src);

	hComp = GetCompositor(theCompositor);

	/* Make a MPEG window */
    if (!sVideoData.hMpegWndList[id])
    {
        if (status == BERR_SUCCESS)
#if (BCHP_CHIP == 7401)
            status = BVDC_Window_Create(hComp, &sVideoData.hMpegWndList[id], src);
#endif

#if (BCHP_CHIP == 7400)
            status = BVDC_Window_Create(hComp, &sVideoData.hMpegWndList[id], BVDC_WindowId_eVideo0+id, src, NULL);

#endif


        if (status == BERR_SUCCESS)
            status = BVDC_Window_SetVisibility(sVideoData.hMpegWndList[id], true);

#if (BCHP_CHIP == 7401)
        if (status == BERR_SUCCESS)
            status = BVDC_Window_SetDeinterlace(sVideoData.hMpegWndList[id], false);
#endif

#if (BCHP_CHIP == 7400)
        if (status == BERR_SUCCESS) {
            BVDC_Window_SetZOrder(sVideoData.hMpegWndList[id], id);
        }

        status = BVDC_Window_SetDeinterlaceConfiguration(sVideoData.hMpegWndList[id], false, 0);
#endif

    }

	if (status == BERR_SUCCESS)
	{
		*phWnd = sVideoData.hMpegWndList[id];
	}

	BERR_TRACE(status);
	return status;
}

BERR_Code PlaceWindow(BVDC_Window_Handle hWnd, WndPlacement_t* placement, bool applyNow)
{
	BERR_Code status = BERR_SUCCESS;

	if (status == BERR_SUCCESS)
		status = BVDC_Window_SetSrcClip(hWnd, placement->clipLeft, placement->clipRight, placement->clipTop, placement->clipBottom);

	if (status == BERR_SUCCESS)
		status = BVDC_Window_SetScalerOutput(hWnd, placement->scalerLeft, placement->scalerTop, placement->scalerWidth, placement->scalerHeight);

	if (status == BERR_SUCCESS)
		status = BVDC_Window_SetDstRect(hWnd, placement->destLeft, placement->destTop, placement->destWidth, placement->destHeight);

	if (status == BERR_SUCCESS)
		status = BVDC_Window_SetAspectRatioMode(hWnd, BVDC_AspectRatioMode_eBypass);

	if (status == BERR_SUCCESS && applyNow)
		status = BVDC_ApplyChanges(GetBVDC());

	BERR_TRACE(status);
	return status;
}

void InitVideoData(void)
{
	memset(&sVideoData, 0, sizeof(sVideoData));
}

#if BCHP_CHIP == 7401
BXVD_ChannelHandle GetBXVD_Channel( uint32_t id )
{
    BVDC_Source_Handle bvdc_source_handle ;
    BXVD_ChannelHandle bxvd_channel_handle ;

    MakeMpegSrc( BVDC_CompositorId_eCompositor0, id, 0, &bvdc_source_handle ) ;
    OpenXvChan( id, bvdc_source_handle, &bxvd_channel_handle ) ;

    return bxvd_channel_handle ;
}
#endif

#if BCHP_CHIP == 7400
BXVD_ChannelHandle GetBXVD_Channel( uint32_t id )
{
    BVDC_Source_Handle bvdc_source_handle ;
    BXVD_ChannelHandle bxvd_channel_handle ;

    /* Use channel number as XVD index */

    MakeMpegSrc( id, id, id, &bvdc_source_handle ) ;
    OpenXvChan( id, bvdc_source_handle, &bxvd_channel_handle ) ;

    return bxvd_channel_handle ;
}
#endif

BVDC_Compositor_Handle GetCompositor(BVDC_CompositorId theCompositor)
{

    if (!sVideoData.hCompositor[theCompositor])
    {

#if BCHP_CHIP == 7401
	if (BERR_SUCCESS == BVDC_Compositor_Create(GetBVDC(), &sVideoData.hCompositor[theCompositor], theCompositor)) {
#endif
#if BCHP_CHIP == 7400
    	if (BERR_SUCCESS == BVDC_Compositor_Create(GetBVDC(), &sVideoData.hCompositor[theCompositor], BVDC_CompositorId_eCompositor0+theCompositor, NULL))        {
#endif
            /* set background color */
            BVDC_Compositor_SetBackgroundColor(sVideoData.hCompositor[theCompositor], 0xFF, 0xFF, 0xFF);
        }
    }

    return sVideoData.hCompositor[theCompositor];
}

BVDC_Display_Handle GetDisplay(BVDC_CompositorId theCompositor)
{

    if (!sVideoData.hDisplay[theCompositor])
    {
        BVDC_Compositor_Handle compositor;

        compositor = GetCompositor(theCompositor);

        if (compositor)
        {
#if BCHP_CHIP == 7401
            BVDC_Display_Create(compositor, &sVideoData.hDisplay[theCompositor]);
#endif
#if BCHP_CHIP == 7400
            BVDC_Display_Create(compositor, &sVideoData.hDisplay[theCompositor], BVDC_DisplayId_eDisplay1, NULL);
#endif
        }
    }

    return sVideoData.hDisplay[theCompositor];
}

BERR_Code OpenXvChan(uint32_t channelNum, BVDC_Source_Handle src, BXVD_ChannelHandle* phChannel)
{
	BERR_Code					status = BERR_SUCCESS;
	BXVD_ChannelSettings		settings;
	BINT_Id						intId;

    BDBG_ASSERT(src);

    if (!sVideoData.asDec[channelNum].dec)
    {
        if (status == BERR_SUCCESS)
            status = BXVD_GetChannelDefaultSettings(GetBXVD(channelNum), 0, &settings);

        settings.ui32MonitorRefreshRate = 60;
        if (status == BERR_SUCCESS)
            status = BXVD_OpenChannel(GetBXVD(channelNum), &sVideoData.asDec[channelNum].dec, 0, &settings);

        /* get interrupt name for top field (to drive MVD) */
        if (status == BERR_SUCCESS)
            status = BVDC_Source_GetInterruptName(src, BAVC_Polarity_eTopField, &intId);

        /* pass interrupt names to MVD. They need to handle these interrupts and (eventually) call the VDC callback when they fire */
        if (status == BERR_SUCCESS)
            status = BXVD_RegisterVdcInterrupt(sVideoData.asDec[channelNum].dec, intId, BAVC_Polarity_eTopField);

        /* get interrupt name for bottom field (to drive MVD) */
        if (status == BERR_SUCCESS)
            status = BVDC_Source_GetInterruptName(src, BAVC_Polarity_eBotField, &intId);

        if (status == BERR_SUCCESS)
            status = BXVD_RegisterVdcInterrupt(sVideoData.asDec[channelNum].dec, intId, BAVC_Polarity_eBotField);

        /* get interrupt name for progressive frame (to drive MVD) */
        if (status == BERR_SUCCESS)
            status = BVDC_Source_GetInterruptName(src, BAVC_Polarity_eFrame, &intId);

        if (status == BERR_SUCCESS)
            status = BXVD_RegisterVdcInterrupt(sVideoData.asDec[channelNum].dec, intId, BAVC_Polarity_eFrame);

        if (status == BERR_SUCCESS)
            status = BXVD_InstallInterruptCallback(sVideoData.asDec[channelNum].dec, BXVD_Interrupt_ePictureDataReady, PictureReady_isr,
                                                    (void *)src, 0);
    }

    if (status == BERR_SUCCESS)
    {
        if (phChannel)
        {
            *phChannel = sVideoData.asDec[channelNum].dec;
        }
    }

	BERR_TRACE(status);
	return status;
}
static double
fFrameRateFromCode(BAVC_FrameRateCode eFRC)
{
    switch (eFRC) {
    case BAVC_FrameRateCode_eUnknown: return 0;
    case BAVC_FrameRateCode_e23_976: return 23.976;
    case BAVC_FrameRateCode_e24: return 24;
    case BAVC_FrameRateCode_e25: return 25;
    case BAVC_FrameRateCode_e29_97: return 29.97;
    case BAVC_FrameRateCode_e30: return 30;
    case BAVC_FrameRateCode_e50: return 50;
    case BAVC_FrameRateCode_e59_94: return 59.94;
    case BAVC_FrameRateCode_e60: return 60;
    default: return -1;
    }
}

static void PictureReady_isr(void *pvSourceHandle, int iParm2, void *pvFieldData)
{
	BAVC_MVD_Field *pFieldData = (BAVC_MVD_Field *)pvFieldData;

	/* Temporary fixed for 1088 stream. */
	if(pFieldData->ulDisplayVerticalSize > BFMT_1080I_HEIGHT)
	{
		pFieldData->ulDisplayVerticalSize = BFMT_1080I_HEIGHT;
	}

	if(pFieldData->ulSourceVerticalSize > BFMT_1080I_HEIGHT)
	{
		pFieldData->ulSourceVerticalSize = BFMT_1080I_HEIGHT;
	}

    // Hide warning from bvdc_source.c
    pFieldData->eYCbCrType = BAVC_YCbCrType_e4_2_0;

    /* If we have bad MVD field just tell VDC to display fixed color */
	if(!pFieldData->ulDisplayVerticalSize ||
		!pFieldData->ulDisplayHorizontalSize ||
		!pFieldData->ulSourceVerticalSize ||
		!pFieldData->ulSourceHorizontalSize)
	{
//		BKNI_Printf("PictureReady_isr: muting picture\n");
		pFieldData->bMute = true;
	}

    if (!pFieldData->bMute) {
        static uint32_t ulDVS;
        static uint32_t ulDHS;
        static uint32_t ulSVS;
        static uint32_t ulSHS;
        static BAVC_FrameRateCode eFRC;
        if (ulDVS != pFieldData->ulDisplayVerticalSize
            || ulDHS != pFieldData->ulDisplayHorizontalSize
            || ulSVS != pFieldData->ulSourceVerticalSize
            || ulSHS != pFieldData->ulSourceHorizontalSize
            || eFRC != pFieldData->eFrameRateCode) {
            ulDVS = pFieldData->ulDisplayVerticalSize;
            ulDHS = pFieldData->ulDisplayHorizontalSize;
            ulSVS = pFieldData->ulSourceVerticalSize;
            ulSHS = pFieldData->ulSourceHorizontalSize;
            eFRC = pFieldData->eFrameRateCode;
            BDBG_WRN(("PictureReady_isr: valid picture (%d, %d) -> (%d, %d) rate: %g",
                      ulSHS, ulSVS, ulDHS, ulDVS, fFrameRateFromCode(eFRC)));
        }
	}


	if (sVideoData.hMI[0] && MpegIntercept_CapturePending_isr(sVideoData.hMI[0]))
		MpegIntercept_CopyFieldData_isr(sVideoData.hMI[0], pFieldData);


	BVDC_Source_MpegDataReady_isr(pvSourceHandle, iParm2, pvFieldData);
}

/*---------------------------------------------------------------------------------*/
bool MpegIntercept_CapturePending_isr(
				MpegIntercept_Handle	hMI
				)
{
	BDBG_ASSERT(hMI);

	return hMI->bCapturePending;
}



/*---------------------------------------------------------------------------------*/
void MpegIntercept_CopyFieldData_isr(
				MpegIntercept_Handle	hMI,
				BAVC_MVD_Field*         pFieldData
				)
{
	uint32_t row;
	uint32_t minHeight;
	uint32_t minStride;

	if (pFieldData->ulSourceHorizontalSize == hMI->stFrameBuffer.uiRequestedWidth && pFieldData->ulRowStride)
	{
		/* Get minimum of the pitches of our buffer and the decoder buffer */
		minStride = (pFieldData->ulRowStride < hMI->stFrameBuffer.uiLumaRowStride) ? pFieldData->ulRowStride : hMI->stFrameBuffer.uiLumaRowStride;

		/* Get minimum of the heights of our buffer and  the decoder buffer */
		minHeight = (pFieldData->ulSourceVerticalSize < hMI->stFrameBuffer.uiRequestedHeight) ? pFieldData->ulSourceVerticalSize : hMI->stFrameBuffer.uiRequestedHeight;

		/* Do macroblock row-by-row copy (to match up row starts) */
		for (row = 0; row < minHeight/32; row++)
		{
			BKNI_Memcpy(
				(char*)hMI->stFrameBuffer.pucChromaAddr + row * hMI->stFrameBuffer.uiChromaRowStride,
				(char*)pFieldData->pChrominanceFrameBufferAddress + row * pFieldData->ulRowStride,
				minStride);
		}

		for (row = 0; row < minHeight/16; row++)
		{
			BKNI_Memcpy(
				(char*)hMI->stFrameBuffer.pucLumaAddr + row * hMI->stFrameBuffer.uiLumaRowStride,
				(char*)pFieldData->pLuminanceFrameBufferAddress + row * pFieldData->ulRowStride,
				minStride);
		}


		hMI->captureErr = BERR_SUCCESS;
	}
	else
	{
		/* Size did not match allocated frame buffer */
		hMI->captureErr = BERR_INVALID_PARAMETER;
	}

	/* Clear the flag */
	hMI->bCapturePending = false;
}
