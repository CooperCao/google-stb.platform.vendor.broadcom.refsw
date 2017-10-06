#include "stdafx.h"
#include "utility.h"

/* define the name of the DLL and the Dll entry points */
#define BRCMWLU_DLL				 "brcm_wlu.dll"
#define BRCMWLU_LIB_PREP_FUNC    "wl_lib_prep"
#define BRCMWLU_LIB_FUNC         "wl_lib"

/* handles for read/write operation */
HANDLE wlThreadPipe_in[2] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
HANDLE wlThreadPipe_out[2] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};;

HINSTANCE loadWlDll() {
	HINSTANCE dllHndl = LoadLibrary(TEXT(BRCMWLU_DLL));
	
	if (dllHndl == NULL) {
		DBGMSG(("Error loading %s\n",BRCMWLU_DLL));
		return NULL;
	}

	if ((wlLibPrep = (wlLibPrep_t) GetProcAddress (dllHndl, WIDE_CHAR(BRCMWLU_LIB_PREP_FUNC))) == NULL) {
		DBGMSG(("Function %s address is invalid.\n", BRCMWLU_LIB_PREP_FUNC));
		FreeLibrary(dllHndl);
		return NULL;
	}
	
	if ((wlLib = (wlLib_t) GetProcAddress (dllHndl, WIDE_CHAR(BRCMWLU_LIB_FUNC))) == NULL) {
		DBGMSG(("Function %s address is invalid.\n", BRCMWLU_LIB_FUNC));
		FreeLibrary(dllHndl);
		return NULL;
	}
	return dllHndl;
}

void stopWlDll(HINSTANCE dllHndl) {
	if (dllHndl) {
		FreeLibrary(dllHndl);
	}
}

int write (const char *cmdstr) {
		int numToWrite = 0, numWritten = 0, totalWritten = 0;
		int cmdstrLen =(int) strlen(cmdstr); /* No of characters pending to be written */

		if (!cmdstr) {
			DBGMSG(("%s : cmdstr is NULL.\n", __FUNCTION__));
			return -1;
		}

		//This loop only happens more than once if the incoming data is larger than DEFAULT_BUFFER_SIZE
		while (cmdstrLen) {			
			numToWrite = (cmdstrLen > DEFAULT_WRITE_BUFFER_SIZE) ? DEFAULT_WRITE_BUFFER_SIZE : cmdstrLen;
			numWritten = writeBuffer (cmdstr, numToWrite);
			if (numWritten <= 0) {
				return numWritten;          //error, or pipe closed
			}
			totalWritten += numWritten;
			cmdstr += numWritten; /* Increment pointer */
			cmdstrLen -= numWritten; /* Decrement no of pending characters */
		}
		return totalWritten;
}

int read (char *data) {
	int numRead = 0;
	char err_str[] = "error:";
	char wl_err_str[] = "wl: error -";
	int err_str_len =(int)strlen(err_str);
	data[0] = '\0';
	if (!data) {
		DBGMSG(("%s : data pointer is NULL.\n", __FUNCTION__));
		return -1;
	}
	numRead = readBuffer(data);
	if (numRead <= 0) {
		DBGMSG(("%s : Error - readBuffer returned %d\n",__FUNCTION__, numRead));
		return numRead;
	} 
	
	/* add null to end of the buffer */
	data [numRead] = '\0';
	DBGMSG(("Read : %s\n", data));
	
	/* If the string begins with 'error:', we have encountered error
		in execution. */		
	for(int i = 0; i < (int)strlen(data); i++) {
		if ( i == err_str_len) {
			return -1; /* We have encountered the message, 'error:' */
		}
		else if (data[i] != err_str[i]) {
			break;
		}
	}

	/* Sometimes we get 'wl: error -nn' as error string */
	if(strstr(data, wl_err_str))
		return -1;

	return numRead;
}

int read_to_prompt(char *data) {
	int numRead = 0;
	int totalNumRead = 0;
	char newData[MAX_IOCTL_BUFFER] = ""; /* Point to the next location where data to be read */
	data[0] = '\0';

	while ((numRead = read(newData)) > 0 ) {
		totalNumRead += numRead;
					
		if (!strcmp(newData, CMDPROMPT)) {
			return totalNumRead;
		}

		strcat(data, newData);

		if ( strstr(data, "\r\n" CMDPROMPT) || strstr(data, "\n\r" CMDPROMPT) 
			|| strstr(data, "\n" CMDPROMPT)) {
			data[strlen(data) - 2] = '\0';
			return totalNumRead;
		}
		newData[0] = '\0';
	}
	return totalNumRead;
}
bool command(const char *cmd, char *data) {
	data[0] = '\0';
	write(cmd);
	
	if (!read_to_prompt(data)) {
		DBGMSG(("Read zero bytes or Encountered 'error:'. Command failed\n"));
		return false;
	}
	else
		return true;
}

/* 
 * process handling functions 
 */
void* createThread(void *(func)(void*), void *arg) {
		HANDLE h;
		DWORD id;

		h = CreateThread(NULL, 16 * 1024, (LPTHREAD_START_ROUTINE) func,
			(LPVOID) arg, 0, &id);

		return (h);
}

void* closeThread (void *threadid) {
	HANDLE realThread = (HANDLE) threadid;
	//threadid is really a HANDLE
	if (realThread != NULL) {
		CloseHandle (realThread);
	}
	return NULL;
}

void killThread (void *threadid) {
	DWORD exitCode = -1;
	HANDLE realThread = (HANDLE) threadid;
	TerminateThread (realThread, exitCode);
	closeThread(threadid);
}

void wlThread(char *dllStartCmd) {
	setupIPC();
	
	wlLib(dllStartCmd);
					
	deInitIPC();

	if (thread != NULL) {
		closeThread(thread);
		thread = NULL;
	}
}


void* threadStart(void *arg) {
	wlThread((char *)arg);
	thread = NULL;
	return NULL;
}


void threadStop() {
	deInitIPC();
	if (thread != NULL) {
		killThread (thread);
	}
}

void clearFirstPrompt() {
	char data[DEFAULT_READ_BUFFER_SIZE];
	read(data);  /*always eat the first '>' prompt in the pipe */
}

bool wlInit(HINSTANCE dllHndl, char *dllStartCmd) {
	dllHndl = NULL;
	dllHndl = loadWlDll();

	if (dllHndl == NULL) {
		DBGMSG(("%s : loadWlDll failed to return valid handle.\n", __FUNCTION__));
		return false;
	}
	
	if (!initIPC()) {
		stopWlDll(dllHndl);
		return false;
	}
	DBGMSG(("%s : initIPC successful.\n", __FUNCTION__));

	thread = createThread(threadStart, (void*) dllStartCmd);
	
	if (thread == NULL) {
		DBGMSG(("Failed to create a thread.\n"));
		return false;
	}	

	clearFirstPrompt();
	/* run wl ver query to verify if dut is available */
	return true;

}

void exitInteractiveMode() {
	char data[MAX_IOCTL_BUFFER] = "exit";	
	write(data);
	//read(data); /* clear the all pending IO read. */
}

void stopDUT (HINSTANCE dllHndl) {
	exitInteractiveMode();
	threadStop();
	stopWlDll(dllHndl);
}


/* 
 * Pipe handling functions 
 */
bool set_pipe_timeout (HANDLE hdl) 
{
	COMMTIMEOUTS to;
	to.ReadIntervalTimeout = 20;
	to.ReadTotalTimeoutConstant = 1;
	to.ReadTotalTimeoutMultiplier = 1;
	to.WriteTotalTimeoutMultiplier = 20;
	to.WriteTotalTimeoutConstant = 1;
	
	SetCommTimeouts(hdl, &to);

	return true;
}
int pipe_create (HANDLE handles []) {
		SECURITY_ATTRIBUTES sa;

		sa.nLength = sizeof (SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;

		if (!CreatePipe (&handles[READ_HANDLE], &handles[WRITE_HANDLE], &sa, 0)) {
			DBGMSG(("Error creating pipe.\n"));
			handles[READ_HANDLE] = handles[WRITE_HANDLE] = INVALID_HANDLE_VALUE;
			return -1;
		}

		set_pipe_timeout (handles[READ_HANDLE]);
		DBGMSG(("Pipe creation successful.\n"));
		return 0;
}

int pipe_close (HANDLE p) {
	if (p != INVALID_HANDLE_VALUE) {
			if (CloseHandle (p)) {
				return 0;
			}
			DBGMSG(("Error closing pipe.\n"));
			return -1;
		}
		return 0;
}



int pipe_read (HANDLE fd, void *buffer, DWORD count) {
		DWORD numRead;
		
		if(fd == INVALID_HANDLE_VALUE) 
			return -1;

		if (!ReadFile (fd, buffer, count, &numRead, NULL)) {        //non overlapped
			DBGMSG(("%s : read %d bytes\n", __FUNCTION__, numRead));
			return -1 * GetLastError();
		}
		DBGMSG(("%s : read %d bytes\n", __FUNCTION__, numRead));
		return ((int) numRead);
}

int pipe_write (HANDLE fd, const void *buffer, DWORD count) {
		DWORD numWritten;

		if(fd == INVALID_HANDLE_VALUE) 
			return -1;

		if (!WriteFile (fd, buffer, count, &numWritten, NULL)) {
			return -1 * GetLastError();
		}
		return ((int) numWritten);
}

int writeBuffer (const char *buffer, int length) {
	int numWritten;
	int totalWritten = 0;

	/* Keep writting out the content of the buffer until it all gets written to the pipe
	 * Block if the pipe blocks. Error out if the pipe is closed or if there's a pipe error
	 */
	while ((numWritten = pipe_write(wlThreadPipe_in[WRITE_HANDLE], (const char *) &buffer[totalWritten], length - totalWritten)) > 0) {
		totalWritten += numWritten;
		if (totalWritten >= length) {
			break;
		}
	}
	if (numWritten <= 0) {
		return numWritten;          //error, or pipe got closed
	}
	return totalWritten;
}

int readBuffer(char *buffer)
{
	return pipe_read(wlThreadPipe_out[READ_HANDLE], buffer, DEFAULT_READ_BUFFER_SIZE - 1);
}
int initIPC()
{
	wlThreadPipe_in[READ_HANDLE] = INVALID_HANDLE_VALUE;
	wlThreadPipe_in[WRITE_HANDLE] = INVALID_HANDLE_VALUE;
	wlThreadPipe_out[READ_HANDLE] = INVALID_HANDLE_VALUE;
	wlThreadPipe_out[WRITE_HANDLE] = INVALID_HANDLE_VALUE;
	
	if (pipe_create(wlThreadPipe_in) < 0 ) {
			DBGMSG(("Error creating wlThreadPipe_in.\n"));
			return false;
	}

	if (pipe_create(wlThreadPipe_out) < 0 ) {
		DBGMSG(("Error creating wlThread_out pipe.\n"));
		pipe_close(wlThreadPipe_in[READ_HANDLE]);
		pipe_close(wlThreadPipe_out[WRITE_HANDLE]);
		return false;
	}
	return true;
}
void deInitIPC()
{
	pipe_close (wlThreadPipe_in[WRITE_HANDLE]);
	wlThreadPipe_in[WRITE_HANDLE] = INVALID_HANDLE_VALUE;

	pipe_close (wlThreadPipe_out [READ_HANDLE]);
	wlThreadPipe_out[READ_HANDLE] = INVALID_HANDLE_VALUE;

	/* these two handle must have been closed earlier already */
	pipe_close (wlThreadPipe_in[READ_HANDLE]);
	wlThreadPipe_in[READ_HANDLE] = INVALID_HANDLE_VALUE;
		
	pipe_close (wlThreadPipe_out[WRITE_HANDLE]);
	wlThreadPipe_out[WRITE_HANDLE] = INVALID_HANDLE_VALUE;
}

void setupIPC()
{
	wlLibPrep(wlThreadPipe_in[READ_HANDLE], wlThreadPipe_out[WRITE_HANDLE]);
}
