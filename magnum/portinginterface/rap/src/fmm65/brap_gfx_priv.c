/***************************************************************************
*     Copyright (c) 2006-2010, Broadcom Corporation
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
*	This file contains structures, enums, macros and function prototypes, 
*	which are exposed to the upper layer for Grahics on ZSP by Rap PI. In other 
*	words this file defines the interfaces between the upper layer and the 
*	Raptor Gfx PI layer.
*
* Revision History:
*
* $brcm_Log: $
* 
*****************************************************************************/
#include "brap_gfx_priv.h"
#include "brap_priv.h"


BDBG_MODULE(rap_gfx_priv);

#ifdef RAP_GFX_SUPPORT

BERR_Code
BRAP_GFX_DSPCHN_P_StartGfx(
    BRAP_DSPCHN_Handle  hDspCh               /* [in] DSPCHN handle */
    );

BERR_Code BRAP_GFX_P_InitInterframeBuffer(
				BRAP_FWIF_P_FwTaskHandle	hTask);

BERR_Code
BRAP_GFX_DSPCHN_P_StopGfx(
    BRAP_DSPCHN_Handle  hDspCh               /* [in] DSPCHN handle */
    );

BERR_Code BRAP_P_OpenGfxPath( 
    BRAP_ChannelHandle          hRapCh,         /* [in] RAP Channel handle */
    const BRAP_GFX_OpenSettings    *pGfxParams   /* [in] Gfx parameters for 
                                                   starting this channel */
    );


BERR_Code BRAP_P_StartGfxPath( 
    BRAP_ChannelHandle          hRapCh,         /* [in] RAP Channel handle */
    const BRAP_GFX_OpenSettings    *pGfxParams,   /* [in] Gfx parameters for 
                                                   starting this channel */
    const unsigned int          uiPth       	/* [in] Path idx corresponding 
                                                   to pPath[] array element */                                                   
    );


BERR_Code BRAP_P_StopGfxPath ( 
    BRAP_ChannelHandle          hRapCh,        /* [in] RAP Channel handle */
    const unsigned int          uiPth       	/* [in] Path idx corresponding 
                                                   to pPath[] array element */       
    );

BERR_Code BRAP_P_CloseGfxPath( 
    BRAP_ChannelHandle          hRapCh         /* [in] RAP Channel handle */
    );

    BERR_Code
    BRAP_GFX_P_ConvertPixelFormat (
        BPXL_Format    ePixelAppFormat,/*[in]*/
        BRAP_GFX_P_ePixelFormatSupported    *ePixelFwFormat,/*[out]*/
        uint32_t            *ui32BitsPerPixel   /*[out]*/
        );

BERR_Code BRAP_FWIF_P_GenerateGfxCitInput(
    BRAP_Handle                         hRap,               /* [in] Rap Device Handle */
    BRAP_DSPCHN_Handle	hDspCh,					/* [in] DSPCHN handle */
    BRAP_P_AudioProcNetwork             *psAudProcNetwork,  /* [in] Audio Processing Network */
    BRAP_CIT_P_InputInfo                *pCitInputStruct,    /* [out] CIT input structure */
    BRAP_FWIF_P_FwTaskHandle hFwTaskCreate     /*[in] Fw Task handle */
);

BERR_Code   BRAP_GFX_P_ConvertSurface(
        BRAP_Handle    hRap,
                BSUR_Surface_Handle  psSurfaceAppFormat,
                BRAP_GFX_P_SurfaceStruct    *psSurfaceFwFormat);

BERR_Code   BRAP_GFX_P_NormalizeRectangle(
                    BRAP_GFX_P_SurfaceStruct    *psSurface,/*[In]*/
                    const BRAP_GFX_Rect                     *psRectAppFormat,/*[in]*/
                    BRAP_GFX_P_Rect                 *psRectFwFormat   /*[out]*/ 
);

/***************************************************************************
Summary:
    Starts a Graphics channel.
    
Description:
    This API is required to start the selected Graphics channel.
    
Returns:
    BERR_SUCCESS if successful else error value.            
    
See Also:
    BRAP_GFX_StopChannel
    BRAP_GFX_GetDefaultChannelParams
****************************************************************************/

BERR_Code BRAP_GFX_P_StartChannel ( 
    BRAP_GFX_Handle          hGfxCh,         /* [in] Gfx Channel handle */
    const BRAP_GFX_OpenSettings    *pChnSettings   /* [in] Gfx parameters for 
                                                   starting this channel */
    )
{
    BERR_Code ret = BERR_SUCCESS;
    bool bWdgRecovery = false;
    BRAP_ChannelHandle hRapCh=NULL;    

    BDBG_ENTER(BRAP_GFX_P_StartChannel);

    /* Validate parameters */
    BDBG_ASSERT(hGfxCh);
    BDBG_ASSERT(pChnSettings);

    hRapCh = (BRAP_ChannelHandle)hGfxCh;

    /* Check if this is a watchdog recovery. */
    bWdgRecovery = BRAP_P_GetWatchdogRecoveryFlag(hRapCh->hRap);

    /*Start Sanity Checks*/
    if(false == BRAP_FWDWNLD_P_IsGfxSupported(0))
    {
        BDBG_ERR(("Please export RAP_GFX_SUPPORT=y at the compile time for SOft Gfx"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }    
    /*End Sanity Checks*/


    /* Open all the paths in this channel */                                                    
    BDBG_MSG(("Open Gfx Path"));
    ret = BRAP_P_OpenGfxPath(hRapCh,pChnSettings);


    /* Start all the paths in this channel */                                                    
    BDBG_MSG(("Start Gfx Path"));
    ret = BRAP_P_StartGfxPath(hRapCh, pChnSettings,0);
       
    /* Update the channel state */
    if((false == bWdgRecovery)
    && (BERR_SUCCESS == ret))
    {
        hRapCh->eState = BRAP_P_State_eStarted;
        hRapCh->sGfxSettings.sOpenGfxSettings= *pChnSettings;
    }  

BDBG_LEAVE(BRAP_GFX_P_StartChannel);

return ret;

}


/***************************************************************************
Summary:
    Stops a Graphics channel

Description:
    This API is required to stop a Graphics channel. 

Returns:
    BERR_SUCCESS if successful else error value.            

See Also:
    BRAP_GFX_StartChannel
****************************************************************************/

BERR_Code BRAP_GFX_P_StopChannel ( 
    BRAP_GFX_Handle          hGfxCh          /* [in] Gfx channel handle */
    )
{
    BERR_Code ret = BERR_SUCCESS;
    BRAP_ChannelHandle hRapCh=NULL;     

    BDBG_ENTER(BRAP_GFX_P_StopChannel);

    /* Validate parameters */
    BDBG_ASSERT(hGfxCh);
    hRapCh = (BRAP_ChannelHandle)hGfxCh;


    hRapCh->bStopinvoked= true;


    /* Stop all the paths in this channel */                                                    
    BDBG_MSG(("Stop Gfx Path"));
    ret = BRAP_P_StopGfxPath(hRapCh, 0);
    

    /* Close all the paths in this channel */                                                    
    BDBG_MSG(("Close Gfx Path"));
    ret = BRAP_P_CloseGfxPath(hRapCh);


       
    /* Update the channel state */
    /* Update the channel state */
    if(BERR_SUCCESS == ret)
    {
        hRapCh->eState = BRAP_P_State_eOpened;
        hRapCh->bStopinvoked= false;                
    }
BDBG_LEAVE(BRAP_GFX_P_StopChannel);
return ret;

}



BERR_Code BRAP_P_OpenGfxPath( 
    BRAP_ChannelHandle          hRapCh,         /* [in] RAP Channel handle */
    const BRAP_GFX_OpenSettings    *pChnSettings   /* [in] Gfx parameters for 
                                                   starting this channel */
    )
{
    BERR_Code ret = BERR_SUCCESS;
    bool bWdgRecovery = false;
    unsigned int  uiPth = 0;    
    BRAP_DSPCHN_P_Settings	*pDspChSettings = NULL;

    BDBG_ENTER(BRAP_P_OpenGfxPath);

    /* Validate parameters */
    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(pChnSettings);
    BSTD_UNUSED(pChnSettings);

    /* Check if this is a watchdog recovery. */
    bWdgRecovery = BRAP_P_GetWatchdogRecoveryFlag(hRapCh->hRap);

    /*Create Gfx Network*/

    if(false == bWdgRecovery)
    {
        hRapCh->sChAudProcessingStage[0][0].bDecoderStage = true;
    }

    /*Create Gfx Path*/
    pDspChSettings = ( BRAP_DSPCHN_P_Settings *) BKNI_Malloc( sizeof( BRAP_DSPCHN_P_Settings ));
    if ( NULL==pDspChSettings )
    {
        ret = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto end;        
    }            
    BKNI_Memset(pDspChSettings, 0, sizeof(BRAP_DSPCHN_P_Settings));         
    /* Form the DSP Channel Settings */

    ret = BRAP_DSPCHN_P_GetDefaultSettings(pDspChSettings);
    if(BERR_SUCCESS != ret)
    {
        BDBG_ERR(("DSPCHN_P_GetDefaultSettings: returned %d", ret));
        ret = BERR_TRACE(ret); 
        goto end;        
    }      

    if(NULL == hRapCh->pPath[uiPth])
     {
         ret = BRAP_P_MallocPath(&(hRapCh->pPath[uiPth]));
         if(BERR_SUCCESS != ret)
         {
             ret = BERR_TRACE(ret);
             goto end;
         }            
     }
    
    uiPth=0;
     BRAP_P_SET_USG_PATH(hRapCh->pPath[uiPth],
                                             BRAP_P_UsgPath_eGfx,
                                             BRAP_INVALID_VALUE,
                                             uiPth,
                                             uiPth+1,0);
     hRapCh->pPath[uiPth]->uiAssocId = BRAP_INVALID_VALUE;         
     
     /* Instantiate the DSP Channel corresponding to the DSP context resource 
     manager has allocated */
     ret = BRAP_DSPCHN_P_Open(&(hRapCh->pPath[uiPth]->hDspCh),
                                                     hRapCh,
                                                     pDspChSettings);
     if(BERR_SUCCESS != ret)
     {
         BDBG_ERR(("DSPCHN_P_Opened failed with err = %d", ret));
         ret = BERR_TRACE(ret);
         goto end;
     }
     
     if((BERR_SUCCESS == ret) && (false == bWdgRecovery))
     {
         /* Increment the open count */
         hRapCh->pPath[uiPth]->uiPthOpenCnt++;
              BDBG_MSG(("BRAP_P_OpenGfxPath: hRapCh->pPath[uiPth=%d]->uiPthOpenCnt = %d",
             uiPth, hRapCh->pPath[uiPth]->uiPthOpenCnt));
     }

end:
    if(pDspChSettings!=NULL)
        BKNI_Free(pDspChSettings);

BDBG_LEAVE(BRAP_P_OpenGfxPath);
return ret;    
}

BERR_Code BRAP_P_StartGfxPath( 
    BRAP_ChannelHandle          hRapCh,         /* [in] RAP Channel handle */
    const BRAP_GFX_OpenSettings    *pChnSettings,   /* [in] Gfx parameters for 
                                                   starting this channel */
    const unsigned int          uiPth       	/* [in] Path idx corresponding 
                                                   to pPath[] array element */                                                   
    )
{
        BERR_Code ret = BERR_SUCCESS;
        bool                        bWdgRecovery = false;
        bWdgRecovery = BRAP_P_GetWatchdogRecoveryFlag(hRapCh->hRap);
    
    
        BDBG_ENTER (BRAP_P_StartGfxPath);
        BDBG_ASSERT(hRapCh);
        BDBG_ASSERT(pChnSettings);   
        BSTD_UNUSED(pChnSettings);
        
        if(NULL == hRapCh->pPath[uiPth])
        {
            ret = BERR_TRACE(BERR_NOT_INITIALIZED);
            goto exit;
        }    
        /* Check if the path has already been started. If yes, just increment the 
           count and return success. */
        if((bWdgRecovery == false) 
            && (0 != hRapCh->pPath[uiPth]->uiPthStartCnt))
        {
            /* Increment the open count */
            hRapCh->pPath[uiPth]->uiPthStartCnt++;
    
            BDBG_MSG(("BRAP_P_StartGfxPath: GFX Path %d was already started. " 
                    "New start count = %d", uiPth, hRapCh->pPath[uiPth]->uiPthStartCnt));
    
            /* Return success */
            BDBG_LEAVE (BRAP_P_StartGfxPath);
            ret = BERR_SUCCESS;
            goto exit;        
        }  
            
        ret = BRAP_GFX_DSPCHN_P_StartGfx(hRapCh->pPath[uiPth]->hDspCh);
        if(BERR_SUCCESS != ret)
        {
            BDBG_ERR(("BRAP_P_StartGfxPath: BRAP_DSPCHN_P_StartGfx failed %d", ret));
            ret = BERR_TRACE(ret); 
            goto exit;
        }    
        
        if((BERR_SUCCESS == ret) && (bWdgRecovery == false))
        {
            /* Increment the start count */
            hRapCh->pPath[uiPth]->uiPthStartCnt++;
    
            BDBG_MSG(("BRAP_P_StartGfxPath: hRapCh->pPath[uiPth=%d]->uiPthStartCnt = %d",
            uiPth, hRapCh->pPath[uiPth]->uiPthStartCnt));
    
            /* Exit successfully */
            goto exit;    
        }
        
    exit:        
        BDBG_MSG(("BRAP_P_StartGfxPath Done!"));
        BDBG_LEAVE(BRAP_P_StartGfxPath);
        return ret;
    }

    /***************************************************************************
    Summary: Starts a DSP context for Gfx
    
    Description:
        This function is responsible for starting a DSP context for the given
        input decode mode.
        
    Returns:
        BERR_SUCCESS - If successful
    
    See Also:
        BRAP_DSPCHN_P_Stop
    **************************************************************************/
    BERR_Code
    BRAP_GFX_DSPCHN_P_StartGfx(
        BRAP_DSPCHN_Handle  hDspCh               /* [in] DSPCHN handle */
        )
{
            BRAP_P_AudioProcNetwork     *psAudProcNetwork = NULL;
            BRAP_CIT_P_InputInfo        *psCitInputStruct = NULL;
            unsigned int i = 0, j = 0, k = 0;
            BERR_Code err = BERR_SUCCESS;
            BRAP_FWIF_MsgQueueHandle hMsgQueue;
            uint32_t ui32SyncFifo = BRAP_DSP_P_FIFO_INVALID;
            uint32_t ui32AsyncFifo = BRAP_DSP_P_FIFO_INVALID;
            BRAP_FWIF_P_FwTaskHandle hFwTaskCreate = NULL;
            BRAP_FWIF_P_Command sCommand;
            BRAP_FWIF_P_Response sRsp;
            BRAP_Handle hRap = NULL;
            uint32_t ui32PhysAddr = 0,ui32StatusBufAdr=BRAP_INVALID_VALUE;
            BRAP_P_MsgType      eMsgType;
            BRAP_DSP_Handle hDsp = NULL;
            bool                        bWdgRecovery = false;
            uint32_t    ui32QueueId;     
            BRAP_FWDWNLD_P_Param    sParam;
            BRAP_FWIF_P_TaskParamInfo *pTaskParams = NULL;
            
            
            BDBG_ENTER( BRAP_GFX_DSPCHN_P_StartGfx );
        
            hRap = hDspCh->uHandle.hRapCh->hRap;
        
            bWdgRecovery = BRAP_P_GetWatchdogRecoveryFlag(hRap);
        
            hDspCh->bChSpecificDspCh = true;
            
            /* Allocate big structures on heap */
            psAudProcNetwork = (BRAP_P_AudioProcNetwork *) BKNI_Malloc(sizeof(BRAP_P_AudioProcNetwork));
            if (NULL==psAudProcNetwork)
            {
                BDBG_ERR(("Out of System memory"));
                err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                goto error;
            }
            BKNI_Memset(psAudProcNetwork, 0,(sizeof(BRAP_P_AudioProcNetwork)));
        
            psCitInputStruct = (BRAP_CIT_P_InputInfo *)BKNI_Malloc(sizeof(BRAP_CIT_P_InputInfo));
            if (NULL==psCitInputStruct)
            {
                BDBG_ERR(("Out of System memory"));
                err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                goto error;
            }
            BKNI_Memset( psCitInputStruct, 0, sizeof(BRAP_CIT_P_InputInfo) );
                           
            if(BRAP_P_CheckIfPtrInvalid((void *)hDspCh->uHandle.hRapCh) == false)
            {
                for(j=0;j<BRAP_P_MAX_DST_PER_RAPCH;j++)
                {
                    for(k=0;k<BRAP_MAX_STAGE_PER_BRANCH_SUPPORTED;k++)
                    {
                        psAudProcNetwork->sAudProcessingStage[j][k]=  hDspCh->uHandle.hRapCh->sChAudProcessingStage[j][k];
                    }
                }
            }        
            
            if(false == bWdgRecovery)
            {
                /* TODO: Split the network into multiple small networks */
                BRAP_DSPCHN_P_CreateFwTask(hDspCh,psAudProcNetwork);
            }
        
            if(false == hRap->sSettings.bOpenTimeFwDownload)
            {
                /* Download the firmware binaries required by the complete network */
                sParam.hRap = hRap;
                BRAP_FWDWNLD_P_DownloadGfxExecs(&sParam,false,BRAP_DSPCHN_GfxType_eSoftGfx);        
                if( BERR_SUCCESS!=err )
                {
                    goto error;
                }
            }
        
            for (i = 0; i < BRAP_FWIF_P_MAX_FW_TASK_PER_DSPCHN; i++)
            {
                hFwTaskCreate = hDspCh->sFwTaskInfo[i].hFwTask;
                hFwTaskCreate->uiCommandCounter= 0;
                if(BRAP_P_CheckIfPtrInvalid((void *)hFwTaskCreate) == true)
                {
                    BDBG_MSG(("Task handle is NULL for Task no. %d",i+BRAP_FWIF_P_TASK_ID_START_OFFSET));
                    continue;
                }
                hDsp = hFwTaskCreate->hDsp;
        
                BDBG_MSG(("hFwTaskCreate->uiTaskId = %d , hDsp->uiDspIndex =%d",hFwTaskCreate->uiTaskId,hDsp->uiDspIndex));
        
                /* Convert processing network into CIT module input */
                err = BRAP_FWIF_P_GenerateGfxCitInput(hRap,hDspCh, &(hDspCh->sFwTaskInfo[i].sProcessingNw), psCitInputStruct,hFwTaskCreate);
                if( BERR_SUCCESS!=err )
                {
                    goto error;
                }     
                BRAP_P_ShowCitSettings(&(hDspCh->sFwTaskInfo[i].sProcessingNw), psCitInputStruct);
        
                /* Call CIT module */
                BDBG_MSG(("sCitOutput = %#x",&(hFwTaskCreate->sGfxCitOutput)));
                BDBG_MSG(("Calling CIT Generation Module"));
                err =BRAP_P_GenCitGfx(hRap->hHeap, hRap->hRegister, psCitInputStruct, NULL,&(hFwTaskCreate->sGfxCitOutput));
                if(BERR_SUCCESS != err)
                {
                    BDBG_ERR(("ERROR returned from Cit module %d!",err));
                    err =BERR_TRACE(err);
                    goto error;
                }

                if (BRAP_P_CheckIfPtrInvalid((void *)psCitInputStruct->pBranchInfo[0]) == false)
                {
                    BKNI_Free(psCitInputStruct->pBranchInfo[0]);
                    psCitInputStruct->pBranchInfo[0] = NULL;
                }
                
                BDBG_MSG(("GENCIT DONE"));
        
                /*  Analyze the CIT generated : First level Information */
                BRAP_P_AnalyseGfxCit(hRap->hHeap, &(hFwTaskCreate->sGfxCitOutput.sGfxCit));
                BDBG_MSG(("ANALYSECIT DONE"));

                /* Download CIT structure into DRAM */
                err = BRAP_P_CopyDataToDram(
                        hDsp->hHeap, 
                        (uint32_t *)(&(hFwTaskCreate->sGfxCitOutput.sGfxCit)),      
                        hRap->sMemInfo.sTskMemInfo[hDsp->uiDspIndex].sCitStruct[BRAP_GET_TASK_INDEX(hFwTaskCreate->uiTaskId)].ui32BaseAddr,
                        hRap->sMemInfo.sTskMemInfo[hDsp->uiDspIndex].sCitStruct[BRAP_GET_TASK_INDEX(hFwTaskCreate->uiTaskId)].ui32Size
                        );
        
                if(BERR_SUCCESS != err)
                {
                       BDBG_ERR(("Error in Copying data to DRAM"));
                       err = BERR_TRACE(err);
                       goto error;
                }           
        
                /* Initialize interframe buffers for all the nodes */
        
                err = BRAP_GFX_P_InitInterframeBuffer(hFwTaskCreate);  
                if(BERR_SUCCESS != err)
                {
                       BDBG_ERR(("Error in initializing Interframe buffers for Task id %d",hFwTaskCreate->uiTaskId));
                       err = BERR_TRACE(err);
                       goto error;
                }
        
                /* Initialize Status buffers for all the nodes */                
                ui32StatusBufAdr = hFwTaskCreate->sGfxCitOutput.sCitBranchInfo[0].sCitStgInfo[0].sFwOpNodeInfo[0].ui32StatusBufAdr; 
                
                if((ui32StatusBufAdr != BRAP_P_INVALID_DRAM_ADDRESS) &&
                   (ui32StatusBufAdr != 0) &&
                   (ui32StatusBufAdr != BRAP_AF_P_DRAM_ADDR_INVALID)&&
                   (0 != hFwTaskCreate->sGfxCitOutput.sCitBranchInfo[0].sCitStgInfo[0].sFwOpNodeInfo[0].ui32StatusBufSize)) 
                {
                    BKNI_Memset((void *)ui32StatusBufAdr,0xFF,hFwTaskCreate->sGfxCitOutput.sCitBranchInfo[0].sCitStgInfo[0].sFwOpNodeInfo[0].ui32StatusBufSize);
                }
        
                
                if(false == bWdgRecovery)
                {
                    /* Find free Fifo Ids */
                    for(j=0;j<BRAP_DSP_P_NUM_FIFOS;j++)
                    {
                        if(false == hDsp->bFifoUsed[j])
                            break;
                    }
                    if(BRAP_DSP_P_NUM_FIFOS == j)
                    {
                        BDBG_ERR(("Unable to find free fifo for SYNC MSG!"));
                        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                        goto error;
                    }
                    else
                    {
                        hDsp->bFifoUsed[j] = true;
                        ui32SyncFifo = j;
                    }
                    for(j=0;j<BRAP_DSP_P_NUM_FIFOS;j++)
                    {
                        if(false == hDsp->bFifoUsed[j])
                            break;
                    }
                    if(BRAP_DSP_P_NUM_FIFOS == j)
                    {
                        BDBG_ERR(("Unable to find free fifo for ASYNC MSG!"));
                        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                        goto error;
                    }
                    else
                    {
                        hDsp->bFifoUsed[j] = true;
                        ui32AsyncFifo = j;
                    }
                    
                    for(j=0;j<BRAP_MAX_FW_TSK_SUPPORTED;j++)
                    {
                            if(BRAP_FWIF_P_INVALID_TSK_ID == hRap->sMemInfo.sTskMemInfo[hDsp->uiDspIndex].sTskQueue[j].ui32TaskId)
                            break;
                    }
                    if(BRAP_MAX_FW_TSK_SUPPORTED == j)
                    {
                        BDBG_ERR(("Unable to find free queues for this task!"));
                        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                        goto error;
                    }
                    else
                    {
                        ui32QueueId = j;
                    }
                        hRap->sMemInfo.sTskMemInfo[hDsp->uiDspIndex].sTskQueue[ui32QueueId].ui32TaskId = hFwTaskCreate->uiTaskId;
                        hRap->sMemInfo.sTskMemInfo[hDsp->uiDspIndex].sTskQueue[ui32QueueId].sTskSyncQueue.ui32FifoId
                            = ui32SyncFifo;
                        hRap->sMemInfo.sTskMemInfo[hDsp->uiDspIndex].sTskQueue[ui32QueueId].sTskAsyncQueue.ui32FifoId
                            = ui32AsyncFifo;
                }
                else
                {
                    for(j=0;j<BRAP_MAX_FW_TSK_SUPPORTED;j++)
                    {
                        if(hFwTaskCreate->uiTaskId == hRap->sMemInfo.sTskMemInfo[hDsp->uiDspIndex].sTskQueue[j].ui32TaskId)
                            break;
                    }
                    if(BRAP_MAX_FW_TSK_SUPPORTED == j)
                    {
                        BDBG_ERR(("Unable to find matching queues for this task in the handle for watchdog recovery!"));
                        err = BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
                        goto error;
                    }
                    else
                    {
                        ui32QueueId = j;
                    }
                }
        
                if(true == bWdgRecovery)
                {
                       hMsgQueue = hFwTaskCreate->hSyncMsgQueue;
                }
                    
                    err = BRAP_FWIF_P_CreateMsgQueue(&(hRap->sMemInfo.sTskMemInfo[hDsp->uiDspIndex].sTskQueue[ui32QueueId].sTskSyncQueue), 
                        hDspCh->hHeap, hDspCh->hRegister, hDsp->ui32Offset, &hMsgQueue,bWdgRecovery);
                    if(BERR_SUCCESS != err)
                    {
                        BDBG_ERR(("BRAP_GFX_DSPCHN_P_StartGfx: SYNC RSP queue creation failed!"));
                        err = BERR_TRACE(err);
                        goto error;
                    }
                   if(false == bWdgRecovery)
                    {
                        /* Fill up task handle message queue */
                        hFwTaskCreate->hSyncMsgQueue = hMsgQueue;
                    }
        
                    if(true == bWdgRecovery)
                    {
                           hMsgQueue = hFwTaskCreate->hAsyncMsgQueue;
                    }
                    err = BRAP_FWIF_P_CreateMsgQueue(&(hRap->sMemInfo.sTskMemInfo[hDsp->uiDspIndex].sTskQueue[j].sTskAsyncQueue), 
                            hDspCh->hHeap, hDspCh->hRegister, hDsp->ui32Offset, &hMsgQueue,bWdgRecovery);
                    if(BERR_SUCCESS != err)
                    {
                        BDBG_ERR(("BRAP_GFX_DSPCHN_P_StartGfx: ASYNC RSP queue creation failed!"));
                        err = BERR_TRACE(err);
                        goto error;
                    }
                   if(false == bWdgRecovery)
                    {
                        /* Fill up task handle message queue */
                        hFwTaskCreate->hAsyncMsgQueue = hMsgQueue;
                    }        
                   if(false == bWdgRecovery)
                    {
                       hFwTaskCreate->pAsyncMsgMemory = 
                        (void *)(hRap->sMemInfo.sTskMemInfo[hDsp->uiDspIndex].sTskQueue[j].sAsyncMsgBufmem.ui32BaseAddr);
                    }        
        
        
                /* Unmask interrupt bits corresponding to synchronous and asynchronous msg queues */
                err = BRAP_P_UnmaskTask(hFwTaskCreate);
                if(BERR_SUCCESS != err)
                {
                    BDBG_ERR(("BRAP_GFX_DSPCHN_P_StartGfx: Unable to unmask task id interrupt!"));
                    err = BERR_TRACE(err);
                    goto error;
                }
        
                /* Send Start Task Command */
                sCommand.sCommandHeader.ui32CommandID = BRAP_FWIF_P_START_TASK_COMMAND_ID;
                sCommand.sCommandHeader.ui32CommandCounter = hFwTaskCreate->uiCommandCounter++;
                sCommand.sCommandHeader.ui32TaskID = hFwTaskCreate->uiTaskId;
                sCommand.sCommandHeader.eResponseType = BRAP_FWIF_P_ResponseType_eResponseRequired;
        
                sCommand.uCommand.sStartTask.eTaskAlgoType = BRAP_FWIF_P_AlgoType_eGfxDecode; 
                sCommand.uCommand.sStartTask.eDeadlineComputationFuncType =  BRAP_FWIF_P_DeadLineComputeFuncType_eGraphicsTask;
                sCommand.uCommand.sStartTask.ui32MasterTaskId= BRAP_FWIF_P_INVALID_TSK_ID;                        
                sCommand.uCommand.sStartTask.eTaskType = BRAP_FWIF_P_TaskType_eSoftRealtime;


                BRAP_P_ConvertAddressToOffset(hRap->hHeap, 
                    (void *)(hRap->sMemInfo.sTskMemInfo[hDsp->uiDspIndex].sTskInfo[BRAP_GET_TASK_INDEX(hFwTaskCreate->uiTaskId)].ui32BaseAddr),
                    &ui32PhysAddr);
                sCommand.uCommand.sStartTask.ui32DramDeadlineConfigStructAddr = ui32PhysAddr;

                
                BRAP_P_ConvertAddressToOffset(hRap->hHeap, 
                    (void *)(hRap->sMemInfo.sTskMemInfo[hDsp->uiDspIndex].sCitStruct[BRAP_GET_TASK_INDEX(hFwTaskCreate->uiTaskId)].ui32BaseAddr),
                    &ui32PhysAddr);
                sCommand.uCommand.sStartTask.ui32DramTaskConfigAddr = ui32PhysAddr;
                sCommand.uCommand.sStartTask.ui32SyncQueueId = hFwTaskCreate->hSyncMsgQueue->ui32FifoId;
                sCommand.uCommand.sStartTask.ui32AsyncQueueId = hFwTaskCreate->hAsyncMsgQueue->ui32FifoId;
        
                BRAP_P_ConvertAddressToOffset(hRap->hHeap, 
                    (void *)(hFwTaskCreate->sGfxCitOutput.sStackSwapBuff.ui32DramBufferAddress),
                    &ui32PhysAddr);
                sCommand.uCommand.sStartTask.sDramStackBuffer.ui32DramBufferAddress = ui32PhysAddr;
                sCommand.uCommand.sStartTask.sDramStackBuffer.ui32BufferSizeInBytes = hFwTaskCreate->sGfxCitOutput.sStackSwapBuff.ui32BufferSizeInBytes;                 

                BRAP_P_ConvertAddressToOffset(hRap->hHeap, 
                    (void *)(hRap->sMit.sAlgoDwnldDetail[BRAP_AF_P_AlgoId_eSystemGraphicTask].ui32AlgoCodePtr),
                    &ui32PhysAddr);
                
                sCommand.uCommand.sStartTask.sDramTaskCodeBuffer.ui32DramBufferAddress = ui32PhysAddr;
                sCommand.uCommand.sStartTask.sDramTaskCodeBuffer.ui32BufferSizeInBytes= hRap->sMit.sAlgoDwnldDetail[BRAP_AF_P_AlgoId_eSystemGraphicTask].ui32AlgoCodeSize;                
                
                /* Enable required asynchronous event notification */        
                hFwTaskCreate->hDsp->ui32EventIdMatrix[BRAP_GET_TASK_INDEX(hFwTaskCreate->uiTaskId)] |= BRAP_FWIF_P_DEFAULT_EVENTS_ENABLE_MASK;
                if(hDspCh->bChSpecificDspCh == true)
                {
                    hFwTaskCreate->hDsp->ui32EventIdMatrix[BRAP_GET_TASK_INDEX(hFwTaskCreate->uiTaskId)] |= hDspCh->uHandle.hRapCh->ui32AsynIntMask;
                }
                hFwTaskCreate->hDsp->ui32EventIdMatrix[BRAP_GET_TASK_INDEX(hFwTaskCreate->uiTaskId)] |= (
                                               BRAP_FWIF_P_EventIdMask_eGFXCompletionCmd);                    
        
                sCommand.uCommand.sStartTask.ui32EventEnableMask = hFwTaskCreate->hDsp->ui32EventIdMatrix[BRAP_GET_TASK_INDEX(hFwTaskCreate->uiTaskId)];
                BDBG_MSG(("hFwTaskCreate->hDsp->ui32EventIdMatrix > %x", hFwTaskCreate->hDsp->ui32EventIdMatrix[BRAP_GET_TASK_INDEX(hFwTaskCreate->uiTaskId )]));



                pTaskParams = (BRAP_FWIF_P_TaskParamInfo *)(hRap->sMemInfo.sTskMemInfo[hDsp->uiDspIndex].sTskInfo[BRAP_GET_TASK_INDEX(hFwTaskCreate->uiTaskId)].ui32BaseAddr);

                pTaskParams->sNonRealTimeTaskParams.ui32MaxTimeSlice = 1350;
                pTaskParams->sNonRealTimeTaskParams.ui32MinTimeThreshold = 45;                

                
        
                hFwTaskCreate->uiLastEventType = sCommand.sCommandHeader.ui32CommandID;     
#if 1
                BRAP_P_EventReset(hDsp,BRAP_GET_TASK_INDEX(hFwTaskCreate->uiTaskId));
                /* End Fill up start task parameters */
        
                err = BRAP_FWIF_P_SendCommand(hDsp->hCmdQueue, &sCommand,hRap,hFwTaskCreate);
                /*Accept the other Commands , After posting Start task Command */
        
                
                if(BERR_SUCCESS != err)
                {
                    if(hRap->bWatchdogTriggered == false)
                    {        
                        BDBG_ERR(("BRAP_GFX_DSPCHN_P_StartGfx: START_TASK creation failed!"));
                        err = BERR_TRACE(err);
                        BDBG_ASSERT(0);
                        goto error;
                    }
                    else
                        err = BERR_SUCCESS;              
                }
                /* Wait for Ack_Response_Received event w/ timeout */
                err = BRAP_P_EventWait(hDsp, BRAP_DSPCHN_P_START_STOP_EVENT_TIMEOUT_IN_MS,BRAP_GET_TASK_INDEX(hFwTaskCreate->uiTaskId));
                if(BERR_TIMEOUT == err)
                {
                    if((hRap->bWatchdogTriggered == false))
                    {
                        /* Please note that, If the code reaches at this point then there is a potential Bug in Fw 
                        code which needs to be debugged. However Watchdog is being triggered to recover the system*/            
                        BDBG_WRN(("BRAP_GFX_DSPCHN_P_StartGfx: START_TASK ACK timeout! Triggering Watchdog"));

                        BRAP_Write32(hDsp->hRegister, BCHP_AUD_DSP_INTH0_R5F_SET+ hDsp->ui32Offset,0x1);
                        hRap->bWatchdogTriggered  = true;
                        err = BERR_SUCCESS;              
                    }
                    else
                        err = BERR_SUCCESS;              
                }
        /* Send command for the task , Only if the ack for the Start of the task is recieved */
                hFwTaskCreate->bStopped = false;            
        
        
                eMsgType = BRAP_P_MsgType_eSyn;
                
                if(hRap->bWatchdogTriggered == false)
                {
                    err = BRAP_FWIF_P_GetMsg(hFwTaskCreate->hSyncMsgQueue, (void *)&sRsp,eMsgType);
                }
        
                if(BERR_SUCCESS != err)
                {
                    if(hRap->bWatchdogTriggered == false)
                    {
                        BDBG_ERR(("BRAP_GFX_DSPCHN_P_StartGfx: Unable to read ACK!"));
                        err = BERR_TRACE(err);
                        goto error;
                    }
                    else
                        err = BERR_SUCCESS;              
                }
                if((sRsp.sCommonAckResponseHeader.eStatus != BERR_SUCCESS)||
                    (sRsp.sCommonAckResponseHeader.ui32ResponseID != BRAP_FWIF_P_START_TASK_RESPONSE_ID)||
                    (sRsp.sCommonAckResponseHeader.ui32TaskID != hFwTaskCreate->uiTaskId))
                {
                    if((hRap->bWatchdogTriggered == false))
                    {
                       BDBG_ERR(("BRAP_GFX_DSPCHN_P_StartGfx: START_TASK ACK not received successfully!eStatus = %d , ui32ResponseID = %d , ui32TaskID %d ",
                            sRsp.sCommonAckResponseHeader.eStatus,sRsp.sCommonAckResponseHeader.ui32ResponseID,sRsp.sCommonAckResponseHeader.ui32TaskID));
                        err = BERR_TRACE(sRsp.sCommonAckResponseHeader.eStatus);
                        goto error;
                    }
                    else
                        err = BERR_SUCCESS;              
                }      
#endif        
                }
        error:     
            if(false == bWdgRecovery)
            {
                hDspCh->eChState = BRAP_DSPCHN_P_ChState_eStart;     
            }
                
           
            if(psCitInputStruct)
            BKNI_Free(psCitInputStruct);
            if(psAudProcNetwork)
            BKNI_Free(psAudProcNetwork);
            
            BDBG_LEAVE( BRAP_GFX_DSPCHN_P_StartGfx );
            return err;    
}

/***************************************************************************
Summary: Stop a DSP context for Gfx

Description:
    This function is responsible for starting a DSP context for the given
    input decode mode.
    
Returns:
    BERR_SUCCESS - If successful

See Also:
    BRAP_DSPCHN_P_Stop
**************************************************************************/
BERR_Code
BRAP_GFX_DSPCHN_P_StopGfx(
    BRAP_DSPCHN_Handle  pDspChn               /* [in] DSPCHN handle */
    )

{
    BERR_Code err = BERR_SUCCESS;
    BRAP_FWIF_P_FwTaskHandle hFwTask = NULL;
    BRAP_DSP_Handle hDsp; 
    BRAP_FWIF_P_Command sCommand;
    BRAP_Handle         hRap;
    int         i=0,j=0;
    BRAP_P_MsgType  eMsgType;
    BRAP_FWIF_P_Response sRsp;
    unsigned int uiPth = 0, uiPathIndx = 0;
    bool bFoundPath = false;
    BRAP_RM_P_ResrcGrant *psRsrcGrnt = NULL;    

    BDBG_ENTER(BRAP_GFX_DSPCHN_P_StopGfx);

    /* Assert on the arguments */
    BDBG_ASSERT(pDspChn);

    if(pDspChn->eChState == BRAP_DSPCHN_P_ChState_eStop)
    {
        BDBG_MSG(("BRAP_GFX_DSPCHN_P_StopGfx: hDspChn 0x%x is already stoped", pDspChn));
        return err;
    }
       
    BKNI_Memset((void *)&sRsp,0,sizeof(BRAP_FWIF_P_Response));
       
    if(true == pDspChn->bChSpecificDspCh)
    {
        hRap = pDspChn->uHandle.hRapCh->hRap;
    }
    else
    {
        hRap = pDspChn->uHandle.hAssociation->hRap;
    }
    
    BRAP_DSPCHN_P_FreeFwCtxtBuf(hRap, BRAP_DSPCHN_GfxType_eSoftGfx, BRAP_DSP_ExecType_eGfx);

    if(true == pDspChn->bChSpecificDspCh)
    {
        for(uiPth = 0; uiPth < BRAP_P_MAX_PATHS_IN_A_CHAN; uiPth++)
        {
            if((BRAP_P_CheckIfPtrInvalid((void *)pDspChn->uHandle.hRapCh->pPath[uiPth]) == false)
                && (pDspChn->uHandle.hRapCh->pPath[uiPth]->hDspCh == pDspChn))
                {
                    uiPathIndx = uiPth;
                    bFoundPath = true;                    
                    break;
                }
        }
        if(uiPth >= BRAP_P_MAX_PATHS_IN_A_CHAN)
        {
            BDBG_ERR(("No sPath allocated for the running DspCh"));
    		return BERR_TRACE(err); 
        }
    	psRsrcGrnt = ( BRAP_RM_P_ResrcGrant *) BKNI_Malloc( sizeof( BRAP_RM_P_ResrcGrant ));
    	if ( NULL==psRsrcGrnt )
    	{
    			return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    	}
	BKNI_Memset( psRsrcGrnt, 0, sizeof(BRAP_RM_P_ResrcGrant) ); 
    
        BRAP_RM_P_InitResourceGrant(psRsrcGrnt,true);
        psRsrcGrnt->uiDspId = pDspChn->uHandle.hRapCh->pPath[uiPathIndx]->sRsrcGrnt.uiDspId;
        psRsrcGrnt->uiDspContextId = pDspChn->uHandle.hRapCh->pPath[uiPathIndx]->sRsrcGrnt.uiDspContextId;
        err = BRAP_RM_P_FreeResources(hRap->hRm, psRsrcGrnt,true);
        if(BERR_SUCCESS != err)
        {
            BDBG_ERR(("Unable to free allocated DSP"));
            BKNI_Free(psRsrcGrnt);
    		return BERR_TRACE(err); 
        }
        pDspChn->uHandle.hRapCh->pPath[uiPathIndx]->sRsrcGrnt.uiDspId = BRAP_RM_P_INVALID_INDEX;
        pDspChn->uHandle.hRapCh->pPath[uiPathIndx]->sRsrcGrnt.uiDspContextId = BRAP_RM_P_INVALID_INDEX;
        BKNI_Free(psRsrcGrnt);
    }


    for(i=0; i<BRAP_FWIF_P_MAX_FW_TASK_PER_DSPCHN; i++)
    {
        hFwTask = pDspChn->sFwTaskInfo[i].hFwTask;

        if(BRAP_P_CheckIfPtrInvalid((void *)hFwTask) == true)
            continue;
        if(hFwTask->bStopped == true)
        {
            BDBG_MSG(("Task Already stopped"));
            continue;
        }
        hDsp = hFwTask->hDsp;

        /*Prepare command to stop the task */
        sCommand.sCommandHeader.ui32CommandID = BRAP_FWIF_P_STOP_TASK_COMMAND_ID;
        sCommand.sCommandHeader.ui32CommandCounter = hFwTask->uiCommandCounter;
        sCommand.sCommandHeader.ui32TaskID = hFwTask->uiTaskId;
        sCommand.sCommandHeader.eResponseType = BRAP_FWIF_P_ResponseType_eResponseRequired;

        sCommand.uCommand.sStopTask.ui32MasterTaskId = BRAP_FWIF_P_INVALID_TSK_ID;
        sCommand.uCommand.sStopTask.eTaskType = BRAP_FWIF_P_TaskType_eNonRealtime;        

        hFwTask->uiLastEventType = sCommand.sCommandHeader.ui32CommandID;
        BRAP_P_EventReset(hDsp,BRAP_GET_TASK_INDEX(hFwTask->uiTaskId));
        err = BRAP_FWIF_P_SendCommand(hDsp->hCmdQueue, &sCommand,hRap,hFwTask);            
        if(BERR_SUCCESS != err)
        {
            if(hRap->bWatchdogTriggered == false)
            {        
                BDBG_ERR(("BRAP_GFX_DSPCHN_P_StopGfx: STOP_TASK creation failed!"));

                err = BERR_TRACE(err);            
                goto freetask;    
                   
             }
            else
                err = BERR_SUCCESS;   
        }

        /* Wait for Ack_Response_Received event w/ timeout */
        err = BRAP_P_EventWait(hDsp, BRAP_DSPCHN_P_START_STOP_EVENT_TIMEOUT_IN_MS,BRAP_GET_TASK_INDEX(hFwTask->uiTaskId));
        if(BERR_TIMEOUT == err)
        {
            if(hRap->bWatchdogTriggered == false)
            {        
                /* Please note that, If the code reaches at this point then there is a potential Bug in Fw 
                code which needs to be debugged. However Watchdog is being triggered to recover the system*/
                BDBG_WRN(("BRAP_GFX_DSPCHN_P_StopGfx: STOP_TASK Timeout! Triggering watchdog"));

                BRAP_Write32(hDsp->hRegister, BCHP_AUD_DSP_INTH0_R5F_SET+ hDsp->ui32Offset,0x1);
                hRap->bWatchdogTriggered  = true;
                err = BERR_SUCCESS;                     
             }
            else
                err = BERR_SUCCESS;             
        }
        
        eMsgType = BRAP_P_MsgType_eSyn;
        if(hRap->bWatchdogTriggered == false)
        {
            err = BRAP_FWIF_P_GetMsg(hFwTask->hSyncMsgQueue, (void *)&sRsp,eMsgType);
        }
        if(BERR_SUCCESS != err)
        {
            if(hRap->bWatchdogTriggered == false)
            {
                BDBG_ERR(("BRAP_GFX_DSPCHN_P_StopGfx: Unable to read ACK!"));
                err = BERR_TRACE(err);            
                goto freetask;                     
            }
            else
                err = BERR_SUCCESS;             
        }
        if((sRsp.sCommonAckResponseHeader.eStatus != BERR_SUCCESS)||
            (sRsp.sCommonAckResponseHeader.ui32ResponseID != BRAP_FWIF_P_STOP_TASK_RESPONSE_ID)||
            (sRsp.sCommonAckResponseHeader.ui32TaskID != hFwTask->uiTaskId))
        {
            if(hRap->bWatchdogTriggered == false)
            {
                BDBG_ERR(("BRAP_GFX_DSPCHN_P_StopGfx: STOP_TASK ACK not received successfully!"));
                err = BERR_TRACE(err);            
                goto freetask;                     
    
            }
            else
                err = BERR_SUCCESS;             
        }
        
        /*Mask the task */
        err = BRAP_P_MaskTask(hFwTask);
        if(BERR_SUCCESS != err)
        {
            BDBG_ERR(("BRAP_GFX_DSPCHN_P_StopGfx: Unable to mask task id interrupt!"));
            err = BERR_TRACE(err);            
            goto freetask;
        }
        /* Destroy synchronous and asynchronous queue of Task */
        hDsp->bFifoUsed[hFwTask->hSyncMsgQueue->ui32FifoId] = false;
        err = BRAP_FWIF_P_DestroyMsgQueue(hFwTask->hSyncMsgQueue,hDsp);
        if(BERR_SUCCESS != err)
        {
            BDBG_ERR(("BRAP_GFX_DSPCHN_P_StopGfx: SYNC queue destroy failed!"));
            err = BERR_TRACE(err);            
            goto freetask;            
        }

        hDsp->bFifoUsed[hFwTask->hAsyncMsgQueue->ui32FifoId] = false;
        err = BRAP_FWIF_P_DestroyMsgQueue(hFwTask->hAsyncMsgQueue,hDsp);
        if(BERR_SUCCESS != err)
        {
            BDBG_ERR(("BRAP_GFX_DSPCHN_P_StopGfx: ASYNC queue destroy failed!"));
            err = BERR_TRACE(err);
            goto freetask;            
        }
freetask:
        for(j=0;j<BRAP_MAX_FW_TSK_SUPPORTED;j++)
        {
            if(hRap->sMemInfo.sTskMemInfo[hDsp->uiDspIndex].sTskQueue[j].ui32TaskId == hFwTask->uiTaskId)
            {
                hRap->sMemInfo.sTskMemInfo[hDsp->uiDspIndex].sTskQueue[j].ui32TaskId = BRAP_FWIF_P_INVALID_TSK_ID;
                hFwTask->pAsyncMsgMemory = NULL;
                hFwTask->uiTaskId = BRAP_FWIF_P_INVALID_TSK_ID;
                hFwTask->bStopped = true;
                hFwTask->uiCommandCounter= 0;
           }
        }
        if(BERR_SUCCESS != err)
        {
            return err;
        }
    }

    
    pDspChn->eChState = BRAP_DSPCHN_P_ChState_eStop; 
    
    BDBG_LEAVE(BRAP_GFX_DSPCHN_P_StopGfx);
    return err;
}    

BERR_Code BRAP_P_StopGfxPath ( 
    BRAP_ChannelHandle          hRapCh,        /* [in] RAP Channel handle */
    const unsigned int          uiPth       	/* [in] Path idx corresponding 
                                                   to pPath[] array element */       
    )
{
        BERR_Code ret = BERR_SUCCESS;
    
        BDBG_ENTER (BRAP_P_StopGfxPath);
        BDBG_ASSERT(hRapCh);
        if(NULL == hRapCh->pPath[uiPth])
        {
            ret = BERR_TRACE(BERR_NOT_INITIALIZED);
            goto exit;
        }      
    
        /* Error state check */
        if (0 == hRapCh->pPath[uiPth]->uiPthStartCnt)
        { 
            /* This should never happen. If it does, it means the system
               has gone into an invalid state!!!!*/
            BDBG_ERR (("BRAP_P_StopGfxPath: Gfx Path start count is already 0"));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
        }
    
        /* Decrement the start count */
        hRapCh->pPath[uiPth]->uiPthStartCnt--;
    
        BDBG_MSG(("BRAP_P_StopGfxPath: For uiPth=%d New uiPthStartCnt = %d", 
            uiPth, hRapCh->pPath[uiPth]->uiPthStartCnt));
    
        /* If the new start count is not 0, do not close the resources instead 
           return success */
        if(0 != hRapCh->pPath[uiPth]->uiPthStartCnt)
        {
            BDBG_MSG(("BRAP_P_StopGfxPath: uiPthStartCnt = %d so not closing resources!"));
            BDBG_LEAVE(BRAP_P_StopGfxPath);
            return BERR_SUCCESS;
        }
        else
        {
             hRapCh->pPath[uiPth]->uiPthStartCnt++;
        }
    
        /* Stop the DSP channel, if any in this path */
        if(NULL != hRapCh->pPath[uiPth]->hDspCh)
        {  
            BDBG_MSG(("Calling BRAP_GFX_DSPCHN_P_StartGfx"));
            ret = BRAP_GFX_DSPCHN_P_StopGfx(hRapCh->pPath[uiPth]->hDspCh);
            if(BERR_SUCCESS != ret)
            {
                BDBG_ERR(("BRAP_P_StopGfxPath: BRAP_GFX_DSPCHN_P_StartGfx failed %d",ret));
                return BERR_TRACE(ret); 
            }
        }
        else
            BDBG_MSG(("Did **NOT** Call BRAP_GFX_DSPCHN_P_StartGfx"));
    
        /* Decrement the start count */
        hRapCh->pPath[uiPth]->uiPthStartCnt--;
        BDBG_LEAVE(BRAP_P_StopGfxPath);
    
    exit:        
        return ret;
    }

    BERR_Code BRAP_P_CloseGfxPath( 
        BRAP_ChannelHandle          hRapCh         /* [in] RAP Channel handle */
        )
    {
        BERR_Code ret = BERR_SUCCESS;
        unsigned int  uiPth = 0;    
    
        BDBG_ENTER(BRAP_P_CloseGfxPath);
    
        /* Validate parameters */
        BDBG_ASSERT(hRapCh);
   
        hRapCh->pPath[uiPth]->uiPthOpenCnt--;            
        hRapCh->sChAudProcessingStage[0][0].bDecoderStage = false;
        if(false == hRapCh->bInternalCallFromRap)
        {    
            ret = BRAP_P_UngetChannelPaths(hRapCh);
            if(ret != BERR_SUCCESS) 
            {
                BDBG_ERR(("BRAP_P_CloseGfxPath: UngetChannelPaths returned %d",ret));
                ret = BERR_TRACE(ret);
                goto end;
            }
        }
    
    end:
    
    BDBG_LEAVE(BRAP_P_CloseGfxPath);
    return ret;    
    }

    /**************************************************************************
Summary:
    Private function that gets the current channel settings.
**************************************************************************/
BERR_Code
BRAP_GFX_P_GetCurrentChannelSettings (
	BRAP_ChannelHandle	    hRapCh,         /* [in] Decode channel handle */
	BRAP_GFX_OpenSettings    *pChanSettings  /* [out] Current channel settings */
	)
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER(BRAP_GFX_P_GetCurrentChannelSettings);
    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(pChanSettings);
    BSTD_UNUSED(pChanSettings);
    *pChanSettings = hRapCh->sGfxSettings.sOpenGfxSettings;
    BDBG_LEAVE(BRAP_GFX_P_GetCurrentChannelSettings);
    return ret;
}

BERR_Code BRAP_GFX_P_InitInterframeBuffer(
				BRAP_FWIF_P_FwTaskHandle	hTask)
{
	BRAP_CIT_P_GfxCITOutput	*pCitOutput = NULL;
	BRAP_CIT_P_FwBufInfo *pOpNodeInfo = NULL;
	BERR_Code	rc = BERR_SUCCESS;
	unsigned int i,j,k;
	const BIMG_Interface *iface = NULL;	
	void *pImgContext = NULL;
	uint32_t	ui32ImgId = BRAP_MAX_IMG_ID,ui32physAddress=0;
	BRAP_Handle	hRap = NULL;

	BDBG_ENTER(BRAP_GFX_P_InitInterframeBuffer);
	BDBG_ASSERT(hTask);

    if(true == hTask->bChSpecificTask)
    {
        hRap = hTask->uHandle.hRapCh->hRap;
    }
    else
    {
        hRap = hTask->uHandle.hAssociation->hRap;
    }
	iface = hRap->sSettings.pImgInterface;	
	pImgContext = hRap->sSettings.pImgContext;
	pCitOutput = &(hTask->sGfxCitOutput);
	for(i = 0;i < pCitOutput->ui32NumBranches; i++)
	{
		for(j = 0; j < pCitOutput->sCitBranchInfo[i].ui32NumStages; j++)
		{
			for(k = 0; k < pCitOutput->sCitBranchInfo[i].sCitStgInfo[j].ui32NumNodes; k++)
			{
				if(pCitOutput->sCitBranchInfo[i].sCitStgInfo[j].sFwOpNodeInfo[k].eFwExecId != BRAP_AF_P_AlgoId_eMax)
				{			
					pOpNodeInfo = &(pCitOutput->sCitBranchInfo[i].sCitStgInfo[j].sFwOpNodeInfo[k]);

					/* check if enough memory is allocated for interframe buffer */
					if(pOpNodeInfo->ui32InterframeBufSize < hRap->sImgIdMappingArrays.ui32InterframeSize[pOpNodeInfo->eFwExecId])
					{
						BDBG_ERR(("Allocated memory (%d) for interframe buffer is less than required (%d) for ExecId =%#x",
							pOpNodeInfo->ui32InterframeBufSize ,
							hRap->sImgIdMappingArrays.ui32InterframeSize[pOpNodeInfo->eFwExecId],
							pOpNodeInfo->eFwExecId));
					}
					else
					{
                                    	if(pOpNodeInfo->eFwExecId < BRAP_AF_P_AlgoId_eMax)
                                    	{
                                    		ui32ImgId = hRap->sImgIdMappingArrays.ui32InterframeImgId[pOpNodeInfo->eFwExecId];
                                    	}      
                                    	else
                                    	{
                                    		BDBG_ERR(("Invalid FW exec ID"));
                                    		return BERR_INVALID_PARAMETER;
                                    	}                                        

						BRAP_P_ConvertAddressToOffset(hRap->hHeap, (void *)(pOpNodeInfo->ui32InterframeBufAdr), &ui32physAddress);	
						BDBG_MSG(("pOpNodeInfo->ui32InterframeBufAdr %#x physical %#x",pOpNodeInfo->ui32InterframeBufAdr,ui32physAddress));

					
					if(BRAP_MAX_IMG_ID == ui32ImgId)
					{
						BDBG_MSG(("No FW Image file exists for Exec Id %#x",pOpNodeInfo->eFwExecId));
					}
					else
					{
				        BDBG_MSG(("pOpNodeInfo->eFwExecId > %d, ui32ImgId > %d", pOpNodeInfo->eFwExecId, ui32ImgId));
						BRAP_FWDWNLD_P_CopyFWImageToMem(iface,pImgContext,pOpNodeInfo->ui32InterframeBufAdr,ui32ImgId,hRap->hHeap);
					}
				}
				}
				else
					break;
				
			}
		}
	}

	
	BDBG_LEAVE(BRAP_GFX_P_InitInterframeBuffer);
	return rc;

}

BERR_Code   BRAP_GFX_P_ConvertSurface(
        BRAP_Handle    hRap,
                BSUR_Surface_Handle  hSurfaceHandle,
                BRAP_GFX_P_SurfaceStruct    *psSurfaceFwFormat)
{
    BERR_Code ret = BERR_SUCCESS;
    BPXL_Format     ePixelFormat;
    BSUR_Palette_Handle  hPalette=NULL;
    unsigned int uiTemp;
    void *pTemp=NULL;
    BDBG_ENTER(BRAP_GFX_P_ConvertSurface);
    BDBG_ASSERT(psSurfaceFwFormat);
    BSTD_UNUSED(uiTemp);
    BSTD_UNUSED(hRap);

    if(hSurfaceHandle == NULL)
    {
        BDBG_ERR(("ERROR!!! Surface handle cannot be NULL."));
        return BERR_INVALID_PARAMETER;
    }
    ret =BSUR_Surface_GetAddress(hSurfaceHandle,
                                                (void **)&(pTemp),
                                                &(psSurfaceFwFormat->sSurfaceDimension.pitch));
    if(BERR_SUCCESS != ret)
    {
        BDBG_ERR(("Error in getting surface address."));
            ret =BERR_TRACE(ret);
            goto end;
    }    

    BDBG_MSG(("Surface Virtual Address =%x",pTemp));


    ret =BSUR_Surface_GetOffset(hSurfaceHandle,&(psSurfaceFwFormat->sDramSurfaceBuffer.ui32DramBufferAddress));
    if(BERR_SUCCESS != ret)
    {
        BDBG_ERR(("Error in getting surface offset."));
            ret =BERR_TRACE(ret);
            goto end;
    }    

    ret = BSUR_Surface_GetDimensions(hSurfaceHandle,
                                                        &(psSurfaceFwFormat->sSurfaceDimension.blockSize.width),
                                                        &(psSurfaceFwFormat->sSurfaceDimension.blockSize.height));
    if(BERR_SUCCESS != ret)
    {
        BDBG_ERR(("Error in getting surface dimension"));
            ret =BERR_TRACE(ret);
            goto end;
    }    

    ret =BSUR_Surface_GetFormat(hSurfaceHandle,&ePixelFormat);
    if(BERR_SUCCESS != ret)
    {
        BDBG_ERR(("Error in getting surface pixel format"));
            ret =BERR_TRACE(ret);
            goto end;
    }    
    
    ret =BRAP_GFX_P_ConvertPixelFormat(ePixelFormat,
                                                                    &(psSurfaceFwFormat->ePixelFormat),
                                                                    &(psSurfaceFwFormat->ui32BitsPerPixel));
    if(BERR_SUCCESS != ret)
    {
        BDBG_ERR(("Error in Mapping Pixel format."));
        ret =BERR_TRACE(ret);
        goto end;
    }      

    psSurfaceFwFormat->sDramSurfaceBuffer.ui32BufferSizeInBytes =
            psSurfaceFwFormat->sSurfaceDimension.blockSize.width * 
            psSurfaceFwFormat->sSurfaceDimension.blockSize.height
            *4;    


    if(psSurfaceFwFormat->ePixelFormat == BRAP_GFX_P_PixelFormat_ePalette8)
    {
        ret = BSUR_Surface_GetPalette(hSurfaceHandle,&hPalette);
        if(BERR_SUCCESS != ret)
        {
            BDBG_ERR(("Error in getting Palette Handle"));
                ret =BERR_TRACE(ret);
                goto end;
        }    

        ret = BSUR_Palette_GetOffset(hPalette,&(psSurfaceFwFormat->sPalette.sDramPaletteBuffer.ui32DramBufferAddress));
        if(BERR_SUCCESS != ret)
        {
            BDBG_ERR(("Error in getting palette offset"));
                ret =BERR_TRACE(ret);
                goto end;
        }    
        
        ret =BSUR_Palette_GetFormat(hPalette,&ePixelFormat);
        if(BERR_SUCCESS != ret)
        {
            BDBG_ERR(("Error in getting palette pixel format"));
                ret =BERR_TRACE(ret);
                goto end;
        }    
        
        ret = BRAP_GFX_P_ConvertPixelFormat(ePixelFormat,&(psSurfaceFwFormat->sPalette.ePalettePixelFormat),&uiTemp);
        if(BERR_SUCCESS != ret)
        {
            BDBG_ERR(("Error in Mapping Palette Pixel format."));
                ret =BERR_TRACE(ret);
                goto end;
        }

        ret = BSUR_Palette_GetNumEntries(hPalette,&(psSurfaceFwFormat->sPalette.ui32NumEntries));
        if(BERR_SUCCESS != ret)
        {
            BDBG_ERR(("Error in Getting Num entries"));
                ret =BERR_TRACE(ret);
                goto end;
        }
    }

end:
    BDBG_LEAVE(BRAP_GFX_P_ConvertSurface);
        return ret;
    }


    BERR_Code
    BRAP_GFX_P_MapFillSettings (
        BRAP_Handle    hRap,
        BRAP_ChannelHandle  hRapCh,
        const BRAP_GFX_FillSettings                   *psFillSetting, /*[in] Fill Settings*/
        BRAP_GFX_P_Fill    *psFillSettingsInFwFormat  /* [out] Current channel settings */
        )
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER(BRAP_GFX_P_MapFillSettings);
    BDBG_ASSERT(psFillSetting);
    BDBG_ASSERT(psFillSettingsInFwFormat);
    BDBG_ASSERT(hRap);
    BDBG_ASSERT(hRapCh);        

    ret = BRAP_GFX_P_ConvertSurface(hRap,psFillSetting->hOutputSurface,&(psFillSettingsInFwFormat->sOutSurface));
    if(BERR_SUCCESS != ret)
    {
        BDBG_ERR(("Error in Mapping Surface."));
        ret =BERR_TRACE(ret);
        goto end;
    }       

    BDBG_MSG(("sOutSurface.sDramSurfaceBuffer.ui32DramBufferAddress Physical addr= %x",psFillSettingsInFwFormat->sOutSurface.sDramSurfaceBuffer.ui32DramBufferAddress));        

    ret = BRAP_GFX_P_NormalizeRectangle(&(psFillSettingsInFwFormat->sOutSurface),&(psFillSetting->sFillRect),&(psFillSettingsInFwFormat->sFillRect));
    if(BERR_SUCCESS != ret)
    {
        BDBG_ERR(("Invalid Dimensions of Fill Rect"));
        ret =BERR_TRACE(ret);
        goto end;
    }       

    psFillSettingsInFwFormat->eAplhaOperation = psFillSetting->eAlphaOperation;
    psFillSettingsInFwFormat->eColorOperation= psFillSetting->eColorOperation;
    psFillSettingsInFwFormat->ui32FillColor= psFillSetting->uiFillColor;        
    
end:
    BDBG_LEAVE(BRAP_GFX_P_MapFillSettings);        
    return ret;
    } 
        

    BERR_Code
    BRAP_GFX_P_MapBlitSettings (
        BRAP_Handle    hRap,
        BRAP_ChannelHandle  hRapCh,
        const BRAP_GFX_BlitSettings                   *psBlitSetting, /*[in] Fill Settings*/
        BRAP_GFX_P_Blit    *psBlitSettingsInFwFormat  /* [out] Current channel settings */
        )
{
    BERR_Code ret = BERR_SUCCESS;
    unsigned int i=0;
    BDBG_ENTER(BRAP_GFX_P_MapBlitSettings);
    BDBG_ASSERT(psBlitSetting);
    BDBG_ASSERT(psBlitSettingsInFwFormat);
    BDBG_ASSERT(hRap);
    BDBG_ASSERT(hRapCh);        



    ret = BRAP_GFX_P_ConvertSurface(hRap,psBlitSetting->hSourceSurface,&(psBlitSettingsInFwFormat->sSrcSurface));
    if(BERR_SUCCESS != ret)
    {
        BDBG_ERR(("Error in Mapping SRC Surface."));
        ret =BERR_TRACE(ret);
        goto end;
    }         

    BDBG_MSG(("sSrcSurface.sDramSurfaceBuffer.ui32DramBufferAddress Physical addr= %x",psBlitSettingsInFwFormat->sSrcSurface.sDramSurfaceBuffer.ui32DramBufferAddress));        
    
    ret = BRAP_GFX_P_NormalizeRectangle(&(psBlitSettingsInFwFormat->sSrcSurface),&(psBlitSetting->sSourceRect),&(psBlitSettingsInFwFormat->sSrcRect));
    if(BERR_SUCCESS != ret)
    {
        BDBG_ERR(("Invalid Dimensions of Source Rect"));
        ret =BERR_TRACE(ret);
        goto end;
    }       

    if(!((psBlitSetting->eColorOperation == BRAP_GFX_eBlitColorOperation_eCopySource)&& 
           ((psBlitSetting->eAlphaOperation == BRAP_GFX_eBlitAlphaOperation_eCopySource) ||
            (psBlitSetting->eAlphaOperation == BRAP_GFX_eBlitAlphaOperation_eCopyConstant))))
    {/*If it is not a Simple Blit OP*/
        ret = BRAP_GFX_P_ConvertSurface(hRap,psBlitSetting->hDestinationSurface,&(psBlitSettingsInFwFormat->sDstSurface));
        if(BERR_SUCCESS != ret)
        {
            BDBG_ERR(("Error in Mapping DST Surface."));
            ret =BERR_TRACE(ret);
            goto end;
        }       

        BDBG_MSG(("sDstSurface.sDramSurfaceBuffer.ui32DramBufferAddress Physical addr= %x",psBlitSettingsInFwFormat->sDstSurface.sDramSurfaceBuffer.ui32DramBufferAddress));        

        
        ret = BRAP_GFX_P_NormalizeRectangle(&(psBlitSettingsInFwFormat->sDstSurface),&(psBlitSetting->sDestinationRect),&(psBlitSettingsInFwFormat->sDstRect));
        if(BERR_SUCCESS != ret)
        {
            BDBG_ERR(("Invalid Dimensions of Dest Rect"));
            ret =BERR_TRACE(ret);
            goto end;
        }     
    }

    ret = BRAP_GFX_P_ConvertSurface(hRap,psBlitSetting->hOutputSurface,&(psBlitSettingsInFwFormat->sOutSurface));
    if(BERR_SUCCESS != ret)
    {
        BDBG_ERR(("Error in Mapping OUT Surface."));
        ret =BERR_TRACE(ret);
        goto end;
    }  
    
    BDBG_MSG(("sOutSurface.sDramSurfaceBuffer.ui32DramBufferAddress Physical addr= %x",psBlitSettingsInFwFormat->sOutSurface.sDramSurfaceBuffer.ui32DramBufferAddress));        

    ret = BRAP_GFX_P_NormalizeRectangle(&(psBlitSettingsInFwFormat->sOutSurface),&(psBlitSetting->sOutputRect),&(psBlitSettingsInFwFormat->sOutRect));
    if(BERR_SUCCESS != ret)
    {
        BDBG_ERR(("Invalid Dimensions of Out Rect"));
        ret =BERR_TRACE(ret);
        goto end;
    } 



    for(i = 0 ; i < BRAP_GFX_COLOR_MATRIX_COEFF_COUNT ; i++)
        psBlitSettingsInFwFormat->sCSCMatrix.coeffMatrix[i] = psBlitSetting->sCSCMatrix.iCoeffMatrix[i]; 
    
    psBlitSettingsInFwFormat->sCSCMatrix.ui32Shift  = psBlitSetting->sCSCMatrix.uiShift;         
    
    psBlitSettingsInFwFormat->eColorOperation = psBlitSetting->eColorOperation; 
    psBlitSettingsInFwFormat->eAlphaOperation = psBlitSetting->eAlphaOperation; 
    psBlitSettingsInFwFormat->constAlphaColorOp = psBlitSetting->uiConstAlphaColorOp; 
    psBlitSettingsInFwFormat->constAlphaAlphaOp = psBlitSetting->uiConstAlphaAlphaOp; 
    psBlitSettingsInFwFormat->eApplyAFF = psBlitSetting->bEnableAntiFlutterFilter; 
    psBlitSettingsInFwFormat->eApplyCSC = psBlitSetting->bEnableCSC; 

    psBlitSettingsInFwFormat->eScalingFilter = psBlitSetting->eScalingFilter;     

    /*Map Color Key parameters*/
    psBlitSettingsInFwFormat->sGfxColorKeyParams.sSource.ui32ColorKeyEnabled = psBlitSetting->sColorKeyParams.sSource.bEnabled;
    psBlitSettingsInFwFormat->sGfxColorKeyParams.sSource.ui32ColorKeyLower = psBlitSetting->sColorKeyParams.sSource.uiLower;
    psBlitSettingsInFwFormat->sGfxColorKeyParams.sSource.ui32ColorKeyUpper = psBlitSetting->sColorKeyParams.sSource.uiUpper;
    psBlitSettingsInFwFormat->sGfxColorKeyParams.sSource.ui32ColorKeyMask = psBlitSetting->sColorKeyParams.sSource.uiMask;
    psBlitSettingsInFwFormat->sGfxColorKeyParams.sSource.ui32ColorKeyReplace = psBlitSetting->sColorKeyParams.sSource.uiReplace;
    psBlitSettingsInFwFormat->sGfxColorKeyParams.sSource.ui32ColorKeyRplMask = psBlitSetting->sColorKeyParams.sSource.uiReplaceMask;    


    psBlitSettingsInFwFormat->sGfxColorKeyParams.sDest.ui32ColorKeyEnabled = psBlitSetting->sColorKeyParams.sDest.bEnabled;
    psBlitSettingsInFwFormat->sGfxColorKeyParams.sDest.ui32ColorKeyLower = psBlitSetting->sColorKeyParams.sDest.uiLower;
    psBlitSettingsInFwFormat->sGfxColorKeyParams.sDest.ui32ColorKeyUpper = psBlitSetting->sColorKeyParams.sDest.uiUpper;
    psBlitSettingsInFwFormat->sGfxColorKeyParams.sDest.ui32ColorKeyMask = psBlitSetting->sColorKeyParams.sDest.uiMask;
    psBlitSettingsInFwFormat->sGfxColorKeyParams.sDest.ui32ColorKeyReplace = psBlitSetting->sColorKeyParams.sDest.uiReplace;
    psBlitSettingsInFwFormat->sGfxColorKeyParams.sDest.ui32ColorKeyRplMask = psBlitSetting->sColorKeyParams.sDest.uiReplaceMask;    
    

end:
    BDBG_LEAVE(BRAP_GFX_P_MapBlitSettings);
    return ret;
}


    


BERR_Code
BRAP_GFX_P_ConvertPixelFormat (
    BPXL_Format    ePixelAppFormat,/*[in]*/
    BRAP_GFX_P_ePixelFormatSupported    *ePixelFwFormat,/*[out]*/
    uint32_t            *ui32BitsPerPixel   /*[out]*/
    )
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER(BRAP_GFX_P_ConvertPixelFormat);

    switch(ePixelAppFormat)
    {
        case BPXL_eP8:
            *ePixelFwFormat = BRAP_GFX_P_PixelFormat_ePalette8;
            *ui32BitsPerPixel = 8;
            break;
        case BPXL_eA8_R8_G8_B8:
            *ePixelFwFormat = BRAP_GFX_P_PixelFormat_eA8_R8_G8_B8;
            *ui32BitsPerPixel = 32;                
            break;
        case BPXL_eR8_G8_B8_A8:
            *ePixelFwFormat = BRAP_GFX_P_PixelFormat_eR8_G8_B8_A8;
            *ui32BitsPerPixel = 32;                
            break;
        case BPXL_eR5_G6_B5:
            *ePixelFwFormat = BRAP_GFX_P_PixelFormat_eR5_G6_B5;
            *ui32BitsPerPixel = 16;                
            break;
        case BPXL_eA1_R5_G5_B5:
            *ePixelFwFormat = BRAP_GFX_P_PixelFormat_eA1_R5_G5_B5;
            *ui32BitsPerPixel = 16;                
            break;
        case BPXL_eR5_G5_B5_A1:
            *ePixelFwFormat = BRAP_GFX_P_PixelFormat_eR5_G5_B5_A1;
            *ui32BitsPerPixel = 16;                
            break;
        case BPXL_eA4_R4_G4_B4:
            *ePixelFwFormat = BRAP_GFX_P_PixelFormat_eA4_R4_G4_B4;
            *ui32BitsPerPixel = 16;                                
            break;
        case BPXL_eR4_G4_B4_A4:
            *ePixelFwFormat = BRAP_GFX_P_PixelFormat_eR4_G4_B4_A4;
            *ui32BitsPerPixel = 16;                                
            break;
        case BPXL_eA8_Y8_Cb8_Cr8:
        case BPXL_eY08_Cb8_Y18_Cr8:            
            *ePixelFwFormat = BRAP_GFX_P_PixelFormat_eA8_Y8_Cb8_Cr8;
            *ui32BitsPerPixel = 32;
            break;
        default:
            *ePixelFwFormat = BRAP_GFX_P_PixelFormat_eInvalid;                
            BDBG_ERR(("Pixel Format =%d Not supported",ePixelAppFormat));
            ret = BERR_NOT_SUPPORTED;
            goto end;
    }

end:             
    BDBG_LEAVE(BRAP_GFX_P_ConvertPixelFormat);
    return ret;
}    

    
    /***************************************************************************
    Summary:
            Generate the Gfx CIT input Structure
        
    Description:
            This function takes the audio processing network from channel handle and
            Generates the CIT input structure to be passed as Input to CIT module.
            Preprocessing is not supported.
    Returns:
        BERR_SUCCESS else error
    
    **************************************************************************/
    BERR_Code BRAP_FWIF_P_GenerateGfxCitInput(
        BRAP_Handle                         hRap,               /* [in] Rap Device Handle */
        BRAP_DSPCHN_Handle  hDspCh,                 /* [in] DSPCHN handle */
        BRAP_P_AudioProcNetwork             *psAudProcNetwork,  /* [in] Audio Processing Network */
        BRAP_CIT_P_InputInfo                *pCitInputStruct,    /* [out] CIT input structure */
        BRAP_FWIF_P_FwTaskHandle hFwTaskCreate     /*[in] Fw Task handle */
    )
{
        BERR_Code                       ret=BERR_SUCCESS;
        unsigned int                        ui32BranchId =0,ui32StageId =0;
        BRAP_ChannelHandle          hRapCh = NULL;
    
        BDBG_ENTER(BRAP_FWIF_P_GenerateGfxCitInput);
    
        BDBG_ASSERT(hRap);
        BDBG_ASSERT(pCitInputStruct);
        BDBG_ASSERT(psAudProcNetwork);
        
        BKNI_Memset(pCitInputStruct,0,sizeof(BRAP_CIT_P_InputInfo));
        pCitInputStruct->pBranchInfo[0] = (BRAP_CIT_P_FwBranchInfo *)BKNI_Malloc(sizeof(BRAP_CIT_P_FwBranchInfo));
        if (NULL==pCitInputStruct->pBranchInfo[0])
        {
            BDBG_ERR(("Out of System memory"));
            ret = BERR_OUT_OF_SYSTEM_MEMORY;
            goto error;
        }
        else
        {
            BKNI_Memset(pCitInputStruct->pBranchInfo[0],0,sizeof(BRAP_CIT_P_FwBranchInfo));
        }

    
        hRapCh = hDspCh->uHandle.hRapCh;        
        ui32StageId = 0;
        ui32BranchId=0;
        if(true == psAudProcNetwork->sAudProcessingStage[0][0].bDecoderStage)
        {
            BDBG_MSG(("This is a decoder stage"));
            pCitInputStruct->ui32NumBranches=1;        
            pCitInputStruct->pBranchInfo[ui32BranchId]->ui32NumStages=1;
            pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].ui32BranchId = ui32BranchId;
            pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].ui32StageId = ui32StageId;
    
            pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].eAudioAlgoType = 
                                            BRAP_CIT_P_GfxAlgoType_eDecode;
    
            pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eGfxAlgo = BRAP_DSPCHN_GfxType_eSoftGfx;
    
            BDBG_MSG(("pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eGfxAlgo %d"
                ,pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].uAlgorithm.eGfxAlgo));        
            /* Stage Connectivity */
            pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.ui32NumSrc =0 ;
            pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.sSrcDetails[0].eType =BRAP_CIT_P_FwStgSrcDstType_eInvalid;
    

            pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.ui32NumDistinctOp = 0;
            pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.ui32NumDst[0] = 0;        
            pCitInputStruct->pBranchInfo[ui32BranchId]->sFwStgInfo[ui32StageId].sStgConnectivity.eDistinctOpType[0] = BRAP_AF_P_DistinctOpType_eInvalid;         
       
            psAudProcNetwork->sAudProcessingStage[0][0].ui32CitBranchId = ui32BranchId;
            psAudProcNetwork->sAudProcessingStage[0][0].ui32CitStageId = ui32StageId;                   
        }       
    
        
    
            /* Assigning Mit Structure */
            pCitInputStruct->psMit= &(hRap->sMit);
    
            /* Assigning CIT memInfo */
            pCitInputStruct->sMemInfo.ui32SharedMemPhysAdr = hRap->sMemInfo.sDspScratchInfo[hFwTaskCreate->hDsp->uiDspIndex].ui32BaseAddr;      
            pCitInputStruct->sMemInfo.ui32SharedMemSize = hRap->sMemInfo.sDspScratchInfo[hFwTaskCreate->hDsp->uiDspIndex].ui32Size;
            pCitInputStruct->sMemInfo.ui32TaskMemPhysAdr = hRap->sMemInfo.sTskMemInfo[hFwTaskCreate->hDsp->uiDspIndex].sTskIFrameCfgBufInfo[BRAP_GET_TASK_INDEX(hFwTaskCreate->uiTaskId)].ui32BaseAddr;
            pCitInputStruct->sMemInfo.ui32TaskMemSize = hRap->sMemInfo.sTskMemInfo[hFwTaskCreate->hDsp->uiDspIndex].sTskIFrameCfgBufInfo[BRAP_GET_TASK_INDEX(hFwTaskCreate->uiTaskId )].ui32Size;
            pCitInputStruct->sRdbCfg.ui32RdbBaseOffset = 0;
            pCitInputStruct->sRdbCfg.ui32RdbDspOffset = BCHP_PHYSICAL_OFFSET;
        
   
            
    error:    
        if(BERR_SUCCESS != ret)
        {
                if (NULL!=pCitInputStruct->pBranchInfo[0])
                {
                    BKNI_Free(pCitInputStruct->pBranchInfo[0]);
                    pCitInputStruct->pBranchInfo[0] = NULL;
                }
        }
        BDBG_LEAVE(BRAP_FWIF_P_GenerateGfxCitInput);
        return ret;
    }

BERR_Code   BRAP_GFX_P_GetAndFreeBuffer(
    BRAP_ChannelHandle  hRapCh,
    unsigned int    uiLastCompletedOperationId,/*[In]*/
    unsigned int    *uiBufferId /*Out*/)
{
    BERR_Code                       ret=BERR_SUCCESS;
    unsigned int i =0;

    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(uiBufferId);    

    for(i = 0; i < BRAP_MAX_GFX_OP_IN_QUEUE ; i++)
    {
        if((hRapCh->hRap->sMemInfo.sGfxfInfo.bFree[i] == false)
            &&(hRapCh->hRap->sMemInfo.sGfxfInfo.uiGfxOpId[i] < uiLastCompletedOperationId))
        {
            hRapCh->hRap->sMemInfo.sGfxfInfo.bFree[i] = true;
            hRapCh->hRap->sMemInfo.sGfxfInfo.uiGfxOpId[i] = 0;            
        }
    }
    for(i = 0; i < BRAP_MAX_GFX_OP_IN_QUEUE ; i++)
    {
        if(hRapCh->hRap->sMemInfo.sGfxfInfo.bFree[i] == true)
        {
            *uiBufferId = i;
            break;
        }
    }    
    BDBG_MSG(("BRAP_GFX_P_GetAndFreeBuffer i = %d",i));
    if(i >= BRAP_MAX_GFX_OP_IN_QUEUE)
    {
        ret = BERR_NOT_SUPPORTED;
    }
    return ret;
}
BERR_Code   BRAP_GFX_P_NormalizeRectangle(
                    BRAP_GFX_P_SurfaceStruct    *psSurface,/*[In]*/
                    const BRAP_GFX_Rect                     *psRectAppFormat,/*[in]*/
                    BRAP_GFX_P_Rect                 *psRectFwFormat   /*[out]*/ 
)
{
    BERR_Code                       ret=BERR_SUCCESS;

    BDBG_ASSERT(psSurface);
    BDBG_ASSERT(psRectAppFormat);    
    BDBG_ASSERT(psRectFwFormat);      

/*Correcting X coordinate*/
    if(psRectAppFormat->iX < 0)
    {
        psRectFwFormat->sStartXY.x = 0;
    }
    else if(psRectAppFormat->iX > (int32_t)psSurface->sSurfaceDimension.blockSize.width)
    {
        BDBG_ERR(("X-Coordinate (%d) of Rectangle is beyond width (%d) of Surface",
            psRectAppFormat->iX,psSurface->sSurfaceDimension.blockSize.width));
        ret = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto end;
    }
    else
    {
        psRectFwFormat->sStartXY.x = psRectAppFormat->iX;        
    }

/*Correcting Y Coordinate*/
    if(psRectAppFormat->iY < 0)
    {
        psRectFwFormat->sStartXY.y = 0;
    }
    else if(psRectAppFormat->iY > (int32_t)psSurface->sSurfaceDimension.blockSize.height)
    {
        BDBG_ERR(("Y-Coordinate (%d) of Rectangle is beyond height (%d) of Surface",
            psRectAppFormat->iY,psSurface->sSurfaceDimension.blockSize.height));
        ret = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto end;
    }
    else
    {
        psRectFwFormat->sStartXY.y = psRectAppFormat->iY;        
    }        

/*Correcting Height and width*/
    if((psRectAppFormat->uiHeight == 0)
        &&(psRectAppFormat->uiWidth== 0))
    {
            /*Keep Rect Height and width within Range of */
            psRectFwFormat->sBlkSize.height = psSurface->sSurfaceDimension.blockSize.height - psRectFwFormat->sStartXY.y;
            psRectFwFormat->sBlkSize.width = psSurface->sSurfaceDimension.blockSize.width - psRectFwFormat->sStartXY.x;            
    }
    else
    {
                /*Keep Rect Height and width within Range of */
                if((psRectAppFormat->uiHeight + psRectFwFormat->sStartXY.y) > psSurface->sSurfaceDimension.blockSize.height)
                {
                    psRectFwFormat->sBlkSize.height = psSurface->sSurfaceDimension.blockSize.height - psRectFwFormat->sStartXY.y;
                    BDBG_MSG(("Truncating Height of Rect  to %d, Height  of surface = %d, y Coordinate of Rect =%d ",
                        psRectFwFormat->sBlkSize.height ,psSurface->sSurfaceDimension.blockSize.height,psRectFwFormat->sStartXY.y));
                }
               else
                {
                    psRectFwFormat->sBlkSize.height = psRectAppFormat->uiHeight;
                }

                if((psRectAppFormat->uiWidth + psRectFwFormat->sStartXY.x) > psSurface->sSurfaceDimension.blockSize.width)
                {
                    psRectFwFormat->sBlkSize.width = psSurface->sSurfaceDimension.blockSize.width - psRectFwFormat->sStartXY.x;
                    BDBG_MSG(("Truncating Width of Rect  to %d, Width  of surface = %d, X Coordinate of Rect =%d ",
                        psRectFwFormat->sBlkSize.width,psSurface->sSurfaceDimension.blockSize.width,psRectFwFormat->sStartXY.x));                    
                }
                else
                {
                    psRectFwFormat->sBlkSize.width = psRectAppFormat->uiWidth;                
                }
    }

end:        
    return ret;
}    


BERR_Code   BRAP_P_GetGfxStatusInfo(
unsigned int	        uiStatusBufAddr,    
BRAP_FWIF_P_GfxStageStatus  *psGfxStatusBuf)
{
    uint32_t           *bufPtr=NULL; 
    unsigned int i=0;
    BERR_Code                       ret=BERR_SUCCESS;    

    BDBG_ASSERT(psGfxStatusBuf);

    bufPtr =(uint32_t *) psGfxStatusBuf;
    for(i=0; i<sizeof(BRAP_FWIF_P_GfxStageStatus)/4; i++)
    {
        *((volatile uint32_t *)(bufPtr+i)) = (uint32_t)BRAP_MemRead32(uiStatusBufAddr+i*4);
    }
    if(0 != psGfxStatusBuf->ui32StatusValid)
    {
        BDBG_MSG(("BRAP_P_GetGfxStatusInfo: Status buffer is not in valid status %d",psGfxStatusBuf->ui32StatusValid));
        ret  = (BRAP_ERR_BAD_DEVICE_STATE);
        goto end;        
    }    

end:
    return ret;
}  
    
#endif    


