#
# Core Provenance Library
#
# Copyright (c) Peter Macko
#

ROOT := ../..

include $(ROOT)/make/header.mk

#
# Customize the build
#

INCLUDE_FLAGS := $(INCLUDE_FLAGS) -I$(ROOT)/include \
	-I$(PLATFORM_COMPAT)/include


#
# Flags and libraries
#

CXXFLAGS      := $(CXXFLAGS) -Wno-sign-compare
INCLUDE_FLAGS := $(INCLUDE_FLAGS)
LINKER_FLAGS  := $(LINKER_FLAGS)
LIBRARIES     := $(LIBRARIES)


#
# Target executable
#

TARGET := standalone-test


#
# Include the magic script
#

include $(ROOT)/make/program.mk

