/***************************************************************************
 *     Copyright (c) 2007 Broadcom Corporation
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
 * Hash table associative container
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef __BHASH_H__
#define __BHASH_H__

#include "blst_slist.h"

/*================== Module Overview =====================================
This modules defines macros to control hash table associative container.
This container allows to associate any user defined data type with the user
defined key. Single data type could be simultaneously member of many associative
tables.

All operations are typesafe (doesn't require typecasting) and run in a constant space.

This module implements hash table with chaining type of collision resolution.
And conainer supports the following operations:

  o Insert a new entry - time complexity O(1)
  o Removal of entry by key - time complexity O(1) .. O(n) (*)
  o Lookup by key - time complexity O(1) .. O(n) (*)
  o Each element requires single pointer in order to be addded in to the container
  o Code size and execution time is about twice that for singly-linked list

Note *:
Performance of hash table is strongly corellated with quality of hash function.
With good quality hash function removal and lookup are O(1) (constant time), with poor
hash function there are meny collisions ans peformance degraded to O(n) (e.g. proportional
to number of elements stored in the container).
  
========================================================================*/

/***************************************************************************
Summary:
    Creates new data type for the heash head
	
Description:
    Creates new data type for the hash head, this type used to create variable for the hash head.
    User should create new the hash head data type for every different element datatype.
	
Input:
	name - name for the new data type (structure)
    type - existing user data type(structure) used for element of the container

Example:    
     BHASH_HEAD(block_hash, block);
     struct block_hash container;

Returns:
    <none>
****************************************************************************/
#define BHASH_HEAD(name, type, size) struct name { \
	BLST_S_HEAD(name##_index, type) hash_bucket[size]; }  

/***************************************************************************
Summary:
    Defines hash entry
	
Description:
    Defines entrys for the hash inside the user data type.
	
Input:
    type - the datatype for element

Example:
     struct block {
        BHASH_ENTRY(block) hash_link;
		int key;
        char string[256];
     };  

Returns:
    <none>
****************************************************************************/
#define BHASH_ENTRY(type) BLST_S_ENTRY(type) 

/***************************************************************************
Summary:
    Defines internal functions to operate with a container
	
Description:
     This macro shall be used in the C file and it defines static function
	 that are needed for internal working of the hash container.
	
Input:
    name - user selected name for the hash
	type_key - datatype of key
    type - the datatype(structure) of the element
	field - name of the hash link field in the element
	hash - hash function that takes key and returns it's hash (unsigned int)
	compare - compare function that takes key and pointer to the element and returns true 
	          if and only if key is exactly matches user element

Example:
	static unsigned block_key_hash(int key) {return key;}
	static bool block_key_compare(const struct block *block, int key) {return block->key == key;}
	BHASH_GENERATE(block_container, int, block, hash_link, block_key_hash, block_key_compare)

Returns:
    <none>
****************************************************************************/
#define BHASH_GENERATE(name, type_key, type, field, hash, compare) \
static void name##_hash_init(struct name *h) { \
	unsigned i; \
	for(i=0;i<sizeof(h->hash_bucket)/sizeof(h->hash_bucket[0]);i++) { BLST_S_INIT(&h->hash_bucket[i]); } \
	return; } \
static void \
name##_hash_add(struct name *h, type_key key, struct type *t) { \
	unsigned key_hash = hash(key); \
	BDBG_ASSERT(h); \
	key_hash = key_hash % (sizeof(h->hash_bucket)/sizeof(h->hash_bucket[0])); \
	BLST_S_INSERT_HEAD(&h->hash_bucket[key_hash], t, field); return; } \
static struct type * \
name##_hash_find(struct name *h, type_key key) { \
	struct type *t; unsigned key_hash = hash(key); \
	BDBG_ASSERT(h); \
	key_hash = key_hash % (sizeof(h->hash_bucket)/sizeof(h->hash_bucket[0])); \
	for(t = BLST_S_FIRST(&h->hash_bucket[key_hash]); t ; t = BLST_S_NEXT(t, field)) { \
		if(compare(key, t)) { return t;} } \
	return NULL; } \
static bool \
name##_hash_remove(struct name *h, type_key key) { \
 	struct type *t, *prev_t; \
	unsigned key_hash = hash(key); \
	BDBG_ASSERT(h); \
	key_hash = key_hash % (sizeof(h->hash_bucket)/sizeof(h->hash_bucket[0])); \
	for(prev_t = NULL, t = BLST_S_FIRST(&h->hash_bucket[key_hash]); t; t = BLST_S_NEXT(t, field)) { \
		if(compare(key, t)) { \
			if(prev_t==NULL) { BLST_S_REMOVE_HEAD(&h->hash_bucket[key_hash], field); } \
			else { BLST_S_REMOVE_NEXT(&h->hash_bucket[key_hash], prev_t, field); }\
			return true; \
		} prev_t = t; \
	}return false; } 


/***************************************************************************
Summary:
    Initializes conteiner
	
Description:
    Initializes container, e.g. wipes out all it's content.
	
Input:
    name - name of the hash container
	hash - pointer to the container

See also:
    BHASH_GENERATE

Example:
     struct block_hash container;
	 BHASH_INIT(block_container, &container);

Returns:
    <none>
****************************************************************************/
#define BHASH_INIT(name, hash) name##_hash_init(hash)

/***************************************************************************
Summary:
    Adds element into the container
	
Description:
    This macro is used to associate a key with the data element. Note only 
	one key could be associated with the given data element (e.g. data element
	could be added only once in the container).
	
Input:
    name - name of the hash container
	hash - pointer to the container
	key - key for association
	elem - pointer to the data element

See also:
    BHASH_REMOVE
	BHASH_FIND

Returns:
    <none>

Example:
	struct block b;
    BHASH_ADD(block_container, &container, 0, &b );
****************************************************************************/
#define BHASH_ADD(name, hash, key, elm) name##_hash_add((hash), (key), (elm))

/***************************************************************************
Summary:
    Removes element into the container
	
Description:
    This macro is used to remove association between key and data element;
	
Input:
    name - name of the hash container
	hash - pointer to the container
	key - key for removal from the container

See also:
    BHASH_ADD

Returns:
    true - if element was in the container
	false - if element wasn't in the container

Example:
    if(!BHASH_REMOVE(block_container, &container, 0)) {
		printf("key 0 wasn't in the table!!!");
	}
****************************************************************************/
#define BHASH_REMOVE(name, hash, key) name##_hash_remove((hash), (key))

/***************************************************************************
Summary:
    Removes element into the container
	
Description:
    This macro is used to remove association between key and data element;
	
Input:
    name - name of the hash container
	hash - pointer to the container
	key - key to search in the container

See also:
    BHASH_ADD

Returns:
    pointer to data element - if element was in the container
	NULL - if element wasn't in the container

Example:
	struct block *b = BHASH_FIND(block_container, &container, 0)) {
    if(b) {
		printf("key 0 associated with %s", b->string);
	}
****************************************************************************/
#define BHASH_FIND(name, hash, key) name##_hash_find((hash), (key))

/***************************************************************************
Summary:
    Open itterator for the container 
	
Description:
    The BHASH_FOREACH_BEGIN and BHASH_FOREACH_END are used to itterate over 
	all elements in the container.
	Please note that 'break' can't be used to escape from the itterator,
	however goto could safely used if it points outside of itterator.

	User shall not add or remove elements from the container, the only
	exception is that user could remove currently itterated element, e.g.
	itterator is widely useful for controlled removal of all elements
	from the container
	
Input:
    name - name of the hash container
	hash - pointer to the container
    type - the datatype(structure) of the element
	elm - element to be itterated
	field - name of the hash link field in the element

See also:
    BHASH_REMOVE
	BHASH_FOREACH_END

Returns:
    <none>

Example:
	struct block *b;
	BHASH_FOREACH_BEGIN(block_container, &container, block, b, hash_link)
		printf("key %d associated with %s", b->key, b->string);
	BHASH_FOREACH_END()
****************************************************************************/
#define BHASH_FOREACH_BEGIN(name,hash,type,elm,field) { unsigned _hash_i; for(_hash_i=0;_hash_i<sizeof((hash)->hash_bucket)/sizeof((hash)->hash_bucket[0]);_hash_i++) { \
										struct type *_hash_next;for(_hash_next=elm=BLST_S_FIRST(&(hash)->hash_bucket[_hash_i]);elm;elm=_hash_next) { _hash_next = BLST_S_NEXT(elm, field);

/***************************************************************************
Summary:
    Open itterator for the container 

See also:
	BHASH_FOREACH_BEGIN

Returns:
    <none>
****************************************************************************/
#define BHASH_FOREACH_END()	}}}

#endif  /* __BHASH_H__ */





