/**************************************************************
**
**      Copyright (c) 2002-2009, Broadcom Corporation
**      All Rights Reserver
**      Confidential Property of Broadcom Corporation
**
**  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
**  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
**  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
**
**  $brcm_Workfile: $
**  $brcm_Revision: $
**  $brcm_Date: $
**
**
**  Module description:
**     Queue management primitives for BMC7041
**
**  Revision History:
**
**  $brcm_Log: $
** 
**************************************************************/

#ifndef BLST_SQUEUE_H
#define BLST_SQUEUE_H

/*================== Module Overview =====================================
This modules defines macros to control singly-linked queue.
All operations are typesafe (doesn't required typecasting) and constant time.

This list allow:
  o Insert a new entry at the head of the list
  o Insert a new entry to the tail of the list
  o Insert a new entry after any element in the list
  o O(1) removal of an entry from the list head
  o O(n) removal of any entry from the list
  o Forward traversal through the list
  o Each element requires one pointers
  o Code size and execution time is aboute 1.5 that for singly-linked list
  
  
========================================================================*/

/***************************************************************************
Summary:
    Creates new data type for the list head
	
Description:
    Creates new data type for the list head, this type used to create variable for the lists head.
    User should create new the list head data type for every different element datatype.
	
Input:
	field - name for the new data type (structure)
    type - existing user data type used for element of the list 

Example:    
     BLST_SQ_HEAD(block_head,  block);
     struct block_head  head;

Returns:
    <none>
****************************************************************************/
#define BLST_SQ_HEAD(itemname,type) struct itemname { struct type *sq_first, *sq_last;}

/***************************************************************************
Summary:
    Defines links entry
	
Description:
    Defines entrys for the list inside the user structure.for the element.
	
Input:
    type - the datatype for element

Example:
     struct block {
        BLST_SQ_ENTRY(block) link;
        char string[256];
     };  

Returns:
    <none>
****************************************************************************/
#define BLST_SQ_ENTRY(type) struct { struct type *sq_next; }

/***************************************************************************
Summary:
    Initializes lists head
	
Description:
    Initializes the head of the list. The head shall be initialized before list can be used.
    This macro used for dynamic initialization.
	
Input:
    phead - pointer to the list head

See also:
    BLST_SQ_INITIALIZER

Example:
    BLST_SQ_INIT(&head);

Returns:
    <none>
****************************************************************************/
#define BLST_SQ_INIT(phead)    ((phead)->sq_first = (phead)->sq_last = NULL)

/***************************************************************************
Summary:
    Initializes lists head
	
Description:
    Initializes the head of the list. The head shall be initialized before list can be used.
    This macro used for static initialization.
	
Input:
    phead - pointer to the list head

See also:
    BLST_S_INIT

Example:
    static struct block_head  head = BLST_S_INITIALIZER(head);

Returns:
    <none>
****************************************************************************/
#define BLST_SQ_INITIALIZER(phead)    {NULL, NULL}

/***************************************************************************
Summary:
    Tests if list is empty
	
Description:
    Tests if list is empty.
	
Input:
    phead - pointer to the list head

Returns:
    true - list empty
    false - list has elements

Example:
    if (BLST_SQ_EMPTY(&head) { return ; }

****************************************************************************/
#define BLST_SQ_EMPTY(phead) ((phead)->sq_first==NULL)


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
    struct block *first=BLST_SQ_FIRST(&head);
****************************************************************************/
#define BLST_SQ_FIRST(phead) ((phead)->sq_first)

/***************************************************************************
Summary:
    Returns the lists last element
	
Description:
    Returns pointer to the last element in the list
	
Input:
    phead - pointer to the list head

Returns:
    pointer to the last element int the list.

Example:
    struct block *last=BLST_SQ_LAST(&head);
****************************************************************************/
#define BLST_SQ_LAST(phead) ((phead)->sq_last)

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
    struct block *second=BLST_S_NEXT(first);
****************************************************************************/
#define BLST_SQ_NEXT(pitem, field) ((pitem)->field.sq_next)

/***************************************************************************
Summary:
    Inserts element into the list
	
Description:
    Inserts new element into the head of the list.
	
Input:
    phead - pointer to the list head
    elm - pointer to the new element
    field - name of the elements link field

Returns:
    <none>

Example:
    BLST_SQ_INSERT_HEAD(&head, new_block, link);
****************************************************************************/
#define BLST_SQ_INSERT_HEAD(phead, pitem, field) do { \
                (pitem)->field.sq_next = (phead)->sq_first; \
                if ((phead)->sq_last==NULL) { \
                    (phead)->sq_last = (pitem); \
                } \
                (phead)->sq_first = (pitem); \
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
    BLST_SQ_INSERT_AFTER(&head, first, second, link);
****************************************************************************/
#define BLST_SQ_INSERT_AFTER(head, elm, new_elm, field) do { \
        (new_elm)->field.sq_next = (elm)->field.sq_next; \
        if((elm)->field.sq_next == NULL) (head)->sq_last = (new_elm); \
        (elm)->field.sq_next = new_elm; \
      } while(0)

/***************************************************************************
Summary:
    Inserts element into the list

Description:
    Inserts new element into the tail list.

Input:
    phead - pointer to the list head
    elm - pointer to the new element
    field - name of the elements link field

Returns:
    <none>

Example:
    BLST_SQ_INSERT_TAIL(&head, elm, link);
****************************************************************************/
#define BLST_SQ_INSERT_TAIL(phead, pitem, field) do { \
                (pitem)->field.sq_next = NULL; \
                if ((phead)->sq_last) {        \
                        (phead)->sq_last->field.sq_next = pitem; \
                } else { \
                        (phead)->sq_first = pitem; \
                }\
                (phead)->sq_last = pitem; \
               } while(0)

/***************************************************************************
Summary:
    Removes element from the list
	
Description:
    Removes element from the head of the list.
	
Input:
    phead - pointer to the list head
    field - name of the elements link field

See also:
    BLST_SQ_REMOVE

Returns:
    <none>

Example:
    BLST_SQ_REMOVE_HEAD(&head, link);
****************************************************************************/
#define BLST_SQ_REMOVE_HEAD(phead, field) do { \
                if ((phead)->sq_first==(phead)->sq_last) { \
                        (phead)->sq_first=(phead)->sq_last=NULL;\
                } else { \
                        (phead)->sq_first = (phead)->sq_first->field.sq_next; \
                } \
               } while(0)

/***************************************************************************
Summary:
    Removes element from the list
	
Description:
    Removes element from the of the list. This implementation is O(n), 
    where n it's position of the element in the list
	
Input:
    phead - pointer to the list head
    elm - pointer to the list element
    type - datatype for an element of the list
    field - name of the elements link field

See also:
    BLST_S_REMOVE_HEAD

Returns:
    <none>

Example:
    BLST_SQ_REMOVE(&head, first, block, link);
****************************************************************************/
#define BLST_SQ_REMOVE(phead, pitem, type, field) do { \
                if ((phead)->sq_first == (pitem)) { \
                   BLST_SQ_REMOVE_HEAD(phead, field);\
                } else { \
                  struct type *blst_sq_remove_cur; \
                                    \
                  for (blst_sq_remove_cur=(phead)->sq_first; blst_sq_remove_cur->field.sq_next != (pitem); blst_sq_remove_cur = blst_sq_remove_cur->field.sq_next) {} \
                  blst_sq_remove_cur->field.sq_next=blst_sq_remove_cur->field.sq_next->field.sq_next; \
                  if (blst_sq_remove_cur->field.sq_next==NULL) { \
                    (phead)->sq_last = blst_sq_remove_cur; \
                  } \
                } \
               } while(0)

#endif /* BLST_SQUEUE_H */

