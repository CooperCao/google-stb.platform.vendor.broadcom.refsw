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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/bbMacOptions.h $
*
* DESCRIPTION:
*   MAC options setup.
*
* $Revision: 2879 $
* $Date: 2014-07-10 11:08:01Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_OPTIONS_H
#define _BB_MAC_OPTIONS_H


/************************* INCLUDES *****************************************************/
#include "bbSysOptions.h"           /* Compiler and SYS options setup. */
#include "bbMacConfig.h"            /* MAC configuration file. */


/************************* VALIDATIONS **************************************************/
/*
 * Validate MAC entities visibility switches.
 */
#if defined(_MAKE_TEST_MAC_) && (defined(_MAKE_CC_) || defined(_MAKE_CC_MAC_))
# error MAC static entities are not accessible for tests in the case of single CC-file build target.
#endif


/*
 * Validate the Stack higher layers selection.
 */
#if !defined(_MAC_CONTEXT_ZBPRO_) && !defined(_MAC_CONTEXT_RF4CE_)
#  error At least one of two contexts, ZigBee PRO and RF4CE, shall be selected for build.
#endif


/*
 * Validate the RF4CE Stack higher layers selection.
 */
#if defined(_MAC_CONTEXT_RF4CE_CONTROLLER_) && !defined(MAILBOX_UNIT_TEST)
# if defined(_MAC_CONTEXT_RF4CE_TARGET_)
#  error Both RF4CE Target and RF4CE Controller cannot be implemented in a single device.
# elif defined(_MAC_CONTEXT_ZBPRO_)
#  error Both RF4CE Controller and ZigBee PRO cannot be implemented in a single device.
# endif
#endif


/************************* AUTO CONFIGURATION *******************************************/
/*
 * Switch the number of contexts supported by MAC.
 */
#if defined(_MAC_CONTEXT_ZBPRO_) && defined(_MAC_CONTEXT_RF4CE_)
# define _MAC_DUAL_CONTEXT_
# undef  _MAC_SINGLE_CONTEXT_
#else
# define _MAC_SINGLE_CONTEXT_
# undef  _MAC_DUAL_CONTEXT_
#endif


#endif /* _BB_MAC_OPTIONS_H */