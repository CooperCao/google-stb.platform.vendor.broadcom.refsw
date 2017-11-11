/****************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ****************************************************************************/

/**
 * @file
 * @ingroup libdspcontrol
 * @brief Support for FirePath core interactive debug.
 *
 * The DBG module bridges between the user debugger (firepath-elf-gdb) and
 * the Debug Agent living in FirePath cores. Usually each DBG instance serves
 * a single FP core.
 *
 * A DBG instance must be initialised using DBG_init prior of being used. The
 * provided DBG_PARAMETERS are dependent on the specific target SoC and the
 * library built configuration. Once initialised, a DBG instance must be
 * periodically serviced using DBG_service. DBG_finish closes and releases
 * all DBG instance resources.
 */

#ifndef _DBG_H_
#define _DBG_H_

#include <stdint.h>
#ifndef __cplusplus
#  include <stdbool.h>
#endif

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/UTIL_c.h"

#if FEATURE_IS(DBG_TARGET_IF, DEBUG_CONSOLE) || \
    FEATURE_IS(DBG_TARGET_IF, MISC_BLOCK) || \
    FEATURE_IS(DBG_TARGET_IF, MISC_BLOCK_BUFFER)
#include "libdspcontrol/DSP.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif


/**
 * Enumeration of the possible return values of DBG functions.
 */
typedef enum
{
    DBG_SUCCESS = 0,
    DBG_FAILURE = 1,
} DBG_RET;

typedef enum
{
    RW_NOT_READY = 0,
    R_READY,
    W_READY
} DBG_TARGET_STATE;

typedef enum
{
    DBG_TARGET_SUCCESS = 0,
    DBG_TARGET_CANNOT_ACQUIRE_MUTEX,
    DBG_TARGET_NO_SPACE_RECV_BUFFER,
    DBG_TARGET_CANNOT_SEND_DATA,
    DBG_TARGET_CANNOT_READ_DATA,
    DBG_TARGET_RW_NOT_READY,
    DBG_TARGET_WRITE_BEFORE_READ
} DBG_TARGET_ERR_CODE;


/** Returns true if ret_value indicates a success. */
#define DBG_SUCCEEDED(ret_value)    ((ret_value) == DBG_SUCCESS)
/** Returns true if ret_value indicates a failure. */
#define DBG_FAILED(ret_value)       ((ret_value) != DBG_SUCCESS)


/**
 * Container for user-provided settings used at DBG instances initialisation.
 */
typedef struct
{
    unsigned id;                    /**< numeric ID to distinguish logs from different instances */
#if FEATURE_IS(DBG_HOST_IF, SOCKET)
    uint16_t socket_port;           /**< the TCP port the DBG server will listen at */
#endif
#if FEATURE_IS(DBG_TARGET_IF, DEBUG_CONSOLE)
    DSP     *dsp;                   /**< the DSP instance talking to the FP subsystem of interest */
    DSP_CORE dsp_core;              /**< which FP core this DBG instance talks to */
    unsigned fast_write_retries;    /**< how many times to retry writing in a row
                                         if the debug console is out of space */
    unsigned slow_write_retries;    /**< how many times to retry writing with a sleep
                                         in between if the debug console is out of space */
    unsigned slow_retry_sleep_ms;   /**< how many milliseconds to sleep between two slow retries */
#endif
#if FEATURE_IS(DBG_TARGET_IF, MISC_BLOCK) || FEATURE_IS(DBG_TARGET_IF, MISC_BLOCK_BUFFER)
    DSP         *dsp;
    DSP_CORE    dsp_core;
    unsigned    mutex_acquire_retries;
#endif
#if FEATURE_IS(DBG_TARGET_IF,SOCKET)
    uint16_t portno;
    char *hostname;
#endif

} DBG_PARAMETERS;


/*
 * DBG Target interface structure
 * FIXME:Ideally would like individual target interfaces, have the DBG_TARGET structure
 * defined within their own header file. But it will be not possible to include
 * them in a public header
 */
typedef struct
{
    unsigned id;                    /**< to distinguish between different DBG instances */
#if FEATURE_IS(DBG_TARGET_IF, DEBUG_CONSOLE) || \
    FEATURE_IS(DBG_TARGET_IF, MISC_BLOCK) || \
    FEATURE_IS(DBG_TARGET_IF, MISC_BLOCK_BUFFER)
    DSP     *dsp;                   /**< the DSP instance we are communicating with */
    DSP_CORE dsp_core;              /**< the core we are talking to */
#endif
#if FEATURE_IS(DBG_TARGET_IF, DEBUG_CONSOLE)
    unsigned fast_write_retries;    /**< how many times to retry writing in a row
                                         if the debug console is out of space */
    unsigned slow_write_retries;    /**< how many times to retry writing with a sleep
                                         in between if the debug console is out of space */
    unsigned slow_retry_sleep_ms;   /**< how many milliseconds to sleep between a slow retry and the next */
#endif
#if FEATURE_IS(DBG_TARGET_IF, LOOPBACK)
    /** @{ Overflow buffer: all the sent data that doesn't fit in the
     *     receive buffer is temporarily stored here. */
    uint8_t *overflow_buffer;
    size_t   overflow_buffer_size;
    /** @} */
#endif
#if FEATURE_IS(DBG_TARGET_IF, MISC_BLOCK)
    unsigned mutex_acquire_retries; /**< Number of times to try to acquire the misc block mutex*/
    uint8_t max_payload_size;
    DBG_TARGET_ERR_CODE target_err_code;
#endif
#if FEATURE_IS(DBG_TARGET_IF, MISC_BLOCK_BUFFER)
    unsigned mutex_acquire_retries; /**< Number of times to try to acquire the misc block mutex*/
    bool b_channel_configured;
    uint8_t *p_u8_channel_address;
    uint16_t u16_channel_size;
    DBG_TARGET_ERR_CODE target_err_code;
#endif
#if FEATURE_IS(DBG_TARGET_IF, SOCKET)
    int      sockfd;
    DBG_TARGET_ERR_CODE target_err_code;
#endif
} DBG_TARGET;


/*
 * DBG Host interface structure
 */
typedef struct
{
    unsigned id;             /**< to distinguish between different DBG instances */
#if FEATURE_IS(DBG_HOST_IF, SOCKET)
    int      server_fd;      /**< server socket file descriptor */
    int      data_fd;        /**< data socket file descriptor */
    uint16_t server_port;    /**< port the server socket is listening at;
                                  may be different from socket_port in parameters */
#endif
} DBG_HOST;

#if FEATURE_IS(DBG_CORE, DEBUG_SERVER)
typedef struct DBG_core_debug_server_sp DBG_core_debug_server;
#endif

#if FEATURE_IS(DBG_CORE, DEBUG_SERVER_MP)
typedef struct dbg_core_debug_server_mp DBG_core_debug_server ;
#endif

typedef struct
{
    unsigned id;                /**< to distinguish between different DBG instances */
#if FEATURE_IS(DBG_CORE, BRIDGE_BUFF)
    /** @{ Buffers: when one side can't receive all the data the other side has sent,
     *     data is temporarily stored here. */
    uint8_t *host2dsp_buff;     /**< buffer for host -> target data */
    size_t   host2dsp_amount;   /**< amount of bytes stored in host2dsp_buff */
    uint8_t *dsp2host_buff;     /**< buffer for target -> host data */
    size_t   dsp2host_amount;   /**< amount of bytes stored in dsp2host_buff */
    /** @} */
#endif
#if FEATURE_IS(DBG_CORE, DEBUG_SERVER) || FEATURE_IS(DBG_CORE, DEBUG_SERVER_MP)
    DBG_core_debug_server *p_dbg_server;
#endif
} DBG_CORE;

/**
 * The DBG structure.\n
 * Target/Host-specific attributes are initialised by the respective interfaces,
 * common attributes are initialised by the core.
 */
typedef struct
{
    DBG_CORE   dbg_core;        /**< DBG core module*/
    DBG_HOST   dbg_host_if;     /**< DBG interface to the host*/
    DBG_TARGET dbg_target_if;   /**< DBG interface to the target DSP*/
} DBG;


/**
 * Constructor for a DBG 'object'.
 * Once initialised, the DBG object can start being serviced.
 *
 * @param[out] dbg         the DBG instance to initialise
 * @param[in]  parameters  configuration-specific parameters used to initialise and customise
 *                         the DBG object; the #DBG_PARAMETERS pointer should remain valid until
 *                         the return from DBG_init, not for the whole life of the DBG object.
 * @return                 one of the #DBG_RET return values
 */
BFPSDK_NONNULL BFPSDK_WARN_UNUSED_RESULT
DBG_RET DBG_init(DBG *dbg, DBG_PARAMETERS *parameters);


/**
 * Destructor for a DBG 'object'. The provided DBG instance should not be used
 * anymore after the call to this function.
 *
 * @param[in] dbg  the DBG instance to terminate
 */
BFPSDK_NONNULL
void DBG_finish(DBG *dbg);


/**
 * Services the DBG instance. Available data is processed and transferred between
 * the target DSP and the host.
 *
 * @param[in] dbg  the DBG instance
 * @return         one of the #DBG_RET return values
 */
BFPSDK_NONNULL
DBG_RET DBG_service(DBG *dbg);


#ifdef __cplusplus
}
#endif


#endif  // _DBG_H_
