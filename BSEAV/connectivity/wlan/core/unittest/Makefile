# Package make commands now moved into UTF.tcl

.PHONY: pkgIndex.tcl etags tags TAGS id ID clean keywords

all: keywords pkgIndex.tcl

keywords:
	bin/svn_missing_keywords.sh

pkgIndex.tcl:
	bin/pkg_mkindex.tcl

#
#  Rules for gnu utils and emacs support -  makes TAGS and ID index files
#
ETAGS=etags.emacs
GNU_MKID=mkid
GNU_ID_OPTIONS= --lang-map="./id-lang.map" --lang-option="text:--include=.:"

etags tags TAGS:
	$(ETAGS) --lang=none --regex='/proc[ \t]+\([^ \t]+\)/\1/' *.tcl; \
	$(ETAGS) --lang=none --regex='/[ \t]+method[ \t]+\([^ \t]+\) /\1/' -a *.tcl; \
	$(ETAGS) --lang=none --regex='/proc[ \t]+\([^ \t]+\)/\1/' -a UTF/*.tcl ; \
	$(ETAGS) --lang=none --regex='/[ \t]+method[ \t]+\([^ \t]+\) /\1/' -a UTF/*.tcl ; \
	$(ETAGS) --lang=none --regex='/proc[ \t]+\([^ \t]+\)/\1/' -a Test/*.test; \
	$(ETAGS) --lang=none --regex='/UTF::Test[ \t]+\([^ \t]+\)/\1/' -a Test/*.test; \
	$(ETAGS) --lang=none --regex='/[ \t]+proc[ \t]+\([^ \t]+\)/\1/' -a Test/*.test; \
	$(ETAGS) --lang=none --regex='/snit::type[ \t]+\([^ \t]+\)/\1/' -a *.tcl; \
	$(ETAGS) --lang=none --regex='/snit::type[ \t]+\([^ \t]+\)/\1/' -a UTF/*.tcl; \
	$(ETAGS) --lang=none --regex='/snit::type[ \t]+\([^ \t]+\)/\1/' -a Test/*.test; \
	$(ETAGS) --lang=none --regex='/[^ \t]::[^ \t][ \t]+\([^ \t]+\)[ \t]+/\1/' -a Test/*.test; 

id ID:	
	$(GNU_MKID) $(GNU_ID_OPTIONS) .*

clean:
	rm -f ID TAGS pkgIndex.tcl

