#
# Core Provenance Library
#
# Copyright (c) Peter Macko
#

ROOT := ../../..


#
# Customize the build
#

INSTALL       := yes
DEPENDENCIES  := ../../cpl.i $(ROOT)/include/cpl.h $(ROOT)/include/cpl*.h
CXXFLAGS      := $(CXXFLAGS)
INCLUDE_FLAGS := $(INCLUDE_FLAGS) -I$(ROOT)/include
LINKER_FLAGS  := $(LINKER_FLAGS)
LIBRARIES     := $(LIBRARIES) -lcpl -lcpl-odbc -lcpl-rdf


#
# Include the magic script
#

include $(ROOT)/make/java-module-swig.mk

