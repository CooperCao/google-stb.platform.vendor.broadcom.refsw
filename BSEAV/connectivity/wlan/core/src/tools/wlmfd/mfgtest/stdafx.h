// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <windows.h>


#include <sstream>
#include <io.h>
#include <string.h>
using namespace std; /*without this, string type will cause syntax error, we have to use std::string instead*/

#define WIDE_CHAR(X)  X

// TODO: reference additional headers your program requires here
