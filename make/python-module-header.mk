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
	PYTHON := python -B
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
					$(PYTHON_MODULE_DEPENDECIES)
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

ifdef PYTHON_MODULE_DEPENDECIES
	PYTHON_PATH_EXTRA := $(PYTHON_PATH_EXTRA) \
		$(foreach pdir, \
			$(PYTHON_MODULE_DEPENDECIES), \
			$(foreach incdir, \
				$(shell unset MAKEFLAGS MFLAGS && \
					$(MAKE) --no-print-directory --quiet -C $(pdir) \
					list-python-inc-dirs), \
				$(realpath $(pdir)/$(incdir))))
endif


#
# Determine information about the Python installation
#

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
	@rm -f *.pyc *.pyo || true

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
	@echo $(BUILD_DIR)/_$(PROJECT).so

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
ABS_SO_DIR_PATH := $(ABS_BUILD_DIR):
EXTRA_LD_PATHS := $(ABS_SO_DIR_PATH)

ifndef TEST
	TEST := test.py
endif

test: $(TEST) all
	@cd "$(BUILD_DIR)" && $(LD_PATH_VAR)=$$$(LD_PATH_VAR):$(EXTRA_LD_PATHS) \
		PYTHONPATH=$(ABS_BUILD_DIR):$(subst $(SPACE),:,$(PYTHON_PATH_EXTRA)) \
		$(PYTHON) "$(realpath $<)"


#
# Install & unistall
#

.PHONY: install uninstall

install:: release $(INSTALL_DEPENDENCIES) setup.py
ifdef INSTALL
ifdef RELEASE
ifeq ($(OUTPUT_TYPE),kernel)
	@echo '  INSTALL $(PWD_REL_SEP)$(BUILD_DIR)'
	@cd "$(BUILD_DIR)" && python $(abspath setup.py) install --skip-build
else
	cd "$(BUILD_DIR)" && python $(abspath setup.py) install --skip-build
endif
else
	@$(MAKE) --no-print-directory RELEASE=yes install
endif
else
	@true
endif

uninstall::
ifdef INSTALL
	@echo "Error: Uninstall of Python modules is currently not supported"
	@exit 1
else
	@true
endif

