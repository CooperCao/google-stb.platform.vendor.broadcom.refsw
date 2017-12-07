/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/* This is code which demonstrates the interface to the Zigbee kernel module
   driver. This code will need to be written by Benjamin's team. */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <memory.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include "zigbee_ioctl.h"
#include "zigbee_driver.h"



/* Firmware file has a table of metadata describing which parameters it supports.
   This table is located at the end of the file.

   The file structure is:

   <Firmware image>
   <Parameter metadata table -- Param_entry[param_header.table_size] >
   <Parameter metadata header -- Param_header>

   The Parameter metadata will be offset backwards from the end of the file so the
   offsets will be independent of the firmware image size.
*/

typedef struct
{
    uint32_t iccm_size;
    uint32_t table_size;       // The number of Param_entry's in the table
    uint32_t magic_number;     // 0x12345678 if the firmware file supports parameters
} Param_header;

#define MAX_PARAM_NAME_LENGTH 52

typedef struct
{
    char param_name[MAX_PARAM_NAME_LENGTH];
    uint32_t param_addr;
    uint32_t param_item_size;
    uint32_t param_num_items; // used for arrays
} Param_entry;


#define INIT_HEADER_MAGIC 0x12345678
#define INIT_CONFIG_HDR_OFFSET -sizeof(Param_header)

/* Given a param_name, find the corresponding param_entry in the table */
static Param_entry *find_parameter_entry(char *param_name, Param_entry *param_entry, int param_table_size)
{
    int i;
    for (i=0; i<param_table_size; i++)
    {
        if (strncasecmp(param_entry->param_name, param_name, sizeof(param_entry->param_name)-1) == 0)
            return param_entry;
        param_entry++;
    }
    return NULL;
}


/* Read one number from the file.  Return 0 if data is valid and more data is available, 1 if data is valid
   and no more data is available on the line, or 2 if the data isn't valid */
static int get_data(FILE *fp, uint32_t *data)
{
    int x;
    char val[32] = {0};
    int i = 1;

    while (((x = fgetc(fp)) !=  EOF) && ((x == ' ') || (x == '\t')));

    if ((x == EOF) || (x == '\n'))
        return 2;

    val[0] = x;

    while (((x = fgetc(fp)) !=  EOF) && (!isspace(x)) && (x != ','))
    {
        if (i < sizeof(val) -1)
           val[i++] = x;
    }
    val[i] = '\0';

    *data = strtol(val, NULL,0);

    if ((x == EOF) || (x == '\n'))
        return 1;

    return 0;
}

/* Read file until we reach the start of a new line */
static int eat_line(FILE *fp)
{
    char x;
    // eat up rest of line
    while((x=fgetc(fp)) != EOF)
    {
        if (x == '\n') return 1;
        if (x == '\r') return 0;
    }
    return 0;
}

/* Patch a firmware file in memory with parameters read from parameters_filename */
static void patch_init_params(char *parameters_filename, unsigned char *fw_img, int fw_len, Param_entry *param_table, int param_table_size, unsigned int iccm_size)
{
    Param_entry *param_entry;
    char parameter[MAX_PARAM_NAME_LENGTH];
    char *c;
    int line=1;
    int x=0;

    FILE *fp = fopen(parameters_filename,"r");

    if (fp == NULL)
    {
//       syslog(LOG_INFO, "can't open initial parameters file: %s", parameters_filename);
       return;
    }

    /* Loop for all lines in the file */
    while (x != EOF)
    {
        /* Maximum one parameter per line.  Initialize */
        c = &parameter[0];
        memset(c, 0, sizeof(parameter));

        // parse until '=' or end-of-line or end-of-file
        while ((x=fgetc(fp)) != EOF)
        {
            *c = x;
            if ((*c == '=') || (*c == '\n') || (*c == '\r'))
               break;

            // truncate parameter names to fit in our variable.  Ignore spaces
            if ((c < &parameter[MAX_PARAM_NAME_LENGTH-1]) && (!isspace(*c)))
               c++;
        }

        if (*c == '=') // found a parameter to be set
        {
            /* Terminate parameter string */
            *c = '\0';

            /* Find the parameter in the param table */
            param_entry = find_parameter_entry(parameter, param_table, param_table_size);

            if (param_entry)
            {
                uint32_t data;
                int item_count = 0;
                int rc=0;

                // Get data from conf file and patch fw

                while((rc = get_data(fp, &data)) < 2)
                {
                    if (item_count >= param_entry->param_num_items)
                    {
//                        syslog(LOG_INFO, "Too many entries on line %d.  Max %d allowed for parameter %s", line, param_entry->param_num_items, parameter);
                        // eat up rest of line
                        if (rc == 0)
                            line += eat_line(fp);
                        break;
                    }
                    else
                    {
                        /* don't overflow the buffer */
                        if (param_entry->param_addr + param_entry->param_item_size * (item_count+1) > fw_len)
                        {
//                            syslog(LOG_INFO, "Param addr exceeds fw_image length for parameter %s on line %d", parameter, line);
                            if (rc == 0)
                                line += eat_line(fp);
                            break;
                        }

                        /* Patch the firmware with 1, 2 or 4 bytes here */
                        if (param_entry->param_item_size == 1)
                           *(uint8_t *)(fw_img + param_entry->param_addr + item_count*1) = (uint8_t) data & 0xFF;
                        else if (param_entry->param_item_size == 2)
                           *(uint16_t *)(fw_img + param_entry->param_addr + item_count*2) = (uint16_t) data & 0xFFFF;
                        else if (param_entry->param_item_size == 4)
                           *(uint32_t *)(fw_img + param_entry->param_addr + item_count*4) = (uint32_t) data & 0xFFFFFFFF;

                        item_count++;
                    }

                    if (rc == 1)
                    {
                        line++;
                        break;
                    }

                }
                if (rc == 2) line++;
            }
            else
            {
//                syslog(LOG_INFO, "unsupported parameter on line %d: %s. Ignoring", line, parameter);

                // eat up rest of line
                line += eat_line(fp);
            }
        }
        else // end-of-line or end-of-file
        {
            if (*c == '\n')
                line++;
        }
    }

    fclose(fp);
}

#define PARAMETERS_FILENAME "/etc/broadot.conf"

/*******************************************************************************
   Download and start the Zigbee core.
*******************************************************************************/
int Zigbee_Ioctl_Start(int fd, unsigned char *pImage, unsigned int size_in_bytes)
{
    struct fw f;
    Param_header *param_hdr;

    /* Check to see if firmware supports initialization parameters */
    if (((size_in_bytes & 3) == 0) && (size_in_bytes >= sizeof(Param_header)))
    {
        param_hdr = (Param_header *)(pImage + size_in_bytes + INIT_CONFIG_HDR_OFFSET);

        if ((param_hdr->magic_number == INIT_HEADER_MAGIC) &&
            (param_hdr->table_size != 0) &&
            (param_hdr->table_size*sizeof(Param_entry) + sizeof(Param_header) <= size_in_bytes))
        {
            size_in_bytes = size_in_bytes - sizeof(Param_header) - param_hdr->table_size*sizeof(Param_entry);
            patch_init_params(PARAMETERS_FILENAME, pImage, size_in_bytes, (Param_entry *)(pImage + size_in_bytes), param_hdr->table_size, param_hdr->iccm_size);
        }
    }

    f.size_in_bytes = size_in_bytes;
    f.pImage = pImage;

    if (ioctl(fd, ZIGBEE_IOCTL_START, &f) == 0) {
        return ZIGBEE_START_OK;
    }
    return ZIGBEE_START_NOT_OK;
}

/*******************************************************************************
   Stop the Zigbee core.
*******************************************************************************/
int Zigbee_Ioctl_Stop(int fd)
{
    unsigned int data;

    if (ioctl(fd, ZIGBEE_IOCTL_STOP, &data) == 0) {
        return ZIGBEE_STOP_OK;
    }
    return ZIGBEE_STOP_NOT_OK;
}

/*******************************************************************************
   It is assumed that the data pointed to by pData has a 32 bit header at the
   top, which contains the size in bits 4:0, as specified in the BroadBee
   Application Software Reference Manual.

   This function may block.
*******************************************************************************/
int Zigbee_Ioctl_WriteToMbox(int fd, char *pData)
{
    if (ioctl(fd, ZIGBEE_IOCTL_WRITE_TO_MBOX, pData) == 0) {
        return ZIGBEE_WRITE_TO_MBOX_QUEUED;
    }
    return ZIGBEE_WRITE_TO_MBOX_ERROR;
}

/*******************************************************************************
   This function may block.
*******************************************************************************/
int Zigbee_Ioctl_ReadFromMbox(int fd, char *pData)
{

    if (ioctl(fd, ZIGBEE_IOCTL_READ_FROM_MBOX, pData) == 0) {
        return ZIGBEE_READ_FROM_MBOX_OK;
    }
    return ZIGBEE_READ_FROM_MBOX_ERROR;
}

/*******************************************************************************
   This function may block.
*******************************************************************************/
int Zigbee_Ioctl_WaitForWDTInterrupt(int fd, char *pData)
{
    if (ioctl(fd, ZIGBEE_IOCTL_WAIT_FOR_WDT_INTERRUPT, pData) == 0) {
        return ZIGBEE_WDT_OK;
    }
    return ZIGBEE_WDT_ERROR;
}

int Zigbee_Ioctl_GetRf4ceMacAddr(int fd, char *pData)
{
    if (ioctl(fd, ZIGBEE_IOCTL_GET_RF4CE_MAC_ADDR, pData) == 0) {
        return ZIGBEE_WDT_OK;
    }
    return ZIGBEE_WDT_ERROR;
}

int Zigbee_Ioctl_GetZbproMacAddr(int fd, char *pData)
{
    if (ioctl(fd, ZIGBEE_IOCTL_GET_ZBPRO_MAC_ADDR, pData) == 0) {
        return ZIGBEE_WDT_OK;
    }
    return ZIGBEE_WDT_ERROR;
}

/* eof zigbee_ioctl.c */
