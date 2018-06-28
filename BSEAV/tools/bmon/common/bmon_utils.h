/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/
#ifndef BMON_UTILS_H
#define BMON_UTILS_H

#ifdef PRINTF
#undef PRINTF
#endif
#define PRINTF noprintf

#ifdef FPRINTF
#undef FPRINTF
#endif
#define FPRINTF nofprintf

#define PROC_NET_TCP_FILENAME    "/proc/net/tcp"
#define PROC_NET_TCP_SIZE_MAX    (40*1024)
#define FREE_SAFE(buffer)        if (buffer) { free(buffer); buffer=0; }
#define TEMP_FILE_FULL_PATH_LEN  (64)

#ifdef MIN
#undef MIN
#endif
#define MIN(a, b)                ((a<b) ? (a) : (b))

const char *nofprintf( FILE *io, const char *format, ... );
const char *noprintf( const char *format, ... );
int         bmon_set_usage_response( char *buffer, int buffer_size, const char *Module, const char *KnownFields[], int KnownFieldsMax );
int         bmon_trim_line( char * line );
char       *bmon_get_temp_directory_str( void );
unsigned long int bmon_get_seconds_since_epoch( void );
char       *bmon_date_yyyy_mm_dd_hh_mm_ss( void );
char       *bmon_get_file_contents( const char *filename );
char       *GetFileContents( const char *filename );
char       *bmon_get_file_contents_proc( const char *filename, int max_expected_file_size );
void        bmon_prepend_temp_directory( char *filenamePath, int filenamePathLen, const char *filename );
char       *bmon_get_time_now_str( void );
unsigned long int bmon_delta_time_microseconds( unsigned long int seconds, unsigned long int microseconds );
int         bmon_get_my_ip_addr_from_ifname( const char *ifname, char *ipaddr, int ipaddr_len );
int         bmon_get_cfg_file_entry( const char* cfg_filename, const char* cfg_tagline, char* output_buffer, int output_buffer_len );
char       *bmon_date_str( void );
int         bmon_get_mac_addr( const char *ifname, char *macAddrBuffer, int macAddrBufferLen );
int         bmon_remove_colons( char *macAddrBuffer );
int         bmon_get_file_size( const char *filename );

#endif /* BMON_UTILS_H */
