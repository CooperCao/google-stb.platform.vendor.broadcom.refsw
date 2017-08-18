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

#define HUMAN 0

#define TEMP_BUFFER_SIZE 128

void *BKNI_Memset(void *mem, int ch, size_t n)
{
    return memset(mem,ch,n);
}

void *BKNI_Malloc(size_t size)
{
    return malloc(size);
}

void BKNI_Free(void *mem)
{
    free(mem);
}

/* This is used to indent and pretty up the output for human readability */
static int Depth = 0;

void lineBreak( void )
{
    int i;
    printf("\n");

    for(i=0; i<Depth; i++)
    {
        printf("\t");
    }
}

void startTable( char * class, char * commaSeparatedNames )
{
#if !HUMAN
    char * current = commaSeparatedNames;
    char * next = NULL;
    int length = 0;

    if( Depth > 0)
    {
        printf("<td>");
    }
#endif

    Depth++;

#if !HUMAN
    lineBreak();
    printf("<table class=\"%s\">", class);

    lineBreak();
    printf("<tr>");

    /* Create table header entries from our comma separated names */
    while(current != NULL)
    {
        next = strchr(current, ',');
        if( next == NULL )
        {
            length = strlen(current);
        }
        else
        {
            length = next - current;
        }
        printf("<th>");
        printf("%.*s", length, current);
        printf("</th>");
        if( next != NULL )
        {
            current = next + 1;
        }
        else
        {
            current = NULL;
        }
    }
    printf("</tr>");
    lineBreak();
#endif
}

void endTable( void )
{
    Depth--;
#if !HUMAN
    printf("</table>");
    lineBreak();
    if( Depth > 0)
    {
        printf("</td>");
    }
#endif
}

void startRow( unsigned highlight )
{
#if !HUMAN
    if (highlight) {
        printf("<tr class=\"d0\">");
    } else {
        printf("<tr>");
    }
#endif
}

void endRow( void )
{
#if !HUMAN
    printf("</tr>");
    lineBreak();
#endif
}

void addElement( char * name, char * value)
{
#if HUMAN
    lineBreak();
    printf("%s - %s", name, value);
#else
    printf("<td>%s</td>", value);
#endif
}

#if HUMAN
#define _H1 "\n"
#define H1_ "\n"
#define _H2 "\n"
#define H2_ "\n"
#define BR  "\n"
#else
#define _H1 "<h1>"
#define H1_ "</h1>"
#define _H2 "<h2><strong>"
#define H2_ "</strong></h2>"
#define BR  "<br>"
#endif

void startHeader( void )
{
#if !HUMAN
    printf("<!DOCTYPE html>\n<html>\n<head>\n<style>\n");
    printf("table, th, td {\n\tborder: 1px solid black;\n\tborder-collapse: collapse;\n\ttext-align: center;\n}\n");
    printf("table.windows, table.display_capabilities {\n\twidth: 100%%;\n}\n");
    printf("th, td {\n\tpadding: 5px;\n}\n");
    printf("tr.d0 td { background-color: #EEEEEE;}\n");
    printf("</style>\n</head>\n<body>\n");
#endif
    printf("%s %d %s Box mode comparison table for URSR %d.%d %s",_H1,BCHP_CHIP,BCHP_STR_VER,NEXUS_PLATFORM_VERSION_MAJOR,NEXUS_PLATFORM_VERSION_MINOR,H1_);
    printf("%sThis table is provided for use as a high level comparison only.%sBefore selecting a box mode you should review the full implementation",_H2,BR);
    printf(" details provided in the pdfs in nexus/platforms/9%d/docs as all the use cases cannot be accurately described in a simple table.%s",BCHP_CHIP,H2_);
}

void endHeader( void )
{
#if !HUMAN
    printf("</body>\n</html>");
#else
    printf("\n");
#endif
}
