#
# Core Provenance Library
#
# Copyright (c) Peter Macko
#

ROOT := ../..

include $(ROOT)/make/header.mk


#
# Source files
#

PLATFORM_COMPAT := $(ROOT)/private-lib/platform-compat
DEPENDENCIES := $(ROOT)/include/*.h $(PLATFORM_COMPAT)/include/*.h


#
# Customize the build
#

INCLUDE_FLAGS := $(INCLUDE_FLAGS) -I$(ROOT)/include \
	-I$(PLATFORM_COMPAT)/include


#
# Flags and libraries
#

CXXFLAGS      := $(CXXFLAGS)
INCLUDE_FLAGS := $(INCLUDE_FLAGS)
LINKER_FLAGS  := $(LINKER_FLAGS)
LIBRARIES     := $(LIBRARIES) -ltermcap


#
# Target executable
#

TARGET := cpl


#
# Other configuration
#

INSTALL := yes


#
# Include the magic script
#

include $(ROOT)/make/program.mk

