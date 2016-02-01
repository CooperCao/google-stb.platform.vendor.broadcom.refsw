
/* include file to perform test on any given link type */
typedef struct data {
   char buf1[16];
   LST_ENTRY(data) link;
   int number;
   char buf2[16];
} data;


typedef struct data_head data_head;

LST_HEAD(data_head, data);

static void
list_populate( data_head *list)
{
    data *itm;
    int i;

    for(i=0;i< DATA_SET_LEN; i++) {
        itm = BKNI_Malloc(sizeof(*itm));
        BKNI_Memset(itm, 0xCD, sizeof(*itm)); /* fill with junk */
        itm->number = data_set[i];
        LST_INSERT_HEAD(list, itm, link);
    }
    return;
}


static void
list_populate2( data_head *list)
{
    data *itm, *cur, *last;
    int i;

    for(i=0;i< DATA_SET_LEN; i++) {
        itm = BKNI_Malloc(sizeof(*itm));
        BKNI_Memset(itm, 0xCD, sizeof(*itm)); /* fill with junk */
        itm->number = data_set[i];

		for (last=NULL, cur=LST_FIRST(list); cur; cur = LST_NEXT(cur, link)) {
			last=cur;
		}
		if (last) {
			LST_INSERT_AFTER(list, last, itm, link);
		} else {
			LST_INSERT_HEAD(list, itm, link);
		}
    }
    return;
}

static int 
list_compare_content( data_head *list)
{
    int i;
    data *itm;

    for(i=DATA_SET_LEN-1, itm=LST_FIRST(list); itm ; itm=LST_NEXT(itm, link), i--) {
        if (itm->number!=data_set[i]) {
            return -1;
        }
    }
    return 0;
}

static int 
list_compare_content2( data_head *list)
{
    int i;
    data *itm;

    for(i=0, itm=LST_FIRST(list); itm ; itm=LST_NEXT(itm, link), i++) {
        if (itm->number!=data_set[i]) {
            return -1;
        }
    }
    return 0;
}

static void
list_destroy( data_head *list)
{
    data *itm;

    while( NULL!=(itm=LST_FIRST(list))) {
	    LST_REMOVE_HEAD(list, link);
        BKNI_Free(itm);
    }
    return;

}

int 
list_test(void)
{
    data_head list1 = LST_INITIALIZER(&list1);
    data_head list2;
    int rc;

	BKNI_Printf("Start list_test for %s\n", list_name);
    LST_INIT(&list2);

    list_populate(&list1);
    list_populate(&list2);

    rc = list_compare_content(&list1);
    if (rc<0) {
        TEST_FAILED(list_compare_content(&list1));
        return rc;
    }

    list_destroy(&list1);

    rc = list_compare_content(&list2);
    if (rc<0) {
        TEST_FAILED(list_compare_content(&list2));
        return rc;
    }

    list_destroy(&list2);

	BKNI_Printf(".");

    list_populate2(&list1);
    list_populate2(&list2);

    rc = list_compare_content2(&list1);
    if (rc<0) {
        TEST_FAILED(list_compare_content(&list1));
        return rc;
    }

    list_destroy(&list1);

    rc = list_compare_content2(&list2);
    if (rc<0) {
        TEST_FAILED(list_compare_content(&list2));
        return rc;
    }

    list_destroy(&list2);

	BKNI_Printf(".");

	BKNI_Printf("\nDone\n");

    return 0;

}

