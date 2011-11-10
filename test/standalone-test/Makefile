#
# Core Provenance Library
#
# Copyright (c) Peter Macko
#

ROOT := ../..


#
# Source files
#

DEPENDENCIES := $(ROOT)/include/*.h ../platform-compat/include/*.h


#
# Customize the build
#

INCLUDE_FLAGS := $(INCLUDE_FLAGS) -I$(ROOT)/include \
	-I../platform-compat/include


#
# Curl
#

ifeq ($(UNAME),Darwin)
	LIBRARIES_CURL := -lcurl
else
	LIBRARIES_CURL := `pkg-config --libs libcurl`
endif


#
# Flags and libraries
#

CXXFLAGS      := $(CXXFLAGS) -Wno-sign-compare
INCLUDE_FLAGS := $(INCLUDE_FLAGS)
LINKER_FLAGS  := $(LINKER_FLAGS)
LIBRARIES     := $(LIBRARIES) -lodbc $(LIBRARIES_CURL)


#
# Target executable
#

TARGET := standalone-test


#
# Include the magic script
#

include $(ROOT)/make/program.mk

