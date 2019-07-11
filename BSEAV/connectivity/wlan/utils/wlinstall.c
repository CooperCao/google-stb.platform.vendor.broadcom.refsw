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
 *"wlinstall" utlity automatically does the necessary intilization to run generic wifi driver on any board.
 *
 *Usage:
 *wlinstall <name of the driver binary(optional)> <name of the interface(Optional)>
 *Examples NIC chips:
 *1. #wlinstall --- Just turn on the wifi radio power . After this step user can do insmod to install the driver
 *2. #wlinstall wl.ko --- Turn on the radio power,copy the nvram file,remove existing wl.ko from kernel
 * and reinstall wl.ko. After this just run the wl commands as usual. Utlity expects nvram file in the current
 * directory and to be in the format boardname.txt e.g bcm97271wlan.txt,bcm97271ip.txt
 *3. #wlinstall wl.ko wlan0  --- Turn on the radio power,copy the nvram file,remove existing wl.ko from kernel
 * and reinstall wl.ko, changes the ame of the network interface to wlan0 . After this just run the wl commands
 * as usual. Utlity expects nvram file in the current directory and to be in the format boardname.txt e.g bcm97271wlan.txt,bcm97271ip.txt
 *Examples Full Dongle chips:
 *1. #wlinstall --- Installs default dhd.ko,firmware, nvram, clm_blob. see Usage
 *2. #wlinstall -d path/dhd.ko - --- Turn on the radio power, use  the nvram file that matches boardname.txt,remove existing dhd.ko from kernel
 * and reinstall dhd.ko. After this just run the wl commands as usual. Utlity expects defaults and  nvram file in the current
 * directory and  nvram to be in the format boardname.txt e.g bcm97271wlan.txt,bcm97271ip.txt
 *
 *How to build:
 * arm-linux-gcc -o wlinstall wlinstall.c
 * or aarch64-linux-gcc -o wlinstall wlinstall.c for 64 bit toolchain
 *
 *What does "wlinstall" do:
 *1. Reads the board information and turns on the wifi radio
 *2. Copies the board specific nvram file to standard nvram.txt e.g bcm97271wlan.txt -> nvram.txt
 *  a. For Full dongle based designs. it will look for nvram file based on board name detected. ie bcm972181stk.txt
 *3. Update the wifi mac address to eth0 mac address + 1
 *4. If the name of the driver binary specified then
 *	 a) Tries to remove the existing driver
 *	 b) install the driver using insmod command
 *5. If the interface name is specifed, is changes the interfcace name otherwise driver picked interface
 *   name is used whic is usually ethX
 *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>

#define BOARD_NAME "/proc/device-tree/bolt/board"
#define SYSFS_GPIO "/sys/class/gpio"
#define AON_GPIO_OFFSET 128
#define AON_GPIO_OFFSET_72161 64

static struct
{
    char * board_name;
    int gpio_num;
}board_info[] = {
                    /* {"BCM97271SV",-1}, */
                    {"BCM97271IP",(AON_GPIO_OFFSET+15)},
                    {"BCM97271IPCQ",(AON_GPIO_OFFSET+15)},
                    {"BCM97271DV",-1},
                    {"BCM97271C",(AON_GPIO_OFFSET+15)},
                    {"BCM97271T",(AON_GPIO_OFFSET+15)},
                    {"BCM97271WLAN",(AON_GPIO_OFFSET+21)},
                    {"BCM97271_4L",(AON_GPIO_OFFSET+21)},
                    {"BCM97271D",(AON_GPIO_OFFSET+21)},
                    {"BCM97271IPC",(AON_GPIO_OFFSET+15)},
                    {"BCM97271HB",(AON_GPIO_OFFSET+15)},
                    {"BCM972180DV",-1},
                    {"BCM972180HB",-1},
                    {"BCM972180SV",-1},
                    {"BCM972180VC",-1},
                    {"BCM972181SKT",-1},
                    {"BCM972181STK",-1},
                    {"BCM972181SV",-1},
                    {NULL,0},
                };

static int get_board_name(char *board_name, int size)
{
    FILE *fd = NULL;
    char *tmp = NULL;

    fd= fopen(BOARD_NAME, "r" );
    if(fd == NULL)
    {
        printf("%s: Failed to open %s\n",__FUNCTION__,BOARD_NAME);
        return -1;
    }
    memset((void*)board_name,0,size);
    tmp = fgets(board_name,size,fd);
    if(tmp == NULL)
    {
        printf("%s:Failed to read board name\n",__FUNCTION__,BOARD_NAME);
        fclose(fd);
        return -1;
    }
    fclose(fd);
    return 0;
}

static int wlan_radio_on(int gpio)
{
    char buf[256];
    int fd = 0;

    printf("*** Turning on wlan radio using %s gpio pin %d***\n",
           gpio>AON_GPIO_OFFSET?"aon":"",gpio>AON_GPIO_OFFSET?(gpio-AON_GPIO_OFFSET):gpio);
    if(gpio <0 )
    {
        return 0;
    }

    /* remove the gpio pin if already exported */
    snprintf(buf, sizeof(buf), "%s/unexport", SYSFS_GPIO);
    fd = open(buf, O_WRONLY);
    if (fd == 0)
    {
        printf("%s(): Unable to open %s\n", __FUNCTION__, buf);
        goto error;
    }
    snprintf(buf, sizeof(buf),"%d",gpio);
    write(fd, buf, strlen(buf));
    close(fd);

    /* export gpio */
    snprintf(buf, sizeof(buf), "%s/export", SYSFS_GPIO);
    fd = open(buf, O_WRONLY);
    if (fd == 0)
    {
        printf("%s(): Unable to open %s\n", __FUNCTION__, buf);
        goto error;
    }
    snprintf(buf, sizeof(buf),"%d",gpio);
    if (write(fd, buf, strlen(buf)) < 0)
    {
        printf("%s(): Not able to write gpio %d to %s/export\n", __FUNCTION__,gpio,SYSFS_GPIO);
        goto error;
    }
    close(fd);

    /* change the direction of the pin to output */
    snprintf(buf, sizeof(buf), "%s/gpio%d/direction", SYSFS_GPIO, gpio);
    if ((fd = open(buf, O_WRONLY)) < 0)
    {
        printf("%s(): Unable to open %s/gpio%d/direction\n", __FUNCTION__, SYSFS_GPIO, gpio);
        goto error;
    }
    snprintf(buf, sizeof(buf), "%s", "out");
    if (write(fd, buf, strlen(buf)) < 0)
    {
        printf("%s(): Not able to write %s to %s/gpio%d/direction\n", __FUNCTION__,buf,SYSFS_GPIO, gpio);
        goto error;
    }
    close(fd);

    /* turn on the radio */
    snprintf(buf, sizeof(buf), "%s/gpio%d/value", SYSFS_GPIO, gpio);
    if ((fd = open(buf, O_WRONLY)) < 0)
    {
        printf("%s(): Failed to Open %s/gpio%d/value\n", __FUNCTION__,SYSFS_GPIO, gpio);
        goto error;
    }
    snprintf(buf, sizeof(buf),"%d",1);
    if (write(fd, buf, strlen(buf)) < 0)
    {
        printf("%s(): Not able to write %s to %s/gpio%d/value\n", __FUNCTION__,buf,SYSFS_GPIO, gpio);
        goto error;
    }
    close(fd);

    /*Done with GPIO unexport... */
    snprintf(buf, sizeof(buf), "%s/unexport", SYSFS_GPIO);
    fd = open(buf, O_WRONLY);
    if (fd == 0)
    {
        printf("%s(): Unable to open %s\n", __FUNCTION__, buf);
        goto error;
    }
    snprintf(buf, sizeof(buf),"%d",gpio);
    write(fd, buf, strlen(buf));
    close(fd);

    return 0;

error:
    if (fd > 0)
    {
        close(fd);
    }
    return -1;
}


#define SYSFS_ETH0_MAC "/sys/class/net/eth0/address"

int update_mac_addr(void)
{
    FILE *fd = NULL;
    char mac[]="00:00:00:00:00:00";
    char system_command[]="sed -i '/macaddr/c\\macaddr=00:00:00:00:00:00' nvram.txt";
    unsigned m[6];
    int i=0;
    fd= fopen(SYSFS_ETH0_MAC, "r" );
    if(fd == NULL)
    {
        printf("%s: Failed to open %s\n",__FUNCTION__,SYSFS_ETH0_MAC);
        return -1;
    }
    if(fgets((char *)mac,sizeof(mac),fd)==NULL)
    {
        printf("Error:Unable to read eth0 mac address\n");
        fclose(fd);
        return -1;
    }
    printf("Board eth0 mac address = %s\n",mac);
    sscanf(mac, "%x:%x:%x:%x:%x:%x", &m[0], &m[1], &m[2], &m[3], &m[4], &m[5]);
    for (i=5;i>=0;i--)
    {
        if (m[i]==0xff)
            continue;
        m[i]++;
        break;
    }
    sprintf(mac,"%02x:%02x:%02x:%02x:%02x:%02x",m[0], m[1], m[2], m[3], m[4], m[5]);
    printf("*** Setting board wlan0 mac addrees to %s ***\n",mac);
    sprintf(system_command,"sed -i '/macaddr/c\\macaddr=%s' nvram.txt",mac);
    system(system_command);
    /* Make sure changes to file makes it the fs */
    system("sync");
    fclose(fd);
    return 0;
}

static void usage(void)
{
	printf( "Usage: wlinstall <driver name(wl.ko will be used )> <intefacename(wlan0 will be used ) <name of nvram file (or else local path will be tried)>\n" );
	printf( "\n\n \t******************************\t \n\n");
	printf( "Usage for dhd: wlinstall  [-d <dhd module(defaults: dhd.ko)>]\n" );
	printf( "\t[-m <dhd_msg_level(default: 1)>]\n");
	printf( "\t[-i <intefacename(default: wlan0)>]\n");
	printf( "\t[-f  <firmware file(default: config_pcie_stb_release.bin)>]\n");
	printf( "\t[-n <nvram file(default: <boardname>.txt)>]\n");
	printf( "\t[-c <clm_blob(default: 4375b1.clm_blob)>]\n");
	printf( "Examples: wlinstall /*No options will load all defaults*/ \n" );
	printf( "Examples: wlinstall -d dhd-msgbuf-pciefd-cfg80211-stb-armv8-debug-4.9.135.1.11.ko -f config_pcie_stb_cert.bin /* specified dhd.ko and firmware,rest DEFAULTS \n" );

} /* usage */


#define BUF_SIZE 1024
#define COMMAND_BUF_SIZE 2048
#define DEFAULT_IFACE_NAME "wlan0"
#define DEFAULT_NVRAM_PATH "nvram.txt"
#define PATH_SIZE 256
#define BCM972181 "972181"
#define BCM972180 "972180"
#define BCM972161 "972161"

/* Call this function when board detects 4375 based MCM */
int dhd_install(int argc, char **argv, char* boardname)
{
	int opt;
	int size=0, i=0;
	char *buf=NULL;
	FILE *fd=NULL;
	char *command_buf=NULL;
	char *nvram_name=NULL;
	char dhd_file[PATH_SIZE]="dhd.ko";  /*  -d */
	char dhd_path[PATH_SIZE];
	char nvram_file[PATH_SIZE]="bcm972181.txt";  /*  -n */
	char nvram_path[PATH_SIZE];
	char firmware_file[PATH_SIZE]="config_pcie_stb_release.bin"; /* -f */
	char firmware_path[PATH_SIZE];
	char clm_file[PATH_SIZE]="4375b1.clm_blob";  /* -c */
	char clm_path[PATH_SIZE];
	char iface_name[PATH_SIZE]="iface_name=wlan0";  /* -i: default iface_name=wlan0 */
	char dhd_msg_level[PATH_SIZE]="dhd_msg_level=1";  /* -m: default 1*/

	printf( "dhd_install: Inside dhd install: boardname: %s\n", boardname );
	/*Get board specific nvram file */
	{
		size = strlen(boardname);

		if(size)
		{
			nvram_name = (char *)malloc(PATH_SIZE);
			memset((void*)nvram_name,0,PATH_SIZE);
			for(i=0;i<size;i++)
				nvram_name[i] = (char)tolower(boardname[i]);
			snprintf(nvram_file, sizeof(nvram_file),"%s.txt",nvram_name);
			printf("*** nvram file name is	%s ***\n", nvram_file);
		}
	}

	while((opt = getopt(argc, argv, "n:f:c:i:m:d:")) != -1)
	{
		switch(opt)
		{
			case 'n':
				snprintf(nvram_file, sizeof(nvram_file),"%s",optarg);
				printf("nvram_file: %s\n",nvram_file);
				break;
			case 'c':
				snprintf(clm_file, sizeof(clm_file),"%s",optarg);
				printf("clm_file: %s\n",clm_file);
				break;
			case 'i':
				snprintf(iface_name, sizeof(iface_name),"iface_name=%s",optarg);
				printf("if_name: %s\n",iface_name);
				break;
			case 'f':
				snprintf(firmware_file, sizeof(firmware_file),"%s",optarg);
				printf("firmware_file: %s\n",firmware_file);
				break;
			case 'm':
				snprintf(dhd_msg_level, sizeof(dhd_msg_level),"dhd_msg_level=%s",optarg);
				printf("dhd_msg_level: %s\n",dhd_msg_level);
				break;
			case 'd':
				snprintf(dhd_file, sizeof(dhd_file),"%s",optarg);
				printf("dhd_file: %s\n",dhd_file);
				break;
			case ':':
				printf("option needs a value\n");
				break;
			case '?':
				printf("unknown option: %c\n", optopt);
				break;
		}
	}

	/* Check if file exist */
	if( access(firmware_file, F_OK) == -1 ) {
		printf("\n -------Error: Can't find firmware file (%s)-----------\n",firmware_file);
		return -1;
	}
	if( access(dhd_file, F_OK) == -1 ) {
		printf("\n -------Error: Can't find dhd.ko (%s)-----------\n",dhd_file);
		return -1;
	}
	if( access(nvram_file, F_OK) == -1 ) {
		printf("\n -------Error: Can't find nvram file(%s)-----------\n",nvram_file);
		return -1;
	}
	if( access(clm_file, F_OK) == -1 ) {
		printf("\n -------Error: Can't find clm_blob file(%s)-----------\n",clm_file);
		return -1;
	}


    /* Construct parameters */
	snprintf(nvram_path, sizeof(nvram_path),"nvram_path=%s",nvram_file);
	snprintf(clm_path, sizeof(clm_path),"clm_path=%s",clm_file);
	snprintf(firmware_path, sizeof(firmware_path),"firmware_path=%s",firmware_file);
	snprintf(dhd_path, sizeof(dhd_path),"%s",dhd_file);

	printf("\n -------Parameter values-----------\n");
	printf("dhd_path: %s\n",dhd_path);
	printf("dhd_msg_level: %s\n",dhd_msg_level);
	printf("firmware_path: %s\n",firmware_path);
	printf("if_name: %s\n",iface_name);
	printf("nvram_path: %s\n",nvram_path);
	printf("clm_path: %s\n",clm_path);


	command_buf = (char *)malloc(COMMAND_BUF_SIZE);
	buf = (char *)malloc(BUF_SIZE);
	if (buf == NULL)
	{
	   printf("***Failed to allocate memory %d***\n",BUF_SIZE);
	   goto error;
	}

	 /* Clean up any prior installed kernel drivers */
    memset((void*)buf,0,BUF_SIZE);
    fd = popen("lsmod | grep -w dhd", "r");
    if(fread(buf, 1, sizeof(buf), fd) > 0) {
        memset((void*)command_buf, 0, COMMAND_BUF_SIZE);
        snprintf(command_buf, COMMAND_BUF_SIZE, "rmmod %s", "dhd");
        printf("*** Uninstall driver: %s ***\n", command_buf);
        sleep(1);
        system(command_buf);
    }
    else {
        printf("*** Module dhd is not loaded\n");
    }

	/* Install dhd driver*/
    {
        memset((void*)command_buf,0,COMMAND_BUF_SIZE);
        snprintf(command_buf, COMMAND_BUF_SIZE,"insmod %s %s %s %s %s %s",dhd_path,dhd_msg_level,iface_name,firmware_path,nvram_path,clm_path);
        printf("*** Installing driver: %s\n",command_buf);
        sleep(1);
        system(command_buf);
    }
    /* Install wl driver*/

    free(buf);
    free(command_buf);

    return 0;
error:
    if (buf)
        free(buf);
    if (command_buf)
        free(command_buf);

    return 1;
}


int main(int argc, char **argv)
{
    int board_index=0,i=0;
    char *buf=NULL;
    char *nvram_name=NULL;
    char *command_buf=NULL;
    char if_name[PATH_SIZE];
    char nvram_path[PATH_SIZE];
    int size=0;
    FILE *fd=NULL;
    char *wlpath=NULL;
    unsigned int pathlen=0;

    if(argc == 2)
    {
        if(strcmp("-h", argv[1]) == 0) {
            usage();
            return 0;
        }
    }

    command_buf = (char *)malloc(COMMAND_BUF_SIZE);
    buf = (char *)malloc(BUF_SIZE);
    if (buf == NULL)
    {
        printf("***Failed to allocate memory %d***\n",BUF_SIZE);
        goto error;
    }
    memset((void*)if_name,0,sizeof(if_name));

    /* check the board and turn on wifi radio */
    memset((void*)buf,0,BUF_SIZE);
    get_board_name(buf, BUF_SIZE);

	/* replace BCX with BCM, BCX972181(LPDDR4x) should be renamed to BCM972181 for nvram purposes*/
	if(buf[0] == 'B' && buf[1] == 'C' && buf[2] == 'X')
	{
		printf("***Board name %s board ***\n", buf);
		buf[2] = 'M';
		printf("***Treat as %s board for nvram purposes ***\n", buf);
	}
	/* replace  BCX97219XXX(LPDDR4x) should be renamed to BCM97218XXX for nvram purposes*/
	if(buf[3] == '9' && buf[4] == '7' && buf[5] == '2' && buf[6] == '1' && buf[7] == '9')
	{
		printf("***Board name %s board ***\n", buf);
		buf[7] = '8';
		printf("***Treat as %s board for nvram purposes ***\n", buf);
	}

    board_index=0;
    while(board_info[board_index].board_name)
    {
        if(strcasecmp(board_info[board_index].board_name, buf) == 0)
        {
            printf("*** Board[%s] Found! ***\n",board_info[board_index].board_name);
            if(wlan_radio_on(board_info[board_index].gpio_num))
            {
                printf("Failed to turn on radio for %s board on gpio %d\n",board_info[board_index].board_name,board_info[board_index].gpio_num);
             }
             break;
        }
        board_index++;
    }

    if(board_info[board_index].board_name == NULL)
    {
        printf("*** Add support for %s if PA is controlled through GPIO ***\n",buf);
    }
    /* check the board and turn on wifi radio */

	/**************************************************/
	/* If board is dhd based board.  972181 or 972161 */
	/**************************************************/
    if(strstr(buf,BCM972181) != NULL || strstr(buf,BCM972180) != NULL || strstr(buf,BCM972161) != NULL)
    {
		printf("%s board treated as an MCM(has 4375 full dongle wifi)\n",buf);
		dhd_install(argc, argv, buf);
		printf("Finished wlinstall for 4375 based wifi\n");
		goto done;
    }

    /*Get board specific nvram file */
    {
        size = strlen(buf);
        if(size)
        {
            nvram_name = (char *)malloc(PATH_SIZE);
            memset((void*)nvram_name,0,PATH_SIZE);
            for(i=0;i<size;i++)
                nvram_name[i] = (char)tolower(buf[i]);
            printf("*** nvram file name is  %s ***\n", nvram_name);
        }
    }
    /*Get board specific nvram file */


    /* Clean up any prior installed kernel drivers */
    memset((void*)buf,0,BUF_SIZE);
    fd = popen("lsmod | grep -w wl", "r");
    if(fread(buf, 1, sizeof(buf), fd) > 0) {
        memset((void*)command_buf, 0, COMMAND_BUF_SIZE);
        snprintf(command_buf, COMMAND_BUF_SIZE, "rmmod %s", "wl");
        printf("*** Uninstall driver: %s ***\n", command_buf);
        sleep(1);
        system(command_buf);
    }
    else {
        printf("*** Module wl is not loaded\n");
    }

    memset((void*)buf,0,BUF_SIZE);
    fd = popen("lsmod | grep -w wlplat","r");
    if(fread(buf, 1, sizeof(buf), fd) > 0) {
        memset((void*)command_buf, 0, COMMAND_BUF_SIZE);
        snprintf(command_buf, COMMAND_BUF_SIZE, "rmmod %s", "wlplat.ko");
        printf("*** Uninstall driver: %s ***\n", command_buf);
        sleep(1);
        system(command_buf);
    }
    else {
        printf("*** Module wlplat is not loaded\n");
    }
    /* Clean up any prior installed kernel drivers */

    /*Parse arguments */
    if(argc >= 2){
        wlpath = (char *)malloc(PATH_SIZE);
        if (wlpath == NULL)
        {
            printf("***Failed to allocate memory %d***\n",PATH_SIZE);
            goto error;
        }
        memset((void*)wlpath,0,PATH_SIZE);
        pathlen=strlen(argv[1]);
        if (pathlen>4)
            memcpy(wlpath,argv[1],pathlen-5);
    }

    if(argc >= 3)
            snprintf(if_name, sizeof(if_name),"intf_name=%s",argv[2]);
    else
            snprintf(if_name, sizeof(if_name),"intf_name=%s",DEFAULT_IFACE_NAME);

    if(argc >= 4)
    {
        memset((void*)nvram_path,0,sizeof(nvram_path));
        snprintf(nvram_path, sizeof(nvram_path),"nvram_path=%s",argv[3]);
    } else if(nvram_name) {
        memset((void*)command_buf,0,COMMAND_BUF_SIZE);
        snprintf(command_buf, COMMAND_BUF_SIZE,"ls | grep -w %s",nvram_name);

        memset((void*)buf,0,BUF_SIZE);
        fd = popen(command_buf,"r");
        if(fread(buf, 1, sizeof(buf), fd) > 0) {
            snprintf(nvram_path, sizeof(nvram_path), "nvram_path=%s.txt",nvram_name);
        } else {
            memset((void*)command_buf,0,COMMAND_BUF_SIZE);
            snprintf(command_buf, COMMAND_BUF_SIZE,"ls %s | grep -w %s",wlpath,nvram_name);

            memset((void*)buf,0,BUF_SIZE);
            fd = popen(command_buf,"r");
            if(fread(buf, 1, sizeof(buf), fd) > 0)
                snprintf(nvram_path, sizeof(nvram_path), "nvram_path=%s%s.txt",wlpath,nvram_name);
        }
    } else {
        snprintf(nvram_path, sizeof(nvram_path),"nvram_path=%s",DEFAULT_NVRAM_PATH);
    }
    /*Parse arguments */


    /* Install plat driver*/
    {
        memset((void*)command_buf,0,COMMAND_BUF_SIZE);
        if(wlpath)
            snprintf(command_buf, COMMAND_BUF_SIZE,"insmod %swlplat.ko",wlpath);
        else
            snprintf(command_buf, COMMAND_BUF_SIZE,"insmod %s","wlplat.ko");
        printf("*** Installing driver: %s ***\n",command_buf);
        sleep(1);
        system(command_buf);
    }
    /* Install plat driver*/

    /* Install wl driver*/
    {
        memset((void*)buf,0,BUF_SIZE);
        if(wlpath)
            snprintf(buf, BUF_SIZE,"%swl.ko",wlpath);
        else
            snprintf(buf, BUF_SIZE,"%s","wl.ko");
        memset((void*)command_buf,0,COMMAND_BUF_SIZE);
        snprintf(command_buf, COMMAND_BUF_SIZE,"insmod %s %s %s",buf,if_name,nvram_path);
        printf("*** Installing driver: %s\n",command_buf);
        sleep(1);
        system(command_buf);
    }
    /* Install wl driver*/
done:
    free(buf);
    free(command_buf);
    free(nvram_name);
    free(wlpath);
    return 0;
error:
    if (buf)
        free(buf);
    if (command_buf)
        free(command_buf);
    if (nvram_name)
        free(nvram_name);
    if (wlpath)
        free(wlpath);
    return 1;
}
