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
#include "bkni.h"

#include <stdio.h>
#include <stdlib.h>
BDBG_MODULE(blst_aa_tree_test);

struct int_tree_node {
    unsigned key;
    BLST_AA_TREE_ENTRY(int_tree_node) node;
};

BLST_AA_TREE_HEAD(int_tree, int_tree_node);

static int int_comp(const struct int_tree_node *node, unsigned key)
{
    if(key > node->key) {
        return 1;
    } else if(key==node->key) {
        return 0;
    } else {
        return -1;
    }
}

BLST_AA_TREE_GENERATE_FIND(int_tree, int, int_tree_node, node, int_comp)
BLST_AA_TREE_GENERATE_FIND_SOME(int_tree, int, int_tree_node, node, int_comp)
BLST_AA_TREE_GENERATE_INSERT(int_tree, int, int_tree_node, node, int_comp)
BLST_AA_TREE_GENERATE_REMOVE(int_tree, int_tree_node, node)
BLST_AA_TREE_GENERATE_FIRST(int_tree, int_tree_node, node)
BLST_AA_TREE_GENERATE_LAST(int_tree, int_tree_node, node)
BLST_AA_TREE_GENERATE_PREV(int_tree, int_tree_node, node)
BLST_AA_TREE_GENERATE_NEXT(int_tree, int_tree_node, node)


#define N_NODES    1000

#define LAST_NODE(tree) (&(tree)->aat_root)
#define NODE_LEVEL(node) ((node)->aa_node.aan_level)

#define PRINT_NODE(tree,node) (unsigned long)(node), (node!=LAST_NODE(tree))?BLST_AA_TREE_P_CAST(node, int_tree_node, off)->key:0, (node!=LAST_NODE(tree))?(node)->aa_node.aan_level:0
static void
print_node(const struct int_tree *int_tree, const struct int_tree_node *int_node)
{
    const unsigned off = BLST_AA_TREE_P_OFF(int_tree_node, node);
    const struct BLST_AA_Tree_Head *tree = &int_tree->aa_tree;
    const struct BLST_AA_Tree_Node *node = BLST_AA_TREE_P_NODE(int_node, off);
    BDBG_WRN(("node %#lx[%d] level:%u left:%#lx[%d->%u] right:%#lx[%d->%u] parent:%#lx[%d->%u]", (unsigned long)node, int_node->key, node->aa_node.aan_level, PRINT_NODE(tree, node->aa_node.aan_left), PRINT_NODE(tree, node->aa_node.aan_right), PRINT_NODE(tree, node->aa_node.aan_parent)));
}

static uint32_t next_rand(uint32_t prev)
{
     return (1664525*prev) + 1013904223;
}

static bool test_node(struct int_tree *int_tree, struct int_tree_node *int_node)
{
    struct BLST_AA_Tree_Head *tree = &int_tree->aa_tree;
    struct BLST_AA_Tree_Node *node = BLST_AA_TREE_P_NODE(int_node, BLST_AA_TREE_P_OFF(int_tree_node, node));

    BDBG_ASSERT(LAST_NODE(tree)==node->aa_node.aan_left || node->aa_node.aan_left->aa_node.aan_parent==node); /* if child exists then child's parent points back */
    BDBG_ASSERT(LAST_NODE(tree)==node->aa_node.aan_right || node->aa_node.aan_right->aa_node.aan_parent==node); /* if child exists then child's parent points back */
    BDBG_ASSERT(node->aa_node.aan_parent==&tree->aat_root || node->aa_node.aan_parent->aa_node.aan_level >= node->aa_node.aan_level); /* if parent exists then it shall be same or higher level */
    BDBG_ASSERT(node->aa_node.aan_level == NODE_LEVEL(node->aa_node.aan_left)+1); /* our level shall be level of left node + 1 */
    BDBG_ASSERT(node->aa_node.aan_level == 1 || (LAST_NODE(tree)!=node->aa_node.aan_right && node->aa_node.aan_level <= node->aa_node.aan_right->aa_node.aan_level + 1)); /* this is bottom node or we are lower then  right level */
    BDBG_ASSERT(node->aa_node.aan_parent==&tree->aat_root || node->aa_node.aan_parent->aa_node.aan_parent==&tree->aat_root || node->aa_node.aan_level < node->aa_node.aan_parent->aa_node.aan_parent->aa_node.aan_level); /* node level shall be smaller than parent's parent level */
    BDBG_ASSERT(BLST_AA_TREE_NEXT(int_tree, int_tree, int_node) == NULL || int_node->key < BLST_AA_TREE_NEXT(int_tree, int_tree, int_node)->key); /* nodes shall be properly ordered */
    return true;
}

static bool
is_unique(int elem, const int *data, size_t nelements)
{
    unsigned i;
    for(i=0;i<nelements;i++) {
        if(elem == data[i]) {
            return false;
        }
    }
    return true;
}

#define WAIT()  /* getchar() */
void test_char_map(void);

void
test_tree(void)
{
    int data[N_NODES];
    struct int_tree_node nodes[N_NODES];
    struct int_tree tree;
    unsigned i;
    struct int_tree_node *node;
    uint32_t rand = 1;

    test_char_map();

    BLST_AA_TREE_INIT(int_tree, &tree);
    for(i=0;i<N_NODES;) {
        rand = next_rand(rand);
        data[i] = rand%(N_NODES*20) ;
        if(data[i]!=0 && is_unique(data[i], data, i)) {
            i++;
        }
    }
    for(i=0;i<N_NODES;i++) {
        nodes[i].key = data[i];
        BLST_AA_TREE_INSERT(int_tree, &tree, data[i], &nodes[i]);
    }
    for(i=0;i<N_NODES;i++) {
        test_node(&tree, &nodes[i]);
    }
    for(i=0;i<N_NODES;i++) {
        node = BLST_AA_TREE_FIND(int_tree, &tree, data[i]);
        BDBG_ASSERT(node==&nodes[i]);
    }
#if 1
    for(node=BLST_AA_TREE_FIRST(int_tree, &tree);node;node=BLST_AA_TREE_NEXT(int_tree, &tree, node)) {
        /* BDBG_ERR(("%lx -> %d", (unsigned long)node, node->key)); */
        test_node(&tree, node);
        print_node(&tree, node);
    }
    WAIT();

    for(node=BLST_AA_TREE_LAST(int_tree, &tree);node;node=BLST_AA_TREE_PREV(int_tree, &tree, node)) {
        /* BDBG_ERR(("%lx -> %d", (unsigned long)node, node->key)); */
        print_node(&tree, node);
    }
    WAIT();
#endif

    for(i=N_NODES/2;i<N_NODES;i++) {
        BLST_AA_TREE_REMOVE(int_tree, &tree, &nodes[i]);
    }
    for(i=0;i<N_NODES/2;i++) {
        test_node(&tree, &nodes[i]);
    }
    for(i=0;i<N_NODES/2;i++) {
        node = BLST_AA_TREE_FIND(int_tree, &tree, data[i]);
        BDBG_ASSERT(node==&nodes[i]);
        node = BLST_AA_TREE_FIND_SOME(int_tree, &tree, data[i]);
        BDBG_ASSERT(node==&nodes[i]);
    }

    for(node=BLST_AA_TREE_FIRST(int_tree, &tree);node;node=BLST_AA_TREE_NEXT(int_tree, &tree, node)) {
        /* BDBG_ERR(("%lx -> %d", (unsigned long)node, node->key)); */
        test_node(&tree, node);
        print_node(&tree, node);
    }
    for(i=N_NODES/2;i<N_NODES;i++) {
        nodes[i].key = data[i];
        BLST_AA_TREE_INSERT(int_tree, &tree, data[i], &nodes[i]);
    }
    for(i=0;i<N_NODES;i++) {
        test_node(&tree, &nodes[i]);
    }
    for(i=2;i<N_NODES;i++) {
        BLST_AA_TREE_REMOVE(int_tree, &tree, &nodes[i]);
    }
    for(i=0;i<N_NODES && i<2;i++) {
        test_node(&tree, &nodes[i]);
    }
    WAIT();

    for(node=BLST_AA_TREE_FIRST(int_tree, &tree);node;node=BLST_AA_TREE_NEXT(int_tree, &tree, node)) {
        /* BDBG_ERR(("%lx -> %d", (unsigned long)node, node->key)); */
        test_node(&tree, node);
        print_node(&tree, node);
    }
    WAIT();
    return;
}

struct char_map_node {
    char key;
    BLST_AA_TREE_ENTRY(char_map_node) node;
    int data;
};

BLST_AA_TREE_HEAD(char_map, char_map_node);

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

BLST_AA_TREE_GENERATE_FIND(char_map, char, char_map_node, node, char_map_cmp)
BLST_AA_TREE_GENERATE_FIND_SOME(char_map, char, char_map_node, node, char_map_cmp)
BLST_AA_TREE_GENERATE_INSERT(char_map, char, char_map_node, node, char_map_cmp)
BLST_AA_TREE_GENERATE_REMOVE(char_map, char_map_node, node)

void test_char_map(void)
{
    struct char_map_node nodes[10];
    struct char_map_node *node;
    struct char_map map;
    char keys[10] = {'0','1','2','3','4','5','6','7','8','9'};
    unsigned i;

    /* initialize the map */
    BLST_AA_TREE_INIT(char_map, &map);

    /* insert elements */
    for(i=0;i<10;i++) {
        char key = keys[i];
        nodes[i].key = key; /* set key */
        nodes[i].data = i; /* set data */
        BLST_AA_TREE_INSERT(char_map, &map, key, &nodes[i]);
    }

    /* lookup elements */
    node = BLST_AA_TREE_FIND(char_map, &map, '0');
    BDBG_ASSERT(node->data == 0);
    node = BLST_AA_TREE_FIND(char_map, &map, '5');
    BDBG_ASSERT(node->data == 5);
    node = BLST_AA_TREE_FIND(char_map, &map, 'A');
    BDBG_ASSERT(node == NULL);

    node = BLST_AA_TREE_FIND_SOME(char_map, &map, '0');
    BDBG_ASSERT(node->data == 0);
    node = BLST_AA_TREE_FIND_SOME(char_map, &map, '5');
    BDBG_ASSERT(node->data == 5);
    node = BLST_AA_TREE_FIND_SOME(char_map, &map, 'A');
    BDBG_ASSERT(node);

    /* remove elements */
    for(i=0;i<10;i++) {
        BLST_AA_TREE_REMOVE(char_map, &map, &nodes[i]);
    }

    /* lookup elements */
    node = BLST_AA_TREE_FIND(char_map, &map, '0');
    BDBG_ASSERT(node == NULL);

    return;
}

/* largest key to store - control size of bitmap */
#define RANDOM_TEST_MAX_NUMBER 1048576*1024
/* largest number of 'live' nodes, controls amount of space allocated by nodes */
#define RANDOM_TEST_MAX_SIZE    (RANDOM_TEST_MAX_NUMBER/8)
/* number of operations between tree test */
#define RANDOM_TEST_ROUNDS_SIZE  RANDOM_TEST_MAX_SIZE/4
#define RANDOM_TEST_ROUNDS 100
#define RANDOM_OP_RANGE     10

typedef struct {
    uint8_t data[RANDOM_TEST_MAX_NUMBER/8];
} bitmap;

static bool bitmap_test(bitmap *b, unsigned no)
{
    unsigned offset = no/8;
    unsigned bit = no%8;
    return 0 != (b->data[offset] & 1<<bit);
}

static void bitmap_set(bitmap *b, unsigned no)
{
    unsigned offset = no/8;
    unsigned bit = no%8;
    b->data[offset] |= 1<<bit;
    return;
}

static void bitmap_clear(bitmap *b, unsigned no)
{
    unsigned offset = no/8;
    unsigned bit = no%8;
    b->data[offset] &= ~(1<<bit);
    return;
}

void random_test(void)
{
    static bitmap bitmap;
    unsigned i;
    size_t nodes=0;
    struct int_tree tree;
    struct int_tree_node *node;
    unsigned op_threshold=1;

    BLST_AA_TREE_INIT(int_tree, &tree);
    BKNI_Memset(&bitmap,0,sizeof(bitmap));

    for(i=0;i<RANDOM_TEST_ROUNDS;i++) {
        unsigned j;
        unsigned remove=0;
        unsigned insert=0;
        unsigned min_nodes=nodes;
        unsigned max_nodes=nodes;
        for(j=0;j<RANDOM_TEST_ROUNDS_SIZE;j++) {
            unsigned n = rand()%RANDOM_TEST_MAX_NUMBER;
            unsigned op = rand()%RANDOM_OP_RANGE;

            if(op<op_threshold && bitmap_test(&bitmap, n)) {
                node = BLST_AA_TREE_FIND(int_tree, &tree, n);
                if(node->key != n) {
                    abort();
                }
                BLST_AA_TREE_REMOVE(int_tree, &tree, node);
                bitmap_clear(&bitmap, n);
                BKNI_Free(node);
                nodes--;
                remove++;
                if(min_nodes>nodes) {
                    min_nodes=nodes;
                }
            }
            if(op>=op_threshold && !bitmap_test(&bitmap, n) && nodes<RANDOM_TEST_ROUNDS_SIZE) {
                node = BKNI_Malloc(sizeof(*node));
                node->key = n;
                BLST_AA_TREE_INSERT(int_tree, &tree, n, node);
                bitmap_set(&bitmap, n);
                nodes++;
                insert++;
                if(max_nodes<nodes) {
                    max_nodes=nodes;
                }
            }
        }
        BKNI_Printf("%u/%u:Tree: %u(%u/%u) nodes (%u/%u) ops\n", i, RANDOM_TEST_ROUNDS, nodes, min_nodes, max_nodes, insert, remove);
        for(j=0;j<RANDOM_TEST_MAX_NUMBER;j++) {
            if(bitmap_test(&bitmap, j)) {
                node = BLST_AA_TREE_FIND(int_tree, &tree, j);
                if(node->key != j) {
                    abort();
                }
            }
        }
        for(j=0,node=BLST_AA_TREE_FIRST(int_tree, &tree);node;node=BLST_AA_TREE_NEXT(int_tree, &tree, node),j++) {
            test_node(&tree, node);
        }
        if(j!=nodes) {
            abort();
        }
        for(j=0,node=BLST_AA_TREE_LAST(int_tree, &tree);node;node=BLST_AA_TREE_PREV(int_tree, &tree, node),j++) {
            test_node(&tree, node);
        }
        if(j!=nodes) {
            abort();
        }
        op_threshold = rand()%RANDOM_OP_RANGE;
    }
    BKNI_Printf("Freeing %u nodes\n", nodes);
    for(i=0;i<RANDOM_TEST_MAX_NUMBER;i++) {
        if(bitmap_test(&bitmap, i)) {
            node = BLST_AA_TREE_FIND(int_tree, &tree, i);
            if(node->key != i) {
                abort();
            }
            BLST_AA_TREE_REMOVE(int_tree, &tree, node);
            BKNI_Free(node);
            nodes--;
        }
    }
    return;
}

#if B_REFSW_WITH_MAIN
int main(void)
{
    BKNI_Init();
    BDBG_Init();

    test_tree();
    random_test();

    BDBG_Uninit();
    BKNI_Uninit();

    return 0;
}
#endif

