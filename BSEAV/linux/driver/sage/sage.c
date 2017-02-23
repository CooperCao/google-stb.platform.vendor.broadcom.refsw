/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include <linux/version.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/mutex.h>
#include <linux/seq_file.h>
#include <linux/brcmstb/brcmstb.h>
#include <linux/brcmstb/memory_api.h>
#include <linux/kthread.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/slab.h>
#include <linux/namei.h>
#include <linux/dma-contiguous.h>
#include "quad.h"
#include "sage.h"

#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
#include "bcmdriver_arm.c"
#endif

BDBG_MODULE( sage );

#define MODULE_NAME "sage"
#define INFINITE 0xFFFFFFFF
#define SAGE_MAJOR 31 /* Major device number */

#define REGISTER_BASE   (BCHP_PHYSICAL_OFFSET + (BCHP_REGISTER_START & ~0xFFF))
#define REGISTER_SIZE   (BCHP_REGISTER_END - (BCHP_REGISTER_START & ~0xFFF))
    #define PINFO(fmt, args...) printk( KERN_INFO "SAGEDRV: " fmt, ## args)

int64_t __aeabi_ldivmod_c(int64_t n, int64_t d, int64_t *rem)
{
    /* Run-time ABI for the ARM Architecture, Table 9  */
    *rem =  __moddi3(n, d);
    return __divdi3(n, d);
}

uint64_t __aeabi_uldivmod_c(uint64_t n, uint64_t d, uint64_t *rem)
{
    /* Run-time ABI for the ARM Architecture, Table 9  */
    return __qdivrem(n, d, rem);
}

static SageBase sagebase;

static irqreturn_t sage_isr(int irq, void *wq)
{
    IrqCtx *pIsr = (IrqCtx *) wq;

    disable_irq_nosync(irq);

    if(irq < 32 || irq > 32 + MAX_IRQ_NUM || irq != pIsr->linuxIrqNum)
    {
        printk("Wrong IRQ %d received! expecting %d\n",irq,pIsr->linuxIrqNum);
        return IRQ_HANDLED;
    }

    pIsr->irqreceived++;
    wake_up_interruptible(&pIsr->isrEvent);

    return IRQ_HANDLED;
}
static bool test(IrqCtx *pIsr)
{
    return pIsr->irqProcessed < pIsr->irqreceived;
}
static int sage_isr_thread(void *data)
{
    IrqCtx *pIsr = (IrqCtx *) data;

    while(!kthread_should_stop())
    {
        if(0 >= wait_event_interruptible_timeout(pIsr->isrEvent,test(pIsr),msecs_to_jiffies(500)))
        {
//            pr_err("\n %s %d wait interrupt timeout!",__FUNCTION__,__LINE__);
        }else
        {
            BINT_Isr(sagebase.hInt,pIsr->linuxIrqNum-32);
            pIsr->irqProcessed = pIsr->irqreceived;
            enable_irq(pIsr->linuxIrqNum);
        }
    }
	return 0;
}

static int SAGE_EnableInterrupt(int linuxIrqNum,IrqCtx *pIsr)
{
    int rc;
    char name[50];

    pIsr->irqreceived = 0;
    pIsr->irqProcessed = 0;

    pIsr->linuxIrqNum = linuxIrqNum;

    sprintf(name,"Sage ISR %5d",linuxIrqNum);

    init_waitqueue_head(&pIsr->isrEvent);

    pIsr->isrTask = kthread_run(sage_isr_thread,pIsr,name);
    if(pIsr->isrTask == NULL)
    {
        pr_err("Start Interrupt thread %d failed\n",linuxIrqNum);
        return BERR_TRACE(BERR_OS_ERROR);
    }

    if((rc = request_irq(linuxIrqNum, sage_isr, IRQF_TRIGGER_HIGH|IRQF_SHARED,name,(void *)pIsr)) != 0)
    {
        pr_err("request_irq %d failed %d\n",linuxIrqNum,rc);
        return BERR_TRACE(BERR_OS_ERROR);
    }

    return BERR_SUCCESS;
}

static void SAGE_DisableInterrupt(IrqCtx *pIsr)
{
    if(pIsr->linuxIrqNum != 0)
    {
        free_irq(pIsr->linuxIrqNum,pIsr);
        pIsr->linuxIrqNum = 0;
    }

    if(pIsr->isrTask != NULL)
    {
        kthread_stop(pIsr->isrTask);
        pIsr->isrTask = NULL;
    }
}

static void Sage_FlushCache_isrsafe(const void *address, size_t numBytes)
{
    brcm_cpu_dcache_flush(address, numBytes);
}
static uint64_t Sage_AddrToOffset(const void *addr)
{
    BMEM_Heap_Handle  Heap = sagebase.hMemGlr;
    void              *pvAddress = (void *)addr;
    uint32_t          ulOffset;

    if(Heap == NULL)
    {
        BDBG_WRN(("%s %d hMemGlr NULL!",__FUNCTION__,__LINE__));
        return 0;
    }

    BMEM_Heap_ConvertAddressToOffset_isrsafe(Heap,pvAddress,&ulOffset);

    return (uint64_t)ulOffset;
}
/* .offset_to_addr : BSAGElib_MemoryMap_OffsetToAddrCallback prototype */
static void *Sage_OffsetToAddr(uint64_t offset)
{
    BMEM_Heap_Handle   Heap = sagebase.hMemGlr;
    uint32_t           ulOffset = offset;
    void              *pvAddress;

    if(Heap == NULL)
    {
        BDBG_WRN(("%s %d hMemGlr NULL!",__FUNCTION__,__LINE__));
        return NULL;
    }

    BMEM_Heap_ConvertOffsetToAddress_isrsafe(Heap,ulOffset,&pvAddress);

    return pvAddress;
}

static void * Sage_Malloc(size_t size)
{
    BMEM_Heap_Handle   Heap = sagebase.hMemGlr;
    if(Heap == NULL)
    {
        BDBG_WRN(("%s %d hMemGlr NULL!",__FUNCTION__,__LINE__));
        return NULL;
    }
    return BMEM_Heap_Alloc(Heap,size);
}
static void Sage_MfreeOffset(uint32_t offset)
{
    void *pMemory;
    BMEM_Heap_Handle   Heap = sagebase.hMemGlr;
    if(Heap == NULL)
    {
        BDBG_WRN(("%s %d hMemGlr NULL!",__FUNCTION__,__LINE__));
        return;
    }
    BMEM_Heap_ConvertOffsetToAddress_isrsafe(Heap,offset,&pMemory);

    BMEM_Heap_Free(Heap,pMemory);
}
static void Sage_Mfree( void *pMemory )
{
    BMEM_Heap_Handle   Heap = sagebase.hMemGlr;
    if(Heap == NULL)
    {
        BDBG_WRN(("%s %d hMemGlr NULL!",__FUNCTION__,__LINE__));
        return;
    }
    BMEM_Heap_Free(Heap,pMemory);
}

static void Sage_Lock_Sage(void)
{
    BKNI_AcquireMutex(sagebase.sageLock);
}
/* .unlock_sage : BSAGElib_Sync_UnlockCallback prototype */
static void Sage_Unlock_Sage(void)
{
    BKNI_ReleaseMutex(sagebase.sageLock);
}

static BCMD_VKLID_e
SAGE_AllocSageVkl(void)
{
    BERR_Code rc;
    BCMD_VKLID_e vklId;
    BHSM_AllocateVKLIO_t allocateVKLIO;

    BKNI_Memset(&allocateVKLIO, 0, sizeof(BHSM_AllocateVKLIO_t));
    allocateVKLIO.client                  = BHSM_ClientType_eSAGE;
    allocateVKLIO.customerSubMode         = BCMD_CustomerSubMode_eGeneric_CA_64_4;
    allocateVKLIO.bNewVKLCustSubModeAssoc = false;

    rc = BHSM_AllocateVKL(sagebase.hHsm, &allocateVKLIO);
    if(rc != BERR_SUCCESS) {
        vklId = BCMD_VKL_eMax;
        BDBG_ERR(("%s - BHSM_AllocateVKL() fails", __FUNCTION__));
    }
    else {
        vklId = allocateVKLIO.allocVKL;
        BDBG_MSG(("%s - allocate vkl id=%u", __FUNCTION__, vklId));
    }

    return vklId;
}

static void
SAGE_FreeSageVkl(
    BCMD_VKLID_e vklId)
{
    BHSM_FreeVKL(sagebase.hHsm, vklId);
}

static void SAGE_Hsm_Uninit(void)
{

    BHSM_Close(sagebase.hHsm);

    SAGE_DisableInterrupt(&sagebase.isrHsm);

    __free_pages(sagebase.pageHsm,BLOCK_SIZE_2M);
    BMEM_Heap_Destroy(sagebase.hMemHsm);
}

static void SAGE_Base_Uninit(void)
{
    brcmstb_memory_kva_unmap(sagebase.addrGLR);
    BMEM_Heap_Destroy(sagebase.hMemGlr);

    if(sagebase.hSAGE != NULL)
    {
        BSAGE_Close(sagebase.hSAGE);
        sagebase.hSAGE = NULL;
    }

    SAGE_DisableInterrupt(&sagebase.isrRpc);

    BKNI_DestroyMutex(sagebase.sageLock);

    BTMR_Close(sagebase.hTmr);
    BMEM_Close(sagebase.hMemModule);
    BINT_Close(sagebase.hInt);
    BREG_Close(sagebase.hReg);
    iounmap(sagebase.pRegAddress);

    BDBG_Uninit();
    BKNI_Uninit();

}

static int SAGE_Base_Init(void)
{
    BERR_Code rc;
    BREG_OpenSettings openSettings;
    const BINT_Settings  *pIntSetting;
    BMEM_Settings memModuleSettings;
    BTMR_DefaultSettings  stTimerSettings;
    BSAGE_Settings sage_rpc_settings;

    rc = BKNI_Init();
    if (rc != BERR_SUCCESS)
    {
        printk(" %s %d BKNI_Init failed ! rc %d \n",__FUNCTION__,__LINE__,rc);
        return rc;
    }

    rc = BDBG_Init();
    if (rc != BERR_SUCCESS)
    {
        printk(" %s %d BDBG_Init failed ! rc %d \n",__FUNCTION__,__LINE__,rc);
        return rc;
    }

    /*BDBG_SetModuleLevel("BHSI",BDBG_eTrace);*/
    /* mmap registers in uncached address space */
    sagebase.pRegAddress = ioremap(REGISTER_BASE, REGISTER_SIZE);
    if ( NULL == sagebase.pRegAddress) {
        printk(" %s %d ioremap REGISTER_BASE failed ! \n",__FUNCTION__,__LINE__);
        return BERR_TRACE(BERR_OS_ERROR);
    }

    BREG_GetDefaultOpenSettings(&openSettings);

    /* Open register interface with mapped address */
    rc = BREG_Open(&sagebase.hReg, sagebase.pRegAddress, BCHP_REGISTER_END, &openSettings);
    if (rc != BERR_SUCCESS)
    {
        printk(" %s %d BREG_Open failed ! rc %d \n",__FUNCTION__,__LINE__,rc);
        return rc;
    }

    pIntSetting = BINT_GETSETTINGS();
    rc = BINT_Open(&sagebase.hInt, sagebase.hReg, pIntSetting, NULL);
    if (rc != BERR_SUCCESS)
    {
        printk(" %s %d BINT_Open failed ! rc %d \n",__FUNCTION__,__LINE__,rc);
        return rc;
    }

    rc = BMEM_GetDefaultSettings(&memModuleSettings);
    if (rc != BERR_SUCCESS)
    {
        printk(" %s %d BMEM_GetDefaultSettings failed ! rc %d \n",__FUNCTION__,__LINE__,rc);
        return rc;
    }

    rc = BMEM_Open(&sagebase.hMemModule, &memModuleSettings);
    if (rc != BERR_SUCCESS)
    {
        printk(" %s %d BMEM_Open failed ! rc %d \n",__FUNCTION__,__LINE__,rc);
        return rc;
    }

    BTMR_GetDefaultSettings(&stTimerSettings);
    BTMR_Open(&sagebase.hTmr, NULL, sagebase.hReg,sagebase.hInt, &stTimerSettings);
    if (rc != BERR_SUCCESS)
    {
        printk(" %s %d BMEM_Open failed ! rc %d \n",__FUNCTION__,__LINE__,rc);
        return rc;
    }

    rc = BKNI_CreateMutex(&sagebase.sageLock);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: fail to BKNI_CreateMutex for BSAGE", __FUNCTION__));
        return rc;
    }

    sage_rpc_settings.hReg = sagebase.hReg;
    sage_rpc_settings.hInt = sagebase.hInt;
    sage_rpc_settings.hTmr = sagebase.hTmr;

    sage_rpc_settings.i_memory_sync.flush = Sage_FlushCache_isrsafe;
    sage_rpc_settings.i_memory_sync.invalidate = Sage_FlushCache_isrsafe;
    sage_rpc_settings.i_memory_map.offset_to_addr = Sage_OffsetToAddr;
    sage_rpc_settings.i_memory_map.addr_to_offset = Sage_AddrToOffset;
    sage_rpc_settings.i_sync_sage.lock = Sage_Lock_Sage;
    sage_rpc_settings.i_sync_sage.unlock = Sage_Unlock_Sage;
    sage_rpc_settings.i_sync_hsm.lock = Sage_Lock_Sage;
    sage_rpc_settings.i_sync_hsm.unlock = Sage_Unlock_Sage;

    sage_rpc_settings.enablePinmux = 1;

    rc = BSAGE_Open(&sagebase.hSAGE,&sage_rpc_settings);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: fail to open SAGE", __FUNCTION__));
        return rc;
    }

    return BERR_SUCCESS;
}

static int SAGE_Hsm_Init(void)
{
    BERR_Code rc;
    BMEM_Heap_Settings    mem_heap_settings;
    void *addrHSM;
    phys_addr_t offsetHSM;
    BHSM_Settings hsmSettings;
    BHSM_InitKeySlotIO_t keyslot_io;

    sagebase.pageHsm = alloc_pages(GFP_KERNEL | __GFP_ZERO, BLOCK_SIZE_2M);
    addrHSM = page_address(sagebase.pageHsm);
    offsetHSM = page_to_phys(sagebase.pageHsm);

    mem_heap_settings.eSafetyConfig = BMEM_CONFIG_FASTEST;
    mem_heap_settings.eBookKeeping  = BMEM_BOOKKEEPING_SYSTEM;
    mem_heap_settings.uiAlignment = 8;
    mem_heap_settings.pCachedAddress = addrHSM;

    rc = BMEM_Heap_Create(sagebase.hMemModule,addrHSM, offsetHSM, SIZE_2M, &mem_heap_settings,&sagebase.hMemHsm);
    if(rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}

    rc = SAGE_EnableInterrupt(32 + BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_BSP_CPU_INTR_SHIFT,&sagebase.isrHsm);
    if(rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}

    BHSM_GetDefaultSettings(&hsmSettings, NULL);
    hsmSettings.hHeap = sagebase.hMemHsm;

    hsmSettings.sageEnabled = true;

    rc = BHSM_Open(&sagebase.hHsm, sagebase.hReg, NULL, sagebase.hInt, &hsmSettings);
    if (rc) { return BERR_TRACE(rc); }

    BKNI_Memset(&keyslot_io, 0, sizeof(keyslot_io));

    keyslot_io.unKeySlotType0Num = 20;
    keyslot_io.unKeySlotType1Num = 10;
    keyslot_io.unKeySlotType2Num = 15;
    keyslot_io.unKeySlotType3Num = 25;
    keyslot_io.unKeySlotType4Num = 7;
    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    keyslot_io.unKeySlotType5Num = 2;
    #endif

    #if (BHSM_ZEUS_VERSION == BHSM_ZEUS_VERSION_CALC(3,0))
    keyslot_io.unKeySlotType3Num = 0;
    #endif

    #if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0))
    keyslot_io.unKeySlotType5Num = 0;
    #endif

    #if (BHSM_ZEUS_VERSION == BHSM_ZEUS_VERSION_CALC(1,0))
    keyslot_io.unKeySlotType1Num = 0;
    keyslot_io.unKeySlotType4Num = 0;
    keyslot_io.unKeySlotType5Num = 0;
    keyslot_io.unKeySlotType6Num = 0;
    #endif

    keyslot_io.bMulti2SysKey     = 0;
    keyslot_io.numMulti2KeySlots = 8;

    /* For API backward compatability */
    if( keyslot_io.bMulti2SysKey && (keyslot_io.numMulti2KeySlots == 0) )
    {
        keyslot_io.numMulti2KeySlots = BCMD_MULTI2_MAXSYSKEY;
    }

    /* Disregard errors, as this can only be run once per board boot. */
    rc = BHSM_InitKeySlot(sagebase.hHsm, &keyslot_io);
    /* Print out warning and ignore the error */
    if (rc)
    {
        PINFO("**********************************************");
        PINFO("If you see this warning and the HSM errors above, you need to perform some");
        PINFO("board reconfiguration. This is not required. If you want, you can ignore them.");
        PINFO("Use BBS to check if BSP_GLB_NONSECURE_GLB_IRDY = 0x07.");
        PINFO("If not, BSP/Aegis is not ready to accept a command.");
        PINFO("SUN_TOP_CTRL_STRAP_VALUE[bit28:29 strap_test_debug_en] should be 0b'00 if you plan");
        PINFO("to use BSP ROM code. If not, check with board designer on your strap pin.");
        PINFO("**********************************************");
        rc = BERR_SUCCESS;
    }

    return rc;
}

/* copy regionMaps from sagebase.regionMap into pRegionMap */
static BERR_Code SAGE_Boot_WriteRegionMap(BSAGElib_RegionInfo *pRegionMap)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGE_BootSettings *pBootSettings = &sagebase.bootSettings;
    int i;

    if(pRegionMap == NULL)
    {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s %d NULL pRegionMap !!"));
        goto err;
    }

    for(i=0;i < pBootSettings->regionMapNum && i < REGION_MAP_MAX_NUM;i++)
    {
        pRegionMap[i] = sagebase.regionMap[i];
    }
err:
    return rc;
}

static BERR_Code SAGE_Boot_Load(char *filename,uint32_t *pOffset, uint32_t *pSize)
{
    BERR_Code rc = BERR_SUCCESS;
    struct kstat stat;
    struct file* filp = NULL;
    mm_segment_t oldfs;
    loff_t fileoffset;
    void *pBuff;

    if(sagebase.hMemGlr == NULL)
    {
        rc = BERR_NOT_INITIALIZED;
        printk("%s %d GLR heap not available yet!\n",__FUNCTION__,__LINE__);
        goto err;
    }

    if(pOffset == NULL || pSize == NULL )
    {
        rc = BERR_INVALID_PARAMETER;
        printk("%s %d pOffset or pSize is NULL !\n",__FUNCTION__,__LINE__);
        goto err;
    }
    *pOffset = 0;
    *pSize = 0;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(filename, O_RDONLY , 0);
    if(IS_ERR(filp)) {
        rc = BERR_UNKNOWN;
        printk("%s %d file %s not found!\n",__FUNCTION__,__LINE__,filename);
        goto err;
    }

    vfs_getattr(&filp->f_path, &stat);
    *pSize = stat.size;

    pBuff = (uint32_t *) Sage_Malloc(*pSize);
    if(pBuff == NULL) {
        rc = BERR_OUT_OF_DEVICE_MEMORY;
        printk("%s %d alloc 0x%x failed!\n",__FUNCTION__,__LINE__,(unsigned int)*pSize);
        goto err;
    }

    fileoffset = 0;
    *pSize = vfs_read(filp, pBuff,*pSize, &fileoffset);

    filp_close(filp, NULL);
    set_fs(oldfs);

    *pOffset = Sage_AddrToOffset(pBuff);

err:
    return rc;
}
/* get the index of region with regionId in sagebase.regionMap */
static int SAGE_Boot_GetRegion(uint32_t regionId)
{
    int i;

    for(i=0;i<sagebase.bootSettings.regionMapNum  && i < REGION_MAP_MAX_NUM;i++)
    {
        if(sagebase.regionMap[i].id == regionId)
        {
            return i;
        }
    }
    return -1;
}

/* Add the region to sagebase.regionMap, if duplicate, change the old one in sagebase.regionMap
   return the index of added region */
static int SAGE_Boot_AddRegion(BSAGElib_RegionInfo *pRegionInfo)
{
    int i;

    for(i=0;i<sagebase.bootSettings.regionMapNum  && i < REGION_MAP_MAX_NUM;i++)
    {
        if(sagebase.regionMap[i].id == pRegionInfo->id)
        {
            sagebase.regionMap[i] = *pRegionInfo;
            BDBG_MSG(("%s %d add region %d offset 0x%lx size 0x%lx at %d!!",__FUNCTION__,__LINE__,pRegionInfo->id,pRegionInfo->offset,pRegionInfo->size,i));
            goto end;
        }
    }

    if(i >= REGION_MAP_MAX_NUM)
    {
        i = -1;
        BDBG_ERR(("%s %d Regions full at %d, add region %d offset 0x%lx size 0x%lx failed !!",__FUNCTION__,__LINE__,sagebase.bootSettings.regionMapNum, pRegionInfo->id,pRegionInfo->offset,pRegionInfo->size));
        goto end;
    }

    sagebase.regionMap[i].id = pRegionInfo->id;
    sagebase.regionMap[i]      = *pRegionInfo;

    BDBG_MSG(("%s %d add region %d offset 0x%lx size 0x%lx at %d!!",__FUNCTION__,__LINE__,pRegionInfo->id,pRegionInfo->offset,pRegionInfo->size,i));

    sagebase.bootSettings.regionMapNum++;
end:
    return i;
}

static BERR_Code SAGE_Boot_ReadRegionMap(uint32_t regionMapNum,BSAGElib_RegionInfo *pRegionMap)
{
    BERR_Code rc;
    BMEM_Heap_Settings    mem_heap_settings;
    uint32_t GlrOffset,GlrSize,i;

    for(i=0;i < regionMapNum && i < REGION_MAP_MAX_NUM;i++)
    {
        rc = SAGE_Boot_AddRegion(&pRegionMap[i]);
        if(rc < 0)
        {
            BDBG_ERR(("%s %d add region %d offset 0x%lx size 0x%lx failed !!",__FUNCTION__,__LINE__,pRegionMap[i].id,pRegionMap[i].offset,pRegionMap[i].size));
            goto err;
        }
    }

    i = SAGE_Boot_GetRegion(BSAGElib_RegionId_Glr);

    if(i < 0 || sagebase.regionMap[i].id != BSAGElib_RegionId_Glr)
    {
        rc = BERR_UNKNOWN;
        BDBG_ERR(("%s %d Fail to read GLR info from region Map!!",__LINE__));
        goto err;
    }

    GlrOffset = sagebase.regionMap[i].offset;
    GlrSize = sagebase.regionMap[i].size;

    if(sagebase.addrGLR != NULL)
        brcmstb_memory_kva_unmap(sagebase.addrGLR);

    if(sagebase.hMemGlr != NULL)
        BMEM_Heap_Destroy(sagebase.hMemGlr);

    sagebase.addrGLR = brcmstb_memory_kva_map_phys(GlrOffset, GlrSize,true);
    if ( NULL == sagebase.addrGLR)
    {
        rc = BERR_OS_ERROR;
        BDBG_ERR(("%s %d brcmstb_memory_kva_map_phys offset 0x%lx size 0x%lx failed!",__FUNCTION__,__LINE__,GlrOffset, GlrSize));
        goto err;
    }

    mem_heap_settings.eSafetyConfig = BMEM_CONFIG_FASTEST;
    mem_heap_settings.eBookKeeping  = BMEM_BOOKKEEPING_SYSTEM;
    mem_heap_settings.uiAlignment = 8;
    mem_heap_settings.pCachedAddress = sagebase.addrGLR;

    rc = BMEM_Heap_Create(sagebase.hMemModule,sagebase.addrGLR, GlrOffset, GlrSize, &mem_heap_settings,&sagebase.hMemGlr);
    if(rc!=BERR_SUCCESS)
    {
        BDBG_ERR(("%s %d BMEM_Heap_Create offset 0x%lx size 0x%lx at %p failed!",__FUNCTION__,__LINE__,GlrOffset, GlrSize,sagebase.addrGLR));
        goto err;
    }

    i = SAGE_Boot_GetRegion(BSAGElib_RegionId_Glr2);

    if(i >= 0 && sagebase.regionMap[i].id == BSAGElib_RegionId_Glr2)
    {

        GlrOffset = sagebase.regionMap[i].offset;
        GlrSize = sagebase.regionMap[i].size;

        if(sagebase.addrGLR2 != NULL)
            brcmstb_memory_kva_unmap(sagebase.addrGLR2);

        if(sagebase.hMemGlr2 != NULL)
            BMEM_Heap_Destroy(sagebase.hMemGlr2);

        sagebase.addrGLR2 = brcmstb_memory_kva_map_phys(GlrOffset, GlrSize,true);
        if ( NULL == sagebase.addrGLR2)
        {
            rc = BERR_OS_ERROR;
            BDBG_ERR(("%s %d brcmstb_memory_kva_map_phys offset 0x%lx size 0x%lx failed!",__FUNCTION__,__LINE__,GlrOffset, GlrSize));
            goto err;
        }

        mem_heap_settings.eSafetyConfig = BMEM_CONFIG_FASTEST;
        mem_heap_settings.eBookKeeping  = BMEM_BOOKKEEPING_SYSTEM;
        mem_heap_settings.uiAlignment = 8;
        mem_heap_settings.pCachedAddress = sagebase.addrGLR2;

        rc = BMEM_Heap_Create(sagebase.hMemModule,sagebase.addrGLR2, GlrOffset, GlrSize, &mem_heap_settings,&sagebase.hMemGlr2);
        if(rc!=BERR_SUCCESS)
        {
            BDBG_ERR(("%s %d BMEM_Heap_Create offset 0x%lx size 0x%lx at %p failed!",__FUNCTION__,__LINE__,GlrOffset, GlrSize,sagebase.addrGLR2));
            goto err;
        }
    }
err:
    return rc;
}

static BERR_Code SAGE_Boot_Region_Check(uint32_t regionMapNum,BSAGElib_RegionInfo *pRegionMap)
{
    BERR_Code rc = BERR_SUCCESS;
    uint8_t cntNewRegion = 0;
    int i,regionIndex,srrIndex = -1,glr2Index = -1;

    for(i=0;i < regionMapNum && i < REGION_MAP_MAX_NUM;i++)
    {
        if(pRegionMap[i].id == BSAGElib_RegionId_Srr)
            srrIndex = i;
        if(pRegionMap[i].id == BSAGElib_RegionId_Glr2)
            glr2Index = i;

        regionIndex = SAGE_Boot_GetRegion(pRegionMap[i].id);
        if(regionIndex < 0)
        {
            /* the region is not found in sagebase.regionMap[], must be a new region */
            cntNewRegion++;
            continue;
        }

        if(sagebase.regionMap[regionIndex].id != pRegionMap[i].id)
        {
            rc = BERR_UNKNOWN;
            BDBG_ERR(("%s %d found new region(%d@%d) different with old(%d@%d) something is wrong ",__FUNCTION__,__LINE__,
                      sagebase.regionMap[regionIndex].id, regionIndex, pRegionMap[i].id,i));
            goto err;
        }

        if(sagebase.regionMap[regionIndex].offset != pRegionMap[i].offset ||
           sagebase.regionMap[regionIndex].size   != pRegionMap[i].size)
        {
            /* new region do not match with old */
            BDBG_ERR(("%s %d new region(%d) changed, old(%d) offset 0x%lx, size 0x%lx ---old ox%lx size 0x%lx",__FUNCTION__,__LINE__,
                  sagebase.regionMap[regionIndex].id,      pRegionMap[i].id,
                  sagebase.regionMap[regionIndex].offset,  pRegionMap[i].offset,
                  sagebase.regionMap[regionIndex].size,    pRegionMap[i].size));

            if(sagebase.regionMap[regionIndex].id == BSAGElib_RegionId_Srr ||
               sagebase.regionMap[regionIndex].id == BSAGElib_RegionId_Glr2)
            {
                /* SRR and GLR2 can not be changed */
                BDBG_ERR(("%s %d  SRR and GLR2 can not change, but new region SRR, GLR2 changed.",__FUNCTION__,__LINE__));
                /*rc = BERR_INVALID_PARAMETER;
                goto err;*/
            }
        }
    }
    if(cntNewRegion + sagebase.bootSettings.regionMapNum > REGION_MAP_MAX_NUM)
    {
        BDBG_ERR(("%s %d Number of Regions too many, new region number %d, old %d ",__FUNCTION__,__LINE__,cntNewRegion, sagebase.bootSettings.regionMapNum));
        rc = BERR_INVALID_PARAMETER;
        goto err;
    }

    if(srrIndex == -1)
        srrIndex = SAGE_Boot_GetRegion(BSAGElib_RegionId_Srr);
    if(glr2Index == -1)
        glr2Index = SAGE_Boot_GetRegion(BSAGElib_RegionId_Glr2);

    if(srrIndex == -1)
    {
        BDBG_ERR(("%s %d SRR region not available ",__FUNCTION__,__LINE__));
        rc = BERR_INVALID_PARAMETER;
        goto err;
    }
/*
    if(glr2Index == -1)
    {
        BDBG_ERR(("%s %d GLR2 region not available ",__FUNCTION__,__LINE__));
        rc = BERR_INVALID_PARAMETER;
        goto err;
    }*/
err:
    return rc;
}

static int SAGE_Boot_Rpc_Init(uint32_t HSIBufferOffset)
{
    BERR_Code rc;

    if(NULL == Sage_OffsetToAddr(HSIBufferOffset))
    {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s %d HSIBufferOffset 0x%lx is wrong, it's not within GLR region!!",__FUNCTION__,__LINE__,(unsigned long)HSIBufferOffset));
        goto err;
    }

    rc = BSAGE_Rpc_Init(HSIBufferOffset);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BSAGE_Rpc_Init failed.", __FUNCTION__));
        goto err;
    }

    SAGE_DisableInterrupt(&sagebase.isrRpc);

    rc = SAGE_EnableInterrupt(32 + BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_SCPU_CPU_INTR_SHIFT,&sagebase.isrRpc);
    if(rc!=BERR_SUCCESS)
    {
        BDBG_ERR(("%s %d SAGE_EnableInterrupt BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_SCPU_CPU_INTR_SHIFT %d failed!",__FUNCTION__,__LINE__,32 + BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_SCPU_CPU_INTR_SHIFT));
        goto err;
    }
err:
    return rc;
}
static int SAGE_Boot_RegionInfo(uint32_t HSIBufferOffset,uint32_t regionMapNum,BSAGElib_RegionInfo *pRegionMap)
{
    BERR_Code rc = BERR_SUCCESS;
    char *bootloader_filename,*framwork_filename,*bootloaderDev_filename,*framworkDev_filename;
    BCMD_VKLID_e vkl;
    BSAGElib_RegionInfo *pBootRegionMap=NULL;

    if(pRegionMap == NULL)
    {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s %d NULL pRegionMap !!",__FUNCTION__,__LINE__));
        goto err;
    }

    if(regionMapNum > REGION_MAP_MAX_NUM)
    {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s %d regionMapNum %d is bigger than %d !!",__FUNCTION__,__LINE__,regionMapNum,REGION_MAP_MAX_NUM));
        goto err;
    }

    if(HSIBufferOffset == 0)
    {
        if(sagebase.bootSettings.HSIBufferOffset == 0)
        {
            rc = BERR_INVALID_PARAMETER;
            BDBG_ERR(("%s %d HSIBufferOffset is zero!!",__FUNCTION__,__LINE__));
            goto err;
        }
        if(sagebase.bootSettings.HSIBufferOffset != HSIBufferOffset)
        {
            BDBG_WRN(("%s %d HSIBufferOffset(0x%lx) is different from old(0x%lx)!! HSIBufferOffset can not change, set it to old value !!",__FUNCTION__,__LINE__,HSIBufferOffset,sagebase.bootSettings.HSIBufferOffset));
            HSIBufferOffset = sagebase.bootSettings.HSIBufferOffset;
        }
    }

    rc = SAGE_Boot_Region_Check(regionMapNum,pRegionMap);
    if(rc != BERR_SUCCESS)
    {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s %d region map not valid!!",__FUNCTION__,__LINE__));
        goto err;
    }

    rc = SAGE_Boot_ReadRegionMap(regionMapNum,pRegionMap);
    if(rc != BERR_SUCCESS)
    {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s %d SAGE_Boot_ReadRegionMap failed!!",__FUNCTION__,__LINE__));
        goto err;
    }

    rc = SAGE_Boot_Rpc_Init(HSIBufferOffset);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SAGE_Rpc_Init failed.", __FUNCTION__));
        goto err;
    }

    if(sagebase.bootSettings.HSIBufferOffset != 0)
    {
        /* sage already booted */
        /* send command for Region map change */
        goto err;
    }

    sagebase.bootSettings.HSIBufferOffset = HSIBufferOffset;

    pBootRegionMap = (BSAGElib_RegionInfo *)Sage_Malloc(regionMapNum*sizeof(BSAGElib_RegionInfo));
    if(pBootRegionMap == NULL)
    {
        rc = BERR_OUT_OF_DEVICE_MEMORY;
        BDBG_ERR(("%s %d alloc %d boot regionMap memeory failed!!",__FUNCTION__,__LINE__,regionMapNum));
        goto err;
    }

    sagebase.bootSettings.regionMapOffset = Sage_AddrToOffset(pBootRegionMap);

    rc = SAGE_Boot_WriteRegionMap(pBootRegionMap);
    if(rc != BERR_SUCCESS) {
        printk("%s %d load RegionMap failed !\n",__FUNCTION__,__LINE__);
        goto err;
    }

    bootloaderDev_filename = "sage_bl_dev.bin";
    framworkDev_filename = "sage_framework_dev.bin";
    bootloader_filename = "sage_bl.bin";
    framwork_filename = "sage_framework.bin";

    rc = SAGE_Boot_Load(bootloader_filename,&sagebase.bootSettings.bootloaderOffset,&sagebase.bootSettings.bootloaderSize);
    if(rc != BERR_SUCCESS) {
        printk("%s %d load %s failed !\n",__FUNCTION__,__LINE__,bootloader_filename);
        goto err_load1;
    }

    rc = SAGE_Boot_Load(framwork_filename,&sagebase.bootSettings.frameworkOffset,&sagebase.bootSettings.frameworkSize);
    if(rc != BERR_SUCCESS) {
        printk("%s %d load %s failed !\n",__FUNCTION__,__LINE__,framwork_filename);
        goto err_load2;
    }

    rc = SAGE_Boot_Load(bootloaderDev_filename,&sagebase.bootSettings.bootloaderDevOffset,&sagebase.bootSettings.bootloaderDevSize);
    if(rc != BERR_SUCCESS) {
        printk("%s %d load %s failed !\n",__FUNCTION__,__LINE__,bootloaderDev_filename);
        goto err_load3;
    }

    rc = SAGE_Boot_Load(framworkDev_filename,&sagebase.bootSettings.frameworkDevOffset,&sagebase.bootSettings.frameworkDevSize);
    if(rc != BERR_SUCCESS) {
        printk("%s %d load %s failed !\n",__FUNCTION__,__LINE__,framworkDev_filename);
        goto err_load4;
    }

    SAGE_Hsm_Init();

    vkl = SAGE_AllocSageVkl();
    if(vkl == BCMD_VKL_eMax)
    {
        rc = BERR_LEAKED_RESOURCE;
        BDBG_ERR(("%s %d fail to alloc VKL!\n",__FUNCTION__,__LINE__));
        goto err_core;
    }
    sagebase.bootSettings.vkl1 = vkl;

    vkl = SAGE_AllocSageVkl();
    if(vkl == BCMD_VKL_eMax)
    {
        rc = BERR_LEAKED_RESOURCE;
        BDBG_ERR(("%s %d fail to alloc VKL!\n",__FUNCTION__,__LINE__));
        goto err_core;
    }
    sagebase.bootSettings.vkl2 = vkl;

    vkl = SAGE_AllocSageVkl();
    if(vkl == BCMD_VKL_eMax)
    {
        rc = BERR_LEAKED_RESOURCE;
        BDBG_ERR(("%s %d fail to alloc VKL!\n",__FUNCTION__,__LINE__));
        goto err_core;
    }
    sagebase.bootSettings.vklBoot = vkl;

    rc = BSAGE_Boot_Launch(sagebase.hHsm,&sagebase.bootSettings);
    if(rc != BERR_SUCCESS) {
        printk("%s %d BSAGE_Boot_Launch failed !\n",__FUNCTION__,__LINE__);
        goto err_core;
    }

    PINFO("Initialization complete\n");

    BSAGE_Boot_GetInfo(&sagebase.chipInfo,&sagebase.bootloaderInfo,&sagebase.frameworkInfo);

err_core:
    SAGE_Hsm_Uninit();
err_load4:
    if(sagebase.bootSettings.frameworkDevOffset)
        Sage_MfreeOffset(sagebase.bootSettings.frameworkDevOffset);
err_load3:
    if(sagebase.bootSettings.bootloaderDevOffset)
        Sage_MfreeOffset(sagebase.bootSettings.bootloaderDevOffset);
err_load2:
    if(sagebase.bootSettings.frameworkOffset)
        Sage_MfreeOffset(sagebase.bootSettings.frameworkOffset);
err_load1:
    if(sagebase.bootSettings.bootloaderOffset)
        Sage_MfreeOffset(sagebase.bootSettings.bootloaderOffset);
err:
    if(pBootRegionMap)
        Sage_Mfree(pBootRegionMap);
    return rc;
}

static BERR_Code SAGE_Boot_GetInfo(BSAGElib_ChipInfo *pChipInfo,BSAGElib_ImageInfo *pBootloaderInfo,BSAGElib_ImageInfo *pFrameworkInfo)
{
    return BSAGE_Boot_GetInfo(pChipInfo,pBootloaderInfo,pFrameworkInfo);
}
static BERR_Code SAGE_GetStatus(BSAGElib_Status *pStatus)
{
    return BSAGE_GetStatus(pStatus);
}
static void SAGE_Management_Reset(void)
{
    BSAGE_Management_Reset();
}

EXPORT_SYMBOL(SAGE_Boot_RegionInfo);
EXPORT_SYMBOL(SAGE_Boot_GetInfo);
EXPORT_SYMBOL(SAGE_GetStatus);
EXPORT_SYMBOL(SAGE_Management_Reset);

static int SAGE_BootSetting_Check(BSAGE_BootSettings *pBootSettings)
{
    BERR_Code rc = BERR_SUCCESS;

    if(pBootSettings == NULL)
    {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s %d NULL pBootSettings !!",__FUNCTION__,__LINE__));
        goto err;
    }

    if(pBootSettings->HSIBufferOffset == 0)
    {
        if(sagebase.bootSettings.HSIBufferOffset == 0)
        {
            rc = BERR_INVALID_PARAMETER;
            BDBG_ERR(("%s %d HSIBufferOffset is zero!!",__FUNCTION__,__LINE__));
            goto err;
        }
        if(sagebase.bootSettings.HSIBufferOffset != pBootSettings->HSIBufferOffset)
        {
            BDBG_WRN(("%s %d HSIBufferOffset(0x%lx) is different from old(0x%lx)!! HSIBufferOffset can not change, set it to old value !!",__FUNCTION__,__LINE__,pBootSettings->HSIBufferOffset,sagebase.bootSettings.HSIBufferOffset));
            pBootSettings->HSIBufferOffset = sagebase.bootSettings.HSIBufferOffset;
        }
    }

err:
    return rc;
}

static BERR_Code
SAGE_Boot_Launch(
    BHSM_Handle hHsm,
    BSAGE_BootSettings *pBootSettings)
{
    BERR_Code rc;
    BSAGElib_RegionInfo *pRegionMap;
    uint8_t *pPage = NULL;
    uint32_t pageoffset;

    /* check bootsetting map except regionMap */
    rc = SAGE_BootSetting_Check(pBootSettings);
    if(rc != 0)
    {
        BDBG_ERR(("%s %d Fail to read CRR info from pBootSettings region Map!!"));
        goto err;
    }

    pRegionMap = (BSAGElib_RegionInfo *)Sage_OffsetToAddr(pBootSettings->regionMapOffset);
    if(pRegionMap == NULL)
    {
        pageoffset = pBootSettings->regionMapOffset&(~(PAGE_SIZE-1));
        pPage = (uint8_t *)brcmstb_memory_kva_map_phys(pageoffset, PAGE_SIZE*2,true);
        if ( NULL == pPage) {
            rc = BERR_OS_ERROR;
            BDBG_ERR(("%s %d unable to map regionMapOffset pageoffset 0x%xlx!",__FUNCTION__,__LINE__,pageoffset));
            goto err;
        }
        pRegionMap = (BSAGElib_RegionInfo *)(pPage + (pBootSettings->regionMapOffset - pageoffset));
    }

    if(pRegionMap == NULL)
    {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s %d Fail to map regionMapOffset(offset 0x%lx) addr!!",__FUNCTION__,__LINE__,pBootSettings->regionMapOffset));
        goto err;
    }

    rc = SAGE_Boot_Region_Check(pBootSettings->regionMapNum,pRegionMap);
    if(rc != 0)
    {
        BDBG_ERR(("%s %d region Map check fail !!",__FUNCTION__,__LINE__));
        goto err;
    }

    rc = SAGE_Boot_ReadRegionMap(pBootSettings->regionMapNum,pRegionMap);

    if(pPage != NULL)
        brcmstb_memory_kva_unmap(pPage);

    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SAGE_Boot_ReadRegionMap failed.", __FUNCTION__));
        goto err;
    }

    rc = SAGE_Boot_Rpc_Init(pBootSettings->HSIBufferOffset);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SAGE_Rpc_Init failed.", __FUNCTION__));
        goto err;
    }

    if(sagebase.bootSettings.HSIBufferOffset != 0)
    {
        /* sage already booted */
        /* send command for Region map change */
        goto err;
    }

    sagebase.bootSettings.HSIBufferOffset = pBootSettings->HSIBufferOffset;

    SAGE_Hsm_Init();

    rc = BSAGE_Boot_Launch(sagebase.hHsm,pBootSettings);
    if(rc != BERR_SUCCESS) {
        printk("%s %d BSAGE_Boot_Launch failed !\n",__FUNCTION__,__LINE__);
        goto err_core;
    }

    PINFO("Initialization complete\n");

    BSAGE_Boot_GetInfo(&sagebase.chipInfo,&sagebase.bootloaderInfo,&sagebase.frameworkInfo);

err_core:
    SAGE_Hsm_Uninit();
err:
    return rc;
}
EXPORT_SYMBOL(SAGE_Boot_Launch);

#define BUILD_RO_FOPS(name)					\
static int proc_open_##name(struct inode *inode, struct file *file) \
{								\
	return single_open(file, proc_read_##name, NULL);	\
}								\
static const struct file_operations read_fops_##name = {	\
	.open	=	proc_open_##name,			\
	.read	=	seq_read,				\
	.llseek =	seq_lseek,				\
	.release = 	seq_release,				\
};

#define BUILD_RW_FOPS(name)					\
static int proc_open_##name(struct inode *inode, struct file *file) \
{								\
	return single_open(file, proc_read_##name, file);	\
}								\
static const struct file_operations rw_fops_##name = {	\
	.open	=	proc_open_##name,			\
	.read	=	seq_read,				\
	.write	=	proc_write_##name,			\
	.llseek =	seq_lseek,				\
	.release = 	seq_release,				\
};

#define BUILD_IOCTL_FOPS(name)					\
static int proc_open_##name(struct inode *inode, struct file *file) \
{								\
	return single_open(file, proc_read_##name, file);	\
}								\
static const struct file_operations ioctl_fops_##name = {	\
	.open	=	proc_open_##name,			\
	.read	=	seq_read,				\
	.write	=	proc_write_##name,			\
	.llseek =	seq_lseek,				\
	.release = 	proc_release_##name,				\
    .unlocked_ioctl	= proc_ioctl_##name,	\
};

static BSAGE_RpcRemoteHandle SAGE_AddRemote(uint32_t platformId,uint32_t moduleId,void *async_argument,uint64_t messageOffset);
static void SAGE_RemoveRemote(BSAGE_RpcRemoteHandle remote);
static BERR_Code SAGE_SendCommand(BSAGE_RpcRemoteHandle remote,BSAGElib_RpcCommand *command,uint32_t *pAsync_id);
static BERR_Code SAGE_SendCallbackResponse(BSAGE_RpcRemoteHandle remote,uint32_t sequenceId,BERR_Code retCode);
static BERR_Code SAGE_RegisterCallback(SAGE_Event event,void *callback,void *context,BSAGE_RpcRemoteHandle remote);
static BERR_Code SAGE_UnRegisterCallback(SAGE_Event event,BSAGE_RpcRemoteHandle remote);
static void SAGE_ResponseCallback(struct platform *p,BERR_Code response_rc);
static void SAGE_IndicationRecvCallback(struct platform *p,void *async_argument,uint32_t indication_id,uint32_t value);
static BERR_Code SAGE_RequestCallback(struct platform *p,void *async_argument);
static void SAGE_TATerminateCallback(struct platform *p,void *async_argument,uint32_t reason,uint32_t source);
static void SAGE_WatchdogCallback(struct platform *p);
static void *SAGE_AddWatchdog(void *callback,void *context);

static int proc_read_platform(struct seq_file *seq, void *v)
{
    struct platform *p = PDE_DATA(((struct file *)(seq->private))->f_inode);

    if(p == NULL)
    {
        printk("%s platform NULL\n",__FUNCTION__);
        goto err;
    }

    if(p->watchdog.watchdog_isr != NULL)
        seq_printf(seq,"0x%08lx \nwatchdog callback %p \n",(long unsigned)p,p->watchdog.watchdog_isr);
    else
        seq_printf(seq,"0x%08lx \nplatformID 0x%3x moduleId 0x%1x async_argument %p messageOffset 0x%llx\n",
                  (long unsigned )p,p->platformId,p->moduleId,p->async_argument,p->messageOffset);
err:
    return 0;
}

static ssize_t proc_write_platform(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    char commandBuffer[100];
    size_t received;
    ssize_t rv = count;
    BERR_Code rc;
    BSAGE_RpcRemoteHandle remote;
    struct platform *p = PDE_DATA(((struct file *)file)->f_inode);

    if(p == NULL)
    {
        rv = ENXIO;
        printk("%s platform NULL\n",__FUNCTION__);
        goto err;
    }

    if(p->watchdog.watchdog_isr != NULL)
    {
        rv = ENXIO;
        printk("watchdog callback %p, can not accept commands\n",p->watchdog.watchdog_isr);
        goto err;
    }

    if(count > 100)
    {
        rv = ERANGE;
        printk("%s command buffer size %d too large \n",__FUNCTION__,count);
        goto err;
    }

    received = copy_from_user(commandBuffer,buffer,count);

    commandBuffer[count - received - 1] = 0;

    if(count > 30)
    {
        BSAGElib_RpcCommand command;
        uint32_t Async_id;

        sscanf(commandBuffer,"%lx %lx %lx %llx %lx",(long unsigned *)&remote,(unsigned long *)&command.systemCommandId,(unsigned long *)&command.moduleCommandId,(unsigned long long *)&command.containerOffset,(long unsigned *)&command.containerVAddr);

        rc = SAGE_SendCommand((BSAGE_RpcRemoteHandle)p,&command,&Async_id);

        if(rc == BERR_SUCCESS)
            rv = Async_id + 100;
        else
            rv = EIO;
    }else{
        uint32_t sequenceId;
        BERR_Code retCode;

        sscanf(buffer,"%lx %lx %x",(long unsigned *)&remote,(unsigned long *)&sequenceId,&retCode);

        rc = SAGE_SendCallbackResponse((BSAGE_RpcRemoteHandle)p,sequenceId,retCode);

        if(rc == BERR_SUCCESS)
            rv = count;
        else
            rv = EIO;
    }
err:
    return rv;
}

static long proc_ioctl_platform(struct file *file, unsigned int command, unsigned long arg)
{
    struct sk_buff *skb;
    struct platform *p = PDE_DATA(((struct file *)file)->f_inode);
    long rc = 0;

    if(p == NULL)
    {
        printk("%s platform NULL\n",__FUNCTION__);
        goto err;
    }

    switch(command)
    {
    case SAGE_IOCTL_WAIT_FOR_CALLBACK:
    {
        rc = down_interruptible(&p->callbackSem);
        if(rc != 0)
        {
//            printk("%s %d down_interruptible interrupted \n",__FUNCTION__,__LINE__);
        }
        skb = skb_dequeue(&p->callbackQueue);
        if(skb != NULL)
        {
            struct callback_message *msg;
            msg = (struct callback_message *) skb->data;

            rc = copy_to_user((void __user *)arg,msg,sizeof(struct callback_message));
        }
        break;
    }
    case SAGE_IOCTL_REGISTER_msgIndication_CALLBACK:
    {
        p->rpc.indicationRecv_isr = (BSAGElib_Rpc_IndicationRecvCallback)arg;
        rc = SAGE_RegisterCallback(BSAGE_Event_msgIndication,SAGE_IndicationRecvCallback,p,(BSAGE_RpcRemoteHandle)p);
        break;
    }
    case SAGE_IOCTL_REGISTER_msgResponse_CALLBACK:
    {
        p->rpc.response_isr = (BSAGE_Rpc_DispatchResponse_isr)arg;
        rc = SAGE_RegisterCallback(BSAGE_Event_msgResponse,SAGE_ResponseCallback,p,(BSAGE_RpcRemoteHandle)p);
        break;
    }
    case SAGE_IOCTL_REGISTER_msgCallbackRequest_CALLBACK:
    {
        p->rpc.callbackRequest_isr = (BSAGElib_Rpc_CallbackRequestISRCallback)arg;
        rc = SAGE_RegisterCallback(BSAGE_Event_msgCallbackRequest,SAGE_RequestCallback,p,(BSAGE_RpcRemoteHandle)p);
        break;
    }
    case SAGE_IOCTL_REGISTER_msgTATermination_CALLBACK:
    {
        p->rpc.taTerminate_isr = (BSAGElib_Rpc_TATerminateCallback)arg;
        rc = SAGE_RegisterCallback(BSAGE_Event_msgTATermination,SAGE_TATerminateCallback,p,(BSAGE_RpcRemoteHandle)p);
        break;
    }
    case SAGE_IOCTL_REGISTER_watchdog_CALLBACK:
    {
        p->watchdog.watchdog_isr = (BSAGElib_Management_WatchdogCallback)arg;
        rc = SAGE_RegisterCallback(BSAGE_Event_watchdog,SAGE_WatchdogCallback,p,(BSAGE_RpcRemoteHandle)p);
        break;
    }
    case SAGE_IOCTL_REGISTER_msgIndication_CONTEXT:
    {
        p->rpc.indicationContext = (void *)arg;
        break;
    }
    case SAGE_IOCTL_REGISTER_msgResponse_CONTEXT:
    {
        p->rpc.responseContext = (void *)arg;
        break;
    }
    case SAGE_IOCTL_REGISTER_msgCallbackRequest_CONTEXT:
    {
        p->rpc.callbackRequestContext = (void *)arg;
        break;
    }
    case SAGE_IOCTL_REGISTER_msgTATermination_CONTEXT:
    {
        p->rpc.taTerminationContext = (void *)arg;
        break;
    }
    default:
        break;
    }

err:
    if(rc)
    {
//        BDBG_ERR(("%s %d IOCTL command %d arg 0x%lx failed %d!",__FUNCTION__,__LINE__,command,arg,rc));
    }
    return rc;
}

static int proc_release_platform(struct inode *inode, struct file *file)
{
    struct platform *p = PDE_DATA(((struct file *)file)->f_inode);

    SAGE_RemoveRemote((BSAGE_RpcRemoteHandle)p);

    return seq_release(inode, file);
}

static int proc_read_remove_watchdog(struct seq_file *seq, void *v)
{
	seq_printf(seq,"To remove a watchdog, then write the watchdog ID(filename) here \n");

	return  0;
}

static ssize_t proc_write_remove_watchdog(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    struct platform *p;
    char platform[100];
    size_t received;
    int rv = count;

    if(count > 100)
    {
        rv = ERANGE;
        printk("%s command buffer size %d too large\n",__FUNCTION__,count);
        goto err;
    }

    received = copy_from_user(platform,buffer,count);

    platform[count - received - 1] = 0;

    sscanf(platform,"%lx",(long unsigned *) &p);

    if( p == NULL)
    {
        printk("platform context NULL\n");
        goto err;
    }

    SAGE_RemoveRemote((BSAGE_RpcRemoteHandle) p);

err:
    return rv;
}

static int proc_read_create_watchdog(struct seq_file *seq, void *v)
{
	seq_printf(seq,"To create the watchdog , write watchdogID(callback function) here\n");

	return  0;
}

static ssize_t proc_write_create_watchdog(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    BSAGE_RpcRemoteHandle p;
    char createInput[100];
    size_t received;
    void *callback,*context;
    int cnt,rv = count;

    if(count > 100)
    {
        rv = ERANGE;
        printk("%s command buffer size %d too large\n",__FUNCTION__,count);
        goto err;
    }

    received = copy_from_user(createInput,buffer,count);

    createInput[count - received - 1] = 0;

    cnt = sscanf(createInput,"%lx %lx",(long signed *)&callback,(long unsigned *)&context);

    p = SAGE_AddWatchdog(callback,context);
    if(p == NULL)
        rv = EIO;

err:
    return rv;
}

static int proc_read_remove_platform(struct seq_file *seq, void *v)
{
	seq_printf(seq,"To remove a platform, then write the platform filename here \n");

	return  0;
}

static ssize_t proc_write_remove_platform(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    struct platform *p;
    char platform[100];
    size_t received;
    int rv = count;

    if(count > 100)
    {
        rv = ERANGE;
        printk("%s command buffer size %d too large\n",__FUNCTION__,count);
        goto err;
    }

    received = copy_from_user(platform,buffer,count);

    platform[count - received - 1] = 0;

    sscanf(platform,"%lx",(long unsigned *) &p);

    if( p == NULL)
    {
        printk("platform context NULL\n");
        goto err;
    }

    SAGE_RemoveRemote((BSAGE_RpcRemoteHandle) p);

err:
    return rv;
}

static int proc_read_create_platform(struct seq_file *seq, void *v)
{
	seq_printf(seq,"To create the platform , write 'platform ID module ID async_argument,&messageOffset'\n");

	return  0;
}

static ssize_t proc_write_create_platform(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    BSAGE_RpcRemoteHandle p;
    char createInput[100];
    size_t received;
    uint32_t platformId;
    uint32_t moduleId;
    void *async_argument;
    uint64_t messageOffset;
    int cnt,rv=count;

    if(count > 100)
    {
        rv = ERANGE;
        printk("%s command buffer size %d too large\n",__FUNCTION__,count);
        goto err;
    }

    received = copy_from_user(createInput,buffer,count);

    createInput[count - received - 1] = 0;

    cnt = sscanf(createInput,"%3x %2x %lx %llx",&platformId,&moduleId,(long unsigned *)&async_argument,&messageOffset);

    p = SAGE_AddRemote(platformId,moduleId,async_argument,messageOffset);
    if(p == NULL)
        rv = EIO;

err:
    return rv;
}

static int proc_read_bootRegionInfo(struct seq_file *seq, void *v)
{
    int i;
    seq_printf(seq,"write to /proc/sage/boot/bootRegionInfo with RegionInfo will boot sage\n");

    seq_printf(seq,"regionMapNum %d\n",sagebase.bootSettings.regionMapNum);
    for(i = 0; i < sagebase.bootSettings.regionMapNum && i <REGION_MAP_MAX_NUM; i++)
    {
        switch(sagebase.regionMap[i].id)
        {
        case BSAGElib_RegionId_Glr:
            seq_printf(seq,"GLR     Offset 0x%lx size 0x%lx\n",(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
/*            seq_printf(seq,"GLR     Offset 0x%lx size 0x%lx\n",(long unsigned)sagebase.GlrOffset,          (long unsigned)sagebase.GlrSize);*/
            break;
        case BSAGElib_RegionId_Glr2:
            seq_printf(seq,"GLR2    Offset 0x%lx size 0x%lx\n",(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
            break;
        case BSAGElib_RegionId_Srr:
            seq_printf(seq,"SRR     Offset 0x%lx size 0x%lx\n",(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
            break;
        case BSAGElib_RegionId_Crr:
            seq_printf(seq,"CRR     Offset 0x%lx size 0x%lx\n",(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
            break;
        case BSAGElib_RegionId_Xrr:
            seq_printf(seq,"XRR     Offset 0x%lx size 0x%lx\n",(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
            break;
        case BSAGElib_RegionId_Urr0:
            seq_printf(seq,"URR0    Offset 0x%lx size 0x%lx\n",(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
            break;
        case BSAGElib_RegionId_Urr1:
            seq_printf(seq,"URR1    Offset 0x%lx size 0x%lx\n",(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
            break;
        case BSAGElib_RegionId_Urr2:
            seq_printf(seq,"URR2    Offset 0x%lx size 0x%lx\n",(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
            break;
        case BSAGElib_RegionId_SecGfx:
            seq_printf(seq,"SECGFX  Offset 0x%lx size 0x%lx\n",(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
            break;
        default:
            seq_printf(seq,"R %4d   Offset 0x%lx size 0x%lx\n",sagebase.regionMap[i].id,(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
            break;
        }
    }
    return  0;
}

static ssize_t proc_write_bootRegionInfo(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    BERR_Code rc = BERR_SUCCESS;
    uint32_t HSIBufferOffset,regionMapNum;
    BSAGElib_RegionInfo pRegionMap[REGION_MAP_MAX_NUM];
    size_t received;
    int rv=count,cnt;
    char input[300],*nextline;
    uint8_t i;

    if(count < 100 || count > 30*(REGION_MAP_MAX_NUM+1))
    {
        rv = ERANGE;
        printk("%s %d received buffer size %d not right (%d %d)\n",__FUNCTION__,__LINE__,count,100,30*(REGION_MAP_MAX_NUM+1));
        goto err;
    }

    received = copy_from_user(input,buffer,count);

    if(received != 0)
    {
        rv = ERANGE;
        printk("%s %d read BSAGElib_RegionInfo Error cout %d!\n",__FUNCTION__,__LINE__,count);
        goto err;
    }

/*    printk("%s\n",input);*/

    cnt = sscanf(input,"%lx %lx\n",(unsigned long *)&HSIBufferOffset,(unsigned long *)&regionMapNum);
    if(cnt != 2 || HSIBufferOffset == 0 || regionMapNum > REGION_MAP_MAX_NUM)
    {
        rv = ERANGE;
        printk("%s %d read HSIBufferOffset 0x%lx or regionMapNum %lu out of range!\n",__FUNCTION__,__LINE__,(unsigned long)HSIBufferOffset,(unsigned long)regionMapNum);
        goto err;
    }

/*    printk("%lx %lx\n",(unsigned long )HSIBufferOffset,(unsigned long )regionMapNum);*/

    nextline = input;
    for(i = 0; i< regionMapNum && i < REGION_MAP_MAX_NUM;i++)
    {
        nextline = strchr(nextline,'\n')+1;
        if(nextline == NULL)
        {
            rv = ERANGE;
            printk("%s %d read the %d line wrong!\n",__FUNCTION__,__LINE__,i+1);
            goto err;
        }
/*        printk("line %d:%s\n",i,nextline);*/

        cnt = sscanf(nextline,"%lx %lx %lx\n",(unsigned long *)&pRegionMap[i].id,(unsigned long *)&pRegionMap[i].offset,(unsigned long *)&pRegionMap[i].size);
        if(cnt != 3 || pRegionMap[i].id > BSAGElib_RegionId_SecGfx || pRegionMap[i].offset == 0 || pRegionMap[i].size == 0)
        {
            rv = ERANGE;
            printk("%s %d read HSIBufferOffset 0x%lx or regionMapNum %lu out of range!\n",__FUNCTION__,__LINE__,(unsigned long)HSIBufferOffset,(unsigned long)regionMapNum);
            goto err;
        }
/*        printk("%lx %lx %lx\n",(unsigned long )pRegionMap[i].id,(unsigned long )pRegionMap[i].offset,(unsigned long )pRegionMap[i].size);*/
    }

    rc = SAGE_Boot_RegionInfo(HSIBufferOffset,regionMapNum,pRegionMap);

err:
    return rv;
}

static int proc_read_toreset(struct seq_file *seq, void *v)
{
    seq_printf(seq,"write to /proc/sage/toreset will reset sage(wihtout bootloader, framework loadded)\n");

    return  0;
}

static ssize_t proc_write_toreset(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    int rv=count;

    BSAGE_Management_Reset();

    return rv;
}

static ssize_t proc_write_bootSettings(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    size_t received;
    int rv=count;
    BSAGE_BootSettings bootSettings;
    BERR_Code rc;

    if(count != sizeof(BSAGE_BootSettings))
    {
        rv = ERANGE;
        printk("%s received buffer size %d not the same as sizeof(BSAGE_BootSettings)\n",__FUNCTION__,count);
        goto err;
    }

    received = copy_from_user(&bootSettings,buffer,count);

    if(received != 0)
    {
        rv = ERANGE;
        printk("%s read bootSetting Error cout %d!\n",__FUNCTION__,count);
        goto err;
    }
/*
    printk(        "Bootloader Offset 0x%8lx size 0x%lx\n"
                   "Framework  Offset 0x%8lx size 0x%lx\n"
                   "RegionMap  Offset 0x%8lx num    %lu\n"
                   "HsiBuffer  Offset 0x%8lx size 0x%lx\n"
                   "VKL1       %d\n"
                   "VKL2       %d\n"
                   "VKLBoot    %d\n",
        (long unsigned)bootSettings.bootloaderOffset,(long unsigned)bootSettings.bootloaderSize,
        (long unsigned)bootSettings.frameworkOffset, (long unsigned)bootSettings.frameworkSize,
        (long unsigned)bootSettings.regionMapOffset, (long unsigned)bootSettings.regionMapNum,
        (long unsigned)bootSettings.HSIBufferOffset, (long unsigned)SAGE_HOST_BUF_SIZE,
        bootSettings.vkl1,bootSettings.vkl2,bootSettings.vklBoot);
*/
    rc = SAGE_Boot_Launch(NULL,&bootSettings);
    if(rc != BERR_SUCCESS)
    {
        rv = ERANGE;
        printk("%s SAGE_Boot_Launch failed\n",__FUNCTION__);
        goto err;
    }

err:
    return rv;
}

static int proc_read_bootSettings(struct seq_file *seq, void *v)
{
    int i;

    seq_printf(seq,"Write here with bootSettings will boot Sage\n\n"
                   "Bootloader    Offset 0x%8lx size 0x%lx\n"
                   "Framework     Offset 0x%8lx size 0x%lx\n"
                   "BootloaderDev Offset 0x%8lx size 0x%lx\n"
                   "FrameworkDev  Offset 0x%8lx size 0x%lx\n"
                   "RegionMap     Offset 0x%8lx num    %lu\n"
                   "HsiBuffer     Offset 0x%8lx size 0x%lx\n"
                   "VKL1          %d\n"
                   "VKL2          %d\n"
                   "VKLBoot       %d\n",
        (long unsigned)sagebase.bootSettings.bootloaderOffset,   (long unsigned)sagebase.bootSettings.bootloaderSize,
        (long unsigned)sagebase.bootSettings.frameworkOffset,    (long unsigned)sagebase.bootSettings.frameworkSize,
        (long unsigned)sagebase.bootSettings.bootloaderDevOffset,(long unsigned)sagebase.bootSettings.bootloaderDevSize,
        (long unsigned)sagebase.bootSettings.frameworkDevOffset, (long unsigned)sagebase.bootSettings.frameworkDevSize,
        (long unsigned)sagebase.bootSettings.regionMapOffset,    (long unsigned)sagebase.bootSettings.regionMapNum,
        (long unsigned)sagebase.bootSettings.HSIBufferOffset,    (long unsigned)SAGE_HOST_BUF_SIZE,
        sagebase.bootSettings.vkl1,sagebase.bootSettings.vkl2,sagebase.bootSettings.vklBoot);

    for(i = 0; i < sagebase.bootSettings.regionMapNum && i <REGION_MAP_MAX_NUM; i++)
    {
        switch(sagebase.regionMap[i].id)
        {
        case BSAGElib_RegionId_Glr:
            seq_printf(seq,"GLR     Offset 0x%lx size 0x%lx\n",(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
            break;
        case BSAGElib_RegionId_Glr2:
            seq_printf(seq,"GLR2    Offset 0x%lx size 0x%lx\n",(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
            break;
        case BSAGElib_RegionId_Srr:
            seq_printf(seq,"SRR     Offset 0x%lx size 0x%lx\n",(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
            break;
        case BSAGElib_RegionId_Crr:
            seq_printf(seq,"CRR     Offset 0x%lx size 0x%lx\n",(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
            break;
        case BSAGElib_RegionId_Xrr:
            seq_printf(seq,"XRR     Offset 0x%lx size 0x%lx\n",(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
            break;
        case BSAGElib_RegionId_Urr0:
            seq_printf(seq,"URR0    Offset 0x%lx size 0x%lx\n",(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
            break;
        case BSAGElib_RegionId_Urr1:
            seq_printf(seq,"URR1    Offset 0x%lx size 0x%lx\n",(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
            break;
        case BSAGElib_RegionId_Urr2:
            seq_printf(seq,"URR2    Offset 0x%lx size 0x%lx\n",(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
            break;
        case BSAGElib_RegionId_SecGfx:
            seq_printf(seq,"SECGFX  Offset 0x%lx size 0x%lx\n",(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
            break;
        default:
            seq_printf(seq,"R %4d   Offset 0x%lx size 0x%lx\n",sagebase.regionMap[i].id,(long unsigned)sagebase.regionMap[i].offset,(long unsigned)sagebase.regionMap[i].size);
            break;
        }
    }

    return  0;
}
static int proc_read_bootStatus(struct seq_file *seq, void *v)
{
    BERR_Code rc;
    BSAGElib_Status status;

    rc = BSAGE_GetStatus(&status);

    if(rc == BERR_SUCCESS)
    {
        if(status.urr.secured)
            seq_printf(seq,"status.urr.secured true\n");
        else
            seq_printf(seq,"status.urr.secured false\n");
    }else{
        seq_printf(seq,"status.urr.secured NA\n");
    }
    return 0;
}
static int proc_read_bootInfo(struct seq_file *seq, void *v)
{
    seq_printf(seq,"chipType %d ",
                       sagebase.chipInfo.chipType);

    switch(sagebase.chipInfo.chipType)
    {
    case BSAGElib_ChipType_eZS:
        seq_printf(seq,"(BSAGElib_ChipType_eZS)");
        break;
    case BSAGElib_ChipType_eZB:
        seq_printf(seq,"(BSAGElib_ChipType_eZB)");
        break;
    case BSAGElib_ChipType_eCustomer:
        seq_printf(seq,"(BSAGElib_ChipType_eCustomer)");
        break;
    case BSAGElib_ChipType_eCustomer1:
        seq_printf(seq,"(BSAGElib_ChipType_eCustomer1)");
        break;
    default:
        break;
    }

    seq_printf(seq,"\n0x%08lx %s\n",
                       (long unsigned)sagebase.bootloaderInfo.THLShortSig,
                       sagebase.bootloaderInfo.versionString);

    seq_printf(seq,"0x%08lx %s\n",
                       (long unsigned)sagebase.frameworkInfo.THLShortSig,
                       sagebase.frameworkInfo.versionString);

    return  0;
}

static int proc_read_bmemInfo(struct seq_file *seq, void *v)
{
    struct brcmstb_memory osmem;
    int i,j;

    brcmstb_memory_get(&osmem);

    for(i=0;i<MAX_BRCMSTB_MEMC;i++)
    {
        for(j=0;j<osmem.memc[i].count && j<MAX_BRCMSTB_RANGE;j++)
        {
            seq_printf(seq," memc[%d] addr 0x%8llx size 0x%8llx\n",i,osmem.memc[i].range[j].addr,osmem.memc[i].range[j].size);
        }
    }

    for(j=0;j<osmem.lowmem.count && j<MAX_BRCMSTB_RANGE;j++)
    {
        seq_printf(seq,"  lowmem addr 0x%8llx size 0x%8llx\n",osmem.lowmem.range[j].addr,osmem.lowmem.range[j].size);
    }

    for(j=0;j<osmem.bmem.count && j<MAX_BRCMSTB_RANGE;j++)
    {
        seq_printf(seq,"    bmem addr 0x%8llx size 0x%8llx\n",osmem.bmem.range[j].addr,osmem.bmem.range[j].size);
    }

    for(j=0;j<osmem.cma.count && j<MAX_BRCMSTB_RANGE;j++)
    {
        seq_printf(seq,"     cma addr 0x%8llx size 0x%8llx\n",osmem.cma.range[j].addr,osmem.cma.range[j].size);
    }

    for(j=0;j<osmem.reserved.count && j<MAX_BRCMSTB_RANGE;j++)
    {
        seq_printf(seq,"reserved addr 0x%8llx size 0x%8llx\n",osmem.reserved.range[j].addr,osmem.reserved.range[j].size);
    }
    return  0;
}

static int SAGE_CallbackTask(void * data)
{
    int rc;
    struct platform *p = (struct platform *)data;
    if(p == NULL)
    {
        BDBG_ERR(("%s NULL platform(Remote) handle!",__FUNCTION__));
        return -1;
    }

    while(p->remote != NULL || p->watchdog.watchdog_isr != NULL)
    {
        struct sk_buff *skb;

        rc = down_interruptible(&p->callbackSem);
        if(rc != 0)
            goto err;

        skb = skb_dequeue(&p->callbackQueue);
        if(skb != NULL)
        {
            struct callback_message *msg;
            msg = (struct callback_message *) skb->data;
            switch(msg->event)
            {
            case BSAGE_Event_msgResponse:
            {
                BERR_Code response_rc = (BERR_Code) msg->context1;

                if(p->rpc.response_isr)
                {
                    p->rpc.response_isr(p->rpc.responseContext, response_rc);
                }
                break;
            }
            case BSAGE_Event_msgIndication:
            {
                uint32_t indication_id = msg->context1;
                uint32_t value = msg->context2;

                if(p->rpc.indicationRecv_isr)
                    p->rpc.indicationRecv_isr(p->rpc.indicationContext, p->async_argument, indication_id, value);
                break;
            }
            case BSAGE_Event_msgCallbackRequest:
            {
                if(p->rpc.callbackRequest_isr)
                    p->rpc.callbackRequest_isr(p->rpc.callbackRequestContext, p->async_argument);
                break;
            }
            case BSAGE_Event_msgTATermination:
            {
                uint32_t reason      = msg->context1;
                uint32_t source      = msg->context2;

                if(p->rpc.taTerminate_isr)
                    p->rpc.taTerminate_isr(p->rpc.taTerminationContext, p->async_argument, reason, source);
                break;
            }
            case BSAGE_Event_watchdog:
            {
                if(p->watchdog.watchdog_isr)
                    p->watchdog.watchdog_isr();
                break;
            }
            default:
                break;
            }
            kfree_skb(skb);
        }

    }
err:
    return 0;
}

static void SAGE_ResponseCallback(
    struct platform *p,
    BERR_Code response_rc)
{
    struct sk_buff *skb;

    if(p == NULL)
    {
        BDBG_ERR(("%s NULL platform(Remote) handle!",__FUNCTION__));
        return;
    }

    skb = dev_alloc_skb(sizeof(struct callback_message));
    if (! skb)
    {
        pr_err("\n %s skb allocation failed ",__FUNCTION__);
    }else
    {
        struct callback_message *msg = (struct callback_message *)skb_put(skb, sizeof(struct callback_message));

        msg->event = BSAGE_Event_msgResponse;
        msg->callback = (BSAGE_Callback) p->rpc.response_isr;
        msg->context  = (uint32_t)p->rpc.responseContext;
        msg->context0 = (uint32_t)p->async_argument;
        msg->context1 = response_rc;
        msg->context2 = 0;

        skb_queue_tail(&p->callbackQueue, skb);
        up(&p->callbackSem);
    }
}

static void SAGE_IndicationRecvCallback(
    struct platform *p,
    void *async_argument,
    uint32_t indication_id,
     uint32_t value)
{
    struct sk_buff *skb;

    if(p == NULL)
    {
        BDBG_ERR(("%s NULL platform(Remote) handle!",__FUNCTION__));
        return;
    }

    skb = dev_alloc_skb(sizeof(struct callback_message));
    if (! skb)
    {
        pr_err("\n %s skb allocation failed ",__FUNCTION__);
    }else
    {
        struct callback_message *msg = (struct callback_message *)skb_put(skb, sizeof(struct callback_message));

        msg->event = BSAGE_Event_msgIndication;
        msg->callback = (BSAGE_Callback) p->rpc.indicationRecv_isr;
        msg->context  = (uint32_t)p->rpc.indicationContext;
        msg->context0 = (uint32_t)p->async_argument;
        msg->context1 = indication_id;
        msg->context2 = value;

        skb_queue_tail(&p->callbackQueue, skb);
        up(&p->callbackSem);
    }
}

static BERR_Code SAGE_RequestCallback(
    struct platform *p,
    void *async_argument)
{
    BERR_Code rc = BERR_SUCCESS;
    struct sk_buff *skb;

    if(p == NULL)
    {
        BDBG_ERR(("%s NULL platform(Remote) handle!",__FUNCTION__));
        return BERR_INVALID_PARAMETER;
    }

    skb = dev_alloc_skb(sizeof(struct callback_message));
    if (! skb)
    {
        pr_err("\n %s skb allocation failed ",__FUNCTION__);
    }else
    {
        struct callback_message *msg = (struct callback_message *)skb_put(skb, sizeof(struct callback_message));

        msg->event = BSAGE_Event_msgCallbackRequest;
        msg->callback = (BSAGE_Callback) p->rpc.callbackRequest_isr;
        msg->context  = (uint32_t)p->rpc.callbackRequestContext;
        msg->context0 = (uint32_t)p->async_argument;
        msg->context1 = 0;
        msg->context2 = 0;

        skb_queue_tail(&p->callbackQueue, skb);
        up(&p->callbackSem);
    }

    return rc;
}

static void SAGE_TATerminateCallback(
    struct platform *p,
    void *async_argument,
    uint32_t reason,
    uint32_t source)
{
    struct sk_buff *skb;

    if(p == NULL)
    {
        BDBG_ERR(("%s NULL platform(Remote) handle!",__FUNCTION__));
        return;
    }

    skb = dev_alloc_skb(sizeof(struct callback_message));
    if (! skb)
    {
        pr_err("\n %s skb allocation failed ",__FUNCTION__);
    }else
    {
        struct callback_message *msg = (struct callback_message *)skb_put(skb, sizeof(struct callback_message));

        msg->event = BSAGE_Event_msgTATermination;
        msg->callback = (BSAGE_Callback) p->rpc.taTerminate_isr;
        msg->context  = (uint32_t)p->rpc.taTerminationContext;
        msg->context0 = (uint32_t)p->async_argument;
        msg->context1 = reason;
        msg->context2 = source;

        skb_queue_tail(&p->callbackQueue, skb);
        up(&p->callbackSem);
    }
}

static void SAGE_WatchdogCallback(
    struct platform *p)
{
    struct sk_buff *skb;

    if(p == NULL)
    {
        BDBG_ERR(("%s NULL platform(Remote) handle!",__FUNCTION__));
        return;
    }

    skb = dev_alloc_skb(sizeof(struct callback_message));
    if (! skb)
    {
        pr_err("\n %s skb allocation failed ",__FUNCTION__);
    }else
    {
        struct callback_message *msg = (struct callback_message *)skb_put(skb, sizeof(struct callback_message));

        msg->event = BSAGE_Event_watchdog;
        msg->callback = (BSAGE_Callback) p->watchdog.watchdog_isr;
        msg->context  = 0;
        msg->context0 = 0;
        msg->context1 = 0;
        msg->context2 = 0;

        skb_queue_tail(&p->callbackQueue, skb);
        up(&p->callbackSem);
    }
}

BUILD_RW_FOPS(bootRegionInfo);
BUILD_RW_FOPS(bootSettings);
BUILD_RO_FOPS(bootInfo);
BUILD_RO_FOPS(bootStatus);
BUILD_RO_FOPS(bmemInfo);
BUILD_RW_FOPS(create_platform);
BUILD_RW_FOPS(remove_platform);
BUILD_IOCTL_FOPS(platform);
BUILD_RW_FOPS(create_watchdog);
BUILD_RW_FOPS(remove_watchdog);
BUILD_RW_FOPS(toreset);

static struct proc_dir_entry *platforms_dir,*management_dir;

static void *SAGE_AddWatchdog(
    void *callback,
    void *context)
{
    struct proc_dir_entry *watchdogFile;
    struct platform *p;

    BDBG_ENTER((SAGE_AddWatchdog));

    if( callback == NULL)
    {
        printk("%s callback NULL\n",__FUNCTION__);
        goto err;
    }

    p = kzalloc(sizeof(struct platform), GFP_KERNEL);
    if( p == NULL)
    {
        printk("%s kzalloc watchdog context %d byte failed\n",__FUNCTION__,sizeof(struct platform));
        goto err;
    }

    sprintf(p->name,"%p",callback);

    p->watchdog.watchdog_isr = callback;

    sema_init(&p->callbackSem,0);
    skb_queue_head_init(&p->callbackQueue);
//    p->calbackTask = kthread_run(SAGE_CallbackTask,(void *) p,p->name);

    watchdogFile = proc_create_data(p->name,0664, management_dir,&ioctl_fops_platform,p);
    if (watchdogFile == NULL)
    {
        printk(" %s %d create watchdog %s dir failed \n",__FUNCTION__,__LINE__,p->name);
        goto err_proc;
    }

    p->platformFile = watchdogFile;
    p->parentDir    = management_dir;

    BDBG_LEAVE((SAGE_AddWatchdog));
    return p;

err_proc:
    kfree(p);
err:
    BDBG_LEAVE((SAGE_AddWatchdog));
    return NULL;
}

static BERR_Code SAGE_SendCommand(
    BSAGE_RpcRemoteHandle remote,
    BSAGElib_RpcCommand *command,
    uint32_t *pAsync_id)
{
    BERR_Code rc;
    struct platform *p = (struct platform *) remote;

    BDBG_ENTER((SAGE_SendCommand));

    if(p == NULL)
    {
        BDBG_ERR(("%s NULL platform(Remote) handle!",__FUNCTION__));
        return BERR_INVALID_PARAMETER;
    }

    if(p->remote)
        rc = BSAGE_Rpc_SendCommand(p->remote,command,pAsync_id);
    else{
        BDBG_ERR(("%s NULL remote handle at platform %p!",__FUNCTION__,p));
        return BERR_UNKNOWN;
    }

    BDBG_LEAVE((SAGE_SendCommand));

    return rc;
}

static BERR_Code SAGE_SendCallbackResponse(
    BSAGE_RpcRemoteHandle remote,
    uint32_t sequenceId,
    BERR_Code retCode)
{
    BERR_Code rc;
    struct platform *p = (struct platform *) remote;

    BDBG_ENTER((SAGE_SendCallbackResponse));

    if(p == NULL)
    {
        BDBG_ERR(("%s NULL platform(Remote) handle!",__FUNCTION__));
        return BERR_INVALID_PARAMETER;
    }

    if(p->remote)
        rc = BSAGE_Rpc_SendCallbackResponse(p->remote,sequenceId,retCode);
    else{
        rc = BERR_UNKNOWN;
        BDBG_ERR(("%s NULL remote handle at platform %p!",__FUNCTION__,p));
        goto err;
    }

err:
    BDBG_LEAVE((SAGE_SendCallbackResponse));
    return rc;
}

static BERR_Code SAGE_RegisterCallback(
    SAGE_Event event,
    void *callback,
    void *context,
    BSAGE_RpcRemoteHandle remote)
{
    BERR_Code rc;
    struct platform *p = (struct platform *) remote;

    BDBG_ENTER((SAGE_RegisterCallback));

    if(p == NULL && event != BSAGE_Event_watchdog)
    {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s NULL platform(Remote) handle! event %d ",__FUNCTION__,event));
        goto err;
    }

    if(p == NULL)
        rc = BSAGE_RegisterCallback(event,callback,context,NULL);
    else if(p->remote)
        rc = BSAGE_RegisterCallback(event,callback,context,p->remote);
    else{
        rc = BERR_UNKNOWN;
        BDBG_ERR(("%s NULL remote handle with event %d at platform %p!\n",__FUNCTION__,event,p));
        goto err;
    }

err:
    BDBG_LEAVE((SAGE_RegisterCallback));
    return rc;
}

static BERR_Code SAGE_UnRegisterCallback(
    SAGE_Event event,
    BSAGE_RpcRemoteHandle remote)
{
    BERR_Code rc;
    struct platform *p = (struct platform *) remote;

    BDBG_ENTER((SAGE_UnRegisterCallback));

    if(p == NULL)
    {
        BDBG_ERR(("%s NULL platform(Remote) handle!",__FUNCTION__));
        return BERR_INVALID_PARAMETER;
    }

    if(p->remote)
        rc = BSAGE_UnRegisterCallback(event,p->remote);
    else{
        rc = BERR_UNKNOWN;
        BDBG_ERR(("%s NULL remote handle at platform %p!",__FUNCTION__,p));
        goto err;
    }

err:
    BDBG_LEAVE((SAGE_UnRegisterCallback));
    return rc;
}

static BSAGE_RpcRemoteHandle SAGE_AddRemote(
    uint32_t platformId,
    uint32_t moduleId,
    void *async_argument,
    uint64_t messageOffset)
{
    struct proc_dir_entry *platformFile;
    struct platform *p;
    BSAGE_RpcRemoteHandle remote;

    BDBG_ENTER((SAGE_AddRemote));

    p = kzalloc(sizeof(struct platform), GFP_KERNEL);
    if( p == NULL)
    {
        printk("%s kzalloc platform context %d byte failed\n",__FUNCTION__,sizeof(struct platform));
        goto err;
    }

    sprintf(p->name,"%3x-%1x-%llx",platformId,moduleId,messageOffset);

    p->platformId = platformId;
    p->moduleId = moduleId;
    p->async_argument = async_argument;
    p->messageOffset = messageOffset;
    sema_init(&p->callbackSem,0);
    skb_queue_head_init(&p->callbackQueue);
//    p->calbackTask = kthread_run(SAGE_CallbackTask,(void *) p,p->name);
    p->watchdog.watchdog_isr = NULL;

    platformFile = proc_create_data(p->name,0664, platforms_dir,&ioctl_fops_platform,p);
    if (platformFile == NULL)
    {
        printk(" %s %d create platform %s dir failed \n",__FUNCTION__,__LINE__,p->name);
        goto err_proc;
    }

    p->platformFile = platformFile;
    p->parentDir    = platforms_dir;

    remote = BSAGE_Rpc_AddRemote(platformId,moduleId,async_argument,messageOffset);
    if (remote == NULL)
    {
        printk(" %s %d create platform %s remote failed \n",__FUNCTION__,__LINE__,p->name);
        goto err_remote;
    }

    p->remote = remote;

    BDBG_LEAVE((SAGE_UnRegisterCallback));
    return (BSAGE_RpcRemoteHandle) p;

err_remote:
    proc_remove(platformFile);
err_proc:
    kfree(p);
err:
    BDBG_LEAVE((SAGE_AddRemote));
    return NULL;
}

static void SAGE_RemoveRemote(
    BSAGE_RpcRemoteHandle remote)
{
    struct platform *p = (struct platform *) remote;
    struct sk_buff *skb;

    BDBG_ENTER((SAGE_RemoveRemote));

    if(p == NULL)
    {
        BDBG_ERR(("%s NULL platform(Remote) handle!",__FUNCTION__));
        return;
    }

    if(p->watchdog.watchdog_isr)
        BDBG_MSG(("remove watchdog %p",p->watchdog.watchdog_isr));
    else
        BDBG_MSG(("remove %3x-%1x-%llx",p->platformId,p->moduleId,p->messageOffset));

    if(p->platformFile)
    {
        int len1;
        len1 = strlen(p->name);
        p->name[len1] = 0xa0;
        remove_proc_subtree(p->name,p->parentDir);
    }
    else{
        BDBG_ERR(("%s NULL platformFile at platform %p!",__FUNCTION__,p));
    }

    if(p->remote)
        BSAGE_Rpc_RemoveRemote(p->remote);
    else if(p->watchdog.watchdog_isr == NULL){
        BDBG_ERR(("%s NULL remote handle and NULL watchdog at platform %p!",__FUNCTION__,p));
    }

    p->remote = NULL;
    p->watchdog.watchdog_isr = NULL;
//    kthread_stop(p->calbackTask);

    while(!skb_queue_empty(&p->callbackQueue))
    {
        skb = skb_dequeue(&p->callbackQueue);
        if(skb != NULL)
            kfree_skb(skb);
        else
            break;
    }

    kfree(p);
    BDBG_LEAVE((SAGE_RemoveRemote));
}

static BERR_Code SAGE_RegisterCallback1(
    SAGE_Event event,
    void *callback,
    void *context,
    BSAGE_RpcRemoteHandle remote)
{
    BERR_Code rc;
    struct platform *p = (struct platform *) remote;

    if(event == BSAGE_Event_watchdog)
    {
        p = SAGE_AddWatchdog(callback,context);
    }

    if(p == NULL)
    {
        BDBG_ERR(("%s NULL platform(Remote) handle!",__FUNCTION__));
        return BERR_INVALID_PARAMETER;
    }

    switch(event)
    {
    case BSAGE_Event_msgIndication:
        p->rpc.indicationRecv_isr = (BSAGElib_Rpc_IndicationRecvCallback)callback;
        p->rpc.indicationContext = context;
        callback = (void *) SAGE_IndicationRecvCallback;
        context = (void *)p;
        break;
    case BSAGE_Event_msgResponse:
        p->rpc.response_isr = (BSAGE_Rpc_DispatchResponse_isr) callback;
        p->rpc.responseContext = context;
        callback = (void *) SAGE_ResponseCallback;
        context = (void *)p;
        break;
    case BSAGE_Event_msgCallbackRequest:
        p->rpc.callbackRequest_isr = (BSAGElib_Rpc_CallbackRequestISRCallback) callback;
        p->rpc.callbackRequestContext = context;
        callback = (void *) SAGE_RequestCallback;
        context = (void *)p;
        break;
    case BSAGE_Event_msgTATermination:
        p->rpc.taTerminate_isr = (BSAGElib_Rpc_TATerminateCallback) callback;
        p->rpc.taTerminationContext = context;
        callback = (void *) SAGE_TATerminateCallback;
        context = (void *)p;
        break;
    case BSAGE_Event_watchdog:
        p->watchdog.watchdog_isr = (BSAGElib_Management_WatchdogCallback ) callback;
        callback = (void *) SAGE_WatchdogCallback;
        context = (void *)p;
        break;
    default:
        break;
    }

    rc = SAGE_RegisterCallback(event,callback,context,(BSAGE_RpcRemoteHandle)p);
    if(rc)
    {
        BDBG_ERR(("%s SAGE_RegisterCallback failed %p!",__FUNCTION__,rc));
        goto err;
    }

err:
    return rc;
}

static BERR_Code SAGE_UnRegisterCallback1(
    SAGE_Event event,
    BSAGE_RpcRemoteHandle remote)
{
    BERR_Code rc;
    struct platform *p = (struct platform *) remote;

    if(p == NULL)
    {
        BDBG_ERR(("%s NULL platform(Remote) handle!",__FUNCTION__));
        return BERR_INVALID_PARAMETER;
    }

    rc = SAGE_UnRegisterCallback(event,(BSAGE_RpcRemoteHandle)p);

    if(event == BSAGE_Event_watchdog)
    {
        SAGE_RemoveRemote((BSAGE_RpcRemoteHandle)p);
    }

    return rc;
}

EXPORT_SYMBOL(SAGE_SendCallbackResponse);
EXPORT_SYMBOL(SAGE_SendCommand);
EXPORT_SYMBOL(SAGE_UnRegisterCallback);
EXPORT_SYMBOL(SAGE_RegisterCallback);
EXPORT_SYMBOL(SAGE_RemoveRemote);
EXPORT_SYMBOL(SAGE_AddRemote);

static int Sage_create_management_files(struct proc_dir_entry *sage_dir)
{
    struct proc_dir_entry *create_watchdog,*remove_watchdog,*toReset;

    int rv=0;

    create_watchdog = proc_create_data("create_watchdog",0644, sage_dir,&rw_fops_create_watchdog,NULL);
    if (create_watchdog == NULL)
    {
        printk(" %s %d create 'create_watchdog' file failed \n",__FUNCTION__,__LINE__);
        rv  = -ENOMEM;
        goto error;
    }
    remove_watchdog = proc_create_data("remove_watchdog",0644, sage_dir,&rw_fops_remove_watchdog,NULL);
    if (remove_watchdog == NULL)
    {
        printk(" %s %d create 'remove_watchdog' file failed \n",__FUNCTION__,__LINE__);
        rv  = -ENOMEM;
        goto error;
    }
    toReset = proc_create_data("toReset",0644, sage_dir,&rw_fops_toreset,NULL);
    if (toReset == NULL)
    {
        printk(" %s %d create 'toReset' file failed \n",__FUNCTION__,__LINE__);
        rv  = -ENOMEM;
        goto error;
    }
    return 0;
error:
    return rv;
}

static int Sage_create_platform_files(struct proc_dir_entry *sage_dir)
{
    struct proc_dir_entry *create_platform,*remove_platform;

    int rv=0;

    create_platform = proc_create_data("create_platform",0644, sage_dir,&rw_fops_create_platform,NULL);
    if (create_platform == NULL)
    {
        printk(" %s %d create 'create_platform' file failed \n",__FUNCTION__,__LINE__);
        rv  = -ENOMEM;
        goto error;
    }
    remove_platform = proc_create_data("remove_platform",0644, sage_dir,&rw_fops_remove_platform,NULL);
    if (remove_platform == NULL)
    {
        printk(" %s %d create 'remove_platform' file failed \n",__FUNCTION__,__LINE__);
        rv  = -ENOMEM;
        goto error;
    }
    return 0;
error:
    return rv;
}

static int Sage_create_boot_files(struct proc_dir_entry *sage_dir)
{
    struct proc_dir_entry *bootSettings_file, *bootInfo_file, *bootStatus_file, \
                          *bootRegionInfo_file, *toreset_file,*bmemInfo_file;


    int rv=0;

    bootRegionInfo_file = proc_create_data("bootRegionInfo",0644, sage_dir,&rw_fops_bootRegionInfo,NULL);
    if (bootRegionInfo_file == NULL)
    {
        printk(" %s %d create bootRegionInfo_file failed \n",__FUNCTION__,__LINE__);
        rv  = -ENOMEM;
        goto error;
    }

    toreset_file = proc_create_data("toreset",0644, sage_dir,&rw_fops_toreset,NULL);
    if (toreset_file == NULL)
    {
        printk(" %s %d create sage/toreset failed \n",__FUNCTION__,__LINE__);
        rv  = -ENOMEM;
        goto error;
    }

    bootSettings_file = proc_create_data("BootSettings",0664, sage_dir,&rw_fops_bootSettings,NULL);
    if (bootSettings_file == NULL)
    {
        printk(" %s %d create sage/bootSettings failed \n",__FUNCTION__,__LINE__);
        rv  = -ENOMEM;
        goto error;
    }

    bootInfo_file = proc_create_data("showBootInfo",0664, sage_dir,&read_fops_bootInfo,NULL);
    if (bootInfo_file == NULL)
    {
        printk(" %s %d create sage/showBootInfo failed \n",__FUNCTION__,__LINE__);
        rv  = -ENOMEM;
        goto error;
    }

    bootStatus_file = proc_create_data("showBootStatus",0664, sage_dir,&read_fops_bootStatus,NULL);
    if (bootStatus_file == NULL)
    {
        printk(" %s %d create sage/showBootStatus failed \n",__FUNCTION__,__LINE__);
        rv  = -ENOMEM;
        goto error;
    }

    bmemInfo_file = proc_create_data("showBmemInfo",0664, sage_dir,&read_fops_bmemInfo,NULL);
    if (bmemInfo_file == NULL)
    {
        printk(" %s %d create sage/showBmemInfo failed \n",__FUNCTION__,__LINE__);
        rv  = -ENOMEM;
        goto error;
    }

    return 0;
error:
    return rv;

}

static struct proc_dir_entry *sage_dir;

static int __init __init_module(void)
{
    struct proc_dir_entry *boot_dir;
    int rv=0;

    sage_dir = proc_mkdir(MODULE_NAME, NULL);
    if (sage_dir == NULL)
    {
        printk(" %s %d create proc/sage failed \n",__FUNCTION__,__LINE__);
        rv = -ENOMEM;
        goto error;
    }

    boot_dir = proc_mkdir("boot", sage_dir);
    if (boot_dir == NULL)
    {
        printk(" %s %d create boot dir failed \n",__FUNCTION__,__LINE__);
        rv = -ENOMEM;
        goto no_sage_dir;
    }

    platforms_dir = proc_mkdir("platforms", sage_dir);
    if (platforms_dir == NULL)
    {
        printk(" %s %d create platforms dir failed \n",__FUNCTION__,__LINE__);
        rv = -ENOMEM;
        goto no_sage_dir;
    }

    management_dir = proc_mkdir("management", sage_dir);
    if (management_dir == NULL)
    {
        printk(" %s %d create management dir failed \n",__FUNCTION__,__LINE__);
        rv = -ENOMEM;
        goto no_sage_dir;
    }

    rv = Sage_create_boot_files(boot_dir);
    if (rv != BERR_SUCCESS)
    {
        printk(" %s %d create boot files failed \n",__FUNCTION__,__LINE__);
        goto no_sage_dir;
    }

    rv = Sage_create_platform_files(platforms_dir);
    if (rv != BERR_SUCCESS)
    {
        printk(" %s %d create platforms files failed \n",__FUNCTION__,__LINE__);
        goto no_sage_dir;
    }

    rv = Sage_create_management_files(management_dir);
    if (rv != BERR_SUCCESS)
    {
        printk(" %s %d create management files failed \n",__FUNCTION__,__LINE__);
        goto no_sage_dir;
    }

    rv = SAGE_Base_Init();
    if (rv != BERR_SUCCESS)
    {
        printk(" %s %d magnum init  rc %d failed \n",__FUNCTION__,__LINE__,rv);
        goto no_sage_dir;
    }

    PINFO("Initialization complete\n");

    return 0;
no_sage_dir:
    remove_proc_subtree(MODULE_NAME, NULL);
error:
    return rv;
}

module_init(__init_module);

static void __exit __cleanup_module(void)
{
    PINFO("Cleanup_modules...\n");

    remove_proc_entry("100-0-28eb2b80",platforms_dir);
    remove_proc_subtree("platforms", sage_dir);
    remove_proc_subtree(MODULE_NAME, NULL);

    SAGE_Base_Uninit();

    PINFO("Cleanup complete\n");
}

module_exit(__cleanup_module);

MODULE_LICENSE("Proprietary");
/* End of file */
