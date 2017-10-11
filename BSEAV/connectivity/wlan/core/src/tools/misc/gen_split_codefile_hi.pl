#!/usr/bin/perl

$file_in_name = $ARGV[0];
($file_out_4_name = $file_in_name) =~ s/L01/004/;
($file_out_5_name = $file_in_name) =~ s/L01/005/;
($file_out_6_name = $file_in_name) =~ s/L01/006/;
($file_out_7_name = $file_in_name) =~ s/L01/007/;

open (FILE_IN, "<$file_in_name") or die "\nERROR: unable to open file $file_in_name: $!";
open (FILE_OUT_4, ">$file_out_4_name") or die "\nERROR: unable to open file $file_out_4_name: $!";
open (FILE_OUT_5, ">$file_out_5_name") or die "\nERROR: unable to open file $file_out_5_name: $!";
open (FILE_OUT_6, ">$file_out_6_name") or die "\nERROR: unable to open file $file_out_6_name: $!";
open (FILE_OUT_7, ">$file_out_7_name") or die "\nERROR: unable to open file $file_out_7_name: $!";

$count = 0;

while(<FILE_IN>) {
    $line = $_;
    chomp $_;
    if (($count % 4 ) == 0) {
        printf FILE_OUT_4 ("$_\n");
    }
    if (($count % 4 ) == 1) {
        printf FILE_OUT_5 ("$_\n");
    }
    if (($count % 4 ) == 2) {
        printf FILE_OUT_6 ("$_\n"); 
    }
    if (($count % 4 ) == 3) {
        printf FILE_OUT_7 ("$_\n"); 
    }
    $count++;
}
close FILE_IN;
close FILE_OUT_4;
close FILE_OUT_5;
close FILE_OUT_6;
close FILE_OUT_7;
