#!/usr/bin/env python

# Copyright 2015, Broadcom Corporation.
# All Rights Reserved.
#
import re
import pdb
import os
import fileinput
import sys, getopt
import errno

# Read the config values under manufacturer section from the vendor config file
# into an array, which will be copied over the final INF
def read_config ( config_file, mark_begin, mark_end, array):
	try:
		f = open(config_file)
	except IOError:
		print " file does not exist"

	finally:
		found_match = False
		# read the contents of config file line by line
		contents = f.readline()
		while contents:
			# condition which check if this current section begins with
			# mark end ignoring the mark start pattern
			if contents.startswith(mark_end) and not contents.startswith(mark_begin):
				if found_match == True:
					break
				found_match = False
				contents = f.readline()
			# the section which it is looking for matched
			elif contents.startswith(mark_begin) or found_match == True:
				if found_match == False:
					contents = f.readline()
				array.append(contents)
				found_match = True
				contents = f.readline()
			# non matching sections ignore
			else:
				found_match = False
				contents = f.readline()
		f.close()
	return

# read the values from [strings]  or [replace] sections from
# vendor config file into an array
def read_strings_config ( config_file, mark_begin, mark_end, array):
	try:
		f = open(config_file)
		found_match = False
	except IOError:
		print " file does not exist"
	finally:
		found_match = False
		contents = f.readline()
		while contents:
			# condition to check mark end but not begin
			if contents.startswith(mark_end) and contents != mark_begin:
				if found_match == True:
					break
				found_match = False
				contents = f.readline()
			# condition matched for mark pattern, [strings] in this case
			elif contents == mark_begin or found_match == True:
				if found_match == False:
					contents = f.readline()
				array.append(contents)
				found_match = True
				contents = f.readline()
			# not matching the string section so ignore
			else:
				found_match = False
				contents = f.readline()
		f.close()
	return

# update the contents of Manufacturer section into the array_upd which will be written
# to the INF file
def upd_manf_contents(inf_file, mark_begin, mark_end, array_upd, array):
	try:
		f =  open(inf_file)
	except IOError:
		print "file does not exist"
	finally:
		exit_loop = False
		found_match = False
		contents = f.readline()
		while contents:
			# condition which check if this current section begins with
			# mark end ignoring the mark start pattern
			if contents.startswith(mark_end) and not contents.startswith(mark_begin):
				found_match = False
				array_upd.append(contents)
				contents = f.readline()
			# the section which needs to be updated matched so overwrite into array_upd
			elif contents.startswith(mark_begin) or found_match == True:
				if found_match == False:
					array_upd.append(contents)
				if exit_loop == False:
					for i in range(len(array)):
						found_match = True
						array_upd.append(array[i])
					exit_loop = True
				contents = f.readline()
			# unmatched sections retain it into the array_upd
			else:
				found_match = False
				array_upd.append(contents)
				contents = f.readline()
	f.close()
	return

# write array_udp to the INF File
def write_inf(inf_file, array_upd):
	try:
		fd = open(inf_file, 'w')
	except IOError:
		print " file does not exist"
	finally:
		for i in range(len(array_upd)):
			fd.write(array_upd[i])
	fd.close()
	return

# append the [strings] section from array into array_upd
def append_strings_inf(inf_file, mark_begin, mark_end, array_upd, array):
	try:
		f =  open(inf_file)
	except IOError:
		print " file does not exist"
	finally:
		found_match = False
		exit_loop = False
		contents = f.readline()
		while contents:
			# condition to check mark end but not begin
			if contents.startswith(mark_end) and contents != mark_begin:
				found_match = False
				array_upd.append(contents)
				contents = f.readline()
			# condition matched for mark pattern, [strings] in this case
			# so update the array_upd to be written to INF file
			elif contents == mark_begin or found_match == True:
				if found_match == False:
					array_upd.append(contents)
				if exit_loop == False:
					for i in range(len(array)):
						found_match = True
						array_upd.append(array[i])
					exit_loop = True
				contents = f.readline()
				array_upd.append(contents)
			# unmatched sections retain into array_upd
			else:
				array_upd.append(contents)
				contents = f.readline()
				found_match = False
	f.close()
	return


# replace the section which starts with [replace] with first
# one being used to match the value to updated in the INF file
# the second being the value to be copied over to the INF file
def replace_config_values(inf_file, replace_upd, replace):
	try:
		f = open(inf_file)
	except IOError:
		print " file does not exist"
	finally:
		contents = f.readline()
		match = False
		while contents:
			# iterate over the [replace] section to find a match
			for i in range(len(replace)):
				if( replace[i] == contents):
					# match found replace the configuration section
					if (( i + 1) < len(replace)):
						contents  =  replace[i+1]
						replace_upd.append(contents)
						match = True
			if match == False:
				replace_upd.append(contents)
			match = False
			# read next line
			contents = f.readline()
	return;

manf_begin = '[BROADCOM.NT'
mark_end = '['
strings_begin = '[strings]\n'
replace_begin = '[replace]\n'
array = []
array_upd = []
string_array = []
string_array_upd = []
replace = []
replace_upd = []

def main():
	if len(sys.argv) < 2:
		print "Usage: %s inf_file config_file\n", sys.argv[0]
		return 1
	if not (os.path.exists(sys.argv[1]) or os.path.exists(sys.argv[2])):
		print "Error: File %s was not found!\n", sys.argv[1]
		return 1

# replace the Manufacturer contents from config file to target inf file
	read_config(sys.argv[2], manf_begin, mark_end, array)
	upd_manf_contents(sys.argv[1], manf_begin, mark_end, array_upd, array)
	write_inf(sys.argv[1], array_upd)

# append the strings sections from config file to strings section in target inf file
	read_strings_config(sys.argv[2], strings_begin, mark_end, string_array)
	append_strings_inf(sys.argv[1], strings_begin, mark_end, string_array_upd, string_array)
	write_inf(sys.argv[1], string_array_upd)

# replace the config values by comparing the first config value with that in target and then
# replace the next value in the inf file
	read_strings_config(sys.argv[2], replace_begin, mark_end, replace)
	replace_config_values(sys.argv[1], replace_upd, replace)
	write_inf(sys.argv[1], replace_upd)
if __name__ == '__main__':
	main()
