/***************************************************************************
*     Copyright (c) 2004-2010, Broadcom Corporation
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
* Module Description:
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "brap.h"
#include "brap_priv.h"
#include "brap_dsp.h"
#include "brap_dsp_priv.h"
#include "brap_dsp_mit.h"
#include "brap_img.h"

BDBG_MODULE(rap_dsp);		/* Register software module with debug interface */
/*******************************************************
 * Local defines
 *******************************************************/
 
#define BRAP_DSP_P_PREBOOT_MAILBOX_PATTERN			0xA5A5
#define BRAP_DSP_P_BOOT_SUCCESS_PATTERN				0xBB000001
#define BRAP_DSP_P_BOOT_TIMEOUT_COUNT				50
#define BRAP_DSP_P_DMA_BUSY							1
#define BRAP_DSP_P_DMA_TIMEOUT_COUNT				500
#define BAF_AC3_DOWNMIX_TABLE						0 /* Should be in baud_dsphostcommon.h */
#define BRAP_DSP_P_FMM_BLOCKOUT_COUNT				250 
#define BRAP_DSP_P_DSP_BLOCKOUT_COUNT				37
#define BRAP_DSP_P_SCB_LATENCY_LIMIT				1800
#ifndef BCHP_7411_VER
#define BRAP_DSP_P_CHIP_PHYSICAL_OFFSET				BCHP_PHYSICAL_OFFSET
#else
#define BRAP_DSP_P_CHIP_PHYSICAL_OFFSET				BRAP_PHYSICAL_OFFSET
#endif

#define BRAP_DSP_P_DEF_LOW_MASK			(0x0000FFFF) /* Lower 2 byte mask */
#define BRAP_DSP_P_DEF_HIGH_MASK			(0xFFFF0000) /* Higher 2 byte mask */
#define BRAP_DSP_P_DEF_CUST_ID_MASK		(0x0000FF00) /* Mask Value to Customer ID */
#define BRAP_DSP_P_DEF_MAJOR_VER_MASK	(0x000000FF) /* Mask Value to  Major version */
#define BRAP_DSP_P_DEFAULT_WATCHDOG_COUNT	(0x3FFFFFFF)

#define BRAP_DSP_P_FIRMWARE_VERSION_SIZE			16
#define BRAP_DSP_P_AC3_INTERFRAME_BUF_SIZE			BRAP_P_ALIGN(3374, 2)
#define BRAP_DSP_P_MPEG_INTERFRAME_BUF_SIZE			BRAP_P_ALIGN(11172, 2)
#define BRAP_DSP_P_BDLPCM_INTERFRAME_BUF_SIZE			BRAP_P_ALIGN(964, 2)
#define BRAP_DSP_P_HDDVDLPCM_INTERFRAME_BUF_SIZE		BRAP_P_ALIGN(968, 2)
#define BRAP_DSP_P_DVDLPCM_INTERFRAME_BUF_SIZE			BRAP_P_ALIGN(964, 2)
#define BRAP_DSP_P_MLP_INTERFRAME_BUF_SIZE				BRAP_P_ALIGN(1024, 2)
#define BRAP_DSP_P_SRC_INTERFRAME_BUF_SIZE				BRAP_P_ALIGN(4352, 2) /* 136*8*4 */
#define BRAP_DSP_P_WMAPRO_INTERFRAME_BUF_SIZE				BRAP_P_ALIGN(((6144*4) + 32), 2)


/* MP3 encoder debug */
/*#define MP3ENC_STANDALONE*/ /* Need to enable this now in local view as scheduler is not merged */

/*******************************************************
 * Local Type definitions
 *******************************************************/

/*******************************************************
 * Static Variables & Functions
 *******************************************************/
static void BRAP_DSP_P_InitializeConfigBuffArray(BRAP_DSP_Handle hDsp);
static void BRAP_DSP_P_InitializeArrays(BRAP_DSP_Handle hDsp, sMITArrays *psMitArrays);
static void BRAP_DSP_P_CreateMIT(BRAP_DSP_Handle hDsp);
static BERR_Code BRAP_DSP_P_AllocMem(BRAP_DSP_Handle hDsp);
static void	BRAP_DSP_P_InitCtxUsageInfo(BRAP_DSP_Handle hDsp);
static void BRAP_DSP_P_FreeMem(BRAP_DSP_Handle hDsp);
static uint32_t BRAP_DSP_P_ResetHardware(BRAP_DSP_Handle hDsp);
static BERR_Code BRAP_DSP_P_CopyFWToMem(BRAP_DSP_Handle hDsp);
BERR_Code BRAP_DSP_P_CopyFWImageToMem (BRAP_DSP_Handle hDsp, uint32_t memAdr, uint32_t firmware_id);
BERR_Code BRAP_DSP_P_GetFWSize (BRAP_DSP_Handle hDsp, uint32_t firmware_id, uint32_t *size);

#ifndef BCHP_7411_VER
static BERR_Code BRAP_DSP_P_DownloadAllSupportedFirmware(BRAP_DSP_Handle hDsp);
#endif

#if BRAP_DSP_P_DBG_MIT_TABLE==1 
static void BRAP_DSP_P_PrintMit(	BRAP_DSP_Handle hDsp);
#endif

static BERR_Code BRAP_DSP_P_DmaIn(	
					BRAP_DSP_Handle hDsp,
					uint8_t dataType,
					uint8_t swapType,
					uint32_t size,
					uint32_t src_addr,
					uint32_t dst_addr);
static void BRAP_DSP_P_WriteMIT(
				unsigned char *mitPt,
				uint32_t data,
				uint32_t nBytes);

static void BRAP_DSP_P_FsUpdateDRAMEntry(
		BRAP_DSP_Handle hDsp,
		uint32_t dramAddr,
		uint32_t id,
		uint32_t size,
		uint32_t addr);

static void BRAP_DSP_P_AlgTblUpdateDRAMEntry(
		BRAP_DSP_Handle hDsp,
		uint32_t dramAddr,
		uint32_t algId,
		uint32_t algSize,
		uint32_t algAddr,
		uint32_t tblSize,
		uint32_t tblAddr,
#if (BRAP_DSP_P_7401_NEWMIT==1)
		uint32_t decScratchBufSize,
		bool    bAlg   /* true=>decoder, false=> pp */
#else
		uint32_t interstageBufSize,
		uint32_t interstageIfBufSize,
		uint32_t decScratchBufSize
#endif		
);

/* Updating FW download functions & MIT functions for Encoder */
#ifndef BCHP_7411_VER /* For chips other than 7411 with encoder support */

static void BRAP_DSP_P_CopyEncFWToMem(BRAP_DSP_Handle hDsp, sEncMITArrays *psMitArrays); 
static void BRAP_DSP_P_EncInitializeArrays(BRAP_DSP_Handle hDsp, sEncMITArrays *psMitArrays);
static BERR_Code BRAP_DSP_P_EncAllocMem(BRAP_DSP_Handle hDsp,sEncMITArrays *psMitArrays);
static BERR_Code BRAP_DSP_P_GetEncFwSize(BRAP_DSP_Handle hDsp,BRAP_DSP_P_MemInfo *memInfo);
static void BRAP_DSP_P_EncFreeMem(BRAP_DSP_Handle hDsp);
static void BRAP_DSP_P_CreateEncMIT(BRAP_DSP_Handle hDsp, sEncMITArrays *psMitArrays);
#endif

static BRAP_DSP_P_ExecID sExecIdArray[/*Audio Type*/][BRAP_DSP_P_MAX_NUM_DECODE_MODE_SUPPORTED /*Decode Mode*/] = {

	{	{ 	/* Audio type - MPEG, Mode - decode */
			{BAF_FRAME_SYNC_MPEG}, /*fsID (1)*/ 	
			{BAF_ALGORITHM_MPEG_L1, BAF_ALGORITHM_MPEG_L2, BAF_ALGORITHM_MP3,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - MPEG, Mode - pass thru */
			{BAF_FRAME_SYNC_MPEG},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_PASS_THRU_MPEG},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - MPEG, Mode - simul */
			{BAF_FRAME_SYNC_MPEG},/*fsID (1)*/ 	
			{BAF_ALGORITHM_MPEG_L1, BAF_ALGORITHM_MPEG_L2, BAF_ALGORITHM_MP3,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_PASS_THRU_MPEG},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/}},
	{	{
				/* Audio type - AAC-SBR / AAC_HE, Mode - decode */
				{BAF_FRAME_SYNC_AAC_HE},/*fsID (1)*/ 	
			{BAF_ALGORITHM_AACHE_0, BAF_ALGORITHM_AACHE_1, BAF_ALGORITHM_AACHE_2,BAF_ALGORITHM_AACHE_3, BAF_ALGORITHM_AACHE_4},/*decID (3)*/			
				{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
				{BAF_INVALID},/* ptID (1)*/
				BAF_FRAME_SYNC_AAC/* transID (1)*/},
			{
				/* Audio type - AAC-SBR / AAC_HE, Mode - passthru */
				/* No such combination */
				{BAF_FRAME_SYNC_AAC_HE},/*fsID (1)*/ 	
				{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
				{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
				{BAF_PASS_THRU_AAC},/* ptID (1)*/
				BAF_FRAME_SYNC_AAC/* transID (1)*/},
			{
				/* Audio type - AAC-SBR / AAC_HE, Mode - simul */
				/* No such combination */
				{BAF_INVALID},/*fsID (1)*/ 	
				{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
				{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
				{BAF_INVALID},/* ptID (1)*/
				BAF_INVALID/* transID (1)*/}},
	{	{
			/* Audio type - AAC-SBR / AAC_HE, Mode - decode */
			{BAF_FRAME_SYNC_AAC_HE},/*fsID (1)*/ 	
#if (BRAP_7401_FAMILY == 1) || ( BCHP_CHIP == 7400 )
			{BAF_ALGORITHM_AACHE_0, BAF_ALGORITHM_AACHE_1, BAF_ALGORITHM_AACHE_2,BAF_ALGORITHM_AACHE_3, BAF_ALGORITHM_AACHE_4},/*decID (3)*/						
#else
			{BAF_ALGORITHM_AAC_HE, BAF_ALGORITHM_AAC_HE_1, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
#endif
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_INVALID},/* ptID (1)*/
			BAF_FRAME_SYNC_AAC/* transID (1)*/},
		{
			/* Audio type - AAC-SBR / AAC_HE, Mode - passthru */
			/* No such combination */
			{BAF_FRAME_SYNC_AAC_HE},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_PASS_THRU_AAC_HE},/* ptID (1)*/
			BAF_FRAME_SYNC_AAC/* transID (1)*/},
		{
			/* Audio type - AAC-SBR / AAC_HE, Mode - simul */
			/* No such combination */
			{BAF_INVALID},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/}},
#if defined ( BCHP_7411_VER ) || (BRAP_7401_FAMILY == 1)
	{	{
			/* Audio type - AC3, Mode - decode */
			{BAF_FRAME_SYNC_AC3},/*fsID (1)*/ 	
			{BAF_ALGORITHM_AC3, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - AC3, Mode - passthru */
			{BAF_FRAME_SYNC_AC3},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_PASS_THRU_AC3},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - AC3, Mode - simul */
			{BAF_FRAME_SYNC_AC3},/*fsID (1)*/ 	
			{BAF_ALGORITHM_AC3, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_PASS_THRU_AC3},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/}},
#else			
        {	{
			/* Audio type - AC3Plus / DDP, Mode - decode */
			{BAF_FRAME_SYNC_DDP},/*fsID (1)*/ 	
			{BAF_ALGORITHM_DDP, BAF_ALGORITHM_DDP_1, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - AC3Plus / DDP, Mode - passthru */
			{BAF_FRAME_SYNC_DDP},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_PASS_THRU_DDP},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - AC3Plus / DDP, Mode - simul */
			{BAF_FRAME_SYNC_DDP},/*fsID (1)*/ 	
			{BAF_ALGORITHM_DDP, BAF_ALGORITHM_DDP_1, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/ 
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_CONVERT_DDP},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/}},
#endif
	{	{
			/* Audio type - AC3Plus / DDP, Mode - decode */
			{BAF_FRAME_SYNC_DDP},/*fsID (1)*/ 	
			{BAF_ALGORITHM_DDP, BAF_ALGORITHM_DDP_1, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
#if ( BCHP_CHIP == 7400 )				
		{
			/* Audio type - AC3Plus / DDP, Mode - passthru */
			{BAF_FRAME_SYNC_DDP_7_1},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_PASS_THRU_DDP},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - AC3Plus / DDP, Mode - simul */
			{BAF_FRAME_SYNC_DDP},/*fsID (1)*/ 	
			{BAF_ALGORITHM_DDP, BAF_ALGORITHM_DDP_1, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/ 
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_CONVERT_DDP},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
#else
		{
			/* Audio type - AC3Plus / DDP, Mode - passthru */
			{BAF_FRAME_SYNC_DDP},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_PASS_THRU_DDP},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - AC3Plus / DDP, Mode - simul */
			{BAF_FRAME_SYNC_DDP},/*fsID (1)*/ 	
			{BAF_ALGORITHM_DDP, BAF_ALGORITHM_DDP_1, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/ 
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_PASS_THRU_DDP},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
#endif
			},
			
	{	{
			/* Audio type - DTS / DTS, Mode - decode */
			{BAF_FRAME_SYNC_DTS},/*fsID (1)*/ 	
			{BAF_ALGORITHM_DTS, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - DTS / DTS, Mode - passthru */
			{BAF_FRAME_SYNC_DTS},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_PASS_THRU_DTS},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - DTS / DTS, Mode - simul */
			{BAF_FRAME_SYNC_DTS},/*fsID (1)*/ 	
			{BAF_ALGORITHM_DTS,BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/ 
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_PASS_THRU_DTS},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/}},
	{	{
			/* Audio type - BD_LPCM / BD_LPCM, Mode - decode */
			{BAF_FRAME_SYNC_BD_LPCM},/*fsID (1)*/ 	
			{BAF_ALGORITHM_BD_LPCM, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - BD_LPCM / BD_LPCM, Mode - passthru */
			{BAF_FRAME_SYNC_BD_LPCM},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - BD_LPCM / BD_LPCM, Mode - simul */
			{BAF_FRAME_SYNC_BD_LPCM},/*fsID (1)*/ 	
			{BAF_ALGORITHM_BD_LPCM,BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/ 
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/}},
	{	{
			/* Audio type - LPCM_HDDVD / LPCM_HDDVD, Mode - decode */
			{BAF_FRAME_SYNC_HDDVD_LPCM},/*fsID (1)*/ 	
			{BAF_ALGORITHM_LPCM_HDDVD, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - LPCM_HDDVD / LPCM_HDDVD, Mode - passthru */
			{BAF_FRAME_SYNC_HDDVD_LPCM},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - LPCM_HDDVD / LPCM_HDDVD, Mode - simul */
			{BAF_FRAME_SYNC_HDDVD_LPCM},/*fsID (1)*/ 	
			{BAF_ALGORITHM_LPCM_HDDVD,BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/ 
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/}},
	{	{
			/* Audio type - DTSHD, Mode - decode */
			{BAF_FRAME_SYNC_DTSHD},/*fsID (1)*/ 	
			{BAF_ALGORITHM_DTSHD, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_FRAME_SYNC_DTS_HD_HDDVD/* transID (1)*/},
		{
			/* Audio type - DTSHD, Mode - passthru */
			{BAF_FRAME_SYNC_DTSHD},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_PASS_THRU_DTSHD},/* ptID (1)*/
			BAF_FRAME_SYNC_DTS_HD_HDDVD/* transID (1)*/},
		{
			/* Audio type - DTSHD, Mode - simul */
			{BAF_FRAME_SYNC_DTSHD},/*fsID (1)*/ 	
			{BAF_ALGORITHM_DTSHD,BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/ 
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_PASS_THRU_DTSHD},/* ptID (1)*/
			BAF_FRAME_SYNC_DTS_HD_HDDVD/* transID (1)*/}},
	{	{
			/* Audio type - LPCM_DVD / LPCM_DVD, Mode - decode */
			{BAF_FRAME_SYNC_DVD_LPCM},/*fsID (1)*/ 	
			{BAF_ALGORITHM_LPCM_DVD, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - LPCM_DVD / LPCM_DVD, Mode - passthru */
			{BAF_FRAME_SYNC_DVD_LPCM},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - LPCM_DVD / LPCM_DVD, Mode - simul */
			{BAF_FRAME_SYNC_DVD_LPCM},/*fsID (1)*/ 	
			{BAF_ALGORITHM_LPCM_DVD,BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/ 
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/}},
	{	{
			/* Audio type - WMA_STD,  Mode - decode */
			{BAF_FRAME_SYNC_WMA_STD},/*fsID (1)*/ 	
			{BAF_ALGORITHM_WMA_STD, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - WMA_STD, Mode - passthru */
			{BAF_FRAME_SYNC_WMA_STD},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - WMA_STD, Mode - simul */
			{BAF_FRAME_SYNC_WMA_STD},/*fsID (1)*/ 	
			{BAF_ALGORITHM_WMA_STD,BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/ 
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/}},
	{	{
			/* Audio type - AC3 Lossless, Mode - decode */
			{BAF_FRAME_SYNC_DD_LOSSLESS},/*fsID (1)*/ 	
			{BAF_ALGORITHM_DD_LOSSLESS, BAF_ALGORITHM_DD_LOSSLESS_1, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - AC3 Lossless, Mode - passthru */
			{BAF_FRAME_SYNC_DD_LOSSLESS},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_PASS_THRU_DD_LOSSLESS},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - AC3 Lossless, Mode - simul */
			{BAF_FRAME_SYNC_DD_LOSSLESS},/*fsID (1)*/ 	
			{BAF_ALGORITHM_DD_LOSSLESS, BAF_ALGORITHM_DD_LOSSLESS_1, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_PASS_THRU_DD_LOSSLESS},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/}},
	{	{
			/* Audio type - MLP, Mode - decode */
			{BAF_FRAME_SYNC_MLP},/*fsID (1)*/ 	
			{BAF_ALGORITHM_MLP, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_FRAME_SYNC_MLP_HDDVD/* transID (1)*/},
		{
			/* Audio type - MLP, Mode - passthru */
			{BAF_FRAME_SYNC_MLP},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - MLP, Mode - simul */
			{BAF_FRAME_SYNC_MLP},/*fsID (1)*/ 	
			{BAF_ALGORITHM_MLP, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/}},
#if (BRAP_7401_FAMILY == 1) ||( BCHP_CHIP == 7400 )
{	{
			/* Audio type - WMA_PRO,  Mode - decode */
			{BAF_FRAME_SYNC_WMA_PRO},/*fsID (1)*/ 	
			{BAF_ALGORITHM_WMA_PRO_0, BAF_ALGORITHM_WMA_PRO_1, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - WMA_PRO, Mode - passthru */
			{BAF_FRAME_SYNC_WMA_PRO},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_PASS_THRU_WMA_PRO},
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - WMA_PRO, Mode - simul */
			{BAF_FRAME_SYNC_WMA_PRO},/*fsID (1)*/ 	
			{BAF_ALGORITHM_WMA_PRO_0,BAF_ALGORITHM_WMA_PRO_1, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/ 
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_PASS_THRU_WMA_PRO},
			BAF_INVALID/* transID (1)*/}},
#else			
{	{
			/* Audio type - WMA_PRO,  Mode - decode */
			{BAF_INVALID},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - WMA_PRO, Mode - passthru */
			{BAF_INVALID},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - WMA_PRO, Mode - simul */
			{BAF_INVALID},/*fsID (1)*/ 	
			{BAF_INVALID,BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/ 
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_INVALID},
			BAF_INVALID/* transID (1)*/}},
#endif
{	{
			/* Audio type - PCM Wav Mode - decode */
			{BAF_FRAME_SYNC_PCMWAV},/*fsID (1)*/ 	
			{BAF_ALGORITHM_PCMWAV, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - PCM Wav Mode - passthru */
			{BAF_INVALID},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - PCM Wav Mode - simul */
			{BAF_INVALID},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/}},
{	{
			/* Audio type - DRA,  Mode - decode */
			{BAF_FRAME_SYNC_DRA},/*fsID (1)*/ 	
			{BAF_ALGORITHM_DRA, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - DRA, Mode - passthru */
			{BAF_FRAME_SYNC_DRA},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_PASS_THRU_DRA},
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - DRA, Mode - simul */
			{BAF_INVALID},/*fsID (1)*/ 	
			{BAF_INVALID,BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/ 
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_INVALID},
			BAF_INVALID/* transID (1)*/}},
#ifdef RAP_SRSTRUVOL_CERTIFICATION			
{	{
			/* Audio type - PCM Passthru, Mode - decode */
			{BAF_FRAME_PCM_PASSTHRU},/*fsID (1)*/ 	
			{BAF_ALGORITHM_PCM_PASSTHRU, BAF_INVALID, BAF_INVALID,BAF_INVALID,BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - PCM Passthru, Mode - passthru */
			{BAF_FRAME_PCM_PASSTHRU},/*fsID (1)*/ 	
			{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID,BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/},
		{
			/* Audio type - PCM Passthru, Mode - simul */
			{BAF_FRAME_PCM_PASSTHRU},/*fsID (1)*/ 	
			{BAF_ALGORITHM_PCM_PASSTHRU, BAF_INVALID, BAF_INVALID,BAF_INVALID,BAF_INVALID},/*decID (3)*/
			{BAF_INVALID, BAF_INVALID},/* ppID (2)*/ 
			{BAF_INVALID},/* ptID (1)*/
			BAF_INVALID/* transID (1)*/}},
#endif					
};

static BRAP_DSP_P_ExecID sPpExecIdArray[BRAP_DSPCHN_PP_Algo_eMax] = {
	/* BRAP_DSPCHN_PP_Algo_eDdbm */
	{
		{BAF_INVALID}, /*fsID (1)*/
		{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
		{BAF_PP_DDBM, BAF_INVALID},/* ppID (2)*/ 
		{BAF_INVALID}, /* ptID (1)*/
		BAF_INVALID/* transID (1)*/
	},
	/* BRAP_DSPCHN_PP_Algo_eSrc */
	{
		{BAF_INVALID}, /*fsID (1)*/
		{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
		{BAF_PP_SRC, BAF_INVALID},/* ppID (2)*/ 
		{BAF_INVALID}, /* ptID (1)*/
		BAF_INVALID/* transID (1)*/
	},
	/* BRAP_DSPCHN_PP_Algo_eAacDownmix */
	{
		{BAF_INVALID}, /*fsID (1)*/
		{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
		{BAF_PP_DOWNMIX_AAC, BAF_INVALID},/* ppID (2)*/ 
		{BAF_INVALID}, /* ptID (1)*/
		BAF_INVALID/* transID (1)*/
	},
	/* BRAP_DSPCHN_PP_Algo_eDsola */
	{
		{BAF_INVALID}, /*fsID (1)*/
		{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
		{BAF_PP_DSOLA, BAF_INVALID},/* ppID (2)*/
		{BAF_INVALID}, /* ptID (1)*/
		BAF_INVALID/* transID (1)*/
	}
#if (BRAP_AD_SUPPORTED == 1)
	/* BRAP_DSPCHN_PP_Algo_eAD_FadeCtrl */
	,{
		{BAF_INVALID}, /*fsID (1)*/
		{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
		{BAF_PP_AD_FADECTRL, BAF_INVALID},/* ppID (2)*/
		{BAF_INVALID}, /* ptID (1)*/
		BAF_INVALID/* transID (1)*/
	},
	/* BRAP_DSPCHN_PP_Algo_eAD_PanCtrl */
	{
		{BAF_INVALID}, /*fsID (1)*/
		{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
		{BAF_PP_AD_PANCTRL, BAF_INVALID},/* ppID (2)*/
		{BAF_INVALID}, /* ptID (1)*/
		BAF_INVALID/* transID (1)*/
	}	
#endif  
#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)
	/* BRAP_DSPCHN_PP_Algo_eDolbyVolume */
	,{
		{BAF_INVALID}, /*fsID (1)*/
		{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
		{BAF_PP_DOBLYVOLUME, BAF_INVALID},/* ppID (2)*/ 
		{BAF_INVALID}, /* ptID (1)*/
		BAF_INVALID/* transID (1)*/
	}
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)
	/* BRAP_DSPCHN_PP_Algo_eSRS_TruVol */
	,{
		{BAF_INVALID}, /*fsID (1)*/
		{BAF_INVALID, BAF_INVALID, BAF_INVALID,BAF_INVALID, BAF_INVALID},/*decID (3)*/
		{BAF_PP_SRSTRUVOL, BAF_INVALID},/* ppID (2)*/ 
		{BAF_INVALID}, /* ptID (1)*/
		BAF_INVALID/* transID (1)*/
	}
#endif
};

/***************************************************************************
Summary:
	Gets DSP default channel setting

Description:
	The default DSP channel configuration settings are returned
	
Returns:
    BERR_SUCCESS                - if successful 

See Also:
	BRAP_DSP_P_Open
**************************************************************************/
BERR_Code 
BRAP_DSP_P_GetDefaultSettings( 
   BRAP_DSP_P_Settings	*psDefSettings	/* [out] The DEC channel default configuration setting */
   )
{
	BSTD_UNUSED(psDefSettings);
	return BERR_SUCCESS;
}

/***************************************************************************
Summary: Opens an BRAP_DSP channel 

Description:
	This function is responsible for opening a DSP channel. 
	When a BRAP_DSP channel is opened, it will create a module channel handle 
	and configure the module based on the channel settings. 
	Once a channel is opened, it must be closed before it can be opened 
	again.

Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSP_P_Close,
	BRAP_DSP_P_GetDefaultSettings
**************************************************************************/
BERR_Code 
BRAP_DSP_P_Open(
	BRAP_DSP_Handle			*phDsp,			/* [out] DSP handle */
	BRAP_Handle				hRap,			/* [in] AUD handle */
	uint32_t				dspIndex,		/* [in] Channel index starting from 0 to max number of channels */ 
	const BRAP_DSP_P_Settings	*pDefSettings	/* [in] The DSP channel configuration setting */
	)
{
	BRAP_DSP_P_Device *handle;
	sMITArrays *psMITArrays;
#if (BRAP_AD_SUPPORTED == 1)
	uint32_t    ui32PanFadeInterfaceAddr=0x0; /* Dram location for interface with fw */
#endif

#ifndef BCHP_7411_VER /* For chips other than 7411 */
	int regVal=0;
/* Merging Decoder & Encoder */
	sEncMITArrays *psEncMITArrays;
#endif
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BRAP_DSP_Open);
	BSTD_UNUSED(dspIndex);
	BSTD_UNUSED(pDefSettings);
	
	/* Assert the function arguments*/
	BDBG_ASSERT(phDsp);
	BDBG_ASSERT(hRap);

	if (!BRAP_P_GetWatchdogRecoveryFlag(hRap)) { /* External call - Not a watchdog recovery */

		BDBG_ASSERT(pDefSettings);

		/* allocate the AUD handle */
		handle = (BRAP_DSP_P_Device *)BKNI_Malloc( sizeof(BRAP_DSP_P_Device));
		if(handle == NULL)
		{
			rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
			return rc;
		}
		*phDsp = handle;

		/* Reset the entire structure */
		BKNI_Memset( handle, 0, sizeof(BRAP_DSP_P_Device) );

		/* init the data structure  */
		handle->hRap = hRap;
		handle->hChip = hRap->hChip;
		handle->hRegister = hRap->hRegister;
		handle->hHeap = hRap->hHeap;
		handle->hInt = hRap->hInt;
		BRAP_DSP_P_InitCtxUsageInfo(handle);
		
/* Merging Decoder & Encoder */
		BDBG_MSG(("Loading Combined Scheduler"));

		psMITArrays = (sMITArrays *) BKNI_Malloc(sizeof(sMITArrays));
		if(psMITArrays == NULL)
		{
			BKNI_Free(handle);
			rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
			return rc;
		}
		
		handle->psMitArrays = psMITArrays;
		BRAP_DSP_P_InitializeArrays(handle, psMITArrays);	

#ifndef BCHP_7411_VER /* For chips other than 7411 */
		psEncMITArrays = ( sEncMITArrays *)BKNI_Malloc (sizeof(sEncMITArrays));
		if(psEncMITArrays == NULL)
		{
			BKNI_Free(handle);
			BKNI_Free(psMITArrays);
			rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
			return rc;
		}
		handle->psEncMitArrays = psEncMITArrays;
		BRAP_DSP_P_EncInitializeArrays(handle, psEncMITArrays);
#endif

		rc = BRAP_DSP_P_AllocMem(handle);
		if (rc!=BERR_SUCCESS) {
			BDBG_ERR(("BRAP_DSP_P_AllocMem::Error returned %x!",rc));
			goto error;
		}

		/* Intitialize the ConfigBuffer Array entry with invalid Value */
		BRAP_DSP_P_InitializeConfigBuffArray(handle);        

#ifndef BCHP_7411_VER /* For chips other than 7411 */
		rc = BRAP_DSP_P_EncAllocMem(handle, psEncMITArrays);
		if (rc!=BERR_SUCCESS) {
			BDBG_ERR(("BRAP_DSP_P_AllocMem::Error returned %x!",rc));
			goto error;

			}
#endif

	}
	else { /* Watchdog recovery */
		handle = *phDsp;

		/* Need to reset the common ppbuffer size used */
#if (BRAP_DSP_P_7401_NEWMIT==1)		
		handle->memInfo.ui32PpFwBufSizeUsed = 0;
#else
		handle->memInfo.commonPpBufSizeUsed = 0;
#endif
		
		psMITArrays = handle->psMitArrays;
#ifndef BCHP_7411_VER /* For chips other than 7411 */
              psEncMITArrays= handle->psEncMitArrays;
#endif


	}
	
	BRAP_DSP_P_CreateMIT(handle);
#ifndef BCHP_7411_VER /* For chips other than 7411 */
	BRAP_DSP_P_CreateEncMIT(handle, psEncMITArrays);
#endif

#ifndef MP3ENC_STANDALONE
	rc = BRAP_DSP_P_CopyFWToMem(handle);
	if (rc!=BERR_SUCCESS) {
		BDBG_ERR(("BRAP_DSP_P_CopyFWToMem::Error returned %x!",rc));
		goto error;

	}
#endif
#ifndef BCHP_7411_VER /* For chips other than 7411 */
	BRAP_DSP_P_CopyEncFWToMem(handle, psEncMITArrays);
#endif

#ifndef BCHP_7411_VER
	if(hRap->sSettings.bFwAuthEnable==true)
	{
		/* Reset DSP processor and its peripherals */
		BRAP_DSP_P_ResetHardware(handle);

		/* Withdraw only RESET0_B reset */
		  regVal = (regVal & ~(BCHP_MASK(AUD_DSP_MISC_SOFT_RESET,RESET0_B)))|
		    (BCHP_FIELD_DATA(AUD_DSP_MISC_SOFT_RESET,RESET0_B,1));
		  BRAP_Write32(handle->hRegister,BCHP_AUD_DSP_MISC_SOFT_RESET, regVal);
		  BDBG_MSG(("\n**** Soft reset: reset0_b withdrawn = 0x%x",regVal));

		  regVal = BRAP_Read32(handle->hRegister,BCHP_AUD_DSP_MISC_SOFT_RESET);
		  regVal = (regVal & ~(BCHP_MASK(AUD_DSP_MISC_SOFT_RESET,RESET_SCBDMA0_B)))|
		   (BCHP_FIELD_DATA(AUD_DSP_MISC_SOFT_RESET,RESET_SCBDMA0_B,1));
		  BRAP_Write32(handle->hRegister,BCHP_AUD_DSP_MISC_SOFT_RESET, regVal);
		  BDBG_MSG(("\n**** Soft reset: SCBDMA withdrawn = 0x%x ****",regVal));

	  
		BDBG_MSG(("Firmware Auth is enabled .. Downloading all supported Firmware"));
		if (!BRAP_P_GetWatchdogRecoveryFlag(hRap)) 
		{
			BRAP_DSP_P_DownloadAllSupportedFirmware(handle);
		}
	}
	else
	{
#endif	
		rc = BRAP_DSP_P_InitHardware(handle);
		if (rc!=BERR_SUCCESS) {
			BDBG_ERR(("BRAP_DSP_P_InitHardware::Error returned %x!",rc));
			goto error;
		}
#ifndef BCHP_7411_VER		
	}
#endif

#if (BRAP_AD_SUPPORTED == 1)
	if (BRAP_P_GetWatchdogRecoveryFlag(hRap))
	{
	    BRAP_P_ConvertAddressToOffset(hRap->hHeap, 
	            (void *)hRap->sAudioDescObjs[0].ui32PanFadeInterfaceAddr, 
	            (void *)&(ui32PanFadeInterfaceAddr));

	    BDBG_ERR(("ui32PanFadeInterfaceAddr in Watchdog = %x",ui32PanFadeInterfaceAddr));

		BRAP_Write32(hRap->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER5_CXT0, ui32PanFadeInterfaceAddr); 
		BRAP_Write32(hRap->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER5_CXT1, ui32PanFadeInterfaceAddr);
	}
#endif


	BDBG_LEAVE(BRAP_DSP_P_Open);
	return rc;

error: 
	BKNI_Free(handle);
	BKNI_Free(psMITArrays);
	BKNI_Free(psEncMITArrays);
	BDBG_ERR(("BRAP_DSP_P_Open failed ::Error returned %x!",rc));
	return BERR_TRACE(rc);	
	
}

/***************************************************************************
Summary: Closes an BRAP_DSP channel

Description:
	This function is responsible for closing a DSP channel and releasing
	all the resources associated with this channel.
	

Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSP_P_Open
**************************************************************************/
BERR_Code 
BRAP_DSP_P_Close( 
	BRAP_DSP_Handle hDsp /* [in] DSP handle */
	)
{
	/* Assert the function arguments*/
	BDBG_ASSERT(hDsp);

	/* Free device memory used by DSP */
	BRAP_DSP_P_FreeMem(hDsp);
#ifndef BCHP_7411_VER /* For chips other than 7411 */
	BRAP_DSP_P_EncFreeMem(hDsp);
#endif
	/* Reset DSP hardware */
	BRAP_DSP_P_ResetHardware(hDsp);

	BKNI_Free(hDsp->psMitArrays);
#ifndef BCHP_7411_VER /* For chips other than 7411 */
	BKNI_Free(hDsp->psEncMitArrays);
#endif
	/* Free DSP handle */
	BKNI_Free(hDsp);	
	return BERR_SUCCESS;
}

/* BRAP_DSP_P_InitCtxUsageInfo: This function initializes the data structures stored
 * in DSP handle. These data structure contain usage information of firmware
 * download buffers for various DSP contexts. This private function is called from
 * BRAP_DSP_P_Open */
static void	BRAP_DSP_P_InitCtxUsageInfo(BRAP_DSP_Handle hDsp)
{
	uint32_t i = 0;
	unsigned int maxNumBuffers = 0;    
	unsigned int uiMaxPpBranches = BRAP_P_MAX_BRANCH_SUPPORTED;
	unsigned int uiMaxPpStgInBranch = BRAP_P_MAX_PP_SUPPORTED;    


	/* Init FS context buf info */
	for(i=0; i<BRAP_DSP_P_MAX_NUM_FS_CTXT_IN_FS_EXE_BUF ; i++)
	{
		hDsp->ctxtFsBufUsage[i].eAlgoType = BRAP_DSPCHN_AudioType_eInvalid;
		hDsp->ctxtFsBufUsage[i].numUser = 0;
	}
	
	/* Init FW context buf info */
	for(i=0; i<BRAP_DSP_P_MAX_NUM_FW_CTXT_IN_FW_EXE_BUF; i++)
	{
		hDsp->ctxtFwBufUsage[i].eAlgoType = BRAP_DSPCHN_AudioType_eInvalid;
		hDsp->ctxtFwBufUsage[i].eDecodeMode = BRAP_DSPCHN_DecodeMode_eInvalid;
		hDsp->ctxtFwBufUsage[i].numUser = 0;
	}

	/* Init Interframe buffer Free/Busy status */
	for (i = 0; i < BRAP_RM_P_MAX_DECODE_SUPPORTED; i++) {
		hDsp->bInterframeBufFreeStatus[i] = true;
	}
    
	maxNumBuffers = BAF_MAX_CONTEXTS * uiMaxPpBranches * uiMaxPpStgInBranch
		+ BAF_MAX_CONTEXTS; 
	
	for( i = 0; i < maxNumBuffers; i++ )
	{
		hDsp->bDecConfigBufFreeStatus[i] = true;
	}
	
	for( ; i < BAF_MAX_CONTEXTS * BAF_MAX_BRANCHES * BAF_MAX_POSTPROCESS_STAGES + BAF_MAX_CONTEXTS; i++ )
	{
		hDsp->bDecConfigBufFreeStatus[i] = false;
	}

	/* Init array of pointers Free/Busy status */
	for( i = 0; i < BAF_MAX_CONTEXTS; i++ )
	{
		hDsp->bConfigBufArrayFreeStatus[i] = true;
	}    

#if (BRAP_DSP_P_7401_NEWMIT==1)
	/* Init Post Processing interframe buffer Free/Busy status */
	maxNumBuffers = BAF_MAX_CONTEXTS * uiMaxPpBranches * uiMaxPpStgInBranch;

	for( i = 0; i < maxNumBuffers; i++ )
	{
		hDsp->bPpInterframeBufFreeStatus[i] = true;
	}
	
	/* Set all the remaining interstage buffers to Busy */
	for(; i < BAF_MAX_CONTEXTS * BRAP_P_MAX_PP_SUPPORTED * BRAP_P_MAX_BRANCH_SUPPORTED; i++ )
	{
		hDsp->bPpInterframeBufFreeStatus[i] = false;
	}
	
	/* Init Interstage and Interstage Interface buffers Free/Busy status */
	/* Set only those number of buffers Free which are required by application */
	maxNumBuffers = BAF_MAX_CONTEXTS * (1 + 2 * uiMaxPpBranches);
    
	for( i = 0; i < maxNumBuffers; i++ )
	{
		hDsp->bInterstageBufFreeStatus[i] = true;
	}
    
	/* Set all the remaining interstage buffers to Busy */
	for(; i < BAF_MAX_CONTEXTS * (1 + 2 * BRAP_P_MAX_BRANCH_SUPPORTED); i++ )
	{
		hDsp->bInterstageBufFreeStatus[i] = false;
	}	
#endif


	return;
}

/* BRAP_DSP_P_AllocMem: This function allocates memory for DSP module. Various buffers
 * allocated in this function are as below.
 *
 * If bFwAuthEnable is False then sequence is as follows:-
 *
 * 1. FW download buffers for resident code - iboot, scheduler_code, scheduler_data
 * 2. APM scratch buffer
 * 3. Scratch space buffer - While programming MIT this is splitted into interstage buffer,
 * interstage interface buffer and decoder scratch buffer
 * 4. Interframe buffers - One per DSP context
 * 5. FW download buffers for decode executables - Number of these buffers is equal to
 * number of simultaneous decode operations supported in chip.
 * 6. FW download buffers for passthru executables - Number of these buffers is equal to
 * number of simultaneous passthru operations supported in chip.
 * 7. FW download buffers for framesync executables - Number of these buffers is equal to
 * number of total simultaneous passthru and decode operations supported in chip.
 * 8. FW download buffer for post processing algorithms that are common across all
 * the audio algorithms.
 *
 * If bFwAuthEnable is True then sequence is as follows:-
 *
 * 1. APM scratch buffer
 * 2. Scratch space buffer - While programming MIT this is splitted into interstage buffer,
 * interstage interface buffer and decoder scratch buffer
 * 3. Interframe buffers - One per DSP context
 * 4. Contiguous FW download buffers for resident code - iboot, scheduler_code,
 * scheduler_data framesync executables,decode executables,passthru executables,
 * post processing algorithms.
 *
 * The FW download buffers are allocated for worst case requirement of the selected audio
 * algorithms.
 */

#if (BRAP_DSP_P_7401_NEWMIT==1)
static BERR_Code BRAP_DSP_P_AllocMem(BRAP_DSP_Handle hDsp)
{
	BERR_Code rc;
	uint32_t  memReq=0, count=0, size=0, numBuf=0, maxSize=0, physAddress=0;
	bool bSelectedAlgos[BRAP_MAX_AUDIO_TYPES];
	BRAP_DEC_P_ChannelMemReq	sDecModeMemReq, sPtModeMemReq;
    BRAP_DSP_P_MemInfo * encMemInfo;

	uint32_t fwSize=0,ibootSize=0,schdCodeSize=0,schdDataSize=0,bufferUsed=0;
	uint32_t baseAddr=0;    

	bool bSelectedPpAlgos[BRAP_MAX_PP_ALGOS];

#if 0
	unsigned int uiMaxPpBranches = hDsp->hRap->sSettings.uiMaxPpBranches;
	unsigned int uiMaxPpStgInBranch = hDsp->hRap->sSettings.uiMaxPpStgInBranch;
#endif
	unsigned int uiMaxPpBranches = BRAP_P_MAX_BRANCH_SUPPORTED;
	unsigned int uiMaxPpStgInBranch = BRAP_P_MAX_PP_SUPPORTED;


	BRAP_DEC_P_ChannelMemReq sPpMemReq;
	BDBG_ENTER (BRAP_DSP_P_AllocMem);  

	encMemInfo=( BRAP_DSP_P_MemInfo *)BKNI_Malloc (sizeof(BRAP_DSP_P_MemInfo));    

	/* Get algorithms selected by application */
	rc = BRAP_P_GetSelectedAlgos(hDsp->hRap, bSelectedAlgos);
	if (BERR_SUCCESS != rc)
	{
		BKNI_Free(encMemInfo);
		return BERR_TRACE(rc);
	}

	/* Get buffer sizes required for combination of selected algorithms */	
	rc = BRAP_P_GetDecChannelBufferSizes(bSelectedAlgos,
			BRAP_DSPCHN_DecodeMode_eDecode,
			&sDecModeMemReq,
			hDsp->hRap->sSettings.bFwAuthEnable);
	if (BERR_SUCCESS != rc)
	{
		BKNI_Free(encMemInfo);
		return BERR_TRACE(rc);
	}
	
	rc = BRAP_P_GetDecChannelBufferSizes(bSelectedAlgos,
			BRAP_DSPCHN_DecodeMode_ePassThru,
			&sPtModeMemReq,
			hDsp->hRap->sSettings.bFwAuthEnable);
	if (BERR_SUCCESS != rc)
	{
		BKNI_Free(encMemInfo);
		return BERR_TRACE(rc);
	}
             
	/* Get post processing algorithms selected by application */
	rc = BRAP_P_GetSelectedPpAlgos(hDsp->hRap, bSelectedPpAlgos);
	if (BERR_SUCCESS != rc)
	{
		BKNI_Free(encMemInfo);
		return BERR_TRACE(rc);
	}

	/* Get buffer sizes required for combination of selected algorithms */
	BRAP_P_GetPpBufferSizes(bSelectedPpAlgos, &sPpMemReq);

	/* Allocate memory for resident code */	
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_IBOOT_ID, &size)))
    	{
    		BKNI_Free(encMemInfo);
    		return BERR_TRACE(rc);
    	}

		if (size)
		{
			hDsp->memInfo.iboot_ptr = (uint32_t)BRAP_P_AllocAligned(hDsp->hRap,
												size,
												2,
												0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
												);
			if (hDsp->memInfo.iboot_ptr==BRAP_P_INVALID_DRAM_ADDRESS) 
				goto alloc_iboot_buf_failed;
			BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.iboot_ptr, &physAddress);				
			BDBG_MSG(("Memory allocation for iboot: Address = 0x%08x, Physical Address=0x%08x, Size = 0x%08x", 
					hDsp->memInfo.iboot_ptr, physAddress, size));
		}
		else
		{
			hDsp->memInfo.iboot_ptr = BRAP_P_INVALID_DRAM_ADDRESS;
		}

        /* Scheduler code */
        size = 0;
		if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_SCHEDULER_CODE_ID, &size)))
    	{
    		BKNI_Free(encMemInfo);
    		return BERR_TRACE(rc);
    	}

		if (size)
		{
			hDsp->memInfo.scheduler_code_ptr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
												size,
												2,
												0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
												);
			if (hDsp->memInfo.scheduler_code_ptr==BRAP_P_INVALID_DRAM_ADDRESS)
				goto alloc_scheduler_code_buf_failed;

            BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.scheduler_code_ptr, &physAddress);        
			BDBG_MSG(("Memory allocation for scheduler code: Address = 0x%08x, Physical Address=0x%08x, Size = 0x%08x",
					hDsp->memInfo.scheduler_code_ptr, physAddress, size));
		}
		else
		{
			hDsp->memInfo.scheduler_code_ptr = BRAP_P_INVALID_DRAM_ADDRESS;
		}

        /* Scheduler data */
        size = 0;
        
		if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_SCHEDULER_DATA_ID, &size)))
    	{
    		BKNI_Free(encMemInfo);
    		return BERR_TRACE(rc);
    	}

		if (size)
		{
			hDsp->memInfo.scheduler_data_ptr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
												size,
												2,
												0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
												
												);
			if (hDsp->memInfo.scheduler_data_ptr==BRAP_P_INVALID_DRAM_ADDRESS)
				goto alloc_scheduler_data_buf_failed;

            BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.scheduler_data_ptr, &physAddress);
			BDBG_MSG(("Memory allocation for scheduler data: Address = 0x%08x, Physical Address=0x%08x, Size = 0x%08x",
					hDsp->memInfo.scheduler_data_ptr, physAddress, size));
		}
		else
		{
			hDsp->memInfo.scheduler_data_ptr = BRAP_P_INVALID_DRAM_ADDRESS;
		}
	
		/* Downloadable Scheduler code */
		size = 0;
	    BDBG_MSG(("Downloadable Scheduler code for DSP0"));
		if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_SCHEDULER_DNLD_CODE_ID, &size)))
		{
		    goto alloc_dnld_scheduler_code_buf_failed;
		}


		if (size)
		{
			hDsp->memInfo.ui32DwnldableSchedulerPtr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
												size,
												2,
												0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif													
												);
			if (hDsp->memInfo.ui32DwnldableSchedulerPtr==BRAP_P_INVALID_DRAM_ADDRESS)
				goto alloc_dnld_scheduler_code_buf_failed;

            BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.ui32DwnldableSchedulerPtr, &physAddress);
			BDBG_MSG(("Memory allocation for scheduler Downloadable code: Address = 0x%08x, Physical Address=0x%08x, Size = 0x%08x",
					hDsp->memInfo.ui32DwnldableSchedulerPtr, physAddress, size));
		}
		else
		{
			hDsp->memInfo.ui32DwnldableSchedulerPtr = BRAP_P_INVALID_DRAM_ADDRESS;
		}

		hDsp->memInfo.ui32DwnldableSchedulerSize = size;
	}


	/* Allocate memory for APM scratch buffer */
	if (BRAP_DSP_P_APM_SCRATCH_BUF_SIZE)
	{
		hDsp->memInfo.apmScratchBufPtr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
												BRAP_DSP_P_APM_SCRATCH_BUF_SIZE,
												2,
												0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
											
												);
		if (hDsp->memInfo.apmScratchBufPtr==BRAP_P_INVALID_DRAM_ADDRESS)
			goto alloc_apm_scratch_buf_failed;

        BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.apmScratchBufPtr, &physAddress);	
        BDBG_MSG(("Memory allocation for APM scratch buffer: Address = 0x%08x, Physical Address=0x%08x, Size = 0x%08x",
				hDsp->memInfo.apmScratchBufPtr, physAddress, BRAP_DSP_P_APM_SCRATCH_BUF_SIZE));
		BRAP_P_DownloadDspFwInMem((BARC_Handle)hDsp->hRegister, NULL, hDsp->memInfo.apmScratchBufPtr, BRAP_DSP_P_APM_SCRATCH_BUF_SIZE);
        
	}
	else
	{
		hDsp->memInfo.apmScratchBufPtr = BRAP_P_INVALID_DRAM_ADDRESS;
	}

	/* Scratch space buffer allocation */
	if (sDecModeMemReq.ui32ScratchBufSize)
	{
		hDsp->memInfo.scratchSpaceBufPtr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
											sDecModeMemReq.ui32ScratchBufSize,
											2,
											0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
											
												);
		if (hDsp->memInfo.scratchSpaceBufPtr==BRAP_P_INVALID_DRAM_ADDRESS)
			goto alloc_scratch_space_buf_failed;
        
		BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.scratchSpaceBufPtr, &physAddress);
        BDBG_MSG(("Memory allocation for scratch space buffer: Address = 0x%08x, Physical Address=0x%08x, Size = 0x%08x",
				hDsp->memInfo.scratchSpaceBufPtr, physAddress, sDecModeMemReq.ui32ScratchBufSize));
	}
	else
	{
		hDsp->memInfo.scratchSpaceBufPtr = BRAP_P_INVALID_DRAM_ADDRESS;
	}
	hDsp->memInfo.scratchSpaceBufSize = sDecModeMemReq.ui32ScratchBufSize;

	/* Scratch space buffer allocation */
	if (sPpMemReq.ui32PpScratchBufSize)
	{
		hDsp->memInfo.ui32PpScratchBufPtr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
											sPpMemReq.ui32PpScratchBufSize,
											2,
											0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif																							
												);											
		if (hDsp->memInfo.ui32PpScratchBufPtr == BRAP_P_INVALID_DRAM_ADDRESS)
			goto alloc_pp_scratch_space_buf_failed;

        BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.ui32PpScratchBufPtr, &physAddress);
		BDBG_MSG(("Memory allocation for PP scratch space buffer: Address = 0x%08x, Physical Address=0x%08x, Size = 0x%08x",
				hDsp->memInfo.ui32PpScratchBufPtr, physAddress, sPpMemReq.ui32PpScratchBufSize));
	}
	else
	{
		hDsp->memInfo.ui32PpScratchBufPtr = BRAP_P_INVALID_DRAM_ADDRESS;
	}

	hDsp->memInfo.ui32PpScratchBufSize = sPpMemReq.ui32PpScratchBufSize;


	/* Interframe buffer allocation - done only for max decode contexts that can exist together */
	if (sDecModeMemReq.ui32InterframeBufSize)
	{
		hDsp->memInfo.interFrameBufBasePtr[0] =  (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
							sDecModeMemReq.ui32InterframeBufSize * BRAP_RM_P_MAX_DECODE_SUPPORTED,
							2,
							0
#if (BRAP_SECURE_HEAP==1) 
							,false
#endif												
												
							);
		if (hDsp->memInfo.interFrameBufBasePtr[0]==BRAP_P_INVALID_DRAM_ADDRESS)
			goto alloc_interframe_buf_failed;

		for (count = 0; count < BRAP_RM_P_MAX_DECODE_SUPPORTED; count++)
		{
			hDsp->memInfo.interFrameBufBasePtr[count] = hDsp->memInfo.interFrameBufBasePtr[0]
				+ count * sDecModeMemReq.ui32InterframeBufSize;
			BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.interFrameBufBasePtr[count], &physAddress);
			BDBG_MSG(("Memory allocation for interframe buffer %d: Address = 0x%08x, Physical Address = 0x%08x, Size = 0x%08x",
				count, hDsp->memInfo.interFrameBufBasePtr[count], physAddress, sDecModeMemReq.ui32InterframeBufSize));
		}
	}
	else
	{
		for (count = 0; count < BRAP_RM_P_MAX_DECODE_SUPPORTED; count++) 
			hDsp->memInfo.interFrameBufBasePtr[count]=BRAP_P_INVALID_DRAM_ADDRESS;
	}
	hDsp->memInfo.interFrameBufSize = sDecModeMemReq.ui32InterframeBufSize;



	/* Post processsing interframe buffer allocation - done only for max pp stages set by application */
	numBuf = BAF_MAX_CONTEXTS * uiMaxPpBranches * uiMaxPpStgInBranch; 
	memReq = sPpMemReq.ui32PpInterframeBufSize * numBuf; 
	if (sPpMemReq.ui32PpInterframeBufSize)
	{
		hDsp->memInfo.ui32PpInterFrameBufBasePtr[0] =  (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
							memReq,
							2,
							0
#if (BRAP_SECURE_HEAP==1) 
							,false
#endif												
												
							);							
		if (hDsp->memInfo.ui32PpInterFrameBufBasePtr[0]==BRAP_P_INVALID_DRAM_ADDRESS)
			goto alloc_pp_interframe_buf_failed;

		for (count = 0; count < numBuf; count++)
		{
			hDsp->memInfo.ui32PpInterFrameBufBasePtr[count] = hDsp->memInfo.ui32PpInterFrameBufBasePtr[0]
				+ count * sPpMemReq.ui32PpInterframeBufSize;
			BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.ui32PpInterFrameBufBasePtr[count], &physAddress);
			BDBG_MSG(("Memory allocation for Pp interframe buffer %d: Address = 0x%08x, Physical Address = 0x%08x, Size = 0x%08x",
				count, hDsp->memInfo.ui32PpInterFrameBufBasePtr[count], physAddress, sPpMemReq.ui32PpInterframeBufSize));
		}

		for ( /* nothing*/; count < BAF_MAX_CONTEXTS * BAF_MAX_BRANCHES * BAF_MAX_POSTPROCESS_STAGES; count++)
		{
			hDsp->memInfo.ui32PpInterFrameBufBasePtr[count] = BRAP_P_INVALID_DRAM_ADDRESS;
		}
	}
	else
	{
		for (count = 0; count < BAF_MAX_CONTEXTS * BAF_MAX_BRANCHES * BAF_MAX_POSTPROCESS_STAGES; count++) 
			hDsp->memInfo.ui32PpInterFrameBufBasePtr[count]=BRAP_P_INVALID_DRAM_ADDRESS;
	}

	hDsp->memInfo.ui32PpInterFrameBufSize = sPpMemReq.ui32PpInterframeBufSize;

	/* Interstage buffer allocation - done together for decode + post process stages */
	numBuf = BAF_MAX_BRANCHES * (1 + 2 * uiMaxPpBranches); 

	if ( sDecModeMemReq.uiInterstageBufSize > sPpMemReq.uiInterstageBufSize )
		maxSize = sDecModeMemReq.uiInterstageBufSize;
	else
		maxSize = sPpMemReq.uiInterstageBufSize;



	if (maxSize)
	{
		hDsp->memInfo.interStageBufBasePtr[0] = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
							maxSize * numBuf,
							2,
							0
#if (BRAP_SECURE_HEAP==1) 
							,false
#endif																								
							);							
		if (hDsp->memInfo.interStageBufBasePtr[0]==BRAP_P_INVALID_DRAM_ADDRESS)
			goto alloc_interstage_buf_failed;

		for (count = 0; count < numBuf; count++)
		{
			hDsp->memInfo.interStageBufBasePtr[count] = hDsp->memInfo.interStageBufBasePtr[0]
				+ count * maxSize;
			BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.interStageBufBasePtr[count], &physAddress);
			BDBG_MSG(("Memory allocation for interstage buffer %d: Address = 0x%08x, Physical Address = 0x%08x, Size = 0x%08x",
				count, hDsp->memInfo.interStageBufBasePtr[count], physAddress, maxSize));
		}

		for ( ; count < BAF_MAX_BRANCHES * (1 + 2 * BAF_MAX_BRANCHES); count++)
		{
			hDsp->memInfo.interStageBufBasePtr[count] = BRAP_P_INVALID_DRAM_ADDRESS;
		}
	}
	else
	{
		for ( count = 0; count < BAF_MAX_BRANCHES * (1 + 2 * BAF_MAX_BRANCHES); count++)
		{
			hDsp->memInfo.interStageBufBasePtr[count] = BRAP_P_INVALID_DRAM_ADDRESS;
		}
	}

	hDsp->memInfo.interStageBufSize = maxSize;

	/* Interstage interface buffer allocation - done together for decode + post process stages */
	numBuf = 1 + 2 * uiMaxPpBranches;
			
	if ( sDecModeMemReq.uiInterstageInterfaceBufSize > sPpMemReq.uiInterstageInterfaceBufSize )
		maxSize = sDecModeMemReq.uiInterstageInterfaceBufSize;
	else
		maxSize = sPpMemReq.uiInterstageInterfaceBufSize;
    
	if (maxSize)
	{
		hDsp->memInfo.interStageInterfaceBufBasePtr[0] = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
							maxSize * numBuf,
							2,
							0
#if (BRAP_SECURE_HEAP==1) 
							,false
#endif																								
							);
		if (hDsp->memInfo.interStageInterfaceBufBasePtr[0]==BRAP_P_INVALID_DRAM_ADDRESS)
			goto alloc_interstage_interface_buf_failed;

		for (count = 0; count < numBuf; count++)
		{
			hDsp->memInfo.interStageInterfaceBufBasePtr[count] = hDsp->memInfo.interStageInterfaceBufBasePtr[0]
				+ count * maxSize;
			BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.interStageInterfaceBufBasePtr[count], &physAddress);
			BDBG_MSG(("Memory allocation for interstage interface buffer %d: Address = 0x%08x, Physical Address = 0x%08x, Size = 0x%08x",
				count, hDsp->memInfo.interStageInterfaceBufBasePtr[count], physAddress, maxSize));
		}

		for ( ; count < (1 + 2 * BAF_MAX_BRANCHES); count++)
		{
			hDsp->memInfo.interStageInterfaceBufBasePtr[count] = BRAP_P_INVALID_DRAM_ADDRESS;
		}
	}
	else
	{
		for ( count = 0; count < (1 + 2 * BAF_MAX_BRANCHES); count++)
		{
			hDsp->memInfo.interStageInterfaceBufBasePtr[count] = BRAP_P_INVALID_DRAM_ADDRESS;
		}
	}

	hDsp->memInfo.interStageInterfaceBufSize = maxSize;
	
	/* Currenlty Decoder and PP config params are combined together. So the max out of this
	is used for buffer alloaction */
	if (sDecModeMemReq.ui32DecConfigParamBufSize < sPpMemReq.ui32PpConfigParamBufSize )
	{
		sDecModeMemReq.ui32DecConfigParamBufSize = 
			sPpMemReq.ui32PpConfigParamBufSize;
	}

	/* A pool of configuration buffer is allocated for the maximum number of PP supported
	and the maximum number of brach supported per context. Configuration parameters buffer
	allocation done  for max DSP contexts that can exist together. Wuse ping pong buffer
	Mechanism so we need to allocated two pool of bufers of size 
	sDecModeMemReq.ui32DecConfigParamBufSize * (BAF_MAX_CONTEXTS *
	hDsp->hRap->sSettings.uiMaxPpBranches * hDsp->hRap->sSettings.uiMaxPpStgInBranch) */

	numBuf = BAF_MAX_CONTEXTS * uiMaxPpBranches * uiMaxPpStgInBranch  /* Number of buffers for PP stages */
		+ BAF_MAX_CONTEXTS; /* Number of buffers for decode stages */
       
	if (sDecModeMemReq.ui32DecConfigParamBufSize)
	{
		hDsp->memInfo.decConfigBufPtr[0] = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
				(sDecModeMemReq.ui32DecConfigParamBufSize * 2 * numBuf),
					2,
					0
#if (BRAP_SECURE_HEAP==1) 
					,false
#endif																								
					);

		if (hDsp->memInfo.decConfigBufPtr[0]==BRAP_P_INVALID_DRAM_ADDRESS)
			goto alloc_decconfig_buf_failed;

		for (count = 0; count < numBuf; count++)
		{
			hDsp->memInfo.decConfigBufPtr[count] = hDsp->memInfo.decConfigBufPtr[0]
				+ count * ( sDecModeMemReq.ui32DecConfigParamBufSize * 2);
			BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.decConfigBufPtr[count] , &physAddress);
			BDBG_MSG(("Memory allocation for dec config buffer %d: Address = 0x%08x, Physical Address = 0x%08x, Size = 0x%08x",
				count, hDsp->memInfo.decConfigBufPtr[count], physAddress, sDecModeMemReq.ui32DecConfigParamBufSize * 2));
		}
		for( ; count < BAF_MAX_CONTEXTS * BAF_MAX_BRANCHES * BAF_MAX_POSTPROCESS_STAGES + BAF_MAX_CONTEXTS; count++)
		{
			hDsp->memInfo.decConfigBufPtr[count]=BRAP_P_INVALID_DRAM_ADDRESS;
		}
	}
	else
	{
		for (count = 0; count < BAF_MAX_CONTEXTS * BAF_MAX_BRANCHES * BAF_MAX_POSTPROCESS_STAGES + BAF_MAX_CONTEXTS; count++)
			hDsp->memInfo.decConfigBufPtr[count]=BRAP_P_INVALID_DRAM_ADDRESS;
	}
	hDsp->memInfo.decConfigBufSize = sDecModeMemReq.ui32DecConfigParamBufSize; 
    
	
	/* Calculate and allocate total memory required by MIT and its sub structures */
	memReq = SIZE_OF_BAF_HOST_INFO_ALLIGNED + \
			 (BAF_HOST_MAX_DL_MODULE * SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED) + \
			 (3 * BAF_HOST_MAX_DL_MODULE * SIZE_OF_BAF_DOWNLOAD_ALLIGNED); 
			 
	memReq = BRAP_P_ALIGN(memReq, 2);
	hDsp->memInfo.totalMITSize = memReq;

	if (memReq)
	{
		hDsp->memInfo.mitDSPtr =  (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
												memReq,
												2,
												0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif																								
												); 
		if (hDsp->memInfo.mitDSPtr==BRAP_P_INVALID_DRAM_ADDRESS)
			goto alloc_mit_buf_failed;

        BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.mitDSPtr, &physAddress);
		BDBG_MSG(("Memory allocation for master index table: Address = 0x%08x, Physical Address=0x%08x, Size = 0x%08x",
			hDsp->memInfo.mitDSPtr, physAddress, memReq));
	}
	else
	{
		hDsp->memInfo.mitDSPtr=BRAP_P_INVALID_DRAM_ADDRESS;
	}

	
	if(hDsp->hRap->sSettings.bFwAuthEnable==true)
	{
		if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_IBOOT_ID, &ibootSize)))
		{
			BKNI_Free(encMemInfo);
			return BERR_TRACE(rc);
		}
		if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_SCHEDULER_CODE_ID, &schdCodeSize)))
		{
			BKNI_Free(encMemInfo);
			return BERR_TRACE(rc);
		}
		if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_SCHEDULER_DATA_ID, &schdDataSize)))
		{
			BKNI_Free(encMemInfo);
			return BERR_TRACE(rc);
		}
#ifndef MP3ENC_STANDALONE
		fwSize+=ibootSize;
#endif
		fwSize+=schdCodeSize+schdDataSize;
		
		
		if (sDecModeMemReq.ui32FsSize) 
		{	
			fwSize+=sDecModeMemReq.ui32FsSize;
		}
		if (sDecModeMemReq.ui32FwCodeSize)
		{	
			fwSize+=sDecModeMemReq.ui32FwCodeSize;
	
		}
		if (sPtModeMemReq.ui32FwCodeSize)
		{	
			fwSize+=sPtModeMemReq.ui32FwCodeSize;

		}
		if (sPpMemReq.ui32PpFwCodeSize)
		{	
			fwSize+=sPpMemReq.ui32PpFwCodeSize;
		}


        /* Add encoder buffer requirement */
    
    	encMemInfo=( BRAP_DSP_P_MemInfo *)BKNI_Malloc (sizeof(BRAP_DSP_P_MemInfo));    
    	if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetEncFwSize(hDsp,encMemInfo)))
		{
			BKNI_Free(encMemInfo);
			return BERR_TRACE(rc);
		}
		
			
#ifdef MP3ENC_STANDALONE /* Changing for merged scheduler */
    	fwSize+=encMemInfo.iboot_ptr;
#endif



    	fwSize+=encMemInfo->encscheduler_code_ptr 
    			+ encMemInfo->encscheduler_data_ptr
    			+ encMemInfo->totalEncAlgoSize
    			+ encMemInfo->totalEncTblSize
    			+ encMemInfo->totalEncPrePAlgoSize
    			+ encMemInfo->totalEncPrePTblSize
    			+ encMemInfo->totalEncPostPAlgoSize
    			+ encMemInfo->totalEncPostPTblSize;



/*Allocating a chunk of memory */
		baseAddr=(uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
					fwSize,
					5,
					0
#if (BRAP_SECURE_HEAP==1) 
					,false
#endif												
											
				    );
		if (baseAddr==BRAP_P_INVALID_DRAM_ADDRESS)
		{
			BDBG_ERR(("Memory not allocated"));
			goto alloc_total_buf_failed;	
		}
		hDsp->memInfo.allocFwDwnldSize=fwSize;
		hDsp->memInfo.fwDwnldOffset=baseAddr;
		BKNI_Memset((void *)baseAddr, 0, fwSize);
		bufferUsed=0;
#ifdef MP3ENC_STANDALONE 
		if(encMemInfo->iboot_ptr)
		{
			hDsp->memInfo.iboot_ptr=baseAddr+bufferUsed;
			bufferUsed+=encMemInfo->iboot_ptr;
		}
		else
		{
			hDsp->memInfo.iboot_ptr= BRAP_P_INVALID_DRAM_ADDRESS;
		}
#endif
		if(encMemInfo->encscheduler_code_ptr)
		{
			hDsp->memInfo.encscheduler_code_ptr=baseAddr+bufferUsed;
			bufferUsed+=encMemInfo->encscheduler_code_ptr;
		}
		else
		{
			hDsp->memInfo.encscheduler_code_ptr= BRAP_P_INVALID_DRAM_ADDRESS;
		}
		if(encMemInfo->encscheduler_data_ptr)
		{
			hDsp->memInfo.encscheduler_data_ptr=baseAddr+bufferUsed;
			bufferUsed+=encMemInfo->encscheduler_data_ptr;
		}
		else
		{
			hDsp->memInfo.encscheduler_data_ptr= BRAP_P_INVALID_DRAM_ADDRESS;
		}

	
		if(encMemInfo->totalEncAlgoSize)
		{
			hDsp->memInfo.encAlgoBasePtr=baseAddr+bufferUsed;
			hDsp->memInfo.totalEncAlgoSize=encMemInfo->totalEncAlgoSize;
			bufferUsed+=encMemInfo->totalEncAlgoSize;
		}
		else
		{
			hDsp->memInfo.encAlgoBasePtr= BRAP_P_INVALID_DRAM_ADDRESS;
			hDsp->memInfo.totalEncAlgoSize=0;
		}
		if(encMemInfo->totalEncTblSize)
		{
			hDsp->memInfo.encTblBasePtr=baseAddr+bufferUsed;
			hDsp->memInfo.totalEncTblSize=encMemInfo->totalEncTblSize;
			bufferUsed+=encMemInfo->totalEncTblSize;
		}
		else
		{
			hDsp->memInfo.encTblBasePtr= BRAP_P_INVALID_DRAM_ADDRESS;
			hDsp->memInfo.totalEncTblSize=0;			
		}
		if(encMemInfo->totalEncPrePAlgoSize)
		{
			hDsp->memInfo.encPrePAlgoBasePtr = baseAddr+bufferUsed;
			hDsp->memInfo.totalEncPrePAlgoSize=encMemInfo->totalEncPrePAlgoSize;
			bufferUsed+=encMemInfo->totalEncPrePAlgoSize;
		}
		else
		{
	/*		hDsp->memInfo.encPrePAlgoBasePtr= BRAP_P_INVALID_DRAM_ADDRESS;*/
			hDsp->memInfo.totalEncPrePAlgoSize=0;			
		}
		if(encMemInfo->totalEncPrePTblSize)
		{
			hDsp->memInfo.encPrePTblBasePtr= baseAddr+bufferUsed;
			hDsp->memInfo.totalEncPrePTblSize=encMemInfo->totalEncPrePTblSize;						
			bufferUsed+=encMemInfo->totalEncPrePTblSize;
		}
		else
		{
/*			hDsp->memInfo.encPrePTblBasePtr= BRAP_P_INVALID_DRAM_ADDRESS;*/
			hDsp->memInfo.totalEncPrePTblSize=0;						
		}
		if(encMemInfo->totalEncPostPAlgoSize)
		{
			hDsp->memInfo.encPostPAlgoBasePtr= baseAddr+bufferUsed;
			hDsp->memInfo.totalEncPostPAlgoSize=encMemInfo->totalEncPostPAlgoSize;
			bufferUsed+=encMemInfo->totalEncPostPAlgoSize;
		}
		else
		{
/*			hDsp->memInfo.encPostPAlgoBasePtr= BRAP_P_INVALID_DRAM_ADDRESS;*/
			hDsp->memInfo.totalEncPostPAlgoSize=0;			
		}
		if(encMemInfo->totalEncPostPTblSize)
		{
			hDsp->memInfo.encPostPTblBasePtr= baseAddr+bufferUsed;
			hDsp->memInfo.totalEncPostPTblSize=encMemInfo->totalEncPostPTblSize;
			bufferUsed+=encMemInfo->totalEncPostPTblSize;
		}
		else
		{
/*			hDsp->memInfo.encPostPTblBasePtr= BRAP_P_INVALID_DRAM_ADDRESS;*/
			hDsp->memInfo.totalEncPostPTblSize=0;			
		}
		

#ifndef MP3ENC_STANDALONE
		if (ibootSize)
		{
			hDsp->memInfo.iboot_ptr = baseAddr+bufferUsed;
			bufferUsed+=ibootSize;
			BDBG_MSG(("Memory allocation for iboot: Address = 0x%08x, Size = 0x%08x", 
					hDsp->memInfo.iboot_ptr, ibootSize));
		}
		else
		{
			hDsp->memInfo.iboot_ptr = BRAP_P_INVALID_DRAM_ADDRESS;
		}
#endif	
		if (schdCodeSize)
		{
			hDsp->memInfo.scheduler_code_ptr = baseAddr+bufferUsed;
			bufferUsed+=schdCodeSize;
			BDBG_MSG(("Memory allocation for scheduler code: Address = 0x%08x, Size = 0x%08x",
					hDsp->memInfo.scheduler_code_ptr, schdCodeSize));
		}
		else
		{
			hDsp->memInfo.scheduler_code_ptr = BRAP_P_INVALID_DRAM_ADDRESS;
		}
		if (schdDataSize)
		{
			hDsp->memInfo.scheduler_data_ptr = baseAddr+bufferUsed;
			bufferUsed+=schdDataSize;
			
			BDBG_MSG(("Memory allocation for scheduler data: Address = 0x%08x, Size = 0x%08x",
					hDsp->memInfo.scheduler_data_ptr, schdDataSize));
		}
		else
		{
			hDsp->memInfo.scheduler_data_ptr = BRAP_P_INVALID_DRAM_ADDRESS;
		}
	
		hDsp->memInfo.fsBufBasePtr[0]=baseAddr+bufferUsed;
		hDsp->memInfo.actualFwDwnldSize=bufferUsed;
		bufferUsed += sDecModeMemReq.ui32FsSize + sDecModeMemReq.ui32FwCodeSize 
					+sPtModeMemReq.ui32FwCodeSize;

		/* Setting Unused Variables of memInfo to Invalid*/
		for (count = 1; count < BRAP_DSP_P_MAX_NUM_FS_CTXT_IN_FS_EXE_BUF ; count++)
		{
			hDsp->memInfo.fsBufBasePtr[count]=BRAP_P_INVALID_DRAM_ADDRESS;
			hDsp->memInfo.fsBufSize[count] = BRAP_DSP_P_CTXT_FS_EXE_SIZE;
		}
		for (count = 0; count < BRAP_RM_P_MAX_DECODE_SUPPORTED; count++)
		{
			hDsp->memInfo.ctxtFwBufBasePtr[count]=BRAP_P_INVALID_DRAM_ADDRESS;
			hDsp->memInfo.ctxtFwBufSize[count] = 0;
		}
		for (count = 0; count < BRAP_RM_P_MAX_PASSTHRU_SUPPORTED; count++)
		{
			hDsp->memInfo.ctxtFwBufSize[count+BRAP_DSP_P_PASSTHRU_CTXT_BUFFER] = 0;
			hDsp->memInfo.ctxtFwBufBasePtr[count+BRAP_DSP_P_PASSTHRU_CTXT_BUFFER] = BRAP_P_INVALID_DRAM_ADDRESS;
		}
		
    	hDsp->memInfo.ui32PpFwBufSize= sPpMemReq.ui32PpFwCodeSize;
		if(sPpMemReq.ui32PpFwCodeSize)
		{
			hDsp->memInfo.ui32PpFwBufPtr = baseAddr+bufferUsed;
			bufferUsed+=sPpMemReq.ui32PpFwCodeSize;
    		BDBG_MSG(("Memory allocation for post processing buffer: Address = 0x%08x, Size = 0x%08x",
        		hDsp->memInfo.ui32PpFwBufPtr, hDsp->memInfo.ui32PpFwBufSize));    
		}
		else
		{
			hDsp->memInfo.ui32PpFwBufPtr = BRAP_P_INVALID_DRAM_ADDRESS;
		}
		hDsp->memInfo.actualFwDwnldSize += sPpMemReq.ui32PpFwCodeSize;


	}
	else
	{ /* no Fw authentication */
		if (BRAP_DSP_P_CTXT_FS_EXE_SIZE) 
		{
			hDsp->memInfo.fsBufBasePtr[0] = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
					BRAP_DSP_P_CTXT_FS_EXE_SIZE * BRAP_DSP_P_MAX_NUM_FS_CTXT_IN_FS_EXE_BUF,
					2,
					0
#if (BRAP_SECURE_HEAP==1) 
                    ,false
#endif												
											
			);
			if (hDsp->memInfo.fsBufBasePtr[0]==BRAP_P_INVALID_DRAM_ADDRESS)
				goto alloc_framesync_buf_failed;
				
			/* Framesync firmware context buffer */
			for (count = 0; count < BRAP_DSP_P_MAX_NUM_FS_CTXT_IN_FS_EXE_BUF ; count++)
			{
				hDsp->memInfo.fsBufSize[count] = BRAP_DSP_P_CTXT_FS_EXE_SIZE;
				hDsp->memInfo.fsBufBasePtr[count] = hDsp->memInfo.fsBufBasePtr[0]  +
					count * BRAP_DSP_P_CTXT_FS_EXE_SIZE;

                BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.fsBufBasePtr[count], &physAddress); 
				BDBG_MSG(("Memory allocation for frame sync buffer ctxt%d: Address = 0x%08x, Physical Address=0x%08x, Size = 0x%08x",
				count, hDsp->memInfo.fsBufBasePtr[count], physAddress, hDsp->memInfo.fsBufSize[count]));
			}
		}
		else
		{
			for (count = 0; count < BRAP_DSP_P_MAX_NUM_FS_CTXT_IN_FS_EXE_BUF ; count++)
			{
				hDsp->memInfo.fsBufBasePtr[count]=BRAP_P_INVALID_DRAM_ADDRESS;
				hDsp->memInfo.fsBufSize[count] = BRAP_DSP_P_CTXT_FS_EXE_SIZE;
			}
		}
		if (sDecModeMemReq.ui32FwCodeSize)
		{
			hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_DECODE_CTXT_BUFFER] = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
															sDecModeMemReq.ui32FwCodeSize * BRAP_RM_P_MAX_DECODE_SUPPORTED,
															2,
															0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
											
												);


			if (hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_DECODE_CTXT_BUFFER] ==BRAP_P_INVALID_DRAM_ADDRESS)
				goto alloc_decode_buf_failed;
			
			/* FW Decode executables firmware context buffer */
			for (count = 0; count < BRAP_RM_P_MAX_DECODE_SUPPORTED; count++)
			{
				hDsp->memInfo.ctxtFwBufSize[count] = sDecModeMemReq.ui32FwCodeSize;
				hDsp->memInfo.ctxtFwBufBasePtr[count] = hDsp->memInfo.ctxtFwBufBasePtr[0]  +
                                        				count * sDecModeMemReq.ui32FwCodeSize;

                BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.ctxtFwBufBasePtr[count], &physAddress);
				BDBG_MSG(("Memory allocation for Decode Context Fw Buffer ctxt%d: Address = 0x%08x, Physical Address=0x%08x, Size = 0x%08x",
    				count, hDsp->memInfo.ctxtFwBufBasePtr[count], physAddress, hDsp->memInfo.ctxtFwBufSize[count]));
			}
		}
		else
		{
			for (count = 0; count < BRAP_RM_P_MAX_DECODE_SUPPORTED; count++)
			{
				hDsp->memInfo.ctxtFwBufBasePtr[count]=BRAP_P_INVALID_DRAM_ADDRESS;
				hDsp->memInfo.ctxtFwBufSize[count] = 0;
			}
				
		}

		/* Pass thru context buffer */
		if (sPtModeMemReq.ui32FwCodeSize)
		{
			hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_PASSTHRU_CTXT_BUFFER] = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
															sPtModeMemReq.ui32FwCodeSize * BRAP_RM_P_MAX_PASSTHRU_SUPPORTED,
															2,
															0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
											
												);
		
			if (hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_PASSTHRU_CTXT_BUFFER] ==BRAP_P_INVALID_DRAM_ADDRESS)
				goto alloc_passthru_buf_failed;

			/* FW Decode executables firmware context buffer */
			for (count = 0; count < BRAP_RM_P_MAX_PASSTHRU_SUPPORTED; count++)
			{
				hDsp->memInfo.ctxtFwBufSize[count+BRAP_DSP_P_PASSTHRU_CTXT_BUFFER] = sPtModeMemReq.ui32FwCodeSize;
				hDsp->memInfo.ctxtFwBufBasePtr[count+BRAP_DSP_P_PASSTHRU_CTXT_BUFFER] = hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_PASSTHRU_CTXT_BUFFER]  +
					count * sPtModeMemReq.ui32FwCodeSize;
                
                BRAP_P_ConvertAddressToOffset(hDsp->hHeap, 
                    (void *)hDsp->memInfo.ctxtFwBufBasePtr[count+BRAP_DSP_P_PASSTHRU_CTXT_BUFFER], 
                    &physAddress);
                BDBG_MSG(("Memory allocation for Pass-thru Context Fw Buffer ctxt%d: Address = 0x%08x, Physical Address=0x%08x, Size = 0x%08x",
    				count, hDsp->memInfo.ctxtFwBufBasePtr[count+BRAP_DSP_P_PASSTHRU_CTXT_BUFFER], physAddress,
    				hDsp->memInfo.ctxtFwBufSize[count+BRAP_DSP_P_PASSTHRU_CTXT_BUFFER]));
			}	
		
		}
		else
		{
			for (count = 0; count < BRAP_RM_P_MAX_PASSTHRU_SUPPORTED; count++)
			{
				hDsp->memInfo.ctxtFwBufSize[count+BRAP_DSP_P_PASSTHRU_CTXT_BUFFER] = 0;
				hDsp->memInfo.ctxtFwBufBasePtr[count+BRAP_DSP_P_PASSTHRU_CTXT_BUFFER] = BRAP_P_INVALID_DRAM_ADDRESS;
			}
		}

		/* Common post-processing algorithms */
    	if (sPpMemReq.ui32PpFwCodeSize)
    	{
    		hDsp->memInfo.ui32PpFwBufPtr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
    														sPpMemReq.ui32PpFwCodeSize,
    														2,
															0
#if (BRAP_SECURE_HEAP==1) 
                    										,false
#endif																							
            												);
    		
    		if (hDsp->memInfo.ui32PpFwBufPtr ==BRAP_P_INVALID_DRAM_ADDRESS)
    			goto alloc_pp_decode_buf_failed;

    		hDsp->memInfo.ui32PpFwBufSize = sPpMemReq.ui32PpFwCodeSize;
            
            BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.ui32PpFwBufPtr, &physAddress);
            BDBG_MSG(("Memory allocation for PP FW code: Address = 0x%08x, Physical Address=0x%08x, Size = 0x%08x",
                hDsp->memInfo.ui32PpFwBufPtr, physAddress, sPpMemReq.ui32PpFwCodeSize));

    	}
    	else
    	{
    		hDsp->memInfo.ui32PpFwBufPtr=BRAP_P_INVALID_DRAM_ADDRESS;
    	}
	} /* no fw authentication*/

	if (sDecModeMemReq.ui32DecConfigParamBufSize)
	{
		hDsp->memInfo.arrayOfConfigBufSize =
			4 * ( BRAP_P_MAX_PP_SUPPORTED + 1 );
		
		size = hDsp->memInfo.arrayOfConfigBufSize;

		hDsp->memInfo.arrayOfConfigBufPtr[0]= (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
			(size*2*BRAP_P_MAX_BRANCH_SUPPORTED* BAF_MAX_CONTEXTS),
				2,
				0
#if (BRAP_SECURE_HEAP==1) 
				,false
#endif																								
				);

		if (hDsp->memInfo.arrayOfConfigBufPtr[0]==BRAP_P_INVALID_DRAM_ADDRESS)
			goto alloc_arrayOfConfig_buf_failed;

		for (count = 0; count < BAF_MAX_CONTEXTS; count++)
		{
			hDsp->memInfo.arrayOfConfigBufPtr[count] = hDsp->memInfo.arrayOfConfigBufPtr[0]
				+ count * ( hDsp->memInfo.arrayOfConfigBufSize *	 2 * BRAP_P_MAX_BRANCH_SUPPORTED );
			BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.arrayOfConfigBufPtr[count] , &physAddress);
			BDBG_MSG(("Memory allocation for Config buffer Array %d: Address = 0x%08x, Physical Address = 0x%08x, Size = 0x%08x",
				count, hDsp->memInfo.arrayOfConfigBufPtr[count], physAddress,
				( hDsp->memInfo.arrayOfConfigBufSize *	 2 * BRAP_P_MAX_BRANCH_SUPPORTED )));
		}
	}
	else
	{
		for (count = 0; count < BAF_MAX_CONTEXTS; count++)
    			hDsp->memInfo.arrayOfConfigBufPtr[count] = BRAP_P_INVALID_DRAM_ADDRESS;
	}
    
	BKNI_Free(encMemInfo);

	return BERR_SUCCESS; 

	/* Memory allocation failed. Free all allocated memory and return error. */
#if (BRAP_SECURE_HEAP == 1)
alloc_arrayOfConfig_buf_failed:	
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ui32PpFwBufPtr,false);
alloc_pp_decode_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_PASSTHRU_CTXT_BUFFER], false);   
alloc_passthru_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_DECODE_CTXT_BUFFER],false);
	}
alloc_decode_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.fsBufBasePtr[0],false);
	}
alloc_framesync_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.mitDSPtr,false);
	}
alloc_total_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==true)
	{
		BRAP_P_Free(hDsp->hRap,(void *) hDsp->memInfo.mitDSPtr,false);
	}
alloc_mit_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.decConfigBufPtr[0],false);
alloc_decconfig_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.interFrameBufBasePtr[0],false);
alloc_interstage_interface_buf_failed:	
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.interStageBufBasePtr[0],false);
alloc_interstage_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ui32PpInterFrameBufBasePtr[0],false);    
alloc_pp_interframe_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.interFrameBufBasePtr[0], false);
alloc_interframe_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scratchSpaceBufPtr,false);
alloc_pp_scratch_space_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scratchSpaceBufPtr, false);
alloc_scratch_space_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.apmScratchBufPtr,false);
alloc_apm_scratch_buf_failed:
    BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ui32DwnldableSchedulerPtr,false);
alloc_dnld_scheduler_code_buf_failed:		
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scheduler_data_ptr,false);
	}
alloc_scheduler_data_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scheduler_code_ptr,false);
	}
alloc_scheduler_code_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.iboot_ptr,false);
	}
alloc_iboot_buf_failed:
	BKNI_Free(encMemInfo); 
	return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);


#else /* else of #if (BRAP_SECURE_HEAP == 1) */

alloc_pp_decode_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_PASSTHRU_CTXT_BUFFER]);        
alloc_passthru_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_DECODE_CTXT_BUFFER]);
	}
alloc_decode_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.fsBufBasePtr[0]);
	}
alloc_framesync_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.mitDSPtr);
	}
alloc_total_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==true)
	{
		BRAP_P_Free(hDsp->hRap,(void *) hDsp->memInfo.mitDSPtr);
	}
alloc_mit_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.decConfigBufPtr[0]);

alloc_decconfig_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.interStageInterfaceBufBasePtr[0]);
alloc_interstage_interface_buf_failed:	
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.interStageBufBasePtr[0]);
alloc_interstage_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ui32PpInterFrameBufBasePtr[0]);
alloc_pp_interframe_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.interFrameBufBasePtr[0]);

alloc_interframe_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ui32PpScratchBufPtr);
alloc_pp_scratch_space_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scratchSpaceBufPtr);

alloc_scratch_space_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.apmScratchBufPtr);

alloc_apm_scratch_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scheduler_data_ptr);
	}
alloc_scheduler_data_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scheduler_code_ptr);
	}
alloc_scheduler_code_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.iboot_ptr);
	}
alloc_iboot_buf_failed:
	BKNI_Free(encMemInfo); 
	return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);

#endif /* endif of #if (BRAP_SECURE_HEAP == 1) */

}
#else /* else part of (BRAP_DSP_P_7401_NEWMIT==1) */
static BERR_Code BRAP_DSP_P_AllocMem(BRAP_DSP_Handle hDsp)
{
	BERR_Code rc;
	uint32_t memReq, count=0, size=0;
#ifndef BCHP_7411_VER	
	uint32_t fwSize=0,ibootSize=0,schdCodeSize=0,schdDataSize=0,bufferUsed=0;
	uint32_t baseAddr=0;
#endif	
	bool bSelectedAlgos[BRAP_MAX_AUDIO_TYPES];
	BRAP_DEC_P_ChannelMemReq	sDecModeMemReq, sPtModeMemReq;
    BRAP_DSP_P_MemInfo * encMemInfo;
#if (BCHP_7411_VER) || (BRAP_7401_FAMILY == 1)
	bool bSelectedPpAlgos[BRAP_MAX_PP_ALGOS];
	BRAP_DEC_P_ChannelMemReq sPpMemReq;
#endif
#ifdef BCHP_7411_VER
	BRAP_AudioMemAllocation	eAudioMemAllocation = BRAP_AudioMemAllocation_eStereo;
#endif

	encMemInfo=( BRAP_DSP_P_MemInfo *)BKNI_Malloc (sizeof(BRAP_DSP_P_MemInfo));

#if BCHP_7411_VER > BCHP_VER_C0
	eAudioMemAllocation = BRAP_P_GetMemoryAllocationType(hDsp->hRap);
#endif
	/* Get algorithms selected by application */
	rc = BRAP_P_GetSelectedAlgos(hDsp->hRap, bSelectedAlgos);
	if (BERR_SUCCESS != rc)
	{
		BKNI_Free(encMemInfo);
		return BERR_TRACE(rc);
	}

	/* Get buffer sizes required for combination of selected algorithms */	
	rc = BRAP_P_GetDecChannelBufferSizes(bSelectedAlgos,
			BRAP_DSPCHN_DecodeMode_eDecode,
#ifdef BCHP_7411_VER			
			eAudioMemAllocation,
#endif			
			&sDecModeMemReq
#ifndef BCHP_7411_VER
			,hDsp->hRap->sSettings.bFwAuthEnable
#endif
			);
	if (BERR_SUCCESS != rc)
	{
		BKNI_Free(encMemInfo);
		return BERR_TRACE(rc);
	}
	
	rc = BRAP_P_GetDecChannelBufferSizes(bSelectedAlgos,
			BRAP_DSPCHN_DecodeMode_ePassThru,
#ifdef BCHP_7411_VER			
			eAudioMemAllocation,
#endif			
			&sPtModeMemReq
#ifndef BCHP_7411_VER
			,hDsp->hRap->sSettings.bFwAuthEnable
#endif
);
	if (BERR_SUCCESS != rc)
	{
		BKNI_Free(encMemInfo);
		return BERR_TRACE(rc);
	}
         
    

#if (BCHP_7411_VER) || (BRAP_7401_FAMILY == 1)
	/* Get post processing algorithms selected by application */
	rc = BRAP_P_GetSelectedPpAlgos(hDsp->hRap, bSelectedPpAlgos);
	if (BERR_SUCCESS != rc)
	{
		BKNI_Free(encMemInfo);
		return BERR_TRACE(rc);
	}

#if (BRAP_7401_FAMILY == 1)
    if (bSelectedPpAlgos[BRAP_DSPCHN_PP_Algo_eSrc] == true)
    {
        /* For the SRC module, hack the interframe buffer size  */
        BDBG_MSG(("sDecModeMemReq.ui32InterframeBufSize = %d", sDecModeMemReq.ui32InterframeBufSize));
        sDecModeMemReq.ui32InterframeBufSize += BRAP_DSP_P_SRC_INTERFRAME_BUF_SIZE; /* add for SRC interframe */    
        BDBG_MSG(("After adding %d bytes for SRC, sDecModeMemReq.ui32InterframeBufSize = %d", 
            BRAP_DSP_P_SRC_INTERFRAME_BUF_SIZE, sDecModeMemReq.ui32InterframeBufSize));
    }
#endif

	/* Get buffer sizes required for combination of selected algorithms */
	BRAP_P_GetPpBufferSizes(bSelectedPpAlgos, &sPpMemReq);
#endif

	/* Allocate memory for resident code */	
#ifndef BCHP_7411_VER
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
#endif
		if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_IBOOT_ID, &size)))
    	{
    		BKNI_Free(encMemInfo);
    		return BERR_TRACE(rc);
    	}

		if (size)
		{
			hDsp->memInfo.iboot_ptr = (uint32_t)BRAP_P_AllocAligned(hDsp->hRap,
												size,
												2,
												0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
												);
			if (hDsp->memInfo.iboot_ptr==BRAP_P_INVALID_DRAM_ADDRESS) 
				goto alloc_iboot_buf_failed;
				
			BDBG_MSG(("Memory allocation for iboot: Address = 0x%08x, Size = 0x%08x", 
					hDsp->memInfo.iboot_ptr, size));
		}
		else
		{
			hDsp->memInfo.iboot_ptr = BRAP_P_INVALID_DRAM_ADDRESS;
		}

		if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_SCHEDULER_CODE_ID, &size)))
    	{
    		BKNI_Free(encMemInfo);
    		return BERR_TRACE(rc);
    	}

		if (size)
		{
			hDsp->memInfo.scheduler_code_ptr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
												size,
												2,
												0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
												);
			if (hDsp->memInfo.scheduler_code_ptr==BRAP_P_INVALID_DRAM_ADDRESS)
				goto alloc_scheduler_code_buf_failed;
		
			BDBG_MSG(("Memory allocation for scheduler code: Address = 0x%08x, Size = 0x%08x",
					hDsp->memInfo.scheduler_code_ptr, size));
		}
		else
		{
			hDsp->memInfo.scheduler_code_ptr = BRAP_P_INVALID_DRAM_ADDRESS;
		}

		if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_SCHEDULER_DATA_ID, &size)))
    	{
    		BKNI_Free(encMemInfo);
    		return BERR_TRACE(rc);
    	}

		if (size)
		{
			hDsp->memInfo.scheduler_data_ptr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
												size,
												2,
												0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
												
												);
			if (hDsp->memInfo.scheduler_data_ptr==BRAP_P_INVALID_DRAM_ADDRESS)
				goto alloc_scheduler_data_buf_failed;
			
			BDBG_MSG(("Memory allocation for scheduler data: Address = 0x%08x, Size = 0x%08x",
					hDsp->memInfo.scheduler_data_ptr, size));
		}
		else
		{
			hDsp->memInfo.scheduler_data_ptr = BRAP_P_INVALID_DRAM_ADDRESS;
		}
#ifndef BCHP_7411_VER		
	}
#endif

	/* Allocate memory for APM scratch buffer */
	if (BRAP_DSP_P_APM_SCRATCH_BUF_SIZE)
	{
		hDsp->memInfo.apmScratchBufPtr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
												BRAP_DSP_P_APM_SCRATCH_BUF_SIZE,
												2,
												0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
											
												);
		if (hDsp->memInfo.apmScratchBufPtr==BRAP_P_INVALID_DRAM_ADDRESS)
			goto alloc_apm_scratch_buf_failed;
		
		BDBG_MSG(("Memory allocation for APM scratch buffer: Address = 0x%08x, Size = 0x%08x",
				hDsp->memInfo.apmScratchBufPtr, BRAP_DSP_P_APM_SCRATCH_BUF_SIZE));
	}
	else
	{
		hDsp->memInfo.apmScratchBufPtr = BRAP_P_INVALID_DRAM_ADDRESS;
	}

	/* Scratch space buffer allocation */
	if (sDecModeMemReq.ui32ScratchBufSize)
	{
		hDsp->memInfo.scratchSpaceBufPtr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
											sDecModeMemReq.ui32ScratchBufSize,
											2,
											0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
											
												);
		if (hDsp->memInfo.scratchSpaceBufPtr==BRAP_P_INVALID_DRAM_ADDRESS)
			goto alloc_scratch_space_buf_failed;
		
		BDBG_MSG(("Memory allocation for scratch space buffer: Address = 0x%08x, Size = 0x%08x",
				hDsp->memInfo.scratchSpaceBufPtr, sDecModeMemReq.ui32ScratchBufSize));
	}
	else
	{
		hDsp->memInfo.scratchSpaceBufPtr = BRAP_P_INVALID_DRAM_ADDRESS;
	}
	hDsp->memInfo.scratchSpaceBufSize = sDecModeMemReq.ui32ScratchBufSize;

	/* Interframe buffer allocation - done only for max decode contexts that can exist together */
	if (sDecModeMemReq.ui32InterframeBufSize)
	{
		hDsp->memInfo.interFrameBufBasePtr[0] =  (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
							sDecModeMemReq.ui32InterframeBufSize * BRAP_RM_P_MAX_DECODE_SUPPORTED,
							2,
							0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
												
												);
		if (hDsp->memInfo.interFrameBufBasePtr[0]==BRAP_P_INVALID_DRAM_ADDRESS)
			goto alloc_interframe_buf_failed;

		for (count = 0; count < BRAP_RM_P_MAX_DECODE_SUPPORTED; count++)
		{
			hDsp->memInfo.interFrameBufBasePtr[count] = hDsp->memInfo.interFrameBufBasePtr[0]
				+ count * sDecModeMemReq.ui32InterframeBufSize;
			BDBG_MSG(("Memory allocation for interframe buffer %d: Address = 0x%08x, Size = 0x%08x",
				count, hDsp->memInfo.interFrameBufBasePtr[count], 
				sDecModeMemReq.ui32InterframeBufSize));
		}
	}
	else
	{
		for (count = 0; count < BRAP_RM_P_MAX_DECODE_SUPPORTED; count++) 
			hDsp->memInfo.interFrameBufBasePtr[count]=BRAP_P_INVALID_DRAM_ADDRESS;
	}
	hDsp->memInfo.interFrameBufSize = sDecModeMemReq.ui32InterframeBufSize;

	/* Decoder configuration parameters buffer allocation done only for max decode contexts that can exist together. 
	 * Decoder configuration parameters are programmed using ping-pong buffer mechanism. Hence per decode
	 * context we allocate two buffers of size (sDecModeMemReq.ui32DecConfigParamBufSize * 2) 
	 */
	if (sDecModeMemReq.ui32DecConfigParamBufSize)
	{
		hDsp->memInfo.decConfigBufPtr[0] = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
							sDecModeMemReq.ui32DecConfigParamBufSize * 2 * BRAP_RM_P_MAX_DECODE_SUPPORTED,
							2,
							0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
											
												);

		if (hDsp->memInfo.decConfigBufPtr[0]==BRAP_P_INVALID_DRAM_ADDRESS)
			goto alloc_decconfig_buf_failed;

		for (count = 0; count < BRAP_RM_P_MAX_DECODE_SUPPORTED; count++)
		{
			hDsp->memInfo.decConfigBufPtr[count] = hDsp->memInfo.decConfigBufPtr[0]
				+ count * sDecModeMemReq.ui32DecConfigParamBufSize * 2;
			BDBG_MSG(("Memory allocation for dec config buffer %d: Address = 0x%08x, Size = 0x%08x",
				count, hDsp->memInfo.decConfigBufPtr[count], sDecModeMemReq.ui32DecConfigParamBufSize * 2));
		}
	}
	else
	{
		for (count = 0; count < BRAP_RM_P_MAX_DECODE_SUPPORTED; count++)
			hDsp->memInfo.decConfigBufPtr[count]=BRAP_P_INVALID_DRAM_ADDRESS;
	}
	hDsp->memInfo.decConfigBufSize = sDecModeMemReq.ui32DecConfigParamBufSize;
		
	
	/* Calculate and allocate total memory required by MIT and its sub structures */
	memReq = SIZE_OF_BAF_HOST_INFO_ALLIGNED + \
			 (BAF_HOST_MAX_DL_MODULE * SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED) + \
			 (3 * BAF_HOST_MAX_DL_MODULE * SIZE_OF_BAF_DOWNLOAD_ALLIGNED);
	memReq = BRAP_P_ALIGN(memReq, 2);
	hDsp->memInfo.totalMITSize = memReq;

	if (memReq)
	{
		hDsp->memInfo.mitDSPtr =  (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
												memReq,
												2,
												0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
												
												); 
		if (hDsp->memInfo.mitDSPtr==BRAP_P_INVALID_DRAM_ADDRESS)
			goto alloc_mit_buf_failed;

		BDBG_MSG(("Memory allocation for master index table: Address = 0x%08x, Size = 0x%08x",
			hDsp->memInfo.mitDSPtr, memReq));
	}
	else
	{
		hDsp->memInfo.mitDSPtr=BRAP_P_INVALID_DRAM_ADDRESS;
	}

	
#ifndef BCHP_7411_VER
	if(hDsp->hRap->sSettings.bFwAuthEnable==true)
	{
		if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_IBOOT_ID, &ibootSize)))
		{
			BKNI_Free(encMemInfo);
			return BERR_TRACE(rc);
		}
		if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_SCHEDULER_CODE_ID, &schdCodeSize)))
		{
			BKNI_Free(encMemInfo);
			return BERR_TRACE(rc);
		}
		if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_SCHEDULER_DATA_ID, &schdDataSize)))
		{
			BKNI_Free(encMemInfo);
			return BERR_TRACE(rc);
		}

#ifndef MP3ENC_STANDALONE
		fwSize+=ibootSize;
#endif		
		fwSize+=schdCodeSize+schdDataSize;
		
		
		if (sDecModeMemReq.ui32FsSize) 
		{	
			fwSize+=sDecModeMemReq.ui32FsSize;
		}
		if (sDecModeMemReq.ui32FwCodeSize)
		{	
			fwSize+=sDecModeMemReq.ui32FwCodeSize;
	
		}
		if (sPtModeMemReq.ui32FwCodeSize)
		{	
			fwSize+=sPtModeMemReq.ui32FwCodeSize;

		}
#if (BRAP_7401_FAMILY == 1)
		if (sPpMemReq.ui32FwCodeSize)
		{	
			fwSize+=sPpMemReq.ui32FwCodeSize;
		}
#endif


        /* Add encoder buffer requirement */

        encMemInfo=( BRAP_DSP_P_MemInfo *)BKNI_Malloc (sizeof(BRAP_DSP_P_MemInfo));
        if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetEncFwSize(hDsp,encMemInfo)))
        {
        	BKNI_Free(encMemInfo);
        	return BERR_TRACE(rc);
        }
		
			
#ifdef MP3ENC_STANDALONE /* Changing for merged scheduler */
	fwSize+=encMemInfo.iboot_ptr;
#endif



	fwSize+=encMemInfo->encscheduler_code_ptr 
			+ encMemInfo->encscheduler_data_ptr
			+ encMemInfo->totalEncAlgoSize
			+ encMemInfo->totalEncTblSize
			+ encMemInfo->totalEncPrePAlgoSize
			+ encMemInfo->totalEncPrePTblSize
			+ encMemInfo->totalEncPostPAlgoSize
			+ encMemInfo->totalEncPostPTblSize;



/*Allocating a chunk of memory */
		baseAddr=(uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
					fwSize,
					5,
					0
#if (BRAP_SECURE_HEAP==1) 
					,false
#endif												
											
												);
		if (baseAddr==BRAP_P_INVALID_DRAM_ADDRESS)
		{
			BDBG_ERR(("Memory not allocated"));
			goto alloc_total_buf_failed;	
		}
		hDsp->memInfo.allocFwDwnldSize=fwSize;
		hDsp->memInfo.fwDwnldOffset=baseAddr;
		BKNI_Memset((void *)baseAddr, 0, fwSize);
		bufferUsed=0;

	
	#ifdef MP3ENC_STANDALONE 
		if(encMemInfo->iboot_ptr)
		{
			hDsp->memInfo.iboot_ptr=baseAddr+bufferUsed;
			bufferUsed+=encMemInfo->iboot_ptr;
		}
		else
		{
			hDsp->memInfo.iboot_ptr= BRAP_P_INVALID_DRAM_ADDRESS;
		}
	#endif
		if(encMemInfo->encscheduler_code_ptr)
		{
			hDsp->memInfo.encscheduler_code_ptr=baseAddr+bufferUsed;
			bufferUsed+=encMemInfo->encscheduler_code_ptr;
		}
		else
		{
			hDsp->memInfo.encscheduler_code_ptr= BRAP_P_INVALID_DRAM_ADDRESS;
		}
		if(encMemInfo->encscheduler_data_ptr)
		{
			hDsp->memInfo.encscheduler_data_ptr=baseAddr+bufferUsed;
			bufferUsed+=encMemInfo->encscheduler_data_ptr;
		}
		else
		{
			hDsp->memInfo.encscheduler_data_ptr= BRAP_P_INVALID_DRAM_ADDRESS;
		}

	
		if(encMemInfo->totalEncAlgoSize)
		{
			hDsp->memInfo.encAlgoBasePtr=baseAddr+bufferUsed;
			hDsp->memInfo.totalEncAlgoSize=encMemInfo->totalEncAlgoSize;
			bufferUsed+=encMemInfo->totalEncAlgoSize;
		}
		else
		{
			hDsp->memInfo.encAlgoBasePtr= BRAP_P_INVALID_DRAM_ADDRESS;
			hDsp->memInfo.totalEncAlgoSize=0;
		}
		if(encMemInfo->totalEncTblSize)
		{
			hDsp->memInfo.encTblBasePtr=baseAddr+bufferUsed;
			hDsp->memInfo.totalEncTblSize=encMemInfo->totalEncTblSize;
			bufferUsed+=encMemInfo->totalEncTblSize;
		}
		else
		{
			hDsp->memInfo.encTblBasePtr= BRAP_P_INVALID_DRAM_ADDRESS;
			hDsp->memInfo.totalEncTblSize=0;			
		}
		if(encMemInfo->totalEncPrePAlgoSize)
		{
			hDsp->memInfo.encPrePAlgoBasePtr = baseAddr+bufferUsed;
			hDsp->memInfo.totalEncPrePAlgoSize=encMemInfo->totalEncPrePAlgoSize;
			bufferUsed+=encMemInfo->totalEncPrePAlgoSize;
		}
		else
		{
	/*		hDsp->memInfo.encPrePAlgoBasePtr= BRAP_P_INVALID_DRAM_ADDRESS;*/
			hDsp->memInfo.totalEncPrePAlgoSize=0;			
		}
		if(encMemInfo->totalEncPrePTblSize)
		{
			hDsp->memInfo.encPrePTblBasePtr= baseAddr+bufferUsed;
			hDsp->memInfo.totalEncPrePTblSize=encMemInfo->totalEncPrePTblSize;						
			bufferUsed+=encMemInfo->totalEncPrePTblSize;
		}
		else
		{
/*			hDsp->memInfo.encPrePTblBasePtr= BRAP_P_INVALID_DRAM_ADDRESS;*/
			hDsp->memInfo.totalEncPrePTblSize=0;						
		}
		if(encMemInfo->totalEncPostPAlgoSize)
		{
			hDsp->memInfo.encPostPAlgoBasePtr= baseAddr+bufferUsed;
			hDsp->memInfo.totalEncPostPAlgoSize=encMemInfo->totalEncPostPAlgoSize;
			bufferUsed+=encMemInfo->totalEncPostPAlgoSize;
		}
		else
		{
/*			hDsp->memInfo.encPostPAlgoBasePtr= BRAP_P_INVALID_DRAM_ADDRESS;*/
			hDsp->memInfo.totalEncPostPAlgoSize=0;			
		}
		if(encMemInfo->totalEncPostPTblSize)
		{
			hDsp->memInfo.encPostPTblBasePtr= baseAddr+bufferUsed;
			hDsp->memInfo.totalEncPostPTblSize=encMemInfo->totalEncPostPTblSize;
			bufferUsed+=encMemInfo->totalEncPostPTblSize;
		}
		else
		{
/*			hDsp->memInfo.encPostPTblBasePtr= BRAP_P_INVALID_DRAM_ADDRESS;*/
			hDsp->memInfo.totalEncPostPTblSize=0;			
		}
		

#ifndef MP3ENC_STANDALONE
		if (ibootSize)
		{
			hDsp->memInfo.iboot_ptr = baseAddr+bufferUsed;
			bufferUsed+=ibootSize;
			BDBG_MSG(("Memory allocation for iboot: Address = 0x%08x, Size = 0x%08x", 
					hDsp->memInfo.iboot_ptr, ibootSize));
		}
		else
		{
			hDsp->memInfo.iboot_ptr = BRAP_P_INVALID_DRAM_ADDRESS;
		}
#endif	
		if (schdCodeSize)
		{
			hDsp->memInfo.scheduler_code_ptr = baseAddr+bufferUsed;
			bufferUsed+=schdCodeSize;
			BDBG_MSG(("Memory allocation for scheduler code: Address = 0x%08x, Size = 0x%08x",
					hDsp->memInfo.scheduler_code_ptr, schdCodeSize));
		}
		else
		{
			hDsp->memInfo.scheduler_code_ptr = BRAP_P_INVALID_DRAM_ADDRESS;
		}
		if (schdDataSize)
		{
			hDsp->memInfo.scheduler_data_ptr = baseAddr+bufferUsed;
			bufferUsed+=schdDataSize;
			
			BDBG_MSG(("Memory allocation for scheduler data: Address = 0x%08x, Size = 0x%08x",
					hDsp->memInfo.scheduler_data_ptr, schdDataSize));
		}
		else
		{
			hDsp->memInfo.scheduler_data_ptr = BRAP_P_INVALID_DRAM_ADDRESS;
		}
	
		hDsp->memInfo.fsBufBasePtr[0]=baseAddr+bufferUsed;
		hDsp->memInfo.actualFwDwnldSize=bufferUsed;
		bufferUsed+=sDecModeMemReq.ui32FsSize + sDecModeMemReq.ui32FwCodeSize 
					+sPtModeMemReq.ui32FwCodeSize;

		/* Setting Unused Variables of memInfo to Invalid*/
		for (count = 1; count < BRAP_DSP_P_MAX_NUM_FS_CTXT_IN_FS_EXE_BUF ; count++)
		{
			hDsp->memInfo.fsBufBasePtr[count]=BRAP_P_INVALID_DRAM_ADDRESS;
			hDsp->memInfo.fsBufSize[count] = BRAP_DSP_P_CTXT_FS_EXE_SIZE;
		}
		for (count = 0; count < BRAP_RM_P_MAX_DECODE_SUPPORTED; count++)
		{
			hDsp->memInfo.ctxtFwBufBasePtr[count]=BRAP_P_INVALID_DRAM_ADDRESS;
			hDsp->memInfo.ctxtFwBufSize[count] = 0;
		}
		for (count = 0; count < BRAP_RM_P_MAX_PASSTHRU_SUPPORTED; count++)
		{
			hDsp->memInfo.ctxtFwBufSize[count+BRAP_DSP_P_PASSTHRU_CTXT_BUFFER] = 0;
			hDsp->memInfo.ctxtFwBufBasePtr[count+BRAP_DSP_P_PASSTHRU_CTXT_BUFFER] = BRAP_P_INVALID_DRAM_ADDRESS;
		}
		
#if (BRAP_7401_FAMILY == 1)
    	hDsp->memInfo.commonPpBufSize = sPpMemReq.ui32FwCodeSize;
		if(sPpMemReq.ui32FwCodeSize)
		{
			hDsp->memInfo.commonPpBufPtr  =baseAddr+bufferUsed;
			bufferUsed+=sPpMemReq.ui32FwCodeSize;
    		BDBG_MSG(("Memory allocation for post processing buffer: Address = 0x%08x, Size = 0x%08x",
        		hDsp->memInfo.commonPpBufPtr, hDsp->memInfo.commonPpBufSize));    
		}
		else
		{
			hDsp->memInfo.commonPpBufPtr = BRAP_P_INVALID_DRAM_ADDRESS;
		}
		hDsp->memInfo.actualFwDwnldSize += sPpMemReq.ui32FwCodeSize;
#endif

	}
	else
	{
#endif
		if (BRAP_DSP_P_CTXT_FS_EXE_SIZE) 
		{
			hDsp->memInfo.fsBufBasePtr[0] = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
					BRAP_DSP_P_CTXT_FS_EXE_SIZE * BRAP_DSP_P_MAX_NUM_FS_CTXT_IN_FS_EXE_BUF,
					2,
					0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
											
												);
			if (hDsp->memInfo.fsBufBasePtr[0]==BRAP_P_INVALID_DRAM_ADDRESS)
				goto alloc_framesync_buf_failed;
				
			/* Framesync firmware context buffer */
			for (count = 0; count < BRAP_DSP_P_MAX_NUM_FS_CTXT_IN_FS_EXE_BUF ; count++)
			{
				hDsp->memInfo.fsBufSize[count] = BRAP_DSP_P_CTXT_FS_EXE_SIZE;
				hDsp->memInfo.fsBufBasePtr[count] = hDsp->memInfo.fsBufBasePtr[0]  +
					count * BRAP_DSP_P_CTXT_FS_EXE_SIZE;
				BDBG_MSG(("Memory allocation for frame sync buffer ctxt%d: Address = 0x%08x, Size = 0x%08x",
				count, hDsp->memInfo.fsBufBasePtr[count], hDsp->memInfo.fsBufSize[count]));
			}
		}
		else
		{
			for (count = 0; count < BRAP_DSP_P_MAX_NUM_FS_CTXT_IN_FS_EXE_BUF ; count++)
			{
				hDsp->memInfo.fsBufBasePtr[count]=BRAP_P_INVALID_DRAM_ADDRESS;
				hDsp->memInfo.fsBufSize[count] = BRAP_DSP_P_CTXT_FS_EXE_SIZE;
			}
		}
		if (sDecModeMemReq.ui32FwCodeSize)
		{
			hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_DECODE_CTXT_BUFFER] = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
															sDecModeMemReq.ui32FwCodeSize * BRAP_RM_P_MAX_DECODE_SUPPORTED,
															2,
															0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
											
												);


			if (hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_DECODE_CTXT_BUFFER] ==BRAP_P_INVALID_DRAM_ADDRESS)
				goto alloc_decode_buf_failed;
			
			/* FW Decode executables firmware context buffer */
			for (count = 0; count < BRAP_RM_P_MAX_DECODE_SUPPORTED; count++)
			{
				hDsp->memInfo.ctxtFwBufSize[count] = sDecModeMemReq.ui32FwCodeSize;
				hDsp->memInfo.ctxtFwBufBasePtr[count] = hDsp->memInfo.ctxtFwBufBasePtr[0]  +
				count * sDecModeMemReq.ui32FwCodeSize;
				BDBG_MSG(("Memory allocation for Decode Context Fw Buffer ctxt%d: Address = 0x%08x, Size = 0x%08x",
				count, hDsp->memInfo.ctxtFwBufBasePtr[count], hDsp->memInfo.ctxtFwBufSize[count]));
			}
		}
		else
		{
			for (count = 0; count < BRAP_RM_P_MAX_DECODE_SUPPORTED; count++)
			{
				hDsp->memInfo.ctxtFwBufBasePtr[count]=BRAP_P_INVALID_DRAM_ADDRESS;
				hDsp->memInfo.ctxtFwBufSize[count] = 0;
			}
				
		}

		/* Pass thru context buffer */
		if (sPtModeMemReq.ui32FwCodeSize)
		{
			hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_PASSTHRU_CTXT_BUFFER] = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
															sPtModeMemReq.ui32FwCodeSize * BRAP_RM_P_MAX_PASSTHRU_SUPPORTED,
															2,
															0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
											
												);
		
			if (hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_PASSTHRU_CTXT_BUFFER] ==BRAP_P_INVALID_DRAM_ADDRESS)
				goto alloc_passthru_buf_failed;

			/* FW Decode executables firmware context buffer */
			for (count = 0; count < BRAP_RM_P_MAX_PASSTHRU_SUPPORTED; count++)
			{
				hDsp->memInfo.ctxtFwBufSize[count+BRAP_DSP_P_PASSTHRU_CTXT_BUFFER] = sPtModeMemReq.ui32FwCodeSize;
				hDsp->memInfo.ctxtFwBufBasePtr[count+BRAP_DSP_P_PASSTHRU_CTXT_BUFFER] = hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_PASSTHRU_CTXT_BUFFER]  +
					count * sPtModeMemReq.ui32FwCodeSize;
				BDBG_MSG(("Memory allocation for Pass-thru Context Fw Buffer ctxt%d: Address = 0x%08x, Size = 0x%08x",
				count, hDsp->memInfo.ctxtFwBufBasePtr[count+BRAP_DSP_P_PASSTHRU_CTXT_BUFFER],
				hDsp->memInfo.ctxtFwBufSize[count+BRAP_DSP_P_PASSTHRU_CTXT_BUFFER]));
			}	
		
		}
		else
		{
			for (count = 0; count < BRAP_RM_P_MAX_PASSTHRU_SUPPORTED; count++)
			{
				hDsp->memInfo.ctxtFwBufSize[count+BRAP_DSP_P_PASSTHRU_CTXT_BUFFER] = 0;
				hDsp->memInfo.ctxtFwBufBasePtr[count+BRAP_DSP_P_PASSTHRU_CTXT_BUFFER] = BRAP_P_INVALID_DRAM_ADDRESS;
			}
		}

#if (BCHP_7411_VER) || (BRAP_7401_FAMILY == 1)
		/* Common post-processing algorithms */
		hDsp->memInfo.commonPpBufSize = sPpMemReq.ui32FwCodeSize;
		if (sPpMemReq.ui32FwCodeSize)
		{
			hDsp->memInfo.commonPpBufPtr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
														hDsp->memInfo.commonPpBufSize,
														2,
														0
#if (BRAP_SECURE_HEAP == 1 ) 											
												,false
#endif												
												);
			if (hDsp->memInfo.commonPpBufPtr==BRAP_P_INVALID_DRAM_ADDRESS)
				goto alloc_pp_buf_failed;

			BDBG_MSG(("Memory allocation for post processing buffer: Address = 0x%08x, Size = 0x%08x",
				hDsp->memInfo.commonPpBufPtr, hDsp->memInfo.commonPpBufSize));
		}
		else
		{
			hDsp->memInfo.commonPpBufPtr = BRAP_P_INVALID_DRAM_ADDRESS;
		}
#endif	
#ifndef BCHP_7411_VER
	}
#endif

	BKNI_Free(encMemInfo);

	return BERR_SUCCESS; 

	/* Memory allocation failed. Free all allocated memory and return error. */
#if (BCHP_7411_VER) || (BRAP_7401_FAMILY == 1)    
alloc_pp_buf_failed:
#if (BRAP_SECURE_HEAP == 1 )
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_PASSTHRU_CTXT_BUFFER],false);
#else
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_PASSTHRU_CTXT_BUFFER]);
#endif
#endif

/*#endif*/
#ifndef BCHP_7411_VER

#if (BRAP_SECURE_HEAP == 1 )

alloc_passthru_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_DECODE_CTXT_BUFFER],false);
	}
alloc_decode_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.fsBufBasePtr[0],false);
	}
alloc_framesync_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.mitDSPtr,false);
	}
alloc_total_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==true)
	{
		BRAP_P_Free(hDsp->hRap,(void *) hDsp->memInfo.mitDSPtr,false);
	}
alloc_mit_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.decConfigBufPtr[0],false);
alloc_decconfig_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.interFrameBufBasePtr[0],false);
alloc_interframe_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scratchSpaceBufPtr,false);
alloc_scratch_space_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.apmScratchBufPtr,false);
alloc_apm_scratch_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scheduler_data_ptr,false);
	}
alloc_scheduler_data_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scheduler_code_ptr,false);
	}
alloc_scheduler_code_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.iboot_ptr,false);
	}
alloc_iboot_buf_failed:
	BKNI_Free(encMemInfo);  
	return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);


#else

alloc_passthru_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_DECODE_CTXT_BUFFER]);
	}
alloc_decode_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.fsBufBasePtr[0]);
	}
alloc_framesync_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.mitDSPtr);
	}
alloc_total_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==true)
	{
		BRAP_P_Free(hDsp->hRap,(void *) hDsp->memInfo.mitDSPtr);
	}
alloc_mit_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.decConfigBufPtr[0]);

alloc_decconfig_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.interFrameBufBasePtr[0]);

alloc_interframe_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scratchSpaceBufPtr);

alloc_scratch_space_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.apmScratchBufPtr);

alloc_apm_scratch_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scheduler_data_ptr);
	}
alloc_scheduler_data_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scheduler_code_ptr);
	}
alloc_scheduler_code_buf_failed:
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.iboot_ptr);
	}
alloc_iboot_buf_failed:
	BKNI_Free(encMemInfo); 
	return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);

#endif

#else

#if (BRAP_SECURE_HEAP == 1 )

alloc_passthru_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_DECODE_CTXT_BUFFER],false);
alloc_decode_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.fsBufBasePtr[0],false);

alloc_framesync_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.mitDSPtr,false);
alloc_mit_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.decConfigBufPtr[0],false);
alloc_decconfig_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.interFrameBufBasePtr[0],false);
alloc_interframe_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scratchSpaceBufPtr,false);
alloc_scratch_space_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.apmScratchBufPtr,false);
alloc_apm_scratch_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scheduler_data_ptr,false);

alloc_scheduler_data_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scheduler_code_ptr,false);
alloc_scheduler_code_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.iboot_ptr,false);


#else

alloc_passthru_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_DECODE_CTXT_BUFFER]);
alloc_decode_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.fsBufBasePtr[0]);

alloc_framesync_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.mitDSPtr);
alloc_mit_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.decConfigBufPtr[0]);
alloc_decconfig_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.interFrameBufBasePtr[0]);
alloc_interframe_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scratchSpaceBufPtr);
alloc_scratch_space_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.apmScratchBufPtr);
alloc_apm_scratch_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scheduler_data_ptr);

alloc_scheduler_data_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scheduler_code_ptr);
alloc_scheduler_code_buf_failed:
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.iboot_ptr);

#endif	

alloc_iboot_buf_failed:
	BKNI_Free(encMemInfo);
	return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);

#endif
}
#endif /* old MIT */



static void BRAP_DSP_P_FreeMem(BRAP_DSP_Handle hDsp)
{
	BDBG_ASSERT(hDsp);


	/* Free all the memory allocated in BRAP_DSP_P_AllocMem */
#ifndef BCHP_7411_VER
if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
#endif	
#if (BRAP_SECURE_HEAP == 1 )	
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_PASSTHRU_CTXT_BUFFER],false);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_DECODE_CTXT_BUFFER],false);
#if (BRAP_DSP_P_7401_NEWMIT==1)
    	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.arrayOfConfigBufPtr[0],false);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ui32PpFwBufPtr,false);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ui32PpInterFrameBufBasePtr[0], false);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.interStageBufBasePtr[0], false);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.interStageInterfaceBufBasePtr[0], false);   
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ui32PpScratchBufPtr, false); 
#else
        BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.commonPpBufPtr,false);
#endif

		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.fsBufBasePtr[0],false);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.mitDSPtr,false);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.decConfigBufPtr[0],false);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.interFrameBufBasePtr[0],false);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scratchSpaceBufPtr,false);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.apmScratchBufPtr,false);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scheduler_data_ptr,false);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scheduler_code_ptr,false);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.iboot_ptr,false);
	    BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ui32DwnldableSchedulerPtr,false);				
#else
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_PASSTHRU_CTXT_BUFFER]);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ctxtFwBufBasePtr[BRAP_DSP_P_DECODE_CTXT_BUFFER]);

#if (BRAP_DSP_P_7401_NEWMIT==1)
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ui32PpFwBufPtr);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ui32PpInterFrameBufBasePtr[0]);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.interStageBufBasePtr[0]);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.interStageInterfaceBufBasePtr[0]);   
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.ui32PpScratchBufPtr, false);              
#else
        BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.commonPpBufPtr);
#endif		

		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.fsBufBasePtr[0]);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.mitDSPtr);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.decConfigBufPtr[0]);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.interFrameBufBasePtr[0]);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scratchSpaceBufPtr);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.apmScratchBufPtr);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scheduler_data_ptr);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scheduler_code_ptr);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.iboot_ptr);
#endif

#ifndef BCHP_7411_VER
	}
	else
	{
#if (BRAP_SECURE_HEAP == 1 )
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.fwDwnldOffset,false);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.mitDSPtr,false);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.decConfigBufPtr[0],false);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.interFrameBufBasePtr[0],false);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scratchSpaceBufPtr,false);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.apmScratchBufPtr,false);
#else
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.fwDwnldOffset);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.mitDSPtr);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.decConfigBufPtr[0]);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.interFrameBufBasePtr[0]);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.scratchSpaceBufPtr);
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.apmScratchBufPtr);
#endif
	}
#endif	

	return; 

}


/*
	This function uses the Image Interface to download the firmware binary.
*/
BERR_Code BRAP_DSP_P_CopyFWImageToMem (BRAP_DSP_Handle hDsp, uint32_t memAdr, uint32_t firmware_id)
{
	void *image;	
	BERR_Code rc;	
	const void *data;	
	unsigned i;	
	const BIMG_Interface *iface = hDsp->hRap->sSettings.pImgInterface;	
	void *context = hDsp->hRap->sSettings.pImgContext;
	uint32_t size, number_of_chunks, len;
	BDBG_ENTER (BRAP_DSP_P_CopyFWImageToMem);  

	rc = iface->open (context, &image, firmware_id);
	if (rc != BERR_SUCCESS) 
	{		
		BDBG_ERR(("BRAP_DSP_P_CopyFWImageToMem: Error in Opening the Image Interface"));
        BDBG_ERR(("ImgContext=%d, firmware_id=%d, 0x%x", context, firmware_id, firmware_id));
		return BERR_TRACE(rc);

	}


	rc = iface->next(image, 0, &data, 8);
	if (rc != BERR_SUCCESS) 
	{		
		BDBG_ERR(("\nError in fetching next chunk in Image Interface\n"));;	
		iface->close(image);
		return BERR_TRACE(rc);
	}

	size =((uint32_t *) data)[0];
	number_of_chunks = ((uint32_t *) data)[1];

	for (i=1; i<=number_of_chunks; i++)
	{
		/* The implementation of image interface has to be such that there is one header array and then there are firmware binary
		arrays each having bytes of size BRAP_IMG_CHUNK_SIZE. But the last array most probably will not have a size equal to exactly BRAP_IMG_CHUNK_SIZE.
		So we are testing for the last chunk here and getting the size based on the total firmware binary size and number of chunks*/
		
		len = (i==number_of_chunks) ?  size-((i-1)*BRAP_IMG_CHUNK_SIZE): BRAP_IMG_CHUNK_SIZE;
		rc = iface->next(image, i, &data, len);
		if (rc != BERR_SUCCESS) 
		{		
			BDBG_ERR(("\nError in fetching next chunk in Image Interface\n"));;	
			iface->close(image);
			return BERR_TRACE(rc);
		}

		/* With the pointer got from the Image interface, we will call downloaddspfw function and pass the address offsetted by the chunk size and the current 
		chunk */
		BRAP_P_DownloadDspFwInMem((BARC_Handle)hDsp->hRegister, (uint32_t *) data, memAdr+BRAP_IMG_CHUNK_SIZE*(i-1), len);
	}	

	iface->close(image);

	BDBG_LEAVE (BRAP_DSP_P_CopyFWImageToMem);  
	return rc;

}

static BERR_Code BRAP_DSP_P_CopyFWToMem(BRAP_DSP_Handle hDsp) 
{
	BERR_Code rc = BERR_SUCCESS;

#if (BCHP_7411_VER) || (BRAP_7401_FAMILY == 1) ||(BCHP_CHIP == 7400 )
	int i;
	BRAP_DSP_P_ExecID sExecID;
	bool bSupportedPpAlgo[BRAP_DSPCHN_PP_Algo_eMax];
#endif	
	
	uint32_t size, physAddress;
    unsigned char *mitPtr;
	BDBG_ENTER (BRAP_DSP_P_CopyFWToMem);  

	BRAP_DSP_P_CopyFWImageToMem (hDsp, hDsp->memInfo.iboot_ptr, BRAP_IMG_IBOOT_ID); 
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_IBOOT_ID, &size);
	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.iboot_ptr, &physAddress);
	BDBG_MSG(("Downloading iboot at Address = 0x%08x, Physical Address=0x%08x, Size = 0x%08x", 
					hDsp->memInfo.iboot_ptr, physAddress, size));


	BRAP_DSP_P_CopyFWImageToMem (hDsp, hDsp->memInfo.scheduler_code_ptr, BRAP_IMG_SCHEDULER_CODE_ID);
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_SCHEDULER_CODE_ID, &size);
	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.scheduler_code_ptr, &physAddress);
	BDBG_MSG(("Downloading scheduler code at Address = 0x%08x, Physical Address=0x%08x, Size = 0x%08x",
					hDsp->memInfo.scheduler_code_ptr, physAddress, size));
	
	BRAP_DSP_P_CopyFWImageToMem (hDsp, hDsp->memInfo.scheduler_data_ptr, BRAP_IMG_SCHEDULER_DATA_ID);
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_SCHEDULER_DATA_ID, &size);
	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.scheduler_data_ptr, &physAddress);    
	BDBG_MSG(("Downloading scheduler data at Address = 0x%08x, Physical Address=0x%08x, Size = 0x%08x",
					hDsp->memInfo.scheduler_data_ptr, physAddress, size));

    /* Downloadble Scheduler Code */
    BRAP_DSP_P_CopyFWImageToMem (hDsp, hDsp->memInfo.ui32DwnldableSchedulerPtr, BRAP_IMG_SCHEDULER_DNLD_CODE_ID);
    BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_SCHEDULER_DNLD_CODE_ID, &size);	
    BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)(hDsp->memInfo.ui32DwnldableSchedulerPtr), &physAddress);
	BDBG_MSG(("Downloading Downloadable scheduler code at Address = 0x%08x, Physical Address=0x%08x, Size = 0x%08x",
					hDsp->memInfo.ui32DwnldableSchedulerPtr, physAddress, size));
	
    /* Update Downloadable Schedular address in MIT */
    mitPtr = (unsigned char *)hDsp->memInfo.mitDSPtr + (4+4);
    BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4); 
    
    mitPtr += 4;
    BRAP_DSP_P_WriteMIT(mitPtr, size, 4);   

#if (BCHP_7411_VER) || (BRAP_7401_FAMILY == 1) ||(BCHP_CHIP == 7400 )
	/* Download firmware into common post processing buffer here */
	rc = BRAP_P_GetSelectedPpAlgos(hDsp->hRap, bSupportedPpAlgo);
	if (rc!=BERR_SUCCESS)
		return BERR_TRACE(rc);
	
	for (i =0; i < BRAP_DSPCHN_PP_Algo_eMax; i++) {
		if (bSupportedPpAlgo[i]) {
			/* Initialize sExecId struct */
			BRAP_DSP_P_GetPpExecIds((BRAP_DSPCHN_PP_Algo) i, &sExecID);

            
			rc = BRAP_DSP_P_DownloadExecFW(hDsp, 
					(BRAP_DSPCHN_DecodeMode)0 /* Don't care */, (BRAP_DSPCHN_AudioType)0 /* Don't care */, (BRAP_DSPCHN_PP_Algo) i, false, true, 0 /* Don't care */, sExecID);

			if (rc!=BERR_SUCCESS)
				return BERR_TRACE(rc);
		}
	}
#endif	

#if BRAP_DSP_P_DBG_MIT_TABLE==1 
	BRAP_DSP_P_PrintMit(hDsp);
#endif	
	BDBG_LEAVE (BRAP_DSP_P_CopyFWToMem);  
	return rc;
}

BERR_Code BRAP_DSP_P_InitHardware(BRAP_DSP_Handle hDsp)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t	regVal=0, size;
	uint8_t		dmaDataType, swapType;
	uint32_t	index, dspCfgRamSize, physAddress;
	uint32_t count = 0;

	/* Reset DSP processor and its peripherals */
	/*regVal = BRAP_Read32(hDsp->hRegister,BCHP_AUD_DSP_MISC_SOFT_RESET);*/
	regVal=BRAP_DSP_P_ResetHardware(hDsp);
	


	/* Withdraw only RESET0_B reset */
	regVal = (regVal & ~(BCHP_MASK(AUD_DSP_MISC_SOFT_RESET,RESET0_B)))|
				(BCHP_FIELD_DATA(AUD_DSP_MISC_SOFT_RESET,RESET0_B,1));
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_MISC_SOFT_RESET, regVal);
	BDBG_MSG(("\n**** Soft reset: reset0_b withdrawn = 0x%x",regVal));

#if defined ( BCHP_7411_VER ) || (BRAP_7401_FAMILY == 1)
	/* Select Host boot mode */
	regVal = 0;
	regVal = (regVal & ~(BCHP_MASK(AUD_DSP_CTRL0_CORE_BOOT_CONFIG, BOOT_CODE_SEL)))|
				(BCHP_FIELD_DATA(AUD_DSP_CTRL0_CORE_BOOT_CONFIG, BOOT_CODE_SEL,1));
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_CORE_BOOT_CONFIG,regVal);
	BDBG_MSG(("\n**** Boot Config = 0x%x ****",regVal));
#endif	

	/* Initialize Mailbox5 register to zero */
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_MAILBOX5,BRAP_DSP_P_PREBOOT_MAILBOX_PATTERN);
	regVal = BRAP_Read32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_MAILBOX5);
	BDBG_MSG(("**** Mailbox5 = 0x%x ****",regVal));

	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_MAILBOX0,BRAP_DSP_P_PREBOOT_MAILBOX_PATTERN);
	regVal = BRAP_Read32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_MAILBOX0);
	BDBG_MSG(("**** Mailbox0 = 0x%x ****",regVal));

#if defined ( BCHP_7411_VER ) || (BRAP_7401_FAMILY == 1)
	/* Initialize entire DSP configuration RAM to zero */
	dspCfgRamSize = BCHP_AUD_DSP_CFG0_SPDIF_PARSER_PTR_CXT2 - BCHP_AUD_DSP_CFG0_MASTER_IT_POINTER;
#else
	/* Initialize entire DSP configuration RAM to zero */
	dspCfgRamSize = BCHP_AUD_DSP_CFG0_SPDIF_PARSER_PTR_CXT5 - BCHP_AUD_DSP_CFG0_MASTER_IT_POINTER;
#endif
	for (index = 0; index < dspCfgRamSize; index+=4) {
		BRAP_Write32(hDsp->hRegister, BCHP_AUD_DSP_CFG0_MASTER_IT_POINTER + index, 0x0);
		}

	/* Initialize master index table pointer */	
	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.mitDSPtr, &physAddress);
	BRAP_Write32(hDsp->hRegister, BCHP_AUD_DSP_CFG0_MASTER_IT_POINTER, physAddress);
#ifndef BCHP_7411_VER /* For chips other than 7411 */
	/* Initialize master index table pointer 2, for encoder */	
	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.encmitDSPtr, &physAddress);
	BRAP_Write32(hDsp->hRegister, BCHP_AUD_DSP_CFG0_MASTER_IT_POINTER2, physAddress);
#endif

#if defined ( BCHP_7411_VER ) || (BRAP_7401_FAMILY == 1)
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_RBASE_ADDR0, BCHP_AUD_FMM_MISC_REG_START + BRAP_DSP_P_CHIP_PHYSICAL_OFFSET);
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_RBASE_ADDR1, BCHP_AUD_FMM_BF_CTRL_REG_START + BRAP_DSP_P_CHIP_PHYSICAL_OFFSET);

	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_RBASE_ADDR3,BCHP_AUD_FMM_OP_CTRL_REG_START + BRAP_DSP_P_CHIP_PHYSICAL_OFFSET);
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_RBASE_ADDR4,BCHP_AUD_FMM_PLL0_REG_START + BRAP_DSP_P_CHIP_PHYSICAL_OFFSET);
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_RBASE_ADDR6,BCHP_AUD_FMM_DP_CTRL0_REG_START + BRAP_DSP_P_CHIP_PHYSICAL_OFFSET);

	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_RBASE_ADDR10,BCHP_AUD_FMM_MS_CTRL_REG_START + 0x800 + BRAP_DSP_P_CHIP_PHYSICAL_OFFSET);
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_RBASE_ADDR11,BCHP_AUD_DSP_CTRL0_REG_START + BRAP_DSP_P_CHIP_PHYSICAL_OFFSET);    

	/* NO OP and jump instruction */
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_EXT_IMEM0,0x07ffadf7);
#endif	

#if defined ( BCHP_7411_VER )
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_RBASE_ADDR2,BCHP_AUD_FMM_MS_ESR_REG_START + BRAP_DSP_P_CHIP_PHYSICAL_OFFSET); 
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_RBASE_ADDR7,BCHP_XPT_AVM_REG_START + BRAP_DSP_P_CHIP_PHYSICAL_OFFSET);
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_RBASE_ADDR8,BCHP_XPT_AVM_REG_START + 0x800 + BRAP_DSP_P_CHIP_PHYSICAL_OFFSET);
#elif (BRAP_7401_FAMILY == 1)
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_RBASE_ADDR2,BCHP_AUD_FMM_MS_CTRL_REG_START + BRAP_DSP_P_CHIP_PHYSICAL_OFFSET); 
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_RBASE_ADDR5,BCHP_HIFIDAC_CTRL0_REG_START+ BRAP_DSP_P_CHIP_PHYSICAL_OFFSET);
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_RBASE_ADDR7,BCHP_XPT_RAVE_REG_START + BRAP_DSP_P_CHIP_PHYSICAL_OFFSET);
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_RBASE_ADDR8,BCHP_XPT_RAVE_REG_START + 0x800 + BRAP_DSP_P_CHIP_PHYSICAL_OFFSET);
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_RBASE_ADDR9,BCHP_HIFIDAC_CTRL0_REG_START + BRAP_DSP_P_CHIP_PHYSICAL_OFFSET);
#endif
	
	/* NO OP and jump instruction */
/*	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_EXT_IMEM0,0x07ffadf7);*/
	
	/* Program default block out counters for Raptor */
	regVal = 0x0;
#ifdef BCHP_7411_VER
	/* Block out count for Raptor-FMM */
	regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_MISC_SCB_BLOCKOUT_CTRL, 
					SCB_CLIENT1_BLOCKOUT, BRAP_DSP_P_FMM_BLOCKOUT_COUNT);
	/* Block out count for DSP */
	regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_MISC_SCB_BLOCKOUT_CTRL,
					SCB_CLIENT0_BLOCKOUT, BRAP_DSP_P_DSP_BLOCKOUT_COUNT);
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_MISC_SCB_BLOCKOUT_CTRL, regVal);
	regVal = 0x0;
	regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_MISC_SCB_MONITOR_CTRL, SCB_MONITOR_SELECT, DSP);
	regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_MISC_SCB_MONITOR_CTRL, SCB_LATENCY_LIMIT, 
						BRAP_DSP_P_SCB_LATENCY_LIMIT);
	BRAP_Write32(hDsp->hRegister, BCHP_AUD_DSP_MISC_SCB_MONITOR_CTRL, regVal); 
#endif 

	/* Withdraw SCBDMA reset before DMA */
	regVal = BRAP_Read32(hDsp->hRegister,BCHP_AUD_DSP_MISC_SOFT_RESET);
	regVal = (regVal & ~(BCHP_MASK(AUD_DSP_MISC_SOFT_RESET,RESET_SCBDMA0_B)))|
				(BCHP_FIELD_DATA(AUD_DSP_MISC_SOFT_RESET,RESET_SCBDMA0_B,1));
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_MISC_SOFT_RESET, regVal);
	BDBG_MSG(("\n**** Soft reset: SCBDMA withdrawn = 0x%x ****",regVal));

	/* Boot the processor */
#ifndef BCHP_7411_VER
	dmaDataType = 2;
	swapType = 1;
#else
	dmaDataType = 0;
	swapType = 0;
#endif

#ifndef MP3ENC_STANDALONE
	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.iboot_ptr, &physAddress);
	
	if (BERR_SUCCESS != (err = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_IBOOT_ID, &size)))
		return BERR_TRACE(err);
	err = BRAP_DSP_P_DmaIn(hDsp, dmaDataType, swapType, size, physAddress, 0);

	/* Download scheduler code into processor */
	/* Note: Scheduler code should always start at 512 even if iboot < 512 */
	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)(hDsp->memInfo.scheduler_code_ptr + 16), &physAddress);

	if (BERR_SUCCESS != (err = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_SCHEDULER_CODE_ID, &size)))
		return BERR_TRACE(err);
	err = BRAP_DSP_P_DmaIn(hDsp, dmaDataType, swapType, size,  physAddress, 512);

	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.scheduler_data_ptr, &physAddress);

	if (BERR_SUCCESS != (err = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_SCHEDULER_DATA_ID, &size)))
		return BERR_TRACE(err);

	err = BRAP_DSP_P_DmaIn(hDsp, dmaDataType, swapType, size, physAddress, 0x8000000);
#else
	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.iboot_ptr, &physAddress);
	
	if (BERR_SUCCESS != (err = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_ENC_IBOOT_ID, &size)))
			return BERR_TRACE(err); 
	err = BRAP_DSP_P_DmaIn(hDsp, dmaDataType, swapType, size, physAddress, 0);

	/* Download scheduler code into processor */
	/* Note: Scheduler code should always start at 512 even if iboot < 512 */
	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)(hDsp->memInfo.encscheduler_code_ptr + 16), &physAddress);

	if (BERR_SUCCESS != (err = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_ENC_SCHEDULER_CODE_ID, &size)))
			return BERR_TRACE(err); 
#if (BRAP_7401_FAMILY == 1)
/*	err = BRAP_DSP_P_DmaIn(hDsp, dmaDataType, swapType, size,  physAddress, 0x5400);*/
/* Changed due to updated scheduler */
	err = BRAP_DSP_P_DmaIn(hDsp, dmaDataType, swapType, size,  physAddress, 0x6000);
#elif (BCHP_CHIP == 7400)
/*	err = BRAP_DSP_P_DmaIn(hDsp, dmaDataType, swapType, size,  physAddress, 0x200);*/
	err = BRAP_DSP_P_DmaIn(hDsp, dmaDataType, swapType, size,  physAddress, 0x6000);
#endif
#endif

#ifdef MP3ENC_STANDALONE
	/* Provide the context */
	/* Program the Algo Type */
	BRAP_Write32 (hDsp->hRegister, BCHP_AUD_DSP_CFG0_ALGO_ID_CXT0, 0x0000000F);	
	BRAP_Write32 (hDsp->hRegister, BCHP_AUD_DSP_CFG0_ALGO_ID_CXT1, 0x00004000);	
#endif

	/* Withdraw processor resets */
	regVal = BRAP_Read32(hDsp->hRegister,BCHP_AUD_DSP_MISC_SOFT_RESET);
	regVal = (regVal & ~(BCHP_MASK(AUD_DSP_MISC_SOFT_RESET,RESET_PROC0_B)))|
				(BCHP_FIELD_DATA(AUD_DSP_MISC_SOFT_RESET,RESET_PROC0_B,1));
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_MISC_SOFT_RESET, regVal);
	BDBG_MSG(("\n**** Soft reset: reset withdrawn = 0x%x ****",regVal));

	regVal = BRAP_DSP_P_PREBOOT_MAILBOX_PATTERN;
	while (regVal == BRAP_DSP_P_PREBOOT_MAILBOX_PATTERN)  {
		regVal = BRAP_Read32(hDsp->hRegister, BCHP_AUD_DSP_CTRL0_MAILBOX5);
		count++;
		if (count==BRAP_DSP_P_BOOT_TIMEOUT_COUNT) break;
		/* TODO: Add appropriate sleep here */
	}
	BDBG_MSG(("**** Mailbox5 = 0x%x ****",regVal));
	return err;

}

static uint32_t BRAP_DSP_P_ResetHardware(BRAP_DSP_Handle hDsp)
{
	uint32_t regVal;

	/* Reset DSP processor and its peripherals */

	regVal = 0;
	regVal = (regVal & ~(BCHP_MASK(AUD_DSP_MISC_SOFT_RESET,RESET_SCBDMA0_B)))|
				(BCHP_FIELD_DATA(AUD_DSP_MISC_SOFT_RESET,RESET_SCBDMA0_B,0));
	regVal = (regVal & ~(BCHP_MASK(AUD_DSP_MISC_SOFT_RESET,RESET_PROC0_B)))|
				(BCHP_FIELD_DATA(AUD_DSP_MISC_SOFT_RESET,RESET_PROC0_B,0));
	regVal = (regVal & ~(BCHP_MASK(AUD_DSP_MISC_SOFT_RESET,RESET0_B)))|
				(BCHP_FIELD_DATA(AUD_DSP_MISC_SOFT_RESET,RESET0_B,0));
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_MISC_SOFT_RESET, regVal);
	BDBG_MSG(("\n**** Soft reset: reset all = 0x%x",regVal));
	return regVal;
}

BERR_Code BRAP_DSP_P_GetFWSize (BRAP_DSP_Handle hDsp, uint32_t firmware_id, uint32_t *size)
{
	void *image;	
	BERR_Code rc;	
	const void *data;	
	const BIMG_Interface *iface = hDsp->hRap->sSettings.pImgInterface;	
	void *context = hDsp->hRap->sSettings.pImgContext;

	rc = iface->open (context, &image, firmware_id);

	if (rc != BERR_SUCCESS) 
	{		
		BDBG_ERR(("\nBRAP_DSP_P_GetFWSize: Error in Opening the Image Interface \n"));;	
        BDBG_ERR(("ImgContext=%d, firmware_id=%d, 0x%x", context, firmware_id));
		iface->close(image);
		return BERR_TRACE(rc);
	}

	rc = iface->next(image, 0, &data, 8);

	if (rc != BERR_SUCCESS) 
	{		
		BDBG_ERR(("\nError in fetching next chunk in Image Interface\n"));;	
		iface->close(image);
		return BERR_TRACE(rc);
	}

	*size =((uint32_t *) data)[0];

	iface->close(image);
	return rc;
	
}
#ifndef BCHP_7411_VER

/* BRAP_DSP_P_DownloadAllSupportedFirmware: This function Downloads FW executables
 * in contiguous memory locations.
 * The Flow of download is as follows:
 * For each Supported Audio Type
 *		- Download Framesync Exec
 *		- Download Decode Code Exec
 *		- Download Decode Table Exec
 *		- Download Postprocessing Algo Code for Decode
 *		- Download Postprocessing Algo Table for Decode
 *		- Download Passthru Code Exec
 *		- Download Passthru Table Exec
 *
 * And it also updates the MIT entries.
 */

static BERR_Code
BRAP_DSP_P_DownloadAllSupportedFirmware(
								BRAP_DSP_Handle hDsp)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t	i=0,j=0,algoId,fwId,allocFwDwnldSize;
	uint32_t tblPtr=0,algoSize=0,bufSizeUsed=0,algoPtr=0,tblSize=0,mitPtr;
	BRAP_DSPCHN_AudioType eAudType;
	BRAP_DSP_P_ExecID sExecID;
	BRAP_DSP_P_ExecID sExecIDPt;

#if (BRAP_DSP_P_7401_NEWMIT==1)	
	BRAP_P_Dec_AlgoMemRequirement sAlgoMemReq;
#else  
	BRAP_P_AlgoMemRequirement sAlgoMemReq;
#endif    


	allocFwDwnldSize=hDsp->memInfo.allocFwDwnldSize;

	/* Downloading Frame Sync, for	each supported Audio types*/
	for (j = 0; j < BRAP_MAX_AUDIO_TYPES; j++) 
	{
		/* If algorithm is not selected by application, go to next algorithm */
		if (hDsp->hRap->sSettings.bSupportAlgos[j]==false)
			continue;
		
		eAudType=  (BRAP_DSPCHN_AudioType)j;
		err = BRAP_DSP_P_GetExecIds(BRAP_DSPCHN_DecodeMode_eDecode, eAudType,
				&sExecID);
		if(err != BERR_SUCCESS)
		{
			BDBG_ERR(("Error: Aborting Firmware download - %d",err));
			return BERR_TRACE(err);
		}

		for(i=0; i<BRAP_DSP_P_MAX_FS_ALG_IN_A_CTXT; i++)
		{
			/* --------------------------- */
			/* *** Frame Sync firmware *** */
			/* --------------------------------- */	
			fwId = sExecID.fsID[i];
			if(fwId == BAF_INVALID)
			{
				BDBG_MSG(("Invalid FS Id - 0x%x\n",fwId));				
				continue;
			}
			algoSize = hDsp->psMitArrays->fsAlgoSizes[fwId];			
			
			if (bufSizeUsed + algoSize > allocFwDwnldSize)
			{
				BDBG_ERR(("ERROR: size available (%d) less than fsFwSize(%d)",allocFwDwnldSize-bufSizeUsed ,algoSize));
				return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			}

			BDBG_MSG(("Valid FS Id - 0x%x\n , fwSize=%d",fwId,algoSize));
			
			/* Download Frame Sync FW */
			algoPtr = hDsp->memInfo.fsBufBasePtr[0] + bufSizeUsed;
			if (algoSize) {
				err = BRAP_DSP_P_CopyFWImageToMem (hDsp, algoPtr ,
					hDsp->psMitArrays->fsAlgoArrays[fwId]);
				if (err!=BERR_SUCCESS)
					return BERR_TRACE(err);
				BDBG_MSG(("Downloading frame sync algorithm %d at address 0x%x,"
					"size = 0x%x ,i=%d ,j=%d", fwId, algoPtr,algoSize,i,j));
			}
   			/* Update the buf size used */

			bufSizeUsed+=algoSize;
			
			/* Update the MIT Entry */
            mitPtr = /* (unsigned char *) */(hDsp->memInfo.mitDSPtr +
    	        	SIZE_OF_BAF_HOST_INFO_ALLIGNED +
	    	        (fwId * SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED));	

            BRAP_DSP_P_FsUpdateDRAMEntry(hDsp, mitPtr, fwId, algoSize, algoPtr); 
		}/* for  each BRAP_DSP_P_MAX_FS_ALG_IN_A_CTXT */

		/* Download all the FS which appears in transID, but not downloaded */
		if(BRAP_DSPCHN_AudioType_eAac == (BRAP_DSPCHN_AudioType)j)
		{
			fwId = sExecID.transID;
			if(fwId == BAF_INVALID)
			{
				BDBG_MSG(("Invalid Alternate FS Id - 0x%x\n",fwId));				
				continue;
			}
			algoSize = hDsp->psMitArrays->fsAlgoSizes[fwId];			
			
			if (bufSizeUsed + algoSize > allocFwDwnldSize)
			{
				BDBG_ERR(("ERROR: size available (%d) less than fsFwSize(%d)",allocFwDwnldSize-bufSizeUsed ,algoSize));
				return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			}

			BDBG_MSG(("Valid FS Id - 0x%x\n , fwSize=%d",fwId,algoSize));
			
			/* Download Frame Sync FW */
			algoPtr = hDsp->memInfo.fsBufBasePtr[0] + bufSizeUsed;
			if (algoSize) {
				err = BRAP_DSP_P_CopyFWImageToMem (hDsp, algoPtr ,
					hDsp->psMitArrays->fsAlgoArrays[fwId]);
				if (err!=BERR_SUCCESS)
					return BERR_TRACE(err);
				BDBG_MSG(("Downloading frame sync algorithm %d at address 0x%x,"
					"size = 0x%x ", fwId, algoPtr,algoSize));
			}
   			/* Update the buf size used */

			bufSizeUsed+=algoSize;
			
			/* Update the MIT Entry */
            mitPtr = /* (unsigned char *) */(hDsp->memInfo.mitDSPtr +
    	        	SIZE_OF_BAF_HOST_INFO_ALLIGNED +
	    	        (fwId * SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED));	

            BRAP_DSP_P_FsUpdateDRAMEntry(hDsp, mitPtr, fwId, algoSize, algoPtr); 
		}
		
		/* Download all the Passthru FrameSyncs which appears in PassthruMode array of execId, but not in DecodeMode array of execid */
		if(BRAP_DSPCHN_AudioType_eAc3Plus== (BRAP_DSPCHN_AudioType)j)
    		{
        		err = BRAP_DSP_P_GetExecIds(BRAP_DSPCHN_DecodeMode_ePassThru, eAudType,
        				&sExecIDPt);
        		if(err != BERR_SUCCESS)
        		{
        			BDBG_ERR(("Error: Aborting Firmware download - %d",err));
        			return BERR_TRACE(err);
        		}

        		for(i=0; i<BRAP_DSP_P_MAX_FS_ALG_IN_A_CTXT; i++)
        		{
        			/* --------------------------- */
        			/* *** Frame Sync firmware *** */
        			/* --------------------------------- */	
        			fwId = sExecIDPt.fsID[i];
        			if(fwId == BAF_INVALID)
        			{
        				BDBG_MSG(("Invalid FS Id - 0x%x\n",fwId));				
        				continue;
        			}
        			algoSize = hDsp->psMitArrays->fsAlgoSizes[fwId];			
        			
        			if (bufSizeUsed + algoSize > allocFwDwnldSize)
        			{
        				BDBG_ERR(("ERROR: size available (%d) less than fsFwSize(%d)",allocFwDwnldSize-bufSizeUsed ,algoSize));
        				return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        			}

        			BDBG_MSG(("Valid FS Id - 0x%x\n , fwSize=%d",fwId,algoSize));
        			
        			/* Download Frame Sync FW */
        			algoPtr = hDsp->memInfo.fsBufBasePtr[0] + bufSizeUsed;
        			if (algoSize) {
        				err = BRAP_DSP_P_CopyFWImageToMem (hDsp, algoPtr ,
        					hDsp->psMitArrays->fsAlgoArrays[fwId]);
        				if (err!=BERR_SUCCESS)
        					return BERR_TRACE(err);
        				BDBG_MSG(("Downloading frame sync algorithm %d at address 0x%x,"
        					"size = 0x%x ,i=%d ,j=%d", fwId, algoPtr,algoSize,i,j));
        			}
           			/* Update the buf size used */

        			bufSizeUsed+=algoSize;
        			
        			/* Update the MIT Entry */
                    mitPtr = /* (unsigned char *) */(hDsp->memInfo.mitDSPtr +
            	        	SIZE_OF_BAF_HOST_INFO_ALLIGNED +
        	    	        (fwId * SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED));	

                    BRAP_DSP_P_FsUpdateDRAMEntry(hDsp, mitPtr, fwId, algoSize, algoPtr); 
        		}/* for  each BRAP_DSP_P_MAX_FS_ALG_IN_A_CTXT */
    		}
		
		err = BRAP_P_GetAlgoMemRequirement (hDsp->hRap, eAudType, &sAlgoMemReq);
		if (err != BERR_SUCCESS)
			return BERR_TRACE(err);

		/*Downloading Decode Code+Table Firmware */
		for(i=0,tblSize=0,algoSize=0; i<BRAP_DSP_P_MAX_DEC_EXE_N_TBL_IN_A_CTXT; i++,tblSize=0,algoSize=0)
		{
			algoId = sExecID.decID[i];
			if(algoId == BAF_INVALID)
			{
				BDBG_MSG(("Invalid decId - 0x%x\n",algoId));				
				continue;
			}
			BDBG_MSG(("Valid decId - 0x%x\n",algoId));

				/* ----------------------------------- */
				/*** Decode Algorithm Code Firmware ****/
				/* ---_------------------------------- */
				
			algoSize = hDsp->psMitArrays->decAlgoSizes[algoId];
			if (bufSizeUsed + algoSize > allocFwDwnldSize)
			{
				BDBG_ERR(("ERROR: size available (%d) less than fsFwSize(%d)",allocFwDwnldSize-bufSizeUsed ,algoSize));
				return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			}

			algoPtr = hDsp->memInfo.fsBufBasePtr[0] + bufSizeUsed;
			if (algoSize)
			{
				err = BRAP_DSP_P_CopyFWImageToMem (hDsp, algoPtr ,
					hDsp->psMitArrays->decAlgoArrays[algoId]);
				if (err!=BERR_SUCCESS)
					return BERR_TRACE(err);
				BDBG_MSG(("Downloading decode algorithm %d at address 0x%x," 
					"size = 0x%x i=%d ,j=%d",algoId, algoPtr,algoSize,i,j));
			}
			/* Update the buf size used */
			bufSizeUsed += algoSize;
			
				/* ----------------------------------- */
				/*** Decode Algorithm Table Firmware ****/
				/* ---_------------------------------- */
			if(algoId != BAF_ALGORITHM_MPEG_L2 && algoId != BAF_ALGORITHM_MP3
					&& algoId != BAF_ALGORITHM_DDP_1
#if (BRAP_7401_FAMILY == 1) || ( BCHP_CHIP == 7400 )
					&& algoId!=BAF_ALGORITHM_WMA_PRO_1
#endif
					)

			{
				tblSize = hDsp->psMitArrays->decTblSizes[algoId];
				if (bufSizeUsed + tblSize > allocFwDwnldSize)
				{
					BDBG_ERR(("ERROR: size available (%d) less than fsFwSize(%d)",allocFwDwnldSize-bufSizeUsed ,tblSize));
					return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
				}
				tblPtr = hDsp->memInfo.fsBufBasePtr[0] + bufSizeUsed;
				if (tblSize) 
				{
					err = BRAP_DSP_P_CopyFWImageToMem (hDsp, tblPtr ,
						hDsp->psMitArrays->decTblArrays[algoId]);
					if (err!=BERR_SUCCESS)
						return BERR_TRACE(err);
					BDBG_MSG(("Downloading decode table %d at address 0x%x, "
						"size = 0x%x, i=%d, j=%d",algoId, tblPtr,tblSize,i,j));
				}
				/* Update the buf size used */
				bufSizeUsed += tblSize;
			}
			else if (algoId == BAF_ALGORITHM_MPEG_L2 || algoId == BAF_ALGORITHM_MP3) 
			{
				/* Exceptional Case: MPEG L2 and MP3 share the same table as 
				MPEG L1 (Just download it once). tblPtr will be same as  MPEG-L1
				which will default taken from previous iteration, so no need to 
				set it again*/
				BDBG_MSG(("Exceptional Case: MPEG-L2/MP3 using the same decode"
							"table as MPEG-L1"));
				tblSize = hDsp->psMitArrays->decTblSizes[BAF_ALGORITHM_MPEG_L1];
			}
			else if (algoId == BAF_ALGORITHM_DDP_1) 
			{
				/* tblPtr will be same as  DDP which will default taken 
				from previous iteration, so no need to set it again*/
				
				BDBG_MSG(("Exceptional Case: Using DDP stage 1 table for DDP "
						"stage 2 executatble"));
				tblSize = hDsp->psMitArrays->decTblSizes[BAF_ALGORITHM_DDP];
			}
#if (BRAP_7401_FAMILY == 1) || ( BCHP_CHIP ==7400 )
			else if (algoId == BAF_ALGORITHM_WMA_PRO_1) 
			{
				/* tblPtr will be same as  DDP which will default taken 
				from previous iteration, so no need to set it again*/
				
				BDBG_MSG(("Exceptional Case: Using Wma Pro stage 1 table for Wma Pro "
						"stage 2 executatble"));
				tblSize = hDsp->psMitArrays->decTblSizes[BAF_ALGORITHM_WMA_PRO_0];
			}
#endif
			/* Update the MIT Entry */
			mitPtr = (hDsp->memInfo.mitDSPtr + SIZE_OF_BAF_HOST_INFO_ALLIGNED + 
	   			(BAF_HOST_MAX_DL_MODULE * SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED) +
	        	((BRAP_DSP_P_FwType_eDec - 1) * (BAF_HOST_MAX_DL_MODULE *
	        	SIZE_OF_BAF_DOWNLOAD_ALLIGNED))+
	        	(algoId * SIZE_OF_BAF_DOWNLOAD_ALLIGNED) );

	           BRAP_DSP_P_AlgTblUpdateDRAMEntry(hDsp, mitPtr, algoId, 
	                    algoSize, algoPtr, tblSize, tblPtr, 
#if (BRAP_DSP_P_7401_NEWMIT==1)
	                    sAlgoMemReq.sDecodeMemReq.ui32DecScratchBufSize,
	                    true);
#else		                    
	                    sAlgoMemReq.sDecodeMemReq.ui32InterstageBufSize,
	                    sAlgoMemReq.sDecodeMemReq.ui32InterstageInterfaceBufSize,
	                    sAlgoMemReq.sDecodeMemReq.ui32DecScratchBufSize);
#endif
		}/* for  each BRAP_DSP_P_MAX_DEC_EXE_N_TBL_IN_A_CTXT */

		/*Downloading Postprocessing Algorithms Code+Table Firmware*/
		for(i=0, tblSize=0,algoSize=0; i<BRAP_DSP_P_MAX_PP_EXE_N_TBL_IN_A_CTXT; i++, tblSize=0,algoSize=0)
		{
			/* ----------------------------------- */
			/*** Post Processing Algo Firmware *** */
			/* ---_------------------------------- */
			algoId = sExecID.ppID[i];
			if(algoId == BAF_INVALID)
			{
				BDBG_MSG(("Invalid ppId - 0x%x\n",algoId));				
				continue;
			}

			BDBG_MSG(("Valid ppId - 0x%x\n",algoId));				

			algoSize = hDsp->psMitArrays->ppAlgoSizes[algoId];
			if (bufSizeUsed + algoSize > allocFwDwnldSize)
			{
				BDBG_ERR(("ERROR: size available (%d) less than fsFwSize(%d)",allocFwDwnldSize-bufSizeUsed ,algoSize));
				return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			}
			algoPtr = hDsp->memInfo.fsBufBasePtr[0] + bufSizeUsed;
			if (algoSize) {
				err = BRAP_DSP_P_CopyFWImageToMem (hDsp, algoPtr ,hDsp->psMitArrays->ppAlgoArrays[algoId]);
				if (err!=BERR_SUCCESS)
					return BERR_TRACE(err);
				BDBG_MSG(("Downloading PP algorithm %d at address 0x%x, "
					"size = 0x%x,i=%d , j=%d",algoId, algoPtr,algoSize,i,j));
			}

			/* Update the buf size used */
			bufSizeUsed += algoSize;

			/* ------------------------------------ */
			/*** Post Processing Table Firmware *** */
			/* ------------------------------------ */
			tblSize = hDsp->psMitArrays->ppTblSizes[algoId];
			if (bufSizeUsed + tblSize > allocFwDwnldSize)
			{
				BDBG_ERR(("ERROR: size available (%d) less than fsFwSize(%d)",allocFwDwnldSize-bufSizeUsed ,tblSize));
				return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			}
			tblPtr = hDsp->memInfo.fsBufBasePtr[0] + bufSizeUsed;
			if (tblSize) {
				err = BRAP_DSP_P_CopyFWImageToMem (hDsp, tblPtr ,hDsp->psMitArrays->ppTblArrays[algoId]);
				if (err!=BERR_SUCCESS)
					return BERR_TRACE(err);
				BDBG_MSG(("Downloading PP table %d at address 0x%x, "
				"size = 0x%x , i =%d ,j=%d",algoId, tblPtr ,tblSize,i,j));
			}	

			/* Update the buf size used */
			bufSizeUsed += tblSize;

           	/* Update the MIT Entry */
			mitPtr = (hDsp->memInfo.mitDSPtr + SIZE_OF_BAF_HOST_INFO_ALLIGNED + 
           			(BAF_HOST_MAX_DL_MODULE * SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED) +
		        	((BRAP_DSP_P_FwType_ePp - 1) * (BAF_HOST_MAX_DL_MODULE * SIZE_OF_BAF_DOWNLOAD_ALLIGNED))+
        			(algoId * SIZE_OF_BAF_DOWNLOAD_ALLIGNED) );

            BRAP_DSP_P_AlgTblUpdateDRAMEntry(hDsp, mitPtr, algoId, 
                    algoSize, algoPtr, tblSize, tblPtr, 
#if (BRAP_DSP_P_7401_NEWMIT==1)
                    0, false);
#else
                    0, 0, 0);
#endif

		}/* for  each BRAP_DSP_P_MAX_PP_EXE_N_TBL_IN_A_CTXT */

		/* Downloading Passthru Code+Table */

		err = BRAP_DSP_P_GetExecIds(BRAP_DSPCHN_DecodeMode_ePassThru, eAudType,
				&sExecID);
		if(err != BERR_SUCCESS)
		{
			BDBG_ERR(("Error: Aborting Firmware download - %d",err));
			return BERR_TRACE(err);
		}
		
		for(i=0, tblSize=0,algoSize=0; i<BRAP_DSP_P_MAX_PT_EXE_N_TBL_IN_A_CTXT; i++, tblSize=0,algoSize=0)
		{
			algoId = sExecID.ptID[i];
			if(algoId == BAF_INVALID)
			{
				BDBG_MSG(("Invalid ptId - 0x%x\n",algoId));				
				continue;
			}
			BDBG_MSG(("Valid ptId - 0x%x\n",algoId));
			
				/* ----------------------------------- */
				/*  Passthru Algorithm Code  Firmware  */
				/* ---_------------------------------- */
				
			algoSize = hDsp->psMitArrays->ptAlgoSizes[algoId];
			if (bufSizeUsed + algoSize > allocFwDwnldSize)
			{
				BDBG_ERR(("ERROR: size available (%d) less than fsFwSize(%d)",allocFwDwnldSize-bufSizeUsed ,algoSize));
				return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			}
			algoPtr = hDsp->memInfo.fsBufBasePtr[0]	+ bufSizeUsed;
			if (algoSize) {
				err = BRAP_DSP_P_CopyFWImageToMem (hDsp, algoPtr ,
					hDsp->psMitArrays->ptAlgoArrays[algoId]);
				if (err!=BERR_SUCCESS)
					return BERR_TRACE(err);
				BDBG_MSG(("Downloading PT algorithm %d at address 0x%x,"
					"size = 0x%x",algoId, algoPtr, algoSize));
			}	
			/* Update the buf size used */
			bufSizeUsed += algoSize;

				/* ----------------------------------- */
				/*  Passthru Algorithm Table Firmware  */
				/* ---_------------------------------- */

			tblSize = hDsp->psMitArrays->ptTblSizes[algoId];
			if (bufSizeUsed + tblSize > allocFwDwnldSize)
			{
				BDBG_ERR(("ERROR: size available (%d) less than fsFwSize(%d)",allocFwDwnldSize-bufSizeUsed ,tblSize));
				return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			}
			/* Download FW */
			tblPtr = hDsp->memInfo.fsBufBasePtr[0]+ bufSizeUsed;
			if (tblSize) {
				err = BRAP_DSP_P_CopyFWImageToMem (hDsp, tblPtr ,
					hDsp->psMitArrays->ptTblArrays[algoId]);
				if (err!=BERR_SUCCESS)
					return BERR_TRACE(err);
				BDBG_MSG(("Downloading PT table %d at address 0x%x, "
					"size = 0x%x",algoId, tblPtr, tblSize));
			}
			/* Update the buf size used */
			bufSizeUsed += tblSize;
			mitPtr = (hDsp->memInfo.mitDSPtr + SIZE_OF_BAF_HOST_INFO_ALLIGNED + 
               		(BAF_HOST_MAX_DL_MODULE * SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED) +
    		        ((BRAP_DSP_P_FwType_ePt - 1) * (BAF_HOST_MAX_DL_MODULE 
    		        * SIZE_OF_BAF_DOWNLOAD_ALLIGNED))+
            		(algoId * SIZE_OF_BAF_DOWNLOAD_ALLIGNED) );

                BRAP_DSP_P_AlgTblUpdateDRAMEntry(hDsp, mitPtr, algoId, 
                        algoSize, algoPtr, tblSize, tblPtr,
#if (BRAP_DSP_P_7401_NEWMIT==1)
                    0, true);
#else
                    0, 0, 0);
#endif
		}/* for  each BRAP_DSP_P_MAX_PT_EXE_N_TBL_IN_A_CTXT */

		/* Download all the Passthru Ids which appears in SimulMode array of execId, but not in PassthruMode array of execid */
		if(BRAP_DSPCHN_AudioType_eAc3Plus== (BRAP_DSPCHN_AudioType)j)
    		{
        		err = BRAP_DSP_P_GetExecIds(BRAP_DSPCHN_DecodeMode_eSimulMode, eAudType,
        				&sExecID);
        		if(err != BERR_SUCCESS)
        		{
        			BDBG_ERR(("Error: Aborting Firmware download - %d",err));
        			return BERR_TRACE(err);
        		}
        		
        		for(i=0, tblSize=0,algoSize=0; i<BRAP_DSP_P_MAX_PT_EXE_N_TBL_IN_A_CTXT; i++, tblSize=0,algoSize=0)
        		{
        			algoId = sExecID.ptID[i];
        			if(algoId == BAF_INVALID)
        			{
        				BDBG_MSG(("Invalid ptId - 0x%x\n",algoId));				
        				continue;
        			}
        			BDBG_MSG(("Valid ptId - 0x%x\n",algoId));
        			
        				/* ----------------------------------- */
        				/*  Passthru Algorithm Code  Firmware  */
        				/* ---_------------------------------- */
        				
        			algoSize = hDsp->psMitArrays->ptAlgoSizes[algoId];
        			if (bufSizeUsed + algoSize > allocFwDwnldSize)
        			{
        				BDBG_ERR(("ERROR: size available (%d) less than fsFwSize(%d)",allocFwDwnldSize-bufSizeUsed ,algoSize));
        				return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        			}
        			algoPtr = hDsp->memInfo.fsBufBasePtr[0]	+ bufSizeUsed;
        			if (algoSize) {
        				err = BRAP_DSP_P_CopyFWImageToMem (hDsp, algoPtr ,
        					hDsp->psMitArrays->ptAlgoArrays[algoId]);
        				if (err!=BERR_SUCCESS)
        					return BERR_TRACE(err);
        				BDBG_MSG(("Downloading PT algorithm %d at address 0x%x,"
        					"size = 0x%x",algoId, algoPtr, algoSize));
        			}	
        			/* Update the buf size used */
        			bufSizeUsed += algoSize;

        				/* ----------------------------------- */
        				/*  Passthru Algorithm Table Firmware  */
        				/* ---_------------------------------- */

        			tblSize = hDsp->psMitArrays->ptTblSizes[algoId];
        			if (bufSizeUsed + tblSize > allocFwDwnldSize)
        			{
        				BDBG_ERR(("ERROR: size available (%d) less than fsFwSize(%d)",allocFwDwnldSize-bufSizeUsed ,tblSize));
        				return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        			}
        			/* Download FW */
        			tblPtr = hDsp->memInfo.fsBufBasePtr[0]+ bufSizeUsed;
        			if (tblSize) {
        				err = BRAP_DSP_P_CopyFWImageToMem (hDsp, tblPtr ,
        					hDsp->psMitArrays->ptTblArrays[algoId]);
        				if (err!=BERR_SUCCESS)
        					return BERR_TRACE(err);
        				BDBG_MSG(("Downloading PT table %d at address 0x%x, "
        					"size = 0x%x",algoId, tblPtr, tblSize));
        			}
        			/* Update the buf size used */
        			bufSizeUsed += tblSize;
        			mitPtr = (hDsp->memInfo.mitDSPtr + SIZE_OF_BAF_HOST_INFO_ALLIGNED + 
                       		(BAF_HOST_MAX_DL_MODULE * SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED) +
            		        ((BRAP_DSP_P_FwType_ePt - 1) * (BAF_HOST_MAX_DL_MODULE 
            		        * SIZE_OF_BAF_DOWNLOAD_ALLIGNED))+
                    		(algoId * SIZE_OF_BAF_DOWNLOAD_ALLIGNED) );

                        BRAP_DSP_P_AlgTblUpdateDRAMEntry(hDsp, mitPtr, algoId, 
                                algoSize, algoPtr, tblSize, tblPtr,
#if (BRAP_DSP_P_7401_NEWMIT==1)
                            0, true);
#else
                            0, 0, 0);
#endif
        		}/* for  each BRAP_DSP_P_MAX_PT_EXE_N_TBL_IN_A_CTXT */
    		}

	}/* for  each BRAP_MAX_AUDIO_TYPES */



	/* Download Encoder Firmware */

	/* Add encoder PreProcessing Algos */
		BDBG_ERR(("Downloading Encoder Fw"));
		algoPtr = hDsp->memInfo.encAlgoBasePtr;
		tblPtr = hDsp->memInfo.encTblBasePtr;

		algoSize=hDsp->memInfo.totalEncAlgoSize;
		
		if (bufSizeUsed + algoSize > allocFwDwnldSize)
		{
			BDBG_ERR(("ERROR: size available (%d) less than fsFwSize(%d)",allocFwDwnldSize-bufSizeUsed ,algoSize));
			return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		}
		BRAP_DSP_P_CopyFWImageToMem (hDsp, algoPtr, BRAP_IMG_MP3_ENCODE_STG0_ID);
		BDBG_ERR(("Downloading algo at %#x",algoPtr));
		algoPtr += hDsp->psEncMitArrays->encAlgoSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+0];
		bufSizeUsed+=hDsp->psEncMitArrays->encAlgoSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+0];
		
		BRAP_DSP_P_CopyFWImageToMem (hDsp, algoPtr, BRAP_IMG_MP3_ENCODE_STG1_ID);
		algoPtr += hDsp->psEncMitArrays->encAlgoSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+1];
		bufSizeUsed+=hDsp->psEncMitArrays->encAlgoSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+1];		

/*		BRAP_DSP_P_CopyFWImageToMem (hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId], algoPtr, BRAP_IMG_MP3_ENCODE_STG2_ID);
		algoPtr += hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->psEncMitArrays->encAlgoSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+2];*/

		algoSize=hDsp->memInfo.totalEncTblSize;

		if (bufSizeUsed + algoSize > allocFwDwnldSize)
		{
			BDBG_ERR(("ERROR: size available (%d) less than fsFwSize(%d)",allocFwDwnldSize-bufSizeUsed ,algoSize));
			return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		}
		BDBG_ERR(("Downloading algo Table at %#x",tblPtr));
		BRAP_DSP_P_CopyFWImageToMem (hDsp, tblPtr, BRAP_IMG_MP3_ENCODE_TABLE_ID);
		tblPtr +=hDsp->psEncMitArrays->encTblSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+0];
		bufSizeUsed+=hDsp->psEncMitArrays->encTblSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+0];
		
		BRAP_DSP_P_CopyFWImageToMem (hDsp, tblPtr, BRAP_IMG_MP3_ENCODE_TABLE_ID);
		tblPtr +=hDsp->psEncMitArrays->encTblSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+1];
		bufSizeUsed+=hDsp->psEncMitArrays->encTblSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+1];

		hDsp->memInfo.actualFwDwnldSize +=bufSizeUsed;
	return BERR_SUCCESS;
}

#endif

static void BRAP_DSP_P_InitializeConfigBuffArray(BRAP_DSP_Handle hDsp)
{
	unsigned int uiSize;
		
	BDBG_ASSERT(hDsp);

	if ( hDsp->memInfo.arrayOfConfigBufPtr[0] != BRAP_P_INVALID_DRAM_ADDRESS &&
		hDsp->memInfo.arrayOfConfigBufPtr[0] != 0 )
	{
		uiSize = (sizeof(uint32_t) * ( BRAP_P_MAX_PP_SUPPORTED + 1 )*2*
			BRAP_P_MAX_BRANCH_SUPPORTED*BRAP_RM_P_MAX_DECODE_SUPPORTED );

		BKNI_Memset( (unsigned int *)hDsp->memInfo.arrayOfConfigBufPtr[0],
			BRAP_P_INVALID_DRAM_ADDRESS,
			uiSize );
	}
}

BERR_Code BRAP_DSP_P_GetExecIds(BRAP_DSPCHN_DecodeMode eDecMode, 
						   BRAP_DSPCHN_AudioType eAudType, 
						   BRAP_DSP_P_ExecID *psExecID /* [out] */)
{
	uint32_t k;
#if BRAP_DSP_P_DBG_ENABLED==1
	uint32_t i,j;
#endif

	*psExecID = sExecIdArray[eAudType][eDecMode];

#if BRAP_DSP_P_DBG_ENABLED==1
	for(i=0; i<BRAP_DSP_P_MAX_NUM_AUDIO_TYPE_SUPPORTED; i++)
	{
		BDBG_MSG(("\n=====Audio Type - 0x%X=====",i));
		for(j=0; j<BRAP_DSP_P_MAX_NUM_DECODE_MODE_SUPPORTED ; j++)
		{
			BDBG_MSG(("--- Decode Mode - 0x%X ---",j));
			BDBG_MSG((" Frame Sync IDs: "));	
			for(k=0; k<BRAP_DSP_P_MAX_FS_ALG_IN_A_CTXT; k++)
				BDBG_MSG(("0x%X ",sExecIdArray[i][j].fsID[k]));					

			BDBG_MSG((" Decode IDs: "));	
			for(k=0; k<BRAP_DSP_P_MAX_DEC_EXE_N_TBL_IN_A_CTXT; k++)
				BDBG_MSG(("0x%X ",sExecIdArray[i][j].decID[k]));					

			BDBG_MSG((" Post Processing IDs: "));	
			for(k=0; k<BRAP_DSP_P_MAX_PP_EXE_N_TBL_IN_A_CTXT; k++)
				BDBG_MSG(("0x%X ",sExecIdArray[i][j].ppID[k]));					

			BDBG_MSG((" Pass Thru IDs: "));	
			for(k=0; k<BRAP_DSP_P_MAX_PT_EXE_N_TBL_IN_A_CTXT; k++)
				BDBG_MSG(("0x%X ",sExecIdArray[i][j].ptID[k]));					

			BDBG_MSG((" Transport ID: 0x%X \n",sExecIdArray[i][j].transID));	
		}
	}
#endif 
	return BERR_SUCCESS;
}

void  BRAP_DSP_P_GetPpExecIds(
				BRAP_DSPCHN_PP_Algo	ePpAlgo,	/* [in] Post processing algorithm */
				BRAP_DSP_P_ExecID *psExecID /* [out] */)
{
	*psExecID = sPpExecIdArray[ePpAlgo];
}

static void BRAP_DSP_P_InitializeArrays(BRAP_DSP_Handle hDsp, sMITArrays *psMitArrays)
{
	uint32_t count, size;
#if (BRAP_7401_FAMILY == 1) || (BRAP_DSP_P_7401_NEWMIT==1)
	bool bSupportedPpAlgo[BRAP_DSPCHN_PP_Algo_eMax];    
#endif
	/* Pre-initialize size arrays to zeros */
	for (count = 0; count < BAF_HOST_MAX_DL_MODULE; count++) {
		psMitArrays->fsAlgoSizes[count] = 0x0;
		psMitArrays->decAlgoSizes[count] = 0x0;
		psMitArrays->decTblSizes[count] = 0x0;
		psMitArrays->ppAlgoSizes[count] = 0x0;
		psMitArrays->ppTblSizes[count] = 0x0;
		psMitArrays->ptAlgoSizes[count] = 0x0;
		psMitArrays->ptTblSizes[count] = 0x0;
	}

	/* Pre-initialize pointer arrays to 0xFF */
	for (count = 0; count < BAF_HOST_MAX_DL_MODULE; count++) {
		psMitArrays->fsAlgoArrays[count] = 0xFFFFFFFF;
		psMitArrays->decAlgoArrays[count] = 0xFFFFFFFF;
		psMitArrays->decTblArrays[count] = 0xFFFFFFFF;
		psMitArrays->ppAlgoArrays[count] = 0xFFFFFFFF;
		psMitArrays->ppTblArrays[count] = 0xFFFFFFFF;
		psMitArrays->ptAlgoArrays[count] = 0xFFFFFFFF;
		psMitArrays->ptTblArrays[count] = 0xFFFFFFFF;
	}
	
	/* Now initialize the arrays */
	/******** AC3 algorithm **********/
#if defined ( BCHP_7411_VER ) || (BRAP_7401_FAMILY == 1)	
	/* Frame Sync Exe */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AC3_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_AC3] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_AC3] = (uint32_t) BRAP_IMG_AC3_FRAMESYNC_ID;
	
	/* Decode Exe and decode tables */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AC3_DECODE_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_AC3] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_AC3] = (uint32_t) BRAP_IMG_AC3_DECODE_ID;
	
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AC3_DECODE_TABLE_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_AC3] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_AC3] = (uint32_t) BRAP_IMG_AC3_DECODE_TABLE_ID;
	
	/* Pass Thru */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AC3_PASSTHRU_ID, &size);
	psMitArrays->ptAlgoSizes[BAF_PASS_THRU_AC3] = BRAP_P_ALIGN(size, 2);
	psMitArrays->ptAlgoArrays[BAF_PASS_THRU_AC3] = (uint32_t) BRAP_IMG_AC3_PASSTHRU_ID;
#endif	
	
	/* Post processing */
#ifdef BCHP_7411_VER /* Only for the 7411 */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AC3_DOWNMIX_CODE_ID, &size);
	psMitArrays->ppAlgoSizes[BAF_PP_DOWNMIX_AC3] = BRAP_P_ALIGN(size, 2);
	psMitArrays->ppAlgoArrays[BAF_PP_DOWNMIX_AC3] = (uint32_t) BRAP_IMG_AC3_DOWNMIX_CODE_ID;

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AC3_DOWNMIX_TABLE_ID, &size);
	psMitArrays->ppTblSizes[BAF_PP_DOWNMIX_AC3] = BRAP_P_ALIGN(size, 2);
	psMitArrays->ppTblArrays[BAF_PP_DOWNMIX_AC3] = (uint32_t) BRAP_IMG_AC3_DOWNMIX_TABLE_ID;
	
#endif
	/******** AC3 algorithm **********/
	
	/******** MPEG algorithm **********/
	/* Frame Sync Exe */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_MPEG_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_MPEG] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_MPEG] = (uint32_t) BRAP_IMG_MPEG_FRAMESYNC_ID;
	
	/* Decode Exe and decode tables */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_MPEG_L1_DECODE_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_MPEG_L1] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_MPEG_L1] = (uint32_t) BRAP_IMG_MPEG_L1_DECODE_ID;
	
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_MPEG_L2_DECODE_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_MPEG_L2] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_MPEG_L2] = (uint32_t) BRAP_IMG_MPEG_L2_DECODE_ID;
	
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_MP3_DECODE_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_MP3] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_MP3] = (uint32_t) BRAP_IMG_MP3_DECODE_ID;
	
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_MPEG_DECODE_TABLE_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_MPEG_L1] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_MPEG_L1] = (uint32_t) BRAP_IMG_MPEG_DECODE_TABLE_ID;
	
	/* Pass Thru */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_MPEG_PASSTHRU_ID, &size);
	psMitArrays->ptAlgoSizes[BAF_PASS_THRU_MPEG] = BRAP_P_ALIGN(size, 2);
	psMitArrays->ptAlgoArrays[BAF_PASS_THRU_MPEG] = (uint32_t) BRAP_IMG_MPEG_PASSTHRU_ID;
	/******** MPEG algorithm **********/
	

	/******** AAC algorithm **********/
	/* Frame Sync Exe */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AAC_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_AAC] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_AAC] = (uint32_t) BRAP_IMG_AAC_FRAMESYNC_ID;

	/* Pass Thru */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AAC_PASSTHRU_ID, &size);
	psMitArrays->ptAlgoSizes[BAF_PASS_THRU_AAC] = BRAP_P_ALIGN(size, 2);
	psMitArrays->ptAlgoArrays[BAF_PASS_THRU_AAC] = (uint32_t) BRAP_IMG_AAC_PASSTHRU_ID;

	/******** AAC algorithm **********/

	/******** AACHE algorithm **********/
	/* Frame Sync Exe */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AACHE_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_AAC_HE] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_AAC_HE] = (uint32_t) BRAP_IMG_AACHE_FRAMESYNC_ID;

#if (BRAP_7401_FAMILY == 1) || ( BCHP_CHIP == 7400 )

	/* Decode Exe and decode tables */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AACHE_DECODE_STG0_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_AACHE_0] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_AACHE_0] = (uint32_t) BRAP_IMG_AACHE_DECODE_STG0_ID;

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AACHE_TABLE_STG0_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_AACHE_0] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_AACHE_0] = (uint32_t) BRAP_IMG_AACHE_TABLE_STG0_ID;

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AACHE_DECODE_STG1_ID, &size);	
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_AACHE_1] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_AACHE_1] = (uint32_t) BRAP_IMG_AACHE_DECODE_STG1_ID;

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AACHE_TABLE_STG1_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_AACHE_1] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_AACHE_1] = (uint32_t) BRAP_IMG_AACHE_TABLE_STG1_ID;

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AACHE_DECODE_STG2_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_AACHE_2] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_AACHE_2] = (uint32_t) BRAP_IMG_AACHE_DECODE_STG2_ID;

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AACHE_TABLE_STG2_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_AACHE_2] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_AACHE_2] = (uint32_t) BRAP_IMG_AACHE_TABLE_STG2_ID;

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AACHE_DECODE_STG3_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_AACHE_3] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_AACHE_3] = (uint32_t) BRAP_IMG_AACHE_DECODE_STG3_ID;

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AACHE_TABLE_STG3_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_AACHE_3] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_AACHE_3] = (uint32_t) BRAP_IMG_AACHE_TABLE_STG3_ID;

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AACHE_DECODE_STG4_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_AACHE_4] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_AACHE_4] = (uint32_t) BRAP_IMG_AACHE_DECODE_STG4_ID;

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AACHE_TABLE_STG4_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_AACHE_4] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_AACHE_4] = (uint32_t) BRAP_IMG_AACHE_TABLE_STG4_ID;
      
	/* Pass Thru */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AACHE_PASSTHRU_ID, &size);
	psMitArrays->ptAlgoSizes[BAF_PASS_THRU_AAC_HE] = BRAP_P_ALIGN(size, 2);
	psMitArrays->ptAlgoArrays[BAF_PASS_THRU_AAC_HE] = (uint32_t) BRAP_IMG_AACHE_PASSTHRU_ID;
#else	
	/* Decode Exe and decode tables */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AAC_DECODE_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_AAC_HE] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_AAC_HE] = (uint32_t) BRAP_IMG_AAC_DECODE_ID;
	
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AAC_DECODE_TABLE_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_AAC_HE] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_AAC_HE] = (uint32_t) BRAP_IMG_AAC_DECODE_TABLE_ID;
	
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AACHE_DECODE_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_AAC_HE_1] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_AAC_HE_1] = (uint32_t) BRAP_IMG_AACHE_DECODE_ID;
	
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AACHE_DECODE_TABLE_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_AAC_HE_1] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_AAC_HE_1] = (uint32_t) BRAP_IMG_AACHE_DECODE_TABLE_ID;
#endif	
	/******** AACHE algorithm **********/
	
	
	/******** DDP algorithm **********/
	/* Frame Sync Exe */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DDP_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_DDP] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_DDP] = (uint32_t) BRAP_IMG_DDP_FRAMESYNC_ID;
	
	/* Decode Exe and decode tables */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DDP_FE_DECODE_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_DDP] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_DDP] = (uint32_t) BRAP_IMG_DDP_FE_DECODE_ID;
	
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DDP_DECODE_TABLE_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_DDP] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_DDP] = (uint32_t) BRAP_IMG_DDP_DECODE_TABLE_ID;

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DDP_BE_DECODE_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_DDP_1] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_DDP_1] = (uint32_t) BRAP_IMG_DDP_BE_DECODE_ID;
	
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DDP_DECODE_TABLE_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_DDP_1] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_DDP_1] = (uint32_t) BRAP_IMG_DDP_DECODE_TABLE_ID;
	
	/* Pass Thru */
#if (BCHP_CHIP == 7400 )    
#ifdef BRAP_DDP_SUPPORT	
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DDP_7_1_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_DDP_7_1] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_DDP_7_1] = (uint32_t) BRAP_IMG_DDP_7_1_FRAMESYNC_ID;
#endif	
#endif    
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DDP_PASSTHRU_ID, &size);
	psMitArrays->ptAlgoSizes[BAF_PASS_THRU_DDP] = BRAP_P_ALIGN(size, 2);
	psMitArrays->ptAlgoArrays[BAF_PASS_THRU_DDP] = (uint32_t)BRAP_IMG_DDP_PASSTHRU_ID ;

	
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DDP_PASSTHRU_TABLE_ID, &size);
	psMitArrays->ptTblSizes[BAF_PASS_THRU_DDP] = BRAP_P_ALIGN(size, 2);
	psMitArrays->ptTblArrays[BAF_PASS_THRU_DDP] = (uint32_t) BRAP_IMG_DDP_PASSTHRU_TABLE_ID;

#if ( BCHP_CHIP == 7400 )
	/* Adding for converter */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DDP_CONVERT_ID, &size);
	psMitArrays->ptAlgoSizes[BAF_CONVERT_DDP] = BRAP_P_ALIGN(size, 2);
	psMitArrays->ptAlgoArrays[BAF_CONVERT_DDP] = (uint32_t) BRAP_IMG_DDP_CONVERT_ID;

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DDP_PASSTHRU_TABLE_ID, &size);
	psMitArrays->ptTblSizes[BAF_CONVERT_DDP] = BRAP_P_ALIGN(size, 2);
	psMitArrays->ptTblArrays[BAF_CONVERT_DDP] = (uint32_t) BRAP_IMG_DDP_PASSTHRU_TABLE_ID;
#endif	
	/******** DDP algorithm **********/


	/******** WMA_STD algorithm **********/
	/* Frame Sync Exe */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_WMA_STD_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_WMA_STD] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_WMA_STD] = (uint32_t) BRAP_IMG_WMA_STD_FRAMESYNC_ID;
	
	/* Decode Exe and decode tables */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_WMA_STD_STG1_DECODE_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_WMA_STD] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_WMA_STD] = (uint32_t) BRAP_IMG_WMA_STD_STG1_DECODE_ID;

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_WMA_STD_STG1_DECODE_TABLE_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_WMA_STD] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_WMA_STD] = (uint32_t) BRAP_IMG_WMA_STD_STG1_DECODE_TABLE_ID;

#if (BRAP_7401_FAMILY == 1) || ( BCHP_CHIP == 7400 )
		/******** WMA_PRO algorithm **********/
	/* Frame Sync Exe */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_WMA_PRO_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_WMA_PRO] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_WMA_PRO] = (uint32_t) BRAP_IMG_WMA_PRO_FRAMESYNC_ID;
	
	/* Decode Exe and decode tables */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_WMA_PRO_DECODE_STG0_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_WMA_PRO_0] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_WMA_PRO_0] = (uint32_t) BRAP_IMG_WMA_PRO_DECODE_STG0_ID;

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_WMA_PRO_DECODE_TABLE_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_WMA_PRO_0] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_WMA_PRO_0] = (uint32_t) BRAP_IMG_WMA_PRO_DECODE_TABLE_ID;


	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_WMA_PRO_DECODE_STG1_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_WMA_PRO_1] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_WMA_PRO_1] = (uint32_t) BRAP_IMG_WMA_PRO_DECODE_STG1_ID;

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_WMA_PRO_DECODE_TABLE_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_WMA_PRO_1] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_WMA_PRO_1] = (uint32_t) BRAP_IMG_WMA_PRO_DECODE_TABLE_ID;

	/* Pass Thru */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_WMA_PRO_PASSTHRU_ID, &size);
	psMitArrays->ptAlgoSizes[BAF_PASS_THRU_WMA_PRO] = BRAP_P_ALIGN(size, 2);
	psMitArrays->ptAlgoArrays[BAF_PASS_THRU_WMA_PRO] = (uint32_t) BRAP_IMG_WMA_PRO_PASSTHRU_ID;


	/******** WMA_PRO algorithm **********/
#endif

#ifdef RAP_DRA_SUPPORT
		/******** DRA algorithm **********/
	/* Decode Exe and decode tables */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DRA_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_DRA] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_DRA] = (uint32_t) BRAP_IMG_DRA_ID;

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DRA_TABLE_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_DRA] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_DRA] = (uint32_t) BRAP_IMG_DRA_TABLE_ID;

	/* Frame Sync Exe */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DRA_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_DRA] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_DRA] = (uint32_t) BRAP_IMG_DRA_FRAMESYNC_ID;
	
	/* Passthru Exe */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DRA_PASSTHRU_ID, &size);
	psMitArrays->ptAlgoSizes[BAF_PASS_THRU_DRA] = BRAP_P_ALIGN(size, 2);
	psMitArrays->ptAlgoArrays[BAF_PASS_THRU_DRA] = (uint32_t) BRAP_IMG_DRA_PASSTHRU_ID;

	/******** DRA algorithm **********/
#endif


	/******** LPCM_DVD algorithm **********/ 
	/* Frame Sync Exe */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_LPCM_DVD_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_DVD_LPCM] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_DVD_LPCM] = (uint32_t) BRAP_IMG_LPCM_DVD_FRAMESYNC_ID;
	
	/* Decode Exe and decode tables */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_LPCM_DVD_DECODE_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_LPCM_DVD] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_LPCM_DVD] = (uint32_t) BRAP_IMG_LPCM_DVD_DECODE_ID;
 
	/* Post processing */
       /* No post processing for LPCM_DVD */ 
	/******** LPCM_DVD  algorithm **********/ 
	
#if (BRAP_DTS_PASSTHRU_SUPPORTED == 1)

	/******** DTS algorithm **********/
	/* Frame Sync Exe */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DTS_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_DTS] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_DTS] = (uint32_t) BRAP_IMG_DTS_FRAMESYNC_ID;
		
	/* Pass Thru */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DTS_PASSTHRU_ID, &size);
	psMitArrays->ptAlgoSizes[BAF_PASS_THRU_DTS] = BRAP_P_ALIGN(size, 2);
	psMitArrays->ptAlgoArrays[BAF_PASS_THRU_DTS] = (uint32_t) BRAP_IMG_DTS_PASSTHRU_ID;

#endif

#if (RAP_PCMWAV_SUPPORT == 1)
	/* Frame Sync Exe */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_PCMWAV_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_PCMWAV] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_PCMWAV] = (uint32_t) BRAP_IMG_PCMWAV_FRAMESYNC_ID;
	
	/* Decode Exe and decode tables */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_PCMWAV_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_PCMWAV] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_PCMWAV] = (uint32_t) BRAP_IMG_PCMWAV_ID;	
#endif

#if (BCHIP_CHIP == 7411) || (BRAP_DTS_SUPPORTED == 1)

	/******** DTS algorithm **********/
	/* Frame Sync Exe */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DTS_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_DTS] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_DTS] = (uint32_t) BRAP_IMG_DTS_FRAMESYNC_ID;
	
	/* Decode Exe and decode tables */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DTS_DECODE_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_DTS] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_DTS] = (uint32_t) BRAP_IMG_DTS_DECODE_ID;
	
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DTS_DECODE_TABLE_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_DTS] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_DTS] = (uint32_t) BRAP_IMG_DTS_DECODE_TABLE_ID;
	
	/* Pass Thru */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DTS_PASSTHRU_ID, &size);
	psMitArrays->ptAlgoSizes[BAF_PASS_THRU_DTS] = BRAP_P_ALIGN(size, 2);
	psMitArrays->ptAlgoArrays[BAF_PASS_THRU_DTS] = (uint32_t) BRAP_IMG_DTS_PASSTHRU_ID;

#endif

#ifdef BCHP_7411_VER /* Only for the 7411 */
	
	/* Post processing */
	/******** DTS algorithm **********/

	/******** BD_LPCM algorithm **********/
	/* Frame Sync Exe */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_BD_LPCM_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_BD_LPCM] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_BD_LPCM] = (uint32_t) BRAP_IMG_BD_LPCM_FRAMESYNC_ID;
	
	/* Decode Exe and decode tables */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_BD_LPCM_DECODE_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_BD_LPCM] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_BD_LPCM] = (uint32_t) BRAP_IMG_BD_LPCM_DECODE_ID;
	
	/* Post processing */
	/******** BD_LPCM  algorithm **********/

	/******** LPCM_HDDVD algorithm **********/
	/* Frame Sync Exe */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_LPCM_HDDVD_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_HDDVD_LPCM] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_HDDVD_LPCM] = (uint32_t) BRAP_IMG_LPCM_HDDVD_FRAMESYNC_ID;
	
	/* Decode Exe and decode tables */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_LPCM_HDDVD_DECODE_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_LPCM_HDDVD] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_LPCM_HDDVD] = (uint32_t) BRAP_IMG_LPCM_HDDVD_DECODE_ID;
 
	/* Post processing */
       /* No post processing for LPCM_HDDVD */ 
	/******** LPCM_HDDVD  algorithm **********/ 

	/******** DTSHD algorithm **********/
	/* Frame Sync Exe */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DTSHD_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_DTSHD] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_DTSHD] = (uint32_t) BRAP_IMG_DTSHD_FRAMESYNC_ID;
	
	/* Frame Sync Exe if stream type is HD-DVD format */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DTSHD_HDDVD_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_DTS_HD_HDDVD] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_DTS_HD_HDDVD] = (uint32_t) BRAP_IMG_DTSHD_HDDVD_FRAMESYNC_ID;
	
	/* Decode Exe and decode tables */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DTSHD_DECODE_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_DTSHD] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_DTSHD] = (uint32_t) BRAP_IMG_DTSHD_DECODE_ID;
	
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DTSHD_DECODE_TABLE_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_DTSHD] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_DTSHD] = (uint32_t) BRAP_IMG_DTSHD_DECODE_TABLE_ID;
	
	/* Pass Thru */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DTSHD_PASSTHRU_ID, &size);
	psMitArrays->ptAlgoSizes[BAF_PASS_THRU_DTSHD] = BRAP_P_ALIGN(size, 2);
	psMitArrays->ptAlgoArrays[BAF_PASS_THRU_DTSHD] = (uint32_t) BRAP_IMG_DTSHD_PASSTHRU_ID;
	/******** DTSHD algorithm **********/

	/******** LPCM_DVD algorithm **********/
	/* Frame Sync Exe */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_LPCM_DVD_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_DVD_LPCM] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_DVD_LPCM] = (uint32_t) BRAP_IMG_LPCM_DVD_FRAMESYNC_ID;
	
	/* Decode Exe and decode tables */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_LPCM_DVD_DECODE_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_LPCM_DVD] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_LPCM_DVD] = (uint32_t) BRAP_IMG_LPCM_DVD_DECODE_ID;
 
	/* Post processing */
       /* No post processing for LPCM_DVD */ 
	/******** LPCM_DVD  algorithm **********/ 

	/* Post processing */
       /* No post processing for WMA_STD */ 
	/******** WMA_STD  algorithm **********/ 

	/******** AC3_LOSSLESS algorithm **********/
	/* Frame Sync Exe */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AC3LOSSLESS_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_DD_LOSSLESS] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_DD_LOSSLESS] = (uint32_t) BRAP_IMG_AC3LOSSLESS_FRAMESYNC_ID;
	
	/* Decode Exe and decode tables */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AC3LOSSLESS_FE_DECODE_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_DD_LOSSLESS] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_DD_LOSSLESS] = (uint32_t) BRAP_IMG_AC3LOSSLESS_FE_DECODE_ID;
	
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AC3LOSSLESS_DECODE_TABLE_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_DD_LOSSLESS] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_DD_LOSSLESS] = (uint32_t) BRAP_IMG_AC3LOSSLESS_DECODE_TABLE_ID;
	
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AC3LOSSLESS_BE_DECODE_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_DD_LOSSLESS_1] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_DD_LOSSLESS_1] = (uint32_t) BRAP_IMG_AC3LOSSLESS_BE_DECODE_ID;
	
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AC3LOSSLESS_DECODE_TABLE_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_DD_LOSSLESS_1] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_DD_LOSSLESS_1] = (uint32_t) BRAP_IMG_AC3LOSSLESS_DECODE_TABLE_ID;
	
	/* Pass Thru */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AC3LOSSLESS_PASSTHRU_ID, &size);
	psMitArrays->ptAlgoSizes[BAF_PASS_THRU_DD_LOSSLESS] = BRAP_P_ALIGN(size, 2);
	psMitArrays->ptAlgoArrays[BAF_PASS_THRU_DD_LOSSLESS] = (uint32_t) BRAP_IMG_AC3LOSSLESS_PASSTHRU_ID;
	
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_AC3LOSSLESS_PASSTHRU_TABLE_ID, &size);
	psMitArrays->ptTblSizes[BAF_PASS_THRU_DD_LOSSLESS] = BRAP_P_ALIGN(size, 2);
	psMitArrays->ptTblArrays[BAF_PASS_THRU_DD_LOSSLESS] = (uint32_t) BRAP_IMG_AC3LOSSLESS_PASSTHRU_TABLE_ID;
	
	/* Post processing */
       /* No post processing for AC3_LOSSLESS */ 
	/******** AC3_LOSSLESS algorithm **********/

	/******** MLP algorithm **********/
	/* Frame Sync Exe */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_MLP_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_MLP] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_MLP] = (uint32_t) BRAP_IMG_MLP_FRAMESYNC_ID;

    /* Frame Sync Exe if stream type is HD-DVD format */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_MLP_HDDVD_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_SYNC_MLP_HDDVD] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_SYNC_MLP_HDDVD] = (uint32_t) BRAP_IMG_MLP_HDDVD_FRAMESYNC_ID;

	/* Decode Exe and decode tables */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_MLP_DECODE_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_MLP] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_MLP] = (uint32_t) BRAP_IMG_MLP_DECODE_ID;

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_MLP_DECODE_TABLE_ID, &size);
	psMitArrays->decTblSizes[BAF_ALGORITHM_MLP] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decTblArrays[BAF_ALGORITHM_MLP] = (uint32_t) BRAP_IMG_MLP_DECODE_TABLE_ID;
	/******** MLP algorithm **********/
	
#endif

#ifdef BCHP_7411_VER	
	/* Post processing */
	/******** Dolby Digital Bass Management *********/
	BRAP_DSP_P_GetFWSize(hDsp, BRAP_IMG_DDBM_ID, &size);
	psMitArrays->ppAlgoSizes[BAF_PP_DDBM] = BRAP_P_ALIGN(size, 2);
	psMitArrays->ppAlgoArrays[BAF_PP_DDBM] = (uint32_t) BRAP_IMG_DDBM_ID;

	BRAP_DSP_P_GetFWSize(hDsp, BRAP_IMG_DDBM_TABLE_ID, &size);
	psMitArrays->ppTblSizes[BAF_PP_DDBM] = BRAP_P_ALIGN(size, 2);
	psMitArrays->ppTblArrays[BAF_PP_DDBM] = (uint32_t) BRAP_IMG_DDBM_TABLE_ID;
	/******** Dolby Digital Bass Management *********/
#endif	

#if (BRAP_DSP_P_7401_NEWMIT==1)
	/* Post processing */
	/******** AAC downmix *********/
	BRAP_P_GetSelectedPpAlgos(hDsp->hRap, bSupportedPpAlgo);
	
    if (bSupportedPpAlgo[BRAP_DSPCHN_PP_Algo_eAacDownmix] == true)
    {           
    	BRAP_DSP_P_GetFWSize(hDsp, BRAP_IMG_AAC_DOWNMIX_ID, &size);
        BDBG_MSG(("Adding aac downmix to psMitArrays, size=0x%x,", size));    
    	psMitArrays->ppAlgoSizes[BAF_PP_DOWNMIX_AAC] = BRAP_P_ALIGN(size, 2);
    	psMitArrays->ppAlgoArrays[BAF_PP_DOWNMIX_AAC] = (uint32_t) BRAP_IMG_AAC_DOWNMIX_ID;

        /* no table for AAC downmix */
    }
	/******** AAC downmix  *********/

	/******** DSOLA *********/
    if (bSupportedPpAlgo[BRAP_DSPCHN_PP_Algo_eDsola] == true)
    {           
    	BRAP_DSP_P_GetFWSize(hDsp, BRAP_IMG_DSOLA_ID, &size);
        BDBG_MSG(("Adding DSOLA to psMitArrays, size=0x%x,", size));    
    	psMitArrays->ppAlgoSizes[BAF_PP_DSOLA] = BRAP_P_ALIGN(size, 2);
    	psMitArrays->ppAlgoArrays[BAF_PP_DSOLA] = (uint32_t) BRAP_IMG_DSOLA_ID;

    	BRAP_DSP_P_GetFWSize(hDsp, BRAP_IMG_DSOLA_TABLE_ID, &size);
        BDBG_MSG(("Adding DSOLA table to psMitArrays, sizxe=0x%x ", size)); 
    	psMitArrays->ppTblSizes[BAF_PP_DSOLA] = BRAP_P_ALIGN(size, 2);
    	psMitArrays->ppTblArrays[BAF_PP_DSOLA] = (uint32_t) BRAP_IMG_DSOLA_TABLE_ID;
    }
	/******** AAC downmix  *********/

#endif
#if (BRAP_7401_FAMILY == 1) || (BCHP_CHIP == 7400)
	/* Post processing */
	/******** Sample Rate Change *********/
	BRAP_P_GetSelectedPpAlgos(hDsp->hRap, bSupportedPpAlgo);
	
    if (bSupportedPpAlgo[BRAP_DSPCHN_PP_Algo_eSrc] == true)
    {        
    	BRAP_DSP_P_GetFWSize(hDsp, BRAP_IMG_SRC_ID, &size);
        BDBG_MSG(("Adding SRC to psMitArrays, size=0x%x,", size));    
    	psMitArrays->ppAlgoSizes[BAF_PP_SRC] = BRAP_P_ALIGN(size, 2);
    	psMitArrays->ppAlgoArrays[BAF_PP_SRC] = (uint32_t) BRAP_IMG_SRC_ID;


    	BRAP_DSP_P_GetFWSize(hDsp, BRAP_IMG_SRC_TABLE_ID, &size);
        BDBG_MSG(("Adding SRC table to psMitArrays, sizxe=0x%x ", size)); 
    	psMitArrays->ppTblSizes[BAF_PP_SRC] = BRAP_P_ALIGN(size, 2);
    	psMitArrays->ppTblArrays[BAF_PP_SRC] = (uint32_t) BRAP_IMG_SRC_TABLE_ID;
    }
	/******** Sample Rate Change *********/
#endif	

#if (BRAP_AD_SUPPORTED == 1)
	/* Post processing */
	/******** Fade Control *********/
	BRAP_P_GetSelectedPpAlgos(hDsp->hRap, bSupportedPpAlgo);
	
    if (bSupportedPpAlgo[BRAP_DSPCHN_PP_Algo_eAD_FadeCtrl] == true)
    {        
    	BRAP_DSP_P_GetFWSize(hDsp, BRAP_IMG_AD_FADECTRL_ID, &size);
        BDBG_MSG(("Adding SRC to psMitArrays, size=0x%x,", size));    
    	psMitArrays->ppAlgoSizes[BAF_PP_AD_FADECTRL] = BRAP_P_ALIGN(size, 2);
    	psMitArrays->ppAlgoArrays[BAF_PP_AD_FADECTRL] = (uint32_t) BRAP_IMG_AD_FADECTRL_ID;


    	BRAP_DSP_P_GetFWSize(hDsp, BRAP_IMG_AD_FADECTRL_TABLE_ID, &size);
        BDBG_MSG(("Adding SRC table to psMitArrays, sizxe=0x%x ", size)); 
    	psMitArrays->ppTblSizes[BAF_PP_AD_FADECTRL] = BRAP_P_ALIGN(size, 2);
    	psMitArrays->ppTblArrays[BAF_PP_AD_FADECTRL] = (uint32_t) BRAP_IMG_AD_FADECTRL_TABLE_ID;
    }
	/******** Fade Control *********/

	/* Post processing */
	/******** Pan Control*********/
	
    if (bSupportedPpAlgo[BRAP_DSPCHN_PP_Algo_eAD_PanCtrl] == true)
    {        
    	BRAP_DSP_P_GetFWSize(hDsp, BRAP_IMG_AD_PANCTRL_ID, &size);
        BDBG_MSG(("Adding SRC to psMitArrays, size=0x%x,", size));    
    	psMitArrays->ppAlgoSizes[BAF_PP_AD_PANCTRL] = BRAP_P_ALIGN(size, 2);
    	psMitArrays->ppAlgoArrays[BAF_PP_AD_PANCTRL] = (uint32_t) BRAP_IMG_AD_PANCTRL_ID;


    	BRAP_DSP_P_GetFWSize(hDsp, BRAP_IMG_AD_PANCTRL_TABLE_ID, &size);
        BDBG_MSG(("Adding SRC table to psMitArrays, sizxe=0x%x ", size)); 
    	psMitArrays->ppTblSizes[BAF_PP_AD_PANCTRL] = BRAP_P_ALIGN(size, 2);
    	psMitArrays->ppTblArrays[BAF_PP_AD_PANCTRL] = (uint32_t) BRAP_IMG_AD_PANCTRL_TABLE_ID;
    }
	/******** Pan Control *********/    
#endif

#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)
    /******** Dolby Volume ***************/	
    if (bSupportedPpAlgo[BRAP_DSPCHN_PP_Algo_eDolbyVolume] == true)
    {        
    	BRAP_DSP_P_GetFWSize(hDsp, BRAP_IMG_DOLBYVOLUME_ID, &size);
        BDBG_MSG(("Adding Dobly Volume to psMitArrays, size=0x%x,", size));    
    	psMitArrays->ppAlgoSizes[BAF_PP_DOBLYVOLUME] = BRAP_P_ALIGN(size, 2);
    	psMitArrays->ppAlgoArrays[BAF_PP_DOBLYVOLUME] = (uint32_t) BRAP_IMG_DOLBYVOLUME_ID;


    	BRAP_DSP_P_GetFWSize(hDsp, BRAP_IMG_DOLBYVOLUME_TABLE_ID, &size);
        BDBG_MSG(("Adding Dolby Volume table to psMitArrays, sizxe=0x%x ", size)); 
    	psMitArrays->ppTblSizes[BAF_PP_DOBLYVOLUME] = BRAP_P_ALIGN(size, 2);
    	psMitArrays->ppTblArrays[BAF_PP_DOBLYVOLUME] = (uint32_t) BRAP_IMG_DOLBYVOLUME_TABLE_ID;
    }
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)
    /******** SRS Volume ***************/	
    if (bSupportedPpAlgo[BRAP_DSPCHN_PP_Algo_eSRS_TruVol] == true)
    {        
    	BRAP_DSP_P_GetFWSize(hDsp, BRAP_IMG_SRSTRUVOL_ID, &size);
        BDBG_MSG(("Adding SRS Volume to psMitArrays, size=0x%x,", size));    
    	psMitArrays->ppAlgoSizes[BAF_PP_SRSTRUVOL] = BRAP_P_ALIGN(size, 2);
    	psMitArrays->ppAlgoArrays[BAF_PP_SRSTRUVOL] = (uint32_t) BRAP_IMG_SRSTRUVOL_ID;


    	BRAP_DSP_P_GetFWSize(hDsp, BRAP_IMG_SRSTRUVOL_TABLE_ID, &size);
        BDBG_MSG(("Adding SRS Volume table to psMitArrays, sizxe=0x%x ", size)); 
    	psMitArrays->ppTblSizes[BAF_PP_SRSTRUVOL] = BRAP_P_ALIGN(size, 2);
    	psMitArrays->ppTblArrays[BAF_PP_SRSTRUVOL] = (uint32_t) BRAP_IMG_SRSTRUVOL_TABLE_ID;
    }
#endif 

#ifdef RAP_SRSTRUVOL_CERTIFICATION
	BRAP_DSP_P_GetFWSize(hDsp, BRAP_IMG_PCM_PASSTTHRU_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_PCM_PASSTHRU] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_PCM_PASSTHRU] = (uint32_t) BRAP_IMG_PCM_PASSTTHRU_ID;

	BRAP_DSP_P_GetFWSize(hDsp, BRAP_IMG_PCM_PASSTTHRU_FRAMESYNC_ID, &size);
	psMitArrays->fsAlgoSizes[BAF_FRAME_PCM_PASSTHRU] = BRAP_P_ALIGN(size, 2);
	psMitArrays->fsAlgoArrays[BAF_FRAME_PCM_PASSTHRU] = (uint32_t) BRAP_IMG_PCM_PASSTTHRU_FRAMESYNC_ID;    
    
#endif


#ifndef BCHP_7411_VER /* For chips other than 7411 */
	/* Encoder Scheduler */	
        BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_ENC_SCHEDULER_CODE_ID, &size);
	psMitArrays->decAlgoSizes[BAF_ALGORITHM_ENCODER_SCHEDULER] = BRAP_P_ALIGN(size, 2);
	psMitArrays->decAlgoArrays[BAF_ALGORITHM_ENCODER_SCHEDULER] = (uint32_t) BRAP_IMG_ENC_SCHEDULER_CODE_ID;
#endif	

#if BRAP_DSP_P_DBG_ENABLED==1
	for (count = 0; count < BAF_HOST_MAX_DL_MODULE; count++)
		BDBG_MSG(("fsAlgoSizes[%d] = 0x%x, fsAlgoArrays[%d] = 0x%x", count, psMitArrays->fsAlgoSizes[count], count, psMitArrays->fsAlgoArrays[count]));
	
	for (count = 0; count < BAF_HOST_MAX_DL_MODULE; count++) {
		BDBG_MSG(("decAlgoSizes[%d] = 0x%x, decAlgoArrays[%d] = 0x%x", count, psMitArrays->decAlgoSizes[count], count, psMitArrays->decAlgoArrays[count]));
		BDBG_MSG(("decTblSizes[%d] = 0x%x, decTblArrays[%d] = 0x%x", count, psMitArrays->decTblSizes[count], count, psMitArrays->decTblArrays[count]));
	}
	
	for (count = 0; count < BAF_HOST_MAX_DL_MODULE; count++) {
		BDBG_MSG(("ppAlgoSizes[%d] = 0x%x, ppAlgoArrays[%d] = 0x%x",count, psMitArrays->ppAlgoSizes[count], count, psMitArrays->ppAlgoArrays[count]));
		BDBG_MSG(("ppTblSizes[%d] = 0x%x, ppTblArrays[%d] = 0x%x",count, psMitArrays->ppTblSizes[count], count, psMitArrays->ppTblArrays[count]));
	}

	for (count = 0; count < BAF_HOST_MAX_DL_MODULE; count++) {
		BDBG_MSG(("ptAlgoSizes[%d] = 0x%x, ptAlgoArrays[%d] = 0x%x",count, psMitArrays->ptAlgoSizes[count], count, psMitArrays->ptAlgoArrays[count]));
		BDBG_MSG(("ptTblSizes[%d] = 0x%x, ptTblArrays[%d] = 0x%x",count, psMitArrays->ptTblSizes[count], count, psMitArrays->ptTblArrays[count]));
	}

#endif
}

BERR_Code BRAP_DSP_P_DownloadExecFW( BRAP_DSP_Handle hDsp, 
									 BRAP_DSPCHN_DecodeMode eDecMode,
									 BRAP_DSPCHN_AudioType eAudType,
									 BRAP_DSPCHN_PP_Algo	ePpAlgo,
									 bool bDnldFs,
									 bool bDnPp,
									 uint32_t fwCtxIndx,
									 BRAP_DSP_P_ExecID sExecID)
{
	uint32_t algoPtr, fwSize, sizeAvailable, fwId, ppId, i;
  	uint32_t tblPtr=0, algoSize, tblSize, algoId;
	uint32_t bufSizeUsed;
	BERR_Code err = BERR_SUCCESS;
    uint32_t mitPtr, physAddress;

#if (BRAP_DSP_P_7401_NEWMIT==1)
	BRAP_P_Dec_AlgoMemRequirement sDecAlgoMemReq;
    BRAP_P_Pp_AlgoMemRequirement sPpAlgoMemReq;
#else
	BRAP_P_AlgoMemRequirement sAlgoMemReq;
#endif 
	
#if BRAP_DSP_P_DBG_MIT_TABLE==1
	uint32_t cnt, memVal;			
#endif

	BDBG_ENTER (BRAP_DSP_P_DownloadExecFW);  

	BDBG_MSG(("DnldExecFW: eDecMode(%d) eAudType(%d)  ePpAlgo(%d) bDnldFs(%d) bDnPp(%d) fwCtxIndx(%d)",
				eDecMode,eAudType, ePpAlgo, bDnldFs, bDnPp, fwCtxIndx));
	
	if(bDnldFs)
	{
		for(i=0, bufSizeUsed=0; i<BRAP_DSP_P_MAX_FS_ALG_IN_A_CTXT; i++)
		{
			/* --------------------------- */
			/* *** Frame Sync firmware *** */
			/* --------------------------------- */			
			fwId = sExecID.fsID[i];
			if(fwId == BAF_INVALID)
			{
				BDBG_MSG(("Invalid FS Id - 0x%x\n",fwId));				
				continue;
			}

			BDBG_MSG(("Valid FS Id - 0x%x\n",fwId));				
			
			fwSize = hDsp->psMitArrays->fsAlgoSizes[fwId];			
			sizeAvailable = hDsp->memInfo.fsBufSize[fwCtxIndx] - bufSizeUsed; 
			BDBG_MSG(("fsBufSize size available (%d) less than fsFwSize(%d)",sizeAvailable,fwSize));
			/* Check if size is sufficient */
			if (sizeAvailable < fwSize)
			{
				BDBG_ERR(("ERROR: size available (%d) less than fsFwSize(%d)",sizeAvailable,fwSize));
				return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			}
			/* Download FW */
			algoPtr = hDsp->memInfo.fsBufBasePtr[fwCtxIndx] + bufSizeUsed;
			if (fwSize) {
				err = BRAP_DSP_P_CopyFWImageToMem (hDsp, algoPtr ,hDsp->psMitArrays->fsAlgoArrays[fwId]);
				if (err!=BERR_SUCCESS)
					return BERR_TRACE(err);
				BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)(algoPtr), &physAddress);
				BDBG_MSG(("Downloading frame sync algorithm %d at physical address 0x%08x, size = 0x%08x", 
					fwId, 
					physAddress,
					fwSize));
			}

			/* Update the buf size used */
			bufSizeUsed += fwSize;
            
			/* Update the MIT Entry */
            mitPtr = /* (unsigned char *) */(hDsp->memInfo.mitDSPtr + SIZE_OF_BAF_HOST_INFO_ALLIGNED + 
		        (fwId * SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED));	

            BRAP_DSP_P_FsUpdateDRAMEntry(hDsp, mitPtr, fwId, fwSize, algoPtr); 
		}
	}
	else if (bDnPp)
	{
		/* --------------------------- */
		/* *** Post Processing firmware *** */
		/* --------------------------------- */
		/* Get scratch buffer split up */
#if (BRAP_DSP_P_7401_NEWMIT==1)
		err = BRAP_P_GetPpAlgoMemRequirement (hDsp->hRap, ePpAlgo, &sPpAlgoMemReq);
#else
		err = BRAP_P_GetPpAlgoMemRequirement (hDsp->hRap, ePpAlgo, &sAlgoMemReq);
#endif 		
		
		if (err != BERR_SUCCESS)
			return BERR_TRACE(err);
		
		ppId = sExecID.ppID[0];

		if (ppId==BAF_INVALID)
		{
			BDBG_MSG(("Invalid FS Id - 0x%x\n",ppId));	
			return BERR_TRACE(BERR_NOT_INITIALIZED);
		}

		fwSize = hDsp->psMitArrays->ppAlgoSizes[ppId];
#if (BRAP_DSP_P_7401_NEWMIT==1)
        sizeAvailable = hDsp->memInfo.ui32PpFwBufSize - hDsp->memInfo.ui32PpFwBufSizeUsed;
#else		
		sizeAvailable = hDsp->memInfo.commonPpBufSize - hDsp->memInfo.commonPpBufSizeUsed;
#endif 	
		/* Check if size is sufficient */
		if (sizeAvailable < fwSize)
		{
			BDBG_ERR(("ERROR: size available (%d) ppid(%d) less than ppFwSize(%d)",sizeAvailable,ppId,fwSize));
			return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		}

		/* Download FW */
#if (BRAP_DSP_P_7401_NEWMIT==1)
        algoPtr = hDsp->memInfo.ui32PpFwBufPtr + hDsp->memInfo.ui32PpFwBufSizeUsed;
#else		
		algoPtr = hDsp->memInfo.commonPpBufPtr + hDsp->memInfo.commonPpBufSizeUsed;
#endif 	
		if (fwSize) {
			err = BRAP_DSP_P_CopyFWImageToMem (hDsp, algoPtr ,hDsp->psMitArrays->ppAlgoArrays[ppId]);
			if (err!=BERR_SUCCESS)
				return BERR_TRACE(err);
			BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)(algoPtr), &physAddress);
			BDBG_MSG(("Downloading post processing algorithm %d at algoptr 0x%08x, physical address 0x%08x, size = 0x%08x", ppId, algoPtr, physAddress, fwSize));
		}

		/* Update the buf size used */
#if (BRAP_DSP_P_7401_NEWMIT==1)
        hDsp->memInfo.ui32PpFwBufSizeUsed += fwSize;
#else
		hDsp->memInfo.commonPpBufSizeUsed += fwSize;
#endif      
        
		/* --------------------------------- */			
		/* *** Post Processing Table firmware *** */
		/* --------------------------------- */	
		tblSize = hDsp->psMitArrays->ppTblSizes[ppId];
#if (BRAP_DSP_P_7401_NEWMIT==1)
		sizeAvailable = hDsp->memInfo.ui32PpFwBufSize - hDsp->memInfo.ui32PpFwBufSizeUsed;
#else		
		sizeAvailable = hDsp->memInfo.commonPpBufSize - hDsp->memInfo.commonPpBufSizeUsed;
#endif 	
		/* Check if size is sufficient */
		if (sizeAvailable < tblSize)
		{
			BDBG_ERR(("ERROR: size available (%d) less than ppTblSize(%d)",sizeAvailable,tblSize));
			return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		}

		/* Download Table */
#if (BRAP_DSP_P_7401_NEWMIT==1)
		tblPtr = hDsp->memInfo.ui32PpFwBufPtr + hDsp->memInfo.ui32PpFwBufSizeUsed;
#else
		tblPtr = hDsp->memInfo.commonPpBufPtr + hDsp->memInfo.commonPpBufSizeUsed;
#endif 		

		
		if (tblSize) {
			err = BRAP_DSP_P_CopyFWImageToMem (hDsp, tblPtr ,hDsp->psMitArrays->ppTblArrays[ppId]);
			if (err!=BERR_SUCCESS)
				return BERR_TRACE(err);
			BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)(tblPtr), &physAddress);
			BDBG_MSG(("Downloading table for post processing algorithm %d at physical address 0x%08x, size = 0x%08x", ppId, physAddress,tblSize));
		}

		/* Update the buf size used */
#if (BRAP_DSP_P_7401_NEWMIT==1)
		hDsp->memInfo.ui32PpFwBufSizeUsed += tblSize;
#else
		hDsp->memInfo.commonPpBufSizeUsed += tblSize;
#endif 		

		
		/* Update the MIT Entry */
		mitPtr = (hDsp->memInfo.mitDSPtr + SIZE_OF_BAF_HOST_INFO_ALLIGNED + 
       			(BAF_HOST_MAX_DL_MODULE * SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED) +
	        		((BRAP_DSP_P_FwType_ePp - 1) * (BAF_HOST_MAX_DL_MODULE * SIZE_OF_BAF_DOWNLOAD_ALLIGNED))+
    				(ppId * SIZE_OF_BAF_DOWNLOAD_ALLIGNED) );

		BRAP_DSP_P_AlgTblUpdateDRAMEntry(hDsp, mitPtr, ppId, 
                        fwSize, algoPtr, tblSize, tblPtr, 
#if (BRAP_DSP_P_7401_NEWMIT==1)
                        sPpAlgoMemReq.sPpMemReq.ui32PpScratchBufSize, false);
#else                        
                        sAlgoMemReq.sDecodeMemReq.ui32InterstageBufSize,
                        sAlgoMemReq.sDecodeMemReq.ui32InterstageInterfaceBufSize,
                        sAlgoMemReq.sDecodeMemReq.ui32DecScratchBufSize);
#endif
		/* DD Bass Management Table needs to initialized on every "start decode". Hence store
		 * the address where this table gets downloaded. 
		 * TODO: Initializing table on every decode start is not a good idea and FW-PI interface 
		 * needs to be re-designed. */
		if (ppId==BAF_PP_DDBM) 
			hDsp->memInfo.ddbmTblPtr = tblPtr;
	}
	else
	{
		if (eDecMode == BRAP_DSPCHN_DecodeMode_eDecode)
		{
			/* Get scratch buffer split up */
#if (BRAP_DSP_P_7401_NEWMIT==1)
			err = BRAP_P_GetAlgoMemRequirement (hDsp->hRap, eAudType, &sDecAlgoMemReq);
#else								
			err = BRAP_P_GetAlgoMemRequirement (hDsp->hRap, eAudType, &sAlgoMemReq);
#endif			
			if (err != BERR_SUCCESS)
				return BERR_TRACE(err);

			for(i=0, bufSizeUsed=0, tblSize=0; i<BRAP_DSP_P_MAX_DEC_EXE_N_TBL_IN_A_CTXT; i++, tblSize=0)
			{
				/* --------------------------------- */
				/* *** Decode Algorithm firmware *** */
				/* --------------------------------- */			
				algoId = sExecID.decID[i];
				if(algoId == BAF_INVALID)
				{
					BDBG_MSG(("Invalid decId - 0x%x\n",algoId));				
					continue;
				}

				BDBG_MSG(("Valid decId - 0x%x\n",algoId));				

				algoSize = hDsp->psMitArrays->decAlgoSizes[algoId];
 				sizeAvailable = hDsp->memInfo.ctxtFwBufSize[fwCtxIndx] - bufSizeUsed; 

				/* Check if size is sufficient */
				if (sizeAvailable < algoSize)
				{
					BDBG_ERR(("ERROR: size available (%d) less than decAlgoFwSize(%d)",sizeAvailable,algoSize));
					return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
				}

				/* Download FW */
				algoPtr = hDsp->memInfo.ctxtFwBufBasePtr[fwCtxIndx] + bufSizeUsed;
				if (algoSize) {
					err = BRAP_DSP_P_CopyFWImageToMem (hDsp, algoPtr ,hDsp->psMitArrays->decAlgoArrays[algoId]);
					if (err!=BERR_SUCCESS)
						return BERR_TRACE(err);
					BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)(algoPtr), &physAddress);
					BDBG_MSG(("Downloading decode algorithm %d at physical address 0x%08x, size = 0x%08x",algoId, physAddress,algoSize));
				}
				/* Update the buf size used */
				bufSizeUsed += algoSize;

				/* ----------------------------- */			
				/* *** Decode Table firmware *** */
				/* ----------------------------- */			
				if(algoId != BAF_ALGORITHM_MPEG_L2 && algoId != BAF_ALGORITHM_MP3
					&& algoId != BAF_ALGORITHM_DDP_1 && algoId != BAF_ALGORITHM_PCMWAV
#ifdef RAP_SRSTRUVOL_CERTIFICATION          
&& (algoId != BAF_ALGORITHM_PCM_PASSTHRU)
#endif
)
				{
					tblSize = hDsp->psMitArrays->decTblSizes[algoId];
					sizeAvailable = hDsp->memInfo.ctxtFwBufSize[fwCtxIndx] - bufSizeUsed; 
	
					/* Check if size is sufficient */
					if (sizeAvailable < tblSize)
					{
						BDBG_ERR(("ERROR: size available (%d) less than decTblFwSize(%d)",sizeAvailable,tblSize));
						return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
					}

					/* Download FW */
					tblPtr = hDsp->memInfo.ctxtFwBufBasePtr[fwCtxIndx] + bufSizeUsed;
					if (tblSize) {
						err = BRAP_DSP_P_CopyFWImageToMem (hDsp, tblPtr ,hDsp->psMitArrays->decTblArrays[algoId]);
						if (err!=BERR_SUCCESS)
							return BERR_TRACE(err);
						BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)(tblPtr), &physAddress);
						BDBG_MSG(("Downloading decode table %d at physical address 0x%08x, size = 0x%08x",algoId, physAddress,tblSize));
					}

					/* Update the buf size used */
					bufSizeUsed += tblSize;
				}
				else if (algoId == BAF_ALGORITHM_MPEG_L2 || algoId == BAF_ALGORITHM_MP3) 
				{
					/* Exceptional Case: MPEG L2 and MP3 share the same table as MPEG L1 (Just download it once) */
					BDBG_MSG(("Exceptional Case: MPEG-L2/MP3 using the same decode table as MPEG-L1"));
					tblSize = hDsp->psMitArrays->decTblSizes[BAF_ALGORITHM_MPEG_L1];
				}
				else if (algoId == BAF_ALGORITHM_DDP_1) 
				{
					BDBG_MSG(("Exceptional Case: Using DDP stage 1 table for DDP stage 2 executatble"));
					tblSize = hDsp->psMitArrays->decTblSizes[BAF_ALGORITHM_DDP];
				}

               	/* Update the MIT Entry */
				mitPtr = (hDsp->memInfo.mitDSPtr + SIZE_OF_BAF_HOST_INFO_ALLIGNED + 
               			(BAF_HOST_MAX_DL_MODULE * SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED) +
    		        	((BRAP_DSP_P_FwType_eDec - 1) * (BAF_HOST_MAX_DL_MODULE * SIZE_OF_BAF_DOWNLOAD_ALLIGNED))+
            			(algoId * SIZE_OF_BAF_DOWNLOAD_ALLIGNED) );

                BRAP_DSP_P_AlgTblUpdateDRAMEntry(hDsp, mitPtr, algoId, 
                        algoSize, algoPtr, tblSize, tblPtr, 
#if (BRAP_DSP_P_7401_NEWMIT==1)
                        sDecAlgoMemReq.sDecodeMemReq.ui32DecScratchBufSize, true);
#else                        
                        sAlgoMemReq.sDecodeMemReq.ui32InterstageBufSize,
                        sAlgoMemReq.sDecodeMemReq.ui32InterstageInterfaceBufSize,
                        sAlgoMemReq.sDecodeMemReq.ui32DecScratchBufSize);
#endif
			}/* for each BRAP_DSP_P_MAX_DEC_EXE_N_TBL_IN_A_CTXT */
		
			for(i=0, tblSize=0; i<BRAP_DSP_P_MAX_PP_EXE_N_TBL_IN_A_CTXT; i++, tblSize=0)
			{
				/* ----------------------------------- */
				/*** Post Processing Algo Firmware *** */
				/* ---_------------------------------- */
				algoId = sExecID.ppID[i];
				if(algoId == BAF_INVALID)
				{
					BDBG_MSG(("Invalid ppId - 0x%x\n",algoId));				
					continue;
				}

				BDBG_MSG(("Valid ppId - 0x%x\n",algoId));				

				algoSize = hDsp->psMitArrays->ppAlgoSizes[algoId];
				sizeAvailable = hDsp->memInfo.ctxtFwBufSize[fwCtxIndx] - bufSizeUsed; 

				/* Check if size is sufficient */
				if (sizeAvailable < algoSize)
				{
					BDBG_ERR(("ERROR: size available (%d) less than ppAlgoFwSize(%d)",sizeAvailable,algoSize));
					return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
				}

				/* Download FW */
				algoPtr = hDsp->memInfo.ctxtFwBufBasePtr[fwCtxIndx] + bufSizeUsed;
				if (algoSize) {
					err = BRAP_DSP_P_CopyFWImageToMem (hDsp, algoPtr ,hDsp->psMitArrays->ppAlgoArrays[algoId]);
					if (err!=BERR_SUCCESS)
						return BERR_TRACE(err);
                	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)algoPtr, &physAddress);
                	BDBG_MSG(("Downloading PP algorithm %d at Address = 0x%08x, Physical Address=0x%08x, Size = 0x%08x", 
                					algoPtr, physAddress, algoSize));                   
				}

				/* Update the buf size used */
				bufSizeUsed += algoSize;
	
				/* ------------------------------------ */
				/*** Post Processing Table Firmware *** */
				/* ------------------------------------ */
				tblSize = hDsp->psMitArrays->ppTblSizes[algoId];
				sizeAvailable = hDsp->memInfo.ctxtFwBufSize[fwCtxIndx] - bufSizeUsed; 

				/* Check if size is sufficient */
				if (sizeAvailable < tblSize)
				{
					BDBG_ERR(("ERROR: size available (%d) less than ppTblFwSize(%d)",sizeAvailable,tblSize));
					return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
				}
	
				/* Download FW */
				tblPtr = hDsp->memInfo.ctxtFwBufBasePtr[fwCtxIndx] + bufSizeUsed;
				if (tblSize) {
					err = BRAP_DSP_P_CopyFWImageToMem (hDsp, tblPtr ,hDsp->psMitArrays->ppTblArrays[algoId]);
					if (err!=BERR_SUCCESS)
						return BERR_TRACE(err);
                	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)tblPtr, &physAddress);
                	BDBG_MSG(("Downloading PP table %d at address Address = 0x%08x, Physical Address=0x%08x, Size = 0x%08x", 
                					algoId, tblPtr, physAddress, tblSize));                   
                    
					BDBG_MSG(("Downloading PP table %d at address 0x%x, size = 0x%x",algoId, tblPtr ,tblSize));
				}	

				/* Update the buf size used */
				bufSizeUsed += tblSize;

               	/* Update the MIT Entry */
				mitPtr = (hDsp->memInfo.mitDSPtr + SIZE_OF_BAF_HOST_INFO_ALLIGNED + 
               			(BAF_HOST_MAX_DL_MODULE * SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED) +
    		        	((BRAP_DSP_P_FwType_ePp - 1) * (BAF_HOST_MAX_DL_MODULE * SIZE_OF_BAF_DOWNLOAD_ALLIGNED))+
            			(algoId * SIZE_OF_BAF_DOWNLOAD_ALLIGNED) );

                BRAP_DSP_P_AlgTblUpdateDRAMEntry(hDsp, mitPtr, algoId, 
                        algoSize, algoPtr, tblSize, tblPtr, 
#if (BRAP_DSP_P_7401_NEWMIT==1)                         
                        0, false);
#else
						0,0,0);
#endif						                        

			}/* for  each BRAP_DSP_P_MAX_PP_EXE_N_TBL_IN_A_CTXT */
		} /* if (eDecMode == BRAP_DSPCHN_DecodeMode_eDecode) */
		else if( eDecMode == BRAP_DSPCHN_DecodeMode_ePassThru)
		{
			for(i=0, bufSizeUsed=0,tblSize=0; i<BRAP_DSP_P_MAX_PT_EXE_N_TBL_IN_A_CTXT; i++, tblSize=0)
			{
				/* ------------------------------------- */
				/*** Pass Through Algorithm Firmware *** */
				/* ------------------------------------- */
				algoId = sExecID.ptID[i];
				if(algoId == BAF_INVALID)
				{
					BDBG_MSG(("Invalid ptId - 0x%x\n",algoId));				
					continue;
				}

				BDBG_MSG(("Valid ptId - 0x%x\n",algoId));				

				algoSize = hDsp->psMitArrays->ptAlgoSizes[algoId];
				sizeAvailable = hDsp->memInfo.ctxtFwBufSize[fwCtxIndx] - bufSizeUsed; 
	
				/* Check if size is sufficient */
				if (sizeAvailable < algoSize)
				{
					BDBG_ERR(("ERROR: size available (%d) less than ptAlgoFwSize(%d)",sizeAvailable,algoSize));
					return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
				}
	
				/* Download FW */
				algoPtr = hDsp->memInfo.ctxtFwBufBasePtr[fwCtxIndx] + bufSizeUsed;
				if (algoSize) {
					err = BRAP_DSP_P_CopyFWImageToMem (hDsp, algoPtr ,hDsp->psMitArrays->ptAlgoArrays[algoId]);
					if (err!=BERR_SUCCESS)
						return BERR_TRACE(err);
					BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)(algoPtr), &physAddress);
					BDBG_MSG(("Downloading PT algorithm %d at address 0x%x, size = 0x%x",algoId, physAddress, algoSize));
				}	

				/* Update the buf size used */
				bufSizeUsed += algoSize;

				/* --------------------------------- */
				/*** Pass Through Table Firmware *** */
				/* --------------------------------- */
				tblSize = hDsp->psMitArrays->ptTblSizes[algoId];
				sizeAvailable = hDsp->memInfo.ctxtFwBufSize[fwCtxIndx] - bufSizeUsed; 

				/* Check if size is sufficient */
				if (sizeAvailable < tblSize)
				{
					BDBG_ERR(("ERROR: size available (%d) less than ptTblFwSize(%d)",sizeAvailable,tblSize));
					return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
				}
	
				/* Download FW */
				tblPtr = hDsp->memInfo.ctxtFwBufBasePtr[fwCtxIndx] + bufSizeUsed;
				if (tblSize) {
					err = BRAP_DSP_P_CopyFWImageToMem (hDsp, tblPtr ,hDsp->psMitArrays->ptTblArrays[algoId]);
					if (err!=BERR_SUCCESS)
						return BERR_TRACE(err);
					BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)(tblPtr), &physAddress);
					BDBG_MSG(("Downloading PT table %d at address 0x%x, size = 0x%x",algoId, physAddress, tblSize));
				}
				/* Update the buf size used */
				bufSizeUsed += tblSize;

               	/* Update the MIT Entry */
				mitPtr = (hDsp->memInfo.mitDSPtr + SIZE_OF_BAF_HOST_INFO_ALLIGNED + 
               			(BAF_HOST_MAX_DL_MODULE * SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED) +
    		        	((BRAP_DSP_P_FwType_ePt - 1) * (BAF_HOST_MAX_DL_MODULE * SIZE_OF_BAF_DOWNLOAD_ALLIGNED))+
            			(algoId * SIZE_OF_BAF_DOWNLOAD_ALLIGNED) );

                BRAP_DSP_P_AlgTblUpdateDRAMEntry(hDsp, mitPtr, algoId, 
#if (BRAP_DSP_P_7401_NEWMIT==1)
                        algoSize, algoPtr, tblSize, tblPtr, 0, true);
#else           
                        algoSize, algoPtr, tblSize, tblPtr, 0, 0, 0);
#endif                        
			}/* for each BRAP_DSP_P_MAX_PT_EXE_N_TBL_IN_A_CTXT */
		}/*	else if( eDecMode == BRAP_DSPCHN_DecodeMode_ePassThru) */
	}/* else of if( bDnldFs) */

#if BRAP_DSP_P_DBG_MIT_TABLE==1 
	mitPtr = hDsp->memInfo.mitDSPtr;					
	BDBG_MSG(("======== Complete MIT Data Structure============ 0x%x", mitPtr));
	for (cnt = 0; cnt < hDsp->memInfo.totalMITSize;cnt+=4)
	{
		memVal = BRAP_MemRead32((BARC_Handle)hDsp->hRegister, (uint32_t)(mitPtr+cnt));
		BDBG_MSG(("Addr[0x%08x] -> 0x%02x",mitPtr+cnt+0, (memVal & 0xFF000000)>>24));
		BDBG_MSG(("Addr[0x%08x] -> 0x%02x",mitPtr+cnt+1, (memVal & 0x00FF0000)>>16));
		BDBG_MSG(("Addr[0x%08x] -> 0x%02x",mitPtr+cnt+2, (memVal & 0x0000FF00)>>8));
		BDBG_MSG(("Addr[0x%08x] -> 0x%02x",mitPtr+cnt+3, (memVal & 0x000000FF)>>0));
	}
	BDBG_MSG(("========Complete MIT Data Structures============\n"));	
#endif
	BDBG_LEAVE (BRAP_DSP_P_DownloadExecFW);  
	return err;
}

static void BRAP_DSP_P_CreateMIT(BRAP_DSP_Handle hDsp/*, sMITArrays *psMitArrays*/)
{
	uint32_t mitIndexPtr, count;
#ifdef BCHP_7411_VER /* Only for the 7411 */
	uint32_t memVal;
#endif	
	uint32_t physAddress;
	unsigned char *mitPtr, *mitBase;
	BDBG_ENTER (BRAP_DSP_P_CreateMIT);      

	/* First create MIT table in system memory and then download in device memory */ 
	mitBase = mitPtr = (unsigned char *) BKNI_Malloc(hDsp->memInfo.totalMITSize);
	BKNI_Memset(mitPtr, 0, hDsp->memInfo.totalMITSize);
		
	/* Initialize BAF_HostInfo structure */
	BRAP_DSP_P_WriteMIT(mitPtr, 0x0, 4); mitPtr += 4;	/* MIT ID */
	BRAP_DSP_P_WriteMIT(mitPtr, 0x0, 4); mitPtr += 4;	/* MIT version */

    BRAP_DSP_P_WriteMIT(mitPtr, 0x0, 4); mitPtr += 4;   /* Downloadable Scheduler Addr */
    BRAP_DSP_P_WriteMIT(mitPtr, 0x0, 4); mitPtr += 4;   /* Downloadable Scheduler Size */
	
	BRAP_DSP_P_WriteMIT(mitPtr, 0x0, 4); mitPtr += 4;	/* APM Scratch Buf ID */
	BRAP_DSP_P_WriteMIT(mitPtr, BRAP_DSP_P_APM_SCRATCH_BUF_SIZE, 4); mitPtr += 4;	/* APM Scratch Buf size */
	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.apmScratchBufPtr, &physAddress);
	BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4); mitPtr += 4;	/* APM Scratch Buf pointer */

	/* Skip Interframe & InterStage buffer entries. These are programmed at decode start time */
#if (BRAP_DSP_P_7401_NEWMIT==1)
	mitPtr+=SIZE_OF_BAF_HOST_BUF_INFO * BAF_MAX_BRANCHES * BAF_MAX_CONTEXTS 
	    * (BAF_MAX_POSTPROCESS_STAGES+1) * 5;
#else
	mitPtr+=SIZE_OF_BAF_HOST_BUF_INFO * NUM_CHANNELS;
#endif 

    BDBG_MSG(("Writing the pFrameSync pointers to MIT"));
	mitIndexPtr = hDsp->memInfo.mitDSPtr + SIZE_OF_BAF_HOST_INFO_ALLIGNED;
 	for (count = 0; count < BAF_HOST_MAX_DL_MODULE; count++) {
		BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)mitIndexPtr, &physAddress);
        BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4); mitPtr += 4;			/* pFrameSync[count] struct pointers */
		mitIndexPtr += SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED;
	}
    BDBG_MSG(("Writing the pDecode pointers to MIT"));
	for (count = 0; count < BAF_HOST_MAX_DL_MODULE; count++) {
		BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)mitIndexPtr, &physAddress);
		BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4); mitPtr += 4;			/* pDecode[count] struct pointers */
		mitIndexPtr += SIZE_OF_BAF_DOWNLOAD_ALLIGNED;
	}

    BDBG_MSG(("Writing the pPostProcess pointers to MIT"));
	for (count = 0; count < BAF_HOST_MAX_DL_MODULE; count++) {
		BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)mitIndexPtr, &physAddress);
		BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4); mitPtr += 4;			/* pPostProcess[count] struct pointers */
		mitIndexPtr += SIZE_OF_BAF_DOWNLOAD_ALLIGNED;
	}

    BDBG_MSG(("Writing the pPassThru pointers to MIT"));
	for (count = 0; count < BAF_HOST_MAX_DL_MODULE; count++) {
		BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)mitIndexPtr, &physAddress);
		BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4); mitPtr += 4;			/* pPassThru[count] struct pointers */
		mitIndexPtr += SIZE_OF_BAF_DOWNLOAD_ALLIGNED;
	}

#if BRAP_DSP_P_DBG_MIT_TABLE==1
	BDBG_MSG(("========Master Index Table============"));
	for (count = 0; count < SIZE_OF_BAF_HOST_INFO_ALLIGNED; count++) 
		BDBG_MSG(("MIT[%d] = 0x%02x",count,*(mitBase+count)));
	BDBG_MSG(("========End of Master Index Table============"));
#endif /* BRAP_DSP_P_DBG_MIT_TABLE==1 */

	/* Download MIT table in device memory */
	for (count = 0; count < BRAP_P_ALIGN(hDsp->memInfo.totalMITSize, 2); count += 4)
	{
#ifdef BCHP_7411_VER /* Only for the 7411 */
		memVal = 0;
		memVal = (*(mitBase+count) << 24)|(*(mitBase+count+1) << 16)|(*(mitBase+count+2) << 8)|(*(mitBase+count+3));
		BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, hDsp->memInfo.mitDSPtr+count, memVal);
#else
		BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, hDsp->memInfo.mitDSPtr+count, *(uint32_t *)(mitBase + count)); 
#endif 
	}
	BRAP_P_DummyMemRead32((BARC_Handle)hDsp->hRegister, hDsp->memInfo.mitDSPtr+count);

    BDBG_MSG(("CreateMIT Done"));
#if BRAP_DSP_P_DBG_MIT_TABLE==1 
	mitPtr = (unsigned char*)(hDsp->memInfo.mitDSPtr);
	BDBG_MSG(("\nCreateMIT: read MIT from DRAM 0x%x", mitPtr));
	for (count = 0; count < hDsp->memInfo.totalMITSize;count+=4)
	{
		memVal = BRAP_MemRead32((BARC_Handle)hDsp->hRegister, (uint32_t)(mitPtr+count));
		BDBG_MSG(("Addr[0x%08x] -> 0x%02x",mitPtr+count+0, (memVal & 0xFF000000)>>24));
		BDBG_MSG(("Addr[0x%08x] -> 0x%02x",mitPtr+count+1, (memVal & 0x00FF0000)>>16));
		BDBG_MSG(("Addr[0x%08x] -> 0x%02x",mitPtr+count+2, (memVal & 0x0000FF00)>>8));
		BDBG_MSG(("Addr[0x%08x] -> 0x%02x",mitPtr+count+3, (memVal & 0x000000FF)>>0));
	}
#endif /* BRAP_DSP_P_DBG_MIT_TABLE==1 */

	/* Free MIT table system memory */
	BKNI_Free(mitBase); 
	BDBG_LEAVE (BRAP_DSP_P_CreateMIT);  
}

#if BRAP_DSP_P_DBG_MIT_TABLE==1 
static void BRAP_DSP_P_PrintMit(	BRAP_DSP_Handle hDsp)
{
	unsigned char *mitPtr;
	uint32_t count, memVal;
	
	mitPtr = (unsigned char*)(hDsp->memInfo.mitDSPtr);
	BDBG_MSG(("\nPrintMIT: read MIT from DRAM 0x%x", mitPtr));
	for (count = 0; count < hDsp->memInfo.totalMITSize;count+=4)
	{
		memVal = BRAP_MemRead32((BARC_Handle)hDsp->hRegister, (uint32_t)(mitPtr+count));
		BDBG_MSG(("Addr[0x%08x] -> 0x%02x",mitPtr+count+0, (memVal & 0xFF000000)>>24));
		BDBG_MSG(("Addr[0x%08x] -> 0x%02x",mitPtr+count+1, (memVal & 0x00FF0000)>>16));
		BDBG_MSG(("Addr[0x%08x] -> 0x%02x",mitPtr+count+2, (memVal & 0x0000FF00)>>8));
		BDBG_MSG(("Addr[0x%08x] -> 0x%02x",mitPtr+count+3, (memVal & 0x000000FF)>>0));
	}
}
#endif

static BERR_Code BRAP_DSP_P_DmaIn(	BRAP_DSP_Handle hDsp,
								uint8_t dataType,
								uint8_t swapType,
								uint32_t size,
								uint32_t src_addr,
								uint32_t dst_addr)
{
	uint32_t regVal, dmaCount = 0;
	BERR_Code err = BERR_SUCCESS;

	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_DMA_SRC_ADDR0,src_addr);
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_DMA_DST_ADDR0,dst_addr);
	/* regVal = BRAP_Read32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_DMA_TRANSFER0); */
	regVal = 0x0;
	regVal = (regVal & ~(BCHP_MASK(AUD_DSP_CTRL0_DMA_TRANSFER0,SWAP_TYPE)))|
				(BCHP_FIELD_DATA(AUD_DSP_CTRL0_DMA_TRANSFER0,SWAP_TYPE,swapType));
	regVal = (regVal & ~(BCHP_MASK(AUD_DSP_CTRL0_DMA_TRANSFER0,DATA_TYPE)))|
				(BCHP_FIELD_DATA(AUD_DSP_CTRL0_DMA_TRANSFER0,DATA_TYPE,dataType));
	regVal = (regVal & ~(BCHP_MASK(AUD_DSP_CTRL0_DMA_TRANSFER0,TRAN_TYPE)))|
				(BCHP_FIELD_ENUM(AUD_DSP_CTRL0_DMA_TRANSFER0,TRAN_TYPE,Read));
	regVal = (regVal & ~(BCHP_MASK(AUD_DSP_CTRL0_DMA_TRANSFER0,NUM_BYTES)))|
				(BCHP_FIELD_DATA(AUD_DSP_CTRL0_DMA_TRANSFER0,NUM_BYTES,size));
	BDBG_MSG(("\n**** DMA TRANSFER0 = 0x%x", regVal));
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_DMA_TRANSFER0,regVal);

	BDBG_MSG(("Waiting for DMA to complete..."));	
	regVal = BRAP_DSP_P_DMA_BUSY;
	while(regVal){
		regVal = BRAP_Read32(hDsp->hRegister,BCHP_AUD_DSP_CTRL0_DMA_STATUS);
		regVal = BCHP_GET_FIELD_DATA(regVal, AUD_DSP_CTRL0_DMA_STATUS, BUSY0);
		dmaCount++;
		if (dmaCount==BRAP_DSP_P_DMA_TIMEOUT_COUNT) {
			BDBG_ERR(("DMA timeout: SRC Adr = 0x%x, DST Adr = 0x%x, size = 0x%x", src_addr, dst_addr, size));
			return BERR_TRACE(BERR_TIMEOUT);
		}
		BKNI_Delay(10);	/* 10 us of delay for total timeout of (BRAP_DSP_P_DMA_TIMEOUT_COUNT * 0.01) ms */
	}
	BDBG_MSG(("Done!"));
	return err;
}

void BRAP_DSP_P_WriteMIT(
		unsigned char *mitPtr,
		uint32_t data,
		uint32_t nBytes
		)
{
	uint32_t count;

	for (count = 0; count < nBytes; count++)
	{
		*mitPtr++ = (data >> (8 * count)) & 0xFF; 
	}
}

static void BRAP_DSP_P_AlgTblUpdateDRAMEntry(
		BRAP_DSP_Handle hDsp,
		uint32_t dramAddr,
		uint32_t algId,
		uint32_t algSize,
		uint32_t algAddr,
		uint32_t tblSize,
		uint32_t tblAddr,
#if (BRAP_DSP_P_7401_NEWMIT==1)
		uint32_t decScratchBufSize,
		bool    bAlg    /* true=>decoder, false=> pp */		
#else		
		uint32_t interstageBufSize,
		uint32_t interstageIfBufSize,
		uint32_t decScratchBufSize
#endif	
		)
{
    unsigned char ui32AlgoDetail[SIZE_OF_BAF_DOWNLOAD_ALLIGNED];
	unsigned char *mitPtr, *mitBase;
    int count;
    uint32_t physAddress, scratchSpaceBuf;
#ifdef BCHP_7411_VER /* Only for the 7411 */
	uint32_t memVal;
#endif	
	BDBG_ENTER (BRAP_DSP_P_AlgTblUpdateDRAMEntry);  
    mitBase = mitPtr = &ui32AlgoDetail[0];    
    BKNI_Memset(mitPtr, 0, SIZE_OF_BAF_DOWNLOAD_ALLIGNED);

    BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)dramAddr, &physAddress);    
    BDBG_MSG(("MIT Alg DramAddr > 0x%x, physical address > 0x%x", dramAddr, physAddress));

    /* Update locally: Algo's entry */
	BRAP_DSP_P_WriteMIT(mitPtr, algId, 4); mitPtr += 4;	
	BRAP_DSP_P_WriteMIT(mitPtr, algSize, 4); mitPtr += 4;	
   	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)algAddr, &physAddress);
    BDBG_MSG(("Algo Id > 0x%x", algId));
    BDBG_MSG(("Algo Buf physical Addr > 0x%x, Size > 0x%x", physAddress, algSize));
	BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4); mitPtr += 4;
    BDBG_MSG(("DEC: id(%d) algsize(%d) algaddr(%x) physAddr(%x)", 
        algId, algSize, algAddr, physAddress));

    /* if(tblSize != 0) */
    {
        /* Update locally: Table's entry */
	BRAP_DSP_P_WriteMIT(mitPtr, algId, 4); mitPtr += 4;	
	BRAP_DSP_P_WriteMIT(mitPtr, tblSize, 4); mitPtr += 4;	
	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)tblAddr, &physAddress);
    BDBG_MSG(("Tbl Buf Addr > 0x%x, Size > 0x%x", physAddress, tblSize));
	BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4); mitPtr += 4;
	BDBG_MSG(("TBL: id(%d) tblsize(%d) tbladdr(%x) physAddr(%x)", algId, tblSize, tblAddr, physAddress));
    }

#if (BRAP_DSP_P_7401_NEWMIT==1)
    if(true == bAlg)
    {
	    scratchSpaceBuf = hDsp->memInfo.scratchSpaceBufPtr;
    }
    else
    {
    	scratchSpaceBuf = hDsp->memInfo.ui32PpScratchBufPtr;
    }
    
	/* Update locally: Decoder Scratch buffer entry */
    if(scratchSpaceBuf != 0xFFFFFFFF)
    {
    	BRAP_DSP_P_WriteMIT(mitPtr, 0x0, 4); mitPtr += 4;	
    	BRAP_DSP_P_WriteMIT(mitPtr, decScratchBufSize, 4); mitPtr += 4;	
    	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)scratchSpaceBuf, &physAddress);
        BDBG_MSG(("Scratch Buf Addr > 0x%x, Size > 0x%x", physAddress, decScratchBufSize));
    	BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4); mitPtr += 4;
    	BDBG_MSG(("DEC Scratch Buf: id(0x0) bufsize(%d) bufaddr(%x) physAddr(%x)", decScratchBufSize, scratchSpaceBuf, physAddress));
    }

    /* Update MIT entry in the DRAM */
	for (count = 0; count < SIZE_OF_BAF_DOWNLOAD_ALLIGNED; count += 4)
	{
		BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, dramAddr+count, *(uint32_t *)(mitBase + count)); 
	}
    
	BRAP_P_DummyMemRead32((BARC_Handle)hDsp->hRegister, dramAddr+count);

    
#else
	scratchSpaceBuf = hDsp->memInfo.scratchSpaceBufPtr;
	/* Update locally: Interstage buffer entry */
	BRAP_DSP_P_WriteMIT(mitPtr, 0x0, 4); mitPtr += 4;	
	BRAP_DSP_P_WriteMIT(mitPtr, interstageBufSize, 4); mitPtr += 4;	
	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)scratchSpaceBuf, &physAddress);
	BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4); mitPtr += 4;
	BDBG_MSG(("InterStage Buf: id(0x0) bufsize(%d) bufaddr(%x) physAddr(%x)", interstageBufSize, scratchSpaceBuf, physAddress));
	scratchSpaceBuf += interstageBufSize;
		
	/* Update locally: Interstage Interface buffer entry */	
	BRAP_DSP_P_WriteMIT(mitPtr, 0x0, 4); mitPtr += 4;	
	BRAP_DSP_P_WriteMIT(mitPtr, interstageIfBufSize, 4); mitPtr += 4;	
	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)scratchSpaceBuf, &physAddress);
	BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4); mitPtr += 4;
	BDBG_MSG(("InterStage IF Buf: id(0x0) bufsize(%d) bufaddr(%x) physAddr(%x)", interstageIfBufSize, scratchSpaceBuf, physAddress));
	scratchSpaceBuf += interstageIfBufSize;
	
	/* Update locally: Decoder Scratch buffer entry */
	BRAP_DSP_P_WriteMIT(mitPtr, 0x0, 4); mitPtr += 4;	
	BRAP_DSP_P_WriteMIT(mitPtr, decScratchBufSize, 4); mitPtr += 4;	
	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)scratchSpaceBuf, &physAddress);
	BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4); mitPtr += 4;
	BDBG_MSG(("DEC Scratch Buf: id(0x0) bufsize(%d) bufaddr(%x) physAddr(%x)", decScratchBufSize, scratchSpaceBuf, physAddress));


    /* Update MIT entry in the DRAM */
	for (count = 0; count < SIZE_OF_BAF_DOWNLOAD_ALLIGNED; count += 4)
	{
#ifdef BCHP_7411_VER /* Only for the 7411 */
		memVal = 0;
		memVal = (*(mitBase+count) << 24)|(*(mitBase+count+1) << 16)|(*(mitBase+count+2) << 8)|(*(mitBase+count+3));
		BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, dramAddr+count, memVal);
#else
		BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, dramAddr+count, *(uint32_t *)(mitBase + count)); 
#endif 
	}
    
	BRAP_P_DummyMemRead32((BARC_Handle)hDsp->hRegister, dramAddr+count);
#endif 

	BDBG_LEAVE (BRAP_DSP_P_AlgTblUpdateDRAMEntry);     
}

static void BRAP_DSP_P_FsUpdateDRAMEntry(
		BRAP_DSP_Handle hDsp,
		uint32_t dramAddr,
		uint32_t id,
		uint32_t size,
		uint32_t addr)
{
    unsigned char ui32AlgoDetail[SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED];
    int count;
    uint32_t physAddress;
#ifdef BCHP_7411_VER /* Only for the 7411 */
	uint32_t memVal;
#endif	
	unsigned char *mitPtr, *mitBase;
	BDBG_ENTER (BRAP_DSP_P_FsUpdateDRAMEntry); 

    mitBase = mitPtr = &ui32AlgoDetail[0];    
    BKNI_Memset(mitPtr, 0, SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED);

    /* Update MIT entry locally with FS algo details */
	BRAP_DSP_P_WriteMIT(mitPtr, id, 4); mitPtr += 4;	
	BRAP_DSP_P_WriteMIT(mitPtr, size, 4); mitPtr += 4;	

    if(0xFFFFFFFF == addr)
    {
        physAddress = addr;
    }
    else
    {
   	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)addr, &physAddress);
    } 
	BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4); mitPtr += 4;

    /* Update MIT entry in DRAM */
	for (count = 0; count < SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED; count += 4)
	{
#ifdef BCHP_7411_VER /* Only for the 7411 */
		memVal = 0;
		memVal = (*(mitBase+count) << 24)|(*(mitBase+count+1) << 16)|(*(mitBase+count+2) << 8)|(*(mitBase+count+3));
		BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, dramAddr+count, memVal);
#else
		BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, dramAddr+count, *(uint32_t *)(mitBase + count)); 
#endif 
	}

	BRAP_P_DummyMemRead32((BARC_Handle)hDsp->hRegister, dramAddr+count);
	BDBG_LEAVE (BRAP_DSP_P_FsUpdateDRAMEntry);     
}

/* BRAP_DSP_AllocateInterframeBuf: This function allocates a free Interframe buffer
 * that is required for decode operation.
 */
BERR_Code BRAP_DSP_AllocateInterframeBuf(
				BRAP_DSP_Handle		hDsp,
				unsigned int 	*puiInterframeBufIndex
)
{
	unsigned int i = 0;
	BERR_Code err = BERR_SUCCESS;
#if BRAP_DSP_P_DBG_MIT_TABLE==1 
	unsigned int memVal,cnt;
#endif
	
	BDBG_ASSERT(hDsp);
	BDBG_ASSERT(puiInterframeBufIndex);
	BDBG_ENTER (BRAP_DSP_AllocateInterframeBuf);     

	for (i = 0; i < BRAP_RM_P_MAX_DECODE_SUPPORTED; i++) {
		if (hDsp->bInterframeBufFreeStatus[i]) {
			hDsp->bInterframeBufFreeStatus[i] = false;
			*puiInterframeBufIndex = i;
			break;
		}
	}

	if (i == BRAP_RM_P_MAX_DECODE_SUPPORTED) {
		BDBG_ERR(("No interframe buffer available for the decode channel"));
		return BERR_TRACE(BRAP_ERR_RESOURCE_EXHAUSTED);
	}

    BDBG_MSG(("Interframe Buffer %d allocated.", *puiInterframeBufIndex));
    
	BDBG_LEAVE (BRAP_DSP_AllocateInterframeBuf);  
	return err;
}

/* BRAP_DSP_FreeInterframeBuf: Free interframe buffer */
void BRAP_DSP_FreeInterframeBuf(
		BRAP_DSP_Handle	hDsp,  
		unsigned int uiInterframeBufIndex
)
{
	BDBG_ASSERT(hDsp);
	BDBG_ENTER (BRAP_DSP_FreeInterframeBuf); 
	hDsp->bInterframeBufFreeStatus[uiInterframeBufIndex] = true;
    BDBG_MSG(("Interframe Buffer %d freed.", uiInterframeBufIndex));    
	BDBG_LEAVE (BRAP_DSP_FreeInterframeBuf); 
    return;
}

#if (BRAP_DSP_P_7401_NEWMIT == 1) 
BERR_Code BRAP_DSP_P_AllocatePpInterframeBuf(
				BRAP_DSP_Handle		hDsp,
				BRAP_DSPCHN_P_PpBranchInfo * sPpBranchInfo,
				unsigned int	* puiPpInterframeBuf
)
{
	unsigned int i = 0, numMaxPpStg, uiPpBrIndex = 0, uiPpStgIndex = 0;
	BERR_Code err = BERR_SUCCESS;

	unsigned int uiMaxPpBranches = BRAP_P_MAX_BRANCH_SUPPORTED;
	unsigned int uiMaxPpStgInBranch = BRAP_P_MAX_PP_SUPPORTED;
	
	BDBG_ASSERT( hDsp );
	BDBG_ASSERT( sPpBranchInfo );
	BDBG_ASSERT( puiPpInterframeBuf );

   	BDBG_ENTER (BRAP_DSP_P_AllocatePpInterframeBuf);  

	numMaxPpStg = BAF_MAX_CONTEXTS * uiMaxPpBranches * uiMaxPpStgInBranch;

	for( uiPpBrIndex = 0; uiPpBrIndex < BRAP_P_MAX_BRANCH_SUPPORTED; uiPpBrIndex++ )
	{
		for ( uiPpStgIndex = 0; uiPpStgIndex < BRAP_P_MAX_PP_SUPPORTED; uiPpStgIndex++ )
		{
			if( sPpBranchInfo[uiPpBrIndex].sPpStgSettings.ePpAlgo[uiPpStgIndex] != BRAP_DSPCHN_PP_Algo_eMax )
			{
				for ( ; i < numMaxPpStg; i++) 
				{
					if (hDsp->bPpInterframeBufFreeStatus[i]) 
					{
						hDsp->bPpInterframeBufFreeStatus[i] = false;
						puiPpInterframeBuf[uiPpBrIndex * BRAP_P_MAX_PP_SUPPORTED + uiPpStgIndex] = i;
						break;
					}
				}
				if (i == numMaxPpStg) {
					BDBG_ERR(("No interframe buffer available for the post processing stage"));
					return BERR_TRACE(BRAP_ERR_RESOURCE_EXHAUSTED);
				}
			}
			else
			{
				puiPpInterframeBuf[uiPpBrIndex * BRAP_P_MAX_PP_SUPPORTED + uiPpStgIndex] = BRAP_INVALID_VALUE;
			}
			BDBG_MSG(( "PpInterframeBuf[%d][%d] %d allocated", uiPpBrIndex, uiPpStgIndex, 
				puiPpInterframeBuf[uiPpBrIndex * BRAP_P_MAX_PP_SUPPORTED + uiPpStgIndex] ));
		}
	}
  

   	BDBG_LEAVE (BRAP_DSP_P_AllocatePpInterframeBuf); 
	return err;

}

void BRAP_DSP_P_FreePpInterframeBuf(
		BRAP_DSP_Handle	hDsp,  
		unsigned int uiPpInterframeBufIndex
)
{
	BDBG_ASSERT(hDsp);

	hDsp->bPpInterframeBufFreeStatus[uiPpInterframeBufIndex] = true;
}


/* BRAP_DSP_P_AllocateIsIsIfBuffers: Allocate interstage ( IS ), interstage interface ( ISIF ) buffers.
 * These buffers are allocated in pairs */
BERR_Code BRAP_DSP_P_AllocateIsIsIfBuffers(
		BRAP_DSP_Handle	hDsp,
		unsigned int		*puiBufferIndex
)
{
	unsigned int i = 0, maxNumBuffers = 0;
	unsigned int uiMaxPpBranches = BRAP_P_MAX_BRANCH_SUPPORTED;
    
	BDBG_ASSERT( hDsp );
	BDBG_ASSERT( puiBufferIndex );

   	BDBG_ENTER (BRAP_DSP_P_AllocateIsIsIfBuffers);     

	maxNumBuffers =  BAF_MAX_CONTEXTS * ( 1 + 2 * uiMaxPpBranches);

    BDBG_MSG(("maxNumBuffers=%d", maxNumBuffers));

	for( i = 0; i < maxNumBuffers; i++ )
	{
		if( hDsp->bInterstageBufFreeStatus[i] == true)
		{
			hDsp->bInterstageBufFreeStatus[i] = false;
			*puiBufferIndex = i;
			break;
		}
	}

	if (i == maxNumBuffers) {
		BDBG_ERR(("No interstage and interstage interface  buffers available for the decode channel"));
		return BERR_TRACE(BRAP_ERR_RESOURCE_EXHAUSTED);
	}


    BDBG_MSG(("interstage (IS), interstage interface (ISIF) Buffer %d allocated.", *puiBufferIndex));    

   	BDBG_LEAVE (BRAP_DSP_P_AllocateIsIsIfBuffers);   
	return BERR_SUCCESS;
	
}

BERR_Code BRAP_DSP_P_FreeIsIsIfBuffers(
		BRAP_DSP_Handle	hDsp,
		unsigned int		uiBufferIndex
)
{
	BDBG_ASSERT(hDsp);
   	BDBG_ENTER (BRAP_DSP_P_FreeIsIsIfBuffers);   
    
	hDsp->bInterstageBufFreeStatus[uiBufferIndex] = true;

    BDBG_MSG(("interstage (IS), interstage interface (ISIF) Buffer %d freed.", uiBufferIndex));        

   	BDBG_LEAVE (BRAP_DSP_P_FreeIsIsIfBuffers);      
	return BERR_SUCCESS;	
}

BERR_Code BRAP_DSP_P_InitPpInterframeBuffer(
				BRAP_DSP_Handle		hDsp,
				unsigned int	channelIndex,
				unsigned int	uiPpBrIndex,
				unsigned int	uiPpStgIndex,
				unsigned int 	uiInterframeBufIndex,
				BRAP_DSPCHN_PP_Algo ePpAlgo
				)
{
	BERR_Code err = BERR_SUCCESS;
	unsigned int dramPtr, interframeBufPtr, mitPtr, interframeBufSize;
	bool bUpdateMit = false;
    uint32_t physAdr=0;
	
	BDBG_ASSERT( hDsp );

   	BDBG_ENTER (BRAP_DSP_P_InitPpInterframeBuffer);       

	dramPtr = hDsp->memInfo.interFrameBufBasePtr[uiInterframeBufIndex];
	
	interframeBufPtr = dramPtr = hDsp->memInfo.ui32PpInterFrameBufBasePtr[uiInterframeBufIndex];
	interframeBufSize = hDsp->memInfo.ui32PpInterFrameBufSize;

	switch( ePpAlgo )
	{
		case BRAP_DSPCHN_PP_Algo_eSrc:        
        	BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_SRC_INTERFRAME_ID);
        	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)dramPtr, &physAdr); 
			BDBG_MSG(("Initializing SRC interframe at addr = 0x%08x", physAdr));
			bUpdateMit = true;
			break;
		case BRAP_DSPCHN_PP_Algo_eAacDownmix:        
            BDBG_MSG(("No interframe for AAC downmix "));			
			break;
		case BRAP_DSPCHN_PP_Algo_eDsola:        
        	BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_DSOLA_INTERFRAME_ID); 
        	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)dramPtr, &physAdr); 
			BDBG_MSG(("Initializing DSOLA interframe at addr = 0x%08x", physAdr));
			bUpdateMit = true;
			break;
#if (BRAP_AD_SUPPORTED == 1)      
		case BRAP_DSPCHN_PP_Algo_eAD_FadeCtrl:        
        	BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_AD_FADECTRL_INTERFRAME_ID); 
        	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)dramPtr, &physAdr); 
			BDBG_MSG(("Initializing AD FADE interframe at addr = 0x%08x", physAdr));
			bUpdateMit = true;
			break;
		case BRAP_DSPCHN_PP_Algo_eAD_PanCtrl:        
        	BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_AD_PANCTRL_INTERFRAME_ID); 
        	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)dramPtr, &physAdr); 
			BDBG_MSG(("Initializing AD PAN interframe at addr = 0x%08x", physAdr));
			bUpdateMit = true;
			break;            
#endif  
#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)      
		case BRAP_DSPCHN_PP_Algo_eDolbyVolume:        
        	BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_DOLBYVOLUME_INTERFRAME_ID);
        	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)dramPtr, &physAdr); 
			BDBG_MSG(("Initializing DoblyVolume interframe at addr = 0x%08x", physAdr));
			bUpdateMit = true;
			break;
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)      
		case BRAP_DSPCHN_PP_Algo_eSRS_TruVol:        
        	BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_SRSTRUVOL_INTERFRAME_ID);
        	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)dramPtr, &physAdr); 
			BDBG_MSG(("Initializing SRS TrueVolume interframe at addr = 0x%08x", physAdr));
			bUpdateMit = true;
			break;
#endif			
		default:
            /* Do nothing */
            break;
	}

    if ( true==bUpdateMit )
	{
		/* Update decoder interstage input buffer entry in MIT */
		mitPtr = hDsp->memInfo.mitDSPtr +  (4 + 4+ 4 + 4) + ( SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * 1 ) +
			 ( SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * (channelIndex * BAF_MAX_BRANCHES * (BAF_MAX_POSTPROCESS_STAGES + 1)) ) +
			 ( SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * uiPpBrIndex * (BAF_MAX_POSTPROCESS_STAGES + 1) ) +
			 ( SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * uiPpStgIndex  );


       	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)mitPtr, &physAdr);    
    	BDBG_MSG(("MIT Update for Post processing interframe Buffer: \nMIT entry address = 0x%08x (0x%08x) \n"
    		"Id = %d, Size = 0x%08x, Addr = 0x%08x\n",
    		mitPtr,physAdr,
    		uiPpStgIndex, 
    		interframeBufSize,
    		interframeBufPtr));

		BRAP_DSP_P_FsUpdateDRAMEntry(hDsp, 
				mitPtr,
				uiPpStgIndex, 
				interframeBufSize, 
				interframeBufPtr);
	}

   	BDBG_LEAVE (BRAP_DSP_P_InitPpInterframeBuffer);
	return err;
}

/* BRAP_DSP_P_UpdateMitForIsIsIfBuffers: Update MIT entries for interstage and interstage
 * interface buffers */
void	BRAP_DSP_P_UpdateMitForIsIsIfBuffers(
	BRAP_DSP_Handle	hDsp,
	unsigned int uiContextNumber,
	unsigned int uiDecInterstageOutputBuf,
	unsigned int * puiInterstageInputBuf,
	unsigned int * puiInterstageOutputBuf 
	)
{
	unsigned int uiDramAdr, uiMitBaseAdr, uiBufAdr;
	unsigned int uiPpBrIndex = 0, uiPpStgIndex = 0;
    uint32_t physAddr1=0, physAddr2;
	
	BDBG_ASSERT( puiInterstageInputBuf );
	BDBG_ASSERT( puiInterstageOutputBuf );

   	BDBG_ENTER (BRAP_DSP_P_UpdateMitForIsIsIfBuffers);

	uiMitBaseAdr = hDsp->memInfo.mitDSPtr;
	 
	/* Update decoder interstage input buffer entry in MIT */
	uiDramAdr = uiMitBaseAdr + (4 + 4+ 4 + 4) + ( SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * 1 ) +
		 ( SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * (BAF_MAX_CONTEXTS * BAF_MAX_BRANCHES * (BAF_MAX_POSTPROCESS_STAGES + 1)) ) +
		 ( SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * (uiContextNumber * BAF_MAX_BRANCHES * (BAF_MAX_POSTPROCESS_STAGES + 1)) ) +
		 ( SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * BAF_MAX_POSTPROCESS_STAGES );

   	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)uiDramAdr, &physAddr1);    
	BDBG_MSG(("MIT Update for Decoder Interstage Input Buffer: \nMIT entry address = 0x%08x (0x%08x) \n"
		"Id = %d, Size = 0x%08x, Addr = 0x%08x\n",
		uiDramAdr,physAddr1,
		0, 
		hDsp->memInfo.interStageBufSize,
		0xFFFFFFFF));	/* No input interstage buffer for decode stage */

	BRAP_DSP_P_FsUpdateDRAMEntry(
		hDsp,
		uiDramAdr,
		0,
		hDsp->memInfo.interStageBufSize,
		0xFFFFFFFF);
	
	/* Update decoder interstage output buffer entry in MIT */
	uiDramAdr += SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * BAF_MAX_CONTEXTS * BAF_MAX_BRANCHES * (BAF_MAX_POSTPROCESS_STAGES + 1);

	uiBufAdr = (BRAP_INVALID_VALUE==uiDecInterstageOutputBuf)? 
		BRAP_INVALID_VALUE: hDsp->memInfo.interStageBufBasePtr[uiDecInterstageOutputBuf];

   	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)uiDramAdr, &physAddr1);    
   	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)uiBufAdr, &physAddr2);    
	BDBG_MSG(("MIT Update for Decoder Interstage Output Buffer: \nMIT entry address = 0x%08x (0x%08x) \n"
		"Id = %d, Size = 0x%08x, Addr = 0x%08x (0x%08x)\n",
		uiDramAdr, physAddr1, 0, hDsp->memInfo.interStageBufSize, uiBufAdr, physAddr2));

	BRAP_DSP_P_FsUpdateDRAMEntry(hDsp, uiDramAdr,0, hDsp->memInfo.interStageBufSize, uiBufAdr);

	/* Update decoder interstage interface input buffer entry in MIT */
	uiDramAdr += SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * BAF_MAX_CONTEXTS * BAF_MAX_BRANCHES * (BAF_MAX_POSTPROCESS_STAGES + 1);

 
	BDBG_MSG(("MIT Update for Decoder Interstage Interface Input Buffer: \nMIT entry address = 0x%08x, \n"
		"IsIf In Id = %d, Size = 0x%08x, Addr = 0x%08x\n",
		uiDramAdr, 0, hDsp->memInfo.interStageInterfaceBufSize, 0xFFFFFFFF));	/* No input interstage interface buffer for decode stage */

	BRAP_DSP_P_FsUpdateDRAMEntry(hDsp, uiDramAdr, 0, hDsp->memInfo.interStageInterfaceBufSize,	0xFFFFFFFF);
	
	/* Update decoder interstage interface output buffer entry in MIT */
	uiDramAdr += SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * BAF_MAX_CONTEXTS * BAF_MAX_BRANCHES * (BAF_MAX_POSTPROCESS_STAGES + 1);

	uiBufAdr = (BRAP_INVALID_VALUE==uiDecInterstageOutputBuf)? 
		BRAP_INVALID_VALUE: hDsp->memInfo.interStageInterfaceBufBasePtr[uiDecInterstageOutputBuf];

    
   	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)uiDramAdr, &physAddr1);    
   	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)uiBufAdr, &physAddr2);   
	BDBG_MSG(("MIT Update for Decoder Interstage Interface Output Buffer: \nMIT entry address = 0x%08x (0x%08x), \n"
		"IsIf out Id = %d, Size = 0x%08x, Addr = 0x%08x (0x%08x)\n",
		uiDramAdr, physAddr1, 0, hDsp->memInfo.interStageInterfaceBufSize, uiBufAdr, physAddr2));

	BRAP_DSP_P_FsUpdateDRAMEntry(
		hDsp, uiDramAdr,	0, hDsp->memInfo.interStageInterfaceBufSize, uiBufAdr);
	
	for( uiPpBrIndex = 0; uiPpBrIndex < BRAP_P_MAX_BRANCH_SUPPORTED; uiPpBrIndex++ )
	{
		for( uiPpStgIndex = 0; uiPpStgIndex < BRAP_P_MAX_PP_SUPPORTED; uiPpStgIndex++ )
		{
			uiDramAdr = uiMitBaseAdr +  (4 + 4+ 4 + 4) + ( SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * 1 ) +
				 ( SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * (BAF_MAX_CONTEXTS * BAF_MAX_BRANCHES * (BAF_MAX_POSTPROCESS_STAGES + 1)) ) +
				 ( SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * (uiContextNumber * BAF_MAX_BRANCHES * (BAF_MAX_POSTPROCESS_STAGES + 1)) ) +
				 ( SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * ( uiPpBrIndex * (BAF_MAX_POSTPROCESS_STAGES + 1)) ) +
				 ( SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * uiPpStgIndex );

			uiBufAdr = (BRAP_INVALID_VALUE==puiInterstageInputBuf[uiPpBrIndex * BAF_MAX_BRANCHES + uiPpStgIndex]) ? 
				BRAP_INVALID_VALUE: hDsp->memInfo.interStageBufBasePtr[puiInterstageInputBuf[uiPpBrIndex * BAF_MAX_BRANCHES + uiPpStgIndex]];
			
			/* Update interstage input buffer entry in MIT for this post processing stage */
           	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)uiDramAdr, &physAddr1);    
           	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)uiBufAdr, &physAddr2);             
			BDBG_MSG(("MIT Update for Interstage Input Buffer: \n Post Process Branch Id = %d\n"
				"Post Processing Stage = %d, \nMIT entry address = 0x%08x (0x%08x), \n"
				"PP stage Id = %d, Size = 0x%08x, Addr = 0x%08x (0x%08x)\n",
				uiPpBrIndex,
				uiPpStgIndex,
				uiDramAdr, physAddr1,
				uiPpStgIndex, 
				hDsp->memInfo.interStageBufSize,
				uiBufAdr, physAddr2));

			BRAP_DSP_P_FsUpdateDRAMEntry(
				hDsp,
				uiDramAdr,
				uiPpStgIndex,
				hDsp->memInfo.interStageBufSize,
				uiBufAdr);
			
			/* Update interstage output buffer entry in MIT for this post processing stage */
			uiDramAdr += SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * BAF_MAX_CONTEXTS * BAF_MAX_BRANCHES * (BAF_MAX_POSTPROCESS_STAGES + 1);

			uiBufAdr = (BRAP_INVALID_VALUE==puiInterstageOutputBuf[uiPpBrIndex * BAF_MAX_BRANCHES + uiPpStgIndex]) ? 
				BRAP_INVALID_VALUE: hDsp->memInfo.interStageBufBasePtr[puiInterstageOutputBuf[uiPpBrIndex * BAF_MAX_BRANCHES + uiPpStgIndex]];

           	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)uiDramAdr, &physAddr1);    
           	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)uiBufAdr, &physAddr2);             
			BDBG_MSG(("MIT Update for Interstage Output Buffer: \n Post Process Branch Id = %d \n"
				"Post Processing Stage = %d, \n MIT entry address = 0x%08x (0x%08x), \n"
				"PP Id = %d, Size = 0x%08x, Addr = 0x%08x (0x%08x)\n",
				uiPpBrIndex,
				uiPpStgIndex,
				uiDramAdr, physAddr1, 
				uiPpStgIndex, 
				hDsp->memInfo.interStageBufSize,
				uiBufAdr, physAddr2));

			BRAP_DSP_P_FsUpdateDRAMEntry(
				hDsp,
				uiDramAdr,
				uiPpStgIndex,
				hDsp->memInfo.interStageBufSize,
				uiBufAdr);

			/* Update interstage interface input buffer entry in MIT for this post processing stage */
			uiDramAdr += SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * BAF_MAX_CONTEXTS * BAF_MAX_BRANCHES * (BAF_MAX_POSTPROCESS_STAGES + 1);

			uiBufAdr = (BRAP_INVALID_VALUE==puiInterstageInputBuf[uiPpBrIndex * BAF_MAX_BRANCHES + uiPpStgIndex]) ? 
				BRAP_INVALID_VALUE: hDsp->memInfo.interStageInterfaceBufBasePtr[puiInterstageInputBuf[uiPpBrIndex * BAF_MAX_BRANCHES + uiPpStgIndex]];

           	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)uiDramAdr, &physAddr1);    
           	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)uiBufAdr, &physAddr2);              
			BDBG_MSG(("MIT Update for Interstage Interface Input Buffer: \nPost Process Branch Id = %d\n"
				"Post Processing Stage = %d\n, MIT entry address = 0x%08x (0x%08x), \n"
				"PP Id = %d, Size = 0x%08x, Addr = 0x%08x (0x%08x)\n",
				uiPpBrIndex,
				uiPpStgIndex,
				uiDramAdr, physAddr1,
				uiPpStgIndex, 
				hDsp->memInfo.interStageInterfaceBufSize,
				uiBufAdr, physAddr2));

			BRAP_DSP_P_FsUpdateDRAMEntry(
				hDsp,
				uiDramAdr,
				uiPpStgIndex,
				hDsp->memInfo.interStageInterfaceBufSize,
				uiBufAdr);
			
			/* Update interstage interface output buffer entry in MIT for this post processing stage */
			uiDramAdr += SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * BAF_MAX_CONTEXTS * BAF_MAX_BRANCHES * (BAF_MAX_POSTPROCESS_STAGES + 1);

			uiBufAdr = (BRAP_INVALID_VALUE==puiInterstageOutputBuf[uiPpBrIndex * BAF_MAX_BRANCHES + uiPpStgIndex]) ? 
				BRAP_INVALID_VALUE: hDsp->memInfo.interStageInterfaceBufBasePtr[puiInterstageOutputBuf[uiPpBrIndex * BAF_MAX_BRANCHES + uiPpStgIndex]];

           	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)uiDramAdr, &physAddr1);    
           	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)uiBufAdr, &physAddr2);              
			BDBG_MSG(("MIT Update for Interstage Interface Output Buffer: \nPost Process Branch Id = %d\n"
				"Post Processing Stage = %d, \nMIT entry address = 0x%08x (0x%08x), \n"
				"PP Id = %d, Size = 0x%08x, Addr = 0x%08x (0x%08x)\n",
				uiPpBrIndex,
				uiPpStgIndex,
				uiDramAdr, physAddr1,
				uiPpStgIndex, 
				hDsp->memInfo.interStageInterfaceBufSize,
				uiBufAdr, physAddr2));

			BRAP_DSP_P_FsUpdateDRAMEntry(
				hDsp,
				uiDramAdr,
				uiPpStgIndex,
				hDsp->memInfo.interStageInterfaceBufSize,
				uiBufAdr);
			
		}
		
	}
   	BDBG_LEAVE (BRAP_DSP_P_UpdateMitForIsIsIfBuffers);
}

#endif /* 7401_NEWMIT */
/* BRAP_DSP_P_InitInterframeBuffer: This function initializes interframe buffer as per the 
 * input audio type and programs corresponding entry in MIT. */
BERR_Code BRAP_DSP_P_InitInterframeBuffer(
				BRAP_DSP_Handle		hDsp,
				unsigned int	channelIndex,
				unsigned int 	uiInterframeBufIndex,
				BRAP_DSPCHN_AudioType	eAudioType
				)
{
	BERR_Code err = BERR_SUCCESS;
	unsigned int dramPtr, interframeBufPtr, mitPtr, interframeBufSize;
#if ((BRAP_7401_FAMILY == 1) && (BRAP_DSP_P_7401_NEWMIT==0)	)
	BRAP_P_AlgoMemRequirement sAlgoMemReq;
    uint32_t physAddress;
	bool bSupportedPpAlgo[BRAP_DSPCHN_PP_Algo_eMax];
#endif

#if (BRAP_DSP_P_7401_NEWMIT==1)
	BRAP_P_Dec_AlgoMemRequirement sAlgoMemRequired;
#endif

	BDBG_ASSERT(hDsp);

	if ((channelIndex >= BRAP_DSP_P_MAX_AUDIO_CHANNELS) ||
		(uiInterframeBufIndex >= BRAP_RM_P_MAX_DECODE_SUPPORTED)) {
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	interframeBufPtr = dramPtr = hDsp->memInfo.interFrameBufBasePtr[uiInterframeBufIndex];
	interframeBufSize = hDsp->memInfo.interFrameBufSize;

	BRAP_P_GetAlgoMemRequirement (hDsp->hRap, eAudioType, &sAlgoMemRequired);		   	
	
	switch (eAudioType) {
		case BRAP_DSPCHN_AudioType_eMpeg:
			/* MPEG algorithm requires downloading of interframe buffer initialization
		   	file which is small compared to buffer size and initialize remaining buffer
		   	to zero. We will initialize entire buffer to zero first and then download
		   	interframe buffer initialization file */
			BRAP_P_DownloadDspFwInMem((BARC_Handle)hDsp->hRegister, NULL, dramPtr, sAlgoMemRequired.sDecodeMemReq.ui32InterframeBufSize);
			BRAP_DSP_P_CopyFWImageToMem (hDsp, dramPtr, BRAP_IMG_MPEG_INTERFRAME_ID); 
			break;
		case BRAP_DSPCHN_AudioType_eAc3:
#if defined ( BCHP_7411_VER ) || (BRAP_7401_FAMILY == 1)
			/* AC3 interframe buffer need to be initialized to all zeros except the first
		 	  32-bit word, which should be 0x00010001 */
			BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, dramPtr, 0x00010001);
			dramPtr+=4;
			BRAP_P_DownloadDspFwInMem((BARC_Handle)hDsp->hRegister, NULL, dramPtr, sAlgoMemRequired.sDecodeMemReq.ui32InterframeBufSize - 4);
#else			
			BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_DDP_INTERFRAME_ID);
#endif
			break;
		case BRAP_DSPCHN_AudioType_eAc3Lossless:
			BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_AC3LOSSLESS_INTERFRAME_ID);
			break;
		case BRAP_DSPCHN_AudioType_eAc3Plus:
			BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_DDP_INTERFRAME_ID);
			break;
		case BRAP_DSPCHN_AudioType_eAac:
			BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_AACHE_INTERFRAME_ID);
			break;
		case BRAP_DSPCHN_AudioType_eAacSbr:
#if ( BCHP_CHIP == 7400 )
			BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_AACHE_INTERFRAME_ID);
#else
			BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_AACHE_INTERFRAME_ID);
#endif
			break;
		case BRAP_DSPCHN_AudioType_eDts:
			BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_DTS_INTERFRAME_ID);
			break;
		case BRAP_DSPCHN_AudioType_eDtshd:
			BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_DTSHD_INTERFRAME_ID);
			break;
		case BRAP_DSPCHN_AudioType_eWmaStd:
			BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_WMA_STD_INTERFRAME_ID);
			break;
		case BRAP_DSPCHN_AudioType_eLpcmBd:
			BRAP_P_DownloadDspFwInMem((BARC_Handle)hDsp->hRegister, NULL, dramPtr, BRAP_DSP_P_BDLPCM_INTERFRAME_BUF_SIZE);
			BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_BD_LPCM_INTERFRAME_ID);
			break;
		case BRAP_DSPCHN_AudioType_eLpcmHdDvd:
			BRAP_P_DownloadDspFwInMem((BARC_Handle)hDsp->hRegister, NULL, dramPtr, BRAP_DSP_P_HDDVDLPCM_INTERFRAME_BUF_SIZE);
			BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_LPCM_HDDVD_INTERFRAME_ID);
			break;
		case BRAP_DSPCHN_AudioType_eLpcmDvd:
			BRAP_P_DownloadDspFwInMem((BARC_Handle)hDsp->hRegister, NULL, dramPtr, sAlgoMemRequired.sDecodeMemReq.ui32InterframeBufSize);
			BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_LPCM_DVD_INTERFRAME_ID);
			break;
		case BRAP_DSPCHN_AudioType_ePcmWav:
			BRAP_P_DownloadDspFwInMem((BARC_Handle)hDsp->hRegister, NULL, dramPtr, sAlgoMemRequired.sDecodeMemReq.ui32InterframeBufSize);
			BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_PCMWAV_INTERFRAME_ID);
			break;            
		case BRAP_DSPCHN_AudioType_eMlp:
            /* MLP interframe buffer needs to be initialized to all zeros except
               the first 64-bit word, which should be 0x00010001 0x00010001 */
			BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, dramPtr, 0x00010001);
			dramPtr+=4;            
			BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, dramPtr, 0x00010001);
			dramPtr+=4;            
			BRAP_P_DownloadDspFwInMem((BARC_Handle)hDsp->hRegister, NULL, dramPtr, BRAP_DSP_P_MLP_INTERFRAME_BUF_SIZE - 8);
			break;
		case BRAP_DSPCHN_AudioType_eWmaPro:
			BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, dramPtr, 0x00000001);
			dramPtr+=4;
			BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, dramPtr, 0x00000001);
			dramPtr+=4;            
			BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, dramPtr, 0x00000800);
			dramPtr+=4;            
			BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, dramPtr, 0x00000800);
			dramPtr+=4;            
			BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, dramPtr, 0x00000800);
			dramPtr+=4;         
			BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, dramPtr, 0x00000800);
			dramPtr+=4;            
			BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, dramPtr, 0x00000800);
			dramPtr+=4;            
			BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, dramPtr, 0x00000800);
			dramPtr+=4;            			
			BRAP_P_DownloadDspFwInMem((BARC_Handle)hDsp->hRegister, NULL, dramPtr,sAlgoMemRequired.sDecodeMemReq.ui32InterframeBufSize - 32);
			break;
		case BRAP_DSPCHN_AudioType_eDra:
			BRAP_P_DownloadDspFwInMem((BARC_Handle)hDsp->hRegister, NULL, dramPtr, sAlgoMemRequired.sDecodeMemReq.ui32InterframeBufSize);
			BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_DRA_INTERFRAME_ID);
            {
                uint32_t physAddress1;
                BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)(dramPtr), &physAddress1);
                BDBG_MSG(("InterframeBuffer dramPtr = 0x%x\n\n\n",physAddress1));
		    }

            break;
#ifdef RAP_SRSTRUVOL_CERTIFICATION            
              case BRAP_DSPCHN_AudioType_ePCM:
                break;
#endif 
		default:
			BDBG_ERR(("Algorithm Type %d is not supported",eAudioType));
	}

#if ((BRAP_7401_FAMILY == 1) && (BRAP_DSP_P_7401_NEWMIT == 0))
	err = BRAP_P_GetSelectedPpAlgos(hDsp->hRap, bSupportedPpAlgo);
	if (err!=BERR_SUCCESS)
		return BERR_TRACE(err);

    if (bSupportedPpAlgo[BRAP_DSPCHN_PP_Algo_eSrc] == true)
    {
		err = BRAP_P_GetAlgoMemRequirement (hDsp->hRap, eAudioType, &sAlgoMemReq);
        BDBG_MSG(("Algo interframe buf download starts at 0x%x, size for algo=0x%x", 
            interframeBufPtr, sAlgoMemReq.sDecodeMemReq.ui32InterframeBufSize));

        dramPtr = interframeBufPtr + sAlgoMemReq.sDecodeMemReq.ui32InterframeBufSize;
    
        BDBG_MSG(("Download SRC interframe buf at DRAM addr 0x%x", dramPtr));          
    	BRAP_DSP_P_CopyFWImageToMem(hDsp, dramPtr, BRAP_IMG_SRC_INTERFRAME_ID);
  
    	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)dramPtr, &physAddress); 
        BDBG_MSG(("physical add=0x%x", physAddress)); 
        BRAP_Write32(hDsp->hRegister, BCHP_AUD_DSP_CFG0_FSPB_SRC_CONFIG_CXT0, physAddress);   /* remove later */     

    }
#endif

#if (BRAP_DSP_P_7401_NEWMIT == 1) 
	/* Update decoder interframe buffer entry in MIT */
	mitPtr = hDsp->memInfo.mitDSPtr + (4 + 4 + 4 + 4) + ( SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * 1 ) +
		 ( SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * (channelIndex * BAF_MAX_BRANCHES * (BAF_MAX_POSTPROCESS_STAGES + 1)) ) +
		 ( SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * BAF_MAX_POSTPROCESS_STAGES );
#else
	mitPtr = hDsp->memInfo.mitDSPtr + (4 + 4) + ( SIZE_OF_BAF_HOST_BUF_INFO * 1 ) + channelIndex * SIZE_OF_BAF_HOST_BUF_INFO;
#endif

	BRAP_DSP_P_FsUpdateDRAMEntry(hDsp, 
			mitPtr,
			channelIndex, 
			interframeBufSize, 
			interframeBufPtr);

	return err;
}

/* BRAP_DSP_P_AllocateDecConfigBuf: This function allocates a free decoder config buffer
 * that is required for decode operation.
 */
BERR_Code BRAP_DSP_P_AllocateDecConfigBuf(
				BRAP_DSP_Handle		hDsp,
				unsigned int			*puiDecConfigBufIndex,
				unsigned int			*puiDecConfigBufStartAdr
				)
{
	unsigned int i = 0, numMaxBuf = 0;
	BERR_Code err = BERR_SUCCESS;
	
	BDBG_ASSERT(hDsp);
	BDBG_ASSERT(puiDecConfigBufIndex);
	BDBG_ASSERT(puiDecConfigBufStartAdr);

	numMaxBuf = BAF_MAX_CONTEXTS * BRAP_P_MAX_BRANCH_SUPPORTED * BRAP_P_MAX_PP_SUPPORTED + BAF_MAX_CONTEXTS;

	for (i = 0; i < numMaxBuf; i++) {
		if (hDsp->bDecConfigBufFreeStatus[i]) {
			hDsp->bDecConfigBufFreeStatus[i] = false;
			*puiDecConfigBufIndex = i;
			*puiDecConfigBufStartAdr = hDsp->memInfo.decConfigBufPtr[i];
			break;
		}
	}

	if (i == numMaxBuf) {
		BDBG_ERR(("No decoder configuration buffer available for the decode channel"));
		return BERR_TRACE(BRAP_ERR_RESOURCE_EXHAUSTED);
	}

	return err;
}

/* BRAP_DSP_P_FreeDecConfigBuf: Free decoder configuration buffer */
void BRAP_DSP_P_FreeDecConfigBuf(
		BRAP_DSP_Handle	hDsp,  
		unsigned int			puiDecConfigBufIndex
		)
{
	BDBG_ASSERT(hDsp);

	hDsp->bDecConfigBufFreeStatus[puiDecConfigBufIndex] = true;
}
	
/* Allocate config buffer array */
BERR_Code BRAP_DSP_P_AllocateConfigBufArray(
				BRAP_DSP_Handle		hDsp,
				unsigned int			*puiConfigBufArrayIndex,
				unsigned int			*puiConfigBufArrayStartAdr
				)
{
	unsigned int i = 0;
	BERR_Code err = BERR_SUCCESS;
	
	BDBG_ASSERT(hDsp);
	BDBG_ASSERT(puiConfigBufArrayIndex);
	BDBG_ASSERT(puiConfigBufArrayStartAdr);

	for (i = 0; i < BAF_MAX_CONTEXTS; i++) {
		if (hDsp->bConfigBufArrayFreeStatus[i]) {
			hDsp->bConfigBufArrayFreeStatus[i] = false;
			*puiConfigBufArrayIndex = i;
			*puiConfigBufArrayStartAdr = hDsp->memInfo.arrayOfConfigBufPtr[i];
			break;
		}
	}

	if (i == BAF_MAX_CONTEXTS) {
		BDBG_ERR(("No decoder configuration buffer available for the decode channel"));
		return BERR_TRACE(BRAP_ERR_RESOURCE_EXHAUSTED);
	}

	return err;

}

/* Free the given config buffer array */
void BRAP_DSP_P_FreeConfigBufArray(
		BRAP_DSP_Handle	hDsp,  
		unsigned int		puiDecConfigBufIndex
		)
{
	BDBG_ASSERT(hDsp);

	hDsp->bConfigBufArrayFreeStatus[puiDecConfigBufIndex] = true;
}

void BRAP_DSP_P_GetFwVersionInfo(
		BRAP_DSP_Handle hDsp,
		BRAP_DSPVersionInfo	*psDSPVersion
		)
{
	void *image;	
	BERR_Code rc;	
	const void *data;	
	const BIMG_Interface *iface = hDsp->hRap->sSettings.pImgInterface;	
	void *context = hDsp->hRap->sSettings.pImgContext;

	unsigned int uiCount = 0;
	unsigned int uiTemp = 0;
	uint32_t *pFWArray = NULL;
	sMITArrays *psMitArrays;

	/* Get the MIT Structure Array */
	psMitArrays = hDsp->psMitArrays;

	/* Read and assign the Version info for decoder algorithms */
	for (uiCount = 0; uiCount < BRAP_DSP_P_numDec; uiCount++)
	{
		BDBG_MSG(("BRAP_DSP_P_GetFwVersionInfo: DecAlgoID=%d", BRAP_DSP_P_supportedDecode[uiCount]));

		
		if (psMitArrays->decAlgoArrays[BRAP_DSP_P_supportedDecode[uiCount]] == 0xFFFFFFFF)
			continue;

		rc = iface->open (context, &image, psMitArrays->decAlgoArrays[BRAP_DSP_P_supportedDecode[uiCount]]);

		if (rc != BERR_SUCCESS) 
		{		
			BDBG_ERR(("\nBRAP_DSP_P_GetFwVersionInfo: Error in Opening the Image Interface for decoder algo\n"));;	
            BDBG_ERR(("ImgContext=%d, firmware_id=%d, 0x%x", context, psMitArrays->decAlgoArrays[BRAP_DSP_P_supportedDecode[uiCount]]));            
			iface->close (image);
		}

		rc = iface->next(image, 1, &data, BRAP_DSP_P_FIRMWARE_VERSION_SIZE);
		if (rc != BERR_SUCCESS) 
		{		
			BDBG_ERR(("\nError in fetching next chunk in Image Interface\n"));;	
			iface->close (image);
		}
		pFWArray = (uint32_t *)data;
		
		psDSPVersion->sDecAlgoVersion[BRAP_DSP_P_supportedDecode[uiCount]].ui32FwVersionField1
			= *pFWArray++;
		psDSPVersion->sDecAlgoVersion[BRAP_DSP_P_supportedDecode[uiCount]].ui32FwVersionField2
			= *pFWArray++;

		uiTemp = (*pFWArray & BRAP_DSP_P_DEF_HIGH_MASK) >> 16;
		uiTemp = ( uiTemp & BRAP_DSP_P_DEF_CUST_ID_MASK ) >> 8;
		psDSPVersion->sDecAlgoVersion[BRAP_DSP_P_supportedDecode[uiCount]].ui32CustomerID
			= uiTemp;

		uiTemp = (*pFWArray & BRAP_DSP_P_DEF_HIGH_MASK) >> 16;			
		uiTemp = ( uiTemp & BRAP_DSP_P_DEF_MAJOR_VER_MASK );
		psDSPVersion->sDecAlgoVersion[BRAP_DSP_P_supportedDecode[uiCount]].ui32MajorVersion
			= uiTemp;

		uiTemp = (*pFWArray & BRAP_DSP_P_DEF_LOW_MASK);
		psDSPVersion->sDecAlgoVersion[BRAP_DSP_P_supportedDecode[uiCount]].ui32MinorVersion
			= uiTemp;

		pFWArray++;

		uiTemp = (*pFWArray & BRAP_DSP_P_DEF_LOW_MASK);
		psDSPVersion->sDecAlgoVersion[BRAP_DSP_P_supportedDecode[uiCount]].ui32Year
			= uiTemp;
		uiTemp = (*pFWArray & BRAP_DSP_P_DEF_HIGH_MASK) >> 16;
		psDSPVersion->sDecAlgoVersion[BRAP_DSP_P_supportedDecode[uiCount]].ui32Date
			= uiTemp;
		BDBG_MSG(("FwVersionField1=%x",
		  psDSPVersion->sDecAlgoVersion[BRAP_DSP_P_supportedDecode[uiCount]].ui32FwVersionField1));
	       BDBG_MSG(("FwVersionField2=%x",
		  psDSPVersion->sDecAlgoVersion[BRAP_DSP_P_supportedDecode[uiCount]].ui32FwVersionField2));		
	       BDBG_MSG(("Customer ID=%x",
		  psDSPVersion->sDecAlgoVersion[BRAP_DSP_P_supportedDecode[uiCount]].ui32CustomerID));		
	       BDBG_MSG(("MajorVersion=%x",
		  psDSPVersion->sDecAlgoVersion[BRAP_DSP_P_supportedDecode[uiCount]].ui32MajorVersion));
	       BDBG_MSG(("MinorVersion=%x",
		  psDSPVersion->sDecAlgoVersion[BRAP_DSP_P_supportedDecode[uiCount]].ui32MinorVersion));
	       BDBG_MSG(("Month and Date=%x",
		  psDSPVersion->sDecAlgoVersion[BRAP_DSP_P_supportedDecode[uiCount]].ui32Date));
	       BDBG_MSG(("Year=%x",
		  psDSPVersion->sDecAlgoVersion[BRAP_DSP_P_supportedDecode[uiCount]].ui32Year));
	}

	/* Read and assign the Version info for Frame Sync algorithms */
	for (uiCount = 0; uiCount < BRAP_DSP_P_numFS; uiCount++)
	{
		BDBG_MSG(("BRAP_DSP_P_GetFwVersionInfo: BRAP_DSP_P_GetFwVersionInfo: FrameSyncAlgoID=%d", BRAP_DSP_P_supportedFS[uiCount]));

		if (psMitArrays->fsAlgoArrays[BRAP_DSP_P_supportedFS[uiCount]] == 0xFFFFFFFF)
			continue;

		rc = iface->open (context, &image, psMitArrays->fsAlgoArrays[BRAP_DSP_P_supportedFS[uiCount]]);

		if (rc != BERR_SUCCESS) 
		{		
			BDBG_ERR(("\nBRAP_DSP_P_GetFwVersionInfo: Error in Opening the Image Interface for framesync algo\n"));;	
            BDBG_ERR(("ImgContext=%d, firmware_id=%d, 0x%x", context, psMitArrays->fsAlgoArrays[BRAP_DSP_P_supportedFS[uiCount]]));            
			iface->close (image);
		}

		rc = iface->next(image, 1, &data, BRAP_DSP_P_FIRMWARE_VERSION_SIZE);
		if (rc != BERR_SUCCESS) 
		{		
			BDBG_ERR(("\nError in fetching next chunk in Image Interface\n"));;	
			iface->close (image);
		}

		pFWArray = (uint32_t *)data;

		psDSPVersion->sDecFsVersion[BRAP_DSP_P_supportedFS[uiCount]].ui32FwVersionField1
			= *pFWArray++;
		psDSPVersion->sDecFsVersion[BRAP_DSP_P_supportedFS[uiCount]].ui32FwVersionField2
			= *pFWArray++;

		uiTemp = (*pFWArray & BRAP_DSP_P_DEF_HIGH_MASK) >> 16;
		uiTemp = ( uiTemp & BRAP_DSP_P_DEF_CUST_ID_MASK ) >> 8;
		psDSPVersion->sDecFsVersion[BRAP_DSP_P_supportedFS[uiCount]].ui32CustomerID
			= uiTemp;

		uiTemp = (*pFWArray & BRAP_DSP_P_DEF_HIGH_MASK) >> 16;
		uiTemp = ( uiTemp & BRAP_DSP_P_DEF_MAJOR_VER_MASK );
		psDSPVersion->sDecFsVersion[BRAP_DSP_P_supportedFS[uiCount]].ui32MajorVersion
			= uiTemp;

		uiTemp = (*pFWArray & BRAP_DSP_P_DEF_LOW_MASK);		
		psDSPVersion->sDecFsVersion[BRAP_DSP_P_supportedFS[uiCount]].ui32MinorVersion
			= uiTemp;

		pFWArray++;

		uiTemp = (*pFWArray & BRAP_DSP_P_DEF_LOW_MASK);
		psDSPVersion->sDecFsVersion[BRAP_DSP_P_supportedFS[uiCount]].ui32Year
			= uiTemp;
		uiTemp = (*pFWArray & BRAP_DSP_P_DEF_HIGH_MASK) >> 16;
		psDSPVersion->sDecFsVersion[BRAP_DSP_P_supportedFS[uiCount]].ui32Date
			= uiTemp;
		BDBG_MSG(("FwVersionField1=%x",
		  psDSPVersion->sDecFsVersion[BRAP_DSP_P_supportedFS[uiCount]].ui32FwVersionField1));
	       BDBG_MSG(("FwVersionField2=%x",
		  psDSPVersion->sDecFsVersion[BRAP_DSP_P_supportedFS[uiCount]].ui32FwVersionField2));
	       BDBG_MSG(("Customer ID=%x",
		  psDSPVersion->sDecFsVersion[BRAP_DSP_P_supportedFS[uiCount]].ui32CustomerID));
		BDBG_MSG(("MajorVersion=%x",
		  psDSPVersion->sDecFsVersion[BRAP_DSP_P_supportedFS[uiCount]].ui32MajorVersion));
	       BDBG_MSG(("MinorVersion=%x",
		  psDSPVersion->sDecFsVersion[BRAP_DSP_P_supportedFS[uiCount]].ui32MinorVersion));
	       BDBG_MSG(("Month and Date=%x",
		  psDSPVersion->sDecFsVersion[BRAP_DSP_P_supportedFS[uiCount]].ui32Date));
	       BDBG_MSG(("Year=%x",
		  psDSPVersion->sDecFsVersion[BRAP_DSP_P_supportedFS[uiCount]].ui32Year));
	}

	/* Read and assign the Version info for Frame Sync algorithms */
	for (uiCount = 0; uiCount < BRAP_DSP_P_numPP; uiCount++)
	{
		BDBG_MSG(("BRAP_DSP_P_GetFwVersionInfo: PPAlgoID=%d", BRAP_DSP_P_supportedPP[uiCount]));

		if (psMitArrays->ppAlgoArrays[BRAP_DSP_P_supportedPP[uiCount]] == 0xFFFFFFFF)
			continue;

		rc = iface->open (context, &image, psMitArrays->ppAlgoArrays[BRAP_DSP_P_supportedPP[uiCount]]);

		if (rc != BERR_SUCCESS) 
		{		
			BDBG_ERR(("\nBRAP_DSP_P_GetFwVersionInfo: Error in Opening the Image Interface for PP algo\n"));;	
            BDBG_ERR(("ImgContext=%d, firmware_id=%d, 0x%x", context, psMitArrays->ppAlgoArrays[BRAP_DSP_P_supportedPP[uiCount]]));                        
			iface->close (image);
		}

		rc = iface->next(image, 1, &data, BRAP_DSP_P_FIRMWARE_VERSION_SIZE);
		if (rc != BERR_SUCCESS) 
		{		
			BDBG_ERR(("\nError in fetching next chunk in Image Interface\n"));;	
			iface->close (image);
		}
		pFWArray = (uint32_t *)data;

		psDSPVersion->sPPAlgoVersion[BRAP_DSP_P_supportedPP[uiCount]].ui32FwVersionField1
			= *pFWArray++;
		psDSPVersion->sPPAlgoVersion[BRAP_DSP_P_supportedPP[uiCount]].ui32FwVersionField2
			= *pFWArray++;

		uiTemp = (*pFWArray & BRAP_DSP_P_DEF_HIGH_MASK) >> 16;
		uiTemp = ( uiTemp & BRAP_DSP_P_DEF_CUST_ID_MASK ) >> 8;
		psDSPVersion->sPPAlgoVersion[BRAP_DSP_P_supportedPP[uiCount]].ui32CustomerID
			= uiTemp;

		uiTemp = (*pFWArray & BRAP_DSP_P_DEF_HIGH_MASK) >> 16;
		uiTemp = ( uiTemp & BRAP_DSP_P_DEF_MAJOR_VER_MASK );
		psDSPVersion->sPPAlgoVersion[BRAP_DSP_P_supportedPP[uiCount]].ui32MajorVersion
			= uiTemp;
		
		uiTemp = (*pFWArray & BRAP_DSP_P_DEF_LOW_MASK);		
		psDSPVersion->sPPAlgoVersion[BRAP_DSP_P_supportedPP[uiCount]].ui32MinorVersion
			= uiTemp;

		pFWArray++;

		uiTemp = (*pFWArray & BRAP_DSP_P_DEF_LOW_MASK);
		psDSPVersion->sPPAlgoVersion[BRAP_DSP_P_supportedPP[uiCount]].ui32Year
			= uiTemp;
		uiTemp = (*pFWArray & BRAP_DSP_P_DEF_HIGH_MASK) >> 16;
		psDSPVersion->sPPAlgoVersion[BRAP_DSP_P_supportedPP[uiCount]].ui32Date
			= uiTemp;
		BDBG_MSG(("FwVersionField1=%x",
		  psDSPVersion->sPPAlgoVersion[BRAP_DSP_P_supportedPP[uiCount]].ui32FwVersionField1));
	       BDBG_MSG(("FwVersionField2=%x",
		  psDSPVersion->sPPAlgoVersion[BRAP_DSP_P_supportedPP[uiCount]].ui32FwVersionField2));		
		BDBG_MSG(("CustomerID=%x",
		  psDSPVersion->sPPAlgoVersion[BRAP_DSP_P_supportedPP[uiCount]].ui32CustomerID));
		BDBG_MSG(("MajorVersion=%x",
		  psDSPVersion->sPPAlgoVersion[BRAP_DSP_P_supportedPP[uiCount]].ui32MajorVersion));
	       BDBG_MSG(("MinorVersion=%x",
		  psDSPVersion->sPPAlgoVersion[BRAP_DSP_P_supportedPP[uiCount]].ui32MinorVersion));
	       BDBG_MSG(("Month and Date=%x",
		  psDSPVersion->sPPAlgoVersion[BRAP_DSP_P_supportedPP[uiCount]].ui32Date));
	       BDBG_MSG(("Year=%x",
		  psDSPVersion->sPPAlgoVersion[BRAP_DSP_P_supportedPP[uiCount]].ui32Year));
	}

}

bool
BRAP_DSP_P_GetWatchdogRecoveryFlag(BRAP_DSP_Handle hDsp)
{
	return BRAP_P_GetWatchdogRecoveryFlag(hDsp->hRap);
}

BERR_Code 
BRAP_DSP_P_EnableDspWatchdogTimer (
						BRAP_DSP_Handle		hDsp,
						bool				bEnable)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t regVal = 0;
	BDBG_ASSERT(hDsp);

	if (bEnable) {
		/* Disable auto reload of count */
		regVal = regVal & ~(BCHP_MASK(AUD_DSP_CTRL0_WATCHDOG_TIMER, CM));
		/* Enable timer bit */
		regVal = regVal | BCHP_MASK(AUD_DSP_CTRL0_WATCHDOG_TIMER, ET);
		/* Program default watchdog count */
		regVal = (regVal & ~(BCHP_MASK(AUD_DSP_CTRL0_WATCHDOG_TIMER, COUNT)))
				| BCHP_FIELD_DATA(AUD_DSP_CTRL0_WATCHDOG_TIMER, COUNT, BRAP_DSP_P_DEFAULT_WATCHDOG_COUNT);
	}
	else {
		/* Disable timer bit */
		regVal = regVal & ~(BCHP_MASK(AUD_DSP_CTRL0_WATCHDOG_TIMER, ET));
	}
	
	BRAP_Write32(hDsp->hRegister, BCHP_AUD_DSP_CTRL0_WATCHDOG_TIMER, regVal);
	
	return err;
}

#ifndef BCHP_7411_VER
void BRAP_DSP_P_WatchDogResetHardware(BRAP_DSP_Handle hDsp)
{
	uint32_t regVal;


	/* Reset DSP processor and its peripherals */
	regVal = BRAP_Read32(hDsp->hRegister,BCHP_AUD_DSP_MISC_SOFT_RESET);
	regVal = (regVal & ~(BCHP_MASK(AUD_DSP_MISC_SOFT_RESET,RESET_SCBDMA0_B)))|
				(BCHP_FIELD_DATA(AUD_DSP_MISC_SOFT_RESET,RESET_SCBDMA0_B,0));
	regVal = (regVal & ~(BCHP_MASK(AUD_DSP_MISC_SOFT_RESET,RESET_PROC0_B)))|
				(BCHP_FIELD_DATA(AUD_DSP_MISC_SOFT_RESET,RESET_PROC0_B,0));
	regVal = (regVal & ~(BCHP_MASK(AUD_DSP_MISC_SOFT_RESET,RESET0_B)))|
				(BCHP_FIELD_DATA(AUD_DSP_MISC_SOFT_RESET,RESET0_B,0));
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_MISC_SOFT_RESET, regVal);

	/* Withdraw only RESET0_B reset */
	regVal = (regVal & ~(BCHP_MASK(AUD_DSP_MISC_SOFT_RESET,RESET0_B)))|
				(BCHP_FIELD_DATA(AUD_DSP_MISC_SOFT_RESET,RESET0_B,1));
	BRAP_Write32(hDsp->hRegister,BCHP_AUD_DSP_MISC_SOFT_RESET, regVal);
	BDBG_MSG(("\n**** Soft reset: reset0_b withdrawn = 0x%x",regVal));

	 /*Reset Dsp Hardware is set to True- This variable is used when application call start and stop decode when hardware is reset*/
	hDsp->hRap->bResetHwDone=true;
}
#endif

#ifndef BCHP_7411_VER /* For chips other than 7411 */
/* Updating FW download functions & MIT functions for Encoder */
static void BRAP_DSP_P_CopyEncFWToMem(BRAP_DSP_Handle hDsp, sEncMITArrays *psMitArrays) 
{
	uint32_t size;

	BDBG_ENTER (BRAP_DSP_P_CopyEncFWToMem);    
	BSTD_UNUSED(psMitArrays);
#ifdef MP3ENC_STANDALONE /* Changing for merged scheduler */
	BRAP_DSP_P_CopyFWImageToMem (hDsp, hDsp->memInfo.iboot_ptr, BRAP_IMG_ENC_IBOOT_ID); 
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_ENC_IBOOT_ID, &size);
	BDBG_MSG(("Downloading Encoder iboot at address 0x%x, size = 0x%x",
					hDsp->memInfo.iboot_ptr,
					size));
#endif

	BRAP_DSP_P_CopyFWImageToMem (hDsp, hDsp->memInfo.encscheduler_code_ptr, BRAP_IMG_ENC_SCHEDULER_CODE_ID);
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_ENC_SCHEDULER_CODE_ID, &size);
	BDBG_MSG(("Downloading Encoder scheduler code at address 0x%x, size = 0x%x",
					hDsp->memInfo.encscheduler_code_ptr,
					size));

	BRAP_DSP_P_CopyFWImageToMem (hDsp, hDsp->memInfo.encscheduler_data_ptr, BRAP_IMG_ENC_SCHEDULER_DATA_ID);
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_ENC_SCHEDULER_DATA_ID, &size);
	BDBG_MSG(("Downloading Encoder scheduler data at address 0x%x, size = 0x%x",
					hDsp->memInfo.encscheduler_data_ptr,
					size));

	BDBG_LEAVE (BRAP_DSP_P_CopyEncFWToMem);    
}

static void BRAP_DSP_P_EncInitializeArrays(BRAP_DSP_Handle hDsp, sEncMITArrays *psMitArrays)
{
	uint32_t count, size;

	/* Pre-initialize size arrays to zeros */
	for (count = 0; count < BAF_HOST_MAX_ENCODED_DL_MODULE; count++) {
		psMitArrays->encAlgoSizes[count] = 0x0;
	}
	for (count = 0; count < BAF_HOST_MAX_ENCODED_DL_MODULE; count++){	
			psMitArrays->encTblSizes[count] = 0x0;
		}
	for (count = 0; count < BAF_HOST_MAX_PRE_PROCESS_DL_MODULE; count++) {
		psMitArrays->encPrePAlgoSizes[count] = 0x0;
	}
	for (count = 0; count < BAF_HOST_MAX_PRE_PROCESS_DL_MODULE; count++) {
		psMitArrays->encPrePTblSizes[count] = 0x0;
	}
	for (count = 0; count < BAF_HOST_MAX_POST_PROCESS_DL_MODULE; count++){	
		psMitArrays->encPostPAlgoSizes[count] = 0x0;
	}
	for (count = 0; count < BAF_HOST_MAX_POST_PROCESS_DL_MODULE; count++){	
		psMitArrays->encPostPTblSizes[count] = 0x0;
	}
	for (count = 0; count < BAF_HOST_MAX_ENCODE_BUF_COUNT; count++){	
		psMitArrays->encIFrameSizes[count] = 0x0;
	}
	for (count = 0; count < BAF_HOST_MAX_ENCODE_BUF_COUNT; count++){	
		psMitArrays->encIStageSizes[count] = 0x0;
	}

	/* Pre-initialize pointer arrays to 0xFF */
	for (count = 0; count < BAF_HOST_MAX_ENCODED_DL_MODULE; count++) {
		psMitArrays->encAlgoArrays[count] = 0xFFFFFFFF;
	}
	for (count = 0; count < BAF_HOST_MAX_ENCODED_DL_MODULE; count++) {	
		psMitArrays->encTblArrays[count] = 0xFFFFFFFF;
	}
	for (count = 0; count < BAF_HOST_MAX_PRE_PROCESS_DL_MODULE; count++) {
		psMitArrays->encPrePAlgoArrays[count] = 0x0;
	}
	for (count = 0; count < BAF_HOST_MAX_PRE_PROCESS_DL_MODULE; count++) {
		psMitArrays->encPrePTblArrays[count] = 0x0;
	}
	for (count = 0; count < BAF_HOST_MAX_POST_PROCESS_DL_MODULE; count++){	
		psMitArrays->encPostPAlgoArrays[count] = 0x0;
	}
	for (count = 0; count < BAF_HOST_MAX_POST_PROCESS_DL_MODULE; count++){	
		psMitArrays->encPostPTblArrays[count] = 0x0;
	}
	for (count = 0; count < BAF_HOST_MAX_ENCODE_BUF_COUNT; count++){	
		psMitArrays->encIFrameArrays[count] = 0x0;
	}
	for (count = 0; count < BAF_HOST_MAX_ENCODE_BUF_COUNT; count++){	
		psMitArrays->encIStageArrays[count] = 0x0;
	}
	
	/* Now initialize the arrays */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_MP3_ENCODE_STG0_ID, &size);
	psMitArrays->encAlgoSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER] = BRAP_P_ALIGN(size, 2);
	psMitArrays->encAlgoArrays[BAF_ALGORITHM_MPEG1_L3_ENCODER] = (uint32_t) BRAP_IMG_MP3_ENCODE_STG0_ID;
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_MP3_ENCODE_STG1_ID, &size);
	psMitArrays->encAlgoSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+1] = BRAP_P_ALIGN(size, 2);
	psMitArrays->encAlgoArrays[BAF_ALGORITHM_MPEG1_L3_ENCODER+1] = (uint32_t) BRAP_IMG_MP3_ENCODE_STG1_ID;
/*	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_MP3_ENCODE_STG2_ID, &size);
	psMitArrays->encAlgoSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+2] = BRAP_P_ALIGN(size, 2);
	psMitArrays->encAlgoArrays[BAF_ALGORITHM_MPEG1_L3_ENCODER+2] = (uint32_t) BRAP_IMG_MP3_ENCODE_STG2_ID;*/

#if ( BCHP_CHIP == 7440 )
 	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DTS_ENCODE_CODE_ID, &size);
	psMitArrays->encAlgoSizes[BAF_ALGORITHM_DTS_ENCODER] = BRAP_P_ALIGN(size, 2);
	psMitArrays->encAlgoArrays[BAF_ALGORITHM_DTS_ENCODER] = (uint32_t) BRAP_IMG_DTS_ENCODE_CODE_ID;
#endif	

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_MP3_ENCODE_TABLE_ID, &size);
	psMitArrays->encTblSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER] = BRAP_P_ALIGN(size, 2);
	psMitArrays->encTblArrays[BAF_ALGORITHM_MPEG1_L3_ENCODER] = (uint32_t) BRAP_IMG_MP3_ENCODE_TABLE_ID;
	psMitArrays->encTblSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+1] = BRAP_P_ALIGN(size, 2);
	psMitArrays->encTblArrays[BAF_ALGORITHM_MPEG1_L3_ENCODER+1] = (uint32_t) BRAP_IMG_MP3_ENCODE_TABLE_ID;
/*	psMitArrays->encTblSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+2] = BRAP_P_ALIGN(size, 2);
	psMitArrays->encTblArrays[BAF_ALGORITHM_MPEG1_L3_ENCODER+2] = (uint32_t) BRAP_IMG_MP3_ENCODE_TABLE_ID;*/

#if ( BCHP_CHIP == 7440 )
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DTS_ENCODE_TABLE_ID, &size);
	psMitArrays->encTblSizes[BAF_ALGORITHM_DTS_ENCODER] = BRAP_P_ALIGN(size, 2);
	psMitArrays->encTblArrays[BAF_ALGORITHM_DTS_ENCODER] = (uint32_t) BRAP_IMG_DTS_ENCODE_TABLE_ID;
#endif

	/* Do similar as above for pre/post processing algos & other encoding algorithms*/

	/* Initialize the Iframe & IStage Buffers */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_MP3_ENCODE_INTER_FRAME_ID, &size);
	psMitArrays->encIFrameSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER] = BRAP_P_ALIGN(size, 2);
	psMitArrays->encIFrameArrays[BAF_ALGORITHM_MPEG1_L3_ENCODER] = (uint32_t) BRAP_IMG_MP3_ENCODE_INTER_FRAME_ID;

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_MP3_ENCODE_INTER_STAGE_ID, &size);
	psMitArrays->encIStageSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER] = BRAP_P_ALIGN(size, 2);
	psMitArrays->encIStageArrays[BAF_ALGORITHM_MPEG1_L3_ENCODER] = (uint32_t) BRAP_IMG_MP3_ENCODE_INTER_STAGE_ID;

#if ( BCHP_CHIP == 7440 )
 	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DTS_ENCODE_INTER_FRAME_ID, &size);
	psMitArrays->encIFrameSizes[BAF_ALGORITHM_DTS_ENCODER] = BRAP_P_ALIGN(size, 2);
	psMitArrays->encIFrameArrays[BAF_ALGORITHM_DTS_ENCODER] = (uint32_t) BRAP_IMG_DTS_ENCODE_INTER_FRAME_ID;

	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_DTS_ENCODE_INTER_STAGE_ID, &size);
	psMitArrays->encIStageSizes[BAF_ALGORITHM_DTS_ENCODER] = BRAP_P_ALIGN(size, 2);
	psMitArrays->encIStageArrays[BAF_ALGORITHM_DTS_ENCODER] = (uint32_t) BRAP_IMG_DTS_ENCODE_INTER_STAGE_ID;
#endif

#ifdef BRAP_DBG_ENABLED
	for (count = 0; count < BAF_HOST_MAX_ENCODED_DL_MODULE; count++)
		BDBG_MSG(("encAlgoSizes[%d] = 0x%x, encAlgoArrays[%d] = 0x%x\n", count, psMitArrays->encAlgoSizes[count], count, psMitArrays->encAlgoArrays[count]));
	
	for (count = 0; count < BAF_HOST_MAX_ENCODED_DL_MODULE; count++)
		BDBG_MSG(("encTblSizes[%d] = 0x%x, encTblArrays[%d] = 0x%x\n",count,psMitArrays->encTblSizes[count],count,psMitArrays->encTblArrays[count]));

	for (count = 0; count < BAF_HOST_MAX_PRE_PROCESS_DL_MODULE; count++)
		BDBG_MSG(("encPrePSizes[%d] = 0x%x, encPrePArrays[%d] = 0x%x\n", count, psMitArrays->encPrePSizes[count], count, psMitArrays->encPrePArrays[count]));
	
	for (count = 0; count < BAF_HOST_MAX_POST_PROCESS_DL_MODULE; count++)
		BDBG_MSG(("encPostPSizes[%d] = 0x%x, encPostPArrays[%d] = 0x%x\n",count,psMitArrays->encPostPSizes[count],count,psMitArrays->encPostPArrays[count]));
#endif

}

static BERR_Code BRAP_DSP_P_GetEncFwSize(BRAP_DSP_Handle hDsp,BRAP_DSP_P_MemInfo *memInfo)
{

	BERR_Code rc;
	uint32_t memReq = 0, count = 0, size = 0, sizemax = 0, stageCount = 0, algosize = 0, stgcnt = 0;
#ifdef MP3ENC_STANDALONE /* Changing for merged scheduler */
	if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_ENC_IBOOT_ID, &size)))
		return rc;
	memInfo.iboot_ptr=size;
#endif

	if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_ENC_SCHEDULER_CODE_ID, &size)))
		return rc;
	memInfo->encscheduler_code_ptr = size;

	if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_ENC_SCHEDULER_DATA_ID, &size)))
		return rc;
	memInfo->encscheduler_data_ptr = size;

	/* Calculate and allocate total memory required by encode algos */

	sizemax = 0;stgcnt = 0;
	for (count = 0; count < BRAP_DSP_P_numEnc; count++) {
		algosize = 0;
		for ( stageCount = 0; stageCount < BRAP_DSP_P_EncodeStages[count]; stageCount++ ) {
			if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_DSP_P_EncodeAlgoIDs[stgcnt + stageCount], &size)))
				return rc;
			algosize += size;
			}
		sizemax += algosize;
		stgcnt += stageCount;
	}
	
	memInfo->totalEncAlgoSize = sizemax;


	/* Calculate and allocate total memory required by encode tables */
	sizemax = 0;
	for (count = 0; count < BRAP_DSP_P_numEnc; count++) {
		if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_DSP_P_EncodeTblIDs[count], &size)))
			return rc;
		size *= BRAP_DSP_P_EncodeStages[count];/*have to ask... why multiplication???*/

		sizemax += size;
		size=0;
	}
	memInfo->totalEncTblSize = sizemax;

	
	/* Add memory alloc for preprocessor & postprocessor algos */
	/* Calculate and allocate total memory required by encode PreP algos */
	memReq = 0;
	for (count = 0; count < BAF_HOST_MAX_PRE_PROCESS_DL_MODULE; count++)
		memReq += hDsp->psEncMitArrays->encPrePAlgoSizes[count];
	
	memInfo->totalEncPrePAlgoSize = memReq;

	/* Calculate and allocate total memory required by encode PreP tables */
	memReq = 0;
	for (count = 0; count < BAF_HOST_MAX_PRE_PROCESS_DL_MODULE; count++)
		memReq += hDsp->psEncMitArrays->encPrePTblSizes[count];
	memInfo->totalEncPrePTblSize = memReq;

	/* Calculate and allocate total memory required by encode PostP algos */
	memReq = 0;
	for (count = 0; count < BAF_HOST_MAX_POST_PROCESS_DL_MODULE; count++)
		memReq += hDsp->psEncMitArrays->encPostPAlgoSizes[count];
	
	memInfo->totalEncPostPAlgoSize = memReq;

	
	/* Calculate and allocate total memory required by encode PostP tables */
	memReq = 0;
	for (count = 0; count < BAF_HOST_MAX_POST_PROCESS_DL_MODULE; count++)
		memReq += hDsp->psEncMitArrays->encPostPTblSizes[count];
	memInfo->totalEncPostPTblSize = memReq;

	/* Calculate and allocate total memory required by MIT and its sub structures */
/*	memReq = SIZE_OF_BAF_HOST_ENCODE_INFO 
			+ ((BAF_HOST_MAX_ENCODED_DL_MODULE + BAF_HOST_MAX_PRE_PROCESS_DL_MODULE + BAF_HOST_MAX_POST_PROCESS_DL_MODULE) * SIZE_OF_BAF_ENCODE_DOWNLOAD);
	
	memInfo.enctotalMITSize = memReq;
*/

	return BERR_SUCCESS; 
}

static BERR_Code BRAP_DSP_P_EncAllocMem(BRAP_DSP_Handle hDsp,sEncMITArrays *psMitArrays)
{
	BERR_Code rc;
	uint32_t memReq = 0, count = 0, size = 0, sizemax = 0, stageCount = 0, algosize = 0, stgcnt = 0;


if(hDsp->hRap->sSettings.bFwAuthEnable==false)
{
#ifdef MP3ENC_STANDALONE /* Changing for merged scheduler */
	if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_ENC_IBOOT_ID, &size)))
		return rc;
	hDsp->memInfo.iboot_ptr = (uint32_t)BRAP_P_AllocAligned(hDsp->hRap,
											size,
											8,
											0
#if (BRAP_SECURE_HEAP==1) 
											,false
#endif												
											
												);
	BDBG_MSG(("Memory allocation for encoder iboot: Address = 0x%08x, Size = 0x%08x", 
			hDsp->memInfo.iboot_ptr, size));
#endif

	if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_ENC_SCHEDULER_CODE_ID, &size)))
		return rc;
	hDsp->memInfo.encscheduler_code_ptr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
											size,
											8,
											0
#if (BRAP_SECURE_HEAP==1) 
											,false
#endif												
											
												);
	BDBG_MSG(("Memory allocation for encoder scheduler code: Address = 0x%08x, Size = 0x%08x",
			hDsp->memInfo.encscheduler_code_ptr, size));

	if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_ENC_SCHEDULER_DATA_ID, &size)))
		return rc;
	hDsp->memInfo.encscheduler_data_ptr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
											size,
											8,
											0
#if (BRAP_SECURE_HEAP==1) 
											,false
#endif												
											
												);
	BDBG_MSG(("Memory allocation for encoder scheduler data: Address = 0x%08x, Size = 0x%08x",
			hDsp->memInfo.encscheduler_data_ptr, size));
}
	sizemax = 0;
	for (count = 0; count < BRAP_DSP_P_numEnc; count++) {
		if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_DSP_P_EncodeScratchIDs[count], &size)))
		return rc;
		if(size > sizemax) sizemax = size;
	}
	hDsp->memInfo.encScratchBufsSize = sizemax;
	hDsp->memInfo.encScratchBufsBasePtr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
											sizemax,
											2,
											0
#if (BRAP_SECURE_HEAP==1) 
											,false
#endif												
											
												);
	BDBG_MSG(("Memory allocation for all encoder scratch buffers: Address = 0x%08x," 
			"Total size = 0x%08x, Size of each buffer = 0x%08x",
			hDsp->memInfo.encScratchBufsBasePtr, sizemax,
			size));

	/* Calculate & allocate total memory required by encoder iframe buffers */
	sizemax = 0;
	for (count = 0; count < BRAP_DSP_P_numEnc; count++) {
		if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_DSP_P_EncodeIFrameIDs[count], &size)))
			return rc;
		if(size > sizemax) sizemax = size;
	}
	memReq = sizemax * BAF_ENCODED_NUM_CHANNELS; /* Maximum no. of parallel encodings supported */
	
	hDsp->memInfo.totalEncIFrameSize = memReq;

	hDsp->memInfo.encIFrameBasePtr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
											memReq,
											2,
											0
#if (BRAP_SECURE_HEAP==1) 
											,false
#endif												
											
												);
	BDBG_MSG(("Memory allocation for all IFrame buffers: Address = 0x%08x, Size = 0x%08x",
			hDsp->memInfo.encIFrameBasePtr, memReq));

	/* Calculate & allocate total memory required by encoder istage buffers */
	sizemax = 0;
	for (count = 0; count < BRAP_DSP_P_numEnc; count++) {
		if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_DSP_P_EncodeIStageIDs[count], &size)))
			return rc;
		if(size > sizemax) sizemax = size;
	}
	memReq = sizemax * BAF_ENCODED_NUM_CHANNELS; /* Maximum no. of parallel encodings supported */

	hDsp->memInfo.totalEncIStageSize = memReq;

	hDsp->memInfo.encIStageBasePtr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
											memReq,
										2,
										0
#if (BRAP_SECURE_HEAP==1) 
										,false
#endif												
											
												);
	BDBG_MSG(("Memory allocation for all IFrame buffers: Address = 0x%08x, Size = 0x%08x",
			hDsp->memInfo.encIStageBasePtr, memReq));


if(hDsp->hRap->sSettings.bFwAuthEnable==false)
{	
	/* Calculate and allocate total memory required by encode algos */
	sizemax = 0;stgcnt = 0;
	for (count = 0; count < BRAP_DSP_P_numEnc; count++) {
		algosize = 0;
		for ( stageCount = 0; stageCount < BRAP_DSP_P_EncodeStages[count]; stageCount++ ) {
			if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_DSP_P_EncodeAlgoIDs[stgcnt + stageCount], &size)))
				return rc;
			algosize += size;
			}
		if(algosize > sizemax) sizemax = algosize;
		stgcnt += stageCount;
	}
	memReq = sizemax * BAF_ENCODED_NUM_CHANNELS; /* Maximum no. of parallel encodings supported */
	
	hDsp->memInfo.totalEncAlgoSize = memReq;

	hDsp->memInfo.encAlgoBasePtr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
											memReq,
											2,
											0
#if (BRAP_SECURE_HEAP==1) 
											,false
#endif												
											
												);
	BDBG_MSG(("Memory allocation for all encode executables: Address = 0x%08x, Size = 0x%08x",
			hDsp->memInfo.encAlgoBasePtr, memReq));

	/* Calculate and allocate total memory required by encode tables */
	sizemax = 0;
	for (count = 0; count < BRAP_DSP_P_numEnc; count++) {
		if (BERR_SUCCESS != (rc = BRAP_DSP_P_GetFWSize (hDsp, BRAP_DSP_P_EncodeTblIDs[count], &size)))
			return rc;
		size *= BRAP_DSP_P_EncodeStages[count];
		if(size > sizemax) sizemax = size;
	}
	memReq = sizemax * BAF_ENCODED_NUM_CHANNELS; /* Maximum no. of parallel encodings supported */
	hDsp->memInfo.totalEncTblSize = memReq;

	hDsp->memInfo.encTblBasePtr =  (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
											memReq,
											2,
											0
#if (BRAP_SECURE_HEAP==1) 
											,false
#endif												
												
												);
	BDBG_MSG(("Memory allocation for all encode tables: Address = 0x%08x, Size = 0x%08x",
			hDsp->memInfo.encTblBasePtr, memReq));

	/* Add memory alloc for preprocessor & postprocessor algos */
	/* Calculate and allocate total memory required by encode PreP algos */
	memReq = 0;
	for (count = 0; count < BAF_HOST_MAX_PRE_PROCESS_DL_MODULE; count++)
		memReq += psMitArrays->encPrePAlgoSizes[count];
	
	hDsp->memInfo.totalEncPrePAlgoSize = memReq;

	if(hDsp->memInfo.totalEncPrePAlgoSize)
		hDsp->memInfo.encPrePAlgoBasePtr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
												memReq,
												2,
												0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
											
												);
	BDBG_MSG(("Memory allocation for all encode PreP executables: Address = 0x%08x, Size = 0x%08x",
			hDsp->memInfo.encPrePAlgoBasePtr, memReq));

	/* Calculate and allocate total memory required by encode PreP tables */
	memReq = 0;
	for (count = 0; count < BAF_HOST_MAX_PRE_PROCESS_DL_MODULE; count++)
		memReq += psMitArrays->encPrePTblSizes[count];
	hDsp->memInfo.totalEncPrePTblSize = memReq;

	if(hDsp->memInfo.totalEncPrePTblSize)
		hDsp->memInfo.encPrePTblBasePtr =  (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
												memReq,
												2,
												0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
												
												);
	BDBG_MSG(("Memory allocation for all encode PreP tables: Address = 0x%08x, Size = 0x%08x",
			hDsp->memInfo.encPrePTblBasePtr, memReq));

	/* Calculate and allocate total memory required by encode PostP algos */
	memReq = 0;
	for (count = 0; count < BAF_HOST_MAX_POST_PROCESS_DL_MODULE; count++)
		memReq += psMitArrays->encPostPAlgoSizes[count];
	
	hDsp->memInfo.totalEncPostPAlgoSize = memReq;

	if(hDsp->memInfo.totalEncPostPAlgoSize)
		hDsp->memInfo.encPostPAlgoBasePtr = (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
												memReq,
												2,
												0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
												
												);
	BDBG_MSG(("Memory allocation for all encode PostP executables: Address = 0x%08x, Size = 0x%08x",
			hDsp->memInfo.encPostPAlgoBasePtr, memReq));

	/* Calculate and allocate total memory required by encode PostP tables */
	memReq = 0;
	for (count = 0; count < BAF_HOST_MAX_POST_PROCESS_DL_MODULE; count++)
		memReq += psMitArrays->encPostPTblSizes[count];
	hDsp->memInfo.totalEncPostPTblSize = memReq;

	if(hDsp->memInfo.totalEncPostPAlgoSize)
		hDsp->memInfo.encPostPTblBasePtr =  (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
												memReq,
												2,
												0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
											
												);
	BDBG_MSG(("Memory allocation for all encode PostP tables: Address = 0x%08x, Size = 0x%08x",
			hDsp->memInfo.encPostPTblBasePtr, memReq));

}
	/* Calculate and allocate total memory required by MIT and its sub structures */
	memReq = SIZE_OF_BAF_HOST_ENCODE_INFO 
			+ ((BAF_HOST_MAX_ENCODED_DL_MODULE + BAF_HOST_MAX_PRE_PROCESS_DL_MODULE + BAF_HOST_MAX_POST_PROCESS_DL_MODULE) * SIZE_OF_BAF_ENCODE_DOWNLOAD);
	
	hDsp->memInfo.enctotalMITSize = memReq;

	hDsp->memInfo.encmitDSPtr =  (uint32_t) BRAP_P_AllocAligned(hDsp->hRap,
											memReq,
											2,
											0
#if (BRAP_SECURE_HEAP==1) 
												,false
#endif												
											
												);
	BDBG_MSG(("Memory allocation for encoder master index table: Address = 0x%08x, Size = 0x%08x",
		hDsp->memInfo.encmitDSPtr, memReq));

	return BERR_SUCCESS; 
}

static void BRAP_DSP_P_EncFreeMem(BRAP_DSP_Handle hDsp)
{
	BDBG_ASSERT(hDsp);

	/* Free all the memory allocated in BRAP_DSP_P_AllocMem */
#if (BRAP_SECURE_HEAP == 1 )	
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encScratchBufsBasePtr,false);
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encscheduler_data_ptr,false);
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encscheduler_code_ptr,false);
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encAlgoBasePtr,false);
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encTblBasePtr,false);
	if(hDsp->memInfo.totalEncPrePAlgoSize)
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encPrePAlgoBasePtr,false);
	if(hDsp->memInfo.totalEncPrePTblSize)
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encPrePTblBasePtr,false);
	if(hDsp->memInfo.totalEncPostPAlgoSize)
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encPostPAlgoBasePtr,false);
	if(hDsp->memInfo.totalEncPostPTblSize)
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encPostPTblBasePtr,false);
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encIFrameBasePtr,false);
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encIStageBasePtr,false);
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encmitDSPtr,false);
#else
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encScratchBufsBasePtr);
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encscheduler_data_ptr);
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encscheduler_code_ptr);
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encAlgoBasePtr);
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encTblBasePtr);
	if(hDsp->memInfo.totalEncPrePAlgoSize)
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encPrePAlgoBasePtr);
	if(hDsp->memInfo.totalEncPrePTblSize)
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encPrePTblBasePtr);
	if(hDsp->memInfo.totalEncPostPAlgoSize)
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encPostPAlgoBasePtr);
	if(hDsp->memInfo.totalEncPostPTblSize)
		BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encPostPTblBasePtr);
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encIFrameBasePtr);
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encIStageBasePtr);
	BRAP_P_Free(hDsp->hRap, (void *) hDsp->memInfo.encmitDSPtr);
#endif	
	return;
}

static void BRAP_DSP_P_CreateEncMIT(BRAP_DSP_Handle hDsp, sEncMITArrays *psMitArrays)
{
	uint32_t mitIndexPtr, count, stageCount;
	uint32_t algoPtr, algoTblPtr, physAddress, algoIFramePtr, algoIStagePtr;
	uint32_t tempEncoderId, size;
	unsigned char *mitPtr, *mitBase;
	unsigned char *encStructBasePtr;

	/* First create MIT table in system memory and then download in device memory */ 
	mitBase = mitPtr = (unsigned char *) BKNI_Malloc(hDsp->memInfo.enctotalMITSize);

	/* Initialize BAF_HostInfo structure */
	BRAP_DSP_P_WriteMIT(mitPtr, 0x0, 4); mitPtr += 4;	/* MIT ID */
	BRAP_DSP_P_WriteMIT(mitPtr, 0x0, 4); mitPtr += 4;	/* MIT version */

	BRAP_DSP_P_WriteMIT(mitPtr, 0x0, 4);	mitPtr += 4;				/* scheduler data buf ID */
	BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_ENC_SCHEDULER_DATA_ID, &size);
	BRAP_DSP_P_WriteMIT(mitPtr, size, 4); mitPtr += 4;	/* scheduler data buf size */
	BDBG_MSG(("hDsp->memInfo.encscheduler_data_ptr : %x", hDsp->memInfo.encscheduler_data_ptr));
	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.encscheduler_data_ptr, &physAddress);
	BDBG_MSG(("hDsp->memInfo.encscheduler_data_ptr converted : %x", physAddress));
	BRAP_DSP_P_WriteMIT(mitPtr, physAddress,4);	mitPtr += 4;	/* sheduler data buf pointer */

	BRAP_DSP_P_WriteMIT(mitPtr, 0x0, 4);	mitPtr += 4;				/* Enc scratch buf ID */
	BRAP_DSP_P_WriteMIT(mitPtr, hDsp->memInfo.encScratchBufsSize, 4); mitPtr += 4;	/* Enc scratch buf size */
	BDBG_MSG(("hDsp->memInfo.encScratchBufsBasePtr : %x", hDsp->memInfo.encScratchBufsBasePtr));
	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)hDsp->memInfo.encScratchBufsBasePtr, &physAddress);
	BDBG_MSG(("hDsp->memInfo.encScratchBufsBasePtr converted : %x", physAddress));
	BRAP_DSP_P_WriteMIT(mitPtr, physAddress,4);mitPtr += 4;	/* Enc scratch buf pointer */

	/* Encoding PreProcessing algorithms */
	mitIndexPtr = hDsp->memInfo.encmitDSPtr + SIZE_OF_BAF_HOST_ENCODE_INFO;
	for (count = 0; count < BAF_HOST_MAX_PRE_PROCESS_DL_MODULE; count++) {
	BDBG_MSG(("hDsp->memInfo.encmitDSPtr : %x", mitIndexPtr));
	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)mitIndexPtr, &physAddress);
	BDBG_MSG(("hDsp->memInfo.encmitDSPtr Converted : %x", physAddress));
		BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4);	mitPtr += 4; /* pPreProcess[count] struct pointers */
		mitIndexPtr += SIZE_OF_BAF_ENCODE_DOWNLOAD;
	}

	/* Encoding algorithms */
	mitIndexPtr = hDsp->memInfo.encmitDSPtr + SIZE_OF_BAF_HOST_ENCODE_INFO + SIZE_OF_BAF_ENCODE_DOWNLOAD*BAF_HOST_MAX_PRE_PROCESS_DL_MODULE ;
	for (count = 0; count < BAF_HOST_MAX_ENCODED_DL_MODULE; count++) {
	BDBG_MSG(("hDsp->memInfo.encmitDSPtr : %x", mitIndexPtr));
	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)mitIndexPtr, &physAddress);
	BDBG_MSG(("hDsp->memInfo.encmitDSPtr Converted : %x", physAddress));
		BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4);	mitPtr += 4; /* pEncode[count] struct pointers */
		mitIndexPtr += SIZE_OF_BAF_ENCODE_DOWNLOAD;
	}

	/* Encoding PostProcessing algorithms */
	mitIndexPtr = hDsp->memInfo.encmitDSPtr + SIZE_OF_BAF_HOST_ENCODE_INFO + SIZE_OF_BAF_ENCODE_DOWNLOAD*BAF_HOST_MAX_PRE_PROCESS_DL_MODULE  + SIZE_OF_BAF_ENCODE_DOWNLOAD*BAF_HOST_MAX_ENCODED_DL_MODULE;
	for (count = 0; count < BAF_HOST_MAX_POST_PROCESS_DL_MODULE; count++) {
	BDBG_MSG(("hDsp->memInfo.encmitDSPtr : %x", mitIndexPtr));
	BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)mitIndexPtr, &physAddress);
	BDBG_MSG(("hDsp->memInfo.encmitDSPtr Converted : %x", physAddress));
		BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4);	mitPtr += 4; /* pPreProcess[count] struct pointers */
		mitIndexPtr += SIZE_OF_BAF_ENCODE_DOWNLOAD;
	}

#ifdef BRAP_DBG_ENABLED
	BDBG_MSG(("========Master Index Table============"));
	for (count = 0; count < SIZE_OF_BAF_HOST_ENCODE_INFO; count++) 
		BDBG_MSG(("MIT[%d] = 0x%02x",count,*(mitBase+count)));
	BDBG_MSG(("========End of Master Index Table============"));
#endif

	/* Add here for Encoder PreProcessing Algos */

	/* Initialize BAF_Download structure for encode algos */
	algoPtr = hDsp->memInfo.encAlgoBasePtr;
	algoTblPtr = hDsp->memInfo.encTblBasePtr;
	algoIFramePtr = hDsp->memInfo.encIFrameBasePtr;
	algoIStagePtr = hDsp->memInfo.encIStageBasePtr;
	encStructBasePtr = mitBase + SIZE_OF_BAF_HOST_ENCODE_INFO + SIZE_OF_BAF_ENCODE_DOWNLOAD*BAF_HOST_MAX_PRE_PROCESS_DL_MODULE;
	for (count = 0; count < BRAP_DSP_P_numEnc; count++) {
		for ( stageCount = 0; stageCount < BRAP_DSP_P_EncodeStages[count]; stageCount++ )
		{
			mitPtr = encStructBasePtr +  ( BRAP_DSP_P_supportedEncode[count] + stageCount )* SIZE_OF_BAF_ENCODE_DOWNLOAD;

			tempEncoderId = ( BRAP_DSP_P_supportedEncode[count] + stageCount ) | ( BRAP_DSP_P_EncodeStages[count] << 28 );

			BDBG_MSG(("Count %x, StageCount %x, TempEncoderId %x ", count, stageCount, tempEncoderId));

			/* TBD: Check for invalid Encoder size and address */	
			BRAP_DSP_P_WriteMIT(mitPtr, tempEncoderId, 4); mitPtr += 4;	/* encoder ID */
			BRAP_DSP_P_WriteMIT(mitPtr, psMitArrays->encAlgoSizes[BRAP_DSP_P_supportedEncode[count]+stageCount], 4); mitPtr += 4;		/* Decode algo size */
			BDBG_MSG(("Algoptr : %x, StageCount : %d", algoPtr, stageCount));
			BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)algoPtr, &physAddress);
			BDBG_MSG(("Algoptr Converted : %x, StageCount : %d", physAddress, stageCount));
			BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4);	mitPtr += 4;									/* pointer Decode algo */
			algoPtr += psMitArrays->encAlgoSizes[BRAP_DSP_P_supportedEncode[count]+stageCount];
			BRAP_DSP_P_WriteMIT(mitPtr, 0x0, 4); mitPtr += 4;									/* Decode table ID */
			BRAP_DSP_P_WriteMIT(mitPtr, psMitArrays->encTblSizes[BRAP_DSP_P_supportedEncode[count]+stageCount], 4); mitPtr += 4;				/* Decode table size */
			BDBG_MSG(("algoTblPtr : %x", algoTblPtr));
			BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)algoTblPtr, &physAddress);
			BDBG_MSG(("algoTblPtr Converted : %x", physAddress));
			BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4);mitPtr += 4;								/* pointer to decode table */
			algoTblPtr += psMitArrays->encTblSizes[BRAP_DSP_P_supportedEncode[count]+stageCount];

		/* for instance 1 */
			BRAP_DSP_P_WriteMIT(mitPtr, 0x0, 4); mitPtr += 4;									/* IFrame 0 ID */
			BRAP_DSP_P_WriteMIT(mitPtr, psMitArrays->encIFrameSizes[BRAP_DSP_P_supportedEncode[count]+stageCount], 4); mitPtr += 4;		/* Decode IFrame size */
			BDBG_MSG(("AlgoIFrameptr : %x, StageCount : %d", algoIFramePtr, stageCount));
			BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)algoIFramePtr, &physAddress);
			BDBG_MSG(("AlgoIFrameptr Converted : %x, StageCount : %d", physAddress, stageCount));
			BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4);	mitPtr += 4; /* IFrame 0 addr */
			algoIFramePtr += psMitArrays->encIFrameSizes[BRAP_DSP_P_supportedEncode[count]+stageCount];

			mitPtr += 12; /* skipping instance 2 as of now */

		/* for instance 1 */
			BRAP_DSP_P_WriteMIT(mitPtr, 0x0, 4); mitPtr += 4;									/* IStage 0 ID */
			BRAP_DSP_P_WriteMIT(mitPtr, psMitArrays->encIStageSizes[BRAP_DSP_P_supportedEncode[count]+stageCount], 4); mitPtr += 4;		/* Decode IStage size */
			BDBG_MSG(("AlgoIStageptr : %x, StageCount : %d", algoIStagePtr, stageCount));
			BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)algoIStagePtr, &physAddress);
			BDBG_MSG(("AlgoIStageptr Converted : %x, StageCount : %d", physAddress, stageCount));
			BRAP_DSP_P_WriteMIT(mitPtr, physAddress, 4);	mitPtr += 4; /* IStage 0 addr */
			algoIStagePtr += psMitArrays->encIStageSizes[BRAP_DSP_P_supportedEncode[count]+stageCount];

			mitPtr += 12; /* skipping instance 2 as of now */
		}
	}

	/* Add here for Encoder PostProcessing Algos */

#ifdef BRAP_DBG_ENABLED
	BDBG_MSG(("========Encode Structures============"));
	for (count = 0; count < BAF_HOST_MAX_ENCODED_DL_MODULE * SIZE_OF_BAF_ENCODE_DOWNLOAD; count++)
		BDBG_MSG(("Enc[%d] = 0x%x",(count/SIZE_OF_BAF_ENCODE_DOWNLOAD),*(encStructBasePtr + count)));
	BDBG_MSG(("========End of Encode Structures============"));
#endif
	
	/* Download MIT table in device memory */
	for (count = 0; count < BRAP_P_ALIGN(hDsp->memInfo.enctotalMITSize, 2); count += 4){
		BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, hDsp->memInfo.encmitDSPtr+count, *(uint32_t *)(mitBase + count)); 
	}
	BRAP_P_DummyMemRead32((BARC_Handle)hDsp->hRegister, hDsp->memInfo.encmitDSPtr+count);

#ifdef BRAP_DBG_ENABLED
	BDBG_MSG(("MIT read from DRAM"));
	for (count = 0; count < BRAP_P_ALIGN(hDsp->memInfo.enctotalMITSize, 2); count += 4){
		memVal = BRAP_MemRead32((BARC_Handle)hDsp->hRegister,hDsp->memInfo.encmitDSPtr+count);
		BDBG_MSG(("%08x : %08x",hDsp->memInfo.encmitDSPtr+count,memVal & 0xFF));
		BDBG_MSG(("%08x : %08x",hDsp->memInfo.encmitDSPtr+count+1,(memVal >> 8) & 0xFF));
		BDBG_MSG(("%08x : %08x",hDsp->memInfo.encmitDSPtr+count+2,(memVal >> 16) & 0xFF));
		BDBG_MSG(("%08x : %08x",hDsp->memInfo.encmitDSPtr+count+3,(memVal >> 24) & 0xFF));
	}
#endif

#ifndef MP3ENC_STANDALONE /* MIT creation for Encoder Scheduler */
	{
		BAF_Download *pEncSch = NULL;
		unsigned char *encschmitptr;
		uint32_t *schmitptr;
		pEncSch = (BAF_Download *)((unsigned char *) BKNI_Malloc(SIZE_OF_BAF_DOWNLOAD));
		pEncSch->sFirmware.id= BAF_ALGORITHM_ENCODER_SCHEDULER | 0x00001000;
		BRAP_DSP_P_GetFWSize (hDsp, BRAP_IMG_ENC_SCHEDULER_CODE_ID, &(pEncSch->sFirmware.size));
		BRAP_P_ConvertAddressToOffset(hDsp->hHeap, (void *)(hDsp->memInfo.encscheduler_code_ptr), &physAddress);
		pEncSch->sFirmware.data = physAddress;
		{
			uint32_t ii = hDsp->memInfo.mitDSPtr;
			/* Moving to the encoder scheduler entry */
			ii = ii + (uint32_t)(SIZE_OF_BAF_HOST_INFO_ALLIGNED + SIZE_OF_BAF_HOST_BUF_INFO_ALLIGNED * BAF_HOST_MAX_DL_MODULE + SIZE_OF_BAF_DOWNLOAD_ALLIGNED*BAF_ALGORITHM_ENCODER_SCHEDULER);
			schmitptr = (uint32_t *)ii;
		}
		encschmitptr = (unsigned char *)pEncSch;
		/* Download MIT table in device memory */
		for (count = 0; count < SIZE_OF_BAF_DOWNLOAD_ALLIGNED; count += 4){
			BRAP_MemWrite32((BARC_Handle)hDsp->hRegister, (uint32_t)(schmitptr)+count, *(uint32_t *)(encschmitptr + count)); 
			}
		BRAP_P_DummyMemRead32((BARC_Handle)hDsp->hRegister, (uint32_t)(schmitptr)+count);
		BKNI_Free(pEncSch);
	}
#endif
	/* Free MIT table system memory */
	BKNI_Free(mitBase); 
}
#endif

/* End of File */
