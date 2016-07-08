/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#include <string.h>
#include <ctype.h>

#include "bip_priv.h"

BDBG_MODULE( bip_string );

BDBG_OBJECT_ID( BIP_String );

static char BIP_String_NullByte = '\0';  /* BIP_Strings without any allocations will point here
                                          * so they look like zero-length string.              */
typedef struct BIP_String
{
    BDBG_OBJECT( BIP_String )
    char    *pString;
    size_t  stringLen;      /* Number of bytes in current string. */
    size_t  memLen;         /* Size of allocated memory. */
} BIP_String;


/*****************************************************************************
 *  Return a pointer to the BIP_String's null-terminated char string.
 *****************************************************************************/
const char *
BIP_String_GetString(BIP_StringHandle hString)
{
    BDBG_OBJECT_ASSERT(hString, BIP_String);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Returning %p (\"%s\")" BIP_MSG_PRE_ARG, (void *)hString,(void *) hString->pString, hString->pString ));
    return (hString->pString);
}


/*****************************************************************************
 *  Return the length of a BIP_String.
 *****************************************************************************/
size_t
BIP_String_GetLength(BIP_StringHandle hString)
{
    BDBG_OBJECT_ASSERT(hString, BIP_String);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Returning %zu" BIP_MSG_PRE_ARG, (void *)hString, hString->stringLen ));
    return (hString->stringLen);
}


/*****************************************************************************
 *  Return the number of excess bytes allocated to the BIP_String.
 *****************************************************************************/
int
BIP_String_GetAlloc(BIP_StringHandle hString)
{
    BDBG_OBJECT_ASSERT(hString, BIP_String);

    return(hString->memLen);   /* -1 for null terminator*/
}


/*****************************************************************************
 *  Return the number of excess bytes allocated to the BIP_String.
 *****************************************************************************/
int
BIP_String_GetExcess(BIP_StringHandle hString)
{
    BDBG_OBJECT_ASSERT(hString, BIP_String);

    if (hString->memLen == 0) {
        return 0;
    }
    return(hString->memLen - hString->stringLen - 1);   /* -1 for null terminator*/
}


/*****************************************************************************
 *  Clear/empty out the BIP_String.
 *****************************************************************************/
BIP_Status
BIP_String_Clear(BIP_StringHandle hString)
{
    BDBG_OBJECT_ASSERT(hString, BIP_String);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Clearing out old string (%zu bytes)" BIP_MSG_PRE_ARG, (void *)hString, hString->stringLen ));
    hString->stringLen = 0;     /* Empties string, but retains allocated memory. */
    if (hString->memLen > 0) {
        *(hString->pString) = '\0';
    }
    return BIP_SUCCESS;
}


/*****************************************************************************
 *  Remove a range of chars from a BIP_String.
 *****************************************************************************/
BIP_Status
BIP_String_Trim(BIP_StringHandle hString, const char * pTrim, size_t trimCount)
{
    char * stringStart = NULL;
    char * stringEnd   = NULL;
    char * trimStart   = NULL;
    char * trimEnd     = NULL;

    BDBG_OBJECT_ASSERT(hString, BIP_String);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Entry: pTrim=%p trimCount=%zu"
               BIP_MSG_PRE_ARG, (void *)hString, (void *)pTrim, trimCount));
    BDBG_MSG(( BIP_MSG_PRE_FMT  BIP_STRING_TO_PRINTF_FMT
               BIP_MSG_PRE_ARG, BIP_STRING_TO_PRINTF_ARG(hString)));

    if (hString->stringLen == 0) return (BIP_SUCCESS);  /* Nothing there to trim. */

    stringStart = hString->pString;
    stringEnd   = hString->pString + hString->stringLen - 1;
    trimStart   = stringStart + (pTrim - stringStart);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: strStart=%p trimStart=%p  strEnd=%p"
               BIP_MSG_PRE_ARG, (void *)hString, (void *)stringStart, (void *)trimStart, (void *)stringEnd));

    /* Make sure that the start of the trim is somewhere within the string. */
    if (trimStart < stringStart || trimStart > stringEnd)
    {
        BDBG_WRN(( BIP_MSG_PRE_FMT "hString %p: Start of trim range %p is outside of BIP_String range %p:%p"
                   BIP_MSG_PRE_ARG, (void *)hString,
                   (void *)trimStart,
                   (void *)stringStart, (void *)stringEnd ));
        return(BIP_ERR_INVALID_PARAMETER);
    }

    /* If trimCount is zero, trim to end of string. */
    if (trimCount == 0) {
        trimCount = stringEnd - trimStart + 1;
    }
    trimEnd     = trimStart + trimCount - 1;

    BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: strStart=%p trimStart=%p  strEnd=%p trimEnd=%p"
               BIP_MSG_PRE_ARG, (void *)hString, (void *)stringStart, (void *)trimStart, (void *)stringEnd, (void *)trimEnd));

    /* If they're trying to trim more bytes than the string has, warn them, then adjust the trimCount. */
    if (trimEnd > stringEnd) {

        BDBG_WRN(( BIP_MSG_PRE_FMT "hString %p: End of Trim range %p:%p is outside of BIP_String range %p:%p"
                   BIP_MSG_PRE_ARG, (void *)hString,
                   (void *)trimStart, (void *)(trimStart+trimCount-1),
                   (void *)hString->pString, (void *)(hString->pString + hString->stringLen-1) ));
        trimEnd = stringEnd;
        trimCount = trimEnd - trimStart + 1;
        BDBG_WRN(( BIP_MSG_PRE_FMT "hString %p: Adjusted trimCount to %zu, trimEnd to %p"
                   BIP_MSG_PRE_ARG, (void *)hString, trimCount, (void *)trimEnd))   ;
    }

    /* Now do the trim. */
    if (trimEnd < stringEnd) {
        BKNI_Memmove(trimStart, trimEnd+1, stringEnd - trimEnd);
        *(trimStart + (stringEnd - trimEnd)) = '\0';
    }
    else {  /* Trimming to end of string, nothing to move. */
        *trimStart = '\0';
    }

    hString->stringLen -= trimCount;     /* Reduce current string length. */

    BDBG_MSG(( BIP_MSG_PRE_FMT  BIP_STRING_TO_PRINTF_FMT
               BIP_MSG_PRE_ARG, BIP_STRING_TO_PRINTF_ARG(hString)));

    return BIP_SUCCESS;
}


/*****************************************************************************
 *  Make sure that a BIP_String has a specified amount of memory allocated.
 *  Here, "newLen" is length of string to be stored.  We'll add one byte for
 *  the null terminator.
 *****************************************************************************/
BIP_Status
BIP_String_Allocate(BIP_StringHandle hString, size_t newLen)
{
    BIP_Status      rc = BIP_SUCCESS;
    char           *pNewCharString;

    BDBG_OBJECT_ASSERT(hString, BIP_String);

    newLen++;       /* Add a byte for null-termination. */

    BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: newLen:%zu" BIP_MSG_PRE_ARG, (void *)hString, newLen ));
    if (hString->memLen >= newLen) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: memLen:%zu is okay. Returning." BIP_MSG_PRE_ARG, (void *)hString, hString->memLen ));
        return rc;
    }

    newLen = (((newLen-1) / 32) + 1) * 32;
    BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Adjusted newLen to %zu." BIP_MSG_PRE_ARG, (void *)hString, newLen ));

    pNewCharString = B_Os_Malloc(newLen);
    BIP_CHECK_GOTO(pNewCharString != NULL, ("B_Os_Calloc() (for %zu bytes) failed", newLen), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc);
    BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Allocate new string buffer:%p size:%zu" BIP_MSG_PRE_ARG, (void *)hString, (void *)pNewCharString, newLen ));

    BDBG_ASSERT(hString->stringLen < newLen);

    if (hString->memLen) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Copying %zu chars to new string buffer:%p" BIP_MSG_PRE_ARG, (void *)hString, hString->stringLen, (void *)hString->pString ));
        strncpy( pNewCharString, hString->pString, newLen);
        BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Freeing old string buf:%p length was %zu" BIP_MSG_PRE_ARG, (void *)hString, (void *)hString->pString, hString->memLen ));
        B_Os_Free(hString->pString);
    }

    hString->pString = pNewCharString;
    hString->memLen = newLen;

    error:
        return rc;
}


/*****************************************************************************
 *  Concatenate the string from a BIP_String to an existing BIP_String.
 *****************************************************************************/
BIP_Status
BIP_String_StrcatBipString(BIP_StringHandle hStringDest, BIP_StringHandle hStringSource)
{
    BIP_Status      rc = BIP_SUCCESS;

    rc = BIP_String_StrcatCharN(hStringDest, BIP_String_GetString(hStringSource), BIP_String_GetLength(hStringSource));

    return rc;
}


/*****************************************************************************
 *  Concatenate a printf-formatted string to an existing BIP_String.
 *****************************************************************************/
BIP_Status
BIP_String_StrcatPrintf(BIP_StringHandle hString, const char *fmt, ...)
{
    BIP_Status      rc = BIP_SUCCESS;
    va_list         ap;

    va_start(ap, fmt);
    rc = BIP_String_StrcatPrintfByVaList(hString, fmt, ap);
    va_end( ap );

    return rc;
}


/*****************************************************************************
 *  Concatenate a printf-formatted string to an existing BIP_String using a
 *  variable argument list.
 *****************************************************************************/
BIP_Status
BIP_String_StrcatPrintfByVaList(BIP_StringHandle hString, const char *fmt, va_list ap)
{
    BIP_Status      rc = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT(hString, BIP_String);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Entry: Old stringlength:%zu" BIP_MSG_PRE_ARG, (void *)hString, hString->stringLen ));
    BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Old string:   \"%.*s\"" BIP_MSG_PRE_ARG, (void *)hString, (int)hString->stringLen, hString->pString ));
    BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Adding string formatted by:\"%s\"" BIP_MSG_PRE_ARG, (void *)hString, fmt ));

    if (fmt == NULL) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Format string to add is NULL. Returning." BIP_MSG_PRE_ARG, (void *)hString ));
        return rc;
    }
    if (*fmt == '\0') {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Format string to add is zero length. Returning." BIP_MSG_PRE_ARG, (void *)hString ));
        return rc;
    }

    {
        size_t          oldLen = hString->stringLen;; /* Original length of BIP_String. */
        size_t          newLen;                       /* Final length of BIP_String.    */
        int             addLen;                       /* Length of string being added.  */
        char            tempBuffer[4];
        int             excess;
        int             myErrno;

        char           *pFmtDest;
        int             sizeFmtDest;


        /* First, we need to know how long the printf-formatted string will be so we can make
         * sure that we have enough space to hold it.  We can find out how long the result string
         * will be by calling vsnprintf into a short buffer, and even though it truncates, it
         * tells us how long the final result would have been.  */

        /* See if we have any excess space at the end of the current string in the BIP_String. */
        excess = BIP_String_GetExcess(hString);
        BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Excess bytes in existing BIP_String:%d" BIP_MSG_PRE_ARG, (void *)hString, excess ));

        if (excess > 0) {
            /* We have some excess space in the BIP_String, so try formatting into that space. */
            pFmtDest = hString->pString + hString->stringLen;
            sizeFmtDest = excess + 1;
        }
        else {
            /* No excess space, so format into a short temp buffer. */
            pFmtDest = tempBuffer;
            sizeFmtDest = sizeof (tempBuffer);
        }

        /* Do the first attempt at formatting... It might work, but will probably truncate.  But
         * either way, "addLen" will wind up with the number of characters that the format
         * string will produce. */
        addLen = BKNI_Vsnprintf(pFmtDest, sizeFmtDest, fmt, ap);
        myErrno = errno;
        BIP_CHECK_GOTO(addLen>=0, ("BKNI_Vsnprintf() failed, errno:%d, format:\"%s\" failed", myErrno, fmt), error, BIP_ERR_OS_CHECK_ERRNO, rc);

        BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Length of formatted string:%d" BIP_MSG_PRE_ARG, (void *)hString, addLen ));
        newLen = oldLen + addLen;

        /* If the first format into the excess space didn't work, then we need to allocate more space to the
         * BIP_String and do the snprintf again.  This time it should fit for sure. */
        if (addLen > excess) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: First format didn't have enough room, had %d bytes, needed %d." BIP_MSG_PRE_ARG, (void *)hString, excess, addLen ));

            rc = BIP_String_Allocate(hString, newLen);
            BIP_CHECK_GOTO(rc==B_ERROR_SUCCESS, ("BIP_String_Allocate() (for %zu bytes) failed", newLen), error, rc, rc);

            addLen = BKNI_Vsnprintf(hString->pString + oldLen, hString->memLen - oldLen, fmt, ap);
            myErrno = errno;
            BIP_CHECK_GOTO(addLen>=0, ("BKNI_Vsnprintf() failed, errno:%d, format:\"%s\" failed", myErrno, fmt), error, BIP_ERR_OS_CHECK_ERRNO, rc);
        }

        hString->stringLen = newLen;

        BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: New stringlength:  %zu" BIP_MSG_PRE_ARG, (void *)hString, hString->stringLen ));
        BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: New string: \"%s\"" BIP_MSG_PRE_ARG, (void *)hString, hString->pString ));
    }
    return rc;

error:
    if (hString->pString != &BIP_String_NullByte) {
        *(hString->pString + hString->stringLen) = '\0';     /* In case we clobbered null-terminator when formatting into excess area. */
    }
    return rc;
}

/*****************************************************************************
 *  Concatenate a null-terminated string to an existing BIP_String.
 *****************************************************************************/
BIP_Status
BIP_String_StrcatChar(BIP_StringHandle hString, const char *pCharString)
{
    BIP_Status      rc = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT(hString, BIP_String);

    if (pCharString != NULL)
    {
        rc = BIP_String_StrcatCharN(hString, pCharString, strlen(pCharString));
    }

    return rc;
}


/*****************************************************************************
 *  Concatenate an unterminated string to an existing BIP_String.
 *****************************************************************************/
BIP_Status
BIP_String_StrcatCharN(BIP_StringHandle hString, const char *pCharString, size_t length)
{
    BIP_Status      rc = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT(hString, BIP_String);

    BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Old stringlength:%zu" BIP_MSG_PRE_ARG, (void *)hString, hString->stringLen ));
    BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Old string:   \"%s\"" BIP_MSG_PRE_ARG, (void *)hString, hString->pString ));
    BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Adding string:\"%.*s\"" BIP_MSG_PRE_ARG, (void *)hString, (int)length, pCharString ));

    if (pCharString == NULL) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: String to add is NULL. Returning." BIP_MSG_PRE_ARG, (void *)hString ));
        return rc;
    }

    length = length;

    if (length == 0) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Length to add is zero. Returning." BIP_MSG_PRE_ARG, (void *)hString ));
        return rc;
    }

    {
        size_t          oldLen = hString->stringLen;
        size_t          newLen = oldLen + length;

        if (newLen+1 > hString->memLen) {
            rc = BIP_String_Allocate(hString, newLen);
            BIP_CHECK_GOTO(rc==B_ERROR_SUCCESS, ("BIP_String_Allocate() (for %zu bytes) failed", newLen), error, rc, rc);
        }

        strncpy(hString->pString + oldLen, pCharString, hString->memLen - oldLen);
        hString->stringLen = newLen;
        *(hString->pString + newLen) = '\0';       /* Insure null termination. */

        BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: New stringlength:  %zu" BIP_MSG_PRE_ARG, (void *)hString, hString->stringLen ));
        BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: New string: \"%s\"" BIP_MSG_PRE_ARG, (void *)hString, hString->pString ));
    }

    error:
        return rc;
}


/*****************************************************************************
 *  Copy the string from a BIP_String to an existing BIP_String, replacing
 *  any previous contents.
 *****************************************************************************/
BIP_Status
BIP_String_StrcpyBipString(BIP_StringHandle hStringDest, BIP_StringHandle hStringSource)
{
    BIP_Status      rc = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT(hStringDest, BIP_String);

    rc = BIP_String_Clear(hStringDest);
    BIP_CHECK_GOTO(rc==BIP_SUCCESS, ("BIP_String_Clear() failed"), error, rc, rc);

    rc = BIP_String_StrcatBipString(hStringDest, hStringSource);
    BIP_CHECK_GOTO(rc==BIP_SUCCESS, ("BIP_String_StrcatBipString() failed"), error, rc, rc);

error:
    return rc;
}


/*****************************************************************************
 *  Copy a printf-formatted string to an existing BIP_String, replacing
 *  any previous contents.
 *****************************************************************************/
BIP_Status
BIP_String_StrcpyPrintf(BIP_StringHandle hString, const char *fmt, ...)
{
    BIP_Status      rc = BIP_SUCCESS;
    va_list         ap;

    BDBG_OBJECT_ASSERT(hString, BIP_String);

    rc = BIP_String_Clear(hString);
    BIP_CHECK_GOTO(rc==BIP_SUCCESS, ("BIP_String_Clear() failed"), error, rc, rc);

    va_start(ap, fmt);
    rc = BIP_String_StrcatPrintfByVaList(hString, fmt, ap);
    va_end( ap );
    BIP_CHECK_GOTO(rc==BIP_SUCCESS, ("BIP_String_StrcatPrintfByVaList() failed"), error, rc, rc);

error:
    return rc;
}


/*****************************************************************************
 *  Copy a null-terminated string to an existing BIP_String, replacing
 *  any previous contents.
 *****************************************************************************/
BIP_Status
BIP_String_StrcpyChar(BIP_StringHandle hString, const char *pCharString)
{
    BIP_Status      rc = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT(hString, BIP_String);

    rc = BIP_String_Clear(hString);
    BIP_CHECK_GOTO(rc==BIP_SUCCESS, ("BIP_String_Clear() failed"), error, rc, rc);

    rc = BIP_String_StrcatChar(hString, pCharString);
    BIP_CHECK_GOTO(rc==BIP_SUCCESS, ("BIP_String_StrcatChar() failed"), error, rc, rc);

error:
    return rc;
}


/*****************************************************************************
 *  Copy an unterminated string to an existing BIP_String, replacing
 *  any previous contents.
 *****************************************************************************/
BIP_Status
BIP_String_StrcpyCharN(BIP_StringHandle hString, const char *pCharString, size_t length)
{
    BIP_Status      rc = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT(hString, BIP_String);

    rc = BIP_String_Clear(hString);
    BIP_CHECK_GOTO(rc==BIP_SUCCESS, ("BIP_String_Clear() failed"), error, rc, rc);

    rc = BIP_String_StrcatCharN(hString, pCharString, length);
    BIP_CHECK_GOTO(rc==BIP_SUCCESS, ("BIP_String_StrcatCharN() failed"), error, rc, rc);

error:
    return rc;
}


/*****************************************************************************
 * Constructor: Create an instance of a class.
 *****************************************************************************/
BIP_StringHandle
BIP_String_CreateFromBipString( BIP_StringHandle hString)
{
    BIP_StringHandle hNewString;
    const char      *pCharString = NULL;

    if (hString != NULL) {
        BDBG_OBJECT_ASSERT(hString, BIP_String);
        pCharString = BIP_String_GetString(hString);
    }
    hNewString = BIP_String_CreateFromChar(pCharString);

    return(hNewString);

} /* BIP_String_CreateFromBipString */


/*****************************************************************************
 * Constructor: Create an instance of a class.
 *****************************************************************************/
BIP_StringHandle
BIP_String_CreateFromPrintf( const char *fmt, ...)
{
    BIP_Status   rc = BIP_SUCCESS;
    BIP_String  *hString = BIP_String_Create();
    va_list      ap;

    if (fmt != NULL) {
        va_start(ap, fmt);
        rc = BIP_String_StrcatPrintfByVaList(hString, fmt, ap);
        va_end( ap );
        BIP_CHECK_GOTO(rc==BIP_SUCCESS, ("BIP_String_CatPrintfByVaList() failed"), error, rc, rc);
    }
    return(hString);

error:
    return(NULL);
} /* BIP_String_CreateFromPrintf */


/*****************************************************************************
 * Constructor: Create an instance of a class.
 *****************************************************************************/
BIP_StringHandle
BIP_String_CreateFromChar( const char *pInitString )
{
    BIP_String  *hString = BIP_String_Create();

   if (pInitString != NULL) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Initializing to \"%s\"" BIP_MSG_PRE_ARG, (void *)hString, pInitString));
        BIP_String_StrcatChar(hString, pInitString);
    }

    return(hString);

} /* BIP_String_CreateFromChar*/


/*****************************************************************************
 * Constructor: Create an instance of a class.
 *****************************************************************************/
BIP_StringHandle
BIP_String_CreateFromCharN( const char *pInitString, size_t length )
{
    BIP_String  *hString = BIP_String_Create();

   if (pInitString != NULL) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Initializing to \"%.*s\"" BIP_MSG_PRE_ARG, (void *)hString, (int)length, pInitString));
        BIP_String_StrcatCharN(hString, pInitString, length);
    }

    return(hString);

} /* BIP_String_CreateFromCharN */


/*****************************************************************************
 * Constructor: Create an instance of a class.
 *****************************************************************************/
BIP_StringHandle
BIP_String_Create( void )
{
    BIP_String  *hString;

    hString = B_Os_Calloc(1, sizeof(BIP_String));
    if (NULL == hString)
    {
        BERR_TRACE( BIP_ERR_OUT_OF_SYSTEM_MEMORY );
        return(NULL);
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hString %p: Allocated object (%zu bytes)" BIP_MSG_PRE_ARG, (void *)hString, sizeof(BIP_String)));

    BDBG_OBJECT_SET( hString, BIP_String );

    hString->pString = &BIP_String_NullByte;

    return(hString);

} /* BIP_String_Create */


/*****************************************************************************
 *  Destructor: Destroy an instance a class.
 *****************************************************************************/
void
BIP_String_Destroy( BIP_StringHandle hString )
{
    BDBG_OBJECT_ASSERT( hString, BIP_String );

    if (hString->pString != &BIP_String_NullByte) {
        B_Os_Free(hString->pString);
    }

    /* Then finally free the BIP_String struct. */
    BDBG_OBJECT_DESTROY( hString, BIP_String );
    B_Os_Free( hString );
    return;
}
