/***************************************************************************
 *     (c)2007-2011 Broadcom Corporation
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
 ****************************************************************************/
#include "b_dvr_datainjectionservice.h"
#include "b_dvr_manager.h"
#include "b_dvr_manager_priv.h"

BDBG_MODULE(b_dvr_datainjectionservice);
BDBG_OBJECT_ID(B_DVR_DataInjectionService);
#define PSI_DATA_DUMP 0
/*
 * MPEG2 transport header definitions in little endian format used 
 * for packing the PSI table data into MPEG2 TS format
 */
#define B_DVR_TS_HDR_SIZE                 (4)               /* Header size */
#define B_DVR_TS_HDR_SYNC_BYTE            (0x00000047)       /* Sync bytes */
#define B_DVR_TS_HDR_SYNC_MASK            (0x000000ff)       /* Sync mask */
#define B_DVR_TS_HDR_PID_MASK             (0x1FFF)           /* PID mask */
#define B_DVR_TS_HDR_PID_SHIFT            (16)              /* PID shift */
#define B_DVR_TS_HDR_PID_HI_MASK          (0x1F00)           /* PID mask */
#define B_DVR_TS_HDR_PID_LO_MASK          (0x00FF)           /* PID mask */
#define B_DVR_TS_HDR_PID_LO_SHIFT         (16)              /* PID shift */
#define B_DVR_TS_HDR_PUSI                 (0x00004000)       /* plyload_unit_start_indicator. */
#define B_DVR_TS_HDR_AFC_ON               (0x10000000)       /* adaptation field on. */
#define B_DVR_TS_PACKET_SIZE              (188)             /* TS packet size */
#define B_DVR_TS_PACKET_PAD               (0xFF)            /* Packet padding */

struct B_DVR_DataInjectionService
{
    BDBG_OBJECT(B_DVR_DataInjectionService)
    unsigned index;
    B_DVR_DataInjectionServiceSettings settings;
    NEXUS_PacketSubHandle packetSub;
    B_DVR_ServiceCallback registeredCallback;
    void * appContext;
    B_MutexHandle dataInjectionMutex;
};
#if PSI_DATA_DUMP
static int  b_dvr_dump_count = 0;
static void B_DVR_DataInjectionService_P_DumpTsTables(uint8_t *buf, uint32_t length)
{
    uint32_t   i = 0;

    b_dvr_dump_count ++;

    if (b_dvr_dump_count > 4)
        return;

    BKNI_Printf("converting table format from PSI to TS... buf:(0x%08x), length:(%d)\n[", buf, length);

    for (i = 0; i < length; i ++)
    {
        BKNI_Printf("%02x ", *(buf + i));
        if ((i % 16) == 15)
        {
            BKNI_Printf("]\n[");
        }
    }

    BKNI_Printf("]\n\n");
}
#endif

static uint32_t B_DVR_DataInjectionService_P_ComputeTsPackCnt(uint32_t length) 
{
   int round = (length % (B_DVR_TS_PACKET_SIZE-B_DVR_TS_HDR_SIZE))?1:0;
   return length/(B_DVR_TS_PACKET_SIZE-B_DVR_TS_HDR_SIZE)+round;
}

static B_DVR_ERROR B_DVR_DataInjectionService_P_ConvertPsi2Ts(uint8_t *tsBuf, unsigned *pTsBufSize, uint8_t *pPsiTableBuff, unsigned length, unsigned pid)
{
   B_DVR_ERROR rc = B_DVR_SUCCESS;
   uint8_t *pIdx   = tsBuf; 
   uint32_t *pHeader = (uint32_t*)pIdx;              /* Header pointer.*/
   uint32_t cnt = B_DVR_DataInjectionService_P_ComputeTsPackCnt(length);           /* How many TS packets will it take? */
    uint32_t header = (B_DVR_TS_HDR_SYNC_BYTE & B_DVR_TS_HDR_SYNC_MASK) | B_DVR_TS_HDR_AFC_ON | 
                (pid & B_DVR_TS_HDR_PID_HI_MASK) | ((pid & B_DVR_TS_HDR_PID_LO_MASK) << B_DVR_TS_HDR_PID_LO_SHIFT);

   bool firstPkt = true;
   uint32_t continuityCount=0;

   BDBG_MSG(("B_DVR_DataInjectionService_P_ConvertPSI2TS >>>>"));
   BDBG_MSG(("pid %d: Psi Table length %d ts packet count %d",pid,length,cnt));

   if (cnt*B_DVR_TS_PACKET_SIZE > *pTsBufSize) 
   {
      BDBG_ERR(("Insufficient tsBufSize availableSize %d reqSize %d",*pTsBufSize,cnt*B_DVR_TS_PACKET_SIZE));
      rc = B_DVR_OUT_OF_DEVICE_MEMORY;
      goto error_tsBufSize;
   }

   *pTsBufSize = cnt*B_DVR_TS_PACKET_SIZE;
   
   BKNI_Memset((void*)pIdx, 0,cnt*B_DVR_TS_PACKET_SIZE);

   while (cnt--) 
   {
       uint32_t cpySize = 
           (length > (unsigned)(B_DVR_TS_PACKET_SIZE-B_DVR_TS_HDR_SIZE-(firstPkt?1:0)))? 
           (unsigned)(B_DVR_TS_PACKET_SIZE-B_DVR_TS_HDR_SIZE-(firstPkt?1:0)):
           length;  /* number of bytes to copy */
       uint32_t padSize = (B_DVR_TS_PACKET_SIZE-B_DVR_TS_HDR_SIZE - (firstPkt?1:0)) - cpySize;  /* number of bytes to PAD (last packet) */

       continuityCount = (continuityCount + 1) & 0xf; 
      /* Set the pid value and Cc. */
      *pHeader = header | continuityCount | (firstPkt?B_DVR_TS_HDR_PUSI:0);

      pIdx += B_DVR_TS_HDR_SIZE;    /* Skip over the header to add the data. */
      if (firstPkt)
      {
          *pIdx++ = 0;      /* set the pointer field to 0. */
      }
      firstPkt = false;
      BKNI_Memcpy((void *)pIdx, (void *)pPsiTableBuff, cpySize);
      pIdx += cpySize;
      pPsiTableBuff += cpySize;
      length -=cpySize;

      /* If we have leftovers, then pad the end of the last packet.*/
      if (padSize) 
      {
         BKNI_Memset(pIdx, B_DVR_TS_PACKET_PAD, padSize);
      }
      pHeader = (uint32_t*)pIdx;
   }
error_tsBufSize:
   BDBG_MSG(("TsBufSize %d",*pTsBufSize));
   return rc;
}

static void B_DVR_DataInjectionService_P_Complete(void *context, int param)
{
    B_DVR_DataInjectionServiceHandle dataInjectionService= (B_DVR_DataInjectionServiceHandle) context;
    B_DVR_Event event = eB_DVR_EventDataInjectionCompleted;
    B_DVR_Service service = eB_DVR_ServiceDataInjection;
    BSTD_UNUSED(param);
    BDBG_OBJECT_ASSERT(dataInjectionService,B_DVR_DataInjectionService);
    BDBG_MSG(("dataInjection complete index %d", dataInjectionService->index));
    if(dataInjectionService->registeredCallback)
    {
        dataInjectionService->registeredCallback(dataInjectionService->appContext,dataInjectionService->index,event,service);
    }
    return;
}

B_DVR_DataInjectionServiceHandle B_DVR_DataInjectionService_Open(B_DVR_DataInjectionServiceOpenSettings *pOpenSettings)
{
    B_DVR_DataInjectionServiceHandle dataInjectionService=NULL;
    B_DVR_ManagerHandle dvrManager;
    NEXUS_PacketSubOpenSettings packetSubOpenSettings;
    BDBG_MSG(("B_DVR_DataInjectionService_Open >>>>"));

    dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    dataInjectionService = BKNI_Malloc(sizeof(*dataInjectionService));
    if(!dataInjectionService)
    {
        BDBG_ERR(("error in alloc"));
        goto error_alloc;
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System, 
                                            sizeof(*dataInjectionService),
                                            true,__FUNCTION__,__LINE__);
    BKNI_Memset((void*)dataInjectionService,0,sizeof(*dataInjectionService));
    BDBG_OBJECT_SET(dataInjectionService,B_DVR_DataInjectionService);
    dataInjectionService->index = dvrManager->dataInjectionServiceCount;

    dataInjectionService->dataInjectionMutex = B_Mutex_Create(NULL);
    if(!dataInjectionService->dataInjectionMutex)
    {
        BDBG_ERR(("%s mutex create failed", __FUNCTION__));
        goto error_mutex_create;
    }
    NEXUS_PacketSub_GetDefaultOpenSettings(&packetSubOpenSettings);
    /*
     * TODO: Should this be reconfigured based on the app settings 
     */
    packetSubOpenSettings.fifoSize = pOpenSettings->fifoSize;
    dataInjectionService->packetSub = NEXUS_PacketSub_Open(dataInjectionService->index, &packetSubOpenSettings);

    if(!dataInjectionService->packetSub)
    {
        BDBG_ERR(("error in alloc"));
        goto error_packetSub;
    }

    B_Mutex_Lock(dvrManager->dvrManagerMutex);
    dvrManager->dataInjectionService[dataInjectionService->index]=dataInjectionService;
    dvrManager->dataInjectionServiceCount++;
    B_Mutex_Unlock(dvrManager->dvrManagerMutex);

    return dataInjectionService;
error_packetSub:
    B_Mutex_Destroy(dataInjectionService->dataInjectionMutex);
error_mutex_create:
    BDBG_OBJECT_DESTROY(dataInjectionService,B_DVR_DataInjectionService);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*dataInjectionService),
                                            false,__FUNCTION__,__LINE__);
    BKNI_Free(dataInjectionService);
error_alloc:
    BDBG_MSG(("B_DVR_DataInjectionService_Open >>>>"));
    return NULL;
}

B_DVR_ERROR B_DVR_DataInjectionService_Close(
    B_DVR_DataInjectionServiceHandle dataInjectionService)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_OBJECT_ASSERT(dataInjectionService,B_DVR_DataInjectionService);

    B_Mutex_Lock(dvrManager->dvrManagerMutex);
    dvrManager->dataInjectionService[dataInjectionService->index]=NULL;
    dvrManager->dataInjectionServiceCount--;
    B_Mutex_Unlock(dvrManager->dvrManagerMutex);

    B_Mutex_Lock(dataInjectionService->dataInjectionMutex);
    NEXUS_PacketSub_Close(dataInjectionService->packetSub);
    B_Mutex_Unlock(dataInjectionService->dataInjectionMutex);
    B_Mutex_Destroy(dataInjectionService->dataInjectionMutex);
    BDBG_OBJECT_DESTROY(dataInjectionService,B_DVR_DataInjectionService);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System, 
                                            sizeof(*dataInjectionService),
                                            false,__FUNCTION__,__LINE__);
    BKNI_Free(dataInjectionService);
    return rc;
}

B_DVR_ERROR B_DVR_DataInjectionService_SetSettings(
    B_DVR_DataInjectionServiceHandle dataInjectionService,
    B_DVR_DataInjectionServiceSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(dataInjectionService,B_DVR_DataInjectionService);
    BDBG_ASSERT(pSettings);
    B_Mutex_Lock(dataInjectionService->dataInjectionMutex);
    if(pSettings->dataInjectionServiceType >= eB_DVR_DataInjectionServiceTypeMax) 
    {
        rc = B_DVR_NOT_SUPPORTED;
        goto error;
    }
    BKNI_Memcpy((void *)&dataInjectionService->settings,(void *)pSettings,sizeof(*pSettings));
error:
    B_Mutex_Unlock(dataInjectionService->dataInjectionMutex);
    return rc;
}

B_DVR_ERROR B_DVR_DataInjectionService_GetSettings(
    B_DVR_DataInjectionServiceHandle dataInjectionService,
    B_DVR_DataInjectionServiceSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(dataInjectionService,B_DVR_DataInjectionService);
    BDBG_ASSERT(pSettings);
    B_Mutex_Lock(dataInjectionService->dataInjectionMutex);
    BKNI_Memcpy((void *)pSettings,(void *)&dataInjectionService->settings,sizeof(*pSettings));
    B_Mutex_Unlock(dataInjectionService->dataInjectionMutex);
    return rc;
}

B_DVR_ERROR B_DVR_DataInjectionService_Start(
    B_DVR_DataInjectionServiceHandle dataInjectionService,
    NEXUS_PidChannelHandle pidChannel,
    uint8_t *buf,
    unsigned size)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    uint8_t *psubBuffer;
    unsigned availableSize;
    NEXUS_PacketSubSettings packetSubSettings;
    NEXUS_PidChannelStatus pidChannelStatus;
    BDBG_OBJECT_ASSERT(dataInjectionService,B_DVR_DataInjectionService);
    BDBG_MSG(("B_DVR_DataInjectionService_Start index %u >>>>",dataInjectionService->index));
    B_Mutex_Lock(dataInjectionService->dataInjectionMutex);
    NEXUS_PidChannel_GetStatus(pidChannel,&pidChannelStatus);
    NEXUS_PacketSub_GetSettings(dataInjectionService->packetSub, &packetSubSettings);
    if(pidChannelStatus.playback) 
    {
        packetSubSettings.forcedInsertion = true;
    }
    packetSubSettings.pidChannel = pidChannel;
    packetSubSettings.loop = false;
    packetSubSettings.finished.callback = B_DVR_DataInjectionService_P_Complete;
    packetSubSettings.finished.context = dataInjectionService;
    packetSubSettings.finished.param = dataInjectionService->index;
    NEXUS_PacketSub_SetSettings(dataInjectionService->packetSub, &packetSubSettings);
    NEXUS_PacketSub_GetBuffer(dataInjectionService->packetSub, (void **)&psubBuffer, &availableSize);
    if(dataInjectionService->settings.dataInjectionServiceType == eB_DVR_DataInjectionServiceTypePSI)
    {
        unsigned tsBufSize = availableSize;
        rc = B_DVR_DataInjectionService_P_ConvertPsi2Ts(psubBuffer,
                                                        &tsBufSize,
                                                        buf,
                                                        size,
                                                        dataInjectionService->settings.pid);
        BDBG_MSG(("Converted PSI TS, psubBuffer:(0x%08x), tsBufSize:(%d)", psubBuffer, tsBufSize));
        #if PSI_DATA_DUMP
        B_DVR_DataInjectionService_P_DumpTsTables(psubBuffer, tsBufSize);
        #endif
        if(rc!=B_DVR_SUCCESS)
        {
            BDBG_ERR(("convertion of PSI tables to TS formatted PSI stream failed reqSize %d availableSize %d",tsBufSize,availableSize));
            NEXUS_PacketSub_Flush(dataInjectionService->packetSub);
            goto error;
        }

        NEXUS_PacketSub_WriteComplete(dataInjectionService->packetSub, tsBufSize);
    }
    else
    {
        if(availableSize < size)
        {
            BDBG_ERR(("Insufficient PSUB FIFO. Stop PSUB and Restart PSUB"));
            NEXUS_PacketSub_Flush(dataInjectionService->packetSub);
            rc = B_DVR_OUT_OF_DEVICE_MEMORY;
            goto error;
        }
        BKNI_Memcpy((void *)psubBuffer,(void *)buf,size);
        BDBG_MSG(("TS, psubBuffer:(0x%08x), size:(%d)", psubBuffer, size));
        #if PSI_DATA_DUMP
        B_DVR_DataInjectionService_P_DumpTsTables(psubBuffer, size);
        #endif

        NEXUS_PacketSub_WriteComplete(dataInjectionService->packetSub, size);
    }

    NEXUS_PacketSub_Start(dataInjectionService->packetSub);
error:
    B_Mutex_Unlock(dataInjectionService->dataInjectionMutex);
    BDBG_MSG(("B_DVR_DataInjectionService_Start index %u <<<",dataInjectionService->index));
    return rc;
}

B_DVR_ERROR B_DVR_DataInjectionService_Stop(
    B_DVR_DataInjectionServiceHandle dataInjectionService)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    NEXUS_PacketSubStatus psubStatus;
    BDBG_OBJECT_ASSERT(dataInjectionService,B_DVR_DataInjectionService);
    BDBG_MSG(("B_DVR_DataInjectionService_Stop index %u >>>>",dataInjectionService->index));
    B_Mutex_Lock(dataInjectionService->dataInjectionMutex);
    NEXUS_PacketSub_GetStatus(dataInjectionService->packetSub,&psubStatus);
    BDBG_MSG(("dataInjectionService busy %d finished %d",psubStatus.busy,psubStatus.finished));
    NEXUS_PacketSub_Flush(dataInjectionService->packetSub);
    NEXUS_PacketSub_Stop(dataInjectionService->packetSub);
    B_Mutex_Unlock(dataInjectionService->dataInjectionMutex);
    BDBG_MSG(("B_DVR_DataInjectionService_Stop index %u <<<<",dataInjectionService->index));
    return rc;

}

B_DVR_ERROR B_DVR_DataInjectionService_InstallCallback(
    B_DVR_DataInjectionServiceHandle dataInjectionService,
    B_DVR_ServiceCallback registeredCallback,
    void * appContext
    )
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(dataInjectionService,B_DVR_DataInjectionService);
    B_Mutex_Lock(dataInjectionService->dataInjectionMutex);
    dataInjectionService->registeredCallback = registeredCallback;
    dataInjectionService->appContext = appContext;
    B_Mutex_Unlock(dataInjectionService->dataInjectionMutex);
    return rc;
}

B_DVR_ERROR B_DVR_DataInjectionService_RemoveCallback(
    B_DVR_DataInjectionServiceHandle dataInjectionService)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(dataInjectionService,B_DVR_DataInjectionService);
    B_Mutex_Lock(dataInjectionService->dataInjectionMutex);
    dataInjectionService->appContext = NULL;
    dataInjectionService->registeredCallback = NULL;
    B_Mutex_Unlock(dataInjectionService->dataInjectionMutex);
    return rc;
}

unsigned B_DVR_DataInjectionService_GetChannelIndex(
    B_DVR_DataInjectionServiceHandle dataInjectionService)
{
    unsigned psubChannel=0;
    BDBG_OBJECT_ASSERT(dataInjectionService,B_DVR_DataInjectionService);
    B_Mutex_Lock(dataInjectionService->dataInjectionMutex);
    psubChannel = dataInjectionService->index;
    B_Mutex_Unlock(dataInjectionService->dataInjectionMutex);
    return psubChannel;
}

