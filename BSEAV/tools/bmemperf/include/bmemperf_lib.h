/******************************************************************************
 *    (c)2013-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELYn
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ******************************************************************************/
#ifndef __BMEMPERF_LIB_H__
#define __BMEMPERF_LIB_H__

#define BMEMPERF_SERVER_PORT  6000
#define BSYSPERF_SERVER_PORT  6001
#define BMEMPERF_MAX_NUM_CPUS          8
#define BMEMPERF_IRQ_NAME_LENGTH  64
#define BMEMPERF_IRQ_MAX_TYPES    100 /* number of different interrupts listed in /proc/interrupts */
#define BMEMPERF_IRQ_VALUE_LENGTH 10  /* each interrupt count is 10 digits long in /proc/interrupts */
#define BMEMPERF_CPU_VALUE_LENGTH 10  /* each CPU count is 10 digits long in /proc/stats */
#define PROC_INTERRUPTS_FILE         "/proc/interrupts"
#define TEMP_INTERRUPTS_FILE         "interrupts"
#define PROC_STAT_FILE               "/proc/stat"

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
char *GetTempDirectoryStr( void );
int get_proc_stat_info( bmemperf_proc_stat_info *pProcStatInfo );
int get_interrupt_counts( bmemperf_irq_data *irqData );
void PrependTempDirectory( char *filenamePath, int filenamePathLen, const char *filename );
int set_cpu_utilization( void );
int get_cpu_utilization( bmemperf_cpu_percent *pcpuData );
int bmemperf_getCpuUtilization( bmemperf_cpu_percent *pCpuData );
int P_getCpuUtilization( void );

#endif /* __BMEMPERF_LIB_H__ */
