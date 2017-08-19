/*
 * Basic unit test for Channel Context module
 *
 * $ Copyright Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:.*>>
 *
 * $Id$
 */

/***************************************************************************************************
************* Definitions for module components to be tested with Check tool ***********************
*/

#include <typedefs.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include <osl.h>
#include <wlc_chctx_reg.h>

#ifdef BCMDBG_ASSERT
#include <assert.h>
#undef ASSERT
#define ASSERT(exp) assert(exp)
#endif

#include "test_chctx_reg.h"

/***************************************************************************************************
************************************* Start of Test Section ****************************************
*/

#include <check.h> /* Includes Check framework */

/*
 * In order to run unit tests with Check, we must create some test cases,
 * aggregate them into a suite, and run them with a suite runner.
 *
 * The pattern of every unit test is as following
 *
 * START_TEST(name_of_test){
 *
 *     perform tests;
 *     ...
 *     assert results
 * }
 * END_TEST
 *
 * Test Case is a set of at least 1 unit test
 * Test Suite is a collection of Test Cases
 * Check Framework can run multiple Test Suites.
 */

/* ------------- Global Definitoions ------------------------- */


/*
 * Global variables definitions, for setup and teardown function.
 */
static osl_t *osh = NULL;
static wlc_chctx_reg_t *chctx_reg = NULL;

/* capacities and objects */

/* 2 clients */
#define NUM_CLIENTS 2
/* 3 cubbies */
#define NUM_CUBBIES 3
/* 4 entries */
#define NUM_ENTRIES 4

/* 2 client objects */
#define CLNT1_OBJECT (void *)(uintptr)1
#define CLNT2_OBJECT (void *)(uintptr)2
#define CLNT3_OBJECT (void *)(uintptr)3

/* 3 cubbie objects - 2 for client 1; 1 for client 2 - see wlc_chctx_reg_add_cubby() calls */
#define CLNT1_CUBBY1_OBJECT (void *)(uintptr)1
#define CLNT1_CUBBY2_OBJECT (void *)(uintptr)2
#define CLNT2_CUBBY1_OBJECT (void *)(uintptr)1
#define CLNT3_CUBBY1_OBJECT (void *)(uintptr)1

/* ------------- Startup and Teardown - Fixtures ---------------
 * Setting up objects for each unit test,
 * it may be convenient to add some setup that is constant across all the tests in a test case
 * rather than setting up objects for each unit test.
 * Before each unit test in a test case, the setup() function is run, if defined.
 */
void
setup(void)
{
	int ret;

	/* try to register dump fn */
	ret = wlc_dump_add_fns(NULL, "chctx", wlc_chctx_reg_dump, NULL, NULL);
	ck_assert_int_eq(ret, BCME_OK);
}

/*
 * Tear down objects for each unit test,
 * it may be convenient to add teardown that is constant across all the tests in a test case
 * rather than tearing down objects for each unit test.
 * After each unit test in a test case, the setup() function is run, if defined.
 * Note: checked teardown() fixture will not run if the unit test fails.
*/
void
teardown(void)
{
}

/* dump */
extern int (*our_dump_fn)(void *ctx, struct bcmstrbuf *b); /* set by wlc_dump_register */

static char output_buf[2048];

static void
output_chctx_reg_info(wlc_chctx_reg_t *chctx_reg)
{
	struct bcmstrbuf b;

	ASSERT(our_dump_fn != NULL);

	bcm_binit(&b, output_buf, sizeof(output_buf));
	(void) our_dump_fn(chctx_reg, &b);
	(void) printf("%s", output_buf);
}

/* storage of all cubbies and contexts */
#define CLNT1_CUBBY_CTXSZ 4
#define CLNT2_CUBBY_CTXSZ 8

static uint8 client1_cubby1_buf[CLNT1_CUBBY_CTXSZ * NUM_ENTRIES];
static uint8 client2_cubby1_buf[CLNT2_CUBBY_CTXSZ * NUM_ENTRIES];
static uint8 client1_cubby2_buf[CLNT1_CUBBY_CTXSZ * NUM_ENTRIES];

/* the current channel chanspec */
static chanspec_t cur_chspec = 0;

static int invalid_ctx_cnt = 0;

/* client 1 callbacks */
/* client callback responsibility (init):
 * - initialize context to known values
 */
static void
test_chctx_client1_init(wlc_chctx_client_ctx_t *client,
	wlc_chctx_cubby_ctx_t *cubby, uint8 *buf)
{
	uint8 val;
	char hex[128];
	struct bcmstrbuf b;

	/* use the client# as the upper nibble */
	/* use the cubby# as the lower nibble */
	val = ((uint8)(uintptr)client << 4) | (uint8)(uintptr)cubby;
	memset(buf, val, CLNT1_CUBBY_CTXSZ);

	bcm_binit(&b, hex, sizeof(hex));
	bcm_bprintf(&b, "%s: client %p cubby %p buf %p: ", __FUNCTION__, client, cubby, buf);
	bcm_bprhex(&b, NULL, TRUE, buf, CLNT1_CUBBY_CTXSZ);
	printf("%s", hex);
}

/* client callback responsibility (save/restore):
 * - verify client pointer is correct
 * - verify cubby pointer is correct
 * - context pointer is correct
 */

static int
test_chctx_client1_restore(wlc_chctx_client_ctx_t *client,
	wlc_chctx_cubby_ctx_t *cubby, uint8 *buf)
{
	uint8 *ctxbuf;
	uint8 chan;
	char hex[128];
	struct bcmstrbuf b;

	if (buf == NULL) {
		invalid_ctx_cnt ++;
		return BCME_OK;
	}

	bcm_binit(&b, hex, sizeof(hex));
	bcm_bprintf(&b, "%s: client %p cubby %p buf %p: ", __FUNCTION__, client, cubby, buf);
	bcm_bprhex(&b, NULL, TRUE, buf, CLNT1_CUBBY_CTXSZ);
	printf("%s", hex);

	ck_assert_ptr_eq(client, CLNT1_OBJECT);

	if (cubby == CLNT1_CUBBY1_OBJECT) {
		ctxbuf = client1_cubby1_buf;
	}
	else if (cubby == CLNT1_CUBBY2_OBJECT) {
		ctxbuf = client1_cubby2_buf;
	}
	else {
		ASSERT(0);
	}

	chan = CHSPEC_CHANNEL(cur_chspec);

	/* use (chan# - 1) as the index: */
	return buf == ctxbuf + (chan - 1) * CLNT1_CUBBY_CTXSZ ? BCME_OK : BCME_ERROR;
}

static int
test_chctx_client1_dump(wlc_chctx_client_ctx_t *client,
	wlc_chctx_cubby_ctx_t *cubby, uint8 *buf, struct bcmstrbuf *b)
{
	bcm_bprhex(b, NULL, TRUE, buf, CLNT1_CUBBY_CTXSZ);

	return BCME_OK;
}

static wlc_chctx_client_fn_t client1_fns = {
	.init = test_chctx_client1_init,
	.restore = test_chctx_client1_restore,
	.dump = test_chctx_client1_dump
};

/* client 2 callbacks */
static void
test_chctx_client2_init(wlc_chctx_client_ctx_t *client,
	wlc_chctx_cubby_ctx_t *cubby, uint8 *buf)
{
	uint8 val;
	char hex[128];
	struct bcmstrbuf b;

	/* use the client# as the upper nibble */
	/* use the cubby# as the lower nibble */
	val = ((uint8)(uintptr)client << 4) | (uint8)(uintptr)cubby;
	memset(buf, val, CLNT2_CUBBY_CTXSZ);

	bcm_binit(&b, hex, sizeof(hex));
	bcm_bprintf(&b, "%s: client %p cubby %p buf %p: ", __FUNCTION__, client, cubby, buf);
	bcm_bprhex(&b, NULL, TRUE, buf, CLNT2_CUBBY_CTXSZ);
	printf("%s", hex);
}

static int
test_chctx_client2_save(wlc_chctx_client_ctx_t *client,
	wlc_chctx_cubby_ctx_t *cubby, uint8 *buf)
{
	char hex[128];
	struct bcmstrbuf b;

	bcm_binit(&b, hex, sizeof(hex));
	bcm_bprintf(&b, "%s: client %p cubby %p buf %p: ", __FUNCTION__, client, cubby, buf);
	bcm_bprhex(&b, NULL, TRUE, buf, CLNT2_CUBBY_CTXSZ);
	printf("%s", hex);

	return BCME_OK;
}

static int
test_chctx_client2_restore(wlc_chctx_client_ctx_t *client,
	wlc_chctx_cubby_ctx_t *cubby, uint8 *buf)
{
	uint8 *ctxbuf;
	uint8 chan;
	char hex[128];
	struct bcmstrbuf b;

	if (buf == NULL) {
		invalid_ctx_cnt ++;
		return BCME_OK;
	}

	bcm_binit(&b, hex, sizeof(hex));
	bcm_bprintf(&b, "%s: client %p cubby %p buf %p: ", __FUNCTION__, client, cubby, buf);
	bcm_bprhex(&b, NULL, TRUE, buf, CLNT2_CUBBY_CTXSZ);
	printf("%s", hex);

	ck_assert_ptr_eq(client, CLNT2_OBJECT);
	ck_assert_ptr_eq(cubby, CLNT2_CUBBY1_OBJECT);

	ctxbuf = client2_cubby1_buf;

	chan = CHSPEC_CHANNEL(cur_chspec);

	/* use (chan# - 1) as the index: */
	return buf == ctxbuf + (chan - 1) * CLNT2_CUBBY_CTXSZ ? BCME_OK : BCME_ERROR;
}

static int
test_chctx_client2_dump(wlc_chctx_client_ctx_t *client,
	wlc_chctx_cubby_ctx_t *cubby, uint8 *buf, struct bcmstrbuf *b)
{
	bcm_bprhex(b, NULL, TRUE, buf, CLNT2_CUBBY_CTXSZ);

	return BCME_OK;
}

static wlc_chctx_client_fn_t client2_fns = {
	.init = test_chctx_client2_init,
	.save = test_chctx_client2_save,
	.restore = test_chctx_client2_restore,
	.dump = test_chctx_client2_dump
};

static wlc_chctx_client_fn_t client3_fns = {
	.init = test_chctx_client2_init,
	.save = test_chctx_client2_save,
	.restore = test_chctx_client2_restore,
	.dump = test_chctx_client2_dump
};

/* test 1 */
START_TEST(test_1)
{
	int client1_id;
	int client2_id;
	int client1_cubby1_id;
	int client2_cubby1_id;
	int client1_cubby2_id;
	int ret;
	wlc_chctx_notif_t notif;
	uint8 *loc;

	chctx_reg = wlc_chctx_reg_attach(osh, 0, WLC_CHCTX_REG_H_TYPE,
			NUM_CLIENTS, NUM_CUBBIES, NUM_ENTRIES);
	ASSERT(chctx_reg != NULL);

	/* try to add client1 */
	client1_id = wlc_chctx_reg_add_client(chctx_reg, &client1_fns, CLNT1_OBJECT);
	printf("wlc_chctx_reg_add_client(%p, %d): cid=%d\n", &client1_fns, CLNT1_OBJECT,
	       client1_id);
	ck_assert_int_eq(client1_id, 0);

	/* try to add client2 */
	client2_id = wlc_chctx_reg_add_client(chctx_reg, &client2_fns, CLNT2_OBJECT);
	printf("wlc_chctx_reg_add_client(%p, %d): cid=%d\n", &client2_fns, CLNT2_OBJECT,
	       client2_id);
	ck_assert_int_eq(client2_id, 1);

	/* negative: try add another client */
	ret = wlc_chctx_reg_add_client(chctx_reg, &client3_fns, CLNT3_OBJECT);
	printf("[negative] wlc_chctx_reg_add_client(%p, %d): ret=%d\n",
		&client3_fns, CLNT3_OBJECT, ret);
	ck_assert_int_eq(ret, BCME_NORESOURCE);

	/* try add client 1 cubby 1 */
	client1_cubby1_id = wlc_chctx_reg_add_cubby(chctx_reg, client1_id,
		client1_cubby1_buf, sizeof(client1_cubby1_buf), CLNT1_CUBBY1_OBJECT);
	printf("wlc_chctx_reg_add_cubby(%d, %p, %p, %d): cid=%d\n",
	       client1_id, client1_cubby1_buf, sizeof(client1_cubby1_buf), CLNT1_CUBBY1_OBJECT,
	       client1_cubby1_id);
	ck_assert_int_eq(client1_cubby1_id, 0);

	/* try add client 2 cubby 1 */
	client2_cubby1_id = wlc_chctx_reg_add_cubby(chctx_reg, client2_id,
		client2_cubby1_buf, sizeof(client2_cubby1_buf), CLNT2_CUBBY1_OBJECT);
	printf("wlc_chctx_reg_add_cubby(%d, %p, %p, %d): cid=%d\n",
	       client2_id, client2_cubby1_buf, sizeof(client2_cubby1_buf), CLNT2_CUBBY1_OBJECT,
	       client2_cubby1_id);
	ck_assert_int_eq(client2_cubby1_id, 1);

	/* try add client 1 cubby 2 */
	client1_cubby2_id = wlc_chctx_reg_add_cubby(chctx_reg, client1_id,
		client1_cubby2_buf, sizeof(client1_cubby2_buf), CLNT1_CUBBY2_OBJECT);
	printf("wlc_chctx_reg_add_cubby(%d, %p, %p, %d): cid=%d\n",
	       client1_id, client1_cubby2_buf, sizeof(client1_cubby2_buf), CLNT1_CUBBY2_OBJECT,
	       client1_cubby2_id);
	ck_assert_int_eq(client1_cubby2_id, 2);

	/* negative: try add another cubby */
	ret = wlc_chctx_reg_add_cubby(chctx_reg, 0, client2_cubby1_buf, sizeof(client2_cubby1_buf), 0);
	printf("[negative] wlc_chctx_reg_add_cubby(%d, %p, %p, %d): ret=%d\n", 0, NULL, 0, NULL, ret);
	ck_assert_int_eq(ret, BCME_NORESOURCE);

	/* try delete client 1 cubby 1 */
	wlc_chctx_reg_del_cubby(chctx_reg, client1_cubby1_id);
	printf("wlc_chctx_reg_del_cubby(%d)\n", client1_cubby1_id);

	/* try add back client 1 cubby 1 */
	client1_cubby1_id = wlc_chctx_reg_add_cubby(chctx_reg, client1_id,
		client1_cubby1_buf, sizeof(client1_cubby1_buf), CLNT1_CUBBY1_OBJECT);
	printf("wlc_chctx_reg_add_cubby(%d, %p, %d, %d): cid=%d\n",
	       client1_id, client1_cubby1_buf, sizeof(client1_cubby1_buf), CLNT1_CUBBY1_OBJECT,
	       client1_cubby1_id);
	ck_assert_int_eq(client1_cubby1_id, 0);

	/* try delete client 2 cubby 1 */
	wlc_chctx_reg_del_cubby(chctx_reg, client2_cubby1_id);
	printf("wlc_chctx_reg_del_cubby(%d)\n", client2_cubby1_id);

	/* try add back client 2 cubby 1 */
	client2_cubby1_id = wlc_chctx_reg_add_cubby(chctx_reg, client2_id,
		client2_cubby1_buf, sizeof(client2_cubby1_buf), CLNT2_CUBBY1_OBJECT);
	printf("wlc_chctx_reg_add_cubby(%d, %p, %d, %d): cid=%d\n",
	       client2_id, client2_cubby1_buf, sizeof(client2_cubby1_buf), CLNT2_CUBBY1_OBJECT,
	       client2_cubby1_id);
	ck_assert_int_eq(client2_cubby1_id, 1);

	/* try delete client 1 cubby 2 */
	wlc_chctx_reg_del_cubby(chctx_reg, client1_cubby2_id);
	printf("wlc_chctx_reg_del_cubby(%d)\n", client1_cubby2_id);

	/* try add back client 1 cubby 2 */
	client1_cubby2_id = wlc_chctx_reg_add_cubby(chctx_reg, client1_id,
		client1_cubby2_buf, sizeof(client1_cubby2_buf), CLNT1_CUBBY2_OBJECT);
	printf("wlc_chctx_reg_add_cubby(%d, %p, %d, %d): cid=%d\n",
	       client1_id, client1_cubby2_buf, sizeof(client1_cubby2_buf), CLNT1_CUBBY2_OBJECT,
	       client1_cubby2_id);
	ck_assert_int_eq(client1_cubby2_id, 2);

	/* signal context open for channel 1 */
	notif.event = WLC_CHCTX_OPEN_CHAN;
	notif.chanspec = CH20MHZ_CHSPEC(1);
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("wlc_chctx_reg_notif(%d, %d): ret=%d\n", WLC_CHCTX_OPEN_CHAN, 1, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* signal context open for channel 2 */
	notif.event = WLC_CHCTX_OPEN_CHAN;
	notif.chanspec = CH20MHZ_CHSPEC(2);
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("wlc_chctx_reg_notif(%d, %d): ret=%d\n", WLC_CHCTX_OPEN_CHAN, 2, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* signal context switch - make channel 1 the current context */
	cur_chspec = CH20MHZ_CHSPEC(1);
	notif.event = WLC_CHCTX_ENTER_CHAN;
	notif.chanspec = cur_chspec;
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("wlc_chctx_reg_notif(%d, %d): ret=%d\n", WLC_CHCTX_ENTER_CHAN, 1, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* negative: signal context - make channel 3 the current context */
	invalid_ctx_cnt = 0;
	cur_chspec = CH20MHZ_CHSPEC(3);
	notif.event = WLC_CHCTX_ENTER_CHAN;
	notif.chanspec = cur_chspec;
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("[negative] wlc_chctx_reg_notif(%d, %d): ret=%d\n", WLC_CHCTX_ENTER_CHAN, 3, ret);
	ck_assert_int_eq(ret, BCME_NOTFOUND);
	/* make sure no context notification happened */
	ck_assert_int_eq(invalid_ctx_cnt, 3);

	/* signal context switch - make channel 2 the current context */
	cur_chspec = CH20MHZ_CHSPEC(2);
	notif.event = WLC_CHCTX_ENTER_CHAN;
	notif.chanspec = cur_chspec;
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("wlc_chctx_reg_notif(%d, %d): ret=%d\n", WLC_CHCTX_ENTER_CHAN, 2, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* signal context switch - make channel 1 the current context */
	cur_chspec = CH20MHZ_CHSPEC(1);
	notif.event = WLC_CHCTX_ENTER_CHAN;
	notif.chanspec = cur_chspec;
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("wlc_chctx_reg_notif(%d, %d): ret=%d\n", WLC_CHCTX_ENTER_CHAN, 1, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* query storage location for channel 2 */
	loc = wlc_chctx_reg_query_cubby(chctx_reg, client1_cubby2_id, CH20MHZ_CHSPEC(2));
	printf("wlc_chctx_reg_query_cubby(%d, %x): loc=%p\n",
	       client1_cubby2_id, CH20MHZ_CHSPEC(2), loc);
	ck_assert_int_eq(loc, client1_cubby2_buf + CLNT1_CUBBY_CTXSZ);

	/* signal context switch - save the current context (channel 1) */
	notif.event = WLC_CHCTX_LEAVE_CHAN;
	notif.chanspec = 0;
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("wlc_chctx_reg_notif(%d): ret=%d\n", WLC_CHCTX_LEAVE_CHAN, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* signal context switch - make channel 2 the current context */
	cur_chspec = CH20MHZ_CHSPEC(2);
	notif.event = WLC_CHCTX_ENTER_CHAN;
	notif.chanspec = cur_chspec;
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("wlc_chctx_reg_notif(%d, %d): ret=%d\n", WLC_CHCTX_ENTER_CHAN, 2, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* signal context switch - make channel 1 the current context */
	cur_chspec = CH20MHZ_CHSPEC(1);
	notif.event = WLC_CHCTX_ENTER_CHAN;
	notif.chanspec = cur_chspec;
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("wlc_chctx_reg_notif(%d, %d): ret=%d\n", WLC_CHCTX_ENTER_CHAN, 1, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* signal context open for channel 3 */
	notif.event = WLC_CHCTX_OPEN_CHAN;
	notif.chanspec = CH20MHZ_CHSPEC(3);
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("wlc_chctx_reg_notif(%d, %d): ret=%d\n", WLC_CHCTX_OPEN_CHAN, 3, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* signal context - make channel 3 the current context */
	cur_chspec = CH20MHZ_CHSPEC(3);
	notif.event = WLC_CHCTX_ENTER_CHAN;
	notif.chanspec = cur_chspec;
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("wlc_chctx_reg_notif(%d, %d): ret=%d\n", WLC_CHCTX_ENTER_CHAN, 3, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* signal context - make channel 2 the current context */
	cur_chspec = CH20MHZ_CHSPEC(2);
	notif.event = WLC_CHCTX_ENTER_CHAN;
	notif.chanspec = cur_chspec;
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("wlc_chctx_reg_notif(%d, %d): ret=%d\n", WLC_CHCTX_ENTER_CHAN, 2, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* dump */
	output_chctx_reg_info(chctx_reg);

	/* signal context close for channel 1 */
	notif.event = WLC_CHCTX_CLOSE_CHAN;
	notif.chanspec = CH20MHZ_CHSPEC(1);
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("wlc_chctx_reg_notif(%d, %d): ret=%d\n", WLC_CHCTX_CLOSE_CHAN, 1, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* signal context close for channel 2 */
	notif.event = WLC_CHCTX_CLOSE_CHAN;
	notif.chanspec = CH20MHZ_CHSPEC(2);
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("wlc_chctx_reg_notif(%d, %d): ret=%d\n", WLC_CHCTX_CLOSE_CHAN, 2, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* dump */
	output_chctx_reg_info(chctx_reg);

	wlc_chctx_reg_detach(chctx_reg);
	printf("wlc_chctx_reg_detach(%p)\n", chctx_reg);
}
END_TEST

#define TEST2_CLNT1_CUBBY_CTXSZ 8

/* client 1 callbacks */
/* client callback responsibility (init):
 * - initialize context to known values
 */
static void
test2_chctx_client1_init(wlc_chctx_client_ctx_t *client,
	wlc_chctx_cubby_ctx_t *cubby, uint8 *buf)
{
	uint8 val;
	char hex[128];
	struct bcmstrbuf b;

	/* use the client# as the upper nibble */
	/* use the cubby# as the lower nibble */
	val = ((uint8)(uintptr)client << 4) | (uint8)(uintptr)cubby;
	memset(buf, val, TEST2_CLNT1_CUBBY_CTXSZ);

	bcm_binit(&b, hex, sizeof(hex));
	bcm_bprintf(&b, "%s: client %p cubby %p buf %p: ", __FUNCTION__, client, cubby, buf);
	bcm_bprhex(&b, NULL, TRUE, buf, TEST2_CLNT1_CUBBY_CTXSZ);
	printf("%s", hex);
}

/* client callback responsibility (save/restore):
 * - verify client pointer is correct
 * - verify cubby pointer is correct
 * - context pointer is correct
 */

static int
test2_chctx_client1_restore(wlc_chctx_client_ctx_t *client,
	wlc_chctx_cubby_ctx_t *cubby, uint8 *buf)
{
	uint8 *ctxbuf;
	uint8 chan;
	char hex[128];
	struct bcmstrbuf b;

	bcm_binit(&b, hex, sizeof(hex));
	bcm_bprintf(&b, "%s: client %p cubby %p buf %p: ", __FUNCTION__, client, cubby, buf);
	bcm_bprhex(&b, NULL, TRUE, buf, TEST2_CLNT1_CUBBY_CTXSZ);
	printf("%s", hex);

	ck_assert_ptr_eq(client, CLNT1_OBJECT);

	return BCME_OK;
}

static int
test2_chctx_client1_dump(wlc_chctx_client_ctx_t *client,
	wlc_chctx_cubby_ctx_t *cubby, uint8 *buf, struct bcmstrbuf *b)
{
	bcm_bprhex(b, NULL, TRUE, buf, TEST2_CLNT1_CUBBY_CTXSZ);

	return BCME_OK;
}

static wlc_chctx_client_fn_t test2_client1_fns = {
	.init = test2_chctx_client1_init,
	.restore = test2_chctx_client1_restore,
	.dump = test2_chctx_client1_dump
};

static uint8 chan1_buf[TEST2_CLNT1_CUBBY_CTXSZ * 2];
static uint8 chan2_buf[TEST2_CLNT1_CUBBY_CTXSZ * 2];
static uint8 chan3_buf[TEST2_CLNT1_CUBBY_CTXSZ * 2];

/* test 1 */
START_TEST(test_2)
{
	int client1_id;
	int client1_cubby1_id;
	int client1_cubby2_id;
	int ctl1;
	int ctl2;
	int ctl3;
	int ret;
	wlc_chctx_notif_t notif;
	chanspec_t chanspec;
	uint8 *loc;

	chctx_reg = wlc_chctx_reg_attach(osh, 0, WLC_CHCTX_REG_V_TYPE, 1, 2, 2);
	ASSERT(chctx_reg != NULL);

	/* try to add client3 */
	client1_id = wlc_chctx_reg_add_client(chctx_reg, &test2_client1_fns, CLNT1_OBJECT);
	printf("wlc_chctx_reg_add_client(%p, %d): cid=%d\n", &test2_client1_fns, CLNT1_OBJECT,
	       client1_id);
	ck_assert_int_eq(client1_id, 0);

	/* try add client 1 cubby 1 */
	client1_cubby1_id = wlc_chctx_reg_add_cubby(chctx_reg, client1_id,
		NULL, TEST2_CLNT1_CUBBY_CTXSZ, CLNT1_CUBBY1_OBJECT);
	printf("wlc_chctx_reg_add_cubby(%d, %p, %d): cid=%d\n",
	       client1_id, TEST2_CLNT1_CUBBY_CTXSZ, CLNT1_CUBBY1_OBJECT,
	       client1_cubby1_id);
	ck_assert_int_eq(client1_cubby1_id, 0);

	/* try add client 1 cubby 2 */
	client1_cubby2_id = wlc_chctx_reg_add_cubby(chctx_reg, client1_id,
		NULL, TEST2_CLNT1_CUBBY_CTXSZ, CLNT1_CUBBY2_OBJECT);
	printf("wlc_chctx_reg_add_cubby(%d, %p, %d): cid=%d\n",
	       client1_id, TEST2_CLNT1_CUBBY_CTXSZ, CLNT1_CUBBY2_OBJECT,
	       client1_cubby2_id);
	ck_assert_int_eq(client1_cubby2_id, 1);

	/* try add channel 1 storage */
	chanspec = CH20MHZ_CHSPEC(1);
	ctl1 = wlc_chctx_reg_add_entry(chctx_reg, chanspec, chan1_buf, sizeof(chan1_buf));
	printf("wlc_chctx_reg_add_entry(%d, %p, %d): ctl=%d\n",
	       1, chan1_buf, sizeof(chan1_buf), ctl1);
	ck_assert_int_eq(ctl1, 0);

	/* try add channel 3 storage */
	chanspec = CH20MHZ_CHSPEC(3);
	ctl2 = wlc_chctx_reg_add_entry(chctx_reg, chanspec, chan3_buf, sizeof(chan3_buf));
	printf("wlc_chctx_reg_add_entry(%d, %p, %d): ctl=%d\n",
	       3, chan3_buf, sizeof(chan3_buf), ctl2);
	ck_assert_int_eq(ctl2, 1);

	/* negative: try add another channel storage */
	chanspec = CH20MHZ_CHSPEC(2);
	ctl3 = wlc_chctx_reg_add_entry(chctx_reg, chanspec, chan2_buf, sizeof(chan2_buf));
	printf("[negative] wlc_chctx_reg_add_entry(%d, %p, %d): ctl=%d\n",
	       2, chan2_buf, sizeof(chan2_buf), ctl3);
	ck_assert_int_eq(ctl3, BCME_NORESOURCE);

	/* signal context open for channel 1 */
	notif.event = WLC_CHCTX_OPEN_CHAN;
	notif.chanspec = CH20MHZ_CHSPEC(1);
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("wlc_chctx_reg_notif(%d, %d): ret=%d\n", WLC_CHCTX_OPEN_CHAN, 1, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* signal context open for channel 3 */
	notif.event = WLC_CHCTX_OPEN_CHAN;
	notif.chanspec = CH20MHZ_CHSPEC(3);
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("wlc_chctx_reg_notif(%d, %d): ret=%d\n", WLC_CHCTX_OPEN_CHAN, 3, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* query storage location for channel 1 */
	loc = wlc_chctx_reg_query_cubby(chctx_reg, client1_cubby2_id, CH20MHZ_CHSPEC(1));
	printf("wlc_chctx_reg_query_cubby(%d, %x): loc=%p\n",
	       client1_cubby2_id, CH20MHZ_CHSPEC(1), loc);
	ck_assert_int_eq(loc, chan1_buf + TEST2_CLNT1_CUBBY_CTXSZ);

	/* signal context switch - make channel 1 the current context */
	cur_chspec = CH20MHZ_CHSPEC(1);
	notif.event = WLC_CHCTX_ENTER_CHAN;
	notif.chanspec = cur_chspec;
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("wlc_chctx_reg_notif(%d, %d): ret=%d\n", WLC_CHCTX_ENTER_CHAN, 1, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* signal context switch - make channel 3 the current context */
	cur_chspec = CH20MHZ_CHSPEC(3);
	notif.event = WLC_CHCTX_ENTER_CHAN;
	notif.chanspec = cur_chspec;
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("wlc_chctx_reg_notif(%d, %d): ret=%d\n", WLC_CHCTX_ENTER_CHAN, 3, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* dump */
	output_chctx_reg_info(chctx_reg);

	/* signal context close for channel 1 */
	notif.event = WLC_CHCTX_CLOSE_CHAN;
	notif.chanspec = CH20MHZ_CHSPEC(1);
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("wlc_chctx_reg_notif(%d, %d): ret=%d\n", WLC_CHCTX_CLOSE_CHAN, 1, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* signal context close for channel 3 */
	notif.event = WLC_CHCTX_CLOSE_CHAN;
	notif.chanspec = CH20MHZ_CHSPEC(3);
	ret = wlc_chctx_reg_notif(chctx_reg, &notif);
	printf("wlc_chctx_reg_notif(%d, %d): ret=%d\n", WLC_CHCTX_CLOSE_CHAN, 3, ret);
	ck_assert_int_eq(ret, BCME_OK);

	wlc_chctx_reg_del_entry(chctx_reg, ctl1);
	printf("wlc_chctx_reg_del_entry(%d)\n", ctl1);
	wlc_chctx_reg_del_entry(chctx_reg, ctl2);
	printf("wlc_chctx_reg_del_entry(%d)\n", ctl2);

	/* dump */
	output_chctx_reg_info(chctx_reg);

	wlc_chctx_reg_detach(chctx_reg);
	printf("wlc_chctx_reg_detach(%p)\n", chctx_reg);
}
END_TEST

/*
 * Suite of test cases which check the Multiple Channel Scheduler for each given set of inputs.
 */
Suite *wlc_chctx_reg_suite(void)
{
	/* Suite creation */
	Suite *s = suite_create("wlc_chctx_reg_run");

	/* Test case creation */
	TCase *tc = tcase_create("Test Case");

	tcase_add_checked_fixture(tc, setup, teardown);

	/* Adding unit tests to test case */
	tcase_add_test(tc, test_1);
	tcase_add_test(tc, test_2);

	/* Adding test case to the Suite */
	suite_add_tcase(s, tc);

	return s;
}
