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

sub printerror
{
    my $linenum = shift;
    my $line = shift;
    my $message = shift;
    print "Line $linenum: $line\n";
    print "  ERROR: $message\n";
    $totalerrors++;
}

sub check_source_file
{
    local *FILE;
    my $filename = shift;
    open FILE, $filename;
    my $text = join '',<FILE>;

    # convert DOS to unix
    $text =~ s/\r\n/\n/sg;
    # convert \r to \n
    $text =~ s/\r/\n/sg;

    my @lines = split /\n/, $text;

    my $malloc = 0;
    my $linenum = 1;
    my $line;
    my $in_comment = 0;
    my $looking_for_dbg_object = 0;
    my $in_priv = 0;

    for $line (@lines) {
        if (!$in_comment) {
            if ($line =~ s/(.*)\/\*.*\*\/(.*)/$1 $2/) {
                $in_comment = 0;
            }
            elsif ($line =~ s/(.*)\/\*.*/$1/) {
                $in_comment = 1;
            }
        }
        else {
            if ($line =~ s/.*\*\/(.*)/$1/) {
                $in_comment = 0;
            }
            else {
                goto nextline;
            }
        }

        if ($line =~ /^\s*$/) {
            # if the line is all whitespace, skip it
            goto nextline;
        }

        if ($malloc) {
            if (!($line =~ /^\s*if/)) {
                printerror $linenum, $line, "No check on result of Malloc";
            }
            $malloc = 0;
        }
        if ($looking_for_dbg_object) {
            if ($line =~ /^\}/) {
                printerror $looking_for_dbg_object, $line, "Never found BDBG_OBJECT_SET or BDBG_OBJECT_INIT for Malloc. Might be needed.";
                $looking_for_dbg_object = 0;
            }
            if (($line =~ /BDBG_OBJECT_SET/) || ($line =~ /BDBG_OBJECT_INIT/)) {
                $looking_for_dbg_object = 0;
            }
        }
        if ($in_priv) {
            if ($line =~ /^\}/) {
                printerror $in_priv, $line, "Never found NEXUS_ASSERT_MODULE in _priv function.";
                $in_priv = 0;
            }
            if ($line =~ /NEXUS_ASSERT_MODULE/) {
                $in_priv = 0;
            }
        }

        if ($line =~ /Malloc/) {
            $malloc = 1;
            $looking_for_dbg_object = $linenum;
        }
        if (($line =~ /_priv\s*\(/) && !($line =~ /;/)) {
            $in_priv = $linenum;
        }

nextline:
        $linenum++;
    }
}

sub check_header_file
{
    local *FILE;
    my $filename = shift;
    open FILE, $filename;
    my $text = join '',<FILE>;

    # convert DOS to unix
    $text =~ s/\r\n/\n/sg;
    # convert \r to \n
    $text =~ s/\r/\n/sg;

    my @lines = split /\n/, $text;
    my $line;
    my $linenum = 1;
    my $in_enum = 0;
    my $in_struct = 0;
    my $in_func = 0;
    my $current_name;
    my $struct_depth = 0;
    my $in_comment = 0;

    for $line (@lines) {
        if ($line =~ /^\#/) {
            # skip preprocessor
            goto nextline;
        }

        if (!$in_comment) {
            if ($line =~ s/(.*)\/\*.*\*\/(.*)/$1 $2/) {
                $in_comment = 0;
            }
            elsif ($line =~ s/(.*)\/\*.*/$1/) {
                $in_comment = 1;
            }
        }
        else {
            if ($line =~ s/.*\*\/(.*)/$1/) {
                $in_comment = 0;
            }
            else {
                goto nextline;
            }
        }
        #print "$line\n";

        if ($line =~ /^\s*$/) {
            # if the line is all whitespace, skip it
            goto nextline;
        }

        if ($line =~ /\t/) {
            printerror $linenum, $line, "hard tabs not allowed";
            goto nextline;
        }
        if (!$in_struct) {
            #TODO: deal with nested structs later
            if (!($line =~ /^    \S/ || $line =~ /^\S/)) {
                printerror $linenum, $line, "indentation must be 4 spaces";
                goto nextline;
            }
        }
        if ($in_enum || $in_struct) {
            if ($line =~ /^\}/) {
                $in_enum = $in_struct = 0;
                if (!($line =~ /^\} $current_name\;/)) {
                    printerror $linenum, $line, "name of typedef must match struct/enum name";
                    goto nextline;
                }
            }
            elsif ($line =~ /^\{/) {
                # ok
            }
            elsif ($in_enum) {
                if (!($line =~ /    ${current_name}_e[A-Z0-9]\w*/)) {
                    printerror $linenum, $line, "invalid enum: ${current_name}_eName";
                    goto nextline;
                }
            }
        }
        else {
            if ($line =~ /enum/) {
                ($current_name) = $line =~ /^typedef enum (NEXUS_\w+)\s*$/;
                if (!$current_name) {
                    printerror $linenum, $line, "invalid enum typedef";
                    goto nextline;
                }
                else {
                    $in_enum = 1;
                }
            }
            elsif ($line =~ /struct/) {
                ($current_name) = $line =~ /^typedef struct (NEXUS_\w+)\s*$/;
                if (!$current_name) {
                    # could be a handle
                    ($current_name) = $line =~ /^typedef struct (NEXUS_\w+) \*\w+\;$/;
                    if (!$current_name) {
                        printerror $linenum, $line, "invalid struct typedef";
                        goto nextline;
                    }
                }
                else {
                    $in_struct = 1;
                }
            }
        }
        if ($line =~ /\(/) {
            my $returnval;
            # function name
            ($returnval,$current_name) = $line =~ /^(\w+) (NEXUS_\w+)\(\s*$/;
            if (!$returnval || !$current_name) {
                ($returnval,$current_name) = $line =~ /^(\w+) (NEXUS_\w+)\(void\)\;$/;
                if (!$returnval || !$current_name) {
                    printerror $linenum, $line, "invalid function prototype";
                    goto nextline;
                }

                # returnval must be NEXUS_Error, NEXUS_XxxxHandle or void
                if (!($returnval =~ /NEXUS_Error/ || $returnval =~ /void/ || $returnval =~ /NEXUS_\w+Handle/)) {
                    printerror $linenum, $line, "invalid function return type";
                    goto nextline;
                }
            }
            $in_func = 1;
        }
        if ($in_func) {
            if ($line =~ /\)/) {
                $in_func = 0;
                if (!($line =~ /    \);/) && !($line =~ /\(void\);/)) {
                    printerror $linenum, $line, "invalid close of function prototype";
                    goto nextline;
                }
            }
            if ($line =~ /\*/) {
                # this doesn't work yet because we stripped comments
                if (0) {
                    if ($line =~ /const/) {
                        if (($line =~ /\[out\]/)) {
                            printerror $linenum, $line, "in param should be const, out param should have [out] in comment";
                            goto nextline;
                        }
                    }
                    else {
                        if (!($line =~ /\[out\]/)) {
                            printerror $linenum, $line, "in param should be const, out param should have [out] in comment";
                            goto nextline;
                        }
                    }
                }
                # search for "Type *pName"
                if (!($line =~ /\w+ \*?\*p[A-Z]\w*/)) {
                    printerror $linenum, $line, "reference param should be called pName";
                    goto nextline;
                }
            }
        }

# Ideas:
# camel case in structure members
# indenting into sub-structures
# comments before prototype
# cplusplus defines
# const pointers or [out]

nextline:
        $linenum++;
    }
#    print $text;
}


###############
# main
###############
print "Nexus Code Check\n";
print "Revision: 10/4/2007\n";
my $file;
if ($#ARGV == -1) {
    print "Usage: nexus_codecheck.pl file1.h file2.h ...\n";
    exit;
}
for $file (@ARGV) {
    print "Checking $file\n";
    print "------------------------------------------\n";
    if ($file =~ /\.h$/) {
        check_header_file $file;
    }
    elsif ($file =~ /\.c$/) {
        check_source_file $file;
    }
    print "------------------------------------------\n";
}

print "Total errors: $totalerrors\n";
$totalerrors;
