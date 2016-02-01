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

/*Includes*/
#include "brap_gfx_priv.h"
#include "brap_priv.h"

BDBG_MODULE(rap_gfx);


/*Static Consts*/
/* Default BRAP_ChannelSettings definition */
static const BRAP_GFX_OpenSettings BRAP_sDefGfxOpenSettings = 
{
    BRAP_MAX_GFX_OP_SUPPORTED
};

static const BRAP_GFX_Settings BRAP_sDefGfxStartSettings = 
{
    NULL,
    NULL,
    0
};


static const BRAP_GFX_BlitSettings  BRAP_sBlitSettings =
{
    NULL,
    NULL,
    NULL,
    {/*sSrcRect*/
            0,
            0,
            0,
            0
    },
    {/*sDstRect*/
            0,
            0,
            0,
            0
    },
    {/*sOutRect*/
            0,
            0,
            0,
            0
    },
    BRAP_GFX_eBlitColorOperation_eCopySource,/*eColorOperation*/
    BRAP_GFX_eBlitAlphaOperation_eCopySource,/*eAplhaOperation*/
    0,/*uiConstAlphaColorOp*/
    0,/*uiConstAlphaAlphaOp*/
    false,/*bEnableAntiFlutterFilter*/
    false,/*bEnableCSC*/
    {/*sCSCMatrix*/
        0,/*uiShift*/
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}/*iCoeffMatrix*/
    },
    BRAP_GFX_eScalingFilter_LinearInterpolation,
    {/*sColorKeyParams*/
        {/*sSource*/
            false,
            0xffffffff,
            0xffffffff,
            0xffffffff,
            0xffffffff,
            0xffffffff                
        },
        {/*sDest*/
            false,
            0xffffffff,
            0xffffffff,
            0xffffffff,
            0xffffffff,
            0xffffffff                
        }
    }
};

static const BRAP_GFX_FillSettings  BRAP_sFillSettings =
{    
    NULL,
    {/*sFillRect*/
        0,
        0,
        0,
        0
    },
    0,/*uiFillColor*/
    BRAP_GFX_eFillOperation_eCopy,
    BRAP_GFX_eFillOperation_eCopy
};


/***************************************************************************
Summary:
	Gets the default channel settings for an Graphics channel.
	
Description:
	This API returns the default channel settings of a Graphics channel.

Returns:
	BERR_SUCCESS on success else error
	
See Also:
	BRAP_GFX_Open
***************************************************************************/
BERR_Code BRAP_GFX_GetDefaultOpenSettings(
	BRAP_GFX_OpenSettings    *pDefSettings	/* [out] Default channel settings */
	)
{
    BDBG_ENTER(BRAP_GFX_GetDefaultOpenSettings);

    /* Assert the function argument(s) */
    BDBG_ASSERT(pDefSettings);


    *pDefSettings = BRAP_sDefGfxOpenSettings;

    BDBG_LEAVE(BRAP_GFX_GetDefaultOpenSettings);

    return BERR_SUCCESS;
}


/***************************************************************************
Summary:
	API used to open a Graphics channel.
	
Description:
    It is used to instantiate a Graphics channel. It allocates the handle and
    starts the GFX task in the DSP.
	
	Note: This should be called only after the device has been opened with 
	BRAP_Open().
	
Returns:
	BERR_SUCCESS on success else error
	
See Also:
	BRAP_GFX_Close
****************************************************************************/
BERR_Code BRAP_GFX_Open(
	BRAP_Handle 			    hRap,		/* [in] The Raptor Audio device handle*/
	BRAP_GFX_Handle 		    *phGfxCh,	/* [out] The Gfx Channel handle */
	const BRAP_GFX_OpenSettings	*pChnSettings  /* [in] Open-time Settings*/ 
	)
{
    BERR_Code ret = BERR_SUCCESS;
    BRAP_ChannelHandle hRapCh = NULL;
    bool bWdgRecovery = false;
    unsigned int    uiChannelNo=0;

    BDBG_ENTER(BRAP_GFX_Open);

    /* Assert the function argument(s) */
    BDBG_ASSERT(hRap);
    BDBG_ASSERT(phGfxCh); 
    BDBG_ASSERT(pChnSettings);

    if(false == BRAP_FWDWNLD_P_IsGfxSupported(0))
    {
        BDBG_ERR(("Please export RAP_GFX_SUPPORT=y at the compile time for SOft Gfx"));
        ret = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto exit;
    }    


#ifdef RAP_GFX_SUPPORT

    if(pChnSettings->maxOperations > BRAP_MAX_GFX_OP_SUPPORTED)
    {
        BDBG_ERR(("Max Operation Supported is  = %d .maxOperations = %d is exceeding this Value",BRAP_MAX_GFX_OP_SUPPORTED,pChnSettings->maxOperations));
        ret = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto error;                
    }


    bWdgRecovery = BRAP_P_GetWatchdogRecoveryFlag(hRap);

    if(bWdgRecovery == false)
    {
        /* Allocate an Gfx Channel handle */
        hRapCh = (BRAP_ChannelHandle)BKNI_Malloc(sizeof(BRAP_P_Channel));
        if(NULL == hRapCh)
        {
            BDBG_ERR(("Memory allocation for channel handle failed"));
            ret = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto exit;
        }
        BKNI_Memset(hRapCh,0,sizeof(BRAP_P_Channel));
        /* Initialize the Channel Handle */
        ret = BRAP_P_InitRapChannelHandle(hRapCh,hRap);
        if( ret!=BERR_SUCCESS )
        {
            ret = BERR_TRACE( ret );
            goto error;
        }

        /* Populate hRapCh using hRap, pChnSettings etc. */
        hRapCh->hChip                   = hRap->hChip ;
        hRapCh->hRegister               = hRap->hRegister;
        hRapCh->hHeap                   = hRap->hHeap ;
        hRapCh->hInt                    = hRap->hInt ;
        hRapCh->hRap                    = hRap;
        hRapCh->hTimer                  = NULL;

        /* Invalid state as not yet opened successfully */
        hRapCh->eState                  = BRAP_P_State_eInvalid; 
        hRapCh->eChannelType            = BRAP_ChannelType_eGfx; 
        hRapCh->bInternalCallFromRap    = false;

        hRapCh->sGfxSettings.sOpenGfxSettings = BRAP_sDefGfxOpenSettings;
        hRapCh->sGfxSettings.sStartGfxSettings = BRAP_sDefGfxStartSettings;
        

        /* Validate if this channel can be added to hRap */
        for(uiChannelNo=0; uiChannelNo<BRAP_RM_P_MAX_GFX_CHANNELS; uiChannelNo++)
        {
            if(BRAP_P_CheckIfPtrInvalid((void *)hRap->hRapGfxCh[uiChannelNo]) == true)
            {
                break;
            }
        }

        if(BRAP_RM_P_MAX_GFX_CHANNELS <= uiChannelNo)
        {
            BDBG_ERR(("Max number of GFX channels(%d) already exist", uiChannelNo));
            ret = BERR_TRACE(BERR_NOT_SUPPORTED);
            goto end_open;
        }
        
        if (ret != BERR_SUCCESS)
        {
            ret = BERR_TRACE(ret);
            goto end_open;
        }
    }
    else
    { 
        /* In watchdog recovery */
        hRapCh = (BRAP_ChannelHandle)*phGfxCh;
    } /* bWdgRecovery */    

    ret = BRAP_P_InterruptInstall (hRapCh);
    if(ret != BERR_SUCCESS)
    {
        ret = BERR_TRACE(ret);
        BDBG_ERR(("InstallInterrupt()failed for RAP DEC Channel handle 0x%x", 
        hRapCh));
        goto error;
    }

    /* only if channel has been successfully opened, save the handle */
     if((BERR_SUCCESS == ret) && (false == bWdgRecovery))
     {
         hRapCh->uiChannelNo = uiChannelNo;  
         hRap->hRapGfxCh[uiChannelNo] = hRapCh ;    
     }       

    ret = BRAP_GFX_P_StartChannel(hRapCh,pChnSettings);
    if(ret != BERR_SUCCESS)
    {
        ret = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto error;        
    }    

end_open:	
    
    if(false == bWdgRecovery)
    {
        if(BERR_SUCCESS == ret)
        {
            *phGfxCh = (BRAP_GFX_Handle)hRapCh;
            goto exit;
        }
        else
        {
            *phGfxCh = NULL;
            goto error;
        }
    }
    else
    {
        if(BERR_SUCCESS == ret)
        {
            goto exit;
        }
        else
        {
            goto error;
        }
    }

error:
    /* Free up the channel handle */
    BKNI_Free(hRapCh);

#else
BSTD_UNUSED(hRapCh);
BSTD_UNUSED(bWdgRecovery);
BSTD_UNUSED(uiChannelNo);
BSTD_UNUSED(pChnSettings);
#endif
exit:

    BDBG_LEAVE(BRAP_GFX_Open);
    return ret; 
}	


    /***************************************************************************
    Summary:
        API used to close a Raptor Graphics Channel.
    
    Description:
        It closes the instance of a Graphics channel operation. It frees the channel
    handle and stops the gfx task in the DSP.
    
    Returns:
        BERR_SUCCESS on success else error
    
    See Also:
    BRAP_GFX_Open
    ****************************************************************************/
BERR_Code BRAP_GFX_Close( 
    BRAP_GFX_Handle  hGfxCh                  /* [in]The Gfx Channel handle */
        )

{
    BERR_Code ret = BERR_SUCCESS;
    unsigned int    uiChannelNo = 0;
    BRAP_Handle 	hRap = NULL;
    BRAP_ChannelHandle hRapCh=NULL;

    BDBG_ENTER(BRAP_GFX_Close);
    /* Assert the function argument */
    BDBG_ASSERT(hGfxCh);


    if(false == BRAP_FWDWNLD_P_IsGfxSupported(0))
    {
        BDBG_ERR(("Please export RAP_GFX_SUPPORT=y at the compile time for SOft Gfx"));
        ret =BERR_TRACE(BERR_NOT_SUPPORTED);
        goto error;
    }    
 
#ifdef RAP_GFX_SUPPORT

    hRapCh = (BRAP_ChannelHandle)hGfxCh;

    hRap = hRapCh->hRap;

    ret = BRAP_GFX_P_StopChannel(hGfxCh);
    if(ret != BERR_SUCCESS)
    {
        ret = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto error;        
    }

    /* Mark the place for the current channel handle to 'invalid' inside RAP 
    handle */
    for(uiChannelNo=0; uiChannelNo<BRAP_RM_P_MAX_GFX_CHANNELS; uiChannelNo++)
    {
        if(hRapCh == hRap->hRapGfxCh[uiChannelNo])
        {
            break;
        }
    }	

    if(BRAP_RM_P_MAX_GFX_CHANNELS <= uiChannelNo)
    {
        BDBG_ERR(("No such Open Channel Exist, hRapCh = %x", hRapCh));
        ret = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto error;
    }    
    hRapCh->hRap->hRapGfxCh[hRapCh->uiChannelNo] = NULL;

    BKNI_Free( hRapCh );


#else
BSTD_UNUSED(hRapCh);
BSTD_UNUSED(hRap);
BSTD_UNUSED(uiChannelNo);   
#endif    
error:
    BDBG_LEAVE(BRAP_GFX_Close);
    return ret;
}    





/***************************************************************************
Summary:
    It does the Gfx Fill Operation.

Description:
    This API does the Gfx Fill Operation as per the setting passed. It returns Operation Id which 
    corresponds to current request.

Returns:
    BERR_SUCCESS if successful else error value.            

See Also:
    BRAP_GFX_BlitOperation
****************************************************************************/

BERR_Code  BRAP_GFX_Fill(        
                BRAP_GFX_Handle          hGfxCh,          /* [in] Gfx channel handle */
                const BRAP_GFX_FillSettings                   *psFillSetting /*[in] Fill Settings*/
    )
{
    BERR_Code ret = BERR_SUCCESS;
    BRAP_FWIF_P_Command sCommand;
    BRAP_FWIF_P_Response sRsp;    
    BRAP_FWIF_P_FwTaskHandle    hTask = NULL;
    BRAP_Handle hRap=NULL;
    BRAP_Fwif_P_FwBufInfo  *psFwBufInfo=NULL;
    BRAP_GFX_P_Fill                   sFillSettingsInFwFormat;
    BRAP_ChannelHandle hRapCh=NULL;        
    unsigned int uiResponseId;
    unsigned int uiLastReadOperationId=0,uiLastCompletedOperationId= 0, uiBufferId =BRAP_INVALID_VALUE;
    BRAP_FWIF_P_GfxStageStatus  sGfxStatusBuf;    
        

    BDBG_ENTER(BRAP_GFX_Fill);
    BDBG_ASSERT(hGfxCh);
    BDBG_ASSERT(psFillSetting);
    BDBG_MSG(("psFillSetting->sFillRect.uiHeight = %d , psFillSetting->sFillRect.uiWidth = %d",psFillSetting->sFillRect.uiHeight,psFillSetting->sFillRect.uiWidth));

    if(false == BRAP_FWDWNLD_P_IsGfxSupported(0))
    {
        BDBG_ERR(("Please export RAP_GFX_SUPPORT=y at the compile time for SOft Gfx"));
        ret = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto end;
    }    
    

#ifdef RAP_GFX_SUPPORT    

    if(((psFillSetting->sFillRect.uiWidth == 0)&&(psFillSetting->sFillRect.uiHeight != 0)) 
        ||((psFillSetting->sFillRect.uiWidth != 0)&&(psFillSetting->sFillRect.uiHeight == 0)))
    {
        BDBG_ERR(("Height and/or width for Fill Rect is not valid"));
        ret =  BERR_TRACE(BERR_INVALID_PARAMETER);
        goto end;
    }  
   

    hRapCh = (BRAP_ChannelHandle)hGfxCh;    

    hRap = hRapCh->hRap;

    BKNI_Memset(&sCommand,0,sizeof(BRAP_FWIF_P_Command));
    
    if((hRapCh->eState == BRAP_P_State_eOpened)
        ||(hRapCh->bStopinvoked == true))
    {
        BDBG_ERR(("BRAP_GFX_FillOperation: Gfx Channel hRapCh =%x is not started", hRapCh));    
        ret = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto end;
    }
    else if(hRapCh->eState == BRAP_P_State_eStarted)
    {
        BRAP_P_GetTask(hRapCh, &hTask);            
        if(hTask == NULL)
        {   
            BDBG_ERR(("BRAP_GFX_FillOperation: hTask Is NULL for hRapCh =%x", hRapCh));
            ret = BERR_TRACE(BRAP_ERR_INVALID_TASK);
            goto end;
        }
        BKNI_Memset((void *)&sRsp,0,sizeof(BRAP_FWIF_P_Response));


/*        BKNI_Memset((void *)&sGfxStatusBuf,0xFF,sizeof(BRAP_FWIF_P_GfxStageStatus));    */

        ret = BRAP_P_GetGfxStatusInfo(hTask->sGfxCitOutput.sCitBranchInfo[0].sCitStgInfo[0].sFwOpNodeInfo[0].ui32StatusBufAdr,
                                                &sGfxStatusBuf);

        if(BERR_SUCCESS != ret)
        {
            uiLastReadOperationId = 0;
            uiLastCompletedOperationId = 0;
        }
        else
        {            
            BDBG_MSG(("---> FILL  sGfxStatusBuf.ui32LastReadGfxCmdId = %d , sGfxStatusBuf.ui32LastCompletedGfxCmdId = %d Last Command Send = %d",
                sGfxStatusBuf.ui32LastReadGfxCmdId,sGfxStatusBuf.ui32LastCompletedGfxCmdId,(hTask->uiCommandCounter-1)));        
            uiLastReadOperationId  = sGfxStatusBuf.ui32LastReadGfxCmdId;
            uiLastCompletedOperationId  = sGfxStatusBuf.ui32LastCompletedGfxCmdId;
        }


        if((hTask->uiCommandCounter - uiLastReadOperationId -1) >= hRapCh->sGfxSettings.sOpenGfxSettings.maxOperations)
        {
            BDBG_ERR(("Can not Handle This Fill Command. Number of Pending Operations on ZSP = %d Last Command Sent = %d , sGfxStatusBuf.ui32LastReadGfxCmdId = %d"
                ,(hTask->uiCommandCounter - uiLastReadOperationId -1),(hTask->uiCommandCounter-1),uiLastReadOperationId));
            ret = BERR_TRACE(BRAP_GFX_QUEUE_FULL);
            goto end;                
        }


        /* Create Fill command */
        sCommand.sCommandHeader.ui32CommandID = BRAP_FWGFX_P_FILL_COMMAND_ID;
        sCommand.sCommandHeader.ui32CommandCounter = hTask->uiCommandCounter++;
        sCommand.sCommandHeader.ui32TaskID =  hTask->uiTaskId;
        sCommand.sCommandHeader.eResponseType = BRAP_FWIF_P_ResponseType_eNone;
        uiResponseId = BRAP_FWGFX_P_FILL_ACK_ID;            


        ret = BRAP_GFX_P_MapFillSettings(hRap,hRapCh,psFillSetting,&sFillSettingsInFwFormat);
        if(BERR_SUCCESS != ret)
        {
            BDBG_ERR(("Error in Mapping Pixel format."));
            ret =BERR_TRACE(ret);
            goto end;
        }

        ret = BRAP_GFX_P_GetAndFreeBuffer(hRapCh,uiLastCompletedOperationId,&uiBufferId);
        if((ret !=BERR_SUCCESS)
            ||(uiBufferId == BRAP_INVALID_VALUE))
        {   
            BDBG_ERR(("BRAP_GFX_Fill: Out of GFX Buffer to send Fill Command, "));
            BDBG_ERR(("---> FILL  sGfxStatusBuf.ui32LastReadGfxCmdId = %d , sGfxStatusBuf.ui32LastCompletedGfxCmdId = %d Last Command Send = %d",
                uiLastReadOperationId,uiLastCompletedOperationId,(hTask->uiCommandCounter-1)));        
            
            ret = BERR_TRACE(BRAP_GFX_QUEUE_FULL);
            goto end;
        }            
        psFwBufInfo = &(hRap->sMemInfo.sGfxfInfo.sGfxBuffer[uiBufferId]);
        hRap->sMemInfo.sGfxfInfo.bFree[uiBufferId] = false;
        hRap->sMemInfo.sGfxfInfo.uiGfxOpId[uiBufferId] = sCommand.sCommandHeader.ui32CommandCounter;
        
        BKNI_Memcpy((void *)(volatile void *)psFwBufInfo->ui32BaseAddr,(uint32_t *)&sFillSettingsInFwFormat,sizeof(BRAP_GFX_P_Fill));   
        

        BRAP_P_ConvertAddressToOffset(hRap->hHeap, (void *)(psFwBufInfo ->ui32BaseAddr), 
        &(sCommand.uCommand.sGfxFill.sCmdBufferInfo.ui32DramBufferAddress));	

        sCommand.uCommand.sGfxFill.sCmdBufferInfo.ui32BufferSizeInBytes = psFwBufInfo->ui32Size;        
        hTask->uiLastEventType = sCommand.sCommandHeader.ui32CommandID;
        ret = BRAP_FWIF_P_SendCommand(hTask->hDsp->hCmdQueue, &sCommand,hRap,hTask);
        if(BERR_SUCCESS != ret)
        {
            if((hRap->bWatchdogTriggered == false)
            &&(hTask->bStopped == false))
            {
                BDBG_ERR(("BRAP_GFX_FillOperation: Send_Command failed!"));
                ret = BERR_TRACE(ret);
                goto end;
            }
            else
                ret = BERR_SUCCESS;    
        }
    
    }

#else
BSTD_UNUSED(sCommand);
BSTD_UNUSED(sRsp);   
BSTD_UNUSED(hTask);
BSTD_UNUSED(hRap);
BSTD_UNUSED(psFwBufInfo);       
BSTD_UNUSED(hRapCh);
BSTD_UNUSED(uiResponseId);   
BSTD_UNUSED(sFillSettingsInFwFormat);
BSTD_UNUSED(sGfxStatusBuf);
BSTD_UNUSED(uiBufferId);
BSTD_UNUSED(uiLastReadOperationId);
BSTD_UNUSED(uiLastCompletedOperationId);
#endif    
end:  
#ifdef RAP_GFX_SUPPORT    
    if(ret !=BERR_SUCCESS)
    {
        if(uiBufferId != BRAP_INVALID_VALUE)
        {
            hRap->sMemInfo.sGfxfInfo.bFree[uiBufferId] = true;
                hRap->sMemInfo.sGfxfInfo.uiGfxOpId[uiBufferId] = 0;                
        }
    }
#endif    


    BDBG_LEAVE(BRAP_GFX_Fill);    
    return ret;
}

/***************************************************************************
Summary:
    It does the Gfx Blit Operation.

Description:
    This API does the Gfx Blit Operation as per the setting passed. It returns Operation Id which 
    corresponds to current request. It can perform following as per settings passed to this function
    -Blit
    -Blend
    -CSC
    -Scaling

Returns:
    BERR_SUCCESS if successful else error value.            

See Also:
    BRAP_GFX_FillOperation
****************************************************************************/
BERR_Code  BRAP_GFX_Blit(        
                BRAP_GFX_Handle          hGfxCh,          /* [in] Gfx channel handle */
                const BRAP_GFX_BlitSettings                   *psBlitSetting/*[in] Blit Settings*/
                
    )
    {
        BERR_Code ret = BERR_SUCCESS;
        BRAP_FWIF_P_Command sCommand;
        BRAP_FWIF_P_Response sRsp;    
        BRAP_FWIF_P_FwTaskHandle    hTask = NULL;
        BRAP_Handle hRap=NULL;
        BRAP_Fwif_P_FwBufInfo  *psFwBufInfo=NULL;
        BRAP_GFX_P_Blit                   sBlitSettingsInFwFormat;    
        BRAP_ChannelHandle hRapCh=NULL;        
        unsigned int uiResponseId = 0;
        unsigned int uiLastReadOperationId=0,uiLastCompletedOperationId=0,uiBufferId =BRAP_INVALID_VALUE;
        BRAP_FWIF_P_GfxStageStatus  sGfxStatusBuf;
        
        
    
        BDBG_ENTER(BRAP_GFX_Blit);
        BDBG_ASSERT(hGfxCh);
        BDBG_ASSERT(psBlitSetting);
        BDBG_MSG(("psBlitSetting->sSourceRect.uiHeight = %d , psBlitSetting->sSourceRect.uiWidth = %d"
                        "psBlitSetting->sDestinationRect.uiHeight = %d , psBlitSetting->sDestinationRect.uiWidth = %d"
                        "psBlitSetting->sOutputRect.uiHeight = %d , psBlitSetting->sOutputRect.uiWidth = %d"            
            ,psBlitSetting->sSourceRect.uiHeight,psBlitSetting->sSourceRect.uiWidth
            ,psBlitSetting->sDestinationRect.uiHeight,psBlitSetting->sDestinationRect.uiWidth            
            ,psBlitSetting->sOutputRect.uiHeight,psBlitSetting->sOutputRect.uiWidth                
            ));        

        if(false == BRAP_FWDWNLD_P_IsGfxSupported(0))
        {
            BDBG_ERR(("Please export RAP_GFX_SUPPORT=y at the compile time for SOft Gfx"));
            ret = BERR_TRACE(BERR_NOT_SUPPORTED);
            goto end;
        }    

#ifdef RAP_GFX_SUPPORT
/*Sanity Checks*/
        
        
        if(psBlitSetting->eColorOperation == BRAP_GFX_eBlitColorOperation_eMax || 
           psBlitSetting->eAlphaOperation == BRAP_GFX_eBlitAlphaOperation_eMax)    
        {
            BDBG_ERR(("colorOp[%d] and/or alphaOp[%d] is invalid",
                psBlitSetting->eColorOperation, psBlitSetting->eAlphaOperation));
            ret =  BERR_TRACE(BERR_INVALID_PARAMETER);
            goto end;     
        }
        if(((psBlitSetting->sSourceRect.uiWidth == 0)&&(psBlitSetting->sSourceRect.uiHeight != 0)) 
            ||((psBlitSetting->sSourceRect.uiWidth != 0)&&(psBlitSetting->sSourceRect.uiHeight == 0))) 
        {
            BDBG_ERR(("Height and/or width of Source  Rect is not valid "));
            ret =  BERR_TRACE(BERR_INVALID_PARAMETER);
            goto end;
        }  
        
        if(((psBlitSetting->sOutputRect.uiWidth == 0)&&(psBlitSetting->sOutputRect.uiHeight != 0)) 
            ||((psBlitSetting->sOutputRect.uiWidth != 0)&&(psBlitSetting->sOutputRect.uiHeight == 0))) 
        {
            BDBG_ERR(("Height and/or width of Out  Rect is not valid "));
            ret =  BERR_TRACE(BERR_INVALID_PARAMETER);
            goto end;     
        }
        if(!((psBlitSetting->eColorOperation == BRAP_GFX_eBlitColorOperation_eCopySource)&& 
           ((psBlitSetting->eAlphaOperation == BRAP_GFX_eBlitAlphaOperation_eCopySource) ||
            (psBlitSetting->eAlphaOperation == BRAP_GFX_eBlitAlphaOperation_eCopyConstant))))
        {/*If it is not a Simple Blit OP*/
            if(((psBlitSetting->sDestinationRect.uiWidth == 0)&&(psBlitSetting->sDestinationRect.uiHeight != 0)) 
                ||((psBlitSetting->sDestinationRect.uiWidth != 0)&&(psBlitSetting->sDestinationRect.uiHeight == 0))) 
            {
                BDBG_ERR(("Height and/or width of Destination  Rect is not valid "));
                ret =  BERR_TRACE(BERR_INVALID_PARAMETER);
                goto end;
            }           
        }

        hRapCh = (BRAP_ChannelHandle)hGfxCh;    
        hRap = hRapCh->hRap;
        BKNI_Memset(&sCommand,0,sizeof(BRAP_FWIF_P_Command));        
        
        if((hRapCh->eState == BRAP_P_State_eOpened)
            ||(hRapCh->bStopinvoked == true))
        {
            BDBG_ERR(("BRAP_GFX_Blit: Gfx Channel hRapCh =%x is not started", hRapCh));    
            ret = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto end;
        }
        else if(hRapCh->eState == BRAP_P_State_eStarted)
        {
            BRAP_P_GetTask(hRapCh, &hTask);            
            if(hTask == NULL)
            {   
                BDBG_ERR(("BRAP_GFX_Blit: hTask Is NULL for hRapCh =%x", hRapCh));
                ret = BERR_TRACE(BRAP_ERR_INVALID_TASK);
                goto end;
            }
            BKNI_Memset((void *)&sRsp,0,sizeof(BRAP_FWIF_P_Response));

            /*        BKNI_Memset((void *)&sGfxStatusBuf,0xFF,sizeof(BRAP_FWIF_P_GfxStageStatus));    */

            ret = BRAP_P_GetGfxStatusInfo(hTask->sGfxCitOutput.sCitBranchInfo[0].sCitStgInfo[0].sFwOpNodeInfo[0].ui32StatusBufAdr,
                                                    &sGfxStatusBuf);
            if(BERR_SUCCESS != ret)
            {
                uiLastReadOperationId = 0;
                uiLastCompletedOperationId  =0;
            }
            else
            {
                BDBG_MSG(("---> BLIT sGfxStatusBuf.ui32LastReadGfxCmdId = %d , sGfxStatusBuf.ui32LastCompletedGfxCmdId = %d ",
                    sGfxStatusBuf.ui32LastReadGfxCmdId,sGfxStatusBuf.ui32LastCompletedGfxCmdId)); 
                uiLastReadOperationId  = sGfxStatusBuf.ui32LastReadGfxCmdId;
                uiLastCompletedOperationId  =sGfxStatusBuf.ui32LastCompletedGfxCmdId;                
            }            

            if((hTask->uiCommandCounter - uiLastReadOperationId - 1) >= hRapCh->sGfxSettings.sOpenGfxSettings.maxOperations)
            {
                BDBG_ERR(("Can not Handle This Blit Command. Number of Pending Operations on ZSP = %d Last Command Sent = %d , sGfxStatusBuf.ui32LastReadGfxCmdId = %d"
                ,(hTask->uiCommandCounter - uiLastReadOperationId -1),(hTask->uiCommandCounter-1),uiLastReadOperationId));                
                ret = BERR_TRACE(BRAP_GFX_QUEUE_FULL);
                goto end;                
            }
    
            /* Create Blit command */
            sCommand.sCommandHeader.ui32CommandID = BRAP_FWGFX_P_BLIT_COMMAND_ID;
            sCommand.sCommandHeader.ui32CommandCounter = hTask->uiCommandCounter++;
            sCommand.sCommandHeader.ui32TaskID =  hTask->uiTaskId;


            sCommand.sCommandHeader.eResponseType = BRAP_FWIF_P_ResponseType_eNone;
            uiResponseId = BRAP_FWGFX_P_BLIT_ACK_ID;            


            BKNI_Memset(&sBlitSettingsInFwFormat,0,sizeof(BRAP_GFX_P_Blit));
            ret = BRAP_GFX_P_MapBlitSettings(hRap,hRapCh,psBlitSetting,&sBlitSettingsInFwFormat);
            if(BERR_SUCCESS != ret)
            {
                BDBG_ERR(("Error in Mapping Pixel format."));
                ret =BERR_TRACE(ret);
                goto end;
            }

            ret = BRAP_GFX_P_GetAndFreeBuffer(hRapCh,uiLastCompletedOperationId,&uiBufferId);
            if((ret !=BERR_SUCCESS)
                ||(uiBufferId == BRAP_INVALID_VALUE))
            {   
                BDBG_ERR(("BRAP_GFX_Blit: Out of GFX Buffer to send Blit Command"));
                BDBG_ERR(("---> BLIT  sGfxStatusBuf.ui32LastReadGfxCmdId = %d , sGfxStatusBuf.ui32LastCompletedGfxCmdId = %d Last Command Send = %d",
                uiLastReadOperationId,uiLastCompletedOperationId,(hTask->uiCommandCounter-1)));                   
                ret = BERR_TRACE(BRAP_GFX_QUEUE_FULL);
                goto end;
            }            
            
            psFwBufInfo = &(hRap->sMemInfo.sGfxfInfo.sGfxBuffer[uiBufferId]);
            hRap->sMemInfo.sGfxfInfo.bFree[uiBufferId] = false;
            hRap->sMemInfo.sGfxfInfo.uiGfxOpId[uiBufferId] = sCommand.sCommandHeader.ui32CommandCounter;
            
            BKNI_Memcpy((void *)(volatile void *)psFwBufInfo->ui32BaseAddr,(uint32_t *)&sBlitSettingsInFwFormat,sizeof(BRAP_GFX_P_Blit));   
    
            BRAP_P_ConvertAddressToOffset(hRap->hHeap, (void *)(psFwBufInfo ->ui32BaseAddr), 
            &(sCommand.uCommand.sGfxBlit.sCmdBufferInfo.ui32DramBufferAddress));    
    
            sCommand.uCommand.sGfxBlit.sCmdBufferInfo.ui32BufferSizeInBytes = psFwBufInfo->ui32Size;        
            
            hTask->uiLastEventType = sCommand.sCommandHeader.ui32CommandID;
            ret = BRAP_FWIF_P_SendCommand(hTask->hDsp->hCmdQueue, &sCommand,hRap,hTask);
            if(BERR_SUCCESS != ret)
            {
                if((hRap->bWatchdogTriggered == false)
                &&(hTask->bStopped == false))
                {
                    BDBG_ERR(("BRAP_GFX_Blit: Send_Command failed!"));
                    ret = BERR_TRACE(ret);
                    goto end;
                }
                else
                    ret = BERR_SUCCESS;    
            }
        
        }

#else
    BSTD_UNUSED(sCommand);
    BSTD_UNUSED(sRsp);   
    BSTD_UNUSED(hTask);
    BSTD_UNUSED(hRap);
    BSTD_UNUSED(psFwBufInfo);       
    BSTD_UNUSED(hRapCh);
    BSTD_UNUSED(uiResponseId);   
    BSTD_UNUSED(sBlitSettingsInFwFormat); 
    BSTD_UNUSED(sGfxStatusBuf);
    BSTD_UNUSED(uiBufferId); 
    BSTD_UNUSED(uiLastCompletedOperationId);
	BSTD_UNUSED(uiLastReadOperationId);    
#endif        
end:    

#ifdef RAP_GFX_SUPPORT    
        if(ret !=BERR_SUCCESS)
        {
            if(uiBufferId != BRAP_INVALID_VALUE)
            {
                hRap->sMemInfo.sGfxfInfo.bFree[uiBufferId] = true;
                hRap->sMemInfo.sGfxfInfo.uiGfxOpId[uiBufferId] = 0;                
            }
        }
#endif
        BDBG_LEAVE(BRAP_GFX_Blit);    
        return ret;
    }

    BERR_Code  BRAP_GFX_GetDefaultBlitSettings(        
                    BRAP_GFX_BlitSettings                   *psBlitSetting                    
        )
{
    BERR_Code ret = BERR_SUCCESS;           

    BDBG_ENTER(BRAP_GFX_GetDefaultBlitSettings);
    BDBG_ASSERT(psBlitSetting);
    *psBlitSetting = BRAP_sBlitSettings;
    BDBG_LEAVE(BRAP_GFX_GetDefaultBlitSettings);
    return ret;
}
    
    BERR_Code  BRAP_GFX_GetDefaultFillSettings(        
                    BRAP_GFX_FillSettings                   *psFillSetting                    
        )
{
    BERR_Code ret = BERR_SUCCESS;           

    BDBG_ENTER(BRAP_GFX_GetDefaultFillSettings);
    BDBG_ASSERT(psFillSetting);
    *psFillSetting = BRAP_sFillSettings;
    BDBG_LEAVE(BRAP_GFX_GetDefaultFillSettings);
    return ret;
}        

BERR_Code  BRAP_GFX_Checkpoint(        
        BRAP_GFX_Handle          hGfxCh          /* [in] Gfx channel handle */
    )
{
    BERR_Code ret = BERR_SUCCESS;           
    BRAP_ChannelHandle hRapCh=NULL;        
    BRAP_FWIF_P_FwTaskHandle    hTask = NULL;
    unsigned int uiLastCompletedOperationId=0,uiLastReadOperationId=0;
    BRAP_FWIF_P_GfxStageStatus  sGfxStatusBuf;
    BRAP_FWIF_P_Command sCommand;
#if 0    
    BRAP_FWIF_P_Response sRsp;     
#endif    

    BDBG_ENTER(BRAP_GFX_Checkpoint);
    BDBG_ASSERT(hGfxCh);

    if(false == BRAP_FWDWNLD_P_IsGfxSupported(0))
    {
        BDBG_ERR(("Please export RAP_GFX_SUPPORT=y at the compile time for SOft Gfx"));
        ret = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto end;
    }

#ifdef RAP_GFX_SUPPORT        
    hRapCh = (BRAP_ChannelHandle)hGfxCh;
    
    if(hRapCh->eState == BRAP_P_State_eStarted)
    {
        BRAP_P_GetTask(hRapCh, &hTask);            
        if(hTask == NULL)
        {   
            BDBG_ERR(("BRAP_GFX_BlitOperation: hTask Is NULL for hRapCh =%x", hRapCh));
            ret = BERR_TRACE(BRAP_ERR_INVALID_TASK);
            goto end;
        }
    }

    /*Read Last Completed Operation ID (i.e. Command Counter) from Status Buffer*/
    /**TODO*/
    /*        BKNI_Memset((void *)&sGfxStatusBuf,0xFF,sizeof(BRAP_FWIF_P_GfxStageStatus));    */

    ret = BRAP_P_GetGfxStatusInfo(hTask->sGfxCitOutput.sCitBranchInfo[0].sCitStgInfo[0].sFwOpNodeInfo[0].ui32StatusBufAdr,
                                            &sGfxStatusBuf);

    if(BERR_SUCCESS != ret)
    {
        uiLastCompletedOperationId = 0;
        uiLastReadOperationId  = 0;
    }
    else
    {            
        BDBG_MSG(("sGfxStatusBuf.ui32LastCompletedGfxCmdId = %d",sGfxStatusBuf.ui32LastCompletedGfxCmdId));            
        uiLastCompletedOperationId  = sGfxStatusBuf.ui32LastCompletedGfxCmdId;
        uiLastReadOperationId  = sGfxStatusBuf.ui32LastReadGfxCmdId;
    }

    if((hTask->uiCommandCounter-1) == 0)
    {
        ret = BERR_SUCCESS;
        goto end;   
    }

    if(uiLastCompletedOperationId == (hTask->uiCommandCounter-1))
    {
        BDBG_MSG(("There are no outstanding blit and fill operations in the ZSP"));
        ret = BERR_SUCCESS;
        goto end;
    }
    else
    {
            BDBG_MSG(("There are some outstanding blit and/or fill operations in the ZSP queue.\n"
                                "When these blits/fills are completed Callback installed on BRAP_Interrupt_eGfxOperationComplete will be fired."
								"sGfxStatusBuf.ui32LastCompletedGfxCmdId = %d ,Last Read= %d Last  Sent = %d"
								,uiLastCompletedOperationId,uiLastReadOperationId,(hTask->uiCommandCounter-1)));
            /*Send Command to ZSP for firing the Interrupt when hTask->uiCommandCounter is completed.*/
            /**TODO*/

            /* Create Fill command */
            sCommand.sCommandHeader.ui32CommandID = BRAP_FWGFX_P_GFX_COMPLETION_CMD_ID;
            sCommand.sCommandHeader.ui32CommandCounter = 0;
            sCommand.sCommandHeader.ui32TaskID =  hTask->uiTaskId;
            sCommand.sCommandHeader.eResponseType = BRAP_FWIF_P_ResponseType_eNone;
            
            sCommand.uCommand.sGfxCmdCnt.ui32CmdCnt = hTask->uiCommandCounter-1;        
            
            hTask->uiLastEventType = sCommand.sCommandHeader.ui32CommandID;
            ret = BRAP_FWIF_P_SendCommand(hTask->hDsp->hCmdQueue, &sCommand,hRapCh->hRap,hTask);
            if(BERR_SUCCESS != ret)
            {
                if((hRapCh->hRap->bWatchdogTriggered == false)
                &&(hTask->bStopped == false))
                {
                    BDBG_ERR(("BRAP_GFX_Checkpoint: Send_Command failed!"));
                    ret = BERR_TRACE(ret);
                    goto end;
                }
                else
                    ret = BERR_SUCCESS;    
            }
            
            ret = BRAP_GFX_QUEUED;
            goto end;            
    }


#else
BSTD_UNUSED(hTask);
BSTD_UNUSED(uiLastCompletedOperationId);
BSTD_UNUSED(hRapCh);
BSTD_UNUSED(sCommand);
BSTD_UNUSED(sGfxStatusBuf);
BSTD_UNUSED(uiLastReadOperationId);
#endif    
end:
    BDBG_LEAVE(BRAP_GFX_Checkpoint);    
    return ret;
}

BERR_Code  BRAP_GFX_Idle(        
        BRAP_GFX_Handle          hGfxCh          /* [in] Gfx channel handle */
    )
{
    BERR_Code ret = BERR_SUCCESS;           
    BRAP_ChannelHandle hRapCh=NULL;        
    BRAP_FWIF_P_FwTaskHandle    hTask = NULL;
    unsigned int uiLastCompletedOperationId=0,uiLastReadOperationId=0;
    BRAP_FWIF_P_GfxStageStatus  sGfxStatusBuf;
    

    BDBG_ENTER(BRAP_GFX_Idle);
    BDBG_ASSERT(hGfxCh);

    if(false == BRAP_FWDWNLD_P_IsGfxSupported(0))
    {
        BDBG_ERR(("Please export RAP_GFX_SUPPORT=y at the compile time for SOft Gfx"));
        ret = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto end;
    }

#ifdef RAP_GFX_SUPPORT        
    hRapCh = (BRAP_ChannelHandle)hGfxCh;
    
    if(hRapCh->eState == BRAP_P_State_eStarted)
    {
        BRAP_P_GetTask(hRapCh, &hTask);            
        if(hTask == NULL)
        {   
            BDBG_ERR(("BRAP_GFX_Idle: hTask Is NULL for hRapCh =%x", hRapCh));
            ret = BERR_TRACE(BRAP_ERR_INVALID_TASK);
            goto end;
        }
    }

    /*Read Last Completed Operation ID (i.e. Command Counter) from Status Buffer*/
    /**TODO*/

    /*        BKNI_Memset((void *)&sGfxStatusBuf,0xFF,sizeof(BRAP_FWIF_P_GfxStageStatus));    */

    ret = BRAP_P_GetGfxStatusInfo(hTask->sGfxCitOutput.sCitBranchInfo[0].sCitStgInfo[0].sFwOpNodeInfo[0].ui32StatusBufAdr,
                                            &sGfxStatusBuf);

    if(BERR_SUCCESS != ret)
    {
        uiLastCompletedOperationId = 0;
        uiLastReadOperationId  = 0;
    }
    else
    {            
        BDBG_MSG(("sGfxStatusBuf.ui32LastCompletedGfxCmdId = %d",sGfxStatusBuf.ui32LastCompletedGfxCmdId));                
        uiLastCompletedOperationId  = sGfxStatusBuf.ui32LastCompletedGfxCmdId;
        uiLastReadOperationId  = sGfxStatusBuf.ui32LastReadGfxCmdId;
        
    }

    if((hTask->uiCommandCounter-1) == 0)
    {
        ret = BERR_SUCCESS;
        goto end;   
    }    

    if(uiLastCompletedOperationId == (hTask->uiCommandCounter-1))
    {
        BDBG_MSG(("BRAP_GFX_Idle: There are no outstanding blit and fill operations in the ZSP"));
        ret = BERR_SUCCESS;
        goto end;
    }
    else
    {
            BDBG_MSG(("BRAP_GFX_Idle :There are some outstanding blit and/or fill operations in the ZSP queue.\n"
								"sGfxStatusBuf.ui32LastCompletedGfxCmdId = %d ,Last Read= %d Last  Sent = %d"
								,uiLastCompletedOperationId,uiLastReadOperationId,(hTask->uiCommandCounter-1)));            
            ret = BRAP_GFX_BUSY;
            goto end;            
    }


#else
BSTD_UNUSED(hTask);
BSTD_UNUSED(uiLastCompletedOperationId);
BSTD_UNUSED(hRapCh);
BSTD_UNUSED(sGfxStatusBuf);
BSTD_UNUSED(uiLastReadOperationId);
#endif    
end:
    BDBG_LEAVE(BRAP_GFX_Idle);    
    return ret;
}

BERR_Code BRAP_GFX_SetSettings(
    BRAP_GFX_Handle              hGfxCh,        /* [in] Gfx channel handle */
    const BRAP_GFX_Settings     *pSettings      /* [in] Current settings */
    )
    {
        BERR_Code ret = BERR_SUCCESS;           
        BRAP_ChannelHandle hRapCh=NULL;        

    
        BDBG_ENTER(BRAP_GFX_SetSettings);
        BDBG_ASSERT(hGfxCh);
    
        if(false == BRAP_FWDWNLD_P_IsGfxSupported(0))
        {
            BDBG_ERR(("Please export RAP_GFX_SUPPORT=y at the compile time for SOft Gfx"));
            ret = BERR_TRACE(BERR_NOT_SUPPORTED);
            goto end;
        }
    
#ifdef RAP_GFX_SUPPORT        
        hRapCh = (BRAP_ChannelHandle)hGfxCh;

        if(pSettings->BRAP_CallbackFunc !=NULL)
        {
            BRAP_InstallAppInterruptCb(hRapCh,BRAP_Interrupt_eGfxOperationComplete,pSettings->BRAP_CallbackFunc,pSettings->pCheckpointParam1,pSettings->iCheckpointParam2);
        }
        else
        {
            BRAP_RemoveAppInterruptCb(hRapCh,BRAP_Interrupt_eGfxOperationComplete);
        }    
        hRapCh->sGfxSettings.sStartGfxSettings = *pSettings;
#else
    BSTD_UNUSED(hRapCh);
    BSTD_UNUSED(pSettings);
#endif    
    end:
        BDBG_LEAVE(BRAP_GFX_SetSettings);    
        return ret;
    }

BERR_Code BRAP_GFX_GetSettings(
    BRAP_GFX_Handle              hGfxCh,        /* [in] Gfx channel handle */
    BRAP_GFX_Settings           *pSettings      /* [out] Current settings */
    )
{
    BERR_Code ret = BERR_SUCCESS;           
    BRAP_ChannelHandle hRapCh=NULL;        


    BDBG_ENTER(BRAP_GFX_GetSettings);
    BDBG_ASSERT(hGfxCh);

    if(false == BRAP_FWDWNLD_P_IsGfxSupported(0))
    {
        BDBG_ERR(("Please export RAP_GFX_SUPPORT=y at the compile time for SOft Gfx"));
        ret = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto end;
    }

#ifdef RAP_GFX_SUPPORT        
    hRapCh = (BRAP_ChannelHandle)hGfxCh;

    *pSettings = hRapCh->sGfxSettings.sStartGfxSettings;
    
#else
BSTD_UNUSED(hRapCh);
BSTD_UNUSED(pSettings);
#endif    
end:
    BDBG_LEAVE(BRAP_GFX_GetSettings);    
    return ret;
}

