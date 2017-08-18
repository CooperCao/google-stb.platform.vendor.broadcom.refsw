#!/bin/bash

for file in *.htm
	do \
	../../../tools/locale/free/mb2uni.exe < $file > $file.1 
	../../../tools/locale/free/uni2utf8.exe < $file.1 > $file
done
