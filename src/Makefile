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

testudo.pdf: testudo.tex ascii_logo_colour_cropped.pdf \
		reports/testudo.use_instructions.tex
	rubber -d testudo

ascii_logo_colour.pdf: %.pdf: %.tex
	rubber -m xelatex $<

ascii_logo_colour_cropped.pdf: %_cropped.pdf: %.pdf
	pdfcrop $< $@

clean_doc:
	rubber -d --clean testudo
	rubber -m xelatex --clean ascii_logo_colour.tex
	rm -f ascii_logo_colour_cropped.pdf

GPP= g++-10 -std=c++17 -pedantic

COMPILEOPTS = $(BASICOPTS) $(CODEOPTS) $(WARNINGOPTS) $(INCLUDEOPTS)

LINKOPTS = $(COMPILEOPTS) $(LIBRARYOPTS) $(COMMONLIBRARIES)

BASICOPTS := -ggdb3
CODEOPTS :=
INCLUDEOPTS :=
LIBRARYOPTS :=
COMMONLIBRARIES= -lm -ldl -pthread

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
ALLHEADERS := $(wildcard *.h)
ALLSOURCES := $(wildcard *.cpp)
ALLOBJECTS := $(ALLSOURCES:%.cpp=$(OBJDIR)/%.o)
ALLDEPENDS := $(ALLSOURCES:%.cpp=$(DEPDIR)/%.d)
TEST := $(OBJDIR)/ttest

STYLES := $(wildcard *.tst)
STYLE_HEADERS := $(STYLES:%.tst=testudo_%.h)


check_variables:
	echo $(ALLOBJECTS)

$(STYLE_HEADERS): testudo_%.h: %.tst generate_style
	./generate_style $<

$(OBJDIR)/%.o: %.cpp $(STYLE_HEADERS)
	@ mkdir -p $(DEPDIR) $(OBJDIR)
	$(GPP) -Wp,-MMD,$(DEPDIR)/$*.d,-MP,-MT,$@ -c $(CFLAGS) $(COMPILEOPTS) \
		-fPIC $< -o $@

$(TEST): $(ALLOBJECTS)
	$(GPP) -Wp,-MMD,$(DEPDIR)/$*.d,-MP,-MT,$@ $(CFLAGS) \
		$^ $(LINKOPTS) $(LDFLAGS) -o $@

test: $(TEST)

-include $(ALLDEPENDS)

clean_exe:
	rm -rf $(OBJDIR) $(DEPDIR)

XML_TO_COLOR := ./xml_to_color
XML_TO_COLOR_SUMMARY := ./xml_to_color -s
XML_TO_COLOR_LATEX := ./xml_to_color -l -w 63
UNCOLOR := ./uncolor
XML_TO_TEXT := ./xml_to_color -b
XML_FORMAT := -f xml
SYNC_COLOR_TEXT_FORMAT := -f color_text
SUCCESS_FILE := ttest.output.success

run_test: $(TEST)
	$< $(XML_FORMAT) | $(XML_TO_COLOR)
runsync_test: $(TEST)
	$< $(SYNC_COLOR_TEXT_FORMAT)
summary_test: $(TEST)
	$< $(XML_FORMAT) | $(XML_TO_COLOR_SUMMARY)
view_test: $(TEST)
	$< $(XML_FORMAT) | $(XML_TO_COLOR) | less -R
diff_test: $(TEST)
	diff -bwu <(cat $(SUCCESS_FILE) | $(XML_TO_TEXT)) \
		<($< $(XML_FORMAT) | $(XML_TO_TEXT))
diffxml_test: $(TEST)
	diff -bwu <(cat $(SUCCESS_FILE)) <($< $(XML_FORMAT))
success_test: $(TEST)
	cp -i <($< $(XML_FORMAT)) $(SUCCESS_FILE)

reports/%.tex: $(TEST) xml_to_color
	mkdir -p reports
	$< $(XML_FORMAT) -s $* | $(XML_TO_COLOR_LATEX) > $@

ascii_logo.png: ascii_logo_colour_cropped.pdf
	pdftocairo $< -png -singlefile -r 720 ascii_logo

clean_ascii_logo:
	rm -f ascii_logo.png

clean_report:
	rm -rf reports

clean: clean_doc clean_exe clean_report clean_ascii_logo
