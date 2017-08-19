/*
 * Shell command implementation for WinCE
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#define NEED_IR_TYPES
#define   _WIN32_WINNT   0x0600

#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winioctl.h>
#include <malloc.h>
#include <assert.h>
#include <ntddndis.h>

#include <typedefs.h>
#include <wlioctl.h>
#include <epictrl.h>
#include <irelay.h>
#include <proto/ethernet.h>
#include <nuiouser.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmcdc.h>
#include <proto/802.11.h>
#include "wlu_remote.h"

__inline wchar_t *wmemset(wchar_t *_S, wchar_t _C, size_t _N)
	{wchar_t *_Su = _S;
	for (; 0 < _N; ++_Su, --_N)
		*_Su = _C;
	return (_S); }

#define stricmp _stricmp
#define strnicmp _strnicmp
#define strrev _strrev
#define FILE_OPEN_RETRY			25
#define MAX_ARGS_LENGTH          4096

#define MAX_FILE_NAME_INDEX      2
#define MAX_FILE_NAME_SIZE	         30

#define SHELL_FILE_PATH			 "C:\\RWL\\tmp1x2x.txt"
#define WC_SHELL_RESP_PATH		 "C:\\RWL"
#define WC_SHELL_FILE_PATH		 "C:\\RWL\\tmp1x2x.txt"
HANDLE g_hshell_partialThread;
HANDLE g_hrecvCtrlcEvent;
/* Function prototypes */
static int g_errno = 0;
static int remote_shell_sync_exec(char *cmd_buf_ptr);
static int rwl_get_file_size(char *buf_ptr);

/* Global variables */

static char shell_fname[MAX_FILE_NAME_SIZE];

/* Open the command processor and execute the shell command
 * passed in the buf_ptr
 */
void
rwl_shell_exe(LPVOID buf_ptr)
{
	DWORD exitcode;
	char Args[MAX_ARGS_LENGTH];
	TCHAR pDefaultCMD[100];
	PROCESS_INFORMATION ProcessInfo;
	STARTUPINFO StartupInfo;
	HANDLE hkill_job;
	Args[0] = 0;
	g_errno = 0;
	memset(Args, 0, MAX_ARGS_LENGTH);
	strcat((char*)buf_ptr, " >>");
	strcat((char*)buf_ptr, SHELL_FILE_PATH);
	strcat((char*)buf_ptr, " 2>&1");
	sprintf(Args, "%s", (const char *)buf_ptr);
	memset(&ProcessInfo, 0, sizeof(ProcessInfo));
	memset(&StartupInfo, 0, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(StartupInfo);
	hkill_job = CreateJobObject(0, 0);
	/*  "/C" option - Do the command and EXIT the command processor */
	_tcscpy(pDefaultCMD, _T("cmd.exe /C "));
	_tcscat(pDefaultCMD, Args);

	/* Invoke the command processor and execute the shell command */
	if (!CreateProcess(NULL, (LPTSTR)pDefaultCMD, NULL, NULL, FALSE, FALSE,
		NULL, NULL, &StartupInfo, &ProcessInfo)) {
		DPRINT_DBG(OUTPUT, "rwl_shell_exe:Error in createprocess %d\n", GetLastError());
		g_errno = GetLastError();
		return;
	}
	if (!AssignProcessToJobObject(hkill_job, ProcessInfo.hProcess)) {
		DPRINT_DBG(OUTPUT, "Assigning ProcessToJobObject failed !!! %d", BCME_ERROR);
		g_errno = GetLastError();
		return;
	}
	while (WaitForSingleObject(g_hrecvCtrlcEvent, 0) != WAIT_OBJECT_0) {
			if (!WaitForSingleObject(ProcessInfo.hProcess, 1)) {
				break;
			}
	}
	GetExitCodeProcess(ProcessInfo.hProcess, &exitcode);
	TerminateJobObject(hkill_job, exitcode);
	CloseHandle(hkill_job);
	CloseHandle(ProcessInfo.hProcess);
	DPRINT_DBG(OUTPUT, "rwl_shell_exe:errno=%d\n", GetLastError());
	return;
}


/*
 * Function to get the response of the shell cmd from the file and
 * copies the result in buf_ptr for WinCE OS
 */
int
remote_shell_get_resp(char* shell_fname, void* wl)
{
	FILE *shell_fpt;
	char *buf_ptr[SHELL_RESP_SIZE];
	int msg_len;

	/*
	 * g_errno is global variable which stores create process error if any
	 * from the thread created. Create process error is passed to client
	 */
	if (g_errno != 0) {
		sprintf((char*)buf_ptr, "Error in create process errno = %d\n", g_errno);
		remote_tx_response(wl, buf_ptr, 0);
		g_rem_ptr->msg.cmd = g_errno;
		g_rem_ptr->msg.len = 0;
		/* Transmit the failure so that client can exit */
		remote_tx_response(wl, buf_ptr, 0);
		return FAIL;
	}

	/*
	 * Failed to open the file then inform client
	 */
	if ((shell_fpt = fopen(SHELL_FILE_PATH, "rb")) == NULL) {
		strcpy((char*)buf_ptr, "File open error\n");
		remote_tx_response(wl, buf_ptr, 0);
		g_rem_ptr->msg.cmd = GetLastError();
		g_rem_ptr->msg.len = 0;
		/* Transmit the failure so that client can exit */
		remote_tx_response(wl, buf_ptr, 0);
		return FAIL;
	}

	/* copy the file into the buffer: */
	while (1) {
		memset(buf_ptr, 0, SHELL_RESP_SIZE);
		msg_len  = fread(buf_ptr, sizeof(char), SHELL_RESP_SIZE, shell_fpt);
		g_rem_ptr->msg.len = msg_len;
		if (msg_len > 0) {
			remote_tx_response(wl, buf_ptr, 0);
		}
		/* check if process ended, if so return to client saying we are done */
		if ((WaitForSingleObject(g_hshell_partialThread, 0) == WAIT_OBJECT_0)) {
			if (g_rem_ptr->msg.len == 0) {
				remote_tx_response(wl, buf_ptr, 0);
				break;
			}
		}
		/* Check if client sending ctr+c message */
		if (get_ctrlc_header(wl) >= 0) {
				if (g_rem_ptr->msg.flags == CTRLC_FLAG) {
					SetEvent(g_hrecvCtrlcEvent);
					g_rem_ptr->msg.len = 0;
					g_rem_ptr->msg.cmd = 0;
					remote_tx_response(wl, buf_ptr, 0);
					break;
				}
		}
	}
	/* wait for thread to exit and release file for close and delete */
	while (WaitForSingleObject(g_hshell_partialThread, 0) != WAIT_OBJECT_0);
	fclose(shell_fpt);

	/* DeleteFile() requires Wide-Character String for the File path */
	if (!DeleteFile(WC_SHELL_FILE_PATH)) {
		DPRINT_DBG(OUTPUT, "\n Unable to delete file %d\n", GetLastError());
		return FAIL;
	}

	return SUCCESS;
}

int
rwl_create_dir(void)
{
	/* CreateDirectory() requires the arguement to be in Wide-Character Format */
	if (!CreateDirectory(WC_SHELL_RESP_PATH, NULL)) {
		/* If directory already exists then it is not a failure */
		if (GetLastError() != ERROR_ALREADY_EXISTS) {
			DPRINT_ERR(ERR, "\n Error Creating the Directory = %d\n", GetLastError());
			return FAIL;
		}
	}
	return SUCCESS;
}

/*
 * Main function for shell command execution
 */
int
remote_shell_execute(char *buf_ptr, void *wl)
{
	DWORD RecvPartialThread;
	FILE *file_truncate;
	/* Create a event that will be set when a Ctrl-C is read from client */
	if ((g_hrecvCtrlcEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == 0) {
		DPRINT_DBG(OUTPUT, "\n create event failed = %d\n", GetLastError());
		return GetLastError();
	}

	/*
	 * File is opened in w mode so that if any data exist gets truncated
	 * It is closed in next line to allow createprocess to use it
	 */
	if ((file_truncate = fopen(SHELL_FILE_PATH, "w")) == NULL) {
		strcpy((char*)buf_ptr, "File open error\n");
		remote_tx_response(wl, buf_ptr, 0);
		g_rem_ptr->msg.cmd = GetLastError();
		g_rem_ptr->msg.len = 0;
		/* Transmit the failure so that client can exit */
		remote_tx_response(wl, buf_ptr, 0);
		return FAIL;
	}
	else
		fclose(file_truncate);

	/*
	 * Execute partial output implementation in case of all transports
	 * This thread will execute the shell command and wait for ctrl-c event from
	 * client side
	 */
	g_hshell_partialThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)rwl_shell_exe,
	(LPVOID)buf_ptr, 0, &RecvPartialThread);
	if (g_hshell_partialThread == NULL) {
		DPRINT_DBG(OUTPUT, "\n shell partial not created\n");
		return GetLastError();
	}
	return SUCCESS;
}
