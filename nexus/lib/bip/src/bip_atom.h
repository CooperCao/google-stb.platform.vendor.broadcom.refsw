/******************************************************************************
 * (c) 2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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
 *
 *****************************************************************************/

#ifndef BIP_ATOM_H
#define BIP_ATOM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bip_priv.h"

/**
 * Summary:
 * API to create an atom from an existing BIP_String.  The atom will then "own" the
 * BIP_String, and will destroy it when the atom's reference count goes to zero.
 *
 * Description:
 *
 **/
batom_t
BIP_Atom_AtomFromBipString(batom_factory_t factory, BIP_StringHandle hString);

/**
 * Summary:
 * API to create an atom from a range that has been allocated by B_Os_Malloc() or
 * B_Os_Calloc().  The atom will then "own" that range, and will free it with
 * B_Os_Free() when the atom's reference count goes to zero.
 *
 * Description:
 *
 **/
batom_t
BIP_Atom_AtomFromBOsMallocRange(batom_factory_t factory, void *base, size_t len);

/**
 * Summary:
 * API to create an atom by duplicating an given range.  This is intended for
 * creating atoms from some pointer being passed to an API.  Since the caller
 * owns the passed range, we will need to create a duplicate range for the
 * atom to own.
 *
 * Description:
 *
 **/
batom_t
BIP_Atom_AtomFromDupedRange(batom_factory_t factory, const void *base, size_t len);

/**
 * Summary:
 * API to print the contents of an atom that contains (mostly) printable
 * characters.
 *
 * Description:
 *
 **/
void
BIP_Atom_AtomPrint(batom_t atom, const char *name);

/**
 * Summary:
 * API to print the contents of a cursor that contains (mostly) printable
 * characters.
 *
 * Description:
 *
 **/
void
BIP_Atom_CursorPrint(batom_cursor *pCursor, const char *name);

/**
 * Summary:
 * API to print the contents of an atom that contains (mostly) printable
 * characters.
 *
 * Description:
 *
 **/
void
BIP_Atom_AtomDump(batom_t atom, const char *name);

/**
 * Summary:
 * API to print the contents of a cursor that contains (mostly) printable
 * characters.
 *
 * Description:
 *
 **/
void
BIP_Atom_CursorDump(batom_cursor *pCursor, const char *name);




#ifdef __cplusplus
}
#endif

#endif /* BIP_ATOM_H */
