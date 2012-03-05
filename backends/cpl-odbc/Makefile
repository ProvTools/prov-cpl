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
LIBRARIES := -lodbc

ifeq ($(OSTYPE),darwin)
LINKER_SUBPROJECT_DEPENDENCIES := cpl-standalone
LIBRARIES := $(LIBRARIES) -lcpl
endif


#
# Include the magic script
#

include $(ROOT)/make/library.mk

