/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/


#include "bstd.h"
#include "bxpt.h"
#include "bxpt_interrupt.h"
#include "bkni.h"
#include "bxpt_priv.h"

BDBG_MODULE( xpt_interrupt );

#if BXPT_HAS_MESG_L2

#include "bchp_xpt_msg_buf_dat_rdy_cpu_intr_aggregator.h"
#include "bchp_xpt_msg_buf_ovfl_cpu_intr_aggregator.h"
#include "bchp_int_id_xpt_msg_dat_err_intr_l2.h"
#include "bchp_xpt_msg_buf_dat_rdy_intr_00_31_l2.h"
#include "bchp_xpt_msg_buf_dat_rdy_intr_32_63_l2.h"
#include "bchp_xpt_msg_buf_ovfl_intr_00_31_l2.h"
#include "bchp_xpt_msg_buf_ovfl_intr_32_63_l2.h"

void BXPT_Interrupt_P_Init(
    BXPT_Handle hXpt                /* [in] Handle for this transport */
    )
{
    unsigned ii;
    uint32_t masks;

    for( ii = 0; ii < BXPT_NUM_MESG_BUFFERS; ii++ )
    {
        hXpt->MesgIntrCallbacks[ ii ] = hXpt->OverflowIntrCallbacks[ ii ] = NULL;
    }

    masks = BCHP_XPT_MSG_BUF_DAT_RDY_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MSG_BUF_DAT_RDY_INTR_00_31_MASK |
        BCHP_XPT_MSG_BUF_DAT_RDY_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MSG_BUF_DAT_RDY_INTR_32_63_MASK |
        BCHP_XPT_MSG_BUF_DAT_RDY_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MSG_BUF_DAT_RDY_INTR_64_95_MASK |
        BCHP_XPT_MSG_BUF_DAT_RDY_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MSG_BUF_DAT_RDY_INTR_96_127_MASK;

#if BXPT_NUM_MESG_BUFFERS > 128
    masks |= BCHP_XPT_MSG_BUF_DAT_RDY_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MSG_BUF_DAT_RDY_INTR_128_159_MASK |
        BCHP_XPT_MSG_BUF_DAT_RDY_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MSG_BUF_DAT_RDY_INTR_160_191_MASK |
        BCHP_XPT_MSG_BUF_DAT_RDY_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MSG_BUF_DAT_RDY_INTR_192_223_MASK |
        BCHP_XPT_MSG_BUF_DAT_RDY_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MSG_BUF_DAT_RDY_INTR_224_255_MASK;
#endif
    BREG_Write32( hXpt->hRegister, BCHP_XPT_MSG_BUF_DAT_RDY_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR, masks );

    masks = BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_00_31_MASK |
        BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_32_63_MASK |
        BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_64_95_MASK |
        BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_96_127_MASK;

#if BXPT_NUM_MESG_BUFFERS > 128
    masks |= BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_128_159_MASK |
        BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_160_191_MASK |
        BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_192_223_MASK |
        BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_224_255_MASK;
#endif
    BREG_Write32( hXpt->hRegister, BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_CLEAR, masks );
}

void BXPT_Interrupt_P_Shutdown(
    BXPT_Handle hXpt                /* [in] Handle for this transport */
    )
{
    unsigned ii;

    for( ii = 0; ii < BXPT_NUM_MESG_BUFFERS; ii++ )
    {
        if( hXpt->MesgIntrCallbacks[ ii ] )
        {
            BINT_DisableCallback( hXpt->MesgIntrCallbacks[ ii ] );
            BINT_DestroyCallback( hXpt->MesgIntrCallbacks[ ii ] );
            hXpt->MesgIntrCallbacks[ ii ] = NULL;
        }

        if( hXpt->OverflowIntrCallbacks[ ii ] )
        {
            BINT_DisableCallback( hXpt->OverflowIntrCallbacks[ ii ] );
            BINT_DestroyCallback( hXpt->OverflowIntrCallbacks[ ii ] );
            hXpt->OverflowIntrCallbacks[ ii ] = NULL;
        }
    }
}

#define BXPT_P_STATUS_REG_STEP ( BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_STATUS - BCHP_XPT_MSG_BUF_DAT_RDY_INTR_00_31_L2_W0_CPU_STATUS )

static BINT_Id GetMesgReadyIntId(
    int MessageBufferNum,       /* [in] Which message buffer to watch. */
    uint32_t BaseAddr
    )
{
    uint32_t StatusAddr, StatusShift;

    StatusAddr = BaseAddr + ( MessageBufferNum / 32 ) * BXPT_P_STATUS_REG_STEP;
    StatusShift = ( MessageBufferNum % 32 );
    return BCHP_INT_ID_CREATE( StatusAddr, StatusShift );
}

BERR_Code BXPT_Interrupt_EnableMessageInt(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int MessageBufferNum,       /* [in] Which message buffer to watch. */
    BINT_CallbackFunc Callback,     /* [in] Handler for this interrupt. */
    void *Parm1,                    /* [in] First arg to be passed to callback */
    int Parm2                       /* [in] Second arg to be passed to callback */
    )
{
    BERR_Code rc = BERR_SUCCESS;

    if( !hXpt->MesgIntrCallbacks[ MessageBufferNum ] )
    {
        rc = BINT_CreateCallback( &hXpt->MesgIntrCallbacks[ MessageBufferNum ], hXpt->hInt,
            GetMesgReadyIntId( MessageBufferNum, BCHP_XPT_MSG_BUF_DAT_RDY_INTR_00_31_L2_W0_CPU_STATUS ),
            Callback, Parm1, Parm2 );
        if( rc )
        {
            rc = BERR_TRACE( rc );
            goto Done;
        }
    }

    rc = BINT_EnableCallback( hXpt->MesgIntrCallbacks[ MessageBufferNum ] );
    Done:
    return rc;
}


BERR_Code BXPT_Interrupt_DisableMessageInt(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int MessageBufferNum        /* [in] Message interrupt to disable. */
    )
{
    BERR_Code rc = BERR_SUCCESS;

    if( hXpt->MesgIntrCallbacks[ MessageBufferNum ] )
    {
        BINT_DisableCallback( hXpt->MesgIntrCallbacks[ MessageBufferNum ] );
        BINT_DestroyCallback( hXpt->MesgIntrCallbacks[ MessageBufferNum ] );
        hXpt->MesgIntrCallbacks[ MessageBufferNum ] = NULL;
    }

    return rc;
}


BERR_Code BXPT_Interrupt_EnableMessageOverflowInt(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int MessageBufferNum,       /* [in] Which message buffer to watch. */
    BINT_CallbackFunc Callback,     /* [in] Handler for this interrupt. */
    void *Parm1,                    /* [in] First arg to be passed to callback */
    int Parm2                       /* [in] Second arg to be passed to callback */
    )
{
    BERR_Code rc = BERR_SUCCESS;

    if( !hXpt->OverflowIntrCallbacks[ MessageBufferNum ] )
    {
        rc = BINT_CreateCallback( &hXpt->OverflowIntrCallbacks[ MessageBufferNum ], hXpt->hInt,
            GetMesgReadyIntId( MessageBufferNum, BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_STATUS ),
            Callback, Parm1, Parm2 );
        if( rc )
        {
            rc = BERR_TRACE( rc );
            goto Done;
        }
    }

    rc = BINT_EnableCallback( hXpt->OverflowIntrCallbacks[ MessageBufferNum ] );
    Done:
    return rc;
}


BERR_Code BXPT_Interrupt_DisableMessageOverflowInt(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int MessageBufferNum        /* [in] Message interrupt to disable. */
    )
{
    BERR_Code rc = BERR_SUCCESS;

    if( hXpt->OverflowIntrCallbacks[ MessageBufferNum ] )
    {
        BINT_DisableCallback( hXpt->OverflowIntrCallbacks[ MessageBufferNum ] );
        BINT_DestroyCallback( hXpt->OverflowIntrCallbacks[ MessageBufferNum ] );
        hXpt->OverflowIntrCallbacks[ MessageBufferNum ] = NULL;
    }

    return rc;
}

void BXPT_P_Interrupt_MsgSw_isr(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int MessageBufferNum            /* [in] Message Buffer */
    )
{
    BINT_TriggerInterruptByHandle_isr( hXpt->MesgIntrCallbacks[ MessageBufferNum ] );
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_Interrupt_EnableMessageInt_isr(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int MessageBufferNum,       /* [in] Which message buffer to watch. */
    BINT_CallbackFunc Callback,     /* [in] Handler for this interrupt. */
    void *Parm1,                    /* [in] First arg to be passed to callback */
    int Parm2                       /* [in] Second arg to be passed to callback */
    )
{
    BSTD_UNUSED( Callback );
    BSTD_UNUSED( Parm1 );
    BSTD_UNUSED( Parm2 );

    if( !hXpt->MesgIntrCallbacks[ MessageBufferNum ] )
    {
        /* Need to create the callback in a non-ISR context, then we can enable in the ISR */
        BDBG_ERR(( "%s (%s:%d): Not currently support in ISR contexts. Port needed", BSTD_FUNCTION, __FILE__, __LINE__ ));
        return BERR_NOT_SUPPORTED;
    }
    return BINT_EnableCallback_isr( hXpt->MesgIntrCallbacks[ MessageBufferNum ] );
}
#endif

BERR_Code BXPT_Interrupt_DisableMessageInt_isr(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int MessageBufferNum        /* [in] Message interrupt to disable. */
    )
{
    if( !hXpt->MesgIntrCallbacks[ MessageBufferNum ] )
    {
        /* No callback defined. See comments in BXPT_Interrupt_EnableMessageInt_isr() */
        BDBG_ERR(( "%s (%s:%d): Not currently support in ISR contexts. Port needed", BSTD_FUNCTION, __FILE__, __LINE__ ));
        return BERR_NOT_SUPPORTED;
    }
    return BINT_DisableCallback_isr( hXpt->MesgIntrCallbacks[ MessageBufferNum ] );
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_Interrupt_EnableMessageOverflowInt_isr(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int MessageBufferNum,       /* [in] Which message buffer to watch. */
    BINT_CallbackFunc Callback,     /* [in] Handler for this interrupt. */
    void *Parm1,                    /* [in] First arg to be passed to callback */
    int Parm2                       /* [in] Second arg to be passed to callback */
    )
{
    BSTD_UNUSED( Callback );
    BSTD_UNUSED( Parm1 );
    BSTD_UNUSED( Parm2 );

    if( !hXpt->OverflowIntrCallbacks[ MessageBufferNum ] )
    {
        /* Need to create the callback in a non-ISR context, then we can enable in the ISR */
        BDBG_ERR(( "%s (%s:%d): Not currently support in ISR contexts. Port needed", BSTD_FUNCTION, __FILE__, __LINE__ ));
        return BERR_NOT_SUPPORTED;
    }
    return BINT_EnableCallback_isr( hXpt->OverflowIntrCallbacks[ MessageBufferNum ] );
}
#endif

BERR_Code BXPT_Interrupt_DisableMessageOverflowInt_isr(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int MessageBufferNum        /* [in] Message interrupt to disable. */
    )
{
    if( !hXpt->OverflowIntrCallbacks[ MessageBufferNum ] )
    {
        /* No callback defined. See comments in BXPT_Interrupt_EnableMessageInt_isr() */
        BDBG_ERR(( "%s (%s:%d): Not currently support in ISR contexts. Port needed", BSTD_FUNCTION, __FILE__, __LINE__ ));
        return BERR_NOT_SUPPORTED;
    }
    return BINT_DisableCallback_isr( hXpt->OverflowIntrCallbacks[ MessageBufferNum ] );
}

#else

#include "bchp_xpt_msg.h"

#define INTERRUPT_BITS_PER_REGISTER     ( 32 )
#define REGISTER_SIZE                   ( 4 )


BERR_Code BXPT_Interrupt_EnableMessageInt(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int MessageBufferNum,       /* [in] Which message buffer to watch. */
    BINT_CallbackFunc Callback,     /* [in] Handler for this interrupt. */
    void *Parm1,                    /* [in] First arg to be passed to callback */
    int Parm2                       /* [in] Second arg to be passed to callback */
    )
{
    BERR_Code ExitCode;

    BKNI_EnterCriticalSection();
    ExitCode = BXPT_Interrupt_EnableMessageInt_isr( hXpt, MessageBufferNum, Callback, Parm1, Parm2 );
    BKNI_LeaveCriticalSection();

    return ExitCode;
}

BERR_Code BXPT_Interrupt_DisableMessageInt(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int MessageBufferNum        /* [in] Message interrupt to disable. */
    )
{
    BERR_Code ExitCode;

    BKNI_EnterCriticalSection();
    ExitCode = BXPT_Interrupt_DisableMessageInt_isr( hXpt, MessageBufferNum );
    BKNI_LeaveCriticalSection();

    return ExitCode;
}

BERR_Code BXPT_Interrupt_DisableMessageOverflowInt(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int MessageBufferNum        /* [in] Message interrupt to disable. */
    )
{
    BERR_Code ExitCode;

    BKNI_EnterCriticalSection();
    ExitCode = BXPT_Interrupt_DisableMessageOverflowInt_isr( hXpt, MessageBufferNum );
    BKNI_LeaveCriticalSection();

    return ExitCode;
}

BERR_Code BXPT_Interrupt_EnableMessageOverflowInt(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int MessageBufferNum,       /* [in] Which message buffer to watch. */
    BINT_CallbackFunc Callback,     /* [in] Handler for this interrupt. */
    void *Parm1,                    /* [in] First arg to be passed to callback */
    int Parm2                       /* [in] Second arg to be passed to callback */
    )
{
    BERR_Code ExitCode;

    BKNI_EnterCriticalSection();
    ExitCode = BXPT_Interrupt_EnableMessageOverflowInt_isr( hXpt, MessageBufferNum, Callback, Parm1, Parm2 );
    BKNI_LeaveCriticalSection();

    return ExitCode;
}

static void MapBitAndAddress_isrsafe(
    int MessageBufferNum,
    uint32_t InAddress,
    uint32_t *BitNum,
    uint32_t *OutAddress
    );

static void SetBit_isr(
    BXPT_Handle hXpt,                       /* [in] Handle for this transport */
    uint32_t Addr,
    int BitNum
    );

static void ClearBit_isr(
    BXPT_Handle hXpt,                       /* [in] Handle for this transport */
    uint32_t Addr,
    int BitNum
    );


BERR_Code BXPT_Interrupt_EnableMessageInt_isr(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int MessageBufferNum,       /* [in] Which message buffer to watch. */
    BINT_CallbackFunc Callback,     /* [in] Handler for this interrupt. */
    void *Parm1,                    /* [in] First arg to be passed to callback */
    int Parm2                       /* [in] Second arg to be passed to callback */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( Callback );

    if( MessageBufferNum >= BXPT_NUM_MESG_BUFFERS )
    {
        /* Bad interrupt number. Complain. */
        BDBG_ERR(( "MessageBufferNum %lu is out of range!", ( unsigned long ) MessageBufferNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        uint32_t BitNum = 0;
        uint32_t RegAddr = 0;

        /* Load the callback into the jump table. */
        hXpt->MesgIntrCallbacks[ MessageBufferNum ].Callback = Callback;
        hXpt->MesgIntrCallbacks[ MessageBufferNum ].Parm1 = Parm1;
        hXpt->MesgIntrCallbacks[ MessageBufferNum ].Parm2 = Parm2;

        MapBitAndAddress_isrsafe( MessageBufferNum, BCHP_XPT_MSG_BUF_DAT_RDY_INTR_EN_00_31, &BitNum, &RegAddr );
        SetBit_isr( hXpt, RegAddr, BitNum );
    }

    return( ExitCode );
}


BERR_Code BXPT_Interrupt_DisableMessageInt_isr(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int MessageBufferNum        /* [in] Message interrupt to disable. */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    if( MessageBufferNum >= BXPT_NUM_MESG_BUFFERS )
    {
        /* Bad interrupt number. Complain. */
        BDBG_ERR(( "MessageBufferNum %lu is out of range!", ( unsigned long ) MessageBufferNum ));
        ExitCode = BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    else
    {
        uint32_t BitNum = 0;
        uint32_t RegAddr = 0;

        MapBitAndAddress_isrsafe( MessageBufferNum, BCHP_XPT_MSG_BUF_DAT_RDY_INTR_EN_00_31, &BitNum, &RegAddr );
        ClearBit_isr( hXpt, RegAddr, BitNum );

        /* Clear the callback entry the jump table. */
        hXpt->MesgIntrCallbacks[ MessageBufferNum ].Callback = ( BINT_CallbackFunc ) NULL;
    }

    return( ExitCode );
}


BERR_Code BXPT_Interrupt_EnableMessageOverflowInt_isr(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int MessageBufferNum,       /* [in] Which message buffer to watch. */
    BINT_CallbackFunc Callback,     /* [in] Handler for this interrupt. */
    void *Parm1,                    /* [in] First arg to be passed to callback */
    int Parm2                       /* [in] Second arg to be passed to callback */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( Callback );

    if( MessageBufferNum >= BXPT_NUM_MESG_BUFFERS )
    {
        /* Bad interrupt number. Complain. */
        BDBG_ERR(( "MessageBufferNum %lu is out of range!", ( unsigned long ) MessageBufferNum ));
        ExitCode = BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    else
    {
        uint32_t BitNum = 0;
        uint32_t RegAddr = 0;

        /* Load the callback into the jump table. */
        hXpt->OverflowIntrCallbacks[ MessageBufferNum ].Callback = Callback;
        hXpt->OverflowIntrCallbacks[ MessageBufferNum ].Parm1 = Parm1;
        hXpt->OverflowIntrCallbacks[ MessageBufferNum ].Parm2 = Parm2;

        MapBitAndAddress_isrsafe( MessageBufferNum, BCHP_XPT_MSG_BUF_OVFL_INTR_EN_00_31, &BitNum, &RegAddr );
        SetBit_isr( hXpt, RegAddr, BitNum );
    }

    return( ExitCode );
}


BERR_Code BXPT_Interrupt_DisableMessageOverflowInt_isr(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int MessageBufferNum        /* [in] Message interrupt to disable. */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    if( MessageBufferNum >= BXPT_NUM_MESG_BUFFERS )
    {
        /* Bad interrupt number. Complain. */
        BDBG_ERR(( "MessageBufferNum %lu is out of range!", ( unsigned long ) MessageBufferNum ));
        ExitCode = BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    else
    {
        uint32_t BitNum = 0;
        uint32_t RegAddr = 0;

        MapBitAndAddress_isrsafe( MessageBufferNum, BCHP_XPT_MSG_BUF_OVFL_INTR_EN_00_31, &BitNum, &RegAddr );
        ClearBit_isr( hXpt, RegAddr, BitNum );

        MapBitAndAddress_isrsafe( MessageBufferNum, BCHP_XPT_MSG_BUF_OVFL_INTR_00_31, &BitNum, &RegAddr );
        ClearBit_isr( hXpt, RegAddr, BitNum );

        /* Clear the callback entry the jump table. */
        hXpt->OverflowIntrCallbacks[ MessageBufferNum ].Callback = ( BINT_CallbackFunc ) NULL;
    }

    return( ExitCode );
}

void BXPT_P_Interrupt_MsgVector_isr(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int L1Shift         /* [in] Dummy arg. Not used by this interface. */
    )
{
    unsigned i, j;
    uint32_t Status, Mask, StatusAddr, MaskAddr;

    BDBG_ASSERT( hXpt );
    BSTD_UNUSED( L1Shift );

    for( i = 0; i < BXPT_NUM_MESG_BUFFERS; i += INTERRUPT_BITS_PER_REGISTER )
    {
        StatusAddr = BCHP_XPT_MSG_BUF_DAT_RDY_INTR_00_31 + ( REGISTER_SIZE * ( i / INTERRUPT_BITS_PER_REGISTER ));
        Status = BREG_Read32( hXpt->hRegister, StatusAddr );

        MaskAddr = BCHP_XPT_MSG_BUF_DAT_RDY_INTR_EN_00_31 + ( REGISTER_SIZE * ( i / INTERRUPT_BITS_PER_REGISTER ));
        Mask = BREG_Read32( hXpt->hRegister, MaskAddr );

        if( Status & Mask )
        {
            for( j = 0; j < INTERRUPT_BITS_PER_REGISTER; j++ )
            {
                if( ( Status & Mask ) & ( 1ul << j ) )
                {
                    BXPT_P_InterruptCallbackArgs *Cb = &hXpt->MesgIntrCallbacks[ i + j ];
                    if( Cb->Callback != ( BINT_CallbackFunc ) NULL )
                        (*( Cb->Callback )) ( Cb->Parm1, Cb->Parm2 );
                }
            }
        }
    }
}

void BXPT_P_Interrupt_MsgSw_isr(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int MessageBufferNum            /* [in] Message Buffer */
    )
{
    BXPT_P_InterruptCallbackArgs *Cb = &hXpt->MesgIntrCallbacks[ MessageBufferNum ];
    if( Cb->Callback != ( BINT_CallbackFunc ) NULL )
            (*( Cb->Callback )) ( Cb->Parm1, Cb->Parm2 );
}

void BXPT_P_Interrupt_MsgOverflowVector_isr(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    int L1Shift         /* [in] Dummy arg. Not used by this interface. */
    )
{
    unsigned i, j;
    uint32_t Status, Mask, StatusAddr, MaskAddr;

    BDBG_ASSERT( hXpt );
    BSTD_UNUSED( L1Shift );

    for( i = 0; i < BXPT_NUM_MESG_BUFFERS; i += INTERRUPT_BITS_PER_REGISTER )
    {
        StatusAddr = BCHP_XPT_MSG_BUF_OVFL_INTR_00_31 + ( REGISTER_SIZE * ( i / INTERRUPT_BITS_PER_REGISTER ));
        Status = BREG_Read32( hXpt->hRegister, StatusAddr );

        MaskAddr = BCHP_XPT_MSG_BUF_OVFL_INTR_EN_00_31 + ( REGISTER_SIZE * ( i / INTERRUPT_BITS_PER_REGISTER ));
        Mask = BREG_Read32( hXpt->hRegister, MaskAddr );

        if( Status & Mask )
        {
            for( j = 0; j < INTERRUPT_BITS_PER_REGISTER; j++ )
            {
                if( ( Status & Mask ) & ( 1ul << j ) )
                {
                    BXPT_P_InterruptCallbackArgs *Cb = &hXpt->OverflowIntrCallbacks[ i + j ];

                    if( Cb->Callback != ( BINT_CallbackFunc ) NULL )
                        (*( Cb->Callback )) ( Cb->Parm1, Cb->Parm2 );
                }
            }
        }
    }
}

static void MapBitAndAddress_isrsafe(
    int MessageBufferNum,
    uint32_t InAddress,
    uint32_t *BitNum,
    uint32_t *OutAddress
    )
{
    *OutAddress = InAddress + ( REGISTER_SIZE * ( MessageBufferNum / INTERRUPT_BITS_PER_REGISTER ) );
    *BitNum = MessageBufferNum % INTERRUPT_BITS_PER_REGISTER;
}

static void SetBit_isr(
    BXPT_Handle hXpt,                       /* [in] Handle for this transport */
    uint32_t Addr,
    int BitNum
    )
{
    uint32_t EnReg = BREG_Read32( hXpt->hRegister, Addr );

    EnReg |= ( 1ul << BitNum );
    BREG_Write32( hXpt->hRegister, Addr, EnReg );
}

static void ClearBit_isr(
    BXPT_Handle hXpt,                       /* [in] Handle for this transport */
    uint32_t Addr,
    int BitNum
    )
{
    uint32_t Reg = BREG_Read32( hXpt->hRegister, Addr );

    Reg &= ~( 1ul << BitNum );
    BREG_Write32( hXpt->hRegister, Addr, Reg );
}
#endif /* BXPT_HAS_MESG_L2 */

/* end of file */
