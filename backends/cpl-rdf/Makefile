#
# Core Provenance Library
#
# Copyright (c) Peter Macko
#

ROOT :=../..


#
# Customize the build
#

SHARED := yes
INSTALL := yes

DEPENDENCIES := $(ROOT)/include/*.h
INCLUDE_FLAGS := $(INCLUDE_FLAGS) -I$(ROOT)/include
INCLUDE_FLAGS := $(INCLUDE_FLAGS) $(shell xml2-config --cflags)
LIBRARIES := -lcurl $(shell xml2-config --libs)

ifeq ($(UNAME),Darwin)
LINKER_SUBPROJECT_DEPENDENCIES := cpl-standalone
LIBRARIES := $(LIBRARIES) -lcpl-standalone
endif


#
# Include the magic script
#

include $(ROOT)/make/library.mk

