#!/usr/bin/perl

$file_in_name = $ARGV[0];
($file_out_0_name = $file_in_name) =~ s/L00/000/;
($file_out_1_name = $file_in_name) =~ s/L00/001/;
($file_out_2_name = $file_in_name) =~ s/L00/002/;
($file_out_3_name = $file_in_name) =~ s/L00/003/;

open (FILE_IN, "<$file_in_name") or die "\nERROR: unable to open file $file_in_name: $!";
open (FILE_OUT_0, ">$file_out_0_name") or die "\nERROR: unable to open file $file_out_0_name: $!";
open (FILE_OUT_1, ">$file_out_1_name") or die "\nERROR: unable to open file $file_out_1_name: $!";
open (FILE_OUT_2, ">$file_out_2_name") or die "\nERROR: unable to open file $file_out_2_name: $!";
open (FILE_OUT_3, ">$file_out_3_name") or die "\nERROR: unable to open file $file_out_3_name: $!";

$count = 0;

while(<FILE_IN>) {
    $line = $_;
    chomp $_;
    if (($count % 4 ) == 0) {
        printf FILE_OUT_0 ("$_\n");
    }
    if (($count % 4 ) == 1) {
        printf FILE_OUT_1 ("$_\n");
    }
    if (($count % 4 ) == 2) {
        printf FILE_OUT_2 ("$_\n"); 
    }
    if (($count % 4 ) == 3) {
        printf FILE_OUT_3 ("$_\n"); 
    }
    $count++;
}
close FILE_IN;
close FILE_OUT_0;
close FILE_OUT_1;
close FILE_OUT_2;
close FILE_OUT_3;
