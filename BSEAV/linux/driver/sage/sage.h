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
#ifndef SAGEDRIVER_H
#define SAGEDRIVER_H
#include <linux/sched.h>
#include <linux/skbuff.h>
#include "bdbg.h"
#include "bkni.h"
#include "bstd.h"
#include "bkni.h"
#include "bchp.h"
#include "bmem.h"
#include "breg_mem.h"
#include "bint.h"
#include "bint_plat.h"
#include "bhsm.h"
#include "bhsi.h"
#include "bhsm_keyslots.h"
#include "bchp_hif_cpu_intr1.h"
#include "bchp_scpu_host_intr2.h"
#include "bchp_cpu_ipi_intr2.h"
#include "bchp_scpu_top_ctrl.h"

#include "bsagelib.h"
#include "bsagelib_boot.h"
#include "bsagelib_rai.h"
#include "bsagelib_client.h"
#include "bsagelib_rpc.h"
#include "priv/bsagelib_rpc_shared.h"
#include "bsagelib_management.h"

#include "bsage.h"
#include "bsage_management.h"

#include "bhsm_otpmsp.h"
#include "bhsm_keyladder_enc.h"

/* update this number for any non-trivial changes in the file */
#define SAGEDRIVER_VERSION  1

#define MAX_IRQ_CNT         2
#define MAX_IRQ_NUM         64

#define REGION_MAP_MAX_NUM 8

typedef struct IrqCtx
{
    wait_queue_head_t   isrEvent;
    int                 linuxIrqNum;
    struct task_struct  *isrTask;

    uint32_t irqreceived,irqProcessed;

} IrqCtx;

typedef struct SageBase
{
    BREG_Handle         hReg;
    BTMR_Handle         hTmr;
    BMEM_ModuleHandle   hMemModule;
    BINT_Handle         hInt;
    BHSI_Handle         hHsi;
    BSAGE_Handle        hSAGE;
    BKNI_MutexHandle    sageLock;
    void                *pRegAddress;

    IrqCtx              isrHsm;
    BHSM_Handle         hHsm;
    BMEM_Heap_Handle    hMemHsm;
    struct page         *pageHsm;

    BMEM_Heap_Handle    hMemGlr;
    void                *addrGLR;
    BMEM_Heap_Handle    hMemGlr2;
    void                *addrGLR2;

    IrqCtx              isrRpc;
    uint32_t            HSIBufferOffset;

    BSAGE_BootSettings bootSettings;
    BSAGElib_RegionInfo regionMap[REGION_MAP_MAX_NUM];

    BSAGElib_ChipInfo chipInfo;

    BSAGElib_ImageInfo bootloaderInfo;
    BSAGElib_ImageInfo frameworkInfo;

} SageBase;

struct platform
{
    struct proc_dir_entry *platformFile,*parentDir;
    char                   name[50];
    BSAGE_RpcRemoteHandle  remote;
    BSAGE_RpcInterface     rpc;
    uint32_t               platformId;
    uint32_t               moduleId;
    void                  *async_argument;
    uint64_t               messageOffset;

    BSAGElib_ManagementInterface watchdog;

    struct sk_buff_head   callbackQueue;
    struct semaphore      callbackSem;
    struct task_struct    *calbackTask;
};

struct callback_message
{
    SAGE_Event    event;
    BSAGE_Callback callback;
    uint32_t      context;
    uint32_t      context0;
    uint32_t      context1;
    uint32_t      context2;
};

typedef enum {
    SAGE_IOCTL_WAIT_FOR_CALLBACK = 100,
    SAGE_IOCTL_REGISTER_msgIndication_CALLBACK,
    SAGE_IOCTL_REGISTER_msgResponse_CALLBACK,
    SAGE_IOCTL_REGISTER_msgCallbackRequest_CALLBACK,
    SAGE_IOCTL_REGISTER_msgTATermination_CALLBACK,
    SAGE_IOCTL_REGISTER_watchdog_CALLBACK,
    SAGE_IOCTL_REGISTER_msgIndication_CONTEXT,
    SAGE_IOCTL_REGISTER_msgResponse_CONTEXT,
    SAGE_IOCTL_REGISTER_msgCallbackRequest_CONTEXT,
    SAGE_IOCTL_REGISTER_msgTATermination_CONTEXT,
} sage_ioctl_commands;

/* Nested macros to build BCHP_OPEN and BINT_GETSETTINGS using BCHP_CHIP */
#define BCHP_OPEN_PROTOTYPE_P(CHIP) BCHP_Open##CHIP
#define BCHP_OPEN_PROTOTYPE(CHIP) BCHP_OPEN_PROTOTYPE_P(CHIP)
#define BINT_GETSETTINGS_PROTOTYPE_P(CHIP) BINT_##CHIP##_GetSettings
#define BINT_GETSETTINGS_PROTOTYPE(CHIP) BINT_GETSETTINGS_PROTOTYPE_P(CHIP)

/* Macros required by magnum */
#define BCHP_OPEN(pChip, reg) BCHP_OPEN_PROTOTYPE(BCHP_CHIP)(pChip, reg)
#define BINT_GETSETTINGS(void) BINT_GETSETTINGS_PROTOTYPE(BCHP_CHIP)(void)

extern BERR_Code BCHP_OPEN(BCHP_Handle *phChip,BREG_Handle hRegister);
extern const const BINT_Settings *BINT_GETSETTINGS(void);

#define BLOCK_SIZE_2M 9
#define SIZE_2M 2*1024*1024

#endif /* SAGEDRIVER_H */
