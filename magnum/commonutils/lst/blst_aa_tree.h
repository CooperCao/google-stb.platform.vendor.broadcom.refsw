/***************************************************************************
 * Copyright (C) 2007-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 * Interface for AA tree, type of binary search tree
 *
 ***************************************************************************/
#ifndef BLST_AA_TREE_H__
#define BLST_AA_TREE_H__


#ifdef __cplusplus
extern "C"
{
#endif

/*================== Module Overview =====================================
This modules defines macros to implement AA binary search tree.
All operations are type safe (don't require typecasting) and run either constant or in O(log n) time.

Thee AA tree allows:
  o O(log n) insert a new entry into the tree
  o O(log n) exact lookup of entry based on a key
  o O(log n) approximate lookup of entry based on a key (returns entry with a key that is not larger then a requested key)
  o O(log n) removal of any entry from the tree
  o O(log n) Forward/Backward traversal through the tree nodes (traversal in ascending/descending order of keys)
  o O(1) access to first(smallest) or last(largest) element
  o Each element requires three pointers and one integer for internal bookkeeping
 
Usage guide:
  o To allow type safety and embedded semantics the AA tree implemented using mix of generic functions, auto-generated functions and macros
  o Use of the binary search tree requires total ordering between key and node in a tree. 
  o User must provide compare function that compares node with a key, this function should return zero if elements are equal, positive value if key large then mode, and negative value otherwise.
  o Binary search tree doesn't allow duplicate elements in the tree, and attempt to insert element with a key that already in the tree, would return element that already in the tree and would not modify tree.
  o All operations should be accessed using BLST_AAA_TREE_##OP## macros
  o Not trivial operations (find, next, prev, etc) require used of BLST_AAA_TREE_GENERATE_##OP## macro prior to use, this macros would generate function that is to perform requested operation
  o Since auto-generated functions are static, each C file that uses BLST_AAA_TREE_##OP## macros should include BLST_AAA_TREE_GENERATE_##OP## macro
  
  
Example: Make a map(tree) that allows search of integer data by the character key

#include "blst_aa_tree.h"

// node (element) of the tree
struct char_map_node {
    char key;
    BLST_AA_TREE_ENTRY(char_map_node) node;
    int data;
};

// declares type for the tree data structure
BLST_AA_TREE_HEAD(char_map, char_map_node);

// declare comparison function  between key and node
static int char_map_cmp(const struct char_map_node *node, char key)
{
    if(key > node->key) {
        return 1;
    } else if(key==node->key) {
        return 0;
    } else {
        return -1;
    }
}

// generate FIND, INSERT and REMOVE operations 
BLST_AA_TREE_GENERATE_FIND(char_map, char, char_map_node, node, char_map_cmp)
BLST_AA_TREE_GENERATE_INSERT(char_map, char, char_map_node, node, char_map_cmp)
BLST_AA_TREE_GENERATE_REMOVE(char_map, char_map_node, node)

void main(void)
{
    struct char_map_node nodes[10];
    struct char_map_node *node;
    struct char_map map;
    char keys[10] = {'0','1','2','3','4','5','6','7','8','9'};
    unsigned i;

    // initialize the map
    BLST_AA_TREE_INIT(char_map, &map);

    // insert elements
    for(i=0;i<10;i++) {
        char key = keys[i];
        nodes[i].key = key; // set key 
        nodes[i].data = i; // set data 
        BLST_AA_TREE_INSERT(char_map, &map, key, &nodes[i]);
    }

    // lookup elements 
    node = BLST_AA_TREE_FIND(char_map, &map, '0');
    BDBG_ASSERT(node->data == 0);
    node = BLST_AA_TREE_FIND(char_map, &map, '5');
    BDBG_ASSERT(node->data == 5);
    node = BLST_AA_TREE_FIND(char_map, &map, 'A');
    BDBG_ASSERT(node == NULL);

    // remove elements 
    for(i=0;i<10;i++) {
        BLST_AA_TREE_REMOVE(char_map, &map, &nodes[i]);
    }

    // lookup elements
    node = BLST_AA_TREE_FIND(char_map, &map, '0');
    BDBG_ASSERT(node == NULL);

    return;
}

========================================================================*/

struct BLST_AA_Tree_Node;

/***************************************************************************
Summary:
    Defines tree entry

Description:
    Defines entry for the tree inside the users structure.

Input:
    type - the existing user data type for the element of the list

Example:
    struct char_map_node {
        char key;
        BLST_AA_TREE_ENTRY(char_map_node) node;
        int data;
    };

Returns:
    <none>
****************************************************************************/
#define BLST_AA_TREE_ENTRY(type) struct { \
        struct BLST_AA_Tree_Node *aan_parent, *aan_left, *aan_right; \
        unsigned aan_level; }


#define BLST_AA_TREE_P_CAST(node, type, off) ((struct type *)((uint8_t *)(node)-off))
#define BLST_AA_TREE_P_OFF(type, field) ((uint8_t *)&(((struct type*)0)->field.aan_parent) - (uint8_t *)0)
#define BLST_AA_TREE_P_NODE(node, off) ((struct BLST_AA_Tree_Node *)((uint8_t *)(node)+off))


/***************************************************************************
Summary:
    Creates new data type for the tree root

Description:
    Creates new data type for the tree root, this type used to create variable for the tree.
    User should create new the tree root data type for every different node data type.

Input:
    name - name for the new tree root data type (this can be any unique structure name)
    type - existing data type used for the element of the tree 

Example:
    struct char_map_node {
        char key;
        BLST_AA_TREE_ENTRY(char_map_node) node;
        int data;
    };

    BLST_AA_TREE_HEAD(char_map, char_map_node);

Returns:
    <none>
****************************************************************************/
#define BLST_AA_TREE_HEAD(name, type) struct name { struct BLST_AA_Tree_Head aa_tree; }


/***************************************************************************
Summary:
    Initializes root of the tree

Description:
    Initializes the root of the tree. The root shall be initialized before tree can be used.

Input:
    name - name of the tree
    root - pointer to the list head

See also:
    BLST_D_INITIALIZER

Example:
    BLST_D_INIT(&head);

Returns:
    <none>
****************************************************************************/
#define BLST_AA_TREE_INIT(name, root) BLST_AAT_P_Init_isrsafe(&(root)->aa_tree)

/***************************************************************************
Summary:
    Generate code that is used to insert element into the tree

Description:
    This macro is used to generate private function that is specific to pustular type of the tree.
    It should be used outside of scope of any function and before any references to the BLST_AA_TREE_INSERT.

Input:
    name - name of the tree
    type_key - type of the key
    type  - type of the tree node
    field - name of the elements tree entry field
    compare - name of the compare function

See also:
    BLST_AA_TREE_GENERATE_REMOVE
    BLST_AA_TREE_INSERT

Example:
    BLST_AA_TREE_GENERATE_FIND(char_map, char, char_map_node, node, char_map_cmp)

Returns:
    <none>
****************************************************************************/
#define BLST_AA_TREE_GENERATE_INSERT(name, type_key, type, field, compare) \
static struct type * name##_BLST_AAT_Insert_isrsafe(struct name *tree, type_key key, struct type *new_node) { \
    const unsigned off = BLST_AA_TREE_P_OFF(type, field); \
    struct BLST_AA_Tree_Node *node = tree->aa_tree.aat_root.aa_node.aan_left; \
    struct BLST_AA_Tree_Node *parent = &tree->aa_tree.aat_root; int cmp = -1; \
    BDBG_ASSERT(compare(new_node, key)==0); \
    while(node!=&tree->aa_tree.aat_root) { \
        cmp = compare(BLST_AA_TREE_P_CAST(node, type, off), key); parent = node; \
        if(cmp<0) { node = node->aa_node.aan_left; } \
        else if (cmp>0) { node = node->aa_node.aan_right; } \
        else { return BLST_AA_TREE_P_CAST(node, type, off); } \
    } BLST_AAT_P_Insert_Node_isrsafe(&tree->aa_tree, BLST_AA_TREE_P_NODE(new_node, off), parent, cmp); return new_node; }

/***************************************************************************
Summary:
    Inserts new element into the tree

Description:
    This macro is used to insert element into the tree. Element should be initialized prior to insertion.
    This implementation doesn't allow tree to have elements with duplicate keys.

Input:
    name - name of the tree
    root - pointer to  root of the tree
    key - key for the insertion
    elm - pointer to node to be inserted

See also:
    BLST_AA_TREE_GENERATE_INSERT
    BLST_AA_TREE_REMOVE

Example:
    BLST_AA_TREE_INSERT(char_map, &map, key, &nodes[i]);

Returns:
    o - pointer to the inserted element if insertion was successful
    o - pointer to the element already in the tree, if tree already has element with matching key
****************************************************************************/
#define BLST_AA_TREE_INSERT(name, root, key, elm) name##_BLST_AAT_Insert_isrsafe((root), (key), (elm))

/***************************************************************************
Summary:
    Generate code that is used to remove element from the tree

Description:
    This macro is used to generate private function that is specific to particular type of the tree.
    It should be used outside of scope of any function and before any references to the BLST_AA_TREE_REMOVE.

Input:
    name - name of the tree
    type  - type of the tree node
    field - name of the elements tree entry field

See also:
    BLST_AA_TREE_GENERATE_INSERT
    BLST_AA_TREE_REMOVE

Example:
    BLST_AA_TREE_GENERATE_REMOVE(char_map, char_map_node, node)

Returns:
    <none>
****************************************************************************/
#define BLST_AA_TREE_GENERATE_REMOVE(name, type, field) \
    static void name##_BLST_AAT_P_Remove_isrsafe(struct name *tree, struct type *node) { BLST_AAT_P_Remove_isrsafe(&tree->aa_tree, BLST_AA_TREE_P_NODE(node, BLST_AA_TREE_P_OFF(type, field)));}

/***************************************************************************
Summary:
    Removes element from the tree

Description:
    This macro is used to remove element from tree. 
    Element must be in tree prior to its removal, removal element that is not belong to the tree has undefined behaviour.

Input:
    name - name of the tree
    root - pointer to  root of the tree
    key - key for the insertion
    elm - pointer to node to be inserted

See also:
    BLST_AA_TREE_GENERATE_REMOVE
    BLST_AA_TREE_INSERT

Example:
    BLST_AA_TREE_REMOVE(char_map, &map, &nodes[i]);

Returns:
    <none>
****************************************************************************/
#define BLST_AA_TREE_REMOVE(name, root, elm) name##_BLST_AAT_P_Remove_isrsafe((root), (elm))


#define BLST_AA_TREE_GENERATE_FIND(name, type_key, type, field, compare) \
static struct type *name##_BLST_AAT_Find_isrsafe(const struct name *tree, type_key key) { \
    const unsigned off = BLST_AA_TREE_P_OFF(type, field);\
    const struct BLST_AA_Tree_Node *node;\
    for(node=tree->aa_tree.aat_root.aa_node.aan_left;node!=&tree->aa_tree.aat_root;) { \
        int cmp = compare(BLST_AA_TREE_P_CAST(node, type, off), key);\
        if (cmp!=0) {\
            const struct BLST_AA_Tree_Node *left = node->aa_node.aan_left;\
            const struct BLST_AA_Tree_Node *right = node->aa_node.aan_right;\
            node = (cmp < 0) ? left : right; \
        } else { return BLST_AA_TREE_P_CAST(node, type, off); } \
    } return NULL; } 

#define BLST_AA_TREE_GENERATE_FIND_SOME(name, type_key, type, field, compare) \
static struct type *name##_BLST_AAT_Find_some_isrsafe(const struct name *tree, type_key key) { \
    const unsigned off = BLST_AA_TREE_P_OFF(type, field);\
    const struct BLST_AA_Tree_Node *node;\
    node = tree->aa_tree.aat_root.aa_node.aan_left;\
    if(node==&tree->aa_tree.aat_root) { return NULL; } \
    for(;;) { \
        int cmp = compare(BLST_AA_TREE_P_CAST(node, type, off), key);\
        const struct BLST_AA_Tree_Node *prev = node; \
        if (cmp!=0) {\
            const struct BLST_AA_Tree_Node *left = node->aa_node.aan_left;\
            const struct BLST_AA_Tree_Node *right = node->aa_node.aan_right;\
            node = (cmp < 0) ? left : right; \
            if(node==&tree->aa_tree.aat_root) {return BLST_AA_TREE_P_CAST(prev, type, off); } \
        } else { return BLST_AA_TREE_P_CAST(node, type, off); } \
    } }

#define BLST_AA_TREE_GENERATE_FIRST(name, type, field) \
    static struct type * name##_BLST_AAT_P_First_isrsafe(struct name *tree) { return (struct type *)BLST_AAT_P_First_isrsafe(&tree->aa_tree, BLST_AA_TREE_P_OFF(type, field));}

#define BLST_AA_TREE_GENERATE_LAST(name, type, field) \
    static struct type * name##_BLST_AAT_P_Last_isrsafe(struct name *tree) { return (struct type *)BLST_AAT_P_Last_isrsafe(&tree->aa_tree, BLST_AA_TREE_P_OFF(type, field));}

#define BLST_AA_TREE_GENERATE_PREV(name, type, field) \
    static struct type * name##_BLST_AAT_P_Prev_isrsafe(struct name *tree, struct type *node) { return (struct type *)BLST_AAT_P_Prev_isrsafe(&tree->aa_tree, (struct BLST_AA_Tree_Node *)node, BLST_AA_TREE_P_OFF(type, field));}

#define BLST_AA_TREE_GENERATE_NEXT(name, type, field) \
    static struct type * name##_BLST_AAT_P_Next_isrsafe(struct name *tree, struct type *node) { return (struct type *)BLST_AAT_P_Next_isrsafe(&tree->aa_tree, (struct BLST_AA_Tree_Node *)node, BLST_AA_TREE_P_OFF(type, field));}


#define BLST_AA_TREE_FIND(name, head, key) name##_BLST_AAT_Find_isrsafe(head, key)
#define BLST_AA_TREE_FIND_SOME(name, head, key) name##_BLST_AAT_Find_some_isrsafe(head, key)
#define BLST_AA_TREE_FIRST(name, head) name##_BLST_AAT_P_First_isrsafe(head)
#define BLST_AA_TREE_LAST(name, head) name##_BLST_AAT_P_Last_isrsafe(head)
#define BLST_AA_TREE_NEXT(name, head, elm) name##_BLST_AAT_P_Next_isrsafe(head, elm)
#define BLST_AA_TREE_PREV(name, head, elm) name##_BLST_AAT_P_Prev_isrsafe(head, elm)

struct BLST_AA_Tree_Node {
    BLST_AA_TREE_ENTRY(aa_node) aa_node;
};

struct BLST_AA_Tree_Head {
    struct BLST_AA_Tree_Node aat_root;
};

void BLST_AAT_P_Init_isrsafe(struct BLST_AA_Tree_Head *tree);
struct BLST_AA_Tree_Node *BLST_AAT_P_First_isrsafe(const struct BLST_AA_Tree_Head *tree, unsigned off);
struct BLST_AA_Tree_Node *BLST_AAT_P_Last_isrsafe(const struct BLST_AA_Tree_Head *tree, unsigned off);
struct BLST_AA_Tree_Node *BLST_AAT_P_Next_isrsafe(struct BLST_AA_Tree_Head *tree, struct BLST_AA_Tree_Node *node, unsigned off);
struct BLST_AA_Tree_Node *BLST_AAT_P_Prev_isrsafe(struct BLST_AA_Tree_Head *tree, struct BLST_AA_Tree_Node *node, unsigned off);
void BLST_AAT_P_Insert_Node_isrsafe(struct BLST_AA_Tree_Head *tree, struct BLST_AA_Tree_Node *new_node, struct BLST_AA_Tree_Node *parent, int cmp);
void BLST_AAT_P_Remove_isrsafe(struct BLST_AA_Tree_Head *tree, struct BLST_AA_Tree_Node *node);




#ifdef __cplusplus
}
#endif



#endif /* BLST_AA_TREE_H__ */




