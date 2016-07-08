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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/bbMacKeywords.h $
*
* DESCRIPTION:
*   MAC macro keywords definition.
*
* $Revision: 2722 $
* $Date: 2014-06-24 19:37:15Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_KEYWORDS_H
#define _BB_MAC_KEYWORDS_H


/************************* INCLUDES *****************************************************/
#include "bbSysKeywords.h"          /* SYS macro keywords definition. */


/************************* DEFINITIONS **************************************************/
/*
 * MAC functions visibility attributes:
 * - MAC_PUBLIC  - function will be accessible from the outside of the MAC by any other
 *                 unit, stack layer, the end-user, or the Mailbox. Use this attribute for
 *                 all MAC-SAP functions declarations and only for them;
 * - MAC_FRIEND  - function will be accessible from the outside of the MAC by other stack
 *                 layers, but will not be accessible by the end-user and must not be used
 *                 by the Mailbox. Use this attribute for all MAC Friend API functions
 *                 declarations and only for them. The MAC Friend API is used by the
 *                 MAC-PIB to support Information Bases nesting through the stack layers;
 * - MAC_PRIVATE - function will be accessible only inside the MAC between its units, but
 *                 will not be accessible from the outside of the MAC. Use this attribute
 *                 for all MAC-FE, MAC-LE, MAC-PIB and other MAC units private interfaces
 *                 functions declarations and only for them;
 * - MAC_STATIC  - function will be accessible only inside the unit where it is declared,
 *                 but in the case of MAC tests build it becomes externally accessible.
 *
 * MAC variables visibility attributes:
 * - MAC_MEMDECL - wrapper for MAC private memory declaration;
 * - MAC_MEMDEF  - attribute for MAC private memory definition with initializer;
 * - MAC_STATIC  - attribute for pure static MAC variable that must not be accessible in
 *                 normal conditions from the outside of the unit where it is declared,
 *                 but to be externally accessible under MAC tests.
 */
#if defined(_MAKE_CC_)          /* Single CC-file build target. */
# define MAC_PUBLIC             static
# define MAC_FRIEND             static
# define MAC_PRIVATE            static
# define MAC_MEMDECL(vardecl)                                                                     /* TODO: Eliminate. */
# define MAC_MEMDEF             static                                                            /* TODO: Eliminate. */
# define MAC_STATIC             static
#
#elif defined(_MAKE_CC_MAC_)    /* Single MAC CC-file build target. */
# define MAC_PUBLIC             extern
# define MAC_FRIEND             extern
# define MAC_PRIVATE            static
# define MAC_MEMDECL(vardecl)                                                                     /* TODO: Eliminate. */
# define MAC_MEMDEF             static                                                            /* TODO: Eliminate. */
# define MAC_STATIC             static
#
#elif defined(_MAKE_TEST_MAC_)  /* All variables and functions are externally accessible. */
# define MAC_PUBLIC             extern
# define MAC_FRIEND             extern
# define MAC_PRIVATE            extern
# define MAC_MEMDECL(vardecl)   extern vardecl                                                    /* TODO: Eliminate. */
# define MAC_MEMDEF                                                                               /* TODO: Eliminate. */
# define MAC_STATIC             extern
#
#else                           /* All files are compiled separately. */
# define MAC_PUBLIC             extern
# define MAC_FRIEND             extern
# define MAC_PRIVATE            extern
# define MAC_MEMDECL(vardecl)   extern vardecl                                                    /* TODO: Eliminate. */
# define MAC_MEMDEF                                                                               /* TODO: Eliminate. */
# define MAC_STATIC             static
#
#endif


#endif /* _BB_MAC_KEYWORDS_H */