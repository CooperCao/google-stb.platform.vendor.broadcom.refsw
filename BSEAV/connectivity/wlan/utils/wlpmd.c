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
 *****************************************************************************/

/******************************************************************************
 *"wlpmd" is a daemon that automatically scales the station power window based on station 
 *  thruput demans 
 *
 *Usage:
 * wlpmd 
 *Examples:
 *1. 
 *2. 
 *How to build:
 * arm-linux-gcc -o wlpmd wlpmd.c
 * or aarch64-linux-gcc -o wlpmd wlpmd.c for 64 bit toolchain
 *
 *What does "wlpmd" do:
 *1. Monitors 
 *   A. wlan current data rate
 *   B. video buffers
 *2. Automtically scales the power window
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>
#include <sys/time.h>
#include <string.h>

#define DEFAULT_WLAN_IFNAME "wlan0"

static void usage(void)
{
    printf( "Usage: wlpmd\n" );
} /* usage */

/* read network rx and tx bytes count */
int get_network_bytes(char* if_name, unsigned int *rx_bytes,unsigned int *tx_bytes)
{
#if 0
    struct ifaddrs *ifaddr, *ifa;
    int family;
    *rx_bytes = *tx_bytes = 0;

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return -1;
    }
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;
        if (strcmp(ifa->ifa_name,if_name))
            continue;

        family = ifa->ifa_addr->sa_family;
        if (family == AF_PACKET && ifa->ifa_data != NULL)
        {
            struct rtnl_link_stats *stats = (struct rtnl_link_stats *)ifa->ifa_data;
            *rx_bytes = stats->rx_bytes;
            *tx_bytes = stats->tx_bytes;
            /*printf("\t\ttx_packets = %10u; rx_packets = %10u\n"
                   "\t\ttx_bytes   = %10u; rx_bytes   = %10u\n",
                   stats->tx_packets, stats->rx_packets,
                   stats->tx_bytes, stats->rx_bytes);*/
            break;
        }
    }
    freeifaddrs(ifaddr);
    return 0;
#else
    FILE *fp;
    char buf[512];
    char *tmp = NULL;
    int counter=0;
    *rx_bytes = *tx_bytes =0;
    /* fp = popen("/bin/ethtool -S  wlan0", "r");*/
    fp = popen("wl counters", "r");
    while (fgets(buf, 64, fp) != NULL)
    {
        if(tmp = strstr(buf,"txbyte"))
        {
            sscanf(tmp,"txbyte %u",tx_bytes);
            /* printf("%u\n",*tx_bytes);*/
            if(++counter >= 2)
                break;
            continue;
        }
        if(tmp = strstr(buf,"rxbyte"))
        {
            sscanf(tmp,"rxbyte %u",rx_bytes);
            /*printf("%u\n",*rx_bytes);*/
            if(++counter >= 2)
                break;
            continue;
        }
        //printf("%s", buf);
    }
    pclose(fp);
    return 0;
#endif
}
/* network bit rate since last read in Bytes per second */
int bit_rate_since_last_read(char* if_name,unsigned int *rx_bps, unsigned int *tx_bps)
{
    static struct timeval prev_time = {0,0};
    static unsigned int prev_rx_bytes = 0;
    static unsigned int prev_tx_bytes =0;
    struct timeval current_time = {0,0};
    unsigned int rx_bytes=0;
    unsigned int tx_bytes=0;
    double elapsed_time_sec =0;

    /* get time and network date count */
    get_network_bytes(if_name,&rx_bytes,&tx_bytes);
    gettimeofday(&current_time, NULL);


    /* compute and print the elapsed time in millisec */
    elapsed_time_sec = (current_time.tv_sec - prev_time.tv_sec);
    elapsed_time_sec += (current_time.tv_usec - prev_time.tv_usec) / 1000000.0;
    /* printf("elapsed_time_sec = %lf\n",elapsed_time_sec); */

    /* compute the bit rate*/
    *rx_bps = (rx_bytes - prev_rx_bytes)/elapsed_time_sec;
    *tx_bps = (tx_bytes - prev_tx_bytes)/elapsed_time_sec;

    /* save */
    prev_rx_bytes = rx_bytes;
    prev_tx_bytes = tx_bytes;
    prev_time.tv_sec = current_time.tv_sec;
    prev_time.tv_usec = current_time.tv_usec;
    return 0;
}
#if 0
#define MAX_PM2_RCV_DUR_COARSE_STEPS 15
static unsigned char pm2_rcv_dur_coarse[MAX_PM2_RCV_DUR_COARSE_STEPS] = {10,15,20,25,30,35,40,45,50,55,60,65,70,75,80};
#else
#define MAX_PM2_RCV_DUR_COARSE_STEPS 19
static unsigned char pm2_rcv_dur_coarse[MAX_PM2_RCV_DUR_COARSE_STEPS] = {10,14,18,22,26,30,34,38,42,46,50,54,58,62,66,70,74,78,80};
#endif

typedef enum 
{
    down=0,
    up=1,
    no_change=2
}direction;
char *direction_str[] = {"down","up","no_change"};

typedef enum 
{
    coarse=0,
    fine
}pm_adjustment;

char *pm_adjustment_str[] = {"coarse","fine"};

typedef struct
{
    direction dir;
    pm_adjustment adjustment;
}wlpmd_state;

int main(int argc, char **argv)
{
    unsigned int rx_bps=0, tx_bps=0;
    unsigned int prev_rx_bps=0, prev_tx_bps=0;
    int pm2_rcv_dur_step = MAX_PM2_RCV_DUR_COARSE_STEPS-1;
    int rx_diff;
    int pm2_rcv_dur; 
    direction prev_direction = no_change;
    direction new_direction = no_change;
    wlpmd_state state = {no_change,coarse};
    char cmd_buf[]="wl pm2_rcv_dur 100";
    int cmd_buf_size = strlen(cmd_buf);
    system("wl PM 2");
    system("wl pm2_sleep_ret 200");
    snprintf(cmd_buf,cmd_buf_size,"wl pm2_rcv_dur %d",pm2_rcv_dur_coarse[pm2_rcv_dur_step]);
    system(cmd_buf);
    int pm2_rcv_dur_value = pm2_rcv_dur_coarse[pm2_rcv_dur_step];
    bit_rate_since_last_read(DEFAULT_WLAN_IFNAME,&prev_rx_bps, &prev_rx_bps);
    sleep(1);
    bit_rate_since_last_read(DEFAULT_WLAN_IFNAME,&prev_rx_bps, &prev_rx_bps);
    while (1)
    {
        usleep(1000000);
        bit_rate_since_last_read(DEFAULT_WLAN_IFNAME,&rx_bps, &tx_bps);
        rx_bps = (rx_bps + prev_rx_bps)/2;
        rx_diff = rx_bps - prev_rx_bps;
        printf("rx=%uMbps,change=(%s)%u%% ",rx_bps*8/1000000,rx_diff<0?"-":"+",abs(rx_diff)*100/(prev_rx_bps?prev_rx_bps:1));
        if (abs(rx_diff) <  (prev_rx_bps * 1 /100))
        {
            if (rx_diff > 0)
            {
                prev_rx_bps = rx_bps;
            }
            printf("%s",pm_adjustment_str[state.adjustment]);
            if(state.adjustment == coarse)
            {
                /* load new value */
                pm2_rcv_dur_value = pm2_rcv_dur_coarse[pm2_rcv_dur_step];
                state.adjustment = fine;
            }
            if(pm2_rcv_dur_value > pm2_rcv_dur_coarse[0])
            {
                pm2_rcv_dur_value--;
                if(pm2_rcv_dur_value == pm2_rcv_dur_coarse[pm2_rcv_dur_step-1])
                {
                    pm2_rcv_dur_step--;
                }
            }
            snprintf(cmd_buf,cmd_buf_size,"wl pm2_rcv_dur %02d",pm2_rcv_dur_value);
            system(cmd_buf);
            prev_direction = down;
            state.dir = down;
            state.adjustment = fine;
            printf("->%s pm2_rcv_dur_value=%d\n",pm_adjustment_str[state.adjustment],pm2_rcv_dur_value);    
            continue;
        }
        state.adjustment =coarse;
        switch(prev_direction)
        {
        case up:
        case no_change:
            if (rx_diff <= 0)
            {
                /* bit rate went down go down */
                if (pm2_rcv_dur_step > 0)
                {
                    pm2_rcv_dur_step--;
                    new_direction = down;
                }
                else
                {
                    new_direction = no_change;
                }
            }
            else
            {
                /* bit rate went up go up one more step  */
                if (pm2_rcv_dur_step < (MAX_PM2_RCV_DUR_COARSE_STEPS-1))
                {
                    pm2_rcv_dur_step++;
                    new_direction = up;
                }
                else
                {
                    new_direction = no_change;
                }

            }
            break;
        case down:
            if (rx_diff == 0)
            {
                /* no change in bit rate go down */
                if (pm2_rcv_dur_step > 0)
                {
                    pm2_rcv_dur_step--;
                    new_direction = down;
                }
                else
                {
                    new_direction = no_change;
                }
            }
            else
            {
                if (pm2_rcv_dur_step < (MAX_PM2_RCV_DUR_COARSE_STEPS-1))
                {
                    pm2_rcv_dur_step++;
                    new_direction = up;
                }
                else
                {
                    new_direction = no_change;
                }
            }
            break;
        }
        snprintf(cmd_buf,cmd_buf_size,"wl pm2_rcv_dur %02d",pm2_rcv_dur_coarse[pm2_rcv_dur_step]);
        printf(" %s->%s => %d\n",direction_str[prev_direction],direction_str[new_direction],pm2_rcv_dur_coarse[pm2_rcv_dur_step]);
        if(new_direction != no_change)
        {
            system(cmd_buf);
        }
        prev_rx_bps = rx_bps;
        prev_direction = new_direction;
    }
}
