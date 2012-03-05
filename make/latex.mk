#
# Alpha Make
#
# Copyright (c) Peter Macko
#


#
# Set the project root
#

ifndef ROOT
	ROOT := .
endif


#
# Include the header file
#

include $(ROOT)/make/header.mk


#
# Configure the project
#

ifndef PROJECT
	PROJECT := $(strip $(notdir $(shell "pwd")))
endif


#
# Sources
#

ifndef SOURCES
	SOURCES := $(shell ls *.tex 2> /dev/null)
endif

ifndef BIB
	BIB := $(shell ls *.bib 2> /dev/null)
endif

ifndef MAIN_SOURCE
	MAIN_SOURCE := $(strip $(PROJECT)).tex
endif

ifeq ($(shell ls -1 -d $(MAIN_SOURCE) 2> /dev/null | grep -c $(MAIN_SOURCE) 2> /dev/null),1)
	MAIN_SOURCE_STATUS := ok
	ifneq ($(strip $(MAIN_SOURCE)),$(strip $(PROJECT)).tex)
		PROJECT := $(basename $(MAIN_SOURCE))
	endif
else
	MAIN_SOURCE_STATUS := error
	MAIN_SOURCE :=
endif


#
# Target
#

ifndef TARGET
	TARGET := $(PROJECT).pdf
endif


#
# Additional dependencies
#

ifndef DEPENDENCIES_DIRS
	DEPENDENCIES_DIRS := figures images screenshots graphs sections
endif

DEPENDENCIES := $(DEPENDENCIES) $(foreach dir,$(DEPENDENCIES_DIRS),$(wildcard $(dir)/*))
DEPENDENCIES := $(sort $(DEPENDENCIES))


#
# Latex commands
#

PDFLATEX   := pdflatex -halt-on-error -file-line-error
BIBTEX     := bibtex
CMD_SUFFIX := 2>&1 | $(ROOT)/make/tools/colorlatex.pl ; exit $${PIPESTATUS[0]}


#
# Preview
#

ifeq ($(OSTYPE),darwin)
	VIEW := open
	VIEW_SUFFIX :=
else
	ifneq ($(shell ps -A | grep -c kdeinit),0)
		VIEW := okular
	else
		VIEW := evince
	endif
	VIEW_SUFFIX := 2> /dev/null &
endif


#
# Main targets
#

.PHONY: all pdf target

all pdf target: $(TARGET)


#
# Compile
#

$(TARGET): $(MAIN_SOURCE) $(SOURCES) $(BIB) $(DEPENDENCIES)
	@if [ "++$(MAIN_SOURCE_STATUS)" = "++error" ]; then	\
		echo "Error: The main source file does not exist."; \
		echo "       Either create $(PROJECT).tex or specify MAIN_SOURCE in your Makefile."; \
		exit 1; \
	fi
	@$(PDFLATEX) $(MAIN_SOURCE) $(CMD_SUFFIX)
	@$(BIBTEX)   $(PROJECT)     $(CMD_SUFFIX)
	@$(PDFLATEX) $(MAIN_SOURCE) $(CMD_SUFFIX)
	@$(PDFLATEX) $(MAIN_SOURCE) $(CMD_SUFFIX)


#
# View
#

.PHONY: view

view: $(TARGET)
	$(VIEW) $(TARGET) $(VIEW_SUFFIX)


#
# Clean
#

.PHONY: clean messclean distclean

messclean::
	@rm -f *.aux .*~ \\#* .#* *~ *.log *.lot *.lof *.blg *.bbl 2> /dev/null || true

clean distclean:: messclean
	@rm -f $(TARGET) 2> /dev/null || true
