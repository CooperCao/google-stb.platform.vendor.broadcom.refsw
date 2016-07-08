/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2007-2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description:
 *   OTF highlevel API
 *
 **************************************************************************/
/* Magnum */
#include "bstd.h"
#include "bkni.h"
#include "bmem.h"
#include "bchp_xpt_rave.h"

/* OTF */
#include "botf.h"
#include "botf_priv.h"
#include "botf_gop_manager.h"
#include "botf_scv_parser.h"
#include "botf_platform.h"
#include "botf_itb_feeder.h"

BDBG_MODULE(botf);

BDBG_OBJECT_ID(BOTF);

#define     BERR_CONFIG_NOTSET      0x100
#define DO_NOT_COUNT 0xFFFFFFFF


/*****************************************************************************
 * Private routines
 *****************************************************************************/
static BERR_Code BOTF_P_ConfigSet (BOTF_Handle hOtf, const BOTF_Config *Config);
static void BOTF_P_VbvDoneCb(void *context, const void *cdb, const void *itb);
static void BOTF_P_VbvResumeCb(void *cntx);
static BERR_Code BOTF_P_OpenCore(BOTF_Handle hOtf);
static void BOTF_P_CloseCore(BOTF_Handle hOtf);

void 
BOTF_GetDefaultParams(BOTF_Params *Params)
{
    BDBG_ASSERT(Params);
    BKNI_Memset(Params, 0, sizeof(*Params));
    return;
}

/*****************************************************************************
 * BOTF_Open
 * This routine initializes and allocates all the required resources for OTF channel.
 *****************************************************************************/
BERR_Code BOTF_Open(const BOTF_Params *Params, BOTF_Handle *hOtf)
{
	BERR_Code rc = BERR_SUCCESS;
    BOTF_Handle lhOtf;

    BDBG_ASSERT(hOtf);
    BDBG_ASSERT(Params);


    if ((Params->InputParserCDBSize == 0) || (Params->InputParserITBSize == 0))
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ((Params->OutputParserITBSize == 0))
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }


	lhOtf = BKNI_Malloc(sizeof(*lhOtf));
    if(!lhOtf) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_alloc; }

	/* Init values */
    BKNI_Memset(lhOtf, 0, sizeof(*lhOtf));
    BDBG_OBJECT_SET(lhOtf, BOTF);

    lhOtf->ParserState.State = BOTF_FeederState_eStopped;

    rc = BKNI_CreateEvent(&lhOtf->DataEndEvent);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_event; }

    lhOtf->hBMem = Params->hBMem;
    lhOtf->hBReg = Params->hBReg;
    lhOtf->InputParserCDBSize = Params->InputParserCDBSize;
    lhOtf->InputParserITBSize = Params->InputParserITBSize;
    lhOtf->OPParserITBSize = Params->OutputParserITBSize;
    lhOtf->OPParserITBAlign = Params->OutputParserITBAlign;

    lhOtf->bUsePtsAsTag = Params->bUsePtsAsTag;
    lhOtf->IpParserRegMap = Params->inputContext;
    lhOtf->OpParserRegMap = Params->outputContext;

	if (rc == BERR_SUCCESS) {
        BMEM_HeapInfo hi;

        /* Calculate the mem_base value, this value will be added
         * to the CDB base address from ITB entries.
         * This may be negative value depending on the memory mapping.
         */
        BMEM_Heap_GetInfo(lhOtf->hBMem, &hi);
        botf_mem_init(&lhOtf->mem, hi.ulOffset, hi.pvAddress, hi.zSize, lhOtf);
        lhOtf->OPParserPtrs.mem = &lhOtf->mem;
        lhOtf->IPParserPtrs.mem = &lhOtf->mem;
    }


    /* Platform specific initialization */
    BOTF_PlatformOpen(lhOtf);

	/* Open the OTF core */
	rc = BOTF_P_OpenCore(lhOtf);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_core; }

    BDBG_MSG(("BOTF_Open() succeeded"));
    *hOtf = lhOtf;
	
    return BERR_SUCCESS;

err_core:
    BKNI_DestroyEvent(lhOtf->DataEndEvent);
    BOTF_PlatformClose(lhOtf);
err_event:
    BKNI_Free(lhOtf);
err_alloc:
    BDBG_WRN(("BOTF_Open() failed"));
    return rc;
}

void BOTF_Close(BOTF_Handle hOtf)
{
    BDBG_OBJECT_ASSERT(hOtf, BOTF);

	if (hOtf->ParserState.State != BOTF_FeederState_eStopped)
	{
		BDBG_ERR(("closing otf while pump is active, state is %d", hOtf->ParserState.State));
	}

	BKNI_DestroyEvent(hOtf->DataEndEvent);
	BOTF_P_CloseCore(hOtf);	
    BOTF_PlatformClose(hOtf);
    BDBG_OBJECT_ASSERT(hOtf, BOTF);
    BKNI_Free(hOtf);

    return ;
}

/*****************************************************************************
 * BOTF_Start
 *****************************************************************************/
BERR_Code BOTF_Start(BOTF_Handle hOtf)
{
	BERR_Code rc = BERR_SUCCESS;      
	
    BDBG_OBJECT_ASSERT(hOtf, BOTF);

	if (hOtf->bStarted == true)
	{
		BDBG_WRN(("BOTF_Start: OTF already started"));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}
	hOtf->DataEndTime = DO_NOT_COUNT;  	

	/* Set the configuration */
	/* Although we set gop mode when config set is called outside,
	 * we need to do this, each time otf_start is called because we
	 * need to reset gop player and set the mode 
	 */
	rc = BOTF_P_ConfigSet(hOtf, &hOtf->Settings);
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("BOTF_P_ConfigSet returned error"));
		return BERR_TRACE(rc);
	}

    hOtf->bOtfStop = false;

	if (rc != BERR_SUCCESS)
	{
        return BERR_TRACE(rc);
    }

    BDBG_MSG(("feeder mode is %d", hOtf->ParserState.FeederState.FeederMode));

    botf_scv_parser_reset(hOtf->ScvParser);
    bpvr_gop_manager_reset(hOtf->GopManager, hOtf->ParserState.FeederState.FeederMode, bpvr_stream_type_mpeg);
    bpvr_gop_player_start(hOtf->GopPlayer);

	hOtf->ParserState.State = BOTF_FeederState_eVbvInvalid;

	hOtf->bStarted = true;

    hOtf->bOtfTaskDone = false;    
    hOtf->ParserState.ParserScvReadPtr = NULL;
    BOTF_P_InitBufferPtrs(hOtf);    

    return BERR_SUCCESS;
}

/*****************************************************************************
 * BOTF_Stop
 *****************************************************************************/
void BOTF_Stop(BOTF_Handle hOtf)
{
	
	if (!hOtf->bStarted)
	{
		BDBG_WRN(("BOTF_Stop: OTF not started or already stopped"));
		goto ExitFunc;
	}	

	if (hOtf->ParserState.State == BOTF_FeederState_eStopped)
	{
		BDBG_ERR(("BOTF_Stop: Already stopped"));
        goto ExitFunc;	
	}

    /* Stop gop player, so OTF task can return */
    bpvr_gop_player_stop(hOtf->GopPlayer);
    
	/* Set the flag to exit the OTF task */
    hOtf->bOtfStop = true;


    BOTF_PlatformFlushOpParser(hOtf);

	hOtf->ParserState.State = BOTF_FeederState_eStopped;

ExitFunc:
	hOtf->bStarted = false;	

    return ;
}

void BOTF_ConfigGet( BOTF_Handle hOtf, BOTF_Config *Config)
{
    BDBG_OBJECT_ASSERT(hOtf, BOTF);
    BDBG_ASSERT(Config);
	
	*Config = hOtf->Settings;
	
    return ;
}

BERR_Code BOTF_ConfigSet( BOTF_Handle hOtf, const BOTF_Config *Config)
{
	bpvr_gop_player_mode gopmode;

    BDBG_OBJECT_ASSERT(hOtf, BOTF);
    BDBG_ASSERT(Config);

	if ((hOtf->ParserState.State != BOTF_FeederState_eStopped)
			&& ( (Config->IsRewind != hOtf->Settings.IsRewind)
			|| (Config->FeederMode  != hOtf->Settings.FeederMode) )
	    ) 
	{
		/* When OTF is running, we can change only rate and frame filter */
		return BERR_TRACE(BERR_CONFIG_NOTSET);
	}

	if (hOtf->ParserState.State != BOTF_FeederState_eStopped)
	{
		/* Set the configuration on the fly */
		bpvr_gop_player_get_mode(hOtf->GopPlayer, &gopmode);        
		switch (Config->FrameFilter)
		{
			case BOTF_FrameFilter_eIFrames:
				gopmode.frame_filter = BOTF_DISPLAY_I;
				break;
			case BOTF_FrameFilter_eIPFrames:
				gopmode.frame_filter = BOTF_DISPLAY_I|BOTF_DISPLAY_P;
				break;
			case BOTF_FrameFilter_eIPBFrames:
				gopmode.frame_filter = BOTF_DISPLAY_I|BOTF_DISPLAY_P|BOTF_DISPLAY_B;
				break;
			default:
				return BERR_TRACE(BERR_INVALID_PARAMETER);
		}	
		gopmode.rate = Config->FrameRate;
	
        if (gopmode.rate)
        {
            bpvr_gop_player_set_mode(hOtf->GopPlayer, &gopmode);
        }
	}
		
	hOtf->Settings = *Config;

    return BERR_SUCCESS;
}

void BOTF_PTSReferenceGet( BOTF_Handle hOtf)
{
    BDBG_OBJECT_ASSERT(hOtf, BOTF);

    hOtf->DataEndTime = 0;

	return;
}

void BOTF_DisableForFlush(BOTF_Handle hOtf)
{
    BDBG_MSG(("BOTF_DisableForFlush"));
    BOTF_PlatformFlushOpParser(hOtf);
    return;
}

void BOTF_Flush(BOTF_Handle hOtf)
{
    BERR_Code berr;

    BDBG_MSG(("BOTF_Flush"));

    berr = BOTF_P_ConfigSet(hOtf, &(hOtf->Settings));
    if (berr != BERR_SUCCESS) { berr = BERR_TRACE(berr); }

    botf_scv_parser_reset(hOtf->ScvParser);
    bpvr_gop_manager_reset(hOtf->GopManager, hOtf->ParserState.FeederState.FeederMode, bpvr_stream_type_mpeg);
    bpvr_gop_player_start(hOtf->GopPlayer);

    hOtf->ParserState.State = BOTF_FeederState_eVbvInvalid;
    hOtf->prevVideoDecoderStatus.pts = 0;
    hOtf->prevVideoDecoderStatus.ptsValid = false;
    hOtf->prevVideoDecoderStatus.pictureDepth = 0;

    hOtf->ParserState.ParserScvReadPtr = NULL;
    BOTF_P_InitBufferPtrs(hOtf);

    return ;
}

/*****************************************************************************
 * BOTF_P_ConfigSet  
 *****************************************************************************/
static BERR_Code BOTF_P_ConfigSet (BOTF_Handle hOtf, const BOTF_Config *Config)
{
	bpvr_gop_player_mode gopmode;
    BERR_Code berr = BERR_SUCCESS;

    BDBG_ASSERT(Config);

	bpvr_gop_player_reset(hOtf->GopPlayer);
	bpvr_gop_player_get_mode(hOtf->GopPlayer, &gopmode);
		
	if (Config->IsRewind) {
		gopmode.rewind = true;
    } else {
		gopmode.rewind = false;
    }

    hOtf->ParserState.FeederState.FeederMode = Config->FeederMode;
	gopmode.rate_control = bpvr_rate_control_frame;

	switch (Config->FrameFilter)
	{
		case BOTF_FrameFilter_eIFrames:
			gopmode.frame_filter = BOTF_DISPLAY_I;
			break;
		case BOTF_FrameFilter_eIPFrames:
			gopmode.frame_filter = BOTF_DISPLAY_I|BOTF_DISPLAY_P;
			break;
		case BOTF_FrameFilter_eIPBFrames:
			gopmode.frame_filter = BOTF_DISPLAY_I|BOTF_DISPLAY_P|BOTF_DISPLAY_B;
			break;
		default:
			berr = BERR_TRACE(BERR_INVALID_PARAMETER);
			break;
	}	
	gopmode.rate = Config->FrameRate;	

    if (gopmode.rate)
    {
        bpvr_gop_player_set_mode(hOtf->GopPlayer, &gopmode);
    }

	return	berr;		
}
/*****************************************************************************
 * BOTF_P_VbvDoneCb
 *****************************************************************************/
void BOTF_P_VbvDoneCb(void *context, const void *cdb, const void *itb)
{

	BOTF_Handle hOtf= (BOTF_Handle) context;
    
    BOTF_P_SetIPReadPtrs(hOtf, cdb, itb);        

    return;
}

/*****************************************************************************
 * BOTF_P_VbvResumeCb
 *****************************************************************************/
void BOTF_P_VbvResumeCb(void *context)
{
	BOTF_Handle hOtf= (BOTF_Handle) context;	

    BDBG_ASSERT(NULL != hOtf);
	hOtf->ParserState.State = BOTF_FeederState_eRunning;
    return;
}


BERR_Code 
BOTF_GetStatus(BOTF_Handle hOtf, BOTF_Status *status)
{
    BDBG_OBJECT_ASSERT(hOtf, BOTF);
    BDBG_ASSERT(status);

    /* Calculate input and output ITB levels and print them */
    status->inputItb = botf_itb_feeder_get_occupied(&hOtf->IPParserPtrs);
    status->outputItb = botf_itb_feeder_get_occupied(&hOtf->OPParserPtrs);

    if (hOtf->ParserState.ParserScvReadPtr <= hOtf->IPParserPtrs.ItbValidPtr) {
        status->unconsumedInputItb = hOtf->IPParserPtrs.ItbValidPtr - hOtf->ParserState.ParserScvReadPtr;
    } else {
        status->unconsumedInputItb = (hOtf->IPParserPtrs.ItbWrapAroundPtr - hOtf->ParserState.ParserScvReadPtr) +
                                  (hOtf->IPParserPtrs.ItbValidPtr - hOtf->IPParserPtrs.ItbStartPtr);
    }

    if (hOtf->IPParserPtrs.CdbReadPtr <= hOtf->IPParserPtrs.CdbValidPtr) {
        status->inputCdb = hOtf->IPParserPtrs.CdbValidPtr - hOtf->IPParserPtrs.CdbReadPtr;
    } else {
        status->inputCdb = (hOtf->IPParserPtrs.CdbWrapAroundPtr - hOtf->IPParserPtrs.CdbReadPtr) +
                        (hOtf->IPParserPtrs.CdbValidPtr - hOtf->IPParserPtrs.CdbStartPtr);
    }

    return BERR_SUCCESS;
}

BERR_Code 
BOTF_PlayStep(BOTF_Handle hOtf, const BOTF_VideoDecoderStatus  *videoDecoderStatus, unsigned timeout, BOTF_StepStatus *status)
{
    int ScvLen1, ScvLen2;
    bool bEmpty = true;	
    size_t size;
    size_t ScvProcessed;
    const uint8_t *LastOPParserItbReadPtr;
    bool boRaveFull;

    BSTD_UNUSED(bEmpty);

    BDBG_OBJECT_ASSERT(hOtf, BOTF);
    BDBG_ASSERT(status);

    status->flush = false;
    status->stop = false;
    status->eos = false;
    status->referencePtsValid = false;

    if(hOtf->bOtfStop) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    LastOPParserItbReadPtr = hOtf->OPParserPtrs.ItbReadPtr;
    BOTF_P_UpdateBufferPtrs (hOtf);        


    /* Free GOPS used by decoder */
    bpvr_gop_freecompleted(hOtf->GopPlayer);

    switch (hOtf->ParserState.State) {
    case BOTF_FeederState_eStopped:
        status->stop = true;
        goto done;
    case BOTF_FeederState_ePaused:
        goto done;
    case BOTF_FeederState_eDraining:
        status->eos = true;
        goto done;
    default:
        break;
    }

    if (hOtf->ParserState.ParserScvReadPtr == NULL) {
        hOtf->ParserState.ParserScvReadPtr = hOtf->IPParserPtrs.ItbReadPtr;
    }        

    /* Check if there is any valid data to process */
    if (hOtf->IPParserPtrs.ItbValidPtr != hOtf->ParserState.ParserScvReadPtr)
    {
        hOtf->ParserState.State = BOTF_FeederState_eRunning;
    }


    /* compute length of data before and after wrap */
    if (hOtf->ParserState.ParserScvReadPtr <= hOtf->IPParserPtrs.ItbValidPtr) {
        ScvLen1 = hOtf->IPParserPtrs.ItbValidPtr - hOtf->ParserState.ParserScvReadPtr + 1;
        ScvLen2 = 0;
    } else {
        ScvLen1 = hOtf->IPParserPtrs.ItbWrapAroundPtr - hOtf->ParserState.ParserScvReadPtr + 1;
        ScvLen2 = hOtf->IPParserPtrs.ItbValidPtr - hOtf->IPParserPtrs.ItbStartPtr + 1;
    }

    BDBG_MSG(("BOTF_PlayStep: SCV:%u:%u(%u:%u) ITB %p..[%p:%p]..%p..%p..%p PTS:%u %u", ScvLen1, ScvLen2, botf_itb_feeder_get_occupied(&hOtf->IPParserPtrs), botf_itb_feeder_get_occupied(&hOtf->OPParserPtrs), (void *)hOtf->IPParserPtrs.ItbStartPtr, (void *)hOtf->ParserState.ParserScvReadPtr, hOtf->IPParserPtrs.ItbReadPtr, hOtf->IPParserPtrs.ItbValidPtr, hOtf->IPParserPtrs.ItbWrapAroundPtr, hOtf->IPParserPtrs.ItbEndPtr, videoDecoderStatus->pts, videoDecoderStatus->pictureDepth));

    /* Don't consume last ITB entry to avoid race conditions */
    if (ScvLen2 >= B_SCV_LEN) {
        ScvLen2 -= B_SCV_LEN;
    } else {
        ScvLen1 -= B_SCV_LEN;
    }        
    
    if ( (ScvLen1 > 0) || (ScvLen2 > 0) )
    {
        bool bPaused;
        bEmpty = false;

        if (DO_NOT_COUNT != hOtf->DataEndTime)
        {
            hOtf->DataEndTime = 0;
        }

        if (ScvLen1 > 0)
        {
            bPaused = botf_scv_parser_feed(hOtf->ScvParser, hOtf->ParserState.ParserScvReadPtr, ScvLen1, &ScvProcessed);
            hOtf->ParserState.ParserScvReadPtr = ((uint8_t *)hOtf->ParserState.ParserScvReadPtr) + ScvProcessed;
            BDBG_MSG(("1 ParserScvReadPtr = %p", hOtf->ParserState.ParserScvReadPtr));
            if (bPaused)
            {                    
                goto Pause;
            }

            BDBG_ASSERT(ScvProcessed == (size_t) ScvLen1);                
        }            

        if (ScvLen2 > 0)
        {		
            hOtf->ParserState.ParserScvReadPtr = hOtf->IPParserPtrs.ItbStartPtr;
            bPaused = botf_scv_parser_feed(hOtf->ScvParser, hOtf->ParserState.ParserScvReadPtr, ScvLen2, &ScvProcessed);
            hOtf->ParserState.ParserScvReadPtr = ((uint8_t *)hOtf->ParserState.ParserScvReadPtr) + ScvProcessed;
            BDBG_MSG(("2 ParserScvReadPtr = %p", hOtf->ParserState.ParserScvReadPtr));
            if (bPaused)
            {
                goto Pause;
            }
            BDBG_ASSERT(ScvProcessed == (size_t) ScvLen2);                
        }
    } else {  

        if (DO_NOT_COUNT != hOtf->DataEndTime)
        {
            hOtf->DataEndTime+=timeout;
            if (4*BOTF_TASK_SLEEPMS <= hOtf->DataEndTime)
            {
                status->referencePts = bpvr_gop_manager_get_pts(hOtf->GopManager);
                status->referencePtsValid = true;
                hOtf->DataEndTime = DO_NOT_COUNT;
            }
        }

        
        /* if SCV FIFO is empty and CDB FIFO is near full, it's likely that we have
         * overflowed CDB FIFO, so we have to force out all halfway finished GOPs */ 
        size = botf_itb_feeder_get_occupied(&hOtf->IPParserPtrs);
        /* If input RAVE is full and decoder has stopped updating itb pointers, it means that
         * OTF is stuck.
         */
        {
            /* Better way to determine if input rave is full */
            uint32_t uiCxHoldClrStatus = BREG_Read32(hOtf->hBReg, BCHP_XPT_RAVE_CX_HOLD_CLR_STATUS);
            boRaveFull = (uiCxHoldClrStatus & (1 << hOtf->IpParserRegMap.ContextIdx)) != 0;
        }
        BDBG_MSG(("RAVE:%s %u:%u ITB:%p:%p", boRaveFull?"FULL":"", (unsigned)size, botf_itb_feeder_get_occupied(&hOtf->OPParserPtrs), hOtf->OPParserPtrs.ItbReadPtr, LastOPParserItbReadPtr));
        if (videoDecoderStatus->pictureDepth==0 && videoDecoderStatus->pts == hOtf->prevVideoDecoderStatus.pts && videoDecoderStatus->pictureDepth==hOtf->prevVideoDecoderStatus.pictureDepth &&
            hOtf->Settings.FrameRate && (boRaveFull || size > hOtf->InputParserCDBSize*95/100) && hOtf->OPParserPtrs.ItbReadPtr == LastOPParserItbReadPtr) {
            hOtf->DecoderStuckTime+=timeout;
            if (hOtf->DecoderStuckTime >= 4*BOTF_TASK_SLEEPMS) {
                status->flush = true;
                goto done;
            }
            if (hOtf->DecoderStuckTime >= 2*BOTF_TASK_SLEEPMS) {
                /* force out all halfway finished GOPs */
                bpvr_gop_manager_force(hOtf->GopManager, hOtf->IPParserPtrs.CdbValidPtr, hOtf->IPParserPtrs.ItbValidPtr);                    
            }
        } else {
            hOtf->DecoderStuckTime = 0;
        }
    }     

    goto done;

Pause:
    BDBG_MSG(("parsing paused"));
    if(hOtf->ParserState.State != BOTF_FeederState_eDraining) {
        hOtf->ParserState.State = BOTF_FeederState_ePaused;
    }
done:
    hOtf->prevVideoDecoderStatus = *videoDecoderStatus;
    BDBG_MSG(("parsing done"));

	return BERR_SUCCESS;
}


/*****************************************************************************
 * BOTF_P_OpenCore
 *****************************************************************************/
BERR_Code BOTF_P_OpenCore(BOTF_Handle hOtf)
{
	BERR_Code rc = BERR_SUCCESS;
	bpvr_gop_manager_cfg gop_manager_cfg;

	/* Create gop manager */
	if (rc == BERR_SUCCESS)
	{
		bpvr_gop_manager_init_cfg(&gop_manager_cfg);
		gop_manager_cfg.cdb_done = BOTF_P_VbvDoneCb;
		gop_manager_cfg.cdb_resume = BOTF_P_VbvResumeCb;
		gop_manager_cfg.cdb_cntx = hOtf;
        gop_manager_cfg.OPParserPtrs = &hOtf->OPParserPtrs;
        gop_manager_cfg.IPParserPtrs = &hOtf->IPParserPtrs;
        gop_manager_cfg.bUsePtsAsTag = hOtf->bUsePtsAsTag;        

		hOtf->GopManager = bpvr_gop_manager_create(&gop_manager_cfg);
		if (hOtf->GopManager == NULL)
			rc = BERR_TRACE(BERR_OS_ERROR);
	}

	/* Create gop manager player */
	if (rc == BERR_SUCCESS)
	{
		hOtf->GopPlayer = bpvr_gop_manager_player(hOtf->GopManager);
		if (hOtf->GopPlayer == NULL)
			rc = BERR_TRACE(BERR_OS_ERROR);
	}

	/* Create scv parser */
	if (rc == BERR_SUCCESS)
	{
        hOtf->ScvParser = botf_scv_parser_create(hOtf->GopManager, &hOtf->IPParserPtrs);

		if (hOtf->ScvParser == NULL) {
			rc = BERR_TRACE(BERR_OS_ERROR);
        }
        bpvr_gop_manager_setscvparser(hOtf->GopManager, hOtf->ScvParser);
	}

	/* Clean-up any errors */
	if (rc != BERR_SUCCESS)
	{
		BOTF_P_CloseCore(hOtf);
	}

	return rc;
}

/*****************************************************************************
 * BOTF_P_CloseCore
 *****************************************************************************/
void BOTF_P_CloseCore(BOTF_Handle hOtf)
{
	if (hOtf->ScvParser != NULL)
	{
		botf_scv_parser_destroy(hOtf->ScvParser);
		hOtf->ScvParser = NULL;
	}

	if (hOtf->GopPlayer != NULL)
	{
		bpvr_gop_player_destroy(hOtf->GopPlayer);
		hOtf->GopPlayer = NULL;
	}

	if (hOtf->GopManager != NULL)
	{
		bpvr_gop_manager_destroy(hOtf->GopManager);
		hOtf->ScvParser = NULL;
	}
}

void BOTF_P_DetectedEof(void *hOtf_)
{
    BOTF_Handle hOtf = hOtf_;
    BDBG_OBJECT_ASSERT(hOtf, BOTF);

    hOtf->ParserState.State = BOTF_FeederState_eDraining;
    BDBG_MSG(("%#lx draining", (unsigned long)hOtf));
    return;
}

void BOTF_P_FrameCountCallBack(void *hOtf_, uint32_t chunk_no, uint32_t frame_cnt)
{
    BOTF_Handle hOtf = hOtf_;

    BDBG_OBJECT_ASSERT(hOtf, BOTF);

    if (hOtf->Settings.FrameCountCbck != NULL) {
        hOtf->Settings.FrameCountCbck(hOtf, chunk_no, frame_cnt);
    }   
}

