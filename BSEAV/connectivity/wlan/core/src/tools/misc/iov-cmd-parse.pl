#!/usr/bin/perl
# 
# This file contains code to parse C code for command and
# IO Variable descriptions and produce output for use in
# documentation.  Currently, it only creates Twiki tables.
#
# In general, it will parse C code initializers of the form
# some_declaration = { {"name", stuff...}, {"name", stuff...}...};
#
# In particular, it uses the first entry as a key.  This allows
# correlating declarations in different files for related data.
#

use FileHandle;
use strict 'refs';

# $DEBUG = 1;

################################################################
# Some constants

# For temporarily replacing commas when splitting a data
$comma_protector = "QcOmMa";

# Start of IOV declarations
$iov_dec_start = "wlu_iov_info_t wlu_iov_info";

# Start of primary command list declarations
$cmd_list_start = "cmd_t wl_cmds";

# Start of command category declarations
$cmd_cat_start = "cmd2cat_t cmd2cat";

$dquote = "\"";

# Maps from code identifiers to display info
$flag_map_hash = {
# Categories
    "CMD_PHY" =>   "PHY",
    "CMD_CHAN" =>  "channel",
    "CMD_RATE" =>  "rate",
    "CMD_POWER" => "power",
    "CMD_MAC" =>   "mac",
    "CMD_MGMT" =>  "management",
    "CMD_SEC" =>   "security",
    "CMD_WME" =>   "wme/wmm",
    "CMD_MON" =>   "monitor",
    "CMD_AP" =>    "AP",
    "CMD_STA" =>   "STA",
    "CMD_BOARD" => "board",
    "CMD_ADMIN" => "admin",
    "CMD_DEV" =>   "device",
    "CMD_DEP" =>   "deprecated",
    "CMD_UNCAT" => "uncategorized",

# diag iov flags
    "WLU_IOVI_READ_ONLY" =>      "read-only",
    "WLU_IOVI_WRITE_ONLY" =>     "write-only",
    "WLU_IOVI_BCM_INTERNAL" =>   "BCM-internal",
    "WLU_IOVI_DEFAULT_VALID" =>  "default-valid",

# driver iov flags
    "IOVF_WHL" =>        "zero-or-more",
    "IOVF_NTRL" =>       "one-or-more",
    "IOVF_SET_UP" =>     "set-drv-up",
    "IOVF_SET_DOWN" =>   "set-drv-down",
    "IOVF_SET_CLK" =>    "set-clock",
    "IOVF_SET_BAND" =>   "set-band",
    "IOVF_GET_CLK" =>    "get-clock",
    "IOVF_GET_BAND" =>   "get-band",

#IOV data types
    "IOVT_VOID" =>      "void",
    "IOVT_BOOL" =>      "bool",
    "IOVT_INT8" =>      "int8",
    "IOVT_UINT8" =>     "uint8",
    "IOVT_INT16" =>     "int16",
    "IOVT_UINT16" =>    "uint16",
    "IOVT_INT32" =>     "int32",
    "IOVT_UINT32" =>    "uint32",
    "IOVT_BUFFER" =>    "buffer"
};
################################################################

# Protect commas within quoted strings; used before splitting entry
sub protect_quoted_commas {
    my $string = shift;

    my $out_string = "";
    my @str_ar = split '',$string;
    my $qcount = 0;

    # Count double quotes and replace commas inside them.  
    # Yes, I'm sure there's a better way.
    foreach $char (@str_ar) {
        if ($char eq "\"") {
            $qcount++;
            $out_string .= $char;
        } elsif ($char eq ",") {
            if ( !($qcount % 2)) {
                $out_string .= $char;
            } else {
                $out_string .= $comma_protector;
            }
        } else {
            $out_string .= $char;
        }
    }

    return $out_string;
}

# Change protected commas back to commas
sub unprotect_quoted_commas {
    my $string = shift;

    $string =~ s/$comma_protector/,/g;
    return $string;
}

# Split up a flags field with the given sep; store in the given hash ref
sub split_flags {
    my $flags = shift;
    my $sep = shift;
    my $hash_ref = shift;

    foreach ( split /$sep/,$flags) {
        /\b(\w+)\b/;
        if ($1 ne 0) {
            $hash_ref->{$1} = 1;
        }
    }
}

# Initial parse of entry assuming key in first location of comma sep list
sub split_entry_get_key {
    my $entry = shift;
    my @ar;
    my $key;

    $entry = protect_quoted_commas($entry);
    $entry =~ s/{//g;
    $entry =~ s/}//g;
    @ar = split ',',$entry;
    $ar[0] =~ /$dquote([^"]+)"/;
    $key = $1;

    return ($key, @ar);
}

# Parse an IOV entry and update hash ref for the key
# $ar is an array ref of split entry
sub hash_iov_entry {
    my $mod_name = shift;
    my $hash_ref = shift;
    my $key = shift;
    my $ar = shift;

    $hash_ref->{$key}{CATS} = {};   # Need to init before referencing below
    $hash_ref->{$key}{FLAGS} = {};

    # Get categories and flags, + and | separated lists respectively
    split_flags($ar->[1], "\\\+", $hash_ref->{$key}{CATS});
    split_flags($ar->[2], "\\\|", $hash_ref->{$key}{FLAGS});
    # Init value
    $hash_ref->{$key}{INITVAL} = $ar->[3];

    $ar->[4] = unprotect_quoted_commas($ar->[4]);
    $ar->[4] =~ /$dquote([^"]+)"/;
    $hash_ref->{$key}{DESC} = $1;
    $hash_ref->{$key}{DIAG} = 1;
}

# Return 1 if already defined

sub check_multi_def {
    my $mod_name = shift;
    my $hash_ref = shift;
    my $key = shift;
    my $tstr = shift;  # Output type string

    if (defined($hash_ref->{$key}{MODULE})) {
        if ($hash_ref->{$key}{MODULE} eq $mod_name) {
            print STDERR "NOTE: $tstr $key is conditionally defined in ".
                "$mod_name\n";
            $hash_ref->{$key}{MULTIPLY_DEFINED} = 1;
        } else {
            print STDERR "WARNING, $tstr $key is defined in $mod_name " .
                "and $hash_ref->{$key}{MODULE}\n";
            $hash_ref->{$key}{OTHER_MODS}{$mod_name} = 1;
        }
        return 1;
    }

    return 0;
}

# Parse an IOV driver entry (e.g. from wlc.c)
# Driver flag at [2], data type at [3]
# $ar is an array ref of split entry
sub hash_iov_drv {
    my $mod_name = shift;
    my $hash_ref = shift;
    my $key = shift;
    my $ar = shift;

    if (!check_multi_def($mod_name, $hash_ref, $key, "IOV")) {
        $hash_ref->{$key}{MODULE} = $mod_name;
        # Currently, only a single driver flag supported/occurs
        if ($ar->[2] =~ /\b(IOVF.+)\b/) {
            $hash_ref->{$key}{FLAGS}{$1} = 1;
        }
        $ar->[3] =~ s/\s//g;
        $hash_ref->{$key}{TYPE} = $ar->[3];
    }
}

# Parse a command category entry and update hash ref for the key
# $ar is an array ref of split entry
sub hash_cmd_cat {
    my $mod_name = shift;
    my $hash_ref = shift;
    my $key = shift;
    my $ar = shift;

    $hash_ref->{$key}{CATS} = {};   # Need to init before referencing below
    split_flags($ar->[1], "\\\+", $hash_ref->{$key}{CATS});
}

# Parse a command list entry and update hash ref for the key
# $ar is an array ref of split entry
sub hash_cmd_list {
    my $mod_name = shift;
    my $hash_ref = shift;
    my $key = shift;
    my $ar = shift;

    if (!check_multi_def($mod_name, $hash_ref, $key, "command")) {
        $hash_ref->{$key}{MODULE} = $mod_name;
        # Can check for set/get indications in ar->[2] and ar->[3] here
        # FIXME:  Multiple line descriptions
        $ar->[4] = unprotect_quoted_commas($ar->[4]);
        $ar->[4] =~ s/^[^"]*"//g;
        $ar->[4] =~ s/$dquote[^"]*$//g; # "
        $ar->[4] =~ s/$dquote//g;
        $hash_ref->{$key}{DESC} = $ar->[4];
#        $ar->[4] =~ /"([^"]+)"/; # "
#        $hash_ref->{$key}{DESC} = $1;
    }
}

# Read and parse an entry from { to }
sub read_entry {
    my $infile = shift;
    my $file_ref = shift;
    my $line;
    my $entry = "";
    my $start_found = 0;

    while ($line = <$infile>) {
        next if ($line =~ /^\s*#/);  # Skip ifdefs, etc
        # Ignore single line comment contents
        $line =~ s/\/\/.*$//g;
        chomp($line);
        if ($line =~ /\}\s*\;/) {  # end of structure
            if ($start_found) {
                print STDERR "ERROR: End of structure in entry in $file_ref\n";
                print STDERR "   Had read $entry\n";
            }
            return "";
        }
        if ($entry =~ /\{/) { # Found start of entry
            $start_found = 1;
        }
        # The only issue here is that closed bracket inside of /* comment */ 
        # will get parsed
        $entry .= $line;
        if ($entry =~ /\}/) {
            return $entry;
        }
    }
    print STDERR "ERROR: End of input in $file_ref when reading $entry\n";
    return "";
}

# Read in the C code in a command or IOV file
#    file_ref       Name of file
#    hash_ref       Pointer to hash to fill out
#    dec_start      String indicating start of declaration structure to parse
#    parse_routine  What routine to call to parse an entry
#
# Updates the hash in hash_ref according to the per-entry parsing routine
# (above) selected by file_type.
#
sub read_file {
    my $file_ref = shift;
    my $hash_ref = shift;
    my $dec_start = shift;
    my $parse_routine = shift;
    my @ar;
    my $key;
    my $entry;
    my $line;
    my $mod_name;
    my $found = 0;

    open(INFILE, "<$file_ref") ||
	die "Could not open $file_ref: $!\n";
    print STDERR "Openned $file_ref\n";
    $mod_name = $file_ref;
    $mod_name =~ s:.*/::g;
    $mod_name =~ s/\.c//g;
    $mod_name =~ s/wl_//g;
    $mod_name =~ s/wlc_//g;
    $mod_name =~ s/wlu_//g;

    while (<INFILE>) {
        if (/$dec_start/) {
            $found = 1;
            while (1) { # Parse entries until end of structure or file
                $entry = read_entry(INFILE, $file_ref);
                last if !$entry;

                # Remove /* comments */
                $entry =~ s:/\*.*\*/::g;
                # Split the entry by commas and get key from first elt
                ($key, @ar) = split_entry_get_key($entry);
                next if (($key eq "") || ($key =~ /zzzz/));
                &$parse_routine($mod_name, $hash_ref, $key, \@ar);
            }
        }
        last if $found; # Have processed all entries
    }

    close(INFILE);
    if (!$found) {
        print STDERR "ERROR: Failed to find $dec_start in $file_ref\n";
    }
}

# Format flags/categories according to the given map

sub map_flags {
    my $ref = shift;
    my $hash_key = shift;
    my $map = shift;
    my $output = "";
    my $first = 1;

    foreach my $key (sort keys %{$ref->{$hash_key}}) {
        $output .= ($first ? "" : ", ") . "$map->{$key}";
        $first = 0;
    }

    return $output;
}

# Wrap description as needed for html output
sub wrap_desc {
    my $ref = shift;
    my $desc = shift;

    $desc =~ s/\|/ -or- /g;  # Use I for vertical bar
    $desc =~ s/\\n/%BR%/g;
    $desc =~ s/\\t/ /g;
    $desc =~ s/\s+/ /g;
    $desc =~ s/</[/g;
    $desc =~ s/>/]/g;
    $desc =~ s/\\'/'/g;
    $desc =~ s/\\//g;
    if ($ref->{OTHER_MODS}) {
# TODO:  Append alternate descriptions here
        $desc .= "%BR% *NOTE:* Defined in multiple modules.";
#        foreach my $mod (sort keys %{$ref->{OTHER_MODS}}) {
#            $desc .= " $mod";
#        }
    }
    if ($ref->{MULTIPLY_DEFINED}) {
        $desc .= "%BR% *NOTE:* Conditionally defined in $ref->{MODULE}; ".
            "%BR%given info is valid for first definition only.";
    }
    return $desc;
}

################################################################
# Output routines.  One each for an IO variable entry and for
# a command entry.
################################################################

# Eventually, should probably create XML output.  For now, simple
# html/twiki table output.

# Header for IOV table
# TODO:  Add origin module and default value (when valid)
$iov_tab_header = "| *IO Variable* | *Data Type* | *Module* | " .
    "*Categories* | *Flags* | *Description* |";

# ref     Pointer to entry
sub gen_iov_entry {
    my $key = shift;
    my $ref = shift;
    my $output = "";

    $output = "| $key |";
    $output .= " $flag_map_hash->{$ref->{TYPE}} |";
    $output .= " $ref->{MODULE} ";
    if ($ref->{OTHER_MODS}) {
# TODO:  This should be in RED as it's probably a bad thing
        foreach my $mod (sort keys %{$ref->{OTHER_MODS}}) {
            $output .= "%BR%$mod";
        }
    }
    $output .= " |";
    $output .= " " . map_flags($ref, "CATS", $flag_map_hash) . " |";
    $output .= " " . map_flags($ref, "FLAGS", $flag_map_hash) . " |";
    $output .= " " . wrap_desc($ref, $ref->{DESC}) . " |";
    return $output;
}

# Header for command table
$cmd_tab_header = "| *Command* | *Module* | *Categories* | *Description* |";

# ref     Pointer to entry
sub gen_cmd_entry {
    my $key = shift;
    my $ref = shift;
    my $output = "";

    $output = "| $key |";
    $output .= " $ref->{MODULE} ";
    if ($ref->{OTHER_MODS}) {
        foreach my $mod (sort keys %{$ref->{OTHER_MODS}}) {
            $output .= "%BR%$mod";
        }
    }
    $output .= " |";
    $output .= " " . map_flags($ref, "CATS", $flag_map_hash) . " |";
    $output .= " " . wrap_desc($ref, $ref->{DESC}) . " |";
    return $output;
}

# Where to get source code from
$src_dir = "/projects/hnd_software/gallery/src";
$exe_dir = "$src_dir/wl/exe";
$sys_dir = "$src_dir/wl/sys";

# This is the list of files in the driver that have IO Vars and
# the declaration start marker for each
@iov_drv_files = 
    (
     { FILE => "wlc.c", DEC_START => "bcm_iovar_t wlc_iovars"},
     { FILE => "wlc_phy.c", DEC_START => "bcm_iovar_t phy_iovars"},
     { FILE => "wl_ndfips.c", DEC_START => "bcm_iovar_t fips_iovars"},
#     { FILE => "wl_ndis.c", DEC_START => "bcm_iovar_t ndis_iovars"},
     { FILE => "wl_oidcmn.c", DEC_START => "bcm_iovar_t oid_iovars"},
     { FILE => "wl_pfn.c", DEC_START => "bcm_iovar_t wl_pfn_iovars"},
     { FILE => "wl_arpoe.c", DEC_START => "bcm_iovar_t arp_iovars"},
     { FILE => "wlc_ampdu.c", DEC_START => "bcm_iovar_t ampdu_iovars"},
     { FILE => "wlc_amsdu.c", DEC_START => "bcm_iovar_t amsdu_iovars"},
     { FILE => "wlc_ap.c", DEC_START => "bcm_iovar_t wlc_ap_iovars"},
     { FILE => "wlc_ba.c", DEC_START => "bcm_iovar_t ba_iovars"},
     { FILE => "wlc_cac.c", DEC_START => "bcm_iovar_t cac_iovars"},
     { FILE => "wlc_ccx.c", DEC_START => "bcm_iovar_t ccx_iovars"},
     { FILE => "wlc_cram.c", DEC_START => "bcm_iovar_t cram_iovars"},
     { FILE => "wlc_led.c", DEC_START => "bcm_iovar_t led_iovars"},
     { FILE => "wlc_nitro.c", DEC_START => "bcm_iovar_t nitro_iovars"},
     { FILE => "wlc_rate_sel.c", DEC_START => "bcm_iovar_t ratesel_iovars"},
     { FILE => "wlc_scan.c", DEC_START => "bcm_iovar_t wlc_scan_iovars"},
     { FILE => "wlc_wet.c", DEC_START => "bcm_iovar_t wet_iovars"},
     { FILE => "wl_toe.c", DEC_START => "bcm_iovar_t toe_iovars"}
     );

# This is the list of files in the diag that have command lists and
# the declaration start marker for each
@cmd_list_files = 
    (
     { FILE => "wlu.c", DEC_START => "cmd_t wl_cmds"},
     { FILE => "wlu_macos.c", DEC_START => "cmd_t apple80211_cmds"},
     { FILE => "wlu_ndis.c", DEC_START => "cmd_t ndis_cmds"},
     { FILE => "wlu_vista.c", DEC_START => "cmd_t vista_cmds"}
     );

# Output file names
$out_dir = "/home/dtalayco/pub/iov-cmd-output";
$cmd_file = "$out_dir/cmd_tables.txt";
$iov_file = "$out_dir/iov_tables.txt";

################################################################
# Main file processing code
################################################################

print STDERR "Writing to file $iov_file\n";
open (OUTFILE, ">$iov_file") || 
    die "Could not open $iov_file for writing: $!\n";

my $iov_hash;
read_file("$exe_dir/wlu_iov.c", \%{$iov_hash}, $iov_dec_start, \
          &hash_iov_entry);
foreach my $hash (@iov_drv_files) {
    read_file("$sys_dir/$hash->{FILE}", \%{$iov_hash}, 
              $hash->{DEC_START}, \&hash_iov_drv);
}

# Keys with TYPE and not DIAG are in driver, but not documented.
my $found = 0;
print OUTFILE "---+++ Last update\n";
print OUTFILE "This IO Variable content generated " . `date` . "\n";
print OUTFILE <<"EOT";
---+++ IO Variables Missing Descriptions
The following IO variables appear in the driver but do not appear in
the diag list in src/wl/exe/wlu_iov.c:
<verbatim>
EOT
foreach my $key (sort keys %$iov_hash) {
    if (!($iov_hash->{$key}{DIAG}) && $iov_hash->{$key}{TYPE}) {
        $found++;
        printf OUTFILE "    %-30s\n", $key;
    }
}
print OUTFILE "    (None found)\n" if ($found eq 0);
print OUTFILE "\n";
print OUTFILE "</verbatim>\n";

# Keys with DIAG and not TYPE are in diag, but not in the driver.
my $found = 0;
print OUTFILE <<"EOT";
---+++ Defunct IO Variables (not in driver)
The following IO variables appear in the diag description, but
were not found in the driver:
<verbatim>
EOT

foreach my $key (sort keys %$iov_hash) {
    if ($iov_hash->{$key}{DIAG} && !($iov_hash->{$key}{TYPE})) {
        $found++;
        printf OUTFILE "    %-30s\n", $key;
    }
}
print OUTFILE "    (None found)\n" if ($found eq 0);
print OUTFILE "\n";
print OUTFILE "</verbatim>\n";

print OUTFILE "---+++ IO Variable Table\n";
print OUTFILE "$iov_tab_header\n";

foreach my $key (sort keys %$iov_hash) {
    # if TYPE is not defined, IOV is no longer in driver
    # if ? is not defined, IOV is in driver, but not in UI
    print OUTFILE gen_iov_entry($key, $iov_hash->{$key}) . "\n";
}
close(OUTFILE);
print STDERR "Done processing IO variables\n\n";

################################################################
# Command output
################################################################

print STDERR "Writing to file $cmd_file\n";
open (OUTFILE, ">$cmd_file") || 
    die "Could not open $cmd_file for writing: $!\n";
my $cmd_hash;
read_file("$exe_dir/wlu_cmd.c", \%{$cmd_hash}, $cmd_cat_start, \&hash_cmd_cat);
foreach my $hash (@cmd_list_files) {
    read_file("$exe_dir/$hash->{FILE}", \%{$cmd_hash}, 
              $hash->{DEC_START}, \&hash_cmd_list);
}

# Keys with TYPE and not DIAG are in driver, but not documented.
my $found = 0;
print OUTFILE "---+++ Last update\n";
print OUTFILE "This command list content generated " . `date` . "\n";
print OUTFILE <<"EOT";
---+++ Commands Missing Categorization
The following commands appear in wlu.c but do not appear in
the list in src/wl/exe/wlu_cmd.c:
<verbatim>
EOT

foreach my $key (sort keys %$cmd_hash) {
    if (!($cmd_hash->{$key}{CATS}) && $cmd_hash->{$key}{DESC}) {
        $found++;
        printf OUTFILE "    %-30s\n", $key;
    }
}
print OUTFILE "    (None found)\n" if ($found eq 0);
print OUTFILE "\n";
print OUTFILE "</verbatim>\n";

# Keys with DIAG and not TYPE are in diag, but not in the driver.
my $found = 0;
print OUTFILE <<"EOT";
---+++ Defunct Commands (not in wlu.c)
The following commands appear in wlu_cmd.c, but were not
found in the executable command list in wlu.c:
<verbatim>
EOT

foreach my $key (sort keys %$cmd_hash) {
    if ($cmd_hash->{$key}{CATS} && !($cmd_hash->{$key}{DESC})) {
        $found++;
        printf OUTFILE "    %-30s\n", $key;
    }
}
print OUTFILE "    (None found)\n" if ($found eq 0);
print OUTFILE "\n";
print OUTFILE "</verbatim>\n";

print OUTFILE "---+++ Command List Table\n";
print OUTFILE "$cmd_tab_header\n";
foreach my $key (sort keys %$cmd_hash) {
    # if ? not defined, command is not categorized.
    # if ? not defined, command is no longer in UI
    print OUTFILE gen_cmd_entry($key, $cmd_hash->{$key}) . "\n";
}
print STDERR "Done processing commands\n";

close(OUTFILE);



################################################################
# Test code
################################################################

sub test_comma_quoting {
    my $tstr = "abc,def";
    print STDERR "in: $tstr\n";
    $tstr = protect_quoted_commas($tstr);
    print STDERR "out: $tstr\n";
    $tstr = unprotect_quoted_commas($tstr);
    print STDERR "out2: $tstr\n";

    my $tstr = "abc,def\"ghi,jkl\"mno,pqr\"stu,vwx\"hello,";
    print STDERR "in: $tstr\n";
    $tstr = protect_quoted_commas($tstr);
    print STDERR "out: $tstr\n";
    $tstr = unprotect_quoted_commas($tstr);
    print STDERR "out2: $tstr\n";

    my $tstr = "abcdef";
    print STDERR "in: $tstr\n";
    $tstr = protect_quoted_commas($tstr);
    print STDERR "out: $tstr\n";
    $tstr = unprotect_quoted_commas($tstr);
    print STDERR "out2: $tstr\n";
}
