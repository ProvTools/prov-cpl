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
# Include the header script
#

include $(ROOT)/make/header.mk


#
# Perl
#

ifndef PERL
	PERL := perl
endif


#
# Target
#

ifndef TARGET
	TARGET := $(PROJECT).pm
endif


#
# Handle subproject libraries
#

ifndef PROJECT_DIRS
	PROJECT_DIRS := $(ROOT) $(ADDITIONAL_PROJECT_DIRS) \
					$(PERL_MODULE_DEPENDECIES)
endif

ifdef __PROGRAM__NO_SUBPROJECT_FILES
	SUBPROJECT_SO_FILES :=
else
	SUBPROJECT_SO_FILES := \
		$(foreach pdir, \
			$(PROJECT_DIRS), \
			$(foreach proj, \
				$(shell unset MAKEFLAGS MFLAGS && $(MAKE) --no-print-directory \
					--quiet -C $(pdir) list-subproject-shared-lib-files), \
				$(pdir)/$(proj)))
endif

ifdef PERL_MODULE_DEPENDECIES
	PERL := $(PERL) \
		$(foreach pdir, \
			$(PERL_MODULE_DEPENDECIES), \
			$(foreach incdir, \
				$(shell unset MAKEFLAGS MFLAGS && \
					$(MAKE) --no-print-directory --quiet -C $(pdir) \
					list-perl-inc-dirs), \
				-I $(realpath $(pdir)/$(incdir))))
endif


#
# Build-related phony and preliminary targets
#

.PHONY: all clean distclean messclean

all: $(BUILD_DIR)/blib/lib/$(TARGET)

clean:: messclean
	@rm -rf $(BUILD_DIR) || true

distclean:: messclean
	@rm -rf $(BUILD_DIR_ALL) || true

messclean::
	@find . -name '*~' -delete 2> /dev/null || true

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)


#
# Special-purpose targets
#

.PHONY: lines todo list-subproject-lib-files \
		list-subproject-shared-lib-files list-subproject-libs \
		list-perl-inc-dirs

lines:
	@echo Total number of lines:
	@cat Makefile *.i *.cxx *.pl *.PL *.cpp *.c *.h *.hpp | wc -l

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
	@echo $(BUILD_DIR)/blib/arch/auto/$(PROJECT)/$(PROJECT).so

list-subproject-libs::

list-perl-inc-dirs::
	@echo $(BUILD_DIR)/blib/arch $(BUILD_DIR)/blib/lib


#
# Man page viewer
#

.PNONY: view-man view-man3

view-man view-man3: all
	@man -l $(BUILD_DIR)/blib/man3/$(PROJECT).3pm


#
# The library test target
#

.PHONY: test

ifndef RUN_DIR
RUN_DIR=.
endif

ABS_BUILD_DIR := $(abspath $(BUILD_DIR))
ABS_SO_DIR_PATH := $(ABS_BUILD_DIR)/blib/arch/auto/$(PROJECT)
EXTRA_LD_PATHS := $(ABS_BUILD_DIR):$(ABS_SO_DIR_PATH)

ifndef TEST
	TEST := test.pl
endif

test: $(TEST) all
	@cd "$(BUILD_DIR)" && $(LD_PATH_VAR)=$$$(LD_PATH_VAR):$(EXTRA_LD_PATHS) \
	$(PERL) -I $(ABS_BUILD_DIR)/blib/arch -I $(ABS_BUILD_DIR)/blib/lib \
		"$(realpath $<)"

