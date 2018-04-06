/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"

#include <ctype.h>
#include <stdlib.h>

#if defined(ANDROID) || defined(USE_ANDROID)
#include <cutils/properties.h>
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

static unsigned char sanitize(unsigned char c)
{
#if defined(ANDROID) || defined(USE_ANDROID)
   if (!(isalnum(c) || c == '.')) c = '_';
   else if (isupper(c)) c = tolower(c);
#else
   if (!isalnum(c)) c = '_';
   else if (islower(c)) c = toupper(c);
#endif
   return c;
}

static int compose_key(char *newkey, size_t newkey_sz, const char *oldkey)
{
    char *keycopy_d;
    const char *keycopy_s;
    const char *prefix = "";

#if defined(ANDROID) || defined(USE_ANDROID)
    // Ensure Android properties start "ro.v3d."
    prefix = "ro.v3d.";
#endif

    if (strlen(oldkey) + strlen(prefix) + 1 > newkey_sz) {
        return -1;
    }

    /* Make a copy of the key, attaching the prefix, and sanitizing
       the alphabet */

    keycopy_d = newkey;

    strcpy(keycopy_d, prefix);
    keycopy_d += strlen(prefix);

    for(keycopy_s = oldkey; *keycopy_s; keycopy_s++) {
       char c;

       c = sanitize(*keycopy_s);
       *keycopy_d++ = c;
    }
    *keycopy_d = '\0';

    return 0;
}

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
int vcos_property_get(const char *key, char *value, size_t value_bufsz, const char *default_value)
{
    char env_varname[100];
    const char *env_val;
    int s;

    if (key == NULL || value == NULL || value_bufsz < 1) {
        return -1;
    }
    s = compose_key(env_varname, sizeof(env_varname), key);
    if (s < 0) {
       return -1;
    }

#if defined(ANDROID) || defined(USE_ANDROID)
    if (value_bufsz >= PROPERTY_VALUE_MAX)
    {
       return property_get(env_varname, value, default_value);
    }
    else
    {
       char tmp[PROPERTY_VALUE_MAX];
       property_get(env_varname, tmp, default_value);
       return vcos_safe_strcpy(value, tmp, value_bufsz, 0);
    }
#else
    /* Lookup in environment or use default (or empty) */
    env_val = getenv(env_varname);
    if (env_val == NULL) env_val = default_value ? default_value : "";
    return vcos_safe_strcpy(value, env_val, value_bufsz, 0);
#endif
}

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
int vcos_property_set(const char *key, const char *value)
{
    char env_varname[100];
    int s = compose_key(env_varname, sizeof(env_varname), key);
    if (s < 0) {
       return -1;
    }

#if defined(ANDROID) || defined(USE_ANDROID)
    if (!value) value = "";
    return property_set(env_varname, value) == 0 ? 0 : -1;
#elif defined(UNIX)
    if ((value == NULL) ||
        (strlen(value) == 0)) {
        return unsetenv(env_varname);
    } else {
        return setenv(env_varname, value, 1);
    }
#else
    return -1;
#endif
}

/*
 * Initialize properties library
 *
 * Opens, reads, etc. persistent storage for properties and
 * makes them ready for access by the get and set routines.
 *
 * Return - Zero success, -1 on failure.
 */
int vcos_property_init(void)
{
    return 0;
}

/*
 * Close properties library
 *
 * Closes and releases any resources used by the properties
 * library.
 */
void vcos_property_close(void)
{
}
