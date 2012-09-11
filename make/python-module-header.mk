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

ifndef PYTHON
	PYTHON := python
endif


#
# Target
#

ifndef TARGET
	TARGET := $(PROJECT).pyc
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
# Determine information about the Python installation
#

PYTHON_INCLUDE="/usr/include/$(shell ls -1 /usr/include | grep python \
			   | grep -v '_d' | tail -n 1)"

#PYTHON_PATH="$(shell $(PYTHON) -c \
#			"exec(\"import sys\\nimport string\\nprint(string.join(sys.path, \\\":\\\"))\")")"


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
# Shortcut targets
#

.PHONY: release Release debug Debug

debug: all

release:
	@$(MAKE) --no-print-directory RELEASE=yes all


# Aliases

Debug: debug

Release: release


#
# Special-purpose targets
#

.PHONY: lines todo list-subproject-lib-files \
		list-subproject-shared-lib-files list-subproject-libs \
		list-perl-inc-dirs

lines:
	@echo Total number of lines:
	@cat Makefile *.i *.cxx *.py *.PY *.cpp *.c *.h *.hpp | wc -l

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
	@echo $(BUILD_DIR)/$(PROJECT).so

list-subproject-libs::

list-python-inc-dirs::
	@echo $(BUILD_DIR)


#
# The library test target
#

.PHONY: test

ifndef RUN_DIR
RUN_DIR=.
endif

ABS_BUILD_DIR := $(abspath $(BUILD_DIR))
ABS_SO_DIR_PATH := $(ABS_BUILD_DIR)
EXTRA_LD_PATHS := $(ABS_SO_DIR_PATH)

ifndef TEST
	TEST := test.py
endif

test: $(TEST) all
	@cd "$(BUILD_DIR)" && $(LD_PATH_VAR)=$$$(LD_PATH_VAR):$(EXTRA_LD_PATHS) \
		PYTHONPATH=$(ABS_BUILD_DIR) \
		$(PYTHON) "$(realpath $<)"


#
# Install & unistall
#

.PHONY: install uninstall

install:: release $(INSTALL_DEPENDENCIES)
ifdef INSTALL
ifdef RELEASE
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  INSTALL $(PWD_REL_SEP)$(BUILD_DIR)'
	@make --no-print-directory -C "$(BUILD_DIR)" install
else
	make -C "$(BUILD_DIR)" install
endif
else
	@$(MAKE) --no-print-directory RELEASE=yes install
endif
else
	@true
endif

uninstall:: $(BUILD_DIR)/Makefile
ifdef INSTALL
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  UNINST  $(PWD_REL_SEP)$(BUILD_DIR)'
	@make --no-print-directory -C "$(BUILD_DIR)" uninstall
else
	make -C "$(BUILD_DIR)" uninstall
endif
else
	@true
endif

