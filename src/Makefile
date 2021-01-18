# Copyright © 2020 Miguel González Cuadrado <mgcuadrado@gmail.com>

# This file is part of Testudo.

#     Testudo is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.

#     Testudo is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.

#     You should have received a copy of the GNU General Public License
#     along with Testudo.  If not, see <http://www.gnu.org/licenses/>.

SHELL= /bin/bash

doc: testudo.pdf

BINSDIR := .
HEADERSDIR := .headers
TESTUDOBIN = $(OBJDIR)/$(TESTUDONAME)
STYLES := $(wildcard *.tst)
STYLE_HEADERS := $(STYLES:%.tst=testudo_%)

XML_TO_COLOR_XSLT = $(BINSDIR)/xml_to_color
XML_TO_COLOR_SUMMARY_XSLT = $(BINSDIR)/xml_to_color -s

TTD_ADDITIONAL_PREREQUISITES = \
	$(STYLE_HEADERS) mock_turtle_macro_n.gh $(HEADERSDIR)

O_ADDITIONAL_PREREQUISITES := $(TTD_ADDITIONAL_PREREQUISITES)

include Makefile.testudo

testudo.pdf: testudo.tex ascii_logo_colour_cropped.pdf version \
		reports/testudo.use_instructions.tex \
		style_table.tex keywords.tex
	rubber -d testudo

style_table.tex: generate_LaTeX_style_table lc.tst uc.tst
	./$^ > $@

keywords.tex: generate_keywords lc.tst uc.tst eo.tst
	./$^ > $@

emacs_add_keywords_%.txt: generate_emacs_add_keywords %.tst
	./$^ > $@

ascii_logo_colour.pdf \
		ascii_logo_mig_colour.pdf \
		testudo_gaming_colour.pdf \
		testudo_kavalir_colour.pdf \
		dune_buggy_logo.pdf: %.pdf: %.tex
	rubber -m xelatex $<

ascii_logo_colour_cropped.pdf \
		ascii_logo_mig_colour_cropped.pdf \
		testudo_gaming_colour_cropped.pdf \
		testudo_kavalir_colour_cropped.pdf \
		dune_buggy_logo_cropped.pdf: %_cropped.pdf: %.pdf
	pdfcrop $< $@

clean_doc:
	rubber -d --clean testudo
	rubber -m xelatex --clean ascii_logo_colour.tex
	rm -f ascii_logo_colour_cropped.pdf style_table.tex keywords.tex
	rm -f emacs_add_keywords_*.txt

LINKOPTS = $(COMPILEOPTS) $(LIBRARYOPTS) $(COMMONLIBRARIES)

LIBRARYOPTS :=

# i think, from my experiments, that "-rdynamic" is needed for "dlopen()" to
# initialise static objects like objects automatically created by macros for
# test nodes and tests; dynamic libraries must be created with "-fPIC -shared"
COMMONLIBRARIES= -rdynamic -lm -ldl -pthread

PRINTDIR := .print
OBJDIR := .object
ALLHEADERS := $(ALLHEADERS) mock_turtle_macro_n.gh

TESTUDOBIN := .object/$(TESTUDONAME)
TESTUDOLIB := .object/lib$(TESTUDONAME).so

ALLDUNEBUGGYDIRS := $(wildcard dune_buggy_*_*)
ALLDUNEBUGGYOBJECTS := $(ALLDUNEBUGGYDIRS:%=$(TESTUDOBINDIR)/%/dune_buggy.o)
ALLDUNEBUGGYBINS := $(ALLDUNEBUGGYDIRS:%=$(TESTUDOBINDIR)/%/dune_buggy.ttb)
.SECONDARY: $(ALLDUNEBUGGYOBJECTS) $(ALLDUNEBUGGYBINS)

TRACKDIR := .track


check_variables:
	echo $(ALLOBJECTS)

$(STYLE_HEADERS): testudo_%: %.tst generate_style
	./generate_style $<

mock_turtle_macro_n_customize.m4:
	echo "define(\`max_n', 32)\`'dnl" > $@

mock_turtle_macro_n.gh: mock_turtle_macro_n.m4 mock_turtle_macro_n_customize.m4
	m4 $< > $@

$(HEADERSDIR):
	mkdir $(HEADERSDIR)
	ln -s .. $(HEADERSDIR)/testudo

# DuneBuggy object files are treated as ".ttb" files, for convenience
$(TESTUDOBINDIR)/%.o: %.cpp $(O_ADDITIONAL_PREREQUISITES)
	@ mkdir -p $(DEPDIR)/$(dir $<) $(dir $@)
	@ echo compiling $@
	@ $(GPP) -Wp,-MMD,$(DEPDIR)/$*.d,-MP,-MT,$@ $(CFLAGS) $(COMPILEOPTS) \
		-x c++ -fPIC -shared $< -o $@

$(TESTUDOLIB): $(filter-out %/testudo_main.o, $(ALLOBJECTS))
	@ echo linking $@
	@ $(GPP) $(CFLAGS) $(COMPILEOPTS) -fPIC -shared $^ -o $@

$(TESTUDOBIN): $(ALLOBJECTS)
	@ echo linking $@
	@ $(GPP) -Wp,-MMD,$(DEPDIR)/$*.d,-MP,-MT,$@ $(CFLAGS) \
		$^ $(LINKOPTS) $(LDFLAGS) -o $@

test: $(TESTUDOBIN)

clean_exe:
	rm -rf $(HEADERSDIR) $(PRINTDIR)
	rm -f $(STYLE_HEADERS)
	rm -f mock_turtle_macro_n.gh mock_turtle_macro_n_customize.m4

diff_xml_text_test_xslt: $(TESTUDOBIN) $(ALLTESTUDOBINS)
	diff -u \
	    <($< run $(XML_FORMAT) $(TESTUDOOPT) $(ALLTESTUDOSPEC) \
		  | $(XML_TO_COLOR)) \
	    <($< run $(XML_FORMAT) $(TESTUDOOPT) $(ALLTESTUDOSPEC) \
		  | $(XML_TO_COLOR_XSLT))
diff_xml_text_test_bw: $(TESTUDOBIN) $(ALLTESTUDOBINS)
	diff -u \
	    <($< run $(XML_FORMAT) $(TESTUDOOPT) $(ALLTESTUDOSPEC) \
		  | $(XML_TO_TEXT)) \
	    <($< run $(SYNC_COLOR_TEXT_FORMAT) $(TESTUDOOPT) \
			$(ALLTESTUDOSPEC) \
		  | $(UNCOLOR))
diff_xml_text_test_color: $(TESTUDOBIN) $(ALLTESTUDOBINS)
	diff -u \
	    <($< run $(XML_FORMAT) $(TESTUDOOPT) $(ALLTESTUDOSPEC) \
		  | $(XML_TO_COLOR)) \
	    <($< run $(SYNC_COLOR_TEXT_FORMAT) $(TESTUDOOPT) $(ALLTESTUDOSPEC))
diff_xml_text_test_color_ansi: $(TESTUDOBIN) $(ALLTESTUDOBINS)
	diff -u \
	    <($< run $(XML_FORMAT) $(TESTUDOOPT) $(ALLTESTUDOSPEC) \
		  | $(XML_TO_COLOR) | ./show_ansi) \
	    <($< run $(SYNC_COLOR_TEXT_FORMAT) $(TESTUDOOPT) \
			$(ALLTESTUDOSPEC) \
		  | ./show_ansi)
diff_xml_summary_test_xslt: $(TESTUDOBIN) $(ALLTESTUDOBINS)
	diff -u \
	    <($< run $(XML_FORMAT) $(TESTUDOOPT) $(ALLTESTUDOSPEC) \
		  | $(XML_TO_COLOR_SUMMARY)) \
	    <($< run $(XML_FORMAT) $(TESTUDOOPT) $(ALLTESTUDOSPEC) \
		  | $(XML_TO_COLOR_SUMMARY_XSLT))

reports/%.tex: $(TESTUDOBIN) xml_to_color $(TESTUDOBINDIR)/testudo_doc.ttb
	mkdir -p reports
	$< run $(XML_FORMAT) $(TESTUDOBINDIR)/testudo_doc.ttb -s $* \
	    | $(XML_TO_COLOR_LATEX) > $@


run_dune_buggy_test_%: $(TESTUDOBIN) \
			$(TESTUDOBINDIR)/dune_buggy_%/dune_buggy.o \
		$(TESTUDOBINDIR)/dune_buggy_%/dune_buggy.ttb
	$< run $(SYNC_COLOR_TEXT_WITH_LINES_FORMAT) $(TESTUDOOPT) \
	    -d dune_buggy_$* $(filter-out $<,$^) \
	    | tee $(TRACKDIR)/dune_buggy_$*/report
	mkdir -p $(TRACKDIR)/dune_buggy_$*
	$< run $(TRACK_FORMAT) $(TESTUDOOPT) $(filter-out $<,$^) \
	    -d dune_buggy_$* > $(TRACKDIR)/dune_buggy_$*/track

diff_dune_buggy_test: $(TESTUDOBIN) \
			run_dune_buggy_test_0_0 run_dune_buggy_test_0_1
	$(TESTUDOBIN) diff \
		.track/dune_buggy_0_0/track .track/dune_buggy_0_1/track

ICC:=/usr/share/color/icc/colord/sRGB.icc

ascii_logo.png \
		ascii_logo_mig.png \
		testudo_gaming.png \
		testudo_kavalir.png: %.png: %_colour_cropped.pdf
	pdftocairo $< -png -singlefile -r 720 -icc $(ICC) $*

clean_ascii_logo:
	rm -f ascii_logo.png

clean_report:
	rm -rf reports

clean:: clean_doc clean_exe clean_report clean_ascii_logo

diff_tests: diff_test \
		diff_xml_text_test_xslt \
		diff_xml_text_test_bw \
		diff_xml_text_test_color \
		diff_xml_summary_test_xslt

PREFIX?=$(HOME)/local
BIN_DIR=$(PREFIX)/bin
LIB_DIR=$(PREFIX)/lib
INCLUDE_DIR=$(PREFIX)/include
TESTUDO_INCLUDE_DIR=$(INCLUDE_DIR)/testudo

INSTALL_SCRIPT := no_loc_info show_ansi uncolor xml_to_color generate_style
INSTALL_XSLT := $(wildcard *.xslt)
INSTALL_HEADERS := $(shell ./headers \
				testudo_lc \
				testudo_uc \
				testudo_ext.h \
				testudo_opt.h \
				testudo_try_catch.h) \
			lc.tst uc.tst
TESTUDO_MAKEFILE := Makefile.testudo
TEMPLATE_MAKEFILE := Makefile.template

cat_install_params:
	@echo BIN_DIR: $(BIN_DIR)
	@echo INCLUDE_DIR: $(INCLUDE_DIR)
	@echo TESTUDO_INCLUDE_DIR: $(TESTUDO_INCLUDE_DIR)

install: $(TESTUDOBIN) $(TESTUDOLIB) $(INSTALL_HEADERS)
	mkdir -p $(TESTUDO_INCLUDE_DIR)
	cp -f $(INSTALL_HEADERS) $(TEMPLATE_MAKEFILE) $(TESTUDO_INCLUDE_DIR)
	cp -f $(TESTUDO_MAKEFILE) $(TESTUDO_INCLUDE_DIR)/Makefile
	mkdir -p $(BIN_DIR)
	cp -f $(TESTUDOBIN) $(INSTALL_SCRIPT) $(INSTALL_XSLT) $(BIN_DIR)
	mkdir -p $(LIB_DIR)
	cp -f $(TESTUDOLIB) $(LIB_DIR)

uninstall:
	for f in $(TESTUDO_INCLUDE_DIR) \
		$(BIN_DIR)/$(TESTUDONAME) \
		$(INSTALL_SCRIPT:%=$(BIN_DIR)/%) \
		$(INSTALL_XSLT:%=$(BIN_DIR)/%) \
		$(LIB_DIR)/$(TESTUDOLIB) \
		; do \
	    if [ -e $$f ]; then \
		rm -ri $$f; \
	    fi; \
	done
