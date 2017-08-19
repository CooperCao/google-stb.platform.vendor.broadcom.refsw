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

/******************************************************************************
*
* DESCRIPTION:
*       APSME Security Services data types definitions.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_APS_SAP_SECURITY_TYPES_H
#define _BB_ZBPRO_APS_SAP_SECURITY_TYPES_H


/************************* INCLUDES *****************************************************/
#include "bbZbProApsKeywords.h"     /* ZigBee PRO APS macro keywords definition. */
#include "bbZbProApsCommon.h"       /* ZigBee PRO APS general types definitions. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief Key material types enumeration.
 * \ingroup ZBPRO_APS_Misc
 */
typedef enum _ZBPRO_APS_KeyType_t
{
    ZBPRO_APS_TRUST_CENTER_MASTER_KEY      = 0x00,      /*!< A master key used to set up link keys between the
                                                            Trust Center and another device. */

    ZBPRO_APS_STANDARD_NETWORK_KEY         = 0x01,      /*!< A network key to be used in standard security mode
                                                            and may be distributed using key-transport
                                                            or a standard network key. */

    ZBPRO_APS_APPLICATION_MASTER_KEY       = 0x02,      /*!< A master key used to set up link keys
                                                            between two devices. */

    ZBPRO_APS_APPLICATION_LINK_KEY         = 0x03,      /*!< A link key used as a basis of security
                                                            between two devices. */

    ZBPRO_APS_TRUST_CENTER_LINK_KEY        = 0x04,      /*!< A link key used as a basis for security
                                                            between the Trust Center and another device. */

    ZBPRO_APS_HIGH_SECURITY_NETWORK_KEY    = 0x05,      /*!< A unique, pair-wisae network key to be used
                                                            in high security mode and may be distributed
                                                            using keytransport only. */
} ZBPRO_APS_KeyType_t;

/**//**
 * \brief Possible device initial security states. See ZigBee Spec r20, 4.6.3.2.3.
 * \ingroup ZBPRO_APS_Misc
 */
typedef enum _ZBPRO_APS_SecurityInitialStatus_t
{
    ZBPRO_APS_PRECONFIGURED_NETWORK_KEY             = 0x00,
    ZBPRO_APS_PRECONFIGURED_TRUST_CENTER_KEY        = 0x01,
    ZBPRO_APS_PRECONFIGURED_TRUST_CENTER_MASTER_KEY = 0x02, /*!< \note Not supported */
    ZBPRO_APS_NOT_PRECONFIGURED                     = 0x03  /*!< \note Not supported */
} ZBPRO_APS_SecurityInitialStatus_t;

/**//**
 * \brief APSME Security Services confirmation parameters data structure.
 * \ingroup ZBPRO_APS_Misc
 * \note According to the Standard there is no confirmation assumed on APSME Security
 *  Services Requests. Nevertheless, this data structure is used by the current
 *  implementation to issue confirmation status parameter that shows at least successful
 *  or unsuccessful result from NWK layer.
 */
typedef struct _ZBPRO_APS_SecurityServicesConfParams_t
{
    ZBPRO_APS_Status_t  status;     /*!< The result of the attempt to perform
                                        APSME Security Service Request operation. */
} ZBPRO_APS_SecurityServicesConfParams_t;

#endif /* _BB_ZBPRO_APS_SAP_SECURITY_TYPES_H */

/* eof bbZbProApsSapSecurityTypes.h */