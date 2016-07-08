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
 ******************************************************************************
/*****************************************************************************
*
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   ML507 and SoC Pseudo-Random Number Generator Software implementation.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


/************************* INCLUDES *****************************************************/
#include "bbHalPrng.h"              /* ML507 and SoC Pseudo-Random Number Generator Software interface. */


/************************* VALIDATIONS **************************************************/
#if defined(_HAL_USE_TRNG_)
# error This unit shall be compiled only if the PRNG is used.
#endif


/************************* IMPLEMENTATION ***********************************************/
/*
 * Performs initialization of the pseudo-random number generator.
 */
void HAL_PrngInit(HAL_PrngDescr_t *const prngDescr, const HAL_PrngSeed_t seed)
{
    SYS_DbgAssertComplex(NULL != prngDescr, HALT_HAL_PrngInit_NullPrngDescr);
    SYS_DbgAssertComplex(0 != seed, HALT_HAL_PrngInit_ZeroSeed);

    HAL_PrngAlfgHistory_t  tempAlfgHistory[HAL_PRNG_ALFG_HISTORY_SIZE];     /*!< Buffer for ALFG history to make the
                                                                                atomic section as short as possible. */

    /* Initialize the stored history (lagged) values of the ALFG (Additive Lagged Fibonacci Generator) with the output
     * of the LSFR (Linear-Feedback Shift Register) in Galois Configuration. The LSFR is implemented in the 32-bit
     * configuration for the polynomial [X^32 + X^22 + X^2 + X^1 + 1] that is denoted here with the constant
     * 0x80200003. */

    uint32_t  lsfr = seed;              /* Initially load the LSFR with the Seed value. */

    /* Perform cycle over the ALFG history array to initialize it with the output of the LSFR. */
    for (size_t i = 0; i < HAL_PRNG_ALFG_HISTORY_SIZE; i++)
    {
        /* Run LSFR 32 times to renew all the 32 bits in the LSFR. */
        for (int j = 0; j < 32; j++)
        {
            uint32_t  lsb =             /* Save the LSB of the LSFR prior to shift it. */
                    lsfr & 0x1;
            lsfr >>= 1;                 /* Shift the LSFR to the right, then will perform the XOR on it. */
            if (0x0 != lsb)
                lsfr ^= 0x80200003;     /* Perform XOR on the shifted LSFR; load the MSB with the previous LSB. */
        }

        /* Save the new current value of the LSFR as the next history (lagged) value of the ALFG. */
        tempAlfgHistory[i] = lsfr;
    }

    ATOMIC_SECTION_ENTER(SYS_ATOMIC_DEFAULT_UID)
        /* Copy the ALFG history from the buffer to the destination PRNG descriptor. */
        memcpy(prngDescr->alfgHistory, tempAlfgHistory, sizeof(HAL_PrngAlfgHistory_t) * HAL_PRNG_ALFG_HISTORY_SIZE);
        /* Save the Seed value; reset the Counter value. */
        prngDescr->seed = seed;
        prngDescr->counter = 0;
    ATOMIC_SECTION_LEAVE(SYS_ATOMIC_DEFAULT_UID)
}


/*
 * Returns the current Counter value of the specified PRNG.
 */
HAL_PrngCounter_t HAL_PrngCounter(const HAL_PrngDescr_t *const prngDescr)
{
    SYS_DbgAssertComplex(NULL != prngDescr, HALT_HAL_PrngCounter_NullPrngDescr);

    HAL_PrngCounter_t  counter;     /* Counter value of the PRNG. */

    ATOMIC_SECTION_ENTER(SYS_ATOMIC_DEFAULT_UID)
        counter = prngDescr->counter;
    ATOMIC_SECTION_LEAVE(SYS_ATOMIC_DEFAULT_UID)

    return counter;
}


/*
 * Forwards or rewinds the PRNG to the specified counter value.
 */
void HAL_PrngMove(HAL_PrngDescr_t *const prngDescr, const HAL_PrngCounter_t counter)
{
    SYS_DbgAssertComplex(NULL != prngDescr, HALT_HAL_PrngMove_NullPrngDescr);
    SYS_DbgAssertComplex(counter < HAL_PRNG_COUNTER_MAX, HALT_HAL_PrngMove_InvalidCounter);

    HAL_PrngDescr_t  tempPrngDescr;         /*!< Temporary copy of the PRNG Descriptor to make the atomic section as
                                                short as possible. */

    /* Make a temporary copy of the PRNG current state. */
    ATOMIC_SECTION_ENTER(SYS_ATOMIC_DEFAULT_UID)
        memcpy(&tempPrngDescr, prngDescr, sizeof(tempPrngDescr));
    ATOMIC_SECTION_LEAVE(SYS_ATOMIC_DEFAULT_UID)

    HAL_PrngCounter_t  prngCounter =        /* Current value of the PRNG Counter. */
            tempPrngDescr.counter;

    /* If the target Counter value is in the past, then initialize the PRNG with the stored Seed value for the first
     * (i.e., rewind to the origin), and then proceed with forwarding if needed. */
    if (counter < prngCounter)
    {
        HAL_PrngInit(&tempPrngDescr, tempPrngDescr.seed);
        prngCounter = 0;
    }

    /* If need to perform forwarding, then call the ALFG (Additive Lagged Fibonacci Generator) appropriate number of
     * times. It will renew its history and the Counter value. */
    for (HAL_PrngCounter_t i = prngCounter; i < counter; i++)
        HAL_Prng(&tempPrngDescr);

    /* If the target Counter value equals the current one (initially or after rewinding), then do nothing. */

    /* Assign the PRNG with forwarded/rewinded state. */
    ATOMIC_SECTION_ENTER(SYS_ATOMIC_DEFAULT_UID)
        memcpy(prngDescr, &tempPrngDescr, sizeof(*prngDescr));
    ATOMIC_SECTION_LEAVE(SYS_ATOMIC_DEFAULT_UID)
}


/*
 * Returns unsigned integer random number uniformly distributed on the range from 0 to
 * 2^32-1.
 */
HAL_Random_t HAL_Prng(HAL_PrngDescr_t *const prngDescr)
{
    SYS_DbgAssertComplex(NULL != prngDescr, HALT_HAL_Prng_NullPrngDescr);

    HAL_PrngAlfgHistory_t  Sn;                  /* New pseudo-random value to be returned. */

    HAL_PrngAlfgHistory_t *alfgHistory =        /* Shortcut to the ALFG history array. */
            prngDescr->alfgHistory;

    ATOMIC_SECTION_ENTER(SYS_ATOMIC_DEFAULT_UID)
    {
        /* Take the lagged values from the ALFG (Additive Lagged Fibonacci Generator) history array. The first element
         * of the array with index [0] correspond to the lagged value of S with index (n-1), etc. */
        HAL_PrngAlfgHistory_t  Sn_j =
                alfgHistory[HAL_PRNG_ALFG_J - 1];       /* Value of the S_(n-j). */
        HAL_PrngAlfgHistory_t  Sn_k =
                alfgHistory[HAL_PRNG_ALFG_K - 1];       /* Value of the S_(n-k). */

        /* Perform a single iteration of the ALFG (Additive Lagged Fibonacci Generator). */
        Sn = Sn_j + Sn_k;                               /* Value of the S_n = [S_(n-j) + S_(n-k)] mod 2^32. */

        /* Save the new value in the ALFG history. */
        memmove(alfgHistory + 1, alfgHistory, (HAL_PRNG_ALFG_HISTORY_SIZE - 1) * sizeof(HAL_PrngAlfgHistory_t));
        alfgHistory[0] = Sn;

        /* Increment the Counter limiting it on the 0xFFFFFFFF. */
        if (prngDescr->counter < HAL_PRNG_COUNTER_MAX)
            prngDescr->counter++;
    }
    ATOMIC_SECTION_LEAVE(SYS_ATOMIC_DEFAULT_UID)

    /* Return the new Sn value. */
    return Sn;
}


/* eof bbHalPrng.c */