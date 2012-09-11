#
# Core Provenance Library
#
# Copyright (c) Peter Macko
#

ROOT :=../..

include $(ROOT)/make/header.mk


#
# Customize the build
#

SHARED := yes
INSTALL := yes

SO_MAJOR_VERSION := $(shell cat "$(ROOT)/include/cpl.h" \
	| grep 'define CPL_VERSION_MAJOR' \
	| sed 's/^[^0-9]*//g' | head -n 1)
SO_MINOR_VERSION := $(shell cat "$(ROOT)/include/cpl.h" \
	| grep 'define CPL_VERSION_MINOR' \
	| sed 's/^[^0-9]*//g' | head -n 1)

DEPENDENCIES := $(ROOT)/include/*.h
INCLUDE_FLAGS := $(INCLUDE_FLAGS) -I$(ROOT)/include
INCLUDE_FLAGS := $(INCLUDE_FLAGS) $(shell xml2-config --cflags)
LIBRARIES := -lcurl $(shell xml2-config --libs)

ifeq ($(OSTYPE),darwin)
LINKER_SUBPROJECT_DEPENDENCIES := cpl-standalone
LIBRARIES := $(LIBRARIES) -lcpl
endif


#
# Include the magic script
#

include $(ROOT)/make/library.mk

