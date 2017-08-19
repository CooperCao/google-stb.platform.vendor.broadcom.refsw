#!/bin/bash

rm patch.txt
for file in  *.htm
do \
#		diff -c0 -biBaI'<[^>]*>' $file ../$1/$file >> patch.txt
		mgdiff -args -biBa $file ../$1/$file
done
#		unix2dos  patch.txt

#sed -e 's/^[^<>]*//' -e 's/[^<>]*$//' -e 's/>[^<>]*</></g' | sed -e 's/>/>\
#/g' | fgrep  '>'
