/***************************************************************************
 *     Copyright (c) 2007-2013, Broadcom Corporation
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
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/

#include "bstd.h"
#include "bkni.h"

#include "subtitle_control.h"

#include <stdlib.h>

BDBG_MODULE(bsubtitle_parse);

/* These need to match with defines in bmkv_player.h */
#define B_AUX_TAG_PAYLOAD   0
#define B_AUX_TAG_CODEC_PRIVATE 1
#define B_AUX_TAG_DURATION  2
#define B_AUX_TAG_ATTACHMENT_LINK   3

#define read8(pBuffer,offs)       (*(pBuffer + offs))
#define read16(pBuffer,offs)      (*(pBuffer + offs) | (*(pBuffer + offs + 1) << 8))
#define read32(pBuffer,offs)      (*(pBuffer + offs) | (*(pBuffer + offs + 1) << 8) | (*(pBuffer + offs + 2) << 16) | (*(pBuffer + offs + 3) << 24))

bstitle_parse_t g_stitle_parse;

static uint8_t getNibble(uint8_t *pBuffer, int32_t offs)
{
    int32_t b = offs/2;
    int32_t h = (offs%2 == 1);

    uint8_t ret;

    if(h == 0)
    {
        ret = (pBuffer[b] & 0xF0) >> 4;
    }
    else
    {
        ret = (pBuffer[b] & 0x0F) >> 0;
    }

    return ret;
}

static void setNibble(uint8_t *pBuffer, int32_t offs, uint8_t n)
{
    int32_t b = offs/2;
    int32_t h = (offs%2 == 1);

    if(h == 0)
    {
        pBuffer[b] = (pBuffer[b] & 0x0F) | (n << 4);
    }
    else
    {
        pBuffer[b] = (pBuffer[b] & 0xF0) | (n << 0);
    }

    return;
}


void bstitle_get_header_info(uint8_t *buffer, bstitle_parse_t stitle_parse)
{
	BKNI_Memcpy(stitle_parse->info.duration, buffer, 27);

	stitle_parse->info.width = read16(buffer, 0x1B);
	stitle_parse->info.height = read16(buffer, 0x1D);
	stitle_parse->info.left = read16(buffer, 0x1F);
	stitle_parse->info.top = read16(buffer, 0x21);
	stitle_parse->info.right = read16(buffer, 0x23);
	stitle_parse->info.bottom = read16(buffer, 0x25);
	stitle_parse->info.field_offset = read16(buffer, 0x27);

	stitle_parse->info.background.red = read8(buffer, 0x29);
	stitle_parse->info.background.green = read8(buffer, 0x2A);
	stitle_parse->info.background.blue = read8(buffer, 0x2B);

	stitle_parse->info.pattern1.red = read8(buffer, 0x2C);
	stitle_parse->info.pattern1.green = read8(buffer, 0x2D);
	stitle_parse->info.pattern1.blue = read8(buffer, 0x2E);

	stitle_parse->info.pattern2.red = read8(buffer, 0x2F);
	stitle_parse->info.pattern2.green = read8(buffer, 0x30);
	stitle_parse->info.pattern2.blue = read8(buffer, 0x31);

	stitle_parse->info.pattern3.red = read8(buffer, 0x32);
	stitle_parse->info.pattern3.green = read8(buffer, 0x33);
	stitle_parse->info.pattern3.blue = read8(buffer, 0x34);	

	

	if(stitle_parse->subtitle_type == bavi_subtitle_type_dxsa){
		stitle_parse->info.background_transparency = read8(buffer, 0x35);
		stitle_parse->info.pattern1_transparency = read8(buffer, 0x36);
		stitle_parse->info.pattern2_transparency = read8(buffer, 0x37);
		stitle_parse->info.pattern3_transparency = read8(buffer, 0x38);
		stitle_parse->info.rleData = buffer + BSUBTITLE_HDR_SIZE + (4 * sizeof(uint8_t));
	}
	else {
	    stitle_parse->info.background_transparency = 0;
	    stitle_parse->info.pattern1_transparency = 0xFF;
	    stitle_parse->info.pattern2_transparency = 0xFF;
	    stitle_parse->info.pattern3_transparency = 0xFF;
		stitle_parse->info.rleData = buffer + BSUBTITLE_HDR_SIZE;
	}

}

int bstitle_parse_rle2raw(bstitle_parse_t stitle_parse, int32_t rleSize)
{
	int32_t width, height, field_offset;
	
	/* encoded top/bottom fields */
	uint8_t *pEncoded[2];

	/* write/read cursors for top/bottom fields */
	int32_t w[2], r[2]={0,0};

	/* read stop offsets */
	int32_t rstop[2];

	uint8_t* rawData = stitle_parse->raw_data[stitle_parse->wptr] + sizeof(stitle_parse->info);

	if(stitle_parse->subtitle_type == bavi_subtitle_type_dxsa){
		rleSize -= (BSUBTITLE_HDR_SIZE + (4 * sizeof(uint8_t)));
	}
	else{
		rleSize -= BSUBTITLE_HDR_SIZE;
	}
	
	/* cache metrics */
	width = stitle_parse->info.width;
	height = stitle_parse->info.height;
	field_offset = stitle_parse->info.field_offset;

	/* read stop offsets */
	rstop[0] = field_offset*2;
	rstop[1] = (rleSize - field_offset)*2;

	/* encode field (top) */
	pEncoded[0] = (uint8_t*)(stitle_parse->info.rleData);

	/* encode field (bottom) */
	pEncoded[1] = (uint8_t*)(stitle_parse->info.rleData + field_offset);

	/* write cursors */
	w[0] = width*(height-1);
	w[1] = width*(height-2);

	/* continuously decode lines until height is reached */
	while(w[0] >= 0 || w[1] >= 0) {
		int32_t v;
		
		/* iterate through both top and bottom fields */
		for(v=0;v<2;v++) {
			int32_t col, len, i;
			
			if(w[v] < 0)
			  continue;
			
			if(r[v] < rstop[v]) {
				/* grab next input nibble */
				int32_t rle = getNibble(pEncoded[v], r[v]++);
				
				if(rle < 0x04) {
					rle = (rle << 4) | getNibble(pEncoded[v], r[v]++);
					
					if(rle < 0x10) {		      
						rle = (rle << 4) | getNibble(pEncoded[v], r[v]++);

						if(rle < 0x040) {                       
							rle = (rle << 4) | getNibble(pEncoded[v], r[v]++);
							
							if(rle < 0x0004)
							  rle |= (width - w[v]%width) << 2;
						}
					}
				}

				col = rle & 0x03;
				len = rle >> 2;
				
				if(len > (width - w[v]%width) || len == 0) {
					len = width - w[v]%width;
				}
			}
			else {			 
				col = 0;
				len = width - w[v]%width;
			}

			for(i=0;i<len;i++) {
				setNibble(rawData, w[v]++, (uint8_t)col);
			}
			
			if(w[v]%width == 0) {           
				w[v] -= width*3;
				
				if(r[v]%2 == 1)
				  r[v]++;
			}
		}
	}
			
	return 0;
}

uint8_t * bstitle_get_buffer(bstitle_parse_t stitle_parse) 
{
	uint8_t * buf = NULL;

	if(stitle_parse->wptr != stitle_parse->rptr){	
		buf = stitle_parse->raw_data[stitle_parse->rptr];		
	}
	
	return buf;	
}

void bstitle_read_complete(bstitle_parse_t stitle_parse)
{
    if(stitle_parse->wptr != stitle_parse->rptr){
	BKNI_Memset(stitle_parse->raw_data[stitle_parse->rptr], 0, stitle_parse->raw_data_size);
	stitle_parse->rptr++;
	if(stitle_parse->rptr >= DATA_BUFFERS){
	    stitle_parse->rptr = 0;
	}
    }
}

int32_t bstitle_get_start_time(bstitle_parse_t stitle_parse)
{
	int32_t hr, min, sec, msec, time;
	char hrs[3], mins[3], secs[3], msecs[4];

	BKNI_Memcpy(hrs, stitle_parse->info.duration+1 ,2);
	BKNI_Memcpy(mins, stitle_parse->info.duration+4 ,2);
	BKNI_Memcpy(secs, stitle_parse->info.duration+7 ,2);
	BKNI_Memcpy(msecs, stitle_parse->info.duration+10 ,3);
	hrs[2] = mins[2] = secs[2] = msecs[3] = '\0';

	hr = atoi(hrs);
	min = atoi(mins);
	sec = atoi(secs);
	msec = atoi(msecs);

	time = (((((hr*60) + min)*60) + sec)*1000) + msec;

	return time;
}

int32_t bstitle_get_end_time(bstitle_parse_t stitle_parse)
{
	int32_t hr, min, sec, msec, time;
	char hrs[3], mins[3], secs[3], msecs[4];	

	BKNI_Memcpy(hrs, stitle_parse->info.duration+14 ,2);
	BKNI_Memcpy(mins, stitle_parse->info.duration+17 ,2);
	BKNI_Memcpy(secs, stitle_parse->info.duration+20 ,2);
	BKNI_Memcpy(msecs, stitle_parse->info.duration+23 ,3);
	hrs[2] = mins[2] = secs[2] = msecs[3] = '\0';

	hr = atoi(hrs);
	min = atoi(mins);
	sec = atoi(secs);
	msec = atoi(msecs);

	time = (((((hr*60) + min)*60) + sec)*1000) + msec;

	return time;
}

void bstitle_parser_bmp(bstitle_parse_t stitle_parse, uint8_t *buffer, size_t length)
{		
	int32_t start_time, end_time;
	size_t len = (buffer[4]<<8 | buffer[5]) + 6;
	
	BSTD_UNUSED(length);

	buffer += 9;
	len -= 9;

	bstitle_get_header_info(buffer, stitle_parse);	

	start_time = bstitle_get_start_time(stitle_parse);
	end_time = bstitle_get_end_time(stitle_parse);

	if((end_time-start_time)<500)
		return;

	/*Write Header Info to buffer*/
	BKNI_Memcpy(stitle_parse->raw_data[stitle_parse->wptr], &stitle_parse->info, BSUBTITLE_INFO_SIZE);	
	/*Convert RLE to Raw Data and write to buffer*/
	bstitle_parse_rle2raw(stitle_parse, len/* -BSUBTITLE_HDR_SIZE */);	
	stitle_parse->wptr++;
	if(stitle_parse->wptr >= DATA_BUFFERS){
		stitle_parse->wptr = 0;
	}

	BDBG_ASSERT((stitle_parse->wptr != stitle_parse->rptr));       

	if((stitle_parse->cfg_params->app_cb)){
		stitle_parse->cfg_params->app_cb(stitle_parse->cfg_params->cnxt, start_time, end_time);
	}
}


void bstitle_parser_txt(bstitle_parse_t stitle_parse, uint8_t *buffer, size_t length)
{
	uint32_t start_time, end_time, size;
	uint8_t tag;
	uint8_t * buf = buffer;

	if(buf[6]>>7) {
		uint64_t pts;
		uint32_t pts1, pts2, pts3;
		pts1 = (buf[9]&0x0E)>>1;
		pts2 = (((buf[10]<<8) | buf[11])&0xFFFE)>>1;
		pts3 = (((buf[12]<<8) | buf[13])&0xFFFE)>>1;		
		pts = (pts1<<30) | (pts2<<15) | pts3;
		start_time = (uint32_t)(pts/90);	
	}	     	
	
	/*Advance to start of payload*/
	buf += 14;

	while((buffer+length) > buf){
		if(((uint32_t)(buf[0]<<24) | (uint32_t)(buf[1]<<16) | (uint32_t)(buf[2]<<8) | (uint32_t)(buf[3])) != 
			    ((uint32_t)('b'<<24) | (uint32_t)('m'<<16) | (uint32_t)('k'<<8) | (uint32_t)('v')))
			break;

		buf += 4;

		size = (buf[0]<<16 | buf[1]<<8 | buf[2]);			
		tag = buf[3];

		buf += 4;
	
		if((buf+size) > (buffer+length))
			break;

		if(tag == B_AUX_TAG_CODEC_PRIVATE || tag == B_AUX_TAG_ATTACHMENT_LINK ){

			/* We skip over this information for now as it is handled duing probe */
						
		} else if(tag == B_AUX_TAG_DURATION){
			unsigned duration = (buf[0]<<24 | buf[1]<<16 | buf[2]<<8 | buf[3]);
			end_time = start_time + duration - 1;
				
		} else if (tag == B_AUX_TAG_PAYLOAD){			
			char *str = (char*)buf;
			int left = size;
			unsigned cnt=0;	

			while(cnt<stitle_parse->txt_subtitle_position){
				str = (char*)findchr((char*)str, ',', left);
				if(str==NULL){
					str = (char*)buf;
					left = size;
					break;
				}
				left = size - ((uint32_t)str-(uint32_t)buf);
				if(left<=0)
					goto done;
				str++;
				cnt++;			
			}	

			left = size - ((uint32_t)str-(uint32_t)buf);
				
			/* First 4 bytes used to indicate length of payload */
			BKNI_Memcpy(stitle_parse->raw_data[stitle_parse->wptr], &left, 4); 
			BKNI_Memcpy(stitle_parse->raw_data[stitle_parse->wptr]+4, str, left);
			stitle_parse->wptr++;
		
			if(stitle_parse->wptr >= DATA_BUFFERS){
				stitle_parse->wptr = 0;
			}
		
			BDBG_ASSERT((stitle_parse->wptr != stitle_parse->rptr));       
		
			if((stitle_parse->cfg_params->app_cb)){		  
				stitle_parse->cfg_params->app_cb(stitle_parse->cfg_params->cnxt, start_time, end_time);
			}
		
		}

		buf += size;
	}
	
done:	
	return;
}     

