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
 * Module Description: Cross-platform kernel inteface conformance test
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "bkni_test.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bdbg.h"
#include "btst_kni.h"

BDBG_MODULE(kni_test);
#define BUFFER_SIZE 128

#define CHECK(FUNC) \
	do { \
	BKNI_Printf("Calling %s\n", #FUNC); \
	BKNI_TST_TEST(FUNC); \
	} while (0)

#define BKNI_TST_TEST(x)  do { \
       if (!(x)) {BKNI_Printf("### %s failed at %s:%d\n", #x, __FILE__, __LINE__); BKNI_Fail(); } \
    } while (0)

#define CHECK_AND_RETURN(FUNC) \
	do {BERR_Code result = (FUNC); \
		if (result) {BKNI_Printf("### %s failed\n", #FUNC); return result;}\
	} while (0)

BERR_Code BKNI_TestAll(void) {
/* 	CHECK_AND_RETURN(BKNI_TestInit()); */
	CHECK_AND_RETURN(BKNI_TestMemoryFunctions());
/*	CHECK_AND_RETURN(BKNI_TestMemoryFunctions2()); */
	CHECK_AND_RETURN(BKNI_TestPrintFunctions());
	CHECK_AND_RETURN(BKNI_TestMallocAndFree());
	CHECK_AND_RETURN(BKNI_TestSleepAndDelay());
	CHECK_AND_RETURN(BKNI_TestEvents());
	CHECK_AND_RETURN(BKNI_TestEventWithTask());
	CHECK_AND_RETURN(BKNI_TestMutexes());
	CHECK_AND_RETURN(BKNI_TestMutexWithTask());
	return BERR_SUCCESS;
}

/**
* BKNI_Init can only be called successfully once.
* It should already have been called before any tests.
**/
BERR_Code BKNI_TestInit(void)
{
	/* This should FAIL, because it should have already been called. */
    /* This test is not valid because we didn't document such behaviour and shouldn't.
       the specific implementation of the BKNI_Init should use ASSERT if it detected illegal double init 
     */
	BKNI_TST_TEST(BKNI_Init() == BERR_OS_ERROR);
	return BERR_SUCCESS;
}

BERR_Code BKNI_TestMemoryFunctions(void)
{
	unsigned char buf1[BUFFER_SIZE];
	unsigned char buf2[BUFFER_SIZE];
	int i;

	BKNI_Printf("\nMemory Function Test, entering\n");
	BKNI_Printf("  BKNI_Memset() Test\n");
	BKNI_TST_TEST(BKNI_Memset(buf1,0x12,BUFFER_SIZE) == buf1);
	for (i=0;i<BUFFER_SIZE;i++)
		BKNI_TST_TEST(buf1[i] == 0x12);

	BKNI_Printf("  BKNI_Memcpy() Test\n");
	BKNI_TST_TEST(BKNI_Memcpy(buf2,buf1,BUFFER_SIZE) == buf2);
	for (i=0;i<BUFFER_SIZE;i++)
		BKNI_TST_TEST(buf2[i] == 0x12);

	BKNI_Printf("  BKNI_Memcmp() Test\n");
	/* memcmp success */
	BKNI_TST_TEST(BKNI_Memcmp(buf1,buf2,BUFFER_SIZE) == 0);

	buf2[5] = 0x1;
	BKNI_Printf("  BKNI_Memchr() Test\n");
	/* memchr fail */
	BKNI_TST_TEST(BKNI_Memchr(buf2, 0x1, 5) == NULL);
	/* memchr success, on the boundary and beyond */
	BKNI_TST_TEST(BKNI_Memchr(buf2, 0x1, 6) == &buf2[5]);
	BKNI_TST_TEST(BKNI_Memchr(buf2, 0x1, BUFFER_SIZE) == &buf2[5]);

	/* memcmp fail */
	BKNI_TST_TEST(BKNI_Memcmp(buf1,buf2,BUFFER_SIZE) != 0);

	BKNI_Printf("  BKNI_Memmove() Test, non-overlapping\n");
	/* non-overlapping move operation */
	BKNI_TST_TEST(BKNI_Memset(buf2,0xFE,BUFFER_SIZE) == buf2);
	BKNI_TST_TEST(BKNI_Memmove(buf1, buf2, BUFFER_SIZE) == buf1);
	BKNI_TST_TEST(BKNI_Memcmp(buf1,buf2,BUFFER_SIZE) == 0);

	BKNI_Printf("  BKNI_Memmove() Test, overlapping\n");
	/* overlapping move operation to the left */
	for (i=0;i<BUFFER_SIZE;i++)
		buf2[i] = i;
	BKNI_TST_TEST(BKNI_Memmove(buf2, &buf2[10], BUFFER_SIZE-10) == buf2);
	for (i=0;i<BUFFER_SIZE-10;i++)
		BKNI_TST_TEST(buf2[i] == i+10);
	for (i=BUFFER_SIZE-10;i<BUFFER_SIZE;i++)
		BKNI_TST_TEST(buf2[i] == i);

	/* overlapping move operation to the right */
	for (i=0;i<BUFFER_SIZE;i++)
		buf2[i] = i;
	BKNI_TST_TEST(BKNI_Memmove(&buf2[10], buf2, BUFFER_SIZE-10) == &buf2[10]);
	for (i=0;i<10;i++)
		BKNI_TST_TEST(buf2[i] == i);
	for (i=BUFFER_SIZE-10;i<BUFFER_SIZE;i++)
		BKNI_TST_TEST(buf2[i] == i-10);
	BKNI_Printf("Memory Function Test, leaving\n");
	return BERR_SUCCESS;
}

static int _strlen(const char *s) {
	int i=0;
	while (*s++) i++;
	return i;
}

static int _strcmp(const char *s1, const char *s2) {
	while (*s1 && *s2) {
		if (*s1 != *s2)
			return *s2>*s1?1:-1;
		s1++;
		s2++;
	}
	return 0;
}

int TestVPrintf(const char *fmt, ...)
{
	va_list ap;
	int actual;

	va_start(ap, fmt);
	actual = BKNI_Vprintf(fmt, ap);
	va_end(ap);
	return actual;
}

int TestVsnprintf(char *buf, int n, const char *fmt, ...)
{
	va_list ap;
	int actual;

	va_start(ap, fmt);
	actual = BKNI_Vsnprintf(buf, n, fmt, ap);
	va_end(ap);
	return actual;
}

BERR_Code BKNI_TestPrintFunctions(void)
{
	int d = 1;
	long ld = 2;
	unsigned int u = 3;
	unsigned long lu = 4;
	unsigned int x = 0xfe;
	unsigned long lx = 0xac;
	char buf[BUFFER_SIZE];

	BKNI_Printf("\nPrint Function Test, entering\n");
	CHECK_AND_RETURN(9 != BKNI_Printf("%d %u %x %s\n", d, u, x, "4"));
	CHECK_AND_RETURN(9 != BKNI_Printf("%ld %lu %lx %s\n", ld, lu, lx, "4"));
	CHECK_AND_RETURN(17 != BKNI_Printf("%04d %04ld %04x %s\n", d, lu, x, "4"));
	CHECK_AND_RETURN(8 != BKNI_Printf("%c %s\n", 'q', "Hello"));

	CHECK_AND_RETURN(9 != TestVPrintf("%d %u %x %s\n", d, u, x, "4"));
	CHECK_AND_RETURN(9 != TestVPrintf("%ld %lu %lx %s\n", ld, lu, lx, "4"));
	CHECK_AND_RETURN(17 != TestVPrintf("%04d %04ld %04x %s\n", d, lu, x, "4"));
	CHECK_AND_RETURN(8 != TestVPrintf("%c %s\n", 'q', "Hello"));

	CHECK_AND_RETURN(9 != BKNI_Snprintf(buf, BUFFER_SIZE, "%d %u %x %s\n", d, u, x, "4"));
	CHECK_AND_RETURN(9 != BKNI_Snprintf(buf, BUFFER_SIZE, "%ld %lu %lx %s\n", ld, lu, lx, "4"));
	CHECK_AND_RETURN(17 != BKNI_Snprintf(buf, BUFFER_SIZE, "%04d %04ld %04x %s\n", d, lu, x, "4"));
	CHECK_AND_RETURN(8 != BKNI_Snprintf(buf, BUFFER_SIZE, "%c %s\n", 'q', "Hello"));

	CHECK_AND_RETURN(9 != TestVsnprintf(buf, BUFFER_SIZE, "%d %u %x %s\n", d, u, x, "4"));
	CHECK_AND_RETURN(9 != TestVsnprintf(buf, BUFFER_SIZE, "%ld %lu %lx %s\n", ld, lu, lx, "4"));
	CHECK_AND_RETURN(17 != TestVsnprintf(buf, BUFFER_SIZE, "%04d %04ld %04x %s\n", d, lu, x, "4"));
	CHECK_AND_RETURN(8 != TestVsnprintf(buf, BUFFER_SIZE, "%c %s\n", 'q', "Hello"));

	BKNI_Printf("Print Function Test, leaving\n");
	return BERR_SUCCESS;
}

/**
* We can't test the actual time expired in a platform-generic way.
**/
BERR_Code BKNI_TestSleepAndDelay(void)
{
	BKNI_Printf("\nSleep and Delay Test, entering\n");
	BKNI_TST_TEST(BKNI_Sleep(0) == BERR_SUCCESS);
	BKNI_TST_TEST(BKNI_Sleep(10) == BERR_SUCCESS);
	BKNI_Delay(0);
	BKNI_Delay(10);
	BKNI_Printf("Sleep and Delay Test, leaving\n");
	return BERR_SUCCESS;
}

BERR_Code BKNI_TestMallocAndFree(void)
{
	unsigned char *temp = (unsigned char *)BKNI_Malloc(10);
    BKNI_TST_TEST(temp);
	/* make sure we can write to all of it */
	BKNI_Memset(temp, 0x1, 10);
	BKNI_Free(temp);

	return BERR_SUCCESS;
}

BERR_Code BKNI_TestEvents(void)
{
	BKNI_EventHandle event;

    /* test unused event */
	BKNI_Printf("\nSingle-Thread Event Test, entering\n");
	BKNI_Printf("  BKNI_Create/DestroyEvent(hEvent) Test\n");
	BKNI_TST_TEST(BKNI_CreateEvent(&event) == BERR_SUCCESS);
	BKNI_DestroyEvent(event);

	BKNI_Printf("  BKNI_CreateEvent(hEvent) Test\n");
	/* create a unsignalled event */
	BKNI_TST_TEST(BKNI_CreateEvent(&event) == BERR_SUCCESS);
	/* no initial event */
	BKNI_TST_TEST(BKNI_WaitForEvent(event, 1000) == BERR_TIMEOUT);

	/* reuse the event */
	/* event is truly consumed */
	BKNI_Printf("  One BKNI_SetEvent(hEvent), multiple BKNI_WaitForEvent(hEvent,1000) Test\n");
	BKNI_SetEvent(event);
	BKNI_TST_TEST(BKNI_WaitForEvent(event, 1000) == BERR_SUCCESS);
	BKNI_TST_TEST(BKNI_WaitForEvent(event, 1000) == BERR_TIMEOUT);

	/* WaitForEvent consumes all events */
	BKNI_Printf("  Multiple BKNI_SetEvent(hEvent), one BKNI_WaitForEvent(hEvent,1000) Test\n");
	BKNI_SetEvent(event);
	BKNI_SetEvent(event);
	BKNI_SetEvent(event);
	BKNI_SetEvent(event);
	BKNI_TST_TEST(BKNI_WaitForEvent(event, 1000) == BERR_SUCCESS);
	BKNI_TST_TEST(BKNI_WaitForEvent(event, 1000) == BERR_TIMEOUT);

	/* event is truly consumed */
	BKNI_Printf("  One BKNI_SetEvent(hEvent), ResetEvent Test\n");
	BKNI_SetEvent(event);
	BKNI_ResetEvent(event);
	BKNI_TST_TEST(BKNI_WaitForEvent(event, 1000) == BERR_TIMEOUT);

	BKNI_Printf("  Multiple BKNI_SetEvent(hEvent), ResetEvent Test\n");
	BKNI_SetEvent(event);
	BKNI_SetEvent(event);
	BKNI_SetEvent(event);
	BKNI_SetEvent(event);
	BKNI_ResetEvent(event);
	BKNI_TST_TEST(BKNI_WaitForEvent(event, 1000) == BERR_TIMEOUT);

	BKNI_DestroyEvent(event);

	BKNI_Printf("Single-Thread Event Test, leaving\n");
	return BERR_SUCCESS;
}


/**
* Create a task to set an event immediately. Wait for that event.
**/
static void BKNI_TestEventWithTask_Thread(BTST_TaskHandle task, void *data)
{
	static int t = 0;

	BKNI_TST_TEST(BKNI_Sleep((t++ % 10) * 10) == BERR_SUCCESS);
	BKNI_Printf("  hTask, BKNI_SetEvent(hEvent)\n");
	BKNI_SetEvent((BKNI_EventHandle)data);
	return;
}

BERR_Code BKNI_TestEventWithTask(void)
{
	BTST_TaskHandle task;
	BKNI_EventHandle event;
	int i;

	BKNI_Printf("\nMulti-Thread Event Test, entering\n");
	/* repeat this with varying sleeps, looking for race conditions */
	for (i=0;i<100;i++) {
		BKNI_TST_TEST(BKNI_CreateEvent(&event) == BERR_SUCCESS);
		if(i%20== 0) BKNI_Printf("  mainTask, BTST_CreateTask(hTask)\n");
		BKNI_TST_TEST(BTST_CreateTask(&task, BKNI_TestEventWithTask_Thread,
			(void*)event) == BERR_SUCCESS);

		/* BKNI_Test8b_Thread may or may not have set the event already. */
		if(i%20== 0) BKNI_Printf("  mainTask, BKNI_WaitForEvent(hEvent,BKNI_INFINITE)\n");
		BKNI_TST_TEST(BKNI_WaitForEvent(event, BKNI_INFINITE) == BERR_SUCCESS);
		BKNI_Printf("  mainTask, Got hEvent\n");
		if(i%20== 0) BKNI_DestroyEvent(event);

		if(i%20== 0) BKNI_Printf("  mainTask, BTST_DestroyTask(hTask)\n");
		BKNI_TST_TEST(BTST_DestroyTask(task) == BERR_SUCCESS);
	}
	BKNI_Printf("Multi-Thread Event Test, leaving\n");
	return BERR_SUCCESS;
}

BERR_Code BKNI_TestMutexes(void)
{
	int i;
	BKNI_MutexHandle mutex;

	BKNI_Printf("\nSingle-Thread Mutex Test, entering\n");
	/* unused */
	BKNI_Printf("  BKNI_Create/DestroyMutex(hMutex) Test\n");
	BKNI_TST_TEST(BKNI_CreateMutex(&mutex) == BERR_SUCCESS);
	BKNI_DestroyMutex(mutex);

	/* normal use */
	BKNI_TST_TEST(BKNI_CreateMutex(&mutex) == BERR_SUCCESS);

    /* non blocking acquire */
	BKNI_Printf("  BKNI_AcquireMutex(hMutex) Test, non-blocking\n");
	for (i=0;i<100;i++) {
	    BKNI_TST_TEST(BKNI_TryAcquireMutex(mutex) == BERR_SUCCESS);
		BKNI_ReleaseMutex(mutex);
	}

    /* blocking acquire */
	BKNI_Printf("  BKNI_AcquireMutex(hMutex) Test, blocking\n");
	for (i=0;i<100;i++) {
	    BKNI_TST_TEST(BKNI_AcquireMutex(mutex) == BERR_SUCCESS);
		BKNI_ReleaseMutex(mutex);
	}

	BKNI_DestroyMutex(mutex);

	BKNI_Printf("Single-Thread Mutex Test, leaving\n");
	return BERR_SUCCESS;
}

static int g_TestMutexWithTask_flag = 0;
static void BKNI_TestMutexWithTask_Thread(BTST_TaskHandle task, void *data)
{

	/* The main task should be sleeping, which means the flag should still be 0. */
	BKNI_TST_TEST(g_TestMutexWithTask_flag == 0);

	/* We should be able to TryAcquire and Acquire while the main task sleeps. */
	BKNI_Printf("  hTask, BKNI_TryAcquireMutex(hMutex)\n");
	BKNI_TST_TEST(BKNI_TryAcquireMutex((BKNI_MutexHandle)data) == BERR_SUCCESS);
	BKNI_ReleaseMutex((BKNI_MutexHandle)data);

	BKNI_Printf("  hTask, BKNI_AcquireMutex(hMutex)\n");
	BKNI_TST_TEST(BKNI_AcquireMutex((BKNI_MutexHandle)data) == BERR_SUCCESS);

	/* Now wait a sufficient amount of time for the main task to wake up. */
	BKNI_Printf("  hTask, hMutex acquired\n");
	g_TestMutexWithTask_flag = 1;
	BKNI_TST_TEST(BKNI_Sleep(5 * 1000) == BERR_SUCCESS);

	BKNI_TST_TEST(g_TestMutexWithTask_flag == 2);
	g_TestMutexWithTask_flag = 3;
	BKNI_Printf("  hTask, BKNI_ReleaseMutex(hMutex)\n");
	BKNI_ReleaseMutex((BKNI_MutexHandle)data);

	/* The main task should have acquired it as soon as we released. */
/*
This isn't working on linux. I suppose this test isn't valid. We can't guarantee
who runs next.
	BKNI_TST_TEST(BKNI_TryAcquireMutex((BKNI_MutexHandle)data) == BERR_TIMEOUT);
*/
	return;
}

BERR_Code BKNI_TestMutexWithTask(void)
{
	BKNI_MutexHandle mutex;
	BTST_TaskHandle task;

	BKNI_Printf("\nMulti-Thread Mutex Test, entering\n");
	BKNI_TST_TEST(BKNI_CreateMutex(&mutex) == BERR_SUCCESS);

	g_TestMutexWithTask_flag = 0;
	BKNI_Printf("  mainTask, BTST_CreateTask(hTask)\n");
	BKNI_TST_TEST(BTST_CreateTask(&task, BKNI_TestMutexWithTask_Thread,
		(void*)mutex) == BERR_SUCCESS);

	/* This sleep causes the task to acquire the mutex then sleep. */
	BKNI_TST_TEST(BKNI_Sleep(2 * 1000) == BERR_SUCCESS);

	/* The task should have the mutex and should be sleeping. Our 2 second sleep
	should be over before its 5 second sleep. */
	BKNI_TST_TEST(g_TestMutexWithTask_flag == 1);

	/* This Acquire should timeout  */
	BKNI_TST_TEST(BKNI_TryAcquireMutex(mutex) == BERR_TIMEOUT);

	g_TestMutexWithTask_flag = 2;

	/* This Acquire should block until the task releases the mutex. */
	BKNI_Printf("  mainTask, BKNI_AcquireMutex(hMutex)\n");
	BKNI_TST_TEST(BKNI_AcquireMutex(mutex) == BERR_SUCCESS);
	BKNI_Printf("  mainTask, hMutex acquired\n");

	/* Task should have released it. */
	BKNI_TST_TEST(g_TestMutexWithTask_flag == 3);

	/* Sleep just a little to prove that this task executed first.
	This should cause the TryAcquire to fail. */
	BKNI_TST_TEST(BKNI_Sleep(10) == BERR_SUCCESS);

	BKNI_Printf("  mainTask, BKNI_ReleaseMutex(hMutex)\n");
	BKNI_ReleaseMutex(mutex);

	/* Clean up. */
	BKNI_Printf("  mainTask, BKNI_DestoryTask(hTask)\n");
	BKNI_TST_TEST(BTST_DestroyTask(task) == BERR_SUCCESS);
	BKNI_DestroyMutex(mutex);

	BKNI_Printf("Multi-Thread Mutex Test, leaving\n");
	return BERR_SUCCESS;
}


static unsigned char arrayA[100];
static unsigned char arrayB[50];
#define TEST_SZ 50
#define TEST_PATTERN1 0xA5
#define TEST_PATTERN2 0xBA
#define TEST_PATTERN3 0xBE
BERR_Code BKNI_TestMemoryFunctions2(void)
{
    int idx;
	BERR_Code retVal = BERR_OS_ERROR;


	BKNI_Printf("\nMemory Function Test2, entering\n");
    /* Test BKNI_Memset() */
	BKNI_Printf("  BKNI_Memset() Test\n");
    BKNI_Memset((void *) arrayA, TEST_PATTERN1, TEST_SZ);
    for( idx = 0; idx < TEST_SZ; idx++ )
    {
        if( arrayA[idx] != TEST_PATTERN1 )
        {
            BDBG_ERR(("    Failed BKNI_Memset() test 1"));
            goto end;
        }
    }
    for( idx = TEST_SZ; idx < sizeof(arrayA); idx++ )
    {
        if( arrayA[idx] == TEST_PATTERN1 )
        {
            BDBG_ERR(("    Failed BKNI_Memset() test 2"));
            goto end;
        }
    }
    /* Test BKNI_Memcpy() */
	BKNI_Printf("  BKNI_Memcpy() Test\n");
    BKNI_Memcpy((void *) arrayB, (const void *) arrayA, (TEST_SZ-10));
    for( idx = 0; idx < (TEST_SZ-10); idx++ )
    {
        if( arrayB[idx] != TEST_PATTERN1 )
        {
            BDBG_ERR(("    Failed BKNI_Memcpy() test 1"));
            goto end;
        }
    }
    for( idx = (TEST_SZ-10); idx < sizeof(arrayB); idx++ )
    {
        if( arrayB[idx] == TEST_PATTERN1 )
        {
            BDBG_ERR(("    Failed BKNI_Memcpy() test 2"));
            goto end;
        }
    }
    /* Test BKNI_Memcmp() */
	BKNI_Printf("  BKNI_Memcmp() Test\n");
    if( BKNI_Memcmp((const void *) arrayA, (const void *) arrayB, (TEST_SZ-10)) != 0 )
    {
        BDBG_ERR(("    Failed BKNI_Memcpy() test 1"));
        goto end;
    }
    if( BKNI_Memcmp((const void *) &arrayA[TEST_SZ-10], (const void *) &arrayB[TEST_SZ-10], (sizeof(arrayB) - (TEST_SZ-10))) <= 0 )
    {
        BDBG_ERR(("    Failed BKNI_Memcpy() test 2"));
        goto end;
    }
    if( BKNI_Memcmp((const void *) &arrayB[TEST_SZ-10], (const void *) &arrayA[TEST_SZ-10], (sizeof(arrayB) - (TEST_SZ-10))) >= 0 )
    {
        BDBG_ERR(("    Failed BKNI_Memcpy() test 3"));
        goto end;
    }
    /* Test BKNI_Memchr() */
	BKNI_Printf("  BKNI_Memchr() Test\n");
    arrayA[25] = TEST_PATTERN2;
    arrayA[30] = TEST_PATTERN3;
    if( BKNI_Memchr((const void *) arrayA, TEST_PATTERN2, sizeof(arrayA)) != &arrayA[25] )
    {
        BDBG_ERR(("    Failed BKNI_Memchr() test 1"));
        goto end;
    }
    if( BKNI_Memchr((const void *) arrayA, TEST_PATTERN3, sizeof(arrayA)) != &arrayA[30] )
    {
        BDBG_ERR(("    Failed BKNI_Memchr() test 2"));
        goto end;
    }
    if( BKNI_Memchr((const void *) arrayA, 0x66, sizeof(arrayA)) != NULL )
    {
        BDBG_ERR(("    Failed BKNI_Memchr() test 3"));
        goto end;
    }
    /* Test BKNI_Memmove() */
    for( idx = 0; idx < 10; idx++ )
    {
        arrayA[idx] = idx;
        arrayB[idx] = idx;
    }
    /* Straight copy */
	BKNI_Printf("  BKNI_Memmove() Test, non-overlapping\n");
    BKNI_Memmove((void *) &arrayA[30], (const void *) &arrayA[0], 10);
    if( BKNI_Memcmp((const void *) &arrayA[30], (const void *) arrayB, 10) != 0 )
    {
        BDBG_ERR(("    Failed BKNI_Memmove() test 1"));
        goto end;
    }
    /* Overlapping copy */
	BKNI_Printf("  BKNI_Memmove() Test, overlapping\n");
    BKNI_Memmove((void *) &arrayA[3], (const void *) &arrayA[0], 10);
    if( BKNI_Memcmp((const void *) &arrayA[3], (const void *) arrayB, 10) != 0 )
    {
        BDBG_ERR(("    Failed BKNI_Memmove() test 2"));
        goto end;
    }
    if( arrayA[0] != 0x00 || arrayA[1] != 0x01 || arrayA[2] != 0x02 )
    {
        BDBG_ERR(("    Failed BKNI_Memmove() test 3"));
        goto end;
    }
	retVal = BERR_SUCCESS;
end:
	BKNI_Printf("Memory Function Test2, leaving\n");
	return(retVal);
}
