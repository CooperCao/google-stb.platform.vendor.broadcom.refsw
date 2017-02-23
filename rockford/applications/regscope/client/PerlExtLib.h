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
#ifndef __PERLEXTLIB_H__
#define __PERLEXTLIB_H__

#include <stdbool.h>

#if WIN32
#define PEL_WIN_API __declspec(dllexport)
#else
#define PEL_WIN_API
#endif

PEL_WIN_API const char * bcmUseSocket
    ( const char * pchServerName,
      const unsigned short usPortNum );

PEL_WIN_API const char *bcmOpenDevice
    ( void );

PEL_WIN_API const char *bcmCloseDevice
    ( void );

PEL_WIN_API const char *bcmRead
    ( unsigned long ulAddr );

PEL_WIN_API const char *bcmWrite
    ( const unsigned long ulAddr,
      const unsigned long ulData );

PEL_WIN_API const char* bcmRead64
    ( const unsigned long ulAddr );

#if WIN32

PEL_WIN_API const char* bcmWrite64
    ( const unsigned long  ulAddr,
      const char          *pchData );

PEL_WIN_API const char* bcmReadMem
    ( const char          *pchAddr );

PEL_WIN_API const char* bcmWriteMem
    ( const char          *pchAddr,
      unsigned long        ulData,
      const unsigned long  ulCount );

PEL_WIN_API const char* bcmReadMemBlk
    ( const char          *pchAddr,
      unsigned long       *pulData,
      unsigned long        ulCount );

PEL_WIN_API const char* bcmWriteMemBlk
    ( const char          *pchAddr,
      unsigned long       *pulData,
      unsigned long        ulCount );

PEL_WIN_API const char* bcmReadMemBlkToFile
    ( const char          *pchAddr,
      const unsigned long  ulCount,
      const char          *pchMemfile );

PEL_WIN_API const char* bcmWriteMemBlkFromFile
    ( const char          *pchAddr,
      const char          *pchMemfile );

PEL_WIN_API const char* bcmReadMemBlkToFileFormatted
    ( const char          *pchAddr,
      const unsigned long  ulCount,
      const char          *pchMemfile );

PEL_WIN_API const char* bcmWriteMemBlkFromFileFormatted
    ( const char          *pchAddr,
      const char          *pchMemfile );

#else /* linux */

PEL_WIN_API const char* bcmWrite64
    ( const unsigned long      ulAddr,
      const unsigned long long ulData );

PEL_WIN_API const char* bcmReadMem
    ( const unsigned long long ulAddr );

PEL_WIN_API const char* bcmWriteMem
    ( const unsigned long long ulAddr,
      unsigned long            ulData,
      const unsigned long      ulCount );

PEL_WIN_API const char* bcmReadMemBlkToFile
    ( const unsigned long long ulAddr,
      const unsigned long      ulCount,
      const char              *pchMemfile );

PEL_WIN_API const char* bcmWriteMemBlkFromFile
    ( const unsigned long long ulAddr,
      const char              *pchMemfile);

PEL_WIN_API const char* bcmReadMemBlkToFileFormatted
    ( const unsigned long long ulAddr,
      const unsigned long      ulCount,
      const char              *pchMemfile );

PEL_WIN_API const char* bcmWriteMemBlkFromFileFormatted
    ( const unsigned long long ulAddr,
      const char              *pchMemfile);

#endif

PEL_WIN_API const char* bcmReadI2C
    ( const unsigned char uchChipAddr,
      const unsigned char uchSubAddr );

PEL_WIN_API const char* bcmWriteI2C
    ( const unsigned char uchChipAddr,
      const unsigned char uchSubAddr,
      const unsigned char uchData );

PEL_WIN_API const char * bcmUseI2CParallel
    ( const char * pchDeviceName,
      const unsigned short usPortAddr,
      const unsigned short usSlaveAddr,
      const unsigned char uchSpeed,
      const unsigned long ulUseCOM );

PEL_WIN_API const char * bcmRunTCS
    ( const char * pchFileName,
      const unsigned long ulRegOffset );

#endif /* __PERLEXTLIB_H__ */
