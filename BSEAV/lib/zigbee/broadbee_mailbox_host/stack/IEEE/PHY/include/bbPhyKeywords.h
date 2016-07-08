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
* FILENAME: $Workfile: trunk/stack/IEEE/PHY/include/bbPhyKeywords.h $
*
* DESCRIPTION:
*   PHY macro keywords definition.
*
* $Revision: 2722 $
* $Date: 2014-06-24 19:37:15Z $
*
*****************************************************************************************/


#ifndef _BB_PHY_KEYWORDS_H
#define _BB_PHY_KEYWORDS_H


/************************* INCLUDES *****************************************************/
#include "bbSysKeywords.h"          /* SYS macro keywords definition. */


/************************* DEFINITIONS **************************************************/
/*
 * PHY functions visibility attributes:
 * - PHY_PUBLIC  - function will be accessible from the outside of the PHY by any other
 *                 unit (namely the MAC-LE Real-Time Dispatcher unit). Use this attribute
 *                 for all PHY-SAP functions declarations and only for them;
 * - PHY_FRIEND  - function will be accessible from the outside of the PHY by other stack
 *                 layers, but will not be accessible by the end-user and must not be used
 *                 by the Mailbox. Use this attribute for all PHY Friend API functions
 *                 declarations and only for them. The PHY Friend API is used by the
 *                 PHY-PIB to support Information Bases nesting through the stack layers;
 * - PHY_PRIVATE - function will be accessible only inside the PHY between its units, but
 *                 will not be accessible from the outside of the PHY. Use this attribute
 *                 for PHY-PIB and other PHY units private interfaces functions
 *                 declarations and only for them;
 * - PHY_STATIC  - function will be accessible only inside the unit where it is declared,
 *                 but in the case of PHY tests build it becomes externally accessible.
 *
 * PHY variables visibility attributes:
 * - PHY_MEMDECL - wrapper for PHY private memory declaration;
 * - PHY_MEMDEF  - attribute for PHY private memory definition with initializer;
 * - PHY_STATIC  - attribute for pure static PHY variable that must not be accessible in
 *                 normal conditions from the outside of the unit where it is declared,
 *                 but to be externally accessible under PHY tests.
 */
#if defined(_MAKE_CC_)          /* Single CC-file build target. */
# define PHY_PUBLIC             static
# define PHY_FRIEND             static
# define PHY_PRIVATE            static
# define PHY_MEMDECL(vardecl)                                                                     /* TODO: Eliminate. */
# define PHY_MEMDEF             static                                                            /* TODO: Eliminate. */
# define PHY_STATIC             static
#
#elif defined(_MAKE_CC_PHY_)    /* Single PHY CC-file build target. */
# define PHY_PUBLIC             extern
# define PHY_FRIEND             extern
# define PHY_PRIVATE            static
# define PHY_MEMDECL(vardecl)                                                                     /* TODO: Eliminate. */
# define PHY_MEMDEF             static                                                            /* TODO: Eliminate. */
# define PHY_STATIC             static
#
#elif defined(_MAKE_TEST_PHY_)  /* All variables and functions are externally accessible. */
# define PHY_PUBLIC             extern
# define PHY_FRIEND             extern
# define PHY_PRIVATE            extern
# define PHY_MEMDECL(vardecl)   extern vardecl                                                    /* TODO: Eliminate. */
# define PHY_MEMDEF                                                                               /* TODO: Eliminate. */
# define PHY_STATIC             extern
#
#else                           /* All files are compiled separately. */
# define PHY_PUBLIC             extern
# define PHY_FRIEND             extern
# define PHY_PRIVATE            extern
# define PHY_MEMDECL(vardecl)   extern vardecl                                                    /* TODO: Eliminate. */
# define PHY_MEMDEF                                                                               /* TODO: Eliminate. */
# define PHY_STATIC             static
#
#endif


#endif /* _BB_PHY_KEYWORDS_H */