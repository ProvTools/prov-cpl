#
# Core Provenance Library
#
# Copyright (c) Peter Macko
#

ROOT :=..

include $(ROOT)/make/header.mk


#
# Customize the build
#

PROJECT := cpl

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
CXXFLAGS := -std=c++11

ifeq ($(OSTYPE),darwin)
LIBRARIES := -framework IOKit -framework CoreFoundation
else
LIBRARIES := -luuid
endif


#
# Include the magic script
#

include ../make/library.mk
