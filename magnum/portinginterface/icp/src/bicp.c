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
#include "bstd.h"
#include "bicp.h"
#include "bicp_priv.h"
#include "bchp_icap.h"
#if (BCHP_CHIP == 7125) || (BCHP_CHIP == 7325) || (BCHP_CHIP == 7335) || (BCHP_CHIP == 7340) || (BCHP_CHIP == 7342) || (BCHP_CHIP == 7400) || \
    (BCHP_CHIP == 7405) || (BCHP_CHIP == 7408) || (BCHP_CHIP == 7420) || (BCHP_CHIP == 7468) || (BCHP_CHIP == 7550)
    #include "bchp_irq0.h"
    #include "bchp_int_id_irq0.h"
#elif (BCHP_CHIP == 7271) || (BCHP_CHIP == 7268) || (BCHP_CHIP == 7260) || (BCHP_CHIP == 7278)
    #include "bchp_int_id_upg_main_aon_irq.h"
#else
    #include "bchp_int_id_irq0_aon.h"
    #include "bchp_irq0_aon.h"
#endif

BDBG_MODULE(bicp);

#define DEV_MAGIC_ID            ((BERR_ICP_ID<<16) | 0xFACE)

#define BICP_CHK_RETCODE( rc, func )        \
do {                                        \
    if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
    {                                       \
        goto done;                          \
    }                                       \
} while(0)

#define MAX_ICAP_RCNT               15

/*******************************************************************************
*
*   Private Module Handles
*
*******************************************************************************/

typedef struct BICP_P_Handle
{
    uint32_t        magicId;                    /* Used to check if structure is corrupt */
    BCHP_Handle     hChip;
    BREG_Handle     hRegister;
    BINT_Handle     hInterrupt;
    unsigned int    maxChnNo;
    unsigned int    uiRCPinMask;
    BICP_ChannelHandle hIcpChn[MAX_ICP_CHANNELS];
} BICP_P_Handle;

typedef struct BICP_P_RC6_KEYBIT {
    unsigned char ve;       /* 1: ve+, 0: ve */
    unsigned char used;     /* counted or not */
    unsigned char len;      /* in T unit */
} BICP_P_RC6_KEYBIT;

typedef struct BICP_P_RC6
{
    uint8_t lead;
    uint8_t start;
    uint8_t trailer;
    uint8_t mode;
    uint8_t modebits;
    uint8_t ctrl;
    uint16_t ctrlbits;
    uint8_t total_ctrl_bits;
    uint8_t info;
    uint16_t data;
    uint8_t edge;   /*0: 1->0, 1: 0->1 */
    uint16_t last;
    int                 keycnt;
    BICP_P_RC6_KEYBIT   keybits[314];
} BICP_P_RC6;

typedef struct BICP_P_ChannelHandle
{
    uint32_t            magicId;                    /* Used to check if structure is corrupt */
    BICP_Handle         hIcp;
    uint32_t            chnNo;
    uint32_t            isrCount;
    unsigned int        handleRC6;
    BICP_P_RC6          rc6;
    BKNI_EventHandle    hChnEvent;
    BINT_CallbackHandle hChnCallback;
    BICP_Callback       pInterruptEventUserCallback;
} BICP_P_ChannelHandle;

#if 0
/* move inside handle */
static unsigned int uiRCPinMask = 0;
#endif

#define RCCOUNTER   (1<<16)
#define TUNIT       (27000*16/36)
#define TUNIT14     (TUNIT>>2)
#define TUNIT34     ((TUNIT*3)>>2)
#define INFOBITS    12      /* only support 12 bits info now */

/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/
static const BICP_ChannelSettings defIcpChn0Settings =
{
    MAX_ICAP_RCNT,
    true
};

static const BICP_ChannelSettings defIcpChn1Settings =
{
    MAX_ICAP_RCNT,
    true
};

static const BICP_ChannelSettings defIcpChn2Settings =
{
    MAX_ICAP_RCNT,
    true
};

static const BICP_ChannelSettings defIcpChn3Settings =
{
    MAX_ICAP_RCNT,
    true
};

/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/
BERR_Code BICP_Open(
    BICP_Handle *pIcp,                  /* [output] Returns handle */
    BCHP_Handle hChip,                  /* Chip handle */
    BREG_Handle hRegister,              /* Register handle */
    BINT_Handle hInterrupt,             /* Interrupt handle */
    const BICP_Settings *pDefSettings   /* Default settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BICP_Handle hDev;
    unsigned int chnIdx;
    BSTD_UNUSED(pDefSettings);

    /* Sanity check on the handles we've been given. */
    BDBG_ASSERT( hChip );
    BDBG_ASSERT( hRegister );
    BDBG_ASSERT( hInterrupt );

    /* Alloc memory from the system heap */
    hDev = (BICP_Handle) BKNI_Malloc( sizeof( BICP_P_Handle ) );
    if( hDev == NULL )
    {
        *pIcp = NULL;
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BICP_Open: BKNI_malloc() failed"));
        goto done;
    }

    hDev->magicId   = DEV_MAGIC_ID;
    hDev->hChip     = hChip;
    hDev->hRegister = hRegister;
    hDev->hInterrupt = hInterrupt;
    hDev->maxChnNo  = MAX_ICP_CHANNELS;
    hDev->uiRCPinMask = 0;
    for( chnIdx = 0; chnIdx < hDev->maxChnNo; chnIdx++ )
    {
        hDev->hIcpChn[chnIdx] = NULL;
    }


    *pIcp = hDev;

done:
    return( retCode );
}

BERR_Code BICP_Close(
    BICP_Handle hDev                    /* Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    unsigned int chnIdx;


    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    /* If channel is not closed, close the channel to free up the memory */
    for( chnIdx = 0; chnIdx < hDev->maxChnNo; chnIdx++ )
    {
        if (hDev->hIcpChn[chnIdx])
        {
            BICP_CloseChannel(hDev->hIcpChn[chnIdx]);
        }
    }

    BKNI_Free( (void *) hDev );

    return( retCode );
}

#if !B_REFSW_MINIMAL
BERR_Code BICP_GetDefaultSettings(
    BICP_Settings *pDefSettings,        /* [output] Returns default setting */
    BCHP_Handle hChip                   /* Chip handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(hChip);

    *pDefSettings = NULL; /*none available*/

    return( retCode );
}

BERR_Code BICP_GetTotalChannels(
    BICP_Handle hDev,                   /* Device handle */
    unsigned int *totalChannels         /* [output] Returns total number downstream channels supported */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    *totalChannels = hDev->maxChnNo;

    return( retCode );
}
#endif

BERR_Code BICP_GetChannelDefaultSettings(
    BICP_Handle hDev,                   /* Device handle */
    unsigned int channelNo,             /* Channel number to default setting for */
    BICP_ChannelSettings *pChnDefSettings /* [output] Returns channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

#if !BDBG_DEBUG_BUILD
    BSTD_UNUSED(hDev);
#endif

    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    switch (channelNo)
    {
        case 0:
            *pChnDefSettings = defIcpChn0Settings;
            break;

        case 1:
            *pChnDefSettings = defIcpChn1Settings;
            break;

        case 2:
            *pChnDefSettings = defIcpChn2Settings;
            break;

        case 3:
            *pChnDefSettings = defIcpChn3Settings;
            break;

        default:
            retCode = BERR_INVALID_PARAMETER;
            break;

    }

    return( retCode );
}

BERR_Code BICP_OpenChannel(
    BICP_Handle hDev,                   /* Device handle */
    BICP_ChannelHandle *phChn,          /* [output] Returns channel handle */
    unsigned int channelNo,             /* Channel number to open */
    const BICP_ChannelSettings *pChnDefSettings /* Channel default setting */
    )
{
    BERR_Code           retCode = BERR_SUCCESS;
    BICP_ChannelHandle  hChnDev;

    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hChnDev = NULL;

    if( channelNo < hDev->maxChnNo )
    {
        if( hDev->hIcpChn[channelNo] == NULL )
        {
            /* Alloc memory from the system heap */
            hChnDev = (BICP_ChannelHandle) BKNI_Malloc( sizeof( BICP_P_ChannelHandle ) );
            if( hChnDev == NULL )
            {
                *phChn = NULL;
                retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                BDBG_ERR(("BICP_OpenChannel: BKNI_malloc() failed"));
                goto done;
            }

            BICP_CHK_RETCODE( retCode, BKNI_CreateEvent( &(hChnDev->hChnEvent) ) );
            hChnDev->magicId    = DEV_MAGIC_ID;
            hChnDev->hIcp       = hDev;
            hChnDev->chnNo      = channelNo;
            hChnDev->isrCount   = 0;
            hChnDev->rc6.lead   = 0;
            hChnDev->rc6.start  = 0;
            hChnDev->rc6.trailer    = 0;
            hChnDev->rc6.mode   = 0;
            hChnDev->rc6.modebits   = 0;
            hChnDev->rc6.ctrl   = 0;
            hChnDev->rc6.ctrlbits   = 0;
            hChnDev->rc6.total_ctrl_bits    = 8;
            hChnDev->rc6.info   = 0;
            hChnDev->rc6.data   = 0;
            hChnDev->rc6.edge   = 0;
            hChnDev->rc6.last   = 0;
            hChnDev->rc6.keycnt = 0;
            hChnDev->pInterruptEventUserCallback = NULL;
            hDev->hIcpChn[channelNo] = hChnDev;

            /* Program glitch rejector count */
            BICP_CHK_RETCODE( retCode, BICP_P_SetRejectCnt (hChnDev, pChnDefSettings->rejectCnt) );

            /*
             * Enable interrupt for this channel
             */
            if (pChnDefSettings->intMode == true)
            {
                /*
                 * Register and enable L2 interrupt.
                 */
                #if defined(BCHP_INT_ID_icap_irqen)
                    BICP_CHK_RETCODE( retCode, BINT_CreateCallback(
                        &(hChnDev->hChnCallback), hDev->hInterrupt, BCHP_INT_ID_icap_irqen,
                        BICP_P_HandleInterrupt_Isr, (void *) hChnDev, (1 << channelNo) ) );
                #elif defined(BCHP_INT_ID_icap)
                    BICP_CHK_RETCODE( retCode, BINT_CreateCallback(
                        &(hChnDev->hChnCallback), hDev->hInterrupt, BCHP_INT_ID_icap,
                        BICP_P_HandleInterrupt_Isr, (void *) hChnDev, (1 << channelNo) ) );
                #else
                    #error BCHP_CHIP set to unsupport chip.
                #endif
                BICP_CHK_RETCODE( retCode, BINT_EnableCallback( hChnDev->hChnCallback ) );

                /*
                 * Enable ICP interrupt in ICP
                 */
                BKNI_EnterCriticalSection();
                BICP_P_EnableInt (hChnDev);
                BKNI_LeaveCriticalSection();
            }

            *phChn = hChnDev;
        }
        else
        {
            retCode = BICP_ERR_NOTAVAIL_CHN_NO;
        }
    }
    else
    {
        retCode = BERR_INVALID_PARAMETER;
    }

done:
    if( retCode != BERR_SUCCESS )
    {
        if( hChnDev != NULL )
        {
            BKNI_DestroyEvent( hChnDev->hChnEvent );
            BKNI_Free( hChnDev );
            hDev->hIcpChn[channelNo] = NULL;
            *phChn = NULL;
        }
    }
    return( retCode );
}

BERR_Code BICP_CloseChannel(
    BICP_ChannelHandle hChn         /* Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BICP_Handle hDev;
    unsigned int chnNo;


    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hDev = hChn->hIcp;

    /* Reset uiRCPinMask if channel is RC6 */
    if (hChn->handleRC6)
    {
        hDev->uiRCPinMask &= ~(1<<hChn->chnNo);
    }

    /*
     * Disable interrupt for this channel
     */
    BKNI_EnterCriticalSection();
    BICP_P_DisableInt (hChn);
    BKNI_LeaveCriticalSection();

    BICP_CHK_RETCODE( retCode, BINT_DisableCallback( hChn->hChnCallback ) );
    BICP_CHK_RETCODE( retCode, BINT_DestroyCallback( hChn->hChnCallback ) );
    BKNI_DestroyEvent( hChn->hChnEvent );
    chnNo = hChn->chnNo;
    BKNI_Free( hChn );
    hDev->hIcpChn[chnNo] = NULL;

done:
    return( retCode );
}

#if !B_REFSW_MINIMAL
BERR_Code BICP_GetDevice(
    BICP_ChannelHandle hChn,            /* Device channel handle */
    BICP_Handle *phDev                  /* [output] Returns Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    *phDev = hChn->hIcp;

    return( retCode );
}
#endif

BERR_Code BICP_GetEventHandle(
    BICP_ChannelHandle hChn,            /* Device channel handle */
    BKNI_EventHandle *phEvent           /* [output] Returns event handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );


    *phEvent = hChn->hChnEvent;
    return( retCode );
}

BERR_Code BICP_EnableEdge(
    BICP_ChannelHandle  hChn,           /* Device channel handle */
    BICP_EdgeConfig     edge            /* edge config */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval, mask;
    BICP_Handle hDev;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hDev = hChn->hIcp;

    lval = BREG_Read32(hDev->hRegister, BCHP_ICAP_INEDGE);
    mask = 1 << hChn->chnNo;
    if (edge == BICP_EdgeConfig_ePositive || edge == BICP_EdgeConfig_eBoth)
        lval |= (mask << BCHP_ICAP_INEDGE_icap_pedgedet_SHIFT);
    if (edge == BICP_EdgeConfig_eNegative || edge == BICP_EdgeConfig_eBoth)
        lval |= mask;

    BREG_Write32(hDev->hRegister, BCHP_ICAP_INEDGE, lval);

    return( retCode );
}

#if !B_REFSW_MINIMAL
BERR_Code BICP_DisableEdge(
    BICP_ChannelHandle  hChn,           /* Device channel handle */
    BICP_EdgeConfig     edge            /* edge config */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval, mask;
    BICP_Handle hDev;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hDev = hChn->hIcp;

    lval = BREG_Read32(hDev->hRegister, BCHP_ICAP_INEDGE);
    mask = 1 << hChn->chnNo;
    if (edge == BICP_EdgeConfig_ePositive || edge == BICP_EdgeConfig_eBoth)
        lval &= ~(mask << BCHP_ICAP_INEDGE_icap_pedgedet_SHIFT);
    if (edge == BICP_EdgeConfig_eNegative || edge == BICP_EdgeConfig_eBoth)
        lval &= ~mask;
    BREG_Write32(hDev->hRegister, BCHP_ICAP_INEDGE, lval);

    return( retCode );
}
#endif

BERR_Code BICP_GetTimerCnt(
    BICP_ChannelHandle  hChn,           /* Device channel handle */
    uint16_t            *timerCnt       /* pointer to count */
    )
{
    BERR_Code       retCode = BERR_SUCCESS;
    uint32_t        msb = 0, lsb = 0;
    BICP_Handle     hDev;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hDev = hChn->hIcp;

    switch (hChn->chnNo)
    {
        case 0:
            msb = BREG_Read32(hDev->hRegister, BCHP_ICAP_TCNT0MSB);
            lsb = BREG_Read32(hDev->hRegister, BCHP_ICAP_TCNT0LSB);
            break;

        case 1:
            msb = BREG_Read32(hDev->hRegister, BCHP_ICAP_TCNT1MSB);
            lsb = BREG_Read32(hDev->hRegister, BCHP_ICAP_TCNT1LSB);
            break;

        case 2:
            msb = BREG_Read32(hDev->hRegister, BCHP_ICAP_TCNT2MSB);
            lsb = BREG_Read32(hDev->hRegister, BCHP_ICAP_TCNT2LSB);
            break;

        case 3:
            msb = BREG_Read32(hDev->hRegister, BCHP_ICAP_TCNT3MSB);
            lsb = BREG_Read32(hDev->hRegister, BCHP_ICAP_TCNT3LSB);
            break;

    }
    *timerCnt = ((uint16_t)msb << 8) | ((uint16_t)lsb);

    return( retCode );
}

#if !B_REFSW_MINIMAL
BERR_Code BICP_PollTimer(
    BICP_ChannelHandle  hChn,           /* Device channel handle */
    bool                *triggered,     /* status of trigger */
    uint16_t            *timerCnt       /* pointer to count */
    )
{
    BERR_Code       retCode = BERR_SUCCESS;
    uint32_t        mask, lval;
    BICP_Handle     hDev;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hDev = hChn->hIcp;
    lval = BREG_Read32(hDev->hRegister, BCHP_ICAP_INSTATUS);
    mask = 1 << hChn->chnNo;
    *triggered =  (lval & mask) ? true : false;
    if (*triggered)
    {
        /* Reset interrupt pins */
        BREG_Write32(hDev->hRegister, BCHP_ICAP_RST, mask);
        BREG_Write32(hDev->hRegister, BCHP_ICAP_RST, 0);

        /* Get the count for caller */
        BICP_CHK_RETCODE (retCode, BICP_GetTimerCnt (hChn, timerCnt) );
    }

done:
    return( retCode );
}

void BICP_EnableInt(
    BICP_ChannelHandle  hChn            /* Device channel handle */
    )
{
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BICP_P_EnableInt( hChn );
}

void BICP_DisableInt(
    BICP_ChannelHandle  hChn            /* Device channel handle */
    )
{
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BICP_P_DisableInt( hChn );
}

void BICP_EnableRC6(
    BICP_ChannelHandle  hChn,           /* Device channel handle */
    BICP_Callback pCallback             /* Pointer to completion callback. */
    )
{
    BICP_Handle hDev;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    hChn->handleRC6 = 1;
    hChn->pInterruptEventUserCallback = pCallback;
    hDev = hChn->hIcp;
    hDev->uiRCPinMask |= (1<<hChn->chnNo);
}

void BICP_DisableRC6(
    BICP_ChannelHandle  hChn            /* Device channel handle */
    )
{
    BICP_Handle hDev;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    hChn->handleRC6 = 0;

    hDev = hChn->hIcp;
    hDev->uiRCPinMask &= ~(1<<hChn->chnNo);
}

void BICP_ResetIntCount(
    BICP_ChannelHandle  hChn            /* Device channel handle */
    )
{
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    hChn->isrCount = 0;
}

void BICP_GetIntCount(
    BICP_ChannelHandle  hChn,           /* Device channel handle */
    uint32_t            *data
    )
{
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    *data = hChn->isrCount;
}
#endif

/*******************************************************************************
*
*   Private Module Functions
*
*******************************************************************************/
BERR_Code BICP_P_SetRejectCnt(
    BICP_ChannelHandle  hChn,           /* Device channel handle */
    uint8_t             clks            /* number of clocks to reject */
    )
{
    uint32_t    lval;
    BICP_Handle hDev;
    BERR_Code   retCode = BERR_SUCCESS;

    hDev = hChn->hIcp;

    lval = clks - 1;
    if (lval > MAX_ICAP_RCNT)
    {
        retCode = BERR_INVALID_PARAMETER;
        goto done;
    }

    switch (hChn->chnNo)
    {
        case 0:
            BREG_Write32(hDev->hRegister, BCHP_ICAP_RCNT0, lval);
            break;

        case 1:
            BREG_Write32(hDev->hRegister, BCHP_ICAP_RCNT1, lval);
            break;

        case 2:
            BREG_Write32(hDev->hRegister, BCHP_ICAP_RCNT2, lval);
            break;

        case 3:
            BREG_Write32(hDev->hRegister, BCHP_ICAP_RCNT3, lval);
            break;

        default:
            retCode = BERR_INVALID_PARAMETER;
            goto done;
    }

done:
    return retCode;
}

void BICP_P_EnableInt(
    BICP_ChannelHandle  hChn            /* Device channel handle */
    )
{
    uint32_t    lval;
    BICP_Handle hDev;

    hDev = hChn->hIcp;

    lval = BREG_Read32(hDev->hRegister, BCHP_ICAP_MASK);
    lval |= (1 << hChn->chnNo);
    BREG_Write32(hDev->hRegister, BCHP_ICAP_MASK, lval);

}

void BICP_P_DisableInt(
    BICP_ChannelHandle  hChn            /* Device channel handle */
    )
{
    uint32_t    lval;
    BICP_Handle hDev;

    hDev = hChn->hIcp;

    lval = BREG_Read32(hDev->hRegister, BCHP_ICAP_MASK);
    lval &= ~(1 << hChn->chnNo);
    BREG_Write32(hDev->hRegister, BCHP_ICAP_MASK, lval);

}

void BICP_P_RC6Handle(BICP_ChannelHandle hIcpChan, uint8_t reg)
{
    BICP_Handle hDev;
    uint16_t cur=0;
    uint32_t len, len1, adj_count;
    uint8_t ve1, ve2;
    uint8_t used1, used2;
    uint8_t found_key = 0;

/* T = 1/36KHZ * 16 = 444.44us, 7115 counter is 27MHz, so T = 12000 ticks
   1bit data will use 2T with mancherster coding.
   RC6 data sequence is:
   Leader: 8T (6T ve+, 2T ve-)
   Start : 2T (ve+, ve-)
   Mode  : 6T (3 bits)
   Trailer:4T (2T ve+, 2T ve-)
   Control:16T/32T(8bits/16bits, depends on the first bit, 0: 8, 1: 16)
   Info  : 16T-256T(8bits-128bits)
*/

    BDBG_ASSERT( hIcpChan );
    hDev = hIcpChan->hIcp;

    if (reg & hDev->uiRCPinMask){
        BICP_GetTimerCnt(hIcpChan, &cur);

        hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt].ve = 1-hIcpChan->rc6.edge;  /* edge 0: 1->0, ve 1: ve+ */
        hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt].used = 0;
        if (hIcpChan->rc6.keycnt){  /* now we can tell last ve len */
            /* counter is 16 bits, there is chance of wrap around */
            if (cur > hIcpChan->rc6.last)
                len = cur - hIcpChan->rc6.last;
            else{
                len = RCCOUNTER - hIcpChan->rc6.last + cur;
            }

            if ((hIcpChan->rc6.keycnt == 1) && (len < 9800 ))/*Fudge delta after a key is found */
                len += RCCOUNTER;

            if (len < TUNIT34)              /*I bet it is leader bit 6T, wrap around */
                len += RCCOUNTER;

            adj_count = len;
            len = adj_count / TUNIT;
            if ((adj_count % TUNIT) > (TUNIT/2))
                len++;                      /* round up */

            /*  Check for invalid lengths and cross check len with state;  */
            if ((len == 5) || ((len == 6) && (hIcpChan->rc6.lead) )) {
                hIcpChan->rc6.keycnt=0;     /* Start over */
                hIcpChan->rc6.edge = 0;
                hIcpChan->rc6.last = cur;

                hIcpChan->rc6.lead = hIcpChan->rc6.start = hIcpChan->rc6.mode = hIcpChan->rc6.trailer = 0;
                hIcpChan->rc6.ctrl = hIcpChan->rc6.info = 0;
                hIcpChan->rc6.ctrlbits = hIcpChan->rc6.modebits = 0;
                return;
            }




            if ((len == 6) && hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-1].ve)     /* assume found lead bit */
            {
                hIcpChan->rc6.keybits[0].ve = hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-1].ve;
                hIcpChan->rc6.keybits[0].used = hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-1].used;
                hIcpChan->rc6.keycnt = 1;
            }

            hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-1].len = len;

            if (hIcpChan->rc6.keycnt > 1){
                ve1 = hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-2].ve;
                len1 = hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-2].len;
                used1 = hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-2].used;
                used2 = hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-1].used;
                ve2 = hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-1].ve;

                if (!used1 && !used2){  /* make sure 2 states captured */

                    if (!hIcpChan->rc6.lead){
                        if ((len1 == 6) && ve1 && (len>1) && !ve2){ /* found lead bit */
                            hIcpChan->rc6.lead = 1;
                            hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-2].used = 1;
                            hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-1].used = 1;
                        }
                    }
                    else if (!hIcpChan->rc6.start){
                        if ((len1 == 1) && ve1 && !ve2){
                            hIcpChan->rc6.start = 1;
                            hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-2].used = 1;
                            if (len==1)             /* next dat is same as current one */
                                hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-1].used = 1;
                        }
                    }
                    else if (hIcpChan->rc6.mode != 3){
                        hIcpChan->rc6.modebits <<= 1;
                        if (ve1 && !ve2)
                            hIcpChan->rc6.modebits |= 1;
                        hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-2].used = 1;
                        if (len==1)             /* next dat is same as current one */
                            hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-1].used = 1;
                        hIcpChan->rc6.mode=hIcpChan->rc6.mode+1;
                    }
                    else if (!hIcpChan->rc6.trailer){
                        if ((len1>=2) && (len>=2)){
                            hIcpChan->rc6.trailer = 1;
                            hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-2].used = 1;
                            if (len == 2)   /* next dat is same as current one */
                                hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-1].used = 1;
                        }
                    }
                    else if(hIcpChan->rc6.ctrl != hIcpChan->rc6.total_ctrl_bits){
                        hIcpChan->rc6.ctrlbits <<= 1;
                        if (ve1 && !ve2)
                            hIcpChan->rc6.ctrlbits |= 1;

                        hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-2].used = 1;
                        if (len==1)             /* next dat is same as current one */
                            hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-1].used = 1;

                        if (!hIcpChan->rc6.ctrl){
                            if (hIcpChan->rc6.ctrlbits)
                                hIcpChan->rc6.total_ctrl_bits = 16;
                            else
                                hIcpChan->rc6.total_ctrl_bits = 8;
                        }

                        hIcpChan->rc6.ctrl++;
                    }
                    else if( hIcpChan->rc6.lead && hIcpChan->rc6.start && hIcpChan->rc6.mode && hIcpChan->rc6.trailer && hIcpChan->rc6.ctrl){   /* it is data now */
                        hIcpChan->rc6.data <<= 1;
                        if (ve1 && !ve2){   /* it is 1 */
                            hIcpChan->rc6.data |= 1;
                        }
                        if (!ve1 && ve2){   /* it is 0 */
                        }
                        hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-2].used = 1;
                        if (len==1)             /* next dat is same as current one */
                            hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-1].used = 1;
                        hIcpChan->rc6.info++;
                        if (hIcpChan->rc6.info == INFOBITS){    /* got data */
                            found_key = 1;
                        }
                    }
                    else{
                        BDBG_ERR(("RC6 key something wrong, redo"));
                        hIcpChan->rc6.lead = hIcpChan->rc6.start = hIcpChan->rc6.mode = hIcpChan->rc6.trailer = hIcpChan->rc6.ctrl = hIcpChan->rc6.info = 0;
                        hIcpChan->rc6.keycnt = hIcpChan->rc6.ctrlbits = hIcpChan->rc6.modebits = 0;
                        hIcpChan->rc6.edge = 0;
                    }
                }

                /* for last bit == 1, it is ve+ and ve- */
                if ((hIcpChan->rc6.info == INFOBITS-1) && !hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-1].used){     /* this is fake */
                    if (ve2){
                        hIcpChan->rc6.data <<= 1;
                        hIcpChan->rc6.data |= 1;
                        hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt-1].used = 1;
                        hIcpChan->rc6.keybits[hIcpChan->rc6.keycnt].used = 1;
                        found_key = 1;
                    }
                }

            }
        }
        hIcpChan->rc6.keycnt++;
        if ((hIcpChan->rc6.keycnt>4) && !hIcpChan->rc6.lead)
        {
            hIcpChan->rc6.keycnt=0;     /* Start over */
            hIcpChan->rc6.edge = 0;
            hIcpChan->rc6.last = 0;

            hIcpChan->rc6.lead = hIcpChan->rc6.start = hIcpChan->rc6.mode = hIcpChan->rc6.trailer = 0;
            hIcpChan->rc6.ctrl = hIcpChan->rc6.info = 0;
            hIcpChan->rc6.ctrlbits = hIcpChan->rc6.modebits = 0;
        }
        else
        {
            hIcpChan->rc6.edge = 1-hIcpChan->rc6.edge;
            hIcpChan->rc6.last = cur;
        }
    }
    if (found_key){
        /* Invoke callback */
        if( hIcpChan->pInterruptEventUserCallback != NULL )
        {
            hIcpChan->pInterruptEventUserCallback( hIcpChan->rc6.ctrlbits, hIcpChan->rc6.modebits, hIcpChan->rc6.data );
        }
        hIcpChan->rc6.lead = hIcpChan->rc6.start = hIcpChan->rc6.mode = hIcpChan->rc6.trailer = hIcpChan->rc6.ctrl = hIcpChan->rc6.info = 0;
        hIcpChan->rc6.keycnt = hIcpChan->rc6.ctrlbits = hIcpChan->rc6.modebits = 0;
        hIcpChan->rc6.edge = 0;
        hIcpChan->rc6.data = 0;
        found_key = 0;
    }
}

static void BICP_P_HandleInterrupt_Isr
(
    void *pParam1,                      /* Device channel handle */
    int parm2                           /* not used */
)
{
    BICP_ChannelHandle  hChn;
    BICP_Handle         hDev;
    uint32_t            lval;
    uint32_t            mask;

    hChn = (BICP_ChannelHandle) pParam1;
    BDBG_ASSERT( hChn );

    hDev = hChn->hIcp;

    /* Read interrupt status register */
    lval = BREG_Read32(hDev->hRegister, BCHP_ICAP_INSTATUS);

    /* Is this interrupt for this channel? */
    mask = lval & parm2;

    if (!mask)
        return;

    hChn->isrCount++;

    /* Reset interrupt pins */
    BREG_Write32(hDev->hRegister, BCHP_ICAP_RST, mask);
    BREG_Write32(hDev->hRegister, BCHP_ICAP_RST, 0);

    if (hChn->handleRC6)
    {
        BICP_P_RC6Handle(hChn, (unsigned char)(lval & 0xf));
    }

    BKNI_SetEvent( hChn->hChnEvent );
    return;
}
