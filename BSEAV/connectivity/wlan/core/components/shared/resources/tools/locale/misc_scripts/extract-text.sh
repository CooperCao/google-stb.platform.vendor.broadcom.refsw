#!/bin/bash

unzip -o *.zip
for file in *.htm
	do \
	dos2unix -k $file
	cat $file \
	| ../../../tools/locale/misc_scripts/rmcharref \
	| ../../../tools/locale/misc_scripts/addcharref \
	| perl ../../../tools/locale/misc_scripts/formattext.pl \
	| perl ../../../tools/locale/misc_scripts/splitprep.pl \
	| perl ../../../tools/locale/misc_scripts/formatprep.pl \
	| perl ../../../tools/locale/misc_scripts/splittags.pl \
	| perl ../../../tools/locale/misc_scripts/rmemptylines.pl \
	| perl ../../../tools/locale/misc_scripts/splitlines.pl \
	> $file.1 
	mv $file.1 $file
done

#	| perl ../../../tools/locale/misc_scripts/convert-savannah.pl \

#	| perl ../../../tools/locale/misc_scripts/formatprep.pl \

#	| perl ../../../tools/locale/misc_scripts/splittags.pl \
#	| perl ../../../tools/locale/misc_scripts/rmfonttags.pl \
#	| perl ../../../tools/locale/misc_scripts/rmemptylines.pl \
