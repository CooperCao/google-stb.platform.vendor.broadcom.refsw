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
** File:		mpod_test_io.h
** Description: headers for io functions for pod test.
**
** Created: 04/18/2001 
**
** REVISION:
**
** $Log: $
**
**
****************************************************************/
#ifndef MPOD_TEST_IO_H
#define MPOD_TEST_IO_H

#define MPOD_open(file) open(file,O_RDWR)

#ifdef __cplusplus
extern "C" {
#endif

int GetInputString(char *s);
int GetInputChar(void);

#ifdef __cplusplus
}
#endif


#endif

