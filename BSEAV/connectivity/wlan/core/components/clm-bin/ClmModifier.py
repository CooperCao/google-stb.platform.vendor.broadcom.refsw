import sys

import xml.etree.ElementTree as et
from copy import deepcopy
from operator import attrgetter

parent_map = {}
def generate_parents(root):
    # generate a table of every elements parent element
    global parent_map
    parent_map = dict((c, p) for p in root.getiterator() for c in p)

def get_parent(root, element):
    # get an elements parent element
    # caching for speed, but doesn't handle elements moving (deletion and creation are fine)
    if not element in parent_map:
        generate_parents(root)
    return parent_map[element]

def delete_element(root, element):
    # delete a single element (and everything it contains)
    parent = get_parent(root, element)
    out = []
    for e in parent:
        if e is not element:
            out.append(e)
    parent[:] = out

def get_locale(root, ident):
    # find a locale given its id
    for e in root.findall("./locale_list/*"):
        if e.find("./id").text == ident:
            return e

def get_crp(locale, channel, rate):
    # get a channel_rate_power entry from the locale
    for e in locale.findall("./channel_rate_power_list/channel_rate_power"):
        c_start = int(e.find("./chan_start").text)
        c_end = int(e.find("./chan_end").text)
        if c_start <= channel <= c_end:
            for r in e.findall("./rate"):
                if r.text == rate:
                    return e

def get_regulatory_power(locale, channel):
    # get a regulatory_power entry from the locale
    for e in locale.findall("./regulatory_power_list/regulatory_power"):
        c_start = int(e.find("./chan_start").text)
        c_end = int(e.find("./chan_end").text)
        if c_start <= channel <= c_end:
            return e

def copy_regulatory_power(regulatory_power, dest_locale):
    # copy the regulatory power to the dest locale
    e = deepcopy(regulatory_power)
    dest_locale.append(e)
    return e

def split_regulatory_power(root, regulatory_power, split_point):
    # split the back channels off a regulatory_power
    assert int(regulatory_power.find("./chan_start").text) < split_point <= int(regulatory_power.find("./chan_end").text)
    locale = get_parent(root, get_parent(root, regulatory_power)) # get the locale
    new_regulatory_power = copy_regulatory_power(regulatory_power, locale) # create a copy of the regulatory_power in the same locale
    regulatory_power.find("./chan_end").text = str(split_point - 1)
    new_regulatory_power.find("./chan_start").text = str(split_point)
    return new_regulatory_power

def get_aggregate_country(root, ccode, rev):
    # get an aggregate country
    for e in root.findall("./aggregate_country_list/aggregate_country"):
        if e.find("./ccode").text == ccode:
            if int(e.find("./rev").text) == rev:
                return e

def get_mapping(aggregate_country, ccode):
    # get a mapping from the aggregate country
    for e in aggregate_country.findall("./mapping_list/mapping"):
        if e.find("./ccode").text == ccode:
            return e

def get_region(root, ccode, rev):
    # get a region
    for e in root.findall("./region_list/region"):
        if e.find("./ccode").text == ccode:
            if int(e.find("./rev").text) == rev:
                return e

def get_locale_ref(region, ref_type):
    # get a locale ref from the region
    for e in region.findall("./locale_ref_list/locale_ref"):
        if e.get("type") == ref_type:
            return e

def add_locale_ref(region, ref_type, ident):
    # add a new locale ref to the region
    e = et.SubElement(region.find("./locale_ref_list"), "locale_ref")
    e.text = ident
    e.set("type", ref_type)
    return e

def copy_aggregate_country(root, aggregate_country, new_ccode, new_rev):
    # copy the given aggregate country, to the new CC/rr, return the new aggregate country
    e = deepcopy(aggregate_country)
    e.find("./ccode").text = new_ccode
    e.find("./rev").text = str(new_rev)
    root.find("./aggregate_country_list").append(e)
    return e

def copy_region(root, region, new_ccode, new_rev):
    # copy the given region, to the new CC/rr, return the new region
    e = deepcopy(region)
    e.find("./ccode").text = new_ccode
    e.find("./rev").text = str(new_rev)
    root.find("./region_list").append(e)
    return e

def add_mapping(aggregate_country, ccode, rev):
    # add a new mapping to the aggregate_country
    e = et.SubElement(aggregate_country.find("./mapping_list"), "mapping")
    et.SubElement(e, "ccode").text = ccode
    et.SubElement(e, "rev").text = str(rev)
    return e

def copy_locale(root, locale, new_id):
    # copy the given locale, to the new id, return the new locale
    e = deepcopy(locale)
    e.find("./id").text = new_id
    root.find("./locale_list").append(e)
    return e

def copy_crp(crp, dest_locale):
    # copy the channel_rate_power to the dest_locale
    e = deepcopy(crp)
    dest_locale.append(e)
    return e

def get_rate(crp, rate):
    # get a particular rate tag from a channel rate power (presumably so you can delete it)
    for e in crp.findall("./rate"):
        if e.text == rate:
            return e

def add_rate(crp, rate):
    # add a rate to a channel_rate_power
    et.SubElement(crp, "rate").text = rate

def split_crp_by_rate(root, crp, new_rates):
    # split split the listed rates out into a new channel_rate_power
    locale = get_parent(root, get_parent(root, crp)) # get the locale
    new_crp = copy_crp(crp, locale) # create a copy of the crp in the same locale

    for e in crp.findall("./rate"):
        if e.text in new_rates:
            delete_element(root, e)

    for e in new_crp.findall("./rate"):
        if e.text not in new_rates:
            delete_element(root, e)

    return new_crp

def split_crp_by_channel(root, crp, split_point):
    # split the back channels off a channel_rate_power
    assert int(crp.find("./chan_start").text) < split_point <= int(crp.find("./chan_end").text)
    locale = get_parent(root, get_parent(root, crp)) # get the locale
    new_crp = copy_crp(crp, locale) # create a copy of the crp in the same locale
    crp.find("./chan_end").text = str(split_point - 1)
    new_crp.find("./chan_start").text = str(split_point)
    return new_crp

def get_restrict(root, set_id):
    # find a restrict given its set
    for e in root.findall("./restrict_list/restrict"):
        if int(e.find("./set").text) == set_id:
            return e

def add_restrict(root, set_id, band):
    # create a new restirct with the given set_id and return it
    l = root.find("./restrict_list")
    e = et.SubElement(l, "restrict")
    e.set("band", band)
    et.SubElement(e, "set").text = str(set_id)
    et.SubElement(e, "channel_list")
    return e

def get_restrict_channels(restrict):
    # return the start/end channel pairs as an ordered list of tuples
    starts = sorted(restrict.findall("./channel_list/chan_start"), key=attrgetter("text"))
    ends = sorted(restrict.findall("./channel_list/chan_end"), key=attrgetter("text"))
    assert len(starts) == len(ends)
    return zip(starts, ends)

def add_restrict_channels(restrict, start, end):
    # add a start/end channel pair to the restrict
    l = restrict.find("./channel_list")
    et.SubElement(l, "chan_start").text = str(start)
    et.SubElement(l, "chan_end").text = str(end)

def read_xml(fname):
    # read xml
    tree = et.parse(fname)
    root = tree.getroot()

    # strip namespaces
    for e in tree.findall(".//*"):
        e.tag = e.tag.split("}")[1]

    return root

def write_xml(fname, root):
    # write xml, stripping the namespacing
    out = et.tostring(root)
    out = out.replace("ns0:", "")
    out = out.replace("xmlns:ns0", "xmlns")
    with open(fname, "w") as f:
        f.write(out)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print "Usage: %s <src.xml> <dest.xml>" % sys.argv[0]
        sys.exit(1)

    in_fname = sys.argv[1]
    out_fname = sys.argv[2]

    root = read_xml(in_fname)

    # change some version strings so people know we've been here
    root.find("./generated_by/tool_name").text = "ClmModifier"
    root.find("./table_revision").text += "m"

#    # remove the A locale
#    delete_element(root, get_locale(root, "A"))
#
#    # replace all use of locale A with A-1
#    for e in root.findall("./region_list/region/locale_ref_list/locale_ref"):
#        if e.text == "A":
#            e.text = "A-1"
#
#    # change the power limit to 10 for the entire A-1 channel/rate set that includes channel 1 OFDM
#    get_crp(get_locale(root, "A-1"), 1, "OFDM").find("./power").text = str(10)
#
#    # change the measurement type to eirp for the entire A-1 channel/rate set that includes channel 1 OFDM
#    get_crp(get_locale(root, "A-1"), 1, "OFDM").find("./measure").text = "eirp"
#
#    # change the max power to 31 for the entire A-1 channel set that includes channel 1
#    get_regulatory_power(get_locale(root, "A-1"), 1).find("./power").text = str(31)
#
#    # remove the BN mapping from CN/1
#    delete_element(root, get_mapping(get_aggregate_country(root, "CN", 1), "BN"))
#
#    # change CN/1 AE mapping to rev 0
#    get_mapping(get_aggregate_country(root, "CN", 1), "AE").find("./rev").text = str(0)
#
#    # add a ww/5 mapping to CN/1
#    add_mapping(get_aggregate_country(root, "CN", 1), "ww", 5)
#
#    # remove DE/2 aggregate country
#    delete_element(root, get_aggregate_country(root, "DE", 2))
#
#    # copy ww/8 aggregate country to ww/9
#    copy_aggregate_country(root, get_aggregate_country(root, "ww", 8), "ww", 9)
#
#    # copy region ww/5 to ww/6
#    copy_region(root, get_region(root, "ww", 5), "ww", 6)
#
#    # delete region AC/0
#    delete_element(root, get_region(root, "AC", 0))
#
#    # copy locale A-17 to A-18
#    copy_locale(root, get_locale(root, "A-17"), "A-18")
#
#    # change region AE/0 base_2.4 to locale A-18
#    get_locale_ref(get_region(root, "AE", 0), "base_2.4").text = "A-18"
#
#    # give An1-T1-S3 channel 9 MCS15 its own power
#    crp1 = get_crp(get_locale(root, "An1-T1-S3"), 9, "MCS15") # get the original
#    crp2 = split_crp_by_rate(root, crp1, ["MCS15"]) # split out MCS15
#    crp2.find("./power").text = "13"
#
#    # give A-12 channel 1 DSSS its own power
#    crp1 = get_crp(get_locale(root, "A-12"), 1, "DSSS") # get the original
#    crp2 = split_crp_by_channel(root, crp1, 2) # split off channels 2+
#    crp1.find("./power").text = "17"
#
#    # add MCS15 to A-10 channel 1
#    add_rate(get_crp(get_locale(root, "A-10"), 1, "DSSS"), "MCS15")
#
#    # add An1-T1 to 0A/0
#    add_locale_ref(get_region(root, "0A", 0), "ht_2.4", "An1-T1")
#
#    # change the max power to 31 for just A-10 channels 5-11
#    e = split_regulatory_power(root, get_regulatory_power(get_locale(root, "A-10"), 11), 5)
#    e.find("./power").text = "31"
#
#    # add a new restrict set 11 for just channel 44
#    add_restrict_channels(add_restrict(root, 11, "5"), 44, 44)
#
#    # delete restrict set 7
#    delete_element(root, get_restrict(root, 7))
#
#    # change all use of restrict set 7 to set 11
#    for e in root.findall("./locale_list/*/restricted_set"):
#        if e.text == "7":
#            e.text = "11"
#
#    # change restrict set 4 to include channel 11
#    ((start, end), ) = get_restrict_channels(get_restrict(root, 4))
#    start.text = "11"
#
#    # add channels 5-7 to restrict set 4
#    add_restrict_channels(get_restrict(root, 4), 5, 7)

    write_xml(out_fname, root)




