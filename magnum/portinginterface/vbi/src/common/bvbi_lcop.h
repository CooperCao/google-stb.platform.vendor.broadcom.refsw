/***************************************************************************
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
 *
 * Module Description:
 *   See Module Overview below
 *
 ***************************************************************************/

#ifndef BVBI_LCOP_H__
#define BVBI_LCOP_H__

/* Limited Counted Object Paradigm
 * -------------------------------
 *  Implements a simplified letter/envelope or "smart pointer" pattern. For
 *  details, see the paper
 *  .../DVTSJ/portinginterface/vbi/7400/teletext_problem.txt. It is in a
 *  ClearCase VOB.
 *
 *  Note: the bottom of this header contains a complete test program. It is
 *  only compiled if BVBI_P_LCOP_TEST_PROGRAM is defined.
 */

/*
 * Summary:
 * -------
 *  Defines a data type such that any struct that has this data type as a
 *  member becomes suitable for use as a counted object. Also used to define
 *  the free list.
 *
 *  Example:
 *  -------
 *      struct My_counted_object {
 *          ...
 *          BVBI_P_LCOP_COUNTEDPROP(My_counted_object) link;
 *      };
 *      ...
 *      BVBI_P_LCOP_COUNTEDPROP(My_counted_object) freelist;
 */
#define BVBI_P_LCOP_COUNTEDPROP(co_type) \
    union { struct co_type* l_co; unsigned int i_count; }

/*
 * Summary:
 * -------
 *  For a given counted object type, creates a new data type such that any
 *  struct that has the new data type as a member becomes suitable for use as
 *  an owner object.
 *
 *  Example:
 *  -------
 *      struct My_counted_object {
 *          ...
 *      };
 *      ...
 *      struct My_owner_object {
 *          ...
 *          BVBI_P_LCOP_OWNERPROP(My_counted_object) link;
 *      };
 */
#define BVBI_P_LCOP_OWNERPROP(co_type) \
    struct { struct co_type* l_co; }

/*
 * Summary:
 * -------
 * Initialize the free list (to empty state).
 *
 * Example:
 * -------
 *     struct My_counted_object {
 *         ...
 *     };
 *     BVBI_P_LCOP_COUNTEDPROP(My_counted_object) freelist;
 *     ...
 *     BVBI_P_LCOP_INITFREELIST_isr (&freelist);
 */
#define BVBI_P_LCOP_INITFREELIST_isr(freelist) \
    ((freelist)->l_co = NULL)
#define BVBI_P_LCOP_INITFREELIST(freelist) \
    do { \
    BKNI_EnterCriticalSection(); \
    (BVBI_P_LCOP_INITFREELIST_isr(freelist)); \
    BKNI_LeaveCriticalSection(); \
    } while (0)

/*
 * Summary:
 * -------
 *  Obtain address of counted object owned by an owner.
 *
 *  Example:
 *  -------
 *      struct My_counted_object {
 *          ...
 *      };
 *      struct My_owner_object {
 *          ...
 *          BVBI_P_LCOP_OWNERPROP(My_counted_object) link;
 *      };
 *      struct My_owner_object owner;
 *      ...
 *      struct My_counted_object* counted = BVBI_P_LCOP_GET_isr (&owner, link);
 */
#define BVBI_P_LCOP_GET_isr(owner,owner_link) \
    ((owner)->owner_link.l_co)
#define BVBI_P_LCOP_GET(owner,owner_link) \
    (BVBI_P_LCOP_GET_isr((owner),owner_link))

/*
 * Summary:
 * -------
 * Operation: create.
 *
 * Example:
 * -------
 *      struct My_counted_object {
 *          ...
 *          BVBI_P_LCOP_COUNTED_OBJECT(linkc);
 *      };
 *      struct My_owner_object {
 *          ...
 *          BVBI_P_LCOP_OWNERPROP(My_counted_object) linko;
 *      };
 *      struct My_owner_object owner;
 *      struct My_counted_object counted;
 *      ...
 *      BVBI_P_LCOP_CREATE_isr (&owner, linko, &counted, linkc);
 */
#define \
BVBI_P_LCOP_CREATE_isr(owner_object,owner_link,counted_object,counted_link) \
    do { \
    (owner_object)->owner_link.l_co = (counted_object); \
    (counted_object)->counted_link.i_count = 1; \
    } \
    while (0)
#define \
BVBI_P_LCOP_CREATE(owner_object,owner_link,counted_object,counted_link) \
    do { \
    BKNI_EnterCriticalSection(); \
    BVBI_P_LCOP_CREATE_isr( \
        (owner_object),owner_link,(counted_object),counted_link); \
    BKNI_LeaveCriticalSection(); \
    } while (0)

/*
 * Summary:
 * -------
 * Operation: destroy. After calling, the user should recycle both the
 * owner object and its pointed-to counted object. This function
 * should be paired with calls to BVBI_P_LCOP_CREATE_isr(), for the
 * same pair of objects.
 *
 * Example:
 * -------
 *      struct My_counted_object {
 *          ...
 *          BVBI_P_LCOP_COUNTEDPROP(My_counted_object) linkc;
 *      };
 *      struct My_owner_object {
 *          ...
 *          BVBI_P_LCOP_OWNERPROP(My_counted_object) linko;
 *      };
 *      BVBI_P_LCOP_COUNTEDPROP(My_counted_object) freelist;
 *      struct My_owner_object* pOwner;
 *      ...
 *      BVBI_P_LCOP_DESTROY_isr (pOwner, linko, &freelist, linkc);
 *      free (BVBI_P_LCOP_GET_isr(pOwner,linko));
 *      free (pOwner);
 */
#define BVBI_P_LCOP_DESTROY_isr(owner,owner_link,freelist,counted_link) \
    do { \
    BDBG_ASSERT ((owner)->owner_link.l_co->counted_link.i_count); \
    if ((owner)->owner_link.l_co->counted_link.i_count > 1) \
    { \
        BDBG_ASSERT ((freelist)->l_co); \
        --((owner)->owner_link.l_co->counted_link.i_count); \
        (owner)->owner_link.l_co = (freelist)->l_co; \
        (freelist)->l_co = (freelist)->l_co->counted_link.l_co; \
        (owner)->owner_link.l_co->counted_link.i_count = 1; \
    } \
    } while (0)
#define BVBI_P_LCOP_DESTROY(owner,owner_link,freelist,counted_link) \
    do { \
    BKNI_EnterCriticalSection(); \
    BVBI_P_LCOP_DESTROY_isr((owner),owner_link,(freelist),counted_link); \
    BKNI_LeaveCriticalSection(); \
    } while (0)

/*
 * Summary:
 * -------
 *  Operation: write.
 *
 *  Example:
 *  -------
 *      struct My_counted_object {
 *          ...
 *          BVBI_P_LCOP_COUNTED_OBJECT(My_counted_object) linkc;
 *      };
 *      struct My_owner_object {
 *          ...
 *          BVBI_P_LCOP_OWNERPROP(My_counted_object) linko;
 *      };
 *      BVBI_P_LCOP_COUNTEDPROP(My_counted_object) freelist;
 *      struct My_owner_object owner;
 *      ...
 *      BVBI_P_LCOP_WRITE_isr (&owner, linko, &freelist, linkc);
 */
#define BVBI_P_LCOP_WRITE_isr(owner,owner_link,freelist,counted_link) \
    BVBI_P_LCOP_DESTROY_isr((owner),owner_link,(freelist),counted_link)
#define BVBI_P_LCOP_WRITE(owner,owner_link,freelist,counted_link) \
    do { \
    BKNI_EnterCriticalSection(); \
    BVBI_P_LCOP_WRITE_isr((owner),owner_link,(freelist),counted_link); \
    BKNI_LeaveCriticalSection(); \
    } while (0)

/*
 * Summary:
 * -------
 *  Operation: Copy
 *
 *  Example:
 *  -------
 *      struct My_counted_object_from {
 *          ...
 *          BVBI_P_LCOP_COUNTED_OBJECT(My_counted_object) fromlink;
 *      };
 *      struct My_counted_object_to {
 *          ...
 *          BVBI_P_LCOP_COUNTED_OBJECT(My_counted_object) tolink;
 *      };
 *      struct My_owner_object {
 *          ...
 *          BVBI_P_LCOP_OWNERPROP(My_counted_object) linko;
 *      };
 *      BVBI_P_LCOP_COUNTEDPROP(My_counted_object) freelist;
 *      struct My_owner_object_from from_owner;
 *      struct My_owner_object_to   to_owner;
 *      ...
 *      BVBI_P_LCOP_COPY_isr (
 *          &from_owner, fromlink, &to_owner, tolink, &freelist, linkc);
 */
#define \
BVBI_P_LCOP_COPY_isr(\
from_owner,from_link,to_owner,to_link,freelist,counted_link) \
    do { \
    if (((to_owner)->to_link.l_co) != ((from_owner)->from_link.l_co)) \
    { \
        BDBG_ASSERT ((from_owner)->from_link.l_co->counted_link.i_count); \
        BDBG_ASSERT ((to_owner)->to_link.l_co->counted_link.i_count); \
        if (--((to_owner)->to_link.l_co->counted_link.i_count) == 0) \
        { \
            (to_owner)->to_link.l_co->counted_link.l_co = (freelist)->l_co; \
            (freelist)->l_co = (to_owner)->to_link.l_co; \
        } \
        (to_owner)->to_link.l_co = (from_owner)->from_link.l_co; \
        ++((from_owner)->from_link.l_co->counted_link.i_count); \
    } \
    } while (0)
#define \
BVBI_P_LCOP_COPY(\
from_owner,from_link,to_owner,to_link,freelist,counted_link) \
    do { \
    BKNI_EnterCriticalSection(); \
    BVBI_P_LCOP_COPY_isr( \
        (from_owner),from_link,(to_owner),to_link,(freelist),counted_link); \
    BKNI_LeaveCriticalSection(); \
    } while (0)

/*
 * Summary:
 * -------
 *  Operation: The transfer sequence. Operations copy, then write.
 *
 *  Example:
 *  -------
 *      struct My_counted_object_from {
 *          ...
 *          BVBI_P_LCOP_COUNTED_OBJECT(My_counted_object) fromlink;
 *      };
 *      struct My_counted_object_to {
 *          ...
 *          BVBI_P_LCOP_COUNTED_OBJECT(My_counted_object) tolink;
 *      struct My_owner_object {
 *          ...
 *          BVBI_P_LCOP_OWNERPROP(My_counted_object) linko;
 *      };
 *      BVBI_P_LCOP_COUNTEDPROP(My_counted_object) freelist;
 *      struct My_owner_object_from from_owner;
 *      struct My_owner_object_to   to_owner;
 *      ...
 *      BVBI_P_LCOP_TRANSFER (
 *          &from_owner, fromlink, &to_owner, tolink, &freelist, linkc);
 */
#define \
BVBI_P_LCOP_TRANSFER_isr(\
from_owner,from_link,to_owner,to_link,freelist,counted_link) \
    do { \
    BVBI_P_LCOP_COPY_isr( \
        (from_owner),from_link,(to_owner),to_link,(freelist),counted_link); \
    BVBI_P_LCOP_WRITE_isr( \
        (from_owner),from_link,(freelist),counted_link); \
    } while (0)
#define \
BVBI_P_LCOP_TRANSFER(\
from_owner,from_link,to_owner,to_link,freelist,counted_link) \
    do { \
    BKNI_EnterCriticalSection(); \
    BVBI_P_LCOP_TRANSFER_isr( \
        (from_owner),from_link,(to_owner),to_link,(freelist),counted_link); \
    BKNI_LeaveCriticalSection(); \
    } while (0)

/*
 * Here is the test program for this set of tools. The following preprocessor
 * symbol BVBI_P_LCOP_TEST_PROGRAM should never be defined.
 */
#ifdef BVBI_P_LCOP_TEST_PROGRAM /** { **/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#define NULL (0x0)
#include "bvbi_lcop.h"

#define MALLOC(struct_tag) \
    ((struct struct_tag*)malloc (sizeof(struct struct_tag)))
#define FREE(object) (free ((void*)object));
#define BDBG_ASSERT(cond) (assert(cond))

typedef struct
{
    int whatever[10];
} Wongo;
typedef struct
{
    char whomever[20];
} Tongo;

struct My_counted
{
    int sn;
    int version;
    Wongo fff[1024];
    BVBI_P_LCOP_COUNTEDPROP(My_counted) linkc;
    int iii[88];
};

struct My_owner
{
    Tongo ggg[99];
    BVBI_P_LCOP_OWNERPROP(My_counted) linko;
    long jjj[24];
};

static BVBI_P_LCOP_OWNERPROP(My_counted) fl;

int main (int argc, char* argv[])
{
    struct My_counted* c1;
    struct My_counted* c2;
    struct My_counted* c3;
    struct My_owner*   o1;
    struct My_owner*   o2;
    struct My_owner*   o3;
    struct My_counted* cn;
    int version = 0;
    int sn = 0;

    BVBI_P_LCOP_INITFREELIST_isr (&fl);

    /* Create object 1 */
    o1 = MALLOC (My_owner);
    c1 = MALLOC (My_counted);
    c1->sn = ++sn;
    c1->version = ++version;
    BVBI_P_LCOP_CREATE_isr (o1, linko, c1, linkc);
    cn = BVBI_P_LCOP_GET_isr (o1, linko);
    printf ("After create, object 1 sn %d, version %d\n", cn->sn, cn->version);

    /* Create object 2 */
    o2 = MALLOC (My_owner);
    c2 = MALLOC (My_counted);
    c2->sn = ++sn;
    c2->version = ++version;
    BVBI_P_LCOP_CREATE_isr (o2, linko, c2, linkc);
    cn = BVBI_P_LCOP_GET_isr (o2, linko);
    printf ("After create, object 2 sn %d, version %d\n", cn->sn, cn->version);

    /* Create object 3 */
    o3 = MALLOC (My_owner);
    c3 = MALLOC (My_counted);
    c3->sn = ++sn;
    c3->version = ++version;
    BVBI_P_LCOP_CREATE_isr (o3, linko, c3, linkc);
    cn = BVBI_P_LCOP_GET_isr (o3, linko);
    printf ("After create, object 3 sn %d, version %d\n", cn->sn, cn->version);

    /* Write (modify) object 1 */
    BVBI_P_LCOP_WRITE_isr (o1, linko, &fl, linkc);
    cn = BVBI_P_LCOP_GET_isr (o1, linko);
    cn->version = ++version;
    cn = BVBI_P_LCOP_GET_isr (o1, linko);
    printf ("After write, object 1 sn %d, version %d\n", cn->sn, cn->version);
    cn = BVBI_P_LCOP_GET_isr (o2, linko);
    printf ("             object 2 sn %d, version %d\n", cn->sn, cn->version);

    /* Copy object 1 to object 2 */
    BVBI_P_LCOP_COPY_isr (o1, linko, o2, linko, &fl, linkc);
    cn = BVBI_P_LCOP_GET_isr (o1, linko);
    printf ("After copy, object 1 sn %d, version %d\n", cn->sn, cn->version);
    cn = BVBI_P_LCOP_GET_isr (o2, linko);
    printf ("            object 2 sn %d, version %d\n", cn->sn, cn->version);

    /* Copy object 2 to object 3 */
    BVBI_P_LCOP_COPY_isr (o2, linko, o3, linko, &fl, linkc);
    cn = BVBI_P_LCOP_GET_isr (o2, linko);
    printf ("After copy, object 2 sn %d, version %d\n", cn->sn, cn->version);
    cn = BVBI_P_LCOP_GET_isr (o3, linko);
    printf ("            object 3 sn %d, version %d\n", cn->sn, cn->version);

    /* Transfer object 2 to object 1, then modify object 2. */
    BVBI_P_LCOP_TRANSFER_isr (o2, linko, o1, linko, &fl, linkc);
    cn = BVBI_P_LCOP_GET_isr (o2, linko);
    cn->version = ++version;
    cn = BVBI_P_LCOP_GET_isr (o1, linko);
    printf (
        "After transfer, object 1 sn %d, version %d\n", cn->sn, cn->version);
    cn = BVBI_P_LCOP_GET_isr (o2, linko);
    printf (
        "                object 2 sn %d, version %d\n", cn->sn, cn->version);

    BVBI_P_LCOP_DESTROY_isr (o1, linko, &fl, linkc);
    FREE (BVBI_P_LCOP_GET_isr (o1, linko));
    FREE (o1);

    BVBI_P_LCOP_DESTROY_isr (o2, linko, &fl, linkc);
    FREE (BVBI_P_LCOP_GET_isr (o2, linko));
    FREE (o2);

    BVBI_P_LCOP_DESTROY_isr (o3, linko, &fl, linkc);
    FREE (BVBI_P_LCOP_GET_isr (o3, linko));
    FREE (o3);

    return 0;
}

#endif /** } BVBI_P_LCOP_TEST_PROGRAM **/

#endif /**  BVBI_LCOP_H__ */
