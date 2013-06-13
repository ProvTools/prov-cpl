#
# Core Provenance Library
#
# Copyright (c) Peter Macko
#

ROOT := .


#
# Subprojects
#

LIBRARIES := cpl-standalone backends private-lib
HEADERS := include
PROGRAMS := tools test
OPTIONAL := bindings/perl bindings/java bindings/python


#
# Project configuration
#

RUN_PROGRAM := test


#
# Include the magic script
#

include $(ROOT)/make/project.mk

