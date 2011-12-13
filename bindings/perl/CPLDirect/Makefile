#
# Core Provenance Library
#
# Copyright (c) Peter Macko
#

ROOT :=../../..


#
# Include the header script
#

include $(ROOT)/make/header.mk


#
# Target
#

TARGET=cpl_wrap.o


#
# Handle subproject libraries
#

ifdef __PROGRAM__NO_SUBPROJECT_FILES
	SUBPROJECT_SO_FILES :=
else
	SUBPROJECT_SO_FILES := $(shell unset MAKEFLAGS MFLAGS && $(MAKE) \
					--no-print-directory --quiet \
						-C $(ROOT) list-subproject-shared-lib-files)
	SUBPROJECT_SO_FILES := $(foreach proj, $(SUBPROJECT_SO_FILES), $(ROOT)/$(proj))
endif


#
# Build-related phony and preliminary targets
#

.PHONY: all clean distclean messclean

all: $(BUILD_DIR)/$(TARGET)

clean:: messclean
	@rm -rf $(BUILD_DIR) || true

distclean:: messclean
	@rm -rf $(BUILD_DIR_ALL) || true

messclean::
	@find . -name '*~' -delete 2> /dev/null || true

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)


#
# Compile Targets
#

$(BUILD_DIR)/CPLDirect.pm $(BUILD_DIR)/cpl_wrap.cxx: cpl.i ../../cpl.i
	@mkdir -p $(BUILD_DIR)
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  SWIG    $(PWD_REL_SEP)$<'
	@swig -c++ -perl5 -outdir $(BUILD_DIR) -o $(BUILD_DIR)/cpl_wrap.cxx $<
else
	swig -c++ -perl5 -outdir $(BUILD_DIR) -o $(BUILD_DIR)/cpl_wrap.cxx $<
endif

$(BUILD_DIR)/Makefile.PL: makemake.pl
	@mkdir -p $(BUILD_DIR)
ifeq ($(OUTPUT_TYPE),kernel)
	@/bin/cp -f $< $@
else
	/bin/cp -f $< $@
endif

# TODO Make the hard-coded "../../" to be dynamically computed

$(BUILD_DIR)/Makefile: $(BUILD_DIR)/Makefile.PL
ifneq ($(SUBPROJECT_SO_FILES),)
	@cd $(BUILD_DIR); \
		for i in $(SUBPROJECT_SO_FILES); do \
			/bin/rm -f `basename $$i` || true; \
			ln -s ../../$$i .; \
		done
endif
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  PERL    $(PWD_REL_SEP)$<'
	@cd "$(BUILD_DIR)" && perl Makefile.PL
else
	cd "$(BUILD_DIR)" && perl Makefile.PL
endif

$(BUILD_DIR)/$(TARGET): $(BUILD_DIR)/cpl_wrap.cxx $(BUILD_DIR)/Makefile
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  MAKE    $(PWD_REL)$(BUILD_DIR)'
	@make -C "$(BUILD_DIR)"
else
	make -C "$(BUILD_DIR)"
endif


#
# Special-purpose targets
#

.PHONY: lines todo list-subproject-lib-files \
		list-subproject-shared-lib-files list-subproject-libs

lines:
	@echo Total number of lines:
	@cat Makefile *.i *.cpp *.c *.h *.hpp | wc -l

todo:
	@(grep -n 'TODO' $(ALL_SOURCES) $(shell ls *.h 2>/dev/null) Makefile ;	\
	  grep -n '###'  $(ALL_SOURCES) $(shell ls *.h 2>/dev/null) Makefile ;	\
	  grep -n 'XXX'  $(ALL_SOURCES) $(shell ls *.h 2>/dev/null) Makefile )	\
		| sed 's/\(^.*:.*:\).*TODO/\1\tTODO/'				\
		| sed 's/\(^.*:.*:\).*###/\1\t###/'				\
		| sed 's/\(^.*:.*:\).*XXX/\1\tXXX/'				\
		| sed 's|\(^.\)|$(PWD_REL_SEP)\1|' | sort

list-subproject-lib-files::

list-subproject-shared-lib-files::

list-subproject-libs::


#
# The library test target
#

.PHONY: test

ifndef RUN_DIR
RUN_DIR=.
endif

REAL_BUILD_DIR := $(realpath $(BUILD_DIR))
REAL_SO_DIR_PATH := $(REAL_BUILD_DIR)/blib/arch/auto/$(PROJECT)
EXTRA_LD_PATHS := $(REAL_BUILD_DIR):$(REAL_SO_DIR_PATH)


test: test.pl all
	@cd "$(BUILD_DIR)" && $(LD_PATH_VAR)=$$$(LD_PATH_VAR):$(EXTRA_LD_PATHS) \
	perl -I $(REAL_BUILD_DIR)/blib/arch -I $(REAL_BUILD_DIR)/blib/lib \
		"$(realpath $<)"

