/***************************************************************************
 *     Copyright (c) 2003, Broadcom Corporation
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

#include "bstd.h"
#include "bkni.h"

#include "blst_list.h"
#include "blst_queue.h"
#include "blst_slist.h"
#include "blst_squeue.h"
#include "blst_circleq.h"



const static int data_set[] = 
{
/* some random sequence */
    1, 5, 7,  9,  10, 23, 45, 200, 12, 14, 78, 90, 1023, 343, 12, 14, 19, 1323, 932, 34, 544, 6346, 634
};
#define DATA_SET_LEN (sizeof(data_set)/sizeof(*data_set))

#define TEST_FAILED(name) BKNI_Printf("test '%s' failed at %s:%d\n", #name, __FILE__, __LINE__)


/* start of the test for singly-linked list */

#define LST_HEAD BLST_S_HEAD 
#define LST_ENTRY BLST_S_ENTRY
#define LST_INIT BLST_S_INIT
#define LST_INITIALIZER BLST_S_INITIALIZER
#define LST_EMPTY BLST_S_EMPTY
#define LST_FIRST BLST_S_FIRST
#define LST_NEXT BLST_S_NEXT
#define LST_INSERT_HEAD BLST_S_INSERT_HEAD
#define LST_INSERT_AFTER BLST_S_INSERT_AFTER
#define LST_REMOVE_HEAD BLST_S_REMOVE_HEAD

#define data sl_data
#define data_head  sl_data_head 
#define list_populate sl_list_populate
#define list_populate2 sl_list_populate2
#define list_destroy sl_list_destroy
#define list_compare_content sl_list_compare_content
#define list_compare_content2 sl_list_compare_content2
#define list_test sl_list_test
#define list_name "singly-linked list"


#include "list_test.inc"

#undef LST_HEAD 
#undef LST_ENTRY 
#undef LST_INIT 
#undef LST_INITIALIZER 
#undef LST_EMPTY 
#undef LST_FIRST 
#undef LST_NEXT 
#undef LST_INSERT_HEAD 
#undef LST_INSERT_AFTER
#undef LST_REMOVE_HEAD 

#undef data 
#undef data_head  
#undef list_populate 
#undef list_populate2
#undef list_destroy 
#undef list_compare_content
#undef list_compare_content2
#undef list_test 
#undef list_name

/* start of the test for doubly linked list */

#define LST_HEAD BLST_D_HEAD 
#define LST_ENTRY BLST_D_ENTRY
#define LST_INIT BLST_D_INIT
#define LST_INITIALIZER BLST_D_INITIALIZER
#define LST_EMPTY BLST_D_EMPTY
#define LST_FIRST BLST_D_FIRST
#define LST_NEXT BLST_D_NEXT
#define LST_INSERT_HEAD BLST_D_INSERT_HEAD
#define LST_INSERT_AFTER BLST_D_INSERT_AFTER
#define LST_REMOVE_HEAD BLST_D_REMOVE_HEAD

#define data l_data
#define data_head  l_data_head 
#define list_populate l_list_populate
#define list_populate2 l_list_populate2
#define list_destroy l_list_destroy
#define list_compare_content l_list_compare_content
#define list_compare_content2 l_list_compare_content2
#define list_test l_list_test
#define list_name "doubly linked list"

#include "list_test.inc"

#undef LST_HEAD 
#undef LST_ENTRY 
#undef LST_INIT 
#undef LST_INITIALIZER 
#undef LST_EMPTY 
#undef LST_FIRST 
#undef LST_NEXT 
#undef LST_INSERT_HEAD 
#undef LST_INSERT_AFTER
#undef LST_REMOVE_HEAD 

#undef data 
#undef data_head  
#undef list_populate 
#undef list_populate2
#undef list_destroy 
#undef list_compare_content 
#undef list_compare_content2
#undef list_test 
#undef list_name

/* start of the test for singly-linked queue */


#define LST_HEAD BLST_SQ_HEAD 
#define LST_ENTRY BLST_SQ_ENTRY
#define LST_INIT BLST_SQ_INIT
#define LST_INITIALIZER BLST_SQ_INITIALIZER
#define LST_EMPTY BLST_SQ_EMPTY
#define LST_FIRST BLST_SQ_FIRST
#define LST_NEXT BLST_SQ_NEXT
#define LST_INSERT_HEAD BLST_SQ_INSERT_HEAD
#define LST_INSERT_AFTER BLST_SQ_INSERT_AFTER
#define LST_REMOVE_HEAD BLST_SQ_REMOVE_HEAD

#define data sq_data
#define data_head  sq_data_head 
#define list_populate sq_list_populate
#define list_populate2 sq_list_populate2
#define list_destroy sq_list_destroy
#define list_compare_content sq_list_compare_content
#define list_compare_content2 sq_list_compare_content2
#define list_test sq_list_test
#define list_name "singly-linked queue"


#include "list_test.inc"

#undef LST_HEAD 
#undef LST_ENTRY 
#undef LST_INIT 
#undef LST_INITIALIZER 
#undef LST_EMPTY 
#undef LST_FIRST 
#undef LST_NEXT 
#undef LST_INSERT_HEAD 
#undef LST_INSERT_AFTER
#undef LST_REMOVE_HEAD 

#undef data 
#undef data_head  
#undef list_populate 
#undef list_populate2
#undef list_destroy 
#undef list_compare_content
#undef list_compare_content2
#undef list_test 
#undef list_name


/* start of the test for doubly linked queue */

#define LST_HEAD BLST_Q_HEAD 
#define LST_ENTRY BLST_Q_ENTRY
#define LST_INIT BLST_Q_INIT
#define LST_INITIALIZER BLST_Q_INITIALIZER
#define LST_EMPTY BLST_Q_EMPTY
#define LST_FIRST BLST_Q_FIRST
#define LST_NEXT BLST_Q_NEXT
#define LST_INSERT_HEAD BLST_Q_INSERT_HEAD
#define LST_INSERT_AFTER BLST_Q_INSERT_AFTER
#define LST_REMOVE_HEAD BLST_Q_REMOVE_HEAD

#define data q_data
#define data_head  q_data_head 
#define list_populate q_list_populate
#define list_populate2 q_list_populate2
#define list_destroy q_list_destroy
#define list_compare_content q_list_compare_content
#define list_compare_content2 q_list_compare_content2
#define list_test q_list_test
#define list_name "doubly linked queue"


#include "list_test.inc"

#undef LST_HEAD 
#undef LST_ENTRY 
#undef LST_INIT 
#undef LST_INITIALIZER 
#undef LST_EMPTY 
#undef LST_FIRST 
#undef LST_NEXT 
#undef LST_INSERT_HEAD 
#undef LST_INSERT_AFTER
#undef LST_REMOVE_HEAD 

#undef data 
#undef data_head  
#undef list_populate 
#undef list_populate2
#undef list_destroy 
#undef list_compare_content 
#undef list_compare_content2
#undef list_test 
#undef list_name


typedef struct cq_data {
   char buf1[16];
   BLST_CQ_ENTRY(cq_data) link;
   int number;
   char buf2[16];
} cq_data;


typedef struct cq_data_head cq_data_head;

BLST_CQ_HEAD(cq_data_head, cq_data);

static void
cq_list_populate( cq_data_head *list)
{
    cq_data *itm;
    int i;

    for(i=0;i< DATA_SET_LEN; i++) {
        itm = BKNI_Malloc(sizeof(*itm));
        BKNI_Memset(itm, 0xCD, sizeof(*itm)); /* fill with junk */
        itm->number = data_set[i];
        BLST_CQ_INSERT_HEAD(list, itm, link);
    }
    return;
}


static void
cq_list_populate2( cq_data_head *list)
{
    cq_data *itm, *cur, *last;
    int i;

    for(i=0;i< DATA_SET_LEN; i++) {
        itm = BKNI_Malloc(sizeof(*itm));
        BKNI_Memset(itm, 0xCD, sizeof(*itm)); /* fill with junk */
        itm->number = data_set[i];

		for (last=NULL, cur=BLST_CQ_FIRST(list); cur;) {
			last=cur;
            cur = BLST_CQ_NEXT(cur, link);
            if (cur==BLST_CQ_FIRST(list)) { break; }
		}
		if (last) {
			BLST_CQ_INSERT_AFTER(list, last, itm, link);
		} else {
			BLST_CQ_INSERT_HEAD(list, itm, link);
		}
    }
    return;
}

static void
cq_list_populate3( cq_data_head *list)
{
    cq_data *itm;
    int i;

    for(i=DATA_SET_LEN-1;i>0 ; i--) {
        itm = BKNI_Malloc(sizeof(*itm));
        BKNI_Memset(itm, 0xCD, sizeof(*itm)); /* fill with junk */
        itm->number = data_set[i];
        BLST_CQ_INSERT_TAIL(list, itm, link);
    }
    return;
}

static int 
cq_list_compare_content( cq_data_head *list)
{
    int i;
    cq_data *itm;

    for(i=DATA_SET_LEN-1, itm=BLST_CQ_FIRST(list); ; i--) {
        if (itm->number!=data_set[i]) {
            return -1;
        }
        itm=BLST_CQ_NEXT(itm, link);
        if (itm == BLST_CQ_FIRST(list)) { break; }
    }
    return 0;
}

static int 
cq_list_compare_content2( cq_data_head *list)
{
    int i;
    cq_data *itm;

    for(i=0, itm=BLST_CQ_FIRST(list); ; i++) {
        if (itm->number!=data_set[i]) {
            return -1;
        }
        itm=BLST_CQ_NEXT(itm, link);
        if (itm == BLST_CQ_FIRST(list)) { break; }
    }
    return 0;
}


static void
cq_list_destroy( cq_data_head *list)
{
    cq_data *itm;

    while( NULL!=(itm=BLST_CQ_FIRST(list))) {
	    BLST_CQ_REMOVE_HEAD(list, link);
        BKNI_Free(itm);
    }
    return;

}

int 
cq_list_test(void)
{
    cq_data_head list1 = BLST_CQ_INITIALIZER(&list1);
    cq_data_head list2;
    int rc;

	BKNI_Printf("Start list_test for %s\n", "circular queue");
    BLST_CQ_INIT(&list2);

    cq_list_populate(&list1);
    cq_list_populate(&list2);

    rc = cq_list_compare_content(&list1);
    if (rc<0) {
        TEST_FAILED(cq_list_compare_content(&list1));
        return rc;
    }

    cq_list_destroy(&list1);

    rc = cq_list_compare_content(&list2);
    if (rc<0) {
        TEST_FAILED(cq_list_compare_content(&list2));
        return rc;
    }

    cq_list_destroy(&list2);

	BKNI_Printf(".");

    cq_list_populate2(&list1);
    cq_list_populate2(&list2);

    rc = cq_list_compare_content2(&list1);
    if (rc<0) {
        TEST_FAILED(cq_list_compare_content(&list1));
        return rc;
    }

    cq_list_destroy(&list1);

    rc = cq_list_compare_content2(&list2);
    if (rc<0) {
        TEST_FAILED(cq_list_compare_content(&list2));
        return rc;
    }

    cq_list_destroy(&list2);

    cq_list_populate3(&list1);
    cq_list_populate3(&list2);

    rc = cq_list_compare_content(&list1);
    if (rc<0) {
        TEST_FAILED(cq_list_compare_content(&list1));
        return rc;
    }

    cq_list_destroy(&list1);

    rc = cq_list_compare_content(&list2);
    if (rc<0) {
        TEST_FAILED(cq_list_compare_content(&list2));
        return rc;
    }

    cq_list_destroy(&list2);


	BKNI_Printf(".");

	BKNI_Printf("\nDone\n");

    return 0;

}

static void
q_list_populate3( q_data_head *list)
{
    q_data *itm;
    int i;

    for(i=0;i< DATA_SET_LEN; i++) {
        itm = BKNI_Malloc(sizeof(*itm));
        BKNI_Memset(itm, 0xCD, sizeof(*itm)); /* fill with junk */
        itm->number = data_set[i];
        BLST_Q_INSERT_TAIL(list, itm, link);
    }
    return;
}

static int 
q_list_compare_content3( q_data_head *list)
{
    int i;
    q_data *itm;

    for(i=DATA_SET_LEN-1, itm=BLST_Q_LAST(list); i>=0; itm=BLST_Q_PREV(itm, link), i--) {
        if (itm->number!=data_set[i]) {
            return -1;
        }
    }
    return 0;
}



int 
q_list_test2(void)
{
    q_data_head list1 = BLST_Q_INITIALIZER(&list1);
    int rc;

	BKNI_Printf("Start list_test for %s\n", "queue2");
    BLST_Q_INIT(&list1);

    q_list_populate3(&list1);

    rc = q_list_compare_content2(&list1);
    if (rc<0) {
        TEST_FAILED(q_list_compare_content2(&list1));
        return rc;
    }
    rc = q_list_compare_content3(&list1);
    if (rc<0) {
        TEST_FAILED(q_list_compare_content3(&list1));
        return rc;
    }


    q_list_destroy(&list1);

	BKNI_Printf(".");

	BKNI_Printf("\nDone\n");

    return 0;

}


int main(void)
{
    BKNI_Init();
    BDBG_Init();

    sl_list_test();
    l_list_test();
    sq_list_test();
    q_list_test();
    q_list_test2();
    cq_list_test();

    BDBG_Uninit();
    BKNI_Uninit();

    return 0;

}

