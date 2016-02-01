/**************************************************************************
 *     (c)2005-2014 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
#include "bads.h"
#include "bads_priv.h"
#include "btmr.h"
#include "bmth.h"
#include "bkni.h"
#include "bads_def.h"
#include "bads_3x7x_priv.h"
#include "bads_acquire.h"
#include "bads_status.h"
#include "bads_utils.h"
#include "bchp_ds_topm.h"
#include "bchp_ds_tops.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_int_id_ds.h"
#include "bads_global_clk.h"
#include "bads_ver.h"
#include "bads_mth.h"
#if ((BCHP_CHIP==7552) || (BCHP_CHIP==35230)  || (BCHP_CHIP==35233))
#include "bchp_int_id_ds_topm.h"
#endif
#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr_resources.h" 
#include "bchp_pwr.h"
#endif

BERR_Code BADS_P_ResetDSCore(BADS_3x7x_ChannelHandle hImplChnDev);

BDBG_MODULE(bads_3x7x_priv);

/*******************************************************************************
*   BADS_P_3x7x_Isr
*******************************************************************************/

void BADS_P_3x7x_Isr(void *p, int param)
{
  BADS_3x7x_ChannelHandle pHandle = (BADS_3x7x_ChannelHandle)p;
  uint32_t RegStatus, RegMaskStatus;
  BSTD_UNUSED(param);

  RegStatus = BREG_Read32(pHandle->hRegister, BCHP_DS_IRQSTS2);
  RegMaskStatus = BREG_Read32(pHandle->hRegister, BCHP_DS_IRQMSK2);

  if ((RegStatus)& ~(RegMaskStatus))
  {
    RegStatus &= ~RegMaskStatus;
#if (BADS_P_BCHP_DS_CORE_VER == BADS_P_BCHP_DS_CORE_V_9_1)
    if (RegStatus & BCHP_DS_IRQSTS2_FCA_IRQ_MASK)
    {
        BREG_Write32(pHandle->hRegister, BCHP_DS_IRQCLR2, BCHP_DS_IRQCLR2_FCA_ICLR_MASK);
        BKNI_SetEvent(pHandle->hFcaEvent);
    }
    else if (RegStatus & BCHP_DS_IRQSTS2_CHSCN_IRQ_MASK)
    {
        BREG_Write32(pHandle->hRegister, BCHP_DS_IRQCLR2, BCHP_DS_IRQCLR2_CHSCN_ICLR_MASK);
        BKNI_SetEvent(pHandle->hChscnEvent);
    }
#elif (BADS_P_BCHP_DS_CORE_VER >= BADS_P_BCHP_DS_CORE_V_9_2)
    if (RegStatus & BCHP_DS_IRQCLR2_FFT_ICLR_MASK)
    {
        BKNI_SetEvent(pHandle->hFftEvent);
        BREG_Write32(pHandle->hRegister, BCHP_DS_IRQCLR2, BCHP_DS_IRQCLR2_FFT_ICLR_MASK);
    }
#endif  
  }
}


/*******************************************************************************
*   BADS_P_TimerFunc
*******************************************************************************/
BERR_Code BADS_P_TimerFunc(void *myParam1, int myParam2)
{

    BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_Handle hImplDev;
    BADS_3x7x_ChannelHandle h;
    BADS_Handle hDev;
    BSTD_UNUSED(myParam2);

    BDBG_ENTER(BADS_P_TimerFunc);
    hDev = (BADS_Handle) myParam1;
    hImplDev = hDev->pImpl;
    BDBG_ASSERT( hImplDev );
    h = (BADS_3x7x_ChannelHandle)(hImplDev->hAdsChn[0]);
    BDBG_ASSERT( h );
  
    BKNI_ResetEvent(hImplDev->hIntEvent);
    BADS_P_ChnLockStatus(h);
    if ( h->pCallback[BADS_Callback_eLockChange] != NULL )
    {
            if (h->pChnLockStatus->FLK_te == 1)
            {
                if (h->LockUpdate)
                {
                    h->LockUpdate = false;
                    (h->pCallback[BADS_Callback_eLockChange])(h->pCallbackParam[BADS_Callback_eLockChange] );
                }
            }
            else
            {
                if (h->lock_status != h->pChnLockStatus->FLK_te )
                {
                    h->LockUpdate = true;
                   (h->pCallback[BADS_Callback_eLockChange])(h->pCallbackParam[BADS_Callback_eLockChange] );
                   h->pChnLockStatus->ReAck_Count_u32 = 0;
                }
            }
    }
    h->lock_status = h->pChnLockStatus->FLK_te;

    BMEM_FlushCache(hDev->settings.hHeap, h->pChnAcqParam, sizeof( BADS_3x7x_AcqParams_t ) );
    if (h->pChnAcqParam->BADS_Acquire_Params.NexusStatusMode & BADS_NexusStatusMode_EnableStatusForNexus)
    {
        BMEM_FlushCache(hDev->settings.hHeap, h->pChnStatus, sizeof( BADS_3x7x_ChnStatus_t ) );
        BADS_P_ChnStatus(h);
        h->pChnAcqParam->BADS_Acquire_Params.NexusStatusMode &= (~BADS_NexusStatusMode_EnableStatusForNexus);
    }

    if (h->pChnAcqParam->BADS_Acquire_Params.AcquireStartMode & BADS_AcquireStartMode_ResetStatus)
    {
        BDBG_MSG(("call reset"));
        BMEM_FlushCache(hDev->settings.hHeap, h->pChnStatus, sizeof( BADS_3x7x_ChnStatus_t ) );
        BADS_P_ChnStatusReset(h);
        h->pChnAcqParam->BADS_Acquire_Params.AcquireStartMode &= (~BADS_AcquireStartMode_ResetStatus);
    }

    if (h->pChnAcqParam->BADS_Acquire_Params.AcquireStartMode & BADS_AcquireStartMode_Acquire)
    {
        h->pChnAcqParam->BADS_Acquire_Params.AcquireStartMode &= ~BADS_AcquireStartMode_Acquire;
        h->pChnLockStatus->ReAck_Count_u32 = 0;
        BDBG_MSG(("call1 BADS_3x7x_ProcessInterruptEvent"));
        BKNI_SetEvent(hImplDev->hIntEvent);
        return BERR_SUCCESS;
    }
    else
    {
        if (h->pChnAcqParam->BADS_Acquire_Params.Auto_te == BADS_Acquire_Params_eEnable)
        {
            if (h->lock_status == 0)
            {
                BDBG_MSG(("call  BADS_3x7x_ProcessInterruptEvent in AUTO mode"));
                BKNI_SetEvent(hImplDev->hIntEvent);
            }
        }
    }

    BDBG_LEAVE(BADS_P_TimerFunc);
    return retCode;
}


/*******************************************************************************
*   BADS_3x7x_Open
*******************************************************************************/
BERR_Code BADS_3x7x_Open(
    BADS_Handle *pAds,                          /* [out] Returns handle */
    BCHP_Handle hChip,                          /* [in] Chip handle */
    BREG_Handle hRegister,                      /* [in] Register handle */
    BINT_Handle hInterrupt,                     /* [in] Interrupt handle */
    const struct BADS_Settings *pDefSettings    /* [in] Default settings */
    )
{

    BERR_Code retCode = BERR_SUCCESS;
    BADS_Handle hDev;
    uint8_t chnIdx;
    BADS_3x7x_Handle hImplDev = NULL;;
    BDBG_ENTER(BADS_3x7x_Open);

    /* Alloc memory from the system heap */
    hDev = (BADS_Handle) BKNI_Malloc( sizeof( BADS_P_Handle ) );

    if( hDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BADS_Open: BKNI_malloc() failed"));
        goto done;
    }
   
    BKNI_Memset( hDev, 0x00, sizeof( BADS_P_Handle ) );
    hDev->magicId = DEV_MAGIC_ID;
    BKNI_Memcpy(&hDev->settings, pDefSettings, sizeof(*pDefSettings));

    /* Alloc memory from the system heap */
    hImplDev = (BADS_3x7x_Handle) BKNI_Malloc( sizeof( BADS_P_3x7x_Handle ) );
    if( hImplDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BADS_Open: BKNI_malloc() failed"));
        goto done;
    }
    
    BKNI_Memset( hImplDev, 0x00, sizeof( BADS_P_3x7x_Handle ) );
    hImplDev->magicId = DEV_MAGIC_ID;
    hImplDev->hChip = hChip;
    hImplDev->mxChnNo = MX_ADS_CHANNELS;
    hImplDev->hRegister = hRegister;
    hImplDev->hInterrupt = hInterrupt;
#ifdef BCHP_PWR_RESOURCE_DS_TOP
    BCHP_PWR_AcquireResource(hImplDev->hChip, BCHP_PWR_RESOURCE_DS_TOP);
#endif   

    BKNI_CreateEvent(&(hImplDev->hIntEvent));

    for( chnIdx = 0; chnIdx < MX_ADS_CHANNELS; chnIdx++ )
    {
        hImplDev->hAdsChn[chnIdx] = NULL;
    }

    hDev->pImpl = hImplDev;

    *pAds = hDev;
done:
#ifdef BCHP_PWR_RESOURCE_DS_TOP
        BCHP_PWR_ReleaseResource(hImplDev->hChip, BCHP_PWR_RESOURCE_DS_TOP);
#endif 
    if( retCode != BERR_SUCCESS )
    {
        if( hImplDev != NULL )
        {
            BKNI_Free( hImplDev );
        }  
        if( hDev != NULL )
        {
            BKNI_Free( hDev );
        }
        *pAds = NULL;
    }

    return( retCode );
}


/*******************************************************************************
*   BADS_3x7x_Close
*******************************************************************************/
BERR_Code BADS_3x7x_Close(
    BADS_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_Handle hImplDev = (BADS_3x7x_Handle) hDev->pImpl;
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    BDBG_ENTER(BADS_3x7x_Close);
    BKNI_DestroyEvent(hImplDev->hIntEvent);

    hDev->magicId = 0x00;       /* clear it to catch improper use */
    BKNI_Free( (void *) hDev->pImpl  );
    BKNI_Free( (void *) hDev );

    BDBG_LEAVE(BADS_3x7x_Close);
    return( retCode );
}

/*******************************************************************************
*   BADS_3x7x_Init
*******************************************************************************/
BERR_Code BADS_3x7x_Init(
    BADS_Handle hDev                    /* [in] Device handle */
    )
{ 
    BERR_Code retCode = BERR_SUCCESS;
    BDBG_ENTER(BADS_3x7x_Init);
    BSTD_UNUSED(hDev);
    BDBG_LEAVE(BADS_3x7x_Init);
    return( retCode );
}

/*******************************************************************************
*   BADS_3x7x_Init
*******************************************************************************/

BERR_Code BADS_3x7x_GetVersion(
    BADS_Handle hDev,                   /* [in] Device handle */
    BADS_Version *pVersion              /* [out] Returns version */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_Handle hImplDev;
    BADS_Version Version;

    BDBG_ENTER(BADS_3x7x_GetVersion);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_3x7x_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );

    Version.familyId = BREG_Read32(hImplDev->hRegister,  BCHP_SUN_TOP_CTRL_CHIP_FAMILY_ID);
    Version.chipId = BREG_Read32(hImplDev->hRegister, BCHP_SUN_TOP_CTRL_PRODUCT_ID);
    Version.majVer = BADS_COMPONENT_VERSION_MAJOR;
    Version.minVer = BADS_COMPONENT_VERSION_MINOR;
    Version.apVer = BADS_COMPONENT_VERSION_BUILD_TYPE;
    Version.minApVer = BADS_COMPONENT_VERSION_BUILD_ID; 

    *pVersion = Version;     

    BDBG_LEAVE(BADS_3x7x_GetVersion);
    return( retCode );
}


/*******************************************************************************
*   BADS_3x7x_GetTotalChannels
*******************************************************************************/
BERR_Code BADS_3x7x_GetTotalChannels(
    BADS_Handle hDev,                   /* [in] Device handle */
    unsigned int *totalChannels         /* [out] Returns total number downstream channels supported */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_Handle hImplDev;

    BDBG_ENTER(BADS_3x7x_GetTotalChannels);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_3x7x_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );

    *totalChannels = hImplDev->mxChnNo; /* use saved data */

    BDBG_LEAVE(BADS_3x7x_GetTotalChannels);
    return( retCode );
}


/*******************************************************************************
*  Channel Managment level
*  BADS_3x7x_OpenChannel
*******************************************************************************/

BERR_Code BADS_3x7x_OpenChannel(
    BADS_Handle hDev,                                   /* [in] Device handle */
    BADS_ChannelHandle *phChn,                          /* [out] Returns channel handle */
    unsigned int channelNo,                             /* [in] Channel number to open */
    const struct BADS_ChannelSettings *pChnDefSettings  /* [in] Channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_Handle hImplDev;
    BADS_3x7x_ChannelHandle hImplChnDev = NULL;
    BADS_ChannelHandle hChnDev;
    uint32_t BufSrc;
    void *cached_ptr, *tmpAddress;

    BDBG_ENTER(BADS_3x7x_OpenChannel);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_3x7x_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );
    hChnDev = NULL;
    if( channelNo < hImplDev->mxChnNo )
    {
        if( hImplDev->hAdsChn[channelNo] == NULL )
        {
            BDBG_MSG(("BADS_3x7x_OpenChannel"));

            /* Alloc memory from the system heap */
            hChnDev = (BADS_ChannelHandle) BKNI_Malloc( sizeof( BADS_P_ChannelHandle ) );
            if( hChnDev == NULL )
            {
                retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                BDBG_ERR(("BADS_OpenChannel: BKNI_malloc() failed"));
                goto done;
            }
            
            BKNI_Memset( hChnDev, 0x00, sizeof( BADS_P_ChannelHandle ) );
            hChnDev->magicId = DEV_MAGIC_ID;
            hChnDev->hAds = hDev;

            hImplChnDev = (BADS_3x7x_ChannelHandle) BKNI_Malloc( sizeof( BADS_P_3x7x_ChannelHandle ) );
            if( hImplChnDev == NULL )
            {
                retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                BDBG_ERR(("BADS_OpenChannel: BKNI_malloc() failed, impl"));
                goto done;
            }
            
            hImplChnDev->chnNo = channelNo;
            hImplChnDev->hAds = hImplDev;
            hImplChnDev->magicId = DEV_MAGIC_ID;
            hImplChnDev->hChnAds = hChnDev;
            #if (BADS_P_BCHP_DS_CORE_VER == BADS_P_BCHP_DS_CORE_V_9_1)
            #if (BCHP_CHIP!=3461)
            BKNI_CreateEvent(&(hImplChnDev->hChscnEvent));
            BKNI_CreateEvent(&(hImplChnDev->hFcaEvent));
            #endif
            #elif (BADS_P_BCHP_DS_CORE_VER >= BADS_P_BCHP_DS_CORE_V_9_2)
            BKNI_CreateEvent(&(hImplChnDev->hFftEvent));
            #endif

            BINT_CreateCallback( &(hImplChnDev->hDs2Callback), hImplDev->hInterrupt, BCHP_INT_ID_DS0_2_IS, BADS_P_3x7x_Isr, (void*)hImplChnDev, BADS_DS0_2_EVENT );
            BINT_EnableCallback(hImplChnDev->hDs2Callback);
            hImplChnDev->hRegister = hImplDev->hRegister;

            tmpAddress = (BADS_3x7x_ChnStatus_t *)BMEM_AllocAligned(hDev->settings.hHeap, sizeof(BADS_3x7x_ChnStatus_t), 0, 0 );
            if( tmpAddress == NULL )
            {
                retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                BDBG_ERR(("BADS_Open: BKNI_malloc() failed"));
                goto done;
            }
            BMEM_ConvertAddressToCached(hDev->settings.hHeap, tmpAddress, &cached_ptr);
            BKNI_Memset(cached_ptr, 0x00, sizeof( BADS_3x7x_ChnStatus_t ) );
            hImplChnDev->pChnStatus = cached_ptr;
            BMEM_FlushCache(hDev->settings.hHeap, hImplChnDev->pChnStatus, sizeof( BADS_3x7x_ChnStatus_t ) );
            BMEM_ConvertAddressToOffset(hDev->settings.hHeap, tmpAddress, &BufSrc );
            BREG_Write32(hImplDev->hRegister, BCHP_DS_TOPM_SW_SPARE1, BufSrc);

            tmpAddress = (BADS_3x7x_ChnScanStatus_t *)BMEM_AllocAligned(hDev->settings.hHeap, sizeof(BADS_3x7x_ChnScanStatus_t), 0, 0 );
            if( tmpAddress == NULL )
            {
                retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                BDBG_ERR(("BADS_Open: BKNI_malloc() failed"));
                goto done;
            }
            BMEM_ConvertAddressToCached(hDev->settings.hHeap, tmpAddress, &cached_ptr);
            BKNI_Memset(cached_ptr, 0x00, sizeof( BADS_3x7x_ChnScanStatus_t ) );
            hImplChnDev->pChnScanStatus = cached_ptr;
            BMEM_FlushCache(hDev->settings.hHeap, hImplChnDev->pChnScanStatus, sizeof( BADS_3x7x_ChnScanStatus_t ));
         
            tmpAddress = (BADS_3x7x_AcqParams_t *)BMEM_AllocAligned(hDev->settings.hHeap, sizeof(BADS_3x7x_AcqParams_t), 0, 0 );
            if( tmpAddress == NULL )
            {
                retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                BDBG_ERR(("BADS_OpenChannel: BMEM_AllocAligned() failed"));
                goto done;
            }
            BMEM_ConvertAddressToCached(hDev->settings.hHeap, tmpAddress, &cached_ptr);
            BKNI_Memset(cached_ptr, 0x00, sizeof( BADS_3x7x_AcqParams_t ) );
            hImplChnDev->pChnAcqParam = cached_ptr;
            BMEM_FlushCache(hDev->settings.hHeap, hImplChnDev->pChnAcqParam, sizeof( BADS_3x7x_AcqParams_t ));
            BMEM_ConvertAddressToOffset(hDev->settings.hHeap, tmpAddress, &BufSrc );
            BREG_Write32(hImplDev->hRegister, BCHP_DS_TOPM_SW_SPARE2, BufSrc);

            tmpAddress = (BADS_3x7x_ChnLockStatus_t *)BMEM_AllocAligned(hDev->settings.hHeap, sizeof(BADS_3x7x_ChnLockStatus_t), 0, 0 );
            if( tmpAddress == NULL )
            {
                retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                BDBG_ERR(("BADS_OpenChannel: BMEM_AllocAligned() failed"));
                goto done;
            }
            BMEM_ConvertAddressToCached(hDev->settings.hHeap, tmpAddress, &cached_ptr);
            BKNI_Memset(cached_ptr, 0x00, sizeof( BADS_3x7x_ChnLockStatus_t ) );
            hImplChnDev->pChnLockStatus = cached_ptr;
            BMEM_FlushCache(hDev->settings.hHeap, hImplChnDev->pChnLockStatus, sizeof( BADS_3x7x_ChnLockStatus_t ));
            BMEM_ConvertAddressToOffset(hDev->settings.hHeap, tmpAddress, &BufSrc );
            BREG_Write32(hImplDev->hRegister, BCHP_DS_TOPM_SPARE1, BufSrc);

            hImplChnDev->lock_status = 2;   /* init to a value different than 1 and 0 */

            if (pChnDefSettings->autoAcquire == true)
            {
                hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Auto_te = BADS_Acquire_Params_eEnable;
            }
            else
            {
                hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Auto_te = BADS_Acquire_Params_eDisable;
            }

            hImplDev->hAdsChn[channelNo] = hImplChnDev;
            hChnDev->pImpl = hImplChnDev;
            hImplChnDev->ePowerStatus = BADS_eChan_Power_Off;

            *phChn = hChnDev;

        }
        else
        {
            retCode = BERR_TRACE(BADS_ERR_NOTAVAIL_CHN_NO);
        }
    }
    else
    {
        retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
    }

done:
    if( retCode != BERR_SUCCESS )
    {
        if( hImplChnDev->pChnStatus != NULL )
            BMEM_Heap_FreeCached(hDev->settings.hHeap,(void *)(hImplChnDev->pChnStatus) );
        if( hImplChnDev->pChnAcqParam != NULL )
            BMEM_Heap_FreeCached(hDev->settings.hHeap ,(void *)(hImplChnDev->pChnAcqParam) );
        if( hImplChnDev->pChnLockStatus != NULL )
            BMEM_Heap_FreeCached(hDev->settings.hHeap,(void *)(hImplChnDev->pChnLockStatus) );
        if( hImplChnDev->pChnScanStatus != NULL )
            BMEM_Heap_FreeCached(hDev->settings.hHeap,(void *)(hImplChnDev->pChnScanStatus) );
        if( hImplChnDev != NULL )
            BKNI_Free( hImplChnDev );
        if( hChnDev != NULL )
            BKNI_Free( hChnDev );
        *phChn = NULL;
    }
    BDBG_LEAVE(BADS_3x7x_OpenChannel);
    return( retCode );
}

/*******************************************************************************
*   BADS_3x7x_CloseChannel
*******************************************************************************/
BERR_Code BADS_3x7x_CloseChannel(
    BADS_ChannelHandle hChn  
    )
{
    BADS_Handle hDev;
        BADS_3x7x_Handle hImplDev;

    BADS_3x7x_ChannelHandle hImplChnDev = (BADS_3x7x_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    hDev = hChn->hAds;
    hImplDev = (BADS_3x7x_Handle) hDev->pImpl;

#if (BADS_P_BCHP_DS_CORE_VER == BADS_P_BCHP_DS_CORE_V_9_1)
        BKNI_DestroyEvent(hImplChnDev->hFcaEvent);
        BKNI_DestroyEvent(hImplChnDev->hChscnEvent);
#elif (BADS_P_BCHP_DS_CORE_VER >= BADS_P_BCHP_DS_CORE_V_9_2)
        BKNI_DestroyEvent(hImplChnDev->hFftEvent);
#endif
    BINT_DisableCallback(hImplChnDev->hDs2Callback);
    BINT_DestroyCallback(hImplChnDev->hDs2Callback);
    hImplChnDev->ePowerStatus = BADS_eChan_Power_Off;

    hChn->magicId = 0x00;       /* clear it to catch inproper use */
    BMEM_Heap_FreeCached(hDev->settings.hHeap ,(void *)(hImplChnDev->pChnAcqParam) );
    BMEM_Heap_FreeCached(hDev->settings.hHeap,(void *)(hImplChnDev->pChnLockStatus) );
    BMEM_Heap_FreeCached(hDev->settings.hHeap,(void *)(hImplChnDev->pChnStatus) );
    BMEM_Heap_FreeCached(hDev->settings.hHeap,(void *)(hImplChnDev->pChnScanStatus) );
    hImplDev->hAdsChn[hImplChnDev->chnNo] = NULL;
    BKNI_Free( hChn->pImpl );
    BKNI_Free( hChn );

    return BERR_SUCCESS;
}


/*******************************************************************************
*   BADS_3x7x_GetDevice
*******************************************************************************/
BERR_Code BADS_3x7x_GetDevice(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_Handle *phDev                  /* [out] Returns Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BADS_3x7x_GetDevice);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    *phDev = hChn->hAds;

    BDBG_LEAVE(BADS_3x7x_GetDevice);
    return( retCode );
}


/*******************************************************************************
*   BADS_3x7x_SetScanParam
*******************************************************************************/
BERR_Code BADS_3x7x_SetScanParam(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_ChannelScanSettings *pChnScanSettings                 
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_ChannelHandle hImplChnDev;

    BDBG_ENTER(BADS_3x7x_SetScanParam);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    hImplChnDev = (BADS_3x7x_ChannelHandle) hChn->pImpl;

    hImplChnDev->pChnAcqParam->BADS_Scan_Params.AI_te = pChnScanSettings->AI;
    hImplChnDev->pChnAcqParam->BADS_Scan_Params.QM_te = pChnScanSettings->QM;
    hImplChnDev->pChnAcqParam->BADS_Scan_Params.CO_te = pChnScanSettings->CO;
    hImplChnDev->pChnAcqParam->BADS_Scan_Params.TO_te = pChnScanSettings->TO;
    hImplChnDev->pChnAcqParam->BADS_Scan_Params.B1024_te = pChnScanSettings->B1024;
    hImplChnDev->pChnAcqParam->BADS_Scan_Params.B256_te = pChnScanSettings->B256;
    hImplChnDev->pChnAcqParam->BADS_Scan_Params.B64_te = pChnScanSettings->B64;
    hImplChnDev->pChnAcqParam->BADS_Scan_Params.A1024_te = pChnScanSettings->A1024;
    hImplChnDev->pChnAcqParam->BADS_Scan_Params.A512_te = pChnScanSettings->A512;
    hImplChnDev->pChnAcqParam->BADS_Scan_Params.A256_te = pChnScanSettings->A256;
    hImplChnDev->pChnAcqParam->BADS_Scan_Params.A128_te = pChnScanSettings->A128;
    hImplChnDev->pChnAcqParam->BADS_Scan_Params.A64_te = pChnScanSettings->A64;
    hImplChnDev->pChnAcqParam->BADS_Scan_Params.A32_te = pChnScanSettings->A32;
    hImplChnDev->pChnAcqParam->BADS_Scan_Params.A16_te = pChnScanSettings->A16;
    hImplChnDev->pChnAcqParam->BADS_Scan_Params.Carrier_Search_u32 = pChnScanSettings->carrierSearch;
    hImplChnDev->pChnAcqParam->BADS_Scan_Params.Upper_Baud_Search_u32 = pChnScanSettings->upperBaudSearch;
    hImplChnDev->pChnAcqParam->BADS_Scan_Params.Lower_Baud_Search_u32 = pChnScanSettings->lowerBaudSearch;

    BDBG_LEAVE(BADS_3x7x_SetScanParam);
    return( retCode );
}



/*******************************************************************************
*   BADS_3x7x_GetScanParam
*******************************************************************************/
BERR_Code BADS_3x7x_GetScanParam(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_ChannelScanSettings *pChnScanSettings                 
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_ChannelHandle hImplChnDev;

    BDBG_ENTER(BADS_3x7x_GetScanParam);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    hImplChnDev = (BADS_3x7x_ChannelHandle) hChn->pImpl;

    pChnScanSettings->AI = hImplChnDev->pChnAcqParam->BADS_Scan_Params.AI_te ;
    pChnScanSettings->QM = hImplChnDev->pChnAcqParam->BADS_Scan_Params.QM_te ;
    pChnScanSettings->CO = hImplChnDev->pChnAcqParam->BADS_Scan_Params.CO_te ;
    pChnScanSettings->TO = hImplChnDev->pChnAcqParam->BADS_Scan_Params.TO_te ;
    pChnScanSettings->B1024 = hImplChnDev->pChnAcqParam->BADS_Scan_Params.B1024_te ;
    pChnScanSettings->B256 = hImplChnDev->pChnAcqParam->BADS_Scan_Params.B256_te ;
    pChnScanSettings->B64 = hImplChnDev->pChnAcqParam->BADS_Scan_Params.B64_te ;
    pChnScanSettings->A1024 = hImplChnDev->pChnAcqParam->BADS_Scan_Params.A1024_te ;
    pChnScanSettings->A512 = hImplChnDev->pChnAcqParam->BADS_Scan_Params.A512_te ;
    pChnScanSettings->A256 = hImplChnDev->pChnAcqParam->BADS_Scan_Params.A256_te ;
    pChnScanSettings->A128 = hImplChnDev->pChnAcqParam->BADS_Scan_Params.A128_te ;
    pChnScanSettings->A64 = hImplChnDev->pChnAcqParam->BADS_Scan_Params.A64_te ;
    pChnScanSettings->A32 = hImplChnDev->pChnAcqParam->BADS_Scan_Params.A32_te ;
    pChnScanSettings->A16 = hImplChnDev->pChnAcqParam->BADS_Scan_Params.A16_te ;
    pChnScanSettings->carrierSearch = hImplChnDev->pChnAcqParam->BADS_Scan_Params.Carrier_Search_u32 ;
    pChnScanSettings->upperBaudSearch = hImplChnDev->pChnAcqParam->BADS_Scan_Params.Upper_Baud_Search_u32 ;
    pChnScanSettings->lowerBaudSearch = hImplChnDev->pChnAcqParam->BADS_Scan_Params.Lower_Baud_Search_u32 ;

    BDBG_LEAVE(BADS_3x7x_GetScanParam);
    return( retCode );
}
 

/*******************************************************************************
*   BADS_3x7x_SetAcquireParams
*******************************************************************************/

BERR_Code BADS_3x7x_SetAcquireParams(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    const BADS_InbandParam *ibParam           /* [in] Inband Parameters to use */
    )
{
BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_ChannelHandle hImplChnDev;
    BADS_Handle hDev;
    BADS_3x7x_Handle hImplDev;

    BDBG_ENTER(BADS_3x7x_SetAcquireParams);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_3x7x_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    hDev = (BADS_Handle)(hChn->hAds);
    BDBG_ASSERT( hDev );
    hImplDev = (BADS_3x7x_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );
    BDBG_MSG(("BADS_3x7x_SetAcquireParams"));


    switch(ibParam->modType)
    {
        case BADS_ModulationType_eAnnexAQam16:
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Annex_te = BADS_Acquire_Params_Annex_eAnnexA;
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Qam_Mode_te = BADS_Acquire_Params_BPS_eQam16;
             break;
        case BADS_ModulationType_eAnnexAQam32:
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Annex_te = BADS_Acquire_Params_Annex_eAnnexA;
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Qam_Mode_te = BADS_Acquire_Params_BPS_eQam32;
             break;
        case BADS_ModulationType_eAnnexAQam64:
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Annex_te = BADS_Acquire_Params_Annex_eAnnexA;
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Qam_Mode_te = BADS_Acquire_Params_BPS_eQam64;
             break;
        case BADS_ModulationType_eAnnexAQam128:
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Annex_te = BADS_Acquire_Params_Annex_eAnnexA;
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Qam_Mode_te = BADS_Acquire_Params_BPS_eQam128;
             break;
        case BADS_ModulationType_eAnnexAQam256:
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Annex_te = BADS_Acquire_Params_Annex_eAnnexA;
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Qam_Mode_te  = BADS_Acquire_Params_BPS_eQam256;
             break;
        case BADS_ModulationType_eAnnexAQam1024:
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Annex_te = BADS_Acquire_Params_Annex_eAnnexA;
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Qam_Mode_te = BADS_Acquire_Params_BPS_eQam1024;
             break;
        case BADS_ModulationType_eAnnexBQam16:
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Annex_te = BADS_Acquire_Params_Annex_eAnnexA;
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Qam_Mode_te = BADS_Acquire_Params_BPS_eQam16;
             break;
        case BADS_ModulationType_eAnnexBQam32:
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Annex_te = BADS_Acquire_Params_Annex_eAnnexB;
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Qam_Mode_te = BADS_Acquire_Params_BPS_eQam32;
             break;
        case BADS_ModulationType_eAnnexBQam64:
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Annex_te = BADS_Acquire_Params_Annex_eAnnexB;
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Qam_Mode_te = BADS_Acquire_Params_BPS_eQam64;
             break;
        case BADS_ModulationType_eAnnexBQam128:
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Annex_te = BADS_Acquire_Params_Annex_eAnnexB;
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Qam_Mode_te = BADS_Acquire_Params_BPS_eQam128;
             break;
        case BADS_ModulationType_eAnnexBQam256:
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Annex_te = BADS_Acquire_Params_Annex_eAnnexB;
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Qam_Mode_te = BADS_Acquire_Params_BPS_eQam256;
             break;
        case BADS_ModulationType_eAnnexBQam1024:
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Annex_te = BADS_Acquire_Params_Annex_eAnnexB;
             hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Qam_Mode_te = BADS_Acquire_Params_BPS_eQam1024;
             break;
        default:
            BDBG_WRN(("BADS_3x7x_SetAcquireParams: QAM mode not supported"));
            break;

    }
    hImplChnDev->pChnAcqParam->BADS_Acquire_Params.AnnexA_Sym_Rate_u32 = ibParam->symbolRate;
    hImplChnDev->pChnAcqParam->BADS_Acquire_Params.IS_te = ibParam->invertSpectrum;
    hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Auto_te = ibParam->autoAcquire;
    hImplChnDev->pChnAcqParam->BADS_Acquire_Params.AcqType_te = ibParam->acquisitionType;
    hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Carrier_Range_u32 = ibParam->frequencyOffset / 256; /* from Hz to 256Hz */
	hImplChnDev->pChnAcqParam->BADS_Acquire_Params.AnnexA_NullXport_te = ibParam->enableNullPackets;

    BDBG_LEAVE(BADS_3x7x_SetAcquireParams);
    return( retCode );

}
/*******************************************************************************
*   BADS_3x7x_Acquire
*******************************************************************************/
BERR_Code BADS_3x7x_Acquire(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_InbandParam *ibParam           /* [in] Inband Parameters to use */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_ChannelHandle hImplChnDev;
    BADS_Handle hDev;
    BADS_3x7x_Handle hImplDev;

    BDBG_ENTER(BADS_3x7x_Acquire);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BSTD_UNUSED(ibParam);

    hImplChnDev = (BADS_3x7x_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    hDev = (BADS_Handle)(hChn->hAds);
    BDBG_ASSERT( hDev );
    hImplDev = (BADS_3x7x_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );
    BDBG_MSG(("BADS_3x7x_Acquire"));

    if (hImplChnDev->ePowerStatus  != BADS_eChan_Power_On)
    {
        BDBG_WRN(("!!!BADS_3x7x_Acquire: chan power not on yet"));
        return BERR_NOT_INITIALIZED;
    }
    BKNI_Memset(hImplChnDev->pChnLockStatus, 0x00, sizeof( BADS_3x7x_ChnLockStatus_t ) ); /* Clear status structure upon new TuneAcquire command from host */
    hImplChnDev->pChnLockStatus->ReAck_Count_u32 = 0;

    BTMR_StopTimer(hImplDev->hTimer);  /* the timer is in Micro second */

    hImplChnDev->pChnStatus->FLK_te = 0;
    hImplChnDev->pChnStatus->QLK_te = 0;
    hImplChnDev->lock_status = 1;
    hImplChnDev->LockUpdate = true;
    BKNI_EnterCriticalSection();
    if ( hImplChnDev->pCallback[BADS_Callback_eLockChange] != NULL )
            (hImplChnDev->pCallback[BADS_Callback_eLockChange])(hImplChnDev->pCallbackParam[BADS_Callback_eLockChange] );
    BKNI_LeaveCriticalSection();
    BADS_P_ResetDSCore(hImplChnDev);
    BADS_P_Acquire(hImplChnDev);
    BTMR_StartTimer(hImplDev->hTimer, 25000);   /* the timer is in Micro second */

    BDBG_LEAVE(BADS_3x7x_Acquire);
    return( retCode );
}


/*******************************************************************************
*   BADS_3x7x_GetStatus
*******************************************************************************/
BERR_Code BADS_3x7x_GetStatus(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_Status *pStatus                /* [out] Returns status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_ChannelHandle hImplChnDev;
    BADS_3x7x_Handle hImplDev;

    BDBG_ENTER(BADS_3x7x_GetStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_3x7x_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    hImplDev = hImplChnDev->hAds;

    if (hImplChnDev->ePowerStatus  != BADS_eChan_Power_On)
    {
        BDBG_WRN(("!!!BADS_3x7x_GetStatus: chan power not on yet"));
        return BERR_NOT_INITIALIZED;
    }

    BADS_P_ChnStatus(hImplChnDev);
  

    pStatus->isPowerSaverEnabled = 0;                                                   /* Enable=1, Disable=0 */

    pStatus->isFecLock = (hImplChnDev->pChnStatus->FLK_te==BADS_3x7x_ChnStatus_eLock)?1:0;  /* lock=1, unlock=0 */
    pStatus->isQamLock = (hImplChnDev->pChnStatus->QLK_te==BADS_3x7x_ChnStatus_eLock)?1:0;  /* lock=1, unlock=0 */
    pStatus->symbolRate = hImplChnDev->pChnStatus->Symbol_Rate_u32;                             /* in Baud */

    if (hImplChnDev->pChnStatus->AB_te == BADS_3x7x_ChnStatus_Annex_eAnnexA)
    {
        switch(hImplChnDev->pChnStatus->BPS_te)
        {
            case BADS_Acquire_Params_BPS_eQam16:
                pStatus->modType = BADS_ModulationType_eAnnexAQam16;
                break;
            case BADS_Acquire_Params_BPS_eQam32:
                pStatus->modType = BADS_ModulationType_eAnnexAQam32;
                break;
            case BADS_Acquire_Params_BPS_eQam64:
                pStatus->modType = BADS_ModulationType_eAnnexAQam64;
                break;
            case BADS_Acquire_Params_BPS_eQam128:
                pStatus->modType = BADS_ModulationType_eAnnexAQam128;
                break;
            case BADS_Acquire_Params_BPS_eQam256:
                pStatus->modType = BADS_ModulationType_eAnnexAQam256;
                break;
            case BADS_Acquire_Params_BPS_eQam512:
                pStatus->modType = BADS_ModulationType_eAnnexAQam512;
                break;
            case BADS_Acquire_Params_BPS_eQam1024:
                pStatus->modType = BADS_ModulationType_eAnnexAQam1024;
                break;
        }
    }
    else if (hImplChnDev->pChnStatus->AB_te == BADS_3x7x_ChnStatus_Annex_eAnnexB)
    {
        switch(hImplChnDev->pChnStatus->BPS_te)
        {
            case BADS_Acquire_Params_BPS_eQam16:
                pStatus->modType = BADS_ModulationType_eAnnexBQam16;
                break;
            case BADS_Acquire_Params_BPS_eQam32:
                pStatus->modType = BADS_ModulationType_eAnnexBQam32;
                break;
            case BADS_Acquire_Params_BPS_eQam64:
                pStatus->modType = BADS_ModulationType_eAnnexBQam64;
                break;
            case BADS_Acquire_Params_BPS_eQam128:
                pStatus->modType = BADS_ModulationType_eAnnexBQam128;
                break;
            case BADS_Acquire_Params_BPS_eQam256:
                pStatus->modType = BADS_ModulationType_eAnnexBQam256;
                break;
            case BADS_Acquire_Params_BPS_eQam512:
                pStatus->modType = BADS_ModulationType_eAnnexBQam512;
                break;
            case BADS_Acquire_Params_BPS_eQam1024:
                pStatus->modType = BADS_ModulationType_eAnnexBQam1024;
                break;
        }
    }
    pStatus->carrierFreqOffset = hImplChnDev->pChnStatus->Carrier_Error_i32/1000;       /* in 1/1000 Hz */
    pStatus->carrierPhaseOffset = hImplChnDev->pChnStatus->Phase_Error_i32/1000;        /* in 1/1000 Hz */
    pStatus->symbolRateError = hImplChnDev->pChnStatus->Symbol_Error_i32;
    pStatus->rxSymbolRate = hImplChnDev->pChnStatus->Symbol_Error_i32 + hImplChnDev->pChnStatus->Symbol_Rate_u32;
    pStatus->snrEstimate = hImplChnDev->pChnStatus->SNRAVG_db_u32;
    pStatus->correctedBits = hImplChnDev->pChnStatus->FEC_Corr_RS_Bits_u32;
    pStatus->berRawCount = hImplChnDev->pChnStatus->BERT_u32;
    pStatus->isSpectrumInverted  = hImplChnDev->pChnStatus->SI_te;
    pStatus->dsChannelPower = ((hImplChnDev->pChnStatus->EstChannelPower_dbm_i32) * 10) / 256;  /* it is in units of 1/10 db. */
    pStatus->symbolRate = hImplChnDev->pChnStatus->Symbol_Rate_u32;                             /* in Baud */
    pStatus->mainTap =  hImplChnDev->pChnStatus->EQ_Main_Tap_i32;       /* Channel main tap coefficient */
    pStatus->equalizerGain = hImplChnDev->pChnStatus->EQ_Gain_db_i32*100;   /* Channel equalizer gain value in 1/100th of dB */
    pStatus->correctedBits = hImplChnDev->pChnStatus->FEC_Corr_RS_Bits_u32;
    pStatus->accCorrectedCount = hImplChnDev->pChnStatus->FEC_Corr_RS_Blocks_u32;
    pStatus->accUncorrectedCount = hImplChnDev->pChnStatus->FEC_UCorr_RS_Blocks_u32;
    pStatus->accCleanCount = hImplChnDev->pChnStatus->FEC_Clean_RS_Blocks_u32;

    pStatus->goodRsBlockCount = 0; /* Not supported */
    pStatus->uncorrectedCount = 0; /* Not supported */
    pStatus->correctedCount = 0; /* Not supported */
    pStatus->ifFreq =  0; /* Not supported */
    pStatus->agcIntLevel = 0;    /* Not supported */                                                                                /* returns 0, moved to tuner*/
    pStatus->agcExtLevel = 0;    /* Not supported */                                                                                /* returns 0, moved to tuner*/
    pStatus->interleaveDepth = 0; /* Not supported */
    pStatus->goodRsBlockCount = 0; /* Not supported */
    pStatus->postRsBER = 0;     /* Not supported */
    pStatus->elapsedTimeSec = 0;    /* Not supported */
    pStatus->cleanCount = 0;        /* Not supported */

    BDBG_LEAVE(BADS_3x7x_GetStatus);
    return( retCode );
}


/*******************************************************************************
*   BADS_3x7x_GetLockStatus
*******************************************************************************/
BERR_Code BADS_3x7x_GetLockStatus(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_LockStatus *pLockStatus         /* [out] Returns lock status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_ChannelHandle hImplChnDev;

    BDBG_ENTER(BADS_3x7x_GetLockStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_3x7x_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );

    if (hImplChnDev->ePowerStatus  != BADS_eChan_Power_On)
    {
        BDBG_WRN(("!!!BADS_3x7x_GetLockStatus: chan power not on yet"));
        return BERR_NOT_INITIALIZED;
    }

    BADS_P_ChnStatus(hImplChnDev);
    if (hImplChnDev->pChnStatus->FLK_te == BADS_3x7x_ChnStatus_eUnlock)
    {
        *pLockStatus = BADS_LockStatus_eUnlocked;
        if ( (hImplChnDev->pChnStatus->AcqStatus_te == BADS_3x7x_ChnStatus_AcqStatus_eNoPower ) ||
             (hImplChnDev->pChnStatus->AcqStatus_te == BADS_3x7x_ChnStatus_AcqStatus_eNoTiming) )
        *pLockStatus = BADS_LockStatus_eNoSignal;
    }
    else
    {
        *pLockStatus = BADS_LockStatus_eLocked;
    }

    BDBG_LEAVE(BADS_3x7x_GetLockStatus);
    return( retCode );

    BDBG_LEAVE(BADS_3x7x_GetLockStatus);
    return( retCode );

}

/*******************************************************************************
*   BADS_3x7x_GetSoftDecision
*******************************************************************************/
BERR_Code BADS_3x7x_GetSoftDecision(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    int16_t nbrToGet,                   /* [in] Number values to get */
    int16_t *iVal,                      /* [out] Ptr to array to store output I soft decision */
    int16_t *qVal,                      /* [out] Ptr to array to store output Q soft decision */
    int16_t *nbrGotten                  /* [out] Number of values gotten/read */
    )
{
    BADS_3x7x_ChannelHandle hImplChnDev;
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(nbrToGet);
    BDBG_ENTER(BADS_3x7x_GetSoftDecision);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_3x7x_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );

    if (hImplChnDev->ePowerStatus  != BADS_eChan_Power_On)
    {
        BDBG_WRN(("!!!BADS_3x7x_GetSoftDecision: chan power not on yet"));
        return BERR_NOT_INITIALIZED;
    }

    BADS_P_Read_Constellation(hImplChnDev, nbrToGet, iVal, qVal, nbrGotten);

    BDBG_LEAVE(BADS_3x7x_GetSoftDecision);
    return( retCode );
}

 
/*******************************************************************************
*   BADS_3x7x_EnablePowerSaver
*******************************************************************************/
BERR_Code BADS_3x7x_EnablePowerSaver(
    BADS_ChannelHandle hChn             /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_ChannelHandle hImplChnDev;

    BADS_Handle hDev;
    BADS_3x7x_Handle hImplDev;

    BDBG_ENTER(BADS_3x7x_EnablePowerSaver);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_3x7x_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    hDev = hChn->hAds;
    hImplDev = (BADS_3x7x_Handle) hDev->pImpl;

    BINT_DisableCallback(hImplChnDev->hDs2Callback);

    if(hImplDev->hTimer)BTMR_StopTimer(hImplDev->hTimer);
    if(hImplDev->hStatusTimer)BTMR_StopTimer(hImplDev->hStatusTimer);

     #if (BADS_P_BCHP_DS_CORE_VER == BADS_P_BCHP_DS_CORE_V_9_1)
    BREG_Write32(hImplChnDev->hRegister, BCHP_DS_PD, 0x1F300023);           /* power down the DS core */
    #elif (BADS_P_BCHP_DS_CORE_VER >= BADS_P_BCHP_DS_CORE_V_9_2)
    BREG_WriteField(hImplChnDev->hRegister, DS_TOPM_PD, DS0_540_DISABLE, 1);        /* power down the DS core */
    #endif

    if(hImplDev->hTimer){
        BTMR_DestroyTimer(hImplDev->hTimer);
        hImplDev->hTimer = NULL;
    }
    if(hImplDev->hStatusTimer){
        BTMR_DestroyTimer(hImplDev->hStatusTimer);
        hImplDev->hStatusTimer = NULL;
    }

    hImplChnDev->ePowerStatus = BADS_eChan_Power_Off;

    BDBG_LEAVE(BADS_3x7x_EnablePowerSaver);
    return( retCode );
}

/*******************************************************************************
*   BADS_3x7x_DisablePowerSaver
*******************************************************************************/
BERR_Code BADS_3x7x_DisablePowerSaver(
    BADS_ChannelHandle hChn             /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_ChannelHandle hImplChnDev;
    BTMR_Settings sTimerSettings;

    BADS_Handle hDev;
    BADS_3x7x_Handle hImplDev;

    BDBG_ENTER(BADS_3x7x_DisablePowerSaver);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_3x7x_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );

    hDev = hChn->hAds;
    hImplDev = (BADS_3x7x_Handle) hDev->pImpl;
    hImplChnDev->pChnAcqParam->BADS_Acquire_Params.Auto_te = BADS_Acquire_Params_eDisable;
    BREG_WriteField(hImplChnDev->hRegister, DS_TOPS_CNTL, FIFO_RESET, 1);
    BREG_WriteField(hImplChnDev->hRegister, DS_TOPS_CNTL, FIFO_RESET, 0);
    #if (BADS_P_BCHP_DS_CORE_VER == BADS_P_BCHP_DS_CORE_V_9_1)
    BREG_Write32(hImplChnDev->hRegister, BCHP_DS_PD, 0x0);          /* power up the DS core */
    #elif (BADS_P_BCHP_DS_CORE_VER >= BADS_P_BCHP_DS_CORE_V_9_2)
    BREG_WriteField(hImplChnDev->hRegister, DS_TOPM_PD, DS0_540_DISABLE, 0);        /* power up the DS core */
    #endif
    BADS_P_ResetDSCore(hImplChnDev);
    /* reset the FE Fifo */
    BREG_WriteField(hImplChnDev->hRegister, DS_TOPS_CNTL, FIFO_RESET, 1);
    BREG_WriteField(hImplChnDev->hRegister, DS_TOPS_CNTL, FIFO_RESET, 0);

    /* Create timer for status lock check */

    sTimerSettings.type = BTMR_Type_ePeriodic;
    sTimerSettings.cb_isr = (BTMR_CallbackFunc)BADS_P_TimerFunc;
    sTimerSettings.pParm1 = (void*)hDev;
    sTimerSettings.parm2 = 0;
    sTimerSettings.exclusive = false;
    retCode = BTMR_CreateTimer (hChn->hAds->settings.hTmr, &hImplDev->hTimer, &sTimerSettings);
    if ( retCode != BERR_SUCCESS )
    {
       BDBG_ERR(("BADS_3x7x_DisablePowerSaver: Create Timer Failed"));
       goto done;
    }

    sTimerSettings.type = BTMR_Type_eStopWatch;
    sTimerSettings.cb_isr = 0;
    sTimerSettings.pParm1 = 0;
    sTimerSettings.parm2 = 0;
    sTimerSettings.exclusive = false;
    retCode = BTMR_CreateTimer (hChn->hAds->settings.hTmr, &hImplDev->hStatusTimer, &sTimerSettings);
    if ( retCode != BERR_SUCCESS )
    {
       BDBG_ERR(("BADS_3x7x_DisablePowerSaver: Create Timer Failed"));
       goto done;
    }
    BTMR_StartTimer(hImplDev->hStatusTimer, POWER2_30/MIPS_TIMER);   /* the timer is in Micro second */;
    BINT_EnableCallback(hImplChnDev->hDs2Callback);

    hImplChnDev->ePowerStatus = BADS_eChan_Power_On;

    BDBG_LEAVE(BADS_3x7x_DisablePowerSaver);
done:
    return( retCode );
}

/*******************************************************************************
*   BADS_3x7x_EnablePowerSaver
*******************************************************************************/
BERR_Code BADS_3x7x_ProcessNotification(
    BADS_ChannelHandle hChn,                /* [in] Device channel handle */
    unsigned int event                      /* [in] Event code and event data*/
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(hChn);
    BSTD_UNUSED(event);

    BDBG_WRN(("Not supported for this frontend chip."));
    return( retCode );
}



/*******************************************************************************
*   BADS_3x7x_InstallCallback
*******************************************************************************/
BERR_Code BADS_3x7x_InstallCallback(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_Callback callbackType,         /* [in] Type of callback */
    BADS_CallbackFunc pCallback,        /* [in] Function Ptr to callback */
    void *pParam                        /* [in] Generic parameter send on callback */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BADS_3x7x_ChannelHandle hImplChnDev = (BADS_3x7x_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
   BDBG_ENTER(BADS_3x7x_InstallCallback);

    switch( callbackType )
    {
        case BADS_Callback_eLockChange:
            hImplChnDev->pCallback[callbackType] = pCallback;
            hImplChnDev->pCallbackParam[callbackType] = pParam;
            break;
        case BADS_Callback_eTuner:
            hImplChnDev->pCallback[callbackType] = pCallback;
            hImplChnDev->pCallbackParam[callbackType] = pParam;
            break;

        default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
    }

    BDBG_LEAVE(BADS_3x7x_InstallCallback);
    return( retCode );
}

/*******************************************************************************
*   BADS_3x7x_SetDaisyChain
*******************************************************************************/
BERR_Code BADS_3x7x_SetDaisyChain(
    BADS_Handle hDev,       /* [in] Returns handle */
    bool enableDaisyChain   /* [in] Eanble/disable daisy chain. */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_Handle hImplDev;


    BDBG_ENTER(BADS_3x7x_SetDaisyChain);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_3x7x_Handle) hDev->pImpl;  
    BDBG_ASSERT( hImplDev );
    
    hImplDev->isDaisyChain = enableDaisyChain;

    BDBG_LEAVE(BADS_3x7x_SetDaisyChain);
    return( retCode );
}
/*******************************************************************************
*   BADS_3x7x_GetDaisyChain
*******************************************************************************/
BERR_Code BADS_3x7x_GetDaisyChain(
    BADS_Handle hDev,           /* [in] Returns handle */
    bool *isEnableDaisyChain    /* [in] Eanble/disable daisy chain. */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_Handle hImplDev;
    
    BDBG_ENTER(BADS_3x7x_GetDaisyChain);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_3x7x_Handle) hDev->pImpl;  
    BDBG_ASSERT( hImplDev );
    
    *isEnableDaisyChain = hImplDev->isDaisyChain;

    BDBG_LEAVE(BADS_3x7x_GetDaisyChain);
    return( retCode );
}

/*******************************************************************************
*   BADS_3x7x_ResetStatus
*******************************************************************************/
BERR_Code BADS_3x7x_ResetStatus(
    BADS_ChannelHandle hChannel           /* [in] Returns handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_ChannelHandle hImplChnDev = (BADS_3x7x_ChannelHandle) hChannel->pImpl;
    BDBG_ASSERT( hChannel );
    BDBG_ASSERT( hChannel->magicId == DEV_MAGIC_ID );
    BDBG_ENTER(BADS_3x7x_ResetStatus);

    if (hImplChnDev->ePowerStatus  != BADS_eChan_Power_On)
    {
        BDBG_WRN(("!!!BADS_3x7x_ResetStatus: chan power not on yet"));
        return BERR_NOT_INITIALIZED;
    }
    if (!(hImplChnDev->pChnAcqParam->BADS_Acquire_Params.NexusStatusMode & BADS_NexusStatusMode_IgnoreNexusChanReset))
    BADS_P_ChnStatusReset(hImplChnDev);

    BDBG_LEAVE(BADS_3x7x_ResetStatus);
    return( retCode );
}


/*******************************************************************************
*   BADS_3x7x_GetInterruptEventHandle
*******************************************************************************/
BERR_Code BADS_3x7x_GetInterruptEventHandle(
   BADS_Handle hDev,            /* [in] BADS handle */
   BKNI_EventHandle *phEvent /* [out] event handle */
)
{

    BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_Handle hImplDev;
    
    BDBG_ENTER(BADS_3x7x_GetInterruptEventHandle);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_3x7x_Handle) hDev->pImpl;  
    BDBG_ASSERT( hImplDev );
    *phEvent = hImplDev->hIntEvent;


    BDBG_LEAVE(BADS_3x7x_GetInterruptEventHandle);
    return( retCode );
}

/*******************************************************************************
*   BADS_3x7x_ProcessInterruptEvent
*******************************************************************************/
BERR_Code BADS_3x7x_ProcessInterruptEvent(
    BADS_Handle hDev           /* [in] Returns handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_Handle hImplDev;
    BADS_3x7x_ChannelHandle hImplChnDev;

    
    BDBG_ENTER(BADS_3x7x_ProcessInterruptEvent);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_3x7x_Handle) hDev->pImpl;  
    BDBG_ASSERT( hImplDev );
    hImplChnDev = (BADS_3x7x_ChannelHandle)(hImplDev->hAdsChn[0]);
    BDBG_MSG(("BADS_3x7x_ProcessInterruptEvent"));

    if (hImplChnDev->ePowerStatus  != BADS_eChan_Power_On)
    {
        BDBG_WRN(("!!!BADS_3x7x_ProcessInterruptEvent: chan power not on yet"));
        return BERR_NOT_INITIALIZED;
    }

    BTMR_StopTimer(hImplDev->hTimer);  /* the timer is in Micro second */
    BKNI_ResetEvent(hImplDev->hIntEvent);
    BADS_P_ResetDSCore(hImplChnDev);
    BADS_P_Acquire(hImplChnDev);
    BTMR_StartTimer(hImplDev->hTimer, 25000);   /* the timer is in Micro second */


    BDBG_LEAVE(BADS_3x7x_ProcessInterruptEvent);
    return( retCode );
}

/*******************************************************************************
*   BADS_3x7x_GetChannelDefaultSettings
*******************************************************************************/
BERR_Code BADS_3x7x_GetChannelDefaultSettings(
    BADS_Handle hDev,                       /* [in] Device handle */
    unsigned int channelNo,                 /* [in] Channel number to default setting for */
    BADS_ChannelSettings *pChnDefSettings   /* [out] Returns channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_Handle hImplDev;

    BDBG_ENTER(BADS_3x7x_GetChannelDefaultSettings);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_3x7x_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );

    switch( channelNo )
    {
        case 0:
        if( channelNo < hImplDev->mxChnNo )
        {
            pChnDefSettings->ifFreq = 0;    /*The if freq for 3x7x is set to 0 as the internal tuner does not spit out seperate if frequency. */
            pChnDefSettings->autoAcquire = true;
            pChnDefSettings->fastAcquire = false;
            break;
        }
        default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
    }

    BDBG_LEAVE(BADS_3x7x_GetChannelDefaultSettings);
    return( retCode );
}

/*******************************************************************************
*   BADS_3x7x_Untune
*******************************************************************************/
BERR_Code BADS_3x7x_Untune(
    BADS_Handle hDev           /* [in] Returns handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_3x7x_Handle hImplDev;

    BDBG_ENTER(BADS_3x7x_Untune);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_3x7x_Handle) hDev->pImpl;  
    BDBG_ASSERT( hImplDev );

    BTMR_StopTimer(hImplDev->hTimer);  /* the timer is in Micro second */
    hImplDev->timerStopped = true;

    BDBG_LEAVE(BADS_3x7x_Untune);
    return( retCode );
}



/*******************************************************************************
*   BADS_GetScanStatus
*******************************************************************************/
BERR_Code BADS_3x7x_GetScanStatus(
    BADS_ChannelHandle hChn,                    /* [in] Device channel handle */
    BADS_ScanStatus *pChnScanStatus       /* [out] Device Scan Status */
    )
{
    BERR_Code eResult = BERR_SUCCESS;
    BADS_3x7x_ChannelHandle hImplChnDev = (BADS_3x7x_ChannelHandle) hChn->pImpl;   
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    switch (hImplChnDev->pChnScanStatus->AcqStatus_te)
    {
        case BADS_3x7x_ChnScanStatus_AcqStatus_eLockedSlow:
                pChnScanStatus->acquisitionStatus = BADS_AcquisitionStatus_eLockedSlow;
                break;
        case BADS_3x7x_ChnScanStatus_AcqStatus_eLockedFast:
                pChnScanStatus->acquisitionStatus = BADS_AcquisitionStatus_eLockedFast;
                break;
        case BADS_3x7x_ChnScanStatus_AcqStatus_eNoFECLock:
                pChnScanStatus->acquisitionStatus = BADS_AcquisitionStatus_eUnlocked;
                break;
        case BADS_3x7x_ChnScanStatus_AcqStatus_eNoCarrier:
                pChnScanStatus->acquisitionStatus = BADS_AcquisitionStatus_eNoSignal;
                break;
        default:
                pChnScanStatus->acquisitionStatus = BADS_AcquisitionStatus_eNoSignal;
                break;
    }

    if (hImplChnDev->pChnScanStatus->Spectrum_te == BADS_3x7x_ChnScanStatus_Spectrum_eNormal)
        pChnScanStatus->isSpectrumInverted = false;
    else
        pChnScanStatus->isSpectrumInverted = true;

    if (hImplChnDev->pChnScanStatus->AB_te == BADS_3x7x_ChnScanStatus_Annex_eAnnexA)
    {
        switch (hImplChnDev->pChnScanStatus->BPS_te)
        {
            case BADS_3x7x_ChnScanStatus_BPS_eQam16:
                pChnScanStatus->modType = BADS_ModulationType_eAnnexAQam16;
                break;
            case BADS_3x7x_ChnScanStatus_BPS_eQam32:
                pChnScanStatus->modType = BADS_ModulationType_eAnnexAQam32;
                break;
            case BADS_3x7x_ChnScanStatus_BPS_eQam64:
                pChnScanStatus->modType = BADS_ModulationType_eAnnexAQam64;
                break;
            case BADS_3x7x_ChnScanStatus_BPS_eQam128:
                pChnScanStatus->modType = BADS_ModulationType_eAnnexAQam128;
                break;
            case BADS_3x7x_ChnScanStatus_BPS_eQam256:
                pChnScanStatus->modType = BADS_ModulationType_eAnnexAQam256;
                break;
            case BADS_3x7x_ChnScanStatus_BPS_eQam512:
                pChnScanStatus->modType = BADS_ModulationType_eAnnexAQam512;
                break;
            case BADS_3x7x_ChnScanStatus_BPS_eQam1024:
                pChnScanStatus->modType = BADS_ModulationType_eAnnexAQam1024;
                break;
            default:
                pChnScanStatus->modType = BADS_ModulationType_eAnnexAQam64;
                break;
        }
    }
    if (hImplChnDev->pChnScanStatus->AB_te == BADS_3x7x_ChnScanStatus_Annex_eAnnexB)
    {
        switch (hImplChnDev->pChnScanStatus->BPS_te)
        {
            case BADS_3x7x_ChnScanStatus_BPS_eQam16:
                pChnScanStatus->modType = BADS_ModulationType_eAnnexBQam16;
                break;
            case BADS_3x7x_ChnScanStatus_BPS_eQam32:
                pChnScanStatus->modType = BADS_ModulationType_eAnnexBQam32;
                break;
            case BADS_3x7x_ChnScanStatus_BPS_eQam64:
                pChnScanStatus->modType = BADS_ModulationType_eAnnexBQam64;
                break;
            case BADS_3x7x_ChnScanStatus_BPS_eQam128:
                pChnScanStatus->modType = BADS_ModulationType_eAnnexBQam128;
                break;
            case BADS_3x7x_ChnScanStatus_BPS_eQam256:
                pChnScanStatus->modType = BADS_ModulationType_eAnnexBQam256;
                break;
            case BADS_3x7x_ChnScanStatus_BPS_eQam512:
                pChnScanStatus->modType = BADS_ModulationType_eAnnexBQam512;
                break;
            case BADS_3x7x_ChnScanStatus_BPS_eQam1024:
                pChnScanStatus->modType = BADS_ModulationType_eAnnexBQam1024;
                break;
            default:
                pChnScanStatus->modType = BADS_ModulationType_eAnnexBQam64;
                break;
        }
    }

    if (hImplChnDev->pChnScanStatus->AcqType_te ==  BADS_3x7x_ChnScanStatus_AcqType_eAuto)
        pChnScanStatus->autoAcquire = true;
    else
        pChnScanStatus->autoAcquire = false;

    pChnScanStatus->symbolRate = hImplChnDev->pChnScanStatus->Symbol_Rate_u32;
    pChnScanStatus->carrierFreqOffset = hImplChnDev->pChnScanStatus->RF_Offset_i32;
    pChnScanStatus->interleaver = hImplChnDev->pChnScanStatus->Interleaver_te;
    return ( eResult );
}


/*******************************************************************************
*   BADS_GetScanStatus
*******************************************************************************/

BERR_Code BADS_P_ResetDSCore(
    BADS_3x7x_ChannelHandle hImplChnDev                   /* [in] Device channel handle */
    )
{
    BERR_Code eResult = BERR_SUCCESS;
    BREG_Write32(hImplChnDev->hRegister, BCHP_DS_TOPM_RST_MASK, 0xFFFFFFFF);            /*Enable the DS core reset  */
    BREG_Write32(hImplChnDev->hRegister, BCHP_DS_TOPM_GBL , 0x1);           /* Enable the DS core reset */
    BREG_Write32(hImplChnDev->hRegister, BCHP_DS_TOPM_GBL , 0x0);           /* Disable the DS core reset */
    BKNI_Delay(20);                                     /* it takes around 10us to be dave I used 20 Us */
    if (BREG_ReadField(hImplChnDev->hRegister, DS_STAT, RST_DATA_N) == 0)
        BDBG_ERR(("*********DS core is not coming out of reset "));
    return ( eResult );
}





