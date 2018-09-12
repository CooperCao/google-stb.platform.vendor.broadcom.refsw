/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "bsagelib_types.h"
#include "bkni.h"
#include "sage_srai.h"
#include "bp3_platform_host.h"
#include "bp3_module_host.h"

#include "nexus_security_types.h"
#include "nexus_security.h"
#include "nexus_base_os.h" // for NEXUS_GetEnv

#include "sage_app_utils.h"
#include "bp3_session.h"

extern char bp3_bin_file_name[];
extern char bp3_bin_file_path[];


BDBG_MODULE(bp3_main);

#define BP3_HOST_MAX_BUF_SIZE 65536

uint32_t  bp3SessionTokenSize = 0;
uint8_t  *pBp3SessionToken = NULL;
uint32_t  bp3EncryptedCcfSize = 0;
uint8_t  *pEncryptedCcfBuff = NULL;

static int SAGE_Write_Log_File(uint8_t** logBuff, uint32_t* logSize)
{
    int rc = -1;
    int fd = -1;
    ssize_t bytesWritten = 0;

    fd = open("bp3_log.bin", O_CREAT|O_SYNC|O_RDWR,0644);
    if (fd == -1)
    {
        rc = -2;
        goto end;
    }
    bytesWritten = write (fd, (void *)*logBuff, (size_t)*logSize);
    if (bytesWritten != (size_t)*logSize)
    {
        rc = -6;
        goto end;
    }
    rc = 0;
  end:
    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }
    if (rc)
    {
        fprintf(stderr, "%s ERROR #%d\n", BSTD_FUNCTION, rc);
    }
    return rc;
}


static int SAGE_Write_BP3_Bin_File(uint8_t** bp3BinBuff, uint32_t* bp3BinSize, char *filePath)
{
    int rc = -1;
    int fd = -1;
    ssize_t bytesWritten = 0;


    fd = open(filePath, O_CREAT|O_SYNC|O_RDWR,0644);
    if (fd == -1)
    {
        rc = -2;
        goto end;
    }
    bytesWritten = write (fd, (void *)*bp3BinBuff, (size_t)*bp3BinSize);
    if (bytesWritten != (size_t)*bp3BinSize)
    {
        rc = -6;
        goto end;
    }
    rc = 0;
  end:
    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }
    if (rc)
    {
        fprintf(stderr, "%s ERROR #%d\n", BSTD_FUNCTION, rc);
    }
    return rc;
}

static int SAGE_Read_File(const char *filePath, uint8_t** binBuff, uint32_t* binSize)
{
    int rc = -1;
    int fd = -1;
    uint32_t size = 0;
    uint8_t *buff = NULL;
    ssize_t readSize;

    fd = open(filePath, O_RDONLY);
    if (fd == -1)
    {
        rc = -2;
        goto end;
    }

    /* poll size, allocate destination buffer and return pos to start */
    {
        off_t pos = lseek(fd, 0, SEEK_END);
        if (pos == -1)
        {
            rc = -3;
            goto end;
        }
        size = (uint32_t)pos;
        buff = SRAI_Memory_Allocate(size, SRAI_MemoryType_Shared);
        if (buff == NULL)
        {
            rc = -4;
            goto end;
        }

        pos = lseek(fd, 0, SEEK_SET);
        if (pos != 0)
        {
            rc = -5;
            goto end;
        }
    }

    /* read file in memory */
    {
        readSize = read(fd, (void *)buff, (size_t)size);
        if (readSize != (ssize_t)size)
        {
            rc = -6;
            goto end;
        }
    }
    rc = 0;
end:
    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }

    if (rc)
    {
        if (buff)
        {
            SRAI_Memory_Free(buff);
            buff = NULL;
        }
        fprintf(stderr, "%s ERROR #%d\n", BSTD_FUNCTION, rc);
    }
    *binBuff = buff;
    *binSize = size;
    return rc;
}

static void _usage(const char *binName)
{
    printf("usage:\n");
    printf("%s <bp3_ccf.bin file> \n", binName);
}

static int _parse_cmdline(int argc, char *argv[])
{
    int rc = -1;
    int curr_arg = 1;

    if (argc < 2)
    {
        _usage("bp3");
        rc = -1;
        goto end;
    }

    rc = SAGE_Read_File(argv[curr_arg],&pEncryptedCcfBuff, &bp3EncryptedCcfSize);
    if (rc) { goto end; }

    rc = 0;
end:
    return rc;
}

int bp3_session_start(uint8_t **token, uint32_t *size)
{
  int rc = 0;

  if (!pBp3SessionToken)
  {
    /* This is also initializing all attached modules. */
    if (SAGE_BP3Platform_Init())
    {
      rc = -1;
      goto err_sage;
    }

    // Generate Session Token
    bp3SessionTokenSize = 16;
    pBp3SessionToken = SRAI_Memory_Allocate(bp3SessionTokenSize, SRAI_MemoryType_Shared);
    if (pBp3SessionToken == NULL)
    {
      rc = -1;
      goto err_token;
    }
    if (SAGE_BP3Module_GetSessionToken(pBp3SessionToken, bp3SessionTokenSize))
    {
      SRAI_Memory_Free(pBp3SessionToken);
      rc = -7;
      goto err_token;
    }
  }

  if (token)
    *token = pBp3SessionToken;
  if (size)
    *size = bp3SessionTokenSize;
  return rc;

  err_token:
      SAGE_BP3Platform_Uninit();
  err_sage:
      SAGE_app_leave_nexus();
  err_nexus:
      pBp3SessionToken = NULL;
      if (rc)
      {
          fprintf(stderr, "%s ERROR #%d\n", BSTD_FUNCTION, rc);
      }
      return rc;
}


int bp3_get_otp_id (uint32_t *pOtpIdHigh, uint32_t *pOtpIdLow)
{
    int     rc = 0;

    if (SAGE_BP3Module_GetOtpId(pOtpIdHigh, pOtpIdLow))
    {
        rc = -8;
        goto error;
    }
error:
    if (rc)
    {
        fprintf(stderr, "%s ERROR #%d\n", BSTD_FUNCTION, rc);
    }
    return rc;
}

int bp3_get_chip_info (
    uint8_t  *pFeatureList,
    uint32_t  featureListByteSize,
    uint32_t *pProductID,
    uint32_t *pSecurityCode,
    uint32_t *pBondOption,
    bool     *pProvisioned)
{
    int rc = 0;
    uint8_t *pFeatures = NULL;

    pFeatures = SRAI_Memory_Allocate(featureListByteSize, SRAI_MemoryType_Shared);
    if (pFeatures == NULL)
    {
      rc = -1;
      goto error;
    }

    if (SAGE_BP3Module_GetChipInfo (
        pFeatures,
        featureListByteSize,
        pProductID,
        pSecurityCode,
        pBondOption,
        pProvisioned))
    {
        rc = -9;
        goto error;
    }
    // copy feature list
    memcpy(pFeatureList, pFeatures, featureListByteSize);

error:
    if (pFeatures != NULL)
    {
        SRAI_Memory_Free(pFeatures);
    }
    if (rc)
    {
        fprintf(stderr, "%s ERROR #%d\n", BSTD_FUNCTION, rc);
    }
    return rc;

}

int bp3_ta_start()
{
    int rc = 0;
    if (SAGE_BP3Platform_Init())
    {
      rc = -1;
      goto error;
    }
error:
    if (rc)
    {
        fprintf(stderr, "%s ERROR #%d\n", BSTD_FUNCTION, rc);
    }
    return rc;
}

void bp3_ta_end()
{
    int rc = 0;
    SAGE_BP3Platform_Uninit();
}

int bp3_session_end(uint8_t *ccfBuf, uint32_t ccfSize, uint8_t **logBuf, uint32_t *logSize, uint32_t **status, uint32_t *statusSize, uint8_t **binBuf, uint32_t *binSize)
{
    int       rc = 0;
    uint32_t *pCcfStatus = NULL;
    uint32_t  ccfStatusSize;
    uint32_t  bp3BinSize;
    uint8_t  *pBp3BinBuff = NULL;
    uint32_t  bp3LogBinSize;
    uint8_t  *pBp3LogBinBuff = NULL;
    char     *bp3binFilePath = NULL;
    size_t    filePathLength = 0;
    uint8_t  *pExistingBp3Bin = NULL;
    uint32_t  existingBp3BinSize = 0;

    if (logBuf == NULL)
      // log file not wanted. Must be some kind of error!
      goto leave;

    if (ccfBuf) {
      if (pEncryptedCcfBuff) SRAI_Memory_Free(pEncryptedCcfBuff);
      pEncryptedCcfBuff = SRAI_Memory_Allocate(ccfSize, SRAI_MemoryType_Shared);
      if (pEncryptedCcfBuff == NULL) {
           rc = -2;
           goto leave;
      }
      memcpy(pEncryptedCcfBuff, ccfBuf, ccfSize);
      bp3EncryptedCcfSize = ccfSize;
    }

    // allocate large enough buffer to hold max log size
    bp3LogBinSize  = BP3_HOST_MAX_BUF_SIZE;
    // memory is initialized to 0
    pBp3LogBinBuff = SRAI_Memory_Allocate(bp3LogBinSize, SRAI_MemoryType_Shared);
    if (pBp3LogBinBuff == NULL)
    {
        rc = -3;
        goto leave;
    }

    // allocate large enough buffer to hold bp3.bin
    bp3BinSize = BP3_HOST_MAX_BUF_SIZE;
    // initialize to 0
    pBp3BinBuff = SRAI_Memory_Allocate(bp3BinSize, SRAI_MemoryType_Shared);
    if (pBp3BinBuff == NULL)
    {
        rc = -1;
        goto leave;
    }
    BDBG_LOG(("pBp3BinBuff 0x%x",pBp3BinBuff));

    // Get bp3.bin file path/name
    filePathLength = strlen(bp3_bin_file_name);
    filePathLength += strlen(bp3_bin_file_path) + 2; // for '/' and trailing 0
    BDBG_MSG(("bp3.bin filepath/name length %d",filePathLength));

    bp3binFilePath = BKNI_Malloc(filePathLength);
    if (!bp3binFilePath)
    {
        BDBG_ERR((" can't allocate buffer for file path "));
        rc = -1;
        goto leave;
    }
    if (BKNI_Snprintf(bp3binFilePath, filePathLength, "%s/%s", bp3_bin_file_path, bp3_bin_file_name) != (int)(filePathLength-1))
    {
        BDBG_ERR(("%s - Cannot build final bp3.bin path", BSTD_FUNCTION));
        rc = -1;
        goto leave;
    }
    BDBG_LOG(("bp3bin filePath %s",bp3binFilePath)); // change to msg

    // read bp3.bin for in-field provisioning if it exists
    struct stat fileStat;
    if (stat(bp3binFilePath, &fileStat) == 0) {
        rc = SAGE_Read_File(bp3binFilePath, &pExistingBp3Bin, &existingBp3BinSize);
        if (rc !=0 || existingBp3BinSize == 0)
        {
            BDBG_ERR(("Error reading bp3.bin file"));
            goto leave;
        }
        // read existing bp3.bin - in-field provisioning
        BDBG_LOG (("in field provisioning"));
        memcpy(pBp3BinBuff, pExistingBp3Bin, existingBp3BinSize);
        SRAI_Memory_Free(pExistingBp3Bin);
    }
    else
    {
        // bp3.bin doesn't exist
        BDBG_LOG(("oem provisioning"));
    }

    ccfStatusSize = sizeof(uint32_t) * 5; // 4 byte status * max IP owners
    pCcfStatus = (uint32_t *)SRAI_Memory_Allocate(ccfStatusSize, SRAI_MemoryType_Shared);
    if (pCcfStatus == NULL)
    {
        BDBG_ERR(("failed to allocate buffer for BP3 CCF Block Status"));
        rc = -1;
        goto leave;
    }
    BDBG_LOG(("pEncryptedCcfBuff 0x%x, size %d",pEncryptedCcfBuff, bp3EncryptedCcfSize));
    rc = SAGE_BP3Module_Provision(
            pEncryptedCcfBuff,
            bp3EncryptedCcfSize,
            pBp3BinBuff,
            &bp3BinSize,
            existingBp3BinSize,
            pBp3LogBinBuff,
            &bp3LogBinSize,
            pCcfStatus,
            ccfStatusSize);

    if (bp3LogBinSize > 0)
    {
      // add padding size to log file
      bp3LogBinSize += 16 - (bp3LogBinSize % 16);
      // caller must free logBuf. For example, in bp3_host app, it's freed in hnd_get_log()
      *logBuf = (uint8_t*) malloc(sizeof(uint8_t) * bp3LogBinSize);
      if (*logBuf == NULL)
      {
        BDBG_ERR(("failed to allocate buffer for BP3 log file"));
      }
      else
      {
        memcpy(*logBuf, pBp3LogBinBuff, bp3LogBinSize);
        *logSize = bp3LogBinSize;
      }
    }

    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("BP3 provisioning failed"));
        goto leave;
    }

    if (binBuf != NULL)
    {
      *binBuf = (uint8_t*) malloc(bp3BinSize);
      if (*binBuf == NULL)
      {
        BDBG_ERR(("failed to allocate buffer for bp3.bin"));
      }
      else
      {
        memcpy(*binBuf, pBp3BinBuff, bp3BinSize);
        *binSize = bp3BinSize;
      }
    }
    SAGE_Write_BP3_Bin_File(&pBp3BinBuff, &bp3BinSize, bp3binFilePath);
leave:
    if (bp3binFilePath != NULL)
    {
        BKNI_Free(bp3binFilePath);
    }
    if (pCcfStatus)
    {
        if (status)
        {
          // caller must free *status. For example, in bp3_host app, it's freed in hnd_post_bp3()
          *status = (uint32_t*) malloc(ccfStatusSize);
          if (*status == NULL)
          {
            BDBG_ERR(("failed to allocate buffer for BP3 IP owner status"));
          }
          else
          {
            memcpy(*status, pCcfStatus, ccfStatusSize);
            if (statusSize) *statusSize = ccfStatusSize / sizeof(uint32_t);
          }
        }
        SRAI_Memory_Free((uint8_t *)pCcfStatus);
        pCcfStatus = NULL;
    }
    if (pEncryptedCcfBuff)
    {
        SRAI_Memory_Free(pEncryptedCcfBuff);
        pEncryptedCcfBuff = NULL;
    }
    if (pBp3BinBuff)
    {
        SRAI_Memory_Free(pBp3BinBuff);
        pBp3BinBuff = NULL;
    }
    if (pBp3LogBinBuff)
    {
        SRAI_Memory_Free(pBp3LogBinBuff);
        pBp3LogBinBuff = NULL;
    }
    if (pBp3SessionToken) {
      SRAI_Memory_Free(pBp3SessionToken);
      SAGE_BP3Platform_Uninit();
      pBp3SessionToken = NULL;
    }
    if (rc)
    {
        fprintf(stderr, "%s ERROR #%d\n", BSTD_FUNCTION, rc);
    }
    return rc;
}


int bp3(int argc, char *argv[])
{
    int       rc = 0;
    uint8_t* logBuff = NULL;
    uint32_t logSize = 0;
    /* Join Nexus: Initialize platform ... */
    rc = SAGE_app_join_nexus();
    if (rc)
      goto leave_nexus;
    rc = bp3_session_start(NULL, NULL);
    if (rc)
      goto leave;

    if (_parse_cmdline(argc, argv))
    {
        rc = -1;
        goto leave;
    }

    rc = bp3_session_end(NULL, 0, &logBuff, &logSize, NULL, NULL, NULL, NULL);
    if (rc)
      goto leave;

    SAGE_Write_Log_File(&logBuff, &logSize);

leave:
    /* Leave Nexus: Finalize platform ... */
    SAGE_app_leave_nexus();
leave_nexus:
    if (logBuff)
      free(logBuff);
    if (rc)
    {
        fprintf(stderr, "%s ERROR #%d\n", BSTD_FUNCTION, rc);
    }
    return rc;
}
