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
 * Interface for AA tree, type of binary search tree
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "blst_aa_tree.h"
BDBG_MODULE(blst_aa_tree);


#define B_AA_NODE_LEVEL(node) ((node)->aa_node.aan_level)
#define B_AA_LAST_NODE(tree) (&(tree)->aat_root)
#define B_AA_ROOT_NODE(tree) (&(tree)->aat_root)


void 
BLST_AAT_P_Init(struct BLST_AA_Tree_Head *tree)
{
	tree->aat_root.aa_node.aan_level = 0;
	tree->aat_root.aa_node.aan_left = B_AA_LAST_NODE(tree);
	tree->aat_root.aa_node.aan_right = B_AA_LAST_NODE(tree);
	tree->aat_root.aa_node.aan_parent = B_AA_ROOT_NODE(tree);
	return;
}


struct BLST_AA_Tree_Node *
BLST_AAT_P_First(const struct BLST_AA_Tree_Head *tree, unsigned off) 
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
BLST_AAT_P_Last(const struct BLST_AA_Tree_Head *tree, unsigned off) 
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
BLST_AAT_P_Next(struct BLST_AA_Tree_Head *tree, struct BLST_AA_Tree_Node *node, unsigned off)
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
BLST_AAT_P_Prev(struct BLST_AA_Tree_Head *tree, struct BLST_AA_Tree_Node *node, unsigned off)
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
BLST_AAT_P_Skew(struct BLST_AA_Tree_Node *parent)
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
baat_split(struct BLST_AA_Tree_Head *tree, struct BLST_AA_Tree_Node *parent)
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
BLST_AAT_P_Insert_Node(struct BLST_AA_Tree_Head *tree, struct BLST_AA_Tree_Node *new_node, struct BLST_AA_Tree_Node *parent, int cmp)
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
			BLST_AAT_P_Skew(node);
			if( (node->aa_node.aan_level != node->aa_node.aan_right->aa_node.aan_level)) {
				node = node->aa_node.aan_parent;
			}
		}
		if(!baat_split(tree, node->aa_node.aan_parent)) {
			break;
		}
	}
}

void 
BLST_AAT_P_Remove(struct BLST_AA_Tree_Head *tree, struct BLST_AA_Tree_Node *node)
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
			 if(baat_split(tree, parent)) {
				 if(baat_split(tree, parent)) {
					 BLST_AAT_P_Skew(parent->aa_node.aan_parent->aa_node.aan_parent);
					 break;
				 }
			 }
			 parent = parent->aa_node.aan_parent;
		 } else if(parent->aa_node.aan_level <= B_AA_NODE_LEVEL(parent->aa_node.aan_right)+1) {
			 break;
		 } else {
			 BLST_AAT_P_Skew(parent);
			 if(parent->aa_node.aan_level > parent->aa_node.aan_parent->aa_node.aan_level) {
				 BLST_AAT_P_Skew(parent);
				 baat_split(tree, parent->aa_node.aan_parent->aa_node.aan_parent);
				 break;
			 }
			 parent = parent->aa_node.aan_parent->aa_node.aan_parent;
		 }
	 }
     return;
}

