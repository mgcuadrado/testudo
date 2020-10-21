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
.PHONY: clean

testudo.pdf: testudo.tex ascii_logo_colour_cropped.pdf version \
		reports/testudo.use_instructions.tex \
		style_table.tex keywords.tex
	rubber -d testudo

style_table.tex: generate_LaTeX_style_table lc.tst uc.tst
	./$^ > $@

keywords.tex: generate_keywords lc.tst uc.tst
	./$^ > $@

emacs_add_keywords_%.txt: generate_emacs_add_keywords %.tst
	./$^ > $@

ascii_logo_colour.pdf dune_buggy_logo.pdf: %.pdf: %.tex
	rubber -m xelatex $<

ascii_logo_colour_cropped.pdf dune_buggy_logo_cropped.pdf: %_cropped.pdf: %.pdf
	pdfcrop $< $@

clean_doc:
	rubber -d --clean testudo
	rubber -m xelatex --clean ascii_logo_colour.tex
	rm -f ascii_logo_colour_cropped.pdf style_table.tex keywords.tex
	rm -f emacs_add_keywords_*.txt

GPP= g++-10 -std=c++17 -pedantic

COMPILEOPTS = $(BASICOPTS) $(CODEOPTS) $(WARNINGOPTS) $(INCLUDEOPTS)

LINKOPTS = $(COMPILEOPTS) $(LIBRARYOPTS) $(COMMONLIBRARIES)

BASICOPTS := -ggdb3
CODEOPTS :=
INCLUDEOPTS :=
LIBRARYOPTS :=
# i think, from my experiments, that "-rdynamic" is needed for "dlopen()" to
# initialise static objects like objects automatically created by macros for
# test nodes and tests; dynamic libraries must be created with "-fPIC -shared"
COMMONLIBRARIES= -rdynamic -lm -ldl -pthread

CFLAGS :=
LDFLAGS :=

WARNINGOPTS += \
	-Wall -Wextra \
	-Wno-ctor-dtor-privacy -Wnon-virtual-dtor \
	-Wno-ignored-qualifiers \
	-Wreorder \
	-Wold-style-cast -Wsign-promo \
	-Woverloaded-virtual -Wfloat-equal -Wpointer-arith -Wcast-qual \
	-Wcast-align -Wwrite-strings -Wconversion \
	-Wdisabled-optimization \
	-Werror \
	-Wundef -Wpointer-arith \
	-Wcast-qual -Wsign-compare \
	-Wmissing-field-initializers \
	-Winvalid-pch -Wstrict-null-sentinel \
	-Winit-self -Wswitch-default \
	-Wunused-parameter -Waddress \
	-Wno-noexcept-type


OBJDIR := .object
DEPDIR := .depend
ALLHEADERS := $(wildcard *.h) mock_turtle_macro_n.h
ALLSOURCES := $(wildcard *.cpp)
ALLOBJECTS := $(ALLSOURCES:%.cpp=$(OBJDIR)/%.o)
ALLDEPENDS := $(ALLSOURCES:%.cpp=$(DEPDIR)/%.d)
TEST := $(OBJDIR)/ttest

TESTUDOBINDIR   := .testudo
ALLTESTUDOTESTS := $(wildcard *.ttd)
ALLTESTUDOBINS  := $(ALLTESTUDOTESTS:%.ttd=$(TESTUDOBINDIR)/%.ttb)
ALLTESTUDODEPENDS := $(ALLTESTUDOTESTS:%.ttd=$(DEPDIR)/%.d)
ALLTESTUDOOPTS  := $(ALLTESTUDOBINS:%=%)

STYLES := $(wildcard *.tst)
STYLE_HEADERS := $(STYLES:%.tst=testudo_%.h)


check_variables:
	echo $(ALLOBJECTS)

$(STYLE_HEADERS): testudo_%.h: %.tst generate_style
	./generate_style $<

mock_turtle_macro_n_customize.m4:
	echo "define(\`max_n', 32)\`'dnl" > $@

mock_turtle_macro_n.h: mock_turtle_macro_n.m4 mock_turtle_macro_n_customize.m4
	m4 $< > $@

$(OBJDIR)/%.o: %.cpp $(STYLE_HEADERS) mock_turtle_macro_n.h
	@ mkdir -p $(DEPDIR) $(OBJDIR)
	@ echo compiling $@
	@ $(GPP) -Wp,-MMD,$(DEPDIR)/$*.d,-MP,-MT,$@ \
		-c $(CFLAGS) $(COMPILEOPTS) \
		-fPIC $< -o $@

$(TESTUDOBINDIR)/%.ttb: %.ttd $(STYLE_HEADERS) mock_turtle_macro_n.h
	@ mkdir -p $(DEPDIR) $(TESTUDOBINDIR)
	@ echo compiling $@
	@ $(GPP) -Wp,-MMD,$(DEPDIR)/$*.d,-MP,-MT,$@ $(CFLAGS) $(COMPILEOPTS) \
		-x c++ -fPIC -shared $< -o $@

$(TEST): $(ALLOBJECTS)
	@ echo linking $@
	@ $(GPP) -Wp,-MMD,$(DEPDIR)/$*.d,-MP,-MT,$@ $(CFLAGS) \
		$^ $(LINKOPTS) $(LDFLAGS) -o $@

test: $(TEST)

-include $(ALLDEPENDS)

-include $(ALLTESTUDODEPENDS)

clean_exe:
	rm -rf $(OBJDIR) $(DEPDIR) $(TESTUDOBINDIR)
	rm -f $(STYLE_HEADERS)
	rm -f mock_turtle_macro_n.h mock_turtle_macro_n_customize.m4

XML_TO_COLOR := ./xml_to_color
XML_TO_COLOR_SUMMARY := ./xml_to_color -s
XML_TO_COLOR_LATEX := ./xml_to_color -l -w 63
UNCOLOR := ./uncolor
XML_TO_TEXT := ./xml_to_color -b
XML_FORMAT := -f xml
SYNC_COLOR_TEXT_FORMAT := -f color_text
SUCCESS_FILE := ttest.output.success

run_test: $(TEST) $(ALLTESTUDOBINS)
	$< $(XML_FORMAT) $(ALLTESTUDOOPTS) | $(XML_TO_COLOR)
runsync_test: $(TEST) $(ALLTESTUDOBINS)
	$< $(SYNC_COLOR_TEXT_FORMAT) $(ALLTESTUDOOPTS)
summary_test: $(TEST) $(ALLTESTUDOBINS)
	$< $(XML_FORMAT) $(ALLTESTUDOOPTS) | $(XML_TO_COLOR_SUMMARY)
view_test: $(TEST) $(ALLTESTUDOBINS)
	$< $(XML_FORMAT) $(ALLTESTUDOOPTS) | $(XML_TO_COLOR) | less -R
diff_test: $(TEST) $(ALLTESTUDOBINS)
	diff -bwu <(cat $(SUCCESS_FILE) | $(XML_TO_TEXT)) \
		<($< $(XML_FORMAT) $(ALLTESTUDOOPTS) | $(XML_TO_TEXT))
diffxml_test: $(TEST) $(ALLTESTUDOBINS)
	diff -bwu <(cat $(SUCCESS_FILE)) <($< $(XML_FORMAT) $(ALLTESTUDOOPTS))
success_test: $(TEST) $(ALLTESTUDOBINS)
	cp -i <($< $(XML_FORMAT) $(ALLTESTUDOOPTS)) $(SUCCESS_FILE)

diff_xml_text_test_bw: $(TEST) $(ALLTESTUDOBINS)
	diff -u <($< $(XML_FORMAT) $(ALLTESTUDOOPTS) | $(XML_TO_TEXT)) \
		<($< $(SYNC_COLOR_TEXT_FORMAT) $(ALLTESTUDOOPTS) | $(UNCOLOR))
diff_xml_text_test_color: $(TEST) $(ALLTESTUDOBINS)
	diff -u <($< $(XML_FORMAT) $(ALLTESTUDOOPTS) | $(XML_TO_COLOR)) \
		<($< $(SYNC_COLOR_TEXT_FORMAT) $(ALLTESTUDOOPTS))
diff_xml_text_test_color_ansi: $(TEST) $(ALLTESTUDOBINS)
	diff -u <($< $(XML_FORMAT) $(ALLTESTUDOOPTS) | $(XML_TO_COLOR) \
			| ./show_ansi) \
		<($< $(SYNC_COLOR_TEXT_FORMAT) $(ALLTESTUDOOPTS) \
			| ./show_ansi)

reports/%.tex: $(TEST) xml_to_color $(TESTUDOBINDIR)/testudo_doc.ttb
	mkdir -p reports
	$< $(XML_FORMAT) $(TESTUDOBINDIR)/testudo_doc.ttb -s $* \
	    | $(XML_TO_COLOR_LATEX) > $@

ascii_logo.png: ascii_logo_colour_cropped.pdf
	pdftocairo $< -png -singlefile -r 720 ascii_logo

clean_ascii_logo:
	rm -f ascii_logo.png

clean_report:
	rm -rf reports

clean: clean_doc clean_exe clean_report clean_ascii_logo

diff_tests: diff_test diff_xml_text_test_bw diff_xml_text_test_color
