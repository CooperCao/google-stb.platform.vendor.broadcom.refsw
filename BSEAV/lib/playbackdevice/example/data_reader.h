/***************************************************************************
 * 
   (c)2005-2011 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
#ifndef __DATA_READER_H__
#define __DATA_READER_H__
#include <curl.h>

//typedef signed   long long LONGLONG;
//typedef unsigned long long ULONGLONG;
typedef off_t LONGLONG;
typedef off_t ULONGLONG;
typedef long               LONG;
typedef unsigned long      ULONG;
typedef void              *PVOID;

#define READER_SUCCESS              ((int) 0)    /**< Success */
#define READER_FAILURE              ((int)-1)    /**< General failure */
#define READER_INVALID_PARAMETER    ((int)-2)    /**< Invalide parameter */
#define READER_NULL_PTR             ((int)-3)    /**< NULL pointer */
#define READER_OUT_OF_MEMORY        ((int)-4)    /**< Malloc has failed */
#define READER_NOT_IMPLEMENTED      ((int)-5)    /**< Function not implemented */
#define READER_DISC_NOT_ENCRYPTED   ((int)-6)    /**< Disc is not encrypted */
#define READER_REGION_NOT_SET       ((int)-7)    /**< Region is not set on drive */
#define READER_NOT_READY            ((int)-8)    /**< Drive is not ready */
#define READER_HARDWARE_ERROR       ((int)-9)    /**< General hardware failure */
#define READER_MEDIUM_ERROR         ((int)-10)   /**< Medium error.  Disc may be damaged. */
#define READER_FILE_ERROR           ((int)-11)   /**< Filesystem error. */
#define READER_FILE_EOF             ((int)-12)   /**< File is at the end. */

bool isHttp(char *path);

class cDataReader
{
public:
    cDataReader() { }

    virtual ~cDataReader() { }

    virtual int Open(char *url) = 0;
    virtual int Close(void) = 0;
    virtual int Read(ULONG bytesRequested, PVOID readData, ULONG *readSize) = 0;
	virtual int ReadPacket(ULONG bytesRequested,PVOID pHdr,ULONG *hdrSize,PVOID readData, ULONG *readSize)=0;
    virtual int Seek(int origin,  LONGLONG byteOffset) = 0;
    virtual int Tell(ULONGLONG *byteOffset) = 0;
    virtual LONGLONG Size(void) = 0;
    virtual char    *GetName(void) = 0;

private:
};

class cFileReader : public cDataReader
{
public:
    cFileReader(void)
    { 
        m_handle       = -1;
        m_pFilename    = NULL;
        m_size         = 0;
    }

    ~cFileReader() 
    { 
        Close();
    }

    virtual int Open(char *filename);
    virtual int Close(void);
    virtual int Read(ULONG bytesRequested, PVOID readData, ULONG *readSize);
    virtual int Seek(int origin,  LONGLONG byteOffset);
    virtual int Tell(ULONGLONG *byteOffset);
    virtual LONGLONG Size(void)
    {
        return(m_size);
    }

    virtual char *GetName()
    {
        return(m_pFilename);
    }
	virtual int ReadPacket(ULONG bytesRequested,PVOID pHdr,ULONG *hdrSize,PVOID readData, ULONG *readSize){return 0;};

private:
    int           m_handle;
    char         *m_pFilename;
    LONGLONG      m_size;
};

class cHttpReader : public cDataReader
{
public:
    cHttpReader()
    { 
        m_pCurlHandle  = NULL;
        m_pUrl         = NULL;
    
        m_resourceSize     = 0;
        m_startOffset      = 0;
        m_stopOffset       = 0;
        m_streamOffset     = 0;
        m_numBytesLastRead = 0;
    }

    ~cHttpReader() 
    { 
        Close();
    }

    virtual int Open(char *url);
    virtual int Close(void);
    virtual int Read(ULONG bytesRequested, PVOID readData, ULONG *readSize);
    virtual int Seek(int origin,  LONGLONG byteOffset);
    virtual int Tell(ULONGLONG *byteOffset);
    virtual LONGLONG Size(void)
    {
        return(m_resourceSize);
    }
    virtual char *GetName()
    {
        return(m_pUrl);
    }
	virtual int ReadPacket(ULONG bytesRequested,PVOID pHdr,ULONG *hdrSize,PVOID readData, ULONG *readSize){return 0;};
    size_t CopyData(void *buffer, size_t bytesRead);

private:
    CURL     *m_pCurlHandle;
    char     *m_pUrl;
    
    LONGLONG  m_resourceSize;
 
    LONGLONG  m_startOffset;
    LONGLONG  m_stopOffset;
    LONGLONG  m_streamOffset;

    size_t    m_numBytesLastRead;
    void     *m_pBuffer;
};



class cEncrptedPesReader : public cDataReader
{
public:
    cEncrptedPesReader()
    { 
       
		m_pFileReader=new cFileReader();
		m_headerBuf=NULL;
		m_payloadBuf = NULL;
		m_curOffset =0x0;
	    m_curSz =0x10;
	
    }
	int Open(char* fname){return m_pFileReader->Open(fname);}
	int Close(){return m_pFileReader->Close();}
	int Seek(int origin, LONGLONG offset){return m_pFileReader->Seek(origin,offset);}
	int Tell(ULONGLONG* off){return m_pFileReader->Tell(off);}
	LONGLONG Size(){return m_pFileReader->Size();}
	char* GetName(){return m_pFileReader->GetName();}

	virtual ~cEncrptedPesReader() {delete m_pFileReader;}
	virtual int Read(ULONG bytesRequested, PVOID readData, ULONG *readSize) {return 0;}
	virtual int ReadPacket(ULONG bytesRequested,PVOID pHdr,ULONG *hdrSize,PVOID readData, ULONG *readSize);
	
   
private:
    unsigned long  m_headerSz;
    unsigned long	   m_headerOffset;
    unsigned long  m_payloadSz;
    unsigned long	   m_payloadOffset;
	unsigned long	   m_nextOffset;
	unsigned long  m_nextSz;	
    unsigned long  m_pesPktLen;
	unsigned long	   m_curOffset;
	unsigned long  m_curSz;
	void     *m_headerBuf;
	void     *m_payloadBuf;
	unsigned char * m_pBuffer;
	cFileReader *m_pFileReader;
};


#endif //__DATA_READER_H__
