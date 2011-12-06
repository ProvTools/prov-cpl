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
# libxml2
#

LIBRARIES_XML2 := $(shell xml2-config --libs)


#
# uuid
#

ifneq ($(UNAME),Darwin)
LIBRARIES_UUID := -luuid
endif


#
# Flags and libraries
#

CXXFLAGS      := $(CXXFLAGS) -Wno-sign-compare
INCLUDE_FLAGS := $(INCLUDE_FLAGS)
LINKER_FLAGS  := $(LINKER_FLAGS)
LIBRARIES     := $(LIBRARIES) -lodbc \
						$(LIBRARIES_UUID) $(LIBRARIES_CURL) $(LIBRARIES_XML2)


#
# Target executable
#

TARGET := standalone-test


#
# Include the magic script
#

include $(ROOT)/make/program.mk

