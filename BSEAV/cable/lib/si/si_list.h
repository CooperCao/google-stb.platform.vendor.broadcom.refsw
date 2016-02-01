/***************************************************************************
 *     Copyright (c) 2002-2009, Broadcom Corporation
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

#ifndef SI_LIST_H
#define SI_LIST_H


/*================== Module Overview =====================================
This modules defines macros to control doubly linked list. 
All operations are typesafe (doesn't required typecasting) and constant time.

This list allow:
  o Insert a new entry at the head of the list
  o Insert a new entry after/before any element in the list
  o O(1) removal of any entry in the list
  o Forward/Backward traversal through the list
  o Each element requires two pointers
  o Code size and execution time is about twice that for singly-linked list
  
========================================================================*/

 
/***************************************************************************
Summary:
    Creates new data type for the list head
   
Description:
    Creates new data type for the list head, this type used to create 
variable for the lists head.
    User should create new the list head data type for every different 
element datatype.
   
Input:
   name - name for the new data type (structure)
    type - existing user data type used for element of the list 

Example:    
     SI_LST_D_HEAD(block_head, block);
     struct block_head  head;

Returns:
    <none>
****************************************************************************/
#define SI_LST_D_HEAD(name, type) struct name { struct type *l_first; }

/***************************************************************************
Summary:
    Defines links entry
   
Description:
    Defines entrys for the list inside the user structure.for the element.
   
Input:
    type - the datatype for element

Example:
     struct block {
        SI_LST_D_ENTRY(block) link;
        char string[256];
     };  

Returns:
    <none>
****************************************************************************/
#define SI_LST_D_ENTRY(type)  struct { struct type *l_next, *l_prev; }

/***************************************************************************
Summary:
    Initializes lists head
   
Description:
    Initializes the head of the list. The head shall be initialized before 
list can be used.
    This macro used for dynamic initialization.
   
Input:
    head - pointer to the list head

See also:
    SI_LST_D_INITIALIZER

Example:
    SI_LST_D_INIT(&head);

Returns:
    <none>
****************************************************************************/
#define SI_LST_D_INIT(head) ((head)->l_first=NULL)


/***************************************************************************
Summary:
    Initializes link entry
   
Description:
    Initializes the link entry. The entry shall be initialized before 
link can be used.
    This macro used for dynamic initialization.
   
Input:
    entry - pointer to the link entry

See also:
    

Example:
    SI_LST_D_INIT_ENTRY(&entry);

Returns:
    <none>
****************************************************************************/
#define SI_LST_D_INIT_ENTRY(entry) ((entry)->l_next=(entry)->l_prev=NULL)


/***************************************************************************
Summary:
    Initializes lists head
   
Description:
    Initializes the head of the list. The head shall be initialized before 
list can be used.
    This macro used for static initialization.
   
Input:
    head - pointer to the list head

See also:
    SI_LST_D_INIT

Example:
    static struct block_head  head = SI_LST_D_INITIALIZER(head);

Returns:
    <none>
****************************************************************************/
#define SI_LST_D_INITIALIZER(head) {NULL}

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
    if (SI_LST_D_EMPTY(&head) { return ; }

****************************************************************************/
#define SI_LST_D_EMPTY(head) ((head)->l_first == NULL)

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
    struct block *first=SI_LST_D_FIRST(&head);
****************************************************************************/
#define SI_LST_D_FIRST(head) ((head)->l_first)

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
    struct block *second=SI_LST_D_NEXT(first);
****************************************************************************/
#define SI_LST_D_NEXT(elm, field) ((elm)->field.l_next)

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
    struct block *first=SI_LST_D_PREV(second);
****************************************************************************/
#define SI_LST_D_PREV(elm, field) ((elm)->field.l_prev)

/***************************************************************************
Summary:
    Inserts element into the list
   
Description:
    Inserts new element into the head of the list.
   
Input:
    head - pointer to the list head
    elm - pointer to the new element
    field - name of the elements link field

Returns:
    <none>

Example:
    SI_LST_D_INSERT_HEAD(&head, new_block, link);
****************************************************************************/
#define SI_LST_D_INSERT_HEAD(head, elm, field) do { \
			if ( ((elm)->field.l_next = (head)->l_first) != NULL ) \
				(head)->l_first->field.l_prev = (elm); \
			(head)->l_first = (elm); \
			(elm)->field.l_prev = NULL; \
		}  while(0)

/***************************************************************************
Summary:
    Inserts element into the list
   
Description:
    Inserts new element after existing element.
   
Input:
    elm - pointer to the element from the list
    new_elm - pointer to the new element
    field - name of the elements link field

Returns:
    <none>

Example:
    SI_LST_D_INSERT_AFTER(first, second, link);
****************************************************************************/
#define SI_LST_D_INSERT_AFTER(elm, new_elm, field) do { \
			(new_elm)->field.l_prev = (elm); \
			if (((new_elm)->field.l_next = elm->field.l_next)!=NULL) \
				elm->field.l_next->field.l_prev = new_elm; \
			(elm)->field.l_next = (new_elm); \
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
    SI_LST_D_INSERT_BEFORE(&head, second, first, link);
****************************************************************************/
#define SI_LST_D_INSERT_BEFORE(head, elm, new_elm, field) do {  \
			(new_elm)->field.l_next = (elm);  \
			if (((new_elm)->field.l_prev = (elm)->field.l_prev)!=NULL) \
				elm->field.l_prev->field.l_next = new_elm; \
			else \
				(head)->l_first = (new_elm); \
			(elm)->field.l_prev = (new_elm); \
		} while(0)

    

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
    SI_LST_D_REMOVE_HEAD

Returns:
    <none>

Example:
    SI_LST_D_REMOVE(&head, first, link);
****************************************************************************/
#define SI_LST_D_REMOVE(head, elm, field) do { \
			if ((elm)->field.l_next) \
				(elm)->field.l_next->field.l_prev = (elm)->field.l_prev; \
			if ((elm)->field.l_prev) \
				(elm)->field.l_prev->field.l_next = (elm)->field.l_next; \
			else \
				(head)->l_first = (elm)->field.l_next; \
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
    SI_LST_D_REMOVE

Returns:
    <none>

Example:
    SI_LST_D_REMOVE_HEAD(&head, first, link);
****************************************************************************/
#define SI_LST_D_REMOVE_HEAD(head, field) do { \
			(head)->l_first = (head)->l_first->field.l_next;  \
			if ((head)->l_first) \
			{ \
				(head)->l_first->field.l_prev = NULL;\
			} \
		} while(0)




#endif

