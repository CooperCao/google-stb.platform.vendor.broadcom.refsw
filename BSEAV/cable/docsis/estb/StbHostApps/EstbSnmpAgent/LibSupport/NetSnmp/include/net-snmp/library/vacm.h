/*
// ****************************************************************************
//
// Copyright (c) 2007-2011 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
// ****************************************************************************
//  $Id$
//
//  Filename:       vacm.h
//
// ****************************************************************************
 * vacm.h
 *
 * SNMPv3 View-based Access Control Model
 */

#ifndef VACM_H
#define VACM_H

#ifdef __cplusplus
extern          "C" {
#endif

#define VACM_SUCCESS       0
#define VACM_NOSECNAME     1
#define VACM_NOGROUP       2
#define VACM_NOACCESS      3
#define VACM_NOVIEW        4
#define VACM_NOTINVIEW     5
#define VACM_NOSUCHCONTEXT 6
#define VACM_SUBTREE_UNKNOWN 7

#define SECURITYMODEL	1
#define SECURITYNAME	2
#define SECURITYGROUP	3
#define SECURITYSTORAGE	4
#define SECURITYSTATUS	5

#define ACCESSPREFIX	1
#define ACCESSMODEL	2
#define ACCESSLEVEL	3
#define ACCESSMATCH	4
#define ACCESSREAD	5
#define ACCESSWRITE	6
#define ACCESSNOTIFY	7
#define ACCESSSTORAGE	8
#define ACCESSSTATUS	9

#define VACMVIEWSPINLOCK 1
#define VIEWNAME	2
#define VIEWSUBTREE	3
#define VIEWMASK	4
#define VIEWTYPE	5
#define VIEWSTORAGE	6
#define VIEWSTATUS	7

// BRCM_MOD: define fields for vacmSecurityToGroupTable,
// represented by the vacm_groupEntry structure.
#define SECURITYMODEL     1
#define SECURITYNAME      2
#define GROUPNAME         3
#define GROUPSTORAGETYPE  4
#define GROUPSTATUS       5

/* BRCM_MOD  these are too short for CH */
#define VACM_MAX_STRING 34 /* 32 */
#define VACMSTRINGLEN   36 /* 34 */     /* VACM_MAX_STRING + 2 */

    struct vacm_groupEntry {
        int             securityModel;
        char            securityName[VACMSTRINGLEN];
        char            groupName[VACMSTRINGLEN];
        int             storageType;
        int             status;

        u_long          bitMask;
        struct vacm_groupEntry *reserved;
        struct vacm_groupEntry *next;
    };

#define CONTEXT_MATCH_EXACT  1
#define CONTEXT_MATCH_PREFIX 2
    struct vacm_accessEntry {
        char            groupName[VACMSTRINGLEN];
        char            contextPrefix[VACMSTRINGLEN];
        int             securityModel;
        int             securityLevel;
        int             contextMatch;
        char            readView[VACMSTRINGLEN];
        char            writeView[VACMSTRINGLEN];
        char            notifyView[VACMSTRINGLEN];
        int             storageType;
        int             status;

        u_long          bitMask;
        struct vacm_accessEntry *reserved;
        struct vacm_accessEntry *next;
    };

    struct vacm_viewEntry {
        char            viewName[VACMSTRINGLEN];
        oid             viewSubtree[MAX_OID_LEN];
        size_t          viewSubtreeLen;
        u_char          viewMask[VACMSTRINGLEN];
        size_t          viewMaskLen;
        int             viewType;
        int             viewStorageType;
        int             viewStatus;

        u_long          bitMask;

        struct vacm_viewEntry *reserved;
        struct vacm_viewEntry *next;
    };

    void            vacm_destroyViewEntry(const char *, oid *, size_t);
    void            vacm_destroyAllViewEntries(void);

#define VACM_MODE_FIND                0
#define VACM_MODE_IGNORE_MASK         1
#define VACM_MODE_CHECK_SUBTREE       2
    struct vacm_viewEntry *vacm_getViewEntry(const char *, oid *, size_t,
                                             int);
    /*
     * Returns a pointer to the viewEntry with the
     * same viewName and viewSubtree
     * Returns NULL if that entry does not exist.
     */

    int vacm_checkSubtree(const char *, oid *, size_t);

    /*
     * Check to see if everything within a subtree is in view, not in view,
     * or possibly both.
     *
     * Returns:
     *   VACM_SUCCESS          The OID is included in the view.
     *   VACM_NOTINVIEW        If no entry in the view list includes the
     *                         provided OID, or the OID is explicitly excluded
     *                         from the view. 
     *   VACM_SUBTREE_UNKNOWN  The entire subtree has both allowed and
     *                         disallowed portions.
     */

    void
                    vacm_scanViewInit(void);
    /*
     * Initialized the scan routines so that they will begin at the
     * beginning of the list of viewEntries.
     *
     */


    struct vacm_viewEntry *vacm_scanViewNext(void);
    /*
     * Returns a pointer to the next viewEntry.
     * These entries are returned in no particular order,
     * but if N entries exist, N calls to view_scanNext() will
     * return all N entries once.
     * Returns NULL if all entries have been returned.
     * view_scanInit() starts the scan over.
     */

    struct vacm_viewEntry *vacm_createViewEntry(const char *, oid *,
                                                size_t);
    /*
     * Creates a viewEntry with the given index
     * and returns a pointer to it.
     * The status of this entry is created as invalid.
     */

    void            vacm_destroyGroupEntry(int, const char *);
    void            vacm_destroyAllGroupEntries(void);
    struct vacm_groupEntry *vacm_createGroupEntry(int, const char *);
    struct vacm_groupEntry *vacm_getGroupEntry(int, const char *);
    void            vacm_scanGroupInit(void);
    struct vacm_groupEntry *vacm_scanGroupNext(void);

    void            vacm_destroyAccessEntry(const char *, const char *,
                                            int, int);
    void            vacm_destroyAllAccessEntries(void);
    struct vacm_accessEntry *vacm_createAccessEntry(const char *,
                                                    const char *, int,
                                                    int);
    struct vacm_accessEntry *vacm_getAccessEntry(const char *,
                                                 const char *, int, int);
    void            vacm_scanAccessInit(void);
    struct vacm_accessEntry *vacm_scanAccessNext(void);

    void            vacm_destroySecurityEntry(const char *);
    struct vacm_securityEntry *vacm_createSecurityEntry(const char *);
    struct vacm_securityEntry *vacm_getSecurityEntry(const char *);
    void            vacm_scanSecurityInit(void);
    struct vacm_securityEntry *vacm_scanSecurityEntry(void);
    int             vacm_is_configured(void);

    void            vacm_save(const char *token, const char *type);
    void            vacm_save_view(struct vacm_viewEntry *view,
                                   const char *token, const char *type);
    void            vacm_save_access(struct vacm_accessEntry *access_entry,
                                     const char *token, const char *type);
    void            vacm_save_group(struct vacm_groupEntry *group_entry,
                                    const char *token, const char *type);

    void            vacm_parse_config_view(const char *token, char *line);
    void            vacm_parse_config_group(const char *token, char *line);
    void            vacm_parse_config_access(const char *token,
                                             char *line);

    int             store_vacm(int majorID, int minorID, void *serverarg,
                               void *clientarg);


#ifdef __cplusplus
}
#endif
#endif                          /* VACM_H */
