#include <Windows.h>
#include <stdio.h>
#include <TChar.h>
#include "getopt.h"

static void
Usage(char *name)
	{
	fprintf(stderr, "Usage: %s -p <codepage> [ -i ] <command line>\n", name);
	exit(1);
	}

int main( int argc, char** argv )
	{
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInfo;
	LPTHREAD_START_ROUTINE setCPGlobal;
	HANDLE remoteThread;
	DWORD 		exitCode;
	BOOL		invertRC = FALSE;
	long		codepage = 0;
	int			opt;
	char		*cmd;

	while ((opt = getopt(argc, argv, "?hip:")) != EOF)
		{
		switch (opt)
			{
			case 'p':
				codepage = atoi(optarg);
				break;
			case 'c':
				cmd = optarg;
				break;
			case 'i':
				invertRC = TRUE;
				break;
			default:
				Usage(argv[0]);
			}
		}

	if ((optind >= argc) || (codepage == 0))
		Usage(argv[0]);

	cmd = argv[optind];


	startupInfo.cb = sizeof (startupInfo);
	GetStartupInfo (&startupInfo);
	if( !CreateProcess (NULL, cmd, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL,
						&startupInfo, &processInfo))
		return 2;
	
	setCPGlobal = (LPTHREAD_START_ROUTINE)GetProcAddress( GetModuleHandle(TEXT("Kernel32.dll")), TEXT("SetCPGlobal") );
	if( NULL == setCPGlobal )
		return 3;

	remoteThread = CreateRemoteThread( processInfo.hProcess, NULL, 0, setCPGlobal, (LPVOID)codepage, 0, NULL );
	if( NULL == remoteThread )
		return 4;

	ResumeThread( processInfo.hThread );

	WaitForSingleObject( processInfo.hProcess, INFINITE );

	if( !GetExitCodeProcess( processInfo.hProcess, &exitCode ) )
		return 5;

	CloseHandle( remoteThread );
	CloseHandle( processInfo.hThread );
	CloseHandle( processInfo.hProcess );

	if (invertRC)
		{
		/* Help compiler seems to return 1 on success. */
		if( 1 == exitCode )
			return 0;
		else
			return 6;
		}
	else
		{
		return exitCode;
		}
	}
