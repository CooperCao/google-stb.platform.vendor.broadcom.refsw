/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 ***************************************************************************/

#ifndef BMUXLIB_ASP_SLIST_H_
#define BMUXLIB_ASP_SLIST_H_

/*================== Module Overview =====================================
This modules defines macros to control singly-linked list.
All operations are typesafe (doesn't required typecasting) and constant time.

This list allow:
  o Insert a new entry at the head of the list
  o Insert a new entry after any element in the list
  o O(1) removal of an entry from the list head
  o O(n) removal of any entry from the list
  o Forward traversal through the list
  o Each element requires one pointers
  o Code size and execution time is the smallest and fastest compared to other lists

========================================================================*/

/***************************************************************************
Summary:
    Creates new data type for the list head

Description:
    Creates new data type for the list head, this type used to create variable for the lists head.
    User should create new the list head data type for every different element datatype.

Input:
    name - name for the new data type (structure)
    type - existing user data type used for element of the list

Example:
     BMUXLIB_ASP_LST_S_HEAD(block_head, block);
     struct block_head  head;

Returns:
    <none>
****************************************************************************/
#define BMUXLIB_ASP_LST_S_HEAD(name, type) struct name { struct type *sl_first;}

/***************************************************************************
Summary:
    Defines links entry

Description:
    Defines entrys for the list inside the user structure.for the element.

Input:
    type - the datatype for element

Example:
     struct block {
        BMUXLIB_ASP_LST_S_ENTRY(block) link;
        char string[256];
     };

Returns:
    <none>
****************************************************************************/
#define BMUXLIB_ASP_LST_S_ENTRY(type) struct { struct type *sl_next; }

/***************************************************************************
Summary:
    Initializes lists head

Description:
    Initializes the head of the list. The head shall be initialized before list can be used.
    This macro used for dynamic initialization.

Input:
    phead - pointer to the list head

See also:
    BMUXLIB_ASP_LST_S_INITIALIZER

Example:
    BMUXLIB_ASP_LST_S_INIT(&head);

Returns:
    <none>
****************************************************************************/
#define BMUXLIB_ASP_LST_S_INIT(phead)    (phead)->sl_first = NULL

/***************************************************************************
Summary:
    Initializes lists head

Description:
    Initializes the head of the list. The head shall be initialized before list can be used.
    This macro used for static initialization.

Input:
    head - pointer to the list head

See also:
    BMUXLIB_ASP_LST_S_INIT

Example:
    static struct block_head  head = BMUXLIB_ASP_LST_S_INITIALIZER(head);

Returns:
    <none>
****************************************************************************/
#define BMUXLIB_ASP_LST_S_INITIALIZER(head) { NULL }


/***************************************************************************
Summary:
    Tests if list is empty

Description:
    Tests if list is empty.

Input:
    phead - pointer to the list head

Returns:
    TRUE - list empty
    false - list has elements

Example:
    if (BMUXLIB_ASP_LST_S_EMPTY(&head) { return ; }

****************************************************************************/
#define BMUXLIB_ASP_LST_S_EMPTY(phead) ((phead)->sl_first==NULL)

/***************************************************************************
Summary:
    Returns the lists first element

Description:
    Returns pointer to the first element from the list

Input:
    phead - pointer to the list head

Returns:
    pointer to the first element from the list.

Example:
    struct block *first=BMUXLIB_ASP_LST_S_FIRST(&head);
****************************************************************************/
#define BMUXLIB_ASP_LST_S_FIRST(phead) (phead)->sl_first

/***************************************************************************
Summary:
    Returns next element from the lists

Description:
    Returns pointer to the next element from the list

Input:
    elm - pointer to the list element
    field - name of the elements link field

Returns:
    pointer to the next element from the list

Example:
    struct block *second=BMUXLIB_ASP_LST_S_NEXT(first, link);
****************************************************************************/
#define BMUXLIB_ASP_LST_S_NEXT(pitem, field) (pitem->field.sl_next)

/***************************************************************************
Summary:
    Inserts element into the list

Description:
    Inserts new element into the head of the list.

Input:
    phead - pointer to the list head
    pitem - pointer to the new element
    field - name of the elements link field

Returns:
    <none>

Example:
    BMUXLIB_ASP_LST_S_INSERT_HEAD(&head, new_block, link);
****************************************************************************/
#define BMUXLIB_ASP_LST_S_INSERT_HEAD(phead, pitem, field) do { \
                (pitem)->field.sl_next = (phead)->sl_first; \
                (phead)->sl_first = (pitem); \
               } while(0)

/***************************************************************************
Summary:
    Inserts element into the list

Description:
    Inserts new element after existing element.

Input:
    head - pointer to the list head
    elm - pointer to the element from the list
    new_elm - pointer to the new element
    field - name of the elements link field

Returns:
    <none>

Example:
    BMUXLIB_ASP_LST_S_INSERT_AFTER(&head, first, second, link);
****************************************************************************/
#define BMUXLIB_ASP_LST_S_INSERT_AFTER(head, elm, new_elm, field) do { \
        (new_elm)->field.sl_next = (elm)->field.sl_next; \
        (elm)->field.sl_next = new_elm; \
      } while(0)


/***************************************************************************
Summary:
    Removes element from the list

Description:
    Removes element from the head of the list.

Input:
    head - pointer to the list head
    field - name of the elements link field

See also:
    BMUXLIB_ASP_LST_S_REMOVE

Returns:
    <none>

Example:
    BMUXLIB_ASP_LST_S_REMOVE_HEAD(&head, link);
****************************************************************************/
#define BMUXLIB_ASP_LST_S_REMOVE_HEAD(phead, field) (phead)->sl_first = (phead)->sl_first->field.sl_next


/***************************************************************************
Summary:
    Removes element from the list

Description:
    Removes element from the of the list. This implementation is O(n),
    where n it's position of the element in the list

Input:
    head - pointer to the list head
    elm - pointer to the list element
    type - datatype for an element of the list
    field - name of the elements link field

See also:
    BMUXLIB_ASP_LST_S_REMOVE_HEAD

Returns:
    <none>

Example:
    BMUXLIB_ASP_LST_S_REMOVE(&head, first, block, link);
****************************************************************************/
#define BMUXLIB_ASP_LST_S_REMOVE(phead, elm, type, field) do { \
        if ((phead)->sl_first == (elm)) { \
             BMUXLIB_ASP_LST_S_REMOVE_HEAD(phead, field);\
        } else { \
            struct type *BMUXLIB_ASP_LST_S_remove_cur; \
                                    \
            for (BMUXLIB_ASP_LST_S_remove_cur=(phead)->sl_first; BMUXLIB_ASP_LST_S_remove_cur->field.sl_next != (elm); BMUXLIB_ASP_LST_S_remove_cur = BMUXLIB_ASP_LST_S_remove_cur->field.sl_next) {} \
            BMUXLIB_ASP_LST_S_remove_cur->field.sl_next=BMUXLIB_ASP_LST_S_remove_cur->field.sl_next->field.sl_next; \
        } \
    } while(0)

/***************************************************************************
Summary:
    Removes element from the list

Description:
    Removes next element from the list.

Input:
    head - pointer to the list head
    elm - pointer to the element from the list
    field - name of the elements link field

See also:
    BMUXLIB_ASP_LST_S_REMOVE

Returns:
    <none>

Example:
    BMUXLIB_ASP_LST_S_REMOVE_NEXT(elm, link);
****************************************************************************/
#define BMUXLIB_ASP_LST_S_REMOVE_NEXT(head, elm, field) \
           elm->field.sl_next=elm->field.sl_next->field.sl_next;

/***************************************************************************
Summary:
    Adds new entry to the list in ascending order

Description:
    Adds new element to the list implemented as sorted single linked list

Input:
    head - pointer to the list head
    elm - pointer to the element from the list
    type - datatype for an element of the list
    key - name of the key element
    link - name of the elements link field

Returns:
    <none>

Example:
    BMUXLIB_ASP_LST_S_ADD(head, elm, block, key, link);
****************************************************************************/

#define BMUXLIB_ASP_LST_S_ADD(head, elm, type, fcmp, link) do { \
    struct type *BMUXLIB_ASP_LST_S_dict_add_i, *BMUXLIB_ASP_LST_S_dict_add_prev; \
    for(BMUXLIB_ASP_LST_S_dict_add_prev=NULL,BMUXLIB_ASP_LST_S_dict_add_i=BMUXLIB_ASP_LST_S_FIRST((head));BMUXLIB_ASP_LST_S_dict_add_i!=NULL;BMUXLIB_ASP_LST_S_dict_add_prev=BMUXLIB_ASP_LST_S_dict_add_i,BMUXLIB_ASP_LST_S_dict_add_i=BMUXLIB_ASP_LST_S_NEXT(BMUXLIB_ASP_LST_S_dict_add_i, link)) { \
        if((fcmp)((elm), BMUXLIB_ASP_LST_S_dict_add_i) < 0 ) {  break;} \
    } \
    if(BMUXLIB_ASP_LST_S_dict_add_prev) { BMUXLIB_ASP_LST_S_INSERT_AFTER((head), BMUXLIB_ASP_LST_S_dict_add_prev, (elm), link);} \
    else { BMUXLIB_ASP_LST_S_INSERT_HEAD(head, (elm), link);}\
    } while(0)


/***************************************************************************
Summary:
    Adds new entry to the dictionary

Description:
    Adds new element to the dictionary implemented as sordted single linked list

Input:
    head - pointer to the list head
    elm - pointer to the element from the list
    type - datatype for an element of the list
    key - name of the key element
    link - name of the elements link field
    duplicate - label that would be used (gotoed) if duplicate element is found

See also:
    BMUXLIB_ASP_LST_S_DICT_FIND
    BMUXLIB_ASP_LST_S_DICT_REMOVE

Returns:
    <none>

Example:
    BMUXLIB_ASP_LST_S_DICT_ADD(head, elm, block, key, link, err_duplicate);
****************************************************************************/
#define BMUXLIB_ASP_LST_S_DICT_ADD(head, elm, type, key, link, duplicate) do { \
    struct type *BMUXLIB_ASP_LST_S_dict_add_i, *BMUXLIB_ASP_LST_S_dict_add_prev; \
    for(BMUXLIB_ASP_LST_S_dict_add_prev=NULL,BMUXLIB_ASP_LST_S_dict_add_i=BMUXLIB_ASP_LST_S_FIRST((head));BMUXLIB_ASP_LST_S_dict_add_i!=NULL;BMUXLIB_ASP_LST_S_dict_add_prev=BMUXLIB_ASP_LST_S_dict_add_i,BMUXLIB_ASP_LST_S_dict_add_i=BMUXLIB_ASP_LST_S_NEXT(BMUXLIB_ASP_LST_S_dict_add_i, link)) { \
        if((elm)->key > BMUXLIB_ASP_LST_S_dict_add_i->key) {  break;} \
        if((elm)->key == BMUXLIB_ASP_LST_S_dict_add_i->key) { goto duplicate; } \
    } \
    if(BMUXLIB_ASP_LST_S_dict_add_prev) { BMUXLIB_ASP_LST_S_INSERT_AFTER((head), BMUXLIB_ASP_LST_S_dict_add_prev, (elm), link);} \
    else { BMUXLIB_ASP_LST_S_INSERT_HEAD(head, (elm), link);}\
    } while(0)

/***************************************************************************
Summary:
    Finds element in the dictionary

Description:
    Finds element in the dictinary with matching key

Input:
    head - pointer to the list head
    elm - pointer to the element from the list
    key - value of key to find
    field - name of the key element
    link - name of the elements link field

See also:
    BMUXLIB_ASP_LST_S_DICT_ADD
    BMUXLIB_ASP_LST_S_DICT_REMOVE

Returns:
    elm = NULL, if element not found in the list
    elm - pointer to the found element

Example:
    BMUXLIB_ASP_LST_S_DICT_FIND(head, elm, 123, key, link);
****************************************************************************/
#define BMUXLIB_ASP_LST_S_DICT_FIND(head, elm, key, field, link) do { \
    for((elm) = BMUXLIB_ASP_LST_S_FIRST(head); (elm)!=NULL ; (elm) = BMUXLIB_ASP_LST_S_NEXT((elm), link)) { \
        if(key == (elm)->field )  { break; } \
        else if( key > (elm)->field ) { (elm) = NULL;break; } } } while(0)


/***************************************************************************
Summary:
    Removed element in the dictionary

Description:
    Removes element in the dictinary with matching key

Input:
    head - pointer to the list head
    elm - pointer to the element from the list
    type - datatype for an element of the list
    key - value of key to find
    field - name of the key element
    link - name of the elements link field

See also:
    BMUXLIB_ASP_LST_S_DICT_ADD
    BMUXLIB_ASP_LST_S_DICT_FIND

Returns:
    elm = NULL, if element not found in the list
    elm - pointer to the found element

Example:
    BMUXLIB_ASP_LST_S_DICT_REMOVE(head, elm, 123, block, key, link);
****************************************************************************/
#define BMUXLIB_ASP_LST_S_DICT_REMOVE(head, elm_, key, type, field, link) do { \
    struct type *BMUXLIB_ASP_LST_S_dict_remove_prev, *BMUXLIB_ASP_LST_S_dict_remove_elm; \
    for(BMUXLIB_ASP_LST_S_dict_remove_prev=NULL,(BMUXLIB_ASP_LST_S_dict_remove_elm)=BMUXLIB_ASP_LST_S_FIRST((head));(BMUXLIB_ASP_LST_S_dict_remove_elm)!=NULL;BMUXLIB_ASP_LST_S_dict_remove_prev=BMUXLIB_ASP_LST_S_dict_remove_elm, BMUXLIB_ASP_LST_S_dict_remove_elm=BMUXLIB_ASP_LST_S_NEXT((BMUXLIB_ASP_LST_S_dict_remove_elm), link)) { \
        if( (key) == (BMUXLIB_ASP_LST_S_dict_remove_elm)->field ) { \
            if(BMUXLIB_ASP_LST_S_dict_remove_prev) { BMUXLIB_ASP_LST_S_REMOVE_NEXT((head), BMUXLIB_ASP_LST_S_dict_remove_prev, link); } \
            else { BMUXLIB_ASP_LST_S_REMOVE_HEAD((head), link);} break; \
        } else if( key > BMUXLIB_ASP_LST_S_dict_remove_elm->field ) { BMUXLIB_ASP_LST_S_dict_remove_elm = NULL;break; } \
    } (elm_)=BMUXLIB_ASP_LST_S_dict_remove_elm;} while(0)




#endif /* BMUXLIB_ASP_SLIST_H_ */
