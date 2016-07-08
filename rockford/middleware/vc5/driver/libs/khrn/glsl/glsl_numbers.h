/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_NUMBERS_H__
#define GLSL_NUMBERS_H__

#define NUM_INT 0
#define NUM_UINT 1
#define NUM_FLOAT 2
#define NUM_INVALID 3

/* Given a ppnumber return which type of value it represents and fill in
 * 'value' with the bitpattern representing the number. If the number is
 * invalid then 'value' will be uninitialised
 */
int numlex(const char *ppnumber, uint32_t *value);

#endif
