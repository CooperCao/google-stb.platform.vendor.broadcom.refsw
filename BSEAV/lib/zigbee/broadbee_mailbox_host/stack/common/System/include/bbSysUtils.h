/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 ******************************************************************************
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/common/System/include/bbSysUtils.h $
*
* DESCRIPTION:
*   Auxiliary utilities toolset interface.
*
* $Revision: 3612 $
* $Date: 2014-09-17 09:29:25Z $
*
*****************************************************************************************/


#ifndef _BB_SYS_UTILS_H
#define _BB_SYS_UTILS_H


/************************* INCLUDES *****************************************************/
#include "bbSysTypes.h"             /* Common System Types definition. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief Returns the pointer to the data structure \p structName by a given pointer
 *  \p fieldPtr to its field named \p fieldName.
 */
#define GET_PARENT_BY_FIELD(structName, fieldName, fieldPtr)                \
    ( (structName*)(void*)(                                                 \
            ((uint8_t*)fieldPtr) - offsetof(structName, fieldName)   ) )


/**//**
 * \brief Returns the size of an array \p a, in elements of this array.
 */
#define ARRAY_SIZE(a)   (sizeof(a) / sizeof(a[0]))


/**//**
 * \brief Returns the size of a structure field
 */
#define FIELD_SIZE(type, field) (sizeof(((type*) NULL)->field))


/**//**
 * \brief Returns the offset of a structure field
 * \note  Use instead of buit-in offsetof() for packed and unaligned structures.
 */
#define OFFSETOF(type, field) ((size_t)(void*)&(((type*) NULL)->field))


/**//**
 * \brief Returns the implication of two given expressions \p x and \p y.
 * \details The implication means:  if X==TRUE then Y==TRUE.
 * \details The formula is: (X imp Y) = ((not X) or Y)
 */
#define IMP(x, y)       ( !(x) || (y) )


/**//**
 * \brief Returns the minimum of two given expressions \p x and \p y.
 */
#define MIN(x, y)       ( ((x) < (y)) ? (x) : (y) )


/**//**
 * \brief Returns the maximum of two given expressions \p x and \p y.
 */
#define MAX(x, y)       ( ((x) > (y)) ? (x) : (y) )


/**//**
 * \brief Returns the quotient of \p a divided by \p b rounded upwards to the nearest
 *  integer.
 */
#define CEIL(a, b)      ((a) ? (((a) - 1U) / (b) + 1U) : 0U)


/**//**
 * \brief Returns the quotient of \p a divided by \p b rounded to the nearest integer
 *  according to the standard arithmetic rules: if the fractional part of (a/b) is greater
 *  or equal to 0.5 then the result is rounded upwards; if the fractional part of (a/b) is
 *  less then 0.5 the result is rounded downwards.
 * \note Use this formula only for unsigned arguments. The formula is not compatible with
 *  the signed arguments: when a and b have different signs it gives incorrect result.
 */
#define ROUND(a, b)      ( ((a) + ((b) >> 1)) / (b) )


/**//**
 * \brief Declares a long bitmap named \p name of \p size bits. The \p size is rounded
 *  upwards to come a multiple of 8.
 */
#define BITMAP_DECLARE(name, size)          uint8_t name[CEIL(size, 8)]

/**//**
 * \brief Clears all bits in given bitmap.
 */
#define BITMAP_RESET(name)                  memset((name), 0U, sizeof(name))

/**//**
 * \brief Returns the value of a bit at position \p bit in the long bitmap named \p name.
 */
#define BITMAP_ISSET(name, bit)             ( 0 != ((name)[(bit) >> 3] & (1 << ((bit) & 0x7))) )

/**//**
 * \brief Sets the bit at position \p bit in the long bitmap named \p name.
 */
#define BITMAP_SET(name, bit)               (name)[(bit) >> 3] |= (1 << ((bit) & 0x7))

/**//**
 * \brief Clears the bit at position \p bit in the long bitmap named \p name.
 */
#define BITMAP_CLR(name, bit)               (name)[(bit) >> 3] &= ~(1 << ((bit) & 0x7))

/**//**
 * \brief Assigns the given bitmap with the second bitmap.
 */
#define BITMAP_ASSIGN(nameDst, nameSrc)     memcpy((nameDst), (nameSrc), sizeof(nameDst))

/**//**
 * \brief Checks number. Return true if number is power of two.
 */
#define IS_POWER_OF_TWO(name)       ((0 != (name)) && (0 == ((name)&(name - 1))))

/**//**
 * \brief Return True if mask is fully included into a given set and False otherwise
 */
#define IS_SUBSET_OF(mask, set)     ((mask) == ((set)&(mask)))

#define IS_TURNED_ON(oldValue, newValue, mask)  \
    ((!IS_SUBSET_OF((mask), (oldValue)))        \
     && (IS_SUBSET_OF((mask), (newValue))))

/* Macros below are intended to access bitfields. The result macros to be used are
   GET_BITFIELD_VALUE() and SET_BITFIELD_VALUE(). */

/**//**
 * \brief Creates a bit mask with single set bit on the specified position.
 */
#define BIT(pos)                    (1UL << (pos))

/**//**
 * \brief Creates a bit mask of specified length.
 */
#define BIT_MASK(len)               (BIT(len) - 1UL)

/**//**
 * \brief Creates a bit field mask of specified length and start position.
 */
#define BIT_FIELD_MASK(start, len)  (BIT_MASK(len) << (start))

/**//**
 * \brief Creates a bit field mask of specified length, start position and value.
 */
#define BIT_FIELD_VALUE(value, start, len)  (((value) & BIT_MASK(len)) << (start))

/**//**
 * \brief Extracts a bit field value of specified start position and length.
 */
#define GET_BITFIELD_VALUE(bitmask, start, len)    (((bitmask) >> (start)) & BIT_MASK(len))

/**//**
 * \brief Inserts a bit field value with specified start position and length.
 */
#define SET_BITFIELD_VALUE(bitmask, start, len, value) \
    (bitmask = (bitmask & ~BIT_FIELD_MASK(start, len)) | BIT_FIELD_VALUE(value, start, len))

/**//**
 * \brief Extracts a mask from a BITMAP. \n
 * BITMAP MUST be aligned and mask length MUST be one of 2, 4, 8, 16, 32.
 */
#define BITMAP_MASK_GET(bitmap, bit, len)                                   \
    GET_BITFIELD_VALUE(((uint32_t*)(bitmap))[(bit) >> 5], (bit) & 0x1F, len)

/**//**
 * \brief Sets up a mask to a BITMAP. \n
 * BITMAP MUST be aligned and mask length MUST be one of 2, 4, 8, 16, 32.
 */
#define BITMAP_MASK_SET(bitmap, bit, len, value)                            \
    SET_BITFIELD_VALUE(((uint32_t*)(bitmap))[(bit) >> 5], (bit) & 0x1F, len, value)

/**//**
 * \brief Gets amount of the arguments. Execute by preprocessor and have a bug in case without arguments
 *      (VA_NARGS() is not equal 0).
 *  Commented numbers are a workaround for the arc preprocessor memory leak.
 */
#define VA_NARGS(...) VA_NARGS_EVAL(__VA_ARGS__)
#define VA_NARGS_EVAL(...) VA_NARGS_IMPL(__VA_ARGS__, \
        /* 255, 254, */ 253, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 242, 241, 240, \
        239, 238, 237, 236, 235, 234, 233, 232, 231, 230, 229, 228, 227, 226, 225, 224, \
        223, 222, 221, 220, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210, 209, 208, \
        207, 206, 205, 204, 203, 202, 201, 200, 199, 198, 197, 196, 195, 194, 193, 192, \
        191, 190, 189, 188, 187, 186, 185, 184, 183, 182, 181, 180, 179, 178, 177, 176, \
        175, 174, 173, 172, 171, 170, 169, 168, 167, 166, 165, 164, 163, 162, 161, 160, \
        159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144, \
        143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132, 131, 130, 129, 128, \
        127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112, \
        111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100,  99,  98,  97,  96, \
        095,  94,  93,  92,  91,  90,  89,  88,  87,  86,  85,  84,  83,  82,  81,  80, \
        079,  78,  77,  76,  75,  74,  73,  72,  71,  70,  69,  68,  67,  66,  65,  64, \
        063,  62,  61,  60,  59,  58,  57,  56,  55,  54,  53,  52,  51,  50,  49,  48, \
        047,  46,  45,  44,  43,  42,  41,  40,  39,  38,  37,  36,  35,  34,  33,  32, \
        031,  30,  29,  28,  27,  26,  25,  24,  23,  22,  21,  20,  19,  18,  17,  16, \
        015,  14,  13,  12,  11,  10,   9,   8,   7,   6,   5,   4,   3,   2,   1,   0)
/**//**
 * \brief Helper macro. Gets amount of the arguments.
 * Commented numbers are a workaround for the arc preprocessor memory leak.
 */
#define VA_NARGS_IMPL(_________1,   _2,   _3,   _4,   _5,   _6,   _7,   _8,   _9,  _10,  _11,  _12,  _13,  _14,  _15, \
                      __16,  _17,  _18,  _19,  _20,  _21,  _22,  _23,  _24,  _25,  _26,  _27,  _28,  _29,  _30,  _31, \
                      __32,  _33,  _34,  _35,  _36,  _37,  _38,  _39,  _40,  _41,  _42,  _43,  _44,  _45,  _46,  _47, \
                      __48,  _49,  _50,  _51,  _52,  _53,  _54,  _55,  _56,  _57,  _58,  _59,  _60,  _61,  _62,  _63, \
                      __64,  _65,  _66,  _67,  _68,  _69,  _70,  _71,  _72,  _73,  _74,  _75,  _76,  _77,  _78,  _79, \
                      __80,  _81,  _82,  _83,  _84,  _85,  _86,  _87,  _88,  _89,  _90,  _91,  _92,  _93,  _94,  _95, \
                      __96,  _97,  _98,  _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, \
                      _112, _113, _114, _115, _116, _117, _118, _119, _120, _121, _122, _123, _124, _125, _126, _127, \
                      _128, _129, _130, _131, _132, _133, _134, _135, _136, _137, _138, _139, _140, _141, _142, _143, \
                      _144, _145, _146, _147, _148, _149, _150, _151, _152, _153, _154, _155, _156, _157, _158, _159, \
                      _160, _161, _162, _163, _164, _165, _166, _167, _168, _169, _170, _171, _172, _173, _174, _175, \
                      _176, _177, _178, _179, _180, _181, _182, _183, _184, _185, _186, _187, _188, _189, _190, _191, \
                      _192, _193, _194, _195, _196, _197, _198, _199, _200, _201, _202, _203, _204, _205, _206, _207, \
                      _208, _209, _210, _211, _212, _213, _214, _215, _216, _217, _218, _219, _220, _221, _222, _223, \
                      _224, _225, _226, _227, _228, _229, _230, _231, _232, _233, _234, _235, _236, _237, _238, _239, \
                      _240, _241, _242, _243, _244, _245, _246, _247, _248, _249, _250, _251, _252, _253, /* _254, _255, */\
                      N, ...) N

/**//**
 * \brief Gets amount of the arguments. Execute by compiler.
 */
#define VA_NARGS_COMPILE_TIME(...) ((uint8_t)(sizeof((uint8_t[]){ __VA_ARGS__ })/sizeof(uint8_t)))

/**//**
 * \brief Swaps values.
 */
#define SWAP_XOR(a, b) (((a) ^ (b)) && ((b) ^= (a) ^= (b), (a) ^= (b)))

/**//**
 * \brief Compare two number and take care of overflow threshold limit.
 */
#define COMPARE_WITH_THRESHOLD(a, b, threshold) \
    (((MAX((a), (b)) - MIN((a), (b))) < (threshold)) ? ((a) >= (b) ? 1 : 0) : ((a) > (b) ? 0 : 1))


/**//**
 * \brief Makes a code block wrapped with the do-while operator. It allows to set
 *  semicolon at the end of this macro inside other operators like if-else, while, etc.
 */
#define SYS_WRAPPED_BLOCK(codeBlock)   do { codeBlock; } while (0)


/**//**
 * \brief   Zeroes the specified object.
 * \param[in]   object      Pointer to the object to be zeroed.
 */
#define SYS_ZERO_OBJECT(object)     memset((object), 0x0, sizeof(*(object)))

/**//**
 * \brief Helper macros which work with structures of variable length such as
 *  struct _VarStruct_t
 *  {
 *      type1_t     a;
 *      type2_t     b;
 *      someUint_t  size;       // obligatory field
 *      type3_t     value[1];   // variable part
 *  } VarStruct_t;
 */
/**//**
 * \brief Gets size of the specified Variable length structure.
 *        Prototype is int SYS_VarStructSize(type_t *pVarStruct)
 */
#define SYS_VARSTRUCT_SIZE(pVarStruct)                                      \
        (offsetof(typeof(*(pVarStruct)), value) + (pVarStruct)->size)

/**//**
 * \brief Check if the specified Variable length structure does not exceed pointer pEnd
 *        Prototype is bool SYS_VarStructCheck(uint8_t *const pEnd, type_t *pVarStruct)
 */
#define SYS_VARSTRUCT_CHECK(pEnd, pVarStruct)                               \
        ((uint8_t*)(pVarStruct) + SYS_VARSTRUCT_SIZE(pVarStruct) <= (uint8_t*)(pEnd))

/**//**
 * \brief Updates the pointer to pointer to a variable length structure to the next one
 *        Prototype is void SYS_VarStruct2Next(type_t **ppVarStruct)
 */
#define SYS_VARSTRUCT2_NEXT(ppVarStruct)                                    \
        (*ppVarStruct = (typeof(*(ppVarStruct)))SYS_ALIGN2((uint8_t*)*(ppVarStruct) + SYS_VARSTRUCT_SIZE(*(ppVarStruct))))

/**//**
 * \brief Returns aligned to 2-byte boundary value of the specified pointer
 *        Prototype is uint8_t* SYS_Align2(void *pointer)
 */
#define SYS_ALIGN2(pointer)                                                 \
        ((uint8_t*)(pointer) + ((uint32_t)(pointer) & 1))


/************************* INLINES *****************************************************/
/**//**
 * \brief Swaps values of two bytes.
 */
INLINE void SWAP8(uint8_t *const x, uint8_t *const y)
{
    uint8_t _x = *x;
    *x = *y;
    *y = _x;
}

/**//**
 * \brief Swaps values of two double words (DWORD).
 */
INLINE void SWAP32(uint32_t *const x, uint32_t *const y)
{
    uint32_t _x = *x;
    *x = *y;
    *y = _x;
}

/**//**
 * \brief Swaps values of two arrays.
 * \param[in] x,y - array pointers
 * \param[in] length - amount bytes to swap
 */
INLINE void SWAP(void *x, void *y, uint32_t length)
{
    uint8_t *_x = (uint8_t *)(void *)x;
    uint8_t *_y = (uint8_t *)(void *)y;
    if (0x0 == ((((size_t)_x) | ((size_t)_y)) & 0x3))
    {
        size_t len4 = length / sizeof(uint32_t);
        for (size_t i = 0; i < len4; i++)
        {
            SWAP32((uint32_t*)_x, (uint32_t*)_y);
            _x += sizeof(uint32_t);
            _y += sizeof(uint32_t);
        }
        length = length & 0x3;
    }

    for (size_t i = 0; i < length; i++)
    {
        SWAP8(_x, _y);
        _x++;
        _y++;
    }
}

/**//**
 * \brief Returns Absolute value of specified value
 * \param[in] x - value to convert
 *
 * \return Absolute value of x
 */
#ifdef ABS
#undef ABS
#endif
INLINE uint32_t ABS(int32_t x)
{
    const int32_t mask = x >> 31;

    return (x + mask) ^ mask;
}

INLINE bool OPPOSITE_SIGNS(int32_t a, int32_t b)
{
    return (a ^ b) < 0;
}


/*
 * Repeat pragma GCC optimize because function definitions (including inlined) turn these pragrmas off automatically
 * when compiled by G++ but not GCC.
 */
#if (defined(__arm__) || defined(__i386__)) && !defined(__clang__)
# pragma GCC optimize "short-enums"     /* Implement short enums. */
# pragma GCC diagnostic ignored "-Wattributes"
#endif

#endif /* _BB_SYS_UTILS_H */
