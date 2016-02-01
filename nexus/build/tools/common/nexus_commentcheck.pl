#!/usr/bin/perl
#     (c)2003-2008 Broadcom Corporation
#  
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
#   
#  Except as expressly set forth in the Authorized License,
#   
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#   
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
#  USE OR PERFORMANCE OF THE SOFTWARE.
#  
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
#  ANY LIMITED REMEDY.
# 
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# File Description:
#
# Revision History:
#
# $brcm_Log: $
# 
#############################################################################
use strict;

my $totalerrors = 0;

sub check_file
{
    local *FILE;
    my $filename = shift;
    my $line;
    my $linenum = 1;
    open FILE, $filename;
    my $comment_line = 0;
    
    my $in_summary = 0;
    my $in_description = 0;
    
    foreach $line (<FILE>) {
        if ($comment_line) {
            if ($line =~ /\*\//) {
                $comment_line = 0;
                $in_summary = 0;
                $in_description = 0;
            }
            else {
                if ($line =~ /^Summary:/) {
                    $in_summary = 1;
                    if ($comment_line != 1) {
                        print "ERROR: Summary: in invalid location at line $linenum\n";
                        $totalerrors++;
                    }
                }
                elsif ($line =~ /^Description:/) {
                    $in_description = 1;
                    if ($in_summary) {
                        print "ERROR: No blank line between Summary: and Description: at line $linenum\n";
                        $totalerrors++;
                    }
                }
                elsif ($line =~ /^See Also:/) {
                    if (!$in_description) {
                        print "ERROR: See Also: must be in Description: at line $linenum\n";
                        $totalerrors++;
                    }
                }
                elsif ($line =~ /^\s*$/) {
                    $in_summary = 0;
                }
                    
                if ($comment_line == 1 && !$in_summary) {
                    print "ERROR: Missing Summary: on line $linenum\n";
                    $totalerrors++;
                }
                    
                $comment_line++;
            }
        }
        elsif ($line =~ /^\/\*=/) {
            # module overview
        }
        elsif ($line =~ /^\/\*/) {
            if ($linenum == 1) {
                # standard header
            }
            else {
                $comment_line = 1;
            }
        }
        $linenum++;
    }
}


###############
# main
###############
print "Nexus Comment Check\n";
print "Revision: 1/23/2008\n";
my $file;
if ($#ARGV == -1) {
    print "Usage: nexus_commentcheck.pl file1.h file2.h ...\n";
    exit;
}
for $file (@ARGV) {
    print "Checking $file\n";
    print "------------------------------------------\n";
    check_file $file;
    print "------------------------------------------\n";
}

print "Total errors: $totalerrors\n";
$totalerrors;
