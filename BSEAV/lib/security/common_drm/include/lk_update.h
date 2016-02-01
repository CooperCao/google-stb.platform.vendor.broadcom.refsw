/***************************************************************************
 *     Copyright (c) 2005-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 ***************************************************************************/
/**
 *  File : psk_lk_update.h
 *  
 *  Author : kdasu
 * 
 *  Header file for the linux kernel update API
 * 
 */
#ifndef LK_UPDATE_H
#define LK_UPDATE_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "vfile_funcs.h"

/*
 * Current raw partition updates supported on NAND as well as NOR
 */
#define  LK_PARTITION_CFE       "cfe"
#define  LK_PARTITION_DRMREGION "drmregion"
#define  LK_PARTITION_VMLINUX   "vmlinux"
#define  LK_PARTITION_SPLASH    "splash"
#define  LK_PARTITION_RAWNVR    "rawnvr"


#ifdef __cplusplus
extern "C" {
#endif

/*
 * Structure to store the partion information
 */
typedef struct _LKPartInfo
{
    char            partitionName[128];
    char            partitionDevName[128];
    char            partitionBDevName[128];
    char            partitionRDevName[128];
    unsigned  int   partitionSize;
    unsigned  int   partitionEraseBlockSize;
} LK_PARTITION_INFO;


/* Function   : hexStringToUint()
 *
 * Parameters : IN char* hex string 
 *
 * Returns    : unsigned int the converted decimal value from  hex number in string format
 * 
 * Function is a helper function to deive the decimal value of a given hex string 
 * function assumes maximum 32 bit hex number
 */
unsigned int hexStringToUint(char *hexStr);

/* LKGetPartitionInfo 
 * Parameter : IN char* partion name 
 *             LK_PARTION_INFO* partition_info structure
 *
 * Returns   : -1 on a failure, 1 on Success
 * 
 * Function validates the passed kernel image 
 *
 */
int LKGetPartitionInfo (const char *part_name, LK_PARTITION_INFO *part_info);

/*
 * Function  : LKUpdate()
 * Parameter : IN char* ifile input file name
 *             IN int   no_header , variable to let the API know if header is expected 
 *                      for validating the image
 * Returns   : -1 on failure and  0 on success
 *
 * Public function to invoke a kernel upgrade with an input file
 */
int LKUpdate(const char* ifile, int no_header);

/*
 * Function  : LKUpdatePartition()
 * Parameter : IN char* ifile input file name
 *             IN char* partition name 
 *             IN int   no_header , variable to let the API know if header is expected 
 *                      for validating the image
 * Returns   : -1 on failure and  0 on success
 *
 * Public function to invoke a partition update  with an input file
 */
int LKUpdatePartition(const char* ifile, const char* part_name, int no_header);

/*
 * Function  : LKReadPartition()
 * Parameter : 
 *             IN char* partName partition name 
 *             IN char* input buffer to read into
 *             IN int   len
 * Returns   : -1 on failure and > 0 on success
 *
 * Public function to read partition data into buffer
 */
int LKReadPartition(const char* partName, void * data, int length);

/*
 * Function  : LKUpdatePartition_checkVFS()
 * Parameter : IN char* ifile input file name
 *             IN char* part_name partition name 
 *             IN int   header_present, variable to let the API know if header is expected 
 *                      for validating the image
 * Returns   : -1 on failure and > 0 on success
 *
 * Public function to invoke a partition upgrade with an input file. Uses vfile funcs if (and
 *  only if) the file starts with "./".
 * 
 * Note: This will update a partition with a file that is not encrypted.
 * NOTE2: This is only expected to be called by an external update task, such as the loader!!
 */

int LKUpdatePartition_checkVFS(const char* ifile, const char* part_name, int header_present, void *vfuncs2use);


typedef void 
(*UpdateAndVerifyCallbackFunc) (
    /*in */ void      *callback_context,
    /*in */ size_t    total_input_bytes,  /* number of bytes in the input file */
    /*in */ bool      erasing, 
    /*in */ size_t    bytes_erased_so_far,
    /*in */ bool      writing, 
    /*in */ size_t    bytes_written_so_far,
    /*in*/  size_t    total_device_space);

/***********************************************************************
** FUNCTION:
**   LKUpdateAndVerifyPartitionWithProgress
**
** DESCRIPTION:
**   Like LKReadPartition, except that (a) it notifies the caller via 
**   a callback about the incremental progress made and (b) it reads
**   back the data and verifies/compares it against the input.
**
** RETURNS:
**   0 on success.
**  -1 on failure (file missing)
**  -2 on failure (partition missing)
**  -3 on failure (verify failed)
**  -4 on failure (other)
**
** EXAMPLE:
**   LKUpdateAndVerifyPartitionWithProgress("images/2612-3.1-v10r15_016/cramfs-7440b0.img", "vmlinux", callback_context, my_callback_func)
**
************************************************************************/
int 
LKUpdateAndVerifyPartitionWithProgress(
    /*in */ const char *ifile, 
    /*in */ const char *part_name, 
    /*in */ void *callback_context
    );

/***********************************************************************
** FUNCTION:
**   LKUpdateAndVerifyPartitionWithProgressVfile
**
** DESCRIPTION:
**   Like LKUpdateAndVerifyPartitionWithProgress, except that is also
**   takes a structure of function pointers for the fiel functions
**   (open, seek, read, close) so that they can be overridden.
**
** RETURNS:
**   0 on success.
**  -1 on failure (file missing)
**  -2 on failure (partition missing)
**  -3 on failure (verify failed)
**  -4 on failure (other)
**
** EXAMPLE:
**   LKUpdateAndVerifyPartitionWithProgress("images/2612-3.1-v10r15_016/cramfs-7440b0.img", "vmlinux", callback_context, my_callback_func)
**
************************************************************************/
int 
LKUpdateAndVerifyPartitionWithProgressVfile(
    /*in */ const char *ifile, 
    /*in */ const char *part_name, 
    /*in */ void *callback_context,
    /*in */ virtual_file_funcs_t *vfile_funcs /* this can be NULL, and if so the standard file functions are used */
    );

/***********************************************************************
** FUNCTION:
**   LKUpdateAndVerifyPartitionWithProgressFromBuffer
**
** DESCRIPTION:
**   Like LKUpdateAndVerifyPartitionWithProgress, except that it takes
**   its source data from a buffer not a filename.
**
** RETURNS:
**   0 on success.
**  -1 on failure (file missing)
**  -2 on failure (partition missing)
**  -3 on failure (verify failed)
**  -4 on failure (other)
**
** EXAMPLE:
**   LKUpdateAndVerifyPartitionWithProgressFromBuffer(file_pointer, file_size, "vmlinux", my_callback_func)
**
************************************************************************/
int LKUpdateAndVerifyPartitionWithProgressFromBuffer(
    /*in */ unsigned char *file_pointer, 
    /*in */ size_t file_size,
    /*in */ const char *part_name, 
    /*in */ void *callback_context
    );

#ifdef __cplusplus
}
#endif

#endif
