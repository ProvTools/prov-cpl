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
OPTIONAL := bindings/java

#
# Project configuration
#

RUN_PROGRAM := test


#
# Include the magic script
#

include $(ROOT)/make/project.mk
