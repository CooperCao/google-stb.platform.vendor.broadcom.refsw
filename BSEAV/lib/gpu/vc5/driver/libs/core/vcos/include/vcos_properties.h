/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
