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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>

#define ICONS_PER_ROW 6

int ScanDirectory(
    void
    )
{
    DIR           *d;
    struct dirent *dir;
    struct stat statbuf;

    /* search the current directory for any files with html extension */
    if (( d = opendir( "." )) != NULL)
    {
        int   file_count = 0;
        char *posDot     = NULL;
        char  iconFilename[64];
        char  strFilenameNoExtension[64];
        char  iconHeight = 120;

        memset ( &statbuf, 0, sizeof(statbuf) );

        printf( "~HTMLFILES~" );
        printf( "<style>\n");
        printf( "html, body, div, img {\n");
        printf( "padding:0;\n");
        printf( "margin:0;\n");
        printf( "border:0;\n");
        printf( "}\n");
        printf( ".container {\n");
        printf( "width:100%;\n");
        printf( "}\n");
        printf( ".column {\n");
        printf( "  width:120px;\n");
        printf( "  display:inline-block;\n");
        printf( "  vertical-align:top;\n");
        printf( "}\n");
        printf( ".column img {\n");
        printf( "  width:100%;\n");
        printf( "  padding:0px;\n");
        printf( "  vertical-align:top;\n");
        printf( "}\n");
        printf( "</style>\n");
        printf( "<div class=\"container\" >\n");

        /* repeat for all entries in the directory */
        while (( dir = readdir( d )) != NULL)
        {
            if (( dir->d_type == DT_REG ) || ( dir->d_type == DT_LNK ))
            {
                if (strstr( dir->d_name, ".html" ))
                {
                    if (strcmp ( dir->d_name, "index.html" ) == 0 )
                    {
                        /* we do not want to show the default html page */
                    }
                    else
                    {
                        iconHeight = 120; /* the broadcom pulse icon has much smaller height than others */

                        strncpy ( iconFilename, dir->d_name, sizeof(iconFilename) - strlen(iconFilename) - 1 );
                        posDot = strchr ( iconFilename, '.' );
                        /* if the dot before the extension was found ... this should always be the case */
                        if (posDot)
                        {
                            *posDot = 0;
                        }
                        strncpy ( strFilenameNoExtension, iconFilename, sizeof(strFilenameNoExtension) - 1 );
                        strncat ( iconFilename, ".png", sizeof(iconFilename) - strlen(iconFilename) - 1 );

                        /* determine if the associated PNG file exists */
                        if (lstat( iconFilename, &statbuf ) == -1)
                        {
                            /* could not find image file ... use the default */
                            strncpy ( iconFilename, "brcm-logo-white.jpg", sizeof(iconFilename) - strlen(iconFilename) - 1 );
                            iconHeight = 60; /* default icon is shorter than others */
                        }

                        printf( "<div class=\"column\"> <table border=0 style=\"border-collapse:collapse;\" ><tr><th width=120 valign=bottom > <a href=\"%s\" style=\"text-decoration:none\">"
                                "<img src=\"%s\" height=\"%d\" width=\"120\"></th></tr><tr><th align=center>%s</a></th></tr></table></div>\n",
                                dir->d_name, iconFilename, iconHeight, strFilenameNoExtension );

                        file_count++;
                    }
                }
            }
        }
        closedir( d );
        /* if we output an incomplete row above, end the row now */
        if (file_count && ((file_count%ICONS_PER_ROW)!=0) )
        {
            printf( "</tr>" );
        }
        printf( "</table>~" );
    }

    return( 0 );
} /* ScanDirectory */

int main(
    void
    )
{
    printf( "Content-type: text/html%c%c", 10, 10 );

    ScanDirectory();

    return( 0 );
}
