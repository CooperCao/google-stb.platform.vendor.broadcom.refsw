/*
********************************************************************************
**  Copyright (c) 2006-2012, Broadcom Corporation
**  All Rights Reserved;
**  Confidential Property of Broadcom Corporation
**
**  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
**  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
**  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
**
********************************************************************************
*/
#ifndef __VFILE_FUNCS_H__
#define __VFILE_FUNCS_H__

#include <sys/types.h>


typedef struct{
    int (*vopen)(const char *pathname, int flags, ...);
    off_t (*vlseek)(int fildes, off_t offset, int whence);   
    ssize_t (*vread)(int fd, void *buf, size_t count);        
    int (*vclose)(int fd);
} virtual_file_funcs_t;


#endif // __VFILE_FUNCS_H__

