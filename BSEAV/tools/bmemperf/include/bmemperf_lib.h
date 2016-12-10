/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
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
#ifndef __BMEMPERF_LIB_H__
#define __BMEMPERF_LIB_H__

#include <sys/socket.h>

#define BMEMPERF_SERVER_PORT  6000
#define BSYSPERF_SERVER_PORT  6001
#define BMEMPERF_MAX_NUM_CPUS          8
#define BMEMPERF_IRQ_NAME_LENGTH  64
#define BMEMPERF_IRQ_MAX_TYPES    120 /* number of different interrupts listed in /proc/interrupts */
#define BMEMPERF_IRQ_VALUE_LENGTH 10  /* each interrupt count is 10 digits long in /proc/interrupts */
#define BMEMPERF_CPU_VALUE_LENGTH 10  /* each CPU count is 10 digits long in /proc/stats */
#define BMEMPERF_DDR_VALUE_LENGTH 10  /* each DDR frequency is 10 digits long in /sys/devices/.../frequency */
#define BMEMPERF_SCB_VALUE_LENGTH 9   /* each SCB frequency is 9 digits long in /sys/kernel/debug/clk/clk_summary */
#define PROC_INTERRUPTS_FILE         "/proc/interrupts"
#define TEMP_INTERRUPTS_FILE         "interrupts"
#define PROC_STAT_FILE               "/proc/stat"
#define MAX_LENGTH_GETPIDOF_CMD      128

typedef struct
{
    unsigned long int irqCount[BMEMPERF_MAX_NUM_CPUS];
    unsigned long int irqCountPrev[BMEMPERF_MAX_NUM_CPUS];
    char              irqName[BMEMPERF_IRQ_NAME_LENGTH];
} bmemperf_irq_details;

typedef struct
{
    unsigned char     numActiveCpus;
    float             uptime;
    unsigned char     idlePercentage[BMEMPERF_MAX_NUM_CPUS];
    unsigned long int user[BMEMPERF_MAX_NUM_CPUS];
    unsigned long int nice[BMEMPERF_MAX_NUM_CPUS];
    unsigned long int system[BMEMPERF_MAX_NUM_CPUS];
    unsigned long int idle[BMEMPERF_MAX_NUM_CPUS];
    unsigned long int cpu_iowait[BMEMPERF_MAX_NUM_CPUS];
    unsigned long int cpu_irq[BMEMPERF_MAX_NUM_CPUS];
    unsigned long int cpu_softirq[BMEMPERF_MAX_NUM_CPUS];
    unsigned long int cpu_steal[BMEMPERF_MAX_NUM_CPUS];
} bmemperf_cpu_percent;

typedef struct
{
    unsigned long int irqTotal;
    unsigned long int contextSwitches;
} bmemperf_proc_stat_info;

typedef struct
{
    float                uptime;
    unsigned long int    irqCount[BMEMPERF_MAX_NUM_CPUS];
    unsigned long int    irqTotal;
    bmemperf_irq_details irqDetails[BMEMPERF_IRQ_MAX_TYPES];
} bmemperf_irq_data;

typedef enum /* MEMC_DDR_0_CNTRLR_CONFIG -> DRAM_DEVICE_TYPE ... can be 0, 1, 2, or 5 */
{
    MEMC_DRAM_TYPE_DDR2=0,
    MEMC_DRAM_TYPE_DDR3=1,
    MEMC_DRAM_TYPE_DDR4=2,
    MEMC_DRAM_TYPE_LPDDR4=5
} MEMC_DRAM_TYPE;

typedef enum /* MEMC_DDR_0_CNTRLR_CONFIG -> DRAM_TOTAL_WIDTH ... can be 1, 2, or 3 */
{
    MEMC_DRAM_WIDTH_16=1,
    MEMC_DRAM_WIDTH_32=2,
    MEMC_DRAM_WIDTH_64=3
} MEMC_DRAM_WIDTH;

typedef enum
{
    DVFS_GOVERNOR_CONSERVATIVE=1,
    DVFS_GOVERNOR_PERFORMANCE,
    DVFS_GOVERNOR_POWERSAVE
} DVFS_GOVERNOR_TYPES;

typedef struct
{
    unsigned int interface_bit_width; /* 16 or 32 */
    unsigned int burst_length; /* (8 for ddr3/4; 16 lpddr4) */
    char         ddr_type[8];
} BMEMPERF_BUS_BURST_INFO;

const char *noprintf( const char *format, ... );
char *getPlatformVersion(
    void
    );
char *getPlatform(
    void
    );
int Close(
    int socketFd
    );
int send_request_read_response(
    struct sockaddr_in *server,
    unsigned char      *request,
    int                 request_len,
    unsigned char      *response,
    int                 response_len,
    int                 server_port,
    int                 cmd
    );
int gethostbyaddr2(
    const char *HostName,
    int         port,
    char       *HostAddr,
    int         HostAddrLen
    );
int scanForInt(
    const char *queryRequest,
    const char *formatStr,
    int        *returnValue
    );
int scanForStr(
    const char  *queryRequest,
    const char  *formatStr,
    unsigned int returnValuelen,
    char        *returnValue
    );
char *DateYyyyMmDdHhMmSs( void );
char *HhMmSs( unsigned long int timestamp );
char *DayMonDateYear( unsigned long int timestamp );
int convert_to_string_with_commas(
    unsigned long int value,
    char             *valueStr,
    unsigned int      valueStrLen
    );
unsigned long int getSecondsSinceEpoch( void );
char *GetFileContents( const char *filename );
char *GetFileContentsSeek( const char *filename, unsigned int offset );
unsigned int GetFileLength( const char *filename );
char *GetTempDirectoryStr( void );
int get_proc_stat_info( bmemperf_proc_stat_info *pProcStatInfo );
int get_interrupt_counts( bmemperf_irq_data *irqData );
void PrependTempDirectory( char *filenamePath, int filenamePathLen, const char *filename );
int set_cpu_utilization( void );
int get_cpu_utilization( bmemperf_cpu_percent *pcpuData );
int bmemperf_getCpuUtilization( bmemperf_cpu_percent *pCpuData );
int P_getCpuUtilization( void );
char * bmemperf_get_boa_error_log( const char * appname );
unsigned int bmemperf_get_boa_error_log_line_count( const char * errorLogContents );
unsigned int bmemperf_readReg32( unsigned int offset );

#ifdef BMEMCONFIG_READ32_SUPPORTED
char * getProductIdStr( void );
#endif /* BMEMCONFIG_READ32_SUPPORTED */
int bmemperfOpenDriver( void );
void *bmemperfMmap( int g_memFd );
unsigned int bmemperf_bus_width( unsigned int memc_index, volatile unsigned int *g_pMem );
unsigned int bmemperf_burst_length( unsigned int memc_index, volatile unsigned int *g_pMem );
char        *bmemperf_get_ddrType( unsigned int memc_index, volatile unsigned int *g_pMem );
int          bmemperf_cas_to_cycle( unsigned int memc_index, volatile unsigned int *g_pMem );
volatile unsigned int *bmemperf_openDriver_and_mmap( void );
unsigned int convert_from_msec( unsigned int msec_value, unsigned int g_interval );
unsigned int get_my_ip4_addr( void );
unsigned long int  getPidOf ( const char * processName );
const char        *executable_fullpath( const char * exe_name );
const char        *get_executing_command( const char * exe_name );
#define            Bsysperf_Free(buffer)    if(buffer){free(buffer); buffer=0;}
int                replace_space_with_nbsp( char *buffer, long int buffer_size );
char              *Bsysperf_GetProcessCmdline( const char * process_name );
char              *Bsysperf_ReplaceNewlineWithNull ( char *buffer );
int                Bsysperf_RestoreNewline( char * posEol );
int                decodeURL ( char * URL );
int                get_cpu_frequency( unsigned int cpuId);
int                output_cpu_frequencies( void );
int                set_governor_control ( int cpu, DVFS_GOVERNOR_TYPES GovernorSetting );
int                get_governor_control ( int cpu );
int                get_cpu_frequencies_supported( int cpu, char *cpu_frequencies_supported, int cpu_frequencies_supported_size );
int                output_file_size_in_human( const char *sFilename );
char              *decodeFilename( const char *filename );
int                sendFileToBrowser( const char *filename);
int                readFileFromBrowser( const char *contentType, const char *contentLength, char *sFilename, unsigned long int lFilenameLen);

#endif /* __BMEMPERF_LIB_H__ */
