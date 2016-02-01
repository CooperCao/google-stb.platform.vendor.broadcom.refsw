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
* FILENAME: $Workfile: trunk/stack/common/System/include/bbSysAtomic.h $
*
* DESCRIPTION:
*   Atomic sections toolset interface.
*
* $Revision: 3046 $
* $Date: 2014-07-24 20:36:50Z $
*
*****************************************************************************************/


#ifndef _BB_SYS_ATOMIC_H
#define _BB_SYS_ATOMIC_H


/************************* INCLUDES *****************************************************/
#include "bbSysAtomicUids.h"        /* Atomic sections UIDs enumeration. */
#include "bbHalIrqCtrl.h"           /* Interrupt Controller Hardware interface. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Starts profiling of an atomic section.
 * \param[in]   atomicSectionUid    Global numeric identifier of the atomic section.
 */
#if defined(_PROFILE_)
# define ATOMIC_PROFILE_START(atomicSectionUid)     sysAtomicProfileStart(atomicSectionUid)
#else
# define ATOMIC_PROFILE_START(atomicSectionUid)
#endif


/**//**
 * \brief   Finishes profiling of an atomic section.
 * \param[in]   atomicSectionUid    Global numeric identifier of the atomic section.
 */
#if defined(_PROFILE_)
# define ATOMIC_PROFILE_FINISH(atomicSectionUid)    sysAtomicProfileFinish(atomicSectionUid)
#else
# define ATOMIC_PROFILE_FINISH(atomicSectionUid)
#endif


/**//**
 * \brief   Disables interrupts and starts profiling at beginning of an atomic section.
 * \param[in]   atomicSectionUid    Global numeric identifier of the atomic section.
 */
#if defined(__arc__)
# define ATOMIC_SECTION_HEADER(atomicSectionUid)\
        uint32_t __status32_copy = HAL_IRQ_GET_STATUS();\
        HAL_IRQ_DISABLE();\
        ATOMIC_PROFILE_START(atomicSectionUid);
#else
# define ATOMIC_SECTION_HEADER(atomicSectionUid)\
        TEST_EnterAtomicSection();
#endif


/**//**
 * \brief   Restores interrupts and finishes profiling at the end of an atomic section.
 * \param[in]   atomicSectionUid    Global numeric identifier of the atomic section.
 */
#if defined(__arc__)
# define ATOMIC_SECTION_FOOTER(atomicSectionUid)\
        ATOMIC_PROFILE_FINISH(atomicSectionUid);\
        HAL_IRQ_SET_STATUS(__status32_copy);
#else
# define ATOMIC_SECTION_FOOTER(atomicSectionUid)\
        TEST_LeaveAtomicSection();
#endif


/**//**
 * \brief   Starts an atomic section.
 * \param[in]   atomicSectionUid    Global numeric identifier of the atomic section.
 * \details Binds a code block into the atomic (thread-safe) section.
 */
#define ATOMIC_SECTION_ENTER(atomicSectionUid)\
        {\
            ATOMIC_SECTION_HEADER(atomicSectionUid)\
            do\
            {


/**//**
 * \brief   Finishes an atomic section.
 * \param[in]   atomicSectionUid    Global numeric identifier of the atomic section.
 * \details Binds a code block into the atomic (thread-safe) section.
 */
#define ATOMIC_SECTION_LEAVE(atomicSectionUid)\
            } while(0);\
            ATOMIC_SECTION_FOOTER(atomicSectionUid)\
        }


/************************* PROTOTYPES ***************************************************/
/*
 * Functions for profiling time spent in atomic sections for ARC platform.
 */
#if defined(_PROFILE_)
/*************************************************************************************//**
 * \brief   Starts profiling of an atomic section.
 * \param[in]   atomicSectionUid    Atomic section identifier.
*****************************************************************************************/
SYS_PUBLIC void sysAtomicProfileStart(const uint32_t atomicSectionUid);


/*************************************************************************************//**
 * \brief   Finishes profiling of an atomic section.
 * \param[in]   atomicSectionUid    Atomic section identifier.
*****************************************************************************************/
SYS_PUBLIC void sysAtomicProfileFinish(const uint32_t atomicSectionUid);
#endif /* _PROFILE_ */


/*
 * Functions for implementing simulation of atomization on different platforms.
 */
#if !defined(__arc__)
/*************************************************************************************//**
 * \brief   Simulates starting of profiling of an atomic section.
*****************************************************************************************/
extern void TEST_EnterAtomicSection(void);


/*************************************************************************************//**
 * \brief   Simulates finishing of profiling of an atomic section.
*****************************************************************************************/
extern void TEST_LeaveAtomicSection(void);
#endif /* ! __arc__ */


#endif /* _BB_SYS_ATOMIC_H */