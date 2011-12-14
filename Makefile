#
# Core Provenance Library
#
# Copyright (c) Peter Macko
#

ROOT := .


#
# Subprojects
#

LIBRARIES := cpl-standalone backends
HEADERS := include
PROGRAMS := test
OPTIONAL := bindings/perl


#
# Include the magic script
#

include $(ROOT)/make/project.mk

