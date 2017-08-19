/*
 * A few shared tcl-related utility routines.
 *
 * Copyright 1999, Broadcom Corporation.
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <tcl.h>
#include <string.h>
#include <typedefs.h>
#include <utils.h>
#include <sys_xx.h>
#include <tclutil.h>

//extern struct tclutilops pcicmds[];

extern void *simlock;


/* Find the location of a file as indicated by the env var TCLPATH */
int
whereis(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	char *fname;
	static char pathkey[1024];
	char *path;
	char *np;
	FILE *fp;

	/* See if path is absolute or located locally */
	if (argc < 2) {
		interp->result = "Incorrect number of args";
		return TCL_ERROR;
	}

	if ((argv[1][0] == '?') && (argv[1][1] == '\0')) {
		sprintf (interp->result, "Usage: whereis FILENAME\n"
					 "  Find FILENAME in $TCLPATH");
		return TCL_OK;
	}

	fname = argv[1];

	pathkey[0] = '\0';
	interp->result = pathkey;

	if ((fp = fopen(fname, "r")) != NULL) {
		fclose(fp);
		strcpy (pathkey, fname);
		return TCL_OK;
	}

	/* Get the path */
	if ((path = getenv("TCLPATH")) == NULL)
		goto notfound;

	/* Scan through TCLPATH to find the location of the file */
	while (path != NULL && *path != '\0') {

		/* memmove below isn't safe without some zeros in pathkey */
		memset (pathkey, 0, 1024);

		/* Find next path key */
		np = strchr (path, ':');

		/* Extract the pathkey */
		if (np != NULL) {
			strncpy (pathkey, path, np - path);
			pathkey[np - path] = '\0';
			np++;
		}
		else strcpy (pathkey, path);

		/* Convert //<drive-letter> to <drive-letter>:\ format */
		if (pathkey[0] == '/' && pathkey[1] == '/') {
			pathkey[0] = pathkey[2];
			pathkey[1] = ':';
			pathkey[2] = '\\';
			memmove (pathkey + 3, pathkey + 4,
						strlen (pathkey + 4) + 1);
		}

		/* Construct the path */
		if (strlen (pathkey))
			strcat(pathkey, "/");

		strcat(pathkey, fname);

		if ((fp = fopen(pathkey, "r")) != NULL) {
			fclose(fp);
			return TCL_OK;
		}

		path = np;
	}

notfound:
	sprintf(interp->result, "%s: not found", fname);
	return TCL_ERROR;
}

/* Execute s <tclscript> <args> command, which sources a file and sets
 * argc, argv, and searches the environment variable TCLPATH for the tcl
 * script.  This is much cleaner when done with sigwin (see epidiag.c).
 */
int
s(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	char val_argc[16];
	int i, rc;
	Tcl_DString oldargc, oldargv, oldargv0;
	char *str;

	/* Find the location of the tcl script */
	if ((argc == 2) && (argv[1][0] == '?') && (argv[1][1] == '\0')) {
		sprintf (interp->result, "Usage: s FILENAME [ARGS ...]\n"
					"  Source FILENAME passing ARGS to it");
		return TCL_OK;
	}


	rc = whereis (cd, interp, argc, argv);

	if (rc != TCL_OK)
		return rc;

	/* Retain argxx values so they can be restored after a script calls s */
	Tcl_DStringInit (&oldargc);
	str = (char *)Tcl_GetVar (interp, "argc", TCL_GLOBAL_ONLY);
	Tcl_DStringAppend (&oldargc, str, -1);

	Tcl_DStringInit (&oldargv);
	str = (char *)Tcl_GetVar (interp, "argv", TCL_GLOBAL_ONLY);
	Tcl_DStringAppend (&oldargv, str, -1);

	Tcl_DStringInit (&oldargv0);
	str = (char *)Tcl_GetVar (interp, "argv0", TCL_GLOBAL_ONLY);
	Tcl_DStringAppend (&oldargv0, str, -1);



	/* Set argc */
	sprintf (val_argc, "%d", (argc - 2));
	Tcl_SetVar (interp, "argc", val_argc, TCL_GLOBAL_ONLY);

	/* Set argv0 */
	Tcl_SetVar (interp, "argv0", argv[1], TCL_GLOBAL_ONLY);

	/* Set argv */
	Tcl_SetVar (interp, "argv", "", TCL_GLOBAL_ONLY);
	for (i = 2; i < argc; i++) {
		if (i != 2)
			Tcl_SetVar (interp, "argv", " ",
				    TCL_GLOBAL_ONLY | TCL_APPEND_VALUE);
		Tcl_SetVar (interp, "argv", argv[i],
			    TCL_GLOBAL_ONLY | TCL_APPEND_VALUE);
	}
	rc = Tcl_EvalFile (interp, interp->result);

	str = Tcl_DStringValue (&oldargc);
	Tcl_SetVar (interp, "argc", str, TCL_GLOBAL_ONLY);
	str = Tcl_DStringValue (&oldargv);
	Tcl_SetVar (interp, "argv", str, TCL_GLOBAL_ONLY);
	str = Tcl_DStringValue (&oldargv0);
	Tcl_SetVar (interp, "argv0", str, TCL_GLOBAL_ONLY);

	Tcl_DStringFree (&oldargc);
	Tcl_DStringFree (&oldargv);
	Tcl_DStringFree (&oldargv0);
	return rc;
}

/*
 * A generic tcl command helper function.
 * The ClientData is assumed to be a pointer to an array of tclutilops structures.
 */
int
cmdutil(ClientData cd, Tcl_Interp *interp, int ac, char *av[])
{
	int i, rc;
	char *cmd, *basecmd;
	struct tclutilops *cmds;

	basecmd = av[0];
	cmds = (struct tclutilops*) cd;

	rc = TCL_OK;
	/* Someone hit cr. */
	if (ac < 2) {
		sprintf (interp->result, "\"%s ?\" for help", basecmd);
		return rc;
	}

	cmd = av[1];
	av += 2;
	ac -= 2;

	/* Scan through the jump table for a match to the user command.  */
	for (i = 0; cmds[i].name != NULL; i++) {
		if (strcmp (cmds[i].name, cmd) == 0) {
			if (cmds[i].fn == NULL)
				break;
			lock (simlock);
			rc = (*cmds[i].fn)(ac, av, interp);
			unlock (simlock);
			break;
		}
	}

	/* Check for an invalid command. */
	if (!cmds[i].name)
		sprintf (interp->result, "\"%s ?\" for help", basecmd);

	else if (cmds[i].fn == NULL) {
		for (i = 0; cmds[i].name != NULL; i++) {
			printf ("%s %s\n", cmds[i].name, cmds[i].help);
		}
	}

	return rc;
}

/*
 * Find the location of the tcl variable.  Return it in buf, with
 * len in len.
 */
int
cmdtclvar(char *var, char *buf, int *len, int maxbytes, Tcl_Interp *interp)
{
	Tcl_Obj *obj, *part1;
	int objlen;
	char *b;


	/* create part1 name object */
	part1 = Tcl_NewStringObj(var, -1);
	ASSERT (part1);
		
	Tcl_IncrRefCount(part1);

	/* get pointer to variable object */
	obj = Tcl_ObjGetVar2(interp, part1, NULL,
			     TCL_LEAVE_ERR_MSG | TCL_PARSE_PART1);
	Tcl_DecrRefCount(part1);
	if (obj == NULL) {
		goto badret;
	}

	/* get pointer to current variable bytearray */
	b = Tcl_GetStringFromObj(obj, &objlen);
	if (b == NULL) {
		Tcl_SetResult(interp, "Tcl_GetStringFromObj failed", TCL_STATIC);
		goto badret;
	}
	if (maxbytes < objlen)  {
		Tcl_SetResult(interp, "Too many bytes in array", TCL_STATIC);
		goto badret;
	}
		
	*len = objlen;
	memcpy (buf, b, objlen);

	return TCL_OK;
badret:
	return TCL_ERROR;
}

/*
 * Look up a tcl variable and return the object it points to
 */
Tcl_Obj*
cmdtclvarobj(char *var, Tcl_Interp *interp)
{
	Tcl_Obj *obj, *part1;

	/* create part1 name object */
	part1 = Tcl_NewStringObj(var, strlen(var));
	ASSERT (part1);
		
	Tcl_IncrRefCount(part1);

	/* get pointer to variable object */
	obj = Tcl_ObjGetVar2(interp, part1, NULL, TCL_PARSE_PART1 | TCL_LEAVE_ERR_MSG);

	Tcl_DecrRefCount(part1);

	return obj;
}

Tcl_Obj*
tclsetvar(Tcl_Interp *interp, char* name, Tcl_Obj* val, int flags)
{
	Tcl_Obj* var_obj = NULL;
	Tcl_Obj* name_obj;
	
	ASSERT(name != NULL);
	ASSERT(val != NULL);
	
	name_obj = Tcl_NewStringObj(name, -1);

        if (name_obj != NULL) {
		var_obj = Tcl_ObjSetVar2(interp, name_obj, NULL, val, flags);
		Tcl_IncrRefCount(name_obj); Tcl_DecrRefCount(name_obj);
	}
	
	if (var_obj == NULL || var_obj != val) {
		/* if there was an error, or val is not the object ultimately
		 * used for the var, incr and decr to dispose in case
		 * it is not shared */
		Tcl_IncrRefCount(val); Tcl_DecrRefCount(val);
	}
	return var_obj;
}
