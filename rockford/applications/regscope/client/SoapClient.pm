#############################################################################
# Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.

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
#############################################################################

package SoapClient;

use strict;

# for EJTAG SOAP server
use SOAP::Lite;
use Data::Dumper;
use MIME::Base64;
use Carp;
#use SOAP::Lite +trace => "debug";
use File::Basename;

my $URI = "urn:schemas-upnp-org:service:RegSvc:1";
my $URN_GET_MEM_RANGE = '"urn:schemas-upnp-org:service:NumberStorage:1#GetMemoryRange"';
my $URN_SET_MEM_RANGE = '"urn:schemas-upnp-org:service:NumberStorage:1#SetMemoryRange"';
my $URL_EJTAG = 'http://stbgit.broadcom.com/php/ejtag_name2ip.php';
my $P = basename $0;

use Exporter;
our @ISA = qw(Exporter);

our @EXPORT = qw(
    SetProxy
    GetProxy
    ReadReg
    WriteReg
    ReadMem
    WriteMem
    ReadMemBlkToFile
    WriteMemBlkFromFile
    ReadMemBlkToFileFormatted
    WriteMemBlkFromFileFormatted
);

our @EXPORT_OK = qw();

our $g_soap_proxy;

sub SetProxy($)
{
    my $ip;
    my @actions;
    my $ip_re = qr/^\d+\.\d+\.\d+\.\d+$/;
    $ip = shift;

    if ($ip =~ /[a-zA-Z]/)
    {
        if($^O =~ m/linux/i)
        {
            # It must be a probe name -- try to look it up.
            my $cmd = "wget $URL_EJTAG?name=$ip -q -O - ";
            my $new_ip = `$cmd 2>/dev/null`;
            chomp $new_ip;
            die "$P: Error: could not get ipaddr from probe '$ip'  $new_ip\n "
                    if (($? >> 8) || $new_ip !~ $ip_re);
            $ip = $new_ip;
        }
        else #WIN32
        {
            die "Error: Please specify EJTAG IP address.\n";
        }
    }
    elsif ($ip !~ $ip_re)
    {
        $g_soap_proxy = undef;
        print "$P: Error: incorrect syntax for ipaddr\n ";
        return undef
    }

   $g_soap_proxy = "http://$ip:1700/ncs_control";
   return 1;
}

sub GetProxy()
{
    return $g_soap_proxy;
}

#*************************************************************************func
# Purpose:
#   Read a register via a SOAP server
#
# Arguments:
#   $proxy    - the proxy addr of the SOAP server
#   $addr_v   - device's address.  This is usually given in the
#               data sheet of the device.
#
#   $count_v  - number of bytes to read
#
# Return:
#   register value
#*****************************************************************************
sub ReadReg($$)
{
    my($addr_v, $count_v, $verbose) = @_;

    my $c = SOAP::Lite
            ->readable(1)
            ->on_action(sub { return '"urn:schemas-upnp-org:service:RegSvc:1#GetRegisterValueAction"';})
            ->uri($URI)
            ->proxy($g_soap_proxy);

    my $count = SOAP::Data->new("name" => "Count", "type" => "");
    $count->value($count_v);

    my $addr = SOAP::Data->new("name" => "Address", "type" => "");
    $addr->value($addr_v);

    my $res = $c->GetRegisterValueAction($count, $addr);
    if ($res->fault && $verbose) {
            print "faultcode: ".$res->faultcode."\n";
            print "faultstring: ".$res->faultstring."\n";
            return 0;
    }
    else
    {
        return $res->result();
    }
}

#*************************************************************************func
# Purpose:
#   Write to a register via a SOAP server
#
# Arguments:
#   $proxy    - the proxy addr of the SOAP server
#   $addr_v   - device's address.  This is usually given in the
#               data sheet of the device.
#   $data_v   - the data to write
#   $count_v  - number of bytes to write
#
# Return:
#   register value
#*****************************************************************************
sub WriteReg($$$)
{
    my($addr_v, $data_v, $count_v, $verbose) = @_;

    my $c = SOAP::Lite
            ->readable(1)
            ->on_action(sub { return '"urn:schemas-upnp-org:service:RegSvc:1#SetRegister"';})
            ->uri($URI)
            ->proxy($g_soap_proxy);

    my $count = SOAP::Data->new("name" => "Count", "type" => "");
    $count->value($count_v);

    my $addr = SOAP::Data->new("name" => "AddressToSet", "type" => "");
    $addr->value($addr_v);

    my $data = SOAP::Data->new("name" => "DesiredValue", "type" => "");
    $data->value($data_v);

    my $res = $c->SetRegister($count, $addr, $data);
    if ($res->fault && $verbose)
    {
            print "faultcode: ".$res->faultcode."\n";
            print "faultstring: ".$res->faultstring."\n";
            return 0;
    }
    else
    {
        return 1;
    }
}

#*************************************************************************func
# Purpose:
#   Read memory via a SOAP server
#
# Arguments:
#   $proxy    - the proxy addr of the SOAP server
#   $addr_v   - device's address.  This is usually given in the
#               data sheet of the device.
#
#   $count_v  - number of bytes to read
#
# Return:
#   register value
#*****************************************************************************
sub ReadMem($$$$$)
{
    # Note: (in)  $addr_hi_v
    # Note: (in)  $data_lo_v
    # Note: (in)  $size_v (bytes)
    # Note: (out) $data_arr_v
    my ($addr_hi_v, $addr_lo_v, $size_v, $data_arr_v, $verbose) = @_;
    $size_v *= 4;

    my $c = SOAP::Lite
        ->readable(1)
        ->on_action(sub { return $URN_GET_MEM_RANGE; })
        ->uri($URN_GET_MEM_RANGE)
        ->proxy($g_soap_proxy);

    my $addr_hi = SOAP::Data->new(name => 'AddrHi')->value($addr_hi_v);
    my $addr_lo = SOAP::Data->new(name => 'AddrLo')->value($addr_lo_v);
    my $size = SOAP::Data->new(name => 'Count')->value($size_v);
    my $res = $c->GetMemoryRange($addr_hi, $addr_lo, $size);

    if ($res->fault && $verbose) {
        print "faultcode: ".$res->faultcode."\n";
        print "faultstring: ".$res->faultstring."\n";
        return undef;
    }
    else
    {
        my $data_base64 = $res->result();

        @{$data_arr_v} = unpack("N*", decode_base64($data_base64)) if ($data_base64);

        return @{$data_arr_v};
    }
}

########################################
#   Write memory via a SOAP server
#
# Arguments:
#   $proxy        - the proxy addr of the SOAP server
#   $addr_v       - device's address.  This is usually given in the
#                   data sheet of the device.
#   $data_arr_v   - data to write
#
# Return:
#   register value

########################################
sub WriteMem($$$$)
{
    # Note: (in)  $addr_hi_v
    # Note: (in)  $data_lo_v
    # Note: (out) $data_arr_v
    my ($addr_hi_v, $addr_lo_v, $data_arr_v, $verbose) = @_;


    my $size_v = scalar(@{$data_arr_v}) * 4;
    my $data_packed = pack("N*", @{$data_arr_v});

    my $c = SOAP::Lite
        ->readable(1)
        ->on_action(sub { return $URN_SET_MEM_RANGE; })
        ->uri($URN_SET_MEM_RANGE)
        ->proxy($g_soap_proxy);

    my $addr_hi = SOAP::Data->new(name => 'AddrHi')->value($addr_hi_v);
    my $addr_lo = SOAP::Data->new(name => 'AddrLo')->value($addr_lo_v);
    my $size = SOAP::Data->new(name => 'Count')->value($size_v);
    my $data = SOAP::Data->new(name => 'DataIn')->value($data_packed)->type('base64');

    my $res = $c->SetMemoryRange($addr_hi, $addr_lo, $size, $data);

    if ($res->fault && $verbose) {
        print "faultcode: ".$res->faultcode."\n";
        print "faultstring: ".$res->faultstring."\n";
        return undef;
    }
    else
    {
        return $size_v;
    }
}

sub ReadMemBlkToFile($$$$$)
{
    # Note: (in)  $addr_hi_v
    # Note: (in)  $addr_lo_v
    # Nite: (in)  $size_v
    # Note: (in)  $file (where to put the read data)
    # Note: (in)  $verbose (for debug messages)

    my ($addr_hi_v, $addr_lo_v, $size_v, $memfile, $verbose) = @_;
    my $ret;
    my @data;
    my @tmp;

    open (my $OUTFILE, ">", $memfile) or die "Unable to open: $!";
    binmode $OUTFILE;

    $ret = ReadMem($addr_hi_v, $addr_lo_v, $size_v, \@tmp, $verbose);
    if ($ret >= 1)
    {
        push @data, @tmp;

        if ($ret == 1)
        {
            print "Warning: Only read " . length(@data) . " dwords instead of ";
            print $size_v ." dwords. \n";
        }

        # Write data to file
        for (0..$ret-1)
        {
            print $OUTFILE pack("L", $data[$_]);
        }
    }

    close($OUTFILE);
    return $ret;
}

sub WriteMemBlkFromFile($$$$)
{
    # Note: (in)  $addr_hi
    # Note: (in)  $addr_lo
    # Note: (in)  $memfile
    # Note: (out) $verbose
    my ($addr_hi, $addr_lo, $memfile, $verbose) = @_;
    my $num_items_written = 0;
    my @val_arr;
    my $INFILE;

    open ($INFILE, "<:raw", $memfile) or die "Unable to open: $!";
    binmode $INFILE;

    my $data;
    my $count = 0;
    while ( read($INFILE, $data, 4, 0) )
    {
        my $_val = unpack("L", $data);

        push @val_arr, $_val;
    }

    # Write data
    $num_items_written = WriteMem($addr_hi, $addr_lo, \@val_arr, $verbose);
    close($INFILE);
    return $num_items_written;
}

sub ReadMemBlkToFileFormatted($$$$$)
{
    # Note: (in)  $addr_hi_v
    # Note: (in)  $addr_lo_v
    # Nite: (in)  $size_v
    # Note: (in)  $file (where to put the read data)
    # Note: (in)  $verbose (for debug messages)

    my ($addr_hi_v, $addr_lo_v, $size_v, $memfile, $verbose) = @_;
    my $ret;
    my @data;
    my @tmp;

    open (my $OUTFILE, ">", $memfile) or die "Unable to open: $!";
    binmode $OUTFILE;

    $ret = ReadMem($addr_hi_v, $addr_lo_v, $size_v, \@tmp, $verbose);
    if ($ret >= 1)
    {
        push @data, @tmp;

        if ($ret == 1)
        {
            print "Warning: Only read " . length(@data) . " dwords instead of ";
            print $size_v ." dwords. \n";
        }

        print $OUTFILE "// Memory\n";
        my $info_str = sprintf("%x\n", ($ret & ~15));
        print $OUTFILE $info_str;
        print $OUTFILE "\@0\n";

        # Write data to file
        for (0..$ret-1)
        {
            my $data_str = sprintf("0x%x\n", $data[$_]);
            print $OUTFILE $data_str;
        }
    }

    close($OUTFILE);
    return $ret;
}

sub WriteMemBlkFromFileFormatted($$$$)
{
    # Note: (in)  $addr_hi
    # Note: (in)  $addr_lo
    # Note: (in)  $memfile
    # Note: (out) $verbose
    my ($addr_hi, $addr_lo, $memfile, $verbose) = @_;
    return (WriteMemBlkFromFile($addr_hi, $addr_lo, $memfile, $verbose));
}

1;
