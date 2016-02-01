/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
/*****************************************************************************
*
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   ML507 and SoC Pseudo-Random Number Generator Software interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_HAL_PRNG_H
#define _BB_HAL_PRNG_H


/************************* INCLUDES *****************************************************/
#include "bbHalRandom.h"            /* Random Number Generator Hardware interface. */


/************************* VALIDATIONS **************************************************/
#if defined(_HAL_USE_TRNG_)
# error This header shall be compiled only if the PRNG is used.
#endif


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Data type for the PRNG Seed value.
 */
typedef uint32_t  HAL_PrngSeed_t;


/**//**
 * \brief   Data type for the PRNG Counter value.
 */
typedef uint32_t  HAL_PrngCounter_t;


/**//**
 * \brief   Maximum value of PRNG Counter at which it locks.
 */
#define HAL_PRNG_COUNTER_MAX    0xFFFFFFFF


/**//**
 * \name    Parameters of the ALFG (Additive Lagged Fibonacci Generator).
 */
/**@{*/
#define HAL_PRNG_ALFG_J     7       /*!< Lag \a j of the pair (j,k)=(7,10). */
#define HAL_PRNG_ALFG_K     10      /*!< Lag \a k of the pair (j,k)=(7,10). */
/**@}*/


/**//**
 * \brief   Size of the history array for the ALFG.
 */
#define HAL_PRNG_ALFG_HISTORY_SIZE  MAX(HAL_PRNG_ALFG_J, HAL_PRNG_ALFG_K)


/**//**
 * \brief   Data type for the history (lagged) values of the ALFG.
 */
typedef HAL_Random_t  HAL_PrngAlfgHistory_t;


/**//**
 * \brief   Structure for the Descriptor of a PRNG.
 */
typedef struct _HAL_PrngDescr_t
{
    /* 32-bit values. */
    HAL_PrngSeed_t         seed;            /*!< Seed value with which the PRNG generating function was initialized.
                                                Used for PRNG rewinding backward. */

    HAL_PrngCounter_t      counter;         /*!< Current value of the PRNG Counter. Counts the number of generated
                                                pseudo-random numbers. Assigned to zero on initialization with Seed.
                                                Holds at the maximum allowed value instead of overrun. */

    HAL_PrngAlfgHistory_t  alfgHistory[HAL_PRNG_ALFG_HISTORY_SIZE]; /*!< Stored history (lagged) values of the ALFG. */

} HAL_PrngDescr_t;


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Performs initialization of the pseudo-random number generator.
 * \param[in]   prngDescr       Pointer to the descriptor of the PRNG to be used.
 * \param[in]   seed            Seed value for initialization of the PRNG, unsigned 32-bit
 *                                  nonzero integer.
 * \details
 *  The \p prngDescr points to the descriptor of the PRNG (Pseudo-Random Number Generator)
 *  to be initialized with this function call. The \p seed value is used for pseudo-random
 *  number generating function initialization. The same \p seed value instigates the PRNG
 *  to perform the same output pseudo-random sequence.
 * \details
 *  The main pseudo-random number generator in this implementation is the ALFG (Additive
 *  Lagged Fibonacci Generator); it is used for generating the output values of the PRNG.
 *  The LSFR (Linear-Feedback Shift Register) in Galois Configuration is used for
 *  initialization of the ALFG. And the \p seed value in turn is used for initialization
 *  of the LSFR prior to initialize the ALFG.
*****************************************************************************************/
HAL_PUBLIC void HAL_PrngInit(HAL_PrngDescr_t *const prngDescr, const HAL_PrngSeed_t seed);


/*************************************************************************************//**
 * \brief   Returns the current Counter value of the specified PRNG.
 * \param[in]   prngDescr       Pointer to the descriptor of the PRNG to be used.
 * \return  Value of the PRNG Counter, unsigned 32-bit integer.
 * \details Each PRNG automatically counts random numbers generated by it and may return
 *  the current value of its Counter with this function. After initialization with the
 *  Seed value the Counter is set to zero; then when the PRNG generates the first random
 *  number, the Counter is incremented to one, etc. Any PRNG may be independently
 *  forwarded or rewinded to a specific Counter value at any moment to reproduce its
 *  pseudo-random output sequence from desired point.
*****************************************************************************************/
HAL_PUBLIC HAL_PrngCounter_t HAL_PrngCounter(const HAL_PrngDescr_t *const prngDescr);


/*************************************************************************************//**
 * \brief   Forwards or rewinds the PRNG to the specified counter value.
 * \param[in]   prngDescr       Pointer to the descriptor of the PRNG to be used.
 * \param[in]   counter         Counter value for the PRNG to be forwarded or rewinded to,
 *                                  unsigned 32-bit integer except 0xFFFFFFFF.
 * \details
 *  The \p counter value specifies the target Counter value that shall be set on the
 *  specified PRNG. Each PRNG automatically counts random numbers generated by it and may
 *  publish the current value of its Counter. After initialization with the Seed value the
 *  Counter is set to zero; then when the PRNG generates the first random number, the
 *  Counter is incremented to one, etc. Note that after initialization with the same Seed
 *  value the PRNG produces the same output pseudo-random sequence. If one needs the PRNG
 *  to be forwarded to specific Counter value just in a single call, it shall call this
 *  function. The PRNG as well may also be rewinded backward by this function to arbitrary
 *  value of its Counter and then reproduce its output sequence from the specified point.
 * \note
 *  The Counter value is unsigned 32-bit integer. In the case of overrun its value may
 *  become invalid because the period of the PRNG may not be equal to 2^32; according to
 *  this the PRNG counter is hold when it reaches the maximum allowed value 0xFFFFFFFF.
 * \note
 *  This function uses cycling calculations that take a lot of time and must not be
 *  interrupted with other requests to the same PRNG.
*****************************************************************************************/
HAL_PUBLIC void HAL_PrngMove(HAL_PrngDescr_t *const prngDescr, const HAL_PrngCounter_t counter);


/*************************************************************************************//**
 * \brief   Returns unsigned integer random number uniformly distributed on the range from
 *  0 to 2^32-1.
 * \param[in]   prngDescr       Pointer to the used PRNG descriptor.
 * \return  Random number, 32-bit unsigned integer value.
 * \details
 *  This function implements the ALFG (Additive Lagged Fibonacci Generator).
*****************************************************************************************/
HAL_PUBLIC HAL_Random_t HAL_Prng(HAL_PrngDescr_t *const prngDescr);


#endif /* _BB_HAL_PRNG_H */