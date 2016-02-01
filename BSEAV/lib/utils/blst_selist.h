/***************************************************************************
 *     Copyright (c) 2007, Broadcom Corporation
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
 * Module Description:
 *
 * Single linked list embeddable list (e.g. heead is the same as any element)
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef __BLST_SELIST_H__
#define __BLST_SELIST_H__

#define BLST_SE_ENTRY(type) struct type *sel_next
#define BLST_SE_INIT(phead)    (phead)->sel_next = NULL
#define BLST_SE_INITIALIZER(head) { NULL }
#define BLST_SE_EMPTY(phead) ((phead)->sel_next==NULL)
#define BLST_SE_FIRST(phead) (phead)->sel_next
#define BLST_SE_NEXT(pitem) ((pitem)->sel_next)
#define BLST_SE_INSERT_AFTER(elm, new_elm) do { \
        (new_elm)->sel_next = (elm)->sel_next; \
        (elm)->sel_next = new_elm; \
      } while(0)
#define BLST_SE_REMOVE_BEFORE(before_elm, elm) do { \
        (before_elm)->sel_next = (elm)->sel_next; \
      } while(0)

#endif /* __BLST_SELIST_H__ */

