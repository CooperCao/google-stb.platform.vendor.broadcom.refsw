/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ******************************************************************************/

/*
    SID Power Management
*/
#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#include "bsid.h"
#include "bsid_fw_load.h"
#include "bsid_priv.h"
#include "bsid_power.h"
#include "bsid_platform.h"

BDBG_MODULE(BSID_PWR);

BERR_Code BSID_P_PowerInit(BSID_Handle hSid)
{
   BERR_Code retCode = BERR_SUCCESS;
   /* NOTE: id = 0 indicates resource not supported */
   BKNI_Memset(hSid->PowerResources, 0, sizeof(hSid->PowerResources));

#ifdef BCHP_PWR_SUPPORT
   /* NOTE:Some chips (e.g. 7231) do NOT have a SRAM resource! */
#ifdef BCHP_PWR_RESOURCE_SID_SRAM
   hSid->PowerResources[BSID_P_ResourceType_ePower].id = BCHP_PWR_RESOURCE_SID_SRAM;
#endif
#ifdef BCHP_PWR_RESOURCE_SID
   hSid->PowerResources[BSID_P_ResourceType_eClock].id = BCHP_PWR_RESOURCE_SID;
#endif
#endif
   return BERR_TRACE(retCode);
}

bool BSID_P_Power_SupportsResource(BSID_Handle hSid, BSID_P_ResourceType eResourceType)
{
   return (hSid->PowerResources[eResourceType].id != 0);
}

BERR_Code BSID_P_Power_AcquireResource(BSID_Handle hSid, BSID_P_ResourceType eResourceType)
{
   BERR_Code retCode = BERR_SUCCESS;
#ifndef BCHP_PWR_SUPPORT
   BSTD_UNUSED(hSid);
   BSTD_UNUSED(eResourceType);
#endif
   BDBG_ENTER(BSID_P_Power_AcquireResource);

#ifdef BCHP_PWR_SUPPORT
   if (eResourceType >= BSID_P_ResourceType_eLast)
   {
      BDBG_MSG(("Attempt to acquire an invalid resource type: %d", eResourceType));
      retCode = BERR_TRACE(BERR_NOT_SUPPORTED);
   }
   else
   {
      if (BSID_P_Power_SupportsResource(hSid, eResourceType))
      {
         retCode = BCHP_PWR_AcquireResource(hSid->hChp, hSid->PowerResources[eResourceType].id);
         BDBG_MSG(("Acquiring resource: type: %d, id: %d", eResourceType, hSid->PowerResources[eResourceType].id));
      }
      /* Do nothing if the specified resource is not supported (not all chips have all the resources) */
   }
#endif
   /* Do nothing if BCHP_PWR not supported */
   BDBG_LEAVE(BSID_P_Power_AcquireResource);
   return BERR_TRACE(retCode);
}

BERR_Code BSID_P_Power_ReleaseResource(BSID_Handle hSid, BSID_P_ResourceType eResourceType)
{
   BERR_Code retCode = BERR_SUCCESS;
#ifndef BCHP_PWR_SUPPORT
   BSTD_UNUSED(hSid);
   BSTD_UNUSED(eResourceType);
#endif
   BDBG_ENTER(BSID_P_Power_ReleaseResource);

#ifdef BCHP_PWR_SUPPORT
   if (eResourceType >= BSID_P_ResourceType_eLast)
   {
      BDBG_MSG(("Attempt to release an invalid resource type: %d", eResourceType));
      retCode = BERR_TRACE(BERR_NOT_SUPPORTED);
   }
   else
   {
      if (BSID_P_Power_SupportsResource(hSid, eResourceType))
      {
         retCode = BCHP_PWR_ReleaseResource(hSid->hChp, hSid->PowerResources[eResourceType].id);
         BDBG_MSG(("Releasing resource: type: %d, id: %d", eResourceType, hSid->PowerResources[eResourceType].id));
      }
      /* Do nothing if the specified resource is not supported (not all chips have all the resources) */
   }
#endif
   /* Do nothing if BCHP_PWR not supported */
   BDBG_LEAVE(BSID_P_Power_ReleaseResource);
   return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_P_Standby
*
* Comments:
*     NOTE: This is called internally by Close()
******************************************************************************/
BERR_Code BSID_P_Standby(BSID_Handle hSid)
{
    BERR_Code retCode = BERR_SUCCESS;
    BERR_Code failureCode = BERR_SUCCESS;

    BDBG_ENTER( BSID_P_Standby );

    if (hSid->bStandby)
    {
       BDBG_WRN(("Already in Standby"));
       BDBG_LEAVE(BSID_P_Standby);
       return BERR_SUCCESS;
    }

#ifdef BSID_P_CLOCK_CONTROL
    retCode = BSID_P_Power_AcquireResource(hSid, BSID_P_ResourceType_eClock);
#endif
    if (retCode == BERR_SUCCESS)
    {
       /* NOTE: This function will attempt to continue until the end, attempting
          all steps as required.  */

       /* NOTE: cant suspend channels if watchdog - suspend channels sends sync then close
         commands to the firmware, which is invalid if firmware not responding */
       if (!hSid->bWatchdogOccurred)
       {
         retCode = BSID_P_SuspendChannels(hSid);
         if (retCode != BERR_SUCCESS)
         {
             BDBG_ERR(("Standby: BSID_SuspendChannels failed with error 0x%x", retCode));
             if (BERR_SUCCESS == failureCode)
                failureCode = retCode;
         }
       }

       BSID_P_HaltArc(hSid->hReg);

       /* callback must be disabled and re-enabled */
       retCode = BINT_DisableCallback(hSid->hServiceIsr);
       if (retCode != BERR_SUCCESS)
       {
           BDBG_ERR(("Standby: BINT_DisableCallback returned with error 0x%x", retCode));
           if (BERR_SUCCESS == failureCode)
                failureCode = retCode;
       }
       retCode = BSID_P_Power_ReleaseResource(hSid, BSID_P_ResourceType_eClock);
       if (retCode != BERR_SUCCESS)
       {
          BDBG_ERR(("Standby: Unable to release Clock Resource"));
          if (BERR_SUCCESS == failureCode)
                failureCode = retCode;
       }
    }
    else
    {
        BDBG_ERR(("Standby: Unable to Acquire Clock Resource"));
        failureCode = retCode;
    }

    /* release power resources */
    retCode = BSID_P_Power_ReleaseResource(hSid, BSID_P_ResourceType_ePower);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("Standby: Unable to release Power Resource"));
        if (BERR_SUCCESS == failureCode)
           failureCode = retCode;
    }

    if (BERR_SUCCESS == failureCode)
       hSid->bStandby = true;

    BDBG_LEAVE( BSID_P_Standby );
    return BERR_TRACE(failureCode);
}

/******************************************************************************
* Function name: BSID_P_Resume
*
* Comments:
*    NOTE: This is called internally by Open()
******************************************************************************/
BERR_Code BSID_P_Resume(BSID_Handle hSid)
{
    BERR_Code retCode = BERR_SUCCESS;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BSID_P_Resume );

    if (!hSid->bStandby)
    {
        BDBG_WRN(("Resume: Not Suspended!"));
        BDBG_LEAVE(BSID_P_Resume);
        return BERR_SUCCESS;
    }

    /* NOTE: since this is an "open" type API, this does NOT need to continue
       all steps upon failure - if it fails to resume due to any step failure
       we treat the entire resume as failed, and it simply releases all resource
       (i.e. leaves the firmware disabled) */

    /* acquire power resources */
    retCode = BSID_P_Power_AcquireResource(hSid, BSID_P_ResourceType_eClock);
    if (BERR_SUCCESS != retCode)
    {
        /* if we can't acquire the clock, then were done - cant resume so dont do anything */
        BDBG_ERR(("Resume: Unable to Acquire Clock Resource"));
        BDBG_LEAVE(BSID_P_Resume);
        return BERR_SUCCESS;
    }

    retCode = BSID_P_Power_AcquireResource(hSid, BSID_P_ResourceType_ePower);
    if (BERR_SUCCESS != retCode)
    {
        BDBG_ERR(("Resume: Unable to Acquire Power Resource"));
        goto errorClock;  /* release clock */
    }
    /* NOTE: from this point on, if fail any step then release power and clock */

    /* Clear the DMA info to avoid "sticky" abort status */
    BSID_P_ResetDmaInfo(hSid);

    retCode = BSID_P_BootArc(hSid);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("Resume: BSID_P_BootArc failed"));
        goto errorPower;
    }

    retCode = BINT_EnableCallback(hSid->hServiceIsr);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("Resume: BINT_EnableCallback failed"));
        goto errorPower;
    }

#if 0
    /* SWSTB-379 - enable this when FWAVD-787 is complete */
    retCode = BINT_EnableCallback(hSid->hWatchdogIsr);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("Resume: BINT_EnableCallback failed"));
        goto errorPower;
    }
#endif

    retCode = BSID_P_SendCmdInit(hSid);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("Resume: BSID_P_SendCmdInit failed"));
        goto errorPower;
    }

    retCode = BSID_P_ResumeActiveChannels(hSid);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BSID_P_ResumeActiveChannels failed with error 0x%x", retCode));
        goto errorPower;
    }

    /* NOTE: we can release this here since SendCmdInit and SendCmdOpenChannels
       (called from ResumeChannel) are blocking calls */
#ifdef BSID_P_CLOCK_CONTROL
    retCode = BSID_P_Power_ReleaseResource(hSid, BSID_P_ResourceType_eClock);
    if (BERR_SUCCESS != retCode)
    {
        BDBG_ERR(("Resume: Unable to release Clock Resource"));
        goto errorPower;
    }
#endif

    hSid->bStandby = false;
    BDBG_LEAVE( BSID_P_Resume );
    return BERR_SUCCESS;

errorPower:
    rc = BSID_P_Power_ReleaseResource(hSid, BSID_P_ResourceType_ePower);
    if (BERR_SUCCESS != rc)
    {
       BDBG_ERR(("Resume: Unable to release Power Resource"));
    }

errorClock:
    rc = BSID_P_Power_ReleaseResource(hSid, BSID_P_ResourceType_eClock);
    if (BERR_SUCCESS != rc)
    {
       BDBG_ERR(("Resume: Unable to release Clock Resource"));
    }

    BDBG_LEAVE( BSID_P_Resume );
    return BERR_TRACE(retCode);
}

/***********************************
         End of File
************************************/