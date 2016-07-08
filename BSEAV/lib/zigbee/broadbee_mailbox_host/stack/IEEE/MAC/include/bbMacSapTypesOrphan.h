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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/bbMacSapTypesOrphan.h $
*
* DESCRIPTION:
*   MLME-ORPHAN service data types definition.
*
* $Revision: 3402 $
* $Date: 2014-08-26 14:23:56Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_SAP_TYPES_ORPHAN_H
#define _BB_MAC_SAP_TYPES_ORPHAN_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapTypesCommStatus.h"    /* MLME-COMM-STATUS service data types. */
#include "bbMacSapService.h"            /* MAC-SAP service data types. */


/************************* VALIDATIONS **************************************************/
#if !defined(_MAC_CONTEXT_ZBPRO_)
# error This header shall be compiled only if the ZigBee PRO context is included into the build.
#endif


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for parameters of the MLME-ORPHAN.indication.
 * \note    Security parameters are excluded because the MAC Security is not implemented.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.1.8.1.1, table 61.
 */
typedef struct _MAC_OrphanIndParams_t
{
    /* 64-bit data. */
    MAC_Addr64bit_t  orphanAddress;     /*!< The 64-bit address of the orphaned device. */

} MAC_OrphanIndParams_t;


/**//**
 * \brief   Template for the callback handler-function of the MLME-ORPHAN.indication.
 * \param[in]   indParams   Pointer to the indication parameters object.
 * \details Call function of this type provided by the higher layer from the MAC to issue
 *  the MLME-ORPHAN.indication to the destination ZigBee PRO higher layer. The indication
 *  callback handler-function must be statically linked in the project.
 * \details Treat the parameters structure pointed by the \p indParams and passed into the
 *  indication callback handler-function as it has been allocated in the program stack by
 *  the MAC before calling this callback function and will be destroyed just after this
 *  callback function returns.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.1.8.1.
 */
typedef void MAC_OrphanIndCallback_t(MAC_OrphanIndParams_t *const indParams);


/**//**
 * \brief   Structure for parameters of the MLME-ORPHAN.response.
 * \note    The following standard parameters are excluded due to the following reasons:
 *  - AssociatedMember      This parameter is assumed to be TRUE when the higher layer
 *      issues the MLME-ORPHAN.response to the MAC. According to the specification, if
 *      this parameter equals FALSE, the MAC shall ignore the MLME-ORPHAN.response from
 *      the higher layer and shall not issue the MLME-COMM-STATUS.indication back to the
 *      higher layer. That is equivalent to the case when the higher layer simply ignores
 *      the MLME-ORPHAN.indication received from the MAC without any negative
 *      MLME-ORPHAN.response to the MAC.
 *
 * \note    Security parameters are excluded because the MAC Security is not implemented.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.1.8.2.1, table 62.
 */
typedef struct _MAC_OrphanRespParams_t
{
    /* 64-bit data. */
    MAC_Addr64bit_t  orphanAddress;     /*!< The 64-bit address of the orphaned device. */

    /* 16-bit data. */
    MAC_Addr16bit_t  shortAddress;      /*!< The 16-bit short address allocated to the orphaned device if it is
                                            associated with this coordinator. */
} MAC_OrphanRespParams_t;


/**//**
 * \brief   Structure for parameters of the MLME-COMM-STATUS-ORPHAN.indication.
 * \details This is a particular case of the common MLME-COMM-STATUS.indication when it is
 *  issued by the MAC to the higher layer as a confirmation on a previous
 *  MLME-ORPHAN.response.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.1.8.2, 7.1.12.1, table 69.
 */
typedef MAC_CommStatusIndParams_t  MAC_CommStatusOrphanIndParams_t;


/**//**
 * \brief   Structure for descriptor of the MLME-ORPHAN.response.
 */
typedef struct _MAC_OrphanRespDescr_t  MAC_OrphanRespDescr_t;


/**//**
 * \brief   Template for the callback handler-function of the
 *  MLME-COMM-STATUS-ORPHAN.indication.
 * \param[in]   respDescr   Pointer to the confirmed response descriptor.
 * \param[in]   indParams   Pointer to the indication parameters object.
 * \details Call function of this type provided by the higher layer from the MAC to issue
 *  the MLME-COMM-STATUS.indication to the ZigBee PRO higher layer that originally issued
 *  the MLME-ORPHAN.response primitive to the MAC.
 * \details To issue the indication primitive the MAC calls the indication callback
 *  handler-function that was specified with the \c callback parameter of the original
 *  response primitive descriptor that is pointed here by the \p respDescr parameter.
 * \details The response descriptor object that was originally used to issue response to
 *  the MAC and is pointed here with the \p respDescr is released by the MAC for random
 *  use by the higher layer (the owner of the response descriptor object) when this
 *  indication callback handler-function is called by the MAC.
 * \details Treat the parameters structure pointed by the \p indParams and passed into the
 *  indication callback handler-function as it has been allocated in the program stack by
 *  the MAC before calling this callback function and will be destroyed just after this
 *  callback function returns.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.1.8.2, 7.1.12.1.
 */
typedef void MAC_CommStatusOrphanIndCallback_t(MAC_OrphanRespDescr_t           *const respDescr,
                                               MAC_CommStatusOrphanIndParams_t *const indParams);


/**//**
 * \brief   Structure for descriptor of the MLME-ORPHAN.response.
 */
struct _MAC_OrphanRespDescr_t
{
    /* 32-bit data. */
    MAC_CommStatusOrphanIndCallback_t *callback;        /*!< Entry point of the confirmation callback function. */

    /* Structured data. */
    MacServiceField_t                  service;         /*!< MAC requests service field. */

    MAC_OrphanRespParams_t             params;          /*!< Response parameters structured object. */
};


#endif /* _BB_MAC_SAP_TYPES_ORPHAN_H */