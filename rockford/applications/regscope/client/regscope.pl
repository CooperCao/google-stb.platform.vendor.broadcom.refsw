: # -*- perl -*- -p
eval 'exec perl -S -w $0 ${1+"$@"}'
if 0;

############################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#
# This program is the proprietary software of Broadcom and/or its
# licensors, and may only be used, duplicated, modified or distributed pursuant
# to the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and Broadcom
# expressly reserves all rights in and to the Software and all intellectual
# property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
#    constitutes the valuable trade secrets of Broadcom, and you shall use all
#    reasonable efforts to protect the confidentiality thereof, and to use
#    this information only in connection with your use of Broadcom integrated
#    circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
#    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
############################################################

use strict;
no warnings 'portable';  # Support for 64-bit ints required

our $gCodeBase;
our $gStartDir;

# add source path to the include path for the modules
BEGIN
{
    use Cwd;

    $| = 1;
    $0 =~ m|(.*)/|;

    $gStartDir = Cwd::getcwd();  chdir $1 if(defined $1);
    $gCodeBase = Cwd::getcwd();  chdir $gStartDir;

    if ($^O =~ m/linux/i)
    {
        if ($^V !~ m/v5.14.1/)
        {
            printf("\nYou must use Perl v5.14.1\n");
            printf("Set your PATH to include /tools/oss/packages/x86_64-rhel6/perl/5.14.1/bin/\n\n");
            die;
        }
    }
    else
    {
        if ($^O =~ m/Win32/i)
        {
            if ($^V !~ m/v5.14.2/)
            {
                printf("\nYou must use Perl v5.14.2\n");
                die;
            }
        }
    }
}

#
# Recommended method: Use the RDB-related perl module from the same relative
#   tree as this script.
#
use lib $gCodeBase . "/Broadcom";

# use of shell keystroke shortcuts eg up arrow for repeat commands
use Term::ReadLine;

#
# load RDB Parser modules & misc standard modules
#
use rdb::RDB;
use rdb::RDBCommon;
use Getopt::Std;
use File::Basename;

# for regscope server
use PerlExtLib;

# for EJTAG SOAP server
use SoapClient;

#****************************************************************************
# Global Variables
#****************************************************************************

# The default base which values are in
our $g_base = 16;

# Should unknown registers be written to!  Default easy to cause thing to hang?
our $g_allowunknown = 0;

# Turn on verbose messages
our $g_verbose = 0;

# Shuts off errors in RDBRegister stuff
our $g_hush = 1;

# Command line options
our %g_CmdOption = ();
our $g_Options   = "hnR:I:S:E:";
getopts($g_Options, \%g_CmdOption);

# List of registers to watch
our @g_watchlist = ();
our $g_watchshow = 1;

# Sanity check
our $g_sanitycheck = 1;

# Checkpoint
our %g_checkpoint = ();

# vnet_hash{vnet_valuename} = {vnet_abvname, vnet_regname}
#
# This is just look up table so that we have better consistence naming.
# TODO: May need to update this table if RDB naming deviate
#awk '{print $2}' vnet_b_values.txt | sort | uniq | cut -c 30-100 > vnet_b_names.txt
#grep -h -R BCHP_VNET_B_CAP_0_SRC_SOURCE_ magnum/basemodules/chp/include/ | sort | uniq > vnet_b_values.txt
#

my %vnet_hash = (
# value from VNET_B_*_SRC_*
    # passthru
    ("Free_Ch_0",           "FCH_0,VNET_F_FCH_0_SRC"  ),
    ("Free_Ch_1",           "FCH_1,VNET_F_FCH_1_SRC"  ),
    ("Free_Ch_2",           "FCH_2,VNET_F_FCH_2_SRC"  ),
    ("Free_Ch_3",           "FCH_3,VNET_F_FCH_3_SRC"  ),
    ("Free_Ch_4",           "FCH_4,VNET_F_FCH_4_SRC"  ),
    ("Free_Ch_5",           "FCH_5,VNET_F_FCH_5_SRC"  ),
    ("Free_Ch_6",           "FCH_6,VNET_F_FCH_6_SRC"  ),
    ("Free_Ch_7",           "FCH_7,VNET_F_FCH_7_SRC"  ),
    ("Free_Ch_8",           "FCH_8,VNET_F_FCH_8_SRC"  ),
    ("Free_Ch_9",           "FCH_9,VNET_F_FCH_9_SRC"  ),
    ("Free_Ch_10",          "FCH_A,VNET_F_FCH_10_SRC" ),
    ("Free_Ch_11",          "FCH_B,VNET_F_FCH_11_SRC" ),

    # middle processing blocks
    ("DNR",                 "DNR_0,VNET_F_DNR_SRC"    ),
    ("DNR_0",               "DNR_0,VNET_F_DNR_0_SRC"  ),
    ("DNR_1",               "DNR_1,VNET_F_DNR_1_SRC"  ),
    ("DNR_2",               "DNR_2,VNET_F_DNR_2_SRC"  ),
    ("DNR_3",               "DNR_3,VNET_F_DNR_3_SRC"  ),
    ("DNR_4",               "DNR_4,VNET_F_DNR_4_SRC"  ),
    ("DNR_5",               "DNR_5,VNET_F_DNR_5_SRC"  ),
    ("FGT_0",               "FGT_0,VNET_F_FGT_0_SRC"  ),
    ("MAD32",               "MAD_0,VNET_F_MAD32_SRC"  ),
    ("MAD_0",               "MAD_0,VNET_F_MAD_0_SRC"  ),
    ("MAD_1",               "MAD_1,VNET_F_MAD_1_SRC"  ),
    ("MCVP_0",              "MVP_0,VNET_F_MVCP_0_SRC" ),
    ("MCVP_1",              "MVP_1,VNET_F_MVCP_1_SRC" ),
    ("MVP_0",               "MVP_0,VNET_F_MVP_0_SRC"  ),
    ("MVP_1",               "MVP_1,VNET_F_MVP_1_SRC"  ),
    ("MVP_2",               "MVP_2,VNET_F_MVP_2_SRC"  ),
    ("MVP_3",               "MVP_3,VNET_F_MVP_3_SRC"  ),
    ("MVP_4",               "MVP_4,VNET_F_MVP_4_SRC"  ),
    ("MVP_5",               "MVP_5,VNET_F_MVP_5_SRC"  ),

    ("Scaler_0",            "SCL_0,VNET_F_SCL_0_SRC"  ),
    ("Scaler_1",            "SCL_1,VNET_F_SCL_1_SRC"  ),
    ("Scaler_2",            "SCL_2,VNET_F_SCL_2_SRC"  ),
    ("Scaler_3",            "SCL_3,VNET_F_SCL_3_SRC"  ),
    ("Scaler_4",            "SCL_4,VNET_F_SCL_4_SRC"  ),
    ("Scaler_5",            "SCL_5,VNET_F_SCL_5_SRC"  ),
    ("Scaler_6",            "SCL_6,VNET_F_SCL_6_SRC"  ),
    ("Scaler_7",            "SCL_7,VNET_F_SCL_7_SRC"  ),

    ("TNTD_0",              "TNT_0,VNET_F_TNTD_0_SRC" ),
    ("XSRC_0",              "XRC_0,VNET_F_XSRC_0_SRC" ),
    ("XSRC_1",              "XRC_1,VNET_F_XSRC_1_SRC" ),

# value from VNET_F_*_SRC_*
    ("Loopback_0",          "LPB_0,VNET_B_LOOPBACK_0_SRC"),
    ("Loopback_1",          "LPB_1,VNET_B_LOOPBACK_1_SRC"),
    ("Loopback_2",          "LPB_2,VNET_B_LOOPBACK_2_SRC"),
    ("Loopback_3",          "LPB_3,VNET_B_LOOPBACK_3_SRC"),
    ("Loopback_4",          "LPB_4,VNET_B_LOOPBACK_4_SRC"),
    ("Loopback_5",          "LPB_5,VNET_B_LOOPBACK_5_SRC"),
    ("Loopback_6",          "LPB_6,VNET_B_LOOPBACK_6_SRC"),
    ("Loopback_7",          "LPB_7,VNET_B_LOOPBACK_7_SRC"),
    ("Loopback_8",          "LPB_8,VNET_B_LOOPBACK_8_SRC"),
    ("Loopback_9",          "LPB_9,VNET_B_LOOPBACK_9_SRC"),
    ("Loopback_10",         "LPB_A,VNET_B_LOOPBACK_10_SRC"),
    ("Loopback_11",         "LPB_B,VNET_B_LOOPBACK_11_SRC"),
    ("Loopback_12",         "LPB_C,VNET_B_LOOPBACK_12_SRC"),
    ("Loopback_13",         "LPB_D,VNET_B_LOOPBACK_13_SRC"),
    ("Loopback_14",         "LPB_E,VNET_B_LOOPBACK_14_SRC"),

    ("Video_Feeder_0",      "CAP_0 -> VFD_0,VNET_B_CAP_0_SRC"),
    ("Video_Feeder_1",      "CAP_1 -> VFD_1,VNET_B_CAP_1_SRC"),
    ("Video_Feeder_2",      "CAP_2 -> VFD_2,VNET_B_CAP_2_SRC"),
    ("Video_Feeder_3",      "CAP_3 -> VFD_3,VNET_B_CAP_3_SRC"),
    ("Video_Feeder_4",      "CAP_4 -> VFD_4,VNET_B_CAP_4_SRC"),
    ("Video_Feeder_5",      "CAP_5 -> VFD_5,VNET_B_CAP_5_SRC"),
    ("Video_Feeder_6",      "CAP_6 -> VFD_6,VNET_B_CAP_6_SRC"),
    ("Video_Feeder_7",      "CAP_7 -> VFD_7,VNET_B_CAP_7_SRC"),

    ("VNET_F_LBOX_0_SRC",   "BOX_0,VNET_F_LBOX_0_SRC" ),
    ("VNET_F_LBOX_1_SRC",   "BOX_1,VNET_F_LBOX_1_SRC" ),
    ("VNET_F_LBOX_2_SRC",   "BOX_2,VNET_F_LBOX_2_SRC" ),
    ("VNET_F_LBOX_3_SRC",   "BOX_3,VNET_F_LBOX_3_SRC" ),
    ("VNET_F_LBOX_4_SRC",   "BOX_4,VNET_F_LBOX_4_SRC" ),
    ("VNET_F_DRAIN_0_SRC",  "DRN_0,VNET_F_DRAIN_0_SRC"),
    ("VNET_F_DRAIN_1_SRC",  "DRN_1,VNET_F_DRAIN_1_SRC"),
    ("VNET_F_DRAIN_2_SRC",  "DRN_2,VNET_F_DRAIN_2_SRC"),
    ("VNET_F_DRAIN_3_SRC",  "DRN_3,VNET_F_DRAIN_3_SRC"),
    ("VNET_F_DRAIN_4_SRC",  "DRN_4,VNET_F_DRAIN_4_SRC"),

    ("VNET_B_CAP_0_SRC",    "CAP_0,VNET_B_VFD_0_SRC"),
    ("VNET_B_CAP_1_SRC",    "CAP_1,VNET_B_VFD_1_SRC"),
    ("VNET_B_CAP_2_SRC",    "CAP_2,VNET_B_VFD_2_SRC"),
    ("VNET_B_CAP_3_SRC",    "CAP_3,VNET_B_VFD_3_SRC"),
    ("VNET_B_CAP_4_SRC",    "CAP_4,VNET_B_VFD_4_SRC"),
    ("VNET_B_CAP_5_SRC",    "CAP_5,VNET_B_VFD_5_SRC"),
    ("VNET_B_CAP_6_SRC",    "CAP_6,VNET_B_VFD_6_SRC"),
    ("VNET_B_CAP_7_SRC",    "CAP_7,VNET_B_VFD_7_SRC"),

    ("VNET_B_CMP_0_V0_SRC", "CMP_0_V0,VNET_B_CMP_0_V0_SRC"),
    ("VNET_B_CMP_0_V1_SRC", "CMP_0_V1,VNET_B_CMP_0_V1_SRC"),
    ("VNET_B_CMP_1_V0_SRC", "CMP_1_V0,VNET_B_CMP_1_V0_SRC"),
    ("VNET_B_CMP_1_V1_SRC", "CMP_1_V1,VNET_B_CMP_1_V1_SRC"),
    ("VNET_B_CMP_2_V0_SRC", "CMP_2_V0,VNET_B_CMP_2_V0_SRC"),
    ("VNET_B_CMP_3_V0_SRC", "CMP_3_V0,VNET_B_CMP_3_V0_SRC"),
    ("VNET_B_CMP_4_V0_SRC", "CMP_4_V0,VNET_B_CMP_4_V0_SRC"),
    ("VNET_B_CMP_5_V0_SRC", "CMP_5_V0,VNET_B_CMP_5_V0_SRC"),
    ("VNET_B_CMP_6_V0_SRC", "CMP_6_V0,VNET_B_CMP_6_V0_SRC"),
    ("VNET_B_CMP_7_V0_SRC", "CMP_7_V0,VNET_B_CMP_7_V0_SRC"),
    ("VNET_B_CRC_SRC",      "CRC_0,VNET_B_CRC_SRC"),
    ("VNET_B_DRAIN_0_SRC",  "DRN_0,VNET_B_DRAIN_0_SRC"),

    # TERMINAL
    ("MPEG_Feeder_0",       "MFD_0,TERMINAL_NODE"),
    ("MPEG_Feeder_1",       "MFD_1,TERMINAL_NODE"),
    ("MPEG_Feeder_2",       "MFD_2,TERMINAL_NODE"),
    ("MPEG_Feeder_3",       "MFD_3,TERMINAL_NODE"),
    ("MPEG_Feeder_4",       "MFD_4,TERMINAL_NODE"),
    ("MPEG_Feeder_5",       "MFD_5,TERMINAL_NODE"),
    ("CCIR_656_0",          "656_0,TERMINAL_NODE"),
    ("CCIR_656_1",          "656_1,TERMINAL_NODE"),
    ("CCIR656_1",           "656_1,TERMINAL_NODE"),
    ("HD_DVI_0",            "DVI_0,TERMINAL_NODE"),
    ("HD_DVI_1",            "DVI_1,TERMINAL_NODE"),
    ("VDEC_0",              "DEC_0,TERMINAL_NODE"),
    ("VDEC_1",              "DEC_1,TERMINAL_NODE"),
    ("Output_Disabled",     "OFF_0,TERMINAL_NODE"),
);


#****************************************************************************
# Main Program Entry
#****************************************************************************
print "\n"."Using perl version $^V"."\n\n";

if (defined $g_CmdOption{E} && defined $g_CmdOption{S})
{
    die "Please specify either Regscope or SOAP server, not both!\n";
}

##
# Open device
#
our $g_server = StdBcmOpen(\%g_CmdOption);

if(!defined $g_server)
{
    printf STDERR "\n\nError: bcmOpenDevice failed, reading/writing to registers disabled.\n";
    printf STDERR "\nThere is no server attached!\n";

    if (!defined ($g_CmdOption{n}))
    {
        printf STDERR "       (You can still decode, though).\n\n";
    }
}

our @g_RDBFiles;
our $g_rdb;

if($g_CmdOption{h} || $g_CmdOption{r})
{
    PrintUsage();
}
else
{
    ##
    # What to do?
    #   (1) Print helps
    #   (2) Dump registers
    #   (3) Set register field  -- Read/Modify/Write a register
    #   (4) Write register
    #   (5) Read register
    #   (6) Print more helps (default)
    #

    #
    # Load RDBs -- Register DataBases
    #

    if (!defined $g_CmdOption{n})
    {
        @g_RDBFiles = StdRDBFiles($g_CmdOption{R}, @INC);
        $g_rdb      = RDB->new(@g_RDBFiles);
        $g_rdb->CheckChipWithHW();
        $g_rdb->SetVarHush($g_hush);
        InteractiveMode();
    }
    else
    {
        # Accesses raw register data ie., no RDB
        $g_rdb = undef;
        print "\nThere is no RDB loaded. ";
        if (defined $g_server)
        {
            if (defined SoapClient::GetProxy())
            {
               printf("You are attached to an EJTAG so you must specify the ");
               printf("register's physical address.\n");
            }
            printf("\n");
            InteractiveMode();
        }
        else
        {
            printf("There is no server attached! Exiting.\n");
        }
    }
}

##
# Close device
#
if (defined $g_server)
{
    StdBcmClose(\%g_CmdOption);
}

exit(0);                               # All done!


#*************************************************************************func
# Purpose:
#   Reconnect to IP_Server/I2C_Port/NT_DD/etc.
#
#*****************************************************************************
sub ReConnect
{
    ##
    # Close device
    #
    StdBcmClose();

    ##
    # Re Open again.
    #
    if(!defined StdBcmOpen(\%g_CmdOption))
    {
        printf STDERR "\n\nError: bcmOpenDevice failed, reading/writing to registers disabled.\n";
        printf STDERR "       (You can still decode, though).\n\n";
    }

    ##
    # Load RDBs -- Register DataBases
    #
    @g_RDBFiles = StdRDBFiles($g_CmdOption{R}, @INC);
    $g_rdb->CheckChipWithHW();
    $g_rdb->SetVarHush($g_hush);

    return;
}


#*************************************************************************func
# Purpose:
#   Main user interactive mode.
#
# Arguments:
#   None
#
# Return:
#   None
#*****************************************************************************
sub InteractiveMode
{
    my $InputLine = "";

    my $chipid = $g_rdb->Chips->[0]->ID if (defined $g_rdb);

    my $Prompt    = ((defined $ENV{COMPUTERNAME}) && (defined $chipid))
        ? "$ENV{COMPUTERNAME}:$chipid>"
        : "regscope>";

    my $term;
    my $cmd;
    my $break;

    if($^O =~ m/linux/i)
    {
        $term  = new Term::ReadLine 'RegScope';
    }

    printf("\nRegScope  ver 3.1 (c) 2016 Broadcom\n");
    print("(Use \"h\" for help.)\n\n");

    do
    {
        $break = 0;

        if($^O =~ m/linux/i)
        {
            $cmd = $term->readline($Prompt);
        }
        else # Win32
        {
            print "$Prompt";
            $cmd = <STDIN>;
        }

        $InputLine = $cmd;

        if(!ExecuteCommand($InputLine))
        {
            $break = 1;
        }

        PrintWatch() if $g_watchshow;

    } while ($break == 0);
}

#*************************************************************************func
# Purpose:
#   Given a script in BBS Visual Basic, convert it to an RDB script
#
# Arguments:
#   @script
#
# Return:
#   None
#*****************************************************************************
sub ConvertVisualBasicToRegScope
{
    my $fields = shift;
    my @ret;

    foreach my $line (@_)
    {
        #'Configure DDR MEMC & do a simple
        #'MSA Read & Write
        #'0xFFE00000 - MiniTitan Base
        #
        #Option Explicit
        #
        #dim GlobalDevice
        #set GlobalDevice = Bcm7115
        #
        #'Reset miniTitan
        #'rFFE001A0 = &h08000
        #MemoryWrite &hFFE001A0, 4, &h8000
        #
        #'Program RB_MMC_MEMC_DRAM_MODE_REG (0x01)
        #'rFFE00004 = &h00007023
        #MemoryWrite &hFFE00004, 4, &h00007023
        #
        #'Program RB_MMC_MEMC_DRAM_MODE2_REG

        push @ret, "//-- $line";

        $line =~ s/'.*//;
        $line =~ s/script.write//i;
        $line =~ s/[()]/ /g;
        $line =~ s/^\s*//;
        $line =~ s/\s*$//;

        if($line ne '')
        {
            my $cmd = "n";
            my $reg;
            my $value;
            my $size;

            if($line =~ m/^MemoryWrite/i)
            {
                ($reg, $size, $value) =
                    $line=~/MemoryWrite\s+([^, ]+)\s*,\s*([0-9])\s*,\s*(.*)/i;
                if($size != 4)
                {
                    push @ret, "//** Warning, size not 4, pretending it is.";
                }

                $cmd = "mw";
                if($reg=~m/^\&h/i)
                {
                    $reg=~s/^&h//i;
                    $reg = hex($reg);
                    $reg = sprintf("%08x", $reg);
                    $reg="x:$reg";
                }

                if($value=~m/^&h/i)
                {
                    $value=~s/^&h//i;
                    $value=~s/&//i;
                    $value="x:$value";
                }
                else
                {
                    $value="d:$value";
                }
            }
            elsif($line =~ m/^MemoryRead/i)
            {
                ($reg, $size) =
                    $line=~/MemoryRead\s+([^, ]+)\s*,\s*([0-9])/i;
                if($size != 4)
                {
                    push @ret, "//** Warning, size not 4, pretending it is.";
                }

                $cmd = "mr";
                $value = '';
                if($reg=~m/^&h/i)
                {
                    $reg=~s/^&h//i;
                    $reg = hex($reg);
                    $reg = sprintf("%08x", $reg);
                    $reg="x:$reg";
                }
            }
            elsif($line =~ m/^\S+\s*=/i)
            {
                $cmd="w ";
                ($reg, $value) = $line=~/(\S+)\s*=\s*(\S+)/;

                my ($core, $register, $field) = split('\.', $reg);

                #printf "oldreg = %s, core = %s, reg = %s, field = %s\n",
                    #$reg, $core, $register, $field;
                $reg = $core . "_" . $register . " " . $field;

                if($value=~m/^&h/i)
                {
                    $value=~s/^&h//i;
                    $value=~s/&//i;
                    $value="x:$value";
                }
                else
                {
                    $value="d:$value";
                }
            }
            elsif($line =~ m/^hex\s+/i)
            {
                $cmd="mr";
                $value = '';
                ($reg) = $line=~/hex\s+(\S+)/i;
            }
            else
            {
                push @ret, "$line";
                push @ret, "//** Unknown command, ignored";
            }

            if($cmd ne 'n')
            {
                if($cmd ne 'w ' || !$fields)
                {
                    push @ret, "$cmd $reg $value";
                }
                else
                {
                    my $foo = DecodeToScript(DecodeRegisterToString($reg, $value));
                    foreach $foo (split(/\n/, $foo))
                    {
                        push @ret, $foo;
                    }
                }
            }

            push @ret, '';
        }
    }

    return @ret;
}


#*************************************************************************func
# Purpose:
#   Read in a script and execute it
#
# Arguments:
#   $filename    filename of script to run
#
# Return:
#   None
#*****************************************************************************
sub DoScript
{
    my $silent = 0;
    my $fields = 0;

    my $filename = shift;

    my $param = shift;
    while(defined($param))
    {
        if($param=~/^s/i)
        {
            $silent = 1;
        }
        elsif($param=~/^f/i)
        {
            $fields = 1;
        }

        $param = shift;
    }

    if($filename =~ m/.tcs$/i)
    {
        RunTCSScript($filename, 0);
    }
    else
    {
        my $script;
        open(my $SCRIPTFILE, "<$filename");
        if(defined($SCRIPTFILE))
        {
            local $/; # slurp
            $script = <$SCRIPTFILE>;
            close $SCRIPTFILE;
        }
        else
        {
            print "Error: Unable to open script file ($filename).\n";
        }

        my @script = split(/\n/, $script);

        if(($filename =~ m/.bsf$/i) ||
           ($filename =~ m/.bss$/i))
        {
            @script = ConvertVisualBasicToRegScope($fields, @script);
        }


        if(defined($script))
        {
            foreach my $InputLine (@script)
            {
                if(!$silent)
                {
                    print "$InputLine\n";
                }
                if(!ExecuteCommand($InputLine))
                {
                    last;
                }
            }
        }
    }
}

#*************************************************************************func
# Purpose:
#   Read in a TCS script and execute it
#
# Arguments:
#   $filename    filename of script to run
#   $regoffset   (optional) register offset for TCS script.
#
# Return:
#   None
#*****************************************************************************
sub RunTCSScript
{
    my $filename = shift;
    my $regoffset = shift;

    if (!defined($regoffset))
    {
        $regoffset = 0;
    }

    PerlExtLib::bcmRunTCS($filename, $regoffset);
}

#*************************************************************************func
# Purpose:
#   Read in a BBS Visual Basic script and convert it to a Regscope script
#
# Arguments:
#   $inscript   filename of script to convert
#   $outscript  filename of new regscope script
#   $fields     Flag determining if each field should be set individually
#               or all at once.
#
# Return:
#   None
#*****************************************************************************
sub ConvertScript
{
    my $inscript = shift;
    my $outscript = shift;
    my $fields = shift;

    my $script;
    open(my $SCRIPTFILE, "<$inscript");
    if(defined($SCRIPTFILE))
    {
        local $/; # slurp
        $script = <$SCRIPTFILE>;
        close $SCRIPTFILE;
    }
    else
    {
        print "Error: Unable to open script file ($inscript).\n";
    }

    my @script = split(/\n/, $script);
    @script = ConvertVisualBasicToRegScope($fields, @script);

    open($SCRIPTFILE, ">$outscript");
    if(defined($SCRIPTFILE))
    {
        print $SCRIPTFILE "//\n";
        print $SCRIPTFILE "// Script converted from $inscript on " . localtime() . "\n";
        print $SCRIPTFILE "//\n";
        print $SCRIPTFILE join("\n", @script);
        print $SCRIPTFILE "// End of Script\n";
        close $SCRIPTFILE;
    }
    else
    {
        print "Error: Unable to write to file ($outscript).\n";
    }
}

#*************************************************************************func
# Purpose:
#   Parse and execute a single command line
#
# Arguments:
#   $InputLine    The line to execute
#
# Return:
#   0 if quit or 1 if not
#*****************************************************************************
sub ExecuteCommand
{
    my $InputLine = shift;

    chomp($InputLine);
    $InputLine =~ s/\s*// if $InputLine;              # remove leading spaces
    ($InputLine) = split("//", $InputLine) if $InputLine;
    if($InputLine =~ /^\/\//)           # comment line begin w/ //
    {
        $InputLine = "";
    }
    $InputLine =~ s/\s+/ /g;            # compress whitespaces between argv's
    my @argv  = split(/\s/, $InputLine);

    if(defined $argv[0])
    {
        if((lc($argv[0]) eq "base") ||
           (lc($argv[0]) eq "b"))
        {
            if(defined $argv[1])
            {
                if($argv[1]==16 || $argv[1]==10 || $argv[1]==2)
                {
                    $g_base = $argv[1];
                }
                else
                {
                    printf("Error: Base must be set to 16, 10, or 2\n");
                }
            }
            printf("Default base set to %d\n", $g_base);
        }
        elsif(lc($argv[0]) eq "cpadd" ||
            lc($argv[0]) eq "cpa")
        {
            AddCheckpoint($argv[1], $argv[2]);
        }
        elsif(lc($argv[0]) eq "cpclear" ||
            lc($argv[0]) eq "cpc")
        {
            ClearCheckpoint();
        }
        elsif(lc($argv[0]) eq "cpdiff" ||
            lc($argv[0]) eq "cpd")
        {
            DeltaCheckpoint(@argv[1 .. $#argv]);
        }
        elsif(lc($argv[0]) eq "cpremove" ||
            lc($argv[0]) eq "cpr")
        {
            RemoveCheckpoint($argv[1], $argv[2]);
        }
        elsif(lc($argv[0]) eq "cpscript" ||
            lc($argv[0]) eq "cps")
        {
            DeltaScriptCheckpoint(@argv[1 .. $#argv]);
        }
        elsif(lc($argv[0]) eq "cpupdate" ||
            lc($argv[0]) eq "cpu")
        {
            UpdateCheckpoint();
        }
        elsif(lc($argv[0]) eq "cvt")
        {
            ConvertScript(@argv[1 ... $#argv]);
        }
        elsif(lc($argv[0]) eq "tcs")
        {
            RunTCSScript(@argv[1 ... $#argv]);
        }
        elsif(lc($argv[0]) eq "decode")
        {
            EnhancedDecode(@argv[1 .. $#argv]);
        }
        elsif(lc($argv[0]) eq "dump")
        {
            DumpRegisters(@argv[1 .. $#argv]);
        }
        elsif(lc($argv[0]) eq "iw")
        {
            WriteI2C(@argv[1 .. $#argv]);
        }
        elsif((lc($argv[0]) eq "help") ||
              (lc($argv[0]) eq "h"))
        {
            PrintHelp();
        }
        elsif(lc($argv[0]) eq "ir")
        {
            ReadI2C(@argv[1 .. $#argv]);
        }
        elsif(lc($argv[0]) eq "iw")
        {
            WriteI2C(@argv[1 .. $#argv]);
        }
        elsif((lc($argv[0]) eq "load") ||
              (lc($argv[0]) eq "l"))
        {
            my $error = $g_rdb->ParseFile(dirname($argv[1]), basename($argv[1]));
            if($error)
            {
                print $error->ErrorMsg;
                printf("\nError: Unable to load %s\n", $argv[1]);
            }
            else
            {
                push @g_RDBFiles, $argv[1];
            }
        }
        elsif(lc($argv[0]) eq "mrblk2")
        {
            ReadBlkMemInMemViewFormat(@argv[1 .. $#argv]);
        }
        elsif(lc($argv[0]) eq "mwblk2")
        {
            WriteBlkMemInMemViewFormat(@argv[1 .. $#argv]);
        }
        elsif(lc($argv[0]) eq "mrblk")
        {
            ReadBlkMem(@argv[1 .. $#argv]);
        }
        elsif(lc($argv[0]) eq "mwblk")
        {
            WriteBlkMem(@argv[1 .. $#argv]);
        }
        elsif((lc($argv[0]) eq "mr") || (lc($argv[0]) eq "zmr"))
        {
            ReadMemory(@argv[1 .. $#argv]);
        }
        elsif((lc($argv[0]) eq "mw") || (lc($argv[0]) eq "zmw"))
        {
            WriteMemory(@argv[1 .. $#argv]);
        }
        elsif((lc($argv[0]) eq "zmr8") || (lc($argv[0]) eq "lzmr8"))
        {
            ReadMemByte(@argv[0 .. $#argv]);
        }
        elsif((lc($argv[0]) eq "zmw8") || (lc($argv[0]) eq "lzmw8"))
        {
            WriteMemByte(@argv[0 .. $#argv]);
        }
        elsif(lc($argv[0]) eq "n")
        {
            PrintRegisters(@argv[1 .. $#argv]);
        }
        elsif((lc($argv[0]) eq "option") ||
              (lc($argv[0]) eq "o"))
        {
            ProcessOptions(@argv[1 .. $#argv]);
        }
        elsif((lc($argv[0]) eq "quit") ||
              (lc($argv[0]) eq "exit") ||
              (lc($argv[0]) eq "q"))
        {
            return 0;
        }
        elsif((lc($argv[0]) eq "bvn") ||
              (lc($argv[0]) eq "printbvn"))
        {
            DoPrintBvnRouting();
        }
        elsif((lc($argv[0]) eq "vec") ||
              (lc($argv[0]) eq "printvec"))
        {
            DoPrintVecRouting();
        }
        elsif((lc($argv[0]) eq "check") ||
              (lc($argv[0]) eq "checkbvn"))
        {
            DoPrintBvnErrors(0);
        }
        elsif((lc($argv[0]) eq "clear") ||
              (lc($argv[0]) eq "clearbvn"))
        {
            DoPrintBvnErrors(1);
        }
        elsif(lc($argv[0]) eq "r")
        {
            ReadRegister(@argv[1 .. $#argv]);
        }
        elsif(lc($argv[0]) eq "r64")
        {
            ReadRegister64(@argv[1 .. $#argv]);
        }
        elsif(lc($argv[0]) eq "rs")
        {
            ReadRegister('*script*', @argv[1 .. $#argv]);
        }
        elsif(lc($argv[0]) eq "s")
        {
            DoScript(@argv[1 ... $#argv]);
        }
        elsif(lc($argv[0]) eq "wa")
        {
            AddWatch($argv[1]);
        }
        elsif(lc($argv[0]) eq "wd")
        {
            $g_watchshow = 0;
        }
        elsif(lc($argv[0]) eq "we")
        {
            $g_watchshow = 1;
        }
        elsif(lc($argv[0]) eq "wr")
        {
            RemoveWatch($argv[1]);
        }
        elsif((lc($argv[0]) eq "connect") ||
              (lc($argv[0]) eq "reconnect"))
        {
            ReConnect();
        }
        elsif((lc($argv[0]) eq "reset") ||
              (lc($argv[0]) eq "reboot"))
        {
            Reboot();
        }
        elsif($argv[0]=~m/^w$/i)
        {
            if(defined $argv[3])
            {
                SetRegisterField($argv[1], $argv[2], $argv[3]);
            }
            else
            {
                WriteRegister($argv[1], $argv[2]);
            }
        }
        elsif($argv[0] eq "w64")
        {
            WriteRegister64($argv[1], $argv[2]);
        }
        elsif(defined $argv[2])           # SETFIELD register's field
        {
            SetRegisterField($argv[0], $argv[1], $argv[2]);
        }
        elsif(defined $argv[1])           # WRITE register
        {
            WriteRegister($argv[0], $argv[1]);
        }
        elsif(defined $argv[0])           # READ register
        {
            ReadRegister($argv[0]);
        }
    }

    return 1;
}

#*************************************************************************func
# Purpose:
#   Process options during interactive mode, and update global option
#   value.
#
# Arguments:
#   Option string
#
# Return:
#   None
#*****************************************************************************
sub ProcessOptions
{
    my $option = shift;

    if(defined $option)
    {
        if($option=~m/^a/i)
        {
            my $val = shift;
            if($val==0 || $val==1 || $val=~m/true/i || $val=~m/false/i)
            {
                $g_allowunknown = $val;
            }
            elsif($val=~m/true/i)
            {
                $g_allowunknown = 1;
            }
            elsif($val=~m/false/i)
            {
                $g_allowunknown = 0;
            }
            else
            {
                printf("Error: allowunknown must be set to 0 or 1\n");
            }
        }
        elsif($option=~m/^s/i)
        {
            my $val = shift;
            if($val==0 || $val==1 || $val=~m/true/i || $val=~m/false/i)
            {
                $g_sanitycheck = $val;
            }
            elsif($val=~m/true/i)
            {
                $g_sanitycheck  = 1;
            }
            elsif($val=~m/false/i)
            {
                $g_sanitycheck  = 0;
            }
            else
            {
                printf("Error: sanitycheck must be set to 0 or 1\n");
            }
        }
        elsif($option=~m/^v/i)
        {
            my $val = shift;
            if($val==0 || $val==1 || $val=~m/true/i || $val=~m/false/i)
            {
                $g_verbose = $val;
            }
            elsif($val=~m/true/i)
            {
                $g_verbose  = 1;
            }
            elsif($val=~m/false/i)
            {
                $g_verbose  = 0;
            }
            else
            {
                printf("Error: verbose must be set to 0 or 1\n");
            }
        }
    }

    printf("Current options:\n");
    printf("\ta-llowunknown = %d\n", $g_allowunknown);
    printf("\ts-anitycheck  = %d\n", $g_sanitycheck);
    printf("\tv-erbose      = %d\n", $g_verbose);
}

sub _ReadMem
{
    my $val;
    my $addr = shift;


    if (defined SoapClient::GetProxy())
    {
        my $addr_hi;
        my $addr_lo;
        my @data;
        my @tmp;
        my $num_addr_hi;
        my $num_addr_lo;


        if($^O =~ m/Win32/i)
        {
            # Below method to get upper and lower dwords are
            # needed in Windows. Somehow bit-wise operator >>
            # doesn't work in Windows
            my $_num_addr_hi;
            my $_num_addr_lo;

            # get upper 32 chars
            my $addr_hi_len = length($addr) - 8;
            $_num_addr_hi = substr($addr, 0, $addr_hi_len);

            # For 32-bit address. This  sets high word to 0
            if ($_num_addr_hi eq "")
            {
                $_num_addr_hi = 0;
            }

            # get lower 32 chars
            $_num_addr_lo = substr($addr, $addr_hi_len, 8);

            $num_addr_hi = $_num_addr_hi;
            $num_addr_lo = $_num_addr_lo;
        }
        else
        {
            $num_addr_hi = $addr >> 32;
            $num_addr_lo = $addr & 0xFFFFFFFF;
        }

        $addr_hi = $num_addr_hi;
        $addr_lo = $num_addr_lo;

        $val = SoapClient::ReadMem($addr_hi, $addr_lo, 1, \@tmp, $g_verbose);
        if (defined $val)
        {
            $val = $tmp[0];
        }
    }
    else
    {
        $val = PerlExtLib::bcmReadMem($addr);
    }

    return $val;
}

sub _WriteMem
{
    my ($addr, $val, $count) = @_;
    my $res;
    my @val_arr;
    push @val_arr, $val;

    if (defined SoapClient::GetProxy())
    {
        my $addr_hi;
        my $addr_lo;
        my $num_addr_hi;
        my $num_addr_lo;

        if($^O =~ m/Win32/i)
        {
            # Below method to get upper and lower dwords are
            # needed in Windows. Somehow bit-wise operator >>
            # doesn't work in Windows
            my $_num_addr_hi;
            my $_num_addr_lo;

            # get upper 32 chars
            my $addr_hi_len = length($addr) - 8;
            $_num_addr_hi = substr($addr, 0, $addr_hi_len);

            # For 32-bit address. This  sets high word to 0
            if ($_num_addr_hi eq "")
            {
                $_num_addr_hi = 0;
            }

            # get lower 32 chars
            $_num_addr_lo = substr($addr, $addr_hi_len, 8);

            $num_addr_hi = $_num_addr_hi;
            $num_addr_lo = $_num_addr_lo;
        }
        else
        {
            $num_addr_hi = $addr >> 32;
            $num_addr_lo = $addr & 0xFFFFFFFF;
        }

        $addr_hi = $num_addr_hi;
        $addr_lo = $num_addr_lo;

        $res = SoapClient::WriteMem($addr_hi, $addr_lo, \@val_arr, $g_verbose);
    }
    else
    {
        $res = PerlExtLib::bcmWriteMem($addr, $val, $count);
    }

    return $res;
}

sub _ReadReg
{
    my $ret;
    my $regaddr = shift;
    my $regobj = GetRegVar($regaddr);
    my $regsize = $regobj->Type->Size() if (defined $regobj);

    if (defined SoapClient::GetProxy())
    {
        my $bytes_to_read = ($regsize > 32) ? 8 : 4;
        $regaddr += $regobj->ChipRegBaseAddr;
        $ret = SoapClient::ReadReg($regaddr, $bytes_to_read);
    }
    else
    {
        $ret =  ($regsize > 32) ? PerlExtLib::bcmRead64($regaddr) :
                                  PerlExtLib::bcmRead($regaddr);
    }
    return $ret;
}

sub _WriteReg
{
    my $ret;
    my $regaddr = shift;
    my $regvalue = shift;
    my $regobj = GetRegVar($regaddr);
    my $regsize = $regobj->Type->Size() if (defined $regobj);
    my $_addr = $regobj->BaseAddr() + $regobj->GetAddr();

    if (defined SoapClient::GetProxy())
    {
        my $bytes_to_write = ($regsize > 32) ? 8 : 4;
        $_addr += $regobj->ChipRegBaseAddr;
        $ret = SoapClient::WriteReg($_addr, $regvalue, $bytes_to_write);
    }
    else
    {
        $ret =  ($regsize > 32) ? PerlExtLib::bcmWrite64($_addr, $regvalue) :
                                  PerlExtLib::bcmWrite($_addr, $regvalue);
    }

    return $ret;
}

#*************************************************************************func
# Purpose:
#   Read SDRAM
#
# Arguments:
#   start address of memory to be read.  Address must be dword alignment
#
# Return:
#   Out result to screen
#*****************************************************************************
sub ReadMemory
{
    my @argv = @_;
    my $count = 1;

    if(defined $argv[1])
    {
        # read several dwords
        $count = atoi($argv[1]);

        if(!defined $count)
        {
            $count = 1;
        }
        elsif($g_sanitycheck && $count>128)
        {
            printf("Warning: You're trying to read more than 128 dwords, which is quite a lot\n");
            printf("         and is likely not what you intended. Stopping at 128.\n");
            printf("\n");
            printf("Hint: If you really want to read lots of data, you can turn off checks like\n");
            printf("      this by shutting off sanity checks option (i.e. option sanitycheck 0).\n\n");

            $count = 128;
        }
        elsif($count<1)
        {
            printf("Warning: Reading less than one dword has little effect.\n\n");
        }
    }

    my $addr;

    if (defined SoapClient::GetProxy() || ($^O =~ m/linux/i))
    {
        $addr = atoi($argv[0]);
    }
    else # Windows need string address to handle 40-bit memory addresses
    {
        $addr = ($argv[0]);
    }

    if(defined $addr)
    {
        if ((defined SoapClient::GetProxy()) || ($^O =~ m/linux/i))
        {
            for(my $curaddr = $addr;
                $curaddr<($addr+4*$count);
                $curaddr+=4)
            {
                my $val = _ReadMem($curaddr);

                if(defined $val)
                {
                    if ($^O =~ m/Win32/i)
                    {
                        printf("0x%08x = 0x%08x\n", $curaddr, $val);
                    }
                    else
                    {
                        printf("0x%08x = 0x%08x\n", $curaddr, $val);
                    }
                }
                else
                {
                    printf("Error: Unable to read from device address 0x%08x\n\n", $curaddr);
                    last;
                }
            }
        }
        else
        {
             my $val;
             my $num_chars_to_replace = 0;
             for (0..$count-1)
             {
                 my $elem_addr = $addr;
                # increment offset by 4 but done through a string since Win32 cannot handle  > 32-bit values
                 my $replacement_offset = sprintf("%x", ($_*4));

                 # account for how many characters need replacing. 16 is used because the replacement offset
                 # maxes out at 16 and carries over to the next character
                 my $char_pos_to_replace = ($num_chars_to_replace > 0) ? (16 ** $num_chars_to_replace) : 16;
                 if ((atoi($replacement_offset) % $char_pos_to_replace) == 0)
                 {
                     $num_chars_to_replace += 1;
                 }
                 substr($elem_addr, length($addr) - $num_chars_to_replace, $num_chars_to_replace) = $replacement_offset;

                 $val = _ReadMem($elem_addr);
                 printf("0x%s = 0x%08x\n", $elem_addr, $val);
             }
        }
    }
    else
    {
        printf("Error: <%s> isn't a recognizable address\n", $argv[0]);
    }
}

#*************************************************************************func
# Purpose:
#   Write SDRAM or fill
#
# Arguments:
#   Starting Address, and counts (number of dword).
#
# Return:
#   None
#*****************************************************************************
sub WriteMemory
{
    my @argv = @_;
    my $count = 1;
    my $addr  = undef;
    my $val   = undef;

    if((defined $argv[2]) && ($argv[1] =~ /\:/))
    {
        # setfield at memory location.  It has the form of
        # mw <address> <high:low> <value>
        $val  = atoi($argv[2]);

        if (defined SoapClient::GetProxy() || ($^O =~ m/linux/i))
        {
            $addr = atoi($argv[0]);
        }
        else # Windows need string address to handle 40-bit memory addresses
        {
            $addr = $argv[0];
        }

        my ($High, $Low) = split(":", $argv[1]);

        #
        # Swap if user swapping low:high
        #
        if(eval($High) < eval($Low))
        {
            my $Temp = $High;
            $High = $Low;
            $Low = $Temp;
        }

        #
        # Make sure that value will is not overlow in given field.
        # Shiftting 32 bits will wrap around and max_field_value will eq to 0
        # which will failed the test condition.  If we see 32-bit field it will
        # not check.
        #
        my $field_value     = $val;
        my $field_width     = eval($High-$Low) + 1;
        my $max_field_value = (1 << $field_width) - 1;

        if(($field_value > $max_field_value) && ($field_width != 32))
        {
            printf(STDERR "ERROR: Are you sure you want to assign $field_value ");
            printf(STDERR "to 0x%08x[%s] (%d-bit).\n", $addr, "$High:$Low", $field_width);
            return;
        }
        #
        # do a [read :: modify :: write] on the address.
        #
        my $data = _ReadMem($addr);

        # modify
        if ($field_width < 32)
        {
          my $mask = $max_field_value << $Low;
          $data = (($data) & ~$mask) | ($field_value << $Low);
        }
        elsif (($field_width == 32) && ($Low == 0))
        {
          $data = $field_value;
        }

        # write
        $val = $data;
    }
    elsif(defined $argv[2])
    {
        # fill memory
        # mw <address> <value> <count>
        $count = atoi($argv[2]);
        $val   = atoi($argv[1]);

        if (defined SoapClient::GetProxy() || ($^O =~ m/linux/i))
        {
            $addr = atoi($argv[0]);
        }
        else # Windows need string address to handle 40-bit memory addresses
        {
            $addr = $argv[0];
        }

        if(!defined $count)
        {
            $count = 1;
        }
        elsif($count<1)
        {
            printf("Warning: Writing less than one dword has little effect.\n\n");
        }
    }
    else
    {
        # normal memory write
        # mw <address> <value>
        $val  = atoi($argv[1]);

        if (defined SoapClient::GetProxy() || ($^O =~ m/linux/i))
        {
            $addr = atoi($argv[0]);
        }
        else # Windows need string address to handle 40-bit memory addresses
        {
            $addr = $argv[0];
        }
    }

    if(defined $addr && defined $val)
    {
        my $res = _WriteMem($addr, $val, $count);

        if(!defined $res)
        {
            printf("Error: Unable to write to device address 0x%08x\n\n", $addr);
            last;
        }
    }
    else
    {
        printf("Error: <%s> isn't a recognizable address or <%s> isn't a valid value.\n",
            $argv[0], $argv[1]);
    }

    return;
}

#*************************************************************************func
# Purpose:
#   Read one byte to SDRAM or fill
#
# Arguments:
#   big/little endian, starting Address, and counts (number of bytes).
#
# Return:
#   None
#*****************************************************************************
sub ReadMemByte
{
    my @argv = @_;
    my $count = 1;
    my $addr  = undef;
    my $val   = undef;

    # normal memory write
    # zmr8 <address>
    $addr = atoi($argv[1]);

    if(defined $addr)
    {
        #
        # do a [read] on the address.
        #
        my $dwAddr = ($addr) & ~3;
        my $byteOffset = ($addr) - ($dwAddr);
        my $data = _ReadMem($dwAddr);

        my $mask;
        if(!defined $data)
        {
            printf("Error: Unable to read to device address 0x%08x\n\n", $addr);
            last;
        }

        # Note: I2C bus always assumes Big-Endian addressing mode
        if( lc($argv[0]) eq "zmr8" )   # big endian
        {
            $mask = 255 << (8 * ($byteOffset));
            $val = (($data) & $mask) >> (8 * ($byteOffset));
        }
        else                           # little endian
        {
            $mask = 255 << (8 * (3 - ($byteOffset)));
            $val = (($data) & $mask) >> (8 * (3 - ($byteOffset)));
        }
        printf("0x%08x = 0x%02x\n", $addr, $val);
    }
    else
    {
        printf("Error: <%s> isn't a recognizable address or <%s> isn't a valid value.\n",
            $argv[0], $argv[1]);
    }

    return;
}

#*************************************************************************func
# Purpose:
#   Write one byte to SDRAM or fill
#
# Arguments:
#   big/little endian, starting Address, and counts (number of bytes).
#
# Return:
#   None
#*****************************************************************************
sub WriteMemByte
{
    my @argv = @_;
    my $count = 1;
    my $addr  = undef;
    my $val   = undef;

    # normal memory write
    # zmw8 <address> <value>
    $val  = atoi($argv[2]) & 255; # only pick the LSByte
    $addr = atoi($argv[1]);

    if(defined $addr && defined $val)
    {
        #
        # do a [read :: modify :: write] on the address.
        #
        my $dwAddr = ($addr) & ~3;
        my $byteOffset = ($addr) - ($dwAddr);
        my $data = _ReadMem($dwAddr);

        my $mask;
        if( lc($argv[0]) eq "zmw8" )   # big endian
        {
            $mask = 255 << (8 * ($byteOffset));                     # modify
            $val = (($data) & ~$mask) | ($val << (8 * ($byteOffset))); # write
        }
        else                           # little endian
        {
            $mask = 255 << (8 * (3 - ($byteOffset)));                     # modify
            $val = (($data) & ~$mask) | ($val << (8 * (3 - ($byteOffset)))); # write
        }

        my $res = _WriteMem($dwAddr, $val, $count);

        if(!defined $res)
        {
            printf("Error: Unable to write to device address 0x%08x\n\n", $addr);
            last;
        }
    }
    else
    {
        printf("Error: <%s> isn't a recognizable address or <%s> isn't a valid value.\n",
            $argv[0], $argv[1]);
    }

    return;
}


#*************************************************************************func
# Purpose:
#   Dump memory to file.
#
# Arguments:
#   $StartAddress (dword address)
#   $Count        (number of dword)
#   $Memfile      (file name to write out)
#
# Return:
#   memory content is in $Memfile
#*****************************************************************************
sub ReadBlkMem
{
    my $addr = shift;
    my $Count   = atoi(shift);
    my $Memfile = shift;
    my $val;
    my $_addr;

    if(defined $Memfile)
    {
        if (defined SoapClient::GetProxy())
        {
            my $num_addr_hi;
            my $num_addr_lo;

            $_addr = atoi($addr);

            if($^O =~ m/Win32/i)
            {
                # Below method to get upper and lower dwords are
                # needed in Windows. Somehow bit-wise operator >>
                # doesn't work in Windows
                my $_num_addr_hi;
                my $_num_addr_lo;

                # get upper 32 chars
                my $addr_hi_len = length($_addr) - 8;
                $_num_addr_hi = substr($_addr, 0, $addr_hi_len);

                # For 32-bit address. This  sets high word to 0
                if ($_num_addr_hi eq "")
                {
                    $_num_addr_hi = 0;
                }

                # get lower 32 chars
                $_num_addr_lo = substr($_addr, $addr_hi_len, 8);

                $num_addr_hi = $_num_addr_hi;
                $num_addr_lo = $_num_addr_lo;
            }
            else
            {
                $num_addr_hi = $_addr >> 32;
                $num_addr_lo = $_addr & 0xFFFFFFFF;
            }

            my $addr_hi = $num_addr_hi;
            my $addr_lo = $num_addr_lo;

            $val = SoapClient::ReadMemBlkToFile($addr_hi, $addr_lo, $Count, $Memfile, $g_verbose);
        }
        else
        {
            if ($^O =~ m/linux/i)
            {
                $_addr = atoi($addr);
            }
            else# Windows need string address to handle 40-bit memory addresses
            {
                $_addr = $addr;
            }

            $val = PerlExtLib::bcmReadMemBlkToFile($_addr, $Count, $Memfile);
        }

        if(defined $val)
        {
            printf("Read %s dwords from SDRAM starting at ", $val);
            if($^O =~ m/linux/i)
            {
                printf("0x%08x.\n", $_addr);
            }
            else
            {
                printf("0x%s.\n", $addr);
            }
        }
    }
    else
    {
        if (!defined $Memfile)
        {
            printf("Error: Missing file that will store values read.\n");
        }
        if (!defined $Count)
        {
            printf("Error: Missing number of locations to read.\n");
        }
    }
}


#*************************************************************************func
# Purpose:
#  Load Memory image file to SDRAM
#
# Arguments:
#   $StartAddress (dword address)
#   $Memfile      (file name to write out)
#
# Return:
#   None.  Ignore last impartial dword in file.
#*****************************************************************************
sub WriteBlkMem
{
    my $addr = shift;
    my $val;
    my $Memfile = shift;
    my $_addr;

    if(defined $Memfile && -e $Memfile)
    {
        if (defined SoapClient::GetProxy())
        {
            my $addr_hi;
            my $addr_lo;
            my $num_addr_hi;
            my $num_addr_lo;

            $_addr = atoi($addr);

            if($^O =~ m/Win32/i)
            {
                # Below method to get upper and lower dwords are
                # needed in Windows. Somehow bit-wise operator >>
                # doesn't work in Windows
                my $_num_addr_hi;
                my $_num_addr_lo;

                # get upper 32 chars
                my $addr_hi_len = length($_addr) - 8;
                $_num_addr_hi = substr($_addr, 0, $addr_hi_len);

                # For 32-bit address. This  sets high word to 0
                if ($_num_addr_hi eq "")
                {
                    $_num_addr_hi = 0;
                }

                # get lower 32 chars
                $_num_addr_lo = substr($_addr, $addr_hi_len, 8);

                $num_addr_hi = $_num_addr_hi;
                $num_addr_lo = $_num_addr_lo;
            }
            else
            {
                $num_addr_hi = $_addr >> 32;
                $num_addr_lo = $_addr & 0xFFFFFFFF;
            }

            $addr_hi = $num_addr_hi;
            $addr_lo = $num_addr_lo;

            $val = SoapClient::WriteMemBlkFromFile($addr_hi, $addr_lo, $Memfile, $g_verbose);
        }
        else
        {
            if ($^O =~ m/linux/i)
            {
                $_addr = atoi($addr);
            }
            else# Windows need string address to handle 40-bit memory addresses
            {
                $_addr = $addr;
            }

            $val = PerlExtLib::bcmWriteMemBlkFromFile($_addr, $Memfile);
        }

        if(defined $val)
        {
            printf("Wrote %s dwords to SDRAM starting at ", $val/4);
            if($^O =~ m/linux/i)
            {
                printf("0x%08x.\n", $_addr);
            }
            else
            {
                printf("0x%s.\n", $addr);
            }
        }
    }
    else
    {
        printf("Error: Missing file that contain values to write to memory.\n");
    }

}

#*************************************************************************func
# Purpose:
#   read sdram and store in mem file
#
# Arguments:
#   $ChipAddr  - an i2c device's address.  This is usually given in the
#     data sheet of the device.
#
#   $SubAddr   - an i2c device sub address
#
#   $Data      - data byte that will go to the device's sub address.
#
# Return:
#   None!  Just output result to stdout.
#*****************************************************************************
sub ReadBlkMemInMemViewFormat()
{
    my $addr = shift;
    my $Count   = atoi(shift);
    my $Memfile = shift;
    my $_addr;

    if(defined $Memfile)
    {
        my $val;

        if (defined SoapClient::GetProxy())
        {
            my $num_addr_hi;
            my $num_addr_lo;

            $_addr = atoi($addr);

            if($^O =~ m/Win32/i)
            {
                # Below method to get upper and lower dwords are
                # needed in Windows. Somehow bit-wise operator >>
                # doesn't work in Windows
                my $_num_addr_hi;
                my $_num_addr_lo;

                # get upper 32 chars
                my $addr_hi_len = length($_addr) - 8;
                $_num_addr_hi = substr($_addr, 0, $addr_hi_len);

                # For 32-bit address. This  sets high word to 0
                if ($_num_addr_hi eq "")
                {
                    $_num_addr_hi = 0;
                }

                # get lower 32 chars
                $_num_addr_lo = substr($_addr, $addr_hi_len, 8);

                $num_addr_hi = $_num_addr_hi;
                $num_addr_lo = $_num_addr_lo;
            }
            else
            {
                $num_addr_hi = $_addr >> 32;
                $num_addr_lo = $_addr & 0xFFFFFFFF;
            }

            my $addr_hi = $num_addr_hi;
            my $addr_lo = $num_addr_lo;

            $val = SoapClient::ReadMemBlkToFileFormatted($addr_hi, $addr_lo, $Count, $Memfile, $g_verbose);
        }
        else
        {
            if ($^O =~ m/linux/i)
            {
                $_addr = atoi($addr);
            }
            else# Windows need string address to handle 40-bit memory addresses
            {
                $_addr = $addr;
            }

            $val = PerlExtLib::bcmReadMemBlkToFileFormatted($_addr, $Count, $Memfile);
        }

        if(defined $val)
        {
            printf("Read %s dwords from SDRAM starting at ", $val);
            if($^O =~ m/linux/i)
            {
                printf("0x%08x ", $_addr);
            }
            else
            {
                printf("0x%s ", $addr);
            }
            printf("and stored in $Memfile (in MemView File Format).\n");
        }
    }
    else
    {
        if (!defined $Memfile)
        {
            printf("Error: Missing file that will store values read.\n");
        }
        if (!defined $Count)
        {
            printf("Error: Missing number of locations to read.\n");
        }
    }
}


#*************************************************************************func
# Purpose:
#   write mem file to sdram
#
# Arguments:
#   $ChipAddr  - an i2c device's address.  This is usually given in the
#     data sheet of the device.
#
#   $SubAddr   - an i2c device sub address
#
#   $Data      - data byte that will go to the device's sub address.
#
# Return:
#   None!  Just output result to stdout.
#*****************************************************************************
sub WriteBlkMemInMemViewFormat()
{
    my $addr = shift;;
    my $Memfile = shift;
    my $_addr;

    if(defined $Memfile && -e $Memfile)
    {
        my $val;

        if (defined SoapClient::GetProxy())
        {
            my $num_addr_hi;
            my $num_addr_lo;

            $_addr = atoi($addr);

            if($^O =~ m/Win32/i)
            {
                # Below method to get upper and lower dwords are
                # needed in Windows. Somehow bit-wise operator >>
                # doesn't work in Windows
                my $_num_addr_hi;
                my $_num_addr_lo;

                # get upper 32 chars
                my $addr_hi_len = length($_addr) - 8;
                $_num_addr_hi = substr($_addr, 0, $addr_hi_len);

                # For 32-bit address. This  sets high word to 0
                if ($_num_addr_hi eq "")
                {
                    $_num_addr_hi = 0;
                }

                # get lower 32 chars
                $_num_addr_lo = substr($_addr, $addr_hi_len, 8);

                $num_addr_hi = $_num_addr_hi;
                $num_addr_lo = $_num_addr_lo;
            }
            else
            {
                $num_addr_hi = $_addr >> 32;
                $num_addr_lo = $_addr & 0xFFFFFFFF;
            }

            my $addr_hi = $num_addr_hi;
            my $addr_lo = $num_addr_lo;

            $val = SoapClient::WriteMemBlkFromFileFormatted($addr_hi, $addr_lo, $Memfile, $g_verbose);
        }
        else
        {
            if ($^O =~ m/linux/i)
            {
                $_addr = atoi($addr);
            }
            else# Windows need string address to handle 40-bit memory addresses
            {
                $_addr = $addr;
            }

            $val = PerlExtLib::bcmWriteMemBlkFromFileFormatted($_addr, $Memfile);
        }

        if(defined $val)
        {
            printf("Wrote %s dwords to SDRAM starting at ", $val/4);
            if($^O =~ m/linux/i)
            {
                printf("0x%08x.\n", $_addr);
            }
            else
            {
                printf("0x%s.\n", $addr);
            }
        }
    }
    else
    {
        printf("Error: Missing file that contain values to write to memory.\n");
    }
}


#*************************************************************************func
# Purpose:
#   Read register
#
# Arguments:
#   $regname  - register address or name.
#   $count    - count of registers to dump (optional)
#   $filename - filename to dump to (optional)
#
# Return:
#   print out result.
#*****************************************************************************
sub ReadRegister
{
    my @argv = @_;
    my $dump_as_script = 0;
    my $DUMPFILE;

    #
    # OK, this is a hack, but this program is just not designed right any
    # more. It's grown much bigger than we expected. Anyway, this looks for
    # the "dump as script" flag.
    #
    if(defined $argv[0])
    {
        if($argv[0] eq '*script*')
        {
            shift @argv;
            $dump_as_script = 1;
        }
    }

    my $regname  = shift @argv;
    my $regnum   = atoi($regname);

    my $count = 1;
    my $regobj;
    my $numregsread = 0;

    if ((!defined $regnum) && (defined $g_rdb))
    {
        $regobj = GetRegVar($regname);
        if(!defined($regobj))
        {
            printf("Error: <%s> isn't a valid register name or address.\n", $regname);
            if($g_allowunknown==0)
            {
                printf("Hint: Turn on the allowunknown option to read from undefined registers.\n");
                printf("      (regscope> o allowunknown 1)\n");
            }
            return;
        }
        $regnum = $regobj->BaseAddr + $regobj->Offset;
    }
    $regname = $regnum;

    if(defined $argv[0])
    {
        # read several registers
        $count = atoi($argv[0]);
    }

    if(defined $argv[1])
    {
        open $DUMPFILE, ">$argv[1]" or die();
        if(!defined($DUMPFILE))
        {
            print "Error: Unable to open output file ($argv[1])\n";
            return;
        }
        select $DUMPFILE;
    }

    while ($numregsread < $count)
    {
        my $curreg = $regnum;

        if (!defined $g_rdb)
        {
            my $rawres = _ReadRaw($curreg, 0);
            if (defined $rawres)
            {
                printf("%s\n", $rawres);
            }
            else
            {
                printf("Failed to read register 0x%08x\n", $curreg);
            }
        }
        else
        {
            $regobj = GetRegVar($curreg);

            if(defined $regobj)
            {
                my $longname = $regobj->ID;
                printf("Reading %s register.\n", $longname) if $g_verbose;
                if(defined $regobj->ReadHW($g_verbose))
                {
                    my $decoded = $regobj->Decode($regobj->Get, $g_verbose);

                    if($dump_as_script)
                    {
                        $decoded = DecodeToScript($decoded);
                    }

                    printf("%s\n", $decoded);


                    if ($regobj->Type->GetProp("LongReg"))
                    {
                        $curreg += 4;
                    }
                }
                else
                {
                    printf("%s = <unknown>\n", $longname);
                }
            }
            elsif($g_allowunknown && $regnum!=0)
            {
                printf("Reading 0x%08x register.\n", $regnum) if $g_verbose;

                my $Ret;
                $regobj = GetRegVar($regnum);

                my $islongreg = $regobj->Type->GetProp("LongReg") if (defined $regobj);

                $Ret = _ReadReg($regnum);
                if ($islongreg)
                {
                    $curreg += 4;
                }

                if(defined $Ret)
                {
                    printf("0x%08x = 0x%08x\n", $regnum, $Ret);
                }
                else
                {
                    printf("0x%08x = <unknown>\n", $regnum);
                }
            }
            else
            {
                printf("Error: <%s> isn't a valid register name or address.\n", $regname);
                if($g_allowunknown==0)
                {
                    printf("Hint: Turn on the allowunknown option to read from undefined registers.\n");
                    printf("      (regscope> o allowunknown 1)\n");
                }
            }
        }
        $curreg+=4;
        $numregsread++;
        $regnum = $curreg;
    }

    if(defined $DUMPFILE)
    {
        select STDOUT;
        close $DUMPFILE;
    }

    print("\n");
}

#*************************************************************************func
# Purpose:
#   Read 64-bit register
#
# Arguments:
#   $regname  - register address or name.
#   $count    - count of registers to dump (optional)
#   $filename - filename to dump to (optional)
#
# Return:
#   print out result.
#*****************************************************************************
sub ReadRegister64
{
    my @argv = @_;
    my $dump_as_script = 0;
    my $DUMPFILE;

    #
    # OK, this is a hack, but this program is just not designed right any
    # more. It's grown much bigger than we expected. Anyway, this looks for
    # the "dump as script" flag.
    #
    if(defined $argv[0])
    {
        if($argv[0] eq '*script*')
        {
            shift @argv;
            $dump_as_script = 1;
        }
    }

    my $regname  = shift @argv;
    my $regnum   = atoi($regname);

    my $count = 1;
    my $numregsread = 0;

    $regname = $regnum;

    if(defined $argv[0])
    {
        # read several registers
        $count = atoi($argv[0]);
    }

    if(defined $argv[1])
    {
        open $DUMPFILE, ">$argv[1]" or die();
        if(!defined($DUMPFILE))
        {
            print "Error: Unable to open output file ($argv[1])\n";
            return;
        }
        select $DUMPFILE;
    }

    while ($numregsread < $count)
    {
        my $curreg = $regnum;
        my $rawres = _ReadRaw($curreg, 1);
        if (defined $rawres)
        {
            printf("%s\n", $rawres);
        }
        else
        {
            printf("Failed to read %x\n", $curreg);
        }

        $curreg+=4;
        $numregsread++;
        $regnum = $curreg;
    }

    if(defined $DUMPFILE)
    {
        select STDOUT;
        close $DUMPFILE;
    }

    print("\n");
}

#*************************************************************************func
# Purpose:
#   Write to a register
#
# Arguments:
#   $regname - register address or name.
#
# Return:
#   None
#*****************************************************************************
sub WriteRegister
{
    my $regname    = shift;
    my $regvalue   = shift;
    my $regnum     = atoi($regname);
    my $regobj     = undef;

    # Windows cannot process hex conversion of 64-bit values
    if (defined SoapClient::GetProxy() || ($^O =~ m/linux/i))
    {
        if(!defined atoi($regvalue))
        {
            printf("Error: <%s> isn't valid number.\n", $regvalue);
            return;
        }

        $regvalue = atoi($regvalue);
    }

    if(defined $regnum)
    {
        $regname = $regnum;
    }

    #
    # Lookup in register offset/name in RDB.  This could be inefficients if
    # the database is really huge.  We can cached this lookup.  For now
    # if a numeral I'm just going to write it.
    #
    if(!defined $g_rdb && defined $regnum)
    {
        my $Ret = _WriteRaw($regnum, $regvalue, 0);
        if(!defined $Ret)
        {
            printf("\tError: Failed to write to register 0x%08x.\n", $regnum);
        }
    }
    else
    {

        if($g_allowunknown && defined $regnum)
        {
            printf("Writing 0x%08x to 0x%08x register (no-lookup).\n", $regvalue, $regnum) if $g_verbose;
            my $Ret;

            $Ret = _WriteReg($regname, $regvalue);

            if(!defined $Ret)
            {
                printf("\tError: Failed to write to register offset 0x%08x.\n", $regnum);
            }
        }
        elsif($regobj = GetRegVar($regname))
        {
            my $longname = $regobj->ID;

            if (defined SoapClient::GetProxy() || ($^O =~ m/linux/i))
            {
                printf("Writing 0x%08x ", $regvalue) if $g_verbose;
            }
            else
            {
                printf("Writing 0x%s ", $regvalue) if $g_verbose;
            }

            printf(" to %s register.\n", $longname) if $g_verbose;

            $regobj->Set($regvalue);
            if(!defined $regobj->WriteHW($g_verbose))
            {
                printf("\tError: Failed writing to %s register.\n", $longname);
            }
        }
        else
        {
            printf("Error: <%s> isn't a valid register name or address.\n", $regname);
            if($g_allowunknown==0)
            {
                printf("Hint: Turn on the allowunknown option to write to undefined registers.\n");
                printf("      (regscope> o allowunknown 1)\n");
            }
        }
    }
}

#*************************************************************************func
# Purpose:
#   Write to a 64-bit register
#
# Arguments:
#   $regname - register address or name.
#
# Return:
#   None
#*****************************************************************************
sub WriteRegister64
{
    my $regname    = shift;
    my $regvalue   = shift;
    my $regnum     = atoi($regname);
    my $regobj     = undef;

    # Only apply if running in linux until perl is updated to > ver 20
    # Windows Perl 5.14.2 can't handle 64-bit. Math::BigInt mangles it.
    # Math::Int 64 isn't available in 5.14.2.
    if($^O =~ m/linux/i)
    {
        if(!defined atoi($regvalue))
        {
            printf("Error: <%s> isn't valid number.\n", $regvalue);
            return;
        }

        $regvalue = atoi($regvalue);
    }

    my $Ret = _WriteRaw($regnum, $regvalue, 1);
    if(!defined $Ret)
    {
        printf("\tError: Failed to write to register offset 0x%08x.\n", $regnum);
    }
}


#*************************************************************************func
# Purpose:
#   Set a specific field of register
#
# Arguments:
#   $regname - a register name or address
#   $fieldname - a register's field
#   $value     - a field value.
#
# Return:
#   None
#*****************************************************************************
sub SetRegisterField
{
    my $regname    = shift;
    my $Fieldname  = shift;
    my $Fieldvalue = shift;

    my $regnum     = atoi($regname);
    $regname       = $regnum if(defined $regnum);

    my $regobj = GetRegVar($regname);
    if(defined $regobj)
    {
        printf("Setting %s.%s field to %s.\n",
               $regobj->ID,
               $Fieldname,
               $Fieldvalue) if $g_verbose;
        if($Fieldname=~/\d+:\d+/)
        {
            my ($High, $Low) = split(":", $Fieldname);
            if(eval($High) < eval($Low))
            {
                my $Temp = $High;
                $High = $Low;
                $Low = $Temp;
            }

            ##
            # Make sure that value will is not overlow in given field.
            # Shiftting 32 bits will wrap around and max_field_value will eq to 0
            # which will failed the test condition.  If we see 32-bit field it will
            # not check.
            #
            my $field_value = atoi($Fieldvalue);
            my $field_width = eval($High-$Low) + 1;
            my $max_field_value = (1 << $field_width) - 1;
            if(($field_value > $max_field_value) && ($field_width != 32))
            {
              printf(STDERR "ERROR: Are you sure you want to assign $field_value ");
              printf(STDERR "to %s[%s] (%d-bit).\n", $regobj->ID, "$High:$Low", $field_width);
              return;
            }
            $regobj->ReadHW($g_verbose);
            my $data = $regobj->Get();
            if ($field_width < 32)
            {
              my $mask      = $max_field_value << $Low;
              $data = (($data) & ~$mask) | ($field_value << $Low);
            }
            elsif (($field_width == 32) && ($Low == 0))
            {
              $data = $field_value;
            }
            $regobj->Set($data);
            $regobj->WriteHW($g_verbose);
        }
        else
        {
            # Read | Modify | Write
            $regobj->ReadHW($g_verbose);
            (defined atoi($Fieldvalue))
                ? $regobj->SetField($Fieldname, atoi($Fieldvalue))
                : $regobj->SetField($Fieldname, $Fieldvalue);
            $regobj->WriteHW($g_verbose);
        }
    }
    else
    {
        printf("Error: <%s> isn't a valid register name or address.\n", $regname);
    }
}

#*************************************************************************func
# Purpose:
#   Store a set of registers for checkpointing
#
# Arguments:
#   $regname - starting register name or address
#   $count   - count of registers to checkpoint
#
# Return:
#   None
#*****************************************************************************
sub AddCheckpoint
{
    my $regname = shift;
    my $count   = shift;
    my $regobj;
    my $block;

    # name or offset
    $regobj = GetRegVar($regname);
    $block  = SearchBlock($regname);

    if(defined($regobj))
    {
        #printf("Adding register(s) to checkpoint: %s\n", $regobj->ID);
        AddCheckpoint_Registers($regobj->ID, $count);

    }
    elsif(defined($block))
    {
        printf("Adding (%s) registers to checkpoint.\n", $block->ID);
        AddBlockCheckPoint_Recursive($block);
    }
    else
    {
        printf("Error: <%s> isn't a valid register name or address.\n", $regname);
        return;
    }
}

#*************************************************************************func
# Purpose:
#   Store a set of registers for checkpointing
#
# Arguments:
#   $regname - starting register name or address
#   $count   - count of registers to checkpoint
#
# Return:
#   None
#*****************************************************************************
sub AddCheckpoint_Registers
{
    my $regname = shift;
    my $count   = shift || 1;
    my $regnum  = atoi($regname);
    my $regobj;

    if(!defined $regnum)
    {
        $regobj = GetRegVar($regname);
        if(!defined($regobj))
        {
            printf("Error: <%s> isn't a valid register name or address.\n", $regname);
            return;
        }
        $regnum = $regobj->BaseAddr + $regobj->Offset;
    }

    for(my $curreg = $regnum;
        $curreg<($regnum+4*$count);
        $curreg+=4)
    {
        $g_checkpoint{$curreg} = _ReadReg($curreg);
    }

    printf("Added %d registers starting at 0x%08x\n", $count, $regnum);
}

#*************************************************************************func
# Purpose:
#   Store a set of registers for checkpointing
#
# Arguments:
#   $regname - starting register name or address
#   $count   - count of registers to checkpoint
#
# Return:
#   None
#*****************************************************************************
sub RemoveCheckpoint
{
    my $regname    = shift;
    my $count      = shift;

    if(!defined $count)
    {
        $count = 1;
    }

    my $regnum = atoi($regname);

    if(!defined $regnum)
    {
        my $regobj = GetRegVar($regname);
        if(!defined($regobj))
        {
            printf("Error: <%s> isn't a valid register name or address.\n", $regname);
            return;
        }
        $regnum = $regobj->BaseAddr + $regobj->Offset;
    }

    for(my $curreg = $regnum;
        $curreg<($regnum+4*$count);
        $curreg+=4)
    {
        delete $g_checkpoint{$curreg} if($g_checkpoint{$curreg});
    }

    printf("Removed %d registers starting at 0x%08x\n", $count, $regnum);
}

#*************************************************************************func
# Purpose:
#   Update the checkpoint
#
# Arguments:
#   None
#
# Return:
#   None
#*****************************************************************************
sub UpdateCheckpoint
{
    my $count = 0;
    my $regObj;

    foreach (keys(%g_checkpoint))
    {
        $regObj = GetRegVar($_);
        $g_checkpoint{$_} = _ReadReg($_);
        $count++;
    }

    printf("Updated checkpoint (%d registers)\n", $count);
}

#*************************************************************************func
# Purpose:
#   Update the checkpoint
#
# Arguments:
#   None
#
# Return:
#   None
#*****************************************************************************
sub DeltaCheckpoint
{
    my @argv = @_;
    my $simple = 0;
    my $count = 0;
    my $diffcount = 0;
    my $regobj;

    if(defined($argv[0]))
    {
        if($argv[0]=~/^s/)
        {
            $simple = 1;
        }
    }

    foreach my $regaddr (keys(%g_checkpoint))
    {
        if(defined $g_checkpoint{$regaddr})
        {
            my $newval;
            $regobj = GetRegVar($regaddr);
            $count++;
            $newval = _ReadReg($regaddr);

            if($newval != $g_checkpoint{$regaddr})
            {
                $diffcount++;
                if($simple)
                {
                    printf("Reg 0x%08x: was 0x%08x   is 0x%08x\n", $regaddr,
                        $g_checkpoint{$regaddr}, $newval);
                }
                else
                {
                    if(defined $regobj)
                    {
                        $regobj->Set($g_checkpoint{$regaddr});
                        my $old = $regobj->Decode($regobj->Get());
                        $regobj->Set($newval);
                        my $new = $regobj->Decode($regobj->Get());

                        my @old = split /\n/, $old;
                        my @new = split /\n/, $new;

                        my $maxlen = 0;
                        for(my $i=0; $i<=$#old; $i++)
                        {
                            my $diff = 0;
                            if($old[$i] ne $new[$i])
                            {
                                $old[$i] = '*' . $old[$i];
                            }
                            else
                            {
                                $old[$i] = ' ' . $old[$i];
                            }
                            $old[$i] =~ s/\t/   /g;
                            if(length($old[$i]) > $maxlen)
                            {
                                $maxlen = length($old[$i]);
                            }

                            if($diff)
                            {
                                $old[$i] = '*' . $old[$i];
                            }
                        }

                        for(my $i=0; $i<=$#old; $i++)
                        {
                            my $tmp = $new[$i];
                            $tmp =~ s/[^=]+= //;
                            $old[$i] = sprintf("%-$maxlen" . "s -> $tmp", $old[$i]);
                        }
                        print join("\n", @old);
                        print "\n";
                    }
                    else
                    {
                        printf("Reg 0x%08x: was 0x%08x   is 0x%08x\n", $regaddr,
                            $g_checkpoint{$regaddr}, $newval);
                    }
                }
            }
        }
    }

    printf("\n%d different of %d registers checked\n", $diffcount, $count);
}

#*************************************************************************func
# Purpose:
#   Dumps out the checkpoint differences in script form.
#
# Arguments:
#   filename to dump to (optional)
#
# Return:
#   None
#*****************************************************************************
sub DeltaScriptCheckpoint
{
    my $filename = shift;
    my $doall = 0;
    my $SCRIPTFILE;

    if($filename=~m/all/i)
    {
        $filename = shift;
        $doall = 1;
    }

    if(defined($filename))
    {
        open $SCRIPTFILE, ">$filename";
        if(!defined($SCRIPTFILE))
        {
            print "Error: Unable to write to script file ($filename)\n";
            return;
        }
        select $SCRIPTFILE;
    }

    foreach my $regaddr (keys(%g_checkpoint))
    {
        if(defined $g_checkpoint{$regaddr})
        {
            my $regobj = GetRegVar($regaddr);
            my $newval;

            $newval = _ReadReg($regaddr);

            if($newval != $g_checkpoint{$regaddr} || $doall)
            {
                my $regobj = GetRegVar($regaddr);
                if(defined $regobj)
                {
                    my $longname = $regobj->ID;

                    print "//\n// $longname\n//\n";

                    $regobj->Set($g_checkpoint{$regaddr});
                    my $old = DecodeToScript($regobj->Decode($regobj->Get()));
                    $regobj->Set($newval);
                    my $new = DecodeToScript($regobj->Decode($regobj->Get()));

                    my @old = split /\n/, $old;
                    my @new = split /\n/, $new;

                    if($doall)
                    {
                        print $new;
                    }
                    else
                    {
                        my $diff = 3;
                        for(my $i=3; $i<=$#new; $i++)
                        {
                            if($old[$i] ne $new[$i])
                            {
                                print "$new[$i]\n";
                                $diff++;
                            }
                        }
                        if($diff <= $#new && $g_verbose)
                        {
                            print "// Unchanged fields:\n";
                            for(my $i=3; $i<=$#old; $i++)
                            {
                                if($old[$i] eq $new[$i])
                                {
                                    print "// $new[$i]\n";
                                }
                            }
                        }
                    }
                }
                else
                {
                    printf("// Warning: Undefined register 0x%08x\n", $regaddr);
                    printf("w h:%08x h:%08x\n", $regaddr, $newval);
                }
            }
        }
    }

    if(defined($filename))
    {
        close $SCRIPTFILE;
        select STDOUT;
    }
}

#*************************************************************************func
# Purpose:
#   Remove all checkpoint info
#
# Arguments:
#   None
#
# Return:
#   None
#*****************************************************************************
sub ClearCheckpoint
{
    %g_checkpoint = ();
}

#*****************************************************************************
# Helper functions
#*****************************************************************************
#*************************************************************************func
# Purpose:
#   Convert ascii string to integer
#
# Arguments:
#   $StrValue
#
# Return:
#   return the value from string.
#*****************************************************************************
sub atoi
{
    my %bases=( 'd' => 10, 'D' => 10, 'h' => 16, 'H' => 16, 'x' => 16, 'X' => 16, 'b' => 2, 'B' => 2 );
    my $StrValue = shift;

    my $base;
    my $retval = undef;

    if(defined $StrValue)
    {
        if(($base) = ($StrValue =~ m/^([dxb]):/i))
        {
            $base = $bases{$base};
            $StrValue = substr($StrValue, 2);
        }
        elsif($StrValue =~ s/^0x//i)
        {
            $base = 16;
        }
        else
        {
            $base = $g_base;
        }

        if($base == 16)
        {
            if($StrValue =~ m/^[a-f0-9_]+$/i)
            {
                $StrValue=~s/_//g;

                $retval = hex($StrValue);
            }
        }
        elsif($base == 10)
        {
            if($StrValue =~ m/^[0-9]+$/i)
            {
                $retval = $StrValue;
            }
        }
        elsif($base == 2)
        {
            if($StrValue =~ m/^[01]+$/i)
            {
                # This magic is from Perl Cookbook sec 2.4, pg 48
                $retval = unpack("N", pack("B32", substr("0" x 32 . $StrValue, -32)));
            }
        }
    }

    return $retval;
}

#*************************************************************************func
# Purpose:
#   Given a string from $regobj->Decode(), translate it into a regscope
#   script which will set the register.
#
# Arguments:
#   string to translate
#
# Return:
#   translated string
#*****************************************************************************
sub DecodeToScript
{
    my $decoded = shift;

    my $ret = '';

    my @decoded = split /\n/, $decoded;

    (my $longname) = $decoded[0]=~ m/\s*(\w+) \[/;
    $ret .= "//\n// $decoded[0]\n//\n";

    #
    # I do this in a very inefficient manner, but it works
    # and is very straightforward.
    #
    # Loop through and find the longest field so we can do
    # some pretty formating
    #
    my $maxlen = 0;
    for(my $i=1; $i<=$#decoded; $i++)
    {
        my ($field) =
            $decoded[$i] =~ m{\[.*\] ([^ ]+)\s*=};

        if(length($field) > $maxlen)
        {
            $maxlen = length($field);
        }
    }

    #
    # Now actually do the work
    #
    for(my $i=1; $i<=$#decoded; $i++)
    {
        #   [25:25] STRAP_PCI_SLAVE           = 0x0          // Off
        # OK, first we need the register name
        # w REG_NAME FIELD value
        my ($field, $value, $enum) =
            $decoded[$i] =~
                m{\[.*\] ([^ ]+)\s*=([^/]+)//\s+(.*)};

        if(!defined($enum))
        {
            $enum = $value;
        }

        if($enum =~ m/^\s*\d+\s*$/)
        {
            $enum = sprintf("x:%x", int $enum);
        }
        elsif($enum =~ /^\s*On\s*$/i)
        {
            $enum = 1;
        }
        elsif($enum =~ /^\s*Off\s*$/i)
        {
            $enum = 0;
        }

        if($field eq 'reserved')
        {
            $ret .= sprintf("//$longname %-$maxlen"."s $enum\n", $field) if ($g_verbose);
        }
        else
        {
            $ret .= sprintf("w $longname %-$maxlen"."s $enum\n", $field);
        }
    }

    return $ret;
}

#*************************************************************************func
# Purpose:
#   Print out program usage and help
# Arguments:
#   None
#
# Return:
#   None
#*****************************************************************************
sub PrintUsage
{
    print "NAME\n";
    print "\tRegScope - Device access testing.\n";
    print "\n\n";
    print "SYNOPSIS\n";
    print "\tperl RegScope.pl [-h] [-n] [-R RDBFile] [< script]\n";
    print "\t                 [-I Device:Port:SlaveAddr:Speed:UseCOM]\n";
    print "\t                 [-S Server's IP:Port]\n";
    print "\t                 [-E EJTAG IP or name]\n";
    print "DESCRIPTION\n";
    print "\tThis is a generic test tool for rd/wr registers and memory.\n\n";
    print "OPTIONS\n";
    print "\tThe following options are supported:\n";
    print "\t-h            This help screen.\n";
    print "\t-n            Doesn't load RDB file. All registers and memory are\n";
    print "\t              still accessible. Register reads will only display\n";
    print "\t              raw content. All accesses require numeric addresses.\n";
    print "\t              For block reads, be aware that addresses will be \n";
    print "\t              incremented by 4 which may not apply to 64-bit \n";
    print "\t              registers. No advance regscope features, eg., 'bvn' are\n";
    print "\t              available. This option is used when the intended RDB\n";
    print "\t              failed load.\n";
    print "\t-R RDBfile    RDB file that contain register field definitions.\n";
    print "\t              If this option is omit it will search for location\n";
    print "\t              specified by environment variable called RDBFILE.\n";
    print "\t-I D:P:A:S:U  Direct all reads/write to use the parallel port\n";
    print "\t              I2C adapter.\n";
    print "\t                (D)evice     The device on the other side of\n";
    print "\t                             the I2C. (i.e. \"7115\").\n";
    print "\t                (P)ort       The parallel port addr. (i.e. 378,\n";
    print "\t                             278, 3BC, or 2BC)\n";
    print "\t                Slave(A)ddr  The I2C slave address. (i.e. 9)\n";
    print "\t                (S)peed      The speed to run the parallel port\n";
    print "\t                             I2C protocol at. A value between \n";
    print "\t                             1-100 (use 50 as default)\n";
    print "\t                (U)seCOM     \"Y\" to use the BBS COM objects.\n";
    print "\n";
    print "EXAMPLES\n";
    print "\tSetting RDBFILE so we don\'t have to specify with -R each time:\n";
    print "\t\tset RDBFILE=q:\\Software\\src\\lib\\VenomLib\\bge_regs.rdb\n\n";
    print "\tSetting regscope to use the I2C parallel port card to talk to\n";
    print "\tthe 7115:\n";
    print "\t\tregscope -I 7115:378:9:50:Y\n\n";
    print "\tSetting regscope to communicate with regscope server:\n";
    print "\t\tregscope -S 10.21.12.87:9999\n\n";
    print "\tSetting regscope to communicate with an EJTAG attached to a board:\n";
    print "\t\tregscope -E 10.21.2.67\n";
    print "\t\tregscope -E refsw_sanjose_4\n\n";
    print "\tSetting regscope to communicate with an EJTAG attached to a board\n";
    print "\tbut don't load the RDB:\n";
    print "\t\tregscope -E refsw_sanjose_4 -n\n\n";

    if($g_CmdOption{r})
    {
        PrintRegisters();
    }
    print "SEE ALSO\n";
    print "\trdb.pl, RDBdecode.pl, PerlExtLib.pm\n\n";
    return;
}


#*************************************************************************func
#
#
sub DumpRegisters
{
    return if(!($#{$g_rdb->Chips} > -1));

    my $DUMPFILE;
    my $block = SearchBlock(shift);

    if(defined $block)
    {
        my $o_file = shift;

        if(defined $o_file)
        {
            open $DUMPFILE, ">$o_file" or die();
            if(!defined($DUMPFILE))
            {
                print "Error: Unable to open output file ($o_file)\n";
                return;
            }
            else
            {
                print "Dumping registers to ($o_file)\n";
            }
            select $DUMPFILE;
        }
        else
        {
            printf "Available Registers\n";
        }

        printf "\n\n%-30s ( 0x%08x )\n", $block->HierID, $block->StartAddr;
        DumpBlock_Recursive($block);

        if(defined $DUMPFILE)
        {
            select STDOUT;
            close $DUMPFILE;
        }
    }
    return;
}


#*************************************************************************func
#
#
sub SearchBlock_recursive
{
    my $block = shift;
    my $query = shift;

    if(lc $block->ID =~ m/^$query$/i)
    {
        #printf ("found: ********[%s]*******, is_block = %d\n", $block->ID, $block->IsBlock);
        return $block;
    }

    if($block->IsA("RDBBlock"))
    {
        #printf ("matching? %s == %s, is_block = %d, num =%d\n",
            #$query, $block->ID, $block->IsBlock, $block->NumBlocksAndRegsets);

        foreach (@{$block->BlocksAndRegsets})
        {
            my $regset = SearchBlock_recursive($_, $query);
            if(defined $regset)
            {
                return $regset;
            }
        }
    }
    else
    {
        #printf ("matching? %s == %s, is_block = %d\n",
            #$query, $block->ID, $block->IsBlock);
    }
    return undef;
}

#*************************************************************************func
#
#
sub SearchBlock
{
    my $query = shift;

    return undef if (!($#{$g_rdb->Chips} > -1));

    my $Chip = $g_rdb->Chips->[0];

    foreach (@{$Chip->BlocksAndRegsets})
    {
        my $block = SearchBlock_recursive($_, $query);
        if(defined $block)
        {
            return $block;
        }
    }
    return undef;
}

#*************************************************************************func
#
#
sub DumpBlock_Recursive
{
    my $block = shift;

    if($block->IsA("RDBBlock"))
    {
        foreach (@{$block->BlocksAndRegsets})
        {
            DumpBlock_Recursive($_);
        }
    }
    else
    {
        DumpBlock($block);
    }

    return;
}

#*************************************************************************func
#
#
sub DumpBlock
{
    my $block = shift;

    foreach (@{$block->Regset->Registers})
    {
        my $regobj = GetRegVar($block->ID . "_" . $_->ID);
        $regobj->ReadHW();
        printf "%s\n", $regobj->Decode($regobj->Get(), $g_verbose);
    }

    return;
}


#*************************************************************************func
#
#
sub AddBlockCheckPoint_Recursive
{
    my $block = shift;

    if($block->IsA("RDBBlock"))
    {
        foreach (@{$block->BlocksAndRegsets})
        {
            AddBlockCheckPoint_Recursive($_);
        }
    }
    else
    {
        AddBlockCheckPoint($block);
    }

    return;
}

#*************************************************************************func
#
#
sub AddBlockCheckPoint
{
    my $block = shift;

    foreach (@{$block->Regset->Registers})
    {
        my $regobj    = GetRegVar($block->ID . "_" . $_->ID);
        my $curreg = $regobj->Offset + $block->StartAddr;

        printf("\t%-35s (%s) %s\n",
            ($regobj->ID),
            ($regobj->Reg->{access}),
            ($regobj->Reg->{access} ne "RW") ? "skipped" : "");

        #adding to check hash!
        $g_checkpoint{$curreg} = _ReadReg($curreg)
                if($regobj->Reg->{access} eq "RW");
    }

    return;
}


#*************************************************************************func
# Purpose:
#   Print out all available register read in from RDBs.
#
# Arguments:
#   None
#
# Return:
#   None
#*****************************************************************************
sub PrintRegisters
{
    my $QueryCore = shift;

    print "Available Registers\n";
    if($#{$g_rdb->Chips} > -1)
    {
        foreach my $Chip (@{$g_rdb->Chips})
        {
            foreach my $RegsetInst (@{$Chip->RegsetInsts})
            {
                my $iItem = 0;
                if($QueryCore && (lc $RegsetInst->ID =~ m/^$QueryCore/i))
                {
                    printf "\n\n%-30s ( 0x%08x )\n", $Chip->ID ." - ". $RegsetInst->ID, $RegsetInst->StartAddr;
                    foreach my $Reg (@{$RegsetInst->Regset->Registers})
                    {
                        printf("\t%-30s", $RegsetInst->ID . "_" . $Reg->ID);
                        print "\n" unless !(++$iItem % 3 == 0);
                    }
                    goto done;
                }
                elsif(!defined $QueryCore)
                {
                    printf "\n\n%-30s ( 0x%08x )\n", $Chip->ID ." -- ". $RegsetInst->ID, $RegsetInst->StartAddr;
                    foreach my $Reg (@{$RegsetInst->Regset->Registers})
                    {
                        printf("\t%-30s", $RegsetInst->ID . "_" . $Reg->ID);
                        print "\n" unless !(++$iItem % 3 == 0);
                    }
                }
            }
        }
    }
    elsif($#{$g_rdb->Cores} > -1)
    {
        foreach my $Core (@{$g_rdb->Cores})
        {
            foreach my $Regset (@{$Core->Regsets})
            {
                my $iItem = 0;
                if($QueryCore && (lc $Regset->ID =~ m/^$QueryCore/i))
                {
                    printf "\n\n%-30s\n", $Core->ID ." - ". $Regset->ID;
                    foreach my $Reg (@{$Regset->Registers})
                    {
                        printf("\t%-30s", $Core->ID . "_" . $Reg->ID);
                        print "\n" unless !(++$iItem % 3 == 0);
                    }
                    goto done;
                }
                elsif(!defined $QueryCore)
                {
                    printf "\n\n%-30s\n", $Core->ID ." -- ". $Regset->ID;
                    foreach my $Reg (@{$Regset->Registers})
                    {
                        printf("\t%-30s", $Core->ID . "_" . $Reg->ID);
                        print "\n" unless !(++$iItem % 3 == 0);
                    }
                }
            }
        }
    }
    else
    {
        printf "\tWarning: No registers found.\n";
        printf "\tYou need to specify RDB file(s) by using the -R option or\n";
        printf "\tsetting environment variable RDBFILE.\n";
    }

done:
    print "\n";
}

#*************************************************************************func
# Purpose:
#   Print out help and program usage.
#
# Arguments:
#   None
#
# Return:
#   None
#*****************************************************************************
sub PrintHelp
{
    print <<'EOINPUT';

Regscope Command Parameters:
    <register> is either a <regname> or a register <offset>.

    <regname> is a string which matches one of the registers defined in an
       RDB file. You can set the RDB file with the -R command line option or
       by setting the RDBFILE environment variable.

    <offset>, <address>, and <value> are numeric and are parsed according
       to the current base setting. (The base can be set with "o base")
       The current base setting can be overridden by prefixing the value with
       "d:" for decimal, "x:" for hex, or "b:" for binary.

Watches:
    wa  <register>                      Add a register to the Watch set
    wd                                  Disable the Watch list
    we                                  Enable the Watch list
    wr  <register>                      Remove a register to the Watch set
    wr  foo*                            Remove Watches starting with "foo"

Miscellaneous:
    b   [16|10|2]                       set the default numeric Base
    h                                   this Help
    ir  <chip_addr> <sub_addr>          Reads 8 bit word at i2c device address
    iw  <chip_addr> <sub_addr> <value>  Write 8 bit value to i2c device address
    l   <rdbfilename>                   Loads given RDB file
    n   [core]                          prints out all [core's] register Names
    o                                   shows current Option settings
    o   <option> <value>                sets Option to value
    q                                   Quit

Checkpoints:
    cpa[dd] <register> <count>          Add a range of registers to the
                                        checkpoint set.
    cpc[lear]                           Clear out all registers from the
                                        checkpoint set.
    cpd[iff] [s[imple]]                 Compare the checkpoint with the
                                        current register values.
    cpr[emove] <register> <count>       Remove a range of registers in the
                                        checkpoint set.
    cps[cript] [all] [<filename>]       generate a Script which will program
                                        the changed fields (or [all] fields)
                                        in the checkpoint.
    cpu[pdate]                          Update the checkpoint with the current
                                        register values.

Memory:
    decode <register> <value>           Decode a register value into fields
    mr  <address>                       Reads 32 bit word at device address
    mr  <address> <count>               Reads count words at device address
    mw  <address> <value>               Writes 32 bit value to device address
    mw  <address> <value> <count>       Writes fill device addresses with value
    zmr8  <address>                     Reads 8 bit byte at device address
    lzmr8 <address>                     Reads 8 bit byte at device address in little-endian mode
    zmw8  <address> <value>             Writes 8 bit byte to device address
    lzmw8 <address> <value>             Writes 8 bit byte to device address in little-endian mode
    mwblk <address> <filename>          Writes file to device addresses with
                                        contents in filename.
    mrblk <address> <count> <filename>  Reads count words at device address and
                                        store in filename.
    mwblk2 <address> <filename>         Writes file to device addresses with
                                        contents in filename (MemView format).
    mrblk2 <address> <count> <filename> Reads count words at device address and
                                        store in filename (MemView format).

Scripts:
    s  <script filename> [s] [f]        runs given Script file ([s]ilently)
                                        If the filename ends in .bsf, then
                                        the script is assumed to be BBS VB
                                        and is converted. If the filename
                                        ends in .tcs, then the script is
                                        assumed to be a TCS script and ran
                                        as such. If [f]ields is specified,
                                        then each field is set individually.
                                        Otherwise the whole whole register
                                        is set at once.
    tcs <script> [regoffset]            Runs the TCS script <script>. If
                                        [regoffset] is specified, all register
                                        values in the script are offset by that
                                        value.
    cvt <in script> <out script> [f]    Converts BBS VB script <in script>
                                        to a Regscope script <out script>
                                        If [f]ields is specified, then each
                                        field is set individually. Otherwise
                                        the whole whole register is set at
                                        once.


Registers:
    r  <register> [count] [filename]    Reads count registers starting at
                                        register and dump to filename.
    r64 <register> [count] [filename]   Reads count 64-bit registers starting at
                                        register and dump to filename. Only used
                                        when -n (no loaded RDB) option is
                                        specified during program invocation.
    rs <register> [count] [filename]    Reads count registers starting at
                                        register and dump to filename in
                                        script form.
    w  <register> <value>               Writes value into register
    w64  <register> <value>             Writes value into 64-bit register. Only
                                        used when -n (no loaded RDB) option is
                                        specified during program invocation.
    w  <regname> <field> <value>        Writes value into a specific field

EOINPUT
    print "Loaded RDB files:\n";
    for (@g_RDBFiles)
    {
        print "\t$_\n";
    }

    printf "\n";
}


#*************************************************************************func
# Purpose:
#   Remove register from watch list.
#
# Arguments:
#   $Register - a register name or address.
#
# Return:
#   None
#*****************************************************************************
sub AddWatch
{
    my $regname  = shift;
    my $regobj   = GetRegVar($regname);

    if(defined $regobj)
    {
        push @g_watchlist, $regobj->ID;
    }
    elsif($g_allowunknown && defined atoi($regname))
    {
        push @g_watchlist, sprintf("0x%08x", atoi($regname));
    }
    else
    {
        printf("Error: <%s> isn't a valid register name or address.\n", $regname);
        if($g_allowunknown==0)
        {
            printf("Hint: Turn on the allowunknown option to write to undefined registers.\n");
            printf("      (regscope> o allowunknown 1)\n");
        }
    }
}

#*************************************************************************func
# Purpose:
#   Remove register from watch list.
#
# Arguments:
#   None
#
# Return:
#   None
#*****************************************************************************
sub RemoveWatch
{
    my $pattern = "^" . shift;
    $pattern =~ s/\*/.*/g;

    @g_watchlist = grep(!/$pattern/i, @g_watchlist);
}

#*************************************************************************func
# Purpose:
#   Print out register in watch list before every command.
#
# Arguments:
#   None!
#
# Return:
#  None!
#*****************************************************************************
sub PrintWatch
{
    foreach my $regname (@g_watchlist)
    {
        my $val      = undef;
        my $name     = $regname;
        my $regobj   = GetRegVar($regname);

        if(defined $regobj)
        {
            $name = $regobj->ID;
            if(defined $regobj->ReadHW($g_verbose))
            {
                $val = $regobj->Get();
            }
        }
        elsif($g_allowunknown && defined atoi($regname))
        {
            my $localreg = GetRegVar(atoi($regname));

            $val = _ReadReg(atoi($regname));
        }

        my $longname = $regobj->ID;
        if(defined $val)
        {
            printf("\t%-25s = 0x%08x\n", $longname, $val);
        }
        else
        {
            printf("\t%-25s = <unknown>\n", $longname);
        }
    }
}

#*************************************************************************func
# Purpose:
#   Decode a register, value supply from user instead of hardware.
#
# Arguments:
#   $RegisterName  - regiser name or address
#   $Value         - value to be decode with this register.
#
# Return:
#   None!  Just output result to stdout.
#*****************************************************************************
sub DecodeRegister
{
    my $regname    = shift;
    my $regvalue   = shift;
    my $regobj     = undef;

    #
    # User Error!  Bail out if we can't even get a register name!
    #
    if((!defined $regname) || (!defined $regvalue))
    {
        return PrintHelp();
    }

    #
    # Is is an address, Convert regvalue to number.
    #
    $regvalue = atoi($regvalue);
    $regname  = atoi($regname) if(defined atoi($regname));

    #
    # Initialize the register!
    #
    $regobj = GetRegVar( $regname);
    $regobj = $g_rdb->MakeTypeVar($regname) if(!defined $regobj);

    #
    # If find it decode out the values.
    #
    if(defined $regobj)
    {
        $regobj->Set($regvalue);
        printf "%s\n", $regobj->Decode($regobj->Get(), $g_verbose);
        return;
    }
    else
    {
        printf("Error: <%s> isn't a valid register name or address.\n\n", $regname);
    }
}

#*************************************************************************func
# Purpose:
#   Decode a register, value supply from user instead of hardware.
#
# Arguments:
#   $RegisterName  - regiser name or address
#   $Value         - value to be decode with this register.
#
# Return:
#   None!  Just output result to stdout.
#*****************************************************************************
sub DecodeRegisterToString
{
    my $regname    = shift;
    my $regvalue   = shift;
    my $regobj     = undef;

    my $ret;

    #
    # Is is an address, Convert regvalue to number.
    #
    $regvalue = atoi($regvalue);
    $regname  = atoi($regname) if(defined atoi($regname));

    #
    # Initialize the register!
    #
    $regobj = GetRegVar( $regname);
    $regobj = $g_rdb->MakeTypeVar($regname) if(!defined $regobj);

    #
    # If find it decode out the values.
    #
    if(defined $regobj)
    {
        $regobj->Set($regvalue);
        $ret =  $regobj->Decode($regobj->Get(), $g_verbose);
    }
    else
    {
        $ret = "Error: <$regname> isn't a valid register name or address.\n\n";
    }

    return $ret;
}


#*************************************************************************func
# Purpose:
#   Read from an i2c device
#
# Arguments:
#   $ChipAddr  - an i2c device's address.  This is usually given in the
#     data sheet of the device.
#
#   $SubAddr   - an i2c device sub address
#
# Return:
#   None!  Just output result to stdout.
#*****************************************************************************
sub ReadI2C
{
    my $ChipAddr = atoi(shift);
    my $SubAddr  = atoi(shift);
    my $Result   = undef;

    if( (defined $ChipAddr) &&
        (defined $SubAddr) )
    {
        $Result = PerlExtLib::bcmReadI2C($ChipAddr, $SubAddr);
        printf("i2c_device[0x%x][0x%x] = 0x%x\n",
               $ChipAddr, $SubAddr, $Result) if defined $Result;
    }
}


#*************************************************************************func
# Purpose:
#   write to an i2c device
#
# Arguments:
#   $ChipAddr  - an i2c device's address.  This is usually given in the
#     data sheet of the device.
#
#   $SubAddr   - an i2c device sub address
#
#   $Data      - data byte that will go to the device's sub address.
#
# Return:
#   None!  Just output result to stdout.
#*****************************************************************************
sub WriteI2C
{
    my $ChipAddr = atoi(shift);
    my $SubAddr  = atoi(shift);
    my $Data     = atoi(shift);
    my $Result   = undef;

    if( (defined $ChipAddr) &&
        (defined $SubAddr) &&
        (defined $Data) )
    {
        $Result = PerlExtLib::bcmWriteI2C($ChipAddr, $SubAddr, $Data);
    }
}

#*************************************************************************func
# Purpose:
#   Decode given value register/descriptor
#
# Arguments:
#   $regname
#     a register name or one of the of the descriptor tag.
#
#   $regvalue
#     a register value or the descriptor word0 address
#
# Return:
#   Decoded fields of the value.
#
#*****************************************************************************
sub EnhancedDecode
{
    my $regname    = shift;
    my $regvalue   = shift;
    my $DescAddr   = $regvalue;
    my ($chipdef, $potentialrdbfile) = GuessChipRDB("hush");

    if(lc($regname) eq "vdec_ft")
    {
        my $DescWord0 = GetRegVar("FD_0_MF_FORMAT_ENTRY_00");
        my $DescWord1 = GetRegVar("FD_0_MF_FORMAT_ENTRY_01");

        if((defined $DescWord0) && (defined $DescWord1))
        {
            $DescAddr = atoi($DescAddr);

            my $Word0 = _ReadMem($DescAddr + 0x0);
            my $Word1 = _ReadMem($DescAddr + 0x4);

            printf "%s\n", $DescWord0->Decode($Word0);
            printf "%s\n", $DescWord1->Decode($Word1);
            printf "\n";
        }
    }
    else
    {
        return DecodeRegister($regname, $regvalue);
    }
}

#*************************************************************************func
#
# Print the BVN errors!
#
sub DoPrintBvnErrors
{
    my $clear = shift;

    if($clear)
    {
        my $f_clear = GetRegVar("BVNF_INTR2_5_PCI_CLEAR");
        if(defined $f_clear)
        {
            $f_clear->Set(0xffffffff);
            $f_clear->WriteHW($g_verbose);
        }

        my $m_clear = GetRegVar("BVNM_INTR2_0_PCI_CLEAR");
        if(defined $m_clear)
        {
            $m_clear->Set(0xffffffff);
            $m_clear->WriteHW($g_verbose);
        }

        my $b_clear = GetRegVar("BVNB_INTR2_PCI_CLEAR");
        if(defined $b_clear)
        {
            $b_clear->Set(0xffffffff);
            $b_clear->WriteHW($g_verbose);
        }
    }

    my $f_status = GetRegVar("BVNF_INTR2_5_PCI_STATUS");
    if(defined $f_status)
    {
        $f_status->ReadHW($g_verbose);
        printf("%s\n", $f_status->Decode());
    }

    my $m_status = GetRegVar("BVNM_INTR2_0_PCI_STATUS");
    if(defined $m_status)
    {
        $m_status->ReadHW($g_verbose);
        printf("%s\n", $m_status->Decode());
    }

    my $b_status = GetRegVar("BVNB_INTR2_PCI_STATUS");
    if(defined $b_status)
    {
        $b_status->ReadHW($g_verbose);
        printf("%s\n", $b_status->Decode());
    }

    return;
}

#*************************************************************************func
#
# Print the VEC routing!
#
sub DoPrintVecRouting
{
    my $chip = $g_rdb->Chips->[0];
    my @vec_mux_regs = ();
    my @dac_mux_regs = ();

    # Add terminal nodes to start searching upstreams
    foreach my $regset_inst (@{$chip->RegsetInsts})
    {
        # done? added
        next if(($#vec_mux_regs > -1) && ($#dac_mux_regs > -1));

        if(($regset_inst->ID =~ m/vec_cfg/i) && ($#vec_mux_regs == -1))
        {
            # find vec stg/656/dvi muxes
            foreach my $reg_inst (@{$regset_inst->Regset->Registers})
            {
                my $reg = sprintf("%s", $regset_inst->ID . "_" . $reg_inst->ID);
                if(($reg =~ /vec_cfg_stg_[\d]_source$/i) ||
                   ($reg =~ /vec_cfg_dvi_dtg_[\d]_source$/i) ||
                   ($reg =~ /vec_cfg_itu656_dtg_[\d]_source$/i))
                {
                    #printf("adding vec mux: %s\n", $reg);
                    push(@vec_mux_regs, $reg);
                }
            }
        }
        elsif(($regset_inst->ID =~ m/misc/i) && ($#dac_mux_regs == -1))
        {
            # find dac muxes
            foreach my $reg_inst (@{$regset_inst->Regset->Registers})
            {
                my $reg = sprintf("%s", $regset_inst->ID . "_" . $reg_inst->ID);
                if(($reg =~ /misc_dac_[\d]_cfg$/i) ||
                   ($reg =~ /misc_rfm_[\d]_cfg$/i))
                {
                    #printf("adding dac mux: %s\n", $reg);
                    push(@dac_mux_regs, $reg);
                }
            }
        }
    }

    # For each of the registers in the vec muxes (ternminal node) traverse
    # up until we hit the entry node.
    foreach my $regname (@vec_mux_regs)
    {
        if($regname =~ m/vec_cfg_stg_([\d])_source$/i)
        {
            _DoTerminalPrint($regname, $1, "STG");
        }
        elsif($regname =~ m/vec_cfg_dvi_dtg_([\d])_source$/i)
        {
            _DoTerminalPrint($regname, $1, "HDM");
        }
        elsif($regname =~ m/vec_cfg_itu656_dtg_([\d])_source$/i)
        {
            _DoTerminalPrint($regname, $1, "656");
        }
        else
        {
            printf("Not expected terminal node: %s\n", $regname);
        }
    }

    # For each of the registers in the dac muxes (ternminal node) traverse
    # up until we hit the entry node.
    foreach my $regname (@dac_mux_regs)
    {
        if($regname =~ m/misc_dac_([\d])_cfg$/i)
        {
            _DoDacPrint($regname, $1, "DAC");
        }
        elsif($regname =~ m/misc_rfm_([\d])_cfg$/i)
        {
            _DoDacPrint($regname, $1, "RFM");
        }
        else
        {
            printf("Not expected terminal node: %s\n", $regname);
        }
    }

    return;
}

sub _DoTerminalPrint
{
    my $reg = GetRegVar(shift);
    my $ins = shift;
    my $nam = shift;
    my @decoded_lines = split("\n", $reg->Decode($reg->ReadHW));

    foreach my $line (@decoded_lines)
    {
        if($line =~ m/.*\[.*\].*source/i)
        {
            my ($bits, $field_name, $equal_sign, $field_value,
                $cpp_comment, $value_name) = split(" ", $line);
            #printf("%s = %d, %s\n", $reg->ID, hex($field_value), $value_name);
            if($value_name =~ m/DECIM_([\d]).*$/i)
            {
                _DoDecimPrint("VEC_CFG_DECIM_$1_SOURCE", $1, $value_name);
                printf("%s_%d\n", $nam, $ins);
            }
            elsif($value_name ne "DISABLE")
            {
                $value_name =~ s/S_/CMP_/g;
                printf("%s -> %s_%d\n", $value_name, $nam, $ins);
            }
        }
    }

    return;
}

sub _DoDacPrint
{
    my $reg = GetRegVar(shift);
    my $ins = shift;
    my $nam = shift;
    my @decoded_lines = split("\n", $reg->Decode($reg->ReadHW));

    foreach my $line (@decoded_lines)
    {
        if($line =~ m/.*\[.*\].*sel/i)
        {
            my ($bits, $field_name, $equal_sign, $field_value,
                $cpp_comment, $value_name) = split(" ", $line);
            #printf("%s = %d, %s\n", $reg->ID, hex($field_value), $value_name);
            if($value_name =~ m/SDSRC_([\d]).*$/i)
            {
                _DoSrcPrint("VEC_CFG_SDSRC_$1_SOURCE", $1, $value_name);
                printf("%s_%d\n", $nam, $ins);
            }
            elsif($value_name =~ m/HDSRC_([\d]).*$/i)
            {
                _DoSrcPrint("VEC_CFG_HDSRC_$1_SOURCE", $1, $value_name);
                printf("%s_%d\n", $nam, $ins);
            }
            elsif($value_name ne "CONST")
            {
                printf("Not expected value for %s = %s\n", $reg->ID, $value_name);
            }
        }
    }

    return;
}

sub _DoSrcPrint
{
    my $reg = GetRegVar(shift);
    my $ins = shift;
    my $nam = shift;
    my @decoded_lines = split("\n", $reg->Decode($reg->ReadHW));

    foreach my $line (@decoded_lines)
    {
        if($line =~ m/.*\[.*\].*source/i)
        {
            my ($bits, $field_name, $equal_sign, $field_value,
                $cpp_comment, $value_name) = split(" ", $line);
            #printf("%s = %d, %s\n", $reg->ID, hex($field_value), $value_name);
            if($value_name =~ m/SSP_([\d]).*$/i)
            {
                _DoSppPrint("VEC_CFG_SECAM_$1_SOURCE", $1, $value_name);
                printf("%s -> ", $nam);
            }
            elsif($value_name ne "DISABLE")
            {
                printf("Not expected value for %s = %s\n", $reg->ID, $value_name);
            }
        }
    }

    return;
}

sub _DoSppPrint
{
    my $reg = GetRegVar(shift);
    my $ins = shift;
    my $nam = shift;
    my @decoded_lines = split("\n", $reg->Decode($reg->ReadHW));

    foreach my $line (@decoded_lines)
    {
        if($line =~ m/.*\[.*\].*source/i)
        {
            my ($bits, $field_name, $equal_sign, $field_value,
                $cpp_comment, $value_name) = split(" ", $line);
            #printf("%s = %d, %s\n", $reg->ID, hex($field_value), $value_name);
            if($value_name =~ m/VF_([\d]).*$/i)
            {
                _DoVfPrint("VEC_CFG_VF_$1_SOURCE", $1, $value_name);
                printf("%s -> ", $nam);
            }
            elsif($value_name ne "DISABLE")
            {
                printf("Not expected value for %s = %s\n", $reg->ID, $value_name);
            }
        }
    }

    return;
}

sub _DoVfPrint
{
    my $reg = GetRegVar(shift);
    my $ins = shift;
    my $nam = shift;
    my @decoded_lines = split("\n", $reg->Decode($reg->ReadHW));

    foreach my $line (@decoded_lines)
    {
        if($line =~ m/.*\[.*\].*source/i)
        {
            my ($bits, $field_name, $equal_sign, $field_value,
                $cpp_comment, $value_name) = split(" ", $line);
            #printf("%s = %d, %s\n", $reg->ID, hex($field_value), $value_name);
            if($value_name =~ m/IT_([\d]).*$/i)
            {
                _DoItPrint("VEC_CFG_IT_$1_SOURCE", $1, $value_name);
                printf("%s -> ", $nam);
            }
            elsif($value_name ne "DISABLE")
            {
                printf("Not expected value for %s = %s\n", $reg->ID, $value_name);
            }
        }
    }

    return;
}

sub _DoItPrint
{
    my $reg = GetRegVar(shift);
    my $ins = shift;
    my $nam = shift;
    my @decoded_lines = split("\n", $reg->Decode($reg->ReadHW));

    foreach my $line (@decoded_lines)
    {
        if($line =~ m/.*\[.*\].*source/i)
        {
            my ($bits, $field_name, $equal_sign, $field_value,
                $cpp_comment, $value_name) = split(" ", $line);
            #printf("%s = %d, %s\n", $reg->ID, hex($field_value), $value_name);
            if($value_name =~ m/S_([\d]).*$/i)
            {
                $value_name =~ s/S_/CMP_/g;
                printf("%s -> %s -> ", $value_name, $nam);
            }
            elsif($value_name =~ m/DECIM_([\d]).*$/i)
            {
                _DoDecimPrint("VEC_CFG_DECIM_$1_SOURCE", $1, $value_name);
                printf("%s -> ", $nam);
            }
            elsif($value_name =~ m/TPG_([\d]).*$/i)
            {
                printf("%s -> %s -> ", $value_name, $nam);
            }
            elsif($value_name ne "DISABLE")
            {
                printf("Not expected value for %s = %s\n", $reg->ID, $value_name);
            }
        }
    }

    return;
}

sub _DoDecimPrint
{
    my $reg = GetRegVar(shift);
    my $ins = shift;
    my $nam = shift;
    my @decoded_lines = split("\n", $reg->Decode($reg->ReadHW));

    foreach my $line (@decoded_lines)
    {
        if($line =~ m/.*\[.*\].*source/i)
        {
            my ($bits, $field_name, $equal_sign, $field_value,
                $cpp_comment, $value_name) = split(" ", $line);
            #printf("%s = %d, %s\n", $reg->ID, hex($field_value), $value_name);
            if($value_name =~ m/S_([\d]).*$/i)
            {
                $value_name =~ s/S_/CMP_/g;
                printf("%s -> %s -> ", $value_name, $nam);
            }
            elsif($value_name ne "DISABLE")
            {
                printf("Not expected value for %s = %s\n", $reg->ID, $value_name);
            }
        }
    }

    return;
}


#*************************************************************************func
#
# Print the BVN routing!
#
sub DoPrintBvnRouting
{
    my $chip        = $g_rdb->Chips->[0];
    my @vnet_nodes  = ();
    my $vnet_f_done = 0;
    my $vnet_b_done = 0;

    # Look up all the terminal nodes
    foreach my $regset_inst (@{$chip->RegsetInsts})
    {
        # done?
        next if($vnet_f_done && $vnet_b_done);

        if(($regset_inst->ID =~ m/vnet_f/i) && (!$vnet_f_done))
        {
            # find vnet front
            foreach my $reg_inst (@{$regset_inst->Regset->Registers})
            {
                my $reg = sprintf("%s", $regset_inst->ID . "_" . $reg_inst->ID);
                if(($reg =~ /_crc.*_src$/i)  ||
                   ($reg =~ /_lbox.*_src$/i) ||
                   ($reg =~ /_drain.*_src$/i))
                {
                    #printf("adding.... %s\n", $reg);
                    push(@vnet_nodes, $reg)
                }
                $vnet_f_done = 1;
            }
        }
        elsif(($regset_inst->ID =~ m/vnet_b/i) && (!$vnet_b_done))
        {
            # find vnet back
            foreach my $reg_inst (@{$regset_inst->Regset->Registers})
            {
                my $reg = sprintf("%s", $regset_inst->ID . "_" . $reg_inst->ID);
                if(($reg =~ /_cmp.*_src$/i)   ||
                   ($reg =~ /_bp.*_src$/i)    ||
                   ($reg =~ /_crc.*_src$/i)   ||
                   ($reg =~ /_drain.*_src$/i) ||
                   ($reg =~ /_histogram.*_src$/i))
                {
                    #printf("adding.... %s\n", $reg);
                    push(@vnet_nodes, $reg)
                }
                $vnet_b_done = 1;
            }
        }
    }

    # For each of the registers in the Vnet_b (ternminal node) recursively
    # traverse until we hit the entry node.
    foreach my $regname (@vnet_nodes)
    {
        my $reg = GetRegVar($regname);
        my $val = $reg->ReadHW();

        if($val != 0xf)
        {
            _P_PrintVnet($reg->ID);
            my ($vnet, $vnet_reg) = split(",", $vnet_hash{$reg->ID});
            #printf "%s (%d)", $vnet, $val;
            printf "%s\n", $vnet, $val;
        }
    }
    printf("\n");

    return;
}

sub _ReadRaw
{
    my $addr = shift;
    my $res;
    my $is64bit = shift;
    my $ret;

    if (defined SoapClient::GetProxy())
    {
        $ret = ($is64bit) ? SoapClient::ReadReg($addr, 8) : SoapClient::ReadReg($addr, 4);
    }
    else
    {
        $ret = ($is64bit) ? PerlExtLib::bcmRead64($addr) : PerlExtLib::bcmRead($addr);
    }

    if (defined $ret)
    {
        if ($is64bit)
        {
            if($^O =~ m/Win32/i) # WIN32 Perl 5.14.2 can't handle 64-bit. Math::BigInt mangles it.
                                 #       Math::Int 64 isn't available in 5.14.2
            {
                $res = sprintf("[0x%08x] = %s\n",
                               $addr,
                               $ret);
            }
            else
            {
                 $res = sprintf("[0x%08x] = 0x%llx (%lld)\n",
                               $addr,
                               $ret,
                               $ret);
            }
        }
        else
        {
            $res = sprintf("[0x%08x] = 0x%08x (%d)\n",
                               $addr,
                               $ret,
                               $ret);
        }
    }
    else
    {
        $res = undef;
    }

    return $res;
}

sub _WriteRaw
{
    my $addr = shift;
    my $data = shift;
    my $is64bit = shift;
    my $res;

    if (defined SoapClient::GetProxy())
    {
        $res = ($is64bit) ? SoapClient::WriteReg($addr, $data, 8) : SoapClient::WriteReg($addr, $data, 4);
    }
    else
    {
        $res = ($is64bit) ? PerlExtLib::bcmWrite64($addr, $data) : PerlExtLib::bcmWrite($addr, $data);
    }

    return $res;
}

##############################################################################
#
#
#
sub _P_PrintVnet
{
    my $regname = shift;
    my $reg = GetRegVar($regname);

    if(!defined($reg))
    {
        printf "%s not found!\n", $regname;
        return;
    }

    # read hardware value!
    my $vnet = undef, my $vnet_reg = undef;
    my $value = $reg->ReadHW();
    $reg->Set($value);
    $value = $reg->GetField("SOURCE");

    foreach my $field (@{$reg->Type->Fields})
    {
        my $valuename = $field->FindValue($value);
        if(defined($valuename))
        {
            if($vnet_hash{$valuename->ID})
            {
                ($vnet, $vnet_reg) = split(",", $vnet_hash{$valuename->ID});
                last;
            }
        }
    }

    if($vnet_reg ne "TERMINAL_NODE")
    {
        _P_PrintVnet($vnet_reg);
        #printf("%s (%d) -> ", $vnet, $value);
        printf("%s -> ", $vnet, $value);
    }
    else
    {
        #printf("%s (%d) -> ", $vnet, $value);
        printf("%s -> ", $vnet, $value);
    }

    return;
}

#*************************************************************************func
# Purpose:
#   Reboot box from IP_Server/I2C_Port/NT_DD/etc.
#
#*****************************************************************************
sub Reboot
{
    my $reset_sw   = GetRegVar("SUN_TOP_CTRL_SW_RESET");
    my $reset_ctrl = GetRegVar("SUN_TOP_CTRL_RESET_CTRL");

    # Software reseting the board!
    if((defined $reset_sw) && (defined $reset_ctrl))
    {
        $reset_sw->ReadHW();
        $reset_ctrl->ReadHW();

        $reset_sw->SetField("chip_master_reset", 1);
        $reset_ctrl->SetField("master_reset_en", 1);

        # must be the correct order!
        printf("Reboot!!!  Please wait while reconnecting.\n");
        $reset_ctrl->WriteHW();
        $reset_sw->WriteHW();

        # reconnect
        ReConnect();
    }

    return;
}

#*************************************************************************func
# Purpose:
#   Get the cached reg
#
#*****************************************************************************
sub GetRegVar
{
    return $g_rdb->MakeRegVar(shift);
}

#*************************************************************************func
# Purpose:
#   Convert a Perl array into a C unsigned array
#
#*****************************************************************************
sub _CreateUnsignedArray
{
    if($^O =~ m/Win32/i)
    {
        my $len = scalar(@_);
        my $ia = PerlExtLib::ul_array_create($len);
        for (my $i = 0; $i < $len; $i++)
        {
            my $val = shift;
            PerlExtLib::ul_array_set_elem($ia,$i,$val);
        }
        return $ia;
    }
    else
    {
        return undef;
    }
}
# End of File
