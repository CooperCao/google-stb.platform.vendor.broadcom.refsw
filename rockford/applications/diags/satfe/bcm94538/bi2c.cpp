/******************************************************************************* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#include "bstd.h"
#include "breg_i2c_priv.h"
#include "bi2c.h"
#include "bi2c_priv.h"
#include "bkni_multi.h"
#include <windows.h>
#include <conio.h>
#include "stdafx.h"


#define BI2C_P_ACQUIRE_MUTEX(handle) BKNI_AcquireMutex((handle)->hMutex)
#define BI2C_P_RELEASE_MUTEX(handle) BKNI_ReleaseMutex((handle)->hMutex)


#define B_I2C_MSB_FIX 0

BDBG_MODULE(bi2c);

#define   DEV_MAGIC_ID         ((BERR_I2C_ID<<16) | 0xFACE)

#define   BI2C_CHK_RETCODE( rc, func )      \
do {                              \
   if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
   {                              \
      goto done;                     \
   }                              \
} while(0)

#define I2C_SDA         0x08
#define I2C_SCL         0x80
#define I2C_WR_TRIG     0x20
#define I2C_RD_TRIG     0x10
#define I2C_WAIT(x)

extern SATFE_Diags_Config g_SATFE_Config;


/*******************************************************************************
*
*   Private Module Handles
*
*******************************************************************************/

typedef struct BI2C_P_Handle
{
   uint32_t      magicId;         /* Used to check if structure is corrupt */
   unsigned int    maxChnNo;
   BI2C_ChannelHandle hI2cChn[BI2C_MAX_I2C_CHANNELS];
} BI2C_P_Handle;

typedef struct BI2C_P_ChannelHandle
{
   uint32_t         magicId;               /* Used to check if structure is corrupt */
   BI2C_Handle      hI2c;
   unsigned int     chnNo;
   bool             noAck;
   CRITICAL_SECTION cs;
   WORD             dataAddr;
   WORD             statusAddr;
   WORD             controlAddr;

} BI2C_P_ChannelHandle;



/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/
static const BI2C_Settings defI2cSettings =
{
   NULL
};

static const BI2C_ChannelSettings defI2cXChnSettings[] =
{
   {   /* Channel A */
      BI2C_Clk_eClk400Khz,
      true
   }
#if (BI2C_MAX_I2C_CHANNELS == 2) || (BI2C_MAX_I2C_CHANNELS == 3) || (BI2C_MAX_I2C_CHANNELS == 4)
   ,{   /* Channel B */
      BI2C_Clk_eClk400Khz,
      true
   }
#endif
#if (MAX_MSTR_CHANNELS == 3) || (BI2C_MAX_I2C_CHANNELS == 4)
   ,{   /* Channel C */
      BI2C_Clk_eClk400Khz,
      true
   }
#endif
#if (BI2C_MAX_I2C_CHANNELS == 4)
   ,{   /* Channel D */
      BI2C_Clk_eClk400Khz,
      true
   }
#endif
};


/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/
BERR_Code BI2C_Open(
   BI2C_Handle *pI2c,               /* [output] Returns handle */
   BCHP_Handle hChip,               /* Chip handle */
   BREG_Handle hRegister,            /* Register handle */
   BINT_Handle hInterrupt,            /* Interrupt handle */
   const BI2C_Settings *pDefSettings   /* Default settings */
   )
{
   BERR_Code retCode = BERR_SUCCESS;
   BI2C_Handle hDev;
   unsigned int chnIdx;
   HANDLE h = NULL;
   char strPathName[MAX_PATH], strDir[MAX_PATH], szArg[512];
   int i, nPos, len;

   /* Sanity check on the handles we've been given. */
   BSTD_UNUSED( hChip );
   BSTD_UNUSED( hInterrupt );
   BSTD_UNUSED( pDefSettings );
   BSTD_UNUSED( hRegister );

   h = CreateFile( "\\\\.\\DirectIO", GENERIC_READ, 0, NULL,
                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
   if ( h == INVALID_HANDLE_VALUE )
   {
      // Parallel port not installed, so install it

      // Get the directory of the exe this dll is loaded in
      GetModuleFileName( NULL, strPathName, MAX_PATH );

	  strcpy(strDir, strPathName);
	  len = strlen(strPathName);
	  for (i=len; i>=0; i--)
		if (strPathName[i] == '\\')
		{
			nPos = i;
			strncpy(strDir, strPathName, i);
			strDir[i] = '\0';
			break;
		}

      // Make the shell command arg
      wsprintf( szArg, "DirectIO \"%s\\DirectIO.sys\"", strDir );

      HINSTANCE hInst = ShellExecute( NULL, "open", "instdrv",
                              szArg,   strDir,  SW_HIDE  );

      if ((long)hInst > 32)
      {
         // Succeeded so try CreateFile again
         DWORD dwRetval = WaitForSingleObject( hInst, 10000 );
         h = NULL;
         short nCount = 0;
         while ( (h == INVALID_HANDLE_VALUE || h == NULL) && nCount < 10 )
         {
            h = CreateFile("\\\\.\\DirectIO", GENERIC_READ, 0, NULL,
               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            Sleep(500);
            nCount++;
         }
         if ( nCount == 10 )
            retCode = BERR_OS_ERROR;
      }
      else // ShellExecute Failed
         retCode = BERR_OS_ERROR;
   }
   if (retCode)
      goto done;

   /* Alloc memory from the system heap */
   hDev = (BI2C_Handle) BKNI_Malloc( sizeof( BI2C_P_Handle ) );
   if( hDev == NULL )
   {
      *pI2c = NULL;
      retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
      BDBG_ERR(("BI2C_Open: BKNI_malloc() failed\n"));
      goto done;
   }

   hDev->magicId   = DEV_MAGIC_ID;
   hDev->maxChnNo   = BI2C_MAX_I2C_CHANNELS;
   for( chnIdx = 0; chnIdx < hDev->maxChnNo; chnIdx++ )
   {
      hDev->hI2cChn[chnIdx] = NULL;
   }

   *pI2c = hDev;

done:
   return( retCode );
}


BERR_Code BI2C_Close(
   BI2C_Handle hDev               /* Device handle */
   )
{
   BERR_Code retCode = BERR_SUCCESS;


   BDBG_ASSERT( hDev );
   BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

   BKNI_Free( (void *) hDev );

   return( retCode );
}


BERR_Code BI2C_GetDefaultSettings(
   BI2C_Settings *pDefSettings,      /* [output] Returns default setting */
   BCHP_Handle hChip               /* Chip handle */
   )
{
   BERR_Code retCode = BERR_SUCCESS;

   BSTD_UNUSED( hChip );

   *pDefSettings = defI2cSettings;

   return( retCode );
}


BERR_Code BI2C_GetTotalChannels(
   BI2C_Handle hDev,               /* Device handle */
   unsigned int *totalChannels         /* [output] Returns total number downstream channels supported */
   )
{
   BERR_Code retCode = BERR_SUCCESS;


   BDBG_ASSERT( hDev );
   BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

   *totalChannels = hDev->maxChnNo;

   return( retCode );
}


BERR_Code BI2C_GetChannelDefaultSettings(
   BI2C_Handle hDev,               /* Device handle */
   unsigned int channelNo,            /* Channel number to default setting for */
    BI2C_ChannelSettings *pChnDefSettings /* [output] Returns channel default setting */
    )
{
   BERR_Code retCode = BERR_SUCCESS;


   BDBG_ASSERT( hDev );
   BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

   if( channelNo < BI2C_MAX_I2C_CHANNELS )
   {
      *pChnDefSettings = defI2cXChnSettings[channelNo];
   }
   else
   {
      retCode = BERR_TRACE (BERR_INVALID_PARAMETER);
   }

   return( retCode );
}


BERR_Code BI2C_OpenChannel(
   BI2C_Handle hDev,               /* Device handle */
   BI2C_ChannelHandle *phChn,         /* [output] Returns channel handle */
   unsigned int channelNo,            /* Channel number to open */
   const BI2C_ChannelSettings *pChnDefSettings /* Channel default setting */
   )
{
   BERR_Code          retCode = BERR_SUCCESS;
   BI2C_ChannelHandle   hChnDev;
   HANDLE h = NULL;
   BYTE x;

   BDBG_ASSERT( hDev );
   BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

   hChnDev = NULL;
   if( channelNo < hDev->maxChnNo )
   {
      if( hDev->hI2cChn[channelNo] == NULL )
      {
         /* Alloc memory from the system heap */
         hChnDev = (BI2C_ChannelHandle) BKNI_Malloc( sizeof( BI2C_P_ChannelHandle ) );
         if( hChnDev == NULL )
         {
            *phChn = NULL;
            retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            BDBG_ERR(("BI2C_OpenChannel: BKNI_malloc() failed\n"));
            goto done;
         }

         InitializeCriticalSection(&(hChnDev->cs));
         hChnDev->magicId = DEV_MAGIC_ID;
         hChnDev->hI2c = hDev;
         hChnDev->chnNo = channelNo;
         hDev->hI2cChn[channelNo] = hChnDev;
         *phChn = hChnDev;

         hChnDev->dataAddr = g_SATFE_Config.lpt_addr;
         hChnDev->statusAddr = g_SATFE_Config.lpt_addr+ 1;
         hChnDev->controlAddr = g_SATFE_Config.lpt_addr+ 2;

         /* initialize the parallel port */
         _outp(hChnDev->controlAddr,0x08);
         _outp(hChnDev->controlAddr,0x0e);
         _outp(hChnDev->dataAddr, 0);
         _outp(hChnDev->controlAddr, I2C_SDA);


         /* enable parallel port irq */
         x = (uint8_t)_inp(hChnDev->controlAddr);
         x |= 0x10;
         _outp(hChnDev->controlAddr, x);
      }
      else
      {
         retCode = BERR_TRACE (BI2C_ERR_NOTAVAIL_CHN_NO);
      }
   }
   else
   {
      retCode = BERR_TRACE (BERR_INVALID_PARAMETER);
   }

done:
   return( retCode );
}

BERR_Code BI2C_CloseChannel(
   BI2C_ChannelHandle hChn         /* Device channel handle */
   )
{
   BERR_Code retCode = BERR_SUCCESS;
   BI2C_Handle hDev;
   unsigned int chnNo;


   BDBG_ASSERT( hChn );
   BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

   hDev = hChn->hI2c;
   chnNo = hChn->chnNo;
   BKNI_Free( hChn );
   hDev->hI2cChn[chnNo] = NULL;

   return( retCode );
}

BERR_Code BI2C_GetDevice(
   BI2C_ChannelHandle hChn,         /* Device channel handle */
   BI2C_Handle *phDev               /* [output] Returns Device handle */
   )
{
   BERR_Code retCode = BERR_SUCCESS;


   BDBG_ASSERT( hChn );
   BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

   *phDev = hChn->hI2c;

   return( retCode );
}


BERR_Code BI2C_CreateI2cRegHandle(
   BI2C_ChannelHandle hChn,         /* Device channel handle */
   BREG_I2C_Handle *pI2cReg         /* [output]  */
   )
{
   BERR_Code retCode = BERR_SUCCESS;


   BDBG_ASSERT( hChn );
   BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );


   *pI2cReg = (BREG_I2C_Handle)BKNI_Malloc( sizeof(BREG_I2C_Impl) );
   if( *pI2cReg == NULL )
   {
      retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
      BDBG_ERR(("BI2C_CreateI2cRegHandle: BKNI_malloc() failed"));
      goto done;
   }

   if (hChn->hI2c->hI2cChn[0] == NULL)
   {
      retCode = BERR_TRACE(BI2C_ERR_SINGLE_MSTR_CREATE);
      BDBG_ERR(("BI2C_CreateI2cRegHandle: For Single I2C master core, Channel 0 must be created"));
      goto done;
   }

   (*pI2cReg)->context                  = (void *)hChn;
   (*pI2cReg)->BREG_I2C_Write_Func       = BI2C_P_Write;
   (*pI2cReg)->BREG_I2C_WriteNoAck_Func    = BI2C_P_WriteNoAck;
   (*pI2cReg)->BREG_I2C_WriteA16_Func      = BI2C_P_WriteA16;
   (*pI2cReg)->BREG_I2C_WriteA24_Func      = BI2C_P_WriteA24;
   (*pI2cReg)->BREG_I2C_WriteNoAddr_Func   = BI2C_P_WriteNoAddr;
   (*pI2cReg)->BREG_I2C_WriteNoAddrNoAck_Func   = BI2C_P_WriteNoAddrNoAck;
   (*pI2cReg)->BREG_I2C_WriteNvram_Func   = BI2C_P_WriteNvram;
   (*pI2cReg)->BREG_I2C_Read_Func          = BI2C_P_Read;
   (*pI2cReg)->BREG_I2C_ReadNoAck_Func      = BI2C_P_ReadNoAck;
   (*pI2cReg)->BREG_I2C_ReadA16_Func      = BI2C_P_ReadA16;
   (*pI2cReg)->BREG_I2C_ReadA24_Func      = BI2C_P_ReadA24;
   (*pI2cReg)->BREG_I2C_ReadNoAddr_Func   = BI2C_P_ReadNoAddr;
   (*pI2cReg)->BREG_I2C_ReadEDDC_Func      = BI2C_P_ReadEDDC;
   (*pI2cReg)->BREG_I2C_WriteEDDC_Func      = BI2C_P_WriteEDDC;

done:
   return( retCode );
}

BERR_Code BI2C_CloseI2cRegHandle(
   BREG_I2C_Handle      hI2cReg
   )
{
   BERR_Code retCode = BERR_SUCCESS;

   BDBG_ASSERT( hI2cReg );
   BKNI_Free( (void *) hI2cReg );

   return( retCode );
}


void BI2C_SetClk(
   BI2C_ChannelHandle    hChn,         /* Device channel handle */
   BI2C_Clk         clkRate         /* pointer to clock rate setting */
   )
{
   BDBG_ASSERT( hChn );
   BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
}


BI2C_Clk BI2C_GetClk(
   BI2C_ChannelHandle    hChn         /* Device channel handle */
)
{
   BDBG_ASSERT( hChn );
   BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

   return (BI2C_Clk)0;
}


/*******************************************************************************
*
*   Private Module Functions
*
*******************************************************************************/


BERR_Code BI2C_P_ReadNoAck
(
   void *context,            /* Device channel handle */
   uint16_t chipAddr,               /* chip address */
   uint32_t subAddr,               /* 8-bit sub address */
   uint8_t *pData,            /* pointer to memory location to store read data  */
   size_t length                  /* number of bytes to read */
)
{
   return BERR_NOT_SUPPORTED; //BI2C_P_ReadCmdNoAck ((BI2C_ChannelHandle)context, chipAddr, (void *)&subAddr, 1, pData, length);
}

BERR_Code BI2C_P_ReadA16
(
   void *context,            /* Device channel handle */
   uint16_t chipAddr,               /* chip address */
   uint32_t subAddr,               /* 16-bit sub address */
   uint8_t *pData,            /* pointer to memory location to store read data  */
   size_t length                  /* number of bytes to read */
)
{
   return BERR_NOT_SUPPORTED; //BI2C_P_ReadCmd ((BI2C_ChannelHandle)context, chipAddr, (void *)&subAddr, 2, pData, length);
}

BERR_Code BI2C_P_ReadA24
(
   void *context,            /* Device channel handle */
   uint16_t chipAddr,               /* chip address */
   uint32_t subAddr,               /* 24-bit sub address */
   uint8_t *pData,            /* pointer to memory location to store read data  */
   size_t length                  /* number of bytes to read */
)
{
   return BERR_NOT_SUPPORTED; //BI2C_P_ReadCmd ((BI2C_ChannelHandle)context, chipAddr, (void *)&subAddr, 3, pData, length);
}


BERR_Code BI2C_P_WriteNoAck
(
   void *context,            /* Device channel handle */
   uint16_t chipAddr,               /* chip address */
   uint32_t subAddr,               /* 8-bit sub address */
   const uint8_t *pData,            /* pointer to data to write */
   size_t length                  /* number of bytes to write */
)
{
   return BERR_NOT_SUPPORTED; //BI2C_P_WriteCmdNoAck ((BI2C_ChannelHandle)context, chipAddr, (void *)&subAddr, 1, pData, length);
}

BERR_Code BI2C_P_WriteA16
(
   void *context,            /* Device channel handle */
   uint16_t chipAddr,               /* chip address */
   uint32_t subAddr,               /* 16-bit sub address */
   const uint8_t *pData,            /* pointer to data to write */
   size_t length                  /* number of bytes to write */
)
{
   return BERR_NOT_SUPPORTED; //BI2C_P_WriteCmd ((BI2C_ChannelHandle)context, chipAddr, (void *)&subAddr, 2, pData, length, NVRAM_NO);
}

BERR_Code BI2C_P_WriteA24
(
   void *context,            /* Device channel handle */
   uint16_t chipAddr,               /* chip address */
   uint32_t subAddr,               /* 24-bit sub address */
   const uint8_t *pData,            /* pointer to data to write */
   size_t length                  /* number of bytes to write */
)
{
   return BERR_NOT_SUPPORTED; //BI2C_P_WriteCmd ((BI2C_ChannelHandle)context, chipAddr, (void *)&subAddr, 3, pData, length, NVRAM_NO);
}

BERR_Code BI2C_P_WriteNoAddrNoAck
(
   void *context,            /* Device channel handle */
   uint16_t chipAddr,               /* chip address */
   const uint8_t *pData,            /* pointer to data to write */
   size_t length                  /* number of bytes to write */
)
{
   return BERR_NOT_SUPPORTED; //BI2C_P_WriteCmdNoAck ((BI2C_ChannelHandle)context, chipAddr, (void *)NULL, 0, pData, length);
}

BERR_Code BI2C_P_WriteNvram
(
   void *context,            /* Device channel handle */
   uint16_t chipAddr,               /* chip address */
   uint32_t subAddr,               /* 8-bit sub address */
   const uint8_t *pData,            /* pointer to data to write */
   size_t length                  /* number of bytes to write */
)
{
   return BERR_NOT_SUPPORTED; //BI2C_P_WriteCmd ((BI2C_ChannelHandle)context, chipAddr, (void *)&subAddr, 1, pData, length, NVRAM_YES);
}

BERR_Code BI2C_P_ReadEDDC(
   void             *context,            /* Device channel handle */
   uint8_t            chipAddr,            /* chip address */
   uint8_t            segment,            /* EDDC segment */
   uint32_t            subAddr,            /* 8-bit sub address */
   uint8_t            *pData,               /* pointer to memory location to store read data */
   size_t            length               /* number of bytes to read */
   )
{
   return BERR_NOT_SUPPORTED;
}

BERR_Code BI2C_P_WriteEDDC(
   void             *context,      /* Device channel handle */
   uint8_t            chipAddr,      /* chip address */
   uint8_t            segment,      /* EDDC segment */
   uint32_t            subAddr,      /* 8-bit sub address */
   const uint8_t      *pData,         /* pointer to data to write */
   size_t            length         /* number of bytes to read */
   )
{
   return BERR_NOT_SUPPORTED;
}


BERR_Code BI2C_P_Read
(
   void *context,            /* Device channel handle */
   uint16_t chipAddr,               /* chip address */
   uint32_t subAddr,               /* 8-bit sub address */
   uint8_t *pData,            /* pointer to memory location to store read data  */
   size_t length                  /* number of bytes to read */
)
{
   BI2C_ChannelHandle h = (BI2C_ChannelHandle)context;
   uint32_t i, n, mask;
   BYTE send_data, rcv_data, b, no_ack = 0;

   EnterCriticalSection(&(h->cs));

   /* generate the start bit */
   _outp(h->dataAddr, I2C_RD_TRIG);   /* set wr trig  */
   I2C_WAIT(_outp(h->dataAddr, I2C_RD_TRIG))
   _outp(h->controlAddr, 0);          /* set sda lo */
   I2C_WAIT(_outp(h->controlAddr, 0))
   _outp(h->dataAddr, I2C_SCL);       /* set scl lo */
   I2C_WAIT(_outp(h->dataAddr, I2C_SCL))

   no_ack = 0;
   for (n = 0; !no_ack && (n <= 1); n++)
   {
      if (n)
         send_data = (uint8_t)subAddr;
      else
         send_data = (uint8_t)((chipAddr << 1) & 0xFE);

      /* transmit the data */
      mask = 0x80;
      for (i = 0; i < 8; i++)
      {
         _outp(h->controlAddr, (mask & send_data) ? I2C_SDA : 0);  // set sda bit
         I2C_WAIT(_outp(h->controlAddr, (mask & send_data) ? I2C_SDA : 0))
         _outp(h->dataAddr, 0);                                  // set scl hi
         I2C_WAIT(_outp(h->dataAddr, 0))
         _outp(h->dataAddr, I2C_SCL);                              // set scl lo
         I2C_WAIT(_outp(h->dataAddr, I2C_SCL))
         mask = (mask >> 1);
      }

      /* wait for internal registers to load data before reading */

      /* generate the acknowledge bit */
      _outp(h->controlAddr, I2C_SDA);
      I2C_WAIT(_outp(h->controlAddr, I2C_SDA))
      _outp(h->dataAddr, 0);            // set scl hi
      I2C_WAIT(_outp(h->dataAddr, 0))
      b = (BYTE)_inp(h->statusAddr);
      if (b & I2C_SDA)
      {
         no_ack=1;  // verify ack
      }
      _outp(h->dataAddr, I2C_SCL);      // set scl lo
      I2C_WAIT(_outp(h->m_DataAddr, I2C_SCL))
   }

   if (no_ack)
      goto done;

   /* generate a stop/start (or repeated start) */
#ifdef NO_REPEAT_START
   _outp(h->controlAddr, 0);         // sda lo
   I2C_WAIT(_outp(h->controlAddr, 0))
   _outp(h->dataAddr, 0);            // scl hi
   I2C_WAIT(_outp(h->dataAddr, 0))
   _outp(h->controlAddr, I2C_SDA);   // sda hi
   I2C_WAIT(_outp(h->controlAddr, I2C_SDA))
   _outp(h->controlAddr, 0);         // sda lo
   I2C_WAIT(_outp(h->controlAddr, 0))
   _outp(h->dataAddr, I2C_SCL);      // scl lo
   I2C_WAIT(_outp(h->dataAddr, I2C_SCL))
#else
   _outp(h->controlAddr, I2C_SDA);   // sda hi
   I2C_WAIT(_outp(h->controlAddr, I2C_SDA))
   _outp(h->dataAddr, 0);            // scl hi
   I2C_WAIT(_outp(h->dataAddr, 0))
   _outp(h->controlAddr, 0);         // sda lo
   I2C_WAIT(_outp(h->controlAddr, 0))
   _outp(h->dataAddr, I2C_SCL);      // scl lo
   I2C_WAIT(_outp(h->dataAddr, I2C_SCL))
#endif

   /* read the requested data */
   for (n = 0 ; n <= length; n++)
   {
      if (n == 0)
         send_data = (chipAddr << 1) | 0x01;
      else
         send_data = 0xff;

      /* transmit the data */
      rcv_data = 0;
      mask     = 0x80;
      for (i=0; i<8; i++)
      {
         _outp(h->controlAddr, (mask & send_data) ? I2C_SDA : 0);  // set sda bit
         I2C_WAIT(_outp(h->controlAddr, (mask & send_data) ? I2C_SDA : 0))
         _outp(h->dataAddr, 0);                                    // set scl hi
         I2C_WAIT(_outp(h->dataAddr, 0))
         if (n > 0) // sample data
         {
            b = (BYTE)_inp(h->statusAddr);
            if (b & I2C_SDA)
               rcv_data |= mask;
         }
         _outp(h->dataAddr, I2C_SCL);                              // set SCL lo
         I2C_WAIT(_outp(h->dataAddr, I2C_SCL))
         mask = (mask >> 1);
      }

      /* generate the acknowledge bit */
      if ((n == 0) || (n == length))
      {
         _outp(h->controlAddr, I2C_SDA);  // sda hiz
         I2C_WAIT(_outp(h->controlAddr, I2C_SDA))
      }
      else
      {
         _outp(h->controlAddr, 0);        // sda lo
         I2C_WAIT(_outp(h->controlAddr, 0))
      }

      _outp(h->dataAddr, 0);              // scl hi
      I2C_WAIT(_outp(h->dataAddr, 0))

      /* wait for internal registers to load data before reading */
      //Sleep(m_BusDelay);

      b = (BYTE)_inp(h->statusAddr);
      if ((n == 0) && (b & I2C_SDA))
      {
         no_ack=1;  // verify ack
      }

      _outp(h->dataAddr, I2C_SCL);        // scl lo
      I2C_WAIT(_outp(h->dataAddr, I2C_SCL))

      /* store the data read */
      if (n > 0)
         pData[n-1] = rcv_data;
   }

done:
   /* generate the stop bit */
   _outp(h->controlAddr, 0);
   I2C_WAIT(_outp(h->controlAddr, 0))
   _outp(h->dataAddr, 0);
   I2C_WAIT(_outp(h->dataAddr, 0))
   _outp(h->controlAddr, I2C_SDA);
   I2C_WAIT(_outp(h->controlAddr, I2C_SDA))

   LeaveCriticalSection(&(h->cs));

   if (no_ack)
      return BI2C_ERR_NO_ACK;

   return BERR_SUCCESS;
}


BERR_Code BI2C_P_ReadNoAddr
(
   void *context,            /* Device channel handle */
   uint16_t chipAddr,               /* chip address */
   uint8_t *pData,            /* pointer to memory location to store read data  */
   size_t length                  /* number of bytes to read */
)
{
   BI2C_ChannelHandle h = (BI2C_ChannelHandle)context;
   uint32_t i, n, mask;
   BYTE send_data, rcv_data, b, no_ack = 0;

   EnterCriticalSection(&(h->cs));

   /* generate the start bit */
   _outp(h->dataAddr, I2C_RD_TRIG);   /* set wr trig  */
   I2C_WAIT(_outp(h->dataAddr, I2C_RD_TRIG))
   _outp(h->controlAddr, 0);          /* set sda lo */
   I2C_WAIT(_outp(h->controlAddr, 0))
   _outp(h->dataAddr, I2C_SCL);       /* set scl lo */
   I2C_WAIT(_outp(h->dataAddr, I2C_SCL))

   /* read the requested data */
   for (n = 0 ; n <= length; n++)
   {
      if (n == 0)
         send_data = (chipAddr << 1) | 0x01;
      else
         send_data = 0xff;

      /* transmit the data */
      rcv_data = 0;
      mask     = 0x80;
      for (i=0; i<8; i++)
      {
         _outp(h->controlAddr, (mask & send_data) ? I2C_SDA : 0);  // set sda bit
         I2C_WAIT(_outp(h->controlAddr, (mask & send_data) ? I2C_SDA : 0))
         _outp(h->dataAddr, 0);                                    // set scl hi
         I2C_WAIT(_outp(h->dataAddr, 0))
         if (n > 0) // sample data
         {
            b = (BYTE)_inp(h->statusAddr);
            if (b & I2C_SDA)
               rcv_data |= mask;
         }
         _outp(h->dataAddr, I2C_SCL);                              // set SCL lo
         I2C_WAIT(_outp(h->dataAddr, I2C_SCL))
         mask = (mask >> 1);
      }

      /* generate the acknowledge bit */
      if ((n == 0) || (n == length))
      {
         _outp(h->controlAddr, I2C_SDA);  // sda hiz
         I2C_WAIT(_outp(h->controlAddr, I2C_SDA))
      }
      else
      {
         _outp(h->controlAddr, 0);        // sda lo
         I2C_WAIT(_outp(h->controlAddr, 0))
      }

      _outp(h->dataAddr, 0);              // scl hi
      I2C_WAIT(_outp(h->dataAddr, 0))

      /* wait for internal registers to load data before reading */
      //Sleep(m_BusDelay);

      b = (BYTE)_inp(h->statusAddr);
      if ((n == 0) && (b & I2C_SDA))
      {
         no_ack=1;  // verify ack
      }

      _outp(h->dataAddr, I2C_SCL);        // scl lo
      I2C_WAIT(_outp(h->dataAddr, I2C_SCL))

      /* store the data read */
      if (n > 0)
         pData[n-1] = rcv_data;
   }

   /* generate the stop bit */
   _outp(h->controlAddr, 0);
   I2C_WAIT(_outp(h->controlAddr, 0))
   _outp(h->dataAddr, 0);
   I2C_WAIT(_outp(h->dataAddr, 0))
   _outp(h->controlAddr, I2C_SDA);
   I2C_WAIT(_outp(h->controlAddr, I2C_SDA))

   LeaveCriticalSection(&(h->cs));

   if (no_ack)
      return BI2C_ERR_NO_ACK;

   return BERR_SUCCESS;
}


BERR_Code BI2C_P_Write
(
   void *context,            /* Device channel handle */
   uint16_t chipAddr,               /* chip address */
   uint32_t subAddr,               /* 8-bit sub address */
   const uint8_t *pData,            /* pointer to data to write */
   size_t length                  /* number of bytes to write */
)
{
   uint32_t i,n,mask;
   uint8_t data,no_ack=0;
   BI2C_ChannelHandle h = (BI2C_ChannelHandle)context;

   EnterCriticalSection(&(h->cs));

   /* generate the start bit */
   _outp(h->dataAddr, I2C_WR_TRIG);   // set wr trig
   I2C_WAIT(_outp(h->dataAddr, I2C_WR_TRIG))
   _outp(h->controlAddr, 0  );        // set data  lo
   I2C_WAIT(_outp(h->controlAddr, 0  ))
   _outp(h->dataAddr, I2C_SCL);       // set clock lo
   I2C_WAIT(_outp(h->dataAddr, I2C_SCL))

   for (n = 0; n < (length + 2); n++)
   {
      if (n == 0)
         data = (uint8_t)((chipAddr << 1) & 0xFE);
      else if (n == 1)
         data = (uint8_t)subAddr;
      else
         data = pData[n-2];

      // transmit the data
      mask = 0x80;
      for (i = 0; i < 8; i++)
      {
         _outp(h->controlAddr,(mask & data) ? I2C_SDA : 0); // set data bit
         I2C_WAIT(_outp(h->controlAddr,(mask & data) ? I2C_SDA : 0))
         _outp(h->dataAddr, 0  );                           // set clock hi
         I2C_WAIT(_outp(h->dataAddr, 0  ))
         _outp(h->dataAddr, I2C_SCL);                       // set clock lo
         I2C_WAIT(_outp(h->dataAddr, I2C_SCL));
         mask = (mask >> 1);
      }

      /* generate the acknowledge bit */
      _outp(h->controlAddr, I2C_SDA);
      I2C_WAIT(_outp(h->controlAddr, I2C_SDA))
      _outp(h->dataAddr, 0);                      // set clock hi
      I2C_WAIT(_outp(h->dataAddr, 0))

      if (_inp(h->statusAddr) & I2C_SDA)
         no_ack=1;                              // test ack

      _outp(h->dataAddr, I2C_SCL);                // set clock lo
      I2C_WAIT(_outp(h->dataAddr, I2C_SCL))
   }

   /* generate the stop bit */
   _outp(h->controlAddr, 0);                      // set data  lo
   I2C_WAIT(_outp(h->controlAddr, 0))
   _outp(h->dataAddr, 0);                         // set clock hi
   I2C_WAIT(_outp(h->dataAddr, 0))
   _outp(h->controlAddr, I2C_SDA);                // set data  hi
   I2C_WAIT(_outp(h->controlAddr, I2C_SDA))

   LeaveCriticalSection(&(h->cs));

   if (no_ack)
      return BI2C_ERR_NO_ACK;

   return BERR_SUCCESS;
}


BERR_Code BI2C_P_WriteNoAddr
(
   void *context,          /* Device channel handle */
   uint16_t chipAddr,      /* chip address */
   const uint8_t *pData,   /* pointer to data to write */
   size_t length           /* number of bytes to write */
)
{
   uint32_t i,n,mask;
   uint8_t data,no_ack=0;
   BI2C_ChannelHandle h = (BI2C_ChannelHandle)context;

   EnterCriticalSection(&(h->cs));

   /* generate the start bit */
   _outp(h->dataAddr, I2C_WR_TRIG);   // set wr trig
   I2C_WAIT(_outp(h->dataAddr, I2C_WR_TRIG))
   _outp(h->controlAddr, 0  );        // set data  lo
   I2C_WAIT(_outp(h->controlAddr, 0  ))
   _outp(h->dataAddr, I2C_SCL);       // set clock lo
   I2C_WAIT(_outp(h->dataAddr, I2C_SCL))

   for (n = 0; n < (length + 1); n++)
   {
      if (n == 0)
         data = (uint8_t)((chipAddr << 1) & 0xFE);
      else
         data = pData[n-1];

      // transmit the data
      mask = 0x80;
      for (i = 0; i < 8; i++)
      {
         _outp(h->controlAddr,(mask & data) ? I2C_SDA : 0); // set data bit
         I2C_WAIT(_outp(h->controlAddr,(mask & data) ? I2C_SDA : 0))
         _outp(h->dataAddr, 0  );                           // set clock hi
         I2C_WAIT(_outp(h->dataAddr, 0  ))
         _outp(h->dataAddr, I2C_SCL);                       // set clock lo
         I2C_WAIT(_outp(h->dataAddr, I2C_SCL));
         mask = (mask >> 1);
      }

      /* generate the acknowledge bit */
      _outp(h->controlAddr, I2C_SDA);
      I2C_WAIT(_outp(h->controlAddr, I2C_SDA))
      _outp(h->dataAddr, 0);                      // set clock hi
      I2C_WAIT(_outp(h->dataAddr, 0))

      if (_inp(h->statusAddr) & I2C_SDA)
         no_ack=1;                              // test ack

      _outp(h->dataAddr, I2C_SCL);                // set clock lo
      I2C_WAIT(_outp(h->dataAddr, I2C_SCL))
   }

   /* generate the stop bit */
   _outp(h->controlAddr, 0);                      // set data  lo
   I2C_WAIT(_outp(h->controlAddr, 0))
   _outp(h->dataAddr, 0);                         // set clock hi
   I2C_WAIT(_outp(h->dataAddr, 0))
   _outp(h->controlAddr, I2C_SDA);                // set data  hi
   I2C_WAIT(_outp(h->controlAddr, I2C_SDA))

   LeaveCriticalSection(&(h->cs));

   if (no_ack)
      return BI2C_ERR_NO_ACK;

   return BERR_SUCCESS;
}
