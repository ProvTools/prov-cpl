#
# Core Provenance Library
#
# Copyright (c) Peter Macko
#

ROOT := ../../..


#
# Customize the build
#

INSTALL := yes
DEPENDENCIES := ../../cpl.i
CXXFLAGS     := $(CXXFLAGS) -Wno-strict-aliasing
LINKER_FLAGS := $(LINKER_FLAGS)
LIBRARIES    := $(LIBRARIES) -lcpl -lcpl-odbc -lcpl-rdf


#
# Include the magic script
#

include $(ROOT)/make/java-module-swig.mk

