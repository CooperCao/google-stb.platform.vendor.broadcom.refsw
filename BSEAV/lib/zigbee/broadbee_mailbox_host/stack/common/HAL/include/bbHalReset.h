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
* FILENAME: $Workfile: trunk/stack/common/HAL/include/bbHalReset.h $
*
* DESCRIPTION:
*   Hardware Reset interface.
*
* $Revision: 3943 $
* $Date: 2014-10-07 20:55:38Z $
*
*****************************************************************************************/


#ifndef _BB_HAL_RESET_H
#define _BB_HAL_RESET_H


/************************* INCLUDES *****************************************************/
#include "bbSysBasics.h"            /* Basic system environment set. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Performs the software restart.
 */
#if defined(__SoC__)
# define HAL_RestartSw(asFactoryNew)     SOC_RestartSw(asFactoryNew)
#
#elif defined(__ML507__)
# define HAL_RestartSw(asFactoryNew)     ML507_RestartSw(asFactoryNew)
#
#else /* __i386__ */
# define HAL_RestartSw(asFactoryNew)     PC_RestartSw(asFactoryNew)
#
#endif

/**//**
 * \brief Returns a kind of restart it was.
 * \return True if it is set to a factory new and false otherwise.
 */
#if defined(__SoC__)
# define HAL_GetZbProResetType()    SOC_GetZbProResetType()
#
#elif defined(__ML507__)
# define HAL_GetZbProResetType()    ML507_GetZbProResetType()
#
#else /* __i386__ */
# define HAL_GetZbProResetType()    PC_GetZbProResetType()
#
#endif

/**//**
 * \brief   Performs the hardware reset after power-on or restart.
 */
#if defined(__SoC__)
# define HAL_ResetHw()          SOC_ResetHw()
#
#elif defined(__ML507__)
# define HAL_ResetHw()          ML507_ResetHw()
#
#else /* __i386__ */
# define HAL_ResetHw()          PC_ResetHw()
#
#endif


/**//**
 * \brief   Performs the hardware initial configuration after reset.
 */
#if defined(__SoC__)
# define HAL_ConfigHw()         SOC_ConfigHw()
#
#elif defined(__ML507__)
# define HAL_ConfigHw()         ML507_ConfigHw()
#
#else /* __i386__ */
# define HAL_ConfigHw()         PC_ConfigHw()
#
#endif


/************************* INCLUDES *****************************************************/
#if defined(__SoC__)
# include "bbSocReset.h"            /* SoC Hardware Reset interface. */
#elif defined(__ML507__)
# include "bbMl507Reset.h"          /* ML507 Hardware Reset interface. */
#else /* __i386__ */
# include "bbPcReset.h"             /* i386 Simulator Reset interface. */
#endif


#endif /* _BB_HAL_RESET_H */