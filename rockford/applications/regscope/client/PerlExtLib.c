/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>

#if WIN32
#define bcmSnprintf                 sprintf_s
#define bcmStrdup                  _strdup
#else /* LINUX */
#define bcmSnprintf                 snprintf
#define bcmStrdup                   strdup
#endif

/* #define PERLEXTLIB_DEBUG */
#define               BUFFER_LENGTH 0x1000

/* Globals */
static int g_useSocket = 0;
static char *g_hostname;
static unsigned short g_portnum;
char g_achResult[BUFFER_LENGTH];


extern int bcmOpenSocket(const char * hostname, const unsigned short portnum);
extern int bcmCloseSocket( bool shutdown );
extern int bcmSocketReadRegister
    ( uint32_t  reg_addr,
      void     *pData,
      bool      is64Bit );
extern int bcmSocketWriteRegister
    ( uint32_t reg_addr,
      uint64_t data,
      bool     is64Bit );
extern int bcmSocketReadMemory
    ( uint64_t         mem_addr,
      void            *data,
      size_t           size );
extern int bcmSocketWriteMemory
    ( uint64_t         mem_addr,
      void            *data,
      size_t           size );
extern int bcmSocketCommand
    ( uint32_t  ulCmd,
      int32_t   ilSize,
      int32_t  *pilData );


const char * bcmUseSocket
    ( const char           *pchServerName,
      const unsigned short  usPortNum )
{
    g_useSocket = 1;
    g_hostname = (char *)pchServerName;
    g_portnum = usPortNum;

    return "1";
}

/***********************************************************************func*
 * bcmOpenDevice
 *
 * INPUT: none
 * FUNCTION: Open the device.
 * RETURN: NULL if failed to open the device, non-NULL to indicate success.
 ****************************************************************************/
const char* bcmOpenDevice
    ( void )
{
    if(g_useSocket)
    {
        printf("bcmOpenDevice: Using TCP/IP Socket.\n");
        return (bcmOpenSocket(g_hostname, g_portnum) == 0) ? "1" : NULL;
    }

    return NULL;
}

/***********************************************************************func*
 * bcmCloseDevice
 *
 * INPUT: none
 * FUNCTION: Close Device
 * RETURN: NULL if failed to close the device, non-NULL to indicate success.
 ****************************************************************************/
const char* bcmCloseDevice
    ( void )
{
    bool shutdown_server = false;
    if(g_useSocket)
    {
        return (bcmCloseSocket(shutdown_server)== 0) ? "1" : NULL;
    }
    return NULL;
}

static bool readRegister
    ( const uint32_t  ulAddr,
      uint32_t       *pulVal )
{
    bool ret = false;

    if (g_useSocket)
    {
        ret = (bcmSocketReadRegister(ulAddr, pulVal, false) == 0) ? true : false;
    }

    return ret;
}

static bool writeRegister
    ( const uint32_t ulAddr,
      const uint32_t ulData )
{
    bool ret = false;

    if (g_useSocket)
    {
        ret = (bcmSocketWriteRegister(ulAddr, ulData, false) == 0) ? true : false;
    }

    return ret;
}

static bool readRegister64
    ( const uint32_t  ulAddr,
      uint64_t  *pulVal )
{
    bool ret = false;

    if (g_useSocket)
    {
        ret = (bcmSocketReadRegister(ulAddr, pulVal, true) == 0) ? true : false;
    }

    return ret;
}


static bool writeRegister64
    ( const uint32_t      ulAddr,
      const uint64_t ulData )
{
    bool ret = false;

    if (g_useSocket)
    {
        ret = (bcmSocketWriteRegister(ulAddr, ulData, true) == 0) ? true : false;
    }

    return ret;
}

static const uint32_t readMemory
    ( const uint64_t  ulAddr,
      uint32_t            *pulData,
      const uint32_t       ulCount )
{
    if (g_useSocket)
    {
        return (bcmSocketReadMemory(ulAddr, (void *)pulData, ulCount<<2) == 0);
    }
    return 0;
}

static const uint32_t writeMemory
    ( const uint64_t  ulAddr,
      uint32_t       *pulData,
      const uint32_t  ulCount )
{
    if(g_useSocket)
    {
        return (bcmSocketWriteMemory(ulAddr, (void *)pulData, ulCount<<2) == 0);
    }
    return 0;
}


/************************************************************************func
 * Name:
 *   bcmRead() - read a register
 *
 * Input:
 *   a valid register address.
 *
 * Return:
 *   a register's dword if success, or NULL (undefined in perl).
 *
 * Description:
 *   read a register content of the given address.
 ****************************************************************************/
const char* bcmRead
    ( const uint32_t ulAddr )
{
    uint32_t ulValue = 0x00000000;

    if (readRegister(ulAddr, &ulValue))
    {
        bcmSnprintf(g_achResult, BUFFER_LENGTH, "%ld", ulValue);

        return g_achResult;
    }
    else
    {
        /* Call failed */
        return (const char*)NULL;
    }
}

/************************************************************************func
 * Name:
 *   bcmWrite() - write to a register
 *
 * Input:
 *   a valid register address, data.
 *
 * Return:
 *   true for success, and false (undefined in perl) for failed.
 *
 * Description:
 *   write a register content of the given address.
 ****************************************************************************/
const char* bcmWrite
    ( const uint32_t ulAddr,
      const uint32_t ulData )
{
    if (writeRegister(ulAddr, ulData))
    {
        return (const char*)"1";
    }
    else
    {
        return (const char*)NULL;
    }
}

/************************************************************************func
 * Name:
 *   bcmRead64() - read a 64-bit register
 *
 * Input:
 *   a valid register address.
 *
 * Return:
 *   a register's dword if success, or NULL (undefined in perl).
 *
 * Description:
 *   read a register content of the given address.
 ****************************************************************************/
const char* bcmRead64
    ( const uint32_t ulAddr )
{
    uint64_t ulValue = 0x00000000;

    if (readRegister64(ulAddr, &ulValue))
    {
#if WIN32
        /* Win32 Perl 5.14.2 can't handle 64-bit. Math::BigInt mangles it.
         Math::Int 64 isn't available in 5.14.2. So just convert to hex here. */
        bcmSnprintf(g_achResult, BUFFER_LENGTH, "%llx", ulValue);
#else
        bcmSnprintf(g_achResult, BUFFER_LENGTH, "%lld", ulValue);
#endif

        return g_achResult;
    }
    else
    {
        /* Call failed */
        return (const char*)NULL;
    }
}


#if WIN32
/************************************************************************func
 * bcmWrite64
 *
 * INPUT: a valid register address and data.
 *
 * FUNCTION: Single 64-bit register write
 * RETURN:  true for success, and false (undefined in perl) for failed.
****************************************************************************/
const char* bcmWrite64
    ( const unsigned long      ulAddr,
      const char *             pchData )
{
    unsigned long long ulData;
    ulData = strtoull(pchData, NULL, 16);

    if (writeRegister64(ulAddr, ulData))
    {
        return (const char*)"1";
    }
    else
    {
        return (const char*)NULL;
    }
}

/***********************************************************************func*
 * bcmReadMem
 *
 * INPUT: pchAddr - a string literal that represents an offset address of memory
 *                  Win 32 Perl 5.14.2 can't handle 64-bit. Math::BigInt mangles it.
 *                  Math::Int 64 isn't available in 5.14.2.
 *
 * FUNCTION: Single memory read
 * RETURN:  NULL if failed to read the device.  Otherwise return the
 *   32-bit value string in decimal.
 ****************************************************************************/
const char* bcmReadMem
    ( const char * pchAddr )
{
    unsigned long long ulAddr;
    unsigned long ulData;

    ulAddr = strtoull(pchAddr, NULL, 16);

    if (readMemory(ulAddr, &ulData, 1))
    {
        bcmSnprintf(g_achResult, BUFFER_LENGTH, "%u", ulData);

        return g_achResult;
    }
    return (const char*)NULL;
}

/***********************************************************************func*
 * bcmWriteMem
 *
 * INPUT: pchAddr  - a string literal that represents an offset address of memory
 *                  Win32 Perl 5.14.2 can't handle 64-bit. Math::BigInt mangles it.
 *                  Math::Int 64 isn't available in 5.14.2.
 *        ulData  - a 32-bit value to write to memroy
 *        ulCount - how many to fill
 * FUNCTION: Multiple memory write
 * RETURN:  NULL if failed to write the device.  Otherwise return the
 *   ulCount wrote.
 ****************************************************************************/
const char* bcmWriteMem
    ( const char          *pchAddr,
      unsigned long        ulData,
      const unsigned long  ulCount )
{
    unsigned long i            = 0;
    unsigned long long ulAddr;
    unsigned long *pulData     = NULL;
    int ret                    = 0;

    ulAddr = strtoull(pchAddr, NULL, 16);

    if(ulCount == 1)
    {
        if(!writeMemory(ulAddr, &ulData, ulCount))
        {
            return (const char*)NULL;
        }
    }
    else /* Create a chunk and fill it with 'ulData' and send it down to the driver. */
    {
        pulData = (unsigned long *)malloc(sizeof(unsigned long) * ulCount);
        for(i = 0; i < ulCount; i++)
        {
            pulData[i] = ulData;
        }

        ret = writeMemory(ulAddr, pulData, ulCount);
        free(pulData);

        if(ret==0)
        {
            return (const char*)NULL;
        }
    }

    bcmSnprintf(g_achResult, BUFFER_LENGTH, "%d", ulCount);
    return g_achResult;
}

/************************************************************************func
 * Name:
 *   bcmReadMemBlk() - read block of SDRAM and store into array.
 *
 * Input:
 *   pchAddr - a string literal that represents a valid SDRAM address
 *   pulData - an array to hold the read dwords
 *   ulCount - number of dword to read.
 *
 * Return:
 *   1 for success, and NULL for failed.
 *
 * Description:
 *   result are in pulData.  It returns number of SDRAM dwords read.
 ****************************************************************************/
const char* bcmReadMemBlk
    ( const char     *pchAddr,
      unsigned long  *pulData,
      unsigned long   ulCount )
{
    unsigned long i;
    unsigned long long ulAddr;
    unsigned long ulData;

    ulAddr = strtoull(pchAddr, NULL, 16);

    for (i=0; i<ulCount; i++)
    {
        if (readMemory(ulAddr, &ulData, 1))
        {
            pulData[i] = ulData;
        }
        else
        {
            return (const char*)NULL;
        }

        ulAddr += sizeof(uint32_t);
    }

    bcmSnprintf(g_achResult, BUFFER_LENGTH, "%d", i);
    return g_achResult;
}

/************************************************************************func
 * Name:
 *   bcmReadMemBlkToFile() - write to a block of SDRAM
 *
 * Input:
 *   pchAddr - a string literal that represents a valid SDRAM address
 *   pulData - an arry that holds the dwords to write
 *   ulCount - number of dword to read.
 *
 * Return:
 *   true for number of bytes wrote, or NULL for failure.
 *
 * Description:
 *   write the content of file to sdram (raw data).
 ****************************************************************************/
const char* bcmWriteMemBlk
    ( const char *        pchAddr,
      unsigned long *     pulData,
      const unsigned long ulCount )
{
    unsigned long long ulAddr;
    int ret = 0;

    ulAddr = strtoull(pchAddr, NULL, 16);

    ret = writeMemory(ulAddr, pulData, ulCount);
    if(ret)
    {
        return (const char*)NULL;
    }

    bcmSnprintf(g_achResult, BUFFER_LENGTH, "%d", ulCount);
    return g_achResult;

}

/************************************************************************func
 * Name:
 *   bcmReadMemBlkToFile() - read block of SDRAM and store into file.
 *
 * Input:
 *   ulAddr  - a valid SDRAM address (0 to 64Meg + whatever add on).
 *   ulCount - number of dwords to read.
 *   pchMemfile - a file to write the read dwords to
 *
 * Return:
 *   true for success, and false for failed.
 *
 * Description:
 *   result are in pulData.  It returns number of SDRAM dword read.
 ****************************************************************************/
const char* bcmReadMemBlkToFile
    ( const char          *pchAddr,
      const unsigned long  ulCount,
      const char          *pchMemfile )
{
    assert(pchMemfile);
    uint32_t i;
    FILE* fpMemfile = NULL;
    unsigned long ulData;
    unsigned long long ulAddr, ulTempAddr;

    ulAddr = strtoull(pchAddr, NULL, 16);

    if(fopen_s(&fpMemfile, pchMemfile, "wb")!=0)
    {
        fprintf(stderr, "Failed to open %s\n", pchMemfile);
        goto error;
    }

    if (fpMemfile)
    {
        for (i=0; i<ulCount; i++)
        {
            ulTempAddr = ulAddr + (i * sizeof(uint32_t));

            if (readMemory(ulTempAddr, &ulData, 1))
            {
                fwrite((const void*)&ulData, sizeof(uint32_t), 1, fpMemfile);
            }
        }

        fclose(fpMemfile);
        bcmSnprintf(g_achResult, BUFFER_LENGTH, "%d", i);
        return g_achResult;
    }
error:
    return (const char*)NULL;
}


/************************************************************************func
 * Name:
 *   bcmWriteMemBlkFromFile() - write to SDRAM with values from a file
 *
 * Input:
 *   ulAddr     - a valid SDRAM address (0 to 64Meg + whatever add on).
 *   pchMemfile - a binary file containing the dwords to write
 *
 * Return:
 *   true for number of bytes wrote, or NULL for failure.
 *
 * Description:
 *   write the content of file to sdram (raw data).
 ****************************************************************************/
const char* bcmWriteMemBlkFromFile
    ( const char *pchAddr,
      const char *pchMemfile )
{
    assert(pchMemfile);
    FILE* fpMemfile = NULL;
    unsigned long ulItemRead   = 0;
    unsigned long ulByteOffset = 0;
    unsigned long ulData;
    unsigned long long ulAddr;

    ulAddr = strtoull(pchAddr, NULL, 16);

    if(fopen_s(&fpMemfile, pchMemfile, "rb")!=0)
    {
        fprintf(stderr, "Failed to open %s\n", pchMemfile);
        goto error;
    }

    if (fpMemfile)
    {
        do
        {
            ulItemRead = fread((void*)&ulData,
                               sizeof(uint32_t),
                               1,
                               fpMemfile);

            if(ulItemRead > 0)
            {
                /* No longer can write or device is not ready
                 */
                if(!writeMemory(ulAddr + ulByteOffset, &ulData, 1))
                {
                    goto error;
                }

                ulByteOffset += (ulItemRead * sizeof(uint32_t));
            }
        } while(!feof(fpMemfile));
        fclose(fpMemfile);
        bcmSnprintf(g_achResult, BUFFER_LENGTH, "%d", ulByteOffset);
        return g_achResult;
    }
error:
    return (const char*)NULL;
}

/************************************************************************func
 * Name:
 *   bcmReadMemBlkToFileFormatted() - read block of SDRAM and store into file
 *                                    in memview format.
 *
 * Input:
 *   ulAddr  - a valid SDRAM address (0 to 64Meg + whatever add on).
 *   ulCount - number of dwords to read.
 *   pchMemfile - a file to write the read dwords to
 *
 * Return:
 *   true for success, and false for failed.
 *
 * Description:
 *   result are in pulData.  It returns number of SDRAM dword read.
 ****************************************************************************/
const char* bcmReadMemBlkToFileFormatted
    ( const char          *pchAddr,
      const unsigned long  ulCount,
      const char          *pchMemfile )
{
    assert(pchMemfile);
    uint32_t i;
    FILE* fpMemfile = NULL;
    unsigned long ulData;
    unsigned long long ulAddr, ulTempAddr;

    ulAddr = strtoull(pchAddr, NULL, 16);

    if(fopen_s(&fpMemfile, pchMemfile, "wb")!=0)
    {
        fprintf(stderr, "Failed to open %s\n", pchMemfile);
        goto error;
    }

    if (fpMemfile)
    {
        fprintf(fpMemfile, "// Memory\n");
        fprintf(fpMemfile, "%x\n", (ulCount & ~15UL));
        fprintf(fpMemfile, "@%x\n", 0);

        for (i=0; i<ulCount; i++)
        {
            ulTempAddr = ulAddr + (i * sizeof(uint32_t));

            if (readMemory(ulTempAddr, &ulData, 1))
            {
                fprintf(fpMemfile, "%x\n", ulData);
            }
        }
        fclose(fpMemfile);
        bcmSnprintf(g_achResult, BUFFER_LENGTH, "%d", i);
        return g_achResult;
    }
error:
    return (const char*)NULL;
}

/************************************************************************func
 * Name:
 *   bcmWriteMemBlkFromFileFormatted() - write to SDRAM with values from a file
 *                                       in memview format
 *
 * Input:
 *   ulAddr     - a valid SDRAM address (0 to 64Meg + whatever add on).
 *   pchMemfile - a binary file containing the dwords to write
 *
 * Return:
 *   true for number of bytes wrote, or NULL for failure.
 *
 * Description:
 *   write the content of file to sdram (raw data).
 ****************************************************************************/
const char* bcmWriteMemBlkFromFileFormatted
    ( const char *pchAddr,
      const char *pchMemfile )
{
    assert(pchMemfile);
    return (bcmWriteMemBlkFromFile(pchAddr, pchMemfile));
}


#else /* linux */
/************************************************************************func
/************************************************************************func
 * bcmWrite64
 *
 * INPUT: a valid register address and data.
 *
 * FUNCTION: Single 64-bit register write
 * RETURN:  true for success, and false (undefined in perl) for failed.
****************************************************************************/
const char* bcmWrite64
    ( const uint32_t ulAddr,
      const uint64_t ulData )
{
    if (writeRegister64(ulAddr, ulData))
    {
        return (const char*)"1";
    }
    else
    {
        return (const char*)NULL;
    }
}

/***********************************************************************func*
 * Name:
 *   bcmReadMem
 *
 * INPUT: ulAddr - an offset address of memory
 * FUNCTION: Single memory read
 * RETURN:  NULL if failed to read the device.  Otherwise return the
 *   32-bit value string in decimal.
 ****************************************************************************/
const char* bcmReadMem
    ( const uint64_t ulAddr )
{
    uint32_t ulData;

    if (readMemory(ulAddr, &ulData, 1))
    {
        bcmSnprintf(g_achResult, BUFFER_LENGTH, "%u", ulData);
        return g_achResult;
    }
    return (const char*)NULL;
}

/***********************************************************************func*
 * Name:
 *   bcmWriteMem
 *
 * INPUT: ulAddr  - an offset address of memory
 *        ulData  - a 32-bit value to write to memroy
 *        ulCount - how many to fill
 * FUNCTION: Multiple memory write
 * RETURN:  NULL if failed to write the device.  Otherwise return the
 *   ulCount wrote.
 ****************************************************************************/
const char* bcmWriteMem
    ( const uint64_t  ulAddr,
      uint32_t        ulData,
      const uint32_t  ulCount )
{
    uint32_t i        = 0;
    uint32_t *pulData = NULL;
    int ret                = 0;

    if(ulCount == 1)
    {
        if(!writeMemory(ulAddr, &ulData, ulCount))
        {
            return (const char*)NULL;
        }
    }
    else /* Create a chunk and fill it with 'ulData' and send it down to the driver. */
    {
        pulData = (uint32_t *)malloc(sizeof(uint32_t) * ulCount);
        for(i = 0; i < ulCount; i++)
        {
            pulData[i] = ulData;
        }

        ret = writeMemory(ulAddr, pulData, ulCount);
        free(pulData);

        if(ret==0)
        {
            return (const char*)NULL;
        }
    }

    bcmSnprintf(g_achResult, BUFFER_LENGTH, "%d", ulCount);
    return g_achResult;
}

/************************************************************************func
 * Name:
 *   bcmReadBlkMemToFile() - read block of SDRAM and store into file.
 *
 * Input:
 *   ulAddr  - a valid SDRAM address (0 to 64Meg + whatever add on).
 *   ulCount - number of dwords to read.
 *   pchMemfile - a file to write the read dwords to
 *
 * Return:
 *   true for success, and false for failed.
 *
 * Description:
 *   result are in pulData.  It returns number of SDRAM dword read.
 ****************************************************************************/
const char* bcmReadMemBlkToFile
    ( const uint64_t  ulAddr,
      const uint32_t  ulCount,
      const char     *pchMemfile )
{
    assert(pchMemfile);
    uint32_t i;
    FILE* fpMemfile = NULL;
    uint32_t ulData;
    uint64_t ulTempAddr;

    if (fpMemfile = fopen(pchMemfile, "wb"))
    {
        for (i=0; i<ulCount; i++)
        {
            ulTempAddr = ulAddr + (i * sizeof(uint32_t));

            if (readMemory(ulTempAddr, &ulData, 1))
            {
                fwrite((const void*)&ulData, sizeof(uint32_t), 1, fpMemfile);
            }
        }
        fclose(fpMemfile);
        sprintf(g_achResult, "%d", i);
        return g_achResult;
    }
    return (const char*)NULL;
}


/************************************************************************func
 * Name:
 *   bcmWriteMemBlkFromFile() - write to SDRAM using values from a file
 *
 * Input:
 *   ulAddr     - a valid SDRAM address (0 to 64Meg + whatever add on).
 *   pchMemfile - a binary file containing the dwords to write
 *
 * Return:
 *   true for number of bytes wrote, or NULL for failure.
 *
 * Description:
 *   write the content of file to sdram (raw data).
 ****************************************************************************/
const char* bcmWriteMemBlkFromFile
    ( const uint64_t  ulAddr,
      const char     *pchMemfile)
{
    assert(pchMemfile);
    FILE* fpMemfile = NULL;
    uint32_t ulItemRead   = 0;
    uint32_t ulByteOffset = 0;
    uint32_t ulData;

    if(NULL != (fpMemfile = fopen(pchMemfile, "rb")))
    {
        do
        {
            ulItemRead = fread((void*)&ulData,
                               sizeof(uint32_t),
                               1,
                               fpMemfile);

            if(ulItemRead > 0)
            {
                /* No longer can write or device is not ready
                 */
                if(!writeMemory(ulAddr + ulByteOffset, &ulData, 1))
                {
                    goto error;
                }

                ulByteOffset += (ulItemRead * sizeof(uint32_t));
            }
        } while(!feof(fpMemfile));
        fclose(fpMemfile);
        sprintf(g_achResult, "%d", ulByteOffset);
        return g_achResult;
    }
error:
    return (const char*)NULL;
}

/************************************************************************func
 * Name:
 *   bcmReadMemBlkToFileFormatted() - read block of SDRAM and store into file
 *                                    in memview format.
 *
 * Input:
 *   ulAddr  - a valid SDRAM address (0 to 64Meg + whatever add on).
 *   ulCount - number of dwords to read.
 *   pchMemfile - the file to write the read dwords to
 *
 * Return:
 *   true for success, and false for failed.
 *
 * Description:
 *   result are in pulData.  It returns number of SDRAM dword read.
 ****************************************************************************/
const char* bcmReadMemBlkToFileFormatted
    ( const uint64_t  ulAddr,
      const uint32_t  ulCount,
      const char     *pchMemfile )
{
    assert(pchMemfile);
    uint32_t i;
    FILE* fpMemfile = NULL;
    uint32_t ulData, ulTempAddr = ulAddr;

    if(NULL != (fpMemfile = fopen(pchMemfile, "wb")))
    {
        fprintf(fpMemfile, "// Memory\n");
        fprintf(fpMemfile, "%x\n", (ulCount & ~15UL));
        fprintf(fpMemfile, "@%x\n", 0);

        for (i=0; i<ulCount; i++)
        {
            ulTempAddr = ulAddr + (i * sizeof(uint32_t));

            if (readMemory(ulTempAddr, &ulData, 1))
            {
                fprintf(fpMemfile, "%x\n", ulData);
            }
        }
        fclose(fpMemfile);
        sprintf(g_achResult, "%d", i);
        return g_achResult;
    }
    return (const char*)NULL;
}

/************************************************************************func
 * Name:
 *   bcmWriteMemBlkFromFileFormatted() - write to SDRAM with values from a file
 *                                       in memview format
 *
 * Input:
 *   ulAddr     - a valid SDRAM address (0 to 64Meg + whatever add on).
 *   pchMemfile - a binary file containing the dwords to write
 *
 * Return:
 *   true for number of bytes wrote, or NULL for failure.
 *
 * Description:
 *   write the content of file to sdram (raw data).
 ****************************************************************************/
const char* bcmWriteMemBlkFromFileFormatted
    ( const uint64_t  ulAddr,
      const char     *pchMemfile)
{
    assert(pchMemfile);
    return (bcmWriteMemBlkFromFile(ulAddr, pchMemfile));
}

#endif


/* Deprecated legacy functions */
const char* bcmReadI2C(
    const unsigned char uchChipAddr,
    const unsigned char uchSubAddr)
{
    fprintf(stderr, "I2C interface not available.\n");
    return NULL;
}

const char* bcmWriteI2C(
    const unsigned char uchChipAddr,
    const unsigned char uchSubAddr,
    const unsigned char uchData)
{
    fprintf(stderr, "I2C interface not available.\n");
    return NULL;
}

const char * bcmUseI2CParallel(
    const char * pchDeviceName,
    const unsigned short usPortAddr,
    const unsigned short usSlaveAddr,
    const unsigned char uchSpeed,
    const uint32_t ulUseCOM)
{
    fprintf(stderr, "I2C interface not available.\n");
    return NULL;
}

const char * bcmRunTCS(
    const char * pchFileName,
    const uint32_t ulRegOffset)
{
    fprintf(stderr, "TCS is not available.\n");
    return NULL;
}

/* End of file */
