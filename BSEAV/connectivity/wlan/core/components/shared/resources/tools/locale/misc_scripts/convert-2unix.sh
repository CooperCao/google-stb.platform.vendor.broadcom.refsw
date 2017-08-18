#!/bin/bash

for file in *.htm
	do \
	dos2unix -k $file
done
