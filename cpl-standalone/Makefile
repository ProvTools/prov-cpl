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

DEPENDENCIES := $(ROOT)/include/*.h
INCLUDE_FLAGS := $(INCLUDE_FLAGS) -I$(ROOT)/include

ifeq ($(OSTYPE),darwin)
LIBRARIES := -framework IOKit -framework CoreFoundation
else
LIBRARIES := -luuid
endif


#
# Include the magic script
#

include ../make/library.mk

