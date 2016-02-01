/***************************************************************************
 *     Copyright (c) 2006-2009, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef __BFILE_IO_H__
#define __BFILE_IO_H__

#include "bfile_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
Summary:
bfile_io_read_t is used for the I/O read interface

Description:
The following is one example using bfile_stdio.h:

    FILE *file = fopen(filename, "r+");
    bfile_io_read_t readfile = bfile_stdio_read_attach(file);
    readfile->seek(readfile, offset, SEEK_SET); // a generic seek
    readfile->read(readfile, buffer, length);   // a generic read
    bfile_stdio_read_detach(readfile);
    fclose(file);

**/
typedef struct bfile_io_read *bfile_io_read_t;

/**
Summary:
  bfile_io_read_t is used for the I/O write interface
**/
typedef struct bfile_io_write *bfile_io_write_t;

/**
Summary:
  bio_get_priority is a function typedef used to get the priority of an I/O transaction
**/
typedef int (*bio_get_priority)(void *cntx);

/**
Summary:
  bfile_io_priority is used to get the priority of an I/O transaction
**/
typedef struct bfile_io_priority {
    bio_get_priority get;
    void *cntx;
} bfile_io_priority;

/**
Summary:
  Return the default priority of an I/O transaction
**/
int bio_default_priority(void *cntx);

/**
Summary:
  Initialize an I/O interface with the default priortity
**/
#define BIO_DEFAULT_PRIORITY {bio_default_priority, NULL}

/**
Summary:
  bfile_io_read defines the methods for an I/O read interface
**/
struct bfile_io_read {
    ssize_t (*read)(bfile_io_read_t fd, void *buf, size_t length); /* this method is used to read data, it returns number of bytes read, where 0 means end of file, negative return code means an error, and BFILE_ERROR_NO_DATA means that  there is temporary no data in the file */
    off_t (*seek)(bfile_io_read_t fd, off_t offset, int whence);  /* this method is used to move next read location */
    int (*bounds)(bfile_io_read_t fd, off_t *first, off_t *last); /* this method is used to return size of the file */
    bfile_io_priority priority; /* this member is used to request current priority of I/O transaction */
};

/**
Summary:
  bfile_io_write defines the methods for an I/O write interface
**/
struct bfile_io_write {
    ssize_t (*write)(bfile_io_write_t fd, const void *buf, size_t length); /* this method is used to write data, it returns number of bytes written */
    off_t (*trim)(bfile_io_write_t fd, off_t trim_pos); /* this method is used to truncate file */
    bfile_io_priority priority; /* this member is used to request current priority of I/O transaction */
};

/**
Summary:
 Attach a new priority function to the I/O read interface
**/
void bio_read_attach_priority(bfile_io_read_t fd, bio_get_priority priority, void *cntx);

/**
Summary:
 Attach a new priority function to the I/O write interface
**/
void bio_write_attach_priority(bfile_io_write_t fd, bio_get_priority priority, void *cntx);

/**
Summary:
  This constant defines the default block size for I/O

Description:
  The BIO_BLOCK_SIZE constant is used to optimize I/O. It enables a best effort scheme to have the size of all I/O transactions be a multiplier of the PAGE size.
  BIO_BLOCK_SIZE shall be power of 2.
**/
#define BIO_BLOCK_SIZE 4096

#ifdef __cplusplus
}
#endif

#endif /* __BFILE_IO_H__ */


