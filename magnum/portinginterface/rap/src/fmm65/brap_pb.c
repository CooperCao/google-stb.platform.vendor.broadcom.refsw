/***************************************************************************
*     Copyright (c) 2004-2012, Broadcom Corporation
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
#include "brap.h"
#include "brap_priv.h"

BDBG_MODULE(rap_pb); /* Register software module with debug interface */

static BERR_Code BRAP_PB_P_PrepBufInfo_isr(
	BRAP_ChannelHandle 	hRapCh,
    unsigned int 		uiPth,
	unsigned int 		uiRBufId,
	BRAP_RingBufInfo	*pRBufInfo
	);

static BERR_Code BRAP_PB_P_WriteBufUsg_isr(
	BRAP_ChannelHandle 	hRapCh,
    unsigned int 		uiPth,	
	unsigned int 		uiRBufId,
	unsigned int		uiSzUpdt
	);

/***************************************************************************
Summary:
	Private API used to open a Play Back channel.
Description:
	It is used to instantiate a Play Back channel. It allocates channel handle 
	and resource required for the channel if any.
Returns:
	BERR_SUCCESS on success else error
See Also:
	BRAP_PB_P_ChannelClose
****************************************************************************/
BERR_Code BRAP_PB_P_ChannelOpen( 
    BRAP_Handle                     hRap,		    /* [in] Raptor Audio Device 
                                                       handle*/
    BRAP_ChannelHandle              hRapCh,		    /* [in] Raptor Play Back 
                                                       Channel handle */
    const BRAP_ChannelSettings      *pChnSettings   /* [in] Channel settings*/ 
	)
{
	BERR_Code               err = BERR_SUCCESS;
	unsigned int            uiChannelNo = 0;
    BRAP_RM_P_ResrcReq      *psRsrcReq = NULL;
    BRAP_RM_P_ResrcGrant    *psRsrcGrnt = NULL;
    bool                    bWdgRecovery = BRAP_P_GetWatchdogRecoveryFlag(hRap);
    unsigned int            uiPth = 0, i=0;    
#if (BRAP_7405_FAMILY == 1)
    BRAP_RBUF_P_Settings    sRBufSettings;
    BRAP_SRCCH_P_Settings   sSrcChSettings;    
    BRAP_ChannelParams	sAudioParams;
#endif
#if BRAP_P_WATERMARK_WORKAROUND==1
	BTMR_Settings sTimerSettings;
#endif    

    BDBG_ENTER(BRAP_PB_P_ChannelOpen);

	/* Validate input params */
	BDBG_ASSERT(hRap);
	BDBG_ASSERT(hRapCh);

	if(false == bWdgRecovery)
	{
        /* pChnSettings is valid only when not in watchdog recovery */
        BDBG_ASSERT(pChnSettings);
        BDBG_MSG(("BRAP_PB_P_ChannelOpen():"
                  "hRap=0x%x," 
                  "\n\t pChnSettings->eChType = %d" 
                  "\n\t pChnSettings->eChSubType = %d",
                  hRap, pChnSettings->eChType, pChnSettings->eChSubType));

        /* Validate if this channel can be added to hRap */
        for(uiChannelNo=0; uiChannelNo<BRAP_RM_P_MAX_PCM_CHANNELS; uiChannelNo++)
        {
        	if(NULL == hRap->hRapPbCh[uiChannelNo])
        	{
        		break;
        	}
        }

        if(BRAP_RM_P_MAX_PCM_CHANNELS == uiChannelNo)
        {
        	BDBG_ERR(("Max number of PB channels(%d) already exist", uiChannelNo));
        	err = BERR_TRACE(BERR_NOT_SUPPORTED);
        	goto end_open;
        }
        hRapCh->uiChannelNo = uiChannelNo;

    	/* Malloc large structures */
    	psRsrcReq = hRap->sOpenTimeMallocs.psResrcReq;
    	if ( NULL==psRsrcReq )
    	{
    			return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    	}
        BKNI_Memset(psRsrcReq, 0, sizeof(BRAP_RM_P_ResrcReq));
        
    	psRsrcGrnt = hRap->sOpenTimeMallocs.psResrcGrant;
    	if ( NULL==psRsrcGrnt )
    	{
    			return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    	}
        BKNI_Memset(psRsrcGrnt, 0, sizeof(BRAP_RM_P_ResrcGrant));
        
        /* Prepare the resource request for DSP */
        BRAP_RM_P_InitResourceReq(psRsrcReq);
        psRsrcReq->eChType                = hRapCh->eChannelType;
        psRsrcReq->eChSubType             = hRapCh->eChannelSubType;
        psRsrcReq->bAllocateDSP            = false;
#if BRAP_P_WATERMARK_WORKAROUND==1
        BTMR_GetDefaultTimerSettings(&sTimerSettings);
        sTimerSettings.type = BTMR_Type_eCountDown;
        sTimerSettings.cb_isr = (BTMR_CallbackFunc) BRAP_P_FmmRbufFreeWaterMark_isr;
        sTimerSettings.pParm1 = hRapCh;
        sTimerSettings.parm2 = 0;
        sTimerSettings.exclusive = false;

        err = BTMR_CreateTimer (hRap->hTmr, &hRapCh->hTimer, &sTimerSettings);
        if ( err != BERR_SUCCESS )
        {
        		BDBG_ERR(("BRAP_PB_OpenChannel: Create Timer Failed"));
        		err = BERR_TRACE(err);
        		goto end_open;
        }
#endif        

#if (BRAP_7405_FAMILY == 1)
    if(hRapCh->bOpenTimeWrToRbuf == true)
    {
        /* Rbuf */
        psRsrcReq->sRbufReq[0].bAllocate = true;
        /* SrcCh */
        psRsrcReq->sSrcChReq[0].bAllocate = true;                        


        if(hRapCh->sChanRBufPool.sExtRBufPoolSettings.uiMaxNumRBuf == 1)
        {
            psRsrcReq->sRbufReq[0].eBufDataMode =   BRAP_BufDataMode_eStereoInterleaved;
        }
        else if(hRapCh->sChanRBufPool.sExtRBufPoolSettings.uiMaxNumRBuf == 2)
        {
            psRsrcReq->sRbufReq[0].eBufDataMode =   BRAP_BufDataMode_eStereoNoninterleaved;
        }            
        else if(hRapCh->sChanRBufPool.sExtRBufPoolSettings.uiMaxNumRBuf == 0)
        {
            psRsrcReq->sRbufReq[0].eBufDataMode =   BRAP_BufDataMode_eMaxNum;
        } 
        /* Find a free pPath index to save granted resources */
        for(uiPth = 0; uiPth < BRAP_P_MAX_PATHS_IN_A_CHAN; uiPth++)
        {
            if(hRapCh->pPath[uiPth] == NULL)
                break;
        }
        if(uiPth >= BRAP_P_MAX_PATHS_IN_A_CHAN)
        {
            BDBG_ERR(("No free sPath available"));
			err = BERR_TRACE(err); goto error;
        }
        else
        {            
            BDBG_ASSERT(0 == uiPth); /* For Debug */
#ifdef BRAP_OPEN_TIME_PATH_ALLOCATION
            err = BRAP_P_MallocPath(hRapCh,uiPth,&(hRapCh->pPath[uiPth]));
#else
            err = BRAP_P_MallocPath(&(hRapCh->pPath[uiPth]));
#endif            
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("BRAP_P_MallocPath failed"));
			    err = BERR_TRACE(err); goto error;
            }                        
            for (i=0;i<BRAP_P_MAX_OUT_PARALLEL_FMM_PATH;i++)
            {
                BRAP_P_SET_USG_PATH(hRapCh->pPath[uiPth], 
                                BRAP_P_UsgPath_eDecodePcm,
                                BRAP_INVALID_VALUE,
                                uiPth,
                                BRAP_INVALID_VALUE,i);
            }
        }

        /* Call resource manager to allocate required resources. */
		err = BRAP_RM_P_AllocateResources(hRap->hRm, psRsrcReq, psRsrcGrnt);
		if(BERR_SUCCESS != err)
		{
			BDBG_ERR(("Resource alloc failed with err = %d", err));
			err = BERR_TRACE(err); goto error;
		}
               
        /* Update the resource grant structure in hRapCh */
        err = BRAP_RM_P_UpdateResrcGrant(
            &(hRapCh->pPath[uiPth]->sRsrcGrnt), 
            psRsrcGrnt);
		if(BERR_SUCCESS != err)
		{
			BDBG_ERR(("UpdateResrcGrant failed %d", err));
			err = BERR_TRACE(err); goto error;
		}
                /* Get default settings */
            err = BRAP_RBUF_P_GetDefaultSettings(&sRBufSettings);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR(("ERR!! BRAP_RBUF_P_GetDefaultSettings"));            
                err = BERR_TRACE(err);
                goto close_rbuf;
            }  
            BRAP_GetDefaultChannelParams(hRapCh,&sAudioParams);            
            for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNELS; i++)            
            {
                if(hRapCh->pPath[uiPth]->sRsrcGrnt.uiRbufId[i] != BRAP_RM_P_INVALID_INDEX)
                {                                                    
                    sRBufSettings.bProgRdWrRBufAddr = true;
                    sRBufSettings.bRbufOfClonedPort = false;
                    sAudioParams.sBufParams.uiWaterMark = pChnSettings->sChnRBufPool.sBufSettings.uiWaterMark;     
#if (BRAP_OPEN_TIME_RBUF_ALLOCATION==1)                           
                err = BRAP_P_GetRbufFromPool(hRapCh,&sAudioParams, hRapCh->pPath[uiPth],false,0,0,&sRBufSettings, 0);   
                if(BERR_SUCCESS != err)
                {
                    BDBG_ERR(("ERR!! BRAP_P_GetRbufFromPool"));
                    err = BERR_TRACE(err);
                }
#endif
                    /* Open Rbuf handle */
                    err = BRAP_RBUF_P_Open( hRap->hFmm[0],
                                            &(hRapCh->pPath[uiPth]->hRBuf[i]),
                                            hRapCh->pPath[uiPth]->sRsrcGrnt.uiRbufId[i],
                                            &sRBufSettings);
                    if(BERR_SUCCESS != err)
                    {
                        BDBG_ERR(("ERR!! BRAP_RBUF_P_Open"));
                        err = BERR_TRACE(err);
                        goto close_rbuf;
                    }
                }
            }
        }
#else
        BSTD_UNUSED(i);
        BSTD_UNUSED(uiPth);
#endif        
    }/* if !Watchdog */
#if (BRAP_7405_FAMILY == 1)    
    else
    {
        if(hRapCh->bOpenTimeWrToRbuf == true)
        {    
            /* Open Rbuf handle */
            err = BRAP_RBUF_P_Open( hRap->hFmm[0],
                                    &(hRapCh->pPath[uiPth]->hRBuf[i]),
                                    hRapCh->pPath[uiPth]->sRsrcGrnt.uiRbufId[i],
                                    NULL);
            if(BERR_SUCCESS != err)
              {err = BERR_TRACE(err);goto close_rbuf;}                
        }
    }            

    if(hRapCh->bOpenTimeWrToRbuf == true)
    {  
        for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
        {
            if(hRapCh->pPath[uiPth]->sRsrcGrnt.uiSrcChId[i] != BRAP_RM_P_INVALID_INDEX)
            {
                /* Open SrcCh handle */
                err = BRAP_SRCCH_P_Open( hRap->hFmm[0],
                                        &(hRapCh->pPath[uiPth]->sSrcCh[i].hSrcCh),
                                        hRapCh->pPath[uiPth]->sRsrcGrnt.uiSrcChId[i],
                                        &sSrcChSettings);
                if(BERR_SUCCESS != err)
                { err =  BERR_TRACE(err);goto close_srcch;}
                
                BDBG_MSG(("Source Channel %d opened",hRapCh->pPath[uiPth]->sRsrcGrnt.uiSrcChId[i]));
            }
        }/* for i */
        /* SrcCh Open Done */
    }
#endif        


	/* Intialise Raptor interrupt handling */
	err = BRAP_P_InterruptInstall (hRapCh);
	if(err != BERR_SUCCESS)
	{
		err = BERR_TRACE(err);
		BDBG_ERR(("InstallInterrupt()failed for RAP PB Channel handle 0x%x", 
                hRapCh));
		if(true == bWdgRecovery)
		{
            goto error;
        }
		else
		{
    		goto free_rs;
        }
    }

#if (BRAP_P_WATERMARK_WORKAROUND == 0)
#if (BRAP_P_EDGE_TRIG_INTRPT == 0)
    /* Mask the Free Mark interrupt */
    err = BRAP_P_MaskInterrupt(hRapCh, BRAP_Interrupt_eFmmRbufFreeByte);
    if(err != BERR_SUCCESS)
    {
    	BDBG_ERR(("Interrupt installation failed for RAP Playback Channel handle 0x%x", hRapCh));
        err = BERR_TRACE(err);
    	goto free_rs;
    }
#endif
#endif
    
    goto end_open;
    

#if (BRAP_7405_FAMILY == 1)    
close_srcch:
    BDBG_ERR(("BRAP_PB_P_ChannelOpen: Came in close_srcch"));        
    for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    {
        if(NULL != hRapCh->pPath[uiPth]->sSrcCh[i].hSrcCh)
        {
            err = BRAP_SRCCH_P_Close(hRapCh->pPath[uiPth]->sSrcCh[i].hSrcCh);
            if (err != BERR_SUCCESS)
            {
                BDBG_ERR (("BRAP_PB_P_ChannelOpen: call to"
                  " BRAP_SRCCH_P_Close() failed. Ignoring error!!!!!"));
                err = BERR_TRACE (err);
            }
	     BRAP_P_InitSrcChHandleInfo(&(hRapCh->pPath[uiPth]->sSrcCh[i]));
            BDBG_MSG(("SrcCh closed"));
        }
    }/* for i */


close_rbuf:
#if (BRAP_OPEN_TIME_RBUF_ALLOCATION==1)
			/* Returning the Rbuf to global pool */
			BRAP_P_ReturnRBufToPool(hRapCh,hRapCh->pPath[uiPth],0,0,0);
#endif

    BDBG_ERR(("BRAP_PB_P_ChannelOpen: Came in close_rbuf"));        
    for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNELS; i++)
    {
        if((hRapCh->pPath[uiPth]->sRsrcGrnt.uiRbufId[i] != BRAP_RM_P_INVALID_INDEX) &&
           (hRapCh->pPath[uiPth]->hRBuf[i] != NULL))
        {
            err = BRAP_RBUF_P_Close(hRapCh->pPath[uiPth]->hRBuf[i]);
            if(BERR_SUCCESS != err)
            {
                BDBG_ERR (("BRAP_RBUF_P_Close() failed. Ignoring error!!!!!"));
                err = BERR_TRACE(err);
            }
            hRapCh->pPath[uiPth]->hRBuf[i] = NULL;
            BDBG_MSG(("Ring buffer %d closed",hRapCh->pPath[uiPth]->sRsrcGrnt.uiRbufId[i]));
        }
    }/* for i */
#endif
    
free_rs:
	/* TODO: check all exit conditions and resource free/close */

error:
    /* TODO: */
    
end_open:
	
    /* only if channel has been successfully opened, save the handle */
	if((BERR_SUCCESS == err) && (false == bWdgRecovery))
	{
    	hRapCh->eState = BRAP_P_State_eOpened; /* Opened successfully */
    	hRap->hRapPbCh[uiChannelNo] = hRapCh ; 
        BDBG_MSG(("BRAP_PB_P_ChannelOpen: channel open Successful hRapCh=0x%x",
                  hRapCh));
	}

	BDBG_LEAVE(BRAP_PB_P_ChannelOpen);
	return err;
}


/***************************************************************************
Summary:
	API used to close a Play Back channel.
Description:
	It closes the instance of a Play Back channel operation. It frees the 
	channel handle and resources associated with it if any.
Returns:
	BERR_SUCCESS on success else error
See Also:
	BRAP_PB_P_ChannelOpen
****************************************************************************/
BERR_Code BRAP_PB_P_ChannelClose( 
	BRAP_ChannelHandle 	hRapCh	/* [in] The RAP Channel handle */
	)
{
    BERR_Code	    ret = BERR_SUCCESS;
    unsigned int    uiGrpId = 0;
    int             i = 0;
    unsigned int    uiChannelNo = 0;
    BRAP_Handle 	hRap = NULL;

	BDBG_ENTER(BRAP_PB_P_ChannelClose);

	/* Validate input parameters. */
	BDBG_ASSERT(hRapCh);

    hRap = hRapCh->hRap;

#if (BRAP_P_WATERMARK_WORKAROUND == 0)
#if (BRAP_P_EDGE_TRIG_INTRPT == 0)
    /* Mask the Free Mark interrupt */
    ret = BRAP_P_MaskInterrupt(hRapCh, BRAP_Interrupt_eFmmRbufFreeByte);
	if(ret != BERR_SUCCESS)
	{
	    BDBG_ERR(("BRAP_PB_CloseChannel: BRAP_P_MaskInterrupt() returned err(%d)",ret));	
    }
#endif    
#else
    if (NULL != hRapCh->hTimer)
    {
        ret = BTMR_DestroyTimer(hRapCh->hTimer);
        if (ret != BERR_SUCCESS)
        {
            BDBG_ERR (("BRAP_PB_CloseChannel: BTMR_DestroyTimer Failed"));
            ret = BERR_TRACE(ret);
        }
    }

#endif    

	/* Mask interrupts and uninstall callbacks */
	BRAP_P_InterruptUnInstall(hRapCh);

	/* We have associtated channel pairs and we need to check each of them to 
	   see if the channel exits there. If so we need to remove the channel from 
	   there */

	/* Find the group */
	for(uiGrpId=0; uiGrpId < BRAP_MAX_ASSOCIATED_GROUPS; uiGrpId++)
	{
	    for(i=0; i<BRAP_MAX_PB_CHAN_IN_ASSOCIATED_GRP; i++)
		{   
			if(hRap->sAssociatedCh[uiGrpId].hPBCh[i] == hRapCh)
			{
				hRap->sAssociatedCh[uiGrpId].hPBCh[i] = NULL;
				break;
			}
		}
        if(i != BRAP_MAX_PB_CHAN_IN_ASSOCIATED_GRP)
    	break;
	}

#if (BRAP_7405_FAMILY == 1)    
if(hRapCh->bOpenTimeWrToRbuf == true)
{
    for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    {
        if(NULL != hRapCh->pPath[0]->sSrcCh[i].hSrcCh)
        {
            ret = BRAP_SRCCH_P_Close(hRapCh->pPath[0]->sSrcCh[i].hSrcCh);
            if (ret != BERR_SUCCESS)
            {
                BDBG_ERR (("BRAP_PB_P_ChannelOpen: call to"
                  " BRAP_SRCCH_P_Close() failed. Ignoring error!!!!!"));
                ret = BERR_TRACE (ret);
            }
	     BRAP_P_InitSrcChHandleInfo(&(hRapCh->pPath[0]->sSrcCh[i]));
            BDBG_MSG(("SrcCh closed"));
        }
    }/* for i */


#if (BRAP_OPEN_TIME_RBUF_ALLOCATION==1)
			/* Returning the Rbuf to global pool */
			BRAP_P_ReturnRBufToPool(hRapCh,hRapCh->pPath[0],0,0,0);
#endif

    for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNELS; i++)
    {
        if((hRapCh->pPath[0]->sRsrcGrnt.uiRbufId[i] != BRAP_RM_P_INVALID_INDEX) &&
           (hRapCh->pPath[0]->hRBuf[i] != NULL))
        {
            ret = BRAP_RBUF_P_Close(hRapCh->pPath[0]->hRBuf[i]);
            if(BERR_SUCCESS != ret)
            {
                BDBG_ERR (("BRAP_RBUF_P_Close() failed. Ignoring error!!!!!"));
                ret = BERR_TRACE(ret);
            }
            hRapCh->pPath[0]->hRBuf[i] = NULL;
            BDBG_MSG(("Ring buffer %d closed",hRapCh->pPath[0]->sRsrcGrnt.uiRbufId[i]));
        }
    }/* for i */
}
#endif

	/* Call Resource manager to release these resources */    
  	if( hRapCh->pPath[0] )
	{
		ret = BRAP_RM_P_FreeResources(hRapCh->hRap->hRm, &(hRapCh->pPath[0]->sRsrcGrnt),true); /* TODO for all paths */
		if(ret != BERR_SUCCESS)    
		{        
			BDBG_ERR(("BRAP_PB_CloseChannel: call to BRAP_RM_P_FreeResources()" 
				" failed. Ignoring error!!!!!"));        
			ret = BERR_TRACE (ret);    
		}	
		BDBG_MSG(("Resources Freed."));

        /* Free path memory */
        BRAP_P_FreePath(&(hRapCh->pPath[0]));   
	}

    /* Mark the place for the current channel handle to 'invalid' inside RAP 
    handle */
    for(uiChannelNo=0; uiChannelNo<BRAP_RM_P_MAX_PCM_CHANNELS; uiChannelNo++)
    {
        if(hRapCh == hRap->hRapPbCh[uiChannelNo])
        {
            break;
        }
    }	
    hRapCh->hRap->hRapPbCh[hRapCh->uiChannelNo] = NULL;

	/* Free the channel handle */
	/* BKNI_Free(hRapCh);*/

	BDBG_LEAVE(BRAP_PB_P_ChannelClose);
	return(ret);
}


/***************************************************************************
Summary:	
	Gets the associated ring buffers's information.
Description:
	This API returns the associated ring buffers's information.
Returns:
    Returns error on failure or BERR_SUCCESS on success
See Also:
	BRAP_PB_UpdateBufUsg
****************************************************************************/
BERR_Code BRAP_PB_GetBufInfo(
	BRAP_ChannelHandle 	hRapCh,		   /* [in] Audio channel handle */
	BRAP_PcmBufInfo		*psPcmBufInfo  /* [Out] Associated Pcm buffer 
													 information */		
	)
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ENTER(BRAP_PB_GetBufInfo);

    /* Pre-fill with invalid values */
    psPcmBufInfo->sRtBufInfo.pBasePtr = NULL;
    psPcmBufInfo->sRtBufInfo.pEndPtr = NULL;
    psPcmBufInfo->sRtBufInfo.pReadPtr = NULL;
    psPcmBufInfo->sRtBufInfo.pWritePtr = NULL;
    psPcmBufInfo->sRtBufInfo.uiContiguousSize = 0;
    psPcmBufInfo->sRtBufInfo.uiTotalSize = 0;

    psPcmBufInfo->sLtOrSnglBufInfo.pBasePtr = NULL;
    psPcmBufInfo->sLtOrSnglBufInfo.pEndPtr = NULL;
    psPcmBufInfo->sLtOrSnglBufInfo.pReadPtr = NULL;
    psPcmBufInfo->sLtOrSnglBufInfo.pWritePtr = NULL;
    psPcmBufInfo->sLtOrSnglBufInfo.uiContiguousSize = 0;
    psPcmBufInfo->sLtOrSnglBufInfo.uiTotalSize = 0;

    BKNI_EnterCriticalSection();
    ret = BRAP_PB_GetBufInfo_isr(hRapCh, psPcmBufInfo);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_PB_GetBufInfo);
    return ret;
}


/***************************************************************************
Summary:	Gets the associated ring buffers's information.
Description:
	This API returns the associated ring buffers's information. This is an 
	ISR version of BRAP_PB_GetBufInfo(). If ring buffer is written with PCM
    data from an ISR context, this API should be used to get the rbuf info.
Returns:
	Returns error on failure or BERR_SUCCESS on success
See Also:
****************************************************************************/
BERR_Code BRAP_PB_GetBufInfo_isr(
	BRAP_ChannelHandle 	hRapCh,		   /* [in] Audio channel handle */
	BRAP_PcmBufInfo 	*psPcmBufInfo  /* [Out] Associated Pcm buffer 
													 information */		
	)
{
	BERR_Code ret = BERR_SUCCESS;
	unsigned int uiLtChId = BRAP_RM_P_INVALID_INDEX;
	unsigned int uiRtChId = BRAP_RM_P_INVALID_INDEX;
	unsigned int uiPth = BRAP_RM_P_INVALID_INDEX;    
		
	BDBG_ENTER(BRAP_PB_GetBufInfo_isr);

	/* Validate input params */
	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(psPcmBufInfo);

	BKNI_Memset(psPcmBufInfo, 0, sizeof(BRAP_PcmBufInfo));

	BDBG_MSG(("BRAP_PB_GetBufInfo_isr: hRapCh=%d psPcmBufInfo=%d",hRapCh, psPcmBufInfo));
	
    if(BRAP_ChannelType_ePcmPlayback == hRapCh->eChannelType)
    {
        for(uiPth=0; uiPth<BRAP_P_MAX_PATHS_IN_A_CHAN; uiPth++)
        {
            if((hRapCh->pPath[uiPth] != NULL) && 
               (hRapCh->pPath[uiPth]->eUsgPath == BRAP_P_UsgPath_eDecodePcm))
               break;
        }
    	if (uiPth == BRAP_P_MAX_PATHS_IN_A_CHAN) 
        {
    		BDBG_ERR(("BRAP_PB_GetBufInfo_isr: Decode PCM path not found in hRapCh=%p",
              hRapCh));
		return BERR_TRACE(BERR_NOT_INITIALIZED);
	}
    }
    else if(BRAP_ChannelType_eDecode == hRapCh->eChannelType)
    {
        for(uiPth=0; uiPth<BRAP_P_MAX_PATHS_IN_A_CHAN; uiPth++)
        {
            if((hRapCh->pPath[uiPth] != NULL) && 
               (hRapCh->pPath[uiPth]->eUsgPath == BRAP_P_UsgPath_eCapture))
               break;
        }
    	if (uiPth == BRAP_P_MAX_PATHS_IN_A_CHAN) 
        {
    		BDBG_ERR(("BRAP_PB_GetBufInfo_isr: Capture path not found in hRapCh=%p",
                      hRapCh));
		return BERR_TRACE(BERR_NOT_INITIALIZED);
	}
    }
    else
    {
        BDBG_ERR(("This API should not be called for this channel type %d",
                  hRapCh->eChannelType));
		return BERR_TRACE(BERR_NOT_SUPPORTED);        
	}

	/* Get the valid rbuf ids */
    /* TODO: these are wrong: pPath[indx] indx can't be eDecodePcm */
    if((unsigned int)BRAP_RM_P_INVALID_INDEX != 
                hRapCh->pPath[uiPth]->sRsrcGrnt.uiRbufId[0])
	    uiLtChId = 0;

    if((unsigned int)BRAP_RM_P_INVALID_INDEX != 
                hRapCh->pPath[uiPth]->sRsrcGrnt.uiRbufId[1])
        uiRtChId = 1;

    if(uiLtChId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
	{
		ret = BRAP_PB_P_PrepBufInfo_isr(hRapCh, uiPth, uiLtChId,
                                        &(psPcmBufInfo->sLtOrSnglBufInfo));
		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("BRAP_PB_GetBufInfo_isr: Couldn't get left ring buffer[id - %d]info",
				uiLtChId));
            ret = BERR_TRACE(ret);
            return ret;
		}
	}
    else
    {
		BDBG_ERR(("BRAP_PB_GetBufInfo_isr: left ring buffer id(%d) is INVALID", uiLtChId));
        return BERR_TRACE(BERR_NOT_INITIALIZED);
    }

	if(uiRtChId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
	{
		ret = BRAP_PB_P_PrepBufInfo_isr(hRapCh, uiPth, uiRtChId, 
                                        &(psPcmBufInfo->sRtBufInfo));
		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("BRAP_PB_GetBufInfo_isr: Couldn't get right ring buffer[id - %d]info",
				uiRtChId));			
            ret = BERR_TRACE(ret);
            return ret;
		}
	}

	BDBG_MSG(("BRAP_PB_GetBufInfo_isr: OutParam:\n"
			"\tpsPcmBufInfo->sLtOrSnglBufInfo.pBasePtr=%p\n"
			"\tpsPcmBufInfo->sLtOrSnglBufInfo.pEndPtr=%p\n"
			"\tpsPcmBufInfo->sLtOrSnglBufInfo.pReadPtr=%p\n"
			"\tpsPcmBufInfo->sLtOrSnglBufInfo.pWritePtr=%p\n"
			"\tpsPcmBufInfo->sLtOrSnglBufInfo.uiContiguousSize=%d\n"
			"\tpsPcmBufInfo->sLtOrSnglBufInfo.uiTotalSize=%d\n",
			psPcmBufInfo->sLtOrSnglBufInfo.pBasePtr,
			psPcmBufInfo->sLtOrSnglBufInfo.pEndPtr,
			psPcmBufInfo->sLtOrSnglBufInfo.pReadPtr,
			psPcmBufInfo->sLtOrSnglBufInfo.pWritePtr,
			psPcmBufInfo->sLtOrSnglBufInfo.uiContiguousSize,
			psPcmBufInfo->sLtOrSnglBufInfo.uiTotalSize));

	BDBG_MSG(("\tpsPcmBufInfo->sRtBufInfo.pBasePtr=%p\n"
			"\tpsPcmBufInfo->sRtBufInfo.pEndPtr=%p\n"
			"\tpsPcmBufInfo->sRtBufInfo.pWritePtr=%p\n"
			"\tpsPcmBufInfo->sRtBufInfo.uiContiguousSize=%d\n"
			"\tpsPcmBufInfo->sRtBufInfo.uiTotalSize=%d\n",
			psPcmBufInfo->sRtBufInfo.pBasePtr,
			psPcmBufInfo->sRtBufInfo.pEndPtr,
			psPcmBufInfo->sRtBufInfo.pWritePtr,
			psPcmBufInfo->sRtBufInfo.uiContiguousSize,
			psPcmBufInfo->sRtBufInfo.uiTotalSize));
			
	BDBG_LEAVE(BRAP_PB_GetBufInfo_isr);
	return ret;
}

/***************************************************************************
Summary:	
	Updates the write pointers for the associated ring buffers as per the 
	ring buffer usage provided.
Description:
	This API updates the write pointer associated to the ring-buffer as per 
	the information related to the data written in the ring buffer, which 
	is provided by the caller through the structure BRAP_PcmBufSzUpdt.
Returns:
	Returns error on failure or BERR_SUCCESS on success
See Also:
	BRAP_PB_GetBufInfo
****************************************************************************/
BERR_Code BRAP_PB_UpdateBufUsg( 
	BRAP_ChannelHandle 	    hRapCh,			/* [in] Audio channel handle */
    const BRAP_PcmBufSzUpdt	*psPcmBufSzUpdt	/* [in] Ring buffer usage 
    											   information */
    )
{
	BERR_Code ret = BERR_SUCCESS;

	BDBG_ENTER(BRAP_PB_UpdateBufUsg);

    BKNI_EnterCriticalSection();
    ret = BRAP_PB_UpdateBufUsg_isr(hRapCh, psPcmBufSzUpdt);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_PB_UpdateBufUsg);
    return ret;
}

/***************************************************************************
Summary:	Updates the write pointers for the associated ring buffers as 
    per the ring buffer usage provided.
Description:
	This API updates the write pointer associated to the ring-buffer as per 
	the information related to the data rwritten to the ring buffer, which 
	is provided by the caller through the structure BRAP_PcmBufSzUpdt.
	This is an ISR version of BRAP_PB_UpdateBufUsg(). If ring buffer is 
	written with PCM data from an ISR context, this API should be used to 
	update the ring buffer usage info.
Returns:
	Returns error on failure or BERR_SUCCESS on success
See Also:
	
****************************************************************************/
BERR_Code BRAP_PB_UpdateBufUsg_isr( 
	BRAP_ChannelHandle 	    hRapCh,			/* [in] Audio channel handle */
    const BRAP_PcmBufSzUpdt	*psPcmBufSzUpdt	/* [in] Ring buffer usage 
    											   information */
    )
{
	BERR_Code ret = BERR_SUCCESS;
	unsigned int uiLtChId = BRAP_RM_P_INVALID_INDEX;
	unsigned int uiRtChId = BRAP_RM_P_INVALID_INDEX;
	unsigned int uiPth = 0;    
		
	BDBG_ENTER(BRAP_PB_UpdateBufUsg_isr);

	/* Validate input params */
	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(psPcmBufSzUpdt);

#if BRAP_PB_P_DEBUG_PCM
	BDBG_MSG(("BRAP_PB_UpdateBufUsg_isr: hRapCh=%d"
			  	"\tpsPcmBufSzUpdt->uiLtOrSnglBufSzUpdt=%d"
			  	"\tpsPcmBufSzUpdt->uiRtBufSzUpdt=%d",hRapCh,
			  	psPcmBufSzUpdt->uiLtOrSnglBufSzUpdt,
			  	psPcmBufSzUpdt->uiRtBufSzUpdt));
#endif

    if(BRAP_ChannelType_ePcmPlayback == hRapCh->eChannelType)
    {
        for(uiPth=0; uiPth<BRAP_P_MAX_PATHS_IN_A_CHAN; uiPth++)
        {
            if((hRapCh->pPath[uiPth] != NULL) && 
               (hRapCh->pPath[uiPth]->eUsgPath == BRAP_P_UsgPath_eDecodePcm))
               break;
        }
    	if (uiPth == BRAP_P_MAX_PATHS_IN_A_CHAN) 
        {
    		BDBG_ERR(("BRAP_PB_UpdateBufUsg_isr: Decode PCM path not found in hRapCh=%p",
              hRapCh));
    		return BERR_TRACE(BERR_NOT_INITIALIZED);
    	}
    }
    else if(BRAP_ChannelType_eDecode == hRapCh->eChannelType)
    {
        for(uiPth=0; uiPth<BRAP_P_MAX_PATHS_IN_A_CHAN; uiPth++)
        {
            if((hRapCh->pPath[uiPth] != NULL) && 
               (hRapCh->pPath[uiPth]->eUsgPath == BRAP_P_UsgPath_eCapture))
               break;
        }
    	if (uiPth == BRAP_P_MAX_PATHS_IN_A_CHAN) 
        {
    		BDBG_ERR(("BRAP_PB_UpdateBufUsg_isr: Capture path not found in hRapCh=%p",
                      hRapCh));
    		return BERR_TRACE(BERR_NOT_INITIALIZED);
    	}    
    }
    else
    {
        BDBG_ERR(("This API should not be called for this channel type %d",
                  hRapCh->eChannelType));
		return BERR_TRACE(BERR_NOT_SUPPORTED);        
    }
    
    /* Get the valid rbuf ids */
    if((unsigned int)BRAP_RM_P_INVALID_INDEX != 
                hRapCh->pPath[uiPth]->sRsrcGrnt.uiRbufId[0])
	    uiLtChId = 0;

    if((unsigned int)BRAP_RM_P_INVALID_INDEX != 
                hRapCh->pPath[uiPth]->sRsrcGrnt.uiRbufId[1])
        uiRtChId = 1;

	if(uiLtChId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
	{
		ret = BRAP_PB_P_WriteBufUsg_isr(hRapCh, uiPth, uiLtChId ,psPcmBufSzUpdt->uiLtOrSnglBufSzUpdt);
		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("BRAP_PB_UpdateBufUsg_isr: Couldn't update left ring buffer[0] usage"));			
			return BERR_TRACE(ret);	
		}
	}
    else
    {
		BDBG_ERR(("BRAP_PB_UpdateBufUsg_isr: left ring buffer id(%d) is INVALID", uiLtChId));			
		return BERR_TRACE(ret);	
    }        
        
    /* ToDo: Use the buffer mode that application has provided in audio params.(Add to chan handle?)
             Only if it is non-interleaved mode check the below condition, and add else condition */
	if(uiRtChId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
	{
		ret = BRAP_PB_P_WriteBufUsg_isr(hRapCh, uiPth, uiRtChId, psPcmBufSzUpdt->uiRtBufSzUpdt);
		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("BRAP_PB_UpdateBufUsg_isr: Couldn't update right ring buffer[1] usage"));			
			return BERR_TRACE(ret);	
		}
	}

    /* Unmask the free mark interrupt */
#if (BRAP_P_WATERMARK_WORKAROUND == 0)
#if (BRAP_P_EDGE_TRIG_INTRPT == 0)
    ret = BRAP_P_UnmaskInterrupt_isr(hRapCh, BRAP_Interrupt_eFmmRbufFreeByte,BRAP_FWIF_P_ResponseType_eNone);
    if(ret != BERR_SUCCESS)
	{
		BDBG_ERR(("BRAP_PB_UpdateBufUsg_isr: Couldn't unmask eFmmRbufFreeByte interrupt"));			
		return BERR_TRACE(ret);	
	}
#else
	BRAP_P_ReArmFreeMarkInterrupt_isr(hRapCh);
#endif    
#endif    



	BDBG_LEAVE(BRAP_PB_UpdateBufUsg_isr);
	return ret;
}


static BERR_Code BRAP_PB_P_PrepBufInfo_isr(
	BRAP_ChannelHandle 	    hRapCh,
    unsigned int 		    uiPth,	
	unsigned int 			uiChId,
	BRAP_RingBufInfo		*pRBufInfo
	)
{
	BERR_Code err = BERR_SUCCESS;
	BRAP_RBUF_P_Handle hRBuf;
	uint32_t ui32RdAddr = 0;
	uint32_t ui32WrAddr = 0;
	uint32_t ui32RegVal = 0;
	uint32_t ui32BaseAddr = 0;
    uint32_t ui32EndAddrOffset = 0;
	unsigned int uiRBufSize = 0;
	unsigned int uiFrameSize = 0;
    unsigned int uiGap = 256;

	BDBG_ENTER(BRAP_PB_P_PrepBufInfo_isr);

	/* Get the ring buffer handle */
	hRBuf = hRapCh->pPath[uiPth]->hRBuf[uiChId];

    pRBufInfo->uiRBufId= uiChId;
	if(hRBuf == NULL)
	{
		BDBG_ERR(("BRAP_PB_P_PrepBufInfo_isr: hRBuf[%d] is NULL", uiChId));
		return BERR_TRACE(BERR_NOT_INITIALIZED);
	}

	/* Get the base address */	
	ui32BaseAddr = BRAP_Read32_isr(
					hRBuf->hRegister,  
                	(BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR + hRBuf->ui32Offset));

    /* Get the end address */
    ui32EndAddrOffset = BRAP_Read32_isr(
					hRBuf->hRegister,  
                	(BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR + hRBuf->ui32Offset));
    	
	/* Get current write address */	
	ui32RegVal = BRAP_Read32_isr(
					hRBuf->hRegister,  
                	(BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR + hRBuf->ui32Offset));

    ui32WrAddr = BCHP_GET_FIELD_DATA(ui32RegVal,
				AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR,
				RINGBUF_WRADDR);	

	/* Get current read address */	
	ui32RegVal = BRAP_Read32_isr(
					hRBuf->hRegister,  
                	(BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + hRBuf->ui32Offset));

    ui32RdAddr = BCHP_GET_FIELD_DATA(ui32RegVal,
				AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR,
				RINGBUF_RDADDR);	


	/* Prepare return buf info structure */
	uiRBufSize = hRBuf->sSettings.sExtSettings.uiSize;

	/* Each Ring buffer is logically divided into 4 frames. This is also the max free
   	   contiguous ring buffer size in bytes that is informed to the user */
    BRAP_RBUF_P_GetFrameSize(hRBuf->sSettings.sExtSettings.uiSize, &uiFrameSize);
		
	pRBufInfo->pBasePtr = (hRBuf->sSettings.sExtSettings.pBufferStart);
    err = BRAP_ConvertOffsetToAddress(hRBuf->hHeap, ui32EndAddrOffset, (void **)&(pRBufInfo->pEndPtr));
    if(err != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_PB_P_PrepBufInfo_isr: BRAP_ConvertOffsetToAddress returned err(%d)", err));
        return BERR_TRACE(err);
    }

    err = BRAP_ConvertOffsetToAddress(hRBuf->hHeap, ui32WrAddr, (void **)&(pRBufInfo->pWritePtr));
    if(err != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_PB_P_PrepBufInfo_isr: BMEM_ConvertOffsetToAddress returned err(%d)", err));
        return BERR_TRACE(err);
    }

    err = BRAP_ConvertOffsetToAddress(hRBuf->hHeap, ui32RdAddr, (void **)&(pRBufInfo->pReadPtr));
    if(err != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_PB_P_PrepBufInfo_isr: BMEM_ConvertOffsetToAddress returned err(%d)", err));
        return BERR_TRACE(err);
    }

	if(ui32WrAddr >= ui32RdAddr)
	{
		/* tot free size = rbuf size - (wrPtr - rdPtr) */ 
		pRBufInfo->uiTotalSize = uiRBufSize - (ui32WrAddr - ui32RdAddr);

		/* cont free size = endPtr - wrPtr */
		pRBufInfo->uiContiguousSize = (ui32BaseAddr + uiRBufSize) - ui32WrAddr; 	 

		/* Clip cont free size if more than tot free size */
		if(pRBufInfo->uiContiguousSize > pRBufInfo->uiTotalSize)
			pRBufInfo->uiContiguousSize = pRBufInfo->uiTotalSize;
 	}
	else
	{
		/* cont free size = tot free size = rdPtr - wrPtr */ 
		pRBufInfo->uiTotalSize = ui32RdAddr - ui32WrAddr;
		pRBufInfo->uiContiguousSize = pRBufInfo->uiTotalSize;
	}

#if BRAP_PB_P_DEBUG_PCM
	BDBG_MSG(("Before FRAME logic W=%p R=%p T=%p C=%p F=%p",ui32WrAddr, ui32RdAddr, 
				pRBufInfo->uiTotalSize, pRBufInfo->uiContiguousSize, uiFrameSize));
#endif

	/* Return contiguous size keeping some gap between R-W ptrs */
   	if(pRBufInfo->uiTotalSize == pRBufInfo->uiContiguousSize)
    {
    	if(pRBufInfo->uiContiguousSize > uiGap)
    	{
    		pRBufInfo->uiContiguousSize -= uiGap;
    	}
    	else
    	{
        	pRBufInfo->uiContiguousSize = 0;
        }    
    }
    else if(pRBufInfo->uiTotalSize > pRBufInfo->uiContiguousSize)
    {
        /* The complete contiguous size can be returned to the app */
    }        
    else
    {
        /* This case should never occur */
        BDBG_ERR(("T(0x%x) < C(0x%x)",pRBufInfo->uiTotalSize,pRBufInfo->uiContiguousSize));
        BDBG_ASSERT(0);
    }

#if BRAP_PB_P_DEBUG_PCM
	BDBG_ERR(("W=%p R=%p T=%p C=%p F=%p",ui32WrAddr, ui32RdAddr, 
				pRBufInfo->uiTotalSize, pRBufInfo->uiContiguousSize, uiFrameSize));
#endif

	BDBG_LEAVE(BRAP_PB_P_PrepBufInfo_isr);
	return BERR_SUCCESS;
}

static BERR_Code BRAP_PB_P_WriteBufUsg_isr(
	BRAP_ChannelHandle 	hRapCh,
	unsigned int 		uiPth,
	unsigned int 		uiChId,
	unsigned int		uiSzUpdt
	)
{
	BRAP_RBUF_P_Handle hRBuf;
	uint32_t ui32WrAddr = 0;
	uint32_t ui32RegVal = 0;
	uint32_t ui32BaseAddr = 0;	
	unsigned int uiRBufSize = 0;
	uint32_t ui32NewWrAddr = 0;
	uint32_t ui32WrapAroundBit = 0;
	
	BDBG_ENTER(BRAP_PB_P_WriteBufUsg_isr);
#if BRAP_PB_P_DEBUG_PCM    
	BDBG_MSG(("BRAP_PB_P_WriteBufUsg_isr: hRapCh=%d, uiRBufI=%d, uiSzUpdt=%d",
		hRapCh, uiChId, uiSzUpdt));
#endif

	/* Get the ring buffer handle */
	hRBuf = hRapCh->pPath[uiPth]->hRBuf[uiChId];
	if(hRBuf == NULL)
	{
		BDBG_ERR(("BRAP_PB_P_WriteBufUsg_isr: hRBuf[%d] is NULL", uiChId));
		return BERR_TRACE(BERR_NOT_INITIALIZED);
	}

	/* Get the base address */	
	ui32BaseAddr = BRAP_Read32_isr(
					hRBuf->hRegister,  
                	(BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR + hRBuf->ui32Offset));

	/* Get current write address */	
	ui32RegVal = BRAP_Read32_isr(
					hRBuf->hRegister,  
                	(BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR + hRBuf->ui32Offset));

    ui32WrAddr = BCHP_GET_FIELD_DATA(ui32RegVal,
						AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR,
						RINGBUF_WRADDR);	

	ui32WrapAroundBit = BCHP_GET_FIELD_DATA(ui32RegVal,
						AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR,
						RINGBUF_WRADDR_WRAP);
#if BRAP_PB_P_DEBUG_PCM
	BDBG_MSG(("uiSzUpdt(0x%x) ui32WrAddr(0x%x) ui32BaseAddr(0x%x) uiRBufSize(0x%x)",
			uiSzUpdt , ui32WrAddr, ui32BaseAddr, uiRBufSize));
#endif

	/* Check if the uiSzUpdt is within the ring buffer */
	uiRBufSize = hRBuf->sSettings.sExtSettings.uiSize;
	if(uiSzUpdt + ui32WrAddr > ui32BaseAddr + uiRBufSize)
	{
		BDBG_ERR(("BRAP_PB_P_WriteBufUsg_isr: uiUsedSize exceeds ring buffer boundary"));
		BDBG_ERR(("uiSzUpdt(0x%x) + ui32WrAddr(0x%x) > ui32BaseAddr(0x%x) + uiRBufSize(0x%x)",
			uiSzUpdt , ui32WrAddr, ui32BaseAddr, uiRBufSize));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}
	else if(uiSzUpdt + ui32WrAddr == ui32BaseAddr + uiRBufSize)
	{ 
		/* wrap around write ptr to base ptr */
		ui32NewWrAddr = ui32BaseAddr;
		ui32WrapAroundBit ^= 1;
	}
	else
	{
		ui32NewWrAddr = ui32WrAddr + uiSzUpdt;
	}
		
	/* Write back the updated write pointer */
	ui32RegVal = (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, 
                        RINGBUF_WRADDR, 
                        ui32NewWrAddr));

	ui32RegVal |= (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, 
                        RINGBUF_WRADDR_WRAP,
                        ui32WrapAroundBit));

    BRAP_Write32_isr (hRBuf->hRegister,  
                  BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR + hRBuf->ui32Offset, 
                  ui32RegVal);

	BDBG_LEAVE(BRAP_PB_P_WriteBufUsg_isr);
	return BERR_SUCCESS;
}
/***************************************************************************
Summary:	
	Informs the PI about the completion of pumping of data to ring buffers.
Description:
	This API is required to be called by the application after it has completed
	writing of PCM data to ring buffers. This call is mandatory to ensure that
	it gets further interrupts in future.
Returns:
    Returns error on failure or BERR_SUCCESS on success
See Also:
	BRAP_PB_BufferWriteDone_isr
****************************************************************************/
BERR_Code BRAP_PB_BufferWriteDone(
	BRAP_ChannelHandle 	hRapCh		   /* [in] Audio channel handle */
	)
{
	BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER(BRAP_PB_BufferWriteDone);

#if BRAP_P_WATERMARK_WORKAROUND
	/* 1. Validate input parameters. */
	BDBG_ASSERT(hRapCh);	

	BKNI_EnterCriticalSection();
	ret = BRAP_PB_BufferWriteDone_isr(hRapCh);
	BKNI_LeaveCriticalSection();
#else
    BSTD_UNUSED(hRapCh);
#endif /* BRAP_P_WATERMARK_WORKAROUND */

	BDBG_LEAVE(BRAP_PB_BufferWriteDone);
	return ret;
}

/***************************************************************************
Summary:	
	Informs the PI about the completion of pumping of data to ring buffers.
Description:
	This API is required to be called by the application after it has completed
	writing of PCM data to ring buffers. This call is mandatory to ensure that
	it gets further interrupts in future. This API is used within the ISR context
Returns:
    Returns error on failure or BERR_SUCCESS on success
See Also:
	BRAP_PB_BufferWriteDone
****************************************************************************/
BERR_Code BRAP_PB_BufferWriteDone_isr(
	BRAP_ChannelHandle 	hRapCh		   /* [in] Audio channel handle */
	)
{
	BERR_Code ret = BERR_SUCCESS;
#if BRAP_P_WATERMARK_WORKAROUND
    BRAP_PcmBufInfo sPcmBufInfo;

	uint32_t	ui32WaterMarkinBytes =0;
	uint32_t	ui32TotalFreeBytes=0;
	uint32_t	ui32InitalTimerValue=0;
	uint32_t	ui32Diff=0;
	uint32_t	uiPth=0;    
	uint32_t	ui32IpBitsPerSample;
    BRAP_RBUF_P_Handle hRBuf = NULL;
    BRAP_BufDataMode eBufDataMode = BRAP_BufDataMode_eMaxNum;

	BDBG_ENTER(BRAP_PB_BufferWriteDone_isr);
    BDBG_MSG(("Entering BRAP_PB_BufferWriteDone_isr"));

    /* 1. Validate input parameters. */
	BDBG_ASSERT(hRapCh);

    ret = BTMR_StopTimer_isr ( hRapCh->hTimer );
	if ( ret != BERR_SUCCESS )
	{
		BDBG_ERR(("BRAP_PB_BufferWriteDone_isr() Failed to stop timer"));
		return BERR_TRACE(ret);
	}

	ret = BRAP_PB_GetBufInfo_isr(hRapCh, &sPcmBufInfo);
	if ( ret != BERR_SUCCESS )
	{
		BDBG_ERR(("BRAP_PB_BufferWriteDone_isr() Failed to ger buffer info"));
		return BERR_TRACE(ret);
	}

    if(BRAP_ChannelType_ePcmPlayback == hRapCh->eChannelType)
    {
        for(uiPth=0; uiPth<BRAP_P_MAX_PATHS_IN_A_CHAN; uiPth++)
        {
            if((hRapCh->pPath[uiPth] != NULL) && 
               (hRapCh->pPath[uiPth]->eUsgPath == BRAP_P_UsgPath_eDecodePcm))
               break;
        }
    	if (uiPth == BRAP_P_MAX_PATHS_IN_A_CHAN) 
        {
    		BDBG_ERR(("BRAP_PB_BufferWriteDone_isr: Decode PCM path not found in hRapCh=%p",
              hRapCh));
    		return BERR_TRACE(BERR_NOT_INITIALIZED);
    	}
    }
    else if(BRAP_ChannelType_eDecode == hRapCh->eChannelType)
    {
        for(uiPth=0; uiPth<BRAP_P_MAX_PATHS_IN_A_CHAN; uiPth++)
        {
            if((hRapCh->pPath[uiPth] != NULL) && 
               (hRapCh->pPath[uiPth]->eUsgPath == BRAP_P_UsgPath_eCapture))
               break;
        }
    	if (uiPth == BRAP_P_MAX_PATHS_IN_A_CHAN) 
        {
    		BDBG_ERR(("BRAP_PB_BufferWriteDone_isr: Capture path not found in hRapCh=%p",
                      hRapCh));
    		return BERR_TRACE(BERR_NOT_INITIALIZED);
    	}    
    }
    else
    {
        BDBG_ERR(("This API should not be called for this channel type %d",
                  hRapCh->eChannelType));
		return BERR_TRACE(BERR_NOT_SUPPORTED);        
    }
    
  	/* Get the ring buffer handle */
	hRBuf = hRapCh->pPath[uiPth]->hRBuf[sPcmBufInfo.sLtOrSnglBufInfo.uiRBufId];


	ui32WaterMarkinBytes = ( hRBuf->sSettings.sExtSettings.uiWaterMark *
						            hRBuf->sSettings.sExtSettings.uiSize )/100;
	ui32TotalFreeBytes = sPcmBufInfo.sLtOrSnglBufInfo.uiTotalSize;
    BDBG_MSG(("Before StartTimer"));
	if ( ui32TotalFreeBytes >= ui32WaterMarkinBytes )
	{
		ret=BTMR_StartTimer_isr(hRapCh->hTimer, 1000);
        if ( ret != BERR_SUCCESS )
    	{
    		BDBG_ERR(("BRAP_PB_BufferWriteDone_isr() Failed to ger buffer info"));
    		return BERR_TRACE(ret);
    	}
        goto end_writedone;
	}
	else
	{
	    
		ui32Diff = ( ui32WaterMarkinBytes - ui32TotalFreeBytes );

        ui32IpBitsPerSample = hRapCh->pPath[uiPth]->
         sSrcCh[BRAP_OutputChannelPair_eLR].hSrcCh->sParams.eInputBitsPerSample;



        eBufDataMode = hRapCh->pPath[uiPth]->
                sSrcCh[BRAP_OutputChannelPair_eLR].hSrcCh->sParams.eBufDataMode;

        if ( eBufDataMode == BRAP_BufDataMode_eStereoInterleaved )
		{
			ui32InitalTimerValue = ( ui32Diff * 4 * 1000 *1000) / ( hRapCh->uiInputSamplingRate * ui32IpBitsPerSample);
		}
		else
		{
			ui32InitalTimerValue = ( ui32Diff * 8 * 1000 *1000) / ( hRapCh->uiInputSamplingRate * ui32IpBitsPerSample);
		}

		BDBG_MSG(("ui32Diff= %d, hRapCh->uiInputSamplingRate= %d,ui32IpBitsPerSample = %d, ui32InitalTimerValue=%d"
                  "ui32WaterMarkinBytes=%d, ui32TotalFreeBytes= %d",
                  ui32Diff,hRapCh->uiInputSamplingRate,ui32IpBitsPerSample,ui32InitalTimerValue,
                  ui32WaterMarkinBytes,ui32TotalFreeBytes));
        
	}
    BDBG_MSG(("After StartTimer"));

	if ( ui32InitalTimerValue == 0 )
	{
		ui32InitalTimerValue = 1;
	}

	ret = BTMR_StartTimer_isr ( hRapCh->hTimer, ui32InitalTimerValue );
	if ( ret != BERR_SUCCESS )
	{
		BDBG_ERR(("BRAP_PB_BufferWriteDone_isr() Failed to start timer"));
		return BERR_TRACE(ret);
	}
    BDBG_MSG(("After BTMR_StartTimer"));

end_writedone:
	BDBG_LEAVE(BRAP_PB_BufferWriteDone_isr);
#else
    BSTD_UNUSED(hRapCh);

#endif /* BRAP_P_WATERMARK_WORKAROUND */
	return ret;
}


/**************************************************************************
Summary:
    Private function that gets the current audio params for a PCM Playback
    channel.
**************************************************************************/
BERR_Code
BRAP_PB_P_GetCurrentAudioParams(
	BRAP_ChannelHandle	    hRapCh,        /* [in] PCM PB channel handle */
	BRAP_ChannelParams	    *pAudioParams  /* [out] Current channel params */
	)
{
	BERR_Code                   err = BERR_SUCCESS;
    BRAP_OutputChannelPair      eChP = BRAP_OutputChannelPair_eMax;
    BRAP_SRCCH_P_Params         sSrcChParams;
    unsigned int                uiPth = 0;

	BDBG_ENTER(BRAP_PB_P_GetCurrentAudioParams);
	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(pAudioParams);
    
    if(hRapCh->eChannelType != BRAP_ChannelType_ePcmPlayback)
    {
        BDBG_ERR(("BRAP_PB_P_GetCurrentAudioParams: ChType(%d) is not PCM PB"
            " ChType(%d)", hRapCh->eChannelType, BRAP_ChannelType_ePcmPlayback));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BKNI_Memset(pAudioParams, 0, sizeof(BRAP_ChannelParams));
    
	/* Get parameters that are available in channel handle */
    pAudioParams->eAudioSource      = hRapCh->eAudioSource;
    BRAP_P_ConvertSrToEnum(hRapCh->uiInputSamplingRate,&(pAudioParams->eInputSR));        
    pAudioParams->bInputLfePresent  = hRapCh->bInputLfeOn;
    pAudioParams->eInputAudMode     = hRapCh->eInputAudMode;
    pAudioParams->sMixingCoeff      = hRapCh->sMixingCoeff;
    pAudioParams->eTimebase = BAVC_Timebase_e0;	

    /* Get SrcCh Params */
    for(eChP=0; eChP < BRAP_OutputChannelPair_eMax; eChP++)
    {
        if(NULL == hRapCh->pPath[uiPth]->sSrcCh[eChP].hSrcCh)
        {
            continue;
        }

        err = BRAP_SRCCH_P_GetCurrentParams(
                                hRapCh->pPath[uiPth]->sSrcCh[eChP].hSrcCh,
                                &sSrcChParams);
        if(err != BERR_SUCCESS)
        {
            return BERR_TRACE(err);
        }
        /* We got one SrcCh, break from the loop */
        break;
    }/* for eChP */
    if(BRAP_OutputChannelPair_eMax == eChP)
    {
        BDBG_ERR(("BRAP_PB_P_GetCurrentAudioParams: hSrcCh not found"));
		return BERR_TRACE(BERR_NOT_INITIALIZED);
    }

    pAudioParams->eInputBitsPerSample   = sSrcChParams.eInputBitsPerSample;
    pAudioParams->eBufDataMode          = sSrcChParams.eBufDataMode; 

#if (BRAP_UNSIGNED_PCM_SUPPORTED == 1)
    pAudioParams->bIsPcmSigned = sSrcChParams.bIsSigned;
#endif
    
    
    /* Invalid entries for params not required for a decode channel */
    pAudioParams->eCapMode      = BRAP_CaptureMode_eMaxCaptureMode;
    pAudioParams->eCapInputPort = hRapCh->eCapInputPort;
    pAudioParams->eInputChPair  = BRAP_OutputChannelPair_eLR;

    BKNI_Memset(&(pAudioParams->sXptContextMap), BRAP_INVALID_VALUE, 
                sizeof(BAVC_XptContextMap));
    
	BDBG_LEAVE( BRAP_PB_P_GetCurrentAudioParams );
	return err;
}

#if (BRAP_7550_FAMILY != 1)
BERR_Code 
BRAP_PB_SetInputSampleRate(
		BRAP_ChannelHandle hRapCh,/*PCM PB Channel Handle*/
		unsigned int uiInputSampleRate/*Input Sample rate of PCM PB content.
		                                                   It can have any +ve possible value. UNit should be Hz.
		                                                   e.g. 44100 for 44.1K , 9000 for 9k. etc.*/
		)
{
    BERR_Code   ret= BERR_SUCCESS;
    bool                    bPrimary[BRAP_MAX_ASSOCIATED_GROUPS];    
    BRAP_ChannelHandle      hStartedAssocCh[BRAP_MAX_ASSOCIATED_GROUPS];
    unsigned int uiAssocId=BRAP_INVALID_VALUE,uiOutputSamplerate;

    BDBG_ENTER(BRAP_PB_SetInputSampleRate);
    BDBG_ASSERT(hRapCh);
    if(hRapCh->eChannelType != BRAP_ChannelType_ePcmPlayback)
    {
        BDBG_ERR(("BRAP_PB_SetInputSampleRate: This function should be called only for Plaback Channel"));
        ret = BERR_INVALID_PARAMETER;
        goto end;
    }
    if(uiInputSampleRate  == 0)
    {
        BDBG_ERR(("BRAP_PB_SetInputSampleRate: Invalid Input Sample rate =%d",uiInputSampleRate));
        ret = BERR_INVALID_PARAMETER;
        goto end;
    }


    if(hRapCh->eState == BRAP_P_State_eStarted)
    {
        for(uiAssocId=0; uiAssocId<BRAP_MAX_ASSOCIATED_GROUPS; uiAssocId++)
        {
            hStartedAssocCh[uiAssocId]=NULL;
            bPrimary[uiAssocId]=false;
        }    
        ret = BRAP_P_GetOtherStartedChannel(hRapCh,bPrimary,hStartedAssocCh);
        if(BERR_SUCCESS != ret)
        {
            ret = BERR_TRACE(ret);
            goto end;    
        }                        
        for(uiAssocId=0; uiAssocId<BRAP_MAX_ASSOCIATED_GROUPS; uiAssocId++)
        {
            if(hRapCh->uiAssociationId[uiAssocId] != BRAP_INVALID_VALUE)
            {
                if((hStartedAssocCh[hRapCh->uiAssociationId[uiAssocId]] != NULL)
                    &&(bPrimary[hRapCh->uiAssociationId[uiAssocId]] == true)
                    &&(hStartedAssocCh[hRapCh->uiAssociationId[uiAssocId]]->bGateOpened == true))
                    {
                        BRAP_P_ConvertSR(hStartedAssocCh[hRapCh->uiAssociationId[uiAssocId]]->eSamplingRate,&uiOutputSamplerate);
                        BDBG_MSG(("Function = %s , Line = %d,uiOutputSamplerate = %d",__FUNCTION__,__LINE__,uiOutputSamplerate));                                                    
                    }
                    else
                    {
                        BRAP_P_ConvertSR(hRapCh->eSamplingRate,&uiOutputSamplerate);
                        BDBG_MSG(("Function = %s , Line = %d,uiOutputSamplerate = %d",__FUNCTION__,__LINE__,uiOutputSamplerate));                                                    
                    }
                    break;
            }            
            
        }
    	hRapCh->uiInputSamplingRate = uiInputSampleRate;
        BKNI_EnterCriticalSection();        
        ret = BRAP_P_ConfigureAssociatedChannelsSrc_isr(hRapCh,uiOutputSamplerate);
        BKNI_LeaveCriticalSection();        
        if(BERR_SUCCESS != ret)
        {
            ret = BERR_TRACE(ret);
            goto end;    
        }              
    }
    else
    {
    	hRapCh->uiInputSamplingRate = uiInputSampleRate;
    }

end:        

    BDBG_ENTER(BRAP_PB_SetInputSampleRate);   
    return ret;
}
#else
BERR_Code 
BRAP_PB_SetInputSampleRate(
		BRAP_ChannelHandle hRapCh,/*PCM PB Channel Handle*/
		unsigned int uiInputSampleRate/*Input Sample rate of PCM PB content.
		                                                   It can have any +ve possible value. UNit should be Hz.
		                                                   e.g. 44100 for 44.1K , 9000 for 9k. etc.*/
		)
{
    BERR_Code   ret= BERR_SUCCESS;
    BDBG_ENTER(BRAP_PB_SetInputSampleRate);
    BDBG_ASSERT(hRapCh);
    BSTD_UNUSED(hRapCh);
    BSTD_UNUSED(uiInputSampleRate);    
    BDBG_ERR(("API NOT Supported for 7550"));
    BDBG_ENTER(BRAP_PB_SetInputSampleRate);   
    return ret;    
}
#endif

