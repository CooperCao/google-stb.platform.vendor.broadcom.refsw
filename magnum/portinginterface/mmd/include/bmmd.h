/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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

#ifndef BMMD_H__
#define BMMD_H__

#include "bstd.h"
#include "bkni.h"
#include "bchp.h"
#include "breg_mem.h"
#include "bmem.h"
#include "bint.h"
#include "berr_ids.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
Module Overview:
    MMD is a Magnum portinginterface for mem-to-mem DMA transfers.
    It is intended to replace the mem-to-mem functionality of BDMA.
    
    In MMD, DMA transfers are performed using one or more MMD "contexts"
    (BMMD_ContextHandle). A MMD context represents a single DMA job that you
    want to perform: a chain of DMA descriptors and its associated completion 
    callback. 
   
    After a context has been created (BMMD_Context_Create()), it is "enqueued" 
    (BMMD_Context_Enqueue()), which queues up the context in the DMA HW. 

    A single MMD context can only be used for a single DMA job at a time.
    That is, after a context has been enqueued, it cannot be enqueued again 
    before it has finished. To perform multiple DMA jobs in succession, you 
    create multiple contexts and enqueue them in the desired order. 
    As each context completes, the completion callback associated with that 
    context will fire.
         
    A MMD context can be enqueued while another context is currently active, 
    i.e. while the HW is currently busy. The new context's chain of descriptors
    is appended to the last-enqueued descriptor in HW. The completion callback
    of previously-enqueued contexts will still fire at their respective 
    completions.    
*******************************************************************************/

/*******************************************************************************
Summary:
    MMD return codes
*******************************************************************************/
#define BMMD_QUEUED   BERR_MAKE_CODE(BERR_MMD_ID, 0)


/*******************************************************************************
Summary:
    MMD engine (HW) types
*******************************************************************************/
typedef enum BMMD_EngineType
{
    BMMD_EngineType_eDma = 0, /* MEM_DMA HW */
    BMMD_EngineType_eSharf,   /* SHARF_DMA HW */
    BMMD_EngineType_eMax
} BMMD_EngineType;


/*******************************************************************************
Summary:
    MMD settings
*******************************************************************************/
typedef struct BMMD_Settings 
{
    BMMD_EngineType engineType; /* DMA or Sharf HW */
    int engineIndex;            /* some platforms have more than one DMA or Sharf HW block. 
                                   index 0 opens the first HW block */
} BMMD_Settings;


/*******************************************************************************
Summary:
    Get default MMD settings
*******************************************************************************/
void BMMD_GetDefaultSettings(
    BMMD_Settings *pSettings /* [out] */
    );

    
/*******************************************************************************
Summary:
    Opaque MMD handle
*******************************************************************************/
typedef struct BMMD_Handle_Tag *BMMD_Handle;


/*******************************************************************************
Summary:
    Create a MMD handle with MMD settings
*******************************************************************************/
BERR_Code BMMD_Open(
    BCHP_Handle hChp,               /* chip handle */
    BREG_Handle hReg,               /* register handle */
    BMEM_Handle hMem,               /* memory handle */
    BINT_Handle hInt,               /* interrupt handle */
    const BMMD_Settings *pSettings, /* settings */
    BMMD_Handle *phMmd              /* [out] MMD handle */
    );

    
/*******************************************************************************
Summary:
    Endian format of source data to be read from memory 
*******************************************************************************/
typedef enum BMMD_EndianMode
{
    BMMD_EndianMode_eLittle,
    BMMD_EndianMode_eBig,
    BMMD_EndianMode_eMax
} BMMD_EndianMode;

    
/*******************************************************************************
Summary:
    Endian translation method of data to be written to memory
*******************************************************************************/
typedef enum BMMD_SwapMode
{
    BMMD_SwapMode_eNone,
    BMMD_SwapMode_eWord, /* swap 16-bit words (0x00112233 -> 0x22330011) */
    BMMD_SwapMode_eByte, /* swap  8-bit bytes (0x00112233 -> 0x33221100) */
    BMMD_SwapMode_eMax
} BMMD_SwapMode;


/*******************************************************************************
Summary:
    Data scrambling mode

Description:
    This enum is only used with MEM_DMA HW (BMMD_EngineType_eDma)
*******************************************************************************/
typedef enum BMMD_ScramMode
{
    BMMD_ScramMode_eNone = 0, /* no scrambling */
    BMMD_ScramMode_eBlock,    /* scramble all data as a generic block */
    BMMD_ScramMode_eMpeg,     /* only scramble the payload of 188-byte MPEG2-TS packets. 
                                 192-byte MPEG2-TS packets are not supported */
    BMMD_ScramMode_eDss,      /* only scramble the payload of 130-byte DSS packets */
    BMMD_ScramMode_eMax
} BMMD_ScramMode;


/*******************************************************************************
Summary:
    Sharf scrambling mode

Description:
    This enum is only used with SHARF_DMA HW (BMMD_EngineType_eSharf)
*******************************************************************************/
typedef enum BMMD_SharfMode
{
    BMMD_SharfMode_ePassThrough = 0,
    BMMD_SharfMode_eSha1,
    BMMD_SharfMode_eAes128CbcDecrypt,
    BMMD_SharfMode_eAes128CbcEncrypt,
    BMMD_SharfMode_eCmac,
    BMMD_SharfMode_eAes128EcbDecrypt,
    BMMD_SharfMode_eAes128EcbEncrypt,
    BMMD_SharfMode_eMax
} BMMD_SharfMode;


/*******************************************************************************
Summary:
    Context settings
    
See Also:
    BMMD_ContextHandle    
    BMMD_Context_Enqueue()
*******************************************************************************/
typedef struct BMMD_ContextSettings
{
    unsigned maxNumBlocks;      /* maximum number of blocks in this context. 
                                   this parameter is used to allocate memory from the system heap */

    BMMD_EndianMode endianMode; /* data endianness */
    BMMD_SwapMode swapMode;     /* endian translation method */
    BMMD_ScramMode scramMode;   /* scrambling mode */
    unsigned keyslot;           /* keyslot number. this is ignored if BMMD_ScramMode_eNone or 
                                   BMMD_EngineType_eSharf */

    struct {
        uint32_t offset; /* bounded memory offset (0=no bounds check) */
        unsigned size;   /* bounded memory size  (0=no bounds check) */
    } memoryBounds; /* used to verify that transfers do not violate memory bounds */

    /* Sharf-specific settings */
    struct {
        bool useBspKey; /* if true, SHARF will use a secure key supplied by BSP directly 
                           for data encryption/decryption. if false, SHARF will use the key
                           that prepends the descriptor data */
        BMMD_SharfMode mode;
        unsigned shaContext; /* SHARF HW can hold the intermediate or final SHA-1 digest 
                                for up to 3 contexts across all SHARF DMA channels. This 
                                allows interleaving SHARF DMA operations. 
                                Valid values are 0,1 or 2. */
    } sharf;
    
    /* callback that fires in ISR-context when this MMD context has completed. can be NULL */
    void (*callback_isr)(void *pParm1, int parm2);
    void *pParm1;
    int pParm2;
} BMMD_ContextSettings;


/*******************************************************************************
Summary:
    Get default context settings
*******************************************************************************/
void BMMD_Context_GetDefaultSettings(
    BMMD_ContextSettings *pSettings /* [out] */
    );
      
    
/*******************************************************************************
Summary:
    Context block settings. A context block is a single DMA descriptor.
*******************************************************************************/
typedef struct BMMD_ContextBlockSettings
{
    uint32_t src;  /* source physical address */
    uint32_t dst;  /* destination physical address */
    unsigned size; /* block size in bytes */
    
    bool resetCrypto;  /* if true, discontinue crypto from previous block and re-initialize.
                          this is ignored if BMMD_EngineType_eSharf */
    bool sgScramStart; /* if true, then this block indicates the start of scatter-gather scrambling operation */
    bool sgScramEnd;   /* if true, then this block indicates the end of scatter-gather scrambling operation */

    /* Sharf-specific settings */
    struct {
        bool keyPresent;    /* if true, a crypto key prepends the data */
        bool digestPresent; /* if true, a hash digest or MAC prepends the data */
    } sharf;

    const void* srcPtr; /* unused */
    void* dstPtr;       /* unused */
} BMMD_ContextBlockSettings;


/*******************************************************************************
Summary:
    Get default context block settings
*******************************************************************************/
void BMMD_Context_GetDefaultBlockSettings(
    BMMD_ContextBlockSettings *pSettings /* [out] */
    );


/*******************************************************************************
Summary:
    Opaque MMD context handle
    
Description:
    A MMD context represents a single DMA job to be performed: a chain of 
    DMA descriptors and its associated completion callback.
    
    A single MMD context can only be used for a single DMA job at a time.
    That is, after a context has been enqueued, it cannot be enqueued again 
    before it has finished. To perform multiple DMA jobs in succession, you 
    create multiple contexts and enqueue them in the desired order. 
    
    There is no MMD-imposed limit to how many contexts can be created.
*******************************************************************************/
typedef struct BMMD_Context *BMMD_ContextHandle;
    
        
/*******************************************************************************
Summary:
    Create a MMD context handle
*******************************************************************************/
BMMD_ContextHandle BMMD_Context_Create(
    BMMD_Handle hMmd,
    const BMMD_ContextSettings *pSettings
    );


/*******************************************************************************
Summary:
    Get current context settings
*******************************************************************************/
BERR_Code BMMD_Context_GetSettings(
    BMMD_ContextHandle hCtx,
    BMMD_ContextSettings *pSettings /* [out] */
    );

    
/*******************************************************************************
Summary:
    Set new context settings

Description:
    Context settings can only be changed when the context is idle. Otherwise,
    this function will fail.
    Changing BMMD_ContextSettings.maxNumBlocks is not allowed. To change 
    maxNumBlocks, a new context must be created.
*******************************************************************************/
BERR_Code BMMD_Context_SetSettings(
    BMMD_ContextHandle hCtx,
    const BMMD_ContextSettings *pSettings
    );

    
/*******************************************************************************
Summary:
    Enqueue a context with an array of context block settings

Description:
    If no other contexts are currently active, then the context starts 
    immediately.
    Otherwise, the context is processed after all previously-queued contexts 
    have completed.
    
    The context has to be idle in order to be enqueued.

Returns:
    BERR_SUCCESS       if context was completed
    BMMD_QUEUED        if context is in progress or is queued 
    BERR_NOT_SUPPORTED if context was not idle
*******************************************************************************/
BERR_Code BMMD_Context_Enqueue(
    BMMD_ContextHandle hCtx,
    const BMMD_ContextBlockSettings *pSettings, /* array of BMMD_ContextBlockSettings */
    unsigned numBlocks                          /* number of BMMD_ContextBlockSettings elements. 
                                                   must be <= BMMD_ContextSettings.maxNumBlocks */
    );


/*******************************************************************************
Summary:
    Context state
*******************************************************************************/
typedef enum BMMD_ContextState
{
    BMMD_ContextState_eIdle,       /* idle or complete */
    BMMD_ContextState_eInProgress  /* queued in HW. the HW may not yet have started 
                                      the context if there are other active contexts */
} BMMD_ContextState;

    
/*******************************************************************************
Summary:
    Context status
*******************************************************************************/
typedef struct BMMD_ContextStatus
{
    BMMD_ContextState state;
} BMMD_ContextStatus;


/*******************************************************************************
Summary:
    Get current status of a MMD context
*******************************************************************************/
BERR_Code BMMD_Context_GetStatus(
    BMMD_ContextHandle hCtx,
    BMMD_ContextStatus *pStatus /* [out] */
    );


/*******************************************************************************
Summary:
    Destroy a MMD context
    
Description:
    If there are no currently active MMD contexts, then the memory allocated 
    for DMA descriptors is freed immediately.
    
    If there are currently active MMD contexts, then this function does not wait 
    for them to finish. The completion callback will not fire. The contexts 
    may or may not finish; this behavior is undefined.   
    The memory allocated for DMA descriptors may not be freed immediately.
*******************************************************************************/
void BMMD_Context_Destroy(
    BMMD_ContextHandle hCtx
    );    


/*******************************************************************************
Summary:
    Close MMD handle
    
Description:
    This function immediately stops the HW, destroys any undestroyed contexts,
    and frees all resources.
*******************************************************************************/
void BMMD_Close(
    BMMD_Handle hMmd
    );

/*******************************************************************************
Summary:
    Standby settings. Currently unused.
*******************************************************************************/
typedef struct BMMD_StandbySettings {
    unsigned unused; /* dummy placeholder */
} BMMD_StandbySettings;

/*******************************************************************************
Summary:
    Enter standby

Description:
    MMD requires that there are no unfinished contexts before entering standby.
    MMD does not require that opened contexts are destroyed before entering standby.
*******************************************************************************/
BERR_Code BMMD_Standby(
    BMMD_Handle hMmd,
    const BMMD_StandbySettings *pSettings /* optional */
    );

/*******************************************************************************
Summary:
    Exit standby

Description: 
    Upon exitting standby, MMD programs the HW back to a state similar to 
    BMMD_Open, while retaining any opened contexts.
*******************************************************************************/
void BMMD_Resume(
    BMMD_Handle hMmd
    );

#ifdef __cplusplus
}
#endif
 
#endif
