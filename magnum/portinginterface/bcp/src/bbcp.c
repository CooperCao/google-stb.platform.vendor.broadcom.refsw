/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
#include "bstd.h"
#include "bbcp.h"
#include "bchp_bicap.h"
#include "bchp_sun_top_ctrl.h"

#if (BCHP_CHIP==7125) || (BCHP_CHIP==7325) || (BCHP_CHIP==7335) || (BCHP_CHIP==7336) || (BCHP_CHIP==7340) || \
	(BCHP_CHIP==7405) || (BCHP_CHIP==7408) || (BCHP_CHIP==7420) || (BCHP_CHIP==7468) || (BCHP_CHIP==7550)
    #include "bchp_int_id_mcif_intr2.h"
#else
    #include "bchp_int_id_upg_aux_aon_intr2.h"
    #include "bchp_pm_aon.h"
#endif

BDBG_MODULE(bbcp);

#define DEV_MAGIC_ID            ((BERR_BCP_ID<<16) | 0xFACE)

#define BBCP_CHK_RETCODE( rc, func )        \
do {                                        \
    if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
    {                                       \
        goto done;                          \
    }                                       \
} while(0)

#define SINGLE_EVENT

#ifdef BCHP_INT_ID_BICAP1_FIFO_OF_INTR
    #define BCHP_INT_ID_BICAP_FIFO_OF_INTR          BCHP_INT_ID_BICAP1_FIFO_OF_INTR
    #define BCHP_INT_ID_BICAP_FIFO_LVL_INTR         BCHP_INT_ID_BICAP1_FIFO_LVL_INTR
    #define BCHP_INT_ID_BICAP_FIFO_INACT_INTR       BCHP_INT_ID_BICAP1_FIFO_INACT_INTR
    #define BCHP_INT_ID_BICAP_TIMEOUT3_INTR         BCHP_INT_ID_BICAP1_TIMEOUT3_INTR
    #define BCHP_INT_ID_BICAP_TIMEOUT2_INTR         BCHP_INT_ID_BICAP1_TIMEOUT2_INTR
    #define BCHP_INT_ID_BICAP_TIMEOUT1_INTR         BCHP_INT_ID_BICAP1_TIMEOUT1_INTR
    #define BCHP_INT_ID_BICAP_TIMEOUT0_INTR         BCHP_INT_ID_BICAP1_TIMEOUT0_INTR
#endif

/* Forward reference */
static void BBCP_P_HandleFifoOfInterrupt_Isr(void *pParam1, int parm2);
static void BBCP_P_HandleFifoIrqInterrupt_Isr(void *pParam1, int parm2);
static void BBCP_P_HandleCpuTimeoutIrqInterrupt_Isr(void *pParam1, int parm2);
static void BBCP_P_HandleEdgeTimeoutIrqInterrupt_Isr(void *pParam1, int parm2);


/*******************************************************************************
*
*   Private Module Handles
*
*******************************************************************************/

typedef struct BBCP_P_Handle
{
    uint32_t            magicId;                    /* Used to check if structure is corrupt */
    BCHP_Handle         hChip;
    BREG_Handle         hRegister;
    BINT_Handle         hInterrupt;
    unsigned int        maxChnNo;
    unsigned int        uiRCPinMask;
    bool                intMode;
    uint32_t            isrCount;
    #ifdef SINGLE_EVENT
        BKNI_EventHandle    hEvent;
    #else
        BKNI_EventHandle    hFifoOfEvent;
        BKNI_EventHandle    hFifoIrqEvent;
        BKNI_EventHandle    hCpuTimeoutEvent;
        BKNI_EventHandle    hEdgeTimeoutEvent[4];
    #endif
    BINT_CallbackHandle hFifoOfCallback;
    BINT_CallbackHandle hFifoIrqCallback;
    BINT_CallbackHandle hCpuTimeoutIrqCallback;
    BINT_CallbackHandle hEdgeTimeoutIrqCallback[4];
    BBCP_ChannelHandle  hBcpChn[MAX_BCP_CHANNELS];
} BBCP_P_Handle;

typedef struct BBCP_P_RC6_KEYBIT {
    unsigned char ve;       /* 1: ve+, 0: ve */
    unsigned char used;     /* counted or not */
    unsigned char len;      /* in T unit */
} BBCP_P_RC6_KEYBIT;

typedef struct BBCP_P_RC6
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
    BBCP_P_RC6_KEYBIT   keybits[314];
} BBCP_P_RC6;

typedef struct BBCP_P_ChannelHandle
{
    uint32_t            magicId;                    /* Used to check if structure is corrupt */
    BBCP_Handle         hBcp;
    uint32_t            chnNo;
    unsigned int        handleRC6;
    BBCP_P_RC6          rc6;
    BBCP_Callback       pInterruptEventUserCallback;
} BBCP_P_ChannelHandle;

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
static const BBCP_Settings defBcpSettings =
{
    true
};

static const BBCP_ChannelSettings defBcpChn0Settings =
{
    {
        0,          /* tout clk div */
        0xf,        /* sys out clk div */
        0,          /* cnt mode */
        1,          /* pedgedet_en */
        1,          /* nedgedet_en */
        0,          /* insig_inv */
        1           /* bicap_en */
    },
    {
        0,          /* bypass filter */
        0,          /* which clock */
        0           /* value */
    },
};

static const BBCP_ChannelSettings defBcpChn1Settings =
{
    {
        0,          /* tout clk div */
        0xf,        /* sys out clk div */
        0,          /* cnt mode */
        1,          /* pedgedet_en */
        1,          /* nedgedet_en */
        0,          /* insig_inv */
        1           /* bicap_en */
    },
    {
        0,          /* bypass filter */
        0,          /* which clock */
        0           /* value */
    },
};

/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/
BERR_Code BBCP_Open(
    BBCP_Handle *pBcp,                  /* [output] Returns handle */
    BCHP_Handle hChip,                  /* Chip handle */
    BREG_Handle hRegister,              /* Register handle */
    BINT_Handle hInterrupt,             /* Interrupt handle */
    const BBCP_Settings *pDefSettings   /* Default settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BBCP_Handle hDev;
    unsigned int i;
    #ifdef BCHP_PM_AON_CLK_CTRL_bicap_MASK
        uint32_t lval;
    #endif
    BBCP_Timeout_Settings timeout;

    /* Sanity check on the handles we've been given. */
    BDBG_ASSERT( hChip );
    BDBG_ASSERT( hRegister );
    BDBG_ASSERT( hInterrupt );

    /* Alloc memory from the system heap */
    hDev = (BBCP_Handle) BKNI_Malloc( sizeof( BBCP_P_Handle ) );
    if( hDev == NULL )
    {
        *pBcp = NULL;
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BBCP_Open: BKNI_malloc() failed\n"));
        goto done;
    }

    hDev->magicId   = DEV_MAGIC_ID;
    hDev->hChip     = hChip;
    hDev->hRegister = hRegister;
    hDev->hInterrupt = hInterrupt;
    hDev->maxChnNo  = MAX_BCP_CHANNELS;
    hDev->uiRCPinMask = 0;
    hDev->intMode = pDefSettings->intMode;
    for( i = 0; i < hDev->maxChnNo; i++ )
        hDev->hBcpChn[i] = NULL;

    /* Enable clock */
    #ifdef BCHP_PM_AON_CLK_CTRL_bicap_MASK
        lval = BREG_Read32( hDev->hRegister, BCHP_PM_AON_CLK_CTRL );
        lval |= BCHP_PM_AON_CLK_CTRL_bicap_MASK;
        BREG_Write32( hDev->hRegister, BCHP_PM_AON_CLK_CTRL, lval );
    #endif

    /* Set trigger level */
    BBCP_CHK_RETCODE( retCode, BBCP_SetFifoTrigLevel(hDev, 0) );

    /* Initialize timeouts */
    timeout.input_sel=0;
    timeout.edge_sel=0;
    timeout.clk_sel=0;
    timeout.tout=0;
    for (i=0; i<4; i++)
        BBCP_CHK_RETCODE( retCode, BBCP_SetTimeout(hDev, i, &timeout) );

    /*
     * Enable interrupt for this channel
     */
    if (hDev->intMode == true)
    {
        #ifdef SINGLE_EVENT
            BBCP_CHK_RETCODE( retCode, BKNI_CreateEvent( &(hDev->hEvent) ) );
        #else
            BBCP_CHK_RETCODE( retCode, BKNI_CreateEvent( &(hDev->hFifoOfEvent) ) );
            BBCP_CHK_RETCODE( retCode, BKNI_CreateEvent( &(hDev->hFifoIrqEvent) ) );
            BBCP_CHK_RETCODE( retCode, BKNI_CreateEvent( &(hDev->hCpuTimeoutEvent) ) );
            BBCP_CHK_RETCODE( retCode, BKNI_CreateEvent( &(hDev->hEdgeTimeoutEvent[0]) ) );
            BBCP_CHK_RETCODE( retCode, BKNI_CreateEvent( &(hDev->hEdgeTimeoutEvent[1]) ) );
            BBCP_CHK_RETCODE( retCode, BKNI_CreateEvent( &(hDev->hEdgeTimeoutEvent[2]) ) );
            BBCP_CHK_RETCODE( retCode, BKNI_CreateEvent( &(hDev->hEdgeTimeoutEvent[3]) ) );
        #endif

        /*
         * Register and enable L2 interrupt.
         */
        BBCP_CHK_RETCODE( retCode, BINT_CreateCallback(
            &(hDev->hFifoOfCallback), hDev->hInterrupt, BCHP_INT_ID_BICAP_FIFO_OF_INTR,
            BBCP_P_HandleFifoOfInterrupt_Isr, (void *) hDev, 0 ) );
        BBCP_CHK_RETCODE( retCode, BINT_EnableCallback( hDev->hFifoOfCallback ) );

        BBCP_CHK_RETCODE( retCode, BINT_CreateCallback(
            &(hDev->hFifoIrqCallback), hDev->hInterrupt, BCHP_INT_ID_BICAP_FIFO_LVL_INTR,
            BBCP_P_HandleFifoIrqInterrupt_Isr, (void *) hDev, 0 ) );
        BBCP_CHK_RETCODE( retCode, BINT_EnableCallback( hDev->hFifoIrqCallback ) );

        BBCP_CHK_RETCODE( retCode, BINT_CreateCallback(
            &(hDev->hCpuTimeoutIrqCallback), hDev->hInterrupt, BCHP_INT_ID_BICAP_FIFO_INACT_INTR,
            BBCP_P_HandleCpuTimeoutIrqInterrupt_Isr, (void *) hDev, 0 ) );
        BBCP_CHK_RETCODE( retCode, BINT_EnableCallback( hDev->hCpuTimeoutIrqCallback ) );

        BBCP_CHK_RETCODE( retCode, BINT_CreateCallback(
            &(hDev->hEdgeTimeoutIrqCallback[0]), hDev->hInterrupt, BCHP_INT_ID_BICAP_TIMEOUT0_INTR,
            BBCP_P_HandleEdgeTimeoutIrqInterrupt_Isr, (void *) hDev, 0 ) );
        BBCP_CHK_RETCODE( retCode, BINT_EnableCallback( hDev->hEdgeTimeoutIrqCallback[0] ) );

        BBCP_CHK_RETCODE( retCode, BINT_CreateCallback(
            &(hDev->hEdgeTimeoutIrqCallback[1]), hDev->hInterrupt, BCHP_INT_ID_BICAP_TIMEOUT1_INTR,
            BBCP_P_HandleEdgeTimeoutIrqInterrupt_Isr, (void *) hDev, 1 ) );
        BBCP_CHK_RETCODE( retCode, BINT_EnableCallback( hDev->hEdgeTimeoutIrqCallback[1] ) );

        BBCP_CHK_RETCODE( retCode, BINT_CreateCallback(
            &(hDev->hEdgeTimeoutIrqCallback[2]), hDev->hInterrupt, BCHP_INT_ID_BICAP_TIMEOUT2_INTR,
            BBCP_P_HandleEdgeTimeoutIrqInterrupt_Isr, (void *) hDev, 2 ) );
        BBCP_CHK_RETCODE( retCode, BINT_EnableCallback( hDev->hEdgeTimeoutIrqCallback[2] ) );

        BBCP_CHK_RETCODE( retCode, BINT_CreateCallback(
            &(hDev->hEdgeTimeoutIrqCallback[3]), hDev->hInterrupt, BCHP_INT_ID_BICAP_TIMEOUT3_INTR,
            BBCP_P_HandleEdgeTimeoutIrqInterrupt_Isr, (void *) hDev, 3 ) );
        BBCP_CHK_RETCODE( retCode, BINT_EnableCallback( hDev->hEdgeTimeoutIrqCallback[3] ) );
    }

    *pBcp = hDev;

done:
    if( retCode != BERR_SUCCESS )
    {
        if( hDev != NULL )
        {
            #ifdef SINGLE_EVENT
                BKNI_DestroyEvent( hDev->hEvent );
            #else
                BKNI_DestroyEvent( hDev->hFifoOfEvent );
                BKNI_DestroyEvent( hDev->hFifoIrqEvent );
                BKNI_DestroyEvent( hDev->hCpuTimeoutEvent );
                for (i=0; i<4; i++)
                    BKNI_DestroyEvent( hDev->hEdgeTimeoutEvent[i] );
            #endif
            BKNI_Free( hDev );
            *pBcp = NULL;
        }
    }
    return( retCode );
}

BERR_Code BBCP_Close(
    BBCP_Handle hDev                    /* Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    unsigned int i;
    #ifdef BCHP_PM_AON_CLK_CTRL_bicap_MASK
        uint32_t lval;
    #endif

    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    if (hDev->intMode == true)
    {
        /*
         * Disable interrupt.
         */
        BINT_DisableCallback( hDev->hFifoOfCallback );
        BINT_DestroyCallback( hDev->hFifoOfCallback );
        BINT_DisableCallback( hDev->hFifoIrqCallback );
        BINT_DestroyCallback( hDev->hFifoIrqCallback );
        BINT_DisableCallback( hDev->hCpuTimeoutIrqCallback );
        BINT_DestroyCallback( hDev->hCpuTimeoutIrqCallback );
        for (i=0; i<4; i++) {
            BINT_DisableCallback( hDev->hEdgeTimeoutIrqCallback[i] );
            BINT_DestroyCallback( hDev->hEdgeTimeoutIrqCallback[i] );
        }
        #ifdef SINGLE_EVENT
            BKNI_DestroyEvent( hDev->hEvent );
        #else
            BKNI_DestroyEvent( hDev->hFifoOfEvent );
            BKNI_DestroyEvent( hDev->hFifoIrqEvent );
            BKNI_DestroyEvent( hDev->hCpuTimeoutEvent );
            for (i=0; i<4; i++)
                BKNI_DestroyEvent( hDev->hEdgeTimeoutEvent[i] );
        #endif
    }

    /* Disable clock */
    #ifdef BCHP_PM_AON_CLK_CTRL_bicap_MASK
        lval = BREG_Read32( hDev->hRegister, BCHP_PM_AON_CLK_CTRL );
        lval &= ~BCHP_PM_AON_CLK_CTRL_bicap_MASK;
        BREG_Write32( hDev->hRegister, BCHP_PM_AON_CLK_CTRL, lval );
    #endif

    /* If channel is not closed, close the channel to free up the memory */
    for( i = 0; i < hDev->maxChnNo; i++ )
    {
        if (hDev->hBcpChn[i])
        {
            BBCP_CloseChannel(hDev->hBcpChn[i]);
        }
    }

    BKNI_Free( (void *) hDev );

    return( retCode );
}

BERR_Code BBCP_GetDefaultSettings(
    BBCP_Settings *pDefSettings     /* [output] Returns default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    *pDefSettings = defBcpSettings;

    return( retCode );
}

BERR_Code BBCP_GetTotalChannels(
    BBCP_Handle hDev,                   /* Device handle */
    unsigned int *totalChannels         /* [output] Returns total number downstream channels supported */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    *totalChannels = hDev->maxChnNo;

    return( retCode );
}

BERR_Code BBCP_GetChannelDefaultSettings(
    BBCP_Handle hDev,                   /* Device handle */
    unsigned int channelNo,             /* Channel number to default setting for */
    BBCP_ChannelSettings *pChnDefSettings /* [output] Returns channel default setting */
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
            *pChnDefSettings = defBcpChn0Settings;
            break;

        case 1:
            *pChnDefSettings = defBcpChn1Settings;
            break;

        default:
            retCode = BERR_INVALID_PARAMETER;
            break;

    }

    return( retCode );
}

BERR_Code BBCP_OpenChannel(
    BBCP_Handle hDev,                   /* Device handle */
    BBCP_ChannelHandle *phChn,          /* [output] Returns channel handle */
    unsigned int channelNo,             /* Channel number to open */
    const BBCP_ChannelSettings *pChnDefSettings /* Channel default setting */
    )
{
    BERR_Code           retCode = BERR_SUCCESS;
    BBCP_ChannelHandle  hChn;

    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( (channelNo==0) || (channelNo==1) );

    hChn = NULL;

    if( channelNo < hDev->maxChnNo )
    {
        if( hDev->hBcpChn[channelNo] == NULL )
        {
            /* Alloc memory from the system heap */
            hChn = (BBCP_ChannelHandle) BKNI_Malloc( sizeof( BBCP_P_ChannelHandle ) );
            if( hChn == NULL )
            {
                *phChn = NULL;
                retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                BDBG_ERR(("BBCP_OpenChannel: BKNI_malloc() failed\n"));
                goto done;
            }

            hChn->magicId   = DEV_MAGIC_ID;
            hChn->hBcp      = hDev;
            hChn->chnNo     = channelNo;
            hChn->pInterruptEventUserCallback = NULL;
            hDev->hBcpChn[channelNo] = hChn;

            BBCP_CHK_RETCODE( retCode, BBCP_SetFilter(hChn, (BBCP_Filter_Settings *)&(pChnDefSettings->filter)) );
            BBCP_CHK_RETCODE( retCode, BBCP_SetEdge(hChn, (BBCP_Edge_Settings *)&(pChnDefSettings->edge)) );

            *phChn = hChn;
        }
        else
        {
            retCode = BBCP_ERR_NOTAVAIL_CHN_NO;
        }
    }
    else
    {
        retCode = BERR_INVALID_PARAMETER;
    }

done:
    if( retCode != BERR_SUCCESS )
    {
        if( hChn != NULL )
        {
            BKNI_Free( hChn );
            hDev->hBcpChn[channelNo] = NULL;
            *phChn = NULL;
        }
    }
    return( retCode );
}

BERR_Code BBCP_CloseChannel(
    BBCP_ChannelHandle hChn         /* Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BBCP_Handle hDev;
    unsigned int chnNo;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hDev = hChn->hBcp;

    /* Disable BICAP */
    BBCP_SetBicapEnable(hChn, false);

    chnNo = hChn->chnNo;
    BKNI_Free( hChn );
    hDev->hBcpChn[chnNo] = NULL;

    return( retCode );
}

BERR_Code BBCP_GetDevice(
    BBCP_ChannelHandle hChn,            /* Device channel handle */
    BBCP_Handle *phDev                  /* [output] Returns Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;


    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    *phDev = hChn->hBcp;

    return( retCode );
}

#ifdef SINGLE_EVENT
BERR_Code BBCP_GetEventHandle(
    BBCP_Handle hDev,                   /* Device handle */
    BKNI_EventHandle *phEvent           /* [output] Returns event handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    *phEvent = hDev->hEvent;
    return( retCode );
}
#endif

bool BBCP_IntMode(
    BBCP_Handle hDev                    /* Device handle */
    )
{
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    return ( hDev->intMode ? true : false );
}

BERR_Code BBCP_GetEdge( 
    BBCP_ChannelHandle  hChn,           /* [in] Device channel handle */
    BBCP_Edge_Settings  *pEdge          /* [out] edge information */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BBCP_Handle hDev;
    uint32_t lval;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( (hChn->chnNo==0) || (hChn->chnNo==1) );

    hDev = hChn->hBcp;

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4);
    pEdge->tout_clk_div = (lval & BCHP_BICAP_CONTROL0_tout_clk_div_MASK) >> BCHP_BICAP_CONTROL0_tout_clk_div_SHIFT;
    pEdge->sys_clk_div = (lval & BCHP_BICAP_CONTROL0_sys_clk_div_MASK) >> BCHP_BICAP_CONTROL0_sys_clk_div_SHIFT;
    pEdge->cnt_mode = (lval & BCHP_BICAP_CONTROL0_cnt_mode_MASK) >> BCHP_BICAP_CONTROL0_cnt_mode_SHIFT;
    pEdge->pedgedet_en = (lval & BCHP_BICAP_CONTROL0_pedgedet_en_MASK) >> BCHP_BICAP_CONTROL0_pedgedet_en_SHIFT;
    pEdge->nedgedet_en = (lval & BCHP_BICAP_CONTROL0_nedgedet_en_MASK) >> BCHP_BICAP_CONTROL0_nedgedet_en_SHIFT;
    pEdge->insig_inv = (lval & BCHP_BICAP_CONTROL0_insig_inv_MASK) >> BCHP_BICAP_CONTROL0_insig_inv_SHIFT;
    pEdge->bicap_en = (lval & BCHP_BICAP_CONTROL0_bicap_en_MASK) >> BCHP_BICAP_CONTROL0_bicap_en_SHIFT;

    return( retCode );
}

BERR_Code BBCP_SetEdge( 
    BBCP_ChannelHandle  hChn,           /* [in] Device channel handle */
    BBCP_Edge_Settings  *pEdge          /* [out] edge information */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BBCP_Handle hDev;
    uint32_t lval;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( (hChn->chnNo==0) || (hChn->chnNo==1) );

    if (pEdge->tout_clk_div & ~(BCHP_BICAP_CONTROL0_tout_clk_div_MASK >> BCHP_BICAP_CONTROL0_tout_clk_div_SHIFT)) return ( BERR_INVALID_PARAMETER );
    if (pEdge->tout_clk_div & ~(BCHP_BICAP_CONTROL0_sys_clk_div_MASK >> BCHP_BICAP_CONTROL0_sys_clk_div_SHIFT)) return ( BERR_INVALID_PARAMETER );
    if (pEdge->cnt_mode > 3) return ( BERR_INVALID_PARAMETER );
    if (pEdge->pedgedet_en > 1) return ( BERR_INVALID_PARAMETER );
    if (pEdge->nedgedet_en > 1) return ( BERR_INVALID_PARAMETER );
    if (pEdge->insig_inv > 1) return ( BERR_INVALID_PARAMETER );
    if (pEdge->bicap_en > 1) return ( BERR_INVALID_PARAMETER );

    hDev = hChn->hBcp;

    lval = (pEdge->tout_clk_div << BCHP_BICAP_CONTROL0_tout_clk_div_SHIFT) & BCHP_BICAP_CONTROL0_tout_clk_div_MASK;
    lval |= ((pEdge->sys_clk_div << BCHP_BICAP_CONTROL0_sys_clk_div_SHIFT) & BCHP_BICAP_CONTROL0_sys_clk_div_MASK);
    lval |= ((pEdge->cnt_mode << BCHP_BICAP_CONTROL0_cnt_mode_SHIFT) & BCHP_BICAP_CONTROL0_cnt_mode_MASK);
    lval |= ((pEdge->pedgedet_en << BCHP_BICAP_CONTROL0_pedgedet_en_SHIFT) & BCHP_BICAP_CONTROL0_pedgedet_en_MASK);
    lval |= ((pEdge->nedgedet_en << BCHP_BICAP_CONTROL0_nedgedet_en_SHIFT) & BCHP_BICAP_CONTROL0_nedgedet_en_MASK);
    lval |= ((pEdge->insig_inv << BCHP_BICAP_CONTROL0_insig_inv_SHIFT) & BCHP_BICAP_CONTROL0_insig_inv_MASK);
    lval |= ((pEdge->bicap_en << BCHP_BICAP_CONTROL0_bicap_en_SHIFT) & BCHP_BICAP_CONTROL0_bicap_en_MASK);
    BREG_Write32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4, lval);

    return( retCode );
}

BERR_Code BBCP_SetToutClkDiv( 
    BBCP_ChannelHandle  hChn,           /* [in] Device channel handle */
    uint32_t            val             /* [in] value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;
    BBCP_Handle hDev;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( (hChn->chnNo==0) || (hChn->chnNo==1) );

    if ( val & ~(BCHP_BICAP_CONTROL0_tout_clk_div_MASK >> BCHP_BICAP_CONTROL0_tout_clk_div_SHIFT)) return ( BERR_INVALID_PARAMETER );

    hDev = hChn->hBcp;

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4);
    lval &= ~BCHP_BICAP_CONTROL0_tout_clk_div_MASK;
    lval |= ((val << BCHP_BICAP_CONTROL0_tout_clk_div_SHIFT) & BCHP_BICAP_CONTROL0_tout_clk_div_MASK);
    BREG_Write32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4, lval);

    return( retCode );
}

BERR_Code BBCP_SetSysClkDiv( 
    BBCP_ChannelHandle  hChn,           /* [in] Device channel handle */
    uint32_t            val             /* [in] value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;
    BBCP_Handle hDev;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( (hChn->chnNo==0) || (hChn->chnNo==1) );

    if ( val & ~(BCHP_BICAP_CONTROL0_sys_clk_div_MASK >> BCHP_BICAP_CONTROL0_sys_clk_div_SHIFT)) return ( BERR_INVALID_PARAMETER );

    hDev = hChn->hBcp;

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4);
    lval &= ~BCHP_BICAP_CONTROL0_sys_clk_div_MASK;
    lval |= ((val << BCHP_BICAP_CONTROL0_sys_clk_div_SHIFT) & BCHP_BICAP_CONTROL0_sys_clk_div_MASK);
    BREG_Write32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4, lval);

    return( retCode );
}

BERR_Code BBCP_GetCntMode( 
    BBCP_ChannelHandle  hChn,           /* [in] Device channel handle */
    uint32_t            *mode           /* [out] mode */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;
    BBCP_Handle hDev;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( (hChn->chnNo==0) || (hChn->chnNo==1) );

    hDev = hChn->hBcp;

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4);
    *mode = (lval & BCHP_BICAP_CONTROL0_cnt_mode_MASK) >> BCHP_BICAP_CONTROL0_cnt_mode_SHIFT;

    return( retCode );
}

BERR_Code BBCP_SetCntMode( 
    BBCP_ChannelHandle  hChn,           /* [in] Device channel handle */
    uint32_t            mode            /* [in] mode */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;
    BBCP_Handle hDev;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( (hChn->chnNo==0) || (hChn->chnNo==1) );

    hDev = hChn->hBcp;

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4);
    lval |= ((mode << BCHP_BICAP_CONTROL0_cnt_mode_SHIFT) & BCHP_BICAP_CONTROL0_cnt_mode_MASK);
    BREG_Write32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4, lval);

    return( retCode );
}

BERR_Code BBCP_EnableEdge(
    BBCP_ChannelHandle  hChn,           /* Device channel handle */
    BBCP_EdgeConfig     edge            /* edge config */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;
    BBCP_Handle hDev;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( (hChn->chnNo==0) || (hChn->chnNo==1) );

    hDev = hChn->hBcp;

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4);
    if (edge == BBCP_EdgeConfig_ePositive || edge == BBCP_EdgeConfig_eBoth)
        lval |= BCHP_BICAP_CONTROL0_pedgedet_en_MASK;
    if (edge == BBCP_EdgeConfig_eNegative || edge == BBCP_EdgeConfig_eBoth)
        lval |= BCHP_BICAP_CONTROL0_nedgedet_en_MASK;
    BREG_Write32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4, lval);

    return( retCode );
}

BERR_Code BBCP_DisableEdge(
    BBCP_ChannelHandle  hChn,           /* Device channel handle */
    BBCP_EdgeConfig     edge            /* edge config */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;
    BBCP_Handle hDev;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( (hChn->chnNo==0) || (hChn->chnNo==1) );

    hDev = hChn->hBcp;

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4);
    if (edge == BBCP_EdgeConfig_ePositive || edge == BBCP_EdgeConfig_eBoth)
        lval &= ~BCHP_BICAP_CONTROL0_pedgedet_en_MASK;
    if (edge == BBCP_EdgeConfig_eNegative || edge == BBCP_EdgeConfig_eBoth)
        lval &= ~BCHP_BICAP_CONTROL0_nedgedet_en_MASK;
    BREG_Write32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4, lval);

    return( retCode );
}

BERR_Code BBCP_GetRisingEdge( 
    BBCP_ChannelHandle  hChn,           /* [in] Device channel handle */
    bool    *edge                       /* [out] edge value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BBCP_Handle hDev;
    uint32_t lval;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( (hChn->chnNo==0) || (hChn->chnNo==1) );

    hDev = hChn->hBcp;

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4);
    *edge = lval & BCHP_BICAP_CONTROL0_pedgedet_en_MASK;

    return( retCode );
}

BERR_Code BBCP_GetFallingEdge( 
    BBCP_ChannelHandle  hChn,           /* [in] Device channel handle */
    bool    *edge                       /* [out] edge value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BBCP_Handle hDev;
    uint32_t lval;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( (hChn->chnNo==0) || (hChn->chnNo==1) );

    hDev = hChn->hBcp;

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4);
    *edge = lval & BCHP_BICAP_CONTROL0_nedgedet_en_MASK;

    return( retCode );
}

BERR_Code BBCP_GetInvertInput( 
    BBCP_ChannelHandle  hChn,           /* [in] Device channel handle */
    bool                *invert         /* [out] invert value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BBCP_Handle hDev;
    uint32_t lval;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( (hChn->chnNo==0) || (hChn->chnNo==1) );

    hDev = hChn->hBcp;

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4);
    *invert = (lval & BCHP_BICAP_CONTROL0_insig_inv_MASK) >> BCHP_BICAP_CONTROL0_insig_inv_SHIFT;

    return( retCode );
}

BERR_Code BBCP_SetInvertInput( 
    BBCP_ChannelHandle  hChn,           /* [in] Device channel handle */
    bool                invert          /* [in] invert value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BBCP_Handle hDev;
    uint32_t lval;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( (hChn->chnNo==0) || (hChn->chnNo==1) );

    hDev = hChn->hBcp;

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4);
    if (invert)
        lval |= BCHP_BICAP_CONTROL0_insig_inv_MASK;
    else
        lval &= ~BCHP_BICAP_CONTROL0_insig_inv_MASK;
    BREG_Write32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4, lval);

    return( retCode );
}

BERR_Code BBCP_GetBicapEnable( 
    BBCP_ChannelHandle  hChn,           /* [in] Device channel handle */
    bool                *enable         /* [in] enable */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BBCP_Handle hDev;
    uint32_t lval;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( (hChn->chnNo==0) || (hChn->chnNo==1) );

    hDev = hChn->hBcp;

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4);
    *enable = (lval & BCHP_BICAP_CONTROL0_bicap_en_MASK) >> BCHP_BICAP_CONTROL0_bicap_en_SHIFT;

    return( retCode );
}

BERR_Code BBCP_SetBicapEnable( 
    BBCP_ChannelHandle  hChn,           /* [in] Device channel handle */
    bool    enable                      /* [in] enable */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BBCP_Handle hDev;
    uint32_t lval;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( (hChn->chnNo==0) || (hChn->chnNo==1) );

    hDev = hChn->hBcp;

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4);
    if (enable)
        lval |= BCHP_BICAP_CONTROL0_bicap_en_MASK;
    else
        lval &= ~BCHP_BICAP_CONTROL0_bicap_en_MASK;
    BREG_Write32(hDev->hRegister, BCHP_BICAP_CONTROL0+hChn->chnNo*4, lval);

    return( retCode );
}

void BBCP_GetFilter( 
    BBCP_ChannelHandle  hChn,           /* [in] Device channel handle */
    BBCP_Filter_Settings    *filter     /* [in] filter settings */
    )
{
    BBCP_Handle hDev;
    uint32_t lval;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( (hChn->chnNo==0) || (hChn->chnNo==1) );

    hDev = hChn->hBcp;

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_FILTER0+hChn->chnNo*4);

    #ifdef BCHP_BICAP_FILTER0_bicap_clk_sel_filter_MASK
        filter->bypass = (lval & BCHP_BICAP_FILTER0_filter_bypass_MASK) >> BCHP_BICAP_FILTER0_filter_bypass_SHIFT;
        filter->clk_sel = (lval & BCHP_BICAP_FILTER0_filter_clk_sel_MASK) >> BCHP_BICAP_FILTER0_filter_clk_sel_SHIFT;
        filter->val = (lval & BCHP_BICAP_FILTER0_bicap_clk_sel_filter_MASK) >> BCHP_BICAP_FILTER0_bicap_clk_sel_filter_SHIFT;
    #else
        filter->val = (lval & BCHP_BICAP_FILTER0_filter_MASK) >> BCHP_BICAP_FILTER0_filter_SHIFT;
    #endif

    return;
}

BERR_Code BBCP_SetFilter( 
    BBCP_ChannelHandle  hChn,           /* [in] Device channel handle */
    BBCP_Filter_Settings    *filter     /* [in] filter settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BBCP_Handle hDev;
    uint32_t lval;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( (hChn->chnNo==0) || (hChn->chnNo==1) );

    #ifdef BCHP_BICAP_FILTER0_bicap_clk_sel_filter_MASK
        if (filter->bypass > 1) return ( BERR_INVALID_PARAMETER );
        if (filter->clk_sel > 1) return ( BERR_INVALID_PARAMETER );
        if (filter->val & ~BCHP_BICAP_FILTER0_bicap_clk_sel_filter_MASK) return ( BERR_INVALID_PARAMETER );
    #else
        if (filter->val & ~BCHP_BICAP_FILTER0_filter_MASK) return ( BERR_INVALID_PARAMETER );
    #endif

    hDev = hChn->hBcp;
    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_FILTER0+hChn->chnNo*4);

    #ifdef BCHP_BICAP_FILTER0_bicap_clk_sel_filter_MASK
        lval &= ~(BCHP_BICAP_FILTER0_filter_bypass_MASK | BCHP_BICAP_FILTER0_filter_clk_sel_MASK | BCHP_BICAP_FILTER0_bicap_clk_sel_filter_MASK);
        lval |= ((filter->bypass << BCHP_BICAP_FILTER0_filter_bypass_SHIFT) & BCHP_BICAP_FILTER0_filter_bypass_MASK);
        lval |= ((filter->clk_sel << BCHP_BICAP_FILTER0_filter_clk_sel_SHIFT) & BCHP_BICAP_FILTER0_filter_clk_sel_MASK);
        lval |= ((filter->val << BCHP_BICAP_FILTER0_bicap_clk_sel_filter_SHIFT) & BCHP_BICAP_FILTER0_bicap_clk_sel_filter_MASK);
    #else
        lval &= ~(BCHP_BICAP_FILTER0_filter_MASK);
        lval |= ((filter->val << BCHP_BICAP_FILTER0_filter_SHIFT) & BCHP_BICAP_FILTER0_filter_MASK);
    #endif

    BREG_Write32(hDev->hRegister, BCHP_BICAP_FILTER0+hChn->chnNo*4, lval);

    return( retCode );
}

BERR_Code BBCP_GetTimeout( 
    BBCP_Handle     hDev,               /* [in] Device handle */
    uint32_t which,
    BBCP_Timeout_Settings   *timeout    /* {out} timeout settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;

    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    if ( which > 3 ) return ( BERR_INVALID_PARAMETER );

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_TIMEOUT0+which*4);
    timeout->input_sel = (lval & BCHP_BICAP_TIMEOUT0_input_sel_MASK) >> BCHP_BICAP_TIMEOUT0_input_sel_SHIFT;
    timeout->edge_sel = (lval & BCHP_BICAP_TIMEOUT0_edge_sel_MASK) >> BCHP_BICAP_TIMEOUT0_edge_sel_SHIFT;
    timeout->clk_sel = (lval & BCHP_BICAP_TIMEOUT0_clk_sel_MASK) >> BCHP_BICAP_TIMEOUT0_clk_sel_SHIFT;
    timeout->tout = (lval & BCHP_BICAP_TIMEOUT0_tout_MASK) >> BCHP_BICAP_TIMEOUT0_tout_SHIFT;

    return( retCode );
}

BERR_Code BBCP_SetTimeout( 
    BBCP_Handle     hDev,               /* [in] Device handle */
    uint32_t which,
    BBCP_Timeout_Settings   *timeout    /* {in} timeout settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;

    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    if ( which > 3 ) return ( BERR_INVALID_PARAMETER );
    if ( timeout->input_sel > 1 ) return ( BERR_INVALID_PARAMETER );
    if ( timeout->edge_sel > 3 ) return ( BERR_INVALID_PARAMETER );
    if ( timeout->tout & ~BCHP_BICAP_TIMEOUT0_tout_MASK ) return ( BERR_INVALID_PARAMETER );

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_TIMEOUT0+which*4);
    lval &= ~(BCHP_BICAP_TIMEOUT0_input_sel_MASK | BCHP_BICAP_TIMEOUT0_edge_sel_MASK | BCHP_BICAP_TIMEOUT0_clk_sel_MASK | BCHP_BICAP_TIMEOUT0_tout_MASK);
    lval |= ((timeout->input_sel << BCHP_BICAP_TIMEOUT0_input_sel_SHIFT) & BCHP_BICAP_TIMEOUT0_input_sel_MASK);
    lval |= ((timeout->edge_sel << BCHP_BICAP_TIMEOUT0_edge_sel_SHIFT) & BCHP_BICAP_TIMEOUT0_edge_sel_MASK);
    lval |= ((timeout->clk_sel << BCHP_BICAP_TIMEOUT0_clk_sel_SHIFT) & BCHP_BICAP_TIMEOUT0_clk_sel_MASK);
    lval |= ((timeout->tout << BCHP_BICAP_TIMEOUT0_tout_SHIFT) & BCHP_BICAP_TIMEOUT0_tout_MASK);
    BREG_Write32(hDev->hRegister, BCHP_BICAP_TIMEOUT0+which*4, lval);

    return( retCode );
}

BERR_Code BBCP_GetFifoInactTimeout( 
    BBCP_Handle         hDev,           /* [in] Device handle */
    BBCP_FifoInactTimeout_Settings *inact
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;

    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_FIFO_INACT_TIMEOUT);
    inact->bicap_clk_sel = (lval & BCHP_BICAP_FIFO_INACT_TIMEOUT_bicap_clk_sel_MASK) >> BCHP_BICAP_FIFO_INACT_TIMEOUT_bicap_clk_sel_SHIFT;
    inact->event_sel = (lval & BCHP_BICAP_FIFO_INACT_TIMEOUT_event_sel_MASK) >> BCHP_BICAP_FIFO_INACT_TIMEOUT_event_sel_SHIFT;
    inact->clk_sel = (lval & BCHP_BICAP_FIFO_INACT_TIMEOUT_clk_sel_MASK) >> BCHP_BICAP_FIFO_INACT_TIMEOUT_clk_sel_SHIFT;
    inact->tout = (lval & BCHP_BICAP_FIFO_INACT_TIMEOUT_tout_MASK) >> BCHP_BICAP_FIFO_INACT_TIMEOUT_tout_SHIFT;

    return( retCode );
}

BERR_Code BBCP_SetFifoInactTimeout( 
    BBCP_Handle         hDev,           /* [in] Device handle */
    BBCP_FifoInactTimeout_Settings *inact
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;

    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    if ( inact->bicap_clk_sel > 1 ) return ( BERR_INVALID_PARAMETER );
    if ( inact->event_sel > 3 ) return ( BERR_INVALID_PARAMETER );
    if ( inact->clk_sel > 1 ) return ( BERR_INVALID_PARAMETER );
    if ( inact->tout & ~BCHP_BICAP_FIFO_INACT_TIMEOUT_tout_MASK ) return ( BERR_INVALID_PARAMETER );

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_FIFO_INACT_TIMEOUT);
    lval &= ~(BCHP_BICAP_FIFO_INACT_TIMEOUT_bicap_clk_sel_MASK | BCHP_BICAP_FIFO_INACT_TIMEOUT_event_sel_MASK | BCHP_BICAP_FIFO_INACT_TIMEOUT_clk_sel_MASK | BCHP_BICAP_FIFO_INACT_TIMEOUT_tout_MASK);

    lval |= ((inact->bicap_clk_sel << BCHP_BICAP_FIFO_INACT_TIMEOUT_bicap_clk_sel_SHIFT) & BCHP_BICAP_FIFO_INACT_TIMEOUT_bicap_clk_sel_MASK);
    lval |= ((inact->event_sel << BCHP_BICAP_FIFO_INACT_TIMEOUT_event_sel_SHIFT) & BCHP_BICAP_FIFO_INACT_TIMEOUT_event_sel_MASK);
    lval |= ((inact->clk_sel << BCHP_BICAP_FIFO_INACT_TIMEOUT_clk_sel_SHIFT) & BCHP_BICAP_FIFO_INACT_TIMEOUT_clk_sel_MASK);
    lval |= ((inact->tout << BCHP_BICAP_FIFO_INACT_TIMEOUT_tout_SHIFT) & BCHP_BICAP_FIFO_INACT_TIMEOUT_tout_MASK);
    BREG_Write32(hDev->hRegister, BCHP_BICAP_FIFO_INACT_TIMEOUT, lval);

    return( retCode );
}

BERR_Code BBCP_GetFifoData( 
    BBCP_Handle         hDev,           /* [in] Device handle */
    uint32_t            *count,         /* [out] number of elements */
    uint32_t            *data           /* [out] pointer to data */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    #ifdef BCHP_BICAP_STATUS_fifo_count_MASK
        uint32_t i;
    #endif

    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    #ifdef BCHP_BICAP_STATUS_fifo_count_MASK
        *count = (BREG_Read32(hDev->hRegister, BCHP_BICAP_STATUS) & BCHP_BICAP_STATUS_fifo_count_MASK) >> BCHP_BICAP_STATUS_fifo_count_SHIFT;
        for (i=0; i<*count; i++)
            *(data+i) = BREG_Read32(hDev->hRegister, BCHP_BICAP_FIFO_DATA);
    #else
        *count = 0;
        while (!(BREG_Read32(hDev->hRegister, BCHP_BICAP_STATUS) & BCHP_BICAP_STATUS_fifo_empty_MASK)) {
            *(data+*count) = BREG_Read32(hDev->hRegister, BCHP_BICAP_FIFO_DATA);
            (*count)+=1;
        }
    #endif

    return( retCode );
}

BERR_Code BBCP_GetOneFifoData( 
    BBCP_Handle         hDev,           /* [in] Device handle */
    uint32_t            *count,         /* [out] number of elements */
    uint32_t            *data           /* [out] pointer to data */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    #ifdef BCHP_BICAP_STATUS_fifo_count_MASK
        *count = (BREG_Read32(hDev->hRegister, BCHP_BICAP_STATUS) & BCHP_BICAP_STATUS_fifo_count_MASK) >> BCHP_BICAP_STATUS_fifo_count_SHIFT;
        if (*count)
            *data = BREG_Read32(hDev->hRegister, BCHP_BICAP_FIFO_DATA);
    #else
        *count = 0;
        if (!(BREG_Read32(hDev->hRegister, BCHP_BICAP_STATUS) & BCHP_BICAP_STATUS_fifo_empty_MASK))
        {
            *data = BREG_Read32(hDev->hRegister, BCHP_BICAP_FIFO_DATA);
            *count = 1;
        }
    #endif

    return( retCode );
}

BERR_Code BBCP_GetFifo( 
    BBCP_Handle         hDev,           /* [in] Device handle */
    uint32_t            *fifo_depth,    /* [out] size of the fifo */
    uint32_t            *trig_lvl       /* [out] current value of the trigger level */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    
    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_FIFO_CONTROL);
    *fifo_depth = (lval & BCHP_BICAP_FIFO_CONTROL_fifo_depth_MASK) >> BCHP_BICAP_FIFO_CONTROL_fifo_depth_SHIFT;
    *trig_lvl = (lval & BCHP_BICAP_FIFO_CONTROL_trig_lvl_MASK) >> BCHP_BICAP_FIFO_CONTROL_trig_lvl_SHIFT;
    return( retCode );
}

BERR_Code BBCP_SetFifo( 
    BBCP_Handle         hDev,           /* [in] Device handle */
    uint32_t            trig_lvl,       /* [in] when fifo reaches this level, event is generated */
    uint32_t            reset           /* [in] reset the fifo */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    
    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_FIFO_CONTROL);
    lval &= ~(BCHP_BICAP_FIFO_CONTROL_trig_lvl_MASK | BCHP_BICAP_FIFO_CONTROL_reset_MASK);
    lval |= ((trig_lvl << BCHP_BICAP_FIFO_CONTROL_trig_lvl_SHIFT) & BCHP_BICAP_FIFO_CONTROL_trig_lvl_MASK);
    if (reset) lval |= BCHP_BICAP_FIFO_CONTROL_reset_MASK;
    BREG_Write32(hDev->hRegister, BCHP_BICAP_FIFO_CONTROL, lval);
    return( retCode );
}

BERR_Code BBCP_SetFifoTrigLevel( 
    BBCP_Handle         hDev,           /* [in] Device handle */
    uint16_t            level           /* [in] trigger level */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;

    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_FIFO_CONTROL);
    lval &= ~BCHP_BICAP_FIFO_CONTROL_trig_lvl_MASK;
    lval |= ((level << BCHP_BICAP_FIFO_CONTROL_trig_lvl_SHIFT) & BCHP_BICAP_FIFO_CONTROL_trig_lvl_MASK);
    BREG_Write32(hDev->hRegister, BCHP_BICAP_FIFO_CONTROL, lval);
    return ( retCode );
}

BERR_Code BBCP_ResetFifo( 
    BBCP_ChannelHandle  hChn            /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BBCP_Handle hDev;
    uint32_t lval;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hDev = hChn->hBcp;

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_FIFO_CONTROL);
    lval |= BCHP_BICAP_FIFO_CONTROL_reset_MASK;
    BREG_Write32(hDev->hRegister, BCHP_BICAP_FIFO_CONTROL, lval);
    return ( retCode );
}

#if 0
uint16_t BBCP_GetFifoCount( 
    BBCP_Handle hDev            /* [in] Device handle */
    )
{
    uint32_t lval;

    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    lval = BREG_Read32(hDev->hRegister, BCHP_BICAP_STATUS);
    return ((lval & BCHP_BICAP_STATUS_fifo_count_MASK) >> BCHP_BICAP_STATUS_fifo_count_SHIFT);
}
#endif

void BBCP_EnableRC6(
    BBCP_ChannelHandle  hChn,           /* Device channel handle */
    BBCP_Callback pCallback             /* Pointer to completion callback. */
    )
{
    BBCP_Handle hDev;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    hChn->handleRC6 = 1;
    hChn->pInterruptEventUserCallback = pCallback;
    hDev = hChn->hBcp;
    hDev->uiRCPinMask |= (1<<hChn->chnNo);
}

void BBCP_DisableRC6(
    BBCP_ChannelHandle  hChn            /* Device channel handle */
    )
{
    BBCP_Handle hDev;

    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    hChn->handleRC6 = 0;

    hDev = hChn->hBcp;
    hDev->uiRCPinMask &= ~(1<<hChn->chnNo);
}

void BBCP_ResetIntCount(
    BBCP_Handle     hDev            /* Device handle */
    )
{
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    hDev->isrCount = 0;
}

void BBCP_GetIntCount(
    BBCP_Handle     hDev,           /* Device handle */
    uint32_t            *data
    )
{
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    *data = hDev->isrCount;
}

/*******************************************************************************
*
*   Private Module Functions
*
*******************************************************************************/

static void BBCP_P_HandleFifoOfInterrupt_Isr
(
    void *pParam1,                      /* Device channel handle */
    int parm2                           /* not used */
)
{
    BBCP_Handle         hDev;
    hDev = (BBCP_Handle) pParam1;
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BSTD_UNUSED( parm2 );

    #ifdef SINGLE_EVENT
        BKNI_SetEvent( hDev->hEvent );
    #endif
}

static void BBCP_P_HandleFifoIrqInterrupt_Isr
(
    void *pParam1,                      /* Device channel handle */
    int parm2                           /* not used */
)
{
    BBCP_Handle         hDev;
    hDev = (BBCP_Handle) pParam1;
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BSTD_UNUSED( parm2 );

    #ifdef SINGLE_EVENT
        BKNI_SetEvent( hDev->hEvent );
    #endif
}

static void BBCP_P_HandleCpuTimeoutIrqInterrupt_Isr
(
    void *pParam1,                      /* Device channel handle */
    int parm2                           /* not used */
)
{
    BBCP_Handle         hDev;
    hDev = (BBCP_Handle) pParam1;
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BSTD_UNUSED( parm2 );

    #ifdef SINGLE_EVENT
        BKNI_SetEvent( hDev->hEvent );
    #endif
}

static void BBCP_P_HandleEdgeTimeoutIrqInterrupt_Isr
(
    void *pParam1,                      /* Device channel handle */
    int parm2                           /* not used */
)
{
    BBCP_Handle         hDev;
    hDev = (BBCP_Handle) pParam1;
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BSTD_UNUSED( parm2 );

    #ifdef SINGLE_EVENT
        BKNI_SetEvent( hDev->hEvent );
    #endif
}

