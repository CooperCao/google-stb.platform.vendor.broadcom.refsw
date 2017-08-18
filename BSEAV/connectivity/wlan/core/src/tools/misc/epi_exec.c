/*
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * This file implements a command_line program the runs a subcommand
 * and provides start and finish status. Also provieds command line
 * options for stdin/stdout/stderr redirection to files.
 *
 * $Id$
 */
#if !defined(TARGETOS_Windows_NT)
#error "Only Win32 target supported"
#endif /* TARGETOS_Windows_NT */

#include <stdio.h>
#include <windows.h>

#define ABNORMAL_EXIT 9999

#define PROC_COMBINE_OUTPUT 0x01
#define PROC_APPEND_STDOUT  0x02
#define PROC_APPEND_STDERR  0x04

typedef struct {
	char* name;
	int combine;
	int quiet;
	int pass_exit_code;
	int append_out;
	int append_err;
	char* in;
	char* out;
	char* err;
	char** command_args;
} prog_args_t;

static void
parse_command_line(
	int argc,
	char** argv,
	prog_args_t* prog_args
);

static void check_option_arg(char *prog_name, char *desc, char *opt, int argc);

static void usage(char *prog_name);

typedef HANDLE proc_id_t;

typedef struct {
	HANDLE in;
	HANDLE out;
	HANDLE err;
} io_handles_t;

static DWORD create_io(int flags, char *stdin_filename, char *stdout_filename,
		       char *stderr_filename, io_handles_t* child);

static DWORD create_io_file(char *name, int input, int append, HANDLE *file);

static char *flatten_args(char **args);

static HANDLE swap_std_handle(DWORD num_std_handle, HANDLE new_handle);

static void DebugOutput(const char *fmt, ...);

static char *win_error_string(DWORD err);

static DWORD create_process(char **  command_args, int io_flags, char *stdin_filename,
			    char *stdout_filename, char *stderr_filename, HANDLE *process);

static int wait_process(HANDLE process, int *pexit_code);

#define	warn	DebugOutput

#define DEBUG 2

#if DEBUG
#define DEBUG_ERR(x) DebugOutput x
#else
#define DEBUG_ERR(x)
#endif

#if DEBUG > 1
#define PDEBUG(x) DebugOutput x
#else
#define PDEBUG(x)
#endif

int
main(int argc, char **argv)
{
	char *command_line = NULL;
	HANDLE process;
	prog_args_t prog_args;
	int err;
	int exit_code;
	int io_flags = 0;
	int quiet;
	int i = 0;

	parse_command_line(argc, argv, &prog_args);

	quiet = prog_args.quiet;

	if (prog_args.combine)
		io_flags |= PROC_COMBINE_OUTPUT;
	if (prog_args.append_out)
		io_flags |= PROC_APPEND_STDOUT;
	if (prog_args.append_err)
		io_flags |= PROC_APPEND_STDERR;

	err = create_process(prog_args.command_args, io_flags,
	                     prog_args.in, prog_args.out, prog_args.err,
	                     &process);

	command_line = flatten_args(prog_args.command_args);

	if (!quiet && err == 0) {
		fprintf(stderr, "STARTED %s\n", command_line); fflush(stderr);
		HeapFree(GetProcessHeap(), 0, command_line);
		command_line = NULL;
	} else {
		fprintf(stderr, "FAILED STARTUP %s\n", command_line);
		fflush(stderr);
		HeapFree(GetProcessHeap(), 0, command_line);
		command_line = NULL;
		exit(1);
	}

	err = wait_process(process, &exit_code);

	CloseHandle(process);
	process = NULL;

	if (!quiet && err == 0) {
		if (exit_code == 0) {
			fprintf(stderr, "DONE\n");
			fflush(stderr);
		} else {
			fprintf(stderr, "ERROR EXIT_CODE(%d)\n", exit_code);
			fflush(stderr);
		}
	} else {
		fprintf(stderr, "FAILED WAIT\n");
		fflush(stderr);
		exit(1);
	}

	/* pass on the exit code if it was asked for */
	if (prog_args.pass_exit_code) err = exit_code;

	return err;
}

static void
parse_command_line(int argc, char **argv, prog_args_t *prog_args)
{
	char* prog_name = argv[0];
	char* in = NULL;
	char* out = NULL;
	char* err = NULL;
	int combine = FALSE;

	prog_args->name = prog_name;
	prog_args->quiet = FALSE;
	prog_args->pass_exit_code = FALSE;
	prog_args->append_out = FALSE;
	prog_args->append_err = FALSE;
	prog_args->command_args = NULL;

	/* drop the program name */
	argc--; argv++;

	/* process option args */
	while (argc > 0 && argv[0][0] == '-') {
		char* arg = argv[0];
		if ((strcmp(arg, "-h") == 0) || (strcmp(arg, "--help") == 0)) {
			usage(prog_args->name);
		} else if (strcmp(arg, "-c") == 0) {
			if (err != NULL) {
				fprintf(stderr, "%s: cannot specify -c and standard error file\n",
				        prog_name);
				usage(prog_name);
			}
			combine = TRUE;
			argc -= 1; argv += 1;
		} else if (strcmp(arg, "-i") == 0) {
			check_option_arg(prog_name, "standard input filename", argv[0], argc);
			in = argv[1];
			argc -= 2; argv += 2;
		} else if (strcmp(arg, "-r") == 0 || strcmp(arg, "-R") == 0) {
			if (out != NULL) {
				fprintf(stderr, "%s: cannot specify standard output file twice\n",
				        prog_name);
				usage(prog_name);
			}
			check_option_arg(prog_name, "standard output filename", argv[0], argc);
			out = argv[1];
			if (argv[0][1] == 'R')
				prog_args->append_out = 1;
			argc -= 2; argv += 2;
		} else if (strcmp(arg, "-s") == 0 || strcmp(arg, "-S") == 0) {
			if (err != NULL) {
				fprintf(stderr, "%s: cannot specify standard error file twice\n",
				        prog_name);
				usage(prog_name);
			} else if (combine) {
				fprintf(stderr, "%s: cannot specify standard error file and -c\n",
				        prog_name);
				usage(prog_name);
			}
			check_option_arg(prog_name, "standard error filename", argv[0], argc);
			err = argv[1];
			if (argv[0][1] == 'S')
				prog_args->append_err = 1;
			argc -= 2; argv += 2;
		} else if (strcmp(arg, "-q") == 0) {
			prog_args->quiet = TRUE;
			argc -= 1; argv += 1;
		} else if (strcmp(arg, "-e") == 0) {
			prog_args->pass_exit_code = TRUE;
			argc -= 1; argv += 1;
		} else {
			fprintf(stderr, "%s: unknown option \"%s\"\n",
			        prog_args->name, arg);
			usage(prog_args->name);
		}
	}

	prog_args->combine = combine;
	prog_args->in = in;
	prog_args->out = out;
	prog_args->err = err;

	if (argc > 0) {
		prog_args->command_args = argv;
	} else {
		fprintf(stderr, "%s: missing command line\n", prog_args->name);
		usage(prog_args->name);
	}
}

static void
check_option_arg(char *prog_name, char *desc, char *opt, int argc)
{
	if (argc < 2) {
		fprintf(stderr,
		        "%s: Expected %s after \"%s\" option but ran out of arguments.\n",
		        prog_name, desc, opt);
		usage(prog_name);
	}
}

static void
usage(char *prog_name)
{
	fprintf(stderr,
	        "usage: %s [options] command_line\n"
	        "    Runs command_line as a subprocess and provides additional\n"
	        "    subcommand start and finish status.\n\n"
	        "Options:\n"
	        "  -c            Combine stderr on stdout.\n"
	        "  -q            Do not print start and end status.\n"
	        "  -e            Return the sub-command's exit code as our own.\n"
	        "  -i            Specify a file for input instead of standard input.\n"
	        "  -r or -R      Specify a file for output instead of standard output.\n"
	        "                The -r option specifies that output will overwrite any existing\n"
	        "                file by the same name, and the -R option specifies that the\n"
	        "                output will be appended.\n"
	        "  -s or -S      Specify a file for error output instead of standard error.\n"
	        "                The -s option specifies that output will overwrite any existing\n"
	        "                file by the same name, and the -S option specifies that the\n"
	        "                output will be appended.\n"
	        "  -h or --help  Print this help.\n",
	        prog_name);
	exit(2);
}

static DWORD
create_process(char **command_args, int io_flags, char *stdin_filename, char *stdout_filename,
               char *stderr_filename, HANDLE *process)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	io_handles_t child_io;
	io_handles_t parent_io;
	BOOL status_ok;
	DWORD err;
	char* err_msg;
	char* command_line;
	int result = 0;

	err = create_io(io_flags,
	                stdin_filename, stdout_filename, stderr_filename,
	                &child_io);

	if (err != ERROR_SUCCESS)
		return err;

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);

	command_line = flatten_args(command_args);
	PDEBUG(("command_line = %s\n", command_line));

	if (child_io.in != NULL) {
		parent_io.in = swap_std_handle(STD_INPUT_HANDLE, child_io.in);
	}
	if (child_io.out != NULL) {
		parent_io.out = swap_std_handle(STD_OUTPUT_HANDLE, child_io.out);
	}
	if (child_io.err != NULL) {
		parent_io.err = swap_std_handle(STD_ERROR_HANDLE, child_io.err);
	}

	status_ok = CreateProcess(NULL, command_line,
	                          NULL, NULL,
	                          TRUE, 0, NULL, NULL,
	                          &si, &pi);

	if (child_io.in != NULL) {
		SetStdHandle(STD_INPUT_HANDLE, parent_io.in);
	}
	if (child_io.out != NULL) {
		SetStdHandle(STD_OUTPUT_HANDLE, parent_io.out);
	}
	if (child_io.err != NULL) {
		SetStdHandle(STD_ERROR_HANDLE, parent_io.err);
	}

	HeapFree(GetProcessHeap(), 0, command_line);
	command_line = NULL;

	if (!status_ok) {
		err = GetLastError();
		err_msg = win_error_string(err);
		warn("Error %d from CreateProcess().\n%s", err, err_msg);
		LocalFree(err_msg);
		*process = NULL;
	} else {
		PDEBUG(("CreateProcess() succeded, pid = %u\n", pi.dwProcessId));
		*process = pi.hProcess;
		/* Do not need the main thread handle */
		CloseHandle(pi.hThread);
	}

	/* Close the pipe handles that have been inherited by the child process.
	 * The parent does not need the child end references.
	 */
	if ((child_io.in && !CloseHandle(child_io.in)) ||
	    (child_io.out && !CloseHandle(child_io.out)) ||
	    (child_io.err && !CloseHandle(child_io.err))) {
		err = GetLastError();
		err_msg = win_error_string(err);
		warn("Error %d from CloseHandle().\n%s", err, err_msg);
		LocalFree(err_msg);
	}

	return err;
}

static HANDLE
swap_std_handle(DWORD num_std_handle, HANDLE new_handle)
{
	HANDLE old_handle = GetStdHandle(num_std_handle);
	SetStdHandle(num_std_handle, new_handle);
	return old_handle;
}

static DWORD
create_io(int flags, char *stdin_filename, char *stdout_filename, char *stderr_filename,
          io_handles_t *child)
{
	DWORD err = ERROR_SUCCESS;

	memset(child, 0, sizeof(io_handles_t));

	/* sanity check the args */
	if ((flags & PROC_COMBINE_OUTPUT) && stderr_filename != NULL) {
		DEBUG_ERR(("create_io(): PROC_COMBINE_OUTPUT flag on but stderr "
		           "filename was not null.\n"));
		stderr_filename = NULL;
	}

	if ((flags & PROC_APPEND_STDOUT) && stdout_filename == NULL)
		DEBUG_ERR(("create_io(): PROC_APPEND_STDOUT flag on but stdout "
		           "filename was null.\n"));

	if ((flags & PROC_APPEND_STDERR) && stderr_filename == NULL)
		DEBUG_ERR(("create_io(): PROC_APPEND_STDERR flag on but stderr "
		           "filename was null.\n"));

	if (stdin_filename != NULL) {
		/* Open a file for the child process's STDIN */
		err = create_io_file(stdin_filename, TRUE, 0, &child->in);
		if (err != ERROR_SUCCESS) {
			DEBUG_ERR(("Could not open stdin file \"%s\" (error = %u)\n",
			           stdin_filename, err));
			goto error_exit;
		}
	}

	if (stdout_filename != NULL) {
		/* Open or Create a file for the child process's STDOUT */
		err = create_io_file(stdout_filename, FALSE,
		                     (flags & PROC_APPEND_STDOUT) != 0,
		                     &child->out);
		if (err != ERROR_SUCCESS) {
			DEBUG_ERR(("Could not open stdout file \"%s\" (error = %u)\n",
			           stdout_filename, err));
			goto error_exit;
		}
	}

	if (stderr_filename != NULL) {
		/* Open or Create a file for the child process's STDERR */
		err = create_io_file(stderr_filename, FALSE,
		                     (flags & PROC_APPEND_STDERR) != 0,
		                     &child->err);
		if (err != ERROR_SUCCESS) {
			DEBUG_ERR(("Could not open stderr file \"%s\" (error = %u)\n",
			           stderr_filename, err));
			goto error_exit;
		}
	} else if (0 != (flags & PROC_COMBINE_OUTPUT)) {
		BOOL ok;
		HANDLE out = child->out;
		if (out == NULL) out = GetStdHandle(STD_OUTPUT_HANDLE);
		/* Create a duplicate of the stdout write handle for the
		 * stderr write handle. It is necessary to dup the stdout
		 * handle instead of just passing the same handle because
		 * the child may close one of the output handles and
		 * expect to still use the other.
		 */
		ok = DuplicateHandle(GetCurrentProcess(), out,
		                     GetCurrentProcess(), &child->err,
		                     0, TRUE, DUPLICATE_SAME_ACCESS);
		if (!ok) {
			err = GetLastError();
			DEBUG_ERR(("DuplicateHandle failed (error = %u)\n", err));
			goto error_exit;
		}
	}

	return err;

error_exit:
	if (child->in != NULL)      CloseHandle(child->in);
	if (child->out != NULL)     CloseHandle(child->out);
	if (child->err != NULL)     CloseHandle(child->err);
	memset(child, 0, sizeof(io_handles_t));

	return err;
}

static DWORD
create_io_file(char *name, int input, int append, HANDLE *file)
{
	SECURITY_ATTRIBUTES sa;
	DWORD disposition, access;
	DWORD share = FILE_SHARE_READ | FILE_SHARE_WRITE;
	DWORD err = ERROR_SUCCESS;

	/* Init a security attribute structure for file creation.
	 * Set the bInheritHandle flag so file handles are inherited.
	 */
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	if (input) {
		access = GENERIC_READ;
		disposition = OPEN_EXISTING;
		/* sanity check the args */
		if (append) {
			DEBUG_ERR(("create_io_file(): Both Input and Append flags were "
			           "specifed.\n"));
			append = FALSE;
		}
	} else {
		access = GENERIC_WRITE;
		if (append)
			disposition = OPEN_ALWAYS;
		else
			disposition = CREATE_ALWAYS;
	}

	*file = CreateFile(name, access, share, &sa, disposition,
	                   FILE_ATTRIBUTE_NORMAL, NULL);

	if (*file == INVALID_HANDLE_VALUE) {
		err = GetLastError();
		DEBUG_ERR(("create_io_file(): File creation failed (error = %u)\n", err));
	} else if (append) {
		/* seek to the end of the file */
		DWORD pos;

		pos = SetFilePointer(*file, 0, NULL, FILE_END);

		if (pos == 0xFFFFFFFF) {
			err = GetLastError();
			if (err != ERROR_SUCCESS) {
				DEBUG_ERR(("create_io_file(): SetFilePointer failed "
				           "(error = %u)\n", err));
			}
		}
	}

	return err;
}

static char *
flatten_args(char **args)
{
	char** parg = args;
	char* command_line = NULL;
	size_t command_length = 0;

	while (*parg != NULL) {
		command_length += strlen(*parg) + 1;
		parg++;
	}

	command_line = (char*)HeapAlloc(GetProcessHeap(), 0, command_length);
	command_line[0] = '\0';

	parg = args;
	while (*parg != NULL) {
		strcat(command_line, *parg);
		parg++;
		if (*parg != NULL)
			strcat(command_line, " ");
	}

	return command_line;
}

static int
wait_process(HANDLE process, int *pexit_code)
{
	DWORD wait_result;
	BOOL status_ok;
	DWORD exit_code;

	*pexit_code = 0;

	PDEBUG(("Starting WaitForSingleObject on proc handle %#X\n", process));
	wait_result = WaitForSingleObject(process, INFINITE);
	PDEBUG(("Finished WaitForSingleObject on proc handle %#X\n", process));

	if (wait_result == WAIT_FAILED) {
#if DEBUG
		DWORD err = GetLastError();
		char* err_msg = win_error_string(err);
		DEBUG_ERR(("Error %d from WaitForSingleObject().\n%s\n", err, err_msg));
		LocalFree(err_msg);
#endif
		return 1;
	}

	status_ok = GetExitCodeProcess(process, &exit_code);

	if (!status_ok) {
#if DEBUG
		DWORD err = GetLastError();
		char* err_msg = win_error_string(err);
		DEBUG_ERR(("Error %d from GetExitCodeProcess().\n%s\n", err, err_msg));
		LocalFree(err_msg);
#endif
		return 1;
	}

	PDEBUG(("GetExitCodeProcess(%u) returned %u\n", process, exit_code));
	*pexit_code = (int)exit_code;

	return 0;
}

static void
DebugOutput(const char *fmt, ...)
{
	char msg_buf[1024];
	va_list args;

	va_start(args, fmt);

	wvsprintf(msg_buf, fmt, args);
	OutputDebugString(msg_buf);

	va_end(args);
}

static char *
win_error_string(DWORD err)
{
	char* msg_buffer;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
	              FORMAT_MESSAGE_FROM_SYSTEM,
	              NULL, err,
	              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	              (LPTSTR)&msg_buffer, 0, NULL);

	return msg_buffer;
}
