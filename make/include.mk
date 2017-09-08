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
# List the header files
#

ifndef HEADER_FILES
	HEADER_FILES := $(shell find . \( -name '*.h' -or -name '*.hpp' \) -print \
								| sed 's/^\.\///')
endif

HEADER_FILES := $(sort $(HEADER_FILES))
ALL_SOURCES := $(HEADER_FILES)


#
# No-op targets
#

.PHONY: all relink

all relink:
	@true


#
# Cleaning targets
#

.PHONY: clean distclean messclean

clean distclean messclean::
	@find . -name '*~' -delete 2> /dev/null || true


#
# Special-purpose targets
#

.PHONY: lines todo list-subproject-lib-files \
		list-subproject-shared-lib-files list-subproject-libs

lines:
	@echo Total number of lines:
	@cat Makefile *.cpp *.c *.h *.hpp | wc -l

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
# Install & uninstall
#

.PHONY: install uninstall

XXX := XXX
ifdef INSTALL
	INSTALL_DEPENDENCIES := $(INSTALL_DEPENDENCIES)
	INSTALL_FILES := $(INSTALL_FILES) $(HEADER_FILES)
	INSTALL_DIR := $(INSTALL_PREFIX)/include
else
	INSTALL_DEPENDENCIES :=
endif

install:: $(INSTALL_DEPENDENCIES)
ifdef INSTALL
	@mkdir -p $(INSTALL_DIR)
ifeq ($(OUTPUT_TYPE),kernel)
	@for i in $(INSTALL_FILES); do \
		echo '  INSTALL $(PWD_REL_SEP)'$$i; \
		mkdir -p "`dirname $(INSTALL_DIR)/$$i`"; \
		install -m 644 $$i $(INSTALL_DIR)/$$i; \
	done
else
	@for i in $(INSTALL_FILES); do \
		mkdir -p "`dirname $(INSTALL_DIR)/$$i`"; \
		echo 'install -m 644 $$i $(INSTALL_DIR)/'$$i; \
		install -m 644 $i $(INSTALL_DIR)/$$i; \
	done
endif
else
	@true
endif


# TODO Prune empty directories after uninstall

uninstall::
ifdef INSTALL
ifeq ($(OUTPUT_TYPE),kernel)
	@for i in $(INSTALL_FILES); do \
		echo '  DELETE  $(INSTALL_DIR)/'$$i; \
		/bin/rm -f $(INSTALL_DIR)/$$i; \
	done
else
	@for i in $(INSTALL_FILES); do \
		echo 'rm -f $(INSTALL_DIR)'/$$i; \
		/bin/rm -f $(INSTALL_DIR)/$$i; \
	done
endif
else
	@true
endif

