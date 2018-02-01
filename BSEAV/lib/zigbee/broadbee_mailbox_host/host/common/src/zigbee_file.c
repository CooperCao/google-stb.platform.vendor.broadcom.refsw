/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <dirent.h>
#include "bbMailAPI.h"
//#include "bbSysNvmManager.h"
#include "zigbee_api.h"
#include "zigbee_common.h"

static int open_fd;
DIR *open_dir_fd;
static char org_filename[256];
static char tmp_filename[256];
static char dirname[256];

static int (*open_func)(const char *path, int oflag, ... );
static int (*close_func)(int fildes);
static ssize_t (*write_func)(int fildes, const void *buf, size_t nbyte);
static ssize_t (*read_func)(int fildes, void *buf, size_t nbyte);
static off_t (*lseek_func)(int fd, off_t offset, int whence);

//#define FILE_TEST

static void copyfile(int dst_fd, int src_fd)
{
    char buf[8192];
    while (1) {
        ssize_t result = read_func(src_fd, &buf[0], sizeof(buf));
        if (!result) break;
        assert(result > 0);
        assert(write_func(dst_fd, &buf[0], result) == result);
    }
}

void NVM_ReadFileInd( NVM_ReadFileIndDescr_t *readFileFromHostParam)
{
    int fd;
    int ret;
    NVM_ReadFileRespParams_t returnData = {0};
    char filename[256];
    char index[16];
    char *pData = NULL;

    if (readFileFromHostParam->params.fileIndex >= MAX_ZIGBEE_FILES) {
        printf("NVM_ReadFileInd:  error, filename index %d larger than allowed %d\n", readFileFromHostParam->params.fileIndex, MAX_ZIGBEE_FILES);
        goto read_file_error;
    }

    if (readFileFromHostParam->params.address+readFileFromHostParam->params.length > MAX_ZIGBEE_FILE_SIZE_IN_BYTES) {
        printf("NVM_ReadFileInd:  error, file access out of range:  address=%d, length=%d, address+length=%d, max size=%d\n", readFileFromHostParam->params.address, readFileFromHostParam->params.length, readFileFromHostParam->params.address+readFileFromHostParam->params.length, MAX_ZIGBEE_FILE_SIZE_IN_BYTES);
        goto read_file_error;
    }

    sprintf(filename, "/tmp/zigbee/dat%d.bin", readFileFromHostParam->params.fileIndex);
    //returnData.payload.size = 0;

    if ((fd = open_func(filename, O_CREAT | O_RDONLY /*O_WRONLY*/)) == -1) {
        returnData.status = NVM_ACCESS_DENIED;
        printf("NVM_ReadFileInd:  error opening file... errno=%d\n", errno);
        goto read_file_error;
    }

    if ((ret = lseek_func(fd, readFileFromHostParam->params.address, SEEK_SET)) == -1) {
        returnData.status = NVM_DATA_NOT_AVAILABLE;
        printf("NVM_ReadFileInd:  error lseek... errno=%d\n", errno);
        goto read_file_error_after_open;
    }

    pData = calloc(sizeof(char), readFileFromHostParam->params.length);
    if ((ret = read_func(fd, pData, readFileFromHostParam->params.length)) != readFileFromHostParam->params.length) {
        returnData.status = NVM_DATA_NOT_AVAILABLE;
        printf("NVM_ReadFileInd:  error reading file... errno=%d, %s, buff=%p\n", errno, strerror(errno), pData);
        goto read_file_error_after_open;
    }

    returnData.status = NVM_NO_ERROR;
    SYS_MemAlloc(&returnData.payload, ret);
    SYS_CopyToPayload(&returnData.payload, 0, pData, ret);

read_file_error_after_open:
    if(pData)
        free(pData);
    if (fdatasync(fd)) {
        printf("fdatasync not successful\n");
    }
    if (close_func(fd) == -1) {
        printf("NVM_ReadFileInd:  error closing file... errno=%d\n", errno);
    }
read_file_error:
    readFileFromHostParam->callback(readFileFromHostParam, &returnData);
}

#ifdef FILE_TEST
void my_WriteFileResp(struct _NVM_WriteFileIndDescr_t *orgInd, NVM_WriteFileRespParams_t *resp)
{
    printf("BBPRJMAIN:  my_WriteFileResp callback invoked!  result=0x%x\n", *resp);
}

void my_ReadFileResp(struct _NVM_ReadFileIndDescr_t *orgInd, NVM_ReadFileRespParams_t *resp)
{
    int i;
    printf("BBPRJMAIN:  my_ReadFileResp callback invoked!\n");
    int length = SYS_GetPayloadSize(&resp->payload);
    char *pData = calloc(sizeof(char), length);
    SYS_CopyFromPayload(pData, &resp->payload, 0, length);
    for (i=0; i < length; i++) {
        printf("%d:  0x%02x\n", i, *(pData + i));
    }
    free(pData);
    SYS_FreePayload(&resp->payload);
}

void my_OpenFileResp(struct _NVM_OpenFileIndDescr_t *orgInd, NVM_OpenFileRespParams_t *resp)
{
    printf("BBPRJMAIN:  my_OpenFileResp callback invoked!\n");
}

void my_CloseFileResp(struct _NVM_CloseFileIndDescr_t *orgInd, NVM_CloseFileRespParams_t *resp)
{
    printf("BBPRJMAIN:  my_CloseFileResp callback invoked!\n");
}

void file_test(void)
{
    unsigned char data_out[16] = {0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f};
    unsigned char data_in[16];

    NVM_WriteFileIndDescr_t writeFileParam = {0};
    NVM_ReadFileIndDescr_t readFileParam = {0};
    NVM_OpenFileIndDescr_t openFileParam = {0};
    NVM_CloseFileIndDescr_t closeFileParam = {0};

    writeFileParam.params.fileIndex=1;
    writeFileParam.params.address=0;
    SYS_MemAlloc(&writeFileParam.params.payload, sizeof(data_out));
    SYS_CopyToPayload(&writeFileParam.params.payload, 0, data_out, sizeof(data_out));
    writeFileParam.callback = my_WriteFileResp;
    NVM_WriteFileInd(&writeFileParam);
    SYS_FreePayload(&writeFileParam.params.payload);


    readFileParam.params.fileIndex=1;
    readFileParam.params.address=0;
    readFileParam.params.length=sizeof(data_in);
    readFileParam.callback = my_ReadFileResp;
    NVM_ReadFileInd(&readFileParam);

    writeFileParam.params.fileIndex=1;
    writeFileParam.params.address=3;
    data_out[0] = 0x47;
    data_out[1] = 0x48;
    data_out[2] = 0x49;
    SYS_MemAlloc(&writeFileParam.params.payload, 3);
    SYS_CopyToPayload(&writeFileParam.params.payload, 0, data_out, 3);
    writeFileParam.callback = my_WriteFileResp;
    NVM_WriteFileInd(&writeFileParam);
    SYS_FreePayload(&writeFileParam.params.payload);

    readFileParam.params.fileIndex=1;
    readFileParam.params.address=0;
    readFileParam.params.length=sizeof(data_in);
    readFileParam.callback = my_ReadFileResp;
    NVM_ReadFileInd(&readFileParam);

    /* Test open and close */
    openFileParam.params.fileIndex=1;
    openFileParam.callback = my_OpenFileResp;
    NVM_OpenFileInd(&openFileParam);

    writeFileParam.params.fileIndex=1;
    writeFileParam.params.address=3;
    data_out[0] = 0x12;
    data_out[1] = 0x34;
    data_out[2] = 0x56;

    SYS_MemAlloc(&writeFileParam.params.payload, 3);
    SYS_CopyToPayload(&writeFileParam.params.payload, 0, data_out, 3);
    writeFileParam.callback = my_WriteFileResp;
    NVM_WriteFileInd(&writeFileParam);
    SYS_FreePayload(&writeFileParam.params.payload);

    writeFileParam.params.fileIndex=1;
    writeFileParam.params.address=6;
    data_out[0] = 0x78;
    data_out[1] = 0x9a;
    data_out[2] = 0xbc;

    SYS_MemAlloc(&writeFileParam.params.payload, 3);
    SYS_CopyToPayload(&writeFileParam.params.payload, 0, data_out, 3);
    writeFileParam.callback = my_WriteFileResp;
    NVM_WriteFileInd(&writeFileParam);
    SYS_FreePayload(&writeFileParam.params.payload);

    closeFileParam.params.fileIndex=1;
    closeFileParam.callback = my_CloseFileResp;
    NVM_CloseFileInd(&closeFileParam);

    /* Verify update */
    readFileParam.params.fileIndex=1;
    readFileParam.params.address=0;
    readFileParam.params.length=sizeof(data_in);
    readFileParam.callback = my_ReadFileResp;
    NVM_ReadFileInd(&readFileParam);
}
#endif

void Zigbee_NVM_Init(
    int (*open_func_user)(const char *path, int oflag, ... ),
    int (*close_func_user)(int fildes),
    ssize_t (*write_func_user)(int fildes, const void *buf, size_t nbyte),
    ssize_t (*read_func_user)(int fildes, void *buf, size_t nbyte),
    off_t (*lseek_func_user)(int fd, off_t offset, int whence)
    )
{
    open_fd = -1;
    open_dir_fd = (DIR *)(-1);
    sprintf(dirname, "/tmp/zigbee");
    open_dir_fd = opendir(dirname);

    if (open_func_user) {
        open_func = open_func_user;
    } else {
        open_func = open;
    }
    if (close_func_user) {
        close_func = close_func_user;
    } else {
        close_func = close;
    }
    if (write_func_user) {
        write_func = write_func_user;
    } else {
        write_func = write;
    }
    if (read_func_user) {
        read_func = read_func_user;
    } else {
        read_func = read;
    }
    if (lseek_func_user) {
        lseek_func = lseek_func_user;
    } else {
        lseek_func = lseek;
    }

#ifdef FILE_TEST
    file_test();    /* Do some file test */
#endif
}

void Zigbee_NVM_Uninit()
{
    closedir(open_dir_fd);
    open_dir_fd = 0;
    open_fd = 0;
}

void NVM_OpenFileInd( NVM_OpenFileIndDescr_t *openFileIndDescr)
{
    int org_fd, tmp_fd;
    int ret;
    NVM_OpenFileRespParams_t returnData;
    returnData.status = BROADBEE_NO_ERROR;

    if (open_fd != -1) {
        if (fdatasync(open_fd)) {
            printf("fdatasync not successful\n");
        }
        close_func(open_fd);
        open_fd = -1;
    }

    if (openFileIndDescr->params.fileIndex >= MAX_ZIGBEE_FILES) {
        printf("NVM_OpenFileInd:  error, filename index %d larger than allowed %d\n", openFileIndDescr->params.fileIndex, MAX_ZIGBEE_FILES);
        goto open_file_error;
    }

    sprintf(org_filename, "/tmp/zigbee/dat%d.bin", openFileIndDescr->params.fileIndex);
    sprintf(tmp_filename, "/tmp/zigbee/dat%d.bin.tmp", openFileIndDescr->params.fileIndex);

    if ((org_fd = open_func(org_filename, O_RDONLY)) == -1) {
        /* File does not already exist. */
        if ((tmp_fd = open_func(tmp_filename, O_CREAT | O_WRONLY)) == -1) {
            printf("NVM_OpenFileInd:  error opening file %s... errno=%d, %s\n", tmp_filename, errno, strerror(errno));
            returnData.status = BROADBEE_ACCESS_DENIED;
            goto open_file_error;
        }
    } else {
        if ((tmp_fd = open_func(tmp_filename, O_CREAT | O_WRONLY)) != -1) {
            copyfile(tmp_fd, org_fd);

            /* Close original file */
            if (fdatasync(org_fd)) {
                printf("fdatasync not successful\n");
            }
            if (close_func(org_fd) == -1) {
                printf("NVM_OpenFileInd:  error closing file... errno=%d\n", errno);
                //returnData.status = BROADBEE_ACCESS_DENIED;
            }
        } else {
            printf("NVM_OpenFileInd:  error creating temporary file %s\n", tmp_filename);
        }
    }
    open_fd = tmp_fd;

open_file_error:
    openFileIndDescr->callback(openFileIndDescr, &returnData);
}

void NVM_CloseFileInd( NVM_CloseFileIndDescr_t *closeFileIndDescr)
{
//    char filename[256];
    int ret;
    NVM_CloseFileRespParams_t returnData;
    returnData.status = BROADBEE_NO_ERROR;

    if (open_fd == -1) {
        returnData.status = BROADBEE_ACCESS_DENIED;
        goto close_file_error;
    }

    if (closeFileIndDescr->params.fileIndex >= MAX_ZIGBEE_FILES) {
        printf("NVM_CloseFileInd:  error, filename index %d larger than allowed %d\n", closeFileIndDescr->params.fileIndex, MAX_ZIGBEE_FILES);
        goto close_file_error;
    }

    //sprintf(filename, "/tmp/zigbee/dat%d.bin", openFileIndDescr->params.id);

    if (fdatasync(open_fd)) {
        printf("fdatasync not successful\n");
    }
    if (close_func(open_fd) == -1) {
        printf("NVM_CloseFileInd:  error closing file... errno=%d\n", errno);
        returnData.status = BROADBEE_ACCESS_DENIED;
        goto close_file_error;
    }
    open_fd = -1;
    if ((ret = rename(tmp_filename, org_filename)) == -1) {
        printf("NVM_CloseFileInd:  error renaming file... errno=%d, %s\n", errno, strerror(errno));
        returnData.status = BROADBEE_ACCESS_DENIED;
    }
    if (fdatasync(dirfd(open_dir_fd))) {
        printf("fsync on open_dir_fd not successful\n");
    }

close_file_error:
    closeFileIndDescr->callback(closeFileIndDescr, &returnData);
}

#ifdef RF4CE_TEST
#define CC_PROFILE_ID  0xCC

#ifdef PACKED
#undef PACKED
#endif
#define PACKED __attribute__((packed))

#define MAXIMUM_RANGES                    28

static char *reporter_description_for_different_range[MAXIMUM_RANGES] = {
  "0ms-10ms    ",
  "10ms-20ms   ",
  "20ms-30ms   ",
  "30ms-40ms   ",
  "40ms-50ms   ",
  "50ms-60ms   ",
  "60ms-70ms   ",
  "70ms-80ms   ",
  "80ms-90ms   ",
  "90ms-100ms  ",
  "100ms-150ms ",
  "150ms-200ms ",
  "200ms-250ms ",
  "250ms-300ms ",
  "300ms-350ms ",
  "350ms-400ms ",
  "400ms-450ms ",
  "450ms-500ms ",
  "500ms-550ms ",
  "550ms-600ms ",
  "600ms-650ms ",
  "650ms-700ms ",
  "700ms-750ms ",
  "750ms-800ms ",
  "800ms-850ms ",
  "850ms-900ms ",
  "900ms-950ms ",
  "950ms-1000ms"
};

typedef struct PACKED _RC72_Report_t{
    unsigned short received_packets;
    unsigned short error_packets;
    unsigned short received_packets_in_different_range[MAXIMUM_RANGES];
}RC72_Report_t;

static void dumpOutTestResult(const char *const result, int length)
{
    int range = 0;
    RC72_Report_t *report = (RC72_Report_t *)result;

    if(length != sizeof(RC72_Report_t))
    {
        printf("Got a incorrect report packet from remote control\n");
        return;
    }
    printf("Got a correct report packet from remote control, the detail is following\n");
    printf("PacketSent     = %d\n", report->received_packets + report->error_packets);
    printf("PacketReceived = %d\n", report->received_packets);
    printf("PER            = %.2f%%\n", (float)report->error_packets * 100/(float)(report->received_packets + report->error_packets));
    printf("\n");
    for(range = 0; range < MAXIMUM_RANGES; range++)
        printf("%s = %d\n", reporter_description_for_different_range[range], report->received_packets_in_different_range[range]);
}
#endif

void NVM_WriteFileInd( NVM_WriteFileIndDescr_t *writeFileToHostParam)
{
    int org_fd, tmp_fd;
    int ret;
    char *pData = NULL;
    //char filename[256];
    NVM_WriteFileRespParams_t returnData;
    returnData.status = BROADBEE_NO_ERROR;

    if (writeFileToHostParam->params.fileIndex >= MAX_ZIGBEE_FILES) {
        printf("NVM_WriteFileInd:  error, filename index %d larger than allowed %d\n", writeFileToHostParam->params.fileIndex, MAX_ZIGBEE_FILES);
        goto write_file_error;
    }

    if (writeFileToHostParam->params.address+writeFileToHostParam->params.payload.size > MAX_ZIGBEE_FILE_SIZE_IN_BYTES) {
        printf("NVM_WriteFileInd:  error, file access out of range:  address=%d, length=%d, address+length=%d, max size=%d\n", writeFileToHostParam->params.address, writeFileToHostParam->params.payload.size, writeFileToHostParam->params.address+writeFileToHostParam->params.payload.size, MAX_ZIGBEE_FILE_SIZE_IN_BYTES);
        goto write_file_error;
    }

    sprintf(org_filename, "/tmp/zigbee/dat%d.bin", writeFileToHostParam->params.fileIndex);
    sprintf(tmp_filename, "/tmp/zigbee/dat%d.bin.tmp", writeFileToHostParam->params.fileIndex);

    /*
       To ensure no data integrity issue with file, during possible power-loss in the middle of update:
       1. Open file with read only and without create.
       2. If file does not exist, open file with temporary name (filename.tmp).  Open with create and write only.
       2a.    Write file.
       2b.    After done, close file.  Then, rename to permanent name.
       3. If file does exist, copy file into a temporary file (using temporary name, filename.tmp).
       3a.    Modify file.
       3b.    After done, close file.  Then, rename to permanent name.
    */

    if (open_fd == -1) {
        if ((org_fd = open_func(org_filename, O_RDONLY)) == -1) {
            /* File does not already exist. */
            if ((tmp_fd = open_func(tmp_filename, O_CREAT | O_WRONLY)) == -1) {
                printf("NVM_WriteFileInd:  error opening file %s... errno=%d, %s\n", tmp_filename, errno, strerror(errno));
                returnData.status = BROADBEE_ACCESS_DENIED;
                goto write_file_error;
            }
        } else {
            if ((tmp_fd = open_func(tmp_filename, O_CREAT | O_WRONLY)) != -1) {
                copyfile(tmp_fd, org_fd);

                /* Close original file */
                if (fdatasync(org_fd)) {
                    printf("fdatasync not successful\n");
                }
                if (close_func(org_fd) == -1) {
                    printf("NVM_WriteFileInd:  error closing file... errno=%d\n", errno);
                    //returnData.status = BROADBEE_ACCESS_DENIED;
                }
            } else {
                printf("NVM_WriteFileInd:  error creating temporary file %s\n", tmp_filename);
                goto write_file_error;
            }
        }
    } else {
        tmp_fd = open_fd;
    }

    if ((ret = lseek_func(tmp_fd, writeFileToHostParam->params.address, SEEK_SET)) == -1) {
        printf("NVM_WriteFileInd:  error lseek... errno=%d\n", errno);
        returnData.status = BROADBEE_ACCESS_DENIED;
        goto write_file_error_after_open;
    }
    int length = SYS_GetPayloadSize(&writeFileToHostParam->params.payload);
    pData = calloc(sizeof(char), length);
    SYS_CopyFromPayload(pData, &writeFileToHostParam->params.payload, 0, length);
    if ((ret = write_func(tmp_fd, pData, length)) == -1) {
        printf("NVM_WriteFileInd:  error writing file fd=%d... errno=%d, %s\n", tmp_fd, errno, strerror(errno));
        returnData.status = BROADBEE_ACCESS_DENIED;
        goto write_file_error_after_open;
    }
#ifdef RF4CE_TEST
    // Dump out the test result
    if(writeFileToHostParam->params.fileIndex == CC_PROFILE_ID){
        dumpOutTestResult(pData, length);
        int appendFileFd = -1;
        if ((appendFileFd = open_func("/tmp/zigbee/datReports.bin", O_WRONLY | O_CREAT | O_APPEND)) != -1) {
            write_func(appendFileFd, pData, length);
            if (fdatasync(appendFileFd)) {
                printf("fdatasync not successful\n");
            }
            close_func(appendFileFd);
        }
    }
#endif
write_file_error_after_open:
    if(pData)
        free(pData);
    if (open_fd == -1) {
        if (fdatasync(tmp_fd)) {
            printf("fdatasync not successful\n");
        }
        if (close_func(tmp_fd) == -1) {
            printf("NVM_WriteFileInd:  error closing file... errno=%d\n", errno);
            returnData.status = BROADBEE_ACCESS_DENIED;
        }
        if ((ret = rename(tmp_filename, org_filename)) == -1) {
            printf("NVM_WriteFileInd:  error renaming file... errno=%d, %s\n", errno, strerror(errno));
            returnData.status = BROADBEE_ACCESS_DENIED;
        }
        if (fdatasync(dirfd(open_dir_fd))) {
            printf("fsync on open_dir_fd not successful\n");
        }
    }
write_file_error:
    writeFileToHostParam->callback(writeFileToHostParam, &returnData);
}

/* eof zigbee_file.c */
