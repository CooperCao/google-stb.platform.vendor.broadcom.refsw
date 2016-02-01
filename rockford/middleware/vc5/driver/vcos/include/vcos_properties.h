/*****************************************************************************
 (c)2011 Broadcom Corporation.  All rights reserved.

 This program is the proprietary software of Broadcom Corporation and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").

 Except as set forth in an Authorized License, Broadcom grants no license
 (express or implied), right to use, or waiver of any kind with respect to the
 Software, and Broadcom expressly reserves all rights in and to the Software
 and all intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED
 LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

  Except as expressly set forth in the Authorized License,
 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use all
    reasonable efforts to protect the confidentiality thereof, and to use this
    information only in connection with your use of Broadcom integrated
    circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION.
    YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE
    SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#ifndef VCOS_PROP_H
#define VCOS_PROP_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file
 *
 * Properties
 *
 * This provides support for a "C" string based key/value
 * library for persistent storage of configuration "properties"
 *
 * Actual storage of the properties is target dependent and
 * may use APIs specific to the target.
 *
 */


/*
 * Get Property Value
 *
 * Parameters:
 *    key           - The key name for the desired property.
 *    value         - If successful, the value of the property.
 *                    Memory to be supplied by caller.
 *    value_bufsz   - Number of bytes the caller has allowed
 *                    for the value (including string terminator)
 *    default_value - If non-NULL, a value to use if the key
 *                    does not exist.
 *
 * If the key exists in storage, the value of the key is return
 * with a success indication.
 *
 * If the key does not exist but a non-NULL default_value was
 * given, then the default value is returned with a success
 * indication.
 *
 * Return - Full length of value. If the length is greater than value_bufsz
 * the buffer will contain truncated value terminated with NULL.
 * Return -1 on failure or zero on no value.
 */
int vcos_property_get(const char *key, char *value, size_t value_bufsz, const char *default_value);

/*
 * Set Property Value
 *
 * Parameters:
 *    key        - The name of the property
 *    value      - The value to set the property to
 *
 * Sets the property specified by the key name to the value given.
 * Failure should not occur.
 *
 * Return - Zero success, -1 on failure.
 */
int vcos_property_set(const char *key, const char *value);

/*
 * Initialize properties library
 *
 * Opens, reads, etc. persistent storage for properties and
 * makes them ready for access by the get and set routines.
 *
 * Return - Zero success, -1 on failure.
 */
int vcos_property_init(void);

/*
 * Close properties library
 *
 * Closes and releases any resources used by the properties
 * library.
 */
void vcos_property_close(void);

#ifdef __cplusplus
}
#endif

#endif /* VCOS_PROP_H */
