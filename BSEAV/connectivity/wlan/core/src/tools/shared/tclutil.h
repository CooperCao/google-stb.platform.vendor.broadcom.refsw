/*
 * A few shared tcl-related utility routines.
 *
 * Copyright 1999 Broadcom Corporation
 * $Id$
 */

#ifndef _tclutil_h_
#define _tclutil_h_

struct tclutilops {
	char *name;
	char *help;
	int (*fn)(int ac, char *av[], Tcl_Interp *interp);
};

extern Tcl_CmdProc cmdutil;
extern Tcl_CmdProc whereis;
extern Tcl_CmdProc s;
extern int cmdtclvar(char *var, char *buf, int *len, int maxbytes, Tcl_Interp *interp);
extern struct Tcl_Obj* cmdtclvarobj(char *var, Tcl_Interp *interp);
extern struct Tcl_Obj* tclsetvar(Tcl_Interp *interp, char* name, Tcl_Obj* val, int flags);

#endif /* _tclutil_h_ */
