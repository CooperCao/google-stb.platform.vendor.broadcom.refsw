/***************************************************************************
 *     Copyright (c) 2003, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Cross-platform standard inteface conformance test
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "btst.h"


volatile static uint8_t u8a, u8b, u8c;

volatile static uint8_t u8a, u8b, u8c;
volatile static int8_t i8a, i8b, i8c;

volatile static uint16_t u16a, u16b, u16c;
volatile static int16_t i16a, i16b, i16c;

volatile static uint32_t u32a, u32b, u32c;
volatile static int32_t i32a, i32b, i32c;

volatile static uint64_t u64a, u64b, u64c;
volatile static int64_t i64a, i64b, i64c;

#define SIZE_TEST(type, size) BTST_TEST(sizeof( type##a) == size)

#define INT_ARITH_TEST(width, a1, a2, op, res) do { \
    width##a = a1; width##b = a2; \
    width##c = width##a op width##b; \
    BTST_TEST( width##c == (res) ); } while(0)

BERR_Code 
BTST_EntryPoint(const BTST_TestContext *context)
{
        int error_count = 0;

        SIZE_TEST(u8, 1);
        SIZE_TEST(i8, 1);

        SIZE_TEST(u16, 2);
        SIZE_TEST(i16, 2);

        SIZE_TEST(u32, 4);
        SIZE_TEST(i32, 4);

        SIZE_TEST(u64, 8);
        SIZE_TEST(i64, 8);

        /* 16 bit integer test */
        
        /* basic operations */
        INT_ARITH_TEST(u8, 1, 1, +, 2); 
        INT_ARITH_TEST(u8, 1, 1, -, 0);
        INT_ARITH_TEST(u8, 1, 1, *, 1);
        INT_ARITH_TEST(u8, 1, 1, /, 1);
        INT_ARITH_TEST(u8, 1, 1, <<, 2);
        INT_ARITH_TEST(u8, 1, 1, >>, 0);
        INT_ARITH_TEST(u8, 1, 1, &, 1);
        INT_ARITH_TEST(u8, 1, 1, |, 1);
        INT_ARITH_TEST(u8, 1, 1, ^, 0);

        /* not overflow cases */
        INT_ARITH_TEST(u8, 127, 127, +, 254); 
        INT_ARITH_TEST(u8, 255, 1, -, 254);
        INT_ARITH_TEST(u8, 15, 16, *, 240);
        INT_ARITH_TEST(u8, 255, 5, /, 51);
        INT_ARITH_TEST(u8, 1, 7, <<, 128);
        INT_ARITH_TEST(u8, 127, 4, >>, 7);

        /* overflow cases */
        INT_ARITH_TEST(u8, 255, 255, +, 254); 
        INT_ARITH_TEST(u8, 63, 63, *, 129);

        /* bit operations */
        INT_ARITH_TEST(u8, 0x55, 0xAA, &, 0);
        INT_ARITH_TEST(u8, 0x55, 0xAA, |, 0xFF);
        INT_ARITH_TEST(u8, 0x55, 0xAA, ^, 0xFF);

        /* basic operations */
        INT_ARITH_TEST(i8, 1, 1, +, 2); 
        INT_ARITH_TEST(i8, 1, 1, -, 0);
        INT_ARITH_TEST(i8, 1, 1, *, 1);
        INT_ARITH_TEST(i8, 1, 1, /, 1);
        INT_ARITH_TEST(i8, 1, 1, <<, 2);
        INT_ARITH_TEST(i8, 1, 1, >>, 0);
        INT_ARITH_TEST(i8, 1, 1, &, 1);
        INT_ARITH_TEST(i8, 1, 1, |, 1);
        INT_ARITH_TEST(i8, 1, 1, ^, 0);

        INT_ARITH_TEST(i8, -1, -1, +, -2); 
        INT_ARITH_TEST(i8, -1, 1, +, 0);
        INT_ARITH_TEST(i8, -1, 1, *, -1);
        INT_ARITH_TEST(i8, 1, -1, /, -1);

        /* not overflow cases */
        INT_ARITH_TEST(i8, 63, 63, +, 126); 
        INT_ARITH_TEST(i8, 127, 1, -, 126);
        INT_ARITH_TEST(i8, 15, 8, *, 120);
        INT_ARITH_TEST(i8, 125, 5, /, 25);
        INT_ARITH_TEST(i8, 1, 6, <<, 64);
        INT_ARITH_TEST(i8, 63, 4, >>, 3);

        /* overflow cases */
        INT_ARITH_TEST(i8, 127, 127, *, 01); 



        /* 16 bit integer test */
        
        /* basic operations */
        INT_ARITH_TEST(u16, 1, 1, +, 2); 
        INT_ARITH_TEST(u16, 1, 1, -, 0);
        INT_ARITH_TEST(u16, 1, 1, *, 1);
        INT_ARITH_TEST(u16, 1, 1, /, 1);
        INT_ARITH_TEST(u16, 1, 1, <<, 2);
        INT_ARITH_TEST(u16, 1, 1, >>, 0);
        INT_ARITH_TEST(u16, 1, 1, &, 1);
        INT_ARITH_TEST(u16, 1, 1, |, 1);
        INT_ARITH_TEST(u16, 1, 1, ^, 0);

        /* not overflow cases */
        INT_ARITH_TEST(u16, 127, 127, +, 254); 
        INT_ARITH_TEST(u16, 255, 1, -, 254);
        INT_ARITH_TEST(u16, 15, 16, *, 240);
        INT_ARITH_TEST(u16, 255, 5, /, 51);
        INT_ARITH_TEST(u16, 1, 7, <<, 128);
        INT_ARITH_TEST(u16, 127, 4, >>, 7);

        /* overflow cases */

        /* bit operations */
        INT_ARITH_TEST(u16, 0x55, 0xAA, &, 0);
        INT_ARITH_TEST(u16, 0x55, 0xAA, |, 0xFF);
        INT_ARITH_TEST(u16, 0x55, 0xAA, ^, 0xFF);

        /* basic operations */
        INT_ARITH_TEST(i16, 1, 1, +, 2); 
        INT_ARITH_TEST(i16, 1, 1, -, 0);
        INT_ARITH_TEST(i16, 1, 1, *, 1);
        INT_ARITH_TEST(i16, 1, 1, /, 1);
        INT_ARITH_TEST(i16, 1, 1, <<, 2);
        INT_ARITH_TEST(i16, 1, 1, >>, 0);
        INT_ARITH_TEST(i16, 1, 1, &, 1);
        INT_ARITH_TEST(i16, 1, 1, |, 1);
        INT_ARITH_TEST(i16, 1, 1, ^, 0);

        INT_ARITH_TEST(i16, -1, -1, +, -2); 
        INT_ARITH_TEST(i16, -1, 1, +, 0);
        INT_ARITH_TEST(i16, -1, 1, *, -1);
        INT_ARITH_TEST(i16, 1, -1, /, -1);

        /* not overflow cases */
        INT_ARITH_TEST(i16, 63, 63, +, 126); 
        INT_ARITH_TEST(i16, 127, 1, -, 126);
        INT_ARITH_TEST(i16, 15, 8, *, 120);
        INT_ARITH_TEST(i16, 125, 5, /, 25);
        INT_ARITH_TEST(i16, 1, 6, <<, 64);
        INT_ARITH_TEST(i16, 63, 4, >>, 3);

        /* overflow cases */

        /* 32 bit integer test */

        /* basic operations */
        INT_ARITH_TEST(u32, 1, 1, +, 2); 
        INT_ARITH_TEST(u32, 1, 1, -, 0);
        INT_ARITH_TEST(u32, 1, 1, *, 1);
        INT_ARITH_TEST(u32, 1, 1, /, 1);
        INT_ARITH_TEST(u32, 1, 1, <<, 2);
        INT_ARITH_TEST(u32, 1, 1, >>, 0);
        INT_ARITH_TEST(u32, 1, 1, &, 1);
        INT_ARITH_TEST(u32, 1, 1, |, 1);
        INT_ARITH_TEST(u32, 1, 1, ^, 0);

        /* not overflow cases */
        INT_ARITH_TEST(u32, 127, 127, +, 254); 
        INT_ARITH_TEST(u32, 255, 1, -, 254);
        INT_ARITH_TEST(u32, 15, 16, *, 240);
        INT_ARITH_TEST(u32, 255, 5, /, 51);
        INT_ARITH_TEST(u32, 1, 7, <<, 128);
        INT_ARITH_TEST(u32, 127, 4, >>, 7);

        /* overflow cases */

        /* bit operations */
        INT_ARITH_TEST(u32, 0x55, 0xAA, &, 0);
        INT_ARITH_TEST(u32, 0x55, 0xAA, |, 0xFF);
        INT_ARITH_TEST(u32, 0x55, 0xAA, ^, 0xFF);

        /* basic operations */
        INT_ARITH_TEST(i32, 1, 1, +, 2); 
        INT_ARITH_TEST(i32, 1, 1, -, 0);
        INT_ARITH_TEST(i32, 1, 1, *, 1);
        INT_ARITH_TEST(i32, 1, 1, /, 1);
        INT_ARITH_TEST(i32, 1, 1, <<, 2);
        INT_ARITH_TEST(i32, 1, 1, >>, 0);
        INT_ARITH_TEST(i32, 1, 1, &, 1);
        INT_ARITH_TEST(i32, 1, 1, |, 1);
        INT_ARITH_TEST(i32, 1, 1, ^, 0);

        INT_ARITH_TEST(i32, -1, -1, +, -2); 
        INT_ARITH_TEST(i32, -1, 1, +, 0);
        INT_ARITH_TEST(i32, -1, 1, *, -1);
        INT_ARITH_TEST(i32, 1, -1, /, -1);

        /* not overflow cases */
        INT_ARITH_TEST(i32, 63, 63, +, 126); 
        INT_ARITH_TEST(i32, 127, 1, -, 126);
        INT_ARITH_TEST(i32, 15, 8, *, 120);
        INT_ARITH_TEST(i32, 125, 5, /, 25);
        INT_ARITH_TEST(i32, 1, 6, <<, 64);
        INT_ARITH_TEST(i32, 63, 4, >>, 3);

        /* overflow cases */

        /* 64 bit integer test */

        /* basic operations */
        INT_ARITH_TEST(u64, 1, 1, +, 2); 
        INT_ARITH_TEST(u64, 1, 1, -, 0);
        INT_ARITH_TEST(u64, 1, 1, *, 1);
        INT_ARITH_TEST(u64, 1, 1, /, 1);
        INT_ARITH_TEST(u64, 1, 1, <<, 2);
        INT_ARITH_TEST(u64, 1, 1, >>, 0);
        INT_ARITH_TEST(u64, 1, 1, &, 1);
        INT_ARITH_TEST(u64, 1, 1, |, 1);
        INT_ARITH_TEST(u64, 1, 1, ^, 0);

        /* not overflow cases */
        INT_ARITH_TEST(u64, 127, 127, +, 254); 
        INT_ARITH_TEST(u64, 255, 1, -, 254);
        INT_ARITH_TEST(u64, 15, 16, *, 240);
        INT_ARITH_TEST(u64, 255, 5, /, 51);
        INT_ARITH_TEST(u64, 1, 7, <<, 128);
        INT_ARITH_TEST(u64, 127, 4, >>, 7);

        /* overflow cases */

        /* bit operations */
        INT_ARITH_TEST(u64, 0x55, 0xAA, &, 0);
        INT_ARITH_TEST(u64, 0x55, 0xAA, |, 0xFF);
        INT_ARITH_TEST(u64, 0x55, 0xAA, ^, 0xFF);

        /* basic operations */
        INT_ARITH_TEST(i64, 1, 1, +, 2); 
        INT_ARITH_TEST(i64, 1, 1, -, 0);
        INT_ARITH_TEST(i64, 1, 1, *, 1);
        INT_ARITH_TEST(i64, 1, 1, /, 1);
        INT_ARITH_TEST(i64, 1, 1, <<, 2);
        INT_ARITH_TEST(i64, 1, 1, >>, 0);
        INT_ARITH_TEST(i64, 1, 1, &, 1);
        INT_ARITH_TEST(i64, 1, 1, |, 1);
        INT_ARITH_TEST(i64, 1, 1, ^, 0);

        INT_ARITH_TEST(i64, -1, -1, +, -2); 
        INT_ARITH_TEST(i64, -1, 1, +, 0);
        INT_ARITH_TEST(i64, -1, 1, *, -1);
        INT_ARITH_TEST(i64, 1, -1, /, -1);

        /* not overflow cases */
        INT_ARITH_TEST(i64, 63, 63, +, 126); 
        INT_ARITH_TEST(i64, 127, 1, -, 126);
        INT_ARITH_TEST(i64, 15, 8, *, 120);
        INT_ARITH_TEST(i64, 125, 5, /, 25);
        INT_ARITH_TEST(i64, 1, 6, <<, 64);
        INT_ARITH_TEST(i64, 63, 4, >>, 3);

        /* overflow cases */

        return error_count ? BERR_UNKNOWN : BERR_SUCCESS;
}
