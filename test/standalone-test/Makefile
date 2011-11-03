#
# Core Provenance Library
#
# Copyright (c) Peter Macko
#

ROOT := ../..


#
# Source files
#

DEPENDENCIES := $(ROOT)/include/*.h


#
# Customize the build
#

INCLUDE_FLAGS := $(INCLUDE_FLAGS) -I$(ROOT)/include


#
# Flags and libraries
#

CXXFLAGS      := $(CXXFLAGS) -Wno-sign-compare
INCLUDE_FLAGS := $(INCLUDE_FLAGS)
LINKER_FLAGS  := $(LINKER_FLAGS)
LIBRARIES     := $(LIBRARIES) -lodbc `pkg-config --libs libcurl`


#
# Target executable
#

TARGET := standalone-test


#
# Include the magic script
#

include $(ROOT)/make/program.mk

