#!/usr/bin/perl
#  Broadcom Proprietary and Confidential. (c)2003-2016 Broadcom. All rights reserved.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
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
package bapi_parse_c;
use strict;

sub get_file_text
{
    my $filename = shift;
    open (my $file, $filename) or die "Can't open $filename";
    my $text = join '',<$file>;
    close $file;

    # pack preprocessor line continutation
    $text =~ s/\\\s+(\w)/$1/sg;
    # remove preprocessor stuff that can cause parsing problems
    $text =~ s/#\w+.*?\n//g;

    # Grab all attr{} hints and move them out of comment
    # Blow away the comment because it isn't going to stay anyway.
    $text =~ s/
        \/\*            # start of a comment
        [^\/]*           # inside comment
        (attr\{[^}]+})   # keep attr{..}
        .*?             # rest of stuff
        \*\/            # end of the comment
        /$1/sgx;

    # Remove all comments
    # No nested comments allowed
    $text =~ s/\/\*.*?\*\///sg;

    return $text;
}

sub get_func_prototypes
{
    my $filename = shift;
    my $text = get_file_text $filename;

    # If we have any ", attr", reverse the order. Much easier to parse later.
    $text =~ s/(,)\s*(attr\{[^}]+})/ $2$1/sg;

    # Parse functions
    my @funcs = $text =~ /(
        \w[\w\s]+   # return type
        [\s\*]      # whitespace between return type & function name, or * for pointer
        \w+         # function name
        \s*         # possible whitespace between function name and param list. this violates coding convention, but should not break the function parsing.
        \([^\)]*?\)) # parameters. excluding ) avoids detecting "typedef int (*function)(param);" as return type of "typedef" and function name of "int".
        \s*;        # must terminate with ;
        /xsg;

    @funcs;
}

sub checksum
{
    my $sum = 0;
    my $str;
    for $str (@_) {
        $sum += unpack("%16C*", $str);
    }
    return $sum;
}


# parse_attr
#
# parse attributes in the comma separated name=value pairs
# store result in the hash
sub parse_attr
{
    my $attr = shift;
    my %attrs;


#   print "ATTR -> $attr\n";
   for(split(';',$attr)) {
        if (/(\w+)\s*=\s*(\S+)/) {
             $attrs{$1}=$2;
#            print "attr '$1' -> '$2'\n";
        }
   }
   return \%attrs;
}

sub print_attr
{
    my $attr = shift;
    my ($name, $value);
    if(defined $attr) {
        print "attr{";
        while (($name, $value) = each %$attr) {
            print " $name = $value  ";
        }
        print "}\n";
    }
}

sub version_attr
{
    my $attr = shift;
    my ($name, $value);
    my $sum = 0;
    if(defined $attr) {
        while (($name, $value) = each %$attr) {
            $sum += checksum $name, $value;
        }
    }
    $sum;
}

sub print_struct
{
    my $struct = shift;
    my $name;
    my $fields;
    while (($name, $fields) = each %$struct) {
        my $field;
        print "structure $name: \n";
        for $field (@$fields) {
            print "  field: $field->{TYPE} $field->{NAME}\n";
            print_attr $field->{ATTR};
        }
    }
}

sub copy_structs
{
    my %dst;
    my $structs = shift;
    my $name; my $members;
    while (($name, $members) = each %$structs) {
        $dst{$name} = [@$members];
    }
    \%dst;
}

sub expand_structs
{
    my ($structs,$preload_structs,$opts) = @_;
    my $name; my $members;
    my %all_structs;
    my $combine=1;

    if(defined $opts) {
        $combine = $opts->{COMBINE} if(exists $opts->{COMBINE});
    }

    while (($name, $members) = each %$preload_structs) {
        $all_structs{$name} = $members;
        if($combine) {
            $structs->{$name} = $members;
        }
    }
    while (($name, $members) = each %$structs) {
        $all_structs{$name} = $members;
    }
    while(1) {
        my $expanded = 0;
        while (($name, $members) = each %$structs ) {
            foreach my $i (0..$#$members) {
                my $type = $members->[$i]{TYPE};
                if(exists $all_structs{$type}) {
                    my @fields;
                    foreach my $field (@{$all_structs{$type}}) {
                        my %paramhash = %$field;
                        $paramhash{NAME} = "$members->[$i]{NAME}.$field->{NAME}";
                        my @kind = @{$members->[$i]{KIND}};
                        push @kind, @{$paramhash{KIND}};
                        $paramhash{KIND} = \@kind;
                        push @fields, \%paramhash;
                    }
                    splice @$members, $i,1,@fields;
                    $expanded=1;
                    last;
                }
            }
        }
        last unless $expanded; # expand all structures
    }

}

sub parse_struct
{
    my $filename = shift;
    my $preload_struct = shift;
    my $text = get_file_text $filename;
    my %struct;

    while ($text =~  /
        typedef\s+struct\s+(\w+)\s*{  # typedef
        (.+?)}                 # body
        \s*\1                   # termination
        \s*;                      # must terminate with ;
        /xgs) {
        my @level= ( [] );; # fields on the current level
        my @level_kind =  ( 'struct' ); # type (struct or union) on the current level
        my $struct_name= $1;
        my $body = $2;
        my $kind;
        #print "structure $1\n";
        $struct{$struct_name} = $level[0];
        while($body =~ /(struct\s*{)|(union\s*{)|(}[^;]+;)|((\w[\w\s]*[\s\*]+)(\w+[^;]*))\s*;\s*(attr\{([^}]+)})?/sg) {
            
            # interpretation of regex:
            # $1 = "struct {"  ==> the beginning of a substruct
            # $2 = "union {"   ==> the beginning of a subunion
            # $3 = "} name;"   ==> close of a sub-struct/union
            # $4 = "MemberType member"  ==> struct member, type and name
            # $5 = "MemberType"         ==> struct member, type
            # $6 = "member"             ==> struct member, name
            # $7 = "attr{name=value}"   ==> member's whole attr string
            # $7 = "name=value"         ==> member's inner attr string
                
            #print "'$1' '$2' '$3' '$4' '$5' '$6' '$7' '$8'\n";
            if(defined $5) {
                my $type = $5;
                my $names = $6;
                my $attr = $8;
                $type =~ s/\s{2,}/ /sg;
                $type =~ s/\s$//s;
                $names =~ s/\s//g;
                if(defined $attr) {
                    $attr = parse_attr $attr;
                }
                foreach(  (split /[,;]/, $names) ) {
                    my @field_kind = @level_kind;
                    my $field = {
                        KIND => \@field_kind,
                        NAME => $_,
                        TYPE => $type,
                    };
                    if(defined $attr) {
                        $field->{ATTR} = $attr;
                    }
                    push @{$level[$#level]}, $field;
                }
            } elsif (defined $1 || defined $2) {
                $kind = 'struct';
                $kind = 'union' if (defined $2);
                push @level_kind, $kind;
                push @level, [];
            } elsif (defined $3) {
                my $names = $3;
                $names =~ s/\s+//g;
                pop @level_kind;
                my $fields = pop @level;
                while($names =~ /(\w+[^,;]*)/sg) {
                    # append structure name to each field
                    my $name = $1;
                    for my $field (@$fields) {
                        my %paramhash= %$field;
                        $paramhash{NAME} = "$name.$field->{NAME}";
                        push @{$level[$#level]},\%paramhash;
                    }
                }
            }
        }
    }

#    print_struct \%struct;

    return \%struct;
}

sub version_struct
{
    my $struct = shift;
    my $sum = 0;
    my $name;
    my $fields;
    my $field;

    while (($name, $fields) = each %$struct) {
        $sum += unpack("%32C*", $name);
        for $field (@$fields) {
            $sum += checksum $field->{TYPE} , $field->{NAME};
            $sum += version_attr $field->{ATTR};
        }
    }
    return $sum;
}


sub print_enum
{
    my $enum = shift;
    my $name;
    my $fields;
    while (($name, $fields) = each %$enum) {
        my $field;
        print "enum $name: \n";
        for $field (@$fields) {
            print "  field: $field->{TYPE} $field->{NAME}\n";
            print_attr $field->{ATTR};
        }
    }
}

sub parse_enum
{
    my $filename = shift;
    my $preload_enum = shift;
    my $text = get_file_text $filename;
    my %enum;

    if (defined $preload_enum) {
        %enum = %$preload_enum;
    }
    
    while ($text =~  /
        typedef\s+enum\s+(\w+)\s*{  # typedef
        (.+?)}                      # body
        \s*\1                       # termination
        \s*;                        # must terminate with ;
        /xgs) {
        my @params; # this is a list of a param hash
        my @values;
        my $current_value = "0";
        my %interpreted_values; # hash containing current enum values seen (to bypass duplication)
        my $name= $1;
        my $body = $2;
        #print "enum $1\n";
        #print ">>enum name $name\n";
        #print ">>enum body $body\n";
        $enum{$name} = \@params;
        while($body =~ /((\w+),)|((\w+)\s*=\s*([\-\w]+))|(\w+)?/sg) {
            
            # interpretation of regex:
            # $1 = "EnumName,"  ==> EnumName captured, with trailing ,
            # $2 = "EnumName"   ==> (from EnumName,)
            # $3 = "EnumName = value" ==> aliased enum
            # $4 = "EnumName"   ==> EnumName (when aliased to another value)
            # $5 = "value"      ==> value this enum is aliased to
            # $6 = "EnumName"   ==> from final enum value, no trailing ,

            #if(defined $1 or defined $2 or defined $3 or defined $4 or defined $5 or defined $6) { print "from regexp: 1'$1' 2'$2' 3'$3' 4'$4' 5'$5' 6'$6'\n"; }
            #print "  cur value: '$current_value'\n";
            if (defined $2 || defined $4 || defined $6) {
                my $name;
                if (defined $2) {
                    $name = $2;
                    $interpreted_values{$current_value} = $name;
                }
                elsif (defined $4) {
                    # check if aliased
                    if (defined $5) {
                        my $saved_name = $4;
                        my $saved_value = $5;
                        if ($5 =~ /^([0-9,.E]+)$/) {
                            if (defined $1) {
                                #print "  found numeral: '$1'!\n";
                                #print "  current hash: ".$interpreted_values{$1}."!\n";
                                $current_value = $1;
                                if (exists $interpreted_values{$1}) {
                                    #print "  found value '$1' already!\n"
                                } else {
                                    $interpreted_values{$1} = $name;
                                    $name = $saved_name;
                                }
                            }
                        } # aliased to number, probably OK
                    } else {
                        $name = $4;
                    }
                }
                elsif (defined $6) {
                    $name = $6;
                }
                #print "  name: $name\n";
                if (defined $name) {
                    push @values, $name;
                    $current_value += 1;
                }
            }
        }
        push @params, @values;
    }

    #print_enum \%enum;

    return \%enum;
}


# parse_func
#
# The function data structure is a hash with the following members:
# PROTOTYPE = string, the original prototype
# RETTYPE = string, the return type
# FUNCNAME = string, the function name
# PARAMS = reference to array of hash references
#   Each hash reference is a single parameter with the following members:
#     NAME
#     TYPE
#
# This subroutine returns a reference to the function hash.
#
sub parse_func
{
    my $prototype = shift;
    my %func;
    my $more;
    my @params; # this is a list of a param hash

    # comment out the attr hint int the actual prototype
    my $actual_prototype = $prototype;
    $actual_prototype =~ s/(attr\{.+?})/\/* $1 *\//sg;
    $func{PROTOTYPE} = $actual_prototype;
#    print "'$actual_prototype'\n";


    ($func{RETTYPE}, $more) = $prototype =~ /(.*?)\s*([\s*\*])\w+\s*\(/;
    if ($more eq "*") {
        $func{RETTYPE} .= $more;
    }
    $func{RETTYPE_ISHANDLE} = ($func{RETTYPE} =~ /Handle$/ ||
                               $func{RETTYPE} =~ /NEXUS_(Video|Audio)(Input|Output)/
                               );
    ($func{FUNCNAME}) = $prototype =~ /(\w+)\s*\(/;

    # get the attributes and params into a raw list
    $prototype =~ /\(\s*(attr\{(.+?)})?(.*?)\)$/s;
    # function attribute check
    if(defined $2) {
        $func{ATTR} = parse_attr $2;
    }
    my $params=$3;
    # strip whitespace
    $params =~ s/^\s*(.*?)\s*$/$1/;
    if ($params eq "void") {
        $params = undef;
    }
    my @rawparams = split /,/, $params;
    my $p;

    for $p (@rawparams) {
        my %paramhash;

        # See if we have a attr hint and grab it now
        # This also removes that hint from the variable
        if ($p =~ s/attr\{(.+?)}//) {
#            print "$func{FUNCNAME} attr = $1\n";
            $paramhash{ATTR} = parse_attr $1;
        }

        # parse type and name
        my ($type, $name) = $p =~ /(\w[\w\s]*[\s\*]+)(\w+)/;
        # replace multiple whitespaces with a single whitespace */
        $type =~ s/\s+/ /g;
        # strip leading and trailing whitespace
        $type =~ s/^\s?(.*?)\s?$/$1/;

        $paramhash{NAME} = $name;
        $paramhash{TYPE} = $type;
        # a pointer in the type means the data is passed by reference
        $paramhash{ISREF} = ($type =~ /\*\s*$/);

        if ($paramhash{ISREF}) {
            # a const pointer is assumed to be an input parameter,
            # note that "const void **" is parsed as a non-const pointer to "const void *", so it's out
            # nexus does not parse [out]
            $paramhash{INPARAM} = ($type =~ /^\s*const/) && !($type =~ /\*\s*\*/);
            if ($paramhash{INPARAM}) {
                ($paramhash{BASETYPE}) = ($type =~ /^\s*const\s*(.*?)\s*\*\s*$/);
            }
            else {
                ($paramhash{BASETYPE}) = ($type =~ /^\s*(.*?)\s*\*\s*$/);

                # non-const void* params w/ attr{nelem} are out params, otherwise they are actually in params
                if ($paramhash{BASETYPE} eq 'void' && !defined $paramhash{ATTR}->{'nelem'}) {
                    $paramhash{INPARAM} = 1;
                }
                # out params with nelem but no nelem_out is poor api design. may deprecate.
                #if (defined $paramhash{ATTR}->{'nelem'} && !defined $paramhash{ATTR}->{'nelem_out'}) {
                #    print "WARNING: $func{FUNCNAME} out param $paramhash{NAME} missing nelem_out\n";
                #}
            }
        }
        else {
            # if not by-reference, then by-value, which is always in in param
            $paramhash{INPARAM} = 1;
            $paramhash{ISHANDLE} = ($paramhash{TYPE} =~ /Handle$/ ||
                                    $paramhash{TYPE} =~ /NEXUS_(Video|Audio)(Input|Output)/
                                    );
        }
        push @params, \%paramhash;
    }
    $func{PARAMS} = \@params;

    # The only case this is needed is bsettop_uninit (no return code, no parameters)
    # and this is a special exception. But I'm leaving it in for completeness.
    if ($#params == -1 && $func{RETTYPE} eq "void") {
        $func{NOSTRUCT} = 1;
    }

    # Return a reference to the func hash
    return \%func;
}

# this is the main function of the module
# it takes an array of functions as input
# and returns an array of function data structures
sub parse_funcs
{
    my $func;
    my @funcrefs;
    for $func (@_) {
        my $funcref = parse_func $func;

        push @funcrefs, $funcref;

#       print_api($funcref);
    }
    @funcrefs;
}

#
# debug printout
#
sub print_api
{
    my $func;
    for $func (@_) {
        my $params = $func->{PARAMS};
        my $param;
        print "Function: $func->{RETTYPE} $func->{FUNCNAME}\n";
        print_attr $func->{ATTR};
        for $param (@$params) {
            print "  Param: $param->{TYPE} $param->{NAME}\n";
            print "    isref:    $param->{ISREF}\n";
            print "    inparam:  $param->{INPARAM}\n";
            print "    basetype: $param->{BASETYPE}\n";
            print_attr $param->{ATTR};
        }
    }
}

sub version_api
{
    my $sum = 0;
    my $func;
    for $func (@_) {
        my $params = $func->{PARAMS};
        my $param;

        $sum += checksum $func->{RETTYPE}, $func->{FUNCNAME};
        for $param (@$params) {
            $sum += checksum $param->{TYPE}, $param->{NAME}, $param->{ISREF}, $param->{INPARAM}, $param->{BASETYPE};
            $sum += version_attr $param->{ATTR};
        }
    }
    return $sum;
}

1;
