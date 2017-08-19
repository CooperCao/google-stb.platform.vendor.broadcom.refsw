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
*       Trust Center policy settings (as ZigBee Document 085195r01
*       "ZigBee Pro Trust Center Best Practices" R01 specifies) public interface
*
*******************************************************************************/

#ifndef _BB_ZBPRO_TC_POLICY_H
#define _BB_ZBPRO_TC_POLICY_H

/************************* INCLUDES *****************************************************/
#include "bbSysTypes.h"

/***************************** DEFINITIONS **********************************************/
/**//**
 * \brief Initializer for OPERATIONAL mode policy
 */
#define ZBPRO_TC_OPERATIONAL_MODE_SEC_POLICY \
    { \
        .allowJoins = true, \
        .allowAnyJoiningDevice = false, \
        .transportKeyMethod = ZBPRO_TC_TRANSPORTKEYMETHOD_LINKKEY_ENCRYPTION \
    }

/**//**
 * \brief Initializer for COMMISSIONING mode policy
 */
#define ZBPRO_TC_COMMISSIONING_MODE_SEC_POLICY \
    { \
        .allowJoins = true, \
        .allowAnyJoiningDevice = true, \
        .transportKeyMethod = ZBPRO_TC_TRANSPORTKEYMETHOD_NO_ENCRYPTION \
    }

/***************************** TYPES ****************************************************/

/**//**
 * \brief Indicates whether the Trust Center is currently allowing devices to join the network
 */
typedef bool ZBPRO_TC_AllowJoins_t;

/**//**
 * \brief Indicates whether the Trust Center allows any device with any IEEE address
 */
typedef bool ZBPRO_TC_AllowAnyJoiningDevice_t;  /*!< true, because Permission Table is optional by PICS */

/**//**
 * \brief How the network key is transported to a device that is joining
 *        or rejoining without network security
 * \ingroup ZBPRO_TC_Misc
 */
typedef enum _ZBPRO_TC_TransportKeyMethod_t
{
    ZBPRO_TC_TRANSPORTKEYMETHOD_NO_ENCRYPTION       = 0,
    ZBPRO_TC_TRANSPORTKEYMETHOD_LINKKEY_ENCRYPTION  = 1,
    ZBPRO_TC_TRANSPORTKEYMETHOD_NWK_KEY_ENCRYPTION  = 2
} ZBPRO_TC_TransportKeyMethod_t;

/**//**
 * \brief Trust Center operating mode
 * \ingroup ZBPRO_TC_Misc
 */
typedef enum _ZBPRO_TC_Mode_t
{
    ZBPRO_TC_MODE_OPERATIONAL   = 0,
    ZBPRO_TC_MODE_COMMISSIONING = 1,
    ZBPRO_TC_MODE_NUM
} ZBPRO_TC_Mode_t;

/**//**
 * \brief How long the Trust Center will remain in commissioning mode
 */
typedef uint16_t ZBPRO_TC_CommissioningModeTimeout_t;   /*!< seconds */

/**//**
 * \brief TC Policy Settings
 * \ingroup ZBPRO_TC_Misc
 */
typedef struct _ZbProTcPolicy_t
{
    ZBPRO_TC_Mode_t                     mode;                      /*!< TC mode */
    ZBPRO_TC_CommissioningModeTimeout_t commissioningModeTimeout;  /*!< Timeout in seconds */

    struct
    {
        ZBPRO_TC_AllowJoins_t               allowJoins;            /*!< Does allow join? */
        ZBPRO_TC_AllowAnyJoiningDevice_t    allowAnyJoiningDevice; /*!< True, because Permission Table is optional */
        ZBPRO_TC_TransportKeyMethod_t       transportKeyMethod;    /*!< Transport key method */
    } modePolicy[ZBPRO_TC_MODE_NUM];                               /*!< Array of modes */
} ZbProTcPolicy_t;

#endif /* _BB_ZBPRO_TC_POLICY_H */

/* eof bbZbProTcPolicy.h */