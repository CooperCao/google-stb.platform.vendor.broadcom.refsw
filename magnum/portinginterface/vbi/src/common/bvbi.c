/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/

#include "bstd.h"                /* standard types */
#include "bdbg.h"                /* Dbglib */
#include "bkni.h"                /* malloc */
#include "bvbi.h"                /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"           /* VBI internal data structures */

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

BDBG_MODULE(BVBI);
BDBG_OBJECT_ID(BVBI);

/***************************************************************************
* Private data items
***************************************************************************/

static const BVBI_Settings s_DefaultSettings =
{
    24576,                      /* in656bufferSize    */
    false                       /* tteShiftDirMsb2Lsb */
};

static const BVBI_XSER_Settings s_XSER_DefaultSettings =
{
    BVBI_TTserialDataContent_DataMagFrmRun,    /* xsSerialDataContent */
    BVBI_TTserialDataSync_EAV,                 /* ttSerialDataSync */
    27                                         /* iTTserialDataSyncDelay */
};
/* TODO: determine sensible defaults (above) from our customers. */

/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/

/***************************************************************************
 *
 */
BERR_Code BVBI_GetDefaultSettings(  BVBI_Settings *pSettings )
{
    BDBG_ENTER(BVBI_GetDefaultSettings);

    if (!pSettings)
    {
        BDBG_ERR(("Invalid parameter"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    *pSettings = s_DefaultSettings;

    BDBG_LEAVE(BVBI_GetDefaultSettings);

    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVBI_Open
    ( BVBI_Handle       *pVbiHandle,
      BCHP_Handle        hChip,
      BREG_Handle        hRegister,
      BMMA_Heap_Handle   hMmaHeap,
      const BVBI_Settings       *pDefSettings )
{
    int type;
    BERR_Code eErr;
    BVBI_P_Handle *pVbi = NULL;
    const BVBI_Settings* settings =
        (pDefSettings ? pDefSettings : &s_DefaultSettings);

    BDBG_ENTER(BVBI_Open);

    if((!pVbiHandle) ||
       (!hChip) ||
       (!hRegister) ||
       (!hMmaHeap))
    {
        BDBG_ERR(("Invalid parameter"));
        eErr = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto BVBI_Open_Done;
    }

    /* Alloc the main VBI context. */
    pVbi = (BVBI_P_Handle*)(BKNI_Malloc(sizeof(BVBI_P_Handle)));

    if(!pVbi)
    {
        eErr = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto BVBI_Open_Done;
    }
    BDBG_OBJECT_INIT (pVbi, BVBI);

    /* Store the hChip, hRegister, hMemory, and hRdc for later use. */
    pVbi->hChip     = hChip;
    pVbi->hReg      = hRegister;
    pVbi->hMmaHeap  = hMmaHeap;

    /* Store other settings from user */
    pVbi->in656bufferSize = settings->in656bufferSize;
    pVbi->tteShiftDirMsb2Lsb = settings->tteShiftDirMsb2Lsb;

    /* Initialize empty lists of decode and encode contexts */
    BLST_D_INIT(&pVbi->decode_contexts);
    BLST_D_INIT(&pVbi->encode_contexts);

    /* Initialize freelists for bulky data */
    BVBI_P_LCOP_INITFREELIST (&pVbi->ttFreelist);

    /* Clear out list of "in use" VEC cores */
    for (type = 0 ; type < BVBI_P_EncCoreType_eLAST ; ++type)
    {
        pVbi->vecHwCoreMask[type] = 0x0;
        pVbi->vecHwCoreMask_656[type] = 0x0;
    }

#ifdef BCHP_PWR_RESOURCE_VDC_VEC
    BCHP_PWR_AcquireResource(pVbi->hChip, BCHP_PWR_RESOURCE_VDC_VEC);
#endif

    /* Bang on the hardware until it makes a nice sound */
    if ((eErr = BERR_TRACE (BVBI_P_CC_Init (pVbi))) != BERR_SUCCESS)
    {
        goto BVBI_Open_Done;
    }
    if ((eErr = BERR_TRACE (BVBI_P_CGMS_Init (pVbi))) != BERR_SUCCESS)
    {
        goto BVBI_Open_Done;
    }
    if ((eErr = BERR_TRACE (BVBI_P_WSS_Init (pVbi))) != BERR_SUCCESS)
    {
        goto BVBI_Open_Done;
    }
    if ((eErr = BERR_TRACE (BVBI_P_VPS_Init (pVbi))) != BERR_SUCCESS)
    {
        goto BVBI_Open_Done;
    }
    if ((eErr = BERR_TRACE (BVBI_P_TT_Init (pVbi))) != BERR_SUCCESS)
    {
        goto BVBI_Open_Done;
    }
#if (BVBI_NUM_GSE > 0)
    if ((eErr = BERR_TRACE (BVBI_P_GS_Init (pVbi))) != BERR_SUCCESS)
    {
        goto BVBI_Open_Done;
    }
#endif
#if (BVBI_NUM_AMOLE > 0)
    if ((eErr = BERR_TRACE (BVBI_P_AMOL_Init (pVbi))) != BERR_SUCCESS)
    {
        goto BVBI_Open_Done;
    }
#endif
#if (BVBI_NUM_SCTEE > 0)
    if ((eErr = BERR_TRACE (BVBI_P_SCTE_Init (pVbi))) != BERR_SUCCESS)
    {
        goto BVBI_Open_Done;
    }
#endif
    if ((eErr = BERR_TRACE (BVBI_P_VE_Init (pVbi))) != BERR_SUCCESS)
    {
        goto BVBI_Open_Done;
    }
#if (BVBI_P_HAS_XSER_TT)
    if ((eErr = BERR_TRACE (
        BVBI_P_ITU656_Init (pVbi->hReg, BVBI_P_GetDefaultXserSettings())))
        != BERR_SUCCESS)
    {
        goto BVBI_Open_Done;
    }
#endif
#if (BVBI_NUM_ANCI656_656 > 0)
    if ((eErr = BERR_TRACE (BVBI_P_A656_Init (pVbi))) != BERR_SUCCESS)
    {
        goto BVBI_Open_Done;
    }
#endif
#if (BVBI_NUM_IN656 > 0)
    if ((eErr = BERR_TRACE (BVBI_P_IN656_Init (pVbi))) != BERR_SUCCESS)
    {
        goto BVBI_Open_Done;
    }
#endif

    /* All done. now return the new fresh context to user. */
    *pVbiHandle = (BVBI_Handle)pVbi;

  BVBI_Open_Done:

    BDBG_LEAVE(BVBI_Open);

    if ((BERR_SUCCESS != eErr) && (NULL != pVbi)) {
        /* release power if open fails */
#ifdef BCHP_PWR_RESOURCE_VDC_VEC
        BCHP_PWR_ReleaseResource(pVbi->hChip, BCHP_PWR_RESOURCE_VDC_VEC);
#endif
        BDBG_OBJECT_DESTROY (pVbi, BVBI);
        BKNI_Free((void*)pVbi);
    }

    return eErr;
}


/***************************************************************************
 *
 */
void  BVBI_Close ( BVBI_Handle vbiHandle )
{
    BVBI_P_Handle *pVbi;
    BERR_Code eErr;

    BDBG_ENTER(BVBI_Close);
    BSTD_UNUSED (eErr);

    /* check parameters */
    pVbi = vbiHandle;
    BDBG_OBJECT_ASSERT (pVbi, BVBI);

    /* Refuse service if user left any decoder objects open */
    BDBG_ASSERT (NULL == BLST_D_FIRST (&pVbi->decode_contexts));

    /* Refuse service if user left any encoder objects open */
    BDBG_ASSERT (NULL == BLST_D_FIRST (&pVbi->encode_contexts));

    /* Close the individual cores.  These same functions were used in
       BVBI_Open(). */
    eErr = BERR_TRACE (BVBI_P_CC_Init   (pVbi));
    BDBG_ASSERT (eErr == BERR_SUCCESS);
    eErr = BERR_TRACE (BVBI_P_CGMS_Init (pVbi));
    BDBG_ASSERT (eErr == BERR_SUCCESS);
    eErr = BERR_TRACE (BVBI_P_WSS_Init  (pVbi));
    BDBG_ASSERT (eErr == BERR_SUCCESS);
    eErr = BERR_TRACE (BVBI_P_TT_Init   (pVbi));
    BDBG_ASSERT (eErr == BERR_SUCCESS);
    eErr = BERR_TRACE (BVBI_P_VPS_Init  (pVbi));
    BDBG_ASSERT (eErr == BERR_SUCCESS);
#if (BVBI_NUM_GSE > 0)
    eErr = BERR_TRACE (BVBI_P_GS_Init  (pVbi));
    BDBG_ASSERT (eErr == BERR_SUCCESS);
#endif
#if (BVBI_NUM_AMOLE > 0)
    eErr = BERR_TRACE (BVBI_P_AMOL_Init  (pVbi));
    BDBG_ASSERT (eErr == BERR_SUCCESS);
#endif
#if (BVBI_NUM_SCTEE > 0)
    eErr = BERR_TRACE (BVBI_P_SCTE_Init  (pVbi));
    BDBG_ASSERT (eErr == BERR_SUCCESS);
#endif
    eErr = BERR_TRACE (BVBI_P_VE_Init   (pVbi));
    BDBG_ASSERT (eErr == BERR_SUCCESS);
#if (BVBI_P_HAS_XSER_TT)
    eErr =
        BERR_TRACE (
            BVBI_P_ITU656_Init (pVbi->hReg, BVBI_P_GetDefaultXserSettings()));
    BDBG_ASSERT (eErr == BERR_SUCCESS);
#endif
#if (BVBI_NUM_ANCI656_656 > 0)
    eErr = BERR_TRACE (BVBI_P_A656_Init (pVbi));
    BDBG_ASSERT (eErr == BERR_SUCCESS);
#endif
#if (BVBI_NUM_IN656 > 0)
    eErr = BERR_TRACE (BVBI_P_IN656_Init(pVbi));
    BDBG_ASSERT (eErr == BERR_SUCCESS);
#endif

    /* Sanity check for bulky storage */
    BDBG_ASSERT (pVbi->ttFreelist.l_co == 0x0);

#ifdef BCHP_PWR_RESOURCE_VDC_VEC
    BCHP_PWR_ReleaseResource(pVbi->hChip, BCHP_PWR_RESOURCE_VDC_VEC);
#endif

    /* Release context in system memory */
    BDBG_OBJECT_DESTROY (pVbi, BVBI);
    BKNI_Free((void*)pVbi);

    BDBG_LEAVE(BVBI_Close);
}

static const BVBI_Capabilities s_Cap =
{
    BVBI_NUM_AMOLE,
    BVBI_NUM_AMOLE_656,
    BVBI_NUM_CCE,
    BVBI_NUM_CCE_656,
    BVBI_NUM_CGMSAE,
    BVBI_NUM_CGMSAE_656,
    BVBI_NUM_GSE,
    BVBI_NUM_GSE_656,
    BVBI_NUM_TTE,
    BVBI_NUM_TTE_656,
    BVBI_NUM_WSE,
    BVBI_NUM_WSE_656,
    BVBI_P_HAS_XSER_TT
};

/***************************************************************************
 *
 */
BERR_Code BVBI_GetCapabilities
    ( BVBI_Handle                      vbiHandle,
      BVBI_Capabilities               *pCapabilities )
{
    BVBI_P_Handle *pVbi;

    BDBG_ENTER(BVBI_GetCapabilities);

    /* check parameters */
    pVbi = vbiHandle;
    BDBG_OBJECT_ASSERT (pVbi, BVBI);

    if (pCapabilities)
    {
        *pCapabilities = s_Cap;
    }

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Standby
    ( BVBI_Handle hVbi,
      BVBI_StandbySettings *pSettings )
{
    BVBI_P_Handle *pVbi;
    BSTD_UNUSED(pSettings); /* unused for now */

    pVbi = hVbi;
    BDBG_OBJECT_ASSERT (pVbi, BVBI);

    /* TODO: if BVBI is in use and there are clocks that need to be kept powered,
       we should short-circuit and return an error. for now, no such check is deemed necessary */
    if (0) {
        BDBG_ERR(("Cannot enter standby due to ( ) in use"));
        return BERR_UNKNOWN;
    }

#ifdef BCHP_PWR_RESOURCE_VDC_VEC
    BCHP_PWR_ReleaseResource(pVbi->hChip, BCHP_PWR_RESOURCE_VDC_VEC);
#endif
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Resume (BVBI_Handle hVbi)
{
    BVBI_P_Handle *pVbi;

    BDBG_ENTER (BVBI_Resume);
    BSTD_UNUSED (pVbi);

    pVbi = hVbi;
    BDBG_OBJECT_ASSERT (pVbi, BVBI);

#ifdef BCHP_PWR_RESOURCE_VDC_VEC
    BCHP_PWR_AcquireResource(pVbi->hChip, BCHP_PWR_RESOURCE_VDC_VEC);
#endif

    BDBG_LEAVE (BVBI_Resume);
    return BERR_SUCCESS;
}

/***************************************************************************
 * VBI private (top level) functions
 ***************************************************************************/

const BVBI_XSER_Settings* BVBI_P_GetDefaultXserSettings (void)
{
    return &s_XSER_DefaultSettings;
}

#if (BSTD_CPU_ENDIAN != BSTD_ENDIAN_LITTLE)
/***************************************************************************
 *
 */
uint32_t BVBI_P_LEBE_SWAP (uint32_t ulDatum)
{
    uint8_t temp;
    union {
        uint8_t b[4];
        uint32_t l;
    } scratch;

    scratch.l = ulDatum;
    P_SWAP (scratch.b[0], scratch.b[3], temp);
    P_SWAP (scratch.b[1], scratch.b[2], temp);

    return scratch.l;
}
#endif

/* End of File */
