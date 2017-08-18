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

/*******************************************************************************
 *
 * DESCRIPTION:
 *   Random Number Generator Hardware interface.
 *
*******************************************************************************/

#ifndef _BB_HAL_RANDOM_H
#define _BB_HAL_RANDOM_H


/************************* INCLUDES *****************************************************/
#include "bbSysBasics.h"            /* Basic system environment set. */


/************************* VALIDATIONS **************************************************/
/*
 * The True Random Number Generator (TRNG) and the Pseudo-Random Number Generator (PRNG)
 * may not be used in the same Project simultaneously.
 */
#if defined(_HAL_USE_TRNG_) && defined(_HAL_USE_PRNG_)
# error The TRNG and PRNG may not be used in the same Project simultaneously.
#endif


/*
 * The True Random Number Generator (TRNG) may be used only for the SoC target build. The
 * ML507 target build shall use the Pseudo-Random Number Generator (PRNG). The PRNG may
 * also be used by the SoC target platform for test purposes, because the output sequence
 * of the PRNG may be fully reproduced for the same Seed value.
 */
#if defined(_HAL_USE_TRNG_) && !defined(__SoC__)
# error Target platforms other than SoC do not provide a TRNG, they shall use PRNG.
#endif


/************************* AUTO CONFIGURATION *******************************************/
/*
 * By default use the Pseudo-Random Number Generator (PRNG).
 */
#if !defined(_HAL_USE_TRNG_) && !defined(_HAL_USE_PRNG_)
# define _HAL_USE_PRNG_
#endif


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Data type for the random value.
 */
typedef uint32_t  HAL_Random_t;


/**//**
 * \brief   Performs initialization of the random number generator.
 * \param[in]   prngDescr       Pointer to the descriptor of the PRNG to be used.
 * \param[in]   seed            Seed value for initialization of the PRNG, unsigned 32-bit
 *                                  nonzero integer.
 * \details The random number is generated either by the dedicated hardware True Random
 *  Number Generator (TRNG), or by the special software Pseudo-Random Number Generator
 *  (PRNG). In the case of PRNG a number of independent generators may be created in the
 *  same project for different purposes, each with its own output sequence. The
 *  \p prngDescr points to the descriptor of a software PRNG to be initialized here. This
 *  pointer is ignored by the TRNG.
 * \details The \p seed value is used in the case of PRNG for initialization of the
 *  generating function. The same Seed value instigates the function to perform the same
 *  output sequence. This value is ignored by the TRNG.
 */
#if defined(_HAL_USE_TRNG_)
# define HAL_RandomInit(prngDescr, seed)        SOC_TrngInit()
#
#else /* _HAL_USE_PRNG_ */
# define HAL_RandomInit(prngDescr, seed)        HAL_PrngInit(prngDescr, seed)
#
#endif


/**//**
 * \brief   Returns the current Counter value of the specified PRNG.
 * \param[in]   prngDescr       Pointer to the descriptor of the PRNG to be used.
 * \return  Value of the PRNG Counter, unsigned 32-bit integer.
 * \details In the case when random numbers are generated by the PRNG a number of
 *  independent generators may be created in the same project for different purposes, each
 *  with its own output sequence. The \p prngDescr points to the descriptor of the
 *  software PRNG to be asked for its Counter value.
 * \details Each PRNG automatically counts random numbers generated by itself and may
 *  return the current value of its Counter with this function. After initialization with
 *  the Seed value the Counter is set to zero; then when the PRNG generates the first
 *  random number, the Counter is incremented to one, etc. Any PRNG may be independently
 *  forwarded or rewound to a specific Counter value at any moment to reproduce its
 *  pseudo-random output sequence from the desired point.
 * \details This function is not implemented for the case of TRNG.
 */
#if defined(_HAL_USE_TRNG_)
# define HAL_RandomCounter(prngDescr)       0
#
#else /* _HAL_USE_PRNG_ */
# define HAL_RandomCounter(prngDescr)       HAL_PrngCounter(prngDescr)
#
#endif


/**//**
 * \brief   Forwards or rewinds the PRNG to the specified counter value.
 * \param[in]   prngDescr       Pointer to the descriptor of the PRNG to be used.
 * \param[in]   counter         Counter value for the PRNG to be forwarded or rewound to,
 *                                  unsigned 32-bit integer except 0xFFFFFFFF.
 * \details In the case when random numbers are generated by the PRNG a number of
 *  independent generators may be created in the same project for different purposes, each
 *  with its own output sequence. The \p prngDescr points to the descriptor of the
 *  software PRNG to be forwarded or rewound here.
 * \details The \p counter value specifies the target Counter value that shall be set on
 *  the specified PRNG. Each PRNG automatically counts random numbers generated by it and
 *  may publish the current value of its Counter. After initialization with the Seed value
 *  the Counter is set to zero; then when the PRNG generates the first random number, the
 *  Counter is incremented to one, etc. Note that after initialization with the same Seed
 *  value the PRNG produces the same output pseudo-random sequence. If one needs the PRNG
 *  to be forwarded to specific Counter value just in a single call, it shall call this
 *  function. The PRNG as well may also be rewound backward by this function to arbitrary
 *  value of its Counter and then reproduce its output sequence from the specified point.
 * \details This function is not implemented for the case of TRNG.
 * \note    The Counter value is unsigned 32-bit integer. In the case of overrun its value
 *  may become invalid because the period of the PRNG may not be equal to 2^32; according
 *  to this the PRNG counter is hold when it reaches the maximum allowed value 0xFFFFFFFF.
 * \note    This function uses cycling calculations that take a lot of time to reach the
 *  given \p counter. In order not to lock real-time interrupts (because PRNG may be used
 *  by their handlers) a local copy of the given PRNG is made, then it is forwarded or
 *  rewound by this function to the given Counter value, and finally assigned to the
 *  specified PRNG descriptor within atomic operation. Until this local copy of PRNG is
 *  ready, the corresponding PRNG continues producing numbers (if they are requested from
 *  real-time interrupts handlers) with the old descriptor state.
 */
#if defined(_HAL_USE_TRNG_)
# define HAL_RandomMove(prngDescr, counter)     while(0)
#
#else /* _HAL_USE_PRNG_ */
# define HAL_RandomMove(prngDescr, counter)     HAL_PrngMove(prngDescr, counter)
#
#endif


/**//**
 * \brief   Returns unsigned integer random number uniformly distributed on the range from
 *  0 to 2^32-1.
 * \param[in]   prngDescr       Pointer to the used PRNG descriptor.
 * \return  Random number, 32-bit unsigned integer value.
 * \details The random number is generated either by the dedicated hardware True Random
 *  Number Generator (TRNG), or by the special software Pseudo-Random Number Generator
 *  (PRNG). In the case of PRNG a number of independent generators may be created in the
 *  same project for different purposes, each with its own output sequence. The
 *  \p prngDescr points to the descriptor of the software PRNG that is asked for a random
 *  number here. This pointer is ignored by the TRNG.
 */
#if defined(_HAL_USE_TRNG_)
# define HAL_Random(prngDescr)          SOC_Trng()
#
#else /* _HAL_USE_PRNG_ */
# define HAL_Random(prngDescr)          HAL_Prng(prngDescr)
#
#endif


/************************* INCLUDES *****************************************************/
#if defined(_HAL_USE_TRNG_)
# include "bbSocTrng.h"             /* SoC True Random Number Generator Hardware interface. */
#
#else /* _HAL_USE_PRNG_ */
# include "bbHalPrng.h"             /* Cross-platform Pseudo-Random Number Generator Software interface. */
#
#endif


#endif /* _BB_HAL_RANDOM_H */

/* eof bbHalRandom.h */
