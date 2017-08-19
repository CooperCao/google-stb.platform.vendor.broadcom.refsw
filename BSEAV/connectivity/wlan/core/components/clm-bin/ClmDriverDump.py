#!/usr/bin/python

"""
Filename: ClmDriverDump.py
Author: hoges
Description: Reads the available country code/regrevs from the driver, then
             for each one requests the available channel specs, then prints
             the regulatory tx power limits for the various rates for 20MHz,
             40MHz and 20 in 40MHz.

Information contained herein is proprietary to and constitutes valuable
confidential trade secrets of Broadcom, or its licensors, and
is subject to restrictions on use and disclosure.

Copyright (c) 2011 Broadcom. All rights reserved.

The copyright notices above do not evidence any actual or
intended publication of this material.
"""

import sys
import os.path
import commands

print commands.getoutput('wl clmver') # Make sure there is at least one difference?
commands.getoutput('wl down')     # ensure driver is down for this mimo_bw_cap...
mimo_cap = commands.getoutput('wl mimo_bw_cap')
commands.getoutput('wl mimo_bw_cap 1') # make sure we can set 40MHz channels for 2.4G

commands.getoutput('wl mpc 0')    # stop driver from going auto-down
commands.getoutput('wl up')       # ensure driver is up...
commands.getoutput('wl disassoc') # and not associated (so channels aren't locked)

save_country = commands.getoutput('wl country')
countries = []

start_country = ""
end_country = ""

start = 0
if len(sys.argv) == 2:
    country_range = sys.argv[1]
    i = country_range.find('-')
    if i != -1:
        start_country = country_range[0:i]
        end_country = country_range[i + 1:len(country_range)]
        print 'Range requested: >%s< to >%s<\n' % (start_country, end_country)

if len(sys.argv) < 2 or start_country is not "":
    pipe = os.popen("wl dump country_regrevs")
    countries = [x.strip() for x in pipe.readlines()]
    status = pipe.close()
    if status:
        sys.exit("ERROR: Failed to dump country/regrevs")
    countries.sort()
#    pipe = os.popen("wl dump legacy_country_regrevs")
##    pipe = os.popen("wl dump country_regrevs")
#    legacy_countries = [x.strip() for x in pipe.readlines()]
#    status = pipe.close()
#    if status:
#        sys.exit("ERROR: Failed to dump legacy country/regrevs")
#    legacy_countries.sort()
else:
    countries = sys.argv[1:] # don't include sys.argv[0] becuase it's ClmDiverDump.py
#    legacy_countries = sys.argv

end = len(countries)

lstart = start
lend = end

# Three letters in cc: ALL, RDR, J10
# Legacy data wrong: JP/1, XU/0
# Reg changes: AI/0 (locales changed)
# (Legacy?) Weirdness: "CA/0", "TW/0"
# Not supported by XML X0/0, X1/0, X2/0, X3/0, ww/6, DE/0
# Reg changed to use locale 1b, not supported by legacy: "BS/0", "BH/0", "BH/1", "BB/0", "CA/1", "MX/1"
# Reg changed to use locale 18-3, not supported by legacy: "EU/11", "IL/3"
# Reg changed to use locale 18-1, not supported by legacy: "EU/5"
# Reg changed to use locale 27-1, not supported by legacy: "NZ/1"
# Reg changed to use locale 10-1, not supported by legacy: "PG/0"
# Reg changed to use locale 27-1, not supported by legacy: "TW/5", "US/56"
# Reg changed to use locale 27-d, not supported by legacy: "US/39"
# Reg changed to use locale 5rn, not supported by legacy: "EG/0"
# Reg changed to use locale C-9, not supported by legacy: "JP/13"
# Reg changed to use locale 25-2, not supported by legacy: "KR/11"
# Reg changed to use locale 1l, not supported by legacy: "PA/1", "SV/1"
# Reg changed to use locale A3-2, 19l-3, An1-T8, 19ln-5 not supported by legacy: "US/53"
# Reg changed to use locale 19l not supported by legacy: "X0/4"
# Reg changed to use locale a3-2 not supported by legacy: "X0/8"
# Reg changed to use locale Bn7-1, 5en not supported by legacy: "X1/7"
# Reg changed to use locale Bn7-1, 11ln-5 not supported by legacy: "X2/7"
# Reg changed to use locale Bn7-1, 3en not supported by legacy: "X3/7"
# Reg changed to use locale Bn-11 not supported by legacy: "EU/10"
# Reg changed to use locale An11 not supported by legacy: "US/54"
exceptions = "ALL/0", "RDR/0", "J10/0"
#, "AI/0", "B2/0", "CA/0", "TW/0", "X0/0", "X1/0", "X3/0", "X2/0", "ww/6", "DE/0", "BS/0", "BH/0", "BH/1", "BB/0", "CA/1", "MX/1", "EU/11", "IL/3", "EU/5", "NZ/1", "PG/0", "TW/5", "US/56", "US/39", "EG/0", "KR/11", "JP/13", "PA/1", "SV/1", "US/53", "X0/4", "X0/8", "X1/7", "X2/7", "X3/7", "EU/10", "US/54", "XU/0"

#if start == 0:
#    lstart = 0
#    lend = len(legacy_countries)
#    print 'XML data does not have the following Legacy Country Code/regrevs:'
#    for k in range(lstart, lend):
#        cc_reg = legacy_countries[k]
##        if cc_reg in legacy_countries[k+1:lend]:
##            print 'Legacy data has Duplicate Country Code/regrev: ' +  cc_reg + '\n'
#        if cc_reg in countries:
#            """print 'XML data has Country Code/regrev: ' +  cc_reg + '\n'"""
#        else:
#            """print cc_reg"""
#
#    print '\nLegacy data does not have the following XML Country Code/regrevs:'
#    for k in range(start, end):
#        cc_reg = countries[k]
##        if cc_reg in legacy_countries[k+1:lend]:
##            print 'Legacy data has Duplicate Country Code/regrev: ' +  cc_reg + '\n'
#        if cc_reg in legacy_countries:
#            """print 'XML data has Country Code/regrev: ' +  cc_reg + '\n'"""
#        else:
#            """print cc_reg"""
#
#    if start_country is not "":
#        i = -1
#        for k in range(lstart, lend):
#            if i == -1:
#                j = legacy_countries[k].find(start_country)
#                if j != -1:
#                    print 'Found %s in %s\n' % (start_country, legacy_countries[k])
#                    i = k
#                    start = k
#                    lstart = k
#        if i != -1:
#            i = -1
#            for k in range(lend-1, lstart, -1):
#                if i == -1:
#                    j = legacy_countries[k].find(end_country)
#                    if j != -1:
#                        print 'Found %s in %s (not %s)\n' % (end_country, legacy_countries[k], legacy_countries[k + 1#])
#                        i = k
#                        end = k + 1
#                        lend = k + 1

print "Loop len = %d" % (lend - lstart)

#for k in range (start, end):
#    cc_reg = countries[k]
for k in range(lstart, lend):
    cc_reg = countries[k]
    if cc_reg in countries and cc_reg not in exceptions:
        if cc_reg in "#a/0":
            cc_reg = "ALL/0"
        if cc_reg in "#r/0":
            cc_reg = "RDR/0"
        print 'Read  Country Code/regrev: ' + cc_reg + '\n'

        result = commands.getoutput('wl country ' + cc_reg)

        if 'Bad' not in result and 'Could not' not in result:
            print 'Set  Country Code/regrev: ' + cc_reg + '\n'

            channels = []
            print commands.getoutput('wl country')

            pipe = os.popen("wl chanspecs")
            chspecs = pipe.readlines()
            status = pipe.close()

            if status:
                sys.exit("ERROR: Failed to read wl chanspecs")

            for cline in chspecs:
                cline = cline.strip()
                i = cline.find(' ')
                if i != -1:
                    channels.append(cline[:i])

            print 'Channels:'
            for ch in channels:
                print ch + ' ',
            print


            print commands.getoutput('wl dump locale')
            """commands.getoutput('wl chanspec ' + channels[0])
            print commands.getoutput('wl dump txpwr_reg_max')

            pipe = os.popen("wl dump agg_map")
            agg_map = pipe.readlines()
            status = pipe.close()

            for aline in agg_map:
                aline = aline.strip()
            agg_map.sort()
            for aline in agg_map:
                i = aline.find('Map')
                if i == -1:
                    print aline,"""

            for ch in channels:
                result = commands.getoutput('wl chanspec ' + ch)
                if 'Bad Channel' not in result:
                # This is a little more complicated than it needs to be so we can get rid of the
                # manky whitespace at the ends of the lines
                #        cleaned_output = commands.getoutput('wl dump txpwr_reg')
                    pipe = os.popen("wl dump txpwr_reg")
                    cleaned_output = [x.strip() for x in pipe.readlines()]
                    status = pipe.close()
                    if status:
                        sys.exit("ERROR: Failed to read wl dump txpwr_reg")
                    for xline in cleaned_output:
                        print xline
                else:
                    print 'ERROR: Bad Channel: ' + ch
        else:
            print 'ERROR: Bad Country Code/regrev: ' + cc_reg + '\n'
    else:
        print 'Skipping  Country Code/regrev: ' + cc_reg + '\n'


# clean up
commands.getoutput('wl down')       # ensure driver is down for this mimo_bw_cap...
commands.getoutput('wl mimo_bw_cap ' + mimo_cap)
commands.getoutput('wl up')

i = save_country.find('(')
j = save_country.find(')')

if i != -1 and j != -1:
    result = commands.getoutput('wl country ' + save_country[i + 1:j])
    print('Country/regrev restored to: ' + commands.getoutput('wl country'))
