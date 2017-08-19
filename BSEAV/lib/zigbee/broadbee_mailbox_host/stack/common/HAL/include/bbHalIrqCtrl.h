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
 *      Interrupt Controller Hardware interface.
 *
*******************************************************************************/

#ifndef _BB_HAL_IRQ_CTRL_H
#define _BB_HAL_IRQ_CTRL_H

/************************* INCLUDES ***********************************************************************************/
#if defined(__SoC__)
# include "bbSocIrqCtrl.h"
#elif defined(__ML507__)
# include "bbSysTypes.h"
#endif


#if defined(__SoC__) || defined(__ML507__)
# ifndef  _AUX_DEFINES_NOT_WANTED
#  define _AUX_DEFINES_NOT_WANTED
# endif
# include <arc/arc_reg.h>
#
#elif defined(__i386__)
# include <pthread.h>
#
#endif

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \brief   Enables interrupts processing.
 */
#if defined(__SoC__) || defined(__ML507__)
# define HAL_IRQ_ENABLE()                   _enable()
#else /* ARM, i386 */
# define HAL_IRQ_ENABLE()                   while(0)
#endif

/**//**
 * \brief   Disables all interrupts processing.
 * \details This macro disables both Level 1 and Level 2 interrupts processing on all platforms.
 */
#if defined(__SoC__) || defined(__ML507__)
# define HAL_IRQ_DISABLE()                  asm volatile("flag 0")
#else /* ARM, i386 */
# define HAL_IRQ_DISABLE()                  while(0)
#endif

/**//**
 * \brief   Disables low priority interrupts processing.
 * \details On ML507 the high priority interrupts are left in their current status. On SoC the high priority interrupt
 *  are also disabled.
 */
#if defined(__SoC__)
# define HAL_IRQ_DISABLE_LP()               _disable()
#elif defined(__ML507__)
# define HAL_IRQ_DISABLE_LP()               _disable1()
#else /* ARM, i386 */
# define HAL_IRQ_DISABLE_LP()               while(0)
#endif

/**//**
 * \brief   Returns STATUS_32 register of ARC CPU.
 * \return  Current value of the STATUS_32 register.
 * \details This macro shall be used prior to call HAL_IRQ_DISABLE() at the beginning of atomic section in order to save
 *  the current state of global Interrupt Enable Level 1 and 2 flags and restore them at the end of the atomic section
 *  with HAL_IRQ_SET_STATUS().
 */
#if defined(__SoC__)
# define HAL_IRQ_GET_STATUS()               _lr(ARC_AUXILIARY_REGS_STATUS32)
#elif defined(__ML507__)
# define HAL_IRQ_GET_STATUS()               _lr(REG_STATUS32)
#else /* ARM, i386 */
# define HAL_IRQ_GET_STATUS()               (0)
#endif

/**//**
 * \brief   Assigns STATUS_32 register of ARC CPU.
 * \param[in]   status      New value for the STATUS_32 register.
 * \details This macro shall be used in pair with the HAL_IRQ_GET_STATUS().
 */
#if defined(__SoC__) || defined(__ML507__)
# define HAL_IRQ_SET_STATUS(status)         _flag(status)
#else /* ARM, i386 */
# define HAL_IRQ_SET_STATUS(status)         ((void)(status))
#endif

/**//**
 * \brief   Starts atomic section with all interrupts disabled.
 */
#if defined(__SoC__) || defined(__ML507__)
# define HAL_ATOMIC_START\
        {\
            const uint32_t status32_saved = _lr(REG_STATUS32);\
            _disable();\
            do {
#
#else /* ARM, i386 */
# define HAL_ATOMIC_START\
        {
#endif

/**//**
 * \brief   Starts atomic section with only low priority interrupts disabled.
 * \note    On ML507 this macro disables only Level 1 interrupts, while Level 2 interrupts stays enabled. These Level 2
 *  interrupts are used on ML507 for the radio hardware servicing. SoC does not use Level 2 interrupts.
 */
#if defined(__ML507__)
# define HAL_ATOMIC_START_LP\
        {\
            const uint32_t status32_saved = _lr(REG_STATUS32);\
            _disable1();\
            do {
#
#else /* SoC, ARM, i386 */
# define HAL_ATOMIC_START_LP                HAL_ATOMIC_START
#endif

/**//**
 * \brief   Finishes atomic section and restores the original interrupt status.
 */
#if defined(__SoC__) || defined(__ML507__)
# define HAL_ATOMIC_END\
            } while(0);\
            _flag(status32_saved);\
        }
#
#else /* ARM, i386 */
# define HAL_ATOMIC_END\
        }
#endif

#ifdef __ML507__
# ifndef ML507_MAIN_CLOCK_MHZ
#  error The main clock frequency is not specified. It must be set either to 54 MHz or 27 MHz in the project config.mk.
# elif (ML507_MAIN_CLOCK_MHZ != 54) && (ML507_MAIN_CLOCK_MHZ != 27)
#  error The main clock frequency must be set either to 54 MHz or 27 MHz.
# endif
#endif

/**//**
 * \brief   Delays the ARC program execution for the specified number of main clocks.
 * \param[in]   clocks      Period of time, in main clocks, for which the program execution is to be delayed.
 * \note    The \p clocks is specified in the main clocks. Due to this reason the performed delay depends on the main
 *  clock frequency. The caller is responsible for proper converting values from the real time scale into main clocks.
 */
#define HAL_DELAY(clocks)\
        do {\
            for (uint32_t t = (clocks); t > 0; t--)\
                asm volatile("nop_s");\
        } while(0)

/**//**
 * \brief   Initializes the Interrupt Controller hardware and resets ARC Timers #0 and #1.
 * \details This function performs the default configuration of interrupts. By default all interrupts are configured as
 *  Level 2 (high priority) interrupts and assumed not to be used. All IRQs that are actually in use must be further
 *  reconfigured by dedicated configuration functions, if necessary.
 * \note    This function does not enable interrupts processing.
 */
#if defined(__SoC__)
# define HAL_IrqCtrl__Init()                SOC_IrqCtrlInit()
#elif defined(__ML507__)
# define HAL_IrqCtrl__Init()\
        asm volatile("\t"\
                "flag    0\n\t"\
                "sr      0xFFFFFFFF, [%aux_irq_lev]\n\t"\
                "sr      0, [%control0]\n\t"\
                "sr      0, [%control1]")
#
#else /* ARM, i386 */
# define HAL_IrqCtrl__Init()                while(0)
#endif

#endif /* _BB_HAL_IRQ_CTRL_H */

/* eof bbHalIrqCtrl.h */