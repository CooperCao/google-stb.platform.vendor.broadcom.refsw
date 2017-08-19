// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//


#pragma once


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
//#include <tchar.h>

#include <windows.h>
#include <stdio.h>

#include <sstream>
#include <string.h>
#include <io.h>

using namespace std; /*without this, string type will cause syntax error, we have to use std::string instead*/

// TODO: reference additional headers your program requires here
#define WIDE_CHAR(X)  X

#if defined(_DEBUG) || defined(DEBUG)
#define DBGMSG(args) printf args
#else
#define DBGMSG(args)
#endif
