# Copyright © 2020-2023 Miguel González Cuadrado <mgcuadrado@gmail.com>

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

COLOR_TO_LATEX = ./color_to_latex
XML_TO_TEXT_LATEX = $(TESTUDOBIN) xml_to_color -w 63 | $(COLOR_TO_LATEX)
XML_TO_TEXT_NL = $(TESTUDOBIN) xml_to_color -w 63 -n
XML_TO_TEXT_LATEX_NL = $(XML_TO_TEXT_NL) | $(COLOR_TO_LATEX)
COLOR_NO_LOC_INFO = ./color_no_loc_info
UNCOLOR = ./uncolor

TTD_ADDITIONAL_PREREQUISITES = \
	$(STYLE_HEADERS) testudo_macros_n.gh $(HEADERSDIR)

O_ADDITIONAL_PREREQUISITES := $(TTD_ADDITIONAL_PREREQUISITES)

include Makefile.testudo

EXAMPLESDIR := examples
EXAMPLESTESTUDOBINDIR := $(EXAMPLESDIR)/$(TESTUDOBINDIR)
EXAMPLESDEPDIR := $(EXAMPLESDIR)/$(DEPDIR)
REPORTSDIR := reports

$(EXAMPLESDIR)/doc_examples_op.ttd: doc_examples.ttd
	@ mkdir -p $(EXAMPLESDIR)
	cp $< $@

$(EXAMPLESDIR)/doc_examples_cl.ttd: doc_examples.ttd
	@ mkdir -p $(EXAMPLESDIR)
	cat $< | ./op_to_cl > $@

$(EXAMPLESTESTUDOBINDIR)/%.ttb: $(EXAMPLESDIR)/%.ttd \
		$(ALLHEADERS) $(TTD_ADDITIONAL_PREREQUISITES)
	@ mkdir -p $(EXAMPLESTESTUDOBINDIR) $(EXAMPLESDEPDIR)
	@ echo compiling $@
	@ $(GPP) -Wp,-MMD,$(EXAMPLESDEPDIR)/$*.td,-MP,-MT,$@ \
		$(CFLAGS) $(COMPILEOPTS) \
		-x c++ -I$(HEADERSDIR) -fPIC -shared $< -o $@

-include $(EXAMPLESDEPDIR)/doc_examples_cl.td
-include $(EXAMPLESDEPDIR)/doc_examples_op.td

# we can't split "examples_generated" into a closed version and an open
# version, because the guide includes both "styles-lc-cl.tex" and
# "styles-uc-op.tex" for all versions
$(EXAMPLESDIR)/examples_generated: \
		$(EXAMPLESDIR)/doc_examples_op.ttd \
		$(EXAMPLESDIR)/doc_examples_cl.ttd \
		generate_examples
	@ mkdir -p $(EXAMPLESDIR)
	./generate_examples $(EXAMPLESDIR) > $@

$(REPORTSDIR)/reports_generated: \
	$(REPORTSDIR)/reports_generated_cl \
	$(REPORTSDIR)/reports_generated_op
	cat $^ > $@

$(REPORTSDIR)/reports_generated_lc_cl: $(REPORTSDIR)/reports_generated_cl

$(REPORTSDIR)/reports_generated_uc_op: $(REPORTSDIR)/reports_generated_op

$(REPORTSDIR)/reports_generated_%: \
		$(EXAMPLESTESTUDOBINDIR)/doc_examples_%.ttb $(TESTUDOBIN) \
		generate_example_reports
	@ mkdir -p $(REPORTSDIR)
	./generate_example_reports \
		"$(TESTUDOBIN) run $(XML_FORMAT) $<" $(REPORTSDIR) \
		$* "$(XML_TO_TEXT_NL)" "$(COLOR_TO_LATEX)" > $@

clean_examples:
	rm -rf $(EXAMPLESDIR)
clean_reports:
	rm -rf $(REPORTSDIR)

RUBBERDIR := rubbertmp

MANUALS := $(wildcard testudo*.tex)
PDFMANUALS := $(MANUALS:%.tex=%.pdf)

all_manuals: $(PDFMANUALS)

$(PDFMANUALS): testudo%.pdf: testudo%.tex testudo.tex the_race.pdf \
		ascii_logo_colour_cropped.pdf\
		ascii_logo_mig_colour_cropped.pdf \
		version \
		$(REPORTSDIR)/testudo.use_instructions.tex \
		style_table.tex keywords.tex copyright.txt \
		$(EXAMPLESDIR)/examples_generated \
		$(REPORTSDIR)/reports_generated%
	@ mkdir -p $(RUBBERDIR)
	rubber --force \
		--jobname $(RUBBERDIR)/$(basename $@) --pdf $(basename $@)
	mv $(RUBBERDIR)/$@ $@

style_table.tex: generate_LaTeX_style_table generate_style lc.tst uc.tst
	./$< lc.tst uc.tst > $@

keywords.tex: generate_keywords lc.tst uc.tst eo.tst
	./$^ > $@

emacs_add_keywords_%.txt: generate_emacs_add_keywords %.tst
	./$^ > $@

the_race.pdf: %.pdf: %.tex %.txt
	@ mkdir -p $(RUBBERDIR)
	rubber -m xelatex --jobname $(RUBBERDIR)/$* $*
	mv $(RUBBERDIR)/$@ $@

LOGOS := $(wildcard *_logo*.tex)
PDFLOGOS := $(LOGOS:%.tex=%.pdf)
CROPPEDPDFLOGOS := $(PDFLOGOS:%.pdf=%_cropped.pdf)

$(PDFLOGOS): %.pdf: %.tex
	@ mkdir -p $(RUBBERDIR)
	rubber -m xelatex --jobname $(RUBBERDIR)/$* $*
	mv $(RUBBERDIR)/$@ $@

$(CROPPEDPDFLOGOS): %_cropped.pdf: %.pdf
	pdfcrop $< $@

clean_doc:
	rm -f testudo.pdf
	rm -rf $(RUBBERDIR)
	rm -f $(PDFLOGOS) $(CROPPEDPDFLOGOS)
	rm -f style_table.tex keywords.tex
	rm -f emacs_add_keywords_*.txt

LINKOPTS = $(COMPILEOPTS) $(LIBRARYOPTS) $(COMMONLIBRARIES)

LIBRARYOPTS :=

# i think, from my experiments, that "-rdynamic" is needed for "dlopen()" to
# initialise static objects like objects automatically created by macros for
# test nodes and tests; dynamic libraries must be created with "-fPIC -shared"
COMMONLIBRARIES= -rdynamic -lm -ldl -pthread

PRINTDIR := .print
OBJDIR := .object
ALLHEADERS := $(ALLHEADERS) testudo_macros_n.gh

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

testudo_macros_n_customize.m4:
	echo "define(\`max_n', 32)\`'dnl" > $@

testudo_macros_n.gh: testudo_macros_n.m4 testudo_macros_n_customize.m4
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
	rm -f testudo_macros_n.gh testudo_macros_n_customize.m4

XML_TO_TEXT_REPEATABLE = $(TESTUDOBIN) xml_to_color -w 78
XML_TO_TEXT_SUMMARY_REPEATABLE = $(XML_TO_TEXT_REPEATABLE) -s

dev_diff_xml_text_test_bw: $(TEST_PREREQUISITES)
	diff -u \
	    <($< run $(XML_FORMAT) $(TESTUDOCONF) $(TESTUDOBINS) \
		  | $(XML_TO_TEXT_REPEATABLE) $(BW)) \
	    <($< run $(NO_XML_TEXT_FORMAT) $(TESTUDOCONF) \
			$(TESTUDOBINS) \
		  | $(UNCOLOR))
dev_diff_xml_text_test_color: $(TEST_PREREQUISITES)
	diff -u \
	    <($< run $(XML_FORMAT) $(TESTUDOCONF) $(TESTUDOBINS) \
		  | $(XML_TO_TEXT)) \
	    <($< run $(NO_XML_TEXT_FORMAT) $(TESTUDOCONF) $(TESTUDOBINS))
dev_diff_xml_text_test_color_ansi: $(TEST_PREREQUISITES)
	diff -u \
	    <($< run $(XML_FORMAT) $(TESTUDOCONF) $(TESTUDOBINS) \
		  | $(XML_TO_TEXT) | ./show_ansi) \
	    <($< run $(NO_XML_TEXT_FORMAT) $(TESTUDOCONF) \
			$(TESTUDOBINS) \
		  | ./show_ansi)
dev_diff_xml_text_test_saved_color: $(TEST_PREREQUISITES)
	diff -u \
	    <(cat testudo.report.saved | $(COLOR_NO_LOC_INFO)) \
	    <($< run $(XML_FORMAT) $(TESTUDOCONF) $(TESTUDOBINS) \
		  | $(XML_TO_TEXT_REPEATABLE) | $(COLOR_NO_LOC_INFO))
dev_diff_xml_summary_test_saved_color: $(TEST_PREREQUISITES)
	diff -u \
	    testudo.summary.saved \
	    <($< run $(XML_FORMAT) $(TESTUDOCONF) $(TESTUDOBINS) \
		  | $(XML_TO_TEXT_SUMMARY_REPEATABLE))

dev_save_color: $(TEST_PREREQUISITES)
	cp -i \
	    <($< run $(XML_FORMAT) $(TESTUDOCONF) $(TESTUDOBINS) \
		  | $(XML_TO_TEXT_REPEATABLE)) \
	    testudo.report.saved
dev_save_summary_color: $(TEST_PREREQUISITES)
	cp -i \
	    <($< run $(XML_FORMAT) $(TESTUDOCONF) $(TESTUDOBINS) \
		  | $(XML_TO_TEXT_SUMMARY_REPEATABLE)) \
	    testudo.summary.saved

dev_diff_tests: dev_diff_xml_text_test_bw \
		dev_diff_xml_text_test_color \
		dev_diff_xml_text_test_saved_color \
		dev_diff_xml_summary_test_saved_color \
		progress

dev_save_tests: dev_save_color dev_save_summary_color save_progress


$(REPORTSDIR)/testudo.use_instructions.tex: $(REPORTSDIR)/%.tex: \
		$(TESTUDOBIN) $(TESTUDOBINDIR)/testudo_doc.ttb color_to_latex
	@ mkdir -p $(REPORTSDIR)
	$< run $(XML_FORMAT) $(TESTUDOBINDIR)/testudo_doc.ttb -s $* \
	    | $(XML_TO_TEXT_LATEX) > $@

run_dune_buggy_test_%: $(TESTUDOBIN) \
			$(TESTUDOBINDIR)/dune_buggy_%/dune_buggy.o \
		$(TESTUDOBINDIR)/dune_buggy_%/dune_buggy.ttb
	$< run $(NO_XML_COLOR_TEXT_WITH_LINES_FORMAT) $(TESTUDOCONF) \
	    -d dune_buggy_$* $(filter-out $<,$^) \
	    | tee $(TRACKDIR)/dune_buggy_$*/report
	@ mkdir -p $(TRACKDIR)/dune_buggy_$*
	$< run $(TRACK_FORMAT) $(TESTUDOCONF) $(filter-out $<,$^) \
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

clean:: clean_doc clean_exe clean_reports clean_ascii_logo clean_examples

PREFIX?=$(HOME)/local
BIN_DIR=$(PREFIX)/bin
LIB_DIR=$(PREFIX)/lib
INCLUDE_DIR=$(PREFIX)/include
TESTUDO_INCLUDE_DIR=$(INCLUDE_DIR)/testudo

INSTALL_SCRIPT := no_loc_info show_ansi uncolor generate_style
INSTALL_HEADERS = $(shell ./headers \
				testudo_lc \
				testudo_uc \
				testudo_opt.h \
				testudo_try_catch.h) \
			lc.tst uc.tst
TESTUDO_MAKEFILE := Makefile.testudo
TEMPLATE_MAKEFILE := Makefile.template

cat_install_params:
	@echo BIN_DIR: $(BIN_DIR)
	@echo INCLUDE_DIR: $(INCLUDE_DIR)
	@echo TESTUDO_INCLUDE_DIR: $(TESTUDO_INCLUDE_DIR)

install: $(TESTUDOBIN) $(TESTUDOLIB)
	mkdir -p $(TESTUDO_INCLUDE_DIR)
	cp -f $(INSTALL_HEADERS) $(TEMPLATE_MAKEFILE) $(TESTUDO_INCLUDE_DIR)
	cp -f $(TESTUDO_MAKEFILE) $(TESTUDO_INCLUDE_DIR)/Makefile
	mkdir -p $(BIN_DIR)
	cp -f $(TESTUDOBIN) $(INSTALL_SCRIPT) $(BIN_DIR)
	mkdir -p $(LIB_DIR)
	cp -f $(TESTUDOLIB) $(LIB_DIR)

uninstall:
	for f in $(TESTUDO_INCLUDE_DIR) \
		$(BIN_DIR)/$(TESTUDONAME) \
		$(INSTALL_SCRIPT:%=$(BIN_DIR)/%) \
		$(LIB_DIR)/$(TESTUDOLIB) \
		; do \
	    if [ -e $$f ]; then \
		rm -ri $$f; \
	    fi; \
	done
