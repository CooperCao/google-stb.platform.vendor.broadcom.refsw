/***************************************************************************
 *     (c)2007-2011 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *  ANY LIMITED REMEDY..
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 * Media storage disk utility
 * 
 * msutil gives utility functions to for dvr extension library media storage.
 * it does create, format, mount and unmount. it is wrtten only with standard libraries 
 * so that it can be compiled and used as a stand alone utility.
 * msapp.c gives user interface.
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "msutil.h"

void print_menu(void)
{
    printf("\n==================================\n");
	printf("2: print partitions\n");
	printf("3: create partition\n");
	printf("4: check volume\n");
	printf("5: format volume\n");
	printf("6: mount volume\n");
	printf("7: unmount volume\n");
	printf("9: zap hard disk\n");
	printf("99: exit\n");
    printf("==================================\n");
}

void create_partition(const char *hdd)
{
	int ret;
	char cmdstr[256];
    MS_createPartitionSettings_t settings;

    printf("enter 0 for default.\n");
    printf("start sector number: "); 
	fgets(cmdstr, 256, stdin); 
    settings.startSec = atoi(cmdstr);

    printf("size (MBytes): "); 
	fgets(cmdstr, 256, stdin);
    settings.size = atoi(cmdstr); 

    printf("create volume starting %d size %d MByes\n",(int)settings.startSec, (int)settings.size);
    ret = ms_create_volume(hdd,&settings);
    if(ret<0) {
        printf("ms_create_volume failed:%d\n", ret);
        return;
    }

    printf("%s: %s %s %s\n", hdd, settings.mediaPartition, settings.navPartition, settings.metadataPartition);
    printf("done\n");

}

void check_volume(const char *hdd)
{
	int ret;
    MS_StorageStatus_t result;

    ret = ms_check_volume(hdd,&result);
    if (ret<0) 
        printf("ms_check_volume failed: %d\n", ret);
    
    switch (result.state) {
        case eMS_StateInvalid:
            printf("%s is not a valid media volume.\n",hdd);
            break;
        case eMS_StateEmpty:
            printf("%s is a empty media volume.\n",hdd);
            break;
        case eMS_StateReady:
            printf("%s is ready to be used as a media volume.\n",hdd);
            break;
        default:
            printf("%s error.\n",hdd);
            break;
    }
    printf("done\n");

}

void format_volume(const char *hdd)
{
	int ret;

    ret = ms_format_volume(hdd);
    printf("done\n");

}

void mount_volume(const char *hdd)
{
	int ret;
    char mountname[50];
    int deleted;

    ret = ms_mount_volume(hdd, mountname,&deleted);
    if (ret == eMS_OK) printf("mountname is %s\n", mountname);
    else printf("mount failed\n");
}

void unmount_volume(const char *hdd)
{
	int ret;

    ret = ms_unmount_device(hdd);
    printf("done\n");

}

void zap_disk(const char *hdd)
{
    char cmdstr[50];

    sprintf(cmdstr, "sgdisk -og %s",hdd);
    system(cmdstr);
}

int main(int argc, char *argv[])
{
    char hdd[50];
    char cmdstr[50];
    int cmd=1;

    memset(hdd,0,50);
    ms_creat_mount_path();

    if (argc>1) {
        strcpy(hdd,argv[1]);
    } else {
        printf("usage: msapp <device>\n");
        printf("       using /dev/sda\n");
        strcpy(hdd,"/dev/sda");
    }
     
	while (cmd != 99) {
        print_menu();
		fgets(cmdstr, 256, stdin);
		cmd = atoi(cmdstr);
		switch (cmd) {
		    case 2:
		        sprintf(cmdstr,"sgdisk -p %s",hdd);
		        system(cmdstr);
		        break;
            case 3:
                create_partition(hdd);
                break;
            case 4:
                check_volume(hdd);
                break;
            case 5:
                format_volume(hdd);
                break;
            case 6:
                mount_volume(hdd);
                break;
            case 7:
                unmount_volume(hdd);
                break;
            case 9:
                zap_disk(hdd);
                break;
		    default:
		        break;		        
        }
    }

	return 0;
}
