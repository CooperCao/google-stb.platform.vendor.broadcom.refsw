/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include <sys/stat.h>
#include <sys/types.h>

#include "bip_priv.h"

BDBG_MODULE( bip_atom );



/* User data descriptor and free function for atoms with BIP_String payloads */
static void
userFreeForBipString(batom_t atom, void *user)
{
    BIP_StringHandle hString = *(BIP_StringHandle  *)user;
    BSTD_UNUSED(atom);
    BDBG_MSG(( BIP_MSG_PRE_FMT "Destroying BIP_String=%p for atom=%p." BIP_MSG_PRE_ARG, (void *)hString, (void *)atom ));
    BIP_String_Destroy(hString);
    return;
}

/* Atom's user data descriptor for atom to find the destruction callback. */
static const batom_user userDescForBipString = {
    userFreeForBipString,
    sizeof(void **)
};

/* User data descriptor and free function for atoms with B_Os_Malloc'd payloads */
static void
userFreeForBOsMalloc(batom_t atom, void *user)
{
    void *ptr = *(void **)user;
    BSTD_UNUSED(atom);
    BDBG_MSG(( BIP_MSG_PRE_FMT "Freeing B_Os_Malloc memory=%p for atom=%p." BIP_MSG_PRE_ARG, (void *)ptr, (void *)atom ));
    B_Os_Free(ptr);
    return;
}

/* Atom's user data descriptor for atom to find the destruction callback. */
static const batom_user userDescForBOsMalloc = {
    userFreeForBOsMalloc,
    sizeof(void **)
};


/*****************************************************************************
 *  API to create an atom from an existing BIP_String.  The atom will then "own" the
 *  BIP_String, and will destroy it when the atom's reference count goes to zero.
 *****************************************************************************/
batom_t
BIP_Atom_AtomFromBipString(batom_factory_t factory, BIP_StringHandle hString)
{
    batom_t aString;

    BDBG_MSG(( BIP_MSG_PRE_FMT "Creating batom from range" BIP_MSG_PRE_ARG ));
    aString = batom_from_range(factory,
                                BIP_String_GetString(hString),
                                BIP_String_GetLength(hString),
                                &userDescForBipString,
                                &hString);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Created atom aString1=%p" BIP_MSG_PRE_ARG, (void *)aString ));
    return aString;
}


/*****************************************************************************
 *  API to create an atom by duplicating an given range.  This is intended for
 *  creating atoms from some pointer being passed to an API.  Since the caller
 *  owns the passed range, we will need to create a duplicate range for the
 *  atom to own.
 *****************************************************************************/
batom_t
BIP_Atom_AtomFromDupedRange(batom_factory_t factory, const void *base, size_t len)
{
    batom_t aRange;
    void   *ptr;

    BDBG_MSG(( BIP_MSG_PRE_FMT "Creating batom from duped range" BIP_MSG_PRE_ARG ));

    ptr = B_Os_Malloc(len);
    BKNI_Memcpy(ptr, base, len);

    aRange = batom_from_range(factory,
                                ptr,
                                len,
                                &userDescForBOsMalloc,
                                &ptr);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Created atom aRange=%p" BIP_MSG_PRE_ARG, (void *)aRange ));

    return aRange;
}


/*****************************************************************************
 *  API to create an atom with a pre-allocated chunk of B_Os_Malloc
 *  memory.  The atom will "own" the chunk of memory and will free it when
 *  atom is recycled (destroyed).
 *****************************************************************************/
batom_t
BIP_Atom_AtomFromBOsMallocRange(batom_factory_t factory, void *base, size_t len)
{
    batom_t aRange;

    BDBG_MSG(( BIP_MSG_PRE_FMT "Creating batom from B_Os_Malloc'd range" BIP_MSG_PRE_ARG ));

    aRange = batom_from_range(factory,
                                base,
                                len,
                                &userDescForBOsMalloc,
                                &base);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Created atom aRange=%p" BIP_MSG_PRE_ARG, (void *)aRange ));

    return aRange;
}


/*****************************************************************************
 *  API to print the contents of a cursor that contains (mostly) printable
 *  characters.
 *****************************************************************************/
#define B_ATOM_PRINT_MAX 256
void
BIP_Atom_CursorPrint(batom_cursor *pCursor, const char *name)
{
    char    buffer[4];  /* make this bigger! */
    unsigned i;
    int     byteCount = 0;
    batom_cursor    myCursor;
    BIP_StringHandle hString = BIP_String_Create();

    batom_cursor_clone(&myCursor, pCursor);

    for(byteCount=0;!BATOM_IS_EOF(&myCursor) && byteCount<B_ATOM_PRINT_MAX;byteCount++) {
        for(i=0;i<sizeof(buffer);i++) {
            int b = batom_cursor_next(&myCursor);
            #if 0  /* ==================== GARYWASHERE - Start of Original Code ==================== */
            /* GARYWASHERE (other) */  BDBG_WRN(("%s:%d: i:%d b=0x%x (%c)", BSTD_FUNCTION, __LINE__, i, b, b));
            #endif /* ==================== GARYWASHERE - End of Modified Code   ==================== */

            if(b==BATOM_EOF) {
                break;
            }
            if (isprint(b)) {
            }

            buffer[i] = isprint(b) ? b : '?';
        }
        #if 0  /* ==================== GARYWASHERE - Start of Original Code ==================== */
        /* GARYWASHERE (other) */  BDBG_WRN(("%s:%d: i:%d", BSTD_FUNCTION, __LINE__, i));
        #endif /* ==================== GARYWASHERE - End of Modified Code   ==================== */

        BIP_String_StrcatCharN(hString, buffer, i);
    }

    BDBG_LOG(( BIP_MSG_PRE_FMT "Atom %s : \"%s\"" BIP_MSG_PRE_ARG, name, BIP_String_GetString(hString) ));

    BIP_String_Destroy(hString);
    return;
}

/*****************************************************************************
 *  API to print the contents of an atom that contains (mostly) printable
 *  characters.
 *****************************************************************************/
void
BIP_Atom_AtomPrint(batom_t atom, const char *name)
{
    batom_cursor cursor;

    BDBG_ASSERT(atom);

    batom_cursor_from_atom(&cursor, atom);

    BIP_Atom_CursorPrint(&cursor, name);
    return;
}

/*****************************************************************************
 *  API to dump (in hex and ASCII) the contents of an cursor.
 *****************************************************************************/
void
BIP_Atom_CursorDump(batom_cursor *pCursor, const char *name)
{
    char          ch;
    char             fmtBuf[256];
    unsigned         fmtIdx;              /* which byte in fmtBuf */
    unsigned         lineIdx;             /* which value on the line */
    unsigned         bufIdx = 0;          /* virtual index into cursor being dumped */
    const unsigned   bytesPerLine = 16;
    batom_cursor     myCursor;
    batom_checkpoint startOfLine;

    batom_cursor_clone(&myCursor,pCursor);

    /* Loop for each line. */
    do
    {
        batom_cursor_save(&myCursor, &startOfLine);

        fmtIdx = 0;
        fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, "%s: ", name);
        fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, " 0x%04x: ", bufIdx);

        /* Dump 16 bytes in hex on the first half of the line. */
        lineIdx = 0;
        while (lineIdx < bytesPerLine ) {
            if (lineIdx == bytesPerLine/2) {
                fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, "  ");
            }

            ch = batom_cursor_next(&myCursor);
            if (!batom_cursor_eof(&myCursor)) {
                fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, " %02x", ch);
            }
            else {
                fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, "   ");
            }
            lineIdx++;
        }

        fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, "   |");

        batom_cursor_rollback(&myCursor, &startOfLine);

        lineIdx = 0;
        while (lineIdx < bytesPerLine ) {

            ch = batom_cursor_next(&myCursor);
            if (!batom_cursor_eof(&myCursor)) {
                unsigned char   byteToPrint = ch;
                fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, "%c", isprint(byteToPrint)?byteToPrint:'.');
            }
            else {
                fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, " ");
            }
            lineIdx++;
        }

        /* fmtIdx += */ snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, "|");

        BDBG_LOG(("%s", fmtBuf));

        bufIdx += bytesPerLine;

    } while (!batom_cursor_eof(&myCursor));

    return;
}

/*****************************************************************************
 *  API to dump (in hex and ASCII) the contents of an atom.
 *****************************************************************************/
void
BIP_Atom_AtomDump(batom_t atom, const char *name)
{
    batom_cursor cursor;

    BDBG_ASSERT(atom);

    batom_cursor_from_atom(&cursor, atom);

    BIP_Atom_CursorDump(&cursor, name);
    return;
}
