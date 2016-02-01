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
#ifndef BKNI_MULTI_H__
#define BKNI_MULTI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*=*************************************************************************
The multi-tasking kernel interface can only be used by multi-tasking
modules. In the Magnum architecture, this can only be SysLib modules.

See bkni.h for a kernel interface overview.
****************************************************************************/

/***************************************************************************
Summary:
	BKNI_INFINITE can be passed as a timeout value into BKNI_WaitForEvent
	for multi-threaded modules.
****************************************************************************/
#define BKNI_INFINITE -1

/***************************************************************************
Summary:
	Mutex object initialized by BKNI_CreateMutex.
****************************************************************************/
typedef struct BKNI_MutexObj *BKNI_MutexHandle;

/***************************************************************************
Summary:
	Create a mutex in an unacquired state.

Description:
	The mutex is returned in an unacquired state, which means that the first call
	to BKNI_AcquireMutex or BKNI_TryAcquireMutex for that mutex is guaranteed to succeed immediately.

	Note that there is no name parameter in BKNI_CreateMutex. We do not support named
	mutexes because they are essentially global variables that can lead to deadlock.
	Passing in a name for debugging purposes might lead to someone to think we
	support named mutexes.

Returns:
	BERR_SUCCESS if the mutex is created.
	BERR_OS_ERROR if mutex was not created.
****************************************************************************/
BERR_Code BKNI_CreateMutex(BKNI_MutexHandle *mutex);


/***************************************************************************
Summary:
	Destroy a mutex.

Description:
	Destroying a mutex in an acquired state is not allowed and leads to undefined behavior.
****************************************************************************/
void BKNI_DestroyMutex(BKNI_MutexHandle mutex);


/***************************************************************************
Summary:
	Try acquiring exclusive ownership of a mutex, but do not block.

Description:
	If no other task currently owns the mutex, BKNI_TryAcquireMutex will return
	with BERR_SUCCESS and the caller retains exclusive ownership until
	BKNI_ReleaseMutex is called for that mutex.

	If another task currently owns the mutex, BKNI_TryAcquireMutex will not block
	but will return immediately with BERR_TIMEOUT.

	Magnum code cannot nest calls to BKNI_TryAcquireMutex or BKNI_AcquireMutex.
	If a mutex has already been acquired in a task, that same task cannot acquire it
	a second time. It leads to undefined behavior. Some possible results include:

	* Deadlock. The function will never return and the task is hung forever.
	* System halts.
	* Error returned and mutex is only acquired once. If the error code is not handled (which is a violation of Magnum rules), race conditions will be introduced.
	* No error returned and mutex is acquired twice (two releases are required). This may allow bad code to be hidden until another platform uncovers the problem.

	A platform's implementation may chose any of the above results, although we strongly
	recommend that the function fail and return an error code if possible.

Returns:
	BERR_SUCCESS - Mutex was acquired successfully.
	BERR_TIMEOUT - The mutex is already acquired by another task and so the mutex was not acquired.
	BERR_OS_ERROR - The system failed and the mutex was not acquired.

See Also:
	BKNI_AcquireMutex, BKNI_ReleaseMutex, Magnum ThreadSafe rules
****************************************************************************/
BERR_Code BKNI_TryAcquireMutex(
	BKNI_MutexHandle mutex
	);


/***************************************************************************
Summary:
	Acquire exclusive ownership of a mutex, possibly blocking.

Description:
	Acquire exclusive ownership of a mutex. If another task currently
	owns the mutex, BKNI_AcquireMutex will block until the mutex can be acquired.
	After acquiring the mutex, the caller retains exclusive ownership until
	BKNI_ReleaseMutex is called for that mutex.

	Magnum code cannot nest calls to BKNI_AcquireMutex or BKNI_TryAcquireMutex.
	If a mutex has already been acquired in a task, that same task cannot acquire it
	a second time. It leads to undefined behavior. Some possible results include:

	* Deadlock. The function will never return and the task is hung forever.
	* System halts.
	* Error returned and mutex is only acquired once. If the error code is not handled (which is a violation of Magnum rules), race conditions will be introduced.
	* No error returned and mutex is acquired twice (two releases are required). This may allow bad code to be hidden until another platform uncovers the problem.

	A platform's implementation may chose any of the above results, although we strongly
	recommend that the function fail and return an error code if possible.

Returns:
	BERR_SUCCESS - Mutex was acquired successfully.
	BERR_OS_ERROR - The system failed or was interrupted, and the mutex was not acquired.

See Also:
	BKNI_TryAcquireMutex, BKNI_ReleaseMutex, Magnum ThreadSafe rules
****************************************************************************/
BERR_Code BKNI_AcquireMutex(
	BKNI_MutexHandle mutex
	);

/***************************************************************************
Summary:
	Release exclusive ownership of a mutex.

Description:
	If you successfully acquired the mutex using BKNI_AcquireMutex or BKNI_TryAcquireMutex,
	BKNI_ReleaseMutex will release the mutex.

	In Magnum code, releasing a mutex which was acquired in a different task is not
	allowed	and leads to undefined behavior.
	Also, releasing a mutex which is not in an acquired state is not allowed
	and leads to undefined behavior.

	A platform implementation does not have to enforce the usage rules noted
	above. This is because it may not be possible in all cases, and the platform
	may want to actually allow this behavior in platform-specific (non-Magnum) code.

See Also:
	BKNI_AcquireMutex, BKNI_TryAcquireMutex, Magnum ThreadSafe rules
****************************************************************************/
void BKNI_ReleaseMutex(BKNI_MutexHandle mutex);

#ifdef __cplusplus
}
#endif

#endif /* BKNI_MULTI_H__ */
