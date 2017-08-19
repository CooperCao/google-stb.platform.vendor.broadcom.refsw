#!/bin/bash

for file in  bcmwl-specs.htm
do \
		cat $file | perl ../../../tools/locale/check-tags.pl  | fgrep -v define | fgrep -vi '</li>' | fgrep -vi '</p>' | egrep -vi '\</*B\>' > /tmp/$file.1 
		cat ../../locale/english/$file | perl ../../../tools/locale/check-tags.pl  | fgrep -vi define | fgrep -vi '</li>' | fgrep -v '</p>' | egrep -vi '\</*B\>' > /tmp/$file.2
		mgdiff -args -ibaI'\[.*\]' /tmp/$file.1 /tmp/$file.2 
done

#sed -e 's/^[^<>]*//' -e 's/[^<>]*$//' -e 's/>[^<>]*</></g' | sed -e 's/>/>\
#/g' | fgrep  '>'
