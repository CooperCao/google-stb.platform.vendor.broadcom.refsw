/*
 * Math component interface file.
 * API for 32 bit calculation.
 * API for combined 32 bit/64 bit calculation is exported by math64.h.
 *
 * $Copyright (C) 2016 Broadcom Corporation$
 *
 * $Id:$
 */

#ifndef _math32_h_
#define _math32_h_

#include <typedefs.h>


/* Calculates a number of bits in 32 bit integer number.
 *
 * value - 32 bit signed integer number
 * return - the number of bits in value
 */
uint8 math_nbits_32(int32 value);

/* A simple implementation of gcd (greatest common divisor) assuming
 * argument 1 is bigger than argument 2, both of them are positive numbers.
 *
 * bigger - the bigger unsigned 32 bit integer
 * smaller - the smaller unsigned 32 bit integer
 * return - the result gcd(value)
 */
uint32 math_gcd_32(uint32 bigger, uint32 smaller);

/* Calculates square root of 32 bit integer number.
 *
 * value - 32 bit unsigned integer number
 * return - 32 bit result of sqrt(value)
 */
uint32 math_sqrt_int_32(uint32 value);

/**/
uint32 math_qdiv_32(uint32 dividend, uint32 divisor, uint8 precision, bool round);

/**/
uint32 math_qdiv_roundup_32(uint32 dividend, uint32 divisor, uint8 precision);


/* ============================================
 * Fixed point routines
 * ============================================ */

/* Finds the number of bits available for shifting in unsigned 32 bit number. */
uint8 math_fp_calc_head_room_32(uint32 num);

/* Does unsigned 32 bit fixed point floor */
uint32 math_fp_floor_32(uint32 num, uint8 floor_pos);

/* Does unsigned 32 bit fixed point rounding */
uint32 math_fp_round_32(uint32 num, uint8 rnd_pos);


#endif /* _math32_h_ */
