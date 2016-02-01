/***************************************************************************
 *     Copyright (c) 2002-2010, Broadcom Corporation
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
 ****************************************************************/

#include "bstd.h"
#include "tshdrbuilder.h"

#include <string.h> /* memset */
#include <stdio.h>  /* printf,file io */

/* ============================ AUX =============================== */
/* FOR TESTING ONLY                                                 */
/* ================================================================ */
void write_buf2hdd(void * buf, size_t buf_size, char * filename){
    FILE * file;
    unsigned i;
    uint8_t* buf_ptr = buf;

    file = fopen (filename, "wb");
    if(file){
        for(i = 0; i < buf_size; i++)    
            fputc( buf_ptr[i], file);
    }
    fclose(file);
}

unsigned find_next_pes_start (FILE *file, uint8_t stream_id, unsigned offset_byte){
    uint8_t buf[4] = {0xFF,0xFF,0xFF,0xFF};
    unsigned i;
    int offset = 0;
    for(i = 0; i < offset_byte; i++) fgetc(file);
    while( !feof(file) ){
        buf[0]=buf[1];
        buf[1]=buf[2];
        buf[2]=buf[3];
        buf[3]=fgetc(file);
        offset++;
        if(buf[0]==0 && buf[1]==0 && buf[2]==1 && buf[3]==stream_id){
            fseek(file, -(offset+(int)offset_byte), SEEK_CUR);
            return (unsigned) offset-3+offset_byte;
        }
    }
    if(offset == 0) {
        return 0;
    } else {
        fseek(file, -(offset+(int)offset_byte), SEEK_CUR);
        return (unsigned) (offset+(int)offset_byte);
    }
}

#define update_psi_cc(buf) ((buf[4]&0x0F)+1)&0xF + (buf[4]&0xF0)
void pack_ts_w_psi(uint8_t *patBuf, uint8_t *pmtBuf, char * pes_filename, uint16_t pes_pid, char * out_filename) {
    FILE *vfile, *ofile;
    uint8_t *patBufTmp;
    uint8_t *pmtBufTmp;
    uint8_t hdr_len;
    uint8_t payload_len;
    unsigned i,j;
    unsigned pes_len;
    size_t payload_pked;
    uint8_t ts_hdr_buf[1024*1024];
    uint8_t *hdr_ptr;
    size_t buf_used;
    vfile = fopen (pes_filename, "rb");
    ofile = fopen (out_filename, "wb");

    if(vfile == NULL) {
        printf("Error: PES stream cnnticker.pes not found in the current directory. Obtain a copy from /projects_local/stbdevstream_nas0001/local/ts_teststream/cnnticker.pes, or generated one by stripping cnnticker.mpg with Broadcom internal tool and try again.\n\n");
        fclose(ofile);
        exit(0);
    }

    unsigned payload_cc = 0x0;
    unsigned psi_cc = 0x0;
    TS_PID_info pidInfo;
    TS_PID_state pidState;

    TS_PID_info_Init(&pidInfo, pes_pid, 1);
    TS_PID_state_Init(&pidState);

    pes_len = find_next_pes_start(vfile, 0xE0, 1);
    while(pes_len != 0){
        TS_buildTSHeader(&pidInfo, &pidState, ts_hdr_buf, 1024*1024, &buf_used, pes_len, &payload_pked, 5000);

        patBufTmp = patBuf;
        pmtBufTmp = pmtBuf;
        for( i = 0; i < 188; i++ ) { fputc(*(++patBufTmp),ofile);}
        for( i = 0; i < 188; i++ ) { fputc(*(++pmtBufTmp),ofile);}
        update_psi_cc(patBuf);
        update_psi_cc(pmtBuf);

        hdr_ptr = ts_hdr_buf; 
        for ( i = 0; hdr_ptr < (uint8_t*)ts_hdr_buf+buf_used; i++){
            hdr_len = (*hdr_ptr++) & 0xFF;
            payload_len = 188 - hdr_len;
            for ( j = 0; j < hdr_len; j++ ) { fputc(*hdr_ptr++, ofile); }
            for ( j = 0; j < payload_len; j++) { fputc( fgetc(vfile), ofile); }
        }
        pes_len = find_next_pes_start(vfile, 0xE0, 1);
    }
    fclose(vfile);
    fclose(ofile);
}

unsigned tell_fileSize(char* name){
    FILE *file;
    unsigned size;
    file = fopen (name, "rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        size = ftell(file);
        fclose(file);
        return size;
    }

    return 0;
}

int main(){
    uint8_t desc_buf[] = {0xAB, 0xCD};
    uint8_t pat_hdr_buf[189], pmt_hdr_buf[189];
    uint8_t pat_pl_buf[189], pmt_pl_buf[189];
    size_t pat_pl_size, pmt_pl_size;
    size_t buf_used = 0;
    size_t payload_pked = 0;
    char pes_filename[] = "cnnticker.pes";
    uint16_t pes_pid = 0x21;
    unsigned streamNum;
    BERR_Code rc;

    TS_PAT_state pat;
    TS_PSI_header psi_header;
    TS_PMT_state pmt;
    TS_PAT_program program;
    TS_PMT_stream pmt_stream;
    TS_PSI_descriptor desc;

    TS_PID_info pidInfo;
    TS_PID_state pidState;

#if 1
    /* == CREATE PSI TABLES == */
    TS_PSI_header_Init(&psi_header);
    TS_PAT_Init(&pat, &psi_header, pat_pl_buf, 189);

    TS_PAT_program_Init(&program, 1, 0x100);
    TS_PAT_addProgram(&pat, &pmt, &program, pmt_pl_buf, 189);

    TS_PMT_stream_Init(&pmt_stream, 0xFF, pes_pid);
    TS_PMT_addStream(&pmt, &pmt_stream, &streamNum);

#if 0
    /* Optional: set descriptor */
    TS_PSI_descriptor_Init(&desc, desc_buf);
    TS_PMT_setDescriptor(&pmt, desc, 2, streamNum);
#endif

    TS_PAT_finalize(&pat, &pat_pl_size);
    TS_PMT_finalize(&pmt, &pmt_pl_size);

    /* == CREATE TS HEADERS FOR PSI INFORMATION == */
    TS_PID_info_Init(&pidInfo, 0x0, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, pat_hdr_buf, 189, &buf_used, pat.size, &payload_pked, 1);
    memcpy(&pat_hdr_buf[buf_used], pat_pl_buf, pat_pl_size);
    /*Intermediate check*/
    /*write_buf2hdd(pat_hdr_buf, pat.size+buf_used, "pat.hex");*/

    TS_PID_info_Init(&pidInfo, 0x100, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, pmt_hdr_buf, 189, &buf_used, pmt.size, &payload_pked, 1);
    memcpy(&pmt_hdr_buf[buf_used], pmt_pl_buf, pmt_pl_size);
    /*Intermediate check*/
    /*write_buf2hdd(pmt_hdr_buf, pmt.size+buf_used, "pmt.hex");*/

    /* == PACKAGE A TS STREAM == */
    pack_ts_w_psi(pat_hdr_buf, pmt_hdr_buf, pes_filename, pes_pid, "output.ts");
#endif
    return 0;
}
