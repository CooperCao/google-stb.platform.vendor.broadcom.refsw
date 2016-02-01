/***************************************************************
**
** Broadcom Corp. Confidential
** Copyright 2003-2008 Broadcom Corp. All Rights Reserved.
**
** THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED 
** SOFTWARE LICENSE AGREEMENT BETWEEN THE USER AND BROADCOM. 
** YOU HAVE NO RIGHT TO USE OR EXPLOIT THIS MATERIAL EXCEPT 
** SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
**
** File:		mpod_test_io.c
** Description: Performs a io functions for mpod test.
**
** Created: 04/18/2001 
**
** REVISION:
**
** $Log: $
**
**
****************************************************************/

#include "mpod.h"
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include "mpod_test_io.h"

/***************************************************************************
 *  GetInputString
 ***************************************************************************/

int GetInputString(char *s)
{
	fgets(s, 31, stdin);
	return strlen(s);
}

/***************************************************************************
 *  GetInputChar
 ***************************************************************************/
int GetInputChar(void)
{
	struct termios oldattr, tattr;
	int in;

	tcgetattr(0, &tattr);
	oldattr = tattr;
	tattr.c_lflag &= ~(ICANON);
	/* tattr.c_lflag &= ~(ECHO); */
	tattr.c_cc[VTIME] = 0;
	tattr.c_cc[VMIN] = 1;
	tcsetattr(0, TCSANOW, &tattr);
	
	in = fgetc(stdin);

	tcsetattr(0, TCSANOW, &oldattr);

	return in;
}




