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
* FILENAME: $Workfile: trunk/stack/common/HAL/include/bbHalIrqCtrl.h $
*
* DESCRIPTION:
*   Interrupt Controller Hardware interface.
*
* $Revision: 3046 $
* $Date: 2014-07-24 20:36:50Z $
*
*****************************************************************************************/


#ifndef _BB_HAL_IRQ_CTRL_H
#define _BB_HAL_IRQ_CTRL_H


/************************* INCLUDES *****************************************************/
#if defined(__SoC__)
# include "bbSocIrqCtrl.h"          /* SoC Interrupt Controller Hardware interface. */
#elif defined(__ML507__)
# include "bbMl507IrqCtrl.h"        /* ML507 Interrupt Controller Hardware interface. */
#else /* __i386__ */
# include "bbPcIrqCtrl.h"           /* i386 Interrupt Controller Simulator interface. */
#endif


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enables hardware interrupts processing.
 */
#if defined(__SoC__)
# define HAL_IRQ_ENABLE()                   SOC_IRQ_ENABLE()
#elif defined(__ML507__)
# define HAL_IRQ_ENABLE()                   ML507_IRQ_ENABLE()
#else /* __i386__ */
# define HAL_IRQ_ENABLE()                   PC_IRQ_ENABLE()
#endif


/**//**
 * \brief   Disables hardware interrupts processing.
 */
#if defined(__SoC__)
# define HAL_IRQ_DISABLE()                  SOC_IRQ_DISABLE()
#elif defined(__ML507__)
# define HAL_IRQ_DISABLE()                  ML507_IRQ_DISABLE()
#else /* __i386__ */
# define HAL_IRQ_DISABLE()                  PC_IRQ_DISABLE()
#endif


/**//**
 * \brief   Returns CPU Status Register.
 * \return  Current value of the CPU Status Register.
 * \details This macro shall be used prior to HAL_IRQ_DISABLE() at the beginning of an
 *  atomic section in order to save the current state of the global Interrupt Enable flag
 *  and restore it at the end of the atomic section with HAL_IRQ_SET_STATUS().
 */
#if defined(__SoC__)
# define HAL_IRQ_GET_STATUS()               SOC_IRQ_GET_STATUS()
#elif defined(__ML507__)
# define HAL_IRQ_GET_STATUS()               ML507_IRQ_GET_STATUS()
#else /* __i386__ */
# define HAL_IRQ_GET_STATUS()               PC_IRQ_GET_STATUS()
#endif


/**//**
 * \brief   Assigns CPU Status Register.
 * \param[in]   status      New value for the CPU Status Register.
 * \details This macro shall be used in pair with the HAL_IRQ_GET_STATUS().
 */
#if defined(__SoC__)
# define HAL_IRQ_SET_STATUS(status)         SOC_IRQ_SET_STATUS(status)
#elif defined(__ML507__)
# define HAL_IRQ_SET_STATUS(status)         ML507_IRQ_SET_STATUS(status)
#else /* __i386__ */
# define HAL_IRQ_SET_STATUS(status)         PC_IRQ_SET_STATUS(status)
#endif


/**//**
 * \brief   Initializes the Interrupt Controller hardware.
 * \details This function configures which and how hardware interrupts shall be processed.
 * \note    This function does not enables interrupts processing.
 */
#if defined(__SoC__)
# define HAL_IrqCtrlInit()                  SOC_IrqCtrlInit()
#elif defined(__ML507__)
# define HAL_IrqCtrlInit()                  ML507_IrqCtrlInit()
#else /* __i386__ */
# define HAL_IrqCtrlInit()                  PC_IrqCtrlInit()
#endif


#endif /* _BB_HAL_IRQ_CTRL_H */