#!/bin/bash
#A script used to analyze  dongle firmware size.  It generate  a list of sybmol size and categorize  according to object file name.
# ARM toolchain is used and need be in the path
if [ -f .rtecdc.size ]; then 
	rm .rtecdc.size
	rm wlc_sym_size_text.txt
	rm wlc_sym_size_data.txt
fi

symsize rtecdc.map | sort -k1 -nr > .rtecdc.size
for file in *.o
do
echo  "processing $file"
echo -e "\t$file" >> wlc_sym_size_text.txt
arm-none-eabi-nm -S --size-sort --defined-only -tdec $file |grep -v " [D/d] "| cut -f4 -d ' ' | grep -w -f - .rtecdc.size >> wlc_sym_size_text.txt
echo -e "Total:\t\c" >>  wlc_sym_size_text.txt
arm-none-eabi-nm -S --size-sort -tdec $file | grep -v " [D/d] "| cut -f4 -d ' ' | grep -w -f - .rtecdc.size | cut -f2 |xargs echo | sed "s/ /+/g" | bc >> wlc_sym_size_text.txt
echo -e "\n\n" >> wlc_sym_size_text.txt
done
echo "data section size" >> wlc_sym__sizedata.txt
cat rtecdc.map |grep " d " | cut -f3 -d ' ' |grep -f - .rtecdc.size >> wlc_sym_size_data.txt
echo "results are in wlc_sym_size_text.txt for text section adn wlc_sym_size_data.txt for data section"
