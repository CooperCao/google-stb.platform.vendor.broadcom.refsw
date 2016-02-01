/******************************************************************************
 * (c) 2007-2015 Broadcom Corporation
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
 *
 *****************************************************************************/
#ifndef BIP_CONSOLE_H
#define BIP_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bip_priv.h"

typedef struct BIP_Console *  BIP_ConsoleHandle;

/**
 * Summary:
 * BIP Console APIs
 *
 * Description:
 * Starting, stopping and interacting with the BIP telnet console .
 **/

/**
 * Summary:
 * This structure defines create settings for BIP_Console.
 **/
typedef struct BIP_ConsoleCreateSettings
{
    BIP_SETTINGS(BIP_ConsoleCreateSettings)   /* Internal use... for init verification. */

    const char *pPort;                          /*!< Port on which HttpServer Listens for HTTP Requests from clients. */
                                                /*!< Defaults to port 80. */
    const char *pInterfaceName;                 /*!< Binds Media Server to this specific interface name and listens for HTTP Request exclusively on that interface. */
                                                /*!< Defaults to none, meaning HttpServer will listen on all interfaces! */
    BIP_NetworkAddressType ipAddressType;       /*!< If pInterfaceName is specified, then ipAddressType allows caller */
                                                /*!< To specify its perference about using the IP Address associated with this interface. */
                                                /*!< Apps can restrict to use just IPv4, IPv6, both IPv4 & IPv6, or IPv6 if available, otherwise IPv4 type of addresses. */
                                                /*!< This allows one instance of HttpServer to receive HTTP Requests on any type of IP Address. */
    B_ThreadFunc  consoleAppThreadFunction;
    void         *pConsoleAppThreadParam;

} BIP_ConsoleCreateSettings;
BIP_SETTINGS_ID_DECLARE(BIP_ConsoleCreateSettings);


#define BIP_CONSOLE_DEFAULT_LISTENING_PORT  "26" /* Linux telnet server is on port 23. */


#define BIP_Console_GetDefaultCreateSettings(pSettings)                           \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_ConsoleCreateSettings) \
        /* Set non-zero defaults explicitly. */                                   \
        (pSettings)->pPort = BIP_CONSOLE_DEFAULT_LISTENING_PORT;                  \
        BIP_SETTINGS_GET_DEFAULT_END

/**
 * Summary:
 * API to create a BIP_Console.
 *
 * Description:
 *
 * A BIP_Console is a telnet server that listens on a specified port for a connection
 * from a telnet client.
 **/

BIP_ConsoleHandle BIP_Console_Create(const BIP_ConsoleCreateSettings *pSettings);

/**
 * Summary:
 * API to destroy a BIP_Console.
 *
 * Description:
 *
 * If a BIP_Console is running but is no longer needed, the following API can be used to
 * destroy it.
 **/
void BIP_Console_Destroy(BIP_ConsoleHandle  hConsole);

/* ********************************************************************************************** */

#define BIP_CONSOLE_PRINTF_FMT  \
    "[hConsole=%p Port=%s Iface=%s Type=%s hListener=%p]"

#define BIP_CONSOLE_PRINTF_ARG(pObj)                                                                  \
    (pObj),                                                                                           \
    (pObj)->createSettings.pPort          ? (pObj)->createSettings.pPort          : "NULL",           \
    (pObj)->createSettings.pInterfaceName ? (pObj)->createSettings.pInterfaceName : "NULL",           \
    (pObj)->createSettings.ipAddressType==BIP_NetworkAddressType_eIpV4           ? "IpV4"          :  \
    (pObj)->createSettings.ipAddressType==BIP_NetworkAddressType_eIpV6           ? "IpV6"          :  \
    (pObj)->createSettings.ipAddressType==BIP_NetworkAddressType_eIpV6_and_IpV4  ? "IpV6_and_IpV4" :  \
    (pObj)->createSettings.ipAddressType==BIP_NetworkAddressType_eIpV6_over_IpV4 ? "IpV6_over_IpV4":  \
                                                                                  "<undefined>",      \
    (pObj)->hListener

#ifdef __cplusplus
}
#endif

#endif /* BIP_CONSOLE_H */
