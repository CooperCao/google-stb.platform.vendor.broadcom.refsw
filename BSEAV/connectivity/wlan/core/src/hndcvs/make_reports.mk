BOM_LIST := hndrte-bom wl-build-bom
INVENTORY_BOM := modules-list-bom
INVENTORY_REPORT := hndcvs_cvsreport.modules-list-bom.$(TAG).$(DATE).html
REPORT_SRC := /projects/hnd/software/cvsreport/Software/$(DATE)_ALL.html

all: $(INVENTORY_BOM) $(BOM_LIST)

$(BOM_LIST): $(INVENTORY_REPORT)
	@echo $(BOM_FILES)
	@echo $@ for $(TAG) at $(DATE)
	hndcvs -l -rpt tmp4 -of hndcvs_cvsreport.$@.$(TAG).$(DATE).html $@ $(TAG)
	[ ! -e hndcvs_cvsreport.$@.$(TAG).$(DATE).html ] ||  \
	sed '41i  <a href="$(PWD)/hndcvs_cvsreport.$@.$(TAG).$(DATE).html" >  $@ <br> <br>' tmp2 > tmp; \
	cp tmp tmp2; \
	sed '41i  <b> Logs for $@ : </b> <br><br>'  hndcvs_cvsreport.$@.$(TAG).$(DATE).html > tmp3; \
	cp tmp3 hndcvs_cvsreport.$@.$(TAG).$(DATE).html

$(INVENTORY_BOM):
	@echo $@ for $(TAG) at $(DATE) in $(PWD)
	hndcvs -rpt $(REPORT_SRC) $@ $(TAG)
	[ ! -e $(INVENTORY_REPORT) ] ||  cp $(INVENTORY_REPORT)  tmp2; cp tmp2  tmp4;  make $(BOM_LIST)
	[ ! -e tmp ] || sed '41i <b> Modified BOMs : </b> <br><br>'  tmp > $(INVENTORY_REPORT) ; rm tmp 


$(INVENTORY_REPORT):
