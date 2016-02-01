/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"

#ifdef BCHP_AIO_INTH_REG_START
#include "bchp_int_id_aio_inth.h"
#include "bchp_aud_fmm_bf_esr1_h.h"
#include "bchp_aud_fmm_bf_esr2_h.h"

static void BAPE_P_BfEsr1_isr(void *pParam1, int param2);
static void BAPE_P_BfEsr2_isr(void *pParam1, int param2);
#endif

#ifdef BCHP_AUD_INTH_REG_START
#include "bchp_int_id_aud_inth.h"
#include "bchp_aud_fmm_bf_esr.h"

static void BAPE_P_BfEsr2_isr(void *pParam1, int param2);
static void BAPE_P_BfEsr3_isr(void *pParam1, int param2);
static void BAPE_P_BfEsr4_isr(void *pParam1, int param2);
#endif

BDBG_MODULE(bape_fmm_isr);

BERR_Code BAPE_P_InitInterrupts(
    BAPE_Handle handle
    )
{
    BERR_Code errCode;
    uint32_t intId;

    /* Clear the L3 interrupts before enabling any callbacks */
#ifdef BCHP_AUD_FMM_BF_ESR2_H_MASK_SET
    BREG_Write32(handle->regHandle, BCHP_AUD_FMM_BF_ESR2_H_MASK_SET, 0xffffffff&~BCHP_MASK(AUD_FMM_BF_ESR2_H_MASK_SET, reserved0));
    intId = BCHP_INT_ID_FMM_BF2;
#else
    BREG_Write32(handle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR2_MASK_SET, 0xffffffff&~BCHP_MASK(AUD_FMM_BF_ESR_ESR2_MASK_SET, reserved0));
    intId = BCHP_INT_ID_AUD_BF2;
#endif
    
    /* Install the L2 handler */
    errCode = BINT_CreateCallback(&handle->isrBfEsr2,
                                  handle->intHandle,
                                  intId,
                                  BAPE_P_BfEsr2_isr,
                                  handle,
                                  0);
    if ( errCode )
    {
        BAPE_P_UninitInterrupts(handle);
        return BERR_TRACE(errCode);
    }

    BINT_EnableCallback(handle->isrBfEsr2);

#ifdef BCHP_AIO_INTH_REG_START
    /* Clear the L3 interrupts before enabling any callbacks */
    BREG_Write32(handle->regHandle, BCHP_AUD_FMM_BF_ESR1_H_MASK_SET, 0xffffffff&~BCHP_MASK(AUD_FMM_BF_ESR1_H_MASK_SET, reserved0));

    /* Install the L2 handler */
    errCode = BINT_CreateCallback(&handle->isrBfEsr1,
                                  handle->intHandle,
                                  BCHP_INT_ID_FMM_BF1,
                                  BAPE_P_BfEsr1_isr,
                                  handle,
                                  0);
    if ( errCode )
    {
        BAPE_P_UninitInterrupts(handle);
        return BERR_TRACE(errCode);
    }

    BINT_EnableCallback(handle->isrBfEsr1);
#else
    /* Clear the L3 interrupts before enabling any callbacks */
    BREG_Write32(handle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR3_MASK_SET, 0xffffffff&~BCHP_MASK(AUD_FMM_BF_ESR_ESR3_MASK_SET, reserved0));

    /* Install the L2 handler */
    errCode = BINT_CreateCallback(&handle->isrBfEsr3,
                                  handle->intHandle,
                                  BCHP_INT_ID_AUD_BF3,
                                  BAPE_P_BfEsr3_isr,
                                  handle,
                                  0);
    if ( errCode )
    {
        BAPE_P_UninitInterrupts(handle);
        return BERR_TRACE(errCode);
    }

    BINT_EnableCallback(handle->isrBfEsr3);

    /* Clear the L3 interrupts before enabling any callbacks */
    BREG_Write32(handle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR4_MASK_SET, 0xffffffff&~BCHP_MASK(AUD_FMM_BF_ESR_ESR4_MASK_SET, reserved0));

    /* Install the L2 handler */
    errCode = BINT_CreateCallback(&handle->isrBfEsr4,
                                  handle->intHandle,
                                  BCHP_INT_ID_AUD_BF4,
                                  BAPE_P_BfEsr4_isr,
                                  handle,
                                  0);
    if ( errCode )
    {
        BAPE_P_UninitInterrupts(handle);
        return BERR_TRACE(errCode);
    }

    BINT_EnableCallback(handle->isrBfEsr4);
#endif    

    return errCode;
}

void BAPE_P_UninitInterrupts(
    BAPE_Handle handle
    )
{
    if ( handle->isrBfEsr4 )
    {
        BKNI_EnterCriticalSection();
        BINT_DisableCallback_isr(handle->isrBfEsr4);
        BINT_ClearCallback_isr(handle->isrBfEsr4);
        BKNI_LeaveCriticalSection();
        BINT_DestroyCallback(handle->isrBfEsr4);
        handle->isrBfEsr4 = NULL;
    }

    if ( handle->isrBfEsr3 )
    {
        BKNI_EnterCriticalSection();
        BINT_DisableCallback_isr(handle->isrBfEsr3);
        BINT_ClearCallback_isr(handle->isrBfEsr3);
        BKNI_LeaveCriticalSection();
        BINT_DestroyCallback(handle->isrBfEsr3);
        handle->isrBfEsr3 = NULL;
    }

    if ( handle->isrBfEsr2 )
    {
        BKNI_EnterCriticalSection();
        BINT_DisableCallback_isr(handle->isrBfEsr2);
        BINT_ClearCallback_isr(handle->isrBfEsr2);
        BKNI_LeaveCriticalSection();
        BINT_DestroyCallback(handle->isrBfEsr2);
        handle->isrBfEsr2 = NULL;
    }

    if ( handle->isrBfEsr1 )
    {
        BKNI_EnterCriticalSection();
        BINT_DisableCallback_isr(handle->isrBfEsr1);
        BINT_ClearCallback_isr(handle->isrBfEsr1);
        BKNI_LeaveCriticalSection();
        BINT_DestroyCallback(handle->isrBfEsr1);
        handle->isrBfEsr1 = NULL;
    }
}

BERR_Code BAPE_P_SetSourceChannelFreemarkInterrupt(
    BAPE_Handle handle,
    unsigned sourceChannelId,
    BINT_CallbackFunc callback_isr,
    void *pParam1,
    int param2)
{
    uint32_t bitmask;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    if ( sourceChannelId >= BAPE_CHIP_MAX_SFIFOS )
    {
        BDBG_ASSERT(sourceChannelId < BAPE_CHIP_MAX_SFIFOS);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BKNI_EnterCriticalSection();
    handle->sourceRbufFreemark[sourceChannelId].callback_isr = callback_isr;
    handle->sourceRbufFreemark[sourceChannelId].pParam1 = pParam1;
    handle->sourceRbufFreemark[sourceChannelId].param2 = param2;
    BKNI_LeaveCriticalSection();

#ifdef BCHP_AUD_FMM_BF_ESR2_H_MASK
    bitmask = BCHP_MASK(AUD_FMM_BF_ESR2_H_MASK, SOURCE_RINGBUF_0_EXCEED_FREEMARK) << sourceChannelId;

    if ( callback_isr )
    {
        /* Enable the interrupt */
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_BF_ESR2_H_MASK_CLEAR, bitmask);
    }
    else
    {
        /* Disable the interrupt */
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_BF_ESR2_H_MASK_SET, bitmask);
    }
#else
    bitmask = BCHP_MASK(AUD_FMM_BF_ESR_ESR3_MASK, SOURCE_RINGBUF_0_EXCEED_FREEMARK) << sourceChannelId;

    if ( callback_isr )
    {
        /* Enable the interrupt */
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR3_MASK_CLEAR, bitmask);
    }
    else
    {
        /* Disable the interrupt */
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR3_MASK_SET, bitmask);
    }
#endif

    return BERR_SUCCESS;
}

BERR_Code BAPE_P_SetDfifoFullmarkInterrupt(
    BAPE_Handle handle,
    unsigned destChannelId,
    BINT_CallbackFunc callback_isr,
    void *pParam1,
    int param2
    )
{
    uint32_t bitmask;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    if ( destChannelId >= BAPE_CHIP_MAX_DFIFOS )
    {
        BDBG_ASSERT(destChannelId < BAPE_CHIP_MAX_DFIFOS);
        return BERR_TRACE(BERR_INVALID_PARAMETER);;
    }

    BKNI_EnterCriticalSection();
    handle->destRbufFullmark[destChannelId].callback_isr = callback_isr;
    handle->destRbufFullmark[destChannelId].pParam1 = pParam1;
    handle->destRbufFullmark[destChannelId].param2 = param2;
    BKNI_LeaveCriticalSection();

#ifdef BCHP_AUD_FMM_BF_ESR2_H_MASK
    bitmask = BCHP_MASK(AUD_FMM_BF_ESR2_H_MASK, DEST_RINGBUF_0_EXCEED_FULLMARK) << destChannelId;

    if ( callback_isr )
    {
        /* Enable the interrupt */
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_BF_ESR2_H_MASK_CLEAR, bitmask);
    }
    else
    {
        /* Disable the interrupt */
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_BF_ESR2_H_MASK_SET, bitmask);
    }
#else
    bitmask = BCHP_MASK(AUD_FMM_BF_ESR_ESR4_MASK, DEST_RINGBUF_0_EXCEED_FULLMARK) << destChannelId;

    if ( callback_isr )
    {
        /* Enable the interrupt */
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR4_MASK_CLEAR, bitmask);
    }
    else
    {
        /* Disable the interrupt */
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR4_MASK_SET, bitmask);
    }
#endif

    return BERR_SUCCESS;
}

BERR_Code BAPE_P_SetDfifoOverflowInterrupt(
    BAPE_Handle handle,
    unsigned destChannelId,
    BINT_CallbackFunc callback_isr,
    void *pParam1,
    int param2
    )
{
    uint32_t bitmask;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    if ( destChannelId >= BAPE_CHIP_MAX_DFIFOS )
    {
        BDBG_ASSERT(destChannelId < BAPE_CHIP_MAX_DFIFOS);
        return BERR_TRACE(BERR_INVALID_PARAMETER);;
    }

    BKNI_EnterCriticalSection();
    handle->destRbufOverflow[destChannelId].callback_isr = callback_isr;
    handle->destRbufOverflow[destChannelId].pParam1 = pParam1;
    handle->destRbufOverflow[destChannelId].param2 = param2;
    BKNI_LeaveCriticalSection();

#ifdef BCHP_AUD_FMM_BF_ESR1_H_MASK
    bitmask = BCHP_MASK(AUD_FMM_BF_ESR1_H_MASK, DEST_RINGBUF_0_OVERFLOW) << destChannelId;

    if ( callback_isr )
    {
        /* Enable the interrupt */
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_BF_ESR1_H_MASK_CLEAR, bitmask);
    }
    else
    {
        /* Disable the interrupt */
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_BF_ESR1_H_MASK_SET, bitmask);
    }
#else
    bitmask = BCHP_MASK(AUD_FMM_BF_ESR_ESR2_MASK, DEST_RINGBUF_0_OVERFLOW) << destChannelId;

    if ( callback_isr )
    {
        /* Enable the interrupt */
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR2_MASK_CLEAR, bitmask);
    }
    else
    {
        /* Disable the interrupt */
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR2_MASK_SET, bitmask);
    }
#endif

    return BERR_SUCCESS;
}

#ifdef BCHP_AIO_INTH_REG_START
static void BAPE_P_BfEsr1_isr(void *pParam1, int param2)
{
    uint32_t bitmask, mask, status;
    BAPE_Handle deviceHandle = pParam1;
    unsigned i;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BSTD_UNUSED(param2);

    /* TODO: Source channel underflow if needed */

    status = BREG_Read32_isr(deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR1_H_STATUS);
    mask = BREG_Read32_isr(deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR1_H_MASK);
    bitmask = BCHP_MASK(AUD_FMM_BF_ESR1_H_MASK, DEST_RINGBUF_0_OVERFLOW);

    BDBG_MSG(("BF ESR1 ISR S:0x%08x M:0x%08x", status, mask));

    for ( i = 0; i < BAPE_CHIP_MAX_DFIFOS; i++, bitmask<<=1 )
    {
        if ( status & ~mask & bitmask )
        {
            /* Sanity check */
            BDBG_ASSERT(NULL != deviceHandle->destRbufOverflow[i].callback_isr);

            BDBG_MSG(("Destination Channel %d overflow interrupt", i));
            /* Fire the callback */
            deviceHandle->destRbufOverflow[i].callback_isr(deviceHandle->destRbufOverflow[i].pParam1, deviceHandle->destRbufOverflow[i].param2);
            /* Clear the status bit */
            BREG_Write32_isr(deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR1_H_STATUS_CLEAR, bitmask);
        }
    }

    BINT_EnableCallback_isr(deviceHandle->isrBfEsr1);
}

static void BAPE_P_BfEsr2_isr(void *pParam1, int param2)
{
    uint32_t bitmask, mask, status;
    BAPE_Handle deviceHandle = pParam1;
    unsigned i;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BSTD_UNUSED(param2);

    status = BREG_Read32_isr(deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR2_H_STATUS);
    mask = BREG_Read32_isr(deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR2_H_MASK);
    bitmask = BCHP_MASK(AUD_FMM_BF_ESR2_H_MASK, SOURCE_RINGBUF_0_EXCEED_FREEMARK);

    BDBG_MSG(("BF ESR2 ISR S:0x%08x M:0x%08x", status, mask));

    for ( i = 0; i < BAPE_CHIP_MAX_SFIFOS; i++, bitmask<<=1 )
    {
        if ( status & ~mask & bitmask )
        {
            /* Sanity check */
            BDBG_ASSERT(NULL != deviceHandle->sourceRbufFreemark[i].callback_isr);

            BDBG_MSG(("Source Channel %d freemark interrupt", i));
            /* Fire the callback */
            deviceHandle->sourceRbufFreemark[i].callback_isr(deviceHandle->sourceRbufFreemark[i].pParam1, deviceHandle->sourceRbufFreemark[i].param2);
            /* Clear the status bit */
            BREG_Write32_isr(deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR2_H_STATUS_CLEAR, bitmask);
        }
    }

    bitmask = BCHP_MASK(AUD_FMM_BF_ESR2_H_MASK, DEST_RINGBUF_0_EXCEED_FULLMARK);
    for ( i = 0; i < BAPE_CHIP_MAX_DFIFOS; i++, bitmask<<=1 )
    {
        if ( status & ~mask & bitmask )
        {
            /* Sanity check */
            BDBG_ASSERT(NULL != deviceHandle->destRbufFullmark[i].callback_isr);

            BDBG_MSG(("Destination Channel %d fullmark interrupt", i));
            /* Fire the callback */
            deviceHandle->destRbufFullmark[i].callback_isr(deviceHandle->destRbufFullmark[i].pParam1, deviceHandle->destRbufFullmark[i].param2);
            /* Clear the status bit */
            BREG_Write32_isr(deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR2_H_STATUS_CLEAR, bitmask);
        }
    }

    BINT_EnableCallback_isr(deviceHandle->isrBfEsr2);
}
#else
/* 7429-style interrupts */
static void BAPE_P_BfEsr2_isr(void *pParam1, int param2)
{
    uint32_t bitmask, mask, status;
    BAPE_Handle deviceHandle = pParam1;
    unsigned i;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BSTD_UNUSED(param2);

    status = BREG_Read32_isr(deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR2_STATUS);
    mask = BREG_Read32_isr(deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR2_MASK);
    bitmask = BCHP_MASK(AUD_FMM_BF_ESR_ESR2_MASK, DEST_RINGBUF_0_OVERFLOW);

    BDBG_MSG(("BF ESR2 ISR S:0x%08x M:0x%08x", status, mask));

    for ( i = 0; i < BAPE_CHIP_MAX_DFIFOS; i++, bitmask<<=1 )
    {
        if ( status & ~mask & bitmask )
        {
            /* Sanity check */
            BDBG_ASSERT(NULL != deviceHandle->destRbufOverflow[i].callback_isr);

            BDBG_MSG(("Dest Channel %d overflow interrupt", i));
            /* Fire the callback */
            deviceHandle->destRbufOverflow[i].callback_isr(deviceHandle->destRbufOverflow[i].pParam1, deviceHandle->destRbufOverflow[i].param2);
            /* Clear the status bit */
            BREG_Write32_isr(deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR2_STATUS_CLEAR, bitmask);
        }
    }

    BINT_EnableCallback_isr(deviceHandle->isrBfEsr2);
}

static void BAPE_P_BfEsr3_isr(void *pParam1, int param2)
{
    uint32_t bitmask, mask, status;
    BAPE_Handle deviceHandle = pParam1;
    unsigned i;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BSTD_UNUSED(param2);

    status = BREG_Read32_isr(deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR3_STATUS);
    mask = BREG_Read32_isr(deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR3_MASK);
    bitmask = BCHP_MASK(AUD_FMM_BF_ESR_ESR3_MASK, SOURCE_RINGBUF_0_EXCEED_FREEMARK);

    BDBG_MSG(("BF ESR3 ISR S:0x%08x M:0x%08x", status, mask));

    for ( i = 0; i < BAPE_CHIP_MAX_SFIFOS; i++, bitmask<<=1 )
    {
        if ( status & ~mask & bitmask )
        {
            /* Sanity check */
            BDBG_ASSERT(NULL != deviceHandle->sourceRbufFreemark[i].callback_isr);

            BDBG_MSG(("Source Channel %d freemark interrupt", i));
            /* Fire the callback */
            deviceHandle->sourceRbufFreemark[i].callback_isr(deviceHandle->sourceRbufFreemark[i].pParam1, deviceHandle->sourceRbufFreemark[i].param2);
            /* Clear the status bit */
            BREG_Write32_isr(deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR3_STATUS_CLEAR, bitmask);
        }
    }

    BINT_EnableCallback_isr(deviceHandle->isrBfEsr3);
}

static void BAPE_P_BfEsr4_isr(void *pParam1, int param2)
{
    uint32_t bitmask, mask, status;
    BAPE_Handle deviceHandle = pParam1;
    unsigned i;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BSTD_UNUSED(param2);

    status = BREG_Read32_isr(deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR4_STATUS);
    mask = BREG_Read32_isr(deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR4_MASK);
    bitmask = BCHP_MASK(AUD_FMM_BF_ESR_ESR4_MASK, DEST_RINGBUF_0_EXCEED_FULLMARK);

    BDBG_MSG(("BF ESR4 ISR S:0x%08x M:0x%08x", status, mask));

    for ( i = 0; i < BAPE_CHIP_MAX_DFIFOS; i++, bitmask<<=1 )
    {
        if ( status & ~mask & bitmask )
        {
            /* Sanity check */
            BDBG_ASSERT(NULL != deviceHandle->destRbufFullmark[i].callback_isr);

            BDBG_MSG(("Dest Channel %d fullmark interrupt", i));
            /* Fire the callback */
            deviceHandle->destRbufFullmark[i].callback_isr(deviceHandle->destRbufFullmark[i].pParam1, deviceHandle->destRbufFullmark[i].param2);
            /* Clear the status bit */
            BREG_Write32_isr(deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR4_STATUS_CLEAR, bitmask);
        }
    }

    BINT_EnableCallback_isr(deviceHandle->isrBfEsr4);
}

#endif
