/****************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ****************************************************************************/
#include "DBG_core_debug_server_gdb_utils.h"
#include "libdspcontrol/DSPLOG.h"

static const char hexchars[16] = "0123456789abcdef";
/****************************************************************************
 *             UTILITY & STRING MANIPULATION FUNCTIONS
 ****************************************************************************/
/*
 * handle a hex character and return an integer
 * */
int
handle_hex_char (char ch)
{
    if ((ch >= 'a') && (ch <= 'f'))
        return (ch - 'a' + 10);
    if ((ch >= '0') && (ch <= '9'))
        return (ch - '0');
    if ((ch >= 'A') && (ch <= 'F'))
        return (ch - 'A' + 10);
    return (-1);
}

/*Converts pairs of hex digits to ASCII character strings*/
int
hexToAscii(char *p_ch_dest, char *p_ch_src, int count)
{
    int i;

    for(i = 0; i< count ; i++)
    {
        if( p_ch_src[i*2] == 0 || p_ch_src[i*2 + 1] == 0)
        {
            return i;
        }
        p_ch_dest[i] = ((handle_hex_char(p_ch_src[i*2]) & 0xf) << 4) |
                        (handle_hex_char(p_ch_src[i*2 + 1]) & 0xf);

    }
    return i;
}
/*
 * Converts an ASCII character string to pairs of hex digits
 * Both source and destination strings are NULL terminated
 * */
void
asciiToHex(char *p_ch_dest, char *p_ch_src)
{
    char ch;
    while (*p_ch_src != '\0')
    {
        ch = *p_ch_src++;
        *p_ch_dest++ = hexchars[ch>>4 & 0xf];
        *p_ch_dest++ = hexchars[ch & 0xf];
    }
    *p_ch_dest = '\0';
}

/*Converts an array of hex binary into a hex character array*/
void
memToHex(uint8_t *p_u8_mem, char *p_ch_buf, int count)
{
    uint8_t u8_val;
    int i;
    for(i=0;i<count;i++)
    {
        u8_val = *p_u8_mem++;
        *p_ch_buf++ = hexchars[(u8_val >> 4) & 0xf];
        *p_ch_buf++ = hexchars[u8_val & 0xf];
    }
    *p_ch_buf = '\0';
}

/*convert a hex array pointed by buf to binary and place it in mem
 * */
void
hexToMem (char *p_ch_buf, uint8_t *p_u8_mem, int count)
{
    unsigned char ch;
    int i;
    for(i=0; i < count; i++)
    {
        ch = handle_hex_char(*p_ch_buf++) << 4;
        ch = ch | handle_hex_char (*p_ch_buf++);
        *p_u8_mem++ = ch;
    }
}

/* Convert a string of hex chars to an integer
 * returns the number of chars processed
 * */
int
hexToInt(char **p_ch_ptr, uint32_t *p_i_value)
{
    int num_chars = 0;
    int hex_value = 0;

    uint32_t u32_val = 0;

    while (**p_ch_ptr)
    {
        hex_value = handle_hex_char(**p_ch_ptr);
        if(hex_value >= 0)
        {
            u32_val = (u32_val << 4) | hex_value;
            num_chars++;
        }
        else
            break;

        (*p_ch_ptr)++;
    }

    *p_i_value = u32_val;
    return num_chars;
}
/* Convert a string of hex chars to an integer
 * returns the number of chars processed
 * */
int
hexToInt64(char **p_ch_ptr, int64_t *p_i_value)
{
    int num_chars = 0;
    int hex_value = 0;

    *p_i_value = 0;
    while (**p_ch_ptr)
    {
        hex_value = handle_hex_char(**p_ch_ptr);
        if(hex_value >= 0)
        {
            *p_i_value = (*p_i_value << 4) | hex_value;
            num_chars++;
        }
        else
            break;

        (*p_ch_ptr)++;
    }
    return num_chars;
}

/*Convert an integer to a hexadecimal string*/
void
intToHex(uint32_t u32_hex, char *p_ch_buf)
{
    uint8_t *p_u8_val = (uint8_t *)&u32_hex;
    int i;

    for(i=sizeof(uint32_t)-1; i>=0;i--)
    {
        uint8_t u8_val = p_u8_val[i];
       *p_ch_buf++ = hexchars[(u8_val >> 4) & 0xf];
       *p_ch_buf++ = hexchars[u8_val & 0xf];
    }
    *p_ch_buf = '\0';
}

uint32_t
unescape_binary_data(char *p_ch_buf, uint32_t u32_orig_len)
{
    uint32_t u32_from_off  = 0;
    uint32_t u32_to_off = 0;

    while(u32_from_off < u32_orig_len)
    {
        if(p_ch_buf[u32_from_off] == '}')
        {
            u32_from_off++;
            p_ch_buf[u32_to_off] = p_ch_buf[u32_from_off] ^ 0x20;
            DSPLOG_ERROR("escaping from_off %x to_off %x %x->%x",
                    u32_from_off, u32_to_off,(uint8_t) p_ch_buf[u32_from_off],
                    (uint8_t)p_ch_buf[u32_to_off]);
        }
        else
            p_ch_buf[u32_to_off] = p_ch_buf[u32_from_off];

        u32_from_off++;
        u32_to_off++;
    }
    return u32_to_off;
}


void
DBG_core_add_chksum(char *p_ch_buffer, size_t s_pkt_size)
{
    uint8_t u8_chksum = 0;
    uint32_t u32_i;
    for(u32_i = 0; u32_i< s_pkt_size;u32_i++)
    {
        u8_chksum +=*p_ch_buffer++;
    }
    strcat(p_ch_buffer, "#");
    sprintf(p_ch_buffer, "%s%02x",p_ch_buffer, u8_chksum);
}

/* Return a malloc allocated string with special characters from TEXT
   replaced by entity references.  */
char *
DBG_core_xml_escape_text (const char *text)
{
    char *result;
    int i, special;

    /* Compute the length of the result.  */
    for (i = 0, special = 0; text[i] != '\0'; i++)
        switch (text[i])
        {
            case '\'':
            case '\"':
                special += 5;
                break;
            case '&':
                special += 4;
                break;
            case '<':
            case '>':
                special += 3;
                break;
            default:
                break;
        }

    /* Expand the result.  */
    result = (char *) malloc (i + special + 1);
    if(result == NULL)
    {
        DSPLOG_ERROR("xml_escape_text(): cannot allocate buffer");
        return NULL;
    }
    for (i = 0, special = 0; text[i] != '\0'; i++)
        switch (text[i])
        {
            case '\'':
                strcpy (result + i + special, "&apos;");
                special += 5;
                break;
            case '\"':
                strcpy (result + i + special, "&quot;");
                special += 5;
                break;
            case '&':
                strcpy (result + i + special, "&amp;");
                special += 4;
                break;
            case '<':
                strcpy (result + i + special, "&lt;");
                special += 3;
                break;
            case '>':
                strcpy (result + i + special, "&gt;");
                special += 3;
                break;
            default:
                result[i + special] = text[i];
                break;
        }
    result[i + special] = '\0';

    return result;
}
