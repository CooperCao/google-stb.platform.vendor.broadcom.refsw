/***************************************************************************
 *     Copyright (c) 2003-2007, Broadcom Corporation
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
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BLST_CIRCLEQ_H
#define BLST_CIRCLEQ_H   

/*================== Module Overview =====================================
This modules defines macros to control doubly linked queue. 
All operations are typesafe (doesn't required typecasting) and constant time.

This list allow:
  o Insert a new entry at the head of the list
  o Insert a new entry after/before any element in the list
  o O(1) removal of any entry in the list
  o Forward traversal through the list
  o Each element requires two pointers
  o Code size and execution time is about 3.0 that for singly-linked list
  
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
     BLST_CQ_HEAD(block_head, block);
     struct block_head  head;

Returns:
    <none>
****************************************************************************/
#define BLST_CQ_HEAD(name, type) struct name { struct type *cq_first, *cq_last; }

/***************************************************************************
Summary:
    Defines links entry
	
Description:
    Defines entrys for the list inside the user structure.for the element.
	
Input:
    type - the datatype for element

Example:
     struct block {
        BLST_CQ_ENTRY(block) link;
        char string[256];
     };  

Returns:
    <none>
****************************************************************************/
#define BLST_CQ_ENTRY(type)  struct { struct type *cq_next, *cq_prev; }

/***************************************************************************
Summary:
    Initializes lists head
	
Description:
    Initializes the head of the list. The head shall be initialized before list can be used.
    This macro used for dynamic initialization.
	
Input:
    head - pointer to the list head

See also:
    BLST_CQ_INITIALIZER

Example:
    BLST_CQ_INIT(&head);

Returns:
    <none>
****************************************************************************/
#define BLST_CQ_INIT(head) ((head)->cq_last = (head)->cq_first=NULL)

/***************************************************************************
Summary:
    Initializes lists head
	
Description:
    Initializes the head of the list. The head shall be initialized before list can be used.
    This macro used for static initialization.
	
Input:
    head - pointer to the list head

See also:
    BLST_CQ_INIT

Example:
    static struct block_head  head = BLST_CQ_INITIALIZER(&head);

Returns:
    <none>
****************************************************************************/
#define BLST_CQ_INITIALIZER(head) {NULL, NULL}

/***************************************************************************
Summary:
    Tests if list is empty
	
Description:
    Tests if list is empty.
	
Input:
    head - pointer to the list head

Returns:
    true - list empty
    false - list has elements

Example:
    if (BLST_CQ_EMPTY(&head)) { return ; }

****************************************************************************/
#define BLST_CQ_EMPTY(head) ((head)->cq_first == NULL)

/***************************************************************************
Summary:
    Returns the lists first element
	
Description:
    Returns pointer to the first element from the list
	
Input:
    head - pointer to the list head

Returns:
    pointer to the first element from the list.

Example:
    struct block *first=BLST_CQ_FIRST(&head);
****************************************************************************/
#define BLST_CQ_FIRST(head) ((head)->cq_first)

/***************************************************************************
Summary:
    Returns the lists first element
	
Description:
    Returns pointer to the last element from the list
	
Input:
    head - pointer to the list head

Returns:
    pointer to the last element from the list.

Example:
    struct block *first=BLST_CQ_FIRST(&head);
****************************************************************************/
#define BLST_CQ_LAST(head) ((head)->cq_last)

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
    struct block *second=BLST_CQ_NEXT(first);
****************************************************************************/
#define BLST_CQ_NEXT(elm, field) ((elm)->field.cq_next)


/***************************************************************************
Summary:
    Returns next element from the lists
	
Description:
    Returns pointer to the previous element from the list
	
Input:
    elm - pointer to the list element
    field - name of the elements link field

Returns:
    pointer to the previous element from the list

Example:
    struct block *first=BLST_CQ_PREV(second);
****************************************************************************/
#define BLST_CQ_PREV(elm, field) ((elm)->field.cq_prev)

/***************************************************************************
Summary:
    Inserts element into the list
	
Description:
    Inserts new element at the head of the list.
	
Input:
    head - pointer to the list head
    elm - pointer to the new element
    field - name of the elements link field

Returns:
    <none>

Example:
    BLST_Q_INSERT_HEAD(&head, new_block, link);
****************************************************************************/
#define BLST_CQ_INSERT_HEAD(head, elm, field) do { \
   if ( (head)->cq_first != NULL ) { \
        (elm)->field.cq_next = (head)->cq_first; \
        (elm)->field.cq_prev = (head)->cq_last; \
        (head)->cq_first->field.cq_prev = (head)->cq_last->field.cq_next = (elm); \
   } else { \
     (elm)->field.cq_prev = (elm)->field.cq_next = (head)->cq_last = (elm); \
   } \
   (head)->cq_first = (elm);}  while(0)

/***************************************************************************
Summary:
    Inserts element into the list
	
Description:
Description:
    Inserts the new element at the end of the list.
	
Input:
    head - pointer to the list head
    elm - pointer to the new element
    field - name of the elements link field

Returns:
    <none>

Example:
    BLST_CQ_INSERT_TAIL(&head, elm, link);
****************************************************************************/
#define BLST_CQ_INSERT_TAIL(head, elm, field) do {				\
   if ( (head)->cq_last != NULL) { \
    (elm)->field.cq_next = (head)->cq_first; \
    (elm)->field.cq_prev = (head)->cq_last; \
    (head)->cq_first->field.cq_prev = (head)->cq_last->field.cq_next = (elm); \
   } else { \
     (elm)->field.cq_prev = (elm)->field.cq_next = (head)->cq_first = (elm);\
   }\
   (head)->cq_last = (elm); } while(0)

 
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
    BLST_CQ_INSERT_AFTER(&head, first, second, link);
****************************************************************************/
#define BLST_CQ_INSERT_AFTER(head, elm, new_elm, field) do { \
        (new_elm)->field.cq_prev = (elm); \
        (new_elm)->field.cq_next = (elm)->field.cq_next; \
        (elm)->field.cq_next->field.cq_prev = (new_elm); \
        if ((elm) == (head)->cq_last)  (head)->cq_last = (new_elm);\
        (elm)->field.cq_next = (new_elm); } while(0)

/***************************************************************************
Summary:
    Inserts element into the list
	
Description:
    Inserts new element before existing element.
	
Input:
    head - pointer to the list head
    elm - pointer to the element from the list
    new_elm - pointer to the new element
    field - name of the elements link field

Returns:
    <none>

Example:
    BLST_CQ_INSERT_BEFORE(&head, second, first, link);
****************************************************************************/
#define BLST_CQ_INSERT_BEFORE(head, elm, new_elm, field) do { \
        (new_elm)->field.cq_next = (elm); \
        (new_elm)->field.cq_prev = (elm)->field.cq_prev;\
        (elm)->field.cq_prev->field.cq_next = (new_elm); \
        if ((elm) == (head)->cq_first) (head)->cq_first = (new_elm);\
        (elm)->field.cq_prev = (new_elm); } while(0)



/***************************************************************************
Summary:
    Removes element from the list
	
Description:
    Removes element from the list.
	
Input:
    head - pointer to the list head
    elm - pointer to the list element
    field - name of the elements link field

See also:
    BLST_CQ_REMOVE_HEAD

Returns:
    <none>

Example:
    BLST_CQ_REMOVE(&head, first, link);
****************************************************************************/
#define BLST_CQ_REMOVE(head, elm, field) do { \
    (elm)->field.cq_next->field.cq_prev = (elm)->field.cq_prev; \
    if ( (elm) == (head)->cq_last ) { \
        if ( (elm)==(head)->cq_first ) (head)->cq_first = (head)->cq_last = NULL;\
        else (head)->cq_last = (elm)->field.cq_prev; \
    }\
    (elm)->field.cq_prev->field.cq_next = (elm)->field.cq_next; \
    if ( (elm) == (head)->cq_first) (head)->cq_first = (elm)->field.cq_next; \
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
    BLST_CQ_REMOVE

Returns:
    <none>

Example:
    BLST_CQ_REMOVE_HEAD(&head, first, link);
****************************************************************************/
#define BLST_CQ_REMOVE_HEAD(head, field) do { \
    if ((head)->cq_first == (head)->cq_last) (head)->cq_first = (head)->cq_last = NULL;\
    else {\
      (head)->cq_first = (head)->cq_first->field.cq_next; \
      (head)->cq_last->field.cq_next = (head)->cq_first; \
    } } while(0)


#endif /* BLST_CIRCLEQ_H     */
