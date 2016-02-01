/****************************************************************************
 *                Copyright (c) 2013 Broadcom Corporation                   *
 *                                                                          *
 *      This material is the confidential trade secret and proprietary      *
 *      information of Broadcom Corporation. It may not be reproduced,      *
 *      used, sold or transferred to any third party without the prior      *
 *      written consent of Broadcom Corporation. All rights reserved.       *
 *                                                                          *
 ****************************************************************************/

#ifndef _COMMON_H_
#define _COMMON_H_

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */


/*
 * Common definitions
 */
#define __unused    __attribute__((unused))


#ifndef MIN
#  define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) > (b) ? (a) : (b))
#endif

#define NUM_ELEMS(array)    (sizeof(array) / sizeof(array[0]))


#define MALLOC_OR_FAIL(variable, type, size, error_prefix)  \
do                                                          \
{                                                           \
    size_t the_size = size;                                 \
    variable = (type) malloc(the_size);                     \
    if(variable == NULL)                                    \
        FATAL_ERROR(error_prefix "malloc failed while trying to allocate %zu bytes", the_size); \
} while(0)


#endif  /* _COMMON_H_ */
