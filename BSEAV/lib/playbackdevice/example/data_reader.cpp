/***************************************************************************
 *    (c)2005-2011 Broadcom Corporation
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
#include <stdio.h>
//#include <directfb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "data_reader.h"

/**
 * @def Local Debug macros 
 */
#define LIBCURL_TRACE_LEVEL 0
#if 1
#undef  BDBG_ERR
#define BDBG_ERR(x) if (1) {printf x; printf("\n");}
#undef  BDBG_WRN
#define BDBG_WRN(x) if (1) {printf x; printf("\n");}
#undef  BDBG_MSG
#define BDBG_MSG(x) if (0) {printf x; printf("\n");}
#endif

   
bool isHttp(char *path) 
{     
    const char *http = "http://";
    
    bool isHttp = true;
    for (size_t i=0; i<strlen(http); i++) {
        if (path[i] != http[i]) {
            isHttp = false;
            break;
        }
    }
    return(isHttp);
}

int cFileReader::Open(char *filename) 
{
    if( (filename == NULL) || (strlen(filename) == 0) )
    {
        BDBG_ERR(("%s:%d URL invalid!", __FUNCTION__, __LINE__));    
        return READER_FAILURE;
    }

    if( m_pFilename != NULL )
    {
        BDBG_ERR(("%s:%d Already opened!!", __FUNCTION__, __LINE__));    
        return READER_FAILURE;
    }
       
    m_pFilename = (char *)malloc(strlen(filename));
    if( m_pFilename == NULL )
    {
        BDBG_ERR(("%s:%d malloc returned NULL!!", __FUNCTION__, __LINE__));    
        return READER_OUT_OF_MEMORY;
    }

    strcpy(m_pFilename, filename);

    struct stat buf;

    /* Get stats from file */
    if (stat(m_pFilename, &buf) != 0)
    {
        BDBG_ERR(("LoaderFileSize() -- Error getting file stats!\n"));
        return(READER_FILE_ERROR);
    }

    /* return the file size */
    m_size = buf.st_size;

    int flags = O_RDONLY;

    /* open file in read mode */
    m_handle = open(m_pFilename, flags);
    if (m_handle < 0)
    {
        BDBG_ERR(("LoaderFileOpen() -- Error opening file! %d, %s\n", m_handle, m_pFilename));
        return(READER_FILE_ERROR);
    }

    return(READER_SUCCESS);
}
    
int cFileReader::Close(void) 
{
    int status = READER_SUCCESS;
    if( m_pFilename != NULL )
    {
        free(m_pFilename);
        m_pFilename    = NULL;
    }

    if( m_handle >= 0 )
    {
        /* close the file */
        if (close(m_handle) != 0)
        {
            BDBG_ERR(("LoaderFileClose() -- Error closing file!\n"));
            status = READER_FILE_ERROR;
        }
    }
    m_handle = -1;
    m_size   = 0;

    return(status);
}
  
int cFileReader::Read(ULONG bytesRequested, PVOID readData, ULONG *readSize) 
{
    int   err             = READER_SUCCESS;
    LONG  lSz             = 0;
    ULONG ulSizeReturned  = 0;
    ULONG ulSize          = bytesRequested;

    if ( (m_handle < 0) || (readData == NULL) || (readSize == NULL))
    {
        BDBG_ERR(("LoaderFileRead() -- CALLED WITH REQUIRED VALUE == 0!\n"));
        return (READER_NULL_PTR);
    }

    do
    {
        ulSize    -= ulSizeReturned;

        /* Read data from the file */
        lSz = read(m_handle, readData, (size_t)ulSize);

        /* if we got data */
        if (lSz > 0)
        {
            /* if it wasn't all of the data requested, we're going to try again */
            /* but either way, record how much we got so far */
            ulSizeReturned += (ULONG)lSz;
        }
        else if (lSz == 0)
        {
            /* we're at the end of the file, return as such */
            BDBG_MSG(("LoaderFileRead() -- READ REQUESTED AT END OF FILE!\n"));
            err = READER_FILE_EOF;
        }
        else
        {
            /* we've hit a read error, return as such */
            BDBG_ERR(("LoaderFileRead() -- READ ERROR: File Handle 0x%08x: READ REQUEST SIZE %lu, %li BYTES RETURNED!!!\n",
		      m_handle, ulSize, lSz));
            err = READER_FILE_ERROR;
        }
    } while ( (lSz > 0) && ((ULONG)lSz < ulSize) );

    /* even if we got eof or an error eventually, return the amount of data we did read successfully */
    *readSize = ulSizeReturned;

    return (err);
}

int cFileReader::Seek(int origin,  LONGLONG byteOffset) 
{
    int err = READER_SUCCESS;

    if( m_handle < 0 )
    {
        BDBG_ERR(("LoaderFileRead() -- CALLED WITH REQUIRED VALUE == 0!\n"));
        return (READER_NULL_PTR);
    }

    if (lseek(m_handle, byteOffset, origin) < 0)
    {
        BDBG_ERR(("LoaderFileSeek() -- Error seeking in file!\n"));
        err = READER_FILE_ERROR;
    }

    return(err);
}
  
int cFileReader::Tell(ULONGLONG *byteOffset) 
{
    int err = READER_SUCCESS;

    if ( m_handle < 0 )
    {
        BDBG_ERR(("Tell() -- NULL pointer!\n"));
        return (READER_NULL_PTR);
    }

    off_t pos = lseek(m_handle, 0, SEEK_CUR);

    if (pos == -1)
    {
        BDBG_ERR(("Tell() -- lseek error!\n"));
        err = READER_FAILURE;
    }
    else
    {
        *byteOffset = pos;
    }

    return (err);
}

#define HDR_RD_SZ 16
int cEncrptedPesReader:: ReadPacket(ULONG bytesRequested,PVOID pHdr,ULONG *hdrSize,PVOID readData, ULONG *readSize)
{
	 int err = READER_SUCCESS;int i=0;
	
	 //printf("\nbytes requested is 0x%x",bytesRequested);
	m_pFileReader->Read(HDR_RD_SZ, pHdr, hdrSize);
	if(*hdrSize<HDR_RD_SZ){
		//printf("\nWARNING!!!Read incomplete, requestedsz=0x%x, readsize=0x%x",HDR_RD_SZ,hdrSize);
		err = READER_FAILURE;
	}
	else{
				/*printf("\n******HEADER*******\n");
		for ( i=0;i<*hdrSize;i++)
			printf("%02x ",((unsigned char *) (pHdr))[i]);
		printf("\n");*/
	
	}
	
		m_pBuffer =(unsigned char *)pHdr;

		m_nextSz= (*(m_pBuffer+4))<<8 | (*(m_pBuffer+5));
		//printf("\n m_nextsz:0x%x",m_nextSz);
		m_payloadSz= m_nextSz;
		m_nextSz+=(*(m_pBuffer+8));
		
		m_payloadOffset = 8+(*(m_pBuffer+8))+m_curOffset+1;
		//printf("\n payload offset is 0x%x",m_payloadOffset);
		m_headerSz=(8+(*(m_pBuffer+8)))+1;
		m_payloadSz-=(3+(*(m_pBuffer+8)));
		
		//m_nextOffset=m_curOffset+m_nextSz+1;
		m_nextOffset=m_payloadOffset +m_payloadSz;

		*hdrSize=m_headerSz;
		//printf("\nnext_offset is 0x%x, next_size is 0x%x,payloadOffset is 0x%x,m_payloadSz is 0x%x",m_nextOffset, m_nextSz,m_payloadOffset,m_payloadSz);
		//now read the payload
		err = m_pFileReader->Seek(0x0,m_payloadOffset);
		if(err!=0)
		{
			printf("\n seek to 0x%x failed could be end of file",m_payloadOffset);
			
		}
		//printf("\nRead Payload from 0x%x with size :0x%x",m_payloadOffset,m_payloadSz);
		if(m_payloadSz>bytesRequested)
		{
			printf("\nWARNING m_payloadSz>bytesrequested , 0x%x>0x%x",m_payloadSz,bytesRequested );
			getchar();
		}
		m_pFileReader->Read(m_payloadSz, readData,readSize);
		if(*readSize<m_payloadSz){
			printf("\nWARNING!!!Read payload incomplete, requestedsz=0x%x, readsize=0x%x",m_payloadSz,readSize);
			err = READER_FAILURE;
		}
		else
		{
			/*printf("\n*****Payload*******\n");
			for ( i=0;i<16;i++)
			printf("%02x ",((unsigned char*)(readData))[i]);
				printf("\n");*/
		}
		err = m_pFileReader->Seek(0x0,m_nextOffset);
		if(err!=0)
		{
			printf("\n seek to 0x%x failed could be end of file",m_payloadOffset);
			
		}
		m_curOffset=m_nextOffset;
	return (err);

}






//static HttpContext httpCtx;

static size_t httpPayloadCallback(void *buffer, size_t itemSize, size_t numItems, void *callbackCtx) 
{
    cHttpReader *pHttpReader = (cHttpReader *) callbackCtx;

    if( pHttpReader == NULL )
    {
        BDBG_ERR(("%s: Context=NULL!!!", __FUNCTION__));
        return(-1);
    }

    BDBG_MSG(("%s: Start itemSize=%d numItems=%d callBack=%p <%s>",__FUNCTION__, 
                itemSize, numItems, callbackCtx, pHttpReader->GetName()));

    size_t bytesRead = itemSize*numItems;
    if( bytesRead == 0 )
    {
        BDBG_ERR(("%s: ZeroBytesRead!!", __FUNCTION__));
    }
    else
    {
        bytesRead = pHttpReader->CopyData(buffer, bytesRead);
    }
    
    return(bytesRead);
}

int cHttpReader::Open(char *url) 
{     
    BDBG_ERR(("%s: Start",__FUNCTION__));
    CURLcode rc;

    if( (url == NULL) || (strlen(url) == 0) )
    {
        BDBG_ERR(("%s:%d URL invalid!", __FUNCTION__, __LINE__));    
        return READER_FAILURE;
    }

    if( m_pUrl != NULL )
    {
        BDBG_ERR(("%s:%d Already opened!!", __FUNCTION__, __LINE__));    
        return READER_FAILURE;
    }
       
    m_pUrl = (char *)malloc(strlen(url+1));
    if( m_pUrl == NULL )
    {
        BDBG_ERR(("%s:%d malloc returned NULL!!", __FUNCTION__, __LINE__));    
        return READER_OUT_OF_MEMORY;
    }
    strcpy(m_pUrl, url);

    if( m_pCurlHandle == NULL )
    {
        BDBG_ERR(("%s: Initializing curl ",__FUNCTION__));
    
        m_pCurlHandle = curl_easy_init();
        if(m_pCurlHandle == NULL) 
        {
            BDBG_ERR(("%s: curl_easy_init failed!", __FUNCTION__));
            return READER_FAILURE;
        }
    }    
    else
    {
        BDBG_ERR(("%s:%d ",__FUNCTION__, __LINE__));
        curl_easy_reset(m_pCurlHandle);
    }
         
#if LIBCURL_TRACE_LEVEL
    rc = curl_easy_setopt(m_pCurlHandle, CURLOPT_VERBOSE, LIBCURL_TRACE_LEVEL);
    if (rc  != CURLE_OK)
        BDBG_ERR(("*** ERROR (line: %d): curl_easy_setopt CURLOPT_VERBOSE failed!", __LINE__));    
#endif
    
    rc = curl_easy_setopt(m_pCurlHandle, CURLOPT_NOSIGNAL, 1);
    if (rc  != CURLE_OK)
        BDBG_ERR(("*** ERROR (line: %d): curl_easy_setopt CURLOPT_NOSIGNAL failed!", __LINE__));        

    rc = curl_easy_setopt(m_pCurlHandle, CURLOPT_HEADER, 0);
    if (rc  != CURLE_OK)
        BDBG_ERR(("*** ERROR (line: %d): curl_easy_setopt CURLOPT_NOSIGNAL failed!", __LINE__));
    
    rc = curl_easy_setopt(m_pCurlHandle, CURLOPT_LOW_SPEED_LIMIT, 128);
    if (rc  != CURLE_OK)
        BDBG_ERR(("*** ERROR (line: %d): curl_easy_setopt CURLOPT_LOW_SPEED_LIMIT failed!", __LINE__));
    
    rc = curl_easy_setopt(m_pCurlHandle, CURLOPT_LOW_SPEED_TIME, 2);
    if (rc  != CURLE_OK)
        BDBG_ERR(("*** ERROR (line: %d): curl_easy_setopt CURLOPT_LOW_SPEED_TIME failed!", __LINE__));
    
    rc = curl_easy_setopt(m_pCurlHandle, CURLOPT_FOLLOWLOCATION, 1);
    if (rc  != CURLE_OK)
        BDBG_ERR(("*** ERROR (line: %d): curl_easy_setopt CURLOPT_FOLLOWLOCATION failed!", __LINE__));
        
    rc = curl_easy_setopt(m_pCurlHandle, CURLOPT_MAXREDIRS, 4);
    if (rc  != CURLE_OK)
        BDBG_ERR(("*** ERROR (line: %d): curl_easy_setopt CURLOPT_MAXREDIRS failed!", __LINE__));
    
    rc = curl_easy_setopt(m_pCurlHandle, CURLOPT_URL, m_pUrl);
    if (rc  != CURLE_OK) {
        BDBG_ERR(("*** ERROR (line: %d): curl_easy_setopt CURLOPT_URL failed!", __LINE__));
        return READER_FAILURE;
    }         
     
    // Do a HTTP HEAD request to get the resource size
    rc = curl_easy_setopt(m_pCurlHandle, CURLOPT_NOBODY, m_pUrl);
    if (rc  != CURLE_OK) {
        BDBG_ERR(("*** ERROR (line: %d): curl_easy_setopt CURLOPT_NOBODY failed!", __LINE__));
        return READER_FAILURE;
    }
    
    curl_easy_perform(m_pCurlHandle);
    
    double size;
    curl_easy_getinfo(m_pCurlHandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);
    m_resourceSize = (ULONGLONG)size;

    BDBG_ERR(("%s: m_resourceSize=%d", __FUNCTION__, (int) m_resourceSize));
    
    // Set the default HTTP method back to GET
    curl_easy_setopt(m_pCurlHandle, CURLOPT_HTTPGET, m_pUrl);
    
    return READER_SUCCESS;
}            
    
int cHttpReader::Close(void) 
{
    BDBG_ERR(("%s: Start",__FUNCTION__));

    if( m_pCurlHandle != NULL )
    {
        curl_easy_reset(m_pCurlHandle);
    }
    
    if( m_pUrl != NULL )
    {
        free(m_pUrl);
    }

    m_pCurlHandle  = NULL;
    m_pUrl         = NULL;
    
    m_resourceSize     = 0;
    m_startOffset      = 0;
    m_stopOffset       = 0;
    m_streamOffset     = 0;
    m_numBytesLastRead = 0;

    return READER_SUCCESS;    
}
  
int cHttpReader::Read(ULONG bytesRequested, PVOID readData, ULONG *readSize) 
{
    BDBG_MSG(("%s: Start (bytesRequested=%lu)",__FUNCTION__, bytesRequested));
    if( (m_pUrl == NULL) || (m_pCurlHandle == NULL) )
    {
        BDBG_ERR(("%s:%d Reader Not Opened!!", __FUNCTION__, __LINE__));    
        return(READER_FAILURE);
    }

    if( (bytesRequested == 0) || (readData == NULL) || (readSize == NULL) ) 
    {
        BDBG_ERR(("%s:%d invalid Parameters %lu %p %p !!", __FUNCTION__, __LINE__,
                    bytesRequested, readData, (void *) readSize));    
        return(READER_INVALID_PARAMETER);
    }

    if( m_streamOffset == m_resourceSize )
    {
        BDBG_ERR(("%s:%d EndOfFile!!", __FUNCTION__, __LINE__));    
        return READER_FILE_EOF;
    }

    m_pBuffer          = readData;
    m_numBytesLastRead = 0;
    
    m_startOffset = m_streamOffset;
    m_stopOffset  = m_streamOffset + (bytesRequested - 1);
    if( (m_resourceSize > 0) && (m_stopOffset > m_resourceSize) )
    {
        m_stopOffset = m_resourceSize - 1;
    }
    
    static char rangeLine[64];
    snprintf(rangeLine, 64, "%lld-%lld", m_startOffset, m_stopOffset);
    
    CURLcode rc = curl_easy_setopt(m_pCurlHandle, CURLOPT_RANGE, rangeLine);
    if (rc != CURLE_OK) 
    {
        BDBG_ERR(("*** ERROR (line: %d): curl_easy_setopt CURLOPT_RANGE failed!", __LINE__));
        return READER_FAILURE;
    }
    
    rc = curl_easy_setopt(m_pCurlHandle, CURLOPT_WRITEDATA, (void *)this);
    if (rc  != CURLE_OK) {
        BDBG_ERR(("*** ERROR (line: %d): curl_easy_setopt CURLOPT_WRITEFUNCTION failed!", __LINE__));
        return READER_FAILURE;
    }
    
    rc = curl_easy_setopt(m_pCurlHandle, CURLOPT_WRITEFUNCTION, httpPayloadCallback);
    if (rc  != CURLE_OK) {
        BDBG_ERR(("*** ERROR (line: %d): curl_easy_setopt CURLOPT_WRITEFUNCTION failed!", __LINE__));
        return READER_FAILURE;
    }
    
    rc = curl_easy_perform(m_pCurlHandle);
    if (rc  != CURLE_OK)
        return READER_FILE_EOF;
     
    *readSize = m_numBytesLastRead;    

    BDBG_MSG(("%s: End bytesReturned=%u",__FUNCTION__, m_numBytesLastRead));
    return READER_SUCCESS;   
}

int cHttpReader::Seek(int origin,  LONGLONG byteOffset) 
{
    BDBG_ERR(("%s: Start",__FUNCTION__));
    if (origin == 0)
    {
        m_streamOffset = byteOffset;
    }
    else if (origin == 1)
    {
        m_streamOffset += byteOffset;
        if( m_streamOffset >= m_resourceSize )
        {
            m_streamOffset = m_resourceSize;
        }
    }
    else
    {
        if( byteOffset >= m_resourceSize )
        {
            m_streamOffset = 0;
        }
        else
        {
            m_streamOffset = m_resourceSize - byteOffset - 1;
        }
    }
    
    return READER_SUCCESS;
}
  
int cHttpReader::Tell(ULONGLONG *byteOffset) 
{
    BDBG_ERR(("%s: Start",__FUNCTION__));
    
    *byteOffset = m_streamOffset;
    return READER_SUCCESS;
}
  
size_t cHttpReader::CopyData(void *buffer, size_t bytesRead) 
{
    BDBG_MSG(("%s: Start",__FUNCTION__));
    if( m_pBuffer != NULL )
    {
        char *pBuff  = (char *)m_pBuffer; 
              pBuff += m_numBytesLastRead;

        memcpy((void *)pBuff, buffer, bytesRead);
        m_streamOffset     += bytesRead;
        m_numBytesLastRead += bytesRead;
        return(bytesRead);
    }
    return(0);
}
