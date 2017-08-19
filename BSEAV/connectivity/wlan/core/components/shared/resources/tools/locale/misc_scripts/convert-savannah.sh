#!/bin/bash

unzip -o *.zip
for file in *.htm
	do \
	dos2unix -k $file
	cat $file \
	| ../../../tools/locale/rmcharref \
	| perl ../../../tools/locale/formatprep.pl \
	| perl ../../../tools/locale/rmfonttags.pl \
	> $file.1 ; \
	mv $file.1 $file
done

#	| perl ../../../tools/locale/convert-savannah.pl \
