/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ***************************************************************************/
#ifndef BIP_STRING_H
#define BIP_STRING_H

/* BIP String is a class that implements a friendly interface for handling
 * dynamic strings.
 */

#include "bip.h"


#ifdef __cplusplus
extern "C" {
#endif


/*************************************************************************
 *  Class: BIP_String
 **************************************************************************/
typedef struct BIP_String  *BIP_StringHandle;



/**
 * Summary:
 * Macros for dumping contents of a BIP_String.
 *
 * Description:
 * For example:
 *      BDBG_LOG(( "Just received: " BIP_STRING_TO_PRINTF_FMT, BIP_STRING_TO_PRINTF_ARG(hMyFavoriteBipString) ));
 * will print something like this:
 *
 *      00:00:01.274 string_test: Just received: hMyFavoriteBipString(0x2b7b0) len=3 alloc=16 contents(0x2b818)="ABC"
 **/
#define BIP_STRING_TO_PRINTF_FMT     \
    "%s(%p) len=%zu alloc=%d contents(%p)=\"%s\""

#define BIP_STRING_TO_PRINTF_ARG(hString)                                             \
    #hString, (void *)(hString), BIP_String_GetLength(hString), BIP_String_GetAlloc(hString), \
    (void *)BIP_String_GetString(hString), BIP_String_GetString(hString)

/***************************************************************************************
 * Basic BIP_String APIs...
 ***************************************************************************************/

/**
 * Summary:
 * Create a BIP_String object.
 *
 * Description:
 * Allocates a BIP_String structure (object) and associates it with a copy of the
 * specified character string.
 *
 **/
BIP_StringHandle
BIP_String_Create( void );

BIP_StringHandle
BIP_String_CreateFromBipString( BIP_StringHandle hString);

BIP_StringHandle
BIP_String_CreateFromPrintf( const char *fmt, ...);

BIP_StringHandle
BIP_String_CreateFromChar( const char *pInitString );                    /* For null-terminated strings. */

BIP_StringHandle
BIP_String_CreateFromCharN( const char *pInitString, size_t maxLength ); /* For non-null-terminated strings. */

/**
 * Summary:
 * Destroy a BIP_String object.
 *
 * Description:
 * Deallocates a BIP_String structure (object) and any other memory that might be
 * associated with it.
 *
 **/
void
BIP_String_Destroy( BIP_StringHandle hString );

/**
 * Summary:
 * Concatenate a string onto the end of an existing BIP_String.
 **/
BIP_Status
BIP_String_StrcatBipString(BIP_StringHandle hStringDest, BIP_StringHandle hStringSource);

BIP_Status
BIP_String_StrcatPrintf(BIP_StringHandle hString, const char *fmt, ...);

BIP_Status
BIP_String_StrcatChar(BIP_StringHandle hString, const char *pCharString);                    /* For null-terminated strings. */

BIP_Status
BIP_String_StrcatCharN(BIP_StringHandle hString, const char *pCharString, size_t maxLength); /* For non-null-terminated strings. */

/**
 * Summary:
 * Copy a new string into a BIP_String, overwriting any previously existing string.
 **/
BIP_Status
BIP_String_StrcpyBipString(BIP_StringHandle hStringDest, BIP_StringHandle hStringSource);

BIP_Status
BIP_String_StrcpyPrintf(BIP_StringHandle hString, const char *fmt, ...);

BIP_Status
BIP_String_StrcpyChar(BIP_StringHandle hString, const char *pCharString);                    /* For null-terminated strings. */

BIP_Status
BIP_String_StrcpyCharN(BIP_StringHandle hString, const char *pCharString, size_t maxLength); /* For non-null-terminated strings. */

/**
 * Summary:
 * Return the size of a BIP_String.
 **/
size_t
BIP_String_GetLength(BIP_StringHandle);

/**
 * Summary:
 * Return the pointer to the BIP_String's null-terminated char string.
 **/
const char *
BIP_String_GetString(BIP_StringHandle hString);

/**
 * Summary:
 * Clear/empty out a BIP_String (but retain allocated memory).
 **/
BIP_Status
BIP_String_Clear(BIP_StringHandle hString);

/**
 * Summary:
 * Remove a range of chars from a BIP_String.
 **/
BIP_Status
BIP_String_Trim(BIP_StringHandle hString, const char * pTrim, size_t trimCount);


/***************************************************************************************
 * The following API are not normally required, but are provided for advanced users.
 ***************************************************************************************/

/**
 * Summary:
 * (For advanced usage) Return the number of bytes allocated for a BIP_String's character string.
 **/
int
BIP_String_GetAlloc(BIP_StringHandle hString);

/**
 * Summary:
 * (For advanced usage) Return the number of excess allocated bytes at the end of a BIP_String.
 **/
int
BIP_String_GetExcess(BIP_StringHandle hString);

/**
 * Summary:
 * (For advanced usage) Allocate memory to hold the specified number of additional characters.
 **/
BIP_Status
BIP_String_Allocate(BIP_StringHandle hString, size_t newLen) ;

/**
 * Summary:
 * (For advanced usage) Concatenate to a BIP_String using a va_list (variable argument list).
 **/
BIP_Status
BIP_String_StrcatPrintfByVaList(BIP_StringHandle hString, const char *fmt, va_list ap);

#ifdef __cplusplus
}
#endif

#endif /* !defined BIP_STRING_H */
