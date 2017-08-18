#!/bin/bash

for file in *.htm
	do \
	cat $file | egrep '^\.' > /tmp/foo1; \
	cat ../../locale/english/$file | egrep '^\.' > /tmp/foo2; \
	echo $file ;
	diff -b /tmp/foo1 /tmp/foo2 | fgrep -v '.define'
	diff -b /tmp/foo1 /tmp/foo2 | egrep   '\.define.*[<>]' 
	wc /tmp/foo1
	wc /tmp/foo2
	
	cat $file | egrep '\$([^)]*)' | sed 's/[^\$]*\(\$([^)]*)\)[^\$]*/\1/g'  | sed 's/\(.\)\$/\1\
$/g' > /tmp/foo3; 
	cat ../../locale/english/$file | egrep '\$([^)]*)' | sed 's/[^\$]*\(\$([^)]*)\)[^\$]*/\1/g'  | sed 's/\(.\)\$/\1\
$/g' > /tmp/foo4; 
	diff -b /tmp/foo3 /tmp/foo4 
	cat $file | egrep '<[hH][1-6]' | egrep -v '</[hH][1-6]'
done
