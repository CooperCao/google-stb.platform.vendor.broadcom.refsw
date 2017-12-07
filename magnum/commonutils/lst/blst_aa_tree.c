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
 * Module Description:
 *
 * Interface for AA tree, type of binary search tree
 ***************************************************************************/

#include "bstd.h"
#include "blst_aa_tree.h"
BDBG_MODULE(blst_aa_tree);


#define B_AA_NODE_LEVEL(node) ((node)->aa_node.aan_level)
#define B_AA_LAST_NODE(tree) (&(tree)->aat_root)
#define B_AA_ROOT_NODE(tree) (&(tree)->aat_root)


void 
BLST_AAT_P_Init_isrsafe(struct BLST_AA_Tree_Head *tree)
{
	tree->aat_root.aa_node.aan_level = 0;
	tree->aat_root.aa_node.aan_left = B_AA_LAST_NODE(tree);
	tree->aat_root.aa_node.aan_right = B_AA_LAST_NODE(tree);
	tree->aat_root.aa_node.aan_parent = B_AA_ROOT_NODE(tree);
	return;
}


struct BLST_AA_Tree_Node *
BLST_AAT_P_First_isrsafe(const struct BLST_AA_Tree_Head *tree, unsigned off)
{
	struct BLST_AA_Tree_Node *root = tree->aat_root.aa_node.aan_left;
	struct BLST_AA_Tree_Node *parent = NULL;
	while(root!=B_AA_LAST_NODE(tree)) {
		parent = root;
		root = root->aa_node.aan_left;
	}
    if(parent) {
	    return BLST_AA_TREE_P_CAST(parent, BLST_AA_Tree_Node, off);
    } else {
	    return NULL;
    }
}

struct BLST_AA_Tree_Node *
BLST_AAT_P_Last_isrsafe(const struct BLST_AA_Tree_Head *tree, unsigned off)
{
	struct BLST_AA_Tree_Node *root = tree->aat_root.aa_node.aan_left;
	struct BLST_AA_Tree_Node *parent = NULL;
	while(root!=B_AA_LAST_NODE(tree)) {
		parent = root;
		root = root->aa_node.aan_right;
	}
    if(parent) {
	    return BLST_AA_TREE_P_CAST(parent, BLST_AA_Tree_Node, off);
    } else {
        return NULL;
    }
}

struct BLST_AA_Tree_Node *
BLST_AAT_P_Next_isrsafe(struct BLST_AA_Tree_Head *tree, struct BLST_AA_Tree_Node *node, unsigned off)
{
	struct BLST_AA_Tree_Node *next;
	node = BLST_AA_TREE_P_NODE(node, off);

	next = node->aa_node.aan_right;
	if(B_AA_LAST_NODE(tree)!=next) {
		node = next;
		while(B_AA_LAST_NODE(tree)!=(next=node->aa_node.aan_left)) {
			node = next;
		}
	} else {
		while( (B_AA_ROOT_NODE(tree) != (next=node->aa_node.aan_parent)) && (node==next->aa_node.aan_right)) {
			node = next;
		}
		node = next;
		if(node==B_AA_ROOT_NODE(tree)) {
			return NULL;
		}
	}
	return BLST_AA_TREE_P_CAST(node, BLST_AA_Tree_Node, off);
}

struct BLST_AA_Tree_Node *
BLST_AAT_P_Prev_isrsafe(struct BLST_AA_Tree_Head *tree, struct BLST_AA_Tree_Node *node, unsigned off)
{
	struct BLST_AA_Tree_Node *prev;

	node = BLST_AA_TREE_P_NODE(node, off);

	prev = node->aa_node.aan_left;
	if(B_AA_LAST_NODE(tree)!=prev) {
		node = prev;
		while(B_AA_LAST_NODE(tree)!=(prev=node->aa_node.aan_right)) {
			node = prev;
		}
	} else {
		while( (B_AA_ROOT_NODE(tree) != (prev=node->aa_node.aan_parent)) && (node==prev->aa_node.aan_left)) {
			node = prev;
		}
		node = prev;
		if(node==B_AA_ROOT_NODE(tree)) {
			return NULL;
		}
	}
	return BLST_AA_TREE_P_CAST(node, BLST_AA_Tree_Node, off);
}

static void 
BLST_AAT_P_Skew_isrsafe(struct BLST_AA_Tree_Node *parent)
{
	struct BLST_AA_Tree_Node *new_parent = parent->aa_node.aan_left;

	BDBG_ASSERT(new_parent);

	if(parent->aa_node.aan_parent->aa_node.aan_left == parent) {
		parent->aa_node.aan_parent->aa_node.aan_left = new_parent;
	} else {
		parent->aa_node.aan_parent->aa_node.aan_right = new_parent;
	}
	new_parent->aa_node.aan_parent = parent->aa_node.aan_parent;
	parent->aa_node.aan_parent = new_parent;

	parent->aa_node.aan_left = new_parent->aa_node.aan_right;
	parent->aa_node.aan_left->aa_node.aan_parent = parent;
	new_parent->aa_node.aan_right = parent;

	parent->aa_node.aan_level = B_AA_NODE_LEVEL(parent->aa_node.aan_left)+1;

	return;
}

static bool 
BLST_AAT_P_Split_isrsafe(struct BLST_AA_Tree_Head *tree, struct BLST_AA_Tree_Node *parent)
{
	struct BLST_AA_Tree_Node *new_parent = parent->aa_node.aan_right;

	if(B_AA_LAST_NODE(tree)!=new_parent && parent->aa_node.aan_right->aa_node.aan_right->aa_node.aan_level == parent->aa_node.aan_level) {
		if(parent->aa_node.aan_parent->aa_node.aan_left == parent) {
			parent->aa_node.aan_parent->aa_node.aan_left = new_parent;
		} else {
			parent->aa_node.aan_parent->aa_node.aan_right = new_parent;
		}
		new_parent->aa_node.aan_parent = parent->aa_node.aan_parent;
		parent->aa_node.aan_parent = new_parent;

		parent->aa_node.aan_right = new_parent->aa_node.aan_left;
		parent->aa_node.aan_right->aa_node.aan_parent = parent;
		new_parent->aa_node.aan_left = parent;
		new_parent->aa_node.aan_level++;
		return true;
	}
	return false;
}


void 
BLST_AAT_P_Insert_Node_isrsafe(struct BLST_AA_Tree_Head *tree, struct BLST_AA_Tree_Node *new_node, struct BLST_AA_Tree_Node *parent, int cmp)
{
	struct BLST_AA_Tree_Node *node;

	if(cmp<0) {
		parent->aa_node.aan_left = new_node;
	} else {
		parent->aa_node.aan_right = new_node;
	} 
	new_node->aa_node.aan_level = 1;
	new_node->aa_node.aan_left = B_AA_LAST_NODE(tree);
	new_node->aa_node.aan_right = B_AA_LAST_NODE(tree);
	new_node->aa_node.aan_parent = parent;
	for(node=parent; node!= B_AA_ROOT_NODE(tree) ; node = node->aa_node.aan_parent) {
		if(node->aa_node.aan_level != B_AA_NODE_LEVEL(node->aa_node.aan_left) + 1) {
			BLST_AAT_P_Skew_isrsafe(node);
			if( (node->aa_node.aan_level != node->aa_node.aan_right->aa_node.aan_level)) {
				node = node->aa_node.aan_parent;
			}
		}
		if(!BLST_AAT_P_Split_isrsafe(tree, node->aa_node.aan_parent)) {
			break;
		}
	}
}

void 
BLST_AAT_P_Remove_isrsafe(struct BLST_AA_Tree_Head *tree, struct BLST_AA_Tree_Node *node)
{
	 struct BLST_AA_Tree_Node *leaf;
	 struct BLST_AA_Tree_Node *parent;

	 if(B_AA_LAST_NODE(tree)!=node->aa_node.aan_left) {
		 for(leaf=node->aa_node.aan_left;B_AA_LAST_NODE(tree)!=leaf->aa_node.aan_right;leaf=leaf->aa_node.aan_right) {
		 }
	 } else {
		 if(B_AA_LAST_NODE(tree)!=node->aa_node.aan_right) {
			 leaf = node->aa_node.aan_right;
		 } else {
			 leaf = node;
		 }
	 }
	 parent = leaf->aa_node.aan_parent == node ? leaf : leaf->aa_node.aan_parent;
	 if(leaf->aa_node.aan_parent->aa_node.aan_left == leaf) {
		 leaf->aa_node.aan_parent->aa_node.aan_left = B_AA_LAST_NODE(tree);
	 } else {
		 leaf->aa_node.aan_parent->aa_node.aan_right = B_AA_LAST_NODE(tree);
	 }
	 if(node!=leaf) { /* swap leaf and node */
		 if(node->aa_node.aan_parent->aa_node.aan_left == node) {
			 node->aa_node.aan_parent->aa_node.aan_left = leaf;
		 } else {
			 node->aa_node.aan_parent->aa_node.aan_right = leaf;
		 }
		 leaf->aa_node.aan_parent = node->aa_node.aan_parent;
		 node->aa_node.aan_left->aa_node.aan_parent = leaf;
		 leaf->aa_node.aan_left = node->aa_node.aan_left;

		 node->aa_node.aan_right->aa_node.aan_parent = leaf;
		 leaf->aa_node.aan_right = node->aa_node.aan_right;

		 leaf->aa_node.aan_level = node->aa_node.aan_level;
	 }
	 while(parent!=B_AA_ROOT_NODE(tree)) {
		 if(parent->aa_node.aan_level > B_AA_NODE_LEVEL(parent->aa_node.aan_left)+1) {
			 parent->aa_node.aan_level--;
			 if(BLST_AAT_P_Split_isrsafe(tree, parent)) {
				 if(BLST_AAT_P_Split_isrsafe(tree, parent)) {
					 BLST_AAT_P_Skew_isrsafe(parent->aa_node.aan_parent->aa_node.aan_parent);
					 break;
				 }
			 }
			 parent = parent->aa_node.aan_parent;
		 } else if(parent->aa_node.aan_level <= B_AA_NODE_LEVEL(parent->aa_node.aan_right)+1) {
			 break;
		 } else {
			 BLST_AAT_P_Skew_isrsafe(parent);
			 if(parent->aa_node.aan_level > parent->aa_node.aan_parent->aa_node.aan_level) {
				 BLST_AAT_P_Skew_isrsafe(parent);
				 BLST_AAT_P_Split_isrsafe(tree, parent->aa_node.aan_parent->aa_node.aan_parent);
				 break;
			 }
			 parent = parent->aa_node.aan_parent->aa_node.aan_parent;
		 }
	 }
     return;
}

