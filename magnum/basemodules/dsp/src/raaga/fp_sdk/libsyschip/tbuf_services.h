/****************************************************************************
 *                Copyright (c) 2014 Broadcom Corporation                   *
 *                                                                          *
 *      This material is the confidential trade secret and proprietary      *
 *      information of Broadcom Corporation. It may not be reproduced,      *
 *      used, sold or transferred to any third party without the prior      *
 *      written consent of Broadcom Corporation. All rights reserved.       *
 *                                                                          *
 ****************************************************************************/

/**
 * @file
 * @ingroup libsyschip
 * @brief Enumeration of the services that can be transported in a Target Buffer data stream.
 *
 * Note: this file gets included both by Firepath and Host code, so avoid dependencies
 *       to SDK headers as much as possible and macro guard DSP-specific definitions.
 */

#ifndef _TBUF_SERVICES_H_
#define _TBUF_SERVICES_H_

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "fp_sdk_config.h"



#ifdef __cplusplus
extern "C" {
#endif


/**
 * IDs of the services that can be transported in a Target Buffer data stream.
 * One of these values gets embedded in the \ref TB_header#prologue field of a #TB_header.
 */
typedef enum
{
    TB_SERVICE_RAW              = 0,
    TB_SERVICE_TARGET_PRINT     = 1,
    TB_SERVICE_STAT_PROF        = 2,
    TB_SERVICE_INSTRUMENTATION  = 3,
    TB_SERVICE_CORE_DUMP        = 4,
    TB_SERVICE_MAX              = TB_SERVICE_CORE_DUMP,
    TB_SERVICE_COUNT            = TB_SERVICE_MAX + 1
} TB_service_id;


/**
 * Macro to obtain a #TB_service_flag from the corresponding #TB_service_id.
 */
#define TB_SERVICE_FLAG_FROM_ID(service_id)     (1 << (service_id))


/**
 * Services IDs in flag form to allow or'ing.
 */
typedef enum
{
    TB_SERVICE_RAW_FLAG             = TB_SERVICE_FLAG_FROM_ID(TB_SERVICE_RAW),
    TB_SERVICE_TARGET_PRINT_FLAG    = TB_SERVICE_FLAG_FROM_ID(TB_SERVICE_TARGET_PRINT),
    TB_SERVICE_STAT_PROF_FLAG       = TB_SERVICE_FLAG_FROM_ID(TB_SERVICE_STAT_PROF),
    TB_SERVICE_INSTRUMENTATION_FLAG = TB_SERVICE_FLAG_FROM_ID(TB_SERVICE_INSTRUMENTATION),
    TB_SERVICE_CORE_DUMP_FLAG       = TB_SERVICE_FLAG_FROM_ID(TB_SERVICE_CORE_DUMP),
    TB_SERVICE_ALL                  = TB_SERVICE_FLAG_FROM_ID(TB_SERVICE_MAX + 1) - 1
} TB_service_flag;


#ifdef __cplusplus
}
#endif



#endif /* _TBUF_SERVICES_H_ */
